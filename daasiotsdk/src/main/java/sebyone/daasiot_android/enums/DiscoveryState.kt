package sebyone.daasiot_android.enums

/** Mirrors the native discovery_state_t enum (daas_types.hpp). */
enum class DiscoveryState(private val code: Int) : NativeEnum {
    OFF(0),
    SENDER_ONLY(1),
    RECEIVER_ONLY(2),
    FULL(3);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): DiscoveryState = values().firstOrNull { it.code == code } ?: OFF
    }
}
