/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_daas.hpp
 *
 * Main header of android_daas, the daas SDK for Android (daasiot-android), and the only include a native part of an
 * application needs. daas::sdk::Node: the only wrapper of libdaas in android_daas. It owns the DaasAPI of the process
 * and adds the configuration (NodeConfig), the events on the application thread, the persistence through an IDepot
 * (restore, save on every change), the local SID handed to the BLE driver and the DDO helpers; every other function
 * of libdaas is Node::api(). Also NodeConfig, InboxRecord, InMemoryInbox, DdoPtr; it includes the events and the
 * persistence on files. The Java side (it.sebyone.daas.DaasNode) is a binding of this Node (jni/android_daas_jni.cpp).
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
 * 08/11/2026 - plogiacco@sebyone.it - creation (daas_sdk.h): port of daas::sdk of daasiot-esp32 on dw_gnu
 * 08/11/2026 - plogiacco@sebyone.it - typed payloads, EventRouter::eventFd, Node::panicked
 * 08/11/2026 - plogiacco@sebyone.it - one level of wrapping, android_daas.hpp: Node owns DaasAPI (dw_gnu removed), push(DDO&), pull(DdoPtr&), availableLinks
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 */

#ifndef _ANDROID_DAAS_H__
#define _ANDROID_DAAS_H__

#include "android_defaults.hpp"
#include "android_events.hpp"

#include <daas.hpp>
#include <daas_types.hpp>

#include <atomic>
#include <chrono>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace daas { namespace sdk {

/** @brief Owner of a DDO returned by pull(): deleted when it goes out of scope. */
using DdoPtr = std::unique_ptr<DDO>;

/** @brief A remote node known in advance. link _LINK_NONE: the link of the first local driver. */
struct MappingConfig { din_t din = 0; link_t link = _LINK_NONE; std::string uri; std::string securityKey; };
struct JoinConfig { din_t sid = 0; link_t link = _LINK_NONE; uint32_t timeout = ANDROID_DAAS_JOIN_TIMEOUT_MS; };

/**
 * @brief A local driver of the configuration. link _LINK_NONE: INET4. Empty uri: every address of the link on
 *        @p port (anyUri). For _LINK_BLE the uri is the blew node id, 8 hexadecimal digits (e.g. "000000C8").
 */
struct DriverSpec { link_t link = _LINK_NONE; std::string uri; uint16_t port = 0; };
/** @brief A driver enabled by the node: its link and its local uri_t. */
struct DriverConfig { link_t link; std::string uri; };

struct NodeConfig {
    din_t sid = 0;
    din_t din = 0;
    std::vector<DriverSpec> drivers;
    std::vector<MappingConfig> mappings;
    std::vector<std::pair<option_t, uint32_t>> options;
    bool hasAcceptRequests = false;
    accept_request_policy_t acceptRequests = trust_mapped_and_same_network;
    bool hasDiscoveryState = false;
    discovery_state_t discoveryState = discovery_sender_only;
    bool hasDdoPolicy = false;
    ddo_policy_t ddoPolicy = ddo_policy_skip_on_failure;
    bool hasJoin = false;
    JoinConfig join;
    bool hasAtsMaxError = false;
    int32_t atsMaxError = 0;
    performs_mode_t performMode = ANDROID_DAAS_PERFORM_MODE;   // PERFORM_CORE_THREAD: the normal choice on Android
    /** @brief Queues, every option and policy of libdaas, persistence, events, hardware version: the defaults of
     *         android_defaults.hpp; "options", acceptRequests, ddoPolicy above win over them. */
    Parameters parameters;
    bool valid() const;
};

