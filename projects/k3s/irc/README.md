# IRC server

[Ergo](https://ergo.chat) (`ghcr.io/ergochat/ergo:v2.19.1`, pinned by digest) for the family: chat between people
and with LLM bots (gucefIrcHub). Standard IRC apps connect to it, such as Goguma or Revolution IRC on Android.

- Namespace `irc`, Deployment `ergo`. In-cluster address: `ergo.irc.svc.cluster.local:6697` (TLS).
- Clients connect to `IRC_HOST:IRC_LAN_PORT` (default port 32004). The port is a host port of the node
  `IRC_NODE` (default: the first node that is not ephemeral). A LoadBalancer service is not used, because
  k3s ServiceLB replaces the client address and Ergo limits connections per client address.
- TLS only. The certificate is self-signed (cert-manager Issuer `irc-selfsigned`) and valid for 10 years;
  apps ask once to accept it. `setup.sh` prints its SHA-256 fingerprint.
- Private server: every connection logs in with SASL; nobody can register an account from a client.
- Built-in bouncer: accounts stay in their channels while their devices are offline and get the missed
  messages on reconnect. The nickname is always the account name.
- Chat history: database `ergo` in PostgreSQL (`my-postgres`), kept `IRC_HISTORY_EXPIRE` (default 52 weeks).
  The nightly `pg-dumpall` of `../backup` includes it.
- Accounts and channel registrations: PVC `ergo-data-pvc`.

## Setup

Part of the main k3s setup (`../setup.sh`, `IRC=enabled`). Needs Sealed Secrets, cert-manager and PostgreSQL;
`./setup.sh` checks for them and names the script to run when one is missing.

1. Optional: `cp config.env.example config.env` and set `IRC_ACCOUNTS` (the family's account names) and other
   settings. Without it there is one account: the administrator, named after the user running the script.
2. `./setup.sh` (idempotent). It generates the passwords, creates missing accounts, registers the channels in
   `IRC_CHANNELS` and prints how to connect.
3. Passwords: `secrets.env` (gitignored), copied to `~/.config/gucef-k3s/irc-secrets.env`.
   Account `<name>` logs in with `IRC_PASSWORD_<name>`. Generated account passwords are three words and a
   number (`maple-tiger-river-42`), easy to type on a phone. Ergo allows 3 failed logins per account per minute.
   The summary of `setup.sh` prints the login of every account it created or gave a password.

To add a person: add the name to `IRC_ACCOUNTS` and run `./setup.sh`. An existing account keeps its password.

- `./setup.sh --new-password <name>`: generates a new password for the account and prints it.
- `./setup.sh --reset-password <name>`: sets the password to the value in `secrets.env`; edit that value first
  to choose one.
- Anyone can change their own password from the app: `/msg NickServ PASSWD <current> <new> <new>`.

## Channels and rights

Channels in `IRC_CHANNELS` (default `#family`) are registered and invite-only (`+i`).

- Every account may join them: it has the ChanServ AMODE `+v`, which lets it in without an invite.
- The administrator (`IRC_ADMIN_ACCOUNT`) has AMODE `+q` (owner) and can kick, invite and set modes.
- The administrator is also a server operator: `/OPER admin <IRC_ADMIN_OPER_PASSWORD>`.
- The account `steward` registers the channels; it is the founder.

## Bots

`gucefIrcHub` (`tools/gucefIrcHub`, Deployment `irc-hub`) puts each model on the server as a bot with an account of
its own:

| bot      | model                                         | runs when                                         |
|----------|-----------------------------------------------|---------------------------------------------------|
| `qwen`   | the local LLM server (`../llm`, llama-server) | it is installed                                   |
| `claude` | Anthropic, `claude-opus-5-5`, effort low      | `IRC_ANTHROPIC_API_KEY` in `secrets.env` has a value |
| `gpt`    | OpenAI, `gpt-5`                               | `IRC_OPENAI_API_KEY` has a value                  |
| `gemini` | Google, `gemini-2.5-flash`                    | `IRC_GEMINI_API_KEY` has a value                  |
| `grok`   | xAI, `grok-4`                                 | `IRC_XAI_API_KEY` has a value                     |

- Each bot has a room, `#<bot>`, where it answers everything, and is in the shared channels (`#family`), where it
  answers what is addressed to it: `claude: ...`, `claude, ...` or `@claude`.
- A private message to a bot is a one-on-one chat.
- Each channel and private chat has its own memory of the last 20 messages. `!reset` makes the bot forget it;
  `!help`, `!model` and `!usage` tell more.
- The cloud bots allow each person `IRC_BOT_DAILY_TOKENS` tokens per day (default 200000); `qwen` has no limit.
- The rooms are invite-only like the other channels; every account may join them.
- What people write is not logged; the hub logs one line per answer (bot, person, channel, tokens).

To turn on a cloud bot: put its API key in `secrets.env` (the empty lines are there) and run `./setup.sh`. Models,
names, rooms and limits: the `IRC_BOT_*` settings in `config.env.example`. Behaviour and vendors in detail:
`tools/gucefIrcHub/README.md`.

How it runs:
- `hub/build-image.sh` (run by `setup.sh`) builds the hub from this checkout and makes the image
  `gucef-irc-hub:<version>`. There is no registry, so it imports the image into k3s on this machine with
  `sudo k3s ctr images import`, once per version: run `setup.sh` on `IRC_NODE`, and expect a sudo prompt when the
  hub changed.
- The configuration is the ConfigMap `irc-hub-config` (no secrets); the bot passwords and API keys are the Secret
  `irc-hub-secrets`, made from `secrets.env` on every run. A change restarts the hub.
- The bots verify the server's certificate (`ca.crt` of `irc-lan-tls`) and log in with SASL. They are operators of
  the class `bot`, which only lifts the flood limits, so a long answer arrives at once.

## Connect from Android

Any IRC app (Goguma, Revolution IRC, HexDroid): add a server with the address and port from the `setup.sh`
summary and TLS on. Set the nickname to the account name and turn on SASL (PLAIN) with the account name and
password. Apps without SASL settings can use the server password `<account>:<password>`. Without a login the
server refuses the app ("You must log in with SASL"). Accept the certificate when asked; compare the fingerprint
with the one `setup.sh` printed. Join `#family`.

## How it works

- `ergo/ircd.yaml.tmpl` is rendered with the values of `config.env` and the Secret `irc-credentials` into the
  Secret `ergo-config`. The container `reloader` asks Ergo to reload (`/v1/rehash` of the Ergo HTTP API) when the
  configuration or the certificate changes, so a changed setting needs no restart.
- The HTTP API and a plaintext listener exist only on the pod's loopback address. `setup.sh` reaches them through
  `kubectl port-forward`: accounts through the API, channels through an IRC session as `steward`.
- `create-sealed-secret.sh` (run by `setup.sh`) adds missing values to `secrets.env` and reseals
  `sealed-secret.yaml` when it changed. The operator password hashes are made by `ergo genpasswd` in a short-lived pod.

## Uninstall

`./uninstall.sh` removes the server and keeps the accounts, channels and history.
`./uninstall.sh --wipe-data` also deletes the PVC, the database `ergo` and the namespace.
