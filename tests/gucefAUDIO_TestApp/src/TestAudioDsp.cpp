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

#ifndef GUCEF_AUDIO_CBIQUADDESIGN_H
#include "gucefAUDIO_CBiquadDesign.h"
#define GUCEF_AUDIO_CBIQUADDESIGN_H
#endif /* GUCEF_AUDIO_CBIQUADDESIGN_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

#ifndef GUCEF_AUDIO_CDELAYLINE_H
#include "gucefAUDIO_CDelayLine.h"
#define GUCEF_AUDIO_CDELAYLINE_H
#endif /* GUCEF_AUDIO_CDELAYLINE_H ? */

#ifndef GUCEF_AUDIO_CENVELOPEFOLLOWER_H
#include "gucefAUDIO_CEnvelopeFollower.h"
#define GUCEF_AUDIO_CENVELOPEFOLLOWER_H
#endif /* GUCEF_AUDIO_CENVELOPEFOLLOWER_H ? */

#ifndef GUCEF_AUDIO_CSLIDINGMINIMUM_H
#include "gucefAUDIO_CSlidingMinimum.h"
#define GUCEF_AUDIO_CSLIDINGMINIMUM_H
#endif /* GUCEF_AUDIO_CSLIDINGMINIMUM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioDsp.h"

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

const Float64 Pi = 3.14159265358979323846;
const Float64 SampleRate = 48000.0;
const UInt32 ProbeCount = 31;

const AUDIO::TFilterResponse AllResponses[] =
{
    AUDIO::FILTERRESPONSE_LOWPASS, AUDIO::FILTERRESPONSE_HIGHPASS, AUDIO::FILTERRESPONSE_BANDPASS, AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB,
    AUDIO::FILTERRESPONSE_NOTCH, AUDIO::FILTERRESPONSE_ALLPASS, AUDIO::FILTERRESPONSE_PEAKING, AUDIO::FILTERRESPONSE_LOWSHELF, AUDIO::FILTERRESPONSE_HIGHSHELF
};
const UInt32 ResponseCount = sizeof( AllResponses ) / sizeof( AllResponses[ 0 ] );

const char*
ResponseName( AUDIO::TFilterResponse response )
{
    switch ( response )
    {
        case AUDIO::FILTERRESPONSE_LOWPASS : return "low pass";
        case AUDIO::FILTERRESPONSE_HIGHPASS : return "high pass";
        case AUDIO::FILTERRESPONSE_BANDPASS : return "band pass (skirt gain)";
        case AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB : return "band pass (0 dB peak)";
        case AUDIO::FILTERRESPONSE_NOTCH : return "notch";
        case AUDIO::FILTERRESPONSE_ALLPASS : return "all pass";
        case AUDIO::FILTERRESPONSE_PEAKING : return "peaking";
        case AUDIO::FILTERRESPONSE_LOWSHELF : return "low shelf";
        case AUDIO::FILTERRESPONSE_HIGHSHELF : return "high shelf";
        default : return "unknown";
    }
}

bool
HasGain( AUDIO::TFilterResponse response )
{
    return AUDIO::FILTERRESPONSE_PEAKING == response || AUDIO::FILTERRESPONSE_LOWSHELF == response || AUDIO::FILTERRESPONSE_HIGHSHELF == response;
}

/** White noise from a fixed seed, never rand() */
class CXorShift
{
    public:

    explicit CXorShift( UInt32 seed = 2463534242U ) : m_state( seed ) {}

    UInt32 Next( void )
    {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 17;
        m_state ^= m_state << 5;
        return m_state;
    }

    /** -1 to 1 */
    Float32 NextFloat( void ) { return (Float32) ( (Float64) Next() / 2147483648.0 - 1.0 ); }

    private:

    UInt32 m_state;
};

/** Probe frequencies, logarithmically spaced from 20 Hz to 20 kHz */
Float64
ProbeFrequency( UInt32 index )
{
    return 20.0 * pow( 1000.0, (Float64) index / (Float64) ( ProbeCount - 1 ) );
}

