/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: android_daas_jni.cpp — android_daas (daasiot-android)
 *
 * JNI binding of daas::sdk::Node (android_daas.hpp) for the Java class it.sebyone.daas.DaasNode: one node per
 * process (libdaas has one), its file_depot in the files folder of the app, the events dispatched to the Java side
 * from Node::service() (the service thread of DaasNode), the DDOs pulled into it.sebyone.daas.Ddo, the status into
 * it.sebyone.daas.NodeStatus, the logs of libdaas (stdout/stderr) into logcat.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation (from the JNI bridge of the thermobooster app)
 */

#include "android_daas.hpp"

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#define LOG_TAG "android_daas"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using daas::sdk::Event;
using daas::sdk::EventType;
using daas::sdk::Node;
using daas::sdk::NodeConfig;

namespace {

JavaVM*   g_vm = nullptr;
jclass    g_nodeClass = nullptr;     // it/sebyone/daas/DaasNode
jclass    g_ddoClass = nullptr;      // it/sebyone/daas/Ddo
jclass    g_statusClass = nullptr;   // it/sebyone/daas/NodeStatus
jmethodID g_dispatch = nullptr;      // static void dispatch(int, long, long, int, int, int)
jmethodID g_ddoInit = nullptr;       // Ddo(long, long, int, long, byte[])
jmethodID g_statusInit = nullptr;    // NodeStatus(long, int, int, String, int, long, long, int, int, long, boolean)

std::mutex g_mutex;                  // guards g_node / g_depot against create/destroy
std::unique_ptr<daas::persistence::file_depot> g_depot;
std::unique_ptr<Node> g_node;

// Threads of libdaas that reach Java (events not queued) are attached once and detached when they exit.
pthread_key_t g_envKey;
void detachThread(void*) { if (g_vm) g_vm->DetachCurrentThread(); }

JNIEnv* env() {
    JNIEnv* e = nullptr;
    if (g_vm->GetEnv(reinterpret_cast<void**>(&e), JNI_VERSION_1_6) == JNI_OK) return e;
    if (g_vm->AttachCurrentThread(&e, nullptr) != JNI_OK) return nullptr;
    pthread_setspecific(g_envKey, e);
    return e;
}

struct JStr {
    JNIEnv* e; jstring s; const char* c;
    JStr(JNIEnv* e_, jstring s_) : e(e_), s(s_), c(s_ ? e_->GetStringUTFChars(s_, nullptr) : nullptr) {}
    ~JStr() { if (c) e->ReleaseStringUTFChars(s, c); }
    std::string str() const { return c ? std::string(c) : std::string(); }
};

/** The event to DaasNode.dispatch(type, din, sid, a, b, c); never lets a Java exception reach libdaas. */
void toJava(const Event& event) {
    JNIEnv* e = env();
    if (!e || !g_dispatch) return;
    int a = 0, b = 0, c = 0;
    switch (event.type) {
        case EventType::DdoReceived:           a = event.typeset; b = event.payloadSize; break;
        case EventType::NetworkDiscovered:     a = static_cast<int>(event.link); break;
        case EventType::FrisbeeDperfCompleted:
        case EventType::StreamInfoReceived:    a = static_cast<int>(event.value1); b = static_cast<int>(event.value2); break;
        default: break;
    }
    e->CallStaticVoidMethod(g_nodeClass, g_dispatch, static_cast<jint>(event.type), static_cast<jlong>(event.din),
                            static_cast<jlong>(event.sid), a, b, c);
    if (e->ExceptionCheck()) { e->ExceptionDescribe(); e->ExceptionClear(); }
}

jlongArray toLongArray(JNIEnv* e, const std::vector<jlong>& v) {
    jlongArray arr = e->NewLongArray(static_cast<jsize>(v.size()));
    if (arr && !v.empty()) e->SetLongArrayRegion(arr, 0, static_cast<jsize>(v.size()), v.data());
    return arr;
}

std::vector<jint> ints(JNIEnv* e, jintArray a) {
    std::vector<jint> v(a ? e->GetArrayLength(a) : 0);
    if (!v.empty()) e->GetIntArrayRegion(a, 0, static_cast<jsize>(v.size()), v.data());
    return v;
}

std::vector<jlong> longs(JNIEnv* e, jlongArray a) {
    std::vector<jlong> v(a ? e->GetArrayLength(a) : 0);
    if (!v.empty()) e->GetLongArrayRegion(a, 0, static_cast<jsize>(v.size()), v.data());
    return v;
}

std::string stringAt(JNIEnv* e, jobjectArray a, jsize i) {
    jstring s = static_cast<jstring>(e->GetObjectArrayElement(a, i));
    std::string out = JStr(e, s).str();
    if (s) e->DeleteLocalRef(s);
    return out;
}

/** stdout/stderr of libdaas -> logcat. */
void* pipeToLogcat(void* arg) {
    const int fd = static_cast<int>(reinterpret_cast<intptr_t>(arg));
    char buf[512];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        __android_log_write(ANDROID_LOG_DEBUG, "libdaas", buf);
    }
    return nullptr;
}

