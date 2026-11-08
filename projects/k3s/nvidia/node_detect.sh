#!/bin/bash
# ---------------------------------------------------------------------------
# Node and GPU detection helpers. Source this file, then call the functions.
# Uses sysfs so it works without lspci/pciutils being installed.
#
# Set GUCEF_K3S_FORCE_NO_GPU=1 to pretend there is no GPU (for testing the
# no-GPU code paths on a machine that does have one).
# ---------------------------------------------------------------------------

# PCI vendor IDs
GUCEF_PCI_VENDOR_NVIDIA="0x10de"
GUCEF_PCI_VENDOR_AMD="0x1002"
GUCEF_PCI_VENDOR_INTEL="0x8086"

# Prints the vendor IDs of all display controllers (PCI class 0x03xxxx), one per line
gucef_list_display_vendors() {
    local dev
    for dev in /sys/bus/pci/devices/*; do
        [ -r "${dev}/class" ] || continue
        case "$(cat "${dev}/class")" in
            0x03*) cat "${dev}/vendor" ;;
        esac
    done
}

# Returns 0 if the host has at least one NVIDIA display controller
gucef_has_nvidia_gpu() {
    if [ "${GUCEF_K3S_FORCE_NO_GPU}" = "1" ]; then
        return 1
    fi
    gucef_list_display_vendors | grep -qi "^${GUCEF_PCI_VENDOR_NVIDIA}$"
}

# Prints a human readable summary of non-NVIDIA GPUs found (if any)
gucef_describe_other_gpus() {
    if [ "${GUCEF_K3S_FORCE_NO_GPU}" = "1" ]; then
        return 0
    fi
    local vendor
    for vendor in $(gucef_list_display_vendors | sort -u); do
        case "${vendor}" in
            "${GUCEF_PCI_VENDOR_AMD}")   echo "AMD GPU" ;;
            "${GUCEF_PCI_VENDOR_INTEL}") echo "Intel GPU" ;;
        esac
    done
}

# Prints the version of the currently loaded NVIDIA kernel driver (e.g. 595.45.04), or nothing
gucef_loaded_nvidia_driver_version() {
    if [ -r /proc/driver/nvidia/version ]; then
        grep -oE 'Kernel Module( for x86_64)?[[:space:]]+[0-9]+(\.[0-9]+)+' /proc/driver/nvidia/version \
            | grep -oE '[0-9]+(\.[0-9]+)+' | head -n 1
    fi
}

# Prints the total VRAM in MiB of the first NVIDIA GPU, or nothing if unknown
gucef_nvidia_vram_mib() {
    if command -v nvidia-smi > /dev/null 2>&1; then
        nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits 2>/dev/null | head -n 1 | tr -d ' '
    fi
}

# Prints the Kubernetes node name of this machine, or nothing if not found.
# Order: KS_NODE_NAME env var, KS_NODE_NAME node label (host-node/setup.sh),
# kubernetes.io/hostname label, node name, InternalIP.
gucef_k8s_node_name() {
    local kubeconfig="${1:-/etc/rancher/k3s/k3s.yaml}"
    local kubectl_cmd="kubectl --kubeconfig ${kubeconfig}"
    local host_name node_name node_ip
    if [ -n "${KS_NODE_NAME}" ]; then
        echo "${KS_NODE_NAME}"
        return 0
    fi
    host_name="$(hostname | tr '[:upper:]' '[:lower:]')"
    node_name="$(${kubectl_cmd} get nodes -l "KS_NODE_NAME=${host_name}" -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)"
    if [ -z "${node_name}" ]; then
        node_name="$(${kubectl_cmd} get nodes -l "kubernetes.io/hostname=${host_name}" -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)"
    fi
    if [ -z "${node_name}" ] && ${kubectl_cmd} get node "${host_name}" > /dev/null 2>&1; then
        node_name="${host_name}"
    fi
    if [ -z "${node_name}" ]; then
        node_ip="$(hostname -I | awk '{print $1}')"
        node_name="$(${kubectl_cmd} get nodes -o jsonpath="{.items[?(@.status.addresses[?(@.type=='InternalIP')].address=='${node_ip}')].metadata.name}" 2>/dev/null)"
    fi
    echo "${node_name}"
}
