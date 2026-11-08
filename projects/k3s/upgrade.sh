#!/bin/bash
# ---------------------------------------------------------------------------
# Upgrades k3s on all nodes of the cluster in place, using the k3s
# system-upgrade-controller. Non-destructive: workloads, cluster state and
# data are kept. Nodes are cordoned during their upgrade (no new pods are
# scheduled on them) but not drained.
#
# k3s supports upgrading one minor version at a time. When the target is more
# than one minor version ahead, the upgrade steps through the latest patch
# release of each minor version in between.
#
# Each step:
#   1. backs up this server node (k3s binary, datastore, token, credentials,
#      certificates, k3s configuration directory) and records the name,
#      address and k3s version of every node
#   2. upgrades the server nodes, then checks cluster health
#   3. upgrades the agent nodes, then checks cluster health
# Cluster health: all nodes Ready and no namespace with more unhealthy pods
# than before the step.
#
# When a phase fails or the cluster is unhealthy afterwards, the script offers
# a rollback of the nodes of that phase:
#   server phase: the datastore is restored from the backup and the server
#                 nodes return to their previous k3s version. Agent nodes were
#                 not upgraded yet.
#   agent phase:  the agent nodes return to their previous k3s version. The
#                 server nodes stay upgraded; agents one minor version behind
#                 the servers are supported by Kubernetes.
# Other nodes are rolled back over SSH (see K3S_SSH_USER, K3S_SSH_OPTS); this
# node directly. Before upgrading, SSH access to the other nodes is checked.
#
# Datastores:
#   embedded SQLite  single server node; the database is copied (k3s is stopped
#                    for a few seconds, running pods are not affected)
#   embedded etcd    one or more server nodes; an etcd snapshot is taken. A
#                    rollback restores it on this node and the other server
#                    nodes rejoin with their etcd data moved aside.
#   external         not backed up by this script; back it up separately and
#                    use --skip-backup
#
# Usage: sudo ./upgrade.sh [TARGET] [options]
#   TARGET            a k3s version (e.g. v1.34.13+k3s1) or a release channel:
#                     stable (default), latest, or a minor version channel (e.g. v1.34)
#   --check           show the nodes, current and target versions and the
#                     upgrade steps, change nothing
#   --rollback [DIR]  roll all nodes back to a backup (default: the most recent)
#   --yes             answer yes to all prompts
#   --skip-backup     upgrade without a backup; rollback is then not possible
#
# Run on a k3s server node.
# Environment:
#   K3S_BACKUP_DIR  backup location (default /var/backups/gucef-k3s)
#   K3S_DATA_DIR    k3s data directory (default /var/lib/rancher/k3s)
#   K3S_BIN         k3s binary (default /usr/local/bin/k3s)
#   K3S_CONFIG_DIR  k3s configuration directory (default /etc/rancher/k3s)
#   K3S_SSH_USER    SSH user for the other nodes (default root; other users
#                   need passwordless sudo)
#   K3S_SSH_OPTS    extra ssh options, e.g. "-i /home/me/.ssh/id_ed25519"
#
# The plans are rendered from k3s_upgrade_plan.yaml.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="system-upgrade"
PLAN_TEMPLATE="${SCRIPT_DIR}/k3s_upgrade_plan.yaml"
CHANNEL_URL="https://update.k3s.io/v1-release/channels"
SUC_URL="https://github.com/rancher/system-upgrade-controller/releases/latest/download"
BACKUP_ROOT="${K3S_BACKUP_DIR:-/var/backups/gucef-k3s}"
DATA_DIR="${K3S_DATA_DIR:-/var/lib/rancher/k3s}"
SERVER_DIR="${DATA_DIR}/server"
K3S_BIN="${K3S_BIN:-/usr/local/bin/k3s}"
K3S_CONFIG_DIR="${K3S_CONFIG_DIR:-/etc/rancher/k3s}"
SSH_USER="${K3S_SSH_USER:-root}"
SSH_OPTS="-o BatchMode=yes -o ConnectTimeout=10 ${K3S_SSH_OPTS}"
SERVER_SERVICE="k3s"
AGENT_SERVICE="k3s-agent"
SERVER_SELECTOR="node-role.kubernetes.io/control-plane=true"
AGENT_SELECTOR="!node-role.kubernetes.io/control-plane"
CHECK_INTERVAL=15
PHASE_TIMEOUT=3600
API_TIMEOUT=300
HEALTH_TIMEOUT=600

# shellcheck disable=SC1091
. "${SCRIPT_DIR}/nvidia/node_detect.sh"

