/* Export macros of the Ogre module that is built with the define RenderSystem_GL3Plus_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreGL3PlusExport_H
#define _OgreGL3PlusExport_H

#include "OgreBuildSettings.h"

#if defined( RENDERSYSTEM_GL3PLUS_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreGL3PlusExport
#  define RENDERSYSTEM_GL3PLUS_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef RenderSystem_GL3Plus_EXPORTS
#    define _OgreGL3PlusExport __declspec(dllexport)
#  else
#    define _OgreGL3PlusExport __declspec(dllimport)
#  endif
#  define RENDERSYSTEM_GL3PLUS_NO_EXPORT
#else
#  define _OgreGL3PlusExport __attribute__((visibility("default")))
#  define RENDERSYSTEM_GL3PLUS_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreGL3PlusExport_H */
