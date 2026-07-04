/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: NodeInfo.kt
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * s.meduri@sebyone.it - refactoring
 *
 */

package sebyone.daasiot_android.entity

import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DiscoveryState

/** Mirrors the native `node_info_t` struct (daas_types.hpp). */
data class NodeInfo(
    val powerOnTime: Long,
    val linked: Long,
    val lock: Long,
    val sklen: Int,
    val skey: ByteArray,
    val acceptRequestPolicy: AcceptRequestPolicy,
    val sid: Long,
    val din: Long,
    val ddoPolicy: DDOPolicy,
    val discoveryState: DiscoveryState,
    val networkOffset: Long,
    val networkInSync: Boolean
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is NodeInfo) return false
        return powerOnTime == other.powerOnTime &&
                linked == other.linked &&
                lock == other.lock &&
                sklen == other.sklen &&
                skey.contentEquals(other.skey) &&
                acceptRequestPolicy == other.acceptRequestPolicy &&
                sid == other.sid &&
                din == other.din &&
                ddoPolicy == other.ddoPolicy &&
                discoveryState == other.discoveryState &&
                networkOffset == other.networkOffset &&
                networkInSync == other.networkInSync
    }

    override fun hashCode(): Int {
        var result = powerOnTime.hashCode()
        result = 31 * result + linked.hashCode()
        result = 31 * result + lock.hashCode()
        result = 31 * result + sklen
        result = 31 * result + skey.contentHashCode()
        result = 31 * result + acceptRequestPolicy.hashCode()
        result = 31 * result + sid.hashCode()
        result = 31 * result + din.hashCode()
        result = 31 * result + ddoPolicy.hashCode()
        result = 31 * result + discoveryState.hashCode()
        result = 31 * result + networkOffset.hashCode()
        result = 31 * result + networkInSync.hashCode()
        return result
    }
}
