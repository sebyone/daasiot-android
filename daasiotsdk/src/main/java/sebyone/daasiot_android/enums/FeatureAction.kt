package sebyone.daasiot_android.enums

/** Mirrors the native feature_action_e enum (daas_types.hpp). */
enum class FeatureAction(private val code: Int) : NativeEnum {
    STORAGE_RETRIEVE_BLOB(0),
    STORAGE_SEND_BLOB(1),
    STORAGE_FIND_BLOB(2),
    STORAGE_REGISTER_BLOB(3),
    NONE(4);

    override fun value(): Int = code

    companion object {
        @JvmStatic
        fun fromValue(code: Int): FeatureAction = values().firstOrNull { it.code == code } ?: NONE
    }
}
