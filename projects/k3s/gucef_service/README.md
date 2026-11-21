# gucef_service: container image and k3s manifests of a GUCEF service

Generic scripts and templates for running a GUCEF service (a program built on GUCEF that runs unattended, such as a remote service host with its clients on the LAN) as a container on the k3s cluster of `projects/k3s`. This folder is a toolkit, not a module: it has no build files and no code. Each service has a folder of its own (for example `projects/k3s/assetstore/`, the asset store service) with its service file, its bootstrap file and its site settings, and uses the scripts here.

The networking decisions the templates implement are those of `gucefASSETSTORE_RemoteNetworking_Design.md`, section 7 (outside this repository, with the other design documents).

## Contents

| path | what |
|---|---|
| `service.env.example` | every parameter of a service, documented; a service keeps its own copy |
| `docker/gucef_service.dockerfile.template` | two stages: build in the GUCEF development image, run on the minideb of the same Debian release |
| `scripts/lib.sh` | shared functions: the service file reader, the template renderer |
| `scripts/make_context.sh` | the docker build context from the GUCEF checkout and further checkouts (tracked and untracked, not ignored files) |
| `scripts/stage_install.sh` | stages `/opt/<service>` from the build directories and sets the RPATHs (runs in the build stage) |
| `scripts/check_libs.sh` | `ldd` check of every ELF file of the image, of the plugins named by the bootstrap files and of every symbolic link (runs in the image build) |
| `scripts/build_image.sh` | renders the Dockerfile, builds the image with docker or BuildKit, imports it into k3s or pushes it to a registry; `--dry-run` checks without a builder |
| `scripts/run_local.sh` | runs the image with docker with the restrictions of the pod |
| `scripts/resolve_storage_class.sh` | resolves `STORAGE_CLASS` (auto, a friendly name or a class name) against the StorageClasses of the target cluster ("Storage") |
| `scripts/render_manifests.sh` | renders the k3s templates from a service file into a directory for `kubectl apply -k` |
| `scripts/deploy.sh` | resolve, render, namespace, Secret check, apply, rollout wait, idempotent, with `KUBECTL` as a command prefix ("Deploy") |
| `k3s/` | templates: namespace, claim, ConfigMap, Deployment, Service, kustomization, an example Secret, and the optional components lan-discovery, secret-file, host-share and credentials-key |
| `tests/run_tests.sh` | tests of the scripts and templates on constructed trees, without docker or a cluster |

## How a service uses it

A service folder holds:

* `service.env`: a copy of `service.env.example` with the service's values (name, namespace, program, build targets, ports, sizes, resources, drain and grace times, site settings file). It holds no secret.
* the bootstrap file of the image (`BOOTSTRAP_INI`): the plugins of the service, with `"Path"="$MODULEDIR$"`, which the staging points at `plugins/`. Plugins the service cannot run without are marked `"LoadFailAllowed"="false"`; `check_libs.sh` then fails the image build when one is missing. A program that loads its plugins from its settings instead still has this file: it is the list the image build stages and checks.
* the site settings (`SITE_CONFIG_SOURCE`), which become the ConfigMap and are mounted read-only at `/opt/<SERVICE_NAME>/config/site/`.
* optionally its own Dockerfile (`DOCKERFILE`), derived from the template, when the generic one does not fit (an extra stage, a sidecar image).

The program follows the conventions the templates assume: it runs in the foreground with `--console`, logs to stderr (`GUCEF_CONSOLE_LOGGER=stderr`), keeps its state below `/data`, serves `/health/started`, `/health/ready` and `/health/live` on the health port at the pod address, drains on `SIGTERM` within its drain time, and offers its admin endpoint on 127.0.0.1 only.

## Image layout

`/opt/<SERVICE_NAME>/{bin,lib,plugins,data,config}`, user `10001:10001`, read-only root file system; writable are `/data` (the claim) and `/tmp` (an emptyDir). RPATHs are relative to each file: programs `$ORIGIN/../lib:$ORIGIN/../plugins`, libraries `$ORIGIN:$ORIGIN/../plugins` (a library may link a plugin), plugins `$ORIGIN/../lib:$ORIGIN`. A plugin is a library whose name matches `lib<kind>plugin<NAME>.so` or that a bootstrap file names; it is kept in `plugins/` only, so the dynamic loader and the plugin loader load one object. `PLUGIN_LINKS` is a legacy option, set by no service of this repository: GUCEF programs load their plugins through their bootstrap file, whose `Path` points at `plugins/`. Only for a program that loads its plugins as `$MODULEDIR$/<name>` (its own folder `bin/`, then `lib/` next to it) instead of through a bootstrap file with a `Path`, `PLUGIN_LINKS="lib"` makes `lib/` hold a symbolic link to each plugin; the loader opens the one file of `plugins/` through it (glibc knows a loaded object by its file, so nothing is loaded twice), and `check_libs.sh` fails the image build for a dangling link or a link that leaves the tree.

