/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: ble_config.hpp — daasiot-android, android-drivers-sdk
 *
 * The timings of blew used by the BLE driver (included by drv_ble.hpp). Every value can be changed with a -D flag of
 * the build of the drivers_kit (build_drivers_kit.py --cmake-arg "-DCMAKE_CXX_FLAGS=-DDAAS_BLE_SCAN_TIMEOUT_MS=5000").
 *
 *   DAAS_BLE_SCAN_TIMEOUT_MS  how long a discovery or an open scans for the remote node (blew scan_timeout), 3000
 *   DAAS_BLE_OPEN_TIMEOUT_MS  how long the opening of a link may take (blew open_timeout), 5000
 *   DAAS_BLE_PING_INTERVAL_MS PING period of an idle link (blew ping_interval), 1500
 *   DAAS_BLE_LINK_TIMEOUT_MS  a link silent this long is closed (blew link_timeout), 4000
 *
 * The platform definitions (ANDROID_BUILD) come from the platform_defs of the libdaas package and USE_BLE from the
 * CMakeLists.txt of this folder, as -D flags.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  android_drivers_sdk.hpp: DAAS_BLE_SCAN_TIMEOUT_MS 3 s, DAAS_BLE_OPEN_TIMEOUT_MS,
 *                               DAAS_BLE_PING_INTERVAL_MS 1.5 s, DAAS_BLE_LINK_TIMEOUT_MS 4 s (blew: 5 s, 10 s)
 *   2026-11-24  l.grillo@sebyone.it  ble/ble_config.hpp: only the timings (conventions of the daasiot-<target>
 *                                    repositories: the platform definitions come from the package)
 */
#pragma once

#ifndef DAAS_BLE_SCAN_TIMEOUT_MS
#define DAAS_BLE_SCAN_TIMEOUT_MS 3000
#endif
#ifndef DAAS_BLE_OPEN_TIMEOUT_MS
#define DAAS_BLE_OPEN_TIMEOUT_MS 5000
#endif
#ifndef DAAS_BLE_PING_INTERVAL_MS
#define DAAS_BLE_PING_INTERVAL_MS 1500
#endif
#ifndef DAAS_BLE_LINK_TIMEOUT_MS
#define DAAS_BLE_LINK_TIMEOUT_MS 4000
#endif
