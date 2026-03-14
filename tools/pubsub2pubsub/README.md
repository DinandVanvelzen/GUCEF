# The pubsub2pubsub service

The purpose of this service is to allow for a config driven conceptual publish-subscribe flow to be defined and operated, eliminating the need to build specific applications for specific interoperability problemsets. 
This service aims to have a plugin architecture driven approach that acts as a middleware glue solution.

## Roles & responsibilities hierarchy
The service as a whole ties together various building blocks to tie together various abstractions that together allow for a middleware abstracted formulation of pubsub behavior which can then be flow directed 
and managed across multiple different instantiations of said abstractions.
The overall roles & responsibilities hierarchy is as follows:

```
+----------------+         +---------+        +------+        +---------------+        +---------------------+
| pubsub2pubsub  |  1 -> N | channel | 1 -> M | side | 1 -> 1 | pubsub client | 1 -> X | pubsub client topic |
+----------------+         +---------+        +------+        +---------------+        +---------------------+
                                | 1              /\ M
                                | 1               | 
                           +-------------+ 1      |
                           | flow router |<-------|
                           +-------------+						   
```

## What is a 'channel'?
The term 'channel' is borrowed from the fintech world, where it is commonly used to describe an isolated stream of information flow. In this service a channel is a fully self-contained routing pipeline: it owns its own set of sides, its own flow router instance, and its own threading configuration — entirely isolated from every other channel running in the same process.

Each channel defines what data flows, between which backends, in which direction(s), with what reliability guarantees, and on which threads. Multiple channels can run simultaneously in one process, each with completely independent configuration. Two channels may use the same template — for example to achieve parallelism across CPU cores with the same backend wiring — or entirely different templates, for example one channel doing UDP→Kafka while another does Redis→STORAGE.

Channels also serve as the unit of command and control: a single REST API governs all channels regardless of how many are running.

Note that every channel has at minimum its own thread. This is not optional, contrary to many other areas of the threading model that are config-driven. See the **Threading model** section below for how to tune threading further.

## What is a 'side'?
A side is a logical edge of the routing pipeline you define on a per-channel basis. (Right now the application only supports 2 sides, but allowing M number is a design goal down the line.)

What a side is in real-world terms completely depends on your configuration. If traffic flows only in one direction you could think of one side as the ingress and the other as the egress. If data flow is bidirectional that framing makes less sense, which is why the more abstract term 'side' is used. One could argue for calling it a network or transport interface, but that too may conjure up things that do not match reality when, for example, one side is a STORAGE backend.

A side is therefore simply whatever you have defined it to be on that logical edge of the data flow — as dictated by the pubsub backend chosen for that side, the `needPublishSupport` / `needSubscribeSupport` declarations on its topics, and the applicable config settings for that backend. A side acts as the glue that ties the pubsub client abstraction into the larger channel context.

Note that every side can optionally have its own dedicated OS thread. See the **Threading model** section for the full breakdown.

## What is a 'flow router'?
Each channel has exactly one flow router. Its sole responsibility is to direct message traffic between the sides that belong to that specific channel. The router is a local, per-channel concept — not a global or cross-channel one.

Routes are configured as directed edges from a `fromSideId` (the source of messages for that route) to a `toSideId` (the destination). Traffic direction is not fixed by the system: it is determined by what each side declares via `needPublishSupport`, `needSubscribeSupport`, and `DesiredFeatures` in the config. A channel can have routes going in both directions simultaneously — for example A→B and B→A — enabling fully bidirectional data flow where backend capabilities permit.

Beyond simple from→to routing, a route can assign special roles to additional sides:

- **Failover side**: An alternate destination that activates automatically when the primary destination has been unhealthy for longer than `minBadHealthDurationBeforeFailoverInMs`.
- **Spillover buffer side**: An emergency interim destination (typically a STORAGE side) that absorbs messages when both the primary and failover destinations are unavailable, preventing back-pressure on the source side. Unlike a failover, the spillover is not a final destination — it is a temporary stop. Once a healthy primary or failover route is reestablished the spillover drains its held messages toward that route, then regular flow resumes.
- **Dead letter side**: Receives messages that could not be delivered after all retry and failover attempts are exhausted.

The router monitors side health on a configurable interval (`routeSwitchingTimerIntervalInMs`) and switches routes automatically as health state changes. The `ackStyle` setting controls how acknowledgment propagates back to the source: `AllOrNothing` requires all active destination sides to acknowledge a message before the source side considers delivery complete.

## Threading model

The threading model is layered and fully config-driven. Understanding it is important for both performance tuning and avoiding deadlocks.

### Layer 1 — Channel thread (always present)

Every channel has exactly one dedicated OS thread that drives its internal pulse loop. This thread is mandatory and always present regardless of any other threading settings.

### Layer 2 — Side thread (optional)

Each side can optionally be given its own dedicated OS thread by setting `performPubSubInDedicatedThread: true`. When a side runs on its own thread, inter-side message delivery within the channel passes through a mailbox: messages are enqueued asynchronously into the receiving side's inbox rather than being delivered directly on the calling thread.

Thread-to-CPU pinning is available per side via `applyThreadCpuAffinity` and `cpuAffinityForPubSubThread`, which is useful for isolating high-throughput sides to dedicated cores and keeping the load profile predictable.

If every side in every channel uses a dedicated thread, the total OS thread count is approximately:
```
threads = 1 (main) + nrOfChannels × nrOfSidesPerChannel
```

### Layer 3 — Topic thread (optional, backend-provided)

Some backends support an optional thread-per-topic mode, where each topic on a side gets its own dedicated OS thread. This is the finest-grained threading option and carries higher scheduler overhead, so it is recommended only for latency-sensitive, high-frequency topics — for example, real-time market data streams where per-message latency matters more than aggregate throughput.

### Straight-through flow optimization

When thread-per-topic is active, the system checks at message delivery time whether the currently executing thread is already the thread designated for the receiving code path. If so, the mailbox handoff is bypassed entirely and the message flows straight through on the same thread, from the source topic all the way to the destination — eliminating one thread scheduling round-trip.

This optimization can be requested at the route level via `preferFromTopicThreadForDestination: true`. This tells the flow router: "if the backend capabilities of both sides permit it, prefer using the source topic's own thread all the way through to the destination side, rather than handing off via a mailbox." Whether the preference can be honored depends on what the involved backends declare as supported capabilities.

### Deadlock risk

Because the threading model permits many combinations, misconfiguration can lead to deadlocks — for example, if two sides are each waiting on the other's mailbox for an acknowledgment while no thread is draining that mailbox. If you encounter hangs under load, review which sides have dedicated threads and whether the `ackStyle` requirement can be satisfied given the configured threading topology.

