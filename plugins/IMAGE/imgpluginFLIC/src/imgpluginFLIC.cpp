/*
 *  imgpluginITV: GUCEF image codec plugin providing support for ITV images
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
#include <string.h>
#include "flic.h"
#include "imgpluginFLIC.h" /* function prototypes for this plugin */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

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

#define IMGPLUGIN_FLIC_MAYOR_VERSION       0
#define IMGPLUGIN_FLIC_MINOR_VERSION       1
#define IMGPLUGIN_FLIC_PATCH_VERSION       0
#define IMGPLUGIN_FLIC_RELEASE_VERSION     0

/*---------------------------------------------------------------------------*/

static char* supportedTypes[] = {
    "flic" ,
    "itv19"     // A FLIC animation stored as an Into the Void resource, the FLIC data starts after the resource header
};

// Size of the header Into the Void places in front of the payload of every resource
static const UInt32 itvResourceHeaderSize = 55;


static const UInt32 codecCount = sizeof( supportedTypes ) / sizeof(char*); 

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class IOAccessAdapter : public ::flic::FileInterface 
{
    private:
    CORE::TIOAccess* m_ioAccess;
    UInt32 m_baseOffset;        // position in the input where the FLIC data starts
    
    public:
    IOAccessAdapter( CORE::TIOAccess* ioAccess, UInt32 baseOffset )
        : m_ioAccess( ioAccess )
        , m_baseOffset( baseOffset )
    {
        // The FLIC decoder starts reading right away, so make sure we start at the beginning of the FLIC data
        if ( NULL != m_ioAccess )
            m_ioAccess->setpos( m_ioAccess, m_baseOffset );
    }

    virtual ~IOAccessAdapter() {}

    bool ok() const override
    {
        return NULL != m_ioAccess && 0 == m_ioAccess->eof( m_ioAccess );
    }
    
    size_t tell() override
    {
        return (size_t) m_ioAccess->tell( m_ioAccess ) - m_baseOffset;
    }
    
    void seek( size_t absPos ) override
    {
        m_ioAccess->setpos( m_ioAccess, (UInt32) absPos + m_baseOffset );
    }
    
    uint8_t read8() override
    {
        if ( 0 == m_ioAccess->eof( m_ioAccess ) )
            return (uint8_t) m_ioAccess->getc( m_ioAccess );
        else
            return 0;
    }
    
    void write8( uint8_t value ) override
    {
        m_ioAccess->write( m_ioAccess, &value, 1, 1 );
    }
};

/*---------------------------------------------------------------------------*/

static UInt32
GetBaseOffset( const char* codecType )
{
    return ( 0 == strcmp( codecType, "itv19" ) ) ? itvResourceHeaderSize : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX ImageStreamEncode( void* plugdata, void* codecData, const char* familyType, const char* codecType, CORE::TIOAccess* input, CORE::TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX ImageStreamDecode( void* pluginData, void* codecData, const char* familyType, const char* codecType, CORE::TIOAccess* input, CORE::TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

/* The codec link API: codecData is the type name, the image codec API below is called with it */
static CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageCodecLinkEncode( void* pluginData                   ,
                      void* codecData                    ,
                      const CORE::TVariantMapApi* params    ,
                      CORE::TIOAccess* input                ,
                      CORE::TIOAccess* output               ,
                      CORE::TCodecErrorInfo* error          ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != ImageStreamEncode( pluginData, NULL, "ImageCodec", (const char*) codecData, input, output ) )
        return CORE::GUCEF_CODEC_STATUS_OK;
    if ( NULL != error )
    {
        strncpy( error->message, "Unable to encode the image", GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return CORE::GUCEF_CODEC_STATUS_FAILED;
}

/*---------------------------------------------------------------------------*/

static CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageCodecLinkDecode( void* pluginData                   ,
                      void* codecData                    ,
                      const CORE::TVariantMapApi* params    ,
                      CORE::TIOAccess* input                ,
                      CORE::TIOAccess* output               ,
                      CORE::TCodecErrorInfo* error          ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( 0 != ImageStreamDecode( pluginData, NULL, "ImageCodec", (const char*) codecData, input, output ) )
        return CORE::GUCEF_CODEC_STATUS_OK;
    if ( NULL != error )
    {
        strncpy( error->message, "Unable to decode the image", GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return CORE::GUCEF_CODEC_STATUS_FAILED;
}

/*---------------------------------------------------------------------------*/

/* One link per supported type, owned by the plugin from Init until Shutdown */
static CORE::TCodecPluginLink* codecLinks = NULL;

/*---------------------------------------------------------------------------*/

static CORE::TCodecStatus
BuildCodecLinks( void )
{
    UInt32 i = 0;

    free( codecLinks );
    codecLinks = ( CORE::TCodecPluginLink* ) calloc( codecCount > 0 ? codecCount : 1, sizeof( CORE::TCodecPluginLink ) );
    if ( NULL == codecLinks )
        return CORE::GUCEF_CODEC_STATUS_FAILED;

    for ( i=0; i<codecCount; ++i )
    {
        codecLinks[ i ].structSize = sizeof( CORE::TCodecPluginLink );
        codecLinks[ i ].codecFamily = "ImageCodec";
        codecLinks[ i ].codecType = supportedTypes[ i ];
        codecLinks[ i ].codecData = (void*) supportedTypes[ i ];
        codecLinks[ i ].capabilities = GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE;
        codecLinks[ i ].encode = ImageCodecLinkEncode;
        codecLinks[ i ].decode = ImageCodecLinkDecode;
    }
    return CORE::GUCEF_CODEC_STATUS_OK;
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

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata                    ,
                  TGucefCoreCApi* coreApi            ,
                  const CORE::TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

    *plugdata = NULL;
    return BuildCodecLinks();
}

/*---------------------------------------------------------------------------*/

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

    FreeCodecLinks();
    return CORE::GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "imgpluginFLIC: image codec allowing use of FLIC files";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. 2002 - 2017.  LGPLv3 license\0";
}

/*---------------------------------------------------------------------------*/

CORE::TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CORE::TVersion version;

    version.major = IMGPLUGIN_FLIC_MAYOR_VERSION;
    version.minor = IMGPLUGIN_FLIC_MINOR_VERSION;
    version.patch = IMGPLUGIN_FLIC_PATCH_VERSION;
    version.release = IMGPLUGIN_FLIC_RELEASE_VERSION;

    return version;
}

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageStreamEncode( void* plugdata          ,
                    void* codecData         ,
                    const char* familyType  ,
                    const char* codecType   ,
                    CORE::TIOAccess* input  ,
                    CORE::TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* Saving images is not supported atm */
    return 0;
}

/*---------------------------------------------------------------------------*/

static UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
ImageStreamDecode( void* pluginData        ,
                    void* codecData         ,
                    const char* familyType  ,
                    const char* codecType   ,
                    CORE::TIOAccess* input  ,
                    CORE::TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
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

CORE::TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata                          ,
                          UInt32 index                            ,
                          const CORE::TCodecPluginLink** codecLink   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( NULL == codecLink )
        return CORE::GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    if ( NULL == codecLinks || index >= codecCount )
    {
        *codecLink = NULL;
        return CORE::GUCEF_CODEC_STATUS_UNSUPPORTED;
    }
    *codecLink = &codecLinks[ index ];
    return CORE::GUCEF_CODEC_STATUS_OK;
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
            if ( NULL != frame->mipmapLevel )
            {
                UInt32 n;
                for ( n=0; n<frame->frameInfo.nrOfMipmapLevels; ++n )
                    free( frame->mipmapLevel[ n ].pixelData );
            }
            free( frame->mipmapLevel );
        }
        free( image->frames );
        free( image );

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImage( void* pluginData       ,
                            void* codecData        ,
                            const char* codecType  ,
                            CORE::TIOAccess* input ,
                            TImage** imageOutput   ,
                            void** imageData       )
{
    /* input sanity check */
    if ( ( NULL == imageOutput ) || ( NULL == codecType ) || ( NULL == input ) || ( NULL == imageData ) ) 
        return 0;

    IOAccessAdapter ioAccessAdapter( input, GetBaseOffset( codecType ) );
    flic::Decoder flicDecoder( &ioAccessAdapter );

    flic::Header flicHeader;
    if ( !flicDecoder.readHeader( flicHeader ) )
        return 0;
    if ( flicHeader.frames <= 0 || flicHeader.width <= 0 || flicHeader.height <= 0 )
        return 0;

    // The decoder patches the previous frame, so the index buffer has to survive across frames
    const UInt32 pixelCount = (UInt32) flicHeader.width * (UInt32) flicHeader.height;
    GUCEF::vector< UInt8 > indices( pixelCount, 0 );
    flic::Frame flicFrame;
    flicFrame.pixels = &indices[ 0 ];
    flicFrame.rowstride = (uint32_t) flicHeader.width;

    TImage* image = (TImage*) malloc( sizeof(TImage) );
    if ( NULL == image )
        return 0;
    image->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
    image->imageInfo.nrOfFramesInImage = (UInt32) flicHeader.frames;
    image->imageInfo.loopCount = 0; // FLIC animations loop forever
    image->frames = (TImageFrame*) malloc( sizeof(TImageFrame) * flicHeader.frames );
    if ( NULL == image->frames )
    {
        free( image );
        return 0;
    }
    memset( image->frames, 0, sizeof(TImageFrame) * flicHeader.frames );

    for ( int i=0; i<flicHeader.frames; ++i )
    {
        // Read the actual frame from the resource
        if ( !flicDecoder.readFrame( flicFrame ) )
        {
            image->imageInfo.nrOfFramesInImage = (UInt32) i;
            IMGCODECPLUGIN_FreeImageStorage( image, GUCEF_NULL );
            return 0;
        }
        
        // Set basic frame info
        TImageFrame& frame = image->frames[ i ];
        frame.version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;        
        frame.frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
        frame.frameInfo.frameDurationInMs = (UInt32) flicHeader.speed; // FLC speed is the delay between frames in ms
        
        // FLIC only has 1 level per frame
        frame.mipmapLevel = (TImageMipMapLevel*) malloc( sizeof(TImageMipMapLevel) ); 
        if ( NULL == frame.mipmapLevel )
        {
            image->imageInfo.nrOfFramesInImage = (UInt32) i;
            IMGCODECPLUGIN_FreeImageStorage( image, GUCEF_NULL );
            return 0;
        }
        memset( frame.mipmapLevel, 0, sizeof(TImageMipMapLevel) );
        frame.frameInfo.nrOfMipmapLevels = 1;

        TImageMipMapLevel& mipmapLevel = frame.mipmapLevel[ 0 ];
        mipmapLevel.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
        mipmapLevel.mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;        
        
        // width and height are the same for every frame
        mipmapLevel.mipLevelInfo.frameHeight = (UInt32) flicHeader.height;
        mipmapLevel.mipLevelInfo.frameWidth = (UInt32) flicHeader.width;

        // There is a palette per frame but we inflate that right away, not worth the effort
        mipmapLevel.mipLevelInfo.pixelComponentDataType = GUCEF_DATATYPE_UINT8;
        mipmapLevel.mipLevelInfo.pixelStorageFormat = PSF_RGB;
        mipmapLevel.pixelDataSizeInBytes = pixelCount * 3;
        mipmapLevel.pixelData = malloc( mipmapLevel.pixelDataSizeInBytes );
        if ( GUCEF_NULL == mipmapLevel.pixelData )
        {
            image->imageInfo.nrOfFramesInImage = (UInt32) i;
            IMGCODECPLUGIN_FreeImageStorage( image, GUCEF_NULL );
            return 0;
        }

        UInt8* pixelData = (UInt8*) mipmapLevel.pixelData;
        for ( UInt32 n=0; n<pixelCount; ++n )
        {
            const flic::Color& pixel = flicFrame.colormap[ indices[ n ] ];
            *pixelData = pixel.r; ++pixelData;
            *pixelData = pixel.g; ++pixelData;
            *pixelData = pixel.b; ++pixelData;
        }
    }
    
    *imageOutput = image;
    *imageData = NULL;
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGCODECPLUGIN_DecodeImageMetaData( void* pluginData                             ,
                                    void* codecData                              ,
                                    const char* codecType                        ,
                                    CORE::TIOAccess* input                       ,
                                    CORE::TValueMapParserCallbacks* mapCallbacks )
{
    /* this format does not support metadata, frame timing is part of the image frame data */
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
IMGCODECPLUGIN_EncodeImage( void* pluginData        ,
                            void* codecData         ,
                            const char* codecType   ,
                            TImage* inputImage      ,
                            CORE::TIOAccess* output )
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
