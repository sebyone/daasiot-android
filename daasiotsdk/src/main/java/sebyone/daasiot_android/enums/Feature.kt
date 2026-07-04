package sebyone.daasiot_android.enums

/**
 * Mirrors the native feature_t enum (daas_types.hpp). MAX_FEATURE is a bounds marker equal to
 * COMPUTE in the native enum, not a distinct selectable feature, and is intentionally not exposed
 * here.
 */
enum class Feature(private val code: Int) : NativeEnum {
    STORAGE(1),
    COMPUTE(2);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): Feature =
            values().firstOrNull { it.code == code } ?: throw IllegalArgumentException("Unknown feature value: $code")
    }
}
