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

#include "gucefMULTIPLAY_MatchTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* ModeNames[] = { "LIVE", "ASYNC" };
const char* StatusNames[] = { "FORMING", "ACTIVE", "FINISHED", "ABANDONED", "CANCELLED", "INITIALIZING" };
const char* ParticipantStatusNames[] = { "INVITED", "ACTIVE", "DECLINED", "LEFT", "DROPPED", "AI_TAKEOVER", "SPECTATOR" };
const char* OutcomeNames[] = { "NONE", "WON", "LOST", "QUIT", "TIMED_OUT", "TIED" };
const char* PolicyNames[] = { "VOTE", "BOSS" };
const char* DecisionKindNames[] = { "HAND_TO_COMPUTER", "DROP_PARTICIPANT", "CHANGE_CONTACT", "SETTING_CHANGE", "FORCE_START", "KICK", "CANCEL", "MOVE_TO_SPECTATOR" };
const char* GroupNames[] = { "YOUR_TURN", "THEIR_TURN", "FORMING_OR_INVITED", "FINISHED" };
const char* EntryStateNames[] = { "FORMING", "INVITED", "CANCELLED", "INCOMPATIBLE", "SPECTATING", "YOUR_TURN", "THEIR_TURN", "FINISHED", "ABANDONED", "INITIALIZING" };

int
FindName( const char* const* names, int count, const CORE::CString& text, int defaultIndex )
{GUCEF_TRACE;

    CORE::CString upper = text.Uppercase();
    for ( int i=0; i<count; ++i )
        if ( upper == names[ i ] )
            return i;
    return defaultIndex;
}

}

/*-------------------------------------------------------------------------*/

const char*
MatchModeToString( TMatchMode mode )
{GUCEF_TRACE;

    return ( mode >= 0 && mode <= MATCHMODE_ASYNC ) ? ModeNames[ mode ] : ModeNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TMatchMode
StringToMatchMode( const CORE::CString& text )
{GUCEF_TRACE;

    return (TMatchMode) FindName( ModeNames, 2, text, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
MatchStatusToString( TMatchStatus status )
{GUCEF_TRACE;

    return ( status >= 0 && status <= MATCHSTATUS_INITIALIZING ) ? StatusNames[ status ] : StatusNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TMatchStatus
StringToMatchStatus( const CORE::CString& text )
{GUCEF_TRACE;

    return (TMatchStatus) FindName( StatusNames, 6, text, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
MatchParticipantStatusToString( TMatchParticipantStatus status )
{GUCEF_TRACE;

    return ( status >= 0 && status <= MATCHPARTICIPANT_SPECTATOR ) ? ParticipantStatusNames[ status ] : ParticipantStatusNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TMatchParticipantStatus
StringToMatchParticipantStatus( const CORE::CString& text )
{GUCEF_TRACE;

    return (TMatchParticipantStatus) FindName( ParticipantStatusNames, 7, text, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
MatchOutcomeToString( TMatchOutcome outcome )
{GUCEF_TRACE;

    return ( outcome >= 0 && outcome <= MATCHOUTCOME_TIED ) ? OutcomeNames[ outcome ] : OutcomeNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TMatchOutcome
StringToMatchOutcome( const CORE::CString& text )
{GUCEF_TRACE;

    return (TMatchOutcome) FindName( OutcomeNames, 6, text, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
MatchDecisionPolicyToString( TMatchDecisionPolicy policy )
{GUCEF_TRACE;

    return ( policy >= 0 && policy <= MATCHDECISIONPOLICY_BOSS ) ? PolicyNames[ policy ] : PolicyNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

TMatchDecisionPolicy
StringToMatchDecisionPolicy( const CORE::CString& text )
{GUCEF_TRACE;

    return (TMatchDecisionPolicy) FindName( PolicyNames, 2, text, 0 );
}

/*-------------------------------------------------------------------------*/

const char*
MatchDecisionKindToString( TMatchDecisionKind kind )
{GUCEF_TRACE;

    return ( kind >= 0 && kind <= MATCHDECISION_MOVE_TO_SPECTATOR ) ? DecisionKindNames[ kind ] : DecisionKindNames[ 0 ];
}

/*-------------------------------------------------------------------------*/

bool
TryStringToMatchDecisionKind( const CORE::CString& text, TMatchDecisionKind& kind )
{GUCEF_TRACE;

    int index = FindName( DecisionKindNames, 8, text, -1 );
    if ( index < 0 )
        return false;
    kind = (TMatchDecisionKind) index;
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
MatchGroupToString( TMatchGroup group )
{GUCEF_TRACE;

    return ( group >= 0 && group <= MATCHGROUP_FINISHED ) ? GroupNames[ group ] : GroupNames[ 0 ];
}

const char*
MatchEntryStateToString( TMatchEntryState state )
{GUCEF_TRACE;

    return ( state >= 0 && state <= MATCHENTRYSTATE_INITIALIZING ) ? EntryStateNames[ state ] : EntryStateNames[ 0 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
