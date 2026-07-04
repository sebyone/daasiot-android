package sebyone.daasiot_android.enums

/** Mirrors the native accept_request_policy_t enum (daas_types.hpp). */
enum class AcceptRequestPolicy(private val code: Int) : NativeEnum {
    TRUST_NONE(0),
    TRUST_MAPPED_ONLY(1),
    TRUST_MAPPED_AND_SAME_NETWORK(3),
    TRUST_MAPPED_AND_ROUTED(5),
    TRUST_MAPPED_AND_ROUTED_AND_SAME_NETWORK(7),
    TRUST_ALL(255);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): AcceptRequestPolicy = values().firstOrNull { it.code == code } ?: TRUST_NONE
    }
}
