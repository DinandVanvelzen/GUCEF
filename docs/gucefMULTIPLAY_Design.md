# gucefMULTIPLAY

Module `platform/gucefMULTIPLAY`, namespace `GUCEF::MULTIPLAY`, files `gucefMULTIPLAY_*.h/.cpp`. An out-of-the-box multiplayer framework for games and applications: finding games and lobbies (on the LAN and on the internet), lobbies in which players agree on settings and start together, the hand-over of everything the game needs to connect, chat, and ping / latency figures. Every part is an interface with selectable implementations; the built-in ones work with nothing but GUCEF. Depends on gucefMT, gucefCORE, gucefCOMCORE, gucefCOM, gucefWEB and gucefDRN. Tests: `tests/gucefMULTIPLAY_TestApp`.

## 1. The user flow

Modelled on how players experience Beyond All Reason, independent of how that game implements it:

1. **Browse**: a list of games, lobbies waiting for players and games in progress, from the LAN and from internet services, with their ping. Filter by text, kind, version, free places, password, ping.
2. **Join** a lobby (or create one): the lobby shows the slots (teams, factions, colours), the members with their latency, the game settings and a chat.
3. **Agree**: the boss (the host / creator) changes settings directly; other players propose changes, which become a vote. Players pick their slot, faction and colour, or spectate.
4. **Ready**: every player marks ready.
5. **Start**: the boss starts when all are ready, or the players vote to start; a force start can be allowed. A countdown runs (cancelled when someone becomes unready).
6. **Hand-over**: every member gets a launch description: who the server is, or the peer list for peer-to-peer; every member's endpoints; the agreed settings and assignments; a signed token per member. The game connects with it.
7. **Spectate in progress**: a game in progress stays listed; a spectator who joins gets a spectator launch description.

## 2. Parts

