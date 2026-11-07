/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREcodectypes_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREcodectypes_H
#endif /* GUCEF_CORE_COREcodectypes_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_c_imagedata.h"
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

#include "gucefIMAGE_CPngImageCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CPngImageCodec::CodecFamilyName = "ImageCodec";
const CString CPngImageCodec::CodecTypeName = "png";
const CString CPngImageCodec::CompressionCodecTypeName = "zlib";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The CRC-32 (polynomial 0xEDB88320) of every 4 bit value: two lookups per byte, no table to build */
const UInt32 g_crc32NibbleTable[ 16 ] =
{
    0x00000000u, 0x1DB71064u, 0x3B6E20C8u, 0x26D930ACu, 0x76DC4190u, 0x6B6B51F4u, 0x4DB26158u, 0x5005713Cu,
    0xEDB88320u, 0xF00F9344u, 0xD6D6A3E8u, 0xCB61B38Cu, 0x9B64C2B0u, 0x86D3D2D4u, 0xA00AE278u, 0xBDBDF21Cu
};

/** The size of the IDAT chunks the compressed image data is split into */
const UInt32 IdatChunkSize = 1048576;

/** The most bytes of filtered image data a picture may have: CDynamicBuffer sizes are 32 bit */
const UInt64 MaxFilteredImageDataSize = 2000000000;

/** The largest stored deflate block */
const UInt32 MaxStoredBlockSize = 65535;

/** The most bytes the Adler-32 sums take before they have to be reduced, as zlib computes it */
const UInt32 AdlerRunLength = 5552;

const UInt32 AdlerModulo = 65521;

/** PNG colour types */
const UInt8 PngColourTypeGreyscale = 0;
const UInt8 PngColourTypeTruecolour = 2;
const UInt8 PngColourTypeTruecolourWithAlpha = 6;

/*-------------------------------------------------------------------------*/

void
PutUInt32( UInt8* target, UInt32 value )
{GUCEF_TRACE;

    target[ 0 ] = (UInt8) ( value >> 24 );
    target[ 1 ] = (UInt8) ( value >> 16 );
    target[ 2 ] = (UInt8) ( value >> 8 );
    target[ 3 ] = (UInt8) value;
}

/*-------------------------------------------------------------------------*/

void
AppendUInt32( CORE::CDynamicBuffer& buffer, UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ];
    PutUInt32( bytes, value );
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

void
AppendChunk( CORE::CDynamicBuffer& png, const char* type, const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    AppendUInt32( png, size );
    png.Append( type, 4 );
    UInt32 crc = CPngImageCodec::Crc32( reinterpret_cast< const UInt8* >( type ), 4 );
    if ( size > 0 )
    {
        png.Append( data, size );
        crc = CPngImageCodec::Crc32( data, size, crc );
    }
    AppendUInt32( png, crc );
}

/*-------------------------------------------------------------------------*/

UInt32
Adler32( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    UInt32 a = 1;
    UInt32 b = 0;
    while ( size > 0 )
    {
        UInt32 run = size < AdlerRunLength ? size : AdlerRunLength;
        size -= run;
        while ( run > 0 )
        {
            a += *data;
            b += a;
            ++data;
            --run;
        }
        a %= AdlerModulo;
        b %= AdlerModulo;
    }
    return ( b << 16 ) | a;
}

/*-------------------------------------------------------------------------*/

/** A zlib stream of stored deflate blocks: no compression, readable by every inflater */
void
StoreUncompressed( const UInt8* data, UInt32 size, CORE::CDynamicBuffer& out )
{GUCEF_TRACE;

    const UInt32 blockCount = 0 == size ? 1 : ( size + MaxStoredBlockSize - 1 ) / MaxStoredBlockSize;
    out.Clear();
    out.SetBufferSize( 2 + blockCount * 5 + size + 4 );

    // CMF: deflate with a 32K window, FLG: fastest, no dictionary, (CMF * 256 + FLG) a multiple of 31
    const UInt8 header[ 2 ] = { 0x78, 0x01 };
    out.Append( header, 2 );

    UInt32 offset = 0;
    do
    {
        const UInt32 block = size - offset > MaxStoredBlockSize ? MaxStoredBlockSize : size - offset;
        const bool last = offset + block >= size;
        const UInt8 blockHeader[ 5 ] = { (UInt8) ( last ? 1 : 0 )                 ,   // BFINAL, BTYPE 00: stored
                                         (UInt8) ( block & 0xFF )                  ,
                                         (UInt8) ( ( block >> 8 ) & 0xFF )         ,
                                         (UInt8) ( ~block & 0xFF )                 ,
                                         (UInt8) ( ( ~block >> 8 ) & 0xFF )        };
        out.Append( blockHeader, 5 );
        if ( block > 0 )
            out.Append( data + offset, block );
        offset += block;
    }
    while ( offset < size );

    AppendUInt32( out, Adler32( data, size ) );
}

/*-------------------------------------------------------------------------*/

/** The zlib stream of the data: through the "zlib" codec of the codec registry when there is one, stored blocks otherwise */
void
MakeZlibStream( CORE::CDynamicBuffer& raw, CORE::CDynamicBuffer& compressed )
{GUCEF_TRACE;

    // the provider of the highest priority; GetCodecs() is used because its absence is not an error here
    CORE::CCodecRegistry::TICodecPtrVector codecs;
    CORE::CCodecRegistry::TICodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().GetCodecs( CORE::CoreCodecTypes::CompressionCodec, CPngImageCodec::CompressionCodecTypeName, codecs ) &&
         !codecs.empty() )
    {
        codec = codecs.front();
    }
    if ( !codec.IsNULL() )
    {
        CORE::CDynamicBufferAccess source( &raw, false );
        CORE::CDynamicBufferAccess dest( &compressed, false );
        if ( codec->Encode( source, dest ) && compressed.GetDataSize() > 0 )
            return;

        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The \"" << CPngImageCodec::CompressionCodecTypeName <<
            "\" codec of the codec registry failed to compress " << raw.GetDataSize() << " bytes, the image data is stored uncompressed" );
    }

    StoreUncompressed( static_cast< const UInt8* >( raw.GetConstBufferPtr() ), raw.GetDataSize(), compressed );
}

/*-------------------------------------------------------------------------*/

