/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: NativeCleaner.kt
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

package sebyone.daasiot_android.util

import android.util.Log
import java.lang.ref.PhantomReference
import java.lang.ref.ReferenceQueue

/**
 * A minimal, hand-rolled equivalent of `java.lang.ref.Cleaner`, which Android only guarantees
 * from API 33 - this module's minSdk is 30, so `Cleaner` can't be relied on without pushing a
 * core-library-desugaring requirement onto every consuming app. Built directly on
 * [PhantomReference]/[ReferenceQueue] (the same primitives `Cleaner` itself is built on), which
 * have been available since the earliest Android API levels.
 *
 * This is a *safety net*, not a substitute for deterministic cleanup: GC timing is unspecified, so
 * callers should always call `close()` themselves and only rely on this to bound a leak to
 * "until the next GC" if they forget.
 */
internal object NativeCleaner {
    private val queue = ReferenceQueue<Any>()

    // Strong references to pending CleaningReferences: a PhantomReference that is itself
    // unreachable is never enqueued, so it must be kept alive here until it fires.
    private val pending = HashSet<CleaningReference>()

    init {
        Thread {
            while (true) {
                try {
                    val ref = queue.remove() as CleaningReference
                    synchronized(pending) { pending.remove(ref) }
                    runCleanup(ref)
                } catch (_: InterruptedException) {
                    // Never interrupted deliberately; keep this daemon thread running regardless.
                }
            }
        }.apply {
            name = "DaasNativeCleaner"
            isDaemon = true
            start()
        }
    }

    /**
     * Registers [cleanup] to run once [referent] becomes phantom-reachable (i.e. unreachable and
     * about to be collected). Returns an [AutoCloseable] the caller can invoke to run [cleanup]
     * deterministically instead (safe to call even if the GC-triggered path already ran it, or to
     * call more than once - only the first invocation has any effect).
     */
    fun register(referent: Any, cleanup: () -> Unit): AutoCloseable {
        val ref = CleaningReference(referent, queue, cleanup)
        synchronized(pending) { pending.add(ref) }
        return AutoCloseable {
            synchronized(pending) { pending.remove(ref) }
            ref.clear()
            runCleanup(ref)
        }
    }

    private fun runCleanup(ref: CleaningReference) {
        try {
            ref.cleanup()
        } catch (t: Throwable) {
            // One failing cleanup must not take the shared daemon thread down with it.
            Log.e("DaasNativeCleaner", "Native cleanup action failed", t)
        }
    }

    private class CleaningReference(
        referent: Any,
        queue: ReferenceQueue<Any>,
        private val cleanupAction: () -> Unit
    ) : PhantomReference<Any>(referent, queue) {
        private var cleaned = false

        fun cleanup() {
            if (!cleaned) {
                cleaned = true
                cleanupAction()
            }
        }
    }
}
