# OgreRenderSystem_GL3Plus_Wayland

Ogre OpenGL 3+ render system with EGL windows on Wayland surfaces, used with the `Wayland` window backend of GUCEF (`GraphicsApi=None`). Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the module from source, in every build configuration (debug and release), and not to run a built module. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | `libwayland-dev`, `libegl-dev` |
| Running | `libwayland-client0`, `libwayland-egl1`, `libegl1`, a Wayland compositor and an EGL driver (`libegl-mesa0` for Mesa) |
| Tests against a headless compositor (optional) | `weston`, run with `--backend=headless --renderer=pixman` |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libwayland-dev libegl-dev
```

Run time:

```bash
sudo apt install libwayland-client0 libwayland-egl1 libegl1 libegl-mesa0
```

Headless tests (optional):

```bash
sudo apt install weston
```
