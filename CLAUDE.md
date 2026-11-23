# CLAUDE.md

Guidance for Claude Code in **daasiot-android**: daas (Device as a Service) for Android — `android_daas` (Android
library module: Java API `it.sebyone.daas`, JNI binding, C++17 wrapper `daas::sdk::Node`), the android-drivers_sdk
(BLE driver on blew) and the examples. Version in `VERSION` (0.2.0), built on libdaas 0.24.11 pinned in
`android_daas/libdaas.lock.json`. © Sebyone Srl, MPL-2.0. Sibling wrappers: daasiot-gnu (`driver-plg`, the reference
of `platforms/libdaas` and `tools/fetch_ld`), daasiot-macos (the origin of `android_daas`), daasiot-esp32.

## Commands

```sh
tools/fetch_ld/fetch_ld.sh                     # libdaas of the lock into libdaas/ (gitignored); .ps1 on Windows
./gradlew :android_daas:assembleRelease        # the AAR (android_daas/build/outputs/aar/)
./gradlew :simple_node:assembleDebug           # the example: the standard verification of a change
./gradlew :simple_node:installDebug            # on a device (adb), then logcat -s DaasNode android_daas libdaas
./gradlew :android_daas:publishToMavenLocal    # it.sebyone.daas:android_daas:<VERSION>
```

The host is Windows (Git Bash or PowerShell); `local.properties` (gitignored) gives `sdk.dir`. NDK 28.2.13676358,
CMake 3.22.1 of the SDK, JDK 21 (gradle/gradle-daemon-jvm.properties).

## Layout

- `android_daas/` — the component and the Gradle module (`build.gradle.kts`, `AndroidManifest.xml` with the
  permissions of a node, `consumer-rules.pro` for the classes reached from JNI):
  - `java/it/sebyone/daas/` — `DaasNode` (one per process, `Config`, service thread `daas-service`, `dispatch()`
    called by JNI), `DaasListener`, `Ddo`, `NodeStatus`, enums of `daas_types.hpp`;
  - `include/`, `src/` — `android_daas.hpp` (`daas::sdk::Node`), `android_defaults.hpp` (every `ANDROID_DAAS_*`
    default, `-DANDROID_DAAS_USER_CONFIG`), `android_events.hpp` (`EventRouter`), `android_file_depot.hpp`
    (`daas::persistence::file_depot`), `src/android_portable.hpp` (private);
  - `jni/android_daas_jni.cpp` — the binding, `libandroid_daas_jni.so`;
  - `libdaas.lock.json`.
- `android-drivers_sdk/` — specialized drivers built only with `drivers_sdk/include` of the package: `ble/` (one
  `DAAS_DRIVER(ble, ...)` entry), `include/daasiot_android/ble.h`, `third_party/blew/` (prebuilt `libblew.a` per ABI,
  headers, Java sources), `libdaas-drivers.lock.json`.
- `platforms/libdaas/libdaas.cmake` — `libdaas::libdaas`; `DAAS_EXTRA_DRIVERS` (here `daas_drv_ble`) is linked after
  `libdaas.a` and before `libdaas_nodrivers.a`: never link a driver target anywhere else, or CMake moves it after the
  nodriver of its slot (duplicate `daas_static_driver_<id>`).
- `examples/simple_node/` — black box: only `it.sebyone.daas`, `implementation(project(":android_daas"))`.
- `tools/fetch_ld/` — from daasiot-gnu (same `FETCH_LD_VERSION` in `.sh` and `.ps1`).
- `docs/shared/` — developers guide, file names with the version code `_MMMmmmpp` (0.2.0 → `00000200`).
- Generated/gitignored: `libdaas/`, `build/`, `.cxx/`, `.gradle/`, `local.properties`.

## Rules

- **libdaas comes only from the catalog of daasiot.com** via `tools/fetch_ld` (or `DAAS_ROOT`); never commit it.
- The C++ of `android_daas` is a port of `macos_daas`: keep the names, the structure and the comments of the original
  (namespace `daas::sdk` / `daas::persistence`, files `android_*`), and add a Changelog line when you change a file.
- The Java API mirrors `Node`: a new function of the Node gets a `native*` in `DaasNode`, its JNI function and, when
  JNI reaches a class or member by name, a rule in `consumer-rules.pro`.
- Every file starts with the Sebyone header (copyright, `File:`, description, MPL-2.0 notice, Contributors, and a
  Changes/Changelog list); Markdown files start with `<!-- daas © 2019–2026 Sebyone Srl -->`.
- Version bump: `VERSION`, `CHANGELOG.md` (Keep a Changelog), doc file codes. `ANDROID_DAAS_VERSION` and the Maven
  version come from `VERSION`.
- C++17, `-Wall -Wextra` clean. Documentation and comments in English.
