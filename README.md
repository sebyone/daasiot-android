<!-- daas © 2019–2026 Sebyone Srl -->

# daasiot-android

daas (Device as a Service) for Android: **android_daas**, the wrapper of libdaas for Android apps (Java API
`it.sebyone.daas`, C++17 `daas::sdk::Node`), the **android-drivers_sdk** (BLE driver on blew) and the examples.
Version: `VERSION` (0.2.0), on libdaas **0.24.11** (daas protocol 6.1.0), pinned in `android_daas/libdaas.lock.json`.

It follows the layout of the other daas-iot wrappers (daasiot-gnu, daasiot-macos, daasiot-esp32): a component that
wraps libdaas, a `<platform>-drivers_sdk` with the specialized drivers, libdaas out of git and installed by
`tools/fetch_ld`, the examples.

## Layout

```
android_daas/                 the component: an Android library module (AAR)
  build.gradle.kts            com.android.library, namespace it.sebyone.daas, Maven publication
  java/it/sebyone/daas/       DaasNode (+ Config), DaasListener, Ddo, NodeStatus, Link, NodeState, policies, DaasError
  include/, src/              android_daas: daas::sdk::Node, EventRouter, file_depot (port of macos_daas)
  jni/android_daas_jni.cpp    the binding of Node for DaasNode (libandroid_daas_jni.so)
  CMakeLists.txt              daas::android_daas, android_daas_jni
  libdaas.lock.json           the libdaas packages (tarball, SHA-256) of the catalog of daasiot.com
android-drivers_sdk/          specialized drivers, built only with drivers_sdk/include of the package
  ble/                        BLE driver of the `ble` slot of _LINK_BLE (1.0.0)
  include/daasiot_android/    ble.h: daas_ble_set_local_sid()
  third_party/blew/           blew 1.1.0: libblew.a per ABI, headers, Java sources it.sebyone.blew
  libdaas-drivers.lock.json   the drivers of each <target>/<linkage>
platforms/libdaas/libdaas.cmake   imports a libdaas package as libdaas::libdaas (DAAS_TARGET, DAAS_ROOT, DAAS_DRIVERS,
                              DAAS_EXTRA_DRIVERS)
examples/simple_node/         an app with a daas node (BLE + INET4), android_daas as a dependency
tools/fetch_ld/               installs the libdaas packages of the lock into libdaas/ (gitignored)
docs/shared/                  developers guide
CMakeLists.txt                the native project (BLE driver, android_daas, android_daas_jni)
settings.gradle.kts, build.gradle.kts, gradlew   the Gradle build of the repository
```

## Build

Needs the Android SDK (API 36), the NDK r28c (28.2.13676358), CMake 3.22.1 of the SDK, JDK 21.

```bash
tools/fetch_ld/fetch_ld.sh                     # libdaas of the lock into libdaas/android-arm64-v8a/static/
./gradlew :android_daas:assembleRelease        # android_daas/build/outputs/aar/android_daas-release.aar
./gradlew :simple_node:assembleDebug           # the example app
./gradlew :android_daas:publishToMavenLocal    # it.sebyone.daas:android_daas:<VERSION> in ~/.m2
```

`local.properties` (gitignored) gives `sdk.dir`, as in any Android project. Another libdaas package (e.g. one built
from daas-api): unpack it into `libdaas/android-arm64-v8a/static/` or pass `-DDAAS_ROOT=<folder>` to CMake.

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
   `implementation(project(":android_daas"))` in the app. libdaas must be in `<path>/daasiot-android/libdaas/`.

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

Native code of the app can use `daas::sdk::Node` directly: `add_subdirectory(<daasiot-android> daasiot-android)` and
`target_link_libraries(<lib> PRIVATE daas::android_daas)`.

## libdaas is not in git

`libdaas/` is gitignored: the packages are pinned in `android_daas/libdaas.lock.json` and installed by
`tools/fetch_ld/fetch_ld.sh` (`fetch_ld.ps1` on Windows) from the catalog of daasiot.com. An app that vendors
daasiot-android (a copy inside its repository) keeps the package it builds with inside that copy.

## License

MPL-2.0 (`LICENSE`). Third-party components: `THIRD-PARTY-NOTICES.md`.
