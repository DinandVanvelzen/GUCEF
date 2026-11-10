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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#include "pluginglueEOS_CEosPlatformLease.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H
#include "lobbyservicepluginEOS_CEosLobbySdk.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H
#include "lobbyservicepluginEOS_CEosLobby.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBY_H ? */

#include "lobbyservicepluginEOS_CEosLobbyService.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEosLobbyService::ServiceName = "eos";
const CORE::CEvent CEosLobbyService::InviteReceivedEvent = "GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyService::InviteReceivedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosInviteEventData::CEosInviteEventData( void )
    : CORE::CICloneable()
    , lobbyId()
    , senderUserId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosInviteEventData::CEosInviteEventData( const CORE::CString& lobby, const CORE::CString& sender )
    : CORE::CICloneable()
    , lobbyId( lobby )
    , senderUserId( sender )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEosInviteEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEosInviteEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::RegisterEvents( void )
{GUCEF_TRACE;

    InviteReceivedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CEosLobbyService::CEosLobbyService( void )
    : MULTIPLAY::CILobbyService()
    , m_settings()
    , m_lobbyShim()
    , m_p2pShim()
    , m_haveShim( false )
    , m_haveP2PShim( false )
    , m_platform( GUCEF_NULL )
    , m_tickDemand( 0 )
    , m_identity()
    , m_localUser()
    , m_connected( false )
    , m_nextRequestId( 0 )
    , m_ops()
    , m_lobbies()
    , m_browseSettingFilters()
    , m_browseBucket()
    , m_retiredLobbies()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEosLobbyService::~CEosLobbyService()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    SignalUpcomingObserverDestruction();
    Disconnect();

    // The lobbies that somebody still holds go on without a service
    TLobbyMap lobbies = m_lobbies;
    for ( TLobbyMap::iterator i = lobbies.begin(); i != lobbies.end(); ++i )
        (*i).second->OnServiceGone();
    m_lobbies.clear();
    m_retiredLobbies.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobbyService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CLobbyServiceCapabilities
CEosLobbyService::GetCapabilities( void ) const
{GUCEF_TRACE;

    MULTIPLAY::CLobbyServiceCapabilities capabilities;
    capabilities.voting = true;
    capabilities.relay = true;
    capabilities.inProgressSpectating = true;
    capabilities.matchmaking = false;
    capabilities.chat = false;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

const CEosLobbyServiceSettings&
CEosLobbyService::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::SetSettings( const CEosLobbyServiceSettings& settings )
{GUCEF_TRACE;

    if ( !settings.limits.IsWellFormed() )
        return false;
    m_settings = settings;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::SetShim( const SEosLobbyShim& lobbyShim, const SEosP2PShim* p2pShim )
{GUCEF_TRACE;

    m_lobbyShim = lobbyShim;
    m_haveShim = true;
    m_haveP2PShim = GUCEF_NULL != p2pShim && p2pShim->IsUsable();
    m_p2pShim = m_haveP2PShim ? *p2pShim : SEosP2PShim();
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::SetBrowseSettingFilter( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    m_browseSettingFilters[ key ] = value;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::ClearBrowseSettingFilters( void )
{GUCEF_TRACE;

    m_browseSettingFilters.clear();
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::SetBrowseBucket( const CORE::CString& bucketId )
{GUCEF_TRACE;

    m_browseBucket = bucketId;
}

/*-------------------------------------------------------------------------*/

CEosPlatform*
CEosLobbyService::GetPlatform( void ) const
{GUCEF_TRACE;

    return m_platform;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobbyService::GetLocalUser( void ) const
{GUCEF_TRACE;

    return m_localUser;
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CPlayerIdentity&
CEosLobbyService::GetIdentity( void ) const
{GUCEF_TRACE;

    return m_identity;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::GetPendingRequestCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_ops.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::GetLobbyCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_lobbies.size();
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

CEosP2PRouter*
CEosLobbyService::GetRouter( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_platform ? GUCEF_NULL : m_platform->GetP2PRouter();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosLobbyService::DescribeResult( EEosResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case EOSRESULT_NO_CONNECTION :  return "There is no network connection to Epic Online Services";
        case EOSRESULT_TIMEOUT :        return "Epic Online Services did not answer in time";
        case EOSRESULT_NOT_FOUND :      return "The lobby was not found";
        case EOSRESULT_NO_PERMISSION :  return "Epic Online Services refused: no permission";
        case EOSRESULT_LOBBY_FULL :     return "The lobby is full";
        case EOSRESULT_LIMIT_EXCEEDED : return "A limit of Epic Online Services lobbies was exceeded";
        case EOSRESULT_NOT_LOGGED_IN :  return "Not logged in at Epic Online Services";
        case EOSRESULT_INVALID_PARAMETERS : return "Epic Online Services refused the request: invalid parameters";
        case EOSRESULT_ALREADY_PENDING : return "Epic Online Services is busy with the same request";
        default :                       return CORE::CString( "Epic Online Services failed: " ) + EosResultToString( result );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::ResolveShim( CORE::CString& reason )
{GUCEF_TRACE;

    if ( m_haveShim )
    {
        if ( !m_lobbyShim.IsUsable() )
        {
            reason = "The table of functions for the EOS lobby interface is incomplete";
            return false;
        }
        return true;
    }

    SEosLobbyShim real;
    if ( !CEosLobbySdk::FillShim( real ) )
    {
        reason = "The Epic Online Services SDK is not part of this build";
        return false;
    }
    m_lobbyShim = real;
    m_haveShim = true;
    SEosP2PShim p2p;
    if ( CEosGlue::Instance()->GetDefaultP2PShim( p2p ) )
    {
        m_p2pShim = p2p;
        m_haveP2PShim = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::FailConnect( CORE::CFutureResultSource& source, const CORE::CString& reason )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: Cannot connect: " << reason );
    source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::Connect( const MULTIPLAY::CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();

    if ( m_connected )
        Disconnect();

    CORE::CString reason;
    if ( !ResolveShim( reason ) )
        return FailConnect( source, reason );

    const SEosPlatformShim* platformShim = m_lobbyShim.platform.CanCreatePlatform() ? &m_lobbyShim.platform : GUCEF_NULL;
    CEosPlatformLease platformLease( platformShim, GUCEF_NULL, reason );
    CEosPlatform* platform = platformLease.Get();
    if ( GUCEF_NULL == platform )
    {
        if ( reason.HasSubstr( "not part of this build" ) >= 0 )
            return FailConnect( source, reason );
        return FailConnect( source, "The Epic Online Services platform is not initialised: " + reason );
    }

    // Who the player is: the account that is logged in at Epic and its product user, which the auth plugin made known
    CEosLoginState login = platform->GetLoginState();
    if ( !login.IsLoggedIn() )
        return FailConnect( source, "Not logged in at Epic Online Services: log in with the epic provider first" );
    CORE::CString claimedAccount = identity.accounts.GetValueAlways( "epic" ).AsString( CORE::CString::Empty, true );
    if ( !claimedAccount.IsNULLOrEmpty() && !login.epicAccountId.IsNULLOrEmpty() && claimedAccount != login.epicAccountId )
        return FailConnect( source, "The identity is of another Epic account than the one that is logged in" );

    if ( m_haveP2PShim )
        platform->AttachP2P( m_p2pShim );
    CEosP2PRouter* router = platform->GetP2PRouter();
    if ( GUCEF_NULL != router )
        router->SetRelayControl( m_settings.relayControl );

    m_platform = platformLease.Keep();
    m_localUser = login.productUserId;
    m_identity = identity;
    m_identity.linkProofs.clear();
    m_identity.accounts.SetAllowMultipleValues( false );
    m_identity.accounts.Set( "epic", login.epicAccountId );
    m_identity.accounts.Set( "eos", login.productUserId );
    TEventCallback tickCallback( this, &CEosLobbyService::OnPlatformTick );
    TEventCallback loginStateCallback( this, &CEosLobbyService::OnPlatformLoginStateChanged );
    SubscribeTo( m_platform, CEosPlatform::TickEvent, tickCallback );
    SubscribeTo( m_platform, CEosPlatform::LoginStateChangedEvent, loginStateCallback );
    m_tickDemand = m_platform->AddTickDemand( m_settings.tickIntervalInMs );
    m_connected = true;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: Connected as a product user of Epic Online Services" );
    source.Complete( m_identity.Clone(), true );
    NotifyObservers( ServiceConnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::ReleasePlatform( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_platform )
        return;
    UnsubscribeAllFromObserver();
    if ( 0 != m_tickDemand )
        m_platform->RemoveTickDemand( m_tickDemand );
    m_tickDemand = 0;
    CEosGlue::Instance()->ReleasePlatform( m_platform );
    m_platform = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::Disconnect( void )
{GUCEF_TRACE;

    if ( !m_connected && m_ops.empty() && m_lobbies.empty() )
        return;
    bool wasConnected = m_connected;

    // Everything that waits ends: the futures, and the lobbies that were being made
    TOpMap ops = m_ops;
    for ( TOpMap::iterator i = ops.begin(); i != ops.end(); ++i )
    {
        SOp& op = (*i).second;
        if ( OP_CREATE == op.kind || OP_JOIN == op.kind || OP_BROWSE == op.kind )
            AbandonOp( op.requestId, false, "The lobby service was disconnected" );
    }

    // A join that the SDK has not answered yet is undone as far as it is known: the lobby of it is left. A lobby that is
    // being made has no id until the SDK answers, so one that is made after this is closed by EOS when the platform goes
    for ( TOpMap::iterator i = m_ops.begin(); i != m_ops.end(); ++i )
    {
        SOp& op = (*i).second;
        if ( OP_JOIN == op.kind && op.abandoned && !op.sdkAnswered && m_connected && !op.lobbyId.IsNULLOrEmpty() && m_lobbyShim.leaveLobby )
            m_lobbyShim.leaveLobby( m_lobbyShim.context, NextRequestId(), op.lobbyId.C_String(), m_localUser.C_String() );
    }

    // An application that hears of a lobby closing may let go of others: every lobby is looked up again before it is used
    CORE::CString::StringVector lobbyIds;
    for ( TLobbyMap::iterator i = m_lobbies.begin(); i != m_lobbies.end(); ++i )
        lobbyIds.push_back( (*i).first );
    for ( size_t i=0; i<lobbyIds.size(); ++i )
    {
        CEosLobby* lobby = FindLobby( lobbyIds[ i ] );
        if ( GUCEF_NULL != lobby )
            lobby->OnServiceDisconnected( "The lobby service was disconnected" );
    }
    m_ops.clear();
    m_connected = false;
    ReleasePlatform();
    m_localUser.Clear();
    m_retiredLobbies.clear();
    if ( wasConnected )
        NotifyObservers( ServiceDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::Register( CEosLobby* lobby )
{GUCEF_TRACE;

    m_lobbies[ lobby->GetLobbyId() ] = lobby;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::Unregister( CEosLobby* lobby )
{GUCEF_TRACE;

    TLobbyMap::iterator i = m_lobbies.find( lobby->GetLobbyId() );
    if ( i != m_lobbies.end() && (*i).second == lobby )
        m_lobbies.erase( i );
}

/*-------------------------------------------------------------------------*/

CEosLobby*
CEosLobbyService::FindLobby( const CORE::CString& lobbyId ) const
{GUCEF_TRACE;

    TLobbyMap::const_iterator i = m_lobbies.find( lobbyId );
    return i == m_lobbies.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::NextRequestId( void )
{GUCEF_TRACE;

    ++m_nextRequestId;
    if ( 0 == m_nextRequestId )
        ++m_nextRequestId;
    return m_nextRequestId;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::AddOp( EOpKind kind, EEosLobbyEventKind expected, const CORE::CString& lobbyId, UInt32 requestId )
{GUCEF_TRACE;

    SOp& op = m_ops[ requestId ];
    op.requestId = requestId;
    op.kind = kind;
    op.expected = expected;
    op.lobbyId = lobbyId;
    op.deadlineInMs = CEosPlatform::GetMonotonicTimeInMs() + m_settings.requestTimeoutInMs;
    return requestId;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::CopyDetails( const CORE::CString& lobbyId, SEosLobbyDetails& details ) const
{GUCEF_TRACE;

    if ( !m_connected || GUCEF_NULL == m_lobbyShim.copyLobbyDetails )
        return false;
    details = SEosLobbyDetails();
    return m_lobbyShim.copyLobbyDetails( m_lobbyShim.context, lobbyId.C_String(), m_localUser.C_String(), &details );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestUpdate( const CORE::CString& lobbyId, const SEosLobbyUpdateOptions& options )
{GUCEF_TRACE;

    if ( !m_connected )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.updateLobby( m_lobbyShim.context, requestId, &options ) )
        return 0;
    return AddOp( OP_LOBBY, EOSLOBBYEVENT_UPDATE, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestLeave( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    if ( !m_connected )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.leaveLobby( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String() ) )
        return 0;
    return AddOp( OP_CLEANUP, EOSLOBBYEVENT_LEAVE, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestDestroy( const CORE::CString& lobbyId )
{GUCEF_TRACE;

    if ( !m_connected )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.destroyLobby( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String() ) )
        return 0;
    return AddOp( OP_CLEANUP, EOSLOBBYEVENT_DESTROY, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestKick( const CORE::CString& lobbyId, const CORE::CString& target )
{GUCEF_TRACE;

    if ( !m_connected )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.kickMember( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String(), target.C_String() ) )
        return 0;
    return AddOp( OP_CLEANUP, EOSLOBBYEVENT_KICK, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestPromote( const CORE::CString& lobbyId, const CORE::CString& target )
{GUCEF_TRACE;

    if ( !m_connected )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.promoteMember( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String(), target.C_String() ) )
        return 0;
    return AddOp( OP_LOBBY, EOSLOBBYEVENT_PROMOTE, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosLobbyService::RequestInvite( const CORE::CString& lobbyId, const CORE::CString& target )
{GUCEF_TRACE;

    if ( !m_connected || GUCEF_NULL == m_lobbyShim.sendInvite )
        return 0;
    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.sendInvite( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String(), target.C_String() ) )
        return 0;
    return AddOp( OP_CLEANUP, EOSLOBBYEVENT_SEND_INVITE, lobbyId, requestId );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::Cleanup( const CORE::CString& lobbyId, bool owner )
{GUCEF_TRACE;

    if ( owner )
        RequestDestroy( lobbyId );
    else
        RequestLeave( lobbyId );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::Browse( const MULTIPLAY::CGameListingFilter& filter, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }

    SEosLobbySearchOptions options;
    options.localUser = m_localUser;
    options.maxResults = m_settings.browseMaxResults < m_settings.limits.maxSearchResults ? m_settings.browseMaxResults : m_settings.limits.maxSearchResults;
    options.bucketId = !m_browseBucket.IsNULLOrEmpty() ? m_browseBucket : m_settings.GetBucketId( filter.gameId );
    options.minSlotsAvailable = filter.notFull ? 1 : -1;

    // Only lobbies of this wire format
    SEosLobbySearchParameter parameter;
    parameter.type = EOSATTRIBUTE_INT64;
    parameter.key = CEosLobbyCodec::KeyVersion;
    parameter.comparison = EOSCOMPARE_EQUAL;
    parameter.intValue = CEosLobbyCodec::WireVersion;
    options.parameters.push_back( parameter );

    if ( !filter.gameId.IsNULLOrEmpty() )
    {
        SEosLobbySearchParameter game;
        game.key = CEosLobbyCodec::KeyGameId;
        game.type = EOSATTRIBUTE_STRING;
        game.stringValue = filter.gameId;
        options.parameters.push_back( game );
    }
    if ( !filter.gameVersion.IsNULLOrEmpty() )
    {
        SEosLobbySearchParameter version;
        version.key = CEosLobbyCodec::KeyGameVersion;
        version.type = EOSATTRIBUTE_STRING;
        version.stringValue = filter.gameVersion;
        options.parameters.push_back( version );
    }
    if ( filter.noPassword )
    {
        SEosLobbySearchParameter noPassword;
        noPassword.key = CEosLobbyCodec::KeyPassworded;
        noPassword.type = EOSATTRIBUTE_INT64;
        noPassword.intValue = 0;
        options.parameters.push_back( noPassword );
    }
    SEosLobbySearchParameter phase;
    phase.key = CEosLobbyCodec::KeyPhase;
    phase.type = EOSATTRIBUTE_INT64;
    if ( MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY == filter.kind )
    {
        phase.comparison = EOSCOMPARE_LESS_THAN_OR_EQUAL;
        phase.intValue = MULTIPLAY::LOBBYPHASE_LAUNCHING;
    }
    else
    if ( MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS == filter.kind )
    {
        phase.comparison = EOSCOMPARE_EQUAL;
        phase.intValue = MULTIPLAY::LOBBYPHASE_IN_GAME;
    }
    else
    {
        phase.comparison = EOSCOMPARE_LESS_THAN;
        phase.intValue = MULTIPLAY::LOBBYPHASE_CLOSED;
    }
    options.parameters.push_back( phase );

    // The settings that are searched on
    for ( CORE::CString::StringMap::const_iterator i = m_browseSettingFilters.begin(); i != m_browseSettingFilters.end(); ++i )
    {
        SEosLobbySearchParameter setting;
        setting.key = CEosLobbyCodec::MirrorPrefix + (*i).first;
        setting.type = EOSATTRIBUTE_STRING;
        setting.stringValue = (*i).second;
        options.parameters.push_back( setting );
    }

    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.searchLobbies( m_lobbyShim.context, requestId, &options ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The search could not be started" );
        return false;
    }
    AddOp( OP_BROWSE, EOSLOBBYEVENT_SEARCH, CORE::CString::Empty, requestId );
    SOp& op = m_ops[ requestId ];
    op.source = source;
    op.filter = filter;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::CreateLobby( const MULTIPLAY::CLobbyCreateParams& params, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }

    // What does not fit the attributes of a lobby is found out before there is a lobby
    CORE::CString reason;
    if ( !CEosLobby::Validate( *this, params, reason ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: The lobby cannot be made: " << reason );
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
        return false;
    }

    // Made hidden and empty: the data and the permission level it is meant to have follow in one update
    SEosLobbyCreateOptions options;
    options.localUser = m_localUser;
    options.maxMembers = CEosLobby::GetMaxMembers( m_settings, params );
    options.permission = EOSLOBBYPERM_INVITE_ONLY;
    options.presenceEnabled = m_settings.presenceEnabled;
    options.allowInvites = true;
    options.bucketId = m_settings.GetBucketId( params.gameId );
    // The successor of an owner would not know the password
    options.disableHostMigration = !params.password.IsNULLOrEmpty();
    options.enableJoinById = true;
    options.rejoinAfterKickRequiresInvite = true;

    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.createLobby( m_lobbyShim.context, requestId, &options ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby could not be made" );
        return false;
    }
    AddOp( OP_CREATE, EOSLOBBYEVENT_CREATE, CORE::CString::Empty, requestId );
    SOp& op = m_ops[ requestId ];
    op.source = source;
    op.params = params;
    op.password = params.password;
    op.params.password.Clear();
    op.deadlineInMs += m_settings.joinTimeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::JoinLobby( const CORE::CString& lobbyId    ,
                             const CORE::CString& password   ,
                             bool asSpectator                ,
                             CORE::CFutureResult& future     )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( !m_connected )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby service is not connected" );
        return false;
    }
    if ( !CEosLobbyCodec::IsValidLobbyId( lobbyId ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "That is not the id of a lobby" );
        return false;
    }
    if ( GUCEF_NULL != FindLobby( lobbyId ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The local user is in that lobby already" );
        return false;
    }

    UInt32 requestId = NextRequestId();
    if ( !m_lobbyShim.joinLobbyById( m_lobbyShim.context, requestId, lobbyId.C_String(), m_localUser.C_String(), m_settings.presenceEnabled ) )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The lobby could not be joined" );
        return false;
    }
    AddOp( OP_JOIN, EOSLOBBYEVENT_JOIN, lobbyId, requestId );
    SOp& op = m_ops[ requestId ];
    op.source = source;
    op.password = password;
    op.asSpectator = asSpectator;
    op.deadlineInMs += m_settings.joinTimeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::PublishListing( const MULTIPLAY::CGameListingEntry& listing )
{GUCEF_TRACE;

    // A lobby of the local user that is hosted here is listed by being visible; there is nothing else to publish
    CEosLobby* lobby = FindLobby( listing.listingId );
    return GUCEF_NULL != lobby && lobby->SetVisibility( true );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::WithdrawListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    CEosLobby* lobby = FindLobby( listingId );
    return GUCEF_NULL != lobby && lobby->SetVisibility( false );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::PollEvents( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_lobbyShim.pollEvent )
        return;
    SEosLobbyEvent event;
    UInt32 handled = 0;
    while ( handled < 256 && m_connected && m_lobbyShim.pollEvent( m_lobbyShim.context, &event ) )
    {
        ++handled;
        HandleEvent( event );
        event = SEosLobbyEvent();
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::HandleEvent( const SEosLobbyEvent& event )
{GUCEF_TRACE;

    switch ( event.kind )
    {
        case EOSLOBBYEVENT_LOBBY_UPDATED :
        case EOSLOBBYEVENT_MEMBER_UPDATED :
        case EOSLOBBYEVENT_MEMBER_STATUS :
        {
            CEosLobby* lobby = FindLobby( event.lobbyId );
            if ( GUCEF_NULL != lobby )
                lobby->OnLobbyEvent( event );
            break;
        }
        case EOSLOBBYEVENT_INVITE_RECEIVED :
        {
            if ( CEosLobbyCodec::IsValidLobbyId( event.lobbyId ) && CEosLobbyCodec::IsValidUserId( event.senderId ) )
            {
                CEosInviteEventData invite( event.lobbyId, event.senderId );
                NotifyObservers( InviteReceivedEvent, &invite );
            }
            break;
        }
        default :
            OnRequestEnd( event );
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnRequestEnd( const SEosLobbyEvent& event )
{GUCEF_TRACE;

    TOpMap::iterator found = m_ops.find( event.requestId );
    if ( found == m_ops.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobbyService: The end of a request that is not waited for any more" );
        return;
    }
    SOp op = (*found).second;
    if ( op.expected != event.kind )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobbyService: An answer that does not fit the request, it is ignored" );
        return;
    }

    switch ( op.kind )
    {
        case OP_BROWSE :
        {
            m_ops.erase( event.requestId );
            if ( !op.abandoned )
                OnBrowseEnd( op, event );
            break;
        }
        case OP_CREATE :
        {
            OnCreateEnd( (*found).second, event );
            break;
        }
        case OP_JOIN :
        {
            OnJoinEnd( (*found).second, event );
            break;
        }
        case OP_LOBBY :
        {
            m_ops.erase( event.requestId );
            CEosLobby* lobby = FindLobby( op.lobbyId );
            if ( GUCEF_NULL != lobby )
                lobby->OnRequestEnded( event.requestId, event.kind, event.result );
            break;
        }
        default :
        {
            // A leave, a destroy, a kick or an invite that nobody waits for
            if ( EOSRESULT_SUCCESS != event.result )
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobbyService: A request ended with: " << EosResultToString( event.result ) );
            m_ops.erase( event.requestId );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnBrowseEnd( SOp& op, const SEosLobbyEvent& event )
{GUCEF_TRACE;

    if ( EOSRESULT_SUCCESS != event.result )
    {
        op.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, DescribeResult( event.result ) );
        return;
    }

    MULTIPLAY::CGameListingList* list = GUCEF_NEW MULTIPLAY::CGameListingList();
    UInt32 dropped = 0;
    for ( size_t i=0; i<event.found.size() && list->listings.size() < m_settings.browseMaxResults; ++i )
    {
        MULTIPLAY::CGameListingEntry listing;
        CORE::CString reason;

        // Lobbies that do not hold up (not written by their owner, not well formed) are not listed
        if ( !CEosLobbyCodec::MakeListing( event.found[ i ], m_settings.limits, listing, reason ) )
        {
            ++dropped;
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosLobbyService: A lobby is not listed: " << reason );
            continue;
        }
        if ( !op.filter.Matches( listing ) )
            continue;

        // The settings that were asked for are verified against what the owner published
        bool matches = true;
        for ( CORE::CString::StringMap::const_iterator f = m_browseSettingFilters.begin(); matches && f != m_browseSettingFilters.end(); ++f )
        {
            CORE::CString shown = listing.extra.GetValueAlways( "set." + (*f).first ).AsString( CORE::CString::Empty, true );
            matches = shown == (*f).second;
        }
        if ( matches )
            list->listings.push_back( listing );
    }
    if ( dropped > 0 )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: " << dropped << " lobby(s) of the search are not listed" );
    op.source.Complete( list, true );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnCreateEnd( SOp& op, const SEosLobbyEvent& event )
{GUCEF_TRACE;

    UInt32 requestId = op.requestId;
    if ( op.abandoned )
    {
        // Made after nobody wanted it: it goes again
        if ( EOSRESULT_SUCCESS == event.result && !event.lobbyId.IsNULLOrEmpty() )
            Cleanup( event.lobbyId, true );
        m_ops.erase( requestId );
        return;
    }
    if ( EOSRESULT_SUCCESS != event.result || !CEosLobbyCodec::IsValidLobbyId( event.lobbyId ) )
    {
        EndOp( requestId, false, DescribeResult( EOSRESULT_SUCCESS == event.result ? EOSRESULT_FAILED : event.result ) );
        return;
    }

    op.lobbyId = event.lobbyId;
    op.sdkAnswered = true;
    CEosLobby* lobby = GUCEF_NEW CEosLobby( *this, event.lobbyId, m_localUser );
    op.lobby = MULTIPLAY::CILobbyPtr( lobby );
    CORE::CString reason;
    CORE::CString password = op.password;
    MULTIPLAY::CLobbyCreateParams params = op.params;
    op.password.Clear();
    if ( !lobby->StartAsOwner( params, password, reason ) )
        EndOp( requestId, false, reason );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnJoinEnd( SOp& op, const SEosLobbyEvent& event )
{GUCEF_TRACE;

    UInt32 requestId = op.requestId;
    if ( op.abandoned )
    {
        if ( EOSRESULT_SUCCESS == event.result )
            Cleanup( op.lobbyId, false );
        m_ops.erase( requestId );
        return;
    }
    if ( EOSRESULT_SUCCESS != event.result )
    {
        EndOp( requestId, false, DescribeResult( event.result ) );
        return;
    }

    SEosLobbyDetails details;
    if ( !CopyDetails( op.lobbyId, details ) || !CEosLobbyCodec::IsValidUserId( details.ownerPuid ) )
    {
        EndOp( requestId, false, "The lobby could not be read after joining it" );
        return;
    }

    op.sdkAnswered = true;
    op.deadlineInMs = CEosPlatform::GetMonotonicTimeInMs() + m_settings.joinTimeoutInMs;
    CEosLobby* lobby = GUCEF_NEW CEosLobby( *this, op.lobbyId, m_localUser );
    op.lobby = MULTIPLAY::CILobbyPtr( lobby );
    CORE::CString reason;
    CORE::CString password = op.password;
    op.password.Clear();
    if ( !lobby->StartAsMember( details, password, op.asSpectator, reason ) )
        EndOp( requestId, false, reason );
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnLobbyStarted( CEosLobby* lobby, bool succeeded, const CORE::CString& reason )
{GUCEF_TRACE;

    for ( TOpMap::iterator i = m_ops.begin(); i != m_ops.end(); ++i )
    {
        SOp& op = (*i).second;
        if ( ( OP_CREATE == op.kind || OP_JOIN == op.kind ) && op.lobby.GetPointerAlways() == lobby )
        {
            EndOp( op.requestId, succeeded, reason );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::EndOp( UInt32 requestId, bool succeeded, const CORE::CString& reason )
{GUCEF_TRACE;

    TOpMap::iterator found = m_ops.find( requestId );
    if ( found == m_ops.end() )
        return;
    SOp op = (*found).second;
    m_ops.erase( found );

    if ( succeeded )
    {
        op.source.Complete( GUCEF_NEW MULTIPLAY::CLobbyResultData( op.lobby ), true );
        MULTIPLAY::CLobbyResultData joined( op.lobby );
        NotifyObservers( LobbyJoinedEvent, &joined );
        return;
    }

    const CORE::CString action = OP_CREATE == op.kind ? "made" : "joined";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: A lobby could not be " << action << ": " << reason );
    op.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );

    // What was made or joined on the way goes again
    CEosLobby* lobby = static_cast< CEosLobby* >( op.lobby.GetPointerAlways() );
    if ( GUCEF_NULL != lobby )
    {
        bool owner = lobby->IsOwner() || OP_CREATE == op.kind;
        lobby->Detach( CORE::CString::Empty, false );
        Cleanup( op.lobbyId, owner );
        m_retiredLobbies.push_back( op.lobby );
    }
    else
    if ( op.sdkAnswered && !op.lobbyId.IsNULLOrEmpty() )
    {
        Cleanup( op.lobbyId, OP_CREATE == op.kind );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::AbandonOp( UInt32 requestId, bool cancelled, const CORE::CString& reason )
{GUCEF_TRACE;

    TOpMap::iterator found = m_ops.find( requestId );
    if ( found == m_ops.end() )
        return;
    SOp& op = (*found).second;
    if ( op.abandoned )
        return;

    if ( cancelled )
        op.source.Cancel();
    else
        op.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
    const CORE::CString endReason = cancelled ? CORE::CString( "cancelled" ) : reason;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: A request ended before the SDK answered: " << endReason );

    if ( OP_BROWSE == op.kind )
    {
        m_ops.erase( found );
        return;
    }

    // A lobby that is being made or joined: when it exists it goes again, else it goes when the SDK answers
    CEosLobby* lobby = static_cast< CEosLobby* >( op.lobby.GetPointerAlways() );
    if ( GUCEF_NULL != lobby )
    {
        bool owner = lobby->IsOwner() || OP_CREATE == op.kind;
        CORE::CString lobbyId = op.lobbyId;
        lobby->Detach( CORE::CString::Empty, false );
        m_retiredLobbies.push_back( op.lobby );
        m_ops.erase( found );
        Cleanup( lobbyId, owner );
        return;
    }
    op.abandoned = true;
    op.deadlineInMs = CEosPlatform::GetMonotonicTimeInMs() + 4 * m_settings.requestTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::CheckOps( UInt64 nowInMs )
{GUCEF_TRACE;

    GUCEF::vector< UInt32 > ids;
    for ( TOpMap::const_iterator i = m_ops.begin(); i != m_ops.end(); ++i )
        ids.push_back( (*i).first );

    for ( size_t n=0; n<ids.size(); ++n )
    {
        TOpMap::iterator found = m_ops.find( ids[ n ] );
        if ( found == m_ops.end() )
            continue;
        SOp& op = (*found).second;
        bool hasFuture = OP_CREATE == op.kind || OP_JOIN == op.kind || OP_BROWSE == op.kind;

        if ( op.abandoned )
        {
            // The SDK never answered: the record goes
            if ( nowInMs >= op.deadlineInMs )
                m_ops.erase( found );
            continue;
        }
        if ( hasFuture && op.source.IsCancellationRequested() )
        {
            AbandonOp( ids[ n ], true, CORE::CString::Empty );
            continue;
        }
        if ( nowInMs >= op.deadlineInMs )
        {
            if ( hasFuture )
            {
                AbandonOp( ids[ n ], false, "Epic Online Services did not answer in time" );
            }
            else
            {
                // The lobby learns that its request ended without an answer
                UInt32 requestId = op.requestId;
                CORE::CString lobbyId = op.lobbyId;
                EEosLobbyEventKind kind = op.expected;
                EOpKind opKind = op.kind;
                m_ops.erase( found );
                CEosLobby* lobby = FindLobby( lobbyId );
                if ( OP_LOBBY == opKind && GUCEF_NULL != lobby )
                    lobby->OnRequestEnded( requestId, kind, EOSRESULT_TIMEOUT );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnTick( void )
{GUCEF_TRACE;

    if ( !m_connected )
        return;

    PollEvents();
    UInt64 nowInMs = CEosPlatform::GetMonotonicTimeInMs();
    CheckOps( nowInMs );

    // The lobbies have their clocks
    GUCEF::vector< CEosLobby* > lobbies;
    for ( TLobbyMap::const_iterator i = m_lobbies.begin(); i != m_lobbies.end(); ++i )
        lobbies.push_back( (*i).second );
    for ( size_t i=0; i<lobbies.size(); ++i )
    {
        if ( GUCEF_NULL != FindLobby( lobbies[ i ]->GetLobbyId() ) )
            lobbies[ i ]->OnTick( nowInMs );
    }

    // A lobby of an operation that went wrong is let go of here, outside of its own events
    m_retiredLobbies.clear();
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnLoginStateChanged( void )
{GUCEF_TRACE;

    if ( !m_connected || GUCEF_NULL == m_platform )
        return;
    CEosLoginState state = m_platform->GetLoginState();
    if ( state.productUserId != m_localUser )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosLobbyService: The product user of the platform changed, the service is disconnected" );
        Disconnect();
    }
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnPlatformTick( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_platform && notifier == m_platform )
        OnTick();
}

/*-------------------------------------------------------------------------*/

void
CEosLobbyService::OnPlatformLoginStateChanged( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_platform && notifier == m_platform )
        OnLoginStateChanged();
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CEosLobbyService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosLobbyService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::EOS::CEosLobbyService";
    return classTypeName;
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
