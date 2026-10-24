# gucefCOMCORE interfaces and gucefCOM implementations: discovery, round trip times, port mapping, STUN, IRC

gucefCOMCORE holds the networking primitives (sockets, addresses, network interfaces, `CPing`) and the interfaces with their factories. gucefCOM holds out-of-the-box implementations of those interfaces built purely on gucefCOMCORE, as `CStatsDClient` already is: common functionality without third party code. Anything that needs third party code is a plugin (`comcorepluginUPNP` on miniupnpc). Every implementation, built in or plugged in, registers in the same factory and is chosen by name from configuration. Tests: `tests/gucefCOMCORE_TestApp`, `tests/gucefCOM_TestApp`, `tests/gucefWEB_TestApp` (IRC over WebSocket).

## 1. Discovery

Finding services on the network without knowing where they are: LAN game servers and lobbies, devices, other instances of an application.

### 1.1 Interfaces (COMCORE)

* `CIDiscoveryEntry`: the existing accessors (application id, application instance id, host instance id, host, capabilities), plus:
  * `GetServiceType` / `SetServiceType`, e.g. `_gucef-game._udp`
  * `GetInstanceName` / `SetInstanceName`: the human readable name
  * `GetAttributes` / `SetAttributes`: a `CORE::CValueList`, the advertised data, small: kept under 1 KB for UDP
  * `GetEndpoints` / `SetEndpoints`: `CORE::CString::StringVector` of `scheme://host:port`
  * `GetTtlInSecs`, `GetLastSeen`, `GetSourceClientName`
* `CDiscoveryEntry`: the concrete entry. `CIDiscoveryEntryPtr` is the shared pointer type.
* `CIDiscoveryClient : CORE::CObservingNotifier, CORE::CIConfigurable, CIDiscovery` (now a notifier):
  * `Browse( serviceType )`, `StopBrowse( serviceType )`
  * `Advertise( CIDiscoveryEntryPtr )`, `Withdraw( CIDiscoveryEntryPtr )`; `Register` / `Unregister` stay as the same operations
  * `IsBrowsing`, `GetDiscoveredEntries( serviceType, entries )`
  * events `EntryDiscoveredEvent`, `EntryUpdatedEvent`, `EntryLostEvent` (each `CDiscoveryEntryEventData`: the entry), `BrowseStartedEvent`, `BrowseStoppedEvent`, `DiscoveryErrorEvent`
  * An entry is lost when its TTL runs out without being heard again, or when its advertiser withdraws it (a goodbye).
* `CDiscoveryClientFactory`: `CORE::CTAbstractFactory< CString, CIDiscoveryClient >`, held by `CComCoreGlobal` (`GetDiscoveryClientFactory()`).
* `CDiscoveryManager`:
  * `LoadConfig` creates the clients named in its config (`<DiscoveryClient type="mdns" ...>`) through the factory and registers them;
  * `Browse` / `Advertise` go to every registered client;
  * it observes them and re-publishes the same events, entries merged by (service type, instance id).
* `CDiscoveryClientBase` (COMCORE) holds the bookkeeping every client shares (browsed types, advertised and discovered entries, TTL expiry, events, `Find*`), so an implementation only does its wire protocol.
* The instance id of the key is the application instance id, the instance name when there is none; `CDiscoveryEntryEventData` is the `CIDiscoveryEntryPtr` itself.
* `Browse( "*" )` accepts every service type; `IsBrowsing()` without a type asks whether any browse is active.
* An endpoint host of `0.0.0.0` (or none) is replaced by the address the advertisement came from (UDP beacons, SSDP).
* `CDiscoveryManager::UnregisterDiscoveryClientsOfType()` lets a module drop the clients made with its factories before it goes away.

### 1.2 Built-in clients (COM, registered by `CComGlobal`)

| type | how | notes |
|---|---|---|
| `broadcast` | UDP to 255.255.255.255 and to the directed broadcast address of every `CINetworkInterface` (address \| ~mask), port default 47800 | IPv4 only, one subnet |
| `multicast` | UDP to group 239.255.47.80 port 47800 (configurable), joined on all interfaces, TTL 1 by default | crosses routers that forward multicast when TTL > 1 |
| `mdns` | mDNS / DNS-SD (RFC 6762 / 6763) on 224.0.0.251:5353: PTR `_service._udp.local` → SRV + TXT + A | interoperates with Avahi and Bonjour; the socket uses SO_REUSEADDR / SO_REUSEPORT so it can share the port with a system responder |
| `ssdp` | SSDP on 239.255.255.250:1900: M-SEARCH queries, NOTIFY ssdp:alive / ssdp:byebye, the attributes in `X-GUCEF-*` headers | UPnP style; UPnP devices answer too (entries with the ST as service type) |