## The pubsub 'STORAGE' backend
The storage backend was designed to allow for block based recording and replay of any messages from any of the other backends
This provides a singular performant option regardless of the other backends used. In order to achieve this it handles messages as-is without making any assumptions about what is or isnt available on a given message.
such messages are serialized into buffers using a binary format and subsequently written to the VFS as a single block, a container of messages.
The message containers are more than just a raw recording of messages. They include a header and a footer for verification, faster seeking and limited recovery / detection of corrupted and or trunkated files.
The binary format uses the Galaxy Unlimited Platform provided binary serialization code to ensure compatibility. Because the pubsub system at large makes a lot of use of Variant storage every format/type the variant can represent we can also store faithfully.
Note that on a per topic basis the backend only supports mutually exclusive ingress or egress, not both at the same time.
For performance reasons dynamic memory management is kept to an absolute minimum and for this reason buffers are continually reused for serialization of the blocks of messages.
In order to get the best performance it is key to choose the backend settings such to avoid alterations to the in-memory storage. This is easier on egress (writing out blocks) then it is on ingress (reading blocks into memory) especially when compression is used.
The storage backend, when configured as a sink (publish), will produce time series capture files using a hardcoded scheme where each recording time segment produces a file with the following scheme:
```
<containerCaptureStartTimestamp>_<firstMessageInContainerTimestamp>_<lastMessageInContainerTimestamp>.<configuredExtensionName> 
```
These timestamps follow the ISO 8601 standard: https://en.wikipedia.org/wiki/ISO_8601
Note that Kaitai (https://kaitai.io/) specs are also available for the storage backend generated capture files within this repo, should you wish to use independent parsing code.

---

## Configuration

### Overview

pubsub2pubsub is configured via two layers of JSON files:

1. **App config** (`pubsub2pubsub_d.json`) — the top-level config that defines the runtime environment: which channel templates to load, which channel instances to create, VFS mounts, REST API settings, metrics, and cloud SDK settings.
2. **Channel template files** — each defines one routing unit (a channel): which backends are connected as sides, how topics are configured per side, and how data is routed between sides via a flow router.

A channel template is a _blueprint_. The app config instantiates it one or more times as channel instances (via `explicitChannelOverlays` or `numericalAutoChannels`), allowing you to reuse the same wiring pattern at different channel IDs or with different topic/address overrides.

---

### App Config (`pubsub2pubsub_d.json`)

The app config is a flat JSON object with top-level sections:

#### `PubSub2PubSubConfig`

The main runtime controller block.

| Field | Type | Description |
|---|---|---|
| `channelConfigTemplateRefs` | `string[]` | VFS paths to channel template JSON files to load at startup. Supports `vfs://` prefixed paths. |
| `channelConfigTemplates` | `object[]` | Inline channel templates (alternative to file refs). |
| `explicitChannelOverlays` | `object[]` | Channel instances. Each references a template by name and specifies channel-specific overrides (channel ID, channel name, per-side topic names and remote addresses). |
| `numericalAutoChannels` | `object` | Generates a contiguous range of channels from a single template: `usingTemplate`, `channelCount`, `firstChannelId`, `channelIds`. |
| `globalStandbyEnabled` | `bool` | When true, channels start in standby mode. |
| `enableRestApi` | `bool` | Enable the built-in REST API server. |
| `restApiPort` | `number` | Port for the REST API (default: 10000). |
| `restBasicHealthUri` | `string` | URI for the lightweight health probe used by load balancers (e.g. `/health/basic`). |
| `transmitMetrics` | `bool` | Enable metric emission to the StatsD destination. |

**Channel instance overlay** (`explicitChannelOverlays` entry):

```json
{
    "usingTemplate": "<templateName>",
    "channelId": 0,
    "channelName": "my-channel",
    "sides": [
        {
            "sideId": "<sideIdFromTemplate>",
            "topics": [ { "topicName": "my-topic" } ],
            "remoteAddresses": [ "broker:9092" ]
        }
    ]
}
```

The `sides` array overrides only the fields you specify; all other side settings are inherited from the template.

#### `StatsDClient`

| Field | Type | Description |
|---|---|---|
| `statsDestination` | `string` | UDP endpoint for StatsD (`host:port`). |
| `statsNamePrefix` | `string` | Prefix for all metric names. Supports `$HOSTNAME$` substitution. |
| `transmit` | `bool` | Whether to actually send metrics. |
| `logStats` | `bool` | Also log metrics to the application log. |

#### `VFS`

Defines virtual filesystem mount points used throughout all VFS path references in channel configs.

```json
{
    "archives": [
        {
            "actualArchivePath": "$MODULEDIR$",
            "mountPath": "DataPath",
            "archiveName": "DataPath",
            "archiveType": "FileSystem",
            "writeable": true,
            "readable": true
        }
    ]
}
```

The special variable `$MODULEDIR$` resolves to the directory containing the executable. Typical mount names used in templates: `InstallPath` (read-only, for template files) and `DataPath` (read-write, for storage, bookmarks, journals).

#### `AWSSDK`

Cloud credential and region settings. The `AWSCredentialsProviders` field is a semicolon-delimited priority list: `Environment;Config;ProfileConfigFile;InstanceProfile`.

#### `GlobalDnsCache`

Optional async DNS pre-resolution cache for remote endpoints.

#### `Main.AppArgs`

General application settings (log level, output directory).

---

### Channel Template Files

Each channel template is a standalone JSON file that fully describes one routing configuration. Templates live in the `config/examples/channel_templates/` directory and can be referenced by VFS path from the app config.

A JSON Schema for channel templates is provided at:
```
config/channel_template.schema.json
```

#### Top-level fields

| Field | Type | Description |
|---|---|---|
| `channelId` | `number` | Numeric channel ID (use `-1` in a template to indicate "assigned at instantiation"). |
| `channelName` | `string` | Human-readable name. |
| `collectMetrics` | `bool` | Enable per-channel metrics collection. |
| `metricsIntervalInMs` | `number` | How often metrics are sampled (ms). |
| `ticketRefillOnBusyCycle` | `number` | CPU yield ticket refill count for busy-loop throttling. |
| `PubSubSides` | `object` | Map of side ID → side config (see below). |
| `PubSubFlowRouterConfig` | `object` | Route definitions (see below). |

#### Side configuration (`PubSubSides.<sideId>`)

Each side wraps a single pub/sub client instance. Common settings:

| Field | Description |
|---|---|
| `performPubSubInDedicatedThread` | Give this side its own OS thread. |
| `applyThreadCpuAffinity` / `cpuAffinityForPubSubThread` | Pin the side's thread to a logical CPU. |
| `retryFailedPublishAttempts` | Retry publish failures. |
| `maxMsgPublishRetryAttempts` | Max retry count before giving up. |
| `maxMsgPublishRetryTotalTimeInMs` | Max total retry window (ms). |
| `maxPublishedMsgInFlightTimeInMs` | Age limit for in-flight messages. |
| `maxTotalMsgsInFlight` | Back-pressure limit: max concurrent unacknowledged messages. |
| `allowOutOfOrderPublishRetry` | Permit retrying messages out of their original sequence. |
| `subscribeWithoutBookmarkIfNoneIsPersisted` | Subscribe from the beginning if no bookmark exists. |

Each side contains:
- **`PubSubClientConfig`** — backend type and all backend-specific settings.
- **`PubSubBookmarkPersistenceConfig`** — how read positions are persisted across restarts.

#### `PubSubClientConfig`

| Field | Description |
|---|---|
| `pubsubClientType` | Backend discriminator. One of the supported backend names (see next section). |
| `reconnectDelayInMs` | Delay before reconnect attempts (ms). |
| `RemoteAddresses` | Array of `"host:port"` strings for broker/server addresses. |
| `Topics` | Array of topic config overrides. Usually empty `[]` — use `defaultTopicConfig` instead. |
| `defaultTopicConfig` | Default config applied to all topics on this side. Contains `topicName`, `needPublishSupport`, `needSubscribeSupport`, and a backend-specific `CustomConfig` block. |
| `DesiredFeatures` | Capability matrix declared for this side (see below). |
| `journalConfig` | Client-level message journal settings. |
| `CustomConfig` | Client-level backend-specific settings. |

**Journal config** (present at both client and topic level):

```json
{
    "useJournal": false,
    "journalType": "binary",
    "journalPath": "vfs://DataPath/journals/clientType/{clientType}/instance/{clientInstance}"
}
```

Enables binary message journaling for recovery after crashes. Path supports `{clientType}` and `{clientInstance}` substitution variables.

#### `PubSubBookmarkPersistenceConfig`

Persists subscriber read positions to disk so the service can resume from where it left off after a restart.

```json
{
    "bookmarkPersistenceType": "VFS",
    "autoPersistIfClientSideBookmarksNeeded": true,
    "autoPersistIntervalInMs": 1000,
    "autoPersistMsgInterval": 1000,
    "maxNrOfBookmarksToKeep": 10,
    "CustomConfig": {
        "vfsRootPath": "DataPath/bookmarks",
        "persistenceStructure": "{bookmarkNamespace}/clientType/{clientType}/topicName/{topicName}"
    }
}
```

#### `DesiredFeatures`

A boolean capability matrix that the side declares to the flow router and the rest of the system. Most fields are auto-populated based on the backend type but can be overridden. Key fields:

| Field | Meaning |
|---|---|
| `supportsPublishing` / `supportsSubscribing` | Whether the side can send / receive. |
| `supportsAutoReconnect` | Backend reconnects automatically on disconnect. |
| `supportsBinaryPayloads` | Backend can carry raw binary message payloads. |
| `supportsBookmarkingConcept` | Backend has a native read-position concept. |
| `supportsSubscriberMsgReceivedAck` | Subscriber can explicitly acknowledge messages. |
| `supportsMetrics` | Backend reports operational metrics. |
| `supportsMultiHostSharding` | Backend supports sharding across multiple broker hosts. |
| `supportsGlobPatternTopicNames` | Topic names can use glob patterns for subscription. |
| `supportsSubscriptionEndOfDataEvent` | Backend can signal that a finite data set is exhausted. |

#### Flow router configuration (`PubSubFlowRouterConfig`)

| Field | Description |
|---|---|
| `ackStyle` | How message acknowledgment is handled: `AllOrNothing` means all destination sides must ack before the source is acked. |
| `minBadHealthDurationBeforeFailoverInMs` | How long a side must be unhealthy before failover activates. |
| `minBadHealthDurationBeforeSpilloverInMs` | How long before spillover buffer activates. |
| `minPrimarySideGoodHealthDurationBeforeActivationInMs` | How long a recovered primary must be healthy before reactivation. |
| `routeSwitchingTimerIntervalInMs` | Health check polling interval for route switching decisions. |
| `routes` | Array of route definitions. |

**Route definition:**

```json
{
    "fromSideId": "redis-source",
    "toSideId": "storage-sink",
    "failoverSideId": "",
    "spilloverBufferSideId": "",
    "deadLetterSideId": "",
    "toSideTopicsAutoMatchFromSide": true,
    "preferFromTopicThreadForDestination": true,
    "topicAssociations": []
}
```

`topicAssociations` allows explicit source-topic → destination-topic pairing when auto-matching is not desired.

`preferFromTopicThreadForDestination: true` requests the straight-through flow optimization: if both sides' backends support thread-per-topic, the flow router will attempt to use the source topic's thread all the way through to the destination, bypassing the mailbox handoff. See the **Threading model** section for details.

---

### Supported Backends

The `pubsubClientType` field in `PubSubClientConfig` selects the backend plugin. The following types are supported:

#### `Kafka`

Apache Kafka producer/consumer via librdkafka.

**Client-level `CustomConfig`** notable fields:
- `KafkaProducerGlobalConfig` / `KafkaConsumerGlobalConfig` — key/value maps passed directly to librdkafka. All standard [librdkafka configuration properties](https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md) are supported here.

**Topic-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `topicName` | Kafka topic name. |
| `consumerModeStartOffset` | Where to start consuming: `"earliest"`, `"latest"`, or a numeric offset. |
| `addProducerHostnameAsKafkaMsgHeader` | Attach the producer hostname as a Kafka message header. |
| `compression.codec` | Compression codec for producer: `none`, `gzip`, `snappy`, `lz4`, `zstd`. |
| `partitionCount` | Number of partitions to use when producing. |

#### `MSMQ`

Microsoft Message Queuing.

**Topic-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `topicName` | MSMQ queue path (e.g. `.\private$\my-queue`). |
| `PropId` | Array of MSMQ property IDs to read/write. |

#### `STORAGE`

VFS-backed binary block storage. Designed for high-throughput message capture and replay (see the Storage Backend section above for full details).

**Client-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `vfsStorageRootPath` | VFS root path for storage files. Supports `{topicName}` substitution. |
| `dirTopicDiscoveryIsRecursive` | Discover topics by scanning subdirectories recursively. |
| `includeDirParentInTopicName` | Include parent directory name in derived topic name. |
| `destroyTopicWhenDirIsDeleted` | Remove topic when its backing directory is deleted. |

**Topic-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `vfsStorageRootPath` | VFS path root for this topic's storage files. |
| `vfsFileExtention` | File extension for capture files (e.g. `pubsubcapture`). |
| `desiredNrOfBuffers` | Number of in-memory I/O buffers. |
| `desiredMinimalSerializedBlockSize` | Minimum block size before flushing to disk (bytes). |
| `desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs` | Max time to wait for a block to reach target size before flushing. |
| `encodeCodecFamily` / `encodeCodecName` | Compression codec to apply when writing. |
| `decodeCodecFamily` / `decodeCodecName` | Decompression codec to apply when reading. |
| `autoPushAfterStartupIfStorageToPubSub` | Immediately start replaying stored messages after startup. |
| `autoPushAfterStartupIsPersistent` | Treat the auto-push replay as persistent (keep re-reading). |
| `deleteContainersWithFullyAckdContent` | Delete capture files once all messages are acknowledged. |
| `moveContainersWithFullyAckdContent` | Move acknowledged capture files to an archive path instead of deleting. |
| `vfsStorageRootPathForFullyAckdContainers` | Destination VFS path for acknowledged capture files. |
| `youngestStoragePubSubMsgFileToLoad` / `oldestStoragePubSubMsgFileToLoad` | Time-window filter for which capture files to replay. |
| `binarySerializerOptions` | Per-field flags controlling which message fields are stored in the binary format. |
| `bestEffortDeserializeIsAllowed` | Continue reading even if partial data corruption is detected. |

#### `ClusteredRedis`

Redis Streams via a clustered Redis deployment.

**Client-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `xaddMaxLen` | `MAXLEN` argument for `XADD` (trim stream length). |
| `xaddMaxLenIsApproximate` | Use `~` approximate trimming for performance. |
| `xreadCount` | Number of entries to fetch per `XREAD` call. |
| `xreadBlockTimeInMs` | Block timeout for `XREAD` (0 = no block). |
| `consumerGroupName` | Redis consumer group name. |
| `consumerName` | Redis consumer name within the group. |

#### `UDP`

Raw UDP socket transport.

**Client-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `udpStartPort` | Base UDP port number. |
| `udpPortChannelIdOffset` | Add channel ID to start port to derive the actual port per channel. |
| `nrOfUdpReceiveBuffersPerSocket` | Number of OS-level receive buffers per socket. |
| `maxUdpSocketUpdateCyclesPerPulse` | Max receive-loop iterations per pulse cycle. |
| `udpSocketOsReceiveBufferSize` | OS socket receive buffer size (bytes). |

#### `WEB`

HTTP/WebSocket transport.

**Client-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `httpPort` | Port to listen on or connect to. |
| `restBasicHealthUri` | URI for health probe endpoint. |

#### `AwsSqs`

Amazon SQS producer/consumer.

**Topic-level `CustomConfig`** notable fields:

| Field | Description |
|---|---|
| `topicName` | SQS queue name or URL. |
| `topicNameIsQueueName` | Treat the topic name as the SQS queue name directly. |
| `tryToUseSendMessageBatch` | Use `SendMessageBatch` API for higher throughput. |
| `addPrefixWhenSendingKvPairs` / `kvPairPrefixToAddOnSend` | Optionally prefix key names on message attributes. |
| `addPrefixWhenSendingMetaDataKvPairs` / `metaDatakvPairPrefixToAddOnSend` | Optionally prefix metadata key names. |

Credentials and region are configured in the top-level `AWSSDK` section of the app config.

#### `TEST`

A superset of the STORAGE backend that adds health simulation controls. Used for integration testing and chaos testing of flow router behavior.

**Topic-level `CustomConfig`** (in addition to all STORAGE fields):

| Field | Description |
|---|---|
| `defaultIsHealthyStatus` | Initial health state of the test side. |
| `toggleHealthyStatus` | Automatically toggle health at a configurable interval. |
| `healthyStatusToggleIntervalInMs` | How often to flip the health state (ms). |
| `healthyStatusToggleIntervalJitter` | Random jitter added to toggle interval (ms). |

---

### VFS Path Conventions

All file paths in configs use the VFS abstraction. Paths use the form `vfs://MountName/subpath` or just `MountName/subpath` (the `vfs://` prefix is optional in most fields).

Standard substitution variables available in path strings:

| Variable | Resolves to |
|---|---|
| `$MODULEDIR$` | Directory containing the executable |
| `$HOSTNAME$` | System hostname |
| `{topicName}` | Name of the pub/sub topic |
| `{clientType}` | Backend type name (e.g. `STORAGE`) |
| `{clientInstance}` | Instance identifier of the client |
| `{bookmarkNamespace}` | Bookmark namespace from the bookmark persistence config |
| `{channelID}` | Numeric channel ID |

---

### Tooling

#### JSON Schema

A JSON Schema (Draft 7) for channel template files is provided at:
```
config/channel_template.schema.json
```

It can be used with any JSON Schema-aware editor (VS Code, IntelliJ, etc.) to validate and get auto-completion when editing channel template files. The schema uses `if/then/else` conditional blocks on the `pubsubClientType` field to apply the correct `CustomConfig` structure per backend.

To associate the schema in VS Code, add to your workspace `settings.json`:
```json
{
    "json.schemas": [
        {
            "fileMatch": ["**/channel_templates/*.json"],
            "url": "./config/channel_template.schema.json"
        }
    ]
}
```

#### Visual Configurator

A no-code visual configurator is provided as a single self-contained HTML file:
```
config/configurator.html
```

Open it directly in any browser (including from a `file://` path — no web server required). Features:

- **Channel Templates tab** — create, edit, and export channel template JSON files with a form-based UI. Backend-specific fields are dynamically shown based on the selected `pubsubClientType`. Supports importing from local files or fetching from a running service via REST API.
- **App Config tab** — edit the full `pubsub2pubsub_d.json` app config including VFS mounts, StatsD settings, AWS credentials, channel template references, and channel instances.
- **Live JSON preview** — syntax-highlighted JSON output updates as you type.
- **Export options** — copy to clipboard, download as JSON, or push directly to a running service via `PUT /config`.
- **REST API integration** — connect to a running service at a configurable host/port to fetch the live config or push updated configs.

---

### REST API

When `enableRestApi` is true the service exposes an HTTP API on `restApiPort` (default: 10000).

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/info` | Service version and build info. |
| `GET` | `/config` | Full current app config as JSON. |
| `PUT` | `/config` | Replace the full app config. Body: JSON, `Content-Type: application/json`. |
| `GET` | `/channels` | Live channel status and metrics. |
| `GET` | `/config/channels?view=index` | Channel config index. |
| `GET` | `/config/appargs` | Application arguments section of the config. |
| `GET` | `/health` | Detailed health status. |
| `GET` | `/health/basic` | Lightweight health probe (suitable for load balancer checks). |

A Postman collection covering all endpoints is provided at:
```
pubsub2pubsub.postman_collection.json
```
