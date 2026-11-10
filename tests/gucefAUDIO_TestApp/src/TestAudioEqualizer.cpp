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

#ifndef GUCEF_AUDIO_CEQUALIZEREFFECT_H
#include "gucefAUDIO_CEqualizerEffect.h"
#define GUCEF_AUDIO_CEQUALIZEREFFECT_H
#endif /* GUCEF_AUDIO_CEQUALIZEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#include "gucefAUDIO_CFilterEffect.h"
#define GUCEF_AUDIO_CFILTEREFFECT_H
#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioEqualizer.h"

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

const Float64 SampleRate = 48000.0;
const UInt32 ProbeCount = 31;
const UInt32 ImpulseFrames = 32768;

/** The response of the Audio EQ Cookbook that a band type stands for: written down independently of the effect */
AUDIO::TFilterResponse
ReferenceResponse( AUDIO::TFilterBandType type )
{
    switch ( type )
    {
        case AUDIO::FILTERBANDTYPE_PEAK : return AUDIO::FILTERRESPONSE_PEAKING;
        case AUDIO::FILTERBANDTYPE_LOWSHELF : return AUDIO::FILTERRESPONSE_LOWSHELF;
        case AUDIO::FILTERBANDTYPE_HIGHSHELF : return AUDIO::FILTERRESPONSE_HIGHSHELF;
        case AUDIO::FILTERBANDTYPE_LOWPASS : return AUDIO::FILTERRESPONSE_LOWPASS;
        case AUDIO::FILTERBANDTYPE_HIGHPASS : return AUDIO::FILTERRESPONSE_HIGHPASS;
        case AUDIO::FILTERBANDTYPE_BANDPASS : return AUDIO::FILTERRESPONSE_BANDPASS_PEAK0DB;
        case AUDIO::FILTERBANDTYPE_NOTCH : return AUDIO::FILTERRESPONSE_NOTCH;
        default : return AUDIO::FILTERRESPONSE_ALLPASS;
    }
}

const char*
TypeName( AUDIO::TFilterBandType type )
{
    return AUDIO::CFilterBand::GetTypeLabels()[ type ];
}

bool
HasGain( AUDIO::TFilterBandType type )
{
    return AUDIO::FILTERBANDTYPE_PEAK == type || AUDIO::FILTERBANDTYPE_LOWSHELF == type || AUDIO::FILTERBANDTYPE_HIGHSHELF == type;
}

/** Probe frequencies, logarithmically spaced from 20 Hz to 20 kHz */
Float64
ProbeFrequency( UInt32 index )
{
    return 20.0 * pow( 1000.0, (Float64) index / (Float64) ( ProbeCount - 1 ) );
}

/** An equalizer with one band, prepared */
bool
PrepareSingleBand( AUDIO::CEqualizerEffect& equalizer, UInt32 channels = 2 )
{
    return equalizer.Prepare( AUDIO::CAudioFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 );
}

CORE::CValueList
Bands( const char* count )
{
    CORE::CValueList options;
    options.Set( "bands", count );
    return options;
}

/** Largest deviation in dB between the measured response of an effect and an analytic one, over the probe frequencies where the analytic response is above -60 dB */
Float64
WorstDeviation( const GUCEF::vector< Float64 >& impulse, AUDIO::TFilterResponse response, Float64 frequency, Float64 q, Float64 gain )
{
    Float64 worst = 0.0;
    for ( UInt32 p=0; p<ProbeCount; ++p )
    {
        const Float64 analytic = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( response, SampleRate, frequency, q, gain, ProbeFrequency( p ) );
        if ( analytic < -60.0 )
            continue;
        const Float64 measured = MagnitudeOfImpulseResponse( impulse, ProbeFrequency( p ), SampleRate );
        const Float64 deviation = fabs( analytic - measured );
        if ( deviation > worst )
            worst = deviation;
    }
    return worst;
}

/** The response in dB of an effect at one frequency, as measured from its impulse response */
Float64
MeasuredAt( const GUCEF::vector< Float64 >& impulse, Float64 frequency )
{
    return MagnitudeOfImpulseResponse( impulse, frequency, SampleRate );
}

/** Settings of a scenario for the tests that look for clicks: a function that sets the parameters of an effect */
typedef void (*TConfigure)( AUDIO::CAudioEffect& effect );

