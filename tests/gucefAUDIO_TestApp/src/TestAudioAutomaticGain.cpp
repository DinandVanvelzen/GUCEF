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

#ifndef GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H
#include "gucefAUDIO_CAutomaticGainEffect.h"
#define GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H
#endif /* GUCEF_AUDIO_CAUTOMATICGAINEFFECT_H ? */

#ifndef GUCEF_AUDIO_CNOISEGATEEFFECT_H
#include "gucefAUDIO_CNoiseGateEffect.h"
#define GUCEF_AUDIO_CNOISEGATEEFFECT_H
#endif /* GUCEF_AUDIO_CNOISEGATEEFFECT_H ? */

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#include "gucefAUDIO_CFilterEffect.h"
#define GUCEF_AUDIO_CFILTEREFFECT_H
#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */

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
#include "TestAudioVoiceSignals.h"

#include "TestAudioAutomaticGain.h"

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

namespace TVS = TestVoiceSignals;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef AUDIO::CAutomaticGainEffect TAgc;

const UInt32 SubBlock = GUCEF_AUDIO_EFFECT_SUBBLOCK_FRAMES;

bool
PrepareAgc( TAgc& agc, UInt32 rate, UInt32 channels, UInt32 maxBlock = 4096 )
{
    return agc.Prepare( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), maxBlock );
}

/** Speech with white noise at 'noiseRmsInDecibels', 16 kHz unless told otherwise */
struct SMaterial
{
    TVS::SSpeech speech;
    TSamples mixed;
    SMaterial( void ) : speech(), mixed() {}
};

void
MakeMaterial( SMaterial& material, Float64 seconds, UInt64 seed, Float64 activeRmsInDecibels, Float64 noiseRmsInDecibels, Float64 minPause = 0.5, Float64 maxPause = 1.5, UInt32 rate = 16000 )
{
    TVS::SSpeechSpec spec;
    spec.sampleRate = rate;
    spec.seconds = seconds;
    spec.seed = seed;
    spec.minPauseSeconds = minPause;
    spec.maxPauseSeconds = maxPause;
    spec.activeRms = TVS::FromDecibels( activeRmsInDecibels );
    TVS::MakeSpeech( spec, material.speech );
    TSamples noise = TVS::MakeWhiteNoise( material.speech.samples.size(), TVS::FromDecibels( noiseRmsInDecibels ), seed + 100 );
    material.mixed = material.speech.samples;
    TVS::MixInto( material.mixed, noise );
}

/** 'tail' after 'head', with its talkspurts and mask moved along */
void
Concatenate( SMaterial& head, const SMaterial& tail )
{
    const UInt64 offset = head.mixed.size();
    for ( size_t i=0; i<tail.speech.talkspurts.size(); ++i )
        head.speech.talkspurts.push_back( TVS::SSpan( tail.speech.talkspurts[ i ].start + offset, tail.speech.talkspurts[ i ].length ) );
    head.speech.active.insert( head.speech.active.end(), tail.speech.active.begin(), tail.speech.active.end() );
    head.speech.samples.insert( head.speech.samples.end(), tail.speech.samples.begin(), tail.speech.samples.end() );
    head.mixed.insert( head.mixed.end(), tail.mixed.begin(), tail.mixed.end() );
}

/** The pauses, shortened: from 'after' seconds past the end of a talkspurt to 'before' seconds before the next, from sample 'notBefore' on */
void
PauseWindows( const TVS::SSpeech& speech, Float64 after, Float64 before, UInt64 notBefore, TVS::TSpans& windows )
{
    const Float64 rate = (Float64) speech.sampleRate;
    UInt64 from = (UInt64) ( 0.3 * rate );
    for ( size_t i=0; i<=speech.talkspurts.size(); ++i )
    {
        const UInt64 next = i < speech.talkspurts.size() ? speech.talkspurts[ i ].start : (UInt64) speech.samples.size();
        const UInt64 stop = next > (UInt64) ( before * rate ) ? next - (UInt64) ( before * rate ) : 0;
        const UInt64 start = from > notBefore ? from : notBefore;
        if ( stop > start + (UInt64) ( 0.05 * rate ) )
            windows.push_back( TVS::SSpan( start, stop - start ) );
        if ( i < speech.talkspurts.size() )
            from = speech.talkspurts[ i ].End() + (UInt64) ( after * rate );
    }
}

Float64
MeanSquareIn( const TSamples& x, const TVS::TSpans& windows )
{
    Float64 sum = 0.0;
    UInt64 count = 0;
    for ( size_t w=0; w<windows.size(); ++w )
    {
        for ( UInt64 i=windows[ w ].start; i<windows[ w ].End() && i<x.size(); ++i )
        {
            sum += (Float64) x[ (size_t) i ] * (Float64) x[ (size_t) i ];
            ++count;
        }
    }
    return count > 0 ? sum / (Float64) count : 0.0;
}

/** RMS in dB over the active samples from 'from' on */
Float64
ActiveLevel( const TSamples& x, const TVS::TMask& active, size_t from, size_t to )
{
    Float64 sum = 0.0;
    size_t count = 0;
    for ( size_t i=from; i<to && i<x.size() && i<active.size(); ++i )
    {
        if ( 0 != active[ i ] )
        {
            sum += (Float64) x[ i ] * (Float64) x[ i ];
            ++count;
        }
    }
    return count > 0 ? 10.0 * log10( sum / (Float64) count ) : -200.0;
}

/** Runs the effect in blocks of 'block' frames (mono or any channel count) and records the gain after each block */
void
RunTraced( TAgc& agc, TSamples& audio, UInt32 channels, UInt32 block, GUCEF::vector< Float32 >& gainTrace )
{
    const size_t frames = audio.size() / channels;
    size_t done = 0;
    while ( done < frames )
    {
        const size_t count = frames - done < block ? frames - done : block;
        ProcessRealtime( agc, &audio[ done * channels ], (UInt32) count );
        gainTrace.push_back( agc.GetGainInDecibels() );
        done += count;
    }
}

/** First time in seconds at which the trace is within 'tolerance' of 'value', -1 when it never is. 'deviation' gets the largest distance from 'value' after that */
Float64
ArrivalTime( const GUCEF::vector< Float32 >& trace, Float64 secondsPerPoint, Float64 value, Float64 tolerance, Float64* deviation = GUCEF_NULL )
{
    for ( size_t i=0; i<trace.size(); ++i )
    {
        if ( fabs( (Float64) trace[ i ] - value ) <= tolerance )
        {
            if ( GUCEF_NULL != deviation )
            {
                *deviation = 0.0;
                for ( size_t k=i; k<trace.size(); ++k )
                    if ( fabs( (Float64) trace[ k ] - value ) > *deviation )
                        *deviation = fabs( (Float64) trace[ k ] - value );
            }
            return (Float64) i * secondsPerPoint;
        }
    }
    return -1.0;
}

