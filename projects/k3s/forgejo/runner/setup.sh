#!/bin/bash
# Installs a Forgejo Actions runner in the forgejo namespace. Requires ../setup.sh to have run (Forgejo up). Idempotent.
#
# Usage: runner/setup.sh [--oci] [--dry-run]
#   (no option)  the runner forgejo-runner: the forgejo-runner daemon plus a privileged Docker-in-Docker sidecar that
#                runs the job containers; labels gucef-linux and docker (settings FORGEJO_RUNNER_*)
#   --oci        the second, isolated runner forgejo-runner-oci for image builds: a BuildKit daemon beside the runner,
#                the jobs run in host mode in the runner container with buildctl, no container daemon; label gucef-oci
#                (settings FORGEJO_RUNNER_OCI_*, FORGEJO_BUILDKIT_*, FORGEJO_REGISTRY_HOST; ../README.md, "Image build
#                runner"). The runner forgejo-runner and its resources are not touched. The pod resolves the names of
#                FORGEJO_REGISTRY_ALIASES (default worker.lan, the host of the token realm of the registry) to the
#                address of FORGEJO_REGISTRY_HOST through hostAliases, when that host is an IPv4 address.
#   --dry-run    prints the manifests on standard output and every other step on standard error, and changes nothing.
#                Reads the cluster (kubectl get) only when it is reachable; what it cannot read is shown as a
#                placeholder, the pod MTU as the assumed 1450. Check the manifests with:
#                    runner/setup.sh [--oci] --dry-run | kubectl apply --dry-run=client -f -
# Settings: ../config.env (config.env.example documents them).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORGEJO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="forgejo"
RELEASE="forgejo"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

OCI=false
DRY_RUN=false
while [ $# -gt 0 ]; do
    case "$1" in
        --oci) OCI=true ;;
        --dry-run) DRY_RUN=true ;;
        -h|--help) sed -n '2,/^# Settings:/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; echo "Usage: $0 [--oci] [--dry-run]" >&2; exit 2 ;;
    esac
    shift
done

# Defaults, overridable through ../config.env
FORGEJO_HTTP_PORT="32001"
FORGEJO_STORAGE_CLASS=""
FORGEJO_RUNNER_VERSION="13.2.1"
FORGEJO_RUNNER_CAPACITY="2"
FORGEJO_RUNNER_MTU=""      # empty: detected from the pod network MTU, minus a safety margin
FORGEJO_RUNNER_STORAGE_SIZE="40Gi"
FORGEJO_RUNNER_JOB_IMAGE="node:20-bookworm"
FORGEJO_RUNNER_LABELS=""   # empty: gucef-linux and docker, both running FORGEJO_RUNNER_JOB_IMAGE
# the image build runner (--oci)
FORGEJO_RUNNER_OCI_LABELS="gucef-oci:host"
FORGEJO_RUNNER_OCI_CAPACITY="1"
FORGEJO_RUNNER_OCI_STORAGE_SIZE="40Gi"
FORGEJO_RUNNER_OCI_VERSION=""      # empty: FORGEJO_RUNNER_VERSION
FORGEJO_RUNNER_OCI_NODE=""         # empty: any node; else the node name (kubernetes.io/hostname) the runner is bound to
FORGEJO_RUNNER_OCI_PACKAGES="bash git curl python3 nodejs coreutils"
FORGEJO_BUILDKIT_MODE="rootless"   # rootless or privileged
FORGEJO_BUILDKIT_IMAGE=""          # empty: moby/buildkit:<FORGEJO_BUILDKIT_VERSION>[-rootless]
FORGEJO_BUILDKIT_VERSION="v0.23.2"
FORGEJO_REGISTRY_HOST=""           # empty: the Forgejo host (FORGEJO_HOST or the first node address):FORGEJO_HTTP_PORT
FORGEJO_REGISTRY_ALIASES="worker.lan"  # host names that resolve to the registry address inside the pod (hostAliases)

if [ -f "${FORGEJO_DIR}/config.env" ]; then
    set -a
    source <(sed 's/\r//' "${FORGEJO_DIR}/config.env")
    set +a
fi
source "${FORGEJO_DIR}/lib.sh"

