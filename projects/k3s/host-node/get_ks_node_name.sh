#!/bin/bash

# Check if KS_NODE_NAME is set in the environment
if [[ -n "$KS_NODE_NAME" ]]; then
    echo "Using existing KS_NODE_NAME: $KS_NODE_NAME"
else
    echo "KS_NODE_NAME not found in environment. Attempting to retrieve from Kubernetes node labels..."
    
    # Get the hostname of the current machine
    NODE_HOSTNAME=$(hostname | tr '[:upper:]' '[:lower:]')  # Convert to lowercase
    
    # Try to fetch KS_NODE_NAME from the node labels
    KS_NODE_NAME=$(kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml get nodes -o jsonpath="{.items[?(@.metadata.labels['KS_NODE_NAME'])].metadata.labels['KS_NODE_NAME']}")

    # Check if we got the KS_NODE_NAME from the labels
    if [[ -z "$KS_NODE_NAME" ]]; then
        echo "Error: Could not retrieve KS_NODE_NAME from node labels."
        exit 1
    else
        echo "Retrieved KS_NODE_NAME from node labels: $KS_NODE_NAME"
    fi
fi

# Now you can use the KS_NODE_NAME variable in your script
# Example usage
echo "Final KS_NODE_NAME: $KS_NODE_NAME"
export KS_NODE_NAME