#!/usr/bin/env python3
"""Plans a CI run: decides which products changed and which matrix cells need to build.

Inputs (environment):
  CI_MATRIX_DIR      directory with matrix.json and products.json (default projects/CI/matrix)
  CI_SINCE_HOURS     look at commits of the last N hours (default 25)
  CI_BEFORE          commit before the push (push events): changes are the diff to HEAD, overrides CI_SINCE_HOURS
  CI_FORCE           'true': build everything regardless of changes, 'false': plan from the changes,
                     empty: 'false', except for a manual run on a branch other than CI_PUSH_BRANCHES (see below)
  CI_CPP_STANDARDS   comma separated filter, for example 11,17 (empty: all)
  CI_TOOLCHAINS      comma separated toolchain id filter (empty: all enabled)
  CI_PRODUCTS        comma separated product name filter, for example GUCEF_tag_platform (empty: all)
  CI_MODE            'build' (default) or 'test': build, then run the test applications of the products
  CI_EVENT           event that started the run (push, schedule, workflow_dispatch)
  CI_REF             ref of the run, for example refs/heads/scratch/box-20261122T120000Z-abc1234
  CI_PUSH_BRANCHES   comma separated branches that build on push (default master,main). A manual run
                     (workflow_dispatch) on another branch, such as a scratch snapshot, builds everything
                     selected unless CI_FORCE is 'false'.
Output: 'cells' (JSON list for strategy.matrix; every cell gets the fields of matrix.json plus 'products'
(space separated) and 'mode') and 'any' (true/false), written to GITHUB_OUTPUT.
"""
import fnmatch
import json
import os
import subprocess
import sys


def env_list(name):
    return [v.strip() for v in os.environ.get(name, "").split(",") if v.strip()]


def changed_files_since_commit(before):
    """Files changed between a known previous commit and HEAD, None when the commit is not available."""
    if not before or set(before) == {"0"}:
        return None
    r = subprocess.run(["git", "diff", "--name-only", before, "HEAD"], capture_output=True, text=True)
    if r.returncode != 0:
        return None
    return sorted({line.strip() for line in r.stdout.splitlines() if line.strip()})


def changed_files(hours):
    out = subprocess.run(["git", "log", "--since=%s hours ago" % hours, "--name-only", "--pretty=format:"],
                         capture_output=True, text=True, check=True).stdout
    return sorted({line.strip() for line in out.splitlines() if line.strip()})


def read_globs(path):
    """Path globs of one product/platform list, normalized to repository style ('/' separators, no '//')."""
    if not os.path.isfile(path):
        return []
    with open(path, encoding="utf-8") as f:
        lines = [line.strip().replace("\\", "/") for line in f if line.strip()]
    return [l.replace("//", "/") for l in lines]


def touches(files, globs):
    return any(fnmatch.fnmatch(f, g) for f in files for g in globs)


def is_scratch_run():
    """A manual run on a branch that does not build on push: a scratch snapshot, not a published state."""
    if os.environ.get("CI_EVENT", "") != "workflow_dispatch":
        return False
    ref = os.environ.get("CI_REF", "")
    if not ref.startswith("refs/heads/"):
        return False
    push_branches = env_list("CI_PUSH_BRANCHES") or ["master", "main"]
    return ref[len("refs/heads/"):] not in push_branches


def fail(message):
    print("ERROR: %s" % message, file=sys.stderr)
    return 1


def main():
    matrix_dir = os.environ.get("CI_MATRIX_DIR", "projects/CI/matrix")
    with open(os.path.join(matrix_dir, "matrix.json"), encoding="utf-8") as f:
        matrix = json.load(f)
    with open(os.path.join(matrix_dir, "products.json"), encoding="utf-8") as f:
        products = json.load(f)["products"]

    force_input = os.environ.get("CI_FORCE", "").strip().lower()
    if force_input not in ("", "true", "false"):
        return fail("CI_FORCE must be true, false or empty, not '%s'" % force_input)
    scratch = is_scratch_run()
    force = force_input == "true" or (scratch and force_input != "false")

    mode = os.environ.get("CI_MODE", "").strip().lower() or "build"
    if mode not in ("build", "test"):
        return fail("CI_MODE must be build or test, not '%s'" % mode)

    known_products = sorted({p["name"] for p in products})
    only_products = env_list("CI_PRODUCTS")
    unknown = [n for n in only_products if n not in known_products]
    if unknown:
        return fail("unknown product(s) in CI_PRODUCTS: %s. Known products: %s"
                    % (", ".join(unknown), ", ".join(known_products)))
    if only_products:
        products = [p for p in products if p["name"] in only_products]
    # A push event knows the previous commit; scheduled and manual runs look at the recent commit window
    files = changed_files_since_commit(os.environ.get("CI_BEFORE", ""))
    if files is None:
        files = changed_files(os.environ.get("CI_SINCE_HOURS", "25"))
    # Files shared by every product (build system, CI definition) rebuild everything
    everything = force or any(f.startswith(("projects/CMake/", "projects/CI/", ".forgejo/")) for f in files)

    only_std = set(env_list("CI_CPP_STANDARDS"))
    only_tc = set(env_list("CI_TOOLCHAINS"))
    # Unknown filter values select nothing; say so instead of planning an empty run without a word
    for name in sorted(only_std - set(matrix.get("cppStandards", []))):
        print("WARNING: C++ standard '%s' in CI_CPP_STANDARDS is not in the matrix" % name, file=sys.stderr)
    known_tc = {t["id"]: t.get("enabled", True) for t in matrix.get("toolchains", [])}
    for name in sorted(only_tc):
        if name not in known_tc:
            print("WARNING: toolchain '%s' in CI_TOOLCHAINS is not in the matrix" % name, file=sys.stderr)
        elif not known_tc[name]:
            print("WARNING: toolchain '%s' in CI_TOOLCHAINS is disabled (no runner)" % name, file=sys.stderr)

    # A product depends on the same sources on every platform: the lists of all its platforms are combined,
    # since the list of a platform can be empty when its module definitions come from another platform
    globs_by_name = {}
    for p in products:
        globs_by_name.setdefault(p["name"], set()).update(read_globs(p["globpaths"]))

    affected = []
    for p in products:
        globs = globs_by_name[p["name"]]
        # No known paths: cannot tell what it depends on, so any change rebuilds it
        if everything or not globs or touches(files, sorted(globs)):
            affected.append(p)
    if not files and not force:
        affected = []
    print("changed files: %d, affected products: %d of %d, everything=%s, scratch=%s, mode=%s"
          % (len(files), len(affected), len(products), everything, scratch, mode))

    cells = []
    for cell in matrix["cells"]:
        if only_std and cell["cppStandard"] not in only_std:
            continue
        if only_tc and cell["toolchain"] not in only_tc:
            continue
        names = [p["name"] for p in affected
                 if p["platform"] in (cell["platform"], "all") and p["minCppStandardYear"] <= cell["cppStandardYear"]]
        if names:
            c = dict(cell)
            c["products"] = " ".join(sorted(set(names)))
            c["mode"] = mode
            cells.append(c)

    print("cells to build: %d" % len(cells))
    for c in cells:
        print("  %s: %d products" % (c["id"], len(c["products"].split())))

    out = os.environ.get("GITHUB_OUTPUT")
    lines = ["cells=%s" % json.dumps(cells, separators=(",", ":")), "any=%s" % ("true" if cells else "false")]
    if out:
        with open(out, "a", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
    else:
        print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
