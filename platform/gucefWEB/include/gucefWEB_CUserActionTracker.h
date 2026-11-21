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

#ifndef GUCEF_WEB_CUSERACTIONTRACKER_H
#define GUCEF_WEB_CUSERACTIONTRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_COBSERVEDUSERACTION_H
#include "gucefWEB_CObservedUserAction.h"
#define GUCEF_WEB_COBSERVEDUSERACTION_H
#endif /* GUCEF_WEB_COBSERVEDUSERACTION_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#include "gucefWEB_CSiteDiscoverySettings.h"
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decides which user action a request followed, so that the requests behind a button (an export, a search, the next
 *  page) can be told apart from the rest. The heuristic:
 *
 *  1. An action starts with StartAction(): a navigation of a tab's main frame, an in-page navigation of a single page
 *     application, or a click or submit reported by the optional click listener.
 *  2. A request belongs to the latest action when it starts within the action window of the action's start or of the
 *     action's latest request, so the chain of requests one click causes stays together, and no later than the
 *     maximum duration after the start, so polling does not keep an action open.
 *  3. A request outside every action that follows a quiet gap without requests starts an implicit action
 *     "activity-after-pause"; other requests are background (action 0).
 *
 *  Times are milliseconds on the caller's clock; the recorder uses the arrival time of the events, which arrive in
 *  order over one connection. Pure logic without a clock of its own, so it can be tested with explicit times.
 */
class GUCEF_WEB_PUBLIC_CPP CUserActionTracker
{
    public:

    CUserActionTracker( void );

    /** Takes actionWindowMs, actionMaxDurationMs and quietGapMs */
    void Configure( const CSiteDiscoverySettings& settings );

    void SetTimes( UInt32 actionWindowMs, UInt32 actionMaxDurationMs, UInt32 quietGapMs );

    /** Starts an action; the id is the next one */
    const CObservedUserAction& StartAction( UInt64 nowMs, const CORE::CString& kind, const CORE::CString& label, const CORE::CString& page );

    /**
     *  The action a request that starts now belongs to, 0 for none
     *  @param startedPauseAction set when the request started an implicit action, which GetLastAction() then gives
     */
    UInt32 AssignRequest( UInt64 nowMs, bool& startedPauseAction );

    const CObservedUserAction& GetLastAction( void ) const;

    /** The number of actions started so far */
    UInt32 GetActionCount( void ) const;

    void Reset( void );

    private:

    UInt32 m_actionWindowMs;
    UInt32 m_actionMaxDurationMs;
    UInt32 m_quietGapMs;
    bool m_hasAction;
    bool m_anyRequest;
    UInt64 m_actionStartMs;
    UInt64 m_lastActivityMs;
    UInt64 m_lastRequestMs;
    CObservedUserAction m_lastAction;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CUSERACTIONTRACKER_H ? */
