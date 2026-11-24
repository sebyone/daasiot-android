/*
 * daas 2019, 2026 (@) Sebyone Srl
 *
 * File: android_defaults.hpp
 *
 * The defaults of android_daas: the sizes of the RX and TX queues and every operating parameter of libdaas
 * (DaasAPI::setOptions, the policies, the perform mode), of the persistence (when a node saves, the modes and
 * the quota of the files) and of the events and service; and the callback that tells the hardware the node
 * runs on (the hver of DaasAPI), with its default implementation from the resources of the host.
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
 *
 * Changelog:
 * 08/11/2026 - plogiacco@sebyone.it - creation: defaults of the queues, of libdaas and of the persistence; hardware version callback
 * 14/11/2026 - l.grillo@sebyone.it - daasiot-macos: port to macOS (from gnu_daas of daasiot-gnu 0.2.1)
 * 23/11/2026 - l.grillo@sebyone.it - daasiot-android: port to Android (from macos_daas of daasiot-macos 0.1.0)
 * 24/11/2026 - l.grillo@sebyone.it - no namespace: the API of android_daas is global, like libdaas
 */

#ifndef _ANDROID_DEFAULTS_H__
#define _ANDROID_DEFAULTS_H__

/*
 * Every value below can be changed in three ways, from the widest to the narrowest:
 *   1. a file of the project, named by ANDROID_DAAS_USER_CONFIG (CMake: -DANDROID_DAAS_USER_CONFIG=<path>/my_config.hpp),
 *      that #defines some of these macros: android_daas and the application are built with it;
 *   2. one macro on the command line (-DANDROID_DAAS_DDO_RX_BUFFER_SIZE=8192);
 *   3. at run time, in NodeConfig::parameters (initialized from these macros) or with NodeConfig::options
 *      (DaasNode.Config.option() in Java), which win over the parameters.
 * The defaults are those of libdaas 0.24.6–0.24.8 (DaasAPI::setOptions, libdaas-refs), so a node built with no
 * change behaves as libdaas alone.
 */
#ifdef ANDROID_DAAS_USER_CONFIG
#include ANDROID_DAAS_USER_CONFIG
#endif

/* ---- libdaas: queues and buffers (setOptions) ------------------------------------------------------------ */
#ifndef ANDROID_DAAS_DDO_RX_BUFFER_SIZE
#define ANDROID_DAAS_DDO_RX_BUFFER_SIZE 1024u      ///< bytes of received DDOs waiting for pull (RX queues)
#endif
#ifndef ANDROID_DAAS_DDO_TX_BUFFER_SIZE
#define ANDROID_DAAS_DDO_TX_BUFFER_SIZE 1024u      ///< bytes of DDOs queued for sending (TX queue)
#endif
#ifndef ANDROID_DAAS_DDO_TX_FIFO_POLICY
#define ANDROID_DAAS_DDO_TX_FIFO_POLICY 0u         ///< 1: a full TX queue drops the oldest DDOs; 0: push fails
#endif
#ifndef ANDROID_DAAS_RT_BUFFER_SIZE
#define ANDROID_DAAS_RT_BUFFER_SIZE 1024u          ///< bytes of the receive buffer of a real-time session (stream)
#endif

/* ---- libdaas: delivery, routing, discovery (setOptions) -------------------------------------------------- */
#ifndef ANDROID_DAAS_MAX_DDO_RETRY
#define ANDROID_DAAS_MAX_DDO_RETRY 10u             ///< retries of a DDO with a retry policy, 1..15
#endif
#ifndef ANDROID_DAAS_UNSYNCED_HOLD_MS
#define ANDROID_DAAS_UNSYNCED_HOLD_MS 30000u       ///< dATS: how long a user DDO waits for the node to be synchronized
#endif
#ifndef ANDROID_DAAS_ROUTE_TIMEOUT_MS
#define ANDROID_DAAS_ROUTE_TIMEOUT_MS 1000u        ///< how long a route request stays open
#endif
#ifndef ANDROID_DAAS_ROUTE_TTL
#define ANDROID_DAAS_ROUTE_TTL 10u                 ///< hops a route request can travel, 0..255
#endif
#ifndef ANDROID_DAAS_FETCH_TIMEOUT_MS
#define ANDROID_DAAS_FETCH_TIMEOUT_MS 1000u        ///< how long getAllNodes collects the answers
#endif
#ifndef ANDROID_DAAS_CHANNEL_DROP_POLICY
#define ANDROID_DAAS_CHANNEL_DROP_POLICY 0u        ///< channel_drop_policy_t: 0 refuses a connection with no free channel
#endif
#ifndef ANDROID_DAAS_TIME_REFERENCE
#define ANDROID_DAAS_TIME_REFERENCE 0u             ///< dATS: 1 makes the node the time reference of its SID
#endif
#ifndef ANDROID_DAAS_PROTOCOL_MAX
#define ANDROID_DAAS_PROTOCOL_MAX 5u               ///< highest framing negotiated: 3 fixhb-v3, 4 fixhb-v4, 5 flexhb-v5
#endif

