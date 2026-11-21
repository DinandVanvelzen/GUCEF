# gucefLabLink

Links the machines of a home network to its k3s cluster without manual steps. The cluster announces itself on the
local network; a machine that finds it, and is allowed to, joins it as a node (Linux) and sets up its Claude Code for
the MCP services of the nodes (any machine). Machines that are switched off drop out; when they are on again they are
back.

## Roles

One program, `gucefLabLink`, runs as a service on every machine. Its role follows from the machine:

| Role | Machine | What it does |
|---|---|---|
| cluster | a k3s server node | announces the cluster, answers the machines that link to it |
| machine | any other machine | finds the cluster, links to it, joins it as a node when allowed, configures Claude Code |

## Discovery

The cluster advertises the DNS-SD service `_gucef-lab._tcp` over mDNS (`<cluster>._gucef-lab._tcp.local`), and the
same entry with UDP broadcasts for networks where mDNS does not pass. The entry holds no secrets: the cluster name,
the endpoints of the two links below, and the public key of the cluster's link. Machines only use entries whose
address is in their allowed ranges.

## Links and trust

Every link is a gucefDRN link: X25519 key agreement and an AEAD cipher (XChaCha20-Poly1305, from the codec plugin
`codecspluginMONOCYPHER`), on Windows, Linux and macOS alike.

The cluster has two endpoints:

| Endpoint | Port | Who | Offers |
|---|---|---|---|
| lab | 47810/tcp | machines with a token | the service `gucef.lab.Lab` |
| access | 47811/tcp | any machine in the allowed ranges, anonymous | the service `gucef.lab.access.Access`: asking for access |

A machine gets onto the lab endpoint with a token (DRN token authenticator, HMAC, bound to the connection):

- **Lab secret.** A machine that has the lab secret makes its own short-lived tokens with the key derived from it.
  The secret is made by the cluster (`gucefLabLink --init`) and entered once on each machine
  (`gucefLabLink --set-secret`). It never travels: tokens are made from it.
- **Approval.** A machine without the secret asks on the access endpoint. The request waits until it is approved on
  the cluster (`gucefLabLink --approve NAME`); the machine then receives a token of its own, valid for a year, and
  keeps it. Taking the approval back (`--revoke NAME`) refuses the token.

The cluster proves itself to the machine: its answer to `Hello` holds an HMAC, made with the key derived from the lab
secret, over the public key of the link the machine is connected through. A machine with the secret checks it, so a
device that pretends to be the cluster gets nowhere. A machine that was approved keeps the public key of the cluster
it was approved by and only talks to that key afterwards.

Both sides check addresses: the cluster answers only machines in its allowed ranges, a machine only uses clusters in
its allowed ranges (`--allow 172.16.0.0/24 --allow 172.16.1.0/24`). Addresses on a LAN can be faked: the ranges limit
who can try, the token decides.

## Service `gucef.lab.Lab`

