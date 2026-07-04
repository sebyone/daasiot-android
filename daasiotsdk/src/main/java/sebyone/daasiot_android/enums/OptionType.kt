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
