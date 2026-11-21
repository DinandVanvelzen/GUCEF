# assetstore: the asset store service on the k3s cluster

The service folder of `tools/AssetStoreService` for the scripts and templates of `projects/k3s/gucef_service` (its
`README.md` explains the kit). Nothing here is a module or code.

| file | what |
|---|---|
| `service.env` | the service file: names, image, ports, LAN discovery on `worker-x299`, storage `auto` 500Gi, resources, drain and grace, the database password as a secret file |
| `AssetStoreService_site.json` | the site settings, the ConfigMap `assetstore-config`, mounted read-only at `/opt/assetstore/config/site/AssetStoreService.json` |
| `AssetStoreService_bootstrap_container.ini` | the bootstrap file of the image: `stage_install.sh` installs it as `config/AssetStoreService_bootstrap.ini` (bin/ links to it) with every `Path` at `../plugins`; the program loads its plugins through it, before it reads the site settings |

## As configured

| item | value |
|---|---|
| ports | sessions and bulk links 47400/tcp (a bulk link is a second connection to the same port), pairing 47401/tcp (open only while a pairing window is), health 47480/tcp, admin 127.0.0.1:47403, LAN discovery 47800/udp: the program's defaults, also written in the site settings |
| network | `DISCOVERY_MODE=lan`: host network on `worker-x299` (the node of `worker.lan`), namespace `privileged` with `restricted` as warn and audit level, Service `ClusterIP`; the advertised endpoints are `tcp://worker.lan:47400` and `:47401` |
| state | everything on the claim at `/data`: `state/` (devices, bans, admin token), `staging/`, `commands/`, `credentials/` (the credential store and its key, made at the first start; so no credentials key Secret), the library `library/main` (storage location `main`) |
| storage | `STORAGE_CLASS=auto`: on the cluster with `longhorn-raid` the claim is `longhorn-raid`; `DATA_SIZE=500Gi`; a claim's class never changes afterwards (see the kit's "Storage") |
| database | PostgreSQL `my-postgres.postgresql.svc.cluster.local:5432`, database and role `gucef_assets`; the password only as the file `/secrets/assetstore-db/password` (Secret `assetstore-db`, key `password`, mode 0400): the program refuses a password in its settings |
| resources | request 250m CPU and 1Gi, limit 32Gi memory (no CPU limit): a pack conversion holds a model set in memory and the reprocessing jobs of different packs run in parallel (see "Reprocessing packs"). `memorySoftLimitMB` 28672 in the settings logs a warning before the limit; the reprocessing admits jobs within 70 percent of the limit (`AssetStoreService.reprocess.memoryBudgetMB` 0). Two conversion threads for ingests (`ConverterHook.threads`). Edit `MEMORY_LIMIT` and `memorySoftLimitMB` together (the tests check that the soft limit stays below) |
| lifecycle | `drainSeconds` 20 = `DRAIN_SECONDS`, grace 45 s, startup up to 300 s (catalogue migrations) |
| `/tmp` | an emptyDir of 2Gi (`TMP_SIZE`): HOME and the cache directory of the image |
| plugins | the sixteen plugin `RuntimeDependencies` of the program's `ModuleInfo.xml`, all required, loaded from `plugins/` through the bootstrap file (no `PLUGIN_LINKS`); a missing one ends the program with exit code 3. Order: codecs and databases, the archives of the VFS (`vfspluginLIBARCHIVE`, `vfspluginUNITYPACKAGE`), the image codecs (`imgpluginFreeImage`), the importers (`assetpluginGLTF`, `assetpluginFBX`, `assetpluginUNITY`, `assetpluginGODOT`, `assetpluginUNREAL`) and the Ogre cooker (`assetcookerOGRE`) last, so that everything it uses is there when it loads |
| conversion | every source format of the importers is imported into the intermediate (glTF) and cooked for Ogre 1.x (`ConverterHook.cookTarget` `ogre`); the layout of a pack is `original/`, `intermediate/` and `gucef-ogre3d/` below the storage location (see "Conversion and restart") |
| external tool | `ooz_extract` (the Kraken family decompressor of the Unreal asset reader, GPL-3.0-or-later, `dependencies/ooz`), also a `RuntimeDependency` of the program (an executable one, so it is built into `bin/`): `EXTRA_PROGRAMS="ooz_extract=dependencies/ooz"` stages it as `/opt/assetstore/bin/ooz_extract` next to the program, with its `COPYING` and `README.txt` in `/opt/assetstore/licences/ooz/` |

