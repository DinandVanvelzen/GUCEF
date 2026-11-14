#!/bin/bash

#if [ $# -eq 0 ]
#  then
#    tag='latest'
#  else
#    tag=$1
#fi


set -e

# build the derived jenkins server image
# --pull refreshes the base image: plugins resolve to their latest releases, which require a recent Jenkins core
docker build --pull -f jenkins_server_build_image.dockerfile . -t vanvelzen/jenkins-server-base:latest
docker run --rm --entrypoint java vanvelzen/jenkins-server-base:latest -jar /usr/share/jenkins/jenkins.war --version

# save the image as a local tar file
docker save --output ./vanvelzen-jenkins-server-base-latest.tar vanvelzen/jenkins-server-base:latest

# Import the image into k3s
sudo k3s ctr images import ./vanvelzen-jenkins-server-base-latest.tar -t vanvelzen/jenkins-server-base:latest

