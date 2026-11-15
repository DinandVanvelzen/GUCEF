/* Export macros of the Ogre module that is built with the define Plugin_OctreeSceneManager_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreOctreePluginExport_H
#define _OgreOctreePluginExport_H

#include "OgreBuildSettings.h"

#if defined( PLUGIN_OCTREESCENEMANAGER_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreOctreePluginExport
#  define PLUGIN_OCTREESCENEMANAGER_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Plugin_OctreeSceneManager_EXPORTS
#    define _OgreOctreePluginExport __declspec(dllexport)
#  else
#    define _OgreOctreePluginExport __declspec(dllimport)
#  endif
#  define PLUGIN_OCTREESCENEMANAGER_NO_EXPORT
#else
#  define _OgreOctreePluginExport __attribute__((visibility("default")))
#  define PLUGIN_OCTREESCENEMANAGER_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreOctreePluginExport_H */
