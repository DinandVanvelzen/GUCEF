
# Delete the current deployment
sudo kubectl delete deployment local-path-provisioner -n kube-system

# Reapply the local-path-storage.yaml
sudo kubectl apply -f https://raw.githubusercontent.com/rancher/local-path-provisioner/v0.0.29/deploy/local-path-storage.yaml

# Remove the default annotation again if it reapplies it
sudo kubectl patch sc local-path -p '{"metadata":{"annotations":{"storageclass.kubernetes.io/is-default-class":null}}}'
