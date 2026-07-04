package sebyone.daasiot_android.enums

/** Mirrors the native daas_error_t enum (daas_types.hpp). */
enum class DaasError(private val code: Int) : NativeEnum {
    ERROR_NONE(0),
    ERROR_CORE_ALREADY_INITIALIZED(1),
    ERROR_CORE_STOPPED(2),
    ERROR_CANNOT_INITIALIZE(3),
    ERROR_CANNOT_CREATE_NODE(4),
    ERROR_DIN_ALREADY_EXIST(5),
    ERROR_CANNOT_MAP_NODE(6),
    ERROR_INVALID_USER_TYPESET(7),
    ERROR_SEND_DDO(8),
    ERROR_NO_DDO_PRESENT(9),
    ERROR_DIN_UNKNOWN(10),
    ERROR_CHANNEL_FAILURE(11),
    ERROR_ATS_NOT_SYNCED(12),
    ERROR_DISCOVERY_DISABLED(13),
    ERROR_POLICY_STRICT_ENABLED(14),
    ERROR_INVALID_DME(15),
    ERROR_THREADS_ALREADY_STARTED(16),
    ERROR_NOT_IMPLEMENTED(17),
    ERROR_TX_QUEUE_FULL(18),
    ERROR_TIMEOUT_NOT_EXPIRED(19),
    ERROR_ROUTE_NOT_FOUND(20),
    ERROR_UNKNOWN(21);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): DaasError = values().firstOrNull { it.code == code } ?: ERROR_UNKNOWN
    }
}
