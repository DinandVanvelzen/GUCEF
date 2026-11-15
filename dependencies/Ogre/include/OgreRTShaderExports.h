/* Export macros of the Ogre module that is built with the define OgreRTShaderSystem_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreRTSSExport_H
#define _OgreRTSSExport_H

#include "OgreBuildSettings.h"

#if defined( OGRERTSHADERSYSTEM_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreRTSSExport
#  define OGRERTSHADERSYSTEM_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef OgreRTShaderSystem_EXPORTS
#    define _OgreRTSSExport __declspec(dllexport)
#  else
#    define _OgreRTSSExport __declspec(dllimport)
#  endif
#  define OGRERTSHADERSYSTEM_NO_EXPORT
#else
#  define _OgreRTSSExport __attribute__((visibility("default")))
#  define OGRERTSHADERSYSTEM_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreRTSSExport_H */
