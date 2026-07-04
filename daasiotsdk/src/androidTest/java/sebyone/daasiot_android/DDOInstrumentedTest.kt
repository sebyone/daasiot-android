package sebyone.daasiot_android

import sebyone.daasiot_android.entity.DDO
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

/** Exercises the DDO wrapper's native handle lifecycle on a device/emulator. */
@RunWith(AndroidJUnit4::class)
class DDOInstrumentedTest {

    @Test
    fun setPayload_thenGetPayload_roundTrips() {
        val payload = byteArrayOf(1, 2, 3, 4, 5)
        DDO().use { ddo ->
            ddo.setPayload(payload)
            assertEquals(payload.size.toLong(), ddo.getPayloadSize())
            assertArrayEquals(payload, ddo.getPayload())
        }
    }

    @Test
    fun setTypeset_thenGetTypeset_roundTrips() {
        DDO().use { ddo ->
            ddo.setTypeset(42)
            assertEquals(42, ddo.getTypeset())
        }
    }

    @Test
    fun createWithTypeset_setsTypesetUpFront() {
        DDO(7).use { ddo ->
            assertEquals(7, ddo.getTypeset())
        }
    }
}
