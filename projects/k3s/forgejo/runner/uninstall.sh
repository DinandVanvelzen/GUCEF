#!/bin/bash
# Removes the Forgejo runner. --wipe-data also deletes its volume (job image cache).
KC="kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml"
${KC} -n forgejo delete deployment forgejo-runner --ignore-not-found
${KC} -n forgejo delete networkpolicy forgejo-runner-deny-ingress --ignore-not-found
${KC} -n forgejo delete configmap forgejo-runner-config --ignore-not-found
${KC} -n forgejo delete secret forgejo-runner-registration --ignore-not-found
if [ "$1" = "--wipe-data" ]; then
    ${KC} -n forgejo delete pvc forgejo-runner-data-pvc --ignore-not-found
else
    echo "Kept PVC forgejo-runner-data-pvc."
fi
