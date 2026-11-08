# ---------------------------------------------------------------------------
# Runs a coding task with Claude Code (headless) against the local LLM server
# instead of the Anthropic API. The agent may read and edit files in the
# working directory; it cannot run shell commands. Review the result with
# "git diff" afterwards.
#
# Usage: local-agent.ps1 [-Dir <dir>] "<task description>"
#
# Environment:
#   LOCAL_LLM_URL      server base URL (default http://localhost:8080)
#   LOCAL_LLM_API_KEY  API key, if the server requires one
#   LOCAL_LLM_MODEL    model name (default: the model the server reports)
# ---------------------------------------------------------------------------
param(
    [Parameter(Mandatory = $true, Position = 0)][string]$Task,
    [string]$Dir = (Get-Location).Path
)

if (-not (Get-Command claude -ErrorAction SilentlyContinue)) {
    Write-Error "Claude Code CLI ('claude') not found in PATH."
    exit 1
}

$url   = if ($env:LOCAL_LLM_URL)     { $env:LOCAL_LLM_URL }     else { "http://localhost:8080" }
$model = $env:LOCAL_LLM_MODEL
$key   = if ($env:LOCAL_LLM_API_KEY) { $env:LOCAL_LLM_API_KEY } else { "local" }

try {
    Invoke-WebRequest -Uri "$url/health" -TimeoutSec 5 -UseBasicParsing | Out-Null
} catch {
    Write-Error "Local LLM server not reachable at $url"
    exit 1
}

$vars = @{
    ANTHROPIC_BASE_URL                       = $url
    ANTHROPIC_AUTH_TOKEN                     = $key
    ANTHROPIC_API_KEY                        = ""
    ANTHROPIC_MODEL                          = $model
    ANTHROPIC_DEFAULT_OPUS_MODEL             = $model
    ANTHROPIC_DEFAULT_SONNET_MODEL           = $model
    ANTHROPIC_DEFAULT_HAIKU_MODEL            = $model
    CLAUDE_CODE_SUBAGENT_MODEL               = $model
    CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC = "1"
}

# Context window per request slot, read from the server. Claude Code does not
# know the window of a custom model and otherwise assumes 200k tokens.
# Its auto-compact window has a minimum of 100000 tokens.
$minCtx = 100000
try {
    $props = Invoke-RestMethod -Uri "$url/props" -TimeoutSec 5 -Headers @{ Authorization = "Bearer $key" }
    $ctx = [int]$props.default_generation_settings.n_ctx
    if (-not $model -and $props.model_alias) { $model = [string]$props.model_alias }
    if ($ctx -ge $minCtx) {
        $vars["CLAUDE_CODE_AUTO_COMPACT_WINDOW"] = "$ctx"
    } else {
        Write-Warning "Server context size $ctx is below $minCtx; Claude Code cannot compact in time and long tasks will fail. Raise LLM_CTX_SIZE on the server."
    }
} catch {
    Write-Warning "Could not read the context size from $url/props; long tasks may exceed it."
}
if (-not $model) { $model = "local" }
foreach ($name in @("ANTHROPIC_MODEL", "ANTHROPIC_DEFAULT_OPUS_MODEL", "ANTHROPIC_DEFAULT_SONNET_MODEL",
                    "ANTHROPIC_DEFAULT_HAIKU_MODEL", "CLAUDE_CODE_SUBAGENT_MODEL")) {
    $vars[$name] = $model
}
$saved = @{}
foreach ($name in $vars.Keys) {
    $saved[$name] = [Environment]::GetEnvironmentVariable($name, "Process")
    [Environment]::SetEnvironmentVariable($name, $vars[$name], "Process")
}
# CLAUDECODE is cleared so the agent can start from within another Claude Code session
$savedClaudeCode = $env:CLAUDECODE
Remove-Item Env:CLAUDECODE -ErrorAction SilentlyContinue

Push-Location $Dir
try {
    claude -p $Task `
        --permission-mode acceptEdits `
        --tools "Read,Edit,Write,Glob,Grep" `
        --allowedTools "Read,Edit,Write,Glob,Grep" `
        --disallowedTools "Bash,WebFetch,WebSearch,Agent" `
        --strict-mcp-config
    $exitCode = $LASTEXITCODE
} finally {
    Pop-Location
    foreach ($name in $saved.Keys) {
        [Environment]::SetEnvironmentVariable($name, $saved[$name], "Process")
    }
    if ($savedClaudeCode) { $env:CLAUDECODE = $savedClaudeCode }
}
exit $exitCode
