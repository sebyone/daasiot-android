/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: ble_version.hpp — daasiot-android, android-drivers-sdk
 *
 * Version of the BLE driver (drv_ble on blew), independent of libdaas: MAJOR when the uri_t form or the behaviour
 * on the medium breaks peers or applications, MINOR for compatible features, PATCH for fixes. Reported by
 * getBuildInfo() ("drivers: BLE 1.0.0;...") and listed in the README of android-drivers-sdk.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-12  Lorenzo Grillo  creation: 1.0.0, the BLE driver of libdaas 0.24.4 as a specialized driver
 *   2026-11-14  Lorenzo Grillo   daasiot-macos: macos-drivers_sdk, blew on CoreBluetooth
 *   2026-11-23  Lorenzo Grillo   daasiot-android: android-drivers_sdk, blew on the Android BLE stack
 *   2026-11-23  Lorenzo Grillo   1.0.1: scan of 3 s (DAAS_BLE_SCAN_TIMEOUT_MS)
 *   2026-11-23  Lorenzo Grillo   1.1.0: an open link with the node is reused by open_sub; ping 1.5 s, link timeout 4 s
 *   2026-11-24  l.grillo@sebyone.it  android-drivers-sdk (conventions): blew precompiled by the DaaS Build Console
 */
#pragma once

#define DAAS_DRIVER_VERSION_BLE "1.1.0"
