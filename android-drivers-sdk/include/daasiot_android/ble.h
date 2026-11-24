/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: daasiot_android/ble.h — daasiot-android, android-drivers-sdk
 *
 * Public API of the BLE driver for the application (target daas::drv_ble), without the headers of the driver contract.
 *
 *   - The Java side of blew (it.sebyone.blew.Blew.init(context)) must be initialized after loading the native library
 *     and before enabling _LINK_BLE.
 *   - daas_ble_set_local_sid(): the SID that blew publishes for the discovery filters of the other nodes. Call it
 *     after doInit() / loadConfiguration(); until then the driver publishes the SID of the beacons it sends.
 *   - The uri of a BLE node is its blew node id, 8 hexadecimal digits (e.g. "000000C8").
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation
 *   2026-11-24  l.grillo@sebyone.it  android-drivers-sdk (conventions): blew precompiled by the DaaS Build Console
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void daas_ble_set_local_sid(uint64_t sid);

#ifdef __cplusplus
}
#endif
