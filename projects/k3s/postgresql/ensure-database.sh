#!/bin/bash
# ---------------------------------------------------------------------------
# Creates a PostgreSQL role and databases owned by it, when missing, and sets
# the role's password. Idempotent; for use by other components' setup scripts.
#
# Usage: DB_PASSWORD=<password> ./ensure-database.sh <user> <database> [<database> ...]
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="postgresql"
POD="my-postgres-0"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

DB_USER="$1"
shift
if [ -z "${DB_USER}" ] || [ $# -eq 0 ] || [ -z "${DB_PASSWORD}" ]; then
    echo "Usage: DB_PASSWORD=<password> $0 <user> <database> [<database> ...]" >&2
    exit 1
fi
for NAME in "${DB_USER}" "$@"; do
    if ! [[ "${NAME}" =~ ^[a-z][a-z0-9_]*$ ]]; then
        echo "ERROR: user and database names may contain only lower case letters, digits and underscores, starting with a letter." >&2
        exit 1
    fi
done

${KC} -n ${NAMESPACE} get pod ${POD} >/dev/null 2>&1 || {
    echo "ERROR: PostgreSQL is not installed. Run ./setup.sh first." >&2
    exit 1
}
# Single quotes in the password are doubled for the SQL string literal
ESCAPED_PASS="${DB_PASSWORD//\'/\'\'}"

# psql in the pod connects through the local socket, which the server trusts: no superuser password is handled
PSQL="${KC} -n ${NAMESPACE} exec -i ${POD} -c postgres -- psql -v ON_ERROR_STOP=1 -U postgres -d postgres -q"

${PSQL} <<EOT || exit 1
DO \$\$
BEGIN
    IF NOT EXISTS ( SELECT FROM pg_roles WHERE rolname = '${DB_USER}' ) THEN
        CREATE ROLE ${DB_USER} LOGIN;
    END IF;
END
\$\$;
ALTER ROLE ${DB_USER} WITH LOGIN PASSWORD '${ESCAPED_PASS}';
EOT

for DB_NAME in "$@"; do
    EXISTS=$(${PSQL} -tA -c "SELECT 1 FROM pg_database WHERE datname = '${DB_NAME}'") || exit 1
    if [ "${EXISTS}" != "1" ]; then
        ${PSQL} -c "CREATE DATABASE ${DB_NAME} OWNER ${DB_USER} ENCODING 'UTF8' TEMPLATE template0" || exit 1
    fi
    ${PSQL} -c "ALTER DATABASE ${DB_NAME} OWNER TO ${DB_USER}" || exit 1
    echo "PostgreSQL database ${DB_NAME} owned by ${DB_USER} is in place."
done
