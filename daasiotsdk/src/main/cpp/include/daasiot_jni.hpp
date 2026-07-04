/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: daasiot_jni.hpp
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
 * s.meduri@sebyone.it - refactoring
 *
 */

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
