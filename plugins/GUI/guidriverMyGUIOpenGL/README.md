# guidriverMyGUIOpenGL

GUCEF GUI driver `MyGUIOpenGL` rendering MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building (Linux) | `libgl-dev`, `libgles-dev` |
| Running (Linux) | `libgl1` or `libegl1`, as the window backend that creates the GL context requires |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libgl-dev libgles-dev
```

Run time:

```bash
sudo apt install libgl1 libegl1
```
