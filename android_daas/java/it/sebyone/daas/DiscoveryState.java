/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: DiscoveryState.java — android_daas (daasiot-android)
 *
 * discovery_state_t of daas_types.hpp (DaasAPI::setDiscoveryState).
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation
 */
package it.sebyone.daas;

public enum DiscoveryState {
    OFF(0), SENDER_ONLY(1), RECEIVER_ONLY(2), FULL(3);

    public final int value;
    DiscoveryState(int value) { this.value = value; }
}
