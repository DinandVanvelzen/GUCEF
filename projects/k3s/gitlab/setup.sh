#!/bin/bash

# create a namespace to host gitlabresources.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace gitlab

# add helm repo needed to get GitLab helm chart
helm repo add gitlab http://charts.gitlab.io/ --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

helm uninstall my-gitlab --namespace gitlab --kubeconfig /etc/rancher/k3s/k3s.yaml --debug
helm install my-gitlab gitlab/gitlab --version 5.10.2 --namespace gitlab --create-namespace --kubeconfig /etc/rancher/k3s/k3s.yaml --debug --values=./values.yaml
# --version 38.1.1

# Verify installation
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml --namespace gitlab get all
