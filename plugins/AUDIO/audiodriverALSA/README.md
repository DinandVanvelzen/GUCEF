# audiodriverALSA

GUCEF audio driver for ALSA playback and capture. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

| Needed for | Packages |
|---|---|
| Building | none: the ALSA entry points are resolved at run time |
| Running | `libasound2t64` (`libasound2` before Ubuntu 24.04 and Debian 13) |

## Example: installing the packages on Ubuntu

Run time:

```bash
sudo apt install libasound2t64
```

## Behaviour when a package is missing

* `libasound.so.2` is loaded on demand; without it the driver is not available.
