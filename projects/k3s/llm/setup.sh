#!/bin/bash
# ---------------------------------------------------------------------------
# Local LLM server: llama.cpp llama-server serving a Qwen GGUF model.
#
# API (on <node-ip>:LLM_PORT):
#   OpenAI compatible:    /v1/chat/completions, /v1/completions, /v1/models, /v1/embeddings
#   Anthropic compatible: /v1/messages
#   Health / metrics:     /health, /metrics
#
# Uses the NVIDIA GPU when the node advertises nvidia.com/gpu (see nvidia/setup.sh),
# otherwise runs on CPU. The model is chosen from available VRAM and memory unless
# LLM_MODEL_HF is set. Settings: config.env (see config.env.example)
#
# Usage: ./setup.sh [--node NODE] [--instance NAME]
#   (no options)     the main instance "llama-server", on the first GPU node that
#                    is not ephemeral, otherwise on this node
#   --node NODE      an instance on that node. The instance name defaults to the
#                    node name: llama-server-<node>
#   --instance NAME  instance name; config.NAME.env, when present, is read after
#                    config.env and overrides its settings for this instance
#
# On an ephemeral node (agent-node/join.sh --ephemeral) the instance tolerates
# the node's taint, keeps its model in a directory on the node
# (LLM_HOST_MODEL_DIR) and listens on LLM_PORT of the node itself (hostPort)
# instead of a LoadBalancer port, so clients reach it directly while the node is
# up. In the cluster it is the ClusterIP service llama-server-<name>.
#
# Afterwards the MCP server gucefMcpLocalLlm is brought up to date (LLM_MCP,
# default true): mcp/cluster.sh gives every node's MCP service the list of LLM
# servers and its certificate, mcp/install.sh builds, tests and runs the service
# on this machine (see mcp/README.md).
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="llm"
PRIORITY_CLASS_NAME="gucef-low-priority"
CONTAINER_PORT=8080
PREFERRED_STORAGE_CLASS="longhorn-jbod"
EPHEMERAL_LABEL_KEY="gucef.org/node-lifecycle"
EPHEMERAL_TAINT_KEY="gucef.org/ephemeral"

# shellcheck source=../nvidia/node_detect.sh
. "${SCRIPT_DIR}/../nvidia/node_detect.sh"

ARG_NODE=""
ARG_INSTANCE=""
while [ $# -gt 0 ]; do
    case "$1" in
        --node) ARG_NODE="$2"; shift ;;
        --instance) ARG_INSTANCE="$2"; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done
INSTANCE="${ARG_INSTANCE:-${ARG_NODE}}"
if [ -n "${INSTANCE}" ] && [[ ! "${INSTANCE}" =~ ^[a-z0-9]([a-z0-9-]{0,40}[a-z0-9])?$ ]]; then
    echo "ERROR: instance name '${INSTANCE}' must be lower case letters, digits and '-'. Use --instance."
    exit 1
fi
APP_NAME="llama-server${INSTANCE:+-${INSTANCE}}"
PVC_NAME="${APP_NAME}-models"
SECRET_NAME="${APP_NAME}-secrets"

# ---------------------------------------------------------------------------
# Settings
# ---------------------------------------------------------------------------
if [ ! -f "${SCRIPT_DIR}/config.env" ]; then
    cp "${SCRIPT_DIR}/config.env.example" "${SCRIPT_DIR}/config.env"
    echo "Created ${SCRIPT_DIR}/config.env (all settings commented out, defaults apply)."
fi
# shellcheck disable=SC1091
. "${SCRIPT_DIR}/config.env"
if [ -n "${INSTANCE}" ] && [ -f "${SCRIPT_DIR}/config.${INSTANCE}.env" ]; then
    # shellcheck disable=SC1090
    . "${SCRIPT_DIR}/config.${INSTANCE}.env"
    echo "Read ${SCRIPT_DIR}/config.${INSTANCE}.env"
fi

