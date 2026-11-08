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
#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CDUCKEREFFECT_H
#include "gucefAUDIO_CDuckerEffect.h"
#define GUCEF_AUDIO_CDUCKEREFFECT_H
#endif /* GUCEF_AUDIO_CDUCKEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CFILTEREFFECT_H
#include "gucefAUDIO_CFilterEffect.h"
#define GUCEF_AUDIO_CFILTEREFFECT_H
#endif /* GUCEF_AUDIO_CFILTEREFFECT_H ? */

#ifndef GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H
#include "gucefAUDIO_CNoiseSuppressorEffect.h"
#define GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H
#endif /* GUCEF_AUDIO_CNOISESUPPRESSOREFFECT_H ? */

#include "TestAudioCodecSupport.h"
#include "TestAudioCodecMp3Support.h"
#include "TestAudioVoiceCodecSupport.h"
#include "TestAudioIntercomScenario.h"
#include "TestAudioEchoSimulation.h"
#include "TestAudioBenchmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#define SKIP_UNLESS_BENCH() \
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_BENCH" ) != "1", "GUCEF_TEST_AUDIO_BENCH is not 1" )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace CodecTest;
using namespace CodecTest::Mp3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 BlockFrames = 256;
const UInt32 MixRate = 48000;
const Float64 PeriodInUs = 1000000.0 * BlockFrames / MixRate;       // 5333 us

typedef GUCEF::vector< Float32 > TSamples;
typedef GUCEF::vector< Float64 > TTimes;

/*-------------------------------------------------------------------------*/

struct SStats
{
    Float64 minUs;
    Float64 medianUs;
    Float64 p95Us;
    Float64 maxUs;
    SStats( void ) : minUs( 0.0 ), medianUs( 0.0 ), p95Us( 0.0 ), maxUs( 0.0 ) {}
};

int
CompareTimes( const void* a, const void* b )
{
    const Float64 x = *static_cast< const Float64* >( a );
    const Float64 y = *static_cast< const Float64* >( b );
    return x < y ? -1 : ( x > y ? 1 : 0 );
}

SStats
Summarize( TTimes& times )
{
    SStats stats;
    if ( times.empty() )
        return stats;
    qsort( &times[ 0 ], times.size(), sizeof( Float64 ), CompareTimes );
    stats.minUs = times[ 0 ];
    stats.medianUs = times[ times.size() / 2 ];
    stats.p95Us = times[ ( times.size() * 95 ) / 100 < times.size() ? ( times.size() * 95 ) / 100 : times.size() - 1 ];
    stats.maxUs = times[ times.size() - 1 ];
    return stats;
}

/** The value with one decimal, or with two below 10 (the log would show six decimals for a number) */
CORE::CString
Round( Float64 value )
{
    const UInt32 decimals = value < 10.0 ? 100 : 10;
    const Int64 scaled = (Int64) floor( value * decimals + 0.5 );
    CORE::CString text = CORE::Int64ToString( scaled / decimals ) + ".";
    const Int64 fraction = scaled % decimals;
    if ( 100 == decimals && fraction < 10 )
        text += "0";
    return text + CORE::Int64ToString( fraction );
}

/** Microseconds from a tick count difference */
Float64
Microseconds( UInt64 ticks )
{
    return MT::PrecisionTimerTicksToMs( ticks ) * 1000.0;
}

/*-------------------------------------------------------------------------*/

void
Report( const char* what, const SStats& stats, Float64 budgetUs, const char* budgetText )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: " << what << ": median " << Round( stats.medianUs ) << " us, min " << Round( stats.minUs ) <<
                                      " us, 95th percentile " << Round( stats.p95Us ) << " us, max " << Round( stats.maxUs ) << " us, median is " <<
                                      Round( 100.0 * stats.medianUs / budgetUs ) << " % of " << budgetText );
}

/*-------------------------------------------------------------------------*/

/** A piece of music-like audio: a few tones, a slow vibrato and a little noise, a little different on each channel */
AUDIO::CAudioBuffer
MakeMusicLike( UInt32 rate, UInt32 frames, UInt32 seed )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( rate, 2, AUDIO::SAMPLETYPE_INT16 ), frames );
    CRandom random( seed );
    const Float64 twoPi = 2.0 * 3.14159265358979323846;
    for ( UInt32 f=0; f<frames; ++f )
    {
        const Float64 t = (Float64) f / rate;
        for ( UInt32 c=0; c<2; ++c )
        {
            const Float64 shift = 0.3 * c;
            Float64 v = 0.12 * sin( twoPi * 220.0 * t + shift ) + 0.09 * sin( twoPi * 331.0 * t + 2.0 * shift ) + 0.07 * sin( twoPi * 880.0 * t ) +
                        0.05 * sin( twoPi * 1760.0 * t * ( 1.0 + 0.01 * sin( twoPi * 5.0 * t ) ) );
            v += 0.02 * random.NextFloat();
            audio.SetSampleFromFloat( f, c, (Float32) v );
        }
    }
    return audio;
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioEngineParams
BenchParams( bool limiterOn, UInt32 rate = MixRate, UInt32 channels = 2 )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetMaxVoices( 256 );
    params.SetLimiterEnabled( limiterOn );
    params.GetDeviceParams().SetBufferSizeInFrames( BlockFrames );
    params.GetDeviceParams().SetPreferredFormat( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ) );
    return params;
}

/*-------------------------------------------------------------------------*/

AUDIO::TAudioClipPtr
MakeNoiseClip( UInt32 rate, UInt32 channels, UInt32 frames, UInt32 seed )
{
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( MakeNoise( AUDIO::CAudioFormat( rate, channels, AUDIO::SAMPLETYPE_FLOAT32 ), frames, 0.5f, seed ) ) );
}

/*-------------------------------------------------------------------------*/

/** Renders blocks in batches and gives the time per block of each batch: the clock has a resolution of one microsecond */
bool
MeasureRender( AUDIO::CAudioEngine& engine, UInt32 blocksPerBatch, UInt32 batches, SStats& stats )
{
    AUDIO::TAudioDevicePtr device = engine.GetDevice();
    AUDIO::CNullAudioDevice* null = device.IsNULL() ? GUCEF_NULL : device->AsNullDevice();
    if ( GUCEF_NULL == null )
        return false;
    for ( UInt32 i=0; i<300; ++i )
        null->Render( BlockFrames );
    engine.Update();

    TTimes times;
    for ( UInt32 b=0; b<batches; ++b )
    {
        const UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<blocksPerBatch; ++i )
            null->Render( BlockFrames );
        times.push_back( Microseconds( MT::PrecisionTickCount() - start ) / blocksPerBatch );
        engine.Update();
    }
    stats = Summarize( times );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Starts 'count' looping voices of noise on the bus, the clips with 'clipRate' and 'clipChannels' */
