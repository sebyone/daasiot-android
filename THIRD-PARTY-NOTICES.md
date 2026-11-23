<!-- daas © 2019–2026 Sebyone Srl -->

# Third-party notices

daasiot-android is distributed under the Mozilla Public License 2.0 ([LICENSE](LICENSE)). It contains or uses:

| Component | Holder | License | Where |
|---|---|---|---|
| **libdaas** (daas-api) | Sebyone Srl | MPL-2.0 | `libdaas/<target>/<linkage>/` (installed by `tools/fetch_ld/fetch_ld.sh` from the daasiot.com catalog) |
| **blew** (ble-wrapper) 1.1.0 | Sebyone Srl | MPL-2.0 | `android-drivers_sdk/third_party/blew/` (prebuilt `libblew.a`, headers and Java sources, used by the BLE driver `android-drivers_sdk/ble/`) |

The BLE driver uses the Bluetooth APIs of the Android platform (`android.bluetooth`), reached through blew; they are
part of the system and are not redistributed. The native code is built with the Android NDK and its LLVM libc++
(Apache-2.0 with LLVM exceptions), linked statically into `libandroid_daas_jni.so`.

Android is a trademark of Google LLC. Bluetooth is a trademark of the Bluetooth SIG, Inc.
