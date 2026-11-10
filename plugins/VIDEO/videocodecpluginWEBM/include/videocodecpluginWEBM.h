/*
 *  videocodecpluginWEBM: gucefVIDEO codec plugin for WebM video (VP8 / VP9 with Opus)
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

#ifndef GUCEF_VIDEOCODECPLUGIN_WEBM_H
#define GUCEF_VIDEOCODECPLUGIN_WEBM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H
#include "gucefVIDEO_c_videocodec_plugin_api.h"     /* the interface this plugin implements */
#define GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H
#endif /* GUCEF_VIDEO_C_VIDEOCODEC_PLUGIN_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_VIDEOCODECPLUGIN_WEBM_BUILD_MODULE
  #define VIDEOCODECPLUG_EXPORT_C GUCEF_EXPORT
#else
  #define VIDEOCODECPLUG_EXPORT_C
#endif

/*
 *  When linked statically (platforms without loadable modules) the entry points get a prefix so several plugins
 *  can live in one program
 */
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
namespace GUCEF {
namespace VIDEO {
extern "C" {
#endif

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TVideoFormatInfo* format, UInt64* frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Get_Audio_Track_Format )( void** plugdata, void** codecdata, void** sourcedata, UInt32 track, TAudioFormatInfo* format ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Decode_Frame )( void** plugdata, void** codecdata, void** sourcedata, TVideoPictureBuffer* picture, const TVideoAudioOutput* audio, UInt32* endOfStream ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Seek_Frame )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Open )( void** plugdata, void** codecdata, void** sinkdata, TIOAccess* output, const TVariantMapApi* options, TVideoFormatInfo* format, UInt32 audioTrackCount, TAudioFormatInfo* audioTrackFormats ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Write_Frame )( void** plugdata, void** codecdata, void** sinkdata, const TVideoPictureBuffer* picture ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Write_Audio )( void** plugdata, void** codecdata, void** sinkdata, UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

VIDEOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( VIDEOCODECPLUG_Snk_Close )( void** plugdata, void** codecdata, void** sinkdata, UInt32 finish ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
}; /* namespace VIDEO */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEOCODECPLUGIN_WEBM_H ? */