`EXTRA_PROGRAMS` (`name=directory` words) ships external tools the service starts as child processes. The tool is a module the service program lists as an executable `RuntimeDependency` in its `ModuleInfo.xml`, so the generated target builds it into the same `bin/`; `stage_install.sh --extra-program` requires it there, and copies the `COPYING*`, `LICENSE*`, `NOTICE*` and `README*` files of the directory (relative to the targets checkout) to `/opt/<SERVICE_NAME>/licences/<last part of the directory>/`, an error when there is none (a copyleft tool must ship with its licence). `check_libs.sh` checks the tool like every other ELF file, so its libraries must resolve inside the image or the base set.

## Build and import

Requirements on the build machine: docker, git, rsync, the development image `vanvelzen/github-gucef-dev-base:latest` in docker (built once with `projects/k3s/gucef_dev/create_github_gucef_dev_base_image.sh`), and k3s for the import.

```
projects/k3s/gucef_service/scripts/build_image.sh --env <service folder>/service.env [--tag TAG] [--no-import] [--save FILE] [--pull-base] [--dry-run]
```

* The script refuses to start, before docker is called, when a target of `IMAGE_TARGETS` has no generated build files below `projects/CMake/targets` of `TARGETS_CHECKOUT`, or when the bootstrap file is missing. Build files are generated by ProjectGenerator from the program's `ModuleInfo.xml` (`CLAUDE.md` of GUCEF).
* The Debian release and the libssl runtime package are read from the development image, so the runtime stage matches the build. The runtime base image `bitnami/minideb:<release>` must be in docker; `--pull-base` downloads it, the only download the script makes (the image build installs packages with `install_packages`).
* The build context is a copy of the working trees (tracked files and untracked files that git does not ignore), so uncommitted work is in the image and the default tag then ends in `-dirty`. The default tag is the short hashes of the checkouts joined with `-`.
* The image is `<IMAGE_NAME>:<tag>`; containerd stores it as `docker.io/<IMAGE_NAME>:<tag>`. The manifests use `imagePullPolicy: Never`: the image is imported with `k3s ctr images import`, never pushed to a public registry. For a k3s host that is not the build machine, `--save FILE --no-import` keeps the archive to copy and import there.
* The build stage builds every target in release with CMake and Ninja and runs `stage_install.sh`; the runtime stage runs `check_libs.sh`, which fails the build when a file does not resolve its libraries inside the image or the base image.

### Builder and registry

```
projects/k3s/gucef_service/scripts/build_image.sh --env <service folder>/service.env [--builder auto|docker|buildkit]
    [--registry HOST[:PORT][/PATH] [--image-name NAME] [--push [--also-tag TAG]... [--digest-file FILE]]]
    [--dev-image IMAGE] [--runtime-base REPOSITORY] [--tag TAG] [--no-import] [--save FILE] [--dry-run]
```

