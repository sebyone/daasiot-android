#include <jni.h>
#include <string>
#include <vector>
#include <memory>
#include "daas_wrapper.hpp"

// Convenience macro to reduce boilerplate for JNI function signatures
// Package: sebyone.daasiot_android
// Class: DaasWrapper
// JNI names below follow: Java_sebyone_daasiot_1android_DaasWrapper_methodName

static std::string jstringToStdString(JNIEnv* env, jstring s) {
    if (s == nullptr) return std::string();
    const char* c = env->GetStringUTFChars(s, nullptr);
    std::string str(c ? c : "");
    env->ReleaseStringUTFChars(s, c);
    return str;
}

static void throwJavaException(JNIEnv* env, const char* msg) {
    jclass exClass = env->FindClass("java/lang/RuntimeException");
    if (exClass != nullptr) {
        env->ThrowNew(exClass, msg);
    }
}

extern "C" {

// -------------------------
// lifecycle
// -------------------------
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeCreate(JNIEnv* env, jobject /*thiz*/) {
    try {
        auto* wrapper = new DaasWrapper();
        return reinterpret_cast<jlong>(wrapper);
    } catch (const std::exception& ex) {
        throwJavaException(env, ex.what());
        return 0;
    } catch (...) {
        throwJavaException(env, "nativeCreate: unknown error");
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDestroy(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) return;
delete wrapper;
}

// -------------------------
// Info / strings
// -------------------------
JNIEXPORT jstring JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetVersion(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetVersion: null wrapper");
return nullptr;
}
std::string v = wrapper->getVersion();
return env->NewStringUTF(v.c_str());
}

JNIEXPORT jstring JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetBuildInfo(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetBuildInfo: null wrapper");
return nullptr;
}
std::string s = wrapper->getBuildInfo();
return env->NewStringUTF(s.c_str());
}

JNIEXPORT jstring JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeListAvailableDrivers(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeListAvailableDrivers: null wrapper");
return nullptr;
}
std::string s = wrapper->listAvailableDrivers();
return env->NewStringUTF(s.c_str());
}

// -------------------------
// Core / lifecycle ops (return int error codes)
// -------------------------
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeDoInit(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint sid, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeDoInit: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->doInit(static_cast<int>(sid), static_cast<int>(din)));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeDoEnd(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeDoEnd: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->doEnd());
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeDoReset(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeDoReset: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->doReset());
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeDoPerform(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint mode) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeDoPerform: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->doPerform(static_cast<int>(mode)));
}

// -------------------------
// Driver
// -------------------------
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeEnableDriver(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint driverId, jstring localUri) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeEnableDriver: null wrapper");
return -1;
}
std::string uri = jstringToStdString(env, localUri);
return static_cast<jint>(wrapper->enableDriver(static_cast<unsigned>(driverId), uri));
}

// -------------------------
// Status / config (return simple values or pointer handles as long)
// -------------------------
JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetStatusCopy(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetStatusCopy: null wrapper");
return 0;
}
nodestate_t* st = wrapper->getStatusCopy();
return reinterpret_cast<jlong>(st);
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetSyncedTimestamp(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetSyncedTimestamp: null wrapper");
return 0;
}
uint64_t ts = wrapper->getSyncedTimestamp();
return static_cast<jlong>(ts);
}

JNIEXPORT jboolean JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeStoreConfiguration(JNIEnv* env, jobject /*thiz*/, jlong ptr, jlong depot_ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
IDepot* depot = reinterpret_cast<IDepot*>(depot_ptr);
if (!wrapper) {
throwJavaException(env, "nativeStoreConfiguration: null wrapper");
return JNI_FALSE;
}
bool ok = wrapper->storeConfiguration(depot);
return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeLoadConfiguration(JNIEnv* env, jobject /*thiz*/, jlong ptr, jlong depot_ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
IDepot* depot = reinterpret_cast<IDepot*>(depot_ptr);
if (!wrapper) {
throwJavaException(env, "nativeLoadConfiguration: null wrapper");
return JNI_FALSE;
}
bool ok = wrapper->loadConfiguration(depot);
return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeDoStatisticsReset(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeDoStatisticsReset: null wrapper");
return JNI_FALSE;
}
bool ok = wrapper->doStatisticsReset();
return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetSystemStatistics(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint syscode) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetSystemStatistics: null wrapper");
return 0;
}
uint64_t val = wrapper->getSystemStatistics(static_cast<int>(syscode));
return static_cast<jlong>(val);
}

