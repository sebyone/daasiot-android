# Minimum version required
cmake_minimum_required(VERSION 3.22.1)

# Name of the project
project(daasiot_android LANGUAGES C CXX)

# ----------------------------
# Android log library
# ----------------------------
find_library(log-lib log)

# ----------------------------
# Library Name
# ----------------------------
set(LIB_NAME daas_jni)

# ----------------------------
# Include directories
# ----------------------------
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/include
)

# ----------------------------
# Detect Android ABI
# ----------------------------
# message(STATUS "Current Android ABI: ${ANDROID_ABI}")

# ----------------------------
# Define the path for the static library based on ABI
# ----------------------------
# set(DAAS_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdaas.a)
set(DAAS_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/arm64-v8a/libdaas.a)

# libdaas.a now statically links a BLE driver (drv_ble.cpp) that depends on SimpleBLE
# (https://github.com/OpenBluetoothToolbox/SimpleBLE). See PROVENANCE.md for where this came from.
set(SIMPLEBLE_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/arm64-v8a/libsimpleble.a)

# ----------------------------
# Force build only for arm64-v8a
# ----------------------------
set(ANDROID_ABI arm64-v8a)

if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "This library supports only arm64-v8a")
endif()

# if(NOT EXISTS ${DAAS_LIB_PATH})
#     message(FATAL_ERROR "libdaas.a not found for ABI: ${ANDROID_ABI} at ${DAAS_LIB_PATH}")
# endif()

# message(STATUS "Using libdaas.a from: ${DAAS_LIB_PATH}")

# ----------------------------
# Add JNI C++ Source Files
# ----------------------------
add_library(${LIB_NAME} SHARED
        control/JavaBridge.cpp
        control/JniHelpers.cpp
        control/DaasAPINative.cpp
        events/DaasEvents.cpp
        transfer/DDONative.cpp
)

# ----------------------------
# Create imported target for libdaas.a
# ----------------------------
add_library(daas STATIC IMPORTED)

set_target_properties(daas PROPERTIES
        IMPORTED_LOCATION ${DAAS_LIB_PATH}
)

# ----------------------------
# Create imported target for libsimpleble.a
# ----------------------------
add_library(simpleble STATIC IMPORTED)

set_target_properties(simpleble PROPERTIES
        IMPORTED_LOCATION ${SIMPLEBLE_LIB_PATH}
)

# -------------------------------------------------
# C++ standard (REQUIRED by DaaS R0.21.3)
# -------------------------------------------------
set(CMAKE_C_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_NATIVE_API_LEVEL 30)

# ----------------------------
# Link everything
# ----------------------------
target_link_libraries(
        ${LIB_NAME}
        daas
        simpleble
        ${log-lib}
)
