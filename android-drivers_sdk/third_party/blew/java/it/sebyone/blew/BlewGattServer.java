// SPDX-License-Identifier: MPL-2.0
// ble-wrapper (blew) — © 2019–2026 Sebyone Srl
package it.sebyone.blew;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.os.Build;
import android.os.ParcelUuid;

import java.io.ByteArrayOutputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * GATT server + advertiser. CCCD writes, read offsets and prepared (long) writes are handled here;
 * the native side (GattServerAndroid) sees plain read/write/subscribe events per peer address.
 */
@SuppressLint("MissingPermission")
final class BlewGattServer {
    static final int F_READ = 1, F_WRITE = 2, F_WRITE_NR = 4, F_NOTIFY = 8, F_INDICATE = 16;
    private static final UUID CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private final long handle;
    private final Context context;
    private BluetoothGattServer server;
    private BluetoothLeAdvertiser advertiser;
    private final Map<String, ByteArrayOutputStream> prepared = new HashMap<>();
    private final Map<String, BluetoothDevice> devices = new HashMap<>();

    private final AdvertiseCallback advCallback = new AdvertiseCallback() {
        @Override public void onStartSuccess(AdvertiseSettings s) { nativeOnAdvertising(handle, 0); }
        @Override public void onStartFailure(int errorCode) { nativeOnAdvertising(handle, errorCode); }
    };

