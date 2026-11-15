#!/bin/bash
# Removes the MySQL StatefulSet and Service.
# The PVC mysql-pvc (all databases) is kept unless --wipe-data is given.

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="mysql"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"
WIPE_DATA=false

for arg in "$@"; do
    case "${arg}" in
        --wipe-data) WIPE_DATA=true ;;
        *) echo "Usage: $0 [--wipe-data]"; exit 1 ;;
    esac
done

if [ "${WIPE_DATA}" = true ]; then
    echo "WARNING: --wipe-data deletes the PVC mysql-pvc. All MySQL databases are lost."
    read -r -p "Type 'wipe' to continue: " CONFIRM
    if [ "${CONFIRM}" != "wipe" ]; then
        echo "Aborted."
        exit 1
    fi
fi

${KC} -n ${NAMESPACE} delete statefulset mysql --ignore-not-found
${KC} -n ${NAMESPACE} delete service mysql --ignore-not-found

if [ "${WIPE_DATA}" = true ]; then
    ${KC} -n ${NAMESPACE} delete pvc mysql-pvc --ignore-not-found
else
    echo "Kept PVC mysql-pvc."
fi
