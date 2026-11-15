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

#include "gucefAUDIO.h"
#include "gucefAUDIO_CAudioEngine.h"
#include "gucefAUDIO_CAudioGlobal.h"
#include "gucefAUDIO_SpatialSnapshotAdapters.h"

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#ifndef GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#include "gucefGEOMETRY_CTAudioOcclusionEstimator.h"
#define GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#endif /* GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H ? */

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

#include "TestAudioGeoSpace.h"

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
typedef GEOMETRY::CTGeoSpace< Float32 > TNode;
typedef GEOMETRY::CTGeoSpaceSnapshot< Float32 > TSnapshot;

const Float64 Pi = 3.14159265358979323846;
const Float64 Sqrt2 = 1.41421356237309505;
const UInt32 Rate = 48000;
const UInt32 Block = 256;
const AUDIO::CAudioFormat StereoFormat( Rate, 2, AUDIO::SAMPLETYPE_FLOAT32 );

bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

MATH::CTTransform3D< Float32 >
Placement( Float32 x, Float32 y, Float32 z, Float64 yawInRadians )
{
    const MATH::CTQuaternion< Float32 > rotation = MATH::CTQuaternion< Float32 >::FromAxisAngle( MATH::CTDirectionVector3D< Float32 >::UnitY(), MATH::CTRadian< Float32 >( (Float32) yawInRadians ) );
    return MATH::CTTransform3D< Float32 >( MATH::CTVector3D< Float32 >( x, y, z ), rotation, MATH::CTVector3D< Float32 >( 1.0f, 1.0f, 1.0f ) );
}

AUDIO::CAudioEngineParams
NullParams( void )
{
    AUDIO::CAudioEngineParams params;
    params.SetDriverName( "null" );
    params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
    params.SetUpdateIntervalInMs( 1 );
    params.SetLimiterEnabled( false );
    params.GetDeviceParams().SetBufferSizeInFrames( Block );
    params.GetDeviceParams().SetPreferredFormat( StereoFormat );
    params.GetDeviceParams().GetOptions().Set( "keepOutput", "true" );
    return params;
}

AUDIO::TAudioClipPtr
MakeConstant( UInt32 frames, Float32 value )
{
    AUDIO::CAudioBuffer audio( AUDIO::CAudioFormat( Rate, 1, AUDIO::SAMPLETYPE_FLOAT32 ), frames );
    for ( UInt32 f=0; f<frames; ++f )
        audio.SetSampleFromFloat( f, 0, value );
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

bool
StartVoice( AUDIO::CAudioEngine& engine, const AUDIO::TAudioVoicePtr& voice )
{
    CORE::CFutureResult future = engine.Start( voice );
    for ( UInt32 i=0; i<50; ++i )
    {
        if ( future.IsReady() )
            return true;
        RenderBlocks( engine, 1 );
    }
    return future.IsReady();
}

/** The sample of a channel in the last rendered frame */
Float32
Last( AUDIO::CAudioEngine& engine, UInt32 channel )
{
    const AUDIO::CAudioBuffer& rendered = NullDevice( engine )->GetRenderedAudio();
    const UInt32 channels = rendered.GetFormat().GetChannelCount();
    if ( 0 == rendered.GetFrameCount() || channel >= channels )
        return -999.0f;
    return static_cast< const Float32* >( rendered.GetDataPtr() )[ (size_t) ( rendered.GetFrameCount() - 1 ) * channels + channel ];
}

AUDIO::CAudioAttenuation
NoAttenuation( void )
{
    return AUDIO::CAudioAttenuation( MATH::DISTANCEMODEL_NONE, 1.0f, 100.0f, 1.0f );
}

bool
RtReadListener( AUDIO::CAudioListener& listener, AUDIO::SListenerState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return listener.ReadState( state );
}

bool
RtReadEmitter( AUDIO::CAudioEmitter& emitter, AUDIO::SEmitterState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    return emitter.ReadState( state );
}

/*-------------------------------------------------------------------------*/

/**
 *  A listener on the camera node of a small tree: a player node at a position and a yaw, the camera a child of it. One
 *  emitter node at 5 units to the right of the player's start. Renders the voice and returns the output of the channels.
 */
bool
HearFromTree( Float64 playerYaw, Float32& left, Float32& right, Float32 playerX )
{
    TNode player( Placement( playerX, 0.0f, 0.0f, playerYaw ) );
    TNode camera( Placement( 0.0f, 0.0f, 0.0f, 0.0 ) );
    TNode siren( Placement( playerX + 5.0f, 0.0f, 0.0f, 0.0 ) );
    if ( !camera.SetParent( &player, false ) )
        return false;

    AUDIO::CAudioEngine engine;
    if ( !engine.Initialize( NullParams() ) )
        return false;
    AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
    AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 48000, 0.25f ) );

    // the owner thread of the tree captures one snapshot per node and hands it over: the listener and the emitter publish it
    TSnapshot cameraSnapshot;
    TSnapshot sirenSnapshot;
    camera.CaptureSnapshot( cameraSnapshot, MATH::CCoordinateSystem::GucefDefault() );
    siren.CaptureSnapshot( sirenSnapshot, MATH::CCoordinateSystem::GucefDefault() );
    bool ok = emitter->SetAttenuation( NoAttenuation() ) && AUDIO::SetEmitterFromSnapshot( *emitter, sirenSnapshot ) &&
              AUDIO::SetListenerFromSnapshot( engine.GetListener(), cameraSnapshot ) && voice->SetEmitter( emitter ) && StartVoice( engine, voice );
    RenderBlocks( engine, 5 );
    left = Last( engine, 0 );
    right = Last( engine, 1 );
    engine.Shutdown();
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The writer: an owner thread that moves a camera node and hands snapshots over, each tick one whole pose */
class CCameraOwnerThread : public MT::CActiveObject
{
    public:

