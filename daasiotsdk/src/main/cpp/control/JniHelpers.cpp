/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: JniHelpers.cpp
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

#include "../include/daasiot_jni.hpp"

#include <algorithm>
#include <cstring>
#include <string>

// Pure JNI helpers (toStdString, enumFromValue, enumValue) live in java_bridge.hpp/JavaBridge.cpp
// so they can stay free of the libdaas headers. The factories here build Java objects from native
// libdaas structs and therefore stay coupled to daas.hpp/daas_types.hpp.

jobject makeDaasError(JNIEnv* env, daas_error_t err) {
    return enumFromValue(env, "sebyone/daasiot_android/enums/DaasError", static_cast<jint>(err));
}

// Builds a sebyone.daasiot_android.entity.NodeInfo from a native node_info_t.
jobject makeNodeInfo(JNIEnv* env, const node_info_t& info) {
    jclass cls = env->FindClass("sebyone/daasiot_android/entity/NodeInfo");
    jmethodID ctor = env->GetMethodID(cls, "<init>",
            "(JJJI[BLsebyone/daasiot_android/enums/AcceptRequestPolicy;JJ"
            "Lsebyone/daasiot_android/enums/DDOPolicy;Lsebyone/daasiot_android/enums/DiscoveryState;JZ)V");

    jbyteArray skey = env->NewByteArray(sizeof(info.skey));
    env->SetByteArrayRegion(skey, 0, sizeof(info.skey), reinterpret_cast<const jbyte*>(info.skey));

    jobject acceptPolicy = enumFromValue(env, "sebyone/daasiot_android/enums/AcceptRequestPolicy", static_cast<jint>(info.accept_request_policy));
    jobject ddoPolicy = enumFromValue(env, "sebyone/daasiot_android/enums/DDOPolicy", static_cast<jint>(info.ddo_policy));
    jobject discoveryState = enumFromValue(env, "sebyone/daasiot_android/enums/DiscoveryState", static_cast<jint>(info.discovery_state));

    jobject result = env->NewObject(
            cls, ctor,
            static_cast<jlong>(info.power_on_time),
            static_cast<jlong>(info.linked),
            static_cast<jlong>(info.lock),
            static_cast<jint>(info.sklen),
            skey,
            acceptPolicy,
            static_cast<jlong>(info.sid),
            static_cast<jlong>(info.din),
            ddoPolicy,
            discoveryState,
            static_cast<jlong>(info.oCap_net),
            static_cast<jboolean>(info.net_in_sync)
    );

    env->DeleteLocalRef(skey);
    env->DeleteLocalRef(acceptPolicy);
    env->DeleteLocalRef(ddoPolicy);
    env->DeleteLocalRef(discoveryState);
    return result;
}

// Converts a fixed, not-necessarily-null-terminated char buffer (e.g. node_map_entry_t::uri) to a
// jstring without reading past maxLen bytes.
jstring makeBoundedString(JNIEnv* env, const char* buf, size_t maxLen) {
    size_t len = strnlen(buf, maxLen);
    return env->NewStringUTF(std::string(buf, len).c_str());
}

// Builds a sebyone.daasiot_android.entity.NodeMapEntry from a native node_map_entry_t.
jobject makeNodeMapEntry(JNIEnv* env, const node_map_entry_t& entry) {
    jclass cls = env->FindClass("sebyone/daasiot_android/entity/NodeMapEntry");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(JJLsebyone/daasiot_android/enums/LinkType;Ljava/lang/String;Z)V");

    jobject link = enumFromValue(env, "sebyone/daasiot_android/enums/LinkType", static_cast<jint>(entry.link));
    jstring uri = makeBoundedString(env, entry.uri, sizeof(entry.uri));

    jobject result = env->NewObject(
            cls, ctor,
            static_cast<jlong>(entry.din),
            static_cast<jlong>(entry.channel_owner),
            link,
            uri,
            static_cast<jboolean>(entry.direct_channel)
    );

    env->DeleteLocalRef(link);
    env->DeleteLocalRef(uri);
    return result;
}

// Unboxes a sebyone.daasiot_android.entity.FeatureRequest into a native feature_rq_t. params/payload
// shorter than the fixed native buffers are zero-padded (feature_rq_t rq{} zero-initializes both
// arrays up front); FeatureRequest itself already rejects oversized arrays before this is reached.
feature_rq_t toFeatureRequest(JNIEnv* env, jobject request) {
    feature_rq_t rq{};
    jclass cls = env->GetObjectClass(request);

    jmethodID getFeature = env->GetMethodID(cls, "getFeature", "()Lsebyone/daasiot_android/enums/Feature;");
    jmethodID getAction = env->GetMethodID(cls, "getAction", "()Lsebyone/daasiot_android/enums/FeatureAction;");
    jmethodID getParams = env->GetMethodID(cls, "getParams", "()[B");
    jmethodID getPayload = env->GetMethodID(cls, "getPayload", "()[B");

    jobject featureObj = env->CallObjectMethod(request, getFeature);
    jobject actionObj = env->CallObjectMethod(request, getAction);
    auto paramsArr = static_cast<jbyteArray>(env->CallObjectMethod(request, getParams));
    auto payloadArr = static_cast<jbyteArray>(env->CallObjectMethod(request, getPayload));

    rq.feature = static_cast<feature_t>(enumValue(env, featureObj));
    rq.request = static_cast<feature_action_e>(enumValue(env, actionObj));

    jsize paramsLen = std::min<jsize>(env->GetArrayLength(paramsArr), sizeof(rq.params));
    env->GetByteArrayRegion(paramsArr, 0, paramsLen, reinterpret_cast<jbyte*>(rq.params));

    jsize payloadLen = std::min<jsize>(env->GetArrayLength(payloadArr), sizeof(rq.payload));
    env->GetByteArrayRegion(payloadArr, 0, payloadLen, reinterpret_cast<jbyte*>(rq.payload));

    env->DeleteLocalRef(featureObj);
    env->DeleteLocalRef(actionObj);
    env->DeleteLocalRef(paramsArr);
    env->DeleteLocalRef(payloadArr);

    return rq;
}
