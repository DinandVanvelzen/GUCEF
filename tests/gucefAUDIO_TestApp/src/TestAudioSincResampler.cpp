/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CSINCRESAMPLER_H
#include "gucefAUDIO_CSincResampler.h"
#define GUCEF_AUDIO_CSINCRESAMPLER_H
#endif /* GUCEF_AUDIO_CSINCRESAMPLER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioSincResampler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace AudioEffectTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The value with two decimals, for the log */
CORE::CString
Fixed( Float64 value, UInt32 decimals = 2 )
{
    Float64 scale = 1.0;
    for ( UInt32 i=0; i<decimals; ++i )
        scale *= 10.0;
    const bool negative = value < 0.0;
    const Int64 scaled = (Int64) floor( fabs( value ) * scale + 0.5 );
    CORE::CString text = CORE::Int64ToString( scaled / (Int64) scale );
    if ( decimals > 0 )
    {
        CORE::CString fraction = CORE::Int64ToString( scaled % (Int64) scale );
        while ( fraction.Length() < decimals )
            fraction = "0" + fraction;
        text += "." + fraction;
    }
    return negative ? "-" + text : text;
}

/*-------------------------------------------------------------------------*/

/** One call of the resampler, marked as a real time context in a RealtimeSanitizer build */
void
ProcessRealtime( AUDIO::CSincResampler& resampler, const Float32* input, UInt32 inputFrames, UInt32& consumed, Float32* output, UInt32 capacity, UInt32& produced ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    resampler.Process( input, inputFrames, consumed, output, capacity, produced );
}

bool
SetRatesRealtime( AUDIO::CSincResampler& resampler, Float64 inputRate, Float64 outputRate ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return resampler.SetRates( inputRate, outputRate );
}

/*-------------------------------------------------------------------------*/

/** Sets the ratio as a number of input frames per output frame (input rate 'step', output rate 1) */
bool
SetStep( AUDIO::CSincResampler& resampler, Float64 step )
{
    return SetRatesRealtime( resampler, step, 1.0 );
}

/** A resampler for 'channels' with the given input frames per output frame */
bool
Setup( AUDIO::CSincResampler& resampler, UInt32 channels, Float64 step )
{
    return resampler.Configure( channels, step, 1.0 );
}

/*-------------------------------------------------------------------------*/

/** Resamples all of the audio, offering 'inChunk' frames at a time and room for 'outChunk' frames of output */
TSamples
Resample( AUDIO::CSincResampler& resampler, const TSamples& input, UInt32 channels, UInt32 inChunk, UInt32 outChunk )
{
    TSamples result;
    TSamples out( (size_t) outChunk * channels );
    const size_t total = input.size() / channels;
    size_t offset = 0;
    while ( offset < total )
    {
        const UInt32 count = (UInt32) ( total - offset < inChunk ? total - offset : inChunk );
        UInt32 used = 0;
        UInt32 made = 0;
        ProcessRealtime( resampler, &input[ offset * channels ], count, used, &out[ 0 ], outChunk, made );
        result.insert( result.end(), out.begin(), out.begin() + (size_t) made * channels );
        offset += used;
        if ( 0 == used && 0 == made )
            break;
    }
    return result;
}

