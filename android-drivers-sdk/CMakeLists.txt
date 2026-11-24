# daas © 2019–2026 Sebyone Srl — MPL-2.0
#
# File: CMakeLists.txt — daasiot-android, android-drivers-sdk
#
# Builds the specialized drivers of this folder for one entry of the libdaas lock (one ABI, static), only against the
# headers of its libdaas package (libdaas_package.cmake) and the precompiled blew of the DaaS Build Console:
#
#   libdaas_drv_ble.a   the BLE driver in the slot `ble` of _LINK_BLE (daas_static_driver_ble()), on blew and the
#                       Android BLE stack (Java side it.sebyone.blew, in the blew package)
#
# Driven by android-drivers-sdk/build_drivers_kit.py, which also fetches blew and installs the drivers_kit:
#
#   cmake -S android-drivers-sdk -B out/drivers/<target> -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
#         -DANDROID_ABI=<abi> -DANDROID_PLATFORM=android-30 -DANDROID_STL=c++_static -DBLEW_ROOT=<blew package>
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-23  Lorenzo Grillo  creation (from the macos-drivers_sdk of daasiot-macos): daas_drv_ble with blew of
#                               third_party/blew
#   2026-11-24  l.grillo@sebyone.it  a project of its own per lock entry, against the libdaas package and the
#                                    precompiled blew of the DaaS Build Console (conventions); the template driver

cmake_minimum_required(VERSION 3.21)
project(android_drivers_sdk LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/libdaas_package.cmake)
if(NOT BLEW_ROOT OR NOT EXISTS "${BLEW_ROOT}/include/blew/link.h" OR NOT EXISTS "${BLEW_ROOT}/lib/libblew.a")
    message(FATAL_ERROR "set BLEW_ROOT to the unpacked blew package of ${DAAS_TARGET} (include/blew, lib/libblew.a)")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DRIVER_FLAGS -O2 -fPIC -fvisibility=hidden -Wall -Wextra -Wno-unused-parameter)

# A driver: STATIC libdaas_drv_<id>.a (the Android packages are static only).
function(daas_driver id)
    cmake_parse_arguments(D "" "" "SOURCES;INCLUDES;DEFINITIONS" ${ARGN})
    add_library(${id} STATIC ${D_SOURCES})
    set_target_properties(${id} PROPERTIES OUTPUT_NAME daas_drv_${id})
    target_include_directories(${id} PRIVATE ${D_INCLUDES} "${DAAS_DRIVERS_SDK_INCLUDE}")
    target_compile_definitions(${id} PRIVATE ${DAAS_PLATFORM_DEFS} ${D_DEFINITIONS})
    target_compile_options(${id} PRIVATE ${DRIVER_FLAGS})
endfunction()

daas_driver(ble SOURCES ble/drv_ble.cpp INCLUDES ble include "${BLEW_ROOT}/include"
            DEFINITIONS USE_BLE=1)

# The template driver: built (it must keep compiling against every libdaas package), never installed in a kit.
daas_driver(template SOURCES template/drv_template.cpp INCLUDES template DEFINITIONS USE_TEMPLATE=1)
