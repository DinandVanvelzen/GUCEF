GUCEF home lab install medium
=============================

Installs the services of the home lab on a Linux machine of the lab network:
a PC (amd64) or a 64-bit ARM board such as a Raspberry Pi 3/4/5, Orange Pi 5
or Banana Pi (arm64, with a 64-bit OS).
The machine then finds the k3s cluster by itself, joins it as an ephemeral node,
and registers the MCP services of the nodes with Claude Code.

Install, from a terminal on the machine, as the user whose Claude Code is to be
set up:

    sudo bash <where this medium is mounted>/gucef-lab/install.sh

It asks for the lab secret when the medium does not hold it. Without the secret
the machine waits until it is approved on the cluster:

    sudo gucefLabLink --list
    sudo gucefLabLink --approve <name of the machine>

Follow what it does:

    journalctl -u gucef-lablink -f

Contents

    install.sh     installs the packages and sets up the service gucef-lablink
    linux/         the packages, DEB (Debian, Ubuntu, Raspberry Pi OS) and RPM
                   (Fedora, RHEL, openSUSE), per architecture (-amd64, -arm64;
                   -all for the scripts); install.sh picks the right ones:
                     gucef-k3s-scripts  scripts in /usr/share/gucef-k3s
                     gucef-mcp          the MCP server, service gucef-mcp
                     gucef-lablink      the lab link, service gucef-lablink
    docs/          the README of each service and of its program; installed
                   with it in /usr/lib/<service> and /opt/<service>/current
    lablink.env    settings for every machine, e.g. LAB_ALLOW (when the medium has them);
                   they go to /etc/gucef-lablink/settings.env
    lab-secret     the lab secret (only on a medium made with --with-secret)
    SHA256SUMS     checksums; install.sh checks them

The packages need glibc 2.34 or newer (Ubuntu 22.04, Debian 12, Raspberry Pi
OS 12, Fedora 35 and later). Removing them (apt-get remove gucef-lablink gucef-mcp
gucef-k3s-scripts, or dnf remove) removes the services; a node that joined
leaves the cluster with sudo /usr/share/gucef-k3s/agent-node/leave.sh.
