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

#ifndef GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H
#define GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#include "gucefCORE_CIJobThrottlePolicy.h"
#define GUCEF_CORE_CIJOBTHROTTLEPOLICY_H
#endif /* GUCEF_CORE_CIJOBTHROTTLEPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Throttle policy of a lane that spends a day's capacity evenly over what is left of the day:
 *      interval = max( pace, ( next day start - last start ) / ( capacity - spent + 1 ) )
 *  after the last start (spent counting it), so the starts of a whole day are next day start / capacity apart; measured
 *  on the monotonic clock (a jump of the calendar clock does not let requests through
 *  faster), and nothing once the day's capacity is spent, until the next day starts. Days start at UTC midnight plus
 *  DayStartOffsetMs. A reserve keeps part of the capacity for urgent work: runs with a priority at or below
 *  ReserveFromPriority (numerically at or above it) see max( 1, floor( capacity x ( 100 - ReservePercent ) / 100 ) ).
 *  Bookkeeping only, behind a leaf lock of its own (the CIJobThrottlePolicy contract). The kept state is the day and
 *  what was spent on it.
 */
class GUCEF_CORE_PUBLIC_CPP CDailyBudgetPacingPolicy : public CIJobThrottlePolicy
{
    public:

    CDailyBudgetPacingPolicy( void );

    virtual ~CDailyBudgetPacingPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** Requests (cost) per day; 0 = no budget (only the pace applies) */
    void SetDailyCapacity( UInt32 dailyCapacity );
    UInt32 GetDailyCapacity( void ) const;

    /** The shortest interval between two starts */
    void SetPaceMs( UInt64 paceMs );
    UInt64 GetPaceMs( void ) const;

    /** Where a day starts, after UTC midnight */
    void SetDayStartOffsetMs( Int64 dayStartOffsetMs );
    Int64 GetDayStartOffsetMs( void ) const;

    void SetReservePercent( UInt32 reservePercent );
    UInt32 GetReservePercent( void ) const;

    void SetReserveFromPriority( UInt32 priority );
    UInt32 GetReserveFromPriority( void ) const;

    /** Spent on the current day */
    UInt32 GetSpent( void ) const;

    virtual void Check( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, SJobPermitAnswer& answer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnStarted( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnFinished( const SJobPermitRequest& request, UInt32 actualCost, TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetStatus( CDataNode& status, Int64 nowUtcMs ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveState( CDataNode& state ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadState( const CDataNode& state, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Int64 DayIndexOf( Int64 utcMs ) const;
    Int64 NextDayStartUtcMs( Int64 dayIndex ) const;
    void RollDay( Int64 nowUtcMs );
    UInt32 EffectiveCapacity( UInt32 priority ) const;

    CDailyBudgetPacingPolicy( const CDailyBudgetPacingPolicy& src );            /**< not implemented */
    CDailyBudgetPacingPolicy& operator=( const CDailyBudgetPacingPolicy& src ); /**< not implemented */

    private:

    MT::CMutex m_lock;
    UInt32 m_dailyCapacity;
    UInt64 m_paceMs;
    Int64 m_dayStartOffsetMs;
    UInt32 m_reservePercent;
    UInt32 m_reserveFromPriority;
    Int64 m_dayIndex;
    UInt32 m_spent;
    bool m_hasLastStart;
    UInt64 m_lastStartMonotonicMs;
    Int64 m_lastStartUtcMs;
    Int64 m_lastStartDayIndex;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDAILYBUDGETPACINGPOLICY_H ? */
