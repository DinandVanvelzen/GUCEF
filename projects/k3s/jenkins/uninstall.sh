#!/bin/bash

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get deployments -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete deployments/jenkins -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get deployments -n devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete services/jenkins -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pods -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete all -n devops-tools --all
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pods -n devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pvc -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete pvc jenkins-pv-claim -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pvc -n devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get configmaps -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete configmap jenkins-casc -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get configmaps -n devops-tools

# We use the dynamic provisioner now so the below should not be needed
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pv
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml delete pv jenkins-pv
#kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pv

sudo docker image remove jenkins/jenkins --force
sudo docker image remove jenkins/jenkins:lts --force
sudo docker image remove vanvelzen/jenkins-server-base --force
