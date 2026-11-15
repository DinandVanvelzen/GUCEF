/* Export macros of the Ogre module that is built with the define Plugin_ParticleFX_EXPORTS.
   Hand maintained equivalent of the header which CMake generate_export_header
   produces in the upstream build, with the platform and static build cases
   decided by the preprocessor instead of the CMake configuration. */
#ifndef _OgreParticleFXExport_H
#define _OgreParticleFXExport_H

#include "OgreBuildSettings.h"

#if defined( PLUGIN_PARTICLEFX_STATIC_DEFINE ) || defined( OGRE_STATIC_LIB )
#  define _OgreParticleFXExport
#  define PLUGIN_PARTICLEFX_NO_EXPORT
#elif defined( _WIN32 ) || defined( __CYGWIN__ )
#  ifdef Plugin_ParticleFX_EXPORTS
#    define _OgreParticleFXExport __declspec(dllexport)
#  else
#    define _OgreParticleFXExport __declspec(dllimport)
#  endif
#  define PLUGIN_PARTICLEFX_NO_EXPORT
#else
#  define _OgreParticleFXExport __attribute__((visibility("default")))
#  define PLUGIN_PARTICLEFX_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif /* _OgreParticleFXExport_H */
