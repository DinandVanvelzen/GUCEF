/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_COREcodectypes_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREcodectypes_H
#endif /* GUCEF_CORE_COREcodectypes_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "TestPngImageCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The plugin group the tests load their plugins in, so unloading it touches nothing else */
const char* PngTestPluginGroup = "pngimagecodectests";

/** What the walk over the chunks of a PNG found */
struct SPngWalk
{
    UInt32 width;
    UInt32 height;
    UInt8 bitDepth;
    UInt8 colourType;
    UInt8 compressionMethod;
    UInt8 filterMethod;
    UInt8 interlaceMethod;
    UInt32 idatChunkCount;
    CORE::CDynamicBuffer zlibStream;    /**< the IDAT chunks joined */
};

/*-------------------------------------------------------------------------*/

UInt32
ReadUInt32( const UInt8* p )
{
    return ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | p[ 3 ];
}

/*-------------------------------------------------------------------------*/

/**
 *  Walks the chunks of a PNG: the signature, IHDR first with 13 bytes, the CRC of every chunk, the IDAT chunks
 *  one after the other, IEND last with nothing after it
 */
bool
WalkPng( const CORE::CDynamicBuffer& png, SPngWalk& walk )
{
    const UInt8* data = static_cast< const UInt8* >( png.GetConstBufferPtr() );
    const UInt32 size = png.GetDataSize();
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    if ( size < 8 || 0 != memcmp( data, signature, 8 ) )
        return false;

    walk.idatChunkCount = 0;
    walk.zlibStream.Clear();
    bool headerSeen = false;
    bool endSeen = false;
    bool idatDone = false;
    UInt32 offset = 8;
    while ( offset < size )
    {
        if ( endSeen || offset + 12 > size )
            return false;
        const UInt32 length = ReadUInt32( data + offset );
        if ( (UInt64) offset + 12 + length > size )
            return false;
        const UInt8* type = data + offset + 4;
        if ( IMAGE::CPngImageCodec::Crc32( type, 4 + length ) != ReadUInt32( data + offset + 8 + length ) )
            return false;

        if ( 0 == memcmp( type, "IHDR", 4 ) )
        {
            if ( headerSeen || 8 != offset || 13 != length )
                return false;
            headerSeen = true;
            walk.width = ReadUInt32( type + 4 );
            walk.height = ReadUInt32( type + 8 );
            walk.bitDepth = type[ 12 ];
            walk.colourType = type[ 13 ];
            walk.compressionMethod = type[ 14 ];
            walk.filterMethod = type[ 15 ];
            walk.interlaceMethod = type[ 16 ];
        }
        else if ( 0 == memcmp( type, "IDAT", 4 ) )
        {
            if ( !headerSeen || idatDone )
                return false;
            walk.zlibStream.Append( type + 4, length );
            ++walk.idatChunkCount;
        }
        else if ( 0 == memcmp( type, "IEND", 4 ) )
        {
            if ( 0 != length )
                return false;
            endSeen = true;
        }
        else
        {
            return false;   // the encoder writes no other chunks
        }
        if ( walk.idatChunkCount > 0 && 0 != memcmp( type, "IDAT", 4 ) )
            idatDone = true;
        offset += 12 + length;
    }
    return headerSeen && endSeen && walk.idatChunkCount > 0;
}

/*-------------------------------------------------------------------------*/

/** The Adler-32 the plain way, a byte at a time */
UInt32
ReferenceAdler32( const UInt8* data, UInt32 size )
{
    UInt32 a = 1, b = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        a = ( a + data[ i ] ) % 65521;
        b = ( b + a ) % 65521;
    }
    return ( b << 16 ) | a;
}

/*-------------------------------------------------------------------------*/

