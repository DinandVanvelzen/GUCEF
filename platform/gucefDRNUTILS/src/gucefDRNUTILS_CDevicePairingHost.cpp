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

#include <string.h>

#include "gucefDRNUTILS_CDevicePairingHost.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#include "gucefDRNUTILS_CDeviceContracts.h"
#define GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#endif /* GUCEF_DRNUTILS_CDEVICECONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

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

namespace {

COMCORE::CRpcStatus
DetailedStatus( COMCORE::TRpcStatusCode code, const char* detailCode, const CORE::CString& message )
{GUCEF_TRACE;

    COMCORE::CRpcStatus status( code, CORE::CString( detailCode ) + ": " + message );
    CRemoteServiceDetailCodes::SetDetails( status.details, detailCode );
    return status;
}

/*-------------------------------------------------------------------------*/

/** A fingerprint as typed: upper case, without separators and spaces */
CORE::CString
NormaliseFingerprint( const CORE::CString& fingerprint )
{GUCEF_TRACE;

    CORE::CString result;
    CORE::CString upper = fingerprint.Uppercase();
    for ( UInt32 i=0; i<upper.Length(); ++i )
    {
        char c = upper[ i ];
        if ( '-' != c && ' ' != c )
            result += c;
    }
    return result;
}

}

/*-------------------------------------------------------------------------*/

