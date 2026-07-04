/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DaasAPINative.cpp
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

#include <android/log.h>
#include <vector>

#define LOG_TAG "DaaS-Native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/* ---------------- Info / strings ---------------- */

extern "C" {

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetVersion(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) {
        LOGD("[DaaS] nativeGetVersion: invalid handle");
        return env->NewStringUTF("");
    }

    const char* ver = api->getVersion();
    LOGD("[DaaS] nativeGetVersion -> %s", ver ? ver : "(null)");

    return env->NewStringUTF(ver ? ver : "");
}

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetBuildInfo(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) {
        LOGD("[DaaS] nativeGetBuildInfo: invalid handle");
        return env->NewStringUTF("");
    }

    const char* info = api->getBuildInfo();
    LOGD("[DaaS] nativeGetBuildInfo -> %s", info ? info : "(null)");

    return env->NewStringUTF(info ? info : "");
}

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeListAvailableDrivers(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) {
        LOGD("[DaaS] nativeListAvailableDrivers: invalid handle");
        return env->NewStringUTF("");
    }

    const char* list = api->listAvailableDrivers();
    LOGD("[DaaS] nativeListAvailableDrivers -> %s", list ? list : "(null)");

    return env->NewStringUTF(list ? list : "");
}

} // extern "C"


extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeCreate(
        JNIEnv* env, jclass, jobject eventHandler, jstring lhver) {

    LOGD("[DaaS] Creating DaasAPI instance");
    auto* events = new DaasEvents(env, eventHandler);

    DaasAPI* api;
    if (lhver) {
        std::string sver = toStdString(env, lhver);
        api = new DaasAPI(events, sver.c_str());
    } else {
        api = new DaasAPI(events);
    }
    LOGD("[DaaS] Library Version: %s", api->getVersion());

    auto* handle = new ApiHandle{api, events};
    return reinterpret_cast<jlong>(handle);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeInit(
        JNIEnv* env, jclass, jlong handle, din_t sid, din_t din) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    LOGD("[DaaS] Initializing with SID=%lu DIN=%lu", sid, din);
    auto err = api->doInit(sid, din);
    LOGD("[DaaS] doInit() -> %d", err);
    return makeDaasError(env, err);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeReset(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    LOGD("[DaaS] Resetting Node... Node will be disconnected from the network");
    auto err = api->doReset();
    LOGD("[DaaS] doReset() -> %d", err);
    return makeDaasError(env, err);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeEnableDriver(
        JNIEnv* env, jclass, jlong handle, jobject driver, jstring uri) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    const char* c_uri = env->GetStringUTFChars(uri, nullptr);
    jint driverId = enumValue(env, driver);
    LOGD("[DaaS] Enabling driver %d with URI %s", driverId, c_uri);

    auto err = api->enableDriver(static_cast<link_t>(driverId), c_uri);

    LOGD("[DaaS] enableDriver() -> %d", err);

    env->ReleaseStringUTFChars(uri, c_uri);
    return makeDaasError(env, err);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDoEnd(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->doEnd());
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDoStatisticsReset(
        JNIEnv*, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return JNI_FALSE;

    return static_cast<jboolean>(api->doStatisticsReset());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetSystemStatistics(
        JNIEnv*, jclass, jlong handle, jint code) {

    DaasAPI* api = toApi(handle);
    if (!api) return 0;

    return static_cast<jlong>(api->getSystemStatistics(static_cast<syscode_t>(code)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeMap(
        JNIEnv* env, jclass, jlong handle, jlong din, jobject link, jstring uri, jstring skey) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint linkId = enumValue(env, link);
    daas_error_t err;
    if (!uri) {
        LOGD("[DaaS] map din=%lu (no uri)", (unsigned long)din);
        err = api->map((din_t)din);
    } else if (!skey) {
        std::string suri = toStdString(env, uri);
        LOGD("[DaaS] map din=%lu link=%d uri=%s", (unsigned long)din, linkId, suri.c_str());
        err = api->map((din_t)din, (link_t)linkId, suri.c_str());
    } else {
        std::string suri = toStdString(env, uri);
        std::string sskey = toStdString(env, skey);
        LOGD("[DaaS] map din=%lu link=%d uri=%s skey=***", (unsigned long)din, linkId, suri.c_str());
        err = api->map((din_t)din, (link_t)linkId, suri.c_str(), sskey.c_str());
    }

    LOGD("[DaaS] map() -> %d", err);
    return makeDaasError(env, err);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeRemoveNode(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->remove((din_t)din));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDiscoveryByLink(
        JNIEnv* env, jclass, jlong handle, jobject link) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint linkId = enumValue(env, link);
    return makeDaasError(env, api->discovery(static_cast<link_t>(linkId)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDiscoveryBySid(
        JNIEnv* env, jclass, jlong handle, jlong sid) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->discovery(static_cast<din_t>(sid)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeJoin(
        JNIEnv* env, jclass, jlong handle, jlong sid, jobject link, jlong timeoutMillis) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint linkId = enumValue(env, link);
    return makeDaasError(env, api->join(static_cast<din_t>(sid), static_cast<link_t>(linkId), static_cast<uint32_t>(timeoutMillis)));
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSetDiscoveryState(
        JNIEnv* env, jclass, jlong handle, jobject mode) {

    DaasAPI* api = toApi(handle);
    if (!api) return;

    jint modeId = enumValue(env, mode);
    api->setDiscoveryState(static_cast<discovery_state_t>(modeId));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetNodeList(
        JNIEnv* env, jclass, jlong handle) {

    jclass cls = env->FindClass("sebyone/daasiot_android/entity/NodeMapEntry");

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewObjectArray(0, cls, nullptr);

    node_map_list_t entries = api->getNodeList();
    jobjectArray result = env->NewObjectArray((jsize)entries.size(), cls, nullptr);

    for (uint32_t i = 0; i < entries.size(); ++i) {
        jobject obj = makeNodeMapEntry(env, entries.at(i));
        env->SetObjectArrayElement(result, (jsize)i, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetAllNodes(
        JNIEnv* env, jclass, jlong handle, jlong sid) {

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewLongArray(0);

    Vector<din_t> nodes = api->getAllNodes(static_cast<din_t>(sid));
    std::vector<jlong> values(nodes.size());
    for (uint32_t i = 0; i < nodes.size(); ++i) {
        values[i] = static_cast<jlong>(nodes.at(i));
    }

    jlongArray result = env->NewLongArray((jsize)values.size());
    env->SetLongArrayRegion(result, 0, (jsize)values.size(), values.data());
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeLocate(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->locate((din_t)din));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeLocateFeature(
        JNIEnv* env, jclass, jlong handle, jobject feature, jint timeoutMillis, jint ttl) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint featureId = enumValue(env, feature);
    return makeDaasError(env, api->locate(static_cast<feature_t>(featureId), timeoutMillis, ttl));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSendStatus(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->sendStatus((din_t)din));
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetSyncedTimestamp(
        JNIEnv*, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return 0;

    return static_cast<jlong>(api->getSyncedTimestamp());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetStatusCopy(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return nullptr;

    node_info_t info = api->getStatus();
    return makeNodeInfo(env, info);
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSetAcceptRequestsLevel(
        JNIEnv* env, jclass, jlong handle, jobject policyLevel) {

    DaasAPI* api = toApi(handle);
    if (!api) return;

    jint policyId = enumValue(env, policyLevel);
    api->setAcceptRequestsLevel(static_cast<accept_request_policy_t>(policyId));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeStatusCopy(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return nullptr;

    return makeNodeInfo(env, api->status((din_t)din));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFetchCopy(
        JNIEnv* env, jclass, jlong handle, jlong din, jint opts) {

    DaasAPI* api = toApi(handle);
    if (!api) return nullptr;

    return makeNodeInfo(env, api->fetch((din_t)din, static_cast<uint16_t>(opts)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSyncNode(
        JNIEnv* env, jclass, jlong handle, jlong din, jint timezone) {

    DaasAPI* api = toApi(handle);
    if (!api) return nullptr;

    return makeNodeInfo(env, api->syncNode((din_t)din, static_cast<unsigned>(timezone)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSyncNet(
        JNIEnv* env, jclass, jlong handle, jlong din, jint bubbleTime) {

    DaasAPI* api = toApi(handle);
    if (!api) return nullptr;

    return makeNodeInfo(env, api->syncNet((din_t)din, static_cast<unsigned>(bubbleTime)));
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeListNodesCopy(
        JNIEnv* env, jclass, jlong handle) {

    jclass cls = env->FindClass("sebyone/daasiot_android/entity/NodeNetworkInfo");

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewObjectArray(0, cls, nullptr);

    network_info_list_t nodes = api->listNodes();
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(JJ)V");
    jobjectArray result = env->NewObjectArray((jsize)nodes.size(), cls, nullptr);

    for (uint32_t i = 0; i < nodes.size(); ++i) {
        node_network_info_t item = nodes.at(i);
        jobject obj = env->NewObject(cls, ctor, (jlong)item.sid, (jlong)item.din);
        env->SetObjectArrayElement(result, (jsize)i, obj);
        env->DeleteLocalRef(obj);
    }
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDoPerform(
        JNIEnv* env, jclass, jlong handle, jobject mode) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint modeId = enumValue(env, mode);
    return makeDaasError(env, api->doPerform(static_cast<performs_mode_t>(modeId)));
}

/* ---------------- Exchange (real-time session) ---------------- */

extern "C"
JNIEXPORT jboolean JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeUse(
        JNIEnv*, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return JNI_FALSE;

    return static_cast<jboolean>(api->use((din_t)din));
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeEnd(
        JNIEnv*, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return JNI_FALSE;

    return static_cast<jboolean>(api->end((din_t)din));
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSend(
        JNIEnv* env, jclass, jlong handle, jlong din, jbyteArray data) {

    DaasAPI* api = toApi(handle);
    if (!api || !data) return 0;

    jsize size = env->GetArrayLength(data);
    jbyte* bytes = env->GetByteArrayElements(data, nullptr);

    unsigned sent = api->send((din_t)din, reinterpret_cast<unsigned char*>(bytes), (unsigned)size);

    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
    return (jint)sent;
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeReceived(
        JNIEnv*, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return 0;

    return (jint)api->received((din_t)din);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeReceive(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewByteArray(0);

    uint8_t* inbound = nullptr;
    uint32_t size = api->receive((din_t)din, inbound);

    jbyteArray result = env->NewByteArray((jsize)size);
    if (size > 0 && inbound) {
        env->SetByteArrayRegion(result, 0, (jsize)size, reinterpret_cast<jbyte*>(inbound));
    }
    return result;
}

/* ---------------- Transfer ---------------- */

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativePull(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    jclass resultCls = env->FindClass("sebyone/daasiot_android/entity/PullResult");
    jmethodID resultCtor = env->GetMethodID(resultCls, "<init>", "(Lsebyone/daasiot_android/enums/DaasError;Lsebyone/daasiot_android/entity/DDO;)V");

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewObject(resultCls, resultCtor, makeDaasError(env, ERROR_UNKNOWN), nullptr);

    DDO* inbound = nullptr;
    daas_error_t err = api->pull((din_t)din, &inbound);

    jobject ddoObj = nullptr;
    if (inbound) {
        jclass ddoCls = env->FindClass("sebyone/daasiot_android/entity/DDO");
        jmethodID ddoCtor = env->GetMethodID(ddoCls, "<init>", "(JZ)V");
        ddoObj = env->NewObject(ddoCls, ddoCtor, reinterpret_cast<jlong>(inbound), (jboolean)JNI_TRUE);
    }

    jobject errorObj = makeDaasError(env, err);
    jobject result = env->NewObject(resultCls, resultCtor, errorObj, ddoObj);
    env->DeleteLocalRef(errorObj);
    if (ddoObj) env->DeleteLocalRef(ddoObj);
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativePush(
        JNIEnv* env, jclass, jlong handle, jlong din, jlong ddoHandle) {

    DaasAPI* api = toApi(handle);
    if (!api || !ddoHandle) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->push((din_t)din, reinterpret_cast<DDO*>(ddoHandle)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeAvailablesPull(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    jclass cls = env->FindClass("sebyone/daasiot_android/entity/AvailablePullResult");
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Lsebyone/daasiot_android/enums/DaasError;J)V");

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewObject(cls, ctor, makeDaasError(env, ERROR_UNKNOWN), (jlong)0);

    uint32_t count = 0;
    daas_error_t err = api->availablesPull((din_t)din, count);
    return env->NewObject(cls, ctor, makeDaasError(env, err), (jlong)count);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeAddTypeset(
        JNIEnv* env, jclass, jlong handle, jint typesetCode) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    // addTypeset() now takes a native typeset_fun callback (void(*)(din_t)) instead of a
    // size; there is no way to hand Kotlin code a native function pointer, so this only
    // registers the typeset code. Received DDOs for it still surface via ddoReceived().
    return makeDaasError(env, api->addTypeset(static_cast<uint16_t>(typesetCode), nullptr));
}

/* ---------------- Test ---------------- */

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFrisbee(
        JNIEnv* env, jclass, jlong handle, jlong din) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->frisbee((din_t)din));
}

/* ---------------- Network & options ---------------- */

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSetDDOPolicy(
        JNIEnv* env, jclass, jlong handle, jobject policy) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint policyId = enumValue(env, policy);
    return makeDaasError(env, api->setDDOPolicy(static_cast<ddo_policy_t>(policyId)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeCreateNetwork(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->createNetwork());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeUnbindNetwork(
        JNIEnv* env, jclass, jlong handle) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    return makeDaasError(env, api->unbindNetwork());
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSetOptions(
        JNIEnv* env, jclass, jlong handle, jobject option, jlong value) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint optionId = enumValue(env, option);
    return makeDaasError(env, api->setOptions(static_cast<option_t>(optionId), static_cast<uint32_t>(value)));
}

extern "C"
JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeErrorToString(
        JNIEnv* env, jclass, jlong handle, jobject error) {

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewStringUTF("");

    jint errorId = enumValue(env, error);
    const char* str = api->errorToString(static_cast<daas_error_t>(errorId));
    return env->NewStringUTF(str ? str : "");
}

/* ---------------- Feature negotiation ---------------- */

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeAddNodeFeatures(
        JNIEnv* env, jclass, jlong handle, jobject feature) {

    DaasAPI* api = toApi(handle);
    if (!api) return makeDaasError(env, ERROR_UNKNOWN);

    jint featureId = enumValue(env, feature);
    return makeDaasError(env, api->addNodeFeatures(static_cast<feature_t>(featureId)));
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetNodeFeatures(
        JNIEnv* env, jclass, jlong handle) {

    jclass cls = env->FindClass("sebyone/daasiot_android/enums/Feature");

    DaasAPI* api = toApi(handle);
    if (!api) return env->NewObjectArray(0, cls, nullptr);

    features_list features = api->getNodeFeatures();
    jobjectArray result = env->NewObjectArray((jsize)features.size(), cls, nullptr);

    for (uint32_t i = 0; i < features.size(); ++i) {
        jobject obj = enumFromValue(env, "sebyone/daasiot_android/enums/Feature", static_cast<jint>(features.at(i)));
        env->SetObjectArrayElement(result, (jsize)i, obj);
        if (obj) env->DeleteLocalRef(obj);
    }
    return result;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeNodeHasFeature(
        JNIEnv* env, jclass, jlong handle, jlong din, jobject feature) {

    DaasAPI* api = toApi(handle);
    if (!api) return JNI_FALSE;

    jint featureId = enumValue(env, feature);
    return static_cast<jboolean>(api->nodeHasFeature(static_cast<din_t>(din), static_cast<feature_t>(featureId)));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeRequestFeature(
        JNIEnv* env, jclass, jlong handle, jlong din, jobject request, jlong timeoutMillis) {

    (void)din;
    (void)request;
    (void)timeoutMillis;

    // DaasAPI::requestFeature() is declared in daas.hpp but is not an exported symbol in the
    // currently linked libdaas.a for this target (undefined reference at link time), matching
    // daasiot-java's own JNI layer, which stubs this exact method out for the same reason.
    if (!toApi(handle)) return makeDaasError(env, ERROR_UNKNOWN);
    return makeDaasError(env, ERROR_NOT_IMPLEMENTED);
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDestroy(
        JNIEnv*, jclass, jlong handle) {

    if (!handle) return;
    ApiHandle* h = toHandle(handle);

    LOGD("[DaaS] Destroying DaasAPI");
    h->api->doEnd();
    delete h->api;
    delete h->events;
    delete h;
}
