#!/bin/bash
# ---------------------------------------------------------------------------
# Installs the IRC server (Ergo) in namespace irc. Idempotent.
#
# - Chat history in PostgreSQL (database and role "ergo" in my-postgres).
# - Accounts and channel registrations on PVC ergo-data-pvc.
# - IRC over TLS on IRC_LAN_PORT (default 32004), self-signed certificate from cert-manager.
# - Every client logs in with SASL; accounts are created by this script only.
# - Channels in IRC_CHANNELS are registered and invite-only; every account may join them.
#
# Options:
#   --new-password <account>     give the account a new generated password (three words and a
#                                number); printed at the end
#   --reset-password <account>   set the account's password to the value in secrets.env
#                                (IRC_PASSWORD_<account>), e.g. after editing it there
#   Both may be given more than once.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib.sh"

RESET_PASSWORDS=""
NEW_PASSWORDS=""
while [ $# -gt 0 ]; do
    case "$1" in
        --new-password)
            [ -n "$2" ] || irc_fail "--new-password needs an account name."
            NEW_PASSWORDS="${NEW_PASSWORDS} $2"
            RESET_PASSWORDS="${RESET_PASSWORDS} $2"
            shift 2
            ;;
        --reset-password)
            [ -n "$2" ] || irc_fail "--reset-password needs an account name."
            RESET_PASSWORDS="${RESET_PASSWORDS} $2"
            shift 2
            ;;
        *) echo "Usage: $0 [--new-password <account>]... [--reset-password <account>]..."; exit 1 ;;
    esac
done

irc_load_config
irc_preflight
irc_detect_storage_class
irc_detect_host
for ACCT in ${RESET_PASSWORDS}; do
    [[ " ${IRC_ALL_ACCOUNTS} " == *" ${ACCT} "* ]] || irc_fail "${ACCT} is not in IRC_ADMIN_ACCOUNT or IRC_ACCOUNTS."
done
echo "Storage class: ${IRC_STORAGE_CLASS}"
echo "Node:          ${IRC_NODE} (${IRC_NODE_IP})"
echo "Host:          ${IRC_HOST}$([ "${IRC_HOST_AUTODETECTED}" = "true" ] && echo ' (auto-detected; set IRC_HOST in config.env to use a DNS name)')"
echo "Accounts:      ${IRC_ALL_ACCOUNTS}(administrator: ${IRC_ADMIN_ACCOUNT})"
echo "Channels:      ${IRC_CHANNELS}"

# ---------------------------------------------------------------------------
# Namespace and credentials
# ---------------------------------------------------------------------------
${KC} create namespace ${NAMESPACE} --dry-run=client -o yaml | ${KC} apply -f -

# --new-password: drop the old value; create-sealed-secret.sh generates a new one
for ACCT in ${NEW_PASSWORDS}; do
    [ -f "${SCRIPT_DIR}/secrets.env" ] && sed -i "/^IRC_PASSWORD_${ACCT}=/d" "${SCRIPT_DIR}/secrets.env"
done
bash "${SCRIPT_DIR}/create-sealed-secret.sh" || irc_fail "Failed to create the sealed secret."
${KC} apply -f "${SCRIPT_DIR}/sealed-secret.yaml" -n ${NAMESPACE}

# The sealed-secrets controller needs a moment to produce or update the Secret
EXPECTED_TOKEN=$(irc_local_secret IRC_API_TOKEN)
echo "Waiting for secret irc-credentials..."
EXPECTED_BOT_OPER_HASH=$(irc_local_secret IRC_BOT_OPER_HASH)
for _ in $(seq 1 30); do
    [ "$(irc_secret API_TOKEN)" = "${EXPECTED_TOKEN}" ] && [ "$(irc_secret BOT_OPER_HASH)" = "${EXPECTED_BOT_OPER_HASH}" ] && break
    sleep 2
done
[ "$(irc_secret API_TOKEN)" = "${EXPECTED_TOKEN}" ] && [ "$(irc_secret BOT_OPER_HASH)" = "${EXPECTED_BOT_OPER_HASH}" ] || irc_fail "Secret irc-credentials was not created or does not match secrets.env." \
    "After a cluster rebuild, delete sealed-secret.yaml and run ./setup.sh again."