CDevicePairingHost::CDevicePairingHost( CRemoteServiceHost& host, const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_host( host )
    , m_pulse( pulseGenerator )
    , m_node( GUCEF_NULL )
    , m_session( GUCEF_NULL )
    , m_listener()
    , m_rpcHost( GUCEF_NULL )
    , m_pairingService()
    , m_bans()
    , m_request()
    , m_windowId()
    , m_openedBy()
    , m_open( false )
    , m_requireConfirmation( false )
    , m_remaining( 0 )
    , m_failedProofs( 0 )
    , m_paired( 0 )
    , m_expiresAtMs( 0 )
    , m_expiresUtcMs( 0 )
    , m_closedAtMs( 0 )
    , m_port( 0 )
    , m_failedByAddress()
    , m_nonces()
    , m_pending()
{GUCEF_TRACE;

    memset( m_serverProofKey, 0, sizeof( m_serverProofKey ) );
    m_pairingService.BindMethod( "Begin", GUCEF_NEW THandler( this, &CDevicePairingHost::OnBegin ) );
    m_pairingService.BindMethod( "Complete", GUCEF_NEW THandler( this, &CDevicePairingHost::OnComplete ) );
}

/*-------------------------------------------------------------------------*/

CDevicePairingHost::~CDevicePairingHost()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDevicePairingHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CDevicePairingHost";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool CDevicePairingHost::IsOpen( void ) const { return m_open; }
const CORE::CString& CDevicePairingHost::GetWindowId( void ) const { return m_windowId; }
UInt16 CDevicePairingHost::GetPort( void ) const { return m_port; }
UInt32 CDevicePairingHost::GetFailedProofCount( void ) const { return m_failedProofs; }
UInt32 CDevicePairingHost::GetPendingCount( void ) const { return (UInt32) m_pending.size(); }
UInt32 CDevicePairingHost::GetPairedCount( void ) const { return m_paired; }
bool CDevicePairingHost::HasNode( void ) const { return GUCEF_NULL != m_node; }
const DRN::CDRNBanList& CDevicePairingHost::GetBans( void ) const { return m_bans; }

/*-------------------------------------------------------------------------*/

bool
CDevicePairingHost::NeedsConfirmation( const CPairingWindowRequest& request ) const
{GUCEF_TRACE;

    const CORE::CString::StringSet& privileged = m_host.GetSettings().requireConfirmationForRoles;
    bool grantsPrivilege = false;
    for ( CORE::CString::StringSet::const_iterator i = request.roles.begin(); i != request.roles.end(); ++i )
    {
        if ( privileged.find( (*i) ) != privileged.end() )
            grantsPrivilege = true;
    }
    // a privileged window always confirms; another one only when asked
    if ( grantsPrivilege )
        return true;
    return request.hasRequireConfirmation && request.requireConfirmation;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingHost::Open( const CPairingWindowRequest& request, const CORE::CString& openedBy, CORE::CString& windowId,
                          CORE::CString& code, UInt64& expiresUtcMs, CORE::CString& error )
{GUCEF_TRACE;

    const CRemoteServiceHostSettings& settings = m_host.GetSettings();
    if ( request.deviceKind.IsNULLOrEmpty() )
    {
        error = "a pairing window names the device kind it grants";
        return false;
    }
    if ( request.roles.empty() )
    {
        error = "a pairing window grants at least one role";
        return false;
    }
    UInt32 seconds = 0 == request.seconds ? settings.pairingDefaultSeconds : request.seconds;
    if ( 0 != settings.pairingMaxSeconds && seconds > settings.pairingMaxSeconds )
    {
        error = "a pairing window is open for at most " + CORE::ToString( settings.pairingMaxSeconds ) + " seconds";
        return false;
    }
    if ( 0 == request.count || request.count > 100 )
    {
        error = "a pairing window allows 1 to 100 pairings";
        return false;
    }
    if ( m_open || GUCEF_NULL != m_node )
        Shutdown();

    CORE::CString normalised;
    CORE::CString authSecretHex;
    if ( !CDevicePairingCrypto::GenerateCode( normalised ) ||
         !CDevicePairingCrypto::DeriveKeys( normalised, settings.serviceName, authSecretHex, m_serverProofKey ) ||
         !CDevicePairingCrypto::GenerateWindowId( m_windowId ) )
    {
        error = "no pairing code can be made (no randomness or no hash codec)";
        return false;
    }
    m_request = request;
    m_request.seconds = seconds;
    m_requireConfirmation = NeedsConfirmation( request );
    m_openedBy = openedBy;
    m_remaining = request.count;
    m_failedProofs = 0;
    m_paired = 0;
    m_failedByAddress.clear();
    m_nonces.clear();
    if ( !StartNode( authSecretHex, error ) )
    {
        authSecretHex.Clear();
        StopNode();
        return false;
    }
    authSecretHex.Clear();
    m_open = true;
    m_expiresAtMs = CRemoteServiceHost::NowMs() + (UInt64) seconds * 1000;
    m_expiresUtcMs = CRemoteServiceHost::NowUtcMs() + (UInt64) seconds * 1000;
    windowId = m_windowId;
    code = CDevicePairingCrypto::FormatCode( normalised );
    normalised.Clear();
    expiresUtcMs = m_expiresUtcMs;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: pairing window " << m_windowId << " opened by " << openedBy << " on port " << m_port << " for "
               << seconds << " s, kind " << request.deviceKind << ", " << request.count << " pairing(s)" << ( m_requireConfirmation ? ", fingerprint confirmation" : "" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingHost::StartNode( const CORE::CString& authSecretHex, CORE::CString& error )
{GUCEF_TRACE;

    const CRemoteServiceHostSettings& settings = m_host.GetSettings();
    DRN::CDRNNodeSettings nodeSettings;
    nodeSettings.serviceName = settings.serviceName;
    nodeSettings.ownThread = true;
    nodeSettings.linkTimeoutMs = settings.linkTimeoutMs;
    nodeSettings.echoIntervalMs = settings.echoIntervalMs;
    nodeSettings.handshakeTimeoutMs = settings.handshakeTimeoutMs;
    nodeSettings.auth.authenticatorType = "shared-secret";
    nodeSettings.auth.SetSetting( "secret", authSecretHex );
    nodeSettings.securityPolicy = CRemoteServiceHost::MakePairingSecurityPolicy();
    nodeSettings.staticPrivateKey = m_host.GetStaticPrivateKeyHex().Reveal();
    m_node = GUCEF_NEW DRN::CDRNNode( m_pulse );
    if ( !m_node->SetSettings( nodeSettings ) )
    {
        error = "the DRN node of the pairing listener cannot be set up";
        return false;
    }
    m_node->SetBanList( &m_bans );
    TEventCallback onRefused( this, &CDevicePairingHost::OnLinkRefused );
    SubscribeTo( m_node, DRN::CDRNNode::LinkRefusedEvent, onRefused );
    TEventCallback onClosed( this, &CDevicePairingHost::OnLinkRefused );
    SubscribeTo( m_node, DRN::CDRNNode::LinkClosedEvent, onClosed );
    CORE::CString host = settings.bindAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : settings.bindAddress;
    DRN::CDRNEndpoint endpoint( "tcp://" + host + ":" + CORE::ToString( settings.pairingPort ) );
    if ( !settings.bindAddress.IsNULLOrEmpty() )
        endpoint.GetParams().Set( "bindAddress", CORE::CVariant( settings.bindAddress ) );
    if ( !m_node->Listen( endpoint ) )
    {
        error = "the pairing listener cannot listen on port " + CORE::ToString( settings.pairingPort );
        return false;
    }
    DRN::CDRNEndpointList listening = m_node->GetListenEndpoints();
    m_port = listening.GetCount() > 0 ? listening.Get( 0 ).GetPort() : settings.pairingPort;
    m_session = m_node->CreateSession();
    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = settings.GetPairingSessionName();
    sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        if ( !m_session->Host( sessionSettings ) )
        {
            error = "the pairing session cannot be hosted";
            return false;
        }
    }
    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings.nodeName = settings.serviceName + ".pairing";
    hostSettings.endpointSettings.requireSecureTransport = true;
    hostSettings.endpointSettings.limits.resumeGraceMs = 30000;
    hostSettings.maxSessions = 16;
    m_rpcHost = GUCEF_NEW COMCORE::CRpcHost( m_pulse, hostSettings );
    COMCORE::CRpcRoleAccessPolicy* policy = GUCEF_NEW COMCORE::CRpcRoleAccessPolicy();
    policy->SetRequireAuthenticated( true );
    m_rpcHost->SetAccessPolicy( COMCORE::CIRpcAccessPolicyPtr( policy ) );
    COMCORE::CRpcContractPtr contract = CDeviceContracts::GetContract();
    if ( contract.IsNULL() || !m_rpcHost->RegisterService( contract, CDeviceContracts::PairingServiceName, &m_pairingService, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) )
    {
        error = "the pairing service cannot be registered";
        return false;
    }
    DRN::CDRNRpcTransportListener* listener = GUCEF_NEW DRN::CDRNRpcTransportListener( *m_session );
    CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex > typedListener( listener );
    m_listener = typedListener;
    if ( !m_rpcHost->AddListener( m_listener ) || !listener->Listen( CORE::CUri() ) )
    {
        error = "the RPC listener of the pairing session cannot be started";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::StopNode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_node )
        UnsubscribeFrom( m_node );
    if ( GUCEF_NULL != m_rpcHost )
    {
        m_rpcHost->Stop();
        GUCEF_DELETE m_rpcHost;
        m_rpcHost = GUCEF_NULL;
    }
    if ( !m_listener.IsNULL() )
    {
        m_listener->StopListening();
        m_listener.Unlink();
    }
    if ( GUCEF_NULL != m_node )
    {
        m_node->Stop();
        m_node->SetBanList( GUCEF_NULL );
        GUCEF_DELETE m_node;
        m_node = GUCEF_NULL;
        m_session = GUCEF_NULL;
    }
    m_nonces.clear();
    m_port = 0;
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::Close( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    m_open = false;
    memset( m_serverProofKey, 0, sizeof( m_serverProofKey ) );
    m_closedAtMs = CRemoteServiceHost::NowMs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: pairing window " << m_windowId << " closed: " << reason );
    // the node goes at a later tick: an answer given just now still has to leave over its link
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::Shutdown( void )
{GUCEF_TRACE;

    Close( "the pairing listener stops" );
    while ( !m_pending.empty() )
        DeclinePending( m_pending.front().deviceId, "the pairing window ended" );
    StopNode();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingHost::GetCallAddress( const COMCORE::CRpcServerCall& call ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_session )
        return CORE::CString::Empty;
    UInt32 peerId = call.GetPrincipal().attributes.GetValueAlways( "drn.peerId" ).AsUInt32( 0, true );
    MT::CScopeMutex lock( m_node->GetLock() );
    DRN::CDRNPeerLinkPtr link = m_session->GetLinkToMember( peerId );
    return link.IsNULL() ? CORE::CString::Empty : link->GetRemoteEndpoint().GetHost();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingHost::GetCallKey( const COMCORE::CRpcServerCall& call ) const
{GUCEF_TRACE;

    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !call.GetSessionId( sid ) )
        return CORE::CString::Empty;
    return DRN::CDRNCrypto::ToHex( sid, sizeof( sid ) );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CDevicePairingHost::OnBegin( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    if ( !m_open )
    {
        call.Fail( DetailedStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::PairingClosed, "no pairing window is open" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    UInt8 nonce[ CDevicePairingCrypto::NonceSize ];
    CORE::CString key = GetCallKey( call );
    if ( key.IsNULLOrEmpty() || !CDevicePairingCrypto::RandomBytes( nonce, sizeof( nonce ) ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, "no nonce can be made" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    m_nonces[ key ] = CORE::CDynamicBuffer();
    m_nonces[ key ].CopyFrom( sizeof( nonce ), nonce );
    UInt8 proof[ 32 ];
    if ( !CDevicePairingCrypto::ComputeServerProof( m_serverProofKey, m_host.GetStaticPublicKey(), nonce, sizeof( nonce ), proof ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, "no server proof can be made" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( 1, m_host.GetServiceId() );
    response.SetBytes( 2, m_host.GetStaticPublicKey(), 32 );
    response.SetBytes( 3, nonce, sizeof( nonce ) );
    response.SetBytes( 4, proof, sizeof( proof ) );
    response.SetUInt64( 5, m_expiresUtcMs );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingHost::BuildCompletion( CDeviceRecord& device, COMCORE::CRpcMessage& response, CORE::CString& error )
{GUCEF_TRACE;

    UInt64 expiresUtcMs = 0;
    CORE::CString token = m_host.IssueToken( device, expiresUtcMs );
    if ( token.IsNULLOrEmpty() )
    {
        error = "no device token can be issued";
        return false;
    }
    const CRemoteServiceHostSettings& settings = m_host.GetSettings();
    response.SetString( 1, device.deviceId );
    response.SetString( 2, token );
    token.Clear();
    response.SetUInt64( 3, expiresUtcMs );
    for ( CORE::CString::StringSet::const_iterator i = device.roles.begin(); i != device.roles.end(); ++i )
        response.Add( 4, CORE::CVariant( (*i) ) );
    response.SetString( 5, device.kind );
    CORE::CString::StringVector endpoints = m_host.GetSessionEndpoints();
    for ( UInt32 i=0; i<endpoints.size(); ++i )
        response.Add( 6, CORE::CVariant( endpoints[ i ] ) );
    response.SetBytes( 7, m_host.GetStaticPublicKey(), 32 );
    response.SetBytes( 8, m_host.GetIssuerPublicKey(), 32 );
    CORE::CDataNode policy;
    m_host.GetSessionPolicyDocument( policy );
    response.SetDataNode( 9, policy );
    response.SetString( 10, settings.serviceName );
    response.SetString( 11, settings.drnSessionName );
    response.SetString( 12, m_host.GetServiceId() );
    return true;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CDevicePairingHost::OnComplete( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    const CORE::CString address = GetCallAddress( call );
    if ( !m_open )
    {
        call.Fail( DetailedStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::PairingClosed, "the pairing window is closed" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    TNonceMap::iterator nonce = m_nonces.find( GetCallKey( call ) );
    const CORE::CString deviceName = request.GetString( 1 );
    CORE::CDynamicBuffer publicKey;
    CORE::CDynamicBuffer keyProof;
    request.GetBytes( 4, publicKey );
    request.GetBytes( 5, keyProof );
    CORE::CString problem;
    if ( nonce == m_nonces.end() )
        problem = "Begin was not called on this session";
    else if ( deviceName.IsNULLOrEmpty() || deviceName.Length() > 64 )
        problem = "the device name has to have 1 to 64 characters";
    else if ( CDevicePairingCrypto::KeySize != publicKey.GetDataSize() || CDevicePairingCrypto::SignatureSize != keyProof.GetDataSize() )
        problem = "the device key or its proof has the wrong size";
    else if ( !CDevicePairingCrypto::VerifyDeviceProof( static_cast< const UInt8* >( publicKey.GetConstBufferPtr() ), m_host.GetStaticPublicKey(),
                                                        static_cast< const UInt8* >( (*nonce).second.GetConstBufferPtr() ), (*nonce).second.GetDataSize(),
                                                        static_cast< const UInt8* >( keyProof.GetConstBufferPtr() ), keyProof.GetDataSize() ) )
        problem = "the proof of the device key does not verify";
    if ( !problem.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: a pairing from " << address << " was refused: " << problem );
        call.Fail( DetailedStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, CRemoteServiceDetailCodes::BadProof, problem ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    m_nonces.erase( nonce );

    // re-pairing with the same key keeps the device id; a revoked key never pairs again
    const CORE::CString publicKeyHex = DRN::CDRNCrypto::ToHex( publicKey.GetConstBufferPtr(), 32 ).Lowercase();
    CDeviceRecord device;
    bool isNew = !m_host.GetDeviceRegistry().FindByPublicKey( publicKeyHex, device );
    if ( !isNew && DEVICESTATUS_REVOKED == device.status )
    {
        call.Fail( DetailedStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceNotActive, "this device key was revoked; make a new key" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( isNew && !CDevicePairingCrypto::GenerateDeviceId( device.deviceId ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, "no device id can be made" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    device.userId = m_host.GetSettings().ownerUserId;
    device.name = deviceName;
    device.kind = m_request.deviceKind;
    device.platform = request.GetString( 2 );
    device.appVersion = request.GetString( 3 );
    device.roles = m_request.roles;
    device.publicKey = publicKeyHex;
    device.fingerprint = CDevicePairingCrypto::Fingerprint( static_cast< const UInt8* >( publicKey.GetConstBufferPtr() ) );
    device.pairedUtcMs = CRemoteServiceHost::NowUtcMs();
    device.pairedBy = m_openedBy;
    device.lastAddress = address;
    if ( m_remaining > 0 )
        --m_remaining;

    if ( m_requireConfirmation )
    {
        if ( isNew )
        {
            CDeviceRecord pendingRecord( device );
            pendingRecord.status = DEVICESTATUS_PENDING_CONFIRMATION;
            m_host.GetDeviceRegistry().Put( pendingRecord );
            m_host.SaveRegistry();
        }
        SPending pending;
        pending.call = call.GetSharedPtr();
        pending.deviceId = device.deviceId;
        pending.address = address;
        pending.expiresAtMs = CRemoteServiceHost::NowMs() + (UInt64) m_host.GetSettings().confirmationTimeoutSeconds * 1000;
        pending.device = device;
        m_pending.push_back( pending );
        call.Accept( COMCORE::CRpcRequestId( device.deviceId ), false, m_host.GetSettings().confirmationTimeoutSeconds * 1000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: device " << device.deviceId << " (" << device.kind << ", fingerprint " << device.fingerprint
                   << ") waits for the owner's confirmation" );
        if ( 0 == m_remaining )
            Close( "the pairings of the window are made" );
        return COMCORE::RPC_HANDLER_DEFERRED;
    }

    device.status = DEVICESTATUS_ACTIVE;
    COMCORE::CRpcMessage response = call.NewResponse();
    CORE::CString error;
    if ( !BuildCompletion( device, response, error ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, error ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    m_host.GetDeviceRegistry().Put( device );
    m_host.SaveRegistry();
    ++m_paired;
    call.Reply( response );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: paired device " << device.deviceId << " (" << device.kind << ", fingerprint " << device.fingerprint << ")" );
    if ( 0 == m_remaining )
        Close( "the pairings of the window are made" );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingHost::Confirm( const CORE::CString& deviceId, const CORE::CString& fingerprint, bool approve,
                             const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    TPendingVector::iterator i = m_pending.begin();
    for ( ; i != m_pending.end(); ++i )
    {
        if ( (*i).deviceId == deviceId )
            break;
    }
    if ( i == m_pending.end() )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": no pairing of " + deviceId + " waits for a confirmation";
        return false;
    }
    if ( approve && NormaliseFingerprint( fingerprint ) != NormaliseFingerprint( (*i).device.fingerprint ) )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::BadProof ) + ": the fingerprint is not the one of the device's key; nothing was decided";
        return false;
    }
    SPending pending = (*i);
    m_pending.erase( i );
    if ( !approve )
    {
        CDeviceRecord existing;
        if ( m_host.GetDeviceRegistry().Find( deviceId, existing ) && DEVICESTATUS_PENDING_CONFIRMATION == existing.status )
        {
            m_host.GetDeviceRegistry().Remove( deviceId );
            m_host.SaveRegistry();
        }
        pending.call->CompleteResult( DetailedStatus( COMCORE::RPC_STATUS_ABORTED, CRemoteServiceDetailCodes::PairingDeclined, "the owner declined the pairing" ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: the pairing of device " << deviceId << " was declined by " << by );
        return true;
    }
    CDeviceRecord device = pending.device;
    device.status = DEVICESTATUS_ACTIVE;
    COMCORE::CRpcMessage response = pending.call->NewResponse();
    if ( !BuildCompletion( device, response, error ) )
    {
        pending.call->CompleteResult( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, error ) );
        return false;
    }
    m_host.GetDeviceRegistry().Put( device );
    m_host.SaveRegistry();
    ++m_paired;
    pending.call->CompleteResult( response );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: the pairing of device " << deviceId << " was confirmed by " << by );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::GetPending( GUCEF::vector< CDeviceRecord >& pending ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_pending.size(); ++i )
    {
        CDeviceRecord record = m_pending[ i ].device;
        record.status = DEVICESTATUS_PENDING_CONFIRMATION;
        pending.push_back( record );
    }
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::DeclinePending( const CORE::CString& deviceId, const CORE::CString& reason )
{GUCEF_TRACE;

    for ( TPendingVector::iterator i = m_pending.begin(); i != m_pending.end(); ++i )
    {
        if ( (*i).deviceId != deviceId )
            continue;
        SPending pending = (*i);
        m_pending.erase( i );
        CDeviceRecord existing;
        if ( m_host.GetDeviceRegistry().Find( deviceId, existing ) && DEVICESTATUS_PENDING_CONFIRMATION == existing.status )
        {
            m_host.GetDeviceRegistry().Remove( deviceId );
            m_host.SaveRegistry();
        }
        if ( !pending.call.IsNULL() )
            pending.call->CompleteResult( DetailedStatus( COMCORE::RPC_STATUS_ABORTED, CRemoteServiceDetailCodes::PairingDeclined, reason ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: the pairing of device " << deviceId << " was declined: " << reason );
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::OnTick( void )
{GUCEF_TRACE;

    const UInt64 now = CRemoteServiceHost::NowMs();
    if ( m_open && now >= m_expiresAtMs )
        Close( "the window expired" );
    for ( UInt32 i=0; i<m_pending.size(); ++i )
    {
        if ( now >= m_pending[ i ].expiresAtMs )
        {
            DeclinePending( m_pending[ i ].deviceId, "the pairing was not confirmed in time" );
            break;
        }
    }
    // the listener of a closed window goes once nothing waits on it and the last answer had time to leave
    if ( !m_open && m_pending.empty() && GUCEF_NULL != m_node && now - m_closedAtMs >= 2000 )
        StopNode();
    m_bans.PurgeExpired();
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingHost::OnLinkRefused( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventData )->GetData();
    if ( DRN::DRN_LINKREASON_AUTH_FAILED != info.closedInfo.reason || info.link.IsNULL() || info.closedInfo.wasOperational )
        return;
    const CORE::CString address = info.link->GetRemoteEndpoint().GetHost();
    const CRemoteServiceHostSettings& settings = m_host.GetSettings();
    ++m_failedProofs;
    UInt32 fromAddress = ++m_failedByAddress[ address ];
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: a wrong pairing code from " << address << " (" << fromAddress << " from this address, "
               << m_failedProofs << " in the window)" );
    if ( 0 != settings.banAfterFailedProofs && fromAddress >= settings.banAfterFailedProofs )
    {
        m_bans.Ban( DRN::DRN_BAN_ADDRESS, address, DRN::CDRNBanList::GetUnixTimeNow() + (UInt64) settings.banMinutes * 60, "failed pairing proofs" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingHost: the address " << address << " is banned from pairing for " << settings.banMinutes << " minutes" );
    }
    if ( m_open && 0 != settings.maxFailedProofs && m_failedProofs >= settings.maxFailedProofs )
        Close( "too many wrong pairing codes" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
