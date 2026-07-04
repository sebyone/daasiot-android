/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: FeatureRequestTest.kt
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

import sebyone.daasiot_android.entity.FeatureRequest
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

/**
 * FeatureRequest backs the fixed-size native buffers params[16]/payload[128] (feature_rq_t in
 * daas_types.hpp): oversized arrays must be rejected here rather than silently truncated natively.
 */
class FeatureRequestTest {

    @Test
    fun acceptsArraysAtTheSizeLimit() {
        val request = FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            params = ByteArray(FeatureRequest.MAX_PARAMS_SIZE),
            payload = ByteArray(FeatureRequest.MAX_PAYLOAD_SIZE)
        )
        assertEquals(FeatureRequest.MAX_PARAMS_SIZE, request.params.size)
        assertEquals(FeatureRequest.MAX_PAYLOAD_SIZE, request.payload.size)
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsParamsLargerThanTheNativeBuffer() {
        FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            params = ByteArray(FeatureRequest.MAX_PARAMS_SIZE + 1)
        )
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsPayloadLargerThanTheNativeBuffer() {
        FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            payload = ByteArray(FeatureRequest.MAX_PAYLOAD_SIZE + 1)
        )
    }

    @Test
    fun equals_isStructuralNotReferential_forByteArrayFields() {
        val a = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2), byteArrayOf(3, 4))
        val b = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2), byteArrayOf(3, 4))

        assertEquals(a, b)
        assertEquals(a.hashCode(), b.hashCode())
    }

    @Test
    fun equals_detectsDifferingParams() {
        val a = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2))
        val b = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(9, 9))

        assertNotEquals(a, b)
    }
}
