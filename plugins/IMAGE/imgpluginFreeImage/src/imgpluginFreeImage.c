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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <strings.h>
#define _stricmp strcasecmp   /* the case insensitive compare has another name outside of Windows */
#endif

#include "imgpluginFreeImage.h" /* function prototypes for this plugin */

#ifndef FREEIMAGE_H
#include "FreeImage.h"          /* main API include for the FreeImage library */
#define FREEIMAGE_H
#endif /* FREEIMAGE_H ? */

#ifndef FREEIMAGETAG_H
#include "FreeImageTag.h"
#define FREEIMAGETAG_H
#endif /* FREEIMAGETAG_H ? */

#ifndef GUCEF_IMAGE_IMAGETAGS_H
#include "gucefIMAGE_c_image_tags.h"
#define GUCEF_IMAGE_IMAGETAGS_H
#endif /* GUCEF_IMAGE_IMAGETAGS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SFreeImageDataLink
{
    void* pixelData;
    FIBITMAP* fiData;
    void* packedPixelData;      /**< the rows of fiData without their padding when they have any, else NULL */
};
typedef struct SFreeImageDataLink TFreeImageDataLink;

/*---------------------------------------------------------------------------*/

struct SFreeImageDataLinks
{
    UInt32 linkCount;
    TFreeImageDataLink* links;
};
typedef struct SFreeImageDataLinks TFreeImageDataLinks;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define IMGPLUGIN_FREEIMAGE_MAYOR_VERSION       0
#define IMGPLUGIN_FREEIMAGE_MINOR_VERSION       1
#define IMGPLUGIN_FREEIMAGE_PATCH_VERSION       0
#define IMGPLUGIN_FREEIMAGE_RELEASE_VERSION     0

/*---------------------------------------------------------------------------*/

static FreeImageIO io;

/* the logging of the host, valid from Init until Shutdown */
static TGucefCoreCApi* g_coreApi = NULL;

/* the reasons FreeImage gives for what it refuses go to the log of the host */
static void DLL_CALLCONV
FreeImageMessageProc( FREE_IMAGE_FORMAT fif, const char* message )
{
    char text[ 512 ];
    if ( NULL != g_coreApi && NULL != g_coreApi->Log && NULL != message )
    {
        sprintf( text, "imgpluginFreeImage: %.16s: %.450s", ( fif != FIF_UNKNOWN && NULL != FreeImage_GetFormatFromFIF( fif ) ) ? FreeImage_GetFormatFromFIF( fif ) : "FreeImage", message );
        g_coreApi->Log( GUCEF_LOGMSGTYPE_WARNING, GUCEF_LOGLEVEL_IMPORTANT, text );
    }
}

static char* supportedTypes[] = {
    "bmp"    ,
    "ico"    ,
    "jpeg"   ,
    "jpg"    ,
    "jng"    ,
    "koala"  ,
    "lbm"    ,
    "iff"    ,
    "mng"    ,
    "pbm"    ,
    "pbmraw" ,
    "pcd"    ,
    "pcx"    ,
    "pgm"    ,
    "pgmraw" ,
    "png"    ,
    "ppm"    ,
    "ppmraw" ,
    "ras"    ,
    "tga"    ,
    "targa"  ,
    "tiff"   ,
    "tif"    ,
    "exr"    ,
    "dds"    ,
    "wbmp"   ,
    "psd"    ,
    "cut"    ,
    "xbm"    ,
    "xpm"    ,
    "gif"    ,
    "hdr"    ,
    "faxg3"  ,
    "sgi"
};

static const UInt32 codecCount = sizeof( supportedTypes ) / sizeof(char*); // 31;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

