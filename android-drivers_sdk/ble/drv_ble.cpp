/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: drv_ble.cpp — daasiot-android, android-drivers_sdk
 *
 * BLE transport driver on blew (see drv_ble.hpp). The last line, DAAS_DRIVER(ble, ...), defines
 * daas_static_driver_ble(): the entry of the `ble` slot of _LINK_BLE, which libdaas looks up at run time before the
 * `ble5t` slot. Linked with the node, it takes the place of the "not linked" answer of libdaas_nodrivers.
 *
 * Compared with the driver of libdaas 0.24.4 it needs no internal header of libdaas:
 *   - the SID searched by a discovery beacon is read from the beacon itself (layout below), not by decoding a DME;
 *   - the local SID published by blew is set by the application (daas_ble_set_local_sid()), not read from agent_t.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *   Matteo Pagano <m.pagano@sebyone.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-01  Lorenzo Grillo  libdaas 0.24.4 (source/drivers/implementation/ble/drv_ble.cpp of daas-api)
 *   2026-11-12  Lorenzo Grillo  esp32-drivers_sdk: DAAS_DRIVER(ble) entry, contract headers of the package only,
 *                               uri_ble_t parsed once (urn), beacon SID read from the beacon, local SID from the application
 *   2026-11-14  Lorenzo Grillo   daasiot-macos: macos-drivers_sdk, blew on CoreBluetooth
 *   2026-11-23  Lorenzo Grillo   daasiot-android: android-drivers_sdk, blew on the Android BLE stack
 */
#include "../android_drivers_sdk.hpp"

#include <atomic>
#include <cctype>
#include <string.h>

#include "local_ble.hpp"

// ---- uri ----------------------------------------------------------------------------------------------------------

uri_ble_t::uri_ble_t(const char* suri) {
    node_id[0] = '\0';
    if (suri == nullptr) return;
    for (int i = 0; i < BLE_NODE_ID_LEN; i++) {
        const char c = suri[i];
        if (!isxdigit((unsigned char)c)) return;   // too short or not hexadecimal: created stays false
        node_id[i] = (char)tolower((unsigned char)c);
    }
    node_id[BLE_NODE_ID_LEN] = '\0';
    created = true;
}

// ---- local SID ----------------------------------------------------------------------------------------------------

static std::atomic<uint64_t> s_local_sid{0};
static std::atomic<bool>     s_local_sid_set{false};

extern "C" void daas_ble_set_local_sid(uint64_t sid) {
    s_local_sid = sid;
    s_local_sid_set = true;
}

#if USE_BLE

#include "drv_ble.hpp"
#include "ble_version.hpp"

#define DAAS_LOG_DEFAULT_CAT DAAS_CAT_DRV
#include "helpers/logger.hpp"
#include "drivers/type/driver_entry.hpp"

// Discovery beacon of the daas protocol (6.x), little endian: typeset u16, origin DIN u64, searched SID u64, ...
// Only the SID is read here, to let blew connect only to the nodes of that SID.
static constexpr size_t BEACON_SID_OFFSET = 2 + 8;

static uint64_t le_u64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--) v = (v << 8) | p[i];
    return v;
}

// SIDs may come as the user part (e.g. 100) or in the full form (user part << USER_PART_SHIFT): compare user parts.
static uint64_t sid_user_part(uint64_t sid) {
    return (sid >> USER_PART_SHIFT) != 0 ? (sid >> USER_PART_SHIFT) : sid;
}

/* Node id of a peer: from its uri class, parsed when the uri_t was made. */
static std::string node_id_of(const uri_t* u) {
    const uri_ble_t* b = u ? static_cast<const uri_ble_t*>(u->urn) : nullptr;
    if (b != nullptr && b->created) return std::string(b->node_id);
    return std::string();
}

drv_ble_t::drv_ble_t(idriver_event_t* event_incoming_connection_, int idriver_) {
    event_incoming_connection = event_incoming_connection_;
    _idriver          = idriver_;
    _state            = _UNDEFINED;
    discovery_pending = false;
    enabled_uri       = nullptr;
    memset(discovery_buffer, 0, sizeof(discovery_buffer));
}

drv_ble_t::~drv_ble_t() {
    disable();
    _node.stop();
}

