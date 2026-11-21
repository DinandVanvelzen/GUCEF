#!/bin/bash
# ---------------------------------------------------------------------------
# Gives the CI gateway (gucefMcpCi --mode gateway, installed by ci/mcp/install.sh) the narrow access to the cluster it
# needs to deploy the images the CI built. For every namespace that the "deploy" blocks of the CI registry (ci.json)
# name, it applies (kubectl apply, idempotent):
#
#   - the ServiceAccount gucef-deployer
#   - the Role gucef-deployer:
#       apps/deployments   get, patch   only the Deployments ci.json names in that namespace (resourceNames)
#       apps/deployments   list, watch  every Deployment of the namespace: Kubernetes cannot limit list and watch by
#                                       resourceNames (kubectl rollout status lists and watches)
#       apps/replicasets   get, list, watch   for rollout status, history and undo
#       pods               get, list, watch   for rollout status
#   - the RoleBinding gucef-deployer of that Role to that ServiceAccount
#   - the Secret gucef-deployer-token of type kubernetes.io/service-account-token: a token that does not expire,
#     valid until the Secret or the ServiceAccount is deleted
#
# All of them carry the label app.kubernetes.io/managed-by=gucef-ci-deployer, by which --remove finds them. Namespaces
# are never made: a namespace that does not exist is an error (the deployment of the service makes it).
#
# Then it writes ONE kubeconfig for the gateway (default /var/lib/gucef-ci/kubeconfig): the cluster (server and
# authority embedded), per namespace a user with the token of its ServiceAccount and a context named exactly like the
# namespace; the current context is the first namespace. The gateway selects the context per deploy target
# (kubectl --context <namespace>). The file is mode 0600 and, when this runs as root, owned by the gateway user
# (gucef-ci). Run as the owner of the cluster (admin kubectl access, no sudo), the file is left where it can be
# written (--out, or ~/.config/gucef-k3s/gucef-ci-deployer.kubeconfig when --out is not writable) and the one sudo
# command that puts it in place is printed. Tokens are never printed and never passed on a command line.
# Afterwards run sudo projects/k3s/ci/mcp/install.sh --role gateway, which passes the kubeconfig to the gateway.
#
# Usage:
#   ./setup.sh [--dry-run] [--ci-json FILE] [--kubeconfig-admin FILE] [--out FILE] [--server URL] [--owner USER]
#   ./setup.sh --verify [--ci-json FILE] [--out FILE]
#   ./setup.sh --remove [--dry-run] [--kubeconfig-admin FILE] [--out FILE]
#   --ci-json FILE           the CI registry with the deploy blocks; default /etc/gucef-ci/ci.json
#   --kubeconfig-admin FILE  the kubeconfig with admin access that applies the objects; default ~/.kube/config
#   --out FILE               the kubeconfig of the gateway; default /var/lib/gucef-ci/kubeconfig
#   --server URL             the API server the gateway uses; default the server of the admin kubeconfig, else
#                            https://<first address of this machine>:6443
#   --owner USER             the user the kubeconfig is given to when this runs as root; default gucef-ci
#   --verify                 checks with the kubeconfig of the gateway (kubectl auth can-i) that the ServiceAccounts
#                            may get and patch the listed Deployments and may not delete Deployments, patch others,
#                            or read or create Secrets
#   --remove                 deletes what this script made (by its label) and the kubeconfig
#   --dry-run                prints the manifests and the actions and changes nothing
# Needs kubectl (or k3s kubectl) and python3.
# ---------------------------------------------------------------------------

set -u
umask 077

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NAME="gucef-deployer"
TOKEN_SECRET="gucef-deployer-token"
MANAGED_BY="gucef-ci-deployer"
GATEWAY_KUBECONFIG="/var/lib/gucef-ci/kubeconfig"
STAGING_KUBECONFIG="${HOME}/.config/gucef-k3s/gucef-ci-deployer.kubeconfig"

