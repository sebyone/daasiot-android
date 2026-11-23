<!-- daas © 2019–2026 Sebyone Srl -->

# daasiot-android 0.2.0 — developers guide

daas (Device as a Service) for Android apps: android_daas on libdaas 0.24.11 (daas protocol 6.1.0).

## 1. What it is

| Part | Where | Role |
|---|---|---|
| libdaas | `libdaas/android-<ABI>/static/` | the daas library of Sebyone, static package of the catalog of daasiot.com |
| android-drivers_sdk | `android-drivers_sdk/` | the BLE driver of the `ble` slot, on blew |
| android_daas (C++) | `android_daas/include`, `src` | `daas::sdk::Node`: configuration, events, persistence |
| android_daas (Java) | `android_daas/java` | `it.sebyone.daas.DaasNode`: the API of the app |

An app sees only the Java API; the native parts are one shared library, `libandroid_daas_jni.so`, inside the AAR.

## 2. Requirements

- Android API 30 or later, arm64-v8a (the ABI of the libdaas package and of blew).
- Build: Android SDK 36, NDK r28c (28.2.13676358), CMake 3.22.1 of the SDK, AGP 9.0, Gradle 9.4.1, JDK 21.

## 3. Getting libdaas

```bash
tools/fetch_ld/fetch_ld.sh            # the packages of android_daas/libdaas.lock.json
tools/fetch_ld/fetch_ld.sh --check    # what is installed
```

Another package (e.g. built from daas-api): unpack it into `libdaas/android-arm64-v8a/static/`, or give CMake
`-DDAAS_ROOT=<folder>` (Gradle: `externalNativeBuild.cmake.arguments` of the module).

## 4. Adding android_daas to an app

- **AAR**: `./gradlew :android_daas:assembleRelease`, then `implementation(files("libs/android_daas-release.aar"))`;
  or `./gradlew :android_daas:publishToMavenLocal` and `implementation("it.sebyone.daas:android_daas:0.2.0")` with
  `mavenLocal()` in the repositories.
- **Module**: in `settings.gradle.kts` of the app
  `include(":android_daas"); project(":android_daas").projectDir = file("<daasiot-android>/android_daas")`, the
  plugin `com.android.library` 9.0.0 declared in the root build, `implementation(project(":android_daas"))`.

The manifest of android_daas adds the permissions of a node (INTERNET, ACCESS_NETWORK_STATE, the Bluetooth ones);
the app asks the runtime ones (`BLUETOOTH_SCAN`, `BLUETOOTH_CONNECT`, `BLUETOOTH_ADVERTISE`) before creating the node.

## 5. The node

```java
DaasNode.Config config = new DaasNode.Config(SID, DIN)
        .driver(Link.BLE, String.format("%08X", DIN))   // blew node id
        .driver(Link.INET4, "", 2101)
        .map(CLOUD_DIN, Link.INET4, "158.220.97.43:3030")
        .acceptRequests(AcceptRequests.ALL)
        .discoveryState(DiscoveryState.FULL)
        .atsMaxError(1000);
DaasNode node = DaasNode.create(context, config);
node.setListener(listener);
node.start();
```

- **One node per process**: libdaas has one. `create()` throws if a node exists; `close()` frees it.
- **Persistence**: the node lives in `filesDir/daas/` (`file_depot`, one record per key, written atomically). The
  first `create()` applies the Config and saves it; the next ones restore SID, DIN, drivers and map
  (`restored()` is true) and apply only the policies and options of the Config. Every change made through the node
  (`map`, `unmap`, `enableDriver`, `disableDriver`, a join completed) is saved; `stop()` saves the queues too.
- **Drivers**: `Link.BLE` with the blew node id (8 hexadecimal digits), `Link.INET4` / `Link.INET6` with
  `"ip:port"` or `""` and a port (every address). After a restore, a driver whose address changed (e.g. the IPv4
  address of the Wi-Fi) is enabled again with `enableDriver()`.
- **BLE**: `create()` initialises blew with the application context and hands the local SID to the BLE driver; the
  SID is handed again after a restore and when a join completes (`onNodeConnectedToNetwork`).
- **Events**: on the service thread `daas-service`, where the whole API can be called. `onDdoReceived(din, ...)`:
  `pull(din)` until null. Post to the main thread what touches the UI.
- **Network**: `discovery(Link)` (beacons on a link, the current network stays), `discovery(sid)`, `join(sid, link,
  timeout)` (non blocking: `onNodeConnectedToNetwork` or `state() == MEMBER`), `unbindNetwork()`,
  `allNodes(sid)`, `knownNodes()`, `locate(din, timeout)`.
- **Data**: `push(din, typeset, bytes)`, `pull(din)` → `Ddo` (`origin`, `typeset`, `timestamp`, `payload`,
  `buffer()` little endian).
- **Attack panic**: `onAttackPanic` / `panicked()`: the node stays isolated until the process of the app restarts.
- **Logs**: `DaasNode.redirectLogsToLogcat()` (tag `libdaas`); android_daas logs with the tags `DaasNode` and
  `android_daas`.

## 6. Native code

`android_daas.hpp` is the only include: `daas::sdk::Node` has the same API as in daasiot-gnu / daasiot-macos (without
the JSON configuration and the service helpers of the hosts). A native library of the app adds the CMake project of
daasiot-android with `add_subdirectory()` and links `daas::android_daas`. The defaults (queues, options of libdaas,
persistence, events) are the `ANDROID_DAAS_*` macros of `android_defaults.hpp`, overridable with
`-DANDROID_DAAS_USER_CONFIG=<header>`.

## 7. Drivers of your own

A driver built with `DAAS_ROOT/drivers_sdk/include` and ending with `DAAS_DRIVER(<id>, ...)` fills the slot `<id>`
when it is listed in `DAAS_EXTRA_DRIVERS` (a CMake target, or the path of an archive): `libdaas.cmake` links it after
`libdaas.a` and before `libdaas_nodrivers.a`. Do not link it elsewhere: CMake would move it after the nodriver of its
slot. The BLE driver is the example (`android-drivers_sdk/`).
