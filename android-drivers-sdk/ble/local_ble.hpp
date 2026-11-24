/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: local_ble.hpp — daasiot-android, android-drivers-sdk
 *
 * The uri class of the BLE driver (uri_ble_t) and the declarations shared by the driver and the application. The address
 * of a BLE node is its blew node id, 8 hexadecimal digits (e.g. "000000C8"); the application derives it from the
 * DIN (8 hexadecimal digits of the DIN). Only the first 8 characters are the id: what follows is ignored.
 *
 * Contributors:
 *   Paolo Lo Giacco <plogiacco@smartlab.it>
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-01  Paolo Lo Giacco  libdaas 0.24.4 (source/drivers/implementation/ble/local_ble.hpp of daas-api)
 *   2026-11-12  Lorenzo Grillo   esp32-drivers_sdk: the uri is validated (8 hex digits) and parsed once
 *   2026-11-14  Lorenzo Grillo   daasiot-macos: macos-drivers_sdk, blew on CoreBluetooth
 *   2026-11-23  Lorenzo Grillo   daasiot-android: android-drivers_sdk, blew on the Android BLE stack
 *   2026-11-24  l.grillo@sebyone.it  android-drivers-sdk (conventions): blew precompiled by the DaaS Build Console
 */
#pragma once

#include <stdint.h>

#include "interface/ifuri.hpp"

#define _MAX_BT_CONNECTIONS 5   // subs of the driver: links of blew, both roles together
#define BLE_NODE_ID_LEN 8       // hexadecimal digits of a blew node id

class uri_ble_t : public ifuri_t {
public:
    explicit uri_ble_t(const char* suri);
    ~uri_ble_t() override {}

    char node_id[BLE_NODE_ID_LEN + 1];   // lower case, NUL-terminated
};

// Local SID published by blew (INFO) for the discovery filters of the other nodes: the application sets it after doInit().
// Until it is set, the driver publishes the SID of the discovery beacons it sends.
extern "C" void daas_ble_set_local_sid(uint64_t sid);
