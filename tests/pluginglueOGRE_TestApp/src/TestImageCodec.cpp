/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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
#include <typeinfo>

#include "OgreCodec.h"
#include "OgreImage.h"
#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgrePixelFormat.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "TestSupport.h"
#include "TestImageCodec.h"

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

/** The test image codec expects this in front of its data */
static const char TestMagic[ 4 ] = { 'O', 'G', 'T', '1' };

/** The value of a channel of a pixel in the test images: it differs for every face, level, position and channel */
static UInt8
PixelValue( UInt32 face, UInt32 level, UInt32 x, UInt32 y, UInt32 channel )
{
    return (UInt8) ( face * 37 + level * 11 + x * 5 + y * 3 + channel * 7 + 1 );
}

/*-------------------------------------------------------------------------*/

/** A pixel map of 8 bit channels filled with the test values, the channels in the order of the memory */
static IMAGE::TPixelMapPtr
MakeBytePixelMap( UInt32 width, UInt32 height, IMAGE::TPixelStorageFormat format, UInt32 face, UInt32 level )
{GUCEF_TRACE;

    const UInt32 channels = IMAGE::CPixelMap::GetChannelCountForFormat( format );
    GUCEF::vector< UInt8 > data( (size_t) width * height * channels );
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width; ++x )
            for ( UInt32 c=0; c<channels; ++c )
                data[ ( (size_t) y * width + x ) * channels + c ] = PixelValue( face, level, x, y, c );
    return IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( &data[ 0 ], width, height, format, DATATYPE_UINT8 ) );
}

/*-------------------------------------------------------------------------*/

/** A chain of levels, each half the size of the one before */
static IMAGE::CImage::TMipMapList
MakeMipChain( UInt32 width, UInt32 height, UInt32 levels, IMAGE::TPixelStorageFormat format, UInt32 face )
{GUCEF_TRACE;

    IMAGE::CImage::TMipMapList chain;
    for ( UInt32 level=0; level<levels; ++level )
    {
        const UInt32 w = ( width >> level ) > 0 ? ( width >> level ) : 1;
        const UInt32 h = ( height >> level ) > 0 ? ( height >> level ) : 1;
        chain.push_back( MakeBytePixelMap( w, h, format, face, level ) );
    }
    return chain;
}

/*-------------------------------------------------------------------------*/

/**
 *  A codec in the image codec registry of gucefIMAGE, in the form an image plugin has: it decodes by the name it is
 *  registered under into one of the test images. The data must begin with the magic that the stream is checked with
 */
class CTestImageCodec : public IMAGE::CIImageCodec
{
    public:

    explicit CTestImageCodec( const CORE::CString& type ) : IMAGE::CIImageCodec(), m_type( type ) {}
    virtual ~CTestImageCodec() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool Encode( const IMAGE::CImage& inputImage, CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_CODEC_CAPABILITY_DECODE; }
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_type; }
    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ImageCodec"; }
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestImageCodec( m_type ); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::OGRE::TEST::CTestImageCodec";
        return classTypeName;
    }

    virtual bool Decode( CORE::CIOAccess& encodedInput, IMAGE::CImage& outputImage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // the data is read from where it is: the adapter has to hand it over at the start
        char magic[ 4 ];
        if ( 4 != encodedInput.Read( magic, 1, 4 ) || 0 != memcmp( magic, TestMagic, 4 ) )
            return false;

        if ( "ogtfail" == m_type )
            return false;

        if ( "ogtrgba" == m_type )
            outputImage.Assign( MakeBytePixelMap( 5, 3, IMAGE::PSF_RGBA, 0, 0 ) );
        else
        if ( "ogtbgr" == m_type )
            outputImage.Assign( MakeBytePixelMap( 4, 4, IMAGE::PSF_BGR, 0, 0 ) );
        else
        if ( "ogtgray" == m_type )
            outputImage.Assign( MakeBytePixelMap( 6, 2, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, 0, 0 ) );
        else
        if ( "ogtmips" == m_type )
            outputImage.Assign( MakeMipChain( 8, 4, 4, IMAGE::PSF_RGBA, 0 ) );
        else
        if ( "ogtbadmips" == m_type )
        {
            // the third level is not half of the second
            IMAGE::CImage::TMipMapList chain = MakeMipChain( 8, 8, 2, IMAGE::PSF_RGBA, 0 );
            chain.push_back( MakeBytePixelMap( 3, 3, IMAGE::PSF_RGBA, 0, 2 ) );
            outputImage.Assign( chain );
        }
        else
        if ( "ogtcube" == m_type )
        {
            IMAGE::CImage::TFrameList faces;
            for ( UInt32 face=0; face<6; ++face )
                faces.push_back( MakeMipChain( 4, 4, 3, IMAGE::PSF_RGBA, face ) );
            outputImage.Assign( faces );
        }
        else
        if ( "ogtthree" == m_type || "ogtanim" == m_type )
        {
            IMAGE::CImage::TFrameList frames;
            const UInt32 count = "ogtanim" == m_type ? 6 : 3;
            for ( UInt32 frame=0; frame<count; ++frame )
                frames.push_back( MakeMipChain( 4, 4, 1, IMAGE::PSF_RGBA, frame ) );
            outputImage.Assign( frames );
            if ( "ogtanim" == m_type )
            {
                for ( UInt32 frame=0; frame<count; ++frame )
                    outputImage.SetFrameDurationInMs( frame, 40 );
            }
        }
        else
        if ( "ogtmixed" == m_type )
        {
            // the format of the first level is the one for all
            IMAGE::CImage::TMipMapList chain;
            chain.push_back( MakeBytePixelMap( 4, 4, IMAGE::PSF_RGB, 0, 0 ) );
            chain.push_back( MakeBytePixelMap( 2, 2, IMAGE::PSF_RGBA, 0, 1 ) );
            outputImage.Assign( chain );
        }
        else
        if ( "ogtshort" == m_type )
        {
            UInt16 data[ 2 * 2 * 4 ];
            for ( UInt32 i=0; i<16; ++i )
                data[ i ] = (UInt16) ( 1000 + i * 257 );
            outputImage.Assign( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( data, 2, 2, IMAGE::PSF_RGBA, DATATYPE_LE_UINT16 ) ) );
        }
        else
        if ( "ogtfloat" == m_type )
        {
            Float32 data[ 2 * 2 * 4 ];
            for ( UInt32 i=0; i<16; ++i )
                data[ i ] = 0.25f * (Float32) i;
            outputImage.Assign( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( data, 2, 2, IMAGE::PSF_RGBA, DATATYPE_LE_FLOAT32 ) ) );
        }
        else
        if ( "ogtpal" == m_type )
            outputImage.Assign( MakeBytePixelMap( 4, 4, IMAGE::PSF_PALETTE_INDICES, 0, 0 ) );
        else
        if ( "ogtgreen" == m_type )
            outputImage.Assign( MakeBytePixelMap( 4, 2, IMAGE::PSF_SINGLE_CHANNEL_GREEN, 0, 0 ) );
        else
            return false;
        return true;
    }

    private:

    CORE::CString m_type;
};

typedef CORE::CTSharedPtr< CTestImageCodec, MT::CMutex > TTestImageCodecPtr;

/*-------------------------------------------------------------------------*/

static const char* const TestCodecNames[] = { "ogtrgba", "ogtbgr", "ogtgray", "ogtmips", "ogtbadmips", "ogtcube", "ogtthree", "ogtanim",
                                              "ogtmixed", "ogtshort", "ogtfloat", "ogtpal", "ogtgreen", "ogtfail", GUCEF_NULL };

/** Puts the codecs of the test in the image codec registry of gucefIMAGE, as an image plugin would */
static void
RegisterTestCodecs( void )
{GUCEF_TRACE;

    IMAGE::CImageCodecRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    for ( int i=0; GUCEF_NULL != TestCodecNames[ i ]; ++i )
    {
        TTestImageCodecPtr codec( GUCEF_NEW CTestImageCodec( TestCodecNames[ i ] ) );
        registry.TryRegister( TestCodecNames[ i ], codec );
    }
}

/*-------------------------------------------------------------------------*/

