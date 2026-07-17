# daasiot-android — API Reference

Kotlin/JNI wrapper around the native **daas** (`libdaas`) C++ core, letting Android
apps join DaaS-IoT networks: mapping nodes, exchanging DDOs, opening real-time
sessions and negotiating features — through a typed, idiomatic Kotlin API.

| | |
|---|---|
| **Coordinates** | `it.sebyone:daasiot-android:0.2.0` |
| **Library (AAR)** | `daasiotsdk` (Gradle module / AAR filename) |
| **Package** | `sebyone.daasiot_android` |
| **DaaS-IoT core** | `v0.22.0` |
| **Android** | API level **30+** · Kotlin 2.2+ |
| **ABI** | `arm64-v8a` (see [`architectures.md`](architectures.md)) |
| **License** | Mozilla Public License 2.0 |

```kotlin
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.DaasInstance
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.enums.DaasError
import sebyone.daasiot_android.enums.LinkType
```

---

## Core concepts

| Term | Meaning |
|------|---------|
| **SID** | Service / network identifier shared by cooperating nodes |
| **DIN** (`Long`) | Device Identifier inside the Network (unique per node) |
| **DDO** | DaaS Data Object — the typed message unit exchanged between nodes |
| **Link** (`LinkType`) | Communication backend (IPv4, BLE, MQTT, UART, LoRa, …) |
| **Typeset** (`Int`) | 16-bit tag describing the semantic type of a DDO payload |

`DaasWrapper` is a stateless `object`: each **instance** is a native handle
(`ptr: Long`) returned by `create()` and passed as the first argument to every
other function. Multiple independent instances can coexist in one process.

---

## Instance lifecycle

Two equivalent styles:

```kotlin
// Manual handle
val ptr = DaasWrapper.create()
try {
    DaasWrapper.doInit(ptr, sid = 1, din = 100)
    // ...
} finally {
    DaasWrapper.doEnd(ptr)
    DaasWrapper.destroy(ptr)
}

// Closeable wrapper (recommended) — destroy() is automatic
DaasInstance.create().use { instance ->
    DaasWrapper.doInit(instance.handle, sid = 1, din = 100)
    // ...
}
```

| Function | Returns | Description |
|----------|---------|-------------|
| `create(eventHandler? , lhver?)` | `Long` | Create an independent native instance; returns its handle. |
| `destroy(ptr)` | `Unit` | Release the native instance (`ptr` unusable afterwards). |
| `DaasInstance.create(...)` | `DaasInstance` | `Closeable` wrapper around a handle, with a GC safety net. |

---

## `object DaasWrapper`

Every function below takes the instance `ptr: Long` as its first argument.

### Lifecycle & info

| Function | Returns | Description |
|----------|---------|-------------|
| `doInit(ptr, sid, din)` | `DaasError` | Initialize services and resources. |
| `doPerform(ptr, mode)` | `DaasError` | Run node task; call cyclically in `PERFORM_CORE_NO_THREAD`. |
| `doEnd(ptr)` | `DaasError` | Release resources, deactivate node (handle stays valid). |
| `reset(ptr)` | `DaasError` | Reset and disconnect from the current network. |
| `getVersion(ptr)` | `String` | DaaS-IoT version. |
| `getBuildInfo(ptr)` | `String` | Local stack build details. |
| `listAvailableDrivers(ptr)` | `String` | Available drivers, e.g. `2.INET4;3.UART`. |
| `errorToString(ptr, error)` | `String` | Human-readable message for a `DaasError`. |

### Drivers, discovery & networking

| Function | Returns | Description |
|----------|---------|-------------|
| `enableDriver(ptr, driver, localUri)` | `DaasError` | Enable a driver at a local address. |
| `discovery(ptr, link = LINK_NONE)` | `DaasError` | Discover networks over a link (all if unset). |
| `discovery(ptr, sid)` | `DaasError` | Discover a specific network. |
| `join(ptr, sid = 0, link = LINK_NONE, timeoutMillis = 5000)` | `DaasError` | Blocking; join a network. |
| `setDiscoveryState(ptr, mode)` | `Unit` | Configure discovery behaviour. |
| `createNetwork(ptr)` | `DaasError` | Create a fresh network. |
| `unbindNetwork(ptr)` | `DaasError` | Detach, keeping the node map for rebinding. |

### Mapping & nodes

| Function | Returns | Description |
|----------|---------|-------------|
| `map(ptr, din, link, uri? , skey?)` | `DaasError` | Map a remote node (optional URI / security key). |
| `remove(ptr, din)` | `DaasError` | Unmap a node. |
| `locate(ptr, din)` | `DaasError` | Blocking; locate a node by DIN. |
| `locate(ptr, feature, timeoutMillis = 1000, ttl = 10)` | `DaasError` | Blocking; locate a node by feature. |
| `listNodes(ptr)` | `Array<NodeNetworkInfo>` | Known nodes. |
| `getNodeList(ptr)` | `Array<NodeMapEntry>` | Mapped nodes with link/URI info. |
| `getAllNodes(ptr, sid)` | `LongArray` | DINs mapped under a SID. |

