/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_daas.cpp
 *
 * Implementation of daas::sdk::Node, NodeConfig and InMemoryInbox.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * plogiacco@sebyone.it - design, implementation and documentation
 * pierpaolosestito-dev - daas::sdk of daasiot-esp32, the model of Node, NodeConfig, EventRouter
 * l.grillo@sebyone.it - daas::sdk of daasiot-esp32: persistence through IDepot, the model of the restore
 *
 * Changelog:
 * 08/11/2026 - plogiacco@sebyone.it - creation (daas_sdk.cpp): port of daas::sdk of daasiot-esp32
 * 08/11/2026 - plogiacco@sebyone.it - attack panic flag, persistence on the callback nodeConnectedToNetwork
 * 08/11/2026 - plogiacco@sebyone.it - one level of wrapping: DaasAPI owned by the node, DDO helpers moved here from dw_gnu, android_daas.cpp
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 */

#include "android_daas.hpp"

#ifdef DAASIOT_ANDROID_BLE
#include "daasiot_android/ble.h"
#endif

namespace daas { namespace sdk {

bool NodeConfig::valid() const {
    for (const auto& driver : drivers) if (driver.uri.empty() && driver.port == 0) return false;
    for (const auto& mapping : mappings) if (mapping.uri.empty()) return false;
    return true;
}

/* ---- InMemoryInbox ------------------------------------------------------------------------------- */

void InMemoryInbox::add(InboxRecord record) {
    if (capacity_ == 0) return;
    while (records_.size() >= capacity_) records_.pop_front();
    records_.push_back(std::move(record));
}

/* ---- Node ---------------------------------------------------------------------------------------- */

namespace {

/** The hver of a new node: the argument, else NodeConfig::parameters.hardwareVersion, else the callback / the
 *  detection of the host (hardwareVersion()); at most kHardwareVersionMax characters (libdaas keeps 7). */
std::string chooseHardware(const NodeConfig& config, const char* argument) {
    std::string version = argument != nullptr && *argument != '\0' ? std::string(argument)
                        : !config.parameters.hardwareVersion.empty() ? config.parameters.hardwareVersion
                        : sdk::hardwareVersion();
    return version.substr(0, kHardwareVersionMax);
}

} // namespace

Node::Node(NodeConfig config, const char* hardwareVersion)
    : config_(std::move(config)), autoSave_(config_.parameters.persistence.autoSave),
      hardware_(chooseHardware(config_, hardwareVersion)), api_(std::make_unique<DaasAPI>(&events_, hardware_.c_str())) {
    hookEvents();
    resolveDrivers();
    if (config_.valid()) initializationResult_ = apply();
}

Node::Node(NodeConfig config, IDepot* depot, const char* hardwareVersion)
    : config_(std::move(config)), depot_(depot), autoSave_(config_.parameters.persistence.autoSave),
      hardware_(chooseHardware(config_, hardwareVersion)), api_(std::make_unique<DaasAPI>(&events_, hardware_.c_str())) {
    hookEvents();
    resolveDrivers();
    // Ask the depot first whether it holds anything (open in read mode fails when empty), so that libdaas is
    // asked to load only a configuration that exists.
    const bool saved = depot_ != nullptr && depot_->open(true) && depot_->close();
    if (saved && api_->loadConfiguration(depot_)) {
        restored_ = true;
        publishLocalSid();
        initializationResult_ = applyPolicies();
        return;
    }
    if (!config_.valid()) return;
    initializationResult_ = apply();
    if (initializationResult_ == ERROR_NONE) save();
}

Node::~Node() { stop(); }

void Node::hookEvents() {
    events_.setQueued(config_.parameters.eventsQueued);
    // The SID/DIN assigned by a join are saved as soon as the node is a member (persistence on callback).
    // The SID they carry is also the one the BLE driver publishes for the discovery filters of the other nodes.
    events_.onFirst(EventType::NodeConnectedToNetwork, [this](const Event&) { publishLocalSid(); savePending_ = true; });
    // The node isolated by the attack panic defense: the app restarts its process (see panicked()).
    events_.onFirst(EventType::AttackPanic, [this](const Event&) { panic_ = true; });
}


std::string anyUri(link_t link, uint16_t port) {
    if (link == _LINK_INET6) return "[::]:" + std::to_string(port);
    return "0.0.0.0:" + std::to_string(port);
}

void Node::resolveDrivers() {
    drivers_.clear();
    for (const auto& spec : config_.drivers) {
        const link_t link = spec.link == _LINK_NONE ? _LINK_INET4 : spec.link;   // no host detection on Android
        drivers_.push_back({link, spec.uri.empty() ? anyUri(link, spec.port) : spec.uri});
    }
}

void Node::publishLocalSid() {
#ifdef DAASIOT_ANDROID_BLE
    // As dw_esp32::doInit() of daasiot-esp32: the SID that blew publishes (INFO) for the discovery filters.
    daas_ble_set_local_sid(static_cast<uint64_t>(api_->getLocalSid()));
#endif
}

daas_error_t Node::apply() {
    daas_error_t result = api_->doInit(config_.sid, config_.din);
    if (result != ERROR_NONE) return result;
    publishLocalSid();
    if (drivers_.empty() && !config_.drivers.empty()) return ERROR_CANNOT_INITIALIZE;
    for (const auto& driver : drivers_) {
        result = api_->enableDriver(driver.link, driver.uri.c_str());
        if (result != ERROR_NONE) return result;
    }
    result = applyPolicies();
    if (result != ERROR_NONE) return result;
    const link_t fallback = drivers_.empty() ? _LINK_INET4 : drivers_.front().link;
    for (const auto& mapping : config_.mappings) {
        const link_t link = mapping.link == _LINK_NONE ? fallback : mapping.link;
        result = mapping.securityKey.empty()
            ? api_->map(mapping.din, link, mapping.uri.c_str())
            : api_->map(mapping.din, link, mapping.uri.c_str(), mapping.securityKey.c_str());
        if (result != ERROR_NONE) return result;
    }
    if (config_.hasJoin) return api_->join(config_.join.sid, config_.join.link, config_.join.timeout);
    return ERROR_NONE;
}

daas_error_t Node::applyPolicies() {
    // Every parameter of libdaas (android_defaults.hpp, NodeConfig::parameters), then the options of the configuration
    // (JSON "options"), which win; a restored node gets them too: libdaas does not save them in the depot.
    const LibdaasParameters& p = config_.parameters.libdaas;
    const std::pair<option_t, uint32_t> parameters[] = {
        {option_set_ddo_rx_buffer_size, p.ddoRxBufferSize}, {option_set_ddo_tx_buffer_size, p.ddoTxBufferSize},
        {option_set_ddo_tx_fifo_policy, p.ddoTxFifoPolicy}, {option_set_rt_buffer_size, p.rtBufferSize},
        {option_set_max_ddo_retry, p.maxDdoRetry},          {option_set_unsynced_hold_ms, p.unsyncedHoldMs},
        {option_set_route_timeout, p.routeTimeoutMs},       {option_set_route_ttl, p.routeTtl},
        {option_set_fetch_timeout, p.fetchTimeoutMs},       {option_set_channel_drop_policy, p.channelDropPolicy},
        {option_set_time_reference, p.timeReference},       {option_set_protocol_max, p.protocolMax}};
    daas_error_t result = ERROR_NONE;
    for (const auto& parameter : parameters) {
        result = api_->setOptions(parameter.first, parameter.second);
        if (result != ERROR_NONE) return result;
    }
    api_->setAcceptRequestsLevel(p.acceptRequests);
    result = api_->setDDOPolicy(p.ddoPolicy);
    if (result != ERROR_NONE) return result;
    for (const auto& option : config_.options) {
        result = api_->setOptions(option.first, option.second);
        if (result != ERROR_NONE) return result;
    }
    if (config_.hasAcceptRequests) api_->setAcceptRequestsLevel(config_.acceptRequests);
    if (config_.hasDiscoveryState) api_->setDiscoveryState(config_.discoveryState);
    if (config_.hasDdoPolicy) {
        result = api_->setDDOPolicy(config_.ddoPolicy);
        if (result != ERROR_NONE) return result;
    }
    if (config_.hasAtsMaxError) api_->setATSMaxError(config_.atsMaxError);
    return ERROR_NONE;
}

bool Node::save() {
    if (depot_ == nullptr) return false;
    savePending_ = false;
    lastSave_ = std::chrono::steady_clock::now();
    return api_->storeConfiguration(depot_);
}

void Node::changed() {
    if (depot_ != nullptr && autoSave_) save();
}

daas_error_t Node::start() {
    if (initializationResult_ != ERROR_NONE) return initializationResult_;
    if (running_) return ERROR_NONE;
    const daas_error_t result = api_->doPerform(config_.performMode);
    running_ = (result == ERROR_NONE);
    return result;
}

daas_error_t Node::perform() {
    if (!running_ || config_.performMode != PERFORM_CORE_NO_THREAD) return ERROR_NONE;
    return api_->doPerform(PERFORM_CORE_NO_THREAD);
}

size_t Node::service(std::chrono::milliseconds wait) {
    if (running_ && config_.performMode == PERFORM_CORE_NO_THREAD) {
        api_->doPerform(PERFORM_CORE_NO_THREAD);
        wait = std::chrono::milliseconds(0);   // the core runs only inside doPerform: do not sleep here
    }
    const size_t count = events_.dispatch(wait);
    if (savePending_ && autoSave_) save();
    // Periodic save (parameters.persistence.savePeriodMs): the queues and the map a crash would lose are bounded.
    const uint32_t period = config_.parameters.persistence.savePeriodMs;
    if (period > 0 && depot_ != nullptr && running_
        && std::chrono::steady_clock::now() - lastSave_ >= std::chrono::milliseconds(period))
        save();
    return count;
}

daas_error_t Node::stop() {
    if (!running_) return ERROR_NONE;
    // Saved before doEnd, which empties the queues: libdaas stores the RX queues (DDOs not pulled yet) and the
    // user DDOs of the TX queue not sent yet with the configuration, and restores both (libdaas >= 0.24.7).
    if ((savePending_ && autoSave_) || config_.parameters.persistence.saveOnStop) save();
    const daas_error_t result = api_->doEnd();
    if (result == ERROR_NONE) running_ = false;
    events_.wake();
    return result;
}

daas_error_t Node::pushBytes(din_t din, const uint8_t* data, size_t size, typeset_t typeset, bool multipart) {
    DDO ddo(typeset);
    if (size > 0 && ddo.setPayload(data, static_cast<uint32_t>(size)) != size) return ERROR_INVALID_ARGUMENT;
    return api_->push(din, &ddo, multipart);
}

daas_error_t Node::pushText(din_t din, const std::string& text, typeset_t typeset) {
    return pushBytes(din, reinterpret_cast<const uint8_t*>(text.data()), text.size(), typeset);
}

daas_error_t Node::pull(din_t din, DdoPtr& owner) {
    DDO* ddo = nullptr;
    const daas_error_t result = api_->pull(din, &ddo);
    owner.reset(ddo);
    if (result == ERROR_NONE && !owner) return ERROR_NO_DDO_PRESENT;
    return result;
}

daas_error_t Node::pull(din_t din, InboxRecord& record) {
    DdoPtr ddo;
    const daas_error_t result = pull(din, ddo);
    if (result != ERROR_NONE) return result;
    record.din = din;
    record.typeset = ddo->getTypeset();
    record.origin = ddo->getOrigin();
    record.timestamp = ddo->getTimestamp();
    const uint8_t* data = ddo->getPayloadPtr();
    record.payload.assign(data, data + ddo->getPayloadSize());
    return ERROR_NONE;
}

std::string Node::pullText(din_t din) {
    InboxRecord record;
    return pull(din, record) == ERROR_NONE ? record.text() : std::string();
}

uint32_t Node::available(din_t din) {
    uint32_t count = 0;
    return api_->availablesPull(din, count) == ERROR_NONE ? count : 0;
}

InMemoryInbox& Node::enableInbox(size_t capacity) {
    const bool first = !inbox_;
    inbox_ = std::make_unique<InMemoryInbox>(capacity);
    if (first) {
        events_.on(EventType::DdoReceived, [this](const Event& event) {
            InboxRecord record;
            while (pull(event.din, record) == ERROR_NONE) { inbox_->add(std::move(record)); record = InboxRecord(); }
        });
    }
    return *inbox_;
}

daas_error_t Node::map(din_t din, link_t link, const std::string& uri, const std::string& key) {
    const daas_error_t result = key.empty() ? api_->map(din, link, uri.c_str()) : api_->map(din, link, uri.c_str(), key.c_str());
    if (result == ERROR_NONE) changed();
    return result;
}

daas_error_t Node::unmap(din_t din) {
    const daas_error_t result = api_->remove(din);
    if (result == ERROR_NONE) changed();
    return result;
}

daas_error_t Node::enableDriver(link_t link, const std::string& uri) {
    const daas_error_t result = api_->enableDriver(link, uri.c_str());
    if (result == ERROR_NONE) changed();
    return result;
}

daas_error_t Node::disableDriver(link_t link) {
    const daas_error_t result = api_->disableDriver(link);
    if (result == ERROR_NONE) changed();
    return result;
}

}} // namespace daas::sdk
