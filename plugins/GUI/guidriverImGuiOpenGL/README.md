# guidriverImGuiOpenGL

GUCEF GUI driver `ImGui` rendering Dear ImGui with OpenGL 3.3 core or OpenGL ES 3.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building (Linux) | `libgles-dev` (the `GLES2` and `GLES3` headers) |
| Running (Linux) | `libegl1`, `libgl1` and `libopengl0` (the loader opens `libEGL.so.1`, `libGL.so.1` and `libOpenGL.so.0` when the window offers no GL address function) |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libgles-dev
```

Run time:

```bash
sudo apt install libegl1 libgl1 libopengl0
```

## Behaviour when a package is missing

* The driver refuses windows that report no GL version or a version below OpenGL 3.3 or OpenGL ES 3.0, and logs why.
