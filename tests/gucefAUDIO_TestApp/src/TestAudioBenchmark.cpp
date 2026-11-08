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

#include "TestAudioCodecSupport.h"
#include "TestAudioCodecMp3Support.h"
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
EffectBenchmark( AUDIO::CAudioEngine& engine, const char* type, const CORE::CValueList& options, UInt32 channels, const char* what, Float64 budgetUs, const char* parameterNames[], Float32 parameterValues[] )
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
}

/*-------------------------------------------------------------------------*/
