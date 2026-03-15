#!/bin/bash

# show what we have running before we begin
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get all -A

# kill all containers/pods
sudo /usr/local/bin/k3s-killall.sh

# get rid of k3s itself
sudo /usr/local/bin/k3s-uninstall.sh

# get a new copy of k3s
# K3S_KUBECONFIG_MODE=644 makes k3s write the kubeconfig world-readable on every start/upgrade
# so kubectl and helm work without sudo using --kubeconfig /etc/rancher/k3s/k3s.yaml
sudo curl -sfL https://get.k3s.io | K3S_KUBECONFIG_MODE="644" sh -

# set the location of the kubeconfig
export KUBECONFIG=/etc/rancher/k3s/k3s.yaml

# check to see if we are back online
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o wide

# install the system upgrade controller
# this will be used for k3s version upgrades instead of a full reinstall
echo "Installing k3s system-upgrade-controller to manage future k3s upgrades"
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f https://github.com/rancher/system-upgrade-controller/releases/latest/download/system-upgrade-controller.yaml
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f https://github.com/rancher/system-upgrade-controller/releases/latest/download/crd.yaml

# download Helm 3
echo "Installing Helm 3 for helm chart support"
sudo curl -fsSL -o get_helm.sh https://raw.githubusercontent.com/helm/helm/master/scripts/get-helm-3
sudo chmod 700 get_helm.sh

# set up helm
sudo ./get_helm.sh
rm -f ./get_helm.sh

# confirm we are dealing with Helm 3
helm version

# set often used repos and update
echo "Configuring Helm repos"
helm repo add bitnami https://charts.bitnami.com/bitnami --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo add jenkins https://charts.jenkins.io --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo add gitlab https://charts.gitlab.io/ --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

# install the local path provisioner, which is a plugin for k3s
# it comes standard with k3s as provisioner "rancher.io/local-path"
# you can update it independently here (since its a newer feature and you might want latest)
# stable version:
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f https://raw.githubusercontent.com/rancher/local-path-provisioner/v0.0.29/deploy/local-path-storage.yaml
# latest version:
#sudo kubectl apply -f https://raw.githubusercontent.com/rancher/local-path-provisioner/master/deploy/local-path-storage.yaml

# Install Sealed Secrets controller for safe secret management in git
# Must be installed before any component that uses sealed secrets (e.g. immich)
cd ./sealed-secrets
./setup.sh
cd ..

# Make the local drives of the node available to k8s
# comment this line or edit the yaml for your own node
cd ./host-node
./setup.sh
cd ..

# Use Longhorn for storage management
# This will switch the default away from the local-storage-provisioner
cd ./longhorn
./setup.sh
cd ..

# install the common prereq of cert manager
cd ./cert-manager
./setup.sh
cd ..

# setup the K9s TUI (Text User Interface)
cd ./k9s
./setup.sh
cd ..

# setup the Kubernetes dashboard
cd ./dashboard
./setup.sh
cd ..

# setup MySQL
cd ./mysql
./setup.sh
cd ..

# setup PostgreSQL
cd ./postgresql
./setup.sh
cd ..

# setup GitHub runner integration
cd ./github
./setup.sh
cd ..

# setup Nexus for storing build artifacts and deployment, CICD
cd ./nexus
./setup.sh
cd ..

# setup Jenkins for CICD
cd ./jenkins
./setup.sh
cd ..

# setup metrics support
cd ./metrics
./setup.sh
cd ..

# setup Immich photo management
# Requires: sealed-secrets (above), postgresql (above)
# Optional: create immich/setup-home.sh with your NAS symlinks before running
cd ./immich
./setup.sh
cd ..

# Make the kubeconfig available without needing sudo for every kubectl/helm call
mkdir -p ~/.kube
sudo cp /etc/rancher/k3s/k3s.yaml ~/.kube/config
sudo chown $USER:$USER ~/.kube/config
chmod 600 ~/.kube/config

