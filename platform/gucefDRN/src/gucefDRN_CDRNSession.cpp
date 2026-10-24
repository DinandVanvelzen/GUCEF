/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNBULKTRANSFER_H
#include "gucefDRN_CDRNBulkTransfer.h"
#define GUCEF_DRN_CDRNBULKTRANSFER_H
#endif /* GUCEF_DRN_CDRNBULKTRANSFER_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONRECORDER_H
#include "gucefDRN_CDRNSessionRecorder.h"
#define GUCEF_DRN_CDRNSESSIONRECORDER_H
#endif /* GUCEF_DRN_CDRNSESSIONRECORDER_H ? */

#include "gucefDRN_CDRNSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNSession::JoinedEvent = "GUCEF::DRN::CDRNSession::JoinedEvent";
const CORE::CEvent CDRNSession::JoinFailedEvent = "GUCEF::DRN::CDRNSession::JoinFailedEvent";
const CORE::CEvent CDRNSession::MemberJoinedEvent = "GUCEF::DRN::CDRNSession::MemberJoinedEvent";
const CORE::CEvent CDRNSession::MemberLeftEvent = "GUCEF::DRN::CDRNSession::MemberLeftEvent";
const CORE::CEvent CDRNSession::ItemAnnouncedEvent = "GUCEF::DRN::CDRNSession::ItemAnnouncedEvent";
const CORE::CEvent CDRNSession::ItemRemovedEvent = "GUCEF::DRN::CDRNSession::ItemRemovedEvent";
const CORE::CEvent CDRNSession::SessionClosedEvent = "GUCEF::DRN::CDRNSession::SessionClosedEvent";
const CORE::CEvent CDRNSession::AppMessageEvent = "GUCEF::DRN::CDRNSession::AppMessageEvent";

