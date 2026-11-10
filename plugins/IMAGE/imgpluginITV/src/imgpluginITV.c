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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>             /* for memory utils */
#include <string.h>             /* for strcmp() */

#include "imgpluginITV.h" /* function prototypes for this plugin */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* make sure no structure packing is enabled, this allows us to use the structures directly */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif /* WIN32 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define IMGPLUGIN_ITV_MAYOR_VERSION       0
#define IMGPLUGIN_ITV_MINOR_VERSION       1
#define IMGPLUGIN_ITV_PATCH_VERSION       0
#define IMGPLUGIN_ITV_RELEASE_VERSION     0

/*---------------------------------------------------------------------------*/

static char* supportedTypes[] = {
    "itv1", "itv5"
};

static const UInt32 codecCount = sizeof( supportedTypes ) / sizeof(char*); // 31;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX ImageStreamEncode( void* plugdata, void* codecData, const char* familyType, const char* codecType, TIOAccess* input, TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX ImageStreamDecode( void* pluginData, void* codecData, const char* familyType, const char* codecType, TIOAccess* input, TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

/* The codec link API: codecData is the type name, the image codec API below is called with it */
static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageCodecLinkEncode( void* pluginData                   ,
                      void* codecData                    ,
                      const TVariantMapApi* params    ,
                      TIOAccess* input                ,
                      TIOAccess* output               ,
                      TCodecErrorInfo* error          ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != ImageStreamEncode( pluginData, NULL, "ImageCodec", (const char*) codecData, input, output ) )
        return GUCEF_CODEC_STATUS_OK;
    if ( NULL != error )
    {
        strncpy( error->message, "Unable to encode the image", GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return GUCEF_CODEC_STATUS_FAILED;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageCodecLinkDecode( void* pluginData                   ,
                      void* codecData                    ,
                      const TVariantMapApi* params    ,
                      TIOAccess* input                ,
                      TIOAccess* output               ,
                      TCodecErrorInfo* error          ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != ImageStreamDecode( pluginData, NULL, "ImageCodec", (const char*) codecData, input, output ) )
        return GUCEF_CODEC_STATUS_OK;
    if ( NULL != error )
    {
        strncpy( error->message, "Unable to decode the image", GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return GUCEF_CODEC_STATUS_FAILED;
}

/*---------------------------------------------------------------------------*/

/* One link per supported type, owned by the plugin from Init until Shutdown */
static TCodecPluginLink* codecLinks = NULL;

/*---------------------------------------------------------------------------*/

static TCodecStatus
BuildCodecLinks( void )
{
    UInt32 i = 0;

    free( codecLinks );
    codecLinks = ( TCodecPluginLink* ) calloc( codecCount > 0 ? codecCount : 1, sizeof( TCodecPluginLink ) );
    if ( NULL == codecLinks )
        return GUCEF_CODEC_STATUS_FAILED;

    for ( i=0; i<codecCount; ++i )
    {
        codecLinks[ i ].structSize = sizeof( TCodecPluginLink );
        codecLinks[ i ].codecFamily = "ImageCodec";
        codecLinks[ i ].codecType = supportedTypes[ i ];
        codecLinks[ i ].codecData = (void*) supportedTypes[ i ];
        codecLinks[ i ].capabilities = GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE;
        codecLinks[ i ].encode = ImageCodecLinkEncode;
        codecLinks[ i ].decode = ImageCodecLinkDecode;
    }
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

static void
FreeCodecLinks( void )
{
    free( codecLinks );
    codecLinks = NULL;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return GUCEF_CODECPLUGIN_API_VERSION;
}

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata                    ,
                  TGucefCoreCApi* coreApi            ,
                  const TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

    *plugdata = NULL;
    return BuildCodecLinks();
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

    FreeCodecLinks();
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "imgpluginITV: image codec allowing use of ITV image resource\0";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. 2002 - 2015.  LGPLv3 license\0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = IMGPLUGIN_ITV_MAYOR_VERSION;
    version.minor = IMGPLUGIN_ITV_MINOR_VERSION;
    version.patch = IMGPLUGIN_ITV_PATCH_VERSION;
    version.release = IMGPLUGIN_ITV_RELEASE_VERSION;

    return version;
}

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageStreamEncode( void* plugdata         ,
                    void* codecData        ,
                    const char* familyType ,
                    const char* codecType  ,
                    TIOAccess* input       ,
                    TIOAccess* output      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* Saving images is not supported atm */
    return 0;
}

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageStreamDecode( void* pluginData       ,
                    void* codecData        ,
                    const char* familyType ,
                    const char* codecType  ,
                    TIOAccess* input       ,
                    TIOAccess* output      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TImage* image;
    void* imageData;

    /* input sanity check */
    if ( ( NULL == familyType ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == output ) ) return 0;
    if ( 0 != strcmp( familyType, "ImageCodec" ) ) return 0;

    /* this plugin only supports image codecs so we can redirect to the specialized image API */
    if ( 0 != IMGCODECPLUGIN_DecodeImage( pluginData ,
                                          codecData  ,
                                          codecType  ,
                                          input      ,
                                          &image     ,
                                          &imageData ) )
    {
        /* now that we have the image data we can write it to the output, we start with the metadata */
        UInt32 i, n;

        output->write( output, &image->imageInfo, sizeof( TImageInfo ), 1  );

        for ( i=0; i<image->imageInfo.nrOfFramesInImage; ++i )
        {
            TImageFrame* frame = &image->frames[ i ];
            output->write( output, &frame->frameInfo, sizeof( TImageFrameInfo ), 1 );

            for ( n=0; n<frame->frameInfo.nrOfMipmapLevels; ++n )
            {
                TImageMipMapLevelInfo* imageMMInfo = &frame->mipmapLevel[ n ].mipLevelInfo;
                output->write( output, imageMMInfo, sizeof( TImageMipMapLevelInfo ), 1 );
            }
        }

        /* now write the pixel data */
        for ( i=0; i<image->imageInfo.nrOfFramesInImage; ++i )
        {
            TImageFrame* frame = &image->frames[ i ];
            for ( n=0; n<frame->frameInfo.nrOfMipmapLevels; ++n )
            {
                TImageMipMapLevel* mipMapLevel = &frame->mipmapLevel[ n ];
                output->write( output, mipMapLevel->pixelData, mipMapLevel->pixelDataSizeInBytes, 1 );
            }
        }

        /* Now that we have written the image to output we can deallocate the access structures */
        IMGCODECPLUGIN_FreeImageStorage( image, imageData );

        return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecCount( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return NULL != codecLinks ? codecCount : 0;
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata                          ,
                          UInt32 index                            ,
                          const TCodecPluginLink** codecLink   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( NULL == codecLink )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    if ( NULL == codecLinks || index >= codecCount )
    {
        *codecLink = NULL;
        return GUCEF_CODEC_STATUS_UNSUPPORTED;
    }
    *codecLink = &codecLinks[ index ];
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_FreeImageStorage( TImage* image   ,
                                 void* imageData )
{
    if ( NULL != image )
    {
        /* free the image access structures */
        UInt32 i;
        UInt32 frameCount = image->imageInfo.nrOfFramesInImage;
        for ( i=0; i<frameCount; ++i )
        {
            TImageFrame* frame = &image->frames[ i ];
            free( frame->mipmapLevel );
        }
        free( image->frames );
        free( image );

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32
GetNrOfPixelBuffers( TImage* image )
{
    if ( NULL != image )
    {
        UInt32 i, count=0;
        for ( i=0; i<image->imageInfo.nrOfFramesInImage; ++i )
        {
            TImageFrame* frame = &image->frames[ i ];
            count += frame->frameInfo.nrOfMipmapLevels;
        }
        return count;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TIOAccess* input      ,
                            TImage** imageOutput  ,
                            void** imageData      )
{

    /* input sanity check */
    if ( ( NULL == imageOutput ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == imageOutput ) ) return 0;

    if ( 0 == strcmp( codecType, "itv1" ) )  // Palettized image
    {
        UInt16 ui16 = 0;
        
        /* setup the image struct in the same way for any simple single frame/level image */
        TImage* image = malloc( sizeof(TImage) );
        image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
        image->imageInfo.nrOfFramesInImage = 1;
        image->imageInfo.loopCount = 0;
        image->frames = malloc( sizeof(TImageFrame) );
        image->frames[ 0 ].version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;        
        image->frames[ 0 ].frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
        image->frames[ 0 ].frameInfo.nrOfMipmapLevels = 1;
        image->frames[ 0 ].frameInfo.frameDurationInMs = 0;
        image->frames[ 0 ].mipmapLevel = malloc( sizeof(TImageMipMapLevel) );
        image->frames[ 0 ].mipmapLevel[ 0 ].version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
        image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;

        /* setup properties which are fixed due to the codecType */
        image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.pixelComponentDataType = GUCEF_DATATYPE_UINT8;
        image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.pixelStorageFormat = PSF_PALETTE_INDICES;

        /* skip bytes till image size fields */
        input->setpos( input, 55 );        
        if ( 1 != input->read( input, &ui16, 2, 1 ) )
            { IMGCODECPLUGIN_FreeImageStorage( image, *imageData ); return 0; }
        image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameWidth = ui16;
        if ( 1 != input->read( input, &ui16, 2, 1 ) )
            { IMGCODECPLUGIN_FreeImageStorage( image, *imageData ); return 0; }
        image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameHeight = ui16;

        /* Load the indices */
        image->frames[ 0 ].mipmapLevel[ 0 ].pixelDataSizeInBytes = 
                image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameWidth * image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameHeight;
        image->frames[ 0 ].mipmapLevel[ 0 ].pixelData = malloc( image->frames[ 0 ].mipmapLevel[ 0 ].pixelDataSizeInBytes );
        if ( 1 != input->read( input, image->frames[ 0 ].mipmapLevel[ 0 ].pixelData, image->frames[ 0 ].mipmapLevel[ 0 ].pixelDataSizeInBytes, 1 ) )
            { IMGCODECPLUGIN_FreeImageStorage( image, *imageData ); return 0; }
    
        *imageOutput = image;
        *imageData = NULL;

        return 1;
    }
    else
    if ( 0 == strcmp( codecType, "itv5" ) ) // Image Palette
    {
        UInt16 ui16 = 0;
        UInt64 dataSize = 0;
        
        /* setup the image struct in the same way for any simple single frame/level image */
        TImage* image = malloc( sizeof(TImage) );
        if ( GUCEF_NULL != image )
        {
            image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
            image->imageInfo.nrOfFramesInImage = 1;
        image->imageInfo.loopCount = 0;
            image->frames = malloc( sizeof(TImageFrame) );
            if ( GUCEF_NULL != image->frames )
            {
                image->frames[ 0 ].version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;        
                image->frames[ 0 ].frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
                image->frames[ 0 ].frameInfo.nrOfMipmapLevels = 1;
        image->frames[ 0 ].frameInfo.frameDurationInMs = 0;
                image->frames[ 0 ].mipmapLevel = malloc( sizeof(TImageMipMapLevel) );
                if ( GUCEF_NULL != image->frames[ 0 ].mipmapLevel )
                {
                    image->frames[ 0 ].mipmapLevel[ 0 ].version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
                    image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;

                    /* setup properties which are fixed due to the codecType */
                    image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.pixelComponentDataType = GUCEF_DATATYPE_UINT8;
                    image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.pixelStorageFormat = PSF_RGB;
                }
            }

            input->seek( input, 0, SEEK_END );
            dataSize = input->tell( input );
            dataSize -= 55;

            image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameWidth = dataSize / 3;
            image->frames[ 0 ].mipmapLevel[ 0 ].mipLevelInfo.frameHeight = 1;
            image->frames[ 0 ].mipmapLevel[ 0 ].pixelDataSizeInBytes = dataSize;

            /* skip bytes till color values and read the RGB values */
            input->setpos( input, 55 );        
            image->frames[ 0 ].mipmapLevel[ 0 ].pixelData = malloc( dataSize );
            if ( 1 != input->read( input, image->frames[ 0 ].mipmapLevel[ 0 ].pixelData, dataSize, 1 ) )
                { IMGCODECPLUGIN_FreeImageStorage( image, *imageData ); return 0; }
    
            *imageOutput = image;
            *imageData = NULL;

            return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImageMetaData( void* pluginData                       ,
                                    void* codecData                        ,
                                    const char* codecType                  ,
                                    TIOAccess* input                       ,
                                    TValueMapParserCallbacks* mapCallbacks )
{
    /* this format does not support metadata */
    if ( GUCEF_NULL != mapCallbacks )
    {
        mapCallbacks->OnValueMapBegin( mapCallbacks->privateData, 0, 0 );
        mapCallbacks->OnValueMapEnd( mapCallbacks->privateData );
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_EncodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TImage* inputImage    ,
                            TIOAccess* output     )
{
    /* @TODO */
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* restore structure packing */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(pop)
#else
#pragma pack()
#endif /* WIN32 */

/*-------------------------------------------------------------------------*/
