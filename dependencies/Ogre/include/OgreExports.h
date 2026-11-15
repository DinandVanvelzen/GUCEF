/* Export macros of the Ogre core library, the module that is built with the define OgreMain_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header produces in the
   upstream build, with the platform and static build cases decided by the preprocessor instead
   of the CMake configuration. */
#ifndef _OgreExport_H
#define _OgreExport_H

#include "OgreBuildSettings.h"

#if defined( OGREMAIN_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreExport
#  define _OgrePrivate
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef OgreMain_EXPORTS
#    define _OgreExport __declspec(dllexport)
#  else
#    define _OgreExport __declspec(dllimport)
#  endif
#  define _OgrePrivate
#else
#  define _OgreExport __attribute__((visibility("default")))
#  define _OgrePrivate __attribute__((visibility("hidden")))
#endif

#ifndef OGRE_DEPRECATED
#  if defined( _MSC_VER )
#    define OGRE_DEPRECATED __declspec(deprecated)
#  else
#    define OGRE_DEPRECATED __attribute__ ((__deprecated__))
#  endif
#endif

#ifndef OGRE_DEPRECATED_EXPORT
#  define OGRE_DEPRECATED_EXPORT _OgreExport OGRE_DEPRECATED
#endif

#ifndef OGRE_DEPRECATED_NO_EXPORT
#  define OGRE_DEPRECATED_NO_EXPORT _OgrePrivate OGRE_DEPRECATED
#endif

#endif /* _OgreExport_H */
