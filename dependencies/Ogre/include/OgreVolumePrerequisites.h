/* Export macros of the Ogre module that is built with the define OgreVolume_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreVolumeExport_H
#define _OgreVolumeExport_H

#include "OgreBuildSettings.h"

#if defined( OGREVOLUME_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreVolumeExport
#  define OGREVOLUME_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef OgreVolume_EXPORTS
#    define _OgreVolumeExport __declspec(dllexport)
#  else
#    define _OgreVolumeExport __declspec(dllimport)
#  endif
#  define OGREVOLUME_NO_EXPORT
#else
#  define _OgreVolumeExport __attribute__((visibility("default")))
#  define OGREVOLUME_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreVolumeExport_H */
