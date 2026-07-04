/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DaasInstance.kt
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

import sebyone.daasiot_android.events.IDaasApiEvent
import sebyone.daasiot_android.util.NativeCleaner
import java.io.Closeable

/**
 * Optional [Closeable] wrapper around the raw `ptr: Long` handle returned by [DaasWrapper.create].
 * [DaasWrapper]'s functions are unchanged and still take that `ptr` directly - this class doesn't
 * replace them, it just gives the handle itself a lifecycle: an explicit [close], plus a
 * [NativeCleaner] safety net in case a caller forgets, since a bare `Long` floating between call
 * sites has no object identity for the GC to track on its own.
 *
 * ```
 * DaasInstance.create().use { instance ->
 *     DaasWrapper.doInit(instance.handle, sid, din)
 *     ...
 * }
 * ```
 */
class DaasInstance private constructor(private var ptr: Long) : Closeable {

    private val cleanable: AutoCloseable = ptr.let { ptrAtConstruction ->
        // Capture the raw handle value only - never `this` - so the phantom reference can still
        // fire once this wrapper becomes unreachable.
        NativeCleaner.register(this) { DaasWrapper.destroy(ptrAtConstruction) }
    }

    /** The raw native handle, for use with [DaasWrapper]'s ptr-based functions. */
    val handle: Long
        get() {
            check(ptr != 0L) { "DaasInstance is closed" }
            return ptr
        }

    override fun close() {
        cleanable.close()
        ptr = 0L
    }

    companion object {
        @JvmOverloads
        @JvmStatic
        fun create(eventHandler: IDaasApiEvent? = null, lhver: String? = null): DaasInstance =
            DaasInstance(DaasWrapper.create(eventHandler, lhver))
    }
}
