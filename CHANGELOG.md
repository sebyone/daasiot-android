<!-- daas © 2019–2026 Sebyone Srl -->

# Changelog

All notable changes to daasiot-android. The format is based on [Keep a Changelog](https://keepachangelog.com/), and
the project uses a `MAJOR.MINOR.PATCH` scheme (`VERSION`).

## [0.1.0] - 2026-11-23

First version, on libdaas **0.24.11** (daas protocol 6.1.0), static package for android arm64-v8a.

### Added
- `daas::libdaas`: libdaas with its drivers linked in the order of the package (libdaas, BLE driver, INET4, INET6,
  libdaas_nodrivers).
- **android-drivers_sdk/ble**: the BLE driver of the `ble` slot of `_LINK_BLE` (1.0.0) on blew 1.1.0, from the driver
  of daasiot-macos 0.1.0; `daasiot_android/ble.h` with `daas_ble_set_local_sid()`.
- blew 1.1.0 for android arm64-v8a (`libblew.a`, headers, Java sources `it.sebyone.blew`).
