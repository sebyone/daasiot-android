# daas © 2019–2026 Sebyone Srl
#
# File: CMakeLists.txt — android_daas (daasiot-android)
#
# android_daas: the C++17 wrapper of libdaas for Android (daas::android_daas, static) and the shared library of its
# Java binding (android_daas_jni, loaded by it.sebyone.daas.DaasNode).
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-23  Lorenzo Grillo  creation (from the CMakeLists.txt of macos_daas of daasiot-macos 0.1.0)

add_library(android_daas STATIC
    src/android_daas.cpp
    src/android_defaults.cpp
    src/android_events.cpp
    src/android_file_depot.cpp)
add_library(daas::android_daas ALIAS android_daas)
target_include_directories(android_daas PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(android_daas PUBLIC libdaas::libdaas)
if(TARGET daas_drv_ble)
    # daasiot_android/ble.h and DAASIOT_ANDROID_BLE only: the archive itself is linked by libdaas::libdaas, in the
    # slot order (before libdaas_nodrivers); linking it here too would move it after the nodriver of `ble`.
    target_include_directories(android_daas PRIVATE $<TARGET_PROPERTY:daas_drv_ble,INTERFACE_INCLUDE_DIRECTORIES>)
    target_compile_definitions(android_daas PRIVATE $<TARGET_PROPERTY:daas_drv_ble,INTERFACE_COMPILE_DEFINITIONS>)
endif()
target_compile_definitions(android_daas PUBLIC ANDROID_DAAS_VERSION="${PROJECT_VERSION}")
set_target_properties(android_daas PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON)
target_compile_options(android_daas PRIVATE -Wall -Wextra)

# The defaults of the queues, of libdaas and of the persistence (include/android_defaults.hpp) from a file of the
# project: -DANDROID_DAAS_USER_CONFIG=/path/my_config.hpp, a header that #defines some ANDROID_DAAS_* macros.
set(ANDROID_DAAS_USER_CONFIG "" CACHE FILEPATH "header that overrides the ANDROID_DAAS_* defaults of android_defaults.hpp")
if(ANDROID_DAAS_USER_CONFIG)
    if(NOT EXISTS "${ANDROID_DAAS_USER_CONFIG}")
        message(FATAL_ERROR "ANDROID_DAAS_USER_CONFIG: ${ANDROID_DAAS_USER_CONFIG} not found")
    endif()
    target_compile_definitions(android_daas PUBLIC ANDROID_DAAS_USER_CONFIG="${ANDROID_DAAS_USER_CONFIG}")
endif()

# The Java binding: one shared library with android_daas, libdaas, its drivers and blew.
add_library(android_daas_jni SHARED jni/android_daas_jni.cpp)
target_link_libraries(android_daas_jni PRIVATE android_daas)
set_target_properties(android_daas_jni PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_options(android_daas_jni PRIVATE -Wall -Wextra)
target_link_options(android_daas_jni PRIVATE -Wl,--gc-sections)   # drop what libdaas does not use
