package sebyone.daasiot_android.entity

import sebyone.daasiot_android.enums.LinkType

/** Mirrors the native `node_map_entry_t` struct (daas_types.hpp): a mapped node's link/URI info. */
data class NodeMapEntry(
    val din: Long,
    val channelOwner: Long,
    val link: LinkType,
    val uri: String,
    val directChannel: Boolean
)