jclass globalClass(JNIEnv* e, const char* name) {
    jclass local = e->FindClass(name);
    if (!local) return nullptr;
    jclass global = static_cast<jclass>(e->NewGlobalRef(local));
    e->DeleteLocalRef(local);
    return global;
}

} // namespace

#define NODE_OR(ret) std::lock_guard<std::mutex> lock_(g_mutex); if (!g_node) return ret; Node& node = *g_node;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    pthread_key_create(&g_envKey, detachThread);
    JNIEnv* e = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&e), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;
    g_nodeClass = globalClass(e, "it/sebyone/daas/DaasNode");
    g_ddoClass = globalClass(e, "it/sebyone/daas/Ddo");
    g_statusClass = globalClass(e, "it/sebyone/daas/NodeStatus");
    if (!g_nodeClass || !g_ddoClass || !g_statusClass) return JNI_ERR;
    g_dispatch = e->GetStaticMethodID(g_nodeClass, "dispatch", "(IJJIII)V");
    g_ddoInit = e->GetMethodID(g_ddoClass, "<init>", "(JJIJ[B)V");
    g_statusInit = e->GetMethodID(g_statusClass, "<init>", "(JIILjava/lang/String;IJJIIJZ)V");
    if (e->ExceptionCheck()) { e->ExceptionDescribe(); e->ExceptionClear(); return JNI_ERR; }
    return JNI_VERSION_1_6;
}

/* ---- life cycle ---------------------------------------------------------------------------------------- */

