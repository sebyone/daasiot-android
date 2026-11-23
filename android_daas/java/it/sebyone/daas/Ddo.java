/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: Ddo.java — android_daas (daasiot-android)
 *
 * A DDO received (DaasNode.pull), copied out of libdaas.
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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public final class Ddo {
    /** The node it was pulled from. */
    public final long din;
    public final long origin;
    public final int typeset;
    /** Timestamp of the origin (ms, network time). */
    public final long timestamp;
    public final byte[] payload;

    public Ddo(long din, long origin, int typeset, long timestamp, byte[] payload) {
        this.din = din; this.origin = origin; this.typeset = typeset; this.timestamp = timestamp;
        this.payload = payload != null ? payload : new byte[0];
    }

    public String text() { return new String(payload, StandardCharsets.UTF_8); }
    /** The payload in the byte order of libdaas (little endian on Android). */
    public ByteBuffer buffer() { return ByteBuffer.wrap(payload).order(ByteOrder.LITTLE_ENDIAN); }
}
