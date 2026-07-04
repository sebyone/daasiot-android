package sebyone.daasiot_android.entity

import sebyone.daasiot_android.enums.AcceptRequestPolicy
import sebyone.daasiot_android.enums.DDOPolicy
import sebyone.daasiot_android.enums.DiscoveryState

/** Mirrors the native `node_info_t` struct (daas_types.hpp). */
data class NodeInfo(
    val powerOnTime: Long,
    val linked: Long,
    val lock: Long,
    val sklen: Int,
    val skey: ByteArray,
    val acceptRequestPolicy: AcceptRequestPolicy,
    val sid: Long,
    val din: Long,
    val ddoPolicy: DDOPolicy,
    val discoveryState: DiscoveryState,
    val networkOffset: Long,
    val networkInSync: Boolean
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is NodeInfo) return false
        return powerOnTime == other.powerOnTime &&
                linked == other.linked &&
                lock == other.lock &&
                sklen == other.sklen &&
                skey.contentEquals(other.skey) &&
                acceptRequestPolicy == other.acceptRequestPolicy &&
                sid == other.sid &&
                din == other.din &&
                ddoPolicy == other.ddoPolicy &&
                discoveryState == other.discoveryState &&
                networkOffset == other.networkOffset &&
                networkInSync == other.networkInSync
    }

    override fun hashCode(): Int {
        var result = powerOnTime.hashCode()
        result = 31 * result + linked.hashCode()
        result = 31 * result + lock.hashCode()
        result = 31 * result + sklen
        result = 31 * result + skey.contentHashCode()
        result = 31 * result + acceptRequestPolicy.hashCode()
        result = 31 * result + sid.hashCode()
        result = 31 * result + din.hashCode()
        result = 31 * result + ddoPolicy.hashCode()
        result = 31 * result + discoveryState.hashCode()
        result = 31 * result + networkOffset.hashCode()
        result = 31 * result + networkInSync.hashCode()
        return result
    }
}