unsigned DLL_CALLCONV
myReadProc( void *buffer     ,
            unsigned size    ,
            unsigned count   ,
            fi_handle handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->read( input, buffer, size, count );
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int DLL_CALLCONV
mySeekProc( fi_handle handle ,
            long offset      ,
            int origin       )
{
    TIOAccess* input = (TIOAccess*) handle;
    if ( NULL != input )
    {
        return input->seek( input, offset, origin );
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

long DLL_CALLCONV
myTellProc( fi_handle handle )
{
    TIOAccess* input = (TIOAccess*) handle;
    return (long) input->tell( input );
}

/*---------------------------------------------------------------------------*/

unsigned DLL_CALLCONV
myWriteProc( void *buffer     ,
             unsigned size    ,
             unsigned count   ,
             fi_handle handle )
{
    TIOAccess* output = (TIOAccess*) handle;
    return output->write( output, buffer, size, count );
}

/*---------------------------------------------------------------------------*/

FREE_IMAGE_FORMAT
GetFileTypeFromExt( const char* ext )
{
    if ( ext != NULL )
    {
        if ( _stricmp( "tga", ext ) == 0 )
        {
            return FIF_TARGA;
        }
        if ( _stricmp( "ppm", ext ) == 0 )
        {
            return FIF_PPM;
        }
        if ( _stricmp( "jpg", ext ) == 0 || _stricmp( "jpeg", ext ) == 0 )
        {
            return FIF_JPEG;
        }
        if ( _stricmp( "ico", ext ) == 0 )
        {
            return FIF_ICO;
        }
        if ( _stricmp( "bmp", ext ) == 0 )
        {
            return FIF_BMP;
        }
        if ( _stricmp( "pcx", ext ) == 0 )
        {
            return FIF_PCX;
        }
    }
    return FIF_UNKNOWN;
}

/*---------------------------------------------------------------------------*/

/**
 *  The FreeImage format of a type, which is a file extension. FreeImage knows its formats by name ("PNG", "JPEG", "TARGA"),
 *  so the extensions that differ from the name are looked up as the extension of a file name.
 */
FREE_IMAGE_FORMAT
GetFIFOfType( const char* type )
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    if ( NULL == type )
        return FIF_UNKNOWN;

    fif = FreeImage_GetFIFFromFormat( type );
    if ( fif == FIF_UNKNOWN )
    {
        fif = GetFileTypeFromExt( type );
    }
    if ( fif == FIF_UNKNOWN && strlen( type ) < 32 )
    {
        char fileName[ 40 ];
        strcpy( fileName, "x." );
        strcat( fileName, type );
        fif = FreeImage_GetFIFFromFilename( fileName );
    }
    return fif;
}

/*---------------------------------------------------------------------------*/

int
ConvertToPCDT( const FREE_IMAGE_TYPE imageType )
{
    switch( imageType )
    {
        case FIT_BITMAP :
        {
            return GUCEF_DATATYPE_UINT8;
        }
        case FIT_COMPLEX :
        case FIT_UNKNOWN :
        {
            return GUCEF_DATATYPE_UNKNOWN;
        }
        case FIT_UINT32 :
        {
            return GUCEF_DATATYPE_UINT32;
        }
        case FIT_INT32 :
        {
            return GUCEF_DATATYPE_INT32;
        }
        case FIT_FLOAT :
        {
            return GUCEF_DATATYPE_FLOAT32;
        }
        case FIT_DOUBLE :
        {
            return GUCEF_DATATYPE_FLOAT64;
        }
        case FIT_UINT16 :
        {
            return GUCEF_DATATYPE_UINT16;
        }
        case FIT_INT16 :
        {
            return GUCEF_DATATYPE_INT16;
        }
        case FIT_RGBF :
        {
            return GUCEF_DATATYPE_FLOAT32;
        }
        case FIT_RGB16 :
        {
            return GUCEF_DATATYPE_UINT16;
        }
        case FIT_RGBAF :
        {
            return GUCEF_DATATYPE_FLOAT32;
        }
        case FIT_RGBA16 :
        {
            return GUCEF_DATATYPE_UINT16;
        }
        default :
        {
            return GUCEF_DATATYPE_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

int
ConvertToPSF( const FREE_IMAGE_TYPE imageType       ,
              const FREE_IMAGE_COLOR_TYPE colorType )
{
    switch( imageType )
    {
        case FIT_BITMAP :
        {
            switch ( colorType )
            {
                case FIC_RGB :
                {
                    /* FreeImage keeps the channels in the order of the platform */
                    #if ( FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR )
                    return PSF_BGR;
                    #else
                    return PSF_RGB;
                    #endif
                }
                case FIC_RGBALPHA :
                {
                    #if ( FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR )
                    return PSF_BGRA;
                    #else
                    return PSF_RGBA;
                    #endif
                }
                case FIC_MINISBLACK :
                {
                    /* a greyscale picture: the loader turns every grey form into 8 bits with black at 0 */
                    return PSF_SINGLE_CHANNEL_GRAYSCALE;
                }
                default :
                {
                    return PSF_UNKNOWN;
                }
            }
        }
        case FIT_UNKNOWN :
        {
            return PSF_UNKNOWN;
        }
        case FIT_COMPLEX :
        case FIT_UINT32 :
        case FIT_INT32 :
        case FIT_FLOAT :
        case FIT_DOUBLE :
        case FIT_UINT16 :
        case FIT_INT16 :
        {
            return PSF_SINGLE_CHANNEL_STD_LUMINANCE;
        }
        case FIT_RGBF :
        case FIT_RGB16 :
        {
            return PSF_RGB;
        }
        case FIT_RGBAF :
        case FIT_RGBA16 :
        {
            return PSF_RGBA;
        }
        default :
        {
            return PSF_UNKNOWN;
        }
    }
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
        case PSF_SINGLE_CHANNEL_GRAYSCALE :
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
        default :
        {
            return 0;
        }
    }
}

/*---------------------------------------------------------------------------*/

UInt32
GetPixelSize( const int pixelStorageFormat     ,
              const int pixelComponentDataType )
{
    return GetChannelCountForFormat( pixelStorageFormat ) * GetPixelChannelSize( pixelComponentDataType );
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
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

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
        codecLinks[ i ].decode = ImageCodecLinkDecode;

        /* reading is by the signature of the data, writing needs the format: only the formats that FreeImage can write advertise it */
        fif = GetFIFOfType( supportedTypes[ i ] );
        if ( fif != FIF_UNKNOWN && FreeImage_FIFSupportsWriting( fif ) )
        {
            codecLinks[ i ].capabilities = GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE;
            codecLinks[ i ].encode = ImageCodecLinkEncode;
        }
        else
        {
            codecLinks[ i ].capabilities = GUCEF_CODEC_CAPABILITY_DECODE;
            codecLinks[ i ].encode = NULL;
        }

        /* animations are read and written as all their frames */
        if ( FIF_GIF == fif )
        {
            codecLinks[ i ].capabilities |= GUCEF_CODEC_CAPABILITY_MULTIPLE_FRAMES;
        }
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
	int fiMayorVersion=0, fiMinorVersion=0, fiReleaseVersion=0;

	/* call this ONLY when linking with FreeImage as a static library */
    #ifdef FREEIMAGE_LIB
    FreeImage_Initialise();
    #endif /* FREEIMAGE_LIB ? */

    /* make sure that the linked back-end library is the same as the one we compiled against */
    sscanf( FreeImage_GetVersion(), "%d %*c %d %*c %d", &fiMayorVersion, &fiMinorVersion, &fiReleaseVersion );
    if ( fiMayorVersion != FREEIMAGE_MAJOR_VERSION   ||
         fiMinorVersion != FREEIMAGE_MINOR_VERSION   ||
         fiReleaseVersion != FREEIMAGE_RELEASE_SERIAL ) return GUCEF_CODEC_STATUS_FAILED;

    io.read_proc  = myReadProc;
    io.write_proc = myWriteProc;
    io.seek_proc  = mySeekProc;
    io.tell_proc  = myTellProc;

    g_coreApi = coreApi;
    FreeImage_SetOutputMessage( FreeImageMessageProc );
    *plugdata = NULL;
    return BuildCodecLinks();
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    memset( &io, 0, sizeof( io ) );
    FreeImage_SetOutputMessage( NULL );
    g_coreApi = NULL;

    /* call this ONLY when linking with FreeImage as a static library */
    #ifdef FREEIMAGE_LIB
    FreeImage_DeInitialise();
    #endif /* FREEIMAGE_LIB ? */

    FreeCodecLinks();
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "imgpluginFreeImage: image codec collection using FreeImage as a backend\0";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. 2002 - 2012.  LGPLv3 license. Refer to FreeImage for its license\0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = IMGPLUGIN_FREEIMAGE_MAYOR_VERSION;
    version.minor = IMGPLUGIN_FREEIMAGE_MINOR_VERSION;
    version.patch = IMGPLUGIN_FREEIMAGE_PATCH_VERSION;
    version.release = IMGPLUGIN_FREEIMAGE_RELEASE_VERSION;

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
    /*
     *  The input is the decoded 'ImageCodec' stream, as ImageStreamDecode writes it: the image info, for every frame
     *  its info followed by that of its mipmap levels, then the pixels of every level. The formats this plugin writes
     *  hold one picture: the first level of the first frame is encoded
     */
    TImageInfo imageInfo;
    TImageFrameInfo frameInfo;
    TImageFrameInfo firstFrameInfo;
    TImageMipMapLevelInfo levelInfo;
    TImageMipMapLevelInfo firstLevelInfo;
    TImage image;
    TImageFrame frame;
    TImageMipMapLevel level;
    UInt32 i=0, n=0, pixelSize=0, result=0;
    UInt64 pixelDataSize=0;
    void* pixelData = NULL;

    /* input sanity check */
    if ( ( NULL == familyType ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == output ) ) return 0;
    if ( 0 != strcmp( familyType, "ImageCodec" ) ) return 0;

    memset( &firstFrameInfo, 0, sizeof( firstFrameInfo ) );
    memset( &firstLevelInfo, 0, sizeof( firstLevelInfo ) );
    if ( 1 != input->read( input, &imageInfo, sizeof( TImageInfo ), 1 ) ||
         GUCEF_IMAGE_TIMAGEINFO_VERSION != imageInfo.version         ||
         0 == imageInfo.nrOfFramesInImage                            )
        return 0;
    for ( i=0; i<imageInfo.nrOfFramesInImage; ++i )
    {
        if ( 1 != input->read( input, &frameInfo, sizeof( TImageFrameInfo ), 1 ) ||
             GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION != frameInfo.version         ||
             ( 0 == i && 0 == frameInfo.nrOfMipmapLevels )                   )
            return 0;
        if ( 0 == i )
            firstFrameInfo = frameInfo;
        for ( n=0; n<frameInfo.nrOfMipmapLevels; ++n )
        {
            if ( 1 != input->read( input, &levelInfo, sizeof( TImageMipMapLevelInfo ), 1 ) ||
                 GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION != levelInfo.version         )
                return 0;
            if ( 0 == i && 0 == n )
                firstLevelInfo = levelInfo;
        }
    }

    /* the pixels of the first level come first */
    pixelSize = GetPixelSize( firstLevelInfo.pixelStorageFormat, firstLevelInfo.pixelComponentDataType );
    pixelDataSize = (UInt64) pixelSize * firstLevelInfo.frameWidth * firstLevelInfo.frameHeight;
    if ( 0 == pixelDataSize || pixelDataSize > GUCEF_UINT32MAX ) return 0;
    pixelData = malloc( (size_t) pixelDataSize );
    if ( NULL == pixelData ) return 0;
    if ( 1 != input->read( input, pixelData, (UInt32) pixelDataSize, 1 ) )
    {
        free( pixelData );
        return 0;
    }

    level.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
    level.mipLevelInfo = firstLevelInfo;
    level.pixelData = pixelData;
    level.pixelDataSizeInBytes = (UInt32) pixelDataSize;
    frame.version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;
    frame.frameInfo = firstFrameInfo;
    frame.frameInfo.nrOfMipmapLevels = 1;
    frame.mipmapLevel = &level;
    image.version = GUCEF_IMAGE_TIMAGE_VERSION;
    image.imageInfo = imageInfo;
    image.imageInfo.nrOfFramesInImage = 1;
    image.frames = &frame;

    result = IMGCODECPLUGIN_EncodeImage( plugdata, codecData, codecType, &image, output );
    free( pixelData );
    return result;
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
        TFreeImageDataLinks* allImageDataLinks = (TFreeImageDataLinks*) imageData;

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

        /* free the actual FreeImage image storage */
        for ( i=0; i<allImageDataLinks->linkCount; ++i )
        {
            TFreeImageDataLink* imageDataLink = &allImageDataLinks->links[ i ];
            free( imageDataLink->packedPixelData );
            FreeImage_Unload( imageDataLink->fiData );
        }
        free( allImageDataLinks->links );
        free( allImageDataLinks );

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

BOOL
ConvertToFreeImageNative( TImageMipMapLevel* level, UInt8** adjustedBuffer )
{
    #if ( FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR )

    if ( PSF_RGB == level->mipLevelInfo.pixelStorageFormat ||
         PSF_RGBA == level->mipLevelInfo.pixelStorageFormat )
    {    
        UInt32 sizeOfChannelInBytes = GetPixelChannelSize( level->mipLevelInfo.pixelComponentDataType );
        UInt32 channelCount = GetChannelCountForFormat( level->mipLevelInfo.pixelStorageFormat );
        UInt32 pixelSizeInBytes = sizeOfChannelInBytes * channelCount;
        UInt32 pixelCount = level->pixelDataSizeInBytes / pixelSizeInBytes;
        UInt32 i=0;

        UInt8* pixelData = (UInt8*) level->pixelData;    
        UInt8* swapBuffer = (UInt8*) malloc( sizeOfChannelInBytes );        

        *adjustedBuffer = (UInt8*) malloc( level->pixelDataSizeInBytes );
        memcpy( *adjustedBuffer, pixelData, level->pixelDataSizeInBytes );
        pixelData = *adjustedBuffer;

        for ( i=0; i<pixelCount; ++i )
        {
            UInt8* pixelAddrA = pixelData + ( i * pixelSizeInBytes );
            UInt8* pixelAddrB = pixelData + ( i * pixelSizeInBytes ) + ( 2 * sizeOfChannelInBytes );
            memcpy( swapBuffer, pixelAddrA, sizeOfChannelInBytes );
            memcpy( pixelAddrA, pixelAddrB, sizeOfChannelInBytes );
            memcpy( pixelAddrB, swapBuffer, sizeOfChannelInBytes );
        }

        free( swapBuffer );

        return TRUE;
    }

    #elif ( FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB )

    if ( PSF_BGR == level->mipLevelInfo.pixelStorageFormat ||
         PSF_BGRA == level->mipLevelInfo.pixelStorageFormat )
    {    
        UInt32 sizeOfChannelInBytes = GetPixelChannelSize( level->mipLevelInfo.pixelComponentDataType );
        UInt32 channelCount = GetChannelCountForFormat( level->mipLevelInfo.pixelStorageFormat );
        UInt32 pixelSizeInBytes = sizeOfChannelInBytes * channelCount;
        UInt32 pixelCount = level->pixelDataSizeInBytes / pixelSizeInBytes;
        UInt32 i=0;

        UInt8* pixelData = (UInt8*) level->pixelData;    
        UInt8* swapBuffer = (UInt8*) malloc( sizeOfChannelInBytes );

        *adjustedBuffer = (UInt8*) malloc( level->pixelDataSizeInBytes );
        memcpy( *adjustedBuffer, pixelData, level->pixelDataSizeInBytes );
        pixelData = *adjustedBuffer;

        for ( i=0; i<pixelCount; ++i )
        {
            UInt8* pixelAddrA = pixelData + ( i * pixelSizeInBytes );
            UInt8* pixelAddrB = pixelData + ( i * pixelSizeInBytes ) + ( 2 * sizeOfChannelInBytes );
            memcpy( swapBuffer, pixelAddrA, sizeOfChannelInBytes );
            memcpy( pixelAddrA, pixelAddrB, sizeOfChannelInBytes );
            memcpy( pixelAddrB, swapBuffer, sizeOfChannelInBytes );
        }

        free( swapBuffer );

        return TRUE;
    }

    #endif

    *adjustedBuffer = (UInt8*) level->pixelData;
    return FALSE;
}

/*---------------------------------------------------------------------------*/

/**
 *  Brings a loaded picture to what the image interface has: 8 bit grey, 24 bit RGB or 32 bit RGBA, with the top row first.
 *  Takes over the bitmap and gives the one to use, NULL when it cannot be made
 */
static FIBITMAP*
PrepareLoadedBitmap( FIBITMAP* dib, FREE_IMAGE_COLOR_TYPE* colourType )
{
    UInt32 bpp = FreeImage_GetBPP( dib );
    *colourType = FreeImage_GetColorType( dib );

    if ( FIT_BITMAP == FreeImage_GetImageType( dib ) )
    {
        if ( ( bpp <= 8 ) && FreeImage_IsTransparent( dib ) )
        {
            /* a palette or grey picture with transparency (a PNG tRNS chunk): RGBA keeps the alpha of every entry,
               which the conversions to 8 or 24 bits below would drop */
            FIBITMAP* newDIB = FreeImage_ConvertTo32Bits( dib );
            if ( NULL != newDIB )
            {
                FreeImage_Unload( dib );
                dib = newDIB;
                *colourType = FreeImage_GetColorType( dib );
                bpp = FreeImage_GetBPP( dib );
            }
        }

        if ( bpp < 8 )
        {
            /* we do not support a format that has less that 8 bits per pixel */
            FIBITMAP* newDIB = FreeImage_ConvertTo8Bits( dib );
            FreeImage_Unload( dib );
            dib = newDIB;
            if ( NULL == dib )
                return NULL;
            *colourType = FreeImage_GetColorType( dib );
            bpp = FreeImage_GetBPP( dib );
        }

        /* Now that we know the BPP is always >= 8 we can check the color format */
        if ( ( *colourType == FIC_MINISWHITE ) ||
             ( *colourType == FIC_MINISBLACK )  )
        {
            /* custom color ranges are not supported */
            FIBITMAP* newDIB = FreeImage_ConvertToGreyscale( dib );
            FreeImage_Unload( dib );
            dib = newDIB;
            if ( NULL == dib )
                return NULL;
            *colourType = FreeImage_GetColorType( dib );
            bpp = FreeImage_GetBPP( dib );
        }
        else
        if ( ( *colourType == FIC_PALETTE ) ||
             ( *colourType == FIC_CMYK )     )
        {
            /* palettes are not supported */
            FIBITMAP* newDIB = FreeImage_ConvertTo24Bits( dib );
            FreeImage_Unload( dib );
            dib = newDIB;
            if ( NULL == dib )
                return NULL;
            *colourType = FreeImage_GetColorType( dib );
            bpp = FreeImage_GetBPP( dib );
        }

        /* At this point images are always greyscale or RGB or RGBA and BBP is always >= 8 */
    }

    if ( bpp == 32 && *colourType == FIC_RGB )
    {
        /* This is 32bpp thus it is actually RGBA but the Alpha layer has no values thus FreeImage says its RGB
           This screws up usage, correct here */
        *colourType = FIC_RGBALPHA;
    }

    /* FreeImage keeps the bottom row first, a pixel map has the top row first */
    FreeImage_FlipVertical( dib );
    return dib;
}

/*---------------------------------------------------------------------------*/

/** What was loaded: the pictures of the frames with how long each is shown, and how often the sequence plays */
#define FREEIMAGE_MAX_FRAMES 65536

struct SLoadedFrames
{
    UInt32 frameCount;
    FIBITMAP** bitmaps;
    LONG* durationsInMs;
    UInt32 loopCount;
};
typedef struct SLoadedFrames TLoadedFrames;

/*---------------------------------------------------------------------------*/

static void
FreeLoadedFrames( TLoadedFrames* frames )
{
    UInt32 i;
    if ( NULL != frames->bitmaps )
    {
        for ( i=0; i<frames->frameCount; ++i )
            if ( NULL != frames->bitmaps[ i ] )
                FreeImage_Unload( frames->bitmaps[ i ] );
    }
    free( frames->bitmaps );
    free( frames->durationsInMs );
    memset( frames, 0, sizeof( TLoadedFrames ) );
}

/*---------------------------------------------------------------------------*/

/** Reads one animation tag of a frame as a number, 0 when the frame does not have it */
static LONG
GetAnimationTagValue( FIBITMAP* dib, const char* key )
{
    FITAG* tag = NULL;
    if ( FreeImage_GetMetadata( FIMD_ANIMATION, dib, key, &tag ) && NULL != tag && FIDT_LONG == FreeImage_GetTagType( tag ) && NULL != FreeImage_GetTagValue( tag ) )
        return *(LONG*) FreeImage_GetTagValue( tag );
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 *  Loads the frames of an animated GIF: FreeImage plays the animation back so every frame is a complete 32 bit picture,
 *  with the time it is shown. False when the data is not a GIF that can be played
 */
static BOOL
LoadGifFrames( TIOAccess* input, TLoadedFrames* frames )
{
    FIMULTIBITMAP* multiBitmap = NULL;
    int pageCount = 0, page = 0;
    Int64 start = (Int64) input->tell( input );

    multiBitmap = FreeImage_OpenMultiBitmapFromHandle( FIF_GIF, &io, (fi_handle) input, GIF_PLAYBACK );
    if ( NULL == multiBitmap )
        return FALSE;

    pageCount = FreeImage_GetPageCount( multiBitmap );
    if ( pageCount < 1 || pageCount > FREEIMAGE_MAX_FRAMES )
    {
        FreeImage_CloseMultiBitmap( multiBitmap, 0 );
        return FALSE;
    }

    frames->bitmaps = (FIBITMAP**) calloc( pageCount, sizeof( FIBITMAP* ) );
    frames->durationsInMs = (LONG*) calloc( pageCount, sizeof( LONG ) );
    if ( NULL == frames->bitmaps || NULL == frames->durationsInMs )
    {
        FreeImage_CloseMultiBitmap( multiBitmap, 0 );
        FreeLoadedFrames( frames );
        return FALSE;
    }
    frames->frameCount = (UInt32) pageCount;

    for ( page=0; page<pageCount; ++page )
    {
        FIBITMAP* pageBitmap = FreeImage_LockPage( multiBitmap, page );
        if ( NULL == pageBitmap )
        {
            FreeImage_CloseMultiBitmap( multiBitmap, 0 );
            FreeLoadedFrames( frames );
            return FALSE;
        }
        frames->bitmaps[ page ] = FreeImage_Clone( pageBitmap );
        frames->durationsInMs[ page ] = GetAnimationTagValue( pageBitmap, "FrameTime" );
        FreeImage_UnlockPage( multiBitmap, pageBitmap, 0 );

        if ( NULL == frames->bitmaps[ page ] )
        {
            FreeImage_CloseMultiBitmap( multiBitmap, 0 );
            FreeLoadedFrames( frames );
            return FALSE;
        }
    }
    FreeImage_CloseMultiBitmap( multiBitmap, 0 );

    /* the loop count belongs to the file and is only given with the first frame as it is stored, not with the played back ones */
    input->seek( input, start, SEEK_SET );
    {
        FIBITMAP* firstFrame = FreeImage_LoadFromHandle( FIF_GIF, &io, (fi_handle) input, GIF_DEFAULT );
        if ( NULL != firstFrame )
        {
            frames->loopCount = (UInt32) GetAnimationTagValue( firstFrame, "Loop" );
            FreeImage_Unload( firstFrame );
        }
    }
    return TRUE;
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
    UInt32 i=0, frameCount=0;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    TLoadedFrames loaded;
    FREE_IMAGE_COLOR_TYPE* colourTypes = NULL;
    TImage* image = NULL;
    TFreeImageDataLinks* imageDataLinks = NULL;
    Int64 start = 0;

    /* input sanity check */
    if ( ( NULL == imageOutput ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == imageData ) ) return 0;
    memset( &loaded, 0, sizeof( loaded ) );
    start = (Int64) input->tell( input );

    fif = FreeImage_GetFileTypeFromHandle( &io, (fi_handle) input, 0 );
    if ( fif == FIF_UNKNOWN )
    {
        fif = GetFIFOfType( codecType );
    }
    if ( fif == FIF_UNKNOWN )
        return 0;

    /* an animated GIF is played back to its frames, everything else is a single picture */
    if ( FIF_GIF != fif || !LoadGifFrames( input, &loaded ) )
    {
        FIBITMAP* dib = NULL;
        if ( FIF_GIF == fif )
        {
            /* the GIF could not be played: it is a single picture then, read from where it started again */
            input->seek( input, start, SEEK_SET );
        }
        dib = FreeImage_LoadFromHandle( fif, &io, (fi_handle) input, 0 );
        if ( NULL == dib )
            return 0;

        loaded.bitmaps = (FIBITMAP**) calloc( 1, sizeof( FIBITMAP* ) );
        loaded.durationsInMs = (LONG*) calloc( 1, sizeof( LONG ) );
        if ( NULL == loaded.bitmaps || NULL == loaded.durationsInMs )
        {
            FreeImage_Unload( dib );
            free( loaded.bitmaps );
            free( loaded.durationsInMs );
            return 0;
        }
        loaded.frameCount = 1;
        loaded.bitmaps[ 0 ] = dib;
    }
    frameCount = loaded.frameCount;

    /* bring every frame to what the image interface has */
    colourTypes = (FREE_IMAGE_COLOR_TYPE*) calloc( frameCount, sizeof( FREE_IMAGE_COLOR_TYPE ) );
    if ( NULL == colourTypes )
    {
        FreeLoadedFrames( &loaded );
        return 0;
    }
    for ( i=0; i<frameCount; ++i )
    {
        loaded.bitmaps[ i ] = PrepareLoadedBitmap( loaded.bitmaps[ i ], &colourTypes[ i ] );
        if ( NULL == loaded.bitmaps[ i ] )
        {
            free( colourTypes );
            FreeLoadedFrames( &loaded );
            return 0;
        }
    }

    image = (TImage*) malloc( sizeof( TImage ) );
    image->version = GUCEF_IMAGE_TIMAGE_VERSION;
    image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
    image->imageInfo.loopCount = loaded.loopCount;
    image->imageInfo.nrOfFramesInImage = frameCount;
    image->frames = (TImageFrame*) malloc( frameCount * sizeof( TImageFrame ) );

    /* a link per picture, which lets the storage of the image be freed with the FreeImage bitmaps it points into */
    imageDataLinks = (TFreeImageDataLinks*) malloc( sizeof( TFreeImageDataLinks ) );
    imageDataLinks->linkCount = frameCount;
    imageDataLinks->links = (TFreeImageDataLink*) calloc( frameCount, sizeof( TFreeImageDataLink ) );

    for ( i=0; i<frameCount; ++i )
    {
        TImageFrame* frame = &image->frames[ i ];
        TImageMipMapLevel* mipmapLevel = NULL;
        TFreeImageDataLink* dataLink = &imageDataLinks->links[ i ];
        FIBITMAP* dib = loaded.bitmaps[ i ];
        FREE_IMAGE_TYPE fit = FreeImage_GetImageType( dib );
        UInt32 lineSize = FreeImage_GetLine( dib );
        UInt32 pitch = FreeImage_GetPitch( dib );

        frame->version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;
        frame->frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
        frame->frameInfo.frameDurationInMs = loaded.durationsInMs[ i ] > 0 ? (UInt32) loaded.durationsInMs[ i ] : 0;
        frame->frameInfo.nrOfMipmapLevels = 1; /* @TODO: mip-mapping is not supported atm */
        frame->mipmapLevel = (TImageMipMapLevel*) malloc( sizeof( TImageMipMapLevel ) );

        mipmapLevel = &frame->mipmapLevel[ 0 ];
        mipmapLevel->version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
        mipmapLevel->mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
        mipmapLevel->mipLevelInfo.frameHeight = FreeImage_GetHeight( dib );
        mipmapLevel->mipLevelInfo.frameWidth = FreeImage_GetWidth( dib );
        mipmapLevel->mipLevelInfo.pixelComponentDataType = ConvertToPCDT( fit );
        mipmapLevel->mipLevelInfo.pixelStorageFormat = ConvertToPSF( fit, colourTypes[ i ] );
        mipmapLevel->pixelDataSizeInBytes = lineSize * mipmapLevel->mipLevelInfo.frameHeight;

        /* the bitmap now belongs to the image: its link frees it */
        dataLink->fiData = dib;
        loaded.bitmaps[ i ] = NULL;

        if ( lineSize == pitch )
        {
            mipmapLevel->pixelData = FreeImage_GetBits( dib );
        }
        else
        {
            /* FreeImage pads every row to a multiple of 4 bytes, the rows of a pixel map follow each other */
            UInt32 y = 0;
            UInt8* packed = (UInt8*) malloc( mipmapLevel->pixelDataSizeInBytes );
            if ( NULL == packed )
            {
                /* the frames up to this one are complete: free what the image holds, and the bitmaps that are not in it yet */
                image->imageInfo.nrOfFramesInImage = i + 1;
                mipmapLevel->pixelData = NULL;
                IMGCODECPLUGIN_FreeImageStorage( image, imageDataLinks );
                free( colourTypes );
                FreeLoadedFrames( &loaded );
                return 0;
            }
            for ( y=0; y<mipmapLevel->mipLevelInfo.frameHeight; ++y )
                memcpy( packed + ( y * lineSize ), FreeImage_GetScanLine( dib, (int) y ), lineSize );
            dataLink->packedPixelData = packed;
            mipmapLevel->pixelData = packed;
        }
        dataLink->pixelData = mipmapLevel->pixelData;
    }

    free( colourTypes );
    FreeLoadedFrames( &loaded );       /* only what was not handed to the image */

    /* we successfully loaded and processed the image */
    *imageOutput = image;
    *imageData = imageDataLinks;
    return 1;
}

/*---------------------------------------------------------------------------*/

DWORD
MapFreeImageMetaDataTagIdForExifGPS( WORD fiTagId )
{
    switch ( fiTagId )
    {
        case TAG_GPS_VERSION_ID: return GUCEF_IMAGE_TAG_EXIF_GPS_VERSION_ID;
        case TAG_GPS_LATITUDE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_LATITUDE_REF;
        case TAG_GPS_LATITUDE: return GUCEF_IMAGE_TAG_EXIF_GPS_LATITUDE;
        case TAG_GPS_LONGITUDE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_LONGITUDE_REF;
        case TAG_GPS_LONGITUDE: return GUCEF_IMAGE_TAG_EXIF_GPS_LONGITUDE;
        case TAG_GPS_ALTITUDE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_ALTITUDE_REF;
        case TAG_GPS_ALTITUDE: return GUCEF_IMAGE_TAG_EXIF_GPS_ALTITUDE;
        case TAG_GPS_SATELLITES: return GUCEF_IMAGE_TAG_EXIF_GPS_SATELLITES;
        case TAG_GPS_STATUS: return GUCEF_IMAGE_TAG_EXIF_GPS_STATUS;
        case TAG_GPS_DOP: return GUCEF_IMAGE_TAG_EXIF_GPS_DOP;  
        case TAG_GPS_SPEED_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_SPEED_REF;
        case TAG_GPS_SPEED: return GUCEF_IMAGE_TAG_EXIF_GPS_SPEED;
        case TAG_GPS_TRACK_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_TRACK_REF;
        case TAG_GPS_TRACK: return GUCEF_IMAGE_TAG_EXIF_GPS_TRACK;
        case TAG_GPS_IMG_DIRECTION_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_IMG_DIRECTION_REF;
        case TAG_GPS_IMG_DIRECTION: return GUCEF_IMAGE_TAG_EXIF_GPS_IMG_DIRECTION;
        case TAG_GPS_MAP_DATUM: return GUCEF_IMAGE_TAG_EXIF_GPS_MAP_DATUM;
        case TAG_GPS_DEST_LATITUDE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_LATITUDE_REF;
        case TAG_GPS_DEST_LATITUDE: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_LATITUDE;
        case TAG_GPS_DEST_LONGITUDE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_LONGITUDE_REF;
        case TAG_GPS_DEST_LONGITUDE: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_LONGITUDE;
        case TAG_GPS_DEST_BEARING_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_BEARING_REF;
        case TAG_GPS_DEST_BEARING: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_BEARING;
        case TAG_GPS_DEST_DISTANCE_REF: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_DISTANCE_REF;
        case TAG_GPS_DEST_DISTANCE: return GUCEF_IMAGE_TAG_EXIF_GPS_DEST_DISTANCE;
        case TAG_GPS_PROCESSING_METHOD: return GUCEF_IMAGE_TAG_EXIF_GPS_PROCESSING_METHOD;
        case TAG_GPS_AREA_INFORMATION: return GUCEF_IMAGE_TAG_EXIF_GPS_AREA_INFORMATION;
        case TAG_GPS_DIFFERENTIAL: return GUCEF_IMAGE_TAG_EXIF_GPS_DIFFERENTIAL;

        default: 
        {
            return GUCEF_IMAGE_TAG_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

DWORD
MapFreeImageMetaDataTagIdForExifMain( WORD fiTagId )
{
    switch ( fiTagId )
    {
        case TAG_IMAGE_WIDTH: return GUCEF_IMAGE_TAG_EXIF_IFD_IMAGE_WIDTH;
        case TAG_IMAGE_HEIGHT: return GUCEF_IMAGE_TAG_EXIF_IFD_IMAGE_HEIGHT;
        case TAG_BITS_PER_SAMPLE: return GUCEF_IMAGE_TAG_EXIF_IFD_BITS_PER_SAMPLE;
        case TAG_COMPRESSION: return GUCEF_IMAGE_TAG_EXIF_IFD_COMPRESSION;
        case TAG_PHOTOMETRIC_INTERPRETATION: return GUCEF_IMAGE_TAG_EXIF_IFD_PHOTOMETRIC_INTERPRETATION;
        case TAG_ORIENTATION: return GUCEF_IMAGE_TAG_EXIF_IFD_ORIENTATION;
        case TAG_SAMPLES_PER_PIXEL: return GUCEF_IMAGE_TAG_EXIF_IFD_SAMPLES_PER_PIXEL;
        case TAG_PLANAR_CONFIGURATION: return GUCEF_IMAGE_TAG_EXIF_IFD_PLANAR_CONFIGURATION;
        case TAG_YCBCR_SUBSAMPLING: return GUCEF_IMAGE_TAG_EXIF_IFD_YCBCR_SUBSAMPLING;
        case TAG_YCBCR_POSITIONING: return GUCEF_IMAGE_TAG_EXIF_IFD_YCBCR_POSITIONING;
        case TAG_X_RESOLUTION: return GUCEF_IMAGE_TAG_EXIF_IFD_X_RESOLUTION;
        case TAG_Y_RESOLUTION: return GUCEF_IMAGE_TAG_EXIF_IFD_Y_RESOLUTION;
        case TAG_RESOLUTION_UNIT: return GUCEF_IMAGE_TAG_EXIF_IFD_RESOLUTION_UNIT;

        case TAG_STRIP_OFFSETS: return GUCEF_IMAGE_TAG_EXIF_IFD_STRIP_OFFSETS;
        case TAG_ROWS_PER_STRIP: return GUCEF_IMAGE_TAG_EXIF_IFD_ROWS_PER_STRIP;
        case TAG_STRIP_BYTE_COUNTS: return GUCEF_IMAGE_TAG_EXIF_IFD_STRIP_BYTE_COUNTS;
        case TAG_JPEG_INTERCHANGE_FORMAT: return GUCEF_IMAGE_TAG_EXIF_IFD_JPEG_INTERCHANGE_FORMAT;
        case TAG_JPEG_INTERCHANGE_FORMAT_LENGTH: return GUCEF_IMAGE_TAG_EXIF_IFD_JPEG_INTERCHANGE_FORMAT_LENGTH;

        case TAG_DATETIME: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME;
        case TAG_IMAGE_DESCRIPTION: return GUCEF_IMAGE_TAG_EXIF_IFD_IMAGE_DESCRIPTION;
        case TAG_MAKE: return GUCEF_IMAGE_TAG_EXIF_IFD_MAKE;
        case TAG_MODEL: return GUCEF_IMAGE_TAG_EXIF_IFD_MODEL;
        case TAG_SOFTWARE: return GUCEF_IMAGE_TAG_EXIF_IFD_SOFTWARE;
        case TAG_ARTIST: return GUCEF_IMAGE_TAG_EXIF_IFD_ARTIST;
        case TAG_COPYRIGHT: return GUCEF_IMAGE_TAG_EXIF_IFD_COPYRIGHT;

        case TAG_EXIF_VERSION: return GUCEF_IMAGE_TAG_EXIF_IFD_VERSION;
        case TAG_FLASHPIX_VERSION: return GUCEF_IMAGE_TAG_EXIF_IFD_FLASHPIX_VERSION;
        case TAG_COLOR_SPACE: return GUCEF_IMAGE_TAG_EXIF_IFD_COLOR_SPACE;
        case TAG_COMPONENTS_CONFIGURATION: return GUCEF_IMAGE_TAG_EXIF_IFD_COMPONENTS_CONFIGURATION;
        case TAG_COMPRESSED_BITS_PER_PIXEL: return GUCEF_IMAGE_TAG_EXIF_IFD_COMPRESSED_BITS_PER_PIXEL;
        case TAG_PIXEL_X_DIMENSION: return GUCEF_IMAGE_TAG_EXIF_IFD_PIXEL_X_DIMENSION;
        case TAG_PIXEL_Y_DIMENSION: return GUCEF_IMAGE_TAG_EXIF_IFD_PIXEL_Y_DIMENSION;
        case TAG_MARKER_NOTE: return GUCEF_IMAGE_TAG_EXIF_IFD_MARKER_NOTE;
        case TAG_USER_COMMENT: return GUCEF_IMAGE_TAG_EXIF_IFD_USER_COMMENT;
        case TAG_RELATED_SOUND_FILE: return GUCEF_IMAGE_TAG_EXIF_IFD_RELATED_SOUND_FILE;
        case TAG_DATETIME_ORIGINAL: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_ORIGINAL;
        case TAG_DATETIME_DIGITIZED: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_DIGITIZED;
        case TAG_TIMEZONEOFFSET: return GUCEF_IMAGE_TAG_EXIF_IFD_TIMEZONEOFFSET;
        case TAG_OFFSETTIME: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_OFFSETTIME;
        case TAG_OFFSETTIME_ORIGINAL: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_OFFSETTIME_ORIGINAL;
        case TAG_OFFSETTIME_DIGITIZED: return GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_OFFSETTIME_DIGITIZED;
        case TAG_SUBSECOND_TIME: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBSECOND_TIME;
        case TAG_SUBSECOND_TIME_ORIGINAL: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBSECOND_TIME_ORIGINAL;                         
        case TAG_SUBSECOND_TIME_DIGITIZED: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBSECOND_TIME_DIGITIZED;
        case TAG_EXPOSURE_TIME: return GUCEF_IMAGE_TAG_EXIF_IFD_EXPOSURE_TIME;
        case TAG_FNUMBER: return GUCEF_IMAGE_TAG_EXIF_IFD_FNUMBER;
        case TAG_EXPOSURE_PROGRAM: return GUCEF_IMAGE_TAG_EXIF_IFD_EXPOSURE_PROGRAM;
        case TAG_SPECTRAL_SENSITIVITY: return GUCEF_IMAGE_TAG_EXIF_IFD_SPECTRAL_SENSITIVITY;
        case TAG_ISO_SPEED_RATINGS: return GUCEF_IMAGE_TAG_EXIF_IFD_ISO_SPEED_RATINGS;
        case TAG_OECF: return GUCEF_IMAGE_TAG_EXIF_IFD_OECF;
        case TAG_SHUTTER_SPEED_VALUE: return GUCEF_IMAGE_TAG_EXIF_IFD_SHUTTER_SPEED_VALUE;
        case TAG_APERTURE_VALUE: return GUCEF_IMAGE_TAG_EXIF_IFD_APERTURE_VALUE;
        case TAG_BRIGHTNESS_VALUE: return GUCEF_IMAGE_TAG_EXIF_IFD_BRIGHTNESS_VALUE;
        case TAG_EXPOSURE_BIAS_VALUE: return GUCEF_IMAGE_TAG_EXIF_IFD_EXPOSURE_BIAS_VALUE;
        case TAG_MAX_APERTURE_VALUE: return GUCEF_IMAGE_TAG_EXIF_IFD_MAX_APERTURE_VALUE;
        case TAG_SUBJECT_DISTANCE: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBJECT_DISTANCE;
        case TAG_METERING_MODE: return GUCEF_IMAGE_TAG_EXIF_IFD_METERING_MODE;
        case TAG_LIGHT_SOURCE: return GUCEF_IMAGE_TAG_EXIF_IFD_LIGHT_SOURCE;
        case TAG_FLASH: return GUCEF_IMAGE_TAG_EXIF_IFD_FLASH;
        case TAG_FOCAL_LENGTH: return GUCEF_IMAGE_TAG_EXIF_IFD_FOCAL_LENGTH;
        case TAG_SUBJECT_AREA: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBJECT_AREA;
        case TAG_FLASH_ENERGY: return GUCEF_IMAGE_TAG_EXIF_IFD_FLASH_ENERGY;
        case TAG_SPATIAL_FREQ_RESPONSE: return GUCEF_IMAGE_TAG_EXIF_IFD_SPATIAL_FREQ_RESPONSE;
        case TAG_FOCAL_PLANE_X_RES: return GUCEF_IMAGE_TAG_EXIF_IFD_FOCAL_PLANE_X_RES;
        case TAG_FOCAL_PLANE_Y_RES: return GUCEF_IMAGE_TAG_EXIF_IFD_FOCAL_PLANE_Y_RES;
        case TAG_FOCAL_PLANE_UNIT: return GUCEF_IMAGE_TAG_EXIF_IFD_FOCAL_PLANE_UNIT;
        case TAG_SUBJECT_LOCATION: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBJECT_LOCATION;
        case TAG_EXPOSURE_INDEX: return GUCEF_IMAGE_TAG_EXIF_IFD_EXPOSURE_INDEX;
        case TAG_SENSING_METHOD: return GUCEF_IMAGE_TAG_EXIF_IFD_SENSING_METHOD;
        case TAG_FILE_SOURCE: return GUCEF_IMAGE_TAG_EXIF_IFD_FILE_SOURCE;
        case TAG_SCENE_TYPE: return GUCEF_IMAGE_TAG_EXIF_IFD_SCENE_TYPE;
        case TAG_CFA_PATTERN: return GUCEF_IMAGE_TAG_EXIF_IFD_CFA_PATTERN;
        case TAG_CUSTOM_RENDERED: return GUCEF_IMAGE_TAG_EXIF_IFD_CUSTOM_RENDERED;
        case TAG_EXPOSURE_MODE: return GUCEF_IMAGE_TAG_EXIF_IFD_EXPOSURE_MODE;
        case TAG_WHITE_BALANCE: return GUCEF_IMAGE_TAG_EXIF_IFD_WHITE_BALANCE;
        case TAG_DIGITAL_ZOOM_RATIO: return GUCEF_IMAGE_TAG_EXIF_IFD_DIGITAL_ZOOM_RATIO;
        case TAG_FOCAL_LENGTH_IN_35MM_FILM: return GUCEF_IMAGE_TAG_EXIF_IFD_FOCAL_LENGTH_IN_35MM_FILM;
        case TAG_SCENE_CAPTURE_TYPE: return GUCEF_IMAGE_TAG_EXIF_IFD_SCENE_CAPTURE_TYPE;
        case TAG_GAIN_CONTROL: return GUCEF_IMAGE_TAG_EXIF_IFD_GAIN_CONTROL;
        case TAG_CONTRAST: return GUCEF_IMAGE_TAG_EXIF_IFD_CONTRAST;
        case TAG_SATURATION: return GUCEF_IMAGE_TAG_EXIF_IFD_SATURATION;
        case TAG_SHARPNESS: return GUCEF_IMAGE_TAG_EXIF_IFD_SHARPNESS;
        case TAG_DEVICE_SETTING_DESCRIPTION: return GUCEF_IMAGE_TAG_EXIF_IFD_DEVICE_SETTING_DESCRIPTION;
        case TAG_SUBJECT_DISTANCE_RANGE: return GUCEF_IMAGE_TAG_EXIF_IFD_SUBJECT_DISTANCE_RANGE;
        case TAG_IMAGE_UNIQUE_ID: return GUCEF_IMAGE_TAG_EXIF_IFD_IMAGE_UNIQUE_ID;

        default: 
        {
            return GUCEF_IMAGE_TAG_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

DWORD
MapFreeImageMetaDataTagIdForIPTC( WORD fiTagId )
{
    switch ( fiTagId )
    {
        case TAG_RECORD_VERSION: return GUCEF_IMAGE_TAG_IPTC_RECORD_VERSION;
        case TAG_CAPTION: return GUCEF_IMAGE_TAG_IPTC_CAPTION;
        case TAG_WRITER: return GUCEF_IMAGE_TAG_IPTC_WRITER;
        case TAG_HEADLINE: return GUCEF_IMAGE_TAG_IPTC_HEADLINE;
        case TAG_SPECIAL_INSTRUCTIONS: return GUCEF_IMAGE_TAG_IPTC_SPECIAL_INSTRUCTIONS;
        case TAG_BY_LINE: return GUCEF_IMAGE_TAG_IPTC_BY_LINE;
        case TAG_BY_LINE_TITLE: return GUCEF_IMAGE_TAG_IPTC_BY_LINE_TITLE;
        case TAG_CREDIT: return GUCEF_IMAGE_TAG_IPTC_CREDIT;
        case TAG_SOURCE: return GUCEF_IMAGE_TAG_IPTC_SOURCE;
        case TAG_OBJECT_NAME: return GUCEF_IMAGE_TAG_IPTC_OBJECT_NAME;
        case TAG_DATE_CREATED: return GUCEF_IMAGE_TAG_IPTC_DATE_CREATED;
        case TAG_CITY: return GUCEF_IMAGE_TAG_IPTC_CITY;
        case TAG_PROVINCE_OR_STATE: return GUCEF_IMAGE_TAG_IPTC_PROVINCE_OR_STATE;
        case TAG_COUNTRY_OR_PRIMARY_LOCATION: return GUCEF_IMAGE_TAG_IPTC_COUNTRY_OR_PRIMARY_LOCATION;
        case TAG_ORIGINAL_TRANSMISSION_REFERENCE: return GUCEF_IMAGE_TAG_IPTC_ORIGINAL_TRANSMISSION_REFERENCE;
        case TAG_CATEGORY: return GUCEF_IMAGE_TAG_IPTC_CATEGORY;
        case TAG_SUPPLEMENTAL_CATEGORIES: return GUCEF_IMAGE_TAG_IPTC_SUPPLEMENTAL_CATEGORIES;
        case TAG_URGENCY: return GUCEF_IMAGE_TAG_IPTC_URGENCY;
        case TAG_KEYWORDS: return GUCEF_IMAGE_TAG_IPTC_KEYWORDS;
        case TAG_COPYRIGHT_NOTICE: return GUCEF_IMAGE_TAG_IPTC_COPYRIGHT_NOTICE;
        case TAG_RELEASE_DATE: return GUCEF_IMAGE_TAG_IPTC_RELEASE_DATE;
        case TAG_RELEASE_TIME: return GUCEF_IMAGE_TAG_IPTC_RELEASE_TIME;
        case TAG_TIME_CREATED: return GUCEF_IMAGE_TAG_IPTC_TIME_CREATED;
        case TAG_ORIGINATING_PROGRAM: return GUCEF_IMAGE_TAG_IPTC_ORIGINATING_PROGRAM;

        default: 
        {
            return GUCEF_IMAGE_TAG_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

DWORD
MapFreeImageMetaDataTagId( FREE_IMAGE_MDMODEL fiModel, WORD fiTagId )
{
    switch ( fiModel )
    {
        case FIMD_COMMENTS: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_EXIF_MAIN: return MapFreeImageMetaDataTagIdForExifMain( fiTagId );
        case FIMD_EXIF_EXIF: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_EXIF_GPS: return MapFreeImageMetaDataTagIdForExifGPS( fiTagId );
        case FIMD_EXIF_MAKERNOTE: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_EXIF_INTEROP: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_IPTC: return MapFreeImageMetaDataTagIdForIPTC( fiTagId );
        case FIMD_XMP: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_GEOTIFF: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_ANIMATION: return GUCEF_IMAGE_TAG_UNKNOWN;
        case FIMD_CUSTOM: return GUCEF_IMAGE_TAG_UNKNOWN;
        default: 
        {
            return GUCEF_IMAGE_TAG_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
MapFreeImageMetaDataTag( FREE_IMAGE_MDMODEL model               , 
                         FITAG* tag                             , 
                         TValueMapParserCallbacks* mapCallbacks )
{
    if ( tag != GUCEF_NULL )
    {
        WORD tagId = FreeImage_GetTagID( tag );
        FREE_IMAGE_MDTYPE tagType = FreeImage_GetTagType( tag );
        DWORD tagLength = FreeImage_GetTagLength( tag );
        DWORD tagCount =  FreeImage_GetTagCount( tag );
        const void* tagValue = FreeImage_GetTagValue( tag );
        DWORD guTagId = MapFreeImageMetaDataTagId( model, tagId );

        TVariantData keyVariant;
        TVariantData valueVariant;

        keyVariant.union_data.uint32_data = guTagId;
        keyVariant.containedType = GUCEF_DATATYPE_UINT32;

        switch ( tagType )
        {
            case FIDT_BYTE:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.uint8_data = ( (UInt8*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_UINT8;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_ASCII:
            {
                valueVariant.union_data.heap_data.heap_data_is_linked = 1;
                valueVariant.union_data.heap_data.union_data.char_heap_data = ( (char*) tagValue );
                valueVariant.union_data.heap_data.heap_data_size = (UInt32) tagLength;
                valueVariant.containedType = GUCEF_DATATYPE_ASCII_STRING;
                mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                break;
            }
            case FIDT_SHORT:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {                
                    valueVariant.union_data.uint16_data = ( (UInt16*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_UINT16;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_IFD:
            case FIDT_PALETTE:
            case FIDT_LONG:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.uint32_data = ( (UInt32*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_UINT32;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_SBYTE:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.int8_data = ( (Int8*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_INT8;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_NOTYPE:
            case FIDT_UNDEFINED:
            {
                /* 
                 *  for this type the tagCount and tagLength mean the same thing: The number of bytes in the block
                 *  Like the string type, this doesnt support arrays of the same in the field
                 */
                valueVariant.union_data.heap_data.union_data.void_heap_data = (char*) tagValue;
                valueVariant.union_data.heap_data.heap_data_size = tagLength;
                valueVariant.union_data.heap_data.heap_data_is_linked = 1;
                valueVariant.containedType = GUCEF_DATATYPE_BINARY_BLOB;
                mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                break;
            }
            case FIDT_SSHORT:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.int16_data = ( (Int16*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_INT16;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_SLONG:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.int32_data = ( (Int32*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_INT32;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_FLOAT:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.float32_data = ( (Float32*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_FLOAT32;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_DOUBLE:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.float64_data = ( (Float64*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_FLOAT64;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
	        case FIDT_RATIONAL: 
            {
                /* 
                    Every 'tagCount' instance is 2 32 bit unsigned integers, 
                    the first is the numerator and the second is the denominator
                */
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    UInt32 numerator = ( (UInt32*) tagValue )[ (i*2) + 0 ];
                    UInt32 denominator = ( (UInt32*) tagValue )[ (i*2) + 1 ];                
                    valueVariant.union_data.fraction_data.union_data.uint32t2_data.numerator = numerator;
                    valueVariant.union_data.fraction_data.union_data.uint32t2_data.denominator = denominator;
                    valueVariant.containedType = GUCEF_DATATYPE_UINT32T2_FRACTION;

                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }            
            case FIDT_SRATIONAL: 
            {
                /* 
                    Every 'tagCount' instance is 2 32 bit signed integers, 
                    the first is the numerator and the second is the denominator
                */
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    Int32 numerator = ( (Int32*) tagValue )[ (i*2) + 0 ];
                    Int32 denominator = ( (Int32*) tagValue )[ (i*2) + 1 ];                
                    valueVariant.union_data.fraction_data.union_data.int32t2_data.numerator = numerator;
                    valueVariant.union_data.fraction_data.union_data.int32t2_data.denominator = denominator;
                    valueVariant.containedType = GUCEF_DATATYPE_INT32T2_FRACTION;

                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_IFD8:
            case FIDT_LONG8:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.uint64_data = ( (UInt64*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_UINT64;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            case FIDT_SLONG8:
            {
                for ( DWORD i=0; i<tagCount; ++i )
                {
                    valueVariant.union_data.uint64_data = ( (Int64*) tagValue )[ i ];
                    valueVariant.containedType = GUCEF_DATATYPE_INT64;
                    mapCallbacks->OnKeyValuePair( mapCallbacks->privateData, &keyVariant, &valueVariant );
                }
                break;
            }
            default:
            {
                return;    
            }
        }        
    }
}

/*---------------------------------------------------------------------------*/

void
MapFreeImageMetaData( FREE_IMAGE_MDMODEL model               , 
                      FIBITMAP* dib                          , 
                      TValueMapParserCallbacks* mapCallbacks )
{
    BOOL result = 0;
    FITAG* tag = GUCEF_NULL;
    FIMETADATA* metaData = FreeImage_FindFirstMetadata( model, dib, &tag );
    while ( metaData != GUCEF_NULL )
    {
        MapFreeImageMetaDataTag( model, tag, mapCallbacks );
        result = FreeImage_FindNextMetadata( metaData, &tag );
        if ( 0 == result )
            break;
    }
    FreeImage_FindCloseMetadata( metaData );
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
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        FIBITMAP* dib = GUCEF_NULL;
        
        mapCallbacks->OnValueMapBegin( mapCallbacks->privateData, 1, 1 );

        fif = FreeImage_GetFileTypeFromHandle( &io, (fi_handle) input, 0 );

        if ( fif == FIF_UNKNOWN )
        {
            fif = GetFIFOfType( codecType );
        }

	    /* Load the bitmap: the header and the meta-data, the formats that can leave the pixels out do */
        if( fif != FIF_UNKNOWN )
        {
            dib = FreeImage_LoadFromHandle( fif, &io, (fi_handle) input, FIF_LOAD_NOPIXELS );
            if ( dib != NULL )
            {
                /* now we map the metadata */
                //MapFreeImageMetaData( FIMD_COMMENTS, dib, mapCallbacks );
                MapFreeImageMetaData( FIMD_EXIF_MAIN, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_EXIF_EXIF, dib, mapCallbacks );
                MapFreeImageMetaData( FIMD_EXIF_GPS, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_EXIF_MAKERNOTE, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_EXIF_INTEROP, dib, mapCallbacks );       
                MapFreeImageMetaData( FIMD_IPTC, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_XMP, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_GEOTIFF, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_ANIMATION, dib, mapCallbacks );
               // MapFreeImageMetaData( FIMD_CUSTOM, dib, mapCallbacks );

                FreeImage_Unload( dib );
                dib = GUCEF_NULL;

                mapCallbacks->OnValueMapEnd( mapCallbacks->privateData );
                return 1;
            }
	    }

        mapCallbacks->OnValueMapEnd( mapCallbacks->privateData );
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

/** Tells the host that the picture was changed to fit the format it was written in */
static void
LogWriteChange( const char* codecType, const char* what )
{
    char message[ 256 ];
    if ( NULL != g_coreApi && NULL != g_coreApi->Log )
    {
        sprintf( message, "imgpluginFreeImage: the %.32s format holds %.100s: the picture was changed to fit", codecType, what );
        g_coreApi->Log( GUCEF_LOGMSGTYPE_WARNING, GUCEF_LOGLEVEL_IMPORTANT, message );
    }
}

/*---------------------------------------------------------------------------*/

/** The FreeImage type of the pixels of a level, FIT_UNKNOWN when FreeImage has no picture of them */
static FREE_IMAGE_TYPE
GetFreeImageType( const TImageMipMapLevel* level )
{
    UInt32 channels = GetChannelCountForFormat( level->mipLevelInfo.pixelStorageFormat );
    int format = level->mipLevelInfo.pixelStorageFormat;
    int type = level->mipLevelInfo.pixelComponentDataType;
    BOOL colour = ( PSF_RGB == format || PSF_BGR == format || PSF_RGBA == format || PSF_BGRA == format );

    switch ( type )
    {
        case GUCEF_DATATYPE_UINT8 :
        {
            return ( 1 == channels || 3 == channels || 4 == channels ) ? FIT_BITMAP : FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_UINT16 :
        {
            if ( 1 == channels ) return FIT_UINT16;
            if ( colour ) return 3 == channels ? FIT_RGB16 : FIT_RGBA16;
            return FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_INT16 :
        {
            return 1 == channels ? FIT_INT16 : FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_UINT32 :
        {
            return 1 == channels ? FIT_UINT32 : FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_INT32 :
        {
            return 1 == channels ? FIT_INT32 : FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_FLOAT32 :
        {
            if ( 1 == channels ) return FIT_FLOAT;
            if ( colour ) return 3 == channels ? FIT_RGBF : FIT_RGBAF;
            return FIT_UNKNOWN;
        }
        case GUCEF_DATATYPE_FLOAT64 :
        {
            return 1 == channels ? FIT_DOUBLE : FIT_UNKNOWN;
        }
        default :
        {
            return FIT_UNKNOWN;
        }
    }
}

/*---------------------------------------------------------------------------*/

/**
 *  A FreeImage bitmap of the pixels of a level, a copy: the pixels of the caller stay as they are. FreeImage keeps the bottom row first.
 *  Bitmaps of 8 bits per channel have the channel order of the platform, the other types are always RGB.
 */
static FIBITMAP*
BuildBitmap( TImageMipMapLevel* level, FREE_IMAGE_TYPE fit )
{
    UInt32 width = level->mipLevelInfo.frameWidth;
    UInt32 height = level->mipLevelInfo.frameHeight;
    int pixelSizeInBytes = GetPixelSize( level->mipLevelInfo.pixelStorageFormat, level->mipLevelInfo.pixelComponentDataType );
    UInt32 rowBytes = pixelSizeInBytes * width;
    FIBITMAP* bitmap = NULL;

    if ( 0 == width || 0 == height || 0 == pixelSizeInBytes || level->pixelDataSizeInBytes < rowBytes * height )
        return NULL;

    if ( FIT_BITMAP == fit )
    {
        UInt8* pixelData = (UInt8*) level->pixelData;
        BOOL pixelsConverted = ConvertToFreeImageNative( level, &pixelData );
        bitmap = FreeImage_ConvertFromRawBitsEx( 1, (BYTE*) pixelData, FIT_BITMAP, width, height, rowBytes, pixelSizeInBytes * 8, 0, 0, 0, 1 );
        if ( TRUE == pixelsConverted )
            free( pixelData );
        return bitmap;
    }
    else
    {
        UInt32 y = 0;
        UInt32 channelSize = GetPixelChannelSize( level->mipLevelInfo.pixelComponentDataType );
        BOOL swapRedAndBlue = ( PSF_BGR == level->mipLevelInfo.pixelStorageFormat || PSF_BGRA == level->mipLevelInfo.pixelStorageFormat );
        UInt8* source = (UInt8*) level->pixelData;

        bitmap = FreeImage_AllocateT( fit, width, height, 0, 0, 0, 0 );
        if ( NULL == bitmap || FreeImage_GetLine( bitmap ) != rowBytes )
        {
            if ( NULL != bitmap )
                FreeImage_Unload( bitmap );
            return NULL;
        }

        for ( y=0; y<height; ++y )
        {
            UInt8* row = (UInt8*) FreeImage_GetScanLine( bitmap, (int) ( height - 1 - y ) );
            memcpy( row, source + ( (size_t) y * rowBytes ), rowBytes );
            if ( swapRedAndBlue )
            {
                UInt32 x = 0, i = 0;
                UInt8 swap[ 8 ];
                for ( x=0; x<width; ++x )
                {
                    UInt8* red = row + ( (size_t) x * pixelSizeInBytes );
                    UInt8* blue = red + ( 2 * channelSize );
                    for ( i=0; i<channelSize; ++i ) swap[ i ] = red[ i ];
                    for ( i=0; i<channelSize; ++i ) red[ i ] = blue[ i ];
                    for ( i=0; i<channelSize; ++i ) blue[ i ] = swap[ i ];
                }
            }
        }
        return bitmap;
    }
}

/*---------------------------------------------------------------------------*/

/**
 *  Brings a bitmap to what the format can write, taking it over: the bitmap that is to be written, NULL when the format cannot hold it.
 *  Formats that hold only palette pictures (GIF) or only black and white (WBMP) get the picture reduced to them: that is what the
 *  format is, and no picture of ours can be written in it otherwise. When a format can hold 24 or 32 bits but not the ones of the
 *  picture NULL is given: the caller offers another pixel format.
 */
static FIBITMAP*
FitBitmapToFormat( FREE_IMAGE_FORMAT fif, FIBITMAP* bitmap, const char* codecType )
{
    FREE_IMAGE_TYPE fit = FreeImage_GetImageType( bitmap );
    int bpp = (int) FreeImage_GetBPP( bitmap );
    FIBITMAP* fitted = NULL;

    if ( FIT_BITMAP != fit )
    {
        if ( FreeImage_FIFSupportsExportType( fif, fit ) )
            return bitmap;
        FreeImage_Unload( bitmap );
        return NULL;
    }

    if ( FreeImage_FIFSupportsExportBPP( fif, bpp ) )
        return bitmap;

    if ( !FreeImage_FIFSupportsExportBPP( fif, 24 ) && !FreeImage_FIFSupportsExportBPP( fif, 32 ) )
    {
        if ( FreeImage_FIFSupportsExportBPP( fif, 8 ) )
        {
            /* a palette format: the colours are reduced to 256, the alpha channel is left out */
            FIBITMAP* colour = 24 == bpp ? bitmap : FreeImage_ConvertTo24Bits( bitmap );
            if ( NULL != colour )
            {
                fitted = FreeImage_ColorQuantize( colour, FIQ_WUQUANT );
                if ( colour != bitmap )
                    FreeImage_Unload( colour );
            }
            if ( NULL != fitted )
            {
                LogWriteChange( codecType, "at most 256 colours and no alpha channel" );
                FreeImage_Unload( bitmap );
                return fitted;
            }
        }
        else
        if ( FreeImage_FIFSupportsExportBPP( fif, 1 ) )
        {
            /* a black and white format */
            FIBITMAP* grey = FreeImage_ConvertToGreyscale( bitmap );
            if ( NULL != grey )
            {
                fitted = FreeImage_Threshold( grey, 128 );
                FreeImage_Unload( grey );
            }
            if ( NULL != fitted )
            {
                LogWriteChange( codecType, "black and white pixels only" );
                FreeImage_Unload( bitmap );
                return fitted;
            }
        }
    }

    FreeImage_Unload( bitmap );
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void
SetAnimationTag( FIBITMAP* bitmap, const char* key, FREE_IMAGE_MDTYPE type, const void* value, DWORD length )
{
    FITAG* tag = FreeImage_CreateTag();
    if ( NULL != tag )
    {
        FreeImage_SetTagKey( tag, key );
        FreeImage_SetTagType( tag, type );
        FreeImage_SetTagCount( tag, 1 );
        FreeImage_SetTagLength( tag, length );
        FreeImage_SetTagValue( tag, value );
        FreeImage_SetMetadata( FIMD_ANIMATION, bitmap, key, tag );
        FreeImage_DeleteTag( tag );
    }
}

/*---------------------------------------------------------------------------*/

/** Writes the frames as an animated GIF: the first frame makes the file, the others are added to it */
static BOOL
WriteGifFrames( FIBITMAP** bitmaps, UInt32 frameCount, TIOAccess* output )
{
    FIMEMORY* seed = FreeImage_OpenMemory( NULL, 0 );
    FIMEMORY* result = NULL;
    FIMULTIBITMAP* animation = NULL;
    BYTE* data = NULL;
    DWORD size = 0;
    UInt32 i = 0;
    BOOL success = FALSE;

    if ( NULL == seed )
        return FALSE;

    if ( FreeImage_SaveToMemory( FIF_GIF, bitmaps[ 0 ], seed, 0 ) )
    {
        FreeImage_SeekMemory( seed, 0, SEEK_SET );
        animation = FreeImage_LoadMultiBitmapFromMemory( FIF_GIF, seed, 0 );
        if ( NULL != animation )
        {
            for ( i=1; i<frameCount; ++i )
                FreeImage_AppendPage( animation, bitmaps[ i ] );

            result = FreeImage_OpenMemory( NULL, 0 );
            if ( NULL != result && FreeImage_SaveMultiBitmapToMemory( FIF_GIF, animation, result, 0 ) && FreeImage_AcquireMemory( result, &data, &size ) )
            {
                success = ( 1 == output->write( output, data, size, 1 ) );
            }
        }
    }

    if ( NULL != result )
        FreeImage_CloseMemory( result );
    if ( NULL != animation )
        FreeImage_CloseMultiBitmap( animation, 0 );
    FreeImage_CloseMemory( seed );
    return success;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_EncodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TImage* inputImage    ,
                            TIOAccess* output     )
{
    UInt32 i = 0, frameCount = 0, frameCountToWrite = 0;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP** bitmaps = NULL;
    BOOL success = FALSE;

    /* input sanity check */
    if ( ( NULL == inputImage ) || ( NULL == codecType ) || ( NULL == output ) ) return 0;

    fif = GetFIFOfType( codecType );
    frameCount = inputImage->imageInfo.nrOfFramesInImage;
    if ( fif == FIF_UNKNOWN || !FreeImage_FIFSupportsWriting( fif ) || 0 == frameCount ) return 0;

    /* only an animation format takes more than the first frame */
    frameCountToWrite = ( FIF_GIF == fif ) ? frameCount : 1;
    if ( frameCountToWrite < frameCount && NULL != g_coreApi && NULL != g_coreApi->Log )
    {
        char message[ 160 ];
        sprintf( message, "imgpluginFreeImage: the %.32s format holds one picture: only the first of the %u frames was written", codecType, (unsigned) frameCount );
        g_coreApi->Log( GUCEF_LOGMSGTYPE_WARNING, GUCEF_LOGLEVEL_IMPORTANT, message );
    }

    bitmaps = (FIBITMAP**) calloc( frameCountToWrite, sizeof( FIBITMAP* ) );
    if ( NULL == bitmaps ) return 0;

    for ( i=0; i<frameCountToWrite; ++i )
    {
        TImageFrame* frame = &inputImage->frames[ i ];
        FIBITMAP* bitmap = NULL;
        FREE_IMAGE_TYPE fit = FIT_UNKNOWN;

        if ( frame->frameInfo.nrOfMipmapLevels < 1 )
            break;
        fit = GetFreeImageType( &frame->mipmapLevel[ 0 ] );
        if ( FIT_UNKNOWN == fit )
            break;
        bitmap = BuildBitmap( &frame->mipmapLevel[ 0 ], fit );
        if ( NULL == bitmap )
            break;
        bitmap = FitBitmapToFormat( fif, bitmap, codecType );
        if ( NULL == bitmap )
            break;

        if ( frameCountToWrite > 1 )
        {
            LONG durationInMs = (LONG) frame->frameInfo.frameDurationInMs;
            BYTE disposal = 1;      /* every frame is complete: leave it in place */
            if ( durationInMs > 0 )
                SetAnimationTag( bitmap, "FrameTime", FIDT_LONG, &durationInMs, 4 );
            SetAnimationTag( bitmap, "DisposalMethod", FIDT_BYTE, &disposal, 1 );
            if ( 0 == i )
            {
                LONG loopCount = (LONG) inputImage->imageInfo.loopCount;
                SetAnimationTag( bitmap, "Loop", FIDT_LONG, &loopCount, 4 );
            }
        }
        bitmaps[ i ] = bitmap;
    }

    if ( i == frameCountToWrite )
    {
        if ( frameCountToWrite > 1 )
            success = WriteGifFrames( bitmaps, frameCountToWrite, output );
        else
            success = ( 0 != FreeImage_SaveToHandle( fif, bitmaps[ 0 ], &io, (fi_handle) output, 0 ) );
    }

    for ( i=0; i<frameCountToWrite; ++i )
        if ( NULL != bitmaps[ i ] )
            FreeImage_Unload( bitmaps[ i ] );
    free( bitmaps );
    return success ? 1 : 0;
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