# in a dry run the steps go to standard error, so that standard output holds only the manifests
say()
{
    if ${DRY_RUN}; then echo "$*" >&2; else echo "$*"; fi
}
# applies the manifests of standard input, or prints them in a dry run
apply_manifests()
{
    if ${DRY_RUN}; then
        cat
    else
        ${KC} apply -f -
    fi
}

CLUSTER_REACHABLE=true
if ${DRY_RUN}; then
    ${KC} get nodes >/dev/null 2>&1 || CLUSTER_REACHABLE=false
    ${CLUSTER_REACHABLE} || say "DRY-RUN: the cluster is not reachable with ${KUBECONFIG_PATH}: values it would give are placeholders"
fi
if ${CLUSTER_REACHABLE}; then
    if ! ${KC} -n ${NAMESPACE} get deployment ${RELEASE} >/dev/null 2>&1; then
        ${DRY_RUN} || fj_fail "Forgejo is not installed." "Run ../setup.sh first."
        say "DRY-RUN: Forgejo is not installed; the real run stops here (run ../setup.sh first)"
    fi
    fj_detect_storage_class
else
    FORGEJO_STORAGE_CLASS="${FORGEJO_STORAGE_CLASS:-<storage class>}"
fi
INSTANCE_URL="http://${RELEASE}-http.${NAMESPACE}.svc.cluster.local:${FORGEJO_HTTP_PORT}"

# ---------------------------------------------------------------------------
# The runner forgejo-runner (no option)
# ---------------------------------------------------------------------------
default_runner_manifests()
{
cat <<EOT
apiVersion: v1
kind: ConfigMap
metadata:
  name: forgejo-runner-config
  namespace: ${NAMESPACE}
data:
  config.yml: |
    log:
      level: info
    runner:
      file: /data/.runner
      capacity: ${FORGEJO_RUNNER_CAPACITY}
      timeout: 3h
    cache:
      enabled: true
    container:
      privileged: false
  entrypoint.sh: |
    #!/bin/sh
    set -e
    cd /data
    echo "Waiting for the Docker daemon..."
    until wget -q -T2 -O- http://127.0.0.1:2375/_ping >/dev/null 2>&1; do sleep 2; done
    # Re-register when the label set changed
    if [ -f /data/.runner ] && [ "\$(cat /data/.labels 2>/dev/null)" != "\${RUNNER_LABELS}" ]; then
        echo "Labels changed, registering again"
        rm -f /data/.runner
    fi
    if [ ! -f /data/.runner ]; then
        forgejo-runner register --no-interactive \\
            --instance "\${INSTANCE_URL}" --token "\$(cat /secrets/token)" \\
            --name "\${RUNNER_NAME}" --labels "\${RUNNER_LABELS}"
        printf '%s' "\${RUNNER_LABELS}" > /data/.labels
    fi
    exec forgejo-runner daemon --config /config/config.yml
---
# The Docker daemon of the sidecar listens on tcp 2375 without authentication. The runner reaches it
# over the pod's loopback, so all ingress from the cluster network can be denied.
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: forgejo-runner-deny-ingress
  namespace: ${NAMESPACE}
spec:
  podSelector:
    matchLabels:
      app: forgejo-runner
  policyTypes: [Ingress]
  ingress: []
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: forgejo-runner-data-pvc
  namespace: ${NAMESPACE}
spec:
  accessModes: [ReadWriteOnce]
  storageClassName: ${FORGEJO_STORAGE_CLASS}
  resources:
    requests:
      storage: ${FORGEJO_RUNNER_STORAGE_SIZE}
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: forgejo-runner
  namespace: ${NAMESPACE}
spec:
  replicas: 1
  strategy:
    type: Recreate
  selector:
    matchLabels:
      app: forgejo-runner
  template:
    metadata:
      labels:
        app: forgejo-runner
    spec:
      securityContext:
        fsGroup: 1000   # the runner image runs as uid/gid 1000 and writes its state to /data
      containers:
        - name: dind
          image: docker:27-dind
          args: ["--mtu=${FORGEJO_RUNNER_MTU}"]   # stay below the pod network MTU or large downloads stall
          securityContext:
            privileged: true
          env:
            - name: DOCKER_TLS_CERTDIR
              value: ""
          volumeMounts:
            - name: data
              mountPath: /var/lib/docker
              subPath: docker
        - name: runner
          image: code.forgejo.org/forgejo/runner:${FORGEJO_RUNNER_VERSION}
          command: ["/bin/sh", "/config/entrypoint.sh"]
          env:
            - name: DOCKER_HOST
              value: tcp://127.0.0.1:2375
            - name: INSTANCE_URL
              value: ${INSTANCE_URL}
            - name: RUNNER_NAME
              value: ${RELEASE}-runner
            - name: RUNNER_LABELS
              value: "${FORGEJO_RUNNER_LABELS}"
          volumeMounts:
            - name: data
              mountPath: /data
              subPath: runner
            - name: config
              mountPath: /config
            - name: registration
              mountPath: /secrets
              readOnly: true
      volumes:
        - name: data
          persistentVolumeClaim:
            claimName: forgejo-runner-data-pvc
        - name: config
          configMap:
            name: forgejo-runner-config
        - name: registration
          secret:
            secretName: forgejo-runner-registration
EOT
}