| part | interface | built in | later as plugins |
|---|---|---|---|
| listings (games and lobbies) | `CIGameListingProvider` | `lan` (any COMCORE discovery client), `lobby-service`, `http-list` | |
| lobby service | `CILobbyService`, `CILobby` | `gucef` (embeddable `CGucefLobbyServer` + client) | `nakama`, `xmpp`, `tachyon`, `steam`, `eos` |
| chat | `CIChatService`, `CIChatChannel` | `lobby` (over DRN), `irc` (COM's `CIrcClient`) | `matrix`, `xmpp` |
| ping | `CPingService` over COMCORE `CIRttProbe`s | `udp-echo`, `tcp-connect`, `icmp` | |
| latency in a lobby | DRN `LatencyUpdateEvent` | | |
| identity | `CIIdentityProvider` | `local` (a profile file) | `steam`, `eos` |

`CMultiplayGlobal` (singleton, set up by the module) holds the factories (`CTAbstractFactory< CString, … >`) for listing providers, lobby services, chat services and identity providers, registers the built-in ones and the events. Plugins use the GUCEF generic plugin API and register into these factories; one plugin may register into several.

## 3. Listings

* `CGameListingEntry` (`CORE::CIDataNodeSerializable`):
  * `listingId`, `kind` (LOBBY / GAME_IN_PROGRESS)
  * `name`, `gameId`, `gameVersion`, `mapName`, `modeName`
  * `players`, `maxPlayers`, `spectators`, `passworded`
  * `hostName`, `endpoints` (URI strings such as `tcp://host:port` or `ws://host:port/path`, the form `DRN::CDRNEndpoint` parses, so listings do not depend on DRN)
  * `backendName` (which provider), `extra` (`CORE::CValueList`)
  * `pingMs` (-1 unknown), `lastSeen`
* `CIGameListingProvider : CORE::CObservingNotifier, CORE::CIConfigurable`: `Start( filter )`, `Stop()`, `Refresh()`, `GetListings()`; events `ListingAddedEvent`, `ListingUpdatedEvent`, `ListingRemovedEvent`, `ProviderErrorEvent`.
  * The interface keeps the listings: an implementation reports what it finds with `UpsertListing()` / `RemoveListing()`; the filter applies there except the ping limit, which the browser applies.
  * `lan`: browses the service type `_gucef-game._udp` with the COMCORE discovery clients named in its config (default `broadcast` + `multicast` + `mdns`). The entry attributes carry the listing fields (`kind`, `game`, `ver`, `map`, `mode`, `pl`, `max`, `spec`, `pw`, `id`).
    The listing id is the attribute `id`, else the entry key; the attribute `host` carries the host name. Clients come from `DiscoveryClient` children (attribute `type`, the rest is the client's config) or the `discoveryClients` list (`CDiscoveryClientList`).
  * `lobby-service`: lists through `CILobbyService::Browse` of the configured backend.
  * `http-list`: the existing `CPassiveURLServerListProvider` behind the interface (the XML / JSON list of a URL; the entry's attributes become `extra`).
    Listing id `http:<host>:<port>`, endpoint `<endpointScheme>://<host>:<port>` (config, default `udp`); attributes named like the LAN ones also fill the listing fields.
* `CGameAdvertiser`: publishes the local lobby or game to the configured discovery clients (LAN) and lobby services, and keeps the player counts current.
  Lobby services take it through `CILobbyService::PublishListing()` / `WithdrawListing()` (not supported by default); clients or services given in code replace the default discovery clients.
* `CGameBrowser`: aggregates the configured providers, merges duplicates (by listing id, else by endpoint), applies a `CGameListingFilter` (text, kind, compatible game id / version, not full, no password, max ping) and keeps pings current through `CPingService`. Events `BrowserListChangedEvent` (added / updated / removed with the id). It replaces `CServerList`, which stays for compatibility, repaired.
  The provider added first gives the content of a merged listing; the ping target is a listing's first endpoint; an unknown ping passes the ping limit.

## 4. Ping and latency

* `CPingService`: for a target (a listing's endpoints, or a host:port):
  * runs the probes named in its config in order of preference (`udp-echo`, `tcp-connect`, `icmp`), using the first that `IsAvailable()` and answers;
  * keeps a smoothed RTT per target and sends `PingUpdatedEvent` (target, RTT ms, method);
  * when no method can run, the target stays at "unknown": nothing is shown instead of an error.
  * config `methods`, `intervalInMs`, `timeoutInMs`, `smoothing`, `maxFailuresPerMethod` (a method gives way after that many failures in a row), `retryAfterInMs` (the methods are tried again after that).
* In a lobby the figures come from DRN: every member publishes its link latency (to the server in CLIENT_SERVER, to each peer in PEER_TO_PEER) into the lobby's `members` grid (owner-authoritative keys), so everyone sees everyone's latency, refreshed with each `LatencyUpdateEvent`.

## 5. Lobbies

### 5.1 Interfaces

* `CILobbyService : CORE::CObservingNotifier, CORE::CIConfigurable`:
  * `Connect( const CPlayerIdentity&, CORE::CFutureResult& )`, `Disconnect()`;
  * `Browse( filter )` (feeds a `lobby-service` listing provider);
  * `CreateLobby( const CLobbyCreateParams&, CORE::CFutureResult& )`, `JoinLobby( lobbyId, password, asSpectator, CORE::CFutureResult& )`;
  * `GetCapabilities()`: voting, relay, in-progress spectating, matchmaking, chat;
  * events `ServiceConnectedEvent`, `ServiceDisconnectedEvent`, `LobbyJoinedEvent` (a `CILobbyPtr`), `ServiceErrorEvent`.
* `CILobby : CORE::CObservingNotifier`:
  * `GetState()` (`const CLobbyState&`), `GetSchema()`, `GetLocalMemberId()`, `IsBoss()`;
  * settings: `SetSetting( key, value )` (applies when the policy lets this member, else becomes a proposal), `ProposeSetting( key, value )`, `Vote( voteId, yes )`;
  * members: `SetReady`, `SetSpectator`, `RequestSlot( slot )`, `SetFaction`, `SetColour`;
  * boss: `Kick( member )`, `TransferBoss( member )`, `SetSlotKind( slot, kind, aiType )`;
  * start: `RequestStart()`, `ForceStart()`, `Leave()`;
  * events:
    * `LobbyStateChangedEvent` (the changed paths)
    * `MemberJoinedEvent`, `MemberLeftEvent`
    * `VoteStartedEvent`, `VoteEndedEvent`
    * `CountdownEvent` (seconds left / cancelled)
    * `LaunchEvent` (a `CLaunchDescription`)
    * `LobbyClosedEvent` (reason)
    * `RequestRefusedEvent` (what, reason)
  * The requests return whether they could be made; a refusal arrives as `RequestRefusedEvent`. The changed paths are `phase`, `boss`, `members`, `slots`, `vote` and `settings/<key>`; `CountdownEvent` gives -1 when cancelled.
* `Connect()` completes with the identity, `Browse()` with a `CGameListingList`, `CreateLobby()` / `JoinLobby()` with a `CLobbyResultData` (a `CILobbyPtr`).
* `CLobbyCreateParams`: name, password, max players, spectators allowed, public (internet) / LAN, the game id and version, the settings schema and initial values, the slots, the topology for the game (CLIENT_SERVER with the host as server, or PEER_TO_PEER).

### 5.2 State and schema

* `CLobbyState` (`CORE::CIDataNodeSerializable`):
  * `lobbyId`, `name`, `phase` (OPEN, COUNTDOWN, LAUNCHING, IN_GAME, CLOSED), `bossId`
  * `members`: id, name, role BOSS / PLAYER / SPECTATOR, ready, slot, latency ms
  * `slots`: index, team, faction, colour, kind OPEN / CLOSED / HUMAN / AI, AI type, member id
  * `settings` (`CORE::CValueList`, the schema's keys)
  * `activeVote` (id, what, proposer, yes / no / needed, deadline)
  * `chat` (backend + channel)
* `CLobbySettingsSchema` (`CORE::CIConfigurable`, given by the game, travels with the lobby so any client can render it):
  * a list of `CLobbySettingDef`: `key`, `label`, `type` (BOOL, INT, FLOAT, ENUM, STRING), `default`, `min` / `max` / `step`, `options` (ENUM), `changePolicy` (BOSS, VOTE, ANY, OWNER), `uiHint`, `group`;
  * plus the per-slot options (factions, colours).
* `CLobbyRules` (host side, can be replaced by the game through a factory or set on the server):
  * validates every change: slot free, faction / colour allowed, colour unique if asked, value in range, policy;
  * decides when start is allowed: everyone ready, minimum players, the start vote passed;
  * handles the edge cases:
    * the boss leaves: the boss passes to the longest present player, the lobby closes when empty;
    * a vote is cancelled when its proposer leaves;
    * becoming unready cancels the countdown;
    * the host of a LAN lobby leaves: the lobby closes.
  * Per member (OWNER) settings are keyed `<memberId>/<key>`; the boss sets a VOTE setting directly. The proposer counts as a yes; a vote passes with more than half of the players and fails as soon as it cannot pass. "Longest present" is the join order (`CLobbyMember::joinOrder`); a LAN host leaving closes the lobby when `closeWhenHostLeaves` is set.
* Value lists (settings, accounts, extra) are serialized as the attributes of a child node, which every data tree codec keeps.

### 5.3 Built-in service `gucef`

* `CGucefLobbyServer`: embeddable in any process.
  * A `DRN::CDRNNode` (listeners from its settings: tcp, udp, websocket), with one directory session (a grid of listings) and one CLIENT_SERVER `DRN::CDRNSession` per lobby.
  * The lobby state lives in the session's grids:
    * `lobby` (HOST authority): name, phase, boss, settings, the active vote;
    * `slots` (HOST): with proposals for slot requests;
    * `members`: the member's own keys (ready, faction, colour, latency) OWNER, through owner-tagged keys `<memberId>/ready`;
    * `schema` (HOST, sent once).
  * Chat, commands and votes cast are DRN stream slots; settings with the VOTE policy use DRN's PROPOSAL_VOTE.
  * It issues the join tokens (DRN signed tokens, Ed25519 with the server's key, carrying the member's identity key and the policy hash) used by the lobby session and by the game session it hands over, offers a DRN relay for the game sessions, and advertises its lobbies through a `CGameAdvertiser`.
  * The relay (`relayEnabled`) is a DRN node of its own, of the service of the game sessions (`gameServiceName`, default `drn`), at `relayListen` (by default the server's listen endpoints with the next port), with the server's game policy (`MakeGamePolicy()` of the node's). Its authenticator `gucef-lobby-relay` (`CGucefLobbyRelayAuthenticator`) takes the `token` method of the game nodes and checks the launch tokens of every game session with the server's public key only. The relay endpoints (`GetRelayEndpoints()`) are the relay node's published ones, with its key and policy hash (`key`, `policy`), so a lobby with a stricter policy of its own gets the relay too: the link with the relay runs under the relay's policy, the game's links over it under the game's. The relay takes a token of another game policy than its own (the server signs tokens for the policies it accepted only); the game checks the token's policy.
* `CGucefLobbyService`: the client. `Connect` links to a server's endpoints (a LAN host or an internet server); `CreateLobby` / `JoinLobby` / `Browse` use the directory session; a joined lobby is a `CGucefLobby : CILobby` over the lobby session.
* **LAN**: the hosting player runs `CGucefLobbyServer` in-process and joins it over a loopback channel, like any other member; others find it through the `lan` listing provider.
* **Internet**: `apps/gucefLOBBYSERVER` runs the same server standalone, with a config file for listeners, keys, limits and relay.
* One node serves the directory and the lobbies: its authenticator `gucef-lobby` takes proof of the player's identity key and the server password (if any) for the directory and a token of the directory for a lobby (see 5.5); the access policy lets a token join only the lobby session it names (`gucef.lobby.<id>`). The protocol (names, requests, messages) is in `CGucefLobbyProtocol`.
* Requests go in a stream of the client (`requests` in the directory, `commands` in a lobby) kept as history, so what is sent before the server subscribed arrives; replies, refusals, launch descriptions and direct messages are app messages to the one member. Data trees travel in DRN's wire form (no codec plugin needed).
* The grids `lobby`, `members`, `slots`, `schema` are HOST grids the validator (the lobby's `CLobbyRules`) guards; a member writes its own `<memberId>/ready`, `slot`, `faction`, `colour`, `spectator` and the boss (or the policy) `settings/<key>`. Proposals of VOTE settings and of a start are writes to the PROPOSAL_VOTE grid `proposals`; only players have that grid, so spectators do not vote. The vote is mirrored in `lobby` (`vote/...`, the yes / no counts stay with DRN).
* Boss commands (`kick`, `ban`, `boss`, `slotkind`, `start`, `forcestart`), `say`, `report` and `launch` (asked by a member that joins a game in progress) are commands; the countdown's 0 is published one tick before the launch; the latency of the members comes from the session's `drn.latency` grid every `latencyIntervalInMs`.
* `CGucefLobbyServerApp` is the server app without its main (config file read with the codec of its extension, short forms `listen`, `relayEnabled` and `relayListen`); the tests run it in their process, and the executable as a child process when `GUCEF_TEST_LOBBYSERVER_EXE` names it.

### 5.4 Launch description

`CLaunchDescription` (`CORE::CIDataNodeSerializable`, also as a JSON string):
* `gameId`, `gameVersion`, `lobbyId`, `sessionName`, `topology` (CLIENT_SERVER with `serverMemberId`, or PEER_TO_PEER with `sequencerMemberId`);
* `members` (id, name, role, slot, endpoints as URI strings, `publicKey` (the identity key), `integrity` (the member's `CIClientIntegrity` report));
* `settings`, `slots`;
* `localMemberId`, `token` (signed for this member), `relay` (the relay endpoint of the lobby server, as a fallback);
* `spectator` flag;
* `serverPublicKey` (the lobby server's Ed25519 key, hex) and `securityPolicy` (the game's `DRN::CDRNSecurityPolicy`).

The game builds its own `DRN::CDRNSession` from it (`CLaunchDescription::MakeSessionSettings()`, `GetHostEndpoints()`), or uses the endpoints with its own networking.
* The tokens of a game are DRN signed tokens of the lobby server: whoever checks them (the server member in CLIENT_SERVER, every member in PEER_TO_PEER) has the server's public key only, so no member can make a token. A token carries the member's identity key and the hash of the game's policy. `MakeAuthSettings()` is the `token` authenticator with the member's token and `issuerPublicKey`; the host uses it for its node, a joiner for `Join()` (and for its node in PEER_TO_PEER). `MakeSessionSettings()` sets the policy on the session, `ApplyToNodeSettings()` on a node of the game (with the authentication for a node others connect to). The policy the server launches with is the lobby's with `requireSignedTokens` and `requireTokenPolicyHash`, so a node of another policy is refused at the handshake and a token of another policy at AUTH.
* `relay` is the relay service of the lobby server; the host binds there in the realm of the session name as peer 1 (`GetRelayBinding()`) and `GetHostEndpoints( CDRNEndpointList& )` adds that relay endpoint after the host's own, with the relay's `relayPolicy` and `relayKey`, so a member that did not bind first links with the relay under the relay's policy.
`GetHostEndpoints()` gives the endpoints of the server (CLIENT_SERVER) or the sequencer (PEER_TO_PEER).

### 5.5 Security of the `gucef` service

* **Identity proof**: the directory's authenticator (`CGucefLobbyAuthenticator`, proof kind IDENTITY) takes the player's Ed25519 public key and its signature over `"gucef-lobby-identity-v1"`, the connection's nonce, transcript hash, service name and both node ids (`MakeIdentityChallenge()`), with the password MAC when the server has a password. The user id is the one of the key (`CPlayerIdentity::MakeUserId()`), the key goes into the auth result for the ban list. A proof of one connection does not hold on another; a key without its private key proves nothing. `hello` must name the user id of the key or none; `requireIdentityProof` (default true) refuses a client without a key, without it a declared user id is taken as before.
* **Server keys**: `keyFile` holds the server's secret (hex, made when missing; none: a new one per run). HKDF-SHA256 gives the Ed25519 key that signs the tokens (`GetPublicKey()`) and the node's static x25519 key (`GetNodePublicKey()`, unless the node settings name a key of their own). Listings carry `serverKey`, `nodeKey` and `policyHash` in their extras and endpoints with the parameter `key` (`GetPinnedEndpoints()`, also in the replies to `create` / `join` and in the LAN advertisement): a client connecting with them refuses a server with another key.
* **Passwords**: the directory password travels as an HMAC of the challenge; a lobby password is compared as digests (`CDRNCrypto::ConstantTimeEqualsHashed`). Passwords are not stored by the server beyond its config and the lobby's lifetime.
* **Bans**: the server's `DRN::CDRNBanList` (kept in `banFile`) is the node's and the relay's ban list. `KickAndBan( userId, reason, byAddress, expiry )` bans the user id, its identity key and, asked to, its address, kicks it from its lobbies and closes its directory link. The boss's `ban` keeps a member out of that lobby.
* **Rate limits**: per member a token bucket for chat (`say` in the lobby and the directory, `dm`, `report`: `chatMessagesPerSecond`, `chatBurst`) and per directory client for requests (`requestsPerSecond`, `requestsBurst`); what is over is refused with a reason.
* **Reports**: `CILobby::ReportPlayer( memberId, reason )`; the server keeps a `CPlayerReport` (reporter, reported with its key, reason, lobby, the lobby's or the game's session, time) in its report log (`reportFile`, a data tree of the codec its extension names) and notifies `PlayerReportedEvent`.
* **Client integrity**: `CIClientIntegrity` reports name -> value in `create` and `join`; the built-in `CFileHashClientIntegrity` hashes the files it is given (SHA-256). With `expectedIntegrity` the server refuses what differs; the launch passes the reports on (`CLaunchMember::integrity`) and the game decides. A report is a claim of the client.
* **Game policy**: `CLobbyCreateParams::securityPolicy`; the server refuses a lobby whose policy (as launched) is weaker than `minimumSecurityPolicy` (`IsPolicyWeaker()`: a requirement dropped, a cipher, channel type or authenticator the minimum does not allow, a limit above the minimum's). The default minimum requires encryption and sets no limits.

## 6. Chat

* `CIChatService : CORE::CObservingNotifier, CORE::CIConfigurable`:
  * `Connect( identity, CORE::CFutureResult& )`, `Disconnect()`;
  * `JoinChannel( name )`, `LeaveChannel( name )`, `GetChannels()`;
  * `SendToChannel( channel, text )`, `SendDirect( user, text )`;
  * events `ChatMessageEvent` (`CChatMessage`: channel or direct, from, text, time, id), `ChatPresenceEvent` (joined / left / away), `ChatTopicEvent`, `ChatErrorEvent`.
* `CIChatChannel`: name, topic, members.
  `CChatChannel` is the plain implementation; `GetChannel( name )` gives the channel the user is in. Presence data: channel, user, JOINED / LEFT / AWAY / BACK, reason.
* `lobby`: channels are DRN stream slots (entries `{from, text, time}`):
  * the lobby's own chat is a stream of the lobby session;
  * global channels and direct messages go through the server's directory session;
  * retained history (DRN) gives a joiner the last messages.
* `irc`: over COM's `CIrcClient`:
  * channel names are mapped (`#<prefix>-<name>`); direct messages are PRIVMSG to the nick;
  * presence comes from JOIN / PART / QUIT;
  * the IRC server (e.g. Ergo) is configured in the chat service settings;
  * the WebSocket transport is used in browser builds.
  * the default prefix is `gucef`, a name that starts with `#` is used as is; the nick is the configured one or made from the display name; without the `echo-message` capability the service reports the user's own messages itself; a QUIT is reported per shared channel.
* A lobby names its chat backend and channel in its state; the GUI shows whichever it is.
* `lobby` (`CLobbyChatService`): the lobby channel is `lobby:<lobby id>` of an attached `CGucefLobby` (members `say` through their commands, the server republishes in its `chat` stream with a history of 100); global channels are server streams `chat/<name>` with a history of 50 and joined / left entries; direct messages go to a user by name or user id, the sender reports its own. The directory connection is a `gucef` lobby service given or made from the config.

## 7. Identity

* `CPlayerIdentity` (`CORE::CIDataNodeSerializable`): `userId`, `displayName`, `accounts` (backend name → account id), `publicKey` and `privateKey` (an Ed25519 key pair, hex; the private key is not serialized), `previousUserId`. The user id is the one of the public key: the first 16 bytes of its SHA-256 in the text form of a GUID (`MakeUserId()`). `GenerateKeyPair()`, `SetPrivateKey()`, `Sign()`, `GetPublicIdentity()` (a copy without the private key). The key pair needs an `ed25519` SignatureCodec (codecspluginMONOCYPHER or codecspluginOPENSSL).
* `CIIdentityProvider`: `GetIdentity()`, `SetDisplayName()`. The `local` provider reads and writes a profile file (`$APPDATADIR$/<app>/PlayerProfile.json`) and makes the key pair on first use, the private key as the profile's attribute `privateKey`. A profile from before identities had keys gets a key pair when it is loaded; its old GUID is kept as `previousUserId`. Without a signature codec the user id is a GUID until there is one. `IdentityChangedEvent` carries the identity without its private key.

## 8. Files

* the existing `CServerListProvider`, `CPassiveURLServerListProvider` and `CServerList` (repaired);
* `gucefMULTIPLAY_CMultiplayGlobal`;
* `gucefMULTIPLAY_CGameListingEntry`, `gucefMULTIPLAY_CIGameListingProvider`, `gucefMULTIPLAY_CLanListingProvider`, `gucefMULTIPLAY_CLobbyServiceListingProvider`, `gucefMULTIPLAY_CHttpListListingProvider`, `gucefMULTIPLAY_CGameAdvertiser`, `gucefMULTIPLAY_CGameBrowser`, `gucefMULTIPLAY_CGameListingFilter`;
* `gucefMULTIPLAY_CPingService`;
* `gucefMULTIPLAY_CPlayerIdentity`, `gucefMULTIPLAY_CIIdentityProvider`, `gucefMULTIPLAY_CLocalIdentityProvider`;
* `gucefMULTIPLAY_CIClientIntegrity`, `gucefMULTIPLAY_CFileHashClientIntegrity`, `gucefMULTIPLAY_CPlayerReport`, `gucefMULTIPLAY_CTokenBucket`;
* `gucefMULTIPLAY_CILobbyService`, `gucefMULTIPLAY_CILobby`, `gucefMULTIPLAY_CLobbyState`, `gucefMULTIPLAY_CLobbySettingsSchema`, `gucefMULTIPLAY_CLobbyRules`, `gucefMULTIPLAY_CLobbyCreateParams`, `gucefMULTIPLAY_CLaunchDescription`, `gucefMULTIPLAY_CGucefLobbyServer`, `gucefMULTIPLAY_CGucefLobbyService`, `gucefMULTIPLAY_CGucefLobby`;
* `gucefMULTIPLAY_CIChatService`, `gucefMULTIPLAY_CChatMessage`, `gucefMULTIPLAY_CLobbyChatService`, `gucefMULTIPLAY_CIrcChatService`.

`CMultiPlayControl` (never implemented) is removed.

## 9. Tests

* the repaired server list: pings reach the list, entries keep their extras, no leaks;
* LAN advertise and browse on loopback through the `lan` provider;
* the browser merges, filters and pings;
* the whole flow in-process with an embedded server:
  * a boss, two players and a spectator;
  * the boss changes a setting, a player proposes one, the vote passes and fails;
  * slots, factions, colours; ready, countdown and its cancellation;
  * start, and the launch description contents for every member;
  * a spectator joins the game in progress;
  * the boss leaves and the boss passes on;
* chat over `lobby` and over `irc` (the scripted fake IRC server of the COM tests);
* identity: the profile is made once and kept, with its key pair; an old profile gets a key pair and keeps its old user id; the private key is not serialized;
* security of the `gucef` service: an identity proof holds for its key and connection only, a forged user id and a missing identity are refused; the server key persists in its key file, listings pin it and a server with another key is refused; a lobby ban and a server ban by identity and address, which survives a restart; a lobby weaker than the server's minimum policy is refused; PEER_TO_PEER members cannot make a token, and a token of another policy is refused; reports reach the report log and survive a restart; the chat rate limit; client integrity reports refused on a mismatch and passed on in the launch.
* The lobby flow runs against `CTestLobbyService` (in-memory, one process, `CLobbyRules` on the host side) in the test app, through the same interfaces the `gucef` service implements.
