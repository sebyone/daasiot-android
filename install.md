# daasiot-android — Installation Guide

Kotlin/JNI Android SDK for DaaS-IoT.
AAR `daasiotsdk` `0.2.0` · DaaS-IoT core `v0.22.0` · Android API 30+.

The SDK ships as an **Android library (AAR)** with the JNI native library and the
prebuilt `libdaas.a` bundled inside — no separate native install.

Two ways to consume it:

1. [**Local AAR**](#1-local-aar-current) — the supported method today.
2. [**Build from source**](#2-build-the-aar-from-source) — produce the AAR yourself.

> The SDK is **not published to Maven Central yet**. Maven coordinates are planned
> (see `PUBLISHING.md`); for now use the local AAR.

---

## Requirements

| | |
|---|---|
| **Android** | minSdk **30**, compileSdk 36 |
| **Kotlin** | 2.2+ |
| **ABI** | `arm64-v8a` (see [`architectures.md`](architectures.md)) |
| **Build (from source)** | Android Gradle Plugin + NDK, CMake 3.22.1 |

---

## 1. Local AAR (current)

1. Copy the AAR into your app module's `libs/` folder:

   ```
   app/libs/daasiotsdk-0.2.0-release.aar
   ```

2. Declare the dependency in your app's `build.gradle.kts`:

   ```kotlin
   dependencies {
       implementation(files("libs/daasiotsdk-0.2.0-release.aar"))
   }
   ```

3. Target `arm64-v8a` (the only ABI shipped in the AAR). For local/CI builds you
   can restrict the ABI in your app module:

   ```kotlin
   android {
       defaultConfig {
           ndk { abiFilters += "arm64-v8a" }
       }
   }
   ```

4. Add the permissions your drivers need to `AndroidManifest.xml`:

   ```xml
   <uses-permission android:name="android.permission.INTERNET" />
   <!-- Only if you use the BLE driver: -->
   <uses-permission android:name="android.permission.BLUETOOTH_SCAN" />
   <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
   ```

No ProGuard/R8 setup is required — consumer rules are bundled in the AAR.

---

## 2. Build the AAR from source

Requires the Android SDK + NDK and (for the JNI layer) CMake 3.22.1.

```bash
git clone <repo-url> daasiot-android
cd daasiot-android

# assemble the release AAR
./gradlew :daasiotsdk:assembleRelease      # Windows: .\gradlew.bat :daasiotsdk:assembleRelease
```

The AAR is produced (and auto-renamed) at:

```
daasiotsdk/build/outputs/aar/daasiotsdk-0.2.0-release.aar
```

Publish it to your local Maven repo for multi-module use:

```bash
./gradlew :daasiotsdk:publishToMavenLocal   # if/when the publishing plugin is enabled
```

### Rebuilding the native `libdaas.a`

The AAR bundles a prebuilt `libdaas.a` per ABI (checked into
`daasiotsdk/src/main/jniLibs/`). It is built from the separate `daas` core repo,
not from this project. To refresh it, use `scripts/update-native-lib.ps1` and see
`daasiotsdk/src/main/jniLibs/PROVENANCE.md` for the toolchain and versioning
rules (all ABIs + headers must come from the same build).

---

## Verify

```kotlin
import sebyone.daasiot_android.DaasInstance
import sebyone.daasiot_android.DaasWrapper

DaasInstance.create().use { node ->
    android.util.Log.i("DaaS", "version: " + DaasWrapper.getVersion(node.handle))
}
```

If the version string logs, the JNI library loaded correctly. Run the
`daasiot-android-demo` module for an interactive check.

---

## Troubleshooting

| Symptom | Cause / fix |
|---------|-------------|
| `UnsatisfiedLinkError: dlopen failed ... daas_jni` | The device/emulator ABI isn't `arm64-v8a`. Use a 64-bit ARM device (or an arm64 emulator image). |
| `INSTALL_FAILED_NO_MATCHING_ABIS` | Same cause — install on `arm64-v8a`. |
| JNI `NoSuchMethodError` / classes missing in release | You stripped the SDK with a custom R8 config — keep the bundled consumer rules (don't override them). |
| Networking calls fail silently | Missing `INTERNET` permission (or Bluetooth permissions for BLE). |
| `requestFeature` returns `ERROR_NOT_IMPLEMENTED` | Expected — not an exported symbol in the current `libdaas.a`. |
| minSdk error | The library requires API level 30+. |

---

For the API surface, see [`reference.md`](reference.md).
For releasing/publishing, see `RELEASING.md` and `PUBLISHING.md`.

*© Sebyone Srl — Mozilla Public License 2.0*
