#!/bin/bash
# Installs a Forgejo Actions runner in the forgejo namespace: the forgejo-runner daemon plus a
# privileged Docker-in-Docker sidecar that runs the job containers.
# Requires ../setup.sh to have run (Forgejo up). Idempotent.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FORGEJO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
KUBECONFIG_PATH="/etc/rancher/k3s/k3s.yaml"
NAMESPACE="forgejo"
RELEASE="forgejo"
KC="kubectl --kubeconfig ${KUBECONFIG_PATH}"

# Defaults, overridable through ../config.env
FORGEJO_HTTP_PORT="32001"
FORGEJO_STORAGE_CLASS=""
FORGEJO_RUNNER_VERSION="13.2.1"
FORGEJO_RUNNER_CAPACITY="2"
FORGEJO_RUNNER_MTU=""      # empty: detected from the pod network MTU, minus a safety margin
FORGEJO_RUNNER_STORAGE_SIZE="40Gi"
FORGEJO_RUNNER_JOB_IMAGE="node:20-bookworm"
FORGEJO_RUNNER_LABELS=""   # empty: gucef-linux and docker, both running FORGEJO_RUNNER_JOB_IMAGE

if [ -f "${FORGEJO_DIR}/config.env" ]; then
    set -a
    source <(sed 's/\r//' "${FORGEJO_DIR}/config.env")
    set +a
fi
source "${FORGEJO_DIR}/lib.sh"

${KC} -n ${NAMESPACE} get deployment ${RELEASE} >/dev/null 2>&1 \
    || fj_fail "Forgejo is not installed." "Run ../setup.sh first."
fj_detect_storage_class

if [ -z "${FORGEJO_RUNNER_LABELS}" ]; then
    FORGEJO_RUNNER_LABELS="gucef-linux:docker://${FORGEJO_RUNNER_JOB_IMAGE},docker:docker://${FORGEJO_RUNNER_JOB_IMAGE}"
fi
if [ -z "${FORGEJO_RUNNER_MTU}" ]; then
    # MTU of the cluster network as seen by a pod: read from the forgejo pod itself
    POD_MTU=$(${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- cat /sys/class/net/eth0/mtu 2>/dev/null | tr -d '\r\n ')
    FORGEJO_RUNNER_MTU=$(( ${POD_MTU:-1450} - 50 ))
fi
INSTANCE_URL="http://${RELEASE}-http.${NAMESPACE}.svc.cluster.local:${FORGEJO_HTTP_PORT}"

# ---------------------------------------------------------------------------
# Registration token (instance level), kept in a secret so re-runs reuse it
# ---------------------------------------------------------------------------
if ! ${KC} -n ${NAMESPACE} get secret forgejo-runner-registration >/dev/null 2>&1; then
    echo "Generating a runner registration token..."
    TOKEN=$(${KC} -n ${NAMESPACE} exec deploy/${RELEASE} -c forgejo -- forgejo actions generate-runner-token 2>/dev/null | tail -n 1 | tr -d '\r\n ')
    [ -n "${TOKEN}" ] || fj_fail "Could not generate a runner registration token."
    ${KC} -n ${NAMESPACE} create secret generic forgejo-runner-registration --from-literal=token="${TOKEN}"
fi

# ---------------------------------------------------------------------------
# Config, storage and workload
# ---------------------------------------------------------------------------
${KC} apply -f - <<EOT
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

# A changed ConfigMap does not restart the pod by itself
${KC} -n ${NAMESPACE} rollout restart deployment/forgejo-runner >/dev/null
${KC} -n ${NAMESPACE} rollout status deployment/forgejo-runner --timeout=10m || exit 1

echo
echo "Runner labels: ${FORGEJO_RUNNER_LABELS}"
echo "Workflows select it with: runs-on: gucef-linux   (or: docker)"
echo "Logs: kubectl -n ${NAMESPACE} logs deploy/forgejo-runner -c runner"
