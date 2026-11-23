/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: DaasNode.java — android_daas (daasiot-android)
 *
 * The daas node of an Android app: the Java binding of daas::sdk::Node (android_daas.hpp). One node per process
 * (libdaas has one). It is persisted in the files folder of the app (file_depot): the first start applies the
 * Config, the next ones restore the saved configuration (SID, DIN, drivers, map) and apply only its policies.
 * The events of libdaas reach the DaasListener on the service thread of the node ("daas-service"), where the whole
 * API can be called; the BLE driver needs blew, initialised here with the Context.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation
 */
package it.sebyone.daas;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import it.sebyone.blew.Blew;

/**
 * A daas node on Android.
 *
 * <pre>{@code
 * DaasNode.Config config = new DaasNode.Config(100, 101)
 *         .driver(Link.BLE, "00000065")
 *         .driver(Link.INET4, "", 2101)
 *         .acceptRequests(AcceptRequests.ALL);
 * DaasNode node = DaasNode.create(context, config);   // restored from filesDir/daas when saved before
 * node.setListener(new DaasListener() {
 *     @Override public void onDdoReceived(long din, int typeset, int size) {
 *         for (Ddo ddo; (ddo = node.pull(din)) != null; ) handle(ddo);
 *     }
 * });
 * node.start();
 * }</pre>
 */
public final class DaasNode implements AutoCloseable {

    private static final String TAG = "DaasNode";
    /** The folder of the node inside Context.getFilesDir(). */
    public static final String STATE_FOLDER = "daas";
    /** Period of the service thread: how long it waits for an event before checking the pending saves. */
    public static final int SERVICE_PERIOD_MS = 200;

    static {
        System.loadLibrary("android_daas_jni");
    }

    private static DaasNode instance;
    private static volatile DaasListener listener;

    private final File stateDir;
    private final int initializationResult;
    private volatile Thread service;
    private volatile boolean running;

    /** The configuration applied the first time (nothing saved in the state folder yet). */
    public static final class Config {
        long sid, din;
        final List<Integer> driverLinks = new ArrayList<>();
        final List<String> driverUris = new ArrayList<>();
        final List<Integer> driverPorts = new ArrayList<>();
        final List<Long> mapDins = new ArrayList<>();
        final List<Integer> mapLinks = new ArrayList<>();
        final List<String> mapUris = new ArrayList<>();
        final List<String> mapKeys = new ArrayList<>();
        final List<Integer> optionKeys = new ArrayList<>();
        final List<Integer> optionValues = new ArrayList<>();
        int acceptRequests = -1, discoveryState = -1, ddoPolicy = -1, atsMaxError = -1;
        long joinSid;
        int joinLink, joinTimeout;
        String hardwareVersion = "";

        public Config(long sid, long din) { this.sid = sid; this.din = din; }

        /** A local driver: for BLE the uri is the blew node id (8 hexadecimal digits), for INET4 "ip:port". */
        public Config driver(Link link, String uri) { return driver(link, uri, 0); }
        /** A local driver on every address of the link (empty uri) on {@code port}: "0.0.0.0:port" for INET4. */
        public Config driver(Link link, String uri, int port) {
            driverLinks.add(link.value); driverUris.add(uri == null ? "" : uri); driverPorts.add(port); return this;
        }
        public Config map(long din, Link link, String uri) { return map(din, link, uri, ""); }
        public Config map(long din, Link link, String uri, String securityKey) {
            mapDins.add(din); mapLinks.add(link.value); mapUris.add(uri); mapKeys.add(securityKey == null ? "" : securityKey);
            return this;
        }
        /** An option of libdaas (option_t of daas_types.hpp, DaasAPI::setOptions), applied at every start. */
        public Config option(int option, int value) { optionKeys.add(option); optionValues.add(value); return this; }
        public Config acceptRequests(AcceptRequests policy) { acceptRequests = policy.value; return this; }
        public Config discoveryState(DiscoveryState state) { discoveryState = state.value; return this; }
        public Config ddoPolicy(DdoPolicy policy) { ddoPolicy = policy.value; return this; }
        public Config atsMaxError(int milliseconds) { atsMaxError = milliseconds; return this; }
        /** Joins {@code sid} over {@code link} when the configuration is applied (first start). */
        public Config join(long sid, Link link, int timeoutMs) { joinSid = sid; joinLink = link.value; joinTimeout = timeoutMs; return this; }
        /** The hver of the node, 7 characters at most; empty: "AND-" and the architecture. */
        public Config hardwareVersion(String hver) { hardwareVersion = hver == null ? "" : hver; return this; }
    }

