# gucefMcpCi

`gucefMcpCi` is an MCP server that hands the build matrix of a repository to the lab CI. A language model gives it a
local repository; the server sends everything that is not published yet (commits that are not pushed, modified, deleted
and untracked files) as a scratch snapshot, starts a run of the full matrix (or a filtered part of it), and answers
status, report and failure details in a short, bounded form. The working tree, the index and the branches of the
repository are never changed.

The model never sees the CI product. Forgejo Actions is the first backend; others can be added behind the same
interface (`ICiBackend`).

A run can also build container images, and an image of a green run can be rolled out to a Kubernetes Deployment that the
CI registry lists for the repository (see Deployments).

## Modes

The same twelve tools are offered in every mode; only where the work happens differs.

| Mode | Where it runs | What it does |
|---|---|---|
| `direct` | the gateway machine, or a machine without a gateway that has CI credentials of its own | works with the CI itself (`CDirectCiService` over `CForgejoBackend`) |
| `proxy` | any lab machine, next to the model | makes the snapshot locally and forwards every call to a gateway over HTTPS (`CRemoteCiService`); holds no CI credentials and needs a route to the gateway only |
| `gateway` | the CI side, on the machine of the lablink hub | serves the gateway API over HTTPS for proxies, over `CDirectCiService`; holds the registry and the CI credentials. It is no MCP server |
| `auto` (default) | | `proxy` when a gateway URL and a gateway token are there, otherwise `direct` when a CI registry and a CI token are there, otherwise no tools are offered and the server instructions say what is missing |

In proxy mode the instructions of the server also state whether the gateway could be reached when the server started.
While it cannot be reached, every tool answers with an error that says so, within the time limits of its requests.

## Tools

| Tool | Effect | Purpose |
|---|---|---|
| `ci_resolve_repo` | read-only | the CI id of a repository from its remotes, or "no CI is set up"; nothing is guessed |
| `ci_describe` | read-only | toolchains, C++ standards, products and modes of a CI id; with `repo_dir` from the CI files of that working tree |
| `ci_submit` | additive | sends the snapshot and starts a run (full matrix by default, optional filters, mode `build` or `test`, `dry_run`); reports progress while a large snapshot is sent and honours cancellation. `images` also builds container images (names, or `["all"]`); `commit` builds a commit of the published repository instead of a working tree, sent by the machine that works with the CI from its own clone |
| `ci_status` | read-only | state, job counts and failed jobs; `wait_seconds` waits within the call until the run is finished |
| `ci_report` | read-only | per job and product what failed, the first errors with `file:line`, failed test cases |
| `ci_job_details` | read-only | one job: the digest of its logs, the end of a log, or its tests; the full logs are saved on the machine of the server and their absolute paths are returned |
| `ci_cancel` | destructive | stops a run |
| `ci_cleanup` | destructive | removes scratch branches and reports older than a number of days |
| `ci_images` | read-only | the images a run built: tag, digest, status, and whether the run built a published commit |
| `ci_release_status` | read-only | what a deployment target runs now: image and digest, change cause (CI id, run, commit, machine, time), replicas, revision history |
| `ci_deploy` | destructive, idempotent | rolls an image of a green run out to a listed target; needs `confirm=true` and the scope `deploy`; its description tells the model to deploy only when the user asked for it and to show `ci_release_status` first |
| `ci_rollback` | destructive | returns the Deployment of a target to its previous revision; needs `confirm=true` and the scope `deploy` |

Only repositories that the CI registry knows are built.

## Options

