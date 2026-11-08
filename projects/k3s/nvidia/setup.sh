#!/bin/bash
# ---------------------------------------------------------------------------
# Kubernetes side of NVIDIA GPU support:
#   - labels this node as having an NVIDIA GPU (+ VRAM size)
#   - installs the NVIDIA k8s device plugin with GPU time-slicing
#   - runs a smoke test pod that executes nvidia-smi
#
# Requires nvidia/setup-host.sh to have completed (driver + container toolkit).
# Skips itself when there is no NVIDIA GPU.
# Settings: config.env (see config.env.example)
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KUBECONFIG_PATH="${KUBECONFIG_PATH:-/etc/rancher/k3s/k3s.yaml}"
KUBECTL="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="nvidia-device-plugin"
RELEASE_NAME="nvidia-device-plugin"
SMOKE_TEST_POD="gpu-smoke-test"

# shellcheck source=node_detect.sh
. "${SCRIPT_DIR}/node_detect.sh"

if [ -f "${SCRIPT_DIR}/config.env" ]; then
    # shellcheck disable=SC1091
    . "${SCRIPT_DIR}/config.env"
fi
GPU_TIMESLICE_REPLICAS="${GPU_TIMESLICE_REPLICAS:-4}"
NVIDIA_DEVICE_PLUGIN_VERSION="${NVIDIA_DEVICE_PLUGIN_VERSION:-0.20.1}"

if ! gucef_has_nvidia_gpu; then
    echo "No NVIDIA GPU found. Skipping NVIDIA device plugin setup."
    exit 0
fi

if ! command -v nvidia-container-runtime > /dev/null 2>&1; then
    echo "NVIDIA container toolkit not installed. Run 'sudo ${SCRIPT_DIR}/setup-host.sh' first. Skipping."
    exit 0
fi

LOADED_DRIVER="$(gucef_loaded_nvidia_driver_version)"
if [ -z "${LOADED_DRIVER}" ]; then
    echo "NVIDIA driver not loaded (reboot pending?). Skipping."
    exit 0
fi

NODE_NAME="$(gucef_k8s_node_name "${KUBECONFIG_PATH}")"
if [ -z "${NODE_NAME}" ]; then
    echo "ERROR: could not determine the Kubernetes node name of this machine."
    exit 1
fi
VRAM_MIB="$(gucef_nvidia_vram_mib)"
echo "Node ${NODE_NAME}: NVIDIA driver ${LOADED_DRIVER}, VRAM ${VRAM_MIB:-unknown} MiB"

# ---------------------------------------------------------------------------
# Node labels
#   nvidia.com/gpu.present  - selects the node for the device plugin DaemonSet
#   gucef.org/gpu           - GPU vendor, used by other components in this repo
#   gucef.org/gpu-vram-mib  - VRAM per GPU, used to size workloads (e.g. LLM model choice)
# ---------------------------------------------------------------------------
${KUBECTL} label node "${NODE_NAME}" --overwrite \
    nvidia.com/gpu.present=true \
    gucef.org/gpu=nvidia
if [ -n "${VRAM_MIB}" ]; then
    ${KUBECTL} label node "${NODE_NAME}" --overwrite gucef.org/gpu-vram-mib="${VRAM_MIB}"
fi

# ---------------------------------------------------------------------------
# Device plugin with time-slicing
# Time-slicing advertises each physical GPU as GPU_TIMESLICE_REPLICAS
# "nvidia.com/gpu" resources so several pods can share it. VRAM is not isolated.
# Tolerations: the chart defaults, plus the taint of ephemeral nodes
# (agent-node/join.sh --ephemeral) so the GPUs of those nodes are advertised too.
# ---------------------------------------------------------------------------
VALUES_FILE="$(mktemp)"
trap 'rm -f "${VALUES_FILE}"' EXIT
cat > "${VALUES_FILE}" <<EOF
runtimeClassName: nvidia
tolerations:
  - key: CriticalAddonsOnly
    operator: Exists
  - key: nvidia.com/gpu
    operator: Exists
    effect: NoSchedule
  - key: gucef.org/ephemeral
    operator: Exists
    effect: NoSchedule
config:
  default: time-slicing
  map:
    time-slicing: |-
      version: v1
      flags:
        migStrategy: none
      sharing:
        timeSlicing:
          renameByDefault: false
          failRequestsGreaterThanOne: false
          resources:
            - name: nvidia.com/gpu
              replicas: ${GPU_TIMESLICE_REPLICAS}
EOF

helm repo add nvdp https://nvidia.github.io/k8s-device-plugin --kubeconfig "${KUBECONFIG_PATH}" > /dev/null 2>&1 || true
helm repo update nvdp --kubeconfig "${KUBECONFIG_PATH}"

helm upgrade --install "${RELEASE_NAME}" nvdp/nvidia-device-plugin \
    --version "${NVIDIA_DEVICE_PLUGIN_VERSION}" \
    --namespace "${NAMESPACE}" \
    --create-namespace \
    --kubeconfig "${KUBECONFIG_PATH}" \
    --values "${VALUES_FILE}" \
    --wait --timeout 5m

echo "Waiting for the node to advertise nvidia.com/gpu..."
GPU_CAPACITY=""
for _ in $(seq 1 30); do
    GPU_CAPACITY="$(${KUBECTL} get node "${NODE_NAME}" -o jsonpath='{.status.allocatable.nvidia\.com/gpu}')"
    if [ -n "${GPU_CAPACITY}" ] && [ "${GPU_CAPACITY}" != "0" ]; then
        break
    fi
    sleep 5
done
if [ -z "${GPU_CAPACITY}" ] || [ "${GPU_CAPACITY}" = "0" ]; then
    echo "ERROR: node ${NODE_NAME} does not advertise nvidia.com/gpu. Check the device plugin logs:"
    echo "  ${KUBECTL} logs -n ${NAMESPACE} -l app.kubernetes.io/name=nvidia-device-plugin"
    exit 1
fi
echo "Node ${NODE_NAME} advertises nvidia.com/gpu: ${GPU_CAPACITY}"

# ---------------------------------------------------------------------------
# Smoke test: run nvidia-smi in a pod using the nvidia runtime
# ---------------------------------------------------------------------------
${KUBECTL} delete pod "${SMOKE_TEST_POD}" -n "${NAMESPACE}" --ignore-not-found > /dev/null
${KUBECTL} apply -n "${NAMESPACE}" -f "${SCRIPT_DIR}/gpu-test-pod.yaml"
if ${KUBECTL} wait pod "${SMOKE_TEST_POD}" -n "${NAMESPACE}" \
        --for=jsonpath='{.status.phase}'=Succeeded --timeout=300s; then
    ${KUBECTL} logs "${SMOKE_TEST_POD}" -n "${NAMESPACE}"
    echo "GPU smoke test passed."
    ${KUBECTL} delete pod "${SMOKE_TEST_POD}" -n "${NAMESPACE}" > /dev/null
else
    echo "ERROR: GPU smoke test failed. Pod status and logs:"
    ${KUBECTL} describe pod "${SMOKE_TEST_POD}" -n "${NAMESPACE}" | tail -n 20
    ${KUBECTL} logs "${SMOKE_TEST_POD}" -n "${NAMESPACE}"
    exit 1
fi