    private DaasNode(File stateDir, int initializationResult) {
        this.stateDir = stateDir;
        this.initializationResult = initializationResult;
    }

    /** The node of the process, persisted in {@code filesDir/daas}. */
    public static synchronized DaasNode create(Context context, Config config) {
        return create(context, config, new File(context.getFilesDir(), STATE_FOLDER));
    }

    /**
     * The node of the process, persisted in {@code stateDir}. Initialises blew (BLE) with the application context.
     * @throws IllegalStateException if the process already has a node (close() it first).
     */
    public static synchronized DaasNode create(Context context, Config config, File stateDir) {
        if (instance != null) throw new IllegalStateException("libdaas has one node per process: close() the current one");
        Blew.init(context.getApplicationContext());
        final int result = nativeCreate(stateDir.getAbsolutePath(), config.sid, config.din,
                toInts(config.driverLinks), config.driverUris.toArray(new String[0]), toInts(config.driverPorts),
                toLongs(config.mapDins), toInts(config.mapLinks), config.mapUris.toArray(new String[0]),
                config.mapKeys.toArray(new String[0]), toInts(config.optionKeys), toInts(config.optionValues),
                new int[]{config.acceptRequests, config.discoveryState, config.ddoPolicy},
                config.joinSid, config.joinLink, config.joinTimeout, config.atsMaxError, config.hardwareVersion);
        instance = new DaasNode(stateDir, result);
        return instance;
    }

    /** The node of the process, null if none. */
    public static synchronized DaasNode get() { return instance; }

    /** stdout/stderr of the process (the logs of libdaas) to logcat, tag "libdaas". Once per process. */
    public static void redirectLogsToLogcat() { nativeRedirectLogs(); }

    /** The version of android_daas (daasiot-android). */
    public static String wrapperVersion() { return nativeWrapperVersion(); }

    public void setListener(DaasListener l) { listener = l; }

    /** The result of the configuration (ERROR_NONE = 0), or of the restore. */
    public int initializationResult() { return initializationResult; }
    /** True if the node was restored from its state folder instead of applying the Config. */
    public boolean restored() { return nativeRestored(); }
    public File stateDir() { return stateDir; }

    /* ---- life cycle ---------------------------------------------------------------------------------------- */

    /** Starts the core of libdaas and the service thread (events, pending saves). */
    public synchronized int start() {
        final int result = nativeStart();
        if (result != DaasError.NONE || running) return result;
        running = true;
        service = new Thread(() -> {
            while (running) nativeService(SERVICE_PERIOD_MS);
        }, "daas-service");
        service.start();
        return result;
    }