    private final BluetoothGattServerCallback callback = new BluetoothGattServerCallback() {
        @Override public void onConnectionStateChange(BluetoothDevice d, int status, int newState) {
            boolean up = newState == BluetoothProfile.STATE_CONNECTED;
            synchronized (devices) {
                if (up) devices.put(d.getAddress(), d);
                else devices.remove(d.getAddress());
            }
            nativeOnConnection(handle, d.getAddress(), up);
        }

        @Override public void onServiceAdded(int status, BluetoothGattService service) {
            nativeOnServiceAdded(handle, status);
        }

        @Override public void onMtuChanged(BluetoothDevice d, int mtu) { nativeOnMtu(handle, d.getAddress(), mtu); }

        @Override public void onNotificationSent(BluetoothDevice d, int status) {
            nativeOnNotificationSent(handle, d.getAddress(), status);
        }

        @Override public void onCharacteristicReadRequest(BluetoothDevice d, int requestId, int offset,
                                                          BluetoothGattCharacteristic c) {
            byte[] value = nativeOnRead(handle, d.getAddress(), c.getUuid().toString());
            if (value == null) value = new byte[0];
            if (offset > value.length) {
                server.sendResponse(d, requestId, BluetoothGatt.GATT_INVALID_OFFSET, offset, null);
                return;
            }
            byte[] part = new byte[value.length - offset];
            System.arraycopy(value, offset, part, 0, part.length);
            server.sendResponse(d, requestId, BluetoothGatt.GATT_SUCCESS, offset, part);
        }

        @Override public void onCharacteristicWriteRequest(BluetoothDevice d, int requestId,
                                                           BluetoothGattCharacteristic c, boolean preparedWrite,
                                                           boolean responseNeeded, int offset, byte[] value) {
            if (preparedWrite) {
                synchronized (prepared) {
                    String key = d.getAddress() + "|" + c.getUuid();
                    ByteArrayOutputStream buf = prepared.get(key);
                    if (buf == null) prepared.put(key, buf = new ByteArrayOutputStream());
                    buf.write(value, 0, value.length);
                }
            } else {
                nativeOnWrite(handle, d.getAddress(), c.getUuid().toString(), value);
            }
            if (responseNeeded) server.sendResponse(d, requestId, BluetoothGatt.GATT_SUCCESS, offset, value);
        }

        @Override public void onExecuteWrite(BluetoothDevice d, int requestId, boolean execute) {
            String prefix = d.getAddress() + "|";
            synchronized (prepared) {
                for (Map.Entry<String, ByteArrayOutputStream> e : prepared.entrySet()) {
                    if (!e.getKey().startsWith(prefix)) continue;
                    if (execute)
                        nativeOnWrite(handle, d.getAddress(), e.getKey().substring(prefix.length()),
                                e.getValue().toByteArray());
                }
                prepared.keySet().removeIf(k -> k.startsWith(prefix));
            }
            server.sendResponse(d, requestId, BluetoothGatt.GATT_SUCCESS, 0, null);
        }

        @Override public void onDescriptorWriteRequest(BluetoothDevice d, int requestId, BluetoothGattDescriptor desc,
                                                       boolean preparedWrite, boolean responseNeeded, int offset,
                                                       byte[] value) {
            if (CCCD.equals(desc.getUuid()) && value != null && value.length >= 1) {
                boolean on = (value[0] & 0x03) != 0;
                nativeOnSubscribe(handle, d.getAddress(), desc.getCharacteristic().getUuid().toString(), on);
            }
            if (responseNeeded) server.sendResponse(d, requestId, BluetoothGatt.GATT_SUCCESS, offset, value);
        }

        @Override public void onDescriptorReadRequest(BluetoothDevice d, int requestId, int offset,
                                                      BluetoothGattDescriptor desc) {
            server.sendResponse(d, requestId, BluetoothGatt.GATT_SUCCESS, 0,
                    BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
        }
    };

    BlewGattServer(Context context, long handle) {
        this.context = context;
        this.handle = handle;
    }

    /** 0 ok, 1 bluetooth off, 2 unauthorized, 3 not supported. */
    int open() {
        BluetoothAdapter a = BlewScanner.adapter(context);
        if (a == null || !a.isEnabled()) return 1;
        try {
            if (server == null) {
                BluetoothManager m = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
                server = m.openGattServer(context, callback);
            }
            advertiser = a.getBluetoothLeAdvertiser();
            return server == null || advertiser == null ? 3 : 0;
        } catch (SecurityException e) {
            return 2;
        }
    }

    void clearServices() {
        if (server != null) server.clearServices();
    }

    /** Adds one service; completion arrives through nativeOnServiceAdded. */
    boolean addService(String svc, String[] chrs, int[] flags) {
        BluetoothGattService s = new BluetoothGattService(UUID.fromString(svc), BluetoothGattService.SERVICE_TYPE_PRIMARY);
        for (int i = 0; i < chrs.length; i++) {
            int f = flags[i], props = 0, perms = 0;
            if ((f & F_READ) != 0) {
                props |= BluetoothGattCharacteristic.PROPERTY_READ;
                perms |= BluetoothGattCharacteristic.PERMISSION_READ;
            }
            if ((f & F_WRITE) != 0) props |= BluetoothGattCharacteristic.PROPERTY_WRITE;
            if ((f & F_WRITE_NR) != 0) props |= BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE;
            if ((f & (F_WRITE | F_WRITE_NR)) != 0) perms |= BluetoothGattCharacteristic.PERMISSION_WRITE;
            if ((f & F_NOTIFY) != 0) props |= BluetoothGattCharacteristic.PROPERTY_NOTIFY;
            if ((f & F_INDICATE) != 0) props |= BluetoothGattCharacteristic.PROPERTY_INDICATE;
            BluetoothGattCharacteristic c = new BluetoothGattCharacteristic(UUID.fromString(chrs[i]), props, perms);
            if ((f & (F_NOTIFY | F_INDICATE)) != 0)
                c.addDescriptor(new BluetoothGattDescriptor(CCCD,
                        BluetoothGattDescriptor.PERMISSION_READ | BluetoothGattDescriptor.PERMISSION_WRITE));
            s.addCharacteristic(c);
        }
        return server != null && server.addService(s);
    }

    /** Result arrives through nativeOnAdvertising. Name goes in the scan response. */
    boolean startAdvertising(String name, String svc) {
        if (advertiser == null) return false;
        try {
            BluetoothAdapter.getDefaultAdapter().setName(name);
        } catch (RuntimeException e) {
            // keep the system name
        }
        AdvertiseSettings settings = new AdvertiseSettings.Builder()
                .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
                .setConnectable(true)
                .setTimeout(0)
                .build();
        AdvertiseData data = new AdvertiseData.Builder().addServiceUuid(ParcelUuid.fromString(svc)).build();
        AdvertiseData response = new AdvertiseData.Builder().setIncludeDeviceName(true).build();
        try {
            advertiser.startAdvertising(settings, data, response, advCallback);
            return true;
        } catch (RuntimeException e) {
            return false;
        }
    }

    void stopAdvertising() {
        try {
            if (advertiser != null) advertiser.stopAdvertising(advCallback);
        } catch (RuntimeException e) {
            // radio off
        }
    }

    /** 0 queued (completion via nativeOnNotificationSent), 1 not connected, 2 failed. */
    @SuppressWarnings("deprecation")
    int notify(String address, String svc, String chr, byte[] value, boolean indicate) {
        BluetoothDevice d;
        synchronized (devices) {
            d = devices.get(address);
        }
        if (d == null || server == null) return 1;
        BluetoothGattService s = server.getService(UUID.fromString(svc));
        BluetoothGattCharacteristic c = s == null ? null : s.getCharacteristic(UUID.fromString(chr));
        if (c == null) return 2;
        if (Build.VERSION.SDK_INT >= 33) return server.notifyCharacteristicChanged(d, c, indicate, value) == 0 ? 0 : 2;
        c.setValue(value);
        return server.notifyCharacteristicChanged(d, c, indicate) ? 0 : 2;
    }

    void disconnect(String address) {
        BluetoothDevice d;
        synchronized (devices) {
            d = devices.get(address);
        }
        if (d != null && server != null) server.cancelConnection(d);
    }

    void close() {
        stopAdvertising();
        if (server != null) server.close();
        server = null;
    }

    private static native void nativeOnConnection(long handle, String address, boolean connected);
    private static native void nativeOnServiceAdded(long handle, int status);
    private static native void nativeOnAdvertising(long handle, int error);
    private static native void nativeOnMtu(long handle, String address, int mtu);
    private static native void nativeOnNotificationSent(long handle, String address, int status);
    private static native byte[] nativeOnRead(long handle, String address, String characteristic);
    private static native void nativeOnWrite(long handle, String address, String characteristic, byte[] value);
    private static native void nativeOnSubscribe(long handle, String address, String characteristic, boolean on);
}