/** Whether the pixel layout can be written, with its PNG colour type and whether red and blue have to be swapped */
bool
GetPngLayout( TPixelStorageFormat format       ,
              TBuildinDataType channelType     ,
              UInt8& colourType                ,
              UInt32& channels                 ,
              bool& swapRedAndBlue             ,
              CString& problem                 )
{GUCEF_TRACE;

    if ( DATATYPE_UINT8 != channelType )
    {
        problem = "only pictures with 8 bit unsigned channels are written as PNG";
        return false;
    }

    swapRedAndBlue = false;
    switch ( format )
    {
        case PSF_RGBA :
        {
            colourType = PngColourTypeTruecolourWithAlpha;
            channels = 4;
            return true;
        }
        case PSF_BGRA :
        {
            colourType = PngColourTypeTruecolourWithAlpha;
            channels = 4;
            swapRedAndBlue = true;
            return true;
        }
        case PSF_RGB :
        {
            colourType = PngColourTypeTruecolour;
            channels = 3;
            return true;
        }
        case PSF_BGR :
        {
            colourType = PngColourTypeTruecolour;
            channels = 3;
            swapRedAndBlue = true;
            return true;
        }
        case PSF_SINGLE_CHANNEL_GRAYSCALE :
        {
            colourType = PngColourTypeGreyscale;
            channels = 1;
            return true;
        }
        default :
        {
            problem = "only RGB, RGBA, BGR, BGRA and greyscale pictures are written as PNG";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The PNG of pixel data laid out as 'format' with 8 bit channels, the lines one after the other from the top */
bool
EncodePixels( const UInt8* pixels               ,
              UInt32 pixelDataSize              ,
              UInt32 width                      ,
              UInt32 height                     ,
              TPixelStorageFormat format        ,
              TBuildinDataType channelType      ,
              CORE::CDynamicBuffer& png         ,
              CString& problem                  )
{GUCEF_TRACE;

    png.Clear();

    UInt8 colourType = 0;
    UInt32 channels = 0;
    bool swapRedAndBlue = false;
    if ( !GetPngLayout( format, channelType, colourType, channels, swapRedAndBlue, problem ) )
        return false;
    if ( 0 == width || 0 == height || GUCEF_NULL == pixels )
    {
        problem = "an empty picture is not written as PNG";
        return false;
    }
    if ( width > 0x7FFFFFFFu || height > 0x7FFFFFFFu )
    {
        problem = "the picture is wider or higher than PNG allows";
        return false;
    }
    const UInt64 lineBytes = (UInt64) width * channels;
    const UInt64 filteredSize = ( lineBytes + 1 ) * height;
    if ( filteredSize > MaxFilteredImageDataSize )
    {
        problem = "the picture is too large to be written as PNG in memory";
        return false;
    }
    if ( (UInt64) pixelDataSize < lineBytes * height )
    {
        problem = "the pixel data is smaller than the size of the picture";
        return false;
    }

    // the lines, each after its filter byte (0: none)
    CORE::CDynamicBuffer raw;
    raw.SetDataSize( (UInt32) filteredSize );
    UInt8* target = static_cast< UInt8* >( raw.GetBufferPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        const UInt8* line = pixels + (size_t) y * (size_t) lineBytes;
        *target = 0;
        ++target;
        if ( swapRedAndBlue )
        {
            for ( UInt32 x=0; x<width; ++x )
            {
                target[ 0 ] = line[ 2 ];
                target[ 1 ] = line[ 1 ];
                target[ 2 ] = line[ 0 ];
                if ( 4 == channels )
                    target[ 3 ] = line[ 3 ];
                target += channels;
                line += channels;
            }
        }
        else
        {
            memcpy( target, line, (size_t) lineBytes );
            target += lineBytes;
        }
    }

    CORE::CDynamicBuffer compressed;
    MakeZlibStream( raw, compressed );

    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    const UInt32 compressedSize = compressed.GetDataSize();
    const UInt32 idatChunkCount = ( compressedSize + IdatChunkSize - 1 ) / IdatChunkSize;
    png.SetBufferSize( 8 + 25 + compressedSize + idatChunkCount * 12 + 12 );
    png.Append( signature, 8 );

    UInt8 header[ 13 ];
    PutUInt32( header, width );
    PutUInt32( header + 4, height );
    header[ 8 ] = 8;            // bits per channel
    header[ 9 ] = colourType;
    header[ 10 ] = 0;           // compression method: deflate
    header[ 11 ] = 0;           // filter method 0
    header[ 12 ] = 0;           // not interlaced
    AppendChunk( png, "IHDR", header, 13 );

    const UInt8* compressedData = static_cast< const UInt8* >( compressed.GetConstBufferPtr() );
    UInt32 offset = 0;
    while ( offset < compressedSize )
    {
        const UInt32 chunk = compressedSize - offset > IdatChunkSize ? IdatChunkSize : compressedSize - offset;
        AppendChunk( png, "IDAT", compressedData + offset, chunk );
        offset += chunk;
    }

    AppendChunk( png, "IEND", GUCEF_NULL, 0 );
    return true;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPngImageCodec::CPngImageCodec( void )
    : CIImageCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPngImageCodec::CPngImageCodec( const CPngImageCodec& src )
    : CIImageCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPngImageCodec::~CPngImageCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPngImageCodec&
CPngImageCodec::operator=( const CPngImageCodec& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CIImageCodec::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CPngImageCodec::Crc32( const UInt8* data, UInt32 size, UInt32 crc )
{GUCEF_TRACE;

    UInt32 c = crc ^ 0xFFFFFFFFu;
    for ( UInt32 i=0; i<size; ++i )
    {
        c ^= data[ i ];
        c = g_crc32NibbleTable[ c & 0x0F ] ^ ( c >> 4 );
        c = g_crc32NibbleTable[ c & 0x0F ] ^ ( c >> 4 );
    }
    return c ^ 0xFFFFFFFFu;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::IsPixelMapSupported( const CPixelMap& pixels, CString& problem )
{GUCEF_TRACE;

    UInt8 colourType = 0;
    UInt32 channels = 0;
    bool swapRedAndBlue = false;
    return GetPngLayout( pixels.GetPixelStorageFormat(), pixels.GetPixelComponentDataType(), colourType, channels, swapRedAndBlue, problem );
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::EncodePixelMap( const CPixelMap& pixels     ,
                                CORE::CDynamicBuffer& png   ,
                                CString& problem            )
{GUCEF_TRACE;

    return EncodePixels( static_cast< const UInt8* >( pixels.GetDataPtr() ) ,
                         pixels.GetTotalSizeInBytes()                       ,
                         pixels.GetWidthInPixels()                          ,
                         pixels.GetHeightInPixels()                         ,
                         pixels.GetPixelStorageFormat()                     ,
                         pixels.GetPixelComponentDataType()                 ,
                         png                                                ,
                         problem                                            );
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::WritePixelMapToFile( const CPixelMap& pixels   ,
                                     const CString& path       ,
                                     CString& problem          )
{GUCEF_TRACE;

    CORE::CDynamicBuffer png;
    if ( !EncodePixelMap( pixels, png, problem ) )
        return false;
    if ( !png.WriteContentToFile( path, 0, true ) )
    {
        problem = "the file " + path + " cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Encode( const CImage& inputImage       ,
                        CORE::CIOAccess& encodedOutput )
{GUCEF_TRACE;

    TPixelMapPtr pixels;
    if ( !inputImage.TryGetPixelMap( 0, 0, pixels ) || pixels.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The image has no picture to encode" );
        return false;
    }

    CORE::CDynamicBuffer png;
    CString problem;
    if ( !EncodePixelMap( *pixels, png, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The image cannot be encoded: " << problem );
        return false;
    }
    return 1 == encodedOutput.Write( png.GetConstBufferPtr(), png.GetDataSize(), 1 );
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Decode( CORE::CIOAccess& encodedInput ,
                        CImage& outputImage           )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The built-in PNG codec only encodes, load an image codec plugin to decode PNG" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                CORE::CValueList& metaData    )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The built-in PNG codec only encodes, load an image codec plugin to read PNG meta data" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::EncodeSerializedImage( CORE::CIOAccess& source ,
                                       CORE::CIOAccess& dest   ,
                                       CString& problem        )
{GUCEF_TRACE;

    TImageInfo imageInfo;
    if ( 1 != source.Read( &imageInfo, sizeof( imageInfo ), 1 ) || GUCEF_IMAGE_TIMAGEINFO_VERSION != imageInfo.version )
    {
        problem = "the input is not a serialized image";
        return false;
    }
    if ( 0 == imageInfo.nrOfFramesInImage )
    {
        problem = "the image has no frames";
        return false;
    }

    // the infos of all frames and levels come first, the pixel data of the first level of the first frame after them
    TImageMipMapLevelInfo firstLevel;
    memset( &firstLevel, 0, sizeof( firstLevel ) );
    for ( UInt32 frame=0; frame<imageInfo.nrOfFramesInImage; ++frame )
    {
        TImageFrameInfo frameInfo;
        if ( 1 != source.Read( &frameInfo, sizeof( frameInfo ), 1 ) || GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION != frameInfo.version )
        {
            problem = "the frame information of the serialized image is incomplete";
            return false;
        }
        if ( 0 == frame && 0 == frameInfo.nrOfMipmapLevels )
        {
            problem = "the first frame of the image has no picture";
            return false;
        }
        for ( UInt32 level=0; level<frameInfo.nrOfMipmapLevels; ++level )
        {
            TImageMipMapLevelInfo levelInfo;
            if ( 1 != source.Read( &levelInfo, sizeof( levelInfo ), 1 ) || GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION != levelInfo.version )
            {
                problem = "the mipmap level information of the serialized image is incomplete";
                return false;
            }
            if ( 0 == frame && 0 == level )
                firstLevel = levelInfo;
        }
    }

    const TPixelStorageFormat format = (TPixelStorageFormat) firstLevel.pixelStorageFormat;
    const TBuildinDataType channelType = (TBuildinDataType) firstLevel.pixelComponentDataType;
    UInt8 colourType = 0;
    UInt32 channels = 0;
    bool swapRedAndBlue = false;
    if ( !GetPngLayout( format, channelType, colourType, channels, swapRedAndBlue, problem ) )
        return false;
    const UInt64 pixelDataSize = (UInt64) firstLevel.frameWidth * firstLevel.frameHeight * channels;
    if ( 0 == pixelDataSize || pixelDataSize > MaxFilteredImageDataSize )
    {
        problem = "the picture of the serialized image is empty or too large";
        return false;
    }

    CORE::CDynamicBuffer pixels;
    pixels.SetDataSize( (UInt32) pixelDataSize );
    if ( 1 != source.Read( pixels.GetBufferPtr(), (UInt32) pixelDataSize, 1 ) )
    {
        problem = "the pixel data of the serialized image is incomplete";
        return false;
    }

    CORE::CDynamicBuffer png;
    if ( !EncodePixels( static_cast< const UInt8* >( pixels.GetConstBufferPtr() ), pixels.GetDataSize(), firstLevel.frameWidth,
                        firstLevel.frameHeight, format, channelType, png, problem ) )
        return false;

    if ( 1 != dest.Write( png.GetConstBufferPtr(), png.GetDataSize(), 1 ) )
    {
        problem = "the PNG cannot be written to the output";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Encode( CORE::CIOAccess& source ,
                        CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    CString problem;
    if ( EncodeSerializedImage( source, dest, problem ) )
        return true;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The image cannot be encoded: " << problem );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Decode( CORE::CIOAccess& source ,
                        CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The built-in PNG codec only encodes, load an image codec plugin to decode PNG" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Encode( CORE::CIOAccess& source          ,
                        CORE::CIOAccess& dest            ,
                        const CORE::CValueList& params   )
{GUCEF_TRACE;

    CString problem;
    if ( EncodeSerializedImage( source, dest, problem ) )
    {
        SetLastStatus( CORE::GUCEF_CODEC_STATUS_OK );
        return true;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The image cannot be encoded: " << problem );
    SetLastStatus( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT, problem.C_String() );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPngImageCodec::Decode( CORE::CIOAccess& source          ,
                        CORE::CIOAccess& dest            ,
                        const CORE::CValueList& params   )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PngImageCodec: The built-in PNG codec only encodes, load an image codec plugin to decode PNG" );
    SetLastStatus( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED, "The built-in PNG codec only encodes" );
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CPngImageCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_CODEC_CAPABILITY_ENCODE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPngImageCodec::GetType( void ) const
{GUCEF_TRACE;

    return CodecTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPngImageCodec::GetFamilyName( void ) const
{GUCEF_TRACE;

    return CodecFamilyName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPngImageCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPngImageCodec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CPngImageCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::IMAGE::CPngImageCodec";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
