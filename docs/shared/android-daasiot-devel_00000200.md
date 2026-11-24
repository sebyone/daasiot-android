<!-- daas © 2019–2026 Sebyone Srl -->
<!-- Contributors: l.grillo@sebyone.it -->

<p><img src="assets/logo-daasiot.png" alt="daas logo" height="56"></p>

**daas — Device as a Service** · daas-iot · daasiot-android · official site of the daas project: [daasiot.com](https://daasiot.com)

# daasiot-android developers guide

*How to build daas nodes in Android apps with android_daas, libdaas and the BLE driver on blew.*

| Data sheet | |
|---|---|
| **Document code** | `android-daasiot-devel_00000200` |
| **Type** | Guide |
| **Document version** | 1.1.0 |
| **Project version** | daasiot-android 0.2.0 (libdaas 0.24.12, daas protocol 6.1.0) |
| **Applies to** | Android API 30 or later · arm64-v8a, armeabi-v7a, x86_64 · Java 11 · NDK r28 |
| **Status** | Draft |
| **Date** | 24/11/2026 |
| **Author** | Lorenzo Grillo — l.grillo@sebyone.it |
| **Project owner** | Sebyone Srl — [sebyone.it](https://sebyone.it) |
| **Official site** | [daasiot.com](https://daasiot.com) |
| **Language** | English |
| **License** | MPL-2.0 |
| **Source** | `docs/shared/android-daasiot-devel_00000200.md` |

> **Summary.** For developers who build daas nodes in Android apps. It explains what daasiot-android contains, how
> to add the AAR or the module to an app, how a node is configured, started and stopped with `DaasNode`, how data
> objects are exchanged, how the state of a node is kept across restarts, the BLE driver on blew, the native API and
> how the packages are built.

> **Project owner.** daasiot-android is a project of **Sebyone Srl**, the copyright holder (© 2019–2026 Sebyone Srl),
> distributed under the Mozilla Public License 2.0. The official site of the daas project is
> [daasiot.com](https://daasiot.com); contacts: [sebyone.it/contatti](https://sebyone.it/contatti.html).

---

## 1. What it is

| Part | Where | Role |
|---|---|---|
| libdaas | `android_daas/libdaas/android-<ABI>/static/` | the daas library of Sebyone, static packages of the catalog of daasiot.com (`tools/fetch_ld`, not in git) |
| drivers_kit | `android_daas/drivers_kit/android-<ABI>/static/` | the BLE driver of the `ble` slot compiled (`libdaas_drv_ble.a`), blew precompiled by the DaaS Build Console (`libblew.a`, the Java side `it.sebyone.blew` in `java/`), `DRIVERS.txt` (not in git) |
| android_daas (C++) | `android_daas/include`, `src` | `Node`: configuration, events, persistence; global API, like libdaas |
| android_daas (Java) | `android_daas/java` | `it.sebyone.daas.DaasNode`: the API of the app |
| android-drivers-sdk | `android-drivers-sdk/` | the specialized drivers and the kit to build them (`build_drivers_kit.py`, `template/`) |

An app sees only the Java API; the native parts are one shared library per ABI, `libandroid_daas_jni.so`, inside the
AAR.

## 2. Requirements

- Android API 30 or later; ABIs arm64-v8a, armeabi-v7a, x86_64 (the ABIs of the libdaas lock and of blew).
- The AAR of the packages: nothing else. Building the repository: Android SDK 36, NDK r28c, CMake 3.22, JDK 21 for
  Gradle (AGP 9.0); or, without Gradle, the NDK, a JDK and `android.jar` (`platforms/scripts/build_aar.py`).

## 3. libdaas and the drivers_kit

```bash
tools/fetch_ld/fetch_ld.sh                          # the packages of android_daas/libdaas.lock.json
tools/fetch_ld/fetch_ld.sh --check                  # what is installed
python3 android-drivers-sdk/build_drivers_kit.py    # the drivers_kit of every ABI (ANDROID_NDK_HOME; blew from the
                                                    # DaaS Build Console, DAAS_BUILDER_URL)
```

Another libdaas package: give CMake `-DDAAS_ROOT=<folder>` (Gradle: `externalNativeBuild.cmake.arguments` of the
module). Without the drivers_kit the project builds without the BLE driver (`DAASIOT_ANDROID_BLE` OFF); the Java side
of blew comes from the kit too, so the AAR needs it.

## 4. Adding android_daas to an app

- **AAR** (the package `daasiot-android-<v>-aar`): `implementation(files("libs/android_daas-<v>-release.aar"))`. From
  the repository: `./gradlew :android_daas:assembleRelease`, or `python3 platforms/scripts/build_aar.py`.
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
- **BLE**: `create()` initialises blew with the application context and hands the local SID to the BLE driver
  (`daas_ble_set_local_sid()`, a weak default in android_daas replaced by the driver of the kit); the SID is handed
  again after a restore and when a join completes (`onNodeConnectedToNetwork`). The driver (1.1.0) reuses a link
  already open with a node; its timings of blew are in `android-drivers-sdk/ble/ble_config.hpp`.
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

`android_daas.hpp` is the only include: `Node`, `NodeConfig`, `EventRouter`, `file_depot` are global, like the API of
libdaas, with the same API as in daasiot-gnu and daasiot-macos (without the JSON configuration and the service helpers
of the hosts). A native library of the app adds the CMake project of daasiot-android with `add_subdirectory()` and
links `daas::android_daas`. The defaults (queues, options of libdaas, persistence, events) are the `ANDROID_DAAS_*`
macros of `android_defaults.hpp`, overridable with `-DANDROID_DAAS_USER_CONFIG=<header>`.

## 7. Drivers of your own

A driver built with `DAAS_ROOT/drivers_sdk/include` and ending with `DAAS_DRIVER(<id>, ...)` fills the slot `<id>`
when its archive is listed in `DAAS_EXTRA_DRIVERS`: `android_daas/libdaas.cmake` links it after `libdaas.a` and
before the drivers of the package and `libdaas_nodrivers.a`. Start from `android-drivers-sdk/template/` (slot `uartx`)
and build it with `android-drivers-sdk/CMakeLists.txt` and the NDK. The packages carry the specialized drivers
compiled only: the sources of the BLE driver stay in the repository.

## 8. Packages

`platforms/scripts/build_dist.py` (from `platforms/dist.json`), built by the DaaS Build Console
(`platforms/daas_builder.sh`) on its Debian VM with the NDK, a JDK and `android.jar`, without Gradle:

| Package | Content |
|---|---|
| `daasiot-android-<v>-aar.tar.gz` | `android_daas-<v>-release.aar` (every ABI), the example, the guides |
| `daasiot-android-<v>-drivers-sdk-static.tar.gz` | the drivers_kit of every ABI, compiled; the template and the CMake files of a new driver |
| `daasiot-android-<v>-src.tar.gz` | the sources (without the sources of the specialized drivers and the files for developers) |

Every package is one folder with `MANIFEST.json` and `SHA256SUMS`, with `<package>.tar.gz.sha256` next to it.

## 9. Known limits

- The BLE driver 1.1.0 was run between a phone and an ESP32 (FanUp); the AAR built without Gradle is not yet tried in
  an app.
- One node per process (libdaas).

## 10. History

| Version | Date | Changes |
|---|---|---|
| 1.0.0 | 23/11/2026 | Creation (daasiot-android 0.2.0, libdaas 0.24.11). |
| 1.1.0 | 24/11/2026 | Conventions of the daasiot-&lt;target&gt; repositories: global API (no namespace); libdaas in `android_daas/libdaas/` for three ABIs; the BLE driver from the drivers_kit with the precompiled blew of the DaaS Build Console; the AAR without Gradle; libdaas 0.24.12. |
