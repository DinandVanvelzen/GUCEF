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

#ifndef GUCEF_AUDIO_CAUDIOENGINE_H
#include "gucefAUDIO_CAudioEngine.h"
#define GUCEF_AUDIO_CAUDIOENGINE_H
#endif /* GUCEF_AUDIO_CAUDIOENGINE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H
#include "gucefAUDIO_CAudioSpatializerRegistry.h"
#define GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H
#include "gucefAUDIO_CTAudioSpatializerFactory.h"
#define GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H
#endif /* GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H ? */

#ifndef GUCEF_AUDIO_CPANNINGSPATIALIZER_H
#include "gucefAUDIO_CPanningSpatializer.h"
#define GUCEF_AUDIO_CPANNINGSPATIALIZER_H
#endif /* GUCEF_AUDIO_CPANNINGSPATIALIZER_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestAudioTestEffect.h"
#include "TestAudioSpatialVoice.h"

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

typedef AUDIO::CAudioSpatialMath::TPoint3 TPoint3;
typedef AUDIO::CAudioSpatialMath::TVector3 TVector3;
typedef GUCEF::vector< Float32 > TSamples;
typedef GUCEF::vector< Float64 > TTimes;

const Float64 Pi = 3.14159265358979323846;
const Float64 Sqrt2 = 1.41421356237309505;
const UInt32 Rate = 48000;
const UInt32 Block = 256;
const AUDIO::CAudioFormat StereoFormat( Rate, 2, AUDIO::SAMPLETYPE_FLOAT32 );
const AUDIO::CAudioFormat SurroundFormat( Rate, 6, AUDIO::SAMPLETYPE_FLOAT32 );

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

TPoint3
P( Float32 x, Float32 y, Float32 z )
{
    return TPoint3( x, y, z );
}

TVector3
V( Float32 x, Float32 y, Float32 z )
{
    return TVector3( x, y, z );
}

/** Engine on the null driver that keeps what it renders, cooperative, limiter off so the output is the plain sum */
AUDIO::CAudioEngineParams
NullParams( const AUDIO::CAudioFormat& format = StereoFormat )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( Block );
    params.GetDeviceParams().SetPreferredFormat( format );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::TAudioClipPtr
MakeClip( const TSamples& mono )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( Rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), (UInt32) mono.size() );
    for ( UInt32 f=0; f<(UInt32) mono.size(); ++f )
        audio.SetSampleFromFloat( f, 0, mono[ f ] );
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 frames, Float32 value )
{
    TSamples mono( frames, value );
    return MakeClip( mono );
}

AUDIO::TAudioClipPtr
MakeSine( UInt32 frames, Float64 frequency, Float32 amplitude )
{
    TSamples mono( frames, 0.0f );
    for ( UInt32 f=0; f<frames; ++f )
        mono[ f ] = amplitude * (Float32) sin( 2.0 * Pi * frequency * (Float64) f / (Float64) Rate );
    return MakeClip( mono );
}

/** White noise from a fixed seed xorshift, -amplitude .. amplitude */
AUDIO::TAudioClipPtr
MakeNoise( UInt32 frames, Float32 amplitude, UInt32 seed )
{
    TSamples mono( frames, 0.0f );
    UInt32 x = seed;
    for ( UInt32 f=0; f<frames; ++f )
    {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        mono[ f ] = amplitude * ( (Float32) ( x & 0xFFFFFF ) / (Float32) 0x800000 - 1.0f );
    }
    return MakeClip( mono );
}

/** A stereo clip with a constant value per channel */
AUDIO::TAudioClipPtr
MakeStereoConstant( UInt32 frames, Float32 left, Float32 right )
{
    AUDIO::CAudioBuffer audio( StereoFormat, frames );
    for ( UInt32 f=0; f<frames; ++f )
    {
        audio.SetSampleFromFloat( f, 0, left );
        audio.SetSampleFromFloat( f, 1, right );
    }
    return AUDIO::TAudioClipPtr( GUCEF_NEW AUDIO::CAudioClip( audio ) );
}

AUDIO::CNullAudioDevice*
NullDevice( AUDIO::CAudioEngine& engine )
{
    return engine.GetDevice()->AsNullDevice();
}

void
RenderBlocks( AUDIO::CAudioEngine& engine, UInt32 blocks )
{
    for ( UInt32 i=0; i<blocks; ++i )
    {
        NullDevice( engine )->Render( Block );
        engine.Update();
    }
}

/** Renders and updates until the future is ready */
bool
RenderUntilReady( AUDIO::CAudioEngine& engine, const CORE::CFutureResult& future, UInt32 maxBlocks = 50 )
{
    for ( UInt32 i=0; i<maxBlocks; ++i )
    {
        if ( future.IsReady() )
            return true;
        RenderBlocks( engine, 1 );
    }
    return future.IsReady();
}

bool
StartVoice( AUDIO::CAudioEngine& engine, const AUDIO::TAudioVoicePtr& voice )
{
    return RenderUntilReady( engine, engine.Start( voice ) );
}

bool
IsFailedWith( const CORE::CFutureResult& future, CORE::TTaskStatus status )
{
    if ( !future.HasNoFuture() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 1000 );
    return !task.IsNULL() && status == task->GetTaskStatus();
}

UInt32
RenderedFrames( AUDIO::CAudioEngine& engine )
{
    return NullDevice( engine )->GetRenderedAudio().GetFrameCount();
}

