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

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_IMAGE_CGUPIMGIMAGECODEC_H
#include "gucefIMAGE_CGupImgImageCodec.h"
#define GUCEF_IMAGE_CGUPIMGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CGUPIMGIMAGECODEC_H ? */

#include "TestGupImgImageCodec.h"

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
const char* GupImgTestPluginGroup = "gupimgimagecodectests";

/** The size of the header of the file format */
const UInt32 FileHeaderSize = 32;

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakePixels( UInt32 width, UInt32 height, IMAGE::TPixelStorageFormat format, TBuildinDataType type, UInt32 seed )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, format, type ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    const UInt32 size = pixels->GetTotalSizeInBytes();
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( i * 13 + seed * 29 + ( i / 97 ) );
    return pixels;
}

/*-------------------------------------------------------------------------*/

/** A picture that compresses very well */
IMAGE::TPixelMapPtr
MakeFlat( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    memset( pixels->GetDataPtr(), 0x40, pixels->GetTotalSizeInBytes() );
    return pixels;
}

/*-------------------------------------------------------------------------*/

bool
SamePixels( const IMAGE::TPixelMapPtr& left, const IMAGE::TPixelMapPtr& right )
{
    if ( left.IsNULL() || right.IsNULL() )
        return false;
    return left->GetWidthInPixels() == right->GetWidthInPixels()                   &&
           left->GetHeightInPixels() == right->GetHeightInPixels()                 &&
           left->GetPixelStorageFormat() == right->GetPixelStorageFormat()         &&
           left->GetPixelComponentDataType() == right->GetPixelComponentDataType() &&
           left->IsNormalizedAsPercentage() == right->IsNormalizedAsPercentage()   &&
           left->GetTotalSizeInBytes() == right->GetTotalSizeInBytes()             &&
           0 == memcmp( left->GetDataPtr(), right->GetDataPtr(), left->GetTotalSizeInBytes() );
}

/*-------------------------------------------------------------------------*/

bool
SameImages( const IMAGE::CImage& left, const IMAGE::CImage& right )
{
    if ( left.GetFrameCount() != right.GetFrameCount() || left.GetLoopCount() != right.GetLoopCount() )
        return false;
    for ( UInt32 f=0; f<left.GetFrameCount(); ++f )
    {
        if ( left.GetMipmapLevels( f ) != right.GetMipmapLevels( f ) || left.GetFrameDurationInMs( f ) != right.GetFrameDurationInMs( f ) )
            return false;
        for ( UInt32 l=0; l<left.GetMipmapLevels( f ); ++l )
            if ( !SamePixels( left.GetPixelMap( f, l ), right.GetPixelMap( f, l ) ) )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Write( const IMAGE::CImage& image, CORE::CDynamicBuffer& file, const CORE::CValueList& params = CORE::CValueList() )
{
    file.Clear();
    CORE::CDynamicBufferAccess access( &file, false );
    IMAGE::CGupImgImageCodec codec;
    return codec.Encode( image, access, params );
}

/*-------------------------------------------------------------------------*/

bool
Read( const CORE::CDynamicBuffer& file, IMAGE::CImage& image )
{
    CORE::CDynamicBuffer copy;
    copy.CopyFrom( file.GetDataSize(), file.GetConstBufferPtr() );
    CORE::CDynamicBufferAccess access( &copy, false );
    access.Setpos( 0 );
    IMAGE::CGupImgImageCodec codec;
    return codec.Decode( access, image );
}

/*-------------------------------------------------------------------------*/

UInt32
GetStoredU32( const CORE::CDynamicBuffer& file, UInt32 offset )
{
    const UInt8* p = static_cast< const UInt8* >( file.GetConstBufferPtr() ) + offset;
    return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

void
SetStoredU32( CORE::CDynamicBuffer& file, UInt32 offset, UInt32 value )
{
    UInt8* p = static_cast< UInt8* >( file.GetBufferPtr() ) + offset;
    p[ 0 ] = (UInt8) value;
    p[ 1 ] = (UInt8) ( value >> 8 );
    p[ 2 ] = (UInt8) ( value >> 16 );
    p[ 3 ] = (UInt8) ( value >> 24 );
}

/*-------------------------------------------------------------------------*/

/** A compression codec of the tests: run length coding in (count, byte) pairs. It shows the format works with any codec of the registry */
class CRunLengthCodec : public CORE::CICodec
{
    public:

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer bytes;
        if ( !ReadAll( source, bytes ) )
            return false;
        const UInt8* data = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
        const UInt32 size = bytes.GetDataSize();
        CORE::CDynamicBuffer packed;
        for ( UInt32 i=0; i<size; )
        {
            UInt32 run = 1;
            while ( i + run < size && run < 255 && data[ i + run ] == data[ i ] )
                ++run;
            const UInt8 pair[ 2 ] = { (UInt8) run, data[ i ] };
            packed.Append( pair, 2 );
            i += run;
        }
        return packed.GetDataSize() == 0 || 1 == dest.Write( packed.GetConstBufferPtr(), packed.GetDataSize(), 1 );
    }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDynamicBuffer bytes;
        if ( !ReadAll( source, bytes ) || 0 != bytes.GetDataSize() % 2 )
            return false;
        const UInt8* data = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
        for ( UInt32 i=0; i<bytes.GetDataSize(); i+=2 )
        {
            for ( UInt8 n=0; n<data[ i ]; ++n )
                if ( 1 != dest.Write( &data[ i + 1 ], 1, 1 ) )
                    return false;
        }
        return true;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "gupimgtestrle"; }

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CoreCodecTypes::CompressionCodec; }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CRunLengthCodec(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CRunLengthCodec";
        return classTypeName;
    }

    private:

    static bool ReadAll( CORE::CIOAccess& source, CORE::CDynamicBuffer& bytes )
    {
        const UInt32 size = (UInt32) source.GetSize();
        bytes.SetDataSize( size );
        return size == 0 || 1 == source.Read( bytes.GetBufferPtr(), size, 1 );
    }
};

