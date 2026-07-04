#ifndef DAASIOT_JAVA_BRIDGE_HPP
#define DAASIOT_JAVA_BRIDGE_HPP

#include <jni.h>
#include <string>

// Pure JNI<->Java helpers. These depend only on <jni.h>, never on the native libdaas headers, so
// they stay free of any dependency on the proprietary libdaas library.

std::string toStdString(JNIEnv* env, jstring js);

// Generic bridge to a sebyone.daasiot_android.NativeEnum: calls the enum class's static
// fromValue(Int) factory method to box a raw native enum value as a Kotlin enum instance.
jobject enumFromValue(JNIEnv* env, const char* className, jint value);

// Generic bridge to a sebyone.daasiot_android.NativeEnum: calls value() to unbox a Kotlin enum
// instance back to its raw native enum value.
jint enumValue(JNIEnv* env, jobject enumObject);

#endif // DAASIOT_JAVA_BRIDGE_HPP