bool
StartVoices( AUDIO::CAudioEngine& engine, AUDIO::CAudioBus** buses, UInt32 busCount, UInt32 count, UInt32 clipRate, UInt32 clipChannels )
{
    AUDIO::TAudioClipPtr clip = MakeNoiseClip( clipRate, clipChannels, clipRate, 7 );
    for ( UInt32 i=0; i<count; ++i )
    {
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clip, busCount > 0 ? buses[ i % busCount ] : GUCEF_NULL );
        if ( voice.IsNULL() )
            return false;
        voice->SetLoopCount( 0 );
        voice->SetVolume( 0.05f );
        voice->SetPan( -1.0f + 2.0f * (Float32) ( i % 17 ) / 16.0f );
        if ( engine.Start( voice ).HasNoFuture() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
MixBenchmark( const char* what, UInt32 voices, bool limiterOn, UInt32 clipRate, UInt32 clipChannels, const char* budgetText, Float64 budgetUs )
{
    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( BenchParams( limiterOn ) ) || !StartVoices( engine, GUCEF_NULL, 0, voices, clipRate, clipChannels ) )
        return false;
    SStats stats;
    if ( !MeasureRender( engine, 20, 100, stats ) || engine.GetActiveVoiceCount() != voices )
        return false;
    Report( what, stats, budgetUs, budgetText );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
EffectBenchmark( AUDIO::CAudioEngine& engine, const char* type, const CORE::CValueList& options, UInt32 channels, const char* what, Float64 budgetUs, const char* parameterNames[], Float32 parameterValues[], bool duckerManualKey = false )
{
    AUDIO::TAudioEffectPtr effect = engine.CreateEffect( type, options );
    if ( effect.IsNULL() || !effect->IsPrepared() )
        return false;
    for ( UInt32 p=0; GUCEF_NULL != parameterNames && GUCEF_NULL != parameterNames[ p ]; ++p )
    {
        UInt32 id = 0;
        if ( !effect->TryGetParameterId( parameterNames[ p ], id ) )
            return false;
        effect->SetParameter( id, parameterValues[ p ] );
    }
    if ( duckerManualKey )
        effect.StaticCast< AUDIO::CDuckerEffect >()->SetManualKey( true );

    const UInt32 samples = BlockFrames * channels;
    TSamples input( samples );
    CRandom random( 5 );
    for ( UInt32 i=0; i<samples; ++i )
        input[ i ] = 0.3f * random.NextFloat();
    TSamples work( input );

    const UInt32 blocksPerBatch = 400;
    for ( UInt32 i=0; i<blocksPerBatch; ++i )
    {
        memcpy( &work[ 0 ], &input[ 0 ], samples * sizeof( Float32 ) );
        effect->Process( &work[ 0 ], BlockFrames );
    }
    TTimes times;
    for ( UInt32 b=0; b<60; ++b )
    {
        const UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<blocksPerBatch; ++i )
        {
            memcpy( &work[ 0 ], &input[ 0 ], samples * sizeof( Float32 ) );
            effect->Process( &work[ 0 ], BlockFrames );
        }
        times.push_back( Microseconds( MT::PrecisionTickCount() - start ) / blocksPerBatch );
    }
    Report( what, Summarize( times ), budgetUs, ( CORE::CString( "the target of " ) + CORE::Int64ToString( (Int64) budgetUs ) + " us" ).C_String() );
    return true;
}

/*-------------------------------------------------------------------------*/

AUDIO::CAudioBusLayout
MakeLayout( bool variant )
{
    AUDIO::CAudioBusLayout layout;
    AUDIO::CAudioBusLayout::SBus& master = layout.AddBus( "master" );
    master.AddEffect( 3, "limiter" ).SetParameter( "ceiling", variant ? -1.0f : -0.3f );
    AUDIO::CAudioBusLayout::SBus& music = layout.AddBus( "music", "", variant ? -6.0f : -3.0f );
    music.AddEffect( 0, "equalizer" ).SetParameter( "band2Gain", variant ? 4.0f : -2.0f );
    AUDIO::CAudioBusLayout::SBus& sfx = layout.AddBus( "sfx", "", variant ? -2.0f : 0.0f );
    sfx.AddEffect( 0, "compressor" ).SetParameter( "ratio", variant ? 6.0f : 3.0f );
    sfx.AddEffect( 1, variant ? "filter" : "equalizer" );
    sfx.AddSend( "echo", variant ? -9.0f : -12.0f, false );
    AUDIO::CAudioBusLayout::SBus& voice = layout.AddBus( "voice", "", 0.0f );
    voice.AddEffect( 0, "compressor" ).SetParameter( "threshold", variant ? -24.0f : -18.0f );
    layout.AddBus( "ui", "sfx", -4.0f, variant );
    AUDIO::CAudioBusLayout::SBus& echo = layout.AddBus( "echo", "", -3.0f );
    echo.AddEffect( 0, "delay" ).SetParameter( "time", variant ? 250.0f : 400.0f );
    return layout;
}

/** Calls ApplyBusLayout and renders until the audio thread has taken the commands; the time of the call and of the whole */
bool
ApplyAndWait( AUDIO::CAudioEngine& engine, const AUDIO::CAudioBusLayout& layout, Float64& callUs, Float64& totalUs )
{
    const UInt64 start = MT::PrecisionTickCount();
    CORE::CFutureResult applied = engine.ApplyBusLayout( layout );
    callUs = Microseconds( MT::PrecisionTickCount() - start );
    if ( applied.HasNoFuture() )
        return false;
    AUDIO::CNullAudioDevice* null = engine.GetDevice()->AsNullDevice();
    for ( UInt32 i=0; i<200 && !applied.IsReady(); ++i )
    {
        null->Render( BlockFrames );
        engine.Update();
    }
    totalUs = Microseconds( MT::PrecisionTickCount() - start );
    return applied.IsReady() && !applied.HasNoFuture();
}


/** Time per 10 ms of mono audio for a resampler at the given rates, in batches of 100 blocks */
bool
ResamplerBenchmark( AUDIO::IAudioResampler& resampler, Float64 inputRate, Float64 outputRate, SStats& stats )
{
    if ( !resampler.Configure( 1, inputRate, outputRate ) )
        return false;
    const UInt32 inFrames = (UInt32) ( inputRate / 100.0 );
    const UInt32 outCapacity = (UInt32) ( 2.0 * outputRate / 100.0 ) + 64;
    TSamples input( inFrames );
    CRandom random( 11 );
    for ( UInt32 i=0; i<inFrames; ++i )
        input[ i ] = 0.5f * random.NextFloat();
    TSamples output( outCapacity );

    UInt32 used = 0;
    UInt32 made = 0;
    for ( UInt32 i=0; i<100; ++i )
        resampler.Process( &input[ 0 ], inFrames, used, &output[ 0 ], outCapacity, made );

    TTimes times;
    for ( UInt32 b=0; b<50; ++b )
    {
        const UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<100; ++i )
            resampler.Process( &input[ 0 ], inFrames, used, &output[ 0 ], outCapacity, made );
        times.push_back( Microseconds( MT::PrecisionTickCount() - start ) / 100.0 );
    }
    stats = Summarize( times );
    return made > 0;
}

/*-------------------------------------------------------------------------*/

/** Deterministic mono music-like noise at the unit rate, endless: the music bus under the intercom voices */
class CMusicNoiseSource : public AUDIO::IAudioSource
{
    public:

    explicit CMusicNoiseSource( UInt32 sampleRate ) : m_format( sampleRate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), m_state( 0x2545F491 ), m_position( 0 ), m_low( 0.0f ) {}

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_format; }

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE
    {
        Float32* out = static_cast< Float32* >( destination );
        for ( UInt32 i=0; i<frameCapacity; ++i )
        {
            m_state ^= m_state << 13;
            m_state ^= m_state >> 17;
            m_state ^= m_state << 5;
            const Float32 white = (Float32) ( m_state & 0xFFFF ) / 32768.0f - 1.0f;
            m_low += 0.1f * ( white - m_low );
            out[ i ] = 0.5f * m_low;
        }
        m_position += frameCapacity;
        return frameCapacity;
    }

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_position; }

    private:

    AUDIO::CAudioFormat m_format;
    UInt32 m_state;
    UInt64 m_position;
    Float32 m_low;
};

/*-------------------------------------------------------------------------*/

/** Times per 10 ms of audio in microseconds, one entry per 10 ms, of the parts of the unit chain */
struct SChainTimes
{
    TTimes capturePath;         /**< capture conversion, high pass, gate, AGC, VAD, meter, microphone gain (no sender) */
    TTimes sender;              /**< the sender as capture consumer: accumulate, encode, frame */
    TTimes packetize;           /**< RTP packets of the frames, to every link */
    TTimes receive;             /**< packets read, frames pushed into the receivers */
    TTimes decode;              /**< the decoder alone, one per incoming stream */
    TTimes playoutTotal;        /**< playback render with the receivers: jitter buffer, playout, resampler, decode, mix, ducker */
    TTimes mixBaseline;         /**< playback render with the music bus and the ducker, no receiver */
    TTimes totalUnit;           /**< capture step, sender, packets and playback render of one 10 ms, as measured together */
    Float64 ratioPpm;           /**< |playout ratio - 1| at the end, parts per million */
    UInt64 concealed;
    UInt64 underruns;
    UInt64 decoded;