/* ---- libdaas: policies --------------------------------------------------------------------------------- */
#ifndef ANDROID_DAAS_ACCEPT_REQUESTS
#define ANDROID_DAAS_ACCEPT_REQUESTS trust_mapped_and_same_network   ///< accept_request_policy_t
#endif
#ifndef ANDROID_DAAS_DDO_POLICY
#define ANDROID_DAAS_DDO_POLICY ddo_policy_skip_on_failure           ///< ddo_policy_t
#endif
#ifndef ANDROID_DAAS_PERFORM_MODE
#define ANDROID_DAAS_PERFORM_MODE PERFORM_CORE_THREAD                ///< performs_mode_t: the core on its own threads
#endif
#ifndef ANDROID_DAAS_JOIN_TIMEOUT_MS
#define ANDROID_DAAS_JOIN_TIMEOUT_MS 5000u                           ///< join of the configuration
#endif

/* ---- libdaas: identity of the hardware ----------------------------------------------------------------- */
#ifndef ANDROID_DAAS_HARDWARE_VERSION
#define ANDROID_DAAS_HARDWARE_VERSION ""   ///< hver given to DaasAPI (7 characters at most); "" = hardwareVersion()
#endif

/* ---- persistence (IDepot, file_depot) -------------------------------------------------------------------- */
#ifndef ANDROID_DAAS_AUTOSAVE
#define ANDROID_DAAS_AUTOSAVE 1            ///< save the configuration at every change made through the Node
#endif
#ifndef ANDROID_DAAS_SAVE_ON_STOP
#define ANDROID_DAAS_SAVE_ON_STOP 1        ///< save at stop(): the RX and TX queues not yet emptied survive a restart
#endif
#ifndef ANDROID_DAAS_SAVE_PERIOD_MS
#define ANDROID_DAAS_SAVE_PERIOD_MS 0u     ///< also save every period while running (0: never), bounds what a crash loses
#endif
#ifndef ANDROID_DAAS_STATE_DIR_MODE
#define ANDROID_DAAS_STATE_DIR_MODE 0700   ///< mode of the folders made by makeDirectories / stateDirectory
#endif
#ifndef ANDROID_DAAS_STATE_FILE_MODE
#define ANDROID_DAAS_STATE_FILE_MODE 0600  ///< mode of the records of file_depot
#endif
#ifndef ANDROID_DAAS_STORAGE_QUOTA
#define ANDROID_DAAS_STORAGE_QUOTA (16ull * 1024ull * 1024ull)   ///< bytes a file feature storage of the application may take (not used by android_daas)
#endif
#ifndef ANDROID_DAAS_STORAGE_OWNER_ONLY
#define ANDROID_DAAS_STORAGE_OWNER_ONLY 0  ///< 1: only the node that stored a blob can read, see or remove it
#endif

/* ---- android_daas: events and service ---------------------------------------------------------------------- */
#ifndef ANDROID_DAAS_EVENTS_QUEUED
#define ANDROID_DAAS_EVENTS_QUEUED 1       ///< 1: listeners on the application thread (service); 0: in the callbacks
#endif
#ifndef ANDROID_DAAS_INBOX_CAPACITY
#define ANDROID_DAAS_INBOX_CAPACITY 32u    ///< DDOs kept by enableInbox()
#endif
#ifndef ANDROID_DAAS_SERVICE_PERIOD_MS
#define ANDROID_DAAS_SERVICE_PERIOD_MS 200u   ///< period of the service loop of DaasNode (Java): events, pending saves
#endif