    /** Stops the core (the configuration and the queues are saved first) and the service thread. */
    public synchronized int stop() {
        running = false;
        final Thread t = service;
        service = null;
        final int result = nativeStop();
        if (t != null) {
            try { t.join(2 * SERVICE_PERIOD_MS + 1000); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        }
        return result;
    }

    /** Stops the node and frees it: the process can then create another one. */
    @Override public void close() {
        stop();
        synchronized (DaasNode.class) {
            nativeDestroy();
            if (instance == this) instance = null;
        }
    }

    public int reset() { return nativeReset(); }
    /** Saves the configuration (SID, DIN, drivers, map) now; the node also saves it at every change. */
    public boolean save() { return nativeSave(); }
    /** True after an attack panic: the node stays isolated until the process of the app is restarted. */
    public boolean panicked() { return nativePanicked(); }

    /* ---- drivers, map, network (saved in the state folder) -------------------------------------------------- */

    public int enableDriver(Link link, String uri) { return nativeEnableDriver(link.value, uri); }
    public int disableDriver(Link link) { return nativeDisableDriver(link.value); }
    public int map(long din, Link link, String uri) { return nativeMap(din, link.value, uri, ""); }
    public int map(long din, Link link, String uri, String securityKey) { return nativeMap(din, link.value, uri, securityKey); }
    public int unmap(long din) { return nativeUnmap(din); }
    /** Discovery beacons on {@code link} (NONE: every link), without leaving the current network. */
    public int discovery(Link link) { return nativeDiscoveryLink(link.value); }
    /** Discovery of the network {@code sid}. */
    public int discovery(long sid) { return nativeDiscoverySid(sid); }
    /** Joins a network (non blocking: completion is onNodeConnectedToNetwork, or state() MEMBER). */
    public int join(long sid, Link link, int timeoutMs) { return nativeJoin(sid, link.value, timeoutMs); }
    public int createNetwork() { return nativeCreateNetwork(); }
    public int unbindNetwork() { return nativeUnbindNetwork(); }
    public int locate(long din, int timeoutMs) { return nativeLocate(din, timeoutMs); }
    public NodeState state() { return NodeState.of(nativeState()); }
    public long sid() { return nativeSid(); }
    public long din() { return nativeDin(); }
    public long[] knownNodes() { return nativeKnownNodes(); }
    /** The nodes of {@code sid} known to the network (asks the network, waits up to the fetch timeout). */
    public long[] allNodes(long sid) { return nativeAllNodes(sid); }

    /* ---- policies ------------------------------------------------------------------------------------------ */

    public void setAcceptRequests(AcceptRequests policy) { nativeSetAcceptRequests(policy.value); }
    public void setDiscoveryState(DiscoveryState state) { nativeSetDiscoveryState(state.value); }
    public int setDdoPolicy(DdoPolicy policy) { return nativeSetDdoPolicy(policy.value); }
    public void setAtsMaxError(int milliseconds) { nativeSetAtsMaxError(milliseconds); }
    public int setOption(int option, int value) { return nativeSetOption(option, value); }

    /* ---- data ---------------------------------------------------------------------------------------------- */

    public int push(long din, int typeset, byte[] payload) { return nativePush(din, typeset, payload); }
    /** The next DDO received from {@code din}, null if none. */
    public Ddo pull(long din) { return nativePull(din); }
    public int available(long din) { return nativeAvailable(din); }

    /* ---- information --------------------------------------------------------------------------------------- */

    public NodeStatus status() { return nativeStatus(); }
    public String version() { return nativeVersion(); }
    /** One versioning system (version_system_t): 0 all, 1 library, 2 protocol, 3 build. */
    public String version(int system) { return nativeVersionOf(system); }
    public String buildInfo() { return nativeBuildInfo(); }
    public String availableDrivers() { return nativeAvailableDrivers(); }
    public String errorString(int error) { return nativeErrorString(error); }
    public String hardware() { return nativeHardware(); }

    /* ---- events (called by the native side, on the service thread) ----------------------------------------- */

    /** EventType of android_events.hpp, in its order. */
    private static final int DIN_ACCEPTED = 0, DDO_RECEIVED = 1, FRISBEE_RECEIVED = 2, NODE_STATE_RECEIVED = 3,
            ATS_SYNC_COMPLETED = 4, FRISBEE_DPERF_COMPLETED = 5, NETWORK_DISCOVERED = 6,
            NODE_CONNECTED_TO_NETWORK = 7, STREAM_INFO_RECEIVED = 8, ATTACK_PANIC = 9;

    @SuppressWarnings("unused")
    private static void dispatch(int type, long din, long sid, int a, int b, int c) {
        final DaasListener l = listener;
        if (l == null) return;
        try {
            switch (type) {
                case DIN_ACCEPTED: l.onDinAccepted(din); break;
                case DDO_RECEIVED: l.onDdoReceived(din, a, b); break;
                case FRISBEE_RECEIVED: l.onFrisbeeReceived(din); break;
                case NODE_STATE_RECEIVED: l.onNodeStateReceived(din); break;
                case ATS_SYNC_COMPLETED: l.onAtsSyncCompleted(din); break;
                case FRISBEE_DPERF_COMPLETED: l.onFrisbeeDperfCompleted(din, a, b); break;
                case NETWORK_DISCOVERED: l.onNetworkDiscovered(din, sid, Link.of(a)); break;
                case NODE_CONNECTED_TO_NETWORK: l.onNodeConnectedToNetwork(sid, din); break;
                case STREAM_INFO_RECEIVED: l.onStreamInfoReceived(din, a, b); break;
                case ATTACK_PANIC: l.onAttackPanic(din, sid); break;
                default: break;
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "listener of event " + type + " failed", e);
        }
    }

    private static int[] toInts(List<Integer> list) {
        int[] out = new int[list.size()];
        for (int i = 0; i < out.length; i++) out[i] = list.get(i);
        return out;
    }

    private static long[] toLongs(List<Long> list) {
        long[] out = new long[list.size()];
        for (int i = 0; i < out.length; i++) out[i] = list.get(i);
        return out;
    }

    /* ---- native (jni/android_daas_jni.cpp) ----------------------------------------------------------------- */

    private static native void nativeRedirectLogs();
    private static native String nativeWrapperVersion();
    private static native int nativeCreate(String stateDir, long sid, long din, int[] driverLinks, String[] driverUris,
            int[] driverPorts, long[] mapDins, int[] mapLinks, String[] mapUris, String[] mapKeys, int[] optionKeys,
            int[] optionValues, int[] policies, long joinSid, int joinLink, int joinTimeout, int atsMaxError,
            String hardware);
    private static native boolean nativeRestored();
    private static native int nativeStart();
    private static native int nativeService(int waitMs);
    private static native int nativeStop();
    private static native void nativeDestroy();
    private static native int nativeReset();
    private static native boolean nativeSave();
    private static native boolean nativePanicked();
    private static native int nativeEnableDriver(int link, String uri);
    private static native int nativeDisableDriver(int link);
    private static native int nativeMap(long din, int link, String uri, String key);
    private static native int nativeUnmap(long din);
    private static native int nativeDiscoveryLink(int link);
    private static native int nativeDiscoverySid(long sid);
    private static native int nativeJoin(long sid, int link, int timeout);
    private static native int nativeCreateNetwork();
    private static native int nativeUnbindNetwork();
    private static native int nativeLocate(long din, int timeout);
    private static native int nativeState();
    private static native long nativeSid();
    private static native long nativeDin();
    private static native long[] nativeKnownNodes();
    private static native long[] nativeAllNodes(long sid);
    private static native void nativeSetAcceptRequests(int policy);
    private static native void nativeSetDiscoveryState(int state);
    private static native int nativeSetDdoPolicy(int policy);
    private static native void nativeSetAtsMaxError(int error);
    private static native int nativeSetOption(int option, int value);
    private static native int nativePush(long din, int typeset, byte[] payload);
    private static native Ddo nativePull(long din);
    private static native int nativeAvailable(long din);
    private static native NodeStatus nativeStatus();
    private static native String nativeVersion();
    private static native String nativeVersionOf(int system);
    private static native String nativeBuildInfo();
    private static native String nativeAvailableDrivers();
    private static native String nativeErrorString(int error);
    private static native String nativeHardware();
}
