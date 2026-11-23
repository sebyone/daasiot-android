/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: DaasError.java — android_daas (daasiot-android)
 *
 * daas_error_t of daas_types.hpp: the results of the API (DaasNode.errorString() for the text).
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

public final class DaasError {
    private DaasError() {}

    public static final int NONE = 0;
    public static final int CORE_ALREADY_INITIALIZED = 1;
    public static final int CORE_STOPPED = 2;
    public static final int CANNOT_INITIALIZE = 3;
    public static final int CANNOT_CREATE_NODE = 4;
    public static final int DIN_ALREADY_EXIST = 5;
    public static final int CANNOT_MAP_NODE = 6;
    public static final int INVALID_USER_TYPESET = 7;
    public static final int SEND_DDO = 8;
    public static final int NO_DDO_PRESENT = 9;
    public static final int DIN_UNKNOWN = 10;
    public static final int CHANNEL_FAILURE = 11;
    public static final int ATS_NOT_SYNCED = 12;
    public static final int DISCOVERY_DISABLED = 13;
    public static final int POLICY_STRICT_ENABLED = 14;
    public static final int INVALID_DME = 15;
    public static final int THREADS_ALREADY_STARTED = 16;
    public static final int NOT_IMPLEMENTED = 17;
    public static final int TX_QUEUE_FULL = 18;
    public static final int TIMEOUT_NOT_EXPIRED = 19;
    public static final int ROUTE_NOT_FOUND = 20;
    public static final int ROUTE_PENDING = 21;
    public static final int INVALID_ARGUMENT = 22;
    public static final int CHANNEL_POLICY = 23;
    public static final int NO_ANSWER = 24;
    public static final int PANIC_ISOLATED = 25;
    public static final int FEATURE_REFUSED = 26;
    public static final int UNKNOWN = 27;
}
