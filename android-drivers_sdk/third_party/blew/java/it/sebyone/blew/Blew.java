// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
package it.sebyone.blew;

import android.content.Context;

/**
 * Entry point of the blew Android backend.
 *
 * Call {@code Blew.init(context)} once, after loading the native library that links blew and
 * before any native blew call. The app must hold the runtime permissions
 * (BLUETOOTH_SCAN, BLUETOOTH_CONNECT, BLUETOOTH_ADVERTISE on Android 12+,
 * ACCESS_FINE_LOCATION before).
 */
public final class Blew {
    private Blew() {}

    public static void init(Context context) {
        nativeInit(context.getApplicationContext());
    }

    private static native void nativeInit(Context context);
}
