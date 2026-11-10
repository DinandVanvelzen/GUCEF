/*
 *  imgpluginFreeImage: GUCEF image codec plugin using FreeImage as the backend
 *  Copyright (C) 2002 - 2012.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef IMGPLUGINFREEIMAGE_H
#define IMGPLUGINFREEIMAGE_H

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

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"           /* C API for media abstraction  */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H
#include "gucefCORE_c_valuemap_parser_callbacks.h"
#define GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H
#endif /* GUCEF_CORE_C_VALUEMAP_PARSER_CALLBACKS_H ? */

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_c_imagedata.h"          /* plugin API structures */
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

#ifndef GUCEF_CORE_C_CODEC_PLUGIN_API_H
#include "gucefCORE_c_codec_plugin_api.h"   /* the codec plugin C interface */
#define GUCEF_CORE_C_CODEC_PLUGIN_API_H
#endif /* GUCEF_CORE_C_CODEC_PLUGIN_API_H ? */

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

/*
 *  Below is the image codec API which expands upon the generic codec API
 */

/*---------------------------------------------------------------------------*/

/*
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

#endif /* IMGPLUGINFREEIMAGE_H ? */

/*--------------------------------------------------------------------------//
//                                                                          //
//      Info & Changes                                                      //
//                                                                          //
//--------------------------------------------------------------------------//

- 02-11-2006 :
        - Created this set of functions.

----------------------------------------------------------------------------*/
