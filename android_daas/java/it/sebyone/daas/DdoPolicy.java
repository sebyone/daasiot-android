/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: DdoPolicy.java — android_daas (daasiot-android)
 *
 * ddo_policy_t of daas_types.hpp (DaasAPI::setDDOPolicy).
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

public enum DdoPolicy {
    SKIP_ON_FAILURE(0), RETRY_ON_FAILURE(1), EXPONENTIAL_BACKOFF_RETRY_ON_FAILURE(2);

    public final int value;
    DdoPolicy(int value) { this.value = value; }
}
