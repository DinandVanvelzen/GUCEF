# inputdriverXWINMSG

GUCEF input driver that reads keyboard and mouse events of X11 windows. Linux only.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building | `libx11-dev` |
| Running | `libx11-6`, an X server |

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libx11-dev
```

Run time:

```bash
sudo apt install libx11-6
```