IRC_DB_PASSWORD=$(irc_secret DB_PASSWORD)
IRC_API_TOKEN=$(irc_secret API_TOKEN)
STEWARD_PASSWORD=$(irc_secret STEWARD_PASSWORD)
ADMIN_OPER_HASH=$(irc_secret ADMIN_OPER_HASH)
STEWARD_OPER_HASH=$(irc_secret STEWARD_OPER_HASH)
BOT_OPER_HASH=$(irc_secret BOT_OPER_HASH)

# ---------------------------------------------------------------------------
# Bots: a bot runs when gucefIrcHub is enabled and its model can be reached:
# llama-server in namespace llm, or a vendor whose API key is in secrets.env
# ---------------------------------------------------------------------------
ACTIVE_BOTS=""
INACTIVE_BOTS=""
if [ "${IRC_HUB}" = "enabled" ]; then
    for BOT in ${IRC_BOTS}; do
        KEY=$(irc_bot_setting "${BOT}" API_KEY)
        if [ -n "${KEY}" ]; then
            if [ -n "$(irc_local_secret "${KEY}")" ]; then
                ACTIVE_BOTS="${ACTIVE_BOTS}${BOT} "
            else
                INACTIVE_BOTS="${INACTIVE_BOTS}${BOT} (put the API key in ${KEY} in secrets.env)"$'\n'
            fi
        elif [[ "$(irc_bot_setting "${BOT}" BASE_URL)" == *".${LLM_NAMESPACE}.svc"* ]] && ! ${KC} -n ${LLM_NAMESPACE} get svc llama-server >/dev/null 2>&1; then
            INACTIVE_BOTS="${INACTIVE_BOTS}${BOT} (the local LLM server is not installed: ../llm/setup.sh)"$'\n'
        else
            ACTIVE_BOTS="${ACTIVE_BOTS}${BOT} "
        fi
    done
fi
echo "Bots:          ${ACTIVE_BOTS:-none}"

# The channels: IRC_CHANNELS, the rooms of the bots and the channels they are in
ALL_CHANNELS="${IRC_CHANNELS}"
for BOT in ${ACTIVE_BOTS}; do
    ALL_CHANNELS="${ALL_CHANNELS} $(irc_bot_setting "${BOT}" ROOM) $(irc_bot_setting "${BOT}" CHANNELS)"
done
ALL_CHANNELS=$(echo ${ALL_CHANNELS} | tr ' ' '\n' | awk 'NF && !seen[tolower($0)]++' | tr '\n' ' ')

# The bots that are members of a channel: in their room and in their shared channels
bots_in_channel()
{
    local chan="${1,,}" bot c
    for bot in ${ACTIVE_BOTS}; do
        for c in $(irc_bot_setting "${bot}" ROOM) $(irc_bot_setting "${bot}" CHANNELS); do
            [ "${c,,}" = "${chan}" ] && { echo -n "${bot} "; break; }
        done
    done
}

# ---------------------------------------------------------------------------
# Database and role in my-postgres (chat history)
# ---------------------------------------------------------------------------
PG_SUPER_PASS=$(${KC} -n ${PG_NAMESPACE} get secret postgresql-credentials -o jsonpath='{.data.POSTGRES_PASSWORD}' | base64 -d)
[ -n "${PG_SUPER_PASS}" ] || irc_fail "Could not read the PostgreSQL superuser password."

psql_exec()
{
    ${KC} -n ${PG_NAMESPACE} exec -i ${PG_POD} -c postgres -- env PGPASSWORD="${PG_SUPER_PASS}" psql -U postgres -v ON_ERROR_STOP=1 -tA "$@"
}

if [ "$(psql_exec -c "SELECT 1 FROM pg_roles WHERE rolname='ergo'")" != "1" ]; then
    echo "Creating database role ergo..."
    # Password passed through stdin as a psql variable to avoid quoting problems
    printf "CREATE ROLE ergo LOGIN PASSWORD :'pw';\n" | psql_exec -v "pw=${IRC_DB_PASSWORD}" || exit 1
else
    printf "ALTER ROLE ergo PASSWORD :'pw';\n" | psql_exec -v "pw=${IRC_DB_PASSWORD}" >/dev/null || exit 1
fi
if [ "$(psql_exec -c "SELECT 1 FROM pg_database WHERE datname='ergo'")" != "1" ]; then
    echo "Creating database ergo..."
    psql_exec -c "CREATE DATABASE ergo OWNER ergo ENCODING 'UTF8' TEMPLATE template0" || exit 1