* `broadcast` and `multicast` share `CUdpDiscoveryCodec`:
  * the beacon is `['G','D','S','C'][u8 version][u8 kind: QUERY | ANNOUNCE | GOODBYE][string service type][string instance id][string instance name][u16 ttl][u16 endpoint count][strings][CValueList attributes as key/value strings]`;
  * strings are u16 length + UTF-8; integers big endian;
  * a QUERY is answered by every matching advertiser with an ANNOUNCE;
  * advertisers also announce every TTL/3.
  * The attributes are a u16 pair count followed by one key/value string pair per value; a beacon is at most 1400 bytes.
  * Answers to a QUERY go to the same broadcast / multicast destinations as everything else; the `extraTargets` setting adds unicast `host:port` peers.
* `mdns` has its own DNS message codec (`CDnsMessage`: header, questions, answers with name compression), enough for PTR / SRV / TXT / A records, plus:
  * known-answer suppression on queries;
  * announcement on advertise and a goodbye (TTL 0) on withdraw;
  * TXT entries `key=value` carrying the attributes.
  * The application instance id travels as TXT key `gucef-id`, the SRV port is the port of the first endpoint, the host name defaults to `<host name>-gucef.local`; there is no probing for name conflicts.
* `ssdp` sends M-SEARCH from a socket of its own (which gets the unicast answers); the USN is `uuid:<instance id>::<service type>`, the headers `X-GUCEF-NAME`, `X-GUCEF-ENDPOINT` and `X-GUCEF-ATTR-<key>`.

## 2. Round trip times

### 2.1 Interfaces (COMCORE)

* `CIRttProbe : CORE::CObservingNotifier`:
  * `Start( const CHostAddress& target, UInt32 count, UInt32 intervalMs, UInt32 timeoutMs )`, `Stop()`
  * `IsAvailable()`: false when the method cannot run here, e.g. ICMP without privileges or on the web
  * `GetMethodName()`
  * events `ProbeResultEvent` (`CRttProbeEventData`: target, RTT in microseconds, sequence), `ProbeTimeoutEvent`, `ProbeFailedEvent`, `ProbeFinishedEvent`
* `CRttProbeFactory` in `CComCoreGlobal`.
* A count of 0 probes until `Stop()`; timeout and failure events carry an RTT of 0; the measurement is as fine as the pulses the sockets run on.

### 2.2 Built in (COM)

| method | how |
|---|---|
| `icmp` | wraps `COMCORE::CPing`; `IsAvailable()` is false where `CPing` cannot work (no `ping_group_range` / CAP_NET_RAW on Linux, web, macOS) |
| `tcp-connect` | the time from a non-blocking connect to the connection, then close (or the refusal, which also proves the host answers) |
| `udp-echo` | a 16-byte `['G','D','E','C'][u32 seq][u64 send time]` datagram echoed by `CUdpEchoResponder` (any application can run one, e.g. next to its game port) |

## 3. Port mapping and NAT

### 3.1 Interfaces (COMCORE)

* `CIPortMapper : CORE::CObservingNotifier`:
  * `RequestMapping( protocol TCP|UDP, internalPort, externalPort (0 = any), leaseSecs, CORE::CFutureResult& )`
  * `RemoveMapping( … )`, `GetExternalAddress( CORE::CFutureResult& )`
  * `IsAvailable()`
  * events `MappingCreatedEvent`, `MappingLostEvent`, `PortMapperErrorEvent`
  * The future's result data is a `CPortMappingResult` (`CORE::CICloneable`: external address, external port, lease).
* `CPortMapperFactory` in `CComCoreGlobal`.
* `CPortMappingResult::TryGetFromFuture()` reads the result; a failed request ends its future in an error status; `MappingLostEvent` is for mappings that could not be renewed, not for `RemoveMapping()`.

### 3.2 Built in and plugin

* `natpmp` (COM):
  * NAT-PMP (RFC 6886) to the default gateway on UDP 5351: the external address request and map requests, with the PCP (RFC 6887) MAP opcode as the first try;
  * leases renewed at half time.
  * The gateway is the default route's gateway: `/proc/net/route` on Linux, `GetBestRoute` on Windows.
  * Requests are sent again after 250 ms, doubling, 4 times; while the gateway's protocol is unknown PCP gets 2 tries before the request goes as NAT-PMP.
  * The external address is asked with the NAT-PMP opcode 0; a PCP-only gateway answers it with the address learned from its MAP answers.
* `upnp` (plugin `comcorepluginUPNP`): UPnP IGD through the vendored miniupnpc, run as a pool task.
  * miniupnpc 1.4 has no lease: a mapping lasts until removed and the mapper removes its mappings when destroyed; an external port of 0 starts at the internal port and moves up on conflicts.
  * miniupnpc is a static library inside the plugin.

### 3.3 STUN (COM)

