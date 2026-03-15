#!/bin/bash

helm repo add github-actions-runner-controller https://actions-runner-controller.github.io/actions-runner-controller --kubeconfig /etc/rancher/k3s/k3s.yaml
helm repo update --kubeconfig /etc/rancher/k3s/k3s.yaml

# install the pipe to GitHub
./github_access_runner_install.sh

# create the github runner image which is a derived image
./create_github_runner_image.sh

# install the github runner using the image created above
./github_runner_install.sh


