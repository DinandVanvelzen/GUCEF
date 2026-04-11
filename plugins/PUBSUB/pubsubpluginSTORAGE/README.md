# pubsubpluginSTORAGE

A GUCEF pub-sub plugin that uses the VFS (Virtual File System) layer as its storage
back-end.  It can act as both a **publisher** (writing incoming messages into rolling
container files) and a **subscriber** (replaying previously stored container files to
downstream consumers).

---

## Table of Contents

1. [Overview](#overview)
2. [Key Concepts](#key-concepts)
3. [Container File Format](#container-file-format)
4. [Sidecar Index (.sidx) Format](#sidecar-index-sidx-format)
5. [3-Phase Request Pipeline](#3-phase-request-pipeline-subscribe-side)
6. [Threading and Async Model](#threading-and-async-model)
7. [ACK Handling](#ack-handling)
8. [Configuration Reference](#configuration-reference)
9. [Sidecar Index Configuration](#sidecar-index-configuration)
10. [Codec / Compression Support](#codec--compression-support)
11. [Bookmark Persistence](#bookmark-persistence)
12. [Health and Metrics](#health-and-metrics)
13. [Plugin Registration](#plugin-registration)
14. [Code Organisation](#code-organisation)
15. [VFS Caching Behaviour (Important)](#vfs-caching-behaviour-important)

---

## Overview

`pubsubpluginSTORAGE` registers itself in the `CPubSubClientFactory` under the type name
**`"STORAGE"`**.  Each topic maps to a directory in the VFS.  Published messages are
serialised into binary container files; subscriptions replay those files in time order.

The plugin depends on:

| GUCEF Module | Role |
|---|---|
| `gucefCORE`   | Threading, buffers, timers, logging, serialisation |
| `gucefVFS`    | File I/O abstraction (local disk, archives, S3, …) |
| `gucefPUBSUB` | Pub-sub interface types, serialisers, bookmarks |
| `gucefMT`     | Mutexes |

---

## Key Concepts

### Container files

A *container file* holds one serialised batch of pub-sub messages, written by the
binary serialiser (`CPubSubMsgContainerBinarySerializer`).  The filename encodes three
ISO-8601 timestamps (first message datetime, last message datetime, write datetime) so
the topic can reconstruct an ordered timeline without opening the files.

```
<firstMsgDt>_<lastMsgDt>_<writeDt>.v1.pubsubmsgs
```

Example:
```
00000000000000.000Z_00000000000000.000Z_20260404202732.446Z.v1.pubsubmsgs
```

### Buffer swap

Incoming messages are accumulated in a `CDynamicBufferSwap` (double-buffered).  When
the active buffer exceeds `desiredMinimalSerializedBlockSize` bytes **or** the
`desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs` timeout fires, the buffer is
finalised and handed to the VFS ops subsystem for writing.

### VFS storage root

Each topic has a `vfsStorageRootPath` that can be a literal VFS path or a template
containing `{topicName}`.  Container files and sidecar index files are placed inside
this directory.

---

## Container File Format

Managed entirely by `CPubSubMsgContainerBinarySerializer` (in `gucefPUBSUB`).  Key
properties relevant to the STORAGE plugin:

- Variable-length binary format, version tagged.
- Each message's byte offset within the file is recorded in a
  `TMsgOffsetIndex` (`vector<UInt32>`).  The topic uses this index to seek to any
  specific message without scanning the whole file.
- `FinalizeWriteBuffer()` writes the offset index into the file footer so it can be
  reconstructed on load.

---

## Sidecar Index (.sidx) Format

An optional sidecar index allows O(log N) lookup of a replay start position by
logical key value (e.g. FIX sequence number, order ID) without scanning all containers.

**Index filename convention:**
```
idx.<keySource>.<keyName>.<typeCode>.sidx
```

Where:
- `keySource` : `mk` (metadata key) or `k` (key-value pair)
- `keyName`   : the key name being indexed (e.g. `seqNr`, `fix_seq_num`)
- `typeCode`  : `su64` (sorted uint64), `si64` (sorted int64), `sstr` (sorted string)

**Binary layout (all values little-endian):**

```
[Header — 23 + keyNameLen bytes]
  magic           : "GUCEFIDX"  (8 bytes, no null)
  format_version  : UInt8       (= 1)
  index_type_id   : UInt32      (su64=1, si64=2, sstr=3)
  flags           : UInt32      (reserved = 0)
  sampling_interval: UInt32     (1 = every msg)
  key_name_len    : UInt16
  key_name        : key_name_len bytes (ASCII, no null)

[Index Entries — immediately after header; 16 bytes each for su64/si64]
  key_value       : UInt64
  file_id         : UInt32
  msg_index       : UInt32   ← index into that container's TMsgOffsetIndex

[File Registry — starts at file_registry_offset from footer]
  file_registry_count: UInt32
  per entry (variable):
    file_id         : UInt32
    first_key_value : UInt64
    last_key_value  : UInt64
    entry_count     : UInt32
    filename_len    : UInt16
    filename        : filename_len bytes (ASCII, no null)

[Footer — always last 20 bytes]
  index_entry_count    : UInt32
  index_entries_offset : UInt32  (= header size; fixed at creation)
  file_registry_offset : UInt32  (byte position of file registry)
  file_registry_count  : UInt32
  footer_magic         : UInt32  (0x47494458 = "GIDX")
```

**Write path** (`CStoragePubSubIndexWriter`):

The index writer is called from the publish side immediately after each container file
is successfully stored to VFS.  It is lazy-initialised: on the first call it checks
whether a `.sidx` already exists and loads its state if so.

**Important:** The writer maintains all index entries **in memory** (`m_allKeys`,
`m_allFileIds`, `m_allMsgIndexes` parallel vectors).  Each `AppendContainer()` call
rewrites the complete `.sidx` from in-memory data without reading back the previously
written file.  This is deliberate — the VFS layer has a read cache that may return a
stale version of the file, which would corrupt the index if the write relied on
reading it back.

**Read path** (`CStoragePubSubIndexReader`):

`LoadIndex()` parses the entire `.sidx` into memory.
`FindStartBookmark(startKeyValue, bookmark)` uses a binary-search over `m_entries`
to return the first `CStorageBookmarkInfo` where `keyValue >= startKeyValue`.

---

## 3-Phase Request Pipeline (subscribe side)

When a consumer subscribes (especially with a date-time range or key-value start
position), requests flow through three in-memory queues:

| Stage | Queue | Activity |
|---|---|---|
| **0** | `m_stage0StorageToPubSubRequests` | Persistent requests waiting for matching data to appear |
| **1** | `m_stage1StorageToPubSubRequests` | VFS directory scanned; matching container file list built |
| **2** | `m_stage2StorageToPubSubRequests` | Container files loaded into buffers; messages delivered |

Each `StorageToPubSubRequest` carries:
- `startDt`, `endDt` — date-time window
- `vfsPubSubMsgContainersToPush` — ordered list of file paths to serve
- Optional: action ID range for selective serving

Completed containers are tracked in `m_completedContainers` (bounded by
`maxCompletedContainerRefsToRetain`, default 50).

---

## Threading and Async Model

Two orthogonal settings control I/O concurrency:

| Setting | Default | Effect when `false` |
|---|---|---|
| `performVfsOpsASync` | `true` | VFS operations run synchronously in the calling thread |
| `performVfsOpsInDedicatedThread` | `true` | VFS operations still run async but on a shared thread pool |

When `performVfsOpsInDedicatedThread` is `true` a `CStoragePubSubClientTopicVfsTask`
(`CTaskConsumer` subclass) is created and given its own thread from the client's
thread pool.

Optional CPU affinity for the dedicated VFS thread:
- `applyDedicatedVfsOpsThreadCpuAffinity` (default `false`)
- `cpuAffinityForDedicatedVfsOpsThread` (default `0`)

**Timers used by the topic:**

| Timer | Purpose |
|---|---|
| `m_syncVfsOpsTimer` | Drives periodic sync of async VFS operation queue |
| `m_reconnectTimer` | Attempts reconnection after connectivity loss |
| `m_noAckRetransmitTimer` | Checks for unacknowledged message batches and retransmits |
| `m_bufferContentTimeWindowCheckTimer` | Ages out buffers that have been waiting too long |

**Rate limiting:** `ticketRefillOnBusyCycle` (default 10 000) — tickets allocated per
busy-cycle to throttle VFS operations.

---

## ACK Handling

Each `StorageBufferMetaData` carries a `msgAcks` bit-vector.  Downstream consumers
call `AcknowledgeReceipt()` with either a message pointer or a `CStorageBookmarkInfo`.
When all messages in a container are ACK'd the container is considered complete and
can optionally be deleted (`deleteContainersWithFullyAckdContent`) or archived
(`moveContainersWithFullyAckdContent` → `vfsStorageRootPathForFullyAckdContainers`).

Non-ACK'd message batches trigger retransmission after
`maxTimeToWaitForAllMsgBatchAcksInMs` (default 30 s), checked every
`nonAckdMsgCheckIntervalInMs` (default 30 s).

---

## Configuration Reference

All STORAGE-specific settings are placed inside the `customConfig` child of a topic
config node.  Settings not listed here fall back to defaults.

### Topic-level settings

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `vfsStorageRootPath` | string | `{topicName}` | VFS directory for container files. Supports `{topicName}` template. |
| `vfsFileExtention` | string | `pubsubmsgs` | Extension appended to container filenames (no leading dot). |
| `desiredNrOfBuffers` | UInt32 | 2 | Number of swap buffers used for serialisation. |
| `desiredMinimalSerializedBlockSize` | UInt32 | 52428800 (50 MB) | Minimum serialised bytes before a buffer is flushed. |
| `desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs` | UInt32 | 300000 (5 min) | Max time to wait before flushing even an undersized buffer. |
| `performVfsOpsASync` | bool | true | Enable async VFS operations. Set `false` for deterministic unit-test behaviour. |
| `performVfsOpsInDedicatedThread` | bool | true | Use a dedicated thread for VFS operations. |
| `applyDedicatedVfsOpsThreadCpuAffinity` | bool | false | Pin the dedicated VFS thread to a specific CPU core. |
| `cpuAffinityForDedicatedVfsOpsThread` | UInt32 | 0 | Core index for CPU affinity (0-based). |
| `ticketRefillOnBusyCycle` | UInt32 | 10000 | Rate-limit tickets for VFS ops per busy cycle. |
| `collectMetrics` | bool | true | Gather operational metrics (see [Health and Metrics](#health-and-metrics)). |
| `subscribeWithoutBookmarkIfNoneIsPersisted` | bool | true | Subscribe from the beginning of storage if no persisted bookmark is found. |
| `autoPushAfterStartupIfStorageToPubSub` | bool | true | Automatically initiate storage→pubsub replay on startup. |
| `autoPushAfterStartupIsPersistent` | bool | true | Whether the auto-push replay request persists (stays active as new files arrive). |
| `youngestStoragePubSubMsgFileToLoad` | CDateTime | max-future | Upper bound on container file timestamps for replay. |
| `oldestStoragePubSubMsgFileToLoad` | CDateTime | min-past | Lower bound on container file timestamps for replay. |
| `defaultCodecDecodeGrowthRatioExpectation` | Float32 | 6.0 | Expected decompression ratio used for buffer pre-allocation. |
| `bestEffortDeserializeIsAllowed` | bool | false | Continue deserialising even if some messages are corrupt. |
| `maxStorageCorruptionDetectionsToBeHealthy` | Int32 | 0 | Max corruption events before topic becomes unhealthy (0 = never unhealthy). |
| `maxStorageDeserializationFailuresToBeHealthy` | Int32 | 0 | Max deserialisation failures before topic becomes unhealthy (0 = never unhealthy). |
| `treatEveryFullfilledRequestAsEODEvent` | bool | false | Fire an end-of-data event after every fulfilled replay request. |
| `deleteContainersWithFullyAckdContent` | bool | false | Delete container files once all their messages are ACK'd. |
| `moveContainersWithFullyAckdContent` | bool | false | Move fully-ACK'd containers to an archive directory. |
| `vfsStorageRootPathForFullyAckdContainers` | string | `` | Destination VFS path for moved ACK'd containers. |
| `maxCompletedContainerRefsToRetain` | UInt32 | 50 | How many completed-container records to keep in memory. |
| `nonAckdMsgCheckIntervalInMs` | UInt32 | 30000 | How often to check for unacknowledged message batches. |
| `maxTimeToWaitForAllMsgBatchAcksInMs` | UInt32 | 30000 | How long to wait for a full batch ACK before retransmitting. |
| `topicFollowsDirRenames` | bool | false | If the VFS directory is renamed, update the topic root rather than disconnecting. |

### Serialiser sub-nodes

| Sub-node | Type | Description |
|---|---|---|
| `PubSubMsgBinarySerializerOptions` | node | Settings for `CPubSubMsgBinarySerializerOptions` (binary serialisation). |
| `PubSubMsgSerializerOptions` | node | Settings for `CPubSubMsgSerializerOptions` (text/JSON serialisation). |

### Codec settings (optional compression)

| Key | Type | Description |
|---|---|---|
| `encodeCodecFamily` | string | Codec family for encoding (e.g. `"zlib"`). Empty = no encoding. |
| `encodeCodecName` | string | Specific codec name for encoding. |
| `decodeCodecFamily` | string | Codec family for decoding. Empty = no decoding. |
| `decodeCodecName` | string | Specific codec name for decoding. |

---

## Sidecar Index Configuration

One or more `<index>` child nodes may be added inside `customConfig` to enable sidecar
indexing.

```xml
<index keySource="k"  keyName="seqNr"       indexType="su64" samplingInterval="1" />
<index keySource="mk" keyName="fix_seq_num"  indexType="su64" samplingInterval="1" />
<index keySource="k"  keyName="orderId"      indexType="sstr" samplingInterval="10" />
```

| Attribute | Values | Description |
|---|---|---|
| `keySource` | `k` (key-value pair) or `mk` (metadata key) | Where to look for the key in each message. |
| `keyName` | string | The exact key name to index. |
| `indexType` | `su64`, `si64`, `sstr` | Sort type: unsigned uint64, signed int64, or string. |
| `samplingInterval` | UInt32 ≥ 1 | Index every N-th message. 1 = every message. |

The index is populated on the **publish side** automatically and can be queried on the
**subscribe side** via `SubscribeStartingAtKeyValue(keyNameWithPrefix, startKeyValue)`
where `keyNameWithPrefix` is `"k:seqNr"` or `"mk:fix_seq_num"`.

---

## Codec / Compression Support

If `encodeCodecFamily` / `encodeCodecName` are set, each container is encoded before
being stored.  On the read path the matching decode codec is applied.  The codec must
be available as a GUCEF codec plugin (e.g. `codecspluginZLIB`).

`defaultCodecDecodeGrowthRatioExpectation` (default 6.0) is used to pre-size the
decode buffer.  Increase it if messages are highly compressed.

---

## Bookmark Persistence

Bookmarks record the exact position within storage (`CStorageBookmarkInfo`):

| Field | Type | Description |
|---|---|---|
| `bookmarkFormatVersion` | UInt8 | Format version (currently 1). |
| `doneWithFile` | UInt8 | Non-zero = fully consumed this file. |
| `msgIndex` | UInt32 | Message index within container. |
| `offsetInFile` | UInt32 | Byte offset within container. |
| `vfsFilePath` | CString | Full VFS path of the container file. |

Bookmarks are persisted via the optional `PUBSUB::CIPubSubBookmarkPersistence`
interface configured at the client level under `pubsubBookmarkPersistenceConfig`.
On restart, the topic calls `SubscribeStartingAtBookmark()` if a persisted bookmark
exists; otherwise (when `subscribeWithoutBookmarkIfNoneIsPersisted = true`) it starts
from the oldest container.

---

## Health and Metrics

### TopicMetrics

Collected each timer cycle when `collectMetrics = true`:

| Field | Description |
|---|---|
| `queuedReadyToReadBuffers` | Buffers waiting to be delivered downstream. |
| `smallestBufferSizeInBytes` | Smallest container buffer encountered. |
| `largestBufferSizeInBytes` | Largest container buffer encountered. |
| `msgsLoadedFromStorage` | Total messages loaded from storage since start. |
| `msgBytesLoadedFromStorage` | Total bytes loaded from storage since start. |
| `msgsNotifiedAsReceived` | Total messages delivered downstream. |
| `msgsWrittenToStorage` | Total messages written to storage since start. |
| `msgBytesWrittenToStorage` | Total bytes written to storage since start. |
| `storageCorruptionDetections` | Number of corruption events detected. |
| `storageDeserializationFailures` | Number of deserialisation failures. |

### Health thresholds

A topic is considered **healthy** unless corruption or deserialisation failure counts
exceed their configured thresholds.  Thresholds of `0` (default) mean the topic never
becomes unhealthy due to those events.  Health changes fire the
`OnTopicHealthStatusChange` event.

---

## Plugin Registration

On `GUCEFPlugin_Load()` the plugin registers a factory for type `"STORAGE"` with
`PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory()`.

Link-back dependencies reported by `GUCEFPlugin_GetLinkBackModuleDependencies()`:
```
gucefCORE,gucefVFS,gucefPUBSUB
```

---

## Code Organisation

```
plugins/PUBSUB/pubsubpluginSTORAGE/
├── include/
│   ├── pubsubpluginSTORAGE.h                        Plugin API forward declarations
│   ├── pubsubpluginSTORAGE_config.h                 Compile-time config / export macros
│   ├── pubsubpluginSTORAGE_macros.h                 DLL import/export macros
│   ├── pubsubpluginSTORAGE_CStoragePubSubClient.h         Top-level client class
│   ├── pubsubpluginSTORAGE_CStoragePubSubClientConfig.h   Client-level config
│   ├── pubsubpluginSTORAGE_CStoragePubSubClientTopic.h    Topic class (core logic)
│   ├── pubsubpluginSTORAGE_CStoragePubSubClientTopicConfig.h  All topic config params
│   ├── pubsubpluginSTORAGE_CStoragePubSubClientTopicVfsTask.h Dedicated VFS thread task
│   ├── pubsubpluginSTORAGE_CStoragePubSubIndexDef.h       Index definition (key, type, interval)
│   ├── pubsubpluginSTORAGE_CStoragePubSubIndexWriter.h    .sidx write path
│   └── pubsubpluginSTORAGE_CStoragePubSubIndexReader.h    .sidx read / lookup path
└── src/
    ├── pubsubpluginSTORAGE.cpp                       GUCEFPlugin_Load/Unload entry points
    ├── pubsubpluginSTORAGE_CStoragePubSubClient.cpp
    ├── pubsubpluginSTORAGE_CStoragePubSubClientConfig.cpp
    ├── pubsubpluginSTORAGE_CStoragePubSubClientTopic.cpp  ~3 000 lines; main publish/subscribe logic
    ├── pubsubpluginSTORAGE_CStoragePubSubClientTopicConfig.cpp
    ├── pubsubpluginSTORAGE_CStoragePubSubClientTopicVfsTask.cpp
    ├── pubsubpluginSTORAGE_CStoragePubSubIndexDef.cpp
    ├── pubsubpluginSTORAGE_CStoragePubSubIndexWriter.cpp
    └── pubsubpluginSTORAGE_CStoragePubSubIndexReader.cpp
```

**Key relationship summary:**

```
CStoragePubSubClient
  └── owns 1..* CStoragePubSubClientTopic
        ├── config:  CStoragePubSubClientTopicConfig
        │              └── 0..*  CStoragePubSubIndexDef
        ├── write:   CStoragePubSubIndexWriter (one per index def, lazy-init, stored in map)
        ├── read:    CStoragePubSubIndexReader (created on-demand for key-value subscriptions)
        └── thread:  CStoragePubSubClientTopicVfsTask (optional dedicated VFS thread)
```

---

## VFS Caching Behaviour (Important)

The VFS layer maintains a **read cache**.  When `vfs.LoadFile()` loads a file for the
first time it stores the content in the cache; subsequent `LoadFile()` calls for the
same path return the cached version regardless of whether the file has been
overwritten on disk by `vfs.StoreAsFile()`.

**Consequence for index writing:** The `CStoragePubSubIndexWriter` previously used a
read-modify-write pattern (load → append → store) for each container.  This caused
every `AppendContainer()` call after the first to read the original (first-written)
version of the `.sidx` from cache, producing a file with only 2 actual 16-byte index
entries even though `m_totalEntryCount` (and therefore the footer field) correctly
reached 300.

**Current fix:** `CStoragePubSubIndexWriter` keeps all index entries in three parallel
in-memory vectors (`m_allKeys`, `m_allFileIds`, `m_allMsgIndexes`) and always
regenerates the complete `.sidx` from those vectors without ever reading the file
back from VFS.  `vfs.LoadFile()` is now only called during `Initialize()` on startup
to restore state from an existing `.sidx`.

**Implication for tests:** Tests that verify `.sidx` content must also avoid reading
the file via VFS (which would return the cached first-version).  They should either
construct the native filesystem path directly or ensure the VFS cache has been
invalidated.  In the current test suite this is handled by deleting the `.sidx` at
the start of each test run (ensuring the first `LoadFile()` call goes to disk) and
calling the test's `LoadFile()` after all `AppendContainer()` calls are complete (so
the cache miss goes to the most-recent on-disk version).