    CCameraOwnerThread( AUDIO::CAudioListener& listener, AUDIO::CAudioEmitter& emitter, UInt32 ticks )
        : MT::CActiveObject(), m_listener( listener ), m_emitter( emitter ), m_ticks( ticks ), m_done( false ), m_refused( 0 ) {}

    virtual bool OnThreadStart( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* /*taskdata*/ ) GUCEF_VIRTUAL_OVERRIDE
    {
        TNode player( Placement( 0.0f, 0.0f, 0.0f, 0.0 ) );
        TNode camera( Placement( 0.0f, 1.7f, 0.0f, 0.0 ) );
        camera.SetParent( &player, false );
        TSnapshot snapshot;
        for ( UInt32 i=1; i<=m_ticks; ++i )
        {
            // the position encodes the tick, the yaw is the tick / 1000 radians: a reader that sees both from one tick can check them against each other
            player.SetLocalTransform( Placement( (Float32) i, 0.0f, 0.0f, (Float64) i * 0.001 ) );
            camera.CaptureSnapshot( snapshot, MATH::CCoordinateSystem::GucefDefault(), MATH::CTVector3D< Float32 >( (Float32) i, 0.0f, 0.0f ) );
            if ( !AUDIO::SetListenerFromSnapshot( m_listener, snapshot ) )
                ++m_refused;
            if ( !AUDIO::SetEmitterFromSnapshot( m_emitter, snapshot, true ) )
                ++m_refused;
        }
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load( MT::MEMORDER_ACQUIRE ); }
    UInt32 GetRefused( void ) const { return m_refused; }

    private:

    AUDIO::CAudioListener& m_listener;
    AUDIO::CAudioEmitter& m_emitter;
    UInt32 m_ticks;
    MT::CTAtomic< bool > m_done;
    UInt32 m_refused;
};

/*-------------------------------------------------------------------------*/

