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
