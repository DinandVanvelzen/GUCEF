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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATASTREAM_H
#include "gucefDRN_CDRNDataStream.h"
#define GUCEF_DRN_CDRNDATASTREAM_H
#endif /* GUCEF_DRN_CDRNDATASTREAM_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

#include "gucefMULTIPLAY_CGucefLobby.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGucefLobby::LobbyReadyEvent = "GUCEF::MULTIPLAY::CGucefLobby::LobbyReadyEvent";
const CORE::CEvent CGucefLobby::LobbyFailedEvent = "GUCEF::MULTIPLAY::CGucefLobby::LobbyFailedEvent";
const CORE::CEvent CGucefLobby::ChatEntryEvent = "GUCEF::MULTIPLAY::CGucefLobby::ChatEntryEvent";

enum
{
    PROPOSALGRID_LOBBY      = 0,
    PROPOSALGRID_MEMBERS    = 1,
    PROPOSALGRID_PROPOSALS  = 2
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CVariant
Get( const DRN::CDRNDataGrid* grid, const CORE::CString& key )
{GUCEF_TRACE;

    CORE::CVariant value;
    if ( GUCEF_NULL != grid )
        grid->GetValue( CORE::CVariant( key ), value );
    return value;
}

bool
JoinOrderLess( const CLobbyMember& a, const CLobbyMember& b )
{
    return a.joinOrder < b.joinOrder;
}

bool
SameSlots( const CLobbyState::TSlotVector& a, const CLobbyState::TSlotVector& b )
{GUCEF_TRACE;

    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
    {
        if ( a[ i ].index != b[ i ].index || a[ i ].team != b[ i ].team || a[ i ].faction != b[ i ].faction ||
             a[ i ].colour != b[ i ].colour || a[ i ].kind != b[ i ].kind || a[ i ].aiType != b[ i ].aiType ||
             a[ i ].memberId != b[ i ].memberId )
            return false;
    }
    return true;
}

bool
SameMembers( const CLobbyState::TMemberVector& a, const CLobbyState::TMemberVector& b )
{GUCEF_TRACE;

    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
    {
        if ( a[ i ].id != b[ i ].id || a[ i ].name != b[ i ].name || a[ i ].role != b[ i ].role || a[ i ].ready != b[ i ].ready ||
             a[ i ].slot != b[ i ].slot || a[ i ].latencyMs != b[ i ].latencyMs )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGucefLobby::RegisterEvents( void )
{GUCEF_TRACE;

    LobbyReadyEvent.Initialize();
    LobbyFailedEvent.Initialize();
    ChatEntryEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGucefLobby::CGucefLobby( const CORE::CString& lobbyId, const CORE::CString& localMemberId )
    : CILobby()
    , m_node()
    , m_session( GUCEF_NULL )
    , m_lobbyGrid( GUCEF_NULL )
    , m_membersGrid( GUCEF_NULL )
    , m_slotsGrid( GUCEF_NULL )
    , m_schemaGrid( GUCEF_NULL )
    , m_proposalsGrid( GUCEF_NULL )
    , m_chat( GUCEF_NULL )
    , m_commands( GUCEF_NULL )
    , m_lobbyId( lobbyId )
    , m_localMemberId( localMemberId )
    , m_state()
    , m_schema()
    , m_countdown( -1 )
    , m_joined( false )
    , m_ready( false )
    , m_attached( false )
    , m_subscribedToProposals( false )
    , m_asSpectator( false )
    , m_startCount( 0 )
    , m_lastEndedVoteId()
    , m_startedVoteIds()
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<3; ++i )
        m_proposals[ i ].nextId = 0;
}

/*-------------------------------------------------------------------------*/

CGucefLobby::~CGucefLobby()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_session && m_session->IsActive() )
        m_session->Leave();
    m_node.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Start( const DRN::CDRNEndpointList& endpoints, const CORE::CString& token, bool asSpectator )
{GUCEF_TRACE;

    m_asSpectator = asSpectator;
    DRN::CDRNNodeSettings settings;
    settings.serviceName = CGucefLobbyProtocol::ServiceName;
    settings.echoIntervalMs = 100;
    settings.pulseIntervalMs = 1;
    if ( !m_node.SetSettings( settings ) || !m_node.Start() )
        return false;

    m_session = m_node.CreateSession();
    if ( GUCEF_NULL == m_session )
        return false;
    SubscribeTo( m_session );

    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = CGucefLobbyProtocol::MakeLobbySessionName( m_lobbyId );
    DRN::CDRNAuthSettings credentials;
    CGucefLobbyAuthenticator::MakeTokenCredentials( token, credentials );
    m_attached = m_session->Join( endpoints, sessionSettings, &credentials );
    return m_attached;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::IsReady( void ) const
{GUCEF_TRACE;

    return m_ready;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::IsMember( void ) const
{GUCEF_TRACE;

    return m_attached && m_joined;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobby::GetLobbyId( void ) const
{GUCEF_TRACE;

    return m_lobbyId;
}

/*-------------------------------------------------------------------------*/

const CLobbyState&
CGucefLobby::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

const CLobbySettingsSchema&
CGucefLobby::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobby::GetLocalMemberId( void ) const
{GUCEF_TRACE;

    return m_localMemberId;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNNode&
CGucefLobby::GetNode( void )
{GUCEF_TRACE;

    return m_node;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::OnJoined( void )
{GUCEF_TRACE;

    m_joined = true;
    DRN::CDRNDataStreamSettings commands;
    commands.name = CGucefLobbyProtocol::CommandsStream;
    commands.backlogLimit = 1000;
    // What goes out before the server subscribed reaches it as the history of the stream
    commands.retainedHistory = 64;
    m_commands = m_session->CreateStream( commands );

    m_session->Subscribe( CGucefLobbyProtocol::LobbyGrid );
    m_session->Subscribe( CGucefLobbyProtocol::MembersGrid );
    m_session->Subscribe( CGucefLobbyProtocol::SlotsGrid );
    m_session->Subscribe( CGucefLobbyProtocol::SchemaGrid );
    m_session->Subscribe( CGucefLobbyProtocol::ChatStream );
    if ( !m_asSpectator )
    {
        m_session->Subscribe( CGucefLobbyProtocol::ProposalsGrid );
        m_subscribedToProposals = true;
    }
    FindItems();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::FindItems( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_session )
        return;
    DRN::CDRNDataGrid** grids[] = { &m_lobbyGrid, &m_membersGrid, &m_slotsGrid, &m_schemaGrid, &m_proposalsGrid };
    const CORE::CString* names[] = { &CGucefLobbyProtocol::LobbyGrid, &CGucefLobbyProtocol::MembersGrid, &CGucefLobbyProtocol::SlotsGrid,
                                     &CGucefLobbyProtocol::SchemaGrid, &CGucefLobbyProtocol::ProposalsGrid };
    for ( UInt32 i=0; i<5; ++i )
    {
        if ( GUCEF_NULL != *grids[ i ] )
            continue;
        DRN::CDRNDataGrid* grid = m_session->FindGrid( *names[ i ] );
        if ( GUCEF_NULL == grid )
            continue;
        *grids[ i ] = grid;
        grid->SetNotifyMode( DRN::DRN_NOTIFY_PER_PACKET );
        SubscribeTo( grid );
    }
    if ( GUCEF_NULL == m_chat )
    {
        m_chat = m_session->FindStream( CGucefLobbyProtocol::ChatStream );
        if ( GUCEF_NULL != m_chat )
            SubscribeTo( m_chat );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::UpdateProposalsSubscription( void )
{GUCEF_TRACE;

    const CLobbyMember* self = m_state.FindMember( m_localMemberId );
    if ( GUCEF_NULL == self || GUCEF_NULL == m_session || !m_attached )
        return;
    bool player = !self->IsSpectator();
    if ( player && !m_subscribedToProposals )
    {
        m_subscribedToProposals = true;
        m_session->Subscribe( CGucefLobbyProtocol::ProposalsGrid );
        FindItems();
    }
    else
    if ( !player && m_subscribedToProposals )
    {
        // Spectators do not vote: the voters are the members that have the grid
        m_subscribedToProposals = false;
        m_session->Unsubscribe( CGucefLobbyProtocol::ProposalsGrid );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::CheckReady( void )
{GUCEF_TRACE;

    if ( m_ready || !m_joined )
        return;
    DRN::CDRNDataGrid* grids[] = { m_lobbyGrid, m_membersGrid, m_slotsGrid, m_schemaGrid };
    for ( UInt32 i=0; i<4; ++i )
        if ( GUCEF_NULL == grids[ i ] || !grids[ i ]->IsSnapshotComplete() )
            return;
    if ( GUCEF_NULL == m_state.FindMember( m_localMemberId ) )
        return;
    m_ready = true;

    // Joining a game in progress: the launch description comes when asked for, the lobby is in place by then
    if ( LOBBYPHASE_IN_GAME == m_state.phase )
        SendCommand( "launch" );
    NotifyObservers( LobbyReadyEvent );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::Rebuild( void )
{GUCEF_TRACE;

    CLobbyState state;
    state.lobbyId = m_lobbyId;
    state.name = Get( m_lobbyGrid, "name" ).AsString( CORE::CString::Empty, true );
    state.phase = StringToLobbyPhase( Get( m_lobbyGrid, "phase" ).AsString( CORE::CString::Empty, true ) );
    state.bossId = Get( m_lobbyGrid, "boss" ).AsString( CORE::CString::Empty, true );
    state.chatBackend = Get( m_lobbyGrid, "chat/backend" ).AsString( CORE::CString::Empty, true );
    state.chatChannel = Get( m_lobbyGrid, "chat/channel" ).AsString( CORE::CString::Empty, true );
    Int32 countdown = Get( m_lobbyGrid, "countdown" ).AsInt32( -1, true );

    state.activeVote.id = Get( m_lobbyGrid, "vote/id" ).AsString( CORE::CString::Empty, true );
    if ( state.activeVote.IsActive() )
    {
        CLobbyVote& vote = state.activeVote;
        vote.what = Get( m_lobbyGrid, "vote/what" ).AsString( CORE::CString::Empty, true );
        vote.proposer = Get( m_lobbyGrid, "vote/proposer" ).AsString( CORE::CString::Empty, true );
        vote.key = Get( m_lobbyGrid, "vote/key" ).AsString( CORE::CString::Empty, true );
        vote.value = Get( m_lobbyGrid, "vote/value" ).AsString( CORE::CString::Empty, true );
        vote.yes = Get( m_lobbyGrid, "vote/yes" ).AsUInt32( 0, true );
        vote.no = Get( m_lobbyGrid, "vote/no" ).AsUInt32( 0, true );
        vote.needed = Get( m_lobbyGrid, "vote/needed" ).AsUInt32( 0, true );
    }
    CORE::CString lastVoteId = Get( m_lobbyGrid, "vote/lastId" ).AsString( CORE::CString::Empty, true );
    CORE::CString lastVoteResult = Get( m_lobbyGrid, "vote/lastResult" ).AsString( CORE::CString::Empty, true );

    state.settings.SetAllowMultipleValues( false );
    GUCEF::vector< CORE::CVariant > keys;
    if ( GUCEF_NULL != m_lobbyGrid )
    {
        m_lobbyGrid->GetKeys( keys );
        for ( size_t i=0; i<keys.size(); ++i )
        {
            CORE::CString key = keys[ i ].AsString();
            if ( 0 == key.HasSubstr( "settings/", true ) )
                state.settings.Set( key.CutChars( 9, true ), Get( m_lobbyGrid, key ).AsString( CORE::CString::Empty, true ) );
        }
    }

    if ( GUCEF_NULL != m_membersGrid )
    {
        typedef GUCEF::map< CORE::CString, CLobbyMember > TMemberMap;
        TMemberMap members;
        keys.clear();
        m_membersGrid->GetKeys( keys );
        for ( size_t i=0; i<keys.size(); ++i )
        {
            CORE::CString key = keys[ i ].AsString();
            CORE::CString memberId, field;
            if ( !CGucefLobbyProtocol::SplitMemberKey( key, memberId, field ) )
                continue;
            CLobbyMember& member = members[ memberId ];
            member.id = memberId;
            CORE::CVariant value = Get( m_membersGrid, key );
            if ( "name" == field ) member.name = value.AsString( CORE::CString::Empty, true );
            else if ( "role" == field ) member.role = StringToLobbyMemberRole( value.AsString( CORE::CString::Empty, true ) );
            else if ( "ready" == field ) member.ready = value.AsBool( false, true );
            else if ( "slot" == field ) member.slot = value.AsInt32( -1, true );
            else if ( "latency" == field ) member.latencyMs = value.AsInt32( -1, true );
            else if ( "joinOrder" == field ) member.joinOrder = value.AsUInt32( 0, true );
        }
        TMemberMap::iterator m = members.begin();
        while ( m != members.end() )
        {
            // A member is there once the server published its role
            if ( m_membersGrid->HasKey( CORE::CVariant( (*m).first + "/role" ) ) )
                state.members.push_back( (*m).second );
            ++m;
        }
        std::sort( state.members.begin(), state.members.end(), JoinOrderLess );
    }

    if ( GUCEF_NULL != m_slotsGrid )
    {
        UInt32 count = Get( m_slotsGrid, "count" ).AsUInt32( 0, true );
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CString prefix = CORE::ToString( i ) + "/";
            CLobbySlot slot;
            slot.index = Get( m_slotsGrid, prefix + "index" ).AsUInt32( i, true );
            slot.team = Get( m_slotsGrid, prefix + "team" ).AsUInt32( 0, true );
            slot.faction = Get( m_slotsGrid, prefix + "faction" ).AsString( CORE::CString::Empty, true );
            slot.colour = Get( m_slotsGrid, prefix + "colour" ).AsString( CORE::CString::Empty, true );
            slot.kind = StringToLobbySlotKind( Get( m_slotsGrid, prefix + "kind" ).AsString( CORE::CString::Empty, true ) );
            slot.aiType = Get( m_slotsGrid, prefix + "aiType" ).AsString( CORE::CString::Empty, true );
            slot.memberId = Get( m_slotsGrid, prefix + "member" ).AsString( CORE::CString::Empty, true );
            state.slots.push_back( slot );
        }
    }

    if ( GUCEF_NULL != m_schemaGrid && m_schema.settings.empty() )
    {
        CORE::CDataNode tree;
        if ( CGucefLobbyProtocol::VariantToDataTree( Get( m_schemaGrid, "schema" ), tree ) )
            m_schema.LoadConfig( tree );
    }

    // What changed, told once the new state is in place
    CORE::CString::StringVector paths;
    if ( state.name != m_state.name ) paths.push_back( "name" );
    if ( state.phase != m_state.phase ) paths.push_back( "phase" );
    if ( state.bossId != m_state.bossId ) paths.push_back( "boss" );
    if ( !SameMembers( state.members, m_state.members ) ) paths.push_back( "members" );
    if ( !SameSlots( state.slots, m_state.slots ) ) paths.push_back( "slots" );
    if ( state.activeVote.id != m_state.activeVote.id ) paths.push_back( "vote" );
    UInt32 settingCount = state.settings.GetKeyCount();
    for ( UInt32 i=0; i<settingCount; ++i )
    {
        CORE::CString key = state.settings.GetKey( i ).AsString();
        if ( m_state.GetSetting( key ) != state.settings.GetValue( i ).AsString() || !m_state.settings.HasKey( key ) )
            paths.push_back( "settings/" + key );
    }

    CORE::CString::StringVector joined, left;
    for ( size_t i=0; i<state.members.size(); ++i )
        if ( GUCEF_NULL == m_state.FindMember( state.members[ i ].id ) )
            joined.push_back( state.members[ i ].id );
    for ( size_t i=0; i<m_state.members.size(); ++i )
        if ( GUCEF_NULL == state.FindMember( m_state.members[ i ].id ) )
            left.push_back( m_state.members[ i ].id );

    CLobbyVote oldVote = m_state.activeVote;
    Int32 oldCountdown = m_countdown;
    bool wasReady = m_ready;
    m_state = state;
    m_countdown = countdown;

    UpdateProposalsSubscription();
    CheckReady();
    if ( !wasReady )
    {
        // What happened before this member joined is not news to it
        m_lastEndedVoteId = lastVoteId;
        if ( state.activeVote.IsActive() )
            m_startedVoteIds.insert( state.activeVote.id );
        return;
    }

    for ( size_t i=0; i<joined.size(); ++i )
    {
        TMemberEventData eventData( joined[ i ] );
        if ( !NotifyObservers( MemberJoinedEvent, &eventData ) ) return;
    }
    for ( size_t i=0; i<left.size(); ++i )
    {
        TMemberEventData eventData( left[ i ] );
        if ( !NotifyObservers( MemberLeftEvent, &eventData ) ) return;
    }
    // Every vote the server ended is reported, also one that started and ended between two updates of this member
    bool voteEnded = !lastVoteId.IsNULLOrEmpty() && lastVoteId != m_lastEndedVoteId;
    if ( voteEnded )
    {
        m_lastEndedVoteId = lastVoteId;
        CLobbyVote endedVote = oldVote.id == lastVoteId ? oldVote : CLobbyVote();
        endedVote.id = lastVoteId;
        if ( !m_startedVoteIds.erase( lastVoteId ) )
        {
            CLobbyVoteEventData started( endedVote, false );
            if ( !NotifyObservers( VoteStartedEvent, &started ) ) return;
        }
        CLobbyVoteEventData eventData( endedVote, "passed" == lastVoteResult );
        if ( !NotifyObservers( VoteEndedEvent, &eventData ) ) return;
    }
    if ( state.activeVote.IsActive() && oldVote.id != state.activeVote.id && state.activeVote.id != m_lastEndedVoteId )
    {
        m_startedVoteIds.insert( state.activeVote.id );
        CLobbyVoteEventData eventData( state.activeVote, false );
        if ( !NotifyObservers( VoteStartedEvent, &eventData ) ) return;
    }
    if ( countdown != oldCountdown && ( countdown >= 0 || ( oldCountdown > 0 && LOBBYPHASE_OPEN == state.phase ) ) )
    {
        TCountdownEventData eventData( countdown );
        if ( !NotifyObservers( CountdownEvent, &eventData ) ) return;
    }
    if ( !paths.empty() )
    {
        TLobbyStateChangedEventData eventData( paths );
        NotifyObservers( LobbyStateChangedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Propose( UInt32 gridIndex, DRN::CDRNDataGrid* grid, const CORE::CString& key, const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( !IsMember() || GUCEF_NULL == grid )
        return false;
    if ( !grid->SetValue( CORE::CVariant( key ), value ) )
        return false;
    m_proposals[ gridIndex ].keys[ ++m_proposals[ gridIndex ].nextId ] = key;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::WriteMemberKey( const CORE::CString& field, const CORE::CVariant& value )
{GUCEF_TRACE;

    return Propose( PROPOSALGRID_MEMBERS, m_membersGrid, m_localMemberId + "/" + field, value );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SendCommand( const CORE::CString& verb, const CORE::CVariant* arg1, const CORE::CVariant* arg2, const CORE::CVariant* arg3 )
{GUCEF_TRACE;

    if ( !IsMember() || GUCEF_NULL == m_commands )
        return false;
    CORE::CVariantStream& entry = m_commands->GetWriteStream();
    entry << verb;
    const CORE::CVariant* args[ 3 ] = { arg1, arg2, arg3 };
    for ( UInt32 i=0; i<3; ++i )
        if ( GUCEF_NULL != args[ i ] )
            entry << *args[ i ];
    entry.WriteSegmentEnd();
    if ( m_commands->CommitEntry() )
        return true;
    entry.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    CORE::CString ownerId;
    const CLobbySettingDef* def = CLobbyRules::FindSettingDef( m_schema, key, ownerId );
    if ( GUCEF_NULL != def && CLobbySettingDef::CHANGEPOLICY_VOTE == def->changePolicy && !IsBoss() )
        return ProposeSetting( key, value );
    return Propose( PROPOSALGRID_LOBBY, m_lobbyGrid, "settings/" + key, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::ProposeSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    return Propose( PROPOSALGRID_PROPOSALS, m_proposalsGrid, "settings/" + key, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Vote( const CORE::CString& voteId, bool yes )
{GUCEF_TRACE;

    if ( !IsMember() || GUCEF_NULL == m_proposalsGrid || !m_subscribedToProposals )
        return false;
    return m_proposalsGrid->CastVote( CORE::StringToUInt32( voteId, 0 ), yes );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetReady( bool ready )
{GUCEF_TRACE;

    return WriteMemberKey( "ready", CORE::CVariant( ready ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetSpectator( bool spectate )
{GUCEF_TRACE;

    return WriteMemberKey( "spectator", CORE::CVariant( spectate ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::RequestSlot( UInt32 slotIndex )
{GUCEF_TRACE;

    return WriteMemberKey( "slot", CORE::CVariant( (Int32) slotIndex ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetFaction( const CORE::CString& faction )
{GUCEF_TRACE;

    return WriteMemberKey( "faction", CORE::CVariant( faction ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetColour( const CORE::CString& colour )
{GUCEF_TRACE;

    return WriteMemberKey( "colour", CORE::CVariant( colour ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Kick( const CORE::CString& memberId )
{GUCEF_TRACE;

    CORE::CVariant target( memberId );
    return SendCommand( "kick", &target );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Ban( const CORE::CString& memberId )
{GUCEF_TRACE;

    CORE::CVariant target( memberId );
    return SendCommand( "ban", &target );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( memberId.IsNULLOrEmpty() )
        return false;
    CORE::CVariant target( memberId );
    CORE::CVariant why( reason );
    return SendCommand( "report", &target, &why );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::TransferBoss( const CORE::CString& memberId )
{GUCEF_TRACE;

    CORE::CVariant target( memberId );
    return SendCommand( "boss", &target );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SetSlotKind( UInt32 slotIndex, TLobbySlotKind kind, const CORE::CString& aiType )
{GUCEF_TRACE;

    CORE::CVariant slot( slotIndex );
    CORE::CVariant kindArg( CORE::CString( LobbySlotKindToString( kind ) ) );
    CORE::CVariant ai( aiType );
    return SendCommand( "slotkind", &slot, &kindArg, &ai );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::RequestStart( void )
{GUCEF_TRACE;

    if ( IsBoss() )
        return SendCommand( "start" );
    // Every proposal of a start is a new value, so that each one is put to a vote
    return Propose( PROPOSALGRID_PROPOSALS, m_proposalsGrid, "start", CORE::CVariant( m_localMemberId + "/" + CORE::ToString( ++m_startCount ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::ForceStart( void )
{GUCEF_TRACE;

    return SendCommand( "forcestart" );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::SendChat( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CVariant textArg( text );
    return SendCommand( "say", &textArg );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobby::Leave( void )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    Detach( GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::Detach( const CORE::CString* closeReason )
{GUCEF_TRACE;

    if ( !m_attached )
        return;
    m_attached = false;
    if ( GUCEF_NULL != m_session && m_session->IsActive() )
        m_session->Leave();
    if ( GUCEF_NULL != closeReason )
    {
        TLobbyClosedEventData eventData( *closeReason );
        NotifyObservers( LobbyClosedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobby::WhatOf( UInt32 gridIndex, const CORE::CString& key )
{GUCEF_TRACE;

    if ( PROPOSALGRID_LOBBY == gridIndex )
        return "setting";
    if ( PROPOSALGRID_PROPOSALS == gridIndex )
        return "start" == key ? CORE::CString( "start" ) : CORE::CString( "proposal" );
    CORE::CString memberId, field;
    CGucefLobbyProtocol::SplitMemberKey( key, memberId, field );
    return field;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::OnProposeResult( UInt32 gridIndex, UInt32 proposalId, bool accepted, UInt32 outcome, const CORE::CString& reason )
{GUCEF_TRACE;

    CORE::CString key;
    TProposalKeyMap::iterator i = m_proposals[ gridIndex ].keys.find( proposalId );
    if ( i != m_proposals[ gridIndex ].keys.end() )
    {
        key = (*i).second;
        m_proposals[ gridIndex ].keys.erase( i );
    }
    // A vote that failed is the end of a vote, not a refusal of the request
    if ( accepted || DRN::CDRNVotePolicy::VOTEOUTCOME_PENDING != outcome )
        return;
    CLobbyRefusalEventData eventData( WhatOf( gridIndex, key ), reason );
    NotifyObservers( RequestRefusedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::OnAppMessage( const CORE::CDynamicBuffer& payload )
{GUCEF_TRACE;

    DRN::CDRNWireReader reader( payload );
    UInt8 type = 0;
    if ( !reader.ReadUInt8( type ) )
        return;

    if ( CGucefLobbyProtocol::MESSAGE_LAUNCH == type )
    {
        CORE::CDataNode tree;
        CLaunchDescription launch;
        if ( CGucefLobbyProtocol::ReadDataTree( reader, tree ) && launch.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) )
            NotifyObservers( LaunchEvent, &launch );
        return;
    }

    CORE::CString a, b;
    reader.ReadString( a );
    reader.ReadString( b );
    if ( CGucefLobbyProtocol::MESSAGE_REFUSED == type )
    {
        CLobbyRefusalEventData eventData( a, b );
        NotifyObservers( RequestRefusedEvent, &eventData );
    }
    else
    if ( CGucefLobbyProtocol::MESSAGE_KICKED == type || CGucefLobbyProtocol::MESSAGE_CLOSED == type )
    {
        Detach( &a );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::OnChatEntries( DRN::CDRNDataStream& stream )
{GUCEF_TRACE;

    CORE::CVariantStream entry;
    while ( stream.ReadNextEntry( entry ) )
    {
        CORE::CString fromId, fromName, text, id;
        UInt64 timeMs = 0;
        entry >> fromId >> fromName >> text >> timeMs >> id;
        entry.Clear();

        CChatMessage message;
        message.channel = "lobby:" + m_lobbyId;
        message.from = fromName;
        message.text = text;
        message.time.FromUnixEpochBasedTicksInMillisecs( timeMs );
        message.id = id;
        if ( !NotifyObservers( ChatEntryEvent, &message ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobby::OnNotify( CORE::CNotifier* notifier                 ,
                       const CORE::CEvent& eventId               ,
                       CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_session && notifier == m_session )
    {
        if ( DRN::CDRNSession::JoinedEvent == eventId )
        {
            OnJoined();
        }
        else
        if ( DRN::CDRNSession::JoinFailedEvent == eventId )
        {
            m_attached = false;
            CORE::TCloneableString reason( GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CORE::CString( "Could not join the lobby" ) );
            NotifyObservers( LobbyFailedEvent, &reason );
        }
        else
        if ( DRN::CDRNSession::ItemAnnouncedEvent == eventId )
        {
            FindItems();
        }
        else
        if ( DRN::CDRNSession::AppMessageEvent == eventId )
        {
            OnAppMessage( static_cast< DRN::CDRNAppMessageEventData* >( eventData )->GetData().payload );
        }
        else
        if ( DRN::CDRNSession::SessionClosedEvent == eventId )
        {
            if ( !m_ready && m_attached )
            {
                m_attached = false;
                CORE::TCloneableString reason( "The lobby session ended" );
                NotifyObservers( LobbyFailedEvent, &reason );
                return;
            }
            CORE::CString reason( "The lobby session ended" );
            Detach( &reason );
        }
        return;
    }

    if ( GUCEF_NULL != m_chat && notifier == m_chat )
    {
        if ( DRN::CDRNDataStream::EntriesAvailableEvent == eventId )
            OnChatEntries( *m_chat );
        return;
    }

    if ( DRN::CDRNDataGrid::ProposeResultEvent == eventId )
    {
        const DRN::CDRNProposeResult& result = static_cast< DRN::CDRNProposeResultEventData* >( eventData )->GetData();
        UInt32 gridIndex = notifier == m_lobbyGrid ? PROPOSALGRID_LOBBY : ( notifier == m_membersGrid ? PROPOSALGRID_MEMBERS : PROPOSALGRID_PROPOSALS );
        OnProposeResult( gridIndex, result.proposalId, result.accepted, result.voteOutcome, result.reason );
        return;
    }

    if ( DRN::CDRNDataGrid::GridUpdatedEvent == eventId || DRN::CDRNDataGrid::GridKeyChangedEvent == eventId ||
         DRN::CDRNDataItem::SnapshotCompleteEvent == eventId )
    {
        Rebuild();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobby::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobby::ClassTypeName( void )
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobby";
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