CI_JSON="/etc/gucef-ci/ci.json"
ADMIN_KUBECONFIG="${HOME}/.kube/config"
OUT="${GATEWAY_KUBECONFIG}"
SERVER=""
OWNER="gucef-ci"
ACTION="setup"
DRY_RUN=false

usage()
{
    echo "Usage: $0 [--dry-run] [--ci-json FILE] [--kubeconfig-admin FILE] [--out FILE] [--server URL] [--owner USER]"
    echo "       $0 --verify [--ci-json FILE] [--out FILE]"
    echo "       $0 --remove [--dry-run] [--kubeconfig-admin FILE] [--out FILE]"
    exit "${1:-2}"
}
while [ $# -gt 0 ]; do
    case "$1" in
        --ci-json) [ $# -ge 2 ] || usage; CI_JSON="$2"; shift ;;
        --kubeconfig-admin) [ $# -ge 2 ] || usage; ADMIN_KUBECONFIG="$2"; shift ;;
        --out) [ $# -ge 2 ] || usage; OUT="$2"; shift ;;
        --server) [ $# -ge 2 ] || usage; SERVER="$2"; shift ;;
        --owner) [ $# -ge 2 ] || usage; OWNER="$2"; shift ;;
        --verify) ACTION="verify" ;;
        --remove) ACTION="remove" ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) usage 0 ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
    shift
done

dry() { [ "${DRY_RUN}" = "true" ]; }
if [ "${ACTION}" = "verify" ] && dry; then
    echo "ERROR: --verify changes nothing; it takes no --dry-run."
    exit 2
fi
if ! [[ "${OWNER}" =~ ^[a-z_][a-z0-9_-]*$ ]]; then
    echo "ERROR: --owner ${OWNER} is no user name."
    exit 2
