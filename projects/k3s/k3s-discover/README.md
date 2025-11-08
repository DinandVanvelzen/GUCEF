# k3s-discover

Lightweight Rust daemon to detect k3s connectivity loss and advertise the node as a candidate via multicast (mDNS multicast address). This project intentionally keeps the implementation small and dependency-light so it can run on SBCs.

Behavior
- Periodically checks connectivity to the k3s server (default checks extracted from systemd or falls back to 127.0.0.1:6443).
- When connectivity cannot be established, the daemon multicasts a compact JSON payload to 224.0.0.251:5353 indicating hostname, IP, architecture and timestamp.

Why Rust?
- Small, efficient static binaries suitable for SBCs.

Quick build

You need Rust installed to build locally. To build a release binary:

```powershell
# Build (out-of-source). Cargo is configured to place outputs under the repo root at
# <repo-root>/common/bin/k3s-discover (see `.cargo/config.toml`). From the project root run:
cargo build --release
```

When using the project's out-of-source configuration the release binaries will be at:
`<repo-root>/common/bin/k3s-discover/release/` (otherwise `target/release/` for a normal in-tree build).

Install

Copy the binary to `/usr/local/bin/k3s-discover` and the systemd unit to `/etc/systemd/system/k3s-discover.service`, then:

```powershell
sudo systemctl daemon-reload;
sudo systemctl enable --now k3s-discover;
```

Notes
- This implementation sends a small JSON message to the mDNS multicast address rather than performing full DNS-SD record construction; this keeps the code simple and reliable across distros without requiring Avahi.
- The listener side can join the multicast group 224.0.0.251:5353 and parse JSON messages with note=="k3s-candidate".

Cross-compiling for ARM/ARM64
- Example targets: `armv7-unknown-linux-gnueabihf`, `aarch64-unknown-linux-gnu`.
- Build outputs are redirected out-of-source to `<repo-root>/common/bin/k3s-discover` (see `.cargo/config.toml`).
- Use the helper scripts in `deploy/` to cross-build:

```bash
sh deploy/cross-build-aarch64.sh
sh deploy/cross-build-armv7.sh
```

These scripts prefer `cross` if available; otherwise they set `CARGO_TARGET_DIR` and invoke cargo. When building without `cross` you need a suitable cross-linker installed on the host (for example `aarch64-linux-gnu-gcc`).
