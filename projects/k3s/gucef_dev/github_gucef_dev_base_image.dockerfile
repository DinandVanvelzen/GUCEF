# syntax=docker/dockerfile:1

# --------------------------------------------------------------------
# First lets use the 3rd party base image which deals with all the agent setup
# That alone however does not get us to the point of building our software
# we want the runner out-of-the-box to come with the needed build tools and dependencies
# --------------------------------------------------------------------

# install build tooling in base image
FROM bitnami/minideb
RUN install_packages apt-utils build-essential git-all clang clang-tools clang-tidy llvm cppcheck libssl-dev rpm libx11-dev graphviz doxygen cmake premake4 ninja-build ssh gdb gdbserver openssh-server netcat-traditional supervisor vim grep rsync gh

# Expose the port for gdbserver
EXPOSE 4020

# Add the script which will init the repo as needed on startup
COPY ./repo_setup.sh /repo_setup.sh
RUN chmod +x /repo_setup.sh

# --------------------------------------------------------------------
# For broad compatibility and access we will use a dedicated SSH server inside 
# the container
# --------------------------------------------------------------------

# Switch to root to perform file operations
USER root

RUN mkdir /var/run/sshd

# Set root password for SSH access (change 'admin' to your desired password)
RUN echo 'root:admin' | chpasswd

# Enable root login via SSH and password authentication
# which means allowing the "root" user to log in using a password instead of relying on other authentication methods like SSH keys.
# Ensure the SSH configuration is set correctly
RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
RUN echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config

# Modify PAM configuration to prevent issues with systemd
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

# modifies the Pluggable Authentication Module configuration for the SSH daemon to prevent possible issues with systemd.
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
# exposes port 22 in the container, allowing SSH connections to the SSH server running inside the container.
EXPOSE 22
# Add a script to check if the SSH port is open and report back
COPY check_ssh_port.sh /check_ssh_port.sh
RUN chmod +x /check_ssh_port.sh

# --------------------------------------------------------------------
# We want to be able to test and run multiple applications inside the 
# container since its a dev environment. We use supervisor to do that.
# --------------------------------------------------------------------

# Create supervisord configuration directory
RUN mkdir -p /etc/supervisor/conf.d

# Copy supervisord configuration file
COPY ./supervisord.conf /etc/supervisor/conf.d/supervisord.conf

# --------------------------------------------------------------------
# Add this custom entry point to allow to control startup in a script to
# add more custom actions as needed.
# --------------------------------------------------------------------

# Add a script to start both the SSH server and the agent application 
COPY github_gucef_dev_base_entrypoint.sh /github_gucef_dev_base_entrypoint.sh 
RUN chmod +x /github_gucef_dev_base_entrypoint.sh

# Set the entrypoint to the start script we installed above
ENTRYPOINT ["/github_gucef_dev_base_entrypoint.sh"]
