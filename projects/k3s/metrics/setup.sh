#!/bin/bash

# First cleanup whatever may have been left behind from a previous round
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete namespace metrics

# create a namespace to host metrics resources.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace metrics

# install Graphite
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create -f ./graphite-deployment.yml -n metrics

# create the statsite image from source and make it available to use
#./create_statsite_image.sh

# install statsite
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create -f ./statsite-deployment.yml -n metrics

# install Grafana
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create -f ./grafana-deployment.yml -n metrics

# Now, we will run Snap Daemon in our Kubernetes cluster.
# Snap Daemon is the monitoring daemon which will pull various kubernetes monitoring metrics and push them into Graphite.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create -f ./snap_ds.yml -n metrics