TARGET="stable"
CHECK_ONLY=false
ROLLBACK=false
ROLLBACK_DIR=""
ASSUME_YES=false
SKIP_BACKUP=false
while [ $# -gt 0 ]; do
    case "$1" in
        --check) CHECK_ONLY=true ;;
        --yes) ASSUME_YES=true ;;
        --skip-backup) SKIP_BACKUP=true ;;
        --rollback)
            ROLLBACK=true
            if [ -n "$2" ] && [ "${2#-}" = "$2" ]; then
                ROLLBACK_DIR="$2"
                shift
            fi
            ;;
        -*) echo "Unknown option: $1"; exit 1 ;;
        *) TARGET="$1" ;;
    esac
    shift
done

# Root is needed for the backup (k3s data directory, k3s service) and the rollback
if [ "${CHECK_ONLY}" != "true" ] && [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: must run as root (use sudo). --check works without root."
    exit 1
fi
if [ ! -r "${KUBECONFIG_PATH}" ]; then
    echo "ERROR: cannot read ${KUBECONFIG_PATH}. Run host-node/set_kubeconfig_access.sh, or use sudo."
    exit 1
fi

# ---------------------------------------------------------------------------
# Script run on other nodes for a rollback.
# Arguments: VERSION ACTION SERVICE DATA_DIR
#   ACTION agent:         install k3s VERSION and restart SERVICE
#          stop:          stop SERVICE
#          server-rejoin: stop SERVICE, install k3s VERSION, move the server's
#                         etcd data aside and start SERVICE, which then rejoins
#                         the restored cluster
# The k3s binary is downloaded from the k3s GitHub release for the node's CPU
# architecture and verified against the release checksums.
# ---------------------------------------------------------------------------
read -r -d '' REMOTE_SCRIPT <<'EOF'
VERSION="$1"
ACTION="$2"
SERVICE="$3"
DATA_DIR="${4:-/var/lib/rancher/k3s}"
BIN="/usr/local/bin/k3s"

install_version() {
    local CURRENT ASSET SUMS URL EXPECTED
    CURRENT="$("${BIN}" --version 2>/dev/null | awk 'NR == 1 {print $3}')"
    [ "${CURRENT}" = "${VERSION}" ] && return 0
    case "$(uname -m)" in
        x86_64) ASSET="k3s"; SUMS="sha256sum-amd64.txt" ;;
        aarch64|arm64) ASSET="k3s-arm64"; SUMS="sha256sum-arm64.txt" ;;
        armv7*|armhf) ASSET="k3s-armhf"; SUMS="sha256sum-arm.txt" ;;
        *) echo "unsupported CPU architecture $(uname -m)"; return 1 ;;
    esac
    URL="https://github.com/k3s-io/k3s/releases/download/${VERSION}"
    curl -sfL "${URL}/${ASSET}" -o "${BIN}.rollback" || { echo "download of ${URL}/${ASSET} failed"; return 1; }
    EXPECTED="$(curl -sfL "${URL}/${SUMS}" | awk -v asset="${ASSET}" '$2 == asset {print $1}')"
    if [ -z "${EXPECTED}" ] || ! echo "${EXPECTED}  ${BIN}.rollback" | sha256sum -c - > /dev/null 2>&1; then
        echo "checksum verification of ${ASSET} ${VERSION} failed"
        rm -f "${BIN}.rollback"
        return 1
    fi
    chmod 755 "${BIN}.rollback" && mv -f "${BIN}.rollback" "${BIN}"
}

case "${ACTION}" in
    stop)
        systemctl stop "${SERVICE}"
        ;;
    agent)
        install_version && systemctl restart "${SERVICE}"
        ;;
    server-rejoin)
        systemctl stop "${SERVICE}" || exit 1
        install_version || exit 1
        if [ -d "${DATA_DIR}/server/db" ]; then
            mv "${DATA_DIR}/server/db" "${DATA_DIR}/server/db.before-rollback-$(date +%Y%m%d-%H%M%S)" || exit 1
        fi
        systemctl start "${SERVICE}"
        ;;
    *)
        echo "unknown action ${ACTION}"
        exit 1
        ;;
esac
EOF

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

# Resolves a channel name to the k3s version it currently points to
resolve_channel() {
    curl -s -o /dev/null -w '%{redirect_url}' "${CHANNEL_URL}/$1" | sed -e 's|.*/||' -e 's/%2B/+/g'
}

# Minor version number of a k3s version: v1.34.13+k3s1 -> 34
minor_of() {
    echo "$1" | sed -E 's/^v[0-9]+\.([0-9]+)\..*/\1/'
}

# k3s versions of the nodes matching label selector $1 (all nodes if empty)
node_versions() {
    local SELECTOR=()
    [ -n "$1" ] && SELECTOR=(-l "$1")
    ${KUBECTL} get nodes "${SELECTOR[@]}" -o jsonpath='{range .items[*]}{.status.nodeInfo.kubeletVersion}{"\n"}{end}' 2>/dev/null
}

