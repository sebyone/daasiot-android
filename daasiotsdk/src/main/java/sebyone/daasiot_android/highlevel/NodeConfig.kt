package sebyone.daasiot_android.highlevel

data class DriverConfig(val link: Int, val uri: String)
data class MappingConfig(val din: Long, val link: Int, val uri: String, val securityKey: String? = null)
data class JoinConfig(val sid: Long = 0, val link: Int = 0, val timeoutMillis: Long = 5000)

data class NodeConfig(
    val sid: Long,
    val din: Long,
    val drivers: List<DriverConfig> = emptyList(),
    val mappings: List<MappingConfig> = emptyList(),
    val options: Map<Int, Long> = emptyMap(),
    val acceptRequests: Int? = null,
    val discoveryState: Int? = null,
    val ddoPolicy: Int? = null,
    val features: List<Int> = emptyList(),
    val join: JoinConfig? = null,
    val atsMaxError: Int? = null,
    val performMode: Int = 0,
) {
    init {
        require(sid >= 0 && din >= 0) { "sid and din must be non-negative" }
        require(drivers.all { it.uri.isNotBlank() }) { "driver uri is required" }
        require(mappings.all { it.din >= 0 && it.uri.isNotBlank() }) { "mapping din and uri are required" }
    }
}
