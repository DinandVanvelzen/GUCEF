#!/bin/bash
# ---------------------------------------------------------------------------
# Cluster side of the MCP server (gucefMcpLocalLlm) on the nodes: what every
# node's MCP service needs from the cluster, delivered to /var/lib/gucef-mcp on
# each node by the DaemonSet gucef-mcp-sync. Runs on a server node; idempotent.
#
#   - a certificate authority for the MCP services (Secret gucef-mcp-ca), made
#     once. Clients trust its certificate (ca.crt) to reach the services over
#     HTTPS
#   - a certificate per node for its host name, node name and address, signed by
#     that authority, renewed when it expires within 60 days or the names changed
#     (Secret gucef-mcp-tls)
#   - the LLM servers of namespace llm as an endpoints file: the LoadBalancer
#     address of each instance, and node address and host port of instances on
#     ephemeral nodes (ConfigMap gucef-mcp-endpoints). The MCP services read the
#     file again when it changes
#   - the MCP services: every node with its HTTPS endpoint, "name url" lines
#     (ConfigMap gucef-mcp-endpoints, key mcp-nodes), for gucefLabLink, which
#     registers them with Claude Code of the machines of the lab
#   - the lab MCP token: one bearer token every node's MCP service accepts, so a
#     client needs one token for all of them (Secret gucef-mcp-llm, made once)
#   - an LLM server for every node with an NVIDIA GPU that has none (a machine
#     that joins, see agent-node/join.sh): llm/setup.sh --node, the model chosen
#     by its VRAM. Not for a node labelled gucef.org/llm=false (LAB_LLM=false of
#     the lab link of that machine), nor with LLM_AUTO_NODES=false in
#     llm/config.env
#   - DaemonSet gucef-mcp-sync, on every node including ephemeral ones: copies
#     the node's certificate and key, the CA certificate, the endpoints file and
#     the lab MCP token to /var/lib/gucef-mcp, owned by the owner of that directory
#
# Namespace gucef-mcp holds the keys of all nodes: whoever can read Secrets
# there, or is root on a node, can read them.
#
# Run as root it also installs the systemd timer gucef-mcp-cluster, which runs
# it with --if-changed every 2 minutes: a node that joins gets its certificate
# and is in the list of MCP services without anyone running this script.
#
# Usage: sudo ./cluster.sh [--if-changed]
#   --if-changed  only when the nodes, the LLM servers or this script changed
#                 since the last run, or that run is a day old (certificate
#                 renewal), or failed
#   llm/setup.sh, llm/uninstall.sh and lablink/install.sh run it
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="gucef-mcp"
LLM_NAMESPACE="llm"
CA_SECRET="gucef-mcp-ca"
TLS_SECRET="gucef-mcp-tls"
ENDPOINTS_CONFIGMAP="gucef-mcp-endpoints"
SYNC_IMAGE="${GUCEF_MCP_SYNC_IMAGE:-busybox:1.36}"
CERT_DAYS=730
RENEW_SECONDS=$(( 60 * 24 * 3600 ))
STATE_FILE="/var/lib/gucef-mcp-cluster/last-run"
TIMER="gucef-mcp-cluster"

IF_CHANGED=false
while [ $# -gt 0 ]; do
    case "$1" in
        --if-changed) IF_CHANGED=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

if [ ! -r "${KUBECONFIG_PATH}" ]; then
    echo "ERROR: ${KUBECONFIG_PATH} is not readable. Run on a server node with sudo."
    exit 1
fi
for tool in openssl python3; do
    if ! command -v "${tool}" > /dev/null 2>&1; then
        echo "ERROR: ${tool} is needed."
        exit 1
    fi
done

# what the result depends on: the nodes and their addresses, the LLM servers, this script, the day
FINGERPRINT="$( {
    ${KUBECTL} get nodes -o jsonpath='{range .items[*]}{.metadata.name} {.status.addresses} {.metadata.labels} {.status.allocatable.nvidia\.com/gpu}{"\n"}{end}' 2>/dev/null
    ${KUBECTL} get svc,deploy -n "${LLM_NAMESPACE}" -o jsonpath='{range .items[*]}{.metadata.name} {.metadata.resourceVersion}{"\n"}{end}' 2>/dev/null
    sha256sum "${BASH_SOURCE[0]}"
    date -u +%Y-%m-%d
} | sha256sum | cut -c1-32 )"
if [ "${IF_CHANGED}" = "true" ] && [ "$(cat "${STATE_FILE}" 2>/dev/null)" = "${FINGERPRINT}" ]; then
    exit 0
