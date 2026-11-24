<!-- daas © 2019–2026 Sebyone Srl -->

# Changelog

All notable changes to daasiot-android. The format is based on [Keep a Changelog](https://keepachangelog.com/), and
the project uses a `MAJOR.MINOR.PATCH` scheme (`VERSION`).

## [0.2.0] - 2026-11-24

The conventions of the daasiot-&lt;target&gt; repositories (`daasiot-esp32/docs/developers/repo-target-and-tarballs-
conventions.md`), on libdaas **0.24.12**, built and published by the DaaS Build Console (SDK 6 of daasiot.com).

### Changed (24/11/2026)
- **No namespaces in android_daas**: `Node`, `NodeConfig`, `EventRouter`, `file_depot`, ... are global, like the API
  of libdaas; the internal helpers are `portable_<name>`. `daas_ble_set_local_sid()` has a weak default.
- **libdaas 0.24.12** for **three ABIs** (arm64-v8a, armeabi-v7a, x86_64; drivers INET4 and INET6 1.0.1), installed
  in `android_daas/libdaas/` (fetch_ld 2.0.1); `android_daas/libdaas.cmake` (was `platforms/libdaas/libdaas.cmake`,
  never committed) links libdaas and the drivers_kit.
- **android-drivers_sdk renamed android-drivers-sdk**; **blew precompiled by the DaaS Build Console** (`ext/blew/
  android_<arch>`, blew 1.1.0 with ble-wrapper `4a1ec22`, SHA-256 in the drivers lock): the sources and the prebuilt
  `libblew.a` of `third_party/blew/` and `android_drivers_sdk.hpp` are removed (the timings of blew in
  `ble/ble_config.hpp`). `build_drivers_kit.py` builds the drivers_kit of every ABI with the NDK into
  `android_daas/drivers_kit/` (`libdaas_drv_ble.a`, `libblew.a`, the Java side of blew in `java/`); `template/`.
- The Gradle module builds the three ABIs and takes the Java of blew from the drivers_kit.

### Added (24/11/2026)
- `platforms/`: `dist.json`, `scripts/build_aar.py` (the AAR with the NDK, javac and android.jar, no Gradle),
  `scripts/build_dist.py` (packages `aar`, `drivers-sdk-static`, `src` with MANIFEST.json and SHA256SUMS; never the
  sources of the specialized drivers), `daas_builder.sh` and `daas-build.yaml` (DaaS Build Console, SDK 6).
- `tools/docs_pdf/`, the developers guide in the format of the daas documents and its PDF.

### Added (23/11/2026)
- **android_daas**, the component: an Android library module (AAR, or a module of the build of the app) with the Java
  API `it.sebyone.daas` (`DaasNode` and its `Config`, `DaasListener`, `Ddo`, `NodeStatus`, `Link`, `NodeState`,
  `AcceptRequests`, `DiscoveryState`, `DdoPolicy`, `DaasError`), its JNI binding and the C++17 wrapper
  `daas::sdk::Node` (port of `macos_daas` of daasiot-macos 0.1.0: configuration, `EventRouter` on the service thread,
  persistence through `file_depot` in the files folder of the app, restore, autosave, DDO helpers). The local SID is
  handed to the BLE driver after `doInit()`, after a restore and when a join completes.
- `platforms/libdaas/libdaas.cmake` (from daasiot-gnu): `libdaas::libdaas` with the drivers in the order of the package
  (libdaas, `DAAS_EXTRA_DRIVERS`, the generic drivers, libdaas_nodrivers, libdaas); targets `android-<ABI>`.
- `android_daas/libdaas.lock.json` and `tools/fetch_ld/` (`fetch_ld.sh`, `fetch_ld.ps1` 2.0.0 of daasiot-gnu).
- `android-drivers_sdk/libdaas-drivers.lock.json` and README files.
- `examples/simple_node`, an app with a daas node (BLE + INET4).
- The Gradle build of the repository (AGP 9.0, Gradle 9.4.1), `docs/shared/` (developers guide), `CLAUDE.md`,
  `LICENSE`, `THIRD-PARTY-NOTICES.md`.

### Changed
- BLE driver 1.1.0: `open_sub` reuses a link already open with the node (blew opened a second connection in the
  other direction, never accepted while the first one lived: open timeout, then both links dropped); every timing
  of blew from `android_drivers_sdk.hpp`: scan 3 s (`DAAS_BLE_SCAN_TIMEOUT_MS`, blew 5 s), open 5 s, ping 1.5 s
  (`DAAS_BLE_PING_INTERVAL_MS`, blew 5 s), link timeout 4 s (`DAAS_BLE_LINK_TIMEOUT_MS`, blew 10 s).
- blew (Java side), from ble-wrapper `4a1ec22`: the GATT client asks `CONNECTION_PRIORITY_HIGH` when it connects,
  11.25–15 ms instead of the ~39 ms of the default interval: MTU, service discovery and the daas negotiation of a
  link take less.
- libdaas out of git: `libdaas/` is the install folder of `tools/fetch_ld` (was `libdaas/INSTALLED.json` + a package
  copied by hand). The root CMake project builds android_daas and android_daas_jni; `daas::libdaas` of 0.1.0 is
  replaced by `libdaas::libdaas` and `daas::android_daas`.

### Notes
- The SHA-256 of the lock is the one of the package of the DaaS Build Console (daas-api `a461c68`): check it with
  `fetch_ld.sh --verify` when the catalog publishes the Android packages.
- libdaas 0.24.11 crashes in `loadConfiguration()` when the saved configuration has a removed channel before a mapped
  one (`channel_pool_manager_t::context_exists`): fixed in daas-api `threading-mode` (`5dcd20c8`). Until a package with
  the fix is in the catalog, build with that package (`DAAS_ROOT` or `libdaas/`).

## [0.1.0] - 2026-11-23

First version, on libdaas **0.24.11** (daas protocol 6.1.0), static package for android arm64-v8a.

### Added
- `daas::libdaas`: libdaas static with the drivers in the order of the package (libdaas, BLE driver + blew, INET4,
  INET6, libdaas_nodrivers).
- **android-drivers_sdk/ble**: the BLE driver of the `ble` slot of `_LINK_BLE` (1.0.0) on blew 1.1.0, from the driver
  of daasiot-macos 0.1.0; `daasiot_android/ble.h` with `daas_ble_set_local_sid()`.
- blew 1.1.0 for android arm64-v8a (`libblew.a`, headers, Java sources `it.sebyone.blew`).