# "<name> <internal-ip> <k3s-version>" for the nodes matching label selector $1
node_inventory() {
    ${KUBECTL} get nodes -l "$1" \
        -o jsonpath='{range .items[*]}{.metadata.name}{" "}{.status.addresses[?(@.type=="InternalIP")].address}{" "}{.status.nodeInfo.kubeletVersion}{"\n"}{end}' 2>/dev/null
}

# Lowest k3s version among all nodes
current_version() {
    node_versions "" | sort -V | head -n 1
}

# Number of nodes matching selector $2 whose version differs from $1.
# Empty if the API is unreachable.
nodes_not_at() {
    local VERSIONS
    VERSIONS="$(node_versions "$2")" || return
    if [ -z "${VERSIONS}" ]; then
        echo 0
        return
    fi
    echo "${VERSIONS}" | grep -vcx "$1"
}

# k3s version of the binary on this node
local_k3s_version() {
    "${K3S_BIN}" --version 2>/dev/null | awk 'NR == 1 {print $3}'
}

# Datastore of this server node. k3s labels embedded etcd members with
# node-role.kubernetes.io/etcd=true; a db/etcd directory alone can be a leftover.
datastore_type() {
    local NODE_NAME
    NODE_NAME="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
    if [ -n "${NODE_NAME}" ] && [ "$(${KUBECTL} get node "${NODE_NAME}" \
            -o jsonpath='{.metadata.labels.node-role\.kubernetes\.io/etcd}' 2>/dev/null)" = "true" ]; then
        echo "etcd"
    elif [ -f "${SERVER_DIR}/db/state.db" ]; then
        echo "sqlite"
    else
        echo "external"
    fi
}

wait_for_api() {
    local START
    START="$(date +%s)"
    until ${KUBECTL} get --raw /readyz > /dev/null 2>&1; do
        [ $(( $(date +%s) - START )) -gt "${API_TIMEOUT}" ] && return 1
        sleep 5
    done
}

# Removes the upgrade plans so the controller does not (re)upgrade nodes
delete_plans() {
    ${KUBECTL} -n "${NAMESPACE}" delete plans.upgrade.cattle.io server-plan agent-plan \
        --ignore-not-found --request-timeout=30s > /dev/null 2>&1
}

# Plan $2 (server-plan or agent-plan) from the template, pinned to version $1
render_plan() {
    sed -E -e '/^[[:space:]]*#?[[:space:]]*version:/d' \
           -e "s|^([[:space:]]*)channel:.*|\1version: $1|" \
           "${PLAN_TEMPLATE}" \
        | awk -v plan="$2" '
            BEGIN { n = 0 }
            /^---[[:space:]]*$/ { n++; next }
            { doc[n] = doc[n] $0 "\n" }
            END { for (i = 0; i <= n; i++) if (doc[i] ~ ("\n  name: " plan "\n")) printf "%s", doc[i] }'
}

# Runs a command on node address $1 as root over SSH
remote() {
    local IP="$1"
    shift
    if [ "${SSH_USER}" = "root" ]; then
        # shellcheck disable=SC2086
        ssh ${SSH_OPTS} "${SSH_USER}@${IP}" "$@"
    else
        # shellcheck disable=SC2086
        ssh ${SSH_OPTS} "${SSH_USER}@${IP}" sudo -n "$@"
    fi
}

# Runs the rollback script on node address $1 with arguments $2..
remote_rollback() {
    local IP="$1"
    shift
    remote "${IP}" bash -s -- "$@" <<< "${REMOTE_SCRIPT}"
}

# Unhealthy pods per namespace as "<count> <namespace>" lines.
# Unhealthy: not Completed, and not Running with all containers ready.
unhealthy_pods_by_namespace() {
    ${KUBECTL} get pods -A --no-headers 2>/dev/null | awk '
        { split($3, ready, "/") }
        $4 != "Completed" && ($4 != "Running" || ready[1] != ready[2]) { print $1 }' \
        | sort | uniq -c
}

# Namespaces with more unhealthy pods than in baseline $1
health_regressions() {
    local BASELINE="$1" COUNT NS BEFORE
    unhealthy_pods_by_namespace | while read -r COUNT NS; do
        BEFORE="$(echo "${BASELINE}" | awk -v ns="${NS}" '$2 == ns {print $1}')"
        if [ "${COUNT}" -gt "${BEFORE:-0}" ]; then
            echo "${NS}: ${BEFORE:-0} -> ${COUNT} unhealthy pod(s)"
        fi
    done
}

nodes_not_ready() {
    ${KUBECTL} get nodes --no-headers 2>/dev/null | awk '$2 !~ /^Ready/ {print $1}'
}

