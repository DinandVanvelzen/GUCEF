/*
 *  codecspluginOPENSSL: Codec plugin for authenticated encryption, key agreement, signatures, hashing and key derivation using the OpenSSL libcrypto of the system
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

#ifndef GUCEF_CODECSPLUGIN_OPENSSL_H
#define GUCEF_CODECSPLUGIN_OPENSSL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"           /* C API for media abstraction  */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_C_CODEC_PLUGIN_API_H
#include "gucefCORE_c_codec_plugin_api.h"   /* the codec plugin C interface */
#define GUCEF_CORE_C_CODEC_PLUGIN_API_H
#endif /* GUCEF_CORE_C_CODEC_PLUGIN_API_H ? */

#ifndef GUCEF_CODECSPLUGIN_OPENSSL_MACROS_H
#include "codecspluginOPENSSL_macros.h"
#define GUCEF_CODECSPLUGIN_OPENSSL_MACROS_H
#endif /* GUCEF_CODECSPLUGIN_OPENSSL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CODECSPLUGIN {
namespace OPENSSL {
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
 *  The codec plugin C interface, see gucefCORE_c_codec_plugin_api.h for the rules
 */

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata                    ,
                  TGucefCoreCApi* coreApi            ,
                  const TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
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
CODECPLUGIN_GetCodecCount( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

GUCEF_CODEC_EXPORT_C TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata                     ,
                          UInt32 index                       ,
                          const TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

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
}; /* namespace OPENSSL */
}; /* namespace CODECSPLUGIN */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CODECSPLUGIN_OPENSSL_H ? */
