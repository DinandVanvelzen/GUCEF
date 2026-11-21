#!/bin/bash
# Helper functions shared by the IRC scripts. Sourced, not executed.
# Expects SCRIPT_DIR to be set by the caller.

KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"
NAMESPACE="irc"
PG_NAMESPACE="postgresql"
IRC_ERGO_IMAGE_DEFAULT="ghcr.io/ergochat/ergo:v2.19.1@sha256:ef885e44f7fa19101bbbc41baef11dc280dc8107465dccaf6f0860f41b48a682"
IRC_RELOADER_IMAGE="busybox:1.37"
LLM_NAMESPACE="llm"

irc_fail()
{
    echo "ERROR: $1" >&2
    [ -n "$2" ] && echo "       $2" >&2
    exit 1
}

# Defaults, then config.env, then validation.
irc_load_config()
{
    IRC_ADMIN_ACCOUNT=""
    IRC_ACCOUNTS=""
    IRC_CHANNELS="#family"
    IRC_HOST=""
    IRC_NODE=""
    IRC_LAN_PORT="32004"
    IRC_NETWORK_NAME="HomeIRC"
    IRC_SERVER_NAME="irc.home.lan"
    IRC_HISTORY_EXPIRE="52w"
    IRC_PVC_SIZE="1Gi"
    IRC_STORAGE_CLASS=""
    IRC_ERGO_IMAGE="${IRC_ERGO_IMAGE_DEFAULT}"
    IRC_BOTS="qwen claude gpt gemini grok"
    IRC_BOT_CHANNELS=""
    IRC_BOT_DAILY_TOKENS="200000"
    IRC_HUB="enabled"

    if [ -f "${SCRIPT_DIR}/config.env" ]; then
        set -a
        source <(sed 's/\r//' "${SCRIPT_DIR}/config.env")
        set +a
    fi

    if [ -z "${IRC_ADMIN_ACCOUNT}" ]; then
        IRC_ADMIN_ACCOUNT=$(echo "${SUDO_USER:-${USER}}" | tr '[:upper:]' '[:lower:]' | tr -c 'a-z0-9_\n' '_')
    fi
    local acct
    for acct in ${IRC_ADMIN_ACCOUNT} ${IRC_ACCOUNTS}; do
        [[ "${acct}" =~ ^[a-z][a-z0-9_]{0,31}$ ]] \
            || irc_fail "Invalid account name '${acct}'." "Use lower case letters, digits and underscore, starting with a letter."
        case "${acct}" in
            steward|admin|chanserv|nickserv|histserv) irc_fail "Account name '${acct}' is reserved." ;;
        esac
    done
    local chan
    for chan in ${IRC_CHANNELS}; do
        [[ "${chan}" =~ ^#[A-Za-z0-9_-]{1,63}$ ]] || irc_fail "Invalid channel name '${chan}' in IRC_CHANNELS."
    done
    # All accounts, administrator first, without duplicates
    IRC_ALL_ACCOUNTS=$(echo ${IRC_ADMIN_ACCOUNT} ${IRC_ACCOUNTS} | tr ' ' '\n' | awk 'NF && !seen[$0]++' | tr '\n' ' ')

    # Bots: shared channels default to IRC_CHANNELS
    [ -n "${IRC_BOT_CHANNELS}" ] || IRC_BOT_CHANNELS="${IRC_CHANNELS}"
    case "${IRC_HUB}" in
        enabled|none) ;;
        *) irc_fail "IRC_HUB must be enabled or none, not '${IRC_HUB}'." ;;
    esac
    local bot
    for bot in ${IRC_BOTS}; do
        [[ "${bot}" =~ ^[a-z][a-z0-9_]{0,31}$ ]] || irc_fail "Invalid bot name '${bot}' in IRC_BOTS."
        [[ " ${IRC_ALL_ACCOUNTS} " == *" ${bot} "* ]] && irc_fail "Bot '${bot}' has the name of a person's account."
        case "${bot}" in
            steward|admin|chanserv|nickserv|histserv) irc_fail "Bot name '${bot}' is reserved." ;;
        esac
        [ -n "$(irc_bot_setting "${bot}" VENDOR)" ] || irc_fail "Bot '${bot}' has no vendor." "Set IRC_BOT_${bot^^}_VENDOR and IRC_BOT_${bot^^}_BASE_URL in config.env."
        local room
        room=$(irc_bot_setting "${bot}" ROOM)
        [ -z "${room}" ] || [[ "${room}" =~ ^#[A-Za-z0-9_-]{1,63}$ ]] || irc_fail "Invalid room '${room}' of bot ${bot}."
    done
}

# A setting of a bot: IRC_BOT_<BOT>_<KEY> from config.env, else the default for that bot.
#   VENDOR        anthropic, openai, openai-compatible or llamacpp
#   BASE_URL      the API up to and including its version
#   MODEL         the model; empty for llama-server (it serves one)
#   API_KEY       the name of the key in secrets.env; the bot runs only when that key has a value
#   DISPLAY_NAME  what people see the bot as
#   ROOM          its own channel (answers everything), CHANNELS the shared ones (answers when addressed)
#   DAILY_TOKENS  tokens a person may use per day, 0: no limit
#   MAX_TOKENS, EFFORT (anthropic), SYSTEM_PROMPT (empty: the hub's default)
irc_bot_setting()
{
    local bot="$1" key="$2" var value
    var="IRC_BOT_${bot^^}_${key}"
    if [ -n "${!var+set}" ]; then
        echo "${!var}"
        return
    fi
    case "${key}" in
        ROOM) echo "#${bot}"; return ;;
        CHANNELS) echo "${IRC_BOT_CHANNELS}"; return ;;
        SYSTEM_PROMPT|EFFORT) echo ""; return ;;
    esac
    case "${bot}:${key}" in
        qwen:VENDOR) value="llamacpp" ;;
        qwen:BASE_URL) value="http://llama-server.${LLM_NAMESPACE}.svc.cluster.local:8080/v1" ;;
        qwen:DISPLAY_NAME) value="Qwen, running on our own server" ;;
        qwen:DAILY_TOKENS) value="0" ;;
        qwen:MAX_TOKENS) value="2048" ;;
        claude:VENDOR) value="anthropic" ;;
        claude:BASE_URL) value="https://api.anthropic.com/v1" ;;
        claude:MODEL) value="claude-opus-5-5" ;;
        claude:API_KEY) value="IRC_ANTHROPIC_API_KEY" ;;
        claude:DISPLAY_NAME) value="Claude, by Anthropic" ;;
        claude:EFFORT) value="low" ;;
        gpt:VENDOR) value="openai" ;;
        gpt:BASE_URL) value="https://api.openai.com/v1" ;;
        gpt:MODEL) value="gpt-5" ;;
        gpt:API_KEY) value="IRC_OPENAI_API_KEY" ;;
        gpt:DISPLAY_NAME) value="GPT, by OpenAI" ;;
        gemini:VENDOR) value="openai-compatible" ;;
        gemini:BASE_URL) value="https://generativelanguage.googleapis.com/v1beta/openai" ;;
        gemini:MODEL) value="gemini-2.5-flash" ;;
        gemini:API_KEY) value="IRC_GEMINI_API_KEY" ;;
        gemini:DISPLAY_NAME) value="Gemini, by Google" ;;
        grok:VENDOR) value="openai-compatible" ;;
        grok:BASE_URL) value="https://api.x.ai/v1" ;;
        grok:MODEL) value="grok-4" ;;
        grok:API_KEY) value="IRC_XAI_API_KEY" ;;
        grok:DISPLAY_NAME) value="Grok, by xAI" ;;
        *:DAILY_TOKENS) value="${IRC_BOT_DAILY_TOKENS}" ;;
        *:MAX_TOKENS) value="4096" ;;
        *:DISPLAY_NAME) value="${bot}" ;;
        *) value="" ;;
    esac
    echo "${value}"
}

