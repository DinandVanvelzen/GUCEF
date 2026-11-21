# gucefIrcHub

LLM bots for an IRC server: each configured model is a member of the server with an account of its own. Built for
the family IRC server of `projects/k3s/irc`, where `setup.sh` configures, builds and deploys it.

## What a bot does

- In its room (for example `#claude`) it answers every message, except one addressed to someone else (`athena: ...`).
- In the shared channels (for example `#family`) it answers messages addressed to it: `claude: ...`, `claude, ...`,
  `@claude ...`, or `@claude` further on in the message.
- In a private message it answers everything.
- Each channel and each private chat is a conversation with its own memory of the last messages (20 by default).
  In a channel the model sees who wrote each message. The memory is lost when the hub restarts.
- A message that comes in while the model is still answering in the same conversation is answered next, together
  with any others that came in meanwhile.
- Commands, as a message of their own (in a shared channel addressed to the bot): `!help`, `!reset` (forget the
  conversation), `!model`, `!usage` (tokens used today).
- Answers are split into IRC lines; in a channel a long answer is cut off after 15 lines (60 in a private chat).
- While the model works, the bot shows the IRCv3 typing indicator, which many clients display.
- A bot never answers another bot of the hub, nor messages replayed from the history.
- Optional per-person daily token limit per bot (UTC day).

What people write and what the models answer is not logged; the log has one line per answer: bot, person,
channel or private chat, tokens.

## Vendors

| vendor              | API                                    | example `baseUrl`                                          |
|---------------------|----------------------------------------|------------------------------------------------------------|
| `llamacpp`          | llama.cpp llama-server (OpenAI style)  | `http://llama-server.llm.svc.cluster.local:8080/v1`        |
| `anthropic`         | Anthropic Messages API                 | `https://api.anthropic.com/v1`                             |
| `openai`            | OpenAI chat completions                | `https://api.openai.com/v1`                                |
| `openai-compatible` | OpenAI style APIs of other vendors     | `https://generativelanguage.googleapis.com/v1beta/openai`, `https://api.x.ai/v1` |

For Anthropic the hub sends `anthropic-version: 2026-10-21`, the `effort` of the bot in `output_config`, and
`fallbacks: "default"` (beta `server-side-fallback-2026-07-01`): a request a safety classifier declines is answered
by a fallback model the API chooses. A request that is still declined gets the answer "Sorry, I can't help with
that one."

## Configuration

`gucefIrcHub --config /etc/gucef-irc-hub/hub.json` (or the environment variable `IRC_HUB_CONFIG`). Secrets are not
in the file: the attributes ending in `Env` name the environment variables that hold them.

```json
{
  "server": "ergo.irc.svc.cluster.local", "port": 6697,
  "caFile": "/irc-tls/ca.crt", "tlsServerName": "a name in the certificate of the server",
  "operName": "bot", "operPasswordEnv": "IRC_BOT_OPER_PASSWORD",
  "threads": 8,
  "bots": [
    { "nick": "qwen", "passwordEnv": "IRC_BOT_PASSWORD_qwen", "displayName": "Qwen, running on our own server",
      "room": "#qwen", "channels": "#family", "vendor": "llamacpp",
      "baseUrl": "http://llama-server.llm.svc.cluster.local:8080/v1", "model": "",
      "maxTokens": 2048, "dailyTokens": 0, "systemPrompt": "" }
  ]
}
```

Per bot also: `apiKeyEnv`, `effort` (anthropic), `thinking` (llamacpp), `timeoutSecs` (300), `maxHistoryTurns` (20),
`maxChannelReplyLines` (15), `maxPrivateReplyLines` (60). An empty `systemPrompt` gives the default: plain text
without Markdown, short answers, suitable for all ages. A bot without a password, or without an API key for a cloud
vendor, is left out.

- Each bot logs in with SASL PLAIN as the account `nick`, sets user mode `+B` (bot) and, with `operName`, becomes
  an operator of a class without flood limits (Ergo capability `nofakelag`), so a long answer arrives at once.
- TLS: the certificate of the server is verified against `caFile`, by the name `tlsServerName`.
- The requests to the models run on a thread pool of `threads` threads; each request holds a thread for as long as
  the model takes.
- A lost connection is made again after 5 s, then up to a minute between attempts.
- Logging: lines on stderr (`GUCEF_CONSOLE_LOGGER=stderr` unless set otherwise).
