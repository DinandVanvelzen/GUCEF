/* Export macros of the Ogre module that is built with the define Plugin_BSPSceneManager_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreBspPluginExport_H
#define _OgreBspPluginExport_H

#include "OgreBuildSettings.h"

#if defined( PLUGIN_BSPSCENEMANAGER_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreBspPluginExport
#  define PLUGIN_BSPSCENEMANAGER_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Plugin_BSPSceneManager_EXPORTS
#    define _OgreBspPluginExport __declspec(dllexport)
#  else
#    define _OgreBspPluginExport __declspec(dllimport)
#  endif
#  define PLUGIN_BSPSCENEMANAGER_NO_EXPORT
#else
#  define _OgreBspPluginExport __attribute__((visibility("default")))
#  define PLUGIN_BSPSCENEMANAGER_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreBspPluginExport_H */