Float64
FractionOfActive( const TVS::TMask& active )
{
    size_t count = 0;
    for ( size_t i=0; i<active.size(); ++i )
        count += active[ i ];
    return (Float64) count / (Float64) active.size();
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

AUDIO::TAudioClipPtr
MakeConstantClip( Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( 48000, 2, AUDIO::SAMPLETYPE_FLOAT32 ), 480000 );
    for ( UInt32 f=0; f<480000; ++f )
    {
        audio.SetSampleFromFloat( f, 0, value );
        audio.SetSampleFromFloat( f, 1, value );
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

// ---- the capture path on a null capture device -----------------------------------------------------------------------

/** A source that plays the frames of a buffer once and then has nothing */
class CBufferSource : public AUDIO::IAudioSource
{
    public:

    explicit CBufferSource( const AUDIO::CAudioBuffer& buffer ) : m_buffer( buffer ), m_position( 0 ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFormat(); }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt32 left = ( m_position < m_buffer.GetFrameCount() ) ? m_buffer.GetFrameCount() - (UInt32) m_position : 0;
        const UInt32 count = ( left < frameCapacity ) ? left : frameCapacity;
        if ( count > 0 )
            memcpy( destination, m_buffer.GetDataPtr( (UInt32) m_position ), (size_t) count * m_buffer.GetFormat().GetBytesPerFrame() );
        m_position += count;
        return count;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position >= m_buffer.GetFrameCount(); }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }
    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_buffer.GetFrameCount(); }

    private:

    AUDIO::CAudioBuffer m_buffer;
    UInt64 m_position;
};

/** Keeps the processed capture audio and the decision of the voice detector per block in memory made beforehand */
class CRecorder : public AUDIO::IAudioCaptureConsumer
{
    public:

    struct SBlock
    {
        UInt64 firstFrame;
        UInt32 frames;
        bool voiceActive;
        SBlock( void ) : firstFrame( 0 ), frames( 0 ), voiceActive( false ) {}
    };

    CRecorder( size_t capacityInFrames, size_t maxBlocks ) : samples( capacityInFrames, 0.0f ), frames( 0 ), blocks( maxBlocks, SBlock() ), blockCount( 0 ) {}