// -------------------------
// Mapping & availability
// -------------------------
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeMap(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeMap: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->map(static_cast<din_t>(din)));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeMapWithAddr(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jint link, jstring suri) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeMapWithAddr: null wrapper");
return -1;
}
std::string s = jstringToStdString(env, suri);
return static_cast<jint>(wrapper->map_with_addr(static_cast<din_t>(din), static_cast<link_t>(link), s.c_str()));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeMapWithAddrAndSKey(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jint link, jstring suri, jstring skey) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeMapWithAddrAndSKey: null wrapper");
return -1;
}
std::string s = jstringToStdString(env, suri);
std::string k = jstringToStdString(env, skey);
return static_cast<jint>(wrapper->map_with_addr_skey(static_cast<din_t>(din), static_cast<link_t>(link), s.c_str(), k.c_str()));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeRemoveNode(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeRemoveNode: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->removeNode(static_cast<din_t>(din)));
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeListNodesCopy(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeListNodesCopy: null wrapper");
return 0;
}
dinlist_t* list = wrapper->listNodesCopy();
return reinterpret_cast<jlong>(list);
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeLocate(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeLocate: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->locate(static_cast<din_t>(din)));
}

// -------------------------
// Exchange / realtime
// -------------------------
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeSendStatus(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeSendStatus: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->send_status(static_cast<din_t>(din)));
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeStatusCopy(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeStatusCopy: null wrapper");
return 0;
}
nodestate_t* st = wrapper->statusCopy(static_cast<din_t>(din));
return reinterpret_cast<jlong>(st);
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeFetchCopy(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jint opts, jobject outErrHolder) {
// outErrHolder is optional; we'll return pointer and the caller should call a separate method to get error if needed.
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeFetchCopy: null wrapper");
return 0;
}
int err = 0;
nodestate_t* st = wrapper->fetchCopy(static_cast<din_t>(din), static_cast<uint16_t>(opts), err);
// We return the pointer; if user needs error, they should provide an int holder. For simplicity, we don't fill outErrHolder here.
(void) outErrHolder;
return reinterpret_cast<jlong>(st);
}

/*JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeSetATSMaxError(JNIEnv* env, jobject thiz, jlong ptr, jint error) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeSetATSMaxError: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->setATSMaxError(static_cast<int32_t>(error)));
}*/

JNIEXPORT jboolean JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeUse(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeUseSession: null wrapper");
return JNI_FALSE;
}
bool ok = wrapper->use(static_cast<din_t>(din));
return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeEnd(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeEndSession: null wrapper");
return JNI_FALSE;
}
bool ok = wrapper->end(static_cast<din_t>(din));
return ok ? JNI_TRUE : JNI_FALSE;
}

// send: takes byte[] and returns number of bytes sent (int)
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeSend(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jbyteArray data) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeSend: null wrapper");
return -1;
}
if (data == nullptr) return 0;
jsize len = env->GetArrayLength(data);
jbyte* bytes = env->GetByteArrayElements(data, nullptr);
if (!bytes) return 0;
unsigned sent = wrapper->send(static_cast<din_t>(din), reinterpret_cast<const unsigned char*>(bytes), static_cast<unsigned>(len));
env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
return static_cast<jint>(sent);
}

// received: returns number of bytes available
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeReceived(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeReceived: null wrapper");
return -1;
}
unsigned r = wrapper->received(static_cast<din_t>(din));
return static_cast<jint>(r);
}

// receive: fills a byte[] buffer provided by caller and returns number of bytes read
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeReceive(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jbyteArray outBuffer) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeReceive: null wrapper");
return -1;
}
if (outBuffer == nullptr) {
throwJavaException(env, "nativeReceive: outBuffer is null");
return -1;
}
jsize maxSize = env->GetArrayLength(outBuffer);
// allocate temp buffer
std::unique_ptr<unsigned char[]> tmp(new unsigned char[maxSize]);
unsigned read = wrapper->receive(static_cast<din_t>(din), tmp.get(), static_cast<unsigned>(maxSize));
// copy back into Java array
env->SetByteArrayRegion(outBuffer, 0, static_cast<jsize>(read), reinterpret_cast<jbyte*>(tmp.get()));
return static_cast<jint>(read);
}

