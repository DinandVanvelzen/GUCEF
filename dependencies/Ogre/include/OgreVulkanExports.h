/* Export macros of the Ogre module that is built with the define RenderSystem_Vulkan_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreVulkanExport_H
#define _OgreVulkanExport_H

#include "OgreBuildSettings.h"

#if defined( RENDERSYSTEM_VULKAN_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreVulkanExport
#  define RENDERSYSTEM_VULKAN_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef RenderSystem_Vulkan_EXPORTS
#    define _OgreVulkanExport __declspec(dllexport)
#  else
#    define _OgreVulkanExport __declspec(dllimport)
#  endif
#  define RENDERSYSTEM_VULKAN_NO_EXPORT
#else
#  define _OgreVulkanExport __attribute__((visibility("default")))
#  define RENDERSYSTEM_VULKAN_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreVulkanExport_H */
