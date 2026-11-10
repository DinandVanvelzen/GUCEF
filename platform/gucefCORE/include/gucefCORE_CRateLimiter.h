/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CRATELIMITER_H
#define GUCEF_CORE_CRATELIMITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

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
 *  Token bucket rate limiter.
 *  Tokens refill continuously at 'ratePerSecond' up to a capacity of 'burstSize' tokens.
 *  The bucket starts full. Thread-safe.
 *
 *  Typical use is to call TryAcquire() before an operation and, if refused, wait for
 *  GetDelayUntilAvailableInMs() before trying again (for example via CTaskConsumer::RequestTaskCycleDelayInMs)
 */
class GUCEF_CORE_PUBLIC_CPP CRateLimiter
{
    public:

    /**
     *  @param ratePerSecond    token refill rate, must be > 0 for the limiter to ever grant more than the initial burst
     *  @param burstSize        maximum number of tokens that can be accumulated
     *  @param jitterPercent    random extra delay of up to this percentage of the computed delay (0 = none),
     *                          used by GetDelayUntilAvailableInMs() to avoid callers retrying in lockstep
     *  @param clock            time source, must outlive the limiter. Defaults to the system clock
     */
    CRateLimiter( Float64 ratePerSecond                           ,
                  UInt32 burstSize                                ,
                  Float64 jitterPercent = 0.0                     ,
                  const CIMonotonicClock* clock = GUCEF_NULL      );

    ~CRateLimiter();

    /**
     *  Takes 'tokenCount' tokens if they are available right now
     */
    bool TryAcquire( UInt32 tokenCount = 1 );

    /**
     *  Milliseconds until 'tokenCount' tokens will be available, 0 if available now
     *  Returns GUCEF_UINT32MAX if that will never be the case (more than the burst size is requested or the rate is 0)
     *  Does not take any tokens
     */
    UInt32 GetDelayUntilAvailableInMs( UInt32 tokenCount = 1 );

    /**
     *  Currently available (whole and fractional) tokens
     */
    Float64 GetAvailableTokens( void );

    private:

    CRateLimiter( const CRateLimiter& src );
    CRateLimiter& operator=( const CRateLimiter& src );

    void Refill( void );

    private:

    Float64 m_ratePerSecond;
    UInt32 m_burstSize;
    Float64 m_jitterPercent;
    const CIMonotonicClock* m_clock;
    Float64 m_tokens;
    UInt64 m_lastRefillInMs;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CRATELIMITER_H ? */
