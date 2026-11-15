# pluginglueWAYLAND

Shared Wayland support of the GUCEF Wayland window backend and input driver: loading of the Wayland client libraries and the interface declarations. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

| Needed for | Packages |
|---|---|
| Building | none: the interface declarations are part of the plugin (`pluginglueWAYLAND_Abi.h`) |
| Running | `libwayland-client0`, `libwayland-cursor0`, `libwayland-egl1` |

## Example: installing the packages on Ubuntu

Run time:

```bash
sudo apt install libwayland-client0 libwayland-cursor0 libwayland-egl1
```

## Behaviour when a package is missing

* The libraries are loaded on demand with `dlopen`.
