# guidriverWAYLAND

GUCEF window backend `Wayland`: xdg-shell toplevel windows with EGL rendering, frame callbacks, scales and cursor shapes. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | none: the Wayland client interface is declared by `pluginglueWAYLAND` (`pluginglueWAYLAND_Abi.h`) and resolved when the plugin loads |
| Running | `libwayland-client0`, `libwayland-cursor0`, `libwayland-egl1`, `libegl1`, a Wayland compositor and an EGL driver (`libegl-mesa0` for Mesa) |
| Window decorations without server side decorations | `libdecor-0-0`, and `libdecor-0-plugin-1-gtk` for the GTK decoration plugin |
| Tests against a headless compositor (optional) | `weston`, run with `--backend=headless --renderer=pixman` |
| Development against the real headers (optional) | `libwayland-dev`, `wayland-protocols`, `libxkbcommon-dev`, `libdecor-0-dev` |

## Example: installing the packages on Ubuntu

Run time:

```bash
sudo apt install libwayland-client0 libwayland-cursor0 libwayland-egl1 libegl1 libegl-mesa0 libdecor-0-0 libdecor-0-plugin-1-gtk
```

Headless tests and development against the real headers (optional):

```bash
sudo apt install weston libwayland-dev wayland-protocols libxkbcommon-dev libdecor-0-dev
```

## Behaviour when a package is missing

* The libraries are loaded when the backend is first used. The plugin itself loads on a system without Wayland and reports that the backend is not available.
* `libdecor` is used when the compositor does not offer server side decorations; it is loaded on demand.
