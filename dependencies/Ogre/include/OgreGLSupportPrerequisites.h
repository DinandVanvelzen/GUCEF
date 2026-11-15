/* Export macro of the Ogre OpenGL support library, which is a static library that is linked into the
   OpenGL render system plugins, so nothing is exported or imported.
   Hand maintained equivalent of the header which CMake generate_export_header produces in the upstream build. */
#ifndef _OgreGLExport_H
#define _OgreGLExport_H

#ifdef OGREGLSUPPORT_STATIC_DEFINE
#  define _OgreGLExport
#  define OGREGLSUPPORT_NO_EXPORT
#else
#  ifndef _OgreGLExport
#    define _OgreGLExport
#  endif

#  ifndef OGREGLSUPPORT_NO_EXPORT
#    define OGREGLSUPPORT_NO_EXPORT
#  endif
#endif

#endif /* _OgreGLExport_H */
