#include <jni.h>
#include <android/log.h>
#include <string>
#include "../include/daas.hpp"
#include "../include/daas_types.hpp"

#define LOG_TAG "DaaS-Native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* ============================================================
   Global state
   ============================================================ */

static JavaVM* g_vm = nullptr;
static DaasAPI* g_daas = nullptr;

static const typeset_t SIMPLE_TYPESET = 1;

/* ============================================================
   Helpers
   ============================================================ */

static JNIEnv* getEnv() {
    JNIEnv* env = nullptr;
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        g_vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

static std::string toStdString(JNIEnv* env, jstring js) {
    if (!js) return {};
    const char* c = env->GetStringUTFChars(js, nullptr);
    std::string s = c ? c : "";
    env->ReleaseStringUTFChars(js, c);
    return s;
}

/* ============================================================
   Events bridge
   ============================================================ */

class DaasEvents : public IDaasApiEvent {
public:

    void dinAccepted(din_t din) override {
        LOGD("dinAccepted %lu", din);
    }

    void nodeConnectedToNetwork(din_t sid, din_t din) override {
        LOGD("nodeConnected sid=%lu din=%lu", sid, din);
    }

    void nodeDiscovered(din_t din, link_t link) override {
        LOGD("nodeDiscovered din=%lu link=%u", din, link);
    }

    void ddoReceived(int payload_size, typeset_t typeset, din_t origin) override {
        LOGD("ddoReceived origin=%lu size=%d typeset=%u", origin, payload_size, typeset);

        if (typeset != SIMPLE_TYPESET) return;
        if (!g_daas) return;

        DDO* inbound = nullptr;
        if (g_daas->pull(origin, &inbound) != ERROR_NONE || !inbound) {
            LOGE("pull failed");
            return;
        }

        int value = 0;
        inbound->getPayloadAsBinary((uint8_t*)&value, 0, 1);

        JNIEnv* env = getEnv();
        jclass cls = env->FindClass("sebyone/daasiot_android/DaasWrapper");
        jmethodID mid = env->GetStaticMethodID(cls, "onDDOReceived", "(JI)V");

        if (mid)
            env->CallStaticVoidMethod(cls, mid, (jlong)origin, (jint)value);

        delete inbound;
    }

    void frisbeeReceived(din_t) override {}
    void nodeStateReceived(din_t) override {}
    void atsSyncCompleted(din_t) override {}
    void frisbeeDperfCompleted(din_t, uint32_t, uint32_t) override {}
};

static DaasEvents g_events;

/* ============================================================
   JNI lifecycle
   ============================================================ */

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    LOGD("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

/* ============================================================
   Core API
   ============================================================ */

extern "C" {

/* ---------------- Info / strings ---------------- */

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetVersion(
        JNIEnv* env, jclass) {

    if (!g_daas) {
        LOGD("[DaaS] nativeGetVersion: g_daas is null");
        return env->NewStringUTF("");
    }

    const char* ver = g_daas->getVersion();
    LOGD("[DaaS] nativeGetVersion -> %s", ver ? ver : "(null)");

    return env->NewStringUTF(ver ? ver : "");
}

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeGetBuildInfo(
        JNIEnv* env, jclass) {

    if (!g_daas) {
        LOGD("[DaaS] nativeGetBuildInfo: g_daas is null");
        return env->NewStringUTF("");
    }

    const char* info = g_daas->getBuildInfo();
    LOGD("[DaaS] nativeGetBuildInfo -> %s", info ? info : "(null)");

    return env->NewStringUTF(info ? info : "");
}

JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeListAvailableDrivers(
        JNIEnv* env, jclass) {

    if (!g_daas) {
        LOGD("[DaaS] nativeListAvailableDrivers: g_daas is null");
        return env->NewStringUTF("");
    }

    const char* list = g_daas->listAvailableDrivers();
    LOGD("[DaaS] nativeListAvailableDrivers -> %s", list ? list : "(null)");

    return env->NewStringUTF(list ? list : "");
}

} // extern "C"


extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeCreate(
        JNIEnv*, jclass) {
    if (g_daas) return;

    LOGD("[DaaS] Creating DaasAPI instance");
    g_daas = new DaasAPI(&g_events);
    LOGD("[DaaS] Library Version: %s", g_daas->getVersion());
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeInit(
        JNIEnv*, jclass, din_t sid, din_t din) {

    LOGD("[DaaS] Initializing with SID=%lu DIN=%lu", sid, din);
    auto err = g_daas->doInit(sid, din);
    LOGD("[DaaS] doInit() -> %d", err);
    return err;
}

extern "C"
JNIEXPORT jint JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeReset(
        JNIEnv*, jclass) {

LOGD("[DaaS] Resetting Node... Node will be disconnected from the network");
auto err = g_daas->doReset();
LOGD("[DaaS] doReset() -> %d", err);
return err;
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeEnableDriver(
        JNIEnv* env, jclass, jstring uri) {

    const char* c_uri = env->GetStringUTFChars(uri, nullptr);
    LOGD("[DaaS] Enabling driver LINK_INET4 with URI %s", c_uri);

    auto err = g_daas->enableDriver(_LINK_INET4, c_uri);

    LOGD("[DaaS] enableDriver() -> %d", err);

    env->ReleaseStringUTFChars(uri, c_uri);
    return err;
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeMap(
        JNIEnv* env, jclass, jlong din, jstring uri) {

    if (!g_daas) return -1;

    std::string suri = toStdString(env, uri);
    LOGD("map din=%lu uri=%s", din, suri.c_str());
    auto err = g_daas->map((din_t)din, _LINK_INET4, suri.c_str());

    LOGD("[DaaS] map() -> %d", err);
    return err;
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativePerform(
        JNIEnv*, jclass) {

    if (!g_daas) return -1;
    return g_daas->doPerform(PERFORM_CORE_NO_THREAD);
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_PushDDO(
        JNIEnv* env,
        jclass,
        jlong remoteDin,
        jint typeset,
        jbyteArray payload) {

    if (!g_daas) return -1;
    if (!payload) return -2;

    jsize size = env->GetArrayLength(payload);
    if (size <= 0) return -3;

    jbyte* data = env->GetByteArrayElements(payload, nullptr);

    DDO ddo((typeset_t)typeset);
    ddo.setPayload((uint8_t*)data, (size_t)size);

    LOGD("PushDDO -> din=%lu typeset=%d size=%d",
         (unsigned long)remoteDin,
         typeset,
         (int)size);

    auto err = g_daas->push((din_t)remoteDin, &ddo);

    env->ReleaseByteArrayElements(payload, data, JNI_ABORT);

    return err;
}


extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDestroy(
        JNIEnv*, jclass) {

    if (!g_daas) return;

    LOGD("Destroying DaasAPI");
    g_daas->doEnd();
    delete g_daas;
    g_daas = nullptr;
}
