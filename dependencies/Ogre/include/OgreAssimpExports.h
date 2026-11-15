/* Export macros of the Ogre module that is built with the define Codec_Assimp_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreAssimpExport_H
#define _OgreAssimpExport_H

#include "OgreBuildSettings.h"

#if defined( CODEC_ASSIMP_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreAssimpExport
#  define CODEC_ASSIMP_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Codec_Assimp_EXPORTS
#    define _OgreAssimpExport __declspec(dllexport)
#  else
#    define _OgreAssimpExport __declspec(dllimport)
#  endif
#  define CODEC_ASSIMP_NO_EXPORT
#else
#  define _OgreAssimpExport __attribute__((visibility("default")))
#  define CODEC_ASSIMP_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreAssimpExport_H */
