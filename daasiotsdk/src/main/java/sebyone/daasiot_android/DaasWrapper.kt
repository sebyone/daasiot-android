package sebyone.daasiot_android

import sebyone.daasiot_android.entity.AvailablePullResult
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.entity.FeatureRequest
import sebyone.daasiot_android.entity.NodeInfo
import sebyone.daasiot_android.entity.NodeMapEntry
import sebyone.daasiot_android.entity.NodeNetworkInfo
import sebyone.daasiot_android.entity.PullResult
import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.DiscoveryState
import sebyone.daasiot_android.enums.Feature
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.OptionType
import sebyone.daasiot_android.enums.PerformMode
import sebyone.daasiot_android.events.IDaasApiEvent

object DaasWrapper {

    init {
        System.loadLibrary("daas_jni")
    }

    // --- Native JNI Methods ---
    private external fun nativeCreate(eventHandler: IDaasApiEvent?, lhver: String?): Long
    private external fun nativeDestroy(ptr: Long)

    private external fun nativeGetVersion(ptr: Long): String
    private external fun nativeGetBuildInfo(ptr: Long): String
    private external fun nativeListAvailableDrivers(ptr: Long): String

    private external fun nativeInit(ptr: Long, sid: Long, din: Long): DaasError
    private external fun nativeDoEnd(ptr: Long): DaasError
    private external fun nativeReset(ptr: Long): DaasError
    private external fun nativeDoPerform(ptr: Long, mode: PerformMode): DaasError

    private external fun nativeEnableDriver(ptr: Long, driver: LinkType, localUri: String): DaasError
    private external fun nativeGetStatusCopy(ptr: Long): NodeInfo
    private external fun nativeSetAcceptRequestsLevel(ptr: Long, policyLevel: AcceptRequestPolicy)

    private external fun nativeStoreConfiguration(ptr: Long, depotPtr: Long): Boolean
    private external fun nativeLoadConfiguration(ptr: Long, depotPtr: Long): Boolean
    private external fun nativeDoStatisticsReset(ptr: Long): Boolean
    private external fun nativeGetSystemStatistics(ptr: Long, code: Int): Long

    private external fun nativeMap(ptr: Long, din: Long, link: LinkType, uri: String?, skey: String?): DaasError
    private external fun nativeRemoveNode(ptr: Long, din: Long): DaasError
    private external fun nativeDiscoveryByLink(ptr: Long, link: LinkType): DaasError
    private external fun nativeDiscoveryBySid(ptr: Long, sid: Long): DaasError
    private external fun nativeJoin(ptr: Long, sid: Long, link: LinkType, timeoutMillis: Long): DaasError
    private external fun nativeSetDiscoveryState(ptr: Long, mode: DiscoveryState)
    private external fun nativeListNodesCopy(ptr: Long): Array<NodeNetworkInfo>
    private external fun nativeGetNodeList(ptr: Long): Array<NodeMapEntry>
    private external fun nativeGetAllNodes(ptr: Long, sid: Long): LongArray
    private external fun nativeLocate(ptr: Long, din: Long): DaasError
    private external fun nativeLocateFeature(ptr: Long, feature: Feature, timeoutMillis: Int, ttl: Int): DaasError

    private external fun nativeSendStatus(ptr: Long, din: Long): DaasError
    private external fun nativeStatusCopy(ptr: Long, din: Long): NodeInfo
    private external fun nativeFetchCopy(ptr: Long, din: Long, opts: Int): NodeInfo
    private external fun nativeGetSyncedTimestamp(ptr: Long): Long

/*    private external fun nativeUnlock(ptr: Long, din: Long, skey: String): NodeInfo
    private external fun nativeLock(ptr: Long, skey: String, policy: Int): NodeInfo*/

    private external fun nativeSyncNode(ptr: Long, din: Long, timezone: Int): NodeInfo
    private external fun nativeSyncNet(ptr: Long, din: Long, bubbleTime: Int): NodeInfo
//    private external fun nativeSetATSMaxError(ptr: Long, error: Int)

    private external fun nativeUse(ptr: Long, din: Long): Boolean
    private external fun nativeEnd(ptr: Long, din: Long): Boolean
    private external fun nativeSend(ptr: Long, din: Long, data: ByteArray): Int
    private external fun nativeReceived(ptr: Long, din: Long): Int
    private external fun nativeReceive(ptr: Long, din: Long): ByteArray

