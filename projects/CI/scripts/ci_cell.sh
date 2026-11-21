#!/bin/bash
# Builds one matrix cell: every product in CELL_PRODUCTS with the cell's C++ standard, build type and toolchain.
# Environment: CELL_ID CELL_PLATFORM CELL_COMPILER CELL_CPP_STANDARD CELL_BUILD_TYPE CELL_CMAKE_ARGS CELL_PRODUCTS
# Optional:
#   CELL_MODE            'build' (default): configure and build; 'test': also run the built test applications
#   CELL_REPORT_DIR      root of the reports (default ci-report): <root>/<cell>/report.json and <product>.log.gz
#   CELL_TEST_TIMEOUT    seconds per test application (default 300)
#   CELL_BUILD_JOBS      parallel build jobs (default: the build tool's default)
#   CELL_BUILD_ROOT      build directories <root>/<cell>/<product> (default common/bin)
#   CI_MATRIX_DIR        directory with products.json and test-skip.txt (default projects/CI/matrix)
#   CI_RUN_ID            id of the CI run, part of the report package version
#   CI_REPORT_UPLOAD_URL base URL of a Forgejo generic package registry, for example
#                        http://forgejo:3000/api/packages/<owner>/generic; with CI_REPORT_TOKEN (or a file named by
#                        CI_REPORT_TOKEN_FILE) the report and the logs are uploaded as package ci-reports,
#                        version <run id>-<cell id>. Upload failures are warnings.
# Each product is a generated CMake project (projects/CMake/targets/<product>); a failed product does not stop the others.
# Test applications are the executables named *_TestApp in the bin directory of the product's build; the ones listed in
# test-skip.txt are not run. Each runs in its own directory, where it writes <App>_Results.xml.
# The cell exits non-zero when any product failed.

set -u
REPO_ROOT="${GITHUB_WORKSPACE:-$(pwd)}"
cd "${REPO_ROOT}" || exit 1
MATRIX_DIR="${CI_MATRIX_DIR:-projects/CI/matrix}"
PRODUCTS_FILE="${MATRIX_DIR}/products.json"
TEST_SKIP_FILE="${MATRIX_DIR}/test-skip.txt"

: "${CELL_ID:=local}" "${CELL_PLATFORM:=}" "${CELL_COMPILER:=}" "${CELL_CPP_STANDARD:=}" "${CELL_BUILD_TYPE:=Release}"
: "${CELL_CMAKE_ARGS:=}" "${CELL_PRODUCTS:=}"
CELL_MODE="${CELL_MODE:-build}"
case "${CELL_MODE}" in
    build|test) ;;
    *) echo "Unknown CELL_MODE '${CELL_MODE}', expected build or test"; exit 1 ;;
esac
TEST_TIMEOUT="${CELL_TEST_TIMEOUT:-300}"
LOG_TAIL_LINES=200
TEST_LOG_TAIL_LINES=50

