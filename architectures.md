# daasiot-android — Supported Architectures

Kotlin/JNI Android SDK (`daasiotsdk`) for the DaaS-IoT SDK.
DaaS-IoT core `v0.22.0` · Android API 30+ · Kotlin 2.2+ · MPL-2.0.

> This document is the human-readable view of [`architectures.json`](architectures.json),
> which is the machine-readable source of truth.

## Status legend

| Status | Meaning |
|--------|---------|
| ✅ **supported** | Built and validated; recommended for production. |
| 🧪 **experimental** | Prebuilt native present but not enabled/validated in the current build. |
| 🗺️ **planned** | On the roadmap, not available yet. |

## ABI matrix

| ABI | Status | `libdaas.a` | Notes |
|-----|:------:|-------------|-------|
| `arm64-v8a` | ✅ | ~0.22.0 (+ `libsimpleble.a`) | The only ABI enabled in the Gradle build; `assembleRelease` verified. |
| `armeabi-v7a` | 🧪 | 0.20.1 | Prebuilt present but excluded from the build and out of sync with the headers; no SimpleBLE. |
| `x86_64` | 🧪 | 0.20.1 | Prebuilt present but excluded (mainly for emulators); no SimpleBLE. |

> Only `arm64-v8a` is compiled today (`abiFilters` in `daasiotsdk/build.gradle.kts`).
> Re-enabling the other ABIs requires rebuilding their `libdaas.a` (and sourcing a
> matching `libsimpleble.a`) from the `daas` core repo — see
> `daasiotsdk/src/main/jniLibs/PROVENANCE.md`.

## Requirements

- **Android:** minSdk **30**, compileSdk 36
- **Kotlin:** 2.2+
- **JVM target:** 11
- **Build:** Gradle (Android Gradle Plugin), CMake 3.22.1 for the JNI layer

## Runtime notes

- Add the **INTERNET** permission for IPv4 networking.
- Add the relevant **Bluetooth** permissions if the BLE driver is used
  (BLE is provided by SimpleBLE, bundled for `arm64-v8a`).

See [`install.md`](install.md) for full installation instructions.
