<!-- daas © 2019–2026 Sebyone Srl -->

# android-drivers-sdk

The specialized drivers of daasiot-android: drivers of libdaas (`idriver_t`, driver ABI 2) built with the Android NDK
only against the headers of the driver contract of the libdaas package (`DAAS_ROOT/drivers_sdk/include`), as the
drivers SDKs of daasiot-gnu, daasiot-macos and daasiot-esp32. Each driver ends with one `DAAS_DRIVER(<id>, ...)` entry,
which defines `daas_static_driver_<id>()`: linked before `libdaas_nodrivers.a` (`DAAS_EXTRA_DRIVERS` of
`android_daas/libdaas.cmake`), it fills the slot `<id>` of libdaas.

| Driver | Slot | Version | Medium |
|---|---|---|---|
| `ble/` | `ble` (`_LINK_BLE`) | 1.1.0 (`ble/ble_version.hpp`) | Bluetooth LE on blew 1.1.0, precompiled by the DaaS Build Console |
| `template/` | `uartx` (`_LINK_UART`) | 1.0.0 | none: the start of a new driver, never installed in a kit |

| File | Content |
|---|---|
| `build_drivers_kit.py` | builds the drivers_kit of every entry of the drivers lock into `android_daas/drivers_kit/<target>/static/`: `libdaas_drv_ble.a`, `libblew.a` (blew of the DaaS Build Console, checked against the SHA-256 of the lock), the Java side of blew in `java/`, `DRIVERS.txt` |
| `CMakeLists.txt`, `libdaas_package.cmake` | one project per entry (NDK toolchain, `ANDROID_ABI`), against the libdaas package of the entry |
| `libdaas-drivers.lock.json` | the drivers of each `<target>/<linkage>` and the blew packages (paths on the DaaS Build Console, SHA-256) |
| `ble/ble_config.hpp` | the timings of blew used by the BLE driver (`DAAS_BLE_*_MS`) |
| `include/daasiot_android/ble.h` | the public API of the BLE driver, `daas_ble_set_local_sid()` |

```sh
python3 android-drivers-sdk/build_drivers_kit.py                 # every ABI (ANDROID_NDK_HOME, DAAS_BUILDER_URL)
python3 android-drivers-sdk/build_drivers_kit.py --check         # the installed kits
```

blew is never in this repository: a new blew is a new path and SHA-256 in the drivers lock (`--update-lock`, after a
review of the downloaded file). The packages carry the specialized drivers compiled only, in the drivers_kit.

The URI of a BLE node is its blew node id, 8 hexadecimal digits (e.g. `"000000C8"`), derived by the application
from the DIN.
