#!/bin/bash

# print the current services
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n mysql

# create a namespace to host mysql resources.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace mysql

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f mysql-pvc.yaml -n mysql
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f mysql-statefulset.yaml -n mysql
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f mysql-service.yaml -n mysql

# print the list of services again after the above
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n mysql
