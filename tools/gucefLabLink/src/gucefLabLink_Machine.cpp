/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
  #include <pwd.h>
  #include <sys/utsname.h>
#endif

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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#include "gucefCOMCORE_CDiscoveryManager.h"
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H
#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#include "gucefCOMCORE_CIDiscoveryClient.h"
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefLabLink_Cluster.h"
#include "gucefLabLink_Machine.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 LinkTimeoutInMs = 15000;
static const UInt32 CallTimeoutInMs = 90000;
static const UInt32 JoinTimeoutInMs = 45 * 60 * 1000;
static const char* ProfileMarker = "# gucef-lab";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt64
NowInMs( void )
{GUCEF_TRACE;

    const UInt64 resolution = MT::PrecisionTimerResolution();
    return 0 == resolution ? 0 : MT::PrecisionTickCount() * 1000 / resolution;
}

/*-------------------------------------------------------------------------*/

/** One pulse of the application, and of the node when it has a pulse generator of its own */
static void
PumpOnce( DRN::CDRNNode* node )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr global = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    global->ForceDirectPulse();
    if ( GUCEF_NULL != node )
    {
        CORE::PulseGeneratorPtr own = node->GetPulseGenerator();
        if ( !own.IsNULL() && own.GetPointerAlways() != global.GetPointerAlways() )
            own->ForceDirectPulse();
    }
    MT::ThreadDelay( 1 );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MachineName( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    char name[ 256 ];
    if ( 0 == ::gethostname( name, sizeof( name ) - 1 ) )
    {
        name[ sizeof( name ) - 1 ] = '\0';
        return CORE::CString( name ).Lowercase();
    }
    return "machine";
    #else
    return CORE::GetEnv( "COMPUTERNAME" ).Lowercase();
    #endif
}

/*-------------------------------------------------------------------------*/

