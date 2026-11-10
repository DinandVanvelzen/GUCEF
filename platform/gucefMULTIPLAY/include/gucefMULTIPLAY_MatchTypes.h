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

#ifndef GUCEF_MULTIPLAY_MATCHTYPES_H
#define GUCEF_MULTIPLAY_MATCHTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The vocabulary of matches: games in progress that have an identity, participants, a turn holder and a revision,
 *  whatever carries their turns. Every enum has a stable upper case text form that is used in data nodes, in
 *  signed bytes and in the user interface layer; an unknown text gives the first value of the enum.
 */

/** Whether the players take turns at the same time online or at their own pace */
enum EMatchMode
{
    MATCHMODE_LIVE  = 0 ,
    MATCHMODE_ASYNC = 1
};
typedef enum EMatchMode TMatchMode;

enum EMatchStatus
{
    MATCHSTATUS_FORMING   = 0 ,     /**< invitations are out, not everyone has answered */
    MATCHSTATUS_ACTIVE    = 1 ,
    MATCHSTATUS_FINISHED  = 2 ,
    MATCHSTATUS_ABANDONED = 3 ,
    MATCHSTATUS_CANCELLED = 4 ,     /**< the creator (or a boss) closed the match before it started */
    MATCHSTATUS_INITIALIZING = 5    /**< the creator is setting the match up: the players have still to introduce themselves, no invitation was sent, no record exists. Only the creator has a match in this status, it is never sent to anybody */
};
typedef enum EMatchStatus TMatchStatus;

enum EMatchParticipantStatus
{
    MATCHPARTICIPANT_INVITED      = 0 ,
    MATCHPARTICIPANT_ACTIVE       = 1 ,
    MATCHPARTICIPANT_DECLINED     = 2 ,
    MATCHPARTICIPANT_LEFT         = 3 ,
    MATCHPARTICIPANT_DROPPED      = 4 ,
    MATCHPARTICIPANT_AI_TAKEOVER  = 5 ,
    MATCHPARTICIPANT_SPECTATOR    = 6       /**< receives the turns, holds no turn, has no seat, does not vote and is outside every denominator */
};
typedef enum EMatchParticipantStatus TMatchParticipantStatus;

enum EMatchOutcome
{
    MATCHOUTCOME_NONE      = 0 ,
    MATCHOUTCOME_WON       = 1 ,
    MATCHOUTCOME_LOST      = 2 ,
    MATCHOUTCOME_QUIT      = 3 ,
    MATCHOUTCOME_TIMED_OUT = 4 ,
    MATCHOUTCOME_TIED      = 5
};
typedef enum EMatchOutcome TMatchOutcome;

/** How decisions about a match are taken: a vote of the participants, or the command of an assigned boss */
enum EMatchDecisionPolicy
{
    MATCHDECISIONPOLICY_VOTE = 0 ,
    MATCHDECISIONPOLICY_BOSS = 1
};
typedef enum EMatchDecisionPolicy TMatchDecisionPolicy;

enum EMatchDecisionKind
{
    MATCHDECISION_HAND_TO_COMPUTER = 0 ,    /**< the computer plays the participant from now on */
    MATCHDECISION_DROP_PARTICIPANT = 1 ,
    MATCHDECISION_CHANGE_CONTACT   = 2 ,    /**< the contact of a participant, decided by that participant alone */
    MATCHDECISION_SETTING_CHANGE   = 3 ,
    MATCHDECISION_FORCE_START      = 4 ,    /**< before the start: the participants named in the targets are replaced by the computer and the match starts */
    MATCHDECISION_KICK             = 5 ,    /**< before the start: the participant is gone */
    MATCHDECISION_CANCEL           = 6 ,    /**< before the start: the creator (or a boss) closes the match, decided alone */
    MATCHDECISION_MOVE_TO_SPECTATOR = 7     /**< before the start: the participant watches instead of playing; the participant's own signature alone, or the rule of the table */
};
typedef enum EMatchDecisionKind TMatchDecisionKind;

