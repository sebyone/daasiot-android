<!-- daas © 2019–2026 Sebyone Srl -->
<!-- Contributors: l.grillo@sebyone.it -->

# template — the smallest specialized driver

`drv_template.hpp/.cpp` and `template_version.hpp`: an `idriver_t` that builds against `drivers_sdk/include` of the
libdaas package and fills the free static slot `uartx` (`_LINK_UART`) with `DAAS_DRIVER(uartx, ...)`. Every method
refuses the operation. It is built by `build_drivers_kit.py` for every entry (static archive, one per ABI), so it
keeps compiling with every libdaas version, and never installed in a kit.

A new driver:

1. copy `template/` to `<id>/`, rename `template` to `<id>` in the files (`drv_<id>`, `<id>_version.hpp`,
   `DAAS_DRIVER_VERSION_<ID>`, `USE_<ID>`);
2. choose a free slot of libdaas (`lora zigbee uartx mqtt5 legacy ble ble5t`, the link of the slot);
3. implement the medium: subs, beacons, channel plans (driver guide, `docs/shared/android-drivers-sdk_<code>.md`);
4. add `daas_driver(<id> ...)` to `CMakeLists.txt`, the driver to `libdaas-drivers.lock.json` and the files of its kit
   to `build_drivers_kit.py` and to `android_daas/libdaas.cmake`.