/*-------------------------------------------------------------------------*/

bool
IsZlibThere( void )
{
    return !CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "zlib" ).IsNULL();
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
        if ( pluginControl.AddPluginFromDir( firstPluginPath, GupImgTestPluginGroup, true ) ||
             pluginControl.AddPluginFromDir( secondPluginPath, GupImgTestPluginGroup, true ) )
        {
            return IsZlibThere();
        }
    }
    catch ( ... )
    {
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
UnloadTestPlugins( void )
{
    try
    {
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( GupImgTestPluginGroup );
    }
    catch ( ... )
    {
    }
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestRegistration( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "gupimg codec: the native format is registered as gupimg in the image codec registry and says what it holds" )
        try
        {
            IMAGE::CImageCodecRegistry::TImageCodecPtr codec;
            ASSERT_TRUE( IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( "gupimg", codec, false ) );
            ASSERT_TRUE( !codec.IsNULL() && "gupimg" == codec->GetType() && "ImageCodec" == codec->GetFamilyName() );
            if ( !codec.IsNULL() )
            {
                ASSERT_TRUE( ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE ) == codec->GetCapabilities() );
                ASSERT_TRUE( ( IMAGE::CIImageCodec::IMAGECAPABILITY_MULTIPLE_FRAMES | IMAGE::CIImageCodec::IMAGECAPABILITY_MIP_LEVELS ) == codec->GetImageCapabilities() );
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
TestRoundTrips( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "gupimg codec: every pixel format and component type comes back exactly, with frames, durations, loop count and mipmap levels" )
        try
        {
            struct SCase { IMAGE::TPixelStorageFormat format; TBuildinDataType type; };
            const SCase cases[] =
            {
                { IMAGE::PSF_RGBA, DATATYPE_UINT8 }, { IMAGE::PSF_RGB, DATATYPE_UINT8 }, { IMAGE::PSF_BGRA, DATATYPE_UINT8 },
                { IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, DATATYPE_UINT8 }, { IMAGE::PSF_LUMINANCE_ALPHA, DATATYPE_UINT8 },
                { IMAGE::PSF_RGBA, DATATYPE_LE_UINT16 }, { IMAGE::PSF_RGB, DATATYPE_LE_INT32 }, { IMAGE::PSF_RGBA, DATATYPE_LE_FLOAT32 },
                { IMAGE::PSF_SINGLE_CHANNEL_RED, DATATYPE_LE_FLOAT64 }, { IMAGE::PSF_HSV, DATATYPE_UINT8 }, { IMAGE::PSF_PALETTE_INDICES, DATATYPE_UINT8 }
            };
            for ( size_t i=0; i<sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
            {
                IMAGE::CImage::TMipMapList levels;
                levels.push_back( MakePixels( 9, 7, cases[ i ].format, cases[ i ].type, (UInt32) i ) );
                levels.push_back( MakePixels( 4, 3, cases[ i ].format, cases[ i ].type, (UInt32) i + 50 ) );
                IMAGE::CImage image( levels );
                IMAGE::CImage::TMipMapList second;
                second.push_back( MakePixels( 9, 7, cases[ i ].format, cases[ i ].type, (UInt32) i + 100 ) );
                image.AddFrame( second, 120 );
                image.SetFrameDurationInMs( 0, 80 );
                image.SetLoopCount( 3 );

                CORE::CDynamicBuffer file;
                ASSERT_TRUE( Write( image, file ) );
                IMAGE::CImage loaded;
                ASSERT_TRUE( Read( file, loaded ) );
                ASSERT_TRUE( SameImages( image, loaded ) );
            }

            // the normalized flag of float pictures
            IMAGE::TPixelMapPtr normalized = MakePixels( 3, 3, IMAGE::PSF_RGBA, DATATYPE_LE_FLOAT32, 5 );
            ASSERT_TRUE( normalized->SetIsNormalizedAsPercentage( true ) );
            IMAGE::CImage normalizedImage( normalized );
            CORE::CDynamicBuffer file;
            IMAGE::CImage loaded;
            ASSERT_TRUE( Write( normalizedImage, file ) && Read( file, loaded ) );
            ASSERT_TRUE( loaded.GetPixelMap( 0, 0 )->IsNormalizedAsPercentage() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: CImage saves and loads gupimg through the image codec registry, with the meta-data" )
        try
        {
            IMAGE::CImage image( MakePixels( 6, 5, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 3 ) );
            image.GetMetaData().Set( "upscaler", CORE::CVariant( CORE::CString( "realesrgan" ) ) );
            image.GetMetaData().Set( "factor", CORE::CVariant( CORE::CString( "2" ) ) );

            CORE::CDynamicBuffer file;
            CORE::CDynamicBufferAccess access( &file, false );
            ASSERT_TRUE( image.Save( access, "gupimg" ) );

            access.Setpos( 0 );
            IMAGE::CImage loaded;
            ASSERT_TRUE( loaded.Load( access, "gupimg", true, true ) );
            ASSERT_TRUE( SameImages( image, loaded ) );
            ASSERT_TRUE( "realesrgan" == loaded.GetMetaData().GetValueAlways( "upscaler" ).AsString() );
            ASSERT_TRUE( "2" == loaded.GetMetaData().GetValueAlways( "factor" ).AsString() );

            // the meta-data alone, without reading the pixels
            access.Setpos( 0 );
            IMAGE::CImage metaOnly;
            ASSERT_TRUE( metaOnly.Load( access, "gupimg", false, true ) );
            ASSERT_FALSE( metaOnly.HasFrames() );
            ASSERT_TRUE( "realesrgan" == metaOnly.GetMetaData().GetValueAlways( "upscaler" ).AsString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: an image without frames and pictures that cannot be written are refused" )
        try
        {
            IMAGE::CImage empty;
            CORE::CDynamicBuffer file;
            ASSERT_FALSE( Write( empty, file ) );

            IMAGE::TPixelMapPtr noPicture;
            IMAGE::CImage::TMipMapList levels;
            levels.push_back( noPicture );
            IMAGE::CImage broken( levels );
            ASSERT_FALSE( Write( broken, file ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestStoredAndCompressed( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "gupimg codec: without a zlib codec the levels are stored as they are, and asking for zlib fails" )
        try
        {
            ASSERT_FALSE( IsZlibThere() );
            IMAGE::CImage image( MakeFlat( 16, 16 ) );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Write( image, file ) );
            IMAGE::CImage loaded;
            ASSERT_TRUE( Read( file, loaded ) && SameImages( image, loaded ) );

            CORE::CValueList params;
            params.Set( "compression", CORE::CVariant( CORE::CString( "zlib" ) ) );
            ASSERT_FALSE( Write( image, file, params ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    const bool pluginLoaded = LoadZlibCodecPlugin();
    GUCEF_TESTFW_TESTCASE( "gupimg codec: the zlib codec plugin is available for the compression tests" )
        ASSERT_TRUE( pluginLoaded );
    GUCEF_TESTFW_TESTCASE_END
    if ( !pluginLoaded )
        return;

    GUCEF_TESTFW_TESTCASE( "gupimg codec: levels are compressed with zlib by default, a level that does not shrink is stored, none stores all" )
        try
        {
            IMAGE::CImage flat( MakeFlat( 64, 64 ) );
            CORE::CDynamicBuffer compressed;
            ASSERT_TRUE( Write( flat, compressed ) );
            ASSERT_TRUE( compressed.GetDataSize() < 64 * 64 * 4 / 4 );
            IMAGE::CImage loaded;
            ASSERT_TRUE( Read( compressed, loaded ) && SameImages( flat, loaded ) );

            CORE::CValueList none;
            none.Set( "compression", CORE::CVariant( CORE::CString( "none" ) ) );
            CORE::CDynamicBuffer raw;
            ASSERT_TRUE( Write( flat, raw, none ) );
            ASSERT_TRUE( raw.GetDataSize() > 64 * 64 * 4 );
            IMAGE::CImage loadedRaw;
            ASSERT_TRUE( Read( raw, loadedRaw ) && SameImages( flat, loadedRaw ) );

            // noise does not compress: the level is stored and the file does not grow past the raw size plus tables
            IMAGE::CImage noise( MakePixels( 31, 33, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 9 ) );
            UInt8* data = static_cast< UInt8* >( noise.GetPixelMap( 0, 0 )->GetDataPtr() );
            UInt32 state = 12345;
            for ( UInt32 i=0; i<31 * 33 * 4; ++i )
            {
                state = state * 1664525u + 1013904223u;
                data[ i ] = (UInt8) ( state >> 24 );
            }
            CORE::CDynamicBuffer noisy;
            ASSERT_TRUE( Write( noise, noisy ) );
            ASSERT_TRUE( noisy.GetDataSize() < 31 * 33 * 4 + 300 );
            IMAGE::CImage loadedNoise;
            ASSERT_TRUE( Read( noisy, loadedNoise ) && SameImages( noise, loadedNoise ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: any compression codec of the registry can be named, and a file that names one that is gone is refused with its name" )
        try
        {
            CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TICodecTypedPtr typedRunLength( GUCEF_NEW CRunLengthCodec() );
            CORE::CCodecRegistry::TICodecPtr runLength( typedRunLength );
            ASSERT_TRUE( registry.RegisterCodec( runLength ) );

            IMAGE::CImage image( MakeFlat( 16, 16 ) );
            CORE::CValueList params;
            params.Set( "compression", CORE::CVariant( CORE::CString( "gupimgtestrle" ) ) );
            CORE::CDynamicBuffer file;
            ASSERT_TRUE( Write( image, file, params ) );

            // the level is stored as the codec of the test made it: 1024 bytes of one value are four runs of 255 and one of 4
            ASSERT_TRUE( file.GetDataSize() < 200 );
            if ( file.GetDataSize() >= 2 )
            {
                const UInt8* fileBytes = static_cast< const UInt8* >( file.GetConstBufferPtr() );
                ASSERT_TRUE( 4 == fileBytes[ file.GetDataSize() - 2 ] && 0x40 == fileBytes[ file.GetDataSize() - 1 ] );
            }

            IMAGE::CImage loaded;
            ASSERT_TRUE( Read( file, loaded ) && SameImages( image, loaded ) );

            ASSERT_TRUE( registry.UnregisterCodec( runLength ) );
            IMAGE::CImage refused;
            ASSERT_FALSE( Read( file, refused ) );
            ASSERT_FALSE( refused.HasFrames() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    UnloadTestPlugins();
}

/*-------------------------------------------------------------------------*/

static void
TestDamagedFiles( void )
{GUCEF_TRACE;

    IMAGE::CImage image( MakePixels( 7, 6, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 8 ) );
    image.GetMetaData().Set( "key", CORE::CVariant( CORE::CString( "value" ) ) );
    CORE::CValueList none;
    none.Set( "compression", CORE::CVariant( CORE::CString( "none" ) ) );
    CORE::CDynamicBuffer good;
    const bool written = Write( image, good, none );

    GUCEF_TESTFW_TESTCASE( "gupimg codec: a file that is cut short anywhere is refused and leaves the image untouched" )
        try
        {
            ASSERT_TRUE( written );
            IMAGE::CImage loaded;
            ASSERT_TRUE( Read( good, loaded ) && SameImages( image, loaded ) );

            for ( UInt32 length=0; length<good.GetDataSize(); ++length )
            {
                CORE::CDynamicBuffer cut;
                cut.CopyFrom( length, good.GetConstBufferPtr() );
                IMAGE::CImage target( MakePixels( 2, 2, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 1 ) );
                ASSERT_FALSE( Read( cut, target ) );
                ASSERT_TRUE( 1 == target.GetFrameCount() && 2 == target.GetPixelMap( 0, 0 )->GetWidthInPixels() );

                CORE::CDynamicBufferAccess access( &cut, false );
                access.Setpos( 0 );
                CORE::CValueList metaData;
                IMAGE::CGupImgImageCodec codec;
                const UInt32 metaSectionEnd = FileHeaderSize + GetStoredU32( good, 24 ) + GetStoredU32( good, 28 );
                ASSERT_TRUE( codec.DecodeMetaData( access, metaData ) == ( length >= metaSectionEnd ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: a wrong signature, a newer version, unknown flags and impossible counts are refused" )
        try
        {
            ASSERT_TRUE( written );
            IMAGE::CImage target;

            CORE::CDynamicBuffer damaged;
            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            static_cast< UInt8* >( damaged.GetBufferPtr() )[ 0 ] = 'X';
            ASSERT_FALSE( Read( damaged, target ) );

            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            static_cast< UInt8* >( damaged.GetBufferPtr() )[ 8 ] = 2;                       // version 2
            ASSERT_FALSE( Read( damaged, target ) );

            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( damaged, 12, GetStoredU32( good, 12 ) | 0x80 );                   // a flag of the future
            ASSERT_FALSE( Read( damaged, target ) );

            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( damaged, 16, 0 );                                                 // no frames
            ASSERT_FALSE( Read( damaged, target ) );
            SetStoredU32( damaged, 16, 0x7FFFFFFF );                                        // far too many frames
            ASSERT_FALSE( Read( damaged, target ) );

            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( damaged, 24, 0xFFFFFFF0u );                                       // a table section that is larger than the file
            ASSERT_FALSE( Read( damaged, target ) );

            // the first level: 8 bytes of frame table follow the header, then width, height
            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( damaged, FileHeaderSize + 8, 0x40000000u );                       // a width of a billion pixels: not an allocation
            ASSERT_FALSE( Read( damaged, target ) );
            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( damaged, FileHeaderSize + 8, 0 );                                 // no width
            ASSERT_FALSE( Read( damaged, target ) );
            ASSERT_FALSE( target.HasFrames() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: a changed byte in the pixels is found by the checksum of its level" )
        try
        {
            ASSERT_TRUE( written );
            CORE::CDynamicBuffer damaged;
            damaged.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            UInt8* bytes = static_cast< UInt8* >( damaged.GetBufferPtr() );
            bytes[ damaged.GetDataSize() - 3 ] ^= 0x10;
            IMAGE::CImage target;
            ASSERT_FALSE( Read( damaged, target ) );

            // without checksums the same damage goes unnoticed: that is what they are for
            CORE::CValueList params;
            params.Set( "compression", CORE::CVariant( CORE::CString( "none" ) ) );
            params.Set( "checksums", CORE::CVariant( false ) );
            CORE::CDynamicBuffer unchecked;
            ASSERT_TRUE( Write( image, unchecked, params ) );
            UInt8* uncheckedBytes = static_cast< UInt8* >( unchecked.GetBufferPtr() );
            uncheckedBytes[ unchecked.GetDataSize() - 3 ] ^= 0x10;
            IMAGE::CImage targetWithoutCheck;
            ASSERT_TRUE( Read( unchecked, targetWithoutCheck ) );
            ASSERT_FALSE( SameImages( image, targetWithoutCheck ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "gupimg codec: rows that run bottom to top are turned over by the reader" )
        try
        {
            ASSERT_TRUE( written );
            CORE::CDynamicBuffer flipped;
            flipped.CopyFrom( good.GetDataSize(), good.GetConstBufferPtr() );
            SetStoredU32( flipped, 12, GetStoredU32( good, 12 ) | 0x02 );
            IMAGE::CImage loaded;
            ASSERT_TRUE( Read( flipped, loaded ) );

            // the rows of the picture in the opposite order, made here row by row
            IMAGE::TPixelMapPtr original = image.GetPixelMap( 0, 0 );
            IMAGE::TPixelMapPtr expected( GUCEF_NEW IMAGE::CPixelMap( *original ) );
            const UInt32 rowSize = original->GetWidthInBytes();
            for ( UInt32 row=0; row<original->GetHeightInPixels(); ++row )
                memcpy( expected->GetDataAtScanLine( original->GetHeightInPixels() - 1 - row ), original->GetDataAtScanLine( row ), rowSize );
            ASSERT_TRUE( !SamePixels( expected, original ) );
            ASSERT_TRUE( SamePixels( expected, loaded.GetPixelMap( 0, 0 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformGupImgImageCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.GupImgImageCodec" );

    TestRegistration();
    TestRoundTrips();
    TestStoredAndCompressed();
    TestDamagedFiles();
}

/*-------------------------------------------------------------------------*/
