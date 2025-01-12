#!/bin/bash

#install prereq
sudo kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/prerequisite/longhorn-iscsi-installation.yaml
sudo apt-get update;sudo apt-get install -y open-iscsi cryptsetup dmsetup
sudo kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/prerequisite/longhorn-nfs-installation.yaml

# Install longhorn
sudo kubectl apply -f https://raw.githubusercontent.com/longhorn/longhorn/v1.7.2/deploy/longhorn.yaml

# Now set up the local storage in Longhorn
. ./storage_setup.sh
