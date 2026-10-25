/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "TestLobbyService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace {

/** The lobbies of the in-memory "server", shared by every CTestLobbyService of the process */
struct SRegistry
{
    typedef GUCEF::map< CORE::CString, CTestLobbyHost* > THostMap;
    typedef GUCEF::map< CORE::CString, MULTIPLAY::CGameListingEntry > TListingMap;

    THostMap hosts;
    TListingMap published;
    UInt32 lobbyCount;

    SRegistry( void ) : hosts(), published(), lobbyCount( 0 ) {}
};

SRegistry&
GetRegistry( void )
{
    static SRegistry registry;
    return registry;
}

/** A future that is done already */
CORE::CFutureResult
MakeCompletedFuture( CORE::CICloneable* resultData )
{
    CORE::CFutureResultSource source;
    source.Complete( resultData, true );
    return source.GetFuture();
}

CORE::CFutureResult
MakeFailedFuture( const CORE::CString& reason )
{
    CORE::CFutureResultSource source;
    source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
    return source.GetFuture();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestLobbyHost::CTestLobbyHost( const CORE::CString& lobbyId                 ,
                                const MULTIPLAY::CLobbyCreateParams& params  ,
                                const MULTIPLAY::CLobbyRules& rules          ,
                                UInt32 secondInMs                            )
    : CORE::CObserver()
    , m_state()
    , m_params( params )
    , m_rules( rules )
    , m_hostMemberId()
    , m_lobbies()
    , m_endpoints()
    , m_timer( secondInMs )
    , m_countdownLeft( -1 )
    , m_voteSecondsLeft( -1 )
    , m_voteCount( 0 )
{GUCEF_TRACE;

    m_state.lobbyId = lobbyId;
    m_state.name = params.name;
    m_state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    m_state.slots = params.slots;
    if ( m_state.slots.empty() )
    {
        for ( UInt32 i=0; i<params.maxPlayers; ++i )
        {
            MULTIPLAY::CLobbySlot slot;
            slot.index = i;
            slot.team = i;
            m_state.slots.push_back( slot );
        }
    }
    params.GetStartSettings( m_state.settings );

    if ( 0 != params.maxPlayers )
        m_rules.maxPlayers = params.maxPlayers;
    m_rules.allowSpectators = m_rules.allowSpectators && params.allowSpectators;

    TEventCallback callback( this, &CTestLobbyHost::OnTimerUpdate );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, callback );
}

/*-------------------------------------------------------------------------*/

CTestLobbyHost::~CTestLobbyHost()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    TLobbyVector lobbies = m_lobbies;
    m_lobbies.clear();
    for ( size_t i=0; i<lobbies.size(); ++i )
        lobbies[ i ]->OnDetached();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyState&
