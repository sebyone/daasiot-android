/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: OptionType.kt
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

/** Mirrors the native option_t enum (daas_types.hpp). */
enum class OptionType(private val code: Int) : NativeEnum {
    SET_DDO_RX_BUFFER_SIZE(0),
    SET_DDO_TX_BUFFER_SIZE(1),
    SET_RT_BUFFER_SIZE(2),
    SET_MAX_DDO_RETRY(3),
    ENABLE_AUTO_ROUTE_ON_PUSH_FAILURE(4),
    SET_ROUTE_TIMEOUT(5),
    SET_ROUTE_TTL(6),
    SET_PACKET_STATUS_QUEUE_SIZE(7),
    SET_FETCH_TIMEOUT(8);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): OptionType =
            values().firstOrNull { it.code == code } ?: SET_DDO_RX_BUFFER_SIZE
    }
}