// -------------------------
// Transfer / typesets / DDOs
// -------------------------
// TODO: Implement thiis method when possible!
// JNIEXPORT jlong JNICALL
//         Java_sebyone_daasiot_1android_DaasWrapper_nativeListTypesetsCopy(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
// auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
// if (!wrapper) {
// throwJavaException(env, "nativeListTypesetsCopy: null wrapper");
// return 0;
// }
// tsetlist_t* list = wrapper->listTypesetsCopy();
// return reinterpret_cast<jlong>(list);
// }

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativePull(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jlong outDDOptrAddr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativePull: null wrapper");
return -1;
}
DDO* outDDO = nullptr;
int rc = wrapper->pull(static_cast<din_t>(din), &outDDO);
// If caller provided a location (outDDOptrAddr) they can store the pointer there (not done in native)
// We return the error code and the caller should call nativeGetPulledDDOPointer (separate) or change usage to receive pointer as long return.
// Simpler: return pointer as jlong on success; we'll return pointer encoded in jlong (but signature now returns int).
// To keep compatibility with signature, we store pointer into a known global map OR better: change signature - but for now:
// Return error code; user should call nativeGetLastPulledDDOPtr (not implemented). To keep simple, return error and also keep pointer as result in a separate method below.
// For clarity, we will instead return the pointer as jlong via a different JNI method (nativePullReturnPtr). Here we just return rc.
// (User: use nativePullReturnPtr below)
// Free responsibility: caller must call nativeFreeDDO(ptr)
(void) outDDO;
return static_cast<jint>(rc);
}

// Alternative pull that returns pointer directly (recommended)
JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativePullReturnPtr(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativePullReturnPtr: null wrapper");
return 0;
}
DDO* outDDO = nullptr;
int rc = wrapper->pull(static_cast<din_t>(din), &outDDO);
if (rc != static_cast<int>(ERROR_NONE)) {
// if error, free outDDO if set and return 0
if (outDDO) {
wrapper->freeDDO(outDDO);
}
return 0;
}
return reinterpret_cast<jlong>(outDDO);
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativePush(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jlong ddoPtr) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
DDO* ddo = reinterpret_cast<DDO*>(ddoPtr);
if (!wrapper) {
throwJavaException(env, "nativePush: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->push(static_cast<din_t>(din), ddo));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeAvailablesPull(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jobject countHolder) {
// countHolder is unused in this native binding. We'll return count in the jint return value along with an error code - not ideal.
// Simpler approach: call the method to get count via separate JNI if needed. Here we only call and return error code.
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeAvailablesPull: null wrapper");
return -1;
}
uint32_t count = 0;
int rc = wrapper->availablesPull(static_cast<din_t>(din), count);
// If you want to return the count to Java, consider returning a jintArray {rc, count} instead. For now, we return rc and user can call another method to fetch count.
(void) countHolder;
return static_cast<jint>(rc);
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeAddTypeset(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint typeset_code, jint typeset_size) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeAddTypeset: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->addTypeset(static_cast<uint16_t>(typeset_code), static_cast<uint16_t>(typeset_size)));
}

// -------------------------
// Security
// -------------------------

/*JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeUnlock(JNIEnv* env, jobject *//*thiz*//*, jlong ptr, jint din, jstring skey) {
    auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
    if (!wrapper) {
        throwJavaException(env, "nativeUnlock: null wrapper");
        return 0;
    }

    const char* c_skey = env->GetStringUTFChars(skey, nullptr);
    int err = 0;
    nodestate_t* result = wrapper->unlock(static_cast<din_t>(din), c_skey, err);
    env->ReleaseStringUTFChars(skey, c_skey);

    if (err != 0) {
        throwJavaException(env, "nativeUnlock: failed");
        return 0;
    }

    return reinterpret_cast<jlong>(result);
}

JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeLock(JNIEnv* env, jobject *//*thiz*//*, jlong ptr, jstring skey, jint policy) {
    auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
    if (!wrapper) {
        throwJavaException(env, "nativeLock: null wrapper");
        return 0;
    }

    const char* c_skey = env->GetStringUTFChars(skey, nullptr);
    int err = 0;
    nodestate_t* result = wrapper->lock(c_skey, static_cast<unsigned>(policy), err);
    env->ReleaseStringUTFChars(skey, c_skey);

    if (err != 0) {
        throwJavaException(env, "nativeLock: failed");
        return 0;
    }

    return reinterpret_cast<jlong>(result);
}*/

