/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DaasWrapperInstrumentedTest.kt
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * s.meduri@sebyone.it - refactoring
 *
 */

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