fi

# ---------------------------------------------------------------------------
# Volume for accounts and channel registrations
# ---------------------------------------------------------------------------
if ${KC} -n ${NAMESPACE} get pvc ergo-data-pvc >/dev/null 2>&1; then
    echo "PVC ergo-data-pvc exists; keeping it."
else
    ${KC} apply -f - <<EOT
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ergo-data-pvc
  namespace: ${NAMESPACE}
spec:
  accessModes:
    - ReadWriteOnce
  storageClassName: ${IRC_STORAGE_CLASS}
  resources:
    requests:
      storage: ${IRC_PVC_SIZE}
EOT
fi

# ---------------------------------------------------------------------------
# TLS certificate for the LAN listener (self-signed, cert-manager)
# ---------------------------------------------------------------------------
CERT_DNS_NAMES=""
CERT_IPS=""
for NAME in ${IRC_HOST} ${IRC_NODE} ${IRC_NODE_IP} localhost; do
    if [[ "${NAME}" =~ ^[0-9.]+$ ]]; then
        [[ " ${CERT_IPS} " == *" ${NAME} "* ]] || CERT_IPS="${CERT_IPS} ${NAME}"
    else
        [[ " ${CERT_DNS_NAMES} " == *" ${NAME} "* ]] || CERT_DNS_NAMES="${CERT_DNS_NAMES} ${NAME}"
    fi
done
{
    cat <<EOT
apiVersion: cert-manager.io/v1
kind: Issuer
metadata:
  name: irc-selfsigned
  namespace: ${NAMESPACE}
spec:
  selfSigned: {}
---
apiVersion: cert-manager.io/v1
kind: Certificate
metadata:
  name: irc-lan-tls
  namespace: ${NAMESPACE}
spec:
  secretName: irc-lan-tls
  commonName: ${IRC_HOST}
  # long-lived: phones that accepted the certificate are not asked again for years
  duration: 87600h
  renewBefore: 720h
  privateKey:
    algorithm: ECDSA
    size: 256
  issuerRef:
    name: irc-selfsigned
    kind: Issuer
EOT
    if [ -n "${CERT_DNS_NAMES}" ]; then
        echo "  dnsNames:"
        for N in ${CERT_DNS_NAMES}; do echo "    - ${N}"; done
    fi
    if [ -n "${CERT_IPS}" ]; then
        echo "  ipAddresses:"
        for N in ${CERT_IPS}; do echo "    - ${N}"; done
    fi
} | ${KC} apply -f -
${KC} -n ${NAMESPACE} wait --for=condition=Ready certificate/irc-lan-tls --timeout=120s \
    || irc_fail "Certificate irc-lan-tls is not ready." "Check: ${KC} -n ${NAMESPACE} describe certificate irc-lan-tls"

# ---------------------------------------------------------------------------
# Ergo configuration (Secret ergo-config: it holds the database password and the API token)
# ---------------------------------------------------------------------------
RENDER_DIR=$(mktemp -d)
trap 'rm -rf "${RENDER_DIR}"' EXIT
chmod 700 "${RENDER_DIR}"
irc_render "${SCRIPT_DIR}/ergo/ircd.yaml.tmpl" IRC_NETWORK_NAME IRC_SERVER_NAME IRC_PG_HOST IRC_DB_PASSWORD \
    IRC_API_TOKEN ADMIN_OPER_HASH STEWARD_OPER_HASH BOT_OPER_HASH IRC_HISTORY_EXPIRE IRC_POD_CIDR > "${RENDER_DIR}/ircd.yaml"
cp "${SCRIPT_DIR}/ergo/ergo.motd" "${RENDER_DIR}/ergo.motd"
${KC} -n ${NAMESPACE} create secret generic ergo-config \
    --from-file=ircd.yaml="${RENDER_DIR}/ircd.yaml" \
    --from-file=ergo.motd="${RENDER_DIR}/ergo.motd" \
    --dry-run=client -o yaml | ${KC} apply -f -

# ---------------------------------------------------------------------------
# Server
# ---------------------------------------------------------------------------
irc_render "${SCRIPT_DIR}/ergo/deployment.yaml.tmpl" IRC_ERGO_IMAGE IRC_RELOADER_IMAGE IRC_LAN_PORT IRC_NODE | ${KC} apply -f -
# A LoadBalancer service would hold the same port on the nodes and hide the client addresses
${KC} -n ${NAMESPACE} delete service ergo-lan --ignore-not-found
${KC} -n ${NAMESPACE} rollout status deployment/ergo --timeout=10m \
    || irc_fail "Ergo did not become ready." "Check: ${KC} -n ${NAMESPACE} logs deploy/ergo -c ergo"

