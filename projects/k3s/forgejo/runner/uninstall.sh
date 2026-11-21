#!/bin/bash
# Removes a Forgejo runner. --wipe-data also deletes its volume (job image cache, BuildKit cache).
#
# Usage: runner/uninstall.sh [--oci] [--wipe-data]
#   (no option)  the runner forgejo-runner (runner/setup.sh)
#   --oci        the image build runner forgejo-runner-oci (runner/setup.sh --oci); forgejo-runner is not touched
KC="kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml"
NAME="forgejo-runner"
WIPE=false
for argument in "$@"; do
    case "${argument}" in
        --oci) NAME="forgejo-runner-oci" ;;
        --wipe-data) WIPE=true ;;
        *) echo "Usage: $0 [--oci] [--wipe-data]" >&2; exit 2 ;;
    esac
done
${KC} -n forgejo delete deployment ${NAME} --ignore-not-found
${KC} -n forgejo delete networkpolicy ${NAME}-deny-ingress --ignore-not-found
${KC} -n forgejo delete configmap ${NAME}-config --ignore-not-found
${KC} -n forgejo delete secret ${NAME}-registration --ignore-not-found
if ${WIPE}; then
    ${KC} -n forgejo delete pvc ${NAME}-data-pvc --ignore-not-found
else
    echo "Kept PVC ${NAME}-data-pvc."
fi