LLM_MCP="${LLM_MCP:-true}"
LLM_CTX_SIZE="${LLM_CTX_SIZE:-131072}"
LLM_MULTIMODAL="${LLM_MULTIMODAL:-false}"
LLM_VRAM_RESERVE_MIB="${LLM_VRAM_RESERVE_MIB:-1024}"
LLM_STORAGE_SIZE="${LLM_STORAGE_SIZE:-100Gi}"
LLM_PORT="${LLM_PORT:-8080}"
LLM_PARALLEL="${LLM_PARALLEL:-1}"
LLM_IMAGE_CUDA="${LLM_IMAGE_CUDA:-ghcr.io/ggml-org/llama.cpp:server-cuda@sha256:a361e47e28e8801e290f3c855292e0f25047e5d8e998b0d101431f54db68e008}"
LLM_IMAGE_CPU="${LLM_IMAGE_CPU:-ghcr.io/ggml-org/llama.cpp:server@sha256:4d2de15a0ffbce704777c64026b40512a71e0bbc0b303a5fe5ea9bf71760dff9}"

# The MCP services: the LLM servers and certificates for all nodes, the service of this machine
mcp_sync() {
    [ "${LLM_MCP}" = "true" ] || return 0
    echo ""
    echo "MCP services (LLM_MCP=false in config.env leaves them alone)"
    "${SCRIPT_DIR}/mcp/cluster.sh" || echo "WARNING: mcp/cluster.sh failed; the MCP services keep what they have."
    if [ -n "${SUDO_USER}" ] && [ "${SUDO_USER}" != "root" ]; then
        "${SCRIPT_DIR}/mcp/install.sh" --user "${SUDO_USER}" || echo "WARNING: the MCP service of this machine was not updated, see above."
    else
        echo "The MCP service of this machine runs as a user: sudo ${SCRIPT_DIR}/mcp/install.sh from that user."
    fi
}

# ---------------------------------------------------------------------------
# Target node and hardware
# ---------------------------------------------------------------------------
# Whether node $1 advertises an allocatable NVIDIA GPU
node_has_gpu() {
    local ALLOCATABLE
    [ "${GUCEF_K3S_FORCE_NO_GPU}" = "1" ] && return 1
    ALLOCATABLE="$(${KUBECTL} get node "$1" -o jsonpath='{.status.allocatable.nvidia\.com/gpu}' 2>/dev/null)"
    [ -n "${ALLOCATABLE}" ] && [ "${ALLOCATABLE}" != "0" ]
}

USE_GPU=false
if [ -n "${ARG_NODE}" ]; then
    if ! ${KUBECTL} get node "${ARG_NODE}" > /dev/null 2>&1; then
        echo "ERROR: node '${ARG_NODE}' not found."
        exit 1
    fi
    TARGET_NODE="${ARG_NODE}"
    node_has_gpu "${TARGET_NODE}" && USE_GPU=true
else
    # Ephemeral nodes are not always on; they only get instances of their own (--node)
    GPU_NODE="$(${KUBECTL} get nodes -l "gucef.org/gpu=nvidia,${EPHEMERAL_LABEL_KEY}!=ephemeral" \
        -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)"
    if [ -n "${GPU_NODE}" ] && node_has_gpu "${GPU_NODE}"; then
        USE_GPU=true
        TARGET_NODE="${GPU_NODE}"
    fi
    if [ "${USE_GPU}" != "true" ]; then
        TARGET_NODE="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
        if [ -z "${TARGET_NODE}" ]; then
            TARGET_NODE="$(${KUBECTL} get nodes -o jsonpath='{.items[0].metadata.name}')"
        fi
    fi
fi
if [ -z "${TARGET_NODE}" ]; then
    echo "ERROR: no Kubernetes node found."
    exit 1
fi
TARGET_NODE_HOSTNAME="$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.metadata.labels.kubernetes\.io/hostname}')"
TARGET_NODE_HOSTNAME="${TARGET_NODE_HOSTNAME:-${TARGET_NODE}}"
EPHEMERAL=false
if [ "$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath="{.metadata.labels.gucef\.org/node-lifecycle}")" = "ephemeral" ]; then
    EPHEMERAL=true
    LLM_HOST_MODEL_DIR="${LLM_HOST_MODEL_DIR:-/var/lib/gucef/llm-models}"
