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

#ifndef GUCEF_AUDIO_CAUDIOBUSROUTING_H
#include "gucefAUDIO_CAudioBusRouting.h"
#define GUCEF_AUDIO_CAUDIOBUSROUTING_H
#endif /* GUCEF_AUDIO_CAUDIOBUSROUTING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioReferenceScenarios.h"
#include "TestAudioRouting.h"

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

/** Hashes of the reference scenarios, recorded before effect chains, sends, meters and bus order tables existed */
using TestAudioReferenceScenarios::ReferenceMixerLimiterStereo;
using TestAudioReferenceScenarios::ReferenceMixerClippingStereo;
using TestAudioReferenceScenarios::ReferenceMixerLimiterSurround;
using TestAudioReferenceScenarios::ReferenceEngine;

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

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

    private:

    UInt32 m_state;
};

AUDIO::CAudioEngineParams
NullParams( bool threaded = false )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( threaded ? AUDIO::AUDIOTHREADING_THREADED : AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

/** A mono clip: one sample of 'value' at the start, silence after it */
AUDIO::TAudioClipPtr
MakeImpulse( UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, 0 == f ? value : 0.0f );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, value );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeSine( UInt32 frames, Float64 frequency, Float32 amplitude )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, amplitude * (Float32) sin( 2.0 * 3.14159265358979323846 * frequency * (Float64) f / 48000.0 ) );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

/** Cooperative mode: renders and updates until the future is ready (the audio thread has to run for commands) */
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

/** Sample of the rendered stereo output */
Float32
Rendered( AUDIO::CAudioEngine& engine, UInt32 frame, UInt32 channel = 0 )
{
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    if ( frame >= rendered.GetFrameCount() )
        return -999.0f;
    return static_cast< const Float32* >( rendered.GetDataPtr() )[ (size_t) frame * 2 + channel ];
}

struct SEdgeIndex
{
    UInt32 first;
    UInt32 second;
    SEdgeIndex( UInt32 from, UInt32 to ) : first( from ), second( to ) {}
};