    // TODO: Implement this function when possible.
    // private external fun nativeListTypesetsCopy(ptr: Long): IntArray
    private external fun nativePull(ptr: Long, din: Long): PullResult
    private external fun nativePush(ptr: Long, din: Long, ddoHandle: Long): DaasError
    private external fun nativeAvailablesPull(ptr: Long, din: Long): AvailablePullResult
    private external fun nativeAddTypeset(ptr: Long, typesetCode: Int): DaasError

    private external fun nativeFrisbee(ptr: Long, din: Long): DaasError

    private external fun nativeSetDDOPolicy(ptr: Long, policy: DDOPolicy): DaasError
    private external fun nativeCreateNetwork(ptr: Long): DaasError
    private external fun nativeUnbindNetwork(ptr: Long): DaasError
    private external fun nativeSetOptions(ptr: Long, option: OptionType, value: Long): DaasError
    private external fun nativeErrorToString(ptr: Long, error: DaasError): String

    private external fun nativeAddNodeFeatures(ptr: Long, feature: Feature): DaasError
    private external fun nativeGetNodeFeatures(ptr: Long): Array<Feature>
    private external fun nativeNodeHasFeature(ptr: Long, din: Long, feature: Feature): Boolean
    private external fun nativeRequestFeature(ptr: Long, din: Long, request: FeatureRequest, timeoutMillis: Long): DaasError

//    TODO: Implement when ready
//    private external fun nativeFrisbeeICMP(ptr: Long, din: Int, timeout: Int, retry: Int): Int
//    private external fun nativeFrisbeeDPerf(ptr: Long, din: Int, senderTotal: Int, blockSize: Int, period: Int): Int
//    private external fun nativeGetFrisbeeDPerfResult(ptr: Long): Long

    // --- Public Kotlin API ---

    /**
     * Creates a new, independent native DaaS instance and returns its handle. Pass the returned
     * `ptr` to every other function in this object. Call [destroy] when done, or wrap the result
     * in a [DaasInstance] for a [java.io.Closeable] handle with a GC-triggered safety net.
     *
     * @param eventHandler optional per-instance listener for node/DDO events (see [IDaasApiEvent]).
     * @param lhver optional local host version string override.
     */
    @JvmOverloads
    fun create(eventHandler: IDaasApiEvent? = null, lhver: String? = null): Long = nativeCreate(eventHandler, lhver)

    /** Releases the native instance created by [create]. `ptr` must not be used again afterwards. */
    fun destroy(ptr: Long) = nativeDestroy(ptr)

    fun getVersion(ptr: Long): String = nativeGetVersion(ptr)
    fun getBuildInfo(ptr: Long): String = nativeGetBuildInfo(ptr)
    fun listAvailableDrivers(ptr: Long): String = nativeListAvailableDrivers(ptr)

    /** Initializes services and resources for the local node identified by [sid]/[din]. */
    fun doInit(ptr: Long, sid: Long, din: Long): DaasError = nativeInit(ptr, sid, din)

    /** Releases resources and deactivates the node (the instance handle itself stays valid). */
    fun doEnd(ptr: Long): DaasError = nativeDoEnd(ptr)

    /** Resets the local node and clears all resources, disconnecting from its current network. */
    fun reset(ptr: Long): DaasError = nativeReset(ptr)

    /** Runs the node's task; call cyclically in [PerformMode.PERFORM_CORE_NO_THREAD] mode. */
    fun doPerform(ptr: Long, mode: PerformMode): DaasError = nativeDoPerform(ptr, mode)

    /** Enables a communication driver (e.g. [LinkType.LINK_INET4]) at the given local address. */
    fun enableDriver(ptr: Long, driver: LinkType, localUri: String): DaasError = nativeEnableDriver(ptr, driver, localUri)

    /** Returns the local node's own status. */
    fun getStatus(ptr: Long): NodeInfo = nativeGetStatusCopy(ptr)

    /** Configures which incoming requests the local node accepts. */
    fun setAcceptRequestsLevel(ptr: Long, policyLevel: AcceptRequestPolicy) = nativeSetAcceptRequestsLevel(ptr, policyLevel)