# Waits until all nodes are Ready and no namespace is worse than baseline $1,
# confirmed by two consecutive checks
check_health() {
    local BASELINE="$1" START PASSES=0 REGRESSIONS NOT_READY
    START="$(date +%s)"
    while true; do
        sleep 30
        if ! ${KUBECTL} get --raw /readyz > /dev/null 2>&1; then
            REGRESSIONS="API server not reachable"
            NOT_READY=""
            PASSES=0
        else
            REGRESSIONS="$(health_regressions "${BASELINE}")"
            NOT_READY="$(nodes_not_ready)"
            if [ -z "${REGRESSIONS}" ] && [ -z "${NOT_READY}" ]; then
                PASSES=$(( PASSES + 1 ))
                if [ "${PASSES}" -ge 2 ]; then
                    echo "Cluster healthy."
                    return 0
                fi
                continue
            fi
            PASSES=0
        fi
        if [ $(( $(date +%s) - START )) -gt "${HEALTH_TIMEOUT}" ]; then
            echo "ERROR: cluster not healthy ${HEALTH_TIMEOUT}s after the upgrade:"
            [ -n "${NOT_READY}" ] && echo "  nodes not Ready: ${NOT_READY//$'\n'/ }"
            [ -n "${REGRESSIONS}" ] && echo "${REGRESSIONS}" | sed 's/^/  /'
            return 1
        fi
        echo "Waiting for the cluster to settle..."
    done
}

# Applies plan $2 pinned to version $1 and waits until the nodes matching
# selector $3 run that version and are uncordoned
run_phase() {
    local VERSION="$1" PLAN="$2" SELECTOR="$3" START REMAINING CORDONED=""
    render_plan "${VERSION}" "${PLAN}" | ${KUBECTL} apply -f - || return 1
    START="$(date +%s)"
    while true; do
        sleep "${CHECK_INTERVAL}"
        REMAINING="$(nodes_not_at "${VERSION}" "${SELECTOR}")"
        if [ -z "${REMAINING}" ]; then
            echo "API server not reachable (expected while a server node restarts)..."
        elif [ "${REMAINING}" = "0" ]; then
            break
        else
            echo "${REMAINING} node(s) not yet at ${VERSION}:"
            ${KUBECTL} get nodes -l "${SELECTOR}" --no-headers \
                -o custom-columns=NAME:.metadata.name,VERSION:.status.nodeInfo.kubeletVersion,UNSCHEDULABLE:.spec.unschedulable 2>/dev/null \
                | sed 's/^/  /'
        fi
        if [ $(( $(date +%s) - START )) -gt "${PHASE_TIMEOUT}" ]; then
            echo "ERROR: upgrade to ${VERSION} did not complete within ${PHASE_TIMEOUT}s."
            echo "Inspect with: kubectl --kubeconfig ${KUBECONFIG_PATH} -n ${NAMESPACE} get plans,jobs,pods"
            return 1
        fi
    done
    # The controller uncordons each node after its upgrade
    for _ in $(seq 1 20); do
        CORDONED="$(${KUBECTL} get nodes -l "${SELECTOR}" -o jsonpath='{range .items[?(@.spec.unschedulable==true)]}{.metadata.name}{" "}{end}' 2>/dev/null)"
        [ -z "${CORDONED}" ] && break
        sleep "${CHECK_INTERVAL}"
    done
    if [ -n "${CORDONED}" ]; then
        echo "WARNING: node(s) still cordoned after the upgrade: ${CORDONED}"
        echo "Uncordon with: kubectl --kubeconfig ${KUBECONFIG_PATH} uncordon <node>"
    fi
}

# ---------------------------------------------------------------------------
# Backup
# ---------------------------------------------------------------------------

# Backs up this server node and records all nodes. Prints the backup directory on stdout.
backup_cluster() {
    local VERSION DATASTORE DIR ITEM RESULT
    VERSION="$(local_k3s_version)"
    DATASTORE="$(datastore_type)"
    DIR="${BACKUP_ROOT}/k3s-upgrade-$(date +%Y%m%d-%H%M%S)-${VERSION}"
    mkdir -p "${DIR}" || return 1
    chmod 700 "${DIR}"
    echo "${VERSION}" > "${DIR}/version"
    echo "${DATASTORE}" > "${DIR}/datastore"
    gucef_k8s_node_name "${KUBECONFIG_PATH}" > "${DIR}/self"
    node_inventory "${SERVER_SELECTOR}" > "${DIR}/servers.txt" || return 1
    node_inventory "${AGENT_SELECTOR}" > "${DIR}/agents.txt" || return 1
    echo "${REMOTE_SCRIPT}" > "${DIR}/rollback-node.sh"
    cp -a "${K3S_BIN}" "${DIR}/k3s" || return 1
    if [ -d "${K3S_CONFIG_DIR}" ]; then
        cp -a "${K3S_CONFIG_DIR}" "${DIR}/config" || return 1
    fi
    for ITEM in token cred tls; do
        if [ -e "${SERVER_DIR}/${ITEM}" ]; then
            cp -a "${SERVER_DIR}/${ITEM}" "${DIR}/${ITEM}" || return 1
        fi
    done
    case "${DATASTORE}" in
        sqlite)
            # k3s is stopped for a consistent copy of the database; running containers keep running
            echo "Stopping k3s briefly for a consistent datastore copy (running pods are not affected)..." >&2
            systemctl stop "${SERVER_SERVICE}" || return 1
            cp -a "${SERVER_DIR}/db" "${DIR}/db"
            RESULT=$?
            systemctl start "${SERVER_SERVICE}" || return 1
            [ "${RESULT}" -eq 0 ] || return 1
            wait_for_api || return 1
            ;;
        etcd)
            "${K3S_BIN}" etcd-snapshot save --dir "${DIR}/etcd-snapshot" --name pre-upgrade >&2 || return 1
            ;;
        *)
            return 1
            ;;
    esac
    # marks the backup as complete
    touch "${DIR}/complete"
    echo "${DIR}"
}

