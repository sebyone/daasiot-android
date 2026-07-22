package sebyone.daasiot_android.highlevel

import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.StreamType
import sebyone.daasiot_android.events.IDaasApiEvent
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.CopyOnWriteArrayList

data class DaasEvent(val name: String, val arguments: List<Any>)

class EventRouter : IDaasApiEvent {
    private val listeners = ConcurrentHashMap<String, CopyOnWriteArrayList<(DaasEvent) -> Unit>>()
    fun on(name: String, listener: (DaasEvent) -> Unit): AutoCloseable {
        listeners.computeIfAbsent(name) { CopyOnWriteArrayList() }.add(listener)
        return AutoCloseable { listeners[name]?.remove(listener) }
    }
    private fun emit(name: String, vararg values: Any) {
        val event = DaasEvent(name, values.toList())
        listeners[name]?.forEach { listener -> runCatching { listener(event) } }
    }
    override fun dinAccepted(din: Long) = emit("dinAccepted", din)
    override fun ddoReceived(payloadSize: Int, typeset: Int, din: Long) = emit("ddoReceived", payloadSize, typeset, din)
    override fun frisbeeReceived(din: Long) = emit("frisbeeReceived", din)
    override fun nodeStateReceived(din: Long) = emit("nodeStateReceived", din)
    override fun atsSyncCompleted(din: Long) = emit("atsSyncCompleted", din)
    override fun frisbeeDperfCompleted(din: Long, packetsSent: Long, blockSize: Long) = emit("frisbeeDperfCompleted", din, packetsSent, blockSize)
    override fun networkDiscovered(din: Long, sid: Long, link: LinkType) = emit("networkDiscovered", din, sid, link)
    override fun nodeConnectedToNetwork(sid: Long, din: Long) = emit("nodeConnectedToNetwork", sid, din)
    override fun streamInfoReceived(din: Long, packetType: StreamType, streamId: Long) = emit("streamInfoReceived", din, packetType, streamId)
}
