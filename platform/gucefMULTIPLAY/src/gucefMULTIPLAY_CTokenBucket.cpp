/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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


#include "gucefMULTIPLAY_CTokenBucket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTokenBucket::CTokenBucket( void )
    : m_perSecond( 0 )
    , m_burst( 0 )
    , m_milliTokens( 0 )
    , m_lastMs( 0 )
    , m_started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTokenBucket::CTokenBucket( UInt32 perSecond, UInt32 burst )
    : m_perSecond( 0 )
    , m_burst( 0 )
    , m_milliTokens( 0 )
    , m_lastMs( 0 )
    , m_started( false )
{GUCEF_TRACE;

    SetRate( perSecond, burst );
}

/*-------------------------------------------------------------------------*/

void
CTokenBucket::SetRate( UInt32 perSecond, UInt32 burst )
{GUCEF_TRACE;

    m_perSecond = perSecond;
    m_burst = burst > 0 ? burst : perSecond;
    m_milliTokens = (UInt64) m_burst * 1000;
    m_started = false;
}

/*-------------------------------------------------------------------------*/

bool
CTokenBucket::Take( UInt64 nowMs )
{GUCEF_TRACE;

    if ( 0 == m_perSecond )
        return true;
    if ( !m_started )
    {
        m_started = true;
        m_lastMs = nowMs;
    }
    if ( nowMs > m_lastMs )
    {
        // one token is 1000 milli tokens: a rate of n per second gives n milli tokens a millisecond
        UInt64 maxMilliTokens = (UInt64) m_burst * 1000;
        m_milliTokens += ( nowMs - m_lastMs ) * m_perSecond;
        if ( m_milliTokens > maxMilliTokens )
            m_milliTokens = maxMilliTokens;
        m_lastMs = nowMs;
    }
    if ( m_milliTokens < 1000 )
        return false;
    m_milliTokens -= 1000;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
