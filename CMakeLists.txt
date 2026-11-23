# daas © 2019–2026 Sebyone Srl
#
# File: CMakeLists.txt — daasiot-android
#
# daasiot-android: daas (Device as a Service) for Android. Builds, against the libdaas package that
# tools/fetch_ld/fetch_ld.sh installs from the catalog of daasiot.com (or DAAS_ROOT):
#
#   daas::drv_ble           the BLE driver of the android-drivers_sdk (blew), slot `ble` of _LINK_BLE
#   daas::android_daas      android_daas, the C++17 wrapper of libdaas for Android (daas::sdk::Node)
#   android_daas_jni        the shared library of the Java binding it.sebyone.daas.DaasNode (the AAR, or the
#                           android_daas module of a Gradle build, packages it)
#
# The Gradle module android_daas/ (build.gradle.kts) configures this project through externalNativeBuild; a native
# application can also add it with add_subdirectory(<daasiot-android> daasiot-android) and link daas::android_daas.
#
#   DAASIOT_ANDROID_BLE     build the BLE driver (ON)
#   DAAS_DRIVERS            generic drivers of the package (default: every driver found, inet4;inet6)
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-23  Lorenzo Grillo  creation (0.1.0, from the CMakeLists.txt of daasiot-macos 0.1.0)
#   2026-11-23  Lorenzo Grillo  0.2.0: android_daas and its JNI binding, platforms/libdaas/libdaas.cmake

cmake_minimum_required(VERSION 3.22)

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" DAASIOT_ANDROID_VERSION LIMIT_COUNT 1)
project(daasiot-android VERSION ${DAASIOT_ANDROID_VERSION} LANGUAGES C CXX)

option(DAASIOT_ANDROID_BLE "build the BLE driver of the android-drivers_sdk (blew)" ON)

if(DAASIOT_ANDROID_BLE)
    list(APPEND DAAS_EXTRA_DRIVERS daas_drv_ble)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/platforms/libdaas/libdaas.cmake)
message(STATUS "daasiot-android ${DAASIOT_ANDROID_VERSION}: libdaas ${DAAS_VERSION} (${DAAS_TARGET})")

if(DAASIOT_ANDROID_BLE)
    add_subdirectory(android-drivers_sdk)
endif()
add_subdirectory(android_daas)
