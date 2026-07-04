#include "../include/daasiot_jni.hpp"

#include <android/log.h>

#define LOG_TAG "DaaS-Native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Only the JavaVM is process-wide by necessity (needed to attach native-thread callbacks to the
// JVM). The DaaS instance itself is NOT global: each nativeCreate() call owns its own ApiHandle,
// so multiple DaasWrapper instances can coexist without clobbering each other's state.
JavaVM* g_vm = nullptr;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    LOGD("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

static JNIEnv* getEnv() {
    JNIEnv* env = nullptr;
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        g_vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

DaasEvents::DaasEvents(JNIEnv* env, jobject handler)
        : handler_(handler ? env->NewGlobalRef(handler) : nullptr),
          handlerClass_(handler ? static_cast<jclass>(env->NewGlobalRef(env->GetObjectClass(handler))) : nullptr) {
}

DaasEvents::~DaasEvents() {
    if (!handler_ && !handlerClass_) return;
    JNIEnv* env = getEnv();
    if (handler_) env->DeleteGlobalRef(handler_);
    if (handlerClass_) env->DeleteGlobalRef(handlerClass_);
}

void DaasEvents::callLong(const char* name, din_t din) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, name, "(J)V");
    if (m) {
        env->CallVoidMethod(handler_, m, (jlong)din);
    } else {
        env->ExceptionClear();
    }
}

void DaasEvents::dinAccepted(din_t din) {
    callLong("dinAccepted", din);
}

void DaasEvents::ddoReceived(int payload_size, typeset_t typeset, din_t origin) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, "ddoReceived", "(IIJ)V");
    if (m) {
        env->CallVoidMethod(handler_, m, (jint)payload_size, (jint)typeset, (jlong)origin);
    } else {
        env->ExceptionClear();
    }
}

void DaasEvents::frisbeeReceived(din_t din) {
    callLong("frisbeeReceived", din);
}

void DaasEvents::nodeStateReceived(din_t din) {
    callLong("nodeStateReceived", din);
}

void DaasEvents::atsSyncCompleted(din_t din) {
    callLong("atsSyncCompleted", din);
}

void DaasEvents::frisbeeDperfCompleted(din_t din, uint32_t packets_sent, uint32_t block_size) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, "frisbeeDperfCompleted", "(JJJ)V");
    if (m) {
        env->CallVoidMethod(handler_, m, (jlong)din, (jlong)packets_sent, (jlong)block_size);
    } else {
        env->ExceptionClear();
    }
}

void DaasEvents::networkDiscovered(din_t din, din_t sid, link_t link) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, "networkDiscovered", "(JJLsebyone/daasiot_android/enums/LinkType;)V");
    if (m) {
        jobject linkObj = enumFromValue(env, "sebyone/daasiot_android/enums/LinkType", static_cast<jint>(link));
        env->CallVoidMethod(handler_, m, (jlong)din, (jlong)sid, linkObj);
        if (linkObj) env->DeleteLocalRef(linkObj);
    } else {
        env->ExceptionClear();
    }
}

void DaasEvents::nodeConnectedToNetwork(din_t sid, din_t din) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, "nodeConnectedToNetwork", "(JJ)V");
    if (m) {
        env->CallVoidMethod(handler_, m, (jlong)sid, (jlong)din);
    } else {
        env->ExceptionClear();
    }
}

void DaasEvents::streamInfoReceived(din_t din, stream_type pkt_type, uint32_t stream_id) {
    if (!handler_) return;
    JNIEnv* env = getEnv();
    jmethodID m = env->GetMethodID(handlerClass_, "streamInfoReceived", "(JLsebyone/daasiot_android/enums/StreamType;J)V");
    if (m) {
        jobject typeObj = enumFromValue(env, "sebyone/daasiot_android/enums/StreamType", static_cast<jint>(pkt_type));
        env->CallVoidMethod(handler_, m, (jlong)din, typeObj, (jlong)stream_id);
        if (typeObj) env->DeleteLocalRef(typeObj);
    } else {
        env->ExceptionClear();
    }
}