    fun storeConfiguration(ptr: Long, depotPtr: Long): Boolean = nativeStoreConfiguration(ptr, depotPtr)
    fun loadConfiguration(ptr: Long, depotPtr: Long): Boolean = nativeLoadConfiguration(ptr, depotPtr)
    fun doStatisticsReset(ptr: Long): Boolean = nativeDoStatisticsReset(ptr)
    fun getSystemStatistics(ptr: Long, code: Int): Long = nativeGetSystemStatistics(ptr, code)

    /** Maps a remote node ([din]) to the local instance so it can be communicated with. */
    @JvmOverloads
    fun map(ptr: Long, din: Long, link: LinkType, uri: String? = null, skey: String? = null): DaasError =
        nativeMap(ptr, din, link, uri, skey)

    /** Removes a previously [map]ped node and cleans up associated resources. */
    fun remove(ptr: Long, din: Long): DaasError = nativeRemoveNode(ptr, din)

    /** Starts discovery for nearby DaaS networks over [link] (all available links if unset). */
    @JvmOverloads
    fun discovery(ptr: Long, link: LinkType = LinkType.LINK_NONE): DaasError = nativeDiscoveryByLink(ptr, link)

    /** Starts discovery for the DaaS network identified by [sid]. */
    fun discovery(ptr: Long, sid: Long): DaasError = nativeDiscoveryBySid(ptr, sid)

    /** Blocking call: joins network [sid] (or the first one discovered, if `sid == 0`). */
    @JvmOverloads
    fun join(ptr: Long, sid: Long = 0, link: LinkType = LinkType.LINK_NONE, timeoutMillis: Long = 5000): DaasError =
        nativeJoin(ptr, sid, link, timeoutMillis)

    /** Configures whether/how the node seeks and hosts networks during discovery. */
    fun setDiscoveryState(ptr: Long, mode: DiscoveryState) = nativeSetDiscoveryState(ptr, mode)

    /** Returns the known nodes (SID/DIN pairs) currently mapped to this instance. */
    fun listNodes(ptr: Long): Array<NodeNetworkInfo> = nativeListNodesCopy(ptr)

    /** Returns mapped nodes with their link/URI/channel information. */
    fun getNodeList(ptr: Long): Array<NodeMapEntry> = nativeGetNodeList(ptr)

    /** Returns every node currently mapped under network [sid]. */
    fun getAllNodes(ptr: Long, sid: Long): LongArray = nativeGetAllNodes(ptr, sid)

    /** Blocking call: locates [din] on the network if it isn't already known. */
    fun locate(ptr: Long, din: Long): DaasError = nativeLocate(ptr, din)

    /** Blocking call: locates any node advertising [feature] on the network. */
    @JvmOverloads
    fun locate(ptr: Long, feature: Feature, timeoutMillis: Int = 1000, ttl: Int = 10): DaasError =
        nativeLocateFeature(ptr, feature, timeoutMillis, ttl)

    /** Sends this node's status to remote node [din]. */
    fun sendStatus(ptr: Long, din: Long): DaasError = nativeSendStatus(ptr, din)

    /** Returns the last known status of remote node [din] (does not query the network). */
    fun status(ptr: Long, din: Long): NodeInfo = nativeStatusCopy(ptr, din)

    /** Fetches remote node [din]'s current status over the network and updates the local cache. */
    fun fetch(ptr: Long, din: Long, opts: Int): NodeInfo = nativeFetchCopy(ptr, din, opts)

    /** Returns the local node's ATS-corrected timestamp (falls back to local time if not synced). */
    fun getSyncedTimestamp(ptr: Long): Long = nativeGetSyncedTimestamp(ptr)

/*    fun unlock(ptr: Long, din: Long, skey: String): NodeInfo = nativeUnlock(ptr, din, skey)
    fun lock(ptr: Long, skey: String, policy: Int): NodeInfo = nativeLock(ptr, skey, policy)*/

    /** Sets the local system time on remote node [din] and synchronizes ATS. Not yet implemented upstream. */
    fun syncNode(ptr: Long, din: Long, timezone: Int): NodeInfo = nativeSyncNode(ptr, din, timezone)

    /** Sets the local system time on remote node [din] and synchronizes ATS. Not yet implemented upstream. */
    fun syncNet(ptr: Long, din: Long, bubbleTime: Int): NodeInfo = nativeSyncNet(ptr, din, bubbleTime)
//    fun setATSMaxError(ptr: Long, error: Int) = nativeSetATSMaxError(ptr, error)

    /** Blocking call: opens a real-time session with remote node [din]. */
    fun use(ptr: Long, din: Long): Boolean = nativeUse(ptr, din)

