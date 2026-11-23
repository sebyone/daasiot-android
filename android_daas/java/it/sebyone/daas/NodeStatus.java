/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: NodeStatus.java — android_daas (daasiot-android)
 *
 * The status of the local node (node_info_t of daas_types.hpp, DaasNode.status()).
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

public final class NodeStatus {
    public final long powerOnTime;
    public final int linked;
    public final int lock;
    public final String securityKey;
    public final int acceptRequestPolicy;
    public final long sid;
    public final long din;
    public final int ddoPolicy;
    public final int discoveryState;
    /** ATS network time offset. */
    public final long oCapNet;
    public final boolean netInSync;

    public NodeStatus(long powerOnTime, int linked, int lock, String securityKey, int acceptRequestPolicy, long sid,
                      long din, int ddoPolicy, int discoveryState, long oCapNet, boolean netInSync) {
        this.powerOnTime = powerOnTime; this.linked = linked; this.lock = lock; this.securityKey = securityKey;
        this.acceptRequestPolicy = acceptRequestPolicy; this.sid = sid; this.din = din; this.ddoPolicy = ddoPolicy;
        this.discoveryState = discoveryState; this.oCapNet = oCapNet; this.netInSync = netInSync;
    }

    /** The user part of a SID/DIN: its top 16 bits (USER_PART_SHIFT 48), or the value when it has only that part. */
    public static long userPart(long id) { return (id >>> 48) != 0 ? (id >>> 48) : id; }
}
