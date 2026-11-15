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

#include "gucefAUDIO_CAudioListener.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

/** Unit forward and up from the given vectors: false when one is not usable, or when they are (nearly) parallel */
bool
NormalisedOrientation( const CAudioSpatialMath::TVector3& forward, const CAudioSpatialMath::TVector3& up,
                       CAudioSpatialMath::TVector3& unitForward, CAudioSpatialMath::TVector3& unitUp )
{
    CAudioSpatialMath::TVector3 right;
    return forward.IsFinite() && up.IsFinite() && forward.TryNormalize( unitForward, 1.0e-6f ) &&
           up.TryNormalize( unitUp, 1.0e-6f ) && unitForward.Cross( unitUp ).TryNormalize( right, 1.0e-4f );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SListenerState::SListenerState( void )
    : position()
    , forward( CAudioSpatialMath::TVector3::Forward() )
    , up( CAudioSpatialMath::TVector3::Up() )
    , velocity()
    , panWidth2D( 10.0f )
{

}

/*-------------------------------------------------------------------------*/

bool
SListenerState::operator==( const SListenerState& other ) const
{
    return position == other.position && forward == other.forward && up == other.up &&
           velocity == other.velocity && panWidth2D == other.panWidth2D;
}

/*-------------------------------------------------------------------------*/

bool
SListenerState::operator!=( const SListenerState& other ) const
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

const Float32 CAudioListener::DefaultPanWidth2D = 10.0f;

/*-------------------------------------------------------------------------*/

CAudioListener::CAudioListener( void )
    : m_writerLock()
    , m_state()
    , m_exchange()
{GUCEF_TRACE;

    // the reader has a state from the start
    m_exchange.Write( m_state );
}

/*-------------------------------------------------------------------------*/

CAudioListener::~CAudioListener()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetPose( const TPoint3& position, const TVector3& forward, const TVector3& up )
{GUCEF_TRACE;

    TVector3 unitForward;
    TVector3 unitUp;
    if ( !IsFinitePoint( position ) || !NormalisedOrientation( forward, up, unitForward, unitUp ) )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.position = position;
    m_state.forward = unitForward;
    m_state.up = unitUp;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetPosition( const TPoint3& position )
{GUCEF_TRACE;

    if ( !IsFinitePoint( position ) )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.position = position;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetOrientation( const TVector3& forward, const TVector3& up )
{GUCEF_TRACE;

    TVector3 unitForward;
    TVector3 unitUp;
    if ( !NormalisedOrientation( forward, up, unitForward, unitUp ) )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.forward = unitForward;
    m_state.up = unitUp;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetVelocity( const TVector3& velocity )
{GUCEF_TRACE;

    if ( !velocity.IsFinite() )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.velocity = velocity;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetPositionAndVelocity( const TPoint3& position, const TVector3& velocity )
{GUCEF_TRACE;

    if ( !IsFinitePoint( position ) || !velocity.IsFinite() )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.position = position;
    m_state.velocity = velocity;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::Set2DPanWidth( Float32 units )
{GUCEF_TRACE;

    if ( !( units > 0.0f ) || !( units < 1.0e30f ) )
        return false;

    MT::CScopeMutex lock( m_writerLock );
    m_state.panWidth2D = units;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::SetState( const SListenerState& state )
{GUCEF_TRACE;

    SListenerState checked = state;
    if ( !IsFinitePoint( state.position ) || !state.velocity.IsFinite() || !NormalisedOrientation( state.forward, state.up, checked.forward, checked.up ) ||
         !( state.panWidth2D > 0.0f ) || !( state.panWidth2D < 1.0e30f ) )
    {
        return false;
    }

    MT::CScopeMutex lock( m_writerLock );
    m_state = checked;
    m_exchange.Write( m_state );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioListener::GetState( SListenerState& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_writerLock );
    state = m_state;
}

/*-------------------------------------------------------------------------*/

bool
CAudioListener::ReadState( SListenerState& state ) GUCEF_AUDIO_REALTIME_CONTEXT
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
