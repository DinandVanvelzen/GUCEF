/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CServiceLifecycle.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CServiceLifecycle::StateChangedEvent = "GUCEF::CORE::CServiceLifecycle::StateChangedEvent";
const CEvent CServiceLifecycle::ConditionsChangedEvent = "GUCEF::CORE::CServiceLifecycle::ConditionsChangedEvent";

/*-------------------------------------------------------------------------*/

namespace {

/** Monotonic milliseconds of the precision timer */
UInt64
LifecycleNowMs( void )
{GUCEF_TRACE;

    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        return MT::PrecisionTickCount();
    return MT::PrecisionTickCount() / ticksPerMs;
}

const char* const StateNames[] =
{
    "STARTING", "LOCK_WAIT", "LOADING", "READY", "DEGRADED", "READ_ONLY", "DRAINING", "STOPPED", "FAILED"
};
const UInt32 StateCount = (UInt32) ( sizeof( StateNames ) / sizeof( StateNames[ 0 ] ) );

}

/*-------------------------------------------------------------------------*/

const char*
ServiceLifecycleStateToString( TServiceLifecycleState state )
{GUCEF_TRACE;

    const UInt32 index = (UInt32) state;
    if ( index < StateCount )
        return StateNames[ index ];
    return "UNKNOWN";
}

/*-------------------------------------------------------------------------*/