| Option | Modes | Meaning |
|---|---|---|
| `--mode auto\|direct\|proxy\|gateway` | all | see Modes; default `auto` |
| `--ci-config FILE` | direct, gateway, auto | the CI registry (`ci.json`); default: `ci.json` of the lablink directory |
| `--forgejo-token-file FILE` | direct, gateway | the API token of the CI, owner-only; default: `forgejo.token` of the lablink directory |
| `--lablink-ci-dir DIR\|none` | all | the directory lablink writes; default `/var/lib/gucef-mcp/ci` (Linux) or `%ProgramData%\gucef-mcp\ci` (Windows) when it exists |
| `--gateway-url URL` | proxy | the gateway, `https://host:port`; default: the `gateway.url` of `ci.json`. Plain `http` is accepted for 127.0.0.1 and localhost only |
| `--gateway-token-file FILE` | proxy | the token of this machine for the gateway, owner-only; default: `gateway.token` of the lablink directory |
| `--ca-file FILE` | all | the authorities to trust: for the gateway in proxy mode (default: `gateway.caFile` of `ci.json`, else `ca.crt` of the lablink directory), for an HTTPS CI otherwise; default: the system trust store |
| `--reports-owner NAME` | direct, gateway | owner of the report packages when the registry entry names none; default `ci-bot` |
| `--log-dir DIR` | all | full logs of jobs, snapshot work, bundles; default `gucefMcpCi` in the temporary directory |
| `--log-file FILE` | all | also writes the log to a file |
| `--max-snapshot-mb N` | all | the largest snapshot that is sent, or that a gateway takes; default 500 |
| `--machine-name NAME` | direct, proxy | part of the scratch branch names in direct mode; default: the host name |
| `--max-wait-seconds N` | all | the longest wait of `ci_status`; default 600 |
| `--poll-interval-ms N` | all | how often a waiting status asks the CI; default 5000 |
| `--stdio` | direct, proxy | MCP over standard input and output; the default without `--listen` and `--listen-tls` |
| `--listen ADDR:PORT` | all | direct and proxy: MCP over HTTP on a loopback address. gateway: the gateway API over plain HTTP, 127.0.0.1 only |
| `--listen-tls ADDR[:PORT]` | all | direct and proxy: MCP over HTTPS. gateway: the gateway API over HTTPS, default port 8792 |
| `--tls-cert PATH`, `--tls-key PATH` | all | PEM certificate (with its chain) and key for `--listen-tls` |
| `--token-file PATH` | all | direct and proxy: the bearer token of MCP HTTP clients, made owner-only when missing. gateway: an optional static token, accepted as machine `admin` |
| `--allowed-host NAME` | all | a host name clients use for this machine (repeatable); localhost and 127.0.0.1 are always accepted |
| `--gateway-tokens-file FILE\|none` | gateway | the token digests of the machines; default `/var/lib/gucef-lablink/ci/gateway-tokens` |
| `--work-dir DIR` | direct, gateway | bare repositories per CI id, the upload staging and the audit of deployments; default `gateway` in `--log-dir` |
| `--upload-chunk-kb N` | gateway | the chunk size of uploads, at most 8192 (default) |
| `--upload-idle-seconds N` | gateway | an upload without a chunk for this long is removed with its file; default 600 |
| `--max-uploads N` | gateway | uploads at the same time; default 4 |
| `--max-pending-requests N` | gateway | requests that wait or run; default 64 |
| `--max-pending-polls N` | gateway | status requests that wait for their run; default 32 |
| `--gateway-threads N` | gateway | threads of the gateway; default 6 |
| `--max-requests-per-minute N` | gateway | per machine; default 600, 0 for no limit |
| `--kubeconfig FILE` | direct, gateway | the kubeconfig of the restricted ServiceAccount of the deployer; default `/var/lib/gucef-ci/kubeconfig`. The environment variable `KUBECONFIG` is not used |
| `--kubectl PATH` | direct, gateway | the kubectl executable; default `kubectl` |
| `--allow-deploy` | direct | the tools of this server may deploy and roll back (scope `deploy`); without it the scope is `build`. A gateway takes the scopes from its tokens file |
| `--deploy-timeout-seconds N` | direct, gateway | the longest wait for a rollout; default 600, at most 3600 |
| `--registry-token-file FILE` | direct, gateway | the token for the container registry (read:package), owner-only; default: the CI token |

Tokens are never given on the command line.

## The lablink directory

On a machine that runs lablink, the machine role writes the files of this directory, and `gucefMcpCi` takes its
configuration from them; it never calls lablink itself.

| File | Content |
|---|---|
| `ci.json` | the CI registry: `{ "gateway": { "url": "https://host:8792", "caFile": "ca.crt" }, "ci": [ ... ] }`. The `gateway` object is optional; a relative `caFile` is relative to the directory of `ci.json` |
| `gateway.token` | the token of this machine for the gateway (proxy mode) |
| `ca.crt` | the lab authority that vouches for the certificate of the gateway |
| `forgejo.token` | the CI token (direct mode only; machines in proxy mode do not receive one) |

