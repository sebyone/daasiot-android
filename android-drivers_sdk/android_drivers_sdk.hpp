/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: android_drivers_sdk.hpp — daasiot-android, android-drivers_sdk
 *
 * Compile definitions of the android-drivers_sdk, included first by every driver of this folder: they take the place
 * of the -D flags of drivers_sdk/BUILDING.md of the libdaas package (platform_defs of its MANIFEST.json:
 * ANDROID_BUILD) and of the capability macros (USE_<ID>).
 *
 *   USE_BLE   1 (default): the BLE driver on blew (Android BLE stack); -DUSE_BLE=0 leaves the slot to the nodriver
 *   DAAS_BLE_SCAN_TIMEOUT_MS  how long a discovery or an open scans for the remote node (blew scan_timeout), 3000
 *   DAAS_BLE_OPEN_TIMEOUT_MS  how long the opening of a link may take (blew open_timeout), 5000
 *   DAAS_BLE_PING_INTERVAL_MS PING period of an idle link (blew ping_interval), 1500
 *   DAAS_BLE_LINK_TIMEOUT_MS  a link silent this long is closed (blew link_timeout), 4000
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation (from macos_drivers_sdk.hpp of daasiot-macos)
 *   2026-11-23  Lorenzo Grillo  DAAS_BLE_SCAN_TIMEOUT_MS (3 s instead of the 5 s of blew), DAAS_BLE_OPEN_TIMEOUT_MS
 *   2026-11-23  Lorenzo Grillo  DAAS_BLE_PING_INTERVAL_MS 1.5 s, DAAS_BLE_LINK_TIMEOUT_MS 4 s (blew: 5 s, 10 s)
 */
#pragma once

// platform_defs of the libdaas packages for Android
#ifndef ANDROID_BUILD
#define ANDROID_BUILD 1
#endif

// capabilities of the drivers of this sdk
#ifndef USE_BLE
#define USE_BLE 1
#endif

// Timings of blew for the BLE driver (blew::link::Config). A scan blocks the thread that sends a discovery for
// DAAS_BLE_SCAN_TIMEOUT_MS at most (a node advertises every few hundred ms); a link that answers no PING for
// DAAS_BLE_LINK_TIMEOUT_MS is closed, so a peer gone without closing frees its link quickly and can link again.
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
