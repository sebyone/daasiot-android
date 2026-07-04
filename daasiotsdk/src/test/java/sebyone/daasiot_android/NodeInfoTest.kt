package sebyone.daasiot_android

import sebyone.daasiot_android.entity.NodeInfo
import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DiscoveryState
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

/**
 * NodeInfo overrides equals()/hashCode() by hand because its `skey` field is a ByteArray: Kotlin's
 * generated data class equals() would otherwise compare arrays by reference, so two structurally
 * identical NodeInfo instances built from separate native calls would incorrectly compare unequal.
 */
class NodeInfoTest {

    private fun sample(skey: ByteArray = byteArrayOf(1, 2, 3)) = NodeInfo(
        powerOnTime = 1000L,
        linked = 2L,
        lock = 0L,
        sklen = skey.size,
        skey = skey,
        acceptRequestPolicy = AcceptRequestPolicy.TRUST_MAPPED_ONLY,
        sid = 10L,
        din = 20L,
        ddoPolicy = DDOPolicy.SKIP_ON_FAILURE,
        discoveryState = DiscoveryState.SENDER_ONLY,
        networkOffset = 0L,
        networkInSync = true
    )

    @Test
    fun equals_isStructuralNotReferential_forSkeyArray() {
        val a = sample(byteArrayOf(1, 2, 3))
        val b = sample(byteArrayOf(1, 2, 3))

        assertEquals(a, b)
        assertEquals(a.hashCode(), b.hashCode())
    }

    @Test
    fun equals_detectsDifferingSkeyContent() {
        val a = sample(byteArrayOf(1, 2, 3))
        val b = sample(byteArrayOf(9, 9, 9))

        assertNotEquals(a, b)
    }

    @Test
    fun equals_detectsDifferingScalarField() {
        val a = sample()
        val b = sample().copy(din = 999L)

        assertNotEquals(a, b)
    }
}
