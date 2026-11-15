/*
 *  guidriverImGuiOpenGL: gucefGUI driver ImGui: Dear ImGui drawn with OpenGL
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

#ifndef GUIDRIVERIMGUIGL_PLUGINAPI_H
#define GUIDRIVERIMGUIGL_PLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUIDRIVERIMGUIGL_MACROS_H
#include "guidriverImGuiOpenGL_macros.h"
#define GUIDRIVERIMGUIGL_MACROS_H
#endif /* GUIDRIVERIMGUIGL_MACROS_H ? */

/*
 *  On the web there is no loading of modules: the plugin is linked into the application and its entry points get
 *  unique names (guidriverImGuiOpenGL_GUCEFPlugin_Load ...), which the application calls itself
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #define GUIDRIVERIMGUIGL_ENTRY( func ) guidriverImGuiOpenGL_##func
#else
  #define GUIDRIVERIMGUIGL_ENTRY( func ) func
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace GUIDRIVERIMGUIGL {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*-------------------------------------------------------------------------*/

/** Registers the GUI driver of this plugin */
GUIDRIVERIMGUIGL_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUIGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUIGL_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUIGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUIGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUIGL_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUIDRIVERIMGUIGL_ENTRY( GUCEFPlugin_GetLinkBackModuleDependencies )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

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
}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_PLUGINAPI_H ? */
