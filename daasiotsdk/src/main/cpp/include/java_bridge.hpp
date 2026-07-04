/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: java_bridge.hpp
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