* `--builder` selects the engine. `auto` (the default) takes docker when it is installed, else BuildKit when `buildctl` is installed; with docker everything above stays as it is. `buildkit` builds with `buildctl` against the BuildKit daemon of `BUILDKIT_HOST` (default of `buildctl`: `unix:///run/buildkit/buildkitd.sock`) and needs no container daemon. Both build the same Dockerfile template, which is plain OCI Dockerfile syntax. BuildKit runs no container outside a build, so the Debian release and the libssl package of the development image are read by a small build of their own (the release probe) instead of `docker run`.
* `--registry HOST[:PORT][/PATH]` names the image `<registry>/<name>:<tag>`; the name is `--image-name`, else `IMAGE_REGISTRY_NAME` of the service file, else the last path component of `IMAGE_NAME` (`vanvelzen/asset-store-service` gives `<registry>/asset-store-service`). `--push` pushes it after the build: docker pushes every tag with `docker push`; BuildKit pushes all tags in one build output (`type=image,name=<every tag>,push=true,registry.insecure=true`). A pushed image is never imported into k3s: the nodes pull it. `--also-tag TAG` (repeatable) pushes the same image under further tags, for example `candidate`. `--digest-file FILE` writes the digest of the pushed image manifest to FILE as its only line, `sha256:<64 hex digits>`; that digest is the value for `IMAGE_TAG` that pins the image exactly.
* BuildKit never imports into k3s: without `--push` it needs `--no-import`, and `--save FILE` then writes a docker archive for `k3s ctr images import`. `--save` and `--push` exclude each other with BuildKit.
* `--dev-image` may be a registry reference with BuildKit (for example `172.16.1.125:32001/ci/gucef-dev-base:latest`); docker needs it in its local image store, as before. `--runtime-base REPOSITORY` replaces `bitnami/minideb` as the repository of the runtime base image; the tag stays the Debian release of the development image (for example `172.16.1.125:32001/ci/minideb`, filled by `projects/k3s/forgejo/registry/push-base-image.sh`).
* The registry login comes from the environment: `REGISTRY_USER` and `REGISTRY_TOKEN_FILE`, a file holding the token (mode 0600). The script reads the token itself and writes a docker configuration file (`config.json`, mode 0600) into a private temporary directory, which `DOCKER_CONFIG` names for `docker push` and for `buildctl` (BuildKit also pulls registry base images with it); the directory is removed when the script ends. The token is never on a command line and never printed. Without `REGISTRY_USER` the builder uses the credentials it already has.
* The registry of the cluster (the Forgejo container registry) speaks plain HTTP. BuildKit pushes with `registry.insecure=true`, and its daemon lists the registry as `http` in `buildkitd.toml` (`projects/k3s/forgejo/runner/setup.sh --oci`); a docker daemon needs the registry in `insecure-registries` of its `daemon.json`.
* Usage errors (exit code 2): `--push` without `--registry`; `--also-tag` or `--digest-file` without `--push`; an unknown builder; BuildKit without `--push` and without `--no-import`; `REGISTRY_USER` without `REGISTRY_TOKEN_FILE` or the reverse; a login without `--registry`.
* Images of the CI are named `<registry host>/ci/<name>` with the tags `sha-<12 character commit>`, `scratch-<run id>` and `candidate`.

## Local run

```
projects/k3s/gucef_service/scripts/run_local.sh --env <service folder>/service.env <data directory> [--foreground] [--bind 0.0.0.0] [--dry-run]
```

The container runs as the pod does: user 10001 (or the owner of the data directory when it is not 10001), read-only root, a tmpfs `/tmp`, no capabilities, no privilege escalation, the stop timeout of the grace period. The session and pairing ports are published on `--bind` (127.0.0.1 by default), the health port on 127.0.0.1 only, the admin port not at all (`docker exec` reaches it, as `kubectl exec` does in the cluster).

## Manifests

```
projects/k3s/gucef_service/scripts/render_manifests.sh [--storage-classes-file FILE | --query-cluster | --storage-class NAME] [--local-dir DIR] <service folder>/service.env <output directory>
kubectl apply -k <output directory>
```

A placeholder in a template takes the value of the key of the same name of the service file; an unset or empty value refuses the rendering. Before the first apply, label the node that holds the imported image and the local-path claim: `kubectl label node <node> <NODE_SELECTOR_KEY>=<NODE_SELECTOR_VALUE>`. `deploy.sh` ("Deploy") does the whole sequence.