# ---------------------------------------------------------------------------
# The image build runner forgejo-runner-oci (--oci)
# ---------------------------------------------------------------------------
OCI_NAME="forgejo-runner-oci"
OCI_REGISTRATION_SECRET="${OCI_NAME}-registration"
# the BuildKit daemon: its socket in an emptyDir both containers mount (not below /run, which the rootless image may
# replace in its own mount namespace), and tcp on the pod loopback
BUILDKIT_SOCKET_DIR="/buildkit-socket"
BUILDKIT_ADDRESS="unix://${BUILDKIT_SOCKET_DIR}/buildkitd.sock"
OCI_TOOLS_DIR="/opt/oci-tools/bin"

oci_runner_manifests()
{
    local buildkit_args buildkit_security buildkit_state node_selector="" host_aliases="" alias
    case "${FORGEJO_BUILDKIT_MODE}" in
        rootless)
            # rootless BuildKit: user namespaces of an unprivileged user, no seccomp and AppArmor filter (both block
            # the mounts it makes in its namespace); no process sandbox (no PID namespace of its own per step)
            buildkit_args='["--addr", "'"${BUILDKIT_ADDRESS}"'", "--addr", "tcp://127.0.0.1:1234", "--config", "/etc/buildkit/buildkitd.toml", "--oci-worker-no-process-sandbox"]'
            buildkit_security="            runAsUser: 1000
            runAsGroup: 1000
            seccompProfile:
              type: Unconfined
            appArmorProfile:
              type: Unconfined"
            buildkit_state="/home/user/.local/share/buildkit"
            ;;
        privileged)
            buildkit_args='["--addr", "'"${BUILDKIT_ADDRESS}"'", "--addr", "tcp://127.0.0.1:1234", "--config", "/etc/buildkit/buildkitd.toml"]'
            buildkit_security="            privileged: true"
            buildkit_state="/var/lib/buildkit"
            ;;
    esac
    # the registry names inside the pod: the token realm of the registry names its host (worker.lan), which the cluster
    # DNS need not resolve
    if [ -n "${REGISTRY_ALIAS_IP}" ] && [ -n "${FORGEJO_REGISTRY_ALIASES}" ]; then
        host_aliases="      hostAliases:
        - ip: \"${REGISTRY_ALIAS_IP}\"
          hostnames:"
        for alias in ${FORGEJO_REGISTRY_ALIASES//,/ }; do
            host_aliases="${host_aliases}
            - \"${alias}\""
        done
    fi
    if [ -n "${FORGEJO_RUNNER_OCI_NODE}" ]; then
        node_selector="      nodeSelector:
        kubernetes.io/hostname: \"${FORGEJO_RUNNER_OCI_NODE}\""
    fi
cat <<EOT
apiVersion: v1
kind: ConfigMap
metadata:
  name: ${OCI_NAME}-config
  namespace: ${NAMESPACE}
  labels:
    app: ${OCI_NAME}
data:
  config.yml: |
    log:
      level: info
    runner:
      file: /data/.runner
      capacity: ${FORGEJO_RUNNER_OCI_CAPACITY}
      timeout: 3h
    cache:
      enabled: true
    host:
      workdir_parent: /data/work
  buildkitd.toml: |
    # The registry of the cluster (the Forgejo container registry) speaks plain HTTP
    [registry."${FORGEJO_REGISTRY_HOST}"]
      http = true
      insecure = true
  entrypoint.sh: |
    #!/bin/sh
    set -e
    cd /data
    mkdir -p /data/work
    # The jobs run in this container (host mode): the tools they use, from the Alpine packages of the runner image
    if [ -n "\${RUNNER_PACKAGES}" ]; then
        echo "Installing the job tools: \${RUNNER_PACKAGES}"
        apk add --no-cache \${RUNNER_PACKAGES}
    fi
    echo "Waiting for the BuildKit daemon at \${BUILDKIT_HOST}..."
    until buildctl debug workers >/dev/null 2>&1; do sleep 2; done
    # Re-register when the label set changed
    if [ -f /data/.runner ] && [ "\$(cat /data/.labels 2>/dev/null)" != "\${RUNNER_LABELS}" ]; then
        echo "Labels changed, registering again"
        rm -f /data/.runner
    fi
    if [ ! -f /data/.runner ]; then
        forgejo-runner register --no-interactive \\
            --instance "\${INSTANCE_URL}" --token "\$(cat /secrets/token)" \\
            --name "\${RUNNER_NAME}" --labels "\${RUNNER_LABELS}"
        printf '%s' "\${RUNNER_LABELS}" > /data/.labels
    fi
    exec forgejo-runner daemon --config /config/config.yml
---
# Nothing in the cluster connects to this pod: the BuildKit daemon listens on the pod loopback and on a socket in an
# emptyDir, the runner only connects out. All ingress is denied.
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: ${OCI_NAME}-deny-ingress
  namespace: ${NAMESPACE}
  labels:
    app: ${OCI_NAME}
spec:
  podSelector:
    matchLabels:
      app: ${OCI_NAME}
  policyTypes: [Ingress]
  ingress: []
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ${OCI_NAME}-data-pvc
  namespace: ${NAMESPACE}
  labels:
    app: ${OCI_NAME}
spec:
  accessModes: [ReadWriteOnce]
  storageClassName: ${FORGEJO_STORAGE_CLASS}
  resources:
    requests:
      storage: ${FORGEJO_RUNNER_OCI_STORAGE_SIZE}
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ${OCI_NAME}
  namespace: ${NAMESPACE}
  labels:
    app: ${OCI_NAME}
spec:
  replicas: 1
  strategy:
    type: Recreate
  selector:
    matchLabels:
      app: ${OCI_NAME}
  template:
    metadata:
      labels:
        app: ${OCI_NAME}
    spec:
      # the jobs run in this pod: they get no token of the Kubernetes API
      automountServiceAccountToken: false
      enableServiceLinks: false
${node_selector:+${node_selector}
}${host_aliases:+${host_aliases}
}      securityContext:
        fsGroup: 1000   # the BuildKit user (1000) writes its state to the claim
      initContainers:
        # buildctl for the jobs, copied from the BuildKit image (a static binary) into an emptyDir on their PATH
        - name: buildctl
          image: ${FORGEJO_BUILDKIT_IMAGE}
          command: ["cp", "/usr/bin/buildctl", "${OCI_TOOLS_DIR}/buildctl"]
          securityContext:
            runAsUser: 1000
            runAsGroup: 1000
            allowPrivilegeEscalation: false
          volumeMounts:
            - name: tools
              mountPath: ${OCI_TOOLS_DIR}
      containers:
        - name: buildkitd
          image: ${FORGEJO_BUILDKIT_IMAGE}
          args: ${buildkit_args}
          securityContext:
${buildkit_security}
          readinessProbe:
            exec:
              command: ["buildctl", "--addr", "${BUILDKIT_ADDRESS}", "debug", "workers"]
            initialDelaySeconds: 5
            periodSeconds: 30
          volumeMounts:
            - name: buildkit-socket
              mountPath: ${BUILDKIT_SOCKET_DIR}
            - name: data
              mountPath: ${buildkit_state}
              subPath: buildkit
            - name: config
              mountPath: /etc/buildkit/buildkitd.toml
              subPath: buildkitd.toml
              readOnly: true
        - name: runner
          image: code.forgejo.org/forgejo/runner:${FORGEJO_RUNNER_OCI_VERSION}
          command: ["/bin/sh", "/config/entrypoint.sh"]
          # root inside its unprivileged container: the entry point installs the job tools with apk
          securityContext:
            runAsUser: 0
            runAsGroup: 0
            allowPrivilegeEscalation: false
          env:
            - name: BUILDKIT_HOST
              value: ${BUILDKIT_ADDRESS}
            - name: BUILDCTL
              value: ${OCI_TOOLS_DIR}/buildctl
            - name: PATH
              value: ${OCI_TOOLS_DIR}:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
            - name: REGISTRY_HOST
              value: "${FORGEJO_REGISTRY_HOST}"
            - name: INSTANCE_URL
              value: ${INSTANCE_URL}
            - name: RUNNER_NAME
              value: ${OCI_NAME}
            - name: RUNNER_LABELS
              value: "${FORGEJO_RUNNER_OCI_LABELS}"
            - name: RUNNER_PACKAGES
              value: "${FORGEJO_RUNNER_OCI_PACKAGES}"
          volumeMounts:
            - name: data
              mountPath: /data
              subPath: runner
            - name: config
              mountPath: /config
            - name: registration
              mountPath: /secrets
              readOnly: true
            - name: buildkit-socket
              mountPath: ${BUILDKIT_SOCKET_DIR}
            - name: tools
              mountPath: ${OCI_TOOLS_DIR}
              readOnly: true
      volumes:
        - name: data
          persistentVolumeClaim:
            claimName: ${OCI_NAME}-data-pvc
        - name: config
          configMap:
            name: ${OCI_NAME}-config
        - name: registration
          secret:
            secretName: ${OCI_REGISTRATION_SECRET}
        - name: buildkit-socket
          emptyDir: {}
        - name: tools
          emptyDir: {}
EOT
}