## Deploy

The coordinator works on the workstation with the GUCEF checkout; the image is imported on `worker-x299` by someone who has sudo there.
`KUBECTL="ssh k3smaster kubectl"` reaches the cluster; `kubectl` is also needed locally for the offline kustomize build.

1. Build the image (coordinator, docker; the build target `GUCEF_exe_AssetStoreService` must have generated build
   files):

   ```
   projects/k3s/gucef_service/scripts/build_image.sh --env projects/k3s/assetstore/service.env --tag dev --no-import --save <dir>/asset-store-service-dev.tar
   ```

2. Import it on the node (needs sudo):

   ```
   scp <dir>/asset-store-service-dev.tar k3smaster:
   ssh -t k3smaster sudo k3s ctr images import asset-store-service-dev.tar
   ```

3. Label the node once (coordinator, or whoever has the kubectl access):

   ```
   ssh k3smaster kubectl label node worker-x299 gucef-service-host=true
   ```

4. Check, then make the namespace (coordinator). The first real run creates the namespace and stops because the Secret
   is missing:

   ```
   KUBECTL="ssh k3smaster kubectl" projects/k3s/gucef_service/scripts/deploy.sh --env projects/k3s/assetstore/service.env --dry-run
   KUBECTL="ssh k3smaster kubectl" projects/k3s/gucef_service/scripts/deploy.sh --env projects/k3s/assetstore/service.env
   ```

5. Create the database password Secret (coordinator) from the 0600 env file of the role, in a subshell and through a
   pipe, so the value is in no argument, no file of a repository and no output:

   ```
   ( set -a; . ~/.config/gucef/postgresql-gucef_assets.env; printf '%s' "$PGPASSWORD" ) | ssh k3smaster kubectl -n assetstore create secret generic assetstore-db --from-file=password=/dev/stdin
   ```

6. Deploy (coordinator): resolves the class, applies, waits for the rollout:

   ```
   KUBECTL="ssh k3smaster kubectl" projects/k3s/gucef_service/scripts/deploy.sh --env projects/k3s/assetstore/service.env
   ```

7. Look at it:

   ```
   ssh k3smaster kubectl -n assetstore get pod,pvc,svc -o wide
   ssh k3smaster kubectl -n assetstore logs deployment/assetstore
   ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin status
   ```

   Pairing a device: the same with `--admin pair-open` (the code is in that answer only).

A new image tag: set `IMAGE_TAG` (or build with `--tag dev` again and `kubectl -n assetstore rollout restart
deployment/assetstore`), import it, run step 6.

Firewall of `worker-x299`: allow 47400/tcp, 47401/tcp and 47800/udp from the LAN prefix; deny 47480/tcp (health) from
the LAN. The admin port is on the node's loopback.

## Use a share for the library

The library (the files of the storage location: `packs/<source>/<pack>/...` and the download staging below it) can
live on a network share of the node instead of the claim, through the kit's `components/host-share` and a local install
configuration outside the repository (the kit's README, "Local install configuration" and "A network share of the
node"). The claim keeps `state/`, `credentials/`, `commands/` and the upload staging (`AssetStoreHost.stagingDirectory`,
default `/data/staging`).

