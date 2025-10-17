package sebyone.daasiot_android

object DaasWrapper {

    init {
        System.loadLibrary("daas_jni")
    }

    // --- Native JNI Methods ---
    private external fun nativeCreate(): Long
    private external fun nativeDestroy(ptr: Long)

    private external fun nativeGetVersion(ptr: Long): String
    private external fun nativeGetBuildInfo(ptr: Long): String
    private external fun nativeListAvailableDrivers(ptr: Long): String

    private external fun nativeDoInit(ptr: Long, sid: Int, din: Int): Int
    private external fun nativeDoEnd(ptr: Long): Int
    private external fun nativeDoReset(ptr: Long): Int
    private external fun nativeDoPerform(ptr: Long, mode: Int): Int

    private external fun nativeEnableDriver(ptr: Long, driverId: Int, localUri: String): Int
    private external fun nativeGetStatus(ptr: Long): Long // returns a pointer to nodestate_t

    private external fun nativeStoreConfiguration(ptr: Long, depotPtr: Long): Boolean
    private external fun nativeLoadConfiguration(ptr: Long, depotPtr: Long): Boolean
    private external fun nativeDoStatisticsReset(ptr: Long): Boolean
    private external fun nativeGetSystemStatistics(ptr: Long, code: Int): Long

    private external fun nativeMap(ptr: Long, din: Int, link: Int, uri: String?, skey: String?): Int
    private external fun nativeRemove(ptr: Long, din: Int): Int
    private external fun nativeListNodes(ptr: Long): Long // pointer to dinlist_t
    private external fun nativeLocate(ptr: Long, din: Int): Int

    private external fun nativeSendStatus(ptr: Long, din: Int): Int
    private external fun nativeStatus(ptr: Long, din: Int): Long
    private external fun nativeFetch(ptr: Long, din: Int, opts: Int): Long
    private external fun nativeGetSyncedTimestamp(ptr: Long): Long

    private external fun nativeUnlock(ptr: Long, din: Int, skey: String): Long
    private external fun nativeLock(ptr: Long, skey: String, policy: Int): Long

    private external fun nativeSyncNode(ptr: Long, din: Int, timezone: Int): Long
    private external fun nativeSyncNet(ptr: Long, din: Int, bubbleTime: Int): Long
    private external fun nativeSetATSMaxError(ptr: Long, error: Int)

    private external fun nativeUse(ptr: Long, din: Int): Boolean
    private external fun nativeEnd(ptr: Long, din: Int): Boolean
    private external fun nativeSend(ptr: Long, din: Int, data: ByteArray, size: Int): Int
    private external fun nativeReceived(ptr: Long, din: Int): Int
    private external fun nativeReceive(ptr: Long, din: Int, data: ByteArray, maxSize: Int): Int

    private external fun nativeListTypesets(ptr: Long): Long
    private external fun nativePull(ptr: Long, din: Int): Long
    private external fun nativePush(ptr: Long, din: Int, ddoPtr: Long): Int
    private external fun nativeAvailablesPull(ptr: Long, din: Int): Int
    private external fun nativeAddTypeset(ptr: Long, typesetCode: Int, typesetSize: Int): Int

    private external fun nativeFrisbee(ptr: Long, din: Int): Int
    private external fun nativeFrisbeeICMP(ptr: Long, din: Int, timeout: Int, retry: Int): Int
    private external fun nativeFrisbeeDPerf(ptr: Long, din: Int, senderTotal: Int, blockSize: Int, period: Int): Int
    private external fun nativeGetFrisbeeDPerfResult(ptr: Long): Long

    // --- Public Kotlin API ---
    fun create(): Long = nativeCreate()
    fun destroy(ptr: Long) = nativeDestroy(ptr)

    fun getVersion(ptr: Long): String = nativeGetVersion(ptr)
    fun getBuildInfo(ptr: Long): String = nativeGetBuildInfo(ptr)
    fun listAvailableDrivers(ptr: Long): String = nativeListAvailableDrivers(ptr)

