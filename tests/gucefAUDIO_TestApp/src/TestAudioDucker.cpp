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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#include "gucefAUDIO_CAudioBusLayout.h"
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#include "gucefAUDIO_CDuckerEffect.h"
#define GUCEF_AUDIO_CDUCKEREFFECT_H
#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioEffectSupport.h"
#include "TestAudioDucker.h"

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

typedef AUDIO::CDuckerEffect TDucker;

/** The sub-block of the effects in frames */
const UInt32 SubBlock = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;

bool
PrepareDucker( TDucker& ducker, UInt32 channels = 2, UInt32 maxBlock = 4096 )
{
    return ducker.Prepare( AUDIO::CAudioFormat( 48000, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** The manual key is set to 'key' before the call that starts at 'frame' */
struct SToggle
{
    UInt32 frame;
    bool key;
};

/** Runs the ducker over the audio in calls of at most 'blockFrames' frames (0: random sizes up to 300) that never cross a toggle */
void
RunKeyed( TDucker& ducker, TSamples& audio, UInt32 channels, UInt32 blockFrames, const SToggle* toggles, UInt32 toggleCount, UInt32 seed = 7 )
{
    CXorShift random( seed );
    const size_t frames = audio.size() / channels;
    size_t done = 0;
    UInt32 next = 0;
    while ( done < frames )
    {
        while ( next < toggleCount && toggles[ next ].frame <= done )
        {
            ducker.SetManualKey( toggles[ next ].key );
            ++next;
        }
        size_t block = 0 == blockFrames ? 1 + random.Next() % 300 : blockFrames;
        if ( block > frames - done )
            block = frames - done;
        if ( next < toggleCount && done + block > toggles[ next ].frame )
            block = toggles[ next ].frame - done;
        ProcessRealtime( ducker, &audio[ done * channels ], (UInt32) block );
        done += block;
    }
}

/** Gain in dB at a frame of a run over a constant input of 0.5 */
Float64
GainAt( const TSamples& output, UInt32 channels, size_t frame )
{
    return DecibelsOf( (Float64) output[ frame * channels ] / 0.5 );
}

/** First frame at or after 'from' where the gain in dB is above 'level' (or at/below it), -1 when never */
Int32
FirstFrame( const TSamples& output, UInt32 channels, size_t from, Float64 level, bool above )
{
    const size_t frames = output.size() / channels;
    for ( size_t f=from; f<frames; ++f )
    {
        const Float64 gain = GainAt( output, channels, f );
        if ( above ? gain > level : gain <= level )
            return (Int32) f;
    }
    return -1;
}

Float64
FramesToMs( Int32 frames )
{
    return (Float64) frames / 48.0;
}

// ---- the engine side -------------------------------------------------------------------------------------------------

AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    return params;
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

void
RenderBlocks( AUDIO::CAudioEngine& engine, UInt32 blocks, UInt32 blockFrames )
{
    for ( UInt32 i=0; i<blocks; ++i )
        NullDevice( engine )->Render( blockFrames );
}

bool
RenderUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 maxBlocks = 20 )
{
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        if ( future.IsReady() )
            return true;
        NullDevice( engine )->Render( 256 );
        engine.Update();
    }
    return future.IsReady();
}

bool
IsFailedWith( const CORE::CFutureResult& future, CORE::TTaskStatus status )
{
    if ( !future.HasNoFuture() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return !task.IsNULL() && status == task->GetTaskStatus();
}

AUDIO::TAudioClipPtr
MakeStereoClip( UInt32 frames, Float32 left, Float32 right )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
    {
        audio.SetSampleFromFloat( f, 0, left );
        audio.SetSampleFromFloat( f, 1, right );
    }
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

/** A long constant clip, ten seconds */
AUDIO::TAudioClipPtr
MakeLongConstant( Float32 value )
{
    return MakeStereoClip( 480000, value, value );
}

/** Voice like content: syllables of 120 ms every 200 ms (a 140 Hz tone with two harmonics under a raised cosine), 2 s, mono */
AUDIO::TAudioClipPtr
MakeSyllables( Float32 amplitude )
{
    const UInt32 frames = 96000;
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
    {
        const Float64 t = (Float64) f / 48000.0;
        const Float64 phase = fmod( t, 0.2 );
        const Float64 envelope = phase < 0.12 ? 0.5 * ( 1.0 - cos( 2.0 * Pi * phase / 0.12 ) ) : 0.0;
        const Float64 tone = ( sin( 2.0 * Pi * 140.0 * t ) + 0.5 * sin( 2.0 * Pi * 280.0 * t ) + 0.3 * sin( 2.0 * Pi * 420.0 * t ) ) / 1.8;
        audio.SetSampleFromFloat( f, 0, amplitude * (Float32) ( envelope * tone ) );
    }
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

Float32
PeakOfBus( const AUDIO::CAudioBus* bus )
{
    const Float32 left = bus->GetMeter().GetPeak( 0 );
    const Float32 right = bus->GetMeter().GetPeak( 1 );
    return left > right ? left : right;
}

/**
 *  An engine with a bus "key" that plays a constant, and a standalone ducker keyed on it. The key bus is rendered in
 *  blocks of 32 frames (one sub-block), the ducker processes a constant 0.5 of the same length right after: the key it sees is
 *  the one of its own block, which is the best case of the one block lag of the real mixer.
 */
class CKeyRig
{
    public:

    AUDIO::CAudioEngine engine;
    AUDIO::CAudioBus* key;
    TSamples block;                 /**< the last processed block */
    Float32 lastGainDb;

    CKeyRig( void ) : key( GUCEF_NULL ), block( SubBlock * 2, 0.5f ), lastGainDb( 0.0f ) {}

    bool Start( Float32 left, Float32 right, Float32 volume )
    {
        if ( !engine.Initialize( NullParams() ) )
            return false;
        key = engine.CreateBus( "key" );
        if ( GUCEF_NULL == key )
            return false;
        key->SetVolume( volume );
        CORE::CFutureResult played = engine.Play( MakeStereoClip( 480000, left, right ), key );
        if ( played.HasNoFuture() )
            return false;
        RenderBlocks( engine, 2, SubBlock );
        engine.Update();
        return true;
    }

    /** The level the key meter shows now, dBFS of the louder channel */
    Float64 KeyDb( void ) const
    {
        return DecibelsOf( (Float64) PeakOfBus( key ) );
    }

    /** One block of the key bus, then one of the ducker; the output is in 'block' */
    void Step( TDucker& ducker )
    {
        RenderBlocks( engine, 1, SubBlock );
        block.assign( (size_t) SubBlock * 2, 0.5f );
        ProcessRealtime( ducker, &block[ 0 ], SubBlock );
        lastGainDb = ducker.GetGainInDecibels();
    }

    void Steps( TDucker& ducker, UInt32 count )
    {
        for ( UInt32 i=0; i<count; ++i )
            Step( ducker );
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformParameterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Type name, parameter descriptors, defaults, limits, key bus option, no latency or tail, registered as built-in type" )
    try
    {
        TDucker ducker;
        ASSERT_TRUE( ducker.GetEffectTypeName() == "ducker" && TDucker::GetStaticEffectTypeName() == "ducker" );
        ASSERT_TRUE( 6 == ducker.GetParameterCount() && 6 == TDucker::GetStaticParameterCount() );
        ASSERT_TRUE( -18.0f == ducker.GetParameter( TDucker::PARAMETER_DEPTH ) );
        ASSERT_TRUE( -45.0f == ducker.GetParameter( TDucker::PARAMETER_THRESHOLD ) );
        ASSERT_TRUE( 20.0f == ducker.GetParameter( TDucker::PARAMETER_ATTACK ) );
        ASSERT_TRUE( 300.0f == ducker.GetParameter( TDucker::PARAMETER_HOLD ) );
        ASSERT_TRUE( 500.0f == ducker.GetParameter( TDucker::PARAMETER_RELEASE ) );
        ASSERT_TRUE( 0.0f == ducker.GetParameter( TDucker::PARAMETER_KEY_SOURCE ) );

        UInt32 id = 0;
        ASSERT_TRUE( ducker.TryGetParameterId( "keySource", id ) && TDucker::PARAMETER_KEY_SOURCE == id );
        ASSERT_TRUE( ducker.TryGetParameterId( "HOLD", id ) && TDucker::PARAMETER_HOLD == id );
        ASSERT_FALSE( ducker.TryGetParameterId( "keyBus", id ) );                   // an option, not a parameter
        const AUDIO::CAudioEffectParameterInfo* depth = ducker.GetParameterInfo( TDucker::PARAMETER_DEPTH );
        ASSERT_TRUE( GUCEF_NULL != depth && depth->GetName() == "depth" );
        const AUDIO::CAudioEffectParameterInfo* source = ducker.GetParameterInfo( TDucker::PARAMETER_KEY_SOURCE );
        ASSERT_TRUE( GUCEF_NULL != source && source->GetName() == "keySource" );

        ducker.SetDepthInDecibels( -100.0f );
        ASSERT_TRUE( -60.0f == ducker.GetParameter( TDucker::PARAMETER_DEPTH ) );
        ducker.SetDepthInDecibels( 6.0f );
        ASSERT_TRUE( 0.0f == ducker.GetParameter( TDucker::PARAMETER_DEPTH ) );
        ducker.SetThresholdInDecibels( -200.0f );
        ASSERT_TRUE( -80.0f == ducker.GetParameter( TDucker::PARAMETER_THRESHOLD ) );
        ducker.SetThresholdInDecibels( 10.0f );
        ASSERT_TRUE( 0.0f == ducker.GetParameter( TDucker::PARAMETER_THRESHOLD ) );
        ducker.SetAttackInMs( 0.0f );
        ASSERT_TRUE( 1.0f == ducker.GetParameter( TDucker::PARAMETER_ATTACK ) );
        ducker.SetAttackInMs( 9000.0f );
        ASSERT_TRUE( 500.0f == ducker.GetParameter( TDucker::PARAMETER_ATTACK ) );
        ducker.SetHoldInMs( -4.0f );
        ASSERT_TRUE( 0.0f == ducker.GetParameter( TDucker::PARAMETER_HOLD ) );
        ducker.SetHoldInMs( 99999.0f );
        ASSERT_TRUE( 5000.0f == ducker.GetParameter( TDucker::PARAMETER_HOLD ) );
        ducker.SetReleaseInMs( 1.0f );
        ASSERT_TRUE( 10.0f == ducker.GetParameter( TDucker::PARAMETER_RELEASE ) );
        ducker.SetReleaseInMs( 99999.0f );
        ASSERT_TRUE( 5000.0f == ducker.GetParameter( TDucker::PARAMETER_RELEASE ) );
        ducker.SetKeySource( TDucker::KEYSOURCE_RMS );
        ASSERT_TRUE( 1.0f == ducker.GetParameter( TDucker::PARAMETER_KEY_SOURCE ) );

        ASSERT_TRUE( 0 == ducker.GetLatencyInFrames() && 0 == ducker.GetTailInFrames() );

        // the key bus: none, from the options, from SetKeyBus()
        ASSERT_TRUE( ducker.GetKeyBusName().IsNULLOrEmpty() );
        CORE::CValueList noOptions;
        ducker.GetConstructionOptions( noOptions );
        ASSERT_TRUE( noOptions.GetValueAlways( "keyBus" ).AsString().IsNULLOrEmpty() );
        CORE::CValueList options;
        options.Set( "keyBus", "intercom" );
        TDucker keyed( options );
        ASSERT_TRUE( keyed.GetKeyBusName() == "intercom" );
        CORE::CValueList reported;
        keyed.GetConstructionOptions( reported );
        ASSERT_TRUE( reported.GetValueAlways( "keyBus" ).AsString() == "intercom" );
        AUDIO::CAudioBus bus( "other", GUCEF_NULL );
        keyed.SetKeyBus( &bus );
        ASSERT_TRUE( keyed.GetKeyBusName() == "other" );
        keyed.SetKeyBus( GUCEF_NULL );
        ASSERT_TRUE( keyed.GetKeyBusName().IsNULLOrEmpty() );

        // the manual key
        ASSERT_FALSE( ducker.IsManualKeyActive() );
        ducker.SetManualKey( true );
        ASSERT_TRUE( ducker.IsManualKeyActive() );
        ducker.SetManualKey( false );
        ASSERT_FALSE( ducker.IsManualKeyActive() );

        // built in, made by the registry like the others
        AUDIO::TAudioEffectPtr made = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "ducker", options );
        ASSERT_FALSE( made.IsNULL() );
        ASSERT_TRUE( made->GetEffectTypeName() == "ducker" && 6 == made->GetParameterCount() );
        ASSERT_TRUE( made.StaticCast< TDucker >()->GetKeyBusName() == "intercom" );

        // unprepared it passes the audio, prepared without a key it does too
        TSamples audio = MakeNoise( 1000, 2, 3, 0.5f );
        const TSamples original( audio );
        ducker.Process( &audio[ 0 ], 1000 );
        ASSERT_TRUE( IsSame( audio, original ) );
        ASSERT_TRUE( PrepareDucker( ducker ) );
        Run( ducker, audio, 2, 256 );
        ASSERT_TRUE( IsSame( audio, original ) );
        ASSERT_TRUE( 0.0f == ducker.GetGainInDecibels() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTimingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Manual key: the depth is reached in the attack time, held for the hold time, left in the release time, to the bit when done (four settings)" )
    try
    {
        struct SSetting { Float32 depth; Float32 attack; Float32 hold; Float32 release; };
        const SSetting settings[] = { { -18.0f, 20.0f, 300.0f, 500.0f }, { -30.0f, 100.0f, 50.0f, 200.0f }, { -6.0f, 5.0f, 0.0f, 30.0f }, { -60.0f, 500.0f, 1000.0f, 1000.0f } };
        for ( UInt32 i=0; i<4; ++i )
        {
            const SSetting& s = settings[ i ];
            TDucker ducker;
            ducker.SetDepthInDecibels( s.depth );
            ducker.SetAttackInMs( s.attack );
            ducker.SetHoldInMs( s.hold );
            ducker.SetReleaseInMs( s.release );
            ASSERT_TRUE( PrepareDucker( ducker ) );

            // key on from the start, off once the attack is long done (on a sub-block)
            const UInt32 offFrame = ( (UInt32) ( ( s.attack + 150.0f ) * 48.0f / (Float32) SubBlock ) + 1 ) * SubBlock;
            const UInt32 total = offFrame + (UInt32) ( ( s.hold + s.release + 150.0f ) * 48.0f );
            const TSamples input = MakeConstant( total, 2, 0.5f );
            TSamples output( input );
            const SToggle toggles[] = { { 0, true }, { offFrame, false } };
            RunKeyed( ducker, output, 2, 256, toggles, 2 );

            // the attack: straight in dB, one step per sub-block, then at the depth
            const Float64 attackSteps = (Float64) s.attack * 48.0 / (Float64) SubBlock;
            const Float32 stepOfPosition = attackSteps > 1.0 ? (Float32) ( 1.0 / attackSteps ) : 1.0f;
            Float64 worstLine = 0.0;
            for ( UInt32 k=0; (Float32) ( k + 1 ) * stepOfPosition < 1.0f; ++k )
            {
                const Float64 ideal = (Float64) s.depth * (Float64) ( k + 1 ) * (Float64) stepOfPosition;
                const Float64 deviation = fabs( GainAt( output, 2, (size_t) k * SubBlock + SubBlock - 1 ) - ideal );
                if ( deviation > worstLine )
                    worstLine = deviation;
            }
            ASSERT_TRUE( worstLine < 0.02 );
            ASSERT_TRUE( Near( GainAt( output, 2, offFrame - 1 ), s.depth, 0.01 ) );
            const Int32 reached = FirstFrame( output, 2, 0, (Float64) s.depth + 0.05, false );
            ASSERT_TRUE( reached >= 0 );
            const Float64 attackMs = FramesToMs( reached + 1 );
            ASSERT_TRUE( Near( attackMs, s.attack, 0.1 * s.attack + 0.7 ) );

            // the hold: the gain stays at the depth after the key went
            const Int32 leaving = FirstFrame( output, 2, offFrame, (Float64) s.depth + 0.05, true );
            ASSERT_TRUE( leaving >= 0 );
            const Float64 holdMs = FramesToMs( leaving - (Int32) offFrame );
            ASSERT_TRUE( Near( holdMs, s.hold, 1.5 + 0.05 * s.release / fabs( (Float64) s.depth ) ) );

            // the release, and then the audio is untouched
            const Int32 back = FirstFrame( output, 2, leaving, -0.05, true );
            ASSERT_TRUE( back >= 0 );
            const Float64 releaseMs = FramesToMs( back - leaving );
            ASSERT_TRUE( Near( releaseMs, s.release * ( 1.0 - 0.1 / fabs( (Float64) s.depth ) ), 0.1 * s.release + 1.5 ) );
            bool untouched = true;
            for ( size_t f=(size_t) leaving + (size_t) ( ( s.release + 3.0f ) * 48.0f ); f<total; ++f )
                if ( output[ f * 2 ] != input[ f * 2 ] || output[ f * 2 + 1 ] != input[ f * 2 + 1 ] )
                    untouched = false;
            ASSERT_TRUE( untouched );
            ASSERT_TRUE( 0.0f == ducker.GetGainInDecibels() );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDucker: depth " << s.depth << " dB, attack " << s.attack << " ms measured " << attackMs << " ms, hold " << s.hold << " ms measured " << holdMs << " ms, release " << s.release << " ms measured " << releaseMs << " ms; settled gain " << GainAt( output, 2, offFrame - 1 ) << " dB, worst deviation from the dB ramp " << worstLine << " dB" );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: A key that comes back inside the hold time keeps the audio ducked; a key that leaves in the middle of the attack releases from where it got" )
    try
    {
        TDucker ducker;
        ducker.SetDepthInDecibels( -18.0f );
        ducker.SetAttackInMs( 20.0f );
        ducker.SetHoldInMs( 300.0f );
        ducker.SetReleaseInMs( 500.0f );
        ASSERT_TRUE( PrepareDucker( ducker ) );

        // pulses of 100 ms at 0 and 250 ms (the gap is shorter than the hold), the last one ends at 350 ms
        TSamples output = MakeConstant( 48000, 2, 0.5f );
        const SToggle pulses[] = { { 0, true }, { 4800, false }, { 12000, true }, { 16800, false } };
        RunKeyed( ducker, output, 2, 128, pulses, 4 );
        for ( size_t f=20 * 48; f<(size_t) ( 16800 + 300 * 48 - 96 ); ++f )
            ASSERT_TRUE( GainAt( output, 2, f ) <= -18.0 + 0.05 );
        ASSERT_TRUE( GainAt( output, 2, (size_t) ( 16800 + 400 * 48 ) ) > -18.0 + 2.0 );        // 100 ms into the release of 500 ms: 3.6 dB up
        ASSERT_TRUE( Near( GainAt( output, 2, (size_t) ( 16800 + 400 * 48 ) ), -18.0 + 3.6, 0.3 ) );

        // 10 ms of a key against an attack of 100 ms: 10 % of the depth, held 100 ms, back in 10 % of the release
        TDucker partial;
        partial.SetDepthInDecibels( -20.0f );
        partial.SetAttackInMs( 100.0f );
        partial.SetHoldInMs( 100.0f );
        partial.SetReleaseInMs( 200.0f );
        ASSERT_TRUE( PrepareDucker( partial ) );
        TSamples shortKey = MakeConstant( 24000, 2, 0.5f );
        const SToggle blip[] = { { 0, true }, { 480, false } };
        RunKeyed( partial, shortKey, 2, 256, blip, 2 );
        ASSERT_TRUE( Near( GainAt( shortKey, 2, 479 ), -2.0, 0.1 ) );
        ASSERT_TRUE( Near( GainAt( shortKey, 2, 480 + 90 * 48 ), -2.0, 0.1 ) );                 // still held
        const Int32 leaving = FirstFrame( shortKey, 2, 480, -2.0 + 0.05, true );
        const Int32 back = FirstFrame( shortKey, 2, leaving, -0.05, true );
        ASSERT_TRUE( leaving > 0 && back > leaving );
        ASSERT_TRUE( Near( FramesToMs( back - leaving ), 0.1 * 200.0 * 0.95, 2.0 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDucker: a key of 10 ms against an attack of 100 ms ducked " << GainAt( shortKey, 2, 479 ) << " dB (expected -2), came back in " << FramesToMs( back - leaving ) << " ms (expected 20)" );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Click free: no step beyond the signal's own slope plus the slope of the gain, for the shortest attack, a change of the depth and a key that comes and goes; a hard switch would step" )
    try
    {
        // 1 kHz at 0.5: its own largest step is 0.5 * 2 pi 1000 / 48000 = 0.0654
        const TSamples sine = MakeSine( 48000, 2, 1000.0, 0.5f );
        const Float32 ownStep = MaxStep( sine, 2, 0 );

        TDucker fast;
        fast.SetDepthInDecibels( -40.0f );
        fast.SetAttackInMs( 1.0f );
        fast.SetHoldInMs( 0.0f );
        fast.SetReleaseInMs( 10.0f );
        ASSERT_TRUE( PrepareDucker( fast ) );
        TSamples output( sine );
        const SToggle toggles[] = { { 4800, true }, { 14400, false }, { 24000, true }, { 24400, false }, { 33333, true }, { 33400, false } };
        RunKeyed( fast, output, 2, 100, toggles, 6 );
        Float32 worst = MaxStep( output, 2, 0 );
        ASSERT_TRUE( worst < ownStep + 0.02f );
        ASSERT_TRUE( MaxStep( output, 2, 1 ) < ownStep + 0.02f );
        ASSERT_TRUE( fast.GetGainInDecibels() == 0.0f );

        // a hard switch of the gain at 4800 would step by about 0.5
        TSamples hard( sine );
        for ( size_t f=4812; f<24012; ++f )                                        // 4812 is a quarter period in: a peak
            hard[ f * 2 ] *= 0.01f;
        ASSERT_TRUE( MaxStep( hard, 2, 0 ) > 0.4f );

        // the depth changes while ducked
        TDucker moving;
        moving.SetDepthInDecibels( -6.0f );
        moving.SetAttackInMs( 10.0f );
        ASSERT_TRUE( PrepareDucker( moving ) );
        moving.SetManualKey( true );
        TSamples swept( sine );
        for ( UInt32 block=0; block<375; ++block )
        {
            if ( 100 == block )
                moving.SetDepthInDecibels( -40.0f );
            if ( 200 == block )
                moving.SetDepthInDecibels( -3.0f );
            ProcessRealtime( moving, &swept[ (size_t) block * 128 * 2 ], 128 );
        }
        const Float32 sweptStep = MaxStep( swept, 2, 0 );
        ASSERT_TRUE( sweptStep < ownStep + 0.02f );
        ASSERT_TRUE( Near( moving.GetGainInDecibels(), -3.0, 0.01 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDucker: largest step " << worst << " with a 1 ms attack, " << sweptStep << " while the depth moves, the sine's own is " << ownStep << ", a hard switch steps " << MaxStep( hard, 2, 0 ) );

        // no subnormal numbers come out of a long fade
        TDucker fade;
        fade.SetDepthInDecibels( -60.0f );
        ASSERT_TRUE( PrepareDucker( fade ) );
        TSamples tiny = MakeConstant( 96000, 2, 1.0e-30f );
        const SToggle fadeToggles[] = { { 0, true }, { 30000, false } };
        RunKeyed( fade, tiny, 2, 256, fadeToggles, 2 );
        ASSERT_TRUE( AllFiniteAndBelow( tiny, 1.0e-29f ) );
        ASSERT_TRUE( 0 == CountSubnormal( tiny ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformKeyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Key bus meter: ducks above the threshold and not at all below it (exact pass), levels from -20 dB to +6 dB around the threshold" )
    try
    {
        CKeyRig rig;
        ASSERT_TRUE( rig.Start( 0.5f, 0.5f, 1.0f ) );
        const Float64 level = rig.KeyDb();
        ASSERT_TRUE( level < -3.0 && level > -20.0 );

        const Float64 offsets[] = { -20.0, -6.0, -1.0, -0.1, 0.1, 1.0, 6.0 };
        for ( UInt32 i=0; i<7; ++i )
        {
            // the key bus at 'level + offset' relative to a threshold at 'level'
            TDucker ducker;
            ducker.SetThresholdInDecibels( (Float32) level );
            ducker.SetDepthInDecibels( -18.0f );
            ducker.SetAttackInMs( 10.0f );
            ASSERT_TRUE( PrepareDucker( ducker ) );
            ducker.SetKeyBus( rig.key );
            rig.key->SetVolume( (Float32) LinearOf( offsets[ i ] ) );
            RenderBlocks( rig.engine, 4, SubBlock );
            ducker.Reset();

            bool exact = true;
            Float32 deepest = 0.0f;
            for ( UInt32 b=0; b<75; ++b )
            {
                rig.Step( ducker );
                if ( rig.lastGainDb < deepest )
                    deepest = rig.lastGainDb;
                for ( UInt32 f=0; f<SubBlock * 2; ++f )
                    if ( 0.5f != rig.block[ f ] )
                        exact = false;
            }
            if ( offsets[ i ] < 0.0 )
            {
                ASSERT_TRUE( exact );
                ASSERT_TRUE( 0.0f == deepest );
            }
            else
            {
                ASSERT_FALSE( exact );
                ASSERT_TRUE( Near( rig.lastGainDb, -18.0, 0.01 ) );
            }
        }
        rig.engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Key sources: peak against RMS of the key bus meter, either channel of the key bus, manual key together with a key bus" )
    try
    {
        // a burst of 50 ms is a peak above the threshold, but its RMS (300 ms) stays 8 dB under the peak
        {
            CKeyRig rig;
            ASSERT_TRUE( rig.Start( 0.5f, 0.5f, 1.0f ) );
            const Float64 level = rig.KeyDb();
            rig.key->SetVolume( 0.0f );
            RenderBlocks( rig.engine, 100, SubBlock );
            TDucker peak, rms;
            for ( UInt32 i=0; i<2; ++i )
            {
                TDucker& d = 0 == i ? peak : rms;
                d.SetThresholdInDecibels( (Float32) level - 4.0f );
                d.SetDepthInDecibels( -18.0f );
                d.SetAttackInMs( 5.0f );
                d.SetHoldInMs( 0.0f );
                d.SetKeySource( 0 == i ? TDucker::KEYSOURCE_PEAK : TDucker::KEYSOURCE_RMS );
                ASSERT_TRUE( PrepareDucker( d ) );
                d.SetKeyBus( rig.key );
            }
            rig.key->SetVolume( 1.0f );
            Float32 peakDeepest = 0.0f, rmsDeepest = 0.0f;
            for ( UInt32 b=0; b<75; ++b )
            {
                RenderBlocks( rig.engine, 1, SubBlock );
                TSamples a( (size_t) SubBlock * 2, 0.5f ), c( a );
                ProcessRealtime( peak, &a[ 0 ], SubBlock );
                ProcessRealtime( rms, &c[ 0 ], SubBlock );
                if ( peak.GetGainInDecibels() < peakDeepest )
                    peakDeepest = peak.GetGainInDecibels();
                if ( rms.GetGainInDecibels() < rmsDeepest )
                    rmsDeepest = rms.GetGainInDecibels();
            }
            ASSERT_TRUE( Near( peakDeepest, -18.0, 0.01 ) );
            ASSERT_TRUE( 0.0f == rmsDeepest );
            rig.engine.Shutdown();
        }

        // a sustained key: the RMS passes the threshold 4 dB under the level after 0.3 s * -ln( 1 - 0.398 ) = 152 ms
        {
            CKeyRig rig;
            ASSERT_TRUE( rig.Start( 0.5f, 0.5f, 1.0f ) );
            const Float64 level = rig.KeyDb();
            TDucker rms;
            rms.SetThresholdInDecibels( (Float32) level - 4.0f );
            rms.SetDepthInDecibels( -18.0f );
            rms.SetAttackInMs( 5.0f );
            rms.SetKeySource( TDucker::KEYSOURCE_RMS );
            ASSERT_TRUE( PrepareDucker( rms ) );
            rms.SetKeyBus( rig.key );
            // the engine has played two blocks only: the meter's mean square started at 0
            Int32 startedAt = -1;
            for ( UInt32 b=0; b<600 && startedAt < 0; ++b )
            {
                rig.Step( rms );
                if ( rig.lastGainDb < -0.5f )
                    startedAt = (Int32) b;
            }
            ASSERT_TRUE( startedAt > 0 );
            const Float64 startedMs = (Float64) ( startedAt + 2 ) * 32.0 / 48.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDucker: RMS key: ducking began after " << startedMs << " ms, the 300 ms meter predicts 152 ms" );
            ASSERT_TRUE( startedMs > 130.0 && startedMs < 190.0 );
            rig.engine.Shutdown();
        }

        // either channel of the key bus is enough: left only, right only
        for ( UInt32 side=0; side<2; ++side )
        {
            CKeyRig rig;
            ASSERT_TRUE( rig.Start( 0 == side ? 0.5f : 0.0f, 0 == side ? 0.0f : 0.5f, 1.0f ) );
            const Float64 level = rig.KeyDb();
            ASSERT_TRUE( level > -30.0 );
            TDucker ducker;
            ducker.SetThresholdInDecibels( (Float32) level - 1.0f );
            ducker.SetAttackInMs( 5.0f );
            ASSERT_TRUE( PrepareDucker( ducker ) );
            ducker.SetKeyBus( rig.key );
            rig.Steps( ducker, 40 );
            ASSERT_TRUE( Near( rig.lastGainDb, -18.0, 0.01 ) );
            rig.engine.Shutdown();
        }

        // manual key and key bus: either one ducks, both keep it down, neither lets it go
        {
            CKeyRig rig;
            ASSERT_TRUE( rig.Start( 0.5f, 0.5f, 0.0f ) );                       // the key bus is silent
            TDucker ducker;
            ducker.SetAttackInMs( 5.0f );
            ducker.SetHoldInMs( 0.0f );
            ducker.SetReleaseInMs( 10.0f );
            ASSERT_TRUE( PrepareDucker( ducker ) );
            ducker.SetKeyBus( rig.key );
            rig.Steps( ducker, 40 );
            ASSERT_TRUE( 0.0f == rig.lastGainDb );
            ducker.SetManualKey( true );                                         // manual only
            rig.Steps( ducker, 40 );
            ASSERT_TRUE( Near( rig.lastGainDb, -18.0, 0.01 ) );
            rig.key->SetVolume( 1.0f );                                          // both
            rig.Steps( ducker, 40 );
            ASSERT_TRUE( Near( rig.lastGainDb, -18.0, 0.01 ) );
            ducker.SetManualKey( false );                                        // key bus only
            rig.Steps( ducker, 40 );
            ASSERT_TRUE( Near( rig.lastGainDb, -18.0, 0.01 ) );
            rig.key->SetVolume( 0.0f );                                          // neither
            rig.Steps( ducker, 80 );
            ASSERT_TRUE( 0.0f == rig.lastGainDb );
            rig.engine.Shutdown();
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
PerformEngineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDucker: A key bus that does not exist refuses the insert and the bus layout; no key bus inserts, the key bus is found by name" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* key = engine.CreateBus( "key" );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        ASSERT_TRUE( GUCEF_NULL != key && GUCEF_NULL != music );

        CORE::CValueList onKey, onNothing;
        onKey.Set( "keyBus", "key" );
        onNothing.Set( "keyBus", "nosuchbus" );

        // ResolveReferences() itself
        TDucker lost( onNothing );
        ASSERT_FALSE( lost.ResolveReferences( engine ) );
        TDucker found( onKey );
        ASSERT_TRUE( found.ResolveReferences( engine ) );
        TDucker none;
        ASSERT_TRUE( none.ResolveReferences( engine ) );

        // inserts
        AUDIO::TAudioEffectPtr refused = engine.CreateEffect( "ducker", onNothing );
        ASSERT_FALSE( refused.IsNULL() );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( music, 0, refused ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( engine.GetBusEffect( music, 0 ).IsNULL() );
        RenderBlocks( engine, 2, 256 );

        AUDIO::TAudioEffectPtr keyed = engine.CreateEffect( "ducker", onKey );
        CORE::CFutureResult inserted = engine.InsertEffect( music, 0, keyed );
        ASSERT_FALSE( inserted.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, inserted ) );
        ASSERT_TRUE( keyed.GetPointerAlways() == engine.GetBusEffect( music, 0 ).GetPointerAlways() );

        AUDIO::TAudioEffectPtr manual = engine.CreateEffect( "ducker" );
        CORE::CFutureResult manualInserted = engine.InsertEffect( music, 1, manual );
        ASSERT_FALSE( manualInserted.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, manualInserted ) );

        // the effect was resolved: the key bus drives it through the real mixer
        engine.Play( MakeLongConstant( 0.4f ), music );
        RenderBlocks( engine, 10, 256 );
        const Float32 before = PeakOfBus( music );
        ASSERT_TRUE( before > 0.1f );
        engine.Play( MakeLongConstant( 0.5f ), key );
        RenderBlocks( engine, 40, 256 );
        const Float64 ducked = DecibelsOf( (Float64) PeakOfBus( music ) / (Float64) before );
        ASSERT_TRUE( Near( ducked, -18.0, 0.1 ) );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Bus layout round trip keeps keyBus and the parameters; a layout that makes the key bus resolves it; a layout with a key bus nowhere fails" )
    try
    {
        // key bus first, then the bus with the ducker
        AUDIO::CAudioBusLayout layout;
        layout.AddBus( "key" );
        AUDIO::CAudioBusLayout::SBus& ducked = layout.AddBus( "ducked" );
        CORE::CValueList onKey;
        onKey.Set( "keyBus", "key" );
        AUDIO::TAudioEffectPtr described = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "ducker", onKey );
        ASSERT_FALSE( described.IsNULL() );
        described->SetParameter( TDucker::PARAMETER_DEPTH, -24.0f );
        described->SetParameter( TDucker::PARAMETER_HOLD, 100.0f );
        described->SetParameter( TDucker::PARAMETER_KEY_SOURCE, 1.0f );
        ASSERT_TRUE( described->SaveConfig( ducked.AddEffect( 0, "ducker" ).config ) );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
        ASSERT_FALSE( applied.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, applied ) );
        AUDIO::CAudioBus* key = engine.FindBus( "key" );
        AUDIO::CAudioBus* duckedBus = engine.FindBus( "ducked" );
        ASSERT_TRUE( GUCEF_NULL != key && GUCEF_NULL != duckedBus );
        AUDIO::TAudioEffectPtr placed = engine.GetBusEffect( duckedBus, 0 );
        ASSERT_FALSE( placed.IsNULL() );
        ASSERT_TRUE( placed.StaticCast< TDucker >()->GetKeyBusName() == "key" );
        ASSERT_TRUE( -24.0f == placed->GetParameter( TDucker::PARAMETER_DEPTH ) );
        ASSERT_TRUE( 100.0f == placed->GetParameter( TDucker::PARAMETER_HOLD ) );
        ASSERT_TRUE( 1.0f == placed->GetParameter( TDucker::PARAMETER_KEY_SOURCE ) );

        // it ducks by the depth of the layout once the key is playing
        engine.Play( MakeLongConstant( 0.4f ), duckedBus );
        RenderBlocks( engine, 20, 256 );
        const Float32 before = PeakOfBus( duckedBus );
        engine.Play( MakeLongConstant( 0.5f ), key );
        RenderBlocks( engine, 400, 256 );                                       // the RMS key needs about 150 ms
        ASSERT_TRUE( Near( DecibelsOf( (Float64) PeakOfBus( duckedBus ) / (Float64) before ), -24.0, 0.1 ) );

        // what the engine reports applies to a fresh engine and gives the same
        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( engine.GetBusLayout( captured ) );
        engine.Shutdown();

        AUDIO::CAudioEngine fresh;
        ASSERT_TRUE( fresh.Initialize( NullParams() ) );
        CORE::CFutureResult appliedAgain = fresh.ApplyBusLayout( captured );
        ASSERT_FALSE( appliedAgain.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( fresh, appliedAgain ) );
        AUDIO::TAudioEffectPtr rebuilt = fresh.GetBusEffect( fresh.FindBus( "ducked" ), 0 );
        ASSERT_FALSE( rebuilt.IsNULL() );
        ASSERT_TRUE( rebuilt.StaticCast< TDucker >()->GetKeyBusName() == "key" );
        ASSERT_TRUE( -24.0f == rebuilt->GetParameter( TDucker::PARAMETER_DEPTH ) );
        ASSERT_TRUE( 100.0f == rebuilt->GetParameter( TDucker::PARAMETER_HOLD ) );
        ASSERT_TRUE( 1.0f == rebuilt->GetParameter( TDucker::PARAMETER_KEY_SOURCE ) );
        fresh.Play( MakeLongConstant( 0.4f ), fresh.FindBus( "ducked" ) );
        RenderBlocks( fresh, 20, 256 );
        const Float32 freshBefore = PeakOfBus( fresh.FindBus( "ducked" ) );
        fresh.Play( MakeLongConstant( 0.5f ), fresh.FindBus( "key" ) );
        RenderBlocks( fresh, 400, 256 );
        ASSERT_TRUE( Near( DecibelsOf( (Float64) PeakOfBus( fresh.FindBus( "ducked" ) ) / (Float64) freshBefore ), -24.0, 0.1 ) );
        fresh.Shutdown();

        // a key bus that no layout and no engine has: the insert fails, so does the layout, the steps before it stay
        AUDIO::CAudioEngine third;
        ASSERT_TRUE( third.Initialize( NullParams() ) );
        AUDIO::CAudioBusLayout broken;
        AUDIO::CAudioBusLayout::SBus& brokenBus = broken.AddBus( "broken" );
        CORE::CValueList onNothing;
        onNothing.Set( "keyBus", "nosuchbus" );
        AUDIO::TAudioEffectPtr brokenEffect = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "ducker", onNothing );
        ASSERT_TRUE( brokenEffect->SaveConfig( brokenBus.AddEffect( 0, "ducker" ).config ) );
        ASSERT_TRUE( IsFailedWith( third.ApplyBusLayout( broken ), CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED ) );
        ASSERT_TRUE( third.GetBusEffect( third.FindBus( "broken" ), 0 ).IsNULL() );
        third.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Announcement bus with voice like content on the real engine: music and media buses are ducked by the depth, together, and come back to the bit" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* intercom = engine.CreateBus( "intercom" );
        AUDIO::CAudioBus* announce = engine.CreateBus( "announce", intercom );
        AUDIO::CAudioBus* music = engine.CreateBus( "music" );
        AUDIO::CAudioBus* media = engine.CreateBus( "media" );
        ASSERT_TRUE( GUCEF_NULL != announce && GUCEF_NULL != music && GUCEF_NULL != media );

        CORE::CValueList onIntercom;
        onIntercom.Set( "keyBus", "intercom" );
        AUDIO::CAudioBus* ducked[ 2 ] = { music, media };
        TDucker* duckers[ 2 ] = { GUCEF_NULL, GUCEF_NULL };
        for ( UInt32 i=0; i<2; ++i )
        {
            AUDIO::TAudioEffectPtr effect = engine.CreateEffect( "ducker", onIntercom );
            ASSERT_FALSE( effect.IsNULL() );
            effect->SetParameter( TDucker::PARAMETER_DEPTH, -20.0f );
            effect->SetParameter( TDucker::PARAMETER_THRESHOLD, -45.0f );
            effect->SetParameter( TDucker::PARAMETER_ATTACK, 20.0f );
            effect->SetParameter( TDucker::PARAMETER_HOLD, 300.0f );
            effect->SetParameter( TDucker::PARAMETER_RELEASE, 500.0f );
            duckers[ i ] = effect.StaticCast< TDucker >().GetPointerAlways();
            CORE::CFutureResult inserted = engine.InsertEffect( ducked[ i ], 0, effect );
            ASSERT_FALSE( inserted.HasNoFuture() );
            ASSERT_TRUE( RenderUntilReady( engine, inserted ) );
        }
        engine.Play( MakeLongConstant( 0.4f ), music );
        engine.Play( MakeLongConstant( 0.2f ), media );

        // half a second of music alone
        Float32 baseline[ 2 ] = { 0.0f, 0.0f };
        for ( UInt32 b=0; b<100; ++b )
        {
            RenderBlocks( engine, 1, 256 );
            baseline[ 0 ] = PeakOfBus( music );
            baseline[ 1 ] = PeakOfBus( media );
        }
        ASSERT_TRUE( baseline[ 0 ] > 0.1f && baseline[ 1 ] > 0.05f );
        ASSERT_TRUE( 0.0f == duckers[ 0 ]->GetGainInDecibels() && 0.0f == duckers[ 1 ]->GetGainInDecibels() );

        // the announcement: 2 s of syllables with gaps of 80 ms, shorter than the hold time
        engine.Play( MakeSyllables( 0.3f ), announce );
        Int32 fullDepthAt = -1;
        Float64 worstDeviation = 0.0;
        Float64 lowest = 0.0;
        for ( UInt32 b=0; b<412; ++b )
        {
            RenderBlocks( engine, 1, 256 );
            const Float64 musicDb = DecibelsOf( (Float64) PeakOfBus( music ) / (Float64) baseline[ 0 ] );
            const Float64 mediaDb = DecibelsOf( (Float64) PeakOfBus( media ) / (Float64) baseline[ 1 ] );
            if ( fullDepthAt < 0 && musicDb < -19.9 )
                fullDepthAt = (Int32) b;
            if ( musicDb < lowest )
                lowest = musicDb;
            if ( b >= 15 )                                                       // from 80 ms to the end of the announcement and the hold
            {
                const Float64 deviation = fabs( musicDb + 20.0 );
                if ( deviation > worstDeviation )
                    worstDeviation = deviation;
                ASSERT_TRUE( Near( mediaDb, musicDb, 0.05 ) );
            }
        }
        ASSERT_TRUE( fullDepthAt >= 0 );
        const Float64 reachedMs = (Float64) ( fullDepthAt + 1 ) * 256.0 / 48.0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioDucker: announcement ducks the music by " << lowest << " dB (depth -20), full depth after " << reachedMs << " ms (attack 20 ms, blocks of 5.3 ms), worst deviation from the depth while it plays " << worstDeviation << " dB" );
        ASSERT_TRUE( worstDeviation < 0.1 );
        ASSERT_TRUE( reachedMs > 15.0 && reachedMs < 45.0 );
        ASSERT_TRUE( Near( lowest, -20.0, 0.1 ) );
        ASSERT_TRUE( Near( duckers[ 0 ]->GetGainInDecibels(), -20.0, 0.01 ) );

        // hold, release and the end: the music is back at the bit
        RenderBlocks( engine, 380, 256 );
        ASSERT_TRUE( 0.0f == duckers[ 0 ]->GetGainInDecibels() && 0.0f == duckers[ 1 ]->GetGainInDecibels() );
        ASSERT_TRUE( baseline[ 0 ] == PeakOfBus( music ) && baseline[ 1 ] == PeakOfBus( media ) );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformInvarianceAndRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Output does not depend on the block size (fixed sizes and random sizes, bit for bit), with the manual key switched between calls" )
    try
    {
        const SToggle toggles[] = { { 4800, true }, { 19200, false }, { 24001, true }, { 26400, false }, { 70001, true }, { 71000, false } };
        const TSamples noise = MakeNoise( 120000, 2, 99, 0.5f );

        TDucker referenceDucker;
        referenceDucker.SetAttackInMs( 7.0f );
        referenceDucker.SetHoldInMs( 123.0f );
        referenceDucker.SetReleaseInMs( 90.0f );
        ASSERT_TRUE( PrepareDucker( referenceDucker ) );
        TSamples reference( noise );
        RunKeyed( referenceDucker, reference, 2, 4096, toggles, 6 );
        ASSERT_FALSE( IsSame( reference, noise ) );

        const UInt32 sizes[] = { 1, 7, 32, 33, 160, 480, 1000, 0 };
        for ( UInt32 i=0; i<8; ++i )
        {
            TDucker ducker;
            ducker.SetAttackInMs( 7.0f );
            ducker.SetHoldInMs( 123.0f );
            ducker.SetReleaseInMs( 90.0f );
            ASSERT_TRUE( PrepareDucker( ducker ) );
            TSamples output( noise );
            RunKeyed( ducker, output, 2, sizes[ i ], toggles, 6, 1234 + i );
            ASSERT_TRUE( IsSame( output, reference ) );
            ASSERT_TRUE( ducker.GetGainInDecibels() == referenceDucker.GetGainInDecibels() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Settings saved and loaded: parameters round trip, the key bus is a construction option and is saved with the options only" )
    try
    {
        CORE::CValueList onKey;
        onKey.Set( "keyBus", "key" );
        TDucker original( onKey );
        original.SetDepthInDecibels( -33.0f );
        original.SetThresholdInDecibels( -52.0f );
        original.SetAttackInMs( 12.0f );
        original.SetHoldInMs( 750.0f );
        original.SetReleaseInMs( 1250.0f );
        original.SetKeySource( TDucker::KEYSOURCE_RMS );
        CORE::CDataNode config;
        ASSERT_TRUE( original.SaveConfig( config ) );

        const CORE::CDataNode* options = config.FindChild( "Options" );
        ASSERT_TRUE( GUCEF_NULL != options );
        ASSERT_TRUE( options->GetAttributeValue( "keyBus" ).AsString() == "key" );
        const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
        ASSERT_TRUE( GUCEF_NULL != parameters );
        ASSERT_TRUE( parameters->GetAttributeValue( "keySource" ).AsString() == "rms" );

        TDucker loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        for ( UInt32 p=0; p<TDucker::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( loaded.GetParameter( p ) == original.GetParameter( p ) );
        ASSERT_TRUE( loaded.GetKeyBusName().IsNULLOrEmpty() );                           // options are not read by LoadConfig()

        // made from the options and loaded: the same as the original
        CORE::CValueList restoredOptions;
        original.GetConstructionOptions( restoredOptions );
        TDucker made( restoredOptions );
        ASSERT_TRUE( made.LoadConfig( config ) );
        ASSERT_TRUE( made.GetKeyBusName() == "key" );
        for ( UInt32 p=0; p<TDucker::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( made.GetParameter( p ) == original.GetParameter( p ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioDucker: Real time safety: processing with a key bus, the manual key and parameter changes from another context allocates, locks and blocks nowhere (RealtimeSanitizer builds)" )
    try
    {
        CKeyRig rig;
        ASSERT_TRUE( rig.Start( 0.5f, 0.5f, 1.0f ) );
        TDucker ducker;
        ducker.SetAttackInMs( 3.0f );
        ducker.SetHoldInMs( 20.0f );
        ducker.SetReleaseInMs( 30.0f );
        ASSERT_TRUE( PrepareDucker( ducker ) );
        ducker.SetKeyBus( rig.key );

        TSamples audio = MakeNoise( 4096, 2, 17, 0.5f );
        Float32 deepest = 0.0f;
        for ( UInt32 round=0; round<200; ++round )
        {
            rig.key->SetVolume( 0 == ( round / 10 ) % 2 ? 1.0f : 0.0f );
            ducker.SetManualKey( 3 == round % 7 );
            ducker.SetParameter( TDucker::PARAMETER_DEPTH, -6.0f - (Float32) ( round % 40 ) );
            ducker.SetParameter( TDucker::PARAMETER_KEY_SOURCE, (Float32) ( round % 2 ) );
            if ( 99 == round )
                ResetRealtime( ducker );
            RenderBlocks( rig.engine, 1, SubBlock );
            ProcessRealtime( ducker, &audio[ 0 ], SubBlock * ( 1 + round % 7 ) );
            if ( ducker.GetGainInDecibels() < deepest )
                deepest = ducker.GetGainInDecibels();
        }
        ASSERT_TRUE( deepest < -5.0f );
        ASSERT_TRUE( AllFiniteAndBelow( audio, 1.0f ) );
        rig.engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioDuckerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioDucker TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Ducker" );

    PerformParameterTests();
    PerformTimingTests();
    PerformKeyTests();
    PerformEngineTests();
    PerformInvarianceAndRealtimeTests();
}

/*-------------------------------------------------------------------------*/
