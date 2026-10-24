/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CLobbyRules.h"

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

CLobbyRules::CLobbyRules( void )
    : CORE::CIConfigurable()
    , minPlayers( 1 )
    , maxPlayers( 0 )
    , allowSpectators( true )
    , requireAllReady( true )
    , allowForceStart( false )
    , startByVote( true )
    , countdownInSecs( 5 )
    , voteDurationInSecs( 30 )
    , closeWhenHostLeaves( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyRules::CLobbyRules( const CLobbyRules& src )
    : CORE::CIConfigurable( src )
    , minPlayers( src.minPlayers )
    , maxPlayers( src.maxPlayers )
    , allowSpectators( src.allowSpectators )
    , requireAllReady( src.requireAllReady )
    , allowForceStart( src.allowForceStart )
    , startByVote( src.startByVote )
    , countdownInSecs( src.countdownInSecs )
    , voteDurationInSecs( src.voteDurationInSecs )
    , closeWhenHostLeaves( src.closeWhenHostLeaves )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyRules::~CLobbyRules()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyRules&
CLobbyRules::operator=( const CLobbyRules& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        minPlayers = src.minPlayers;
        maxPlayers = src.maxPlayers;
        allowSpectators = src.allowSpectators;
        requireAllReady = src.requireAllReady;
        allowForceStart = src.allowForceStart;
        startByVote = src.startByVote;
        countdownInSecs = src.countdownInSecs;
        voteDurationInSecs = src.voteDurationInSecs;
        closeWhenHostLeaves = src.closeWhenHostLeaves;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CLobbySettingDef*
CLobbyRules::FindSettingDef( const CLobbySettingsSchema& schema, const CORE::CString& key, CORE::CString& ownerId )
{GUCEF_TRACE;

    ownerId.Clear();
    const CLobbySettingDef* def = schema.FindSetting( key );
    if ( GUCEF_NULL != def )
        return def;

    // A per member setting: <memberId>/<key of the schema>
    Int32 slash = key.HasChar( '/', true );
    if ( slash <= 0 )
        return GUCEF_NULL;
    def = schema.FindSetting( key.CutChars( (UInt32) slash + 1, true ) );
    if ( GUCEF_NULL == def || CLobbySettingDef::CHANGEPOLICY_OWNER != def->changePolicy )
        return GUCEF_NULL;
    ownerId = key.SubstrToIndex( (UInt32) slash, true );
    return def;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckJoin( const CLobbyState& state, bool asSpectator, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_CLOSED == state.phase )
    {
        reason = "The lobby is closed";
        return false;
    }
    if ( asSpectator )
    {
        if ( !allowSpectators )
        {
            reason = "The lobby does not allow spectators";
            return false;
        }
        return true;
    }
    if ( LOBBYPHASE_OPEN != state.phase && LOBBYPHASE_COUNTDOWN != state.phase )
    {
        reason = "The game is under way, it can only be joined as a spectator";
        return false;
    }
    if ( 0 != maxPlayers && state.GetPlayerCount() >= maxPlayers )
    {
        reason = "The lobby is full";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckSetting( const CLobbyState& state                 ,
                           const CLobbySettingsSchema& schema       ,
                           const CORE::CString& memberId            ,
                           const CORE::CString& key                 ,
                           const CORE::CString& value               ,
                           bool& needsVote                          ,
                           CORE::CString& reason                    ) const
{GUCEF_TRACE;

    needsVote = false;
    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Settings can only change while the lobby is open";
        return false;
    }
    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
    {
        reason = "Not a member of the lobby";
        return false;
    }
    CORE::CString ownerId;
    const CLobbySettingDef* def = FindSettingDef( schema, key, ownerId );
    if ( GUCEF_NULL == def )
    {
        reason = "There is no setting " + key;
        return false;
    }
    if ( !def->IsValidValue( value, reason ) )
        return false;

    bool isBoss = state.bossId == memberId;
    switch ( def->changePolicy )
    {
        case CLobbySettingDef::CHANGEPOLICY_ANY:
            return true;
        case CLobbySettingDef::CHANGEPOLICY_OWNER:
        {
            // A member's own setting is tagged with the id of the member: <memberId>/<key>
            if ( ownerId == memberId )
                return true;
            if ( ownerId.IsNULLOrEmpty() )
                reason = "The setting " + key + " is set per member as <memberId>/" + key;
            else
                reason = "The setting " + key + " belongs to another member";
            return false;
        }
        case CLobbySettingDef::CHANGEPOLICY_VOTE:
        {
            if ( isBoss )
                return true;
            if ( member->IsSpectator() )
            {
                reason = "Spectators do not propose settings";
                return false;
            }
            if ( state.activeVote.IsActive() )
            {
                reason = "A vote is already running";
                return false;
            }
            needsVote = true;
            return true;
        }
        default:
        {
            if ( isBoss )
                return true;
            reason = "Only the boss changes " + key;
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckProposal( const CLobbyState& state                 ,
                            const CLobbySettingsSchema& schema       ,
                            const CORE::CString& memberId            ,
                            const CORE::CString& key                 ,
                            const CORE::CString& value               ,
                            CORE::CString& reason                    ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Proposals can only be made while the lobby is open";
        return false;
    }
    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member || member->IsSpectator() )
    {
        reason = "Only players propose settings";
        return false;
    }
    if ( state.activeVote.IsActive() )
    {
        reason = "A vote is already running";
        return false;
    }
    CORE::CString ownerId;
    const CLobbySettingDef* def = FindSettingDef( schema, key, ownerId );
    if ( GUCEF_NULL == def )
    {
        reason = "There is no setting " + key;
        return false;
    }
    if ( CLobbySettingDef::CHANGEPOLICY_OWNER == def->changePolicy )
    {
        reason = "The setting " + key + " belongs to a member, it is not voted on";
        return false;
    }
    return def->IsValidValue( value, reason );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckVote( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& voteId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !state.activeVote.IsActive() || state.activeVote.id != voteId )
    {
        reason = "There is no such vote running";
        return false;
    }
    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member || member->IsSpectator() )
    {
        reason = "Only players vote";
        return false;
    }
    if ( state.activeVote.voters.find( memberId ) != state.activeVote.voters.end() )
    {
        reason = "Already voted";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckSlotRequest( const CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Slots can only change while the lobby is open";
        return false;
    }
    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
    {
        reason = "Not a member of the lobby";
        return false;
    }
    const CLobbySlot* slot = state.FindSlot( slotIndex );
    if ( GUCEF_NULL == slot )
    {
        reason = "There is no such slot";
        return false;
    }
    if ( slot->memberId == memberId )
        return true;
    if ( !slot->IsFree() )
    {
        reason = "The slot is not free";
        return false;
    }
    if ( member->IsSpectator() && 0 != maxPlayers && state.GetPlayerCount() >= maxPlayers )
    {
        reason = "There is no room for another player";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckFaction( const CLobbyState& state, const CLobbySettingsSchema& schema, const CORE::CString& memberId, const CORE::CString& faction, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Factions can only change while the lobby is open";
        return false;
    }
    if ( GUCEF_NULL == state.FindSlotOfMember( memberId ) )
    {
        reason = "Only a member in a slot picks a faction";
        return false;
    }
    if ( schema.factions.empty() )
        return true;
    for ( size_t i=0; i<schema.factions.size(); ++i )
        if ( schema.factions[ i ] == faction )
            return true;
    reason = "\"" + faction + "\" is not a faction of this game";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckColour( const CLobbyState& state, const CLobbySettingsSchema& schema, const CORE::CString& memberId, const CORE::CString& colour, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Colours can only change while the lobby is open";
        return false;
    }
    const CLobbySlot* ownSlot = state.FindSlotOfMember( memberId );
    if ( GUCEF_NULL == ownSlot )
    {
        reason = "Only a member in a slot picks a colour";
        return false;
    }
    if ( !schema.colours.empty() )
    {
        bool allowed = false;
        for ( size_t i=0; i<schema.colours.size() && !allowed; ++i )
            allowed = schema.colours[ i ] == colour;
        if ( !allowed )
        {
            reason = "\"" + colour + "\" is not a colour of this game";
            return false;
        }
    }
    if ( schema.uniqueColours )
    {
        for ( size_t i=0; i<state.slots.size(); ++i )
        {
            if ( state.slots[ i ].index != ownSlot->index && state.slots[ i ].colour == colour && !state.slots[ i ].IsFree() )
            {
                reason = "Another slot has the colour \"" + colour + "\"";
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckSlotKind( const CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex, TLobbySlotKind kind, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( state.bossId != memberId )
    {
        reason = "Only the boss changes slots";
        return false;
    }
    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Slots can only change while the lobby is open";
        return false;
    }
    const CLobbySlot* slot = state.FindSlot( slotIndex );
    if ( GUCEF_NULL == slot )
    {
        reason = "There is no such slot";
        return false;
    }
    if ( LOBBYSLOTKIND_HUMAN == kind )
    {
        reason = "A slot becomes human when a member takes it";
        return false;
    }
    if ( !slot->memberId.IsNULLOrEmpty() )
    {
        reason = "A member is in the slot";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckReady( const CLobbyState& state, const CORE::CString& memberId, CORE::CString& reason ) const
{GUCEF_TRACE;

    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
    {
        reason = "Not a member of the lobby";
        return false;
    }
    if ( member->IsSpectator() )
    {
        reason = "Spectators do not get ready";
        return false;
    }
    if ( LOBBYPHASE_OPEN != state.phase && LOBBYPHASE_COUNTDOWN != state.phase )
    {
        reason = "The game is already under way";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckSpectator( const CLobbyState& state, const CORE::CString& memberId, bool spectate, CORE::CString& reason ) const
{GUCEF_TRACE;

    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
    {
        reason = "Not a member of the lobby";
        return false;
    }
    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "Roles can only change while the lobby is open";
        return false;
    }
    if ( spectate )
    {
        if ( state.bossId == memberId )
        {
            reason = "The boss plays: pass the boss on first";
            return false;
        }
        if ( !allowSpectators )
        {
            reason = "The lobby does not allow spectators";
            return false;
        }
        return true;
    }
    if ( member->IsSpectator() && 0 != maxPlayers && state.GetPlayerCount() >= maxPlayers )
    {
        reason = "There is no room for another player";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckKick( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& targetId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( state.bossId != memberId )
    {
        reason = "Only the boss kicks";
        return false;
    }
    if ( targetId == memberId || GUCEF_NULL == state.FindMember( targetId ) )
    {
        reason = "There is no such other member";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckTransferBoss( const CLobbyState& state, const CORE::CString& memberId, const CORE::CString& targetId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( state.bossId != memberId )
    {
        reason = "Only the boss passes the boss on";
        return false;
    }
    const CLobbyMember* target = state.FindMember( targetId );
    if ( GUCEF_NULL == target || targetId == memberId || target->IsSpectator() )
    {
        reason = "The boss passes to another player";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::IsStartAllowed( const CLobbyState& state, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( LOBBYPHASE_OPEN != state.phase )
    {
        reason = "The lobby is not open";
        return false;
    }
    if ( state.GetPlayerCount() < minPlayers )
    {
        reason = "Not enough players";
        return false;
    }
    if ( requireAllReady )
    {
        for ( size_t i=0; i<state.members.size(); ++i )
        {
            const CLobbyMember& member = state.members[ i ];
            if ( !member.IsSpectator() && !member.ready )
            {
                reason = member.name + " is not ready";
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::CheckStart( const CLobbyState& state, const CORE::CString& memberId, bool force, bool& needsVote, CORE::CString& reason ) const
{GUCEF_TRACE;

    needsVote = false;
    const CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member || member->IsSpectator() )
    {
        reason = "Only players start the game";
        return false;
    }
    bool isBoss = state.bossId == memberId;

    if ( force )
    {
        if ( !isBoss || !allowForceStart )
        {
            reason = "A force start is not allowed";
            return false;
        }
        if ( LOBBYPHASE_OPEN != state.phase )
        {
            reason = "The lobby is not open";
            return false;
        }
        return true;
    }

    if ( !IsStartAllowed( state, reason ) )
        return false;
    if ( isBoss )
        return true;
    if ( !startByVote )
    {
        reason = "Only the boss starts the game";
        return false;
    }
    if ( state.activeVote.IsActive() )
    {
        reason = "A vote is already running";
        return false;
    }
    needsVote = true;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CLobbyRules::GetVotesNeeded( const CLobbyState& state ) const
{GUCEF_TRACE;

    return state.GetPlayerCount() / 2 + 1;
}

/*-------------------------------------------------------------------------*/

CLobbyRules::TVoteOutcome
CLobbyRules::EvaluateVote( const CLobbyState& state ) const
{GUCEF_TRACE;

    const CLobbyVote& vote = state.activeVote;
    if ( !vote.IsActive() )
        return VOTEOUTCOME_UNDECIDED;
    if ( vote.yes >= vote.needed )
        return VOTEOUTCOME_PASSED;

    // It fails once the yes votes that are left cannot reach what is needed
    UInt32 players = state.GetPlayerCount();
    UInt32 notYetVoted = players > ( vote.yes + vote.no ) ? players - ( vote.yes + vote.no ) : 0;
    if ( vote.yes + notYetVoted < vote.needed )
        return VOTEOUTCOME_FAILED;
    return VOTEOUTCOME_UNDECIDED;
}

/*-------------------------------------------------------------------------*/

Int32
CLobbyRules::FindFreeSlot( const CLobbyState& state )
{GUCEF_TRACE;

    for ( size_t i=0; i<state.slots.size(); ++i )
        if ( state.slots[ i ].IsFree() )
            return (Int32) state.slots[ i ].index;
    return -1;
}

/*-------------------------------------------------------------------------*/

CLobbyMember&
CLobbyRules::ApplyJoin( CLobbyState& state, const CORE::CString& memberId, const CORE::CString& name, bool asSpectator ) const
{GUCEF_TRACE;

    UInt32 joinOrder = 0;
    for ( size_t i=0; i<state.members.size(); ++i )
        if ( state.members[ i ].joinOrder >= joinOrder )
            joinOrder = state.members[ i ].joinOrder + 1;

    CLobbyMember member;
    member.id = memberId;
    member.name = name;
    member.joinOrder = joinOrder;
    if ( asSpectator )
        member.role = LOBBYMEMBERROLE_SPECTATOR;
    else
    if ( state.bossId.IsNULLOrEmpty() )
    {
        member.role = LOBBYMEMBERROLE_BOSS;
        state.bossId = memberId;
    }
    else
        member.role = LOBBYMEMBERROLE_PLAYER;
    state.members.push_back( member );

    if ( !asSpectator )
    {
        Int32 freeSlot = FindFreeSlot( state );
        if ( freeSlot >= 0 )
            ApplySlot( state, memberId, (UInt32) freeSlot );
    }
    return *state.FindMember( memberId );
}

/*-------------------------------------------------------------------------*/

void
CLobbyRules::ApplySlot( CLobbyState& state, const CORE::CString& memberId, UInt32 slotIndex ) const
{GUCEF_TRACE;

    CLobbyMember* member = state.FindMember( memberId );
    CLobbySlot* slot = state.FindSlot( slotIndex );
    if ( GUCEF_NULL == member || GUCEF_NULL == slot )
        return;

    CLobbySlot* oldSlot = state.FindSlotOfMember( memberId );
    if ( GUCEF_NULL != oldSlot && oldSlot != slot )
    {
        oldSlot->memberId.Clear();
        oldSlot->kind = LOBBYSLOTKIND_OPEN;
    }

    slot->memberId = memberId;
    slot->kind = LOBBYSLOTKIND_HUMAN;
    member->slot = (Int32) slotIndex;
    if ( member->IsSpectator() )
        member->role = LOBBYMEMBERROLE_PLAYER;
}

/*-------------------------------------------------------------------------*/

void
CLobbyRules::ApplySpectator( CLobbyState& state, const CORE::CString& memberId, bool spectate ) const
{GUCEF_TRACE;

    CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
        return;

    if ( spectate )
    {
        CLobbySlot* slot = state.FindSlotOfMember( memberId );
        if ( GUCEF_NULL != slot )
        {
            slot->memberId.Clear();
            slot->kind = LOBBYSLOTKIND_OPEN;
        }
        member->slot = -1;
        member->ready = false;
        member->role = LOBBYMEMBERROLE_SPECTATOR;
        return;
    }

    if ( member->IsSpectator() )
    {
        member->role = LOBBYMEMBERROLE_PLAYER;
        Int32 freeSlot = FindFreeSlot( state );
        if ( freeSlot >= 0 )
            ApplySlot( state, memberId, (UInt32) freeSlot );
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbyRules::ApplyReady( CLobbyState& state, const CORE::CString& memberId, bool ready, bool& countdownCancelled ) const
{GUCEF_TRACE;

    countdownCancelled = false;
    CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
        return;
    member->ready = ready;
    if ( !ready && LOBBYPHASE_COUNTDOWN == state.phase )
    {
        state.phase = LOBBYPHASE_OPEN;
        countdownCancelled = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbyRules::ApplyTransferBoss( CLobbyState& state, const CORE::CString& targetId ) const
{GUCEF_TRACE;

    CLobbyMember* oldBoss = state.FindMember( state.bossId );
    if ( GUCEF_NULL != oldBoss )
        oldBoss->role = LOBBYMEMBERROLE_PLAYER;
    CLobbyMember* newBoss = state.FindMember( targetId );
    if ( GUCEF_NULL != newBoss )
    {
        newBoss->role = LOBBYMEMBERROLE_BOSS;
        state.bossId = targetId;
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbyRules::ApplyLeave( CLobbyState& state, const CORE::CString& memberId, bool isHost, TLeaveOutcome& outcome ) const
{GUCEF_TRACE;

    outcome.lobbyClosed = false;
    outcome.newBossId.Clear();
    outcome.voteCancelled = false;
    outcome.countdownCancelled = false;

    CLobbyMember* member = state.FindMember( memberId );
    if ( GUCEF_NULL == member )
        return;
    bool wasPlayer = !member->IsSpectator();
    bool wasBoss = state.bossId == memberId;

    CLobbySlot* slot = state.FindSlotOfMember( memberId );
    if ( GUCEF_NULL != slot )
    {
        slot->memberId.Clear();
        slot->kind = LOBBYSLOTKIND_OPEN;
    }
    for ( size_t i=0; i<state.members.size(); ++i )
    {
        if ( state.members[ i ].id == memberId )
        {
            state.members.erase( state.members.begin() + i );
            break;
        }
    }

    // A vote ends with its proposer
    if ( state.activeVote.IsActive() && state.activeVote.proposer == memberId )
    {
        state.activeVote.Clear();
        outcome.voteCancelled = true;
    }

    if ( wasPlayer && LOBBYPHASE_COUNTDOWN == state.phase )
    {
        state.phase = LOBBYPHASE_OPEN;
        outcome.countdownCancelled = true;
    }

    if ( state.members.empty() || ( isHost && closeWhenHostLeaves ) )
    {
        state.phase = LOBBYPHASE_CLOSED;
        outcome.lobbyClosed = true;
        if ( wasBoss )
            state.bossId.Clear();
        return;
    }

    if ( wasBoss )
    {
        // To the longest present player, a spectator only when there is no player left
        const CLobbyMember* successor = GUCEF_NULL;
        for ( int pass=0; pass<2 && GUCEF_NULL == successor; ++pass )
        {
            for ( size_t i=0; i<state.members.size(); ++i )
            {
                const CLobbyMember& candidate = state.members[ i ];
                if ( 0 == pass && candidate.IsSpectator() )
                    continue;
                if ( GUCEF_NULL == successor || candidate.joinOrder < successor->joinOrder )
                    successor = &candidate;
            }
        }
        state.bossId.Clear();
        if ( GUCEF_NULL != successor )
        {
            CORE::CString successorId = successor->id;
            ApplyTransferBoss( state, successorId );
            outcome.newBossId = successorId;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "minPlayers", minPlayers );
    config.SetAttribute( "maxPlayers", maxPlayers );
    config.SetAttribute( "allowSpectators", allowSpectators );
    config.SetAttribute( "requireAllReady", requireAllReady );
    config.SetAttribute( "allowForceStart", allowForceStart );
    config.SetAttribute( "startByVote", startByVote );
    config.SetAttribute( "countdownInSecs", countdownInSecs );
    config.SetAttribute( "voteDurationInSecs", voteDurationInSecs );
    config.SetAttribute( "closeWhenHostLeaves", closeWhenHostLeaves );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyRules::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    minPlayers = config.GetAttributeValueOrChildValueByName( "minPlayers" ).AsUInt32( minPlayers, true );
    maxPlayers = config.GetAttributeValueOrChildValueByName( "maxPlayers" ).AsUInt32( maxPlayers, true );
    allowSpectators = config.GetAttributeValueOrChildValueByName( "allowSpectators" ).AsBool( allowSpectators, true );
    requireAllReady = config.GetAttributeValueOrChildValueByName( "requireAllReady" ).AsBool( requireAllReady, true );
    allowForceStart = config.GetAttributeValueOrChildValueByName( "allowForceStart" ).AsBool( allowForceStart, true );
    startByVote = config.GetAttributeValueOrChildValueByName( "startByVote" ).AsBool( startByVote, true );
    countdownInSecs = config.GetAttributeValueOrChildValueByName( "countdownInSecs" ).AsUInt32( countdownInSecs, true );
    voteDurationInSecs = config.GetAttributeValueOrChildValueByName( "voteDurationInSecs" ).AsUInt32( voteDurationInSecs, true );
    closeWhenHostLeaves = config.GetAttributeValueOrChildValueByName( "closeWhenHostLeaves" ).AsBool( closeWhenHostLeaves, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyRules::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbyRules";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
