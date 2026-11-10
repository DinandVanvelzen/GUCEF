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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOREFERENCESCENARIOS_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOREFERENCESCENARIOS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_AUDIO_CAUDIOMIXER_H
#include "gucefAUDIO_CAudioMixer.h"
#define GUCEF_AUDIO_CAUDIOMIXER_H
#endif /* GUCEF_AUDIO_CAUDIOMIXER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#include "gucefAUDIO_CNullAudioDriver.h"
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Fixed mixing scenarios whose output is reduced to a hash of the exact float bits. They only use the mixer and engine
 *  interface that existed before effect chains, sends, meters and routing tables were added, so the same code built
 *  against that earlier library gave the reference hashes the tests compare with: the output of plain bus trees
 *  must stay the same bit for bit.
 */
namespace TestAudioReferenceScenarios {

/**
 *  Hashes of the scenarios below as produced by the mixer and engine before effect chains, sends, meters and bus order
 *  tables existed: built against that library, run once, the output recorded here
 */
const GUCEF::UInt64 ReferenceMixerLimiterStereo = 0xfb22e981bd50d3cfULL;     /**< RunMixerTreeScenario( true, 2 ) */
const GUCEF::UInt64 ReferenceMixerClippingStereo = 0x8128b1ee667169afULL;    /**< RunMixerTreeScenario( false, 2 ) */
const GUCEF::UInt64 ReferenceMixerLimiterSurround = 0x43d4d3c50bad841aULL;   /**< RunMixerTreeScenario( true, 6 ) */
const GUCEF::UInt64 ReferenceEngine = 0x699454e459acdaf2ULL;                 /**< RunEngineScenario() */

inline GUCEF::UInt64
HashFloats( const GUCEF::Float32* samples, size_t count, GUCEF::UInt64 hash = 1469598103934665603ULL )
{
    for ( size_t i=0; i<count; ++i )
    {
        GUCEF::UInt32 bits = 0;
        memcpy( &bits, &samples[ i ], sizeof( bits ) );
        for ( GUCEF::UInt32 b=0; b<4; ++b )
        {
            hash ^= (GUCEF::UInt64) ( ( bits >> ( 8 * b ) ) & 0xFF );
            hash *= 1099511628211ULL;
        }
    }
    return hash;
}

/** A clip of a ramp from 'first' to 'last' on every channel */
inline GUCEF::AUDIO::TAudioClipPtr
MakeRampClip( GUCEF::UInt32 rate, GUCEF::UInt32 channels, GUCEF::UInt32 frames, GUCEF::Float32 first, GUCEF::Float32 last )
{
    GUCEF::AUDIO::CAudioBuffer audio( GUCEF::AUDIO::CAudioFormat( rate, channels, GUCEF::AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( GUCEF::UInt32 f=0; f<frames; ++f )
        for ( GUCEF::UInt32 c=0; c<channels; ++c )
            audio.SetSampleFromFloat( f, c, frames > 1 ? first + ( last - first ) * (GUCEF::Float32) f / (GUCEF::Float32) ( frames - 1 ) + 0.01f * (GUCEF::Float32) c : first );
    return GUCEF::AUDIO::TAudioClipPtr( GUCEF_NEW GUCEF::AUDIO::CAudioClip( audio ) );
}

inline GUCEF::AUDIO::TAudioVoicePtr
MakeMixerVoice( GUCEF::AUDIO::CAudioMixer& mixer, const GUCEF::AUDIO::TAudioClipPtr& clip, GUCEF::AUDIO::CAudioBus* bus )
{
    GUCEF::AUDIO::TAudioVoicePtr voice( GUCEF_NEW GUCEF::AUDIO::CAudioVoice( clip->CreateSource() ) );
    if ( GUCEF_NULL != bus )
        voice->SetBus( bus );
    mixer.PrepareVoice( voice.GetPointerAlways() );
    return voice;
}

/**
 *  A bus tree on a mixer: three buses, voices of other rates and channel counts, pans, playback rate, looping, volume
 *  ramps and mutes while it plays, a mix loud enough for the limiter (instant mode, when on) and for clipping (when off)
 */
inline GUCEF::UInt64
RunMixerTreeScenario( bool limiter, GUCEF::UInt32 channels )
{
    using namespace GUCEF;
    using namespace GUCEF::AUDIO;

    CAudioMixer mixer( CAudioFormat( 48000, channels, SAMPLETYPE_FLOAT32 ), 128, 8, 6 );
    mixer.SetLimiterMode( limiter ? AUDIOLIMITERMODE_INSTANT : AUDIOLIMITERMODE_OFF );
    CAudioBus* music = mixer.CreateBus( "music" );
    CAudioBus* effects = mixer.CreateBus( "effects" );
    CAudioBus* ui = mixer.CreateBus( "ui", effects );
    music->SetVolume( 0.8f );
    effects->SetVolume( 0.7f );
    ui->SetVolume( 1.2f );

    TAudioVoicePtr a = MakeMixerVoice( mixer, MakeRampClip( 22050, 1, 3000, -0.5f, 0.9f ), effects );
    TAudioVoicePtr b = MakeMixerVoice( mixer, MakeRampClip( 48000, 2, 900, 0.1f, 0.4f ), GUCEF_NULL );
    TAudioVoicePtr c = MakeMixerVoice( mixer, MakeRampClip( 48000, 1, 20000, 0.9f, 0.85f ), music );
    TAudioVoicePtr d = MakeMixerVoice( mixer, MakeRampClip( 44100, 1, 8000, 0.6f, -0.6f ), ui );
    a->SetPan( -0.3f );
    a->SetVolume( 0.8f );
    a->SetPlaybackRate( 1.25f );
    b->SetLoopCount( 4 );
    d->SetPan( 0.5f );
    mixer.AddVoice( a.GetPointerAlways() );
    mixer.AddVoice( b.GetPointerAlways() );
    mixer.AddVoice( c.GetPointerAlways() );
    mixer.AddVoice( d.GetPointerAlways() );

    const UInt32 total = 6000;
    const UInt32 piece = 100;
    GUCEF::vector< Float32 > out( (size_t) total * channels, 0.0f );
    for ( UInt32 p=0; p*piece<total; ++p )
    {
        if ( 10 == p ) effects->SetVolume( 0.3f );
        if ( 20 == p ) ui->SetMuted( true );
        if ( 30 == p ) ui->SetMuted( false );
        if ( 40 == p ) mixer.GetMasterBus()->SetVolume( 0.9f );
        if ( 45 == p ) c->SetPan( 0.8f );
        mixer.Mix( &out[ (size_t) p * piece * channels ], piece );
    }
    return HashFloats( &out[ 0 ], out.size() );
}

/** The same kind of tree on an engine with the null driver: buses made while it runs, clips played through Play() */
inline GUCEF::UInt64
RunEngineScenario( void )
{
    using namespace GUCEF;
    using namespace GUCEF::AUDIO;

    CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIOTHREADING_COOPERATIVE );
    params.SetLimiterMode( AUDIOLIMITERMODE_INSTANT );         // the reference was made with the instant limiter
    params.GetDeviceParams().SetBufferSizeInFrames( 256 );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );

    CAudioEngine engine;
    if ( !engine.Initialize( params ) )
        return 0;
    CNullAudioDevice* device = engine.GetDevice()->AsNullDevice();

    CAudioBus* music = engine.CreateBus( "music" );
    CAudioBus* sfx = engine.CreateBus( "sfx" );
    CAudioBus* ui = engine.CreateBus( "ui", sfx );
    music->SetVolume( 0.6f );
    sfx->SetVolume( 0.9f );
    engine.Play( MakeRampClip( 48000, 2, 30000, 0.7f, 0.2f ), music );
    engine.Play( MakeRampClip( 22050, 1, 4000, 0.9f, -0.9f ), sfx );
    engine.Play( MakeRampClip( 48000, 1, 3000, 0.5f, 0.5f ), ui );
    device->Render( 4800 );
    engine.Update();

    CAudioBus* late = engine.CreateBus( "late", music );
    late->SetVolume( 1.5f );
    engine.Play( MakeRampClip( 32000, 2, 6000, -0.8f, 0.8f ), late );
    engine.SetMasterVolume( 0.8f );
    device->Render( 4800 );
    engine.Update();

    const CAudioBuffer& rendered = device->GetRenderedAudio();
    const UInt64 hash = HashFloats( static_cast< const Float32* >( rendered.GetDataPtr() ), (size_t) rendered.GetFrameCount() * 2 );
    engine.Shutdown();
    return hash;
}

}; /* namespace TestAudioReferenceScenarios */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOREFERENCESCENARIOS_H ? */
