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
                    return PSF_RGB;
                }
                case FIC_RGBALPHA :
                {
                    return PSF_RGBA;
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

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata   ,
                  const int argc    ,
                  const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
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
         fiReleaseVersion != FREEIMAGE_RELEASE_SERIAL ) return 0;

    io.read_proc  = myReadProc;
    io.write_proc = myWriteProc;
    io.seek_proc  = mySeekProc;
    io.tell_proc  = myTellProc;

    *plugdata = NULL;
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    memset( &io, 0, sizeof( io ) );

    /* call this ONLY when linking with FreeImage as a static library */
    #ifdef FREEIMAGE_LIB
    FreeImage_DeInitialise();
    #endif /* FREEIMAGE_LIB ? */

    return 1;
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

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Encode( void* plugdata         ,
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

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Decode( void* pluginData       ,
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
CODECPLUGIN_GetCodecSetBegin( void* plugdata  ,
                              void** iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *iterator = malloc( sizeof( UInt32 ) );
    *( (UInt32*) *iterator ) = 0;
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata               ,
                          void* iterator               ,
                          TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt32 codecIndex = *( (UInt32*) iterator );

    if ( codecIndex < codecCount )
    {
        *codecLink = (TCodecPluginLink*) malloc( sizeof( TCodecPluginLink ) );
        (*codecLink)->codecData = NULL;
        (*codecLink)->codecFamily = "ImageCodec";
        (*codecLink)->codecType = supportedTypes[ codecIndex ];
        (*codecLink)->encode = CODECPLUGIN_Encode;
        (*codecLink)->decode = CODECPLUGIN_Decode;
        return 1;
    }

    /* There is no codec with the given index */
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_FreeCodecLink( void* plugdata              ,
                           TCodecPluginLink* codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( codecLink );
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_FreeCodecIterator( void* plugdata ,
                               void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt32* codecIndex = (UInt32*) iterator;
    free( codecIndex );
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecSetNextItem( void* plugdata ,
                                 void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt32* codecIndex = ( (UInt32*) iterator );
    ++(*codecIndex);

    if ( (*codecIndex) < codecCount )
    {
        return 1;
    }

    /* There are no more codecs */
    return 0;
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
        for ( i=0; i<frameCount; ++i )
        {
            TFreeImageDataLink* imageDataLink = &allImageDataLinks->links[ i ];
            FreeImage_Unload( imageDataLink->fiData );
        }
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

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TIOAccess* input      ,
                            TImage** imageOutput  ,
                            void** imageData      )
{
    UInt32 i=0, n=0, mipmapCount=0, frameCount=0;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FREE_IMAGE_TYPE fit = FIT_UNKNOWN;
    FIBITMAP* dib = NULL;
    TFreeImageDataLinks* imageDataLinks = NULL;

    /* input sanity check */
    if ( ( NULL == imageOutput ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == imageOutput ) ) return 0;

    fif = FreeImage_GetFileTypeFromHandle( &io, (fi_handle) input, 0 );

    if ( fif == FIF_UNKNOWN )
    {
        fif = FreeImage_GetFIFFromFormat( codecType );
        if ( fif == FIF_UNKNOWN )
        {
            fif = GetFileTypeFromExt( codecType );
        }
    }

    if( fif != FIF_UNKNOWN )
    {
        dib = FreeImage_LoadFromHandle( fif, &io, (fi_handle) input, 0 );
        if ( dib != NULL )
        {
            /* First we get convert image types we cannot handle */
            TImage* image = NULL;
            UInt32 linkIndex = 0;
            UInt32 bpp = FreeImage_GetBPP( dib );
            FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType( dib );
            fit = FreeImage_GetImageType( dib );

            if ( FIT_BITMAP == fit )
            {
                if ( bpp < 8 )
                {
				    /* we do not support a format that has less that 8 bits per pixel */
				    FIBITMAP* newDIB = FreeImage_ConvertTo8Bits( dib );
				    FreeImage_Unload( dib );
				    dib = newDIB;
				    colourType = FreeImage_GetColorType( dib );
				    bpp = FreeImage_GetBPP( dib );
                }

			    /* Now that we know the BPP is always >= 8 we can check the color format */
			    if ( ( colourType == FIC_MINISWHITE ) ||
			         ( colourType == FIC_MINISBLACK )  )
			    {
				    /* custom color ranges are not supported */
				    FIBITMAP* newDIB = FreeImage_ConvertToGreyscale( dib );
				    FreeImage_Unload( dib );
				    dib = newDIB;
				    colourType = FreeImage_GetColorType( dib );
				    bpp = FreeImage_GetBPP( dib );
                }
                else
			    if ( ( colourType == FIC_PALETTE ) ||
			         ( colourType == FIC_CMYK )     )
			    {
				    /* palettes are not supported */
				    FIBITMAP* newDIB = FreeImage_ConvertTo24Bits( dib );
				    FreeImage_Unload( dib );
				    dib = newDIB;
				    colourType = FreeImage_GetColorType( dib );
				    bpp = FreeImage_GetBPP( dib );
			    }

			    /* At this point images are always greyscale or RGB or RGBA and BBP is always >= 8 */
            }

            if ( bpp == 32 && colourType == FIC_RGB )
            {
                /* This is 32bpp thus it is actually RGBA but the Alpha layer has no values thus FreeImage says its RGB 
                   This screws up usage, correct here */
                colourType = FIC_RGBALPHA;
            }

            image = (TImage*) malloc( sizeof( TImage ) );

            /* build the TImageInfo section */
            image->version = GUCEF_IMAGE_TIMAGE_VERSION;
            image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
            image->imageInfo.nrOfFramesInImage = frameCount = (UInt32) 1; /* @TODO: multiple frames are not supported atm */

            /* Build the TImageFrameInfo sections */
            image->frames = (TImageFrame*) malloc( frameCount * sizeof(TImageFrame) );
            for ( i=0; i<frameCount; ++i )
            {
                TImageFrame* frame = &image->frames[ i ];
                frame->version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;
                frame->frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
                frame->frameInfo.nrOfMipmapLevels = mipmapCount = (UInt32) 1; /* @TODO: mip-mapping is not supported atm */

                /* Build the TImageMipMapLevelInfo sections */
                frame->mipmapLevel = (TImageMipMapLevel*) malloc( mipmapCount * sizeof(TImageMipMapLevel) );
                for ( n=0; n<mipmapCount; ++n )
                {
                    TImageMipMapLevel* mipmapLevel = &frame->mipmapLevel[ n ];
                    mipmapLevel->version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
                    mipmapLevel->mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
                    mipmapLevel->mipLevelInfo.frameHeight = FreeImage_GetHeight( dib );
                    mipmapLevel->mipLevelInfo.frameWidth = FreeImage_GetWidth( dib );
                    mipmapLevel->mipLevelInfo.pixelComponentDataType = ConvertToPCDT( fit );
                    mipmapLevel->mipLevelInfo.pixelStorageFormat = ConvertToPSF( fit, colourType );
                    mipmapLevel->pixelData = NULL;
                    mipmapLevel->pixelDataSizeInBytes = 0;
                }
            }

            /* now that we know the total size/layout we can make our link array which will allow us to link pixel data to our FreeImage structs */
            imageDataLinks = (TFreeImageDataLinks*) malloc( sizeof( TFreeImageDataLinks ) );
            imageDataLinks->linkCount = GetNrOfPixelBuffers( image );
            imageDataLinks->links = (TFreeImageDataLink*) malloc( imageDataLinks->linkCount * sizeof( TFreeImageDataLink ) );
            memset( imageDataLinks->links, 0, imageDataLinks->linkCount * sizeof( TFreeImageDataLink ) );

            /* now we assign the pixel data */
            linkIndex = 0;
            for ( i=0; i<frameCount; ++i )
            {
                TImageFrame* frame = &image->frames[ i ];
                for ( n=0; n<mipmapCount; ++n )
                {
                    /* assign the pixel data. */
                    TImageMipMapLevel* mipmapLevel = &frame->mipmapLevel[ n ];
                    TFreeImageDataLink* dataLink = &imageDataLinks->links[ linkIndex ];

                    /* @TODO: note that GetBits/dib needs to be something else once we support frames/mipmapping */
                    mipmapLevel->pixelData = FreeImage_GetBits( dib );
                    mipmapLevel->pixelDataSizeInBytes = ( FreeImage_GetBPP( dib ) / 8 ) * mipmapLevel->mipLevelInfo.frameWidth * mipmapLevel->mipLevelInfo.frameHeight; 
                    dataLink->pixelData = mipmapLevel->pixelData;
                    dataLink->fiData = dib;

                    ++linkIndex;
                }
            }

            /* we successfully loaded and processed the image */
            *imageOutput = image;
            *imageData = imageDataLinks;
            return 1;
        }
    }

    /*
     *  the image decode failed
     */
    return 0;
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
            fif = FreeImage_GetFIFFromFormat( codecType );
            if ( fif == FIF_UNKNOWN )
            {
                fif = GetFileTypeFromExt( codecType );
            }
        }

	    /* Load the bitmap */
        if( fif != FIF_UNKNOWN )
        {
            dib = FreeImage_LoadFromHandle( fif, &io, (fi_handle) input, 0 );
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

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_EncodeImage( void* pluginData      ,
                            void* codecData       ,
                            const char* codecType ,
                            TImage* inputImage    ,
                            TIOAccess* output     )
{
    UInt32 i=0, n=0, mipmapCount=0, frameCount=0;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FREE_IMAGE_TYPE fit = FIT_UNKNOWN;
    FIBITMAP* dib = NULL;
    TFreeImageDataLinks* imageDataLinks = NULL;

    /* input sanity check */
    if ( ( NULL == inputImage ) || ( NULL == codecType ) || ( NULL == output ) ) return 0;

    if ( inputImage->imageInfo.nrOfFramesInImage > 0 )
    {
        TImageFrame* frame = &inputImage->frames[ 0 ];
        if ( frame->frameInfo.nrOfMipmapLevels > 0 )
        {
            TImageMipMapLevel* level = &frame->mipmapLevel[ 0 ];

            UInt8* pixelData = (UInt8*) level->pixelData;
            BOOL pixelsConverted = ConvertToFreeImageNative( level, &pixelData );

            int pixelSizeInBytes = GetPixelSize( level->mipLevelInfo.pixelStorageFormat     ,
                                                 level->mipLevelInfo.pixelComponentDataType );
            
            int bpp = pixelSizeInBytes * 8;
            int pitch = pixelSizeInBytes * level->mipLevelInfo.frameWidth; 
            FIBITMAP* bitmap = FreeImage_ConvertFromRawBitsEx( 0                               ,
                                                               (BYTE*) pixelData               ,
                                                               FIT_BITMAP                      ,
                                                               level->mipLevelInfo.frameWidth  , 
                                                               level->mipLevelInfo.frameHeight ,
                                                               pitch                           ,
                                                               bpp                             ,
                                                               0, 0, 0                         ,
                                                               1                               );
            if ( NULL == bitmap )
            {
                if ( TRUE == pixelsConverted )
                    free( pixelData );
                return 0;
            }

            fif = FreeImage_GetFIFFromFormat( codecType );            
            if ( 0 != FreeImage_SaveToHandle( fif, bitmap, &io, (fi_handle) output, 0 ) )
            {
                if ( TRUE == pixelsConverted )
                    free( pixelData );
                return 1;
            }
            if ( TRUE == pixelsConverted )
                free( pixelData );
        }
    }
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
