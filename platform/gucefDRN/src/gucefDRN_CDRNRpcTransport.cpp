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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#include "gucefDRN_CDRNAnonymousAuthenticator.h"
#define GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNANONYMOUSAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H
#include "gucefDRN_CDRNRpcSessionComponent.h"
#define GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H
#endif /* GUCEF_DRN_CDRNRPCSESSIONCOMPONENT_H ? */

#include "gucefDRN_CDRNRpcTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNRpcTransport::TransportType = "drn";
const CORE::CString CDRNRpcTransport::UriScheme = "drn";

/** Spare frame buffers kept per transport, and the largest one kept */
static const UInt32 MaxFreeFrameBuffers = 64;
static const UInt32 MaxFreeFrameBufferSize = 64 * 1024;

/*-------------------------------------------------------------------------*/

CDRNRpcTransport::CDRNRpcTransport( void )
    : COMCORE::CIRpcTransport()
    , CORE::CTSharedObjCreator< CDRNRpcTransport, MT::CMutex >( this )
    , m_notifierLock()
    , m_lock()
    , m_queue()
    , m_queueProtection()
    , m_free()
    , m_sending()
    , m_sendingProtection()
    , m_connected( false )
    , m_closed( false )
    , m_closeRequested( false )
    , m_disconnectPending( false )
    , m_connectedPending( false )
    , m_disconnectRaised( false )
    , m_disconnectStatus()
    , m_connectStatus()
    , m_settings()
    , m_session( GUCEF_NULL )
    , m_node( GUCEF_NULL )
    , m_component( GUCEF_NULL )
    , m_link()
    , m_peerId( 0 )
    , m_channel( 0 )
    , m_openedLocally( true )
    , m_sendOpcode( 0 )
    , m_pulseGenerator()
    , m_pulseObserver()
    , m_properties()
    , m_principal( TransportType, CORE::CString::Empty, false )
    , m_remoteUri()
{GUCEF_TRACE;

    m_properties.isReliable = true;
    m_properties.isOrdered = true;
    m_properties.latencyClass = COMCORE::CRpcTransportProperties::LATENCY_LAN;
    m_properties.profileName = TransportType;
    m_properties.maxFrameSize = m_settings.maxFrameSize;
}

/*-------------------------------------------------------------------------*/

CDRNRpcTransport::CDRNRpcTransport( CDRNSession& session, UInt32 peerId, const CDRNRpcTransportSettings& settings )
    : COMCORE::CIRpcTransport()
    , CORE::CTSharedObjCreator< CDRNRpcTransport, MT::CMutex >( this )
    , m_notifierLock()
    , m_lock()
    , m_queue()
    , m_queueProtection()
    , m_free()
    , m_sending()
    , m_sendingProtection()
    , m_connected( false )
    , m_closed( false )
    , m_closeRequested( false )
    , m_disconnectPending( false )
    , m_connectedPending( false )
    , m_disconnectRaised( false )
    , m_disconnectStatus()
    , m_connectStatus()
    , m_settings( settings )
    , m_session( &session )
    , m_node( GUCEF_NULL )
    , m_component( GUCEF_NULL )
    , m_link()
    , m_peerId( peerId )
    , m_channel( 0 )
    , m_openedLocally( true )
    , m_sendOpcode( 0 )
    , m_pulseGenerator()
    , m_pulseObserver()
    , m_properties()
    , m_principal( TransportType, CORE::CString::Empty, false )
    , m_remoteUri()
{GUCEF_TRACE;

    m_properties.isReliable = true;
    m_properties.isOrdered = true;
    m_properties.latencyClass = COMCORE::CRpcTransportProperties::LATENCY_LAN;
    m_properties.profileName = TransportType;
    m_properties.maxFrameSize = m_settings.maxFrameSize;
}

/*-------------------------------------------------------------------------*/

