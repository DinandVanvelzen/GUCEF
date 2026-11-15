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

#include "gucefCORE_CDailyBudgetPacingPolicy.h"

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

static const Int64 PacingDayMs = 86400000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDailyBudgetPacingPolicy::CDailyBudgetPacingPolicy( void )
    : CIJobThrottlePolicy()
    , m_lock()
    , m_dailyCapacity( 0 )
    , m_paceMs( 0 )
    , m_dayStartOffsetMs( 0 )
    , m_reservePercent( 0 )
    , m_reserveFromPriority( 2 )
    , m_dayIndex( -1 )
    , m_spent( 0 )
    , m_hasLastStart( false )
    , m_lastStartMonotonicMs( 0 )
    , m_lastStartUtcMs( 0 )
    , m_lastStartDayIndex( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDailyBudgetPacingPolicy::~CDailyBudgetPacingPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CDailyBudgetPacingPolicy::SetDailyCapacity( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_dailyCapacity = value; }
UInt32 CDailyBudgetPacingPolicy::GetDailyCapacity( void ) const {GUCEF_TRACE; return m_dailyCapacity; }
void CDailyBudgetPacingPolicy::SetPaceMs( UInt64 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_paceMs = value; }
UInt64 CDailyBudgetPacingPolicy::GetPaceMs( void ) const {GUCEF_TRACE; return m_paceMs; }
void CDailyBudgetPacingPolicy::SetDayStartOffsetMs( Int64 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_dayStartOffsetMs = value; }
Int64 CDailyBudgetPacingPolicy::GetDayStartOffsetMs( void ) const {GUCEF_TRACE; return m_dayStartOffsetMs; }
void CDailyBudgetPacingPolicy::SetReservePercent( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_reservePercent = value > 100 ? 100 : value; }
UInt32 CDailyBudgetPacingPolicy::GetReservePercent( void ) const {GUCEF_TRACE; return m_reservePercent; }
void CDailyBudgetPacingPolicy::SetReserveFromPriority( UInt32 value ) {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); m_reserveFromPriority = value; }
UInt32 CDailyBudgetPacingPolicy::GetReserveFromPriority( void ) const {GUCEF_TRACE; return m_reserveFromPriority; }
UInt32 CDailyBudgetPacingPolicy::GetSpent( void ) const {GUCEF_TRACE; MT::CScopeMutex lock( m_lock ); return m_spent; }

/*-------------------------------------------------------------------------*/

Int64
CDailyBudgetPacingPolicy::DayIndexOf( Int64 utcMs ) const
{GUCEF_TRACE;

    Int64 shifted = utcMs - m_dayStartOffsetMs;
    return shifted >= 0 ? shifted / PacingDayMs : ( ( shifted + 1 ) / PacingDayMs ) - 1;
}

/*-------------------------------------------------------------------------*/

Int64
CDailyBudgetPacingPolicy::NextDayStartUtcMs( Int64 dayIndex ) const
{GUCEF_TRACE;

    return ( dayIndex + 1 ) * PacingDayMs + m_dayStartOffsetMs;
}

/*-------------------------------------------------------------------------*/

