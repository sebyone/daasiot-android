# Daasiot-android

This project demonstrates how to integrate and use the **DaaS static library** inside an Android application.  
It provides a minimal working example using Android Studio, CMake, and Kotlin/Java to interact with native C++ code.

## **Project Structure**

```bash
app/
├── src/
│ ├── main/
│ │ ├── cpp/ # Native C++ source files
│ │ │ └── native-lib.cpp
│ │ ├── jniLibs/ # Prebuilt DaaS static libraries
│ │ │ ├── armeabi-v7a/
│ │ │ │ └── libdaas.a
│ │ │ ├── arm64-v8a/
│ │ │ │ └── libdaas.a
│ │ │ └── x86_64/
│ │ │ └── libdaas.a
│ │ └── res/layout/ # UI layout files
│ └── AndroidManifest.xml
└── build.gradle
```

## **Setup Instructions**

### 1. Clone the repository
```bash
git clone https://github.com/your-username/Daasiot-android.git
cd Daasiot-android
```

### 2. Open the project in Android Studio

- Launch Android Studio.
- Go to File > Open and select this project folder.

### 3. Configure the jniLibs folder

Ensure the prebuilt static libraries are correctly placed:
```bash
app/src/main/jniLibs/
    ├── armeabi-v7a/libdaas.a
    ├── arm64-v8a/libdaas.a
    └── x86_64/libdaas.a
```

### 4. Verify the CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.22.1)

project(daasiot_android)

# Prebuilt static library
add_library(daas STATIC IMPORTED)
set_target_properties(daas PROPERTIES
    IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdaas.a
)

# JNI shared library
add_library(native-lib SHARED native-lib.cpp)

# Link libraries
target_link_libraries(native-lib daas log)
```

### 5. Build and Run the Application

- Connect your device or launch an emulator.
- In Android Studio, click Run ▶.

If everything is set up correctly, the app will launch and display the following message on screen:
```cpp
DaaS static library linked successfully!
```

### Expected Output

When the app starts successfully, you should see:
> DaaS static library linked successfully!

This confirms:
- The DaaS .a static library was found and linked properly.
- JNI integration between C++ and Kotlin/Java is functioning.
- The CMake configuration is correct.