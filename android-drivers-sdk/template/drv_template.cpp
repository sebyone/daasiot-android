/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: drv_template.cpp — daasiot-android, android-drivers-sdk
 *
 * Template of a specialized driver (see drv_template.hpp): every method is a placeholder that refuses the operation,
 * so a node that links it keeps working on its other links. The last line is the entry of the static slot.
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
#if USE_TEMPLATE

#include <cstdlib>

#include "drv_template.hpp"
#include "template_version.hpp"

#define DAAS_LOG_DEFAULT_CAT DAAS_CAT_DRV
#include "helpers/logger.hpp"
#include "drivers/type/driver_entry.hpp"

uri_template_t::uri_template_t(const char* suri) {
    char* end = nullptr;
    unsigned long v = suri ? strtoul(suri, &end, 10) : 0;
    created = suri && end && *end == '\0' && v >= 1 && v <= 254;   // TODO: the address form of the medium
    id = (unsigned)v;
}

bool drv_template_t::init(uri_t* l_uri) { enabled_uri = l_uri; return l_uri != nullptr; }   // TODO: open the medium
bool drv_template_t::enable(void) { active_ = true; return true; }
void drv_template_t::disable(void) { active_ = false; }
void drv_template_t::run() {}                    // TODO: accept incoming subs (event_->incoming_request), poll the medium

int  drv_template_t::open_sub(const uri_t*, int&) { LOGW("[TEMPLATE] open_sub not implemented"); return -1; }
void drv_template_t::close_sub(int) {}
int  drv_template_t::read(int, unsigned char*, int) { return 0; }
int  drv_template_t::write(int, const unsigned char*, int) { return -1; }
bool drv_template_t::flush(int, int) { return false; }
int  drv_template_t::rx_data_ready(int) { return -1; }

int drv_template_t::check_discovery_beacon(char*) { return -1; }
int drv_template_t::send_discovery_beacon(const char*, uint32_t) { return -1; }

cx_channel_declaration_t drv_template_t::channel_declaration() const { return probe_default_declaration(); }

DAAS_DRIVER(uartx, drv_template_t, uri_template_t, _LINK_UART, "TEMPLATE", DAAS_DRIVER_VERSION_TEMPLATE)

#endif // USE_TEMPLATE