// -------------------------
// Sync
// -------------------------
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSyncNode(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jint timezone) {
    auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
    if (!wrapper) {
        throwJavaException(env, "nativeSyncNode: null wrapper");
        return 0;
    }

    int err = 0;
    nodestate_t* result = wrapper->syncNode(static_cast<din_t>(din), static_cast<unsigned>(timezone), err);
    if (err != 0) {
        throwJavaException(env, "nativeSyncNode: failed");
        return 0;
    }

    return reinterpret_cast<jlong>(result);
}

JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeSyncNet(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din, jint bubbleTime) {
    auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
    if (!wrapper) {
        throwJavaException(env, "nativeSyncNet: null wrapper");
        return 0;
    }

    int err = 0;
    nodestate_t* result = wrapper->syncNet(static_cast<din_t>(din), static_cast<unsigned>(bubbleTime), err);
    if (err != 0) {
        throwJavaException(env, "nativeSyncNet: failed");
        return 0;
    }

    return reinterpret_cast<jlong>(result);
}

// -------------------------
// Frisbee / test
// -------------------------
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeFrisbee(JNIEnv* env, jobject /*thiz*/, jlong ptr, jint din) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeFrisbee: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->frisbee(static_cast<din_t>(din)));
}

/*JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeFrisbeeIcmp(JNIEnv* env, jobject *//*thiz*//*, jlong ptr, jint din, jint timeout, jint retry) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeFrisbeeIcmp: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->frisbee_icmp(static_cast<din_t>(din), static_cast<uint32_t>(timeout), static_cast<uint32_t>(retry)));
}

JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeFrisbeeDperf(JNIEnv* env, jobject *//*thiz*//*, jlong ptr, jint din, jint sender_pkt_total, jint block_size, jint sender_trip_period) {
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeFrisbeeDperf: null wrapper");
return -1;
}
return static_cast<jint>(wrapper->frisbee_dperf(static_cast<din_t>(din), static_cast<uint32_t>(sender_pkt_total), static_cast<uint32_t>(block_size), static_cast<uint32_t>(sender_trip_period)));
}

JNIEXPORT jlong JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetFrisbeeDperfResult(JNIEnv* env, jobject *//*thiz*//*, jlong ptr) {
// Return pointer to a heap-allocated dperf_info_result (caller must free)
auto* wrapper = reinterpret_cast<DaasWrapper*>(ptr);
if (!wrapper) {
throwJavaException(env, "nativeGetFrisbeeDperfResult: null wrapper");
return 0;
}
dperf_info_result result = wrapper->get_frisbee_dperf_result();
dperf_info_result* heap = new dperf_info_result(result);
return reinterpret_cast<jlong>(heap);
}*/

// -------------------------
// Free helpers for objects allocated by wrapper
// -------------------------
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFreeNodeState(JNIEnv* env, jobject /*thiz*/, jlong /*wrapperPtr*/, jlong nodeStatePtr) {
nodestate_t* p = reinterpret_cast<nodestate_t*>(nodeStatePtr);
if (p) DaasWrapper::freeNodeState(p);
}

JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFreeDDO(JNIEnv* env, jobject /*thiz*/, jlong /*wrapperPtr*/, jlong ddoPtr) {
DDO* p = reinterpret_cast<DDO*>(ddoPtr);
if (p) DaasWrapper::freeDDO(p);
}

JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFreeDinList(JNIEnv* env, jobject /*thiz*/, jlong /*wrapperPtr*/, jlong ptrList) {
dinlist_t* p = reinterpret_cast<dinlist_t*>(ptrList);
if (p) DaasWrapper::freeDinList(p);
}

JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFreeTsetList(JNIEnv* env, jobject /*thiz*/, jlong /*wrapperPtr*/, jlong ptrList) {
tsetlist_t* p = reinterpret_cast<tsetlist_t*>(ptrList);
if (p) DaasWrapper::freeTsetList(p);
}

JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeFreeDperfResult(JNIEnv* env, jobject /*thiz*/, jlong /*wrapperPtr*/, jlong ptr) {
dperf_info_result* p = reinterpret_cast<dperf_info_result*>(ptr);
if (p) delete p;
}

} // extern "C"
