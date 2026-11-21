# vfspluginDRN

A VFS archive type `drn` that presents a share of another node of a DRN session as a read only archive: a network drive
between two GUCEF processes. Code that reads through the VFS (`CVFS::GetFile`, the `vfs` URI scheme, `CVfsFileSystemAccess`)
reads the remote files without knowing it.

* Names (lists, existence, sizes, modification times, revisions, digests the share knows) come from the namespace operations
  of the serving node (`DRN::CDRNNamespaceClient`, gucefDRN).
* Bytes come through the long lived handles of the resource transfer facility (one handle per file that is being read, the least recently used one closed for the next). Without a block cache the chunk cache
  of `DRN::CDRNResourceIOAccess` is the content cache, as it always was; with one (see Remote archive behaviour) the bytes are kept on disk, per block, and served again when the link is down.
* The archive is a remote archive of gucefVFS (`VFS::CVfsRemoteArchive` over `VFS::CVfsRemoteArchiveCore`): the rule of a thread that must not wait, background fetches and their events, the persistent block
  cache, offline serving, pins, digest hints and statistics are the generic layer's, shared with every other remote archive. The plugin supplies the share (`CDrnBridgeBackend`) and the bytes (`CDrnBridgeBlockSource`).
* The archive does not discover, connect or authenticate. The application makes the DRN session (a node, with a thread of its
  own (`CDRNNodeSettings::ownThread`) or worked by the application's main loop, joined to the session of the serving node) and
  gives it to the archive.
* The serving node shares a directory with `DRN::CDRNFileSystemAccessResourceProvider` registered under a share name with
  share rules (who may see it).

## Mounting

The archive settings name type `drn` and, as the actual path (or path) of the archive, one of

| form | meaning |
|---|---|
| `drn://<session name>/<member>/<share>` | the session is found by its name in the DRN session registry (`CDRNGlobal::GetSessionRegistry()`, the application registers its session there) **each time the mount is used**, so it need not exist when the mount is made (see Late session binding); `<member>` is the node id of a member of the session, or `sequencer` for the host of a client-server session; `<share>` is the name the provider is registered under. The member is looked up each time it is used. The form `drn:/<session name>/...` (what the path handling of the archive settings makes of the first one) is accepted too |
| `<connection id>\|<share>` | a connection that the application registered under an id with `CDrnBridgeGlobal::RegisterConnection( id, session, peerId )` (or the form with a member name). For applications that hold the session and the peer id themselves, for example from `DRNUTILS::CRemoteServiceConnection::GetDrnSession()` and `GetServicePeerId()` (this plugin does not link gucefDRNUTILS, it takes the session pointer and the peer id) |

The typed route skips the settings: `CDrnBridgeArchive::LoadArchive( connection, share, settings, archiveName )`.

The archive type is made known to the VFS by `CDrnBridgeGlobal::Instance()` (an application that links the plugin calls it
before mounting, or mounts through `CDrnBridgeMounter`, which does) or by loading the plugin (`GUCEFPlugin_Load` does the same).

### Late session binding

A mount of the `drn://` form does not need its session to exist: the address is checked (a URI of the form `drn://<session name>/<member>/<share>` with
all three parts, a share name a node can share under; a malformed address is refused, as is a `<connection id>|<share>` whose connection is not
registered) and the mount is made. The connection of the mount holds the session name, not the session, and finds the session in the registry
each time it is used:

* **Until the session is registered** every call answers that the share cannot be reached (`GetLastResult()`: `REMOTE_CONNECTION_PENDING` in
  `fileSystemResult`, `UNAVAILABLE` in `code`): nothing waits, and what the metadata cache holds is answered, marked offline, where the offline rules
  allow it. `IsConnected()` is false, `IsHealthy()` is true (a session that is yet to come is a link that is not up, not a fault).
* **When the session is registered** the next call works, without a new mount. The same holds for a session that is unregistered and registered
  again.
* **A replaced session**: when the session of the name is destroyed (its node was stopped) and another session of that name is registered later, the
  mount follows the new one. The handles of the old session are dead: a file that is open reads on from the new session, opened again at the revision
  it was pinned at (a file that changed meanwhile fails as changed and stays failed, as it does for a link loss). The member link and the handle
  budget move to the new session.
* A session name that two sessions want is the application's affair (the registry holds one session per name): the mount follows the one that is
  registered. The mounter does not know the session; `CRemoteShareMount` of gucefDRNUTILS, which does, refuses to mount when the name is taken by another session.

The `<connection id>|<share>` form holds the session it was registered with (`CDrnBridgeGlobal::RegisterConnection()`), as before: there the connection
is required to be registered.

### Mounting from an application: CDrnBridgeMounter

`CDrnBridgeMounter` (`vfspluginDRN_CDrnBridgeMounter.h`, namespace `VFSPLUGIN::DRNBRIDGE`) mounts one share into a `VFS::CVFS` for an application that has the
name of a session, without writing the address or the archive settings:

```cpp
VFSPLUGIN::DRNBRIDGE::CDrnBridgeMounter mounter( vfs );
VFSPLUGIN::DRNBRIDGE::CDrnBridgeMountResult result = mounter.Mount( "lobby", "sequencer", "textures", "remote/textures" );
if ( !result.mounted )
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Cannot mount: " << result.reason );
// ... reads through the VFS ...
// mounter.GetState(): STATE_NOT_MOUNTED, STATE_ONLINE (the share can be reached now) or STATE_OFFLINE (mounted, not reachable now)
mounter.Unmount();     // also done by the destructor
```

`Mount( sessionName, member, share, mountPoint, settings = NULL, archiveName = "" )` is refused, with a reason in the result, when the mounter holds a
mount already, the address is not usable (an empty part, a separator in a part, a share name a node cannot share under), the mount point is empty or the
root, the mount point is in use by an archive (the VFS resolves mounts in order, so a second archive there would overlay the first), the archive name
(default `drn:` plus the mount point) is in use, the VFS does not know the archive type `drn` (the plugin is not loaded) or the VFS refuses the mount.
`settings` is a `CDrnBridgeSettings` (the table below), handed to the archive as its archive specific settings. The mounter unmounts only the archive it
mounted: when that was unmounted by someone else, `IsMounted()` is false and `Unmount()` leaves an archive that has the name now alone.
It does not register sessions: the session is the application's (or the service kit's, see `CRemoteShareMount` in gucefDRNUTILS).

