#!/bin/bash

# Set the namespace where Jenkins is running
NAMESPACE="devops-tools"

# Set the file to extract
FILE_PATH="/var/jenkins_home/secrets/initialAdminPassword"

. ./wait_for_jenkins_pod.sh

# Use kubectl to copy the initialAdminPassword file from the Jenkins pod to the local machine
echo "Attempting to copy from: $NAMESPACE/$POD_NAME:$FILE_PATH"
sudo kubectl cp $NAMESPACE/$POD_NAME:$FILE_PATH ./initialAdminPassword

# Check if the file was successfully copied and is non-empty
if [[ -f ./initialAdminPassword && -s ./initialAdminPassword ]]; then
  echo "File successfully copied to ./initialAdminPassword"
else
  echo "Failed to copy the file."
  exit 1
fi
