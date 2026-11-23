# daas © 2019–2026 Sebyone Srl
#
# File: fetch_ld.ps1 — daasiot-android
#
# PowerShell variant of fetch_ld.sh, for Windows hosts (Android Studio on Windows, WSL, Docker, a cross
# toolchain) without bash. It installs exactly the packages listed in android_daas/libdaas.lock.json: for each
# "<target>/<linkage>" entry it takes the file name ("tarball") and the SHA-256 ("sha256") written there, verifies
# that the official source has the file (listed as available in the catalog https://daasiot.com/api/libdaas.php and
# served as a .tar.gz at the download_url of its entry, https://daasiot.com/api/download-core.php?file=<file>),
# downloads it, checks its SHA-256 against the lock and, when the catalog publishes it, against <file>.sha256, and
# unpacks it into libdaas/<target>/<linkage>/ (with drivers_sdk/, the headers of the driver contract, from libdaas
# 0.24.8). Same files and INSTALLED.json as fetch_ld.sh. The lock is the list: a new target, another package or
# another libdaas version is a change of the lock (-Verify prints the published SHA-256 to write there).
# Needs Windows PowerShell 5.1 (or PowerShell 7) and tar.exe (Windows 10 1803 or later).
#
#   powershell -ExecutionPolicy Bypass -File tools\fetch_ld\fetch_ld.ps1             every package of the lock
#   ... fetch_ld.ps1 -Only android-arm64-v8a/static
#   ... fetch_ld.ps1 -Verify [-Only <target>/<linkage>]   only check the official source (and the published SHA-256)
#   ... fetch_ld.ps1 -Check                  offline check of what is installed (exit code 1 if not ok)
#   ... fetch_ld.ps1 -List                   the libdaas packages of the catalog
#   ... fetch_ld.ps1 -h                      summary of the commands (also -Help, --help, /?)
#
# Exit code: 0 done, 1 a check or a download failed.
#
# Contributors:
#   Paolo Lo Giacco <plogiacco@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Version: $FetchLdVersion below (FETCH_LD_VERSION in fetch_ld.sh), the same in fetch_ld.sh and fetch_ld.ps1 (one
# tool), shown in the heading of every run. Raise it at every change of the code of either (M.m.p: p a fix, m a new
# option or behaviour, M an incompatible change) and add a line to Changes; keep the two equal (daasiot-gnu checks it in tests/auto/t12).
#
# Changes:
#   2026-11-08  Paolo Lo Giacco  creation (same behaviour as fetch_ld.sh)
#   2026-11-09  Paolo Lo Giacco  1.0.0: version of the tool (FETCH_LD_VERSION) shown with the holder and the copyright
#                                at every run; -h/-help/--help; renamed from fetch_libdaas; fetch_ld.ps1 added
#   2026-11-09  Paolo Lo Giacco  1.0.1: fetch_ld.ps1 uses the tar.exe of Windows (from Git Bash the GNU tar of Git
#                                read C:\... as a remote host); fetch_ld.py exits 1 on an unknown option (was 2)
#   2026-11-09  Paolo Lo Giacco  1.0.2: an install keeps in INSTALLED.json the other packages of the lock still
#                                installed (.sh, .ps1 kept only the packages of the run); --check of fetch_ld.py
#                                reports a package of the lock missing from INSTALLED.json
#   2026-11-09  plogiacco@sebyone.it  1.0.3: target windows-x86_64-mingw32 (libdaas-<v>-windows-x86_64-mingw32-gcc-*)
#   2026-11-09  plogiacco@sebyone.it  2.0.0: the packages are the ones written in the lock (tarball, sha256), no file
#                                     name composed from a table of targets: any <target>/<linkage> of the lock is
#                                     installed; fetch_ld.py removed (the tool is .sh and .ps1), -Version removed;
#                                     -Verify prints the published SHA-256; -List shows every libdaas package
#   2026-11-23  l.grillo@sebyone.it  daasiot-android: lock android_daas/libdaas.lock.json, targets android-<ABI>
#                                    (static); same tool, FETCH_LD_VERSION 2.0.0

