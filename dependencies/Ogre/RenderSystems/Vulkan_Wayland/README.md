# OgreRenderSystem_Vulkan_Wayland

Ogre Vulkan render system with windows on Wayland surfaces, used with the `Wayland` window backend of GUCEF (`GraphicsApi=None`). Linux only. It is the Wayland flavour of `OgreRenderSystem_Vulkan` (the sources of the directory `Vulkan`), loaded with the plugin `OgrePlugin_GLSLangProgramManager`.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

The Vulkan headers are part of this code base (`dependencies/Vulkan-Headers`) and the Vulkan functions are loaded at run time, so building needs no package beyond the compiler.

| Needed for | Packages |
|---|---|
| Building | none |
| Running | `libvulkan1` (the Vulkan loader), a Wayland compositor and a Vulkan driver: the one of the graphics driver (NVIDIA, `mesa-vulkan-drivers` for AMD and Intel) |
| Tests against a headless compositor (optional) | `weston`, run with `--backend=headless --renderer=pixman`, and `mesa-vulkan-drivers` (lavapipe) |

## Example: installing the packages on Ubuntu

Run time:

```bash
sudo apt install libvulkan1 mesa-vulkan-drivers
```

Headless tests (optional):

```bash
sudo apt install weston mesa-vulkan-drivers
```
