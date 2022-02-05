/*
 *  codecspluginSTBRUMMEHASH: Codec plugin for CRC32/MD5/SHA1/SHA3/SHA256/Keccak
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CODECSPLUGIN_STBRUMMEHASH_H
#define GUCEF_CODECSPLUGIN_STBRUMMEHASH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_IOACCESS_H
#include "ioaccess.h"           /* C API for media abstraction  */
#define GUCEF_CORE_IOACCESS_H
#endif /* GUCEF_CORE_IOACCESS_H ? */

#ifndef GUCEF_CORE_CODECPLUGINLINK_H
#include "CodecPluginLink.h"
#define GUCEF_CORE_CODECPLUGINLINK_H
#endif /* GUCEF_CORE_CODECPLUGINLINK_H ? */

#ifndef GUCEF_CODECSPLUGIN_STBRUMMEHASH_MACROS_H
#include "codecspluginSTBRUMMEHASH_macros.h"
#define GUCEF_CODECSPLUGIN_STBRUMMEHASH_MACROS_H
#endif /* GUCEF_CODECSPLUGIN_STBRUMMEHASH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CODECSPLUGIN {
namespace STBRUMMEHASH {
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_CODECPLUGIN_BUILD_MODULE
 #define GUCEF_CODEC_EXPORT_C GUCEF_EXPORT
#else
 #define GUCEF_CODEC_EXPORT_C GUCEF_IMPORT
#endif /* GUCEF_CODECPLUGIN_BUILD_MODULE ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
using namespace ::GUCEF;
using namespace ::GUCEF::CORE;
extern "C" {
#endif

/*---------------------------------------------------------------------------*/

/*
 *  This module has the generic codec API as well as the expanded image codec API
 *  We start with the generic codec API below.
 */

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata   ,
                  const int argc    ,
                  const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecSetBegin( void* plugdata  ,
                              void** iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata               ,
                          void* iterator               ,
                          TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_FreeCodecLink( void* plugdata              ,
                           TCodecPluginLink* codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_FreeCodecIterator( void* plugdata ,
                               void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecSetNextItem( void* plugdata ,
                                 void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace STBRUMMEHASH */
}; /* namespace CODECSPLUGIN */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CODECSPLUGIN_STBRUMMEHASH_H ? */
