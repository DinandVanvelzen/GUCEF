/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#include "gucefWEB_CUserActionTracker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUserActionTracker::CUserActionTracker( void )
    : m_actionWindowMs( 3000 )
    , m_actionMaxDurationMs( 15000 )
    , m_quietGapMs( 1500 )
    , m_hasAction( false )
    , m_anyRequest( false )
    , m_actionStartMs( 0 )
    , m_lastActivityMs( 0 )
    , m_lastRequestMs( 0 )
    , m_lastAction()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUserActionTracker::Configure( const CSiteDiscoverySettings& settings )
{GUCEF_TRACE;

    SetTimes( settings.actionWindowMs, settings.actionMaxDurationMs, settings.quietGapMs );
}

/*-------------------------------------------------------------------------*/

void
CUserActionTracker::SetTimes( UInt32 actionWindowMs, UInt32 actionMaxDurationMs, UInt32 quietGapMs )
{GUCEF_TRACE;

    m_actionWindowMs = actionWindowMs;
    m_actionMaxDurationMs = actionMaxDurationMs;
    m_quietGapMs = quietGapMs;
}

/*-------------------------------------------------------------------------*/

void
CUserActionTracker::Reset( void )
{GUCEF_TRACE;

    m_hasAction = false;
    m_anyRequest = false;
    m_actionStartMs = 0;
    m_lastActivityMs = 0;
    m_lastRequestMs = 0;
    m_lastAction = CObservedUserAction();
}

/*-------------------------------------------------------------------------*/

const CObservedUserAction&
CUserActionTracker::StartAction( UInt64 nowMs, const CORE::CString& kind, const CORE::CString& label, const CORE::CString& page )
{GUCEF_TRACE;

    const UInt32 id = m_lastAction.id + 1;
    m_lastAction = CObservedUserAction();
    m_lastAction.id = id;
    m_lastAction.timestamp = nowMs / 1000.0;
    m_lastAction.kind = kind;
    m_lastAction.label = label;
    m_lastAction.page = page;
    m_hasAction = true;
    m_actionStartMs = nowMs;
    m_lastActivityMs = nowMs;
    return m_lastAction;
}

/*-------------------------------------------------------------------------*/

UInt32
CUserActionTracker::AssignRequest( UInt64 nowMs, bool& startedPauseAction )
{GUCEF_TRACE;

    startedPauseAction = false;
    const bool quietBefore = !m_anyRequest || nowMs - m_lastRequestMs >= m_quietGapMs;
    m_anyRequest = true;
    m_lastRequestMs = nowMs;

    if ( m_hasAction && nowMs - m_lastActivityMs <= m_actionWindowMs &&
         ( 0 == m_actionMaxDurationMs || nowMs - m_actionStartMs <= m_actionMaxDurationMs ) )
    {
        m_lastActivityMs = nowMs;
        return m_lastAction.id;
    }
    if ( quietBefore )
    {
        StartAction( nowMs, CObservedUserAction::KindPause, CORE::CString::Empty, CORE::CString::Empty );
        startedPauseAction = true;
        return m_lastAction.id;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

const CObservedUserAction&
CUserActionTracker::GetLastAction( void ) const
{GUCEF_TRACE;

    return m_lastAction;
}

/*-------------------------------------------------------------------------*/

UInt32
CUserActionTracker::GetActionCount( void ) const
{GUCEF_TRACE;

    return m_lastAction.id;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
