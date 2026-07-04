/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: JavaBridge.cpp
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
