#include <jni.h>
#include <string>
#include "daas_wrapper.hpp"

extern "C" {

// nativeCreate()
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeCreate(JNIEnv* env, jobject /*thiz*/) {
    return reinterpret_cast<jlong>(new DaasAPI());
}

// nativeDestroy()
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeDestroy(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
delete reinterpret_cast<DaasAPI*>(ptr);
}

// nativeGetVersion()
JNIEXPORT jstring JNICALL
        Java_sebyone_daasiot_1android_DaasWrapper_nativeGetVersion(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto daas = reinterpret_cast<DaasAPI*>(ptr);
const char* version = daas->getVersion();
return env->NewStringUTF(version);
}

// nativeListAvailableDrivers()
JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_DaasWrapper_nativeListAvailableDrivers(JNIEnv* env, jobject /*thiz*/, jlong ptr) {
auto daas = reinterpret_cast<DaasAPI*>(ptr);
const char* drivers = daas->listAvailableDrivers();
return env->NewStringUTF(drivers);
}

}
