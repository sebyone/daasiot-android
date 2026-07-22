package sebyone.daasiot_android.entity

/** Result counters and timestamps produced by the latest DPERF run. */
data class DperfInfoResult(
    val senderFirstTimestamp: Long,
    val localEndTimestamp: Long,
    val remoteFirstTimestamp: Long,
    val remoteLastTimestamp: Long,
    val remotePacketCounter: Long,
    val remoteDataCounter: Long
)
