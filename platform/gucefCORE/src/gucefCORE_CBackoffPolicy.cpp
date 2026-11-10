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

#ifndef GUCEF_CORE_CBACKOFFPOLICY_H
#include "gucefCORE_CBackoffPolicy.h"
#define GUCEF_CORE_CBACKOFFPOLICY_H
#endif /* GUCEF_CORE_CBACKOFFPOLICY_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

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

CBackoffPolicy::CBackoffPolicy( UInt32 baseDelayInMs ,
                                Float64 factor       ,
                                UInt32 maxDelayInMs  ,
                                Float64 jitterPercent )
    : m_baseDelayInMs( baseDelayInMs )
    , m_factor( factor > 1.0 ? factor : 1.0 )
    , m_maxDelayInMs( maxDelayInMs )
    , m_jitterPercent( jitterPercent > 0.0 ? ( jitterPercent < 100.0 ? jitterPercent : 100.0 ) : 0.0 )
    , m_attemptCount( 0 )
    , m_nextUnjitteredDelayInMs( (Float64) baseDelayInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CBackoffPolicy::NextDelayInMs( void )
{GUCEF_TRACE;

    Float64 delayInMs = m_nextUnjitteredDelayInMs;
    if ( delayInMs > (Float64) m_maxDelayInMs )
        delayInMs = (Float64) m_maxDelayInMs;

    // Advance the series, staying capped so it can never overflow
    m_nextUnjitteredDelayInMs = delayInMs * m_factor;
    if ( m_nextUnjitteredDelayInMs > (Float64) m_maxDelayInMs )
        m_nextUnjitteredDelayInMs = (Float64) m_maxDelayInMs;
    if ( m_attemptCount < GUCEF_UINT32MAX )
        ++m_attemptCount;

    if ( m_jitterPercent > 0.0 )
    {
        // random factor in [-1,1)
        Float64 randomFactor = ( CRandom::GetRandomFloat64() * 2.0 ) - 1.0;
        delayInMs += delayInMs * ( m_jitterPercent / 100.0 ) * randomFactor;
        if ( delayInMs < 0.0 )
            delayInMs = 0.0;
        if ( delayInMs > (Float64) m_maxDelayInMs )
            delayInMs = (Float64) m_maxDelayInMs;
    }
    return (UInt32) delayInMs;
}

/*-------------------------------------------------------------------------*/

void
CBackoffPolicy::Reset( void )
{GUCEF_TRACE;

    m_attemptCount = 0;
    m_nextUnjitteredDelayInMs = (Float64) m_baseDelayInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CBackoffPolicy::GetAttemptCount( void ) const
{GUCEF_TRACE;

    return m_attemptCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
