/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: AcceptRequests.java — android_daas (daasiot-android)
 *
 * accept_request_policy_t of daas_types.hpp (DaasAPI::setAcceptRequestsLevel).
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

public enum AcceptRequests {
    NONE(0), MAPPED_ONLY(1), MAPPED_AND_SAME_NETWORK(3), MAPPED_AND_ROUTED(5),
    MAPPED_AND_ROUTED_AND_SAME_NETWORK(7), ALL(0xFF);

    public final int value;
    AcceptRequests(int value) { this.value = value; }
}
