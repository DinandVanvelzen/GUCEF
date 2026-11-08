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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#include "gucefAUDIO_CAudioEffectChain.h"
#define GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTCHAIN_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#include "gucefAUDIO_CAudioParameterBlock.h"
#define GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H
#endif /* GUCEF_AUDIO_CAUDIOPARAMETERBLOCK_H ? */

#ifndef GUCEF_AUDIO_CSVFFILTER_H
#include "gucefAUDIO_CSvfFilter.h"
#define GUCEF_AUDIO_CSVFFILTER_H
#endif /* GUCEF_AUDIO_CSVFFILTER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioEffects.h"

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

const AUDIO::CAudioFormat StereoFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 );

const Float64 Pi = 3.14159265358979323846;

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
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

GUCEF::vector< Float32 >
MakeNoise( UInt32 frames, UInt32 channels, UInt32 seed )
{
    CXorShift random( seed );
    GUCEF::vector< Float32 > noise( (size_t) frames * channels, 0.0f );
    for ( size_t i=0; i<noise.size(); ++i )
        noise[ i ] = 0.5f * random.NextFloat();
    return noise;
}

GUCEF::vector< Float32 >
MakeSine( UInt32 frames, UInt32 channels, Float64 frequency, Float32 amplitude )
{
    GUCEF::vector< Float32 > sine( (size_t) frames * channels, 0.0f );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<channels; ++c )
            sine[ (size_t) f * channels + c ] = amplitude * (Float32) sin( 2.0 * Pi * frequency * (Float64) f / 48000.0 );
    return sine;
}

/** Largest absolute second difference of one channel of interleaved audio */
Float32
MaxSecondDifference( const GUCEF::vector< Float32 >& audio, UInt32 channels, UInt32 channel )
{
    Float32 largest = 0.0f;
    const size_t frames = audio.size() / channels;
    for ( size_t f=2; f<frames; ++f )
    {
        const Float32 d = audio[ f * channels + channel ] - 2.0f * audio[ ( f - 1 ) * channels + channel ] + audio[ ( f - 2 ) * channels + channel ];
        if ( fabsf( d ) > largest )
            largest = fabsf( d );
    }
    return largest;
}

/** Writes parameters from a thread of its own while the test thread processes */
struct SWriter
{
    CTestAudioEffect* effect;
    UInt32 firstValue;
    UInt32 publications;
    MT::CTAtomic< bool > done;
    SWriter( void ) : effect( GUCEF_NULL ), firstValue( 0 ), publications( 0 ), done( false ) {}
};