/** Magnitude in decibels at 'frequency' of a filter given by its impulse response (long enough to have decayed) */
Float64
MagnitudeOfImpulseResponse( const GUCEF::vector< Float64 >& response, Float64 frequency )
{
    const Float64 w = 2.0 * Pi * frequency / SampleRate;
    const Float64 stepRe = cos( w ), stepIm = -sin( w );
    Float64 phaseRe = 1.0, phaseIm = 0.0;
    Float64 sumRe = 0.0, sumIm = 0.0;
    for ( size_t n=0; n<response.size(); ++n )
    {
        sumRe += response[ n ] * phaseRe;
        sumIm += response[ n ] * phaseIm;
        const Float64 nextRe = phaseRe * stepRe - phaseIm * stepIm;
        phaseIm = phaseRe * stepIm + phaseIm * stepRe;
        phaseRe = nextRe;
    }
    return 20.0 * log10( sqrt( sumRe * sumRe + sumIm * sumIm ) );
}

/** The impulse response of a direct form 1 biquad with the given coefficients */
GUCEF::vector< Float64 >
BiquadImpulseResponse( const AUDIO::SBiquadCoefficients& c, UInt32 length )
{
    GUCEF::vector< Float64 > response( length );
    Float64 x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    for ( UInt32 n=0; n<length; ++n )
    {
        const Float64 x = ( 0 == n ) ? 1.0 : 0.0;
        const Float64 y = c.b0 * x + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        response[ n ] = y;
    }
    return response;
}

/** The impulse response of the state variable filter as set */
GUCEF::vector< Float64 >
SvfImpulseResponse( AUDIO::CSvfFilter& filter, UInt32 length )
{
    GUCEF::vector< Float64 > response( length );
    for ( UInt32 n=0; n<length; ++n )
        response[ n ] = (Float64) filter.Process( 0, ( 0 == n ) ? 1.0f : 0.0f );
    return response;
}

