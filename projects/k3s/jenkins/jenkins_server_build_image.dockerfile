# syntax=docker/dockerfile:1.3-labs

# First create base image
# The LTS line tracks the Jenkins core versions required by current plugin releases
FROM jenkins/jenkins:lts-jdk21 AS jenkins-server-base

# install build tooling in derived base image
FROM jenkins-server-base
# JENKINS_HOME is a persistent volume mounted by the k8s deployment.
# Anything that must be present in JENKINS_HOME is staged in /usr/share/jenkins/ref:
# the stock jenkins.sh entrypoint copies ref into JENKINS_HOME on every start.
# Files with a '.override' suffix in ref replace the existing copy in JENKINS_HOME.
ENV JENKINS_HOME=/var/jenkins_home
ENV JENKINS_WAR=/usr/share/jenkins/jenkins.war
ENV CASC_JENKINS_CONFIG=/var/jenkins_home/casc_configs/jenkins_config.yaml

# Security and global config come from Configuration-as-Code, not the setup wizard
ENV JAVA_OPTS="-Djenkins.install.runSetupWizard=false"

# Plugins baked into this image replace older versions persisted in JENKINS_HOME
ENV PLUGINS_FORCE_UPGRADE=true

# Stage the jenkins client cli, used via kubectl cp / exec
COPY ["extract_jenkins_client_cli.sh", "/usr/local/bin/"]
RUN bash /usr/local/bin/extract_jenkins_client_cli.sh

# Pre-install plugins into /usr/share/jenkins/ref/plugins
COPY ["jenkins_preinstalled_plugins.txt", "/usr/share/jenkins/plugins.txt"]
RUN jenkins-plugin-cli --plugin-file /usr/share/jenkins/plugins.txt
