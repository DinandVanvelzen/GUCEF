/*
 *  gucefCORE_TestPlugin: generic plugin used by the gucefCORE tests of the plugin system
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTPLUGIN_H
#define GUCEF_CORE_TESTPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#include "gucefCORE_TableCodecPluginAPI.h"
#define GUCEF_CORE_TABLECODECPLUGINAPI_H
#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_CORE_TESTPLUGIN_BUILD_MODULE
  #define GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF_EXPORT
#else
  #define GUCEF_CORE_TESTPLUGIN_EXPORT_C
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  A generic plugin that does nothing but what the tests ask of it through environment variables:
 *  GUCEF_TESTPLUGIN_LOAD_THROWS=1 and GUCEF_TESTPLUGIN_UNLOAD_THROWS=1 make GUCEFPlugin_Load() and GUCEFPlugin_Unload()
 *  throw, as a broken plugin would. GUCEF_TESTPLUGIN_LOAD_FAILS=1 makes GUCEFPlugin_Load() return that it failed, and
 *  GUCEF_TESTPLUGIN_LOAD_ADDS_FAILING_CONFIGURABLE=1 makes it bring along a global configurable that fails to load its config,
 *  as the globals of a module that a plugin starts can.
 *  It counts the calls of Load and Unload in GUCEF_TESTPLUGIN_LOADS and GUCEF_TESTPLUGIN_UNLOADS.
 */

#ifdef __cplusplus
extern "C" {
#endif

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  The module is also a table codec plugin (gucefCORE_TableCodecPluginAPI.h) with one codec, for the tests of the table codec
 *  plugin manager. The codec has the type name in GUCEF_TESTPLUGIN_TC_TYPE ("testtable" when not set). The module counts
 *  the calls of Init, Shutdown and the destruction of its codecs in GUCEF_TESTPLUGIN_TC_INITS, GUCEF_TESTPLUGIN_TC_SHUTDOWNS and
 *  GUCEF_TESTPLUGIN_TC_CODECSDESTROYED. GUCEF_TESTPLUGIN_TC_API_VERSION replaces the API version it reports (a number) and
 *  GUCEF_TESTPLUGIN_TC_INIT_FAILS=1 makes Init fail.
 */

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Init( const GUCEF::CORE::CValueList* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetCodecCount( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::CITableCodec* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_CreateCodec( GUCEF::CORE::UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Shutdown( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Description( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Copyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Version( GUCEF::CORE::TVersion* version ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTPLUGIN_H ? */
