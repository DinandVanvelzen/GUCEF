#!/bin/bash
# ---------------------------------------------------------------------------
# Runs a coding task with Claude Code (headless) against the local LLM server
# instead of the Anthropic API. The agent may read and edit files in the
# working directory; it cannot run shell commands. Review the result with
# "git diff" afterwards.
#
# Usage: local-agent.sh [-C <dir>] "<task description>"
#
# Environment:
#   LOCAL_LLM_URL      server base URL (default http://localhost:8080)
#   LOCAL_LLM_API_KEY  API key, if the server requires one
#   LOCAL_LLM_MODEL    model name (default local-qwen)
# ---------------------------------------------------------------------------

WORK_DIR="$(pwd)"
if [ "$1" = "-C" ]; then
    WORK_DIR="$2"
    shift 2
fi
TASK="$1"
if [ -z "${TASK}" ]; then
    echo "Usage: $(basename "$0") [-C <dir>] \"<task description>\""
    exit 1
fi
if ! command -v claude > /dev/null 2>&1; then
    echo "ERROR: Claude Code CLI ('claude') not found in PATH."
    exit 1
fi

LOCAL_LLM_URL="${LOCAL_LLM_URL:-http://localhost:8080}"
LOCAL_LLM_MODEL="${LOCAL_LLM_MODEL:-local-qwen}"

if ! curl -sf --max-time 5 "${LOCAL_LLM_URL}/health" > /dev/null; then
    echo "ERROR: local LLM server not reachable at ${LOCAL_LLM_URL}"
    exit 1
fi

# Context window per request slot, read from the server. Claude Code does not
# know the window of a custom model and otherwise assumes 200k tokens.
# Its auto-compact window has a minimum of 100000 tokens.
MIN_CTX_SIZE=100000
AUTH_HEADER=()
[ -n "${LOCAL_LLM_API_KEY}" ] && AUTH_HEADER=(-H "Authorization: Bearer ${LOCAL_LLM_API_KEY}")
CTX_SIZE="$(curl -sf --max-time 5 "${AUTH_HEADER[@]}" "${LOCAL_LLM_URL}/props" | grep -o '"n_ctx":[0-9]*' | head -n 1 | cut -d: -f2)"
CTX_ENV=()
if [ -z "${CTX_SIZE}" ]; then
    echo "WARNING: could not read the context size from ${LOCAL_LLM_URL}/props; long tasks may exceed it."
elif [ "${CTX_SIZE}" -lt "${MIN_CTX_SIZE}" ]; then
    echo "WARNING: server context size ${CTX_SIZE} is below ${MIN_CTX_SIZE}; Claude Code cannot compact in time and long tasks will fail. Raise LLM_CTX_SIZE on the server."
else
    CTX_ENV=(CLAUDE_CODE_AUTO_COMPACT_WINDOW="${CTX_SIZE}")
fi

cd "${WORK_DIR}" || exit 1

# CLAUDECODE is unset so the agent can start from within another Claude Code session
env -u CLAUDECODE \
    "${CTX_ENV[@]}" \
    ANTHROPIC_BASE_URL="${LOCAL_LLM_URL}" \
    ANTHROPIC_AUTH_TOKEN="${LOCAL_LLM_API_KEY:-local}" \
    ANTHROPIC_API_KEY="" \
    ANTHROPIC_MODEL="${LOCAL_LLM_MODEL}" \
    ANTHROPIC_DEFAULT_OPUS_MODEL="${LOCAL_LLM_MODEL}" \
    ANTHROPIC_DEFAULT_SONNET_MODEL="${LOCAL_LLM_MODEL}" \
    ANTHROPIC_DEFAULT_HAIKU_MODEL="${LOCAL_LLM_MODEL}" \
    CLAUDE_CODE_SUBAGENT_MODEL="${LOCAL_LLM_MODEL}" \
    CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1 \
    claude -p "${TASK}" \
        --permission-mode acceptEdits \
        --allowedTools "Read,Edit,Write,Glob,Grep" \
        --disallowedTools "Bash,WebFetch,WebSearch,Agent" \
        --strict-mcp-config
