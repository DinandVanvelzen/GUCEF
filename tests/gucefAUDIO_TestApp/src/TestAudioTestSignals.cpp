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

static void
PerformDecayAndChimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: A segment decays as exp( -t / decay ) from the end of its fade in, within 1e-4, with the fades at both edges" )
    try
    {
        const UInt32 rate = 48000;
        AUDIO::CToneSequenceSource::SSegment segment( AUDIO::CToneSequenceSource::WAVEFORM_SINE, 997.0, 500, 0.8f );
        ASSERT_TRUE( 0 == segment.decayInMs );
        segment.decayInMs = 100;
        AUDIO::CToneSequenceSource::SSegment square( AUDIO::CToneSequenceSource::WAVEFORM_SQUARE, 750.0, 500, 0.8f );
        square.decayInMs = 100;
        square.fadeMs = 20;

        AUDIO::CToneSequenceSource::TSegmentVector segments;
        segments.push_back( segment );
        segments.push_back( square );
        AUDIO::CToneSequenceSource source( rate, 1, segments );
        const TSamples samples = ReadSource( source );
        ASSERT_TRUE( 2 * 24000 == samples.size() );

        for ( UInt32 s=0; s<2; ++s )
        {
            const UInt64 length = 24000;
            const UInt64 fade = ( (UInt64) ( 0 == s ? 5 : 20 ) * rate ) / 1000;
            for ( UInt64 f=0; f<length; ++f )
            {
                Float64 envelope = 1.0;
                if ( f < fade )
                    envelope = 0.5 - 0.5 * cos( 3.14159265358979323846 * (Float64) f / (Float64) fade );
                else
                if ( f >= length - fade )
                    envelope = 0.5 - 0.5 * cos( 3.14159265358979323846 * (Float64) ( length - 1 - f ) / (Float64) fade );
                if ( f > fade )
                    envelope *= exp( -( (Float64) ( f - fade ) / rate ) / 0.1 );
                const Float32 sample = samples[ (size_t) ( s * length + f ) ];
                if ( 0 == s )
                {
                    const Float64 expected = 0.8 * envelope * sin( 2.0 * 3.14159265358979323846 * 997.0 * (Float64) f / rate );
                    ASSERT_TRUE( fabs( (Float64) sample - expected ) < 1.0e-4 );
                }
                else
                {
                    ASSERT_TRUE( fabs( fabs( (Float64) sample ) - 0.8 * envelope ) < 1.0e-4 );
                }
            }
        }

        // the envelope really falls: 100 ms after the fade in the level is 1/e of the start
        const Float32 early = Peak( samples, 1, 0, 240, 480 );
        const Float32 later = Peak( samples, 1, 0, 240 + 4800, 480 + 4800 );
        ASSERT_TRUE( fabs( (Float64) later / (Float64) early - exp( -1.0 ) ) < 0.03 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: A segment without decay is unchanged: the same frames as the formula of a flat segment, for every waveform" )
    try
    {
        const AUDIO::CToneSequenceSource::TWaveform waveforms[] = {
            AUDIO::CToneSequenceSource::WAVEFORM_SINE, AUDIO::CToneSequenceSource::WAVEFORM_SQUARE, AUDIO::CToneSequenceSource::WAVEFORM_TRIANGLE,
            AUDIO::CToneSequenceSource::WAVEFORM_SAWTOOTH, AUDIO::CToneSequenceSource::WAVEFORM_SWEEP, AUDIO::CToneSequenceSource::WAVEFORM_WHITE_NOISE };
        for ( UInt32 w=0; w<6; ++w )
        {
            AUDIO::CToneSequenceSource::SSegment plain( waveforms[ w ], 440.0, 120, 0.6f );
            plain.endFrequency = 4000.0;
            AUDIO::CToneSequenceSource::SSegment explicitZero = plain;
            explicitZero.decayInMs = 0;
            ASSERT_TRUE( 0 == plain.decayInMs );

            AUDIO::CToneSequenceSource::TSegmentVector a;
            a.push_back( plain );
            AUDIO::CToneSequenceSource::TSegmentVector b;
            b.push_back( explicitZero );
            AUDIO::CToneSequenceSource first( 48000, 2, a );
            AUDIO::CToneSequenceSource second( 48000, 2, b );
            const TSamples x = ReadSource( first );
            const TSamples y = ReadSource( second );
            ASSERT_TRUE( 2 * 5760 == x.size() );
            ASSERT_TRUE( x == y );
        }

        // a flat sine segment equals the formula of the sine with its raised cosine fades, bit for bit
        AUDIO::CToneSequenceSource::TSegmentVector flat;
        flat.push_back( AUDIO::CToneSequenceSource::SSegment( AUDIO::CToneSequenceSource::WAVEFORM_SINE, 440.0, 120, 0.6f ) );
        AUDIO::CToneSequenceSource source( 48000, 1, flat );
        const TSamples samples = ReadSource( source );
        const UInt64 length = 5760;
        const UInt64 fade = 240;
        for ( UInt64 f=0; f<length; ++f )
        {
            Float64 envelope = 1.0;
            if ( f < fade )
                envelope = 0.5 - 0.5 * cos( 3.14159265358979323846 * (Float64) f / (Float64) fade );
            else
            if ( f >= length - fade )
                envelope = 0.5 - 0.5 * cos( 3.14159265358979323846 * (Float64) ( length - 1 - f ) / (Float64) fade );
            const Float64 cycles = 440.0 * ( (Float64) f / 48000.0 );
            const Float64 value = sin( 2.0 * 3.14159265358979323846 * ( cycles - floor( cycles ) ) );
            ASSERT_TRUE( samples[ (size_t) f ] == (Float32) ( value * envelope * 0.6f ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioTestSignals: MakeChime is two decaying tones, 784 Hz and 659 Hz, 350 ms each: length, format, levels, pitch, same on every channel" )
    try
    {
        AUDIO::IAudioSource::TAudioSourcePtr chime = AUDIO::CToneSequenceSource::MakeChime( 48000, 2 );
        ASSERT_TRUE( chime->GetFormat() == AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( 33600 == chime->GetLengthInFrames() );
        const TSamples samples = ReadSource( *chime );
        ASSERT_TRUE( 2 * 33600 == samples.size() );
        ASSERT_TRUE( chime->IsEndOfStream() );
        for ( size_t f=0; f<33600; ++f )
            ASSERT_TRUE( samples[ 2 * f ] == samples[ 2 * f + 1 ] );

        // pitch of each note
        ASSERT_TRUE( Within( RisingCrossings( samples, 2, 0, 0, 16800 ), 274, 2 ) );
        ASSERT_TRUE( Within( RisingCrossings( samples, 2, 0, 16800, 33600 ), 231, 2 ) );

        // level: the first note starts at the amplitude and has decayed by exp( -t / 250 ms) 300 ms in; the second starts again
        const Float32 start = Peak( samples, 2, 0, 240, 240 + 120 );
        ASSERT_TRUE( start > 0.38f && start <= 0.4f );
        const Float32 decayed = Peak( samples, 2, 0, 14400, 14400 + 120 );
        ASSERT_TRUE( fabs( (Float64) decayed - 0.4 * exp( -( 14400.0 - 240.0 ) / 48000.0 / 0.25 ) ) < 0.01 );
        const Float32 second = Peak( samples, 2, 0, 16800 + 240, 16800 + 240 + 120 );
        ASSERT_TRUE( second > 0.38f && second <= 0.4f );

        // silent at the start and the end, and no click on the way: no step larger than the steepest slope of the louder tone
        ASSERT_TRUE( 0.0f == samples[ 0 ] && 0.0f == samples[ 2 * 33599 ] );
        Float32 largestStep = 0.0f;
        for ( size_t f=1; f<33600; ++f )
            if ( fabsf( samples[ 2 * f ] - samples[ 2 * ( f - 1 ) ] ) > largestStep )
                largestStep = fabsf( samples[ 2 * f ] - samples[ 2 * ( f - 1 ) ] );
        ASSERT_TRUE( largestStep < 0.4f * 2.0f * 3.14159265f * 784.0f / 48000.0f * 1.02f );
        ASSERT_TRUE( fabsf( samples[ 2 * 16800 ] ) < 0.005f && fabsf( samples[ 2 * 16799 ] ) < 0.005f );     // the join of the notes

        // other rates and channel counts
        AUDIO::IAudioSource::TAudioSourcePtr mono = AUDIO::CToneSequenceSource::MakeChime( 16000, 1 );
        ASSERT_TRUE( mono->GetFormat() == AUDIO::CAudioFormat( 16000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
        ASSERT_TRUE( 11200 == mono->GetLengthInFrames() );
        ASSERT_TRUE( 11200 == ReadSource( *mono ).size() );

        // seeking gives the same frames as reading straight on; the same chime is made every time
        AUDIO::IAudioSource::TAudioSourcePtr other = AUDIO::CToneSequenceSource::MakeChime( 48000, 2 );
        ASSERT_TRUE( other->SeekToFrame( 20000 ) );
        TSamples tail( 2 * 100 );
        ASSERT_TRUE( 100 == other->ReadFrames( &tail[ 0 ], 100 ) );
        ASSERT_TRUE( tail == TSamples( samples.begin() + 2 * 20000, samples.begin() + 2 * 20100 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

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

    PerformDecayAndChimeTests();
}

/*-------------------------------------------------------------------------*/
