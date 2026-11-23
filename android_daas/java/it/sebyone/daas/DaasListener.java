/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: DaasListener.java — android_daas (daasiot-android)
 *
 * The events of a DaasNode (IDaasApiEvent of libdaas), on its service thread: the whole API can be called.
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

/** Every method has an empty default: override the events you need. */
public interface DaasListener {
    default void onDinAccepted(long din) {}
    /** A DDO from {@code din} is waiting: DaasNode.pull(din) until null. */
    default void onDdoReceived(long din, int typeset, int size) {}
    default void onFrisbeeReceived(long din) {}
    default void onNodeStateReceived(long din) {}
    default void onAtsSyncCompleted(long din) {}
    default void onFrisbeeDperfCompleted(long din, int packets, int blockSize) {}
    /** A network {@code sid} answered the discovery, hosted by {@code din}, over {@code link}. */
    default void onNetworkDiscovered(long din, long sid, Link link) {}
    /** The join completed: the node is a member of {@code sid} with the DIN {@code din} (already saved). */
    default void onNodeConnectedToNetwork(long sid, long din) {}
    default void onStreamInfoReceived(long din, int streamType, int streamId) {}
    /** The node is isolated by the attack panic defense until the process is restarted. */
    default void onAttackPanic(long origin, long from) {}
}