fi

# the timer that keeps this current, run from where this script is
if [ "$(id -u)" -eq 0 ] && [ -d /run/systemd/system ]; then
    SELF="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$(basename "${BASH_SOURCE[0]}")"
    SERVICE_UNIT="[Unit]
Description=GUCEF MCP services: certificates and lists of the nodes, kept current (llm/mcp/cluster.sh)
After=k3s.service

[Service]
Type=oneshot
Environment=KUBECONFIG_PATH=${KUBECONFIG_PATH}
ExecStart=${SELF} --if-changed"
    TIMER_UNIT="[Unit]
Description=Runs ${TIMER}.service every 2 minutes

[Timer]
OnBootSec=2min
OnUnitInactiveSec=2min

[Install]
WantedBy=timers.target"
    if [ "$(cat "/etc/systemd/system/${TIMER}.service" 2>/dev/null)" != "${SERVICE_UNIT}" ] ||
       [ "$(cat "/etc/systemd/system/${TIMER}.timer" 2>/dev/null)" != "${TIMER_UNIT}" ]; then
        echo "${SERVICE_UNIT}" > "/etc/systemd/system/${TIMER}.service"
        echo "${TIMER_UNIT}" > "/etc/systemd/system/${TIMER}.timer"
        systemctl daemon-reload
    fi
    systemctl enable --quiet --now "${TIMER}.timer"
fi

WORK_DIR="$(mktemp -d)"
chmod 700 "${WORK_DIR}"
trap 'rm -rf "${WORK_DIR}"' EXIT

${KUBECTL} create namespace "${NAMESPACE}" --dry-run=client -o yaml | ${KUBECTL} apply -f - > /dev/null

# ---------------------------------------------------------------------------
# Certificate authority
# ---------------------------------------------------------------------------
if ${KUBECTL} get secret "${CA_SECRET}" -n "${NAMESPACE}" > /dev/null 2>&1; then
    ${KUBECTL} get secret "${CA_SECRET}" -n "${NAMESPACE}" -o jsonpath='{.data.ca\.crt}' | base64 -d > "${WORK_DIR}/ca.crt"
    ${KUBECTL} get secret "${CA_SECRET}" -n "${NAMESPACE}" -o jsonpath='{.data.ca\.key}' | base64 -d > "${WORK_DIR}/ca.key"
else
    echo "Making the certificate authority of the MCP services"
    openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 -nodes \
        -keyout "${WORK_DIR}/ca.key" -out "${WORK_DIR}/ca.crt" -days 3650 \
        -subj "/CN=GUCEF MCP CA $(hostname)" \
        -addext "basicConstraints=critical,CA:TRUE,pathlen:0" \
        -addext "keyUsage=critical,keyCertSign,cRLSign" > /dev/null 2>&1 || { echo "ERROR: cannot make the CA."; exit 1; }
    ${KUBECTL} create secret generic "${CA_SECRET}" -n "${NAMESPACE}" \
        --from-file=ca.crt="${WORK_DIR}/ca.crt" --from-file=ca.key="${WORK_DIR}/ca.key" > /dev/null
fi

# ---------------------------------------------------------------------------
# A certificate per node
# ---------------------------------------------------------------------------
# the certificates there are, to keep those that are still good
mkdir -p "${WORK_DIR}/tls"
${KUBECTL} get secret "${TLS_SECRET}" -n "${NAMESPACE}" -o json 2>/dev/null | python3 -c '
import base64, json, os, sys
try:
    data = json.load(sys.stdin).get("data", {})
except ValueError:
    data = {}
for name, value in data.items():
    with open(os.path.join(sys.argv[1], name), "wb") as f:
        f.write(base64.b64decode(value))
' "${WORK_DIR}/tls"
cp "${WORK_DIR}/ca.crt" "${WORK_DIR}/tls/ca.crt"

# node name, host name and address of every node: "node host ip"
${KUBECTL} get nodes -o json | python3 -c '
import json, sys
for node in json.load(sys.stdin)["items"]:
    name = node["metadata"]["name"]
    host = node["metadata"].get("labels", {}).get("kubernetes.io/hostname", name)
    ips = [a["address"] for a in node["status"].get("addresses", []) if a["type"] == "InternalIP"]
    print(name, host, ips[0] if ips else "")
' > "${WORK_DIR}/nodes"

