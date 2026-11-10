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

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#include "gucefMULTIPLAY_CGucefLobby.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#include "gucefMULTIPLAY_CIClientIntegrity.h"
#define GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#endif /* GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefMULTIPLAY_CGucefLobbyService.h"

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

const CORE::CString CGucefLobbyService::ServiceName = "gucef";

const CORE::CEvent CGucefLobbyService::ReplyEvent = "GUCEF::MULTIPLAY::CGucefLobbyService::ReplyEvent";
const CORE::CEvent CGucefLobbyService::DirectMessageEvent = "GUCEF::MULTIPLAY::CGucefLobbyService::DirectMessageEvent";
const CORE::CEvent CGucefLobbyService::ChannelEntryEvent = "GUCEF::MULTIPLAY::CGucefLobbyService::ChannelEntryEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
JoinStrings( const CORE::CString::StringVector& strings )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<strings.size(); ++i )
    {
        if ( i > 0 )
            joined += ';';
        joined += strings[ i ];
    }
    return joined;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefDirectoryReply::CGucefDirectoryReply( void )
    : CORE::CICloneable()
    , requestId( 0 )
    , ok( false )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CGucefDirectoryReply::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CGucefDirectoryReply( *this );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::RegisterEvents( void )
{GUCEF_TRACE;

    ReplyEvent.Initialize();
    DirectMessageEvent.Initialize();
    ChannelEntryEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGucefLobbyService::CGucefLobbyService( void )
    : CILobbyService()
    , m_node()
    , m_directory( GUCEF_NULL )
    , m_listings( GUCEF_NULL )
    , m_requests( GUCEF_NULL )
    , m_serverEndpoints()
    , m_password()
    , m_integrity( GUCEF_NULL )
    , m_localEndpoints()
    , m_identity()
    , m_connectResult()
    , m_connectPending( false )
    , m_connected( false )
    , m_nextRequestId( 0 )
    , m_pendingRequests()
    , m_pendingLobbies()
    , m_retiredLobbies()
    , m_pendingBrowses()
    , m_pendingBrowseFilters()
{GUCEF_TRACE;

    // The authenticator of the directory and the lobbies is registered by the global systems of the module
    CMultiplayGlobal::Instance();

    RegisterEvents();
    CGucefLobby::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CGucefLobbyService::~CGucefLobbyService()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    FailPending( "The lobby service went away" );
    m_pendingLobbies.clear();
    m_retiredLobbies.clear();
    if ( GUCEF_NULL != m_directory && m_directory->IsActive() )
        m_directory->Leave();
    m_node.Stop();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SetServerEndpoints( const DRN::CDRNEndpointList& endpoints )
{GUCEF_TRACE;

    m_serverEndpoints = endpoints;
}

/*-------------------------------------------------------------------------*/

const DRN::CDRNEndpointList&
CGucefLobbyService::GetServerEndpoints( void ) const
{GUCEF_TRACE;

    return m_serverEndpoints;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SetPassword( const CORE::CString& password )
{GUCEF_TRACE;

    m_password = password;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SetClientIntegrity( CIClientIntegrity* integrity )
{GUCEF_TRACE;

    m_integrity = integrity;
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CGucefLobbyService::MakeIntegrityBlob( void )
{GUCEF_TRACE;

    CORE::CValueList report;
    if ( GUCEF_NULL == m_integrity || !m_integrity->GetIntegrityReport( report ) )
        return CORE::CVariant();
    CORE::CDataNode tree( "integrity" );
    tree.SetAttributesOfChildByName( "integrity", report );
    return CGucefLobbyProtocol::DataTreeToVariant( tree );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SetLocalEndpoints( const CORE::CString::StringVector& endpoints )
{GUCEF_TRACE;

    m_localEndpoints = endpoints;
}

/*-------------------------------------------------------------------------*/

const CPlayerIdentity&
CGucefLobbyService::GetIdentity( void ) const
{GUCEF_TRACE;

    return m_identity;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    m_connectResult = CORE::CFutureResultSource();
    future = m_connectResult.GetFuture();
    CPlayerIdentity keyed( identity );
    if ( !keyed.privateKey.IsNULLOrEmpty() && !keyed.SetPrivateKey( identity.privateKey ) )
    {
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The key of the identity cannot be used, is there an \"ed25519\" signature codec?" );
        return false;
    }
    if ( keyed.userId.IsNULLOrEmpty() || m_serverEndpoints.IsEmpty() )
    {
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "An identity and the endpoints of a server are needed" );
        return false;
    }
    if ( m_connected || m_connectPending )
        Disconnect();

    m_identity = keyed;
    DRN::CDRNNodeSettings settings;
    settings.serviceName = CGucefLobbyProtocol::ServiceName;
    settings.echoIntervalMs = 100;
    settings.pulseIntervalMs = 1;
    if ( !m_node.SetSettings( settings ) || !m_node.Start() )
    {
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The node could not start" );
        return false;
    }

    if ( GUCEF_NULL == m_directory )
    {
        m_directory = m_node.CreateSession();
        SubscribeToDirectory( m_directory );
    }
    m_listings = GUCEF_NULL;
    m_requests = GUCEF_NULL;

    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = CGucefLobbyProtocol::DirectorySessionName;
    DRN::CDRNAuthSettings credentials;
    CGucefLobbyAuthenticator::MakeDirectoryCredentials( m_identity.privateKey, m_password, m_identity.linkProofs, credentials );
    // The credentials hold the proofs for the handshake; the identity of the service does not need to
    m_identity.linkProofs.clear();
    m_connectPending = true;
    if ( !m_directory->Join( m_serverEndpoints, sessionSettings, &credentials ) )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Could not join the directory of the server" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::Disconnect( void )
{GUCEF_TRACE;

    bool wasConnected = m_connected;
    m_connected = false;
    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Disconnected" );
    }
    FailPending( "Disconnected" );
    if ( GUCEF_NULL != m_directory && m_directory->IsActive() )
        m_directory->Leave();
    m_listings = GUCEF_NULL;
    m_requests = GUCEF_NULL;
    if ( wasConnected )
        NotifyObservers( ServiceDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::FailPending( const CORE::CString& reason )
{GUCEF_TRACE;

    TPendingRequestMap::iterator i = m_pendingRequests.begin();
    while ( i != m_pendingRequests.end() )
    {
        (*i).second.result.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
        ++i;
    }
    m_pendingRequests.clear();
    for ( size_t n=0; n<m_pendingBrowses.size(); ++n )
        m_pendingBrowses[ n ].Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
    m_pendingBrowses.clear();
    m_pendingBrowseFilters.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CGucefLobbyService::SendRequest( const CORE::CString& verb, const CORE::CVariant* arg1, const CORE::CVariant* arg2,
                                 const CORE::CVariant* arg3, const CORE::CVariant* arg4, const CORE::CVariant* arg5 )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_requests )
        return 0;
    UInt32 requestId = ++m_nextRequestId;
    CORE::CVariantStream& entry = m_requests->GetWriteStream();
    entry << requestId << verb;
    const CORE::CVariant* args[ 5 ] = { arg1, arg2, arg3, arg4, arg5 };
    for ( UInt32 i=0; i<5; ++i )
        if ( GUCEF_NULL != args[ i ] )
            entry << *args[ i ];
    entry.WriteSegmentEnd();
    if ( m_requests->CommitEntry() )
        return requestId;
    entry.Clear();
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnJoined( void )
{GUCEF_TRACE;

    DRN::CDRNDataStreamSettings requests;
    requests.name = CGucefLobbyProtocol::RequestsStream;
    requests.backlogLimit = 1000;
    // What goes out before the server subscribed reaches it as the history of the stream
    requests.retainedHistory = 64;
    m_requests = m_directory->CreateStream( requests );
    m_directory->Subscribe( CGucefLobbyProtocol::ListingsGrid );
    m_listings = m_directory->FindGrid( CGucefLobbyProtocol::ListingsGrid );
    if ( GUCEF_NULL != m_listings )
        SubscribeToListings( m_listings );

    CORE::CVariant userId( m_identity.userId );
    CORE::CVariant name( m_identity.displayName.IsNULLOrEmpty() ? m_identity.userId : m_identity.displayName );
    UInt32 requestId = SendRequest( "hello", &userId, &name );
    if ( 0 == requestId )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Could not say hello to the directory" );
        return;
    }
    m_pendingRequests[ requestId ].kind = REQUEST_HELLO;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::Browse( const CGameListingFilter& filter, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;
    CORE::CFutureResultSource result;
    future = result.GetFuture();
    m_pendingBrowses.push_back( result );
    m_pendingBrowseFilters.push_back( filter );
    if ( GUCEF_NULL != m_listings && m_listings->IsSnapshotComplete() )
        CompleteBrowses();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::CompleteBrowses( void )
{GUCEF_TRACE;

    if ( m_pendingBrowses.empty() || GUCEF_NULL == m_listings )
        return;

    GUCEF::vector< CGameListingEntry > all;
    GUCEF::vector< CORE::CVariant > keys;
    m_listings->GetKeys( keys );
    for ( size_t i=0; i<keys.size(); ++i )
    {
        CORE::CVariant value;
        CORE::CDataNode tree;
        CGameListingEntry listing;
        if ( m_listings->GetValue( keys[ i ], value ) && CGucefLobbyProtocol::VariantToDataTree( value, tree ) &&
             listing.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) )
            all.push_back( listing );
    }

    TFutureSourceVector browses;
    TFilterVector filters;
    browses.swap( m_pendingBrowses );
    filters.swap( m_pendingBrowseFilters );
    for ( size_t b=0; b<browses.size(); ++b )
    {
        CGameListingList list;
        for ( size_t i=0; i<all.size(); ++i )
            if ( filters[ b ].Matches( all[ i ] ) )
                list.listings.push_back( all[ i ] );
        browses[ b ].Complete( list.Clone(), true );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::CreateLobby( const CLobbyCreateParams& params, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;
    CORE::CDataNode tree( "params" );
    params.SaveConfig( tree );
    CORE::CVariant blob = CGucefLobbyProtocol::DataTreeToVariant( tree );
    CORE::CVariant endpoints( JoinStrings( m_localEndpoints ) );
    CORE::CVariant integrity( MakeIntegrityBlob() );
    UInt32 requestId = SendRequest( "create", &blob, &endpoints, &integrity );
    if ( 0 == requestId )
        return false;
    TPendingRequest& request = m_pendingRequests[ requestId ];
    request.kind = REQUEST_LOBBY;
    request.asSpectator = false;
    future = request.result.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::JoinLobby( const CORE::CString& lobbyId    ,
                               const CORE::CString& password   ,
                               bool asSpectator                ,
                               CORE::CFutureResult& future     )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;
    CORE::CVariant id( lobbyId );
    CORE::CVariant secret( password );
    CORE::CVariant spectator( asSpectator );
    CORE::CVariant endpoints( JoinStrings( m_localEndpoints ) );
    CORE::CVariant integrity( MakeIntegrityBlob() );
    UInt32 requestId = SendRequest( "join", &id, &secret, &spectator, &endpoints, &integrity );
    if ( 0 == requestId )
        return false;
    TPendingRequest& request = m_pendingRequests[ requestId ];
    request.kind = REQUEST_LOBBY;
    request.asSpectator = asSpectator;
    future = request.result.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

CLobbyServiceCapabilities
CGucefLobbyService::GetCapabilities( void ) const
{GUCEF_TRACE;

    CLobbyServiceCapabilities capabilities;
    capabilities.voting = true;
    capabilities.relay = true;
    capabilities.inProgressSpectating = true;
    capabilities.chat = true;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::PublishListing( const CGameListingEntry& listing )
{GUCEF_TRACE;

    if ( !m_connected || listing.listingId.IsNULLOrEmpty() )
        return false;
    CORE::CDataNode tree( "listing" );
    listing.Serialize( tree, CORE::CDataNodeSerializableSettings() );
    CORE::CVariant blob = CGucefLobbyProtocol::DataTreeToVariant( tree );
    return 0 != SendRequest( "publish", &blob );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::WithdrawListing( const CORE::CString& listingId )
{GUCEF_TRACE;

    if ( !m_connected )
        return false;
    CORE::CVariant id( listingId );
    return 0 != SendRequest( "withdraw", &id );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::SubscribeChannel( const CORE::CString& channel, bool subscribe )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_directory || !m_connected )
        return false;
    CORE::CString itemName = CGucefLobbyProtocol::ChannelStreamPrefix + channel;
    if ( !subscribe )
        return m_directory->Unsubscribe( itemName, m_directory->GetSequencerPeerId() );
    if ( !m_directory->Subscribe( itemName, m_directory->GetSequencerPeerId() ) )
        return false;
    DRN::CDRNDataStream* stream = m_directory->FindStream( itemName, m_directory->GetSequencerPeerId() );
    if ( GUCEF_NULL != stream )
        SubscribeToChannel( stream );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnLobbyGranted( TPendingRequest& request, const CORE::CVariant* args )
{GUCEF_TRACE;

    CORE::CString lobbyId = args[ 0 ].AsString( CORE::CString::Empty, true );
    CORE::CString token = args[ 1 ].AsString( CORE::CString::Empty, true );
    DRN::CDRNEndpointList endpoints;
    endpoints.FromString( args[ 2 ].AsString( CORE::CString::Empty, true ) );
    if ( endpoints.IsEmpty() )
        endpoints = m_serverEndpoints;

    m_retiredLobbies.clear();
    CGucefLobby* lobby = GUCEF_NEW CGucefLobby( lobbyId, m_identity.userId );
    TPendingLobby pending;
    pending.lobby = CILobbyPtr( lobby );
    pending.result = request.result;
    SubscribeToLobby( lobby );
    if ( !lobby->Start( endpoints, token, request.asSpectator ) )
    {
        UnsubscribeFrom( lobby );
        request.result.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Could not join the lobby session" );
        return;
    }
    m_pendingLobbies.push_back( pending );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnReply( UInt32 requestId, bool ok, const CORE::CString& reason, const CORE::CVariant* args )
{GUCEF_TRACE;

    TPendingRequestMap::iterator i = m_pendingRequests.find( requestId );
    if ( i != m_pendingRequests.end() )
    {
        TPendingRequest request = (*i).second;
        m_pendingRequests.erase( i );
        if ( REQUEST_HELLO == request.kind )
        {
            if ( !m_connectPending )
                return;
            m_connectPending = false;
            if ( !ok )
            {
                m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
                return;
            }
            // The server knows the player by the user id it replies with: the one of the key
            CORE::CString userId = args[ 0 ].AsString( CORE::CString::Empty, true );
            if ( !userId.IsNULLOrEmpty() )
                m_identity.userId = userId;
            m_connected = true;
            CPlayerIdentity identity( m_identity.GetPublicIdentity() );
            m_connectResult.Complete( identity.Clone(), true );
            NotifyObservers( ServiceConnectedEvent );
            return;
        }
        if ( REQUEST_LOBBY == request.kind )
        {
            if ( ok )
                OnLobbyGranted( request, args );
            else
                request.result.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
            return;
        }
    }

    CGucefDirectoryReply reply;
    reply.requestId = requestId;
    reply.ok = ok;
    reply.reason = reason;
    for ( UInt32 n=0; n<3; ++n )
        reply.args[ n ] = args[ n ];
    NotifyObservers( ReplyEvent, &reply );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnAppMessage( const CORE::CDynamicBuffer& payload )
{GUCEF_TRACE;

    DRN::CDRNWireReader reader( payload );
    UInt8 type = 0;
    if ( !reader.ReadUInt8( type ) )
        return;

    if ( CGucefLobbyProtocol::MESSAGE_REPLY == type )
    {
        UInt32 requestId = 0;
        bool ok = false;
        CORE::CString reason;
        CORE::CVariant args[ 3 ];
        if ( !reader.ReadVarUInt32( requestId ) || !reader.ReadBool( ok ) || !reader.ReadString( reason ) )
            return;
        for ( UInt32 i=0; i<3; ++i )
            DRN::CDRNVariantCodec::Read( reader, args[ i ] );
        OnReply( requestId, ok, reason, args );
        return;
    }

    if ( CGucefLobbyProtocol::MESSAGE_DIRECT_MESSAGE == type )
    {
        CORE::CString from, text, id;
        UInt64 timeMs = 0;
        if ( !reader.ReadString( from ) || !reader.ReadString( text ) || !reader.ReadVarUInt64( timeMs ) || !reader.ReadString( id ) )
            return;
        CGucefDirectoryReply message;
        message.ok = true;
        message.args[ 0 ] = CORE::CVariant( from );
        message.args[ 1 ] = CORE::CVariant( text );
        message.args[ 2 ] = CORE::CVariant( timeMs );
        message.args[ 3 ] = CORE::CVariant( id );
        NotifyObservers( DirectMessageEvent, &message );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnChannelEntries( DRN::CDRNDataStream& stream )
{GUCEF_TRACE;

    CORE::CString channel = stream.GetName().CutChars( CGucefLobbyProtocol::ChannelStreamPrefix.Length(), true );
    CORE::CVariantStream entry;
    while ( stream.ReadNextEntry( entry ) )
    {
        CORE::CString kind, from, text, id;
        UInt64 timeMs = 0;
        entry >> kind >> from >> text >> timeMs >> id;
        entry.Clear();

        CGucefDirectoryReply data;
        data.ok = true;
        data.reason = channel;
        data.args[ 0 ] = CORE::CVariant( kind );
        data.args[ 1 ] = CORE::CVariant( from );
        data.args[ 2 ] = CORE::CVariant( text );
        data.args[ 3 ] = CORE::CVariant( timeMs );
        data.args[ 4 ] = CORE::CVariant( id );
        if ( !NotifyObservers( ChannelEntryEvent, &data ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SubscribeToDirectory( DRN::CDRNSession* directory )
{GUCEF_TRACE;

    TEventCallback joinedCallback( this, &CGucefLobbyService::OnDirectoryJoined );
    TEventCallback joinFailedCallback( this, &CGucefLobbyService::OnDirectoryJoinFailed );
    TEventCallback announcedCallback( this, &CGucefLobbyService::OnDirectoryItemAnnounced );
    TEventCallback appMessageCallback( this, &CGucefLobbyService::OnDirectoryAppMessage );
    TEventCallback closedCallback( this, &CGucefLobbyService::OnDirectoryClosed );
    SubscribeTo( directory, DRN::CDRNSession::JoinedEvent, joinedCallback );
    SubscribeTo( directory, DRN::CDRNSession::JoinFailedEvent, joinFailedCallback );
    SubscribeTo( directory, DRN::CDRNSession::ItemAnnouncedEvent, announcedCallback );
    SubscribeTo( directory, DRN::CDRNSession::AppMessageEvent, appMessageCallback );
    SubscribeTo( directory, DRN::CDRNSession::SessionClosedEvent, closedCallback );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SubscribeToListings( DRN::CDRNDataGrid* listings )
{GUCEF_TRACE;

    TEventCallback changedCallback( this, &CGucefLobbyService::OnListingsChanged );
    SubscribeTo( listings, DRN::CDRNDataItem::SnapshotCompleteEvent, changedCallback );
    SubscribeTo( listings, DRN::CDRNDataGrid::GridUpdatedEvent, changedCallback );
    SubscribeTo( listings, DRN::CDRNDataGrid::GridKeyChangedEvent, changedCallback );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SubscribeToChannel( DRN::CDRNDataStream* stream )
{GUCEF_TRACE;

    TEventCallback entriesCallback( this, &CGucefLobbyService::OnChannelEntriesAvailable );
    SubscribeTo( stream, DRN::CDRNDataStream::EntriesAvailableEvent, entriesCallback );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::SubscribeToLobby( CGucefLobby* lobby )
{GUCEF_TRACE;

    TEventCallback readyCallback( this, &CGucefLobbyService::OnLobbyReady );
    TEventCallback failedCallback( this, &CGucefLobbyService::OnLobbyFailed );
    SubscribeTo( lobby, CGucefLobby::LobbyReadyEvent, readyCallback );
    SubscribeTo( lobby, CGucefLobby::LobbyFailedEvent, failedCallback );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnDirectoryJoined( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_directory && notifier == m_directory )
        OnJoined();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnDirectoryJoinFailed( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_directory || notifier != m_directory )
        return;
    if ( m_connectPending )
    {
        m_connectPending = false;
        CORE::CString reason = GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CORE::CString( "Could not join the directory" );
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnDirectoryItemAnnounced( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_directory || notifier != m_directory )
        return;

    const DRN::CDRNItemInfo& info = static_cast< DRN::CDRNItemInfoEventData* >( eventData )->GetData();
    if ( info.name == CGucefLobbyProtocol::ListingsGrid && GUCEF_NULL == m_listings )
    {
        m_listings = m_directory->FindGrid( info.name );
        if ( GUCEF_NULL != m_listings )
            SubscribeToListings( m_listings );
    }
    else
    if ( 0 == info.name.HasSubstr( CGucefLobbyProtocol::ChannelStreamPrefix, true ) )
    {
        DRN::CDRNDataStream* stream = m_directory->FindStream( info.name, info.ownerPeerId );
        if ( GUCEF_NULL != stream )
            SubscribeToChannel( stream );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnDirectoryAppMessage( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_directory && notifier == m_directory )
        OnAppMessage( static_cast< DRN::CDRNAppMessageEventData* >( eventData )->GetData().payload );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnDirectoryClosed( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_directory || notifier != m_directory )
        return;

    bool wasConnected = m_connected;
    m_connected = false;
    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The directory session ended" );
    }
    FailPending( "The directory session ended" );
    m_listings = GUCEF_NULL;
    m_requests = GUCEF_NULL;
    if ( wasConnected )
    {
        CORE::TCloneableString error( "The connection to the lobby server ended" );
        NotifyObservers( ServiceErrorEvent, &error );
        NotifyObservers( ServiceDisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnListingsChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_listings && notifier == m_listings && m_listings->IsSnapshotComplete() )
        CompleteBrowses();
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnChannelEntriesAvailable( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    DRN::CDRNDataStream* stream = static_cast< DRN::CDRNDataStream* >( notifier );
    OnChannelEntries( *stream );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnLobbyReady( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnPendingLobbyEnded( notifier, true, eventData );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnLobbyFailed( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnPendingLobbyEnded( notifier, false, eventData );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyService::OnPendingLobbyEnded( CORE::CNotifier* notifier, bool ready, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_pendingLobbies.size(); ++i )
    {
        if ( static_cast< CORE::CNotifier* >( m_pendingLobbies[ i ].lobby.GetPointerAlways() ) != notifier )
            continue;
        TPendingLobby pending = m_pendingLobbies[ i ];
        m_pendingLobbies.erase( m_pendingLobbies.begin() + i );
        if ( ready )
        {
            CLobbyResultData result( pending.lobby );
            pending.result.Complete( result.Clone(), true );
            NotifyObservers( LobbyJoinedEvent, &result );
        }
        else
        {
            CORE::CString reason = GUCEF_NULL != eventData ? static_cast< CORE::TCloneableString* >( eventData )->GetData() : CORE::CString( "Could not join the lobby" );
            pending.result.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );
            m_retiredLobbies.push_back( pending.lobby );
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "server", m_serverEndpoints.ToString() );
    config.SetAttribute( "password", m_password );
    config.SetAttribute( "localEndpoints", JoinStrings( m_localEndpoints ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString server = config.GetAttributeValueOrChildValueByName( "server" ).AsString( CORE::CString::Empty, true );
    if ( !server.IsNULLOrEmpty() )
        m_serverEndpoints.FromString( server );
    m_password = config.GetAttributeValueOrChildValueByName( "password" ).AsString( m_password, true );
    CORE::CString endpoints = config.GetAttributeValueOrChildValueByName( "localEndpoints" ).AsString( CORE::CString::Empty, true );
    if ( !endpoints.IsNULLOrEmpty() )
    {
        m_localEndpoints.clear();
        CORE::CString::StringVector parts = endpoints.ParseElements( ';', false );
        for ( size_t i=0; i<parts.size(); ++i )
            if ( !parts[ i ].IsNULLOrEmpty() )
                m_localEndpoints.push_back( parts[ i ] );
    }
    return !m_serverEndpoints.IsEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyService::ClassTypeName( void )
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyService";
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
