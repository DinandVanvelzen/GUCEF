#!/bin/bash

# we need the jfrog repo to get the goodies
helm repo add jfrog https://charts.jfrog.io/ --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

# uninstall if we already had it installed
helm uninstall artifactory --kubeconfig /etc/rancher/k3s/k3s.yaml && sleep 90 && kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete pvc -l app=artifactory

# delete artifactory which will ALSO DELETE DATA VOLUMES
#helm delete artifactory --namespace artifactory

# proceed with the install
helm install artifactory jfrog/artifactory-oss --namespace artifactory --create-namespace --set installCRDs=true --kubeconfig /etc/rancher/k3s/k3s.yaml --debug