# Checks SSH access to all other nodes. Prints the nodes that cannot be reached.
unreachable_nodes() {
    local SELF NAME IP VERSION
    SELF="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
    { node_inventory "${SERVER_SELECTOR}"; node_inventory "${AGENT_SELECTOR}"; } \
        | while read -r NAME IP VERSION; do
            [ -z "${NAME}" ] || [ "${NAME}" = "${SELF}" ] && continue
            remote "${IP}" true < /dev/null > /dev/null 2>&1 || echo "${NAME} (${IP})"
        done
}

# ---------------------------------------------------------------------------
# Rollback
# ---------------------------------------------------------------------------

# Prints how to run the rollback script manually on a node
manual_rollback_hint() {
    local DIR="$1" NAME="$2" ARGS="$3"
    echo "  Roll back ${NAME} manually: copy ${DIR}/rollback-node.sh to it and run"
    echo "    sudo bash rollback-node.sh ${ARGS}"
}

# Restores etcd snapshot $1 on this server node with k3s stopped.
# Uses the same server arguments and environment as the k3s service.
etcd_cluster_reset() {
    local SNAPSHOT="$1" ARGV ENV_FILE ARGS
    ENV_FILE="/etc/systemd/system/${SERVER_SERVICE}.service.env"
    ARGV="$(systemctl show "${SERVER_SERVICE}" -p ExecStart --value | sed -n 's/.*argv\[\]=\([^;]*\);.*/\1/p' | head -n 1)"
    read -r -a ARGS <<< "${ARGV}"
    # drop the binary and the "server" subcommand
    ARGS=("${ARGS[@]:2}")
    (
        if [ -f "${ENV_FILE}" ]; then
            set -a
            # shellcheck disable=SC1090
            . "${ENV_FILE}"
            set +a
        fi
        timeout 900 "${K3S_BIN}" server "${ARGS[@]}" --cluster-reset --cluster-reset-restore-path="${SNAPSHOT}"
    )
}

