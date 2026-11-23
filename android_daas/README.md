<!-- daas © 2019–2026 Sebyone Srl -->

# android_daas

The component of daasiot-android: an Android library module (`it.sebyone.daas`) that packages libdaas, its drivers
and blew in `libandroid_daas_jni.so`, with a Java API and a C++17 wrapper.

| Layer | Files | What |
|---|---|---|
| Java API | `java/it/sebyone/daas/` | `DaasNode` (+ `Config`), `DaasListener`, `Ddo`, `NodeStatus`, `Link`, `NodeState`, `AcceptRequests`, `DiscoveryState`, `DdoPolicy`, `DaasError` |
| JNI | `jni/android_daas_jni.cpp` | the binding of `Node` for `DaasNode`: one node per process, events to `DaasNode.dispatch()` |
| C++ | `include/android_daas.hpp`, `src/` | `daas::sdk::Node`, `EventRouter`, `daas::persistence::file_depot`, defaults (port of `macos_daas` of daasiot-macos) |
| libdaas | `../libdaas/<target>/static/` | pinned in `libdaas.lock.json`, installed by `tools/fetch_ld` |

`DaasNode` keeps its configuration in `filesDir/daas/` (one file per record, written atomically): the first `create()`
applies the `Config`, the next ones restore the saved node. Its service thread (`daas-service`) runs
`Node::service()`: the events of libdaas reach the `DaasListener` there, and the pending saves are made.

Build: `./gradlew :android_daas:assembleRelease` from the root of the repository (README.md of the repository).
