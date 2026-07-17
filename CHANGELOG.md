# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.2.3] - 2026-07-17

## libdaas Version 0.22.0

### Changed
- Updated ATS maximum-error handling and BLE support.
- Updated the native event bridge implementation.
- Updated installation, API reference, and architecture documentation.
- Prepared Android SDK publication metadata for version `0.2.3`.

---

## [0.2.0] - 2026-07-03

## libdaas Version 0.22.0

### Breaking Changes
- `DaasWrapper` no longer wraps a single global `DaasAPI` instance: `create()` now allocates and
  returns a handle to its own independent native instance, so multiple `DaasWrapper`-backed
  instances can coexist without clobbering each other's state. `doInit` and `reset` (previously
  instance-less) now take the instance `ptr` as their first parameter, like every other method.
- Every method that previously returned a raw `Int` error code (`doInit`, `doEnd`, `reset`,
  `doPerform`, `enableDriver`, `map`, `remove`, `locate`, `sendStatus`, `addTypeset`, `frisbee`,
  `push`) now returns a typed `DaasError`.
- `enableDriver`, `map` and `doPerform` now take `LinkType`/`PerformMode` enums instead of raw
  `Int` codes.
- `getStatus`, `status`, `fetch`, `syncNode`, `syncNet` now return a typed `NodeInfo` instead of a
  raw native pointer encoded as a `Long`.
- `listNodes` now returns `Array<NodeNetworkInfo>` instead of a raw pointer.
- `pull` now returns a `PullResult(error, ddo)`, and `availablesPull` an
  `AvailablePullResult(error, count)`, instead of raw pointers / a mismatched `IntArray`.
- `push` now takes a `DDO` instance instead of a raw `ddoPtr: Long`. `DDO` is a new public class
  wrapping DDO creation, payload read/write, and disposal.
- `receive` no longer takes a caller-provided buffer; it returns a freshly-sized `ByteArray`,
  matching the updated native `receive(din_t, uint8_t*&)` signature.
- `addTypeset` no longer takes a `typesetSize`: the native function now expects a callback pointer
  that Kotlin code has no way to supply, so this only registers the typeset code (received DDOs
  for it still surface through `ddoReceived`).
- `din` parameters are now consistently `Long` (matching the 64-bit `din_t`), instead of an
  inconsistent mix of `Int`/`Long` across different methods.
- Updated to libdaas 0.22.0 headers (from 0.20.1): new `node_info_t` layout, new `link_t` values
  (LoRa, BLE, Zigbee), `IDaasApiEvent::nodeDiscovered` renamed/reshaped to `networkDiscovered`, new
  `streamInfoReceived` event.

### Added
- `IDaasApiEvent`: a real, per-instance event listener interface, passed to `create()`. Replaces
  the previous single hardcoded native listener, which only logged to LogCat and had one broken
  callback attempt (`onDDOReceived`) that could never fire because the Kotlin method it looked up
  never existed.
- `NativeCleaner`: a `PhantomReference`-based safety net (portable back to this module's
  `minSdk 30`, unlike `java.lang.ref.Cleaner` which needs API 33+) that disposes native memory if a
  `DDO` or `DaasInstance` is garbage collected without being closed explicitly. Not a substitute for
  calling `close()`: GC timing is unspecified, so it only bounds a forgotten leak rather than
  preventing it.
- `DaasInstance`: an optional `Closeable` wrapper around the `ptr` returned by `create()`, for
  callers who want the `NativeCleaner` safety net without changing how they call `DaasWrapper`.
- Typed enums for every native enum currently exposed through the API: `DaasError`, `LinkType`,
  `PerformMode`, `AcceptRequestPolicy`, `DDOPolicy`, `DiscoveryState`, `StreamType`.
- Native implementations for `doEnd`, `doStatisticsReset`, `getSystemStatistics`, `remove`,
  `locate`, `sendStatus`, `getSyncedTimestamp`, `use`, `end`, `send`, `received`, `receive`,
  `addTypeset`, `frisbee`, `getStatus`, `listNodes`, `status`, `fetch`, `syncNode`, `syncNet`,
  `pull`, `push`, `availablesPull` — previously declared in `DaasWrapper` but never implemented
  natively, so calling any of them threw `UnsatisfiedLinkError`.
- New API surface unlocked by the libdaas 0.22.0 headers, not previously exposed at all:
  `discovery` (by link or by SID), `join`, `setDiscoveryState`, `getNodeList`, `getAllNodes`,
  `locate` (by `Feature`), `setDDOPolicy`, `createNetwork`, `unbindNetwork`, `setOptions`,
  `errorToString`, `addNodeFeatures`, `getNodeFeatures`, `nodeHasFeature`, `requestFeature`,
  `setAcceptRequestsLevel` — with new supporting types `Feature`, `FeatureAction`, `OptionType`,
  `NodeMapEntry`, `FeatureRequest`.
