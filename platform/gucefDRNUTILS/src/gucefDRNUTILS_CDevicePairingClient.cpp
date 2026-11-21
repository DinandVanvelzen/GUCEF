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

#include "gucefDRNUTILS_CDevicePairingClient.h"

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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

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

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

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

const CORE::CEvent CDevicePairingClient::FingerprintEvent = "GUCEF::DRNUTILS::CDevicePairingClient::FingerprintEvent";
const CORE::CEvent CDevicePairingClient::AwaitingConfirmationEvent = "GUCEF::DRNUTILS::CDevicePairingClient::AwaitingConfirmationEvent";

namespace {

/** Watches the pairing node and endpoint: refusals of the link, ACCEPTED of Complete */
class CPairingWatch : public CORE::CObserver
{
    public:

    bool refused;
    DRN::TDRNLinkReason refusedReason;
    CORE::CString refusedText;
    bool accepted;

    CPairingWatch( void ) : CORE::CObserver(), refused( false ), refusedReason( DRN::DRN_LINKREASON_NONE ), refusedText(), accepted( false ) {}
    virtual ~CPairingWatch() { SignalUpcomingObserverDestruction(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( ( DRN::CDRNNode::LinkRefusedEvent == eventId || DRN::CDRNNode::LinkClosedEvent == eventId ) && GUCEF_NULL != eventData )
        {
            const DRN::CDRNNodeLinkInfo& info = static_cast< DRN::CDRNNodeLinkEventData* >( eventData )->GetData();
            if ( !info.closedInfo.wasOperational )
            {
                refused = true;
                refusedReason = info.closedInfo.reason;
                refusedText = info.closedInfo.text;
            }
        }
        else
        if ( COMCORE::CRpcEndpoint::AcceptedEvent == eventId )
        {
            accepted = true;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** How telling a failure of a candidate is for the person: a service that refused the code says more than one that did not answer */
UInt32
FailureRank( CDevicePairingClient::TPairingFailure failure )
{GUCEF_TRACE;

    switch ( failure )
    {
        case CDevicePairingClient::PAIRINGFAILURE_WRONG_CODE :      return 3;
        case CDevicePairingClient::PAIRINGFAILURE_SERVER_PROOF :    return 2;
        default :                                                   return 1;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{GUCEF_TRACE;

    return CRemoteServiceHost::NowMs();
}

}

/*-------------------------------------------------------------------------*/

void
CDevicePairingClient::RegisterEvents( void )
{GUCEF_TRACE;

    FingerprintEvent.Initialize();
    AwaitingConfirmationEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDevicePairingClient::SRequest::SRequest( void )
    : pairingEndpoint()
    , code()
    , serviceName()
    , drnSessionName()
    , clientKind( "client" )
    , deviceName()
    , platform()
    , appVersion()
    , connectTimeoutMs( 20000 )
    , confirmationTimeoutMs( 300000 )
    , lanDiscovery( true )
    , lanDiscoveryFinder()
{GUCEF_TRACE;

    lanDiscoveryFinder.findMinCandidates = 0;

}

/*-------------------------------------------------------------------------*/

CDevicePairingClient::CDevicePairingClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator.IsNULL() ? CORE::CPulseGenerator::CreateSharedObj() : pulseGenerator, true, false )
    , m_pulse( pulseGenerator )
    , m_lastFailure( PAIRINGFAILURE_NONE )
    , m_fingerprint()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDevicePairingClient::~CDevicePairingClient()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDevicePairingClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CDevicePairingClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CDevicePairingClient::TPairingFailure CDevicePairingClient::GetLastFailure( void ) const { return m_lastFailure; }
const CORE::CString& CDevicePairingClient::GetFingerprint( void ) const { return m_fingerprint; }

/*-------------------------------------------------------------------------*/

bool
CDevicePairingClient::Fail( TPairingFailure failure, const CORE::CString& why, CORE::CString& error )
{GUCEF_TRACE;

    m_lastFailure = failure;
    error = why;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingClient: pairing failed: " << why );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingClient::PumpStep( const CORE::PulseGeneratorPtr& pulseGeneratorRef )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator( pulseGeneratorRef );
    TThreadId driver = pulseGenerator->GetPulseDriverThreadId();
    if ( 0 == driver || driver == MT::GetCurrentThreadID() )
        pulseGenerator->ForceDirectPulse();
    MT::PrecisionDelay( 1 );
}

/*-------------------------------------------------------------------------*/

CDevicePairingClient::SPairingCandidate::SPairingCandidate( void )
    : pairingEndpoint()
    , displayName()
    , instanceId()
    , keyFingerprint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingClient::FindPairingCandidates( const SRequest& request, TPairingCandidateVector& candidates )
{GUCEF_TRACE;

    candidates.clear();
    if ( !request.lanDiscovery || !request.pairingEndpoint.IsNULLOrEmpty() )
        return false;

    // the discovery clients are registered by gucefCOM
    COM::CComGlobal::Instance();

    COMCORE::CServiceFinder::TServiceCandidateVector found;
    if ( m_pulse.IsNULL() )
    {
        COMCORE::CServiceFinder::FindOnce( request.lanDiscoveryFinder, found );
    }
    else
    {
        // on the pulse generator of the client, which this thread drives or nobody does: the look runs on it
        COMCORE::CServiceFinder finder( m_pulse, request.lanDiscoveryFinder );
        if ( !finder.Start() )
            return false;
        const UInt64 startedAt = NowMs();
        const UInt32 wanted = request.lanDiscoveryFinder.findMinCandidates;
        while ( NowMs() - startedAt < request.lanDiscoveryFinder.findTimeoutMs && ( 0 == wanted || finder.GetCandidateCount() < wanted ) )
            PumpStep( m_pulse );
        finder.GetCandidates( found );
        finder.Stop();
    }

    for ( UInt32 i=0; i<found.size(); ++i )
    {
        const COMCORE::CServiceAnnouncement& announcement = found[ i ].announcement;
        if ( !announcement.properties.HasKey( "pairingPort" ) )
            continue;
        const CORE::CString port = announcement.properties.GetValueAlways( "pairingPort" ).AsString();
        const UInt32 portNumber = port.IsNULLOrEmpty() ? 0 : CORE::StringToUInt32( port );
        if ( portNumber < 1 || portNumber > 65535 )
            continue;

        // the host of the first tcp endpoint: the announcement is about the session listener, the pairing listener has the same host
        CORE::CString host;
        for ( UInt32 e=0; e<announcement.endpoints.size() && host.IsNULLOrEmpty(); ++e )
        {
            const CORE::CString& endpoint = announcement.endpoints[ e ];
            if ( !endpoint.StartsWith( "tcp://" ) )
                continue;
            const CORE::CString address = endpoint.CutChars( 6, true );
            const Int32 colon = address.HasChar( ':', false );
            host = colon > 0 ? address.SubstrToIndex( (UInt32) colon, true ) : address;
        }
        if ( host.IsNULLOrEmpty() )
            continue;

        SPairingCandidate candidate;
        candidate.pairingEndpoint = "tcp://" + host + ":" + CORE::ToString( portNumber );
        candidate.displayName = announcement.displayName;
        candidate.instanceId = announcement.instanceId;
        candidate.keyFingerprint = announcement.keyFingerprint;
        candidates.push_back( candidate );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingClient: " << candidates.size() << " pairing candidates on the local network" );
    return !candidates.empty();
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingClient::Pair( const SRequest& request, CDeviceCredentials& result, CORE::CString& error,
                            AUTH::ICredentialStore* store, const CORE::CString& slot )
{GUCEF_TRACE;

    // a given endpoint always wins: nothing is looked for on the network then
    if ( !request.pairingEndpoint.IsNULLOrEmpty() || !request.lanDiscovery )
        return PairAtEndpoint( request, result, error, store, slot );

    m_lastFailure = PAIRINGFAILURE_NONE;
    m_fingerprint.Clear();
    CORE::CString normalisedCode;
    if ( !CDevicePairingCrypto::NormaliseCode( request.code, normalisedCode ) )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the pairing code is not 16 characters of the code alphabet", error );
    normalisedCode.Clear();
    TPairingCandidateVector candidates;
    if ( !FindPairingCandidates( request, candidates ) )
        return Fail( PAIRINGFAILURE_UNREACHABLE, "no pairing endpoint was given and no service that offers pairing was found on the local network", error );

    // The candidates are not trusted. The code and the proof of the service decide: a candidate that cannot prove it
    // knows the code (SERVER_PROOF) or does not answer is passed over, any other outcome is the one of the service
    SRequest candidateRequest( request );
    TPairingFailure bestFailure = PAIRINGFAILURE_UNREACHABLE;
    CORE::CString bestError;
    for ( UInt32 i=0; i<candidates.size(); ++i )
    {
        candidateRequest.pairingEndpoint = candidates[ i ].pairingEndpoint;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingClient: Pairing with the candidate " << candidates[ i ].displayName << " at " << candidates[ i ].pairingEndpoint );
        if ( PairAtEndpoint( candidateRequest, result, error, store, slot ) )
            return true;

        // a candidate that did not answer, did not prove it knows the code or refused the code is passed over: it may be
        // a service that is not the one of the person. Any other outcome is the one of a service that took the pairing up
        if ( PAIRINGFAILURE_UNREACHABLE != m_lastFailure && PAIRINGFAILURE_SERVER_PROOF != m_lastFailure && PAIRINGFAILURE_WRONG_CODE != m_lastFailure )
            return false;
        if ( FailureRank( m_lastFailure ) >= FailureRank( bestFailure ) || bestError.IsNULLOrEmpty() )
        {
            bestFailure = m_lastFailure;
            bestError = error;
        }
    }
    m_lastFailure = bestFailure;
    error = bestError;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingClient::PairAtEndpoint( const SRequest& request, CDeviceCredentials& result, CORE::CString& error,
                                      AUTH::ICredentialStore* store, const CORE::CString& slot )
{GUCEF_TRACE;

    m_lastFailure = PAIRINGFAILURE_NONE;
    m_fingerprint.Clear();
    CORE::CString code;
    if ( !CDevicePairingCrypto::NormaliseCode( request.code, code ) )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the pairing code is not 16 characters of the code alphabet", error );
    if ( request.pairingEndpoint.IsNULLOrEmpty() || request.serviceName.IsNULLOrEmpty() || request.drnSessionName.IsNULLOrEmpty() )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the pairing endpoint, the service name or the session name is missing", error );
    if ( request.deviceName.IsNULLOrEmpty() || request.deviceName.Length() > 64 )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the name of the device has to have 1 to 64 characters", error );
    if ( CDeviceContracts::GetContract().IsNULL() )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the contract gucef.drnutils.devices cannot be built", error );

    // the device key, made before anything goes over the network
    UInt8 seed[ CDevicePairingCrypto::KeySize ];
    UInt8 devicePublicKey[ CDevicePairingCrypto::KeySize ];
    if ( !CDevicePairingCrypto::GenerateDeviceKey( seed, devicePublicKey ) )
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "no signature codec to make a device key (codecspluginMONOCYPHER)", error );
    m_fingerprint = CDevicePairingCrypto::Fingerprint( devicePublicKey );
    CORE::CString authSecretHex;
    UInt8 serverProofKey[ 32 ];
    if ( !CDevicePairingCrypto::DeriveKeys( code, request.serviceName, authSecretHex, serverProofKey ) )
    {
        memset( seed, 0, sizeof( seed ) );
        return Fail( PAIRINGFAILURE_BAD_REQUEST, "the keys of the code cannot be derived", error );
    }
    code.Clear();

    CORE::PulseGeneratorPtr pulse = m_pulse.IsNULL() ? CORE::CPulseGenerator::CreateSharedObj() : m_pulse;
    TPairingFailure failureKind = PAIRINGFAILURE_NONE;
    CORE::CString failure;
    COMCORE::CRpcMessage completeResponse;
    CORE::CString serviceId;
    {
        DRN::CDRNNode node( pulse );
        DRN::CDRNNodeSettings nodeSettings;
        nodeSettings.serviceName = request.serviceName;
        nodeSettings.ownThread = true;
        nodeSettings.connectTimeoutMs = request.connectTimeoutMs;
        nodeSettings.auth.authenticatorType = "shared-secret";
        nodeSettings.auth.SetSetting( "secret", authSecretHex );
        nodeSettings.securityPolicy = CRemoteServiceHost::MakePairingSecurityPolicy();
        authSecretHex.Clear();
        CPairingWatch watch;
        if ( !node.SetSettings( nodeSettings ) )
        {
            memset( seed, 0, sizeof( seed ) );
            return Fail( PAIRINGFAILURE_BAD_REQUEST, "the DRN node of the pairing cannot be set up (no packet protection?)", error );
        }
        node.Subscribe( &watch, DRN::CDRNNode::LinkRefusedEvent );
        node.Subscribe( &watch, DRN::CDRNNode::LinkClosedEvent );
        DRN::CDRNSession* session = node.CreateSession();
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = request.drnSessionName + ".pair";
        DRN::CDRNEndpoint endpoint( request.pairingEndpoint );
        bool joining = false;
        {
            MT::CScopeMutex lock( node.GetLock() );
            joining = session->Join( DRN::CDRNEndpointList( endpoint ), joinSettings, &nodeSettings.auth );
        }
        if ( !joining )
        {
            node.Stop();
            memset( seed, 0, sizeof( seed ) );
            return Fail( PAIRINGFAILURE_UNREACHABLE, "the pairing listener at " + request.pairingEndpoint + " cannot be joined", error );
        }
        UInt64 startedAt = NowMs();
        bool member = false;
        while ( !member && !watch.refused )
        {
            {
                MT::CScopeMutex lock( node.GetLock() );
                DRN::CDRNSession::TSessionState state = session->GetState();
                if ( DRN::CDRNSession::SESSIONSTATE_MEMBER == state )
                    member = true;
                else if ( DRN::CDRNSession::SESSIONSTATE_JOINING != state )
                    break;
            }
            if ( NowMs() - startedAt > request.connectTimeoutMs )
                break;
            if ( !member )
                PumpStep( pulse );
        }
        UInt32 servicePeer = 0;
        UInt8 handshakeKey[ 32 ];
        bool haveHandshakeKey = false;
        if ( member )
        {
            MT::CScopeMutex lock( node.GetLock() );
            servicePeer = session->GetSequencerPeerId();
            DRN::CDRNPeerLinkPtr link = session->GetLinkToMember( servicePeer );
            if ( !link.IsNULL() && GUCEF_NULL != link->GetPeerStaticKey() )
            {
                memcpy( handshakeKey, link->GetPeerStaticKey(), 32 );
                haveHandshakeKey = true;
            }
        }
        if ( !member )
        {
            PumpStep( pulse );
            node.Stop();
            memset( seed, 0, sizeof( seed ) );
            // the pairing listener refuses a link only for its proof (a wrong code) or a ban of the address
            if ( DRN::DRN_LINKREASON_AUTH_FAILED == watch.refusedReason || DRN::DRN_LINKREASON_BANNED == watch.refusedReason ||
                 DRN::DRN_LINKREASON_REFUSED_BY_PEER == watch.refusedReason )
                return Fail( PAIRINGFAILURE_WRONG_CODE, "the service refused the pairing code", error );
            if ( watch.refused && DRN::DRN_LINKREASON_NONE != watch.refusedReason )
                return Fail( PAIRINGFAILURE_UNREACHABLE, "the link to the pairing listener was refused: " + watch.refusedText, error );
            return Fail( PAIRINGFAILURE_UNREACHABLE, "no pairing window answers at " + request.pairingEndpoint, error );
        }

        // the RPC session over the link, with the contract of the pairing
        CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex > transport( GUCEF_NEW DRN::CDRNRpcTransport( *session, servicePeer ) );
        COMCORE::CRpcEndpointSettings endpointSettings;
        endpointSettings.nodeName = request.clientKind + ".pairing";
        endpointSettings.requireSecureTransport = true;
        endpointSettings.autoReconnect = false;
        COMCORE::CRpcEndpointPtr rpc = COMCORE::CRpcEndpoint::Create( pulse, endpointSettings );
        rpc->AddWantedContract( CDeviceContracts::ContractName, CDeviceContracts::Major );
        rpc->Subscribe( &watch, COMCORE::CRpcEndpoint::AcceptedEvent );
        COMCORE::CIRpcTransportPtr transportBase;
        transportBase = transport;
        bool bound = transport->Connect( CORE::CUri() ) && rpc->Start( transportBase, true );
        startedAt = NowMs();
        while ( bound && !( rpc->IsContractBound( CDeviceContracts::ContractName, CDeviceContracts::Major ) && COMCORE::RPC_SESSIONSTATE_ESTABLISHED == rpc->GetState() ) )
        {
            if ( NowMs() - startedAt > request.connectTimeoutMs )
                bound = false;
            else
                PumpStep( pulse );
        }
        if ( !bound )
        {
            failureKind = PAIRINGFAILURE_UNREACHABLE;
            failure = "the RPC session with the pairing listener did not come up";
        }
        else
        {
            // Begin, and the checks of its answer before the key is trusted
            COMCORE::CRpcMessage beginRequest = COMCORE::CRpcMessage();
            {
                COMCORE::CRpcMethodRef ref;
                COMCORE::CRpcStatus status;
                if ( rpc->ResolveMethod( CDeviceContracts::BeginPath, ref, status ) )
                    beginRequest = rpc->NewRequest( ref );
            }
            beginRequest.SetString( 1, request.clientKind );
            CORE::CFutureResult beginFuture = rpc->CallAsync( CDeviceContracts::BeginPath, beginRequest );
            startedAt = NowMs();
            while ( !beginFuture.IsReady() && !beginFuture.HasNoFuture() && NowMs() - startedAt < 30000 )
                PumpStep( pulse );
            COMCORE::CRpcCallResult beginResult;
            if ( !COMCORE::CRpcEndpoint::TryGetCallResult( beginFuture, beginResult ) )
            {
                failureKind = PAIRINGFAILURE_TIMEOUT;
                failure = "the service did not answer Pairing/Begin";
            }
            else if ( !beginResult.status.IsOk() )
            {
                failureKind = PAIRINGFAILURE_REFUSED;
                failure = "the service refused Pairing/Begin: " + beginResult.status.ToString();
            }
            else
            {
                CORE::CDynamicBuffer shownKey;
                CORE::CDynamicBuffer nonce;
                CORE::CDynamicBuffer proof;
                beginResult.response.GetBytes( 2, shownKey );
                beginResult.response.GetBytes( 3, nonce );
                beginResult.response.GetBytes( 4, proof );
                if ( !haveHandshakeKey || 32 != shownKey.GetDataSize() ||
                     !CDevicePairingCrypto::Equals( handshakeKey, shownKey.GetConstBufferPtr(), 32 ) )
                {
                    failureKind = PAIRINGFAILURE_SERVER_PROOF;
                    failure = "the service showed another static key than the one of its handshake";
                }
                else if ( !CDevicePairingCrypto::VerifyServerProof( serverProofKey, handshakeKey, static_cast< const UInt8* >( nonce.GetConstBufferPtr() ), nonce.GetDataSize(),
                                                                    static_cast< const UInt8* >( proof.GetConstBufferPtr() ), proof.GetDataSize() ) )
                {
                    failureKind = PAIRINGFAILURE_SERVER_PROOF;
                    failure = "the service could not prove that it knows the pairing code";
                }
                else
                {
                    // Complete with the proof of the device key
                    serviceId = beginResult.response.GetString( 1 );
                    UInt8 keyProof[ CDevicePairingCrypto::SignatureSize ];
                    if ( !CDevicePairingCrypto::SignDeviceProof( seed, handshakeKey, static_cast< const UInt8* >( nonce.GetConstBufferPtr() ), nonce.GetDataSize(), keyProof ) )
                    {
                        failureKind = PAIRINGFAILURE_BAD_REQUEST;
                        failure = "the device key proof cannot be made";
                    }
                    else
                    {
                        CORE::TCloneableString fingerprintData( m_fingerprint );
                        NotifyObservers( FingerprintEvent, &fingerprintData );
                        COMCORE::CRpcMessage completeRequest;
                        {
                            COMCORE::CRpcMethodRef ref;
                            COMCORE::CRpcStatus status;
                            if ( rpc->ResolveMethod( CDeviceContracts::CompletePath, ref, status ) )
                                completeRequest = rpc->NewRequest( ref );
                        }
                        completeRequest.SetString( 1, request.deviceName );
                        completeRequest.SetString( 2, request.platform );
                        completeRequest.SetString( 3, request.appVersion );
                        completeRequest.SetBytes( 4, devicePublicKey, CDevicePairingCrypto::KeySize );
                        completeRequest.SetBytes( 5, keyProof, CDevicePairingCrypto::SignatureSize );
                        COMCORE::CRpcCallOptions options;
                        options.deadlineMs = request.connectTimeoutMs;
                        options.resultTimeoutMs = request.confirmationTimeoutMs;
                        CORE::CFutureResult completeFuture = rpc->CallAsync( CDeviceContracts::CompletePath, completeRequest, options );
                        bool toldAwaiting = false;
                        startedAt = NowMs();
                        while ( !completeFuture.IsReady() && !completeFuture.HasNoFuture() &&
                                NowMs() - startedAt < (UInt64) request.confirmationTimeoutMs + 5000 )
                        {
                            PumpStep( pulse );
                            if ( watch.accepted && !toldAwaiting )
                            {
                                toldAwaiting = true;
                                NotifyObservers( AwaitingConfirmationEvent );
                            }
                        }
                        COMCORE::CRpcCallResult completeResult;
                        if ( !COMCORE::CRpcEndpoint::TryGetCallResult( completeFuture, completeResult ) )
                        {
                            failureKind = PAIRINGFAILURE_TIMEOUT;
                            failure = "the service did not complete the pairing in time";
                        }
                        else if ( COMCORE::RPC_STATUS_ABORTED == completeResult.status.code &&
                                  CRemoteServiceDetailCodes::GetCode( completeResult.status.details ) == CRemoteServiceDetailCodes::PairingDeclined )
                        {
                            failureKind = PAIRINGFAILURE_DECLINED;
                            failure = "the pairing was declined: " + completeResult.status.message;
                        }
                        else if ( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED == completeResult.status.code )
                        {
                            failureKind = PAIRINGFAILURE_TIMEOUT;
                            failure = "the pairing was not confirmed in time";
                        }
                        else if ( !completeResult.status.IsOk() )
                        {
                            failureKind = PAIRINGFAILURE_REFUSED;
                            failure = "the service refused the pairing: " + completeResult.status.ToString();
                        }
                        else
                        {
                            completeResponse = completeResult.response;
                        }
                    }
                    memset( keyProof, 0, sizeof( keyProof ) );
                }
            }
        }

        // the pairing link ends before anything is kept
        rpc->Close( true );
        for ( UInt32 i=0; i<20; ++i )
            PumpStep( pulse );
        rpc->Unsubscribe( &watch );
        rpc.Unlink();
        transportBase.Unlink();
        transport.Unlink();
        {
            MT::CScopeMutex lock( node.GetLock() );
            session->Leave();
        }
        PumpStep( pulse );
        node.Unsubscribe( &watch );
        node.Stop();
        PumpStep( pulse );
    }
    memset( serverProofKey, 0, sizeof( serverProofKey ) );
    if ( PAIRINGFAILURE_NONE != failureKind )
    {
        memset( seed, 0, sizeof( seed ) );
        return Fail( failureKind, failure, error );
    }

    CDeviceCredentials credentials;
    credentials.serviceName = completeResponse.GetString( 10 );
    if ( credentials.serviceName.IsNULLOrEmpty() )
        credentials.serviceName = request.serviceName;
    credentials.serviceId = completeResponse.GetString( 12 ).IsNULLOrEmpty() ? serviceId : completeResponse.GetString( 12 );
    credentials.drnSessionName = completeResponse.GetString( 11 ).IsNULLOrEmpty() ? request.drnSessionName : completeResponse.GetString( 11 );
    credentials.deviceId = completeResponse.GetString( 1 );
    credentials.token.Set( completeResponse.GetString( 2 ) );
    credentials.tokenExpiresUtcMs = completeResponse.GetUInt64( 3 );
    for ( UInt32 i=0; i<completeResponse.GetCount( 4 ); ++i )
    {
        CORE::CVariant role;
        if ( completeResponse.GetAt( 4, i, role ) )
            credentials.roles.insert( role.AsString() );
    }
    credentials.deviceKind = completeResponse.GetString( 5 );
    for ( UInt32 i=0; i<completeResponse.GetCount( 6 ); ++i )
    {
        CORE::CVariant endpoint;
        if ( completeResponse.GetAt( 6, i, endpoint ) )
            credentials.sessionEndpoints.push_back( endpoint.AsString() );
    }
    CORE::CDynamicBuffer key;
    if ( completeResponse.GetBytes( 7, key ) && 32 == key.GetDataSize() )
        credentials.serverStaticKeyHex = DRN::CDRNCrypto::ToHex( key.GetConstBufferPtr(), 32 ).Lowercase();
    if ( completeResponse.GetBytes( 8, key ) && 32 == key.GetDataSize() )
        credentials.issuerPublicKeyHex = DRN::CDRNCrypto::ToHex( key.GetConstBufferPtr(), 32 ).Lowercase();
    completeResponse.GetDataNode( 9, credentials.securityPolicy );
    credentials.securityPolicy.SetName( "securityPolicy" );
    credentials.devicePublicKeyHex = DRN::CDRNCrypto::ToHex( devicePublicKey, 32 ).Lowercase();
    credentials.deviceSeedHex.Set( DRN::CDRNCrypto::ToHex( seed, 32 ) );
    memset( seed, 0, sizeof( seed ) );
    if ( !credentials.IsComplete() )
        return Fail( PAIRINGFAILURE_REFUSED, "the answer of the service lacks a part of the credentials", error );
    if ( GUCEF_NULL != store && !slot.IsNULLOrEmpty() )
    {
        CORE::CString storeError;
        if ( !credentials.SaveToStore( *store, slot, storeError ) )
            return Fail( PAIRINGFAILURE_STORE, storeError, error );
    }
    result = credentials;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DevicePairingClient: paired as " << result.deviceId << " with " << result.serviceId );
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