[CmdletBinding(PositionalBinding = $false)]
param(
    [string[]]$Only = @(),
    [switch]$Verify,
    [switch]$Check,
    [switch]$List,
    [Alias("h")][switch]$Help,
    [Parameter(ValueFromRemainingArguments = $true)][string[]]$Rest = @()
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"            # Invoke-WebRequest is much slower with the progress bar
[Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12

$FetchLdVersion = "2.0.0"                    # version of this tool (see Version above)
$Catalog   = "https://daasiot.com/api/libdaas.php"
$Site      = "https://daasiot.com"
$Download  = "/api/download-core.php?file="
$UserAgent = "daasiot-android-fetch-ld"
$Root  = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Lock  = Join-Path $Root "android_daas\libdaas.lock.json"
$Dest  = Join-Path $Root "libdaas"
$Stamp = Join-Path $Dest "INSTALLED.json"

function Log([string]$m) { Write-Host "[libdaas] $m" }
function Fail([string]$m) { Write-Host "[libdaas] $m" -ForegroundColor Red; exit 1 }

function Abs-Url([string]$u) { if ($u -match '^https?://') { $u } else { $Site + $u } }

function Get-Bytes([string]$url) {
    (Invoke-WebRequest -Uri $url -UserAgent $UserAgent -UseBasicParsing).Content
}

# first two bytes of a URL (HttpWebRequest: the Range header cannot be set through Invoke-WebRequest in 5.1)
function Probe([string]$url) {
    try {
        $req = [Net.HttpWebRequest]::Create($url)
        $req.UserAgent = $UserAgent
        $req.AddRange(0, 1)
        $req.Timeout = 60000
        $resp = $req.GetResponse()
        try {
            $s = $resp.GetResponseStream()
            $b = New-Object byte[] 2
            $n = $s.Read($b, 0, 2)
            if ($n -lt 2) { $n += $s.Read($b, $n, 2 - $n) }
            return ($n -eq 2 -and $b[0] -eq 0x1f -and $b[1] -eq 0x8b)
        } finally { $resp.Close() }
    } catch { return $false }
}

$script:Entries = $null
function Catalog-Entries {
    if ($null -eq $script:Entries) {
        try { $json = Invoke-RestMethod -Uri $Catalog -UserAgent $UserAgent -UseBasicParsing }
        catch { Fail "catalog not reachable: $Catalog ($($_.Exception.Message))" }
        $script:Entries = @($json.releases | Where-Object { $_.file })
    }
    return $script:Entries
}

# the URL of the package and of its published checksum ($null if none), or $null with a reason
function Verify-Package([string]$name) {
    $e = Catalog-Entries | Where-Object { $_.file -eq $name } | Select-Object -First 1
    if ($null -eq $e) { Log "$name is not in the catalog $Catalog"; return $null }
    if (-not $e.available) { Log "$name is listed in the catalog but marked not available"; return $null }
    $url = if ($e.download_url) { Abs-Url $e.download_url } else { Abs-Url ($Download + $name) }
    if (-not (Probe $url)) { Log "$name is listed as available but $url does not serve a .tar.gz"; return $null }
    $sha = $null
    if ($e.sig_available -and "$($e.signature)" -eq "$name.sha256") {
        $sha = if ($e.sig_url) { Abs-Url $e.sig_url } else { Abs-Url ($Download + "$name.sha256") }
    }
    return @{ url = $url; sha = $sha }
}

# the SHA-256 the catalog publishes for a file, or $null
function Published-Sha256([string]$url) {
    try { $text = [Text.Encoding]::ASCII.GetString((Get-Bytes $url)) } catch { return $null }
    return ($text.Trim() -split '\s+')[0].ToLower()
}

function Show-Help {
    @'
fetch_ld.ps1: libdaas for daasiot-android from the catalog of daasiot.com, the packages of android_daas\libdaas.lock.json

usage: powershell -ExecutionPolicy Bypass -File tools\fetch_ld\fetch_ld.ps1 [command] [options]

commands:
  (none)                      verify, download and install every package of the lock into libdaas\
  -Verify                     only check that the catalog has the packages of the lock and serves them, and
                              compare the published SHA-256 with the lock (nothing installed)
  -Check                      offline check of what is installed (exit code 1 if not ok)
  -List                       the libdaas packages of the catalog
  -h, -Help, --help, /?       this summary

options:
  -Only <target>/<linkage>    one package of the lock (several: -Only a,b), e.g. -Only android-arm64-v8a/static

the lock: "version" and, under "packages", one "<target>/<linkage>": { "tarball": "<file of the catalog>",
"sha256": "<its SHA-256>" } per package; edit it to add a target or change the version (-Verify gives the SHA-256)
exit code: 0 done, 1 a check or a download failed
'@
}

# heading of every run: the tool, the release of daasiot-android, the holder and the copyright
if ([Console]::IsOutputRedirected) { [Console]::OutputEncoding = New-Object Text.UTF8Encoding $false }   # a pipe or a file
$releaseFile = Join-Path $Root "VERSION"
$release = if (Test-Path $releaseFile) { " " + (Get-Content -Raw $releaseFile).Trim() } else { "" }
Write-Host "fetch_ld $FetchLdVersion (daasiot-android$release): libdaas from the catalog of daasiot.com"
Write-Host "daas $([char]0x00A9) 2019$([char]0x2013)2026 Sebyone Srl (sebyone.it), holder of daas and of daasiot-android. MPL-2.0."
Write-Host ""

if ($Help -or ($Rest | Where-Object { $_ -match '^(--?h(elp)?|/\?)$' })) { Show-Help; exit 0 }
if ($Rest.Count -gt 0) { Fail "unknown option $($Rest[0]) (-h: the commands)" }
if (-not (Test-Path $Lock)) { Fail "lock not found: $Lock" }
$lockJson = Get-Content -Raw -Encoding UTF8 $Lock | ConvertFrom-Json
$Version = "$($lockJson.version)"
if (-not $Version) { Fail "no `"version`" in $Lock" }
if (-not $lockJson.packages) { Fail "no package in $Lock" }

# the packages of the lock: key, tarball, sha256
$all = [ordered]@{}
foreach ($p in $lockJson.packages.PSObject.Properties) {
    $all[$p.Name] = @{ name = "$($p.Value.tarball)"; sha256 = "$($p.Value.sha256)".ToLower() }
}
$keys = if ($Only.Count -gt 0) { @($Only | ForEach-Object { $_ -split ',' } | Where-Object { $_ }) } else { @($all.Keys) }
foreach ($k in $keys) {
    if (-not $all.Contains($k)) { Fail "$k is not in the lock android_daas\libdaas.lock.json ($(@($all.Keys) -join ' '))" }
    if (-not $all[$k].name) { Fail "${k}: no `"tarball`" in the lock" }
}

# ---- list
if ($List) {
    Log "catalog $Catalog (lock: libdaas $Version)"
    Catalog-Entries | Where-Object { $_.file -match '^libdaas-.*\.tar\.gz$' } |
        Sort-Object -Property @{ Expression = { [version]$_.version } }, file -Descending |
        ForEach-Object { "  {0,-10} {1}  (available: {2})" -f $_.version, $_.file, $_.available.ToString().ToLower() }
    exit 0
}

# ---- check (offline)
if ($Check) {
    $problems = 0
    $installed = $null
    if (Test-Path $Stamp) { $installed = Get-Content -Raw -Encoding UTF8 $Stamp | ConvertFrom-Json }
    foreach ($k in $keys) {
        $dir = Join-Path $Dest $k
        if (-not (Test-Path (Join-Path $dir "include\daas.hpp"))) { Log "missing libdaas/$k/include/daas.hpp"; $problems++ }
        elseif (-not (Test-Path (Join-Path $dir "lib\libdaas.a")) -and -not (Test-Path (Join-Path $dir "lib\libdaas.so"))) {
            Log "missing libdaas/$k/lib/libdaas.*"; $problems++
        }
        if ($installed -and $installed.packages.$k -and $installed.packages.$k.tarball -ne $all[$k].name) {
            Log "libdaas/$k was not installed from $($all[$k].name)"; $problems++
        }
    }
    if (-not $installed -or "$($installed.version)" -ne $Version) {
        Log "installed libdaas $(if ($installed) { $installed.version } else { 'unknown' }), required $Version"; $problems++
    }
    if ($problems) { Log "run: tools\fetch_ld\fetch_ld.ps1"; exit 1 }
    Log "libdaas $Version installed: $($keys -join ' ')"
    exit 0
}

# ---- verify / install
Log "libdaas ${Version}: verifying $($keys.Count) package(s) of the lock in $Catalog"
$found = @{}
$missing = $false
foreach ($k in $keys) {
    $name = $all[$k].name
    $v = Verify-Package $name
    if ($null -eq $v) { $missing = $true } else { $found[$k] = $v; Log "${k}: $name found" }
}
if ($Verify) {
    foreach ($k in $keys) {
        if (-not $found.Contains($k) -or -not $found[$k].sha) { continue }
        $published = Published-Sha256 $found[$k].sha
        if (-not $published) { Log "${k}: published checksum not readable"; $missing = $true }
        elseif (-not $all[$k].sha256) { Log "${k}: no sha256 in the lock; published SHA-256 of $($all[$k].name): $published"; $missing = $true }
        elseif ($all[$k].sha256 -ne $published) { Log "${k}: the lock has SHA-256 $($all[$k].sha256), the catalog publishes $published"; $missing = $true }
        else { Log "${k}: the SHA-256 of the lock is the published one" }
    }
    if ($missing) { exit 1 }
    Log "every package of the lock is in the official source"
    exit 0
}
if ($missing) { Fail "nothing installed: the official source does not have every package (see above)" }
# the tar.exe of Windows (bsdtar): from Git Bash the first tar.exe of PATH is the GNU tar of Git, which reads
# C:\... as a remote host
$Tar = Join-Path $env:SystemRoot "System32\tar.exe"
if (-not (Test-Path $Tar)) {
    $cmd = Get-Command tar.exe -ErrorAction SilentlyContinue
    if (-not $cmd) { Fail "tar.exe is required (Windows 10 1803 or later)" }
    $Tar = $cmd.Source
}

$work = Join-Path ([IO.Path]::GetTempPath()) ("daas-fetch-" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force $work | Out-Null
New-Item -ItemType Directory -Force $Dest | Out-Null
$records = [ordered]@{}
try {
    foreach ($k in $keys) {
        $name = $all[$k].name
        $expected = $all[$k].sha256
        if (-not $expected) { Fail "${k}: no `"sha256`" in the lock: write the published one (fetch_ld.ps1 -Verify -Only $k)" }
        $file = Join-Path $work $name
        try { Invoke-WebRequest -Uri $found[$k].url -UserAgent $UserAgent -UseBasicParsing -OutFile $file }
        catch { Fail "download failed: $($found[$k].url) ($($_.Exception.Message))" }
        $head = [IO.File]::ReadAllBytes($file) | Select-Object -First 2
        if ($head.Count -lt 2 -or $head[0] -ne 0x1f -or $head[1] -ne 0x8b) { Fail "$name is not a .tar.gz archive" }
        $digest = (Get-FileHash -Algorithm SHA256 $file).Hash.ToLower()
        if ($digest -ne $expected) { Fail "${name}: SHA-256 $digest does not match the lock ($expected)" }
        if ($found[$k].sha) {
            $published = Published-Sha256 $found[$k].sha
            if (-not $published) { Fail "checksum not readable: $($found[$k].sha)" }
            if ($digest -ne $published) { Fail "${name}: SHA-256 $digest does not match the published checksum ($published)" }
            Log "${k}: SHA-256 matches the lock and the published $name.sha256"
        }
        $entries = & $Tar -tzf $file
        if ($LASTEXITCODE -ne 0) { Fail "${name}: cannot list the archive" }
        if ($entries | Where-Object { $_ -match '^(/|[A-Za-z]:)|(^|/)\.\.(/|$)' }) { Fail "unsafe path in $name" }
        $x = Join-Path $work "x"
        if (Test-Path $x) { Remove-Item -Recurse -Force $x }
        New-Item -ItemType Directory $x | Out-Null
        & $Tar -xzf $file -C $x
        if ($LASTEXITCODE -ne 0) { Fail "${name}: cannot unpack the archive" }
        $src = $x
        $tops = @(Get-ChildItem $x)
        if ($tops.Count -eq 1 -and $tops[0].PSIsContainer -and -not (Test-Path (Join-Path $x "include"))) { $src = $tops[0].FullName }
        if (-not (Test-Path (Join-Path $src "include\daas.hpp"))) { Fail "${k}: include/daas.hpp missing: not a libdaas package" }
        $vfile = Join-Path $src "VERSION.txt"
        if (Test-Path $vfile) {
            $pkg = (Get-Content -Raw $vfile).Trim()
            if ($pkg -and $pkg -ne $Version) { Fail "${k}: package version $pkg is not the $Version of the lock" }
        }
        $target = Join-Path $Dest $k
        if (Test-Path $target) { Remove-Item -Recurse -Force $target }
        New-Item -ItemType Directory -Force $target | Out-Null
        Copy-Item -Recurse -Force (Join-Path $src "*") $target
        Log "${k}: $name -> libdaas/$k/"
        $records[$k] = "    `"$k`": { `"tarball`": `"$name`", `"sha256`": `"$digest`" }"
    }
    # the other packages of the lock still installed from this version stay in the record
    if (Test-Path $Stamp) {
        $old = Get-Content -Raw -Encoding UTF8 $Stamp | ConvertFrom-Json
        if ("$($old.version)" -eq $Version -and $old.packages) {
            foreach ($k in @($all.Keys)) {
                if ($records.Contains($k) -or -not $old.packages.$k) { continue }
                $name = $all[$k].name
                if ($old.packages.$k.tarball -ne $name -or -not (Test-Path (Join-Path $Dest "$k\include\daas.hpp"))) { continue }
                $records[$k] = "    `"$k`": { `"tarball`": `"$name`", `"sha256`": `"$($all[$k].sha256)`" }"
            }
        }
    }
    $body = "{`n  `"version`": `"$Version`",`n  `"source`": `"$Catalog`",`n  `"packages`": {`n" + (($records.Values) -join ",`n") + "`n  }`n}`n"
    [IO.File]::WriteAllText($Stamp, $body, (New-Object Text.UTF8Encoding $false))   # no BOM, LF: as fetch_ld.sh
    Log "done"
} finally {
    Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
}
