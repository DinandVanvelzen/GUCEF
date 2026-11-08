#!/bin/bash
# ---------------------------------------------------------------------------
# Opens the ports the home lab needs in the firewall of this machine, for the
# lab networks only. Detects the firewall: ufw when it is active, firewalld when
# it runs; without either nothing is to be done. Idempotent.
#
# Usage: sudo ./open_firewall_ports.sh ROLE [CIDR ...]
#        sudo ./open_firewall_ports.sh close
#   close    takes out the rules this script added: ufw rules with the comment
#            "gucef-lab"; firewalld rules of the lab ports from a network
#   ROLE     cluster  a k3s server node: k3s API 6443/tcp, kubelet 10250/tcp,
#                     flannel 8472/udp, lab link 47810-47811/tcp, MCP 8791/tcp,
#                     discovery 5353/udp and 47812/udp
#            node     an agent node: kubelet 10250/tcp, flannel 8472/udp,
#                     MCP 8791/tcp, discovery 5353/udp and 47812/udp
#            machine  a machine that is no node: discovery 5353/udp and 47812/udp
#   CIDR     the networks the ports are open to; default: the networks of the
#            interfaces of this machine (not those of containers and VMs)
# ---------------------------------------------------------------------------

ROLE="$1"
shift
RANGES="$*"

case "${ROLE}" in
    cluster) TCP_PORTS="6443 10250 47810 47811 8791"; UDP_PORTS="8472 5353 47812" ;;
    node)    TCP_PORTS="10250 8791";                  UDP_PORTS="8472 5353 47812" ;;
    machine) TCP_PORTS="";                            UDP_PORTS="5353 47812" ;;
    close) ;;
    *) echo "Usage: open_firewall_ports.sh cluster|node|machine [CIDR ...] | close"; exit 1 ;;
esac

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run with sudo: it changes the firewall."
    exit 1
fi

if [ "${ROLE}" = "close" ]; then
    if command -v ufw > /dev/null 2>&1 && ufw status 2>/dev/null | grep -q "Status: active"; then
        # highest number first: deleting a rule renumbers the ones after it
        for number in $(ufw status numbered 2>/dev/null | grep "# gucef-lab" | sed -n 's/^\[ *\([0-9]*\)\].*/\1/p' | sort -rn); do
            ufw --force delete "${number}" > /dev/null
        done
        echo "Firewall (ufw): the ports of the lab are closed again"
    elif command -v firewall-cmd > /dev/null 2>&1 && firewall-cmd --state > /dev/null 2>&1; then
        firewall-cmd --permanent --list-rich-rules 2>/dev/null | \
            grep -E 'port port="?(6443|10250|47810|47811|8791|8472|5353|47812)"? protocol="?(tcp|udp)"? accept' | \
            while read -r rule; do firewall-cmd --permanent --quiet --remove-rich-rule="${rule}"; done
        firewall-cmd --reload --quiet
        echo "Firewall (firewalld): the ports of the lab are closed again"
    fi
    exit 0
fi

if [ -z "${RANGES}" ]; then
    RANGES="$(ip -o -4 addr show scope global 2>/dev/null | awk '$2 !~ /^(docker|br-|cni|flannel|veth|virbr|tailscale|lxc|kube)/ {print $4}' | \
        while read -r cidr; do python3 -c 'import ipaddress,sys; print(ipaddress.ip_interface(sys.argv[1]).network)' "${cidr}"; done | \
        sort -u | tr '\n' ' ')"
fi
if [ -z "${RANGES// /}" ]; then
    echo "WARNING: no network found to open the firewall to; the firewall is left as it is."
    exit 0
fi

if command -v ufw > /dev/null 2>&1 && ufw status 2>/dev/null | grep -q "Status: active"; then
    for range in ${RANGES}; do
        for port in ${TCP_PORTS}; do
            ufw allow from "${range}" to any port "${port}" proto tcp comment "gucef-lab" > /dev/null || exit 1
        done
        for port in ${UDP_PORTS}; do
            ufw allow from "${range}" to any port "${port}" proto udp comment "gucef-lab" > /dev/null || exit 1
        done
    done
    echo "Firewall (ufw): ${ROLE} ports open to ${RANGES% }"
elif command -v firewall-cmd > /dev/null 2>&1 && firewall-cmd --state > /dev/null 2>&1; then
    for range in ${RANGES}; do
        for port in ${TCP_PORTS}; do
            firewall-cmd --permanent --quiet --add-rich-rule="rule family=ipv4 source address=${range} port port=${port} protocol=tcp accept" 2>/dev/null
        done
        for port in ${UDP_PORTS}; do
            firewall-cmd --permanent --quiet --add-rich-rule="rule family=ipv4 source address=${range} port port=${port} protocol=udp accept" 2>/dev/null
        done
    done
    firewall-cmd --reload --quiet || exit 1
    echo "Firewall (firewalld): ${ROLE} ports open to ${RANGES% }"
fi
exit 0