CDRNRpcTransport::~CDRNRpcTransport()
{GUCEF_TRACE;

    m_pulseObserver.UnsubscribeAllFromObserver();

    // the peer hears that the channel is gone, unless it went away first
    Detach( true );

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
    for ( UInt32 i=0; i<m_sending.size(); ++i )
        GUCEF_DELETE m_sending[ i ];
    m_sending.clear();
    for ( UInt32 i=0; i<m_free.size(); ++i )
        GUCEF_DELETE m_free[ i ];
    m_free.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::SetSettings( const CDRNRpcTransportSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
    if ( !m_connected )
        m_properties.maxFrameSize = m_settings.maxFrameSize;
}

/*-------------------------------------------------------------------------*/

const CDRNRpcTransportSettings&
CDRNRpcTransport::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcStatus
CDRNRpcTransport::GetConnectStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_connectStatus;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRpcTransport::GetPeerId( void ) const
{GUCEF_TRACE;

    return m_peerId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRpcTransport::GetChannel( void ) const
{GUCEF_TRACE;

    return m_channel;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNRpcTransport::GetTransportType( void ) const
{GUCEF_TRACE;

    return TransportType;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::IsOpenedLocally( void ) const
{GUCEF_TRACE;

    return m_openedLocally;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::DescribeLink( const CDRNSession& session, UInt32 peerId, const CDRNPeerLink& link, const CDRNRpcTransportSettings& settings,
                                COMCORE::CRpcPrincipal& principal, COMCORE::CRpcTransportProperties& properties )
{GUCEF_TRACE;

    const CDRNAuthResult& auth = link.GetAuthResult();

    // the side that accepted the link checked the peer's proof with the node's authenticator; the side that connected
    // knows the peer by the static key it proved in the handshake, which identifies the peer only when the endpoint
    // pinned that key: otherwise the key and the node id are the peer's own word
    bool acceptedLink = !link.IsInitiator();
    const UInt8* peerStaticKey = link.GetPeerStaticKey();
    principal = COMCORE::CRpcPrincipal();
    principal.transport = TransportType;
    if ( !auth.userId.IsNULLOrEmpty() )
        principal.subject = "drn:" + auth.userId;
    else
        principal.subject = "drn-node:" + link.GetRemoteNodeId();
    if ( acceptedLink )
    {
        const CORE::CString& method = session.GetNode().GetSettings().auth.authenticatorType;
        principal.isAuthenticated = !method.IsNULLOrEmpty() && !method.Equals( CDRNAnonymousAuthenticator::MethodName, false );
    }
    else
    {
        principal.isAuthenticated = GUCEF_NULL != peerStaticKey && link.HasPinnedKey();
    }

    // one role string per authentication, several roles comma separated
    CORE::CString::StringVector roles = auth.role.ParseElements( ',', false );
    for ( UInt32 i=0; i<roles.size(); ++i )
    {
        CORE::CString role = roles[ i ].Trim( true ).Trim( false );
        if ( !role.IsNULLOrEmpty() )
            principal.roles.insert( role );
    }

    principal.attributes.Set( "drn.session", CORE::CVariant( session.GetName() ) );
    principal.attributes.Set( "drn.peerId", CORE::CVariant( peerId ) );
    principal.attributes.Set( "drn.nodeId", CORE::CVariant( link.GetRemoteNodeId() ) );
    if ( acceptedLink && auth.publicKey.GetDataSize() > 0 )
        principal.attributes.Set( "drn.publicKey", CORE::CVariant( CDRNCrypto::ToHex( auth.publicKey.GetConstBufferPtr(), auth.publicKey.GetDataSize() ) ) );
    else
    if ( !acceptedLink && GUCEF_NULL != peerStaticKey )
        principal.attributes.Set( "drn.publicKey", CORE::CVariant( CDRNCrypto::ToHex( peerStaticKey, DRN_HANDSHAKE_KEY_SIZE ) ) );
    if ( !auth.accounts.empty() )
    {
        CORE::CString accounts;
        for ( UInt32 i=0; i<auth.accounts.size(); ++i )
        {
            if ( i > 0 )
                accounts += ',';
            accounts += auth.accounts[ i ];
        }
        principal.attributes.Set( "drn.accounts", CORE::CVariant( accounts ) );
    }
    principal.attributes.Set( "drn.protected", CORE::CVariant( link.IsProtected() ) );
    if ( link.IsProtected() )
        principal.attributes.Set( "drn.cipher", CORE::CVariant( link.GetProtection().GetCipher() ) );

    properties.isReliable = true;
    properties.isOrdered = true;
    properties.isSecure = link.IsProtected();
    properties.hasFrameProtectionClasses = link.IsProtected();
    properties.latencyClass = COMCORE::CRpcTransportProperties::LATENCY_LAN;
    properties.profileName = TransportType;

    // a datagram channel would split a larger message into link fragments: RPC fragments instead (design 6)
    UInt32 maxFrameSize = 0 != settings.maxFrameSize ? settings.maxFrameSize : 16 * 1024;
    CIDRNChannelPtr channel = link.GetChannel();
    if ( !channel.IsNULL() && channel->GetCapabilities().isDatagram )
    {
        UInt32 unfragmented = link.GetMaxUnfragmentedPayload();
        if ( 0 != unfragmented && unfragmented < maxFrameSize )
            maxFrameSize = unfragmented;
    }
    properties.maxFrameSize = maxFrameSize;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::Fail( Int32 statusCode, const CORE::CString& why )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_connectStatus.Set( (COMCORE::TRpcStatusCode) statusCode, why );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcTransport: Cannot connect to member " << m_peerId << ": " << m_connectStatus.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::Connect( const CORE::CUri& remote )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_connected || m_closed )
            return false;
    }

    CDRNSession* session = m_session;
    UInt32 peerId = m_peerId;
    if ( !remote.GetScheme().IsNULLOrEmpty() || !remote.GetAuthority().IsNULLOrEmpty() )
    {
        // drn://<session name>/<peer id>
        if ( !remote.GetScheme().Equals( UriScheme, false ) )
        {
            Fail( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "not a drn URI: " + remote.ToUriInStringForm() );
            return false;
        }
        CORE::CString path = remote.GetPath();
        while ( !path.IsNULLOrEmpty() && '/' == path[ 0 ] )
            path = path.CutChars( 1, true, 0 );
        peerId = CORE::StringToUInt32( path, 0 );
        CORE::CString sessionName = remote.GetAuthorityHost().IsNULLOrEmpty() ? remote.GetAuthority() : remote.GetAuthorityHost();
        session = CDRNGlobal::Instance()->GetSessionRegistry().Find( sessionName );
        if ( GUCEF_NULL == session )
        {
            Fail( COMCORE::RPC_STATUS_NOT_FOUND, "no session \"" + sessionName + "\" is registered in CDRNGlobal::GetSessionRegistry()" );
            return false;
        }
    }
    if ( GUCEF_NULL == session || 0 == peerId )
    {
        Fail( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "no session and member to connect to" );
        return false;
    }
    return ConnectTo( *session, peerId );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::ConnectTo( CDRNSession& session, UInt32 peerId )
{GUCEF_TRACE;

    MT::CScopeMutex nodeLock( session.GetNode().GetLock() );

    if ( !session.IsActive() )
    {
        Fail( COMCORE::RPC_STATUS_UNAVAILABLE, "the session \"" + session.GetName() + "\" is not active" );
        return false;
    }
    if ( peerId == session.GetLocalPeerId() )
    {
        Fail( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "a member does not connect to itself" );
        return false;
    }

    // direct links only (design 4.3): a frame forwarded by the sequencer would be decrypted there
    CDRNPeerLinkPtr link = session.GetLinkToMember( peerId );
    if ( link.IsNULL() || !link->IsOperational() )
    {
        Fail( COMCORE::RPC_STATUS_UNAVAILABLE, "no direct link to member " + CORE::ToString( peerId ) );
        return false;
    }
    if ( link->GetRemoteProtocolMinor() < DRN_PROTOCOL_MINOR_RPC )
    {
        Fail( COMCORE::RPC_STATUS_UNAVAILABLE, "the member's node has no RPC (protocol 2." + CORE::ToString( (UInt32) link->GetRemoteProtocolMinor() ) + ")" );
        return false;
    }
    if ( m_settings.requireProtectedLink && !link->IsProtected() )
    {
        Fail( COMCORE::RPC_STATUS_PERMISSION_DENIED, "the link with the member has no packet protection" );
        return false;
    }
    CDRNRpcSessionComponent* component = CDRNRpcSessionComponent::Obtain( session );
    if ( GUCEF_NULL == component )
    {
        Fail( COMCORE::RPC_STATUS_FAILED_PRECONDITION, "the session has another component for the RPC message class" );
        return false;
    }

    UInt32 channel = component->OpenChannel( peerId, this );
    Attach( *component, link, peerId, channel, true );

    // the member's listener accepts the channel on OPEN, before the first frame
    UInt8 open[ 8 ];
    UInt32 openSize = CDRNWireWriter::EncodeVarUInt64( channel, open );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RPC, DRN_RPC_OPCODE_OPEN, open, openSize );
    if ( component->IsDelivering() )
        component->FlushAfterDelivery( *link );
    else
        link->Flush();
    {
        MT::CScopeMutex lock( m_lock );
        m_remoteUri = CORE::CUri( UriScheme + "://" + session.GetName() + "/" + CORE::ToString( peerId ) );
        m_connectStatus = COMCORE::CRpcStatus::Ok;
        m_connectedPending = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::Attach( CDRNRpcSessionComponent& component, const CDRNPeerLinkPtr& link, UInt32 peerId, UInt32 channel, bool openedLocally )
{GUCEF_TRACE;

    COMCORE::CRpcPrincipal principal;
    COMCORE::CRpcTransportProperties properties;
    DescribeLink( component.GetSession(), peerId, *link, m_settings, principal, properties );
    {
        MT::CScopeMutex lock( m_lock );
        m_session = &component.GetSession();
        m_node = &component.GetNode();
        m_component = &component;
        m_link = link;
        m_peerId = peerId;
        m_channel = channel;
        m_openedLocally = openedLocally;
        m_sendOpcode = DRNRpcOpcode( channel, !openedLocally );
        m_principal = principal;
        m_properties = properties;
        m_pulseGenerator = m_node->GetPulseGenerator();
        m_connected = true;
        if ( !openedLocally )
            m_remoteUri = CORE::CUri( UriScheme + "://" + m_session->GetName() + "/" + CORE::ToString( peerId ) );
    }
    SubscribeToPulses( component.GetNode() );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::SubscribeToPulses( CDRNNode& node )
{GUCEF_TRACE;

    // node lock held: the transport is one of the node's objects and works on the node's own pulse generator, which is
    // pulsed under the node lock (gucefDRN design 9.1), so its events are raised under the node lock in both threading
    // modes. A subscription to the application's pulse generator here would take its notification lock under the node
    // lock, against the order
    CORE::PulseGeneratorPtr workPulseGenerator = node.GetWorkPulseGenerator();
    if ( workPulseGenerator.IsNULL() )
        return;
    TEventCallback callback( this, &CDRNRpcTransport::OnPulse );
    m_pulseObserver.SubscribeTo( workPulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::IsOnNodeThread( void ) const
{GUCEF_TRACE;

    // the thread that works on the node right now, under the node lock (CDRNNode::IsWorkingOnThisThread()): the only
    // thread that may take the node lock under the endpoint lock, since it holds it already. The thread of the pulse
    // generator outside the node's work (the application's thread that drives it, calling the endpoint) is not
    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL != node )
        return node->IsWorkingOnThisThread();

    // let go of by the node: the thread of the pulse generator it worked on, which takes no node lock here any more
    return !m_pulseGenerator.IsNULL() && m_pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentTaskID();
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_connected && !m_closed;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::SendFrame( const void* frame, UInt32 size, COMCORE::TRpcProtectionClass protection )
{GUCEF_TRACE;

    if ( GUCEF_NULL == frame || 0 == size )
        return false;

    // on the node thread with nothing queued by other threads the frame leaves at once, without a copy into the queue
    const bool onNodeThread = IsOnNodeThread();
    if ( onNodeThread )
    {
        CDRNNode* node = GUCEF_NULL;
        {
            MT::CScopeMutex lock( m_lock );
            if ( !m_connected || m_closed )
                return false;
            if ( m_queue.empty() )
                node = m_node;
        }
        if ( GUCEF_NULL != node )
        {
            MT::CScopeMutex nodeLock( node->GetLock() );
            bool direct = false;
            {
                MT::CScopeMutex lock( m_lock );
                direct = m_queue.empty() && GUCEF_NULL != m_component;
            }
            if ( direct )
            {
                SendNow( static_cast< const UInt8* >( frame ), size, (UInt8) protection );
                FlushLink();
                return true;
            }
        }
    }

    CORE::PulseGeneratorPtr pulseGenerator;
    {
        // the leaf lock only: callers may hold the endpoint lock
        MT::CScopeMutex lock( m_lock );
        if ( !m_connected || m_closed )
            return false;
        CORE::CDynamicBuffer* buffer = GUCEF_NULL;
        if ( !m_free.empty() )
        {
            buffer = m_free.back();
            m_free.pop_back();
        }
        else
        {
            buffer = GUCEF_NEW CORE::CDynamicBuffer();
        }
        buffer->SetDataSize( 0 );
        buffer->CopyFrom( size, frame );
        m_queue.push_back( buffer );
        m_queueProtection.push_back( (UInt8) protection );

        // a node on a thread of its own sends it in a cycle that starts now (the node outlives the pointer, which is
        // cleared under this lock), a node on the application's pulse generator at its next pulse, asked for below
        if ( !onNodeThread && GUCEF_NULL != m_node )
        {
            m_node->WakeOwnThread();
            if ( !m_node->IsRunningOwnThread() )
                pulseGenerator = m_pulseGenerator;
        }
    }

    // through the driver: it takes only the data lock of the pulse generator, a leaf, and never pulses on this thread
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->RequestImmediatePulseThroughDriver();

    // in queue order: frames queued by other threads before this one leave first. Elsewhere the next pulse of the node
    // sends them: on a node thread of its own the cycle that the wake starts, else the application's next pulse, asked
    // for through the driver of the node's pulse generator (CPulseGenerator::RequestImmediatePulse() is not used: without
    // a driver, or with one that does not loop, it pulses on the calling thread, under the endpoint lock the caller of
    // SendFrame() may hold)
    if ( onNodeThread )
        Drain();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::Drain( void )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_queue.empty() )
            return;
        node = m_node;
        if ( GUCEF_NULL == node || GUCEF_NULL == m_component )
        {
            // the channel is gone: nothing more leaves
            for ( UInt32 i=0; i<m_queue.size(); ++i )
                m_free.push_back( m_queue[ i ] );
            m_queue.clear();
            m_queueProtection.clear();
            return;
        }
    }

    MT::CScopeMutex nodeLock( node->GetLock() );
    {
        MT::CScopeMutex lock( m_lock );
        m_sending.swap( m_queue );
        m_sendingProtection.swap( m_queueProtection );
    }
    for ( UInt32 i=0; i<m_sending.size(); ++i )
    {
        CORE::CDynamicBuffer* buffer = m_sending[ i ];
        SendNow( static_cast< const UInt8* >( buffer->GetConstBufferPtr() ), buffer->GetDataSize(), m_sendingProtection[ i ] );
    }

    FlushLink();

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_sending.size(); ++i )
    {
        if ( m_free.size() < MaxFreeFrameBuffers && m_sending[ i ]->GetBufferSize() <= MaxFreeFrameBufferSize )
            m_free.push_back( m_sending[ i ] );
        else
            GUCEF_DELETE m_sending[ i ];
    }
    m_sending.clear();
    m_sendingProtection.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::FlushLink( void )
{GUCEF_TRACE;

    // node lock held, under which alone the component and the link change. Whoever is delivering a received frame
    // flushes the link once it is done, otherwise the frames leave now
    CDRNRpcSessionComponent* component = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        component = m_component;
    }
    CDRNPeerLink* link = m_link.GetPointerAlways();
    if ( GUCEF_NULL != component && GUCEF_NULL != link )
    {
        if ( component->IsDelivering() )
            component->FlushAfterDelivery( *link );
        else
            link->Flush();
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::SendNow( const UInt8* frame, UInt32 size, UInt8 protection )
{GUCEF_TRACE;

    // node lock held, under which alone the component and the link change
    bool attached = false;
    {
        MT::CScopeMutex lock( m_lock );
        attached = GUCEF_NULL != m_component;
    }
    CDRNPeerLink* link = m_link.GetPointerAlways();
    if ( !attached || GUCEF_NULL == link || link->IsClosed() )
        return false;

    // a frame of a PUBLIC method goes in a MAC packet where the policy allows public items (the link decides)
    UInt8 packetKind = COMCORE::RPC_PROTECTION_PUBLIC == protection ? (UInt8) DRN_PACKETKIND_MAC : (UInt8) DRN_PACKETKIND_AEAD;
    return link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RPC, m_sendOpcode, frame, size, packetKind );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::OnProtectionDowngradeRefused( COMCORE::TRpcProtectionClass required, COMCORE::TRpcProtectionClass arrivedWith )
{GUCEF_TRACE;

    // the RPC session refused a frame of a method whose class is stronger than the packet it came in: DRN reports it
    // as it reports the refusals of other components (gucefDRN_ItemProtection_Design.md 6). On the protocol thread.
    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL == node )
        return;
    MT::CScopeMutex nodeLock( node->GetLock() );
    if ( m_link.IsNULL() )
        return;
    UInt8 packetKind = COMCORE::RPC_PROTECTION_PUBLIC == arrivedWith ? (UInt8) DRN_PACKETKIND_MAC : (UInt8) DRN_PACKETKIND_AEAD;
    m_link->RefuseDowngrade( DRN_MSGCLASS_RPC, m_sendOpcode ^ DRN_RPC_OPCODE_FROM_ACCEPTOR, packetKind,
                             COMCORE::RPC_PROTECTION_CONFIDENTIAL == required ?
                                "the RPC session refused a frame of a CONFIDENTIAL method that came in a public packet" :
                                "the RPC session refused a frame below the protection of its method" );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::OnFrame( CDRNPeerLink& /* link */, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    // node thread, node lock held
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_connected || m_closed )
            return;
    }
    if ( 0 == message.payload.GetDataSize() )
    {
        OnLinkGone( COMCORE::RPC_STATUS_UNAVAILABLE, "the member closed the RPC channel" );
        return;
    }

    // frames queued by other threads go first, so that answers keep their order
    Drain();

    COMCORE::TRpcProtectionClass protection = DRN_PACKETKIND_AEAD == message.packetKind ? COMCORE::RPC_PROTECTION_CONFIDENTIAL : COMCORE::RPC_PROTECTION_PUBLIC;
    if ( !m_properties.hasFrameProtectionClasses )
        protection = COMCORE::RPC_PROTECTION_CONFIDENTIAL;
    COMCORE::CRpcFrameEventData data( message.payload.GetConstBufferPtr(), message.payload.GetDataSize(), protection );
    NotifyObservers( FrameReceivedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::OnLinkGone( Int32 statusCode, const CORE::CString& why )
{GUCEF_TRACE;

    // node lock held; the component forgot the channel already. The link goes in every case, under the node lock: the
    // last reference of a link lets go of its channel, whose locks are taken under the node lock only
    bool disconnect = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_component = GUCEF_NULL;
        m_node = GUCEF_NULL;
        m_session = GUCEF_NULL;
        if ( m_connected && !m_disconnectPending && !m_disconnectRaised )
        {
            disconnect = true;
            m_connected = false;
            m_disconnectPending = true;
            m_disconnectStatus.Set( (COMCORE::TRpcStatusCode) statusCode, why );
        }
    }
    m_link.Unlink();
    if ( !disconnect )
        return;

    // the endpoint suspends the session on the node thread now, or at the next pulse of it
    if ( IsOnNodeThread() )
        RaiseDisconnected();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::OnComponentGone( void )
{GUCEF_TRACE;

    OnLinkGone( COMCORE::RPC_STATUS_UNAVAILABLE, "the session is gone" );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::RaiseDisconnected( void )
{GUCEF_TRACE;

    COMCORE::CRpcStatus status;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_disconnectPending || m_disconnectRaised )
            return;
        m_disconnectPending = false;
        m_disconnectRaised = true;
        m_connectedPending = false;
        status = m_disconnectStatus;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNRpcTransport: RPC channel " << m_channel << " with member " << m_peerId << " disconnected: " << status.ToString() );
    NotifyObservers( DisconnectedEvent, &status );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::Detach( bool sendClose )
{GUCEF_TRACE;

    CDRNNode* node = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        node = m_node;
    }
    if ( GUCEF_NULL == node )
        return;

    MT::CScopeMutex nodeLock( node->GetLock() );
    CDRNRpcSessionComponent* component = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        component = m_component;
        m_component = GUCEF_NULL;
        m_node = GUCEF_NULL;
        m_session = GUCEF_NULL;
    }
    if ( GUCEF_NULL == component )
        return;
    component->RemoveTransport( this );
    if ( sendClose && !m_link.IsNULL() && !m_link->IsClosed() )
    {
        static const UInt8 noPayload = 0;
        m_link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RPC, m_sendOpcode, &noPayload, 0 );
        if ( component->IsDelivering() )
            component->FlushAfterDelivery( *m_link );
        else
            m_link->Flush();
    }
    m_link.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::Close( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_closed )
            return;
        m_closed = true;
        m_closeRequested = true;
        if ( GUCEF_NULL != m_node )
            m_node->WakeOwnThread();
    }

    // the close and DisconnectedEvent happen on the node thread at its next pulse, never inside this call: the endpoint
    // calls Close() while it holds its lock and is in the middle of ending the session
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcTransport::OnPulse( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventId */, CORE::CICloneable* /* eventData */ )
{GUCEF_TRACE;

    bool hasQueue = false;
    bool closeRequested = false;
    bool connectedPending = false;
    bool disconnectPending = false;
    {
        MT::CScopeMutex lock( m_lock );
        hasQueue = !m_queue.empty();
        closeRequested = m_closeRequested;
        connectedPending = m_connectedPending;
        disconnectPending = m_disconnectPending;
        m_closeRequested = false;
    }
    if ( !hasQueue && !closeRequested && !connectedPending && !disconnectPending )
        return;

    if ( hasQueue )
        Drain();

    if ( connectedPending )
    {
        bool raise = false;
        {
            MT::CScopeMutex lock( m_lock );
            raise = m_connectedPending && m_connected && !m_closed;
            m_connectedPending = false;
        }
        if ( raise )
            NotifyObservers( ConnectedEvent );
    }

    if ( closeRequested )
    {
        // what the endpoint sent before it closed (a GOODBYE, an ABORT) leaves first, then the close of the channel
        Drain();
        Detach( true );
        MT::CScopeMutex lock( m_lock );
        if ( !m_disconnectRaised && !m_disconnectPending )
        {
            m_connected = false;
            m_disconnectPending = true;
            m_disconnectStatus.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the transport was closed" );
        }
    }

    RaiseDisconnected();
}

/*-------------------------------------------------------------------------*/

const COMCORE::CRpcTransportProperties&
CDRNRpcTransport::GetProperties( void ) const
{GUCEF_TRACE;

    return m_properties;
}

/*-------------------------------------------------------------------------*/

const COMCORE::CRpcPrincipal&
CDRNRpcTransport::GetPeerPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNRpcTransport::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CDRNRpcTransport::GetRemoteUri( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_remoteUri;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::SetPulseGenerator( const CORE::PulseGeneratorPtr& /* pulseGenerator */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransport::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CDRNRpcTransportSettings settings;
    if ( !settings.LoadConfig( config ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNRpcTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNRpcTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDRNRpcTransport::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notifierLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDRNRpcTransport::Unlock( void ) const
{GUCEF_TRACE;

    return m_notifierLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
