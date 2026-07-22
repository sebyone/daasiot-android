import sebyone.daasiot_android.highlevel.Node
import java.io.File

fun createReceiver(configFile: File): Node = Node.fromJson(configFile).also { node ->
    node.events().on("ddoReceived") { event ->
        val din = (event.arguments[2] as Number).toLong()
        node.pull(din)?.let { println(it.payload.toString(Charsets.UTF_8)) }
    }
    node.start()
}
