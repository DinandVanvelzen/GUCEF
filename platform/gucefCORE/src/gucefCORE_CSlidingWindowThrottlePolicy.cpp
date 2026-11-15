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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCORE_CSlidingWindowThrottlePolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt64 MinuteMs = 60000;
static const UInt64 HourMs = 3600000;
static const UInt64 DayMs = 86400000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSlidingWindowThrottlePolicy::CSlidingWindowThrottlePolicy( void )
    : CIJobThrottlePolicy()
    , m_lock()
    , m_requestsPerMinute( 0 )
    , m_requestsPerHour( 0 )
    , m_requestsPerDay( 0 )
    , m_minimumIntervalMs( 0 )
    , m_reservePercent( 0 )
    , m_reserveFromPriority( 2 )
    , m_starts()
    , m_loadedStarts()
    , m_lastMonotonicMs( 0 )
    , m_lastUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSlidingWindowThrottlePolicy::~CSlidingWindowThrottlePolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CSlidingWindowThrottlePolicy::SetRequestsPerMinute( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_requestsPerMinute = value; }
UInt32 CSlidingWindowThrottlePolicy::GetRequestsPerMinute( void ) const {GUCEF_TRACE; return m_requestsPerMinute; }
void CSlidingWindowThrottlePolicy::SetRequestsPerHour( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_requestsPerHour = value; }
UInt32 CSlidingWindowThrottlePolicy::GetRequestsPerHour( void ) const {GUCEF_TRACE; return m_requestsPerHour; }
void CSlidingWindowThrottlePolicy::SetRequestsPerDay( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_requestsPerDay = value; }
UInt32 CSlidingWindowThrottlePolicy::GetRequestsPerDay( void ) const {GUCEF_TRACE; return m_requestsPerDay; }
void CSlidingWindowThrottlePolicy::SetMinimumIntervalMs( UInt64 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_minimumIntervalMs = value; }
UInt64 CSlidingWindowThrottlePolicy::GetMinimumIntervalMs( void ) const {GUCEF_TRACE; return m_minimumIntervalMs; }
void CSlidingWindowThrottlePolicy::SetReservePercent( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_reservePercent = value > 100 ? 100 : value; }
UInt32 CSlidingWindowThrottlePolicy::GetReservePercent( void ) const {GUCEF_TRACE; return m_reservePercent; }
void CSlidingWindowThrottlePolicy::SetReserveFromPriority( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_reserveFromPriority = value; }
UInt32 CSlidingWindowThrottlePolicy::GetReserveFromPriority( void ) const {GUCEF_TRACE; return m_reserveFromPriority; }

/*-------------------------------------------------------------------------*/

UInt32
CSlidingWindowThrottlePolicy::EffectiveLimit( UInt32 limit, UInt32 priority ) const
{GUCEF_TRACE;

    if ( 0 == limit || 0 == m_reservePercent || priority < m_reserveFromPriority )
        return limit;
    UInt32 kept = (UInt32) ( ( (UInt64) limit * ( 100 - m_reservePercent ) ) / 100 );
    return kept < 1 ? 1 : kept;
}

/*-------------------------------------------------------------------------*/

UInt32
CSlidingWindowThrottlePolicy::UsedInWindow( UInt64 windowMs, UInt64 nowMonotonicMs ) const
{GUCEF_TRACE;

    UInt32 used = 0;
    for ( TStartVector::const_reverse_iterator i = m_starts.rbegin(); i != m_starts.rend(); ++i )
    {
        if ( (*i).monotonicMs + (Int64) windowMs <= (Int64) nowMonotonicMs )
            break;
        used += (*i).cost;
    }
    return used;
}

/*-------------------------------------------------------------------------*/

