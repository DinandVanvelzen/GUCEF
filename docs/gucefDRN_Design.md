# gucefDRN

Module `platform/gucefDRN`, namespace `GUCEF::DRN`, files `gucefDRN_*.h/.cpp`. DRN, the Data Replication Network, keeps data in step between the parties of a session: two peers, a server with its clients, or a mesh of peers. The application publishes data items and subscribes to those of others. DRN hides how the data travels: over which channel (TCP, UDP, WebSocket, a relay), with which reliability, in which topology. Depends on gucefMT, gucefCORE, gucefCOMCORE, gucefCOM and gucefWEB. Tests: `tests/gucefDRN_TestApp`.

## 1. Scope

| supported | can be added without breaking the interface |
|---|---|
| one protocol over every channel, with sequence numbers, transmit timestamps and acks on every packet | compression |
| channels `tcp`, `udp` (with its own reliability), `websocket`, `relay`; more as plugins | WebRTC data channels, SteamNetworkingSockets, EOS P2P (channel plugins) |
| sessions in CLIENT_SERVER and PEER_TO_PEER topology, chosen by a setting | migration of the server / sequencer to another peer |
| data grids: last-one-wins key / value data, delta compressed per receiver, unreliable or reliable | interpolation, prediction (left to the game, DRN gives tick and time) |
| stream slots: ordered, never dropped entries of variants per peer | |
| authority per grid / key: host, owner, proposal + vote | interest management: gucefSMDRN builds it on DRN's subscriptions |
| latency measurement per link with a clock offset estimate | |
| bulk transfers of large blobs (a whole save game) | |
| authentication with tokens (Ed25519 signed or HMAC), shared secrets or none | |
| encrypted links: a key exchange bound to the transcript, the acceptor's static key pinned, every packet protected with an AEAD cipher of the codec registry (11.11) | key exchange plugins other than x25519 |
| a security policy set by the game: encryption, receive limits, handshake limits, UDP cookies, bans, roles, anomaly reports, telemetry, session recording (11) | |

## 2. Layers

```
 application ─► CDRNSession ─► CDRNDataGrid / CDRNDataStream (items, subscriptions, authority, votes)
                     │
                     ▼
                CDRNNode ─► CDRNPeerLink (handshake, auth, lanes, latency) ─► CDRNPacketCodec (protocol)
                     │                         │
                     │                         ▼
                     │                  CIDRNChannel ◄── channel factory: tcp, udp, websocket, relay, plugins
                     ├─► CIDRNChannelListener
                     ├─► CDRNConnector (endpoint candidates)
                     └─► CDRNRelayService (optional)
```

Everything runs on the pulse generator of the node (`CDRNNode` takes one; by default the global one). A node can be given a pulse generator of its own, driven by a thread of its own, so that network timing does not depend on the frame rate of the application (see 9).

## 3. Channels

A channel moves DRN packets between two endpoints. It adds nothing to them and interprets nothing: framing for stream transports is the only thing it does.

* `CIDRNChannel : CORE::CObservingNotifier`
  * `bool Connect( const CDRNEndpoint& remote )`, `void Close( void )`, `bool IsConnected( void ) const`
  * `bool SendPacket( const void* data, UInt32 size )`: one whole DRN packet
  * `const CDRNChannelCapabilities& GetCapabilities( void ) const`
  * `CDRNEndpoint GetRemoteEndpoint( void ) const`, `CString GetChannelType( void ) const`
  * `CIDRNLatencyMeter* GetLatencyMeter( void )`: a channel specific meter, or NULL for the protocol's own (see 7)
  * events `ChannelConnectedEvent`, `ChannelDisconnectedEvent`, `PacketReceivedEvent` (`CDRNPacketEventData`: a `CDynamicBuffer` with the packet), `ChannelErrorEvent`
* `CDRNChannelCapabilities`: `isReliable` (the transport itself never loses or reorders: TCP, WebSocket), `isDatagram`, `maxPacketSize` (the MTU budget of a datagram channel; 0 for streams), `isBrowserCompatible`, `canListen`.
* `CIDRNChannelListener : CORE::CObservingNotifier`: `Listen( const CDRNEndpoint& local )`, `StopListening()`, `GetListenEndpoint()`, event `ChannelAcceptedEvent` (`CDRNChannelEventData` with a `CIDRNChannelPtr`).
* `CDRNEndpoint`: `channelType`, `address` (host:port, ws URL or relay id), `CORE::CValueList params`; parses and writes the URI form `tcp://host:port`, `udp://host:port`, `ws://host:port/path`, `relay://relayhost:port/session/peerId`.
* `CDRNEndpointList`: a prioritised list of endpoints, serialisable to a string and to a `CDataNode`.
* `CDRNChannelFactory` and `CDRNChannelListenerFactory`: `CORE::CTAbstractFactory< CString, … >` keyed by channel type, owned by `CDRNGlobal`. Plugins register more types.
* The URI scheme `ws` is channel type `websocket`; a channel type of a plugin or decorator is its own scheme (`lossy-udp://…`). Endpoint lists are `;` separated. `PacketReceivedEvent` links the packet: observers read it through a const pointer (a non-const `GetData()` makes a private copy).

Built in channels:

| type | over | framing | notes |
|---|---|---|---|
| `tcp` | `COMCORE::CTCPClientSocket` / `CTCPServerSocket` connections | `[0x47 0x44]['GD'][varint length][packet]`, packets up to 16 MB, partial reads accumulated | reliable stream |
| `udp` | `COMCORE::CUDPSocket` | one packet = one datagram (`maxPacketSize` 1200) | the reliability for reliable lanes is done by the link layer (6); stateless cookies before a listener keeps state (11.2) |
| `websocket` | `WEB::CWebSocketClient` / `CWebSocketServer` | one packet = one binary message | reliable; browser compatible |
| `relay` | another channel to a `CDRNRelayService` | the relay wraps packets in RELAY messages (10) | reliability of the carrying channel |

A test only decorator, `CDRNLossyChannel` (in the test app), wraps a channel and drops, delays, duplicates and reorders packets. It is the channel type `lossy-<inner type>` with endpoint parameters `loss`, `dup`, `reorder`, `delay`, `jitter`; when it loses, duplicates or reorders it reports an unreliable datagram channel, so links over it use their own reliability even over TCP.

## 4. The protocol

The protocol is the same over every channel. Every packet:

```
packet  := header message*
header  := u8 protocol (0xD1 = a plain body, 0xD4 / 0xD5 = an encrypted body in key phase 0 / 1, see 11.11)
           varint sequence            per link, per direction; wraps at 2^32, compared wrap-safe
           varint sendTime            microseconds of the sender's link clock since the link epoch (set at the handshake)
           varint ackSequence         highest sequence received from the peer (0 before any)
           u32    ackBits             bit n = sequence ackSequence-1-n was received
           varint echoTime            the sendTime of the packet with ackSequence, as the peer sent it
           varint echoHold            microseconds between receiving that packet and sending this one
message := u8 class, varint opcode, u8 lane, [lane-sequence varint if the lane is reliable], varint length, payload
```

All integers that are not varints are big endian. Varints are unsigned LEB128; signed values are zig-zag encoded. `CDRNWireWriter` / `CDRNWireReader` over `CORE::CDynamicBuffer` do the encoding, with no allocation once the buffer is large enough.

