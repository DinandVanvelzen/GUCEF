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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#include "gucefAUDIO_CAudioEffectRegistry.h"
#define GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#include "gucefAUDIO_CDuckerEffect.h"
#define GUCEF_AUDIO_CDUCKEREFFECT_H
#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioVoiceSignals.h"
#include "TestAudioIntercomScenario.h"
#include "TestAudioHalfDuplex.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceLinkTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

const UInt32 Rate = 48000;
const UInt32 Block = 240;                   /**< 5 ms capture and playback periods */
const UInt32 EchoDelay = 960;               /**< 20 ms acoustic path (section 6.2) */
const Float64 EchoDb = -15.0;               /**< the speaker in the microphone, section 6.2 */

typedef TestVoiceSignals::TSamples TSamples;

Float64
DbToGain( Float64 decibels )
{
    return pow( 10.0, decibels / 20.0 );
}

Float64
GainToDb( Float64 gain )
{
    return gain > 1.0e-10 ? 20.0 * log10( gain ) : -200.0;
}

/** A stretch of seconds */
struct SSeconds
{
    Float64 from;
    Float64 to;
};

/**
 *  A talker: speech-like bursts (syllables at 4 per second, a gliding fundamental) in the given stretches, the active
 *  RMS 'rms', exact in 16 bit PCM so PCM carries it bit for bit
 */
TSamples
MakeTalker( Float64 seconds, const SSeconds* spans, UInt32 spanCount, UInt64 seed, Float64 rms )
{
    TSamples x( (size_t) ( seconds * Rate ), 0.0f );
    TestVoiceSignals::TMask active( x.size(), 0 );
    for ( UInt32 s=0; s<spanCount; ++s )
    {
        const size_t start = (size_t) ( spans[ s ].from * Rate );
        const size_t length = (size_t) ( ( spans[ s ].to - spans[ s ].from ) * Rate );
        TestVoiceSignals::AddVoicedBurst( x, &active, Rate, start, length, 110.0 + 30.0 * s, 160.0 + 20.0 * s, 1.0, 4.0, (size_t) ( 0.008 * Rate ), seed + s );
    }
    const Float64 current = TestVoiceSignals::MaskedRms( x, active );
    if ( current > 0.0 )
        TestVoiceSignals::Scale( x, rms / current );
    for ( size_t n=0; n<x.size(); ++n )
        x[ n ] = (Float32) floor( x[ n ] * 32768.0f + 0.5f ) / 32768.0f;
    return x;
}

/** A tone burst from 'from' to 'to' seconds with 1 ms ramps */
TSamples
MakeToneBurst( Float64 seconds, Float64 from, Float64 to, Float64 frequency, Float64 rmsDb )
{
    TSamples x( (size_t) ( seconds * Rate ), 0.0f );
    const Float64 amplitude = DbToGain( rmsDb ) * sqrt( 2.0 );
    const size_t a = (size_t) ( from * Rate ), b = (size_t) ( to * Rate );
    for ( size_t n=a; n<b && n<x.size(); ++n )
    {
        Float64 ramp = 1.0;
        if ( n - a < 48 )
            ramp = (Float64) ( n - a ) / 48.0;
        if ( b - n < 48 )
            ramp = (Float64) ( b - n ) / 48.0;
        x[ n ] = (Float32) ( ramp * amplitude * sin( 2.0 * TestVoiceSignals::Pi * frequency * (Float64) ( n - a ) / Rate ) );
    }
    return x;
}

/*-------------------------------------------------------------------------*/

/**
 *  The controller alone, block by block, with the loop of a unit around it: the far signal goes through a ducker (the
 *  controller's manual key) to the speaker, a meter (the far-end bus) and the acoustic path into the microphone, where
 *  the near talker and the noise of the room are added. What the controller decides is recorded per block
 */
class CStandaloneLoop
{
    public:

    CStandaloneLoop( const TSamples& far, const TSamples& near, Float64 echoDb = EchoDb, Float64 noiseDb = -70.0, const AUDIO::CHalfDuplexSettings& settings = AUDIO::CHalfDuplexSettings() )
        : controller( settings ), meter(), ducker(), states(), gains(), played(), mic(), m_far( far ), m_near( near ), m_echoGain( DbToGain( echoDb ) ),
          m_noise( TestVoiceSignals::MakeWhiteNoise( far.size(), DbToGain( noiseDb ), 99 ) )
    {}

    bool Init( bool withDucker = true )
    {
        meter.Prepare( 1, Rate );
        if ( withDucker )
        {
            ducker = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "ducker" );
            if ( ducker.IsNULL() || !ducker->Prepare( VoiceStreamTest::MonoFormat( Rate ), Block ) )
                return false;
            static_cast< AUDIO::CDuckerEffect* >( ducker.GetPointerAlways() )->SetDepthInDecibels( -20.0f );
            if ( !controller.SetDucker( ducker ) )
                return false;
        }
        controller.SetFarEndMeter( &meter );
        return controller.Prepare( Rate );
    }

    void Run( void )
    {
        const size_t blocks = m_far.size() / Block;
        played.assign( blocks * Block, 0.0f );
        mic.assign( blocks * Block, 0.0f );
        Float32 speaker[ Block ];
        for ( size_t k=0; k<blocks; ++k )
        {
            const size_t at = k * Block;
            // the speaker plays the far end (ducked when the key is down), the far-end bus meters it
            memcpy( speaker, &m_far[ at ], Block * sizeof( Float32 ) );
            if ( !ducker.IsNULL() )
                ducker->Process( speaker, Block );
            meter.Update( speaker, Block );
            memcpy( &played[ at ], speaker, Block * sizeof( Float32 ) );
            // the microphone: the near talker, the room and the speaker through the acoustic path
            for ( UInt32 i=0; i<Block; ++i )
            {
                const size_t n = at + i;
                Float32 value = m_near[ n ] + m_noise[ n ];
                if ( n >= EchoDelay )
                    value += (Float32) ( m_echoGain * (Float64) played[ n - EchoDelay ] );
                mic[ n ] = value;
            }
            gains.push_back( controller.Process( &mic[ at ], Block, at ) );
            states.push_back( (UInt8) controller.GetState() );
        }
    }

    /** The block that starts at 'seconds' (rounded to the nearest) */
    static size_t BlockAt( Float64 seconds )
    {
        return (size_t) ( seconds * Rate / Block + 0.5 );
    }

    /** The first block from 'fromS' on in state 'state', -1 when none */
    Float64 FirstIn( AUDIO::THalfDuplexState state, Float64 fromS ) const
    {
        for ( size_t k=BlockAt( fromS ); k<states.size(); ++k )
            if ( (UInt8) state == states[ k ] )
                return (Float64) ( k * Block ) / Rate;
        return -1.0;
    }

    /** The first block from 'fromS' on not in state 'state', -1 when none */
    Float64 FirstNotIn( AUDIO::THalfDuplexState state, Float64 fromS ) const
    {
        for ( size_t k=BlockAt( fromS ); k<states.size(); ++k )
            if ( (UInt8) state != states[ k ] )
                return (Float64) ( k * Block ) / Rate;
        return -1.0;
    }

    /** Blocks in 'state' between two times */
    UInt32 CountIn( AUDIO::THalfDuplexState state, Float64 fromS, Float64 toS ) const
    {
        UInt32 count = 0;
        for ( size_t k=BlockAt( fromS ); k<states.size() && k < BlockAt( toS ); ++k )
            if ( (UInt8) state == states[ k ] )
                ++count;
        return count;
    }

    UInt32 Transitions( Float64 fromS, Float64 toS ) const
    {
        UInt32 count = 0;
        for ( size_t k=BlockAt( fromS ) + 1; k<states.size() && k < BlockAt( toS ); ++k )
            if ( states[ k ] != states[ k - 1 ] )
                ++count;
        return count;
    }

    AUDIO::CHalfDuplexController controller;
    AUDIO::CAudioLevelMeter meter;
    AUDIO::TAudioEffectPtr ducker;
    GUCEF::vector< UInt8 > states;
    GUCEF::vector< Float32 > gains;
    TSamples played;
    TSamples mic;

    private:

    TSamples m_far;
    TSamples m_near;
    Float64 m_echoGain;
    TSamples m_noise;
};