if ${OCI}; then
    # settings of the image build runner
    FORGEJO_RUNNER_OCI_VERSION="${FORGEJO_RUNNER_OCI_VERSION:-${FORGEJO_RUNNER_VERSION}}"
    case "${FORGEJO_BUILDKIT_MODE}" in
        rootless) FORGEJO_BUILDKIT_IMAGE="${FORGEJO_BUILDKIT_IMAGE:-moby/buildkit:${FORGEJO_BUILDKIT_VERSION}-rootless}" ;;
        privileged) FORGEJO_BUILDKIT_IMAGE="${FORGEJO_BUILDKIT_IMAGE:-moby/buildkit:${FORGEJO_BUILDKIT_VERSION}}" ;;
        *) fj_fail "FORGEJO_BUILDKIT_MODE '${FORGEJO_BUILDKIT_MODE}' is neither rootless nor privileged." ;;
    esac
    [[ "${FORGEJO_RUNNER_OCI_CAPACITY}" =~ ^[1-9][0-9]?$ ]] || fj_fail "FORGEJO_RUNNER_OCI_CAPACITY '${FORGEJO_RUNNER_OCI_CAPACITY}' is not a number of jobs."
    [ -z "${FORGEJO_RUNNER_OCI_NODE}" ] || [[ "${FORGEJO_RUNNER_OCI_NODE}" =~ ^[a-z0-9]([-a-z0-9.]{0,251}[a-z0-9])?$ ]] \
        || fj_fail "FORGEJO_RUNNER_OCI_NODE '${FORGEJO_RUNNER_OCI_NODE}' is not a node name."
    [[ "${FORGEJO_RUNNER_OCI_PACKAGES}" =~ ^[a-z0-9.+_\ -]*$ ]] || fj_fail "FORGEJO_RUNNER_OCI_PACKAGES holds a word that is no Alpine package name."
    [[ "${FORGEJO_RUNNER_OCI_LABELS}" =~ ^[A-Za-z0-9._-]+:host(,[A-Za-z0-9._-]+:host)*$ ]] \
        || fj_fail "FORGEJO_RUNNER_OCI_LABELS '${FORGEJO_RUNNER_OCI_LABELS}' must be <label>:host labels: the jobs of this runner run in host mode, it has no container daemon."
    if [ -z "${FORGEJO_REGISTRY_HOST}" ]; then
        if ${CLUSTER_REACHABLE}; then
            fj_detect_host
            FORGEJO_REGISTRY_HOST="${FORGEJO_HOST}:${FORGEJO_HTTP_PORT}"
        else
            FORGEJO_REGISTRY_HOST="<forgejo host>:${FORGEJO_HTTP_PORT}"
        fi
    fi
    [[ "${FORGEJO_REGISTRY_HOST}" =~ ^[A-Za-z0-9.-]+:[0-9]{1,5}$ ]] || ! ${CLUSTER_REACHABLE} \
        || fj_fail "FORGEJO_REGISTRY_HOST '${FORGEJO_REGISTRY_HOST}' is not <host>:<port>."
    # the address the alias names resolve to: the host of FORGEJO_REGISTRY_HOST when it is an IPv4 address
    REGISTRY_ALIAS_IP=""
    registry_address="${FORGEJO_REGISTRY_HOST%:*}"
    if [[ "${registry_address}" =~ ^([0-9]{1,3}\.){3}[0-9]{1,3}$ ]]; then
        REGISTRY_ALIAS_IP="${registry_address}"
    fi
    for alias in ${FORGEJO_REGISTRY_ALIASES//,/ }; do
        [[ "${alias}" =~ ^[A-Za-z0-9]([A-Za-z0-9.-]{0,252}[A-Za-z0-9])?$ ]] || fj_fail "FORGEJO_REGISTRY_ALIASES: '${alias}' is not a host name."
    done
    if [ -n "${FORGEJO_REGISTRY_ALIASES}" ] && [ -z "${REGISTRY_ALIAS_IP}" ]; then
        say "Note: FORGEJO_REGISTRY_HOST ${FORGEJO_REGISTRY_HOST} is not an IPv4 address: no hostAliases for ${FORGEJO_REGISTRY_ALIASES}; the pod resolves them through the cluster DNS."
    fi
    if [ -z "${FORGEJO_RUNNER_OCI_NODE}" ] && [ "${FORGEJO_BUILDKIT_MODE}" = "rootless" ]; then
        say "Note: FORGEJO_RUNNER_OCI_NODE is not set. Rootless BuildKit needs a node that allows unprivileged user namespaces"
        say "      (kernel.apparmor_restrict_unprivileged_userns=0 on Ubuntu 23.10 and later); set the node name in config.env."
    fi

    # Registration token: an own secret. It holds the token of forgejo-runner-registration when that exists, because
    # generating a token can invalidate the earlier one (the runner forgejo-runner would then fail to register again).
    # The token is passed through a pipe, never on a command line.
    if ${CLUSTER_REACHABLE} && ${KC} -n ${NAMESPACE} get secret ${OCI_REGISTRATION_SECRET} >/dev/null 2>&1; then
        say "Registration secret ${OCI_REGISTRATION_SECRET}: exists"
    elif ${DRY_RUN}; then
        say "DRY-RUN: create the secret ${OCI_REGISTRATION_SECRET} (key token) from the token of forgejo-runner-registration, or from forgejo actions generate-runner-token when that secret does not exist"
    elif ${KC} -n ${NAMESPACE} get secret forgejo-runner-registration >/dev/null 2>&1; then
        ${KC} -n ${NAMESPACE} get secret forgejo-runner-registration -o jsonpath='{.data.token}' | base64 -d \
            | ${KC} -n ${NAMESPACE} create secret generic ${OCI_REGISTRATION_SECRET} --from-file=token=/dev/stdin >/dev/null \
            || fj_fail "Could not create the secret ${OCI_REGISTRATION_SECRET}."
        say "Registration secret ${OCI_REGISTRATION_SECRET}: created with the token of forgejo-runner-registration"
    else
        say "Generating a runner registration token..."
        ${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo actions generate-runner-token 2>/dev/null | tail -n 1 | tr -d '\r\n ' \
            | ${KC} -n ${NAMESPACE} create secret generic ${OCI_REGISTRATION_SECRET} --from-file=token=/dev/stdin >/dev/null \
            || fj_fail "Could not generate a runner registration token."
        [ -n "$(${KC} -n ${NAMESPACE} get secret ${OCI_REGISTRATION_SECRET} -o jsonpath='{.data.token}')" ] \
            || { ${KC} -n ${NAMESPACE} delete secret ${OCI_REGISTRATION_SECRET} >/dev/null 2>&1; fj_fail "Could not generate a runner registration token."; }
    fi

    oci_runner_manifests | apply_manifests || fj_fail "The manifests of ${OCI_NAME} were not applied."
    if ${DRY_RUN}; then
        say "DRY-RUN: ${KC} -n ${NAMESPACE} rollout restart deployment/${OCI_NAME}, then rollout status --timeout=10m"
        exit 0
    fi
    # A changed ConfigMap does not restart the pod by itself
    ${KC} -n ${NAMESPACE} rollout restart deployment/${OCI_NAME} >/dev/null
    ${KC} -n ${NAMESPACE} rollout status deployment/${OCI_NAME} --timeout=10m || exit 1

    echo
    echo "Runner labels: ${FORGEJO_RUNNER_OCI_LABELS}"
    echo "Workflows select it with: runs-on: ${FORGEJO_RUNNER_OCI_LABELS%%:*}   (host mode: buildctl, BUILDKIT_HOST=${BUILDKIT_ADDRESS})"
    echo "Registry marked as plain HTTP for BuildKit: ${FORGEJO_REGISTRY_HOST}"
    echo "Logs: kubectl -n ${NAMESPACE} logs deploy/${OCI_NAME} -c runner   (BuildKit: -c buildkitd)"
    exit 0
fi

if [ -z "${FORGEJO_RUNNER_LABELS}" ]; then
    FORGEJO_RUNNER_LABELS="gucef-linux:docker://${FORGEJO_RUNNER_JOB_IMAGE},docker:docker://${FORGEJO_RUNNER_JOB_IMAGE}"
fi
if [ -z "${FORGEJO_RUNNER_MTU}" ]; then
    if ${DRY_RUN}; then
        # a dry run executes nothing in a pod: the MTU of the pod network is assumed
        FORGEJO_RUNNER_MTU=$(( 1450 - 50 ))
        say "DRY-RUN: the pod network MTU is not read (kubectl exec); assumed 1450, Docker MTU ${FORGEJO_RUNNER_MTU}"
    else
        # MTU of the cluster network as seen by a pod: read from the forgejo pod itself
        POD_MTU=$(${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- cat /sys/class/net/eth0/mtu 2>/dev/null | tr -d '\r\n ')
        FORGEJO_RUNNER_MTU=$(( ${POD_MTU:-1450} - 50 ))
    fi
fi

# ---------------------------------------------------------------------------
# Registration token (instance level), kept in a secret so re-runs reuse it
# ---------------------------------------------------------------------------
if ${DRY_RUN}; then
    if ! ${CLUSTER_REACHABLE} || ! ${KC} -n ${NAMESPACE} get secret forgejo-runner-registration >/dev/null 2>&1; then
        say "DRY-RUN: generate a runner registration token and create the secret forgejo-runner-registration when it does not exist"
    fi
elif ! ${KC} -n ${NAMESPACE} get secret forgejo-runner-registration >/dev/null 2>&1; then
    echo "Generating a runner registration token..."
    TOKEN=$(${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo actions generate-runner-token 2>/dev/null | tail -n 1 | tr -d '\r\n ')
    [ -n "${TOKEN}" ] || fj_fail "Could not generate a runner registration token."
    ${KC} -n ${NAMESPACE} create secret generic forgejo-runner-registration --from-literal=token="${TOKEN}"
fi

# ---------------------------------------------------------------------------
# Config, storage and workload
# ---------------------------------------------------------------------------
default_runner_manifests | apply_manifests
if ${DRY_RUN}; then
    say "DRY-RUN: ${KC} -n ${NAMESPACE} rollout restart deployment/forgejo-runner, then rollout status --timeout=10m"
    exit 0
fi

# A changed ConfigMap does not restart the pod by itself
${KC} -n ${NAMESPACE} rollout restart deployment/forgejo-runner >/dev/null
${KC} -n ${NAMESPACE} rollout status deployment/forgejo-runner --timeout=10m || exit 1

echo
echo "Runner labels: ${FORGEJO_RUNNER_LABELS}"
echo "Workflows select it with: runs-on: gucef-linux   (or: docker)"
echo "Logs: kubectl -n ${NAMESPACE} logs deploy/forgejo-runner -c runner"
