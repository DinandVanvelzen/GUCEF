/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_guidriverMyGUIOpenGL_PLUGINAPI_H
#define GUCEF_guidriverMyGUIOpenGL_PLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_guidriverMyGUIOpenGL_MACROS_H
#include "guidriverMyGUIOpenGL_macros.h"
#define GUCEF_guidriverMyGUIOpenGL_MACROS_H
#endif /* GUCEF_guidriverMyGUIOpenGL_MACROS_H ? */

/*
 *  On the web there is no loading of modules: the plugin is linked into the application and its entry points get
 *  unique names (guidriverMyGUIOpenGL_GUCEFPlugin_Load ...), which the application calls itself
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #define GUIDRIVERMYGUIOPENGL_ENTRY( func ) guidriverMyGUIOpenGL_##func
#else
  #define GUIDRIVERMYGUIOPENGL_ENTRY( func ) func
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

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
#endif

/*---------------------------------------------------------------------------*/

GUCEF_guidriverMyGUIOpenGL_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERMYGUIOPENGL_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

GUCEF_guidriverMyGUIOpenGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERMYGUIOPENGL_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

GUCEF_guidriverMyGUIOpenGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERMYGUIOPENGL_ENTRY( GUCEFPlugin_GetVersion )( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

GUCEF_guidriverMyGUIOpenGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERMYGUIOPENGL_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*--------------------------------------------------------------------------*/

GUCEF_guidriverMyGUIOpenGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERMYGUIOPENGL_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOpenGL_PLUGINAPI_H ? */
