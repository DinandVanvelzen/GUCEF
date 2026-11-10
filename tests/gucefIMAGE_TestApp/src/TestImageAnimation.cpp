/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
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

#include "TestImageAnimation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< CORE::UInt8 > TByteVector;

static void
Append16( TByteVector& data, CORE::UInt16 value )
{
    data.push_back( (CORE::UInt8) ( value & 0xFF ) );
    data.push_back( (CORE::UInt8) ( value >> 8 ) );
}

static void
Append32( TByteVector& data, CORE::UInt32 value )
{
    Append16( data, (CORE::UInt16) ( value & 0xFFFF ) );
    Append16( data, (CORE::UInt16) ( value >> 16 ) );
}

static void
Set32( TByteVector& data, size_t offset, CORE::UInt32 value )
{
    for ( int i=0; i<4; ++i )
        data[ offset + i ] = (CORE::UInt8) ( ( value >> ( 8 * i ) ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

static void
AppendChunk( TByteVector& frame, CORE::UInt16 type, const TByteVector& chunkData )
{
    Append32( frame, (CORE::UInt32) chunkData.size() + 6 );
    Append16( frame, type );
    frame.insert( frame.end(), chunkData.begin(), chunkData.end() );
}

/*-------------------------------------------------------------------------*/

static void
AppendFrame( TByteVector& flc, const TByteVector& chunks, CORE::UInt16 nrOfChunks )
{
    Append32( flc, (CORE::UInt32) chunks.size() + 16 );
    Append16( flc, 0xF1FA );
    Append16( flc, nrOfChunks );
    for ( int i=0; i<8; ++i )
        flc.push_back( 0 );
    flc.insert( flc.end(), chunks.begin(), chunks.end() );
}

/*-------------------------------------------------------------------------*/

/**
 *  Builds a small 2 frame FLC animation of 10x8 pixels which has the same quirks as the animations
 *  of the game Into the Void:
 *    - the offsets to the first and second frame in the header are garbage, in ITV they are
 *      positions in the archive the animation was stored in
 *    - the second frame is a COPY chunk of a frame size other than 320x200 which holds 2 pixels
 *      less than the frame has, the remaining pixels remain as they were in the previous frame
 *
 *  Frame 1 is entirely palette index 1 (red), frame 2 is palette index 2 (green) except for the last
 *  2 pixels which remain red.
 */
static void
BuildTestFlc( TByteVector& flc, CORE::UInt32 frameDelayInMs, CORE::UInt32 headerOffsetBase )
{
    const CORE::UInt16 width = 10;
    const CORE::UInt16 height = 8;

    // 128 byte FLC header
    Append32( flc, 0 ); // file size, fixed up at the end
    Append16( flc, 0xAF12 );
    Append16( flc, 2 );
    Append16( flc, width );
    Append16( flc, height );
    Append16( flc, 8 );
    Append16( flc, 3 );
    Append32( flc, frameDelayInMs );
    flc.resize( 80, 0 );
    Append32( flc, headerOffsetBase + 12345 ); // offset frame 1: garbage
    Append32( flc, headerOffsetBase + 54321 ); // offset frame 2: garbage
    flc.resize( 128, 0 );

    // Frame 1: palette + BRUN chunk filled with palette index 1
    TByteVector chunks;
    TByteVector palette;
    Append16( palette, 1 );     // nr of packets
    palette.push_back( 0 );     // colors to skip
    palette.push_back( 3 );     // nr of colors
    const CORE::UInt8 colors[ 9 ] = { 0,0,0, 255,0,0, 0,255,0 };
    palette.insert( palette.end(), colors, colors + 9 );
    AppendChunk( chunks, 4, palette );

    TByteVector brun;
    for ( CORE::UInt16 y=0; y<height; ++y )
    {
        brun.push_back( 1 );    // nr of packets, ignored by decoders
        brun.push_back( (CORE::UInt8) width );
        brun.push_back( 1 );
    }
    AppendChunk( chunks, 15, brun );
    AppendFrame( flc, chunks, 2 );

    // Frame 2: COPY chunk with 2 pixels less than the frame size, all palette index 2
    chunks.clear();
    TByteVector copy( (size_t) width * height - 2, 2 );
    AppendChunk( chunks, 16, copy );
    AppendFrame( flc, chunks, 1 );

    Set32( flc, 0, (CORE::UInt32) flc.size() );
}

/*-------------------------------------------------------------------------*/

static bool
IsPixel( const IMAGE::CImage& image, CORE::UInt32 frame, CORE::UInt32 pixelIndex, CORE::UInt8 r, CORE::UInt8 g, CORE::UInt8 b )
{
    IMAGE::TPixelMapPtr pixelMap = image.GetPixelMap( frame, 0 );
    if ( IMAGE::PSF_RGB != pixelMap->GetPixelStorageFormat() )
        return false;
    const CORE::UInt8* pixel = static_cast< const CORE::UInt8* >( pixelMap->GetDataPtr( pixelIndex ) );
    return pixel[ 0 ] == r && pixel[ 1 ] == g && pixel[ 2 ] == b;
}

/*-------------------------------------------------------------------------*/

static bool
LoadImagePlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    // Image codec plugins are plain C API plugins so they have to be told what type of plugin they are,
    // and since they hold on to gucefIMAGE codec registrations they must be unloaded before the modules they link back to
    CORE::CPluginMetaData pluginMetaData;
    pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
    pluginMetaData.SetFullModulePath( "$MODULEDIR$" );
    pluginMetaData.SetModuleFilename( pluginName );
    pluginMetaData.SetAltModuleFilename( pluginName + "_d" );
    pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginMetaData( pluginMetaData, "test", true, true );
}

/*-------------------------------------------------------------------------*/

static IMAGE::TPixelMapPtr
CreateTestPixelMap( CORE::UInt8 gray )
{
    CORE::UInt8 pixels[ 4 * 4 * 3 ];
    memset( pixels, gray, sizeof( pixels ) );
    return IMAGE::TPixelMapPtr( new IMAGE::CPixelMap( pixels, 4, 4, IMAGE::PSF_RGB, (IMAGE::TBuildinDataType) GUCEF_DATATYPE_UINT8 ) );
}

/*-------------------------------------------------------------------------*/

static void
TestImageFrameTiming( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "CImage frame durations and looping" )
        try
        {
            IMAGE::CImage image;
            GUCEF_TESTFW_ASSERT_FALSE( image.IsAnimation() );

            IMAGE::TPixelMapPtr frame1 = CreateTestPixelMap( 10 );
            IMAGE::TPixelMapPtr frame2 = CreateTestPixelMap( 20 );
            IMAGE::TPixelMapPtr frame3 = CreateTestPixelMap( 30 );
            image.AddFrame( frame1, 40 );
            GUCEF_TESTFW_ASSERT_FALSE( image.IsAnimation() ); // a single frame is never an animation
            image.AddFrame( frame2, 60 );
            image.AddFrame( frame3 );  // no duration given

            GUCEF_TESTFW_ASSERT_TRUE( 3 == image.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 40 == image.GetFrameDurationInMs( 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 60 == image.GetFrameDurationInMs( 1 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 0 == image.GetFrameDurationInMs( 2 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 100 == image.GetTotalDurationInMs() );
            GUCEF_TESTFW_ASSERT_TRUE( image.IsAnimation() );

            image.SetFrameDurationInMs( 2, 25 );
            GUCEF_TESTFW_ASSERT_TRUE( 125 == image.GetTotalDurationInMs() );

            GUCEF_TESTFW_ASSERT_TRUE( 0 == image.GetLoopCount() ); // loops forever by default
            image.SetLoopCount( 3 );
            GUCEF_TESTFW_ASSERT_TRUE( 3 == image.GetLoopCount() );

            // replacing a frame must not disturb the timing of the others
            IMAGE::TPixelMapPtr replacement = CreateTestPixelMap( 99 );
            image.SetFrame( replacement, 1 );
            GUCEF_TESTFW_ASSERT_TRUE( 60 == image.GetFrameDurationInMs( 1 ) );

            // invalid index
            bool thrown = false;
            try { image.GetFrameDurationInMs( 3 ); } catch ( const IMAGE::CImage::EInvalidIndex& ) { thrown = true; }
            GUCEF_TESTFW_ASSERT_TRUE( thrown );
            thrown = false;
            try { image.SetFrameDurationInMs( 3, 1 ); } catch ( const IMAGE::CImage::EInvalidIndex& ) { thrown = true; }
            GUCEF_TESTFW_ASSERT_TRUE( thrown );

            image.Clear();
            GUCEF_TESTFW_ASSERT_TRUE( 0 == image.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 0 == image.GetTotalDurationInMs() );
            GUCEF_TESTFW_ASSERT_TRUE( 0 == image.GetLoopCount() );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CImage C style access keeps frame timing" )
        try
        {
            IMAGE::CImage image;
            IMAGE::TPixelMapPtr frame1 = CreateTestPixelMap( 10 );
            IMAGE::TPixelMapPtr frame2 = CreateTestPixelMap( 20 );
            image.AddFrame( frame1, 71 );
            image.AddFrame( frame2, 57 );
            image.SetLoopCount( 5 );

            IMAGE::TImage* cStyleImage = image.CreateCStyleAccess();
            GUCEF_TESTFW_ASSERT_TRUE( GUCEF_IMAGE_TIMAGEINFO_VERSION == cStyleImage->imageInfo.version );
            GUCEF_TESTFW_ASSERT_TRUE( 5 == cStyleImage->imageInfo.loopCount );
            GUCEF_TESTFW_ASSERT_TRUE( 71 == cStyleImage->frames[ 0 ].frameInfo.frameDurationInMs );
            GUCEF_TESTFW_ASSERT_TRUE( 57 == cStyleImage->frames[ 1 ].frameInfo.frameDurationInMs );

            IMAGE::CImage copy;
            copy.Assign( *cStyleImage );
            image.FreeCStyleAccess( cStyleImage );

            GUCEF_TESTFW_ASSERT_TRUE( 2 == copy.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 71 == copy.GetFrameDurationInMs( 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 57 == copy.GetFrameDurationInMs( 1 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 5 == copy.GetLoopCount() );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFlicDecoding( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "FLIC codec plugin decodes animation with ITV style quirks" )
        try
        {
            GUCEF_TESTFW_ASSERT_TRUE( LoadImagePlugin( "imgpluginFLIC" ) );

            // Plain FLC data
            TByteVector flc;
            BuildTestFlc( flc, 42, 0 );
            CORE::CDynamicBuffer buffer( (CORE::UInt32) flc.size(), true );
            buffer.CopyFrom( (CORE::UInt32) flc.size(), &flc[ 0 ] );
            CORE::CDynamicBufferAccess access( &buffer, false );

            IMAGE::CImage animation;
            GUCEF_TESTFW_ASSERT_TRUE( animation.Load( access, "flic", true, false ) );
            GUCEF_TESTFW_ASSERT_TRUE( 2 == animation.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 10 == animation.GetPixelMap( 0, 0 )->GetWidthInPixels() );
            GUCEF_TESTFW_ASSERT_TRUE( 8 == animation.GetPixelMap( 0, 0 )->GetHeightInPixels() );
            GUCEF_TESTFW_ASSERT_TRUE( 42 == animation.GetFrameDurationInMs( 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( 42 == animation.GetFrameDurationInMs( 1 ) );
            GUCEF_TESTFW_ASSERT_TRUE( animation.IsAnimation() );

            // frame 1 is all red
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 0, 0, 255, 0, 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 0, 79, 255, 0, 0 ) );
            // frame 2 is green, except the 2 pixels the COPY chunk did not provide which are still red
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 1, 0, 0, 255, 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 1, 77, 0, 255, 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 1, 78, 255, 0, 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( animation, 1, 79, 255, 0, 0 ) );

            // The same animation as an Into the Void resource: preceded by the 55 byte resource header and the
            // frame offsets are positions within the archive the resource is stored in
            TByteVector itvResource( 55, 0 );
            TByteVector itvFlc;
            BuildTestFlc( itvFlc, 71, 33857 + 55 );
            itvResource.insert( itvResource.end(), itvFlc.begin(), itvFlc.end() );
            CORE::CDynamicBuffer itvBuffer( (CORE::UInt32) itvResource.size(), true );
            itvBuffer.CopyFrom( (CORE::UInt32) itvResource.size(), &itvResource[ 0 ] );
            CORE::CDynamicBufferAccess itvAccess( &itvBuffer, false );

            IMAGE::CImage itvAnimation;
            GUCEF_TESTFW_ASSERT_TRUE( itvAnimation.Load( itvAccess, "itv19", true, false ) );
            GUCEF_TESTFW_ASSERT_TRUE( 2 == itvAnimation.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 71 == itvAnimation.GetFrameDurationInMs( 1 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( itvAnimation, 1, 0, 0, 255, 0 ) );
            GUCEF_TESTFW_ASSERT_TRUE( IsPixel( itvAnimation, 1, 79, 255, 0, 0 ) );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  A codec that has only the basic codec interface (no direct image interface), as every image codec plugin
 *  has that was loaded before gucefIMAGE was initialized. It produces a 3 frame animation of 3x2 RGB pixels:
 *  frame n is entirely the gray value 10 * (n + 1), shown for 20 * (n + 1) ms, looping 5 times.
 */
class CBasicInterfaceTestCodec : public CORE::CICodec
{
    public:

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) { return false; }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest )
    {
        IMAGE::TImageInfo imageInfo;
        imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
        imageInfo.nrOfFramesInImage = 3;
        imageInfo.loopCount = 5;
        dest.Write( &imageInfo, sizeof( imageInfo ), 1 );

        for ( CORE::UInt32 n=0; n<3; ++n )
        {
            IMAGE::TImageFrameInfo frameInfo;
            frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
            frameInfo.nrOfMipmapLevels = 1;
            frameInfo.frameDurationInMs = 20 * ( n + 1 );
            dest.Write( &frameInfo, sizeof( frameInfo ), 1 );

            IMAGE::TImageMipMapLevelInfo mipMapInfo;
            mipMapInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
            mipMapInfo.pixelComponentDataType = GUCEF_DATATYPE_UINT8;
            mipMapInfo.pixelStorageFormat = IMAGE::PSF_RGB;
            mipMapInfo.frameWidth = 3;
            mipMapInfo.frameHeight = 2;
            dest.Write( &mipMapInfo, sizeof( mipMapInfo ), 1 );
        }
        for ( CORE::UInt32 n=0; n<3; ++n )
        {
            CORE::UInt8 pixels[ 3 * 2 * 3 ];
            memset( pixels, (int) ( 10 * ( n + 1 ) ), sizeof( pixels ) );
            dest.Write( pixels, sizeof( pixels ), 1 );
        }
        return true;
    }

    virtual CORE::CString GetType( void ) const { return "testbasic"; }

    virtual CORE::CString GetFamilyName( void ) const { return "ImageCodec"; }

    virtual CORE::CICloneable* Clone( void ) const { return new CBasicInterfaceTestCodec(); }

    virtual const CORE::CString& GetClassTypeName( void ) const
    {
        static CORE::CString classTypeName = "CBasicInterfaceTestCodec";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

static void
TestBasicInterfaceCodec( void )
{GUCEF_TRACE;

    // Regression: the decoding of a codec with only the basic interface compared the number of elements read
    // with the size in bytes of the header, so it failed for every codec that is not known to gucefIMAGE directly
    GUCEF_TESTFW_TESTCASE( "CImage decodes with a codec that has only the basic codec interface" )
        try
        {
            IMAGE::CImageGlobal::Instance();

            CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
            CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
            GUCEF_TESTFW_ASSERT_TRUE( codecRegistry.TryLookup( "ImageCodec", family, false ) && family );
            family->Register( "testbasic", CORE::CCodecRegistry::TICodecTypedPtr( new CBasicInterfaceTestCodec() ) );

            CORE::CDynamicBuffer input( 4, true );
            CORE::CDynamicBufferAccess inputAccess( &input, false );
            IMAGE::CImage animation;
            GUCEF_TESTFW_ASSERT_TRUE( animation.Load( inputAccess, "testbasic", true, false ) );
            GUCEF_TESTFW_ASSERT_TRUE( 3 == animation.GetFrameCount() );
            GUCEF_TESTFW_ASSERT_TRUE( 5 == animation.GetLoopCount() );
            for ( CORE::UInt32 n=0; n<3; ++n )
            {
                GUCEF_TESTFW_ASSERT_TRUE( 20 * ( n + 1 ) == animation.GetFrameDurationInMs( n ) );
                IMAGE::TPixelMapPtr frame = animation.GetPixelMap( n, 0 );
                GUCEF_TESTFW_ASSERT_TRUE( !frame.IsNULL() );
                if ( frame.IsNULL() )
                    continue;
                GUCEF_TESTFW_ASSERT_TRUE( 3 == frame->GetWidthInPixels() && 2 == frame->GetHeightInPixels() );
                const CORE::UInt8* pixels = static_cast< const CORE::UInt8* >( frame->GetDataPtr() );
                GUCEF_TESTFW_ASSERT_TRUE( pixels[ 0 ] == 10 * ( n + 1 ) && pixels[ 17 ] == 10 * ( n + 1 ) );
            }

            family->Unregister( "testbasic" );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformImageAnimationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Image animation" );

    TestImageFrameTiming();
    TestFlicDecoding();
    TestBasicInterfaceCodec();
}

/*-------------------------------------------------------------------------*/