What the header gives:
* **loss and order**: gaps in `sequence` are lost packets, a lower sequence than seen is reordered or a duplicate (dropped when already received: a window of 1024 sequences is remembered);
* **acks**: `ackSequence` + `ackBits` acknowledge the last 33 packets at once, piggybacked on every packet;
* **latency**: the round trip time of a packet is `now - echoTime - echoHold`, measured by the sender of the original packet, on every channel, without extra traffic;
* **clock offset**: with the RTT and the peer's `sendTime`, `offset = peerSendTime + rtt/2 - now` (filtered, see 7).
* Sequences start at 1 (`CDRNNodeSettings::firstSequence`, for testing the wrap) and skip 0 at the wrap, as 0 means "no ack"; lane 2 drops a message whose packet is older than the newest packet that delivered on lane 2 (per link).

Lanes:

| lane | delivery | used for |
|---|---|---|
| 0 `LANE_RELIABLE_ORDERED` | every message, in order | control, stream slots, reliable grid keys, bulk |
| 1 `LANE_RELIABLE_UNORDERED` | every message, any order | bulk chunks |
| 2 `LANE_UNRELIABLE_SEQUENCED` | newest only, older ones dropped | data grid deltas |
| 3 `LANE_UNRELIABLE` | as it comes | application datagrams |

On a channel whose transport is reliable (`isReliable`) the reliable lanes need nothing more. On a datagram channel the link keeps every message of a reliable lane until a packet that carried it is acked, and resends it in the next packet when a packet that carried it is known lost (a later sequence was acked and the gap is older than the RTT) or after the retransmission timeout (2 x smoothed RTT, at least 50 ms). Received reliable messages carry a lane sequence; ordered lanes deliver in that order, holding later ones back.

Messages larger than a datagram's budget are split in `FRAGMENT` messages (class LINK) on a reliable lane and joined on arrival; unreliable messages are never fragmented (the sender keeps them within the budget). FRAGMENT is `{varint group, index, count, totalSize, offset, u8 class, varint opcode, bytes}`. On datagram channels a header-only packet acknowledges received reliable messages after `ackDelayMs` (default 5) when nothing else goes out; the ECHO interval defaults to 250 ms, the link timeout to 10 s; CLOSE and REFUSE get `closeGraceMs` (100) to leave before the channel closes.

Message classes and opcodes (`gucefDRN_DRNProtocol.h`):

| class | opcodes |
|---|---|
| 1 LINK | HELLO, CHALLENGE, AUTH, WELCOME, REFUSE, CLOSE, ECHO, FRAGMENT |
| 2 SESSION | JOIN, JOINED, LEAVE, MEMBER_ADDED, MEMBER_REMOVED, MEMBER_LIST, CONNECT_TO (peer to peer: endpoints of a member to link to) |
| 3 REPL | ITEM_ANNOUNCE (id, name, kind, settings), ITEM_REMOVED, SUBSCRIBE, UNSUBSCRIBE, SNAPSHOT_BEGIN, SNAPSHOT_END, KEY_DEFINE (dense index ↔ variant key), GRID_DELTA, GRID_ACK, GRID_PROPOSE, PROPOSE_RESULT, VOTE_START, VOTE_CAST, VOTE_END |
| 4 STREAM | ENTRIES, CREDIT (flow control) |
| 5 BULK | OFFER, ACCEPT, CHUNK, DONE, CANCEL |
| 6 RELAY | BIND, FORWARD, UNBIND |
| 7 APP | opaque messages of the application (`CDRNSession::SendAppMessage`) |

## 5. Links and nodes

