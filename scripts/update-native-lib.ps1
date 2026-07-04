<#
.SYNOPSIS
    Copies a freshly built libdaas.a (all ABIs) and its matching headers from a local `daas`
    source repo checkout into this SDK, consistently.

.DESCRIPTION
    See daasiotsdk/src/main/jniLibs/PROVENANCE.md for the full story on why this exists and how
    libdaas.a is actually built (short version: it's cross-compiled for Android from the `daas`
    core C++ source repo via `platforms/build_daas_linux.sh android`, run inside WSL). This script
    does not build libdaas - it only copies already-built output from that repo into this one, so
    the copy step itself can't accidentally mix ABIs/headers from different builds.

    By default all three ABIs are required and the script fails if any is missing from the source
    repo's output, since a partial update (e.g. only arm64-v8a) is exactly what caused the
    SimpleBLE link failure documented in PROVENANCE.md. Pass -Abis to override this deliberately.

.PARAMETER DaasRepoPath
    Path to a local checkout of the `daas` source repository, after running
    `platforms/build_daas_linux.sh android` inside it (or an equivalent manual build). This script
    reads from `<DaasRepoPath>/output/android/<armv8|armv7|x86_64>/lib/libdaas.a` and
    `<DaasRepoPath>/output/include/{daas.hpp,daas_types.hpp}`.

.PARAMETER Abis
    Which ABIs to copy. Defaults to all three. Only narrow this if you specifically intend to
    leave the others as they are - see the warning above.

.EXAMPLE
    ./scripts/update-native-lib.ps1 -DaasRepoPath C:\path\to\daas

.EXAMPLE
    # Deliberately update only arm64-v8a (leaves armeabi-v7a/x86_64 untouched - creates a known
    # mismatch versus the shared headers unless you understand why you're doing this):
    ./scripts/update-native-lib.ps1 -DaasRepoPath C:\path\to\daas -Abis arm64-v8a
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$DaasRepoPath,

    [ValidateSet("arm64-v8a", "armeabi-v7a", "x86_64")]
    [string[]]$Abis = @("arm64-v8a", "armeabi-v7a", "x86_64")
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$jniLibsDir = Join-Path $repoRoot "daasiotsdk/src/main/jniLibs"
$includeDir = Join-Path $repoRoot "daasiotsdk/src/main/include"

$abiToOutputDir = @{
    "arm64-v8a"   = "armv8"
    "armeabi-v7a" = "armv7"
    "x86_64"      = "x86_64"
}

if (-not (Test-Path $DaasRepoPath)) {
    throw "DaasRepoPath does not exist: $DaasRepoPath"
}

# --- Verify every requested ABI's output exists before copying anything ---
$sourceLibs = @{}
foreach ($abi in $Abis) {
    $outputDir = $abiToOutputDir[$abi]
    $libPath = Join-Path $DaasRepoPath "output/android/$outputDir/lib/libdaas.a"
    if (-not (Test-Path $libPath)) {
        throw "Missing built libdaas.a for '$abi' at expected path: $libPath`n" +
              "Run 'platforms/build_daas_linux.sh android' inside $DaasRepoPath first (in WSL)."
    }
    $sourceLibs[$abi] = $libPath
}

$headerFiles = @("daas.hpp", "daas_types.hpp")
$sourceHeaders = @{}
foreach ($header in $headerFiles) {
    $headerPath = Join-Path $DaasRepoPath "output/include/$header"
    if (-not (Test-Path $headerPath)) {
        throw "Missing header at expected path: $headerPath"
    }
    $sourceHeaders[$header] = $headerPath
}

# --- All present: copy ---
foreach ($abi in $Abis) {
    $destDir = Join-Path $jniLibsDir $abi
    New-Item -ItemType Directory -Force -Path $destDir | Out-Null
    Copy-Item -Path $sourceLibs[$abi] -Destination (Join-Path $destDir "libdaas.a") -Force
    Write-Host "Copied $abi/libdaas.a"
}

foreach ($header in $headerFiles) {
    Copy-Item -Path $sourceHeaders[$header] -Destination (Join-Path $includeDir $header) -Force
    Write-Host "Copied include/$header"
}

# --- Capture provenance info to help fill in PROVENANCE.md by hand ---
Write-Host ""
Write-Host "Done. Now update the table in daasiotsdk/src/main/jniLibs/PROVENANCE.md:" -ForegroundColor Yellow

$gitDir = Join-Path $DaasRepoPath ".git"
if (Test-Path $gitDir) {
    Push-Location $DaasRepoPath
    try {
        $commit = git rev-parse --short HEAD 2>$null
        $describe = git describe --tags --always 2>$null
        if ($commit) {
            Write-Host "  daas repo commit: $commit ($describe)"
        }
    } finally {
        Pop-Location
    }
} else {
    Write-Host "  ($DaasRepoPath is not a git checkout - record the version manually)"
}
Write-Host "  ABIs updated: $($Abis -join ', ')"
Write-Host "  Date: $(Get-Date -Format 'yyyy-MM-dd')"
