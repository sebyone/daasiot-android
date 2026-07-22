package sebyone.daasiot_android.highlevel

import org.json.JSONObject
import java.io.File
import java.util.Properties

object ConfigLoader {
    @JvmStatic fun json(file: File): NodeConfig = fromJson(JSONObject(file.readText(Charsets.UTF_8)))

    @JvmStatic fun fromJson(value: JSONObject): NodeConfig {
        val drivers = value.optJSONArray("drivers")?.let { array ->
            (0 until array.length()).map { index -> array.getJSONObject(index).let { DriverConfig(it.getInt("link"), it.getString("uri")) } }
        } ?: emptyList()
        val mappings = value.optJSONArray("mappings")?.let { array ->
            (0 until array.length()).map { index -> array.getJSONObject(index).let {
                MappingConfig(it.getLong("din"), it.getInt("link"), it.getString("uri"), it.optString("security_key").ifBlank { null })
            } }
        } ?: emptyList()
        val optionsObject = value.optJSONObject("options")
        val options = optionsObject?.keys()?.asSequence()?.associate { it.toInt() to optionsObject.getLong(it) } ?: emptyMap()
        val features = value.optJSONArray("features")?.let { array -> (0 until array.length()).map(array::getInt) } ?: emptyList()
        val join = value.optJSONObject("join")?.let { JoinConfig(it.optLong("sid"), it.optInt("link"), it.optLong("timeout", 5000)) }
        return NodeConfig(value.getLong("sid"), value.getLong("din"), drivers, mappings, options,
            value.intOrNull("accept_requests"), value.intOrNull("discovery_state"), value.intOrNull("ddo_policy"),
            features, join, value.intOrNull("ats_max_error"), value.optInt("perform_mode", 0))
    }

    @JvmStatic fun properties(file: File): NodeConfig {
        val properties = Properties().apply { file.reader().use(::load) }
        val drivers = properties.getProperty("daas.drivers", "").split(',').filter(String::isNotBlank).map {
            val at = it.indexOf(':'); DriverConfig(it.substring(0, at).toInt(), it.substring(at + 1))
        }
        return NodeConfig(properties.getProperty("daas.sid").toLong(), properties.getProperty("daas.din").toLong(), drivers)
    }

    private fun JSONObject.intOrNull(name: String): Int? = if (has(name) && !isNull(name)) getInt(name) else null
}