KEEP_FILES="ca.crt"
while read -r NODE HOST IP; do
    SAN="DNS:${NODE},DNS:localhost,IP:127.0.0.1"
    [ "${HOST}" != "${NODE}" ] && SAN="${SAN},DNS:${HOST}"
    [ -n "${IP}" ] && SAN="${SAN},IP:${IP}"
    CRT="${WORK_DIR}/tls/${NODE}.crt"
    KEY="${WORK_DIR}/tls/${NODE}.key"
    SANFILE="${WORK_DIR}/tls/${NODE}.san"
    KEEP_FILES="${KEEP_FILES} ${NODE}.crt ${NODE}.key ${NODE}.san"
    if [ -s "${CRT}" ] && [ -s "${KEY}" ] && [ "$(cat "${SANFILE}" 2>/dev/null)" = "${SAN}" ] && \
       openssl x509 -in "${CRT}" -noout -checkend "${RENEW_SECONDS}" > /dev/null 2>&1 && \
       openssl verify -CAfile "${WORK_DIR}/ca.crt" "${CRT}" > /dev/null 2>&1; then
        continue
    fi
    echo "Certificate for node ${NODE} (${SAN})"
    printf 'basicConstraints=CA:FALSE\nkeyUsage=critical,digitalSignature\nextendedKeyUsage=serverAuth\nsubjectAltName=%s\n' "${SAN}" > "${WORK_DIR}/ext"
    openssl req -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 -nodes -keyout "${KEY}" -out "${WORK_DIR}/csr" \
        -subj "/CN=${NODE}" > /dev/null 2>&1 &&
    openssl x509 -req -in "${WORK_DIR}/csr" -CA "${WORK_DIR}/ca.crt" -CAkey "${WORK_DIR}/ca.key" -CAcreateserial \
        -out "${CRT}" -days "${CERT_DAYS}" -extfile "${WORK_DIR}/ext" > /dev/null 2>&1 || { echo "ERROR: cannot make the certificate of ${NODE}."; exit 1; }
    echo "${SAN}" > "${SANFILE}"
done < "${WORK_DIR}/nodes"

FROM_FILES=()
for file in ${KEEP_FILES}; do
    FROM_FILES+=(--from-file="${file}=${WORK_DIR}/tls/${file}")
done
${KUBECTL} create secret generic "${TLS_SECRET}" -n "${NAMESPACE}" "${FROM_FILES[@]}" --dry-run=client -o yaml | \
    ${KUBECTL} apply -f - > /dev/null

# ---------------------------------------------------------------------------
# An LLM server for every GPU node without one
# ---------------------------------------------------------------------------
LLM_AUTO_NODES="$( [ -f "${SCRIPT_DIR}/../config.env" ] && . "${SCRIPT_DIR}/../config.env"; echo "${LLM_AUTO_NODES:-true}" )"
if [ "${LLM_AUTO_NODES}" = "true" ] && [ -x "${SCRIPT_DIR}/../setup.sh" ] && [ "$(id -u)" -eq 0 ]; then
    # GPU nodes that are no server, may have one, and have no instance of their own and no pod of namespace llm:
    # "ready NODE" when the node advertises its GPU, else "pending NODE LIFECYCLE" (the device plugin is not there yet;
    # an instance then would run on the CPU)
    ${KUBECTL} get nodes -o json > "${WORK_DIR}/nodes.json" 2>/dev/null
    ${KUBECTL} get deploy,pods -n "${LLM_NAMESPACE}" -o json > "${WORK_DIR}/llm.json" 2>/dev/null
    WANTED="$( python3 - "${WORK_DIR}/nodes.json" "${WORK_DIR}/llm.json" <<'PYEOF'
import json, sys
def items(path):
    try:
        with open(path) as f:
            return json.load(f).get("items", [])
    except Exception:
        return []
nodes = items(sys.argv[1])
llm = items(sys.argv[2])
deployments = set(i["metadata"]["name"] for i in llm if i.get("kind") == "Deployment")
busy = set(i.get("spec", {}).get("nodeName") for i in llm if i.get("kind") == "Pod")
for node in nodes:
    labels = node["metadata"].get("labels", {})
    name = node["metadata"]["name"]
    if labels.get("gucef.org/gpu") != "nvidia" or labels.get("gucef.org/llm") == "false":
        continue
    if "node-role.kubernetes.io/control-plane" in labels:
        continue
    if "llama-server-" + name in deployments or name in busy:
        continue
    gpus = node.get("status", {}).get("allocatable", {}).get("nvidia.com/gpu", "0")
    if gpus not in ( "", "0" ):
        print("ready", name)
    else:
        print("pending", name, labels.get("gucef.org/node-lifecycle", "-"))
