#!/bin/bash

# create the namespace within which we will place all kafka pods
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace kafka || true

# Allow for logging customization in Kafka
# kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create configmap kafka-log4j-config -n kafka \
#     --from-file=./log4j.properties || true

# Add the Bitnami Legacy repo (No account required - Broadcom changed their chart hosting to use a paywall)
helm --kubeconfig /etc/rancher/k3s/k3s.yaml repo add bitnami-legacy https://charts.bitnami.com/bitnami
helm --kubeconfig /etc/rancher/k3s/k3s.yaml repo update

# Install using your existing values.yaml
helm --kubeconfig /etc/rancher/k3s/k3s.yaml upgrade --install kafka bitnami-legacy/kafka \
  --namespace kafka \
  --create-namespace \
  -f values.yaml

# print the current pods
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pods -n kafka