struct SClickScenario
{
    const char* name;
    TConfigure before;
    TConfigure after;
    Float64 allowed;        /**< times the larger steady step that the largest step may be: 1.3, more where the settings sweep over the frequency of the signal */
};

// equalizer (one band) scenarios
void BandPeakFlat( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 0.0f, 1.0f ); }
void BandPeakUp( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 24.0f, 1.0f ); }
void BandPeakDown( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, -24.0f, 1.0f ); }
void BandPeakLow( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 200.0f, 12.0f, 1.0f ); }
void BandPeakHigh( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 8000.0f, 12.0f, 1.0f ); }
void BandWide( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 12.0f, 0.1f ); }
void BandNarrow( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 12.0f, 18.0f ); }
void BandOff( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, false, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 12.0f, 1.0f ); }
void BandOn( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, 12.0f, 1.0f ); }
void BandLowPass( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_LOWPASS, 500.0f, 0.0f, 0.7071f ); }
void BandHighPass( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_HIGHPASS, 500.0f, 0.0f, 0.7071f ); }
void BandLowShelf( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_LOWSHELF, 1000.0f, 12.0f, 0.7071f ); }
void BandHighShelf( AUDIO::CAudioEffect& e ) { static_cast< AUDIO::CEqualizerEffect& >( e ).SetBand( 0, true, AUDIO::FILTERBANDTYPE_HIGHSHELF, 1000.0f, -12.0f, 0.7071f ); }

