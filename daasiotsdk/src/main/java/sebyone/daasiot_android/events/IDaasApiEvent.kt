package sebyone.daasiot_android.events

import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.StreamType

/**
 * Mirrors the native IDaasApiEvent interface (daas_types.hpp). Implement this and pass it to
 * [DaasWrapper.create] to receive DaaS API events on this specific instance; the native bridge
 * forwards each event straight through without any implicit side effects (e.g. it does not
 * auto-pull on [ddoReceived] — call [DaasWrapper.pull] yourself if you need the payload).
 */
interface IDaasApiEvent {
    fun dinAccepted(din: Long)
    fun ddoReceived(payloadSize: Int, typeset: Int, din: Long)
    fun frisbeeReceived(din: Long)
    fun nodeStateReceived(din: Long)
    fun atsSyncCompleted(din: Long)
    fun frisbeeDperfCompleted(din: Long, packetsSent: Long, blockSize: Long)
    fun networkDiscovered(din: Long, sid: Long, link: LinkType)
    fun nodeConnectedToNetwork(sid: Long, din: Long)
    fun streamInfoReceived(din: Long, packetType: StreamType, streamId: Long)
}