/** True when the pose of a read state is the one of a single tick of the writer */
bool
IsOneTick( const AUDIO::SListenerState& state )
{
    const Float64 tick = (Float64) state.position.x;
    const Float64 yaw = tick * 0.001;
    // forward of a camera turned by 'yaw' about +Y is ( -sin yaw, 0, -cos yaw ); up stays +Y; the velocity was the tick as well
    return Near( state.forward.x, -sin( yaw ), 2.0e-5 ) && Near( state.forward.z, -cos( yaw ), 2.0e-5 ) && Near( state.up.y, 1.0, 1.0e-5 ) &&
           Near( state.velocity.x, tick, 1.0e-3 ) && Near( state.position.y, 1.7, 1.0e-5 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAudioGeoSpaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AudioGeoSpace" );

    GUCEF_TESTFW_TESTCASE( "AudioGeoSpace: a listener on a camera node and an emitter on a node: a source at +X is heard right, left when the player turned round (180 degrees), also far from the origin" )
    try
    {
        const Float32 level = 0.25f * (Float32) Sqrt2;
        for ( UInt32 variant = 0; variant < 4; ++variant )
        {
            const Float64 yaw = ( 1 == variant || 3 == variant ) ? Pi : 0.0;
            const Float32 x = variant < 2 ? 0.0f : 100.0f;
            Float32 left = -999.0f;
            Float32 right = -999.0f;
            ASSERT_TRUE( HearFromTree( yaw, left, right, x ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioGeoSpace: variant " << variant << " yaw " << yaw << ": left " << left << " right " << right );
            if ( 0.0 == yaw )
            {
                ASSERT_TRUE( Near( left, 0.0, 1.0e-5 ) && Near( right, level, 1.0e-5 ) );
            }
            else
            {
                ASSERT_TRUE( Near( left, level, 1.0e-4 ) && Near( right, 0.0, 1.0e-4 ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioGeoSpace: the snapshot of a node gives the listener its position, forward, up and velocity in one update and keeps the 2D pan width" )
    try
    {
        AUDIO::CAudioListener listener;
        ASSERT_TRUE( listener.Set2DPanWidth( 33.0f ) );
        TNode player( Placement( 1.0f, 2.0f, 3.0f, Pi / 2.0 ) );
        TSnapshot snapshot;
        player.CaptureSnapshot( snapshot, MATH::CCoordinateSystem::GucefDefault(), MATH::CTVector3D< Float32 >( 4.0f, 5.0f, 6.0f ) );
        ASSERT_TRUE( AUDIO::SetListenerFromSnapshot( listener, snapshot ) );
        AUDIO::SListenerState state;
        ASSERT_TRUE( RtReadListener( listener, state ) );
        // a quarter turn to the left about +Y: forward -Z becomes -X, the right becomes -Z
        ASSERT_TRUE( Near( state.position.x, 1.0, 0.0 ) && Near( state.position.y, 2.0, 0.0 ) && Near( state.position.z, 3.0, 0.0 ) );
        ASSERT_TRUE( Near( state.forward.x, -1.0, 1.0e-6 ) && Near( state.forward.y, 0.0, 1.0e-6 ) && Near( state.forward.z, 0.0, 1.0e-6 ) );
        ASSERT_TRUE( Near( state.up.y, 1.0, 1.0e-6 ) );
        ASSERT_TRUE( Near( state.velocity.x, 4.0, 0.0 ) && Near( state.velocity.y, 5.0, 0.0 ) && Near( state.velocity.z, 6.0, 0.0 ) );
        ASSERT_TRUE( Near( state.panWidth2D, 33.0, 0.0 ) );

        // a snapshot of a Float64 tree narrows to the same state
        GEOMETRY::CTGeoSpace< Float64 > player64( MATH::CTTransform3D< Float64 >( MATH::CTVector3D< Float64 >( 1.0, 2.0, 3.0 ),
            MATH::CTQuaternion< Float64 >::FromAxisAngle( MATH::CTDirectionVector3D< Float64 >::UnitY(), MATH::CTRadian< Float64 >( Pi / 2.0 ) ), MATH::CTVector3D< Float64 >( 1.0, 1.0, 1.0 ) ) );
        GEOMETRY::CTGeoSpaceSnapshot< Float64 > snapshot64;
        player64.CaptureSnapshot( snapshot64, MATH::CCoordinateSystem::GucefDefault() );
        AUDIO::SListenerState state64;
        listener.GetState( state64 );
        AUDIO::ListenerStateFromSnapshot( snapshot64, state64 );
        ASSERT_TRUE( Near( state64.position.x, 1.0, 1.0e-6 ) && Near( state64.forward.x, -1.0, 1.0e-6 ) && Near( state64.velocity.x, 0.0, 0.0 ) );

        // an emitter follows its node, with the cone axis along the node's forward when asked
        AUDIO::CAudioEmitter emitter;
        ASSERT_TRUE( AUDIO::SetEmitterFromSnapshot( emitter, snapshot, true ) );
        AUDIO::SEmitterState emitterState;
        ASSERT_TRUE( RtReadEmitter( emitter, emitterState ) );
        ASSERT_TRUE( Near( emitterState.position.x, 1.0, 0.0 ) && Near( emitterState.velocity.z, 6.0, 0.0 ) && Near( emitterState.direction.x, -1.0, 1.0e-6 ) );
        ASSERT_TRUE( AUDIO::SetEmitterFromSnapshot( emitter, snapshot, false ) );
        ASSERT_TRUE( RtReadEmitter( emitter, emitterState ) && Near( emitterState.direction.x, -1.0, 1.0e-6 ) );

        // placement and the pair of estimates are single updates: nothing changes when a value is refused
        AUDIO::CAudioEmitter pair;
        ASSERT_TRUE( pair.SetPlacement( TPoint3( 1.0f, 2.0f, 3.0f ), TVector3( 4.0f, 5.0f, 6.0f ), TVector3( 0.0f, 0.0f, -2.0f ) ) );
        AUDIO::SEmitterState pairState;
        ASSERT_TRUE( RtReadEmitter( pair, pairState ) && Near( pairState.position.z, 3.0, 0.0 ) && Near( pairState.velocity.y, 5.0, 0.0 ) && Near( pairState.direction.z, -1.0, 1.0e-6 ) );
        ASSERT_FALSE( pair.SetPlacement( TPoint3( 9.0f, 9.0f, 9.0f ), TVector3( 0.0f, 0.0f, 0.0f ), TVector3( (Float32) ( 1.0e30 * 1.0e30 ), 0.0f, 0.0f ) ) );
        ASSERT_FALSE( RtReadEmitter( pair, pairState ) );
        ASSERT_TRUE( pair.SetOcclusionAndObstruction( 0.25f, 2.0f ) );
        ASSERT_TRUE( RtReadEmitter( pair, pairState ) && Near( pairState.occlusion, 0.25, 0.0 ) && Near( pairState.obstruction, 1.0, 0.0 ) && Near( pairState.position.x, 1.0, 0.0 ) );

        // a state that is not finite is refused as a whole
        TSnapshot bad = snapshot;
        bad.worldPosition.x = (Float32) ( 1.0e30 * 1.0e30 );
        const AUDIO::SListenerState before = state;
        ASSERT_FALSE( AUDIO::SetListenerFromSnapshot( listener, bad ) );
        AUDIO::SListenerState after;
        listener.GetState( after );
        ASSERT_TRUE( after == before );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioGeoSpace: the occlusion estimate of a wall between two nodes reaches the emitter and is heard: gain = lerp( 1, -12 dB, occlusion ); no wall, no change" )
    try
    {
        for ( UInt32 variant = 0; variant < 3; ++variant )
        {
            // listener at the origin facing -Z, the source 5 units to the right; variant 0: no wall, 1: a wall that covers every path, 2: a wall over y > 0 only
            TNode camera( Placement( 0.0f, 0.0f, 0.0f, 0.0 ) );
            TNode siren( Placement( 5.0f, 0.0f, 0.0f, 0.0 ) );
            GEOMETRY::CRayCaster caster;
            GEOMETRY::CTGeoObject< GEOMETRY::CTAxisAlignedBox3D< Float32 > >* wall = GUCEF_NULL;
            if ( 1 == variant )
                wall = GUCEF_NEW GEOMETRY::CTGeoObject< GEOMETRY::CTAxisAlignedBox3D< Float32 > >( GEOMETRY::CTAxisAlignedBox3D< Float32 >( MATH::CTPoint3D< Float32 >( 2.4f, -50.0f, -50.0f ), MATH::CTPoint3D< Float32 >( 2.6f, 50.0f, 50.0f ) ), MATH::CTTransform3D< Float32 >() );
            else
            if ( 2 == variant )
                wall = GUCEF_NEW GEOMETRY::CTGeoObject< GEOMETRY::CTAxisAlignedBox3D< Float32 > >( GEOMETRY::CTAxisAlignedBox3D< Float32 >( MATH::CTPoint3D< Float32 >( 2.4f, 0.0f, -50.0f ), MATH::CTPoint3D< Float32 >( 2.6f, 50.0f, 50.0f ) ), MATH::CTTransform3D< Float32 >() );
            if ( GUCEF_NULL != wall )
                caster.AddObject( *wall );
            caster.Update();

            GEOMETRY::CAudioOcclusionEstimator estimator( &caster );
            estimator.SetRayCount( 256 );
            estimator.SetSourceRadius( 0.5f );
            estimator.SetListenerRadius( 0.5f );
            GEOMETRY::TFloat32AudioOcclusionEstimate estimate;
            TSnapshot cameraSnapshot;
            TSnapshot sirenSnapshot;
            camera.CaptureSnapshot( cameraSnapshot, MATH::CCoordinateSystem::GucefDefault() );
            siren.CaptureSnapshot( sirenSnapshot, MATH::CCoordinateSystem::GucefDefault() );
            estimator.Estimate( sirenSnapshot.worldPosition, cameraSnapshot.worldPosition, estimate );

            AUDIO::CAudioEngine engine;
            ASSERT_TRUE( engine.Initialize( NullParams() ) );
            AUDIO::TAudioEmitterPtr emitter = engine.CreateEmitter();
            AUDIO::TAudioVoicePtr voice = engine.CreateVoice( MakeConstant( 48000, 0.25f ) );
            ASSERT_TRUE( emitter->SetAttenuation( NoAttenuation() ) && AUDIO::SetEmitterFromSnapshot( *emitter, sirenSnapshot ) );
            ASSERT_TRUE( AUDIO::SetListenerFromSnapshot( engine.GetListener(), cameraSnapshot ) );
            ASSERT_TRUE( AUDIO::SetEmitterOcclusionFromEstimate( *emitter, estimate ) );
            AUDIO::SEmitterState read;
            ASSERT_TRUE( RtReadEmitter( *emitter, read ) );
            ASSERT_TRUE( Near( read.occlusion, estimate.occlusion, 0.0 ) && Near( read.obstruction, estimate.obstruction, 0.0 ) );
            ASSERT_TRUE( voice->SetEmitter( emitter ) && StartVoice( engine, voice ) );
            RenderBlocks( engine, 60 );

            const Float64 occlusion = (Float64) estimate.occlusion;
            const Float64 strength = (Float64) estimate.occlusion > (Float64) estimate.obstruction ? occlusion : (Float64) estimate.obstruction;
            const Float64 gain = 1.0 + ( pow( 10.0, -12.0 / 20.0 ) - 1.0 ) * strength;
            const Float64 expected = 0.25 * Sqrt2 * gain;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioGeoSpace: occlusion variant " << variant << ": occlusion " << estimate.occlusion << " obstruction " << estimate.obstruction << " right " << Last( engine, 1 ) << " expected " << expected );
            if ( 0 == variant )
                ASSERT_TRUE( Near( estimate.occlusion, 0.0, 0.0 ) && Near( estimate.obstruction, 0.0, 0.0 ) );
            if ( 1 == variant )
                ASSERT_TRUE( Near( estimate.occlusion, 1.0, 0.0 ) && Near( estimate.obstruction, 1.0, 0.0 ) );
            if ( 2 == variant )
                ASSERT_TRUE( Near( estimate.occlusion, 0.5, 1.0 / 16.0 ) );
            ASSERT_TRUE( Near( Last( engine, 1 ), expected, 0.01 * expected ) );
            ASSERT_TRUE( Near( Last( engine, 0 ), 0.0, 1.0e-4 ) );
            engine.Shutdown();
            if ( GUCEF_NULL != wall )
            {
                caster.RemoveObject( *wall );
                GUCEF_DELETE wall;
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioGeoSpace: an owner thread that moves a camera node and hands over snapshots is read by the audio side one whole pose at a time, in order, without waiting" )
    try
    {
        const UInt32 ticks = 40000;
        AUDIO::CAudioListener listener;
        AUDIO::CAudioEmitter emitter;
        CCameraOwnerThread owner( listener, emitter, ticks );
        ASSERT_TRUE( owner.Activate( GUCEF_NULL, 0, 0 ) );
        UInt64 reads = 0;
        UInt64 torn = 0;
        UInt64 backwards = 0;
        Float32 lastTick = 0.0f;
        UInt32 waited = 0;
        bool finished = false;
        AUDIO::SListenerState state;
        AUDIO::SEmitterState emitterState;
        while ( !finished && waited < 30000 )
        {
            finished = owner.IsDone();
            while ( RtReadListener( listener, state ) )
            {
                ++reads;
                if ( state.position.x > 0.0f && !IsOneTick( state ) )
                    ++torn;
                if ( state.position.x < lastTick )
                    ++backwards;
                lastTick = state.position.x;
            }
            // the emitter is the same node: its direction is the forward of that tick
            while ( RtReadEmitter( emitter, emitterState ) )
            {
                const Float64 yaw = (Float64) emitterState.position.x * 0.001;
                if ( emitterState.position.x > 0.0f && !Near( emitterState.direction.x, -sin( yaw ), 2.0e-5 ) )
                    ++torn;
            }
            if ( !finished )
            {
                MT::PrecisionDelay( 1 );
                ++waited;
            }
        }
        owner.WaitForThreadToFinish( 5000 );
        while ( RtReadListener( listener, state ) )
            ++reads;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioGeoSpace: " << reads << " of " << ticks << " published poses were read, " << torn << " torn, " << backwards << " out of order" );
        ASSERT_TRUE( finished && reads > 0 && 0 == torn && 0 == backwards && 0 == owner.GetRefused() );
        // the last published pose is the last pose that can be read
        AUDIO::SListenerState finalState;
        listener.GetState( finalState );
        ASSERT_TRUE( Near( finalState.position.x, (Float64) ticks, 0.0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