fi
if [ -n "${SERVER}" ] && ! [[ "${SERVER}" =~ ^https://[A-Za-z0-9._:-]+(/[^[:space:]\"\']*)?$ ]]; then
    echo "ERROR: --server ${SERVER} is no URL of the form https://host:port."
    exit 2
fi
case "${OUT}" in
    /*) ;;
    *) OUT="$(pwd)/${OUT}" ;;
esac
dry && echo "DRY-RUN: nothing is changed."

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------
if ! command -v python3 > /dev/null 2>&1; then
    echo "ERROR: python3 is needed to read ci.json and to write the kubeconfig."
    exit 1
fi
KUBECTL=()
if command -v kubectl > /dev/null 2>&1; then
    KUBECTL=(kubectl)
elif command -v k3s > /dev/null 2>&1; then
    KUBECTL=(k3s kubectl)
elif ! dry; then
    echo "ERROR: neither kubectl nor k3s is installed here: run this where kubectl reaches the cluster with admin access."
    exit 1
fi
# kubectl with the admin kubeconfig; never with the kubeconfig of the gateway
kadmin() { "${KUBECTL[@]}" --kubeconfig "${ADMIN_KUBECONFIG}" "$@"; }
check_admin_kubeconfig()
{
    if [ ! -r "${ADMIN_KUBECONFIG}" ]; then
        echo "ERROR: the admin kubeconfig ${ADMIN_KUBECONFIG} cannot be read. Name one with --kubeconfig-admin FILE"
        echo "       (on the k3s server: a copy of /etc/rancher/k3s/k3s.yaml that you may read)."
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# The helper: reads ci.json, writes the manifests and the kubeconfig. Nothing secret is printed.
# ---------------------------------------------------------------------------
helper()
{
    python3 -I - "$@" <<'PYEOF'
import base64, json, os, re, subprocess, sys, time

NAME = "gucef-deployer"
TOKEN_SECRET = "gucef-deployer-token"
MANAGED_BY = "gucef-ci-deployer"
LABEL = re.compile(r"^[a-z0-9]([-a-z0-9]*[a-z0-9])?$")
SUBDOMAIN = re.compile(r"^[a-z0-9]([-a-z0-9]*[a-z0-9])?(\.[a-z0-9]([-a-z0-9]*[a-z0-9])?)*$")
SCHEMA = ('Each CI entry that deploys has "deploy": { "images": [ { "name": ..., "registryName": ..., "targets": '
          '{ "<target>": { "namespace": ..., "deployment": ..., "container": ..., "allowScratch": false } } } ] }.')

def plan(path):
    """namespace -> Deployments, in the order of ci.json"""
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except OSError as e:
        print("ERROR: %s cannot be read: %s" % (path, e.strerror))
        sys.exit(2)
    except ValueError as e:
        print("ERROR: %s is not valid JSON: %s" % (path, e))
        sys.exit(2)
    entries = data.get("ci") if isinstance(data, dict) else None
    if not isinstance(entries, list):
        print('ERROR: %s has no list "ci": it is not a CI registry.' % path)
        sys.exit(2)
    order, deployments, problems = [], {}, []
    for entry in entries:
        if not isinstance(entry, dict) or "deploy" not in entry:
            continue
        cid = str(entry.get("id", "?"))
        deploy = entry["deploy"]
        images = deploy.get("images") if isinstance(deploy, dict) else None
        if not isinstance(images, list):
            problems.append('CI id %s: "deploy" has no list "images"' % cid)
            continue
        for image in images:
            name = str(image.get("name", "?")) if isinstance(image, dict) else "?"
            targets = image.get("targets") if isinstance(image, dict) else None
            if not isinstance(targets, dict) or not targets:
                problems.append('CI id %s, image %s: no object "targets"' % (cid, name))
                continue
            for target, spec in targets.items():
                where = "CI id %s, image %s, target %s" % (cid, name, target)
                if not isinstance(spec, dict):
                    problems.append(where + ": not an object")
                    continue
                namespace, deployment = spec.get("namespace"), spec.get("deployment")
                if not isinstance(namespace, str) or len(namespace) > 63 or not LABEL.match(namespace):
                    problems.append(where + ': "namespace" is missing or no valid namespace name')
                    continue
                if not isinstance(deployment, str) or len(deployment) > 253 or not SUBDOMAIN.match(deployment):
                    problems.append(where + ': "deployment" is missing or no valid Deployment name')
                    continue
                if namespace not in deployments:
                    order.append(namespace)
                    deployments[namespace] = []
                if deployment not in deployments[namespace]:
                    deployments[namespace].append(deployment)
    if problems:
        for problem in problems:
            print("ERROR: %s: %s" % (path, problem))
        print("       " + SCHEMA)
        sys.exit(2)
    if not order:
        print('ERROR: %s names no deploy target: no CI entry has a "deploy" block with targets, so there is nothing' % path)
        print("       the gateway may deploy and nothing to set up. " + SCHEMA)
        sys.exit(2)
    return order, deployments

def quoted(text):
    return json.dumps(text)

def labels(indent):
    return ("%slabels:\n%s  app.kubernetes.io/name: %s\n%s  app.kubernetes.io/managed-by: %s\n"
            % (indent, indent, NAME, indent, MANAGED_BY))

def manifests(namespace, deployments):
    names = ", ".join(quoted(d) for d in sorted(deployments))
    meta = "metadata:\n  name: %s\n  namespace: %s\n" % (NAME, namespace) + labels("  ")
    out = []
    out.append("apiVersion: v1\nkind: ServiceAccount\n" + meta + "automountServiceAccountToken: false\n")
    out.append("apiVersion: rbac.authorization.k8s.io/v1\nkind: Role\n" + meta +
               "rules:\n"
               "  # kubectl set image, rollout undo and the change-cause annotation: only the Deployments of ci.json\n"
               "  - apiGroups: [\"apps\"]\n"
               "    resources: [\"deployments\"]\n"
               "    resourceNames: [%s]\n"
               "    verbs: [\"get\", \"patch\"]\n"
               "  # list and watch cannot be limited by resourceNames in Kubernetes (a list names no object), so they\n"
               "  # cover every Deployment of the namespace; kubectl rollout status lists and watches the Deployment\n"
               "  - apiGroups: [\"apps\"]\n"
               "    resources: [\"deployments\"]\n"
               "    verbs: [\"list\", \"watch\"]\n"
               "  # rollout status, history and undo follow the ReplicaSets and the pods\n"
               "  - apiGroups: [\"apps\"]\n"
               "    resources: [\"replicasets\"]\n"
               "    verbs: [\"get\", \"list\", \"watch\"]\n"
               "  - apiGroups: [\"\"]\n"
               "    resources: [\"pods\"]\n"
               "    verbs: [\"get\", \"list\", \"watch\"]\n" % names)
    out.append("apiVersion: rbac.authorization.k8s.io/v1\nkind: RoleBinding\n" + meta +
               "roleRef:\n  apiGroup: rbac.authorization.k8s.io\n  kind: Role\n  name: %s\n"
               "subjects:\n  - kind: ServiceAccount\n    name: %s\n    namespace: %s\n" % (NAME, NAME, namespace))
    # a token that does not expire: the token controller fills in token and ca.crt
    out.append("apiVersion: v1\nkind: Secret\ntype: kubernetes.io/service-account-token\n"
               "metadata:\n  name: %s\n  namespace: %s\n" % (TOKEN_SECRET, namespace) + labels("  ") +
               "  annotations:\n    kubernetes.io/service-account.name: %s\n" % NAME)
    return "---\n".join(out)

def secret_of(kubectl, admin, namespace):
    """token and ca.crt (base64) of the Secret; waits for the token controller"""
    command = kubectl + ["--kubeconfig", admin, "-n", namespace, "get", "secret", TOKEN_SECRET, "-o", "json"]
    for _ in range(60):
        result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode == 0:
            try:
                data = json.loads(result.stdout.decode("utf-8")).get("data") or {}
            except ValueError:
                data = {}
            if data.get("token") and data.get("ca.crt"):
                return base64.b64decode(data["token"]).decode("ascii"), data["ca.crt"]
        time.sleep(1)
    print("ERROR: the Secret %s/%s has no token after 60 seconds: is the token controller of the cluster running?" % (namespace, TOKEN_SECRET))
    sys.exit(1)

def kubeconfig(path, server, admin, namespaces):
    kubectl = json.loads(os.environ["DEPLOYER_KUBECTL"])
    authority = None
    users, contexts = [], []
    for namespace in namespaces:
        token, ca = secret_of(kubectl, admin, namespace)
        if not re.match(r"^[A-Za-z0-9._-]+$", token):
            print("ERROR: the token of %s/%s is not of the expected form." % (namespace, TOKEN_SECRET))
            sys.exit(1)
        authority = authority or ca
        user = "%s@%s" % (NAME, namespace)
        users.append("- name: %s\n  user:\n    token: %s\n" % (quoted(user), quoted(token)))
        contexts.append("- name: %s\n  context:\n    cluster: gucef-cluster\n    namespace: %s\n    user: %s\n"
                        % (quoted(namespace), quoted(namespace), quoted(user)))
    text = ("# The kubeconfig of the CI gateway, written by projects/k3s/ci/deployer/setup.sh: per namespace the token of its\n"
            "# ServiceAccount %s and a context named like the namespace. Do not edit; run setup.sh again.\n"
            "apiVersion: v1\nkind: Config\n"
            "clusters:\n- name: gucef-cluster\n  cluster:\n    server: %s\n    certificate-authority-data: %s\n"
            "users:\n%scontexts:\n%scurrent-context: %s\npreferences: {}\n"
            % (NAME, quoted(server), quoted(authority), "".join(users), "".join(contexts), quoted(namespaces[0])))
    # the file exists already, made with mode 0600 by mktemp
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)

mode = sys.argv[1]
if mode == "plan":
    order, deployments = plan(sys.argv[2])
    for namespace in order:
        print(namespace + "\t" + " ".join(sorted(deployments[namespace])))
elif mode == "manifests":
    sys.stdout.write(manifests(sys.argv[2], sys.argv[3:]))
elif mode == "kubeconfig":
    kubeconfig(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5:])
else:
    sys.exit(2)
PYEOF
}

# read_plan: NAMESPACES (in the order of ci.json) and DEPLOYMENTS_OF[namespace]
declare -A DEPLOYMENTS_OF=()
NAMESPACES=()
read_plan()
{
    local plan namespace deployments
    if ! plan="$(helper plan "${CI_JSON}")"; then
        printf '%s\n' "${plan}"
        exit 1
    fi
    while IFS=$'\t' read -r namespace deployments; do
        [ -n "${namespace}" ] || continue
        NAMESPACES+=("${namespace}")
        DEPLOYMENTS_OF["${namespace}"]="${deployments}"
    done <<< "${plan}"
}

# ---------------------------------------------------------------------------
# --remove: what this script made, found by its label, and the kubeconfig
# ---------------------------------------------------------------------------
if [ "${ACTION}" = "remove" ]; then
    SELECTOR="app.kubernetes.io/managed-by=${MANAGED_BY}"
    if dry; then
        echo "DRY-RUN: kubectl --kubeconfig ${ADMIN_KUBECONFIG} get rolebindings,roles,secrets,serviceaccounts --all-namespaces -l ${SELECTOR}"
        echo "DRY-RUN: delete each RoleBinding, Role, Secret and ServiceAccount found that is named ${NAME} or ${TOKEN_SECRET}"
        echo "DRY-RUN: remove ${OUT} and ${STAGING_KUBECONFIG} when they exist (sudo rm -f for a file this user cannot remove)"
        echo "Dry run: nothing was changed."
        exit 0
    fi
    check_admin_kubeconfig
    if ! FOUND="$(kadmin get rolebindings.rbac.authorization.k8s.io,roles.rbac.authorization.k8s.io,secrets,serviceaccounts \
            --all-namespaces -l "${SELECTOR}" \
            -o jsonpath='{range .items[*]}{.kind}{"\t"}{.metadata.namespace}{"\t"}{.metadata.name}{"\n"}{end}')"; then
        echo "ERROR: the objects of ${MANAGED_BY} cannot be listed with ${ADMIN_KUBECONFIG}."
        exit 1
    fi
    FAILED=false
    REMOVED=0
    # the bindings first, so that the access ends before its objects go
    for kind in RoleBinding Role Secret ServiceAccount; do
        while IFS=$'\t' read -r found_kind namespace name; do
            [ "${found_kind}" = "${kind}" ] || continue
            [ "${name}" = "${NAME}" ] || [ "${name}" = "${TOKEN_SECRET}" ] || continue
            case "${kind}" in
                RoleBinding) resource="rolebinding.rbac.authorization.k8s.io" ;;
                Role) resource="role.rbac.authorization.k8s.io" ;;
                Secret) resource="secret" ;;
                ServiceAccount) resource="serviceaccount" ;;
            esac
            if kadmin -n "${namespace}" delete "${resource}" "${name}" --ignore-not-found; then
                REMOVED=$(( REMOVED + 1 ))
            else
                FAILED=true
            fi
        done <<< "${FOUND}"
    done
    echo "Removed ${REMOVED} object(s) of ${MANAGED_BY}."
    for file in "${OUT}" "${STAGING_KUBECONFIG}"; do
        [ -e "${file}" ] || [ -L "${file}" ] || continue
        if rm -f "${file}" 2> /dev/null && [ ! -e "${file}" ]; then
            echo "Removed ${file}."
        else
            echo "Remove the kubeconfig of the gateway: sudo rm -f ${file}"
        fi
    done
    [ "${OUT}" != "${GATEWAY_KUBECONFIG}" ] && [ -e "${GATEWAY_KUBECONFIG}" ] && \
        echo "The gateway keeps ${GATEWAY_KUBECONFIG}, whose tokens no longer work: sudo rm -f ${GATEWAY_KUBECONFIG}"
    echo "Then run sudo projects/k3s/ci/mcp/install.sh --role gateway, which starts the gateway without it."
    [ "${FAILED}" = "true" ] && exit 1
    exit 0
fi

# ---------------------------------------------------------------------------
# --verify: what the ServiceAccounts may do, asked with the kubeconfig of the gateway
# ---------------------------------------------------------------------------
if [ "${ACTION}" = "verify" ]; then
    read_plan
    KUBECONFIG_FILE=""
    for file in "${OUT}" "${STAGING_KUBECONFIG}"; do
        if [ -r "${file}" ]; then KUBECONFIG_FILE="${file}"; break; fi
    done
    if [ -z "${KUBECONFIG_FILE}" ]; then
        echo "ERROR: ${OUT} cannot be read by $(id -un): it belongs to the gateway user. Run the check as root:"
        echo "       sudo $0 --verify --ci-json ${CI_JSON} --out ${OUT}"
        exit 1
    fi
    echo "Checking with ${KUBECONFIG_FILE}"
    WRONG=0
    # expect yes|no CONTEXT VERB RESOURCE [NAMESPACE]
    expect()
    {
        local wanted="$1" context="$2" verb="$3" resource="$4" namespace="${5:-$2}" answer
        answer="$("${KUBECTL[@]}" --kubeconfig "${KUBECONFIG_FILE}" --context "${context}" auth can-i "${verb}" "${resource}" -n "${namespace}" 2> /dev/null)"
        answer="${answer%%$'\n'*}"
        if [ "${answer}" = "${wanted}" ]; then
            printf '  ok     %-8s %-46s %-4s (%s)\n' "${verb}" "${resource}" "${answer}" "${namespace}"
        else
            printf '  WRONG  %-8s %-46s %-4s (%s), expected %s\n' "${verb}" "${resource}" "${answer:-error}" "${namespace}" "${wanted}"
            WRONG=$(( WRONG + 1 ))
        fi
    }
    CONTEXTS="$("${KUBECTL[@]}" --kubeconfig "${KUBECONFIG_FILE}" config get-contexts -o name 2> /dev/null)"
    for namespace in "${NAMESPACES[@]}"; do
        echo "Namespace ${namespace}"
        if ! grep -qxF -- "${namespace}" <<< "${CONTEXTS}"; then
            echo "  WRONG  the kubeconfig has no context ${namespace}: run $0 again"
            WRONG=$(( WRONG + 1 ))
            continue
        fi
        for deployment in ${DEPLOYMENTS_OF[${namespace}]}; do
            expect yes "${namespace}" get "deployments.apps/${deployment}"
            expect yes "${namespace}" patch "deployments.apps/${deployment}"
            expect no "${namespace}" delete "deployments.apps/${deployment}"
            expect no "${namespace}" update "deployments.apps/${deployment}"
        done
        expect yes "${namespace}" list deployments.apps
        expect yes "${namespace}" watch deployments.apps
        expect yes "${namespace}" list replicasets.apps
        expect yes "${namespace}" list pods
        expect no "${namespace}" delete deployments.apps
        expect no "${namespace}" create deployments.apps
        expect no "${namespace}" patch "deployments.apps/${NAME}-unlisted-probe"
        expect no "${namespace}" patch replicasets.apps
        expect no "${namespace}" delete pods
        expect no "${namespace}" create pods/exec
        expect no "${namespace}" create secrets
        expect no "${namespace}" get secrets
        expect no "${namespace}" list secrets
        expect no "${namespace}" patch roles.rbac.authorization.k8s.io
        if [ "${namespace}" != "kube-system" ]; then
            expect no "${namespace}" list pods kube-system
        fi
    done
    if [ "${WRONG}" -gt 0 ]; then
        echo "ERROR: ${WRONG} check(s) did not give what was expected."
        exit 1
    fi
    echo "All checks passed."
    exit 0
fi

# ---------------------------------------------------------------------------
# Setup: the objects per namespace
# ---------------------------------------------------------------------------
read_plan
echo "Deploy targets of ${CI_JSON}:"
for namespace in "${NAMESPACES[@]}"; do
    echo "  ${namespace}: ${DEPLOYMENTS_OF[${namespace}]}"
done

if dry; then
    [ -r "${ADMIN_KUBECONFIG}" ] || echo "DRY-RUN: note: the admin kubeconfig ${ADMIN_KUBECONFIG} cannot be read; a real run needs it."
else
    check_admin_kubeconfig
    MISSING=()
    for namespace in "${NAMESPACES[@]}"; do
        kadmin get namespace "${namespace}" -o name > /dev/null 2>&1 || MISSING+=("${namespace}")
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo "ERROR: these namespaces do not exist: ${MISSING[*]}. Deploy the services first (their deploy.sh makes"
        echo "       the namespace), or correct the deploy targets of ${CI_JSON}. Nothing was changed."
        exit 1
    fi
fi

for namespace in "${NAMESPACES[@]}"; do
    # shellcheck disable=SC2086
    MANIFESTS="$(helper manifests "${namespace}" ${DEPLOYMENTS_OF[${namespace}]})" || exit 1
    if dry; then
        echo ""
        echo "DRY-RUN: kubectl --kubeconfig ${ADMIN_KUBECONFIG} apply -f - with:"
        printf '%s\n' "${MANIFESTS}"
        continue
    fi
    echo ""
    echo "Namespace ${namespace}"
    if ! printf '%s\n' "${MANIFESTS}" | kadmin apply -f -; then
        echo "ERROR: the objects of ${namespace} could not be applied."
        exit 1
    fi
    for deployment in ${DEPLOYMENTS_OF[${namespace}]}; do
        kadmin -n "${namespace}" get deployment "${deployment}" -o name > /dev/null 2>&1 || \
            echo "WARNING: the Deployment ${namespace}/${deployment} does not exist (yet); the gateway can deploy it once it does."
    done
done

# ---------------------------------------------------------------------------
# The kubeconfig of the gateway
# ---------------------------------------------------------------------------
if [ -z "${SERVER}" ] && [ ${#KUBECTL[@]} -gt 0 ] && [ -r "${ADMIN_KUBECONFIG}" ]; then
    # the local file only: config view does not contact the cluster
    SERVER="$(kadmin config view --minify -o jsonpath='{.clusters[0].cluster.server}' 2> /dev/null)"
fi
if [ -z "${SERVER}" ]; then
    NODE_ADDRESS="$(hostname -I 2> /dev/null | awk '{print $1}')"
    [ -n "${NODE_ADDRESS}" ] && SERVER="https://${NODE_ADDRESS}:6443"
fi
if ! [[ "${SERVER}" =~ ^https://[A-Za-z0-9._:-]+(/[^[:space:]\"\']*)?$ ]]; then
    echo "ERROR: the API server (${SERVER:-none}) is no https URL: name it with --server https://<node>:6443."
    exit 1
fi

IS_ROOT=false
[ "$(id -u)" -eq 0 ] && IS_ROOT=true
OWNER_GROUP="$(id -gn "${OWNER}" 2> /dev/null)"
OUT_DIR="$(dirname "${OUT}")"
TARGET="${OUT}"
if [ "${IS_ROOT}" != "true" ] && { [ ! -d "${OUT_DIR}" ] || [ ! -w "${OUT_DIR}" ] || { [ -e "${OUT}" ] && [ ! -w "${OUT}" ]; }; }; then
    TARGET="${STAGING_KUBECONFIG}"
fi

if dry; then
    echo ""
    echo "DRY-RUN: wait for the token of each Secret ${TOKEN_SECRET} and write the kubeconfig (mode 0600) to ${TARGET}:"
    echo "DRY-RUN:   server ${SERVER}, the authority of the cluster, contexts ${NAMESPACES[*]} (current ${NAMESPACES[0]})"
    if [ "${IS_ROOT}" = "true" ]; then
        echo "DRY-RUN:   owner ${OWNER}${OWNER_GROUP:+:${OWNER_GROUP}}"
    elif [ "${TARGET}" != "${OUT}" ]; then
        echo "DRY-RUN:   then print: sudo install -m 0600 -o ${OWNER} -g ${OWNER_GROUP:-${OWNER}} ${TARGET} ${OUT} && rm -f ${TARGET}"
    fi
    echo "Dry run: nothing was changed."
    exit 0
fi

if [ "${TARGET}" = "${OUT}" ] && [ ! -d "${OUT_DIR}" ]; then
    echo "ERROR: ${OUT_DIR} does not exist: install the gateway first (sudo projects/k3s/ci/mcp/install.sh --role gateway)."
    exit 1
fi
mkdir -p "$(dirname "${TARGET}")" || exit 1
STAGE="$(mktemp "$(dirname "${TARGET}")/.kubeconfig.XXXXXX")" || exit 1
trap 'rm -f "${STAGE}"' EXIT
KUBECTL_JSON="$(python3 -I -c 'import json, sys; print(json.dumps(sys.argv[1:]))' "${KUBECTL[@]}")"
echo ""
echo "Waiting for the tokens of the ServiceAccounts"
DEPLOYER_KUBECTL="${KUBECTL_JSON}" helper kubeconfig "${STAGE}" "${SERVER}" "${ADMIN_KUBECONFIG}" "${NAMESPACES[@]}" || exit 1

WANTED_OWNER="$(id -un)"
if [ "${IS_ROOT}" = "true" ]; then
    if [ -n "${OWNER_GROUP}" ]; then
        WANTED_OWNER="${OWNER}"
        chown "${OWNER}:${OWNER_GROUP}" "${STAGE}" || exit 1
    else
        echo "WARNING: there is no user ${OWNER} (yet): the kubeconfig stays root's. Install the gateway (ci/mcp/install.sh), then run this again."
    fi
fi
chmod 600 "${STAGE}"
if [ -f "${TARGET}" ] && [ ! -L "${TARGET}" ] && cmp -s "${STAGE}" "${TARGET}" && \
   [ "$(stat -c '%a %U' "${TARGET}" 2> /dev/null)" = "600 ${WANTED_OWNER}" ]; then
    rm -f "${STAGE}"
    echo "The kubeconfig ${TARGET} is up to date."
else
    mv -f "${STAGE}" "${TARGET}" || exit 1
    echo "Wrote the kubeconfig ${TARGET} (mode 0600, owner ${WANTED_OWNER})."
fi
trap - EXIT

echo ""
echo "Contexts (one per namespace, the gateway selects them with --context <namespace>): ${NAMESPACES[*]}"
VERIFY="$0 --verify --ci-json ${CI_JSON} --out ${OUT}"
if [ "${IS_ROOT}" = "true" ]; then
    echo "Check it:  ${VERIFY}"
elif [ "${TARGET}" != "${OUT}" ]; then
    echo "Check it (with this copy):  ${VERIFY}"
    echo "Put it in place for the gateway user (this copy holds the tokens; the command removes it):"
    echo "  sudo install -m 0600 -o ${OWNER} -g ${OWNER_GROUP:-${OWNER}} ${TARGET} ${OUT} && rm -f ${TARGET}"
else
    echo "Check it:  ${VERIFY}"
    if [ "${OUT}" = "${GATEWAY_KUBECONFIG}" ]; then
        echo "Give it to the gateway user:"
        echo "  sudo chown ${OWNER}:${OWNER_GROUP:-${OWNER}} ${OUT}"
    fi
fi
echo "Then: sudo projects/k3s/ci/mcp/install.sh --role gateway   (passes it to the gateway and restarts it)"
exit 0