`ci.json`, `gateway.token` and `forgejo.token` are read with `CORE::ReadOwnerOnlyFile()`, which refuses files that
others than their owner can read or write, and symbolic links. The server must therefore run as the user that owns
them. An entry of the registry is described in `include/gucefMcpCi_CCiDefinition.h`; its optional deploy block in
`include/gucefMcpCi_CCiDeployImage.h` and under Deployments.

## Snapshots in proxy mode

1. The snapshot is made locally with `CSnapshotBuilder`, as in direct mode: a temporary index, `git add -A`, one
   synthetic commit on top of `HEAD`. Only objects are added to the object store of the repository.
2. `GET /v1/ci?id=ID` names the commits the CI has (`have`): the head of the default branch of the scratch repository
   and those of its newest scratch branches. Of these, the commits the repository has and the snapshot builds on are
   left out of a git bundle; without any, the bundle holds the whole history. The bundle is made in a temporary
   repository of the server that borrows the objects of the repository, so no ref of the user is created.
3. The bundle is uploaded in chunks of at most 8 MiB, strictly in order, with its size and SHA-256 announced first.
   When a chunk fails, the proxy asks the gateway how far the upload is and resumes from there, a few times.
4. The gateway checks size and SHA-256, verifies the bundle (`git bundle verify`), fetches what the CI published when
   the bundle needs commits it lacks, takes the bundle into a bare repository per CI id (never into a working tree) and
   pushes the commit to the scratch repository as `scratch/<machine>-<utc>-<shortsha>` with the code of direct mode.
   The machine name is the one of the token, not one the proxy chooses.
5. The run is started on the snapshot that was sent; asked again, the gateway answers with the same run.

## Gateway API

The HTTP server of gucefWEB takes over exact paths only, so the ids are query parameters. Every request needs
`Authorization: Bearer <token>`; answers are JSON, errors are `{ "error": "...", "code": "..." }` with a fitting HTTP
status.

| Method and path | Purpose |
|---|---|
| `GET /v1/ci` | the CI ids and the limits of the gateway |
| `GET /v1/ci?id=ID` | the options of a CI id, its defaults, `have` and the limits |
| `POST /v1/resolve` | `{ "remotes": [ ... ] }` to the CI id |
| `POST /v1/uploads?ci=ID&size=N&sha256=H` | starts an upload: its id and the chunk size |
| `PUT /v1/uploads?id=U&offset=N` | a chunk |
| `GET /v1/uploads?id=U` | how many bytes arrived |
| `DELETE /v1/uploads?id=U` | gives an upload up |
| `POST /v1/uploads/commit?id=U` | takes the bundle in and sends the snapshot to the CI |
| `POST /v1/runs` | `{ "snapshot": U, "options": { ... } }`: the run id; `{ "ci": ID, "options": { "commit": SHA, "images": [ ... ] } }` builds a published commit, which the gateway sends from its own clone |
| `GET /v1/runs?id=R&wait=N` | the state of a run; with `wait` the answer comes when the run is finished or after N seconds |
| `GET /v1/runs/report?id=R` | the report of a run |
| `GET /v1/runs/jobs?id=R&job=J&product=P&part=&limit=&offset=&full=true` | one job: its report, per log the digest and the log within bounds; `full=true` gives the whole log up to the download limit of the gateway (24 MiB per answer), so that the proxy keeps it locally |
| `POST /v1/runs/cancel?id=R` | cancels a run |
| `POST /v1/cleanup` | `{ "ci": ID, "olderThanDays": N }` |
| `GET /v1/images?id=R` | the image report of a run |
| `POST /v1/deploy` | `{ "ci": ID, "image": I, "target": T, "run": R or "commit": SHA, "confirm": true }`: `{ "result": { ... } }` |
| `GET /v1/release?ci=ID&target=T` | what the Deployments of the target run now |
| `POST /v1/rollback` | `{ "ci": ID, "image": I, "target": T, "confirm": true }`: `{ "result": { ... } }` |