PYEOF
)"
    # the device plugin advertises the GPUs of ephemeral nodes once it tolerates their taint
    if echo "${WANTED}" | grep -q "^pending .* ephemeral$" &&
       ! ${KUBECTL} get ds -n nvidia-device-plugin -o jsonpath='{..tolerations[*].key}' 2>/dev/null | grep -q "gucef.org/ephemeral"; then
        echo "The NVIDIA device plugin, also for ephemeral nodes"
        "${SCRIPT_DIR}/../../nvidia/setup.sh" || echo "WARNING: nvidia/setup.sh failed; GPUs of ephemeral nodes are not advertised."
    fi
    echo "${WANTED}" | grep "^pending " | while read -r _ NODE _; do
        echo "GPU node ${NODE} does not advertise its GPU yet: its LLM server follows once it does"
    done
    echo "${WANTED}" | grep "^ready " | while read -r _ NODE; do
        echo "An LLM server on GPU node ${NODE}"
        "${SCRIPT_DIR}/../setup.sh" --node "${NODE}" --no-wait --no-mcp || echo "WARNING: no LLM server on ${NODE}, see above."
    done
fi

# ---------------------------------------------------------------------------
# The LLM servers
# ---------------------------------------------------------------------------
LLM_API_KEY_VALUE=""
if [ -f "${SCRIPT_DIR}/../config.env" ]; then
    LLM_API_KEY_VALUE="$( . "${SCRIPT_DIR}/../config.env"; echo "${LLM_API_KEY}" )"
fi

{
    echo "# LLM servers of namespace ${LLM_NAMESPACE}, written by llm/mcp/cluster.sh"
    ${KUBECTL} get svc,deploy -n "${LLM_NAMESPACE}" -o json 2>/dev/null | python3 -c '
import json, subprocess, sys
items = json.load(sys.stdin).get("items", [])
deployments = { i["metadata"]["name"]: i for i in items if i["kind"] == "Deployment" }
nodes = {}
for line in open(sys.argv[1]):
    parts = line.split()
    if len(parts) == 3:
        nodes[parts[0]] = parts[2]          # by node name
        nodes[parts[1]] = parts[2]          # by host name, which the node selector of an instance uses
lines = []
for svc in items:
    if svc["kind"] != "Service" or not svc["metadata"]["name"].startswith("llama-server"):
        continue
    name = svc["metadata"]["name"]
    ports = svc["spec"].get("ports", [])
    if not ports:
        continue
    if svc["spec"].get("type") == "LoadBalancer":
        ingress = svc.get("status", {}).get("loadBalancer", {}).get("ingress", [])
        if ingress and ingress[0].get("ip"):
            lines.append((0, "http://%s:%d" % (ingress[0]["ip"], ports[0]["port"])))
        continue
    deployment = deployments.get(name)
    if deployment is None:
        continue
    pod = deployment["spec"]["template"]["spec"]
    node = pod.get("nodeSelector", {}).get("kubernetes.io/hostname")
    host_port = None
    for container in pod.get("containers", []):
        for port in container.get("ports", []):
            if port.get("hostPort"):
                host_port = port["hostPort"]
    if node in nodes and host_port:
        lines.append((1, "ephemeral http://%s:%d" % (nodes[node], host_port)))
for _, line in sorted(lines):
    print(line)
' "${WORK_DIR}/nodes"
} > "${WORK_DIR}/endpoints"

# the MCP service of every node, by the address its certificate names
MCP_TLS_PORT="$( [ -f "${SCRIPT_DIR}/config.env" ] && . "${SCRIPT_DIR}/config.env"; echo "${MCP_TLS_PORT:-8791}" )"
while read -r NODE HOST IP; do
    [ -n "${IP}" ] && echo "${NODE} https://${IP}:${MCP_TLS_PORT}/mcp"
done < "${WORK_DIR}/nodes" > "${WORK_DIR}/mcp-nodes"

${KUBECTL} create configmap "${ENDPOINTS_CONFIGMAP}" -n "${NAMESPACE}" --from-file=endpoints="${WORK_DIR}/endpoints" \
    --from-file=mcp-nodes="${WORK_DIR}/mcp-nodes" --dry-run=client -o yaml | ${KUBECTL} apply -f - > /dev/null