- Real unit and instrumented tests, replacing the default template's placeholder tests: enum
  round-trip coverage for every `NativeEnum`, `NodeInfo`/`FeatureRequest` equals/hashCode
  correctness for their `ByteArray` fields, and instrumented JNI lifecycle/multi-instance tests.
- `daasiotsdk/src/main/jniLibs/PROVENANCE.md` and `scripts/update-native-lib.ps1`, documenting and
  partially automating where `libdaas.a`/`libsimpleble.a` come from and how to update them.

### Fixed
- `CMakeLists.txt` referenced a `daas_wrapper.cpp` source file that did not exist anywhere in the
  repository, breaking the native build outright.
- Numerous native method name/signature mismatches between `DaasWrapper`'s `external fun`
  declarations and the actual JNI exports (e.g. `nativeCreate` returning `void` instead of the
  `Long` handle Kotlin expected, `nativeMap` reading 2 native parameters against 5 declared in
  Kotlin, `doPerform`'s native symbol literally named differently from what Kotlin called it).
- The prebuilt `libdaas.a` for `arm64-v8a` statically links a BLE driver that depends on
  [SimpleBLE](https://github.com/OpenBluetoothToolbox/SimpleBLE), which was missing from this
  project entirely and made the native build fail at the link step. Added
  `jniLibs/arm64-v8a/libsimpleble.a` (sourced from the sibling `thermobooster_mobile_app` project,
  which links the same `libdaas.a` + a matching `libsimpleble.a` already - see PROVENANCE.md) and
  linked it in `CMakeLists.txt`. `assembleRelease` now builds a working AAR end to end.
- Removed unused Activity-template cruft left over from this module's project-creation wizard:
  launcher icons, Material Components themes/colors, an accidentally committed screenshot, and the
  `appcompat`/`material`/`constraintlayout`/`core-ktx`/`viewBinding` dependencies they needed - none
  of it applies to a library with no UI.
- Split the ~1150-line monolithic `daas_jni.cpp` into `control/`, `events/`, and `transfer/` source
  files plus a shared `include/` header, mirroring daasiot-java's native layer organization.

### Known issues
- `storeConfiguration`/`loadConfiguration` (an `IDepot` bridge) and typed `SystemCode` remain
  unimplemented; tracked as follow-up work.

---

## [0.1.0] - 2025-02-07

## libdaas Version 0.20.1

### Breaking Changes
- Temporarily removed `lock`, `unlock` and `frisbeeDPerf` wrapper methods from the public API.
  These functions are not available in the current distributed `libdaas.a` build, causing
  unresolved linker symbols. Projects using version `0.0.1` will need to update accordingly.

### Changed
- Simplified native JNI layer to match the available symbols in the underlying library.
- Updated `DaasWrapper` implementation to avoid calls to unavailable functions.
- Cleaned up build warnings and ensured stable linking across architectures.

### Notes
- These methods may be restored in a future release once the upstream library provides stable support.
- This release focuses on ensuring a working and clean Android integration baseline.

## [0.0.1] - 2025-10-24

## libdaas Version 0.20.1

### Added
- Initial release of **DaasWrapper**, providing Kotlin bindings to the native `daas_jni` library.
- Implemented JNI integration with `System.loadLibrary("daas_jni")`.
- Added native bridge methods for:
    - **Lifecycle management**: `create()`, `destroy()`
    - **Information retrieval**: `getVersion()`, `getBuildInfo()`, `listAvailableDrivers()`
    - **Initialization and control**: `doInit()`, `doEnd()`, `doReset()`, `doPerform()`
    - **Driver handling**: `enableDriver()`, `getStatus()`
    - **Configuration management**: `storeConfiguration()`, `loadConfiguration()`, `doStatisticsReset()`, `getSystemStatistics()`
    - **Node management**: `map()`, `remove()`, `listNodes()`, `locate()`
    - **Data operations**: `sendStatus()`, `status()`, `fetch()`, `send()`, `receive()`, `received()`
    - **Synchronization**: `syncNode()`, `syncNet()`, `getSyncedTimestamp()`
    - **Typeset and data transfer**: `listTypesets()`, `pull()`, `push()`, `availablesPull()`, `addTypeset()`
    - **Networking and testing**: `frisbee()`
- Established public Kotlin APIs mirroring the native functions for safe and idiomatic usage.

### Notes
- Some native methods (e.g. `frisbeeICMP`, `frisbeeDPerf`, and `lock/unlock`) are present but currently commented out — to be implemented in future versions.
- Version `0.0.1` marks the foundation for the DaaS IoT Android SDK integration.

---