/** @brief A DDO received, copied out of libdaas. */
struct InboxRecord {
    din_t din = 0; typeset_t typeset = 0; din_t origin = 0; stime_t timestamp = 0;
    std::vector<uint8_t> payload;
    std::string text() const { return std::string(payload.begin(), payload.end()); }
    /** @brief Copies the payload into a trivially copyable @p value; false if the sizes differ. */
    template <typename T> bool as(T& value) const {
        static_assert(std::is_trivially_copyable<T>::value, "as<T>: T must be trivially copyable");
        if (payload.size() != sizeof(T)) return false;
        std::memcpy(&value, payload.data(), sizeof(T));
        return true;
    }
};

class InMemoryInbox {
public:
    explicit InMemoryInbox(size_t capacity = 32) : capacity_(capacity) {}
    void add(InboxRecord record);
    const std::deque<InboxRecord>& messages() const { return records_; }
    void clear() { records_.clear(); }
private:
    size_t capacity_;
    std::deque<InboxRecord> records_;
};

/** @brief The uri_t of every address of @p link on @p port: "0.0.0.0:<port>" (INET4), "[::]:<port>" (INET6). */
std::string anyUri(link_t link, uint16_t port);

/**
 * @brief A daas node on Android: the only wrapper of libdaas in android_daas. It owns the DaasAPI of the process and
 *        adds what Android needs: the configuration (code), the events delivered on the application thread, the
 *        persistence through an IDepot, the local SID of the BLE driver, the DDO helpers. Everything else of libdaas
 *        is api(), the DaasAPI itself (libdaas-refs).
 *
 * @code
 *   daas::persistence::file_depot depot(filesDir + "/daas");   // the files folder of the app (Context.getFilesDir)
 *   daas::sdk::NodeConfig config;            // used only the first time (nothing in the depot)
 *   config.sid = 100; config.din = 101;
 *   config.drivers.push_back({_LINK_BLE, "00000065", 0});
 *   config.drivers.push_back({_LINK_INET4, "", 2101});
 *   daas::sdk::Node node(config, &depot);    // restores the saved configuration, or applies config
 *   node.events().on(daas::sdk::EventType::DdoReceived, [&](const daas::sdk::Event& e) {
 *       daas::sdk::InboxRecord in; while (node.pull(e.din, in) == ERROR_NONE) handle(in); });
 *   node.start();
 *   for (;;) node.service(std::chrono::milliseconds(200));   // events, persistence (Java: DaasNode does it)
 * @endcode
 */
class Node {
public:
    /** @param hardwareVersion the hver of the node; nullptr: NodeConfig::parameters.hardwareVersion, else the
     *        callback of setHardwareVersionProvider(), else detectHardware() (android_defaults.hpp). */
    explicit Node(NodeConfig config, const char* hardwareVersion = nullptr);
    /**
     * @brief Node persisted through an IDepot (e.g. daas::persistence::file_depot).
     * @details If the depot holds a configuration (DaasAPI::loadConfiguration: SID, DIN, drivers, map) the node
     *          is restored from it and only the policies/options of @p config are applied; otherwise @p config is
     *          applied and saved. Afterwards every change of identity or map made through the Node (map, unmap,
     *          enableDriver, disableDriver, the join completed, nodeConnectedToNetwork) is saved again.
     */
    Node(NodeConfig config, IDepot* depot, const char* hardwareVersion = nullptr);
    ~Node();
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    /** @brief The DaasAPI of the node, for every function of libdaas the Node does not wrap. */
    DaasAPI& api() { return *api_; }
    EventRouter& events() { return events_; }
    const NodeConfig& config() const { return config_; }
    daas_error_t initializationResult() const { return initializationResult_; }
    bool restored() const { return restored_; }
    IDepot* depot() const { return depot_; }
    /** @brief The hardware version given to libdaas (hver). */
    const std::string& hardware() const { return hardware_; }
    void setAutoSave(bool enabled) { autoSave_ = enabled; }
    /** @brief Saves the current configuration (SID, DIN, drivers, map) in the IDepot of the node. */
    bool save();

