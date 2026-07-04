package sebyone.daasiot_android.entity

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction

/**
 * Mirrors the native `feature_rq_t` struct (daas_types.hpp), for use with
 * [DaasWrapper.requestFeature]. `params`/`payload` back fixed-size native buffers (16 and 128
 * bytes respectively); shorter arrays are zero-padded natively, but longer ones are rejected here
 * rather than silently truncated.
 */
data class FeatureRequest(
    val feature: Feature,
    val action: FeatureAction,
    val params: ByteArray = ByteArray(0),
    val payload: ByteArray = ByteArray(0)
) {
    init {
        require(params.size <= MAX_PARAMS_SIZE) { "params cannot exceed $MAX_PARAMS_SIZE bytes" }
        require(payload.size <= MAX_PAYLOAD_SIZE) { "payload cannot exceed $MAX_PAYLOAD_SIZE bytes" }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is FeatureRequest) return false
        return feature == other.feature &&
                action == other.action &&
                params.contentEquals(other.params) &&
                payload.contentEquals(other.payload)
    }

    override fun hashCode(): Int {
        var result = feature.hashCode()
        result = 31 * result + action.hashCode()
        result = 31 * result + params.contentHashCode()
        result = 31 * result + payload.contentHashCode()
        return result
    }

    companion object {
        const val MAX_PARAMS_SIZE = 16
        const val MAX_PAYLOAD_SIZE = 128
    }
}
