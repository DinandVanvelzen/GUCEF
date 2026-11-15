# guidriverOgre

GUCEF window backend `Ogre`: Ogre 14.6.3 render windows on the platform window drivers, the overlay interface for GUI drivers and the shared GUI material.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building (Linux) | `libx11-dev`, `libxrandr-dev`, `libegl-dev`, `libgl-dev` (the OpenGL render systems of Ogre use EGL with X11 on Linux) |
| Running (Linux) | `libx11-6`, `libxrandr2`, `libegl1`, `libgl1`, an X server or the packages of `guidriverWAYLAND` |
| Headless tests (optional) | `xvfb`, `libegl-mesa0`, `libgl1-mesa-dri` |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libx11-dev libxrandr-dev libegl-dev libgl-dev
```

Run time:

```bash
sudo apt install libx11-6 libxrandr2 libegl1 libgl1
```

Headless tests (optional):

```bash
sudo apt install xvfb libegl-mesa0 libgl1-mesa-dri
```

## Notes

* The `Offscreen` mode is refused while `DISPLAY` is set, because Ogre chooses between X11 and headless operation once, when its plugins are installed.
