/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: drv_template.hpp — daasiot-android, android-drivers-sdk
 *
 * Template of a specialized driver: the smallest idriver_t that builds against drivers_sdk/include of a libdaas
 * package and fills a free static slot (here `uartx`, _LINK_UART). Copy the folder to <id>/, rename template -> <id>,
 * implement the medium (see the driver guide, docs/shared/android-drivers-sdk_<code>.md) and add the driver to
 * CMakeLists.txt and to libdaas-drivers.lock.json.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Changelog:
 *   2026-11-19  l.grillo@sebyone.it  creation (from the template of the esp32-drivers-sdk of daasiot-esp32)
 *   2026-11-24  l.grillo@sebyone.it  daasiot-android (from the template of the macos-drivers-sdk of daasiot-macos)
 */
#pragma once

#if USE_TEMPLATE

#include <mutex>

#include "interface/idriver.hpp"
#include "interface/idriver_event.hpp"
#include "interface/ifuri.hpp"

// The address of a peer on the medium, validated once (created = true only when valid).
class uri_template_t : public ifuri_t {
public:
    explicit uri_template_t(const char* suri);
    unsigned id = 0;
};

class drv_template_t : public idriver_t {
public:
    drv_template_t(idriver_event_t* ev, int idx) : event_(ev), index_(idx) {}

    bool init(uri_t* l_uri) override;
    bool enable(void) override;
    void disable(void) override;
    void run() override;                                  // never blocks
    bool is_active() override { return active_; }
    bool is_opened() override { return false; }

    link_t get_link() override { return _LINK_UART; }
    unsigned get_mtu() override { return 240; }
    errno_t get_status() override { return 0; }
    unsigned int get_max_subs() override { return 4; }

    int  open_sub(const uri_t* r_uri, int& i_sub) override;
    void close_sub(int isub) override;
    int  read(int isub, unsigned char* buff, int max_size) override;
    int  write(int isub, const unsigned char* buff, int size) override;   // all or nothing
    bool flush(int isub, int size) override;
    int  rx_data_ready(int isub) override;

    int check_discovery_beacon(char* packet) override;
    int send_discovery_beacon(const char* packet, uint32_t packet_size) override;

    cx_channel_declaration_t channel_declaration() const override;

private:
    [[maybe_unused]] idriver_event_t* event_;   // the events of a real medium (incoming connections)
    [[maybe_unused]] int index_;
    bool active_ = false;
    std::mutex mtx_;
};

#endif // USE_TEMPLATE
