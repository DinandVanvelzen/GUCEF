#!/bin/bash
# Builds and pushes the container images a manual CI run asked for (workflow input 'images'), after every selected matrix
# cell passed. Runs on the image runner (label of cihelpergen:Matrix:ImageRunnerLabel), the only runner with an image
# builder. Every image is built from source by projects/k3s/gucef_service/scripts/build_image.sh and pushed to
# <registry host>/<owner>/<registryName> under the tag sha-<first 12 characters of the commit>, plus scratch-<run id> when
# the ref is not one of the push branches (a scratch snapshot) or candidate when it is (a published branch).
# Environment:
#   CI_IMAGES            comma separated image names of images.json, 'all' for every image; empty: nothing to do
#   CI_RUN_ID            id of the CI run: tag scratch-<run id> and the report package version <run id>-images
#   CI_SHA               commit of the run (default GITHUB_SHA)
#   CI_REF               ref of the run, for example refs/heads/scratch/box-20261122T120000Z-abc1234 (default GITHUB_REF)
#   CI_PUSH_BRANCHES     comma separated published branches (default master,main), the same list the planner uses
#   CI_SERVER_URL        URL of the CI server; its host and port are the registry host when CI_REGISTRY_HOST is empty
#   CI_REGISTRY_HOST     host:port of the container registry (default: from CI_SERVER_URL)
#   CI_REGISTRY_OWNER    owner the images are pushed under (default ci)
#   CI_REGISTRY_USER     registry user of the token (default ci-bot)
#   CI_REGISTRY_TOKEN    token of that user with write access to the packages of the owner (repository secret
#                        CI_REGISTRY_TOKEN), or CI_REGISTRY_TOKEN_FILE: a file holding it. It reaches build_image.sh only as
#                        REGISTRY_USER and REGISTRY_TOKEN_FILE (a file of this script, mode 0600, removed at the end)
# Optional:
#   CI_MATRIX_DIR        directory with images.json (default projects/CI/matrix)
#   CI_DEV_IMAGE         development image of the build stage (default <registry host>/<owner>/gucef-dev-base:latest;
#                        'none': build_image.sh uses its own default)
#   CI_IMAGE_REPORT_DIR  root of the reports (default ci-report): <root>/images/images.json and <name>.log.gz
#   CI_REPORT_UPLOAD_URL base URL of a Forgejo generic package registry, for example
#                        http://forgejo:3000/api/packages/<owner>/generic; with CI_REPORT_TOKEN (or a file named by
#                        CI_REPORT_TOKEN_FILE) the report and the logs are uploaded as package ci-reports, version
#                        <run id>-images. Upload failures are warnings.
#   CI_BUILD_IMAGE_SCRIPT for tests only: the script called instead of projects/k3s/gucef_service/scripts/build_image.sh
# images.json (hand maintained): { "schema": 1, "images": [ { "name", "serviceEnv", "registryName", "builder" } ] };
# registryName must be the last part of IMAGE_NAME in the service file, the name build_image.sh pushes.
# The host needs bash (the workflow step installs it when missing), python3, git and rsync (build_image.sh), curl (the
# report upload) and gzip; the ones missing are installed with apk or apt-get when the job runs as root or has
# password-less sudo. The builder client buildctl comes with the image runner; without it the script stops with a message.
# A failed image does not stop the others. The report images.json lists every image with its tag, extra tags, digest,
# status and the tail of its log. The script exits non-zero when any image failed. Tokens are never printed; the output
# of build_image.sh is filtered for the registry token before it reaches the job log and the stored log.

set -u
REPO_ROOT="${GITHUB_WORKSPACE:-$(pwd)}"
cd "${REPO_ROOT}" || exit 1
MATRIX_DIR="${CI_MATRIX_DIR:-projects/CI/matrix}"
IMAGES_FILE="${MATRIX_DIR}/images.json"
BUILD_IMAGE_SCRIPT="${CI_BUILD_IMAGE_SCRIPT:-projects/k3s/gucef_service/scripts/build_image.sh}"
LOG_TAIL_LINES=200