| object | content |
|---|---|
| Namespace | Pod Security standard `privileged` enforced with `DISCOVERY_MODE=lan` (the default), `restricted` with `DISCOVERY_MODE=off`; `restricted` as the warn and audit level in both |
| PersistentVolumeClaim `<name>-data` | `/data`, ReadWriteOnce, the StorageClass resolved from `STORAGE_CLASS` (see "Storage") |
| ConfigMap `<name>-config` | the site settings, mounted read-only |
| Deployment | one replica, `Recreate`, the node selector, no service account token, no service links, non-root 10001 with `fsGroup`, seccomp `RuntimeDefault`, read-only root, all capabilities dropped, startup, readiness and liveness probes on the health port, the grace period, `/tmp` as an emptyDir with a size limit, the image with `imagePullPolicy: Never` (with `IMAGE_REGISTRY`: the registry image with `IfNotPresent`, see "Images from a registry"), no CPU limit |
| Service | `ClusterIP` in lan mode (the default, component `lan-discovery`); `LoadBalancer` with `externalTrafficPolicy: Local` in off mode; the session and pairing ports only |
| `components/lan-discovery` | with `DISCOVERY_MODE=lan`, the default (see "LAN discovery"): host network on one node, the discovery port, a ClusterIP Service; enabled in the rendered `kustomization.yaml` by `render_manifests.sh` |
| `components/secret-file` | with `SECRET_FILE_SECRET_NAME` set: one key of a Secret as the read-only file `<SECRET_FILE_MOUNT_DIR>/<SECRET_FILE_KEY>` (mode `SECRET_FILE_MODE`, default 0400) for a value the program reads from a file its settings name, such as a database password (see "Secrets"); rendered and enabled by `render_manifests.sh` |
| `components/host-share` | with `HOST_SHARE_HOST_PATH` set: a directory of a node-mounted network share at `HOST_SHARE_MOUNT_PATH` (hostPath, `HostToContainer`), with an init container that refuses a path not on a network file system (see "A network share of the node"); rendered and enabled by `render_manifests.sh` |
| `components/credentials-key` | with `CREDENTIALS_SECRET_NAME` set: mounts a Secret with the credential store key read-only (mode 0440, group 10001) and sets the program's key file and trust variables; rendered as a commented line of the `kustomization.yaml`, to enable once the Secret exists. A program that keeps its key on `/data` leaves the name empty and the component is not rendered |

### Images from a registry

Without `IMAGE_REGISTRY` in the service file the manifests use the imported image `docker.io/<IMAGE_NAME>:<IMAGE_TAG>` with `imagePullPolicy: Never`, as before. With `IMAGE_REGISTRY` (`HOST[:PORT][/PATH]`) they use `<IMAGE_REGISTRY>/<name>:<IMAGE_TAG>`, the name being `IMAGE_REGISTRY_NAME` or else the last path component of `IMAGE_NAME` (`gs_registry_image_name`, the same rule as `build_image.sh --registry`), with `imagePullPolicy: IfNotPresent`, in the service container and in the init container of `components/host-share`. An `IMAGE_TAG` of the form `sha256:<64 hex digits>` is a digest and is joined with `@` (one rule, `gs_image_ref` in `scripts/lib.sh`). Every node that may run the pod needs the registry in `/etc/rancher/k3s/registries.yaml` (`projects/k3s/forgejo/registry/setup-node.sh`); no pull Secret is used. The node selector stays where the claim of the data volume needs it.

## Storage

`STORAGE_CLASS` of the service file names what the claim of `/data` wants; `scripts/resolve_storage_class.sh` turns it into a StorageClass the target cluster has:

| `STORAGE_CLASS` | candidates, in order of preference |
|---|---|
| `auto` (**the default**, also for a service file without the key) | the first of `raid`, `mirror`, `jbod`, `local-path` (in this order) of which the cluster has a candidate |
| `raid` | `longhorn-raid`, `raid` |
| `mirror` | `longhorn-mirror`, `mirror` |
| `jbod` | `longhorn-jbod`, `jbod` |
| `local-path` | `local-path` |
| any other value | a StorageClass name, used as it is (with a class list it must be in it) |

* `raid` and `jbod` without the prefix are the local-path based classes of `projects/k3s/host-node/storage-classes.yaml` (Retain); the `longhorn-*` classes those of `projects/k3s/longhorn` (Longhorn volumes on the disks of the tag; Longhorn enforces the size, local-path does not). When a cluster has both forms, the Longhorn-managed class wins.
* The list of classes comes from a file (`--storage-classes-file FILE`, `-` for standard input, in the format of `kubectl get storageclass -o name` or as plain names) or from the cluster: `$KUBECTL get storageclass -o name`. `KUBECTL` (default `kubectl`) is a command prefix such as `ssh k3smaster kubectl`: split on white space into words, never evaluated by a shell (`;`, `$( )` and quotes in it stay literal words), and executed, so it is set in the environment of the person who runs the script and never read from a service file.
* Nothing matching refuses with the classes found and the candidates tried; there is no fallback to the default class or to anything else.
* `render_manifests.sh` resolves with `--storage-classes-file` or `--query-cluster`, or takes an already resolved `--storage-class NAME`. Without any of them a class name is used as it is (with a note that it was not checked) and `auto` or a friendly name refuses the rendering. The claim notes the request and the result in its comment.
* The class of a claim cannot change once it exists: `deploy.sh` refuses a resolution that differs from the class of the existing claim and says to set `STORAGE_CLASS` to the existing class (or to move the data first). So a cluster that gains a preferred class later does not move a running service by surprise.

