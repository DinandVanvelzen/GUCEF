# codecspluginOPENSSL

GUCEF codec plugin with AES-256-GCM, ChaCha20-Poly1305, X25519, Ed25519, HKDF-SHA256, HMAC-SHA256, SHA-256 and SHA-512 through the OpenSSL 3 libcrypto of the system. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | `libssl-dev` (OpenSSL 3 development headers) |
| Running | `libssl3t64` (`libssl3` before Ubuntu 24.04 and Debian 13) |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libssl-dev
```

Run time:

```bash
sudo apt install libssl3t64
```

## Behaviour when a package is missing

* Without the development headers the plugin builds without codecs.
