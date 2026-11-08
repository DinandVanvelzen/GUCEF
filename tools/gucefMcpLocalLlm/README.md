# gucefMcpLocalLlm

An MCP server built on GUCEF's MCP framework. It lets Claude Code hand work to local LLM servers (llama.cpp
`llama-server`, or any other OpenAI-compatible server), which saves Anthropic tokens. On the nodes of the k3s home
lab it runs as the systemd service `gucef-mcp`, which every Claude session of the node uses over HTTP and sessions
on other machines over HTTPS: see [projects/k3s/llm/mcp/README.md](../../projects/k3s/llm/mcp/README.md). Every tool
call of Claude re-reads the whole conversation, so the tools are built to do a whole job per call:


| Tool | What it does |
|---|---|
| `delegate_to_local_llm(repo_dir, task)` | Runs `local-agent.sh` (`local-agent.ps1` on Windows) on `repo_dir` and returns the agent's summary plus the change: the diff itself when it has at most `--max-diff-lines` lines, `git diff --stat` otherwise |
| `delegate_batch(repo_dir, tasks[], max_parallel?, model?)` | Runs several such tasks in one call, in parallel on all free LLM servers, and returns per task ok/failed with a summary, then the combined change |
| `run_and_digest(command, working_dir, timeout_seconds?)` | Runs an allowed build or test command (not through a shell) and returns the distinct errors and warnings with `file:line`, failed targets, failed tests and the test summary. Recognises GCC, Clang, MSVC, linkers, CMake, ninja, make, CTest, the GUCEF test framework and exceptions; when it recognises nothing in a failure, the local LLM explains the end of the output. The full output is kept in a log file. Only offered with `--allow-command` |
| `ask_local_llm(question, paths[], base_dir?, deep?, model?)` | A local LLM reads the files (`path:START-END` for a line range) and answers with `file:line` citations. `deep` lets the model think first |
| `local_llm_status()` | Reports every configured LLM server: reachable, model, context size, idle request slots; and the models that are reachable |

`delegate_to_local_llm` takes an optional `model` as well. A GPU holds one model, so different machines usually
serve different models. Each server reports its model name (`LLM_MODEL_ALIAS`); a call that names a model goes only to
servers running it (the name is matched without case) and is refused, with the models that are reachable, when none
is. A call without a model goes to any server, preferring the `--model` one when that is reachable. Requests and the
delegated agent use the model name of the server they go to. The always visible instructions of the MCP server list
the models reachable when it started; `local_llm_status` lists them at any time.

None of the servers needs to be on the machine the MCP server runs on. Without any server only `run_and_digest` is
offered.

The order of preference of the LLM servers that run the model:

1. servers on the machine the MCP server runs on: it recognises its own addresses, so these are available for as
   long as it runs. That includes the server of an ephemeral node when Claude Code runs on that node.
2. the other `--endpoint` servers, in the order given
3. `--ephemeral-endpoint` servers of other machines: used when the others are busy or down

Each task goes to the first server that is reachable, has a context of at least 100000 tokens and has an idle
request slot that no other task of the MCP server occupies; when all are busy it goes to the first usable one, which
queues it. Several tasks run at the same time on different servers. Cancelling a call stops what it started.

A server that does not answer is not checked again for 30 seconds, so a machine that is off costs no time; once it is
on again it is used. When a server goes away during work, `ask_local_llm` and the failure explanation of
`run_and_digest` move to the next server; a delegated task that changed nothing yet starts again on the next server,
otherwise its result says the server went away.

The same registration works on every machine. For a cluster server node `<node-ip>` and an ephemeral workstation
`<workstation>`: on the workstation its own server comes first, elsewhere it is used after the cluster.

On a machine that is not a node of the cluster it can run as the stdio server of one Claude session. Build (from
the root of a GUCEF checkout; builds the tool, the libraries it needs and its tests):

```bash
cmake -S projects/CMake/targets/GUCEF_exe_gucefMcpLocalLlm_TestApp -B build/gucefMcpLocalLlm -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/gucefMcpLocalLlm
# tests; those that need an LLM server and the agent are skipped without them
LOCAL_LLM_URL=http://<node-ip>:8080 LOCAL_AGENT_SCRIPT=$PWD/projects/k3s/llm/clients/local-agent.sh \
    build/gucefMcpLocalLlm/bin/gucefMcpLocalLlm_TestApp
```

Register with Claude Code for all projects (`--endpoint`, `--ephemeral-endpoint` and `--allow-command` repeatable):

```bash
claude mcp add local-llm --scope user -- <build>/bin/gucefMcpLocalLlm \
    --endpoint http://<node-ip>:8080 --ephemeral-endpoint http://<workstation>:8080 \
    --agent-script <gucef>/projects/k3s/llm/clients/local-agent.sh \
    --allow-command "cmake --build *" --allow-command "ctest *" --allow-command "ninja *"
```

| Option | Environment | Default |
|---|---|---|
| `--endpoint URL` | `LOCAL_LLM_URLS` (comma separated), `LOCAL_LLM_URL` | none |
| `--ephemeral-endpoint URL` | `LOCAL_LLM_EPHEMERAL_URLS` (comma separated) | none |
| `--agent-script PATH` | `LOCAL_AGENT_SCRIPT` | `local-agent.sh` / `local-agent.ps1` next to the executable |
| `--model NAME` | `LOCAL_LLM_MODEL` | none: any model; preferred for calls that name no model |
| `--timeout SECONDS` | `LOCAL_LLM_TIMEOUT` | 900, per delegated task |
| `--min-context N` | | 100000 |
| `--max-diff-lines N` | | 150; 0: only `git diff --stat` |
| `--allow-command PATTERN` | | none; `*` and `?` are wildcards matched against the whole command line |
| `--command-timeout SECONDS` | | 3600 |
| `--log-dir PATH` | | `gucefMcpLocalLlm` in the temporary directory |
| `--log-file PATH` | | log to standard error only |
| `--endpoints-file PATH` | `LOCAL_LLM_ENDPOINTS_FILE` | none; more servers, one URL per line (`ephemeral URL` for one that is not always on), read again when the file changes |
| `--listen ADDR:PORT` | | none: stdio. HTTP (streamable HTTP, `/mcp`) for any number of clients, loopback addresses only |
| `--listen-tls ADDR:PORT` | | none. HTTPS for clients on other machines, with `--tls-cert` and `--tls-key` (PEM; read again when they change) |
| `--token-file PATH` | | required with `--listen` and `--listen-tls`: the bearer token clients send, made for the owner when missing |
| `--allowed-host NAME` | | names clients of `--listen-tls` use for the machine (Host); `localhost` and `127.0.0.1` always |
| | `LOCAL_LLM_API_KEY` | API key for the LLM servers, when they require one |

`run_and_digest` runs a command without a shell: pipes, redirection and `;` are passed to the program as arguments.
A pattern such as `cmake --build *` therefore only ever runs `cmake`. The delegated agent can read and edit files
only. Optionally add to a user-level `CLAUDE.md`:

```markdown
## Local LLM tools
- Review what delegate_to_local_llm and delegate_batch changed before continuing; fix or revert mistakes.
- Build and test with run_and_digest; grep its log file for details instead of running the command again.
```