#include <daas.hpp>
#include <daas_types.hpp>

#include <cstdint>
#include <functional>
#include <string>


/** @brief The options of libdaas (DaasAPI::setOptions), all applied by the Node at every start. */
struct LibdaasParameters {
    uint32_t ddoRxBufferSize = ANDROID_DAAS_DDO_RX_BUFFER_SIZE;
    uint32_t ddoTxBufferSize = ANDROID_DAAS_DDO_TX_BUFFER_SIZE;
    uint32_t ddoTxFifoPolicy = ANDROID_DAAS_DDO_TX_FIFO_POLICY;
    uint32_t rtBufferSize = ANDROID_DAAS_RT_BUFFER_SIZE;
    uint32_t maxDdoRetry = ANDROID_DAAS_MAX_DDO_RETRY;
    uint32_t unsyncedHoldMs = ANDROID_DAAS_UNSYNCED_HOLD_MS;
    uint32_t routeTimeoutMs = ANDROID_DAAS_ROUTE_TIMEOUT_MS;
    uint32_t routeTtl = ANDROID_DAAS_ROUTE_TTL;
    uint32_t fetchTimeoutMs = ANDROID_DAAS_FETCH_TIMEOUT_MS;
    uint32_t channelDropPolicy = ANDROID_DAAS_CHANNEL_DROP_POLICY;
    uint32_t timeReference = ANDROID_DAAS_TIME_REFERENCE;
    uint32_t protocolMax = ANDROID_DAAS_PROTOCOL_MAX;
    accept_request_policy_t acceptRequests = ANDROID_DAAS_ACCEPT_REQUESTS;
    ddo_policy_t ddoPolicy = ANDROID_DAAS_DDO_POLICY;
};

/** @brief How a Node with an IDepot saves, and the defaults of the persistence on files. */
struct PersistenceParameters {
    bool autoSave = ANDROID_DAAS_AUTOSAVE != 0;
    bool saveOnStop = ANDROID_DAAS_SAVE_ON_STOP != 0;
    uint32_t savePeriodMs = ANDROID_DAAS_SAVE_PERIOD_MS;
    uint64_t storageQuota = ANDROID_DAAS_STORAGE_QUOTA;
    bool storageOwnerOnly = ANDROID_DAAS_STORAGE_OWNER_ONLY != 0;
};

/** @brief Every parameter of a node: NodeConfig::parameters. */
struct Parameters {
    LibdaasParameters libdaas;
    PersistenceParameters persistence;
    bool eventsQueued = ANDROID_DAAS_EVENTS_QUEUED != 0;
    uint32_t inboxCapacity = ANDROID_DAAS_INBOX_CAPACITY;
    uint32_t servicePeriodMs = ANDROID_DAAS_SERVICE_PERIOD_MS;
    /** @brief The hver of the node; empty: hardwareVersion() (the callback, else the detection of the host). */
    std::string hardwareVersion = ANDROID_DAAS_HARDWARE_VERSION;
};

/** @brief The longest hardware version libdaas keeps (core: l_hver[8]); longer strings are cut. */
constexpr size_t kHardwareVersionMax = 7;

/**
 * @brief The callback that tells the hardware a node runs on: its result is the hver advertised to the network
 *        (DaasAPI(event, hver)), cut to kHardwareVersionMax characters. The application installs it before
 *        creating its nodes; without it, detectHardware() is used.
 */
using HardwareVersionProvider = std::function<std::string()>;
void setHardwareVersionProvider(HardwareVersionProvider provider);
/** @brief The hver of a new node: the callback, else detectHardware(); at most kHardwareVersionMax characters. */
std::string hardwareVersion();
/**
 * @brief The default implementation of the callback: "AND" and the architecture of the
 *        device (uname): "AND-A64" on arm64-v8a, "AND-A32" on armeabi-v7a, "AND-X64" on x86_64.
 */
std::string detectHardware();


#endif // _ANDROID_DEFAULTS_H__
