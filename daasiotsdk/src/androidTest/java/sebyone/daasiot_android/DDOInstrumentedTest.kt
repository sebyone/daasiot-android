/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DDOInstrumentedTest.kt
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

import sebyone.daasiot_android.entity.DDO
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

/** Exercises the DDO wrapper's native handle lifecycle on a device/emulator. */
@RunWith(AndroidJUnit4::class)
class DDOInstrumentedTest {

    @Test
    fun setPayload_thenGetPayload_roundTrips() {
        val payload = byteArrayOf(1, 2, 3, 4, 5)
        DDO().use { ddo ->
            ddo.setPayload(payload)
            assertEquals(payload.size.toLong(), ddo.getPayloadSize())
            assertArrayEquals(payload, ddo.getPayload())
        }
    }

    @Test
    fun setTypeset_thenGetTypeset_roundTrips() {
        DDO().use { ddo ->
            ddo.setTypeset(42)
            assertEquals(42, ddo.getTypeset())
        }
    }

    @Test
    fun createWithTypeset_setsTypesetUpFront() {
        DDO(7).use { ddo ->
            assertEquals(7, ddo.getTypeset())
        }
    }
}
