package sebyone.daasiot_android

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import sebyone.daasiot_android.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()
    }

    /**
     * A native method that is implemented by the 'daasiot_android' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'daasiot_android' library on application startup.
        init {
            System.loadLibrary("daasiot_android")
        }
    }
}