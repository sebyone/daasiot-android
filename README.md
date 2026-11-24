<!-- daas © 2019–2026 Sebyone Srl -->

# daasiot-android

daas (Device as a Service) for Android: **android_daas**, the wrapper of libdaas for Android apps (Java API
`it.sebyone.daas`, C++17 `Node`, global like libdaas), the **android-drivers-sdk** (BLE driver on blew) and the
examples. Version: `VERSION` (0.2.0), on libdaas **0.24.12** (daas protocol 6.1.0), pinned in `android_daas/libdaas.lock.json`.

It follows the conventions of the daasiot-&lt;target&gt; repositories (daasiot-gnu, daasiot-macos, daasiot-esp32): a
component that wraps libdaas, a `<platform>-drivers-sdk` with the specialized drivers built into a drivers_kit, libdaas
out of git and installed by `tools/fetch_ld` from the catalog, blew precompiled by the DaaS Build Console.

## Layout

| Path | Content |
|---|---|
| `android_daas/` | the component, an Android library module (AAR): `build.gradle.kts`, `java/it/sebyone/daas/` (`DaasNode`, ...), `include/`, `src/` (`Node`, `EventRouter`, `file_depot`), `jni/android_daas_jni.cpp` (`libandroid_daas_jni.so`), `CMakeLists.txt`, `libdaas.cmake` (libdaas and the drivers_kit), `libdaas.lock.json`; `libdaas/android-<ABI>/static/` (fetch_ld) and `drivers_kit/android-<ABI>/static/` (not in git) |
| `android-drivers-sdk/` | `ble/` (BLE driver 1.1.0, slot `ble`), `template/` (slot `uartx`), `CMakeLists.txt`, `libdaas_package.cmake`, `libdaas-drivers.lock.json` (blew paths and SHA-256), `build_drivers_kit.py` |
| `examples/simple_node/` | an app with a daas node (BLE + INET4) |
| `platforms/` | `dist.json`, `scripts/build_aar.py` (the AAR without Gradle), `scripts/build_dist.py` (packages), `daas_builder.sh` (DaaS Build Console, `daas-build.yaml`) |
| `tools/` | `fetch_ld/` (2.0.1), `docs_pdf/` |
| `docs/shared/` | the developers guide (`_<MMMmmmpp>`), its PDF in `latest/` |

## Build

```bash
tools/fetch_ld/fetch_ld.sh                          # libdaas of the lock into android_daas/libdaas/ (three ABIs)
python3 android-drivers-sdk/build_drivers_kit.py    # the drivers_kit (ANDROID_NDK_HOME; blew: DAAS_BUILDER_URL)
./gradlew :android_daas:assembleRelease             # the AAR with Gradle (Android SDK 36, NDK r28c, JDK 21)
python3 platforms/scripts/build_aar.py              # or without Gradle: NDK, JDK, android.jar -> out/aar/
./gradlew :simple_node:assembleDebug                # the example app
```

`local.properties` (gitignored) gives `sdk.dir`, as in any Android project. Another libdaas package: pass
`-DDAAS_ROOT=<folder>` to CMake.

## Use in an app

Two ways, the same module:

1. **AAR**: `implementation(files("libs/android_daas-release.aar"))`, or from Maven
   (`implementation("it.sebyone.daas:android_daas:0.2.0")` after `publishToMavenLocal`). The app needs no native code.
2. **Module of the build** (developing the app and the wrapper together), in `settings.gradle.kts` of the app:

   ```kotlin
   include(":android_daas")
   project(":android_daas").projectDir = file("<path>/daasiot-android/android_daas")
   ```

   with `id("com.android.library") version "9.0.0" apply false` in the root `build.gradle.kts` and
   `implementation(project(":android_daas"))` in the app. libdaas and the drivers_kit must be in
   `<path>/daasiot-android/android_daas/`.

Then:

```java
DaasNode.Config config = new DaasNode.Config(100, 101)
        .driver(Link.BLE, "00000065")                 // blew node id: 8 hexadecimal digits
        .driver(Link.INET4, "", 2101)                 // every IPv4 address, port 2101
        .acceptRequests(AcceptRequests.ALL);
DaasNode node = DaasNode.create(context, config);     // restored from filesDir/daas from the second start
node.setListener(new DaasListener() {
    @Override public void onDdoReceived(long din, int typeset, int size) {
        for (Ddo ddo; (ddo = node.pull(din)) != null; ) handle(ddo);
    }
});
node.start();
```

- One node per process (libdaas has one): `DaasNode.create()` again after `close()`.
- The configuration is saved in `filesDir/daas/` (`file_depot`) at every change made through the node (map,
  drivers, join completed) and at `stop()`; the next `create()` restores it and applies only the policies of the
  Config.
- The events arrive on the service thread of the node (`daas-service`), where the whole API can be called.
- BLE: the app asks the runtime permissions (`BLUETOOTH_SCAN`, `BLUETOOTH_CONNECT`, `BLUETOOTH_ADVERTISE`) before
  `create()`; the manifest of android_daas declares them. `DaasNode.create()` initialises blew and hands the local
  SID to the BLE driver (also after a restore or a join).
- `DaasNode.redirectLogsToLogcat()`: the logs of libdaas in logcat (tag `libdaas`).

Native code of the app can use `Node` directly: `add_subdirectory(<daasiot-android> daasiot-android)` and
`target_link_libraries(<lib> PRIVATE daas::android_daas)`.

## libdaas and blew are not in git

`android_daas/libdaas/` and `android_daas/drivers_kit/` are gitignored: libdaas is pinned in
`android_daas/libdaas.lock.json` and installed by `tools/fetch_ld/fetch_ld.sh` (`fetch_ld.ps1` on Windows) from the
catalog of daasiot.com; blew comes precompiled from the DaaS Build Console, checked against the SHA-256 of
`android-drivers-sdk/libdaas-drivers.lock.json`. The packages carry the specialized drivers compiled only.

## License

MPL-2.0 (`LICENSE`). Third-party components: `THIRD-PARTY-NOTICES.md`.
