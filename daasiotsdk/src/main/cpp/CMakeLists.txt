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
message(STATUS "Current Android ABI: ${ANDROID_ABI}")

# ----------------------------
# Define the path for the static library based on ABI
# ----------------------------
set(DAAS_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdaas.a)

if(NOT EXISTS ${DAAS_LIB_PATH})
    message(FATAL_ERROR "libdaas.a not found for ABI: ${ANDROID_ABI} at ${DAAS_LIB_PATH}")
endif()

message(STATUS "Using libdaas.a from: ${DAAS_LIB_PATH}")

# ----------------------------
# Add JNI C++ Source Files
# ----------------------------
add_library(${LIB_NAME} SHARED
        daas_jni.cpp
)

# ----------------------------
# Create imported target for libdaas.a
# ----------------------------
add_library(daas STATIC IMPORTED)

set_target_properties(daas PROPERTIES
        IMPORTED_LOCATION ${DAAS_LIB_PATH}
)

# ----------------------------
# Compiler Settings
# ----------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Link everything
# ----------------------------
target_link_libraries(
        ${LIB_NAME}
        daas
        ${log-lib}
)