### Status & time

| Function | Returns | Description |
|----------|---------|-------------|
| `getStatus(ptr)` | `NodeInfo` | Local node status. |
| `status(ptr, din)` | `NodeInfo` | Cached status of a remote node. |
| `fetch(ptr, din, opts)` | `NodeInfo` | Query and refresh a remote node status. |
| `sendStatus(ptr, din)` | `DaasError` | Push local status to a remote node. |
| `getSyncedTimestamp(ptr)` | `Long` | ATS-corrected timestamp. |
| `syncNode(ptr, din, timezone)` / `syncNet(ptr, din, bubbleTime)` | `NodeInfo` | Time sync *(not yet implemented upstream)*. |

### DDO transfer (store & forward)

| Function | Returns | Description |
|----------|---------|-------------|
| `push(ptr, din, ddo)` | `DaasError` | Push a DDO to a node. |
| `pull(ptr, din)` | `PullResult` | Pull one pending DDO (`error`, `ddo`). |
| `availablesPull(ptr, din)` | `AvailablePullResult` | Pending DDO count without pulling. |
| `addTypeset(ptr, typesetCode)` | `DaasError` | Register a typeset code. |

```kotlin
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

| Function | Returns | Description |
|----------|---------|-------------|
| `use(ptr, din)` | `Boolean` | Blocking; open a real-time session. |
| `send(ptr, din, data)` | `Int` | Send bytes; returns bytes sent. |
| `received(ptr, din)` | `Int` | Bytes available to receive. |
| `receive(ptr, din)` | `ByteArray` | Receive available data. |
| `end(ptr, din)` | `Boolean` | Close the session. |

### Features

| Function | Returns | Description |
|----------|---------|-------------|
| `addNodeFeatures(ptr, feature)` | `DaasError` | Advertise a local feature. |
| `getNodeFeatures(ptr)` | `Array<Feature>` | Local node features. |
| `nodeHasFeature(ptr, din, feature)` | `Boolean` | Whether a node advertises a feature. |
| `requestFeature(ptr, din, request, timeoutMillis = 1000)` | `DaasError` | Request a feature — **returns `ERROR_NOT_IMPLEMENTED` today**. |

### Policies, options & statistics

| Function | Returns | Description |
|----------|---------|-------------|
| `setAcceptRequestsLevel(ptr, policyLevel)` | `Unit` | Set trust policy. |
| `setDDOPolicy(ptr, policy)` | `DaasError` | Set delivery policy. |
| `setOptions(ptr, option, value)` | `DaasError` | Tune a low-level parameter. |
| `doStatisticsReset(ptr)` | `Boolean` | Reset statistics. |
| `getSystemStatistics(ptr, code)` | `Long` | Read one statistic. |
| `storeConfiguration(ptr, depotPtr)` / `loadConfiguration(ptr, depotPtr)` | `Boolean` | Persist / load config via a native depot handle. |
| `frisbee(ptr, din)` | `DaasError` | Reachability probe. |

> Not currently exposed: `frisbeeICMP`/`frisbeeDPERF`, `unlock`/`lock`,
> `setATSMaxError`, `listTypesets` (declared but disabled in this binding).

---

## `class DDO : Closeable`

```kotlin
DDO()            // empty
DDO(typeset = 1) // with typeset
```

| Member | Returns | Description |
|--------|---------|-------------|
| `setTypeset(t)` / `getTypeset()` | — / `Int` | Typeset tag. |
| `getOrigin()` | `Long` | Origin DIN. |
| `getTimestamp()` | `Long` | Timestamp. |
| `allocatePayload(size)` | `Long` | Pre-allocate the buffer. |
| `getPayloadSize()` | `Long` | Payload size. |
| `setPayload(data)` | `Long` | Set payload from a `ByteArray`. |
| `appendPayloadData(data)` | `Long` | Append to the payload. |
| `getPayload()` | `ByteArray` | Full payload. |
| `getPayloadAsBinary(offset, maxSize)` | `ByteArray` | Payload slice. |
| `clearPayload()` | `Unit` | Empty the payload. |
| `close()` | `Unit` | Free the native handle (use `use { }`). |

---

## `interface IDaasApiEvent`

Pass an implementation to `create(eventHandler = ...)` for per-instance events.
The native layer forwards events as-is (e.g. it does **not** auto-pull on
`ddoReceived` — call `pull()` yourself).

| Callback | Signature |
|----------|-----------|
| `dinAccepted` | `(din: Long)` |
| `ddoReceived` | `(payloadSize: Int, typeset: Int, din: Long)` |
| `frisbeeReceived` | `(din: Long)` |
| `nodeStateReceived` | `(din: Long)` |
| `atsSyncCompleted` | `(din: Long)` |
| `frisbeeDperfCompleted` | `(din: Long, packetsSent: Long, blockSize: Long)` |
| `networkDiscovered` | `(din: Long, sid: Long, link: LinkType)` |
| `nodeConnectedToNetwork` | `(sid: Long, din: Long)` |
| `streamInfoReceived` | `(din: Long, packetType: StreamType, streamId: Long)` |

> Because the listener needs `ptr` to call back but `create()` hasn't returned
> yet, give it a settable property and assign it after `create()`:
> `val h = MyHandler(); val ptr = DaasWrapper.create(eventHandler = h); h.ptr = ptr`.

---

## Data types

`PullResult` (`error`, `ddo`) · `AvailablePullResult` (`error`, `count`) ·
`NodeInfo` · `NodeNetworkInfo` · `NodeMapEntry` · `FeatureRequest`.

---

## Enums (`sebyone.daasiot_android.enums`)

### `LinkType` — communication drivers
| Value | Int | | Value | Int |
|-------|-----|-|-------|-----|
| `LINK_NONE` | 0 | | `LINK_MQTT5` | 5 |
| `LINK_DAAS` | 1 | | `LINK_UART` | 6 |
| `LINK_INET4` | 2 | | `LINK_ZIGBEE` | 7 |
| `LINK_LORA` | 3 | | `LINK_RAW` | 8 |
| `LINK_BLE` | 4 | | `LINK_LEGACY` | 9 |

### `Feature`
`STORAGE` (1) · `COMPUTE` (2).

### `PerformMode`
`PERFORM_CORE_THREAD` · `PERFORM_CORE_NO_THREAD`.

### `DiscoveryState`, `AcceptRequestPolicy`, `DDOPolicy`, `OptionType`, `StreamType`, `FeatureAction`
See the source enums; semantics match the native `daas_types.hpp`.

### `DaasError` — return codes (value)
`ERROR_NONE`(0), `ERROR_CORE_ALREADY_INITIALIZED`(1), `ERROR_CORE_STOPPED`(2),
`ERROR_CANNOT_INITIALIZE`(3), `ERROR_CANNOT_CREATE_NODE`(4),
`ERROR_DIN_ALREADY_EXIST`(5), `ERROR_CANNOT_MAP_NODE`(6),
`ERROR_INVALID_USER_TYPESET`(7), `ERROR_SEND_DDO`(8), `ERROR_NO_DDO_PRESENT`(9),
`ERROR_DIN_UNKNOWN`(10), `ERROR_CHANNEL_FAILURE`(11), `ERROR_ATS_NOT_SYNCED`(12),
`ERROR_DISCOVERY_DISABLED`(13), `ERROR_POLICY_STRICT_ENABLED`(14),
`ERROR_INVALID_DME`(15), `ERROR_THREADS_ALREADY_STARTED`(16),
`ERROR_NOT_IMPLEMENTED`(17), `ERROR_TX_QUEUE_FULL`(18),
`ERROR_TIMEOUT_NOT_EXPIRED`(19), `ERROR_ROUTE_NOT_FOUND`(20), `ERROR_UNKNOWN`(21).

---

## ProGuard / R8

Consumer rules are bundled in the AAR (`consumer-rules.pro`) — nothing to add on
your side. They keep `DaasWrapper`, `DDO`, every entity/enum and `IDaasApiEvent`
from being renamed/stripped, since the JNI layer resolves them by exact name.

---

## Minimal example

```kotlin
import sebyone.daasiot_android.DaasInstance
import sebyone.daasiot_android.DaasWrapper
import sebyone.daasiot_android.entity.DDO
import sebyone.daasiot_android.enums.LinkType

DaasInstance.create().use { node ->
    val ptr = node.handle
    DaasWrapper.doInit(ptr, sid = 100, din = 102)
    DaasWrapper.enableDriver(ptr, LinkType.LINK_INET4, "0.0.0.0:2223")
    DaasWrapper.map(ptr, din = 101, link = LinkType.LINK_INET4, uri = "192.168.1.20:2222")
    DDO(typeset = 1).use { ddo ->
        ddo.setPayload("Hello from Android".toByteArray())
        DaasWrapper.push(ptr, din = 101, ddo = ddo)
    }
}
```

See `daasiot-android-demo` for a full Jetpack Compose demo app.

For installation, see [`install.md`](install.md).

*© Sebyone Srl — Mozilla Public License 2.0*
