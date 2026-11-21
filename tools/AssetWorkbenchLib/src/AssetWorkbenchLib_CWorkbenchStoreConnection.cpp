/*
 *  AssetWorkbenchLib: the connection of the workbench to a remote asset store service: finding, pairing, connecting and staying connected
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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H
#include "AssetWorkbenchLib_CStoreConnectionTaskConsumer.h"
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H
#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKCONSUMER_H ? */

#include "AssetWorkbenchLib_CWorkbenchStoreConnection.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchStoreConnection::StateChangedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::StateChangedEvent";
const CORE::CEvent CWorkbenchStoreConnection::ServicesChangedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::ServicesChangedEvent";
const CORE::CEvent CWorkbenchStoreConnection::ServiceReadyEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::ServiceReadyEvent";
const CORE::CEvent CWorkbenchStoreConnection::ServiceLostEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::ServiceLostEvent";
const CORE::CEvent CWorkbenchStoreConnection::PairedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection::PairedEvent";
const CORE::CString CWorkbenchStoreConnection::DefaultServiceType = "_gucef-assetstore._tcp";
const CORE::CString CWorkbenchStoreConnection::DefaultServiceName = "gucef.assetstore";
const CORE::CString CWorkbenchStoreConnection::DefaultSessionName = "assetstore";
const CORE::CString CWorkbenchStoreConnection::DefaultSlot = "assetstore";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const ThreadPoolName = "AssetWorkbenchStoreConnection";

/** The task consumer factory is registered while any connection exists */
UInt32 g_factoryUsers = 0;
TStoreConnectionTaskConsumerFactory* g_factory = GUCEF_NULL;

void
AcquireFactory( void )
{GUCEF_TRACE;

    if ( 0 == g_factoryUsers++ )
    {
        g_factory = GUCEF_NEW TStoreConnectionTaskConsumerFactory();
        CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CStoreConnectionTaskConsumer::TaskType, g_factory );
    }
}

void
ReleaseFactory( void )
{GUCEF_TRACE;

    if ( 0 == --g_factoryUsers )
    {
        CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CStoreConnectionTaskConsumer::TaskType );
        GUCEF_DELETE g_factory;
        g_factory = GUCEF_NULL;
    }
}