REPORT_ROOT="${CELL_REPORT_DIR:-ci-report}"
case "${REPORT_ROOT}" in /*) ;; *) REPORT_ROOT="${REPO_ROOT}/${REPORT_ROOT}" ;; esac
REPORT_DIR="${REPORT_ROOT}/${CELL_ID}"
WORK_DIR="${REPORT_DIR}/work"
rm -rf "${REPORT_DIR}"
mkdir -p "${WORK_DIR}" || { echo "Cannot create the report directory ${REPORT_DIR}"; exit 1; }
CELL_STARTED_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
CELL_START=${SECONDS}

echo "Cell ${CELL_ID}: platform=${CELL_PLATFORM} compiler=${CELL_COMPILER} c++${CELL_CPP_STANDARD} ${CELL_BUILD_TYPE} mode=${CELL_MODE}"
echo "Products: ${CELL_PRODUCTS}"

# Debian based build images: install what the image lacks
need=""
command -v cmake >/dev/null || need="${need} cmake"
command -v ninja >/dev/null || need="${need} ninja-build"
[ "${CELL_COMPILER}" = "clang" ] && ! command -v clang++ >/dev/null && need="${need} clang"
if [ -n "${need}" ]; then
    echo "Installing:${need}"
    SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"
    ${SUDO} apt-get update -qq && ${SUDO} apt-get install -y -qq ${need} >/dev/null || { echo "Could not install${need}"; exit 1; }
fi
HAVE_PYTHON=false; command -v python3 >/dev/null && HAVE_PYTHON=true
HAVE_TIMEOUT=false; command -v timeout >/dev/null && HAVE_TIMEOUT=true
${HAVE_PYTHON} || echo "python3 is missing: the report holds exit codes only, no failed test case names"

COMPILER_ARGS=""
case "${CELL_COMPILER}" in
    gcc)   COMPILER_ARGS="-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++" ;;
    clang) COMPILER_ARGS="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++" ;;
esac
EXTRA_ARGS="$(eval echo "${CELL_CMAKE_ARGS}")"
GENERATOR=""; command -v ninja >/dev/null && GENERATOR="-G Ninja"
PARALLEL="--parallel"; [ -n "${CELL_BUILD_JOBS:-}" ] && PARALLEL="--parallel ${CELL_BUILD_JOBS}"
BUILD_ROOT="${CELL_BUILD_ROOT:-common/bin}"
case "${BUILD_ROOT}" in /*) ;; *) BUILD_ROOT="${REPO_ROOT}/${BUILD_ROOT}" ;; esac

# Test applications that are never run, one name per line, '#' starts a comment
SKIP_APPS=" "
if [ -f "${TEST_SKIP_FILE}" ]; then
    while read -r line; do
        line="${line%%#*}"; line="$(echo "${line}" | tr -d '[:space:]')"
        [ -n "${line}" ] && SKIP_APPS="${SKIP_APPS}${line} "
    done < <(sed 's/\r//' "${TEST_SKIP_FILE}")
fi

# Runs a command, shows its output and appends it to the log file given first; returns the command's exit code
run_logged()
{
    local log="$1"; shift
    "$@" 2>&1 | tee -a "${log}"
    return "${PIPESTATUS[0]}"
}

# Runs the test applications of one product build. Writes one line per application to <work>/<product>.tests:
# name, status, exit code, seconds, directory with its results (tab separated). Returns non-zero when any failed.
run_tests()
{
    local product="$1" build_abs="$2" log="$3"
    local records="${WORK_DIR}/${product}.tests"
    local any_failed=0 exe app name dir start rc status
    : > "${records}"
    if [ ! -d "${build_abs}/bin" ]; then
        echo "No bin directory in ${build_abs}: no test applications" | tee -a "${log}"
        return 0
    fi
    for exe in "${build_abs}"/bin/*_TestApp "${build_abs}"/bin/*_TestApp_d; do
        [ -f "${exe}" ] && [ -x "${exe}" ] || continue
        app="$(basename "${exe}")"
        name="${app%_d}"
        if [[ "${SKIP_APPS}" == *" ${name} "* ]]; then
            echo "=== test ${app}: skipped (test-skip.txt)" | tee -a "${log}"
            printf '%s\t%s\t%s\t%s\t%s\n' "${app}" "skipped" "0" "0" "" >> "${records}"
            continue
        fi
        dir="${WORK_DIR}/${product}.tests.d/${app}"
        mkdir -p "${dir}"
        echo "=== test ${app}" | tee -a "${log}"
        start=${SECONDS}
        if ${HAVE_TIMEOUT}; then
            ( cd "${dir}" && GUCEF_CONSOLE_LOGGER=none LD_LIBRARY_PATH="${build_abs}/lib:${build_abs}/bin${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}" \
                  timeout --kill-after=10 "${TEST_TIMEOUT}" "${exe}" ) > "${dir}/output.log" 2>&1 < /dev/null
        else
            ( cd "${dir}" && GUCEF_CONSOLE_LOGGER=none LD_LIBRARY_PATH="${build_abs}/lib:${build_abs}/bin${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}" \
                  "${exe}" ) > "${dir}/output.log" 2>&1 < /dev/null
        fi
        rc=$?
        cat "${dir}/output.log" >> "${log}"
        tail -n 20 "${dir}/output.log"
        status="ok"
        [ ${rc} -ne 0 ] && status="failed"
        [ ${rc} -eq 124 ] || [ ${rc} -eq 137 ] && status="timeout"
        [ "${status}" = "ok" ] || any_failed=1
        echo "=== test ${app}: ${status} (exit code ${rc}, $((SECONDS - start)) s)" | tee -a "${log}"
        printf '%s\t%s\t%s\t%s\t%s\n' "${app}" "${status}" "${rc}" "$((SECONDS - start))" "${dir}" >> "${records}"
    done
    return ${any_failed}
}

ok=0; failed=""
for product in ${CELL_PRODUCTS}; do
    start=${SECONDS}
    log="${WORK_DIR}/${product}.log"
    : > "${log}"
    project="projects/CMake/targets/${product}"
    if [ -f "${PRODUCTS_FILE}" ] && ${HAVE_PYTHON}; then
        project="$(python3 -c "import json,sys; p=[x for x in json.load(open(sys.argv[1]))['products'] if x['name']==sys.argv[2]]; print(p[0]['project'] if p else '')" "${PRODUCTS_FILE}" "${product}")"
        [ -n "${project}" ] || project="projects/CMake/targets/${product}"
    fi
    build_dir="${BUILD_ROOT}/${CELL_ID}/${product}"
    phase="configure"
    echo "::group::${product}"
    if [ ! -f "${project}/CMakeLists.txt" ]; then
        echo "No generated CMake project at ${project} for product ${product}" | tee -a "${log}"
        rc=2
    else
        run_logged "${log}" cmake -S "${project}" -B "${build_dir}" ${GENERATOR} -DCMAKE_BUILD_TYPE="${CELL_BUILD_TYPE}" \
                   -DCMAKE_CXX_STANDARD="${CELL_CPP_STANDARD}" ${COMPILER_ARGS} ${EXTRA_ARGS}
        rc=$?
        if [ ${rc} -eq 0 ]; then
            phase="build"
            run_logged "${log}" cmake --build "${build_dir}" ${PARALLEL}
            rc=$?
        fi
        if [ ${rc} -eq 0 ] && [ "${CELL_MODE}" = "test" ]; then
            phase="test"
            run_tests "${product}" "${build_dir}" "${log}"
            rc=$?
        fi
    fi
    echo "::endgroup::"
    if [ ${rc} -eq 0 ]; then
        status="ok"; ok=$((ok + 1))
    else
        status="failed"; failed="${failed} ${product}"
        echo "${product}: FAILED in ${phase} (exit code ${rc})"
    fi
    gzip -c "${log}" > "${REPORT_DIR}/${product}.log.gz"
    tail -n "${LOG_TAIL_LINES}" "${log}" > "${WORK_DIR}/${product}.tail"
    printf '%s\t%s\t%s\t%s\t%s\t%s\n' "${product}" "${status}" "${phase}" "${rc}" "$((SECONDS - start))" "${product}.log.gz" \
        >> "${WORK_DIR}/products.tsv"
done
touch "${WORK_DIR}/products.tsv"

# ---------------------------------------------------------------------------
# report.json
# ---------------------------------------------------------------------------
CELL_STATUS="ok"; [ -n "${failed}" ] && CELL_STATUS="failed"
export CELL_ID CELL_PLATFORM CELL_COMPILER CELL_CPP_STANDARD CELL_BUILD_TYPE CELL_MODE CELL_STATUS CELL_STARTED_UTC
export CELL_SECONDS=$((SECONDS - CELL_START)) REPORT_DIR WORK_DIR LOG_TAIL_LINES TEST_LOG_TAIL_LINES REPO_ROOT

json_escape()
{
    # stdin to one JSON string body: escapes, control characters dropped, lines joined with \n
    LC_ALL=C sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/\t/\\t/g' -e 's/\r//g' | LC_ALL=C tr -d '\000-\010\013\014\016-\037\177-\377' \
        | awk 'NR > 1 { printf "\\n" } { printf "%s", $0 }'
}

write_report_without_python()
{
    local first=1 name status phase rc secs logfile
    {
        printf '{\n  "schema": 1,\n  "cell": "%s",\n  "platform": "%s",\n  "compiler": "%s",\n  "cppStandard": "%s",\n' \
            "${CELL_ID}" "${CELL_PLATFORM}" "${CELL_COMPILER}" "${CELL_CPP_STANDARD}"
        printf '  "buildType": "%s",\n  "mode": "%s",\n  "runId": "%s",\n  "commit": "%s",\n  "ref": "%s",\n' \
            "${CELL_BUILD_TYPE}" "${CELL_MODE}" "${CI_RUN_ID:-}" "${GITHUB_SHA:-}" "${GITHUB_REF:-}"
        printf '  "workspace": "%s",\n' "${REPO_ROOT}"
        printf '  "status": "%s",\n  "started": "%s",\n  "seconds": %s,\n  "products": [' \
            "${CELL_STATUS}" "${CELL_STARTED_UTC}" "${CELL_SECONDS}"
        while IFS=$'\t' read -r name status phase rc secs logfile; do
            [ ${first} -eq 1 ] || printf ','
            first=0
            printf '\n    { "name": "%s", "status": "%s", "phase": "%s", "exitCode": %s, "seconds": %s, "logFile": "%s", "logTail": "%s"' \
                "${name}" "${status}" "${phase}" "${rc}" "${secs}" "${logfile}" "$(json_escape < "${WORK_DIR}/${name}.tail")"
            if [ -f "${WORK_DIR}/${name}.tests" ]; then
                printf ', "tests": ['
                local tfirst=1 app tstatus trc tsecs tdir
                while IFS=$'\t' read -r app tstatus trc tsecs tdir; do
                    [ ${tfirst} -eq 1 ] || printf ', '
                    tfirst=0
                    printf '{ "name": "%s", "status": "%s", "exitCode": %s, "seconds": %s, "failedCases": [] }' \
                        "${app}" "${tstatus}" "${trc}" "${tsecs}"
                done < "${WORK_DIR}/${name}.tests"
                printf ']'
            fi
            printf ' }'
        done < "${WORK_DIR}/products.tsv"
        printf '\n  ]\n}\n'
    } > "${REPORT_DIR}/report.json"
}

write_report_with_python()
{
    python3 - <<'PY'
import json, os, xml.etree.ElementTree as ET

env = os.environ
work = env["WORK_DIR"]

def tail(path, lines):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return "".join(f.readlines()[-lines:])
    except OSError:
        return ""

def parse_results(directory):
    """Counts and failed cases of the GUCEF test framework results (JUnit style) in one test directory."""
    result = {"tests": None, "failures": None, "skipped": None, "failedCases": []}
    files = sorted(f for f in os.listdir(directory) if f.endswith("_Results.xml")) if directory and os.path.isdir(directory) else []
    for name in files:
        try:
            root = ET.parse(os.path.join(directory, name)).getroot()
        except (ET.ParseError, OSError):
            continue
        for key in ("tests", "failures", "skipped"):
            try:
                result[key] = (result[key] or 0) + int(root.get(key, "0"))
            except ValueError:
                pass
        for case in root.iter("testcase"):
            failures = case.findall("failure")
            if failures:
                first = failures[0]
                result["failedCases"].append({
                    "suite": case.get("classname", ""),
                    "name": case.get("name", ""),
                    "message": ((first.get("message") or "") + " " + (first.text or "")).strip()[:500],
                })
    result["resultsFile"] = files[0] if files else ""
    return result

products = []
with open(os.path.join(work, "products.tsv"), encoding="utf-8") as f:
    for line in f:
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 6:
            continue
        name, status, phase, rc, secs, logfile = parts[:6]
        entry = {"name": name, "status": status, "phase": phase, "exitCode": int(rc), "seconds": int(secs),
                 "logFile": logfile, "logTail": tail(os.path.join(work, name + ".tail"), int(env["LOG_TAIL_LINES"]))}
        tests_file = os.path.join(work, name + ".tests")
        if os.path.isfile(tests_file):
            tests = []
            with open(tests_file, encoding="utf-8") as tf:
                for tline in tf:
                    t = tline.rstrip("\n").split("\t")
                    if len(t) < 5:
                        continue
                    app, tstatus, trc, tsecs, tdir = t[:5]
                    test = {"name": app, "status": tstatus, "exitCode": int(trc), "seconds": int(tsecs)}
                    if tstatus != "skipped":
                        test.update(parse_results(tdir))
                        # A run that exits 0 but records failed cases still failed
                        if test["status"] == "ok" and test["failedCases"]:
                            test["status"] = "failed"
                        if test["status"] != "ok":
                            test["logTail"] = tail(os.path.join(tdir, "output.log"), int(env["TEST_LOG_TAIL_LINES"]))
                    else:
                        test["failedCases"] = []
                    tests.append(test)
            entry["tests"] = tests
        products.append(entry)

report = {
    "schema": 1,
    "cell": env.get("CELL_ID", ""),
    "platform": env.get("CELL_PLATFORM", ""),
    "compiler": env.get("CELL_COMPILER", ""),
    "cppStandard": env.get("CELL_CPP_STANDARD", ""),
    "buildType": env.get("CELL_BUILD_TYPE", ""),
    "mode": env.get("CELL_MODE", ""),
    "runId": env.get("CI_RUN_ID", ""),
    "commit": env.get("GITHUB_SHA", ""),
    "ref": env.get("GITHUB_REF", ""),
    "workspace": env.get("REPO_ROOT", ""),
    "status": env.get("CELL_STATUS", ""),
    "started": env.get("CELL_STARTED_UTC", ""),
    "seconds": int(env.get("CELL_SECONDS", "0")),
    "products": products,
}
with open(os.path.join(env["REPORT_DIR"], "report.json"), "w", encoding="utf-8") as f:
    json.dump(report, f, indent=2)
    f.write("\n")
PY
}

if ${HAVE_PYTHON}; then
    write_report_with_python || { echo "WARNING: writing report.json with python3 failed, writing the plain report"; write_report_without_python; }
else
    write_report_without_python
fi
echo "Report: ${REPORT_DIR}/report.json"

# ---------------------------------------------------------------------------
# REPORT UPLOAD (replaceable)
# The one place that sends the report and the logs off the runner. Transport today: PUT to the Forgejo generic
# package registry, package ci-reports, version <run id>-<cell id>, authorised by CI_REPORT_TOKEN (repository
# secret) or the file named by CI_REPORT_TOKEN_FILE. The final method follows from a spike on the live Forgejo;
# replace this function only. The token is never printed; failures are warnings, never a failed cell.
# ---------------------------------------------------------------------------
upload_report()
{
    local base="${CI_REPORT_UPLOAD_URL:-}"
    local token="${CI_REPORT_TOKEN:-}"
    if [ -z "${token}" ] && [ -n "${CI_REPORT_TOKEN_FILE:-}" ] && [ -r "${CI_REPORT_TOKEN_FILE}" ]; then
        token="$(tr -d '\r\n' < "${CI_REPORT_TOKEN_FILE}")"
    fi
    if [ -z "${base}" ] || [ -z "${token}" ]; then
        echo "Report upload: skipped (CI_REPORT_UPLOAD_URL or CI_REPORT_TOKEN not set)"
        return 0
    fi
    command -v curl >/dev/null || { echo "WARNING: report upload skipped, curl is missing"; return 0; }

    local version="${CI_RUN_ID:-local}-${CELL_ID}"
    local url="${base%/}/ci-reports/${version}"
    local header="${WORK_DIR}/.upload-header"
    ( umask 077 && printf 'Authorization: token %s\n' "${token}" > "${header}" )
    local file name code uploaded=0 failures=0
    for file in "${REPORT_DIR}/report.json" "${REPORT_DIR}"/*.log.gz; do
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
upload_report

echo "Cell ${CELL_ID}: ${ok} built"
if [ -n "${failed}" ]; then
    echo "Cell ${CELL_ID}: FAILED:${failed}"
    exit 1
fi
exit 0