irc_find_kubeseal()
{
    if command -v kubeseal >/dev/null 2>&1; then
        command -v kubeseal
    elif [ -x "${HOME}/.local/bin/kubeseal" ]; then
        echo "${HOME}/.local/bin/kubeseal"
    fi
}

# Verifies everything the IRC server needs from the rest of the k3s setup.
irc_preflight()
{
    ${KC} get nodes >/dev/null 2>&1 \
        || irc_fail "Cannot talk to the cluster with ${KUBECONFIG_PATH}." "Is k3s running, and is your user in the k3s-admin group? See ../setup.sh."
    ${KC} -n kube-system get deployment sealed-secrets >/dev/null 2>&1 \
        || irc_fail "The Sealed Secrets controller is not installed." "Run ../sealed-secrets/setup.sh first."
    KUBESEAL_BIN=$(irc_find_kubeseal)
    [ -n "${KUBESEAL_BIN}" ] || irc_fail "kubeseal is not installed." "Run ../sealed-secrets/setup.sh, it installs the CLI."
    export KUBESEAL_BIN
    ${KC} get crd certificates.cert-manager.io >/dev/null 2>&1 \
        || irc_fail "cert-manager is not installed." "Run ../cert-manager/setup.sh first."
    ${KC} get namespace "${PG_NAMESPACE}" >/dev/null 2>&1 \
        || irc_fail "PostgreSQL is not installed (namespace ${PG_NAMESPACE} not found)." "Run ../postgresql/setup.sh first."
    PG_POD=$(${KC} -n "${PG_NAMESPACE}" get pod -o jsonpath='{.items[0].metadata.name}' 2>/dev/null)
    [ -n "${PG_POD}" ] || irc_fail "No PostgreSQL pod found in namespace ${PG_NAMESPACE}." "Run ../postgresql/setup.sh and wait for the pod to start."
    PG_SERVICE=$(${KC} -n "${PG_NAMESPACE}" get svc -o jsonpath='{range .items[?(@.spec.clusterIP!="None")]}{.metadata.name}{"\n"}{end}' 2>/dev/null | head -n 1)
    [ -n "${PG_SERVICE}" ] || irc_fail "No PostgreSQL service found in namespace ${PG_NAMESPACE}."
    IRC_PG_HOST="${PG_SERVICE}.${PG_NAMESPACE}.svc.cluster.local"
    ${KC} -n "${PG_NAMESPACE}" get secret postgresql-credentials >/dev/null 2>&1 \
        || irc_fail "Secret postgresql-credentials not found in namespace ${PG_NAMESPACE}." "Run ../postgresql/setup.sh first."
    command -v curl >/dev/null 2>&1 || irc_fail "curl is not installed." "sudo apt-get install -y curl"
    command -v openssl >/dev/null 2>&1 || irc_fail "openssl is not installed." "sudo apt-get install -y openssl"
}

