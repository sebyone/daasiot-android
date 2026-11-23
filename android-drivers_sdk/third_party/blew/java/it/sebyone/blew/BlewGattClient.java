// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
package it.sebyone.blew;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * GATT client towards one remote device. Every operation is asynchronous: the method returns
 * whether it was started, the result arrives through the matching native callback. The native
 * side (RemoteDeviceAndroid) runs one operation at a time, as Android requires.
 */
@SuppressLint("MissingPermission")
final class BlewGattClient {
    private static final UUID CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private final long handle;
    private final Context context;
    private final String address;
    private BluetoothGatt gatt;

    private final BluetoothGattCallback callback = new BluetoothGattCallback() {
        @Override public void onConnectionStateChange(BluetoothGatt g, int status, int newState) {
            nativeOnConnectionState(handle, status, newState == BluetoothProfile.STATE_CONNECTED);
        }

        @Override public void onServicesDiscovered(BluetoothGatt g, int status) {
            nativeOnServicesDiscovered(handle, status);
        }

        @Override public void onMtuChanged(BluetoothGatt g, int mtu, int status) {
            nativeOnMtu(handle, mtu, status);
        }

        @Override public void onReadRemoteRssi(BluetoothGatt g, int rssi, int status) {
            nativeOnRssi(handle, rssi, status);
        }

        // API < 33
        @Override @SuppressWarnings("deprecation")
        public void onCharacteristicRead(BluetoothGatt g, BluetoothGattCharacteristic c, int status) {
            nativeOnRead(handle, status, c.getValue());
        }

        // API 33+
        @Override public void onCharacteristicRead(BluetoothGatt g, BluetoothGattCharacteristic c, byte[] value,
                                                   int status) {
            nativeOnRead(handle, status, value);
        }

        @Override public void onCharacteristicWrite(BluetoothGatt g, BluetoothGattCharacteristic c, int status) {
            nativeOnWrite(handle, status);
        }

        @Override public void onDescriptorWrite(BluetoothGatt g, BluetoothGattDescriptor d, int status) {
            nativeOnWrite(handle, status);
        }

        // API < 33
        @Override @SuppressWarnings("deprecation")
        public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic c) {
            nativeOnChanged(handle, c.getService().getUuid().toString(), c.getUuid().toString(), c.getValue());
        }

        // API 33+
        @Override public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic c, byte[] value) {
            nativeOnChanged(handle, c.getService().getUuid().toString(), c.getUuid().toString(), value);
        }
    };

    BlewGattClient(Context context, long handle, String address) {
        this.context = context;
        this.handle = handle;
        this.address = address;
    }

    boolean connect() {
        BluetoothAdapter a = BlewScanner.adapter(context);
        if (a == null) return false;
        try {
            BluetoothDevice d = a.getRemoteDevice(address);
            gatt = Build.VERSION.SDK_INT >= 23 ? d.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
                                               : d.connectGatt(context, false, callback);
            return gatt != null;
        } catch (RuntimeException e) {
            return false;
        }
    }

    boolean requestMtu(int mtu) { return gatt != null && gatt.requestMtu(mtu); }

    boolean readRssi() { return gatt != null && gatt.readRemoteRssi(); }

    boolean discoverServices() { return gatt != null && gatt.discoverServices(); }

    /** "service|characteristic|properties" for every characteristic. */
    String[] services() {
        List<String> out = new ArrayList<>();
        if (gatt == null) return new String[0];
        for (BluetoothGattService s : gatt.getServices()) {
            out.add(s.getUuid() + "||0");
            for (BluetoothGattCharacteristic c : s.getCharacteristics())
                out.add(s.getUuid() + "|" + c.getUuid() + "|" + c.getProperties());
        }
        return out.toArray(new String[0]);
    }

    private BluetoothGattCharacteristic find(String svc, String chr) {
        if (gatt == null) return null;
        BluetoothGattService s = gatt.getService(UUID.fromString(svc));
        return s == null ? null : s.getCharacteristic(UUID.fromString(chr));
    }

    @SuppressWarnings("deprecation")
    boolean read(String svc, String chr) {
        BluetoothGattCharacteristic c = find(svc, chr);
        return c != null && gatt.readCharacteristic(c);
    }

    @SuppressWarnings("deprecation")
    boolean write(String svc, String chr, byte[] value, boolean withResponse) {
        BluetoothGattCharacteristic c = find(svc, chr);
        if (c == null) return false;
        int type = withResponse ? BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                                : BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE;
        if (Build.VERSION.SDK_INT >= 33) return gatt.writeCharacteristic(c, value, type) == 0;
        c.setWriteType(type);
        c.setValue(value);
        return gatt.writeCharacteristic(c);
    }

    /** Enables/disables notifications or indications; completion arrives as a write result. */
    @SuppressWarnings("deprecation")
    boolean setNotify(String svc, String chr, boolean enable, boolean indicate) {
        BluetoothGattCharacteristic c = find(svc, chr);
        if (c == null || !gatt.setCharacteristicNotification(c, enable)) return false;
        BluetoothGattDescriptor d = c.getDescriptor(CCCD);
        if (d == null) return false;
        byte[] value = !enable ? BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE
                : indicate ? BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
                           : BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
        if (Build.VERSION.SDK_INT >= 33) return gatt.writeDescriptor(d, value) == 0;
        d.setValue(value);
        return gatt.writeDescriptor(d);
    }

    void disconnect() {
        if (gatt != null) gatt.disconnect();
    }

    void close() {
        if (gatt != null) gatt.close();
        gatt = null;
    }

    private static native void nativeOnConnectionState(long handle, int status, boolean connected);
    private static native void nativeOnServicesDiscovered(long handle, int status);
    private static native void nativeOnMtu(long handle, int mtu, int status);
    private static native void nativeOnRssi(long handle, int rssi, int status);
    private static native void nativeOnRead(long handle, int status, byte[] value);
    private static native void nativeOnWrite(long handle, int status);
    private static native void nativeOnChanged(long handle, String service, String characteristic, byte[] value);
}
