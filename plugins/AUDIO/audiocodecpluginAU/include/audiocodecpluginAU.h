/*
 *  audiocodecpluginAU: gucefAUDIO codec plugin for Sun/NeXT .au audio
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

#ifndef GUCEF_AUDIOCODECPLUGIN_AU_H
#define GUCEF_AUDIOCODECPLUGIN_AU_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#include "gucefAUDIO_c_audiocodec_plugin_api.h"     /* the interface this plugin implements */
#define GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H
#endif /* GUCEF_AUDIO_C_AUDIOCODEC_PLUGIN_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_AUDIOCODECPLUGIN_AU_BUILD_MODULE
  #define AUDIOCODECPLUG_EXPORT_C GUCEF_EXPORT
#else
  #define AUDIOCODECPLUG_EXPORT_C
#endif

#undef MODULE_API_FUNCTION
#ifdef MODULE_API_PREFIX
  #define MODULE_API_FUNCTION( func ) GUCEF_CONCAT( MODULE_API_PREFIX, func )
#else
  #define MODULE_API_FUNCTION( func ) func
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TAudioFormatInfo* format, UInt64* lengthInFrames ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Read_Frames )( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Seek )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Decode_Meta_Data )( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Encode )( void** plugdata, void** codecdata, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIOCODECPLUGIN_AU_H ? */
