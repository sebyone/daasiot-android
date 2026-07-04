package sebyone.daasiot_android.entity

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.util.NativeCleaner
import java.io.Closeable

/**
 * Kotlin wrapper around a native DDO instance. Always call [close] explicitly once done, especially
 * for DDOs created locally for [DaasWrapper.push] - a [NativeCleaner] safety net also disposes the
 * native memory if a DDO is garbage collected without being closed, but GC timing is unspecified,
 * so it only bounds the leak rather than replacing deterministic cleanup. DDOs handed back by
 * [DaasWrapper.pull] are owned by this wrapper and must also be closed by the caller.
 */
class DDO private constructor(private var nativeHandle: Long, owned: Boolean) : Closeable {

    constructor() : this(nativeCreate(), true)
    constructor(typeset: Int) : this(nativeCreateWithTypeset(typeset), true)

    // Only owned DDOs (created here, or pulled and handed off to us) may free native memory;
    // non-owned DDOs never register a cleanup action at all. The cleanup lambda captures the raw
    // handle value only - never `this` (directly or via the nativeHandle property) - since a
    // reference back to the DDO instance would keep it permanently reachable and the phantom
    // reference would then never fire.
    private val cleanable: AutoCloseable? = nativeHandle.let { handleAtConstruction ->
        if (owned) NativeCleaner.register(this) { nativeDispose(handleAtConstruction) } else null
    }

    fun clearPayload() = nativeClearPayload(handle())

    fun setTypeset(typeset: Int) = nativeSetTypeset(handle(), typeset)
    fun getTypeset(): Int = nativeGetTypeset(handle())

    fun getOrigin(): Long = nativeGetOrigin(handle())
    fun getTimestamp(): Long = nativeGetTimestamp(handle())

    fun setPayload(data: ByteArray): Long = nativeSetPayload(handle(), data, 0, data.size)
    fun setPayload(data: ByteArray, offset: Int, length: Int): Long = nativeSetPayload(handle(), data, offset, length)

    fun getPayloadSize(): Long = nativeGetPayloadSize(handle())

    fun getPayload(): ByteArray {
        val size = getPayloadSize()
        check(size <= Int.MAX_VALUE) { "Payload too large for a Kotlin ByteArray: $size" }
        return nativeGetPayloadAsBinary(handle(), 0, size.toInt())
    }

    fun getPayloadAsBinary(offset: Int, maxSize: Int): ByteArray = nativeGetPayloadAsBinary(handle(), offset, maxSize)

    fun allocatePayload(size: Long): Long = nativeAllocatePayload(handle(), size)

    fun appendPayloadData(data: ByteArray): Long = nativeAppendPayloadData(handle(), data, 0, data.size)
    fun appendPayloadData(data: ByteArray, offset: Int, length: Int): Long =
        nativeAppendPayloadData(handle(), data, offset, length)

    override fun close() {
        cleanable?.close()
        nativeHandle = 0
    }

    /** Visible within the module only: used by [DaasWrapper.push] to pass the raw handle across JNI. */
    internal fun nativeHandle(): Long = handle()

    private fun handle(): Long {
        check(nativeHandle != 0L) { "DDO is closed" }
        return nativeHandle
    }

    companion object {
        init {
            System.loadLibrary("daas_jni")
        }

        @JvmStatic private external fun nativeCreate(): Long
        @JvmStatic private external fun nativeCreateWithTypeset(typeset: Int): Long
        @JvmStatic private external fun nativeDispose(handle: Long)
        @JvmStatic private external fun nativeClearPayload(handle: Long)
        @JvmStatic private external fun nativeSetTypeset(handle: Long, typeset: Int)
        @JvmStatic private external fun nativeGetOrigin(handle: Long): Long
        @JvmStatic private external fun nativeGetTimestamp(handle: Long): Long
        @JvmStatic private external fun nativeGetTypeset(handle: Long): Int
        @JvmStatic private external fun nativeSetPayload(handle: Long, data: ByteArray, offset: Int, length: Int): Long
        @JvmStatic private external fun nativeGetPayloadSize(handle: Long): Long
        @JvmStatic private external fun nativeGetPayloadAsBinary(handle: Long, offset: Int, maxSize: Int): ByteArray
        @JvmStatic private external fun nativeAllocatePayload(handle: Long, size: Long): Long
        @JvmStatic private external fun nativeAppendPayloadData(handle: Long, data: ByteArray, offset: Int, length: Int): Long
    }
}
