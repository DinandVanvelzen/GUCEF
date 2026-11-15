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

#include "gucefDRNUTILS_CRemoteServiceHost.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

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

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

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

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#include "gucefDRNUTILS_CDeviceContracts.h"
#define GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#endif /* GUCEF_DRNUTILS_CDEVICECONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H
#include "gucefDRNUTILS_CDevicePairingHost.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

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

const char* const CRemoteServiceHost::KeyProviderSuffix = ".host";
const char* const CRemoteServiceHost::LocalAdmin = "local-admin";

namespace {

const char* const StaticKeySlot = "staticKey";
const char* const IssuerSeedSlot = "issuerSeed";
const char* const ServiceIdSlot = "serviceId";

enum EDeviceCommand
{
    DEVICECOMMAND_REVOKE = 1 ,
    DEVICECOMMAND_SET_ROLES ,
    DEVICECOMMAND_RENAME ,
    DEVICECOMMAND_CONFIRM ,
    DEVICECOMMAND_CLOSE_SESSION
};

/** A status with the detail code as the first word of the message and as the "code" of the details */
COMCORE::CRpcStatus
DetailedStatus( COMCORE::TRpcStatusCode code, const char* detailCode, const CORE::CString& message )
{GUCEF_TRACE;

    COMCORE::CRpcStatus status( code, CORE::CString( detailCode ) + ": " + message );
    CRemoteServiceDetailCodes::SetDetails( status.details, detailCode );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinSet( const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CString text;
    for ( CORE::CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        if ( !text.IsNULLOrEmpty() )
            text += ',';
        text += (*i);
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
EndpointsOrLoopback( const CORE::CString::StringVector& configured, UInt16 port )
{GUCEF_TRACE;

    if ( !configured.empty() )
        return configured;
    CORE::CString::StringVector endpoints;
    if ( 0 != port )
        endpoints.push_back( "tcp://127.0.0.1:" + CORE::ToString( port ) );
    return endpoints;
}

/*-------------------------------------------------------------------------*/

CORE::CString
RandomHexId( UInt32 bytes )
{GUCEF_TRACE;

    UInt8 buffer[ 32 ];
    if ( bytes > sizeof( buffer ) || !CDevicePairingCrypto::RandomBytes( buffer, bytes ) )
        return CORE::CString::Empty;
    return DRN::CDRNCrypto::ToHex( buffer, bytes ).Lowercase();
}

}

/*-------------------------------------------------------------------------*/

CRemoteServiceHost::SRingState::SRingState( void )
    : nextEventSeq( 1 )
    , retainedFrom( 1 )
    , retainedBytes( 0 )
    , retainedCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteServiceHost::CRemoteServiceHost( const CORE::PulseGeneratorPtr& pulseGenerator, const CRemoteServiceHostSettings& settings )
    : CORE::CObservingNotifier()
    , CORE::CIServiceResourceSource()
    , m_pulse( pulseGenerator )
    , m_settings( settings )
    , m_stateDirectory()
    , m_credentials( GUCEF_NULL )
    , m_staticPrivateHex()
    , m_serviceId()
    , m_hostInstanceId()
    , m_registry()
    , m_sessions()
    , m_accessRules( GUCEF_NULL )
    , m_accessRulesPtr()
    , m_drnPolicy( m_registry )
    , m_bans()
    , m_node( GUCEF_NULL )
    , m_drnSession( GUCEF_NULL )
    , m_listener()
    , m_rpcHost( GUCEF_NULL )
    , m_services()
    , m_sessionService()
    , m_devicesService()
    , m_pairing( GUCEF_NULL )
    , m_lanAdvertiser( GUCEF_NULL )
    , m_lanProperties()
    , m_commands()
    , m_extensions()
    , m_endpoints()
    , m_eventCalls()
    , m_timer( pulseGenerator, settings.tickIntervalMs > 0 ? settings.tickIntervalMs : 250 )
    , m_pendingDrain( false )
    , m_eventLock()
    , m_lastRegistrySaveMs( 0 )
    , m_drainDeadlineMs( 0 )
    , m_drainHolds( 0 )
    , m_drainExpectedBackSeconds( 0 )
    , m_sessionPort( 0 )
    , m_running( false )
    , m_draining( false )
    , m_drainFinished( false )
{GUCEF_TRACE;

    memset( m_staticPublic, 0, sizeof( m_staticPublic ) );
    memset( m_issuerSeed, 0, sizeof( m_issuerSeed ) );
    memset( m_issuerPublic, 0, sizeof( m_issuerPublic ) );

    m_accessRules = GUCEF_NEW CDeviceAccessRules( m_registry );
    m_accessRulesPtr = COMCORE::CIRpcAccessPolicyPtr( m_accessRules );

    // the rules of the kit's own methods: every active device has a session; the device administration is for admins
    CORE::CString::StringSet none;
    m_accessRules->AddRule( CORE::CString( CRemoteSessionContracts::ContractName ) + "." + CRemoteSessionContracts::ServiceName + "/*", none, none );
    m_accessRules->AddRule( CORE::CString( CDeviceContracts::ContractName ) + "." + CDeviceContracts::DevicesServiceName + "/*", m_settings.adminRoles, none );
    m_accessRules->AddRule( "gucef.rpc.*", none, none );

    m_sessionService.BindMethod( "Hello", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnHello ) );
    m_sessionService.BindMethod( "Resume", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnResume ) );
    m_sessionService.BindMethod( "Attach", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnAttach ) );
    m_sessionService.BindMethod( "Events", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnEvents ) );
    m_sessionService.BindMethod( "Resync", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnResync ) );
    m_sessionService.BindMethod( "Goodbye", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnGoodbye ) );
    m_sessionService.BindMethod( "RenewToken", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnRenewToken ) );

    m_devicesService.BindMethod( "List", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesList ) );
    m_devicesService.BindMethod( "Revoke", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesRevoke ) );
    m_devicesService.BindMethod( "SetRoles", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesSetRoles ) );
    m_devicesService.BindMethod( "Rename", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesRename ) );
    m_devicesService.BindMethod( "OpenPairingWindow", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesOpenPairingWindow ) );
    m_devicesService.BindMethod( "ClosePairingWindow", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesClosePairingWindow ) );
    m_devicesService.BindMethod( "ListPendingPairings", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesListPendingPairings ) );
    m_devicesService.BindMethod( "ConfirmPairing", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesConfirmPairing ) );
    m_devicesService.BindMethod( "ListSessions", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesListSessions ) );
    m_devicesService.BindMethod( "CloseSession", GUCEF_NEW THandler( this, &CRemoteServiceHost::OnDevicesCloseSession ) );

    TEventCallback onTick( this, &CRemoteServiceHost::OnTick );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTick );
}

/*-------------------------------------------------------------------------*/