bool drv_ble_t::init(uri_t* l_uri) {
    if (!l_uri) return false;

    blew::link::Config cfg;
    cfg.node_id    = node_id_of(l_uri);
    cfg.name       = "daas_node";
    cfg.max_links  = _MAX_BT_CONNECTIONS;
    cfg.sid_source = [] { return (uint64_t)s_local_sid; };

    _node = blew::link::Node::create(cfg);
    if (!_node.initialized()) {
        LOGE("[BLE] Invalid node id '%s' (8 hex digits expected)", l_uri->uri);
        return false;
    }

    /* A remote node opened a link to us: give it a sub. The overlay is told from run(), on the
       core thread, once the auth packet has arrived: this callback runs on blew's dispatcher
       thread, which also delivers the messages, so waiting for the auth packet here would block
       its delivery. */
    _node.set_on_accept([this](blew::link::Link link) {
        int isub = allocate_sub(link, false, false);
        if (isub < 0) {
            LOGE("[BLE] No free sub slots available for new connection");
            link.close();
            return;
        }
        LOGI("[BLE] New connection from %s, sub %d", link.remote_node().c_str(), isub);
    });

    /* A discovery beacon: kept until the overlay reads it; the link is handed out by open_sub(). */
    _node.set_on_discovery([this](const blew::Bytes& beacon, blew::link::Link link) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (discovery_pending) {
            LOGW("[BLE] Discovery from %s dropped, previous beacon not consumed", link.remote_node().c_str());
            link.close();
            return;
        }
        memset(discovery_buffer, 0, sizeof(discovery_buffer));
        memcpy(discovery_buffer, beacon.data(), beacon.size() < sizeof(discovery_buffer) ? beacon.size()
                                                                                           : sizeof(discovery_buffer));
        discovery_link    = link;
        discovery_pending = true;
        LOGI("[BLE] Discovery beacon from %s, %u bytes", link.remote_node().c_str(), (unsigned)beacon.size());
    });

    enabled_uri = l_uri;
    _state = _READY;
    LOGI("[BLE] Driver initialized, node %s", cfg.node_id.c_str());
    return true;
}

bool drv_ble_t::enable() {
    if (_state != _READY) return false;
    blew::Status s = _node.start();
    if (s != blew::Status::Ok) {
        LOGE("[BLE] Cannot start: %s", blew::to_string(s));
        return false;
    }
    _state = _ACTIVE;
    LOGI("[BLE] Driver enabled");
    return true;
}

void drv_ble_t::disable() {
    for (int i = 0; i < _MAX_BT_CONNECTIONS; i++) close_sub(i);
    if (_state == _ACTIVE) _node.stop();
    if (_state != _UNDEFINED) _state = _READY;
}

bool drv_ble_t::is_opened() {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& s : _subs)
        if (s.in_use) return true;
    return false;
}

size_t drv_ble_t::available_locked(const sub_connection_t& s) {
    size_t total = 0;
    for (auto& m : s.rx) total += m.size();
    return total - s.rx_offset;
}

int drv_ble_t::allocate_sub(const blew::link::Link& link, bool discovery, bool announced) {
    int isub = -1;
    uint32_t generation = 0;
    {
        std::lock_guard<std::mutex> lock(_mtx);
        for (int i = 0; i < _MAX_BT_CONNECTIONS && isub < 0; i++)
            if (!_subs[i].in_use) isub = i;
        if (isub < 0) return -1;
        sub_connection_t& s = _subs[isub];
        s.link      = link;
        s.rx.clear();
        s.rx_offset = 0;
        s.in_use    = true;
        s.discovery = discovery;
        s.announced = announced;
        s.pending   = false;
        generation  = ++s.generation;
    }

    /* Incoming messages go to the sub queue. A sub not yet announced (accepted, or our discovery
       link) is reported to the overlay by run() once its first message is in. The generation
       keeps a closed link's late messages out of a slot that was reused meanwhile. */
    blew::link::Link l = link;
    l.set_on_message([this, isub, generation](const blew::Bytes& msg) {
        std::lock_guard<std::mutex> lock(_mtx);
        sub_connection_t& s = _subs[isub];
        if (!s.in_use || s.generation != generation) return;
        s.rx.push_back(msg);
        if (!s.announced) {
            s.announced = true;
            s.pending   = true;
        }
    });
    return isub;
}

void drv_ble_t::release_sub(int isub) {
    std::lock_guard<std::mutex> lock(_mtx);
    sub_connection_t& s = _subs[isub];
    s.link      = blew::link::Link();
    s.rx.clear();
    s.rx_offset = 0;
    s.in_use    = false;
    s.discovery = false;
    s.announced = false;
    s.pending   = false;
}