## Secrets

No secret value is in a service file, a site settings file, a ConfigMap, an environment variable or a file of a repository. A value the program reads from a file reaches the pod as a Secret mounted by `components/secret-file`; the Secret is created in the cluster by a separate step before the rollout (the namespace must exist: `deploy.sh` makes it and stops while the Secret is missing):

```
# from a file that holds only the value (mode 0600):
kubectl -n <NAMESPACE> create secret generic <SECRET_FILE_SECRET_NAME> --from-file=<SECRET_FILE_KEY>=<file>
# from a variable of an env file, through a pipe (the value is in no argument and never printed):
( set -a; . <env file>; printf '%s' "$<VARIABLE>" ) | kubectl -n <NAMESPACE> create secret generic <SECRET_FILE_SECRET_NAME> --from-file=<SECRET_FILE_KEY>=/dev/stdin
```

With `ssh k3smaster kubectl` in place of `kubectl` the pipe works the same (ssh passes standard input on). To replace the value: delete the Secret, create it again, `kubectl -n <NAMESPACE> rollout restart deployment/<SERVICE_NAME>`. `components/secret-file/secret.example.yaml` and `k3s/secret.example.yaml` only show the form and are never applied; Sealed Secrets (`projects/k3s/sealed-secrets`) are the alternative for a value kept in git. The kubelet gives the files of a Secret volume the group of the pod (`fsGroup` 10001) and adds group read, so mode 0400 is `root:10001 0440` on the node and the non-root user reads it through its group.

The credentials key Secret of `components/credentials-key` is made the same way from its key file (`kubectl -n <NAMESPACE> create secret generic <CREDENTIALS_SECRET_NAME> --from-file=credentials.key=<key file>`, or sealed); `k3s/secret.example.yaml` shows its form.

## Local install configuration

The repository holds what every installation of a service shares; the values of one site (the node path of a share, a
host, a different memory limit, settings that name local storage) live in a local directory that is **never inside a
git repository**. `render_manifests.sh` and `deploy.sh` take it with `--local-dir DIR`; without the option, a
`service.env.local` next to the service file makes that directory the local directory.