# ---------------------------------------------------------------------------
# Provisioning through port-forwards to the pod's loopback listeners
# ---------------------------------------------------------------------------
LOCAL_API_PORT=28089
LOCAL_IRC_PORT=26667
${KC} -n ${NAMESPACE} port-forward deploy/ergo ${LOCAL_API_PORT}:8089 ${LOCAL_IRC_PORT}:6667 >/dev/null 2>&1 &
PF_PID=$!
trap 'kill ${PF_PID} 2>/dev/null; rm -rf "${RENDER_DIR}"' EXIT

ergo_api()
{
    curl -s -m 10 -H "Authorization: Bearer ${IRC_API_TOKEN}" -d "$2" "http://127.0.0.1:${LOCAL_API_PORT}/v1/$1"
}
for _ in $(seq 1 30); do
    ergo_api status '' | grep -q '"success":true' && break
    sleep 1
done
ergo_api status '' | grep -q '"success":true' || irc_fail "The Ergo API does not answer through the port-forward."

PROVISION_FAIL=0
# "account password" lines of the accounts created or given a password in this run
NEW_LOGINS=""

# Creates ACCOUNT with PASSWORD when it does not exist; with "reset" sets the password of an existing one.
# The logins of people are listed in the summary; with "quiet" as the fourth argument not.
ensure_account()
{
    local acct="$1" pass="$2" mode="$3" quiet="$4" out
    out=$(ergo_api ns/info "{\"accountName\":\"${acct}\"}")
    if echo "${out}" | grep -q '"success":true'; then
        if [ "${mode}" = "reset" ]; then
            out=$(ergo_api ns/passwd "{\"accountName\":\"${acct}\",\"passphrase\":\"${pass}\"}")
            if echo "${out}" | grep -q '"success":true'; then
                echo "Account ${acct}: password set."
                [ -n "${quiet}" ] || NEW_LOGINS="${NEW_LOGINS}${acct} ${pass}"$'\n'
            else
                echo "ERROR: setting the password of ${acct}: ${out}"
                PROVISION_FAIL=1
            fi
        fi
        return
    fi
    out=$(ergo_api ns/saregister "{\"accountName\":\"${acct}\",\"passphrase\":\"${pass}\"}")
    if echo "${out}" | grep -q '"success":true'; then
        echo "Account ${acct}: created."
        [ -n "${quiet}" ] || NEW_LOGINS="${NEW_LOGINS}${acct} ${pass}"$'\n'
    else
        echo "ERROR: creating account ${acct}: ${out}"
        PROVISION_FAIL=1
    fi
}

# Steward: its password is the cluster's value, set on every run
ensure_account steward "${STEWARD_PASSWORD}" reset quiet >/dev/null
ergo_api ns/saset '{"accountName":"steward","alwaysOn":"off"}' >/dev/null
# Bots: their passwords are those of secrets.env, set on every run; they are only online while the hub runs
for BOT in ${ACTIVE_BOTS}; do
    ensure_account "${BOT}" "$(irc_local_secret "IRC_BOT_PASSWORD_${BOT}")" reset quiet >/dev/null
    ergo_api ns/saset "{\"accountName\":\"${BOT}\",\"alwaysOn\":\"off\"}" >/dev/null
done
for ACCT in ${IRC_ALL_ACCOUNTS}; do
    PASS=$(irc_local_secret "IRC_PASSWORD_${ACCT}")
    [ -n "${PASS}" ] || { echo "ERROR: no IRC_PASSWORD_${ACCT} in secrets.env"; PROVISION_FAIL=1; continue; }
    MODE=""
    [[ " ${RESET_PASSWORDS} " == *" ${ACCT} "* ]] && MODE=reset
    ensure_account "${ACCT}" "${PASS}" "${MODE}"
done