fi

VRAM_MIB=0
if [ "${USE_GPU}" = "true" ]; then
    VRAM_MIB="$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.metadata.labels.gucef\.org/gpu-vram-mib}')"
    VRAM_MIB="${VRAM_MIB:-0}"
fi

# Node allocatable memory in MiB and CPU cores
NODE_MEM_RAW="$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.status.allocatable.memory}')"
case "${NODE_MEM_RAW}" in
    *Ki) NODE_MEM_MIB=$(( ${NODE_MEM_RAW%Ki} / 1024 )) ;;
    *Mi) NODE_MEM_MIB=${NODE_MEM_RAW%Mi} ;;
    *Gi) NODE_MEM_MIB=$(( ${NODE_MEM_RAW%Gi} * 1024 )) ;;
    *)   NODE_MEM_MIB=$(( NODE_MEM_RAW / 1048576 )) ;;
esac
NODE_CPU_RAW="$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.status.allocatable.cpu}')"
case "${NODE_CPU_RAW}" in
    *m) NODE_CPU_CORES=$(( ${NODE_CPU_RAW%m} / 1000 )) ;;
    *)  NODE_CPU_CORES=${NODE_CPU_RAW} ;;
esac

# Resource limits: default 50% of the node so other workloads keep their share
if [ -z "${LLM_MEMORY_LIMIT}" ]; then
    LLM_MEMORY_LIMIT="$(( NODE_MEM_MIB / 2 ))Mi"
fi
if [ -z "${LLM_CPU_LIMIT}" ]; then
    LLM_CPU_LIMIT=$(( NODE_CPU_CORES / 2 ))
    [ "${LLM_CPU_LIMIT}" -lt 1 ] && LLM_CPU_LIMIT=1
fi
case "${LLM_MEMORY_LIMIT}" in
    *Gi) MEM_LIMIT_MIB=$(( ${LLM_MEMORY_LIMIT%Gi} * 1024 )) ;;
    *Mi) MEM_LIMIT_MIB=${LLM_MEMORY_LIMIT%Mi} ;;
    *)   echo "ERROR: LLM_MEMORY_LIMIT must end in Mi or Gi (got '${LLM_MEMORY_LIMIT}')."; exit 1 ;;
esac
case "${LLM_CPU_LIMIT}" in
    *m) LLM_THREADS=$(( ${LLM_CPU_LIMIT%m} / 1000 )) ;;
    *)  LLM_THREADS=${LLM_CPU_LIMIT} ;;
esac
[ "${LLM_THREADS}" -lt 1 ] && LLM_THREADS=1

# Model selection: the MoE model needs about 32 GiB of VRAM and memory together
if [ -z "${LLM_MODEL_HF}" ]; then
    if [ $(( MEM_LIMIT_MIB + VRAM_MIB )) -ge 32768 ]; then
        LLM_MODEL_HF="unsloth/Qwen3.6-35B-A3B-GGUF:Q4_K_M"
    elif [ "${VRAM_MIB}" -ge 7000 ] || [ "${MEM_LIMIT_MIB}" -ge 12288 ]; then
        LLM_MODEL_HF="unsloth/Qwen3.5-9B-GGUF:Q4_K_M"
    else
        LLM_MODEL_HF="unsloth/Qwen3.5-4B-GGUF:Q4_K_M"
    fi
fi

# The name the server reports for its model, by which clients pick it (gucefMcpLocalLlm routes on it):
# the model of the Hugging Face repository in lower case, e.g. unsloth/Qwen3.6-35B-A3B-GGUF:Q4_K_M -> qwen3.6-35b-a3b
if [ -z "${LLM_MODEL_ALIAS}" ]; then
    LLM_MODEL_ALIAS="${LLM_MODEL_HF##*/}"
    LLM_MODEL_ALIAS="${LLM_MODEL_ALIAS%%:*}"
    LLM_MODEL_ALIAS="$(echo "${LLM_MODEL_ALIAS}" | tr '[:upper:]' '[:lower:]' | sed -e 's/-gguf$//')"