    /** Ends a real-time session previously opened with [use]. */
    fun end(ptr: Long, din: Long): Boolean = nativeEnd(ptr, din)

    /** Sends [data] to remote node [din] within an open real-time session; returns bytes sent. */
    fun send(ptr: Long, din: Long, data: ByteArray): Int = nativeSend(ptr, din, data)

    /** Returns how many bytes are available to [receive] from remote node [din]. */
    fun received(ptr: Long, din: Long): Int = nativeReceived(ptr, din)

    /** Receives data from remote node [din] within an open real-time session. */
    fun receive(ptr: Long, din: Long): ByteArray = nativeReceive(ptr, din)

    // TODO: Implement when ready
    // fun listTypesets(ptr: Long): IntArray = nativeListTypesetsCopy(ptr)

    /** Pulls one pending [DDO] from remote node [din], if any (see [PullResult]). */
    fun pull(ptr: Long, din: Long): PullResult = nativePull(ptr, din)

    /** Pushes [ddo] to remote node [din]. */
    fun push(ptr: Long, din: Long, ddo: DDO): DaasError = nativePush(ptr, din, ddo.nativeHandle())

    /** Checks how many DDOs are pending from remote node [din] without pulling them. */
    fun availablesPull(ptr: Long, din: Long): AvailablePullResult = nativeAvailablesPull(ptr, din)

    /** Registers a typeset code so DDOs carrying it are recognized (delivered via [IDaasApiEvent.ddoReceived]). */
    fun addTypeset(ptr: Long, typesetCode: Int): DaasError = nativeAddTypeset(ptr, typesetCode)

    /** Pings remote node [din] to check its availability. */
    fun frisbee(ptr: Long, din: Long): DaasError = nativeFrisbee(ptr, din)
//    fun frisbeeICMP(ptr: Long, din: Int, timeout: Int, retry: Int): Int = nativeFrisbeeICMP(ptr, din, timeout, retry)
//    fun frisbeeDPerf(ptr: Long, din: Int, senderTotal: Int = 10, blockSize: Int = 1024*1024, period: Int = 0): Int =
//        nativeFrisbeeDPerf(ptr, din, senderTotal, blockSize, period)
//    fun getFrisbeeDPerfResult(ptr: Long): Long = nativeGetFrisbeeDPerfResult(ptr)

    /** Sets how the node handles DDOs that fail to send. */
    fun setDDOPolicy(ptr: Long, policy: DDOPolicy): DaasError = nativeSetDDOPolicy(ptr, policy)

    /** Unbinds from any existing network and initializes a brand-new one with the current config. */
    fun createNetwork(ptr: Long): DaasError = nativeCreateNetwork(ptr)

    /** Unbinds from the current network, keeping the node mapping table intact for later rebinding. */
    fun unbindNetwork(ptr: Long): DaasError = nativeUnbindNetwork(ptr)

    /** Tunes a low-level API parameter (buffer sizes, retry counts, timeouts - see [OptionType]). */
    fun setOptions(ptr: Long, option: OptionType, value: Long): DaasError = nativeSetOptions(ptr, option, value)

    /** Converts [error] to a human-readable description. */
    fun errorToString(ptr: Long, error: DaasError): String = nativeErrorToString(ptr, error)

    /** Advertises [feature] as supported by the local node. */
    fun addNodeFeatures(ptr: Long, feature: Feature): DaasError = nativeAddNodeFeatures(ptr, feature)

    /** Returns the features currently advertised by the local node. */
    fun getNodeFeatures(ptr: Long): Array<Feature> = nativeGetNodeFeatures(ptr)

    /** Checks whether remote node [din] advertises [feature]. */
    fun nodeHasFeature(ptr: Long, din: Long, feature: Feature): Boolean = nativeNodeHasFeature(ptr, din, feature)

    /**
     * Requests [feature] from remote node [din].
     *
     * Always returns [DaasError.ERROR_NOT_IMPLEMENTED] today: `DaasAPI::requestFeature` is
     * declared in daas.hpp but is not currently an exported symbol in the linked `libdaas.a` for
     * this target.
     */
    @JvmOverloads
    fun requestFeature(ptr: Long, din: Long, request: FeatureRequest, timeoutMillis: Long = 1000): DaasError =
        nativeRequestFeature(ptr, din, request, timeoutMillis)
}