A deployment or rollback answers with its result also when it fails; a token without the scope `deploy` gets 403 with
code `scope`, the other refusals carry the code of the result (`not_listed` 403, `confirm` and `bad_request` 400,
`unknown_ci` and `unknown_run` 404, `busy` 409, `not_green`, `no_images`, `image_failed`, `unpublished` and `pull_policy`
422, `registry`, `kubectl` and `rollout` 502).

## Security

- Machines in proxy mode hold only their own gateway token. The CI token exists only on the gateway or direct machine.
- The gateway accepts the tokens of `--gateway-tokens-file`: one line per machine, the SHA-256 of its token as 64 lower
  case hexadecimal digits, a TAB, the machine id and optionally a TAB and its scopes, comma separated (`build`,
  `deploy`); without the column a machine has the scope `build`, unknown scope names are ignored with a line in the log.
  The lablink hub grants `deploy` to the machines of its option `--ci-deploy-machines`. The static token of
  `--token-file` has the scope `build`. Lines starting with `#` are comments. The file is read again when
  its time or size changes and at least every 30 seconds, so a revoked machine, whose line the hub removes, is refused
  from then on. A file that cannot be read accepts nobody. The given token is hashed and compared with every digest in
  constant time. The file is read with `CORE::ReadOwnerOnlyFile()`; the default location
  `/var/lib/gucef-lablink/ci/gateway-tokens` is in the state directory of the hub, which only root can enter, so the
  gateway service runs as root there, or is pointed at a copy it owns.
- Tokens never appear in arguments, URLs, logs or tool output: they are registered with the redaction of the HTTP logs
  and of the tools, and a token in the query of a request is refused.
- The gateway serves HTTPS with the certificate of its machine (`--listen-tls`); plain HTTP only on 127.0.0.1. Requests
  for host names other than localhost, 127.0.0.1 and those of `--allowed-host` are refused. Proxies verify the
  certificate of the gateway, follow no redirects and do not use proxy servers of the environment.
- The checks of method, path, host, token, request rate and body size happen before a body is read. Uploads are
  bounded in size, number, chunk size and idle time; bundles are verified and only ever fetched into bare repositories;
  only CI ids of the registry are served.
- Scratch branches execute code on the CI runner with the trust of the owner of the checkout that sent them.

## Deployments

The CI registry names per CI id what may be deployed where. Nothing that is not listed is ever touched:

```
"registryHost": "172.16.1.125:32001",
"registryUser": "ci-bot",
"deploy": {
  "images": [
    {
      "name": "asset-store-service", "registryName": "asset-store-service", "registryOwner": "ci",
      "targets": {
        "live": { "namespace": "assetstore", "deployment": "assetstore", "container": "assetstore", "allowScratch": false }
      }
    }
  ]
}
```

`registryHost` is `host:port` (plain HTTP) or carries `http://` or `https://`; without it the host of `url` with port 32001
and the scheme of `url` are used. `registryUser` (default `ci-bot`) is the user of the registry token. `registryName`
defaults to `name`, `registryOwner` to `ci`, `allowScratch` to false. Target names are free. An entry whose deploy block
lacks a namespace, Deployment or container, or names that are not Kubernetes names, is refused with a line in the log;
the other entries are used. Entries without a deploy block are what they were. The published repository is
`owner/repo` of the entry on the same server (a pull mirror); `publishedUrl` gives another git URL for it.

CI builds the images of a run in its images job and publishes `images.json` in the report package (`ci-reports`, version
`<run>-images`): per image its tag (`sha-<12 characters of the commit>`, `scratch-<run>`, also `candidate`), its digest
and its status. The logs of the image builds are `<name>.log.gz` (or `<name>.log`) in the same package version.
`ci_report` and `ci_job_details` describe the images job from this report: per image its status word, tag, digest and
time, for an image that failed the errors found in the end of its log and that end itself (bounded), and in
`ci_job_details` its full log, saved on the machine of the server. An images job without `images.json` is reported as
having published no report. Image references are `<registryHost>/<registryOwner>/<registryName>@sha256:<digest>`.

The flow for a target that takes published commits only: `ci_submit` with `commit` and `images` (the commit must be on a
branch of the published repository; it is sent as `scratch/release-<12 characters>`), `ci_status` until it is finished,
`ci_images`, `ci_release_status`, then `ci_deploy` with `confirm=true` once the user asked for it.

