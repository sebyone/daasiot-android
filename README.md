# daasiot-android

daas (Device as a Service) for Android (NDK, arm64-v8a). Version: `VERSION`.

It packages, for an Android app:

- **libdaas** 0.24.11 (daas protocol 6.1.0), static package for android arm64-v8a, from the DaaS Build Console
  (`libdaas/`, see `libdaas/INSTALLED.json`), with the generic drivers INET4 and INET6.
- **android-drivers_sdk/ble**: the BLE driver of the `ble` slot of `_LINK_BLE` (driver ABI 2), on
  [blew](android-drivers_sdk/third_party/blew) 1.1.0. The libdaas packages leave the slot empty
  (`libdaas_nodrivers.a`): the BLE driver lives in the wrappers (same driver as daasiot-macos / daasiot-esp32).

## Layout

```
CMakeLists.txt                       daas::libdaas, daas::drv_ble
VERSION
libdaas/
  INSTALLED.json                     package and sha256
  android-arm64-v8a/static/          the libdaas package as released (include/, lib/, drivers_sdk/, ...)
android-drivers_sdk/
  CMakeLists.txt
  android_drivers_sdk.hpp            ANDROID_BUILD, USE_BLE
  include/daasiot_android/ble.h      public API of the BLE driver (daas_ble_set_local_sid)
  ble/                               drv_ble.cpp/.hpp, local_ble.hpp (uri_ble_t), ble_version.hpp
  third_party/blew/                  include/, lib/<abi>/libblew.a, java/ (it.sebyone.blew)
```

## Use in an app

`app/src/main/cpp/CMakeLists.txt`:

```cmake
add_subdirectory(${DAASIOT_ANDROID_ROOT} daasiot-android)
add_library(daas_jni SHARED daas_jni.cpp)
target_link_libraries(daas_jni PRIVATE daas::libdaas)   # libdaas, BLE driver + blew, INET4/INET6, nodrivers
```

`app/build.gradle.kts` (Java side of blew):

```kotlin
android {
    sourceSets["main"].java.srcDir("<path>/daasiot-android/android-drivers_sdk/third_party/blew/java")
}
```

At run time:

1. `System.loadLibrary("daas_jni")`, then `it.sebyone.blew.Blew.init(context)` before enabling `_LINK_BLE`;
2. after `doInit()` / `loadConfiguration()`, `daas_ble_set_local_sid(sid)` (`daasiot_android/ble.h`): the SID
   blew publishes for the discovery filters of the other nodes;
3. `enableDriver(_LINK_BLE, "<node id>")`, where the node id is 8 hexadecimal digits (from the DIN).

CMake options: `DAASIOT_ANDROID_BLE` (ON), `DAASIOT_ANDROID_DRIVERS` (`inet4;inet6`).

## Requirements

Android API 30 or later, Android NDK r28c (clang), C++17.

## libdaas is not in git

`libdaas/android-<abi>/static/` is gitignored: the package is pinned in `libdaas/INSTALLED.json` (version, tarball,
SHA-256). Unpack that tarball there, as is, before building:

```bash
mkdir -p libdaas/android-arm64-v8a/static
tar -xzf libdaas-0.24.11-android-arm64-v8a-clang_arm64-v8a-static.tar.gz -C libdaas/android-arm64-v8a/static
```

To update libdaas, unpack the new package there and update `libdaas/INSTALLED.json`. The BLE driver is rebuilt
against the `drivers_sdk/include` of that package.