bool
ServiceLifecycleStateFromString( const CString& text, TServiceLifecycleState& state )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<StateCount; ++i )
    {
        if ( text.Equals( StateNames[ i ], false ) )
        {
            state = (TServiceLifecycleState) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

SServiceLifecycleChange::SServiceLifecycleChange( void )
    : from( SERVICELIFECYCLE_STARTING )
    , to( SERVICELIFECYCLE_STARTING )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CServiceLifecycle::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
    ConditionsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServiceLifecycle::CServiceLifecycle( PulseGeneratorPtr pulseGenerator )
    : CTSGNotifier( pulseGenerator, true, false )
    , m_stateLock()
    , m_state( SERVICELIFECYCLE_STARTING )
    , m_reason()
    , m_createdAtMs( LifecycleNowMs() )
    , m_stateSinceMs( 0 )
    , m_drainDeadlineMs( 0 )
    , m_conditions()
{GUCEF_TRACE;

    RegisterEvents();
    m_stateSinceMs = m_createdAtMs;
}

/*-------------------------------------------------------------------------*/

CServiceLifecycle::~CServiceLifecycle()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

const CString&
CServiceLifecycle::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CServiceLifecycle";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::IsTransitionAllowed( TServiceLifecycleState from, TServiceLifecycleState to )
{GUCEF_TRACE;

    if ( from == to )
        return false;

    switch ( from )
    {
        case SERVICELIFECYCLE_STARTING:
            return SERVICELIFECYCLE_LOCK_WAIT == to || SERVICELIFECYCLE_LOADING == to || SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_LOCK_WAIT:
            return SERVICELIFECYCLE_LOADING == to || SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_LOADING:
            return SERVICELIFECYCLE_READY == to || SERVICELIFECYCLE_DEGRADED == to || SERVICELIFECYCLE_LOCK_WAIT == to ||
                   SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_READY:
            return SERVICELIFECYCLE_DEGRADED == to || SERVICELIFECYCLE_READ_ONLY == to || SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_DEGRADED:
            return SERVICELIFECYCLE_READY == to || SERVICELIFECYCLE_READ_ONLY == to || SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_READ_ONLY:
            return SERVICELIFECYCLE_DRAINING == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_DRAINING:
            return SERVICELIFECYCLE_STOPPED == to || SERVICELIFECYCLE_FAILED == to;
        case SERVICELIFECYCLE_STOPPED:
        case SERVICELIFECYCLE_FAILED:
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

TServiceLifecycleState
CServiceLifecycle::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::IsFinal( void ) const
{GUCEF_TRACE;

    const TServiceLifecycleState state = GetState();
    return SERVICELIFECYCLE_STOPPED == state || SERVICELIFECYCLE_FAILED == state;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::IsServing( void ) const
{GUCEF_TRACE;

    const TServiceLifecycleState state = GetState();
    return SERVICELIFECYCLE_READY == state || SERVICELIFECYCLE_DEGRADED == state || SERVICELIFECYCLE_READ_ONLY == state;
}

/*-------------------------------------------------------------------------*/

CString
CServiceLifecycle::GetReason( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_reason;
}

/*-------------------------------------------------------------------------*/

UInt64
CServiceLifecycle::GetStateAgeInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    const UInt64 now = LifecycleNowMs();
    return now > m_stateSinceMs ? now - m_stateSinceMs : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CServiceLifecycle::GetUptimeInMs( void ) const
{GUCEF_TRACE;

    const UInt64 now = LifecycleNowMs();
    return now > m_createdAtMs ? now - m_createdAtMs : 0;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::TransitionLocked( TServiceLifecycleState to, const CString& reason, SServiceLifecycleChange& change )
{GUCEF_TRACE;

    if ( !IsTransitionAllowed( m_state, to ) )
        return false;

    change.from = m_state;
    change.to = to;
    change.reason = reason;
    m_state = to;
    m_reason = reason;
    m_stateSinceMs = LifecycleNowMs();
    if ( SERVICELIFECYCLE_DRAINING != to )
        m_drainDeadlineMs = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CServiceLifecycle::RaiseChange( const SServiceLifecycleChange& change )
{GUCEF_TRACE;

    if ( SERVICELIFECYCLE_FAILED == change.to )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ServiceLifecycle: " << ServiceLifecycleStateToString( change.from ) << " -> "
                         << ServiceLifecycleStateToString( change.to ) << ( change.reason.IsNULLOrEmpty() ? "" : ": " ) << change.reason );
    }
    else
    {
        GUCEF_LOG( LOGLEVEL_NORMAL, "ServiceLifecycle: " << ServiceLifecycleStateToString( change.from ) << " -> "
                   << ServiceLifecycleStateToString( change.to ) << ( change.reason.IsNULLOrEmpty() ? "" : ": " ) << change.reason );
    }

    TServiceLifecycleChangeEventData eventData( change );
    NotifyObservers( StateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::Transition( TServiceLifecycleState to, const CString& reason )
{GUCEF_TRACE;

    SServiceLifecycleChange change;
    {
        MT::CScopeMutex lock( m_stateLock );

        // a service with a condition that comes up is degraded, not ready
        TServiceLifecycleState target = to;
        if ( SERVICELIFECYCLE_READY == to && !m_conditions.empty() )
            target = SERVICELIFECYCLE_DEGRADED;
        if ( target == m_state && target != to )
            return true;
        if ( !TransitionLocked( target, reason, change ) )
            return false;
    }
    RaiseChange( change );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::Fail( const CString& reason )
{GUCEF_TRACE;

    return Transition( SERVICELIFECYCLE_FAILED, reason );
}

/*-------------------------------------------------------------------------*/

void
CServiceLifecycle::SetCondition( const CString& name, const CString& text )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return;

    bool changed = false;
    bool stateChanged = false;
    SServiceLifecycleChange change;
    {
        MT::CScopeMutex lock( m_stateLock );

        CString::StringMap::iterator i = m_conditions.find( name );
        if ( i == m_conditions.end() || (*i).second != text )
        {
            m_conditions[ name ] = text;
            changed = true;
        }
        if ( SERVICELIFECYCLE_READY == m_state )
            stateChanged = TransitionLocked( SERVICELIFECYCLE_DEGRADED, name + ": " + text, change );
    }
    if ( changed )
        NotifyObservers( ConditionsChangedEvent );
    if ( stateChanged )
        RaiseChange( change );
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::ClearCondition( const CString& name )
{GUCEF_TRACE;

    bool stateChanged = false;
    SServiceLifecycleChange change;
    {
        MT::CScopeMutex lock( m_stateLock );

        CString::StringMap::iterator i = m_conditions.find( name );
        if ( i == m_conditions.end() )
            return false;
        m_conditions.erase( i );
        if ( m_conditions.empty() && SERVICELIFECYCLE_DEGRADED == m_state )
            stateChanged = TransitionLocked( SERVICELIFECYCLE_READY, "the condition " + name + " cleared", change );
    }
    NotifyObservers( ConditionsChangedEvent );
    if ( stateChanged )
        RaiseChange( change );
    return true;
}

/*-------------------------------------------------------------------------*/

CString::StringMap
CServiceLifecycle::GetConditions( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_conditions;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::BeginDrain( const CString& reason, UInt32 drainMs )
{GUCEF_TRACE;

    SServiceLifecycleChange change;
    {
        MT::CScopeMutex lock( m_stateLock );

        if ( !TransitionLocked( SERVICELIFECYCLE_DRAINING, reason, change ) )
            return false;
        m_drainDeadlineMs = m_stateSinceMs + drainMs;
    }
    RaiseChange( change );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CServiceLifecycle::GetDrainRemainingMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    if ( SERVICELIFECYCLE_DRAINING != m_state )
        return 0;
    const UInt64 now = LifecycleNowMs();
    return now < m_drainDeadlineMs ? m_drainDeadlineMs - now : 0;
}

/*-------------------------------------------------------------------------*/

bool
CServiceLifecycle::IsDrainExpired( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    if ( SERVICELIFECYCLE_DRAINING != m_state )
        return false;
    return LifecycleNowMs() >= m_drainDeadlineMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
