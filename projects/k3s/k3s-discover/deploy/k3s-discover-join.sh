#!/bin/sh
# Sample join script invoked by the DNS-SD listener when a candidate node is discovered.
# Usage: k3s-discover-join.sh <ip> <hostname> <arch>

IP="$1"
HOSTNAME="$2"
ARCH="$3"

echo "Discovered candidate: ip=${IP} hostname=${HOSTNAME} arch=${ARCH}"

# Path to the k3s node token on the server (master)
TOKEN_FILE="/var/lib/rancher/k3s/server/node-token"

if [ ! -f "${TOKEN_FILE}" ]; then
  echo "Token file ${TOKEN_FILE} not found. Cannot auto-join. Exiting." >&2
  exit 1
fi

TOKEN=$(cat "${TOKEN_FILE}")

echo "Attempting to join node ${IP} to cluster using token from ${TOKEN_FILE}."

# Ensure we have passwordless SSH access to the node or provide credentials via ssh-agent
# This SSH command will run the official k3s installer on the remote node and pass K3S_URL and K3S_TOKEN
# NOTE: Adjust SSH options as needed for your environment (user, identity file, strict host key checking).

SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
REMOTE_CMD="curl -sfL https://get.k3s.io | K3S_URL=\"https://$(hostname -I | awk '{print $1}'):6443\" K3S_TOKEN=\"${TOKEN}\" sh -s - --node-ip ${IP}"

echo "Running: ssh ${SSH_OPTS} root@${IP} '${REMOTE_CMD}'"
ssh ${SSH_OPTS} root@${IP} "${REMOTE_CMD}"

if [ $? -eq 0 ]; then
  echo "Join command executed successfully for ${IP}"
else
  echo "Join command failed for ${IP}" >&2
fi

exit 0