| file in the local directory | effect |
|---|---|
| `service.env.local` | `KEY=value` lines (the format of the service file) that override keys of `service.env`; a relative `SITE_CONFIG_SOURCE`, `BOOTSTRAP_INI` or `DOCKERFILE` in it is relative to the local directory. No secret here either |
| `site.json` | replaces the site settings file (`SITE_CONFIG_SOURCE`) as a whole: the ConfigMap holds it instead (no merge; copy the repository's file and change it) |
| `patches/*.yaml` | kustomize patches (strategic merge, with `kind` and `metadata.name` of their target), applied after the base and the components |
| `resources/*.yaml` | further objects, added to the resources |

The local files are rendered like the templates (`@KEY@` placeholders, for example `@NAMESPACE@`) into `local/` of the
output directory, so the rendered directory stays self-contained. A local directory inside a git work tree is refused
with the reason (it names the site's hosts, paths and shares, which do not belong in a repository); `--allow-local-in-git`
uses it anyway, with a note. A typical layout:

```
<outside any repository>/install-local/<service>/
    service.env.local    HOST_SHARE_HOST_PATH='/mnt/<share>/<folder>', other overrides
    site.json            the site settings of this installation
    patches/  resources/ optional
KUBECTL="ssh <host> kubectl" projects/k3s/gucef_service/scripts/deploy.sh --env projects/k3s/<service>/service.env --local-dir <outside any repository>/install-local/<service>
```

## A network share of the node

`components/host-share` puts a directory of a network share that the node mounts (CIFS/SMB or NFS, for example a
systemd automount from `/etc/fstab`) into the pod at `HOST_SHARE_MOUNT_PATH` (default `/share`). It is rendered and
enabled when `HOST_SHARE_HOST_PATH` is set, normally in `service.env.local`.

* `HOST_SHARE_HOST_PATH`: the directory on the node, at or below the mount point of the share. Any characters except
  control characters: spaces and names such as `-== Assets ==-` are fine. Write the value in single quotes
  (`HOST_SHARE_HOST_PATH='/mnt/<share>/-== <folder> ==-/<service>'`); the reader removes one pair of surrounding
  quotes and expands nothing, and the rendering writes it as one YAML double-quoted scalar. `HOST_SHARE_SUBPATH`
  (optional) is joined to it; the hostPath is the joined path (not a volume `subPath`, which the kubelet would create
  when missing and which receives no mount propagation).
* The volume is a `hostPath` of type `Directory`: a missing node path fails the pod; nothing is created on the node.
  Both containers mount it with `mountPropagation: HostToContainer`, so a mount the node makes later (an automount, a
  remount after the server was away) reaches the pod.
* The init container `host-share-check` runs the service image itself with `/bin/bash` (the runtime image has bash
  and the coreutils, `check_libs.sh` runs in it) as user 10001 with no capabilities, no privilege escalation and a
  read-only root. It reads `/proc/self/mountinfo`, finds the mount that holds the path (a subdirectory of the share's
  mount is fine) and stops the pod when its type is none of `HOST_SHARE_FSTYPES` (default `cifs smb3 nfs nfs4`) or
  when the user cannot write there, with a message that says so. So when the share is not mounted, the files of the
  service never land on the node's own disk; the kubelet retries the init container with back-off until the share is
  there.
* The node mount must let the pod's user write: for CIFS mount options such as
  `uid=10001,gid=10001,file_mode=0664,dir_mode=0775` (they only change how the node presents the files) and `nofail`
  with `x-systemd.automount,_netdev` so the node boots without the server. The credentials file of the mount stays on
  the node (root, mode 600), never in the cluster or a repository.
* A `hostPath` volume needs the Pod Security level `privileged` (baseline and restricted refuse it): the namespace is
  rendered with it while the component is on, with `restricted` kept as the warn and audit level (in lan mode it is
  `privileged` already). The pod runs on one node anyway (`DISCOVERY_NODE_NAME` in lan mode, else the node label):
  that node needs the mount.

## Deploy

```
KUBECTL="ssh k3smaster kubectl" projects/k3s/gucef_service/scripts/deploy.sh --env <service folder>/service.env [--local-dir DIR] [--out DIR] [--timeout SECONDS] [--dry-run]
```

In order, each step idempotent (a second run changes nothing that is in place):

1. resolve `STORAGE_CLASS` against the cluster's classes; refuse when the existing claim has another class;
2. render (`render_manifests.sh --storage-class`) and build with kustomize on this machine (`KUSTOMIZE`, default `kubectl kustomize`, offline); the built file is what is applied, through `apply -f -` on standard input, so a remote `kubectl` needs no access to local files; refuse a built Secret;
3. check that a node carries `NODE_SELECTOR_KEY=NODE_SELECTOR_VALUE` (in lan mode: `DISCOVERY_NODE_NAME`), else stop with the `kubectl label node` command;
4. create or update the namespace;
5. with `SECRET_FILE_SECRET_NAME`: check that the Secret has its key (kubectl answers only "present", the value never leaves kubectl), else stop with the command to create it;
6. warn when the image is not in the image list of the node (it is imported by the owner; the node lists a limited number of images, so this is a warning); an image of `IMAGE_REGISTRY` is pulled by the node and not checked;
7. apply and wait for the rollout (`--timeout`, default `STARTUP_FAILURE_THRESHOLD` x 10 + 120 s).

It never builds or imports the image and never creates a Secret. `--dry-run` runs the resolution, the rendering and the read-only checks and applies nothing. The rendered and built manifests are kept below `--out` or `$DEPLOY_WORK_DIR` (default `~/.cache/gucef-deploy/<service>/`). Every argument the script adds to `KUBECTL` is free of white space and shell characters, so a remote shell passes it on unchanged.

## Networking

| decision | reason |
|---|---|
| `LoadBalancer` through the k3s built-in servicelb | the ports are answered on the node addresses without a cloud load balancer, at the service's own port numbers (a NodePort would need the 30000 to 32767 range) |
| `externalTrafficPolicy: Local` | keeps the client's source address (no SNAT): the per-address handshake limits and bans of DRN and the audit depend on it |
| host network only for LAN discovery | `DISCOVERY_MODE=lan` (the default) takes the host network of one node so the service answers LAN discovery, see "LAN discovery"; `DISCOVERY_MODE=off` keeps the restricted standard, which refuses host network and host ports |
| ports as parameters | `SESSION_PORT` and `PAIRING_PORT` in the Service; `HEALTH_PORT` in the pod only; `ADMIN_PORT` on 127.0.0.1 inside the container only |
| health port in no Service | the kubelet probes the pod address; `/info` and the probes are never exposed on the LAN |
| admin endpoint on 127.0.0.1 | reached with `kubectl exec ... <program> --admin <command>`; its token file is in the state directory, so access to the pod is the credential |
| readiness before traffic | the service opens its listeners when it is ready; the readiness probe fails as soon as the drain starts, so no new connection is routed to a draining pod |
| one replica, `Recreate` | the old pod drains (`SIGTERM`, the drain time) and exits before the new one starts: never two writers, never a client routed to two services; between the two pods the ports refuse connections and clients reconnect with jittered backoff |
| `terminationGracePeriodSeconds` above the drain time | the drain ends before the kubelet kills the container; `render_manifests.sh` refuses a grace period that is not above `DRAIN_SECONDS` |

Firewall of the nodes (ufw or nftables): allow the session port and the pairing port from the LAN prefix only. The pairing port is never forwarded to the internet; the service listens on it only while a pairing window is open. The health and admin ports need no rule.

To verify on the cluster: that servicelb with `externalTrafficPolicy: Local` publishes only the nodes that run a ready pod of the service, so that a connection arriving at another node is not dropped; with several nodes, a client's endpoint list should name the node of the pod (or a DNS name for it) first.

## LAN discovery

The configured endpoint is the way to reach the service that always works: a plain value in the client's settings,
a DNS name (for example `worker.lan`) or an address, and the session port (`tcp://worker.lan:47400`). Discovery is a
convenience next to it: a client on the LAN asks "which services of this type are there" and gets candidates to offer.
A candidate is never trusted for being found: the client still pairs with the one-time code and pins the service key,
so a spoofed answer cannot become a trusted service (`gucefASSETSTORE_RemoteNetworking_Design.md`, N-D6).

A pod network does not receive the broadcasts and multicasts of the LAN, and its address is not on the LAN. So the
service file chooses a mode:

| `DISCOVERY_MODE` | what is rendered |
|---|---|
| `lan` (**the default**, also for a service file without the key; requires `DISCOVERY_NODE_NAME`, the rendering refuses without it) | the component `components/lan-discovery`: `hostNetwork: true` with `dnsPolicy: ClusterFirstWithHostNet`, the pod pinned with `kubernetes.io/hostname: <DISCOVERY_NODE_NAME>` (next to the node label of the image), the UDP port `DISCOVERY_PORT` (default 47800) declared, the Service changed to `ClusterIP` (no servicelb: it would claim the same node ports on every node, and the pod holds them on its node), `Recreate` kept; the namespace enforces `privileged` with `restricted` as warn and audit level |
| `off` | the restricted standard, the LoadBalancer Service through servicelb, no host network: clients reach the service at its configured endpoint only |

`lan` is the default by the owner's decision: clients and servers have LAN discovery on unless configured off. A remote
service host of gucefDRNUTILS announces itself by default (setting `lanDiscovery`, default true; its
`lanDiscoveryAdvertiser` section names the service type, for example `_gucef-assetstore._tcp`), and a client without a
configured endpoint looks for its paired service by default. The default mode therefore needs `DISCOVERY_NODE_NAME`:
the rendering stops with a message that names it, there is no silent fallback to `off`. Set `DISCOVERY_MODE=off` (and,
if wanted, `"lanDiscovery": false` in the site settings) to keep the restricted standard; the configured endpoint works
in both modes.

Why `privileged` and not `baseline`: the Pod Security standard `baseline` forbids the host namespaces
(`spec.hostNetwork` must be unset or false), as `restricted` does. A pod on the host network is only admitted at
`privileged`. The namespace holds this one service only; `restricted` stays the warn and audit level, so `kubectl apply`
reports the host network (expected) and anything else that is not restricted (not expected). The pod keeps every other
restriction: non-root 10001, all capabilities dropped, no privilege escalation, read-only root, seccomp `RuntimeDefault`.

What the host network weakens, honestly:

* The pod shares the network namespace of the node: its listeners are on every address of the node, and it can see and
  reach everything the node can on the network (including services bound to the node's loopback), without the pod
  network's isolation. NetworkPolicies do not apply to host-network pods.
* The health port (`HEALTH_PORT`, `/health/*` and `/info`) and the loopback admin endpoint (`ADMIN_PORT`) are now on the
  node: the health port on the node's addresses, the admin endpoint on the node's 127.0.0.1 (still owner token protected,
  but reachable by any process of the node). Block the health port from the LAN on the node firewall.
* The ports must be free on that node, and the pod can only run there: if the node is down, the service is down (it
  is single-replica anyway; without lan mode it could run on any node that has the image and the claim).
* Source addresses are kept natively (no servicelb), as `externalTrafficPolicy: Local` did.

Why it is LAN only: broadcasts stay on the subnet and the multicast TTL is 1, so discovery never reaches beyond the
local network; answers go by unicast to the asker only, are rate limited per source and are never larger than the query
(see the design). The internet path is the configured endpoint, never discovery.

The alternative that keeps `restricted`: set `DISCOVERY_MODE=off` and give clients the endpoint (the DNS name of the
node or the servicelb address). An mDNS announcement made by the node itself (Avahi) is a further option that needs no
change to the pod (follow-up F7 of the design).

Firewall in lan mode: allow UDP `DISCOVERY_PORT` from the LAN prefix (queries arrive there; answers go from it to the
asker's port), the session port and, during pairing, the pairing port from the LAN prefix; deny the health port from
the LAN.

To verify on the cluster in lan mode, the default (not done by the tests, which run without a cluster):

1. `kubectl get nodes` shows the node name used as `DISCOVERY_NODE_NAME`, and it carries the `NODE_SELECTOR_KEY` label.
2. `kubectl apply -k <rendered>` admits the pod (a warning about `hostNetwork` from the `restricted` warn level is
   expected) and `kubectl -n <namespace> get pod -o wide` shows the node's address as the pod IP.
3. The probes pass on the node address; `ss -lntup` on the node shows the session port and UDP `DISCOVERY_PORT`
   owned by the service, and no svclb pod for the service exists.
4. From a LAN machine: `tcp://<node>:<SESSION_PORT>` connects (configured endpoint), and a finder of the service type
   finds the service with that endpoint (for example the client of R2, or a test finder with `discoveryClients`
   `broadcast,multicast`).
5. From a machine on another subnet: discovery finds nothing, the configured endpoint still works.
6. A rollout (`kubectl rollout restart`) frees and binds the ports without a "port in use" crash loop (`Recreate`).

## Verification

```
projects/k3s/gucef_service/tests/run_tests.sh
```

The tests use constructed trees: no docker, no cluster, nothing downloaded. They check the syntax of every script and that no file holds a tab; the service file reader; the rendering of every template from the example service file and its refusals; both discovery modes, their refusals and, when kubectl is installed, their offline `kubectl kustomize` build; the staging of a program, a library and two plugins built with the local compiler (with a stand-in for patchelf), `check_libs.sh` passing on the staged tree and failing for a missing required plugin, a missing program and a library that resolves outside the tree, and the staged program running from the staged tree alone; the build context of a constructed checkout; the rendered Dockerfile and the refusals of `build_image.sh` before docker; and the docker command of `run_local.sh`; the storage class resolution (every friendly name, the auto order on several class lists, both raid forms, no match, an explicit name, the list formats, `KUBECTL` as a prefix through a stand-in kubectl and a stand-in ssh that re-parses the arguments in a shell, shell characters in `KUBECTL` never run) and its use by `render_manifests.sh`; the secret-file and the optional credentials-key components; the plugin links of the staging and `check_libs.sh` on dangling links; `deploy.sh` against the stand-in kubectl (each refusal, the order, the applied manifests, the rollout wait, a second run, the dry run, the run through ssh); the local install configuration (overrides, `site.json`, patches, resources, the refusal inside a git work tree and `--allow-local-in-git`) and `components/host-share` (the quoting of a node path with spaces, quotes and `-== ==-`, the init container and the propagation in the kustomize build, the refusals, and the init container's check itself against constructed mount tables); `build_image.sh` with stand-in docker and `buildctl` that record their arguments (the refusals of the builder, registry and push options, the choice of `--builder auto`, the release probe and the push of two tags with BuildKit, the digest file, the registry login as a docker configuration of mode 0600 with the token on no command line and in no output, the push with docker, the local flow with its k3s import), the image reference rule, `render_manifests.sh` with `IMAGE_REGISTRY` and `deploy.sh` with a registry image; and the service folder `projects/k3s/assetstore` (rendered, built and checked against its site settings, bootstrap file and the program's `ModuleInfo.xml`). Scratch goes to `$TMPDIR` (default `/tmp`); `KEEP_SCRATCH=1` keeps it.
