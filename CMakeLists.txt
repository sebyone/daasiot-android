# daas © 2019–2026 Sebyone Srl
#
# File: CMakeLists.txt — daasiot-android
#
# daasiot-android: daas (Device as a Service) for Android (NDK). Gives the application, from the libdaas package of
# libdaas/ and the drivers of android-drivers_sdk:
#
#   daas::libdaas   libdaas (static) with its drivers, in the order the package wants: libdaas.a, the BLE driver,
#                   the generic drivers (INET4, INET6), then libdaas_nodrivers.a for the empty slots.
#   daas::drv_ble   the BLE driver on blew (also linked by daas::libdaas when DAASIOT_ANDROID_BLE is ON).
#
# Used from the CMakeLists.txt of an Android app (externalNativeBuild):
#
#   add_subdirectory(<path>/daasiot-android daasiot-android)
#   target_link_libraries(<jni library> PRIVATE daas::libdaas log)
#
# and on the Java side the sources of blew, android-drivers_sdk/third_party/blew/java (sourceSets).
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-23  Lorenzo Grillo  creation (from the CMakeLists.txt of daasiot-macos 0.1.0)

cmake_minimum_required(VERSION 3.22)

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" DAASIOT_ANDROID_VERSION LIMIT_COUNT 1)
project(daasiot-android VERSION ${DAASIOT_ANDROID_VERSION} LANGUAGES C CXX)

if(NOT ANDROID)
    message(FATAL_ERROR "daasiot-android: configure it with the Android NDK toolchain")
endif()

option(DAASIOT_ANDROID_BLE "build the BLE driver of the android-drivers_sdk (blew)" ON)
set(DAASIOT_ANDROID_DRIVERS "inet4;inet6" CACHE STRING "generic drivers of the libdaas package to link")

# ---- libdaas package -----------------------------------------------------------------------------------------------
set(DAASIOT_ANDROID_LIBDAAS "${CMAKE_CURRENT_LIST_DIR}/libdaas/android-${ANDROID_ABI}/static")
if(NOT EXISTS "${DAASIOT_ANDROID_LIBDAAS}/lib/libdaas.a")
    message(FATAL_ERROR "daasiot-android: no libdaas package for ${ANDROID_ABI} (${DAASIOT_ANDROID_LIBDAAS})")
endif()
file(STRINGS "${DAASIOT_ANDROID_LIBDAAS}/VERSION.txt" DAASIOT_ANDROID_LIBDAAS_VERSION LIMIT_COUNT 1)
message(STATUS "daasiot-android ${DAASIOT_ANDROID_VERSION}: libdaas ${DAASIOT_ANDROID_LIBDAAS_VERSION} (${ANDROID_ABI})")

if(DAASIOT_ANDROID_BLE)
    add_subdirectory(android-drivers_sdk)
endif()

set(_ld "${DAASIOT_ANDROID_LIBDAAS}/lib")
set(_links "${_ld}/libdaas.a")
if(DAASIOT_ANDROID_BLE)
    # blew's JNI entry points (Java_it_sebyone_blew_*) are only referenced from Java: whole archive, or the linker
    # drops them (UnsatisfiedLinkError at run time).
    list(APPEND _links daas_drv_ble -Wl,--whole-archive "${DAASIOT_ANDROID_BLEW_LIB}" -Wl,--no-whole-archive)
endif()
foreach(_drv IN LISTS DAASIOT_ANDROID_DRIVERS)
    if(NOT EXISTS "${_ld}/drivers/libdaas_drv_${_drv}.a")
        message(FATAL_ERROR "daasiot-android: driver '${_drv}' not in the libdaas package")
    endif()
    list(APPEND _links "${_ld}/drivers/libdaas_drv_${_drv}.a")
endforeach()
list(APPEND _links "${_ld}/libdaas_nodrivers.a" log)

add_library(daasiot_libdaas INTERFACE)
add_library(daas::libdaas ALIAS daasiot_libdaas)
target_include_directories(daasiot_libdaas INTERFACE "${DAASIOT_ANDROID_LIBDAAS}/include")
target_compile_definitions(daasiot_libdaas INTERFACE ANDROID_BUILD)
target_compile_features(daasiot_libdaas INTERFACE cxx_std_17)
target_link_libraries(daasiot_libdaas INTERFACE ${_links})
