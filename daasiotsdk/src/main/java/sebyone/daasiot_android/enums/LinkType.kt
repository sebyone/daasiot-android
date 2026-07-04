package sebyone.daasiot_android.enums

/**
 * Mirrors the native link_t enum (daas_types.hpp). MAX_LINKS is a bounds marker in the native
 * enum, not a selectable link, and is intentionally not exposed here.
 */
enum class LinkType(private val code: Int) : NativeEnum {
    LINK_NONE(0),
    LINK_DAAS(1),
    LINK_INET4(2),
    LINK_LORA(3),
    LINK_BLE(4),
    LINK_MQTT5(5),
    LINK_UART(6),
    LINK_ZIGBEE(7),
    LINK_RAW(8),
    LINK_LEGACY(9);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): LinkType = values().firstOrNull { it.code == code } ?: LINK_NONE
    }
}
