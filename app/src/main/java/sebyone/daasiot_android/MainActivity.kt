package sebyone.daasiot_android

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.TextView

class MainActivity : AppCompatActivity() {

    private var daasPtr: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Create the native DaasAPI instance
        daasPtr = DaasWrapper.create()

        // Call into native methods
        val version = DaasWrapper.getVersion(daasPtr)
        val drivers = DaasWrapper.listAvailableDrivers(daasPtr)

        // Debug logs
        Log.d("DaaS", "Version: $version")
        Log.d("DaaS", "Drivers: $drivers")

        // Update UI
        findViewById<TextView>(R.id.textView).text = "Version: $version\nDrivers: $drivers"
    }

    override fun onDestroy() {
        super.onDestroy()
        DaasWrapper.destroy(daasPtr)
    }
}