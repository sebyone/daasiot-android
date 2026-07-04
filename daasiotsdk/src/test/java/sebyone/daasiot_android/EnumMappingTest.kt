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
