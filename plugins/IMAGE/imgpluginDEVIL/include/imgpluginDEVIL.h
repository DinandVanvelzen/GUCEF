/*
 *  imgpluginDEVIL: GUCEF image codec plugin providing an adapter for the DevIL library
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef IMGPLUGINDEVIL_H
#define IMGPLUGINDEVIL_H

/*
 *      API for an image load/save plugin module
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"           /* gucef core structures, needed for TVersion */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_IOACCESS_H
#include "ioaccess.h"           /* C API for media abstraction  */
#define GUCEF_CORE_IOACCESS_H
#endif /* GUCEF_CORE_IOACCESS_H ? */

#ifndef GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H
#include "gucefCORE_c_valuemap_parser_callbacks.h"
#define GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H
#endif /* GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H ? */

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_c_imagedata.h"          /* plugin API structures */
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

#ifndef GUCEF_CORE_CODECPLUGINLINK_H
#include "CodecPluginLink.h"
#define GUCEF_CORE_CODECPLUGINLINK_H
#endif /* GUCEF_CORE_CODECPLUGINLINK_H ? */

#ifndef GUCEF_IMAGE_MACROS_H
#include "gucefIMAGE_macros.h"  /* gucefIMAGE macros, used here for the export and callspec macros */
#define GUCEF_IMAGE_MACROS_H
#endif /* GUCEF_IMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace IMAGE {
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
extern "C" {
#endif

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

/*
 *  Below is the image codec API which expands upon the generic codec API
 */

/*---------------------------------------------------------------------------*/

/*
 *
 *  Note that when using this image codec API the Codec Family is always "ImageCodec"
 *  For images decoded using this function you need to use
 *      IMGCODECPLUGIN_FreeImageStorage
 *  to deallocate the memory after you are done with the image
 */
GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TIOAccess* input      ,
                            TImage** imageOutput  ,
                            void** imageData      );

/*---------------------------------------------------------------------------*/

/*
 *  Note that when using this image codec API the Codec Family is always "ImageCodec"
 */
GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImageMetaData( void* pluginData                       ,
                                    void* codecData                        ,
                                    const char* codecType                  ,
                                    TIOAccess* input                       ,
                                    TValueMapParserCallbacks* mapCallbacks );

/*---------------------------------------------------------------------------*/

/*
 *
 *  Note that when using this image codec API the Codec Family is always "ImageCodec"
 */
GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_EncodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TImage* inputImage    ,
                            TIOAccess* output     );

/*---------------------------------------------------------------------------*/

/*
 *
 *  Note that when using this image codec API the Codec Family is always "ImageCodec"
 */
GUCEF_CODEC_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_FreeImageStorage( TImage* image   ,
                                 void* imageData );

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
}; /* namespace INPUT */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* IMGPLUGINDEVIL_H ? */

/*--------------------------------------------------------------------------//
//                                                                          //
//      Info & Changes                                                      //
//                                                                          //
//--------------------------------------------------------------------------//

- 02-11-2006 :
        - Created this set of functions.

----------------------------------------------------------------------------*/
