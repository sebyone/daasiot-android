/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DaasError.kt
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

package sebyone.daasiot_android.enums

/** Mirrors the native daas_error_t enum (daas_types.hpp). */
enum class DaasError(private val code: Int) : NativeEnum {
    ERROR_NONE(0),
    ERROR_CORE_ALREADY_INITIALIZED(1),
    ERROR_CORE_STOPPED(2),
    ERROR_CANNOT_INITIALIZE(3),
    ERROR_CANNOT_CREATE_NODE(4),
    ERROR_DIN_ALREADY_EXIST(5),
    ERROR_CANNOT_MAP_NODE(6),
    ERROR_INVALID_USER_TYPESET(7),
    ERROR_SEND_DDO(8),
    ERROR_NO_DDO_PRESENT(9),
    ERROR_DIN_UNKNOWN(10),
    ERROR_CHANNEL_FAILURE(11),
    ERROR_ATS_NOT_SYNCED(12),
    ERROR_DISCOVERY_DISABLED(13),
    ERROR_POLICY_STRICT_ENABLED(14),
    ERROR_INVALID_DME(15),
    ERROR_THREADS_ALREADY_STARTED(16),
    ERROR_NOT_IMPLEMENTED(17),
    ERROR_TX_QUEUE_FULL(18),
    ERROR_TIMEOUT_NOT_EXPIRED(19),
    ERROR_ROUTE_NOT_FOUND(20),
    ERROR_UNKNOWN(21);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): DaasError = values().firstOrNull { it.code == code } ?: ERROR_UNKNOWN
    }
}
