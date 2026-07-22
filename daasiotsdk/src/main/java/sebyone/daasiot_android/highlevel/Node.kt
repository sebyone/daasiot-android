package sebyone.daasiot_android.highlevel

import org.json.JSONObject
import sebyone.daasiot_android.DaasInstance
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.enums.*
import java.io.Closeable
import java.io.File
import java.nio.charset.Charset
import kotlin.concurrent.fixedRateTimer

class Node(val config: NodeConfig) : Closeable {
    private val events = EventRouter()
    private val instance = DaasInstance.create(events, "android-highlevel")
    private val handle get() = instance.handle
    private var running = false
    private var stateTimer: java.util.Timer? = null
    private var inbox: Inbox? = null

    init { applyConfig() }
    fun events(): EventRouter = events
    private fun check(result: DaasError, operation: String) { check(result == DaasError.ERROR_NONE) { "$operation failed: $result" } }
    private fun applyConfig() {
        check(DaasWrapper.doInit(handle, config.sid, config.din), "doInit")
        config.drivers.forEach { check(DaasWrapper.enableDriver(handle, LinkType.fromValue(it.link), it.uri), "enableDriver") }
        config.options.forEach { (option, value) -> check(DaasWrapper.setOptions(handle, OptionType.fromValue(option), value), "setOptions") }
        config.acceptRequests?.let { DaasWrapper.setAcceptRequestsLevel(handle, AcceptRequestPolicy.fromValue(it)) }
        config.discoveryState?.let { DaasWrapper.setDiscoveryState(handle, DiscoveryState.fromValue(it)) }
        config.ddoPolicy?.let { check(DaasWrapper.setDDOPolicy(handle, DDOPolicy.fromValue(it)), "setDDOPolicy") }
        config.atsMaxError?.let { DaasWrapper.setATSMaxError(handle, it) }
        config.features.forEach { check(DaasWrapper.addNodeFeatures(handle, Feature.fromValue(it)), "addNodeFeatures") }
        config.mappings.forEach { mapping -> check(DaasWrapper.map(handle, mapping.din, LinkType.fromValue(mapping.link), mapping.uri, mapping.securityKey), "map") }
        config.join?.let { check(DaasWrapper.join(handle, it.sid, LinkType.fromValue(it.link), it.timeoutMillis), "join") }
    }
    fun start(): Node { if (!running) { check(DaasWrapper.doPerform(handle, PerformMode.fromValue(config.performMode)), "doPerform"); running = true }; return this }
    fun stop() { stateTimer?.cancel(); stateTimer = null; if (running) { check(DaasWrapper.doEnd(handle), "doEnd"); running = false } }
    fun reset() = check(DaasWrapper.reset(handle), "reset")
    fun pushBytes(din: Long, data: ByteArray, typeset: Int = 1) = DDO(typeset).use { it.setPayload(data); check(DaasWrapper.push(handle, din, it), "push") }
    fun pushText(din: Long, text: String, typeset: Int = 1, charset: Charset = Charsets.UTF_8) = pushBytes(din, text.toByteArray(charset), typeset)
    fun pull(din: Long): InboxMessage? { val result = DaasWrapper.pull(handle, din); if (result.error != DaasError.ERROR_NONE) return null; return result.ddo?.use { InboxMessage(din, it.getTypeset(), it.getOrigin(), it.getTimestamp(), it.getPayload()) } }
    fun map(din: Long, link: LinkType, uri: String, key: String? = null) = DaasWrapper.map(handle, din, link, uri, key)
    fun unmap(din: Long) = DaasWrapper.remove(handle, din)
    fun locate(din: Long) = DaasWrapper.locate(handle, din)
    fun discovery(link: LinkType = LinkType.LINK_NONE) = DaasWrapper.discovery(handle, link)
    fun listNodes() = DaasWrapper.listNodes(handle).toList()
    fun nodeList() = DaasWrapper.getNodeList(handle).toList()
    fun addFeature(feature: Feature) = DaasWrapper.addNodeFeatures(handle, feature)
    fun features() = DaasWrapper.getNodeFeatures(handle).toList()
    fun hasFeature(din: Long, feature: Feature) = DaasWrapper.nodeHasFeature(handle, din, feature)
    fun frisbee(din: Long) = DaasWrapper.frisbee(handle, din)
    fun frisbeeIcmp(din: Long, timeoutMillis: Long = 1000, retry: Long = 3) = DaasWrapper.frisbeeICMP(handle, din, timeoutMillis, retry)
    fun frisbeeDperf(din: Long, packets: Long = 10, blockSize: Long = 1024 * 1024, period: Long = 0) = DaasWrapper.frisbeeDPERF(handle, din, packets, blockSize, period)
    fun saveProfile(file: File) = ProfileStore.save(config, file)
    fun storeConfiguration(directory: File) = DaasWrapper.storeConfiguration(handle, directory)
    fun loadConfiguration(directory: File) = DaasWrapper.loadConfiguration(handle, directory)
    fun enableInbox(capacity: Int = 1000): Inbox { val target = Inbox(capacity); inbox = target; events.on("ddoReceived") { event -> pull((event.arguments[2] as Number).toLong())?.let(target::add) }; return target }
    fun saveState(file: File) { val value = JSONObject().put("sid", config.sid).put("din", config.din).put("saved_at", System.currentTimeMillis()); file.parentFile?.mkdirs(); file.writeText(value.toString(2)) }
    fun enableStateAutosave(file: File, periodMillis: Long = 30_000): Node { stateTimer?.cancel(); stateTimer = fixedRateTimer("daas-state", daemon = true, period = periodMillis) { runCatching { saveState(file) } }; return this }
    override fun close() { try { stop() } finally { instance.close() } }

    companion object { @JvmStatic fun fromJson(file: File) = Node(ConfigLoader.json(file)) }
}
