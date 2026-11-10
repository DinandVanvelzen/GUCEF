/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioFormat.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Near( Float32 a, Float32 b, Float32 tolerance = 1e-6f )
{
    return fabsf( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static void
PerformFormatTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioFormat: Default is invalid, a set up format is valid" )
    try
    {
        AUDIO::CAudioFormat none;
        ASSERT_FALSE( none.IsValid() );
        ASSERT_TRUE( 0 == none.GetBytesPerFrame() );

        AUDIO::CAudioFormat cd( 44100, 2, AUDIO::SAMPLETYPE_INT16 );
        ASSERT_TRUE( cd.IsValid() );
        ASSERT_TRUE( 44100 == cd.GetSampleRate() );
        ASSERT_TRUE( 2 == cd.GetChannelCount() );
        ASSERT_TRUE( 2 == cd.GetBytesPerSample() );
        ASSERT_TRUE( 4 == cd.GetBytesPerFrame() );
        ASSERT_FALSE( cd.HasExplicitChannelLayout() );
        ASSERT_TRUE( ( AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT ) == cd.GetChannelLayout() );

        ASSERT_FALSE( AUDIO::CAudioFormat( 0, 2, AUDIO::SAMPLETYPE_INT16 ).IsValid() );
        ASSERT_FALSE( AUDIO::CAudioFormat( 44100, 0, AUDIO::SAMPLETYPE_INT16 ).IsValid() );
        ASSERT_FALSE( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_UNKNOWN ).IsValid() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFormat: Sample sizes and names" )
    try
    {
        ASSERT_TRUE( 1 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_UINT8 ) );
        ASSERT_TRUE( 2 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( 3 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_INT24 ) );
        ASSERT_TRUE( 4 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_INT32 ) );
        ASSERT_TRUE( 4 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( 8 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_FLOAT64 ) );
        ASSERT_TRUE( 0 == AUDIO::CAudioFormat::GetBytesPerSample( AUDIO::SAMPLETYPE_UNKNOWN ) );
        ASSERT_TRUE( 0 == strcmp( "float32", AUDIO::CAudioFormat::SampleTypeToString( AUDIO::SAMPLETYPE_FLOAT32 ) ) );
        ASSERT_TRUE( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_INT24 ).GetBytesPerFrame() == 18 );
        ASSERT_TRUE( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16 ).ToString() == "44100 Hz, 2 ch, int16" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFormat: Usual speaker layouts per channel count" )
    try
    {
        ASSERT_TRUE( 0 == AUDIO::CAudioFormat::GetDefaultChannelLayout( 0 ) );
        ASSERT_TRUE( AUDIO::AUDIOSPEAKER_FRONT_CENTER == AUDIO::CAudioFormat::GetDefaultChannelLayout( 1 ) );
        ASSERT_TRUE( 0x3 == AUDIO::CAudioFormat::GetDefaultChannelLayout( 2 ) );
        ASSERT_TRUE( 0x33 == AUDIO::CAudioFormat::GetDefaultChannelLayout( 4 ) );
        ASSERT_TRUE( 0x3F == AUDIO::CAudioFormat::GetDefaultChannelLayout( 6 ) );       // 5.1
        ASSERT_TRUE( 0x63F == AUDIO::CAudioFormat::GetDefaultChannelLayout( 8 ) );      // 7.1
        for ( UInt32 n=1; n<=12; ++n )
            ASSERT_TRUE( n == AUDIO::CAudioFormat::GetChannelCountOfLayout( AUDIO::CAudioFormat::GetDefaultChannelLayout( n ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFormat: Explicit layout sets the channel count, changing the count drops it" )
    try
    {
        AUDIO::CAudioFormat format( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );
        format.SetChannelLayout( AUDIO::AUDIOSPEAKER_FRONT_LEFT | AUDIO::AUDIOSPEAKER_FRONT_RIGHT | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY );
        ASSERT_TRUE( format.HasExplicitChannelLayout() );
        ASSERT_TRUE( 3 == format.GetChannelCount() );
        format.SetChannelCount( 3 );                        // consistent, layout stays
        ASSERT_TRUE( format.HasExplicitChannelLayout() );
        format.SetChannelCount( 2 );                        // no longer matches
        ASSERT_FALSE( format.HasExplicitChannelLayout() );
        ASSERT_TRUE( 0x3 == format.GetChannelLayout() );

        AUDIO::CAudioFormat constructed( 48000, 0, AUDIO::SAMPLETYPE_INT16, AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY );
        ASSERT_TRUE( 2 == constructed.GetChannelCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFormat: Equality compares everything including the speakers" )
    try
    {
        AUDIO::CAudioFormat a( 44100, 2, AUDIO::SAMPLETYPE_INT16 );
        AUDIO::CAudioFormat b( 44100, 2, AUDIO::SAMPLETYPE_INT16, 0x3 );   // explicit but the same speakers
        ASSERT_TRUE( a == b );
        ASSERT_FALSE( a != b );
        ASSERT_TRUE( a != AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( a != AUDIO::CAudioFormat( 44100, 1, AUDIO::SAMPLETYPE_INT16 ) );
        ASSERT_TRUE( a != AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( a != AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_INT16, AUDIO::AUDIOSPEAKER_FRONT_CENTER | AUDIO::AUDIOSPEAKER_LOW_FREQUENCY ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBufferTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: New buffers are silent in every sample type" )
    try
    {
        const AUDIO::TSampleType types[ 6 ] = { AUDIO::SAMPLETYPE_UINT8, AUDIO::SAMPLETYPE_INT16, AUDIO::SAMPLETYPE_INT24, AUDIO::SAMPLETYPE_INT32, AUDIO::SAMPLETYPE_FLOAT32, AUDIO::SAMPLETYPE_FLOAT64 };
        for ( UInt32 t=0; t<6; ++t )
        {
            AUDIO::CAudioBuffer buffer( AUDIO::CAudioFormat( 44100, 2, types[ t ] ), 100 );
            ASSERT_TRUE( 100 == buffer.GetFrameCount() );
            ASSERT_TRUE( 100 * buffer.GetFormat().GetBytesPerFrame() == buffer.GetSizeInBytes() );
            for ( UInt32 f=0; f<100; f+=17 )
                for ( UInt32 c=0; c<2; ++c )
                    ASSERT_TRUE( Near( 0.0f, buffer.GetSampleAsFloat( f, c ) ) );
        }
        ASSERT_TRUE( 128 == *static_cast< const UInt8* >( AUDIO::CAudioBuffer( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_UINT8 ), 1 ).GetDataPtr() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Samples read and write as floats in every sample type" )
    try
    {
        const AUDIO::TSampleType types[ 6 ] = { AUDIO::SAMPLETYPE_UINT8, AUDIO::SAMPLETYPE_INT16, AUDIO::SAMPLETYPE_INT24, AUDIO::SAMPLETYPE_INT32, AUDIO::SAMPLETYPE_FLOAT32, AUDIO::SAMPLETYPE_FLOAT64 };
        const Float32 tolerances[ 6 ] = { 1.0f / 128.0f, 1.0f / 32768.0f, 1.0f / 8388608.0f, 1e-6f, 1e-7f, 1e-7f };
        for ( UInt32 t=0; t<6; ++t )
        {
            AUDIO::CAudioBuffer buffer( AUDIO::CAudioFormat( 44100, 2, types[ t ] ), 4 );
            buffer.SetSampleFromFloat( 1, 0, 0.5f );
            buffer.SetSampleFromFloat( 1, 1, -0.25f );
            buffer.SetSampleFromFloat( 3, 1, 0.75f );
            ASSERT_TRUE( Near( 0.5f, buffer.GetSampleAsFloat( 1, 0 ), tolerances[ t ] ) );
            ASSERT_TRUE( Near( -0.25f, buffer.GetSampleAsFloat( 1, 1 ), tolerances[ t ] ) );
            ASSERT_TRUE( Near( 0.75f, buffer.GetSampleAsFloat( 3, 1 ), tolerances[ t ] ) );
            ASSERT_TRUE( Near( 0.0f, buffer.GetSampleAsFloat( 0, 0 ), tolerances[ t ] ) );
            ASSERT_TRUE( Near( 0.0f, buffer.GetSampleAsFloat( 99, 0 ) ) );     // outside is silence, not a crash
            buffer.SetSampleFromFloat( 99, 0, 1.0f );
            buffer.SetSampleFromFloat( 0, 9, 1.0f );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Resize keeps the audio and makes new frames silent" )
    try
    {
        AUDIO::CAudioBuffer buffer( AUDIO::CAudioFormat( 8000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 4 );
        for ( UInt32 f=0; f<4; ++f )
            buffer.SetSampleFromFloat( f, 0, 0.1f * (Float32) ( f + 1 ) );
        ASSERT_TRUE( buffer.Resize( 10 ) );
        ASSERT_TRUE( 10 == buffer.GetFrameCount() );
        ASSERT_TRUE( Near( 0.3f, buffer.GetSampleAsFloat( 2, 0 ) ) );
        ASSERT_TRUE( Near( 0.0f, buffer.GetSampleAsFloat( 7, 0 ) ) );
        ASSERT_TRUE( buffer.Resize( 2 ) );
        ASSERT_TRUE( Near( 0.2f, buffer.GetSampleAsFloat( 1, 0 ) ) );
        ASSERT_TRUE( buffer.Resize( 0 ) );
        ASSERT_TRUE( buffer.IsEmpty() );

        AUDIO::CAudioBuffer plain;
        ASSERT_TRUE( plain.Resize( 0 ) );
        ASSERT_FALSE( plain.Resize( 5 ) );          // no format, no frame size
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBuffer: Append, copy, silence and duration" )
    try
    {
        const AUDIO::CAudioFormat format( 10, 2, AUDIO::SAMPLETYPE_INT16 );
        AUDIO::CAudioBuffer a( format, 5 ), b( format, 3 );
        a.SetSampleFromFloat( 4, 1, 0.5f );
        b.SetSampleFromFloat( 0, 0, -0.5f );
        ASSERT_TRUE( a.Append( b ) );
        ASSERT_TRUE( 8 == a.GetFrameCount() );
        ASSERT_TRUE( Near( 0.5f, a.GetSampleAsFloat( 4, 1 ), 1e-4f ) );
        ASSERT_TRUE( Near( -0.5f, a.GetSampleAsFloat( 5, 0 ), 1e-4f ) );
        ASSERT_FALSE( a.Append( AUDIO::CAudioBuffer( AUDIO::CAudioFormat( 10, 1, AUDIO::SAMPLETYPE_INT16 ), 2 ) ) );   // other format
        ASSERT_TRUE( Near( 0.8f, (Float32) a.GetDurationInSeconds() ) );

        AUDIO::CAudioBuffer copy( a );
        AUDIO::CAudioBuffer assigned;
        assigned = a;
        ASSERT_TRUE( copy.GetFrameCount() == 8 && assigned.GetFrameCount() == 8 );
        ASSERT_TRUE( Near( 0.5f, copy.GetSampleAsFloat( 4, 1 ), 1e-4f ) );
        a.FillSilence();
        ASSERT_TRUE( Near( 0.0f, a.GetSampleAsFloat( 4, 1 ) ) );
        ASSERT_TRUE( Near( 0.5f, assigned.GetSampleAsFloat( 4, 1 ), 1e-4f ) );     // the copies are independent

        AUDIO::CAudioBuffer fromData( AUDIO::CAudioFormat( 100, 1, AUDIO::SAMPLETYPE_INT16 ), "\x00\x40\x00\xC0", 2 );
        ASSERT_TRUE( Near( 0.5f, fromData.GetSampleAsFloat( 0, 0 ) ) );
        ASSERT_TRUE( Near( -0.5f, fromData.GetSampleAsFloat( 1, 0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioFormatTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioFormat TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Format" );

    PerformFormatTests();
    PerformBufferTests();
}

/*-------------------------------------------------------------------------*/
