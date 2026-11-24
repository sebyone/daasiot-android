/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: Link.java — android_daas (daasiot-android)
 *
 * link_t of daas_types.hpp: the communication technologies of libdaas.
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

public enum Link {
    NONE(0), DAAS(1), INET4(2), LORA(3), BLE(4), MQTT5(5), UART(6), ZIGBEE(7), IPRAW(8), LEGACY(9), INET6(10);

    public final int value;
    Link(int value) { this.value = value; }

    public static Link of(int value) {
        for (Link l : values()) if (l.value == value) return l;
        return NONE;
    }
}