static CORE::CString
OsName( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    return "linux";
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return "windows";
    #else
    return "other";
    #endif
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ArchName( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct utsname info;
    if ( 0 == ::uname( &info ) )
        return info.machine;
    #endif
    return "unknown";
}

/*-------------------------------------------------------------------------*/

/** A link to one endpoint of the cluster, with an RPC client for its contract; closed by its destructor */
class CRpcLink
{
    public:

    CRpcLink( void ) : m_node( GUCEF_NULL ), m_session( GUCEF_NULL ), m_registry(), m_endpoint() {}

    ~CRpcLink() { Close(); }

    bool Open( const CORE::CString& address, UInt16 port, const CORE::CString& pinnedKey, const CORE::CString& sessionName,
               const DRN::CDRNAuthSettings& credentials, const COMCORE::CRpcContractPtr& contract, CORE::CString& problem )
    {
        DRN::CDRNNodeSettings nodeSettings;
        nodeSettings.serviceName = "gucef-lab";
        nodeSettings.securityPolicy.requireEncryption = true;
        nodeSettings.auth = credentials;
        m_node = GUCEF_NEW DRN::CDRNNode();
        if ( !m_node->SetSettings( nodeSettings ) || !m_node->Start() )
        {
            problem = "the DRN node cannot start (is codecspluginMONOCYPHER there?)";
            return false;
        }

        DRN::CDRNEndpoint endpoint( "tcp://" + address + ":" + CORE::ToString( port ) );
        if ( !pinnedKey.IsNULLOrEmpty() )
            endpoint.GetParams().Set( "key", CORE::CVariant( pinnedKey ) );
        DRN::CDRNSessionSettings joinSettings;
        joinSettings.sessionName = sessionName;
        m_session = m_node->CreateSession();
        if ( GUCEF_NULL == m_session || !m_session->Join( DRN::CDRNEndpointList( endpoint ), joinSettings, &credentials ) )
        {
            problem = "cannot start to link to " + endpoint.ToString();
            return false;
        }
        const UInt64 deadline = NowInMs() + LinkTimeoutInMs;
        while ( !IsActive() && NowInMs() < deadline )
            PumpOnce( m_node );
        if ( !IsActive() )
        {
            problem = "the cluster at " + address + ":" + CORE::ToString( port ) + " did not take the link: not reachable, or it refused the token or the address";
            return false;
        }

        UInt32 hostPeerId = 0;
        {
            MT::CScopeMutex lock( m_node->GetLock() );
            hostPeerId = m_session->GetSequencerPeerId();
        }
        CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex > drnTransport( GUCEF_NEW DRN::CDRNRpcTransport( *m_session, hostPeerId ) );
        COMCORE::CIRpcTransportPtr transport;
        transport = drnTransport;
        m_registry.Register( contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        COMCORE::CRpcEndpointSettings settings;
        settings.nodeName = MachineName();
        settings.contractRegistry = &m_registry;
        settings.requireSecureTransport = true;
        m_endpoint = COMCORE::CRpcEndpoint::Create( m_node->GetPulseGenerator(), settings );
        m_endpoint->AddWantedContract( contract->GetName(), 1 );
        if ( !transport->Connect( CORE::CUri() ) || !m_endpoint->Start( transport, true ) )
        {
            problem = "the RPC session with the cluster cannot start";
            return false;
        }
        while ( !m_endpoint->IsContractBound( contract->GetName(), 1 ) && NowInMs() < deadline + LinkTimeoutInMs )
            PumpOnce( m_node );
        if ( !m_endpoint->IsContractBound( contract->GetName(), 1 ) )
        {
            problem = "the cluster does not offer " + contract->GetName();
            return false;
        }
        return true;
    }

    COMCORE::CRpcMessage NewRequest( const char* path )
    {
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        m_endpoint->ResolveMethod( path, ref, status );
        return m_endpoint->NewRequest( ref );
    }

    bool Call( const char* path, const COMCORE::CRpcMessage& request, COMCORE::CRpcMessage& response, CORE::CString& problem )
    {
        CORE::CFutureResult future = m_endpoint->CallAsync( path, request );
        const UInt64 deadline = NowInMs() + CallTimeoutInMs;
        while ( !future.IsReady() && NowInMs() < deadline )
            PumpOnce( m_node );
        COMCORE::CRpcCallResult result;
        if ( !future.IsReady() || !COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) )
        {
            problem = CORE::CString( path ) + " was not answered";
            return false;
        }
        if ( !result.status.IsOk() )
        {
            problem = CORE::CString( path ) + ": " + result.status.ToString();
            return false;
        }
        response = result.response;
        return true;
    }

    void Close( void )
    {
        if ( !m_endpoint.IsNULL() )
        {
            m_endpoint->Close( false );
            for ( int i=0; i<20; ++i )
                PumpOnce( m_node );
            m_endpoint.Unlink();
        }
        if ( GUCEF_NULL != m_node )
        {
            if ( GUCEF_NULL != m_session )
            {
                m_session->Leave();
                for ( int i=0; i<20; ++i )
                    PumpOnce( m_node );
                m_node->DestroySession( m_session );
                m_session = GUCEF_NULL;
            }
            m_node->Stop();
            GUCEF_DELETE m_node;
            m_node = GUCEF_NULL;
        }
    }

    private:

    bool IsActive( void )
    {
        MT::CScopeMutex lock( m_node->GetLock() );
        return m_session->IsActive();
    }

    DRN::CDRNNode* m_node;
    DRN::CDRNSession* m_session;
    COMCORE::CRpcContractRegistry m_registry;
    COMCORE::CRpcEndpointPtr m_endpoint;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMachineRole::CMachineRole( const SSettings& settings, CStore& store )
    : CORE::CObserver()
    , m_settings( settings )
    , m_store( store )
    , m_identity()
    , m_secret()
    , m_sighting()
    , m_haveSighting( false )
    , m_nextVisitInMs( 0 )
    , m_lastAnswers()
    , m_lastOutcome()
    , m_discoveryClients()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMachineRole::~CMachineRole()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMachineRole::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEFLabLink::CMachineRole";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const SClusterAnswers& CMachineRole::GetLastAnswers( void ) const { return m_lastAnswers; }
const CORE::CString& CMachineRole::GetLastOutcome( void ) const { return m_lastOutcome; }

/*-------------------------------------------------------------------------*/

bool
CMachineRole::Start( CORE::CString& problem )
{GUCEF_TRACE;

    if ( !m_store.LoadIdentity( m_identity, problem ) )
        return false;
    CORE::CString noSecret;
    if ( !m_store.ReadSecret( m_secret, noSecret ) )
        m_secret.Clear();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Machine " << MachineName() << " (" << m_identity.machineId << ") looks for its cluster in " <<
                      m_settings.allowedRanges.Describe() << ( m_secret.IsNULLOrEmpty() ? CORE::CString( ", without the lab secret" ) : CORE::CString( ", with the lab secret" ) ) );

    COMCORE::CDiscoveryManager& manager = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager();
    if ( !AddDiscoveryClients( m_settings, "gucef-lablink", m_discoveryClients ) )
    {
        problem = "no discovery is available";
        return false;
    }
    SubscribeTo( &manager );
    manager.Browse( ServiceType );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMachineRole::Stop( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    COMCORE::CDiscoveryManager& manager = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager();
    if ( !m_discoveryClients.empty() )
    {
        manager.StopBrowse( ServiceType );
        for ( size_t i=0; i<m_discoveryClients.size(); ++i )
            manager.UnregisterDiscoveryClient( m_discoveryClients[ i ] );
        m_discoveryClients.clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CMachineRole::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const bool found = COMCORE::CIDiscoveryClient::EntryDiscoveredEvent == eventId || COMCORE::CIDiscoveryClient::EntryUpdatedEvent == eventId;
    if ( !found || GUCEF_NULL == eventData )
        return;
    COMCORE::CIDiscoveryEntryPtr entry = *static_cast< COMCORE::CDiscoveryEntryEventData* >( eventData );
    if ( entry.IsNULL() || entry->GetServiceType() != ServiceType )
        return;

    COMCORE::CHostAddress host;
    entry->GetHost( host );
    SClusterSighting sighting;
    sighting.address = host.GetFirstAddressAsString();
    const CORE::CValueList& attributes = entry->GetAttributes();
    CORE::CValueList copy = attributes;
    CORE::CVariant value;
    if ( copy.TryGetValue( AttributeCluster, value ) ) sighting.name = value.AsString();
    if ( copy.TryGetValue( AttributeKey, value ) ) sighting.key = value.AsString();
    if ( copy.TryGetValue( AttributeLabPort, value ) ) sighting.labPort = (UInt16) CORE::StringToUInt32( value.AsString() );
    if ( copy.TryGetValue( AttributeAccessPort, value ) ) sighting.accessPort = (UInt16) CORE::StringToUInt32( value.AsString() );
    if ( sighting.address.IsNULLOrEmpty() || sighting.key.IsNULLOrEmpty() || 0 == sighting.labPort )
        return;
    if ( !m_settings.allowedRanges.Allows( sighting.address ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Cluster " << sighting.name << " at " << sighting.address << " is outside the allowed ranges" );
        return;
    }

    // a cluster that is seen anew, or at another address or with another key, is visited at once
    const bool changed = !m_haveSighting || m_sighting.address != sighting.address || m_sighting.key != sighting.key;
    m_sighting = sighting;
    m_haveSighting = true;
    if ( changed )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Found cluster " << sighting.name << " at " << sighting.address );
        m_nextVisitInMs = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CMachineRole::Update( void )
{GUCEF_TRACE;

    if ( !m_haveSighting || NowInMs() < m_nextVisitInMs )
        return;
    const bool linked = Visit( m_sighting );
    m_nextVisitInMs = NowInMs() + 1000 * (UInt64) ( linked ? m_settings.refreshIntervalInSecs : m_settings.retryIntervalInSecs );
}

/*-------------------------------------------------------------------------*/

bool
CMachineRole::AskForAccess( const SClusterSighting& cluster, CORE::CString& token )
{GUCEF_TRACE;

    CORE::CString error;
    COMCORE::CRpcContractPtr contract = BuildAccessContract( error );
    DRN::CDRNAuthSettings anonymous;
    anonymous.authenticatorType = "anonymous";
    CRpcLink link;
    CORE::CString problem;
    if ( contract.IsNULL() || !link.Open( cluster.address, cluster.accessPort, CORE::CString::Empty, AccessSessionName, anonymous, contract, problem ) )
    {
        m_lastOutcome = "cannot ask " + cluster.name + " for access: " + problem;
        return false;
    }
    COMCORE::CRpcMessage request = link.NewRequest( RequestAccessPath );
    request.SetString( 1, m_identity.machineId );
    request.SetString( 2, MachineName() );
    request.SetString( 3, OsName() );
    COMCORE::CRpcMessage response;
    if ( !link.Call( RequestAccessPath, request, response, problem ) )
    {
        m_lastOutcome = "asking " + cluster.name + " for access failed: " + problem;
        return false;
    }

    const CORE::CString state = response.GetString( 1 );
    if ( state == AccessApproved )
    {
        // the approval holds for the cluster whose lab endpoint has the key it names: the one that was found
        if ( response.GetString( 3 ) != cluster.key || response.GetString( 2 ).IsNULLOrEmpty() )
        {
            m_lastOutcome = "the approval of " + cluster.name + " is for another key than the cluster that was found: ignored";
            return false;
        }
        m_identity.token = response.GetString( 2 );
        m_identity.clusterKey = cluster.key;
        m_identity.cluster = cluster.name;
        if ( !m_store.SaveIdentity( m_identity, problem ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Approved by cluster " << cluster.name );
        token = m_identity.token;
        return true;
    }
    m_lastOutcome = state == AccessRefused ? "cluster " + cluster.name + " refused access"
                                           : "waiting for approval by cluster " + cluster.name + ": on it run gucefLabLink --approve " + MachineName();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMachineRole::GetToken( const SClusterSighting& cluster, CORE::CString& token )
{GUCEF_TRACE;

    if ( !m_secret.IsNULLOrEmpty() )
    {
        token = DRN::CDRNTokenAuthenticator::IssueToken( DeriveTokenKey( m_secret ), LabSessionName, m_identity.machineId, RoleLab,
                                                         DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + LabTokenLifetimeInSecs );
        return !token.IsNULLOrEmpty();
    }
    if ( !m_identity.token.IsNULLOrEmpty() )
    {
        if ( m_identity.clusterKey == cluster.key )
        {
            token = m_identity.token;
            return true;
        }
        m_lastOutcome = "cluster " + cluster.name + " at " + cluster.address + " has another key than the cluster that approved this machine: not used";
        return false;
    }
    if ( 0 == cluster.accessPort )
    {
        m_lastOutcome = "cluster " + cluster.name + " takes no access requests and this machine has no lab secret";
        return false;
    }
    return AskForAccess( cluster, token );
}

/*-------------------------------------------------------------------------*/

bool
CMachineRole::TalkToLab( const SClusterSighting& cluster, const CORE::CString& token, SClusterAnswers& answers )
{GUCEF_TRACE;

    CORE::CString error;
    COMCORE::CRpcContractPtr contract = BuildLabContract( error );
    DRN::CDRNAuthSettings credentials;
    credentials.authenticatorType = "token";
    credentials.SetSetting( "token", CORE::CVariant( token ) );
    CRpcLink link;
    CORE::CString problem;
    // the link only comes up with the cluster that has the key of the discovery entry
    if ( contract.IsNULL() || !link.Open( cluster.address, cluster.labPort, cluster.key, LabSessionName, credentials, contract, problem ) )
    {
        m_lastOutcome = problem;
        return false;
    }

    COMCORE::CRpcMessage response;
    if ( !link.Call( HelloPath, link.NewRequest( HelloPath ), response, problem ) )
    {
        m_lastOutcome = problem;
        return false;
    }
    // with the lab secret the cluster proves it has it too, over the key of this link
    if ( !m_secret.IsNULLOrEmpty() && !EqualInConstantTime( response.GetString( 3 ), MakeClusterProof( m_secret, cluster.key ) ) )
    {
        m_lastOutcome = "the cluster at " + cluster.address + " does not prove it has the lab secret: not used";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: " << m_lastOutcome );
        return false;
    }
    answers.cluster = response.GetString( 1 );
    answers.k3sVersion = response.GetString( 2 );

    if ( !link.Call( ClientConfigPath, link.NewRequest( ClientConfigPath ), response, problem ) )
    {
        m_lastOutcome = problem;
        return false;
    }
    answers.caCertificate = response.GetString( 1 );
    answers.mcpToken = response.GetString( 2 );
    for ( UInt32 i=0; i<response.GetCount( 3 ); ++i )
    {
        CORE::CVariant node;
        if ( response.GetAt( 3, i, node ) )
            answers.nodes.push_back( node.AsString() );
    }

    const bool isNode = m_settings.nodeMarkerFile.IsNULLOrEmpty() || CORE::FileExists( m_settings.nodeMarkerFile );
    if ( m_settings.join && !isNode && !m_settings.joinScript.IsNULLOrEmpty() )
    {
        COMCORE::CRpcMessage request = link.NewRequest( JoinPath );
        request.SetString( 1, MachineName() );
        request.SetString( 2, OsName() );
        request.SetString( 3, ArchName() );
        request.SetBool( 4, m_settings.ephemeral );
        if ( link.Call( JoinPath, request, response, problem ) )
        {
            answers.joinGiven = true;
            answers.joinServer = response.GetString( 1 );
            answers.joinVersion = response.GetString( 2 );
            answers.joinToken = response.GetString( 3 );
            answers.joinEphemeral = response.GetBool( 4 );
        }
        else
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Joining is not possible now: " << problem );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMachineRole::RunJoin( const SClusterAnswers& answers )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "--server" ); args.push_back( answers.joinServer );
    args.push_back( "--version" ); args.push_back( answers.joinVersion );
    args.push_back( "--token" ); args.push_back( answers.joinToken );
    if ( answers.joinEphemeral )
        args.push_back( "--ephemeral" );
    // join.sh installs the MCP service as the user that ran sudo
    CORE::CProcess::TStringMap environment;
    if ( !m_settings.claudeUser.IsNULLOrEmpty() )
        environment[ "SUDO_USER" ] = m_settings.claudeUser;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Joining cluster " << answers.cluster << " (k3s " << answers.joinVersion << ")" <<
                      ( answers.joinEphemeral ? " as an ephemeral node" : "" ) );
    std::string output;
    Int32 exitCode = -1;
    if ( !RunProgram( m_settings.joinScript, args, environment, JoinTimeoutInMs, output, exitCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: " << m_settings.joinScript << " cannot be run" );
        return;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << m_settings.joinScript << " ended with " << exitCode << ":\n" << output.c_str() );
    if ( 10 == exitCode )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: The machine needs a reboot (NVIDIA driver); joining continues after it" );
}

/*-------------------------------------------------------------------------*/

bool
CMachineRole::RunClaude( const CORE::CProcess::TStringVector& args, std::string& output )
{GUCEF_TRACE;

    Int32 exitCode = -1;
    CORE::CProcess::TStringVector all;
    CORE::CString exe = m_settings.claudeCommand;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    const bool asOtherUser = 0 == ::geteuid() && m_settings.claudeUser != "root";
    if ( exe.IsNULLOrEmpty() )
    {
        // where the claude of the user is: in its login shell
        CORE::CProcess::TStringVector lookup;
        if ( asOtherUser ) { lookup.push_back( "-u" ); lookup.push_back( m_settings.claudeUser ); lookup.push_back( "-H" ); lookup.push_back( "bash" ); }
        lookup.push_back( "-lc" ); lookup.push_back( "command -v claude" );
        std::string found;
        if ( !RunProgram( asOtherUser ? "sudo" : "bash", lookup, CORE::CProcess::TStringMap(), 30000, found, exitCode ) || 0 != exitCode )
            return false;
        exe = StripText( CORE::CString( found.c_str() ) );
        CORE::CString::StringVector lines = exe.ParseElements( '\n', false );
        if ( lines.empty() )
            return false;
        exe = StripText( lines.back() );
    }
    if ( asOtherUser )
    {
        all.push_back( "-u" ); all.push_back( m_settings.claudeUser ); all.push_back( "-H" ); all.push_back( exe );
        exe = "sudo";
    }
    #endif
    if ( exe.IsNULLOrEmpty() )
        return false;
    for ( size_t i=0; i<args.size(); ++i )
        all.push_back( args[ i ] );
    return RunProgram( exe, all, CORE::CProcess::TStringMap(), 60000, output, exitCode ) && 0 == exitCode;
}

/*-------------------------------------------------------------------------*/

void
CMachineRole::ConfigureClaude( const SClusterAnswers& answers )
{GUCEF_TRACE;

    if ( m_settings.claudeUser.IsNULLOrEmpty() )
        return;

    CORE::CString home = m_settings.claudeHome;
    Int32 uid = -1, gid = -1;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct passwd* user = ::getpwnam( m_settings.claudeUser.C_String() );
    if ( GUCEF_NULL != user )
    {
        if ( home.IsNULLOrEmpty() )
            home = user->pw_dir;
        uid = (Int32) user->pw_uid;
        gid = (Int32) user->pw_gid;
    }
    #endif
    if ( home.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The home of " << m_settings.claudeUser << " is not known" );
        return;
    }

    // the authority of the MCP services, for Node.js and so Claude Code
    const CORE::CString caDir = CORE::CombinePath( home, ".config/gucef-lab" );
    const CORE::CString caFile = CORE::CombinePath( caDir, "ca.crt" );
    if ( !answers.caCertificate.IsNULLOrEmpty() )
    {
        CORE::CreateDirs( caDir );
        CORE::WriteFileAtomically( caFile, answers.caCertificate.C_String(), answers.caCertificate.Length() );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        if ( uid >= 0 && 0 == ::geteuid() )
        {
            if ( 0 != ::chown( caDir.C_String(), (uid_t) uid, (gid_t) gid ) || 0 != ::chown( caFile.C_String(), (uid_t) uid, (gid_t) gid ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: cannot give " << caFile << " to " << m_settings.claudeUser );
        }
        #endif

        // NODE_EXTRA_CA_CERTS in the profile, read when the user logs in
        const CORE::CString profile = CORE::CombinePath( home, ".profile" );
        const CORE::CString line = "export NODE_EXTRA_CA_CERTS=\"" + caFile + "\"  " + ProfileMarker;
        CORE::CDynamicBuffer content;
        CORE::CString text;
        if ( CORE::FileExists( profile ) && content.LoadContentFromFile( profile ) )
            text = ( 0 == content.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
        if ( text.HasSubstr( line ) < 0 )
        {
            CORE::CString::StringVector lines = text.ParseElements( '\n', true );
            CORE::CString updated;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                if ( lines[ i ].HasSubstr( ProfileMarker ) >= 0 )
                    continue;
                if ( i + 1 < lines.size() || !lines[ i ].IsNULLOrEmpty() )
                    updated += lines[ i ] + "\n";
            }
            updated += line + "\n";
            const bool existed = CORE::FileExists( profile );
            CORE::WriteFileAtomically( profile, updated.C_String(), updated.Length() );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            if ( !existed && uid >= 0 && 0 == ::geteuid() && 0 != ::chown( profile.C_String(), (uid_t) uid, (gid_t) gid ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: cannot give " << profile << " to " << m_settings.claudeUser );
            #endif
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << profile << " sets NODE_EXTRA_CA_CERTS: new logins of " << m_settings.claudeUser << " trust the MCP services" );
        }
    }

    if ( answers.mcpToken.IsNULLOrEmpty() )
        return;

    // the registrations wanted: every node by its HTTPS endpoint, this machine's own service locally
    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;
    TStringMap wanted;
    for ( size_t i=0; i<answers.nodes.size(); ++i )
    {
        CORE::CString::StringVector parts = answers.nodes[ i ].ParseElements( ' ', false );
        if ( parts.size() < 2 )
            continue;
        if ( parts[ 0 ] == m_settings.localMcpNode )
            wanted[ "local-llm" ] = "http://127.0.0.1:8790/mcp";
        else
            wanted[ parts[ 0 ] + "-llm" ] = parts[ 1 ];
    }

    // what was registered: "name url token-hash" lines of the state directory
    UInt8 digest[ 32 ];
    DRN::CDRNCrypto::Sha256( answers.mcpToken.C_String(), answers.mcpToken.Length(), digest );
    const CORE::CString tokenHash = DRN::CDRNCrypto::ToHex( digest, 8 );
    const CORE::CString recordFile = m_store.GetPath( "claude-registrations" );
    TStringMap registered;
    CORE::CDynamicBuffer record;
    if ( CORE::FileExists( recordFile ) && record.LoadContentFromFile( recordFile ) )
    {
        CORE::CString::StringVector lines = ( 0 == record.GetDataSize() ? CORE::CString::Empty : CORE::CString( static_cast< const char* >( record.GetConstBufferPtr() ), record.GetDataSize() ) ).ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            CORE::CString::StringVector parts = lines[ i ].ParseElements( ' ', false );
            if ( parts.size() == 3 )
                registered[ parts[ 0 ] ] = parts[ 1 ] + " " + parts[ 2 ];
        }
    }

    CORE::CString newRecord;
    std::string output;
    for ( TStringMap::iterator i = wanted.begin(); i != wanted.end(); ++i )
    {
        const CORE::CString state = (*i).second + " " + tokenHash;
        if ( registered[ (*i).first ] != state )
        {
            CORE::CProcess::TStringVector remove;
            remove.push_back( "mcp" ); remove.push_back( "remove" ); remove.push_back( (*i).first ); remove.push_back( "--scope" ); remove.push_back( "user" );
            RunClaude( remove, output );
            CORE::CProcess::TStringVector add;
            add.push_back( "mcp" ); add.push_back( "add" ); add.push_back( "--transport" ); add.push_back( "http" ); add.push_back( (*i).first );
            add.push_back( (*i).second ); add.push_back( "--scope" ); add.push_back( "user" );
            add.push_back( "--header" ); add.push_back( "Authorization: Bearer " + answers.mcpToken );
            if ( !RunClaude( add, output ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Registering " << (*i).first << " with Claude Code failed: " << output.c_str() );
                continue;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Registered " << (*i).first << " (" << (*i).second << ") with Claude Code of " << m_settings.claudeUser );
        }
        newRecord += (*i).first + " " + state + "\n";
    }
    // nodes that are gone
    for ( TStringMap::iterator i = registered.begin(); i != registered.end(); ++i )
    {
        if ( wanted.find( (*i).first ) != wanted.end() || (*i).second.IsNULLOrEmpty() )
            continue;
        CORE::CProcess::TStringVector remove;
        remove.push_back( "mcp" ); remove.push_back( "remove" ); remove.push_back( (*i).first ); remove.push_back( "--scope" ); remove.push_back( "user" );
        RunClaude( remove, output );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Removed " << (*i).first << " from Claude Code of " << m_settings.claudeUser );
    }
    CORE::CString problem;
    if ( !CORE::WriteFileAtomically( recordFile, newRecord.C_String(), newRecord.Length() ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: cannot write " << recordFile );
}

/*-------------------------------------------------------------------------*/

bool
CMachineRole::Visit( const SClusterSighting& cluster )
{GUCEF_TRACE;

    m_lastOutcome.Clear();
    if ( !m_settings.allowedRanges.Allows( cluster.address ) )
    {
        m_lastOutcome = "cluster " + cluster.name + " at " + cluster.address + " is outside the allowed ranges";
        return false;
    }
    if ( m_identity.machineId.IsNULLOrEmpty() )
    {
        CORE::CString problem;
        if ( !m_store.LoadIdentity( m_identity, problem ) )
        {
            m_lastOutcome = problem;
            return false;
        }
        if ( !m_store.ReadSecret( m_secret, problem ) )
            m_secret.Clear();
    }

    CORE::CString token;
    SClusterAnswers answers;
    if ( !GetToken( cluster, token ) || !TalkToLab( cluster, token, answers ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << m_lastOutcome );
        return false;
    }
    m_lastAnswers = answers;
    ConfigureClaude( answers );
    if ( answers.joinGiven )
        RunJoin( answers );
    m_lastOutcome = "linked to cluster " + answers.cluster;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Linked to cluster " << answers.cluster << " (k3s " << answers.k3sVersion << ", " <<
               (UInt32) answers.nodes.size() << " MCP services)" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