printf '%s' "${LLM_API_KEY_VALUE}" > "${WORK_DIR}/llm-api-key"
# the lab MCP token: made once, kept from then on
# the token alone, no line ending: it is a file the MCP services read as it is
${KUBECTL} get secret gucef-mcp-llm -n "${NAMESPACE}" -o jsonpath='{.data.mcp-token}' 2>/dev/null | base64 -d 2>/dev/null | tr -d '[:space:]' > "${WORK_DIR}/mcp-token"
if [ ! -s "${WORK_DIR}/mcp-token" ]; then
    openssl rand -base64 48 | tr -d '\n/+=' | cut -c1-43 | tr -d '\n' > "${WORK_DIR}/mcp-token"
fi
${KUBECTL} create secret generic gucef-mcp-llm -n "${NAMESPACE}" --from-file=llm-api-key="${WORK_DIR}/llm-api-key" \
    --from-file=mcp-token="${WORK_DIR}/mcp-token" --dry-run=client -o yaml | ${KUBECTL} apply -f - > /dev/null
echo "LLM servers for the MCP services:"
grep -v '^#' "${WORK_DIR}/endpoints" | sed 's/^/  /'

# ---------------------------------------------------------------------------
# Delivery to the nodes
# ---------------------------------------------------------------------------
cat <<EOF | ${KUBECTL} apply -f - > /dev/null
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: gucef-mcp-sync
  namespace: ${NAMESPACE}
spec:
  selector:
    matchLabels:
      app: gucef-mcp-sync
  template:
    metadata:
      labels:
        app: gucef-mcp-sync
    spec:
      # every node, also ephemeral ones and servers
      tolerations:
        - operator: Exists
      automountServiceAccountToken: false
      containers:
        - name: sync
          image: ${SYNC_IMAGE}
          env:
            - name: NODE_NAME
              valueFrom:
                fieldRef:
                  fieldPath: spec.nodeName
          command:
            - /bin/sh
            - -c
            - |
              # copies a file when it differs, through a temporary file: readers never see half of one
              put() {
                  src="\$1"; dst="\$2"; mode="\$3"
                  [ -s "\$src" ] || return 0
                  if ! cmp -s "\$src" "\$dst"; then
                      cp "\$src" "\$dst.tmp" && chmod "\$mode" "\$dst.tmp" && chown "\$owner" "\$dst.tmp" && mv -f "\$dst.tmp" "\$dst"
                  fi
              }
              while true; do
                  owner="\$(stat -c '%u:%g' /host)"
                  mkdir -p /host/tls
                  chown "\$owner" /host/tls
                  chmod 700 /host/tls
                  put "/tls/\${NODE_NAME}.crt" /host/tls/tls.crt 644
                  put "/tls/\${NODE_NAME}.key" /host/tls/tls.key 600
                  put /tls/ca.crt /host/tls/ca.crt 644
                  put /endpoints/endpoints /host/endpoints 644
                  put /endpoints/mcp-nodes /host/mcp-nodes 644
                  put /llm/mcp-token /host/lab-token 600
                  if [ -s /llm/llm-api-key ]; then
                      put /llm/llm-api-key /host/llm-api-key 600
                  else
                      rm -f /host/llm-api-key
                  fi
                  sleep 30
              done
          resources:
            requests:
              cpu: 1m
              memory: 4Mi
            limits:
              memory: 16Mi
          volumeMounts:
            - name: host
              mountPath: /host
            - name: tls
              mountPath: /tls
              readOnly: true
            - name: endpoints
              mountPath: /endpoints
              readOnly: true
            - name: llm
              mountPath: /llm
              readOnly: true
      volumes:
        - name: host
          hostPath:
            path: /var/lib/gucef-mcp
            type: DirectoryOrCreate
        - name: tls
          secret:
            secretName: ${TLS_SECRET}
        - name: endpoints
          configMap:
            name: ${ENDPOINTS_CONFIGMAP}
        - name: llm
          secret:
            secretName: gucef-mcp-llm
EOF
echo "The MCP files reach /var/lib/gucef-mcp on every node within a minute (DaemonSet ${NAMESPACE}/gucef-mcp-sync)."
if [ "$(id -u)" -eq 0 ]; then
    install -d -m 700 "$(dirname "${STATE_FILE}")"
    echo "${FINGERPRINT}" > "${STATE_FILE}"
fi
