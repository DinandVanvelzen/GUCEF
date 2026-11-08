#!/bin/bash
# ---------------------------------------------------------------------------
# Installs the k3s node and all components.
#
# Default: detects what is already present and installs what is missing.
# Re-runnable and non-destructive: an existing k3s install, its cluster data
# and workloads are kept; component scripts install missing components and
# update existing ones in place. k3s version upgrades are done by upgrade.sh.
#
# Options:
#   --reinstall  wipe the existing k3s cluster, including all workloads and
#                cluster data, and install k3s fresh. Asks for confirmation.
#   --yes        with --reinstall: skip the confirmation prompt
# ---------------------------------------------------------------------------

REINSTALL=false
ASSUME_YES=false
for arg in "$@"; do
    case "${arg}" in
        --reinstall) REINSTALL=true ;;
        --yes) ASSUME_YES=true ;;
        *) echo "Unknown option: ${arg}"; exit 1 ;;
    esac
done

K3S_INSTALLED=false
if [ -x /usr/local/bin/k3s ] && [ -f /etc/systemd/system/k3s.service ]; then
    K3S_INSTALLED=true
fi

if [ "${REINSTALL}" = "true" ] && [ "${K3S_INSTALLED}" = "true" ] && [ "${ASSUME_YES}" != "true" ]; then
    echo "--reinstall WIPES the existing k3s cluster and everything running on it."
    read -r -p "Type 'wipe' to continue, anything else aborts: " CONFIRM
    if [ "${CONFIRM}" != "wipe" ]; then
        echo "Aborted. Nothing was changed."
        exit 1
    fi
fi

# Host level GPU preparation (drivers + container toolkit) runs before k3s is installed
# so that k3s detects the NVIDIA container runtime on startup. Skips itself if there is no NVIDIA GPU.
cd ./nvidia
sudo ./setup-host.sh
HOST_GPU_RESULT=$?
cd ..
if [ ${HOST_GPU_RESULT} -eq 10 ]; then
    echo "A reboot is required to load the new GPU driver. Reboot and then re-run this script."
    exit 10
elif [ ${HOST_GPU_RESULT} -ne 0 ]; then
    echo "WARNING: GPU host setup failed (exit code ${HOST_GPU_RESULT}). Continuing without GPU support."
fi

if [ "${REINSTALL}" = "true" ] && [ "${K3S_INSTALLED}" = "true" ]; then
    # show what is running before it is wiped
    kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get all -A
    # kill all containers/pods, then remove k3s and its cluster data
    sudo /usr/local/bin/k3s-killall.sh
    sudo /usr/local/bin/k3s-uninstall.sh
    K3S_INSTALLED=false
fi

# kubectl and helm without sudo: the kubeconfig is readable by group k3s-admin, which the
# invoking user is added to. Set in the k3s config file, so it applies from the first k3s start.
sudo ./host-node/set_kubeconfig_access.sh

if [ "${K3S_INSTALLED}" = "true" ]; then
    echo "k3s is already installed ($(k3s --version | head -n 1)); keeping it. Use upgrade.sh to change the k3s version."
else
    sudo curl -sfL https://get.k3s.io | sh -
fi

# set the location of the kubeconfig
export KUBECONFIG=/etc/rancher/k3s/k3s.yaml

# check to see if we are back online
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o wide

# install the system upgrade controller
# this will be used for k3s version upgrades instead of a full reinstall
echo "Installing k3s system-upgrade-controller to manage future k3s upgrades"
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f https://github.com/rancher/system-upgrade-controller/releases/latest/download/system-upgrade-controller.yaml
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f https://github.com/rancher/system-upgrade-controller/releases/latest/download/crd.yaml

# download Helm 3
echo "Installing Helm 3 for helm chart support"
sudo curl -fsSL -o get_helm.sh https://raw.githubusercontent.com/helm/helm/master/scripts/get-helm-3
sudo chmod 700 get_helm.sh

# set up helm
sudo ./get_helm.sh
rm -f ./get_helm.sh

# confirm we are dealing with Helm 3
helm version

# set often used repos and update
echo "Configuring Helm repos"
helm repo add bitnami https://charts.bitnami.com/bitnami --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo add jenkins https://charts.jenkins.io --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo add gitlab https://charts.gitlab.io/ --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

# The local path provisioner comes standard with k3s as provisioner "rancher.io/local-path" (namespace kube-system).
# That built-in copy is used. An upstream copy conflicts with it and is removed if present.
cd ./local-path-provisioner
./remove_duplicate_provisioner.sh
cd ..

# Install Sealed Secrets controller for safe secret management in git
# Must be installed before any component that uses sealed secrets (e.g. immich)
cd ./sealed-secrets
./setup.sh
cd ..

# Make the local drives of the node available to k8s
# comment this line or edit the yaml for your own node
cd ./host-node
./setup.sh
cd ..

# Use Longhorn for storage management
# This will switch the default away from the local-storage-provisioner
cd ./longhorn
./setup.sh
cd ..

# install the common prereq of cert manager
cd ./cert-manager
./setup.sh
cd ..

# GPU support inside the cluster (NVIDIA device plugin). Skips itself if there is no NVIDIA GPU.
cd ./nvidia
./setup.sh
cd ..

# Local LLM server (llama.cpp llama-server serving a Qwen model)
# Uses the GPU if available, otherwise runs on CPU with a smaller model
cd ./llm
./setup.sh
cd ..

# setup the K9s TUI (Text User Interface)
cd ./k9s
./setup.sh
cd ..

# setup the Kubernetes dashboard
cd ./dashboard
./setup.sh
cd ..

# setup MySQL
cd ./mysql
./setup.sh
cd ..

# setup PostgreSQL
cd ./postgresql
./setup.sh
cd ..

# setup GitHub runner integration
cd ./github
./setup.sh
cd ..

# setup Nexus for storing build artifacts and deployment, CICD
cd ./nexus
./setup.sh
cd ..

# setup Jenkins for CICD
cd ./jenkins
./setup.sh
cd ..

# setup metrics support
cd ./metrics
./setup.sh
cd ..

# setup Immich photo management
# Requires: sealed-secrets (above), postgresql (above)
# Optional: create immich/setup-home.sh with your NAS symlinks before running
cd ./immich
./setup.sh
cd ..

# Make the kubeconfig available without needing sudo for every kubectl/helm call
mkdir -p ~/.kube
sudo cp /etc/rancher/k3s/k3s.yaml ~/.kube/config
sudo chown $USER:$USER ~/.kube/config
chmod 600 ~/.kube/config

