#include "../include/java_bridge.hpp"

std::string toStdString(JNIEnv* env, jstring js) {
    if (!js) return {};
    const char* c = env->GetStringUTFChars(js, nullptr);
    std::string s = c ? c : "";
    env->ReleaseStringUTFChars(js, c);
    return s;
}

jobject enumFromValue(JNIEnv* env, const char* className, jint value) {
    jclass cls = env->FindClass(className);
    if (!cls) return nullptr;
    std::string sig = std::string("(I)L") + className + ";";
    jmethodID m = env->GetStaticMethodID(cls, "fromValue", sig.c_str());
    return m ? env->CallStaticObjectMethod(cls, m, value) : nullptr;
}

jint enumValue(JNIEnv* env, jobject enumObject) {
    if (!enumObject) return 0;
    jclass cls = env->GetObjectClass(enumObject);
    jmethodID m = env->GetMethodID(cls, "value", "()I");
    return m ? env->CallIntMethod(enumObject, m) : 0;
}
