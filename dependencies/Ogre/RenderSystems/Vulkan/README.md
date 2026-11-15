# OgreRenderSystem_Vulkan

Ogre Vulkan render system, used with the `Ogre` window backend of GUCEF on X11 windows (`XWinGL` with `GraphicsApi=None`) on Linux, on Win32 windows on Windows and on the native window of Android. Its programs are SPIR-V compiled from GLSL by the plugin `OgrePlugin_GLSLangProgramManager`, which is loaded with it. The Vulkan functions are loaded at run time (volk), so the module does not link to a Vulkan library.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the module from source, in every build configuration (debug and release), and not to run a built module. Running needs the run-time packages only. The Vulkan headers are part of this code base (`dependencies/Vulkan-Headers`), no Vulkan SDK is needed.

| Needed for | Packages |
|---|---|
| Building | `libx11-dev` |
| Running | `libx11-6`, `libvulkan1` (the Vulkan loader) and a Vulkan driver: the one of the graphics driver (NVIDIA, `mesa-vulkan-drivers` for AMD and Intel) |
| Tests without a GPU (optional) | `mesa-vulkan-drivers` (lavapipe, the software Vulkan driver of Mesa) and `xvfb`, run with `VK_DRIVER_FILES=/usr/share/vulkan/icd.d/lvp_icd.json` |

On Windows the Vulkan loader (`vulkan-1.dll`) comes with the graphics driver.

## Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libx11-dev
```

Run time:

```bash
sudo apt install libx11-6 libvulkan1 mesa-vulkan-drivers
```

Tests without a GPU (optional):

```bash
sudo apt install mesa-vulkan-drivers xvfb
```