/** True when every bus comes after all buses that feed it, and every bus is there once */
bool
IsValidOrder( AUDIO::CAudioBus* const* order, UInt32 count, const GUCEF::vector< AUDIO::CAudioBus* >& buses,
              const GUCEF::vector< Int32 >& parents, const GUCEF::vector< SEdgeIndex >& sends )
{
    if ( count != buses.size() )
        return false;
    GUCEF::vector< Int32 > position( buses.size(), -1 );
    for ( UInt32 i=0; i<count; ++i )
    {
        for ( size_t b=0; b<buses.size(); ++b )
            if ( buses[ b ] == order[ i ] )
            {
                if ( position[ b ] >= 0 )
                    return false;
                position[ b ] = (Int32) i;
            }
    }
    for ( size_t b=0; b<buses.size(); ++b )
    {
        if ( position[ b ] < 0 )
            return false;
        if ( parents[ b ] >= 0 && position[ (size_t) parents[ b ] ] < position[ b ] )
            return false;
    }
    for ( size_t s=0; s<sends.size(); ++s )
        if ( position[ sends[ s ].second ] < position[ sends[ s ].first ] )
            return false;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformRoutingGraphTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRouting: A plain tree is processed in reverse creation order, sends reorder it, cycles are refused" )
    try
    {
        AUDIO::CAudioBus master( "master", GUCEF_NULL );
        AUDIO::CAudioBus music( "music", &master );
        AUDIO::CAudioBus sfx( "sfx", &master );
        AUDIO::CAudioBus ui( "ui", &sfx );
        AUDIO::CAudioBus reverb( "reverb", &master );

        AUDIO::CAudioBusRouting routing( 8 );
        ASSERT_FALSE( routing.AddBus( &music, &master ) );                                 // the master has to come first
        ASSERT_TRUE( routing.AddBus( &master, GUCEF_NULL ) );
        ASSERT_FALSE( routing.AddBus( &music, GUCEF_NULL ) );                              // only one root
        ASSERT_TRUE( routing.AddBus( &music, &master ) && routing.AddBus( &sfx, &master ) && routing.AddBus( &ui, &sfx ) );
        ASSERT_FALSE( routing.AddBus( &ui, &sfx ) );                                       // known already
        ASSERT_TRUE( 4 == routing.GetBusCount() && routing.HasBus( &ui ) && !routing.HasBus( &reverb ) );

        AUDIO::CAudioBus* order[ 8 ];
        UInt32 count = 0;
        ASSERT_TRUE( routing.ComputeOrder( order, 8, count ) && 4 == count );
        ASSERT_TRUE( order[ 0 ] == &ui && order[ 1 ] == &sfx && order[ 2 ] == &music && order[ 3 ] == &master );
        ASSERT_FALSE( routing.ComputeOrder( order, 3, count ) );                           // no room

        // a reverb bus made after the buses that send to it must still come after them
        ASSERT_TRUE( routing.AddBus( &reverb, &master ) );
        ASSERT_TRUE( routing.Reaches( &ui, &master ) && !routing.Reaches( &master, &ui ) );
        ASSERT_TRUE( routing.AddSend( &ui, &reverb ) && routing.AddSend( &music, &reverb ) );
        ASSERT_TRUE( routing.ComputeOrder( order, 8, count ) && 5 == count );
        ASSERT_TRUE( order[ 0 ] == &ui && order[ 1 ] == &sfx && order[ 2 ] == &music && order[ 3 ] == &reverb && order[ 4 ] == &master );

        // cycles: to itself, back into a bus that feeds it, from the master. A send to an ancestor is no cycle
        ASSERT_FALSE( routing.CanAddSend( &sfx, &sfx ) );
        ASSERT_FALSE( routing.AddSend( &sfx, &ui ) );                                       // ui feeds sfx
        ASSERT_FALSE( routing.AddSend( &reverb, &ui ) );                                    // ui sends to reverb
        ASSERT_FALSE( routing.AddSend( &master, &music ) );
        ASSERT_TRUE( routing.CanAddSend( &ui, &master ) );

        // removing a send lifts its constraint, a bus that is used cannot be removed
        ASSERT_TRUE( routing.RemoveSend( &ui, &reverb ) );
        ASSERT_FALSE( routing.RemoveSend( &ui, &reverb ) );
        ASSERT_FALSE( routing.RemoveBus( &music ) );                                        // it sends
        ASSERT_FALSE( routing.RemoveBus( &master ) );
        ASSERT_TRUE( routing.RemoveSend( &music, &reverb ) && routing.RemoveBus( &reverb ) && !routing.HasBus( &reverb ) );
        ASSERT_TRUE( routing.ComputeOrder( order, 8, count ) && 4 == count && order[ 0 ] == &ui );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Random bus graphs (fixed seed): every bus is processed after all buses that feed it" )
    try
    {
        CXorShift random( 4242 );
        UInt32 refused = 0, accepted = 0;
        for ( UInt32 run=0; run<200; ++run )
        {
            const UInt32 busCount = 2 + random.Next() % 15;
            GUCEF::vector< AUDIO::CAudioBus* > buses;
            GUCEF::vector< Int32 > parents;
            GUCEF::vector< SEdgeIndex > sends;
            AUDIO::CAudioBusRouting routing( busCount );
            for ( UInt32 b=0; b<busCount; ++b )
            {
                const Int32 parent = 0 == b ? -1 : (Int32) ( random.Next() % b );
                buses.push_back( GUCEF_NEW AUDIO::CAudioBus( "bus", parent >= 0 ? buses[ (size_t) parent ] : GUCEF_NULL ) );
                parents.push_back( parent );
                ASSERT_TRUE( routing.AddBus( buses.back(), parent >= 0 ? buses[ (size_t) parent ] : GUCEF_NULL ) );
            }
            const UInt32 attempts = random.Next() % ( 2 * busCount );
            for ( UInt32 s=0; s<attempts; ++s )
            {
                const UInt32 from = random.Next() % busCount;
                const UInt32 to = random.Next() % busCount;
                if ( routing.AddSend( buses[ from ], buses[ to ] ) )
                {
                    sends.push_back( SEdgeIndex( from, to ) );
                    ++accepted;
                }
                else
                    ++refused;
            }
            GUCEF::vector< AUDIO::CAudioBus* > order( busCount, GUCEF_NULL );
            UInt32 count = 0;
            ASSERT_TRUE( routing.ComputeOrder( &order[ 0 ], busCount, count ) );
            ASSERT_TRUE( IsValidOrder( &order[ 0 ], count, buses, parents, sends ) );
            ASSERT_TRUE( order[ count - 1 ] == buses[ 0 ] );                                // the master last
            for ( size_t b=0; b<buses.size(); ++b )
                GUCEF_DELETE buses[ b ];
        }
        ASSERT_TRUE( accepted > 100 && refused > 100 );                                     // both cases were exercised
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBitIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Bus trees without effects and sends sound exactly as before (reference output, instant limiter on and off)" )
    try
    {
        ASSERT_TRUE( ReferenceMixerLimiterStereo == TestAudioReferenceScenarios::RunMixerTreeScenario( true, 2 ) );
        ASSERT_TRUE( ReferenceMixerClippingStereo == TestAudioReferenceScenarios::RunMixerTreeScenario( false, 2 ) );
        ASSERT_TRUE( ReferenceMixerLimiterSurround == TestAudioReferenceScenarios::RunMixerTreeScenario( true, 6 ) );
        ASSERT_TRUE( ReferenceEngine == TestAudioReferenceScenarios::RunEngineScenario() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Disabled effects on buses, a bus order table and unused sends leave the mix bit for bit as it was" )
    try
    {
        GUCEF::vector< Float32 > outputs[ 2 ];
        for ( UInt32 variant=0; variant<2; ++variant )
        {
            AUDIO::CAudioMixer mixer( StereoFormat, 128, 8, 6 );
            mixer.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
            AUDIO::CAudioBus* music = mixer.CreateBus( "music" );
            AUDIO::CAudioBus* effects = mixer.CreateBus( "effects" );
            AUDIO::CAudioBus* ui = mixer.CreateBus( "ui", effects );
            effects->SetVolume( 0.7f );

            CORE::CValueList invert;
            invert.Set( "invert", "true" );
            CTestAudioEffect a( invert ), b( invert ), c( invert );
            AUDIO::CAudioBusRouting routing( 6 );
            AUDIO::CAudioBus* order[ 6 ];
            UInt32 count = 0;
            if ( 1 == variant )
            {
                // effects that are switched off, and an order computed by the routing instead of the built in one
                ASSERT_TRUE( a.Prepare( StereoFormat, 128 ) && b.Prepare( StereoFormat, 128 ) && c.Prepare( StereoFormat, 128 ) );
                mixer.GetMasterBus()->SetEffectEnabled( 0, false );
                effects->SetEffectEnabled( 2, false );
                ui->SetEffectEnabled( 7, false );
                ASSERT_TRUE( mixer.SetBusEffect( mixer.GetMasterBus(), 0, &a ) && mixer.SetBusEffect( effects, 2, &b ) && mixer.SetBusEffect( ui, 7, &c ) );
                ASSERT_FALSE( mixer.SetBusEffect( ui, 8, &c ) );                            // 8 slots per bus by default
                routing.AddBus( mixer.GetMasterBus(), GUCEF_NULL );
                routing.AddBus( music, mixer.GetMasterBus() );
                routing.AddBus( effects, mixer.GetMasterBus() );
                routing.AddBus( ui, effects );
                ASSERT_TRUE( routing.ComputeOrder( order, 6, count ) );
                mixer.SetBusOrder( order, count );
            }

            AUDIO::TAudioVoicePtr v1 = TestAudioReferenceScenarios::MakeMixerVoice( mixer, TestAudioReferenceScenarios::MakeRampClip( 22050, 1, 3000, -0.9f, 0.9f ), effects );
            AUDIO::TAudioVoicePtr v2 = TestAudioReferenceScenarios::MakeMixerVoice( mixer, TestAudioReferenceScenarios::MakeRampClip( 48000, 2, 5000, 0.9f, 0.1f ), music );
            AUDIO::TAudioVoicePtr v3 = TestAudioReferenceScenarios::MakeMixerVoice( mixer, TestAudioReferenceScenarios::MakeRampClip( 48000, 1, 5000, 0.5f, 0.6f ), ui );
            mixer.AddVoice( v1.GetPointerAlways() );
            mixer.AddVoice( v2.GetPointerAlways() );
            mixer.AddVoice( v3.GetPointerAlways() );
            outputs[ variant ].assign( 4000 * 2, 0.0f );
            for ( UInt32 p=0; p<40; ++p )
            {
                if ( 20 == p ) music->SetVolume( 0.4f );
                mixer.Mix( &outputs[ variant ][ (size_t) p * 100 * 2 ], 100 );
            }
            mixer.SetBusOrder( GUCEF_NULL, 0 );
        }
        ASSERT_TRUE( 0 == memcmp( &outputs[ 0 ][ 0 ], &outputs[ 1 ][ 0 ], outputs[ 0 ].size() * sizeof( Float32 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSendTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRouting: An impulse sent to another bus arrives there once at the send level, before or after the fader" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* source = engine.CreateBus( "source" );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );           // made after the source, still processed after it
        ASSERT_TRUE( GUCEF_NULL != source && GUCEF_NULL != aux );
        ASSERT_TRUE( 4 == source->GetSendSlotCount() && 8 == source->GetEffectSlotCount() );
        source->SetVolume( 0.5f );

        UInt32 sendIndex = 99;
        CORE::CFutureResult added = engine.AddSend( source, aux, 0.25f, false, &sendIndex );
        ASSERT_FALSE( added.HasNoFuture() );
        ASSERT_TRUE( 0 == sendIndex && 0.25f == source->GetSendLevel( 0 ) );
        ASSERT_TRUE( RenderUntilReady( engine, added ) && added.IsReady() );

        // post fader: 0.5 directly, 0.5 x 0.25 through the aux bus
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeImpulse( 1000, 1.0f ), source );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.625, Rendered( engine, 0 ), 1e-6 ) && Near( 0.625, Rendered( engine, 0, 1 ), 1e-6 ) );
        bool silentAfter = true;
        for ( UInt32 f=1; f<256; ++f )
            silentAfter = silentAfter && 0.0f == Rendered( engine, f );
        ASSERT_TRUE( silentAfter );                                                          // once, not twice
        ASSERT_TRUE( Near( 0.125, aux->GetMeter().GetPeak( 0 ), 1e-6 ) );

        // the aux bus muted: only the direct path
        aux->SetMuted( true );
        NullDevice( engine )->Render( 512 );                                                // the mute ramps in
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeImpulse( 1000, 1.0f ), source );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.5, Rendered( engine, 0 ), 1e-6 ) );
        aux->SetMuted( false );

        // a second send, before the fader: the source volume does not apply to it
        UInt32 preIndex = 99;
        CORE::CFutureResult pre = engine.AddSend( source, aux, 0.5f, true, &preIndex );
        ASSERT_TRUE( RenderUntilReady( engine, pre ) && 1 == preIndex );
        NullDevice( engine )->Render( 512 );
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeImpulse( 1000, 0.5f ), source );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.25 + 0.0625 + 0.25, Rendered( engine, 0 ), 1e-6 ) );           // half an impulse: direct, post fader, pre fader

        // the source muted: what goes before the fader still arrives
        source->SetMuted( true );
        NullDevice( engine )->Render( 512 );
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeImpulse( 1000, 1.0f ), source );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.5, Rendered( engine, 0 ), 1e-6 ) );
        source->SetMuted( false );

        // removing the sends: only the direct path again
        CORE::CFutureResult removed = engine.RemoveSend( source, preIndex );
        ASSERT_TRUE( RenderUntilReady( engine, removed ) );
        removed = engine.RemoveSend( source, sendIndex );
        ASSERT_TRUE( RenderUntilReady( engine, removed ) );
        ASSERT_TRUE( IsFailedWith( engine.RemoveSend( source, sendIndex ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        NullDevice( engine )->Render( 512 );
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeImpulse( 1000, 1.0f ), source );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.5, Rendered( engine, 0 ), 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Sends that would make a cycle, to unknown buses or beyond the free slots are refused and change nothing" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( IsFailedWith( engine.AddSend( GUCEF_NULL, GUCEF_NULL, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );   // not running
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* master = engine.GetMasterBus();
        AUDIO::CAudioBus* sfx = engine.CreateBus( "sfx" );
        AUDIO::CAudioBus* ui = engine.CreateBus( "ui", sfx );
        AUDIO::CAudioBus* reverb = engine.CreateBus( "reverb" );
        AUDIO::CAudioBus foreign( "foreign", GUCEF_NULL );

        ASSERT_TRUE( RenderUntilReady( engine, engine.AddSend( ui, reverb, 1.0f, false ) ) );
        NullDevice( engine )->ClearRenderedAudio();
        engine.Play( MakeConstant( 2000, 0.25f ), ui );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        const Float32 before = Rendered( engine, 100 );
        ASSERT_TRUE( Near( 0.5, before, 1e-6 ) );

        ASSERT_TRUE( IsFailedWith( engine.AddSend( sfx, sfx, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.AddSend( sfx, ui, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );          // ui feeds sfx
        ASSERT_TRUE( IsFailedWith( engine.AddSend( reverb, ui, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );       // ui sends to reverb
        ASSERT_TRUE( IsFailedWith( engine.AddSend( master, sfx, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.AddSend( sfx, &foreign, 1.0f, false ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.RemoveSend( &foreign, 0 ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.RemoveSend( sfx, 9 ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        // nothing changed by the refusals
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( before == Rendered( engine, 100 ) );

        // four send slots, then no more
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( RenderUntilReady( engine, engine.AddSend( ui, master, 0.0f, true ) ) );
        ASSERT_TRUE( IsFailedWith( engine.AddSend( ui, master, 0.0f, true ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMeterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Meters read a full scale sine as 0 dBFS peak and -3.01 dB RMS, after the fader of their bus" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* tone = engine.CreateBus( "tone" );
        tone->SetVolumeInDecibels( -6.0f );
        engine.GetMasterBus()->SetVolumeInDecibels( 6.0f );
        engine.Play( MakeSine( 48000 * 3, 1000.0, 1.0f ), tone );
        NullDevice( engine )->Render( 48000 * 2 );
        engine.Update();

        const AUDIO::CAudioLevelMeter& master = engine.GetMasterBus()->GetMeter();
        ASSERT_TRUE( 2 == master.GetChannelCount() );
        ASSERT_TRUE( Near( 0.0, master.GetPeakInDecibels( 0 ), 0.05 ) && Near( 0.0, master.GetPeakInDecibels( 1 ), 0.05 ) );
        ASSERT_TRUE( Near( -3.0103, master.GetRmsInDecibels( 0 ), 0.05 ) && Near( -3.0103, master.GetRmsInDecibels( 1 ), 0.05 ) );
        ASSERT_TRUE( master.GetPeakHold( 0 ) >= master.GetPeak( 0 ) );
        const AUDIO::CAudioLevelMeter& bus = tone->GetMeter();
        ASSERT_TRUE( Near( -6.0, bus.GetPeakInDecibels( 0 ), 0.05 ) && Near( -9.0103, bus.GetRmsInDecibels( 0 ), 0.05 ) );

        // silence reads as the floor
        AUDIO::CAudioEngine quiet;
        ASSERT_TRUE( quiet.Initialize( NullParams() ) );
        NullDevice( quiet )->Render( 4800 );
        ASSERT_TRUE( AUDIO::CAudioLevelMeter::MinimumDecibels == quiet.GetMasterBus()->GetMeter().GetPeakInDecibels( 0 ) );
        ASSERT_TRUE( 0.0f == quiet.GetMasterBus()->GetMeter().GetPeak( 5 ) );               // no such channel
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformEngineEffectTests( void )
{GUCEF_TRACE;

    AUDIO::CAudioEffectRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry();
    registry.RegisterFactory( AUDIO::TAudioEffectFactoryPtr( GUCEF_NEW TTestAudioEffectFactory() ) );

    GUCEF_TESTFW_TESTCASE( "AudioRouting: The engine makes prepared effects, places them in bus slots and takes them out with futures" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.CreateEffect( "testeffect" ).IsNULL() );                        // not running
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        ASSERT_TRUE( engine.CreateEffect( "nosucheffect" ).IsNULL() );
        CORE::CValueList options;
        options.Set( "invert", "true" );
        AUDIO::TAudioEffectPtr effect = engine.CreateEffect( "testeffect", options );
        ASSERT_FALSE( effect.IsNULL() );
        ASSERT_TRUE( effect->IsPreparedFor( engine.GetMixFormat(), 4096 ) );

        AUDIO::CAudioBus* bus = engine.CreateBus( "effects" );
        engine.Play( MakeConstant( 48000, 0.25f ), bus );
        NullDevice( engine )->Render( 256 );
        engine.Update();
        ASSERT_TRUE( Near( 0.25, Rendered( engine, 100 ), 1e-6 ) );

        CORE::CFutureResult inserted = engine.InsertEffect( bus, 3, effect );
        ASSERT_FALSE( inserted.HasNoFuture() );
        ASSERT_FALSE( inserted.IsReady() );                                                 // the audio thread has not run yet
        ASSERT_TRUE( RenderUntilReady( engine, inserted ) && inserted.IsReady() && !inserted.HasNoFuture() );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( Near( -0.25, Rendered( engine, 100 ), 1e-6 ) );                       // the effect is in

        // slot settings on the bus, from any thread: half wet, half dry cancels out
        bus->SetEffectMix( 3, 0.5f, 0.5f );
        ASSERT_TRUE( 0.5f == bus->GetEffectWet( 3 ) && 0.5f == bus->GetEffectDry( 3 ) );
        NullDevice( engine )->Render( 256 );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( Near( 0.0, Rendered( engine, 100 ), 1e-6 ) );
        bus->SetEffectMix( 3, 1.0f, 0.0f );
        bus->SetEffectEnabled( 3, false );
        ASSERT_FALSE( bus->IsEffectEnabled( 3 ) );
        NullDevice( engine )->Render( 1024 );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( Near( 0.25, Rendered( engine, 100 ), 1e-6 ) );                        // switched off
        bus->SetEffectEnabled( 3, true );
        NullDevice( engine )->Render( 1024 );

        // refusals, all at once
        AUDIO::TAudioEffectPtr other = engine.CreateEffect( "testeffect" );
        AUDIO::CAudioBus foreign( "foreign", GUCEF_NULL );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 3, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );           // occupied
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 8, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );           // no such slot
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( &foreign, 0, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( engine.GetMasterBus(), 0, effect ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );   // in a slot already
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 0, AUDIO::TAudioEffectPtr() ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        AUDIO::TAudioEffectPtr unprepared( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 0, unprepared ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
        unprepared->Prepare( AUDIO::CAudioFormat( 44100, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 );
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 0, unprepared ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );      // another format
        ASSERT_TRUE( IsFailedWith( engine.RemoveEffect( engine.GetMasterBus(), effect ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );   // not on that bus
        ASSERT_TRUE( IsFailedWith( engine.RemoveEffect( bus, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );

        // out again: ready when released, the slot can take a new effect at once
        CORE::CFutureResult removed = engine.RemoveEffect( bus, effect );
        ASSERT_FALSE( removed.HasNoFuture() );
        ASSERT_TRUE( IsFailedWith( engine.RemoveEffect( bus, effect ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );             // on its way out already
        CORE::CFutureResult replaced = engine.InsertEffect( bus, 3, other );
        ASSERT_FALSE( replaced.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, removed ) && RenderUntilReady( engine, replaced ) );
        NullDevice( engine )->ClearRenderedAudio();
        NullDevice( engine )->Render( 256 );
        ASSERT_TRUE( Near( 0.25, Rendered( engine, 100 ), 1e-6 ) );                        // the new one does not invert

        engine.Shutdown();
        ASSERT_TRUE( IsFailedWith( engine.InsertEffect( bus, 0, other ), CORE::TTaskStatus::TASKSTATUS_SETUP_FAILED ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioRouting: Threaded engine: effects go in and out while audio renders, all futures complete, a removed effect is released on the engine thread" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams( true ) ) );
        ASSERT_TRUE( engine.IsThreaded() );
        AUDIO::CAudioBus* buses[ 3 ] = { engine.CreateBus( "a" ), engine.CreateBus( "b" ), engine.CreateBus( "c", engine.FindBus( "a" ) ) };
        for ( UInt32 b=0; b<3; ++b )
        {
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 48000, 300.0 + 100.0 * b, 0.2f ), buses[ b ] );
            voice->SetLoopCount( 0 );
            engine.Start( voice );
        }
        ASSERT_TRUE( RenderUntilReady( engine, engine.AddSend( buses[ 1 ], buses[ 0 ], 0.5f, false ), 200 ) );

        // the test thread is the audio thread: it renders, the engine thread completes the futures
        UInt32 completed = 0;
        const UInt32 cycles = 60;
        for ( UInt32 cycle=0; cycle<cycles; ++cycle )
        {
            AUDIO::TAudioEffectPtr effect = engine.CreateEffect( "testeffect" );
            AUDIO::CAudioBus* bus = buses[ cycle % 3 ];
            CORE::CFutureResult in = engine.InsertEffect( bus, cycle % 8, effect );
            ASSERT_FALSE( in.HasNoFuture() );
            for ( UInt32 i=0; i<2000 && !in.IsReady(); ++i )
            {
                NullDevice( engine )->Render( 64 );
                MT::ThreadDelay( 0 );
            }
            ASSERT_TRUE( in.IsReady() );
            effect->SetParameter( CTestAudioEffect::PARAMETER_CUTOFF, 500.0f + 10.0f * cycle );
            CORE::CFutureResult out = engine.RemoveEffect( bus, effect );
            effect.Unlink();                                                               // only the engine holds it now
            for ( UInt32 i=0; i<2000 && !out.IsReady(); ++i )
            {
                NullDevice( engine )->Render( 64 );
                MT::ThreadDelay( 0 );
            }
            if ( in.IsReady() && out.IsReady() && !out.HasNoFuture() )
                ++completed;
        }
        ASSERT_TRUE( cycles == completed );
        ASSERT_TRUE( 0 != CTestAudioEffect::LastDestructorThread().Load() );
        ASSERT_TRUE( MT::GetCurrentThreadID() != CTestAudioEffect::LastDestructorThread().Load() );   // not on the audio thread
        ASSERT_TRUE( 0 == engine.GetDroppedEventCount() );
        engine.Shutdown();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    registry.TryUnregisterFactory( "testeffect" );
}

/*-------------------------------------------------------------------------*/

void
PerformAudioRoutingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioRouting TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Routing" );

    PerformRoutingGraphTests();
    PerformBitIdentityTests();
    PerformSendTests();
    PerformMeterTests();
    PerformEngineEffectTests();
}

/*-------------------------------------------------------------------------*/
