package sebyone.daasiot_android.enums

/** Mirrors the native performs_mode_t enum (daas_types.hpp). */
enum class PerformMode(private val code: Int) : NativeEnum {
    PERFORM_CORE_THREAD(0),
    PERFORM_CORE_NO_THREAD(1);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): PerformMode = values().firstOrNull { it.code == code } ?: PERFORM_CORE_NO_THREAD
    }
}
