/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#include "lobbyservicepluginEOS_CEosLobbyService.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSERVICE_H ? */

#include "lobbyservicepluginEOS_CEosLobby.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

using namespace PLUGINGLUE::EOS;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
MembersEqual( const MULTIPLAY::CLobbyState::TMemberVector& a, const MULTIPLAY::CLobbyState::TMemberVector& b )
{GUCEF_TRACE;

    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
    {
        if ( a[ i ].id != b[ i ].id || a[ i ].name != b[ i ].name || a[ i ].role != b[ i ].role || a[ i ].ready != b[ i ].ready ||
             a[ i ].slot != b[ i ].slot || a[ i ].joinOrder != b[ i ].joinOrder )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SlotsEqual( const MULTIPLAY::CLobbyState::TSlotVector& a, const MULTIPLAY::CLobbyState::TSlotVector& b )
{GUCEF_TRACE;

    if ( a.size() != b.size() )
        return false;
    for ( size_t i=0; i<a.size(); ++i )
    {
        if ( a[ i ].index != b[ i ].index || a[ i ].team != b[ i ].team || a[ i ].faction != b[ i ].faction || a[ i ].colour != b[ i ].colour ||
             a[ i ].kind != b[ i ].kind || a[ i ].aiType != b[ i ].aiType || a[ i ].memberId != b[ i ].memberId )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
VotesEqual( const MULTIPLAY::CLobbyVote& a, const MULTIPLAY::CLobbyVote& b )
{GUCEF_TRACE;

    return a.id == b.id && a.what == b.what && a.proposer == b.proposer && a.key == b.key && a.value == b.value &&
           a.yes == b.yes && a.no == b.no && a.needed == b.needed && a.voters.size() == b.voters.size();
}

/*-------------------------------------------------------------------------*/

static CORE::CString
SettingOf( const CORE::CValueList& settings, const CORE::CString& key )
{GUCEF_TRACE;

    return settings.GetValueAlways( key ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

static void
CollectSettingChanges( const CORE::CValueList& a, const CORE::CValueList& b, CORE::CString::StringVector& changed )
{GUCEF_TRACE;

    UInt32 count = b.GetKeyCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString key = b.GetKey( i ).AsString();
        if ( !a.HasKey( key ) || SettingOf( a, key ) != SettingOf( b, key ) )
            changed.push_back( key );
    }
    count = a.GetKeyCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString key = a.GetKey( i ).AsString();
        if ( !b.HasKey( key ) )
            changed.push_back( key );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosLobby::CEosLobby( CEosLobbyService& service, const CORE::CString& lobbyId, const CORE::CString& localUser, bool attach )
    : MULTIPLAY::CILobby()
    , m_service( &service )
    , m_lobbyId( lobbyId )
    , m_localUser( localUser )
    , m_stage( STAGE_NEW )
    , m_isOwner( false )
    , m_ownerUserId()
    , m_state()
    , m_ownerState()
    , m_haveOwnerState( false )
    , m_lastGoodState()
    , m_schema()
    , m_params()
    , m_rules()
    , m_summary()
    , m_password()
    , m_joinPassword()
    , m_joinAsSpectator( false )
    , m_hasDefinition( false )
    , m_closing( false )
    , m_closeDestroyAtInMs( 0 )
    , m_wireAttributes()
    , m_definitionAttributes()
    , m_forceFull( true )
    , m_dirty( false )
    , m_updateInFlight( false )
    , m_updateRequestId( 0 )
    , m_memberWriteRequestId( 0 )
    , m_promoteRequestId( 0 )
    , m_updateFailures( 0 )
    , m_nextPublishInMs( 0 )
    , m_updateCount( 0 )
    , m_revision( 0 )
    , m_publishedRevision( 0 )
    , m_confirmedRevision( 0 )
    , m_closeRevision( 0 )
    , m_lastRevisionSeen( 0 )
    , m_updateBucket()
    , m_visible( true )
    , m_visibilityChange( false )
    , m_processedSequences()
    , m_acks()
    , m_refusals()
    , m_rejections()
    , m_bans()
    , m_pendingJoins()
    , m_memberBuckets()
    , m_kickQueue()
    , m_countdownLeft( -1 )
    , m_voteSecondsLeft( -1 )
    , m_voteCount( 0 )
    , m_lastSecondInMs( 0 )
    , m_lastEndedVote()
    , m_lastEndedPassed( false )
    , m_closeReason()
    , m_launchPublished( false )
    , m_haveLaunchKey( false )
    , m_launchPublicKey()
    , m_launch()
    , m_haveLaunch( false )
    , m_lastTokenSent()
    , m_promoteTarget()
    , m_nextSequence( 0 )
    , m_requests()
    , m_requestInFlight( false )
    , m_inFlight()
    , m_lastAck( 0 )
    , m_lastRefusalSequence( 0 )
    , m_pendingAck( 0 )
    , m_pendingRefusal()
    , m_rejection()
    , m_lastCountdownSeen( -1 )
    , m_countdownFromOwner( -1 )
    , m_lastVoteStarted()
    , m_lastVoteEndedSeen()
    , m_launchRaised( false )
    , m_launchWanted( false )
    , m_launchTemplate()
    , m_token()
    , m_nextTokenRequestInMs( 0 )
    , m_tokenDeadlineInMs( 0 )
    , m_joinDeadlineInMs( 0 )
    , m_router( GUCEF_NULL )
    , m_gameSocket( GUCEF_NULL )
    , m_controlSocket( GUCEF_NULL )
{GUCEF_TRACE;

    memset( m_launchKey, 0, sizeof( m_launchKey ) );
    const CEosLobbyServiceSettings& settings = service.GetSettings();
    m_updateBucket.SetRate( settings.updatesPerSecond, settings.updateBurst );
    if ( attach )
        m_service->Register( this );
}

/*-------------------------------------------------------------------------*/

CEosLobby::~CEosLobby()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();

    // A lobby that goes away leaves it, without a word
    if ( GUCEF_NULL != m_service && m_closing && 0 != m_closeDestroyAtInMs )
    {
        // The reason was published and the lobby was waiting for its members to read it
        m_service->RequestDestroy( m_lobbyId );
    }
    else
    if ( GUCEF_NULL != m_service && ( STAGE_ACTIVE == m_stage || STAGE_STARTING == m_stage ) && !m_closing )
    {
        if ( m_isOwner && m_state.members.size() <= 1 )
            m_service->RequestDestroy( m_lobbyId );
        else
        if ( m_isOwner && !m_password.IsNULLOrEmpty() )
            m_service->RequestDestroy( m_lobbyId );
        else
            m_service->RequestLeave( m_lobbyId );
    }
    if ( GUCEF_NULL != m_service )
        m_service->Unregister( this );
    ReleaseSockets();
    memset( m_launchKey, 0, sizeof( m_launchKey ) );
    m_password.Clear();
    m_joinPassword.Clear();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::ReleaseSockets( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_router )
    {
        if ( GUCEF_NULL != m_gameSocket )
            m_router->ReleaseSocket( m_gameSocket );
        if ( GUCEF_NULL != m_controlSocket )
            m_router->ReleaseSocket( m_controlSocket );
    }
    m_gameSocket = GUCEF_NULL;
    m_controlSocket = GUCEF_NULL;
    m_router = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyState&
CEosLobby::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbySettingsSchema&
CEosLobby::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobby::GetLocalMemberId( void ) const
{GUCEF_TRACE;

    return m_localUser;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobby::GetLobbyId( void ) const
{GUCEF_TRACE;

    return m_lobbyId;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::IsOwner( void ) const
{GUCEF_TRACE;

    return m_isOwner;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::IsMember( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_service && STAGE_DETACHED != m_stage && !m_closing;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::IsReady( void ) const
{GUCEF_TRACE;

    return STAGE_ACTIVE == m_stage;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobby::GetUpdateCount( void ) const
{GUCEF_TRACE;

    return m_updateCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobby::GetRevision( void ) const
{GUCEF_TRACE;

    return m_isOwner ? m_revision : m_lastRevisionSeen;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::HasLaunchDescription( void ) const
{GUCEF_TRACE;

    return m_launchRaised;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLaunchDescription&
CEosLobby::GetLaunchDescription( void ) const
{GUCEF_TRACE;

    return m_launch;
}

/*-------------------------------------------------------------------------*/

PLUGINGLUE::EOS::CEosP2PSocket*
CEosLobby::GetGameSocket( void ) const
{GUCEF_TRACE;

    return m_gameSocket;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobby::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobby";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::StateChanged( const CORE::CString& path )
{GUCEF_TRACE;

    MULTIPLAY::CILobby::TLobbyStateChangedEventData paths;
    paths.GetData().push_back( path );
    NotifyObservers( LobbyStateChangedEvent, &paths );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::Refuse( const CORE::CString& what, const CORE::CString& reason )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyRefusalEventData refusal( what, reason );
    NotifyObservers( RequestRefusedEvent, &refusal );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::Detach( const CORE::CString& closeReason, bool raiseClosed )
{GUCEF_TRACE;

    if ( STAGE_DETACHED == m_stage )
        return;
    m_stage = STAGE_DETACHED;
    m_requests.clear();
    m_requestInFlight = false;
    m_pendingJoins.clear();
    ReleaseSockets();
    if ( GUCEF_NULL != m_service )
        m_service->Unregister( this );

    // The lobby object stays valid for whoever still holds it: the last state it saw
    if ( raiseClosed )
    {
        CORE::TCloneableString reason( closeReason );
        NotifyObservers( LobbyClosedEvent, &reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnServiceDisconnected( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( STAGE_DETACHED == m_stage )
        return;
    if ( GUCEF_NULL != m_service && ( STAGE_ACTIVE == m_stage || STAGE_STARTING == m_stage ) )
    {
        bool destroy = m_isOwner && ( m_closing || m_state.members.size() <= 1 || !m_password.IsNULLOrEmpty() );
        if ( destroy )
            m_service->RequestDestroy( m_lobbyId );
        else
            m_service->RequestLeave( m_lobbyId );
    }
    m_closing = false;
    Detach( reason, true );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnServiceGone( void )
{GUCEF_TRACE;

    m_service = GUCEF_NULL;
    if ( STAGE_DETACHED != m_stage )
        Detach( "The lobby service ended", true );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::Leave( void )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;

    if ( m_isOwner )
    {
        // The owner of a lobby with others in it hands it over to EOS, unless there is a password: that lobby ends with its owner
        bool alone = m_state.members.size() <= 1;
        if ( alone || !m_password.IsNULLOrEmpty() )
        {
            HostClose( alone ? "The lobby is empty" : "The host left" );
            return true;
        }
    }
    m_service->RequestLeave( m_lobbyId );
    Detach( CORE::CString::Empty, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::InvitePlayer( const CORE::CString& userId )
{GUCEF_TRACE;

    if ( !IsMember() || !CEosLobbyCodec::IsValidUserId( userId ) || userId == m_localUser )
        return false;
    return 0 != m_service->RequestInvite( m_lobbyId, userId );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason )
{GUCEF_TRACE;

    // The Reports interface of EOS needs the sanction and reporting setup of the product; it is not used
    Refuse( "report", "Reports are not supported by the EOS lobby service" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SendRequest( const CORE::CString& what, const CORE::CString& verb, const CORE::CString& arg1, const CORE::CString& arg2 )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;

    // The owner is the rules: it asks itself
    if ( m_isOwner )
    {
        HostRequestLocal( what, verb, arg1, arg2 );
        return true;
    }

    SRequest request;
    request.sequence = ++m_nextSequence;
    request.what = what;
    request.isReady = false;
    request.readyValue = false;
    request.verb = verb;
    request.arg1 = arg1;
    request.arg2 = arg2;
    request.deadlineInMs = 0;
    m_requests.push_back( request );
    PumpRequests( PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    return SendRequest( "setting", "set", key, value );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::ProposeSetting( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    return SendRequest( "proposal", "prop", key, value );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::Vote( const CORE::CString& voteId, bool yes )
{GUCEF_TRACE;

    return SendRequest( "vote", "vote", voteId, yes ? "1" : "0" );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetReady( bool ready )
{GUCEF_TRACE;

    if ( !IsMember() )
        return false;
    if ( m_isOwner )
    {
        HostSetReady( m_localUser, ready, 0 );
        return true;
    }
    SRequest request;
    request.sequence = ++m_nextSequence;
    request.what = "ready";
    request.isReady = true;
    request.readyValue = ready;
    request.deadlineInMs = 0;
    m_requests.push_back( request );
    PumpRequests( PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetSpectator( bool spectate )
{GUCEF_TRACE;

    return SendRequest( "spectator", "spec", spectate ? "1" : "0", CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::RequestSlot( UInt32 slotIndex )
{GUCEF_TRACE;

    return SendRequest( "slot", "slot", CORE::UInt32ToString( slotIndex ), CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetFaction( const CORE::CString& faction )
{GUCEF_TRACE;

    return SendRequest( "faction", "fac", faction, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetColour( const CORE::CString& colour )
{GUCEF_TRACE;

    return SendRequest( "colour", "col", colour, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::Kick( const CORE::CString& memberId )
{GUCEF_TRACE;

    return SendRequest( "kick", "kick", memberId, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::Ban( const CORE::CString& memberId )
{GUCEF_TRACE;

    return SendRequest( "ban", "ban", memberId, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::TransferBoss( const CORE::CString& memberId )
{GUCEF_TRACE;

    return SendRequest( "transferBoss", "boss", memberId, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::SetSlotKind( UInt32 slotIndex, MULTIPLAY::TLobbySlotKind kind, const CORE::CString& aiType )
{GUCEF_TRACE;

    return SendRequest( "slotKind", "slotkind", CORE::UInt32ToString( slotIndex ), CORE::UInt32ToString( (UInt32) kind ) + ":" + aiType );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::RequestStart( void )
{GUCEF_TRACE;

    return SendRequest( "start", "start", CORE::CString::Empty, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::ForceStart( void )
{GUCEF_TRACE;

    return SendRequest( "forceStart", "fstart", CORE::CString::Empty, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::PumpRequests( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( !IsMember() || m_isOwner )
        return;

    // An answer that does not come ends the request, the next one goes
    if ( m_requestInFlight && 0 != m_inFlight.deadlineInMs && nowInMs >= m_inFlight.deadlineInMs )
    {
        m_requestInFlight = false;
        Refuse( m_inFlight.what, "The host did not answer" );
    }
    if ( m_requestInFlight || m_requests.empty() || STAGE_ACTIVE != m_stage )
        return;

    // A write of the member attributes still under way holds the next one back
    if ( 0 != m_memberWriteRequestId )
        return;

    SRequest request = m_requests.front();
    m_requests.erase( m_requests.begin() );
    request.deadlineInMs = nowInMs + m_service->GetSettings().requestTimeoutInMs;

    TEosAttributeVector attributes;
    if ( request.isReady )
        attributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyReady, CORE::UInt32ToString( request.sequence ) + ":" + ( request.readyValue ? "1" : "0" ) ) );
    else
        attributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyRequest, CEosLobbyCodec::MakeRequest( request.sequence, request.verb, request.arg1, request.arg2 ) ) );

    if ( !WriteMemberAttributes( attributes ) )
    {
        Refuse( request.what, "The request could not be sent" );
        return;
    }
    m_inFlight = request;
    m_requestInFlight = true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::WriteMemberAttributes( const TEosAttributeVector& attributes )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service )
        return false;
    SEosLobbyUpdateOptions options;
    options.lobbyId = m_lobbyId;
    options.localUser = m_localUser;
    options.setMemberAttributes = attributes;
    UInt32 requestId = m_service->RequestUpdate( m_lobbyId, options );
    if ( 0 == requestId )
        return false;
    m_memberWriteRequestId = requestId;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnAck( UInt32 ackedSequence )
{GUCEF_TRACE;

    if ( ackedSequence > m_lastAck )
        m_lastAck = ackedSequence;
    if ( m_requestInFlight && m_inFlight.sequence <= m_lastAck )
        m_requestInFlight = false;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnRefusal( const CORE::CString& text )
{GUCEF_TRACE;

    // "<sequence>|<what>|<reason>": raised once for the request that waits
    CORE::CString::StringVector parts = text.ParseElements( '|', true );
    if ( parts.size() < 3 || parts[ 0 ].IsNULLOrEmpty() )
        return;
    UInt32 sequence = CORE::StringToUInt32( parts[ 0 ], 0 );
    if ( 0 == sequence || sequence <= m_lastRefusalSequence )
        return;
    m_lastRefusalSequence = sequence;
    Refuse( CEosLobbyCodec::SanitizeText( parts[ 1 ], 32 ), CEosLobbyCodec::SanitizeText( parts[ 2 ], 200 ) );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnRequestEnded( UInt32 requestId, EEosLobbyEventKind kind, EEosResult result )
{GUCEF_TRACE;

    if ( requestId == m_updateRequestId && 0 != requestId )
    {
        m_updateRequestId = 0;
        HostOnUpdateEnded( result );
        return;
    }
    if ( requestId == m_memberWriteRequestId && 0 != requestId )
    {
        m_memberWriteRequestId = 0;
        if ( EOSRESULT_SUCCESS != result )
        {
            if ( STAGE_STARTING == m_stage )
            {
                m_service->OnLobbyStarted( this, false, CORE::CString( "The name of the player could not be set: " ) + EosResultToString( result ) );
                return;
            }
            if ( m_requestInFlight )
            {
                m_requestInFlight = false;
                Refuse( m_inFlight.what, CORE::CString( "The request could not be sent: " ) + EosResultToString( result ) );
            }
        }
        return;
    }
    if ( requestId == m_promoteRequestId && 0 != requestId )
    {
        m_promoteRequestId = 0;
        if ( EOSRESULT_SUCCESS != result && m_isOwner )
        {
            // It stays as it was
            m_promoteTarget.Clear();
            m_rules.ApplyTransferBoss( m_state, m_localUser );
            HostCommit( "boss", m_localUser, "transferBoss" );
            Refuse( "transferBoss", CORE::CString( "EOS did not hand the lobby over: " ) + EosResultToString( result ) );
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::IsInEos( const SEosLobbyDetails& details, const CORE::CString& userId )
{GUCEF_TRACE;

    for ( size_t i=0; i<details.members.size(); ++i )
        if ( details.members[ i ].puid == userId )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::FilterMembers( const SEosLobbyDetails& details, MULTIPLAY::CLobbyState& state ) const
{GUCEF_TRACE;

    // Whoever EOS no longer has in the lobby is not a member, whatever the owner last published
    for ( size_t i=0; i<state.members.size(); )
    {
        if ( !IsInEos( details, state.members[ i ].id ) )
        {
            MULTIPLAY::CLobbySlot* slot = state.FindSlotOfMember( state.members[ i ].id );
            if ( GUCEF_NULL != slot )
            {
                slot->memberId.Clear();
                slot->kind = MULTIPLAY::LOBBYSLOTKIND_OPEN;
            }
            state.members.erase( state.members.begin() + i );
        }
        else
        {
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::ReadExtras( const CORE::CDataNode& extras )
{GUCEF_TRACE;

    // The owner's notes next to the state: the countdown, why a lobby closed, who is refused, what was acknowledged
    m_countdownFromOwner = extras.GetAttributeValueOrChildValueByName( "cd" ).AsInt32( -1, true );
    m_closeReason = CEosLobbyCodec::SanitizeText( extras.GetAttributeValueOrChildValueByName( "cr" ).AsString( CORE::CString::Empty, true ), 200 );

    m_lastEndedVote.Clear();
    m_lastEndedPassed = false;
    const CORE::CDataNode* voteEnd = extras.FindChild( "ve" );
    if ( GUCEF_NULL != voteEnd )
    {
        m_lastEndedVote.id = CEosLobbyCodec::SanitizeText( voteEnd->GetAttributeValueOrChildValueByName( "id" ).AsString( CORE::CString::Empty, true ), 64 );
        m_lastEndedVote.what = CEosLobbyCodec::SanitizeText( voteEnd->GetAttributeValueOrChildValueByName( "what" ).AsString( CORE::CString::Empty, true ), 16 );
        m_lastEndedVote.proposer = CEosLobbyCodec::SanitizeText( voteEnd->GetAttributeValueOrChildValueByName( "proposer" ).AsString( CORE::CString::Empty, true ), 64 );
        m_lastEndedVote.key = CEosLobbyCodec::SanitizeText( voteEnd->GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true ), 64 );
        m_lastEndedVote.value = CEosLobbyCodec::SanitizeText( voteEnd->GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true ), 200 );
        m_lastEndedVote.yes = voteEnd->GetAttributeValueOrChildValueByName( "yes" ).AsUInt32( 0, true );
        m_lastEndedVote.no = voteEnd->GetAttributeValueOrChildValueByName( "no" ).AsUInt32( 0, true );
        m_lastEndedPassed = voteEnd->GetAttributeValueOrChildValueByName( "passed" ).AsBool( false, true );
    }

    // What is said about the local user only
    const CORE::CDataNode* acks = extras.FindChild( "ak" );
    if ( GUCEF_NULL != acks )
    {
        UInt32 acked = acks->GetAttributeValueOrChildValueByName( m_localUser ).AsUInt32( 0, true );
        if ( 0 != acked )
            m_pendingAck = acked;
    }
    const CORE::CDataNode* refusals = extras.FindChild( "rf" );
    if ( GUCEF_NULL != refusals )
    {
        CORE::CString text = refusals->GetAttributeValueOrChildValueByName( m_localUser ).AsString( CORE::CString::Empty, true );
        if ( !text.IsNULLOrEmpty() )
            m_pendingRefusal = text;
    }
    const CORE::CDataNode* rejections = extras.FindChild( "rj" );
    if ( GUCEF_NULL != rejections )
    {
        CORE::CString text = rejections->GetAttributeValueOrChildValueByName( m_localUser ).AsString( CORE::CString::Empty, true );
        if ( !text.IsNULLOrEmpty() )
            m_rejection = CEosLobbyCodec::SanitizeText( text, 200 );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::AdoptState( const MULTIPLAY::CLobbyState& newState )
{GUCEF_TRACE;

    MULTIPLAY::CLobbyState old = m_state;
    bool first = old.lobbyId.IsNULLOrEmpty();
    m_state = newState;

    // Members that came and went; the first look at a lobby is not a stream of joins
    if ( !first )
    {
        for ( size_t i=0; i<newState.members.size(); ++i )
        {
            if ( GUCEF_NULL == old.FindMember( newState.members[ i ].id ) )
            {
                CORE::TCloneableString joined( newState.members[ i ].id );
                if ( !NotifyObservers( MemberJoinedEvent, &joined ) )
                    return;
            }
        }
        for ( size_t i=0; i<old.members.size(); ++i )
        {
            if ( GUCEF_NULL == newState.FindMember( old.members[ i ].id ) )
            {
                CORE::TCloneableString left( old.members[ i ].id );
                if ( !NotifyObservers( MemberLeftEvent, &left ) )
                    return;
            }
        }
    }

    if ( old.phase != newState.phase )
        StateChanged( "phase" );
    if ( old.bossId != newState.bossId )
        StateChanged( "boss" );
    if ( !MembersEqual( old.members, newState.members ) )
        StateChanged( "members" );
    if ( !SlotsEqual( old.slots, newState.slots ) )
        StateChanged( "slots" );
    CORE::CString::StringVector changed;
    CollectSettingChanges( old.settings, newState.settings, changed );
    for ( size_t i=0; i<changed.size(); ++i )
        StateChanged( "settings/" + changed[ i ] );
    if ( !VotesEqual( old.activeVote, newState.activeVote ) )
        StateChanged( "vote" );

    RaiseVoteEvents( old, newState );

    // The countdown: the seconds the owner counts, -1 when it stopped
    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN == newState.phase )
    {
        if ( m_countdownFromOwner >= 0 && m_countdownFromOwner != m_lastCountdownSeen )
        {
            m_lastCountdownSeen = m_countdownFromOwner;
            CORE::TCloneableInt32 left( m_countdownFromOwner );
            NotifyObservers( CountdownEvent, &left );
        }
    }
    else
    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN == old.phase && MULTIPLAY::LOBBYPHASE_OPEN == newState.phase )
    {
        m_lastCountdownSeen = -1;
        CORE::TCloneableInt32 cancelled( -1 );
        NotifyObservers( CountdownEvent, &cancelled );
    }
    else
    {
        m_lastCountdownSeen = -1;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::RaiseVoteEvents( const MULTIPLAY::CLobbyState& oldState, const MULTIPLAY::CLobbyState& newState )
{GUCEF_TRACE;

    // A vote that is over: how it ended is what the owner noted
    if ( oldState.activeVote.IsActive() && ( !newState.activeVote.IsActive() || newState.activeVote.id != oldState.activeVote.id ) )
    {
        if ( oldState.activeVote.id != m_lastVoteEndedSeen )
        {
            m_lastVoteEndedSeen = oldState.activeVote.id;
            bool passed = m_lastEndedVote.id == oldState.activeVote.id && m_lastEndedPassed;
            MULTIPLAY::CLobbyVoteEventData ended( oldState.activeVote, passed );
            NotifyObservers( VoteEndedEvent, &ended );
        }
    }
    if ( newState.activeVote.IsActive() && newState.activeVote.id != m_lastVoteStarted )
    {
        m_lastVoteStarted = newState.activeVote.id;
        MULTIPLAY::CLobbyVoteEventData started( newState.activeVote, false );
        NotifyObservers( VoteStartedEvent, &started );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service || STAGE_DETACHED == m_stage || m_closing )
        return;

    SEosLobbyDetails details;
    if ( !m_service->CopyDetails( m_lobbyId, details ) )
    {
        // EOS no longer has the lobby for the local user
        if ( STAGE_STARTING == m_stage )
            m_service->OnLobbyStarted( this, false, m_rejection.IsNULLOrEmpty() ? CORE::CString( "The lobby is gone" ) : m_rejection );
        else
            Detach( m_closeReason.IsNULLOrEmpty() ? CORE::CString( "The lobby is gone" ) : m_closeReason, true );
        return;
    }
    if ( !CEosLobbyCodec::IsValidUserId( details.ownerPuid ) )
        return;
    m_ownerUserId = details.ownerPuid;

    if ( details.ownerPuid == m_localUser )
    {
        if ( !m_isOwner && STAGE_ACTIVE == m_stage )
            Takeover( details );
        return;
    }
    if ( m_isOwner )
    {
        // The lobby was taken from the local user (the hand over of the boss)
        m_isOwner = false;
        m_updateInFlight = false;
        m_dirty = false;
        m_pendingJoins.clear();
        m_countdownLeft = -1;
        m_voteSecondsLeft = -1;
    }
    RefreshAsMember( details );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::RefreshAsMember( const SEosLobbyDetails& details )
{GUCEF_TRACE;

    const CEosLobbyLimits& limits = m_service->GetSettings().limits;
    CORE::CString reason;
    CEosLobbySummary summary;

    // The data of the lobby counts when the owner wrote it: stamped with its Product User Id
    if ( !CEosLobbyCodec::DecodeSummary( details.attributes, details.ownerPuid, summary, reason ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The data of a lobby is ignored: " << reason );
        return;
    }

    if ( !m_hasDefinition )
    {
        CORE::CDataNode definition;
        if ( !CEosLobbyCodec::DecodeBlob( CEosLobbyCodec::BlobDefinition, details.attributes, limits, definition, reason ) || !LoadDefinition( definition, reason ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The definition of a lobby is ignored: " << reason );
            return;
        }
        m_hasDefinition = true;
    }

    // The state, when the owner published a new one
    bool newRevision = !m_haveOwnerState || summary.revision != m_lastRevisionSeen;
    if ( newRevision )
    {
        CORE::CDataNode stateTree;
        if ( !CEosLobbyCodec::DecodeBlob( CEosLobbyCodec::BlobState, details.attributes, limits, stateTree, reason ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The state of a lobby is ignored: " << reason );
            return;
        }
        const CORE::CDataNode* stateNode = stateTree.FindChild( "state" );
        MULTIPLAY::CLobbyState decoded;
        if ( GUCEF_NULL == stateNode || !decoded.Deserialize( *stateNode, CORE::CDataNodeSerializableSettings() ) || decoded.lobbyId != m_lobbyId )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The state of a lobby is not the state of this lobby, it is ignored" );
            return;
        }
        if ( decoded.members.size() > limits.maxLobbyMembers || decoded.slots.size() > 256 )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The state of a lobby is larger than a lobby can be, it is ignored" );
            return;
        }
        m_ownerState = decoded;
        m_haveOwnerState = true;
        m_lastRevisionSeen = summary.revision;
        m_pendingAck = 0;
        m_pendingRefusal.Clear();
        ReadExtras( stateTree );
    }
    m_summary = summary;

    MULTIPLAY::CLobbyState candidate = m_ownerState;
    FilterMembers( details, candidate );

    // Joining ends when the owner has the local user in its lobby; being refused ends it too
    if ( STAGE_STARTING == m_stage )
    {
        if ( GUCEF_NULL != candidate.FindMember( m_localUser ) )
        {
            m_stage = STAGE_ACTIVE;
            AdoptState( candidate );
            m_service->OnLobbyStarted( this, true, CORE::CString::Empty );

            // A game that is under way is handed to the newcomer at once: nothing else is going to change in the lobby to prompt it
            if ( STAGE_DETACHED != m_stage && GUCEF_NULL != m_service )
            {
                CheckLaunch( details, summary );
                UpdateSockets( details );
            }
        }
        else
        if ( !m_rejection.IsNULLOrEmpty() )
        {
            m_service->OnLobbyStarted( this, false, m_rejection );
        }
        else
        {
            // Only the picture of the lobby, the member is not in it yet
            m_state = candidate;
        }
        return;
    }

    AdoptState( candidate );
    if ( STAGE_DETACHED == m_stage || GUCEF_NULL == m_service )
        return;

    // The acknowledgement and the refusal of what the local user asked
    if ( 0 != m_pendingRefusal.Length() )
    {
        CORE::CString text = m_pendingRefusal;
        m_pendingRefusal.Clear();
        OnRefusal( text );
    }
    if ( 0 != m_pendingAck )
    {
        UInt32 acked = m_pendingAck;
        m_pendingAck = 0;
        OnAck( acked );
    }

    if ( MULTIPLAY::LOBBYPHASE_CLOSED == candidate.phase )
    {
        Detach( m_closeReason.IsNULLOrEmpty() ? CORE::CString( "The lobby was closed" ) : m_closeReason, true );
        m_service->RequestLeave( m_lobbyId );
        return;
    }
    if ( GUCEF_NULL == candidate.FindMember( m_localUser ) )
    {
        // The owner took the local user out of the lobby: EOS tells it with a kick
        return;
    }

    CheckLaunch( details, summary );
    UpdateSockets( details );
    PumpRequests( PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::LoadDefinition( const CORE::CDataNode& definition, CORE::CString& reason )
{GUCEF_TRACE;

    const CORE::CDataNode* paramsNode = definition.FindChild( "params" );
    const CORE::CDataNode* rulesNode = definition.FindChild( "rules" );
    if ( GUCEF_NULL == paramsNode || GUCEF_NULL == rulesNode )
    {
        reason = "The definition has no parameters or rules";
        return false;
    }
    MULTIPLAY::CLobbyCreateParams params;
    MULTIPLAY::CLobbyRules rules;
    if ( !params.LoadConfig( *paramsNode ) || !rules.LoadConfig( *rulesNode ) )
    {
        reason = "The definition is not well formed";
        return false;
    }
    // Nothing from another member is a password: the definition has none
    params.password.Clear();
    if ( params.schema.settings.size() > 256 || params.slots.size() > 256 )
    {
        reason = "The definition is larger than a lobby can be";
        return false;
    }
    m_params = params;
    m_rules = rules;
    m_schema = params.schema;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnLobbyEvent( const SEosLobbyEvent& event )
{GUCEF_TRACE;

    if ( STAGE_DETACHED == m_stage )
        return;

    switch ( event.kind )
    {
        case EOSLOBBYEVENT_LOBBY_UPDATED :
        {
            if ( m_isOwner )
            {
                // The echo of the own updates and the changes of the lobby that others made (nobody else can change the lobby)
                return;
            }
            Refresh();
            break;
        }
        case EOSLOBBYEVENT_MEMBER_UPDATED :
        {
            if ( m_isOwner )
                HostOnMemberUpdated( event.userId );
            else
                Refresh();
            break;
        }
        case EOSLOBBYEVENT_MEMBER_STATUS :
        {
            OnMemberStatus( event );
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnMemberStatus( const SEosLobbyEvent& event )
{GUCEF_TRACE;

    bool isLocal = event.userId == m_localUser;
    switch ( event.status )
    {
        case EOSMEMBERSTATUS_JOINED :
        {
            if ( m_isOwner && !isLocal )
                HostMemberJoined( event.userId );
            else
            if ( !m_isOwner )
                Refresh();
            break;
        }
        case EOSMEMBERSTATUS_LEFT :
        case EOSMEMBERSTATUS_DISCONNECTED :
        {
            if ( isLocal )
                break;
            if ( m_isOwner )
                HostMemberGone( event.userId );
            else
                Refresh();
            break;
        }
        case EOSMEMBERSTATUS_KICKED :
        {
            if ( isLocal )
            {
                OnRemoved( m_rejection.IsNULLOrEmpty() ? CORE::CString( "You were kicked" ) : m_rejection );
                break;
            }
            if ( m_isOwner )
                HostMemberGone( event.userId );
            else
                Refresh();
            break;
        }
        case EOSMEMBERSTATUS_PROMOTED :
        {
            // The owner changed; Refresh() sees who it is
            Refresh();
            break;
        }
        case EOSMEMBERSTATUS_CLOSED :
        {
            if ( isLocal || !m_isOwner )
                OnRemoved( m_closeReason.IsNULLOrEmpty() ? CORE::CString( "The lobby was closed" ) : m_closeReason );
            break;
        }
        default :
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnRemoved( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( STAGE_DETACHED == m_stage || m_closing )
        return;
    if ( STAGE_STARTING == m_stage )
    {
        m_service->OnLobbyStarted( this, false, reason );
        return;
    }
    Detach( reason, true );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnTick( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( STAGE_DETACHED == m_stage && !m_closing )
        return;

    if ( m_isOwner )
        HostTick( nowInMs );
    else
        PumpRequests( nowInMs );

    // A member that waits for its token asks the owner again
    if ( m_launchWanted && !m_launchRaised && m_token.IsNULLOrEmpty() )
        RequestToken( nowInMs );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::StartAsMember( const SEosLobbyDetails& details, const CORE::CString& password, bool asSpectator, CORE::CString& reason )
{GUCEF_TRACE;

    const CEosLobbyServiceSettings& settings = m_service->GetSettings();
    CEosLobbySummary summary;
    if ( !CEosLobbyCodec::DecodeSummary( details.attributes, details.ownerPuid, summary, reason ) )
        return false;
    if ( summary.passworded && password.IsNULLOrEmpty() )
    {
        reason = "The lobby needs a password";
        return false;
    }
    if ( summary.phase == MULTIPLAY::LOBBYPHASE_CLOSED )
    {
        reason = "The lobby is closed";
        return false;
    }

    CORE::CDataNode definition;
    if ( !CEosLobbyCodec::DecodeBlob( CEosLobbyCodec::BlobDefinition, details.attributes, settings.limits, definition, reason ) || !LoadDefinition( definition, reason ) )
        return false;
    m_hasDefinition = true;
    m_ownerUserId = details.ownerPuid;
    m_summary = summary;
    m_joinAsSpectator = asSpectator;
    m_joinPassword = password;

    // The first picture of the lobby; the local user is not in it until the owner accepts it
    CORE::CDataNode stateTree;
    CORE::CString stateReason;
    if ( CEosLobbyCodec::DecodeBlob( CEosLobbyCodec::BlobState, details.attributes, settings.limits, stateTree, stateReason ) )
    {
        const CORE::CDataNode* stateNode = stateTree.FindChild( "state" );
        MULTIPLAY::CLobbyState decoded;
        if ( GUCEF_NULL != stateNode && decoded.Deserialize( *stateNode, CORE::CDataNodeSerializableSettings() ) && decoded.lobbyId == m_lobbyId )
        {
            m_ownerState = decoded;
            m_haveOwnerState = true;
            m_lastRevisionSeen = summary.revision;
            ReadExtras( stateTree );
            m_state = m_ownerState;
        }
    }

    // Who the member is, and what it knows of the password: in attributes of its own
    CORE::CString name = CEosLobbyCodec::SanitizeText( m_service->GetIdentity().displayName, 64 );
    if ( name.IsNULLOrEmpty() )
        name = "Player";
    TEosAttributeVector attributes;
    attributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyName, name ) );
    if ( summary.passworded )
        attributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyPasswordProof, CEosLobbyCodec::MakePasswordProof( summary.passwordSalt, m_localUser, password ) ) );
    if ( asSpectator )
        attributes.push_back( CEosLobbyCodec::MakeInt( CEosLobbyCodec::MemberKeySpectator, 1 ) );
    if ( !WriteMemberAttributes( attributes ) )
    {
        reason = "The name of the player could not be sent";
        return false;
    }
    m_stage = STAGE_STARTING;
    m_joinDeadlineInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() + settings.joinTimeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE LAUNCH, MEMBER SIDE                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CEosLobby::CheckLaunch( const SEosLobbyDetails& details, const CEosLobbySummary& summary )
{GUCEF_TRACE;

    if ( m_launchRaised || m_isOwner )
        return;
    if ( summary.phase != MULTIPLAY::LOBBYPHASE_LAUNCHING && summary.phase != MULTIPLAY::LOBBYPHASE_IN_GAME )
        return;
    if ( m_launchWanted )
        return;

    // What the owner published about the game: only the owner's data counts (the stamp was checked with the summary)
    CORE::CString reason;
    CORE::CDataNode tree;
    if ( !CEosLobbyCodec::DecodeBlob( CEosLobbyCodec::BlobLaunch, details.attributes, m_service->GetSettings().limits, tree, reason ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The launch data of a lobby is ignored: " << reason );
        return;
    }
    MULTIPLAY::CLaunchDescription launch;
    if ( !launch.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) || launch.lobbyId != m_lobbyId || launch.members.size() > 256 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobby: The launch data of a lobby is not valid, it is ignored" );
        return;
    }
    // The launch is of the lobby it came from: the members' endpoints name the sockets of this lobby only
    launch.token.Clear();
    m_launchTemplate = launch;
    m_launchWanted = true;
    m_tokenDeadlineInMs = PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() + m_service->GetSettings().launchTimeoutInMs;
    m_nextTokenRequestInMs = 0;

    CORE::CString ignore;
    OpenControlSocket();
    RequestToken( PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::OpenControlSocket( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controlSocket )
        return true;
    if ( GUCEF_NULL == m_service )
        return false;
    CEosP2PRouter* router = m_service->GetRouter();
    if ( GUCEF_NULL == router )
        return false;

    CEosP2PSocket* socket = router->AcquireSocket( CEosLobbyCodec::MakeSocketName( "gc", m_lobbyId ) );
    if ( GUCEF_NULL == socket )
        return false;
    m_router = router;
    m_controlSocket = socket;
    m_controlSocket->SetMaxConnections( m_service->GetSettings().limits.maxLobbyMembers );
    m_controlSocket->SetAcceptPolicy( CEosP2PSocket::ACCEPT_LIST );
    SubscribeTo( m_controlSocket, CEosP2PSocket::PacketReceivedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::UpdateSockets( const SEosLobbyDetails& details )
{GUCEF_TRACE;

    // Who may reach whom on the sockets of the game: the members of the lobby, and nobody else
    if ( GUCEF_NULL != m_controlSocket )
    {
        CORE::CString::StringSet allowed;
        if ( m_isOwner )
        {
            for ( size_t i=0; i<m_state.members.size(); ++i )
                allowed.insert( m_state.members[ i ].id );
        }
        else
        {
            allowed.insert( details.ownerPuid );
        }
        allowed.erase( m_localUser );
        m_controlSocket->SetAllowedRemotes( allowed );
    }
    if ( GUCEF_NULL != m_gameSocket )
    {
        CORE::CString::StringSet allowed;
        for ( size_t i=0; i<m_state.members.size(); ++i )
            allowed.insert( m_state.members[ i ].id );
        allowed.erase( m_localUser );
        m_gameSocket->SetAllowedRemotes( allowed );
    }

    // In a game of peers every member listens, so every member lets the others of the lobby in
    if ( !m_isOwner && m_launchWanted && GUCEF_NULL == m_gameSocket && MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == m_launchTemplate.topology )
        OpenGameSocket();
}

/*-------------------------------------------------------------------------*/

bool
CEosLobby::OpenGameSocket( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_gameSocket )
        return true;
    if ( GUCEF_NULL == m_service )
        return false;
    CEosP2PRouter* router = m_service->GetRouter();
    if ( GUCEF_NULL == router )
        return false;
    CEosP2PSocket* socket = router->AcquireSocket( CEosLobbyCodec::MakeSocketName( "gg", m_lobbyId ) );
    if ( GUCEF_NULL == socket )
        return false;
    m_router = router;
    m_gameSocket = socket;
    m_gameSocket->SetMaxConnections( m_service->GetSettings().limits.maxLobbyMembers );
    m_gameSocket->SetAcceptPolicy( CEosP2PSocket::ACCEPT_LIST );
    CORE::CString::StringSet allowed;
    for ( size_t i=0; i<m_state.members.size(); ++i )
        allowed.insert( m_state.members[ i ].id );
    allowed.erase( m_localUser );
    m_gameSocket->SetAllowedRemotes( allowed );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::RequestToken( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( !m_launchWanted || m_launchRaised || !m_token.IsNULLOrEmpty() )
        return;

    if ( 0 != m_tokenDeadlineInMs && nowInMs >= m_tokenDeadlineInMs )
    {
        m_launchWanted = false;
        Refuse( "launch", "The host did not hand over the token of the game" );
        return;
    }
    if ( nowInMs < m_nextTokenRequestInMs )
        return;
    m_nextTokenRequestInMs = nowInMs + 500;

    if ( GUCEF_NULL == m_controlSocket && !OpenControlSocket() )
        return;
    static const char request[] = { 'G', 'T', 'Q', '1' };
    m_controlSocket->SetAcceptPolicy( CEosP2PSocket::ACCEPT_LIST );
    m_controlSocket->AllowRemote( m_ownerUserId );
    m_controlSocket->SendPacket( m_ownerUserId, request, sizeof( request ), EOSRELIABILITY_RELIABLE_ORDERED );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnControlPacket( const CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    if ( packet.size < 4 )
        return;

    if ( m_isOwner )
    {
        // A member asks for its token: only a member that the owner accepted gets one
        if ( 'G' == packet.data[ 0 ] && 'T' == packet.data[ 1 ] && 'Q' == packet.data[ 2 ] && '1' == packet.data[ 3 ] && 4 == packet.size )
        {
            CORE::CString userId( packet.remoteUser );
            HostSendTokenTo( userId );
        }
        return;
    }

    // The token comes from the owner and from nobody else
    if ( 0 != strcmp( packet.remoteUser, m_ownerUserId.C_String() ) )
        return;
    if ( 'G' != packet.data[ 0 ] || 'T' != packet.data[ 1 ] || 'K' != packet.data[ 2 ] || '1' != packet.data[ 3 ] )
        return;
    if ( packet.size < 8 || packet.size > 4 + 1000 || !m_token.IsNULLOrEmpty() )
        return;

    // A token is printable text
    GUCEF::vector< char > text;
    text.reserve( packet.size );
    for ( UInt32 i=4; i<packet.size; ++i )
    {
        char c = (char) packet.data[ i ];
        if ( c < 0x21 || c > 0x7e )
            return;
        text.push_back( c );
    }
    text.push_back( '\0' );
    m_token = CORE::CString( &text[ 0 ] );
    RaiseLaunch();
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::RaiseLaunch( void )
{GUCEF_TRACE;

    if ( m_launchRaised || !m_launchWanted || m_token.IsNULLOrEmpty() )
        return;

    MULTIPLAY::CLaunchDescription launch( m_launchTemplate );
    launch.localMemberId = m_localUser;
    launch.token = m_token;
    const MULTIPLAY::CLobbyMember* member = m_state.FindMember( m_localUser );
    launch.spectator = GUCEF_NULL != member && member->IsSpectator();
    m_launch = launch;
    m_launchRaised = true;
    m_launchWanted = false;
    NotifyObservers( LaunchEvent, &launch );
}

/*-------------------------------------------------------------------------*/

void
CEosLobby::OnNotify( CORE::CNotifier* notifier                 ,
                     const CORE::CEvent& eventId               ,
                     CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == m_controlSocket && CEosP2PSocket::PacketReceivedEvent == eventId && GUCEF_NULL != eventData )
        OnControlPacket( *static_cast< const CEosP2PPacketEventData* >( eventData ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
