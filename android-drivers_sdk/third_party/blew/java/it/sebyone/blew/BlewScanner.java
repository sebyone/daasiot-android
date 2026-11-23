// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
package it.sebyone.blew;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.SparseArray;

import java.util.List;

/** Central-role scanning. Native side: CentralAndroid. */
@SuppressLint("MissingPermission")
final class BlewScanner {
    static final int OK = 0, BLUETOOTH_OFF = 1, UNAUTHORIZED = 2, FAILED = 3;

    private final long handle;
    private final Context context;
    private BluetoothLeScanner scanner;

    private final ScanCallback callback = new ScanCallback() {
        @Override public void onScanResult(int callbackType, ScanResult result) { report(result); }

        @Override public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult r : results) report(r);
        }

        @Override public void onScanFailed(int errorCode) { nativeOnScanFailed(handle, errorCode); }
    };

    BlewScanner(Context context, long handle) {
        this.context = context;
        this.handle = handle;
    }

    static BluetoothAdapter adapter(Context context) {
        BluetoothManager m = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        return m == null ? null : m.getAdapter();
    }

    String address() {
        try {
            BluetoothAdapter a = adapter(context);
            return a == null ? "" : a.getAddress();
        } catch (SecurityException e) {
            return "";
        }
    }

    boolean isPowered() {
        BluetoothAdapter a = adapter(context);
        return a != null && a.isEnabled();
    }

    int start() {
        BluetoothAdapter a = adapter(context);
        if (a == null || !a.isEnabled()) return BLUETOOTH_OFF;
        try {
            scanner = a.getBluetoothLeScanner();
            if (scanner == null) return BLUETOOTH_OFF;
            ScanSettings settings = new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .setReportDelay(0)
                    .build();
            scanner.startScan(null, settings, callback);
            return OK;
        } catch (SecurityException e) {
            return UNAUTHORIZED;
        } catch (RuntimeException e) {
            return FAILED;
        }
    }

    void stop() {
        try {
            if (scanner != null) scanner.stopScan(callback);
        } catch (RuntimeException e) {
            // radio turned off meanwhile
        }
        scanner = null;
    }

    private void report(ScanResult r) {
        BluetoothDevice d = r.getDevice();
        ScanRecord rec = r.getScanRecord();
        String name = null;
        String[] uuids = new String[0];
        int[] ids = new int[0];
        byte[][] data = new byte[0][];
        int tx = Integer.MIN_VALUE;
        if (rec != null) {
            name = rec.getDeviceName();
            List<ParcelUuid> list = rec.getServiceUuids();
            if (list != null) {
                uuids = new String[list.size()];
                for (int i = 0; i < uuids.length; i++) uuids[i] = list.get(i).toString();
            }
            SparseArray<byte[]> mfg = rec.getManufacturerSpecificData();
            if (mfg != null) {
                ids = new int[mfg.size()];
                data = new byte[mfg.size()][];
                for (int i = 0; i < mfg.size(); i++) {
                    ids[i] = mfg.keyAt(i);
                    data[i] = mfg.valueAt(i);
                }
            }
            tx = rec.getTxPowerLevel();
        }
        boolean connectable = Build.VERSION.SDK_INT < 26 || r.isConnectable();
        nativeOnScanResult(handle, d.getAddress(), name == null ? "" : name, r.getRssi(),
                tx == Integer.MIN_VALUE ? -32768 : tx, connectable, uuids, ids, data);
    }

    private static native void nativeOnScanResult(long handle, String address, String name, int rssi, int txPower,
                                                  boolean connectable, String[] uuids, int[] mfgIds, byte[][] mfgData);

    private static native void nativeOnScanFailed(long handle, int errorCode);
}