CRemoteServiceHost::~CRemoteServiceHost()
{GUCEF_TRACE;

    Stop();
    UnsubscribeAllFromObserver();
    memset( m_issuerSeed, 0, sizeof( m_issuerSeed ) );
    m_staticPrivateHex.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteServiceHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteServiceHost";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceHost::NowUtcMs( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceHost::NowMs( void )
{GUCEF_TRACE;

    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        ticksPerMs = 1;
    return MT::PrecisionTickCount() / ticksPerMs;
}

/*-------------------------------------------------------------------------*/

void CRemoteServiceHost::SetStateDirectory( const CORE::CString& stateDirectory ) { m_stateDirectory = stateDirectory; }
void CRemoteServiceHost::SetCredentialStore( AUTH::ICredentialStore* store ) { m_credentials = store; }
bool CRemoteServiceHost::IsRunning( void ) const { return m_running; }
bool CRemoteServiceHost::IsDraining( void ) const { return m_draining; }
bool CRemoteServiceHost::IsDrainComplete( void ) const { return m_draining && m_drainFinished; }
UInt16 CRemoteServiceHost::GetSessionPort( void ) const { return m_sessionPort; }
const CORE::CString& CRemoteServiceHost::GetHostInstanceId( void ) const { return m_hostInstanceId; }
const CORE::CString& CRemoteServiceHost::GetServiceId( void ) const { return m_serviceId; }
const UInt8* CRemoteServiceHost::GetStaticPublicKey( void ) const { return m_staticPublic; }
CORE::CString CRemoteServiceHost::GetStaticPublicKeyHex( void ) const { return DRN::CDRNCrypto::ToHex( m_staticPublic, 32 ).Lowercase(); }
const UInt8* CRemoteServiceHost::GetIssuerPublicKey( void ) const { return m_issuerPublic; }
DRN::CDRNNode* CRemoteServiceHost::GetNode( void ) const { return m_node; }
DRN::CDRNSession* CRemoteServiceHost::GetDrnSession( void ) const { return m_drnSession; }
COMCORE::CRpcHost* CRemoteServiceHost::GetRpcHost( void ) const { return m_rpcHost; }
const CORE::PulseGeneratorPtr& CRemoteServiceHost::GetPulseGenerator( void ) const { return m_pulse; }
CDeviceAccessRules& CRemoteServiceHost::GetAccessRules( void ) { return *m_accessRules; }
CDeviceDrnAccessPolicy& CRemoteServiceHost::GetDrnAccessPolicy( void ) { return m_drnPolicy; }
CDeviceRegistry& CRemoteServiceHost::GetDeviceRegistry( void ) { return m_registry; }
CRemoteSessionCache& CRemoteServiceHost::GetSessionCache( void ) { return m_sessions; }
DRN::CDRNBanList& CRemoteServiceHost::GetBanList( void ) { return m_bans; }
const CRemoteServiceHostSettings& CRemoteServiceHost::GetSettings( void ) const { return m_settings; }
const AUTH::CSecretString& CRemoteServiceHost::GetStaticPrivateKeyHex( void ) const { return m_staticPrivateHex; }
CDevicePairingHost* CRemoteServiceHost::GetPairingHost( void ) const { return m_pairing; }
CORE::CString CRemoteServiceHost::GetResourceSourceName( void ) const { return "remoteHost"; }

/*-------------------------------------------------------------------------*/

COMCORE::CRpcServiceSetPtr
CRemoteServiceHost::GetServiceSet( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_rpcHost ? m_rpcHost->GetServiceSet() : COMCORE::CRpcServiceSetPtr();
}

/*-------------------------------------------------------------------------*/

UInt16
CRemoteServiceHost::GetPairingPort( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_pairing ? m_pairing->GetPort() : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CRemoteServiceHost::GetSessionEndpoints( void ) const
{GUCEF_TRACE;

    return EndpointsOrLoopback( m_settings.sessionEndpoints, m_sessionPort );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CRemoteServiceHost::GetPairingEndpoints( void ) const
{GUCEF_TRACE;

    return EndpointsOrLoopback( m_settings.pairingEndpoints, GetPairingPort() );
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceHost::GetNodeHeartbeatAgeMs( void ) const
{GUCEF_TRACE;

    if ( !m_running || GUCEF_NULL == m_node || !m_node->IsRunningOwnThread() )
        return 0;
    CORE::PulseGeneratorPtr pulseGenerator = m_node->GetWorkPulseGenerator();
    if ( pulseGenerator.IsNULL() )
        return 0;
    const UInt64 lastPulse = pulseGenerator->GetTickCountAtLastPulse();
    const UInt64 now = MT::PrecisionTickCount();
    if ( now <= lastPulse )
        return 0;
    return ( now - lastPulse ) * 1000 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSecurityPolicy
CRemoteServiceHost::MakeSessionSecurityPolicy( void ) const
{GUCEF_TRACE;

    DRN::CDRNSecurityPolicy policy;
    policy.requireEncryption = true;
    policy.requireSignedTokens = true;
    policy.allowedAuthenticators.clear();
    policy.allowedAuthenticators.insert( DRN::CDRNTokenAuthenticator::MethodName );
    policy.limits.maxPendingHandshakes = m_settings.maxPendingHandshakes;
    policy.limits.maxHandshakesPerAddressPerMinute = m_settings.maxHandshakesPerAddressPerMinute;
    return policy;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNSecurityPolicy
CRemoteServiceHost::MakePairingSecurityPolicy( void )
{GUCEF_TRACE;

    DRN::CDRNSecurityPolicy policy;
    policy.requireEncryption = true;
    policy.allowedAuthenticators.clear();
    policy.allowedAuthenticators.insert( "shared-secret" );
    policy.limits.maxPendingHandshakes = 16;
    policy.limits.maxHandshakesPerAddressPerMinute = 30;
    return policy;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetSessionPolicyDocument( CORE::CDataNode& policy ) const
{GUCEF_TRACE;

    policy.Clear();
    MakeSessionSecurityPolicy().SaveConfig( policy );
    policy.SetName( "securityPolicy" );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::LoadKeys( CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CString provider = m_settings.serviceName + KeyProviderSuffix;
    UInt8 staticPrivate[ 32 ];
    bool fromStore = false;

    // the static x25519 key: settings, store, or made (and stored when there is a store)
    if ( !m_settings.staticPrivateKeyHex.IsNULLOrEmpty() )
    {
        if ( !DRN::CDRNCrypto::FromHex( m_settings.staticPrivateKeyHex, staticPrivate, 32 ) )
        {
            error = "the setting staticPrivateKeyHex is not 64 hexadecimal characters";
            return false;
        }
    }
    else
    {
        AUTH::CSecretString text;
        if ( GUCEF_NULL != m_credentials && m_credentials->Load( provider, StaticKeySlot, text ) && DRN::CDRNCrypto::FromHex( text.Reveal(), staticPrivate, 32 ) )
        {
            fromStore = true;
        }
        else
        {
            if ( !DRN::CDRNCrypto::GenerateKeyAgreementKeyPair( staticPrivate, m_staticPublic ) )
            {
                error = "no static key can be made (no key agreement codec: load codecspluginMONOCYPHER)";
                return false;
            }
            if ( GUCEF_NULL != m_credentials )
            {
                AUTH::CSecretString made( DRN::CDRNCrypto::ToHex( staticPrivate, 32 ) );
                if ( !m_credentials->Store( provider, StaticKeySlot, made ) )
                {
                    memset( staticPrivate, 0, sizeof( staticPrivate ) );
                    error = "the credential store did not keep the static key";
                    return false;
                }
                fromStore = true;
            }
        }
    }
    if ( !DRN::CDRNCrypto::GetKeyAgreementPublicKey( staticPrivate, m_staticPublic ) )
    {
        memset( staticPrivate, 0, sizeof( staticPrivate ) );
        error = "the public part of the static key cannot be computed";
        return false;
    }
    m_staticPrivateHex.Set( DRN::CDRNCrypto::ToHex( staticPrivate, 32 ) );
    memset( staticPrivate, 0, sizeof( staticPrivate ) );

    // the token issuer's Ed25519 seed
    if ( !m_settings.issuerSeedHex.IsNULLOrEmpty() )
    {
        if ( !DRN::CDRNCrypto::FromHex( m_settings.issuerSeedHex, m_issuerSeed, 32 ) )
        {
            error = "the setting issuerSeedHex is not 64 hexadecimal characters";
            return false;
        }
    }
    else
    {
        AUTH::CSecretString text;
        if ( GUCEF_NULL == m_credentials || !m_credentials->Load( provider, IssuerSeedSlot, text ) || !DRN::CDRNCrypto::FromHex( text.Reveal(), m_issuerSeed, 32 ) )
        {
            if ( !DRN::CDRNCrypto::GenerateSignatureKeyPair( m_issuerSeed, m_issuerPublic ) )
            {
                error = "no token issuer key can be made (no signature codec: load codecspluginMONOCYPHER)";
                return false;
            }
            if ( GUCEF_NULL != m_credentials )
            {
                AUTH::CSecretString made( DRN::CDRNCrypto::ToHex( m_issuerSeed, 32 ) );
                if ( !m_credentials->Store( provider, IssuerSeedSlot, made ) )
                {
                    error = "the credential store did not keep the token issuer key";
                    return false;
                }
            }
        }
    }
    if ( !DRN::CDRNCrypto::GetSignaturePublicKey( m_issuerSeed, m_issuerPublic ) )
    {
        error = "the public part of the token issuer key cannot be computed";
        return false;
    }

    // the service id: stable with the keys
    AUTH::CSecretString idText;
    if ( GUCEF_NULL != m_credentials && m_credentials->Load( provider, ServiceIdSlot, idText ) && !idText.Reveal().IsNULLOrEmpty() )
    {
        m_serviceId = idText.Reveal();
    }
    else
    {
        // without a store the id follows from the static key, so a host given the same key keeps its id
        UInt8 digest[ 32 ];
        DRN::CDRNCrypto::Sha256( m_staticPublic, 32, digest );
        m_serviceId = "s-" + CDevicePairingCrypto::ToCrockfordBase32( digest, 16, 20 );
        if ( GUCEF_NULL != m_credentials )
            m_credentials->Store( provider, ServiceIdSlot, AUTH::CSecretString( m_serviceId ) );
    }
    if ( !fromStore && GUCEF_NULL == m_credentials && m_settings.staticPrivateKeyHex.IsNULLOrEmpty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceHost: no credential store and no keys in the settings: the keys are new for this run, clients have to pair again after a restart" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::Start( CORE::CString& error )
{GUCEF_TRACE;

    if ( m_running )
        return true;
    if ( m_pulse.IsNULL() )
    {
        error = "the remote service host has no pulse generator";
        return false;
    }
    if ( CRemoteSessionContracts::GetContract().IsNULL() || CDeviceContracts::GetContract().IsNULL() )
    {
        error = "the contracts of the remote service kit cannot be built";
        return false;
    }
    if ( !LoadKeys( error ) )
        return false;

    if ( m_stateDirectory.IsNULLOrEmpty() )
    {
        m_registry.SetInMemory();
    }
    else
    {
        if ( !m_registry.Load( m_stateDirectory, error ) )
        {
            error = "the device registry cannot be read: " + error;
            return false;
        }
        const CORE::CString bansPath = CORE::CombinePath( m_stateDirectory, "bans.txt" );
        if ( !CORE::FileExists( bansPath ) )
        {
            CORE::CString problem;
            CORE::CreateOwnerOnlyFile( bansPath, "", 0, problem );
        }
        m_bans.LoadFromFile( bansPath );
        m_bans.SetFilePath( bansPath );
    }

    CRemoteSessionCache::SLimits limits;
    limits.maxSessions = m_settings.maxSessions;
    limits.maxSessionsPerDevice = m_settings.maxSessionsPerDevice;
    limits.maxAttachedPerSession = m_settings.maxBulkLinksPerSession;
    limits.budgetBytes = m_settings.sessionBudgetBytes;
    limits.resumeGraceMs = (UInt64) m_settings.resumeGraceSeconds * 1000;
    limits.eventRetentionEvents = m_settings.eventRetentionEvents;
    limits.eventRetentionBytes = m_settings.eventRetentionBytes;
    m_sessions.SetLimits( limits );
    m_commands.SetBounds( 3600000, 256, 4096 );
    m_hostInstanceId = RandomHexId( 8 );
    if ( m_hostInstanceId.IsNULLOrEmpty() )
    {
        error = "no randomness for the host instance id";
        return false;
    }

    // the node of the session listener: its own thread, the static key, signed device tokens only
    DRN::CDRNNodeSettings nodeSettings;
    nodeSettings.serviceName = m_settings.serviceName;
    nodeSettings.ownThread = true;
    nodeSettings.linkTimeoutMs = m_settings.linkTimeoutMs;
    nodeSettings.echoIntervalMs = m_settings.echoIntervalMs;
    nodeSettings.handshakeTimeoutMs = m_settings.handshakeTimeoutMs;
    nodeSettings.auth.authenticatorType = DRN::CDRNTokenAuthenticator::MethodName;
    nodeSettings.auth.SetSetting( "issuerPublicKey", DRN::CDRNCrypto::ToHex( m_issuerPublic, 32 ).Lowercase() );
    nodeSettings.auth.SetSetting( "expectedSession", m_settings.drnSessionName );
    nodeSettings.securityPolicy = MakeSessionSecurityPolicy();
    nodeSettings.staticPrivateKey = m_staticPrivateHex.Reveal();
    m_node = GUCEF_NEW DRN::CDRNNode( m_pulse );
    if ( !m_node->SetSettings( nodeSettings ) )
    {
        error = "the DRN node of the remote service host cannot be set up (packet protection and signatures need codecspluginMONOCYPHER)";
        Stop();
        return false;
    }
    m_node->SetAccessPolicy( &m_drnPolicy );
    m_node->SetBanList( &m_bans );
    CORE::CString host = m_settings.bindAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : m_settings.bindAddress;
    DRN::CDRNEndpoint endpoint( "tcp://" + host + ":" + CORE::ToString( m_settings.sessionPort ) );
    if ( !m_settings.bindAddress.IsNULLOrEmpty() )
        endpoint.GetParams().Set( "bindAddress", CORE::CVariant( m_settings.bindAddress ) );
    if ( !m_node->Listen( endpoint ) )
    {
        error = "the remote service host cannot listen on port " + CORE::ToString( m_settings.sessionPort );
        Stop();
        return false;
    }
    DRN::CDRNEndpointList listening = m_node->GetListenEndpoints();
    m_sessionPort = listening.GetCount() > 0 ? listening.Get( 0 ).GetPort() : m_settings.sessionPort;
    m_drnSession = m_node->CreateSession();
    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = m_settings.drnSessionName;
    sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        if ( !m_drnSession->Host( sessionSettings ) )
        {
            error = "the DRN session " + m_settings.drnSessionName + " cannot be hosted";
            Stop();
            return false;
        }
    }

    // the RPC host on the members of the session: the larger of the control and bulk limits, the client sets its own per channel
    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings.nodeName = m_settings.serviceName;
    hostSettings.endpointSettings.requireSecureTransport = true;
    hostSettings.endpointSettings.limits.resumeGraceMs = m_settings.resumeGraceSeconds * 1000;
    hostSettings.endpointSettings.limits.maxMessageSize = m_settings.controlMaxMessageBytes > m_settings.bulkMaxMessageBytes ? m_settings.controlMaxMessageBytes : m_settings.bulkMaxMessageBytes;
    hostSettings.endpointSettings.limits.receiveWindowBytes = m_settings.controlWindowBytes > m_settings.bulkWindowBytes ? m_settings.controlWindowBytes : m_settings.bulkWindowBytes;
    hostSettings.endpointSettings.limits.maxConcurrentCalls = m_settings.maxConcurrentCalls;
    hostSettings.endpointSettings.sendQueueMaxBytes = m_settings.controlSendQueueBytes > m_settings.bulkSendQueueBytes ? m_settings.controlSendQueueBytes : m_settings.bulkSendQueueBytes;
    hostSettings.maxSessions = 0 == m_settings.maxSessions ? 0 : m_settings.maxSessions * ( 1 + m_settings.maxBulkLinksPerSession );
    hostSettings.maxSessionsPerSubject = 0 == m_settings.maxSessionsPerDevice ? 0 : m_settings.maxSessionsPerDevice * ( 1 + m_settings.maxBulkLinksPerSession );
    m_rpcHost = GUCEF_NEW COMCORE::CRpcHost( m_pulse, hostSettings );
    m_rpcHost->SetAccessPolicy( m_accessRulesPtr );
    if ( !m_rpcHost->RegisterService( CRemoteSessionContracts::GetContract(), CRemoteSessionContracts::ServiceName, &m_sessionService, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) ||
         !m_rpcHost->RegisterService( CDeviceContracts::GetContract(), CDeviceContracts::DevicesServiceName, &m_devicesService, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) )
    {
        error = "the services of the remote service kit cannot be registered";
        Stop();
        return false;
    }
    TEventCallback onCreated( this, &CRemoteServiceHost::OnEndpointCreated );
    SubscribeTo( m_rpcHost, COMCORE::CRpcHost::EndpointCreatedEvent, onCreated );
    TEventCallback onClosed( this, &CRemoteServiceHost::OnEndpointClosed );
    SubscribeTo( m_rpcHost, COMCORE::CRpcHost::EndpointClosedEvent, onClosed );
    DRN::CDRNRpcTransportListener* listener = GUCEF_NEW DRN::CDRNRpcTransportListener( *m_drnSession );
    CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex > typedListener( listener );
    m_listener = typedListener;
    if ( !m_rpcHost->AddListener( m_listener ) || !listener->Listen( CORE::CUri() ) )
    {
        error = "the RPC listener of the remote service host cannot be started";
        Stop();
        return false;
    }

    m_pairing = GUCEF_NEW CDevicePairingHost( *this, m_pulse );
    m_running = true;
    m_draining = false;
    m_drainFinished = false;
    m_timer.SetEnabled( true );
    StartLanDiscovery();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: " << m_settings.serviceName << " listens on port " << m_sessionPort << " as " << m_serviceId
               << " (instance " << m_hostInstanceId << "), " << m_registry.GetActiveCount() << " active devices" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::RegisterService( const COMCORE::CRpcContractPtr& contract, const CORE::CString& serviceName,
                                     COMCORE::CIRpcServiceImpl* impl, COMCORE::TRpcExecutionMode mode )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_rpcHost )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceHost: services are registered after Start()" );
        return false;
    }
    return m_rpcHost->RegisterService( contract, serviceName, impl, mode );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::AddSessionExtension( CIRemoteSessionExtension* extension )
{GUCEF_TRACE;

    if ( GUCEF_NULL == extension )
        return;
    for ( UInt32 i=0; i<m_extensions.size(); ++i )
    {
        if ( m_extensions[ i ] == extension )
            return;
    }
    m_extensions.push_back( extension );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::RemoveSessionExtension( CIRemoteSessionExtension* extension )
{GUCEF_TRACE;

    for ( TExtensionVector::iterator i = m_extensions.begin(); i != m_extensions.end(); ++i )
    {
        if ( (*i) == extension )
        {
            m_extensions.erase( i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::BeginDrain( const CORE::CString& reason, UInt32 expectedBackSeconds )
{GUCEF_TRACE;

    if ( !m_running || m_draining )
        return;
    m_draining = true;
    m_drainFinished = false;
    m_drainExpectedBackSeconds = 0 == expectedBackSeconds ? m_settings.expectedBackSeconds : expectedBackSeconds;
    m_drainDeadlineMs = NowMs() + (UInt64) m_settings.drainSeconds * 1000;

    COMCORE::CRpcMessage event = NewEvent( CRemoteSessionContracts::EventKindServiceStopping );
    CORE::CDataNode data( "data", GUCEF_DATATYPE_OBJECT );
    data.SetAttribute( "reason", reason );
    data.SetAttribute( "expectedBackSeconds", m_drainExpectedBackSeconds );
    data.SetAttribute( "retryAfterMs", (UInt64) m_drainExpectedBackSeconds * 1000 );
    event.SetDataNode( 6, data );
    PublishEventToAll( event );

    // no pairing from now on; the session listener stays: links that are there finish their work (stopping the
    // listener of the node ends its accepted links too), and a new client gets UNAVAILABLE DRAINING at its Hello
    if ( GUCEF_NULL != m_pairing )
        m_pairing->Shutdown();
    // a service that drains is no longer offered to clients that look for one
    StopLanDiscovery();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: draining (" << reason << "), " << m_sessions.GetCount() << " client sessions told, back in about "
               << m_drainExpectedBackSeconds << " s" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::SetLanDiscoveryProperties( const CORE::CValueList& properties )
{GUCEF_TRACE;

    m_lanProperties = properties;
    if ( GUCEF_NULL != m_lanAdvertiser && m_lanAdvertiser->IsAdvertising() )
    {
        COMCORE::CServiceAnnouncement announcement;
        MakeLanAnnouncement( announcement );
        m_lanAdvertiser->Update( announcement );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::MakeLanAnnouncement( COMCORE::CServiceAnnouncement& announcement ) const
{GUCEF_TRACE;

    const COMCORE::CServiceAdvertiserSettings& lan = m_settings.lanDiscoveryAdvertiser;
    announcement.serviceType = lan.serviceType;
    announcement.instanceId = lan.instanceId.IsNULLOrEmpty() ? m_serviceId : lan.instanceId;
    announcement.displayName = lan.displayName.IsNULLOrEmpty() ? m_settings.serviceName : lan.displayName;
    announcement.ttlInSecs = lan.ttlInSecs;

    // the address the announcement is heard from first: that is the address the asker reached, then what the
    // configuration names (a DNS name such as worker.lan survives a change of the node address)
    announcement.endpoints.clear();
    if ( 0 != m_sessionPort )
        announcement.endpoints.push_back( COMCORE::CServiceAnnouncement::MakeEndpoint( "tcp", CORE::CString::Empty, m_sessionPort ) );
    for ( UInt32 i=0; i<m_settings.sessionEndpoints.size() && announcement.endpoints.size() < COMCORE::CServiceAnnouncement::MaxEndpoints; ++i )
    {
        bool known = false;
        for ( UInt32 n=0; n<announcement.endpoints.size() && !known; ++n )
            known = announcement.endpoints[ n ] == m_settings.sessionEndpoints[ i ];
        if ( !known )
            announcement.endpoints.push_back( m_settings.sessionEndpoints[ i ] );
    }

    announcement.protocolVersion = m_settings.lanDiscoveryProtocolVersion.IsNULLOrEmpty() ? CORE::ToString( CRemoteSessionContracts::Major ) : m_settings.lanDiscoveryProtocolVersion;
    announcement.keyFingerprint = m_settings.lanDiscoveryKeyFingerprint ? CDevicePairingCrypto::Fingerprint( m_staticPublic ) : CORE::CString::Empty;
    announcement.properties = m_lanProperties;
    announcement.properties.SetAllowMultipleValues( false );
    if ( m_settings.lanDiscoveryPairingPort && 0 != m_settings.pairingPort )
        announcement.properties.Set( "pairingPort", CORE::ToString( m_settings.pairingPort ) );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::IsAdvertisingOnLan( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_lanAdvertiser && m_lanAdvertiser->IsAdvertising();
}

/*-------------------------------------------------------------------------*/

COMCORE::CServiceAdvertiser*
CRemoteServiceHost::GetLanAdvertiser( void ) const
{GUCEF_TRACE;

    return m_lanAdvertiser;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::StartLanDiscovery( void )
{GUCEF_TRACE;

    if ( !m_settings.lanDiscovery || GUCEF_NULL != m_lanAdvertiser )
        return;

    // the discovery clients are registered by gucefCOM
    COM::CComGlobal::Instance();

    m_lanAdvertiser = GUCEF_NEW COMCORE::CServiceAdvertiser( m_pulse, m_settings.lanDiscoveryAdvertiser );
    COMCORE::CServiceAnnouncement announcement;
    MakeLanAnnouncement( announcement );
    if ( !m_lanAdvertiser->Advertise( announcement ) )
    {
        // a convenience only: the service runs on, clients use the configured endpoints
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: " << m_settings.serviceName << " is not announced on the local network" );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::StopLanDiscovery( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_lanAdvertiser )
        return;
    m_lanAdvertiser->Withdraw();
    GUCEF_DELETE m_lanAdvertiser;
    m_lanAdvertiser = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::FinishDrain( void )
{GUCEF_TRACE;

    if ( m_drainFinished )
        return;
    m_drainFinished = true;
    TRemoteSessionVector sessions;
    m_sessions.GetSessions( sessions );
    for ( UInt32 i=0; i<sessions.size(); ++i )
        CloseClientSession( sessions[ i ], "the service stops", true );
    // RPC sessions without a client session (attached bulk channels, a session that never said Hello)
    if ( GUCEF_NULL != m_rpcHost )
    {
        GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
        m_rpcHost->GetEndpoints( endpoints );
        for ( UInt32 i=0; i<endpoints.size(); ++i )
        {
            if ( COMCORE::RPC_SESSIONSTATE_CLOSED != endpoints[ i ]->GetState() )
                endpoints[ i ]->Close( true );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: drained, " << sessions.size() << " client sessions ended with GOODBYE" );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::RefuseIfDraining( COMCORE::CRpcServerCall& call ) const
{GUCEF_TRACE;

    if ( !m_draining )
        return false;
    COMCORE::CRpcStatus status = DetailedStatus( COMCORE::RPC_STATUS_UNAVAILABLE, CRemoteServiceDetailCodes::Draining, "the service stops" );
    status.details.SetAttribute( "retryAfterMs", (UInt64) ( 0 == m_drainExpectedBackSeconds ? m_settings.expectedBackSeconds : m_drainExpectedBackSeconds ) * 1000 );
    call.Fail( status );
    return true;
}

void
CRemoteServiceHost::AddDrainHold( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_eventLock );
    ++m_drainHolds;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::ReleaseDrainHold( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_eventLock );
    if ( m_drainHolds > 0 )
        --m_drainHolds;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteServiceHost::GetDrainHoldCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_eventLock );
    return m_drainHolds;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::Stop( void )
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    StopLanDiscovery();
    if ( GUCEF_NULL != m_pairing )
    {
        m_pairing->Shutdown();
        GUCEF_DELETE m_pairing;
        m_pairing = GUCEF_NULL;
    }
    TRemoteSessionVector closed;
    m_sessions.CloseAll( closed );
    for ( UInt32 i=0; i<closed.size(); ++i )
        DetachEventsCall( closed[ i ], GUCEF_NULL );
    NotifyClosed( closed, "the host stops" );
    if ( GUCEF_NULL != m_rpcHost )
    {
        UnsubscribeFrom( m_rpcHost );
        GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
        m_rpcHost->GetEndpoints( endpoints );
        for ( UInt32 i=0; i<endpoints.size(); ++i )
            UnsubscribeFrom( endpoints[ i ].GetPointerAlways() );
        m_rpcHost->Stop();
        GUCEF_DELETE m_rpcHost;
        m_rpcHost = GUCEF_NULL;
    }
    m_endpoints.clear();
    {
        MT::CScopeMutex lock( m_eventLock );
        m_eventCalls.clear();
    }
    if ( !m_listener.IsNULL() )
    {
        m_listener->StopListening();
        m_listener.Unlink();
    }
    if ( GUCEF_NULL != m_node )
    {
        m_node->Stop();
        m_node->SetAccessPolicy( GUCEF_NULL );
        m_node->SetBanList( GUCEF_NULL );
        GUCEF_DELETE m_node;
        m_node = GUCEF_NULL;
        m_drnSession = GUCEF_NULL;
    }
    if ( m_running && m_registry.IsDirty() )
    {
        CORE::CString error;
        if ( !m_registry.Save( error ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: the device registry was not saved: " << error );
    }
    m_sessionPort = 0;
    m_running = false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::IsOnPulseThread( void ) const
{GUCEF_TRACE;

    TThreadId driver = m_pulse->GetPulseDriverThreadId();
    return 0 != driver && driver == MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::FindEndpointPeer( const COMCORE::CRpcEndpoint& endpoint, UInt32& peerId ) const
{GUCEF_TRACE;

    // after a resume over a new link the endpoint keeps the principal of its first transport: the transport is asked first
    COMCORE::CIRpcTransportPtr transport = endpoint.GetTransport();
    if ( !transport.IsNULL() )
    {
        peerId = transport->GetPeerPrincipal().attributes.GetValueAlways( "drn.peerId" ).AsUInt32( 0, true );
        if ( 0 != peerId )
            return true;
    }
    peerId = endpoint.GetPeerPrincipal().attributes.GetValueAlways( "drn.peerId" ).AsUInt32( 0, true );
    return 0 != peerId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteServiceHost::GetPeerAddress( UInt32 peerId ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_drnSession || 0 == peerId )
        return CORE::CString::Empty;
    MT::CScopeMutex lock( m_node->GetLock() );
    DRN::CDRNPeerLinkPtr link = m_drnSession->GetLinkToMember( peerId );
    return link.IsNULL() ? CORE::CString::Empty : link->GetRemoteEndpoint().GetHost();
}

/*-------------------------------------------------------------------------*/

CRemoteSessionPtr
CRemoteServiceHost::GetCallSession( const COMCORE::CRpcServerCall& call ) const
{GUCEF_TRACE;

    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !call.GetSessionId( sid ) )
        return CRemoteSessionPtr();
    return m_sessions.FindByRpcSession( sid );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::GetCallPrincipal( const COMCORE::CRpcServerCall& call, CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    const COMCORE::CRpcPrincipal& rpc = call.GetPrincipal();
    CORE::CString deviceId = CDeviceAccessRules::DeviceIdOfPrincipal( rpc );
    CDeviceRecord device;
    if ( deviceId.IsNULLOrEmpty() || !m_registry.Find( deviceId, device ) || !device.IsActive() )
        return false;
    principal.deviceId = deviceId;
    principal.userId = device.userId;
    principal.deviceName = device.name;
    principal.deviceKind = device.kind;
    principal.roles = device.roles;
    principal.subject = rpc.subject;
    principal.peerId = 0;
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    bool haveSid = call.GetSessionId( sid );
    if ( haveSid && GUCEF_NULL != m_rpcHost )
    {
        COMCORE::CRpcEndpointPtr endpoint = m_rpcHost->FindEndpoint( sid );
        if ( !endpoint.IsNULL() )
            FindEndpointPeer( *endpoint, principal.peerId );
    }
    if ( 0 == principal.peerId )
        principal.peerId = rpc.attributes.GetValueAlways( "drn.peerId" ).AsUInt32( 0, true );
    principal.address = GetPeerAddress( principal.peerId );
    principal.sessionId.Clear();
    principal.channel.Clear();
    if ( haveSid )
    {
        bool attached = false;
        CRemoteSessionPtr session = m_sessions.FindByRpcSession( sid, &attached );
        if ( !session.IsNULL() && session->GetDeviceId() == deviceId )
        {
            principal.sessionId = session->GetSessionId();
            principal.channel = attached ? "bulk" : "control";
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::IsAdmin( const CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    for ( CORE::CString::StringSet::const_iterator i = m_settings.adminRoles.begin(); i != m_settings.adminRoles.end(); ++i )
    {
        if ( principal.HasRole( (*i) ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::FailWithDetail( COMCORE::CRpcServerCall& call, COMCORE::TRpcStatusCode code, const char* detailCode, const CORE::CString& message ) const
{GUCEF_TRACE;

    call.Fail( DetailedStatus( code, detailCode, message ) );
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcMessage
CRemoteServiceHost::NewEvent( const CORE::CString& kind ) const
{GUCEF_TRACE;

    COMCORE::CRpcContractPtr contract = CRemoteSessionContracts::GetContract();
    if ( contract.IsNULL() )
        return COMCORE::CRpcMessage();
    COMCORE::CRpcMessage event( contract, contract->FindType( "Event" ) );
    event.SetString( CRemoteSessionEventRing::KindField, kind );
    return event;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceHost::PublishEvent( const CORE::CString& sessionId, const COMCORE::CRpcMessage& event )
{GUCEF_TRACE;

    CRemoteSessionPtr session = m_sessions.Find( sessionId );
    if ( session.IsNULL() || !event.IsBound() )
        return 0;
    UInt64 eventSeq = 0;
    {
        MT::CScopeMutex lock( m_eventLock );
        COMCORE::CRpcMessage copy( event );
        eventSeq = session.GetPointerAlways()->m_ring.Append( copy, NowUtcMs() );
    }
    if ( IsOnPulseThread() )
        DrainEvents( session );
    else
        m_pendingDrain.Store( true );
    return eventSeq;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::PublishEventToAll( const COMCORE::CRpcMessage& event )
{GUCEF_TRACE;

    TRemoteSessionVector sessions;
    m_sessions.GetSessions( sessions );
    for ( UInt32 i=0; i<sessions.size(); ++i )
        PublishEvent( sessions[ i ]->GetSessionId(), event );
}

/*-------------------------------------------------------------------------*/

CRemoteServiceHost::SRingState
CRemoteServiceHost::GetRingState( const CORE::CString& sessionId ) const
{GUCEF_TRACE;

    SRingState state;
    CRemoteSessionPtr session = m_sessions.Find( sessionId );
    if ( session.IsNULL() )
        return state;
    MT::CScopeMutex lock( m_eventLock );
    const CRemoteSessionEventRing& ring = session->GetEventRing();
    state.nextEventSeq = ring.GetNextSeq();
    state.retainedFrom = ring.GetRetainedFrom();
    state.retainedBytes = ring.GetRetainedBytes();
    state.retainedCount = ring.GetRetainedCount();
    return state;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::DrainEvents( const CRemoteSessionPtr& sessionRef )
{GUCEF_TRACE;

    CRemoteSessionPtr sessionCopy( sessionRef );
    CRemoteSession* session = sessionCopy.GetPointerAlways();
    MT::CScopeMutex lock( m_eventLock );
    COMCORE::CRpcServerCallPtr call = session->m_eventsCall;
    if ( call.IsNULL() )
        return;
    while ( session->m_nextToSend < session->m_ring.GetNextSeq() )
    {
        const CRemoteSessionEventRing::SEvent* event = session->m_ring.Find( session->m_nextToSend );
        if ( GUCEF_NULL == event )
        {
            // the ring dropped what this stream had not sent yet: the client resyncs
            COMCORE::CRpcStatus gap = DetailedStatus( COMCORE::RPC_STATUS_RESUME_GAP, "RESUME_GAP", "the events between are no longer kept" );
            gap.details.SetAttribute( "scope", "events" );
            gap.details.SetAttribute( "from", session->m_nextToSend );
            gap.details.SetAttribute( "to", session->m_ring.GetRetainedFrom() - 1 );
            m_eventCalls.erase( call.GetPointerAlways() );
            session->m_eventsCall.Unlink();
            call->Fail( gap );
            return;
        }
        if ( !call->Write( event->message ) )
            return;     // no credit: WritableEvent goes on
        ++session->m_nextToSend;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::DetachEventsCall( const CRemoteSessionPtr& sessionRef, const COMCORE::CRpcStatus* finishWith )
{GUCEF_TRACE;

    CRemoteSessionPtr sessionCopy( sessionRef );
    CRemoteSession* session = sessionCopy.GetPointerAlways();
    COMCORE::CRpcServerCallPtr call;
    {
        MT::CScopeMutex lock( m_eventLock );
        call = session->m_eventsCall;
        if ( call.IsNULL() )
            return;
        m_eventCalls.erase( call.GetPointerAlways() );
        session->m_eventsCall.Unlink();
    }
    UnsubscribeFrom( call.GetPointerAlways() );
    if ( GUCEF_NULL != finishWith )
        call->Finish( *finishWith );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnEventsWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString sessionId;
    {
        MT::CScopeMutex lock( m_eventLock );
        TEventCallMap::iterator i = m_eventCalls.find( static_cast< COMCORE::CRpcServerCall* >( notifier ) );
        if ( i == m_eventCalls.end() )
            return;
        sessionId = (*i).second;
    }
    CRemoteSessionPtr session = m_sessions.Find( sessionId );
    if ( !session.IsNULL() )
        DrainEvents( session );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnEventsCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString sessionId;
    {
        MT::CScopeMutex lock( m_eventLock );
        TEventCallMap::iterator i = m_eventCalls.find( static_cast< COMCORE::CRpcServerCall* >( notifier ) );
        if ( i == m_eventCalls.end() )
            return;
        sessionId = (*i).second;
        m_eventCalls.erase( i );
        CRemoteSessionPtr session = m_sessions.Find( sessionId );
        if ( !session.IsNULL() && session->m_eventsCall.GetPointerAlways() == notifier )
            session.GetPointerAlways()->m_eventsCall.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::NotifyClosed( const TRemoteSessionVector& sessions, const CORE::CString& reason )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sessions.size(); ++i )
    {
        for ( UInt32 e=0; e<m_extensions.size(); ++e )
            m_extensions[ e ]->OnSessionClosed( sessions[ i ], reason );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::BindEndpoint( const UInt8* rpcSessionId, const CORE::CString& sessionId, bool attached )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_rpcHost )
        return;
    COMCORE::CRpcEndpointPtr endpoint = m_rpcHost->FindEndpoint( rpcSessionId );
    if ( endpoint.IsNULL() )
        return;
    SEndpointBinding binding;
    binding.sessionId = sessionId;
    binding.attached = attached;
    m_endpoints[ endpoint.GetPointerAlways() ] = binding;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::UnbindEndpoint( COMCORE::CRpcEndpoint* endpoint, SEndpointBinding& binding, bool& found )
{GUCEF_TRACE;

    TEndpointBindingMap::iterator i = m_endpoints.find( endpoint );
    found = i != m_endpoints.end();
    if ( !found )
        return;
    binding = (*i).second;
    m_endpoints.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::CloseClientSession( const CRemoteSessionPtr& session, const CORE::CString& reason, bool closeRpc )
{GUCEF_TRACE;

    COMCORE::CRpcStatus ended( COMCORE::RPC_STATUS_OK, reason );
    DetachEventsCall( session, &ended );

    // the RPC sessions of the client session: the control channel and the attached ones
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    if ( GUCEF_NULL != m_rpcHost )
    {
        for ( TEndpointBindingMap::iterator i = m_endpoints.begin(); i != m_endpoints.end(); )
        {
            if ( (*i).second.sessionId == session->GetSessionId() )
            {
                UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
                COMCORE::CRpcEndpointPtr endpoint;
                if ( (*i).first->GetSessionId( sid ) )
                    endpoint = m_rpcHost->FindEndpoint( sid );
                if ( !endpoint.IsNULL() )
                    endpoints.push_back( endpoint );
                m_endpoints.erase( i++ );
            }
            else
            {
                ++i;
            }
        }
    }
    m_sessions.Close( session );
    if ( closeRpc )
    {
        for ( UInt32 i=0; i<endpoints.size(); ++i )
            endpoints[ i ]->Close( true );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: client session " << session->GetSessionId().SubstrToIndex( 8, true ) << "... of device "
               << session->GetDeviceId() << " closed: " << reason );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
        m_extensions[ e ]->OnSessionClosed( session, reason );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::SuspendClientSession( const CRemoteSessionPtr& session, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( REMOTESESSION_ACTIVE != session->GetState() )
        return;
    m_sessions.Suspend( session, NowMs() );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: client session " << session->GetSessionId().SubstrToIndex( 8, true ) << "... suspended: " << reason );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
        m_extensions[ e ]->OnSessionSuspended( session );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnEndpointCreated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    COMCORE::CRpcEndpointPtr endpoint = static_cast< COMCORE::CRpcEndpointEventData* >( eventData )->endpoint;
    if ( endpoint.IsNULL() )
        return;
    TEventCallback onSuspended( this, &CRemoteServiceHost::OnRpcSuspended );
    SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionSuspendedEvent, onSuspended );
    TEventCallback onResumed( this, &CRemoteServiceHost::OnRpcResumed );
    SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionResumedEvent, onResumed );
    TEventCallback onGap( this, &CRemoteServiceHost::OnRpcGap );
    SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionGapEvent, onGap );
    TEventCallback onClosed( this, &CRemoteServiceHost::OnRpcClosed );
    SubscribeTo( endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionClosedEvent, onClosed );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnEndpointClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    COMCORE::CRpcEndpointPtr endpoint = static_cast< COMCORE::CRpcEndpointEventData* >( eventData )->endpoint;
    if ( endpoint.IsNULL() )
        return;
    UnsubscribeFrom( endpoint.GetPointerAlways() );
    SEndpointBinding binding;
    bool found = false;
    UnbindEndpoint( endpoint.GetPointerAlways(), binding, found );
    if ( !found )
        return;
    CRemoteSessionPtr session = m_sessions.Find( binding.sessionId );
    if ( session.IsNULL() )
        return;
    if ( binding.attached )
    {
        UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
        if ( endpoint->GetSessionId( sid ) )
            m_sessions.Detach( sid );
        return;
    }
    DetachEventsCall( session, GUCEF_NULL );
    SuspendClientSession( session, "the RPC session ended without Goodbye" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnRpcSuspended( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TEndpointBindingMap::iterator i = m_endpoints.find( static_cast< COMCORE::CRpcEndpoint* >( notifier ) );
    if ( i == m_endpoints.end() || (*i).second.attached )
        return;     // an attached channel stays attached while its RPC session waits for its resume
    CRemoteSessionPtr session = m_sessions.Find( (*i).second.sessionId );
    if ( !session.IsNULL() )
        SuspendClientSession( session, "the link was lost" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnRpcResumed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CRpcEndpoint* endpoint = static_cast< COMCORE::CRpcEndpoint* >( notifier );
    TEndpointBindingMap::iterator i = m_endpoints.find( endpoint );
    if ( i == m_endpoints.end() || (*i).second.attached )
        return;
    CRemoteSessionPtr session = m_sessions.Find( (*i).second.sessionId );
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( session.IsNULL() || !endpoint->GetSessionId( sid ) )
        return;

    // case A: the RPC session goes on over a new member link; the client session follows it
    UInt32 peerId = 0;
    FindEndpointPeer( *endpoint, peerId );
    m_sessions.Bind( session, sid, peerId, NowMs() );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
        m_extensions[ e ]->OnSessionResumed( session );
    DrainEvents( session );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnRpcGap( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    SEndpointBinding binding;
    bool found = false;
    UnbindEndpoint( static_cast< COMCORE::CRpcEndpoint* >( notifier ), binding, found );
    if ( !found )
        return;
    CRemoteSessionPtr session = m_sessions.Find( binding.sessionId );
    if ( session.IsNULL() )
        return;
    if ( binding.attached )
        return;     // the attached channel is gone with its old RPC session; the client attaches the new one
    // the RPC session was replaced: the client resumes its client session on the new one (case B)
    DetachEventsCall( session, GUCEF_NULL );
    SuspendClientSession( session, "the RPC session had a gap" );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnRpcClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CRpcEndpoint* endpoint = static_cast< COMCORE::CRpcEndpoint* >( notifier );
    SEndpointBinding binding;
    bool found = false;
    UnbindEndpoint( endpoint, binding, found );
    if ( !found )
        return;
    CRemoteSessionPtr session = m_sessions.Find( binding.sessionId );
    if ( session.IsNULL() )
        return;
    if ( binding.attached )
    {
        UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
        if ( endpoint->GetSessionId( sid ) )
            m_sessions.Detach( sid );
        return;
    }
    DetachEventsCall( session, GUCEF_NULL );
    SuspendClientSession( session, "the RPC session ended without Goodbye" );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnHello( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !GetCallPrincipal( call, principal ) || !call.GetSessionId( sid ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( RefuseIfDraining( call ) )
        return COMCORE::RPC_HANDLER_DONE;

    // a second Hello on the same RPC session ends the client session it had
    bool attached = false;
    CRemoteSessionPtr previous = m_sessions.FindByRpcSession( sid, &attached );
    if ( !previous.IsNULL() )
    {
        if ( attached )
        {
            FailWithDetail( call, COMCORE::RPC_STATUS_FAILED_PRECONDITION, CRemoteServiceDetailCodes::ChannelLimit, "an attached channel cannot open a client session" );
            return COMCORE::RPC_HANDLER_DONE;
        }
        CloseClientSession( previous, "a new Hello on the same RPC session", false );
    }

    TRemoteSessionVector evicted;
    CORE::CString refusal;
    CRemoteSessionPtr session = m_sessions.Open( principal.deviceId, principal.userId, principal.deviceKind, sid, principal.peerId,
                                                 NowUtcMs(), NowMs(), evicted, refusal );
    for ( UInt32 i=0; i<evicted.size(); ++i )
        DetachEventsCall( evicted[ i ], GUCEF_NULL );
    NotifyClosed( evicted, "evicted for a new session" );
    if ( session.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: Hello of device " << principal.deviceId << " refused: " << refusal );
        FailWithDetail( call, COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, CRemoteServiceDetailCodes::SessionLimit, "the device has the most client sessions it may have" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CRemoteSession* writable = session.GetPointerAlways();
    writable->m_clientKind = request.GetString( 1 );
    writable->m_clientVersion = request.GetString( 2 );
    for ( UInt32 i=0; i<request.GetCount( 3 ); ++i )
    {
        CORE::CVariant capability;
        if ( request.GetAt( 3, i, capability ) )
            writable->m_capabilities.insert( capability.AsString() );
    }
    BindEndpoint( sid, session->GetSessionId(), false );
    m_registry.NoteSeen( principal.deviceId, NowUtcMs(), principal.address );

    CDeviceRecord device;
    m_registry.Find( principal.deviceId, device );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( 1, session->GetSessionId() );
    response.SetBytes( 2, session->m_resumeSecret, CRemoteSession::ResumeSecretSize );
    response.SetString( 3, principal.deviceId );
    response.SetString( 4, principal.deviceKind );
    for ( CORE::CString::StringSet::const_iterator r = principal.roles.begin(); r != principal.roles.end(); ++r )
        response.Add( 5, CORE::CVariant( (*r) ) );
    response.SetString( 6, m_settings.serviceName );
    response.SetString( 7, m_settings.serviceVersion );
    response.SetString( 8, m_hostInstanceId );
    response.SetUInt64( 9, NowUtcMs() );
    response.SetUInt32( 10, m_settings.resumeGraceSeconds );
    response.SetUInt64( 11, GetRingState( session->GetSessionId() ).nextEventSeq - 1 );
    CORE::CDataNode limits( "limits", GUCEF_DATATYPE_OBJECT );
    limits.SetAttribute( "eventRetentionEvents", m_settings.eventRetentionEvents );
    limits.SetAttribute( "eventRetentionBytes", m_settings.eventRetentionBytes );
    limits.SetAttribute( "maxBulkLinksPerSession", m_settings.maxBulkLinksPerSession );
    limits.SetAttribute( "controlWindowBytes", m_settings.controlWindowBytes );
    limits.SetAttribute( "controlMaxMessageBytes", m_settings.controlMaxMessageBytes );
    limits.SetAttribute( "bulkWindowBytes", m_settings.bulkWindowBytes );
    limits.SetAttribute( "bulkMaxMessageBytes", m_settings.bulkMaxMessageBytes );
    limits.SetAttribute( "linkTimeoutMs", m_settings.linkTimeoutMs );
    response.SetDataNode( 12, limits );
    CORE::CString::StringVector endpoints = GetSessionEndpoints();
    for ( UInt32 i=0; i<endpoints.size(); ++i )
        response.Add( 14, CORE::CVariant( endpoints[ i ] ) );
    response.SetUInt64( 15, device.tokenExpiresUtcMs );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: client session " << session->GetSessionId().SubstrToIndex( 8, true ) << "... opened for device "
               << principal.deviceId << " (" << writable->m_clientKind << ")" );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
        m_extensions[ e ]->OnSessionOpened( session );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::FillExtensions( const CRemoteSessionPtr& session, bool resync, CORE::CDataNode& extensions ) const
{GUCEF_TRACE;

    extensions.SetName( "extensions" );
    extensions.SetNodeType( GUCEF_DATATYPE_OBJECT );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
    {
        CORE::CDataNode child( m_extensions[ e ]->GetExtensionName(), GUCEF_DATATYPE_OBJECT );
        if ( resync )
            m_extensions[ e ]->FillResync( session, child );
        else
            m_extensions[ e ]->FillResume( session, child );
        extensions.AddChild( child );
    }
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnResume( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !GetCallPrincipal( call, principal ) || !call.GetSessionId( sid ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString sessionId = request.GetString( 1 );
    CORE::CDynamicBuffer secret;
    request.GetBytes( 2, secret );
    CRemoteSessionPtr session = m_sessions.Find( sessionId );

    // another device, a wrong secret and an unknown session look the same: a session id alone reveals nothing
    if ( session.IsNULL() || session->GetDeviceId() != principal.deviceId || !session->IsResumeSecret( secret.GetConstBufferPtr(), secret.GetDataSize() ) )
    {
        if ( !session.IsNULL() )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: a Resume of device " << principal.deviceId << " was refused (another device or a wrong secret)" );
        FailWithDetail( call, COMCORE::RPC_STATUS_NOT_FOUND, CRemoteServiceDetailCodes::SessionUnknown, "the client session is unknown" );
        return COMCORE::RPC_HANDLER_DONE;
    }

    // the session moves to this RPC session: a stream of the old one ends
    DetachEventsCall( session, GUCEF_NULL );
    for ( TEndpointBindingMap::iterator i = m_endpoints.begin(); i != m_endpoints.end(); )
    {
        if ( (*i).second.sessionId == sessionId && !(*i).second.attached )
            m_endpoints.erase( i++ );
        else
            ++i;
    }
    bool attached = false;
    CRemoteSessionPtr previous = m_sessions.FindByRpcSession( sid, &attached );
    if ( !previous.IsNULL() && previous != session && !attached )
        CloseClientSession( previous, "another session was resumed on its RPC session", false );
    m_sessions.Bind( session, sid, principal.peerId, NowMs() );
    BindEndpoint( sid, sessionId, false );
    m_registry.NoteSeen( principal.deviceId, NowUtcMs(), principal.address );

    SRingState ring = GetRingState( sessionId );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetUInt64( 1, ring.nextEventSeq - 1 );
    response.SetUInt64( 2, ring.retainedFrom );
    response.SetString( 3, m_hostInstanceId );
    response.SetUInt64( 4, NowUtcMs() );
    CORE::CDataNode extensions;
    FillExtensions( session, false, extensions );
    response.SetDataNode( 5, extensions );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: client session " << sessionId.SubstrToIndex( 8, true ) << "... resumed on a new RPC session" );
    for ( UInt32 e=0; e<m_extensions.size(); ++e )
        m_extensions[ e ]->OnSessionResumed( session );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnAttach( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( !GetCallPrincipal( call, principal ) || !call.GetSessionId( sid ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( RefuseIfDraining( call ) )
        return COMCORE::RPC_HANDLER_DONE;
    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString sessionId = request.GetString( 1 );
    CORE::CDynamicBuffer secret;
    request.GetBytes( 2, secret );
    CRemoteSessionPtr session = m_sessions.Find( sessionId );
    if ( session.IsNULL() || session->GetDeviceId() != principal.deviceId || !session->IsResumeSecret( secret.GetConstBufferPtr(), secret.GetDataSize() ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_NOT_FOUND, CRemoteServiceDetailCodes::SessionUnknown, "the client session is unknown" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( session->IsBoundTo( sid ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_FAILED_PRECONDITION, CRemoteServiceDetailCodes::ChannelLimit, "the control channel cannot be attached to its own session" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    UInt32 channelIndex = 0;
    if ( !m_sessions.Attach( session, sid, channelIndex ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, CRemoteServiceDetailCodes::ChannelLimit,
                        "the client session has the most attached channels it may have (" + CORE::ToString( m_settings.maxBulkLinksPerSession ) + ")" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    BindEndpoint( sid, sessionId, true );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetUInt32( 1, channelIndex );
    CORE::CDataNode limits( "limits", GUCEF_DATATYPE_OBJECT );
    limits.SetAttribute( "bulkWindowBytes", m_settings.bulkWindowBytes );
    limits.SetAttribute( "bulkMaxMessageBytes", m_settings.bulkMaxMessageBytes );
    response.SetDataNode( 2, limits );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: channel " << channelIndex << " (" << request.GetString( 3 ) << ") attached to client session "
               << sessionId.SubstrToIndex( 8, true ) << "..." );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnEvents( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteSessionPtr session = GetCallSession( call );
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    if ( session.IsNULL() || !call.GetSessionId( sid ) || !session->IsBoundTo( sid ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_NOT_FOUND, CRemoteServiceDetailCodes::SessionUnknown, "Session/Hello or Session/Resume first, on the control channel" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    UInt64 from = call.GetRequest().GetUInt64( 1 );
    SRingState ring = GetRingState( session->GetSessionId() );
    if ( 0 == from || from > ring.nextEventSeq )
        from = ring.nextEventSeq;
    if ( from < ring.retainedFrom )
    {
        COMCORE::CRpcStatus gap = DetailedStatus( COMCORE::RPC_STATUS_RESUME_GAP, "RESUME_GAP", "the events asked for are no longer kept" );
        gap.details.SetAttribute( "scope", "events" );
        gap.details.SetAttribute( "from", from );
        gap.details.SetAttribute( "to", ring.retainedFrom - 1 );
        call.Fail( gap );
        return COMCORE::RPC_HANDLER_DONE;
    }

    // one stream per session: a new one takes over
    COMCORE::CRpcStatus replaced( COMCORE::RPC_STATUS_CANCELLED, "replaced by a new event stream" );
    DetachEventsCall( session, &replaced );
    COMCORE::CRpcServerCallPtr shared = call.GetSharedPtr();
    {
        MT::CScopeMutex lock( m_eventLock );
        session.GetPointerAlways()->m_eventsCall = shared;
        session.GetPointerAlways()->m_nextToSend = from;
        m_eventCalls[ shared.GetPointerAlways() ] = session->GetSessionId();
    }
    TEventCallback onWritable( this, &CRemoteServiceHost::OnEventsWritable );
    SubscribeTo( shared.GetPointerAlways(), COMCORE::CRpcServerCall::WritableEvent, onWritable );
    TEventCallback onCancelled( this, &CRemoteServiceHost::OnEventsCancelled );
    SubscribeTo( shared.GetPointerAlways(), COMCORE::CRpcServerCall::CancelledEvent, onCancelled );
    DrainEvents( session );
    return COMCORE::RPC_HANDLER_DEFERRED;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnResync( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteSessionPtr session = GetCallSession( call );
    if ( session.IsNULL() )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_NOT_FOUND, CRemoteServiceDetailCodes::SessionUnknown, "Session/Hello or Session/Resume first" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetUInt64( 1, GetRingState( session->GetSessionId() ).nextEventSeq - 1 );
    response.SetString( 2, m_hostInstanceId );
    response.SetUInt64( 3, NowUtcMs() );
    CORE::CDataNode extensions;
    FillExtensions( session, true, extensions );
    response.SetDataNode( 5, extensions );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnGoodbye( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    bool attached = false;
    UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
    CRemoteSessionPtr session;
    if ( call.GetSessionId( sid ) )
        session = m_sessions.FindByRpcSession( sid, &attached );
    if ( !session.IsNULL() )
    {
        if ( attached )
        {
            m_sessions.Detach( sid );
            TEndpointBindingMap::iterator i = m_endpoints.begin();
            for ( ; i != m_endpoints.end(); ++i )
            {
                UInt8 other[ GUCEF_RPC_SESSIONID_SIZE ];
                if ( (*i).first->GetSessionId( other ) && 0 == memcmp( other, sid, sizeof( sid ) ) )
                {
                    m_endpoints.erase( i );
                    break;
                }
            }
        }
        else
        {
            CloseClientSession( session, "Goodbye: " + call.GetRequest().GetString( 1 ), false );
        }
    }
    call.Reply( call.NewResponse() );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteServiceHost::IssueToken( CDeviceRecord& device, UInt64& expiresUtcMs ) const
{GUCEF_TRACE;

    expiresUtcMs = 0;
    if ( !device.IsActive() )
        return CORE::CString::Empty;
    UInt8 devicePublic[ 32 ];
    if ( !DRN::CDRNCrypto::FromHex( device.publicKey, devicePublic, 32 ) )
        return CORE::CString::Empty;
    const UInt64 expirySeconds = NowUtcMs() / 1000 + (UInt64) m_settings.tokenLifetimeDays * 86400;
    CORE::CString token = DRN::CDRNTokenAuthenticator::IssueSignedToken( m_issuerSeed, m_settings.drnSessionName, device.deviceId,
                                                                         device.GetRolesText(), expirySeconds, devicePublic );
    if ( !token.IsNULLOrEmpty() )
    {
        expiresUtcMs = expirySeconds * 1000;
        device.tokenExpiresUtcMs = expiresUtcMs;
    }
    return token;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnRenewToken( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    CDeviceRecord device;
    if ( !GetCallPrincipal( call, principal ) || !m_registry.Find( principal.deviceId, device ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const UInt64 renewWindowMs = (UInt64) m_settings.renewBeforeDays * 86400 * 1000;
    const UInt64 renewFrom = device.tokenExpiresUtcMs > renewWindowMs ? device.tokenExpiresUtcMs - renewWindowMs : 0;
    if ( 0 != device.tokenExpiresUtcMs && NowUtcMs() < renewFrom )
    {
        COMCORE::CRpcStatus status = DetailedStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, CRemoteServiceDetailCodes::TokenRenewTooEarly,
                                                     "the token is renewed in the last " + CORE::ToString( m_settings.renewBeforeDays ) + " days of its life" );
        status.details.SetAttribute( "renewFromUtcMs", renewFrom );
        call.Fail( status );
        return COMCORE::RPC_HANDLER_DONE;
    }
    UInt64 expiresUtcMs = 0;
    CORE::CString token = IssueToken( device, expiresUtcMs );
    if ( token.IsNULLOrEmpty() )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INTERNAL, "no token can be issued" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    m_registry.Put( device );
    SaveRegistry();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: the token of device " << device.deviceId << " was renewed" );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( 1, token );
    response.SetUInt64( 2, expiresUtcMs );
    token.Clear();
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::SaveRegistry( void )
{GUCEF_TRACE;

    m_lastRegistrySaveMs = NowMs();
    CORE::CString error;
    if ( !m_registry.Save( error ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: the device registry was not saved: " << error );
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteServiceHost::GetUsedBytes( void ) const
{GUCEF_TRACE;

    TRemoteSessionVector sessions;
    m_sessions.GetSessions( sessions );
    UInt64 bytes = 0;
    for ( UInt32 i=0; i<sessions.size(); ++i )
    {
        {
            MT::CScopeMutex lock( m_eventLock );
            bytes += sessions[ i ]->GetEventRing().GetRetainedBytes();
        }
        for ( UInt32 e=0; e<m_extensions.size(); ++e )
            bytes += m_extensions[ e ]->GetSessionBytes( sessions[ i ] );
    }
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::OnTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_running )
        return;
    if ( GUCEF_NULL != m_pairing )
        m_pairing->OnTick();

    TRemoteSessionVector ended;
    m_sessions.ExpireSuspended( NowMs(), ended );
    for ( UInt32 i=0; i<ended.size(); ++i )
    {
        DetachEventsCall( ended[ i ], GUCEF_NULL );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: client session " << ended[ i ]->GetSessionId().SubstrToIndex( 8, true ) << "... closed: the resume grace ended" );
    }
    NotifyClosed( ended, "the resume grace ended" );
    ended.clear();
    m_sessions.EnforceBudget( GetUsedBytes(), ended );
    for ( UInt32 i=0; i<ended.size(); ++i )
        DetachEventsCall( ended[ i ], GUCEF_NULL );
    NotifyClosed( ended, "evicted: the session cache is over its budget" );

    if ( m_pendingDrain.Load() )
    {
        m_pendingDrain.Store( false );
        TRemoteSessionVector sessions;
        m_sessions.GetSessions( sessions );
        for ( UInt32 i=0; i<sessions.size(); ++i )
            DrainEvents( sessions[ i ] );
    }

    if ( m_draining && !m_drainFinished )
    {
        // done when the drain time is over, or when no call is open besides the event streams
        bool callsOpen = GetDrainHoldCount() > 0;
        if ( GUCEF_NULL != m_rpcHost )
        {
            GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
            m_rpcHost->GetEndpoints( endpoints );
            for ( UInt32 i=0; i<endpoints.size() && !callsOpen; ++i )
            {
                UInt32 open = endpoints[ i ]->GetOpenCallCount();
                UInt8 sid[ GUCEF_RPC_SESSIONID_SIZE ];
                bool attached = false;
                CRemoteSessionPtr session;
                if ( endpoints[ i ]->GetSessionId( sid ) )
                    session = m_sessions.FindByRpcSession( sid, &attached );
                if ( !session.IsNULL() && !attached && !session->m_eventsCall.IsNULL() && open > 0 )
                    --open;
                callsOpen = open > 0;
            }
        }
        if ( !callsOpen || NowMs() >= m_drainDeadlineMs )
            FinishDrain();
    }

    // last seen and address of joins: saved now and then, never at every join
    if ( m_registry.IsDirty() && NowMs() - m_lastRegistrySaveMs >= 5000 )
        SaveRegistry();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::OpenPairingWindow( const CPairingWindowRequest& request, const CORE::CString& openedBy, CORE::CString& windowId,
                                       CORE::CString& code, UInt64& expiresUtcMs, CORE::CString& error )
{GUCEF_TRACE;

    if ( !m_running || GUCEF_NULL == m_pairing )
    {
        error = "the remote service host does not run";
        return false;
    }
    if ( m_draining )
    {
        error = "the service drains: no pairing";
        return false;
    }
    return m_pairing->Open( request, openedBy, windowId, code, expiresUtcMs, error );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::ClosePairingWindow( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pairing || !m_pairing->IsOpen() )
        return false;
    m_pairing->Close( reason );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::IsPairingWindowOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_pairing && m_pairing->IsOpen();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::ConfirmPairing( const CORE::CString& deviceId, const CORE::CString& fingerprint, bool approve, const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pairing )
    {
        error = "the remote service host does not run";
        return false;
    }
    return m_pairing->Confirm( deviceId, fingerprint, approve, by, error );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetPendingPairings( CDeviceRegistry::TDeviceRecordVector& pending ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_pairing )
        m_pairing->GetPending( pending );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::CloseDeviceLinks( const CORE::CString& deviceId )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_node || GUCEF_NULL == m_drnSession )
        return;
    MT::CScopeMutex lock( m_node->GetLock() );
    DRN::TDRNSessionMemberInfoVector members;
    m_drnSession->GetMembers( members );
    for ( UInt32 i=0; i<members.size(); ++i )
    {
        if ( members[ i ].isLocal )
            continue;
        if ( m_drnSession->GetMemberAuthResult( members[ i ].peerId ).userId != deviceId )
            continue;
        DRN::CDRNPeerLinkPtr link = m_drnSession->GetLinkToMember( members[ i ].peerId );
        if ( !link.IsNULL() )
            link->Close( DRN::DRN_LINKREASON_BANNED, "the device was revoked" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::RevokeDevice( const CORE::CString& deviceId, const CORE::CString& reason, const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    CDeviceRecord device;
    if ( !m_registry.Find( deviceId, device ) )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": the device " + deviceId + " is not in the registry";
        return false;
    }

    // 1. the registry is authoritative: the join policy and the access rules refuse from now on
    device.status = DEVICESTATUS_REVOKED;
    device.revokedUtcMs = NowUtcMs();
    device.revokedBy = by;
    device.revokedReason = reason;
    m_registry.Put( device );
    SaveRegistry();

    // 2. bans of the device id and its key
    m_bans.Ban( DRN::DRN_BAN_USER_ID, deviceId, 0, "revoked" );
    if ( !device.publicKey.IsNULLOrEmpty() )
        m_bans.Ban( DRN::DRN_BAN_PUBLIC_KEY, device.publicKey.Lowercase(), 0, "revoked" );

    // 3. its sessions without grace, its links, a pairing of it that waits
    TRemoteSessionVector sessions;
    m_sessions.GetSessionsOfDevice( deviceId, sessions );
    for ( UInt32 i=0; i<sessions.size(); ++i )
        CloseClientSession( sessions[ i ], "the device was revoked", true );
    CloseDeviceLinks( deviceId );
    if ( GUCEF_NULL != m_pairing )
        m_pairing->DeclinePending( deviceId, "the device was revoked" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: device " << deviceId << " revoked by " << by );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::SetDeviceRoles( const CORE::CString& deviceId, const CORE::CString::StringSet& roles, const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    CDeviceRecord device;
    if ( !m_registry.Find( deviceId, device ) )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": the device " + deviceId + " is not in the registry";
        return false;
    }
    if ( DEVICESTATUS_REVOKED == device.status )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::DeviceNotActive ) + ": the device " + deviceId + " is revoked";
        return false;
    }
    device.roles = roles;
    m_registry.Put( device );
    SaveRegistry();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteServiceHost: the roles of device " << deviceId << " are " << JoinSet( roles ) << " (set by " << by << ")" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::RenameDevice( const CORE::CString& deviceId, const CORE::CString& name, const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    CDeviceRecord device;
    if ( name.IsNULLOrEmpty() || name.Length() > 64 )
    {
        error = "a device name has 1 to 64 characters";
        return false;
    }
    if ( !m_registry.Find( deviceId, device ) )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": the device " + deviceId + " is not in the registry";
        return false;
    }
    device.name = name;
    m_registry.Put( device );
    SaveRegistry();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHost::CloseSession( const CORE::CString& sessionId, const CORE::CString& by, CORE::CString& error )
{GUCEF_TRACE;

    CRemoteSessionPtr session = m_sessions.Find( sessionId );
    if ( session.IsNULL() )
    {
        error = CORE::CString( CRemoteServiceDetailCodes::SessionUnknown ) + ": no client session " + sessionId;
        return false;
    }
    CloseClientSession( session, "closed by " + by, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetDevicesDocument( CORE::CDataNode& devices ) const
{GUCEF_TRACE;

    devices.Clear();
    devices.SetName( "devices" );
    devices.SetNodeType( GUCEF_DATATYPE_ARRAY );
    CDeviceRegistry::TDeviceRecordVector records;
    m_registry.GetAll( records );
    for ( UInt32 i=0; i<records.size(); ++i )
    {
        CORE::CDataNode node;
        records[ i ].ToDataNode( node );
        devices.AddChild( node );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetSessionsDocument( CORE::CDataNode& sessions ) const
{GUCEF_TRACE;

    sessions.Clear();
    sessions.SetName( "sessions" );
    sessions.SetNodeType( GUCEF_DATATYPE_ARRAY );
    TRemoteSessionVector all;
    m_sessions.GetSessions( all );
    for ( UInt32 i=0; i<all.size(); ++i )
    {
        SRingState ring = GetRingState( all[ i ]->GetSessionId() );
        CORE::CDataNode* node = sessions.AddChild( "session", GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "sessionId", all[ i ]->GetSessionId() );
        node->SetAttribute( "deviceId", all[ i ]->GetDeviceId() );
        node->SetAttribute( "clientKind", all[ i ]->GetClientKind() );
        node->SetAttribute( "state", RemoteSessionStateToString( all[ i ]->GetState() ) );
        node->SetAttribute( "openedUtcMs", all[ i ]->GetOpenedUtcMs() );
        node->SetAttribute( "attachedChannels", all[ i ]->GetAttachedCount() );
        node->SetAttribute( "eventSeq", ring.nextEventSeq - 1 );
        node->SetAttribute( "eventsRetained", ring.retainedCount );
        node->SetAttribute( "eventBytes", ring.retainedBytes );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetInfo( CORE::CValueList& info ) const
{GUCEF_TRACE;

    info.Set( "remoteHost.running", CORE::CVariant( m_running ) );
    info.Set( "remoteHost.draining", CORE::CVariant( m_draining ) );
    info.Set( "remoteHost.serviceId", CORE::CVariant( m_serviceId ) );
    info.Set( "remoteHost.hostInstanceId", CORE::CVariant( m_hostInstanceId ) );
    info.Set( "remoteHost.sessionPort", CORE::CVariant( (UInt32) m_sessionPort ) );
    info.Set( "remoteHost.pairingWindowOpen", CORE::CVariant( IsPairingWindowOpen() ) );
    info.Set( "remoteHost.devices", CORE::CVariant( m_registry.GetCount() ) );
    info.Set( "remoteHost.activeDevices", CORE::CVariant( m_registry.GetActiveCount() ) );
    CORE::CValueList counters;
    GetResourceCounters( counters );
    for ( UInt32 i=0; i<counters.GetKeyCount(); ++i )
        info.Set( "remoteHost." + counters.GetKey( i ).AsString(), counters.GetValue( i ) );
}

/*-------------------------------------------------------------------------*/

void
CRemoteServiceHost::GetResourceCounters( CORE::CValueList& counters ) const
{GUCEF_TRACE;

    counters.Set( "sessions", CORE::CVariant( m_sessions.GetCount() ) );
    counters.Set( "sessionsActive", CORE::CVariant( m_sessions.GetActiveCount() ) );
    counters.Set( "sessionsSuspended", CORE::CVariant( m_sessions.GetSuspendedCount() ) );
    counters.Set( "attachedChannels", CORE::CVariant( m_sessions.GetAttachedCount() ) );
    counters.Set( "rpcSessions", CORE::CVariant( GUCEF_NULL != m_rpcHost ? m_rpcHost->GetEndpointCount() : (UInt32) 0 ) );
    counters.Set( "sessionBytes", CORE::CVariant( GetUsedBytes() ) );
    UInt32 links = 0;
    if ( GUCEF_NULL != m_node )
        links = m_node->GetLinkCount( true );
    counters.Set( "links", CORE::CVariant( links ) );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnDevicesList( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    COMCORE::CRpcMessage response = call.NewResponse();
    CORE::CDataNode devices;
    GetDevicesDocument( devices );
    response.SetDataNode( 1, devices );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnDevicesListPendingPairings( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    COMCORE::CRpcMessage response = call.NewResponse();
    CORE::CDataNode devices( "devices", GUCEF_DATATYPE_ARRAY );
    CDeviceRegistry::TDeviceRecordVector pending;
    GetPendingPairings( pending );
    for ( UInt32 i=0; i<pending.size(); ++i )
    {
        CORE::CDataNode node;
        pending[ i ].ToDataNode( node );
        devices.AddChild( node );
    }
    response.SetDataNode( 1, devices );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnDevicesListSessions( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    COMCORE::CRpcMessage response = call.NewResponse();
    CORE::CDataNode sessions;
    GetSessionsDocument( sessions );
    response.SetDataNode( 1, sessions );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnDevicesOpenPairingWindow( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    if ( !GetCallPrincipal( call, principal ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CPairingWindowRequest window;
    window.deviceKind = request.GetString( 1 );
    for ( UInt32 i=0; i<request.GetCount( 2 ); ++i )
    {
        CORE::CVariant role;
        if ( request.GetAt( 2, i, role ) )
            window.roles.insert( role.AsString() );
    }
    window.seconds = request.GetUInt32( 3 );
    window.count = 0 == request.GetUInt32( 4 ) ? 1 : request.GetUInt32( 4 );
    const CORE::CString confirm = request.GetString( 5 ).Lowercase();
    window.hasRequireConfirmation = !confirm.IsNULLOrEmpty();
    window.requireConfirmation = "true" == confirm || "1" == confirm || "yes" == confirm;
    CORE::CString windowId;
    CORE::CString code;
    UInt64 expiresUtcMs = 0;
    CORE::CString error;
    if ( !OpenPairingWindow( window, principal.deviceId, windowId, code, expiresUtcMs, error ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, error ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( 1, windowId );
    response.SetString( 2, code );
    response.SetUInt64( 3, expiresUtcMs );
    CORE::CString::StringVector endpoints = GetPairingEndpoints();
    for ( UInt32 i=0; i<endpoints.size(); ++i )
        response.Add( 4, CORE::CVariant( endpoints[ i ] ) );
    code.Clear();
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::OnDevicesClosePairingWindow( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason = call.GetRequest().GetString( 1 );
    ClosePairingWindow( reason.IsNULLOrEmpty() ? CORE::CString( "closed by an admin device" ) : reason );
    call.Reply( call.NewResponse() );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult CRemoteServiceHost::OnDevicesRevoke( COMCORE::CRpcServerCall& call ) { return RunDeviceCommand( call, 1, DEVICECOMMAND_REVOKE ); }
COMCORE::TRpcHandlerResult CRemoteServiceHost::OnDevicesSetRoles( COMCORE::CRpcServerCall& call ) { return RunDeviceCommand( call, 1, DEVICECOMMAND_SET_ROLES ); }
COMCORE::TRpcHandlerResult CRemoteServiceHost::OnDevicesRename( COMCORE::CRpcServerCall& call ) { return RunDeviceCommand( call, 1, DEVICECOMMAND_RENAME ); }
COMCORE::TRpcHandlerResult CRemoteServiceHost::OnDevicesConfirmPairing( COMCORE::CRpcServerCall& call ) { return RunDeviceCommand( call, 1, DEVICECOMMAND_CONFIRM ); }
COMCORE::TRpcHandlerResult CRemoteServiceHost::OnDevicesCloseSession( COMCORE::CRpcServerCall& call ) { return RunDeviceCommand( call, 1, DEVICECOMMAND_CLOSE_SESSION ); }

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CRemoteServiceHost::RunDeviceCommand( COMCORE::CRpcServerCall& call, UInt32 commandIdField, Int32 command )
{GUCEF_TRACE;

    CRemoteServicePrincipal principal;
    if ( !GetCallPrincipal( call, principal ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_PERMISSION_DENIED, CRemoteServiceDetailCodes::DeviceUnknown, "the device is not paired or not active" );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CORE::CString commandId;
    CORE::CString reason;
    if ( !CCommandOutcomeCache::GetCommandId( call, commandIdField, commandId, reason ) )
    {
        FailWithDetail( call, COMCORE::RPC_STATUS_INVALID_ARGUMENT, CRemoteServiceDetailCodes::BadCommandId, reason );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const CORE::CString requestHash = m_commands.HashRequest( call.GetRequest(), commandIdField );
    CCommandOutcomeCache::SOutcome kept;
    CCommandOutcomeCache::ELookup lookup = m_commands.Find( principal.deviceId, commandId, requestHash, NowMs(), kept );
    if ( CCommandOutcomeCache::COMMAND_REPLAY == lookup )
    {
        CCommandOutcomeCache::AnswerReplay( call, kept, 2 );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( CCommandOutcomeCache::COMMAND_REUSED == lookup )
    {
        CCommandOutcomeCache::RefuseReused( call, commandId );
        return COMCORE::RPC_HANDLER_DONE;
    }

    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString by = principal.deviceId;
    CORE::CString error;
    bool done = false;
    switch ( command )
    {
        case DEVICECOMMAND_REVOKE:
        {
            done = RevokeDevice( request.GetString( 2 ), request.GetString( 3 ), by, error );
            break;
        }
        case DEVICECOMMAND_SET_ROLES:
        {
            CORE::CString::StringSet roles;
            for ( UInt32 i=0; i<request.GetCount( 4 ); ++i )
            {
                CORE::CVariant role;
                if ( request.GetAt( 4, i, role ) )
                    roles.insert( role.AsString() );
            }
            done = SetDeviceRoles( request.GetString( 2 ), roles, by, error );
            break;
        }
        case DEVICECOMMAND_RENAME:
        {
            done = RenameDevice( request.GetString( 2 ), request.GetString( 5 ), by, error );
            break;
        }
        case DEVICECOMMAND_CONFIRM:
        {
            done = ConfirmPairing( request.GetString( 2 ), request.GetString( 6 ), request.GetBool( 7 ), by, error );
            break;
        }
        case DEVICECOMMAND_CLOSE_SESSION:
        {
            // CloseSessionRequest: 1 commandId, 2 sessionId
            done = CloseSession( request.GetString( 2 ), by, error );
            break;
        }
        default:
        {
            error = "unknown command";
            break;
        }
    }
    if ( !done )
    {
        // a refusal changed nothing: not remembered
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_FAILED_PRECONDITION, error ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetBool( 1, true );
    response.SetBool( 2, false );
    CCommandOutcomeCache::SOutcome outcome;
    CCommandOutcomeCache::MakeOutcome( COMCORE::CRpcStatus::Ok, &response, outcome );
    m_commands.Remember( principal.deviceId, commandId, requestHash, NowMs(), outcome );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
