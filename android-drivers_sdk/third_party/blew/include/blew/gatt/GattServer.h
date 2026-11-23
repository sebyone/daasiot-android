// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <blew/Status.h>
#include <blew/Types.h>
#include <blew/export.h>
#include <blew/gatt/Central.h>

namespace blew {
namespace gatt {

class GattServerBackend;

/**
 * Local GATT server + advertiser (peripheral role).
 *
 * Peers are identified by a "peer id": MAC address on Linux/Android/ESP32/Windows, CoreBluetooth
 * central UUID on Apple platforms.
 *
 * The GATT database is published by the first start_advertising(); characteristics added later
 * are published by the next one (ESP32: only while no peer is connected).
 *
 * Write/connect/disconnect/subscribe callbacks run on the dispatcher thread. Read callbacks run
 * synchronously inside the stack: keep them short and do not call back into the library.
 */
class BLEW_EXPORT GattServer {
  public:
    using ReadCallback = std::function<Bytes(const std::string& peer)>;
    using WriteCallback = std::function<void(const Bytes& data, const std::string& peer)>;
    using PeerCallback = std::function<void(const std::string& peer)>;
    using SubscribeCallback = std::function<void(const Uuid& chr, const std::string& peer, bool on)>;

    GattServer() = default;  // empty: use GattServer::create()
    /** Creates a server on `adapter` (default adapter if empty). Check initialized(). */
    static GattServer create(const Central& adapter = Central());

    bool initialized() const { return internal_ != nullptr; }

    Status add_characteristic(const Uuid& service, const Uuid& characteristic, uint32_t flags);

    void set_on_read(const Uuid& characteristic, ReadCallback callback);
    void set_on_write(const Uuid& characteristic, WriteCallback callback);
    void set_on_connect(PeerCallback callback);
    void set_on_disconnect(PeerCallback callback);
    void set_on_subscribe(SubscribeCallback callback);

    /** Publishes the GATT database if needed, then (re)starts connectable advertising. */
    Status start_advertising(const std::string& name, const Uuid& service);
    Status stop_advertising();
    bool is_advertising() const;

    /**
     * Notifies `characteristic`. Empty `peer`: every subscribed peer. Returns NotConnected when
     * `peer` is gone, which is how a vanished central is detected.
     * Linux/BlueZ cannot address a single central: there the value reaches every subscriber.
     */
    Status notify(const Uuid& characteristic, const Bytes& data, const std::string& peer = "");
    /** Drops `peer` (CoreBluetooth cannot: there the peer is only forgotten). */
    Status disconnect(const std::string& peer);
    std::vector<std::string> peers() const;
    /** Largest value notify() can deliver to `peer` in one packet (MTU - 3). */
    uint16_t max_notify_size(const std::string& peer) const;
    /**
     * RSSI of the connection with `peer` in dBm. NotSupported where the peripheral stack does not
     * expose it (only ESP32/NimBLE does).
     */
    Result<int16_t> read_rssi(const std::string& peer) const;

  private:
    explicit GattServer(std::shared_ptr<GattServerBackend> internal) : internal_(std::move(internal)) {}
    std::shared_ptr<GattServerBackend> internal_;
};

}  // namespace gatt
}  // namespace blew
