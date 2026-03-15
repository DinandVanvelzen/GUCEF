#!/bin/bash

# Get the hostname of the current machine
NODE_HOSTNAME=$(hostname | tr '[:upper:]' '[:lower:]')  # Convert to lowercase
echo "Hostname of current machine: $NODE_HOSTNAME"

# Manually setting the JSONPath query to avoid issues with variable expansion
LABEL_KEY="kubernetes.io/hostname"

# Get the Kubernetes node name by matching the hostname label
echo "Attempting to match node by label $LABEL_KEY..."
KS_NODE_NAME=$(kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o jsonpath="{.items[?(@.metadata.labels['$LABEL_KEY']=='$NODE_HOSTNAME')].metadata.name}")

# Debugging: Show the result of the JSONPath query
echo "Result from JSONPath query: '$KS_NODE_NAME'"

# If no match, try directly matching the node name with the hostname
if [[ -z "$KS_NODE_NAME" ]]; then
    echo "No match found using label '$LABEL_KEY'. Trying to match node name with hostname..."
    KS_NODE_NAME=$(kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o jsonpath="{.items[?(@.metadata.name=='$NODE_HOSTNAME')].metadata.name}")
    echo "Result from direct match: '$KS_NODE_NAME'"
fi

# If still no match, try matching based on InternalIP
if [[ -z "$KS_NODE_NAME" ]]; then
    echo "No match found using node name. Trying to match based on InternalIP..."
    NODE_IP=$(hostname -I | awk '{print $1}')
    echo "Internal IP of current machine: $NODE_IP"
    
    # Fix the JSONPath for InternalIP match
    KS_NODE_NAME=$(kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o jsonpath="{.items[?(@.status.addresses[?(@.type=='InternalIP')].address=='$NODE_IP')].metadata.name}")

    # Debugging: Show the result of the InternalIP query
    echo "Result from InternalIP match: '$KS_NODE_NAME'"
fi

# Check if we got the node name
if [[ -z "$KS_NODE_NAME" ]]; then
    echo "Error: Could not find the Kubernetes node corresponding to this machine."
    exit 1
else
    echo "Node Name found: $KS_NODE_NAME"
    echo "Applying label KS_NODE_NAME=$KS_NODE_NAME to node $KS_NODE_NAME"
    kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml label node "$KS_NODE_NAME" KS_NODE_NAME="$KS_NODE_NAME" --overwrite
    export KS_NODE_NAME
    exit 0
fi