/*-------------------------------------------------------------------------*/

/** Keeps what the capture path hands its consumers */
class CRecordingConsumer : public AUDIO::IAudioCaptureConsumer
{
    public:

    CRecordingConsumer( void ) : recorded() { recorded.reserve( 48000 * 20 ); }

    virtual bool PrepareForCapture( const AUDIO::CAudioFormat&, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& ) GUCEF_VIRTUAL_OVERRIDE
    {
        // test code: the vector was reserved for the length of the tests, it does not grow on the capture thread
        recorded.insert( recorded.end(), mono, mono + frameCount );
    }

    TSamples recorded;
};

/*-------------------------------------------------------------------------*/

void
PerformSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: The settings have the defaults of the design, limit their values and survive a save and load" )
    try
    {
        AUDIO::CHalfDuplexSettings settings;
        ASSERT_TRUE( settings.GetEnabled() && settings.GetDuckFarEnd() && settings.GetFarEndBusName().IsNULLOrEmpty() );
        ASSERT_TRUE( -50.0f == settings.GetFarThresholdInDecibels() && -40.0f == settings.GetMicAttenuationInDecibels() && 6.0f == settings.GetBreakInMarginInDecibels() );
        ASSERT_TRUE( 60.0f == settings.GetBreakInInMs() && 300.0f == settings.GetHangoverInMs() && 2000.0f == settings.GetCouplingTimeInMs() );
        ASSERT_TRUE( 0.0f == settings.GetInitialCouplingInDecibels() && 100.0f == settings.GetFarReleaseInMs() );

        settings.SetFarThresholdInDecibels( -200.0f );
        settings.SetMicAttenuationInDecibels( 10.0f );
        settings.SetBreakInMarginInDecibels( 99.0f );
        settings.SetBreakInInMs( -5.0f );
        settings.SetHangoverInMs( 1.0e9f );
        settings.SetCouplingTimeInMs( 0.0f );
        settings.SetInitialCouplingInDecibels( -1.0e9f );
        settings.SetFarReleaseInMs( 0.0f );
        ASSERT_TRUE( -90.0f == settings.GetFarThresholdInDecibels() && 0.0f == settings.GetMicAttenuationInDecibels() && 30.0f == settings.GetBreakInMarginInDecibels() );
        ASSERT_TRUE( 0.0f == settings.GetBreakInInMs() && 5000.0f == settings.GetHangoverInMs() && 100.0f == settings.GetCouplingTimeInMs() );
        ASSERT_TRUE( -60.0f == settings.GetInitialCouplingInDecibels() && 5.0f == settings.GetFarReleaseInMs() );
        const Float32 notANumber = (Float32) sqrt( -1.0 );
        settings.SetHangoverInMs( notANumber );
        ASSERT_TRUE( 5000.0f == settings.GetHangoverInMs() );

        AUDIO::CHalfDuplexSettings edited;
        edited.SetEnabled( false );
        edited.SetFarThresholdInDecibels( -45.0f );
        edited.SetMicAttenuationInDecibels( -30.0f );
        edited.SetBreakInMarginInDecibels( 8.0f );
        edited.SetBreakInInMs( 80.0f );
        edited.SetHangoverInMs( 250.0f );
        edited.SetCouplingTimeInMs( 1500.0f );
        edited.SetInitialCouplingInDecibels( -10.0f );
        edited.SetFarReleaseInMs( 150.0f );
        edited.SetDuckFarEnd( false );
        edited.SetFarEndBusName( "call" );
        AUDIO::CVoiceActivitySettings vad;
        vad.SetOnsetInDecibels( 12.0f );
        vad.SetHangoverInMs( 150.0f );
        edited.SetVoiceActivitySettings( vad );
        ASSERT_TRUE( edited != AUDIO::CHalfDuplexSettings() );

        CORE::CDataNode tree;
        ASSERT_TRUE( edited.SaveConfig( tree ) );
        AUDIO::CHalfDuplexSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded == edited );
        ASSERT_TRUE( 12.0f == loaded.GetVoiceActivitySettings().GetOnsetInDecibels() && loaded.GetFarEndBusName() == "call" );
        CORE::CDataNode empty;
        ASSERT_FALSE( loaded.LoadConfig( empty ) );
        ASSERT_TRUE( loaded == edited );

        // a ducker only
        AUDIO::CHalfDuplexController controller;
        AUDIO::TAudioEffectPtr limiter = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "limiter" );
        ASSERT_TRUE( !limiter.IsNULL() && !controller.SetDucker( limiter ) && controller.GetDucker().IsNULL() );
        ASSERT_FALSE( controller.Prepare( 1000 ) );
        ASSERT_TRUE( controller.Prepare( 16000 ) && 16000 == controller.GetSampleRate() );
        // without a far end and without a voice it stays idle and leaves the microphone alone
        Float32 silence[ 160 ];
        memset( silence, 0, sizeof( silence ) );
        ASSERT_TRUE( 1.0f == controller.Process( silence, 160, 0 ) && AUDIO::HALFDUPLEXSTATE_IDLE == controller.GetState() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformStateTests( void )
{GUCEF_TRACE;

    const SSeconds farSpans[] = { { 1.0, 4.0 }, { 6.0, 8.5 }, { 11.0, 13.0 } };
    const SSeconds nearSpans[] = { { 4.8, 5.8 }, { 9.0, 10.5 } };

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: The far end alone: FAR while it talks (microphone at -40 dB), IDLE after the hangover, no false break-in from the echo, the coupling found within 2 dB" )
    try
    {
        const TSamples far = MakeTalker( 15.0, farSpans, 3, 11, 0.1 );
        const TSamples near( far.size(), 0.0f );
        CStandaloneLoop loop( far, near );
        ASSERT_TRUE( loop.Init() );
        loop.Run();
        UInt32 farBlocks = 0, nearBlocks = 0;
        for ( UInt32 s=0; s<3; ++s )
        {
            farBlocks += loop.CountIn( AUDIO::HALFDUPLEXSTATE_FAR, farSpans[ s ].from + 0.05, farSpans[ s ].to );
            nearBlocks += loop.CountIn( AUDIO::HALFDUPLEXSTATE_NEAR, farSpans[ s ].from, farSpans[ s ].to + 1.0 );
        }
        const Float64 idleAfter = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_IDLE, 4.0 );
        const UInt32 transitions = loop.Transitions( 0.0, 15.0 );
        Float32 farGain = 1.0f;
        for ( size_t k=0; k<loop.states.size(); ++k )
            if ( (UInt8) AUDIO::HALFDUPLEXSTATE_FAR == loop.states[ k ] )
                farGain = loop.gains[ k ];
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: far end alone: " << farBlocks << " of " << (UInt32) ( ( 3.0 + 2.5 + 2.0 - 0.15 ) * Rate / Block ) << " blocks of far speech in FAR, " << nearBlocks << " in NEAR, IDLE again at " << idleAfter <<
                   " s (speech ends at 4 s), " << transitions << " transitions, microphone gain in FAR " << GainToDb( farGain ) << " dB, coupling " << loop.controller.GetCouplingInDecibels() << " dB (path " << EchoDb << " dB)" );
        ASSERT_TRUE( (Float64) farBlocks >= 0.99 * ( 3.0 + 2.5 + 2.0 - 0.15 ) * Rate / Block );
        ASSERT_TRUE( 0 == nearBlocks );
        ASSERT_TRUE( 6 == transitions );
        ASSERT_TRUE( idleAfter > 4.0 && idleAfter < 4.5 );
        ASSERT_TRUE( fabs( GainToDb( farGain ) + 40.0 ) < 0.01 );
        ASSERT_TRUE( loop.controller.HasMeasuredCoupling() && fabs( loop.controller.GetCouplingInDecibels() - EchoDb ) <= 2.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: The near end alone: NEAR while it talks, the far end ducked by the manual key, IDLE after the hangover; noise alone stays IDLE" )
    try
    {
        const TSamples near = MakeTalker( 12.0, nearSpans, 2, 21, 0.1 );
        const TSamples far( near.size(), 0.0f );
        CStandaloneLoop loop( far, near );
        ASSERT_TRUE( loop.Init() );
        AUDIO::CDuckerEffect* ducker = static_cast< AUDIO::CDuckerEffect* >( loop.ducker.GetPointerAlways() );
        loop.Run();
        const Float64 nearStart = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_NEAR, 4.0 );
        const Float64 idleAfter = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_IDLE, 5.8 );
        UInt32 nearBlocks = 0;
        for ( UInt32 s=0; s<2; ++s )
            nearBlocks += loop.CountIn( AUDIO::HALFDUPLEXSTATE_NEAR, nearSpans[ s ].from + 0.05, nearSpans[ s ].to );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: near end alone: NEAR from " << nearStart << " s (speech from 4.8 s), IDLE again at " << idleAfter << " s (speech ends at 5.8 s), " << nearBlocks << " of " <<
                   (UInt32) ( 2.4 * Rate / Block ) << " speech blocks in NEAR, " << loop.Transitions( 0.0, 12.0 ) << " transitions, FAR " << loop.CountIn( AUDIO::HALFDUPLEXSTATE_FAR, 0.0, 12.0 ) << " blocks" );
        ASSERT_TRUE( nearStart >= 4.8 && nearStart <= 4.85 );
        ASSERT_TRUE( idleAfter > 5.8 && idleAfter < 6.4 );
        ASSERT_TRUE( (Float64) nearBlocks >= 0.99 * 2.4 * Rate / Block );
        ASSERT_TRUE( 4 == loop.Transitions( 0.0, 12.0 ) && 0 == loop.CountIn( AUDIO::HALFDUPLEXSTATE_FAR, 0.0, 12.0 ) );
        ASSERT_TRUE( !ducker->IsManualKeyActive() );

        // the key is down in NEAR: run again and stop inside the talkspurt
        CStandaloneLoop partial( TSamples( (size_t) ( 5.5 * Rate ), 0.0f ), TSamples( near.begin(), near.begin() + (size_t) ( 5.5 * Rate ) ) );
        ASSERT_TRUE( partial.Init() );
        partial.Run();
        ASSERT_TRUE( AUDIO::HALFDUPLEXSTATE_NEAR == partial.controller.GetState() );
        ASSERT_TRUE( static_cast< AUDIO::CDuckerEffect* >( partial.ducker.GetPointerAlways() )->IsManualKeyActive() );

        // steady white noise in the room at -45 dBFS, and the hum of a fan: never a voice
        TSamples room = TestVoiceSignals::MakeWhiteNoise( 10 * Rate, DbToGain( -45.0 ), 5 );
        TestVoiceSignals::MixInto( room, TestVoiceSignals::MakeHum( 10 * Rate, Rate, 50.0, 5, DbToGain( -40.0 ) ) );
        CStandaloneLoop noise( TSamples( room.size(), 0.0f ), room );
        ASSERT_TRUE( noise.Init() );
        noise.Run();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: noise alone: " << noise.Transitions( 0.0, 10.0 ) << " transitions" );
        ASSERT_TRUE( 0 == noise.Transitions( 0.0, 10.0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: Double talk: the near end breaks in after the break-in time and keeps the line while it talks; the far end comes back after the hangover" )
    try
    {
        // the far end talks from 1 to 9 s; the near end, as loud, from 4 to 6 s
        const SSeconds farLong[] = { { 1.0, 9.0 } };
        const SSeconds nearMiddle[] = { { 4.0, 6.0 } };
        const TSamples far = MakeTalker( 10.0, farLong, 1, 31, 0.1 );
        const TSamples near = MakeTalker( 10.0, nearMiddle, 1, 41, 0.1 );
        CStandaloneLoop loop( far, near );
        ASSERT_TRUE( loop.Init() );
        loop.Run();
        const Float64 breakIn = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_NEAR, 3.9 );
        const Float64 leftNear = loop.FirstNotIn( AUDIO::HALFDUPLEXSTATE_NEAR, breakIn );
        const Float64 farAgain = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_FAR, breakIn );
        const UInt32 nearBlocks = loop.CountIn( AUDIO::HALFDUPLEXSTATE_NEAR, breakIn, 6.0 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: double talk: break in at " << breakIn << " s (near speech from 4.0 s), NEAR until " << leftNear << " s (near speech ends at 6.0 s), FAR again at " << farAgain <<
                   " s, " << nearBlocks << " blocks in NEAR of " << (UInt32) ( ( 6.0 - breakIn ) * Rate / Block ) << ", transitions " << loop.Transitions( 0.0, 10.0 ) << ", coupling " << loop.controller.GetCouplingInDecibels() << " dB" );
        ASSERT_TRUE( breakIn >= 4.06 && breakIn <= 4.15 );
        ASSERT_TRUE( (Float64) nearBlocks >= ( 6.0 - breakIn ) * Rate / Block - 1.0 );
        ASSERT_TRUE( leftNear >= 6.0 && leftNear <= 6.5 );
        ASSERT_TRUE( farAgain >= leftNear && farAgain <= leftNear + 0.01 );
        // IDLE to FAR, the break in, NEAR to IDLE, IDLE to FAR, FAR to IDLE
        ASSERT_TRUE( 5 == loop.Transitions( 0.0, 10.0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: Timing with tone bursts: hangovers of 0, 300 and 1000 ms and break-in times of 0, 60 and 200 ms within one block" )
    try
    {
        const Float32 hangovers[] = { 0.0f, 300.0f, 1000.0f };
        const Float32 breakIns[] = { 0.0f, 60.0f, 200.0f };
        for ( UInt32 c=0; c<3; ++c )
        {
            AUDIO::CHalfDuplexSettings settings;
            settings.SetHangoverInMs( hangovers[ c ] );
            settings.SetBreakInInMs( breakIns[ c ] );
            settings.SetFarReleaseInMs( 20.0f );
            // far: a 1 kHz tone from 1 to 3 s; near: a 700 Hz tone as loud from 2 to 2.5 s (double talk), the echo at -15 dB
            const TSamples far = MakeToneBurst( 6.0, 1.0, 3.0, 1000.0, -20.0 );
            const TSamples near = MakeToneBurst( 6.0, 2.0, 2.5, 700.0, -20.0 );
            CStandaloneLoop loop( far, near, EchoDb, -70.0, settings );
            ASSERT_TRUE( loop.Init() );
            loop.Run();
            const Float64 breakIn = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_NEAR, 1.5 );
            const Float64 nearEnd = loop.FirstNotIn( AUDIO::HALFDUPLEXSTATE_NEAR, breakIn );
            const Float64 farEnd = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_IDLE, 3.0 );
            // the detector of the near end opens two analysis frames (10 ms each) after the tone starts and holds 200 ms after it ends
            const Float64 voiceOnset = loop.FirstIn( AUDIO::HALFDUPLEXSTATE_NEAR, 1.5 ) - breakIns[ c ] / 1000.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: timing, hangover " << hangovers[ c ] << " ms, break in " << breakIns[ c ] << " ms: break in at " << breakIn << " s (tone at 2.0 s, the voice found at " << voiceOnset <<
                       " s), NEAR until " << nearEnd << " s (tone ends at 2.5 s), IDLE at " << farEnd << " s (far tone ends at 3.0 s)" );
            // a time is reached within the block that completes it: the state shows from the start of that block (5 ms)
            ASSERT_TRUE( breakIn >= 2.0 + breakIns[ c ] / 1000.0 - 0.006 && breakIn <= 2.0 + breakIns[ c ] / 1000.0 + 0.01 );
            // the near end is heard until its detector releases (hangover 200 ms) plus the controller's hangover
            // NEAR ends the hangover after the near end fell below the echo margin: the microphone power falls with 10 ms,
            // and the echo estimate follows the far end down as the ducker lowers it (within 100 ms of the tone's end)
            ASSERT_TRUE( nearEnd >= 2.5 + hangovers[ c ] / 1000.0 - 0.006 && nearEnd <= 2.5 + hangovers[ c ] / 1000.0 + 0.1 );
            if ( nearEnd < 3.0 )
                ASSERT_TRUE( farEnd >= 3.0 + hangovers[ c ] / 1000.0 - 0.005 && farEnd <= 3.0 + hangovers[ c ] / 1000.0 + 0.01 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: No chatter: syllables and short pauses of the far end keep FAR, and switching off releases the microphone and the key" )
    try
    {
        // 6 s of far speech with its syllable gaps: one FAR stretch, whatever the level of the echo
        const SSeconds farOne[] = { { 1.0, 7.0 } };
        const TSamples far = MakeTalker( 9.0, farOne, 1, 51, 0.1 );
        const Float64 echoes[] = { -30.0, -15.0, -6.0 };
        for ( UInt32 e=0; e<3; ++e )
        {
            CStandaloneLoop loop( far, TSamples( far.size(), 0.0f ), echoes[ e ] );
            ASSERT_TRUE( loop.Init() );
            loop.Run();
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: no chatter, echo " << echoes[ e ] << " dB: " << loop.Transitions( 0.0, 9.0 ) << " transitions, NEAR blocks " << loop.CountIn( AUDIO::HALFDUPLEXSTATE_NEAR, 0.0, 9.0 ) <<
                       ", coupling " << loop.controller.GetCouplingInDecibels() << " dB" );
            ASSERT_TRUE( 2 == loop.Transitions( 0.0, 9.0 ) && 0 == loop.CountIn( AUDIO::HALFDUPLEXSTATE_NEAR, 0.0, 9.0 ) );
        }

        // switched off in the middle of FAR: gain 1 at once, IDLE; back on: FAR again
        CStandaloneLoop loop( far, TSamples( far.size(), 0.0f ) );
        ASSERT_TRUE( loop.Init() );
        Float32 block[ Block ];
        memset( block, 0, sizeof( block ) );
        loop.controller.SetEnabled( false );
        ASSERT_FALSE( loop.controller.IsEnabled() );
        loop.Run();
        ASSERT_TRUE( 0 == loop.Transitions( 0.0, 9.0 ) );
        for ( size_t k=0; k<loop.gains.size(); ++k )
            ASSERT_TRUE( 1.0f == loop.gains[ k ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioEngineParams
CaptureParams( void )
{
    AUDIO::CAudioEngineParams params = CTestIntercomUnit::MakeEngineParams( SUnitConfig() );
    return params;
}

void
PerformEngineTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: In the engine's capture path: set, replace and remove with futures, refusals, the gain ramped click-free inside the mute ramp, mute wins, off passes bit for bit" )
    try
    {
        // without capture
        {
            AUDIO::CAudioEngineParams params = CaptureParams();
            params.SetCaptureEnabled( false );
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( params ) );
            AUDIO::CHalfDuplexController controller;
            ASSERT_TRUE( engine.SetCaptureHalfDuplexController( &controller ).HasNoFuture() );
            engine.Shutdown();
        }

        CTestIntercomUnit unit;
        ASSERT_TRUE( unit.Init() );
        AUDIO::CAudioEngine& engine = *unit.engine;
        CRecordingConsumer consumer;
        ASSERT_FALSE( engine.AddCaptureConsumer( &consumer ).HasNoFuture() );
        // a 1 kHz tone at -60 dBFS in the microphone (below what the detectors take for a voice), 16 bit exact
        TSamples tone = TestVoiceSignals::MakeTone( 12 * Rate, Rate, 1000.0, DbToGain( -60.0 ) );
        for ( size_t n=0; n<tone.size(); ++n )
            tone[ n ] = (Float32) floor( tone[ n ] * 32768.0f + 0.5f ) / 32768.0f;
        CBufferSignal signal( tone );
        unit.SetSignal( &signal );

        // an unknown far-end bus is refused
        AUDIO::CHalfDuplexSettings settings;
        settings.SetFarEndBusName( "nowhere" );
        AUDIO::CHalfDuplexController lost( settings );
        ASSERT_TRUE( engine.SetCaptureHalfDuplexController( &lost ).HasNoFuture() );

        // the far end is a meter the test drives: a tone that starts and stops
        AUDIO::CAudioLevelMeter farMeter;
        farMeter.Prepare( 1, Rate );
        const TSamples farTone = TestVoiceSignals::MakeTone( Block, Rate, 500.0, DbToGain( -20.0 ) );
        const TSamples farSilence( Block, 0.0f );
        AUDIO::CHalfDuplexController first;
        first.SetFarEndMeter( &farMeter );
        CORE::CFutureResult set = engine.SetCaptureHalfDuplexController( &first );
        ASSERT_FALSE( set.HasNoFuture() );
        ASSERT_TRUE( engine.GetCaptureHalfDuplexController() == &first );
        ASSERT_TRUE( engine.SetCaptureHalfDuplexController( &first ).IsReady() );     // the one in use: ready at once
        for ( UInt32 k=0; k<200; ++k )      // 1 s: the far end is quiet
        {
            farMeter.Update( &farSilence[ 0 ], Block );
            unit.Capture( Block );
            engine.Update();
        }
        ASSERT_TRUE( set.IsReady() && AUDIO::HALFDUPLEXSTATE_IDLE == first.GetState() );
        const size_t farStart = consumer.recorded.size();
        for ( UInt32 k=0; k<200; ++k )      // 1 s of far end: FAR
        {
            farMeter.Update( &farTone[ 0 ], Block );
            unit.Capture( Block );
            engine.Update();
        }
        ASSERT_TRUE( AUDIO::HALFDUPLEXSTATE_FAR == first.GetState() );
        const size_t farStop = consumer.recorded.size();
        for ( UInt32 k=0; k<200; ++k )      // 1 s quiet again: IDLE after the hangover
        {
            farMeter.Update( &farSilence[ 0 ], Block );
            unit.Capture( Block );
            engine.Update();
        }
        ASSERT_TRUE( AUDIO::HALFDUPLEXSTATE_IDLE == first.GetState() );

        // the recorded audio: the tone, then 10 ms down to -40 dB, then back up; no step larger than the tone's own
        const TSamples& rec = consumer.recorded;
        Float64 toneStep = 0.0, recordedStep = 0.0;
        for ( size_t n=1; n<rec.size() && n<tone.size(); ++n )
        {
            toneStep = fabs( tone[ n ] - tone[ n - 1 ] ) > toneStep ? fabs( tone[ n ] - tone[ n - 1 ] ) : toneStep;
            recordedStep = fabs( rec[ n ] - rec[ n - 1 ] ) > recordedStep ? fabs( rec[ n ] - rec[ n - 1 ] ) : recordedStep;
        }
        size_t rampStart = farStart, rampEnd = farStart;
        while ( rampStart < farStop && rec[ rampStart ] == tone[ rampStart ] )
            ++rampStart;
        // the ramp ends where the gain reached the attenuation (measured where the tone is far from a zero crossing)
        const Float64 attenuated = DbToGain( -40.0 );
        const Float64 amplitude = DbToGain( -60.0 ) * sqrt( 2.0 );
        rampEnd = rampStart;
        while ( rampEnd < farStop && ( fabs( tone[ rampEnd ] ) < 0.5 * amplitude || rec[ rampEnd ] / tone[ rampEnd ] > attenuated * 1.001 ) )
            ++rampEnd;
        const Float64 steadyDb = GainToDb( TestVoiceSignals::Rms( rec, farStop - 9600, 9600 ) / TestVoiceSignals::Rms( tone, farStop - 9600, 9600 ) );
        size_t bitExactAfter = rec.size();
        for ( size_t n=rec.size(); n>0; --n )
        {
            if ( rec[ n - 1 ] != tone[ n - 1 ] )
                break;
            bitExactAfter = n - 1;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: engine: attenuation starts " << ( (Float64) rampStart - (Float64) farStart ) * 1000.0 / Rate << " ms after the far end, ramp " << ( (Float64) rampEnd - (Float64) rampStart ) * 1000.0 / Rate <<
                   " ms, steady " << steadyDb << " dB, largest step " << recordedStep << " (tone " << toneStep << "), bit for bit again from " << ( (Float64) bitExactAfter - (Float64) farStop ) * 1000.0 / Rate << " ms after the far end stopped" );
        ASSERT_TRUE( rampStart > farStart && rampStart <= farStart + 2 * Block );
        ASSERT_TRUE( rampEnd - rampStart >= 460 && rampEnd - rampStart <= 480 );
        ASSERT_TRUE( fabs( steadyDb + 40.0 ) < 0.01 );
        ASSERT_TRUE( recordedStep <= toneStep + 1.0e-6 );
        ASSERT_TRUE( bitExactAfter >= farStop + (size_t) ( 0.3 * Rate ) && bitExactAfter <= farStop + (size_t) ( 0.33 * Rate ) );

        // mute wins: silence, and back to the attenuation of FAR
        engine.SetCaptureMuted( true );
        for ( UInt32 k=0; k<20; ++k )
        {
            farMeter.Update( &farTone[ 0 ], Block );
            unit.Capture( Block );
        }
        ASSERT_TRUE( 0.0f == rec.back() && AUDIO::HALFDUPLEXSTATE_FAR == first.GetState() );
        engine.SetCaptureMuted( false );

        // replaced by another controller (switched off): the first is let go of, the microphone passes bit for bit
        AUDIO::CHalfDuplexController second;
        second.SetFarEndMeter( &farMeter );
        second.SetEnabled( false );
        CORE::CFutureResult replaced = engine.SetCaptureHalfDuplexController( &second );
        ASSERT_FALSE( replaced.HasNoFuture() );
        ASSERT_TRUE( engine.SetCaptureHalfDuplexController( &first ).HasNoFuture() );     // the capture thread may still use it
        for ( UInt32 k=0; k<10; ++k )
        {
            farMeter.Update( &farTone[ 0 ], Block );
            unit.Capture( Block );
            engine.Update();
        }
        ASSERT_TRUE( replaced.IsReady() && engine.GetCaptureHalfDuplexController() == &second );
        const size_t offFrom = rec.size();
        for ( UInt32 k=0; k<40; ++k )
        {
            farMeter.Update( &farTone[ 0 ], Block );
            unit.Capture( Block );
            engine.Update();
        }
        size_t exact = 0;
        for ( size_t n=offFrom + 480; n<rec.size(); ++n )
            if ( rec[ n ] == tone[ n ] )
                ++exact;
        ASSERT_TRUE( exact == rec.size() - offFrom - 480 && AUDIO::HALFDUPLEXSTATE_IDLE == second.GetState() );

        // removed
        CORE::CFutureResult removed = engine.SetCaptureHalfDuplexController( GUCEF_NULL );
        unit.Capture( Block );
        engine.Update();
        ASSERT_TRUE( removed.IsReady() && GUCEF_NULL == engine.GetCaptureHalfDuplexController() );
        ASSERT_FALSE( engine.SetCaptureHalfDuplexController( &first ).HasNoFuture() );       // let go of: it may come back
        unit.Capture( Block );
        engine.Update();
        engine.SetCaptureHalfDuplexController( GUCEF_NULL );
        unit.Capture( Block );
        engine.Update();
        CORE::CFutureResult consumerRemoved = engine.RemoveCaptureConsumer( &consumer );
        unit.Capture( Block );
        engine.Update();
        ASSERT_TRUE( consumerRemoved.IsReady() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** What the echo scenario measures in the output of B */
struct SEchoResult
{
    Float64 echoDb;                 /**< B's own voice coming back, relative to B's voice, where only B talks */
    Float64 doubleTalkEchoDb;       /**< the same while A talks over B */
    Float64 aloneDb;                /**< A's voice at B where A talks alone */
    Float64 doubleTalkDb;           /**< A's voice at B where A talks over B (after the break in) */
    Float64 lockOutMs;              /**< the longest time from the start of a double talk until A is heard within 3 dB */
    Float64 couplingDb;
    UInt64 transitions;

    SEchoResult( void ) : echoDb( 0.0 ), doubleTalkEchoDb( 0.0 ), aloneDb( 0.0 ), doubleTalkDb( 0.0 ), lockOutMs( 0.0 ), couplingDb( 0.0 ), transitions( 0 ) {}
};

/** The gain with which 'signal', delayed by 'lag', is in 'output' over [from, to) of the signal */
Float64
Projection( const TSamples& output, const TSamples& signal, size_t lag, size_t from, size_t to )
{
    Float64 xy = 0.0, xx = 0.0;
    for ( size_t n=from; n<to && n<signal.size() && n + lag<output.size(); ++n )
    {
        xy += (Float64) output[ n + lag ] * (Float64) signal[ n ];
        xx += (Float64) signal[ n ] * (Float64) signal[ n ];
    }
    return xx > 0.0 ? xy / xx : 0.0;
}

/**
 *  The call of section 6.2: units A and B, each with a sender and a receiver (PCM, wired, constant delay, fixed margin
 *  so the playout runs at ratio 1 and the lags are exact). A's speaker reaches A's microphone at -15 dB after 20 ms; A's
 *  receiver plays on the bus "call" with a ducker (depth -20 dB). B talks in three stretches; A talks alone once and
 *  over B twice. With 'controller' the half-duplex controller of A is in A's capture path
 */
bool
RunEchoCall( bool controller, Float64 seconds, SEchoResult& result )
{
    const SSeconds bSpans[] = { { 1.0, 4.0 }, { 7.0, 10.0 }, { 13.0, 16.0 } };
    const SSeconds aSpans[] = { { 4.8, 6.3 }, { 8.0, 9.2 }, { 14.0, 15.0 } };
    const TSamples bVoice = MakeTalker( seconds, bSpans, 3, 61, 0.1 );
    const TSamples aVoice = MakeTalker( seconds, aSpans, 3, 71, 0.1 );
    TSamples aRoom = TestVoiceSignals::MakeWhiteNoise( aVoice.size(), DbToGain( -65.0 ), 81 );
    for ( size_t n=0; n<aRoom.size(); ++n )
        aRoom[ n ] = (Float32) floor( ( aRoom[ n ] + aVoice[ n ] ) * 32768.0f + 0.5f ) / 32768.0f;
    CBufferSignal aSignal( aRoom );
    CBufferSignal bSignal( bVoice );

    CTestIntercomScenario scenario( Block, Rate );
    SUnitConfig aConfig;
    aConfig.echoGain = DbToGain( EchoDb );
    aConfig.echoDelayFrames = EchoDelay;
    aConfig.keepOutput = false;
    SUnitConfig bConfig;
    CTestIntercomUnit* a = scenario.AddUnit( aConfig );
    CTestIntercomUnit* b = scenario.AddUnit( bConfig );
    if ( GUCEF_NULL == a || GUCEF_NULL == b )
        return false;
    a->SetSignal( &aSignal );
    b->SetSignal( &bSignal );

    const AUDIO::CVoiceCodecConfig codec = VoiceStreamTest::PcmConfig( Rate, 10000 );
    AUDIO::CVoiceSenderConfig senderConfig;
    senderConfig.SetCodecConfig( codec );
    senderConfig.SetFirstSequenceNumber( 1 );
    senderConfig.SetFirstTimestamp( 0 );
    senderConfig.SetStreamId( 0xA );
    if ( !a->AttachSender( senderConfig ) )
        return false;
    senderConfig.SetStreamId( 0xB );
    if ( !b->AttachSender( senderConfig ) )
        return false;

    AUDIO::CVoiceReceiverConfig receiverConfig;
    receiverConfig.SetCodecConfig( codec );
    receiverConfig.SetPlayoutProfile( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
    receiverConfig.SetMinMarginInMs( receiverConfig.GetInitialMarginInMs() );
    AUDIO::CAudioBus* aCall = a->engine->CreateBus( "call" );
    AUDIO::CAudioBus* bCall = b->engine->CreateBus( "call" );
    AUDIO::TAudioEffectPtr ducker = a->engine->CreateEffect( "ducker" );
    if ( GUCEF_NULL == aCall || GUCEF_NULL == bCall || ducker.IsNULL() )
        return false;
    static_cast< AUDIO::CDuckerEffect* >( ducker.GetPointerAlways() )->SetDepthInDecibels( -20.0f );
    CORE::CFutureResult inserted = a->engine->InsertEffect( aCall, 0, ducker );
    AUDIO::CVoiceReceiver* atA = a->AddReceiver( receiverConfig, aCall );
    AUDIO::CVoiceReceiver* atB = b->AddReceiver( receiverConfig, bCall );
    if ( inserted.HasNoFuture() || GUCEF_NULL == atA || GUCEF_NULL == atB )
        return false;
    SLinkModel model = SLinkModel::Wired();
    model.jitter = SLinkModel::JITTER_NONE;
    scenario.Connect( a, b, atB, model );
    scenario.Connect( b, a, atA, model );

    AUDIO::CHalfDuplexSettings settings;
    settings.SetFarEndBusName( "call" );
    AUDIO::CHalfDuplexController halfDuplex( settings );
    if ( controller )
    {
        if ( !halfDuplex.SetDucker( ducker ) )
            return false;
        CORE::CFutureResult set = a->engine->SetCaptureHalfDuplexController( &halfDuplex );
        if ( set.HasNoFuture() )
            return false;
    }

    scenario.Run( seconds );
    result.couplingDb = halfDuplex.GetCouplingInDecibels();
    result.transitions = halfDuplex.GetTransitionCount();

    // the lags: one way A to B, and B's voice round trip (B to A, the acoustic path, A to B)
    const TSamples& out = b->output;
    size_t oneWay = 0, roundTrip = 0;
    Float64 best = -1.0;
    for ( size_t lag=600; lag<800; ++lag )
    {
        // A alone, B silent: where A's voice sits in B's output
        const Float64 c = Projection( out, aVoice, lag, (size_t) ( 5.0 * Rate ), (size_t) ( 5.5 * Rate ) );
        if ( c > best )
        {
            best = c;
            oneWay = lag;
        }
    }
    // the round trip: one way to A, played a period later, the acoustic path, one way back
    roundTrip = 2 * oneWay + Block + EchoDelay;

    // B's voice coming back, over the stretches where only B talks (away from A's talk and the edges)
    const Float64 bOnly = Projection( out, bVoice, roundTrip, (size_t) ( 1.5 * Rate ), (size_t) ( 4.0 * Rate ) );
    const Float64 bOnly2 = Projection( out, bVoice, roundTrip, (size_t) ( 10.0 * Rate ), (size_t) ( 10.0 * Rate ) );
    (void) bOnly2;
    const Float64 bDouble = Projection( out, bVoice, roundTrip, (size_t) ( 8.2 * Rate ), (size_t) ( 9.2 * Rate ) );
    result.echoDb = GainToDb( fabs( bOnly ) );
    result.doubleTalkEchoDb = GainToDb( fabs( bDouble ) );
    result.aloneDb = GainToDb( fabs( Projection( out, aVoice, oneWay, (size_t) ( 5.0 * Rate ), (size_t) ( 6.3 * Rate ) ) ) );
    result.doubleTalkDb = GainToDb( fabs( Projection( out, aVoice, oneWay, (size_t) ( 8.2 * Rate ), (size_t) ( 9.2 * Rate ) ) ) );

    // lock-out: from the start of each double talk, the first 20 ms window with A at its full level (within 3 dB)
    const Float64 starts[] = { 8.0, 14.0 };
    result.lockOutMs = 0.0;
    for ( UInt32 s=0; s<2; ++s )
    {
        Float64 heardAt = 1.0e9;
        for ( size_t w=(size_t) ( starts[ s ] * Rate ); w<(size_t) ( ( starts[ s ] + 1.0 ) * Rate ); w+=960 )
        {
            if ( TestVoiceSignals::Rms( aVoice, w, 960 ) < 0.03 )
                continue;   // a pause between the syllables
            if ( GainToDb( Projection( out, aVoice, oneWay, w, w + 960 ) ) >= -3.0 )
            {
                heardAt = (Float64) w / Rate;
                break;
            }
        }
        const Float64 lockOut = ( heardAt - starts[ s ] ) * 1000.0;
        if ( lockOut > result.lockOutMs )
            result.lockOutMs = lockOut;
    }

    if ( controller )
    {
        a->engine->SetCaptureHalfDuplexController( GUCEF_NULL );
        a->Capture( Block );
        a->engine->Update();
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: echo call " << ( controller ? "with" : "without" ) << " the controller: lags " << oneWay << " and " << roundTrip << " samples" );
    return true;
}

void
PerformEchoTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: Echo scenario of 6.2 (A's speaker in A's microphone at -15 dB, 20 ms): without the controller B hears its own voice at about -15 dB, with it below -40 dB; double talk does not lock A out beyond 150 ms" )
    try
    {
        SEchoResult off, on;
        ASSERT_TRUE( RunEchoCall( false, 17.0, off ) );
        ASSERT_TRUE( RunEchoCall( true, 17.0, on ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: echo at B, B alone: " << off.echoDb << " dB without, " << on.echoDb << " dB with the controller (suppressed by " << off.echoDb - on.echoDb <<
                   " dB); during double talk " << off.doubleTalkEchoDb << " / " << on.doubleTalkEchoDb << " dB; A alone at B " << off.aloneDb << " / " << on.aloneDb << " dB; A over B " << off.doubleTalkDb << " / " <<
                   on.doubleTalkDb << " dB; lock-out " << on.lockOutMs << " ms; coupling " << on.couplingDb << " dB; " << on.transitions << " transitions" );
        ASSERT_TRUE( fabs( off.echoDb - EchoDb ) <= 1.0 );
        ASSERT_TRUE( on.echoDb <= -40.0 );
        ASSERT_TRUE( off.echoDb - on.echoDb >= 25.0 );
        ASSERT_TRUE( fabs( on.aloneDb ) <= 0.5 && fabs( off.aloneDb ) <= 0.1 );
        ASSERT_TRUE( on.doubleTalkDb >= -1.0 );
        ASSERT_TRUE( on.lockOutMs <= 150.0 );
        ASSERT_TRUE( fabs( on.couplingDb - EchoDb ) <= 2.0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The capture thread of the threaded test: renders periods with the far-end meter it updates itself */
class CCaptureThread : public MT::CActiveObject
{
    public:

    CCaptureThread( CTestIntercomUnit& unit, AUDIO::CAudioLevelMeter& meter ) : MT::CActiveObject(), stop( 0 ), cycles( 0 ), m_unit( unit ), m_meter( meter ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        const TSamples tone = TestVoiceSignals::MakeTone( Block, Rate, 500.0, DbToGain( -20.0 ) );
        const TSamples silence( Block, 0.0f );
        UInt32 k = 0;
        while ( 0 == stop.Load() )
        {
            m_meter.Update( ( ( k++ / 40 ) % 2 ) ? &tone[ 0 ] : &silence[ 0 ], Block );
            m_unit.Capture( Block );
            cycles.FetchAdd( 1 );
            MT::PrecisionDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    MT::CTAtomic< UInt32 > stop;
    MT::CTAtomic< UInt32 > cycles;

    private:

    CTestIntercomUnit& m_unit;
    AUDIO::CAudioLevelMeter& m_meter;
};

void
PerformThreadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: A capture thread runs the controller while the owner switches it, reads it and replaces it (TSan)" )
    try
    {
        SUnitConfig config;
        config.threading = AUDIO::AUDIOTHREADING_THREADED;
        config.keepOutput = false;
        CTestIntercomUnit unit( config );
        ASSERT_TRUE( unit.Init() );
        const TSamples talk = TestVoiceSignals::MakeTone( 60 * Rate, Rate, 300.0, DbToGain( -30.0 ) );
        CBufferSignal signal( talk );
        unit.SetSignal( &signal );
        AUDIO::CAudioLevelMeter meter;
        meter.Prepare( 1, Rate );
        AUDIO::CHalfDuplexController controllers[ 2 ];
        controllers[ 0 ].SetFarEndMeter( &meter );
        controllers[ 1 ].SetFarEndMeter( &meter );
        // the capture thread carries the command out once it renders
        CORE::CFutureResult set = unit.engine->SetCaptureHalfDuplexController( &controllers[ 0 ] );
        CCaptureThread thread( unit, meter );
        ASSERT_TRUE( thread.Activate() );
        ASSERT_TRUE( unit.engine->WaitFor( set, 5000 ) );
        UInt32 replaced = 0;
        for ( UInt32 i=0; i<60; ++i )
        {
            MT::PrecisionDelay( 10 );
            AUDIO::CHalfDuplexController* current = unit.engine->GetCaptureHalfDuplexController();
            if ( GUCEF_NULL != current )
            {
                current->SetEnabled( 0 != i % 7 );
                (void) current->GetState();
                (void) current->GetMicGain();
                (void) current->GetCouplingInDecibels();
                (void) current->GetNearLevelInDecibels();
                (void) current->GetTransitionCount();
            }
            if ( 0 == i % 15 )
            {
                AUDIO::CHalfDuplexController* next = current == &controllers[ 0 ] ? &controllers[ 1 ] : &controllers[ 0 ];
                if ( unit.engine->WaitFor( unit.engine->SetCaptureHalfDuplexController( next ), 5000 ) )
                    ++replaced;
            }
            unit.engine->SetCaptureMuted( 3 == i % 11 );
        }
        // the controllers outlive the capture thread
        unit.engine->WaitFor( unit.engine->SetCaptureHalfDuplexController( GUCEF_NULL ), 5000 );
        thread.stop.Store( 1 );
        thread.WaitForThreadToFinish( 10000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioHalfDuplex: threads: " << thread.cycles.Load() << " capture periods, " << replaced << " replacements, transitions " << controllers[ 0 ].GetTransitionCount() << " and " << controllers[ 1 ].GetTransitionCount() );
        ASSERT_TRUE( replaced >= 3 && thread.cycles.Load() > 100 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRealtimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioHalfDuplex: The controller and the ducker's key allocate nothing and take no lock on the capture thread (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        // every capture callback of the call below runs the controller in a real time context of this build
        SEchoResult result;
        ASSERT_TRUE( RunEchoCall( true, 10.0, result ) );
        ASSERT_TRUE( result.transitions > 4 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/

void
PerformAudioHalfDuplexTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioHalfDuplex TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.HalfDuplex" );

    PerformSettingsTests();
    PerformStateTests();
    PerformEngineTests();
    PerformEchoTests();
    PerformThreadTests();
    PerformRealtimeTests();
}

/*-------------------------------------------------------------------------*/
