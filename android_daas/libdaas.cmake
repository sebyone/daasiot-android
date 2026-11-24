# daas © 2019–2026 Sebyone Srl
#
# File: libdaas.cmake — daasiot-android, android_daas
#
# Imports an unpacked libdaas package for Android (static) as the target libdaas::libdaas, with the drivers_kit of the
# android-drivers-sdk (the BLE driver on blew).
#
#   DAAS_TARGET         package target: android-<ANDROID_ABI> (android-arm64-v8a, android-armeabi-v7a,
#                       android-x86_64), from the ABI of the NDK toolchain
#   DAAS_ROOT           the package folder; default android_daas/libdaas/<DAAS_TARGET>/static (installed by
#                       tools/fetch_ld/fetch_ld.sh from the catalog of daasiot.com)
#   DAASIOT_ANDROID_BLE ON (default when the kit is there): the BLE driver of android_daas/drivers_kit/<DAAS_TARGET>/
#                       static/ (made by android-drivers-sdk/build_drivers_kit.py with the precompiled blew of the
#                       DaaS Build Console): libdaas_drv_ble.a + libblew.a (whole: its JNI entry points are reached only
#                       from Java); OFF: no BLE driver (the slot of _LINK_BLE answers "not implemented")
#   DAAS_DRIVERS        drivers of the package to use (inet4;inet6); default: every driver found
#   DAAS_EXTRA_DRIVERS  drivers of your own: static archives or objects that define daas_static_driver_<id>(), linked
#                       before the drivers of the package and libdaas_nodrivers, so they fill or replace a slot
#
# Links libdaas.a, the extra drivers, the chosen libdaas_drv_<d>.a, libdaas_nodrivers.a, blew, log.
#
# Contributors:
#   Lorenzo Grillo
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-24  Lorenzo Grillo  creation, from macos_daas/libdaas.cmake of daasiot-macos (conventions of the
#                               daasiot-<target> repositories: the package in android_daas/libdaas/, the drivers_kit)

include_guard(GLOBAL)
cmake_policy(SET CMP0057 NEW)   # IN_LIST

if(NOT DAAS_TARGET)
    if(NOT ANDROID_ABI)
        message(FATAL_ERROR "libdaas: build with the toolchain of the Android NDK (ANDROID_ABI), or set DAAS_TARGET")
    endif()
    set(DAAS_TARGET "android-${ANDROID_ABI}" CACHE STRING "libdaas package target: android-<ABI>")
endif()
set(DAAS_LINKAGE static)

get_filename_component(_daas_component "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)   # android_daas/
if(NOT DAAS_ROOT AND DEFINED ENV{DAAS_ROOT} AND NOT "$ENV{DAAS_ROOT}" STREQUAL "")
    set(DAAS_ROOT "$ENV{DAAS_ROOT}")
endif()
if(NOT DAAS_ROOT AND EXISTS "${_daas_component}/libdaas/${DAAS_TARGET}/static/include/daas.hpp")
    set(DAAS_ROOT "${_daas_component}/libdaas/${DAAS_TARGET}/static" CACHE PATH "unpacked libdaas package")
endif()
if(NOT DAAS_ROOT OR NOT EXISTS "${DAAS_ROOT}/include/daas.hpp")
    message(FATAL_ERROR "libdaas not found in android_daas/libdaas/${DAAS_TARGET}/static. Run: tools/fetch_ld/fetch_ld.sh "
                        "(or set DAAS_ROOT to an unpacked package)")
endif()

if(EXISTS "${DAAS_ROOT}/VERSION.txt")
    file(STRINGS "${DAAS_ROOT}/VERSION.txt" DAAS_VERSION LIMIT_COUNT 1)
endif()
message(STATUS "libdaas ${DAAS_VERSION} (${DAAS_TARGET}): ${DAAS_ROOT}")

# The drivers of the package: lib/drivers/libdaas_drv_<d>.a
set(DAAS_DRIVERS_DIR "${DAAS_ROOT}/lib/drivers")
file(GLOB _daas_found RELATIVE "${DAAS_DRIVERS_DIR}" "${DAAS_DRIVERS_DIR}/libdaas_drv_*.a")
list(TRANSFORM _daas_found REPLACE "^libdaas_drv_(.*)\\.a$" "\\1")
if(NOT DAAS_DRIVERS)
    set(DAAS_DRIVERS ${_daas_found})
endif()
foreach(_d IN LISTS DAAS_DRIVERS)
    if(NOT _d IN_LIST _daas_found)
        message(FATAL_ERROR "driver '${_d}' is not in ${DAAS_DRIVERS_DIR} (${_daas_found})")
    endif()
endforeach()
message(STATUS "libdaas drivers: ${DAAS_DRIVERS}")
foreach(_x IN LISTS DAAS_EXTRA_DRIVERS)
    if(NOT EXISTS "${_x}")
        message(FATAL_ERROR "DAAS_EXTRA_DRIVERS: ${_x} not found")
    endif()
endforeach()

# The drivers_kit of the android-drivers-sdk: the BLE driver in the slot of _LINK_BLE.
set(DAAS_KIT_ROOT "${_daas_component}/drivers_kit/${DAAS_TARGET}/static" CACHE PATH "drivers_kit of the package")
if(NOT DEFINED DAASIOT_ANDROID_BLE)                 # default: ON when the drivers_kit of the package is there
    if(EXISTS "${DAAS_KIT_ROOT}/DRIVERS.txt")
        set(_daas_ble_default ON)
    else()
        set(_daas_ble_default OFF)
        message(STATUS "libdaas: no drivers_kit in ${DAAS_KIT_ROOT}: built without the BLE driver "
                       "(python3 android-drivers-sdk/build_drivers_kit.py makes it)")
    endif()
    option(DAASIOT_ANDROID_BLE "the BLE driver of the drivers_kit (blew on the Android BLE stack)" ${_daas_ble_default})
endif()
set(DAAS_KIT_LIBS "")
set(ANDROID_DAAS_BLE_KIT 0)
if(DAASIOT_ANDROID_BLE)
    foreach(_f libdaas_drv_ble.a libblew.a)
        if(NOT EXISTS "${DAAS_KIT_ROOT}/${_f}")
            message(FATAL_ERROR "BLE: ${DAAS_KIT_ROOT}/${_f} not found. Build the drivers_kit: python3 "
                                "android-drivers-sdk/build_drivers_kit.py, or configure with -DDAASIOT_ANDROID_BLE=OFF")
        endif()
    endforeach()
    list(APPEND DAAS_EXTRA_DRIVERS "${DAAS_KIT_ROOT}/libdaas_drv_ble.a")   # before the drivers of the package
    # blew whole: its JNI entry points (Java_it_sebyone_blew_*) are referenced only from Java
    set(DAAS_KIT_LIBS "-Wl,--whole-archive" "${DAAS_KIT_ROOT}/libblew.a" "-Wl,--no-whole-archive")
    set(ANDROID_DAAS_BLE_KIT 1)
    message(STATUS "libdaas BLE driver of the drivers_kit: ${DAAS_KIT_ROOT}")
endif()

add_library(libdaas INTERFACE)
add_library(libdaas::libdaas ALIAS libdaas)
target_include_directories(libdaas INTERFACE "${DAAS_ROOT}/include")
target_compile_features(libdaas INTERFACE cxx_std_17)
target_compile_definitions(libdaas INTERFACE ANDROID_DAAS_BLE_KIT=${ANDROID_DAAS_BLE_KIT})

# libdaas, your drivers (first: an archive given before the others defines the entry of its slot), the drivers of the
# package, libdaas_nodrivers ("not implemented" for the empty slots), blew
set(_daas_libs "${DAAS_ROOT}/lib/libdaas.a" ${DAAS_EXTRA_DRIVERS})
foreach(_d IN LISTS DAAS_DRIVERS)
    list(APPEND _daas_libs "${DAAS_DRIVERS_DIR}/libdaas_drv_${_d}.a")
endforeach()
list(APPEND _daas_libs "${DAAS_ROOT}/lib/libdaas_nodrivers.a")
find_library(DAAS_LOG_LIB log)
target_link_libraries(libdaas INTERFACE ${_daas_libs} ${DAAS_KIT_LIBS} ${DAAS_LOG_LIB})
