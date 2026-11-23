/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: drv_ble.hpp — daasiot-android, android-drivers_sdk
 *
 * BLE transport driver of libdaas (idriver_t, driver ABI 2) on blew (ble-wrapper, third_party/blew/), Android
 * BLE stack (Java side it.sebyone.blew). Connections, heartbeat, fragmentation and discovery are handled by blew::link; this class maps the
 * links onto the subs of idriver_t. It fills the `ble` slot of _LINK_BLE (daas_static_driver_ble()), which the
 * libdaas packages leave empty (libdaas_nodrivers): this is the BLE driver of daasiot-android.
 *
 * Built only with the headers of the libdaas package (drivers_sdk/include/) and the sources of this folder.
 *
 * Contributors:
 *   Paolo Lo Giacco <plogiacco@smartlab.it>
 *   Sebastiano Meduri <sebastiano.meduri@gmail.com>
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *   Matteo Pagano <m.pagano@sebyone.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-01  Paolo Lo Giacco  libdaas 0.24.4 (source/drivers/implementation/ble/drv_ble.hpp of daas-api)
 *   2026-11-12  Lorenzo Grillo   esp32-drivers_sdk: specialized driver of the `ble` slot, built against
 *                                drivers_sdk/include of the libdaas package (no internal header of libdaas)
 *   2026-11-14  Lorenzo Grillo   daasiot-macos: macos-drivers_sdk, blew on CoreBluetooth
 *   2026-11-23  Lorenzo Grillo   daasiot-android: android-drivers_sdk, blew on the Android BLE stack
 */
#pragma once

#include "../android_drivers_sdk.hpp"

#if USE_BLE

#include <deque>
#include <mutex>

#include <blew/link.h>

#include "interface/idriver.hpp"
#include "interface/idriver_event.hpp"

#include "local_ble.hpp"

#define BLE_MAX_MTU 512

class drv_ble_t : public idriver_t {
public:
    drv_ble_t(idriver_event_t* event_incoming_connection_, int idriver_);
    ~drv_ble_t() override;

    bool init(uri_t*) override;
    bool enable(void) override;
    void disable(void) override;
    void run() override;

    int  open_sub(const uri_t*, int&) override;
    void close_sub(int) override;

    int  read(int, unsigned char*, int) override;
    int  write(int isub_, const unsigned char* buff, int size) override;
    bool flush(int isub_, int size) override;
    int  rx_data_ready(int) override;

    link_t   get_link() override { return _LINK_BLE; }
    unsigned get_mtu() override { return BLE_MAX_MTU; }
    errno_t  get_status() override { return 0; }
    unsigned int get_max_subs() override { return _MAX_BT_CONNECTIONS; }

    bool is_active() override { return _state == _ACTIVE; }
    bool is_opened() override;

    int  send_discovery_beacon(const char* packet, uint32_t packet_size) override;
    int  check_discovery_beacon(char* packet) override;
    bool check_sub_closed_connection(int isub_) override;

    // channel-plans (spec §11.3)
    cx_channel_declaration_t channel_declaration() const override;
    bool channel_sample(int isub_, cx_driver_sample_t& out) override;

private:
    struct sub_connection_t {
        blew::link::Link        link;
        std::deque<blew::Bytes> rx;           // received messages, front one possibly partly read
        size_t                  rx_offset = 0;
        bool                    in_use = false;
        bool                    discovery = false;   // our outgoing discovery link
        bool                    announced = false;   // incoming_request already raised (or not needed)
        bool                    pending = false;     // to be reported by run(), once the auth packet is in
        uint32_t                generation = 0;      // bumped on every (re)use: stale callbacks are ignored
    };

    // Bytes received on the sub, expects _mtx held.
    static size_t available_locked(const sub_connection_t& s);

    int  allocate_sub(const blew::link::Link& link, bool discovery, bool announced);
    void release_sub(int isub);

    idriver_event_t* event_incoming_connection;
    unsigned         _idriver;
    drv_state_t      _state;
    blew::link::Node _node;

    std::mutex       _mtx;
    sub_connection_t _subs[_MAX_BT_CONNECTIONS];

    uint8_t          discovery_buffer[DISCOVERY_PCKT_SIZE];
    bool             discovery_pending;
    blew::link::Link discovery_link;
};

#endif // USE_BLE
