/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#include "gucefCOMCORE_CDiscoveryClientBase.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEDISCOVERYTASKCONSUMER_H
#include "gucefCOMCORE_CServiceDiscoveryTaskConsumer.h"
#define GUCEF_COMCORE_CSERVICEDISCOVERYTASKCONSUMER_H
#endif /* GUCEF_COMCORE_CSERVICEDISCOVERYTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H
#include "gucefCOMCORE_CServiceDiscoveryTaskData.h"
#define GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H
#endif /* GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H ? */

#include "gucefCOMCORE_CServiceFinder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CServiceFinder::CandidateFoundEvent = "GUCEF::COMCORE::CServiceFinder::CandidateFoundEvent";
const CORE::CEvent CServiceFinder::CandidateUpdatedEvent = "GUCEF::COMCORE::CServiceFinder::CandidateUpdatedEvent";
const CORE::CEvent CServiceFinder::CandidateLostEvent = "GUCEF::COMCORE::CServiceFinder::CandidateLostEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CServiceFinder::RegisterEvents( void )
{GUCEF_TRACE;

    CandidateFoundEvent.Initialize();
    CandidateUpdatedEvent.Initialize();
    CandidateLostEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServiceFinder::CServiceFinder( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator()
    , m_settings()
    , m_clients()
    , m_clientsCreated( false )
    , m_active( false )
    , m_browsedServiceType()
    , m_candidates()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServiceFinder::CServiceFinder( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator( pulseGenerator )
    , m_settings()
    , m_clients()
    , m_clientsCreated( false )
    , m_active( false )
    , m_browsedServiceType()
    , m_candidates()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServiceFinder::CServiceFinder( const CServiceFinderSettings& settings )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator()
    , m_settings( settings )
    , m_clients()
    , m_clientsCreated( false )
    , m_active( false )
    , m_browsedServiceType()
    , m_candidates()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServiceFinder::CServiceFinder( const CORE::PulseGeneratorPtr& pulseGenerator, const CServiceFinderSettings& settings )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_pulseGenerator( pulseGenerator )
    , m_settings( settings )
    , m_clients()
    , m_clientsCreated( false )
    , m_active( false )
    , m_browsedServiceType()
    , m_candidates()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CServiceFinder::~CServiceFinder()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( m_active )
    {
        for ( size_t i=0; i<m_clients.size(); ++i )
            m_clients[ i ]->StopBrowse( m_browsedServiceType );
    }
    m_clients.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceFinder::GetBrowsedServiceType( void ) const
{GUCEF_TRACE;

    return m_settings.serviceType.IsNULLOrEmpty() ? CIDiscoveryClient::AnyServiceType : m_settings.serviceType;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::PrepareClient( CIDiscoveryClient& client ) const
{GUCEF_TRACE;

    if ( m_pulseGenerator.IsNULL() )
        return true;
    if ( client.SetPulseGenerator( m_pulseGenerator ) )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceFinder: The discovery client " << client.GetDiscoveryClientName() << " cannot run on the pulse generator of the finder, it is not used" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::EnsureClients( void )
{GUCEF_TRACE;

    if ( m_clientsCreated )
        return true;
    m_clientsCreated = true;

    bool configured = CDiscoveryClientList::NamesClients( m_settings.clientConfig );
    if ( !configured && !m_clients.empty() )
        return true;

    TDiscoveryClientVector made;
    bool allMade = CDiscoveryClientList::CreateFromConfig( m_settings.clientConfig, m_settings.defaultClientTypes, made );
    for ( size_t i=0; i<made.size(); ++i )
    {
        if ( PrepareClient( *made[ i ] ) )
            m_clients.push_back( made[ i ] );
        else
            allMade = false;
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::SubscribeToClient( CIDiscoveryClient& client )
{GUCEF_TRACE;

    TEventCallback onFound( this, &CServiceFinder::OnClientEntryFound );
    SubscribeTo( &client, CIDiscoveryClient::EntryDiscoveredEvent, onFound );
    TEventCallback onUpdated( this, &CServiceFinder::OnClientEntryFound );
    SubscribeTo( &client, CIDiscoveryClient::EntryUpdatedEvent, onUpdated );
    TEventCallback onLost( this, &CServiceFinder::OnClientEntryLost );
    SubscribeTo( &client, CIDiscoveryClient::EntryLostEvent, onLost );
    TEventCallback onError( this, &CServiceFinder::OnClientError );
    SubscribeTo( &client, CIDiscoveryClient::DiscoveryErrorEvent, onError );
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::StartClient( CIDiscoveryClient& client )
{GUCEF_TRACE;

    SubscribeToClient( client );
    if ( !client.Browse( m_browsedServiceType ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ServiceFinder: The discovery client " << client.GetDiscoveryClientName() << " cannot browse " << m_browsedServiceType );
        UnsubscribeFrom( &client );
        return false;
    }

    // What the client knows already
    CIDiscovery::TIDiscoveryEntrySet entries;
    client.GetDiscoveredEntries( m_browsedServiceType, entries );
    CIDiscovery::TIDiscoveryEntrySet::iterator i = entries.begin();
    while ( i != entries.end() )
    {
        if ( !(*i).IsNULL() && !OnEntryFound( &client, *(*i) ) )
            return true;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::Start( void )
{GUCEF_TRACE;

    if ( m_active )
        Stop();

    EnsureClients();
    m_browsedServiceType = GetBrowsedServiceType();
    bool anyStarted = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        anyStarted = StartClient( *m_clients[ i ] ) || anyStarted;
    m_active = anyStarted;
    if ( !anyStarted )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceFinder: No discovery client could look for " << m_browsedServiceType );
    return anyStarted;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::Stop( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_clients.size(); ++i )
    {
        UnsubscribeFrom( m_clients[ i ].GetPointerAlways() );
        if ( m_active )
            m_clients[ i ]->StopBrowse( m_browsedServiceType );
    }
    m_active = false;
    m_candidates.clear();
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::Refresh( void )
{GUCEF_TRACE;

    if ( !m_active )
        return false;

    // Browsing a type that is browsed already asks the network again, what is known stays known
    bool anyStarted = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        anyStarted = m_clients[ i ]->Browse( m_browsedServiceType ) || anyStarted;
    return anyStarted;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::GetCandidates( TServiceCandidateVector& candidates ) const
{GUCEF_TRACE;

    candidates.reserve( candidates.size() + m_candidates.size() );
    TKnownCandidateMap::const_iterator i = m_candidates.begin();
    while ( i != m_candidates.end() )
    {
        candidates.push_back( (*i).second.candidate );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CServiceFinder::GetCandidateCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_candidates.size();
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::GetCandidate( const CORE::CString& instanceId, CServiceCandidate& candidate ) const
{GUCEF_TRACE;

    TKnownCandidateMap::const_iterator i = m_candidates.begin();
    while ( i != m_candidates.end() )
    {
        if ( (*i).second.candidate.announcement.instanceId == instanceId )
        {
            candidate = (*i).second.candidate;
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::SetFilter( const CServiceCandidateFilter& filter )
{GUCEF_TRACE;

    m_settings.filter = filter;

    // What no longer matches is lost now
    TServiceCandidateVector lost;
    TKnownCandidateMap::iterator i = m_candidates.begin();
    while ( i != m_candidates.end() )
    {
        if ( !m_settings.filter.IsMatch( (*i).second.candidate ) )
        {
            lost.push_back( (*i).second.candidate );
            m_candidates.erase( i++ );
        }
        else
            ++i;
    }
    for ( size_t n=0; n<lost.size(); ++n )
    {
        TServiceCandidateEventData eData( lost[ n ] );
        if ( !NotifyObservers( CandidateLostEvent, &eData ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::AddDiscoveryClient( CIDiscoveryClientPtr client )
{GUCEF_TRACE;

    if ( client.IsNULL() )
        return false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        if ( m_clients[ i ] == client )
            return true;
    m_clients.push_back( client );
    if ( m_active )
        return StartClient( *client );
    return true;
}

/*-------------------------------------------------------------------------*/

const CServiceFinder::TDiscoveryClientVector&
CServiceFinder::GetDiscoveryClients( void ) const
{GUCEF_TRACE;

    return m_clients;
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::SetSettings( const CServiceFinderSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CServiceFinderSettings&
CServiceFinder::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClient*
CServiceFinder::FindClient( CORE::CNotifier* notifier )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_clients.size(); ++i )
        if ( static_cast< CORE::CNotifier* >( m_clients[ i ].GetPointerAlways() ) == notifier )
            return m_clients[ i ].GetPointerAlways();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::OnEntryFound( CIDiscoveryClient* client, const CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    if ( !m_settings.serviceType.IsNULLOrEmpty() && entry.GetServiceType() != m_settings.serviceType )
        return true;

    CServiceCandidate heard;
    if ( !heard.FromEntry( entry ) )
        return true;
    CORE::CString key = heard.GetKey();

    // What does not match is ignored. Also for a known candidate: anyone on the network can send an announcement with
    // its key, so a non-matching one (another key fingerprint, say) neither replaces nor removes it; the candidate goes
    // when its TTL runs out, at a goodbye, or when SetFilter() no longer matches it
    TKnownCandidateMap::iterator i = m_candidates.find( key );
    if ( !m_settings.filter.IsMatch( heard ) )
        return true;

    if ( i == m_candidates.end() )
    {
        TKnownCandidate& known = m_candidates[ key ];
        known.candidate = heard;
        known.sources.insert( client );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ServiceFinder: Found " << key << " at " << heard.GetFirstEndpoint() );
        TServiceCandidateEventData eData( known.candidate );
        return NotifyObservers( CandidateFoundEvent, &eData );
    }

    TKnownCandidate& known = (*i).second;
    known.sources.insert( client );
    bool changed = !known.candidate.announcement.HasSameContent( heard.announcement ) || known.candidate.host != heard.host;
    CORE::CString::StringSet sources = known.candidate.sourceClientNames;
    sources.insert( heard.sourceClientNames.begin(), heard.sourceClientNames.end() );
    known.candidate = heard;
    known.candidate.sourceClientNames = sources;
    if ( !changed )
        return true;
    TServiceCandidateEventData eData( known.candidate );
    return NotifyObservers( CandidateUpdatedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::OnEntryLost( CIDiscoveryClient* client, const CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    CORE::CString id;
    entry.GetApplicationInstanceId( id );
    CORE::CString key = CIDiscoveryEntry::MakeEntryKey( entry.GetServiceType(), id );
    TKnownCandidateMap::iterator i = m_candidates.find( key );
    if ( i == m_candidates.end() )
        return true;

    TKnownCandidate& known = (*i).second;
    known.sources.erase( client );
    known.candidate.sourceClientNames.erase( client->GetDiscoveryClientName() );
    if ( !known.sources.empty() )
        return true;

    TServiceCandidateEventData eData( known.candidate );
    m_candidates.erase( i );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ServiceFinder: Lost " << key );
    return NotifyObservers( CandidateLostEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::OnClientEntryFound( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIDiscoveryClient* client = FindClient( notifier );
    CDiscoveryEntryEventData* entry = static_cast< CDiscoveryEntryEventData* >( eventData );
    if ( GUCEF_NULL != client && GUCEF_NULL != entry && !entry->IsNULL() )
        OnEntryFound( client, *(*entry) );
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::OnClientEntryLost( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIDiscoveryClient* client = FindClient( notifier );
    CDiscoveryEntryEventData* entry = static_cast< CDiscoveryEntryEventData* >( eventData );
    if ( GUCEF_NULL != client && GUCEF_NULL != entry && !entry->IsNULL() )
        OnEntryLost( client, *(*entry) );
}

/*-------------------------------------------------------------------------*/

void
CServiceFinder::OnClientError( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CIDiscoveryClient* client = FindClient( notifier );
    const CIDiscoveryClient::TDiscoveryErrorEventData* error = static_cast< const CIDiscoveryClient::TDiscoveryErrorEventData* >( eventData );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ServiceFinder: Discovery client " << ( GUCEF_NULL != client ? client->GetDiscoveryClientName() : CORE::CString::Empty )
        << " reports: " << ( GUCEF_NULL != error ? error->GetData() : CORE::CString::Empty ) );
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceFinder::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceFinder";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::FindOnce( const CServiceFinderSettings& settings, TServiceCandidateVector& candidates )
{GUCEF_TRACE;

    // A pulse generator of our own, driven by the calling thread: everything happens on this thread
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    bool found = false;
    {
        CServiceFinder finder( pulseGenerator, settings );
        if ( !finder.Start() )
            return false;

        UInt64 startMs = CDiscoveryClientBase::GetTimeInMs();
        UInt32 minCandidates = settings.findMinCandidates;
        while ( true )
        {
            if ( minCandidates > 0 && finder.GetCandidateCount() >= minCandidates )
            {
                found = true;
                break;
            }
            if ( CDiscoveryClientBase::GetTimeInMs() - startMs >= settings.findTimeoutMs )
                break;
            pulseGenerator->ForceDirectPulse();
            pulseGenerator->WaitTillNextPulseWindow( 1, 10 );
        }
        if ( 0 == minCandidates )
            found = true;
        finder.GetCandidates( candidates );
        finder.Stop();
    }
    // what the clients still had to finish, such as closing their sockets, on this thread too
    pulseGenerator->ForceDirectPulse();
    return found;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CServiceFinder::FindOnceAsync( const CServiceFinderSettings& settings, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    if ( threadPool.IsNULL() )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        threadPool = settings.threadPoolName.IsNULLOrEmpty() ? taskManager.GetThreadPool() : taskManager.GetOrCreateThreadPool( settings.threadPoolName );
    }
    if ( threadPool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );

    CORE::CFutureResultSource source;
    CServiceDiscoveryTaskData* data = GUCEF_NEW CServiceDiscoveryTaskData();
    data->settings = settings;
    data->source = source;

    CORE::CFutureResult queued = threadPool->QueueTask( CServiceDiscoveryTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The look for services could not be queued on the thread pool" );
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

bool
CServiceFinder::GetFindOnceResult( const CORE::CFutureResult& future, TServiceCandidateVector& candidates, bool& found )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const CServiceDiscoveryTaskData* data = CORE::CICloneable::CloneableAs< CServiceDiscoveryTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    candidates = data->candidates;
    found = data->found;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