CTestLobbyHost::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbySettingsSchema&
CTestLobbyHost::GetSchema( void ) const
{GUCEF_TRACE;

    return m_params.schema;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyCreateParams&
CTestLobbyHost::GetParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyRules&
CTestLobbyHost::GetRules( void ) const
{GUCEF_TRACE;

    return m_rules;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyHost::IsPasswordProtected( void ) const
{GUCEF_TRACE;

    return !m_params.password.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetChat( const CORE::CString& backend, const CORE::CString& channel )
{GUCEF_TRACE;

    m_state.chatBackend = backend;
    m_state.chatChannel = channel;
    StateChanged( "chat" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetMemberLatency( const CORE::CString& memberId, Int32 latencyMs )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyMember* member = m_state.FindMember( memberId );
    if ( GUCEF_NULL == member )
        return;
    member->latencyMs = latencyMs;
    StateChanged( "members" );
}

/*-------------------------------------------------------------------------*/

CTestLobby*
CTestLobbyHost::FindLobby( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_lobbies.size(); ++i )
        if ( m_lobbies[ i ]->GetLocalMemberId() == memberId )
            return m_lobbies[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyHost::IsAttached( CTestLobby* lobby ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_lobbies.size(); ++i )
        if ( m_lobbies[ i ] == lobby )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Refuse( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& reason )
{GUCEF_TRACE;

    CTestLobby* lobby = FindLobby( memberId );
    if ( GUCEF_NULL == lobby )
        return;
    MULTIPLAY::CLobbyRefusalEventData refusal( what, reason );
    lobby->Deliver( MULTIPLAY::CILobby::RequestRefusedEvent, &refusal );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Broadcast( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // Observers may leave while they are told
    TLobbyVector lobbies = m_lobbies;
    for ( size_t i=0; i<lobbies.size(); ++i )
        if ( IsAttached( lobbies[ i ] ) )
            lobbies[ i ]->Deliver( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::StateChanged( const CORE::CString& path )
{GUCEF_TRACE;

    MULTIPLAY::CILobby::TLobbyStateChangedEventData paths;
    paths.GetData().push_back( path );
    Broadcast( MULTIPLAY::CILobby::LobbyStateChangedEvent, &paths );
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyHost::Join( CTestLobby* lobby                          ,
                      const CORE::CString& memberId              ,
                      const CORE::CString& name                  ,
                      const CORE::CString::StringVector& endpoints ,
                      const CORE::CString& password              ,
                      bool asSpectator                           ,
                      CORE::CString& reason                      )
{GUCEF_TRACE;

    if ( !m_params.password.IsNULLOrEmpty() && password != m_params.password )
    {
        reason = "The password is wrong";
        return false;
    }
    if ( GUCEF_NULL != m_state.FindMember( memberId ) )
    {
        reason = "Already a member";
        return false;
    }
    if ( !m_rules.CheckJoin( m_state, asSpectator, reason ) )
        return false;

    m_rules.ApplyJoin( m_state, memberId, name, asSpectator );
    if ( m_hostMemberId.IsNULLOrEmpty() )
        m_hostMemberId = memberId;
    m_endpoints[ memberId ] = endpoints;
    m_lobbies.push_back( lobby );

    CORE::TCloneableString joined( memberId );
    Broadcast( MULTIPLAY::CILobby::MemberJoinedEvent, &joined );
    StateChanged( "members" );

    // A spectator of a game in progress gets on at once
    if ( MULTIPLAY::LOBBYPHASE_IN_GAME == m_state.phase && IsAttached( lobby ) )
    {
        MULTIPLAY::CLaunchDescription launch = MakeLaunchDescription( memberId );
        lobby->Deliver( MULTIPLAY::CILobby::LaunchEvent, &launch );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Detach( CTestLobby* lobby )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_lobbies.size(); ++i )
    {
        if ( m_lobbies[ i ] == lobby )
        {
            m_lobbies.erase( m_lobbies.begin() + i );
            lobby->OnDetached();
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::ApplyLeaveOutcome( const CORE::CString& memberId                      ,
                                   const MULTIPLAY::CLobbyVote& voteBefore            ,
                                   const MULTIPLAY::CLobbyRules::TLeaveOutcome& outcome ,
                                   const CORE::CString& closeReason                   )
{GUCEF_TRACE;

    CORE::TCloneableString left( memberId );
    Broadcast( MULTIPLAY::CILobby::MemberLeftEvent, &left );

    if ( outcome.voteCancelled )
    {
        m_voteSecondsLeft = -1;
        MULTIPLAY::CLobbyVoteEventData ended( voteBefore, false );
        Broadcast( MULTIPLAY::CILobby::VoteEndedEvent, &ended );
        StateChanged( "vote" );
    }
    if ( outcome.countdownCancelled )
    {
        m_countdownLeft = -1;
        CORE::TCloneableInt32 cancelled( -1 );
        Broadcast( MULTIPLAY::CILobby::CountdownEvent, &cancelled );
        StateChanged( "phase" );
    }
    if ( !outcome.newBossId.IsNULLOrEmpty() )
        StateChanged( "boss" );
    StateChanged( "members" );

    if ( outcome.lobbyClosed )
    {
        m_timer.SetEnabled( false );
        CORE::TCloneableString reason( closeReason );
        Broadcast( MULTIPLAY::CILobby::LobbyClosedEvent, &reason );
        TLobbyVector lobbies = m_lobbies;
        for ( size_t i=0; i<lobbies.size(); ++i )
            Detach( lobbies[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Leave( const CORE::CString& memberId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_state.FindMember( memberId ) )
        return;

    // The host of a LAN lobby runs it, a public lobby lives on the server
    bool isHost = memberId == m_hostMemberId && !m_params.isPublic;
    MULTIPLAY::CLobbyVote voteBefore = m_state.activeVote;
    MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
    m_rules.ApplyLeave( m_state, memberId, isHost, outcome );

    CTestLobby* lobby = FindLobby( memberId );
    if ( GUCEF_NULL != lobby )
        Detach( lobby );

    ApplyLeaveOutcome( memberId, voteBefore, outcome, isHost ? "The host left" : "The lobby is empty" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Kick( const CORE::CString& memberId, const CORE::CString& targetId )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckKick( m_state, memberId, targetId, reason ) )
    {
        Refuse( memberId, "kick", reason );
        return;
    }

    MULTIPLAY::CLobbyVote voteBefore = m_state.activeVote;
    MULTIPLAY::CLobbyRules::TLeaveOutcome outcome;
    m_rules.ApplyLeave( m_state, targetId, false, outcome );

    CTestLobby* lobby = FindLobby( targetId );
    if ( GUCEF_NULL != lobby )
    {
        CORE::TCloneableString kicked( "You were kicked" );
        lobby->Deliver( MULTIPLAY::CILobby::LobbyClosedEvent, &kicked );
        Detach( lobby );
    }
    ApplyLeaveOutcome( targetId, voteBefore, outcome, "The lobby is empty" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetSetting( const CORE::CString& memberId, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    bool needsVote = false;
    CORE::CString reason;
    if ( !m_rules.CheckSetting( m_state, m_params.schema, memberId, key, value, needsVote, reason ) )
    {
        Refuse( memberId, "setting", reason );
        return;
    }
    if ( needsVote )
    {
        StartVote( memberId, "setting", key, value );
        return;
    }
    m_state.SetSetting( key, value );
    StateChanged( "settings/" + key );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::ProposeSetting( const CORE::CString& memberId, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckProposal( m_state, m_params.schema, memberId, key, value, reason ) )
    {
        Refuse( memberId, "proposal", reason );
        return;
    }
    StartVote( memberId, "setting", key, value );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::StartVote( const CORE::CString& memberId, const CORE::CString& what, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyVote& vote = m_state.activeVote;
    vote.Clear();
    vote.id = "vote-" + CORE::ToString( ++m_voteCount );
    vote.what = what;
    vote.proposer = memberId;
    vote.key = key;
    vote.value = value;
    vote.yes = 1;
    vote.no = 0;
    vote.needed = m_rules.GetVotesNeeded( m_state );
    vote.voters.insert( memberId );
    vote.deadline = CORE::CDateTime::NowUTCDateTime();
    vote.deadline.AddSeconds( (Int32) m_rules.voteDurationInSecs );
    m_voteSecondsLeft = (Int32) m_rules.voteDurationInSecs;
    m_timer.SetEnabled( true );

    MULTIPLAY::CLobbyVoteEventData started( vote, false );
    Broadcast( MULTIPLAY::CILobby::VoteStartedEvent, &started );
    StateChanged( "vote" );
    EvaluateVote();
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Vote( const CORE::CString& memberId, const CORE::CString& voteId, bool yes )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckVote( m_state, memberId, voteId, reason ) )
    {
        Refuse( memberId, "vote", reason );
        return;
    }
    MULTIPLAY::CLobbyVote& vote = m_state.activeVote;
    if ( yes )
        ++vote.yes;
    else
        ++vote.no;
    vote.voters.insert( memberId );
    StateChanged( "vote" );
    EvaluateVote();
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::EvaluateVote( void )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyRules::TVoteOutcome outcome = m_rules.EvaluateVote( m_state );
    if ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_PASSED == outcome )
        EndVote( true );
    else
    if ( MULTIPLAY::CLobbyRules::VOTEOUTCOME_FAILED == outcome )
        EndVote( false );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::EndVote( bool passed )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyVote vote = m_state.activeVote;
    m_state.activeVote.Clear();
    m_voteSecondsLeft = -1;

    MULTIPLAY::CLobbyVoteEventData ended( vote, passed );
    Broadcast( MULTIPLAY::CILobby::VoteEndedEvent, &ended );
    StateChanged( "vote" );
    if ( !passed )
        return;

    if ( "setting" == vote.what )
    {
        m_state.SetSetting( vote.key, vote.value );
        StateChanged( "settings/" + vote.key );
    }
    else
    if ( "start" == vote.what )
    {
        CORE::CString reason;
        if ( m_rules.IsStartAllowed( m_state, reason ) )
            StartCountdown();
        else
            Refuse( vote.proposer, "start", reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetReady( const CORE::CString& memberId, bool ready )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckReady( m_state, memberId, reason ) )
    {
        Refuse( memberId, "ready", reason );
        return;
    }
    bool countdownCancelled = false;
    m_rules.ApplyReady( m_state, memberId, ready, countdownCancelled );
    StateChanged( "members" );
    if ( countdownCancelled )
        CancelCountdown();
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetSpectator( const CORE::CString& memberId, bool spectate )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckSpectator( m_state, memberId, spectate, reason ) )
    {
        Refuse( memberId, "spectator", reason );
        return;
    }
    m_rules.ApplySpectator( m_state, memberId, spectate );
    StateChanged( "members" );
    StateChanged( "slots" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::RequestSlot( const CORE::CString& memberId, UInt32 slotIndex )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckSlotRequest( m_state, memberId, slotIndex, reason ) )
    {
        Refuse( memberId, "slot", reason );
        return;
    }
    m_rules.ApplySlot( m_state, memberId, slotIndex );
    StateChanged( "slots" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetFaction( const CORE::CString& memberId, const CORE::CString& faction )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckFaction( m_state, m_params.schema, memberId, faction, reason ) )
    {
        Refuse( memberId, "faction", reason );
        return;
    }
    m_state.FindSlotOfMember( memberId )->faction = faction;
    StateChanged( "slots" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetColour( const CORE::CString& memberId, const CORE::CString& colour )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckColour( m_state, m_params.schema, memberId, colour, reason ) )
    {
        Refuse( memberId, "colour", reason );
        return;
    }
    m_state.FindSlotOfMember( memberId )->colour = colour;
    StateChanged( "slots" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::TransferBoss( const CORE::CString& memberId, const CORE::CString& targetId )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckTransferBoss( m_state, memberId, targetId, reason ) )
    {
        Refuse( memberId, "transferBoss", reason );
        return;
    }
    m_rules.ApplyTransferBoss( m_state, targetId );
    StateChanged( "boss" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::SetSlotKind( const CORE::CString& memberId, UInt32 slotIndex, MULTIPLAY::TLobbySlotKind kind, const CORE::CString& aiType )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !m_rules.CheckSlotKind( m_state, memberId, slotIndex, kind, reason ) )
    {
        Refuse( memberId, "slotKind", reason );
        return;
    }
    MULTIPLAY::CLobbySlot* slot = m_state.FindSlot( slotIndex );
    slot->kind = kind;
    slot->aiType = MULTIPLAY::LOBBYSLOTKIND_AI == kind ? aiType : CORE::CString::Empty;
    StateChanged( "slots" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::RequestStart( const CORE::CString& memberId, bool force )
{GUCEF_TRACE;

    bool needsVote = false;
    CORE::CString reason;
    if ( !m_rules.CheckStart( m_state, memberId, force, needsVote, reason ) )
    {
        Refuse( memberId, force ? "forceStart" : "start", reason );
        return;
    }
    if ( needsVote )
        StartVote( memberId, "start", CORE::CString::Empty, CORE::CString::Empty );
    else
        StartCountdown();
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::StartCountdown( void )
{GUCEF_TRACE;

    m_state.phase = MULTIPLAY::LOBBYPHASE_COUNTDOWN;
    m_countdownLeft = (Int32) m_rules.countdownInSecs;
    StateChanged( "phase" );

    CORE::TCloneableInt32 left( m_countdownLeft );
    Broadcast( MULTIPLAY::CILobby::CountdownEvent, &left );
    if ( 0 >= m_countdownLeft )
        Launch();
    else
        m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::CancelCountdown( void )
{GUCEF_TRACE;

    m_countdownLeft = -1;
    CORE::TCloneableInt32 cancelled( -1 );
    Broadcast( MULTIPLAY::CILobby::CountdownEvent, &cancelled );
    StateChanged( "phase" );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::Launch( void )
{GUCEF_TRACE;

    m_countdownLeft = -1;
    m_state.phase = MULTIPLAY::LOBBYPHASE_LAUNCHING;
    StateChanged( "phase" );

    TLobbyVector lobbies = m_lobbies;
    for ( size_t i=0; i<lobbies.size(); ++i )
    {
        if ( !IsAttached( lobbies[ i ] ) )
            continue;
        MULTIPLAY::CLaunchDescription launch = MakeLaunchDescription( lobbies[ i ]->GetLocalMemberId() );
        lobbies[ i ]->Deliver( MULTIPLAY::CILobby::LaunchEvent, &launch );
    }

    m_state.phase = MULTIPLAY::LOBBYPHASE_IN_GAME;
    StateChanged( "phase" );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CLaunchDescription
CTestLobbyHost::MakeLaunchDescription( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    MULTIPLAY::CLaunchDescription launch;
    launch.gameId = m_params.gameId;
    launch.gameVersion = m_params.gameVersion;
    launch.lobbyId = m_state.lobbyId;
    launch.sessionName = m_state.lobbyId + "-game";
    launch.topology = m_params.topology;

    // The host serves, or orders the peers; when it left the boss does
    CORE::CString hostId = GUCEF_NULL != m_state.FindMember( m_hostMemberId ) ? m_hostMemberId : m_state.bossId;
    if ( MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == m_params.topology )
        launch.sequencerMemberId = hostId;
    else
        launch.serverMemberId = hostId;

    for ( size_t i=0; i<m_state.members.size(); ++i )
    {
        const MULTIPLAY::CLobbyMember& member = m_state.members[ i ];
        MULTIPLAY::CLaunchMember launchMember;
        launchMember.id = member.id;
        launchMember.name = member.name;
        launchMember.role = member.role;
        launchMember.slot = member.slot;
        TEndpointMap::const_iterator e = m_endpoints.find( member.id );
        if ( e != m_endpoints.end() )
            launchMember.endpoints = (*e).second;
        launch.members.push_back( launchMember );
    }
    launch.settings = m_state.settings;
    launch.slots = m_state.slots;
    launch.localMemberId = memberId;
    launch.token = "token:" + m_state.lobbyId + ":" + memberId;
    const MULTIPLAY::CLobbyMember* member = m_state.FindMember( memberId );
    launch.spectator = GUCEF_NULL != member && member->IsSpectator();
    return launch;
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CGameListingEntry
CTestLobbyHost::MakeListing( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CGameListingEntry listing;
    listing.listingId = m_state.lobbyId;
    listing.kind = MULTIPLAY::LOBBYPHASE_IN_GAME == m_state.phase ? MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS : MULTIPLAY::CGameListingEntry::KIND_LOBBY;
    listing.name = m_state.name;
    listing.gameId = m_params.gameId;
    listing.gameVersion = m_params.gameVersion;
    listing.players = m_state.GetPlayerCount();
    listing.maxPlayers = m_rules.maxPlayers;
    listing.spectators = m_state.GetSpectatorCount();
    listing.passworded = IsPasswordProtected();
    const MULTIPLAY::CLobbyMember* host = m_state.FindMember( m_hostMemberId );
    if ( GUCEF_NULL != host )
        listing.hostName = host->name;
    TEndpointMap::const_iterator e = m_endpoints.find( m_hostMemberId );
    if ( e != m_endpoints.end() )
        listing.endpoints = (*e).second;
    listing.backendName = CTestLobbyService::ServiceName;
    listing.lastSeen = CORE::CDateTime::NowUTCDateTime();
    return listing;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::OnSecond( void )
{GUCEF_TRACE;

    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN == m_state.phase && m_countdownLeft > 0 )
    {
        --m_countdownLeft;
        CORE::TCloneableInt32 left( m_countdownLeft );
        Broadcast( MULTIPLAY::CILobby::CountdownEvent, &left );
        if ( 0 == m_countdownLeft )
            Launch();
    }
    if ( m_state.activeVote.IsActive() && m_voteSecondsLeft > 0 )
    {
        --m_voteSecondsLeft;
        if ( 0 == m_voteSecondsLeft )
            EndVote( false );
    }
    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN != m_state.phase && !m_state.activeVote.IsActive() )
        m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyHost::OnTimerUpdate( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnSecond();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestLobbyHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CTestLobbyHost";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CTestLobby::CTestLobby( CTestLobbyHost* host, const CORE::CString& localMemberId )
    : MULTIPLAY::CILobby()
    , m_host( host )
    , m_localMemberId( localMemberId )
    , m_attached( true )
    , m_lastState()
    , m_lastSchema()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestLobby::~CTestLobby()
{GUCEF_TRACE;

    if ( m_attached && GUCEF_NULL != m_host )
        m_host->Leave( m_localMemberId );
    if ( m_attached && GUCEF_NULL != m_host )
        m_host->Detach( this );
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyState&
CTestLobby::GetState( void ) const
{GUCEF_TRACE;

    return m_attached && GUCEF_NULL != m_host ? m_host->GetState() : m_lastState;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbySettingsSchema&
CTestLobby::GetSchema( void ) const
{GUCEF_TRACE;

    return m_attached && GUCEF_NULL != m_host ? m_host->GetSchema() : m_lastSchema;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestLobby::GetLocalMemberId( void ) const
{GUCEF_TRACE;

    return m_localMemberId;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::IsMember( void ) const
{GUCEF_TRACE;

    return m_attached && GUCEF_NULL != m_host;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetSetting( m_localMemberId, key, value );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::ProposeSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->ProposeSetting( m_localMemberId, key, value );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::Vote( const CORE::CString& voteId, bool yes )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->Vote( m_localMemberId, voteId, yes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetReady( bool ready )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetReady( m_localMemberId, ready );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetSpectator( bool spectate )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetSpectator( m_localMemberId, spectate );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::RequestSlot( UInt32 slotIndex )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->RequestSlot( m_localMemberId, slotIndex );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetFaction( const CORE::CString& faction )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetFaction( m_localMemberId, faction );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetColour( const CORE::CString& colour )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetColour( m_localMemberId, colour );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::Kick( const CORE::CString& memberId )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->Kick( m_localMemberId, memberId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::Ban( const CORE::CString& memberId )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    lastBannedId = memberId;
    m_host->Kick( m_localMemberId, memberId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    lastReportedId = memberId;
    lastReportReason = reason;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::TransferBoss( const CORE::CString& memberId )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->TransferBoss( m_localMemberId, memberId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::SetSlotKind( UInt32 slotIndex, MULTIPLAY::TLobbySlotKind kind, const CORE::CString& aiType )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->SetSlotKind( m_localMemberId, slotIndex, kind, aiType );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::RequestStart( void )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->RequestStart( m_localMemberId, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::ForceStart( void )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->RequestStart( m_localMemberId, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::Leave( void )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    m_host->Leave( m_localMemberId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobby::Deliver( const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    return NotifyObservers( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CTestLobby::OnDetached( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_host )
    {
        m_lastState = m_host->GetState();
        m_lastSchema = m_host->GetSchema();
    }
    m_attached = false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestLobby::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CTestLobby";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString CTestLobbyService::ServiceName = "test";

/*-------------------------------------------------------------------------*/

CTestLobbyService::CTestLobbyService( void )
    : MULTIPLAY::CILobbyService()
    , m_identity()
    , m_connected( false )
    , m_localEndpoints()
    , m_rules()
    , m_secondInMs( 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestLobbyService::~CTestLobbyService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::Connect( const MULTIPLAY::CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( identity.userId.IsNULLOrEmpty() )
    {
        future = MakeFailedFuture( "An identity needs a user id" );
        return false;
    }
    m_identity = identity;
    m_connected = true;
    future = MakeCompletedFuture( identity.Clone() );
    NotifyObservers( ServiceConnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyService::Disconnect( void )
{GUCEF_TRACE;

    if ( !m_connected )
        return;
    m_connected = false;
    NotifyObservers( ServiceDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::Browse( const MULTIPLAY::CGameListingFilter& filter, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;

    MULTIPLAY::CGameListingList list;
    SRegistry& registry = GetRegistry();
    SRegistry::THostMap::iterator h = registry.hosts.begin();
    while ( h != registry.hosts.end() )
    {
        if ( MULTIPLAY::LOBBYPHASE_CLOSED != (*h).second->GetState().phase )
        {
            MULTIPLAY::CGameListingEntry listing = (*h).second->MakeListing();
            if ( filter.Matches( listing ) )
                list.listings.push_back( listing );
        }
        ++h;
    }
    SRegistry::TListingMap::iterator l = registry.published.begin();
    while ( l != registry.published.end() )
    {
        if ( filter.Matches( (*l).second ) )
            list.listings.push_back( (*l).second );
        ++l;
    }
    future = MakeCompletedFuture( list.Clone() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::EnterLobby( CTestLobbyHost* host, const CORE::CString& password, bool asSpectator, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CTestLobby* lobby = GUCEF_NEW CTestLobby( host, m_identity.userId );
    CORE::CString reason;
    if ( !host->Join( lobby, m_identity.userId, m_identity.displayName, m_localEndpoints, password, asSpectator, reason ) )
    {
        lobby->OnDetached();
        GUCEF_DELETE lobby;
        future = MakeFailedFuture( reason );
        return true;
    }

    MULTIPLAY::CILobbyPtr lobbyPtr( lobby );
    MULTIPLAY::CLobbyResultData result( lobbyPtr );
    future = MakeCompletedFuture( result.Clone() );
    NotifyObservers( LobbyJoinedEvent, &result );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::CreateLobby( const MULTIPLAY::CLobbyCreateParams& params, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;

    SRegistry& registry = GetRegistry();
    CORE::CString lobbyId = "lobby-" + CORE::ToString( ++registry.lobbyCount );
    CTestLobbyHost* host = GUCEF_NEW CTestLobbyHost( lobbyId, params, m_rules, m_secondInMs );
    registry.hosts[ lobbyId ] = host;
    return EnterLobby( host, params.password, false, future );
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::JoinLobby( const CORE::CString& lobbyId    ,
                              const CORE::CString& password   ,
                              bool asSpectator                ,
                              CORE::CFutureResult& future     )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;

    CTestLobbyHost* host = FindHost( lobbyId );
    if ( GUCEF_NULL == host )
    {
        future = MakeFailedFuture( "There is no lobby " + lobbyId );
        return true;
    }
    return EnterLobby( host, password, asSpectator, future );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CLobbyServiceCapabilities
CTestLobbyService::GetCapabilities( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CLobbyServiceCapabilities capabilities;
    capabilities.voting = true;
    capabilities.inProgressSpectating = true;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestLobbyService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::PublishListing( const MULTIPLAY::CGameListingEntry& listing )
{GUCEF_TRACE;

    if ( listing.listingId.IsNULLOrEmpty() )
        return false;
    GetRegistry().published[ listing.listingId ] = listing;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::WithdrawListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    return 0 < GetRegistry().published.erase( listingId );
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyService::SetLocalEndpoints( const CORE::CString::StringVector& endpoints )
{GUCEF_TRACE;

    m_localEndpoints = endpoints;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyService::SetRules( const MULTIPLAY::CLobbyRules& rules )
{GUCEF_TRACE;

    m_rules = rules;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyService::SetSecondInMs( UInt32 secondInMs )
{GUCEF_TRACE;

    m_secondInMs = secondInMs > 0 ? secondInMs : 1;
}

/*-------------------------------------------------------------------------*/

CTestLobbyHost*
CTestLobbyService::FindHost( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    SRegistry::THostMap::iterator i = GetRegistry().hosts.find( lobbyId );
    if ( i == GetRegistry().hosts.end() )
        return GUCEF_NULL;
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CTestLobbyService::Reset( void )
{GUCEF_TRACE;

    SRegistry& registry = GetRegistry();
    SRegistry::THostMap hosts;
    hosts.swap( registry.hosts );
    SRegistry::THostMap::iterator i = hosts.begin();
    while ( i != hosts.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    registry.published.clear();
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "secondInMs", m_secondInMs );
    config.SetAttribute( "endpoints", CORE::ToString( m_localEndpoints ) );
    m_rules.SaveConfig( *config.AddChild( "Rules", GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestLobbyService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    SetSecondInMs( config.GetAttributeValueOrChildValueByName( "secondInMs" ).AsUInt32( m_secondInMs, true ) );
    CORE::CString endpoints = config.GetAttributeValueOrChildValueByName( "endpoints" ).AsString( CORE::CString::Empty, true );
    if ( !endpoints.IsNULLOrEmpty() )
        m_localEndpoints = endpoints.ParseElements( ',', false );
    const CORE::CDataNode* rulesNode = config.FindChild( "Rules" );
    if ( GUCEF_NULL != rulesNode )
        m_rules.LoadConfig( *rulesNode );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestLobbyService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CTestLobbyService";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/