# Storage class: IRC_STORAGE_CLASS if set, else longhorn-raid when present, else the cluster default.
irc_detect_storage_class()
{
    if [ -n "${IRC_STORAGE_CLASS}" ]; then
        ${KC} get storageclass "${IRC_STORAGE_CLASS}" >/dev/null 2>&1 \
            || irc_fail "Storage class ${IRC_STORAGE_CLASS} (IRC_STORAGE_CLASS) does not exist."
        return
    fi
    if ${KC} get storageclass longhorn-raid >/dev/null 2>&1; then
        IRC_STORAGE_CLASS="longhorn-raid"
        return
    fi
    IRC_STORAGE_CLASS=$(${KC} get storageclass -o jsonpath='{range .items[?(@.metadata.annotations.storageclass\.kubernetes\.io/is-default-class=="true")]}{.metadata.name}{"\n"}{end}' | head -n 1)
    [ -n "${IRC_STORAGE_CLASS}" ] || irc_fail "No storage class found." "Set IRC_STORAGE_CLASS in config.env."
}

# The node Ergo runs on: IRC_NODE if set, else the first node without the ephemeral taint.
# Ergo listens on a host port of that node, which keeps the client addresses (a LoadBalancer
# service of k3s ServiceLB replaces them with an internal address).
# Clients connect to IRC_HOST: if not set, the InternalIP of that node.
irc_detect_host()
{
    if [ -n "${IRC_NODE}" ]; then
        ${KC} get node "${IRC_NODE}" >/dev/null 2>&1 || irc_fail "Node ${IRC_NODE} (IRC_NODE) does not exist."
    else
        IRC_NODE=$(${KC} get nodes -o jsonpath='{range .items[*]}{.metadata.name}{" "}{.spec.taints[*].key}{"\n"}{end}' 2>/dev/null \
            | awk '$0 !~ /gucef.org\/ephemeral/ {print $1; exit}')
        [ -n "${IRC_NODE}" ] || irc_fail "No node without the gucef.org/ephemeral taint found." "Set IRC_NODE in config.env."
    fi
    IRC_NODE_IP=$(${KC} get node "${IRC_NODE}" -o jsonpath='{.status.addresses[?(@.type=="InternalIP")].address}' 2>/dev/null)
    [ -n "${IRC_NODE_IP}" ] || irc_fail "Could not detect the address of node ${IRC_NODE}." "Set IRC_HOST in config.env."
    if [ -z "${IRC_HOST}" ]; then
        IRC_HOST="${IRC_NODE_IP}"
        IRC_HOST_AUTODETECTED=true
    fi
    # the pod network of that node: connections from it are exempt from Ergo's per-address limits
    IRC_POD_CIDR=$(${KC} get node "${IRC_NODE}" -o jsonpath='{.spec.podCIDR}' 2>/dev/null)
    [ -n "${IRC_POD_CIDR}" ] || IRC_POD_CIDR="10.42.0.0/16"
}