static void
UnregisterTestCodecs( void )
{GUCEF_TRACE;

    IMAGE::CImageCodecRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    for ( int i=0; GUCEF_NULL != TestCodecNames[ i ]; ++i )
    {
        if ( registry.IsRegistered( TestCodecNames[ i ] ) )
            registry.Unregister( TestCodecNames[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

/** A stream of the test data, which is not at its start when it is handed over: the codec has to go to the start */
static Ogre::DataStreamPtr
MakeTestStream( const char* extension )
{GUCEF_TRACE;

    Ogre::MemoryDataStreamPtr stream = std::make_shared< Ogre::MemoryDataStream >( Ogre::String( "test." ) + extension, (size_t) 64 );
    memset( stream->getPtr(), 0, 64 );
    memcpy( stream->getPtr(), TestMagic, 4 );
    stream->seek( 17 );
    return stream;
}

/*-------------------------------------------------------------------------*/

/** Whether the face and level of the image hold the test pixels of the face and level in the byte format with the given channels */
static bool
HasTestPixels( const Ogre::Image& image, UInt32 face, UInt32 level, UInt32 channels, UInt32 sourceFace, UInt32 sourceLevel )
{GUCEF_TRACE;

    const Ogre::PixelBox box = image.getPixelBox( face, level );
    const size_t pixelSize = Ogre::PixelUtil::getNumElemBytes( box.format );
    if ( pixelSize != channels )
        return false;
    const Ogre::uchar* data = static_cast< const Ogre::uchar* >( box.data );
    for ( UInt32 y=0; y<box.getHeight(); ++y )
        for ( UInt32 x=0; x<box.getWidth(); ++x )
            for ( UInt32 c=0; c<channels; ++c )
                if ( data[ ( (size_t) y * box.rowPitch + x ) * pixelSize + c ] != PixelValue( sourceFace, sourceLevel, x, y, c ) )
                    return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
Throws( const char* extension )
{GUCEF_TRACE;

    try
    {
        Ogre::Image image;
        image.load( MakeTestStream( extension ), extension );
    }
    catch ( const Ogre::Exception& )
    {
        return true;
    }
    return false;
}

} // anonymous namespace

/*-------------------------------------------------------------------------*/

static void
TestRegistration( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: Ogre has a codec for every extension of gucefIMAGE, follows it when codecs come and go, and keeps its own for the formats that go to the GPU" )
        try
        {
            // the built-in PNG encoder of gucefIMAGE is an image codec of the codec registry: encode only, but there
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "png" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "ogtrgba" ) );

            // the codecs of an image plugin that registers later
            RegisterTestCodecs();
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "ogtrgba" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "ogtcube" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "ogtfail" ) );
            Ogre::Codec* codec = Ogre::Codec::getCodec( "ogtrgba" );
            ASSERT_TRUE( GUCEF_NULL != codec && "ogtrgba" == codec->getType() );
            // lower case is the name of the codec whatever case the file has
            ASSERT_TRUE( Ogre::Codec::getCodec( "OGTRGBA" ) == codec );

            // the formats that are uploaded to the GPU as they are stay with Ogre
            const char* gpuFormats[] = { "dds", "pkm", "ktx", "astc", "pvr", GUCEF_NULL };
            for ( int i=0; GUCEF_NULL != gpuFormats[ i ]; ++i )
            {
                if ( Ogre::Codec::isCodecRegistered( gpuFormats[ i ] ) )
                {
                    const Ogre::Codec* gpuCodec = Ogre::Codec::getCodec( gpuFormats[ i ] );
                    ASSERT_TRUE( Ogre::String( typeid( *gpuCodec ).name() ).find( "GucefImage" ) == Ogre::String::npos );
                }
            }
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "dds" ) );

            // and they go when the codec of gucefIMAGE does
            UnregisterTestCodecs();
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "ogtrgba" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "ogtcube" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "png" ) );
            RegisterTestCodecs();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestDecodeFormats( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: images decode through Ogre::Image in the pixel format that matches, with every pixel where it belongs" )
        try
        {
            Ogre::Image image;
            image.load( MakeTestStream( "ogtrgba" ), "ogtrgba" );
            ASSERT_TRUE( 5 == image.getWidth() && 3 == image.getHeight() && 1 == image.getDepth() );
            ASSERT_TRUE( Ogre::PF_BYTE_RGBA == image.getFormat() );
            ASSERT_TRUE( 1 == image.getNumFaces() && 0 == image.getNumMipmaps() );
            ASSERT_TRUE( image.getHasAlpha() );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 4, 0, 0 ) );
            // the buffer is the pixels, not more
            ASSERT_TRUE( 5 * 3 * 4 == image.getSize() );

            image.load( MakeTestStream( "ogtbgr" ), "ogtbgr" );
            ASSERT_TRUE( Ogre::PF_BYTE_BGR == image.getFormat() && 4 == image.getWidth() && 4 == image.getHeight() );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 3, 0, 0 ) );

            image.load( MakeTestStream( "ogtgray" ), "ogtgray" );
            ASSERT_TRUE( Ogre::PF_L8 == image.getFormat() && 6 == image.getWidth() && 2 == image.getHeight() );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 1, 0, 0 ) );

            // 16 bit and floating point channels keep their type
            image.load( MakeTestStream( "ogtshort" ), "ogtshort" );
            ASSERT_TRUE( Ogre::PF_SHORT_RGBA == image.getFormat() && 2 == image.getWidth() );
            {
                const Ogre::uint16* shorts = static_cast< const Ogre::uint16* >( static_cast< const void* >( image.getData() ) );
                ASSERT_TRUE( 1000 == shorts[ 0 ] && 1000 + 15 * 257 == shorts[ 15 ] );
            }
            image.load( MakeTestStream( "ogtfloat" ), "ogtfloat" );
            ASSERT_TRUE( Ogre::PF_FLOAT32_RGBA == image.getFormat() );
            {
                const float* floats = reinterpret_cast< const float* >( image.getData() );
                ASSERT_TRUE( 0.0f == floats[ 0 ] && 3.75f == floats[ 15 ] );
            }

            // the case of the extension does not matter
            image.load( MakeTestStream( "ogtrgba" ), "OgtRgba" );
            ASSERT_TRUE( 5 == image.getWidth() );

            // the name of the stream tells the type when none is given
            image.load( MakeTestStream( "ogtbgr" ) );
            ASSERT_TRUE( Ogre::PF_BYTE_BGR == image.getFormat() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestMipmapsAndCubeFaces( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: mipmap levels and cube faces land in the layout of Ogre: faces one after the other, the levels of a face one after the other, nothing overwritten" )
        try
        {
            Ogre::Image image;
            image.load( MakeTestStream( "ogtmips" ), "ogtmips" );
            ASSERT_TRUE( 8 == image.getWidth() && 4 == image.getHeight() );
            ASSERT_TRUE( 3 == image.getNumMipmaps() && 1 == image.getNumFaces() );
            // 8x4 + 4x2 + 2x1 + 1x1 pixels of 4 bytes
            ASSERT_TRUE( ( 32 + 8 + 2 + 1 ) * 4 == image.getSize() );
            for ( UInt32 level=0; level<4; ++level )
            {
                ASSERT_TRUE( HasTestPixels( image, 0, level, 4, 0, level ) );
            }
            ASSERT_TRUE( 4 == image.getPixelBox( 0, 1 ).getWidth() && 2 == image.getPixelBox( 0, 1 ).getHeight() );
            ASSERT_TRUE( 1 == image.getPixelBox( 0, 3 ).getWidth() && 1 == image.getPixelBox( 0, 3 ).getHeight() );

            // a cube map: six faces of three levels
            image.load( MakeTestStream( "ogtcube" ), "ogtcube" );
            ASSERT_TRUE( 4 == image.getWidth() && 4 == image.getHeight() );
            ASSERT_TRUE( 6 == image.getNumFaces() && 2 == image.getNumMipmaps() );
            ASSERT_TRUE( image.hasFlag( Ogre::IF_CUBEMAP ) );
            ASSERT_TRUE( 6 * ( 16 + 4 + 1 ) * 4 == image.getSize() );
            for ( UInt32 face=0; face<6; ++face )
            {
                for ( UInt32 level=0; level<3; ++level )
                {
                    ASSERT_TRUE( HasTestPixels( image, face, level, 4, face, level ) );
                }
            }

            // a chain whose level does not follow its predecessor stops there
            image.load( MakeTestStream( "ogtbadmips" ), "ogtbadmips" );
            ASSERT_TRUE( 8 == image.getWidth() && 1 == image.getNumMipmaps() );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 4, 0, 0 ) && HasTestPixels( image, 0, 1, 4, 0, 1 ) );

            // more than one frame is not a cube map, and a cube of six is not a cube when it is an animation
            image.load( MakeTestStream( "ogtthree" ), "ogtthree" );
            ASSERT_TRUE( 1 == image.getNumFaces() && !image.hasFlag( Ogre::IF_CUBEMAP ) );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 4, 0, 0 ) );
            image.load( MakeTestStream( "ogtanim" ), "ogtanim" );
            ASSERT_TRUE( 1 == image.getNumFaces() && !image.hasFlag( Ogre::IF_CUBEMAP ) );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 4, 0, 0 ) );

            // levels in another format than the first level are converted to it
            image.load( MakeTestStream( "ogtmixed" ), "ogtmixed" );
            ASSERT_TRUE( Ogre::PF_BYTE_RGB == image.getFormat() && 1 == image.getNumMipmaps() );
            ASSERT_TRUE( HasTestPixels( image, 0, 0, 3, 0, 0 ) );
            {
                // the second level was RGBA, the RGB of it is what is left
                const Ogre::PixelBox box = image.getPixelBox( 0, 1 );
                ASSERT_TRUE( 2 == box.getWidth() );
                const Ogre::uchar* data = static_cast< const Ogre::uchar* >( box.data );
                ASSERT_TRUE( PixelValue( 0, 1, 0, 0, 0 ) == data[ 0 ] && PixelValue( 0, 1, 0, 0, 2 ) == data[ 2 ] );
                ASSERT_TRUE( PixelValue( 0, 1, 1, 1, 1 ) == data[ ( (size_t) box.rowPitch + 1 ) * 3 + 1 ] );
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
TestFailures( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: images that cannot be decoded or used give an Ogre exception, with the image untouched by half results" )
        try
        {
            // the decoder of gucefIMAGE refuses the data
            ASSERT_TRUE( Throws( "ogtfail" ) );
            // palette indices without a palette
            ASSERT_TRUE( Throws( "ogtpal" ) );

            // the data has not the magic: the decoder sees what is at the start, a stream that is positioned elsewhere is not the reason
            Ogre::MemoryDataStreamPtr wrong = std::make_shared< Ogre::MemoryDataStream >( "wrong.ogtrgba", (size_t) 64 );
            memset( wrong->getPtr(), 0, 64 );
            bool refused = false;
            try
            {
                Ogre::Image image;
                image.load( wrong, "ogtrgba" );
            }
            catch ( const Ogre::Exception& )
            {
                refused = true;
            }
            ASSERT_TRUE( refused );

            // a format that Ogre has no pixel format for is converted, when gucefIMAGE can
            IMAGE::TPixelMapPtr green = MakeBytePixelMap( 4, 2, IMAGE::PSF_SINGLE_CHANNEL_GREEN, 0, 0 );
            IMAGE::TPixelMapPtr asRgba;
            if ( green->ConvertFormatTo( IMAGE::PSF_RGBA, DATATYPE_UINT8, asRgba ) && !asRgba.IsNULL() )
            {
                Ogre::Image image;
                image.load( MakeTestStream( "ogtgreen" ), "ogtgreen" );
                ASSERT_TRUE( Ogre::PF_BYTE_RGBA == image.getFormat() && 4 == image.getWidth() && 2 == image.getHeight() );
                ASSERT_TRUE( 4 * 2 * 4 == image.getSize() );
                const Ogre::uchar* data = image.getData();
                const UInt8* expected = static_cast< const UInt8* >( asRgba->GetDataPtr() );
                ASSERT_TRUE( 0 == memcmp( data, expected, 4 * 2 * 4 ) );
            }
            else
            {
                ASSERT_TRUE( Throws( "ogtgreen" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static Ogre::Image
MakeOgreImage( void )
{GUCEF_TRACE;

    Ogre::Image image;
    image.create( Ogre::PF_BYTE_RGBA, 5, 3 );
    Ogre::uchar* data = image.getData();
    for ( UInt32 y=0; y<3; ++y )
        for ( UInt32 x=0; x<5; ++x )
            for ( UInt32 c=0; c<4; ++c )
                data[ ( y * 5 + x ) * 4 + c ] = PixelValue( 0, 0, x, y, c );
    return image;
}

/*-------------------------------------------------------------------------*/

static void
TestEncode( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: Ogre images encode to PNG in memory through the encoder of gucefIMAGE, files are not written by Ogre" )
        try
        {
            Ogre::Image image = MakeOgreImage();
            Ogre::DataStreamPtr png = image.encode( "png" );
            ASSERT_TRUE( (bool) png );
            if ( png )
            {
                ASSERT_TRUE( png->size() > 8 + 25 + 12 );
                UInt8 header[ 29 ];
                ASSERT_TRUE( 29 == png->read( header, 29 ) );
                static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
                ASSERT_TRUE( 0 == memcmp( header, signature, 8 ) );
                ASSERT_TRUE( 0 == memcmp( header + 12, "IHDR", 4 ) );
                // width 5, height 3, 8 bit, colour type 6 (RGBA)
                ASSERT_TRUE( 0 == header[ 16 ] && 0 == header[ 17 ] && 0 == header[ 18 ] && 5 == header[ 19 ] );
                ASSERT_TRUE( 0 == header[ 20 ] && 0 == header[ 21 ] && 0 == header[ 22 ] && 3 == header[ 23 ] );
                ASSERT_TRUE( 8 == header[ 24 ] && 6 == header[ 25 ] );
            }

            // other pixel formats are written as 8 bit RGBA
            Ogre::Image floatImage;
            floatImage.create( Ogre::PF_FLOAT32_RGBA, 2, 2 );
            memset( floatImage.getData(), 0, floatImage.getSize() );
            ASSERT_TRUE( (bool) floatImage.encode( "png" ) );

            // images in the formats of the GPU are not for encoding
            Ogre::Image compressed;
            compressed.create( Ogre::PF_DXT1, 8, 8 );
            bool refused = false;
            try { compressed.encode( "png" ); } catch ( const Ogre::Exception& ) { refused = true; }
            ASSERT_TRUE( refused );

            // there is no encoder for a format that gucefIMAGE can only read
            refused = false;
            try { image.encode( "ogtrgba" ); } catch ( const Ogre::Exception& ) { refused = true; }
            ASSERT_TRUE( refused );

            // Ogre does not write files: saving is not supported and leaves no file
            const CORE::CString dir = MakeCleanTestDir( "image_save" );
            const CORE::CString path = dir + "/saved.png";
            refused = false;
            try { image.save( path.C_String() ); } catch ( const Ogre::Exception& ) { refused = true; }
            ASSERT_TRUE( refused );
            ASSERT_FALSE( CORE::FileExists( path ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static bool
LoadImagePlugin( const CORE::CString& pluginName, const CORE::CString& groupName )
{GUCEF_TRACE;

    CORE::CPluginMetaData pluginMetaData;
    pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
    pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
    pluginMetaData.SetModuleFilename( pluginName );
    pluginMetaData.SetAltModuleFilename( pluginName + "_d" );
    pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );

    return CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginMetaData( pluginMetaData, groupName, true, true );
}

/*-------------------------------------------------------------------------*/

static void
TestWithImagePlugin( void )
{GUCEF_TRACE;

    const CORE::CString group = "ogreimagecodectests";
    const bool loaded = LoadImagePlugin( "imgpluginFreeImage", group );

    GUCEF_TESTFW_TESTCASE( "Image codec plugin: with an image plugin of gucefIMAGE loaded, PNG decodes through it into the pixels that were encoded, and its formats are in Ogre only while it is loaded" )
        GUCEF_TESTFW_SKIP_IF( !loaded, "imgpluginFreeImage is not available" );
        try
        {

            // the formats of the plugin
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "jpg" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "png" ) );
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "bmp" ) );

            // PNG: encoded by Ogre::Image through gucefIMAGE and decoded again through it
            Ogre::Image original = MakeOgreImage();
            Ogre::DataStreamPtr png = original.encode( "png" );
            ASSERT_TRUE( (bool) png );
            if ( png )
            {
                Ogre::Image decoded;
                decoded.load( png, "png" );
                ASSERT_TRUE( 5 == decoded.getWidth() && 3 == decoded.getHeight() && 1 == decoded.getNumFaces() );
                ASSERT_TRUE( Ogre::PixelUtil::isAccessible( decoded.getFormat() ) );
                ASSERT_TRUE( 4 == Ogre::PixelUtil::getNumElemBytes( decoded.getFormat() ) );

                // the pixels are the ones that were encoded, each at its place, whatever the order of the channels in memory
                for ( UInt32 y=0; y<3; ++y )
                {
                    for ( UInt32 x=0; x<5; ++x )
                    {
                        const Ogre::ColourValue expected = original.getColourAt( x, y, 0 );
                        const Ogre::ColourValue actual = decoded.getColourAt( x, y, 0 );
                        ASSERT_TRUE( expected == actual );
                    }
                }
            }

            // the codecs follow the plugin out
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( group ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "jpg" ) );
            ASSERT_FALSE( Ogre::Codec::isCodecRegistered( "bmp" ) );
            // the encoder that gucefIMAGE has itself stays
            ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "png" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformImageCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgreCodec_GucefIMAGE" );

    TestRegistration();
    TestDecodeFormats();
    TestMipmapsAndCubeFaces();
    TestFailures();
    TestEncode();
    TestWithImagePlugin();

    UnregisterTestCodecs();
}

/*-------------------------------------------------------------------------*/