    /* ---- life cycle ------------------------------------------------------------------------------------- */
    daas_error_t start();
    /** @brief One doPerform (real-time mode); in threaded mode it does nothing. */
    daas_error_t perform();
    /**
     * @brief Work of the application thread: a core cycle in real-time mode, the queued events, the pending
     *        save. Waits up to @p wait for an event in threaded mode. @return the events dispatched.
     */
    size_t service(std::chrono::milliseconds wait = std::chrono::milliseconds(0));
    daas_error_t stop();
    daas_error_t reset() { return api_->doReset(); }
    bool running() const { return running_; }
    /**
     * @brief True after an attackPanic event: the node is isolated by the attack panic defense (guide §13) until it
     *        is set up again. A node cannot be set up again in the same process once a peer has opened a channel
     *        (doEnd + doInit crashes or leaves the node deaf), so the way out on Android is to restart the process
     *        of the app: the node starts again restored from its depot.
     */
    bool panicked() const { return panic_; }

    /* ---- data ------------------------------------------------------------------------------------------- */
    daas_error_t pushBytes(din_t din, const uint8_t* data, size_t size, typeset_t typeset = 1, bool multipart = false);
    daas_error_t pushText(din_t din, const std::string& text, typeset_t typeset = 1);
    /** @brief Pushes a trivially copyable value as the payload (its bytes, in the byte order of the host). */
    template <typename T> daas_error_t pushValue(din_t din, const T& value, typeset_t typeset) {
        static_assert(std::is_trivially_copyable<T>::value, "pushValue<T>: T must be trivially copyable");
        return pushBytes(din, reinterpret_cast<const uint8_t*>(&value), sizeof(T), typeset);
    }
    /** @brief Pushes a DDO built by the application (libdaas copies it: @p ddo can be reused). */
    daas_error_t push(din_t din, DDO& ddo, bool multipart = false) { return api_->push(din, &ddo, multipart); }
    /** @brief Pulls the next DDO of @p din into @p record; ERROR_NO_DDO_PRESENT if none. */
    daas_error_t pull(din_t din, InboxRecord& record);
    /** @brief Pulls the next DDO of @p din into an owner (no delete to remember); ERROR_NO_DDO_PRESENT if none. */
    daas_error_t pull(din_t din, DdoPtr& ddo);
    std::string pullText(din_t din);
    uint32_t available(din_t din);
    InMemoryInbox& enableInbox(size_t capacity = ANDROID_DAAS_INBOX_CAPACITY);

    /* ---- drivers, map, network (saved in the depot) ----------------------------------------------------- */
    daas_error_t map(din_t din, link_t link, const std::string& uri, const std::string& key = "");
    daas_error_t unmap(din_t din);
    daas_error_t enableDriver(link_t link, const std::string& uri);
    daas_error_t disableDriver(link_t link);
    /** @brief The drivers enabled by the configuration. */
    const std::vector<DriverConfig>& drivers() const { return drivers_; }

    node_map_list_t mappedNodes() { return api_->getMappedNodes(); }
    network_info_list_t knownNodes() { return api_->getKnownNodes(); }
    Vector<din_t> allNodes(din_t sid) { return api_->getAllNodes(sid); }
    node_state_t state() { return api_->getNodeState(); }
    /** @brief True once the node is synchronised (dATS): the user messages leave the queue. */
    bool isAvailable() { return api_->getNodeState() >= node_state_synchronized; }
    din_t sid() { return api_->getLocalSid(); }
    din_t din() { return api_->getLocalDin(); }
    node_info_t status() { return api_->getStatus(); }
    const node_info_t& status(din_t din) { return api_->status(din); }
    const node_info_t& fetch(din_t din, uint16_t options = 0) { return api_->fetch(din, options); }
    daas_error_t locate(din_t din, uint32_t timeout = 1000, uint32_t ttl = 10) { return api_->locate(din, timeout, ttl); }
    daas_error_t locate(feature_t feature, uint32_t timeout = 1000, uint32_t ttl = 10) { return api_->locate(feature, timeout, ttl); }
    daas_error_t discovery(link_t link = _LINK_NONE) { return api_->discovery(link); }
    /** @brief Joins a network; the SID/DIN it assigns are handed to the BLE driver and saved when it completes. */
    daas_error_t join(din_t sid = 0, link_t link = _LINK_NONE, uint32_t timeout = 5000) { return api_->join(sid, link, timeout); }
    daas_error_t createNetwork() { return api_->createNetwork(); }
    daas_error_t unbindNetwork() { return api_->unbindNetwork(); }

