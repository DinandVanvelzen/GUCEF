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

#ifndef GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H
#define GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H

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
 *  Throttle policy of a lane: at most so many requests (the cost of the runs) per minute, per hour and per day, in
 *  sliding windows on the monotonic clock, and a minimum interval between two starts. A reserve keeps part of each limit
 *  for the urgent work: runs with a priority at or below ReserveFromPriority (numerically at or above it, 0 being the
 *  highest) see each limit L as max( 1, floor( L x ( 100 - ReservePercent ) / 100 ) ).
 *  0 = no limit for every figure. Bookkeeping only, behind a leaf lock of its own (the CIJobThrottlePolicy contract).
 *  The kept state holds the starts of the last day in UTC, so the windows hold across a restart.
 */
class GUCEF_CORE_PUBLIC_CPP CSlidingWindowThrottlePolicy : public CIJobThrottlePolicy
{
    public:

    CSlidingWindowThrottlePolicy( void );

    virtual ~CSlidingWindowThrottlePolicy() GUCEF_VIRTUAL_OVERRIDE;

    void SetRequestsPerMinute( UInt32 requestsPerMinute );
    UInt32 GetRequestsPerMinute( void ) const;

    void SetRequestsPerHour( UInt32 requestsPerHour );
    UInt32 GetRequestsPerHour( void ) const;

    void SetRequestsPerDay( UInt32 requestsPerDay );
    UInt32 GetRequestsPerDay( void ) const;

    void SetMinimumIntervalMs( UInt64 minimumIntervalMs );
    UInt64 GetMinimumIntervalMs( void ) const;

    void SetReservePercent( UInt32 reservePercent );
    UInt32 GetReservePercent( void ) const;

    void SetReserveFromPriority( UInt32 priority );
    UInt32 GetReserveFromPriority( void ) const;

    virtual void Check( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs, SJobPermitAnswer& answer ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnStarted( const SJobPermitRequest& request, Int64 nowUtcMs, UInt64 nowMonotonicMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnFinished( const SJobPermitRequest& request, UInt32 actualCost, TJobRunState outcome, UInt64 retryAfterMs, Int64 nowUtcMs ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetStatus( CDataNode& status, Int64 nowUtcMs ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveState( CDataNode& state ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadState( const CDataNode& state, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SStart
    {
        Int64 monotonicMs;              /**< may be below 0: a kept start older than this process's monotonic clock */
        Int64 utcMs;
        UInt32 cost;
    };
    typedef GUCEF::vector< SStart > TStartVector;

    UInt32 EffectiveLimit( UInt32 limit, UInt32 priority ) const;
    UInt64 WindowWaitMs( UInt64 windowMs, UInt32 limit, UInt32 cost, UInt64 nowMonotonicMs ) const;
    UInt32 UsedInWindow( UInt64 windowMs, UInt64 nowMonotonicMs ) const;
    void Prune( UInt64 nowMonotonicMs );
    void AdoptLoadedStarts( Int64 nowUtcMs, UInt64 nowMonotonicMs );

    CSlidingWindowThrottlePolicy( const CSlidingWindowThrottlePolicy& src );            /**< not implemented */
    CSlidingWindowThrottlePolicy& operator=( const CSlidingWindowThrottlePolicy& src ); /**< not implemented */

    private:

    MT::CMutex m_lock;
    UInt32 m_requestsPerMinute;
    UInt32 m_requestsPerHour;
    UInt32 m_requestsPerDay;
    UInt64 m_minimumIntervalMs;
    UInt32 m_reservePercent;
    UInt32 m_reserveFromPriority;
    TStartVector m_starts;              /**< oldest first, within the longest window */
    TStartVector m_loadedStarts;        /**< kept starts (UTC only) until the clocks are known */
    UInt64 m_lastMonotonicMs;
    Int64 m_lastUtcMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSLIDINGWINDOWTHROTTLEPOLICY_H ? */
