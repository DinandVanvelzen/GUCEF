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

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_IMAGE_CIMAGECONVERTER_H
#include "gucefIMAGE_CImageConverter.h"
#define GUCEF_IMAGE_CIMAGECONVERTER_H
#endif /* GUCEF_IMAGE_CIMAGECONVERTER_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#include "gucefIMAGE_CImageCodecCapabilities.h"
#define GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#endif /* GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H ? */

#include "TestImageConverter.h"

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

/**
 *  A format of the tests that holds one RGB 8 bit picture, as "RGB1", width, height and the pixels.
 *  With 'canWrite' false it can only be read, the way some image plugin formats are
 */
class CPlainRgbCodec : public IMAGE::CIImageCodec
{
    public:

    explicit CPlainRgbCodec( const CORE::CString& type, bool canWrite ) : m_type( type ), m_canWrite( canWrite ) {}

    virtual bool Encode( const IMAGE::CImage& inputImage, CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_canWrite || 1 != inputImage.GetFrameCount() )
            return false;
        IMAGE::TPixelMapPtr pixels = inputImage.GetPixelMap( 0, 0 );
        if ( pixels.IsNULL() || IMAGE::PSF_RGB != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
            return false;
        const UInt32 header[ 3 ] = { 0x31424752, pixels->GetWidthInPixels(), pixels->GetHeightInPixels() };
        return 1 == encodedOutput.Write( header, sizeof( header ), 1 ) && 1 == encodedOutput.Write( pixels->GetDataPtr(), pixels->GetTotalSizeInBytes(), 1 );
    }

    virtual bool Decode( CORE::CIOAccess& encodedInput, IMAGE::CImage& outputImage ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 header[ 3 ];
        if ( 1 != encodedInput.Read( header, sizeof( header ), 1 ) || 0x31424752 != header[ 0 ] || 0 == header[ 1 ] || 0 == header[ 2 ] )
            return false;
        IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, header[ 1 ], header[ 2 ], IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
        if ( 1 != encodedInput.Read( pixels->GetDataPtr(), pixels->GetTotalSizeInBytes(), 1 ) )
            return false;
        outputImage.Assign( pixels );
        return true;
    }

    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_canWrite ? ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE ) : GUCEF_CODEC_CAPABILITY_DECODE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_type; }

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ImageCodec"; }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CPlainRgbCodec( m_type, m_canWrite ); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CPlainRgbCodec";
        return classTypeName;
    }

    private:

    CORE::CString m_type;
    bool m_canWrite;
};

/*-------------------------------------------------------------------------*/

/** Registers the test formats for the length of a scope: "tstrgb" to read and write, "tstrgbro" to read only */
class CTestFormats
{
    public:

    CTestFormats( void )
    {
        IMAGE::CImageCodecRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
        CORE::CTSharedPtr< CPlainRgbCodec, MT::CMutex > writable( GUCEF_NEW CPlainRgbCodec( "tstrgb", true ) );
        CORE::CTSharedPtr< CPlainRgbCodec, MT::CMutex > readOnly( GUCEF_NEW CPlainRgbCodec( "tstrgbro", false ) );
        registry.Register( "tstrgb", writable );
        registry.Register( "tstrgbro", readOnly );
    }

    ~CTestFormats()
    {
        IMAGE::CImageCodecRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
        registry.Unregister( "tstrgb" );
        registry.Unregister( "tstrgbro" );
    }
};

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakePixels( UInt32 width, UInt32 height, IMAGE::TPixelStorageFormat format, TBuildinDataType type, UInt32 seed )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, format, type ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    const UInt32 size = pixels->GetTotalSizeInBytes();
    for ( UInt32 i=0; i<size; ++i )
        data[ i ] = (UInt8) ( i * 11 + seed * 31 + ( i / 89 ) );
    return pixels;
}

/*-------------------------------------------------------------------------*/