    virtual bool PrepareForCapture( const AUDIO::CAudioFormat&, UInt32 ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual void OnCapturedAudio( const Float32* mono, UInt32 frameCount, const SCaptureBlockInfo& info ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT
    {
        const size_t room = samples.size() - frames;
        const size_t count = ( frameCount < room ) ? frameCount : room;
        if ( count > 0 )
            memcpy( &samples[ frames ], mono, count * sizeof( Float32 ) );
        frames += count;
        if ( blockCount < blocks.size() )
        {
            blocks[ blockCount ].firstFrame = info.firstFrame;
            blocks[ blockCount ].frames = frameCount;
            blocks[ blockCount ].voiceActive = info.voiceActive;
        }
        ++blockCount;
    }

    TSamples samples;
    size_t frames;
    GUCEF::vector< SBlock > blocks;
    size_t blockCount;
};

/** What a run of the capture path gave */
struct SChainRun
{
    TSamples recorded;
    GUCEF::vector< CRecorder::SBlock > blocks;
    Float32 gateGainAtEnd;
    Float32 agcGainAtEnd;
};

/**
 *  Plays 'input' (mono, 16 kHz) into the capture path of an engine on the null driver. With 'chain' the path has a high pass
 *  (100 Hz, 24 dB per octave) before the echo stage and the noise gate and the automatic gain behind it; without, the path
 *  is empty
 */
bool
RunChain( const TSamples& input, bool chain, SChainRun& run )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIO::AUDIOLIMITERMODE_INSTANT );
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    params.SetCaptureEnabled( true );
    params.SetCaptureProcessingRate( 16000 );
    params.GetCaptureDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( 16000, 1, AUDIO::SAMPLETYPE_FLOAT32 ) );
    params.GetCaptureDeviceParams().SetBufferSizeInFrames( 160 );

    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( params ) || !engine.IsCaptureAvailable() )
        return false;
    AUDIO::TAudioDevicePtr device = engine.GetCaptureDevice();
    if ( device.IsNULL() || GUCEF_NULL == device->AsNullDevice() )
        return false;
    AUDIO::CNullAudioDevice* null = device->AsNullDevice();

    AUDIO::CAudioBuffer buffer( device->GetFormat(), (UInt32) input.size() );
    for ( size_t f=0; f<input.size(); ++f )
        buffer.SetSampleFromFloat( (UInt32) f, 0, input[ f ] );
    if ( !null->SetCaptureSource( AUDIO::TAudioSourcePtr( GUCEF_NEW CBufferSource( buffer ) ) ) )
        return false;

    AUDIO::TAudioEffectPtr gate, agc;
    GUCEF::vector< CORE::CFutureResult > futures;
    if ( chain )
    {
        AUDIO::TAudioEffectPtr highPass = engine.CreateCaptureEffect( "filter" );
        gate = engine.CreateCaptureEffect( "noiseGate" );
        agc = engine.CreateCaptureEffect( "automaticGain" );
        if ( highPass.IsNULL() || gate.IsNULL() || agc.IsNULL() )
            return false;
        highPass.StaticCast< AUDIO::CFilterEffect >()->SetFilterType( AUDIO::FILTERBANDTYPE_HIGHPASS );
        highPass.StaticCast< AUDIO::CFilterEffect >()->SetFrequency( 100.0f );
        highPass.StaticCast< AUDIO::CFilterEffect >()->SetSlopeInDecibelsPerOctave( 24 );
        futures.push_back( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, highPass ) );
        futures.push_back( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, gate ) );
        futures.push_back( engine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, agc ) );
        for ( size_t i=0; i<futures.size(); ++i )
            if ( futures[ i ].HasNoFuture() )
                return false;
    }

    const UInt64 totalFrames = (UInt64) input.size() + 16000;
    CRecorder recorder( (size_t) totalFrames + 8192, (size_t) totalFrames / 16 + 1024 );
    CORE::CFutureResult added = engine.AddCaptureConsumer( &recorder );
    if ( added.HasNoFuture() )
        return false;
    UInt64 rendered = 0;
    while ( rendered < totalFrames )
    {
        const UInt64 step = ( totalFrames - rendered < 1600 ) ? totalFrames - rendered : 1600;
        null->Render( step );
        engine.Update();
        rendered += step;
    }
    if ( chain )
    {
        run.gateGainAtEnd = gate.StaticCast< AUDIO::CNoiseGateEffect >()->GetGainInDecibels();
        run.agcGainAtEnd = agc.StaticCast< AUDIO::CAutomaticGainEffect >()->GetGainInDecibels();
    }
    CORE::CFutureResult removed = engine.RemoveCaptureConsumer( &recorder );
    null->Render( 160 );
    engine.Update();
    if ( !added.IsReady() || !removed.IsReady() || recorder.blockCount > recorder.blocks.size() )
        return false;
    for ( size_t i=0; i<futures.size(); ++i )
        if ( !futures[ i ].IsReady() )
            return false;

    run.recorded.assign( recorder.samples.begin(), recorder.samples.begin() + recorder.frames );
    run.blocks.assign( recorder.blocks.begin(), recorder.blocks.begin() + recorder.blockCount );
    engine.Shutdown();
    return true;
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

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Type name, parameter descriptors, defaults, limits, voice detector options, no latency or tail, registered as built-in type, rate check" )
    try
    {
        TAgc agc;
        ASSERT_TRUE( agc.GetEffectTypeName() == "automaticGain" && TAgc::GetStaticEffectTypeName() == "automaticGain" );
        ASSERT_TRUE( 8 == agc.GetParameterCount() && 8 == TAgc::GetStaticParameterCount() );
        ASSERT_TRUE( -20.0f == agc.GetParameter( TAgc::PARAMETER_TARGET ) );
        ASSERT_TRUE( 24.0f == agc.GetParameter( TAgc::PARAMETER_MAX_GAIN ) );
        ASSERT_TRUE( -12.0f == agc.GetParameter( TAgc::PARAMETER_MIN_GAIN ) );
        ASSERT_TRUE( 6.0f == agc.GetParameter( TAgc::PARAMETER_RISE_RATE ) );
        ASSERT_TRUE( 20.0f == agc.GetParameter( TAgc::PARAMETER_FALL_RATE ) );
        ASSERT_TRUE( 500.0f == agc.GetParameter( TAgc::PARAMETER_LEVEL_TIME ) );
        ASSERT_TRUE( (Float32) TAgc::FREEZE_ON == agc.GetParameter( TAgc::PARAMETER_FREEZE ) );
        ASSERT_TRUE( -1.0f == agc.GetParameter( TAgc::PARAMETER_CEILING ) );

        UInt32 id = 0;
        ASSERT_TRUE( agc.TryGetParameterId( "maxGain", id ) && TAgc::PARAMETER_MAX_GAIN == id );
        ASSERT_TRUE( agc.TryGetParameterId( "FREEZE", id ) && TAgc::PARAMETER_FREEZE == id );
        const AUDIO::CAudioEffectParameterInfo* freeze = agc.GetParameterInfo( TAgc::PARAMETER_FREEZE );
        ASSERT_TRUE( GUCEF_NULL != freeze && AUDIO::AUDIOPARAMKIND_ENUMERATION == freeze->GetKind() );
        ASSERT_TRUE( freeze->GetEnumLabel( 0 ) == "off" && freeze->GetEnumLabel( 1 ) == "on" );

        agc.SetTargetInDecibels( -90.0f );
        ASSERT_TRUE( -40.0f == agc.GetParameter( TAgc::PARAMETER_TARGET ) );
        agc.SetTargetInDecibels( 0.0f );
        ASSERT_TRUE( -6.0f == agc.GetParameter( TAgc::PARAMETER_TARGET ) );
        agc.SetMaxGainInDecibels( -3.0f );
        ASSERT_TRUE( 0.0f == agc.GetParameter( TAgc::PARAMETER_MAX_GAIN ) );
        agc.SetMaxGainInDecibels( 90.0f );
        ASSERT_TRUE( 40.0f == agc.GetParameter( TAgc::PARAMETER_MAX_GAIN ) );
        agc.SetMinGainInDecibels( -90.0f );
        ASSERT_TRUE( -20.0f == agc.GetParameter( TAgc::PARAMETER_MIN_GAIN ) );
        agc.SetMinGainInDecibels( 3.0f );
        ASSERT_TRUE( 0.0f == agc.GetParameter( TAgc::PARAMETER_MIN_GAIN ) );
        agc.SetRiseRateInDecibelsPerSecond( 0.0f );
        ASSERT_TRUE( 0.5f == agc.GetParameter( TAgc::PARAMETER_RISE_RATE ) );
        agc.SetRiseRateInDecibelsPerSecond( 99.0f );
        ASSERT_TRUE( 30.0f == agc.GetParameter( TAgc::PARAMETER_RISE_RATE ) );
        agc.SetFallRateInDecibelsPerSecond( 0.0f );
        ASSERT_TRUE( 1.0f == agc.GetParameter( TAgc::PARAMETER_FALL_RATE ) );
        agc.SetFallRateInDecibelsPerSecond( 999.0f );
        ASSERT_TRUE( 60.0f == agc.GetParameter( TAgc::PARAMETER_FALL_RATE ) );
        agc.SetLevelTimeInMs( 1.0f );
        ASSERT_TRUE( 100.0f == agc.GetParameter( TAgc::PARAMETER_LEVEL_TIME ) );
        agc.SetLevelTimeInMs( 99999.0f );
        ASSERT_TRUE( 3000.0f == agc.GetParameter( TAgc::PARAMETER_LEVEL_TIME ) );
        agc.SetFreeze( false );
        ASSERT_TRUE( 0.0f == agc.GetParameter( TAgc::PARAMETER_FREEZE ) );
        agc.SetCeilingInDecibels( -40.0f );
        ASSERT_TRUE( -12.0f == agc.GetParameter( TAgc::PARAMETER_CEILING ) );
        agc.SetCeilingInDecibels( 3.0f );
        ASSERT_TRUE( 0.0f == agc.GetParameter( TAgc::PARAMETER_CEILING ) );

        ASSERT_TRUE( 0 == agc.GetLatencyInFrames() && 0 == agc.GetTailInFrames() );
        ASSERT_TRUE( 0.0f == agc.GetGainInDecibels() && -120.0f == agc.GetSpeechLevelInDecibels() );

        // the voice detector settings are construction options
        CORE::CValueList noOptions;
        agc.GetConstructionOptions( noOptions );
        ASSERT_TRUE( 0 == noOptions.GetKeyCount() );
        CORE::CValueList options;
        options.Set( "vadOnsetDb", 12.0f );
        options.Set( "vadMinimumLevelDb", -55.0f );
        TAgc tuned( options );
        ASSERT_TRUE( 12.0f == tuned.GetVoiceActivitySettings().GetOnsetInDecibels() );
        ASSERT_TRUE( -55.0f == tuned.GetVoiceActivitySettings().GetMinimumLevelInDecibels() );
        CORE::CValueList reported;
        tuned.GetConstructionOptions( reported );
        ASSERT_TRUE( 2 == reported.GetKeyCount() );

        // built in, made by the registry (case does not matter)
        AUDIO::TAudioEffectPtr made = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "AUTOMATICGAIN", options );
        ASSERT_FALSE( made.IsNULL() );
        ASSERT_TRUE( made->GetEffectTypeName() == "automaticGain" && 8 == made->GetParameterCount() );
        ASSERT_TRUE( -55.0f == made.StaticCast< TAgc >()->GetVoiceActivitySettings().GetMinimumLevelInDecibels() );

        // unprepared the audio passes; a rate the voice detector cannot work at is refused
        TSamples audio = MakeNoise( 1000, 2, 3, 0.5f );
        const TSamples original( audio );
        agc.Process( &audio[ 0 ], 1000 );
        ASSERT_TRUE( IsSame( audio, original ) );
        TAgc low;
        ASSERT_FALSE( PrepareAgc( low, 4000, 1 ) );
        TAgc high;
        ASSERT_FALSE( PrepareAgc( high, 384000, 1 ) );
        ASSERT_TRUE( PrepareAgc( agc, 8000, 1 ) );
        ASSERT_TRUE( PrepareAgc( agc, 192000, 2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformConvergenceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Speech of -35, -25, -10 and -45 dBFS converges to the target (-20 dBFS, the -45 dBFS case to the maximum gain), at no more than the rise and fall rates, within the time they give" )
    try
    {
        const Float64 inputs[ 4 ] = { -35.0, -25.0, -10.0, -45.0 };
        for ( UInt32 c=0; c<4; ++c )
        {
            SMaterial material;
            MakeMaterial( material, 45.0, 60 + c, inputs[ c ], inputs[ c ] - 35.0 );
            TAgc agc;
            ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
            TSamples out( material.mixed );
            GUCEF::vector< Float32 > trace;
            RunTraced( agc, out, 1, 160, trace );

            const Float64 wanted = -20.0 - inputs[ c ];
            const Float64 expectedGain = wanted > 24.0 ? 24.0 : ( wanted < -12.0 ? -12.0 : wanted );
            Float64 finalGain = 0.0;
            for ( size_t i=trace.size()-1000; i<trace.size(); ++i )                  // the mean of the last 10 s: the gain follows the syllables a little
                finalGain += (Float64) trace[ i ];
            finalGain /= 1000.0;
            Float64 wander = 0.0;
            const Float64 settled = ArrivalTime( trace, 0.01, finalGain, 1.0, &wander );
            const Float64 level = ActiveLevel( out, material.speech.active, 30 * 16000, out.size() );
            const Float64 error = level - ( -20.0 + ( expectedGain - wanted ) );

            // the fastest the gain moved, per second
            Float64 fastestUp = 0.0, fastestDown = 0.0;
            for ( size_t i=1; i<trace.size(); ++i )
            {
                const Float64 slope = ( (Float64) trace[ i ] - (Float64) trace[ i - 1 ] ) / 0.01;
                if ( slope > fastestUp )
                    fastestUp = slope;
                if ( -slope > fastestDown )
                    fastestDown = -slope;
            }
            const Float64 activeFraction = FractionOfActive( material.speech.active );
            const Float64 distance = fabs( expectedGain );
            const Float64 rate = expectedGain >= 0.0 ? 6.0 : 20.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: speech at " << inputs[ c ] << " dBFS: gain " << finalGain << " dB (wanted " << wanted << " dB), reaches within 1 dB after " << settled << " s and wanders " << wander << " dB around it afterwards (at " << rate << " dB/s and " << 100.0 * activeFraction << " % active at least " << ( distance - 1.0 ) / rate << " s), output level " << level << " dBFS, error against the expected level " << error <<
                       " dB, fastest rise " << fastestUp << " dB/s, fall " << fastestDown << " dB/s" );
            ASSERT_TRUE( Near( finalGain, expectedGain, 1.0 ) );
            ASSERT_TRUE( fabs( error ) <= 1.0 );
            ASSERT_TRUE( fastestUp <= 6.0 * 1.02 && fastestDown <= 20.0 * 1.02 );
            ASSERT_TRUE( settled >= 0.0 && wander <= 2.5 );
            ASSERT_TRUE( settled >= ( distance - 1.0 ) / rate - 0.05 );                                  // not faster than the rate allows
            ASSERT_TRUE( settled <= ( distance - 1.0 ) / rate / activeFraction + 5.0 );                   // not slower than the speech gives it time for, plus the leading silence and the first estimate
            if ( c < 3 )
                ASSERT_TRUE( Near( level, -20.0, 1.0 ) );
            else
                ASSERT_TRUE( Near( level, -21.0, 1.0 ) );                                               // the maximum gain of 24 dB is short of 25 dB
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Level steps (-30, -15, -35 dBFS speech): the gain follows each step and the output returns to the target" )
    try
    {
        SMaterial material;
        MakeMaterial( material, 15.0, 71, -30.0, -65.0 );
        SMaterial second, third;
        MakeMaterial( second, 15.0, 72, -15.0, -50.0 );
        MakeMaterial( third, 20.0, 73, -35.0, -70.0 );
        Concatenate( material, second );
        Concatenate( material, third );

        TAgc agc;
        agc.SetRiseRateInDecibelsPerSecond( 12.0f );
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        TSamples out( material.mixed );
        GUCEF::vector< Float32 > trace;
        RunTraced( agc, out, 1, 160, trace );

        const Float64 inputs[ 3 ] = { -30.0, -15.0, -35.0 };
        const size_t starts[ 3 ] = { 0, 15 * 16000, 30 * 16000 };
        const size_t ends[ 3 ] = { 15 * 16000, 30 * 16000, 50 * 16000 };
        for ( UInt32 s=0; s<3; ++s )
        {
            const Float64 level = ActiveLevel( out, material.speech.active, ends[ s ] - 8 * 16000, ends[ s ] );
            // time after the step until the output level (input level plus gain) is first within 1 dB of the target
            Float64 within = -1.0;
            for ( size_t i=starts[ s ] / 160; i<ends[ s ] / 160; ++i )
            {
                if ( fabs( inputs[ s ] + (Float64) trace[ i ] + 20.0 ) <= 1.0 )
                {
                    within = (Float64) ( i - starts[ s ] / 160 ) * 0.01;
                    break;
                }
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: step to " << inputs[ s ] << " dBFS: output level at the end " << level << " dBFS, output level first within 1 dB of the target after " << within << " s" );
            ASSERT_TRUE( Near( level, -20.0, 1.0 ) );
            ASSERT_TRUE( within >= 0.0 && within < 12.0 );
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
PerformPauseAndLimitTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: The gain does not move in the pauses (the noise is not pumped up), without the freeze it does; noise alone leaves the audio untouched" )
    try
    {
        // loud speech, then quiet speech: after the step the gain has to rise
        SMaterial material;
        MakeMaterial( material, 15.0, 81, -15.0, -50.0, 1.0, 1.5 );
        SMaterial quiet;
        MakeMaterial( quiet, 25.0, 82, -40.0, -75.0, 1.0, 1.5 );
        Concatenate( material, quiet );
        TVS::TSpans windows;
        PauseWindows( material.speech, 0.45, 0.05, 15 * 16000, windows );
        ASSERT_TRUE( windows.size() >= 8 );

        Float64 spread[ 2 ] = { 0.0, 0.0 };
        Float64 risen[ 2 ] = { 0.0, 0.0 };
        for ( UInt32 variant=0; variant<2; ++variant )
        {
            TAgc agc;
            agc.SetFreeze( 0 == variant );
            ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
            TSamples out( material.mixed );
            GUCEF::vector< Float32 > trace;
            RunTraced( agc, out, 1, 160, trace );
            for ( size_t w=0; w<windows.size(); ++w )
            {
                Float32 low = 1000.0f, high = -1000.0f;
                for ( UInt64 i=windows[ w ].start / 160; i<windows[ w ].End() / 160; ++i )
                {
                    if ( trace[ (size_t) i ] < low )
                        low = trace[ (size_t) i ];
                    if ( trace[ (size_t) i ] > high )
                        high = trace[ (size_t) i ];
                }
                const Float64 change = (Float64) trace[ (size_t) ( windows[ w ].End() / 160 - 1 ) ] - (Float64) trace[ (size_t) ( windows[ w ].start / 160 ) ];
                if ( (Float64) ( high - low ) > spread[ variant ] )
                    spread[ variant ] = (Float64) ( high - low );
                risen[ variant ] += change;
            }
            if ( 0 == variant )
            {
                // the output in the pauses is the input in the pauses times the (constant) gain of that pause
                Float64 worst = 0.0;
                for ( size_t w=0; w<windows.size(); ++w )
                {
                    TVS::TSpans one;
                    one.push_back( windows[ w ] );
                    const Float64 difference = 10.0 * log10( MeanSquareIn( out, one ) / MeanSquareIn( material.mixed, one ) );
                    const Float64 deviation = fabs( difference - (Float64) trace[ (size_t) ( windows[ w ].start / 160 ) ] );
                    if ( deviation > worst )
                        worst = deviation;
                }
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: noise in the pauses against the gain of the pause: worst deviation " << worst << " dB, final gain " << trace.back() << " dB" );
                ASSERT_TRUE( worst <= 0.3 );
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: gain change within the pauses after the step: with freeze at most " << spread[ 0 ] << " dB in one pause, " << risen[ 0 ] << " dB in all; without freeze " << spread[ 1 ] << " dB and " << risen[ 1 ] << " dB" );
        ASSERT_TRUE( spread[ 0 ] <= 0.02 );
        ASSERT_TRUE( risen[ 1 ] > 3.0 );

        // noise alone: the voice detector never triggers, the gain stays at 0 dB, the audio is not changed
        TSamples noise = TVS::MakeWhiteNoise( 16000 * 20, TVS::FromDecibels( -45.0 ), 9 );
        const TSamples noiseIn( noise );
        TAgc agc;
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        GUCEF::vector< Float32 > trace;
        RunTraced( agc, noise, 1, 160, trace );
        Float32 largest = 0.0f;
        for ( size_t i=0; i<trace.size(); ++i )
            if ( fabsf( trace[ i ] ) > largest )
                largest = fabsf( trace[ i ] );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: 20 s of noise at -45 dBFS: largest gain " << largest << " dB, speech level " << agc.GetSpeechLevelInDecibels() << " dBFS" );
        ASSERT_TRUE( 0.0f == largest );
        ASSERT_TRUE( IsSame( noise, noiseIn ) );
        ASSERT_TRUE( -120.0f == agc.GetSpeechLevelInDecibels() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Maximum and minimum gain are respected, a limit that moves in pulls the gain in at the rates, even in a pause" )
    try
    {
        // very quiet speech: the gain stops at the maximum
        const Float32 maxima[ 3 ] = { 24.0f, 12.0f, 3.0f };
        for ( UInt32 m=0; m<3; ++m )
        {
            SMaterial material;
            MakeMaterial( material, 40.0, 91, -55.0, -90.0 );
            TAgc agc;
            agc.SetMaxGainInDecibels( maxima[ m ] );
            agc.SetRiseRateInDecibelsPerSecond( 30.0f );
            ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
            TSamples out( material.mixed );
            GUCEF::vector< Float32 > trace;
            RunTraced( agc, out, 1, 160, trace );
            Float32 highest = -100.0f;
            for ( size_t i=0; i<trace.size(); ++i )
                if ( trace[ i ] > highest )
                    highest = trace[ i ];
            ASSERT_TRUE( highest <= maxima[ m ] );
            ASSERT_TRUE( Near( (Float64) trace.back(), (Float64) maxima[ m ], 0.01 ) );
        }

        // loud speech: the gain stops at the minimum (the target would ask for -14 dB)
        SMaterial loud;
        MakeMaterial( loud, 30.0, 92, -6.0, -45.0 );
        TAgc agc;
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        TSamples loudOut( loud.mixed );
        GUCEF::vector< Float32 > trace;
        RunTraced( agc, loudOut, 1, 160, trace );
        Float32 lowest = 100.0f;
        for ( size_t i=0; i<trace.size(); ++i )
            if ( trace[ i ] < lowest )
                lowest = trace[ i ];
        ASSERT_TRUE( lowest >= -12.0f );
        ASSERT_TRUE( Near( (Float64) trace.back(), -12.0, 0.01 ) );

        // the maximum lowered while the gain is 24 dB: it falls to the new maximum at the fall rate, in silence as well
        SMaterial quiet;
        MakeMaterial( quiet, 25.0, 93, -50.0, -85.0 );
        TAgc pulled;
        pulled.SetRiseRateInDecibelsPerSecond( 30.0f );
        ASSERT_TRUE( PrepareAgc( pulled, 16000, 1 ) );
        TSamples quietOut( quiet.mixed );
        GUCEF::vector< Float32 > quietTrace;
        RunTraced( pulled, quietOut, 1, 160, quietTrace );
        ASSERT_TRUE( Near( (Float64) quietTrace.back(), 24.0, 0.01 ) );
        pulled.SetMaxGainInDecibels( 6.0f );
        TSamples silence( 16000 * 3, 0.0f );
        GUCEF::vector< Float32 > silenceTrace;
        RunTraced( pulled, silence, 1, 160, silenceTrace );
        // 18 dB at 20 dB/s: 0.9 s
        const Float64 arrival = ArrivalTime( silenceTrace, 0.01, 6.0, 0.01 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: maximum gain lowered from 24 to 6 dB: reached after " << arrival << " s (expected 0.9 s)" );
        ASSERT_TRUE( Near( arrival, 0.9, 0.1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: The output never exceeds the ceiling, also for a sudden loud burst at a high gain; below the ceiling the stage changes nothing" )
    try
    {
        const Float32 ceilings[ 3 ] = { -1.0f, 0.0f, -6.0f };
        for ( UInt32 c=0; c<3; ++c )
        {
            // speech at -12 dBFS with a target of -6 dBFS: the gain of 6 dB puts the peaks above the ceiling
            SMaterial material;
            MakeMaterial( material, 30.0, 95, -12.0, -50.0 );
            TAgc agc;
            agc.SetTargetInDecibels( -6.0f );
            agc.SetRiseRateInDecibelsPerSecond( 30.0f );
            agc.SetCeilingInDecibels( ceilings[ c ] );
            ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
            const Float32 limit = (Float32) LinearOf( (Float64) ceilings[ c ] );
            TSamples out( material.mixed );
            GUCEF::vector< Float32 > trace;
            RunTraced( agc, out, 1, 160, trace );
            const Float32 inPeak = PeakOf( material.mixed );
            const Float32 outPeak = PeakOf( out );
            const Float64 wouldBe = (Float64) inPeak * LinearOf( (Float64) trace.back() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: ceiling " << ceilings[ c ] << " dBFS: input peak " << inPeak << ", gain " << trace.back() << " dB would give " << wouldBe << ", output peak " << outPeak );
            ASSERT_TRUE( wouldBe > 1.05 * (Float64) limit );                                    // the ceiling had work to do
            ASSERT_TRUE( outPeak <= limit );
            ASSERT_TRUE( outPeak > 0.9f * limit );
            ASSERT_TRUE( AllFiniteAndBelow( out, limit ) );
        }

        // the sudden burst: converged at 24 dB, then a full scale square wave of 10 ms
        SMaterial quiet;
        MakeMaterial( quiet, 30.0, 96, -50.0, -85.0 );
        TAgc agc;
        agc.SetRiseRateInDecibelsPerSecond( 30.0f );
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        TSamples audio( quiet.mixed );
        for ( size_t f=0; f<160; ++f )
            audio.push_back( 0 == ( f / 8 ) % 2 ? 1.0f : -1.0f );
        TSamples tail = TVS::MakeWhiteNoise( 16000, TVS::FromDecibels( -85.0 ), 4 );
        audio.insert( audio.end(), tail.begin(), tail.end() );
        TSamples out( audio );
        GUCEF::vector< Float32 > trace;
        RunTraced( agc, out, 1, 160, trace );
        const size_t burst = quiet.mixed.size();
        ASSERT_TRUE( PeakOf( out, burst, burst + 160 ) <= (Float32) LinearOf( -1.0 ) );
        ASSERT_TRUE( PeakOf( out, burst, burst + 160 ) > 0.85f );

        // quiet audio and noise: nothing to limit, so the audio is exactly the input times the gain
        SMaterial calm;
        MakeMaterial( calm, 10.0, 97, -45.0, -80.0 );
        TAgc calmAgc;
        ASSERT_TRUE( PrepareAgc( calmAgc, 16000, 1 ) );
        TSamples calmOut( calm.mixed );
        Run( calmAgc, calmOut, 1, SubBlock );
        TAgc withoutCeiling;
        withoutCeiling.SetCeilingInDecibels( 0.0f );
        ASSERT_TRUE( PrepareAgc( withoutCeiling, 16000, 1 ) );
        TSamples other( calm.mixed );
        Run( withoutCeiling, other, 1, SubBlock );
        ASSERT_TRUE( IsSame( calmOut, other ) );
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

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: The output does not depend on the block size (bit for bit): mono speech, stereo speech at 48 kHz, and loud speech with the ceiling at work" )
    try
    {
        SMaterial quiet, loud, wide;
        MakeMaterial( quiet, 25.0, 101, -38.0, -70.0 );
        MakeMaterial( loud, 25.0, 102, -12.0, -50.0 );
        MakeMaterial( wide, 10.0, 103, -35.0, -65.0, 0.5, 1.5, 48000 );
        TSamples stereo( wide.mixed.size() * 2 );
        for ( size_t f=0; f<wide.mixed.size(); ++f )
        {
            stereo[ f * 2 ] = wide.mixed[ f ];
            stereo[ f * 2 + 1 ] = 0.6f * wide.mixed[ f ];
        }

        const UInt32 blocks[ 6 ] = { 1, 7, 33, 100, 1024, 4096 };
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            const UInt32 rate = 2 == variant ? 48000 : 16000;
            const UInt32 channels = 2 == variant ? 2 : 1;
            const TSamples& input = 0 == variant ? quiet.mixed : ( 1 == variant ? loud.mixed : stereo );

            TAgc reference;
            reference.SetRiseRateInDecibelsPerSecond( 20.0f );
            if ( 1 == variant )
                reference.SetTargetInDecibels( -6.0f );
            ASSERT_TRUE( PrepareAgc( reference, rate, channels ) );
            TSamples expected( input );
            Run( reference, expected, channels, SubBlock );
            ASSERT_FALSE( IsSame( expected, input ) );
            ASSERT_TRUE( reference.GetGainInDecibels() != 0.0f );

            for ( UInt32 b=0; b<6; ++b )
            {
                TAgc agc;
                agc.SetRiseRateInDecibelsPerSecond( 20.0f );
                if ( 1 == variant )
                    agc.SetTargetInDecibels( -6.0f );
                ASSERT_TRUE( PrepareAgc( agc, rate, channels ) );
                TSamples actual( input );
                Run( agc, actual, channels, blocks[ b ] );
                ASSERT_TRUE( IsSame( actual, expected ) );
                ASSERT_TRUE( agc.GetGainInDecibels() == reference.GetGainInDecibels() );
            }
            TAgc agc;
            agc.SetRiseRateInDecibelsPerSecond( 20.0f );
            if ( 1 == variant )
                agc.SetTargetInDecibels( -6.0f );
            ASSERT_TRUE( PrepareAgc( agc, rate, channels ) );
            TSamples randomOut( input );
            RunInRandomBlocks( agc, randomOut, channels, 500, 55 );
            ASSERT_TRUE( IsSame( randomOut, expected ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Settings saved and loaded: parameters round trip by name, the voice detector settings are construction options; reset forgets the level" )
    try
    {
        CORE::CValueList options;
        options.Set( "vadHangoverMs", 350.0f );
        TAgc original( options );
        original.SetTargetInDecibels( -17.0f );
        original.SetMaxGainInDecibels( 31.0f );
        original.SetMinGainInDecibels( -9.0f );
        original.SetRiseRateInDecibelsPerSecond( 3.5f );
        original.SetFallRateInDecibelsPerSecond( 33.0f );
        original.SetLevelTimeInMs( 750.0f );
        original.SetFreeze( false );
        original.SetCeilingInDecibels( -3.0f );
        CORE::CDataNode config;
        ASSERT_TRUE( original.SaveConfig( config ) );
        const CORE::CDataNode* parameters = config.FindChild( "Parameters" );
        ASSERT_TRUE( GUCEF_NULL != parameters );
        ASSERT_TRUE( parameters->GetAttributeValue( "freeze" ).AsString() == "off" );
        const CORE::CDataNode* optionsNode = config.FindChild( "Options" );
        ASSERT_TRUE( GUCEF_NULL != optionsNode && 350.0f == optionsNode->GetAttributeValue( "vadHangoverMs" ).AsFloat32( 0.0f ) );

        CORE::CValueList restored;
        original.GetConstructionOptions( restored );
        TAgc loaded( restored );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        for ( UInt32 p=0; p<TAgc::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( loaded.GetParameter( p ) == original.GetParameter( p ) );
        ASSERT_TRUE( 350.0f == loaded.GetVoiceActivitySettings().GetHangoverInMs() );

        SMaterial material;
        MakeMaterial( material, 12.0, 111, -35.0, -70.0 );
        TAgc agc;
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        TSamples out( material.mixed );
        Run( agc, out, 1, 160 );
        ASSERT_TRUE( agc.GetGainInDecibels() > 1.0f && agc.GetSpeechLevelInDecibels() > -60.0f );
        ResetRealtime( agc );
        ProcessRealtime( agc, &out[ 0 ], 32 );
        ASSERT_TRUE( 0.0f == agc.GetGainInDecibels() && -120.0f == agc.GetSpeechLevelInDecibels() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Real time safety: processing speech with parameter changes and a reset allocates, locks and blocks nowhere (RealtimeSanitizer builds)" )
    try
    {
        SMaterial material;
        MakeMaterial( material, 14.0, 121, -30.0, -60.0 );
        TAgc agc;
        ASSERT_TRUE( PrepareAgc( agc, 16000, 1 ) );
        TSamples audio( material.mixed );
        size_t at = 0;
        Float32 highest = 0.0f;
        for ( UInt32 round=0; round<2000 && at + SubBlock * 8 < audio.size(); ++round )
        {
            agc.SetParameter( TAgc::PARAMETER_TARGET, -30.0f + (Float32) ( round % 20 ) );
            agc.SetParameter( TAgc::PARAMETER_FREEZE, (Float32) ( ( round / 50 ) % 2 ) );
            agc.SetParameter( TAgc::PARAMETER_RISE_RATE, 1.0f + (Float32) ( round % 10 ) * 3.0f );
            agc.SetParameter( TAgc::PARAMETER_MAX_GAIN, 10.0f + (Float32) ( round % 30 ) );
            if ( 888 == round )
                ResetRealtime( agc );
            const UInt32 frames = SubBlock * ( 1 + round % 7 ) + round % 5;
            ProcessRealtime( agc, &audio[ at ], frames );
            at += frames;
            if ( agc.GetGainInDecibels() > highest )
                highest = agc.GetGainInDecibels();
        }
        ASSERT_TRUE( highest > 1.0f );
        ASSERT_TRUE( AllFiniteAndBelow( audio, 1.0f ) );
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

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: In a bus layout: parameters and voice detector options round trip through the layout and the engine" )
    try
    {
        AUDIO::CAudioBusLayout layout;
        AUDIO::CAudioBusLayout::SBus& voice = layout.AddBus( "voice" );
        CORE::CValueList options;
        options.Set( "vadOnsetDb", 11.0f );
        AUDIO::TAudioEffectPtr described = AUDIO::CAudioGlobal::Instance()->GetAudioEffectRegistry().CreateEffect( "automaticGain", options );
        ASSERT_FALSE( described.IsNULL() );
        described->SetParameter( TAgc::PARAMETER_TARGET, -24.0f );
        described->SetParameter( TAgc::PARAMETER_MAX_GAIN, 18.0f );
        described->SetParameter( TAgc::PARAMETER_FREEZE, 0.0f );
        ASSERT_TRUE( described->SaveConfig( voice.AddEffect( 0, "automaticGain" ).config ) );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
        ASSERT_FALSE( applied.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( engine, applied ) );
        AUDIO::CAudioBus* bus = engine.FindBus( "voice" );
        ASSERT_TRUE( GUCEF_NULL != bus );
        AUDIO::TAudioEffectPtr placed = engine.GetBusEffect( bus, 0 );
        ASSERT_FALSE( placed.IsNULL() );
        ASSERT_TRUE( -24.0f == placed->GetParameter( TAgc::PARAMETER_TARGET ) );
        ASSERT_TRUE( 18.0f == placed->GetParameter( TAgc::PARAMETER_MAX_GAIN ) );
        ASSERT_TRUE( 0.0f == placed->GetParameter( TAgc::PARAMETER_FREEZE ) );
        ASSERT_TRUE( 11.0f == placed.StaticCast< TAgc >()->GetVoiceActivitySettings().GetOnsetInDecibels() );

        // a constant has no voice in it: it passes
        engine.Play( MakeConstantClip( 0.3f ), bus );
        RenderBlocks( engine, 100, 256 );
        ASSERT_TRUE( Near( (Float64) PeakOfBus( bus ), 0.3, 0.001 ) );

        AUDIO::CAudioBusLayout captured;
        ASSERT_TRUE( engine.GetBusLayout( captured ) );
        engine.Shutdown();

        AUDIO::CAudioEngine fresh;
        ASSERT_TRUE( fresh.Initialize( NullParams() ) );
        CORE::CFutureResult appliedAgain = fresh.ApplyBusLayout( captured );
        ASSERT_FALSE( appliedAgain.HasNoFuture() );
        ASSERT_TRUE( RenderUntilReady( fresh, appliedAgain ) );
        AUDIO::TAudioEffectPtr rebuilt = fresh.GetBusEffect( fresh.FindBus( "voice" ), 0 );
        ASSERT_FALSE( rebuilt.IsNULL() );
        ASSERT_TRUE( rebuilt->GetEffectTypeName() == "automaticGain" );
        for ( UInt32 p=0; p<TAgc::PARAMETER_COUNT; ++p )
            ASSERT_TRUE( rebuilt->GetParameter( p ) == placed->GetParameter( p ) );
        ASSERT_TRUE( 11.0f == rebuilt.StaticCast< TAgc >()->GetVoiceActivitySettings().GetOnsetInDecibels() );
        fresh.Shutdown();
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

    GUCEF_TESTFW_TESTCASE( "AudioAutomaticGain: Capture chain on a null capture device (high pass, noise gate, automatic gain, voice detector): speech comes through at the target level, the hum and noise are lowered" )
    try
    {
        // speech at -30 dBFS, 50 Hz hum at -40 dBFS and white noise at -50 dBFS in the pauses and under the speech
        SMaterial material;
        MakeMaterial( material, 45.0, 131, -30.0, -50.0, 1.0, 1.5 );
        TSamples hum = TVS::MakeHum( material.mixed.size(), 16000, 50.0, 6, TVS::FromDecibels( -40.0 ) );
        TVS::MixInto( material.mixed, hum );

        SChainRun processed, plain;
        ASSERT_TRUE( RunChain( material.mixed, true, processed ) );
        ASSERT_TRUE( RunChain( material.mixed, false, plain ) );
        ASSERT_TRUE( processed.recorded.size() >= material.mixed.size() && plain.recorded.size() >= material.mixed.size() );
        processed.recorded.resize( material.mixed.size() );
        plain.recorded.resize( material.mixed.size() );

        // from 20 s on: the level of the speech, the noise in the pauses, the signal to noise ratio
        const size_t from = 20 * 16000;
        TVS::TSpans windows;
        PauseWindows( material.speech, 0.5, 0.05, from, windows );
        ASSERT_TRUE( windows.size() >= 5 );
        const Float64 speechOut = ActiveLevel( processed.recorded, material.speech.active, from, processed.recorded.size() );
        const Float64 speechPlain = ActiveLevel( plain.recorded, material.speech.active, from, plain.recorded.size() );
        const Float64 noiseOut = 10.0 * log10( MeanSquareIn( processed.recorded, windows ) );
        const Float64 noisePlain = 10.0 * log10( MeanSquareIn( plain.recorded, windows ) );
        const Float64 snrOut = speechOut - noiseOut;
        const Float64 snrPlain = speechPlain - noisePlain;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: capture chain: speech " << speechPlain << " -> " << speechOut << " dBFS, noise in the pauses " << noisePlain << " -> " << noiseOut << " dBFS, signal to noise ratio " <<
                   snrPlain << " -> " << snrOut << " dB (gate gain " << processed.gateGainAtEnd << " dB, automatic gain " << processed.agcGainAtEnd << " dB)" );
        ASSERT_TRUE( Near( speechPlain, -30.0, 0.5 ) );                                  // the empty path leaves the speech alone (the noise adds a little)
        ASSERT_TRUE( Near( speechOut, -20.0, 1.5 ) );                                    // normalised
        ASSERT_TRUE( snrOut - snrPlain >= 25.0 );                                        // the noise is lowered

        // the voice detector of the path sees the talkspurts and not the pauses
        size_t insideBlocks = 0, insideActive = 0, pauseBlocks = 0, pauseActive = 0;
        for ( size_t b=0; b<processed.blocks.size(); ++b )
        {
            const UInt64 end = processed.blocks[ b ].firstFrame + processed.blocks[ b ].frames;
            if ( end < (UInt64) from )
                continue;
            for ( size_t t=0; t<material.speech.talkspurts.size(); ++t )
            {
                const TVS::SSpan& spurt = material.speech.talkspurts[ t ];
                if ( end > spurt.start + 4800 && end < spurt.End() - 4800 )
                {
                    ++insideBlocks;
                    insideActive += processed.blocks[ b ].voiceActive ? 1 : 0;
                }
            }
            for ( size_t w=0; w<windows.size(); ++w )
            {
                if ( end > windows[ w ].start && end < windows[ w ].End() )
                {
                    ++pauseBlocks;
                    pauseActive += processed.blocks[ b ].voiceActive ? 1 : 0;
                }
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioAutomaticGain: capture chain voice activity: " << insideActive << " of " << insideBlocks << " blocks inside talkspurts, " << pauseActive << " of " << pauseBlocks << " blocks in the pauses" );
        ASSERT_TRUE( insideBlocks > 100 && pauseBlocks > 100 );
        ASSERT_TRUE( insideActive * 10 >= insideBlocks * 9 );
        ASSERT_TRUE( pauseActive * 10 <= pauseBlocks );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAudioAutomaticGainTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioAutomaticGain TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.AutomaticGain" );

    PerformParameterTests();
    PerformConvergenceTests();
    PerformPauseAndLimitTests();
    PerformInvarianceAndRealtimeTests();
    PerformEngineTests();
    PerformChainTests();
}

/*-------------------------------------------------------------------------*/
