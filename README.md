# Daas IoT Android SDK (`sebyone.daasiot_android`)

The **Daas IoT Android SDK** provides a Kotlin interface to the native **Daas** (`daas_jni`) library, enabling Android applications to interact with Daas IoT systems directly through a clean, type-safe API.

This wrapper exposes core functionalities such as initialization, configuration management, node synchronization, and data operations — all powered by JNI bindings.

## 🧱 Requirements

1. Android API level 24+
2. Kotlin 1.9+

## 📦 Installation

1. Put the file in the app module’s libs/ folder.
2. Add the dependency to your project’s `build.gradle.kts`:

```kotlin
dependencies {
    implementation(files("libs/daasiotsdk-0.1.0-release.aar"))
}
```

## Usage Example

```kotlin
import sebyone.daasiot_android.DaasWrapper

fun main() {
    // Create a Daas instance
    val ptr = DaasWrapper.create()

    // Get version
    val version = DaasWrapper.getVersion(ptr)
    println("Daas version: $version")

    // Perform some action
    DaasWrapper.doInit(ptr, sid = 1, din = 100)

    // Clean up
    DaasWrapper.doEnd(ptr)
    DaasWrapper.destroy(ptr)
}
```

## Notes

- Supported ABIs: armeabi-v7a, arm64-v8a, x86_64.

## License

This project is licensed under the Mozilla Public License 2.0 - see the [LICENSE](LICENSE) file for details.

### MPL-2.0 License Summary

- ✅ Commercial use
- ✅ Modification
- ✅ Distribution
- ✅ Patent use
- ✅ Private use
- ❗ License and copyright notice required
- ❗ Disclose source (for modified files)
- ❗ Same license (for modified files)
- ❌ Liability
- ❌ Warranty
- ❌ Trademark use