    bool openStream(din_t din, uint32_t queueSize = 0, uint32_t timeout = 1000, uint32_t retry = 3) { return api_->use(din, queueSize, timeout, retry); }
    bool closeStream(din_t din) { return api_->end(din); }

    /* ---- features: implementations of the application ---------------------------------------------------- */
    daas_error_t addFeature(IFeatureStorage* storage) { return api_->addNodeFeatures(STORAGE, storage); }
    daas_error_t addFeature(IFeatureCompute* compute) { return api_->addNodeFeatures(COMPUTE, compute); }
    daas_error_t removeFeature(feature_t feature) { return api_->removeNodeFeatures(feature); }
    features_list features() { return api_->getNodeFeatures(); }
    bool hasFeature(din_t din, feature_t feature) { return api_->nodeHasFeature(din, feature); }
    Vector<din_t> featureProviders(feature_t feature) { return api_->getFeatureProviders(feature); }
    daas_error_t requestFeature(din_t din, const feature_rq_t& rq, feature_rs_t& rs, uint32_t timeout = 1000) { return api_->requestFeature(din, rq, rs, timeout); }

    /* ---- diagnostics ------------------------------------------------------------------------------------ */
    daas_error_t frisbee(din_t din) { return api_->frisbee(din); }
    daas_error_t frisbeeIcmp(din_t din, uint32_t timeout = 1000, uint32_t retry = 3) { return api_->frisbeeICMP(din, timeout, retry); }
    daas_error_t frisbeeDperf(din_t din, uint32_t packets = 10, uint32_t block = 1024 * 1024, uint32_t period = 0) { return api_->frisbeeDPERF(din, packets, block, period); }
    dperf_info_result dperfResult() { return api_->getFrisbeeResultDPERF(); }
    uint64_t syncedTimestamp() { return api_->getSyncedTimestamp(); }
    const char* version() { return api_->getVersion(); }
    const char* buildInfo() { return api_->getBuildInfo(); }
    const char* availableDrivers() { return api_->listAvailableDrivers(); }
    const char* errorString(daas_error_t error) { return api_->errorToString(error); }

private:
    daas_error_t apply();
    daas_error_t applyPolicies();
    void resolveDrivers();
    void changed();
    void hookEvents();
    void publishLocalSid();
    NodeConfig config_;
    IDepot* depot_ = nullptr;
    bool restored_ = false;
    bool autoSave_ = true;
    std::atomic<bool> savePending_{false};
    std::atomic<bool> panic_{false};
    std::chrono::steady_clock::time_point lastSave_ = std::chrono::steady_clock::now();
    std::string hardware_;               // before api_: given to its constructor
    EventRouter events_;                 // before api_: libdaas keeps a pointer to it
    std::unique_ptr<DaasAPI> api_;
    std::vector<DriverConfig> drivers_;
    bool running_ = false;
    daas_error_t initializationResult_ = ERROR_INVALID_DME;
    std::unique_ptr<InMemoryInbox> inbox_;
};

}} // namespace daas::sdk

// The rest of the SDK, so that android_daas.hpp is the only include of a native application (this header includes
// android_daas.hpp in turn: its guard is already defined, Node is already declared).
#include "android_file_depot.hpp"  // daas::persistence::file_depot (IDepot on files)

#endif // _ANDROID_DAAS_H__
