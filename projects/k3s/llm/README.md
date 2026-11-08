# Local LLM server

[llama.cpp](https://github.com/ggml-org/llama.cpp) `llama-server` serving a Qwen model on the LAN.
Uses the NVIDIA GPU when available (see `../nvidia`), otherwise the CPU.

## Install

```bash
./setup.sh
```

On first run `config.env` is created with all settings commented out. Edit it and re-run `setup.sh` to apply changes.
The first start downloads the model into the `llama-server-models` PVC; later starts reuse it.

## Model selection

Unless `LLM_MODEL_HF` is set, the model is chosen from the hardware:

| Condition | Model | Size |
|---|---|---|
| LLM pod memory limit >= 32 GiB | `unsloth/Qwen3.6-35B-A3B-GGUF:Q4_K_M` | 22 GB |
| VRAM >= 7 GB or memory limit >= 12 GiB | `unsloth/Qwen3.5-9B-GGUF:Q4_K_M` | 5.7 GB |
| otherwise | `unsloth/Qwen3.5-4B-GGUF:Q4_K_M` | 2.7 GB |

The memory limit defaults to 50% of the node's allocatable memory.
`llama-server --fit` places as many layers as fit on the GPU, keeping `LLM_VRAM_RESERVE_MIB` free. For mixture-of-experts
models (Qwen3.6-35B-A3B) the expert weights that do not fit stay in system RAM.

## API

`setup.sh` prints the URL. The service listens on `LLM_PORT` (default 8080) of the node IP.

| API | Base URL | Endpoints |
|---|---|---|
| OpenAI compatible | `http://<node-ip>:8080/v1` | `/chat/completions`, `/completions`, `/models`, `/embeddings` |
| Anthropic compatible | `http://<node-ip>:8080` | `/v1/messages`, `/v1/messages/count_tokens` |
| Health / Prometheus metrics | `http://<node-ip>:8080` | `/health`, `/metrics` |

Model name: `LLM_MODEL_ALIAS` (default `local-qwen`).

Authentication: set `LLM_API_KEY` in `config.env`. Clients send `Authorization: Bearer <key>` or `x-api-key: <key>`.
Without a key the API is open to anyone who can reach the port: keep it LAN-only, do not port-forward it.

## Resources and storage

- CPU and memory limits default to 50% of the node (`LLM_CPU_LIMIT`, `LLM_MEMORY_LIMIT`).
- PriorityClass `gucef-low-priority`: under resource pressure the LLM pod is evicted before other services.
- Model files are re-downloadable and stored on the scratch class `longhorn-jbod`. If that class does not exist,
  the cluster default StorageClass is used. Override with `LLM_STORAGE_CLASS`.

## Clients

All clients need the server URL, plus the API key if one is set.

### Claude Code against the local model: `clients/local-agent.sh` / `clients/local-agent.ps1`

Runs a task headless with Claude Code using the local model. The agent can read and edit files, not run commands.
The script reads the server's context size from `/props` and passes it to Claude Code as
`CLAUDE_CODE_AUTO_COMPACT_WINDOW`. Claude Code requires at least 100000 tokens, so the server's `LLM_CTX_SIZE`
must be 100000 or more (default 131072).

```bash
export LOCAL_LLM_URL=http://<node-ip>:8080
export LOCAL_LLM_API_KEY=<key>          # if set on the server
./clients/local-agent.sh -C ~/src/myrepo "Fix the indentation in src/foo.cpp to match the rest of the file"
git -C ~/src/myrepo diff
```

```powershell
$env:LOCAL_LLM_URL = "http://<node-ip>:8080"
.\clients\local-agent.ps1 -Dir C:\src\myrepo "Fix the indentation in src\foo.cpp to match the rest of the file"
```

### Delegation from Claude Code

Add to a user-level `CLAUDE.md` on the client machine:

```markdown
## Local LLM delegation
- Simple, mechanical edits (formatting, include guards, renames, CMake file lists) can be delegated to the
  local model: `<path>/local-agent.sh -C <repo> "<precise task>"` (Windows: `local-agent.ps1 -Dir <repo> "<task>"`).
- Give one precise task per call, naming the files.
- Always review `git diff` afterwards and fix or revert mistakes.
```

### OpenAI-compatible tools (opencode, Continue, aider, Open WebUI, Home Assistant)

Configure an OpenAI-compatible provider with base URL `http://<node-ip>:8080/v1`, model `local-qwen` and the API key (any
non-empty value when no key is set). Example `opencode.json`:

```json
{
  "$schema": "https://opencode.ai/config.json",
  "provider": {
    "local": {
      "npm": "@ai-sdk/openai-compatible",
      "name": "Local LLM",
      "options": { "baseURL": "http://<node-ip>:8080/v1", "apiKey": "{env:LOCAL_LLM_API_KEY}" },
      "models": { "local-qwen": { "name": "Local Qwen" } }
    }
  },
  "model": "local/local-qwen"
}
```

## Operations

```bash
kubectl --kubeconfig /etc/rancher/k3s/k3s.yaml logs -f -n llm deploy/llama-server   # load / download progress
curl http://<node-ip>:8080/v1/models
./uninstall.sh                  # keeps downloaded models
./uninstall.sh --delete-models  # removes everything
```

To switch models, set `LLM_MODEL_HF` in `config.env` and re-run `setup.sh`. Downloaded models stay in the PVC.

## Benchmark checklist

Compare candidate models on representative tasks before changing the default:

1. Run 5–6 real tasks per model via `local-agent.sh`: formatting fixes, include-guard fixes, a CMake file-list update, a rename.
2. Record per task: success (diff correct without manual fixes), wall time.
3. Record tokens/s and VRAM use: the `llama-server` log prints timings per request, and `nvidia-smi` shows VRAM.
