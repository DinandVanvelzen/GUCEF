#!/bin/bash

# create the namespace within which we will place all kafka pods
sudo env KUBECONFIG=/etc/rancher/k3s/k3s.yaml kubectl create namespace kafka || true

# Allow for logging customization in Kafka
# sudo env KUBECONFIG=/etc/rancher/k3s/k3s.yaml kubectl create configmap kafka-log4j-config -n kafka \
#     --from-file=./log4j.properties || true 

# Add the Bitnami Legacy repo (No account required - Broadcom changed their chart hosting to use a paywall)
sudo helm --kubeconfig /etc/rancher/k3s/k3s.yaml repo add bitnami-legacy https://charts.bitnami.com/bitnami
sudo helm --kubeconfig /etc/rancher/k3s/k3s.yaml repo update

# Install using your existing values.yaml
sudo helm --kubeconfig /etc/rancher/k3s/k3s.yaml upgrade --install kafka bitnami-legacy/kafka \
  --namespace kafka \
  --create-namespace \
  -f values.yaml

# print the current pods
sudo env KUBECONFIG=/etc/rancher/k3s/k3s.yaml kubectl get pods -n kafka

