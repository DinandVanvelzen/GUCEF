#!/bin/bash

helm repo add jetstack https://charts.jetstack.io --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

# install the common prereq of cert manager
helm install cert-manager jetstack/cert-manager --namespace cert-manager --create-namespace --set installCRDs=true --set prometheus.enabled=true --set webhook.timeoutSeconds=4 --kubeconfig /etc/rancher/k3s/k3s.yaml --debug

