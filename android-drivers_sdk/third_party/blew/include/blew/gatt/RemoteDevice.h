// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <blew/Status.h>
#include <blew/Types.h>
#include <blew/export.h>
#include <blew/gatt/Attributes.h>

namespace blew {
namespace gatt {

class RemoteDeviceBackend;

/**
 * A remote device found by scanning (GATT client side).
 *
 * Cheap to copy: copies share the same device. A default-constructed RemoteDevice is empty and
 * every call returns Status::NotInitialized. Calls block; callbacks run on the library's
 * dispatcher thread, from which it is safe to call back into the library.
 */
class BLEW_EXPORT RemoteDevice {
  public:
    using DataCallback = std::function<void(const Bytes& data)>;

    RemoteDevice() = default;
    explicit RemoteDevice(std::shared_ptr<RemoteDeviceBackend> internal) : internal_(std::move(internal)) {}

    bool initialized() const { return internal_ != nullptr; }

    /** Advertised local name (may be empty). */
    std::string name() const;
    Address address() const;
    AddressType address_type() const;
    int16_t rssi() const;
    /** Advertised TX power in dBm, -32768 if not advertised. */
    int16_t tx_power() const;
    bool is_connectable() const;
    std::map<uint16_t, Bytes> manufacturer_data() const;
    /** GATT services while connected, advertised services otherwise. */
    std::vector<ServiceInfo> services() const;
    bool advertises(const Uuid& service) const;

    Status connect();
    Status disconnect();
    bool is_connected() const;
    /** ATT MTU of the connection (23 when unknown). */
    uint16_t mtu() const;
    /**
     * Measures the RSSI of the live connection in dBm (rssi() is the one of the last
     * advertisement). NotSupported on Linux/BlueZ and Windows, which do not expose it.
     */
    Result<int16_t> read_rssi();

    Result<Bytes> read(const Uuid& service, const Uuid& characteristic);
    Status write_request(const Uuid& service, const Uuid& characteristic, const Bytes& data);
    Status write_command(const Uuid& service, const Uuid& characteristic, const Bytes& data);
    Status notify(const Uuid& service, const Uuid& characteristic, DataCallback callback);
    Status indicate(const Uuid& service, const Uuid& characteristic, DataCallback callback);
    Status unsubscribe(const Uuid& service, const Uuid& characteristic);

    void set_on_connected(std::function<void()> callback);
    void set_on_disconnected(std::function<void()> callback);

    bool operator==(const RemoteDevice& o) const { return internal_ == o.internal_; }
    bool operator!=(const RemoteDevice& o) const { return internal_ != o.internal_; }
    bool operator<(const RemoteDevice& o) const { return internal_ < o.internal_; }

  private:
    std::shared_ptr<RemoteDeviceBackend> internal_;
};

}  // namespace gatt
}  // namespace blew
