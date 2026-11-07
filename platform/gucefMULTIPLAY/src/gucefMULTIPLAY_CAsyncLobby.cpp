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

#ifndef GUCEF_CORE_CINOTIFIER_H
#include "gucefCORE_CINotifier.h"
#define GUCEF_CORE_CINOTIFIER_H
#endif /* GUCEF_CORE_CINOTIFIER_H ? */

#include "gucefMULTIPLAY_CAsyncLobby.h"

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

namespace {

/** The plain words for what the kind of lobby has no meaning for */
const char* const NotOffered = "This kind of game does not have that.";

bool
SameMember( const CLobbyMember& a, const CLobbyMember& b )
{GUCEF_TRACE;

    return a.id == b.id && a.name == b.name && a.role == b.role && a.ready == b.ready && a.slot == b.slot && a.statusText == b.statusText;
}

bool
SameSlot( const CLobbySlot& a, const CLobbySlot& b )
{GUCEF_TRACE;

    return a.index == b.index && a.team == b.team && a.faction == b.faction && a.colour == b.colour && a.kind == b.kind && a.aiType == b.aiType && a.memberId == b.memberId;
}

bool
SamePresentation( const CLobbyPresentation& a, const CLobbyPresentation& b )
{GUCEF_TRACE;

    return a.showLatency == b.showLatency && a.showFactionAndColour == b.showFactionAndColour && a.showReady == b.showReady && a.showStart == b.showStart && a.showForceStart == b.showForceStart &&
           a.showVoteStart == b.showVoteStart && a.showSpectate == b.showSpectate && a.showInvite == b.showInvite && a.settingsEditable == b.settingsEditable && a.showBack == b.showBack && a.backText == b.backText && a.showCancel == b.showCancel && a.canCancel == b.canCancel && a.cancelText == b.cancelText &&
           a.startHint == b.startHint && a.forceStartHint == b.forceStartHint && a.leaveHint == b.leaveHint && a.cancelHint == b.cancelHint &&
           a.readyText == b.readyText && a.readyColumnTitle == b.readyColumnTitle && a.spectateText == b.spectateText && a.leaveText == b.leaveText && a.startText == b.startText &&
           a.forceStartText == b.forceStartText && a.voteStartText == b.voteStartText && a.kickText == b.kickText && a.note == b.note && a.voteLines == b.voteLines && a.voteWarning == b.voteWarning &&
           a.canReady == b.canReady && a.canSpectate == b.canSpectate && a.canStart == b.canStart && a.canForceStart == b.canForceStart && a.canVoteStart == b.canVoteStart &&
           a.canLeave == b.canLeave && a.canInvite == b.canInvite && a.canKick == b.canKick && a.canBan == b.canBan && a.canTransferBoss == b.canTransferBoss &&
           a.canManageSlots == b.canManageSlots && a.canChooseSlot == b.canChooseSlot && a.canChooseFaction == b.canChooseFaction && a.canChooseColour == b.canChooseColour;
}

bool
SameSettings( const CORE::CValueList& a, const CORE::CValueList& b )
{GUCEF_TRACE;

    if ( a.GetKeyCount() != b.GetKeyCount() )
        return false;
    for ( UInt32 i=0; i<a.GetKeyCount(); ++i )
    {
        const CORE::CString key = a.GetKey( i ).AsString();
        if ( !b.HasKey( key ) || a.GetValueAlways( key ).AsString() != b.GetValueAlways( key ).AsString() )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

CAsyncLobby::CAsyncLobby( CIAsyncLobbyBackend& backend, const CORE::CString& lobbyId )
    : CILobby()
    , m_backend( &backend )
    , m_lobbyId( lobbyId )
    , m_view()
    , m_hasView( false )
    , m_closedNotified( false )
{GUCEF_TRACE;

    m_hasView = m_backend->GetLobby( m_lobbyId, m_view );
    m_closedNotified = m_hasView && m_view.closed;
    TEventCallback callback( this, &CAsyncLobby::OnBackendLobbyChanged );
    SubscribeTo( m_backend, CIAsyncLobbyBackend::LobbyChangedEvent, callback );
}

/*-------------------------------------------------------------------------*/

CAsyncLobby::~CAsyncLobby()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobby::GetLobbyId( void ) const
{GUCEF_TRACE;

    return m_lobbyId;
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobby::CollectChanges( const CAsyncLobbyView& before, const CAsyncLobbyView& after, CORE::CString::StringVector& paths ) const
{GUCEF_TRACE;

    const CLobbyState& a = before.state;
    const CLobbyState& b = after.state;
    if ( a.phase != b.phase || a.name != b.name )
        paths.push_back( "phase" );
    if ( a.bossId != b.bossId || before.localMemberId != after.localMemberId )
        paths.push_back( "boss" );
    bool membersChanged = a.members.size() != b.members.size();
    for ( size_t i=0; !membersChanged && i<a.members.size(); ++i )
        membersChanged = !SameMember( a.members[ i ], b.members[ i ] );
    if ( membersChanged )
        paths.push_back( "members" );
    bool slotsChanged = a.slots.size() != b.slots.size();
    for ( size_t i=0; !slotsChanged && i<a.slots.size(); ++i )
        slotsChanged = !SameSlot( a.slots[ i ], b.slots[ i ] );
    if ( slotsChanged )
        paths.push_back( "slots" );
    if ( !SameSettings( a.settings, b.settings ) )
        paths.push_back( "settings" );
    if ( a.activeVote.id != b.activeVote.id || a.activeVote.yes != b.activeVote.yes || a.activeVote.no != b.activeVote.no || a.activeVote.needed != b.activeVote.needed )
        paths.push_back( "vote" );
    if ( !SamePresentation( before.presentation, after.presentation ) )
        paths.push_back( "presentation" );
    if ( before.closed != after.closed )
        paths.push_back( "closed" );
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobby::Update( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_backend )
        return;
    CAsyncLobbyView view;
    if ( !m_backend->GetLobby( m_lobbyId, view ) )
    {
        // the game is not in the list of the player any more
        if ( !m_hasView || m_closedNotified )
            return;
        view = m_view;
        view.closed = true;
        view.closedReason = "This game is not in your list any more.";
    }

    CORE::CString::StringVector paths;
    if ( m_hasView )
        CollectChanges( m_view, view, paths );
    else
        paths.push_back( "state" );
    const CLobbyVote before = m_view.state.activeVote;
    const bool hadView = m_hasView;
    const CLobbyState previous = m_view.state;
    m_view = view;
    m_hasView = true;

    // the members that came and went
    if ( hadView )
    {
        for ( size_t i=0; i<m_view.state.members.size(); ++i )
            if ( GUCEF_NULL == previous.FindMember( m_view.state.members[ i ].id ) )
            {
                TMemberEventData member( m_view.state.members[ i ].id );
                if ( !NotifyObservers( MemberJoinedEvent, &member ) )
                    return;
            }
        for ( size_t i=0; i<previous.members.size(); ++i )
            if ( GUCEF_NULL == m_view.state.FindMember( previous.members[ i ].id ) )
            {
                TMemberEventData member( previous.members[ i ].id );
                if ( !NotifyObservers( MemberLeftEvent, &member ) )
                    return;
            }
    }

    if ( !paths.empty() )
    {
        TLobbyStateChangedEventData data( paths );
        if ( !NotifyObservers( LobbyStateChangedEvent, &data ) )
            return;
    }

    // a vote that began and one that ended
    if ( hadView && before.IsActive() && ( !m_view.state.activeVote.IsActive() || m_view.state.activeVote.id != before.id ) )
    {
        CLobbyVoteEventData ended( before, m_view.lastVotePassed );
        if ( !NotifyObservers( VoteEndedEvent, &ended ) )
            return;
    }
    if ( m_view.state.activeVote.IsActive() && ( !hadView || !before.IsActive() || m_view.state.activeVote.id != before.id ) )
    {
        CLobbyVoteEventData started( m_view.state.activeVote, false );
        if ( !NotifyObservers( VoteStartedEvent, &started ) )
            return;
    }

    if ( m_view.closed && !m_closedNotified )
    {
        m_closedNotified = true;
        TLobbyClosedEventData reason( m_view.closedReason );
        NotifyObservers( LobbyClosedEvent, &reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobby::OnBackendLobbyChanged( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CIAsyncLobbyBackend::TLobbyEventData* changed = GUCEF_NULL != eventData ? static_cast< const CIAsyncLobbyBackend::TLobbyEventData* >( eventData ) : GUCEF_NULL;
    if ( GUCEF_NULL != changed && changed->GetData() != m_lobbyId )
        return;
    Update();
}

/*-------------------------------------------------------------------------*/

void
CAsyncLobby::OnNotify( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the standard events do not reach the handlers of the subscriptions
    if ( GUCEF_NULL != m_backend && notifier == m_backend && CORE::CINotifier::DestructionEvent == eventId )
        m_backend = GUCEF_NULL;
    CILobby::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CLobbyState&
CAsyncLobby::GetState( void ) const
{GUCEF_TRACE;

    return m_view.state;
}

/*-------------------------------------------------------------------------*/

const CLobbySettingsSchema&
CAsyncLobby::GetSchema( void ) const
{GUCEF_TRACE;

    return m_view.schema;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAsyncLobby::GetLocalMemberId( void ) const
{GUCEF_TRACE;

    return m_view.localMemberId;
}

/*-------------------------------------------------------------------------*/

CLobbyPresentation
CAsyncLobby::GetPresentation( void ) const
{GUCEF_TRACE;

    return m_view.presentation;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Refuse( const char* what, const CORE::CString& reason )
{GUCEF_TRACE;

    CLobbyRefusalEventData refusal( what, reason.IsNULLOrEmpty() ? CORE::CString( "That did not work." ) : reason );
    NotifyObservers( RequestRefusedEvent, &refusal );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Finish( const char* what, bool done, const CORE::CString& error )
{GUCEF_TRACE;

    if ( !done )
        return Refuse( what, error );
    Update();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::RefuseUnsupported( const char* what )
{GUCEF_TRACE;

    return Refuse( what, NotOffered );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetSetting( const CORE::CString& /* key */, const CORE::CString& /* value */ )
{GUCEF_TRACE;

    return Refuse( "setting", "The settings of the game are fixed when the game is made." );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::ProposeSetting( const CORE::CString& /* key */, const CORE::CString& /* value */ )
{GUCEF_TRACE;

    return Refuse( "proposal", "The settings of the game are fixed when the game is made." );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Vote( const CORE::CString& voteId, bool yes )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "vote", GUCEF_NULL != m_backend && m_backend->Vote( m_lobbyId, voteId, yes, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetReady( bool ready )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "ready", GUCEF_NULL != m_backend && m_backend->Answer( m_lobbyId, ready, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetSpectator( bool spectate )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "spectator", GUCEF_NULL != m_backend && m_backend->SetSpectating( m_lobbyId, spectate, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::RequestSlot( UInt32 /* slotIndex */ )
{GUCEF_TRACE;

    return RefuseUnsupported( "slot" );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetFaction( const CORE::CString& /* faction */ )
{GUCEF_TRACE;

    return RefuseUnsupported( "faction" );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetColour( const CORE::CString& /* colour */ )
{GUCEF_TRACE;

    return RefuseUnsupported( "colour" );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Kick( const CORE::CString& memberId )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "kick", GUCEF_NULL != m_backend && m_backend->Kick( m_lobbyId, memberId, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::TransferBoss( const CORE::CString& /* memberId */ )
{GUCEF_TRACE;

    return RefuseUnsupported( "transferBoss" );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::SetSlotKind( UInt32 /* slotIndex */, TLobbySlotKind /* kind */, const CORE::CString& /* aiType */ )
{GUCEF_TRACE;

    return RefuseUnsupported( "slotKind" );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::RequestStart( void )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "start", GUCEF_NULL != m_backend && m_backend->Start( m_lobbyId, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::ForceStart( void )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "forceStart", GUCEF_NULL != m_backend && m_backend->ForceStart( m_lobbyId, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Leave( void )
{GUCEF_TRACE;

    CORE::CString error;
    const bool left = GUCEF_NULL != m_backend && m_backend->Leave( m_lobbyId, error );
    if ( left )
        m_closedNotified = true;        // the player left: the lobby is over for them, which they know
    return Finish( "leave", left, error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Invite( const CORE::CString& name, const CORE::CString& contact, const CORE::CString& code )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "invite", GUCEF_NULL != m_backend && m_backend->Invite( m_lobbyId, name, contact, code, error ), error );
}

/*-------------------------------------------------------------------------*/

bool
CAsyncLobby::Cancel( void )
{GUCEF_TRACE;

    CORE::CString error;
    return Finish( "cancel", GUCEF_NULL != m_backend && m_backend->Cancel( m_lobbyId, error ), error );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
