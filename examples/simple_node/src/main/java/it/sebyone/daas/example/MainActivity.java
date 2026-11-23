/*
 * daas © 2019–2026 Sebyone Srl
 *
 * File: MainActivity.java — examples/simple_node (daasiot-android)
 *
 * A daas node in an Android app: SID 100, DIN 101, BLE (node id from the DIN) and INET4 on port 2101, every request
 * accepted, full discovery. It shows the events and the DDOs received, and answers every text DDO with "ack".
 * The configuration is saved in the files folder of the app: the next start restores it.
 *
 * Contributors:
 *   Lorenzo Grillo <l.grillo@sebyone.it>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0 (this example is a starting
 * point: license your own project as you wish).
 *
 * Changes:
 *   2026-11-23  Lorenzo Grillo  creation
 */
package it.sebyone.daas.example;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.ScrollView;
import android.widget.TextView;

import java.nio.charset.StandardCharsets;

import it.sebyone.daas.AcceptRequests;
import it.sebyone.daas.DaasListener;
import it.sebyone.daas.DaasNode;
import it.sebyone.daas.Ddo;
import it.sebyone.daas.DiscoveryState;
import it.sebyone.daas.Link;

public class MainActivity extends Activity {

    private static final long SID = 100, DIN = 101;
    private static final int TYPESET_TEXT = 1;
    private static final String[] PERMISSIONS = {
            Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_ADVERTISE};

    private TextView log;
    private DaasNode node;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        log = new TextView(this);
        ScrollView scroll = new ScrollView(this);
        scroll.addView(log);
        setContentView(scroll);
        if (missingPermissions()) requestPermissions(PERMISSIONS, 1);
        else startNode();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (missingPermissions()) show("Bluetooth permissions denied: BLE disabled");
        startNode();
    }

    private boolean missingPermissions() {
        for (String p : PERMISSIONS) if (checkSelfPermission(p) != PackageManager.PERMISSION_GRANTED) return true;
        return false;
    }

    private void startNode() {
        if (node != null) return;
        DaasNode.redirectLogsToLogcat();
        DaasNode.Config config = new DaasNode.Config(SID, DIN)
                .driver(Link.BLE, String.format("%08X", DIN))     // blew node id: 8 hexadecimal digits of the DIN
                .driver(Link.INET4, "", 2101)                    // every IPv4 address, port 2101
                .acceptRequests(AcceptRequests.ALL)
                .discoveryState(DiscoveryState.FULL);
        node = DaasNode.create(this, config);
        show("android_daas " + DaasNode.wrapperVersion() + ", " + node.version());
        show((node.restored() ? "restored" : "configured") + ": " + node.errorString(node.initializationResult()));
        node.setListener(new DaasListener() {
            @Override public void onDinAccepted(long din) { show("DIN accepted: " + din); }
            @Override public void onNetworkDiscovered(long din, long sid, Link link) { show("network " + sid + " via " + din + " (" + link + ")"); }
            @Override public void onNodeConnectedToNetwork(long sid, long din) { show("member of " + sid + " as " + din); }
            @Override public void onDdoReceived(long din, int typeset, int size) {
                for (Ddo ddo; (ddo = node.pull(din)) != null; ) {
                    show("DDO " + ddo.typeset + " from " + ddo.origin + ": " + ddo.text());
                    if (ddo.typeset == TYPESET_TEXT) node.push(ddo.origin, TYPESET_TEXT, "ack".getBytes(StandardCharsets.UTF_8));
                }
            }
            @Override public void onAttackPanic(long origin, long from) { show("attack panic: restart the app"); }
        });
        show("start: " + node.errorString(node.start()) + ", drivers " + node.availableDrivers());
    }

    private void show(String line) {
        runOnUiThread(() -> log.append(line + "\n"));
    }

    @Override
    protected void onDestroy() {
        if (node != null && isFinishing()) node.close();
        super.onDestroy();
    }
}