RUN_ID="${CI_RUN_ID:-local}"
COMMIT="${CI_SHA:-${GITHUB_SHA:-}}"
REF="${CI_REF:-${GITHUB_REF:-}}"
REGISTRY_USER_NAME="${CI_REGISTRY_USER:-ci-bot}"
REGISTRY_OWNER="${CI_REGISTRY_OWNER:-ci}"

REPORT_ROOT="${CI_IMAGE_REPORT_DIR:-ci-report}"
case "${REPORT_ROOT}" in /*) ;; *) REPORT_ROOT="${REPO_ROOT}/${REPORT_ROOT}" ;; esac
REPORT_DIR="${REPORT_ROOT}/images"
WORK_DIR="${REPORT_DIR}/work"
rm -rf "${REPORT_DIR}"
mkdir -p "${WORK_DIR}" || { echo "Cannot create the report directory ${REPORT_DIR}"; exit 1; }
: > "${WORK_DIR}/images.tsv"

# The registry token file lives outside the workspace and the report, owner only, and is removed on exit
SECRET_DIR="$(umask 077 && mktemp -d "${RUNNER_TEMP:-${TMPDIR:-/tmp}}/ci-image.XXXXXX")" || { echo "Cannot create a temporary directory"; exit 1; }
trap 'rm -rf "${SECRET_DIR}"' EXIT

if [ -z "${CI_IMAGES:-}" ] || [ -z "$(echo "${CI_IMAGES}" | tr -d ', ')" ]; then
    echo "No images requested (CI_IMAGES is empty)"
    exit 0
fi

# ---------------------------------------------------------------------------
# Tools of the host
# ---------------------------------------------------------------------------
ensure_tools()
{
    local need="" tool SUDO=""
    for tool in python3 git rsync curl gzip; do
        command -v "${tool}" >/dev/null || need="${need} ${tool}"
    done
    [ -n "${need}" ] || return 0
    if [ "$(id -u)" -ne 0 ]; then
        if command -v sudo >/dev/null && sudo -n true 2>/dev/null; then
            SUDO="sudo"
        else
            echo "WARNING: missing:${need}; not root and no password-less sudo, nothing is installed"
            return 0
        fi
    fi
    echo "Installing:${need}"
    if command -v apk >/dev/null; then
        ${SUDO} apk add --no-cache ${need} >/dev/null || echo "WARNING: apk could not install${need}"
    elif command -v apt-get >/dev/null; then
        { ${SUDO} apt-get update -qq && ${SUDO} apt-get install -y -qq ${need} >/dev/null; } || echo "WARNING: apt-get could not install${need}"
    else
        echo "WARNING: missing:${need}; the host has neither apk nor apt-get"
    fi
}
ensure_tools
command -v python3 >/dev/null || { echo "python3 is missing on the image runner: cannot read images.json or write the report"; exit 1; }

# Forget the tokens in this process: the registry token goes to a file, the report token stays only for the upload
TOKEN_FILE=""
if [ -n "${CI_REGISTRY_TOKEN:-}" ]; then
    TOKEN_FILE="${SECRET_DIR}/registry-token"
    ( umask 077 && printf '%s' "${CI_REGISTRY_TOKEN}" > "${TOKEN_FILE}" )
elif [ -n "${CI_REGISTRY_TOKEN_FILE:-}" ] && [ -r "${CI_REGISTRY_TOKEN_FILE}" ]; then
    TOKEN_FILE="${SECRET_DIR}/registry-token"
    ( umask 077 && tr -d '\r\n' < "${CI_REGISTRY_TOKEN_FILE}" > "${TOKEN_FILE}" )
fi
unset CI_REGISTRY_TOKEN
REPORT_TOKEN="${CI_REPORT_TOKEN:-}"
if [ -z "${REPORT_TOKEN}" ] && [ -n "${CI_REPORT_TOKEN_FILE:-}" ] && [ -r "${CI_REPORT_TOKEN_FILE}" ]; then
    REPORT_TOKEN="$(tr -d '\r\n' < "${CI_REPORT_TOKEN_FILE}")"
fi
unset CI_REPORT_TOKEN

# ---------------------------------------------------------------------------
# Tags
# ---------------------------------------------------------------------------
SCRATCH=true
BRANCH=""
case "${REF}" in refs/heads/*) BRANCH="${REF#refs/heads/}" ;; esac
PUSH_BRANCHES="${CI_PUSH_BRANCHES:-}"
[ -n "$(echo "${PUSH_BRANCHES}" | tr -d ', ')" ] || PUSH_BRANCHES="master,main"
if [ -n "${BRANCH}" ]; then
    IFS=',' read -r -a published <<< "${PUSH_BRANCHES}"
    for b in "${published[@]}"; do
        b="$(echo "${b}" | tr -d '[:space:]')"
        [ -n "${b}" ] && [ "${b}" = "${BRANCH}" ] && SCRATCH=false
    done
fi

TAG=""
ALSO_TAGS=()
PRECONDITION=""
if ! [[ "${COMMIT}" =~ ^[0-9a-f]{12,64}$ ]]; then
    PRECONDITION="the commit '${COMMIT}' (CI_SHA) is not a commit hash"
else
    TAG="sha-${COMMIT:0:12}"
fi
if ${SCRATCH}; then
    if [[ "${RUN_ID}" =~ ^[A-Za-z0-9_][A-Za-z0-9_.-]{0,100}$ ]]; then
        ALSO_TAGS+=( "scratch-${RUN_ID}" )
    else
        PRECONDITION="${PRECONDITION:-the run id '${RUN_ID}' (CI_RUN_ID) cannot be part of a tag}"
    fi
else
    ALSO_TAGS+=( "candidate" )
fi
ALSO_TAGS_TEXT="$(IFS=','; echo "${ALSO_TAGS[*]:-}")"

# ---------------------------------------------------------------------------
# Registry
# ---------------------------------------------------------------------------
REGISTRY_HOST="${CI_REGISTRY_HOST:-}"
if [ -z "${REGISTRY_HOST}" ]; then
    REGISTRY_HOST="${CI_SERVER_URL:-}"
    REGISTRY_HOST="${REGISTRY_HOST#*://}"
    REGISTRY_HOST="${REGISTRY_HOST%%/*}"
    REGISTRY_HOST="${REGISTRY_HOST##*@}"
fi
if ! [[ "${REGISTRY_HOST}" =~ ^[A-Za-z0-9.-]+(:[0-9]+)?$ ]]; then
    PRECONDITION="${PRECONDITION:-no valid registry host (CI_REGISTRY_HOST '${CI_REGISTRY_HOST:-}', CI_SERVER_URL '${CI_SERVER_URL:-}')}"
fi
if ! [[ "${REGISTRY_OWNER}" =~ ^[A-Za-z0-9][A-Za-z0-9._-]*$ ]]; then
    PRECONDITION="${PRECONDITION:-the registry owner '${REGISTRY_OWNER}' (CI_REGISTRY_OWNER) is not valid}"
fi
REGISTRY="${REGISTRY_HOST}/${REGISTRY_OWNER}"
DEV_IMAGE="${CI_DEV_IMAGE:-${REGISTRY}/gucef-dev-base:latest}"
[ "${DEV_IMAGE}" = "none" ] && DEV_IMAGE=""
[ -n "${TOKEN_FILE}" ] || PRECONDITION="${PRECONDITION:-CI_REGISTRY_TOKEN is not set: nothing can be pushed (repository secret CI_REGISTRY_TOKEN)}"

echo "Images: ${CI_IMAGES}"
echo "Commit ${COMMIT}, ref ${REF}, scratch=${SCRATCH}, run ${RUN_ID}"
echo "Registry ${REGISTRY} as ${REGISTRY_USER_NAME}, tag ${TAG:-<none>}, also ${ALSO_TAGS_TEXT:-<none>}"

# ---------------------------------------------------------------------------
# report and upload
# ---------------------------------------------------------------------------
export REPORT_DIR WORK_DIR RUN_ID COMMIT REF SCRATCH LOG_TAIL_LINES

write_report()
{
    python3 - <<'PY'
import json, os

env = os.environ
work = env["WORK_DIR"]

def tail(path, lines):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return "".join(f.readlines()[-lines:])
    except OSError:
        return ""

images = []
with open(os.path.join(work, "images.tsv"), encoding="utf-8") as f:
    for line in f:
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 7:
            continue
        name, registry_name, tag, also, digest, status, secs = parts[:7]
        images.append({"name": name, "registryName": registry_name, "tag": tag,
                       "alsoTags": [t for t in also.split(",") if t], "digest": digest, "status": status,
                       "seconds": int(secs), "logTail": tail(os.path.join(work, name + ".log"), int(env["LOG_TAIL_LINES"]))})

ok = bool(images) and all(i["status"] == "ok" for i in images)
report = {"schema": 1, "run": env.get("RUN_ID", ""), "commit": env.get("COMMIT", ""), "ref": env.get("REF", ""),
          "scratch": env.get("SCRATCH", "true") == "true", "status": "ok" if ok else "failed", "images": images}
with open(os.path.join(env["REPORT_DIR"], "images.json"), "w", encoding="utf-8") as f:
    json.dump(report, f, indent=2)
    f.write("\n")
PY
}

# Copies stdin to stdout line by line with the registry token replaced by '***', in case a tool prints it
redact_stream()
{
    REDACT_TOKEN_FILE="${TOKEN_FILE}" python3 -u -c '
import os, sys
secret = b""
try:
    with open(os.environ["REDACT_TOKEN_FILE"], "rb") as f:
        secret = f.read().strip()
except (OSError, KeyError):
    pass
out = sys.stdout.buffer
for line in sys.stdin.buffer:
    if len(secret) >= 4:
        line = line.replace(secret, b"***")
    out.write(line)
    out.flush()
'
}

# Replaces the token values in a log file by '***', in case a tool printed one
redact_log()
{
    REDACT_FILE="$1" REDACT_TOKEN_FILE="${TOKEN_FILE}" REDACT_REPORT_TOKEN="${REPORT_TOKEN}" python3 - <<'PY'
import os
path = os.environ["REDACT_FILE"]
secrets = []
token_file = os.environ.get("REDACT_TOKEN_FILE", "")
if token_file:
    try:
        with open(token_file, encoding="utf-8", errors="replace") as f:
            secrets.append(f.read().strip())
    except OSError:
        pass
secrets.append(os.environ.get("REDACT_REPORT_TOKEN", ""))
secrets = [s for s in secrets if len(s) >= 4]
if secrets:
    with open(path, encoding="utf-8", errors="replace") as f:
        text = f.read()
    changed = text
    for s in secrets:
        changed = changed.replace(s, "***")
    if changed != text:
        with open(path, "w", encoding="utf-8") as f:
            f.write(changed)
PY
}

# ---------------------------------------------------------------------------
# REPORT UPLOAD (replaceable)
# Same transport as upload_report of ci_cell.sh: PUT to the Forgejo generic package registry, package ci-reports,
# version <run id>-images, authorised by CI_REPORT_TOKEN or the file named by CI_REPORT_TOKEN_FILE. The token is never
# printed; failures are warnings.
# ---------------------------------------------------------------------------
upload_report()
{
    local base="${CI_REPORT_UPLOAD_URL:-}"
    local token="${REPORT_TOKEN}"
    if [ -z "${base}" ] || [ -z "${token}" ]; then
        echo "Report upload: skipped (CI_REPORT_UPLOAD_URL or CI_REPORT_TOKEN not set)"
        return 0
    fi
    command -v curl >/dev/null || { echo "WARNING: report upload skipped, curl is missing"; return 0; }

    local version="${RUN_ID}-images"
    local url="${base%/}/ci-reports/${version}"
    local header="${SECRET_DIR}/upload-header"
    ( umask 077 && printf 'Authorization: token %s\n' "${token}" > "${header}" )
    local file name code uploaded=0 failures=0
    for file in "${REPORT_DIR}/images.json" "${REPORT_DIR}"/*.log.gz; do
        [ -f "${file}" ] || continue
        name="$(basename "${file}")"
        code="$(curl -s -o /dev/null -w '%{http_code}' --max-time 120 -H @"${header}" -X PUT --upload-file "${file}" "${url}/${name}")"
        if [ "${code}" = "409" ]; then
            # A re-run of the same run id: replace the earlier file
            curl -s -o /dev/null --max-time 60 -H @"${header}" -X DELETE "${url}/${name}"
            code="$(curl -s -o /dev/null -w '%{http_code}' --max-time 120 -H @"${header}" -X PUT --upload-file "${file}" "${url}/${name}")"
        fi
        case "${code}" in
            200|201) uploaded=$((uploaded + 1)) ;;
            *) failures=$((failures + 1)); echo "WARNING: report upload of ${name} failed (HTTP ${code})" ;;
        esac
    done
    rm -f "${header}"
    echo "Report upload: ${uploaded} file(s) to ${url}, ${failures} failed"
    return 0
}

# Writes the report, uploads it and exits: 0 when every image was pushed, 1 otherwise
finish()
{
    write_report || echo "WARNING: writing images.json failed"
    echo "Report: ${REPORT_DIR}/images.json"
    upload_report
    local failed
    failed="$(awk -F'\t' '$6 != "ok" { printf " %s", $1 }' "${WORK_DIR}/images.tsv")"
    if [ -n "${failed}" ] || [ ! -s "${WORK_DIR}/images.tsv" ]; then
        echo "Images: FAILED:${failed:- none built}"
        exit 1
    fi
    echo "Images: all pushed"
    exit 0
}

# Records one image: name registryName status seconds digest; the log is ${WORK_DIR}/<name>.log
record()
{
    local name="$1" registry_name="$2" status="$3" secs="$4" digest="$5"
    local log="${WORK_DIR}/${name}.log"
    touch "${log}"
    redact_log "${log}"
    gzip -c "${log}" > "${REPORT_DIR}/${name}.log.gz"
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "${name}" "${registry_name}" "${TAG}" "${ALSO_TAGS_TEXT}" "${digest}" "${status}" "${secs}" \
        >> "${WORK_DIR}/images.tsv"
}

# ---------------------------------------------------------------------------
# The images to build
# ---------------------------------------------------------------------------
SELECTION="${WORK_DIR}/selection.tsv"
IMAGES_FILE="${IMAGES_FILE}" CI_IMAGES="${CI_IMAGES}" python3 - > "${SELECTION}" <<'PY'
import json, os, re, sys

path = os.environ["IMAGES_FILE"]
try:
    with open(path, encoding="utf-8") as f:
        data = json.load(f)
except (OSError, ValueError) as e:
    print("ERROR: cannot read %s: %s" % (path, e), file=sys.stderr)
    sys.exit(2)
if data.get("schema") != 1:
    print("ERROR: %s: schema %r is not supported, expected 1" % (path, data.get("schema")), file=sys.stderr)
    sys.exit(2)
valid = re.compile(r"^[a-z0-9][a-z0-9._-]*$")
known = []
for image in data.get("images", []):
    name = image.get("name", "")
    registry_name = image.get("registryName", name)
    service_env = image.get("serviceEnv", "")
    builder = image.get("builder", "buildkit")
    if not valid.match(name) or not valid.match(registry_name) or not service_env or "\t" in service_env or not valid.match(builder):
        print("ERROR: %s: invalid entry %r" % (path, image), file=sys.stderr)
        sys.exit(2)
    known.append((name, service_env, registry_name, builder))
names = [n.strip() for n in os.environ.get("CI_IMAGES", "").split(",") if n.strip()]
if "all" in names:
    selected = known
else:
    unknown = [n for n in names if n not in [k[0] for k in known]]
    if unknown:
        print("ERROR: unknown image(s): %s. Known images: %s (or all)"
              % (", ".join(unknown), ", ".join(k[0] for k in known) or "none"), file=sys.stderr)
        sys.exit(3)
    selected = [k for k in known if k[0] in names]
for entry in selected:
    print("\t".join(entry))
PY
rc=$?
if [ ${rc} -ne 0 ]; then
    echo "Images: nothing built (images.json or CI_IMAGES rejected, see above)"
    finish
fi

mapfile -t ENTRIES < "${SELECTION}"

# Preconditions that fail every image alike
if [ -z "${PRECONDITION}" ]; then
    for entry in "${ENTRIES[@]}"; do
        IFS=$'\t' read -r name service_env registry_name builder <<< "${entry}"
        if [ "${builder}" = "buildkit" ] && ! command -v buildctl >/dev/null; then
            PRECONDITION="buildctl is not installed on this runner: images are built only on the image runner, whose init container provides the builder client (runner label of cihelpergen:Matrix:ImageRunnerLabel)"
            break
        fi
    done
fi
[ -f "${BUILD_IMAGE_SCRIPT}" ] || PRECONDITION="${PRECONDITION:-the image build script ${BUILD_IMAGE_SCRIPT} does not exist}"
if [ -n "${PRECONDITION}" ]; then
    echo "ERROR: ${PRECONDITION}"
    for entry in "${ENTRIES[@]}"; do
        IFS=$'\t' read -r name service_env registry_name builder <<< "${entry}"
        echo "Not built: ${PRECONDITION}" > "${WORK_DIR}/${name}.log"
        record "${name}" "${registry_name}" "failed" 0 ""
    done
    finish
fi

# ---------------------------------------------------------------------------
# Build and push, one image after the other
# ---------------------------------------------------------------------------
for entry in "${ENTRIES[@]}"; do
    IFS=$'\t' read -r name service_env registry_name builder <<< "${entry}"
    log="${WORK_DIR}/${name}.log"
    digest_file="${WORK_DIR}/${name}.digest"
    : > "${log}"
    rm -f "${digest_file}"
    start=${SECONDS}
    echo "::group::${name}"
    rc=0
    image_name=""
    if [ ! -f "${service_env}" ]; then
        echo "The service file ${service_env} of ${name} does not exist" | tee -a "${log}"
        rc=2
    else
        image_name="$(sed -n 's/^IMAGE_NAME=//p' "${service_env}" | tail -n 1 | tr -d '\r' | sed -e 's/^"\(.*\)"$/\1/' -e "s/^'\(.*\)'$/\1/")"
        if [ "${image_name##*/}" != "${registry_name}" ]; then
            echo "registryName '${registry_name}' of ${name} does not match IMAGE_NAME '${image_name}' of ${service_env}: build_image.sh pushes the last part of IMAGE_NAME" | tee -a "${log}"
            rc=2
        fi
    fi
    if [ ${rc} -eq 0 ]; then
        args=( --env "${service_env}" --builder "${builder}" --registry "${REGISTRY}" --push --tag "${TAG}" )
        for t in "${ALSO_TAGS[@]}"; do
            args+=( --also-tag "${t}" )
        done
        args+=( --digest-file "${digest_file}" )
        [ -n "${DEV_IMAGE}" ] && args+=( --dev-image "${DEV_IMAGE}" )
        echo "=== ${name}: build_image.sh ${args[*]}" | tee -a "${log}"
        # The child sees the registry token only as a file, the report token not at all
        (
            unset CI_REPORT_TOKEN CI_REPORT_TOKEN_FILE CI_REGISTRY_TOKEN_FILE
            export REGISTRY_USER="${REGISTRY_USER_NAME}" REGISTRY_TOKEN_FILE="${TOKEN_FILE}"
            bash "${BUILD_IMAGE_SCRIPT}" "${args[@]}"
        ) < /dev/null 2>&1 | redact_stream | tee -a "${log}"
        rc=${PIPESTATUS[0]}
    fi
    digest=""
    if [ ${rc} -eq 0 ]; then
        digest="$(head -n 1 "${digest_file}" 2>/dev/null | tr -d '[:space:]')"
        if ! [[ "${digest}" =~ ^sha256:[0-9a-f]{64}$ ]]; then
            echo "build_image.sh succeeded but wrote no valid digest to ${digest_file} ('${digest}')" | tee -a "${log}"
            digest=""
            rc=3
        fi
    fi
    echo "::endgroup::"
    status="ok"
    if [ ${rc} -ne 0 ]; then
        status="failed"
        echo "${name}: FAILED (exit code ${rc})" | tee -a "${log}"
    else
        echo "${name}: pushed ${REGISTRY}/${registry_name}:${TAG} ${digest}"
    fi
    record "${name}" "${registry_name}" "${status}" "$((SECONDS - start))" "${digest}"
done
finish
