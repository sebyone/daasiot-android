import sebyone.daasiot_android.highlevel.Node
import java.io.File

fun runSender(configFile: File) {
    Node.fromJson(configFile).use { node ->
        node.start()
        node.locate(101)
        node.pushText(101, "Hello from the Android sender", 10)
    }
}