/** Sample of the rendered output, -999 beyond what was rendered */
Float32
Rendered( AUDIO::CAudioEngine& engine, UInt32 frame, UInt32 channel )
{
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    const UInt32 channels = rendered.GetFormat().GetChannelCount();
    if ( frame >= rendered.GetFrameCount() || channel >= channels )
        return -999.0f;
    return static_cast< const Float32* >( rendered.GetDataPtr() )[ (size_t) frame * channels + channel ];
}

/** The sample of a channel in the last rendered frame */
Float32
Last( AUDIO::CAudioEngine& engine, UInt32 channel )
{
    return Rendered( engine, RenderedFrames( engine ) - 1, channel );
}

void
CopyRendered( AUDIO::CAudioEngine& engine, TSamples& out )
{
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    const size_t samples = (size_t) rendered.GetFrameCount() * rendered.GetFormat().GetChannelCount();
    const Float32* data = static_cast< const Float32* >( rendered.GetDataPtr() );
    out.assign( samples, 0.0f );
    for ( size_t i=0; i<samples; ++i )
        out[ i ] = data[ i ];
}

/** Largest absolute sample of a channel from 'first' to the end of the rendered audio */
Float32
PeakFrom( AUDIO::CAudioEngine& engine, UInt32 first, UInt32 channel )
{
    Float32 peak = 0.0f;
    const UInt32 frames = RenderedFrames( engine );
    for ( UInt32 f=first; f<frames; ++f )
    {
        const Float32 x = (Float32) fabs( Rendered( engine, f, channel ) );
        if ( x > peak )
            peak = x;
    }
    return peak;
}

/** Average frequency, from the upward zero crossings of a channel between two frames */
Float64
FrequencyBetween( AUDIO::CAudioEngine& engine, UInt32 channel, UInt32 from, UInt32 to )
{
    Float64 first = -1.0, last = -1.0;
    UInt32 periods = 0;
    for ( UInt32 f=from+1; f<to; ++f )
    {
        const Float64 a = Rendered( engine, f - 1, channel );
        const Float64 b = Rendered( engine, f, channel );
        if ( a < 0.0 && b >= 0.0 )
        {
            const Float64 t = (Float64) ( f - 1 ) + a / ( a - b );
            if ( first < 0.0 )
                first = t;
            else
                ++periods;
            last = t;
        }
    }
    return periods > 0 ? (Float64) periods * (Float64) Rate / ( last - first ) : 0.0;
}

/** An attenuation that keeps the level: distance is only heard in the other parts */
AUDIO::CAudioAttenuation
NoAttenuation( void )
{
    return AUDIO::CAudioAttenuation( MATH::DISTANCEMODEL_NONE, 1.0f, 100.0f, 1.0f );
}

/** A spatializer of the tests: the mono signal at the frame gain on the first channel only */
class CTestLeftSpatializer : public AUDIO::IAudioSpatializer
{
    public:

    static const CORE::CString& GetStaticSpatializerTypeName( void )
    {
        static const CORE::CString name( "testLeft" );
        return name;
    }

    CTestLeftSpatializer( void ) : AUDIO::IAudioSpatializer(), m_channels( 0 ) {}

    virtual const CORE::CString& GetSpatializerTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return GetStaticSpatializerTypeName(); }

    virtual bool Prepare( const AUDIO::CAudioFormat& mixFormat, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_channels = mixFormat.GetChannelCount();
        return m_channels > 0 && maxBlockFrames > 0;
    }

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT {}

    virtual void Render( const Float32* mono, UInt32 frames, const AUDIO::SSpatialFrame& frame, Float32* output ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT
    {
        for ( UInt32 f=0; f<frames; ++f )
            output[ (size_t) f * m_channels ] += mono[ f ] * frame.gain;
    }

    private:

    UInt32 m_channels;
};

/** Runs a function on its own thread */
class CSpatialVoiceTestThread : public MT::CActiveObject
{
    public:

    typedef void ( *TFunction )( void* argument );

