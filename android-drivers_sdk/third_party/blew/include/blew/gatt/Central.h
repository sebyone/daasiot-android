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
#include <blew/gatt/RemoteDevice.h>

namespace blew {
namespace gatt {

class CentralBackend;

/** Local radio, central role. Obtain it with Central::get_centrals() or Central::get_default(). */
class BLEW_EXPORT Central {
  public:
    using PeripheralCallback = std::function<void(RemoteDevice)>;

    Central() = default;
    explicit Central(std::shared_ptr<CentralBackend> internal) : internal_(std::move(internal)) {}

    static std::vector<Central> get_centrals();
    /** First adapter, or an empty Central if there is none. */
    static Central get_default();
    static bool bluetooth_enabled();

    bool initialized() const { return internal_ != nullptr; }
    std::string identifier() const;
    Address address() const;
    bool is_powered() const;

    Status scan_start();
    Status scan_stop();
    /** Scans for `timeout_ms`, returning early if scan_stop() is called (e.g. from a callback). */
    Status scan(int timeout_ms);
    bool is_scanning() const;
    std::vector<RemoteDevice> scan_results() const;

    /** First advertisement of a peripheral in the current scan. */
    void set_on_scan_found(PeripheralCallback callback);
    /** Every further advertisement of an already found peripheral. */
    void set_on_scan_updated(PeripheralCallback callback);
    void set_on_scan_stop(std::function<void()> callback);

    std::shared_ptr<CentralBackend> internal() const { return internal_; }

  private:
    std::shared_ptr<CentralBackend> internal_;
};

}  // namespace gatt
}  // namespace blew
