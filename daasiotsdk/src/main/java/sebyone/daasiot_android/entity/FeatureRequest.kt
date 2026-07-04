/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: FeatureRequest.kt
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

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction

/**
 * Mirrors the native `feature_rq_t` struct (daas_types.hpp), for use with
 * [DaasWrapper.requestFeature]. `params`/`payload` back fixed-size native buffers (16 and 128
 * bytes respectively); shorter arrays are zero-padded natively, but longer ones are rejected here
 * rather than silently truncated.
 */
data class FeatureRequest(
    val feature: Feature,
    val action: FeatureAction,
    val params: ByteArray = ByteArray(0),
    val payload: ByteArray = ByteArray(0)
) {
    init {
        require(params.size <= MAX_PARAMS_SIZE) { "params cannot exceed $MAX_PARAMS_SIZE bytes" }
        require(payload.size <= MAX_PAYLOAD_SIZE) { "payload cannot exceed $MAX_PAYLOAD_SIZE bytes" }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is FeatureRequest) return false
        return feature == other.feature &&
                action == other.action &&
                params.contentEquals(other.params) &&
                payload.contentEquals(other.payload)
    }

    override fun hashCode(): Int {
        var result = feature.hashCode()
        result = 31 * result + action.hashCode()
        result = 31 * result + params.contentHashCode()
        result = 31 * result + payload.contentHashCode()
        return result
    }

    companion object {
        const val MAX_PARAMS_SIZE = 16
        const val MAX_PAYLOAD_SIZE = 128
    }
}