The session must outlive the archives that use it: unmount (or unregister the connection) before the session is destroyed. The
connection notices a destroyed session and answers "cannot be reached" afterwards, but cannot save a call that is in flight.
A node without a thread of its own works when the application's pulse generator pulses. A mount on such a node needs no
warning: a read made by the thread that drives that pulse generator works the node itself while it waits (see Threads).

## Settings

Archive specific settings, read from the archive's configuration (`"ArchiveSpecificSetting.<key>"`):

| key | default | meaning |
|---|---|---|
| `metadataTtlMs` | 3000 | how long listings and stats are taken as current; 0 turns the metadata cache off |
| `negativeTtlMs` | 1000 | how long "there is no such entry" is taken as current; 0 never remembers an absence |
| `maxCachedEntries` | 50000 | entries (listed entries plus stat records) the metadata cache holds; the least recently used go first |
| `serveStaleWhenOffline` | true | a mount whose share cannot be reached answers from expired metadata. With false the metadata that is held in memory is dropped when the link is seen to go (the files the block cache holds are still offered: that is what a block cache is for) | 
| `maxOpenHandles` | 12 | DRN handles the archive keeps open at once, one per open file; at least 1. When the serving member announces a lower limit the mount holds its share of that instead (see Limits of the serving side) |
| `chunkCacheChunks` | 8 | chunks of 64 KiB each open file keeps in memory |
| `readAheadChunks` | 4 | chunks asked for ahead of a sequential read |
| `callTimeoutMs` | 15000 | time a list or stat may wait for the serving side |
| `openTimeoutMs` | 15000 | time the opening of a file (and the wait for a free handle) may take |
| `listPageEntries` | 0 | entries asked per page of a listing, 0 is the serving side's page size |
| `maxEntriesPerDirectory` | 500000 | entries of one directory that are listed at most (the list is cut there) |
| `maxWalkedDirectories` | 100000 | directories a recursive list visits at most |
| `verifyMode` | `link` | how bytes are checked: `link` (the packet protection of the link), `tree` (every chunk against the hash tree of the serving side) or `treeIfAvailable` |
| `busyRetries` | 8 | times a call that the serving side refused as BUSY (a limit it keeps that the mount could not know, or that others use up) is tried again before the result is handed back as retry-later; 0 never retries, at most 20 |
| `busyBackoffMs` | 25 | the wait before the first retry; every further retry waits twice as long as the one before, at most 2000 ms per wait. With the defaults a call waits 25, 50, 100, 200, 400, 800, 1600 and 2000 ms (about five seconds) before it gives up, which outlasts what the serving side needs to get its data ready |
| `blockCacheFolder` | empty | the folder of the persistent block cache. Empty: the cache that the application made the default one of the VFS (`CVfsGlobal::SetDefaultBlockCache()`, or the `BlockCache` section of the VFS configuration), if any, else no block cache. Every archive that names the same folder uses the same cache (one cache per folder and process) |
| `blockCacheMaxBytes` | 0 | the size the cache keeps to, taken when this archive is the first to open the folder; 0 is no bound. A bound is soft: pinned files and files that are being read are never evicted |
| `blockCacheBlockBytes` | 0 | the block size of the cache (a power of two from 64 KiB to 4 MiB), taken when this archive is the first to open the folder; 0 is 256 KiB |
| `cacheIdentity` | the address | the name of the share in the cache, which does not interpret it. Empty: `drn://<session name>/<member>/<share>`, or `<connection id>|<share>` for a connection by id. Two mounts with one identity (and one cache folder) share what the cache holds; set it when the address of the share is not stable (a connection id that changes per run) or when two addresses are one share |
| `readAheadAfterBlocks` | 2 | blocks read in sequence before read ahead starts (with a block cache) |
| `readAheadBlocks` | 8 | blocks fetched ahead of a sequential read in the same run (with a block cache; `readAheadChunks` is then not used, the facility's chunk read ahead is 0 for the blocks) |
| `maxBlocksPerFetch` | 16 | blocks in one run asked of the share |
| `maxBackgroundFetches` | 8 | background fetches of this archive that run at once; at least 1. Each holds one thread of the pool while it waits for the share |
| `maxQueuedFetches` | 256 | background fetches that wait behind them; beyond that a miss of a thread that must not wait answers retry-later and raises a failure event |
| `fetchThreadPool` | `VfsRemoteOps` | the thread pool the background fetches run on |
| `verifyCompleteFiles` | true | a file that became complete in the cache is checked against its digest when one is known |
| `refetchPinned` | true | a kept (pinned) file whose current revision is not held completely is fetched again in the background while the link is up, and the pin moves to the new revision (see Pins) |
| `offlineListings` | true | while the share cannot be reached a directory is listed from the files the cache holds |
| `openRequiresCompleteFile` | false | a thread that must not wait opens a file only when every block of it is cached; else it is told "not yet available" and the whole file is fetched in the background |

## Behaviour

* **Read only.** `IsWriteable()` is false; storing, deleting, moving, making directories and opening a file for anything but
  reading are refused (`GetLastResult()` says read only). Directory watching is not supported.
* **Paths** are relative to the share, `/` or `\` separated; a leading or trailing separator makes no difference. A path with a
  `..` part is refused by the archive without asking and by the serving side again. The lists have the forms of
  `CFileSystemArchive`: names for files, names with a trailing `/` for directories, entries relative to the listed location in a recursive
  list, and with the path included `mountLocation + "/" + path relative to the share`. `maxListEntries` is honoured, name filters are
  those of `CVFS::FilterValidation()`. A recursive list walks the tree with an explicit stack, one list call per directory.
* **Metadata cache.** A list costs a request per page of the serving side (not per entry), and what a list holds answers stat calls
  of its entries. After the time, a directory is checked with its revision (`ifRevisionNot`): one small request that answers
  "unchanged"; a file's description is checked the same way with the revision the cache has of it. A change of a directory
  (an entry added or removed) shows after the time at the latest; a change of a file shows at the next check of that file.
  `InvalidateMetadata()` forgets what is known of a path, for an application that learns of a change by other means.
* **Digests.** `GetFileHash()` is the SHA-256 (upper case hexadecimal text) of the file when the share knows it, else the empty
  string. Nothing is read or computed on either side for that call; the algorithm is not the MD5 of the file system archive.
  `GetFileDigest()` (and `CVFS::GetFileDigest()`) gives the same digest as a `VFS::CVfsDigest`: the algorithm named (`SHA256`), the value in lower case
  hexadecimal; when the share knows none, the digest that the application gave with `NoteFileDigest()` (a hint), else the one the block cache verified for the file, else none.
  A thread that must not wait never asks the share for `GetFileHash()`: it gets what the metadata cache of the plugin holds.
* **Files.** A file is pinned at the revision it had when it was opened (size and modification time of the serving file system by default).
  When it changes on the serving side the opened file fails from the next read on that needs bytes of the new revision
  (`IsValid()` false, `GetLastResult()` says changed) and stays failed; opening the path again gives the new revision. A handle that
  was closed to make room for another file, or lost with the link, is opened again for the revision the file was pinned at.
  `GetFile()` ignores `memLoadSize`.
  **Without a block cache** the open takes a handle at once, as it always did, and the file is pinned at the revision of that handle. **With a block cache** the open takes no handle: it
  is made from the metadata that is known (size and revision), so the file is pinned at the revision that metadata had, which is no older than `metadataTtlMs` (3 seconds by default; the metadata
  is also dropped by `InvalidateMetadata()`, and with 0 it is asked at every open). The handle is opened when a block is missing and has to be fetched, for that revision; the share says "changed" when it is another one by
  then, so a file that changed within the time reads what the cache holds of the old revision until it needs a block that it does not hold, and then fails as changed (the next open reads the new revision). A
  file that the cache holds completely opens and reads without a handle at any time.
* **Handle pool.** At most `maxOpenHandles` handles are open (or the share of the member's announced limit, see below); when a file needs
  one and the pool is full the handle of the least recently used file that is not being read is closed. When every handle is being read the
  call waits (at most `openTimeoutMs`).
* **Limits of the serving side.** A serving node limits what one member may use (open handles, reads, namespace requests, listings, upload
  rate), and what all members use together; whichever limit is reached first refuses with BUSY. It announces the lower of both for the asking member when the
  mount first asks it for its limits (they are caps, not what is free at the moment). The bridge keeps within them:
    * **One client per (session, member).** Every mount of the same member of the same session (named the same way: the node id, `sequencer` or the peer id) uses
      one namespace client (`CDrnBridgeMemberLink`, handed out by `CDrnBridgeGlobal::AcquireMemberLink()`, gone with the last mount), whatever the share
      and whichever form of source. The client never has more STAT and LIST requests under way than the member announced and queues the rest first come
      first served, so a mount that makes many calls cannot starve one that makes few. The same member named in two ways is two members to the bridge.
    * **One handle budget.** The announced number of open handles is divided between the mounts of the member in the order they were made, each at least 1:
      with a limit of 8 and three mounts they hold 3, 3 and 2 (`CDrnBridgeArchive::GetHandleLimit()`), never more than their own `maxOpenHandles`. A mount that
      has no room left closes its own least recently used idle handle first; only a mount whose handles are all being read waits (`openTimeoutMs`) and then
      fails as busy. A mount that goes gives its share to the others at their next open. A member that announces nothing leaves every mount to its own `maxOpenHandles`.
    * **Retries.** A BUSY that still happens (limits changed after the announcement, another client of the member using the budget, a server-wide limit that
      other members use up, or a share that is getting data ready that a read needs, such as the licence of a file) is tried again `busyRetries` times with the backoff of `busyBackoffMs` before the call ends. This holds for the open of a file (also
      the first open of a block source), for `STAT` and for `LIST`; the waits are made by the thread that waits (a pool thread for the jobs of a thread that must not wait), never by a thread with the rule `FAIL_AND_FETCH`. The results of such a call are
      `CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER` in `GetLastResult()`.
* **Link loss.** The archive stays mounted: `IsConnected()` is false, `IsHealthy()` stays true (it turns false only for a share that
  does not exist or does not speak the protocol), what the metadata cache holds is answered (marked `isOffline` in the metadata),
  everything else fails and says that the share cannot be reached. When the link is back the next call asks again. The state of the link is raised as `ConnectionStateChangedEvent` once per change (from the
  events of the node and the session, and when the state is asked for), see Remote archive behaviour. What the block cache holds is served while the link is down (Offline).
* **Threads.** Calls block on the calling thread. With a node that has a thread of its own (`CDRNNodeSettings::ownThread`) that thread
  works the node while the call waits. A node without one works when the application's pulse generator pulses: the thread that drives
  that pulse generator (the main loop of the application, as recorded by `CPulseGenerator::GetPulseDriverThreadId()`) is the pump
  thread of the node (`CDRNNode::IsPumpThread()`), and a call it makes works the node on that thread while it waits, so the call is
  answered or ends at its timeout. Where the pulse generator does not know its driving thread no thread is the pump thread: a call
  then waits for the node until its timeout. A call from inside work of the node (a handler the node delivers, on its own thread or on
  the pump thread) is refused at once, because its answer could never be delivered there: the result code is `DRN_NSRESULT_WRONG_THREAD`,
  it is logged once, and nothing is pumped. A call that the metadata cache can answer does not block and is answered.
* **Last result.** The calls return bool, size or empty string. `CDrnBridgeArchive::GetLastResult()` gives the result of the latest call of the
  calling thread: the mapping onto `CORE::TFileSystemAccessResult` of the bridge design (`fileSystemResult`), the finer code of the
  namespace client (`code`: unavailable, changed, integrity, wrong thread, ...) and a message. The values the design proposes for
  unavailable, changed, busy and integrity are not part of `TFileSystemAccessResult`: they map onto `FAILED` and `IN_USE` there. A refusal for a limit of the serving side (BUSY) that the retries did not get past is
  the retry-later result of the mapping (`REMOTE_CAPLIMIT_RETRY_LATER`).

## Remote archive behaviour

These come from the generic layer (`VFS::CVfsRemoteArchiveCore`, gucefVFS), which the archive is made on; the table of settings above switches them on. The classes of the plugin that make the share a
place for it: `CDrnBridgeBackend` (stat, list, the source of the bytes, the state of the link, the statistics of the transport), `CDrnBridgeBlockSource` (the bytes of one file at one revision, read through a
`CDrnBridgeFileAccess` of the handle pool, so that the handle limits, the closing of the least recently used handle, the reopening at the pinned revision and the changed rule stay where they were),
`CIDrnBridgeChangeListener` (a file opened without a block cache that turns out changed tells the layer).

* **Block cache.** With `blockCacheFolder` (or a default cache of the VFS) the bytes of a file are kept on disk in blocks, keyed by (identity, path, revision), with a checksum per block, a size bound, least recently used
  eviction (superseded revisions first) and a lock file against a second process on the folder. Reads go through it: a block that is held is read from the disk, a block that is not held is fetched in a run of up to
  `maxBlocksPerFetch` blocks, with read ahead after `readAheadAfterBlocks` sequential blocks. The cache is shared by every archive that names the folder.
* **A thread that must not wait.** A thread with the rule `FAIL_AND_FETCH` (`VFS::CVfsThreadPolicyScope`, or `CVfsGlobal::Instance()->GetThreadPolicyRegistry()`) is never held up by the share: what is known is answered (expired
  metadata too), what is not known answers `FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE` at once (`CDrnBridgeResult::IsNotYetAvailable()`, not an outage: `IsUnreachable()` is false), a job on the pool of
  `fetchThreadPool` fetches it, and an event follows on the thread that pumps the pulse generator of the VFS: `MetadataArrivedEvent`, `ListingArrivedEvent`, `FileArrivedEvent` (with the byte range) or
  `FetchFailedEvent` (with the result, `REMOTE_CAPLIMIT_RETRY_LATER` when the queue is full), exactly one per job. A read ends at the first block that is not held with the bytes it has (and `Eof()` false).
  The rule is per thread: there is no per archive default (a rule that applies to the threads of the pool that run the jobs would have the jobs fail), and the setting `openRequiresCompleteFile` is the one rule of an archive.
  In a single thread application the main thread has the rule, the jobs wait for the answers of the node that the main thread pumps with its normal loop.
* **Offline.** While the link is down, files that the cache holds open and read without a call to the share (a copy needs at least one block cached: a file that was only listed is not available), the stat
  family and `GetFileDigest()` answer from what is held, a listing is made from the files the cache holds (and says so in its result and message: `REMOTE_CONNECTION_PENDING`, "only files kept in the cache of this computer are listed"),
  a read that reaches a block that is not held ends there with `REMOTE_CONNECTION_PENDING` and a message that names the file, the bytes and the share and member. This holds after a restart of the process: a new
  archive on the same cache folder serves the same. A copy of an older revision (superseded when the archive learned of the new one) is not served offline, unless the path is kept (pinned) and the older copy is complete and the newest revision is not: see Pins.
* **Pins (keep offline).** Keeping a file offline is an opt-in; everything else the cache holds is a copy that is thrown away least recently used first, superseded copies first, and is never served when it is not the current revision. `CVFS::FetchFile()` with `pin`, or `CVFS::SetFilePinned()` for a path that has a cache entry, keeps a path: a pinned file is not evicted, and the pin follows the newest revision of the path
  that becomes complete. `GetFileCacheState()` tells what is held.
  * *The share changed and the link is down.* When the newest revision of a kept path is not held completely (not fetched yet, or only partly) and an older revision of it is held completely, the open and the read serve the older copy:
    `CVfsCachedIOAccess::IsStaleRevision()` is true for it, `GetRevision()` and `GetSize()` are those of the older copy, `GetFileCacheState()` says `staleRevision` (with `superseded` and `pinned`), the
    offline listing offers it, and `CVfsRemoteStatistics::staleServes` counts it. The stat family answers what the metadata in memory says (the newest revision), which is not what the open then gives:
    a reader that needs the current revision looks at `IsStaleRevision()`. A copy that is not kept is never served this way.
  * *The link is up.* A kept path whose newest revision is not complete is fetched again in the background, without anyone asking: when the archive learns of the new revision (a stat or a listing that finds it), at the next
    open of the path and when the link comes back. When the new revision is complete the pin moves to it and the older copy is released; it is the first to be evicted. A fetch that fails (the link dropped) is started again
    by the next of those; nothing needs to be done by a person. Needs a block cache; the key `refetchPinned` (default true; `CVfsRemoteSettings::refetchPinned` in the generic layer) is the switch.
  * *Pins and the size limit.* A pin is a promise, so it is never dropped to make room: pinned files, and files that are open, stay when the cache is over its limit, and the bytes of what is not kept go first. To keep the limit
    meaningful a new pin that does not fit is refused: when the file sizes of the kept paths, with the new one, would pass `blockCacheMaxBytes`, `SetFilePinned()` is false and `GetLastResult()` of the thread
    is `FILESYSTEMACCESS_NO_SPACE` with a message that says so, and `FetchFile()` with `pin` ends with that result after the file was fetched (it stays in the cache like any other file). A limit that is lowered later
    below what the pins hold leaves the pins as they are. `CVfsRemoteStatistics::pinsRefused` counts the refusals. A limit of 0 is no bound and refuses nothing.
* **Events.** `ConnectionStateChangedEvent` (the link became usable or was lost) is raised once per change, from the events of the node (`LinkOperationalEvent`, `LinkClosedEvent`) and of the session
  (`JoinedEvent`, `MemberJoinedEvent`, `MemberLeftEvent`, `SessionClosedEvent`) and whenever the state is asked for (every call, `IsConnected()`), so that a change that no event told of (a session that is
  bound by name and replaced) is still told at the next call. `FileChangedEvent` is raised when a read or an open finds that the revision moved, with both revisions when known.
* **Statistics.** `CVFS::GetRemoteStatistics()` fills `VFS::CVfsRemoteStatistics`: the stats, lists and handle opens of the connection (since the archive was mounted: a connection that is shared by mounts counts
  the calls of all of them), the retries of BUSY, the handles of the pool (open, peak, evictions, reopens), what was fetched and the counters of the cache and the jobs.
* **Invalidation.** `CVFS::InvalidateRemoteMetadata()` (for a path or for all mounts) drops what the generic layer and the plugin both hold in memory, so that the next call goes to the share.
* **Measured behaviour that a later phase of performance work starts from** (debug build, loopback, one process; `vfspluginDRN_TestApp baseline`): see the design document, "As built, steps D and E".

## Not in this version

Changes pushed by the serving node, versions of a file, writes, listings and stats persisted across runs (offline browsing of a directory that was never fetched in this run lists only the files the cache holds), a
default rule for a thread per archive, and a release of the lock of the VFS during the calls of the synchronous methods (the asynchronous methods of the VFS do release it for remote archives). The helpers that connect to a known service and mount its shares are in gucefDRNUTILS
(`CRemoteShareMount`, `CRemoteShareMounts`), which does not link this plugin: it mounts by the archive type name `drn`.