fi

echo "Instance:      ${APP_NAME}"
echo "Target node:   ${TARGET_NODE}$([ "${EPHEMERAL}" = "true" ] && echo " (ephemeral)")"
echo "GPU:           ${USE_GPU} (VRAM ${VRAM_MIB} MiB)"
echo "Limits:        memory ${LLM_MEMORY_LIMIT}, cpu ${LLM_CPU_LIMIT}"
echo "Model:         ${LLM_MODEL_HF} (served as '${LLM_MODEL_ALIAS}')"

# ---------------------------------------------------------------------------
# Port conflict check (k3s ServiceLB binds LoadBalancer ports on the node IP;
# an instance on an ephemeral node binds a host port of that node)
# ---------------------------------------------------------------------------
if [ "${EPHEMERAL}" = "true" ]; then
    CONFLICTING_PODS="$(${KUBECTL} get pods -A --field-selector "spec.nodeName=${TARGET_NODE}" \
        -o jsonpath='{range .items[*]}{.metadata.namespace}/{.metadata.name}{" "}{.spec.containers[*].ports[*].hostPort}{"\n"}{end}' \
        | grep -v "^${NAMESPACE}/${APP_NAME}-" | awk -v port="${LLM_PORT}" '{for (i = 2; i <= NF; i++) if ($i == port) print $1}')"
    if [ -n "${CONFLICTING_PODS}" ]; then
        echo "ERROR: host port ${LLM_PORT} on node ${TARGET_NODE} is already used by: ${CONFLICTING_PODS}"
        echo "Set LLM_PORT in ${SCRIPT_DIR}/config.${INSTANCE}.env to a free port."
        exit 1
    fi
else
    CONFLICTING_SERVICES="$(${KUBECTL} get svc -A -o jsonpath='{range .items[?(@.spec.type=="LoadBalancer")]}{.metadata.namespace}/{.metadata.name}{" "}{.spec.ports[*].port}{"\n"}{end}' \
        | grep -v "^${NAMESPACE}/${APP_NAME} " | awk -v port="${LLM_PORT}" '{for (i = 2; i <= NF; i++) if ($i == port) print $1}')"
    if [ -n "${CONFLICTING_SERVICES}" ]; then
        echo "ERROR: port ${LLM_PORT} is already used by LoadBalancer service(s): ${CONFLICTING_SERVICES}"
        echo "Set LLM_PORT in ${SCRIPT_DIR}/config.env to a free port."
        exit 1
    fi
    if ! ${KUBECTL} get svc "${APP_NAME}" -n "${NAMESPACE}" > /dev/null 2>&1 \
            && command -v ss > /dev/null 2>&1 \
            && [ -n "$(ss -ltnH "sport = :${LLM_PORT}" 2>/dev/null)" ]; then
        echo "ERROR: port ${LLM_PORT} is already in use on this host. Set LLM_PORT in ${SCRIPT_DIR}/config.env."
        exit 1
    fi
fi

# ---------------------------------------------------------------------------
# Namespace, priority class, secrets
# ---------------------------------------------------------------------------
${KUBECTL} create namespace "${NAMESPACE}" --dry-run=client -o yaml | ${KUBECTL} apply -f -

# Lower than the default priority (0): under resource pressure the LLM is evicted before other services
${KUBECTL} apply -f - <<EOF
apiVersion: scheduling.k8s.io/v1
kind: PriorityClass
metadata:
  name: ${PRIORITY_CLASS_NAME}
value: -100
preemptionPolicy: Never
globalDefault: false
description: "Workloads that yield to all other services under resource pressure"
EOF

