/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CRemoteServiceChannel.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CLONEABLES_H
#endif /* GUCEF_CLONEABLES_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#include "gucefCOMCORE_CRpcEndpointSettings.h"
#define GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#endif /* GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNLATENCYINFO_H
#include "gucefDRN_CDRNLatencyInfo.h"
#define GUCEF_DRN_CDRNLATENCYINFO_H
#endif /* GUCEF_DRN_CDRNLATENCYINFO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteServiceChannel::SSettings::SSettings( void )
    : nodeName( "client" )
    , linkTimeoutMs( 15000 )
    , echoIntervalMs( 1000 )
    , resumeGraceMs( 600000 )
    , receiveWindowBytes( 1024 * 1024 )
    , maxMessageBytes( 4 * 1024 * 1024 )
    , sendQueueBytes( 4 * 1024 * 1024 )
    , wantedContracts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char*
CRemoteServiceChannel::ChannelStateToString( TChannelState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case CHANNEL_IDLE: return "IDLE";
        case CHANNEL_JOINING: return "JOINING";
        case CHANNEL_BINDING: return "BINDING";
        case CHANNEL_READY: return "READY";
        case CHANNEL_DOWN: return "DOWN";
        case CHANNEL_REFUSED: return "REFUSED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceChannel::IsPermanentRefusal( DRN::TDRNLinkReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case DRN::DRN_LINKREASON_PROTOCOL_VERSION:
        case DRN::DRN_LINKREASON_SERVICE_MISMATCH:
        case DRN::DRN_LINKREASON_AUTH_METHOD:
        case DRN::DRN_LINKREASON_AUTH_FAILED:
        case DRN::DRN_LINKREASON_BANNED:
        case DRN::DRN_LINKREASON_POLICY:
        case DRN::DRN_LINKREASON_KEY_MISMATCH:
        case DRN::DRN_LINKREASON_REFUSED_BY_PEER:      // a REFUSE in the handshake: the proof, a ban or the policy; limits close without a reply
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CRemoteServiceChannel::CRemoteServiceChannel( const CORE::PulseGeneratorPtr& pulseGenerator, UInt32 index, CRemoteServiceConnection* owner )
    : CORE::CObservingNotifier()
    , m_pulse( pulseGenerator )
    , m_index( index )
    , m_owner( owner )
    , m_settings()
    , m_drnSessionName()
    , m_node( GUCEF_NULL )
    , m_session( GUCEF_NULL )
    , m_transport()
    , m_endpoint()
    , m_state( CHANNEL_IDLE )
    , m_rpcResumed( false )
    , m_rpcNewSession( false )
    , m_awaitingRebind( false )
    , m_rpcClosed( false )
    , m_joinFailed( false )
    , m_stepStartedMs( 0 )
    , m_lastReason( DRN::DRN_LINKREASON_NONE )
    , m_lastText()
    , m_links( 0 )
    , m_rpcResumes( 0 )
    , m_newRpcSessions( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteServiceChannel::~CRemoteServiceChannel()
{GUCEF_TRACE;

    Shutdown( false );
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteServiceChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteServiceChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CRemoteServiceChannel::TChannelState CRemoteServiceChannel::GetState( void ) const { return m_state; }
UInt32 CRemoteServiceChannel::GetIndex( void ) const { return m_index; }
COMCORE::CRpcEndpointPtr CRemoteServiceChannel::GetEndpoint( void ) const { return m_endpoint; }
DRN::CDRNSession* CRemoteServiceChannel::GetDrnSession( void ) const { return m_session; }
bool CRemoteServiceChannel::WasRpcResumed( void ) const { return m_rpcResumed; }
bool CRemoteServiceChannel::HasNewRpcSession( void ) const { return m_rpcNewSession; }
void CRemoteServiceChannel::ClearNewRpcSession( void ) { m_rpcNewSession = false; }
bool CRemoteServiceChannel::IsRpcClosed( void ) const { return m_rpcClosed; }
DRN::TDRNLinkReason CRemoteServiceChannel::GetLastLinkReason( void ) const { return m_lastReason; }
const CORE::CString& CRemoteServiceChannel::GetLastLinkText( void ) const { return m_lastText; }
UInt32
CRemoteServiceChannel::GetServicePeerId( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_session )
        return 0;
    MT::CScopeMutex lock( m_node->GetLock() );
    return m_session->GetSequencerPeerId();
}

/*-------------------------------------------------------------------------*/

UInt32 CRemoteServiceChannel::GetLinkCount( void ) const { return m_links; }
UInt32 CRemoteServiceChannel::GetRpcResumeCount( void ) const { return m_rpcResumes; }
UInt32 CRemoteServiceChannel::GetNewRpcSessionCount( void ) const { return m_newRpcSessions; }

/*-------------------------------------------------------------------------*/

COMCORE::CIRpcTransportPtr
CRemoteServiceChannel::WrapTransport( const COMCORE::CIRpcTransportPtr& transport )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_owner )
        return m_owner->WrapTransport( m_index, transport );
    return transport;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceChannel::StartJoin( const CDeviceCredentials& credentials, const SSettings& settings, const CORE::CString& hostOverride, CORE::CString& error )
{GUCEF_TRACE;

    if ( CHANNEL_REFUSED == m_state )
    {
        error = "the service refused this channel: " + m_lastText;
        return false;
    }
    m_settings = settings;
    m_drnSessionName = credentials.drnSessionName;
    DRN::CDRNEndpointList endpoints;
    if ( !credentials.GetEndpointList( endpoints, hostOverride ) || 0 == endpoints.GetCount() )
    {
        error = "the credentials hold no usable session endpoint";
        return false;
    }
    DRN::CDRNAuthSettings auth;
    auth.authenticatorType = DRN::CDRNTokenAuthenticator::MethodName;
    auth.SetSetting( "token", credentials.token.Reveal() );
    if ( GUCEF_NULL == m_node )
    {
        DRN::CDRNNodeSettings nodeSettings;
        nodeSettings.serviceName = credentials.serviceName;
        nodeSettings.ownThread = true;
        nodeSettings.linkTimeoutMs = settings.linkTimeoutMs;
        nodeSettings.echoIntervalMs = settings.echoIntervalMs;
        nodeSettings.auth = auth;
        if ( 0 == credentials.securityPolicy.GetNrOfChildNodes() && credentials.securityPolicy.GetAttributeValueOrChildValueByName( "requireEncryption" ).IsNULLOrEmpty() )
        {
            error = "the credentials hold no security policy of the service (pair again)";
            return false;
        }
        if ( !nodeSettings.securityPolicy.LoadConfig( credentials.securityPolicy ) )
        {
            error = "the security policy of the credentials cannot be read";
            return false;
        }
        m_node = GUCEF_NEW DRN::CDRNNode( m_pulse );
        if ( !m_node->SetSettings( nodeSettings ) )
        {
            GUCEF_DELETE m_node;
            m_node = GUCEF_NULL;
            error = "the DRN node of the channel cannot be set up with the security policy of the service";
            return false;
        }
        TEventCallback onRefused( this, &CRemoteServiceChannel::OnLinkEnded );
        SubscribeTo( m_node, DRN::CDRNNode::LinkRefusedEvent, onRefused );
        TEventCallback onClosed( this, &CRemoteServiceChannel::OnLinkEnded );
        SubscribeTo( m_node, DRN::CDRNNode::LinkClosedEvent, onClosed );
        m_session = m_node->CreateSession();
        TEventCallback onJoinFailed( this, &CRemoteServiceChannel::OnJoinFailed );
        SubscribeTo( m_session, DRN::CDRNSession::JoinFailedEvent, onJoinFailed );
    }
    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = m_drnSessionName;
    bool joining = false;
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        DRN::CDRNSession::TSessionState sessionState = m_session->GetState();
        if ( DRN::CDRNSession::SESSIONSTATE_JOINING == sessionState || DRN::CDRNSession::SESSIONSTATE_MEMBER == sessionState )
            joining = true;
        else
            joining = m_session->Join( endpoints, joinSettings, &auth );
    }
    if ( !joining )
    {
        error = "the session cannot be joined at " + endpoints.ToString();
        m_state = CHANNEL_DOWN;
        return false;
    }
    m_rpcResumed = false;
    m_rpcClosed = false;
    m_joinFailed = false;
    m_lastReason = DRN::DRN_LINKREASON_NONE;
    m_lastText.Clear();
    m_state = CHANNEL_JOINING;
    m_stepStartedMs = CRemoteServiceHost::NowMs();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::OnLinkEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventData )->GetData();
    m_lastReason = info.closedInfo.reason;
    m_lastText = info.closedInfo.text;
    if ( IsPermanentRefusal( info.closedInfo.reason ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceChannel(" << m_index << "): the service refused the link (" << (Int32) info.closedInfo.reason << "): " << info.closedInfo.text );
        m_state = CHANNEL_REFUSED;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::OnJoinFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_joinFailed = true;
    if ( GUCEF_NULL != eventData )
        m_lastText = static_cast< CORE::TCloneableString* >( eventData )->GetData();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::OnRpcResumed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_rpcResumed = true;
    ++m_rpcResumes;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::OnRpcGap( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a gap or a session unknown at the service: the endpoint made a new RPC session in the same round trip
    m_rpcNewSession = true;
    ++m_newRpcSessions;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::OnRpcClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_rpcClosed = true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::ReleaseEndpoint( bool close )
{GUCEF_TRACE;

    if ( !m_endpoint.IsNULL() )
    {
        UnsubscribeFrom( m_endpoint.GetPointerAlways() );
        if ( close )
            m_endpoint->Close( false );
        m_endpoint.Unlink();
    }
    m_transport.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::Step( void )
{GUCEF_TRACE;

    switch ( m_state )
    {
        case CHANNEL_JOINING: StepJoining(); break;
        case CHANNEL_BINDING: StepBinding(); break;
        case CHANNEL_READY: StepReady(); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::StepJoining( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node )
        return;
    DRN::CDRNSession::TSessionState sessionState = DRN::CDRNSession::SESSIONSTATE_IDLE;
    UInt32 servicePeer = 0;
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        sessionState = m_session->GetState();
        servicePeer = m_session->GetSequencerPeerId();
    }
    if ( DRN::CDRNSession::SESSIONSTATE_JOINING == sessionState )
        return;
    if ( DRN::CDRNSession::SESSIONSTATE_MEMBER != sessionState )
    {
        if ( CHANNEL_REFUSED != m_state )
            m_state = CHANNEL_DOWN;
        return;
    }

    // joined: the RPC session over the member link, new or resumed (case A)
    CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex > transport( GUCEF_NEW DRN::CDRNRpcTransport( *m_session, servicePeer ) );
    if ( !transport->Connect( CORE::CUri() ) )
    {
        m_lastText = "the RPC transport over the link cannot be opened: " + transport->GetConnectStatus().ToString();
        m_state = CHANNEL_DOWN;
        return;
    }
    ++m_links;
    COMCORE::CIRpcTransportPtr plain;
    plain = transport;
    m_transport = WrapTransport( plain );
    m_rpcResumed = false;
    m_awaitingRebind = false;
    if ( m_endpoint.IsNULL() || COMCORE::RPC_SESSIONSTATE_CLOSED == m_endpoint->GetState() || m_rpcClosed )
    {
        ReleaseEndpoint( false );
        m_transport = WrapTransport( plain );
        COMCORE::CRpcEndpointSettings endpointSettings;
        endpointSettings.nodeName = m_settings.nodeName;
        endpointSettings.requireSecureTransport = true;
        endpointSettings.autoReconnect = false;
        endpointSettings.limits.resumeGraceMs = m_settings.resumeGraceMs;
        endpointSettings.limits.receiveWindowBytes = m_settings.receiveWindowBytes;
        endpointSettings.limits.maxMessageSize = m_settings.maxMessageBytes;
        endpointSettings.sendQueueMaxBytes = m_settings.sendQueueBytes;
        COMCORE::CRpcEndpointPtr endpoint = COMCORE::CRpcEndpoint::Create( m_pulse, endpointSettings );
        endpoint->AddWantedContract( CRemoteSessionContracts::ContractName, CRemoteSessionContracts::Major );
        for ( UInt32 i=0; i<m_settings.wantedContracts.size(); ++i )
            endpoint->AddWantedContract( m_settings.wantedContracts[ i ], 1 );
        TEventCallback onResumed( this, &CRemoteServiceChannel::OnRpcResumed );
        SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionResumedEvent, onResumed );
        TEventCallback onGap( this, &CRemoteServiceChannel::OnRpcGap );
        SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionGapEvent, onGap );
        TEventCallback onClosed( this, &CRemoteServiceChannel::OnRpcClosed );
        SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionClosedEvent, onClosed );
        m_endpoint = endpoint;
        m_rpcClosed = false;
        // a new endpoint is a new RPC session: the client session is resumed or opened anew on it
        m_rpcNewSession = true;
        ++m_newRpcSessions;
        if ( !m_endpoint->Start( m_transport, true ) )
        {
            m_lastText = "the RPC session cannot be started";
            m_state = CHANNEL_DOWN;
            return;
        }
    }
    else
    {
        // case A: the suspended RPC session goes on over the new member link
        m_rpcNewSession = false;
        m_awaitingRebind = true;
        if ( !m_endpoint->Rebind( m_transport ) )
        {
            ReleaseEndpoint( true );
            m_lastText = "the RPC session cannot be rebound";
            m_state = CHANNEL_DOWN;
            return;
        }
    }
    m_state = CHANNEL_BINDING;
    m_stepStartedMs = CRemoteServiceHost::NowMs();
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::StepBinding( void )
{GUCEF_TRACE;

    if ( m_endpoint.IsNULL() )
    {
        m_state = CHANNEL_DOWN;
        return;
    }
    COMCORE::TRpcSessionState rpcState = m_endpoint->GetState();
    if ( m_rpcClosed || COMCORE::RPC_SESSIONSTATE_CLOSED == rpcState )
    {
        m_state = CHANNEL_DOWN;
        return;
    }
    if ( GUCEF_NULL != m_node )
    {
        // the new link died before the RPC session came up on it: down at once, not at the binding timeout
        DRN::CDRNSession::TSessionState sessionState = DRN::CDRNSession::SESSIONSTATE_IDLE;
        {
            MT::CScopeMutex lock( m_node->GetLock() );
            sessionState = m_session->GetState();
        }
        if ( DRN::CDRNSession::SESSIONSTATE_MEMBER != sessionState )
        {
            m_transport.Unlink();
            m_state = CHANNEL_DOWN;
            return;
        }
    }
    if ( COMCORE::RPC_SESSIONSTATE_ESTABLISHED != rpcState )
    {
        if ( CRemoteServiceHost::NowMs() - m_stepStartedMs > 20000 )
        {
            m_lastText = "the RPC session did not come up";
            m_state = CHANNEL_DOWN;
        }
        return;
    }
    if ( m_awaitingRebind )
    {
        // The endpoint can be ESTABLISHED again (set on the protocol thread) before its SessionResumedEvent or
        // SessionGapEvent reached this thread through the endpoint's mail: wait for the event, so that the connection
        // tells a resumed RPC session (case A) from a new one (case B) by what happened, not by when it looked
        if ( !m_rpcResumed && !m_rpcNewSession )
        {
            if ( CRemoteServiceHost::NowMs() - m_stepStartedMs > 20000 )
            {
                m_lastText = "the RPC session did not say how the rebind went";
                m_state = CHANNEL_DOWN;
            }
            return;
        }
        m_awaitingRebind = false;
    }
    if ( !m_endpoint->IsContractBound( CRemoteSessionContracts::ContractName, CRemoteSessionContracts::Major ) )
        return;
    for ( UInt32 i=0; i<m_settings.wantedContracts.size(); ++i )
    {
        if ( !m_endpoint->IsContractBound( m_settings.wantedContracts[ i ], 1 ) )
            return;
    }
    m_state = CHANNEL_READY;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::StepReady( void )
{GUCEF_TRACE;

    if ( m_endpoint.IsNULL() )
    {
        m_state = CHANNEL_DOWN;
        return;
    }
    COMCORE::TRpcSessionState rpcState = m_endpoint->GetState();
    if ( m_rpcClosed || COMCORE::RPC_SESSIONSTATE_CLOSED == rpcState || COMCORE::RPC_SESSIONSTATE_SUSPENDED == rpcState )
    {
        // the link went: the endpoint waits for a rebind (suspended) or is gone (closed)
        m_transport.Unlink();
        if ( CHANNEL_REFUSED != m_state )
            m_state = CHANNEL_DOWN;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::DropLink( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_session )
        return;
    MT::CScopeMutex lock( m_node->GetLock() );
    DRN::CDRNPeerLinkPtr link = m_session->GetLinkToMember( m_session->GetSequencerPeerId() );
    if ( !link.IsNULL() )
        link->Close( DRN::DRN_LINKREASON_CLOSED_BY_APP, "dropped by the application" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceChannel::Shutdown( bool graceful )
{GUCEF_TRACE;

    if ( !m_endpoint.IsNULL() )
    {
        UnsubscribeFrom( m_endpoint.GetPointerAlways() );
        m_endpoint->Close( graceful );
        m_endpoint.Unlink();
    }
    m_transport.Unlink();
    if ( GUCEF_NULL != m_node )
    {
        UnsubscribeFrom( m_node );
        if ( GUCEF_NULL != m_session )
            UnsubscribeFrom( m_session );
        {
            MT::CScopeMutex lock( m_node->GetLock() );
            m_session->Leave();
        }
        m_node->Stop();
        GUCEF_DELETE m_node;
        m_node = GUCEF_NULL;
        m_session = GUCEF_NULL;
    }
    if ( CHANNEL_REFUSED != m_state )
        m_state = CHANNEL_IDLE;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceChannel::GetRoundTripTime( UInt32& rttUs ) const
{GUCEF_TRACE;

    rttUs = 0;
    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_session )
        return false;
    MT::CScopeMutex lock( m_node->GetLock() );
    DRN::CDRNPeerLinkPtr link = m_session->GetLinkToMember( m_session->GetSequencerPeerId() );
    if ( link.IsNULL() )
        return false;
    DRN::CDRNLatencyInfo info;
    link->GetLatencyInfo( info );
    rttUs = info.smoothedRtt;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