# Reads one key of the Secret irc-credentials
irc_secret()
{
    ${KC} -n ${NAMESPACE} get secret irc-credentials -o jsonpath="{.data.$1}" 2>/dev/null | base64 -d
}

# Value of KEY in the local secrets.env, empty when absent
irc_local_secret()
{
    [ -f "${SCRIPT_DIR}/secrets.env" ] || return 0
    sed 's/\r//' "${SCRIPT_DIR}/secrets.env" | grep -E "^$1=" | tail -n 1 | cut -d= -f2-
}

# Account password that is easy to type on a phone: three common words and a number,
# e.g. "maple-tiger-river-42" (128^3 * 90 = 189 million combinations). Ergo allows 3 failed logins per
# account per minute (accounts.login-throttling).
IRC_PASSWORD_WORDS=(
    apple acorn anchor arrow badge bakery banana basket beach berry bicycle blanket
    bottle bridge brook bubble button cabin cactus camera candle canoe carpet castle
    cherry circle cloud clover cocoa comet cookie copper cotton crayon daisy desert
    dolphin donkey dragon drum eagle engine falcon feather fern forest fossil fox
    garden giant ginger glacier globe goose grape guitar hammer harbor hazel helmet
    honey island jacket jelly jungle kettle kitten koala ladder lemon lantern lion
    mango maple marble meadow melon mirror monkey moon muffin nest ocean olive orange
    otter owl panda paper parrot peach pebble pencil pepper piano pillow pirate planet
    pocket pony puzzle rabbit rainbow river robin rocket saddle salmon sandal silver
    snail spoon squid star sunset tiger tomato tractor tulip turtle violin wagon
    walnut whale window yogurt zebra zipper
)
irc_easy_password()
{
    local n=${#IRC_PASSWORD_WORDS[@]} words="" i r
    for i in 1 2 3; do
        r=$(od -An -N4 -tu4 /dev/urandom | tr -d ' ')
        words="${words}${IRC_PASSWORD_WORDS[$(( r % n ))]}-"
    done
    r=$(od -An -N4 -tu4 /dev/urandom | tr -d ' ')
    echo "${words}$(( 10 + r % 90 ))"
}

# Prints FILE with ${NAME} replaced by the value of variable NAME, for each NAME given.
# Other ${...} text (e.g. shell variables in embedded scripts) is left as it is.
irc_render()
{
    local file="$1"; shift
    local content name value
    content=$(cat "${file}")
    shopt -u patsub_replacement 2>/dev/null
    for name in "$@"; do
        value="${!name}"
        content="${content//\$\{${name}\}/${value}}"
    done
    printf '%s\n' "${content}"
}
