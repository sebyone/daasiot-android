// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
#pragma once

#if defined(__ANDROID__)
#include <jni.h>

#include <blew/export.h>

namespace blew {
namespace android {

/**
 * Must run once, from a Java thread, before any other call. The Java helper
 * it.sebyone.blew.Blew.init(context) calls it for you.
 */
BLEW_EXPORT void initialize(JNIEnv* env, jobject context);

}  // namespace android
}  // namespace blew
#endif