UInt32 GUCEF_CALLSPEC_PREFIX
WriterMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SWriter* writer = static_cast< SWriter* >( data );
    const UInt32 ids[ 2 ] = { CTestAudioEffect::PARAMETER_PAIR_A, CTestAudioEffect::PARAMETER_PAIR_B };
    for ( UInt32 i=0; i<writer->publications; ++i )
    {
        const Float32 value = (Float32) ( writer->firstValue + i );
        const Float32 values[ 2 ] = { value, value };
        writer->effect->SetParameters( ids, values, 2 );
        writer->effect->SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.5f + 0.25f * (Float32) ( i % 3 ) );
    }
    writer->done.Store( true, MT::MEMORDER_RELEASE );
    return 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Parameter descriptors limit, round and name their values" )
    try
    {
        const AUDIO::CAudioEffectParameterInfo* infos = CTestAudioEffect::GetStaticParameterInfos();
        const AUDIO::CAudioEffectParameterInfo& gain = infos[ CTestAudioEffect::PARAMETER_GAIN ];
        ASSERT_TRUE( gain.GetName() == "gain" && AUDIO::AUDIOPARAMUNIT_LINEAR_GAIN == gain.GetUnit() && 1.0f == gain.GetDefaultValue() );
        ASSERT_TRUE( 4.0f == gain.Clamp( 9.0f ) && 0.0f == gain.Clamp( -1.0f ) && 0.25f == gain.Clamp( 0.25f ) );
        const Float32 zero = 0.0f;
        ASSERT_TRUE( 1.0f == gain.Clamp( zero / zero ) );                                  // not a number gives the default

        const AUDIO::CAudioEffectParameterInfo& pair = infos[ CTestAudioEffect::PARAMETER_PAIR_A ];
        ASSERT_TRUE( 3.0f == pair.Clamp( 2.6f ) && 2.0f == pair.Clamp( 2.4f ) );           // integers round

        const AUDIO::CAudioEffectParameterInfo& mode = infos[ CTestAudioEffect::PARAMETER_MODE ];
        ASSERT_TRUE( AUDIO::AUDIOPARAMKIND_ENUMERATION == mode.GetKind() && 2 == mode.GetEnumLabelCount() );
        ASSERT_TRUE( mode.GetEnumLabel( 1 ) == "inverted" && mode.GetEnumLabel( 7 ).IsNULLOrEmpty() );
        Float32 value = -1.0f;
        ASSERT_TRUE( mode.TryGetEnumValue( "INVERTED", value ) && 1.0f == value );
        ASSERT_FALSE( mode.TryGetEnumValue( "sideways", value ) );
        ASSERT_TRUE( 1.0f == mode.Clamp( 5.0f ) && 0.0f == mode.GetMinimum() && 1.0f == mode.GetMaximum() );

        const AUDIO::CAudioEffectParameterInfo& bypass = infos[ CTestAudioEffect::PARAMETER_BYPASS ];
        ASSERT_TRUE( 1.0f == bypass.Clamp( 0.7f ) && 0.0f == bypass.Clamp( 0.2f ) );

        AUDIO::CAudioEffectParameterInfo swapped( 9, "swapped", AUDIO::AUDIOPARAMUNIT_DECIBEL, AUDIO::AUDIOPARAMSCALE_LINEAR, 6.0f, -6.0f, 20.0f );
        ASSERT_TRUE( -6.0f == swapped.GetMinimum() && 6.0f == swapped.GetMaximum() && 6.0f == swapped.GetDefaultValue() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: The parameter block publishes values to its reader only when complete, and only once" )
    try
    {
        AUDIO::CAudioParameterBlock block( 3 );
        ASSERT_TRUE( 3 == block.GetCount() && 0 == block.GetPublicationCount() );
        ASSERT_FALSE( block.Refresh() );                                                    // nothing published yet

        block.Set( 1, 0.5f );
        ASSERT_TRUE( 0.5f == block.Get( 1 ) && 0.0f == block.Read( 1 ) );                  // the reader has not looked yet
        ASSERT_TRUE( block.Refresh() && 0.5f == block.Read( 1 ) );
        ASSERT_FALSE( block.Refresh() );                                                    // seen already

        const UInt32 ids[ 3 ] = { 0, 2, 7 };
        const Float32 values[ 3 ] = { 1.5f, 2.5f, 9.0f };
        block.Set( ids, values, 3 );                                                       // index 7 does not exist, it is ignored
        ASSERT_TRUE( 2 == block.GetPublicationCount() );
        ASSERT_TRUE( block.Refresh() && 1.5f == block.Read( 0 ) && 0.5f == block.Read( 1 ) && 2.5f == block.Read( 2 ) );
        ASSERT_TRUE( 0.0f == block.Read( 7 ) && 0.0f == block.Get( 7 ) );

        block.Set( 0, 3.0f );
        block.SynchronizeSnapshot();
        ASSERT_TRUE( 3.0f == block.Read( 0 ) );
        ASSERT_FALSE( block.Refresh() );                                                    // synchronised: nothing new
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Smoothers ramp linearly in a fixed time, approach exponentially, and per octave for frequencies" )
    try
    {
        // linear: 100 steps of 1 frame, the target exactly at the end
        AUDIO::CAudioParameterSmoother ramp;
        ramp.Prepare( AUDIO::AUDIOSMOOTHING_LINEAR_RAMP, 100.0 / 48000.0, 48000.0 );
        ramp.Reset( 0.0f );
        ramp.SetTarget( 1.0f );
        ASSERT_TRUE( ramp.IsSmoothing() );
        Float32 v = 0.0f;
        for ( UInt32 i=0; i<50; ++i )
            v = ramp.Step();
        ASSERT_TRUE( Near( 0.5, v, 1e-6 ) );
        for ( UInt32 i=0; i<50; ++i )
            v = ramp.Step();
        ASSERT_TRUE( 1.0f == v && !ramp.IsSmoothing() && 1.0f == ramp.GetValue() );
        ramp.SetTargetInSteps( 0.0f, 4 );
        ASSERT_TRUE( Near( 0.75, ramp.Step(), 1e-6 ) );

        // one pole of 20 ms on 32 frame steps: 63 % of the way after one time constant
        AUDIO::CAudioParameterSmoother pole;
        pole.Prepare( AUDIO::AUDIOSMOOTHING_ONE_POLE, 0.020, 48000.0, false, 32 );
        ASSERT_TRUE( 32 == pole.GetStepFrames() );
        pole.Reset( 0.0f );
        pole.SetTarget( 1.0f );
        for ( UInt32 i=0; i<30; ++i )                                                       // 30 steps of 32 frames are 20 ms
            v = pole.Step();
        ASSERT_TRUE( Near( 1.0 - exp( -1.0 ), v, 0.01 ) );
        for ( UInt32 i=0; i<3000 && pole.IsSmoothing(); ++i )
            pole.Step();
        ASSERT_TRUE( !pole.IsSmoothing() && 1.0f == pole.GetValue() );                     // arrives exactly

        // logarithmic: half way in time is half way in octaves, the geometric mean
        AUDIO::CAudioParameterSmoother frequency;
        frequency.Prepare( AUDIO::AUDIOSMOOTHING_LINEAR_RAMP, 10.0 / 48000.0, 48000.0, true );
        frequency.Reset( 100.0f );
        frequency.SetTarget( 6400.0f );
        for ( UInt32 i=0; i<5; ++i )
            v = frequency.Step();
        ASSERT_TRUE( Near( 800.0, v, 0.01 ) );

        // no smoothing: the value jumps
        AUDIO::CAudioParameterSmoother none;
        none.Prepare( AUDIO::AUDIOSMOOTHING_NONE, 0.0, 48000.0 );
        none.SetTarget( 3.0f );
        ASSERT_TRUE( 3.0f == none.GetValue() && !none.IsSmoothing() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: A smoothed gain step from -60 dB to 0 dB does not zipper, a smoothed filter frequency step does not overshoot" )
    try
    {
        // gain: y = g x with g ramped per frame over 20 ms. No step of y is larger than the steepest step of the sine plus the ramp increment
        const GUCEF::vector< Float32 > sine = MakeSine( 4800, 1, 1000.0, 1.0f );
        AUDIO::CAudioParameterSmoother gain;
        gain.Prepare( AUDIO::AUDIOSMOOTHING_LINEAR_RAMP, 0.020, 48000.0 );
        gain.Reset( (Float32) pow( 10.0, -60.0 / 20.0 ) );
        const Float32 increment = ( 1.0f - gain.GetValue() ) / 960.0f;
        gain.SetTarget( 1.0f );
        Float32 sineStep = 0.0f, gainedStep = 0.0f, previous = 0.0f;
        for ( size_t i=0; i<sine.size(); ++i )
        {
            const Float32 y = gain.Step() * sine[ i ];
            if ( i > 0 )
            {
                sineStep = fabsf( sine[ i ] - sine[ i - 1 ] ) > sineStep ? fabsf( sine[ i ] - sine[ i - 1 ] ) : sineStep;
                gainedStep = fabsf( y - previous ) > gainedStep ? fabsf( y - previous ) : gainedStep;
            }
            previous = y;
        }
        ASSERT_TRUE( gainedStep <= sineStep + increment + 1e-6f );

        // filter: a low pass on a 1 kHz sine, its frequency stepped from 200 Hz to 5 kHz and smoothed per octave on 32 frame
        // sub-blocks. The output never goes over the input peak + 1 dB
        AUDIO::CSvfFilter filter;
        filter.Prepare( 1 );
        AUDIO::CAudioParameterSmoother cutoff;
        cutoff.Prepare( AUDIO::AUDIOSMOOTHING_ONE_POLE, 0.020, 48000.0, true, 32 );
        cutoff.Reset( 200.0f );
        filter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, 48000.0, 200.0, 0.7071 );
        const GUCEF::vector< Float32 > input = MakeSine( 24000, 1, 1000.0, 0.5f );
        Float32 peak = 0.0f;
        for ( UInt32 f=0; f<24000; ++f )
        {
            if ( 4800 == f )
                cutoff.SetTarget( 5000.0f );
            if ( 0 == f % 32 && cutoff.IsSmoothing() )
                filter.SetResponse( AUDIO::FILTERRESPONSE_LOWPASS, 48000.0, (Float64) cutoff.Step(), 0.7071 );
            const Float32 y = filter.Process( 0, input[ f ] );
            peak = fabsf( y ) > peak ? fabsf( y ) : peak;
        }
        ASSERT_TRUE( peak <= 0.5f * (Float32) pow( 10.0, 1.0 / 20.0 ) );
        ASSERT_TRUE( peak > 0.45f );                                                        // the sine did get through after the step
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEffectTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEffects: An effect has parameters by id and name, limited to their range, and is prepared for a format" )
    try
    {
        CTestAudioEffect effect;
        ASSERT_TRUE( effect.GetEffectTypeName() == "testeffect" && CTestAudioEffect::PARAMETER_COUNT == effect.GetParameterCount() );
        ASSERT_TRUE( GUCEF_NULL == effect.GetParameterInfo( 99 ) && effect.GetParameterInfo( 1 )->GetName() == "cutoff" );
        UInt32 id = 0;
        ASSERT_TRUE( effect.TryGetParameterId( "PairB", id ) && CTestAudioEffect::PARAMETER_PAIR_B == id );
        ASSERT_FALSE( effect.TryGetParameterId( "nope", id ) );
        ASSERT_TRUE( 1.0f == effect.GetParameter( CTestAudioEffect::PARAMETER_GAIN ) );     // the default
        effect.SetParameter( CTestAudioEffect::PARAMETER_GAIN, 10.0f );
        ASSERT_TRUE( 4.0f == effect.GetParameter( CTestAudioEffect::PARAMETER_GAIN ) );
        ASSERT_TRUE( GUCEF_NULL == effect.GetFactory() && 0 == effect.GetLatencyInFrames() && 0 == effect.GetTailInFrames() );

        // not prepared: audio passes unchanged
        GUCEF::vector< Float32 > audio = MakeNoise( 64, 2, 7 );
        const GUCEF::vector< Float32 > original = audio;
        ASSERT_FALSE( effect.IsPrepared() );
        effect.Process( &audio[ 0 ], 64 );
        ASSERT_TRUE( 0 == memcmp( &audio[ 0 ], &original[ 0 ], audio.size() * sizeof( Float32 ) ) );

        ASSERT_FALSE( effect.Prepare( AUDIO::CAudioFormat(), 256 ) );
        ASSERT_TRUE( effect.Prepare( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_INT16 ), 256 ) );
        ASSERT_TRUE( AUDIO::SAMPLETYPE_FLOAT32 == effect.GetFormat().GetSampleType() && 256 == effect.GetMaxBlockFrames() );
        ASSERT_TRUE( effect.IsPreparedFor( StereoFormat, 256 ) && effect.IsPreparedFor( StereoFormat, 100 ) );
        ASSERT_FALSE( effect.IsPreparedFor( StereoFormat, 512 ) );
        ASSERT_FALSE( effect.IsPreparedFor( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 256 ) );
        ASSERT_FALSE( effect.IsPreparedFor( AUDIO::CAudioFormat( 48000, 6, AUDIO::SAMPLETYPE_FLOAT32 ), 256 ) );

        // more frames than prepared for: refused, the audio is left alone
        GUCEF::vector< Float32 > big = MakeNoise( 300, 2, 8 );
        const GUCEF::vector< Float32 > bigOriginal = big;
        effect.Process( &big[ 0 ], 300 );
        ASSERT_TRUE( 0 == memcmp( &big[ 0 ], &bigOriginal[ 0 ], big.size() * sizeof( Float32 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Effects save and load their parameters by name, enumerations by label" )
    try
    {
        CTestAudioEffect effect;
        effect.SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.75f );
        effect.SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 1234.0f );
        effect.SetParameter( CTestAudioEffect::PARAMETER_MODE, 1.0f );
        effect.SetParameter( CTestAudioEffect::PARAMETER_BYPASS, 1.0f );
        CORE::CDataNode config( "Effect" );
        ASSERT_TRUE( effect.SaveConfig( config ) );
        ASSERT_TRUE( config.GetAttributeValue( "type" ).AsString() == "testeffect" );
        const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
        ASSERT_TRUE( GUCEF_NULL != parameters );
        ASSERT_TRUE( parameters->GetAttributeValue( "mode" ).AsString() == "inverted" );

        CTestAudioEffect other;
        other.SetParameter( CTestAudioEffect::PARAMETER_PAIR_A, 42.0f );                     // not in the configuration below: kept
        CORE::CDataNode partial( config );
        partial.FindChild( "Parameters" )->DelAttribute( "pairA" );
        partial.FindChild( "Parameters" )->SetAttribute( "noSuchParameter", 5.0f );         // ignored, with a log line
        ASSERT_TRUE( other.LoadConfig( partial ) );
        ASSERT_TRUE( 0.75f == other.GetParameter( CTestAudioEffect::PARAMETER_GAIN ) );
        ASSERT_TRUE( 1234.0f == other.GetParameter( CTestAudioEffect::PARAMETER_CUTOFF ) );
        ASSERT_TRUE( 1.0f == other.GetParameter( CTestAudioEffect::PARAMETER_MODE ) );
        ASSERT_TRUE( 1.0f == other.GetParameter( CTestAudioEffect::PARAMETER_BYPASS ) );
        ASSERT_TRUE( 42.0f == other.GetParameter( CTestAudioEffect::PARAMETER_PAIR_A ) );

        // numbers for enumerations work as well, values are limited on load
        CORE::CDataNode numeric( "Effect" );
        numeric.FindOrAddChild( "Parameters" )->SetAttribute( "mode", (Int32) 0 );
        numeric.FindOrAddChild( "Parameters" )->SetAttribute( "gain", 100.0f );
        ASSERT_TRUE( other.LoadConfig( numeric ) );
        ASSERT_TRUE( 0.0f == other.GetParameter( CTestAudioEffect::PARAMETER_MODE ) && 4.0f == other.GetParameter( CTestAudioEffect::PARAMETER_GAIN ) );

        // the configuration of another kind of effect is refused
        CORE::CDataNode foreign( "Effect" );
        foreign.SetAttribute( "type", "reverb" );
        ASSERT_FALSE( other.LoadConfig( foreign ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Output does not depend on how the frames are split into calls (10 s of noise, random block sizes)" )
    try
    {
        const UInt32 frames = 480000;
        const GUCEF::vector< Float32 > noise = MakeNoise( frames, 2, 12345 );

        CTestAudioEffect whole;
        ASSERT_TRUE( whole.Prepare( StereoFormat, frames ) );
        CTestAudioEffect pieces;
        ASSERT_TRUE( pieces.Prepare( StereoFormat, 4096 ) );

        // parameters change after preparing, so both start with the same smoothing
        whole.SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.5f );
        whole.SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 800.0f );
        pieces.SetParameter( CTestAudioEffect::PARAMETER_GAIN, 0.5f );
        pieces.SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 800.0f );

        GUCEF::vector< Float32 > a = noise;
        whole.Process( &a[ 0 ], frames );

        GUCEF::vector< Float32 > b = noise;
        CXorShift random( 99 );
        UInt32 done = 0;
        while ( done < frames )
        {
            UInt32 n = 1 + random.Next() % 4096;
            if ( n > frames - done )
                n = frames - done;
            pieces.Process( &b[ (size_t) done * 2 ], n );
            done += n;
        }
        ASSERT_TRUE( 0 == memcmp( &a[ 0 ], &b[ 0 ], a.size() * sizeof( Float32 ) ) );
        ASSERT_TRUE( 0 != memcmp( &a[ 0 ], &noise[ 0 ], a.size() * sizeof( Float32 ) ) );   // it did change the audio
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Parameters set from two threads while the audio is processed are seen complete, pairs never mixed" )
    try
    {
        CTestAudioEffect effect;
        ASSERT_TRUE( effect.Prepare( StereoFormat, 64 ) );
        GUCEF::vector< Float32 > block( 64 * 2, 0.0f );

        SWriter writers[ 2 ];
        MT::SThreadData* threads[ 2 ] = { GUCEF_NULL, GUCEF_NULL };
        for ( UInt32 w=0; w<2; ++w )
        {
            writers[ w ].effect = &effect;
            writers[ w ].firstValue = 1 + w * 1000000;
            writers[ w ].publications = 20000;
            threads[ w ] = MT::ThreadDataReserve();
            ASSERT_TRUE( 0 != MT::ThreadCreate( threads[ w ], &WriterMain, &writers[ w ] ) );
        }

        // the test thread is the audio thread
        UInt32 blocks = 0;
        while ( !writers[ 0 ].done.Load( MT::MEMORDER_ACQUIRE ) || !writers[ 1 ].done.Load( MT::MEMORDER_ACQUIRE ) )
        {
            effect.Process( &block[ 0 ], 64 );
            ++blocks;
        }
        effect.Process( &block[ 0 ], 64 );
        for ( UInt32 w=0; w<2; ++w )
        {
            MT::ThreadWait( threads[ w ], 60000 );
            MT::ThreadDataCleanup( threads[ w ] );
        }

        ASSERT_TRUE( effect.GetPairCheckCount() > 0 );
        ASSERT_TRUE( 0 == effect.GetMixedPairCount() );
        ASSERT_TRUE( effect.GetParameter( CTestAudioEffect::PARAMETER_PAIR_A ) == effect.GetParameter( CTestAudioEffect::PARAMETER_PAIR_B ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioEffects: " << blocks << " blocks processed, parameters seen " << effect.GetPairCheckCount() << " times while two threads set them" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformChainTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEffects: An empty chain and a chain of disabled slots leave the audio exactly as it is" )
    try
    {
        AUDIO::CAudioEffectChain chain;
        ASSERT_TRUE( chain.Prepare( 4, StereoFormat, 256 ) );
        ASSERT_TRUE( 4 == chain.GetSlotCapacity() && 0 == chain.GetEffectCount() );
        GUCEF::vector< Float32 > audio = MakeNoise( 256, 2, 3 );
        const GUCEF::vector< Float32 > original = audio;
        chain.Process( &audio[ 0 ], 256 );
        ASSERT_TRUE( 0 == memcmp( &audio[ 0 ], &original[ 0 ], audio.size() * sizeof( Float32 ) ) );

        CORE::CValueList invert;
        invert.Set( "invert", "true" );
        CTestAudioEffect first( invert ), second( invert );
        ASSERT_TRUE( first.Prepare( StereoFormat, 256 ) && second.Prepare( StereoFormat, 256 ) );
        chain.SetEnabled( 0, false );
        chain.SetEnabled( 3, false );
        ASSERT_TRUE( chain.SetSlotEffect( 0, &first ) && chain.SetSlotEffect( 3, &second ) );
        ASSERT_FALSE( chain.SetSlotEffect( 4, &second ) );                                  // no such slot
        ASSERT_TRUE( 2 == chain.GetEffectCount() && &second == chain.GetSlotEffect( 3 ) && !chain.IsEnabled( 0 ) );
        for ( UInt32 i=0; i<5; ++i )
        {
            chain.Process( &audio[ 0 ], 256 );
            ASSERT_TRUE( 0 == memcmp( &audio[ 0 ], &original[ 0 ], audio.size() * sizeof( Float32 ) ) );
        }
        ASSERT_TRUE( 0 == first.GetPairCheckCount() );                                      // never processed
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Wet and dry levels mix processed and unprocessed audio exactly, changes ramp over the block" )
    try
    {
        AUDIO::CAudioEffectChain chain;
        ASSERT_TRUE( chain.Prepare( 2, StereoFormat, 256 ) );
        CORE::CValueList invert;
        invert.Set( "invert", "true" );
        CTestAudioEffect effect( invert );
        ASSERT_TRUE( effect.Prepare( StereoFormat, 256 ) );
        chain.SetMix( 1, 0.25f, 0.5f );
        ASSERT_TRUE( 0.25f == chain.GetWet( 1 ) && 0.5f == chain.GetDry( 1 ) );
        ASSERT_TRUE( chain.SetSlotEffect( 1, &effect ) );

        const GUCEF::vector< Float32 > input = MakeNoise( 256, 2, 4 );
        GUCEF::vector< Float32 > audio = input;
        chain.Process( &audio[ 0 ], 256 );
        bool exact = true;
        for ( size_t i=0; i<audio.size(); ++i )
            exact = exact && audio[ i ] == 0.25f * ( -input[ i ] ) + 0.5f * input[ i ];
        ASSERT_TRUE( exact );

        // back to all wet: ramped over one block, then the effect alone
        chain.SetMix( 1, 1.0f, 0.0f );
        audio = input;
        chain.Process( &audio[ 0 ], 256 );
        ASSERT_TRUE( Near( 0.25 * -input[ 0 ] + 0.5 * input[ 0 ], audio[ 0 ], 0.02 ) );
        ASSERT_TRUE( audio[ 510 ] == -input[ 510 ] );
        audio = input;
        chain.Process( &audio[ 0 ], 256 );
        exact = true;
        for ( size_t i=0; i<audio.size(); ++i )
            exact = exact && audio[ i ] == -input[ i ];
        ASSERT_TRUE( exact );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Switching a slot crossfades over 10 ms without a jump; a switched off effect is not processed and starts clean" )
    try
    {
        AUDIO::CAudioEffectChain chain;
        ASSERT_TRUE( chain.Prepare( 1, StereoFormat, 128 ) );
        CORE::CValueList invert;
        invert.Set( "invert", "true" );
        CTestAudioEffect effect( invert );
        ASSERT_TRUE( effect.Prepare( StereoFormat, 128 ) );
        ASSERT_TRUE( chain.SetSlotEffect( 0, &effect ) );

        // a 440 Hz sine, the effect turns it upside down: switching without a crossfade would jump by twice the amplitude
        const UInt32 frames = 128 * 60;
        const GUCEF::vector< Float32 > input = MakeSine( frames, 2, 440.0, 0.5f );
        GUCEF::vector< Float32 > output = input;
        const UInt32 resetsBefore = effect.GetResetCount();
        for ( UInt32 b=0; b<60; ++b )
        {
            if ( 10 == b ) chain.SetEnabled( 0, false );
            if ( 30 == b ) chain.SetEnabled( 0, true );
            chain.Process( &output[ (size_t) b * 128 * 2 ], 128 );
        }
        ASSERT_TRUE( MaxSecondDifference( output, 2, 0 ) <= 2.0f * MaxSecondDifference( input, 2, 0 ) + 0.002f );

        // the fade out takes 480 frames, after that the input passes untouched
        const size_t off = (size_t) 10 * 128;
        ASSERT_TRUE( output[ ( off + 100 ) * 2 ] != input[ ( off + 100 ) * 2 ] );          // still fading
        bool untouched = true;
        for ( size_t f=off + 482; f<(size_t) 30 * 128; ++f )
            untouched = untouched && output[ f * 2 ] == input[ f * 2 ];
        ASSERT_TRUE( untouched );

        // the fade in started from a clean effect, and ends fully processed
        ASSERT_TRUE( effect.GetResetCount() == resetsBefore + 2 );                          // once when placed, once when switched on again
        ASSERT_TRUE( output[ ( (size_t) 59 * 128 ) * 2 ] == -input[ ( (size_t) 59 * 128 ) * 2 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioEffects: Factories register by type name, count their effects, and cannot be unregistered while effects live" )
    try
    {
        AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
        AUDIO::CAudioEffectRegistry::TRegisteredFactoryPtr found;
        ASSERT_FALSE( registry.TryFindFactory( "testeffect", found ) );
        ASSERT_TRUE( registry.CreateEffect( "testeffect" ).IsNULL() );

        AUDIO::TAudioEffectFactoryPtr factory( GUCEF_NEW TTestAudioEffectFactory() );
        ASSERT_TRUE( factory->GetEffectTypeName() == "testeffect" && CTestAudioEffect::PARAMETER_COUNT == factory->GetParameterCount() );
        ASSERT_TRUE( factory->GetParameterInfo( 0 )->GetName() == "gain" && GUCEF_NULL == factory->GetParameterInfo( 99 ) );
        ASSERT_TRUE( registry.RegisterFactory( factory ) );
        ASSERT_FALSE( registry.RegisterFactory( AUDIO::TAudioEffectFactoryPtr( GUCEF_NEW TTestAudioEffectFactory() ) ) );   // the name is taken
        ASSERT_TRUE( registry.TryFindFactory( "TestEffect", found ) && found.GetPointerAlways() == factory.GetPointerAlways() );

        CORE::CValueList options;
        options.Set( "invert", "true" );
        AUDIO::TAudioEffectPtr effect = registry.CreateEffect( "TESTEFFECT", options );
        ASSERT_FALSE( effect.IsNULL() );
        ASSERT_TRUE( effect->GetFactory() == factory.GetPointerAlways() && 1 == factory->GetLiveInstanceCount() );
        AUDIO::TAudioEffectPtr second = factory->Create();
        ASSERT_TRUE( 2 == factory->GetLiveInstanceCount() );

        // a plugin that wants to unload is refused while its effects exist
        ASSERT_FALSE( registry.TryUnregisterFactory( "testeffect" ) );
        ASSERT_TRUE( registry.TryFindFactory( "testeffect", found ) );
        effect.Unlink();
        ASSERT_FALSE( registry.TryUnregisterFactory( "testeffect" ) );
        second.Unlink();
        ASSERT_TRUE( 0 == factory->GetLiveInstanceCount() );
        ASSERT_TRUE( registry.TryUnregisterFactory( "testeffect" ) );
        ASSERT_FALSE( registry.TryFindFactory( "testeffect", found ) );
        ASSERT_FALSE( registry.TryUnregisterFactory( "testeffect" ) );                      // not there any more
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioEffectTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioEffects TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Effects" );

    PerformParameterTests();
    PerformEffectTests();
    PerformChainTests();
    PerformRegistryTests();
}

/*-------------------------------------------------------------------------*/
