package sebyone.daasiot_android.entity

/** Mirrors the native `node_network_info_t` struct (daas_types.hpp): a known node's SID/DIN pair. */
data class NodeNetworkInfo(val sid: Long, val din: Long)