SECRET_ARGS=()
[ -n "${LLM_API_KEY}" ] && SECRET_ARGS+=(--from-literal=LLAMA_API_KEY="${LLM_API_KEY}")
[ -n "${HF_TOKEN}" ] && SECRET_ARGS+=(--from-literal=HF_TOKEN="${HF_TOKEN}")
${KUBECTL} create secret generic "${SECRET_NAME}" -n "${NAMESPACE}" "${SECRET_ARGS[@]}" \
    --dry-run=client -o yaml | ${KUBECTL} apply -f -
if [ -z "${LLM_API_KEY}" ]; then
    echo "WARNING: LLM_API_KEY is not set; the API is open to anyone who can reach port ${LLM_PORT}. Keep it LAN-only."
fi

# ---------------------------------------------------------------------------
# Model storage: a PVC, or with LLM_HOST_MODEL_DIR a directory on the node
# (the default on ephemeral nodes, which have no Longhorn storage)
# ---------------------------------------------------------------------------
if [ -n "${LLM_HOST_MODEL_DIR}" ]; then
    echo "Model directory on node ${TARGET_NODE}: ${LLM_HOST_MODEL_DIR}"
elif ${KUBECTL} get pvc "${PVC_NAME}" -n "${NAMESPACE}" > /dev/null 2>&1; then
    echo "PVC ${PVC_NAME} already exists, keeping it."
else
    if [ -n "${LLM_STORAGE_CLASS}" ]; then
        if ! ${KUBECTL} get storageclass "${LLM_STORAGE_CLASS}" > /dev/null 2>&1; then
            echo "ERROR: StorageClass '${LLM_STORAGE_CLASS}' (LLM_STORAGE_CLASS) does not exist."
            exit 1
        fi
    elif ${KUBECTL} get storageclass "${PREFERRED_STORAGE_CLASS}" > /dev/null 2>&1; then
        LLM_STORAGE_CLASS="${PREFERRED_STORAGE_CLASS}"
    else
        LLM_STORAGE_CLASS="$(${KUBECTL} get storageclass \
            -o jsonpath="{range .items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=='true')]}{.metadata.name}{'\n'}{end}" | head -n 1)"
        if [ -z "${LLM_STORAGE_CLASS}" ]; then
            echo "ERROR: StorageClass '${PREFERRED_STORAGE_CLASS}' not found and there is no default StorageClass. Set LLM_STORAGE_CLASS."
            exit 1
        fi
        echo "WARNING: StorageClass '${PREFERRED_STORAGE_CLASS}' not found, using the default StorageClass '${LLM_STORAGE_CLASS}'."
    fi

    # Verify the storage class can provision a volume before creating the real PVC.
    # Only possible for Immediate binding; WaitForFirstConsumer classes bind when the pod starts.
    BINDING_MODE="$(${KUBECTL} get storageclass "${LLM_STORAGE_CLASS}" -o jsonpath='{.volumeBindingMode}')"
    if [ "${BINDING_MODE}" = "Immediate" ]; then
        echo "Testing StorageClass ${LLM_STORAGE_CLASS} with a small test PVC..."
        ${KUBECTL} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ${PVC_NAME}-test
  namespace: ${NAMESPACE}
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: ${LLM_STORAGE_CLASS}
  resources:
    requests:
      storage: 1Gi
EOF
        if ! ${KUBECTL} wait pvc "${PVC_NAME}-test" -n "${NAMESPACE}" --for=jsonpath='{.status.phase}'=Bound --timeout=120s; then
            echo "ERROR: test PVC on StorageClass ${LLM_STORAGE_CLASS} did not bind."
            ${KUBECTL} describe pvc "${PVC_NAME}-test" -n "${NAMESPACE}" | tail -n 10
            ${KUBECTL} delete pvc "${PVC_NAME}-test" -n "${NAMESPACE}" --wait=false
            exit 1
        fi
        ${KUBECTL} delete pvc "${PVC_NAME}-test" -n "${NAMESPACE}"
    fi

    # Model files are re-downloadable from Hugging Face: scratch storage, no redundancy needed
    ${KUBECTL} apply -f - <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ${PVC_NAME}
  namespace: ${NAMESPACE}
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: ${LLM_STORAGE_CLASS}
  resources:
    requests:
      storage: ${LLM_STORAGE_SIZE}
