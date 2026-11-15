# guidriverXWinGL

GUCEF window backend for X11 windows with GLX OpenGL contexts, also used for X11 windows without a GL context (`GraphicsApi=None`). Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | `libx11-dev`, `libgl-dev` (GLX headers) |
| Running | `libx11-6`, `libgl1`, an X server (`xvfb` for headless tests) |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libx11-dev libgl-dev
```

Run time:

```bash
sudo apt install libx11-6 libgl1
```

Headless tests (optional):

```bash
sudo apt install xvfb
```