    fun doInit(ptr: Long, sid: Int, din: Int): Int = nativeDoInit(ptr, sid, din)
    fun doEnd(ptr: Long): Int = nativeDoEnd(ptr)
    fun doReset(ptr: Long): Int = nativeDoReset(ptr)
    fun doPerform(ptr: Long, mode: Int): Int = nativeDoPerform(ptr, mode)

    fun enableDriver(ptr: Long, driverId: Int, localUri: String): Int = nativeEnableDriver(ptr, driverId, localUri)
    fun getStatus(ptr: Long): Long = nativeGetStatus(ptr)

    fun storeConfiguration(ptr: Long, depotPtr: Long): Boolean = nativeStoreConfiguration(ptr, depotPtr)
    fun loadConfiguration(ptr: Long, depotPtr: Long): Boolean = nativeLoadConfiguration(ptr, depotPtr)
    fun doStatisticsReset(ptr: Long): Boolean = nativeDoStatisticsReset(ptr)
    fun getSystemStatistics(ptr: Long, code: Int): Long = nativeGetSystemStatistics(ptr, code)

    fun map(ptr: Long, din: Int, link: Int, uri: String? = null, skey: String? = null): Int = nativeMap(ptr, din, link, uri, skey)
    fun remove(ptr: Long, din: Int): Int = nativeRemove(ptr, din)
    fun listNodes(ptr: Long): Long = nativeListNodes(ptr)
    fun locate(ptr: Long, din: Int): Int = nativeLocate(ptr, din)

    fun sendStatus(ptr: Long, din: Int): Int = nativeSendStatus(ptr, din)
    fun status(ptr: Long, din: Int): Long = nativeStatus(ptr, din)
    fun fetch(ptr: Long, din: Int, opts: Int): Long = nativeFetch(ptr, din, opts)
    fun getSyncedTimestamp(ptr: Long): Long = nativeGetSyncedTimestamp(ptr)

    fun unlock(ptr: Long, din: Int, skey: String): Long = nativeUnlock(ptr, din, skey)
    fun lock(ptr: Long, skey: String, policy: Int): Long = nativeLock(ptr, skey, policy)

    fun syncNode(ptr: Long, din: Int, timezone: Int): Long = nativeSyncNode(ptr, din, timezone)
    fun syncNet(ptr: Long, din: Int, bubbleTime: Int): Long = nativeSyncNet(ptr, din, bubbleTime)
    fun setATSMaxError(ptr: Long, error: Int) = nativeSetATSMaxError(ptr, error)

    fun use(ptr: Long, din: Int): Boolean = nativeUse(ptr, din)
    fun end(ptr: Long, din: Int): Boolean = nativeEnd(ptr, din)
    fun send(ptr: Long, din: Int, data: ByteArray, size: Int): Int = nativeSend(ptr, din, data, size)
    fun received(ptr: Long, din: Int): Int = nativeReceived(ptr, din)
    fun receive(ptr: Long, din: Int, data: ByteArray, maxSize: Int): Int = nativeReceive(ptr, din, data, maxSize)

    fun listTypesets(ptr: Long): Long = nativeListTypesets(ptr)
    fun pull(ptr: Long, din: Int): Long = nativePull(ptr, din)
    fun push(ptr: Long, din: Int, ddoPtr: Long): Int = nativePush(ptr, din, ddoPtr)
    fun availablesPull(ptr: Long, din: Int): Int = nativeAvailablesPull(ptr, din)
    fun addTypeset(ptr: Long, typesetCode: Int, typesetSize: Int): Int = nativeAddTypeset(ptr, typesetCode, typesetSize)

    fun frisbee(ptr: Long, din: Int): Int = nativeFrisbee(ptr, din)
    fun frisbeeICMP(ptr: Long, din: Int, timeout: Int, retry: Int): Int = nativeFrisbeeICMP(ptr, din, timeout, retry)
    fun frisbeeDPerf(ptr: Long, din: Int, senderTotal: Int = 10, blockSize: Int = 1024*1024, period: Int = 0): Int =
        nativeFrisbeeDPerf(ptr, din, senderTotal, blockSize, period)
    fun getFrisbeeDPerfResult(ptr: Long): Long = nativeGetFrisbeeDPerfResult(ptr)
}