Float64
Max( Float64 a, Float64 b )
{
    return a > b ? a : b;
}

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformBiquadDesignTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDsp: The analytic magnitude of every biquad design equals the response of the filter run on an impulse" )
    try
    {
        const Float64 frequencies[] = { 150.0, 1000.0, 7000.0 };
        const Float64 qs[] = { 0.5, 1.0, 4.0 };
        const Float64 gains[] = { -12.0, 9.0 };
        for ( UInt32 r=0; r<ResponseCount; ++r )
        {
            Float64 worst = 0.0;
            for ( UInt32 fi=0; fi<3; ++fi )
                for ( UInt32 qi=0; qi<3; ++qi )
                    for ( UInt32 gi=0; gi<2; ++gi )
                    {
                        AUDIO::SBiquadCoefficients c;
                        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AllResponses[ r ], SampleRate, frequencies[ fi ], qs[ qi ], gains[ gi ], c ) );
                        const GUCEF::vector< Float64 > impulse = BiquadImpulseResponse( c, 16384 );
                        for ( UInt32 p=0; p<ProbeCount; ++p )
                        {
                            const Float64 analytic = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, ProbeFrequency( p ) );
                            const Float64 measured = MagnitudeOfImpulseResponse( impulse, ProbeFrequency( p ) );
                            // deep notches cannot be compared in decibels: below -90 dB both are zero for the purpose of the test
                            if ( analytic < -90.0 && measured < -90.0 )
                                continue;
                            const Float64 deviation = fabs( analytic - measured );
                            if ( deviation > worst ) worst = deviation;
                        }
                    }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDsp: biquad " << ResponseName( AllResponses[ r ] ) << ": analytic against impulse response, worst deviation " << worst << " dB" );
            ASSERT_TRUE( worst < 1.0e-6 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Biquad designs have the textbook landmark values" )
    try
    {
        AUDIO::SBiquadCoefficients c;

        // low pass: 0 dB far below, -3.01 dB at the corner for Q = 1/sqrt(2), strongly down far above
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 1000.0, 0.70710678, 0.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 1.0 ), 0.0, 0.001 ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 1000.0 ), -3.0103, 0.001 ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 16000.0 ) < -40.0 );

        // peaking: the set gain at the center, 0 dB far away
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_PEAKING, SampleRate, 2000.0, 2.0, 9.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 2000.0 ), 9.0, 0.001 ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 20.0 ), 0.0, 0.05 ) );

        // shelves: the set gain on their side, half of it (in decibels) at the corner
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_LOWSHELF, SampleRate, 500.0, 0.70710678, -12.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 1.0 ), -12.0, 0.01 ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 500.0 ), -6.0, 0.01 ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 20000.0 ), 0.0, 0.05 ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_HIGHSHELF, SampleRate, 4000.0, 0.70710678, 6.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 20.0 ), 0.0, 0.05 ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, 4000.0 ), 3.0, 0.01 ) );

        // all pass: unity everywhere. Band pass: gain Q (skirt) or 1 (0 dB peak) at the center. Notch: very deep at the center
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_ALLPASS, SampleRate, 3000.0, 3.0, 0.0, c ) );
        for ( UInt32 p=0; p<ProbeCount; ++p )
            ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitudeInDecibels( c, SampleRate, ProbeFrequency( p ) ), 0.0, 1.0e-6 ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_BANDPASS, SampleRate, 3000.0, 4.0, 0.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitude( c, SampleRate, 3000.0 ), 4.0, 1.0e-6 ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB, SampleRate, 3000.0, 4.0, 0.0, c ) );
        ASSERT_TRUE( Near( AUDIO::CBiquadDesign::GetMagnitude( c, SampleRate, 3000.0 ), 1.0, 1.0e-6 ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_NOTCH, SampleRate, 3000.0, 4.0, 0.0, c ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::GetMagnitude( c, SampleRate, 3000.0 ) < 1.0e-6 );

        // refused input and limits
        ASSERT_FALSE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_LOWPASS, 0.0, 1000.0, 1.0, 0.0, c ) );
        ASSERT_TRUE( AUDIO::CBiquadDesign::Design( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 1.0e9, 0.0, 0.0, c ) );     // frequency and Q are limited, not refused
        ASSERT_TRUE( c.b0 == c.b0 && c.a1 == c.a1 && c.a2 == c.a2 );                                                       // finite
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSvfTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDsp: The magnitude response of the state variable filter equals the analytic response of the Audio EQ Cookbook, per response type" )
    try
    {
        const Float64 frequencies[] = { 250.0, 1000.0, 5000.0, 12000.0 };
        const Float64 qs[] = { 0.5, 0.70710678, 2.0, 8.0 };
        const Float64 gains[] = { -12.0, 6.0, 15.0 };
        for ( UInt32 r=0; r<ResponseCount; ++r )
        {
            Float64 worst = 0.0;
            UInt32 configurations = 0;
            for ( UInt32 fi=0; fi<4; ++fi )
                for ( UInt32 qi=0; qi<4; ++qi )
                    for ( UInt32 gi=0; gi<( HasGain( AllResponses[ r ] ) ? 3U : 1U ); ++gi )
                    {
                        AUDIO::CSvfFilter filter;
                        ASSERT_TRUE( filter.Prepare( 1 ) );
                        ASSERT_TRUE( filter.SetResponse( AllResponses[ r ], SampleRate, frequencies[ fi ], qs[ qi ], gains[ gi ] ) );
                        const GUCEF::vector< Float64 > impulse = SvfImpulseResponse( filter, 32768 );
                        ++configurations;
                        for ( UInt32 p=0; p<ProbeCount; ++p )
                        {
                            const Float64 analytic = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( AllResponses[ r ], SampleRate, frequencies[ fi ], qs[ qi ], gains[ gi ], ProbeFrequency( p ) );
                            const Float64 measured = MagnitudeOfImpulseResponse( impulse, ProbeFrequency( p ) );
                            // the output is 32 bit float: levels far below the impulse cannot be compared in decibels
                            if ( analytic < -80.0 && measured < -80.0 )
                                continue;
                            const Float64 deviation = fabs( analytic - measured );
                            if ( deviation > worst ) worst = deviation;
                        }
                    }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDsp: SVF " << ResponseName( AllResponses[ r ] ) << ": " << configurations << " configurations at " << ProbeCount << " probe frequencies, worst deviation from the analytic response " << worst << " dB" );
            ASSERT_TRUE( worst < 0.01 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: The state variable filter stays finite and bounded when its settings change every sample" )
    try
    {
        CXorShift random;
        for ( UInt32 r=0; r<ResponseCount; ++r )
        {
            AUDIO::CSvfFilter filter;
            ASSERT_TRUE( filter.Prepare( 2 ) );
            Float32 peak = 0.0f;
            bool finite = true;
            for ( UInt32 n=0; n<48000; ++n )
            {
                const Float64 frequency = 20.0 * pow( 1000.0, ( random.NextFloat() + 1.0 ) * 0.5 );
                const Float64 q = 0.5 + 3.5 * ( random.NextFloat() + 1.0 ) * 0.5;
                const Float64 gain = 12.0 * random.NextFloat();
                filter.SetResponse( AllResponses[ r ], SampleRate, frequency, q, gain );
                for ( UInt32 c=0; c<2; ++c )
                {
                    const Float32 y = filter.Process( c, random.NextFloat() );
                    if ( !( y == y ) || fabsf( y ) > 1.0e6f )
                        finite = false;
                    if ( fabsf( y ) > peak ) peak = fabsf( y );
                }
            }
            ASSERT_TRUE( finite );
            ASSERT_TRUE( peak < 200.0f );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: State variable filter channels are independent, block and sample processing agree and in place processing works" )
    try
    {
        CXorShift random;
        const UInt32 frames = 1000;
        GUCEF::vector< Float32 > input( frames * 3 );
        for ( size_t i=0; i<input.size(); ++i )
            input[ i ] = random.NextFloat();

        AUDIO::CSvfFilter sampleFilter, blockFilter, inPlaceFilter, channelFilter;
        ASSERT_TRUE( sampleFilter.Prepare( 3 ) && blockFilter.Prepare( 3 ) && inPlaceFilter.Prepare( 3 ) && channelFilter.Prepare( 3 ) );
        ASSERT_TRUE( 3 == sampleFilter.GetChannelCount() );
        sampleFilter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 900.0, 1.5 );
        blockFilter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 900.0, 1.5 );
        inPlaceFilter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 900.0, 1.5 );
        channelFilter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 900.0, 1.5 );

        GUCEF::vector< Float32 > bySample( input.size() ), byBlock( input.size() ), inPlace( input ), byChannel( input.size() );
        for ( UInt32 f=0; f<frames; ++f )
            for ( UInt32 c=0; c<3; ++c )
                bySample[ f * 3 + c ] = sampleFilter.Process( c, input[ f * 3 + c ] );
        blockFilter.ProcessInterleaved( &input[ 0 ], &byBlock[ 0 ], frames );
        inPlaceFilter.ProcessInterleaved( &inPlace[ 0 ], &inPlace[ 0 ], frames );
        for ( UInt32 c=0; c<3; ++c )
            channelFilter.ProcessChannel( c, &input[ c ], &byChannel[ c ], frames, 3 );
        ASSERT_TRUE( 0 == memcmp( &bySample[ 0 ], &byBlock[ 0 ], bySample.size() * sizeof( Float32 ) ) );
        ASSERT_TRUE( 0 == memcmp( &bySample[ 0 ], &inPlace[ 0 ], bySample.size() * sizeof( Float32 ) ) );
        ASSERT_TRUE( 0 == memcmp( &bySample[ 0 ], &byChannel[ 0 ], bySample.size() * sizeof( Float32 ) ) );

        // signal on channel 1 only: the others stay silent
        AUDIO::CSvfFilter isolated;
        ASSERT_TRUE( isolated.Prepare( 3 ) );
        isolated.SetResponse( AUDIO::FILTERRESPONSE_PEAKING, SampleRate, 1000.0, 2.0, 12.0 );
        bool leaked = false;
        for ( UInt32 n=0; n<500; ++n )
        {
            if ( 0.0f != isolated.Process( 0, 0.0f ) ) leaked = true;
            isolated.Process( 1, random.NextFloat() );
            if ( 0.0f != isolated.Process( 2, 0.0f ) ) leaked = true;
        }
        ASSERT_FALSE( leaked );

        // Reset clears the state and not the response
        isolated.Reset();
        ASSERT_TRUE( 0.0f == isolated.Process( 1, 0.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Feedback state of the state variable filter is flushed of denormals" )
    try
    {
        // a resonant filter rung by an impulse decays towards zero for a long time; once the state is below the threshold it is set to zero
        AUDIO::CSvfFilter filter;
        ASSERT_TRUE( filter.Prepare( 1 ) );
        filter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, SampleRate, 500.0, 0.6 );
        filter.Process( 0, 1.0f );
        for ( UInt32 n=0; n<200000; ++n )
            filter.Process( 0, 0.0f );
        filter.FlushDenormals();
        // after the flush the filter reports exactly silence for silent input
        ASSERT_TRUE( 0.0f == filter.Process( 0, 0.0f ) );

        // state far above the threshold is left alone
        filter.Process( 0, 1.0f );
        filter.FlushDenormals();
        ASSERT_TRUE( 0.0f != filter.Process( 0, 0.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDelayLineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Delay line integer reads return the sample from that many pushes back, across the wrap around of the ring" )
    try
    {
        AUDIO::CDelayLine line;
        ASSERT_FALSE( line.Prepare( 0, 100 ) );
        ASSERT_TRUE( line.Prepare( 2, 100 ) );
        ASSERT_TRUE( 2 == line.GetChannelCount() );
        ASSERT_TRUE( 100 == line.GetMaxDelayInFrames() );

        // not written yet reads as silence
        ASSERT_TRUE( 0.0f == line.Read( 0, 0 ) && 0.0f == line.Read( 0, 100 ) );

        // the ring holds 128 samples: 1000 pushes wrap it many times. Channel 1 carries the negated values
        bool correct = true;
        for ( UInt32 n=1; n<=1000; ++n )
        {
            line.Push( 0, (Float32) n );
            line.Push( 1, -(Float32) n );
            for ( UInt32 d=0; d<=100; ++d )
            {
                const Float32 expected0 = ( n > d ) ? (Float32) ( n - d ) : 0.0f;
                if ( line.Read( 0, d ) != expected0 || line.Read( 1, d ) != -expected0 )
                    correct = false;
            }
        }
        ASSERT_TRUE( correct );

        // a delay beyond the maximum is limited to the maximum
        ASSERT_TRUE( line.Read( 0, 5000 ) == line.Read( 0, 100 ) );

        // Reset forgets
        line.Reset();
        ASSERT_TRUE( 0.0f == line.Read( 0, 0 ) && 0.0f == line.Read( 1, 50 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Delay line makes an echo at the exact sample positions when read before the push (feedback use)" )
    try
    {
        const UInt32 D = 37;
        const Float32 feedback = 0.5f;
        AUDIO::CDelayLine line;
        ASSERT_TRUE( line.Prepare( 1, D ) );
        bool correct = true;
        for ( UInt32 n=0; n<D*5+3; ++n )
        {
            const Float32 x = ( 0 == n ) ? 1.0f : 0.0f;
            const Float32 y = x + feedback * line.Read( 0, D - 1 );     // y[n] = x[n] + g * y[n-D]
            line.Push( 0, y );
            Float32 expected = 0.0f;
            if ( 0 == n % D )
                expected = powf( feedback, (Float32) ( n / D ) );
            if ( fabsf( y - expected ) > 1.0e-7f )
                correct = false;
        }
        ASSERT_TRUE( correct );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Delay line fractional reads: linear is exact on a ramp, cubic is exact on a parabola and close on a sine, both equal the integer read at a whole delay" )
    try
    {
        AUDIO::CDelayLine ramp, parabola, sine;
        ASSERT_TRUE( ramp.Prepare( 1, 200 ) && parabola.Prepare( 1, 200 ) && sine.Prepare( 1, 200 ) );
        const Float64 sineFrequency = 1000.0;

        Float64 worstLinearRamp = 0.0, worstCubicParabola = 0.0, worstCubicSine = 0.0, worstLinearSine = 0.0, worstWhole = 0.0;
        for ( UInt32 n=0; n<2000; ++n )
        {
            ramp.Push( 0, (Float32) n * 0.01f );
            const Float64 k = (Float64) n * 0.01;
            parabola.Push( 0, (Float32) ( 0.5 * k * k * 0.01 ) );
            sine.Push( 0, (Float32) sin( 2.0 * Pi * sineFrequency * (Float64) n / SampleRate ) );
            if ( n < 300 )
                continue;

            for ( UInt32 i=0; i<8; ++i )
            {
                const Float32 delay = 3.0f + (Float32) i * 11.0f + 0.125f * (Float32) ( i + 1 );
                const Float64 t = (Float64) n - (Float64) delay;
                // ramp
                const Float64 expectedRamp = t * 0.01;
                worstLinearRamp = Max( worstLinearRamp, fabs( (Float64) ramp.ReadLinear( 0, delay ) - expectedRamp ) );
                // parabola
                const Float64 tk = t * 0.01;
                const Float64 expectedParabola = 0.5 * tk * tk * 0.01;
                worstCubicParabola = Max( worstCubicParabola, fabs( (Float64) parabola.ReadCubic( 0, delay ) - expectedParabola ) );
                // sine, a tone of 1 kHz is far below the Nyquist frequency of 24 kHz
                const Float64 expectedSine = sin( 2.0 * Pi * sineFrequency * t / SampleRate );
                worstCubicSine = Max( worstCubicSine, fabs( (Float64) sine.ReadCubic( 0, delay ) - expectedSine ) );
                worstLinearSine = Max( worstLinearSine, fabs( (Float64) sine.ReadLinear( 0, delay ) - expectedSine ) );
            }

            const UInt32 whole = 1 + n % 150;
            worstWhole = Max( worstWhole, fabs( (Float64) sine.ReadLinear( 0, (Float32) whole ) - (Float64) sine.Read( 0, whole ) ) );
            worstWhole = Max( worstWhole, fabs( (Float64) sine.ReadCubic( 0, (Float32) whole ) - (Float64) sine.Read( 0, whole ) ) );
        }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDsp: delay line worst errors: linear on ramp " << worstLinearRamp << ", cubic on parabola " << worstCubicParabola << ", cubic on sine " << worstCubicSine << ", linear on sine " << worstLinearSine );
        ASSERT_TRUE( worstLinearRamp < 1.0e-4 );
        ASSERT_TRUE( worstCubicParabola < 1.0e-4 );
        ASSERT_TRUE( worstCubicSine < 1.0e-4 );
        ASSERT_TRUE( worstCubicSine < worstLinearSine );
        ASSERT_TRUE( worstWhole == 0.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Delay line fractional reads at the limits stay inside the history" )
    try
    {
        AUDIO::CDelayLine line;
        ASSERT_TRUE( line.Prepare( 1, 8 ) );      // ring of 16
        for ( UInt32 n=1; n<=100; ++n )
            line.Push( 0, (Float32) n );
        // newest is 100: delay 8 is 92. Beyond the maximum, and below the minimum, the read is limited
        ASSERT_TRUE( line.ReadLinear( 0, 8.0f ) == 92.0f );
        ASSERT_TRUE( line.ReadLinear( 0, 1000.0f ) == 92.0f );
        ASSERT_TRUE( line.ReadLinear( 0, -3.0f ) == 100.0f );
        ASSERT_TRUE( line.ReadCubic( 0, 8.0f ) == 92.0f );
        ASSERT_TRUE( line.ReadCubic( 0, 1000.0f ) == 92.0f );
        ASSERT_TRUE( line.ReadCubic( 0, 0.0f ) == 99.0f );       // the shortest cubic delay is 1
        ASSERT_TRUE( Near( line.ReadLinear( 0, 7.5f ), 92.5, 1.0e-4 ) );
        ASSERT_TRUE( Near( line.ReadCubic( 0, 7.5f ), 92.5, 1.0e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static Float32
BruteForceMinimum( const GUCEF::vector< Float32 >& values, size_t end, UInt32 window )
{
    const size_t start = ( end + 1 > window ) ? end + 1 - window : 0;
    Float32 minimum = values[ start ];
    for ( size_t i=start+1; i<=end; ++i )
        if ( values[ i ] < minimum )
            minimum = values[ i ];
    return minimum;
}

static void
PerformSlidingMinimumTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Sliding minimum equals the brute force minimum on random data for many window sizes" )
    try
    {
        const UInt32 windows[] = { 1, 2, 3, 7, 16, 64, 255, 1000 };
        CXorShift random( 12345 );
        GUCEF::vector< Float32 > values( 20000 );
        for ( size_t i=0; i<values.size(); ++i )
            values[ i ] = random.NextFloat();

        for ( UInt32 w=0; w<sizeof( windows ) / sizeof( windows[ 0 ] ); ++w )
        {
            AUDIO::CSlidingMinimum minimum;
            ASSERT_TRUE( minimum.Prepare( windows[ w ] ) );
            ASSERT_TRUE( windows[ w ] == minimum.GetWindowSize() );
            bool equal = true;
            for ( size_t i=0; i<values.size(); ++i )
            {
                const Float32 got = minimum.Push( values[ i ] );
                if ( got != BruteForceMinimum( values, i, windows[ w ] ) || got != minimum.GetMinimum() )
                    equal = false;
            }
            ASSERT_TRUE( equal );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Sliding minimum handles monotonic, constant, spiky and repeated data and starts over after Reset" )
    try
    {
        for ( UInt32 shape=0; shape<5; ++shape )
        {
            GUCEF::vector< Float32 > values( 3000 );
            CXorShift random( 99 + shape );
            for ( size_t i=0; i<values.size(); ++i )
            {
                switch ( shape )
                {
                    case 0 : values[ i ] = (Float32) i; break;                                  // rising: the oldest is always the minimum
                    case 1 : values[ i ] = -(Float32) i; break;                                 // falling: the newest is always the minimum
                    case 2 : values[ i ] = 0.5f; break;                                         // constant
                    case 3 : values[ i ] = ( 0 == random.Next() % 50 ) ? -1.0f : 1.0f; break;   // rare dips
                    default : values[ i ] = (Float32) ( random.Next() % 4 ); break;             // many equal values
                }
            }
            AUDIO::CSlidingMinimum minimum;
            ASSERT_TRUE( minimum.Prepare( 50 ) );
            bool equal = true;
            for ( size_t i=0; i<values.size(); ++i )
                if ( minimum.Push( values[ i ] ) != BruteForceMinimum( values, i, 50 ) )
                    equal = false;
            ASSERT_TRUE( equal );

            // after a Reset the old values are not part of any window
            minimum.Reset();
            ASSERT_TRUE( 5.0f == minimum.Push( 5.0f ) );
            ASSERT_TRUE( 5.0f == minimum.Push( 9.0f ) );
        }

        AUDIO::CSlidingMinimum none;
        ASSERT_FALSE( none.Prepare( 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Sliding minimum changes its window within the room Prepare() made, and is exact at every size" )
    try
    {
        CXorShift random( 777 );
        GUCEF::vector< Float32 > values( 5000 );
        for ( size_t i=0; i<values.size(); ++i )
            values[ i ] = random.NextFloat();

        AUDIO::CSlidingMinimum minimum;
        ASSERT_TRUE( 0 == minimum.GetCapacity() );
        ASSERT_FALSE( minimum.SetWindowSize( 1 ) );                                 // no room before Prepare()
        ASSERT_TRUE( minimum.Prepare( 481 ) );
        ASSERT_TRUE( 481 == minimum.GetCapacity() );

        // larger, smaller and equal windows in turn, each starting from nothing, each exact
        const UInt32 windows[] = { 73, 1, 481, 25, 145, 73 };
        for ( UInt32 w=0; w<sizeof( windows ) / sizeof( windows[ 0 ] ); ++w )
        {
            minimum.Push( 0.0f );                                                   // left behind by the change
            ASSERT_TRUE( minimum.SetWindowSize( windows[ w ] ) );
            ASSERT_TRUE( windows[ w ] == minimum.GetWindowSize() && 481 == minimum.GetCapacity() );
            bool equal = true;
            for ( size_t i=0; i<values.size(); ++i )
                if ( minimum.Push( values[ i ] ) != BruteForceMinimum( values, i, windows[ w ] ) )
                    equal = false;
            ASSERT_TRUE( equal );
        }

        // a window that does not fit changes nothing
        ASSERT_FALSE( minimum.SetWindowSize( 482 ) );
        ASSERT_FALSE( minimum.SetWindowSize( 0 ) );
        ASSERT_TRUE( 73 == minimum.GetWindowSize() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Number of samples until the envelope of a constant input first reaches 'level' (counting the first sample as 1), 0 if never within the limit */
static UInt32
SamplesUntil( AUDIO::CEnvelopeFollower& follower, Float32 input, bool rising, Float32 level, UInt32 limit )
{
    for ( UInt32 n=1; n<=limit; ++n )
    {
        const Float32 envelope = follower.Process( input );
        if ( rising ? envelope >= level : envelope <= level )
            return n;
    }
    return 0;
}

static void
PerformEnvelopeFollowerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Envelope follower in peak mode reaches 63.2 percent of a step in the attack time and falls to 36.8 percent in the release time" )
    try
    {
        const Float32 rising = 1.0f - expf( -1.0f );
        const Float32 falling = expf( -1.0f );
        const Float64 attacks[] = { 1.0, 10.0, 50.0 };
        const Float64 releases[] = { 20.0, 100.0, 400.0 };
        for ( UInt32 i=0; i<3; ++i )
        {
            AUDIO::CEnvelopeFollower follower;
            ASSERT_TRUE( follower.Prepare( SampleRate ) );
            follower.SetMode( AUDIO::ENVELOPEMODE_PEAK );
            follower.SetAttackTimeInMs( attacks[ i ] );
            follower.SetReleaseTimeInMs( releases[ i ] );

            const UInt32 attackSamples = SamplesUntil( follower, 1.0f, true, rising, 200000 );
            const Float64 expectedAttack = attacks[ i ] * 0.001 * SampleRate;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDsp: envelope attack " << attacks[ i ] << " ms: 63.2 percent after " << attackSamples << " samples, expected " << expectedAttack );
            ASSERT_TRUE( attackSamples > 0 && fabs( (Float64) attackSamples - expectedAttack ) <= 0.1 * expectedAttack );

            // settle at full level, then release
            for ( UInt32 n=0; n<(UInt32) ( SampleRate * 10 ); ++n )
                follower.Process( 1.0f );
            ASSERT_TRUE( Near( follower.GetEnvelope(), 1.0, 1.0e-4 ) );
            const UInt32 releaseSamples = SamplesUntil( follower, 0.0f, false, falling, 400000 );
            const Float64 expectedRelease = releases[ i ] * 0.001 * SampleRate;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDsp: envelope release " << releases[ i ] << " ms: 36.8 percent after " << releaseSamples << " samples, expected " << expectedRelease );
            ASSERT_TRUE( releaseSamples > 0 && fabs( (Float64) releaseSamples - expectedRelease ) <= 0.1 * expectedRelease );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Envelope follower in peak mode uses the absolute value, follows instantly with a time of zero and holds a burst with a slow release" )
    try
    {
        AUDIO::CEnvelopeFollower follower;
        ASSERT_TRUE( follower.Prepare( SampleRate ) );
        follower.SetAttackTimeInMs( 0.0 );
        follower.SetReleaseTimeInMs( 0.0 );
        ASSERT_TRUE( follower.Process( -0.75f ) == 0.75f );
        ASSERT_TRUE( follower.Process( 0.25f ) == 0.25f );
        ASSERT_TRUE( follower.Process( 0.0f ) == 0.0f );

        // fast attack, slow release: a short burst is held
        follower.SetAttackTimeInMs( 0.1 );
        follower.SetReleaseTimeInMs( 500.0 );
        for ( UInt32 n=0; n<480; ++n )
            follower.Process( 1.0f );
        const Float32 afterBurst = follower.GetEnvelope();
        ASSERT_TRUE( afterBurst > 0.99f );
        for ( UInt32 n=0; n<480; ++n )      // 10 ms of silence against a release of 500 ms
            follower.Process( 0.0f );
        ASSERT_TRUE( follower.GetEnvelope() > 0.97f * afterBurst );

        // block processing with a stride equals sample processing
        AUDIO::CEnvelopeFollower a, b;
        ASSERT_TRUE( a.Prepare( SampleRate ) && b.Prepare( SampleRate ) );
        CXorShift random;
        GUCEF::vector< Float32 > interleaved( 2000 ), envelope( 1000 );
        for ( size_t i=0; i<interleaved.size(); ++i )
            interleaved[ i ] = random.NextFloat();
        a.ProcessBlock( &interleaved[ 1 ], &envelope[ 0 ], 1000, 2 );
        bool same = true;
        for ( UInt32 f=0; f<1000; ++f )
            if ( envelope[ f ] != b.Process( interleaved[ f * 2 + 1 ] ) )
                same = false;
        ASSERT_TRUE( same );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDsp: Envelope follower in RMS mode measures the root mean square with the window as time constant" )
    try
    {
        // sine of amplitude 0.8: RMS 0.8 / sqrt(2)
        AUDIO::CEnvelopeFollower follower;
        ASSERT_TRUE( follower.Prepare( SampleRate ) );
        follower.SetMode( AUDIO::ENVELOPEMODE_RMS );
        follower.SetRmsWindowInMs( 20.0 );
        Float32 envelope = 0.0f;
        for ( UInt32 n=0; n<(UInt32) ( SampleRate * 2 ); ++n )
            envelope = follower.Process( (Float32) ( 0.8 * sin( 2.0 * Pi * 1000.0 * (Float64) n / SampleRate ) ) );
        ASSERT_TRUE( Near( envelope, 0.8 / sqrt( 2.0 ), 0.002 ) );

        // constant 1: the mean square reaches 63.2 percent after the window time
        follower.Reset();
        const UInt32 samples = SamplesUntil( follower, 1.0f, true, sqrtf( 1.0f - expf( -1.0f ) ), 100000 );
        const Float64 expected = 20.0 * 0.001 * SampleRate;
        ASSERT_TRUE( samples > 0 && fabs( (Float64) samples - expected ) <= 0.1 * expected );

        // switching mode starts the envelope over, flushing clears a decayed envelope
        follower.SetMode( AUDIO::ENVELOPEMODE_PEAK );
        ASSERT_TRUE( 0.0f == follower.GetEnvelope() );
        follower.SetReleaseTimeInMs( 1.0 );
        follower.Process( 1.0f );
        for ( UInt32 n=0; n<20000; ++n )
            follower.Process( 0.0f );
        follower.FlushDenormals();
        ASSERT_TRUE( 0.0f == follower.GetEnvelope() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioDspTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioDsp TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Dsp" );

    PerformBiquadDesignTests();
    PerformSvfTests();
    PerformDelayLineTests();
    PerformSlidingMinimumTests();
    PerformEnvelopeFollowerTests();
}

/*-------------------------------------------------------------------------*/
