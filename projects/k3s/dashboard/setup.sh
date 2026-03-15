#!/bin/bash

# first delete the dashboard if one already exists
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete ns kubernetes-dashboard
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete clusterrolebinding --ignore-not-found=true kubernetes-dashboard
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete clusterrole --ignore-not-found=true kubernetes-dashboard

GITHUB_URL=https://github.com/kubernetes/dashboard/releases
VERSION_KUBE_DASHBOARD=$(curl -w '%{url_effective}' -I -L -s -S ${GITHUB_URL}/latest -o /dev/null | sed -e 's|.*/||')
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create -n kube-system -f https://raw.githubusercontent.com/kubernetes/dashboard/${VERSION_KUBE_DASHBOARD}/aio/deploy/recommended.yaml

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply --namespace=kubernetes-dashboard -f https://raw.githubusercontent.com/kubernetes/dashboard/v3.0.0-alpha0/charts/kubernetes-dashboard.yaml

# Deploy the admin-user configuration:
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create --ignore-not-found=true -n kubernetes-dashboard -f ./dashboard.admin-user.yml -f ./dashboard.admin-user-role.yml

# Obtain the Bearer Token
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml -n kubernetes-dashboard describe secret admin-user-token | grep '^token'

# Port forward the dashboard such that its available from the node localhost
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml port-forward --namespace=kubernetes-dashboard service/kubernetes-dashboard-web 8000:8000 --address 0.0.0.0

# To access the Dashboard you must create a secure channel to your K3s cluster:
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml proxy

# The Dashboard is now accessible at:
#  http://localhost:8001/api/v1/namespaces/kubernetes-dashboard/services/https:kubernetes-dashboard:/proxy/
#  Sign In with the admin-user Bearer Token