/** The same with chunk sizes drawn at random, 1 to the given maximums */
TSamples
ResampleRandom( AUDIO::CSincResampler& resampler, const TSamples& input, UInt32 channels, UInt32 maxIn, UInt32 maxOut, UInt32 seed )
{
    CXorShift random( seed );
    TSamples result;
    TSamples out( (size_t) maxOut * channels );
    const size_t total = input.size() / channels;
    size_t offset = 0;
    while ( offset < total )
    {
        UInt32 count = 1 + random.Next() % maxIn;
        if ( count > total - offset )
            count = (UInt32) ( total - offset );
        const UInt32 capacity = 1 + random.Next() % maxOut;
        UInt32 used = 0;
        UInt32 made = 0;
        ProcessRealtime( resampler, &input[ offset * channels ], count, used, &out[ 0 ], capacity, made );
        result.insert( result.end(), out.begin(), out.begin() + (size_t) made * channels );
        offset += used;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

struct SToneFit
{
    Float64 amplitude;
    Float64 residualRms;    /**< everything that is not that tone, as root mean square */
    SToneFit( void ) : amplitude( 0.0 ), residualRms( 0.0 ) {}
};

/** Least squares fit of a cosine and a sine of the given frequency to a mono signal: the amplitude of the tone and what is left */
SToneFit
FitTone( const TSamples& signal, size_t from, size_t to, Float64 cyclesPerSample )
{
    Float64 scc = 0.0, sss = 0.0, scs = 0.0, syc = 0.0, sys = 0.0, syy = 0.0;
    const Float64 w = 2.0 * Pi * cyclesPerSample;
    for ( size_t n=from; n<to; ++n )
    {
        const Float64 c = cos( w * (Float64) n );
        const Float64 s = sin( w * (Float64) n );
        const Float64 y = signal[ n ];
        scc += c * c; sss += s * s; scs += c * s; syc += y * c; sys += y * s; syy += y * y;
    }
    const Float64 det = scc * sss - scs * scs;
    SToneFit fit;
    if ( fabs( det ) < 1.0e-12 )
        return fit;
    const Float64 a = ( syc * sss - sys * scs ) / det;
    const Float64 b = ( sys * scc - syc * scs ) / det;
    fit.amplitude = sqrt( a * a + b * b );
    const Float64 residual = syy - ( a * syc + b * sys );
    fit.residualRms = sqrt( ( residual > 0.0 ? residual : 0.0 ) / (Float64) ( to - from ) );
    return fit;
}

/** Level of the part of the signal that is not the tone, in dB relative to the tone */
Float64
SpuriousDb( const TSamples& signal, size_t from, size_t to, Float64 cyclesPerSample )
{
    const SToneFit fit = FitTone( signal, from, to, cyclesPerSample );
    // the root mean square of a sine is its amplitude over the square root of two
    return DecibelsOf( fit.residualRms * 1.4142135623730951 / fit.amplitude + 1.0e-30 );
}

/** Frequency of a tone as folded back into 0 to half the rate */
Float64
Folded( Float64 frequency, Float64 rate )
{
    Float64 f = fmod( frequency, rate );
    if ( f > rate / 2.0 )
        f = rate - f;
    return f;
}

/** A mono sine, as a double precision computation of every sample */
TSamples
Sine( size_t frames, Float64 cyclesPerSample, Float32 amplitude )
{
    TSamples signal( frames );
    for ( size_t n=0; n<frames; ++n )
        signal[ n ] = amplitude * (Float32) sin( 2.0 * Pi * cyclesPerSample * (Float64) n );
    return signal;
}

/** Largest second difference of a mono signal */
Float64
MaxSecondDifference( const TSamples& signal, size_t from = 2 )
{
    Float64 largest = 0.0;
    for ( size_t n=from; n<signal.size(); ++n )
    {
        const Float64 d = fabs( (Float64) signal[ n ] - 2.0 * (Float64) signal[ n-1 ] + (Float64) signal[ n-2 ] );
        if ( d > largest )
            largest = d;
    }
    return largest;
}

Float64
MaxDifference( const TSamples& a, const TSamples& b )
{
    Float64 largest = 0.0;
    const size_t count = a.size() < b.size() ? a.size() : b.size();
    for ( size_t i=0; i<count; ++i )
    {
        const Float64 d = fabs( (Float64) a[ i ] - (Float64) b[ i ] );
        if ( d > largest )
            largest = d;
    }
    return largest;
}

/** The mono channel c of interleaved audio */
TSamples
Channel( const TSamples& audio, UInt32 channels, UInt32 c )
{
    TSamples mono( audio.size() / channels );
    for ( size_t n=0; n<mono.size(); ++n )
        mono[ n ] = audio[ n * channels + c ];
    return mono;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformResponseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SincResampler: 48 kHz to 16 kHz passes 4 kHz within 0.1 dB and rejects everything from 9 kHz up by at least 60 dB (tone grid)" )
    try
    {
        Float64 passbandWorst = 0.0;
        Float64 stopbandWorst = -1000.0;
        for ( Float64 f=200.0; f<23900.0; f+=400.0 )
        {
            if ( fabs( f - 16000.0 ) < 150.0 )
                continue;
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( resampler.Configure( 1, 48000.0, 16000.0 ) );
            const TSamples output = Resample( resampler, Sine( 14400, f / 48000.0, 0.5f ), 1, 480, 480 );
            ASSERT_TRUE( output.size() > 4500 );
            const SToneFit fit = FitTone( output, 100, output.size() - 100, Folded( f, 16000.0 ) / 16000.0 );
            const Float64 gainDb = DecibelsOf( fit.amplitude / 0.5 + 1.0e-30 );
            if ( f <= 5000.0 && fabs( gainDb ) > passbandWorst )
                passbandWorst = fabs( gainDb );
            if ( f >= 9000.0 && gainDb > stopbandWorst )
                stopbandWorst = gainDb;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: 48 kHz to 16 kHz: worst deviation up to 5 kHz " << Fixed( passbandWorst, 4 ) <<
                                          " dB, the least attenuated tone from 9 kHz up is at " << Fixed( stopbandWorst ) << " dB" );
        ASSERT_TRUE( passbandWorst < 0.1 );
        ASSERT_TRUE( stopbandWorst < -60.0 );

        // exactly 4 kHz
        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( resampler.Configure( 1, 48000.0, 16000.0 ) );
        const TSamples output = Resample( resampler, Sine( 48000, 4000.0 / 48000.0, 0.5f ), 1, 480, 480 );
        const SToneFit fit = FitTone( output, 100, output.size() - 100, 0.25 );
        ASSERT_TRUE( fabs( DecibelsOf( fit.amplitude / 0.5 ) ) < 0.1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: 16 kHz to 48 kHz passes 4 kHz within 0.1 dB with images below -60 dB, and the round trip within 0.2 dB" )
    try
    {
        AUDIO::CSincResampler up;
        ASSERT_TRUE( up.Configure( 1, 16000.0, 48000.0 ) );
        const TSamples output = Resample( up, Sine( 16000, 0.25, 0.5f ), 1, 160, 480 );
        ASSERT_TRUE( output.size() > 47000 );
        const SToneFit fit = FitTone( output, 300, output.size() - 300, 4000.0 / 48000.0 );
        const Float64 gain = DecibelsOf( fit.amplitude / 0.5 );
        const Float64 images = SpuriousDb( output, 300, output.size() - 300, 4000.0 / 48000.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: 16 kHz to 48 kHz at 4 kHz: gain " << Fixed( gain, 4 ) << " dB, images and noise " << Fixed( images ) << " dB" );
        ASSERT_TRUE( fabs( gain ) < 0.1 );
        ASSERT_TRUE( images < -60.0 );

        // down and up again
        AUDIO::CSincResampler down;
        ASSERT_TRUE( down.Configure( 1, 48000.0, 16000.0 ) );
        AUDIO::CSincResampler back;
        ASSERT_TRUE( back.Configure( 1, 16000.0, 48000.0 ) );
        const TSamples trip = Resample( back, Resample( down, Sine( 48000, 4000.0 / 48000.0, 0.5f ), 1, 480, 480 ), 1, 160, 480 );
        const SToneFit tripFit = FitTone( trip, 300, trip.size() - 300, 4000.0 / 48000.0 );
        ASSERT_TRUE( fabs( DecibelsOf( tripFit.amplitude / 0.5 ) ) < 0.2 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: 1 kHz through ratios 1 +- 0.005 has spurious components below -80 dB, the stretched kernel (44.1 kHz to 8 kHz, 48 kHz to 16 kHz) also" )
    try
    {
        const Float64 steps[] = { 0.995, 0.9999, 1.0, 1.0003, 1.005 };
        Float64 worst = -1000.0;
        for ( UInt32 i=0; i<5; ++i )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 1, steps[ i ] ) );
            const TSamples output = Resample( resampler, Sine( 96000, 1000.0 / 48000.0, 0.5f ), 1, 128, 100 );
            const Float64 spurious = SpuriousDb( output, 200, output.size() - 200, 1000.0 / 48000.0 * steps[ i ] );
            if ( spurious > worst )
                worst = spurious;
            ASSERT_TRUE( spurious < -80.0 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: 1 kHz at 48 kHz through ratios 0.995 to 1.005: worst spurious level " << Fixed( worst ) << " dB" );

        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( resampler.Configure( 1, 44100.0, 8000.0 ) );
        const TSamples output = Resample( resampler, Sine( 88200, 1000.0 / 44100.0, 0.5f ), 1, 441, 100 );
        const Float64 stretched = SpuriousDb( output, 200, output.size() - 200, 1000.0 / 8000.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: 1 kHz from 44.1 kHz to 8 kHz: spurious level " << Fixed( stretched ) << " dB" );
        ASSERT_TRUE( stretched < -80.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRatioTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SincResampler: 60 s of audio makes exactly the number of frames the ratio asks for (within one frame, under 1 ppm)" )
    try
    {
        const Float64 steps[] = { 48000.0 / 44100.0, 44100.0 / 48000.0, 1.0 + 300.0e-6, 1.0 - 300.0e-6, 1.005, 0.995, 3.0 };
        const UInt32 total = 48000 * 60;
        const TSamples silence( total, 0.0f );
        for ( UInt32 i=0; i<7; ++i )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 1, steps[ i ] ) );
            const UInt32 latency = resampler.GetLatencyInFrames();
            UInt64 made = 0;
            TSamples out( 9600 );
            for ( UInt32 offset=0; offset<total; offset+=4800 )
            {
                UInt32 used = 0;
                UInt32 produced = 0;
                ProcessRealtime( resampler, &silence[ offset ], 4800, used, &out[ 0 ], 9600, produced );
                made += produced;
                ASSERT_TRUE( 4800 == used );        // there is room for all the output that 4800 frames of input can make
            }
            const Float64 expected = ( (Float64) total - (Float64) latency ) / steps[ i ];
            ASSERT_TRUE( fabs( (Float64) made - expected ) <= 1.0 );
            ASSERT_TRUE( fabs( (Float64) made - (Float64) total / steps[ i ] ) <= ( steps[ i ] > 1.0 ? (Float64) latency / steps[ i ] : (Float64) latency ) + 1.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: the frequency of a tone through 60 s at a drift ratio and at 48 kHz to 44.1 kHz is the ratio times the input frequency within 1 ppm" )
    try
    {
        const Float64 steps[] = { 1.0 + 300.0e-6, 1.0 - 100.0e-6, 48000.0 / 44100.0 };
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 1, steps[ i ] ) );
            const TSamples output = Resample( resampler, Sine( 48000 * 60, 1000.0 / 48000.0, 0.5f ), 1, 4800, 4800 );
            ASSERT_TRUE( output.size() > 2000000 );

            // rising zero crossings with the crossing time interpolated linearly between the two samples
            Float64 first = -1.0;
            Float64 last = -1.0;
            UInt64 crossings = 0;
            for ( size_t n=1000; n<output.size(); ++n )
            {
                if ( output[ n-1 ] < 0.0f && output[ n ] >= 0.0f )
                {
                    const Float64 time = (Float64) ( n-1 ) + (Float64) -output[ n-1 ] / ( (Float64) output[ n ] - (Float64) output[ n-1 ] );
                    if ( first < 0.0 )
                        first = time;
                    last = time;
                    ++crossings;
                }
            }
            ASSERT_TRUE( crossings > 50000 );
            const Float64 measured = (Float64) ( crossings - 1 ) / ( last - first );        // cycles per output frame
            const Float64 expected = 1000.0 / 48000.0 * steps[ i ];
            const Float64 ppm = fabs( measured / expected - 1.0 ) * 1.0e6;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: ratio " << Fixed( steps[ i ], 6 ) << " over 60 s: output frequency off by " << Fixed( ppm, 4 ) << " ppm" );
            ASSERT_TRUE( ppm < 1.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRatioChangeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SincResampler: ratio changes between blocks (+-0.5 %, large steps, a slow ramp through 1) leave the second difference within what the tone itself needs" )
    try
    {
        const Float64 omegaInput = 2.0 * Pi * 1000.0 / 48000.0;
        const TSamples tone = Sine( 192000, 1000.0 / 48000.0, 0.5f );

        // The schedules are followed block after block. The phase of the tone is continuous, so no step is larger than the
        // slope of the fastest tone (amplitude * omega) and the second difference is at most amplitude * omega^2 plus,
        // where the frequency changes, amplitude * the change of omega (a bend in the signal, not a jump).
        struct SSchedule { const char* name; Float64 steps[ 9 ]; UInt32 count; Float64 largest; Float64 smallest; };
        SSchedule schedules[ 3 ] = {
            { "+-0.5 %", { 1.0, 1.005, 0.995, 1.0, 1.005, 1.0, 0.995, 1.005, 0.995 }, 9, 1.005, 0.995 },
            { "large steps", { 1.0, 0.5, 2.0, 1.0, 3.0, 0.8, 1.0, 2.5, 0.25 }, 9, 3.0, 0.25 },
            { "around 1", { 0.97, 1.03, 0.9995, 1.0005, 1.0, 1.02, 0.98, 1.0, 1.0 }, 9, 1.03, 0.97 } };
        for ( UInt32 s=0; s<3; ++s )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 1, schedules[ s ].steps[ 0 ] ) );
            TSamples result;
            TSamples out( 2000 );
            size_t offset = 0;
            UInt32 block = 0;
            while ( offset + 256 <= tone.size() )
            {
                ASSERT_TRUE( SetStep( resampler, schedules[ s ].steps[ block % schedules[ s ].count ] ) );
                UInt32 used = 0;
                UInt32 made = 0;
                ProcessRealtime( resampler, &tone[ offset ], 256, used, &out[ 0 ], 2000, made );
                result.insert( result.end(), out.begin(), out.begin() + made );
                offset += used;
                ++block;
            }
            const Float64 omegaLargest = omegaInput * schedules[ s ].largest;
            const Float64 bound = 0.5 * omegaLargest * omegaLargest + 0.5 * omegaInput * ( schedules[ s ].largest - schedules[ s ].smallest );
            const Float64 measured = MaxSecondDifference( result, 100 );
            Float64 largestStep = 0.0;
            for ( size_t n=101; n<result.size(); ++n )
                if ( fabs( (Float64) result[ n ] - (Float64) result[ n-1 ] ) > largestStep )
                    largestStep = fabs( (Float64) result[ n ] - (Float64) result[ n-1 ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: ratio changes (" << schedules[ s ].name << "): largest step " << Fixed( largestStep, 6 ) << " of at most " <<
                                              Fixed( 0.5 * omegaLargest, 6 ) << ", largest second difference " << Fixed( measured, 6 ) << " of at most " << Fixed( bound, 6 ) );
            ASSERT_TRUE( largestStep <= 0.5 * omegaLargest * 1.001 + 1.0e-6 );
            ASSERT_TRUE( measured <= bound * 1.03 + 1.0e-6 );
        }

        // a slow ramp from 0.9 to 1.1 as a drift controller makes it: 1e-5 per 64 frames, through the point where the kernel starts to stretch
        AUDIO::CSincResampler ramp;
        ASSERT_TRUE( Setup( ramp, 1, 0.9 ) );
        TSamples result;
        TSamples out( 2000 );
        size_t offset = 0;
        Float64 step = 0.9;
        while ( offset + 64 <= tone.size() && step < 1.1 )
        {
            ASSERT_TRUE( SetStep( ramp, step ) );
            UInt32 used = 0;
            UInt32 made = 0;
            ProcessRealtime( ramp, &tone[ offset ], 64, used, &out[ 0 ], 2000, made );
            result.insert( result.end(), out.begin(), out.begin() + made );
            offset += used;
            step += 1.0e-4;
        }
        ASSERT_TRUE( step > 1.0999 );
        const Float64 rampBound = 0.5 * omegaInput * 1.1 * omegaInput * 1.1;
        ASSERT_TRUE( MaxSecondDifference( result, 100 ) <= rampBound * 1.03 + 1.0e-6 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: a change from a strong downsampling to a small ratio, when the next taps lie wholly in the history, reads only inside the buffers" )
    try
    {
        // Regression: after a ratio of 32 the position can lie up to 512 frames before the next chunk; with a ratio of 0.5 the
        // window of 32 taps is then entirely in the history (it once read far past the buffer).
        const Float64 firsts[] = { 32.0, 20.0, 3.0 };
        const Float64 seconds[] = { 0.5, 0.25, 1.0 };
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 2, firsts[ i ] ) );
            const TSamples constant( 2 * 4000, 0.5f );
            TSamples out( 2 * 4000 );
            UInt32 used = 0;
            UInt32 made = 0;
            ProcessRealtime( resampler, &constant[ 0 ], 4000, used, &out[ 0 ], 4000, made );
            ASSERT_TRUE( made > 0 );
            ASSERT_TRUE( SetStep( resampler, seconds[ i ] ) );
            UInt32 total = 0;
            for ( UInt32 block=0; block<400; ++block )
            {
                ProcessRealtime( resampler, &constant[ 0 ], 3, used, &out[ 0 ], 4000, made );
                total += made;
                for ( UInt32 n=0; n<2*made; ++n )
                    ASSERT_TRUE( out[ n ] == out[ n ] && fabsf( out[ n ] ) < 1.0f );
            }
            ASSERT_TRUE( total > 0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: the click check has teeth: a restart in the middle of the tone breaks the bound" )
    try
    {
        const Float64 omega = 2.0 * Pi * 1000.0 / 48000.0;
        const TSamples tone = Sine( 4800, 1000.0 / 48000.0, 0.5f );
        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( Setup( resampler, 1, 1.0 ) );
        TSamples first = Resample( resampler, TSamples( tone.begin(), tone.begin() + 2400 ), 1, 480, 480 );
        resampler.Reset();                  // the history is gone: the signal restarts from nothing
        const TSamples second = Resample( resampler, TSamples( tone.begin() + 2400, tone.end() ), 1, 480, 480 );
        first.insert( first.end(), second.begin(), second.end() );
        ASSERT_TRUE( MaxSecondDifference( first, 100 ) > 5.0 * 0.5 * omega * omega );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformStreamTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SincResampler: the size of the pieces the audio comes in and the room for output does not change the result" )
    try
    {
        const Float64 steps[] = { 0.4, 1.003, 3.0, 44100.0 / 8000.0 };
        const TSamples noise = MakeNoise( 30000, 2, 77, 0.5f );
        for ( UInt32 i=0; i<4; ++i )
        {
            AUDIO::CSincResampler whole;
            ASSERT_TRUE( Setup( whole, 2, steps[ i ] ) );
            const TSamples reference = Resample( whole, noise, 2, 30000, 1 << 20 );
            ASSERT_TRUE( reference.size() > 2 * 3000 );

            const UInt32 shapes[][ 3 ] = { { 1, 1, 5 }, { 700, 50, 6 }, { 64, 7, 7 }, { 13, 1000, 8 } };
            for ( UInt32 k=0; k<4; ++k )
            {
                AUDIO::CSincResampler pieces;
                ASSERT_TRUE( Setup( pieces, 2, steps[ i ] ) );
                const TSamples chunked = ResampleRandom( pieces, noise, 2, shapes[ k ][ 0 ], shapes[ k ][ 1 ], shapes[ k ][ 2 ] );
                ASSERT_TRUE( chunked.size() == reference.size() );
                ASSERT_TRUE( MaxDifference( reference, chunked ) < 2.0e-6 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: latency is reported in frames of input, the first output waits for it and corresponds to the first input frame" )
    try
    {
        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( resampler.Configure( 1, 48000.0, 48000.0 ) );
        ASSERT_TRUE( 16 == resampler.GetLatencyInFrames() );
        ASSERT_TRUE( resampler.SetRates( 48000.0, 96000.0 ) );
        ASSERT_TRUE( 16 == resampler.GetLatencyInFrames() );
        ASSERT_TRUE( resampler.SetRates( 48000.0, 16000.0 ) );
        ASSERT_TRUE( 48 == resampler.GetLatencyInFrames() );
        ASSERT_TRUE( resampler.SetRates( 44100.0, 8000.0 ) );
        ASSERT_TRUE( 89 == resampler.GetLatencyInFrames() );
        ASSERT_TRUE( resampler.SetRates( 48000.0, 48000.0 ) );

        // 16 frames are not enough for a first output, the 17th makes it
        const TSamples ramp( 64, 0.25f );
        TSamples out( 64 );
        UInt32 used = 0;
        UInt32 made = 0;
        ProcessRealtime( resampler, &ramp[ 0 ], 16, used, &out[ 0 ], 64, made );
        ASSERT_TRUE( 0 == made && 16 == used );
        ProcessRealtime( resampler, &ramp[ 0 ], 1, used, &out[ 0 ], 64, made );
        ASSERT_TRUE( 1 == made && 1 == used );

        // an impulse comes out where it went in: at ratio 1 at the same frame, at 2 to 1 at half the frame number
        for ( UInt32 i=0; i<2; ++i )
        {
            AUDIO::CSincResampler aligned;
            ASSERT_TRUE( Setup( aligned, 1, i == 0 ? 1.0 : 2.0 ) );
            TSamples impulse( 400, 0.0f );
            impulse[ 100 ] = 1.0f;
            const TSamples output = Resample( aligned, impulse, 1, 400, 400 );
            size_t peak = 0;
            for ( size_t n=0; n<output.size(); ++n )
                if ( fabsf( output[ n ] ) > fabsf( output[ peak ] ) )
                    peak = n;
            ASSERT_TRUE( ( 0 == i ? 100 : 50 ) == peak );
            ASSERT_TRUE( output[ peak ] > 0.4f );
        }

        // and a tone keeps its phase through 3 to 1: output frame n is the tone at input frame 3n
        AUDIO::CSincResampler phase;
        ASSERT_TRUE( phase.Configure( 1, 48000.0, 16000.0 ) );
        const TSamples output = Resample( phase, Sine( 4800, 500.0 / 48000.0, 0.5f ), 1, 480, 480 );
        Float64 worst = 0.0;
        for ( size_t n=40; n<output.size() - 40; ++n )
        {
            const Float64 expected = 0.5 * sin( 2.0 * Pi * 500.0 * 3.0 * (Float64) n / 48000.0 );
            if ( fabs( (Float64) output[ n ] - expected ) > worst )
                worst = fabs( (Float64) output[ n ] - expected );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SincResampler: 500 Hz through 48 kHz to 16 kHz keeps its phase within " << Fixed( worst, 6 ) );
        ASSERT_TRUE( worst < 1.0e-3 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: a constant stays the same constant at every ratio and through ratio changes (gain at DC is 1)" )
    try
    {
        const Float64 steps[] = { 0.37, 1.0, 1.0001, 2.5, 7.77, 31.5 };
        for ( UInt32 i=0; i<6; ++i )
        {
            AUDIO::CSincResampler resampler;
            ASSERT_TRUE( Setup( resampler, 1, steps[ i ] ) );
            const TSamples output = Resample( resampler, TSamples( 20000, 0.5f ), 1, 300, 300 );
            ASSERT_TRUE( output.size() > 100 );
            const size_t skip = (size_t) ( 2 * resampler.GetLatencyInFrames() / steps[ i ] ) + 4;
            for ( size_t n=skip; n<output.size(); ++n )
                ASSERT_TRUE( fabsf( output[ n ] - 0.5f ) < 5.0e-6f );
        }

        AUDIO::CSincResampler changing;
        ASSERT_TRUE( Setup( changing, 1, 1.0 ) );
        const TSamples constant( 256, 0.5f );
        TSamples out( 1000 );
        for ( UInt32 block=0; block<200; ++block )
        {
            ASSERT_TRUE( SetStep( changing, 0.3 + 0.05 * (Float64) ( block % 40 ) ) );
            UInt32 used = 0;
            UInt32 made = 0;
            ProcessRealtime( changing, &constant[ 0 ], 256, used, &out[ 0 ], 1000, made );
            if ( block > 4 )
                for ( UInt32 n=0; n<made; ++n )
                    ASSERT_TRUE( fabsf( out[ n ] - 0.5f ) < 5.0e-6f );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: channels are independent: every channel of an 8 channel stream equals the same signal alone" )
    try
    {
        const UInt32 channels = 8;
        const UInt32 frames = 6000;
        TSamples interleaved( (size_t) frames * channels );
        GUCEF::vector< TSamples > mono;
        for ( UInt32 c=0; c<channels; ++c )
        {
            mono.push_back( Sine( frames, ( 300.0 + 700.0 * c ) / 48000.0, 0.1f * (Float32) ( c + 1 ) ) );
            for ( UInt32 n=0; n<frames; ++n )
                interleaved[ (size_t) n * channels + c ] = mono[ c ][ n ];
        }
        const Float64 steps[] = { 1.0021, 3.0 };
        for ( UInt32 i=0; i<2; ++i )
        {
            AUDIO::CSincResampler all;
            ASSERT_TRUE( Setup( all, channels, steps[ i ] ) );
            const TSamples together = Resample( all, interleaved, channels, 500, 300 );
            for ( UInt32 c=0; c<channels; ++c )
            {
                AUDIO::CSincResampler alone;
                ASSERT_TRUE( Setup( alone, 1, steps[ i ] ) );
                const TSamples single = Resample( alone, mono[ c ], 1, 500, 300 );
                const TSamples fromAll = Channel( together, channels, c );
                ASSERT_TRUE( fromAll.size() == single.size() );
                ASSERT_TRUE( MaxDifference( fromAll, single ) < 1.0e-6 );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSetupTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SincResampler: setup is validated, Reset starts the stream over, Clone makes an independent resampler of the same setup" )
    try
    {
        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( 0 == ::strcmp( "sinc", resampler.GetName() ) );
        ASSERT_FALSE( resampler.SetRates( 48000.0, 16000.0 ) );             // not configured
        ASSERT_FALSE( resampler.Configure( 0, 48000.0, 16000.0 ) );
        ASSERT_FALSE( resampler.Configure( 1, 0.0, 16000.0 ) );
        ASSERT_FALSE( resampler.Configure( 1, 48000.0, -1.0 ) );
        ASSERT_FALSE( resampler.Configure( 1, 33.0, 1.0 ) );                // stronger than 32 to 1
        ASSERT_TRUE( resampler.Configure( 1, 32.0, 1.0 ) );
        ASSERT_TRUE( Setup( resampler, 2, 3.0 ) );
        ASSERT_FALSE( resampler.SetRates( 0.0, 1.0 ) );
        ASSERT_FALSE( resampler.SetRates( 1.0, 0.0 ) );
        ASSERT_FALSE( resampler.SetRates( 40.0, 1.0 ) );
        ASSERT_TRUE( 3.0 == resampler.GetStep() );                          // refused calls change nothing
        ASSERT_TRUE( resampler.SetRates( 48000.0, 44100.0 ) );
        ASSERT_TRUE( fabs( resampler.GetStep() - 48000.0 / 44100.0 ) < 1.0e-12 );

        const TSamples noise = MakeNoise( 3000, 2, 5, 0.5f );
        ASSERT_TRUE( Setup( resampler, 2, 1.37 ) );
        const TSamples first = Resample( resampler, noise, 2, 700, 500 );
        resampler.Reset();
        const TSamples again = Resample( resampler, noise, 2, 700, 500 );
        ASSERT_TRUE( first == again );
        ASSERT_TRUE( first.size() > 2 * 2000 );

        // a clone has the setup and no history: it makes what a fresh resampler makes
        AUDIO::IAudioResampler* clone = resampler.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone );
        ASSERT_TRUE( 0 == ::strcmp( "sinc", clone->GetName() ) );
        ASSERT_TRUE( 22 == clone->GetLatencyInFrames() );
        TSamples cloned( first.size() );
        UInt32 used = 0;
        UInt32 made = 0;
        cloned.resize( first.size() + 200 );
        clone->Process( &noise[ 0 ], 3000, used, &cloned[ 0 ], (UInt32) cloned.size() / 2, made );
        ASSERT_TRUE( 3000 == used );
        ASSERT_TRUE( made * 2 == first.size() );
        ASSERT_TRUE( MaxDifference( TSamples( cloned.begin(), cloned.begin() + first.size() ), first ) < 2.0e-6 );     // the pieces differ from the first run, the position sums differ in the last bits
        GUCEF_DELETE clone;

        // no room for output, or nothing to convert, does nothing harmful
        UInt32 consumed = 99;
        UInt32 produced = 99;
        resampler.Reset();
        ProcessRealtime( resampler, &noise[ 0 ], 100, consumed, &cloned[ 0 ], 0, produced );
        ASSERT_TRUE( 0 == produced );
        ProcessRealtime( resampler, &noise[ 0 ], 0, consumed, &cloned[ 0 ], 100, produced );
        ASSERT_TRUE( 0 == produced && 0 == consumed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SincResampler: output never holds denormal numbers: a denormal input, and the silence after a loud signal" )
    try
    {
        AUDIO::CSincResampler resampler;
        ASSERT_TRUE( Setup( resampler, 1, 1.01 ) );

        // inputs in the denormal range, then a loud burst with a decaying tail and a long silence
        TSamples input( 20000, 0.0f );
        CXorShift random( 3 );
        for ( size_t n=0; n<2000; ++n )
            input[ n ] = 1.0e-39f * random.NextFloat();
        for ( size_t n=2000; n<2200; ++n )
            input[ n ] = 0.9f * random.NextFloat();
        const TSamples output = Resample( resampler, input, 1, 256, 256 );
        for ( size_t n=0; n<output.size(); ++n )
            ASSERT_TRUE( FP_SUBNORMAL != ::fpclassify( output[ n ] ) );
        for ( size_t n=output.size()-1000; n<output.size(); ++n )
            ASSERT_TRUE( 0.0f == output[ n ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioSincResamplerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioSincResampler TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.SincResampler" );

    PerformResponseTests();
    PerformRatioTests();
    PerformRatioChangeTests();
    PerformStreamTests();
    PerformSetupTests();
}

/*-------------------------------------------------------------------------*/
