/*
 *  gucefVIDEO_TestApp: GUCEF test application for the VIDEO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEFVIDEO_H
#include "gucefVIDEO.h"
#define GUCEFVIDEO_H
#endif /* GUCEFVIDEO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVideoFormat.h"

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
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVideoFormatTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING video format TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefVIDEO.Format" );

    GUCEF_TESTFW_TESTCASE( "Format: Frame times follow the frame rate fraction without drifting" )
    try
    {
        // 15.0015 frames per second, the rate of a video with 6666 units of 10 microseconds per frame
        VIDEO::CVideoFormat format( 320, 200, IMAGE::PSF_PALETTE_INDICES, 100000, 6666, 1235 );
        ASSERT_TRUE( format.IsValid() && format.IsPalettized() && 1 == format.GetBytesPerPixel() );
        ASSERT_TRUE( 0 == format.GetFrameTimeInMicroSeconds( 0 ) );
        ASSERT_TRUE( 66660 == format.GetFrameTimeInMicroSeconds( 1 ) );
        ASSERT_TRUE( 82325100 == format.GetFrameTimeInMicroSeconds( 1235 ) );          // no rounding error added up over the frames
        ASSERT_TRUE( 82325 == format.GetDurationInMs() );
        ASSERT_TRUE( 0 == format.GetFrameAtTime( 66659 ) && 1 == format.GetFrameAtTime( 66660 ) );
        ASSERT_TRUE( 1234 == format.GetFrameAtTime( 82325099 ) );
        for ( UInt64 frame=0; frame<2000; frame+=7 )
            ASSERT_TRUE( frame == format.GetFrameAtTime( format.GetFrameTimeInMicroSeconds( frame ) ) );

        VIDEO::CVideoFormat unknown( 640, 480, IMAGE::PSF_RGBA, 25, 1 );
        ASSERT_TRUE( GUCEF_VIDEO_FRAMECOUNT_UNKNOWN == unknown.GetFrameCount() && 0 == unknown.GetDurationInMs() );
        ASSERT_TRUE( 4 == unknown.GetBytesPerPixel() && !unknown.IsPalettized() );
        ASSERT_TRUE( 40000 == unknown.GetFrameTimeInMicroSeconds( 1 ) );
        ASSERT_TRUE( unknown != format );
        ASSERT_TRUE( unknown == VIDEO::CVideoFormat( 640, 480, IMAGE::PSF_RGBA, 25, 1 ) );

        // what a video cannot be
        ASSERT_FALSE( VIDEO::CVideoFormat().IsValid() );
        ASSERT_FALSE( VIDEO::CVideoFormat( 0, 200, IMAGE::PSF_RGB, 25, 1 ).IsValid() );
        ASSERT_FALSE( VIDEO::CVideoFormat( 320, 200, IMAGE::PSF_RGB, 0, 1 ).IsValid() );
        ASSERT_FALSE( VIDEO::CVideoFormat( 320, 200, IMAGE::PSF_HSV, 25, 1 ).IsValid() );
        ASSERT_TRUE( 1 == VIDEO::CVideoFormat( 320, 200, IMAGE::PSF_RGB, 25, 0 ).GetFrameRateDenominator() );      // a 0 denominator is taken as 1
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frame: The picture is made once and kept while the format stays the same" )
    try
    {
        VIDEO::CVideoFrame frame;
        ASSERT_FALSE( frame.HasPicture() );
        ASSERT_TRUE( 0 == frame.GetWidth() && 0 == frame.GetPitch() );
        ASSERT_FALSE( frame.Prepare( VIDEO::CVideoFormat() ) );

        VIDEO::CVideoFormat format( 8, 4, IMAGE::PSF_PALETTE_INDICES, 10, 1 );
        ASSERT_TRUE( frame.Prepare( format ) && frame.HasPicture() );
        ASSERT_TRUE( 8 == frame.GetWidth() && 4 == frame.GetHeight() && 8 == frame.GetPitch() );
        ASSERT_TRUE( IMAGE::PSF_PALETTE_INDICES == frame.GetPixelFormat() );
        const void* memory = frame.GetPicture().GetDataPtr();
        static_cast< UInt8* >( frame.GetPicture().GetDataPtr() )[ 5 ] = 77;
        ASSERT_TRUE( frame.Prepare( format ) );
        ASSERT_TRUE( memory == frame.GetPicture().GetDataPtr() );                      // the same memory, the picture is kept
        ASSERT_TRUE( 77 == static_cast< const UInt8* >( frame.GetPicture().GetDataPtr() )[ 5 ] );

        ASSERT_TRUE( frame.Prepare( VIDEO::CVideoFormat( 8, 4, IMAGE::PSF_RGB, 10, 1 ) ) );
        ASSERT_TRUE( 24 == frame.GetPitch() && IMAGE::PSF_RGB == frame.GetPixelFormat() );

        // a copy is a copy, not a shared picture
        frame.SetFrameIndex( 3 );
        frame.SetTimeInMicroSeconds( 300000 );
        frame.SetPaletteChanged( true );
        VIDEO::CVideoFrame copy( frame );
        ASSERT_TRUE( 3 == copy.GetFrameIndex() && 300000 == copy.GetTimeInMicroSeconds() && copy.IsPaletteChanged() );
        ASSERT_TRUE( copy.GetPicture().GetDataPtr() != frame.GetPicture().GetDataPtr() );
        static_cast< UInt8* >( copy.GetPicture().GetDataPtr() )[ 0 ] = 1;
        ASSERT_TRUE( 0 == static_cast< const UInt8* >( frame.GetPicture().GetDataPtr() )[ 0 ] );

        // assignment into a frame of the same shape keeps that frame's memory
        VIDEO::CVideoFrame target;
        ASSERT_TRUE( target.Prepare( VIDEO::CVideoFormat( 8, 4, IMAGE::PSF_RGB, 10, 1 ) ) );
        const void* targetMemory = target.GetPicture().GetDataPtr();
        target = copy;
        ASSERT_TRUE( targetMemory == target.GetPicture().GetDataPtr() );
        ASSERT_TRUE( 1 == static_cast< const UInt8* >( target.GetPicture().GetDataPtr() )[ 0 ] && 3 == target.GetFrameIndex() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frame: Pictures become 32 bit RGBA or BGRA, with the palette applied" )
    try
    {
        VIDEO::CVideoFrame frame;
        ASSERT_TRUE( frame.Prepare( VIDEO::CVideoFormat( 3, 2, IMAGE::PSF_PALETTE_INDICES, 10, 1 ) ) );
        UInt8* indices = static_cast< UInt8* >( frame.GetPicture().GetDataPtr() );
        const UInt8 picture[ 6 ] = { 0, 1, 2, 255, 1, 0 };
        memcpy( indices, picture, 6 );
        UInt8* palette = frame.GetPalette();
        palette[ 0 ] = 10;  palette[ 1 ] = 20;  palette[ 2 ] = 30;
        palette[ 3 ] = 40;  palette[ 4 ] = 50;  palette[ 5 ] = 60;
        palette[ 6 ] = 70;  palette[ 7 ] = 80;  palette[ 8 ] = 90;
        palette[ 765 ] = 1; palette[ 766 ] = 2; palette[ 767 ] = 3;

        UInt8 rgba[ 2 * 16 ];
        memset( rgba, 0xEE, sizeof( rgba ) );
        ASSERT_TRUE( frame.ConvertTo32Bit( IMAGE::PSF_RGBA, rgba, 16 ) );         // a pitch wider than the line
        const UInt8 line0[ 12 ] = { 10, 20, 30, 255, 40, 50, 60, 255, 70, 80, 90, 255 };
        const UInt8 line1[ 12 ] = { 1, 2, 3, 255, 40, 50, 60, 255, 10, 20, 30, 255 };
        ASSERT_TRUE( 0 == memcmp( rgba, line0, 12 ) && 0 == memcmp( rgba + 16, line1, 12 ) );
        ASSERT_TRUE( 0xEE == rgba[ 12 ] && 0xEE == rgba[ 15 ] );                  // the padding is not touched

        IMAGE::TPixelMapPtr bgra;
        ASSERT_TRUE( frame.ConvertTo32Bit( IMAGE::PSF_BGRA, bgra ) );
        ASSERT_FALSE( bgra.IsNULL() );
        ASSERT_TRUE( 3 == bgra->GetWidthInPixels() && 2 == bgra->GetHeightInPixels() && IMAGE::PSF_BGRA == bgra->GetPixelStorageFormat() );
        const UInt8* b = static_cast< const UInt8* >( bgra->GetDataPtr() );
        ASSERT_TRUE( 30 == b[ 0 ] && 20 == b[ 1 ] && 10 == b[ 2 ] && 255 == b[ 3 ] );
        const IMAGE::CPixelMap* made = bgra.GetPointerAlways();
        ASSERT_TRUE( frame.ConvertTo32Bit( IMAGE::PSF_BGRA, bgra ) );
        ASSERT_TRUE( made == bgra.GetPointerAlways() );                           // the same pixel map is reused

        // RGB and the other 32 bit order
        VIDEO::CVideoFrame rgb;
        ASSERT_TRUE( rgb.Prepare( VIDEO::CVideoFormat( 1, 1, IMAGE::PSF_RGB, 10, 1 ) ) );
        UInt8* p = static_cast< UInt8* >( rgb.GetPicture().GetDataPtr() );
        p[ 0 ] = 1; p[ 1 ] = 2; p[ 2 ] = 3;
        UInt8 out[ 4 ];
        ASSERT_TRUE( rgb.ConvertTo32Bit( IMAGE::PSF_BGRA, out, 4 ) && 3 == out[ 0 ] && 2 == out[ 1 ] && 1 == out[ 2 ] && 255 == out[ 3 ] );

        VIDEO::CVideoFrame rgbaFrame;
        ASSERT_TRUE( rgbaFrame.Prepare( VIDEO::CVideoFormat( 1, 1, IMAGE::PSF_RGBA, 10, 1 ) ) );
        p = static_cast< UInt8* >( rgbaFrame.GetPicture().GetDataPtr() );
        p[ 0 ] = 1; p[ 1 ] = 2; p[ 2 ] = 3; p[ 3 ] = 4;
        ASSERT_TRUE( rgbaFrame.ConvertTo32Bit( IMAGE::PSF_BGRA, out, 4 ) && 3 == out[ 0 ] && 2 == out[ 1 ] && 1 == out[ 2 ] && 4 == out[ 3 ] );
        ASSERT_TRUE( rgbaFrame.ConvertTo32Bit( IMAGE::PSF_RGBA, out, 4 ) && 1 == out[ 0 ] && 4 == out[ 3 ] );

        // what cannot be done
        ASSERT_FALSE( frame.ConvertTo32Bit( IMAGE::PSF_RGB, rgba, 16 ) );
        ASSERT_FALSE( frame.ConvertTo32Bit( IMAGE::PSF_RGBA, rgba, 11 ) );        // a pitch too small for a line
        ASSERT_FALSE( VIDEO::CVideoFrame().ConvertTo32Bit( IMAGE::PSF_RGBA, rgba, 16 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
