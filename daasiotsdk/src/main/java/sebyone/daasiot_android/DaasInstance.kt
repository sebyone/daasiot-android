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