int drv_ble_t::open_sub(const uri_t* r_uri, int& i_sub) {
    if (_state != _ACTIVE || !r_uri) return -1;
    const std::string peer = node_id_of(r_uri);
    if (peer.empty()) {
        LOGE("[BLE] Invalid remote uri '%s' (8 hex digits expected)", r_uri->uri);
        return -1;
    }

    /* Returns the link a discovery from that node created, or opens a new one. */
    auto opened = _node.open(peer);
    if (!opened.ok()) {
        LOGI("[BLE] Impossible to open with %s: %s", r_uri->uri, blew::to_string(opened.status()));
        return -1;
    }
    {
        std::lock_guard<std::mutex> lock(_mtx);
        if (discovery_link == opened.value()) discovery_link = blew::link::Link();
    }
    i_sub = allocate_sub(opened.value(), false, true);
    if (i_sub < 0) {
        LOGE("[BLE] No free sub slots available");
        opened.value().close();
        return -1;
    }
    LOGI("[BLE] Opened %s on sub %d", r_uri->uri, i_sub);
    return 0;
}

void drv_ble_t::close_sub(int i_sub) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS) return;
    blew::link::Link link;
    {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_subs[i_sub].in_use) return;
        link = _subs[i_sub].link;
    }
    link.close();
    release_sub(i_sub);
}

int drv_ble_t::read(int i_sub, unsigned char* buff, int max_size) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS || max_size <= 0) return 0;
    std::lock_guard<std::mutex> lock(_mtx);
    sub_connection_t& s = _subs[i_sub];
    if (!s.in_use) return 0;

    /* The sub is a byte stream: a read may span several blew messages. */
    size_t done = 0;
    while (done < (size_t)max_size && !s.rx.empty()) {
        const blew::Bytes& msg = s.rx.front();
        size_t n = msg.size() - s.rx_offset;
        if (n > (size_t)max_size - done) n = (size_t)max_size - done;
        memcpy(buff + done, msg.data() + s.rx_offset, n);
        done        += n;
        s.rx_offset += n;
        if (s.rx_offset >= msg.size()) {
            s.rx.pop_front();
            s.rx_offset = 0;
        }
    }
    return (int)done;
}

int drv_ble_t::write(int i_sub, const unsigned char* buff, int size) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS || size < 0) return -1;
    blew::link::Link link;
    {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_subs[i_sub].in_use) return -1;
        link = _subs[i_sub].link;
    }
    blew::Status s = link.send(blew::Bytes(buff, buff + size));
    if (s != blew::Status::Ok) {
        LOGE("[BLE] Write on sub %d failed: %s", i_sub, blew::to_string(s));
        return -1;
    }
    return size;
}

/* Discards size bytes of the sub's stream (the rest of a DME the protocol gave up on). */
bool drv_ble_t::flush(int i_sub, int size) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS || size < 0) return false;
    std::lock_guard<std::mutex> lock(_mtx);
    sub_connection_t& s = _subs[i_sub];
    if (!s.in_use) return false;

    size_t left = (size_t)size;
    while (left > 0 && !s.rx.empty()) {
        const size_t in_front = s.rx.front().size() - s.rx_offset;
        if (left >= in_front) {
            left -= in_front;
            s.rx.pop_front();
            s.rx_offset = 0;
        } else {
            s.rx_offset += left;
            left = 0;
        }
    }
    return true;
}

int drv_ble_t::rx_data_ready(int i_sub) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS) return -1;
    std::lock_guard<std::mutex> lock(_mtx);
    const sub_connection_t& s = _subs[i_sub];
    if (!s.in_use) return -1;
    return (int)available_locked(s);
}