EOF
fi

# ---------------------------------------------------------------------------
# Deployment
# ---------------------------------------------------------------------------
SERVER_ARGS=(
    -hf "${LLM_MODEL_HF}"
    --alias "${LLM_MODEL_ALIAS}"
    --host 0.0.0.0
    --port "${CONTAINER_PORT}"
    -c "${LLM_CTX_SIZE}"
    -np "${LLM_PARALLEL}"
    -t "${LLM_THREADS}"
    -fa on
    --cache-type-k q8_0
    --cache-type-v q8_0
    --metrics
)
if [ "${LLM_MULTIMODAL}" != "true" ]; then
    SERVER_ARGS+=(--no-mmproj)
fi
if [ "${USE_GPU}" = "true" ]; then
    SERVER_ARGS+=(--fit-target "${LLM_VRAM_RESERVE_MIB}")
fi
if [ -n "${LLM_EXTRA_ARGS}" ]; then
    read -r -a EXTRA_ARGS_ARRAY <<< "${LLM_EXTRA_ARGS}"
    SERVER_ARGS+=("${EXTRA_ARGS_ARRAY[@]}")
fi
ARGS_YAML=""
for arg in "${SERVER_ARGS[@]}"; do
    ARGS_YAML+="            - \"${arg}\""$'\n'
done

if [ "${USE_GPU}" = "true" ]; then
    IMAGE="${LLM_IMAGE_CUDA}"
    RUNTIME_CLASS_YAML="      runtimeClassName: nvidia"
    GPU_LIMIT_YAML="              nvidia.com/gpu: 1"
else
    IMAGE="${LLM_IMAGE_CPU}"
    RUNTIME_CLASS_YAML=""
    GPU_LIMIT_YAML=""
fi

TOLERATIONS_YAML=""
HOST_PORT_YAML=""
SERVICE_TYPE="LoadBalancer"
if [ "${EPHEMERAL}" = "true" ]; then
    TOLERATIONS_YAML="      tolerations:
        - key: ${EPHEMERAL_TAINT_KEY}
          operator: Exists
          effect: NoSchedule"
    HOST_PORT_YAML="              hostPort: ${LLM_PORT}"
    SERVICE_TYPE="ClusterIP"
fi
if [ -n "${LLM_HOST_MODEL_DIR}" ]; then
    MODELS_VOLUME_YAML="          hostPath:
            path: ${LLM_HOST_MODEL_DIR}
            type: DirectoryOrCreate"
else
    MODELS_VOLUME_YAML="          persistentVolumeClaim:
            claimName: ${PVC_NAME}"
fi

${KUBECTL} apply -f - <<EOF
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ${APP_NAME}
  namespace: ${NAMESPACE}
  labels:
    app: ${APP_NAME}
spec:
  replicas: 1
  # Covers the startupProbe window (720 x 10s) so a long first model download
  # is not reported as a failed rollout
  progressDeadlineSeconds: 7500
  strategy:
    type: Recreate
  selector:
    matchLabels:
      app: ${APP_NAME}
  template:
    metadata:
      labels:
        app: ${APP_NAME}
    spec:
${RUNTIME_CLASS_YAML}
${TOLERATIONS_YAML}
      priorityClassName: ${PRIORITY_CLASS_NAME}
      nodeSelector:
        kubernetes.io/hostname: ${TARGET_NODE_HOSTNAME}
      containers:
        - name: ${APP_NAME}
          image: ${IMAGE}
          args:
${ARGS_YAML}          env:
            - name: LLAMA_CACHE
              value: /models
            - name: LLAMA_API_KEY
              valueFrom:
                secretKeyRef:
                  name: ${SECRET_NAME}
                  key: LLAMA_API_KEY
                  optional: true
            - name: HF_TOKEN
              valueFrom:
                secretKeyRef:
                  name: ${SECRET_NAME}
                  key: HF_TOKEN
                  optional: true
          ports:
            - name: http
              containerPort: ${CONTAINER_PORT}
