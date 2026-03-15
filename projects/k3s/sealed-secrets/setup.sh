#!/bin/bash

# Sealed Secrets - safe secret management for public git repos
#
# How it works:
#   1. This script installs the Sealed Secrets controller in the cluster
#   2. The controller generates a cluster-specific key pair on first install
#   3. The kubeseal CLI encrypts secrets using the cluster's public key
#   4. Encrypted SealedSecret manifests are safe to commit to git
#   5. Only the controller in THIS cluster can decrypt them
#
# After a full cluster rebuild (setup.sh), re-run this script first, then
# re-run create-sealed-secret.sh in each component directory to regenerate
# sealed secrets with the new cluster key.

KUBECONFIG_PATH=/etc/rancher/k3s/k3s.yaml
KUBESEAL_VERSION="0.27.3"

# Install the kubeseal CLI if not already present
if ! command -v kubeseal &> /dev/null; then
    echo "Installing kubeseal CLI v${KUBESEAL_VERSION}..."
    curl -L "https://github.com/bitnami-labs/sealed-secrets/releases/download/v${KUBESEAL_VERSION}/kubeseal-${KUBESEAL_VERSION}-linux-amd64.tar.gz" \
        -o /tmp/kubeseal.tar.gz
    tar -xzf /tmp/kubeseal.tar.gz -C /tmp kubeseal
    mkdir -p ~/.local/bin
    install -m 755 /tmp/kubeseal ~/.local/bin/kubeseal
    rm -f /tmp/kubeseal.tar.gz /tmp/kubeseal
    # Ensure ~/.local/bin is on PATH (add to .bashrc if not already there)
    if ! grep -q 'HOME/.local/bin' ~/.bashrc; then
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
    fi
    export PATH="$HOME/.local/bin:$PATH"
    echo "kubeseal installed: $(kubeseal --version)"
else
    echo "kubeseal already installed: $(kubeseal --version)"
fi

# Add the Sealed Secrets Helm repo
helm repo add sealed-secrets https://bitnami-labs.github.io/sealed-secrets --kubeconfig ${KUBECONFIG_PATH}
helm repo update --kubeconfig ${KUBECONFIG_PATH}

# Install or upgrade the Sealed Secrets controller
helm upgrade --install sealed-secrets sealed-secrets/sealed-secrets \
    --namespace kube-system \
    --kubeconfig ${KUBECONFIG_PATH}

echo ""
echo "Sealed Secrets controller installed."
echo "If this is a fresh cluster, wait ~30 seconds for the controller to generate its key pair,"
echo "then re-run create-sealed-secret.sh in each component directory."
echo ""
echo "To verify the controller is running:"
echo "  kubectl --kubeconfig ${KUBECONFIG_PATH} get pods -n kube-system -l app.kubernetes.io/name=sealed-secrets"
