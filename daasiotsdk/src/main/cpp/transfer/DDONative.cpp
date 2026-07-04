/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: DDONative.cpp
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
