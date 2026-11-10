/*
 *  guidriverRmlUIOpenGL: module providing a GUI backend using RmlUi and OpenGL
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUIDRIVERRMLUIGL_PLUGINAPI_H
#define GUCEF_GUIDRIVERRMLUIGL_PLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#include "guidriverRmlUIOpenGL_macros.h"
#define GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#endif /* GUCEF_GUIDRIVERRMLUIGL_MACROS_H ? */

/*
 *  On the web there is no loading of modules: the plugin is linked into the application and its entry points get
 *  unique names (guidriverRmlUIOpenGL_GUCEFPlugin_Load ...), which the application calls itself
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #define GUIDRIVERRMLUIGL_ENTRY( func ) guidriverRmlUIOpenGL_##func
#else
  #define GUIDRIVERRMLUIGL_ENTRY( func ) func
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace GUIDRIVERRMLUIGL {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*-------------------------------------------------------------------------*/

GUIDRIVERRMLUIGL_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUIDRIVERRMLUIGL_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERRMLUIGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUIDRIVERRMLUIGL_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERRMLUIGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUIDRIVERRMLUIGL_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERRMLUIGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUIDRIVERRMLUIGL_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERRMLUIGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUIDRIVERRMLUIGL_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   };
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUIGL_PLUGINAPI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/