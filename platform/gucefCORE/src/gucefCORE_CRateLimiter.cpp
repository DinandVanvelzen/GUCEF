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

#include <math.h>

#ifndef GUCEF_CORE_CRATELIMITER_H
#include "gucefCORE_CRateLimiter.h"
#define GUCEF_CORE_CRATELIMITER_H
#endif /* GUCEF_CORE_CRATELIMITER_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

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
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRateLimiter::CRateLimiter( Float64 ratePerSecond        ,
                            UInt32 burstSize             ,
                            Float64 jitterPercent        ,
                            const CIMonotonicClock* clock )
    : m_ratePerSecond( ratePerSecond > 0.0 ? ratePerSecond : 0.0 )
    , m_burstSize( burstSize )
    , m_jitterPercent( jitterPercent > 0.0 ? jitterPercent : 0.0 )
    , m_clock( GUCEF_NULL != clock ? clock : &CSystemMonotonicClock::Instance() )
    , m_tokens( (Float64) burstSize )
    , m_lastRefillInMs( 0 )
    , m_lock()
{GUCEF_TRACE;

    m_lastRefillInMs = m_clock->GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

CRateLimiter::~CRateLimiter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRateLimiter::Refill( void )
{GUCEF_TRACE;

    UInt64 nowInMs = m_clock->GetMonotonicTimeInMs();
    if ( nowInMs > m_lastRefillInMs )
    {
        m_tokens += ( (Float64) ( nowInMs - m_lastRefillInMs ) ) * m_ratePerSecond / 1000.0;
        if ( m_tokens > (Float64) m_burstSize )
            m_tokens = (Float64) m_burstSize;
    }
    m_lastRefillInMs = nowInMs;
}

/*-------------------------------------------------------------------------*/

bool
CRateLimiter::TryAcquire( UInt32 tokenCount )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Refill();
    if ( m_tokens >= (Float64) tokenCount )
    {
        m_tokens -= (Float64) tokenCount;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CRateLimiter::GetDelayUntilAvailableInMs( UInt32 tokenCount )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Refill();
    if ( m_tokens >= (Float64) tokenCount )
        return 0;
    if ( tokenCount > m_burstSize || m_ratePerSecond <= 0.0 )
        return GUCEF_UINT32MAX;

    // The epsilon keeps floating point noise (1.0000000000000009) from rounding up to an extra millisecond
    Float64 delayInMs = ceil( ( ( ( (Float64) tokenCount - m_tokens ) / m_ratePerSecond ) * 1000.0 ) - 1e-6 );
    if ( delayInMs < 1.0 )
        delayInMs = 1.0;
    if ( m_jitterPercent > 0.0 )
        delayInMs += ceil( delayInMs * ( m_jitterPercent / 100.0 ) * CRandom::GetRandomFloat64() );

    if ( delayInMs >= (Float64) GUCEF_UINT32MAX )
        return GUCEF_UINT32MAX;
    return (UInt32) delayInMs;
}

/*-------------------------------------------------------------------------*/

Float64
CRateLimiter::GetAvailableTokens( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Refill();
    return m_tokens;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