void
CDailyBudgetPacingPolicy::RollDay( Int64 nowUtcMs )
{GUCEF_TRACE;

    // only forward: a calendar clock set back does not give the day's budget again
    Int64 dayIndex = DayIndexOf( nowUtcMs );
    if ( dayIndex > m_dayIndex )
    {
        m_dayIndex = dayIndex;
        m_spent = 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDailyBudgetPacingPolicy::EffectiveCapacity( UInt32 priority ) const
{GUCEF_TRACE;

    if ( 0 == m_dailyCapacity || 0 == m_reservePercent || priority < m_reserveFromPriority )
        return m_dailyCapacity;
    UInt32 kept = (UInt32) ( ( (UInt64) m_dailyCapacity * ( 100 - m_reservePercent ) ) / 100 );
    return kept < 1 ? 1 : kept;
}

/*-------------------------------------------------------------------------*/

void
CDailyBudgetPacingPolicy::Check( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, SJobPermitAnswer& answer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RollDay( nowUtcMs );

    UInt32 cost = request.estimatedCost > 0 ? request.estimatedCost : 1;
    Int64 msLeftInDay = NextDayStartUtcMs( m_dayIndex ) - nowUtcMs;
    if ( msLeftInDay < 1 )
        msLeftInDay = 1;
    answer.granted = true;
    answer.waitMs = 0;
    answer.bindingLimit.Clear();

    UInt64 intervalMs = m_paceMs;
    UInt32 capacity = EffectiveCapacity( request.priority );
    if ( 0 != capacity )
    {
        if ( m_spent + cost > capacity )
        {
            answer.granted = false;
            answer.waitMs = (UInt64) msLeftInDay;
            answer.bindingLimit = "dayBudget";
            return;
        }
        // the rest of the day from the last start of this day, spread over what is left including that start's slot
        if ( m_hasLastStart && m_lastStartDayIndex == m_dayIndex )
        {
            Int64 fromLastMs = NextDayStartUtcMs( m_dayIndex ) - m_lastStartUtcMs;
            UInt64 evenMs = fromLastMs > 0 ? (UInt64) fromLastMs / ( capacity - m_spent + 1 ) : 0;
            if ( evenMs > intervalMs )
                intervalMs = evenMs;
        }
    }

    if ( m_hasLastStart && nowMonotonicMs < m_lastStartMonotonicMs + intervalMs )
    {
        answer.granted = false;
        answer.waitMs = m_lastStartMonotonicMs + intervalMs - nowMonotonicMs;
        answer.bindingLimit = "pace";
    }
}

/*-------------------------------------------------------------------------*/

void
CDailyBudgetPacingPolicy::OnStarted( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RollDay( nowUtcMs );
    m_spent += request.estimatedCost > 0 ? request.estimatedCost : 1;
    m_hasLastStart = true;
    m_lastStartMonotonicMs = nowMonotonicMs;
    m_lastStartUtcMs = nowUtcMs;
    m_lastStartDayIndex = m_dayIndex;
}

/*-------------------------------------------------------------------------*/

void
CDailyBudgetPacingPolicy::OnFinished( const SJobPermitRequest& request, UInt32 actualCost, TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs )
{GUCEF_TRACE;

    // spent beyond the estimate (a hold reports its whole use here)
    UInt32 estimated = JOBRUNCAUSE_HOLD == request.cause ? 0 : ( request.estimatedCost > 0 ? request.estimatedCost : 1 );
    if ( actualCost <= estimated )
        return;
    MT::CScopeMutex lock( m_lock );
    RollDay( nowUtcMs );
    m_spent += actualCost - estimated;
}

/*-------------------------------------------------------------------------*/

void
CDailyBudgetPacingPolicy::GetStatus( CDataNode& status, Int64 nowUtcMs ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    status.SetNodeType( GUCEF_DATATYPE_OBJECT );
    status.SetAttribute( "dailyCapacity", m_dailyCapacity );
    status.SetAttribute( "spent", m_spent );
    status.SetAttribute( "paceMs", m_paceMs );
    status.SetAttribute( "nextDayStartUtcMs", NextDayStartUtcMs( m_dayIndex >= 0 ? m_dayIndex : DayIndexOf( nowUtcMs ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CDailyBudgetPacingPolicy::SaveState( CDataNode& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    state.Clear();
    state.SetNodeType( GUCEF_DATATYPE_OBJECT );
    state.SetAttribute( "dayIndex", m_dayIndex );
    state.SetAttribute( "spent", m_spent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDailyBudgetPacingPolicy::LoadState( const CDataNode& state, CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Int64 dayIndex = state.GetAttributeValueOrChildValueByName( "dayIndex" ).AsInt64( -1, true );
    if ( dayIndex >= m_dayIndex )
    {
        m_dayIndex = dayIndex;
        m_spent = state.GetAttributeValueOrChildValueByName( "spent" ).AsUInt32( 0, true );
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