# Channels: an IRC session as steward (operator). Every step is safe to repeat:
# REGISTER of a registered channel and an AMODE that is already set only produce a notice.
exec 3<>"/dev/tcp/127.0.0.1/${LOCAL_IRC_PORT}" || irc_fail "Cannot connect to the IRC port-forward."
irc_send()
{
    printf '%s\r\n' "$1" >&3
}
# Reads lines until one matches the pattern; answers PINGs, prints services notices and errors.
irc_wait()
{
    local pattern="$1" line
    while IFS= read -r -t 20 line <&3; do
        line="${line%$'\r'}"
        case "${line}" in
            PING*) irc_send "PONG ${line#PING }" ;;
        esac
        if [[ "${line}" =~ ^:(ChanServ|NickServ)![^\ ]*\ NOTICE ]]; then
            echo "  ${BASH_REMATCH[1]}: ${line#*NOTICE steward :}"
        elif [[ "${line}" =~ ^:[^\ ]+\ [45][0-9][0-9]\  ]]; then
            echo "  server: ${line#:* }"
        fi
        [[ "${line}" =~ ${pattern} ]] && return 0
    done
    return 1
}

irc_send "PASS steward:${STEWARD_PASSWORD}"
irc_send "NICK steward"
irc_send "USER steward 0 * :Steward"
irc_wait '^:[^ ]+ 001 ' || irc_fail "Steward could not log in to the IRC server."
irc_send "OPER steward ${STEWARD_PASSWORD}"
irc_wait '^:[^ ]+ (381|491) ' || irc_fail "No answer to OPER."
for CHAN in ${ALL_CHANNELS}; do
    echo "Channel ${CHAN}:"
    irc_send "JOIN ${CHAN}"
    irc_send "PRIVMSG ChanServ :REGISTER ${CHAN}"
    irc_send "MODE ${CHAN} +i"
    irc_send "PRIVMSG ChanServ :AMODE ${CHAN} +q ${IRC_ADMIN_ACCOUNT}"
    for ACCT in ${IRC_ALL_ACCOUNTS} $(bots_in_channel "${CHAN}"); do
        [ "${ACCT}" = "${IRC_ADMIN_ACCOUNT}" ] && continue
        # AMODE +v lets the account join the invite-only channel
        irc_send "PRIVMSG ChanServ :AMODE ${CHAN} +v ${ACCT}"
    done
    irc_send "PART ${CHAN}"
    irc_send "PING :done-${CHAN}"
    irc_wait "PONG .*done-${CHAN}" || { echo "ERROR: no answer while setting up ${CHAN}"; PROVISION_FAIL=1; }
done
irc_send "QUIT :provisioning done"
exec 3>&-

# ---------------------------------------------------------------------------
# gucefIrcHub: the bots
# ---------------------------------------------------------------------------
# A JSON string of the text
json_str()
{
    local s="$1"
    shopt -u patsub_replacement 2>/dev/null
    s="${s//\\/\\\\}"
    s="${s//\"/\\\"}"
    s="${s//$'\n'/\\n}"
    s="${s//$'\t'/\\t}"
    s="${s//$'\r'/}"
    printf '"%s"' "${s}"
}