    SChainTimes( void ) : ratioPpm( 0.0 ), concealed( 0 ), underruns( 0 ), decoded( 0 ) {}
};

AUDIO::CVoiceSenderConfig
MakeChainSenderConfig( const AUDIO::CVoiceCodecConfig& codec )
{
    AUDIO::CVoiceSenderConfig config;
    config.SetCodecConfig( codec );
    config.SetStreamId( 0x0A0A0A0A );
    config.SetFirstSequenceNumber( 100 );
    config.SetFirstTimestamp( 0 );
    config.SetTransmitMode( AUDIO::VOICETRANSMITMODE_CONTINUOUS );
    config.SetOutboundQueueFrames( 64 );
    return config;
}

/** Sums consecutive pairs of 5 ms step times: the cost of 10 ms */
TTimes
PairSums( const TTimes& steps )
{
    TTimes result;
    for ( size_t i=0; i+1<steps.size(); i+=2 )
        result.push_back( steps[ i ] + steps[ i + 1 ] );
    return result;
}

/** The mean: parts that run every second or fourth step (a frame of 20 ms) would show as zero or as double in a median of 10 ms sums */
Float64
Mean( const TTimes& values )
{
    Float64 sum = 0.0;
    for ( size_t i=0; i<values.size(); ++i )
        sum += values[ i ];
    return values.empty() ? 0.0 : sum / (Float64) values.size();
}

/**
 *  The whole intercom unit per 10 ms of audio in simulated time, on null devices in cooperative mode: unit A takes the
 *  microphone (speech-like), runs high pass, gate, AGC and VAD, and sends with the given codec; unit B plays a music bus
 *  with a ducker keyed on the bus of the receivers, whose crystal runs 'driftPpm' fast so the playout resamples.
 *  Phase 1 measures the capture path alone and the playback of the music and the ducker alone, phase 2 adds the sender
 *  and 'receiverCount' receivers (each with a link of its own, wired model) and measures everything.
 */
bool
RunUnitChain( const AUDIO::CVoiceCodecConfig& codec, UInt32 receiverCount, Float64 driftPpm, SChainTimes& times )
{
    const UInt32 rate = 48000;
    const UInt32 period = 240;
    const Float64 stepUs = 1000000.0 * period / rate;

    VoiceLinkTest::CTestIntercomScenario scenario( period, rate );
    VoiceLinkTest::SUnitConfig unitConfigA;
    unitConfigA.keepOutput = false;
    VoiceLinkTest::SUnitConfig unitConfigB;
    unitConfigB.capture = false;
    unitConfigB.keepOutput = false;
    unitConfigB.driftPpm = driftPpm;
    VoiceLinkTest::CTestIntercomUnit* a = scenario.AddUnit( unitConfigA );
    VoiceLinkTest::CTestIntercomUnit* b = scenario.AddUnit( unitConfigB );
    if ( GUCEF_NULL == a || GUCEF_NULL == b )
        return false;

    // the capture chain of the design: high pass before the echo stage, gate and AGC after it
    AUDIO::CAudioEngine& captureEngine = *a->engine;
    AUDIO::TAudioEffectPtr highPass = captureEngine.CreateCaptureEffect( "filter" );
    AUDIO::TAudioEffectPtr gate = captureEngine.CreateCaptureEffect( "noiseGate" );
    AUDIO::TAudioEffectPtr agc = captureEngine.CreateCaptureEffect( "automaticGain" );
    if ( highPass.IsNULL() || gate.IsNULL() || agc.IsNULL() )
        return false;
    highPass.StaticCast< AUDIO::CFilterEffect >()->SetFilterType( AUDIO::FILTERBANDTYPE_HIGHPASS );
    highPass.StaticCast< AUDIO::CFilterEffect >()->SetFrequency( 100.0f );
    highPass.StaticCast< AUDIO::CFilterEffect >()->SetSlopeInDecibelsPerOctave( 24 );
    if ( captureEngine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_PRE_ECHO, 0, highPass ).HasNoFuture() ||
         captureEngine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 0, gate ).HasNoFuture() ||
         captureEngine.InsertCaptureEffect( AUDIO::AUDIOCAPTURESTAGE_POST_ECHO, 1, agc ).HasNoFuture() )
        return false;

    const VoiceCodecTest::TSamples speech = VoiceCodecTest::MakeSpeechLike( rate * 14, rate, 0.4f );
    VoiceLinkTest::CBufferSignal signal( speech );
    a->SetSignal( &signal );

    // the playback side: a bus for the received voices, a music bus ducked by it
    AUDIO::CAudioEngine& playEngine = *b->engine;
    AUDIO::CAudioBus* voiceBus = playEngine.CreateBus( "voices" );
    AUDIO::CAudioBus* musicBus = playEngine.CreateBus( "music" );
    if ( GUCEF_NULL == voiceBus || GUCEF_NULL == musicBus )
        return false;
    CORE::CValueList duckerOptions;
    duckerOptions.Set( "keyBus", "voices" );
    AUDIO::TAudioEffectPtr ducker = playEngine.CreateEffect( "ducker", duckerOptions );
    if ( ducker.IsNULL() || playEngine.InsertEffect( musicBus, 0, ducker ).HasNoFuture() )
        return false;
    AUDIO::TAudioVoicePtr music = playEngine.CreateVoice( AUDIO::TAudioSourcePtr( GUCEF_NEW CMusicNoiseSource( rate ) ), musicBus );
    if ( music.IsNULL() || playEngine.Start( music ).HasNoFuture() )
        return false;

    // phase 1: a short warm up, then the capture path alone and the music with the ducker alone
    const UInt32 warmSteps = (UInt32) ( 2.0 * rate / period );
    const UInt32 measureSteps = (UInt32) ( 6.0 * rate / period );
    Float64 nowUs = 0.0;
    TTimes captureSteps, mixSteps;
    for ( UInt32 s=0; s<warmSteps + measureSteps; ++s )
    {
        const UInt64 t0 = MT::PrecisionTickCount();
        a->Capture( period );
        const UInt64 t1 = MT::PrecisionTickCount();
        nowUs += stepUs;
        a->SetTime( nowUs );
        b->SetTime( nowUs );
        a->Play( period );
        a->engine->Update();
        const UInt64 t2 = MT::PrecisionTickCount();
        b->Play( period );
        const UInt64 t3 = MT::PrecisionTickCount();
        b->engine->Update();
        if ( s >= warmSteps )
        {
            captureSteps.push_back( Microseconds( t1 - t0 ) );
            mixSteps.push_back( Microseconds( t3 - t2 ) );
        }
    }
    times.capturePath = PairSums( captureSteps );
    times.mixBaseline = PairSums( mixSteps );

    // phase 2: the sender, the links and the receivers
    if ( !a->AttachSender( MakeChainSenderConfig( codec ) ) )
        return false;
    GUCEF::vector< VoiceLinkTest::CTestSimulatedVoiceLink* > links;
    GUCEF::vector< AUDIO::CVoiceReceiver* > receivers;
    for ( UInt32 r=0; r<receiverCount; ++r )
    {
        AUDIO::CVoiceReceiverConfig receiverConfig;
        receiverConfig.SetCodecConfig( codec );
        receiverConfig.SetPlayoutProfile( AUDIO::VOICEPLAYOUTPROFILE_WIRED );
        receiverConfig.SetInboundQueueFrames( 64 );
        AUDIO::CVoiceReceiver* receiver = b->AddReceiver( receiverConfig, voiceBus );
        if ( GUCEF_NULL == receiver )
            return false;
        receivers.push_back( receiver );
        VoiceLinkTest::CTestSimulatedVoiceLink* link = scenario.Connect( a, b, receiver, VoiceLinkTest::SLinkModel::Wired( 0x2468ACE1 + r ) );
        links.push_back( link );
    }
    GUCEF::vector< AUDIO::TVoiceDecoderPtr > decoders;
    for ( UInt32 r=0; r<receiverCount; ++r )
    {
        AUDIO::TVoiceDecoderPtr decoder;
        if ( !AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateDecoder( codec, decoder ) )
            return false;
        decoders.push_back( decoder );
    }
    const UInt32 frameSize = codec.GetFrameSizeInFrames();
    VoiceCodecTest::TSamples decoded( frameSize + 64, 0.0f );

    TTimes sendSteps, packetSteps, receiveSteps, decodeSteps, playSteps, totalSteps;
    VoiceLinkTest::TSentFrames sent;
    for ( UInt32 s=0; s<warmSteps + measureSteps; ++s )
    {
        const UInt64 t0 = MT::PrecisionTickCount();
        a->Capture( period );
        const UInt64 t1 = MT::PrecisionTickCount();
        sent.clear();
        a->PopFrames( sent );
        UInt64 decodeTicks = 0;
        const UInt64 t2 = MT::PrecisionTickCount();
        for ( size_t f=0; f<sent.size(); ++f )
            for ( size_t l=0; l<links.size(); ++l )
                links[ l ]->Send( sent[ f ].frame, sent[ f ].sendTimeInUs );
        const UInt64 t3 = MT::PrecisionTickCount();
        nowUs += stepUs;
        a->SetTime( nowUs );
        b->SetTime( nowUs );
        for ( size_t l=0; l<links.size(); ++l )
            links[ l ]->Deliver( nowUs, *receivers[ l ], b->LocalScale(), b->LocalOffsetUs() );
        const UInt64 t4 = MT::PrecisionTickCount();
        b->Play( period );
        b->engine->Update();
        const UInt64 t5 = MT::PrecisionTickCount();
        a->Play( period );
        a->engine->Update();

        // the decoder alone, outside the measured chain: what one decode per stream and frame costs
        for ( size_t f=0; f<sent.size(); ++f )
            for ( size_t d=0; d<decoders.size(); ++d )
            {
                const UInt64 d0 = MT::PrecisionTickCount();
                decoders[ d ]->Decode( sent[ f ].frame.GetPayload(), sent[ f ].frame.GetPayloadSize(), &decoded[ 0 ], frameSize );
                decodeTicks += MT::PrecisionTickCount() - d0;
            }
        if ( s >= warmSteps )
        {
            sendSteps.push_back( Microseconds( t1 - t0 ) );
            packetSteps.push_back( Microseconds( t3 - t2 ) );
            receiveSteps.push_back( Microseconds( t4 - t3 ) );
            playSteps.push_back( Microseconds( t5 - t4 ) );
            decodeSteps.push_back( Microseconds( decodeTicks ) );
            totalSteps.push_back( Microseconds( t1 - t0 ) + Microseconds( t5 - t2 ) );
        }
    }
    times.sender = PairSums( sendSteps );              // still holds the capture path: the caller subtracts its median
    times.packetize = PairSums( packetSteps );
    times.receive = PairSums( receiveSteps );
    times.decode = PairSums( decodeSteps );
    times.playoutTotal = PairSums( playSteps );
    times.totalUnit = PairSums( totalSteps );

    AUDIO::CVoiceReceiverStats stats;
    receivers[ 0 ]->GetStats( stats );
    times.ratioPpm = stats.ratioInPpm < 0.0 ? -stats.ratioInPpm : stats.ratioInPpm;
    times.concealed = stats.framesConcealed;
    times.underruns = stats.underruns;
    times.decoded = stats.framesDecoded;
    a->DetachSender();
    return true;
}