// filter effect scenarios
void FilterLow( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS ); f.SetFrequency( 200.0f ); f.SetQ( 0.7071f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterHigh( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS ); f.SetFrequency( 8000.0f ); f.SetQ( 0.7071f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterHighPass( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_HIGHPASS ); f.SetFrequency( 2000.0f ); f.SetQ( 0.7071f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterSteep( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS ); f.SetFrequency( 1500.0f ); f.SetSlopeInDecibelsPerOctave( 24 ); }
void FilterGentle( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS ); f.SetFrequency( 1500.0f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterResonant( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS ); f.SetFrequency( 1000.0f ); f.SetQ( 8.0f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterBandPass( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_BANDPASS ); f.SetFrequency( 1000.0f ); f.SetQ( 2.0f ); f.SetSlopeInDecibelsPerOctave( 12 ); }
void FilterNotch( AUDIO::CAudioEffect& e ) { AUDIO::CFilterEffect& f = static_cast< AUDIO::CFilterEffect& >( e ); f.SetFilterType( AUDIO::FILTERBANDTYPE_NOTCH ); f.SetFrequency( 1000.0f ); f.SetQ( 2.0f ); f.SetSlopeInDecibelsPerOctave( 12 ); }

/** Processes 'input' with an effect configured by 'configure', frames 'from' to 'to', appending to 'output' */
void
Append( AUDIO::CAudioEffect& effect, const TSamples& input, UInt32 channels, size_t fromFrame, size_t toFrame, TSamples& output )
{
    TSamples part( input.begin() + fromFrame * channels, input.begin() + toFrame * channels );
    Run( effect, part, channels, 256 );
    output.insert( output.end(), part.begin(), part.end() );
}

/** Largest step of the output of the effect after the settings were set once and left alone, over the second half of the signal */
template < class TEffect >
Float32
SteadyStep( TEffect& effect, TConfigure configure, const TSamples& input )
{
    configure( effect );
    if ( !effect.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) )
        return -1.0f;
    TSamples output;
    Append( effect, input, 2, 0, input.size() / 4, output );              // half of the frames of the signal
    return MaxStep( output, 2, 0, output.size() / 4, output.size() / 2 );   // frames 12000 to 24000
}

/** Settings change in the middle of a sine: the largest step must stay about that of the sines before and after */
template < class TEffect >
bool
CheckNoClicks( const SClickScenario& scenario, const CORE::CValueList& options, Float64 sineFrequency, Float64& ratioFound )
{
    const UInt32 frames = 48000;
    const TSamples input = MakeSine( frames, 2, sineFrequency, 0.5f );

    TEffect before( options ), after( options );
    const Float32 stepBefore = SteadyStep( before, scenario.before, input );
    const Float32 stepAfter = SteadyStep( after, scenario.after, input );
    const Float32 bound = stepBefore > stepAfter ? stepBefore : stepAfter;
    if ( stepBefore < 0.0f || stepAfter < 0.0f )
        return false;

    TEffect effect( options );
    scenario.before( effect );
    if ( !effect.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) )
        return false;
    TSamples output;
    Append( effect, input, 2, 0, frames / 2, output );
    scenario.after( effect );
    Append( effect, input, 2, frames / 2, frames, output );

    // the largest step anywhere, the change included
    const Float32 largest = MaxStep( output, 2, 0 ) > MaxStep( output, 2, 1 ) ? MaxStep( output, 2, 0 ) : MaxStep( output, 2, 1 );
    ratioFound = bound > 0.0f ? (Float64) largest / (Float64) bound : 0.0;
    return AllFiniteAndBelow( output, 1.0e4f ) && ratioFound <= scenario.allowed;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformEqualizerResponseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: The magnitude response of a band, for every type and a grid of frequency, gain and Q, is that of the analytic biquad design within 0.1 dB" )
    try
    {
        const Float64 frequencies[] = { 150.0, 1000.0, 7000.0 };
        const Float64 qs[] = { 0.5, 1.0, 4.0 };
        const Float64 gains[] = { -12.0, 9.0 };
        for ( UInt32 t=0; t<AUDIO::FILTERBANDTYPE_COUNT; ++t )
        {
            const AUDIO::TFilterBandType type = (AUDIO::TFilterBandType) t;
            Float64 worst = 0.0;
            for ( UInt32 fi=0; fi<3; ++fi )
                for ( UInt32 qi=0; qi<3; ++qi )
                    for ( UInt32 gi=0; gi<( HasGain( type ) ? 2U : 1U ); ++gi )
                    {
                        AUDIO::CEqualizerEffect equalizer( Bands( "1" ) );
                        equalizer.SetBand( 0, true, type, (Float32) frequencies[ fi ], (Float32) gains[ gi ], (Float32) qs[ qi ] );
                        ASSERT_TRUE( PrepareSingleBand( equalizer ) );
                        const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( equalizer, 2, ImpulseFrames );
                        const Float64 deviation = WorstDeviation( impulse, ReferenceResponse( type ), frequencies[ fi ], qs[ qi ], HasGain( type ) ? gains[ gi ] : 0.0 );
                        if ( deviation > worst ) worst = deviation;
                        if ( deviation > 0.1 )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioEqualizer: " << TypeName( type ) << " " << frequencies[ fi ] << " Hz Q " << qs[ qi ] << " gain " << gains[ gi ] << " dB deviates " << deviation << " dB" );
                        ASSERT_TRUE( deviation <= 0.1 );
                    }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEqualizer: band type " << TypeName( type ) << ", worst deviation from the analytic response " << worst << " dB" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: Bands in series add up in decibels" )
    try
    {
        AUDIO::CEqualizerEffect equalizer( Bands( "3" ) );
        equalizer.SetBand( 0, true, AUDIO::FILTERBANDTYPE_PEAK, 200.0f, 6.0f, 1.0f );
        equalizer.SetBand( 1, true, AUDIO::FILTERBANDTYPE_PEAK, 1000.0f, -9.0f, 2.0f );
        equalizer.SetBand( 2, true, AUDIO::FILTERBANDTYPE_HIGHSHELF, 8000.0f, 4.0f, 0.7071f );
        ASSERT_TRUE( PrepareSingleBand( equalizer ) );
        ASSERT_TRUE( 3 == equalizer.GetBandCount() );
        const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( equalizer, 2, ImpulseFrames );
        Float64 worst = 0.0;
        for ( UInt32 p=0; p<ProbeCount; ++p )
        {
            const Float64 f = ProbeFrequency( p );
            const Float64 expected = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( AUDIO::FILTERRESPONSE_PEAKING, SampleRate, 200.0, 1.0, 6.0, f ) +
                                     AUDIO::CBiquadDesign::GetMagnitudeInDecibels( AUDIO::FILTERRESPONSE_PEAKING, SampleRate, 1000.0, 2.0, -9.0, f ) +
                                     AUDIO::CBiquadDesign::GetMagnitudeInDecibels( AUDIO::FILTERRESPONSE_HIGHSHELF, SampleRate, 8000.0, 0.7071, 4.0, f );
            const Float64 deviation = fabs( expected - MeasuredAt( impulse, f ) );
            if ( deviation > worst ) worst = deviation;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEqualizer: three bands in series, worst deviation " << worst << " dB" );
        ASSERT_TRUE( worst <= 0.1 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: The default equalizer has eight bands as designed and passes audio bit for bit; a disabled band does nothing" )
    try
    {
        AUDIO::CEqualizerEffect equalizer;
        ASSERT_TRUE( equalizer.GetEffectTypeName() == "equalizer" );
        ASSERT_TRUE( 8 == equalizer.GetBandCount() && 40 == equalizer.GetParameterCount() && 40 == AUDIO::CEqualizerEffect::GetStaticParameterCount() );
        const Float32 frequencies[ 8 ] = { 80.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20.0f };
        const AUDIO::TFilterBandType types[ 8 ] = { AUDIO::FILTERBANDTYPE_LOWSHELF, AUDIO::FILTERBANDTYPE_PEAK, AUDIO::FILTERBANDTYPE_PEAK, AUDIO::FILTERBANDTYPE_PEAK,
                                                    AUDIO::FILTERBANDTYPE_PEAK, AUDIO::FILTERBANDTYPE_PEAK, AUDIO::FILTERBANDTYPE_HIGHSHELF, AUDIO::FILTERBANDTYPE_HIGHPASS };
        for ( UInt32 band=0; band<8; ++band )
        {
            ASSERT_TRUE( frequencies[ band ] == equalizer.GetParameter( AUDIO::CEqualizerEffect::GetParameterId( band, AUDIO::CEqualizerEffect::BANDFIELD_FREQUENCY ) ) );
            ASSERT_TRUE( (Float32) types[ band ] == equalizer.GetParameter( AUDIO::CEqualizerEffect::GetParameterId( band, AUDIO::CEqualizerEffect::BANDFIELD_TYPE ) ) );
            ASSERT_TRUE( 0.0f == equalizer.GetParameter( AUDIO::CEqualizerEffect::GetParameterId( band, AUDIO::CEqualizerEffect::BANDFIELD_GAIN ) ) );
            ASSERT_TRUE( ( 7 == band ? 0.0f : 1.0f ) == equalizer.GetParameter( AUDIO::CEqualizerEffect::GetParameterId( band, AUDIO::CEqualizerEffect::BANDFIELD_ENABLED ) ) );
        }
        UInt32 id = 99;
        ASSERT_TRUE( equalizer.TryGetParameterId( "band3Frequency", id ) && AUDIO::CEqualizerEffect::GetParameterId( 3, AUDIO::CEqualizerEffect::BANDFIELD_FREQUENCY ) == id );
        ASSERT_TRUE( 3 * 5 + 2 == id );
        ASSERT_TRUE( equalizer.TryGetParameterId( "band7Q", id ) && 39 == id );

        ASSERT_TRUE( equalizer.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        const TSamples input = MakeNoise( 20000, 2, 61, 0.7f );
        TSamples output( input );
        Run( equalizer, output, 2, 300 );
        ASSERT_TRUE( IsSame( input, output ) );

        // the high pass, switched on, makes a difference at 20 Hz and none at 10 kHz
        equalizer.SetParameter( AUDIO::CEqualizerEffect::GetParameterId( 7, AUDIO::CEqualizerEffect::BANDFIELD_ENABLED ), 1.0f );
        equalizer.SetParameter( AUDIO::CEqualizerEffect::GetParameterId( 7, AUDIO::CEqualizerEffect::BANDFIELD_FREQUENCY ), 1000.0f );
        TSamples settle( 9600 * 2, 0.0f );
        Run( equalizer, settle, 2, 300 );
        const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( equalizer, 2, ImpulseFrames );
        ASSERT_TRUE( MeasuredAt( impulse, 50.0 ) < -20.0 && Near( MeasuredAt( impulse, 10000.0 ), 0.0, 0.2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: The number of bands is an option, limited to 1 to 16; the graphic presets have their bands at the center frequencies" )
    try
    {
        const char* counts[] = { "1", "3", "16", "0", "99", "x" };
        const UInt32 expected[] = { 1, 3, 16, 1, 16, 8 };
        for ( UInt32 i=0; i<6; ++i )
        {
            AUDIO::CEqualizerEffect equalizer( Bands( counts[ i ] ) );
            ASSERT_TRUE( expected[ i ] == equalizer.GetBandCount() && expected[ i ] * 5 == equalizer.GetParameterCount() );
        }

        CORE::CValueList options;
        options.Set( "preset", "graphic10" );
        AUDIO::CEqualizerEffect graphic( options );
        ASSERT_TRUE( 10 == graphic.GetBandCount() );
        ASSERT_TRUE( Near( graphic.GetParameter( AUDIO::CEqualizerEffect::GetParameterId( 5, AUDIO::CEqualizerEffect::BANDFIELD_FREQUENCY ) ), 1000.0, 1e-3 ) );
        CORE::CValueList options21;
        options21.Set( "preset", "graphic21" );
        AUDIO::CEqualizerEffect graphic21( options21 );
        ASSERT_TRUE( 21 == graphic21.GetBandCount() && 105 == graphic21.GetParameterCount() );
        CORE::CValueList options6;
        options6.Set( "preset", "graphic6" );
        AUDIO::CEqualizerEffect graphic6( options6 );
        ASSERT_TRUE( 6 == graphic6.GetBandCount() );

        // a band raised by 12 dB: +12 dB at its center, 2.5 dB an octave away (peaking filter of Q 1.414), about nothing four octaves away
        ASSERT_TRUE( PrepareSingleBand( graphic ) );
        graphic.SetParameter( AUDIO::CEqualizerEffect::GetParameterId( 5, AUDIO::CEqualizerEffect::BANDFIELD_GAIN ), 12.0f );
        TSamples settle( 9600 * 2, 0.0f );
        Run( graphic, settle, 2, 300 );
        const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( graphic, 2, ImpulseFrames );
        ASSERT_TRUE( Near( MeasuredAt( impulse, 1000.0 ), 12.0, 0.1 ) );
        ASSERT_TRUE( fabs( MeasuredAt( impulse, 62.0 ) ) < 1.0 && fabs( MeasuredAt( impulse, 16000.0 ) ) < 1.0 );
        ASSERT_TRUE( Near( MeasuredAt( impulse, 2000.0 ), 2.5, 0.1 ) && Near( MeasuredAt( impulse, 500.0 ), 2.5, 0.1 ) );     // neighbours overlap
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: A low frequency effects channel passes through unchanged, the others are equalized" )
    try
    {
        const UInt32 channels = 6;
        AUDIO::CEqualizerEffect equalizer( Bands( "1" ) );
        equalizer.SetBand( 0, true, AUDIO::FILTERBANDTYPE_HIGHPASS, 2000.0f, 0.0f, 0.7071f );
        AUDIO::CAudioFormat format( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 );
        ASSERT_TRUE( 0 != ( format.GetChannelLayout() & AUDIO::AUDIOSPEAKER_LOW_FREQUENCY ) );
        ASSERT_TRUE( equalizer.Prepare( format, 1024 ) );

        const TSamples input = MakeSine( 12000, channels, 100.0, 0.5f );
        TSamples output( input );
        Run( equalizer, output, channels, 256 );
        for ( size_t f=0; f<12000; ++f )
            ASSERT_TRUE( output[ f * channels + 3 ] == input[ f * channels + 3 ] );             // channel 3 is the LFE channel of 5.1
        ASSERT_TRUE( RmsOfChannel( output, channels, 0, 6000, 12000 ) < 0.01 * RmsOfChannel( input, channels, 0, 6000, 12000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformFilterResponseTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioFilter: The magnitude response of the 12 dB filter, for every type and a grid of frequency, gain and Q, is that of the analytic biquad design within 0.1 dB" )
    try
    {
        const Float64 frequencies[] = { 150.0, 1000.0, 7000.0 };
        const Float64 qs[] = { 0.5, 1.0, 4.0 };
        const Float64 gains[] = { -12.0, 9.0 };
        for ( UInt32 t=0; t<AUDIO::FILTERBANDTYPE_COUNT; ++t )
        {
            const AUDIO::TFilterBandType type = (AUDIO::TFilterBandType) t;
            Float64 worst = 0.0;
            for ( UInt32 fi=0; fi<3; ++fi )
                for ( UInt32 qi=0; qi<3; ++qi )
                    for ( UInt32 gi=0; gi<( HasGain( type ) ? 2U : 1U ); ++gi )
                    {
                        AUDIO::CFilterEffect filter;
                        filter.SetFilterType( type );
                        filter.SetFrequency( (Float32) frequencies[ fi ] );
                        filter.SetQ( (Float32) qs[ qi ] );
                        filter.SetGainInDecibels( (Float32) gains[ gi ] );
                        ASSERT_TRUE( filter.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
                        const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( filter, 2, ImpulseFrames );
                        const Float64 deviation = WorstDeviation( impulse, ReferenceResponse( type ), frequencies[ fi ], qs[ qi ], HasGain( type ) ? gains[ gi ] : 0.0 );
                        if ( deviation > worst ) worst = deviation;
                        if ( deviation > 0.1 )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioFilter: " << TypeName( type ) << " " << frequencies[ fi ] << " Hz Q " << qs[ qi ] << " gain " << gains[ gi ] << " dB deviates " << deviation << " dB" );
                        ASSERT_TRUE( deviation <= 0.1 );
                    }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioFilter: type " << TypeName( type ) << ", worst deviation from the analytic response " << worst << " dB" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFilter: The 24 dB per octave slope is -3 dB at the corner frequency, 24 dB per octave in the stop band, and the two Butterworth sections of the design" )
    try
    {
        const Float64 butterworthQ1 = 0.5411961001461970, butterworthQ2 = 1.3065629648763766;
        const Float64 corners[] = { 200.0, 1000.0, 5000.0 };
        for ( UInt32 pass=0; pass<2; ++pass )
        {
            const AUDIO::TFilterBandType type = 0 == pass ? AUDIO::FILTERBANDTYPE_LOWPASS : AUDIO::FILTERBANDTYPE_HIGHPASS;
            const AUDIO::TFilterResponse response = 0 == pass ? AUDIO::FILTERRESPONSE_LOWPASS : AUDIO::FILTERRESPONSE_HIGHPASS;
            for ( UInt32 c=0; c<3; ++c )
            {
                AUDIO::CFilterEffect filter;
                filter.SetFilterType( type );
                filter.SetFrequency( (Float32) corners[ c ] );
                filter.SetSlopeInDecibelsPerOctave( 24 );
                ASSERT_TRUE( filter.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
                const GUCEF::vector< Float64 > impulse = ImpulseResponseOf( filter, 2, ImpulseFrames );

                // -3.01 dB at the corner
                const Float64 atCorner = MeasuredAt( impulse, corners[ c ] );
                ASSERT_TRUE( Near( atCorner, -3.0103, 0.2 ) );

                // the product of the two analytic sections, everywhere
                Float64 worst = 0.0;
                for ( UInt32 p=0; p<ProbeCount; ++p )
                {
                    const Float64 expected = AUDIO::CBiquadDesign::GetMagnitudeInDecibels( response, SampleRate, corners[ c ], butterworthQ1, 0.0, ProbeFrequency( p ) ) +
                                             AUDIO::CBiquadDesign::GetMagnitudeInDecibels( response, SampleRate, corners[ c ], butterworthQ2, 0.0, ProbeFrequency( p ) );
                    if ( expected < -60.0 )
                        continue;
                    const Float64 deviation = fabs( expected - MeasuredAt( impulse, ProbeFrequency( p ) ) );
                    if ( deviation > worst ) worst = deviation;
                }
                ASSERT_TRUE( worst <= 0.1 );
            }
        }

        // slope far from the corner: a 200 Hz low pass at 1.6 kHz and 3.2 kHz, a 4 kHz high pass at 500 and 250 Hz
        AUDIO::CFilterEffect lowPass;
        lowPass.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS );
        lowPass.SetFrequency( 200.0f );
        lowPass.SetSlopeInDecibelsPerOctave( 24 );
        ASSERT_TRUE( lowPass.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        const GUCEF::vector< Float64 > lowImpulse = ImpulseResponseOf( lowPass, 2, ImpulseFrames );
        const Float64 lowSlope = MeasuredAt( lowImpulse, 1600.0 ) - MeasuredAt( lowImpulse, 3200.0 );
        AUDIO::CFilterEffect highPass;
        highPass.SetFilterType( AUDIO::FILTERBANDTYPE_HIGHPASS );
        highPass.SetFrequency( 4000.0f );
        highPass.SetSlopeInDecibelsPerOctave( 24 );
        ASSERT_TRUE( highPass.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        const GUCEF::vector< Float64 > highImpulse = ImpulseResponseOf( highPass, 2, ImpulseFrames );
        const Float64 highSlope = MeasuredAt( highImpulse, 500.0 ) - MeasuredAt( highImpulse, 250.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioFilter: 24 dB slope measured: low pass " << lowSlope << " dB per octave, high pass " << highSlope << " dB per octave" );
        ASSERT_TRUE( Near( lowSlope, 24.0, 1.0 ) && Near( highSlope, 24.0, 1.0 ) );

        // the 12 dB slope is half of that
        AUDIO::CFilterEffect gentle;
        gentle.SetFilterType( AUDIO::FILTERBANDTYPE_LOWPASS );
        gentle.SetFrequency( 200.0f );
        ASSERT_TRUE( gentle.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        const GUCEF::vector< Float64 > gentleImpulse = ImpulseResponseOf( gentle, 2, ImpulseFrames );
        ASSERT_TRUE( Near( MeasuredAt( gentleImpulse, 1600.0 ) - MeasuredAt( gentleImpulse, 3200.0 ), 12.0, 0.5 ) );

        // 24 dB for a peak: the gain at the center is the gain that was asked for
        AUDIO::CFilterEffect peak;
        peak.SetFilterType( AUDIO::FILTERBANDTYPE_PEAK );
        peak.SetFrequency( 1000.0f );
        peak.SetGainInDecibels( 12.0f );
        peak.SetQ( 1.0f );
        peak.SetSlopeInDecibelsPerOctave( 24 );
        ASSERT_TRUE( peak.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 ) );
        ASSERT_TRUE( Near( MeasuredAt( ImpulseResponseOf( peak, 2, ImpulseFrames ), 1000.0 ), 12.0, 0.1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFilter: Defaults, ranges and the slope parameter" )
    try
    {
        AUDIO::CFilterEffect filter;
        ASSERT_TRUE( filter.GetEffectTypeName() == "filter" && 5 == filter.GetParameterCount() );
        ASSERT_TRUE( (Float32) AUDIO::FILTERBANDTYPE_LOWPASS == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_TYPE ) );
        ASSERT_TRUE( 1000.0f == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_FREQUENCY ) );
        ASSERT_TRUE( 12.0f == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_SLOPE ) );
        filter.SetFrequency( 5.0f );
        ASSERT_TRUE( 20.0f == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_FREQUENCY ) );
        filter.SetQ( 100.0f );
        ASSERT_TRUE( 18.0f == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_Q ) );
        filter.SetSlopeInDecibelsPerOctave( 48 );
        ASSERT_TRUE( 24.0f == filter.GetParameter( AUDIO::CFilterEffect::PARAMETER_SLOPE ) );
        ASSERT_TRUE( 0 == filter.GetLatencyInFrames() && 0 == filter.GetTailInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSmoothingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: Jumps of gain, frequency, Q, type and enabled state make no clicks: the largest step stays within 30 percent of that of the steady outputs (more where a peak sweeps over the signal)" )
    try
    {
        const SClickScenario scenarios[] =
        {
            { "gain 0 to +24 dB", BandPeakFlat, BandPeakUp, 1.3 },
            { "gain -24 to +24 dB", BandPeakDown, BandPeakUp, 1.3 },
            { "frequency 200 Hz to 8 kHz", BandPeakLow, BandPeakHigh, 5.0 },      // the +12 dB peak passes over the 1 kHz of the signal on its way: up to 4 times louder
            { "Q 0.1 to 18", BandWide, BandNarrow, 1.6 },                         // the state of the filter swings while its Q moves
            { "switched on", BandOff, BandOn, 1.3 },
            { "switched off", BandOn, BandOff, 1.3 },
            { "low pass to high pass", BandLowPass, BandHighPass, 1.3 },
            { "low shelf to high shelf", BandLowShelf, BandHighShelf, 1.3 }
        };
        for ( UInt32 s=0; s<8; ++s )
        {
            Float64 ratio = 0.0;
            const bool ok = CheckNoClicks< AUDIO::CEqualizerEffect >( scenarios[ s ], Bands( "1" ), 1000.0, ratio );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEqualizer: jump \"" << scenarios[ s ].name << "\": largest step " << ratio << " times that of the steady output" );
            ASSERT_TRUE( ok );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioFilter: Jumps of frequency, Q, type and slope make no clicks: the largest step stays within 30 percent of that of the steady outputs" )
    try
    {
        const SClickScenario scenarios[] =
        {
            { "frequency 200 Hz to 8 kHz", FilterLow, FilterHigh, 1.3 },
            { "frequency 8 kHz to 200 Hz", FilterHigh, FilterLow, 1.3 },
            { "low pass to high pass", FilterHigh, FilterHighPass, 1.3 },
            { "12 to 24 dB per octave", FilterGentle, FilterSteep, 1.3 },
            { "24 to 12 dB per octave", FilterSteep, FilterGentle, 1.3 },
            { "Q 0.7 to 8", FilterHigh, FilterResonant, 1.3 },
            { "band pass to notch", FilterBandPass, FilterNotch, 1.3 }
        };
        for ( UInt32 s=0; s<7; ++s )
        {
            Float64 ratio = 0.0;
            const bool ok = CheckNoClicks< AUDIO::CFilterEffect >( scenarios[ s ], CORE::CValueList(), 1000.0, ratio );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioFilter: jump \"" << scenarios[ s ].name << "\": largest step " << ratio << " times that of the steady output" );
            ASSERT_TRUE( ok );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEqualizer: Random changes of every band at every block for 60 s leave the output finite and bounded" )
    try
    {
        AUDIO::CEqualizerEffect equalizer;
        ASSERT_TRUE( equalizer.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 256 ) );
        AUDIO::CFilterEffect filter;
        ASSERT_TRUE( filter.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 256 ) );

        CXorShift random( 424242 );
        CXorShift noise( 9 );
        TSamples block( 256 * 2, 0.0f );
        Float32 largest = 0.0f;
        bool finite = true;
        for ( UInt32 b=0; b<(UInt32) ( 60 * 48000 / 256 ); ++b )
        {
            for ( UInt32 band=0; band<8; ++band )
                equalizer.SetBand( band, random.NextUnit() < 0.8, (AUDIO::TFilterBandType) ( random.Next() % AUDIO::FILTERBANDTYPE_COUNT ),
                                   (Float32) ( 20.0 * pow( 1000.0, random.NextUnit() ) ), (Float32) ( -24.0 + 48.0 * random.NextUnit() ), (Float32) ( 0.1 * pow( 180.0, random.NextUnit() ) ) );
            filter.SetFilterType( (AUDIO::TFilterBandType) ( random.Next() % AUDIO::FILTERBANDTYPE_COUNT ) );
            filter.SetFrequency( (Float32) ( 20.0 * pow( 1000.0, random.NextUnit() ) ) );
            filter.SetQ( (Float32) ( 0.1 * pow( 180.0, random.NextUnit() ) ) );
            filter.SetGainInDecibels( (Float32) ( -24.0 + 48.0 * random.NextUnit() ) );
            filter.SetSlopeInDecibelsPerOctave( random.Next() & 1 ? 24 : 12 );

            for ( size_t i=0; i<block.size(); ++i )
                block[ i ] = 0.3f * noise.NextFloat();
            TSamples second( block );
            ProcessRealtime( equalizer, &block[ 0 ], 256 );
            ProcessRealtime( filter, &second[ 0 ], 256 );
            const Float32 peak = PeakOf( block ) > PeakOf( second ) ? PeakOf( block ) : PeakOf( second );
            if ( !( peak <= 1.0e7f ) )
                finite = false;
            if ( peak > largest )
                largest = peak;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEqualizer: 60 s of random modulation, largest output sample " << largest );
        ASSERT_TRUE( finite );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioEqualizerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioEqualizer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Equalizer" );

    PerformEqualizerResponseTests();
    PerformFilterResponseTests();
    PerformSmoothingTests();
}

/*-------------------------------------------------------------------------*/
