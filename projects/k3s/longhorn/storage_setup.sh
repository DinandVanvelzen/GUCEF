#!/bin/bash

# Apply Longhorn disk configuration
. ./generate-longhorn-disks-config.sh
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f ./longhorn-disks.yaml -n longhorn-system

# Now that the disks are defined we can define storage classes which will use them
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f storage_class_jbod_disks.yaml -n longhorn-system
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f storage_class_mirror_disks.yaml -n longhorn-system
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml apply -f storage_class_raid_disks.yaml -n longhorn-system

# After setting up the storage you still need to explicitly mark the disks as usable ('schedulable' in longhorn lingo)
. ./mark-disks-schedulable.sh