package sebyone.daasiot_android

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.After
import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Exercises the real JNI layer on a device/emulator (System.loadLibrary only resolves on Android,
 * never on the host JVM, so this can't be a local unit test). Each test creates and tears down its
 * own instance(s); nothing here depends on test execution order.
 */
@RunWith(AndroidJUnit4::class)
class DaasWrapperInstrumentedTest {

    private val createdPtrs = mutableListOf<Long>()

    private fun createInstance(): Long {
        val ptr = DaasWrapper.create()
        createdPtrs += ptr
        return ptr
    }

    @After
    fun tearDown() {
        createdPtrs.forEach { DaasWrapper.destroy(it) }
        createdPtrs.clear()
    }

    @Test
    fun create_returnsANonZeroHandle() {
        val ptr = createInstance()
        assertNotEquals(0L, ptr)
    }

    @Test
    fun getVersion_returnsANonEmptyString() {
        val ptr = createInstance()
        assertTrue(DaasWrapper.getVersion(ptr).isNotEmpty())
    }

    @Test
    fun twoInstances_getIndependentHandles() {
        // Regression test for the old global-singleton native layer: before it was replaced with
        // a per-instance handle, a second create() would silently reuse/clobber the first.
        val first = createInstance()
        val second = createInstance()

        assertNotEquals(first, second)
        // Both handles must stay independently usable after the other is created.
        assertTrue(DaasWrapper.getVersion(first).isNotEmpty())
        assertTrue(DaasWrapper.getVersion(second).isNotEmpty())
    }

    @Test
    fun doInit_returnsATypedDaasError() {
        val ptr = createInstance()
        val result = DaasWrapper.doInit(ptr, sid = 1, din = 100)
        assertNotNull(result)
    }
}
