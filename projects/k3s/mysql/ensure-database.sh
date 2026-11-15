#!/bin/bash
# ---------------------------------------------------------------------------
# Creates a database and a user with all rights on it, when missing, and sets
# the user's password. Idempotent; for use by other components' setup scripts.
#
# Usage: DB_PASSWORD=<password> ./ensure-database.sh <database> <user>
# ---------------------------------------------------------------------------

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="mysql"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

DB_NAME="$1"
DB_USER="$2"
if [ -z "${DB_NAME}" ] || [ -z "${DB_USER}" ] || [ -z "${DB_PASSWORD}" ]; then
    echo "Usage: DB_PASSWORD=<password> $0 <database> <user>" >&2
    exit 1
fi
if ! [[ "${DB_NAME}" =~ ^[A-Za-z0-9_]+$ ]] || ! [[ "${DB_USER}" =~ ^[A-Za-z0-9_]+$ ]]; then
    echo "ERROR: database and user names may contain only letters, digits and underscores." >&2
    exit 1
fi

${KC} -n ${NAMESPACE} get statefulset mysql >/dev/null 2>&1 || {
    echo "ERROR: MySQL is not installed. Run ../mysql/setup.sh first." >&2
    exit 1
}
ROOT_PASS=$(${KC} -n ${NAMESPACE} get secret mysql-credentials -o jsonpath='{.data.MYSQL_ROOT_PASSWORD}' | base64 -d)
[ -n "${ROOT_PASS}" ] || { echo "ERROR: could not read the MySQL root password." >&2; exit 1; }

# Single quotes in the password are doubled for the SQL string literal
ESCAPED_PASS="${DB_PASSWORD//\'/\'\'}"

${KC} -n ${NAMESPACE} exec -i mysql-0 -c mysql -- env MYSQL_PWD="${ROOT_PASS}" mysql -uroot -h 127.0.0.1 <<EOT || exit 1
CREATE DATABASE IF NOT EXISTS \`${DB_NAME}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS '${DB_USER}'@'%' IDENTIFIED BY '${ESCAPED_PASS}';
ALTER USER '${DB_USER}'@'%' IDENTIFIED BY '${ESCAPED_PASS}';
GRANT ALL PRIVILEGES ON \`${DB_NAME}\`.* TO '${DB_USER}'@'%';
FLUSH PRIVILEGES;
EOT
echo "MySQL database ${DB_NAME} and user ${DB_USER} are in place."
