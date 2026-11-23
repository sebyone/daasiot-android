// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <vector>

#include <blew/Types.h>

namespace blew {
namespace gatt {

/** Snapshot of a remote characteristic. */
struct CharacteristicInfo {
    Uuid uuid;
    uint32_t flags = 0;  // CharacteristicFlags

    bool can_read() const { return flags & CHR_READ; }
    bool can_write_request() const { return flags & CHR_WRITE; }
    bool can_write_command() const { return flags & CHR_WRITE_NO_RESPONSE; }
    bool can_notify() const { return flags & CHR_NOTIFY; }
    bool can_indicate() const { return flags & CHR_INDICATE; }
};

/** Snapshot of a remote service. Advertised services (before connecting) have no characteristics. */
struct ServiceInfo {
    Uuid uuid;
    std::vector<CharacteristicInfo> characteristics;
    Bytes data;  // advertised service data, if any
};

}  // namespace gatt
}  // namespace blew
