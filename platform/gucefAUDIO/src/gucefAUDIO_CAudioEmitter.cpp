/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include "gucefAUDIO_CAudioEmitter.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsFinitePoint( const CAudioSpatialMath::TPoint3& point )
{
    return CAudioSpatialMath::TVector3( point.x, point.y, point.z ).IsFinite();
}

/** A value in the range, false for a value that is not a number */
bool
LimitTo( Float32& value, Float32 minimum, Float32 maximum )
{
    if ( !( value == value ) )
        return false;
    if ( value < minimum )
        value = minimum;
    else
    if ( value > maximum )
        value = maximum;
    return true;
}

/** A cutoff in Hz: 0 for none, otherwise 20 .. 20000 */
bool
LimitCutoff( Float32& value )
{
    if ( !( value == value ) )
        return false;
    if ( !( value > 0.0f ) )
        value = 0.0f;
    else
        LimitTo( value, 20.0f, 20000.0f );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SEmitterState::SEmitterState( void )
    : mode( AUDIOSPATIAL_3D )
    , position()
    , velocity()
    , direction()
    , listenerRelative( false )
    , useWorldAttenuation( true )
    , attenuation()
    , coneInnerAngle( 360.0f )
    , coneOuterAngle( 360.0f )
    , coneOuterGain( 0.0f )
    , coneOuterCutoff( 0.0f )
    , spreadRadius( 0.0f )
    , distanceCutoff( 0.0f )
    , occlusion( 0.0f )
    , obstruction( 0.0f )
    , occlusionGainInDecibels( -12.0f )
    , occlusionCutoff( 1500.0f )
    , dopplerFactor( 1.0f )
    , sendsFollowAttenuation( true )
    , deriveVelocity( false )
{

}

/*-------------------------------------------------------------------------*/

const CAudioAttenuation::TEvaluator&
SEmitterState::GetEffectiveAttenuation( const SSpatialWorldState& world ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    return useWorldAttenuation ? world.attenuation : attenuation;
}

/*-------------------------------------------------------------------------*/

Float32
SEmitterState::GetEffectiveSpreadRadius( const SSpatialWorldState& world ) const GUCEF_AUDIO_REALTIME_CONTEXT
{
    return spreadRadius > 0.0f ? spreadRadius : GetEffectiveAttenuation( world ).GetMinDistance();
}

/*-------------------------------------------------------------------------*/

bool
SEmitterState::operator==( const SEmitterState& other ) const
{
    return mode == other.mode && position == other.position && velocity == other.velocity && direction == other.direction &&
           listenerRelative == other.listenerRelative && useWorldAttenuation == other.useWorldAttenuation &&
           attenuation == other.attenuation && coneInnerAngle == other.coneInnerAngle && coneOuterAngle == other.coneOuterAngle &&
           coneOuterGain == other.coneOuterGain && coneOuterCutoff == other.coneOuterCutoff && spreadRadius == other.spreadRadius &&
           distanceCutoff == other.distanceCutoff && occlusion == other.occlusion && obstruction == other.obstruction &&
           occlusionGainInDecibels == other.occlusionGainInDecibels && occlusionCutoff == other.occlusionCutoff &&
           dopplerFactor == other.dopplerFactor && sendsFollowAttenuation == other.sendsFollowAttenuation &&
           deriveVelocity == other.deriveVelocity;
}

/*-------------------------------------------------------------------------*/

bool
SEmitterState::operator!=( const SEmitterState& other ) const
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CAudioEmitter::CAudioEmitter( void )
    : m_writerLock()
    , m_state()
    , m_derivedFrom()
    , m_derivedFromTicks( 0 )
    , m_derivedFromKnown( false )
    , m_exchange()
{GUCEF_TRACE;

    // the reader has a state from the start
    m_exchange.Write( m_state );
}

/*-------------------------------------------------------------------------*/

CAudioEmitter::~CAudioEmitter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::Sanitise( SEmitterState& state )
{GUCEF_TRACE;

    if ( state.mode != AUDIOSPATIAL_2D && state.mode != AUDIOSPATIAL_3D )
        return false;
    if ( !IsFinitePoint( state.position ) || !state.velocity.IsFinite() || !state.direction.IsFinite() )
        return false;

    // the axis of the cone is a unit vector, or zero
    TVector3 axis;
    if ( state.direction.TryNormalize( axis, 1.0e-12f ) )
        state.direction = axis;
    else
        state.direction = TVector3();

    if ( !LimitTo( state.coneInnerAngle, 0.0f, 360.0f ) || !LimitTo( state.coneOuterAngle, 0.0f, 360.0f ) || !LimitTo( state.coneOuterGain, 0.0f, 1.0f ) )
        return false;
    if ( state.coneOuterAngle < state.coneInnerAngle )
        state.coneOuterAngle = state.coneInnerAngle;

    if ( !LimitCutoff( state.coneOuterCutoff ) || !LimitCutoff( state.distanceCutoff ) || !LimitCutoff( state.occlusionCutoff ) )
        return false;
    if ( !LimitTo( state.spreadRadius, 0.0f, 1.0e30f ) || !LimitTo( state.occlusion, 0.0f, 1.0f ) || !LimitTo( state.obstruction, 0.0f, 1.0f ) ||
         !LimitTo( state.occlusionGainInDecibels, -96.0f, 0.0f ) || !LimitTo( state.dopplerFactor, 0.0f, 10.0f ) )
    {
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioEmitter::Publish( void )
{GUCEF_TRACE;

    m_exchange.Write( m_state );
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetMode( TAudioSpatialMode mode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.mode = mode;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetPosition( const TPoint3& position )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.position = position;
    if ( !Sanitise( candidate ) )
        return false;
    DeriveVelocity( candidate );
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetVelocity( const TVector3& velocity )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.velocity = velocity;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetPositionAndVelocity( const TPoint3& position, const TVector3& velocity )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.position = position;
    candidate.velocity = velocity;
    if ( !Sanitise( candidate ) )
        return false;
    DeriveVelocity( candidate );
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetDirection( const TVector3& direction )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.direction = direction;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetPlacement( const TPoint3& position, const TVector3& velocity, const TVector3& direction )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.position = position;
    candidate.velocity = velocity;
    candidate.direction = direction;
    if ( !Sanitise( candidate ) )
        return false;
    DeriveVelocity( candidate );
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetListenerRelative( bool relative )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    m_state.listenerRelative = relative;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetAttenuation( const CAudioAttenuation& attenuation )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    m_state.attenuation = attenuation.GetEvaluator();
    m_state.useWorldAttenuation = false;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::UseWorldAttenuation( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    m_state.useWorldAttenuation = true;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetCone( Float32 innerAngleInDegrees, Float32 outerAngleInDegrees, Float32 outerGain, Float32 outerCutoffInHz )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.coneInnerAngle = innerAngleInDegrees;
    candidate.coneOuterAngle = outerAngleInDegrees;
    candidate.coneOuterGain = outerGain;
    candidate.coneOuterCutoff = outerCutoffInHz;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetSpreadRadius( Float32 radius )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.spreadRadius = radius;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetDistanceLowPass( Float32 cutoffInHz )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.distanceCutoff = cutoffInHz;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetOcclusion( Float32 occlusion )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.occlusion = occlusion;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetObstruction( Float32 obstruction )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.obstruction = obstruction;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetOcclusionAndObstruction( Float32 occlusion, Float32 obstruction )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.occlusion = occlusion;
    candidate.obstruction = obstruction;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetOcclusionResponse( Float32 gainInDecibels, Float32 cutoffInHz )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.occlusionGainInDecibels = gainInDecibels;
    candidate.occlusionCutoff = cutoffInHz;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetDopplerFactor( Float32 factor )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.dopplerFactor = factor;
    if ( !Sanitise( candidate ) )
        return false;
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetSendsFollowAttenuation( bool follow )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    m_state.sendsFollowAttenuation = follow;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetDeriveVelocity( bool derive )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    SEmitterState candidate = m_state;
    candidate.deriveVelocity = derive;
    if ( derive != m_state.deriveVelocity )
        DeriveVelocity( candidate );            // a switch: starts or ends the measuring
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::SetState( const SEmitterState& state )
{GUCEF_TRACE;

    SEmitterState candidate = state;
    if ( !Sanitise( candidate ) )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    DeriveVelocity( candidate );
    m_state = candidate;
    Publish();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioEmitter::DeriveVelocity( SEmitterState& candidate )
{GUCEF_TRACE;

    if ( !candidate.deriveVelocity )
    {
        m_derivedFromKnown = false;
        return;
    }

    const UInt64 now = MT::PrecisionTickCount();
    if ( !m_state.deriveVelocity || !m_derivedFromKnown )
    {
        // just switched on: nothing to measure from yet, the velocity stays what it was until the first measurement
        m_derivedFrom = candidate.position;
        m_derivedFromTicks = now;
        m_derivedFromKnown = true;
        return;
    }

    // updates closer together than this move the position and keep the velocity: a tiny time step would magnify jitter
    const Float64 seconds = (Float64) ( now - m_derivedFromTicks ) / (Float64) MT::PrecisionTimerResolution();
    if ( !( seconds >= 0.001 ) )
    {
        candidate.velocity = m_state.velocity;
        return;
    }
    const TVector3 moved = candidate.position - m_derivedFrom;
    const Float32 scale = (Float32) ( 1.0 / seconds );
    candidate.velocity = TVector3( moved.x * scale, moved.y * scale, moved.z * scale );
    if ( !candidate.velocity.IsFinite() )
        candidate.velocity = TVector3();
    m_derivedFrom = candidate.position;
    m_derivedFromTicks = now;
}

/*-------------------------------------------------------------------------*/

void
CAudioEmitter::GetState( SEmitterState& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    state = m_state;
}

/*-------------------------------------------------------------------------*/

bool
CAudioEmitter::ReadState( SEmitterState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
{
    // no trace macro: this runs where the audio is mixed
    return m_exchange.Read( state );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
