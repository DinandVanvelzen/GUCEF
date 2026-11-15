#!/bin/bash
# Helper functions shared by the Forgejo scripts. Sourced, not executed.
# Expects KC (kubectl with kubeconfig) to be set by the caller.

fj_fail()
{
    echo "ERROR: $1" >&2
    [ -n "$2" ] && echo "       $2" >&2
    exit 1
}

# Finds the kubeseal CLI (PATH, then the location sealed-secrets/setup.sh installs to).
fj_find_kubeseal()
{
    if command -v kubeseal >/dev/null 2>&1; then
        command -v kubeseal
    elif [ -x "${HOME}/.local/bin/kubeseal" ]; then
        echo "${HOME}/.local/bin/kubeseal"
    fi
}

# Verifies everything Forgejo needs from the rest of the k3s setup. Exits with a
# message naming the script that provides the missing piece.
fj_preflight()
{
    ${KC} get nodes >/dev/null 2>&1 \
        || fj_fail "Cannot talk to the cluster with ${KUBECONFIG_PATH}." "Is k3s running, and is your user in the k3s-admin group? See ../setup.sh."

    local helm_ver
    helm_ver=$(helm version --short 2>/dev/null | sed -E 's/^v([0-9]+)\.([0-9]+).*/\1 \2/')
    [ -n "${helm_ver}" ] || fj_fail "helm is not installed." "../setup.sh installs it."
    set -- ${helm_ver}
    if [ "$1" -lt 3 ] || { [ "$1" -eq 3 ] && [ "$2" -lt 8 ]; }; then
        fj_fail "helm 3.8 or newer is required (found $(helm version --short))." "The Forgejo chart is pulled from an OCI registry."
    fi

    ${KC} -n kube-system get deployment sealed-secrets >/dev/null 2>&1 \
        || fj_fail "The Sealed Secrets controller is not installed." "Run ../sealed-secrets/setup.sh first."

    KUBESEAL_BIN=$(fj_find_kubeseal)
    [ -n "${KUBESEAL_BIN}" ] || fj_fail "kubeseal is not installed." "Run ../sealed-secrets/setup.sh, it installs the CLI."
    export KUBESEAL_BIN

    ${KC} get namespace "${PG_NAMESPACE}" >/dev/null 2>&1 \
        || fj_fail "PostgreSQL is not installed (namespace ${PG_NAMESPACE} not found)." "Run ../postgresql/setup.sh first."
    if [ -z "${PG_POD}" ]; then
        PG_POD=$(${KC} -n "${PG_NAMESPACE}" get pod -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)
    fi
    [ -n "${PG_POD}" ] || fj_fail "No PostgreSQL pod found in namespace ${PG_NAMESPACE}." "Run ../postgresql/setup.sh and wait for the pod to start."
    ${KC} -n "${PG_NAMESPACE}" get secret postgresql-credentials >/dev/null 2>&1 \
        || fj_fail "Secret postgresql-credentials not found in namespace ${PG_NAMESPACE}." "Run ../postgresql/setup.sh first."
}

# Storage class: FORGEJO_STORAGE_CLASS if set, else longhorn-raid (RAID backed) when this
# cluster has it, else the cluster default class.
fj_detect_storage_class()
{
    if [ -n "${FORGEJO_STORAGE_CLASS}" ]; then
        ${KC} get storageclass "${FORGEJO_STORAGE_CLASS}" >/dev/null 2>&1 \
            || fj_fail "Storage class ${FORGEJO_STORAGE_CLASS} (FORGEJO_STORAGE_CLASS) does not exist." "Available: $(${KC} get storageclass -o name | sed 's|.*/||' | tr '\n' ' ')"
        return
    fi
    if ${KC} get storageclass longhorn-raid >/dev/null 2>&1; then
        FORGEJO_STORAGE_CLASS="longhorn-raid"
        return
    fi
    FORGEJO_STORAGE_CLASS=$(${KC} get storageclass -o jsonpath='{range .items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=="true")]}{.metadata.name}{"\n"}{end}' | head -n 1)
    [ -n "${FORGEJO_STORAGE_CLASS}" ] \
        || fj_fail "No storage class found: longhorn-raid does not exist and the cluster has no default class." "Set FORGEJO_STORAGE_CLASS in config.env."
}

# Host clients use to reach Forgejo: FORGEJO_HOST if set, else the IP of the first node.
fj_detect_host()
{
    [ -n "${FORGEJO_HOST}" ] && return
    FORGEJO_HOST=$(${KC} get nodes -o jsonpath='{.items[0].status.addresses[?(@.type=="InternalIP")].address}' 2>/dev/null)
    [ -n "${FORGEJO_HOST}" ] || fj_fail "Could not detect the node address." "Set FORGEJO_HOST in config.env."
    FORGEJO_HOST_AUTODETECTED=true
}
