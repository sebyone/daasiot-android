#ifndef DAASIOT_JNI_HPP
#define DAASIOT_JNI_HPP

#include <jni.h>

#include "../../include/daas.hpp"
#include "../../include/daas_types.hpp"
#include "java_bridge.hpp"

extern JavaVM* g_vm;

// Forwards every DaasAPI event to the sebyone.daasiot_android.IDaasApiEvent instance the app
// passed to DaasWrapper.create(), if any. No implicit side effects (e.g. no auto-pull on
// ddoReceived) - the app decides what to do with each event, matching the native interface.
// Defined in events/DaasEvents.cpp.
class DaasEvents final : public IDaasApiEvent {
public:
    DaasEvents(JNIEnv* env, jobject handler);
    ~DaasEvents() override;

    void dinAccepted(din_t din) override;
    void ddoReceived(int payload_size, typeset_t typeset, din_t origin) override;
    void frisbeeReceived(din_t din) override;
    void nodeStateReceived(din_t din) override;
    void atsSyncCompleted(din_t din) override;
    void frisbeeDperfCompleted(din_t din, uint32_t packets_sent, uint32_t block_size) override;
    void networkDiscovered(din_t din, din_t sid, link_t link) override;
    void nodeConnectedToNetwork(din_t sid, din_t din) override;
    void streamInfoReceived(din_t din, stream_type pkt_type, uint32_t stream_id) override;

private:
    jobject handler_;
    jclass handlerClass_;

    void callLong(const char* name, din_t din);
};

// Per-instance handle. Mirrors daasiot-java's ApiHandle{DaasAPI*, DaasEvents*}: nativeCreate()
// allocates one of these and hands its address back to Kotlin as a jlong; every other native call
// receives that same jlong and resolves it back to this instead of touching shared global state.
struct ApiHandle {
    DaasAPI* api;
    DaasEvents* events;
};

inline ApiHandle* toHandle(jlong handle) {
    return reinterpret_cast<ApiHandle*>(handle);
}

inline DaasAPI* toApi(jlong handle) {
    return handle ? toHandle(handle)->api : nullptr;
}

// Factories below depend on libdaas structs/enums, unlike java_bridge.hpp's pure-JNI helpers.
// Defined in control/JniHelpers.cpp.

jobject makeDaasError(JNIEnv* env, daas_error_t err);
jobject makeNodeInfo(JNIEnv* env, const node_info_t& info);
jstring makeBoundedString(JNIEnv* env, const char* buf, size_t maxLen);
jobject makeNodeMapEntry(JNIEnv* env, const node_map_entry_t& entry);
feature_rq_t toFeatureRequest(JNIEnv* env, jobject request);

#endif // DAASIOT_JNI_HPP
