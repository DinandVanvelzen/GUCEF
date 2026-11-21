#!/bin/bash
# Removes the IRC server (Deployment, Service, configuration, certificate).
# Kept unless --wipe-data is given: the PVC ergo-data-pvc (accounts, channel registrations),
# the PostgreSQL database ergo (chat history) and the Secret irc-credentials.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib.sh"
WIPE_DATA=false

for arg in "$@"; do
    case "${arg}" in
        --wipe-data) WIPE_DATA=true ;;
        *) echo "Usage: $0 [--wipe-data]"; exit 1 ;;
    esac
done

if [ "${WIPE_DATA}" = true ]; then
    echo "WARNING: --wipe-data deletes all IRC accounts, channel registrations and chat history."
    read -r -p "Type 'wipe' to continue: " CONFIRM
    if [ "${CONFIRM}" != "wipe" ]; then
        echo "Aborted."
        exit 1
    fi
fi

${KC} -n ${NAMESPACE} delete deployment irc-hub --ignore-not-found
${KC} -n ${NAMESPACE} delete configmap irc-hub-config --ignore-not-found
${KC} -n ${NAMESPACE} delete secret irc-hub-secrets --ignore-not-found
${KC} -n ${NAMESPACE} delete deployment ergo --ignore-not-found
${KC} -n ${NAMESPACE} delete service ergo ergo-lan --ignore-not-found
${KC} -n ${NAMESPACE} delete secret ergo-config --ignore-not-found
${KC} -n ${NAMESPACE} delete certificate irc-lan-tls --ignore-not-found
${KC} -n ${NAMESPACE} delete issuer irc-selfsigned --ignore-not-found
${KC} -n ${NAMESPACE} delete secret irc-lan-tls --ignore-not-found

if [ "${WIPE_DATA}" = true ]; then
    ${KC} -n ${NAMESPACE} delete pvc ergo-data-pvc --ignore-not-found
    PGPOD=$(${KC} -n ${PG_NAMESPACE} get pod -o jsonpath='{.items[0].metadata.name}')
    PGPASS=$(${KC} -n ${PG_NAMESPACE} get secret postgresql-credentials -o jsonpath='{.data.POSTGRES_PASSWORD}' | base64 -d)
    ${KC} -n ${PG_NAMESPACE} exec "${PGPOD}" -c postgres -- env PGPASSWORD="${PGPASS}" psql -U postgres -c "DROP DATABASE IF EXISTS ergo;"
    ${KC} -n ${PG_NAMESPACE} exec "${PGPOD}" -c postgres -- env PGPASSWORD="${PGPASS}" psql -U postgres -c "DROP ROLE IF EXISTS ergo;"
    ${KC} delete namespace ${NAMESPACE} --ignore-not-found
    echo "Local secrets.env and sealed-secret.yaml are kept; delete them to start with new passwords."
else
    echo "Kept PVC ergo-data-pvc, database ergo and Secret irc-credentials."
fi
