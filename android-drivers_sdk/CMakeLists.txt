# daas © 2019–2026 Sebyone Srl
#
# File: CMakeLists.txt — daasiot-android, android-drivers_sdk
#
# The specialized drivers of Android, built only against the headers of the libdaas package (drivers_sdk/include):
#
#   daas_drv_ble   the BLE driver (slot `ble` of _LINK_BLE, daas_static_driver_ble()) on blew (prebuilt libblew.a,
#                  Java side it.sebyone.blew in third_party/blew/java). daas::libdaas links it between libdaas.a and
#                  libdaas_nodrivers.a, so its entry takes the place of the nodriver.
#
# Contributors:
#   Lorenzo Grillo <l.grillo@sebyone.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Changes:
#   2026-11-23  Lorenzo Grillo  creation (from the macos-drivers_sdk of daasiot-macos)

set(BLEW_ROOT "${CMAKE_CURRENT_LIST_DIR}/third_party/blew")
file(STRINGS "${BLEW_ROOT}/VERSION" BLEW_VERSION_FILE LIMIT_COUNT 1)
set(BLEW_LIB "${BLEW_ROOT}/lib/${ANDROID_ABI}/libblew.a")
if(NOT EXISTS "${BLEW_LIB}")
    message(FATAL_ERROR "daasiot-android: blew ${BLEW_VERSION_FILE} not available for ${ANDROID_ABI} (${BLEW_LIB})")
endif()
set(DAASIOT_ANDROID_BLEW_LIB "${BLEW_LIB}" PARENT_SCOPE)

add_library(daas_drv_ble STATIC "${CMAKE_CURRENT_LIST_DIR}/ble/drv_ble.cpp")
add_library(daas::drv_ble ALIAS daas_drv_ble)
set_target_properties(daas_drv_ble PROPERTIES OUTPUT_NAME daas_drv_ble CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(daas_drv_ble
    PRIVATE "${DAASIOT_ANDROID_LIBDAAS}/drivers_sdk/include" "${CMAKE_CURRENT_LIST_DIR}" "${BLEW_ROOT}/include"
    PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_definitions(daas_drv_ble PRIVATE ANDROID_BUILD BLEW_VERSION="${BLEW_VERSION_FILE}"
    PUBLIC DAASIOT_ANDROID_BLE=1)
target_compile_options(daas_drv_ble PRIVATE -Wno-unused-parameter)
