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

#ifndef GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#define GUCEF_DRNUTILS_CRECONNECTBACKOFF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The reconnect delays of a client (gucefASSETSTORE_RemoteNetworking_Design.md 5.2): the first attempt after a lost link
 *  after random( 0, firstMaxMs ); then full-jitter exponential backoff, random( 0, min( capMs, baseMs * 2^(n-1) ) ); a
 *  hint of the service (retry after, expected back) is a lower bound: hint + random( 0, max( hint / 2, minSpreadMs ) ).
 *  The exponent resets once the connection stayed up for stableResetMs (OnConnected() and OnStillConnected()), not at the
 *  first success. The random source is a small seeded generator (SetSeed() for tests; seeded from CORE::CRandom by
 *  default). Not thread safe: the connection's thread.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CReconnectBackoff
{
    public:

    CReconnectBackoff( void );

    /** Defaults 250, 500, 30000, 60000, 1000 */
    void SetPolicy( UInt32 firstMaxMs, UInt32 baseMs, UInt32 capMs, UInt32 stableResetMs, UInt32 minSpreadMs );

    void SetSeed( UInt32 seed );

    /** The delay before the next attempt, counting the attempt; 'hintMs' 0 for none */
    UInt32 NextDelayMs( UInt32 hintMs = 0 );

    /** The connection came up ('nowMs' monotonic) */
    void OnConnected( UInt64 nowMs );

    /** The connection is still up: resets the exponent once it stayed up for stableResetMs */
    void OnStillConnected( UInt64 nowMs );

    /** Back to the first attempt */
    void Reset( void );

    /** Attempts since the last reset */
    UInt32 GetAttempt( void ) const;

    /** The upper bound of the delay of an attempt without a hint (for tests and the GUI) */
    UInt32 GetBoundMs( UInt32 attempt ) const;

    private:

    UInt32 Random( UInt32 upperInclusive );

    UInt32 m_firstMaxMs;
    UInt32 m_baseMs;
    UInt32 m_capMs;
    UInt32 m_stableResetMs;
    UInt32 m_minSpreadMs;
    UInt32 m_attempt;
    UInt64 m_connectedSinceMs;
    bool m_connected;
    UInt32 m_state;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CRECONNECTBACKOFF_H ? */