| Method | Gives |
|---|---|
| `Hello` | the cluster name, the k3s version, the proof of the cluster |
| `Join(name, os, arch, ephemeral)` | what `projects/k3s/agent-node/join.sh` needs: the server URL, the k3s version and a bootstrap token valid for one hour |
| `ClientConfig` | the certificate authority of the MCP services, the MCP token, and the MCP endpoint of every node |
| `Report(...)` | (contract minor 1) what a node tells the hub about itself, see below; answers the hub time, whether the node counts as verified and the report interval |
| `CiConfig` | (contract minor 2) for an approved machine only: the CI registry, the URL of the CI gateway, the gateway token of the machine and the certificate authority of the gateway, see [CI configuration](#ci-configuration) |

The contract is `gucef.lab` 1.2. `Report` was added in 1.1 and `CiConfig` in 1.2; the other methods and their fields
are unchanged, so a machine and a cluster of different versions work together: a cluster of 1.0 answers `Report` as
not implemented, and the machine then leaves reporting alone for an hour; a cluster of 1.1 or 1.0 answers `CiConfig`
as not implemented, and the machine then does not ask for it for an hour. A cluster never needs a machine to call
either.

## Node reports and the hub

The cluster is the hub of the lab. Every machine that has linked to it keeps a link to the lab endpoint and reports
every `--report-interval` seconds (default 20): its name, operating system, architecture, agent version, uptime, node
state, CPU load, memory, free disk, graphics cards (NVIDIA, through `nvidia-smi`), the places it can run workloads in
(`native`, `k3s-agent`) and what it can run (`mcp`, `stt`, `tts`, `llm`, `llm-cpu`, `image`, from its resources).
Reporting is switched off with `--no-report`. The machine only reports to a cluster it checked: with the lab secret the
cluster has to prove it has it, an approved machine only reports to the cluster that approved it.

What the hub makes of it:

- **Online.** A node is online while reports arrive, and offline after three report intervals without one. The hub
  measures that with a monotonic clock; the times it shows and stores are its own wall clock at the moment of the
  report, not the node's.
- **Verified.** A report with the token of an approval, which is bound to one machine id, is verified. A token made
  with the lab secret is made by the machine itself, with a machine id of its own choosing: such a report is kept as
  unverified and can never take the place of a node that was approved.
- **Second instance.** Every run of the agent reports a random instance id and its start time. The newer run of a
  machine id takes over at once (a restart); an older run that reports while the newer one is heard from (a cloned
  disk) is refused and counted.
- **Bounds.** The hub keeps at most 256 verified and 64 unverified nodes, cuts text and lists of a report to fixed
  limits, and forgets a node it did not hear from for 90 days (unverified: 7 days).

### Hub database

The hub keeps what it learns of its nodes, and the history of their changes (online, offline, state, instance), in
tables of the database abstraction of GUCEF (`gucefCOMCORE` `CIDatabaseClient`): `nodes`, `node_events` and
`schema_version`. After a restart of the hub the stored nodes are known again, as unknown until they report. A heartbeat
never writes: a node is written when it changes and, while it is online, every five minutes. The credentials and the
trust of the lab (the lab secret, approvals, the key of the lab endpoint) stay in the files below.

| Option | Default | |
|---|---|---|
| `--db TYPE` | `vfstables` | `vfstables`: tables as Parquet files in a directory; `postgresql`; `none`: the nodes are kept in memory only |
| `--db-dir DIR` | `db` in the state directory | `vfstables`: the directory |
| `--db-host`, `--db-port`, `--db-name`, `--db-user`, `--db-ssl-mode`, `--db-schema` | | `postgresql`: the server and database |
| `--db-password-file PATH` | | `postgresql`: the password is read from this file; there is no option to give it directly |
| `--history-days N`, `--history-max N` | 30, 20000 | how long and how many events of the nodes are kept |

The backends are plugins that are loaded from the directory of the program: `dbpluginVFSTABLES` with
`tablecodecpluginPARQUET` and `dstorepluginJSONPARSER`, or `dbpluginPOSTGRESQL` with `dstorepluginJSONPARSER`. When the
database cannot be opened, because a plugin is missing or the storage is not available, the hub reports that in its log
and continues with the nodes in memory; it tries the database again every minute. Linking, approvals and joining do
not depend on the database. A `vfstables` directory has one writer: only one hub uses it.

## Website of the hub

With `--web` the cluster serves a website that shows every node the hub knows: online now, offline since a given
time, or unknown (no report since the hub started), verified or unverified, with its operating system, graphics cards
and their memory, what it can run, its hosts, its workloads and its agent version, online nodes first. The page asks for
its data every five seconds. Without `--web` the cluster opens no further port and behaves exactly as without the
website.

| Option | Default | |
|---|---|---|
| `--web` | off | serve the website |
| `--web-port N` | 47813 | the port of the site |
| `--web-listen ADDR` | every address | a dotted IPv4 address to bind to |
| `--web-cert PATH`, `--web-key PATH` | `tls/tls.crt`, `tls/tls.key` of the MCP directory | the certificate (chain) and private key of the site, PEM |
| `--web-allow-plain-http` | off | serve over plain HTTP when there is no certificate |
| `--web-token-file PATH` | none | the bearer token of the site; made, readable by its owner only, when the file does not exist |

**Transport.** The site is served over HTTPS. Without `--web-cert` and `--web-key` it uses the certificate of this
node that the cluster authority of `projects/k3s/llm/mcp/cluster.sh` issues to the MCP directory; a browser trusts it
once that authority (`tls/ca.crt` of the MCP directory) is in its trust store. Without a certificate the site is
served only with `--web-allow-plain-http`: the login page then warns that passwords cross the network unencrypted,
and the session cookie has no `Secure` attribute. When the site cannot be served the reason is logged and the rest of
the cluster runs as before: linking, approvals and joining do not depend on the website.

**Who may connect.** Clients outside the `--allow` ranges are refused. On plain HTTP over a loopback address the
`Host` of a request has to name this machine, so that a page that rebinds its DNS name to 127.0.0.1 is refused.
`GET /api/v1/nodes/{id}` is the one path the HTTP server hands to its router, which is not given the address of the
client: on that path the ranges are not checked, but a session or the token is still required.

**Login.** The page itself (HTML, script and style sheet, built into the program; it loads nothing from elsewhere)
holds no data and is served to every allowed client. Everything under `/api/v1` except the login requires a session
or the bearer token.

- Users log in with a user name and a password. Passwords are kept as Argon2id hashes (the key derivation codec
  `argon2id` of `codecspluginMONOCYPHER`: 64 MiB, 3 passes, 1 lane), or as PBKDF2-HMAC-SHA256 hashes (600000
  iterations) when that codec is missing, each with a random salt of its own and the parameters in its record. A
  record made with other parameters than the current ones is made again at the next successful login.
- Hashes are computed as tasks of the thread pool `gucefLabLink.web`, at most two at a time, never on the thread of the
  HTTP server. Up to 16 logins wait for their turn; further ones are answered with 503.
- A failed login is answered with the same text whatever the reason. A user name that does not exist, and a disabled
  user, cost the same hash as a real user, so the time of the answer does not reveal which users exist.
- Failed logins are counted per client address and per user name. From the sixth failure of an address, each further
  failure doubles the wait before the next attempt, from one second up to 15 minutes; from the fourth failure of a
  user name, up to 30 seconds, so that nobody can keep a user out for long. Nothing locks an account permanently.
  Each table holds at most 1024 entries.
- The bearer token is accepted in `Authorization: Bearer` and in the token field of the login page; it counts as an
  admin.
- A successful login sets a cookie that holds a random 256-bit session id and nothing else: `HttpOnly`,
  `SameSite=Strict`, and over TLS `Secure` with the `__Host-` prefix. A session ends after 8 hours without use, 24
  hours after the login (30 days with "stay signed in", whose cookie outlives the browser), at logout, and when its
  user is disabled, removed, or given another password or role. The hub keeps at most 256 sessions, 16 per user, in
  memory only: a restart of the hub ends them.
- Every POST requires the header `X-LabLink-Csrf: 1`, which a page of another site cannot send.
- Logins, logouts, failed logins and changes of users are logged with the client address; the user name of a failed
  login only when that user exists. Passwords, tokens and session ids are never logged.

**Roles.** `admin` may do everything, including managing the users of the site; `viewer` reads the status of the lab.

**Users.** The users are kept in the file `users` of the state directory, readable by its owner only and replaced
atomically, as the other files there. The first user is made on the hub:

```bash
sudo gucefLabLink --add-user NAME --role admin
```

The password is asked twice on the terminal without echo, or read from the first line of standard input; it is never
an option, which the history of the shell and the process list would reveal. A password has at least 12 characters.
The running service reads the file again for every login and, for the other requests, at most once a second, so the
commands take effect while it runs: a login sees a change at once, a session within a second. Admins can also add, disable and enable users on the page of the site.

**API.** JSON, version 1; times are unix seconds on the clock of the hub, sizes MiB (see
`include/gucefLabLink_WebJson.h` for every field).

| Method and path | Who | |
|---|---|---|
| `POST /api/v1/login` | any allowed client | form fields `username`, `password` and `stay`, or `token`; sets the session cookie, answers `user`, `role`, `viaToken`, `hub`, `tls` |
| `POST /api/v1/logout` | session | ends the session |
| `GET /api/v1/session` | session or token | the caller |
| `GET /api/v1/nodes` | session or token | `hub`, `hubTime` and `nodes`: online first, then unknown, then offline |
| `GET /api/v1/nodes/{id}` | session or token | one node by its machine id |
| `GET /api/v1/endpoints` | session or token | the services the workloads of the online nodes publish |
| `GET /api/v1/users` | admin | the users, without their password records |
| `POST /api/v1/users/add`, `POST /api/v1/users/disable`, `POST /api/v1/users/enable` | admin | form field `name`; `role` and `password` to add |

The discovery entry of the cluster carries the attribute `webPort` while the site runs. The website reads the hub
through the interface `ILabHubView` only, so that it can later run in a process of its own. `install.sh` adds `--web`
when `LAB_WEB=true` is set in its settings, and then opens 47813/tcp in the firewall for the lab networks;
`LAB_WEB_PLAIN_HTTP=true` adds `--web-allow-plain-http`.

## What a machine does with it

- **Linux, joining allowed** (`--join auto`): when it is not a k3s node yet, it runs `join.sh` with what `Join` gave,
  `--ephemeral` unless configured otherwise. `join.sh` prepares the host, joins, and installs the MCP service of the
  machine.
- **Claude Code** (`--claude-user NAME`): the authority goes to a file the user's Node.js trusts
  (`NODE_EXTRA_CA_CERTS`), and every node's MCP service is registered with Claude Code of the user as
  `<node>-llm`, with the MCP token. Nodes that are added or removed are registered or removed.

## CI configuration

The hub hands the CI configuration of the lab to its approved machines, so that the CI MCP service
(`tools/gucefMcpCi`) on a machine needs no token that is copied by hand.

**On the hub.** The CI registry is `ci.json` in the state directory (`--ci-registry FILE` names another file), in the
format of `gucefMcpCi`: an object with the list `ci` of CI definitions (`projects/k3s/lablink/ci.json.example`). The
file is at most 64 KiB, must be a JSON object with that list, and is checked with the JSON codec of the plugin
`dstorepluginJSONPARSER`; when the plugin cannot be loaded the hub says so in its log and hands out no registry. The
file is read when a machine first asks and again only when its size or modification time changes. Without the file
the hub answers that no CI is configured. The machines receive the text of the file with the top-level object
`"gateway": { "url": <gateway URL>, "caFile": "ca.crt" }`, which replaces one the file may hold; the gateway URL is
`--ci-gateway-url URL`, by default `https://<cluster name>:8792`, and must be https.

**Who gets it.** `CiConfig` answers a token of an approval only, and the machine id that token is bound to is the
identity. A token made with the lab secret names a machine id its maker chose; it is refused, and the refusal is
logged with the address of the caller. A machine that links with the lab secret therefore gets no CI configuration.

**Gateway tokens.** Each approved machine that asks gets a gateway token of its own: 32 random bytes as 64 hex digits,
made when it first asks and the same on every later request. The hub keeps at most 256. They are kept in
`ci-gateway-tokens` of the state directory, readable by its owner only and replaced atomically. For the CI gateway,
which runs on the hub box, the hub keeps the hash file (`--ci-gateway-tokens FILE`, by default `ci/gateway-tokens` of
the state directory): one `<SHA-256 of the token as hex><TAB><machine id><TAB><scopes>` per line, comment lines start
with `#`. The scopes are a comma separated list: `build` for every machine, `build,deploy` for the machines named by
`--ci-deploy-machines A,B` (names or machine ids, as `--approve` and `--revoke` take them, of approved machines only;
a name that fits no approved machine, or more than one, is ignored and said once in the log). The hub always writes
the third column; a reader takes a line without it as `build`. The gateway reads it with `CORE::ReadOwnerOnlyFile()`,
so the file is mode 0600 and belongs to the user the gateway runs as: with `--ci-gateway-user NAME` (Linux) the hub
gives it to that user before it replaces the old file; without it, or when that user does not exist (said once in the
log), the file belongs to the account of the hub, and the gateway runs as that account. The file is replaced
atomically whenever a token is made or taken away or a scope changes (a change of `--ci-deploy-machines` at the start
of the service, a change of the approvals within a minute), and only when its content differs. `--revoke` on the
command line keeps the scopes of the other machines as the file has them, unless it is given `--ci-deploy-machines` as
well. A missing directory of the file is made, mode 0755, only when it is inside the state directory; a directory
elsewhere has to exist (the hub says once in its log when it does not) and has to let the gateway user in. The state
directory is root's only by default, so a gateway that runs as another user needs a file elsewhere, in a directory the
installer prepares. `--revoke` takes the token of the machine away at once; the service also takes away the tokens of
machines that are no longer approved, by whatever means, at its start and once a minute. The token of a machine stays
the same when its approval token is renewed.

**On the machine.** A machine that links with the token of its approval asks for the CI configuration on every visit
(`--refresh`, 600 seconds by default) and writes it to the directory `ci` of the MCP directory (`--mcp-dir`, Linux
`/var/lib/gucef-mcp/ci`, Windows `%ProgramData%\gucef-mcp\ci`):

| File | Holds |
|---|---|
| `ci.json` | the registry, with the object `gateway` of the hub |
| `gateway.token` | the gateway token of this machine, one line |
| `ca.crt` | the certificate authority of the gateway (`tls/ca.crt` of the MCP directory of the hub) |

A file is written only when its content changed, through a new file next to it that is moved over it. When the hub
answers that no CI is configured, or that this machine gets none, the machine removes the three files; when the hub
does not know `CiConfig`, or cannot hand it out for the moment, they stay as they are. A machine whose approval was
taken back can no longer link to the hub and keeps its files, but the gateway no longer accepts its token.
`--no-ci` switches all of this off; the files are then left alone.

`gucefMcpCi` reads the files with `CORE::ReadOwnerOnlyFile()`, which only takes files of the user it runs as that
nobody else can read:

- **Linux.** The directory is mode 0700 and the files 0600. With `--claude-user NAME` (the user the MCP service runs
  as) the directory and the files are given to that user; without it, or when the user does not exist, they stay with
  the user of the service. The directory is opened without following a link, and the files are made, given away and
  moved relative to it, so a link in their place, which the user that owns the MCP directory could make, is never
  followed.
- **Windows.** The directory and the files have the access of the state directory of gucefLabLink: owner the
  Administrators group, access for SYSTEM and the Administrators only. A `gucefMcpCi` that runs as an ordinary user
  cannot read them; it has to run as a service or elevated.

| Option | Role | Default | |
|---|---|---|---|
| `--ci-registry FILE` | cluster | `ci.json` in the state directory | the CI registry; without the file no CI is configured |
| `--ci-gateway-url URL` | cluster | `https://<cluster name>:8792` | the CI gateway the machines are told |
| `--ci-gateway-tokens FILE` | cluster | `ci/gateway-tokens` in the state directory | the hashes of the gateway tokens, for the gateway |
| `--ci-gateway-user NAME` | cluster | the account of the hub | the user the gateway runs as, who is given the hash file (Linux) |
| `--ci-deploy-machines A,B` | cluster | none: every machine may only build | approved machines (names or machine ids) whose gateway token may also deploy; repeatable |
| `--no-ci` | machine | off | do not ask for the CI configuration |

## Commands

| Command | Where | Does |
|---|---|---|
| `gucefLabLink --init` | cluster | makes the lab secret when there is none, prints it |
| `gucefLabLink --set-secret SECRET` | machine | keeps the lab secret |
| `gucefLabLink --list` | cluster | the machines that asked for access; `ci-token` after the id of a machine that has a CI gateway token, `ci-deploy` when that token may also deploy |
| `gucefLabLink --approve NAME\|ID` / `--revoke NAME\|ID` | cluster | approves a machine / refuses its token from then on and takes its CI gateway token away |
| `gucefLabLink --add-user NAME --role admin\|viewer` | cluster | adds a user of the website; the password is asked for |
| `gucefLabLink --set-password NAME` | cluster | gives a user of the website a new password; its sessions end |
| `gucefLabLink --disable-user NAME` / `--enable-user NAME` / `--remove-user NAME` | cluster | users of the website; disabling or removing ends their sessions |
| `gucefLabLink --list-users` | cluster | the users of the website, their roles and states |

Without a command it runs as the service; `--help` lists its options. `projects/k3s/lablink/install.sh` builds, tests
and installs it as the systemd service `gucef-lablink`; `projects/k3s/cluster-node-installation-media/make-media.sh` makes packages of it and an
install medium for machines without a checkout.

## Configuration file

`--config FILE` reads options from a file before the command line. Each line holds one option of the command line,
without the leading `--` (which is accepted as well): `name = value`, `name value`, or `name` alone for an option
without a value. Lines that start with `#` or `;` are comments; quotes around a value keep spaces at its ends. When an
option is given in the file and on the command line, the command line wins; repeatable options such as `allow` add up.
Commands (`init`, `list`, ...), `config` and `console` are refused in the file, as are options the program does not
know, options without their value, and files that others than the administrators can change (POSIX: owned by root or
the user of the service and not writable by the group or others; Windows: owned by the Administrators, with access for
SYSTEM and the Administrators only). A refused file stops the program with exit code 2.

```
# lablink.conf
role = machine
allow = 192.168.1.0/24
allow = 10.0.0.0/8
state-dir = "C:\ProgramData\gucef-lablink"
no-mdns
```

On Linux no file is read without `--config`, so the systemd service runs exactly as before.

## Windows service

On Windows the program is the Windows service `gucefLabLink`, installed by `projects/k3s/lablink/install.ps1`
(removed by `uninstall.ps1`), which registers it with `sc.exe` for automatic start and restart after a failure.

- **Settings.** The service reads `%ProgramData%\gucef-lablink\lablink.conf` (the registered command line names it with
  `--config`; without `--config` the program reads that file when it exists). The command line of the process is split
  again by the program, in service mode as well, so options given on the registered command line apply too.
- **Defaults.** State directory `%ProgramData%\gucef-lablink`, MCP directory `%ProgramData%\gucef-mcp`, role `machine`
  (no k3s server runs on Windows), machine name the computer name (`--machine-name` sets another), architecture from
  the processor of the machine.
- **State directory.** Owned by the Administrators group, with access for SYSTEM and the Administrators only, not
  inherited from `%ProgramData%`. The service runs as LocalSystem and the commands run from an elevated prompt; both
  write the files of the state directory with that access, so each reads what the other wrote.
- **Start.** The service reports itself running once its role has started; until then the service control manager
  sees it starting.
- **Log.** `%ProgramData%\gucef-lablink\logs\gucefLabLink.log`, rolled over at 10 MiB, five old files kept.
- **Console.** The program is a Windows application without a console of its own. `--console` runs the service in the
  console of the prompt it is started from, and the commands need it as well, from an elevated prompt:
  `gucefLabLink --console --list`. A prompt does not wait for a Windows application; `install.ps1` starts the program
  with `Start-Process -Wait`.
- **Not on Windows.** Joining the cluster as a k3s node (`--join-script`) and the Claude Code setup (`--claude-user`).

## Ports

| Port | Use |
|---|---|
| 47810/tcp | lab endpoint (cluster) |
| 47811/tcp | access endpoint (cluster) |
| 5353/udp | mDNS |
| 47812/udp | UDP broadcast discovery |
| 47813/tcp | website of the hub (cluster, only with `--web`) |

## Files

| Path | Machine | Holds |
|---|---|---|
| `/var/lib/gucef-lablink/secret` | any, root only | the lab secret |
| `/var/lib/gucef-lablink/identity` | machine | the machine id and, after approval, its token and the cluster key |
| `/var/lib/gucef-lablink/approvals` | cluster | approved and waiting machines |
| `/var/lib/gucef-lablink/lab.key` | cluster | the static key of the lab endpoint, which machines pin |
| `/var/lib/gucef-lablink/db/` | cluster | the tables of the hub database (`vfstables`) |
| `/var/lib/gucef-lablink/users` | cluster | the users of the website: name, role, state and the hash record of the password |
| `/var/lib/gucef-lablink/claude-registrations` | machine | what it registered with Claude Code, to change only what differs |
| `/var/lib/gucef-lablink/ci.json` | cluster | the CI registry (`--ci-registry`), written by the administrator |
| `/var/lib/gucef-lablink/ci-gateway-tokens` | cluster, root only | the CI gateway token of every machine that has one |
| `/var/lib/gucef-lablink/ci/gateway-tokens` | cluster, the gateway user only | the SHA-256 of every CI gateway token with its machine id and scopes, for the CI gateway (`--ci-gateway-tokens`) |
| `/var/lib/gucef-mcp/ci/` | machine | `ci.json`, `gateway.token` and `ca.crt` for `gucefMcpCi`, owned by the user of Claude Code |

On Windows the same files are in `%ProgramData%\gucef-lablink`, with `lablink.conf` (the settings of the service) and
`logs\gucefLabLink.log`.

The cluster reads the MCP files the DaemonSet of `projects/k3s/llm/mcp/cluster.sh` delivers to `/var/lib/gucef-mcp`:
`tls/ca.crt`, `lab-token` and `mcp-nodes`.
