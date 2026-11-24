# daas © 2019–2026 Sebyone Srl — MPL-2.0
#
# File: libdaas_package.cmake — daasiot-android, android-drivers-sdk
#
# The libdaas package the drivers of this folder are built against: the drivers are compiled only with the headers of
# the package (include/ and drivers_sdk/include/) and the sources of this folder.
#
#   DAAS_TARGET               the package target, android-arm64-v8a, android-armeabi-v7a or android-x86_64: cache
#                             variable, default android-${ANDROID_ABI} (the ABI of the NDK toolchain)
#   DAAS_LINKAGE              static (the Android packages are static only)
#   DAAS_ROOT                 the unpacked package: cache or environment variable, default
#                             android_daas/libdaas/<DAAS_TARGET>/static of this repository (tools/fetch_ld)
#   DAAS_DRIVERS_SDK_INCLUDE  ${DAAS_ROOT}/drivers_sdk/include: the headers of the driver contract
#   DAAS_PLATFORM_DEFS        the compile definitions of the package (platform_defs of its MANIFEST.json)
#   DAAS_VERSION              the libdaas version of the package (VERSION.txt)
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changelog:
#   2026-11-24  l.grillo@sebyone.it  creation, as macos-drivers-sdk/libdaas_package.cmake of daasiot-macos

include_guard(GLOBAL)

if(NOT DAAS_TARGET AND ANDROID_ABI)
    set(DAAS_TARGET "android-${ANDROID_ABI}")
endif()
if(NOT DAAS_TARGET MATCHES "^android-(arm64-v8a|armeabi-v7a|x86_64)$")
    message(FATAL_ERROR "set DAAS_TARGET to android-arm64-v8a, android-armeabi-v7a or android-x86_64 (or build with "
                        "the toolchain of the NDK and ANDROID_ABI)")
endif()
if(ANDROID_ABI AND NOT DAAS_TARGET STREQUAL "android-${ANDROID_ABI}")
    message(FATAL_ERROR "DAAS_TARGET ${DAAS_TARGET} and ANDROID_ABI ${ANDROID_ABI} differ")
endif()
set(DAAS_LINKAGE static)
if(NOT DAAS_ROOT AND DEFINED ENV{DAAS_ROOT} AND NOT "$ENV{DAAS_ROOT}" STREQUAL "")
    set(DAAS_ROOT "$ENV{DAAS_ROOT}")
endif()
if(NOT DAAS_ROOT)
    set(DAAS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../android_daas/libdaas/${DAAS_TARGET}/static")
endif()
get_filename_component(DAAS_ROOT "${DAAS_ROOT}" ABSOLUTE)
set(DAAS_ROOT "${DAAS_ROOT}" CACHE PATH "unpacked libdaas package of ${DAAS_TARGET}" FORCE)

if(NOT EXISTS "${DAAS_ROOT}/include/daas.hpp")
    message(FATAL_ERROR "libdaas package not found in ${DAAS_ROOT}: run tools/fetch_ld/fetch_ld.sh")
endif()
set(DAAS_DRIVERS_SDK_INCLUDE "${DAAS_ROOT}/drivers_sdk/include")
if(NOT EXISTS "${DAAS_DRIVERS_SDK_INCLUDE}/interface/idriver.hpp")
    message(FATAL_ERROR "${DAAS_ROOT} has no drivers_sdk/include: the drivers need a libdaas package >= 0.24.8")
endif()

set(DAAS_PLATFORM_DEFS ANDROID_BUILD)
file(READ "${DAAS_ROOT}/MANIFEST.json" _daas_manifest)
string(REGEX MATCH "\"platform_defs\"[ \t\r\n]*:[ \t\r\n]*\\[([^]]*)\\]" _m "${_daas_manifest}")
if(CMAKE_MATCH_1)
    string(REGEX REPLACE "[\" \t\r\n]" "" _defs "${CMAKE_MATCH_1}")
    string(REPLACE "," ";" DAAS_PLATFORM_DEFS "${_defs}")
endif()
file(STRINGS "${DAAS_ROOT}/VERSION.txt" DAAS_VERSION LIMIT_COUNT 1)
message(STATUS "drivers of ${DAAS_TARGET} against libdaas ${DAAS_VERSION}: ${DAAS_ROOT} (${DAAS_PLATFORM_DEFS})")