/** The lists a match book presents */
/** The reason code of a move to spectator that the participant chose; every other code names what made the match unplayable for the participant */
#define GUCEF_MULTIPLAY_SPECTATOR_REASON_VOLUNTARY "VOLUNTARY"

/**
 *  What a list shows about a match besides its group: the state in one word. Cancelled and forming matches have a state of their own so that a
 *  list can label them (Forming: the creator waits for answers, Invited: the player was asked and has not answered)
 */
enum EMatchEntryState
{
    MATCHENTRYSTATE_FORMING    = 0 ,    /**< the match has not started, the player takes part */
    MATCHENTRYSTATE_INVITED    = 1 ,    /**< the player was asked and did not answer */
    MATCHENTRYSTATE_CANCELLED  = 2 ,
    MATCHENTRYSTATE_INCOMPATIBLE = 3 ,  /**< the player is a spectator because the game cannot be played here */
    MATCHENTRYSTATE_SPECTATING = 4 ,
    MATCHENTRYSTATE_YOUR_TURN  = 5 ,
    MATCHENTRYSTATE_THEIR_TURN = 6 ,
    MATCHENTRYSTATE_FINISHED   = 7 ,
    MATCHENTRYSTATE_ABANDONED  = 8 ,
    MATCHENTRYSTATE_INITIALIZING = 9    /**< the player made the match and it is being set up: the players have still to introduce themselves */
};
typedef enum EMatchEntryState TMatchEntryState;

enum EMatchGroup
{
    MATCHGROUP_YOUR_TURN          = 0 ,
    MATCHGROUP_THEIR_TURN         = 1 ,
    MATCHGROUP_FORMING_OR_INVITED = 2 ,
    MATCHGROUP_FINISHED           = 3
};
typedef enum EMatchGroup TMatchGroup;

/** What a provider can do; the user interface hides what a provider lacks. Combined as bit flags */
enum EMatchCapabilities
{
    MATCHCAPABILITY_LIVE         = 0x01 ,   /**< turns follow each other in real time */
    MATCHCAPABILITY_PREVIEW_POLL = 0x02 ,   /**< RefreshAll() and Refresh() are supported */
    MATCHCAPABILITY_PUSH         = 0x04 ,   /**< the provider tells when something changed */
    MATCHCAPABILITY_SERVER_CLOCK = 0x08 ,   /**< deadlines and automatic timeouts are enforced by a trusted clock */
    MATCHCAPABILITY_NUDGE        = 0x10 ,
    MATCHCAPABILITY_RESIGN       = 0x20 ,
    MATCHCAPABILITY_REMATCH      = 0x40 ,
    MATCHCAPABILITY_MULTI_DEVICE = 0x80
};
typedef enum EMatchCapabilities TMatchCapabilities;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchModeToString( TMatchMode mode );
GUCEF_MULTIPLAY_EXPORT_CPP TMatchMode StringToMatchMode( const CORE::CString& text );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchStatusToString( TMatchStatus status );
GUCEF_MULTIPLAY_EXPORT_CPP TMatchStatus StringToMatchStatus( const CORE::CString& text );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchParticipantStatusToString( TMatchParticipantStatus status );
GUCEF_MULTIPLAY_EXPORT_CPP TMatchParticipantStatus StringToMatchParticipantStatus( const CORE::CString& text );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchOutcomeToString( TMatchOutcome outcome );
GUCEF_MULTIPLAY_EXPORT_CPP TMatchOutcome StringToMatchOutcome( const CORE::CString& text );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchDecisionPolicyToString( TMatchDecisionPolicy policy );
GUCEF_MULTIPLAY_EXPORT_CPP TMatchDecisionPolicy StringToMatchDecisionPolicy( const CORE::CString& text );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchDecisionKindToString( TMatchDecisionKind kind );

/** @return false when the text names no kind */
GUCEF_MULTIPLAY_EXPORT_CPP bool TryStringToMatchDecisionKind( const CORE::CString& text, TMatchDecisionKind& kind );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchGroupToString( TMatchGroup group );

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchEntryStateToString( TMatchEntryState state );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_MATCHTYPES_H ? */
