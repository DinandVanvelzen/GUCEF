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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestSignals.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< Float32 > TSamples;

/** Reads a whole (or the first maxFrames of a) source, interleaved samples */
TSamples
ReadSource( AUDIO::IAudioSource& source, UInt32 maxFrames = 0xFFFFFFFFU )
{
    const UInt32 channels = source.GetFormat().GetChannelCount();
    TSamples samples;
    TSamples piece( 1000 * channels );
    UInt32 frames = 0;
    while ( !source.IsEndOfStream() && frames < maxFrames )
    {
        const UInt32 want = maxFrames - frames < 1000 ? maxFrames - frames : 1000;
        const UInt32 got = source.ReadFrames( &piece[ 0 ], want );
        if ( 0 == got )
            break;
        samples.insert( samples.end(), piece.begin(), piece.begin() + got * channels );
        frames += got;
    }
    return samples;
}

/** Times the signal of one channel goes from below zero to zero or above, between two frames */
UInt32
RisingCrossings( const TSamples& samples, UInt32 channels, UInt32 channel, size_t fromFrame, size_t toFrame )
{
    UInt32 count = 0;
    for ( size_t f=fromFrame+1; f<toFrame && ( f * channels + channel ) < samples.size(); ++f )
        if ( samples[ ( f-1 ) * channels + channel ] < 0.0f && samples[ f * channels + channel ] >= 0.0f )
            ++count;
    return count;
}

Float32
Peak( const TSamples& samples, UInt32 channels, UInt32 channel, size_t fromFrame, size_t toFrame )
{
    Float32 peak = 0.0f;
    for ( size_t f=fromFrame; f<toFrame && ( f * channels + channel ) < samples.size(); ++f )
    {
        const Float32 v = samples[ f * channels + channel ] < 0.0f ? -samples[ f * channels + channel ] : samples[ f * channels + channel ];
        if ( v > peak )
            peak = v;
    }
    return peak;
}

bool
Within( UInt32 value, UInt32 expected, UInt32 margin )
{
    return value + margin >= expected && value <= expected + margin;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioTestSignalTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioTestSignals TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.TestSignals" );

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: A tone has the asked pitch, level, length and format" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr tone = AUDIO::CAudioTestSignals::MakeTone( 48000, 2, 1000.0, 1000, 0.5f );
        ASSERT_TRUE( tone->GetFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( 48000 == tone->GetLengthInFrames() );
        const TSamples samples = ReadSource( *tone );
        ASSERT_TRUE( 2 * 48000 == samples.size() );
        ASSERT_TRUE( Within( RisingCrossings( samples, 2, 0, 0, 48000 ), 1000, 2 ) );
        ASSERT_TRUE( Within( RisingCrossings( samples, 2, 1, 0, 48000 ), 1000, 2 ) );
        const Float32 peak = Peak( samples, 2, 0, 1000, 47000 );
        ASSERT_TRUE( peak > 0.49f && peak <= 0.5f );

        // the fades: silent at both edges
        ASSERT_TRUE( 0.0f == samples[ 0 ] && Peak( samples, 2, 0, 47999, 48000 ) < 0.001f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: The tone ladder goes up by octaves from 110 Hz with pauses in between" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr ladder = AUDIO::CAudioTestSignals::MakeToneLadder( 48000, 1, 400 );
        AUDIO::CToneSequenceSource* sequence = static_cast< AUDIO::CToneSequenceSource* >( ladder.GetPointerAlways() );
        ASSERT_TRUE( 14 == sequence->GetSegments().size() );                    // 110 to 7040 Hz: 7 tones, each with a pause
        const TSamples samples = ReadSource( *ladder );

        Float64 frequency = 110.0;
        for ( UInt32 step=0; step<7; ++step, frequency *= 2.0 )
        {
            const size_t start = (size_t) step * ( 19200 + 4800 );               // 400 ms of tone and 100 ms of pause
            ASSERT_TRUE( Within( RisingCrossings( samples, 1, 0, start, start + 19200 ), (UInt32) ( frequency * 0.4 ), 2 ) );
            ASSERT_TRUE( 0.0f == Peak( samples, 1, 0, start + 19200, start + 24000 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: The channel check sounds on one channel at a time, in channel order" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr check = AUDIO::CAudioTestSignals::MakeChannelCheck( 48000, 2 );
        const TSamples samples = ReadSource( *check );
        const size_t group = 3 * 12000 + 24000;                                 // three 250 ms notes and a 500 ms pause
        ASSERT_TRUE( 2 * 2 * group == samples.size() );

        // first only the left channel, then only the right
        ASSERT_TRUE( Peak( samples, 2, 0, 0, 3 * 12000 ) > 0.3f && 0.0f == Peak( samples, 2, 1, 0, 3 * 12000 ) );
        ASSERT_TRUE( 0.0f == Peak( samples, 2, 0, group, group + 3 * 12000 ) && Peak( samples, 2, 1, group, group + 3 * 12000 ) > 0.3f );
        ASSERT_TRUE( Within( RisingCrossings( samples, 2, 0, 0, 12000 ), 110, 2 ) );          // 440 Hz for 250 ms
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: The sweep rises, the noise stays within its level, both are the same every time" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr sweep = AUDIO::CAudioTestSignals::MakeSweep( 48000, 1, 4000 );
        const TSamples swept = ReadSource( *sweep );
        const UInt32 early = RisingCrossings( swept, 1, 0, 0, 48000 );
        const UInt32 late = RisingCrossings( swept, 1, 0, 3 * 48000, 4 * 48000 );
        ASSERT_TRUE( early > 10 && late > 50 * early );

        AUDIO::IAudioSource::TAudioSourcePtr noise = AUDIO::CAudioTestSignals::MakeNoise( 48000, 1, 1000, 0.2f );
        const TSamples first = ReadSource( *noise );
        Float64 sum = 0.0;
        for ( size_t i=0; i<first.size(); ++i )
            sum += first[ i ];
        ASSERT_TRUE( Peak( first, 1, 0, 0, first.size() ) <= 0.2f );
        ASSERT_TRUE( sum / first.size() < 0.01 && sum / first.size() > -0.01 );
        ASSERT_TRUE( noise->Rewind() );
        ASSERT_TRUE( first == ReadSource( *noise ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: Seeking and looping give exactly the frames of a straight read" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr ladder = AUDIO::CAudioTestSignals::MakeToneLadder( 8000, 1, 100 );
        const TSamples all = ReadSource( *ladder );
        ASSERT_TRUE( ladder->SeekToFrame( 1234 ) );
        const TSamples part = ReadSource( *ladder, 500 );
        ASSERT_TRUE( 500 == part.size() && TSamples( all.begin() + 1234, all.begin() + 1734 ) == part );
        ASSERT_FALSE( ladder->SeekToFrame( all.size() + 1 ) );

        AUDIO::CToneSequenceSource::TSegmentVector segments;
        segments.push_back( AUDIO::CToneSequenceSource::SSegment( AUDIO::CToneSequenceSource::WAVEFORM_TRIANGLE, 250.0, 100 ) );
        AUDIO::CToneSequenceSource looped( 8000, 1, segments, 0 );
        ASSERT_TRUE( GUCEF_AUDIO_LENGTH_UNKNOWN == looped.GetLengthInFrames() );
        const TSamples many = ReadSource( looped, 8000 );
        ASSERT_FALSE( looped.IsEndOfStream() );
        ASSERT_TRUE( TSamples( many.begin(), many.begin() + 800 ) == TSamples( many.begin() + 4000, many.begin() + 4800 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
