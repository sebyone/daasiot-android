package sebyone.daasiot_android.enums

/** Mirrors the native ddo_policy_t enum (daas_types.hpp). */
enum class DDOPolicy(private val code: Int) : NativeEnum {
    SKIP_ON_FAILURE(0),
    RETRY_ON_FAILURE(1),
    EXPONENTIAL_BACKOFF_RETRY_ON_FAILURE(2);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): DDOPolicy = values().firstOrNull { it.code == code } ?: SKIP_ON_FAILURE
    }
}