HUB_STATUS=""
if [ -n "${ACTIVE_BOTS}" ]; then
    # the configuration; the secrets are environment variables from the Secret irc-hub-secrets
    {
        echo "{"
        echo "  \"server\": \"ergo.${NAMESPACE}.svc.cluster.local\", \"port\": 6697,"
        echo "  \"caFile\": \"/irc-tls/ca.crt\", \"tlsServerName\": $(json_str "${IRC_NODE}"),"
        echo "  \"operName\": \"bot\", \"operPasswordEnv\": \"IRC_BOT_OPER_PASSWORD\","
        echo "  \"threads\": 8,"
        echo "  \"bots\": ["
        FIRST=true
        for BOT in ${ACTIVE_BOTS}; do
            ${FIRST} || echo "    ,"
            FIRST=false
            KEY=$(irc_bot_setting "${BOT}" API_KEY)
            CHANNELS=$(echo $(irc_bot_setting "${BOT}" CHANNELS) | tr ' ' ',')
            echo "    { \"nick\": $(json_str "${BOT}"), \"passwordEnv\": $(json_str "IRC_BOT_PASSWORD_${BOT}"),"
            echo "      \"displayName\": $(json_str "$(irc_bot_setting "${BOT}" DISPLAY_NAME)"),"
            echo "      \"room\": $(json_str "$(irc_bot_setting "${BOT}" ROOM)"), \"channels\": $(json_str "${CHANNELS}"),"
            echo "      \"vendor\": $(json_str "$(irc_bot_setting "${BOT}" VENDOR)"), \"baseUrl\": $(json_str "$(irc_bot_setting "${BOT}" BASE_URL)"),"
            echo "      \"model\": $(json_str "$(irc_bot_setting "${BOT}" MODEL)"), \"apiKeyEnv\": $(json_str "${KEY}"),"
            echo "      \"effort\": $(json_str "$(irc_bot_setting "${BOT}" EFFORT)"),"
            echo "      \"maxTokens\": $(irc_bot_setting "${BOT}" MAX_TOKENS), \"dailyTokens\": $(irc_bot_setting "${BOT}" DAILY_TOKENS),"
            echo "      \"systemPrompt\": $(json_str "$(irc_bot_setting "${BOT}" SYSTEM_PROMPT)") }"
        done
        echo "  ]"
        echo "}"
    } > "${RENDER_DIR}/hub.json"
    if command -v python3 >/dev/null 2>&1 && ! python3 -I -c 'import json,sys; json.load(open(sys.argv[1]))' "${RENDER_DIR}/hub.json" 2>/dev/null; then
        irc_fail "The hub configuration is not valid JSON." "Check the IRC_BOT_* settings in config.env."
    fi

    HUB_SECRET_ARGS=(--from-literal=IRC_BOT_OPER_PASSWORD="$(irc_local_secret IRC_BOT_OPER_PASSWORD)")
    for BOT in ${ACTIVE_BOTS}; do
        HUB_SECRET_ARGS+=(--from-literal="IRC_BOT_PASSWORD_${BOT}=$(irc_local_secret "IRC_BOT_PASSWORD_${BOT}")")
        KEY=$(irc_bot_setting "${BOT}" API_KEY)
        [ -n "${KEY}" ] && HUB_SECRET_ARGS+=(--from-literal="${KEY}=$(irc_local_secret "${KEY}")")
    done
    ${KC} -n ${NAMESPACE} create secret generic irc-hub-secrets "${HUB_SECRET_ARGS[@]}" --dry-run=client -o yaml > "${RENDER_DIR}/hub-secret.yaml"
    ${KC} apply -f "${RENDER_DIR}/hub-secret.yaml" >/dev/null
    ${KC} -n ${NAMESPACE} create configmap irc-hub-config --from-file=hub.json="${RENDER_DIR}/hub.json" --dry-run=client -o yaml | ${KC} apply -f - >/dev/null
    IRC_HUB_CONFIG_HASH=$(cat "${RENDER_DIR}/hub.json" "${RENDER_DIR}/hub-secret.yaml" | sha256sum | cut -c1-16)

    # A hub that the CI deployed (an image of the registry, by digest) keeps its image: nothing is built or imported, and
    # the nodes pull the image (the registry is in their registries.yaml, forgejo/registry/setup-node.sh)
    CURRENT_HUB_IMAGE=$(${KC} -n ${NAMESPACE} get deployment irc-hub -o jsonpath='{.spec.template.spec.containers[0].image}' 2>/dev/null || true)
    if [[ "${CURRENT_HUB_IMAGE}" == */*@sha256:* ]]; then
        IRC_HUB_IMAGE="${CURRENT_HUB_IMAGE}"
        IRC_HUB_PULL_POLICY=IfNotPresent
        irc_render "${SCRIPT_DIR}/hub/deployment.yaml.tmpl" IRC_HUB_IMAGE IRC_HUB_PULL_POLICY IRC_HUB_CONFIG_HASH IRC_NODE | ${KC} apply -f -
        ${KC} -n ${NAMESPACE} rollout status deployment/irc-hub --timeout=2m || {
            echo "ERROR: the hub did not start. Check: ${KC} -n ${NAMESPACE} logs deploy/irc-hub"
            PROVISION_FAIL=1
        }
        HUB_STATUS="running (${IRC_HUB_IMAGE}, from the registry)"
    else
        # The image is imported into the containerd of this machine, so the hub runs here
        THIS_NODE=$(hostname | tr '[:upper:]' '[:lower:]')
        if [ "${THIS_NODE}" != "${IRC_NODE,,}" ]; then
            echo "ERROR: the hub runs on ${IRC_NODE} and its image is made there; run ./setup.sh on ${IRC_NODE}."
            PROVISION_FAIL=1
            HUB_STATUS="not installed: run ./setup.sh on ${IRC_NODE}"
        else
            IRC_HUB_IMAGE=$(bash "${SCRIPT_DIR}/hub/build-image.sh" | tail -n 1)
            if [[ "${IRC_HUB_IMAGE}" != gucef-irc-hub:* ]]; then
                echo "ERROR: the image of the hub could not be made."
                PROVISION_FAIL=1
                HUB_STATUS="not installed: the image could not be made"
            else
                IRC_HUB_PULL_POLICY=Never
                irc_render "${SCRIPT_DIR}/hub/deployment.yaml.tmpl" IRC_HUB_IMAGE IRC_HUB_PULL_POLICY IRC_HUB_CONFIG_HASH IRC_NODE | ${KC} apply -f -
                if ! ${KC} -n ${NAMESPACE} rollout status deployment/irc-hub --timeout=2m; then
                    # an image that was imported before may be gone (k3s reinstalled): import it again
                    if ${KC} -n ${NAMESPACE} get pods -l app=irc-hub -o jsonpath='{.items[*].status.containerStatuses[*].state.waiting.reason}' | grep -q ErrImageNeverPull; then
                        bash "${SCRIPT_DIR}/hub/build-image.sh" --force-import >/dev/null
                        ${KC} -n ${NAMESPACE} delete pod -l app=irc-hub --wait=false >/dev/null
                    fi
                    ${KC} -n ${NAMESPACE} rollout status deployment/irc-hub --timeout=2m || {
                        echo "ERROR: the hub did not start. Check: ${KC} -n ${NAMESPACE} logs deploy/irc-hub"
                        PROVISION_FAIL=1
                    }
                fi
                HUB_STATUS="running (${IRC_HUB_IMAGE})"
            fi
        fi
    fi
else
    ${KC} -n ${NAMESPACE} delete deployment irc-hub --ignore-not-found >/dev/null
    ${KC} -n ${NAMESPACE} delete configmap irc-hub-config --ignore-not-found >/dev/null
    ${KC} -n ${NAMESPACE} delete secret irc-hub-secrets --ignore-not-found >/dev/null
    HUB_STATUS="not running: no bots (IRC_HUB=${IRC_HUB})"
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
FINGERPRINT=$(${KC} -n ${NAMESPACE} get secret irc-lan-tls -o jsonpath='{.data.tls\.crt}' | base64 -d \
    | openssl x509 -noout -fingerprint -sha256 2>/dev/null | cut -d= -f2)
echo
echo "=========================================================================="
echo " IRC server is running"
echo "=========================================================================="
echo " Server:      ${IRC_HOST}, port ${IRC_LAN_PORT}, TLS (LAN)"
echo " Certificate: self-signed; accept it once in the app. SHA-256 fingerprint:"
echo "              ${FINGERPRINT}"
echo " Login:       SASL PLAIN (or server password \"account:password\"); nickname = account name"
echo " Passwords:   ~/.config/gucef-k3s/irc-secrets.env (IRC_PASSWORD_<account>)"
if [ -n "${NEW_LOGINS}" ]; then
    echo " New logins (account, password):"
    printf '%s' "${NEW_LOGINS}" | while read -r ACCT PASS; do printf '              %-16s %s\n' "${ACCT}" "${PASS}"; done
fi
echo " Operator:    /OPER admin <IRC_ADMIN_OPER_PASSWORD>"
echo " Channels:    ${ALL_CHANNELS}"
echo " Bots:        ${HUB_STATUS}"
for BOT in ${ACTIVE_BOTS}; do
    printf '              %-8s %s: everything in %s, in %s when addressed ("%s: ...")\n' "${BOT}" \
        "$(irc_bot_setting "${BOT}" DISPLAY_NAME)" "$(irc_bot_setting "${BOT}" ROOM)" "$(irc_bot_setting "${BOT}" CHANNELS)" "${BOT}"
done
if [ -n "${INACTIVE_BOTS}" ]; then
    echo " Not running:"
    printf '%s' "${INACTIVE_BOTS}" | while IFS= read -r LINE; do echo "              ${LINE}"; done
fi
echo
echo " Android: any IRC app. Add a server with the address and port above, TLS on, and turn"
echo " on SASL (PLAIN) with the account name and password; without it the server refuses the app."
echo " Change your own password: /msg NickServ PASSWD <current> <new> <new>"
echo "=========================================================================="
${KC} -n ${NAMESPACE} get pods,svc
exit ${PROVISION_FAIL}
