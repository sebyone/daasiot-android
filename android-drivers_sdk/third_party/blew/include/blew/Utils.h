// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <string>

#include <blew/Types.h>
#include <blew/export.h>

namespace blew {

BLEW_EXPORT OperatingSystem get_operating_system();
BLEW_EXPORT std::string get_version();
/** Name of the compiled-in backend: "bluez", "corebluetooth", "winrt", "android", "nimble", "dummy". */
BLEW_EXPORT std::string get_backend_name();

namespace uuid {
/** Lowercase 128-bit form. "180d", "0000180D", "0000180d-0000-1000-8000-00805f9b34fb" all map to the last one. */
BLEW_EXPORT Uuid normalize(const Uuid& uuid);
BLEW_EXPORT bool equal(const Uuid& a, const Uuid& b);
/** True for 128-bit UUIDs that are not derived from the Bluetooth base UUID. */
BLEW_EXPORT bool is_custom(const Uuid& uuid);
/** Parses a normalized 128-bit UUID into 16 big-endian bytes. False if `uuid` is not one. */
BLEW_EXPORT bool to_bytes(const Uuid& uuid, uint8_t out[16]);
}  // namespace uuid

}  // namespace blew
