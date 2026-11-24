<!-- daas © 2019–2026 Sebyone Srl -->
<!-- Contributors: plogiacco@sebyone.it -->

# fetch_ld: download libdaas

## What it is for

android_daas wraps **libdaas**, the precompiled daas library: the core, its drivers and its header files. libdaas is
not stored in this repository, and its **only source is the public catalog of daasiot.com**
(<https://daasiot.com/api/libdaas.php>). Before anything is built from a clone of the repository (android_daas, the
persistence, the examples, the drivers), libdaas must be downloaded from the catalog and installed. The tool of
this folder does that, as `tools/fetch_ld/` of daasiot-esp32.

## What it does

`fetch_ld` installs **exactly the packages written in the lock** `android_daas/libdaas.lock.json`. The lock holds the
libdaas `version` and, under `packages`, one entry per `<target>/<linkage>` with the file name of the package in the
catalog (`tarball`) and its SHA-256 (`sha256`):

```json
{
  "version": "0.24.8",
  "packages": {
    "android-arm64-v8a/static": {
      "tarball": "libdaas-0.24.11-android-arm64-v8a-clang_arm64-v8a-static.tar.gz",
      "sha256": "2e1c2099f5fb16b438bd11ff6e8473edf4d6ca561858b3a29e6adf790f128c24"
    }
  }
}
```

For each entry (or the ones given with `--only`):

1. **verifies the file in the official source**, before downloading anything: the name must be listed in the catalog
   as available, **and** the `download_url` of its entry (`https://daasiot.com/api/download-core.php?file=<file>`)
   must serve a `.tar.gz` (only the first two bytes are asked). If one package is missing nothing is installed, and
   the reason is printed: not in the catalog, marked not available, or listed but not served (e.g. HTTP 404);
2. downloads it and checks it: SHA-256 equal to the lock and to the checksum file `<file>.sha256` the catalog
   publishes; safe paths, `VERSION.txt` equal to the `version` of the lock, `include/daas.hpp` present;
3. unpacks it as it is into `libdaas/<target>/<linkage>/` and writes `libdaas/INSTALLED.json`.

The tool has no table of targets: any `<target>/<linkage>` of the lock is installed, under that name. The
`<target>` is the folder `android_daas/libdaas.cmake` looks for (`DAAS_TARGET`, `android-<ABI>`):
`android-arm64-v8a` today; Android packages are `static` only.
`libdaas/` is gitignored.

## The tools

| File | Runs on | Needs |
|---|---|---|
| `fetch_ld.sh` | Linux, macOS, Git Bash, MSYS2, WSL, a Raspberry Pi, a container | bash, curl or wget, tar, od, sha256sum or shasum |
| `fetch_ld.ps1` | Windows (PowerShell 5.1 or 7) | `tar.exe` (Windows 10 1803 or later): `powershell -ExecutionPolicy Bypass -File tools\fetch_ld\fetch_ld.ps1` |

| `fetch_ld.sh` | `fetch_ld.ps1` | |
|---|---|---|
| (none) | (none) | verify, download and install every package of the lock |
| `--only <target>/<linkage>` | `-Only <target>/<linkage>` | only that package of the lock (repeatable; `.ps1`: `-Only a,b`) |
| `--verify` | `-Verify` | check that the catalog has the packages of the lock and serves them, and that the SHA-256 of the lock is the published one (a missing `sha256` is printed, to be written in the lock); nothing is installed; exit code 1 if not ok |
| `--check` | `-Check` | check the installed files and `INSTALLED.json` against the lock, offline (exit code 1 if not ok) |
| `--list` | `-List` | list the libdaas packages of the catalog |
| `-h`, `-help`, `--help` | also `-Help`, `/?` | the summary of the commands |

Every run starts with a heading: the version of the tool, the release of daasiot-android (`VERSION`), the holder and the
copyright. The two files are one tool with one version (`FETCH_LD_VERSION` in `.sh`, `$FetchLdVersion` in `.ps1`,
M.m.p): raise it at every change of the code of either (p a fix, m a new option or behaviour, M an incompatible
change) and add a line to the Changes of the file header; `tests/auto/t12` checks that the two agree.

```sh
tools/fetch_ld/fetch_ld.sh --verify                 # does the official source have the packages of the lock?
tools/fetch_ld/fetch_ld.sh                          # install them, from the root of the repository
tools/fetch_ld/fetch_ld.sh --only android-arm64-v8a/static
tools/fetch_ld/fetch_ld.sh --check
```

```powershell
powershell -ExecutionPolicy Bypass -File tools\fetch_ld\fetch_ld.ps1 -Only android-arm64-v8a/static
```

## Change the lock

A new target, another package or another libdaas version is a change of `android_daas/libdaas.lock.json`, by hand:

1. `fetch_ld.sh --list`: the name of the package in the catalog;
2. add (or change) its entry, `"<target>/<linkage>": { "tarball": "<file>", "sha256": "" }` (for another version,
   also `version` and every `tarball`);
3. `fetch_ld.sh --verify --only <target>/<linkage>` prints the published SHA-256: write it in `sha256`, run
   `--verify` again;
4. `fetch_ld.sh --only <target>/<linkage>` installs it; then
   `python platforms/scripts/write_manifest.py --kind repo` updates the `MANIFEST.json` of the repository.

A target that daasiot-android does not build yet also needs its build environment (see the README of the repository).

## Download manually

1. Download the packages of the lock from the catalog of <https://daasiot.com> (the `tarball` of each entry).
2. Check their SHA-256 against the lock (`sha256sum <file>`).
3. Unpack each one into `libdaas/<target>/<linkage>/` (`tar xzf <file> -C libdaas/android-arm64-v8a/static`), or pass
   the folder to CMake with `-DDAAS_ROOT=<folder>`.
