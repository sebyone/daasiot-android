package sebyone.daasiot_android

object DaasWrapper {

    init {
        // Load the native library compiled by CMake
        System.loadLibrary("daas_jni")
    }

    // --- Native JNI methods ---
    private external fun nativeCreate(): Long
    private external fun nativeDestroy(ptr: Long)
    private external fun nativeGetVersion(ptr: Long): String
    private external fun nativeListAvailableDrivers(ptr: Long): String

    // --- Public API ---
    fun create(): Long {
        return nativeCreate()
    }

    fun destroy(ptr: Long) {
        nativeDestroy(ptr)
    }

    fun getVersion(ptr: Long): String {
        return nativeGetVersion(ptr)
    }

    fun listAvailableDrivers(ptr: Long): String {
        return nativeListAvailableDrivers(ptr)
    }
}