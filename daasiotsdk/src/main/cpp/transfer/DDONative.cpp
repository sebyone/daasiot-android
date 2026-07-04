#include "../include/daasiot_jni.hpp"

#include <vector>

// Backs the Kotlin DDO wrapper used by pull()/push(). Every function resolves its own DDO* from
// the jlong handle instead of touching any shared state.

static inline DDO* toDdo(jlong handle) {
    return reinterpret_cast<DDO*>(handle);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeCreate(
        JNIEnv*, jclass) {
    return reinterpret_cast<jlong>(new DDO());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeCreateWithTypeset(
        JNIEnv*, jclass, jint typeset) {
    return reinterpret_cast<jlong>(new DDO(static_cast<typeset_t>(typeset)));
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeDispose(
        JNIEnv*, jclass, jlong handle) {
    delete toDdo(handle);
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeClearPayload(
        JNIEnv*, jclass, jlong handle) {
    toDdo(handle)->clearPayload();
}

extern "C"
JNIEXPORT void JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeSetTypeset(
        JNIEnv*, jclass, jlong handle, jint typeset) {
    toDdo(handle)->setTypeset(static_cast<typeset_t>(typeset));
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeGetOrigin(
        JNIEnv*, jclass, jlong handle) {
    return static_cast<jlong>(toDdo(handle)->getOrigin());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeGetTimestamp(
        JNIEnv*, jclass, jlong handle) {
    return static_cast<jlong>(toDdo(handle)->getTimestamp());
}

extern "C"
JNIEXPORT jint JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeGetTypeset(
        JNIEnv*, jclass, jlong handle) {
    return static_cast<jint>(toDdo(handle)->getTypeset());
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeSetPayload(
        JNIEnv* env, jclass, jlong handle, jbyteArray data, jint offset, jint length) {

    std::vector<uint8_t> buffer(length);
    env->GetByteArrayRegion(data, offset, length, reinterpret_cast<jbyte*>(buffer.data()));
    return static_cast<jlong>(toDdo(handle)->setPayload(buffer.data(), static_cast<uint32_t>(length)));
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeGetPayloadSize(
        JNIEnv*, jclass, jlong handle) {
    return static_cast<jlong>(toDdo(handle)->getPayloadSize());
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeGetPayloadAsBinary(
        JNIEnv* env, jclass, jlong handle, jint offset, jint maxSize) {

    std::vector<uint8_t> buffer(maxSize);
    uint32_t written = toDdo(handle)->getPayloadAsBinary(buffer.data(), static_cast<unsigned>(offset), static_cast<uint32_t>(maxSize));

    jbyteArray result = env->NewByteArray((jsize)written);
    if (written > 0) {
        env->SetByteArrayRegion(result, 0, (jsize)written, reinterpret_cast<jbyte*>(buffer.data()));
    }
    return result;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeAllocatePayload(
        JNIEnv*, jclass, jlong handle, jlong size) {
    return static_cast<jlong>(toDdo(handle)->allocatePayload(static_cast<uint32_t>(size)));
}

extern "C"
JNIEXPORT jlong JNICALL
Java_sebyone_daasiot_1android_entity_DDO_nativeAppendPayloadData(
        JNIEnv* env, jclass, jlong handle, jbyteArray data, jint offset, jint length) {

    std::vector<uint8_t> buffer(length);
    env->GetByteArrayRegion(data, offset, length, reinterpret_cast<jbyte*>(buffer.data()));
    return static_cast<jlong>(toDdo(handle)->appendPayloadData(buffer.data(), static_cast<uint32_t>(length)));
}