# Rolls the server nodes back to backup directory $1: restores the datastore
# on this node, the other server nodes rejoin
rollback_servers() {
    local DIR="$1" VERSION DATASTORE SNAPSHOT TS SELF NAME IP NODE_VERSION FAILED=0
    VERSION="$(cat "${DIR}/version")"
    DATASTORE="$(cat "${DIR}/datastore")"
    SELF="$(cat "${DIR}/self")"

    SNAPSHOT=""
    if [ "${DATASTORE}" = "etcd" ]; then
        SNAPSHOT="$(find "${DIR}/etcd-snapshot" -type f 2>/dev/null | head -n 1)"
        if [ -z "${SNAPSHOT}" ]; then
            echo "ERROR: backup ${DIR} has no etcd snapshot to restore."
            return 1
        fi
    elif [ "${DATASTORE}" != "sqlite" ] || [ ! -d "${DIR}/db" ]; then
        echo "ERROR: backup ${DIR} has no datastore copy to restore."
        return 1
    fi

    # The other server nodes are stopped before the restore
    while read -r NAME IP NODE_VERSION; do
        [ -z "${NAME}" ] || [ "${NAME}" = "${SELF}" ] && continue
        echo "Stopping k3s on server node ${NAME} (${IP})..."
        if ! remote_rollback "${IP}" "${NODE_VERSION}" stop "${SERVER_SERVICE}" "${DATA_DIR}"; then
            echo "WARNING: could not stop k3s on ${NAME}. Stop it before it comes back online:"
            echo "  sudo systemctl stop ${SERVER_SERVICE}"
        fi
    done < "${DIR}/servers.txt"

    echo "Rolling back this node (${SELF}) to k3s ${VERSION}..."
    systemctl stop "${SERVER_SERVICE}" || return 1
    TS="$(date +%Y%m%d-%H%M%S)"
    # copy then rename, so the binary is replaced atomically
    { cp -a "${DIR}/k3s" "${K3S_BIN}.rollback" && mv -f "${K3S_BIN}.rollback" "${K3S_BIN}"; } || return 1
    if [ "${DATASTORE}" = "etcd" ]; then
        # k3s moves the current etcd data aside, restores the snapshot as a
        # single member cluster and exits
        echo "Restoring etcd snapshot ${SNAPSHOT} (log: ${DIR}/cluster-reset-${TS}.log)..."
        if ! etcd_cluster_reset "${SNAPSHOT}" > "${DIR}/cluster-reset-${TS}.log" 2>&1; then
            echo "ERROR: etcd snapshot restore failed. See ${DIR}/cluster-reset-${TS}.log"
            return 1
        fi
    else
        mv "${SERVER_DIR}/db" "${SERVER_DIR}/db.before-rollback-${TS}" || return 1
        cp -a "${DIR}/db" "${SERVER_DIR}/db" || return 1
    fi
    systemctl start "${SERVER_SERVICE}" || return 1
    if ! wait_for_api; then
        echo "ERROR: the API server did not come back after the rollback. Check: journalctl -u ${SERVER_SERVICE}"
        return 1
    fi
    delete_plans
    if [ "${DATASTORE}" = "etcd" ]; then
        echo "The etcd data from before the rollback is kept in ${SERVER_DIR}/db (etcd-old-*)"
    else
        echo "The datastore from before the rollback is kept in ${SERVER_DIR}/db.before-rollback-${TS}"
    fi

    # The other server nodes rejoin the restored cluster
    while read -r NAME IP NODE_VERSION; do
        [ -z "${NAME}" ] || [ "${NAME}" = "${SELF}" ] && continue
        echo "Server node ${NAME} (${IP}): k3s ${NODE_VERSION}, rejoining the restored cluster..."
        if ! remote_rollback "${IP}" "${NODE_VERSION}" server-rejoin "${SERVER_SERVICE}" "${DATA_DIR}"; then
            echo "ERROR: rollback of server node ${NAME} failed."
            manual_rollback_hint "${DIR}" "${NAME}" "${NODE_VERSION} server-rejoin ${SERVER_SERVICE} ${DATA_DIR}"
            FAILED=1
        fi
    done < "${DIR}/servers.txt"
    return "${FAILED}"
}

# Rolls the agent nodes back to their k3s versions recorded in backup directory $1
rollback_agents() {
    local DIR="$1" NAME IP NODE_VERSION FAILED=0
    while read -r NAME IP NODE_VERSION; do
        [ -z "${NAME}" ] && continue
        echo "Agent node ${NAME} (${IP}): rolling back to k3s ${NODE_VERSION}..."
        if ! remote_rollback "${IP}" "${NODE_VERSION}" agent "${AGENT_SERVICE}" "${DATA_DIR}"; then
            echo "ERROR: rollback of agent node ${NAME} failed."
            manual_rollback_hint "${DIR}" "${NAME}" "${NODE_VERSION} agent ${AGENT_SERVICE}"
            FAILED=1
        fi
    done < "${DIR}/agents.txt"
    return "${FAILED}"
}

# Rolls back to backup directory $1. Scope $2: servers, agents or all.
rollback() {
    local DIR="$1" SCOPE="$2" RESULT=0 NAME IP NODE_VERSION LISTS=()
    if [ ! -f "${DIR}/complete" ]; then
        echo "ERROR: ${DIR} is not a complete k3s upgrade backup."
        return 1
    fi
    delete_plans
    if [ "${SCOPE}" != "agents" ]; then
        rollback_servers "${DIR}" || RESULT=1
        LISTS+=("${DIR}/servers.txt")
    fi
    if [ "${SCOPE}" != "servers" ]; then
        rollback_agents "${DIR}" || RESULT=1
        LISTS+=("${DIR}/agents.txt")
    fi
    delete_plans
    wait_for_api
    # nodes cordoned by an interrupted upgrade
    cat "${LISTS[@]}" | while read -r NAME IP NODE_VERSION; do
        [ -n "${NAME}" ] && ${KUBECTL} uncordon "${NAME}" > /dev/null 2>&1
    done
    echo ""
    echo "Nodes after the rollback:"
    ${KUBECTL} get nodes -o wide
    return "${RESULT}"
}

