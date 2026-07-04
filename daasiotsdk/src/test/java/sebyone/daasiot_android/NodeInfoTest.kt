/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: NodeInfoTest.kt
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

package sebyone.daasiot_android

import sebyone.daasiot_android.entity.NodeInfo
import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DiscoveryState
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

/**
 * NodeInfo overrides equals()/hashCode() by hand because its `skey` field is a ByteArray: Kotlin's
 * generated data class equals() would otherwise compare arrays by reference, so two structurally
 * identical NodeInfo instances built from separate native calls would incorrectly compare unequal.
 */
class NodeInfoTest {

    private fun sample(skey: ByteArray = byteArrayOf(1, 2, 3)) = NodeInfo(
        powerOnTime = 1000L,
        linked = 2L,
        lock = 0L,
        sklen = skey.size,
        skey = skey,
        acceptRequestPolicy = AcceptRequestPolicy.TRUST_MAPPED_ONLY,
        sid = 10L,
        din = 20L,
        ddoPolicy = DDOPolicy.SKIP_ON_FAILURE,
        discoveryState = DiscoveryState.SENDER_ONLY,
        networkOffset = 0L,
        networkInSync = true
    )

    @Test
    fun equals_isStructuralNotReferential_forSkeyArray() {
        val a = sample(byteArrayOf(1, 2, 3))
        val b = sample(byteArrayOf(1, 2, 3))

        assertEquals(a, b)
        assertEquals(a.hashCode(), b.hashCode())
    }

    @Test
    fun equals_detectsDifferingSkeyContent() {
        val a = sample(byteArrayOf(1, 2, 3))
        val b = sample(byteArrayOf(9, 9, 9))

        assertNotEquals(a, b)
    }

    @Test
    fun equals_detectsDifferingScalarField() {
        val a = sample()
        val b = sample().copy(din = 999L)

        assertNotEquals(a, b)
    }
}
