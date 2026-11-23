<!-- daas © 2019–2026 Sebyone Srl -->

# android-drivers_sdk

The specialized drivers of daasiot-android: drivers of libdaas (`idriver_t`, driver ABI 2) built only with the
headers of the driver contract of the libdaas package (`DAAS_ROOT/drivers_sdk/include`), as the `gnu-drivers_sdk` of
daasiot-gnu and the `esp32-drivers_sdk` of daasiot-esp32. Each driver ends with one `DAAS_DRIVER(<id>, ...)` entry,
which defines `daas_static_driver_<id>()`: linked before `libdaas_nodrivers.a` (`DAAS_EXTRA_DRIVERS` of
`platforms/libdaas/libdaas.cmake`), it fills the slot `<id>` of libdaas.

| Driver | Slot | Version | Medium |
|---|---|---|---|
| `ble/` | `ble` (`_LINK_BLE`) | 1.1.0 (`ble/ble_version.hpp`) | Bluetooth LE on blew 1.1.0 (`third_party/blew/`) |

- `android_drivers_sdk.hpp`: the compile definitions of the package (`ANDROID_BUILD`) and the capability macros
  (`USE_BLE`).
- `include/daasiot_android/ble.h`: the public API of the BLE driver, `daas_ble_set_local_sid()` (android_daas calls it).
- `third_party/blew/`: blew (ble-wrapper) 1.1.0, prebuilt `lib/<ABI>/libblew.a`, its headers and the Java sources
  `it.sebyone.blew` (packaged by the android_daas module). Linked whole: its JNI entry points are referenced only from
  Java.
- `libdaas-drivers.lock.json`: the drivers of each `<target>/<linkage>` (generic ones from the package, specialized
  ones from this folder). Update it when a driver version changes or a target is added.

The URI of a BLE node is its blew node id, 8 hexadecimal digits (e.g. `"000000C8"`), derived by the application
from the DIN.