const CORE::CString CDRNSession::LatencyGridName = "drn.latency";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNSessionMemberInfo::CDRNSessionMemberInfo( void )
    : peerId( 0 )
    , nodeId()
    , endpoints()
    , isLocal( false )
    , isSequencer( false )
    , hasDirectLink( false )
    , reason( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAppMessage::CDRNAppMessage( void )
    : fromPeer( 0 )
    , toPeer( 0 )
    , lane( 0 )
    , payload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSession::CMember::CMember( void )
    : peerId( 0 )
    , nodeId()
    , endpoints()
    , link()
    , connector()
    , known( false )
    , isLocal( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNSession::CInternalObserver::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_session )
        m_session->OnInternalNotify( notifier, eventid, eventdata );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::RegisterEvents( void )
{GUCEF_TRACE;

    JoinedEvent.Initialize();
    JoinFailedEvent.Initialize();
    MemberJoinedEvent.Initialize();
    MemberLeftEvent.Initialize();
    ItemAnnouncedEvent.Initialize();
    ItemRemovedEvent.Initialize();
    SessionClosedEvent.Initialize();
    AppMessageEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNSession::CDRNSession( CDRNNode& node )
    : CORE::CTSGNotifier( node.GetAppPulseGenerator(), true, true )
    , m_node( &node )
    , m_observer()
    , m_settings()
    , m_state( SESSIONSTATE_IDLE )
    , m_topology( DRN_TOPOLOGY_CLIENT_SERVER )
    , m_localPeerId( 0 )
    , m_sequencerPeerId( 0 )
    , m_nextPeerId( 2 )
    , m_sessionEpoch( 0 )
    , m_joinDeadline( 0 )
    , m_joinConnector()
    , m_sequencerLink()
    , m_members()
    , m_memberCount( 0 )
    , m_replicator( GUCEF_NULL )
    , m_bulkTransfers( GUCEF_NULL )
    , m_latencyGrid( GUCEF_NULL )
    , m_nextLatencyPublish( 0 )
    , m_txBuffer()
    , m_appMessageEvent()
    , m_recorder( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    m_observer.m_session = this;
    for ( UInt32 i=0; i<DRN_MSGCLASS_COUNT; ++i )
        m_components[ i ] = GUCEF_NULL;

    // the grids and streams are built in
    m_replicator = GUCEF_NEW CDRNReplicator( *this );
    m_components[ DRN_MSGCLASS_REPL ] = m_replicator;
    m_components[ DRN_MSGCLASS_STREAM ] = m_replicator->GetStreamComponent();
    m_bulkTransfers = GUCEF_NEW CDRNBulkTransferManager( *this );
    m_components[ DRN_MSGCLASS_BULK ] = m_bulkTransfers;
}

/*-------------------------------------------------------------------------*/

CDRNSession::~CDRNSession()
{GUCEF_TRACE;

    m_observer.m_session = GUCEF_NULL;
    m_observer.UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_recorder )
    {
        m_recorder->OnSessionGone();
        m_recorder = GUCEF_NULL;
    }
    CloseSession( DRN_LEAVE_SESSION_CLOSED, false );
    m_components[ DRN_MSGCLASS_REPL ] = GUCEF_NULL;
    m_components[ DRN_MSGCLASS_STREAM ] = GUCEF_NULL;
    m_components[ DRN_MSGCLASS_BULK ] = GUCEF_NULL;
    GUCEF_DELETE m_bulkTransfers;
    m_bulkTransfers = GUCEF_NULL;
    GUCEF_DELETE m_replicator;
    m_replicator = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_node->IsRunningOwnThread() )
        NotifyObserversFromThread( eventid, eventData );
    else
        NotifyObservers( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::EmitMemberEvent( const CORE::CEvent& eventid, const CMember& member, UInt32 reason )
{GUCEF_TRACE;

    CDRNSessionMemberEventData eventData;
    CDRNSessionMemberInfo& info = eventData.GetData();
    info.peerId = member.peerId;
    info.nodeId = member.nodeId;
    info.endpoints.FromString( member.endpoints );
    info.isLocal = member.isLocal;
    info.isSequencer = member.peerId == m_sequencerPeerId;
    info.hasDirectLink = !member.link.IsNULL();
    info.reason = reason;
    Emit( eventid, &eventData );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNSession::GetAdvertisedEndpoints( void ) const
{GUCEF_TRACE;

    // with the node's public key, so that members who connect to these endpoints pin it
    if ( !m_settings.advertisedEndpoints.IsEmpty() )
    {
        CORE::CString key = m_node->GetPublicKeyString();
        CDRNEndpointList endpoints;
        for ( UInt32 i=0; i<m_settings.advertisedEndpoints.GetCount(); ++i )
        {
            CDRNEndpoint endpoint = m_settings.advertisedEndpoints.Get( i );
            if ( !key.IsNULLOrEmpty() && !endpoint.GetParams().HasKey( "key" ) )
                endpoint.GetParams().Set( "key", CORE::CVariant( key ) );
            endpoints.Add( endpoint );
        }
        return FilterEndpoints( endpoints.ToString() );
    }
    return FilterEndpoints( m_node->GetPublishedEndpoints().ToString() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNSession::FilterEndpoints( const CORE::CString& endpoints ) const
{GUCEF_TRACE;

    // what the security policy does not allow is never passed on: with relayOnly only relay endpoints
    const CDRNSecurityPolicy& policy = m_settings.securityPolicy;
    if ( !policy.relayOnly && policy.allowedChannelTypes.empty() )
        return endpoints;

    CDRNEndpointList all;
    all.FromString( endpoints );
    CDRNEndpointList allowed;
    for ( UInt32 i=0; i<all.GetCount(); ++i )
    {
        if ( policy.IsEndpointAllowed( all.Get( i ) ) )
            allowed.Add( all.Get( i ) );
    }
    return allowed.ToString();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::IsLinkAllowed( const CDRNPeerLink& link ) const
{GUCEF_TRACE;

    CIDRNChannelPtr channel = link.GetChannel();
    if ( channel.IsNULL() )
        return false;
    CDRNEndpoint endpoint = link.GetRemoteEndpoint();
    endpoint.SetChannelType( channel->GetChannelType() );
    return m_settings.securityPolicy.IsEndpointAllowed( endpoint );
}

/*-------------------------------------------------------------------------*/

CDRNSession::CMember*
CDRNSession::GetMember( UInt32 peerId ) const
{GUCEF_TRACE;
    if ( 0 == peerId || peerId >= m_members.size() )
        return GUCEF_NULL;
    return m_members[ peerId ];
}

/*-------------------------------------------------------------------------*/

CDRNSession::CMember*
CDRNSession::GetOrAddMember( UInt32 peerId )
{GUCEF_TRACE;

    if ( 0 == peerId )
        return GUCEF_NULL;
    if ( peerId >= m_members.size() )
        m_members.resize( peerId + 1, GUCEF_NULL );
    CMember* member = m_members[ peerId ];
    if ( GUCEF_NULL == member )
    {
        member = GUCEF_NEW CMember();
        member->peerId = peerId;
        m_members[ peerId ] = member;
        ++m_memberCount;
    }
    return member;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::BindLink( CDRNPeerLink* link, CMember* member )
{GUCEF_TRACE;

    link->SetSessionBinding( this, member->peerId );
    member->link = link->GetSharedPtr();
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink*
CDRNSession::GetSequencerLink( void ) const
{GUCEF_TRACE;
    return m_sequencerLink.IsNULL() ? GUCEF_NULL : const_cast< CDRNPeerLink* >( m_sequencerLink.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::Host( const CDRNSessionSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );

    if ( SESSIONSTATE_IDLE != m_state && SESSIONSTATE_CLOSED != m_state )
        return false;
    if ( settings.sessionName.IsNULLOrEmpty() || GUCEF_NULL != m_node->FindSession( settings.sessionName ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Cannot host a session without a name or with the name of another session: \"" << settings.sessionName << "\"" );
        return false;
    }
    CORE::CString whyNot;
    if ( !settings.securityPolicy.CanBeHonoured( whyNot ) || !m_node->GetSettings().securityPolicy.CanBeHonoured( whyNot ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Cannot host \"" << settings.sessionName << "\": " << whyNot );
        return false;
    }

    m_replicator->Reset();
    m_bulkTransfers->Reset();
    m_latencyGrid = GUCEF_NULL;
    m_nextLatencyPublish = 0;
    m_settings = settings;
    m_topology = settings.topology;
    m_localPeerId = 1;
    m_sequencerPeerId = 1;
    m_nextPeerId = 2;
    m_sessionEpoch = CDRNClock::NowInMicroseconds();

    CMember* self = GetOrAddMember( m_localPeerId );
    self->nodeId = m_node->GetSettings().nodeId;
    self->endpoints = GetAdvertisedEndpoints();
    self->known = true;
    self->isLocal = true;
    m_state = SESSIONSTATE_HOSTING;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Hosting \"" << m_settings.sessionName << "\" as " << CDRNSessionSettings::TopologyToString( m_topology ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::Join( const CDRNEndpointList& host, const CDRNSessionSettings& settings, const CDRNAuthSettings* credentials )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );

    if ( SESSIONSTATE_IDLE != m_state && SESSIONSTATE_CLOSED != m_state )
        return false;
    if ( settings.sessionName.IsNULLOrEmpty() || host.IsEmpty() )
        return false;

    CORE::CString whyNot;
    if ( !settings.securityPolicy.CanBeHonoured( whyNot ) || !m_node->GetSettings().securityPolicy.CanBeHonoured( whyNot ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Cannot join \"" << settings.sessionName << "\": " << whyNot );
        return false;
    }
    const CORE::CString& authenticatorType = GUCEF_NULL != credentials ? credentials->authenticatorType : m_node->GetSettings().auth.authenticatorType;
    if ( !settings.securityPolicy.IsAuthenticatorAllowed( authenticatorType ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Cannot join \"" << settings.sessionName << "\": the security policy does not allow the authenticator \"" << authenticatorType << "\"" );
        return false;
    }
    CDRNEndpointList allowedHost;
    for ( UInt32 i=0; i<host.GetCount(); ++i )
    {
        if ( settings.securityPolicy.IsEndpointAllowed( host.Get( i ) ) )
            allowedHost.Add( host.Get( i ) );
    }
    if ( allowedHost.IsEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Cannot join \"" << settings.sessionName << "\": the security policy allows none of the host's endpoints" );
        return false;
    }

    m_replicator->Reset();
    m_bulkTransfers->Reset();
    m_latencyGrid = GUCEF_NULL;
    m_nextLatencyPublish = 0;
    m_settings = settings;
    m_topology = settings.topology;
    m_localPeerId = 0;
    m_sequencerPeerId = 0;
    m_state = SESSIONSTATE_JOINING;
    m_joinDeadline = CDRNClock::NowInMicroseconds() + (UInt64) m_settings.joinTimeoutMs * 1000;

    m_joinConnector = m_node->Connect( allowedHost, credentials );
    m_observer.SubscribeTo( m_joinConnector.GetPointerAlways() );
    if ( m_joinConnector->IsDone() )
    {
        OnConnectorDone( m_joinConnector.GetPointerAlways(), m_joinConnector->HasSucceeded() );
        return SESSIONSTATE_JOINING == m_state;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::Leave( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );

    if ( SESSIONSTATE_HOSTING == m_state || SESSIONSTATE_MEMBER == m_state )
    {
        UInt8 payload[ 16 ];
        UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_localPeerId, payload );
        size += CDRNWireWriter::EncodeVarUInt64( SESSIONSTATE_HOSTING == m_state ? DRN_LEAVE_SESSION_CLOSED : DRN_LEAVE_LEFT, payload + size );
        for ( UInt32 i=0; i<m_members.size(); ++i )
        {
            CMember* member = m_members[ i ];
            if ( GUCEF_NULL != member && !member->link.IsNULL() )
                member->link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_LEAVE, payload, size );
        }
    }
    if ( SESSIONSTATE_CLOSED != m_state && SESSIONSTATE_IDLE != m_state )
        CloseSession( DRN_LEAVE_LEFT, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::Shutdown( void )
{GUCEF_TRACE;

    if ( SESSIONSTATE_HOSTING == m_state || SESSIONSTATE_MEMBER == m_state )
    {
        UInt8 payload[ 16 ];
        UInt32 size = CDRNWireWriter::EncodeVarUInt64( m_localPeerId, payload );
        size += CDRNWireWriter::EncodeVarUInt64( SESSIONSTATE_HOSTING == m_state ? DRN_LEAVE_SESSION_CLOSED : DRN_LEAVE_LEFT, payload + size );
        for ( UInt32 i=0; i<m_members.size(); ++i )
        {
            CMember* member = m_members[ i ];
            if ( GUCEF_NULL != member && !member->link.IsNULL() )
                member->link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_LEAVE, payload, size );
        }
    }
    CloseSession( DRN_LEAVE_SESSION_CLOSED, false );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::CloseSession( UInt32 reason, bool notify )
{GUCEF_TRACE;

    bool wasActive = SESSIONSTATE_CLOSED != m_state && SESSIONSTATE_IDLE != m_state;
    m_state = SESSIONSTATE_CLOSED;

    if ( !m_joinConnector.IsNULL() )
    {
        m_observer.UnsubscribeFrom( *m_joinConnector.GetPointerAlways() );
        m_joinConnector->Cancel();
        m_joinConnector.Unlink();
    }

    TMemberVector members;
    members.swap( m_members );
    m_memberCount = 0;
    for ( UInt32 i=0; i<members.size(); ++i )
    {
        CMember* member = members[ i ];
        if ( GUCEF_NULL == member )
            continue;
        if ( !member->connector.IsNULL() )
        {
            m_observer.UnsubscribeFrom( *member->connector.GetPointerAlways() );
            member->connector->Cancel();
        }
        if ( !member->link.IsNULL() )
        {
            member->link->SetSessionBinding( GUCEF_NULL, 0 );
            member->link->Close( DRN_LINKREASON_CLOSED_BY_APP, "session closed" );
        }
        GUCEF_DELETE member;
    }
    members.clear();

    if ( !m_sequencerLink.IsNULL() )
    {
        m_sequencerLink->SetSessionBinding( GUCEF_NULL, 0 );
        if ( !m_sequencerLink->IsClosed() )
            m_sequencerLink->Close( DRN_LINKREASON_CLOSED_BY_APP, "session closed" );
        m_sequencerLink.Unlink();
    }

    if ( wasActive )
    {
        for ( UInt32 i=0; i<DRN_MSGCLASS_COUNT; ++i )
        {
            if ( GUCEF_NULL != m_components[ i ] )
                m_components[ i ]->OnSessionClosed( *this );
        }
        if ( notify )
        {
            CORE::TCloneableUInt32 eventData( reason );
            Emit( SessionClosedEvent, &eventData );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::FailJoin( const CORE::CString& reason )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Joining \"" << m_settings.sessionName << "\" failed: " << reason );
    CloseSession( DRN_LEAVE_LINK_LOST, false );
    CORE::TCloneableString eventData( reason );
    Emit( JoinFailedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnInternalNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( CDRNConnector::ConnectedEvent == eventid )
        OnConnectorDone( static_cast< CDRNConnector* >( notifier ), true );
    else
    if ( CDRNConnector::ConnectFailedEvent == eventid )
        OnConnectorDone( static_cast< CDRNConnector* >( notifier ), false );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnConnectorDone( CDRNConnector* connector, bool succeeded )
{GUCEF_TRACE;

    m_observer.UnsubscribeFrom( *connector );

    if ( !m_joinConnector.IsNULL() && connector == m_joinConnector.GetPointerAlways() )
    {
        CDRNConnectorPtr joinConnector = m_joinConnector;
        m_joinConnector.Unlink();
        if ( SESSIONSTATE_JOINING != m_state )
            return;
        if ( !succeeded )
        {
            FailJoin( "could not link with the host" );
            return;
        }
        m_sequencerLink = joinConnector->GetLink();
        m_sequencerLink->SetSessionBinding( this, 0 );
        SendJoin( m_sequencerLink.GetPointerAlways(), 0, 0 );
        m_sequencerLink->Flush();
        return;
    }

    // peer to peer: a link with another member
    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        CMember* member = m_members[ i ];
        if ( GUCEF_NULL == member || member->connector.IsNULL() || member->connector.GetPointerAlways() != connector )
            continue;

        CDRNConnectorPtr memberConnector = member->connector;
        member->connector.Unlink();
        if ( !succeeded )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: No direct link with member " << member->peerId << ", its messages go through the sequencer" );
            return;
        }
        CDRNPeerLinkPtr link = memberConnector->GetLink();
        // the member is linked once it answered the JOIN, until then messages go through the sequencer
        link->SetSessionBinding( this, member->peerId );
        SendJoin( link.GetPointerAlways(), m_localPeerId, DRN_JOIN_FLAG_MESH );
        link->Flush();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendJoin( CDRNPeerLink* link, UInt32 peerId, UInt32 flags )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteString( m_settings.sessionName );
    writer.WriteVarUInt32( peerId );
    writer.WriteVarUInt32( flags );
    writer.WriteString( m_node->GetSettings().nodeId );
    writer.WriteString( GetAdvertisedEndpoints() );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_JOIN, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendJoinedReply( CDRNPeerLink* link, UInt32 result, const CORE::CString& reason, UInt32 peerId, UInt32 flags )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( result );
    writer.WriteString( reason );
    writer.WriteVarUInt32( peerId );
    writer.WriteVarUInt32( flags );
    writer.WriteUInt8( (UInt8) m_topology );
    writer.WriteVarUInt32( m_sequencerPeerId );
    writer.WriteVarUInt64( m_sessionEpoch );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_JOINED, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendMemberList( CDRNPeerLink* link )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_members.size(); ++i )
        if ( GUCEF_NULL != m_members[ i ] && m_members[ i ]->known )
            ++count;
    writer.WriteVarUInt32( count );
    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        CMember* member = m_members[ i ];
        if ( GUCEF_NULL == member || !member->known )
            continue;
        writer.WriteVarUInt32( member->peerId );
        writer.WriteString( member->nodeId );
        writer.WriteString( FilterEndpoints( member->endpoints ) );
    }
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_MEMBER_LIST, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendMemberAdded( CDRNPeerLink* link, const CMember& member )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( member.peerId );
    writer.WriteString( member.nodeId );
    writer.WriteString( FilterEndpoints( member.endpoints ) );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_MEMBER_ADDED, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendMemberRemoved( CDRNPeerLink* link, UInt32 peerId, UInt32 reason )
{GUCEF_TRACE;

    UInt8 payload[ 16 ];
    UInt32 size = CDRNWireWriter::EncodeVarUInt64( peerId, payload );
    size += CDRNWireWriter::EncodeVarUInt64( reason, payload + size );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_MEMBER_REMOVED, payload, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SendConnectTo( CDRNPeerLink* link, const CMember& member )
{GUCEF_TRACE;

    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( member.peerId );
    writer.WriteString( FilterEndpoints( member.endpoints ) );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_SESSION, DRN_SESSION_CONNECT_TO, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnJoinRequest( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    CDRNWireReader reader( message.payload );
    CORE::CString sessionName;
    UInt32 peerId = 0;
    UInt32 flags = 0;
    CORE::CString nodeId;
    CORE::CString endpoints;
    if ( !reader.ReadString( sessionName ) || !reader.ReadVarUInt32( peerId ) || !reader.ReadVarUInt32( flags ) ||
         !reader.ReadString( nodeId ) || !reader.ReadString( endpoints ) )
    {
        SendJoinedReply( link, DRN_JOIN_DENIED, "malformed JOIN", 0, 0 );
        return;
    }
    if ( !IsLinkAllowed( *link ) )
    {
        SendJoinedReply( link, DRN_JOIN_DENIED, "the security policy of the session does not allow this channel", 0, flags & DRN_JOIN_FLAG_MESH );
        return;
    }

    if ( 0 != ( flags & DRN_JOIN_FLAG_MESH ) )
    {
        // peer to peer: a member links with us directly
        if ( ( SESSIONSTATE_MEMBER != m_state && SESSIONSTATE_HOSTING != m_state ) || 0 == peerId || peerId == m_localPeerId )
        {
            SendJoinedReply( link, DRN_JOIN_NOT_HOSTING, "not a member of the session", 0, DRN_JOIN_FLAG_MESH );
            return;
        }
        CMember* member = GetOrAddMember( peerId );
        if ( member->nodeId.IsNULLOrEmpty() )
            member->nodeId = nodeId;
        if ( member->endpoints.IsNULLOrEmpty() )
            member->endpoints = endpoints;
        if ( !member->link.IsNULL() && member->link.GetPointerAlways() != link )
            member->link->SetSessionBinding( GUCEF_NULL, 0 );
        BindLink( link, member );
        SendJoinedReply( link, DRN_JOIN_OK, CORE::CString::Empty, m_localPeerId, DRN_JOIN_FLAG_MESH );
        return;
    }

    if ( SESSIONSTATE_HOSTING != m_state )
    {
        SendJoinedReply( link, DRN_JOIN_NOT_HOSTING, "this node does not host the session", 0, 0 );
        return;
    }
    UInt32 otherMembers = m_memberCount;
    if ( 0 != m_settings.maxMembers && otherMembers >= m_settings.maxMembers )
    {
        SendJoinedReply( link, DRN_JOIN_SESSION_FULL, "the session is full", 0, 0 );
        return;
    }
    if ( !m_node->GetAccessPolicy().MayJoin( *link, m_settings.sessionName ) )
    {
        SendJoinedReply( link, DRN_JOIN_DENIED, "not allowed to join", 0, 0 );
        return;
    }

    UInt32 newPeerId = m_nextPeerId++;
    CMember* member = GetOrAddMember( newPeerId );
    member->nodeId = nodeId;
    member->endpoints = endpoints;
    member->known = true;
    BindLink( link, member );

    SendJoinedReply( link, DRN_JOIN_OK, CORE::CString::Empty, newPeerId, 0 );
    SendMemberList( link );
    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        CMember* other = m_members[ i ];
        if ( GUCEF_NULL == other || other == member || other->isLocal || !other->known )
            continue;
        if ( !other->link.IsNULL() )
            SendMemberAdded( other->link.GetPointerAlways(), *member );
        if ( DRN_TOPOLOGY_PEER_TO_PEER == m_topology )
            SendConnectTo( link, *other );
    }

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNSession: Member " << newPeerId << " joined \"" << m_settings.sessionName << "\"" );
    EmitMemberEvent( MemberJoinedEvent, *member, 0 );
    for ( UInt32 i=0; i<DRN_MSGCLASS_COUNT; ++i )
    {
        if ( GUCEF_NULL != m_components[ i ] )
            m_components[ i ]->OnMemberJoined( *this, newPeerId );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    if ( DRN_MSGCLASS_SESSION == message.msgClass )
    {
        HandleSessionMessage( link, message );
        return;
    }
    if ( DRN_MSGCLASS_APP == message.msgClass )
    {
        HandleAppMessage( link, message );
        return;
    }
    if ( message.msgClass < DRN_MSGCLASS_COUNT )
    {
        CIDRNSessionComponent* component = m_components[ message.msgClass ];
        if ( GUCEF_NULL != component )
            component->OnSessionMessage( *this, *link, link->GetSessionPeerId(), message );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleSessionMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    CDRNWireReader reader( message.payload );
    switch ( message.opcode )
    {
        case DRN_SESSION_JOIN:              OnJoinRequest( link, message ); break;
        case DRN_SESSION_JOINED:            HandleJoined( link, reader ); break;
        case DRN_SESSION_MEMBER_LIST:       HandleMemberList( reader ); break;
        case DRN_SESSION_MEMBER_ADDED:      HandleMemberAdded( reader ); break;
        case DRN_SESSION_MEMBER_REMOVED:    HandleMemberRemoved( reader ); break;
        case DRN_SESSION_CONNECT_TO:        HandleConnectTo( reader ); break;
        case DRN_SESSION_LEAVE:             HandleLeave( link, reader ); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleJoined( CDRNPeerLink* link, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 result = DRN_JOIN_DENIED;
    CORE::CString reason;
    UInt32 peerId = 0;
    UInt32 flags = 0;
    UInt8 topology = 0;
    UInt32 sequencerPeerId = 0;
    UInt64 sessionEpoch = 0;
    reader.ReadVarUInt32( result );
    reader.ReadString( reason );
    bool complete = reader.ReadVarUInt32( peerId ) && reader.ReadVarUInt32( flags ) && reader.ReadUInt8( topology ) &&
                    reader.ReadVarUInt32( sequencerPeerId ) && reader.ReadVarUInt64( sessionEpoch );

    if ( 0 != ( flags & DRN_JOIN_FLAG_MESH ) )
    {
        // the answer of a member we linked with directly
        CMember* member = GetMember( link->GetSessionPeerId() );
        if ( GUCEF_NULL == member || !complete || DRN_JOIN_OK != result || peerId != member->peerId )
        {
            link->SetSessionBinding( GUCEF_NULL, 0 );
            link->Close( DRN_LINKREASON_CLOSED_BY_APP, "no direct link with the member" );
            return;
        }
        member->link = link->GetSharedPtr();
        return;
    }

    if ( SESSIONSTATE_JOINING != m_state || link != GetSequencerLink() )
        return;

    if ( DRN_JOIN_OK != result || !complete || 0 == peerId )
    {
        FailJoin( reason.IsNULLOrEmpty() ? CORE::CString( "the host refused" ) : reason );
        return;
    }

    m_localPeerId = peerId;
    m_topology = (TDRNTopology) topology;
    m_sequencerPeerId = sequencerPeerId;
    m_sessionEpoch = sessionEpoch;
    m_state = SESSIONSTATE_MEMBER;

    CMember* sequencer = GetOrAddMember( sequencerPeerId );
    BindLink( link, sequencer );
    CMember* self = GetOrAddMember( m_localPeerId );
    self->isLocal = true;
    self->known = true;
    self->nodeId = m_node->GetSettings().nodeId;
    self->endpoints = GetAdvertisedEndpoints();
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleMemberList( CDRNWireReader& reader )
{GUCEF_TRACE;

    if ( SESSIONSTATE_MEMBER != m_state )
        return;

    UInt32 count = 0;
    if ( !reader.ReadVarUInt32( count ) )
        return;

    GUCEF::vector< UInt32 > added;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt32 peerId = 0;
        CORE::CString nodeId;
        CORE::CString endpoints;
        if ( !reader.ReadVarUInt32( peerId ) || !reader.ReadString( nodeId ) || !reader.ReadString( endpoints ) )
            break;
        CMember* member = GetOrAddMember( peerId );
        if ( GUCEF_NULL == member || member->isLocal )
            continue;
        member->nodeId = nodeId;
        member->endpoints = endpoints;
        if ( !member->known )
        {
            member->known = true;
            added.push_back( peerId );
        }
    }

    for ( UInt32 i=0; i<added.size(); ++i )
    {
        CMember* member = GetMember( added[ i ] );
        if ( GUCEF_NULL == member )
            continue;
        EmitMemberEvent( MemberJoinedEvent, *member, 0 );
        for ( UInt32 c=0; c<DRN_MSGCLASS_COUNT; ++c )
        {
            if ( GUCEF_NULL != m_components[ c ] )
                m_components[ c ]->OnMemberJoined( *this, added[ i ] );
        }
    }

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNSession: Joined \"" << m_settings.sessionName << "\" as peer " << m_localPeerId );
    CORE::TCloneableUInt32 eventData( m_localPeerId );
    Emit( JoinedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleMemberAdded( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 peerId = 0;
    CORE::CString nodeId;
    CORE::CString endpoints;
    if ( !reader.ReadVarUInt32( peerId ) || !reader.ReadString( nodeId ) || !reader.ReadString( endpoints ) )
        return;
    if ( SESSIONSTATE_MEMBER != m_state || peerId == m_localPeerId )
        return;

    CMember* member = GetOrAddMember( peerId );
    member->nodeId = nodeId;
    member->endpoints = endpoints;
    if ( member->known )
        return;
    member->known = true;
    EmitMemberEvent( MemberJoinedEvent, *member, 0 );
    for ( UInt32 c=0; c<DRN_MSGCLASS_COUNT; ++c )
    {
        if ( GUCEF_NULL != m_components[ c ] )
            m_components[ c ]->OnMemberJoined( *this, peerId );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleMemberRemoved( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 peerId = 0;
    UInt32 reason = 0;
    if ( !reader.ReadVarUInt32( peerId ) )
        return;
    reader.ReadVarUInt32( reason );
    if ( SESSIONSTATE_MEMBER != m_state )
        return;
    RemoveMember( peerId, reason, false );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleConnectTo( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 peerId = 0;
    CORE::CString endpoints;
    if ( !reader.ReadVarUInt32( peerId ) || !reader.ReadString( endpoints ) )
        return;
    if ( SESSIONSTATE_MEMBER != m_state || DRN_TOPOLOGY_PEER_TO_PEER != m_topology || peerId == m_localPeerId )
        return;

    CMember* member = GetOrAddMember( peerId );
    if ( !member->link.IsNULL() || !member->connector.IsNULL() )
        return;
    member->endpoints = endpoints;

    CDRNEndpointList endpointList;
    endpointList.FromString( FilterEndpoints( endpoints ) );
    if ( endpointList.IsEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNSession: Member " << peerId << " has no endpoints, its messages go through the sequencer" );
        return;
    }

    member->connector = m_node->Connect( endpointList );
    m_observer.SubscribeTo( member->connector.GetPointerAlways() );
    if ( member->connector->IsDone() )
        OnConnectorDone( member->connector.GetPointerAlways(), member->connector->HasSucceeded() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleLeave( CDRNPeerLink* link, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 peerId = 0;
    UInt32 reason = 0;
    reader.ReadVarUInt32( peerId );
    reader.ReadVarUInt32( reason );

    UInt32 linkPeerId = link->GetSessionPeerId();
    if ( !IsSequencer() && link == GetSequencerLink() && linkPeerId == m_sequencerPeerId )
    {
        // the host ends the session
        CloseSession( DRN_LEAVE_SESSION_CLOSED, true );
        return;
    }
    if ( IsSequencer() )
    {
        RemoveMember( linkPeerId, DRN_LEAVE_LEFT, true );
        return;
    }

    // a member we have a direct link with leaves, the sequencer tells us about the membership
    CMember* member = GetMember( linkPeerId );
    if ( GUCEF_NULL != member && member->link.GetPointerAlways() == link )
    {
        link->SetSessionBinding( GUCEF_NULL, 0 );
        member->link.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::RemoveMember( UInt32 peerId, UInt32 reason, bool announce )
{GUCEF_TRACE;

    CMember* member = GetMember( peerId );
    if ( GUCEF_NULL == member || member->isLocal )
        return;

    m_members[ peerId ] = GUCEF_NULL;
    --m_memberCount;

    if ( !member->connector.IsNULL() )
    {
        m_observer.UnsubscribeFrom( *member->connector.GetPointerAlways() );
        member->connector->Cancel();
    }
    if ( !member->link.IsNULL() )
    {
        member->link->SetSessionBinding( GUCEF_NULL, 0 );
        if ( !member->link->IsClosed() )
            member->link->Close( DRN_LINKREASON_CLOSED_BY_APP, "left the session" );
    }

    if ( announce && IsSequencer() )
    {
        for ( UInt32 i=0; i<m_members.size(); ++i )
        {
            CMember* other = m_members[ i ];
            if ( GUCEF_NULL != other && !other->isLocal && !other->link.IsNULL() )
                SendMemberRemoved( other->link.GetPointerAlways(), peerId, reason );
        }
    }

    if ( GUCEF_NULL != m_latencyGrid && IsSequencer() )
        m_latencyGrid->RemoveKey( CORE::CVariant( peerId ) );

    if ( member->known )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNSession: Member " << peerId << " left \"" << m_settings.sessionName << "\"" );
        EmitMemberEvent( MemberLeftEvent, *member, reason );
        for ( UInt32 c=0; c<DRN_MSGCLASS_COUNT; ++c )
        {
            if ( GUCEF_NULL != m_components[ c ] )
                m_components[ c ]->OnMemberLeft( *this, peerId );
        }
    }
    GUCEF_DELETE member;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnLinkClosed( CDRNPeerLink* link )
{GUCEF_TRACE;

    UInt32 peerId = link->GetSessionPeerId();
    link->SetSessionBinding( GUCEF_NULL, 0 );

    if ( SESSIONSTATE_CLOSED == m_state || SESSIONSTATE_IDLE == m_state )
        return;

    if ( link == GetSequencerLink() )
    {
        if ( SESSIONSTATE_JOINING == m_state )
            FailJoin( "the link with the host closed" );
        else
            CloseSession( DRN_LEAVE_LINK_LOST, true );
        return;
    }

    CMember* member = GetMember( peerId );
    if ( GUCEF_NULL == member || member->link.GetPointerAlways() != link )
        return;

    if ( IsSequencer() )
        RemoveMember( peerId, DRN_LEAVE_LINK_LOST, true );
    else
        member->link.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::SendAppEnvelope( CDRNPeerLink* link, UInt32 fromPeer, UInt32 toPeer, UInt8 flags, UInt8 lane, const void* data, UInt32 dataSize )
{GUCEF_TRACE;
    UInt8 header[ 16 ];
    UInt32 headerSize = CDRNWireWriter::EncodeVarUInt64( fromPeer, header );
    headerSize += CDRNWireWriter::EncodeVarUInt64( toPeer, header + headerSize );
    header[ headerSize++ ] = flags;
    return link->SendGather( lane, DRN_MSGCLASS_APP, DRN_APP_MESSAGE, header, headerSize, data, dataSize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::SendAppMessage( UInt32 toPeer, UInt8 lane, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    return SendAppMessage( toPeer, lane, data.GetConstBufferPtr(), data.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::SendAppMessage( UInt32 toPeer, UInt8 lane, const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );

    if ( ( SESSIONSTATE_MEMBER != m_state && SESSIONSTATE_HOSTING != m_state ) || lane >= DRN_LANE_COUNT || toPeer == m_localPeerId )
        return false;

    if ( IsSequencer() )
    {
        if ( 0 == toPeer )
        {
            bool allSent = true;
            for ( UInt32 i=0; i<m_members.size(); ++i )
            {
                CMember* member = m_members[ i ];
                if ( GUCEF_NULL != member && !member->isLocal && !member->link.IsNULL() )
                    allSent = SendAppEnvelope( member->link.GetPointerAlways(), m_localPeerId, 0, 0, lane, data, dataSize ) && allSent;
            }
            return allSent;
        }
        CMember* member = GetMember( toPeer );
        if ( GUCEF_NULL == member || member->link.IsNULL() )
            return false;
        return SendAppEnvelope( member->link.GetPointerAlways(), m_localPeerId, toPeer, 0, lane, data, dataSize );
    }

    CDRNPeerLink* sequencerLink = GetSequencerLink();
    if ( GUCEF_NULL == sequencerLink )
        return false;

    if ( DRN_TOPOLOGY_CLIENT_SERVER == m_topology )
    {
        UInt8 flags = toPeer == m_sequencerPeerId ? 0 : DRN_APP_FLAG_FORWARD;
        return SendAppEnvelope( sequencerLink, m_localPeerId, toPeer, flags, lane, data, dataSize );
    }

    // peer to peer: directly where there is a link, through the sequencer where there is none yet
    if ( 0 == toPeer )
    {
        bool allSent = true;
        for ( UInt32 i=0; i<m_members.size(); ++i )
        {
            CMember* member = m_members[ i ];
            if ( GUCEF_NULL == member || member->isLocal || !member->known )
                continue;
            if ( !member->link.IsNULL() )
                allSent = SendAppEnvelope( member->link.GetPointerAlways(), m_localPeerId, 0, 0, lane, data, dataSize ) && allSent;
            else
                allSent = SendAppEnvelope( sequencerLink, m_localPeerId, member->peerId, DRN_APP_FLAG_FORWARD | DRN_APP_FLAG_TO_ALL, lane, data, dataSize ) && allSent;
        }
        return allSent;
    }

    CMember* member = GetMember( toPeer );
    if ( GUCEF_NULL == member || !member->known )
        return false;
    if ( !member->link.IsNULL() )
        return SendAppEnvelope( member->link.GetPointerAlways(), m_localPeerId, toPeer, 0, lane, data, dataSize );
    return SendAppEnvelope( sequencerLink, m_localPeerId, toPeer, DRN_APP_FLAG_FORWARD, lane, data, dataSize );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::HandleAppMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    if ( SESSIONSTATE_MEMBER != m_state && SESSIONSTATE_HOSTING != m_state )
        return;

    CDRNWireReader reader( message.payload );
    UInt32 fromPeer = 0;
    UInt32 toPeer = 0;
    UInt8 flags = 0;
    if ( !reader.ReadVarUInt32( fromPeer ) || !reader.ReadVarUInt32( toPeer ) || !reader.ReadUInt8( flags ) )
        return;
    const UInt8* data = reader.GetCurrentPtr();
    UInt32 dataSize = reader.GetRemaining();

    // only the sequencer passes on messages of others
    UInt32 linkPeerId = link->GetSessionPeerId();
    if ( linkPeerId != m_sequencerPeerId && fromPeer != linkPeerId )
        return;

    if ( 0 != ( flags & DRN_APP_FLAG_FORWARD ) && IsSequencer() )
    {
        UInt8 forwardFlags = (UInt8) ( flags & DRN_APP_FLAG_TO_ALL );
        if ( 0 == toPeer )
        {
            for ( UInt32 i=0; i<m_members.size(); ++i )
            {
                CMember* member = m_members[ i ];
                if ( GUCEF_NULL != member && !member->isLocal && member->peerId != fromPeer && !member->link.IsNULL() )
                    SendAppEnvelope( member->link.GetPointerAlways(), fromPeer, 0, forwardFlags, message.lane, data, dataSize );
            }
        }
        else
        if ( toPeer != m_localPeerId )
        {
            CMember* member = GetMember( toPeer );
            if ( GUCEF_NULL != member && !member->link.IsNULL() )
                SendAppEnvelope( member->link.GetPointerAlways(), fromPeer, toPeer, forwardFlags, message.lane, data, dataSize );
        }
    }

    if ( 0 == toPeer || toPeer == m_localPeerId )
        DeliverAppMessage( fromPeer, 0 != ( flags & DRN_APP_FLAG_TO_ALL ) ? 0 : toPeer, message.lane, data, dataSize );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::DeliverAppMessage( UInt32 fromPeer, UInt32 toPeer, UInt8 lane, const UInt8* data, UInt32 dataSize )
{GUCEF_TRACE;

    CDRNAppMessage& appMessage = m_appMessageEvent.GetData();
    appMessage.fromPeer = fromPeer;
    appMessage.toPeer = toPeer;
    appMessage.lane = lane;
    appMessage.payload.LinkTo( data, dataSize );
    Emit( AppMessageEvent, &m_appMessageEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::Tick( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( SESSIONSTATE_JOINING == m_state && nodeNow >= m_joinDeadline )
    {
        FailJoin( "joining took too long" );
        return;
    }
    if ( SESSIONSTATE_MEMBER != m_state && SESSIONSTATE_HOSTING != m_state )
        return;

    if ( m_settings.publishLatency && SESSIONSTATE_HOSTING == m_state )
        PublishLatency( nodeNow );

    for ( UInt32 i=0; i<DRN_MSGCLASS_COUNT; ++i )
    {
        if ( GUCEF_NULL != m_components[ i ] )
            m_components[ i ]->OnSessionTick( *this, nodeNow );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::PublishLatency( UInt64 nodeNow )
{GUCEF_TRACE;

    if ( nodeNow < m_nextLatencyPublish )
        return;
    m_nextLatencyPublish = nodeNow + (UInt64) m_settings.latencyPublishIntervalMs * 1000;

    if ( GUCEF_NULL == m_latencyGrid )
    {
        CDRNDataGridSettings gridSettings;
        gridSettings.name = LatencyGridName;
        gridSettings.authority = DRN_AUTHORITY_HOST;
        gridSettings.reliable = true;
        gridSettings.sendRateHz = 0;
        gridSettings.notify = DRN_NOTIFY_POLL;
        m_latencyGrid = m_replicator->CreateGrid( gridSettings );
        if ( GUCEF_NULL == m_latencyGrid )
            return;
    }

    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        CMember* member = m_members[ i ];
        if ( GUCEF_NULL == member || member->isLocal || member->link.IsNULL() )
            continue;
        CDRNLatencyInfo info;
        member->link->GetLatencyInfo( info );
        if ( 0 == info.sampleCount )
            continue;
        m_latencyGrid->SetValue( CORE::CVariant( member->peerId ), CORE::CVariant( info.smoothedRtt ) );
    }
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNSession::CreateGrid( const CDRNDataGridSettings& settings )
{GUCEF_TRACE;

    return m_replicator->CreateGrid( settings );
}

/*-------------------------------------------------------------------------*/

CDRNDataStream*
CDRNSession::CreateStream( const CDRNDataStreamSettings& settings )
{GUCEF_TRACE;

    return m_replicator->CreateStream( settings );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::Subscribe( const CORE::CString& itemName, UInt32 ownerPeerId )
{GUCEF_TRACE;

    return m_replicator->Subscribe( itemName, ownerPeerId );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::Unsubscribe( const CORE::CString& itemName, UInt32 ownerPeerId )
{GUCEF_TRACE;

    return m_replicator->Unsubscribe( itemName, ownerPeerId );
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNSession::FindGrid( const CORE::CString& itemName, UInt32 ownerPeerId ) const
{GUCEF_TRACE;

    return m_replicator->FindGrid( itemName, ownerPeerId );
}

/*-------------------------------------------------------------------------*/

CDRNDataStream*
CDRNSession::FindStream( const CORE::CString& itemName, UInt32 ownerPeerId ) const
{GUCEF_TRACE;

    return m_replicator->FindStream( itemName, ownerPeerId );
}

/*-------------------------------------------------------------------------*/

CDRNReplicator&
CDRNSession::GetReplicator( void ) const
{GUCEF_TRACE;

    return *m_replicator;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransferManager&
CDRNSession::GetBulkTransfers( void ) const
{GUCEF_TRACE;

    return *m_bulkTransfers;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::RegisterComponent( CIDRNSessionComponent* component )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    if ( GUCEF_NULL == component )
        return false;
    UInt8 msgClass = component->GetMessageClass();
    if ( msgClass >= DRN_MSGCLASS_COUNT || DRN_MSGCLASS_LINK == msgClass || DRN_MSGCLASS_SESSION == msgClass || DRN_MSGCLASS_APP == msgClass )
        return false;
    if ( GUCEF_NULL != m_components[ msgClass ] && m_components[ msgClass ] != component )
        return false;
    m_components[ msgClass ] = component;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::UnregisterComponent( CIDRNSessionComponent* component )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    for ( UInt32 i=0; i<DRN_MSGCLASS_COUNT; ++i )
    {
        if ( m_components[ i ] == component )
            m_components[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::SendToMember( UInt32 peerId, UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2 )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    CMember* member = GetMember( peerId );
    if ( GUCEF_NULL == member || member->link.IsNULL() )
        return false;
    return member->link->SendGather( lane, msgClass, opcode, part1, size1, part2, size2 );
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkPtr
CDRNSession::GetLinkToMember( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    CMember* member = GetMember( peerId );
    if ( GUCEF_NULL == member )
        return CDRNPeerLinkPtr();
    return member->link;
}

/*-------------------------------------------------------------------------*/

const CDRNAuthResult&
CDRNSession::GetMemberAuthResult( UInt32 peerId ) const
{GUCEF_TRACE;

    static const CDRNAuthResult noAuthResult;
    CMember* member = GetMember( peerId );
    if ( GUCEF_NULL == member || member->link.IsNULL() )
        return noAuthResult;
    return member->link->GetAuthResult();
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::GetTelemetry( CORE::CDataNode& telemetry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    telemetry.SetName( "DRNSessionTelemetry" );
    telemetry.SetAttribute( "sessionName", m_settings.sessionName );
    telemetry.SetAttribute( "localPeerId", m_localPeerId );
    telemetry.SetAttribute( "sequencerPeerId", m_sequencerPeerId );
    telemetry.SetAttribute( "nodeTime", CDRNClock::NowInMicroseconds() );
    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        const CMember* member = m_members[ i ];
        if ( GUCEF_NULL == member || member->isLocal || member->link.IsNULL() )
            continue;
        CORE::CDataNode* memberNode = telemetry.AddChild( "Member" );
        if ( GUCEF_NULL == memberNode )
            continue;
        memberNode->SetAttribute( "peerId", member->peerId );
        memberNode->SetAttribute( "nodeId", member->nodeId );
        member->link->GetTelemetry( *memberNode );
    }
}

/*-------------------------------------------------------------------------*/

CDRNSessionRecorder*
CDRNSession::GetRecorder( void ) const
{GUCEF_TRACE;

    return m_recorder;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::SetRecorder( CDRNSessionRecorder* recorder )
{GUCEF_TRACE;

    m_recorder = recorder;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::OnLinkTraffic( const CDRNPeerLink& link   ,
                            bool outgoing              ,
                            UInt8 lane                 ,
                            UInt8 msgClass             ,
                            UInt32 opcode              ,
                            const void* part1          ,
                            UInt32 size1               ,
                            const void* part2          ,
                            UInt32 size2               )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_recorder )
        m_recorder->OnTraffic( link, outgoing, lane, msgClass, opcode, part1, size1, part2, size2 );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNSession::GetSessionTime( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    UInt64 now = CDRNClock::NowInMicroseconds();
    if ( SESSIONSTATE_HOSTING == m_state )
        return now >= m_sessionEpoch ? now - m_sessionEpoch : 0;
    CDRNPeerLink* sequencerLink = GetSequencerLink();
    if ( SESSIONSTATE_MEMBER == m_state && GUCEF_NULL != sequencerLink )
    {
        UInt64 remoteNow = sequencerLink->GetEstimatedRemoteTime( now );
        return remoteNow >= m_sessionEpoch ? remoteNow - m_sessionEpoch : 0;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CDRNSession::TSessionState
CDRNSession::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::IsActive( void ) const
{GUCEF_TRACE;

    return SESSIONSTATE_MEMBER == m_state || SESSIONSTATE_HOSTING == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::IsSequencer( void ) const
{GUCEF_TRACE;

    return SESSIONSTATE_HOSTING == m_state;
}

/*-------------------------------------------------------------------------*/

TDRNTopology
CDRNSession::GetTopology( void ) const
{GUCEF_TRACE;

    return m_topology;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSession::GetLocalPeerId( void ) const
{GUCEF_TRACE;

    return m_localPeerId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSession::GetSequencerPeerId( void ) const
{GUCEF_TRACE;

    return m_sequencerPeerId;
}

/*-------------------------------------------------------------------------*/

const CDRNSessionSettings&
CDRNSession::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSession::GetName( void ) const
{GUCEF_TRACE;

    return m_settings.sessionName;
}

/*-------------------------------------------------------------------------*/

void
CDRNSession::GetMembers( TDRNSessionMemberInfoVector& members ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    members.clear();
    for ( UInt32 i=0; i<m_members.size(); ++i )
    {
        const CMember* member = m_members[ i ];
        if ( GUCEF_NULL == member || !member->known )
            continue;
        CDRNSessionMemberInfo info;
        info.peerId = member->peerId;
        info.nodeId = member->nodeId;
        info.endpoints.FromString( member->endpoints );
        info.isLocal = member->isLocal;
        info.isSequencer = member->peerId == m_sequencerPeerId;
        info.hasDirectLink = !member->link.IsNULL();
        members.push_back( info );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSession::GetMemberCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_members.size(); ++i )
        if ( GUCEF_NULL != m_members[ i ] && m_members[ i ]->known )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSession::HasMember( UInt32 peerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    const CMember* member = GetMember( peerId );
    return GUCEF_NULL != member && member->known;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSession::GetDirectLinkCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_members.size(); ++i )
        if ( GUCEF_NULL != m_members[ i ] && !m_members[ i ]->isLocal && !m_members[ i ]->link.IsNULL() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

CDRNNode&
CDRNSession::GetNode( void ) const
{GUCEF_TRACE;

    return *m_node;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSession::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNSession";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