extern "C" JNIEXPORT void JNICALL
Java_it_sebyone_daas_DaasNode_nativeRedirectLogs(JNIEnv*, jclass) {
    static std::atomic_bool done{false};
    if (done.exchange(true)) return;
    int fds[2];
    if (pipe(fds) != 0) return;
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    dup2(fds[1], STDOUT_FILENO);
    dup2(fds[1], STDERR_FILENO);
    close(fds[1]);
    pthread_t t;
    if (pthread_create(&t, nullptr, pipeToLogcat, reinterpret_cast<void*>(static_cast<intptr_t>(fds[0]))) == 0)
        pthread_detach(t);
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeCreate(JNIEnv* e, jclass, jstring stateDir, jlong sid, jlong din,
        jintArray driverLinks, jobjectArray driverUris, jintArray driverPorts,
        jlongArray mapDins, jintArray mapLinks, jobjectArray mapUris, jobjectArray mapKeys,
        jintArray optionKeys, jintArray optionValues, jintArray policies, jlong joinSid, jint joinLink,
        jint joinTimeout, jint atsMaxError, jstring hardware) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_node) return ERROR_CORE_ALREADY_INITIALIZED;

    NodeConfig config;
    config.sid = static_cast<din_t>(sid);
    config.din = static_cast<din_t>(din);
    const auto dl = ints(e, driverLinks), dp = ints(e, driverPorts);
    for (size_t i = 0; i < dl.size(); i++)
        config.drivers.push_back({static_cast<link_t>(dl[i]), stringAt(e, driverUris, static_cast<jsize>(i)),
                                  static_cast<uint16_t>(i < dp.size() ? dp[i] : 0)});
    const auto md = longs(e, mapDins);
    const auto ml = ints(e, mapLinks);
    for (size_t i = 0; i < md.size(); i++)
        config.mappings.push_back({static_cast<din_t>(md[i]), static_cast<link_t>(i < ml.size() ? ml[i] : 0),
                                   stringAt(e, mapUris, static_cast<jsize>(i)), stringAt(e, mapKeys, static_cast<jsize>(i))});
    const auto ok = ints(e, optionKeys), ov = ints(e, optionValues);
    for (size_t i = 0; i < ok.size() && i < ov.size(); i++)
        config.options.push_back({static_cast<option_t>(ok[i]), static_cast<uint32_t>(ov[i])});
    // policies: acceptRequests, discoveryState, ddoPolicy; -1 = not set
    const auto p = ints(e, policies);
    if (p.size() > 0 && p[0] >= 0) { config.hasAcceptRequests = true; config.acceptRequests = static_cast<accept_request_policy_t>(p[0]); }
    if (p.size() > 1 && p[1] >= 0) { config.hasDiscoveryState = true; config.discoveryState = static_cast<discovery_state_t>(p[1]); }
    if (p.size() > 2 && p[2] >= 0) { config.hasDdoPolicy = true; config.ddoPolicy = static_cast<ddo_policy_t>(p[2]); }
    if (joinTimeout > 0) {
        config.hasJoin = true;
        config.join = {static_cast<din_t>(joinSid), static_cast<link_t>(joinLink), static_cast<uint32_t>(joinTimeout)};
    }
    if (atsMaxError >= 0) { config.hasAtsMaxError = true; config.atsMaxError = atsMaxError; }
    const std::string hw = JStr(e, hardware).str();

    const std::string dir = JStr(e, stateDir).str();
    if (!daas::persistence::makeDirectories(dir)) { LOGE("cannot create the state folder %s", dir.c_str()); return ERROR_CANNOT_INITIALIZE; }
    g_depot = std::make_unique<daas::persistence::file_depot>(dir);
    g_node = std::make_unique<Node>(std::move(config), g_depot.get(), hw.empty() ? nullptr : hw.c_str());

    static const EventType kTypes[] = {EventType::DinAccepted, EventType::DdoReceived, EventType::FrisbeeReceived,
        EventType::NodeStateReceived, EventType::AtsSyncCompleted, EventType::FrisbeeDperfCompleted,
        EventType::NetworkDiscovered, EventType::NodeConnectedToNetwork, EventType::StreamInfoReceived,
        EventType::AttackPanic};
    for (EventType type : kTypes) g_node->events().on(type, toJava);
    return g_node->initializationResult();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_it_sebyone_daas_DaasNode_nativeRestored(JNIEnv*, jclass) { NODE_OR(JNI_FALSE) return node.restored(); }

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeStart(JNIEnv*, jclass) { NODE_OR(ERROR_CORE_STOPPED) return node.start(); }