${HOST_PORT_YAML}
          resources:
            requests:
              cpu: 500m
              memory: 1Gi
            limits:
              cpu: "${LLM_CPU_LIMIT}"
              memory: ${LLM_MEMORY_LIMIT}
${GPU_LIMIT_YAML}
          # The first start downloads the model, which can take a long time
          startupProbe:
            httpGet:
              path: /health
              port: http
            periodSeconds: 10
            failureThreshold: 720
          readinessProbe:
            httpGet:
              path: /health
              port: http
            periodSeconds: 10
          livenessProbe:
            httpGet:
              path: /health
              port: http
            periodSeconds: 30
            failureThreshold: 6
          volumeMounts:
            - name: models
              mountPath: /models
      volumes:
        - name: models
${MODELS_VOLUME_YAML}
---
apiVersion: v1
kind: Service
metadata:
  name: ${APP_NAME}
  namespace: ${NAMESPACE}
  labels:
    app: ${APP_NAME}
spec:
  type: ${SERVICE_TYPE}
  selector:
    app: ${APP_NAME}
  ports:
    - name: http
      port: ${LLM_PORT}
      targetPort: http
EOF

# ---------------------------------------------------------------------------
# Wait and smoke test
# ---------------------------------------------------------------------------
echo ""
if [ "$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.status.conditions[?(@.type=="Ready")].status}')" != "True" ]; then
    echo "Node ${TARGET_NODE} is not up. ${APP_NAME} starts when it is."
    mcp_sync
    exit 0
fi
echo "Waiting for ${APP_NAME} to become ready. The first start downloads the model."
echo "Follow progress with: kubectl --kubeconfig ${KUBECONFIG_PATH} logs -f -n ${NAMESPACE} deploy/${APP_NAME}"
if ! ${KUBECTL} rollout status "deployment/${APP_NAME}" -n "${NAMESPACE}" --timeout=7200s; then
    echo "ERROR: llama-server did not become ready."
    ${KUBECTL} logs -n "${NAMESPACE}" "deploy/${APP_NAME}" --tail=30
    exit 1
fi

LB_IP=""
if [ "${EPHEMERAL}" = "true" ]; then
    LB_IP="$(${KUBECTL} get node "${TARGET_NODE}" -o jsonpath='{.status.addresses[?(@.type=="InternalIP")].address}')"
fi
for _ in $(seq 1 30); do
    [ -n "${LB_IP}" ] && break
    LB_IP="$(${KUBECTL} get svc "${APP_NAME}" -n "${NAMESPACE}" -o jsonpath='{.status.loadBalancer.ingress[0].ip}')"
    [ -n "${LB_IP}" ] && break
    sleep 2
done
if [ -z "${LB_IP}" ]; then
    echo "WARNING: no LoadBalancer IP assigned to service ${APP_NAME}."
    mcp_sync
    exit 0
fi
LLM_URL="http://${LB_IP}:${LLM_PORT}"

AUTH_HEADER=()
[ -n "${LLM_API_KEY}" ] && AUTH_HEADER=(-H "Authorization: Bearer ${LLM_API_KEY}")
echo "Smoke test: chat completion"
SMOKE_RESPONSE="$(curl -s --max-time 300 "${AUTH_HEADER[@]}" -H "Content-Type: application/json" \
    "${LLM_URL}/v1/chat/completions" \
    -d "{\"model\":\"${LLM_MODEL_ALIAS}\",\"max_tokens\":64,\"messages\":[{\"role\":\"user\",\"content\":\"Reply with the single word: ready\"}]}")"
if echo "${SMOKE_RESPONSE}" | grep -q '"choices"'; then
    echo "Smoke test passed."
else
    echo "WARNING: smoke test did not return a completion: ${SMOKE_RESPONSE}"
fi

echo ""
echo "LLM server ready."
echo "  OpenAI compatible base URL:    ${LLM_URL}/v1"
echo "  Anthropic compatible base URL: ${LLM_URL}"
echo "  Model name:                    ${LLM_MODEL_ALIAS}"

mcp_sync
