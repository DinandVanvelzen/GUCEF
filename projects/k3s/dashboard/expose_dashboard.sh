#!/bin/bash

# Define the namespace and service name
NAMESPACE="kubernetes-dashboard"
SERVICE_NAME="kubernetes-dashboard-web"

# Change the service type to LoadBalancer
echo "Changing service type to LoadBalancer..."
kubectl patch svc $SERVICE_NAME -n $NAMESPACE -p '{"spec": {"type": "LoadBalancer"}}'

# Wait for the service to be updated
echo "Waiting for the service to be updated..."
sleep 5

# Get the external IP of the service
EXTERNAL_IP=""
while [ -z "$EXTERNAL_IP" ]; do
    echo "Retrieving the external IP..."
    EXTERNAL_IP=$(kubectl get svc $SERVICE_NAME -n $NAMESPACE -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
    if [ -z "$EXTERNAL_IP" ]; then
        echo "External IP is still pending. Retrying..."
        sleep 5
    fi
done

# Print the external IP
echo "Dashboard is accessible at: http://$EXTERNAL_IP"

# Optionally: Specify a specific port if needed
PORT=40080  # Change this to your desired port number

# Update the service to include the specific port configuration
kubectl patch svc $SERVICE_NAME -n $NAMESPACE -p "{\"spec\": {\"ports\": [{\"name\": \"http\", \"port\": $PORT, \"targetPort\": 443}]}}"
echo "Dashboard exposed on port: $PORT"
