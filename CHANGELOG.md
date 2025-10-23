# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.0.1] - 2025-10-24

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