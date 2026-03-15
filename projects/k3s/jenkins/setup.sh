#!/bin/bash

# print the current list of resources
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pvc -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pods -n devops-tools

# create the jenkins server image we wil use for our pod
# this is imported to the k3s installation and made available as a known image
./create_jenkins_server_image.sh
#rm -f ./vanvelzen-jenkins-server-base-latest.tar

# create a namespace to host Jenkins resources.
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create namespace devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml create configmap jenkins-casc --from-file=./jenkins_config.yaml -n devops-tools

kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f jenkins-volume.yaml -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f jenkins-serviceAccount.yaml -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f jenkins-service.yaml -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f jenkins-deployment.yaml -n devops-tools

# extract the config from the pod so we can check it
#rm -f ./jenkins_config_from_pod.yaml
#./extract_jenkins_config.sh

# extract the admin password from the pod so we can check it
# note: when setting a admin password via config as we are this should not be needed and no such file would be generated
#rm -f ./initialAdminPassword
#./extract_initial_admin_password.sh

# print the list of resources again after the above
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get services -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pvc -n devops-tools
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get pods -n devops-tools

# First make the Jenkins CLI client available to our scripts
# This works with out custom image generated above which has the client jar staged inside the image
./copy_jenkins_client_cli_from_pod.sh

# Now generate the auto CI/CD user account
# We dont want to use the admin account for everything
./generate_cicd_user.sh
