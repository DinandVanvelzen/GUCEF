/* Export macros of the Ogre module that is built with the define Codec_STBI_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreSTBICodecExport_H
#define _OgreSTBICodecExport_H

#include "OgreBuildSettings.h"

#if defined( CODEC_STBI_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreSTBICodecExport
#  define CODEC_STBI_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Codec_STBI_EXPORTS
#    define _OgreSTBICodecExport __declspec(dllexport)
#  else
#    define _OgreSTBICodecExport __declspec(dllimport)
#  endif
#  define CODEC_STBI_NO_EXPORT
#else
#  define _OgreSTBICodecExport __attribute__((visibility("default")))
#  define CODEC_STBI_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreSTBICodecExport_H */
