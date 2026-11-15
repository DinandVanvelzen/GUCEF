#ifndef __OGRE_COMPONENTS_Config_H_
#define __OGRE_COMPONENTS_Config_H_

/* The components, plugins and render systems that are part of the Ogre modules of this code base.
   This is the equivalent of the OgreComponents.h which the upstream CMake build generates from
   CMake/Templates/OgreComponents.h.in. The modules which are not built on a platform are left
   out for that platform. */

#if defined(_WIN32) && !defined(__ANDROID__)
#   define OGRE_BUILD_RENDERSYSTEM_D3D9
#   define OGRE_BUILD_RENDERSYSTEM_D3D11
#endif
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#   define OGRE_BUILD_RENDERSYSTEM_GL
#   define OGRE_BUILD_RENDERSYSTEM_GL3PLUS
#endif
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__) || (defined(__linux__) && !defined(__ANDROID__))
#   define OGRE_BUILD_RENDERSYSTEM_GLES2
#endif
#if !defined(__EMSCRIPTEN__)
#   define OGRE_BUILD_PLUGIN_BSP
#endif
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#   define OGRE_BUILD_PLUGIN_PCZ
#endif
#define OGRE_BUILD_PLUGIN_OCTREE
#define OGRE_BUILD_PLUGIN_PFX
#define OGRE_BUILD_PLUGIN_STBI
#if !defined(__EMSCRIPTEN__)
#   define OGRE_BUILD_COMPONENT_PAGING
#   define OGRE_BUILD_COMPONENT_TERRAIN
#   define OGRE_BUILD_COMPONENT_VOLUME
#endif
#define OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#define OGRE_BUILD_COMPONENT_PROPERTY
#define OGRE_BUILD_COMPONENT_OVERLAY
#define OGRE_BUILD_COMPONENT_RTSHADERSYSTEM

#define OGRE_BITES_HAVE_SDL 0

#endif