/** The data of the task of a future that ended */
const CStoreConnectionTaskData*
GetOutcome( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    CORE::CTaskPtr task = future.GetResult( 0 );
    return task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CStoreConnectionTaskData >( task->GetTaskData() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchStoreConnection::SDiscoveredService::SDiscoveredService( void )
    : instanceId()
    , displayName()
    , endpoint()
    , host()
    , pairingPort( 0 )
    , keyFingerprint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchStoreConnection::SSettings::SSettings( void )
    : endpoint()
    , pairingPort( 47401 )
    , lanDiscovery( true )
    , autoRetry( true )
    , serviceName( DefaultServiceName )
    , sessionName( DefaultSessionName )
    , credentialsSlot( DefaultSlot )
    , clientKind( "viewer" )
    , deviceName()
    , connectTimeoutMs( 12000 )
    , retryBaseMs( 500 )
    , retryMaxMs( 30000 )
    , finder()
    , serviceSettings()
{GUCEF_TRACE;

    finder.serviceType = DefaultServiceType;
    finder.findMinCandidates = 0;
    finder.findTimeoutMs = 3000;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::RegisterEvents( void )
{GUCEF_TRACE;

    StateChangedEvent.Initialize();
    ServicesChangedEvent.Initialize();
    ServiceReadyEvent.Initialize();
    ServiceLostEvent.Initialize();
    PairedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchStoreConnection::CWorkbenchStoreConnection( void )
    : CORE::CObservingNotifier()
    , m_settings()
    , m_store( GUCEF_NULL )
    , m_pulse()
    , m_timer( GUCEF_NULL )
    , m_state( STATE_DISCONNECTED )
    , m_error()
    , m_serverName()
    , m_serverVersion()
    , m_info()
    , m_service()
    , m_connecting()
    , m_activeEndpoint()
    , m_pairingEndpoint()
    , m_discovered()
    , m_findFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_finding( false )
    , m_taskFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_taskRunning( false )
    , m_taskKind( CStoreConnectionTaskData::KIND_PAIR )
    , m_connectAfterPair( false )
    , m_discardTask( false )
    , m_wanted( false )
    , m_retrying( false )
    , m_retryAtInMs( 0 )
    , m_retryCount( 0 )
    , m_backoff()
    , m_paired( false )
    , m_wasConnected( false )
    , m_initialized( false )
    , m_inShutdown( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchStoreConnection::~CWorkbenchStoreConnection()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::Initialize( AUTH::ICredentialStore* store, const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    m_store = store;
    m_pulse = pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator;
    AcquireFactory();
    m_backoff.SetPolicy( 250, m_settings.retryBaseMs, m_settings.retryMaxMs, 60000, 1000 );
    m_timer = GUCEF_NEW CORE::CTimer( m_pulse, 100 );
    TEventCallback callback( this, &CWorkbenchStoreConnection::OnTimer );
    SubscribeTo( m_timer, CORE::CTimer::TimerUpdateEvent, callback );
    m_timer->SetEnabled( true );
    m_initialized = true;
    RefreshPaired();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::Shutdown( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    m_inShutdown = true;
    m_wanted = false;
    m_retrying = false;
    if ( GUCEF_NULL != m_timer )
        m_timer->SetEnabled( false );
    // a task that runs ends first: it holds the service it opens and the store of the credentials
    if ( m_taskRunning )
    {
        m_taskFuture.Await( (Int32) m_settings.connectTimeoutMs + 5000 );
        if ( CStoreConnectionTaskData::KIND_CONNECT == m_taskKind && !m_connecting.IsNULL() )
        {
            m_connecting->Close();
            m_connecting.Unlink();
        }
        m_taskRunning = false;
    }
    if ( m_finding )
    {
        m_findFuture.Await( (Int32) m_settings.finder.findTimeoutMs + 5000 );
        m_finding = false;
    }
    if ( !m_service.IsNULL() )
        LoseService();
    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_timer;
    m_timer = GUCEF_NULL;
    ReleaseFactory();
    m_state = STATE_DISCONNECTED;
    m_initialized = false;
    m_inShutdown = false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::SetCredentialStore( AUTH::ICredentialStore* store )
{GUCEF_TRACE;

    m_store = store;
    RefreshPaired();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::SetSettings( const SSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_settings.endpoint = NormaliseEndpoint( settings.endpoint );
    m_pairingEndpoint.Clear();
    if ( m_initialized )
        m_backoff.SetPolicy( 250, m_settings.retryBaseMs, m_settings.retryMaxMs, 60000, 1000 );
}

/*-------------------------------------------------------------------------*/

const CWorkbenchStoreConnection::SSettings&
CWorkbenchStoreConnection::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::NormaliseEndpoint( const CORE::CString& endpoint )
{GUCEF_TRACE;

    CORE::CString text = endpoint.Trim( true ).Trim( false );
    if ( 0 == text.HasSubstr( "tcp://" ) )
        text = text.SubstrToIndex( 6, false );
    while ( text.Length() > 0 && '/' == text[ text.Length() - 1 ] )
        text = text.CutChars( 1, false );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::GetHostOf( const CORE::CString& endpoint )
{GUCEF_TRACE;

    const CORE::CString text = NormaliseEndpoint( endpoint );
    const Int32 colon = text.HasChar( ':', false );
    return colon > 0 ? text.SubstrToIndex( (UInt32) colon, true ) : text;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::SetEndpoint( const CORE::CString& endpoint )
{GUCEF_TRACE;

    m_settings.endpoint = NormaliseEndpoint( endpoint );
    m_pairingEndpoint.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchStoreConnection::GetEndpoint( void ) const
{GUCEF_TRACE;

    return m_settings.endpoint;
}

/*-------------------------------------------------------------------------*/

UInt64
CWorkbenchStoreConnection::NowInMs( void )
{GUCEF_TRACE;

    const UInt64 resolution = MT::PrecisionTimerResolution();
    return 0 == resolution ? 0 : MT::PrecisionTickCount() * 1000 / resolution;
}

/*-------------------------------------------------------------------------*/

const char*
CWorkbenchStoreConnection::GetStateName( TState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case STATE_DISCOVERING:     return "discovering";
        case STATE_PAIRING:         return "pairing";
        case STATE_CONNECTING:      return "connecting";
        case STATE_CONNECTED:       return "connected";
        case STATE_OFFLINE:         return "offline";
        default:                    return "disconnected";
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::SetState( TState state )
{GUCEF_TRACE;

    if ( state == m_state )
        return;
    m_state = state;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: " << GetStateName( state ) );
    CORE::TCloneableInt32 data( (Int32) state );
    NotifyObservers( StateChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::Fail( const CORE::CString& text )
{GUCEF_TRACE;

    m_error = text;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: " << text );
    // a state that did not change still tells its observers that the error did
    if ( STATE_DISCONNECTED == m_state )
    {
        CORE::TCloneableInt32 data( (Int32) m_state );
        NotifyObservers( StateChangedEvent, &data );
    }
    else
        SetState( STATE_DISCONNECTED );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::MakeDeviceName( void ) const
{GUCEF_TRACE;

    if ( !m_settings.deviceName.IsNULLOrEmpty() )
        return m_settings.deviceName;
    const char* host = getenv( "HOSTNAME" );
    if ( GUCEF_NULL == host || '\0' == host[ 0 ] )
        host = getenv( "COMPUTERNAME" );
    CORE::CString name = "AssetWorkbench";
    if ( GUCEF_NULL != host && '\0' != host[ 0 ] )
        name += CORE::CString( " on " ) + host;
    return name.Length() > 64 ? name.SubstrToIndex( 64, true ) : name;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::RefreshPaired( void )
{GUCEF_TRACE;

    m_paired = false;
    if ( GUCEF_NULL == m_store )
        return;
    DRNUTILS::CDeviceCredentials credentials;
    CORE::CString error;
    m_paired = credentials.LoadFromStore( *m_store, m_settings.credentialsSlot, error ) && credentials.IsComplete();
    credentials.Wipe();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::IsPaired( void ) const
{GUCEF_TRACE;

    return m_paired;
}

/*-------------------------------------------------------------------------*/

CWorkbenchStoreConnection::TState
CWorkbenchStoreConnection::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::IsConnected( void ) const
{GUCEF_TRACE;

    return STATE_CONNECTED == m_state || STATE_OFFLINE == m_state;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::IsBusy( void ) const
{GUCEF_TRACE;

    return m_taskRunning || m_finding;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CWorkbenchStoreConnection::GetErrorText( void ) const { return m_error; }
const CORE::CString& CWorkbenchStoreConnection::GetServerName( void ) const { return m_serverName; }
const CORE::CString& CWorkbenchStoreConnection::GetServerVersion( void ) const { return m_serverVersion; }
const ASSETSTORE::CAssetServiceInfo& CWorkbenchStoreConnection::GetServiceInfo( void ) const { return m_info; }
const CORE::CString& CWorkbenchStoreConnection::GetActiveEndpoint( void ) const { return m_activeEndpoint; }
const CWorkbenchStoreConnection::TDiscoveredVector& CWorkbenchStoreConnection::GetDiscoveredServices( void ) const { return m_discovered; }

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreServicePtr
CWorkbenchStoreConnection::GetService( void ) const
{GUCEF_TRACE;

    return IsConnected() ? m_service : ASSETSTORE::TAssetStoreServicePtr();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::GetStoreLabel( void ) const
{GUCEF_TRACE;

    if ( !IsConnected() )
        return CORE::CString::Empty;
    return m_activeEndpoint.IsNULLOrEmpty() ? m_serverName : ( m_serverName.IsNULLOrEmpty() ? m_activeEndpoint : m_activeEndpoint + " (" + m_serverName + ")" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::GetStateText( void ) const
{GUCEF_TRACE;

    switch ( m_state )
    {
        case STATE_DISCOVERING:
            return "Looking for asset store services on the local network...";
        case STATE_PAIRING:
            return "Pairing with " + ( GetPairingEndpoint().IsNULLOrEmpty() ? CORE::CString( "the service on the local network" ) : GetHostOf( GetPairingEndpoint() ) ) + "...";
        case STATE_CONNECTING:
        {
            const CORE::CString where = m_settings.endpoint.IsNULLOrEmpty() ? CORE::CString( "the store service" ) : m_settings.endpoint;
            if ( m_retrying )
            {
                const UInt64 now = NowInMs();
                const UInt32 seconds = m_retryAtInMs > now ? (UInt32) ( ( m_retryAtInMs - now + 999 ) / 1000 ) : 0;
                return "Cannot reach " + where + ". Trying again in " + CORE::ToString( seconds ) + " s (attempt " + CORE::ToString( m_retryCount + 1 ) + ").";
            }
            return "Connecting to " + where + "...";
        }
        case STATE_CONNECTED:
            return "Connected to " + GetStoreLabel() + ". The workbench uses this store.";
        case STATE_OFFLINE:
            return "Offline: the link to " + m_activeEndpoint + " is down and is set up again by itself. The views show what was loaded last.";
        default:
            return m_paired ? CORE::CString( "Not connected. The workbench uses the store of this computer." ) :
                              CORE::CString( "Not connected and not paired. The workbench uses the store of this computer." );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::GetServerSummary( void ) const
{GUCEF_TRACE;

    if ( !IsConnected() )
        return CORE::CString::Empty;
    CORE::CString text = "Service: " + ( m_serverName.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : m_serverName ) +
                         ( m_serverVersion.IsNULLOrEmpty() ? CORE::CString::Empty : " " + m_serverVersion ) + "\n";
    CORE::CString roles;
    for ( CORE::CString::StringSet::const_iterator i = m_info.roles.begin(); i != m_info.roles.end(); ++i )
        roles += ( roles.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( ", " ) ) + *i;
    text += "This computer is: " + ( roles.IsNULLOrEmpty() ? CORE::CString( "no role" ) : roles ) + "\n";
    CORE::CString capabilities;
    for ( CORE::CString::StringSet::const_iterator i = m_info.capabilities.begin(); i != m_info.capabilities.end(); ++i )
        capabilities += ( capabilities.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( ", " ) ) + *i;
    text += "Offers: " + ( capabilities.IsNULLOrEmpty() ? CORE::CString( "nothing reported" ) : capabilities ) + "\n";
    text += "Storage locations: " + CORE::ToString( (UInt32) m_info.locations.size() ) + ", online stores: " + CORE::ToString( (UInt32) m_info.stores.size() ) +
            ", catalogue revision: " + CORE::ToString( m_info.catalogueRevision ) + ( m_info.converterName.IsNULLOrEmpty() ? CORE::CString( ", no converter" ) : ", converter: " + m_info.converterName );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::DescribeService( const SDiscoveredService& service )
{GUCEF_TRACE;

    CORE::CString text = service.displayName.IsNULLOrEmpty() ? CORE::CString( "Asset store" ) : service.displayName;
    text += "   " + service.endpoint;
    if ( !service.keyFingerprint.IsNULLOrEmpty() )
        text += "   key " + ( service.keyFingerprint.Length() > 14 ? service.keyFingerprint.SubstrToIndex( 14, true ) : service.keyFingerprint );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchStoreConnection::GetPairingEndpoint( void ) const
{GUCEF_TRACE;

    if ( !m_pairingEndpoint.IsNULLOrEmpty() )
        return m_pairingEndpoint;
    const CORE::CString host = GetHostOf( m_settings.endpoint );
    return host.IsNULLOrEmpty() ? CORE::CString::Empty : "tcp://" + host + ":" + CORE::ToString( m_settings.pairingPort );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::Discover( void )
{GUCEF_TRACE;

    if ( !m_initialized || m_finding || !m_settings.lanDiscovery )
        return false;
    COMCORE::CServiceFinderSettings finder = m_settings.finder;
    if ( finder.serviceType.IsNULLOrEmpty() )
        finder.serviceType = DefaultServiceType;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( !pool.IsNULL() )
        pool->SetDesiredMaxTotalNrOfThreads( 2 );
    m_findFuture = COMCORE::CServiceFinder::FindOnceAsync( finder, pool );
    m_finding = m_findFuture.HasAFuture();
    if ( m_finding && STATE_DISCONNECTED == m_state )
        SetState( STATE_DISCOVERING );
    return m_finding;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::CheckFind( void )
{GUCEF_TRACE;

    if ( !m_finding || !m_findFuture.IsReady() )
        return;
    COMCORE::CServiceFinder::TServiceCandidateVector candidates;
    bool found = false;
    COMCORE::CServiceFinder::GetFindOnceResult( m_findFuture, candidates, found );
    m_findFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_finding = false;
    m_discovered.clear();
    for ( size_t i=0; i<candidates.size(); ++i )
    {
        SDiscoveredService service;
        const COMCORE::CServiceAnnouncement& announcement = candidates[ i ].announcement;
        service.instanceId = announcement.instanceId;
        service.displayName = announcement.displayName;
        service.keyFingerprint = announcement.keyFingerprint;
        service.endpoint = NormaliseEndpoint( candidates[ i ].GetFirstEndpoint() );
        service.host = GetHostOf( service.endpoint );
        service.pairingPort = announcement.properties.GetValueAlways( "pairingPort" ).AsUInt32( 0 );
        if ( !service.endpoint.IsNULLOrEmpty() )
            m_discovered.push_back( service );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: " << (UInt32) m_discovered.size() << " asset store services on the local network" );
    if ( STATE_DISCOVERING == m_state )
        SetState( STATE_DISCONNECTED );
    NotifyObservers( ServicesChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::SelectService( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_discovered.size() )
        return false;
    const SDiscoveredService& service = m_discovered[ index ];
    m_settings.endpoint = service.endpoint;
    m_pairingEndpoint = service.pairingPort > 0 ? "tcp://" + service.host + ":" + CORE::ToString( service.pairingPort ) : CORE::CString::Empty;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::Pair( const AUTH::CSecretString& code, bool connectAfterwards )
{GUCEF_TRACE;

    if ( !m_initialized || m_taskRunning )
        return false;
    if ( GUCEF_NULL == m_store )
    {
        Fail( "This computer has no credential store to keep the pairing in." );
        return false;
    }
    if ( code.IsEmpty() )
    {
        Fail( "Type the pairing code that the server shows." );
        return false;
    }
    if ( IsConnected() )
        Disconnect();
    CStoreConnectionTaskData data;
    data.kind = CStoreConnectionTaskData::KIND_PAIR;
    data.pairingEndpoint = GetPairingEndpoint();
    data.serviceType = m_settings.finder.serviceType.IsNULLOrEmpty() ? DefaultServiceType : m_settings.finder.serviceType;
    data.serviceName = m_settings.serviceName;
    data.sessionName = m_settings.sessionName;
    data.deviceName = MakeDeviceName();
    data.clientKind = m_settings.clientKind;
    data.credentialsSlot = m_settings.credentialsSlot;
    data.code = code;
    data.store = m_store;
    data.timeoutInMs = m_settings.connectTimeoutMs;
    data.lanDiscovery = m_settings.lanDiscovery;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( !pool.IsNULL() )
        pool->SetDesiredMaxTotalNrOfThreads( 2 );
    m_taskFuture = pool.IsNULL() ? CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED ) : pool->QueueTask( CStoreConnectionTaskConsumer::TaskType, &data );
    data.code.Clear();
    if ( !m_taskFuture.HasAFuture() )
    {
        Fail( "The pairing could not be started." );
        return false;
    }
    m_error.Clear();
    m_taskRunning = true;
    m_taskKind = CStoreConnectionTaskData::KIND_PAIR;
    m_connectAfterPair = connectAfterwards;
    m_discardTask = false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: pairing with " << ( data.pairingEndpoint.IsNULLOrEmpty() ? CORE::CString( "a service found on the local network" ) : data.pairingEndpoint ) );
    SetState( STATE_PAIRING );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::Connect( const CORE::CString& endpoint )
{GUCEF_TRACE;

    if ( !m_initialized || m_taskRunning )
        return false;
    const CORE::CString wanted = NormaliseEndpoint( endpoint );
    if ( IsConnected() )
    {
        if ( wanted.IsNULLOrEmpty() || wanted == m_activeEndpoint )
            return true;
        Disconnect();
    }
    if ( !wanted.IsNULLOrEmpty() )
        SetEndpoint( wanted );
    RefreshPaired();
    if ( !m_paired )
    {
        m_wanted = false;
        Fail( "This computer is not paired with the store service yet. Open pairing on the server, then type the code it shows and press Pair." );
        return false;
    }
    m_wanted = true;
    m_retrying = false;
    m_retryCount = 0;
    m_backoff.Reset();
    m_error.Clear();
    return StartConnectTask();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::StartConnectTask( void )
{GUCEF_TRACE;

    ASSETSTORE::CAssetStoreServiceConfig config;
    config.implementationId = ASSETSTORE::CAssetStoreServiceConfig::RemoteImplementationId;
    config.credentialStore = m_store;
    config.pulseGenerator = m_pulse;
    config.implementationSettings.Copy( m_settings.serviceSettings );
    config.implementationSettings.SetName( "service" );
    config.implementationSettings.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( !m_settings.endpoint.IsNULLOrEmpty() )
        config.implementationSettings.SetAttribute( "endpoint", "tcp://" + m_settings.endpoint );
    config.implementationSettings.SetAttribute( "credentialsSlot", m_settings.credentialsSlot );
    config.implementationSettings.SetAttribute( "clientKind", m_settings.clientKind );
    config.implementationSettings.SetAttribute( "connectTimeoutMs", m_settings.connectTimeoutMs );
    config.implementationSettings.SetAttribute( "lanDiscovery", m_settings.lanDiscovery );
    if ( !m_settings.finder.serviceType.IsNULLOrEmpty() )
        config.implementationSettings.SetAttribute( "lanServiceType", m_settings.finder.serviceType );
    CORE::CString error;
    ASSETSTORE::TAssetStoreServicePtr service = ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( config, error );
    if ( service.IsNULL() )
    {
        m_wanted = false;
        Fail( "The remote store cannot be made in this build: " + error );
        return false;
    }
    CStoreConnectionTaskData data;
    data.kind = CStoreConnectionTaskData::KIND_CONNECT;
    data.service = service;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( !pool.IsNULL() )
        pool->SetDesiredMaxTotalNrOfThreads( 2 );
    m_taskFuture = pool.IsNULL() ? CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED ) : pool->QueueTask( CStoreConnectionTaskConsumer::TaskType, &data );
    if ( !m_taskFuture.HasAFuture() )
    {
        m_wanted = false;
        Fail( "The connection could not be started." );
        return false;
    }
    m_connecting = service;
    m_taskRunning = true;
    m_taskKind = CStoreConnectionTaskData::KIND_CONNECT;
    m_discardTask = false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: connecting to " << ( m_settings.endpoint.IsNULLOrEmpty() ? CORE::CString( "the service of the credentials" ) : m_settings.endpoint ) );
    SetState( STATE_CONNECTING );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::CheckTask( void )
{GUCEF_TRACE;

    if ( !m_taskRunning || !m_taskFuture.IsReady() )
        return;
    const CStoreConnectionTaskData* outcome = GetOutcome( m_taskFuture );
    CStoreConnectionTaskData data;
    const bool haveData = GUCEF_NULL != outcome;
    if ( haveData )
        data = *outcome;
    m_taskFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    m_taskRunning = false;
    if ( m_discardTask )
    {
        m_discardTask = false;
        if ( CStoreConnectionTaskData::KIND_CONNECT == m_taskKind && !m_connecting.IsNULL() )
        {
            m_connecting->Close();
            m_connecting.Unlink();
        }
        if ( CStoreConnectionTaskData::KIND_PAIR == m_taskKind && haveData && data.succeeded )
        {
            m_paired = true;
            NotifyObservers( PairedEvent );
        }
        return;
    }
    if ( !haveData )
    {
        m_wanted = false;
        m_connecting.Unlink();
        Fail( "The step could not run." );
        return;
    }
    if ( CStoreConnectionTaskData::KIND_PAIR == m_taskKind )
        FinishPairing( data );
    else
        FinishConnect( data );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::FinishPairing( const CStoreConnectionTaskData& data )
{GUCEF_TRACE;

    if ( !data.succeeded )
    {
        m_wanted = false;
        Fail( data.problem.IsNULLOrEmpty() ? CORE::CString( "The pairing did not work." ) : data.problem );
        return;
    }
    m_paired = true;
    m_error.Clear();
    NotifyObservers( PairedEvent );
    if ( m_connectAfterPair )
    {
        if ( !Connect() )
        {
            // the reason is in the error text already; the pairing itself worked
            if ( STATE_PAIRING == m_state )
                SetState( STATE_DISCONNECTED );
        }
    }
    else
        SetState( STATE_DISCONNECTED );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::FinishConnect( CStoreConnectionTaskData& data )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreServicePtr service = m_connecting;
    m_connecting.Unlink();
    if ( !data.succeeded )
    {
        if ( !service.IsNULL() )
            service->Close();
        const CORE::CString problem = data.problem.IsNULLOrEmpty() ? CORE::CString( "The store service did not open." ) : data.problem;
        if ( data.transient && m_wanted && m_settings.autoRetry )
        {
            m_error = problem;
            ScheduleRetry();
            return;
        }
        m_wanted = false;
        Fail( problem.HasSubstr( "not paired" ) >= 0 ? problem + " Pair this computer again with the code of a new pairing window." : problem );
        return;
    }
    m_service = service;
    m_info = data.info;
    m_serverName = data.serverName;
    m_serverVersion = data.serverVersion;
    m_activeEndpoint = m_settings.endpoint;
    m_error.Clear();
    m_retrying = false;
    m_backoff.Reset();
    TEventCallback callback( this, &CWorkbenchStoreConnection::OnServiceConnectionState );
    SubscribeTo( m_service.GetPointerAlways(), ASSETSTORE::CIAssetStoreService::ConnectionStateChangedEvent, callback );
    SetState( STATE_CONNECTED );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: connected to " << GetStoreLabel() );
    NotifyObservers( ServiceReadyEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::ScheduleRetry( void )
{GUCEF_TRACE;

    const UInt32 delay = m_backoff.NextDelayMs();
    m_retryAtInMs = NowInMs() + delay;
    m_retryCount = m_backoff.GetAttempt();
    m_retrying = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: the service could not be reached, the next attempt is in " << delay << " ms" );
    if ( STATE_CONNECTING == m_state )
    {
        CORE::TCloneableInt32 data( (Int32) m_state );
        NotifyObservers( StateChangedEvent, &data );
    }
    else
        SetState( STATE_CONNECTING );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::CheckRetry( void )
{GUCEF_TRACE;

    if ( !m_retrying || m_taskRunning || NowInMs() < m_retryAtInMs )
        return;
    m_retrying = false;
    if ( !m_wanted )
        return;
    StartConnectTask();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_inShutdown )
        return;
    CheckFind();
    CheckTask();
    CheckRetry();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::OnServiceConnectionState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetServiceNotice* notice = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetServiceNotice >( eventData );
    if ( GUCEF_NULL == notice || m_service.IsNULL() || !IsConnected() )
        return;
    if ( notice->code.Equals( "CONNECTED", false ) )
    {
        m_error.Clear();
        SetState( STATE_CONNECTED );
    }
    else
    if ( notice->code.Equals( "FAILED", false ) )
    {
        // a permanent refusal: no more attempts; the application goes back to the store of this process
        const CORE::CString reason = notice->text;
        m_wanted = false;
        LoseService();
        Fail( "The store service refused this computer: " + reason + ". If the service was reinstalled or this computer was removed on the server, pair again with the code of a new pairing window." );
    }
    else
        SetState( STATE_OFFLINE );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::ReleaseService( void )
{GUCEF_TRACE;

    if ( m_service.IsNULL() )
        return;
    UnsubscribeFrom( m_service.GetPointerAlways() );
    m_service->Close();
    m_service.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::LoseService( void )
{GUCEF_TRACE;

    if ( m_service.IsNULL() )
        return;
    // the service is still there while the application lets go of it
    NotifyObservers( ServiceLostEvent );
    ReleaseService();
    m_activeEndpoint.Clear();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStoreConnection::Disconnect( void )
{GUCEF_TRACE;

    m_wanted = false;
    m_retrying = false;
    m_connectAfterPair = false;
    if ( m_taskRunning )
        m_discardTask = true;
    LoseService();
    m_error.Clear();
    SetState( STATE_DISCONNECTED );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStoreConnection::ForgetPairing( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_store || m_taskRunning )
        return false;
    Disconnect();
    DRNUTILS::CDeviceCredentials::RemoveFromStore( *m_store, m_settings.credentialsSlot );
    RefreshPaired();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "StoreConnection: the pairing with the store service is forgotten" );
    return !m_paired;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchStoreConnection::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString typeName = "GUCEF::ASSETWORKBENCH::CWorkbenchStoreConnection";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
