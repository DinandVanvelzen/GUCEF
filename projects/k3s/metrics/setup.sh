#!/bin/bash

# Installs or updates the metrics stack in namespace "metrics".
# Re-runnable: existing resources and their data are kept and updated in place.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}" || exit 1

# create a namespace to host metrics resources, if it does not exist yet
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace metrics --dry-run=client -o yaml \
    | kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f -

# install Graphite
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f ./graphite-deployment.yml -n metrics

# create the statsite image from source and make it available to use
#./create_statsite_image.sh

# install statsite
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f ./statsite-deployment.yml -n metrics

# install Grafana
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f ./grafana-deployment.yml -n metrics

# Now, we will run Snap Daemon in our Kubernetes cluster.
# Snap Daemon is the monitoring daemon which will pull various kubernetes monitoring metrics and push them into Graphite.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f ./snap_ds.yml -n metrics