/** Called by the service thread of DaasNode: events, pending saves. Not under g_mutex while it waits. */
extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeService(JNIEnv*, jclass, jint waitMs) {
    Node* node;
    { std::lock_guard<std::mutex> lock(g_mutex); node = g_node.get(); }
    if (!node) return -1;
    return static_cast<jint>(node->service(std::chrono::milliseconds(waitMs)));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeStop(JNIEnv*, jclass) { NODE_OR(ERROR_NONE) return node.stop(); }

extern "C" JNIEXPORT void JNICALL
Java_it_sebyone_daas_DaasNode_nativeDestroy(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_node.reset();
    g_depot.reset();
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeReset(JNIEnv*, jclass) { NODE_OR(ERROR_CORE_STOPPED) return node.reset(); }

extern "C" JNIEXPORT jboolean JNICALL
Java_it_sebyone_daas_DaasNode_nativeSave(JNIEnv*, jclass) { NODE_OR(JNI_FALSE) return node.save(); }

extern "C" JNIEXPORT jboolean JNICALL
Java_it_sebyone_daas_DaasNode_nativePanicked(JNIEnv*, jclass) { NODE_OR(JNI_FALSE) return node.panicked(); }

/* ---- drivers, map, network ----------------------------------------------------------------------------- */

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeEnableDriver(JNIEnv* e, jclass, jint link, jstring uri) {
    NODE_OR(ERROR_CORE_STOPPED) return node.enableDriver(static_cast<link_t>(link), JStr(e, uri).str());
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeDisableDriver(JNIEnv*, jclass, jint link) {
    NODE_OR(ERROR_CORE_STOPPED) return node.disableDriver(static_cast<link_t>(link));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeMap(JNIEnv* e, jclass, jlong din, jint link, jstring uri, jstring key) {
    NODE_OR(ERROR_CORE_STOPPED) return node.map(static_cast<din_t>(din), static_cast<link_t>(link), JStr(e, uri).str(), JStr(e, key).str());
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeUnmap(JNIEnv*, jclass, jlong din) {
    NODE_OR(ERROR_CORE_STOPPED) return node.unmap(static_cast<din_t>(din));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeDiscoveryLink(JNIEnv*, jclass, jint link) {
    NODE_OR(ERROR_CORE_STOPPED) return node.discovery(static_cast<link_t>(link));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeDiscoverySid(JNIEnv*, jclass, jlong sid) {
    NODE_OR(ERROR_CORE_STOPPED) return node.api().discovery(static_cast<din_t>(sid));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeJoin(JNIEnv*, jclass, jlong sid, jint link, jint timeout) {
    NODE_OR(ERROR_CORE_STOPPED) return node.join(static_cast<din_t>(sid), static_cast<link_t>(link), static_cast<uint32_t>(timeout));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeCreateNetwork(JNIEnv*, jclass) { NODE_OR(ERROR_CORE_STOPPED) return node.createNetwork(); }

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeUnbindNetwork(JNIEnv*, jclass) { NODE_OR(ERROR_CORE_STOPPED) return node.unbindNetwork(); }

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeLocate(JNIEnv*, jclass, jlong din, jint timeout) {
    NODE_OR(ERROR_CORE_STOPPED) return node.locate(static_cast<din_t>(din), static_cast<uint32_t>(timeout > 0 ? timeout : 0));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeState(JNIEnv*, jclass) { NODE_OR(-1) return node.state(); }

extern "C" JNIEXPORT jlong JNICALL
Java_it_sebyone_daas_DaasNode_nativeSid(JNIEnv*, jclass) { NODE_OR(0) return static_cast<jlong>(node.sid()); }

extern "C" JNIEXPORT jlong JNICALL
Java_it_sebyone_daas_DaasNode_nativeDin(JNIEnv*, jclass) { NODE_OR(0) return static_cast<jlong>(node.din()); }

extern "C" JNIEXPORT jlongArray JNICALL
Java_it_sebyone_daas_DaasNode_nativeKnownNodes(JNIEnv* e, jclass) {
    NODE_OR(e->NewLongArray(0))
    network_info_list_t nodes = node.knownNodes();
    std::vector<jlong> out;
    for (uint32_t i = 0; i < nodes.size(); ++i) out.push_back(static_cast<jlong>(nodes[i].din));
    return toLongArray(e, out);
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_it_sebyone_daas_DaasNode_nativeAllNodes(JNIEnv* e, jclass, jlong sid) {
    NODE_OR(e->NewLongArray(0))
    Vector<din_t> list = node.allNodes(static_cast<din_t>(sid));
    std::vector<jlong> out;
    for (uint32_t i = 0; i < list.size(); ++i) out.push_back(static_cast<jlong>(list[i]));
    return toLongArray(e, out);
}

/* ---- policies ------------------------------------------------------------------------------------------ */

extern "C" JNIEXPORT void JNICALL
Java_it_sebyone_daas_DaasNode_nativeSetAcceptRequests(JNIEnv*, jclass, jint policy) {
    NODE_OR() node.api().setAcceptRequestsLevel(static_cast<accept_request_policy_t>(policy));
}

extern "C" JNIEXPORT void JNICALL
Java_it_sebyone_daas_DaasNode_nativeSetDiscoveryState(JNIEnv*, jclass, jint state) {
    NODE_OR() node.api().setDiscoveryState(static_cast<discovery_state_t>(state));
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeSetDdoPolicy(JNIEnv*, jclass, jint policy) {
    NODE_OR(ERROR_CORE_STOPPED) return node.api().setDDOPolicy(static_cast<ddo_policy_t>(policy));
}

extern "C" JNIEXPORT void JNICALL
Java_it_sebyone_daas_DaasNode_nativeSetAtsMaxError(JNIEnv*, jclass, jint error) {
    NODE_OR() node.api().setATSMaxError(error);
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeSetOption(JNIEnv*, jclass, jint option, jint value) {
    NODE_OR(ERROR_CORE_STOPPED) return node.api().setOptions(static_cast<option_t>(option), static_cast<uint32_t>(value));
}

/* ---- data ---------------------------------------------------------------------------------------------- */

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativePush(JNIEnv* e, jclass, jlong din, jint typeset, jbyteArray payload) {
    NODE_OR(ERROR_CORE_STOPPED)
    const jsize size = payload ? e->GetArrayLength(payload) : 0;
    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0) e->GetByteArrayRegion(payload, 0, size, reinterpret_cast<jbyte*>(data.data()));
    return node.pushBytes(static_cast<din_t>(din), data.data(), data.size(), static_cast<typeset_t>(typeset));
}

extern "C" JNIEXPORT jobject JNICALL
Java_it_sebyone_daas_DaasNode_nativePull(JNIEnv* e, jclass, jlong din) {
    NODE_OR(nullptr)
    daas::sdk::InboxRecord record;
    if (node.pull(static_cast<din_t>(din), record) != ERROR_NONE) return nullptr;
    jbyteArray payload = e->NewByteArray(static_cast<jsize>(record.payload.size()));
    if (!payload) return nullptr;
    if (!record.payload.empty())
        e->SetByteArrayRegion(payload, 0, static_cast<jsize>(record.payload.size()), reinterpret_cast<const jbyte*>(record.payload.data()));
    jobject ddo = e->NewObject(g_ddoClass, g_ddoInit, static_cast<jlong>(record.din), static_cast<jlong>(record.origin),
                               static_cast<jint>(record.typeset), static_cast<jlong>(record.timestamp), payload);
    e->DeleteLocalRef(payload);
    return ddo;
}

extern "C" JNIEXPORT jint JNICALL
Java_it_sebyone_daas_DaasNode_nativeAvailable(JNIEnv*, jclass, jlong din) {
    NODE_OR(0) return static_cast<jint>(node.available(static_cast<din_t>(din)));
}

/* ---- information --------------------------------------------------------------------------------------- */

extern "C" JNIEXPORT jobject JNICALL
Java_it_sebyone_daas_DaasNode_nativeStatus(JNIEnv* e, jclass) {
    NODE_OR(nullptr)
    const node_info_t s = node.status();
    const std::string key(reinterpret_cast<const char*>(s.skey), std::min<size_t>(s.sklen, sizeof(s.skey)));
    jstring jkey = e->NewStringUTF(key.c_str());
    jobject out = e->NewObject(g_statusClass, g_statusInit, static_cast<jlong>(s.power_on_time), static_cast<jint>(s.linked),
                               static_cast<jint>(s.lock), jkey, static_cast<jint>(s.accept_request_policy),
                               static_cast<jlong>(s.sid), static_cast<jlong>(s.din), static_cast<jint>(s.ddo_policy),
                               static_cast<jint>(s.discovery_state), static_cast<jlong>(s.oCap_net),
                               static_cast<jboolean>(s.net_in_sync));
    e->DeleteLocalRef(jkey);
    return out;
}

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeVersion(JNIEnv* e, jclass) { NODE_OR(nullptr) return e->NewStringUTF(node.version()); }

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeVersionOf(JNIEnv* e, jclass, jint system) {
    NODE_OR(nullptr) return e->NewStringUTF(node.api().getVersion(static_cast<version_system_t>(system)));
}

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeBuildInfo(JNIEnv* e, jclass) { NODE_OR(nullptr) return e->NewStringUTF(node.buildInfo()); }

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeAvailableDrivers(JNIEnv* e, jclass) { NODE_OR(nullptr) return e->NewStringUTF(node.availableDrivers()); }

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeErrorString(JNIEnv* e, jclass, jint error) {
    NODE_OR(nullptr) return e->NewStringUTF(node.errorString(static_cast<daas_error_t>(error)));
}

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeHardware(JNIEnv* e, jclass) { NODE_OR(nullptr) return e->NewStringUTF(node.hardware().c_str()); }

extern "C" JNIEXPORT jstring JNICALL
Java_it_sebyone_daas_DaasNode_nativeWrapperVersion(JNIEnv* e, jclass) { return e->NewStringUTF(ANDROID_DAAS_VERSION); }