# Offers a rollback of scope $2 to backup directory $1 after a failed phase, then exits
offer_rollback() {
    local DIR="$1" SCOPE="$2" ANSWER="n"
    if [ -z "${DIR}" ]; then
        echo "No backup was taken (--skip-backup); a rollback is not possible."
        exit 1
    fi
    if [ "${SCOPE}" = "servers" ]; then
        echo "Rollback: the datastore and the server nodes return to the state before this step (k3s $(cat "${DIR}/version"))."
    else
        echo "Rollback: the agent nodes return to their k3s versions before this step; the server nodes stay upgraded."
    fi
    if [ "${ASSUME_YES}" = "true" ]; then
        ANSWER="y"
    elif [ -t 0 ]; then
        read -r -p "Roll back the ${SCOPE}? [y/N] " ANSWER
    fi
    if [[ "${ANSWER}" =~ ^[Yy] ]]; then
        rollback "${DIR}" "${SCOPE}"
    else
        echo "Not rolled back. To roll back all nodes later: sudo ${SCRIPT_DIR}/upgrade.sh --rollback ${DIR}"
    fi
    exit 1
}

# ---------------------------------------------------------------------------
# Rollback mode
# ---------------------------------------------------------------------------
if [ "${ROLLBACK}" = "true" ]; then
    if [ -z "${ROLLBACK_DIR}" ]; then
        ROLLBACK_DIR="$(find "${BACKUP_ROOT}" -maxdepth 2 -path '*/k3s-upgrade-*/complete' 2>/dev/null | sort | tail -n 1)"
        ROLLBACK_DIR="${ROLLBACK_DIR%/complete}"
        if [ -z "${ROLLBACK_DIR}" ]; then
            echo "ERROR: no backups found in ${BACKUP_ROOT}"
            exit 1
        fi
    fi
    echo "Backup: ${ROLLBACK_DIR}"
    echo "Nodes and the k3s versions they return to:"
    cat "${ROLLBACK_DIR}/servers.txt" "${ROLLBACK_DIR}/agents.txt" 2>/dev/null | sed 's/^/  /'
    if [ "${ASSUME_YES}" != "true" ]; then
        read -r -p "Roll all nodes back to this backup? [y/N] " ANSWER
        [[ "${ANSWER}" =~ ^[Yy] ]] || { echo "Aborted. Nothing was changed."; exit 1; }
    fi
    rollback "${ROLLBACK_DIR}" all
    exit $?
fi