The guards are those of the service, so direct mode and the gateway apply the same ones, and every one of them is tested:

1. the image and the target are listed in the deploy block, else the refusal lists what is listed
2. the caller has the scope `deploy`: on a gateway from the tokens file, in direct mode with `--allow-deploy`
3. `confirm` is the boolean `true`
4. the run is finished and every job succeeded, an images job ran, and the image report has the image with status `ok`
   and a digest
5. for a target without `allowScratch`: the commit of the run is an ancestor of the default branch of the published
   repository, as `git fetch` of that repository into the bare repository of the CI id on this machine and
   `git merge-base --is-ancestor` show; a scratch snapshot or an unknown commit is refused
6. the registry has the tag of the image with the digest of the report (`HEAD /v2/<repository>/manifests/<tag>`, the
   bearer token handshake of OCI registries or Basic credentials, the credentials only to the registry itself)
7. one deployment or rollback per Deployment at a time; another one is refused with `busy`
8. the Deployment does not use `imagePullPolicy: Never`; such a service is switched to the registry once by its owner
   (`IMAGE_REGISTRY` in its `service.env` and one run of its `deploy.sh`)

Then kubectl, always with `--kubeconfig <file> --context <namespace> -n <namespace>` and only verbs the ServiceAccount
of the deployer has (get and patch of the listed Deployments, list and watch of Deployments, ReplicaSets and Pods):
`get deployment -o jsonpath`, one `patch` that sets the image by digest and `kubernetes.io/change-cause` (CI id, run, commit, image, machine,
time), and `rollout status --timeout`. A rollout that fails or times out is not undone: the answer
says what kubectl said and what the Deployment runs (with the strategy RollingUpdate the ready pods of the previous
revision keep serving, with Recreate the service may be down) and leaves the decision to the user. `ci_rollback` runs
`rollout undo` and follows the rollout in the same way. The content of the kubeconfig is never read or shown.

Every deployment, rollback and refusal is a line of `<work-dir>/audit/deploy.jsonl`: time, action, caller machine,
CI id, image, target, namespace, Deployment, run, commit, digest, old digest, result, code and reason. The file is
rotated at 4 MiB to `deploy.jsonl.1` up to `.3`.

| File | Content |
|---|---|
| `/var/lib/gucef-ci/kubeconfig` | the kubeconfig of the ServiceAccount of the deployer, one context per namespace, written by the setup of the deployer |
| `<work-dir>/repos/<ci>.git` | the bare repository of a CI id: snapshots of proxies, the fetched published repository (`refs/remotes/published/*`, `refs/ci/published-head`) |
| `<work-dir>/audit/deploy.jsonl` | the audit of deployments |

## Running the gateway

All work that uses git, the CI or the disk runs as tasks of the thread pool `gucefMcpCi.gateway`; the thread of the
HTTP server only checks requests and answers them when their task is done. A status request with a wait holds no
thread while it waits: it is checked again every poll interval. The number of tasks, waiting requests, waiting statuses,
chunks in hand and uploads is bounded, and requests beyond those bounds are answered with 503 at once. Abandoned
uploads are removed by a timer. Idle, the gateway wakes once a second.

Example on the machine of the hub:

```
gucefMcpCi --mode gateway --listen-tls 0.0.0.0:8792 --tls-cert /path/tls.crt --tls-key /path/tls.key \
           --allowed-host <name of the machine> --ci-config /path/ci.json --forgejo-token-file /path/forgejo.token \
           --work-dir /var/lib/gucef-mcp-ci-gateway
```

## Tests

`tests/gucefMcpCi_TestApp` runs the server as a child process against a fake Forgejo and temporary git repositories:
direct mode, and proxy to gateway to the fake Forgejo over loopback HTTP and HTTPS, the gateway API, uploads, tokens,
limits, a gateway that is down or answers garbage, and chunks that are lost. The deployments run against a fake
registry (token handshake and Basic) and a fake kubectl script that records its arguments, checks the context and the
verbs of every call and answers from state files: every guard, the exact calls of a deployment, failed and timed out
rollouts, rollback, release status, the audit, submits of published commits, the scopes of gateway tokens and one
deployment per target at a time. Nothing goes over the network.
