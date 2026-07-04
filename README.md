# Daas IoT Android SDK

A Kotlin/JNI wrapper around the native **daas** (`libdaas`) C++ core library, letting Android
apps join and communicate over Daas IoT networks: mapping nodes, exchanging data objects (DDOs),
opening real-time sessions, and negotiating node features — through a typed, idiomatic Kotlin API
instead of raw JNI calls.

## Features

- **Instance-based**: each `DaasWrapper.create()` (or [`DaasInstance`](#instance-lifecycle)) call
  owns its own independent native node; multiple instances can coexist in the same process.
- **Typed API**: every native `daas_error_t`/enum/struct is exposed as a real Kotlin type
  (`DaasError`, `LinkType`, `NodeInfo`, `PullResult`, …) instead of raw `Int`/`Long` codes or
  native pointers.
- **Node discovery & networking**: `discovery`, `join`, `createNetwork`/`unbindNetwork`, node
  mapping and lookup (`map`, `locate`, `listNodes`, `getNodeList`).
- **Data exchange**: push/pull typed data objects (`DDO`), real-time sessions (`use`/`send`/`receive`),
  and an event listener (`IDaasApiEvent`) for asynchronous node/DDO notifications.
- **Feature negotiation**: advertise and query node capabilities (`addNodeFeatures`,
  `nodeHasFeature`, `getNodeFeatures`).
- **Safe by default**: consumer ProGuard/R8 rules are bundled in the AAR, so the JNI layer keeps
  working in minified release builds without any setup on your side (see [ProGuard / R8](#proguard--r8)).

## Requirements

- Android API level 30+
- Kotlin 2.2+
- `arm64-v8a` only (see [Native library](#native-library) below)

## Installation

1. Copy the AAR into your app module's `libs/` folder.
2. Add the dependency in your app's `build.gradle.kts`:

```kotlin
dependencies {
    implementation(files("libs/daasiotsdk-0.2.0-release.aar"))
}
```

> This SDK isn't published to a Maven repository today - see [RELEASING.md](RELEASING.md) for how
> a new AAR is built and versioned.

## Quick start

```kotlin
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.DaasError

fun main() {
    val ptr = DaasWrapper.create()
    try {
        println("Daas version: ${DaasWrapper.getVersion(ptr)}")

        val result = DaasWrapper.doInit(ptr, sid = 1, din = 100)
        if (result != DaasError.ERROR_NONE) {
            println("doInit failed: $result")
        }
    } finally {
        DaasWrapper.doEnd(ptr)
        DaasWrapper.destroy(ptr)
    }
}
```

Prefer [`DaasInstance`](#instance-lifecycle) over manual `create`/`destroy` pairs where you can -
it's a drop-in `Closeable` wrapper around the same `ptr` with a garbage-collection safety net.

## Package structure

| Package | Contents |
|---|---|
| `sebyone.daasiot_android` | `DaasWrapper` (the JNI-backed API) and `DaasInstance` (the `Closeable` convenience wrapper) |
| `sebyone.daasiot_android.entity` | Data types: `DDO`, `NodeInfo`, `NodeNetworkInfo`, `NodeMapEntry`, `PullResult`, `AvailablePullResult`, `FeatureRequest` |
| `sebyone.daasiot_android.enums` | Every typed enum: `DaasError`, `LinkType`, `PerformMode`, `AcceptRequestPolicy`, `DDOPolicy`, `DiscoveryState`, `StreamType`, `Feature`, `FeatureAction`, `OptionType` |
| `sebyone.daasiot_android.events` | `IDaasApiEvent`, the listener interface for node/DDO events |
| `sebyone.daasiot_android.util` | Internal helpers (native memory safety net) - not part of the public API |

## Usage

Every `DaasWrapper` function takes the native handle (`ptr: Long` from `create()`, or
`DaasInstance.handle`) as its first argument.

### Instance lifecycle

```kotlin
import sebyone.daasiot_android.DaasInstance

DaasInstance.create().use { instance ->
    DaasWrapper.doInit(instance.handle, sid = 1, din = 100)
    // ...
} // DaasWrapper.destroy() is called automatically, even if an exception is thrown above
```

`DaasInstance` doesn't replace `DaasWrapper`'s functions - it just gives the `ptr` handle a real
object lifecycle (an explicit `close()`, plus a phantom-reference safety net in case you forget),
since a bare `Long` passed between call sites has no identity for the garbage collector to track.

### Joining a network

```kotlin
import sebyone.daasiot_android.enums.LinkType

DaasWrapper.enableDriver(ptr, LinkType.LINK_INET4, "192.168.1.10:2020")
DaasWrapper.discovery(ptr)                 // or discovery(ptr, sid = ...) for a known network
DaasWrapper.join(ptr)                      // blocking; joins the first network discovered
```

### Mapping and locating nodes

```kotlin
DaasWrapper.map(ptr, din = 42, link = LinkType.LINK_INET4, uri = "192.168.1.20:2020")
DaasWrapper.locate(ptr, din = 42)           // blocking; only needed if din isn't mapped yet
val status = DaasWrapper.status(ptr, din = 42)
```

### Exchanging data (DDO)

```kotlin
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.enums.DaasError

// Push
DDO(typeset = 1).use { ddo ->
    ddo.setPayload(byteArrayOf(1, 2, 3))
    DaasWrapper.push(ptr, din = 42, ddo = ddo)
}

// Pull
val (error, ddo) = DaasWrapper.pull(ptr, din = 42)
if (error == DaasError.ERROR_NONE && ddo != null) {
    ddo.use { println("Received ${it.getPayload().size} bytes, typeset ${it.getTypeset()}") }
}
```

### Real-time sessions

```kotlin
if (DaasWrapper.use(ptr, din = 42)) {
    DaasWrapper.send(ptr, din = 42, data = byteArrayOf(1, 2, 3))
    if (DaasWrapper.received(ptr, din = 42) > 0) {
        val data = DaasWrapper.receive(ptr, din = 42)
    }
    DaasWrapper.end(ptr, din = 42)
}
```

### Handling events

Pass an `IDaasApiEvent` implementation to `create()` to receive node/DDO events on that specific
instance - the native layer forwards each event as-is, with no implicit side effects (e.g. it does
not auto-pull on `ddoReceived`; call `pull()` yourself if you need the payload).

Since the listener needs `ptr`/`instance.handle` to call back into `DaasWrapper`, but that handle
doesn't exist until `create()` returns, give the listener a settable property instead of trying to
capture it directly:

```kotlin
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.LinkType
import sebyone.daasiot_android.enums.StreamType
import sebyone.daasiot_android.events.IDaasApiEvent

class MyEventHandler : IDaasApiEvent {
    var ptr: Long = 0

    override fun ddoReceived(payloadSize: Int, typeset: Int, din: Long) {
        val (error, ddo) = DaasWrapper.pull(ptr, din)
        if (error == DaasError.ERROR_NONE && ddo != null) {
            ddo.use { println("Payload: ${it.getPayload().size} bytes") }
        }
    }

    // Override the other IDaasApiEvent methods as needed.
    override fun dinAccepted(din: Long) {}
    override fun frisbeeReceived(din: Long) {}
    override fun nodeStateReceived(din: Long) {}
    override fun atsSyncCompleted(din: Long) {}
    override fun frisbeeDperfCompleted(din: Long, packetsSent: Long, blockSize: Long) {}
    override fun networkDiscovered(din: Long, sid: Long, link: LinkType) {}
    override fun nodeConnectedToNetwork(sid: Long, din: Long) {}
    override fun streamInfoReceived(din: Long, packetType: StreamType, streamId: Long) {}
}

val handler = MyEventHandler()
val ptr = DaasWrapper.create(eventHandler = handler)
handler.ptr = ptr
```

### Node features

```kotlin
import sebyone.daasiot_android.enums.Feature

DaasWrapper.addNodeFeatures(ptr, Feature.STORAGE)
val supportsCompute = DaasWrapper.nodeHasFeature(ptr, din = 42, feature = Feature.COMPUTE)
```

> `requestFeature` always returns `DaasError.ERROR_NOT_IMPLEMENTED` today: it's declared in the
> native headers but not currently an exported symbol in the linked `libdaas.a`.

## ProGuard / R8

Consumer rules are bundled in the AAR automatically (`daasiotsdk/consumer-rules.pro`) - you don't
need to add anything to your app's own ProGuard configuration. They exist because the native layer
locates `DaasWrapper`, `DDO`, every entity/enum, and `IDaasApiEvent` by exact class/member name via
JNI reflection (`FindClass`/`GetMethodID`), which is invisible to R8's static analysis and would
otherwise be renamed or stripped in a minified release build.

## Native library

Only `arm64-v8a` is built today; `armeabi-v7a`/`x86_64` prebuilt libraries exist under `jniLibs/`
but are excluded from the Gradle/CMake configuration. See
[`daasiotsdk/src/main/jniLibs/PROVENANCE.md`](daasiotsdk/src/main/jniLibs/PROVENANCE.md) for where
`libdaas.a`/`libsimpleble.a` come from, their version status per ABI, and how to update them
(`scripts/update-native-lib.ps1`).

## Testing

- Unit tests (`daasiotsdk/src/test`) run on the host JVM and cover everything that doesn't need
  the native library: enum round-tripping, and `equals`/`hashCode` correctness for the data types
  that hold `ByteArray` fields.
- Instrumented tests (`daasiotsdk/src/androidTest`) exercise the real JNI layer on a device or
  emulator: instance creation/isolation, and the `DDO` payload/typeset lifecycle.

## Release process

See [RELEASING.md](RELEASING.md) for the versioning/tagging steps and
[CHANGELOG.md](CHANGELOG.md) for what changed in each release.

## License

This project is licensed under the Mozilla Public License 2.0 - see the [LICENSE](LICENSE) file for details.

### MPL-2.0 License Summary

- ✅ Commercial use
- ✅ Modification
- ✅ Distribution
- ✅ Patent use
- ✅ Private use
- ❗ License and copyright notice required
- ❗ Disclose source (for modified files)
- ❗ Same license (for modified files)
- ❌ Liability
- ❌ Warranty
- ❌ Trademark use
