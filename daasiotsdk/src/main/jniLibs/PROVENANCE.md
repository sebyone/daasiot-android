# Provenance of the bundled `libdaas.a` / `libsimpleble.a`

Every `<abi>/libdaas.a` in this directory is a prebuilt static archive of the native **daas**
(`libdaas`) C++ core library, cross-compiled for Android. It is **not** built from source as part
of this repository's Gradle/CMake build - it is checked in as a binary, produced by the `daas`
core source repository's own Android build. `arm64-v8a` additionally bundles `libsimpleble.a`
(see "Where `libsimpleble.a` came from" below).

This file exists because, before it was written, there was no way to tell from this repo alone
which `libdaas` version/commit any given `.a` came from, or whether the three ABI variants were
even built from the same version as each other or as the headers in `../include/`. That gap is
exactly what caused a real build failure earlier in this project's history: the `arm64-v8a` header
was updated to a newer `libdaas` revision without updating `arm64-v8a/libdaas.a` to match, and
separately, updating `libdaas.a` alone (without every dependency it now requires) surfaced a
missing `SimpleBLE` link dependency that had been silently unbuildable all along (resolved - see
table below). Keep the table below current whenever any of these files change, so the next person
(or the next you) doesn't have to rediscover this by trial and error.

## Current state (fill in / update whenever a file here changes)

| ABI | `libdaas.a` version | Header version (`../include/`) | Last updated | Known issues |
|---|---|---|---|---|
| `arm64-v8a` | ~0.22.0 (unconfirmed exact source commit; byte-identical to the `libdaas.a` bundled in the sibling `thermobooster_mobile_app` project, which uses the same header pair) | ~0.22.0 | `libdaas.a` updated directly in this repo (exact date/commit not recorded); `libsimpleble.a` added 2026-07-04 (copied from `thermobooster_mobile_app`) | None currently open - the build links and produces a working AAR end to end (`assembleRelease` verified 2026-07-04). |
| `armeabi-v7a` | 0.20.1 (matches the version this SDK originally shipped with; not updated since) | Same header is now shared across all ABIs (~0.22.0) - **this ABI's `.a` and the header are almost certainly out of sync** | project inception | Not currently built by Gradle (`abiFilters` in `daasiotsdk/build.gradle.kts` only includes `arm64-v8a`), so this mismatch hasn't surfaced as a build error - but it will if this ABI is ever re-enabled without also rebuilding this `.a`. Also has no `libsimpleble.a` of its own. |
| `x86_64` | 0.20.1 (same as above) | Same as above | project inception | Same as above. |

## Where this comes from: the `daas` source repository

`libdaas.a` is built from the `daas` core C++ source repository (the same codebase that ships
Linux/macOS/Windows/ESP32/etc. builds too - this SDK is one consumer among several). That
repository has a working Android build already set up:

- `platforms/android/armv8/CMakeLists.txt` → `arm64-v8a`
- `platforms/android/armv7/CMakeLists.txt` → `armeabi-v7a`
- `platforms/android/x86_64/CMakeLists.txt` → `x86_64`
- `platforms/android/common_android.cmake` → shared Android toolchain config for all three

### Toolchain requirements (as configured in `common_android.cmake`)

- Built via CMake cross-compiling with the **Android NDK's clang**, targeting `ANDROID_PLATFORM
  30` (matches this SDK's `minSdk`).
- The toolchain paths in `common_android.cmake` are currently **hardcoded to a WSL path**
  (`/mnt/c/android-ndk/...`), meaning today this build is only known to run inside WSL/Linux with
  the NDK available at that exact mount point - not from a native Windows shell. If that path
  doesn't exist on the machine building it, update `common_android.cmake` to point at the actual
  NDK location before building.
- Static compilation is forced (`STATIC_COMPILATION ON`), output file name is `libdaas.a`.

### Build command

From the root of the `daas` source repository, inside WSL:

```bash
platforms/build_daas_linux.sh android        # build all three Android ABIs
platforms/build_daas_linux.sh -c android      # clean rebuild
```

This walks every `CMakeLists.txt` under `platforms/android/` and runs `cmake . && make` in each
one (in-source build). Each ABI's build writes its `libdaas.a` and matching headers here:

```
output/android/armv8/lib/libdaas.a    ->  daasiotsdk/src/main/jniLibs/arm64-v8a/libdaas.a
output/android/armv7/lib/libdaas.a    ->  daasiotsdk/src/main/jniLibs/armeabi-v7a/libdaas.a
output/android/x86_64/lib/libdaas.a   ->  daasiotsdk/src/main/jniLibs/x86_64/libdaas.a
output/include/daas.hpp               ->  daasiotsdk/src/main/include/daas.hpp
output/include/daas_types.hpp         ->  daasiotsdk/src/main/include/daas_types.hpp
```

**All three ABIs and the headers must come from the same build/version.** Copy them together, not
one at a time - see the incident described at the top of this file for what happens otherwise.

### Release packaging convention

The `daas` repository's `release/` directory holds versioned archives named
`libdaas-<version>-Android-.tar.gz`, containing `include/` (the two headers), `lib/` (meant to
hold the built `.a`, per ABI), `docs/reference.pdf`, and `changelog.md`. Producing one of these for
whichever version is copied into this SDK - even just as a local artifact kept alongside this
repo's release notes - is the most reliable way to keep the version table above honest, since the
archive's filename records the version and its contents are exactly what was copied.

## Where `libsimpleble.a` came from

Since `libdaas.a` (0.22.0-era) now statically links a BLE driver (`drv_ble.cpp`) that depends on
[SimpleBLE](https://github.com/OpenBluetoothToolbox/SimpleBLE) (both the `SimpleBLE::*` client API
and the `simpleble::Server` peripheral-mode API), an Android `arm64-v8a` build of SimpleBLE is
required at link time too. The local `daas` source repo checkout available at the time did not
yet contain `drv_ble.cpp` (it's an older, pre-BLE revision), so it couldn't be used to build or
locate this dependency.

`daasiotsdk/src/main/jniLibs/arm64-v8a/libsimpleble.a` was instead copied from the sibling
`thermobooster_mobile_app` Android project (`app/src/main/jniLibs/arm64-v8a/libsimpleble.a`),
which already links `libdaas.a` + `libsimpleble.a` together for the same target (see its
`app/src/main/cpp/CMakeLists.txt`). Before copying, its `libdaas.a` and header pair were verified
to be either byte-identical (`libdaas.a`, `daas.hpp`) or identical modulo line endings and one
unrelated typo in an enum name (`daas_types.hpp`) to the ones already in this repo, confirming
it's a consistent, matching build rather than a different revision that happened to compile.

`armeabi-v7a`/`x86_64` have no SimpleBLE library of their own yet - irrelevant today since only
`arm64-v8a` is built (see the ABI table above), but a blocker if either is ever re-enabled without
also sourcing a matching `libsimpleble.a` for that ABI.

### Updating the files in this directory

Use `scripts/update-native-lib.ps1` at the repository root (see that script's header comment for
usage) to copy a freshly built `libdaas.a` + headers from a local `daas` repo checkout into this
project consistently across all three ABIs and the include directory, and to remind you to update
the table above.