UInt64
CSlidingWindowThrottlePolicy::WindowWaitMs( UInt64 windowMs, UInt32 limit, UInt32 cost, UInt64 nowMonotonicMs ) const
{GUCEF_TRACE;

    // how long until enough of the window's use has slid out of it for 'cost' more
    UInt32 used = UsedInWindow( windowMs, nowMonotonicMs );
    if ( used + cost <= limit )
        return 0;
    UInt32 toFree = used + cost - limit;
    UInt32 freed = 0;
    for ( TStartVector::const_iterator i = m_starts.begin(); i != m_starts.end(); ++i )
    {
        if ( (*i).monotonicMs + (Int64) windowMs <= (Int64) nowMonotonicMs )
            continue;
        freed += (*i).cost;
        if ( freed >= toFree )
            return (UInt64) ( (*i).monotonicMs + (Int64) windowMs - (Int64) nowMonotonicMs );
    }
    return windowMs;
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::Prune( UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    // nothing older than the longest window is kept: bounded by the day limit
    UInt64 keepMs = 0 != m_requestsPerDay ? DayMs : ( 0 != m_requestsPerHour ? HourMs : MinuteMs );
    if ( m_minimumIntervalMs > keepMs )
        keepMs = m_minimumIntervalMs;
    size_t drop = 0;
    while ( drop < m_starts.size() && m_starts[ drop ].monotonicMs + (Int64) keepMs <= (Int64) nowMonotonicMs && drop + 1 < m_starts.size() )
        ++drop;
    if ( drop > 0 )
        m_starts.erase( m_starts.begin(), m_starts.begin() + drop );
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::AdoptLoadedStarts( Int64 nowUtcMs, UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    // kept starts were saved in UTC: placed on this run's monotonic clock by their age
    for ( TStartVector::iterator i = m_loadedStarts.begin(); i != m_loadedStarts.end(); ++i )
    {
        Int64 ageMs = nowUtcMs - (*i).utcMs;
        if ( ageMs < 0 || (UInt64) ageMs >= DayMs )
            continue;
        SStart start = (*i);
        start.monotonicMs = (Int64) nowMonotonicMs - ageMs;
        m_starts.push_back( start );
    }
    m_loadedStarts.clear();
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::Check( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, SJobPermitAnswer& answer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_loadedStarts.empty() )
        AdoptLoadedStarts( nowUtcMs, nowMonotonicMs );
    Prune( nowMonotonicMs );

    UInt32 cost = request.estimatedCost > 0 ? request.estimatedCost : 1;
    answer.granted = true;
    answer.waitMs = 0;
    answer.bindingLimit.Clear();

    struct SWindow { UInt64 windowMs; UInt32 limit; const char* name; };
    SWindow windows[ 3 ] = { { MinuteMs, m_requestsPerMinute, "requestsPerMinute" },
                             { HourMs, m_requestsPerHour, "requestsPerHour" },
                             { DayMs, m_requestsPerDay, "requestsPerDay" } };
    for ( UInt32 w=0; w<3; ++w )
    {
        UInt32 limit = EffectiveLimit( windows[ w ].limit, request.priority );
        if ( 0 == limit )
            continue;
        UInt64 waitMs = WindowWaitMs( windows[ w ].windowMs, limit, cost, nowMonotonicMs );
        if ( waitMs > answer.waitMs )
        {
            answer.granted = false;
            answer.waitMs = waitMs;
            answer.bindingLimit = windows[ w ].name;
        }
    }

    if ( 0 != m_minimumIntervalMs && !m_starts.empty() )
    {
        Int64 last = m_starts.back().monotonicMs;
        if ( (Int64) nowMonotonicMs < last + (Int64) m_minimumIntervalMs )
        {
            UInt64 waitMs = (UInt64) ( last + (Int64) m_minimumIntervalMs - (Int64) nowMonotonicMs );
            if ( waitMs > answer.waitMs )
            {
                answer.granted = false;
                answer.waitMs = waitMs;
                answer.bindingLimit = "minimumInterval";
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::OnStarted( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SStart start;
    start.monotonicMs = (Int64) nowMonotonicMs;
    start.utcMs = nowUtcMs;
    start.cost = request.estimatedCost > 0 ? request.estimatedCost : 1;
    m_starts.push_back( start );
    m_lastMonotonicMs = nowMonotonicMs;
    m_lastUtcMs = nowUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::OnFinished( const SJobPermitRequest& request, UInt32 actualCost, TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs )
{GUCEF_TRACE;

    // what was spent beyond the estimate counts from the last start (a hold reports its whole use here)
    UInt32 estimated = JOBRUNCAUSE_HOLD == request.cause ? 0 : ( request.estimatedCost > 0 ? request.estimatedCost : 1 );
    if ( actualCost <= estimated )
        return;
    MT::CScopeMutex lock( m_lock );
    SStart extra;
    extra.monotonicMs = (Int64) m_lastMonotonicMs + ( nowUtcMs > m_lastUtcMs ? nowUtcMs - m_lastUtcMs : 0 );
    extra.utcMs = nowUtcMs;
    extra.cost = actualCost - estimated;
    m_starts.push_back( extra );
}

/*-------------------------------------------------------------------------*/

void
CSlidingWindowThrottlePolicy::GetStatus( CDataNode& status, Int64 nowUtcMs ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 nowMonotonicMs = m_lastMonotonicMs + (UInt64) ( nowUtcMs > m_lastUtcMs ? nowUtcMs - m_lastUtcMs : 0 );
    status.SetNodeType( GUCEF_DATATYPE_OBJECT );
    status.SetAttribute( "usedPerMinute", UsedInWindow( MinuteMs, nowMonotonicMs ) );
    status.SetAttribute( "limitPerMinute", m_requestsPerMinute );
    status.SetAttribute( "usedPerHour", UsedInWindow( HourMs, nowMonotonicMs ) );
    status.SetAttribute( "limitPerHour", m_requestsPerHour );
    status.SetAttribute( "usedPerDay", UsedInWindow( DayMs, nowMonotonicMs ) );
    status.SetAttribute( "limitPerDay", m_requestsPerDay );
    status.SetAttribute( "minimumIntervalMs", m_minimumIntervalMs );
    status.SetAttribute( "reservePercent", m_reservePercent );
}

/*-------------------------------------------------------------------------*/

bool
CSlidingWindowThrottlePolicy::SaveState( CDataNode& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    state.Clear();
    state.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CDataNode* starts = state.AddChild( "starts", GUCEF_DATATYPE_ARRAY );
    for ( TStartVector::const_iterator i = m_starts.begin(); i != m_starts.end(); ++i )
    {
        CDataNode* start = starts->AddChild( "start", GUCEF_DATATYPE_OBJECT );
        start->SetAttribute( "utcMs", (*i).utcMs );
        start->SetAttribute( "cost", (*i).cost );
    }
    for ( TStartVector::const_iterator i = m_loadedStarts.begin(); i != m_loadedStarts.end(); ++i )
    {
        CDataNode* start = starts->AddChild( "start", GUCEF_DATATYPE_OBJECT );
        start->SetAttribute( "utcMs", (*i).utcMs );
        start->SetAttribute( "cost", (*i).cost );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSlidingWindowThrottlePolicy::LoadState( const CDataNode& state, CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_loadedStarts.clear();
    const CDataNode* starts = state.FindChild( "starts" );
    if ( GUCEF_NULL == starts )
        return true;
    CDataNode::const_iterator i = starts->ConstBegin();
    while ( i != starts->ConstEnd() )
    {
        SStart start;
        start.monotonicMs = 0;
        start.utcMs = (*i)->GetAttributeValueOrChildValueByName( "utcMs" ).AsInt64( 0, true );
        start.cost = (*i)->GetAttributeValueOrChildValueByName( "cost" ).AsUInt32( 1, true );
        if ( 0 != start.utcMs )
            m_loadedStarts.push_back( start );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