    CSpatialVoiceTestThread( TFunction function, void* argument ) : MT::CActiveObject(), m_function( function ), m_argument( argument ), m_done( false ) {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_function( m_argument );
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( ( !m_done.Load( MT::MEMORDER_ACQUIRE ) || IsActive() ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    private:

    TFunction m_function;
    void* m_argument;
    MT::CTAtomic< bool > m_done;
};

/** A game thread that moves 200 emitters and the listener once per millisecond while the test thread renders */
const UInt32 GameEmitterCount = 200;

struct SGame
{
    AUDIO::CAudioEngine* engine;
    AUDIO::TAudioEmitterPtr emitters[ GameEmitterCount ];
    MT::CTAtomic< bool > stop;
    UInt64 ticks;

    static void Run( void* argument )
    {
        SGame& g = *static_cast< SGame* >( argument );
        for ( UInt32 tick = 1; !g.stop.Load( MT::MEMORDER_ACQUIRE ); ++tick )
        {
            const Float32 t = (Float32) tick * 0.001f;
            g.engine->GetListener().SetPose( P( 0.1f * t, 0.0f, 0.0f ), V( (Float32) sin( t ), 0.0f, -(Float32) cos( t ) ), V( 0.0f, 1.0f, 0.0f ) );
            g.engine->GetListener().SetVelocity( V( 0.1f, 0.0f, 0.0f ) );
            for ( UInt32 i = 0; i < GameEmitterCount; ++i )
            {
                const Float32 a = t + (Float32) i * 0.0314f;
                g.emitters[ i ]->SetPositionAndVelocity( P( 5.0f * (Float32) cos( a ), 0.0f, 5.0f * (Float32) sin( a ) ),
                                                         V( -5.0f * (Float32) sin( a ), 0.0f, 5.0f * (Float32) cos( a ) ) );
                if ( 0 == tick % 50 )
                    g.emitters[ i ]->SetOcclusion( 0 == ( tick / 50 ) % 2 ? 0.7f : 0.0f );
            }
            ++g.ticks;
            MT::PrecisionDelay( 1 );
        }
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioSpatialVoiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.SpatialVoice" );

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: A centred spatial voice at d <= dMin equals a non-spatial voice at pan 0 within 1e-5, also with a send, and SetPan() is ignored" )
    try
    {
        const AUDIO::TAudioClipPtr clip = MakeNoise( 20000, 0.5f, 12345 );
        for ( UInt32 variant = 0; variant < 4; ++variant )
        {
            TSamples plainOut, spatialOut;
            for ( UInt32 pass = 0; pass < 2; ++pass )
            {
                const bool spatial = ( 1 == pass );
                const bool withSend = ( 3 == variant );
                AUDIO::CAudioEngine engine;
                ASSERT_TRUE( engine.Initialize( NullParams() ) );
                AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
                ASSERT_TRUE( GUCEF_NULL != aux );
                AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clip );
                if ( withSend )
                    ASSERT_TRUE( voice->SetSend( 0, aux, 0.5f ) );
                if ( spatial )
                {
                    AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
                    if ( 1 == variant )
                        ASSERT_TRUE( emitter->SetPosition( P( 0.0f, 0.0f, -0.5f ) ) );         // straight ahead, nearer than the minimum distance
                    if ( 2 == variant )
                    {
                        ASSERT_TRUE( emitter->SetListenerRelative( true ) && emitter->SetPosition( P( 0.0f, 0.0f, -0.25f ) ) );
                        ASSERT_TRUE( engine.GetListener().SetPose( P( 40.0f, 3.0f, 7.0f ), V( 0.0f, 0.0f, 1.0f ), V( 0.0f, 1.0f, 0.0f ) ) );
                    }
                    ASSERT_TRUE( voice->SetEmitter( emitter ) && voice->IsSpatial() );
                    voice->SetPan( 1.0f );          // not used by a spatial voice
                }
                ASSERT_TRUE( StartVoice( engine, voice ) );
                RenderBlocks( engine, 90 );
                CopyRendered( engine, spatial ? spatialOut : plainOut );
                if ( spatial )
                    ASSERT_TRUE( 1.0f == voice->GetDopplerRatio() );
                engine.Shutdown();
            }

            ASSERT_TRUE( plainOut.size() == spatialOut.size() && plainOut.size() > 40000 );
            Float64 largest = 0.0, energy = 0.0;
            for ( size_t i=0; i<plainOut.size(); ++i )
            {
                largest = fabs( plainOut[ i ] - spatialOut[ i ] ) > largest ? fabs( plainOut[ i ] - spatialOut[ i ] ) : largest;
                energy += plainOut[ i ] * plainOut[ i ];
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: centred variant " << variant << ": largest difference " << largest );
            ASSERT_TRUE( energy > 100.0 );
            ASSERT_TRUE( largest <= 1.0e-5 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: The working signal of a spatial voice is mono: a stereo source is mixed down and a voice effect runs on one channel" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeStereoConstant( 48000, 0.4f, 0.0f ) );
        AUDIO::TAudioEffectPtr effect( GUCEF_NEW CTestAudioEffect() );
        ASSERT_TRUE( voice->AddEffect( effect ) );
        ASSERT_TRUE( voice->SetEmitter( engine.CreateEmitter() ) );
        ASSERT_TRUE( StartVoice( engine, voice ) );
        RenderBlocks( engine, 10 );
        ASSERT_TRUE( 1 == effect->GetFormat().GetChannelCount() );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.2, 1.0e-6 ) && Near( Last( engine, 1 ), 0.2, 1.0e-6 ) );     // ( 0.4 + 0 ) / 2, centred
        ASSERT_TRUE( !voice->GetSpatializer().IsNULL() && AUDIO::CPanningSpatializer::TypeName == voice->GetSpatializer()->GetSpatializerTypeName() );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: The listener frame is heard: a source at +X is right, left when the listener turned round, listener relative emitters ignore the listener, the 2D mode pans from x" )
    try
    {
        // 0: +X with the default listener, 1: listener facing +Z, 2: listener relative at +X with a moved and turned listener, 3: 2D at -panWidth with a far z
        const Float32 expectedLeft[ 4 ] = { 0.0f, 1.0f, 0.0f, 1.0f };
        for ( UInt32 variant = 0; variant < 4; ++variant )
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 48000, 0.25f ) );
            Float32 level = 0.25f * (Float32) Sqrt2;
            if ( variant < 3 )
                ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 5.0f, 0.0f, 0.0f ) ) );
            if ( 1 == variant )
                ASSERT_TRUE( engine.GetListener().SetOrientation( V( 0.0f, 0.0f, 1.0f ), V( 0.0f, 1.0f, 0.0f ) ) );
            if ( 2 == variant )
            {
                ASSERT_TRUE( emitter->SetListenerRelative( true ) );
                ASSERT_TRUE( engine.GetListener().SetPose( P( 100.0f, 0.0f, 0.0f ), V( 0.0f, 0.0f, 1.0f ), V( 0.0f, 1.0f, 0.0f ) ) );
            }
            if ( 3 == variant )
            {
                // the default attenuation, inverse with minimum distance 1: the distance 10 in the plane gives 0.1, z does not count
                ASSERT_TRUE( emitter->SetMode( AUDIO::AUDIOSPATIAL_2D ) && emitter->SetPosition( P( -10.0f, 0.0f, 500.0f ) ) );
                ASSERT_TRUE( engine.GetListener().Set2DPanWidth( 10.0f ) );
                level *= 0.1f;
            }
            ASSERT_TRUE( voice->SetEmitter( emitter ) );
            ASSERT_TRUE( StartVoice( engine, voice ) );
            RenderBlocks( engine, 5 );
            const Float32 left = Last( engine, 0 );
            const Float32 right = Last( engine, 1 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: frame variant " << variant << ": left " << left << " right " << right );
            ASSERT_TRUE( Near( left, expectedLeft[ variant ] * level, 1.0e-5 ) );
            ASSERT_TRUE( Near( right, ( 1.0f - expectedLeft[ variant ] ) * level, 1.0e-5 ) );
            engine.Shutdown();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: Distance, cone and occlusion gains reach the direct path; obstruction and cone do not reach the sends, occlusion and distance do" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::CAudioBus* dry = engine.CreateBus( "dry" );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );
        ASSERT_TRUE( GUCEF_NULL != dry && GUCEF_NULL != aux );

        // a direct voice, and one heard only through its send (its own bus is silent)
        AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
        ASSERT_TRUE( emitter->SetPosition( P( 0.0f, 0.0f, -2.0f ) ) );                // inverse, minimum 1: gain 0.5
        AUDIO::TAudioVoicePtr direct = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
        AUDIO::TAudioVoicePtr sent = engine.CreateVoice( MakeConstant( 4800, 0.25f ), dry );
        direct->SetLoopCount( 0 );
        sent->SetLoopCount( 0 );
        ASSERT_TRUE( sent->SetSend( 0, aux, 1.0f ) );
        ASSERT_TRUE( direct->SetEmitter( emitter ) );
        AUDIO::TAudioEmitterPtr sentEmitter = engine.CreateEmitter();
        ASSERT_TRUE( sentEmitter->SetPosition( P( 0.0f, 0.0f, -2.0f ) ) );
        ASSERT_TRUE( sent->SetEmitter( sentEmitter ) );
        ASSERT_TRUE( StartVoice( engine, direct ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.125, 1.0e-6 ) && Near( Last( engine, 1 ), 0.125, 1.0e-6 ) );
        direct->RequestStop();
        RenderBlocks( engine, 3 );

        dry->SetVolume( 0.0f );
        ASSERT_TRUE( StartVoice( engine, sent ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.125, 1.0e-6 ) );                     // the send follows the distance

        ASSERT_TRUE( sentEmitter->SetSendsFollowAttenuation( false ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.25, 1.0e-6 ) );
        ASSERT_TRUE( sentEmitter->SetSendsFollowAttenuation( true ) );

        // obstruction and the cone: the direct path only, the send keeps its level
        ASSERT_TRUE( sentEmitter->SetObstruction( 1.0f ) && sentEmitter->SetDirection( V( 0.0f, 0.0f, -1.0f ) ) && sentEmitter->SetCone( 90.0f, 180.0f, 0.25f, 0.0f ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.125, 1.0e-6 ) );

        // occlusion reaches the send: -12 dB
        ASSERT_TRUE( sentEmitter->SetObstruction( 0.0f ) && sentEmitter->SetOcclusion( 1.0f ) );
        RenderBlocks( engine, 20 );
        const Float64 occluded = 0.125 * pow( 10.0, -12.0 / 20.0 );
        ASSERT_TRUE( Near( Last( engine, 0 ), occluded, 1.0e-5 ) );            // a constant passes the low pass unchanged
        sent->RequestStop();
        RenderBlocks( engine, 3 );

        // the direct path with the cone pointing away (0.25), obstruction (-12 dB) and both
        dry->SetVolume( 1.0f );
        AUDIO::TAudioEmitterPtr third = engine.CreateEmitter();
        ASSERT_TRUE( third->SetPosition( P( 0.0f, 0.0f, -2.0f ) ) && third->SetDirection( V( 0.0f, 0.0f, -1.0f ) ) && third->SetCone( 90.0f, 180.0f, 0.25f, 0.0f ) );
        AUDIO::TAudioVoicePtr coned = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
        coned->SetLoopCount( 0 );
        ASSERT_TRUE( coned->SetEmitter( third ) && StartVoice( engine, coned ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.125 * 0.25, 1.0e-6 ) );
        ASSERT_TRUE( third->SetObstruction( 1.0f ) );
        RenderBlocks( engine, 20 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.125 * 0.25 * pow( 10.0, -12.0 / 20.0 ), 1.0e-5 ) );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: The low pass of occlusion, cone and distance cuts high frequencies, keeps low ones and is left out at 20 kHz" )
    try
    {
        // 0: occlusion 1 at 1 kHz, 1: the outer cone at 1 kHz, 2: the distance low pass of 1 kHz at the maximum distance
        for ( UInt32 variant = 0; variant < 3; ++variant )
        {
            Float32 levels[ 2 ] = { 0.0f, 0.0f };
            const Float64 frequencies[ 2 ] = { 100.0, 8000.0 };
            for ( UInt32 f = 0; f < 2; ++f )
            {
                AUDIO::CAudioEngine engine;
                ASSERT_TRUE( engine.Initialize( NullParams() ) );
                AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
                ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 0.0f, 0.0f, -5.0f ) ) );
                if ( 0 == variant )
                    ASSERT_TRUE( emitter->SetOcclusionResponse( 0.0f, 1000.0f ) && emitter->SetOcclusion( 1.0f ) );
                if ( 1 == variant )
                    ASSERT_TRUE( emitter->SetDirection( V( 0.0f, 0.0f, -1.0f ) ) && emitter->SetCone( 90.0f, 180.0f, 1.0f, 1000.0f ) );
                if ( 2 == variant )
                    ASSERT_TRUE( emitter->SetPosition( P( 0.0f, 0.0f, -100.0f ) ) && emitter->SetDistanceLowPass( 1000.0f ) );
                AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 48000, frequencies[ f ], 0.5f ) );
                ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
                RenderBlocks( engine, 60 );
                levels[ f ] = PeakFrom( engine, RenderedFrames( engine ) - 4800, 0 );
                engine.Shutdown();
            }
            const Float64 lowDb = 20.0 * log10( levels[ 0 ] / 0.5 );
            const Float64 highDb = 20.0 * log10( levels[ 1 ] / 0.5 + 1.0e-12 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: low pass variant " << variant << ": 100 Hz " << lowDb << " dB, 8 kHz " << highDb << " dB" );
            ASSERT_TRUE( Near( lowDb, 0.0, 0.2 ) );
            ASSERT_TRUE( highDb < -30.0 );
        }

        // nearer than the minimum distance the distance low pass is at 20 kHz: left out, the output is that of no low pass at all
        TSamples outputs[ 2 ];
        for ( UInt32 pass = 0; pass < 2; ++pass )
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
            ASSERT_TRUE( emitter->SetPosition( P( 0.0f, 0.0f, -0.5f ) ) );
            if ( 1 == pass )
                ASSERT_TRUE( emitter->SetDistanceLowPass( 500.0f ) );
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeNoise( 9600, 0.5f, 99 ) );
            ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
            RenderBlocks( engine, 20 );
            CopyRendered( engine, outputs[ pass ] );
            engine.Shutdown();
        }
        ASSERT_TRUE( outputs[ 0 ].size() == outputs[ 1 ].size() );
        ASSERT_TRUE( 0 == memcmp( &outputs[ 0 ][ 0 ], &outputs[ 1 ][ 0 ], outputs[ 0 ].size() * sizeof( Float32 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: Doppler shifts a sine voice in the output: approaching source, approaching listener, ratio limit, factor 0 (zero crossings within 0.5 percent)" )
    try
    {
        const Float64 c = 343.3;
        const Float64 expected[ 4 ] = { c / ( c - 34.33 ), ( c + 34.33 ) / c, 2.0, 1.0 };
        for ( UInt32 variant = 0; variant < 4; ++variant )
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            if ( 3 == variant )
            {
                AUDIO::CAudioSpatialParams world;
                world.SetDopplerFactor( 0.0f );
                engine.SetSpatialParams( world );
            }
            AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
            ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 0.0f, 0.0f, -100.0f ) ) );
            if ( 0 == variant || 3 == variant )
                ASSERT_TRUE( emitter->SetVelocity( V( 0.0f, 0.0f, 34.33f ) ) );          // toward the listener
            if ( 1 == variant )
                ASSERT_TRUE( engine.GetListener().SetVelocity( V( 0.0f, 0.0f, -34.33f ) ) );   // toward the source
            if ( 2 == variant )
                ASSERT_TRUE( emitter->SetVelocity( V( 0.0f, 0.0f, 300.0f ) ) );          // 7.9 limited to 2
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 96000, 1000.0, 0.5f ) );
            ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
            RenderBlocks( engine, 150 );
            const UInt32 end = RenderedFrames( engine );
            const Float64 frequency = FrequencyBetween( engine, 0, end - 24000, end );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: Doppler variant " << variant << ": " << frequency << " Hz, expected " << 1000.0 * expected[ variant ] << ", ratio " << voice->GetDopplerRatio() );
            ASSERT_TRUE( Near( frequency, 1000.0 * expected[ variant ], 5.0 * expected[ variant ] ) );
            ASSERT_TRUE( Near( voice->GetDopplerRatio(), expected[ variant ], 1.0e-4 ) );
            engine.Shutdown();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: The Doppler ratio is smoothed from block to block with the time constant of the world settings" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
        ASSERT_TRUE( emitter->SetPosition( P( 0.0f, 0.0f, -10.0f ) ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeSine( 4800, 500.0, 0.5f ) );
        voice->SetLoopCount( 0 );
        ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
        RenderBlocks( engine, 2 );
        ASSERT_TRUE( 1.0f == voice->GetDopplerRatio() );

        const Float64 target = 343.3 / ( 343.3 - 34.33 );
        ASSERT_TRUE( emitter->SetVelocity( V( 0.0f, 0.0f, 34.33f ) ) );
        RenderBlocks( engine, 1 );
        const Float64 oneBlock = 1.0 + ( target - 1.0 ) * ( 1.0 - exp( -( (Float64) Block / (Float64) Rate ) / 0.050 ) );
        ASSERT_TRUE( Near( voice->GetDopplerRatio(), oneBlock, 1.0e-4 ) );
        RenderBlocks( engine, 100 );
        ASSERT_TRUE( Near( voice->GetDopplerRatio(), target, 1.0e-4 ) );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: An emitter moving round the listener gives continuous gains (bounded first and second difference)" )
    try
    {
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
        ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 0.0f, 0.0f, -3.0f ) ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
        voice->SetLoopCount( 0 );
        ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
        RenderBlocks( engine, 2 );
        const UInt32 first = RenderedFrames( engine );
        for ( UInt32 b = 0; b < 190; ++b )
        {
            // one turn per second
            const Float64 angle = 2.0 * Pi * (Float64) ( b * Block ) / (Float64) Rate;
            ASSERT_TRUE( emitter->SetPosition( P( 3.0f * (Float32) sin( angle ), 0.0f, -3.0f * (Float32) cos( angle ) ) ) );
            RenderBlocks( engine, 1 );
        }
        Float64 largestStep = 0.0, largestBend = 0.0;
        for ( UInt32 ch = 0; ch < 2; ++ch )
        {
            for ( UInt32 f = first + 2; f < RenderedFrames( engine ); ++f )
            {
                const Float64 a = Rendered( engine, f - 2, ch ), b = Rendered( engine, f - 1, ch ), x = Rendered( engine, f, ch );
                largestStep = fabs( x - b ) > largestStep ? fabs( x - b ) : largestStep;
                largestBend = fabs( x - 2.0 * b + a ) > largestBend ? fabs( x - 2.0 * b + a ) : largestBend;
            }
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: moving emitter: largest step " << largestStep << ", largest second difference " << largestBend );
        ASSERT_TRUE( largestStep < 1.0e-3 );
        ASSERT_TRUE( largestBend < 1.0e-4 );
        ASSERT_TRUE( PeakFrom( engine, first, 0 ) > 0.3f && PeakFrom( engine, first, 1 ) > 0.3f );      // it went round: each side reached full level ( 0.25 sqrt( 2 ) )
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: Surround output: ahead is the centre speaker, without the centre the front pair, the LFE send level, and a reopen from stereo to 5.1 keeps the voice spatial" )
    try
    {
        AUDIO::CAudioPanner speakers;
        ASSERT_TRUE( speakers.Prepare( SurroundFormat, true ) );
        Int32 centre = -1, frontLeft = -1, frontRight = -1;
        for ( UInt32 ch = 0; ch < 6; ++ch )
        {
            if ( !speakers.IsRingChannel( ch ) )
                continue;
            const Float32 degrees = speakers.GetChannelAzimuthInDegrees( ch );
            if ( 0.0f == degrees ) centre = (Int32) ch;
            if ( -30.0f == degrees ) frontLeft = (Int32) ch;
            if ( 30.0f == degrees ) frontRight = (Int32) ch;
        }
        const Int32 lfe = speakers.GetLfeChannel();
        ASSERT_TRUE( centre >= 0 && frontLeft >= 0 && frontRight >= 0 && lfe >= 0 );

        // starts on stereo, goes to 5.1 by a reopen
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( NullParams() ) );
        AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
        ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 0.0f, 0.0f, -5.0f ) ) );
        AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
        voice->SetLoopCount( 0 );
        ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, 0 ), 0.25, 1.0e-6 ) && Near( Last( engine, 1 ), 0.25, 1.0e-6 ) );

        ASSERT_TRUE( engine.ReopenDevice( NullParams( SurroundFormat ) ) );
        ASSERT_TRUE( 6 == engine.GetMixFormat().GetChannelCount() );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, centre ), 0.25 * Sqrt2, 1.0e-5 ) );
        ASSERT_TRUE( Near( Last( engine, frontLeft ), 0.0, 1.0e-6 ) && Near( Last( engine, frontRight ), 0.0, 1.0e-6 ) && Near( Last( engine, lfe ), 0.0, 1.0e-9 ) );

        // no centre speaker, and an LFE send of -6 dB
        AUDIO::CAudioSpatialParams world = engine.GetSpatialParams();
        world.SetUseCenterSpeaker( false );
        world.SetLfeSendInDecibels( -6.0f );
        engine.SetSpatialParams( world );
        RenderBlocks( engine, 3 );
        ASSERT_TRUE( Near( Last( engine, centre ), 0.0, 1.0e-6 ) );
        ASSERT_TRUE( Near( Last( engine, frontLeft ), 0.25, 1.0e-5 ) && Near( Last( engine, frontRight ), 0.25, 1.0e-5 ) );
        ASSERT_TRUE( Near( Last( engine, lfe ), 0.25 * pow( 10.0, -6.0 / 20.0 ), 1.0e-5 ) );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: Spatializers come from the registry: panning by default, another by name from the world settings, an unknown name refuses the start" )
    try
    {
        AUDIO::CAudioSpatializerRegistry& registry = AUDIO::CAudioGlobal::Instance()->GetAudioSpatializerRegistry();
        AUDIO::CAudioSpatializerRegistry::TRegisteredFactoryPtr found;
        ASSERT_TRUE( registry.TryFindFactory( "PANNING", found ) && !found.IsNULL() );
        AUDIO::TAudioSpatializerPtr made = registry.CreateSpatializer( "panning" );
        ASSERT_TRUE( !made.IsNULL() && "panning" == made->GetSpatializerTypeName() && made->GetFactory() == found.GetPointerAlways() );
        ASSERT_TRUE( registry.CreateSpatializer( "nothing" ).IsNULL() );
        ASSERT_FALSE( registry.RegisterFactory( AUDIO::TAudioSpatializerFactoryPtr( GUCEF_NEW AUDIO::CTAudioSpatializerFactory< AUDIO::CPanningSpatializer >() ) ) );

        AUDIO::TAudioSpatializerFactoryPtr factory( GUCEF_NEW AUDIO::CTAudioSpatializerFactory< CTestLeftSpatializer >() );
        ASSERT_TRUE( registry.RegisterFactory( factory ) );
        {
            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            AUDIO::CAudioSpatialParams world = engine.GetSpatialParams();
            world.SetSpatializerName( "testleft" );
            engine.SetSpatialParams( world );
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
            voice->SetLoopCount( 0 );
            ASSERT_TRUE( voice->SetEmitter( engine.CreateEmitter() ) && StartVoice( engine, voice ) );
            RenderBlocks( engine, 3 );
            ASSERT_TRUE( Near( Last( engine, 0 ), 0.25, 1.0e-6 ) && 0.0f == Last( engine, 1 ) );
            ASSERT_TRUE( "testLeft" == voice->GetSpatializer()->GetSpatializerTypeName() && 1 == factory->GetLiveInstanceCount() );
            ASSERT_FALSE( registry.TryUnregisterFactory( "testLeft" ) );        // one is alive

            world.SetSpatializerName( "nothing" );
            engine.SetSpatialParams( world );
            AUDIO::TAudioVoicePtr refused = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
            ASSERT_TRUE( refused->SetEmitter( engine.CreateEmitter() ) );
            ASSERT_TRUE( IsFailedWith( engine.Start( refused ), CORE::TASKSTATUS_SETUP_FAILED ) );

            // a voice without an emitter needs no spatializer
            AUDIO::TAudioVoicePtr plain = engine.CreateVoice( MakeConstant( 4800, 0.25f ) );
            ASSERT_TRUE( StartVoice( engine, plain ) && plain->GetSpatializer().IsNULL() && !plain->IsSpatial() );

            // emitter and spatializer are set before the start only
            ASSERT_FALSE( voice->SetEmitter( AUDIO::TAudioEmitterPtr() ) );
            ASSERT_FALSE( voice->SetSpatializer( registry.CreateSpatializer( "panning" ) ) );
            voice->RequestStop();
            RenderBlocks( engine, 3 );
            engine.Shutdown();
        }
        ASSERT_TRUE( 0 == factory->GetLiveInstanceCount() );
        ASSERT_TRUE( registry.TryUnregisterFactory( "testLeft" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: A mixer on its own makes the panning spatializer and places spatial voices with the scene it is given" )
    try
    {
        AUDIO::CAudioMixer mixer( StereoFormat, 128, 4, 2 );
        AUDIO::TAudioVoicePtr voice( GUCEF_NEW AUDIO::CAudioVoice( MakeConstant( 4800, 0.25f )->CreateSource() ) );
        AUDIO::TAudioEmitterPtr emitter( GUCEF_NEW AUDIO::CAudioEmitter() );
        ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && emitter->SetPosition( P( 5.0f, 0.0f, 0.0f ) ) );
        ASSERT_TRUE( voice->SetEmitter( emitter ) );
        ASSERT_TRUE( mixer.PrepareVoice( voice.GetPointerAlways() ) && mixer.AddVoice( voice.GetPointerAlways() ) );
        ASSERT_TRUE( !voice->GetSpatializer().IsNULL() );

        TSamples out( 256 * 2, 0.0f );
        mixer.Mix( &out[ 0 ], 256 );
        ASSERT_TRUE( Near( out[ 255 * 2 ], 0.0, 1.0e-6 ) && Near( out[ 255 * 2 + 1 ], 0.25 * Sqrt2, 1.0e-5 ) );       // right

        AUDIO::SListenerState turned;
        turned.forward = V( 0.0f, 0.0f, 1.0f );
        mixer.SetSpatialScene( turned, mixer.GetSpatialWorld() );
        mixer.Mix( &out[ 0 ], 256 );
        ASSERT_TRUE( Near( out[ 255 * 2 ], 0.25 * Sqrt2, 1.0e-5 ) && Near( out[ 255 * 2 + 1 ], 0.0, 1.0e-6 ) );       // left once the listener turned
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: An emitter that derives its velocity measures it from successive positions" )
    try
    {
        AUDIO::CAudioEmitter emitter;
        ASSERT_TRUE( emitter.SetVelocity( V( 1.0f, 2.0f, 3.0f ) ) && emitter.SetDeriveVelocity( true ) );
        AUDIO::SEmitterState state;
        emitter.GetState( state );
        ASSERT_TRUE( state.deriveVelocity && state.velocity == V( 1.0f, 2.0f, 3.0f ) );      // nothing measured yet

        // 2 units along x per step of about 30 ms: bounded by the times taken around each call
        const UInt64 resolution = MT::PrecisionTimerResolution();
        UInt64 before = MT::PrecisionTickCount();
        ASSERT_TRUE( emitter.SetPosition( P( 0.0f, 0.0f, 0.0f ) ) );
        UInt64 after = MT::PrecisionTickCount();
        for ( UInt32 step = 1; step <= 3; ++step )
        {
            MT::PrecisionDelay( 30 );
            const UInt64 previousBefore = before, previousAfter = after;
            before = MT::PrecisionTickCount();
            ASSERT_TRUE( emitter.SetPosition( P( 2.0f * (Float32) step, 0.0f, 0.0f ) ) );
            after = MT::PrecisionTickCount();
            emitter.GetState( state );
            const Float64 slowest = 2.0 / ( (Float64) ( after - previousBefore ) / (Float64) resolution );
            const Float64 fastest = 2.0 / ( (Float64) ( before - previousAfter ) / (Float64) resolution );
            ASSERT_TRUE( state.velocity.x >= slowest * 0.999 && state.velocity.x <= fastest * 1.001 && 0.0f == state.velocity.y && 0.0f == state.velocity.z );
        }

        // standing still: back to 0; an update within a millisecond keeps the velocity
        MT::PrecisionDelay( 5 );
        ASSERT_TRUE( emitter.SetPosition( P( 6.0f, 0.0f, 0.0f ) ) );
        emitter.GetState( state );
        ASSERT_TRUE( state.velocity == V( 0.0f, 0.0f, 0.0f ) );
        ASSERT_TRUE( emitter.SetPosition( P( 7.0f, 0.0f, 0.0f ) ) );
        emitter.GetState( state );
        ASSERT_TRUE( state.velocity == V( 0.0f, 0.0f, 0.0f ) && state.position == P( 7.0f, 0.0f, 0.0f ) );

        // off: the velocity is the game's again
        ASSERT_TRUE( emitter.SetDeriveVelocity( false ) && emitter.SetVelocity( V( 0.0f, 9.0f, 0.0f ) ) );
        MT::PrecisionDelay( 5 );
        ASSERT_TRUE( emitter.SetPosition( P( 50.0f, 0.0f, 0.0f ) ) );
        emitter.GetState( state );
        ASSERT_TRUE( state.velocity == V( 0.0f, 9.0f, 0.0f ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioSpatialVoice: A game thread updating 200 emitters and the listener at 1 kHz while 200 spatial voices render (TSan and RTSan run)" )
    try
    {
        AUDIO::CAudioEngineParams params = NullParams();
        params.SetMaxVoices( 256 );
        params.SetCommandQueueSize( 1024 );
        params.SetEventQueueSize( 1024 );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );
        AUDIO::CAudioBus* aux = engine.CreateBus( "aux" );

        SGame game;
        game.engine = &engine;
        game.stop.Store( false );
        game.ticks = 0;
        const AUDIO::TAudioClipPtr clip = MakeSine( 4800, 440.0, 0.002f );
        GUCEF::vector< AUDIO::TAudioVoicePtr > voices;
        for ( UInt32 i = 0; i < GameEmitterCount; ++i )
        {
            game.emitters[ i ] = engine.CreateEmitter();
            if ( 0 == i % 3 )
                game.emitters[ i ]->SetDistanceLowPass( 2000.0f );
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( clip );
            voice->SetLoopCount( 0 );
            if ( 0 == i % 2 )
                voice->SetSend( 0, aux, 0.5f );
            voice->SetEmitter( game.emitters[ i ] );
            ASSERT_TRUE( !engine.Start( voice ).HasNoFuture() );
            voices.push_back( voice );
        }
        RenderBlocks( engine, 4 );
        ASSERT_TRUE( GameEmitterCount == engine.GetActiveVoiceCount() );

        CSpatialVoiceTestThread thread( &SGame::Run, &game );
        ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
        const UInt64 start = MT::PrecisionTickCount();
        UInt32 blocks = 0;
        while ( (Float64) ( MT::PrecisionTickCount() - start ) / (Float64) MT::PrecisionTimerResolution() < 1.0 || blocks < 50 )
        {
            RenderBlocks( engine, 1 );
            ++blocks;
        }
        game.stop.Store( true, MT::MEMORDER_RELEASE );
        ASSERT_TRUE( thread.WaitUntilDone( 60000 ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioSpatialVoice: game thread: " << game.ticks << " updates while " << blocks << " blocks rendered" );
        ASSERT_TRUE( game.ticks > 10 );
        ASSERT_TRUE( GameEmitterCount == engine.GetActiveVoiceCount() );

        // the output is finite and not silent
        const UInt32 frames = RenderedFrames( engine );
        bool finite = true;
        for ( UInt32 f = 0; f < frames; ++f )
            for ( UInt32 ch = 0; ch < 2; ++ch )
                finite = finite && Rendered( engine, f, ch ) == Rendered( engine, f, ch );
        ASSERT_TRUE( finite && PeakFrom( engine, frames - 4800, 0 ) > 0.0f );
        for ( size_t v = 0; v < voices.size(); ++v )
            voices[ v ]->RequestStop();
        RenderBlocks( engine, 3 );
        engine.Shutdown();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
