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
#   - DaemonSet gucef-mcp-sync, on every node including ephemeral ones: copies
#     the node's certificate and key, the CA certificate, the endpoints file and
#     the lab MCP token to /var/lib/gucef-mcp, owned by the owner of that directory
#
# Namespace gucef-mcp holds the keys of all nodes: whoever can read Secrets
# there, or is root on a node, can read them.
#
# Usage: ./cluster.sh          (run again when nodes or LLM instances change;
#                               llm/setup.sh and llm/uninstall.sh run it)
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
${KUBECTL} get secret gucef-mcp-llm -n "${NAMESPACE}" -o jsonpath='{.data.mcp-token}' 2>/dev/null | base64 -d > "${WORK_DIR}/mcp-token" 2>/dev/null
if [ ! -s "${WORK_DIR}/mcp-token" ]; then
    openssl rand -base64 48 | tr -d '\n/+=' | cut -c1-43 > "${WORK_DIR}/mcp-token"
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
