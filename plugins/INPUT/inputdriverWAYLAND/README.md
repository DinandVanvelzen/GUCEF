# inputdriverWAYLAND

GUCEF input driver for Wayland: keyboard and pointer events of Wayland windows, with keyboard layouts through xkbcommon. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

| Needed for | Packages |
|---|---|
| Building | none: the xkbcommon and Wayland entry points are resolved at run time |
| Running | `libxkbcommon0` and the packages of `pluginglueWAYLAND` |

## Example: installing the packages on Ubuntu

Run time:

```bash
sudo apt install libxkbcommon0 libwayland-client0 libwayland-cursor0 libwayland-egl1
```

## Behaviour when a package is missing

* `libxkbcommon.so.0` is loaded on demand.
