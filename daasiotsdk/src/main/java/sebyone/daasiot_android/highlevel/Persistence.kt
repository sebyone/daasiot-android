package sebyone.daasiot_android.highlevel

import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.util.ArrayDeque

data class InboxMessage(val din: Long, val typeset: Int, val origin: Long, val timestamp: Long, val payload: ByteArray, val receivedAt: Long = System.currentTimeMillis())

class Inbox(private val capacity: Int = 1000) {
    private val messages = ArrayDeque<InboxMessage>()
    @Synchronized fun add(message: InboxMessage) { while (messages.size >= capacity) messages.removeFirst(); messages.addLast(message) }
    @Synchronized fun messages(din: Long? = null): List<InboxMessage> = messages.filter { din == null || it.din == din }
    @Synchronized fun clear() = messages.clear()
}

object ProfileStore {
    @JvmStatic fun save(config: NodeConfig, file: File): File {
        val json = JSONObject().put("sid", config.sid).put("din", config.din)
            .put("drivers", JSONArray(config.drivers.map { JSONObject().put("link", it.link).put("uri", it.uri) }))
            .put("mappings", JSONArray(config.mappings.map { mapping -> JSONObject().put("din", mapping.din).put("link", mapping.link).put("uri", mapping.uri).also { if (mapping.securityKey != null) it.put("security_key", mapping.securityKey) } }))
            .put("options", JSONObject(config.options.mapKeys { it.key.toString() }))
            .put("features", JSONArray(config.features)).put("perform_mode", config.performMode)
        file.parentFile?.mkdirs(); val temporary = File(file.path + ".tmp"); temporary.writeText(json.toString(2), Charsets.UTF_8)
        check(temporary.renameTo(file) || run { temporary.copyTo(file, overwrite = true); temporary.delete() }) { "Cannot replace ${file.path}" }
        return file
    }
}
