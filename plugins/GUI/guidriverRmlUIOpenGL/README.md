# guidriverRmlUIOpenGL

GUCEF GUI driver `RmlUi` rendering with OpenGL 3 or OpenGL ES 3 (renderer of RmlUi).

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building (Linux) | `libgles-dev` (the `GLES3` headers) |
| Running (Linux) | `libegl1` or `libgl1`, as the window backend that creates the GL context requires |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libgles-dev
```

Run time:

```bash
sudo apt install libegl1 libgl1
```