void drv_ble_t::run() {
    /* Report the subs whose first message arrived (accepted links, our discovery link): on the
       core thread, without the lock, since incoming_request() reads the auth packet through
       rx_data_ready()/read(). A refused connection is closed. */
    for (int i = 0; i < _MAX_BT_CONNECTIONS; i++) {
        bool report;
        {
            std::lock_guard<std::mutex> lock(_mtx);
            report = _subs[i].in_use && _subs[i].pending;
            _subs[i].pending = false;
        }
        if (report && event_incoming_connection && event_incoming_connection->incoming_request(_idriver, i) < 0) {
            LOGW("[BLE] Connection on sub %d refused by the overlay", i);
            close_sub(i);
        }
    }

    /* Heartbeat and timeouts live in blew::link: here we only free the slots of closed links
       once the overlay has read what they received. */
    for (int i = 0; i < _MAX_BT_CONNECTIONS; i++) {
        bool closed;
        {
            std::lock_guard<std::mutex> lock(_mtx);
            closed = _subs[i].in_use && !_subs[i].link.is_open() && _subs[i].rx.empty();
        }
        if (closed) {
            LOGW("[BLE] Link of sub %d closed, releasing it", i);
            release_sub(i);
        }
    }
}

int drv_ble_t::send_discovery_beacon(const char* packet, uint32_t packet_size) {
    if (_state != _ACTIVE || packet == nullptr || packet_size < BEACON_SID_OFFSET + 8) return -1;

    /* Free the slots of our previous discovery links (the connections of the radio are few). */
    for (int i = 0; i < _MAX_BT_CONNECTIONS; i++) {
        bool stale;
        {
            std::lock_guard<std::mutex> lock(_mtx);
            stale = _subs[i].in_use && _subs[i].discovery && !_subs[i].announced;
        }
        if (stale) close_sub(i);
    }

    /* The SID the beacon searches: blew connects only to the nodes that publish it (0: any node). Until the application
       sets the local SID, it is also the SID this node publishes. */
    const uint64_t wanted = sid_user_part(le_u64(reinterpret_cast<const uint8_t*>(packet) + BEACON_SID_OFFSET));
    if (!s_local_sid_set && wanted != 0) s_local_sid = wanted;
    blew::link::Node::SidFilter filter;
    if (wanted != 0) filter = [wanted](uint64_t sid) { return sid_user_part(sid) == wanted; };

    auto link = _node.discover(blew::Bytes(packet, packet + packet_size), filter);
    if (!link.ok()) {
        LOGE("[BLE] No node accepted the discovery beacon: %s", blew::to_string(link.status()));
        return -1;
    }
    if (allocate_sub(link.value(), true, false) < 0) {
        LOGE("[BLE] send_discovery_beacon: no free sub slot");
        link.value().close();
        return -1;
    }
    return 0;
}

int drv_ble_t::check_discovery_beacon(char* packet_out) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (!discovery_pending) return -1;
    memcpy(packet_out, discovery_buffer, DISCOVERY_PCKT_SIZE);
    discovery_pending = false;
    return 0;
}

bool drv_ble_t::check_sub_closed_connection(int i_sub) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS) return true;
    std::lock_guard<std::mutex> lock(_mtx);
    return !_subs[i_sub].in_use || !_subs[i_sub].link.is_open();
}

// Spec §11.3: peripherals wake on advertising, few connection slots, notifications; same radio
// link. No pairing is performed by this driver, so no security flag is claimed. A connection is a
// point-to-point link between two devices: CHANNEL_AV_MULTIPLEXED stays clear.
cx_channel_declaration_t drv_ble_t::channel_declaration() const {
    return probe_make_declaration(CHANNEL_AV_ADVERTISING_WAKE | CHANNEL_AV_CONNECTION_SLOTS, 0, CHANNEL_PERF_NOTIFY,
                                  CHANNEL_EFF_TX_POWER_CONTROL | CHANNEL_EFF_SAME_LINK,
                                  probe_q16(0, 80), 0, probe_q16(0, 80), probe_q16(0, 70), 50, 100, 50, 0,
                                  cx_underlay_security_t::open_local);
}

// Link state and locality only: the RSSI of blew is not sampled here.
bool drv_ble_t::channel_sample(int i_sub, cx_driver_sample_t& out) {
    if (i_sub < 0 || i_sub >= _MAX_BT_CONNECTIONS) return false;
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_subs[i_sub].in_use || !_subs[i_sub].link.is_open()) return false;
    out = cx_driver_sample_t{};
    out.valid = cx_driver_sample_t::LINK_QUALITY | cx_driver_sample_t::LOCALITY;
    out.link_quality = CX_ONE;
    out.locality = 0;
    return true;
}

// The entry of the `ble` slot of _LINK_BLE: replaces "not linked" of libdaas_nodrivers.
DAAS_DRIVER(ble, drv_ble_t, uri_ble_t, _LINK_BLE, "BLE", DAAS_DRIVER_VERSION_BLE)

#endif // USE_BLE
