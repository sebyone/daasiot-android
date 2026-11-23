// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
//
// High level layer: message links between nodes over BLE (protocol: docs/protocol.md).
//
//   auto node = blew::link::Node::create({"a1b2c3d4", sid});
//   node.set_on_accept([](blew::link::Link l) { ... });
//   node.start();                                   // advertise + accept links
//   auto link = node.open("0badcafe");              // or wait for on_accept
//   if (link) link.value().send(bytes);
//
// A Link behaves the same whichever side opened it. Heartbeat, close, fragmentation on the real
// MTU and discovery are handled here, identically on every platform.
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <blew/Status.h>
#include <blew/Types.h>
#include <blew/export.h>
#include <blew/gatt/Central.h>

namespace blew {
namespace link {

struct Config {
    std::string node_id;                  // 8 hex digits, identifies this node
    uint64_t sid = 0;                     // published in INFO, used by discovery filters
    std::function<uint64_t()> sid_source; // if set, called on every INFO read instead of `sid`
    std::string name = "blew";            // advertised local name (not applied on Windows)
    uint16_t max_message = 4096;          // largest message accepted and sent
    int max_links = 5;                    // concurrent links, both roles together
    std::chrono::milliseconds ping_interval{5000};
    std::chrono::milliseconds link_timeout{10000};
    std::chrono::milliseconds open_timeout{5000};
    std::chrono::milliseconds scan_timeout{5000};
    gatt::Central central;                // empty: default adapter
};

enum class Role { Central, Peripheral };  // GATT role of this node on the link

enum class CloseReason { Local, Remote, Timeout, ProtocolError, Disconnected };

/** Where RadioStats::rssi comes from. */
enum class RssiSource {
    None,         // never measured
    Advertising,  // the node's last advertisement heard while scanning
    Connection,   // measured on the live connection (ESP32 both roles; Android, macOS, iOS as central)
};

/**
 * Radio and traffic figures of a remote node (Link::radio_stats(), Node::radio_stats()).
 * What the platform cannot measure keeps its "unknown" value.
 */
struct RadioStats {
    static constexpr int16_t kNoRssi = -127;
    static constexpr int16_t kNoTxPower = -32768;

    std::string node_id;   // remote node
    Address address;       // MAC address, or the CoreBluetooth UUID on Apple; empty if unknown
    bool connected = false;  // an open link exists: the link and traffic fields are valid

    // --- signal ---
    int16_t rssi = kNoRssi;  // dBm
    RssiSource rssi_source = RssiSource::None;
    std::chrono::milliseconds rssi_age{-1};   // since `rssi` was measured, -1 if never
    int16_t tx_power = kNoTxPower;            // advertised TX power in dBm
    std::chrono::milliseconds last_seen{-1};  // since its last advertisement was heard, -1 if never

    // --- link ---
    Role role = Role::Central;
    uint16_t mtu = 23;                    // ATT MTU
    std::chrono::milliseconds rtt{-1};    // last PING -> PONG round trip, -1 until one is answered
    std::chrono::milliseconds uptime{0};  // since the link opened
    std::chrono::milliseconds idle{0};    // since the last frame received

    // --- traffic, since the link opened ---
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t bytes_sent = 0;  // message payload
    uint64_t bytes_received = 0;
    uint64_t frames_sent = 0;  // on air, control frames included
    uint64_t frames_received = 0;
};

class LinkImpl;
class NodeImpl;

/** One message stream with a remote node. Cheap to copy; an empty Link is not open. */
class BLEW_EXPORT Link {
  public:
    using MessageCallback = std::function<void(const Bytes& message)>;
    using CloseCallback = std::function<void(CloseReason reason)>;

    Link() = default;
    explicit Link(std::shared_ptr<LinkImpl> impl) : impl_(std::move(impl)) {}

    bool initialized() const { return impl_ != nullptr; }
    bool is_open() const;
    std::string remote_node() const;
    Role role() const;

    /** Sends one message (fragmented as needed). Blocks until it is handed to the stack. */
    Status send(const Bytes& message);
    /** Pops the oldest received message; false if none. Not used when a message callback is set. */
    bool receive(Bytes& message);
    /** Number of received messages waiting in the queue. */
    size_t pending() const;
    /** Delivers messages through `callback` (dispatcher thread) instead of the queue. */
    void set_on_message(MessageCallback callback);
    void set_on_close(CloseCallback callback);

    Status close();

    /**
     * Radio and traffic figures of this link. With `measure` the RSSI of the connection is read
     * now where the platform can (it blocks for about one connection interval); otherwise, and
     * where it cannot, `rssi` is the last value known (connection or advertisement).
     */
    RadioStats radio_stats(bool measure = true) const;

    bool operator==(const Link& o) const { return impl_ == o.impl_; }
    bool operator!=(const Link& o) const { return impl_ != o.impl_; }

  private:
    std::shared_ptr<LinkImpl> impl_;
};

class BLEW_EXPORT Node {
  public:
    using AcceptCallback = std::function<void(Link link)>;
    using DiscoveryCallback = std::function<void(const Bytes& beacon, Link link)>;
    /** Discovery filter on the remote SID; return true to accept the node. */
    using SidFilter = std::function<bool(uint64_t sid)>;

    Node() = default;
    static Node create(Config config);

    bool initialized() const { return impl_ != nullptr; }
    const Config& config() const;

    /** Publishes the service and starts accepting links. */
    Status start();
    /** Closes every link and stops advertising. */
    Status stop();

    /** Opens a link to `node_id`, or returns the link that node opened with a discovery. */
    Result<Link> open(const std::string& node_id);
    /**
     * Scans for blew nodes, keeps those accepted by `filter` (any node if empty), and opens a
     * discovery link carrying `beacon` to the first one that accepts it.
     */
    Result<Link> discover(const Bytes& beacon, SidFilter filter = {});

    /** A remote node opened a link to us. */
    void set_on_accept(AcceptCallback callback);
    /** A remote node opened a discovery link to us. */
    void set_on_discovery(DiscoveryCallback callback);

    std::vector<Link> links() const;

    /**
     * Radio figures of every node known to this one: one entry per open link (as
     * Link::radio_stats(measure)), then one per node only heard while scanning (connected false,
     * signal fields only). Scans happen in open() and discover().
     */
    std::vector<RadioStats> radio_stats(bool measure = false) const;

    /** "<node_id>-b1e0-4c00-8000-00000000da55" */
    static Uuid service_uuid(const std::string& node_id);

  private:
    explicit Node(std::shared_ptr<NodeImpl> impl) : impl_(std::move(impl)) {}
    std::shared_ptr<NodeImpl> impl_;
};

}  // namespace link
}  // namespace blew
