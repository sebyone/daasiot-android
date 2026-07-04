package sebyone.daasiot_android

import sebyone.daasiot_android.entity.FeatureRequest
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.FeatureAction
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

/**
 * FeatureRequest backs the fixed-size native buffers params[16]/payload[128] (feature_rq_t in
 * daas_types.hpp): oversized arrays must be rejected here rather than silently truncated natively.
 */
class FeatureRequestTest {

    @Test
    fun acceptsArraysAtTheSizeLimit() {
        val request = FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            params = ByteArray(FeatureRequest.MAX_PARAMS_SIZE),
            payload = ByteArray(FeatureRequest.MAX_PAYLOAD_SIZE)
        )
        assertEquals(FeatureRequest.MAX_PARAMS_SIZE, request.params.size)
        assertEquals(FeatureRequest.MAX_PAYLOAD_SIZE, request.payload.size)
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsParamsLargerThanTheNativeBuffer() {
        FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            params = ByteArray(FeatureRequest.MAX_PARAMS_SIZE + 1)
        )
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsPayloadLargerThanTheNativeBuffer() {
        FeatureRequest(
            Feature.STORAGE,
            FeatureAction.STORAGE_RETRIEVE_BLOB,
            payload = ByteArray(FeatureRequest.MAX_PAYLOAD_SIZE + 1)
        )
    }

    @Test
    fun equals_isStructuralNotReferential_forByteArrayFields() {
        val a = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2), byteArrayOf(3, 4))
        val b = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2), byteArrayOf(3, 4))

        assertEquals(a, b)
        assertEquals(a.hashCode(), b.hashCode())
    }

    @Test
    fun equals_detectsDifferingParams() {
        val a = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(1, 2))
        val b = FeatureRequest(Feature.COMPUTE, FeatureAction.NONE, byteArrayOf(9, 9))

        assertNotEquals(a, b)
    }
}