`CStunClient`: RFC 5389 Binding requests over a given `CUDPSocket` (so the reflexive address is the one of that socket's mapping), with the XOR-MAPPED-ADDRESS / MAPPED-ADDRESS answer. `QueryReflexiveAddress( server, CORE::CFutureResult& )`, with retransmissions after 500 ms, 1 s and 2 s. The base for hole punching in DRN.
The future fails 4 s after the last send; its result data is a `CStunClient::CStunResult` (`TryGetResultFromFuture()`).

## 4. IRC (COM, transport in WEB)

* `CIrcClient : CORE::CObservingNotifier`: a generic IRCv3 client for any application.
  * Connection: `Connect( settings )`, `Disconnect( message )`, `IsRegistered()`.
  * Channels: `Join( channel, key )`, `Part( channel, reason )`, `SetTopic`.
  * Messages: `PrivMsg( target, text )`, `Notice`, `SendRaw( line )`.
  * Queries: `GetChannels`, `GetChannelMembers( channel )`, `GetNick`, `SetNick`, `HasCapability( name )`.
  * Events: `ConnectedEvent`, `RegisteredEvent`, `DisconnectedEvent`, `MessageEvent` (`CIrcMessageEventData`: source nick, target, text, server time, msgid, account, tags), `NoticeEvent`, `JoinEvent`, `PartEvent`, `QuitEvent`, `NickChangedEvent`, `TopicEvent`, `NamesEvent`, `ErrorEvent`, `RawLineEvent`.
  * `CIrcClientSettings : CORE::CIConfigurable`: server, port, use WebSocket, nick (+ alternatives), user name, real name, SASL account and password, wanted capabilities, auto-join channels, ping timeout.
* `CIrcMessage`: the parser and writer of IRC lines with IRCv3 message tags (`@key=value;… :prefix COMMAND params :trailing`), tag value escaping.
* Registration:
  1. `CAP LS 302`, `NICK`, `USER`;
  2. `CAP REQ` for the wanted capabilities that are offered (`sasl`, `message-tags`, `server-time`, `echo-message`, `account-tag`, `batch`, `chathistory`, `multi-prefix`);
  3. SASL PLAIN when an account is set;
  4. `CAP END`;
  5. 001 means registered.
  * PING is answered with PONG; a nick in use is retried with the alternatives.
* Transport: `CIIrcTransport` (`Connect`, `Disconnect`, `SendLine`, events connected / line received / disconnected).
  * `CIrcTcpTransport` (COM) over `COMCORE::CTCPClientSocket`, splitting on CRLF.
  * `CIrcWebSocketTransport` (WEB) over `WEB::CWebSocketClient` with the `text.ircv3.net` subprotocol, one line per text message. WEB registers it, so `CIrcClient` picks it when the settings ask for WebSocket and the transport factory (`CIrcTransportFactory` in `CComGlobal`) has it.
* Tested against a scripted fake server in the test app; an optional integration test against a real server (Ergo) runs when `GUCEF_TEST_IRC_SERVER=host:port` is set.
* Every event carries a `CIrcMessageEventData`; KICK is reported as a `PartEvent` of the kicked nick; a nick in use after the alternatives gets an underscore added.
* The transports are named `tcp` and `websocket`; the WebSocket url is `ws://<server>:<port><webSocketPath>`; line breaks in sent text become spaces.

## 5. Files

* COMCORE: `gucefCOMCORE_CDiscoveryEntry`, `gucefCOMCORE_CDiscoveryClientFactory` (typedef header), `gucefCOMCORE_CIRttProbe`, `gucefCOMCORE_CIPortMapper`, `gucefCOMCORE_CPortMappingResult`, changes to `CIDiscoveryEntry`, `CIDiscoveryClient`, `CDiscoveryManager`, `CComCoreGlobal`.
* COM:
  * `gucefCOM_CUdpDiscoveryCodec`, `gucefCOM_CUdpBroadcastDiscoveryClient`, `gucefCOM_CUdpMulticastDiscoveryClient`;
  * `gucefCOM_CDnsMessage`, `gucefCOM_CMdnsDiscoveryClient`, `gucefCOM_CSsdpDiscoveryClient`;
  * `gucefCOM_CIcmpRttProbe`, `gucefCOM_CTcpConnectRttProbe`, `gucefCOM_CUdpEchoRttProbe`, `gucefCOM_CUdpEchoResponder`;
  * `gucefCOM_CNatPmpPortMapper`, `gucefCOM_CStunClient`;
  * `gucefCOM_CIrcMessage`, `gucefCOM_CIIrcTransport`, `gucefCOM_CIrcTcpTransport`, `gucefCOM_CIrcClient`, `gucefCOM_CIrcClientSettings`.
* Shared helpers: `gucefCOMCORE_CDiscoveryClientBase`, `gucefCOM_CUdpDiscoveryClientBase` (the beacon protocol of `broadcast` and `multicast`), `gucefCOM_NetworkByteOrder` (bounds checked big endian reading and writing).
* WEB: `gucefWEB_CIrcWebSocketTransport`.
* Plugin `plugins/COMCORE/comcorepluginUPNP`.