/*-------------------------------------------------------------------------*/

/** What an echo canceller cost per 10 ms of audio at 16 kHz */
struct SEchoCost
{
    Float64 meanUs;             /**< render tap and capture side together */
    Float64 renderUs;           /**< the render tap alone (WriteReference()) */
    Float64 p95Us;
    Float64 transformsPerSecond;
    Float64 erleDb;
    SEchoCost( void ) : meanUs( 0.0 ), renderUs( 0.0 ), p95Us( 0.0 ), transformsPerSecond( 0.0 ), erleDb( 0.0 ) {}
};

/**
 *  Times a canceller over 'seconds' of audio, 10 ms at a time (two periods of 5 ms: the render tap, then the capture side),
 *  after the first 2 s. 'far': the far end talks (else it is silent: the canceller idles); 'near': the near end talks
 *  over it all the time (double talk). The reference comes at 'referenceRate' (48000: three times the frames of 16 kHz,
 *  the canceller decimates); the echo path is a room of 20 ms bulk delay, -15 dB, with early reflections
 */
bool
TimeEchoCanceller( const AUDIO::CEchoCancellerSettings& settings, bool far, bool near, UInt32 referenceRate, Float64 seconds, SEchoCost& cost )
{
    const UInt32 rate = 16000;
    const UInt32 period = 80;
    const UInt32 factor = referenceRate / rate;
    EchoTest::SEchoRigConfig rig;
    const EchoTest::TSamples talker = far ? EchoTest::MakeFarTalker( rate, seconds, 5, -20.0 ) : EchoTest::TSamples( (size_t) ( seconds * rate ), 0.0f );
    EchoTest::TSamples mic = EchoTest::Convolve( EchoTest::Played( talker, rig, talker.size() ), EchoTest::MakeEarlyReflections( rate, 20.0, -15.0, 3 ) );
    const EchoTest::TSamples echo = mic;
    TestVoiceSignals::MixInto( mic, TestVoiceSignals::MakeWhiteNoise( mic.size(), pow( 10.0, -65.0 / 20.0 ), 9 ) );
    if ( near )
        TestVoiceSignals::MixInto( mic, EchoTest::MakeFarTalker( rate, seconds, 77, -26.0, 0.0, 0.3 ) );
    // the reference at its rate: each frame repeated (the decimator does the same work on any content)
    EchoTest::TSamples reference( talker.size() * factor, 0.0f );
    for ( size_t i=0; i<reference.size(); ++i )
        reference[ i ] = talker[ i / factor ];

    AUDIO::CAcousticEchoCanceller canceller( settings );
    if ( !canceller.Prepare( rate, 4096, referenceRate, 1, 4096 ) )
        return false;
    EchoTest::TSamples out = mic;
    TTimes times;
    Float64 renderTotal = 0.0;
    UInt64 transformsAtStart = 0;
    const size_t warmUp = 2 * rate;
    size_t at = 0;
    while ( at + 2 * period <= mic.size() )
    {
        const UInt64 t0 = MT::PrecisionTickCount();
        UInt64 renderTicks = 0;
        for ( UInt32 p=0; p<2; ++p )
        {
            const UInt64 r0 = MT::PrecisionTickCount();
            canceller.WriteReference( &reference[ ( at + p * period ) * factor ], period * factor );
            renderTicks += MT::PrecisionTickCount() - r0;
            canceller.Process( &out[ at + p * period ], period, at + p * period );
        }
        const UInt64 t1 = MT::PrecisionTickCount();
        if ( at == warmUp )
            transformsAtStart = canceller.GetTransformCount();
        if ( at >= warmUp )
        {
            times.push_back( Microseconds( t1 - t0 ) );
            renderTotal += Microseconds( renderTicks );
        }
        at += 2 * period;
    }
    if ( times.empty() )
        return false;
    cost.meanUs = Mean( times );
    cost.renderUs = renderTotal / (Float64) times.size();
    const SStats stats = Summarize( times );
    cost.p95Us = stats.p95Us;
    cost.transformsPerSecond = (Float64) ( canceller.GetTransformCount() - transformsAtStart ) / ( (Float64) times.size() / 100.0 );
    // the linear filter's echo return loss enhancement over the second half (without the residual suppressor the output lines up)
    if ( !settings.GetResidualSuppression() )
        cost.erleDb = EchoTest::ErleDb( echo, EchoTest::Difference( out, EchoTest::Difference( mic, echo ) ), mic.size() / 2, mic.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The noise suppressor per 10 ms at 16 kHz on speech in noise, after the first second */
Float64
TimeNoiseSuppressor( Float64 seconds, Float32 amount, UInt64& transforms )
{
    const UInt32 rate = 16000;
    EchoTest::TSamples x = EchoTest::MakeFarTalker( rate, seconds, 13, -26.0 );
    TestVoiceSignals::MixInto( x, TestVoiceSignals::MakeWhiteNoise( x.size(), 0.003, 5 ) );
    AUDIO::CNoiseSuppressorEffect suppressor;
    suppressor.SetAmountInDecibels( amount );
    suppressor.Prepare( AUDIO::CAudioFormat( rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), 4096 );
    TTimes times;
    for ( size_t at=0; at + 160 <= x.size(); at += 160 )
    {
        const UInt64 t0 = MT::PrecisionTickCount();
        suppressor.Process( &x[ at ], 160 );
        const UInt64 t1 = MT::PrecisionTickCount();
        if ( at >= rate )
            times.push_back( Microseconds( t1 - t0 ) );
    }
    transforms = suppressor.GetTransformedFrameCount();
    return Mean( times );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioBenchmarkTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING audio BENCHMARKS (report only, GUCEF_TEST_AUDIO_BENCH=1)" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.Benchmark" );

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Decode cost per block of 500 ms for each codec (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const bool hasFlac = LoadPlugin( "audiocodecpluginFLAC", "audiobenchflac", "flac" );
        const bool hasOgg = LoadPlugin( "audiocodecpluginOGG", "audiobenchogg", "ogg" );
        const bool hasMp3 = LoadPlugin( "audiocodecpluginMP3", "audiobenchmp3", "mp3" );
        ASSERT_TRUE( hasFlac && hasOgg && hasMp3 );

        const char* kinds[ 4 ] = { "flac", "vorbis", "opus", "mp3" };
        const char* types[ 4 ] = { "flac", "ogg", "opus", "mp3" };
        for ( UInt32 k=0; k<4; ++k )
        {
            TCodecPtr codec = FindCodec( types[ k ] );
            if ( codec.IsNULL() )
                continue;
            const UInt32 rate = ( 2 == k ) ? 48000 : 44100;
            CORE::CDynamicBuffer file;
            if ( 3 == k )
                MakeStream( file, 2, 400, true );                                       // about 10 s of frames made in code
            else
            {
                CORE::CValueList options;
                if ( 1 == k )
                    options.Set( "quality", "0.5" );
                if ( 2 == k )
                    options.Set( "bitrate", "128000" );
                if ( !Encode( codec, MakeMusicLike( rate, rate * 10, 1 ), file, options ) )
                    continue;                                                           // no Opus in this build of the plugin
            }

            TTimes times;
            UInt64 totalFrames = 0;
            for ( UInt32 pass=0; pass<4; ++pass )
            {
                AUDIO::TAudioSourcePtr source;
                ASSERT_TRUE( OpenSource( codec, file, source ) );
                if ( source.IsNULL() )
                    break;
                const UInt32 frameBytes = source->GetFormat().GetBytesPerFrame();
                const UInt32 blockFrames = source->GetFormat().GetSampleRate() / 2;
                CORE::CDynamicBuffer block( blockFrames * frameBytes, true );
                while ( !source->IsEndOfStream() )
                {
                    const UInt64 start = MT::PrecisionTickCount();
                    UInt32 got = 0;
                    while ( got < blockFrames )
                    {
                        const UInt32 more = source->ReadFrames( static_cast< UInt8* >( block.GetBufferPtr() ) + (size_t) got * frameBytes, blockFrames - got );
                        if ( 0 == more )
                            break;
                        got += more;
                    }
                    if ( got < blockFrames )
                        break;                                                          // the short last block is not a full block
                    if ( pass > 0 )                                                     // the first pass warms up caches and tables
                    {
                        times.push_back( Microseconds( MT::PrecisionTickCount() - start ) );
                        totalFrames += got;
                    }
                }
            }
            ASSERT_TRUE( times.size() >= 15 );
            const SStats stats = Summarize( times );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: decode " << kinds[ k ] << ", stereo " << rate << " Hz, block of 500 ms: median " << Round( stats.medianUs ) <<
                                              " us, min " << Round( stats.minUs ) << " us, 95th percentile " << Round( stats.p95Us ) << " us, max " << Round( stats.maxUs ) <<
                                              " us, median is " << Round( 100.0 * stats.medianUs / 500000.0 ) << " % of the block, " << times.size() << " blocks" <<
                                              ( 3 == k ? " (frames made in code, a light spectrum)" : "" ) );
        }

        ASSERT_TRUE( UnloadPlugin( "audiobenchflac", "flac" ) && UnloadPlugin( "audiobenchogg", "ogg" ) && UnloadPlugin( "audiobenchmp3", "mp3" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Mix cost per block of 256 frames at 48 kHz with N voices (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const UInt32 counts[ 5 ] = { 1, 8, 32, 64, 128 };
        for ( UInt32 c=0; c<5; ++c )
        {
            CORE::CString what;
            what = CORE::UInt32ToString( counts[ c ] ) + " stereo voices at 48 kHz on the master bus, lookahead limiter on";
            ASSERT_TRUE( MixBenchmark( what.C_String(), counts[ c ], true, 48000, 2, "the period of 5333 us", PeriodInUs ) );
        }
        ASSERT_TRUE( MixBenchmark( "64 stereo voices at 48 kHz, limiter off", 64, false, 48000, 2, "the period of 5333 us", PeriodInUs ) );
        ASSERT_TRUE( MixBenchmark( "64 mono voices at 48 kHz panned to stereo, lookahead limiter on", 64, true, 48000, 1, "the period of 5333 us", PeriodInUs ) );
        ASSERT_TRUE( MixBenchmark( "64 stereo voices at 44.1 kHz resampled to 48 kHz, lookahead limiter on", 64, true, 44100, 2, "the period of 5333 us", PeriodInUs ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: The typical game graph: 64 voices on 4 buses, compressor, equalizer, master limiter, without and with a reverb send (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        // 0: without reverb, 1: with a reverb bus in normal quality that the sfx and voice buses send to, 2: the same in low quality
        for ( UInt32 variant=0; variant<3; ++variant )
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( BenchParams( true ) ) );
            AUDIO::CAudioBusLayout layout;
            layout.AddBus( "master" );
            layout.AddBus( "music" );
            layout.AddBus( "sfx" );
            layout.AddBus( "voice" );
            layout.AddBus( "ui" );
            if ( variant > 0 )
                layout.AddBus( "reverb" );
            layout.FindBus( "music" )->AddEffect( 0, "equalizer" );
            layout.FindBus( "sfx" )->AddEffect( 0, "compressor" );
            if ( variant > 0 )
            {
                layout.FindBus( "sfx" )->AddSend( "reverb", -9.0f, false );
                layout.FindBus( "voice" )->AddSend( "reverb", -12.0f, false );
                AUDIO::CAudioBusLayout::SEffect& reverb = layout.FindBus( "reverb" )->AddEffect( 0, "reverb" );
                if ( 2 == variant )
                    reverb.SetOption( "quality", "low" );
            }
            Float64 callUs = 0.0;
            Float64 totalUs = 0.0;
            ASSERT_TRUE( ApplyAndWait( engine, layout, callUs, totalUs ) );
            AUDIO::CAudioBus* buses[ 4 ] = { engine.FindBus( "music" ), engine.FindBus( "sfx" ), engine.FindBus( "voice" ), engine.FindBus( "ui" ) };
            ASSERT_TRUE( StartVoices( engine, buses, 4, 64, 48000, 2 ) );
            SStats stats;
            ASSERT_TRUE( MeasureRender( engine, 20, 100, stats ) );
            const char* names[ 3 ] = { "typical game graph without reverb (64 stereo voices, 4 buses, equalizer, compressor, lookahead master limiter)",
                                       "typical game graph with a reverb send (the same and a reverb bus in normal quality, sends from two buses)",
                                       "typical game graph with a reverb send, reverb in low quality" };
            Report( names[ variant ], stats, 0.1 * PeriodInUs, "the budget of 10 % of the period (533 us)" );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: 64 voices with a filter each and one send to an aux bus (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( BenchParams( true ) ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        ASSERT_TRUE( GUCEF_NULL != aux );
        AUDIO::TAudioClipPtr clip = MakeNoiseClip( MixRate, 2, MixRate, 7 );
        for ( UInt32 i=0; i<64; ++i )
        {
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clip );
            AUDIO::TAudioEffectPtr filter = engine.CreateEffect( "filter" );
            ASSERT_TRUE( !voice.IsNULL() && !filter.IsNULL() && voice->AddEffect( filter ) && voice->SetSend( 0, aux, 0.2f ) );
            voice->SetLoopCount( 0 );
            voice->SetVolume( 0.05f );
            voice->SetPan( -1.0f + 2.0f * (Float32) ( i % 17 ) / 16.0f );
            ASSERT_FALSE( engine.Start( voice ).HasNoFuture() );
        }
        SStats stats;
        ASSERT_TRUE( MeasureRender( engine, 20, 100, stats ) && 64 == engine.GetActiveVoiceCount() );
        Report( "64 stereo voices at 48 kHz, each with a filter (12 dB low pass) and a send to an aux bus, lookahead limiter on", stats, PeriodInUs, "the period of 5333 us" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Cost of each built-in effect and the limiter per block of 256 frames (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        AUDIO::CAudioEngine stereo;
        ASSERT_TRUE( stereo.Initialize( BenchParams( false ) ) );

        const CORE::CValueList none;
        const char* limiterInstant[] = { "mode", GUCEF_NULL };
        Float32 instantValue[] = { (Float32) AUDIO::AUDIOLIMITERMODE_INSTANT };
        ASSERT_TRUE( EffectBenchmark( stereo, "limiter", none, 2, "limiter, lookahead mode, stereo", 15.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "limiter", none, 2, "limiter, instant mode, stereo", 15.0, limiterInstant, instantValue ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "compressor", none, 2, "compressor, stereo", 15.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "ducker", none, 2, "ducker, not ducking (no key), stereo", 5.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "ducker", none, 2, "ducker, ducking (manual key), stereo", 10.0, GUCEF_NULL, GUCEF_NULL, true ) );
        const char* gateLevel[] = { "detector", GUCEF_NULL };
        Float32 gateLevelValue[] = { 0.0f };
        ASSERT_TRUE( EffectBenchmark( stereo, "noiseGate", none, 2, "noise gate, voice detector (default), stereo", 30.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "noiseGate", none, 2, "noise gate, level detector (RMS), stereo", 5.0, gateLevel, gateLevelValue ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "automaticGain", none, 2, "automatic gain (voice detector, ceiling stage), stereo", 30.0, GUCEF_NULL, GUCEF_NULL ) );

        CORE::CValueList bands;
        bands.Set( "bands", (Int32) 8 );
        const char* eqNames[] = { "band0Gain", "band1Gain", "band2Gain", "band3Gain", "band4Gain", "band5Gain", "band6Gain", "band7Gain", GUCEF_NULL };
        Float32 eqValues[] = { 3.0f, -3.0f, 3.0f, -3.0f, 3.0f, -3.0f, 3.0f, -3.0f };
        ASSERT_TRUE( EffectBenchmark( stereo, "equalizer", bands, 2, "equalizer, 8 bands all active, stereo", 30.0, eqNames, eqValues ) );

        const char* filterNames[] = { "slope", GUCEF_NULL };
        Float32 filterValues[] = { 24.0f };
        ASSERT_TRUE( EffectBenchmark( stereo, "filter", none, 2, "filter, 24 dB per octave, stereo", 15.0, filterNames, filterValues ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "delay", none, 2, "delay, default feedback and filters, stereo", 10.0, GUCEF_NULL, GUCEF_NULL ) );

        CORE::CValueList lowQuality;
        lowQuality.Set( "quality", "low" );
        ASSERT_TRUE( EffectBenchmark( stereo, "reverb", none, 2, "reverb, normal quality, default settings, stereo", 40.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( stereo, "reverb", lowQuality, 2, "reverb, low quality, default settings, stereo", 20.0, GUCEF_NULL, GUCEF_NULL ) );
        const char* reverbModulation[] = { "decayTime", "size", "modulationDepth", GUCEF_NULL };
        Float32 reverbModulationValues[] = { 6.0f, 1.0f, 1.0f };
        ASSERT_TRUE( EffectBenchmark( stereo, "reverb", none, 2, "reverb, normal quality, decay time 6 s, size 1, full modulation, stereo", 40.0, reverbModulation, reverbModulationValues ) );

        AUDIO::CAudioEngine surround;
        ASSERT_TRUE( surround.Initialize( BenchParams( false, MixRate, 8 ) ) );
        ASSERT_TRUE( EffectBenchmark( surround, "limiter", none, 8, "limiter, lookahead mode, 7.1 (8 channels)", 30.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( surround, "compressor", none, 8, "compressor, 7.1 (8 channels)", 30.0, GUCEF_NULL, GUCEF_NULL ) );
        ASSERT_TRUE( EffectBenchmark( surround, "reverb", none, 8, "reverb, normal quality, 7.1 (8 channels)", 80.0, GUCEF_NULL, GUCEF_NULL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Sinc resampler against the linear resampler, per 10 ms of mono audio (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        struct SCase { const char* name; Float64 inputRate; Float64 outputRate; };
        const SCase cases[] = {
            { "48 kHz to 16 kHz (capture)", 48000.0, 16000.0 },
            { "16 kHz to 48 kHz", 16000.0, 48000.0 },
            { "44.1 kHz to 48 kHz", 44100.0, 48000.0 },
            { "48 kHz drift correction, ratio 1.0003", 48014.4, 48000.0 },
            { "16 kHz drift correction, ratio 1.0003", 16004.8, 16000.0 } };
        for ( UInt32 i=0; i<5; ++i )
        {
            AUDIO::CLinearResampler linear;
            AUDIO::CSincResampler sinc;
            SStats linearStats;
            SStats sincStats;
            ASSERT_TRUE( ResamplerBenchmark( linear, cases[ i ].inputRate, cases[ i ].outputRate, linearStats ) );
            ASSERT_TRUE( ResamplerBenchmark( sinc, cases[ i ].inputRate, cases[ i ].outputRate, sincStats ) );
            const CORE::CString linearName = CORE::CString( "linear resampler, " ) + cases[ i ].name;
            const CORE::CString sincName = CORE::CString( "sinc resampler, " ) + cases[ i ].name;
            Report( linearName.C_String(), linearStats, 10000.0, "10 ms of audio" );
            Report( sincName.C_String(), sincStats, 10000.0, "10 ms of audio" );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: " << cases[ i ].name << ": sinc costs " << Round( sincStats.medianUs / ( linearStats.medianUs > 0.01 ? linearStats.medianUs : 0.01 ) ) <<
                                              " times the linear resampler" );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: ApplyBusLayout on the owner side: building, applying the same layout again and changing it (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const AUDIO::CAudioBusLayout first = MakeLayout( false );
        const AUDIO::CAudioBusLayout second = MakeLayout( true );

        TTimes buildCall;
        TTimes buildTotal;
        for ( UInt32 i=0; i<20; ++i )
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( BenchParams( true ) ) );
            Float64 callUs = 0.0;
            Float64 totalUs = 0.0;
            ASSERT_TRUE( ApplyAndWait( engine, first, callUs, totalUs ) );
            buildCall.push_back( callUs );
            buildTotal.push_back( totalUs );
        }
        Report( "ApplyBusLayout building 6 buses, 7 effects and a send in an engine without them, the call", Summarize( buildCall ), PeriodInUs, "the period of 5333 us" );
        Report( "ApplyBusLayout building the same, until the audio thread has taken the commands (blocks rendered meanwhile)", Summarize( buildTotal ), PeriodInUs, "the period of 5333 us" );

        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( BenchParams( true ) ) );
        Float64 callUs = 0.0;
        Float64 totalUs = 0.0;
        ASSERT_TRUE( ApplyAndWait( engine, first, callUs, totalUs ) );
        TTimes same;
        for ( UInt32 i=0; i<200; ++i )
        {
            ASSERT_TRUE( ApplyAndWait( engine, first, callUs, totalUs ) );
            same.push_back( callUs );
        }
        Report( "ApplyBusLayout with the layout that is already applied (no commands)", Summarize( same ), PeriodInUs, "the period of 5333 us" );

        TTimes changeCall;
        TTimes changeTotal;
        for ( UInt32 i=0; i<200; ++i )
        {
            ASSERT_TRUE( ApplyAndWait( engine, 0 == i % 2 ? second : first, callUs, totalUs ) );
            changeCall.push_back( callUs );
            changeTotal.push_back( totalUs );
        }
        Report( "ApplyBusLayout changing volumes, parameters, one effect type and a send level, the call", Summarize( changeCall ), PeriodInUs, "the period of 5333 us" );
        Report( "ApplyBusLayout changing the same, until the audio thread has taken the commands", Summarize( changeTotal ), PeriodInUs, "the period of 5333 us" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Voice codecs per 10 ms of mono audio at 48 kHz: Opus encode, decode and concealment in both profiles at complexity 0, 5 and 10, and the pcm codec (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const bool hasOpus = VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiobenchvoiceopus", "opus" );
        const VoiceCodecTest::TSamples input = VoiceCodecTest::MakeSpeechLike( 48000 * 4, 48000, 0.4f );

        // codec, profile, frame duration in us, complexity, description
        const char* codecs[] = { "pcm", "opus", "opus", "opus", "opus", "opus", "opus" };
        const AUDIO::TVoiceCodecProfile profiles[] = { AUDIO::VOICECODECPROFILE_LOWDELAY, AUDIO::VOICECODECPROFILE_LOWDELAY, AUDIO::VOICECODECPROFILE_LOWDELAY, AUDIO::VOICECODECPROFILE_LOWDELAY,
                                                       AUDIO::VOICECODECPROFILE_ROBUST, AUDIO::VOICECODECPROFILE_ROBUST, AUDIO::VOICECODECPROFILE_ROBUST };
        const UInt32 durations[] = { 10000, 10000, 10000, 10000, 20000, 20000, 20000 };
        const UInt32 complexities[] = { 0, 0, 5, 10, 0, 5, 10 };
        const char* names[] = { "pcm", "opus low delay, complexity 0", "opus low delay, complexity 5", "opus low delay, complexity 10",
                                "opus robust, complexity 0", "opus robust, complexity 5", "opus robust, complexity 10" };

        for ( UInt32 v=0; v<7; ++v )
        {
            if ( 0 != ::strcmp( codecs[ v ], "pcm" ) && !hasOpus )
                continue;

            AUDIO::CVoiceCodecConfig config = VoiceCodecTest::MakeConfig( codecs[ v ], 48000, durations[ v ] );
            config.SetProfile( profiles[ v ] );
            config.SetComplexity( complexities[ v ] );
            config.SetBitrate( AUDIO::VOICECODECPROFILE_ROBUST == profiles[ v ] ? 32000 : 48000 );
            config.SetInbandFec( AUDIO::VOICECODECPROFILE_ROBUST == profiles[ v ] );
            config.SetExpectedLossPercentage( AUDIO::VOICECODECPROFILE_ROBUST == profiles[ v ] ? 10 : 0 );
            AUDIO::TVoiceEncoderPtr encoder;
            AUDIO::TVoiceDecoderPtr decoder;
            ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateEncoder( config, encoder ) );
            ASSERT_TRUE( AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry().CreateDecoder( config, decoder ) );

            const UInt32 frameSize = encoder->GetFrameSizeInFrames();
            const UInt32 frames = (UInt32) ( input.size() / frameSize );
            const Float64 per10Ms = 10000.0 / durations[ v ];           // the cost of a frame as the cost of 10 ms
            GUCEF::vector< GUCEF::vector< UInt8 > > packets;
            UInt8 payload[ 2048 ];
            VoiceCodecTest::TSamples out( frameSize, 0.0f );
            const UInt32 batch = 10;
            TTimes encodeTimes, decodeTimes, concealTimes;

            // encoding, in batches of frames: the clock has a resolution of one microsecond
            for ( UInt32 f=0; f+batch<=frames; f+=batch )
            {
                const UInt64 start = MT::PrecisionTickCount();
                for ( UInt32 b=0; b<batch; ++b )
                {
                    const Int32 size = encoder->Encode( &input[ (size_t) ( f + b ) * frameSize ], payload, sizeof( payload ) );
                    if ( size <= 0 )
                        break;
                    packets.push_back( GUCEF::vector< UInt8 >( payload, payload + size ) );
                }
                encodeTimes.push_back( Microseconds( MT::PrecisionTickCount() - start ) / batch * per10Ms );
            }
            ASSERT_TRUE( packets.size() + batch > frames );

            for ( UInt32 f=0; f+batch<=(UInt32) packets.size(); f+=batch )
            {
                const UInt64 start = MT::PrecisionTickCount();
                for ( UInt32 b=0; b<batch; ++b )
                    decoder->Decode( &packets[ f + b ][ 0 ], (UInt32) packets[ f + b ].size(), &out[ 0 ], frameSize );
                decodeTimes.push_back( Microseconds( MT::PrecisionTickCount() - start ) / batch * per10Ms );
            }

            for ( UInt32 f=0; f+batch<=(UInt32) packets.size(); f+=batch )
            {
                for ( UInt32 b=0; b<batch; ++b )
                    decoder->Decode( &packets[ f + b ][ 0 ], (UInt32) packets[ f + b ].size(), &out[ 0 ], frameSize );
                const UInt64 start = MT::PrecisionTickCount();
                decoder->Conceal( &out[ 0 ], frameSize );
                concealTimes.push_back( Microseconds( MT::PrecisionTickCount() - start ) * per10Ms );
            }

            const CORE::CString name = CORE::CString( "voice codec " ) + names[ v ];
            Report( ( name + ", encode" ).C_String(), Summarize( encodeTimes ), 10000.0, "10 ms of audio (one core)" );
            Report( ( name + ", decode" ).C_String(), Summarize( decodeTimes ), 10000.0, "10 ms of audio (one core)" );
            Report( ( name + ", conceal one lost frame" ).C_String(), Summarize( concealTimes ), 10000.0, "10 ms of audio (one core)" );
        }
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "audiobenchvoiceopus" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: The whole intercom unit per 10 ms of audio: capture path, sender, packets, receivers with jitter buffer, playout, sinc resampler and decode, mix with the ducker; as a percentage of one core, projected for a Pi 4 and a Pi Zero 2 W (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const bool hasOpus = VoiceCodecTest::LoadVoiceCodecPlugin( "audiovoicecodecOPUS", "audiobenchunitopus", "opus" );

        // Scale factors against this machine. The design (section 7) gives the Pi Zero 2 W about 3.2 times the Pi 4 for each of its rows
        // and no factor between a workstation and a Pi 4; the factor 4 for the Pi 4 is an assumption (clock ratio about 3, lower work per clock),
        // so the Pi Zero 2 W is 4 * 3.2. The projections are indications, not measurements
        const Float64 pi4Factor = 4.0;
        const Float64 piZeroFactor = 4.0 * 3.2;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: unit chain scale factors against this machine: Pi 4 " << Round( pi4Factor ) << ", Pi Zero 2 W " << Round( piZeroFactor ) <<
                                          " (assumed: section 7 of the design gives only the ratio of about 3.2 between the two)" );

        AUDIO::CVoiceCodecConfig opusLowDelay = VoiceCodecTest::MakeConfig( "opus", 48000, 10000 );
        opusLowDelay.SetProfileDefaults( AUDIO::VOICECODECPROFILE_LOWDELAY );
        AUDIO::CVoiceCodecConfig opusRobust = VoiceCodecTest::MakeConfig( "opus", 48000, 10000 );
        opusRobust.SetProfileDefaults( AUDIO::VOICECODECPROFILE_ROBUST );
        const AUDIO::CVoiceCodecConfig codecs[] = { VoiceStreamTest::PcmConfig( 48000, 10000 ), opusLowDelay, opusRobust };
        const char* names[] = { "pcm", "opus low delay", "opus robust" };
        const UInt32 streamCounts[] = { 1, 4 };

        for ( UInt32 c=0; c<3; ++c )
        {
            if ( c > 0 && !hasOpus )
                continue;
            for ( UInt32 n=0; n<2; ++n )
            {
                SChainTimes times;
                ASSERT_TRUE( RunUnitChain( codecs[ c ], streamCounts[ n ], 200.0, times ) );
                ASSERT_TRUE( times.totalUnit.size() > 100 );

                const SStats total = Summarize( times.totalUnit );

                // the parts as means over the 10 ms windows; the sender and the playout path contain others, which are taken out
                const Float64 captureUs = Mean( times.capturePath );
                const Float64 senderUs = Mean( times.sender ) > captureUs ? Mean( times.sender ) - captureUs : 0.0;
                Float64 jitterAndPlayout = Mean( times.playoutTotal ) - Mean( times.decode ) - Mean( times.mixBaseline );
                if ( jitterAndPlayout < 0.0 )
                    jitterAndPlayout = 0.0;

                const CORE::CString label = CORE::CString( "unit chain, " ) + names[ c ] + ", " + CORE::UInt32ToString( streamCounts[ n ] ) + " incoming stream" + ( 1 == streamCounts[ n ] ? "" : "s" );
                const struct { const char* what; Float64 us; } parts[] = {
                    { "capture path (high pass, gate, AGC, VAD, meter)", captureUs },
                    { "sender (encode, frame)", senderUs },
                    { "packetize", Mean( times.packetize ) },
                    { "receive (packet read, push)", Mean( times.receive ) },
                    { "jitter buffer, playout and sinc resampler", jitterAndPlayout },
                    { "decode", Mean( times.decode ) },
                    { "mix, music bus and ducker", Mean( times.mixBaseline ) } };
                for ( UInt32 p=0; p<7; ++p )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: " << label << ", " << parts[ p ].what << ": mean " << Round( parts[ p ].us ) << " us per 10 ms, " << Round( 100.0 * parts[ p ].us / 10000.0 ) << " % of one core" );

                const Float64 meanUs = Mean( times.totalUnit );
                const Float64 percent = 100.0 * meanUs / 10000.0;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: " << label << ", total (capture step, sender, packets, playback render): mean " << Round( meanUs ) << " us per 10 ms, median " << Round( total.medianUs ) << " us, min " << Round( total.minUs ) <<
                                                  " us, 95th percentile " << Round( total.p95Us ) << " us, max " << Round( total.maxUs ) << " us; " << Round( percent ) << " % of one core here, projected " <<
                                                  Round( percent * pi4Factor ) << " % on a Pi 4, " << Round( percent * piZeroFactor ) << " % on a Pi Zero 2 W; playout ratio " << Round( times.ratioPpm ) <<
                                                  " ppm from 1, decoded " << times.decoded << ", concealed " << times.concealed << ", underruns " << times.underruns );
            }
        }
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "audiobenchunitopus" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioBenchmark: Noise suppressor and echo canceller per 10 ms of mono audio at 16 kHz: settings, idle, double talk and the render tap; against the budget of 0.30 % of one core for both, projected for a Pi 4 and a Pi Zero 2 W (report only, opt in)" )
    SKIP_UNLESS_BENCH();
    try
    {
        const Float64 pi4Factor = 4.0;
        const Float64 piZeroFactor = 4.0 * 3.2;
        const Float64 seconds = 32.0;

        UInt64 transforms = 0;
        const Float64 nsUs = TimeNoiseSuppressor( seconds, -15.0f, transforms );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: noise suppressor, 16 kHz, speech in noise: mean " << Round( nsUs ) << " us per 10 ms, " << Round( nsUs / 100.0 ) << " % of one core here, " <<
                   Round( nsUs / 100.0 * pi4Factor ) << " % on a Pi 4, " << Round( nsUs / 100.0 * piZeroFactor ) << " % on a Pi Zero 2 W; " << Round( (Float64) transforms * 2.0 / seconds ) << " FFTs per second" );
        const Float64 nsOffUs = TimeNoiseSuppressor( seconds, 0.0f, transforms );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: noise suppressor, amount 0 dB (only the delay): mean " << Round( nsOffUs ) << " us per 10 ms, " << transforms << " transforms" );

        struct SVariant
        {
            const char* what;
            UInt32 block;
            Float32 filterMs;
            UInt32 adaptEvery;
            bool residual;
            bool suppressNoise;
            bool far;
            bool near;
            UInt32 referenceRate;
        };
        const SVariant variants[] = {
            { "default (block 128, 128 ms, residual suppressor), far end talks, 16 kHz reference", 0, 128.0f, 1, true, false, true, false, 16000 },
            { "default, far end talks, 48 kHz reference (render tap decimates)", 0, 128.0f, 1, true, false, true, false, 48000 },
            { "default, far end silent (idle), 48 kHz reference", 0, 128.0f, 1, true, false, false, false, 48000 },
            { "default, double talk, 16 kHz reference", 0, 128.0f, 1, true, false, true, true, 16000 },
            { "without residual suppressor", 0, 128.0f, 1, false, false, true, false, 16000 },
            { "residual suppressor also suppressing noise (instead of the effect)", 0, 128.0f, 1, true, true, true, false, 16000 },
            { "block 64 (4 ms)", 64, 128.0f, 1, false, false, true, false, 16000 },
            { "block 256 (16 ms)", 256, 128.0f, 1, false, false, true, false, 16000 },
            { "filter 64 ms", 0, 64.0f, 1, false, false, true, false, 16000 },
            { "filter 96 ms", 0, 96.0f, 1, false, false, true, false, 16000 },
            { "filter 256 ms", 0, 256.0f, 1, false, false, true, false, 16000 },
            { "adapting in every second block", 0, 128.0f, 2, false, false, true, false, 16000 },
            { "block 256, filter 96 ms, adapting in every second block", 256, 96.0f, 2, false, false, true, false, 16000 }
        };
        Float64 defaultUs = 0.0, defaultRenderUs = 0.0;
        for ( UInt32 v=0; v<sizeof( variants ) / sizeof( variants[ 0 ] ); ++v )
        {
            AUDIO::CEchoCancellerSettings settings;
            settings.SetBlockFrames( variants[ v ].block );
            settings.SetFilterLengthInMs( variants[ v ].filterMs );
            settings.SetAdaptEveryBlocks( variants[ v ].adaptEvery );
            settings.SetResidualSuppression( variants[ v ].residual );
            settings.SetSuppressNoise( variants[ v ].suppressNoise );
            SEchoCost cost;
            ASSERT_TRUE( TimeEchoCanceller( settings, variants[ v ].far, variants[ v ].near, variants[ v ].referenceRate, seconds, cost ) );
            if ( 1 == v )
            {
                defaultUs = cost.meanUs;
                defaultRenderUs = cost.renderUs;
            }
            const Float64 percent = cost.meanUs / 100.0;
            CORE::CString erle;
            if ( !variants[ v ].residual )
                erle = CORE::CString( "; ERLE of the second half " ) + Round( cost.erleDb ) + " dB";
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: echo canceller, " << variants[ v ].what << ": mean " << Round( cost.meanUs ) << " us per 10 ms (render tap " << Round( cost.renderUs ) <<
                       " us), 95th percentile " << Round( cost.p95Us ) << " us; " << Round( percent ) << " % of one core here, " << Round( percent * pi4Factor ) << " % on a Pi 4, " <<
                       Round( percent * piZeroFactor ) << " % on a Pi Zero 2 W; " << Round( cost.transformsPerSecond ) << " FFTs per second" << erle );
        }
        const Float64 totalUs = nsUs + defaultUs;
        const Float64 totalPercent = totalUs / 100.0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioBenchmark: noise suppressor and echo canceller (default, 48 kHz mix, render tap included) together: " << Round( totalUs ) << " us per 10 ms (render tap " <<
                   Round( defaultRenderUs ) << " us), " << Round( totalPercent ) << " % of one core here against the budget of 0.30 %, " << Round( totalPercent * pi4Factor ) << " % on a Pi 4, " <<
                   Round( totalPercent * piZeroFactor ) << " % on a Pi Zero 2 W (factors 4 and 12.8 assumed, as for the unit chain)" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
