#include <jni.h>
#include <string>
#include <iostream>
#include "../include/daas.hpp"  // From src/main/include

extern "C"
JNIEXPORT jstring JNICALL
Java_sebyone_daasiot_1android_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    // Just return a test message
    std::string message = "DaaS static library linked successfully!";
    return env->NewStringUTF(message.c_str());
}