/** A native format file of an animation of three frames */
void
MakeAnimationFile( IMAGE::TPixelStorageFormat format, TBuildinDataType type, CORE::CDynamicBuffer& file )
{
    IMAGE::CImage image( MakePixels( 6, 4, format, type, 1 ) );
    IMAGE::TPixelMapPtr second = MakePixels( 6, 4, format, type, 2 );
    IMAGE::TPixelMapPtr third = MakePixels( 6, 4, format, type, 3 );
    image.AddFrame( second, 70 );
    image.AddFrame( third, 90 );
    image.SetFrameDurationInMs( 0, 50 );
    image.SetLoopCount( 2 );
    file.Clear();
    CORE::CDynamicBufferAccess access( &file, false );
    image.Save( access, "gupimg" );
}

/*-------------------------------------------------------------------------*/

bool
LoadAs( const CORE::CDynamicBuffer& file, const CORE::CString& type, IMAGE::CImage& image )
{
    CORE::CDynamicBuffer copy;
    copy.CopyFrom( file.GetDataSize(), file.GetConstBufferPtr() );
    CORE::CDynamicBufferAccess access( &copy, false );
    access.Setpos( 0 );
    return image.Load( access, type, true, false );
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString::StringVector& notes, const char* text )
{
    for ( size_t i=0; i<notes.size(); ++i )
        if ( notes[ i ].HasSubstr( text, false ) >= 0 )
            return true;
    return false;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestCapabilities( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "image converter: the capabilities list what the loaded codecs read, write and hold" )
        try
        {
            CTestFormats formats;
            IMAGE::CImageCodecCapabilities::SImageCodecInfo info;

            ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "GUPIMG", info ) );
            ASSERT_TRUE( "gupimg" == info.type && info.canDecode && info.canEncode && info.multipleFrames && info.mipLevels );

            // the built-in PNG writer is a codec of the basic interface: it writes, does not read, one picture
            ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "png", info ) );
            ASSERT_TRUE( info.canEncode && !info.multipleFrames && !info.mipLevels );
            if ( !IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) )
                ASSERT_FALSE( info.canDecode );

            ASSERT_TRUE( IMAGE::CImageCodecCapabilities::TryGetCodec( "tstrgbro", info ) );
            ASSERT_TRUE( info.canDecode && !info.canEncode );

            ASSERT_FALSE( IMAGE::CImageCodecCapabilities::TryGetCodec( "nosuchformat", info ) );

            IMAGE::CImageCodecCapabilities::TImageCodecInfoVector codecs;
            IMAGE::CImageCodecCapabilities::GetCodecs( codecs );
            bool hasGupImg = false, hasTest = false, ordered = true;
            for ( size_t i=0; i<codecs.size(); ++i )
            {
                hasGupImg = hasGupImg || "gupimg" == codecs[ i ].type;
                hasTest = hasTest || "tstrgb" == codecs[ i ].type;
                ordered = ordered && ( 0 == i || codecs[ i - 1 ].type < codecs[ i ].type );
            }
            ASSERT_TRUE( hasGupImg && hasTest && ordered );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestNativeConversions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "image converter: native to native keeps all frames, durations and the loop count" )
        try
        {
            CORE::CDynamicBuffer source;
            MakeAnimationFile( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, source );
            ASSERT_TRUE( source.GetDataSize() > 0 );

            CORE::CDynamicBuffer converted;
            CORE::CDynamicBufferAccess in( &source, false );
            CORE::CDynamicBufferAccess out( &converted, false );
            in.Setpos( 0 );
            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionResult result;
            ASSERT_TRUE( converter.Convert( in, "gupimg", out, "gupimg", IMAGE::CImageConversionSettings(), result ) );
            ASSERT_TRUE( result.IsSuccess() && 3 == result.GetSourceFrameCount() && 3 == result.GetWrittenFrameCount() && result.GetNotes().empty() );

            IMAGE::CImage original, loaded;
            ASSERT_TRUE( LoadAs( source, "gupimg", original ) && LoadAs( converted, "gupimg", loaded ) );
            ASSERT_TRUE( 3 == loaded.GetFrameCount() && 2 == loaded.GetLoopCount() );
            ASSERT_TRUE( 50 == loaded.GetFrameDurationInMs( 0 ) && 70 == loaded.GetFrameDurationInMs( 1 ) && 90 == loaded.GetFrameDurationInMs( 2 ) );
            for ( UInt32 f=0; f<3; ++f )
                ASSERT_TRUE( 0 == memcmp( original.GetPixelMap( f, 0 )->GetDataPtr(), loaded.GetPixelMap( f, 0 )->GetDataPtr(), original.GetPixelMap( f, 0 )->GetTotalSizeInBytes() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "image converter: encoder parameters reach the writer and one frame can be picked" )
        try
        {
            CORE::CDynamicBuffer source;
            MakeAnimationFile( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, source );

            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;
            settings.SetFrameIndex( 1 );
            settings.GetEncoderParams().Set( "compression", CORE::CVariant( CORE::CString( "none" ) ) );
            CORE::CDynamicBuffer converted;
            CORE::CDynamicBufferAccess in( &source, false );
            CORE::CDynamicBufferAccess out( &converted, false );
            in.Setpos( 0 );
            IMAGE::CImageConversionResult result;
            ASSERT_TRUE( converter.Convert( in, "gupimg", out, "gupimg", settings, result ) );
            ASSERT_TRUE( 3 == result.GetSourceFrameCount() && 1 == result.GetWrittenFrameCount() );

            IMAGE::CImage loaded;
            ASSERT_TRUE( LoadAs( converted, "gupimg", loaded ) && 1 == loaded.GetFrameCount() && 70 == loaded.GetFrameDurationInMs( 0 ) );

            // a frame that is not there
            settings.SetFrameIndex( 3 );
            CORE::CDynamicBuffer none;
            CORE::CDynamicBufferAccess noneOut( &none, false );
            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "gupimg", noneOut, "gupimg", settings, result ) );
            ASSERT_TRUE( !result.IsSuccess() && !result.GetProblem().IsNULLOrEmpty() && 0 == none.GetDataSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFormatNegotiation( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "image converter: a picture the target cannot hold as it is becomes RGB 8 bit, and the alpha that is dropped is reported as a loss" )
        try
        {
            CTestFormats formats;
            IMAGE::CImage image( MakePixels( 5, 4, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, 7 ) );
            CORE::CDynamicBuffer source;
            CORE::CDynamicBufferAccess sourceAccess( &source, false );
            ASSERT_TRUE( image.Save( sourceAccess, "gupimg" ) );

            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;
            CORE::CDynamicBuffer converted;
            CORE::CDynamicBufferAccess out( &converted, false );
            sourceAccess.Setpos( 0 );
            IMAGE::CImageConversionResult result;
            ASSERT_TRUE( converter.Convert( sourceAccess, "gupimg", out, "tstrgb", settings, result ) );
            ASSERT_TRUE( result.HasLoss() && 1 == result.GetNotes().size() && Contains( result.GetNotes(), "alpha" ) );

            IMAGE::CImage loaded;
            ASSERT_TRUE( LoadAs( converted, "tstrgb", loaded ) && IMAGE::PSF_RGB == loaded.GetPixelMap( 0, 0 )->GetPixelStorageFormat() );
            const UInt8* rgb = static_cast< const UInt8* >( loaded.GetPixelMap( 0, 0 )->GetDataPtr() );
            const UInt8* rgba = static_cast< const UInt8* >( image.GetPixelMap( 0, 0 )->GetDataPtr() );
            ASSERT_TRUE( rgb[ 0 ] == rgba[ 0 ] && rgb[ 1 ] == rgba[ 1 ] && rgb[ 2 ] == rgba[ 2 ] && rgb[ 3 ] == rgba[ 4 ] );

            // with losses forbidden nothing is written
            settings.SetAllowLossyConversion( false );
            CORE::CDynamicBuffer refused;
            CORE::CDynamicBufferAccess refusedOut( &refused, false );
            sourceAccess.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( sourceAccess, "gupimg", refusedOut, "tstrgb", settings, result ) );
            ASSERT_TRUE( !result.IsSuccess() && 0 == refused.GetDataSize() && result.GetProblem().HasSubstr( "losing", false ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "image converter: a picture without alpha goes to RGB 8 bit without a loss, wider channels are reported as a loss" )
        try
        {
            CTestFormats formats;
            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;

            IMAGE::CImage bgr( MakePixels( 3, 3, IMAGE::PSF_BGR, GUCEF::DATATYPE_UINT8, 2 ) );
            CORE::CDynamicBuffer source;
            CORE::CDynamicBufferAccess sourceAccess( &source, false );
            ASSERT_TRUE( bgr.Save( sourceAccess, "gupimg" ) );
            CORE::CDynamicBuffer converted;
            CORE::CDynamicBufferAccess out( &converted, false );
            sourceAccess.Setpos( 0 );
            IMAGE::CImageConversionResult result;
            ASSERT_TRUE( converter.Convert( sourceAccess, "gupimg", out, "tstrgb", settings, result ) );
            ASSERT_TRUE( !result.HasLoss() );

            IMAGE::CImage wide( MakePixels( 3, 3, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_UINT16, 2 ) );
            CORE::CDynamicBuffer wideSource;
            CORE::CDynamicBufferAccess wideAccess( &wideSource, false );
            ASSERT_TRUE( wide.Save( wideAccess, "gupimg" ) );
            CORE::CDynamicBuffer wideConverted;
            CORE::CDynamicBufferAccess wideOut( &wideConverted, false );
            wideAccess.Setpos( 0 );
            ASSERT_TRUE( converter.Convert( wideAccess, "gupimg", wideOut, "tstrgb", settings, result ) );
            ASSERT_TRUE( result.HasLoss() && Contains( result.GetNotes(), "8 bit" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFrames( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "image converter: a format that holds one picture refuses an animation, and ConvertToFrames gives a file per frame" )
        try
        {
            CTestFormats formats;
            CORE::CDynamicBuffer source;
            MakeAnimationFile( IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, source );
            CORE::CDynamicBufferAccess in( &source, false );

            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;
            IMAGE::CImageConversionResult result;
            CORE::CDynamicBuffer refused;
            CORE::CDynamicBufferAccess refusedOut( &refused, false );
            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "gupimg", refusedOut, "tstrgb", settings, result ) );
            ASSERT_TRUE( !result.IsSuccess() && 0 == refused.GetDataSize() && result.GetProblem().HasSubstr( "3 frames", false ) >= 0 );

            IMAGE::CImageConverter::TEncodedFrameVector frames;
            in.Setpos( 0 );
            ASSERT_TRUE( converter.ConvertToFrames( in, "gupimg", "tstrgb", settings, result, frames ) );
            ASSERT_TRUE( 3 == frames.size() && 3 == result.GetWrittenFrameCount() );

            IMAGE::CImage original;
            ASSERT_TRUE( LoadAs( source, "gupimg", original ) );
            for ( size_t i=0; i<frames.size() && i<3; ++i )
            {
                ASSERT_TRUE( i == frames[ i ].frameIndex );
                IMAGE::CImage frame;
                ASSERT_TRUE( LoadAs( frames[ i ].data, "tstrgb", frame ) && 1 == frame.GetFrameCount() );
                ASSERT_TRUE( 0 == memcmp( original.GetPixelMap( (UInt32) i, 0 )->GetDataPtr(), frame.GetPixelMap( 0, 0 )->GetDataPtr(), original.GetPixelMap( (UInt32) i, 0 )->GetTotalSizeInBytes() ) );
            }

            // one chosen frame fits a format of one picture
            settings.SetFrameIndex( 2 );
            CORE::CDynamicBuffer single;
            CORE::CDynamicBufferAccess singleOut( &single, false );
            in.Setpos( 0 );
            ASSERT_TRUE( converter.Convert( in, "gupimg", singleOut, "tstrgb", settings, result ) );
            IMAGE::CImage third;
            ASSERT_TRUE( LoadAs( single, "tstrgb", third ) );
            ASSERT_TRUE( 0 == memcmp( original.GetPixelMap( 2, 0 )->GetDataPtr(), third.GetPixelMap( 0, 0 )->GetDataPtr(), third.GetPixelMap( 0, 0 )->GetTotalSizeInBytes() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "image converter: the built-in PNG writer takes one frame through the codec registry" )
        try
        {
            CORE::CDynamicBuffer source;
            MakeAnimationFile( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8, source );
            CORE::CDynamicBufferAccess in( &source, false );

            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;
            IMAGE::CImageConversionResult result;
            CORE::CDynamicBuffer png;
            CORE::CDynamicBufferAccess out( &png, false );
            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "gupimg", out, "png", settings, result ) );     // three frames

            settings.SetFrameIndex( 0 );
            in.Setpos( 0 );
            ASSERT_TRUE( converter.Convert( in, "gupimg", out, "png", settings, result ) );
            IMAGE::CImage original;
            ASSERT_TRUE( LoadAs( source, "gupimg", original ) );
            CORE::CDynamicBuffer expected;
            CORE::CString problem;
            ASSERT_TRUE( IMAGE::CPngImageCodec::EncodePixelMap( *original.GetPixelMap( 0, 0 ), expected, problem ) );
            ASSERT_TRUE( png.GetDataSize() == expected.GetDataSize() && 0 == memcmp( png.GetConstBufferPtr(), expected.GetConstBufferPtr(), expected.GetDataSize() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRefusals( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "image converter: unknown formats, formats that cannot be read or written and input that is not the format are refused with a reason" )
        try
        {
            CTestFormats formats;
            CORE::CDynamicBuffer source;
            MakeAnimationFile( IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, source );
            CORE::CDynamicBufferAccess in( &source, false );
            IMAGE::CImageConverter converter;
            IMAGE::CImageConversionSettings settings;
            IMAGE::CImageConversionResult result;
            CORE::CDynamicBuffer sink;
            CORE::CDynamicBufferAccess out( &sink, false );

            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "gupimg", out, "nosuchformat", settings, result ) );
            ASSERT_TRUE( result.GetProblem().HasSubstr( "nosuchformat", false ) >= 0 );

            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "nosuchformat", out, "gupimg", settings, result ) );
            ASSERT_TRUE( result.GetProblem().HasSubstr( "nosuchformat", false ) >= 0 );

            // can be read, not written
            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "gupimg", out, "tstrgbro", settings, result ) );
            ASSERT_TRUE( result.GetProblem().HasSubstr( "not written", false ) >= 0 );

            // can be written, not read: the built-in PNG writer
            if ( !IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().IsRegistered( "png" ) )
            {
                in.Setpos( 0 );
                ASSERT_FALSE( converter.Convert( in, "png", out, "gupimg", settings, result ) );
                ASSERT_TRUE( result.GetProblem().HasSubstr( "not read", false ) >= 0 );
            }

            // the data is not of the format that is claimed
            in.Setpos( 0 );
            ASSERT_FALSE( converter.Convert( in, "tstrgb", out, "gupimg", settings, result ) );
            ASSERT_TRUE( !result.IsSuccess() && 0 == sink.GetDataSize() );

            // the result of a failure is cleared by the next conversion
            in.Setpos( 0 );
            ASSERT_TRUE( converter.Convert( in, "gupimg", out, "gupimg", settings, result ) );
            ASSERT_TRUE( result.IsSuccess() && result.GetProblem().IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformImageConverterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.ImageConverter" );

    TestCapabilities();
    TestNativeConversions();
    TestFormatNegotiation();
    TestFrames();
    TestRefusals();
}

/*-------------------------------------------------------------------------*/
