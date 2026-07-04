package sebyone.daasiot_android.enums

/** Mirrors the native stream_type enum (daas_types.hpp). */
enum class StreamType(private val code: Int) : NativeEnum {
    STREAM_OPEN(0),
    STREAM_DATA(1),
    STREAM_CLOSE(2);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): StreamType = values().firstOrNull { it.code == code } ?: STREAM_OPEN
    }
}
