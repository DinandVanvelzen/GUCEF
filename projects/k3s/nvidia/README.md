# NVIDIA GPU support

Makes an NVIDIA GPU on the k3s node usable by pods. All scripts skip themselves when no NVIDIA GPU is present.

| Script | Runs as | Does |
|---|---|---|
| `setup-host.sh` | root | Installs/upgrades the NVIDIA driver (Ubuntu) and the NVIDIA container toolkit |
| `setup.sh` | user | Labels the node, installs the NVIDIA device plugin with time-slicing, runs an `nvidia-smi` smoke test pod |
| `uninstall.sh` | user | Removes the device plugin and node labels; leaves the host driver installed |

## Usage

```bash
sudo ./setup-host.sh --check   # report what would change, change nothing
sudo ./setup-host.sh           # exit code 10 = reboot required, re-run afterwards
./setup.sh
```

`setup-host.sh` on a node where k3s is already running: a newly installed container toolkit is only picked up when k3s restarts.
A driver install requires a reboot, which covers that. Without a reboot, pass `--restart-k3s` (running pods keep running).

## Driver policy

- The driver is changed only when the loaded/installed driver is older than `NVIDIA_MIN_DRIVER_VERSION` (default 570).
- The package installed is the one `ubuntu-drivers devices` marks as recommended, or `NVIDIA_DRIVER_PACKAGE`.
- Before changing anything, the installed NVIDIA packages are recorded in `/var/backups/gucef-k3s/nvidia-packages-<timestamp>.txt`.
  Restore with `sudo apt-get install $(cat <file>)`. If the new driver fails to install, the script restores them automatically.
- Secure Boot: the kernel module signing key must be enrolled (MOK prompt) during the reboot.
- Distributions other than Ubuntu: install the driver and `nvidia-container-toolkit` manually.

## Using the GPU in a pod

```yaml
spec:
  runtimeClassName: nvidia
  containers:
    - name: app
      resources:
        limits:
          nvidia.com/gpu: 1
```

## Time-slicing

Each physical GPU is advertised as `GPU_TIMESLICE_REPLICAS` (default 4) `nvidia.com/gpu` resources, so several pods can share it.
VRAM is not isolated: the pods sharing a GPU must fit in its VRAM together. Budget VRAM per service, e.g. on a 12 GB card:

| Service | VRAM |
|---|---|
| LLM server (`../llm`) | ~10 GB, reserve configurable via `LLM_VRAM_RESERVE_MIB` |
| Speech-to-text (Whisper) | ~1–1.5 GB |

## Node labels

| Label | Value |
|---|---|
| `nvidia.com/gpu.present` | `true` (schedules the device plugin) |
| `gucef.org/gpu` | `nvidia` |
| `gucef.org/gpu-vram-mib` | VRAM of the GPU in MiB |

## Settings

Copy `config.env.example` to `config.env` (gitignored) and edit.

## Docker

Only k3s (containerd) is configured. To use the GPU from Docker on the same host as well:
`sudo nvidia-ctk runtime configure --runtime=docker && sudo systemctl restart docker`.
