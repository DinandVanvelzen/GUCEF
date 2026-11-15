/* Export macros of the Ogre module that is built with the define Plugin_OctreeZone_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreOctreeZonePluginExport_H
#define _OgreOctreeZonePluginExport_H

#include "OgreBuildSettings.h"

#if defined( PLUGIN_OCTREEZONE_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreOctreeZonePluginExport
#  define PLUGIN_OCTREEZONE_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Plugin_OctreeZone_EXPORTS
#    define _OgreOctreeZonePluginExport __declspec(dllexport)
#  else
#    define _OgreOctreeZonePluginExport __declspec(dllimport)
#  endif
#  define PLUGIN_OCTREEZONE_NO_EXPORT
#else
#  define _OgreOctreeZonePluginExport __attribute__((visibility("default")))
#  define PLUGIN_OCTREEZONE_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreOctreeZonePluginExport_H */
