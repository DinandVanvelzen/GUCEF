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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>             /* for memory utils */
#include <string.h>             /* for strncpy */

#include "imgpluginDEVIL.h"     /* function prototypes for this plugin */

#ifndef __IL_H__
#include "il.h"                 /* main API include for the DevIL library */
#define __IL_H__
#endif /* __IL_H__ ? */

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

#define IMGPLUGIN_DEVIL_MAYOR_VERSION       2
#define IMGPLUGIN_DEVIL_MINOR_VERSION       0
#define IMGPLUGIN_DEVIL_PATCH_VERSION       0
#define IMGPLUGIN_DEVIL_RELEASE_VERSION     0

/*---------------------------------------------------------------------------*/

static TIOAccess* currentResource = NULL;
static UInt32 codecCount = 0;
static char** supportedTypes = NULL;
static const char* codecFamily = "ImageCodec";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void ILAPIENTRY
ilfCloseProc( ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        input->close( input );
    }
}

/*---------------------------------------------------------------------------*/

ILboolean ILAPIENTRY 
ilfEofProc( ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->eof( input ) != 0 ? IL_TRUE : IL_FALSE;
    }
    return IL_TRUE;
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfGetcProc( ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->getc( input );
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

ILHANDLE ILAPIENTRY 
ilfOpenProc( const ILstring ilstring )
{
    TIOAccess* input = currentResource;
    if ( NULL != input )
    {
        input->open( input );
    }
    
    return input;
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfReadProc( void* data, 
             ILuint size     , 
             ILuint elements , 
             ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->read( input, data, size, elements );
    }
    return 0;        
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfSeekProc( ILHANDLE handle , 
             ILint offset    , 
             ILint origin    )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->seek( input, offset, origin );
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfTellProc( ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    return input->tell( input );
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfPutcProc( ILubyte putByte , 
             ILHANDLE handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    return input->write( input, &putByte, 1, 1 );
}

/*---------------------------------------------------------------------------*/

ILint ILAPIENTRY
ilfWriteProc( const void* data , 
              ILuint eSize     , 
              ILuint eCount    ,
              ILHANDLE handle  )
{
    TIOAccess* input = (TIOAccess*) handle;
    return input->write( input, data, eSize, eCount );
}

/*---------------------------------------------------------------------------*/

UInt32
GetChannelCountForFormat( const int pixelStorageFormat )
{
    switch ( pixelStorageFormat )
    {
        case PSF_BGR :
        case PSF_RGB :
        {
            return 3;
        }
        case PSF_BGRA :
        case PSF_RGBA :
        {
            return 4;
        }
        case PSF_SINGLE_CHANNEL_RED :
        case PSF_SINGLE_CHANNEL_GREEN :
        case PSF_SINGLE_CHANNEL_BLUE :
        case PSF_SINGLE_CHANNEL_STD_LUMINANCE :
        case PSF_SINGLE_CHANNEL_P1_LUMINANCE :
        case PSF_SINGLE_CHANNEL_P2_LUMINANCE :
        case PSF_SINGLE_CHANNEL_ALPHA :
        {
            return 1;
        }
        default :
        {
            return 0;
        }
    }
}

/*---------------------------------------------------------------------------*/

UInt32
GetPixelChannelSize( const int pixelComponentDataType )
{
    switch ( pixelComponentDataType )
    {
        case GUCEF_DATATYPE_INT8 :
        case GUCEF_DATATYPE_UINT8 :
        {
            return 1;
        }
        case GUCEF_DATATYPE_INT16 :
        case GUCEF_DATATYPE_UINT16 :
        {
            return 2;
        }
        case GUCEF_DATATYPE_INT32 :
        case GUCEF_DATATYPE_UINT32 :
        case GUCEF_DATATYPE_FLOAT32 :
        {
            return 4;
        }
        case GUCEF_DATATYPE_INT64 :
        case GUCEF_DATATYPE_UINT64 :
        case GUCEF_DATATYPE_FLOAT64 :
        {
            return 8;
        }
        default :
        {
            return 0;
        }
    }
}

/*---------------------------------------------------------------------------*/

int
InitCodecList( void )
{
    const char* ilSpaceDelimLoadOpts = NULL;
    int i=0, n=0, m=0, k=0, d=0; 

    /* Currently our API does not differ between loading/saving when calling out supported types, use Load for now */
    ilSpaceDelimLoadOpts = ilGetString( IL_LOAD_EXT );
    if ( ilSpaceDelimLoadOpts == NULL )
        return 0;
    
    k = strlen( ilSpaceDelimLoadOpts );
    codecCount = 0;
    for ( i=0; i<k; ++i )
    {
        if ( ilSpaceDelimLoadOpts[ i ] == ' ' )
        {
            ++codecCount;
        }
    }
    supportedTypes = (char**) malloc( codecCount * sizeof( char* ) );

    n=0; d=0;
    for ( i=0; i<k; ++i )
    {
        if ( ilSpaceDelimLoadOpts[ i ] == ' ' )
        {
            m = i-n;
            char* opt = (char*) malloc( m+1 );
            memcpy( opt, ilSpaceDelimLoadOpts+n, m );
            opt[ m ] = '\0';

            supportedTypes[ d ] = opt;            
            n=i+1; ++d;
        }
    }
    return 1;
}

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
        codecLinks[ i ].codecFamily = codecFamily;
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

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata                    ,
                  TGucefCoreCApi* coreApi            ,
                  const TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{    
    *plugdata = NULL;

    /* Ensure that the version we are build with is not newer then the version we are running with */
    if ( ilGetInteger( IL_VERSION_NUM ) < IL_VERSION ) 
        return GUCEF_CODEC_STATUS_FAILED;   
    
    if ( 0 == InitCodecList() )
        return GUCEF_CODEC_STATUS_FAILED;
    
    ilInit();
    
    ilSetRead( ilfOpenProc  , 
               ilfCloseProc , 
               ilfEofProc   , 
               ilfGetcProc  , 
               ilfReadProc  , 
               ilfSeekProc  , 
               ilfTellProc  );

    ilSetWrite( ilfOpenProc  ,
                ilfCloseProc ,
                ilfPutcProc  , 
                ilfSeekProc  , 
                ilfTellProc  , 
                ilfWriteProc );

    return BuildCodecLinks();
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    currentResource = NULL;
    codecCount = 0;
    free( supportedTypes );
    supportedTypes = NULL;

    ilSetRead( NULL , 
               NULL , 
               NULL , 
               NULL , 
               NULL , 
               NULL , 
               NULL );
               
    ilSetWrite( NULL , 
                NULL , 
                NULL , 
                NULL , 
                NULL , 
                NULL );

    ilShutDown();
    
    FreeCodecLinks();
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "imgpluginDEVIL: image codec collection using DevIL as a backend\0";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. 2002 - 2017.  All rights reserved.\0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;
    
    version.major = IMGPLUGIN_DEVIL_MAYOR_VERSION;
    version.minor = IMGPLUGIN_DEVIL_MINOR_VERSION;
    version.patch = IMGPLUGIN_DEVIL_PATCH_VERSION;
    version.release = IMGPLUGIN_DEVIL_RELEASE_VERSION;
    
    return version;
}

/*---------------------------------------------------------------------------*/

Int32
ConvertILPixelFormatToGUCEFPixelFormat( ILint devilType )
{
    switch ( devilType )
    {
        case IL_RGB : return PSF_RGB;
        case IL_RGBA : return PSF_RGBA;
        case IL_BGR : return PSF_BGR;
        case IL_BGRA : return PSF_BGRA;
        case IL_LUMINANCE : return PSF_SINGLE_CHANNEL_STD_LUMINANCE;
        case IL_LUMINANCE_ALPHA : return PSF_LUMINANCE_ALPHA;
        case IL_COLOUR_INDEX : return PSF_PALETTE_INDICES;
        case IL_ALPHA : return PSF_SINGLE_CHANNEL_ALPHA;
        default: 
            return -1;
    }
}

/*---------------------------------------------------------------------------*/

ILint
ConvertGUCEFPixelFormatToILPixelFormat( Int32 gucefType )
{
    switch ( gucefType )
    {
        case PSF_RGB : return IL_RGB;
        case PSF_RGBA : return IL_RGBA;
        case PSF_BGR : return IL_BGR;
        case PSF_BGRA : return IL_BGRA;
        case PSF_SINGLE_CHANNEL_STD_LUMINANCE : return IL_LUMINANCE;
        case PSF_LUMINANCE_ALPHA : return IL_LUMINANCE_ALPHA;
        case PSF_PALETTE_INDICES : return IL_COLOUR_INDEX;
        case PSF_SINGLE_CHANNEL_ALPHA : return IL_ALPHA;
        default: 
            return -1;
    }    
}

/*---------------------------------------------------------------------------*/

ILint
ConvertGUCEFTypeToILType( Int32 gucefType )
{
    switch ( gucefType )
    {
        case GUCEF_DATATYPE_FLOAT32 : return IL_FLOAT;
        case GUCEF_DATATYPE_FLOAT64 : return IL_DOUBLE;
        case GUCEF_DATATYPE_UINT8 : return IL_UNSIGNED_BYTE;
        case GUCEF_DATATYPE_INT8 : return IL_BYTE;
        case GUCEF_DATATYPE_UINT16 : return IL_UNSIGNED_SHORT;
        case GUCEF_DATATYPE_INT16 : return IL_SHORT;
        case GUCEF_DATATYPE_UINT32 : return IL_UNSIGNED_INT;
        case GUCEF_DATATYPE_INT32 : return IL_INT;
        default: return -1;
    }
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
    UInt32 i=0, n=0, m=0, readBytes=0, imageBufferSize=0, pixeMapSize=0;
    TImageInfo imageInfo;
    TImageFrame* imageFrames = NULL;
    TImageFrameInfo* imageFrameInfo = NULL;
    TImageMipMapLevelInfo* mipInfo = NULL;
    void* imageBuffer = NULL;
    
    /* input sanity check */
    if ( ( NULL == familyType ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == output ) ) return 0;
    
    /* read the image header */    
    readBytes = input->read( input, &imageInfo, sizeof( TImageInfo ), 1 );
    
    /* sanity check */
    if ( ( readBytes != sizeof( TImageInfo ) )                  ||
         ( imageInfo.version != GUCEF_IMAGE_TIMAGEINFO_VERSION ) ) 
    {
        return 0;
    }

    /* First we gather all information */    
    imageFrames = (TImageFrame*) malloc( sizeof( TImageFrame ) * imageInfo.nrOfFramesInImage );
    for ( i=0; i<imageInfo.nrOfFramesInImage; ++i )
    {
        /* read the frame header */
        readBytes = input->read( input, &imageFrames[ i ].frameInfo, sizeof( TImageFrameInfo ), 1 );
        
        /* sanity check */
        if ( ( readBytes != sizeof( TImageFrameInfo ) )                  ||
             ( imageInfo.version != GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION ) ) 
        {
            free( imageFrames );
            return 0;
        }

        /* read the frame's mipmap info */
        imageFrameInfo = &imageFrames[ i ].frameInfo;
        imageFrames[ i ].mipmapLevel = (TImageMipMapLevel*) malloc( sizeof( TImageMipMapLevel ) * imageFrameInfo->nrOfMipmapLevels );        
        for ( n=0; n<imageFrameInfo->nrOfMipmapLevels; ++n )
        {
            /* get a shortcut */
            mipInfo = &imageFrames[ i ].mipmapLevel[ n ].mipLevelInfo;
            
            /* read the mipmap header */
            readBytes = input->read( input, &mipInfo, sizeof( TImageMipMapLevelInfo ), 1 );
            
            /* sanity check */
            if ( ( readBytes != sizeof( TImageMipMapLevelInfo ) )                  ||
                 ( imageInfo.version != GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION ) ) 
            {
                for ( m=0; m<imageInfo.nrOfFramesInImage; ++m ) free( imageFrames[ i ].mipmapLevel );
                free( imageFrames );
                return 0;
            }            
        }
    }
    
    /* generate an image ID and make that ID the ID of the current image */
    ilBindImage( ilGenImage() );

    /* Only 1 layer is supported atm */
    ilActiveLayer( 0 );

    imageBuffer = malloc( 10240 );
    imageBufferSize = 10240;
    for ( i=0; i<imageInfo.nrOfFramesInImage; ++i )
    {
        /* activate the frame */
        ilActiveImage( i );        
        
        imageFrameInfo = &imageFrames[ i ].frameInfo;
        for ( n=0; n<imageFrameInfo->nrOfMipmapLevels; ++n )
        {
            /* activate the mip-map */
            ilActiveMipmap( n );
            
            /* create a shortcut */
            mipInfo = &imageFrames[ i ].mipmapLevel[ n ].mipLevelInfo;
            
            /* calculate the size of the pixel map */
            pixeMapSize = ( GetPixelChannelSize( mipInfo->pixelComponentDataType ) * GetChannelCountForFormat( mipInfo->pixelStorageFormat ) ) * 
                          ( mipInfo->frameWidth * mipInfo->frameHeight )                    ;
                          
            /* check if our local buffer is large enough to hold the pixel data */
            if ( imageBufferSize < pixeMapSize )
            {
                imageBuffer = realloc( imageBuffer, pixeMapSize );
                imageBufferSize = pixeMapSize;
            }
            
            /* read the pixel map */
            readBytes = input->read( input, imageBuffer, pixeMapSize, 1 );
            
            /* sanity check */
            if ( readBytes != pixeMapSize )
            {
                for ( m=n; m<imageInfo.nrOfFramesInImage; ++m ) free( imageFrames[ i ].mipmapLevel );
                free( imageFrames );    
                imageFrames = NULL;
                free( imageBuffer );
                return 0;
            }
            
            /* hand the data over to DevIL */
            if ( IL_TRUE != ilTexImage( (ILuint)mipInfo->frameWidth                                                   ,
                                        (ILuint)mipInfo->frameHeight                                                  ,
                                        (ILuint)1                                                                     ,
                                        (ILubyte) GetChannelCountForFormat( mipInfo->pixelStorageFormat )             ,
                                        (ILenum)ConvertGUCEFPixelFormatToILPixelFormat( mipInfo->pixelStorageFormat ) ,
                                        (ILenum)ConvertGUCEFTypeToILType( mipInfo->pixelComponentDataType )           ,
                                        (void*)imageBuffer                                                            ) )
            {
                /* Failed to transfer the data over to DevIL */
                for ( m=n; m<imageInfo.nrOfFramesInImage; ++m ) free( imageFrames[ i ].mipmapLevel );
                free( imageFrames );    
                imageFrames = NULL;
                free( imageBuffer );
                return 0;
            }
                        
        }
        free( imageFrames[ i ].mipmapLevel );
        imageFrames[ i ].mipmapLevel = NULL;
    }
    free( imageFrames );    
    imageFrames = NULL;
    free( imageBuffer );
    
    /* now we can perform the actual save */
    currentResource = output;
    if ( IL_TRUE == ilSaveF( ilTypeFromExt( codecType ), output ) )
    {
        currentResource = NULL;
        return 1;
    }
    
    currentResource = NULL;
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
    UInt32 i=0, n=0, mipmapCount=0, frameCount=0;
    ILint imageID = 0, imageSize = 0;
    TImage* image = NULL;

    if ( ( NULL == codecType ) || ( NULL == input ) ) 
        return 0;
    
    /* generate an image ID and make that ID the ID of the current image */
    imageID = ilGenImage();
    ilBindImage( imageID );
    
    currentResource = input;
    
    if ( IL_TRUE == ilLoadF( ilTypeFromExt( codecType ), input ) )
    {        
        image = malloc( sizeof(TImage) );
        image->version = GUCEF_IMAGE_TIMAGE_VERSION;
        image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
        image->imageInfo.loopCount = 0;
        frameCount = image->imageInfo.nrOfFramesInImage = (UInt32) ilGetInteger( IL_NUM_IMAGES );
        if ( frameCount == 0 )
        {
            /* DevIL returns a image count of 0 for single-image images */
            frameCount = image->imageInfo.nrOfFramesInImage = 1;
        }
        image->frames = (TImageFrame*) malloc( frameCount * sizeof( TImageFrame ) );

        /* Only 1 layer is supported atm */
        ilActiveLayer( 0 );
        
        for ( i=0; i<frameCount; ++i )
        {
            TImageFrame* imageFrame = &image->frames[ i ]; 
            imageFrame->version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;

            /* activate the frame */
            ilActiveImage( i );
            
            /* init the TImageFrameInfo section */
            imageFrame->frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
            imageFrame->frameInfo.frameDurationInMs = 0; /* @TODO: frame timing is not supported atm */
            mipmapCount = imageFrame->frameInfo.nrOfMipmapLevels = (UInt32) ilGetInteger( IL_NUM_MIPMAPS );
            if ( mipmapCount == 0 )
            {
                /* DevIL returns a mipmap count of 0 images without mipmapping, we use 0 for the base image */
                mipmapCount = imageFrame->frameInfo.nrOfMipmapLevels = 1;
            }
            imageFrame->mipmapLevel = (TImageMipMapLevel*) malloc( mipmapCount * sizeof(TImageMipMapLevel) );
        
            for ( n=0; n<mipmapCount; ++n )
            {
                TImageMipMapLevel* mipmapLevel = &imageFrame->mipmapLevel[ n ]; 
                mipmapLevel->version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
                
                /* activate the mip-map */
                ilActiveMipmap( n );
                
                // Converted paletted images
                if( ilGetInteger( IL_IMAGE_FORMAT ) == IL_COLOUR_INDEX )
                {
                    ilConvertImage( IL_BGRA, IL_UNSIGNED_BYTE );
                }
                
                /* init the TImageMipMapLevelInfo section */
                mipmapLevel->mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
                mipmapLevel->mipLevelInfo.pixelComponentDataType = GUCEF_DATATYPE_UINT8; /* DevIL only supports this type */
                mipmapLevel->mipLevelInfo.frameHeight = ilGetInteger( IL_IMAGE_HEIGHT );
                mipmapLevel->mipLevelInfo.frameWidth = ilGetInteger( IL_IMAGE_WIDTH );
                mipmapLevel->mipLevelInfo.pixelStorageFormat = ConvertILPixelFormatToGUCEFPixelFormat( ilGetInteger( IL_IMAGE_FORMAT ) );

                /* now we grab the pixel data */
                ilCompressFunc( IL_COMPRESS_NONE );
                mipmapLevel->pixelDataSizeInBytes = ilGetInteger( IL_IMAGE_SIZE_OF_DATA );
                mipmapLevel->pixelData = malloc( mipmapLevel->pixelDataSizeInBytes );
                memcpy( mipmapLevel->pixelData, ilGetData(), mipmapLevel->pixelDataSizeInBytes );
            }
        }
    }
    
    /*
     *  the image has been loaded into our GUCEF structures so we have no need for
     *  DevIL to store the data any more 
     */
    ilDeleteImage( imageID );
    
    *imageOutput = image;
    currentResource = NULL;
    
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImageMetaData( void* pluginData                       ,
                                    void* codecData                        ,
                                    const char* codecType                  ,
                                    TIOAccess* input                       ,
                                    TValueMapParserCallbacks* mapCallbacks )
{
    /* this library does not support metadata */
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
    UInt32 i = 0;
    UInt32 n = 0;
    TImageInfo* imageInfo = NULL;
    char codecTypeExt[ 256 ];

    /* generate an image ID and make that ID the ID of the current image */
    ilBindImage( ilGenImage() );

    /* Only 1 layer is supported atm */
    ilActiveLayer( 0 );

    imageInfo = &inputImage->imageInfo;
    for ( i=0; i<imageInfo->nrOfFramesInImage; ++i )
    {
        TImageFrameInfo* imageFrameInfo = &inputImage->frames[ i ].frameInfo;
        
        /* activate the frame */
        ilActiveImage( i );        

        for ( n=0; n<imageFrameInfo->nrOfMipmapLevels; ++n )
        {
            /* create a shortcut */
            TImageMipMapLevelInfo* mipInfo = &inputImage->frames[ i ].mipmapLevel[ n ].mipLevelInfo;
            void* imageBuffer = inputImage->frames[ i ].mipmapLevel[ n ].pixelData;

            /* activate the mip-map */
            ilActiveMipmap( n );
            
            /* hand the data over to DevIL */
            if ( IL_TRUE != ilTexImage( (ILuint)mipInfo->frameWidth                                                   ,
                                        (ILuint)mipInfo->frameHeight                                                  ,
                                        (ILuint)1                                                                     ,
                                        (ILubyte) GetChannelCountForFormat( mipInfo->pixelStorageFormat )             ,
                                        (ILenum)ConvertGUCEFPixelFormatToILPixelFormat( mipInfo->pixelStorageFormat ) ,
                                        (ILenum)ConvertGUCEFTypeToILType( mipInfo->pixelComponentDataType )           ,
                                        (void*)imageBuffer                                                            ) )
            {
                /* Failed to transfer the data over to DevIL */
                return 0;
            }
                        
        }
    }
    
    /* Make sure we prefix a dot before the codecType */    
    strncpy( codecTypeExt+1, codecType, 254 );
    codecTypeExt[ 255 ] = '\0';
    codecTypeExt[ 0 ] = '.';

    /* now we can perform the actual save */
    currentResource = output;
    if ( IL_TRUE == ilSaveF( ilTypeFromExt( codecTypeExt ), output ) )
    {
        currentResource = NULL;
        return 1;
    }
    
    currentResource = NULL;
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

/*---------------------------------------------------------------------------*/
