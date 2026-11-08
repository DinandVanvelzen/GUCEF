# Install medium

Packages of the home lab services and an install medium made from them, e.g. a USB stick: a machine of the lab
network gets the services from it without a checkout and without building.

## Make a medium

On a machine with a checkout and the build tools (the cluster, to put the lab secret on it):

```bash
cluster-node-installation-media/make-media.sh /media/$USER/<stick>              # the machines ask for the lab secret, or wait for approval
cluster-node-installation-media/make-media.sh /media/$USER/<stick> --with-secret # the machines link without approval
```

It builds `gucefMcpLocalLlm` and `gucefLabLink` (Release, relocatable) for amd64 and arm64 in
`~/.cache/gucef-cluster-node-installation-media`, runs the test applications of both, makes the packages with CPack
([packaging/](packaging/)) and writes `<target>/gucef-lab`. `--arch amd64` or `--arch arm64` builds one of them.

arm64 is for 64-bit ARM boards such as the Raspberry Pi 3, 4 and 5, Orange Pi 5 and Banana Pi, with a 64-bit OS
(Raspberry Pi OS 64-bit, Ubuntu, Debian, Armbian); 32-bit systems (armhf) are not supported. On an amd64 machine it is
cross compiled in a container: docker is needed, and the container ([cross/arm64.Dockerfile](cross/arm64.Dockerfile),
toolchain [cross/aarch64-linux-gnu.cmake](cross/aarch64-linux-gnu.cmake)) is made on the first run. Its tests run
when the machine runs arm64 programs through QEMU (Ubuntu: `sudo apt-get install qemu-user-static`); otherwise they
are skipped and the output says so.

| File | Holds |
|---|---|
| `install.sh` | installs the packages and sets up the service `gucef-lablink` ([medium-install.sh](medium-install.sh)) |
| `linux/` | the packages, DEB and, when `rpmbuild` is there (`sudo apt-get install rpm`), RPM: `<package>-<version>-<amd64\|arm64\|all>.deb` and `.rpm` |
| `docs/` | the README of each service and of its program: `gucef-mcp/README.md`, `gucef-mcp/gucefMcpLocalLlm.md`, `gucef-lablink/README.md`, `gucef-lablink/gucefLabLink.md` |
| `lablink.env` | the settings of `lablink/config.env` of the checkout that apply to every machine, such as `LAB_ALLOW` |
| `lab-secret` | the lab secret, with `--with-secret` only: whoever has the medium can link machines |
| `README.txt` | how to use it ([medium-README.txt](medium-README.txt)) |
| `SHA256SUMS` | checksums, checked by `install.sh` |

Without a target it goes to `common/bin/cluster-node-installation-media`. The packages need glibc 2.34 or newer (Ubuntu 22.04, Debian 12, Raspberry Pi OS
12, Fedora 35 and later); the output gives the version per architecture.

## Use it

On the machine, as the user whose Claude Code is to be set up:

```bash
sudo bash /media/$USER/<stick>/gucef-lab/install.sh
```

The machine then does what [../lablink/README.md](../lablink/README.md) describes: it finds the cluster, joins it as
an ephemeral node and registers the MCP services of the nodes with Claude Code. Running it again with a newer medium
upgrades.

## Packages

| Package | Files | Service |
|---|---|---|
| `gucef-k3s-scripts` | `/usr/share/gucef-k3s`: `agent-node`, `nvidia`, `host-node/set_inotify_limits.sh`, `llm/mcp`, `lablink` | |
| `gucef-mcp` | `/usr/lib/gucef-mcp`: the MCP server, its libraries, `local-agent.sh`, `README.md`, `gucefMcpLocalLlm.md` | `gucef-mcp` |
| `gucef-lablink` | `/usr/lib/gucef-lablink`: `gucefLabLink`, its libraries, `README.md`, `gucefLabLink.md` | `gucef-lablink` |

The installed service has the same files in `/opt/<service>/current`, the READMEs included; the scripts and their
READMEs are in `/usr/share/gucef-k3s`.

The scripts are those of the checkout. Without a checkout around them they install the files of the packages instead
of building (`llm/mcp/install.sh`, `lablink/install.sh`; `--prebuilt DIR` does the same from a checkout), and their
settings are `/etc/gucef-mcp/settings.env` and `/etc/gucef-lablink/settings.env`. The packages hold files only:
the services are set up by those scripts, as `install.sh` of the medium and `agent-node/join.sh` run them. A package
upgrade brings a service that is set up to the new files (`--upgrade`); removing a package removes its service
(`uninstall.sh`); the state in `/var/lib` stays.

Versions are `1.<commits of the checkout>.<UTC time of the build>`.

Windows and macOS machines get their part with phases 2 and 3 of the lab link.
