/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CReconnectBackoff.h"

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
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReconnectBackoff::CReconnectBackoff( void )
    : m_firstMaxMs( 250 )
    , m_baseMs( 500 )
    , m_capMs( 30000 )
    , m_stableResetMs( 60000 )
    , m_minSpreadMs( 1000 )
    , m_attempt( 0 )
    , m_connectedSinceMs( 0 )
    , m_connected( false )
    , m_state( 0 )
{GUCEF_TRACE;

    UInt32 seed = 0;
    if ( !CORE::CRandom::GetRandomBytes( &seed, sizeof( seed ) ) || 0 == seed )
        seed = 0x9E3779B9;
    m_state = seed;
}

/*-------------------------------------------------------------------------*/

void
CReconnectBackoff::SetPolicy( UInt32 firstMaxMs, UInt32 baseMs, UInt32 capMs, UInt32 stableResetMs, UInt32 minSpreadMs )
{GUCEF_TRACE;

    m_firstMaxMs = firstMaxMs;
    m_baseMs = 0 == baseMs ? 1 : baseMs;
    m_capMs = capMs < m_baseMs ? m_baseMs : capMs;
    m_stableResetMs = stableResetMs;
    m_minSpreadMs = minSpreadMs;
}

/*-------------------------------------------------------------------------*/

void
CReconnectBackoff::SetSeed( UInt32 seed )
{GUCEF_TRACE;

    m_state = 0 == seed ? 0x9E3779B9 : seed;
}

/*-------------------------------------------------------------------------*/

UInt32
CReconnectBackoff::Random( UInt32 upperInclusive )
{GUCEF_TRACE;

    // xorshift32: small, seedable, good enough to spread clients over a window
    UInt32 x = m_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_state = x;
    if ( GUCEF_UINT32MAX == upperInclusive )
        return x;
    return x % ( upperInclusive + 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CReconnectBackoff::GetBoundMs( UInt32 attempt ) const
{GUCEF_TRACE;

    if ( 0 == attempt )
        return m_firstMaxMs;
    UInt64 bound = m_baseMs;
    for ( UInt32 i=1; i<attempt && bound < m_capMs; ++i )
        bound *= 2;
    return bound > m_capMs ? m_capMs : (UInt32) bound;
}

/*-------------------------------------------------------------------------*/

UInt32
CReconnectBackoff::NextDelayMs( UInt32 hintMs )
{GUCEF_TRACE;

    m_connected = false;
    UInt32 delay = 0;
    if ( hintMs > 0 )
    {
        UInt32 spread = hintMs / 2 > m_minSpreadMs ? hintMs / 2 : m_minSpreadMs;
        delay = hintMs + Random( spread );
    }
    else
    {
        delay = Random( GetBoundMs( m_attempt ) );
    }
    if ( m_attempt < 64 )
        ++m_attempt;
    return delay;
}

/*-------------------------------------------------------------------------*/

void
CReconnectBackoff::OnConnected( UInt64 nowMs )
{GUCEF_TRACE;

    m_connected = true;
    m_connectedSinceMs = nowMs;
}

/*-------------------------------------------------------------------------*/

void
CReconnectBackoff::OnStillConnected( UInt64 nowMs )
{GUCEF_TRACE;

    if ( !m_connected )
    {
        OnConnected( nowMs );
        return;
    }
    if ( 0 != m_attempt && nowMs >= m_connectedSinceMs && nowMs - m_connectedSinceMs >= m_stableResetMs )
        m_attempt = 0;
}

/*-------------------------------------------------------------------------*/

void
CReconnectBackoff::Reset( void )
{GUCEF_TRACE;

    m_attempt = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CReconnectBackoff::GetAttempt( void ) const
{GUCEF_TRACE;

    return m_attempt;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
