# guidriverCEGUIOpenGL

GUCEF GUI driver rendering CEGUI with OpenGL.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building (Linux) | `libgl-dev` |
| Running (Linux) | `libgl1` |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libgl-dev
```

Run time:

```bash
sudo apt install libgl1
```
