# guidriverEGL

GUCEF window backend with EGL surfaces, including windows without a display server (offscreen, used for tests and headless rendering).

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | `libegl-dev`, `libgl-dev` (the `GL/glcorearb.h` header) |
| Running | `libegl1` and an EGL driver (`libegl-mesa0` and `libgl1-mesa-dri` for Mesa software rendering) |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libegl-dev libgl-dev
```

Run time and headless Mesa software rendering:

```bash
sudo apt install libegl1 libegl-mesa0 libgl1-mesa-dri
```

## Behaviour when a package is missing

* A system with several EGL vendors may offer a GPU device before Mesa. Headless runs that must use Mesa select it with `__EGL_VENDOR_LIBRARY_FILENAMES` pointing at the Mesa vendor file and `LIBGL_ALWAYS_SOFTWARE=1`.