# ---------------------------------------------------------------------------
# Target version and upgrade steps
# ---------------------------------------------------------------------------
if [[ "${TARGET}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+ ]]; then
    TARGET_VERSION="${TARGET}"
else
    TARGET_VERSION="$(resolve_channel "${TARGET}")"
    if [[ ! "${TARGET_VERSION}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+ ]]; then
        echo "ERROR: could not resolve k3s channel '${TARGET}' via ${CHANNEL_URL}/${TARGET}"
        exit 1
    fi
fi

CURRENT_VERSION="$(current_version)"
if [ -z "${CURRENT_VERSION}" ]; then
    echo "ERROR: could not read the node versions from the cluster."
    exit 1
fi
SERVER_COUNT="$(node_inventory "${SERVER_SELECTOR}" | grep -c .)"
AGENT_COUNT="$(node_inventory "${AGENT_SELECTOR}" | grep -c .)"
echo "Nodes: ${SERVER_COUNT} server(s), ${AGENT_COUNT} agent(s)"
echo "Current k3s version (lowest across nodes): ${CURRENT_VERSION}"
echo "Target k3s version:                        ${TARGET_VERSION}"

if [ "$(nodes_not_at "${TARGET_VERSION}" "")" = "0" ]; then
    echo "All nodes already run ${TARGET_VERSION}. Nothing to do."
    exit 0
fi
if [ "$(printf '%s\n%s\n' "${CURRENT_VERSION}" "${TARGET_VERSION}" | sort -V | head -n 1)" != "${CURRENT_VERSION}" ]; then
    echo "ERROR: target ${TARGET_VERSION} is older than the current version ${CURRENT_VERSION}. Downgrades are not supported."
    exit 1
fi

# The latest patch of each minor version in between, then the target
CURRENT_MINOR="$(minor_of "${CURRENT_VERSION}")"
TARGET_MINOR="$(minor_of "${TARGET_VERSION}")"
STEPS=()
for (( MINOR = CURRENT_MINOR + 1; MINOR < TARGET_MINOR; MINOR++ )); do
    STEP_VERSION="$(resolve_channel "v1.${MINOR}")"
    if [[ ! "${STEP_VERSION}" =~ ^v1\.${MINOR}\. ]]; then
        echo "ERROR: could not resolve k3s channel v1.${MINOR} for the intermediate upgrade step."
        exit 1
    fi
    STEPS+=("${STEP_VERSION}")
done
STEPS+=("${TARGET_VERSION}")
echo "Upgrade steps: ${STEPS[*]}"

# ---------------------------------------------------------------------------
# Preconditions for backup and rollback
# ---------------------------------------------------------------------------
if [ "${SKIP_BACKUP}" != "true" ] && [ "$(id -u)" -ne 0 ]; then
    # only reached with --check
    echo "Datastore and SSH access checks need root; run with sudo to include them."
elif [ "${SKIP_BACKUP}" != "true" ]; then
    if [ ! -d "${SERVER_DIR}" ] || [ ! -x "${K3S_BIN}" ]; then
        echo "ERROR: this is not a k3s server node (${SERVER_DIR} or ${K3S_BIN} missing). Run on a server node."
        exit 1
    fi
    if [ "$(datastore_type)" = "external" ]; then
        echo "ERROR: k3s uses an external datastore, which this script cannot back up."
        echo "Back it up separately, then re-run with --skip-backup."
        exit 1
    fi
    echo "Datastore: $(datastore_type), backups: ${BACKUP_ROOT}"
    if [ $(( SERVER_COUNT + AGENT_COUNT )) -gt 1 ]; then
        UNREACHABLE="$(unreachable_nodes)"
        if [ -n "${UNREACHABLE}" ]; then
            echo "WARNING: no SSH access as ${SSH_USER} to these nodes; an automatic rollback cannot reach them:"
            echo "${UNREACHABLE}" | sed 's/^/  /'
            echo "Set K3S_SSH_USER / K3S_SSH_OPTS, or roll those nodes back manually if needed."
            if [ "${CHECK_ONLY}" != "true" ] && [ "${ASSUME_YES}" != "true" ]; then
                ANSWER="n"
                [ -t 0 ] && read -r -p "Continue anyway? [y/N] " ANSWER
                [[ "${ANSWER}" =~ ^[Yy] ]] || { echo "Aborted. Nothing was changed."; exit 1; }
            fi
        else
            echo "SSH access to all other nodes: OK"
        fi
    fi
fi
if [ "${CHECK_ONLY}" = "true" ]; then
    exit 0
fi

# ---------------------------------------------------------------------------
# system-upgrade-controller
# ---------------------------------------------------------------------------
# Installs the latest release, or updates an existing install
echo "Installing/updating the system-upgrade-controller"
${KUBECTL} apply -f "${SUC_URL}/crd.yaml" || exit 1
${KUBECTL} apply -f "${SUC_URL}/system-upgrade-controller.yaml" || exit 1
if ! ${KUBECTL} -n "${NAMESPACE}" rollout status deployment/system-upgrade-controller --timeout=300s; then
    # A pod stuck in container creation is replaced once
    echo "The system-upgrade-controller is not ready; restarting it once..."
    ${KUBECTL} -n "${NAMESPACE}" rollout restart deployment/system-upgrade-controller
    if ! ${KUBECTL} -n "${NAMESPACE}" rollout status deployment/system-upgrade-controller --timeout=300s; then
        echo "ERROR: the system-upgrade-controller did not become ready. Nothing was changed."
        ${KUBECTL} -n "${NAMESPACE}" get pods -o wide
        echo "Inspect with: kubectl --kubeconfig ${KUBECONFIG_PATH} -n ${NAMESPACE} describe pods"
        exit 1
    fi
fi

# Plans left over from earlier runs are removed so they are not part of a backup
delete_plans

# ---------------------------------------------------------------------------
# Upgrade, one step at a time
# ---------------------------------------------------------------------------
for STEP_VERSION in "${STEPS[@]}"; do
    echo ""
    echo "=== Upgrading to ${STEP_VERSION} ==="
    BASELINE="$(unhealthy_pods_by_namespace)"

    BACKUP_DIR=""
    if [ "${SKIP_BACKUP}" != "true" ]; then
        echo "Backing up..."
        if ! BACKUP_DIR="$(backup_cluster)"; then
            echo "ERROR: backup failed. Nothing was upgraded in this step."
            exit 1
        fi
        echo "Backup: ${BACKUP_DIR}"
    fi

    echo "--- Server nodes"
    if ! run_phase "${STEP_VERSION}" server-plan "${SERVER_SELECTOR}"; then
        offer_rollback "${BACKUP_DIR}" servers
    fi
    echo "Server nodes run ${STEP_VERSION}. Checking cluster health..."
    check_health "${BASELINE}" || offer_rollback "${BACKUP_DIR}" servers

    if [ "$(node_inventory "${AGENT_SELECTOR}" | grep -c .)" -gt 0 ]; then
        echo "--- Agent nodes"
        if ! run_phase "${STEP_VERSION}" agent-plan "${AGENT_SELECTOR}"; then
            offer_rollback "${BACKUP_DIR}" agents
        fi
        echo "Agent nodes run ${STEP_VERSION}. Checking cluster health..."
        check_health "${BASELINE}" || offer_rollback "${BACKUP_DIR}" agents
    fi
done

echo ""
echo "k3s upgrade complete:"
${KUBECTL} get nodes -o wide
if [ "${SKIP_BACKUP}" != "true" ]; then
    echo "Backups are kept in ${BACKUP_ROOT}; remove old ones when no longer needed."
fi