* `CDRNPeerLink` (name kept): one authenticated link over one channel.
  * Handshake (protocol 2, 11.11), each step with a timeout (default 5 s):
    1. HELLO `{protocol major, minor, feature bits, node id, service name, link epoch, policy hash, ephemeral key, ciphers on offer}`. A different major, service or policy hash is refused.
    2. CHALLENGE `{nonce, node id, minor, authentication method, policy hash, ephemeral key, static key, chosen cipher}`
    3. AUTH `{method, proof}`, encrypted when a cipher was chosen
    4. WELCOME `{link id, link epoch}`, the first message the acceptor encrypts, or REFUSE `{reason}`.
  * After WELCOME the link is operational: `LinkOperationalEvent`.
  * WELCOME carries the id the accepting node gives the link (the session's peer id comes with JOINED); HELLO and WELCOME carry each side's link epoch on its node clock (`CDRNClock`, process wide monotonic microseconds), so either side can estimate the other's node clock. CHALLENGE also carries the accepting node id, minor version and authentication method.
  * Then: `Send( lane, class, opcode, payload )`, the reliability of 4 on datagram channels, ECHO messages only when nothing else was sent for the echo interval, the latency meter (7).
  * Events `LinkOperationalEvent`, `LinkClosedEvent` (with reason), `LinkRefusedEvent`, `MessageReceivedEvent`, `LatencyUpdateEvent`, and the reports of 11.6 (`PeerLimitExceededEvent`, `ClockAnomalyEvent`, `TrafficAnomalyEvent`).
  * `GetLatencyInfo()`, `GetEstimatedRemoteTime()`, `GetStats()` (packets, bytes, loss, resends), `GetTelemetry()` (11.7).
* `CDRNNode`: owns listeners, links, the connector, the relay service when enabled, and the sessions.
  * `Listen( const CDRNEndpoint& )`, `Connect( const CDRNEndpointList& )` (through the connector), `GetLinks()`.
  * `CDRNNodeSettings` (`CORE::CIConfigurable`): node id, service name, listeners, relay on/off, echo interval, latency update interval, handshake timeout, the authenticator type and its settings, pulse interval, the security policy (11).
* `CDRNConnector`: tries the endpoints of a list in priority order, the next one when a candidate has not reached WELCOME after its timeout (default 2 s) or failed, and keeps the first link that is operational.
* **Authentication** (`CIDRNAuthenticator`, factory keyed by name):
  * `anonymous`;
  * `shared-secret`: HMAC-SHA256 of the nonce with the secret, the LAN password;
  * `token`: a token `{session, user id, role, expiry, policy hash}` issued by a lobby service: signed with the issuer's Ed25519 key and checked with its public key (members cannot make tokens), or with HMAC-SHA256 and a key the session host has.
  * The proof never contains the secret.
  * The shared-secret proof covers the nonce, the transcript hash of the handshake (11.11), the service name and both node ids. A refusal ends a connector, the other endpoints are not tried.
  * Tokens, base64 as text, settings `token` (connecting side), `issuerPublicKey` (hex: only signed tokens are taken) or `key` (HMAC tokens), `expectedSession`:
    * signed (`IssueSignedToken`, version 2): `{u8 2, string session, string userId, string role, varint expiry (Unix seconds), bytes userPublicKey (0 or 32), bytes policyHash (0 or 32)}` followed by the 64 byte Ed25519 signature;
    * HMAC (`IssueToken`, version 3): `{u8 3, string session, string userId, string role, varint expiry, bytes policyHash (0 or 32)}` followed by the 32 byte HMAC-SHA256; version 1 (the same without the policy hash) is still read;
    * `policyHash` is `CDRNSecurityPolicy::GetPolicyHash()` of the game the token is issued for. The checking node compares it with its own: another hash is refused ("the token is for another security policy"); a token without one is refused when its policy has `requireTokenPolicyHash` ("the token has no security policy hash"), taken otherwise;
    * `userPublicKey` comes out as `CDRNAuthResult::publicKey` and is checked against the ban list (`DRN_BAN_PUBLIC_KEY`).
* **Access**: `CIDRNAccessPolicy`: `MaySubscribe( peer, item )`, `MayWrite( peer, item, keyIndex )`. Every subscribe and write is checked. The default allows subscribing and leaves writes to the authority modes (8.3). `MayJoin( link, session )` is checked on JOIN.

## 6. Sessions and topology

`CDRNSession` is a named scope of replication: one lobby, one game. The topology is only a setting in `CDRNSessionSettings` (`CORE::CIConfigurable`). Library users publish, subscribe and get events the same way for every topology; DRN does the routing.

* **CLIENT_SERVER**: one designated server peer (dedicated, or a player hosting). Clients link only to the server. The server is the authority for host-authoritative items, the sequencer, and forwards what clients publish to the other subscribers.
* **PEER_TO_PEER**: every member links to every member. The creator of the session is the sequencer for host-authoritative items and votes. A joining member gets the member list with endpoints (MEMBER_LIST / CONNECT_TO) and links to every member. Owner-authoritative items go directly from owner to subscribers.
* Any link may run relayed (10): the session does not see the difference.

Session API:
* `Host( settings )` / `Join( const CDRNEndpointList& host, credentials )`, `Leave()`;
* `GetMembers()`, `GetLocalPeerId()`, `IsSequencer()`;
* `CreateGrid( name, gridSettings )`, `CreateStream( name, streamSettings )`, `Subscribe( itemName [, ownerPeer] )`, `Unsubscribe( … )`;
* `SendAppMessage( peer | all, lane, buffer )`;
* `Tick()`: called by the node's pulse; flushes the due grids and streams;
* events `MemberJoinedEvent`, `MemberLeftEvent`, `ItemAnnouncedEvent`, `ItemRemovedEvent`, `SessionClosedEvent`, `AppMessageEvent`.

A peer id is a `UInt32` given by the server / sequencer at JOINED; peer 0 is "none".

Sessions are created by and owned by the node (`CDRNNode::CreateSession`). The host decides the topology and sends it in JOINED (`{result, reason, peerId, flags, topology, sequencerPeerId, sessionEpoch}`, a result other than OK is the refusal). In PEER_TO_PEER the joiner links to the members named by CONNECT_TO and sends JOIN with the MESH flag over each such link; until a member answers, messages for it go through the sequencer. APP messages carry `{fromPeer, toPeer, flags}` (FORWARD: the sequencer passes it on; TO_ALL: routed to one member but sent to all). Grids, streams and bulk transfers plug in as a `CIDRNSessionComponent` per message class.

## 7. Latency

* `CIDRNLatencyMeter`: `OnPacketSent`, `OnPacketReceived`, `GetLatencyInfo()`. The default meter of a link uses the packet header (echoTime / echoHold) on every channel. A channel can supply its own (a SteamNetworkingSockets channel would report the SDK's figures; the websocket channel can add WS ping / pong samples).
* `CDRNLatencyInfo`: smoothed RTT (EWMA 1/8), RTT variance (jitter, EWMA 1/4), minimum RTT of the last 10 s, loss ratio of the last 256 packets, clock offset to the peer, sample time.
* The link sends `LatencyUpdateEvent` every `latencyUpdateIntervalMs` (default 1000) through the GUCEF notification system.
* **Clock offset**: the offset of each sample is `peerSendTime + rtt/2 - localReceiveTime`. The estimate is the offset of the sample with the lowest RTT among the last 8 (the least delayed), smoothed. `GetEstimatedRemoteTime( localTime )` uses it. The session clock is the server's / sequencer's time: `CDRNSession::GetSessionTime()`.
* With `CDRNSessionSettings::publishLatency` the sequencer publishes the smoothed RTT of every member (UInt32 microseconds, keyed by peer id) in the HOST grid `drn.latency` every `latencyPublishIntervalMs` (default 1000).

## 8. Replicated data

### 8.1 Data grids: last one wins

`CDRNDataGrid` (replaces `CDRNDataGroup`) is a grid of `CVariant` key → `CVariant` value, one value per key; the newest value wins and older values may be dropped on the way.

* `CDRNDataGridSettings : CORE::CIConfigurable` (`SaveConfig` / `LoadConfig`, section `DRNDataGridSettings`). A game normally fills it in code; loading it from data nodes is supported. It holds:
  * `name`
  * `authority`: HOST / OWNER / PROPOSAL_VOTE
  * `reliable` (default false: deltas on lane 2), plus `reliableKeys` (keys whose changes go reliable)
  * `sendRateHz` (default 20; 0 = every tick) and `maxBytesPerTick`
  * `deltaAgainstAck` (default true)
  * `capacity` (keys preallocated, default 64; grows when exceeded, outside the hot path)
  * `quantisation`: per key `floatPrecision` (decimal places, -1 = full) and `intBits`
  * `notify`: PER_CHANGE / PER_PACKET / POLL
  * `priority`
  * `snapshotOnSubscribe` (default true)
  * vote policy (8.3)
* Reading and writing:
  * `SetValue( const CVariant& key, const CVariant& value )`, `GetValue( key, CVariant& )`, `HasKey`, `RemoveKey`;
  * `GetKeyIndex( key )` and `SetValueByIndex( index, value )` / `GetValueByIndex` for the hot path;
  * `GetKeyCount`, `GetKeys`;
  * `GetLastChangeTick( index )`, `GetLastChangeSendTime( index )`.
* It is a `CORE::CIDataNodeSerializable`: `Serialize` writes every key, value, revision and owner to a `CDataNode` (for debug dumps, JSON / XML); `Deserialize` reads it back. This is never used on the wire.
* Events (by `notify`):
  * `GridKeyChangedEvent` (key index): PER_CHANGE;
  * `GridUpdatedEvent` with the changed key indices: PER_PACKET;
  * none in POLL, where the game reads `GetChangedSince( tick )`.

Efficiency rules (the grid has to serve an FPS game):
* **Keys**: a key's variant is mapped once to a dense index. KEY_DEFINE travels once per subscriber (reliable); deltas carry only varint indices.
* **Storage**: values in a flat array indexed by key index, with `UInt32 revision` and `UInt32 changeTick` per key. Dirty keys are a bitset per subscriber relative to its acked baseline.
* **Per subscriber**: the tick it last acked (GRID_ACK carries the highest applied delta tick). A delta holds every key whose `changeTick` is newer than that baseline: when a packet is lost the next delta simply covers more keys, nothing is resent. With `deltaAgainstAck` off, a delta holds the keys changed since the previous send.
* **Encoding**: GRID_DELTA `{item id, tick, sendTime, count, (key index, value)*}`. Values are written by `CDRNVariantCodec`, compact per type: varint ints, zig-zag signed, floats quantised when asked, strings length-prefixed.
* **Allocation**: none on the hot path. Packet buffers come from a `CORE::CTFreeList` per link; grid storage is preallocated (`capacity`).
* **Packing**: every grid and stream of a session that is due this tick is packed into the link's next packet(s), within `maxPacketSize` on datagram channels. When the budget per tick is short, a priority accumulator per item decides who goes first.
* **Received state**: a received delta is applied to a back buffer and swapped at the end of the tick, so the game always reads a consistent state of one tick (when the node runs on a thread of its own, the swap is the hand-over point; reads take the grid's lock for the copy of the front buffer only).
* Unreliable deltas go on lane 3, not the sequenced lane 2 (a newer delta of another item must not make the link drop this one); a receiver applies a key's value only when the delta that brings it is newer than the one that brought the value it has. GRID_DELTA is `{seq, u8 flags, [baseSeq], sendTime, u16 count, (index, [revision, writer], value)*}`, GRID_ACK `{highest seq, bits of the 32 before}`.
* Per subscriber and key the publisher keeps the change it sent, the delta that carried it and the change acknowledged: an acknowledged delta settles its keys, a newer acknowledged delta or a timeout (2 RTT + 4 jitter + 2 send intervals, at least 100 ms) counts it lost and its keys go again; this replaces the per subscriber dirty bitset.
* SUBSCRIBE is `{flags (RESYNC), u32 first seq}`: the replica picks the first sequence beyond any it saw, so deltas of an earlier subscription are recognised as stale; the snapshot delta has that sequence. KEY_DEFINE is `{u16 count, (index, u8 flags (reliable), float precision, int bits, key)*}`; a delta that names an index not defined yet is not acknowledged and comes again.
* Unreliable deltas carry no revision (the replica counts them); reliable deltas and snapshots carry revision and writer. A replica serves its own subscribers (the server in CLIENT_SERVER) once it has the whole state itself.
* Delta messages stay within the link's unfragmented payload (at most 16 KB); per tick the items that are due send the most starved first (accumulated priority) within `CDRNSessionSettings::maxBytesPerTickPerLink` and the grid's `maxBytesPerTick`.
* Writing, sending, receiving, applying and handing over numeric values does not allocate; the GUCEF notifier delivers an event without allocating for up to 64 observers. The publisher's own grid sends the change events too, its own writes included.

### 8.2 Stream slots: every entry, in order

`CDRNDataStream` (reworked) is an addressable item that is a stream of entries, not one value.

* An entry is one segment of a `CORE::CVariantStream`, written with `<<` and closed with `WriteSegmentEnd()`. It can hold any number of variants, e.g. key / value pairs: what goes where is the application's choice.
* The publishing peer: `AppendEntry( const CVariantStream& segment )`, or `GetWriteStream()` + `CommitEntry()`.
* Entries travel on lane 0 (reliable, ordered) and are never dropped. They are packed into the same per-tick packets as the grids.
* The receiving side queues them per (publisher peer, slot):
  * `GetEntryCount()`
  * `ReadNextEntry( CVariantStream& )`, which reads one segment (`ReadNextSegment`)
  * `PeekEntry( index, CVariantStream& )`, `DiscardEntries( count )`
  * `EntriesAvailableEvent` (with the count)
* `CDRNDataStreamSettings : CORE::CIConfigurable`:
  * `name`
  * `backlogLimit` (entries a receiver may have unread before the sender is held back: the receiver grants CREDIT)
  * `retainedHistory` (the last N entries kept by the publisher and sent to a late subscriber, e.g. chat)
  * `priority`
  * `notify`
* A stream can be dumped to a `CDataNode` (its queued entries) for debugging.
* Streams are per peer: every peer can publish slots; subscribers address them as (peer, slot name).
* ENTRIES `{first seq, u16 count, sized entries}` and CREDIT `{credit seq}` travel on lane 0; SUBSCRIBE carries `{from seq (0 = the retained history), window}` and is answered with SNAPSHOT_BEGIN `{first seq}` and SNAPSHOT_END `{next seq}`. `AppendEntry()` refuses while the slowest subscriber has `backlogLimit` entries not sent; `EntriesAvailableEvent` comes once per tick for PER_CHANGE and PER_PACKET.

### 8.3 Authority, sequencing and votes

* **HOST** (the default): only the server / sequencer writes. Another member's `SetValue` becomes GRID_PROPOSE. The sequencer checks it with the session's `CIDRNWriteValidator` (default: accept) and the access policy, applies it or answers PROPOSE_RESULT with a reason. `ProposeResultEvent` tells the proposer.
* **OWNER**: the grid belongs to the member that created it. Only the owner writes; the server forwards in CLIENT_SERVER, the owner sends directly in PEER_TO_PEER.
* **PROPOSAL_VOTE**: a write by anyone but the sequencer becomes a vote:
  1. The sequencer sends VOTE_START `{vote id, item, key, value, proposer, deadline}`.
  2. Members cast VOTE_CAST.
  3. The sequencer ends it with VOTE_END `{passed / failed / cancelled / timeout}` and applies the value when it passed.
  * `CDRNVotePolicy`: majority / unanimous / quorum fraction, timeout, whether the proposer's vote counts. A vote with no eligible voters passes.
  * The sequencer's own writes apply directly (the boss case).
* **Revisions**: every applied write gives the key `revision + 1`, decided by the sequencer (HOST, PROPOSAL_VOTE) or the owner (OWNER). Receivers keep, per item, the highest delta tick; reliable deltas carry `baseTick` and a receiver that finds a gap asks for a snapshot.
* **Late join and subscribing**: SUBSCRIBE is answered with SNAPSHOT_BEGIN, KEY_DEFINE for every key, one GRID_DELTA with every key (its tick is the baseline), the retained stream history, SNAPSHOT_END. Updates that arrive during the snapshot are buffered and applied after it when newer. `SnapshotCompleteEvent`.
* UNSUBSCRIBE stops all updates of an item to that peer; updates only ever go to subscribers.
* Every REPL and STREAM message starts with `{owner peer id, item id}`; ITEM_ANNOUNCE is `{owner, id, name, u8 kind, settings}`. Announcements go through the sequencer, which tells a joining member about every item. A replica subscribes to the owner when it has a direct link to it, else to the sequencer, which keeps a replica and serves the subscribers from it (always so in CLIENT_SERVER); it unsubscribes upstream when neither the application nor another member wants it.
* HOST and PROPOSAL_VOTE grids are created by the sequencer only. The voters are the members subscribed to the grid (the proposer only when `proposerVoteCounts`, its vote counting as yes). VOTE_START is `{vote id, proposer, key, value, timeout}`, VOTE_CAST `{vote id, u8 yes}`, VOTE_END `{vote id, outcome}`; votes are events of the grid (`VoteStartedEvent`, `VoteEndedEvent`, `CastVote()`).
* The items of a member that leaves are removed. Item objects stay valid (flagged removed) until the session is hosted or joined again or destroyed.

### 8.4 Bulk transfers

`CDRNBulkTransfer`: OFFER `{transfer id, name, size, SHA-256}`, ACCEPT `{resume offset}`, CHUNK `{offset, bytes}` (16 KB chunks on lane 1), DONE, CANCEL. `ProgressEvent`, `CompletedEvent` (the hash checked), `FailedEvent`. Hashing runs as a pool task (`CFutureResult`) so that a large blob does not stall the pulse.

BULK messages carry `{from, to, sender, transfer id}` and go through the sequencer when there is no direct link. A transfer sends at most 8 chunks per tick and only while the link has fewer than 64 unacknowledged reliable messages. The receiver accepts into a file (resuming from the size it has) or into memory; DONE `{u8 hash ok}` comes from the receiver once it hashed what arrived. `CDRNBulkTransferManager` is the session's BULK component (`IncomingTransferEvent`, `OfferFile`, `OfferData`).

## 9. Threading

* By default everything runs on the global pulse generator.
* `CDRNNodeSettings::ownThread = true` makes the node create its own `CPulseGenerator` and drive it on a thread of its own (an `MT::CActiveObject`). Then:
  * the sockets, links and session ticks run there;
  * events to the application are delivered on the application's pulse thread, as `CORE::CTSGNotifier` does;
  * grids swap their received state at the end of a tick under their lock;
  * stream queues are locked.
* The node and its sessions are `CORE::CTSGNotifier`s: with a thread of their own their events are mailed to the application's pulse generator, without they are sent directly. Calls into the node and its sessions take the node lock, which the node's thread holds while it pulses. Links, channels and connectors send their events from the node's thread.
* Channels are destroyed on the node's thread: a link let go of elsewhere (its last reference in mailed event data) hands its channel to the node's `CDRNChannelReaper`. COMCORE sends with MSG_NOSIGNAL (SO_NOSIGPIPE where that is the option), so a peer going away during a send does not end the process.

## 10. Relay and reachability

* `CDRNRelayService` (enabled in `CDRNNodeSettings`): peers BIND `{session, peer id, token}` over any channel. FORWARD `{to peer, packet}` is passed to the bound peer as is, so the end-to-end packet header (sequence, times, acks) stays intact. There is a per-session bandwidth cap.
* The `relay` channel: a channel whose packets are FORWARD messages over a link to a relay service. To the peer link it is just a channel.
* `CDRNReachabilityManager`:
  * asks the configured COMCORE port mappers (`natpmp` built in, `upnp` plugin) for mappings of the node's UDP / TCP listeners;
  * asks COM's STUN client for the reflexive address;
  * builds the node's `CDRNEndpointList`: LAN addresses, mapped public addresses, the relay as the last resort.
* The order a connector tries: LAN direct, mapped public, relay. STUN-based hole punching (both sides sending at an agreed time, coordinated through the session's server or a lobby service) can be added as a connector strategy.
* BIND `{realm, peer id (0 = assign), token}` is answered with BIND `{result (OK, ID_TAKEN, REFUSED, NO_SERVICE), peer id, realm}`; one binding per link; FORWARD keeps the lane it came on. Peer ids 1 to 32767 are the fixed ids a node asks for (a session member binding as its session peer id, a host as 1); the relay assigns ids from 32768 to 65535, so a node that binds without an id never takes a fixed id another node binds with later. A bind for an id whose binding has a link that closed, or that has been silent for four echo intervals of the relay and at least a second, takes the id over. Settings `relayToken` and `relayMaxBytesPerSecondPerRealm` (a token bucket of one second).
* A relay endpoint is `relay://<relay host:port>/<realm>/<peer id>?via=<channel type to the relay>`. A node binds when asked (`CDRNNode::BindAtRelay`) and its relay endpoints follow its listen endpoints; a node that connects to a relay endpoint binds itself (an id from the relay) to talk through it; a FORWARD from a peer id without a channel is a new connection, accepted as from a listener. The relay channel's datagram budget is the relay link's unfragmented payload minus the FORWARD header, at most 1200 bytes.
* **The relay's own policy**: the link to a relay service is infrastructure, the links through it are end to end. A node publishes the hash of its policy in its published endpoints (`policy=<hex>`, beside `key=<hex>`). `BindAtRelay` with such an endpoint makes the link to the relay run under the relay's policy hash (`CDRNConnector::SetServicePolicyHash`, only the relay client uses it) and pins the relay's key; without the parameter the node's own policy is used, and a relay with another policy refuses it. The relay endpoints of the bound node carry `relayPolicy` and `relayKey`, so a peer that connects through the relay links to the relay the same way. The links through the relay check the game's policy hash end to end: games with different policies share one relay, and a node of another game is refused by the host it tries to reach.
* `CDRNReachabilityManager` takes `CDRNReachabilitySettings` (port mappers by factory name with their config, STUN servers, relays and realm, lease, timeout) and builds LAN, mapped, reflexive, relay without doubles; the connector puts relay endpoints last whatever the order of the list.

## 11. Security

The game decides what is acceptable; DRN offers the mechanisms, enforces the limits and reports what it sees. Nothing here is a player setting.

### 11.1 The security policy

`CDRNSecurityPolicy` (`CORE::CIConfigurable`, section `DRNSecurityPolicy`, children `Limits` and `Anomalies`) is set by the game in code or in its config, in `CDRNNodeSettings::securityPolicy` (the links: limits, channel types, authenticators, anomalies) and `CDRNSessionSettings::securityPolicy` (the session: relay only, encryption, which links may join). A game uses one policy for both.

| field | default | meaning |
|---|---|---|
| `requireEncryption` | true | every link encrypted; without it links are still encrypted when both sides can, and an empty `ciphers` makes them plain |
| `requireSignedTokens` | false | token authenticators take only Ed25519 signed tokens: a checking node needs `issuerPublicKey` |
| `requireTokenPolicyHash` | false | a token without the policy hash is refused (one with another hash always is) |
| `ciphers` | `xchacha20-poly1305`, `chacha20-poly1305`, `aes-256-gcm` | acceptable ciphers, most wanted first |
| `allowedChannelTypes` | empty = all | channel types links may use: refused at listen, connect and accept |
| `relayOnly` | false | only relay endpoints are advertised, passed on and connected to; members' addresses are never revealed |
| `allowedAuthenticators` | empty = all | authenticator types: the node's own (`SetSettings`), a connector's, a join's credentials |
| `statelessCookies` | true | UDP listeners answer unknown addresses with a cookie (11.2) |
| `limits.maxMessageSize` | 16 MB | largest message sent (`Send` refuses) or received (the link closes) |
| `limits.maxPreAuthPacketSize` | 16 KB | largest packet a stream channel collects before the link is operational |
| `limits.maxMessagesPerSecond`, `maxBytesPerSecond` | 20000, 64 MB | received per peer (11.3) |
| `limits.sustainedLimitSeconds` | 5 | a peer over a receive limit this many seconds in a row is closed |
| `limits.maxPendingHandshakes` | 256 | accepted links in their handshake, per node |
| `limits.maxHandshakesPerAddressPerMinute` | 120 | accepted channels per remote address (host without port; the relay address of a relayed peer) |
| `limits.maxFragmentGroups`, `maxFragmentBytes` | 64, 32 MB | messages being joined from fragments, per link, and their total size |
| `anomalies.*` | see 11.6 | thresholds of the reports; 0 switches a check off |

* `IsWeakerThan( minimum, why )`: whether a policy is weaker than a minimum (a service that hosts games refuses weaker ones): it does not require what the minimum requires (encryption, signed tokens, token policy hashes, relay only, stateless cookies), allows a cipher, channel type or authenticator the minimum does not (an empty set allows any), or has a limit above the minimum's (0 is no limit; 0 in the minimum asks for none). Anomaly thresholds are reports and are not compared.
* `CanBeHonoured( whyNot )` is false when the policy asks for what the codec registry does not have: `requireEncryption` needs the `x25519` key agreement codec and a codec for one of the ciphers (`IsPacketProtectionAvailable()`), `requireSignedTokens` the `ed25519` signature codec (both come from `codecspluginMONOCYPHER` or `codecspluginOPENSSL`, the AEAD ciphers also from the OS). `CDRNNode::Start` / `Listen`, `CDRNSession::Host` / `Join` and connectors refuse with that reason (`DRN_LINKREASON_POLICY`). An application loads the plugin before it starts its nodes; a test that needs plain links sets `requireEncryption` false and clears `ciphers` on every node.
* **Policy hash**: `GetPolicyHash()` is SHA-256 over the canonical encoding (DRN wire encodings): string `"DRNSecurityPolicy"`, u8 encoding version 2, u8 `requireEncryption`, u8 `requireSignedTokens`, u8 `requireTokenPolicyHash`, u8 `relayOnly`, u8 `statelessCookies`, varint count + strings of the ciphers in their order, of the channel types and of the authenticators sorted, all lowercased and trimmed, then every limit and every anomaly threshold as a varint in the order of the table. Policies with the same meaning have the same hash; the cipher order counts. `GetPolicyHashString()` gives it as lowercase hex.
* Link reasons for refusals and closes: `DRN_LINKREASON_LIMIT_EXCEEDED` (15), `DRN_LINKREASON_BANNED` (16), `DRN_LINKREASON_POLICY` (17: another policy hash, no common cipher, a cipher that was not offered, encryption required), `DRN_LINKREASON_LOCAL_ERROR` (18: no randomness), `DRN_LINKREASON_KEY_MISMATCH` (19: not the pinned key, or no proof of it).

### 11.2 Before a peer is authenticated

* **Restricted receive**: until the link is operational `CDRNLinkReliability` takes a packet only if all its messages are LINK messages of the handshake on lane 0 or 3, without FRAGMENT, and lane 0 holds back at most `DRN_PREAUTH_HOLD_SLOTS` (8) messages. Messages after an in-order WELCOME are taken (they are for the operational link). Anything else refuses the whole packet before any of it is kept or acknowledged: on a datagram channel it is dropped and the peer sends it again once the link is operational; on a stream channel the link closes (`PROTOCOL_ERROR`). Counted in `CDRNLinkStats::preAuthRefusedPackets`.
* **Small packets**: `CIDRNChannel::SetMaxReceivePacketSize` bounds what a stream channel collects; the link sets `maxPreAuthPacketSize` at the start and `DRN_MAX_STREAM_PACKET_SIZE` once operational. The tcp channel applies it to its framer (a larger frame is a framing error, the channel closes). A web socket channel applies it to its connection, accepted (`WEB::CWebSocketConnection::SetMaxMessageSize`) or connecting (`WEB::CWebSocketClient::SetMaxMessageSize`); a larger message closes it with 1009. The web socket listener starts every new connection at `maxPreAuthPacketSize` (`ApplySecurityPolicy`).
* **Handshake limits** (`CDRNNode`, when a listener accepts a channel, before a link exists): the channel type, the address ban list, `maxPendingHandshakes` and `maxHandshakesPerAddressPerMinute` (a window of 60 s per address, at most 65536 addresses tracked). A refused channel is closed without a reply. `GetRefusedAcceptCount()`, `GetPendingHandshakeCount()`, `GetTelemetry()`.
* **UDP stateless cookies** (`CDRNUdpChannelListener`, `CIDRNChannelListener::ApplySecurityPolicy`): a DRN packet of an unknown address is answered with HELLO_RETRY `{u8 0xD2, u8 version 1, u32 window, 16 bytes cookie}`, cookie = the first 16 bytes of HMAC-SHA256( listener secret, IPv4 address, port, window ), window = 10 s of the node clock. Only a COOKIE_ECHO `{u8 0xD3, u8 version, u32 window, cookie, packet}` with the cookie of this or the previous window creates a channel. The listener answers only datagrams at least as large as the retry. The connecting udp channel keeps its last packet until the listener answers, sends it again with the cookie on HELLO_RETRY and puts the cookie in front of what it sends until a DRN packet of the listener arrives; the listener strips it. The secret is 32 random bytes per `Listen()`.

### 11.3 Receive limits per peer

`CDRNLinkMonitor` keeps a token bucket for messages and one for bytes (each holds one second worth). A packet is taken while both are above zero and charged afterwards, so a large packet takes a bucket below zero and the peer waits it off.

* Over a limit on a datagram channel: the packet is dropped before anything of it is taken or acknowledged, reliable messages in it come again later.
* Over a limit on a stream channel: packets are held back in order and taken when the buckets refill; holding back more than a second of bytes (at least one largest message) closes the link (`DRN_LIMIT_DEFERRED_BYTES`).
* Every hit is counted; `PeerLimitExceededEvent` comes at most once per second per kind. Hits in `sustainedLimitSeconds` consecutive seconds close the link with `DRN_LINKREASON_LIMIT_EXCEEDED` and a text; the peer gets it in CLOSE.
* Fragments: a new group beyond `maxFragmentGroups` or `maxFragmentBytes`, or a message (fragmented or not) larger than `maxMessageSize`, closes the link with `LIMIT_EXCEEDED` after `PeerLimitExceededEvent`: dropping part of a reliable message cannot be repaired.

### 11.4 Bans

* `CIDRNBanList::IsBanned( kind, value, entry )` with kinds `DRN_BAN_USER_ID`, `DRN_BAN_PUBLIC_KEY` (lowercase hex, `CDRNBanEntry::PublicKeyToString`) and `DRN_BAN_ADDRESS` (host without port); `CDRNBanEntry` holds the expiry (Unix seconds, 0 = never) and the reason.
* `CDRNNode::SetBanList` (not owned): the address is checked when a channel is accepted (closed, counted), the user id and the user's public key (`CDRNAuthResult::publicKey`) when a link authenticated: REFUSE with `DRN_LINKREASON_BANNED` and the text `"banned: <reason>"`.
* `CDRNBanList`: thread-safe, in memory and in a file, also a `CIConfigurable` (children `Ban` with `kind`, `expiry`, `value`, `reason`). The file is UTF-8 text, one ban per line: `kind TAB expiry TAB value TAB reason`, kind `user` / `key` / `address`, tab, line breaks and backslash escaped as `\t`, `\n`, `\r`, `\\`, `#` starts a comment, expired bans are not written. With `SetFilePath` every change is saved at once.

### 11.5 Access and roles

`CIDRNAccessPolicy::MaySubscribeAs( peerId, authResult, item )` and `MayWriteAs( peerId, authResult, item, key )` carry what the member's authentication established (`CDRNAuthResult`: user id, role, session); DRN asks these, by default they ask `MaySubscribe` / `MayWrite`. `CDRNSession::GetMemberAuthResult( peerId )` gives the authentication of a member's direct link. `CDRNDefaultAccessPolicy` enforces role sets when they are set: `SetJoinRoles`, `SetSubscribeRoles`, `SetWriteRoles` (empty = every role).

### 11.6 Anomalies

DRN reports, the game decides. Events of the link and, forwarded with the link, of the node (`CDRNNodeLinkInfo::anomaly`): `PeerLimitExceededEvent`, `ClockAnomalyEvent`, `TrafficAnomalyEvent`, event data `CDRNLinkAnomaly` `{kind, measured, threshold, nodeTime, text}`.

* **Clock drift** (`clockDriftPercent` 5, `clockWindowMs` 2000, `clockSustainMs` 6000): per window the packet with the highest `sendTime - local link time` (the least delayed) is kept; the drift is the change of that offset between two windows over the time between their samples, in percent. A drift beyond the threshold for `clockSustainMs` is reported (measured = the drift), then measuring starts over. A speed hack runs the peer's clock, and so its `sendTime`, faster than ours.
* **Silence then a burst** (`silenceMs` 1000, `burstPackets` 16, `burstWindowMs` 100): nothing from the peer for `silenceMs` and then `burstPackets` packets within `burstWindowMs`: packets held back and let go at once, a lag switch (measured = the silence in ms).
* **Flood** (`floodPacketsPerSecond` 10000): more packets than that in one second, reported once per second.

### 11.7 Telemetry

* `CDRNPeerLink::GetTelemetry( CDataNode& )`: link id, remote node and endpoint, user id; the counters of `CDRNLinkStats` (sent, received, lost, duplicates, too old, stale, resent, `preAuthRefusedPackets`, `rateLimitedPackets`); open fragment groups and bytes, held back packets and bytes; round trip time, its jitter, minimum, loss ratio, clock offset; and of the monitor: packets and bytes arrived, messages taken, packets / messages / bytes of the last whole second and their peaks, mean arrival interval and its jitter (EWMA 1/8 and 1/16 of the deviation), the longest interval, packets dropped for limits, limit hits in total and per kind (`limitHits.<kind>`), clock and traffic anomalies, the last clock drift.
* `CDRNSession::GetTelemetry`: node `DRNSessionTelemetry` (session name, local and sequencer peer id, node time) with a child `Member` (`peerId`, `nodeId` and the link telemetry) per member with a direct link.
* `CDRNNode::GetTelemetry`: node `DRNNodeTelemetry` (node id, policy hash, links, pending handshakes, refusals at accept per reason) with a child `Link` per link.

### 11.8 Recording

`CDRNSessionRecorder::Start( session, file, classMask )` records what the links of the session send and receive, by message class (default REPL, STREAM and APP: grids, streams, application messages), until `Stop()` or the end of the session; the session calls it for every message (`OnLinkTraffic`). Records are collected in memory and appended every 64 KB. `CDRNSessionRecordingReader` reads a recording back.

```
file    := "DRNREC01" (8 bytes), u8 version (1), string sessionName, varint64 start (CDRNClock microseconds), varint localPeerId, record*
record  := u8 type (1 = message), varint64 timeUs (since the start), u8 flags (bit 0: outgoing), varint peerId, varint linkId,
           u8 lane, u8 class, varint opcode, varint size, payload
```

### 11.9 Secrets and randomness

* Secrets and tokens are compared in constant time: proofs, token MACs, policy hashes, pinned keys, bulk transfer hashes and UDP cookies with `CDRNCrypto::ConstantTimeEquals` (gucefCORE's `CSha256::ConstantTimeEquals`), the relay token with `ConstantTimeEqualsHashed` (the digests of both, so neither content nor length leaks). SHA-256, HMAC-SHA256 and HKDF-SHA256 are gucefCORE's (`gucefCORE_CSha256.h`); DRN has no hash code of its own.
* Every `CORE::CRandom` call is checked and fails closed: no challenge nonce or ephemeral key refuses the link (`DRN_LINKREASON_LOCAL_ERROR`), no cookie secret fails `Listen()`, no static key leaves the node without one (it cannot accept encrypted links), no node id leaves it empty and `SetSettings` / `Start` refuse an empty node id.

### 11.10 Decisions

* Limits are enforced on what is received; what a node sends is bounded only by `maxMessageSize`.
* A packet over a receive limit on a datagram channel is dropped unacknowledged, on a stream channel held back: neither loses a reliable message.
* Fragment and message size limits close the link: a reliable message cannot be partly dropped.
* Refusals at accept close the channel without a reply, so a flood costs no handshake state and learns nothing.
* The address ban is checked at accept, the user and public key bans at AUTH (the key a signed token names for its user).
* Cookies are a matter of the udp channel and its listener, outside the link protocol; tcp and web socket have their own handshake.
* The pre-auth hold of 8 lane 0 messages bounds what a peer can make a listener keep before it authenticated.
* Anomalies are only reported; what to do with a peer is the game's decision.
* The session recorder records through the session, so it sees exactly the messages of the session's links.
* The policy is the game's, carried in settings; DRN never lets a player setting weaken it.
* The policy hash is compared in the handshake: every node of a game has the same policy. Links to a relay service run under the relay's advertised policy, as the relay forwards what is protected end to end; a policy override is only taken from relay bindings, never from a peer's endpoint.
* Encryption is opportunistic below the policy: two nodes that can encrypt always do; `requireEncryption` only decides whether a link without it is refused.
* AUTH is protected, so a bearer token is bound to its connection; tokens that prove a key of the user come with player identities.
* The acceptor protects what it sends only after the initiator's first protected packet: a CHALLENGE that is sent again stays readable for an initiator that lost the first one.
* A protected packet that does not authenticate is dropped on a datagram channel and closes a stream channel: a stream cannot get it again.
* Every link keeps the tag's 16 bytes out of its datagram budget, encrypted or not, so the budget does not change with the handshake.
* Cipher instances come from the codec registry (highest priority first), so OS providers and plugins serve DRN without DRN knowing them.
* The default cipher order stays portable (`xchacha20-poly1305` first); faster hardware ciphers are the game's choice in its policy, with the measured costs in 11.11.
* The client side of `x25519` refuses an all zero shared secret (a low order point).

### 11.11 Handshake 2 and packet protection

* **Keys**:
  * every node has a static x25519 key (`CDRNNodeSettings::staticPrivateKey` in hex, or `staticKeyFile`, made and saved when it does not exist, or a new key per run); `GetPublicKey()`, `GetPublicKeyString()`;
  * `GetPublishedEndpoints()` are the advertised endpoints with the public key as parameter `key`; sessions advertise these. A connector to an endpoint with `key` pins it: an acceptor that shows another key, or none, is refused (`KEY_MISMATCH`). Without a pinned key the initiator takes the acceptor's key from CHALLENGE.
* **Handshake** (Noise NK like):
  1. the initiator, when it has key agreement and one of its ciphers, sends HELLO with an ephemeral public key and the ciphers of its policy it has codecs for, in the policy's order;
  2. the acceptor checks the policy hash, picks the first cipher of its own list that was offered and that it has (none: plain, or REFUSE when it requires encryption), makes an ephemeral key and sends CHALLENGE with its ephemeral and static public keys and the cipher;
  3. transcript hash = SHA-256( "DRN handshake 2", u32 size + HELLO payload, u32 size + CHALLENGE payload );
  4. keys = HKDF-SHA256( salt = transcript hash, ikm = DH( e_i, e_a ) ‖ DH( e_i, s_a ), info = "DRN 2 keys " + cipher ), 64 bytes: initiator to acceptor, acceptor to initiator. Only the holder of the static key gets them;
  5. the initiator checks that the chosen cipher is one it offered, derives the keys and sends AUTH protected, its proof over the transcript hash (`CDRNAuthContext::transcriptHash`);
  6. the acceptor takes AUTH only protected; from the first protected packet it receives it protects what it sends; WELCOME is its first protected message, and the initiator takes WELCOME only protected.
  * Anything changed on the way (the offer, a key, the policy hash) gives other keys on the two sides: the acceptor cannot open AUTH and the handshake fails. Ephemeral private keys are wiped once used.
* **Packets** (`CDRNPacketProtection`, one per link, a `CORE::CICodecInstance` of the `EncryptionCodec` per direction):
  * a protected packet is the header in the clear with the first byte 0xD4 / 0xD5 (the key phase), then the encrypted body and the 16 byte tag; the header is the associated data;
  * nonce = direction byte (0 initiator to acceptor, 1 back), zeros, 64 bit sequence big endian (12 bytes, 24 for xchacha20): the header sequence extended with its wraps;
  * after 2^31 packets or 64 GB the sender derives its next key (HKDF-SHA256 of the key, "DRN 2 rekey") and flips the phase; the receiver follows when a packet of the other phase opens with the next key and keeps the previous key for late packets;
  * once a link is protected a plain packet is dropped (`GetDroppedPlainPacketCount()`); a packet that does not authenticate is counted (`GetAuthenticationFailureCount()`); duplicates and old packets open and are then dropped by the sequence window;
  * `IsProtected()`, `GetProtection()` (cipher, rekeys), `GetPeerStaticKey()`, `GetTranscriptHash()`.
* `CDRNCrypto` reaches the codecs: `x25519` (`KeyAgreementCodec`: Encode private → public, Decode peer public + `privateKey` → shared), `ed25519` (`SignatureCodec`: 32 byte seed; Encode with `privateKey` → signature, with `output` = `publicKey` → the public key; Decode with `publicKey` and `signature` verifies), the AEAD ciphers (`EncryptionCodec`, `key` per instance, nonce and AAD per call).
* **Cost** (release build, -O3, one seal and one open of a 1200 byte datagram with 20 bytes of associated data, Linux x86-64 with AES-NI; the encryption suite logs these per provider):

  | cipher | provider (priority) | µs per seal + open |
  |---|---|---|
  | `xchacha20-poly1305` | codecspluginMONOCYPHER (50) | 5 – 10 |
  | `chacha20-poly1305` | AF_ALG (100) | 5 – 9 |
  | `chacha20-poly1305` | codecspluginMONOCYPHER (50) | 5 – 7 |
  | `chacha20-poly1305` | codecspluginOPENSSL (50) | 1.7 – 3.5 |
  | `aes-256-gcm` | AF_ALG (100) | 4 – 8 |
  | `aes-256-gcm` | codecspluginOPENSSL (50) | 0.8 – 2.9 |

  Through `CDRNPacketProtection` with the default cipher (building the packet included) it is 8 – 17 µs; the loopback round trip of an encrypted link is that of a plain one within the noise. A host of 64 peers at 60 Hz seals and opens 3840 datagrams a second each way: at 10 µs that is 4 % of one core, so every provider serves it.
* **Choosing**: the default order keeps `xchacha20-poly1305` first: it is there wherever codecspluginMONOCYPHER is (every platform), constant time in software on every CPU, and its 24 byte nonce leaves no doubt about nonce reuse. The fastest secure choice here is `aes-256-gcm` of OpenSSL (hardware AES); a game that ships codecspluginOPENSSL and runs on CPUs with AES instructions puts `aes-256-gcm` first in its policy's `ciphers` and gives the plugin a priority above the OS (`codecPriority` in the plugin's metadata, see the codecs design), since AF_ALG has priority 100 and costs a system call per message.

## 12. gucefSMDRN

gucefSMDRN (Sector Managed DRN) is the layer for MMO-style worlds: zones and sectors of a 3D coordinate space come into and go out of relevance as the vantage point moves (as in EVE Online). DRN gives it what it needs and no more:
* items addressable by id;
* subscribe / unsubscribe cheap enough to change every few ticks;
* sessions that can be split per zone (a zone arbiter is the server of a zone session);
* snapshot on subscribe, so a sector that comes into relevance arrives complete.

A sector maps to a set of grids (its objects) and streams (its events) in the zone's session; moving the vantage point changes the subscriptions. SMDRN's headers follow the DRN API and the module keeps building; its implementation comes when a consumer needs it.

## 13. Files

The `include/` / `src/` pairs:
* `gucefDRN_CDRNGlobal` (factories, events)
* `gucefDRN_CDRNWireWriter`, `gucefDRN_CDRNWireReader`, `gucefDRN_CDRNVariantCodec`, `gucefDRN_DRNProtocol.h`
* `gucefDRN_CIDRNChannel`, `gucefDRN_CIDRNChannelListener`, `gucefDRN_CDRNEndpoint`
* `gucefDRN_CDRNTcpChannel`, `gucefDRN_CDRNUdpChannel`, `gucefDRN_CDRNWebSocketChannel`, `gucefDRN_CDRNRelayChannel`
* `gucefDRN_CDRNPeerLink`, `gucefDRN_CDRNLinkReliability`, `gucefDRN_CIDRNLatencyMeter`, `gucefDRN_CDRNLatencyInfo`
* `gucefDRN_CDRNNode`, `gucefDRN_CDRNNodeSettings`, `gucefDRN_CDRNConnector`
* `gucefDRN_CIDRNAuthenticator` (+ the three implementations), `gucefDRN_CIDRNAccessPolicy`, `gucefDRN_CIDRNWriteValidator`
* `gucefDRN_CDRNSession`, `gucefDRN_CDRNSessionSettings`
* `gucefDRN_CDRNReplicator`, `gucefDRN_CDRNDataItem`, `gucefDRN_CDRNDataGrid`, `gucefDRN_CDRNDataGridSettings`, `gucefDRN_CDRNDataStream`, `gucefDRN_CDRNDataStreamSettings`, `gucefDRN_CDRNVotePolicy`
* `gucefDRN_CDRNBulkTransfer`
* `gucefDRN_CDRNRelayService`, `gucefDRN_CDRNReachabilityManager`
* `gucefDRN_CDRNSecurityPolicy`, `gucefDRN_CDRNLinkMonitor`, `gucefDRN_CIDRNBanList`, `gucefDRN_CDRNBanList`, `gucefDRN_CDRNSessionRecorder`
* `gucefDRN_CDRNCrypto`, `gucefDRN_CDRNPacketProtection`

The old `CDRNDataGroup`, `CDRNDataGroupProperties`, `CDRNPeerLinkData`, `CIDRNPeerValidator`, the old wire protocol, the `CIDRNPeerLinkCrypter` hook and DRN's own SHA-256 are replaced (by `CDRNPacketProtection` and gucefCORE's `CSha256`).

## 14. Tests (`tests/gucefDRN_TestApp`, test framework, loopback)

* wire reader / writer and the variant codec round trips; framing with split and merged reads; a 10 MB message;
* the packet header: wrap-safe sequences, duplicates dropped, stale data on lane 2 dropped, RTT and clock offset within tolerance (with a delay-injecting channel) on tcp, udp, websocket and relay;
* reliability on udp through `CDRNLossyChannel` (loss, reorder, duplicates);
* handshake: each authenticator, refusal, timeouts;
* one test body run as CLIENT_SERVER and as PEER_TO_PEER with 3 and 8 peers, the same results for the application;
* data grids:
  * 1000 updates of one key over a lossy channel arrive as the newest value, with fewer sends than updates;
  * 64 players at 60 Hz within the byte budget;
  * the loopback send-to-receive time within a tick plus transport;
  * no allocation on the hot path;
  * the `CDataNode` dump and the settings round trip;
* stream slots: multi-variant entries in order and counted, the backlog limit holds the sender back, retained history reaches a late subscriber;
* authority: a non-owner write refused, a proposal accepted / refused, votes passing / failing / timing out;
* late join converges; a gap leads to a resync;
* bulk transfer with resume and hash check;
* relay on loopback; games with different policies sharing one relay, a node of another game refused end to end;
* gucefSMDRN still builds.
* The allocation test runs over an in-process test channel (`mem`), so it measures DRN and the GUCEF event system and not a socket layer.
* security (`security`): the policy config round trip and its hash pinned to an independently computed value, `IsWeakerThan`; a policy that cannot be honoured stops nodes, sessions and connectors; pre-auth fragment floods keep nothing; fragment caps; a stream peer sending more than the handshake, or a large frame, before it authenticated is closed; pending and per address handshake limits; the UDP cookie (an address that never sees the retry, a forged cookie and another address's cookie get no state); receive limits reported and, sustained, closed, over udp and tcp; the ban list file and config round trip and persistence; refusal at accept and at AUTH; roles; clock drift and a lag switch through the `lossy-` decorator (`warp`, `holdAfter`, `holdFor`) while an honest peer is not reported; the session and node telemetry; the recording round trip.
* encryption (`encryption`, loads `codecspluginMONOCYPHER`): links encrypted with the first common cipher over tcp, udp and websocket (relay in the relay suite), the acceptor's key proven; a pinned key links, another is refused, every handshake has its own transcript; an AUTH recorded from one handshake and sent in another (`recordAuth`, `injectAuth`) is refused; another policy is refused; a changed cipher offer (the `lossy-` decorator's `downgrade`) fails the handshake; changed packets (`tamper`) are dropped on udp and close tcp; duplicated and reordered packets are delivered once; packet protection refuses another key, a changed header or body, rekeys and opens late packets of the previous key; the datagram budget keeps room for the tag; the RFC 8032 Ed25519 vector; signed tokens (HMAC and other issuers refused, a banned user key refused); the policy hash in tokens; the static key file; the cost of a packet per AEAD provider (Monocypher, OpenSSL, AF_ALG) and the round trip. Every other suite runs encrypted, as the default policy requires.
