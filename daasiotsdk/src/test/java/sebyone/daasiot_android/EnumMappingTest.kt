/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: EnumMappingTest.kt
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

import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.DiscoveryState
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.NativeEnum
import sebyone.daasiot_android.enums.OptionType
import sebyone.daasiot_android.enums.PerformMode
import sebyone.daasiot_android.enums.StreamType
import org.junit.Assert.assertEquals
import org.junit.Test

/**
 * Every NativeEnum must round-trip through value()/fromValue() with the exact numeric codes
 * defined in the native headers (daas_types.hpp) - these are hand-transcribed from C++, so a
 * mismatch here would silently corrupt every native call that uses the affected enum.
 */
class EnumMappingTest {

    @Test
    fun daasError_roundTripsAllValues() {
        val expected = mapOf(
            0 to DaasError.ERROR_NONE,
            1 to DaasError.ERROR_CORE_ALREADY_INITIALIZED,
            2 to DaasError.ERROR_CORE_STOPPED,
            3 to DaasError.ERROR_CANNOT_INITIALIZE,
            4 to DaasError.ERROR_CANNOT_CREATE_NODE,
            5 to DaasError.ERROR_DIN_ALREADY_EXIST,
            6 to DaasError.ERROR_CANNOT_MAP_NODE,
            7 to DaasError.ERROR_INVALID_USER_TYPESET,
            8 to DaasError.ERROR_SEND_DDO,
            9 to DaasError.ERROR_NO_DDO_PRESENT,
            10 to DaasError.ERROR_DIN_UNKNOWN,
            11 to DaasError.ERROR_CHANNEL_FAILURE,
            12 to DaasError.ERROR_ATS_NOT_SYNCED,
            13 to DaasError.ERROR_DISCOVERY_DISABLED,
            14 to DaasError.ERROR_POLICY_STRICT_ENABLED,
            15 to DaasError.ERROR_INVALID_DME,
            16 to DaasError.ERROR_THREADS_ALREADY_STARTED,
            17 to DaasError.ERROR_NOT_IMPLEMENTED,
            18 to DaasError.ERROR_TX_QUEUE_FULL,
            19 to DaasError.ERROR_TIMEOUT_NOT_EXPIRED,
            20 to DaasError.ERROR_ROUTE_NOT_FOUND,
            21 to DaasError.ERROR_UNKNOWN
        )
        assertRoundTrips(expected) { DaasError.fromValue(it) }
        assertEquals(DaasError.ERROR_UNKNOWN, DaasError.fromValue(-1))
    }

    @Test
    fun linkType_roundTripsAllValues() {
        val expected = mapOf(
            0 to LinkType.LINK_NONE,
            1 to LinkType.LINK_DAAS,
            2 to LinkType.LINK_INET4,
            3 to LinkType.LINK_LORA,
            4 to LinkType.LINK_BLE,
            5 to LinkType.LINK_MQTT5,
            6 to LinkType.LINK_UART,
            7 to LinkType.LINK_ZIGBEE,
            8 to LinkType.LINK_RAW,
            9 to LinkType.LINK_LEGACY
        )
        assertRoundTrips(expected) { LinkType.fromValue(it) }
        assertEquals(LinkType.LINK_NONE, LinkType.fromValue(-1))
    }

    @Test
    fun performMode_roundTripsAllValues() {
        val expected = mapOf(
            0 to PerformMode.PERFORM_CORE_THREAD,
            1 to PerformMode.PERFORM_CORE_NO_THREAD
        )
        assertRoundTrips(expected) { PerformMode.fromValue(it) }
    }

    @Test
    fun acceptRequestPolicy_roundTripsAllValues() {
        // Bitflag-derived values (see flag_trust_* in daas_types.hpp), not a dense 0..n range.
        val expected = mapOf(
            0 to AcceptRequestPolicy.TRUST_NONE,
            1 to AcceptRequestPolicy.TRUST_MAPPED_ONLY,
            3 to AcceptRequestPolicy.TRUST_MAPPED_AND_SAME_NETWORK,
            5 to AcceptRequestPolicy.TRUST_MAPPED_AND_ROUTED,
            7 to AcceptRequestPolicy.TRUST_MAPPED_AND_ROUTED_AND_SAME_NETWORK,
            255 to AcceptRequestPolicy.TRUST_ALL
        )
        assertRoundTrips(expected) { AcceptRequestPolicy.fromValue(it) }
    }

    @Test
    fun ddoPolicy_roundTripsAllValues() {
        val expected = mapOf(
            0 to DDOPolicy.SKIP_ON_FAILURE,
            1 to DDOPolicy.RETRY_ON_FAILURE,
            2 to DDOPolicy.EXPONENTIAL_BACKOFF_RETRY_ON_FAILURE
        )
        assertRoundTrips(expected) { DDOPolicy.fromValue(it) }
    }

    @Test
    fun discoveryState_roundTripsAllValues() {
        val expected = mapOf(
            0 to DiscoveryState.OFF,
            1 to DiscoveryState.SENDER_ONLY,
            2 to DiscoveryState.RECEIVER_ONLY,
            3 to DiscoveryState.FULL
        )
        assertRoundTrips(expected) { DiscoveryState.fromValue(it) }
    }

    @Test
    fun streamType_roundTripsAllValues() {
        val expected = mapOf(
            0 to StreamType.STREAM_OPEN,
            1 to StreamType.STREAM_DATA,
            2 to StreamType.STREAM_CLOSE
        )
        assertRoundTrips(expected) { StreamType.fromValue(it) }
    }

    @Test
    fun feature_roundTripsAllValues() {
        val expected = mapOf(
            1 to Feature.STORAGE,
            2 to Feature.COMPUTE
        )
        assertRoundTrips(expected) { Feature.fromValue(it) }
    }

    @Test(expected = IllegalArgumentException::class)
    fun feature_fromValue_throwsOnUnknownCode() {
        Feature.fromValue(999)
    }

    @Test
    fun featureAction_roundTripsAllValues() {
        val expected = mapOf(
            0 to FeatureAction.STORAGE_RETRIEVE_BLOB,
            1 to FeatureAction.STORAGE_SEND_BLOB,
            2 to FeatureAction.STORAGE_FIND_BLOB,
            3 to FeatureAction.STORAGE_REGISTER_BLOB,
            4 to FeatureAction.NONE
        )
        assertRoundTrips(expected) { FeatureAction.fromValue(it) }
    }

    @Test
    fun optionType_roundTripsAllValues() {
        val expected = mapOf(
            0 to OptionType.SET_DDO_RX_BUFFER_SIZE,
            1 to OptionType.SET_DDO_TX_BUFFER_SIZE,
            2 to OptionType.SET_RT_BUFFER_SIZE,
            3 to OptionType.SET_MAX_DDO_RETRY,
            4 to OptionType.ENABLE_AUTO_ROUTE_ON_PUSH_FAILURE,
            5 to OptionType.SET_ROUTE_TIMEOUT,
            6 to OptionType.SET_ROUTE_TTL,
            7 to OptionType.SET_PACKET_STATUS_QUEUE_SIZE,
            8 to OptionType.SET_FETCH_TIMEOUT
        )
        assertRoundTrips(expected) { OptionType.fromValue(it) }
    }

    private fun <T : NativeEnum> assertRoundTrips(expected: Map<Int, T>, fromValue: (Int) -> T) {
        for ((code, constant) in expected) {
            assertEquals("value() for $constant", code, constant.value())
            assertEquals("fromValue($code)", constant, fromValue(code))
        }
    }
}