/** Whether the first deflate block of a zlib stream is a stored block */
bool
StartsWithStoredBlock( const CORE::CDynamicBuffer& zlib )
{
    if ( zlib.GetDataSize() < 3 )
        return false;
    const UInt8 blockHeader = static_cast< const UInt8* >( zlib.GetConstBufferPtr() )[ 2 ];
    return 0 == ( ( blockHeader >> 1 ) & 0x03 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Decodes a zlib stream made of stored deflate blocks only: the header, every block with LEN and NLEN,
 *  the Adler-32 at the end. False for anything else
 */
bool
InflateStoredBlocks( const CORE::CDynamicBuffer& zlib, CORE::CDynamicBuffer& raw )
{
    const UInt8* data = static_cast< const UInt8* >( zlib.GetConstBufferPtr() );
    const UInt32 size = zlib.GetDataSize();
    raw.Clear();
    if ( size < 2 + 5 + 4 )
        return false;
    if ( 0x08 != ( data[ 0 ] & 0x0F ) || 0 != ( ( (UInt32) data[ 0 ] << 8 ) | data[ 1 ] ) % 31 || 0 != ( data[ 1 ] & 0x20 ) )
        return false;

    UInt32 offset = 2;
    bool last = false;
    while ( !last )
    {
        if ( offset + 5 > size )
            return false;
        const UInt8 blockHeader = data[ offset ];
        last = 0 != ( blockHeader & 0x01 );
        if ( 0 != ( ( blockHeader >> 1 ) & 0x03 ) )
            return false;   // not a stored block
        const UInt32 length = data[ offset + 1 ] | ( (UInt32) data[ offset + 2 ] << 8 );
        const UInt32 lengthComplement = data[ offset + 3 ] | ( (UInt32) data[ offset + 4 ] << 8 );
        if ( ( length ^ 0xFFFF ) != lengthComplement )
            return false;
        offset += 5;
        if ( offset + length > size )
            return false;
        raw.Append( data + offset, length );
        offset += length;
    }
    if ( offset + 4 != size )
        return false;
    return ReferenceAdler32( static_cast< const UInt8* >( raw.GetConstBufferPtr() ), raw.GetDataSize() ) == ReadUInt32( data + offset );
}

/*-------------------------------------------------------------------------*/

/** Inflates a zlib stream with the "zlib" codec of the codec registry */
bool
InflateWithRegistryCodec( CORE::CDynamicBuffer& zlib, CORE::CDynamicBuffer& raw )
{
    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" );
    if ( codec.IsNULL() )
        return false;
    raw.Clear();
    CORE::CDynamicBufferAccess source( &zlib, false );
    CORE::CDynamicBufferAccess dest( &raw, false );
    return codec->Decode( source, dest );
}

/*-------------------------------------------------------------------------*/

/** A picture with a pattern in every channel: 'seed' tells pictures apart */
IMAGE::TPixelMapPtr
MakePicture( UInt32 width, UInt32 height, IMAGE::TPixelStorageFormat format, UInt32 seed )
{
    IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, format, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( picture->GetDataPtr() );
    const UInt32 size = picture->GetTotalSizeInBytes();
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( i * 7 + seed + ( i / 251 ) );
    return picture;
}

/*-------------------------------------------------------------------------*/

/** A picture that compresses well: smooth gradients */
IMAGE::TPixelMapPtr
MakeGradient( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( picture->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt8* pixel = data + ( (size_t) y * width + x ) * 4;
            pixel[ 0 ] = (UInt8) x;
            pixel[ 1 ] = (UInt8) y;
            pixel[ 2 ] = (UInt8) ( ( x + y ) / 2 );
            pixel[ 3 ] = 255;
        }
    }
    return picture;
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether the filtered lines hold the pixels: each line a filter byte 0 and the line, red and blue of
 *  BGR(A) pixels swapped to RGB(A)
 */
bool
LinesHoldPixels( const CORE::CDynamicBuffer& raw, const IMAGE::CPixelMap& pixels, bool swapRedAndBlue )
{
    const UInt32 width = pixels.GetWidthInPixels();
    const UInt32 height = pixels.GetHeightInPixels();
    const UInt32 channels = pixels.GetNumberOfChannelsPerPixel();
    const UInt32 lineBytes = width * channels;
    if ( raw.GetDataSize() != ( lineBytes + 1 ) * height )
        return false;

    const UInt8* lines = static_cast< const UInt8* >( raw.GetConstBufferPtr() );
    const UInt8* source = static_cast< const UInt8* >( pixels.GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        const UInt8* line = lines + (size_t) y * ( lineBytes + 1 );
        if ( 0 != line[ 0 ] )
            return false;
        ++line;
        const UInt8* sourceLine = source + (size_t) y * lineBytes;
        for ( UInt32 x=0; x<width; ++x )
        {
            for ( UInt32 c=0; c<channels; ++c )
            {
                UInt32 sourceChannel = c;
                if ( swapRedAndBlue && c < 3 )
                    sourceChannel = 2 - c;
                if ( line[ x * channels + c ] != sourceLine[ x * channels + sourceChannel ] )
                    return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A "zlib" compression codec that always fails, to see the encoder fall back to stored blocks */
class CFailingZlibCodec : public CORE::CICodec
{
    public:

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "zlib"; }

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CoreCodecTypes::CompressionCodec; }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CFailingZlibCodec(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CFailingZlibCodec";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

bool
IsBuiltInPngCodec( const CORE::CCodecRegistry::TICodecPtr& codec )
{
    return !codec.IsNULL() && "GUCEF::IMAGE::CPngImageCodec" == codec->GetClassTypeName();
}

/*-------------------------------------------------------------------------*/

bool
LoadZlibCodecPlugin( void )
{
    try
    {
        #ifdef GUCEF_DEBUG_MODE
        const CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginZLIB_d";
        const CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginZLIB";
        #else
        const CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginZLIB";
        const CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginZLIB_d";
        #endif

        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        if ( pluginControl.AddPluginFromDir( firstPluginPath, PngTestPluginGroup, true ) ||
             pluginControl.AddPluginFromDir( secondPluginPath, PngTestPluginGroup, true ) )
        {
            return !CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" ).IsNULL();
        }
    }
    catch ( ... )
    {
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
LoadFreeImageCodecPlugin( void )
{
    try
    {
        CORE::CPluginMetaData pluginMetaData;
        pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
        pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
        pluginMetaData.SetModuleFilename( "imgpluginFreeImage" );
        pluginMetaData.SetAltModuleFilename( "imgpluginFreeImage_d" );
        pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );

        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        return pluginControl.AddPluginMetaData( pluginMetaData, PngTestPluginGroup, true, true );
    }
    catch ( ... )
    {
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
UnloadTestPlugins( void )
{
    try
    {
        return CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PngTestPluginGroup );
    }
    catch ( ... )
    {
    }
    return false;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestCrc32( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: the CRC of the chunks is that of ISO 3309" )
        try
        {
            // the CRC of the IEND chunk type is the one every PNG ends with, 0xCBF43926 the check value of CRC-32
            ASSERT_TRUE( 0xAE426082u == IMAGE::CPngImageCodec::Crc32( reinterpret_cast< const UInt8* >( "IEND" ), 4 ) );
            ASSERT_TRUE( 0xCBF43926u == IMAGE::CPngImageCodec::Crc32( reinterpret_cast< const UInt8* >( "123456789" ), 9 ) );
            ASSERT_TRUE( 0u == IMAGE::CPngImageCodec::Crc32( GUCEF_NULL, 0 ) );

            // continued in parts it is the same
            const UInt32 firstPart = IMAGE::CPngImageCodec::Crc32( reinterpret_cast< const UInt8* >( "1234" ), 4 );
            ASSERT_TRUE( 0xCBF43926u == IMAGE::CPngImageCodec::Crc32( reinterpret_cast< const UInt8* >( "56789" ), 5, firstPart ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestStoredEncoding( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: RGBA, RGB, BGRA, BGR and greyscale pictures are written exactly, in stored blocks without a zlib codec" )
        try
        {
            // no compression codec is loaded at this point of the test application
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" ).IsNULL() );

            struct SFormatCase
            {
                IMAGE::TPixelStorageFormat format;
                UInt8 colourType;
                bool swapRedAndBlue;
            };
            const SFormatCase cases[] = { { IMAGE::PSF_RGBA, 6, false },
                                          { IMAGE::PSF_RGB, 2, false },
                                          { IMAGE::PSF_BGRA, 6, true },
                                          { IMAGE::PSF_BGR, 2, true },
                                          { IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, 0, false } };

            for ( UInt32 i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                IMAGE::TPixelMapPtr pixels = MakePicture( 5, 3, cases[ i ].format, i );
                CORE::CString problem;
                ASSERT_TRUE( IMAGE::CPngImageCodec::IsPixelMapSupported( *pixels, problem ) );

                CORE::CDynamicBuffer png;
                ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );

                SPngWalk walk;
                ASSERT_TRUE( WalkPng( png, walk ) );
                ASSERT_TRUE( 5 == walk.width && 3 == walk.height );
                ASSERT_TRUE( 8 == walk.bitDepth && cases[ i ].colourType == walk.colourType );
                ASSERT_TRUE( 0 == walk.compressionMethod && 0 == walk.filterMethod && 0 == walk.interlaceMethod );
                ASSERT_TRUE( 1 == walk.idatChunkCount );

                CORE::CDynamicBuffer raw;
                ASSERT_TRUE( StartsWithStoredBlock( walk.zlibStream ) );
                ASSERT_TRUE( InflateStoredBlocks( walk.zlibStream, raw ) );
                ASSERT_TRUE( LinesHoldPixels( raw, *pixels, cases[ i ].swapRedAndBlue ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestLargePicture( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: a picture larger than one IDAT chunk is split over several, in stored blocks of at most 65535 bytes" )
        try
        {
            // 700 x 500 RGBA: 1,400,500 bytes of filtered lines, more than one IDAT chunk of 1 MiB and many stored blocks
            IMAGE::TPixelMapPtr pixels = MakePicture( 700, 500, IMAGE::PSF_RGBA, 3 );
            CORE::CDynamicBuffer png;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );

            SPngWalk walk;
            ASSERT_TRUE( WalkPng( png, walk ) );
            ASSERT_TRUE( 700 == walk.width && 500 == walk.height );
            ASSERT_TRUE( 2 == walk.idatChunkCount );

            CORE::CDynamicBuffer raw;
            ASSERT_TRUE( InflateStoredBlocks( walk.zlibStream, raw ) );
            ASSERT_TRUE( LinesHoldPixels( raw, *pixels, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRefusedFormats( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: pictures that have no PNG equivalent are refused with a reason, not converted" )
        try
        {
            IMAGE::CPixelMap hsv( GUCEF_NULL, 2, 2, IMAGE::PSF_HSV, GUCEF::DATATYPE_UINT8 );
            IMAGE::CPixelMap luminance( GUCEF_NULL, 2, 2, IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE, GUCEF::DATATYPE_UINT8 );
            IMAGE::CPixelMap deep( GUCEF_NULL, 2, 2, IMAGE::PSF_RGBA, GUCEF::DATATYPE_LE_UINT16 );
            IMAGE::CPixelMap floats( GUCEF_NULL, 2, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 );
            IMAGE::CPixelMap empty( GUCEF_NULL, 0, 0, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
            const IMAGE::CPixelMap* refused[] = { &hsv, &luminance, &deep, &floats, &empty };

            for ( UInt32 i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                CORE::CDynamicBuffer png;
                png.Append( "old", 3 );
                CORE::CString problem;
                ASSERT_FALSE( IMAGE::CPngImageCodec::EncodePixelMap( *refused[ i ], png, problem ) );
                ASSERT_FALSE( problem.IsNULLOrEmpty() );
                ASSERT_TRUE( 0 == png.GetDataSize() );
            }

            CORE::CString problem;
            ASSERT_FALSE( IMAGE::CPngImageCodec::IsPixelMapSupported( hsv, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );

            // nothing is written for a refused picture
            const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "refused_png_codec_test.png" );
            problem.Clear();
            ASSERT_FALSE( IMAGE::CPngImageCodec::WritePixelMapToFile( hsv, path, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );
            ASSERT_FALSE( CORE::FileExists( path ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestWriteToFile( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: a picture written to a file has the bytes of the encoded picture and replaces the file" )
        try
        {
            IMAGE::TPixelMapPtr pixels = MakePicture( 4, 4, IMAGE::PSF_RGB, 9 );
            CORE::CDynamicBuffer png;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );

            const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "png_codec_test.png" );
            CORE::CDynamicBuffer previous;
            const char* longerContent = "a file that is longer than the PNG of 4 by 4 pixels, so that writing the picture without truncating the file "
                                        "leaves bytes of it behind, which the comparison of the sizes shows";
            previous.Append( longerContent, (UInt32) strlen( longerContent ) );
            ASSERT_TRUE( previous.GetDataSize() > png.GetDataSize() );
            ASSERT_TRUE( previous.WriteContentToFile( path, 0, true ) );

            ASSERT_TRUE( IMAGE::CPngImageCodec::WritePixelMapToFile( *pixels, path, problem ) );
            CORE::CDynamicBuffer written;
            ASSERT_TRUE( written.LoadContentFromFile( path ) );
            ASSERT_TRUE( written.GetDataSize() == png.GetDataSize() );
            ASSERT_TRUE( written.GetDataSize() == png.GetDataSize() && 0 == memcmp( written.GetConstBufferPtr(), png.GetConstBufferPtr(), png.GetDataSize() ) );
            ASSERT_TRUE( CORE::DeleteFile( path ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFailingCompressionCodec( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: when the zlib codec of the registry fails the image data is stored and the PNG is still valid" )
        try
        {
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecPtr failing = CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CFailingZlibCodec() );
            ASSERT_TRUE( codecRegistry.RegisterCodec( failing, CORE::CCodecRegistry::OsCodecPriority + 1000 ) );
            ASSERT_TRUE( codecRegistry.TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" ) == failing );

            IMAGE::TPixelMapPtr pixels = MakePicture( 6, 2, IMAGE::PSF_RGBA, 5 );
            CORE::CDynamicBuffer png;
            CORE::CString problem;
            const bool encoded = IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem );
            ASSERT_TRUE( codecRegistry.UnregisterCodec( failing ) );
            ASSERT_TRUE( encoded );

            SPngWalk walk;
            ASSERT_TRUE( WalkPng( png, walk ) );
            CORE::CDynamicBuffer raw;
            ASSERT_TRUE( InflateStoredBlocks( walk.zlibStream, raw ) );
            ASSERT_TRUE( LinesHoldPixels( raw, *pixels, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestBuiltInCodecInTheRegistry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "PNG codec: the built-in codec is png of ImageCodec in the codec registry, with the priority of software codecs" )
        try
        {
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecPtr codec = codecRegistry.TryGetCodec( "ImageCodec", "png" );
            ASSERT_TRUE( IsBuiltInPngCodec( codec ) );

            CORE::CCodecRegistry::TICodecPtrVector providers;
            ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 1 == providers.size() );
            Int32 priority = 0;
            ASSERT_TRUE( codecRegistry.TryGetCodecPriority( "ImageCodec", "png", codec.GetPointerAlways(), priority ) );
            ASSERT_TRUE( CORE::CCodecRegistry::SoftwareCodecPriority == priority );

            // the family registry, which CImage uses, gives it too; the image codec registry does not have it
            CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
            CORE::CCodecRegistry::TICodecPtr fromFamily;
            ASSERT_TRUE( codecRegistry.TryLookup( "ImageCodec", family, false ) && family && family->TryLookup( "png", fromFamily, false ) );
            ASSERT_TRUE( fromFamily == codec );
            ASSERT_FALSE( IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) );

            if ( !codec.IsNULL() )
            {
                ASSERT_TRUE( GUCEF_CODEC_CAPABILITY_ENCODE == codec->GetCapabilities() );

                // decoding is refused
                CORE::CDynamicBuffer input;
                input.Append( "\x89PNG", 4 );
                CORE::CDynamicBufferAccess inputAccess( &input, false );
                CORE::CDynamicBuffer output;
                CORE::CDynamicBufferAccess outputAccess( &output, false );
                ASSERT_FALSE( codec->Decode( inputAccess, outputAccess, CORE::CValueList() ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_UNSUPPORTED == codec->GetLastStatus() );

                // a serialized image that is cut short is refused
                CORE::CDynamicBuffer shortInput;
                shortInput.Append( "abc", 3 );
                CORE::CDynamicBufferAccess shortAccess( &shortInput, false );
                ASSERT_FALSE( codec->Encode( shortAccess, outputAccess, CORE::CValueList() ) );
                ASSERT_TRUE( CORE::GUCEF_CODEC_STATUS_INVALID_INPUT == codec->GetLastStatus() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PNG codec: CImage saves PNG through the built-in codec of the codec registry when no image plugin has png" )
        try
        {
            IMAGE::TPixelMapPtr firstFrame = MakePicture( 7, 5, IMAGE::PSF_RGB, 11 );
            IMAGE::TPixelMapPtr secondFrame = MakePicture( 7, 5, IMAGE::PSF_RGB, 12 );
            IMAGE::CImage image( firstFrame );
            image.AddFrame( secondFrame, 40 );

            CORE::CDynamicBuffer saved;
            CORE::CDynamicBufferAccess savedAccess( &saved, false );
            ASSERT_TRUE( image.Save( savedAccess, "png" ) );

            // the first frame, the same bytes as the static encoder makes
            CORE::CDynamicBuffer expected;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *firstFrame, expected, problem ) );
            ASSERT_TRUE( saved.GetDataSize() == expected.GetDataSize() );
            ASSERT_TRUE( saved.GetDataSize() == expected.GetDataSize() && 0 == memcmp( saved.GetConstBufferPtr(), expected.GetConstBufferPtr(), expected.GetDataSize() ) );

            // the image interface of the codec writes the same
            IMAGE::CPngImageCodec codec;
            CORE::CDynamicBuffer direct;
            CORE::CDynamicBufferAccess directAccess( &direct, false );
            ASSERT_TRUE( codec.Encode( image, directAccess ) );
            ASSERT_TRUE( direct.GetDataSize() == expected.GetDataSize() && 0 == memcmp( direct.GetConstBufferPtr(), expected.GetConstBufferPtr(), expected.GetDataSize() ) );

            // an image without pictures and a format without PNG equivalent are refused
            IMAGE::CImage emptyImage;
            ASSERT_FALSE( codec.Encode( emptyImage, directAccess ) );
            IMAGE::TPixelMapPtr hsv( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2, 2, IMAGE::PSF_HSV, GUCEF::DATATYPE_UINT8 ) );
            IMAGE::CImage hsvImage( hsv );
            CORE::CDynamicBuffer refused;
            CORE::CDynamicBufferAccess refusedAccess( &refused, false );
            ASSERT_FALSE( hsvImage.Save( refusedAccess, "png" ) );

            // loading PNG needs an image plugin, the built-in codec does not decode
            savedAccess.Setpos( 0 );
            IMAGE::CImage loaded;
            ASSERT_FALSE( loaded.Load( savedAccess, "png", true, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestCompressedEncoding( void )
{GUCEF_TRACE;

    const bool zlibLoaded = LoadZlibCodecPlugin();

    GUCEF_TESTFW_TESTCASE( "PNG codec: with the zlib codec of codecspluginZLIB the image data is compressed and inflates to the pixels" )
        GUCEF_TESTFW_SKIP_IF( !zlibLoaded, "codecspluginZLIB is not available" );
        try
        {
            IMAGE::TPixelMapPtr pixels = MakeGradient( 64, 48 );
            CORE::CDynamicBuffer png;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );

            SPngWalk walk;
            ASSERT_TRUE( WalkPng( png, walk ) );
            ASSERT_TRUE( 64 == walk.width && 48 == walk.height && 6 == walk.colourType );
            ASSERT_FALSE( StartsWithStoredBlock( walk.zlibStream ) );
            ASSERT_TRUE( walk.zlibStream.GetDataSize() < ( 64 * 4 + 1 ) * 48 );

            CORE::CDynamicBuffer raw;
            ASSERT_TRUE( InflateWithRegistryCodec( walk.zlibStream, raw ) );
            ASSERT_TRUE( LinesHoldPixels( raw, *pixels, false ) );

            // the stored blocks of the fallback inflate the same way with the zlib codec
            CORE::CDynamicBuffer stored;
            CORE::CDynamicBuffer filtered( raw );
            CORE::CDynamicBuffer storedRaw;
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecPtr failing = CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CFailingZlibCodec() );
            ASSERT_TRUE( codecRegistry.RegisterCodec( failing, CORE::CCodecRegistry::OsCodecPriority + 1000 ) );
            const bool encoded = IMAGE::CPngImageCodec::EncodePixelMap( *pixels, stored, problem );
            ASSERT_TRUE( codecRegistry.UnregisterCodec( failing ) );
            ASSERT_TRUE( encoded );
            SPngWalk storedWalk;
            ASSERT_TRUE( WalkPng( stored, storedWalk ) );
            ASSERT_TRUE( StartsWithStoredBlock( storedWalk.zlibStream ) );
            ASSERT_TRUE( InflateWithRegistryCodec( storedWalk.zlibStream, storedRaw ) );
            ASSERT_TRUE( storedRaw.GetDataSize() == filtered.GetDataSize() && 0 == memcmp( storedRaw.GetConstBufferPtr(), filtered.GetConstBufferPtr(), filtered.GetDataSize() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( zlibLoaded )
    {
        GUCEF_TESTFW_TESTCASE( "PNG codec: without the zlib codec the image data is stored again" )
            try
            {
                ASSERT_TRUE( UnloadTestPlugins() );
                ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" ).IsNULL() );

                IMAGE::TPixelMapPtr pixels = MakeGradient( 16, 16 );
                CORE::CDynamicBuffer png;
                CORE::CString problem;
                ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );
                SPngWalk walk;
                ASSERT_TRUE( WalkPng( png, walk ) );
                ASSERT_TRUE( StartsWithStoredBlock( walk.zlibStream ) );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }
}

/*-------------------------------------------------------------------------*/

static void
TestImagePluginTakesPrecedence( void )
{GUCEF_TRACE;

    const bool pluginLoaded = LoadFreeImageCodecPlugin();

    GUCEF_TESTFW_TESTCASE( "PNG codec: an image plugin with png takes the place of the built-in codec while it is loaded" )
        GUCEF_TESTFW_SKIP_IF( !pluginLoaded, "imgpluginFreeImage is not available" );
        try
        {
            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

            // the plugin's provider first, the built-in one after it
            CORE::CCodecRegistry::TICodecPtrVector providers;
            ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 2 == providers.size() );
            if ( 2 == providers.size() )
            {
                ASSERT_FALSE( IsBuiltInPngCodec( providers[ 0 ] ) );
                ASSERT_TRUE( IsBuiltInPngCodec( providers[ 1 ] ) );
            }
            providers.clear();  // a codec that is held keeps its plugin from unloading
            ASSERT_FALSE( IsBuiltInPngCodec( codecRegistry.TryGetCodec( "ImageCodec", "png" ) ) );
            ASSERT_TRUE( IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) );

            // FreeImage reads the output of the built-in encoder
            IMAGE::TPixelMapPtr pixels = MakePicture( 9, 4, IMAGE::PSF_RGBA, 21 );
            CORE::CDynamicBuffer png;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *pixels, png, problem ) );
            CORE::CDynamicBufferAccess pngAccess( &png, false );
            IMAGE::CImage decoded;
            ASSERT_TRUE( decoded.Load( pngAccess, "png", true, false ) );
            IMAGE::TPixelMapPtr decodedPixels;
            ASSERT_TRUE( decoded.TryGetPixelMap( 0, 0, decodedPixels ) && !decodedPixels.IsNULL() );
            if ( !decodedPixels.IsNULL() )
                ASSERT_TRUE( 9 == decodedPixels->GetWidthInPixels() && 4 == decodedPixels->GetHeightInPixels() );

            // CImage saves through the plugin now
            IMAGE::CImage image( pixels );
            CORE::CDynamicBuffer saved;
            CORE::CDynamicBufferAccess savedAccess( &saved, false );
            ASSERT_TRUE( image.Save( savedAccess, "png" ) );
            ASSERT_FALSE( saved.GetDataSize() == png.GetDataSize() && 0 == memcmp( saved.GetConstBufferPtr(), png.GetConstBufferPtr(), png.GetDataSize() ) );

            // after the plugin is unloaded the built-in codec is the provider again
            ASSERT_TRUE( UnloadTestPlugins() );
            ASSERT_TRUE( codecRegistry.GetCodecs( "ImageCodec", "png", providers ) && 1 == providers.size() );
            if ( 1 == providers.size() )
                ASSERT_TRUE( IsBuiltInPngCodec( providers[ 0 ] ) );
            providers.clear();
            ASSERT_TRUE( IsBuiltInPngCodec( codecRegistry.TryGetCodec( "ImageCodec", "png" ) ) );
            ASSERT_FALSE( IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) );

            CORE::CDynamicBuffer savedAgain;
            CORE::CDynamicBufferAccess savedAgainAccess( &savedAgain, false );
            ASSERT_TRUE( image.Save( savedAgainAccess, "png" ) );
            ASSERT_TRUE( savedAgain.GetDataSize() == png.GetDataSize() && 0 == memcmp( savedAgain.GetConstBufferPtr(), png.GetConstBufferPtr(), png.GetDataSize() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( pluginLoaded )
        UnloadTestPlugins();
}

/*-------------------------------------------------------------------------*/

void
PerformPngImageCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.PngImageCodec" );

    TestCrc32();
    TestStoredEncoding();
    TestLargePicture();
    TestRefusedFormats();
    TestWriteToFile();
    TestFailingCompressionCodec();
    TestBuiltInCodecInTheRegistry();
    TestCompressedEncoding();
    TestImagePluginTakesPrecedence();
}

/*-------------------------------------------------------------------------*/
