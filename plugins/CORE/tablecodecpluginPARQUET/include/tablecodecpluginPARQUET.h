/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef TABLECODECPLUGIN_PARQUET_H
#define TABLECODECPLUGIN_PARQUET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#include "gucefCORE_TableCodecPluginAPI.h"
#define GUCEF_CORE_TABLECODECPLUGINAPI_H
#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_MACROS_H
#include "tablecodecpluginPARQUET_macros.h"
#define TABLECODECPLUGIN_PARQUET_MACROS_H
#endif /* TABLECODECPLUGIN_PARQUET_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The exports of a table codec plugin module (gucefCORE_TableCodecPluginAPI.h), loaded by the table codec plugin
 *  manager. The module offers one codec, "parquet". It has no generic plugin exports: GUCEFPlugin_GetLinkBackModuleDependencies
 *  would make the plugin control take the module for a generic plugin when its metadata names no plugin type.
 */

#ifdef __cplusplus
extern "C" {
#endif

TABLECODECPLUGIN_PARQUET_PUBLIC_C GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Init( const GUCEF::CORE::CValueList* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetCodecCount( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C GUCEF::CORE::CITableCodec* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_CreateCodec( GUCEF::CORE::UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Shutdown( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Description( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Copyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

TABLECODECPLUGIN_PARQUET_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Version( GUCEF::CORE::TVersion* version ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_H ? */