1. On the node of the pod (`DISCOVERY_NODE_NAME`), once, with sudo: mount the share, for CIFS for example with a line in
   `/etc/fstab` like
   `//<server>/<share> /mnt/<share> cifs credentials=/etc/samba/<credentials file>,uid=10001,gid=10001,file_mode=0664,dir_mode=0775,iocharset=utf8,vers=3.0,x-systemd.automount,noauto,_netdev,nofail 0 0`
   (`uid`/`gid` 10001: the pod's user must write; the credentials file stays on the node, root, mode 600), then
   `systemctl daemon-reload` and start the `.automount` unit.
2. Create the directory of the service on the share (the hostPath is of type `Directory`: it is not created).
3. In a local directory outside every repository:
   * `service.env.local` with `HOST_SHARE_HOST_PATH='/mnt/<share>/<folder of the service>'` (single quotes when the path
     has spaces) and, if wanted, `HOST_SHARE_MOUNT_PATH` (default `/share`) and `HOST_SHARE_FSTYPES`;
   * `site.json`: a copy of `AssetStoreService_site.json` whose storage location has `"path": "/share"` (the mount path)
     and its own id and name, for example
     `"locations": [ { "id": "<id>", "name": "<name>", "path": "/share", "vfsRoot": "assetstore/main" } ], "defaultLocationId": "<id>"`.
4. Deploy with `--local-dir <that directory>`. The init container `host-share-check` stops the pod while the share is
   not mounted on the node (or not writable by uid 10001), so nothing is written to the node's disk instead.

The catalogue keeps, per pack, the id of its storage location and its path relative to the location root; the VFS root
(`vfsRoot`) is only where the process mounts the location and may stay `assetstore/main`. Changing the location id
(or adding a location) leaves the packs of the old id in the catalogue: while no location of that id is configured,
their files are not reachable (the store answers that the storage location of the pack is not reachable), and the row of the old location in the catalogue table `store_locations` stays (the store writes the rows of the configured locations only); remove such packs (`Curation/RemovePack`,
with `deleteFiles` while the old location is still configured) or keep the old id. Keeping the old id with a new path
points its packs at the new folder, where their files must then be.

## Library sync: where the downloads go and the space rules

Every download and every ingest ends up in the storage location of the library. For an installation with a network share
that is the share mounted on the node (through "Use a share for the library" above, with its library folder as
`HOST_SHARE_HOST_PATH`); the claim holds no pack files.

* **Staging.** A download is written to a staging folder first and moved into the pack when it is ingested. By default
  the staging folder is below the storage location itself (on the share). The primary store can take a local scratch
  folder (`CPrimaryAssetStoreSettings::scratchStagingPath`, for example a folder on a local disk of the node mounted
  into the pod) and then stages there first; the section `Storage` of the site settings of the service program does not
  have a key for it yet, so this installation stages on the share. The scratch folder is opportunistic only: when the scratch folder is not configured,
  cannot be created or mounted, its free space cannot be read, or it has less free space than the item needs (its
  expected size, or 1 GiB when the size is not known, plus 64 MiB), the download is staged on the library location
  instead, without an error and without a wait. The first time that happens the service logs one warning with the
  reason. A scratch folder is never required.
* **Free space.** The space check of the library sync applies to the volume of the library location only (the share),
  not to the scratch folder: an item is admitted when the free space of that volume, less the items in flight and the
  item itself (each times the disk factor of the profile, default 2.0), stays above the reserve of the profile
  (`reservePercent` of the volume size or `reserveBytes`, whichever is larger; defaults 5 percent and 20 GiB). A
  scheduled or ordered sync that does not fit waits with the reason "waiting for disk space" and goes on when space is
  free.
* **Volume information that cannot be read.** When the operating system reports no size and no free space for the
  volume (some network file systems), the service admits the items and logs one warning per run; it does not refuse
  and does not wait. A CIFS mount reported its sizes correctly when read through `GetFileSystemStorageVolumeInformationByDirPath()`.
* **Downloads a person selects** ("Download selected" in the workbench, a sync with `manual` set) do not keep the
  reserve of the profile and use a factor of 1.0: an item is refused, and counted as failed with the reason "not
  enough free space on the library volume", only when it does not fit on the volume at all. Such a download never
  waits for space.

* **Names and files on a share (CIFS).** A pack folder is the title of the item made safe for Windows and CIFS
  (`: ? * < > | "` and control characters become `_`, no trailing dots or blanks, no device names, at most 120 bytes);
  a name that is taken, compared without case, gets ` [<short id>]`, and so does a folder that exists on the share but is
  not in the catalogue (the packs of another catalogue on the same folder, such as the probe's, are never taken over). A
  download whose file names differ only in case is refused. Downloads are written as `.part` in the staging folder on the
  share and renamed; a pack is moved into place with a rename; `pack-index.json` is written to a temporary file and
  renamed. The node must present the share to uid 10001 (`uid=10001,gid=10001,file_mode=0664,dir_mode=0775`): the service
  sets no file modes there.

## Conversion and restart

**What the image converts.** A pack that arrives (ingest) is read by the importer that recognises it: glTF, FBX
(`ufbx`), Unity projects and `.unitypackage` archives (the archive is read in place as the project tree it holds),
Godot 4 text scenes and Unreal editor packages (`.uasset` and `.umap`; a `.mhpkg` is an archive of them and is read in place like a zip; `ooz_extract` for Oodle). The result
is the engine neutral intermediate (glTF) in `<location>/intermediate/<pack>/`, and the cooker of the Ogre 1.x runtime files
writes `<location>/gucef-ogre3d/<pack>/<profile>/` (meshes, skeletons, material scripts, DotScene files, colliders, textures);
the originals are in `<location>/original/<pack>/`. The cooker runs headless: it makes the Ogre root with the software
buffers, needs no render system and no Ogre plugin (no `OgrePlugin_*` file and no `plugins.cfg`), and reads and writes
pictures through `imgpluginFreeImage` and `codecspluginZLIB`. The pack index (`pack-index.json`) is written at the end
of a conversion.

**Settings** (all in the site file, `AssetStoreService_site.json` has them):

| key | default | what |
|---|---|---|
| `ConverterHook.cookTarget` | empty in the program, `ogre` in the site file | the cooker whose output the store keeps; empty: the intermediate only. The name of the cooker is `ogre`; `gucef-ogre3d` (the folder its output goes to, and what the workbench sends for "Ogre runtime files") is accepted as an alias of it |
| `AssetStoreService.primaryStore.indexAfterIngest` | `false` in the program, `true` in the site file | an ingest that left the pack without a pack index (nothing was converted, keep `original`, a converter that wrote none) has the converter hook build one, as the job `index-pack` does; a pack without a converter that can stays ingested without one |
| `AssetStoreService.primaryStore.sources` | none: every source is `manual` | one object per source (an online store by its id, or `disk`): `{ "sourceId": "<store id>", "mode": "automatic" or "manual", "keep": "both" or "original" or "converted", "locationId": "" }`. `automatic`: every finished download of that store is ingested without being asked; `manual`: a download that was queued to be ingested is ingested, a pack that is only offered waits. A policy set through the service (a workbench, the `SetSourcePolicy` call) replaces the one of this file and is kept in the state file |
| `AssetStoreService.stateDirectory` | empty in the program, `/data/state/assetstore` in the site file | the folder of the files that carry the work over a restart, on the claim; mounted in the VFS at `assetstore-state`. Empty: nothing is kept |
| `AssetStoreService.thumbnailsDirectory` | empty: the folder `thumbnails` in the default storage location | the folder on the disk (environment variables are resolved) that holds the stored pictures of the assets, mounted in the VFS at `assetstore-thumbnails/thumbnails`. Set it to keep the pictures elsewhere, for example on the fast claim while the library is on a share; the folder is made when it is missing. The pictures that are in the old folder are not moved: copy the folder `thumbnails` of the location over to the new folder, or let the workbenches render and put the pictures again. Only devices with the role `owner` may put or replace a picture; the other roles with access (`viewer`) read them |
| `AssetStoreService.downloads.queueFilePath` | `assetstore-state/downloads.json` when `stateDirectory` is set | the queue of the downloads (VFS path, JSON); saved at each change of a job |
| `AssetStoreService.resumeIngestsOnStart`, `ingestResumeAttempts` | `true`, `3` | at the start, the ingests that stayed staged resume from the step that did not complete; an ingest that was resumed by that many starts is left (one that ends the process each time is not tried for ever) and is resumed through the service |

**Restart.** The queue file keeps the downloads with their state: a job that was running is queued again at the start, a
paused or failed one stays as it is, and the downloads resume when the program starts (`startDownloads`). The state file
`/data/state/assetstore/service-state.json` keeps the downloads that were queued for ingest (with their keep policy and
location), whether their ingest was requested, the source policies set while the program ran, and the resume counts. At
the start, a finished download without a requested ingest is ingested, and the staged ingests resume; everything else
carries on from the files on the claim. A catalogue that is not reachable at the start leaves this to the start that
reaches it (the program opens the catalogue again every `catalogueRetrySeconds`).

## Sign in to online stores on the server

The sessions of the online stores live on the server: in the credential store of the program under `/data/credentials`
(sealed, the directory and its files for the service user alone, the key made at the first start). A person drives a sign
in from a paired device with the role `owner`; the steps come to the device as prompts (an address to open in the person's
own browser, a field for a code) and the answer goes back sensitive and encrypted. Only devices with the role named in
`AssetStoreHost.signInRoles` (default `owner`, never `mcp`) can begin, answer, cancel or sign out, list the prompts or use
the keyring, and only their sessions receive the prompts.

* **Stores whose sign in needs a person and a browser (captcha, second factor).** The service has no browser. The
  person signs in to the store in their own browser and hands the session over to the service from their workstation
  (`ImportWebSession`, a paired device with the role `owner`; the client tool is part of the store plugin). The service
  keeps the cookies of the store's own domains sealed in `/data/credentials`, checks them once with the store, and a
  library sync that waits for a sign in goes on. When the store ends the session the sync waits with the notice
  `SYNC_SIGN_IN_NEEDED` until a new session is handed over (library sync design 13.20). Store plugins are loaded through
  the bootstrap file like the other plugins and are named in the site settings (`"Plugins": [ ... ]`); the settings of a
  store go in the section `Stores.<store id>` of the site settings. A plugin that downloads over https needs the https
  client plugin `httpclientpluginCURL` (libcurl from `dependencies/curl`, the CA bundle of the `ca-certificates` package
  of the runtime image) in the image, and the pod needs outbound https (443) to the hosts of the store; in lan mode that
  is the egress of the node. A store may set a size guard per item and format (`maxDownloadBytes`, `0` is no limit) and
  a cache directory for large downloads (`chunkCacheDirectory`), which belongs on the claim, not on the 2Gi `/tmp`.
* **Other stores that sign in on their web site** show as signed out with the message that the service has no browser;
  their sign in answers `BROWSER_UNAVAILABLE`. A session that was saved earlier or handed over (`ImportWebSession`,
  devices with the role `owner`) is used.
* **The keyring** holds saved store passwords sealed with a code the person knows (`/data/credentials/keyring.bin`). The
  code is never stored: after every start the keyring is locked until it is unlocked, and it locks again after
  `Credentials.keyringIdleLockMinutes` (240) without use. A device with the role `owner` sends it with `UnlockKeyring`, or on the node
  (the code on the standard input or in a file, never an argument, never in a log):

  ```
  ssh k3smaster kubectl -n assetstore exec -i deployment/assetstore -- /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin keyring-setup      # once, the first code
  ssh k3smaster kubectl -n assetstore exec -i deployment/assetstore -- /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin keyring-unlock
  ... --admin keyring-lock | keyring-status
  ```

  (`-i` passes the standard input; `--code-file <file>` reads the first line of a file of the pod instead.)

## To verify on the cluster

* the pod starts with the Secret file readable by user 10001 (`root:10001 0440` through `fsGroup`) and reaches READY
  against PostgreSQL; `pg_hba` of `my-postgres` admits the connection, which comes from the node address
  (host network), not from the pod network;
* the `longhorn-raid` volume of 500Gi is scheduled on a disk with the `raid` tag of `worker-x299` (Longhorn enforces
  the size: the disk needs the room);
* the probes, LAN discovery and the endpoints as in the kit's "LAN discovery" checks.

## Reprocessing packs: newer conversion, original downloaded again, rollback

The service makes the outputs of a pack again with the converter of the running image: the intermediate resources
(`library/main/intermediate/<pack>/`) and the engine target resources (`library/main/gucef-ogre3d/<pack>/`). A
regeneration writes to `staging/regen-...` and swaps the result in; the replaced outputs are kept as
`library/main/generations/<pack>/previous/` until the next successful regeneration, so `reprocess-rollback` undoes it.
Originals are never changed. One job per pack at a time; the jobs of different packs run in parallel (below), the
others wait in the job list; each step writes an audit entry in the ingest history of the pack. Plan first (read only), then the job, followed to its end
(the answer has the counts before and after, the improved and regressed assets and the report path per pack):

    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin reprocess-plan <pack id> [<pack id> ...] --steps intermediate
    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin regen-intermediate <pack id> [<pack id> ...] --follow true
    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin reprocess-status <job id> [<job id> ...]

The commands: `regen-intermediate` (importers again, then the cook; `--no-cook true`: intermediate only), `regen-target`
(the cook again from the intermediate), `redownload` (the original again from its store, verified; needs a signed in
store; `--allow-large true` for the size guard), `reprocess` (`--steps redownload,intermediate,target`),
`reprocess-rollback`, `reprocess-plan`, `reprocess-status`, `reprocess-cancel`. Options: `--failed true`, `--statuses a,b`,
`--kinds a,b`, `--assets <paths or globs>`, `--targets ogre`, `--accept-errors true`, `--dry-run true`, `--follow true`.
Limits for machine callers (MCP): the child `reprocess` of the service configuration (`maxJobsPerPackPerHour` 4,
`maxMachineJobsPerHour` 20, `maxMachineRedownloadsPerDay` 6; a device with the role `owner` and the admin command line
are not held to them unless `limitOwner`). Design: `gucefASSETSTORE_RemoteService_Design.md` section 17.

**Parallel jobs.** Packs are independent of each other (the assets of one pack are converted together in one job), so the
jobs of different packs run at the same time, within a budget of threads and memory that the service computes from the
node and its container at the start (logged as "The reprocessing of packs may use ..."; `reprocess-status` answers it
as `reprocessing` with what the running and waiting jobs use). Settings, all in `AssetStoreService.reprocess` of the
site file (`AssetStoreService_site.json` has them with their automatic values):

| key | default | what |
|---|---|---|
| `maxConcurrentJobs` | 0: automatic, a quarter of the logical cores, at least 1, at most 8 (28 cores: 7) | jobs that run at a time; the others wait in the job list with the state text `waiting for slot` and hold no thread |
| `threadsPerJob` | 0: automatic, (logical cores - `reservedCores`) / jobs at a time, at least 1 (28 cores: 3) | converter threads of one job, on a thread pool of its own (`AssetStoreService.reprocess<slot>`); they replace `ConverterHook.threads` for that run, so the busy converter threads of all jobs stay within the cores the service does not keep |
| `reservedCores` | 2 | cores the automatic `threadsPerJob` leaves to the service (sessions, ingests of the library sync with `ConverterHook.threads`, the database) |
| `memoryBudgetMB` | 0: automatic, 70 percent of the memory limit of the container (cgroup v2 `memory.max`, v1 `memory.limit_in_bytes`), of the node when the container has none (32Gi: 22937 MB) | the estimated memory the running jobs may hold together; a job that does not fit waits (`waiting for memory`), in the order of the queue; a job estimated at more than the whole budget runs alone |
| `memoryPerJobMB` | 2048 | the least a job with a conversion step is estimated at |
| `memoryLargestAssetFactor` | 4 | a job is estimated at this many times the largest asset of its pack when that is more than `memoryPerJobMB` |
| `maxConcurrentWritesPerLocation` | 0: no limit | jobs that write to the staging of the same storage location at a time (`waiting for location`) |
| `maxQueuedJobs` | 64 | jobs that wait or run; one more is refused as busy |

A running job shows `running (<threads> thread(s), <MB> MB)` in the job list, `reprocess-status` and the MCP `job_list`.
The cook step of the Ogre target (Ogre is not thread safe there) runs for one job at a time; the imports and the
intermediate of the jobs run in parallel. The jobs that wait or run are kept in `/data/state/assetstore/service-state.json`
(`reprocessJobs`): after a restart they wait for their turn again, and a job that ran starts from the beginning (its
outputs are swapped in only at its end, so a stopped job left nothing half done). The settings can be changed while the
service runs (see "Run-time settings" below): running jobs finish with their share, the waiting jobs are looked at
again at once.

**A library on a network share.** Every job reads the originals of its pack and writes its staging folder in the storage
location, so seven jobs on a CIFS share are seven streams of reads and writes over the network at once. When the share
or the network is the bottleneck (the jobs spend their time waiting for files rather than converting), set
`maxConcurrentWritesPerLocation` to 1 or 2: the jobs of that location then take turns while those of other locations
still run.

## Run-time settings: read and change them while the service runs

Selected plain settings of the section `AssetStoreService` can be read by every paired device and changed by a device
with the role `owner` (the workbench, Server > Settings..., or its console: `server settings`, `server set <key> <value>`,
`server reset <key>`) and by the admin command line, without a restart. A change is checked first (one key that is
not a setting, or a value outside its type or range, changes nothing and every such key is named), then applied at
once or, for the settings that say so, at the next start. The changes are kept as overrides in
`/data/state/assetstore/settings-overrides.json` (written to a temporary file and renamed in place) and applied over the
site file at every start; a reset puts the value of the site file back. Every change and every refusal is audited in
`/data/state/assetstore/settings-audit.json` (the newest 500 entries: time, device id and name, key, old and new value
of the public settings) and in the log; the connected clients are told of each change.

| keys | apply |
|---|---|
| `reprocess.*`: `enabled`, `maxConcurrentJobs`, `threadsPerJob`, `reservedCores`, `maxQueuedJobs`, `maxJobsPerPackPerHour`, `maxMachineJobsPerHour`, `maxMachineRedownloadsPerDay`, `limitOwner`, `downloadTimeoutMinutes`, `memoryBudgetMB`, `memoryPerJobMB`, `memoryLargestAssetFactor`, `maxConcurrentWritesPerLocation` | at once (the waiting jobs are looked at again) |
| `downloads.*`: `maxConcurrentJobs`, `maxConcurrentJobsPerProvider`, `maxAttempts`, `retryBaseDelayInMs`, `retryMaxDelayInMs`, `progressIntervalInMs`, `connectTimeoutInMs`, `maxBytesPerSecond` | at once (the transfer limits from the next transfer) |
| `downloadStagingKeepHours` | at the next staging sweep |
| `signInPromptSeconds` | at the next step of a sign in; its value is read by devices with the role `owner` only |
| `resumeIngestsOnStart`, `ingestResumeAttempts` | at the next start (shown as pending until then) |

Everything else stays in the site file only and is neither readable nor changeable by a client: paths and folders,
ports, plugin lists, the database, credentials, the keyring, and `allowSettingsChanges` itself. The sync profiles of the
stores keep their own operations and are shown on the same page of the workbench.

`AssetStoreService.allowSettingsChanges` (default `true`): with `false` every change by a client is refused with
"settings changes by clients are switched off on this server" (and audited); reading still works. The admin command
line is the site administrator: it may change the settings either way, and is audited as `admin`. Devices with the
machine role `mcp` and viewers read the public settings only (MCP: the read-only tool `settings_get`) and are refused a
change.

    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin settings-describe
    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin settings-set reprocess.maxConcurrentJobs=4 downloads.maxConcurrentJobs=2
    ssh k3smaster kubectl -n assetstore exec deployment/assetstore -- env GUCEF_CONSOLE_LOGGER=none /opt/assetstore/bin/AssetStoreService --config /opt/assetstore/config/site/AssetStoreService.json --data-dir /data --admin settings-reset reprocess.maxConcurrentJobs

Design: `gucefASSETSTORE_RemoteService_Design.md` section 18.
