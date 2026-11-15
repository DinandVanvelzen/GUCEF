/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#include "gucefASSETSTORE_CAssetIngestEventData.h"
#define GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#include "gucefASSETSTORE_CAssetConversionEventData.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#include "gucefASSETSTORE_CWebSessionSignIn.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CLocalAssetStoreServiceOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONDATA_H
#include "gucefASSETSTORE_CLocalAssetStoreServiceOperationData.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONDATA_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLocalAssetStoreService::IngestJobPrefix = "ingest:";
const CORE::CString CLocalAssetStoreService::ConversionJobPrefix = "convert:";
const CORE::CString CLocalAssetStoreService::PipelineJobPrefix = "pipeline:";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 IngestStepCount = 8;
const UInt32 MaxSuggestions = 64;
const UInt32 MaxBlockBytes = 4 * 1024 * 1024;
const UInt32 MaxStatPaths = 256;

CORE::CString
JoinVfsPath( const CORE::CString& left, const CORE::CString& right )
{
    if ( left.IsNULLOrEmpty() )
        return right;
    if ( right.IsNULLOrEmpty() )
        return left;
    const bool leftSlash = '/' == left[ left.Length() - 1 ];
    const bool rightSlash = '/' == right[ 0 ];
    if ( leftSlash && rightSlash )
        return left + right.CutChars( 1, true );
    if ( leftSlash || rightSlash )
        return left + right;
    return left + "/" + right;
}

Int64
NowMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

TAssetJobState
StateOfDownload( TAssetDownloadJobState state )
{
    switch ( state )
    {
        case ASSETDOWNLOAD_STATE_QUEUED:    return ASSETJOB_STATE_QUEUED;
        case ASSETDOWNLOAD_STATE_PLANNING:  return ASSETJOB_STATE_QUEUED;
        case ASSETDOWNLOAD_STATE_RUNNING:   return ASSETJOB_STATE_RUNNING;
        case ASSETDOWNLOAD_STATE_PAUSED:    return ASSETJOB_STATE_PAUSED;
        case ASSETDOWNLOAD_STATE_VERIFYING: return ASSETJOB_STATE_FINISHING;
        case ASSETDOWNLOAD_STATE_DONE:      return ASSETJOB_STATE_DONE;
        case ASSETDOWNLOAD_STATE_FAILED:    return ASSETJOB_STATE_FAILED;
        default:                            return ASSETJOB_STATE_CANCELLED;
    }
}

/** Takes what the converter reports of a job of the pipeline to the service, and tells it of a cancellation */
class CPipelineJobObserver : public CIAssetConversionObserver
{
    public:

    typedef void ( CLocalAssetStoreService::*TProgressMethod )( const CORE::CString&, UInt32, UInt32, const CORE::CString& );

    CPipelineJobObserver( const CORE::CFutureResultSource& source, CLocalAssetStoreService* service, TProgressMethod progress, const CORE::CString& key )
        : CIAssetConversionObserver()
        , m_source( source )
        , m_service( service )
        , m_progress( progress )
        , m_key( key )
    {
    }

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_source.IsCancellationRequested();
    }

    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {
        ( m_service->*m_progress )( m_key, done, total, assetPath );
    }

    private:

    CORE::CFutureResultSource m_source;
    CLocalAssetStoreService* m_service;
    TProgressMethod m_progress;
    CORE::CString m_key;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLocalAssetStoreService::SIngest::SIngest( void )
    : ingestId()
    , packId()
    , name()
    , step()
    , message()
    , stepIndex( 0 )
    , state( ASSETJOB_STATE_RUNNING )
    , createdAtMs( 0 )
    , updatedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreService::SConversion::SConversion( void )
    : key()
    , jobId()
    , packId()
    , name()
    , message()
    , done( 0 )
    , total( 0 )
    , state( ASSETJOB_STATE_QUEUED )
    , status( ASSETSTORE_STATUS_OK )
    , future( CORE::TASKSTATUS_QUEUEING_FAILED )
    , createdAtMs( 0 )
    , updatedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreService::SPipelineJob::SPipelineJob( void )
    : key()
    , request()
    , name()
    , message()
    , done( 0 )
    , total( 0 )
    , state( ASSETJOB_STATE_QUEUED )
    , status( ASSETSTORE_STATUS_OK )
    , result()
    , source( CORE::TASKSTATUS_QUEUED )
    , createdAtMs( 0 )
    , updatedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreService::CLocalAssetStoreService( const CAssetStoreServiceConfig& config )
    : CIAssetStoreService( config.pulseGenerator )
    , m_config( config )
    , m_storePtr()
    , m_store( GUCEF_NULL )
    , m_manager( GUCEF_NULL )
    , m_imageCache( GUCEF_NULL )
    , m_open( false )
    , m_catalogueOpen( false )
    , m_statusMessage()
    , m_ingests()
    , m_conversions()
    , m_pipelineJobs()
    , m_pendingConversions( CORE::TASKSTATUS_QUEUEING_FAILED )
    , m_rates()
    , m_addJobs()
    , m_conversionCounter( 0 )
    , m_pipelineCounter( 0 )
    , m_endedSessions()
    , m_watchedProviders()
    , m_journal()
    , m_journalHead( 0 )
    , m_revision( 0 )
    , m_commands()
    , m_commandOrder()
    , m_ownerThread( MT::GetCurrentThreadID() )
    , m_pendingCalls( 0 )
    , m_lock()
{GUCEF_TRACE;

    CAssetStoreGlobal::Instance();
    m_config.implementationId = CAssetStoreServiceConfig::LocalImplementationId;
}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreService::~CLocalAssetStoreService()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLocalAssetStoreService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CLocalAssetStoreService";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalAssetStoreService::GetType( void ) const
{GUCEF_TRACE;

    return CAssetStoreServiceConfig::LocalImplementationId;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::IsInProcess( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CAssetStoreServiceConfig& CLocalAssetStoreService::GetConfig( void ) const { return m_config; }
CPrimaryAssetStore* CLocalAssetStoreService::GetPrimaryStore( void ) const { return m_store; }
CAssetStoreDownloadManager* CLocalAssetStoreService::GetDownloadManager( void ) const { return m_manager; }
CAssetStoreImageCache* CLocalAssetStoreService::GetImageCache( void ) const { return m_imageCache; }

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::SetConverterHook( CIAssetConverterHook* converterHook )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_config.converterHook = converterHook;
    if ( GUCEF_NULL != m_store )
        m_store->SetConverterHook( converterHook );
}

/*-------------------------------------------------------------------------*/

CIAssetConverterHook*
CLocalAssetStoreService::GetConverterHook( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.converterHook;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::SetRenderHook( CIAssetRenderHook* renderHook )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_config.renderHook = renderHook;
}

/*-------------------------------------------------------------------------*/

CIAssetRenderHook*
CLocalAssetStoreService::GetRenderHook( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.renderHook;
}

/*-------------------------------------------------------------------------*/

UInt64
CLocalAssetStoreService::GetCatalogueRevision( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_revision;
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalAssetStoreService::GetPendingCallCount( void ) const
{GUCEF_TRACE;

    return m_pendingCalls.Load();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Open( void )
{GUCEF_TRACE;

    if ( m_open )
        return m_catalogueOpen ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_UNAVAILABLE;

    CPrimaryAssetStore* store = GUCEF_NEW CPrimaryAssetStore();
    m_storePtr = TPrimaryAssetStorePtr( store );
    m_store = store;
    m_store->SetSettings( m_config.primaryStore );
    m_store->SetCredentialStore( m_config.credentialStore );
    m_store->SetConverterHook( m_config.converterHook );
    const TAssetStoreStatus status = m_store->Open();
    m_catalogueOpen = ASSETSTORE_STATUS_OK == status;
    if ( !m_catalogueOpen )
    {
        m_statusMessage = "The primary store is not available: " + m_store->GetLastErrorMessage();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CLocalAssetStoreService: " << m_statusMessage );
    }
    else
        m_statusMessage.Clear();

    m_manager = GUCEF_NEW CAssetStoreDownloadManager();
    m_manager->SetSettings( m_config.downloads );
    if ( m_catalogueOpen )
        m_store->AttachDownloadManager( m_manager );

    TEventCallback jobCallback( this, &CLocalAssetStoreService::OnDownloadJobEvent );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobAddedEvent, jobCallback );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobStateChangedEvent, jobCallback );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobFailedEvent, jobCallback );
    TEventCallback finishedCallback( this, &CLocalAssetStoreService::OnDownloadJobFinished );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobFinishedEvent, finishedCallback );
    TEventCallback progressCallback( this, &CLocalAssetStoreService::OnDownloadJobProgress );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobProgressEvent, progressCallback );
    TEventCallback removedCallback( this, &CLocalAssetStoreService::OnDownloadJobRemoved );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobRemovedEvent, removedCallback );

    if ( m_catalogueOpen )
    {
        TEventCallback ingestProgress( this, &CLocalAssetStoreService::OnIngestProgress );
        SubscribeTo( m_store, CPrimaryAssetStore::IngestProgressEvent, ingestProgress );
        TEventCallback ingestFinished( this, &CLocalAssetStoreService::OnIngestFinished );
        SubscribeTo( m_store, CPrimaryAssetStore::IngestFinishedEvent, ingestFinished );
        TEventCallback ingestFailed( this, &CLocalAssetStoreService::OnIngestFailed );
        SubscribeTo( m_store, CPrimaryAssetStore::IngestFailedEvent, ingestFailed );
        TEventCallback conversionProgress( this, &CLocalAssetStoreService::OnConversionProgress );
        SubscribeTo( m_store, CPrimaryAssetStore::ConversionProgressEvent, conversionProgress );
        TEventCallback conversionFinished( this, &CLocalAssetStoreService::OnConversionFinished );
        SubscribeTo( m_store, CPrimaryAssetStore::ConversionFinishedEvent, conversionFinished );
        TEventCallback conversionFailed( this, &CLocalAssetStoreService::OnConversionFailed );
        SubscribeTo( m_store, CPrimaryAssetStore::ConversionFailedEvent, conversionFailed );
        TEventCallback locationCallback( this, &CLocalAssetStoreService::OnLocationStateChanged );
        SubscribeTo( m_store, CPrimaryAssetStore::LocationStateChangedEvent, locationCallback );
    }

    CAssetStoreRegistry& registry = CAssetStoreGlobal::Instance()->GetRegistry();
    TEventCallback addedCallback( this, &CLocalAssetStoreService::OnProviderAdded );
    SubscribeTo( &registry, CAssetStoreRegistry::ProviderAddedEvent, addedCallback );
    TEventCallback removedProviderCallback( this, &CLocalAssetStoreService::OnProviderRemoved );
    SubscribeTo( &registry, CAssetStoreRegistry::ProviderRemovedEvent, removedProviderCallback );

    m_imageCache = GUCEF_NEW CAssetStoreImageCache( GetPulseGenerator() );
    if ( !m_config.imageCacheFolder.IsNULLOrEmpty() )
        m_imageCache->SetCacheFolder( m_config.imageCacheFolder );
    TEventCallback imageCallback( this, &CLocalAssetStoreService::OnImageReady );
    SubscribeTo( m_imageCache, CAssetStoreImageCache::ImageReadyEvent, imageCallback );

    if ( m_config.loadDownloadQueue )
        m_manager->LoadQueue();
    if ( m_config.startDownloads )
        m_manager->Start();
    m_open = true;
    WatchProviders();
    return m_catalogueOpen ? ASSETSTORE_STATUS_OK : status;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::Close( void )
{GUCEF_TRACE;

    if ( !m_open )
        return;
    // the calls that run use the parts below
    while ( m_pendingCalls.Load() > 0 )
        MT::ThreadDelay( 2 );
    UnsubscribeAllFromObserver();
    {
        // the conversions that run end with the store
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_conversions.size(); ++i )
            if ( m_conversions[ i ].future.HasAFuture() && !m_conversions[ i ].future.IsReady() )
                m_conversions[ i ].future.RequestCancellation();
        m_open = false;
    }
    GUCEF_DELETE m_imageCache;
    m_imageCache = GUCEF_NULL;
    if ( GUCEF_NULL != m_store )
        m_store->AttachDownloadManager( GUCEF_NULL );
    if ( GUCEF_NULL != m_manager )
        m_manager->Stop();
    GUCEF_DELETE m_manager;
    m_manager = GUCEF_NULL;
    if ( GUCEF_NULL != m_store )
        m_store->Close();
    m_storePtr.Unlink();
    m_store = GUCEF_NULL;
    m_catalogueOpen = false;

    MT::CScopeMutex lock( m_lock );
    m_watchedProviders.clear();
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::IsCatalogueAvailable( void ) const
{GUCEF_TRACE;

    return m_open && m_catalogueOpen && GUCEF_NULL != m_store && m_store->IsOpen();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalAssetStoreService::GetStatusMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_open )
        return "The asset store service is not open";
    return m_statusMessage;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::NeedCatalogue( CORE::CString& message ) const
{GUCEF_TRACE;

    if ( IsCatalogueAvailable() )
        return ASSETSTORE_STATUS_OK;
    message = m_statusMessage.IsNULLOrEmpty() ? CORE::CString( "The primary store is not open" ) : m_statusMessage;
    return ASSETSTORE_STATUS_UNAVAILABLE;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Refuse( TAssetStoreStatus status, const CORE::CString& message, CAssetServiceResult& result ) const
{GUCEF_TRACE;

    result.status = status;
    result.message = message;
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::IsOnNotificationThread( void ) const
{GUCEF_TRACE;

    const TThreadId current = MT::GetCurrentThreadID();
    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    const TThreadId driver = pulseGenerator.IsNULL() ? 0 : pulseGenerator->GetPulseDriverThreadId();
    if ( 0 != driver )
        return driver == current;
    // nothing drives the pulses (an application or a test that pulses by hand): the thread that made the service owns it
    return m_ownerThread == current;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::Raise( const CORE::CEvent& eventId, const CORE::CICloneable& eventData )
{GUCEF_TRACE;

    // a call on the thread of the events tells its observers at once, as the parts of the store do; other threads mail
    if ( IsOnNotificationThread() )
    {
        CORE::CICloneable* copy = eventData.Clone();
        NotifyObservers( eventId, copy );
        GUCEF_DELETE copy;
    }
    else
        NotifyObserversFromThread( eventId, eventData.Clone() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::RecordChange( TAssetCatalogueChangeKind kind, const CORE::CString& id )
{GUCEF_TRACE;

    CAssetCatalogueChange change;
    {
        MT::CScopeMutex lock( m_lock );
        change.revision = ++m_revision;
        change.kind = kind;
        if ( !id.IsNULLOrEmpty() )
            change.ids.push_back( id );
        change.timeUtcMs = NowMs();
        if ( m_journal.size() < JournalCapacity )
            m_journal.push_back( change );
        else
        {
            m_journal[ m_journalHead ] = change;
            m_journalHead = ( m_journalHead + 1 ) % JournalCapacity;
        }
    }
    Raise( CatalogueChangedEvent, change );
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::ReplayCommand( const CORE::CString& commandId, const CORE::CString& operation, CAssetServiceResult& result )
{GUCEF_TRACE;

    if ( commandId.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    TCommandMap::const_iterator i = m_commands.find( commandId );
    if ( i == m_commands.end() )
        return false;
    if ( (*i).second.operation != operation )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "The command id " + commandId + " was used for " + (*i).second.operation + " already";
        return true;
    }
    result = (*i).second.outcome;
    result.replayed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::RememberCommand( const CORE::CString& commandId, const CORE::CString& operation, const CAssetServiceResult& result )
{GUCEF_TRACE;

    if ( commandId.IsNULLOrEmpty() )
        return;
    MT::CScopeMutex lock( m_lock );
    if ( m_commands.find( commandId ) == m_commands.end() )
    {
        m_commandOrder.push_back( commandId );
        if ( m_commandOrder.size() > CommandCapacity )
        {
            m_commands.erase( m_commandOrder.front() );
            m_commandOrder.erase( m_commandOrder.begin() );
        }
    }
    SCommand& command = m_commands[ commandId ];
    command.operation = operation;
    command.outcome = result;
    command.outcome.replayed = false;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CLocalAssetStoreService::ExecuteAsync( const CAssetServiceCall& call )
{GUCEF_TRACE;

    // the operations of the store that have asynchronous forms of their own: a cancellation of the future reaches them
    if ( IsCatalogueAvailable() && call.commandId.IsNULLOrEmpty() )
    {
        if ( CAssetServiceOperations::Ingest == call.operation )
        {
            CAssetIngestRequest request;
            call.GetRecord( "request", request );
            return m_store->IngestAsync( request );
        }
        if ( CAssetServiceOperations::ResumeIngest == call.operation )
            return m_store->ResumeIngestAsync( call.GetText( "ingestId" ) );
        if ( CAssetServiceOperations::VerifyLocation == call.operation )
            return m_store->VerifyLocationAsync( call.GetText( "locationId" ) );
    }
    return QueueCall( call, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CLocalAssetStoreService::QueueCall( const CAssetServiceCall& call, const CORE::CString& pipelineJobKey )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_config.threadPoolName );
    if ( threadPool.IsNULL() )
        return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
    CLocalAssetStoreServiceOperationData* data = GUCEF_NEW CLocalAssetStoreServiceOperationData();
    data->service = this;
    data->call = call;
    data->pipelineJobKey = pipelineJobKey;
    CORE::CFutureResultSource source;
    if ( !pipelineJobKey.IsNULLOrEmpty() )
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == pipelineJobKey )
                source = m_pipelineJobs[ i ].source;
    }
    data->source = source;
    ++m_pendingCalls;
    CORE::CFutureResult queued = threadPool->QueueTask( CLocalAssetStoreServiceOperationTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        --m_pendingCalls;
        GUCEF_DELETE data;
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The call could not be queued on the thread pool" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::CallEnded( void )
{GUCEF_TRACE;

    --m_pendingCalls;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CLocalAssetStoreService::AnswerPromptAsync( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = AnswerPrompt( commandId, promptId, answer, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CLocalAssetStoreService::UnlockKeyringAsync( const CORE::CString& commandId, const AUTH::CSecretString& code )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = UnlockKeyring( commandId, code, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CLocalAssetStoreService::WriteUploadBlockAsync( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    CAssetServiceResult* result = GUCEF_NEW CAssetServiceResult();
    const TAssetStoreStatus status = WriteUploadBlock( uploadId, relativePath, offset, data, *result );
    return MakeEndedFuture( status, result, result->message );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::GetLocationInfos( TAssetLocationInfoVector& locations ) const
{GUCEF_TRACE;

    locations.clear();
    const CPrimaryAssetStoreSettings settings = GUCEF_NULL != m_store ? m_store->GetSettings() : m_config.primaryStore;
    for ( size_t i=0; i<settings.locations.size(); ++i )
    {
        CAssetLocationInfo info;
        info.location = settings.locations[ i ];
        info.online = IsCatalogueAvailable() && m_store->IsLocationOnline( settings.locations[ i ].locationId );
        locations.push_back( info );
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetServiceInfo( CAssetServiceInfo& result )
{GUCEF_TRACE;

    result.Clear();
    result.implementationId = GetType();
    result.version = "1.0";
    result.inProcess = true;
    result.roles.insert( "viewer" );
    result.roles.insert( "curator" );
    result.roles.insert( "renderer" );
    result.roles.insert( "admin" );
    result.maxPageEntries = MaxPageEntries;
    result.catalogueRevision = GetCatalogueRevision();
    result.keyringState = "none";
    GetLocationInfos( result.locations );
    CAssetStoreInfoPage stores;
    ListStores( stores );
    result.stores = stores.entries;
    const CPrimaryAssetStoreSettings settings = GUCEF_NULL != m_store ? m_store->GetSettings() : m_config.primaryStore;
    result.sources = settings.sources;
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL != hook )
    {
        result.converterName = hook->GetName();
        result.converterProfile = hook->GetProfileName();
        result.capabilities.insert( "conversion" );
        if ( hook->SupportsPipelineOperation( CAssetPipelineOperations::PackList ) || hook->SupportsPipelineOperation( CAssetPipelineOperations::ConvertPack ) )
            result.capabilities.insert( "pipeline" );
    }
    result.hasRenderer = GUCEF_NULL != GetRenderHook();
    if ( result.hasRenderer )
        result.capabilities.insert( "render" );
    if ( IsCatalogueAvailable() )
    {
        result.capabilities.insert( "catalogue" );
        result.capabilities.insert( "search" );
        result.capabilities.insert( "suggest" );
        result.capabilities.insert( "savedSearches" );
        result.capabilities.insert( "vocabulary" );
        result.capabilities.insert( "curation.mappings" );
        result.capabilities.insert( "curation.synonyms" );
        result.capabilities.insert( "curation.rules" );
        result.capabilities.insert( "removePack" );
        result.capabilities.insert( "locations" );
        result.capabilities.insert( "ingest" );
    }
    result.capabilities.insert( "files.read" );
    result.capabilities.insert( "jobs" );
    result.capabilities.insert( "stores" );
    result.capabilities.insert( "listingImages" );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetCatalogueChanges( UInt64 sinceRevision, UInt32 maxEntries, CAssetCatalogueChangePage& result )
{GUCEF_TRACE;

    result.Clear();
    const UInt32 count = ClampPageEntries( maxEntries );
    MT::CScopeMutex lock( m_lock );
    result.revision = m_revision;
    const UInt32 size = (UInt32) m_journal.size();
    const UInt64 oldest = size > 0 ? m_journal[ m_journalHead % size ].revision : m_revision + 1;
    if ( sinceRevision + 1 < oldest && sinceRevision < m_revision )
    {
        // the journal no longer holds the changes after the revision
        CAssetCatalogueChange resync;
        resync.revision = m_revision;
        resync.kind = ASSETCHANGE_RESYNC;
        resync.timeUtcMs = NowMs();
        result.entries.push_back( resync );
        return ASSETSTORE_STATUS_OK;
    }
    for ( UInt32 i=0; i<size; ++i )
    {
        const CAssetCatalogueChange& change = m_journal[ ( m_journalHead + i ) % size ];
        if ( change.revision <= sinceRevision )
            continue;
        if ( (UInt32) result.entries.size() >= count )
        {
            result.nextPageToken = CORE::UInt64ToString( result.entries.back().revision );
            break;
        }
        result.entries.push_back( change );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Search( const CAssetSearchQuery& query, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    const UInt64 revision = GetCatalogueRevision();
    const TAssetStoreStatus status = m_store->Search( query, result );
    result.revision = revision;
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Suggest( const CORE::CString& prefix, UInt32 maxCount, CAssetServiceStringList& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    result.revision = GetCatalogueRevision();
    return m_store->Suggest( prefix, maxCount > MaxSuggestions ? MaxSuggestions : maxCount, result.values );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetVocabulary( CAssetVocabularyInfo& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    CAssetVocabulary& vocabulary = m_store->GetVocabulary();
    result.revision = GetCatalogueRevision();
    vocabulary.GetTerms( "kind", result.kinds );
    vocabulary.GetTerms( "category", result.categories );
    vocabulary.GetTerms( "theme", result.themes );
    vocabulary.GetTerms( "style", result.styles );
    vocabulary.GetTerms( "licenceClass", result.licenceClasses );
    result.restrictionFlags.push_back( CAssetLicence::NoMachineLearning );
    result.restrictionFlags.push_back( CAssetLicence::AttributionRequired );
    result.restrictionFlags.push_back( CAssetLicence::NoCommercialUse );
    result.restrictionFlags.push_back( CAssetLicence::NoRedistribution );
    result.restrictionFlags.push_back( CAssetLicence::EditorialOnly );
    CORE::CString::StringVector providerIds;
    CAssetStoreGlobal::Instance()->GetRegistry().GetProviderIds( providerIds );
    providerIds.push_back( CPrimaryAssetStoreSettings::DiskSourceId );
    for ( size_t i=0; i<providerIds.size(); ++i )
        result.mappingCountsBySource[ providerIds[ i ] ] = vocabulary.GetMappingCount( providerIds[ i ] );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetPack( const CORE::CString& packId, CAssetPackRecord& result )
{GUCEF_TRACE;

    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    if ( packId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    return m_store->GetPack( packId, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetAssets( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRecordPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    if ( packId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    result.revision = GetCatalogueRevision();
    TAssetRecordVector assets;
    const TAssetStoreStatus status = m_store->GetAssets( packId, assets );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    FillPage( assets, pageToken, maxEntries, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetAsset( const CORE::CString& assetId, CAssetRecord& result )
{GUCEF_TRACE;

    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    if ( assetId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CORE::CDataTableQuery query;
    query.WhereEquals( "asset_id", CORE::CVariant( assetId ) );
    CAssetCatalogue::TRowVector rows;
    TAssetStoreStatus status = m_store->GetCatalogue().Select( "assets", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    TAssetRecordVector assets;
    status = m_store->GetAssets( CAssetCatalogue::GetText( rows[ 0 ], "pack_id" ), assets );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    for ( size_t i=0; i<assets.size(); ++i )
    {
        if ( assets[ i ].assetId == assetId )
        {
            result = assets[ i ];
            return ASSETSTORE_STATUS_OK;
        }
    }
    return ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetRelations( const CORE::CString& assetId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetRelationPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    if ( assetId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    result.revision = GetCatalogueRevision();
    GUCEF::vector< CAssetRelationRecord > relations;
    const char* const columns[] = { "from_asset", "to_asset" };
    for ( UInt32 c=0; c<2; ++c )
    {
        CORE::CDataTableQuery query;
        query.WhereEquals( columns[ c ], CORE::CVariant( assetId ) );
        CAssetCatalogue::TRowVector rows;
        const TAssetStoreStatus status = m_store->GetCatalogue().Select( "relations", query, rows );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        for ( size_t r=0; r<rows.size(); ++r )
        {
            CAssetRelationRecord relation;
            relation.fromAssetId = CAssetCatalogue::GetText( rows[ r ], "from_asset" );
            relation.kind = CAssetCatalogue::GetText( rows[ r ], "kind" );
            relation.toAssetId = CAssetCatalogue::GetText( rows[ r ], "to_asset" );
            // a relation of the asset with itself is listed once
            if ( 1 == c && relation.fromAssetId == assetId )
                continue;
            relations.push_back( relation );
        }
    }
    FillPage( relations, pageToken, maxEntries, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetIngestHistory( const CORE::CString& packId, const CORE::CString& pageToken, UInt32 maxEntries, CAssetIngestHistoryPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    result.revision = GetCatalogueRevision();
    GUCEF::vector< CAssetIngestHistoryEntry > entries;
    const TAssetStoreStatus status = m_store->GetIngestHistory( entries, packId );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    FillPage( entries, pageToken, maxEntries, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetSavedSearches( const CORE::CString& pageToken, UInt32 maxEntries, CAssetSavedSearchPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    result.revision = GetCatalogueRevision();
    TAssetSavedSearchVector searches;
    const TAssetStoreStatus status = m_store->GetSavedSearches( searches );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    FillPage( searches, pageToken, maxEntries, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SaveSearch( const CORE::CString& commandId, const CAssetSavedSearch& search, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SaveSearch, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( search.searchId.IsNULLOrEmpty() || search.name.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A saved search needs an id and a name", result );
    status = m_store->SaveSearch( search );
    result.status = status;
    result.id = search.searchId;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_SAVED_SEARCHES, search.searchId );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::SaveSearch, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::DeleteSavedSearch( const CORE::CString& commandId, const CORE::CString& searchId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::DeleteSavedSearch, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    status = m_store->DeleteSavedSearch( searchId );
    result.status = status;
    result.id = searchId;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_SAVED_SEARCHES, searchId );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::DeleteSavedSearch, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RunSavedSearch( const CORE::CString& searchId, UInt32 pageIndex, CAssetSearchResultPage& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    const UInt64 revision = GetCatalogueRevision();
    const TAssetStoreStatus status = m_store->RunSavedSearch( searchId, pageIndex, result );
    result.revision = revision;
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetAssetFileSet( const CORE::CString& assetId, CAssetFileSet& result )
{GUCEF_TRACE;

    result.Clear();
    CAssetRecord asset;
    TAssetStoreStatus status = GetAsset( assetId, asset );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CAssetPackRecord pack;
    status = m_store->GetPack( asset.packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    result.assetId = asset.assetId;
    result.packId = asset.packId;
    result.mainStatus = m_store->GetAssetVfsPath( assetId, result.mainVfsPath, result.mainMessage );
    result.convertedStatus = m_store->GetAssetConvertedVfsPath( assetId, result.convertedVfsPath, result.convertedMessage );
    const CORE::CString packRoot = JoinVfsPath( m_store->GetLocationRoot( pack.locationId ), pack.rootPath );

    // its own files, then the main files of the assets it uses
    for ( size_t f=0; f<asset.files.size(); ++f )
    {
        CAssetFileRecord file = asset.files[ f ];
        file.path = JoinVfsPath( packRoot, file.path );
        result.totalBytes += file.sizeBytes > 0 ? (UInt64) file.sizeBytes : 0;
        result.files.push_back( file );
    }
    if ( ASSETSTORE_STATUS_OK == result.convertedStatus )
    {
        CAssetFileRecord converted;
        converted.path = result.convertedVfsPath;
        converted.role = "converted";
        result.files.push_back( converted );
    }
    CAssetRelationPage relations;
    if ( ASSETSTORE_STATUS_OK == GetRelations( assetId, CORE::CString::Empty, MaxPageEntries, relations ) )
    {
        for ( size_t r=0; r<relations.entries.size(); ++r )
        {
            const CAssetRelationRecord& relation = relations.entries[ r ];
            if ( relation.fromAssetId != assetId || relation.toAssetId == assetId )
                continue;
            CAssetRecord used;
            CAssetPackRecord usedPack;
            if ( ASSETSTORE_STATUS_OK != GetAsset( relation.toAssetId, used ) || ASSETSTORE_STATUS_OK != m_store->GetPack( used.packId, usedPack ) )
                continue;
            const CORE::CString usedRoot = JoinVfsPath( m_store->GetLocationRoot( usedPack.locationId ), usedPack.rootPath );
            for ( size_t f=0; f<used.files.size(); ++f )
            {
                if ( "main" != used.files[ f ].role )
                    continue;
                CAssetFileRecord file = used.files[ f ];
                file.path = JoinVfsPath( usedRoot, file.path );
                file.role = "referenced";
                result.totalBytes += file.sizeBytes > 0 ? (UInt64) file.sizeBytes : 0;
                result.files.push_back( file );
            }
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetLicence( const CORE::CString& licenceOrPackId, CAssetLicence& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    if ( licenceOrPackId.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    CAssetPackRecord pack;
    if ( ASSETSTORE_STATUS_OK == m_store->GetPack( licenceOrPackId, pack ) )
    {
        result = pack.licence;
        return ASSETSTORE_STATUS_OK;
    }
    CORE::CDataTableQuery query;
    query.WhereEquals( "licence_id", CORE::CVariant( licenceOrPackId ) );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = m_store->GetCatalogue().Select( "licences", query, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result.licenceId = CAssetCatalogue::GetText( rows[ 0 ], "licence_id" );
    result.licenceClass = CAssetCatalogue::GetText( rows[ 0 ], "licence_class" );
    result.name = CAssetCatalogue::GetText( rows[ 0 ], "name" );
    result.url = CAssetCatalogue::GetText( rows[ 0 ], "url" );
    result.text = CAssetCatalogue::GetText( rows[ 0 ], "text" );
    result.restrictions = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( rows[ 0 ], "restrictions" ) );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory,
                                             const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SetCategoryMapping, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( sourceId.IsNULLOrEmpty() || storeCategory.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A mapping needs a source and a store category", result );
    status = m_store->GetVocabulary().AddCategoryMapping( m_store->GetCatalogue(), sourceId, storeCategory, categoryId, theme, kind );
    result.status = status;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_VOCABULARY, CORE::CString::Empty );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetCatalogue().GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::SetCategoryMapping, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetSynonym( const CORE::CString& commandId, const CORE::CString& word, const CORE::CString& replacement, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SetSynonym, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( word.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A synonym needs a word", result );
    status = m_store->GetVocabulary().AddSynonym( m_store->GetCatalogue(), word, replacement );
    result.status = status;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_VOCABULARY, CORE::CString::Empty );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetCatalogue().GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::SetSynonym, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetCategoryRule( const CORE::CString& commandId, const CAssetCategoryRule& rule, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SetCategoryRule, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( rule.ruleId.IsNULLOrEmpty() || rule.pattern.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A rule needs an id and a pattern", result );
    status = m_store->GetVocabulary().AddCategoryRule( m_store->GetCatalogue(), rule.ruleId, rule.matchOn, rule.pattern, rule.categoryId,
                                                        rule.kind, rule.themes, rule.artStyles, rule.priority );
    result.status = status;
    result.id = rule.ruleId;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_VOCABULARY, CORE::CString::Empty );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetCatalogue().GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::SetCategoryRule, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RemovePack( const CORE::CString& commandId, const CORE::CString& packId, bool deleteFiles, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::RemovePack, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( packId.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "No pack is given", result );
    status = m_store->RemovePack( packId, deleteFiles );
    result.status = status;
    result.id = packId;
    if ( ASSETSTORE_STATUS_OK == status )
    {
        RecordChange( ASSETCHANGE_PACK_REMOVED, packId );
        result.revision = GetCatalogueRevision();
    }
    else
        result.message = m_store->GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::RemovePack, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::StatFiles( const CORE::CString::StringVector& vfsPaths, CAssetFileStatPage& result )
{GUCEF_TRACE;

    result.Clear();
    if ( vfsPaths.size() > MaxStatPaths )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t i=0; i<vfsPaths.size(); ++i )
    {
        CAssetFileRecord entry;
        entry.path = vfsPaths[ i ];
        if ( vfs.FileExists( vfsPaths[ i ] ) )
        {
            UInt64 size = 0;
            entry.role = "file";
            if ( CAssetDigest::ComputeVfsFileDigest( vfsPaths[ i ], CAssetDigest::Sha256, entry.digest, &size ) )
                entry.sizeBytes = (Int64) size;
            else
                entry.sizeBytes = (Int64) vfs.GetFileSize( vfsPaths[ i ] );
        }
        else
        if ( vfs.DirExists( vfsPaths[ i ] ) )
            entry.role = "directory";
        else
            entry.role = "missing";
        result.entries.push_back( entry );
    }
    result.total = result.entries.size();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ListDirectory( const CORE::CString& vfsPath, const CORE::CString& pageToken, UInt32 maxEntries, CAssetFileStatPage& result )
{GUCEF_TRACE;

    result.Clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( vfsPath ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    VFS::CVFS::TStringVector dirs;
    VFS::CVFS::TStringVector files;
    vfs.GetDirList( dirs, vfsPath, false, false, CORE::CString::Empty );
    vfs.GetFileList( files, vfsPath, false, false, CORE::CString::Empty );
    GUCEF::vector< CAssetFileRecord > entries;
    for ( size_t i=0; i<dirs.size(); ++i )
    {
        CAssetFileRecord entry;
        entry.path = JoinVfsPath( vfsPath, dirs[ i ] );
        entry.role = "directory";
        entries.push_back( entry );
    }
    for ( size_t i=0; i<files.size(); ++i )
    {
        CAssetFileRecord entry;
        entry.path = JoinVfsPath( vfsPath, files[ i ] );
        entry.role = "file";
        entry.sizeBytes = (Int64) vfs.GetFileSize( entry.path );
        entries.push_back( entry );
    }
    FillPage( entries, pageToken, maxEntries, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ReadFileBlock( const CORE::CString& vfsPath, UInt64 offset, UInt32 length, CAssetFileBlock& result )
{GUCEF_TRACE;

    result.Clear();
    result.vfsPath = vfsPath;
    result.offset = offset;
    if ( length > MaxBlockBytes )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( vfsPath ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "rb", false );
    if ( file.IsNULL() )
        return ASSETSTORE_STATUS_IO_ERROR;
    CORE::IOAccessPtr access = file->GetAccess();
    if ( access.IsNULL() )
        return ASSETSTORE_STATUS_IO_ERROR;
    result.fileSize = access->GetSize();
    if ( offset > result.fileSize )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    // Setpos follows fsetpos: 0 is success
    if ( 0 != access->Setpos( offset ) )
        return ASSETSTORE_STATUS_IO_ERROR;
    const UInt64 left = result.fileSize - offset;
    const UInt32 wanted = left < (UInt64) length ? (UInt32) left : length;
    result.data.SetBufferSize( wanted > 0 ? wanted : 1 );
    const UInt32 read = wanted > 0 ? access->Read( result.data.GetBufferPtr(), 1, wanted ) : 0;
    result.data.SetDataSize( read );
    result.endOfFile = offset + read >= result.fileSize;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetLocations( CAssetLocationPage& result )
{GUCEF_TRACE;

    result.Clear();
    TAssetLocationInfoVector locations;
    GetLocationInfos( locations );
    result.entries = locations;
    result.total = locations.size();
    result.revision = GetCatalogueRevision();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RefreshLocationStates( CAssetLocationPage& result )
{GUCEF_TRACE;

    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    m_store->RefreshLocationStates();
    return GetLocations( result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RelocateLocation( const CORE::CString& commandId, const CORE::CString& locationId, const CORE::CString& newVfsRoot,
                                           const CORE::CString& newFileSystemPath, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::RelocateLocation, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    status = m_store->RelocateLocation( locationId, newVfsRoot, newFileSystemPath );
    result.status = status;
    result.id = locationId;
    if ( ASSETSTORE_STATUS_OK != status )
        result.message = m_store->GetLastErrorMessage();
    RememberCommand( commandId, CAssetServiceOperations::RelocateLocation, result );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::VerifyLocation( const CORE::CString& commandId, const CORE::CString& locationId, CAssetVerificationReport& result )
{GUCEF_TRACE;

    // a verification may run again: the command id is not used to answer a repeat
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    return m_store->VerifyLocation( locationId, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Ingest( const CORE::CString& commandId, const CAssetIngestRequest& request, CAssetIngestResult& result )
{GUCEF_TRACE;

    CAssetServiceResult replay;
    if ( ReplayCommand( commandId, CAssetServiceOperations::Ingest, replay ) )
    {
        result.status = replay.status;
        result.message = replay.message;
        result.ingestId = replay.id;
        result.packId = replay.relatedId;
        return result.status;
    }
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
    {
        result.status = available;
        result.message = message;
        return available;
    }
    const TAssetStoreStatus status = m_store->Ingest( request, result );
    CAssetServiceResult outcome;
    outcome.status = status;
    outcome.message = result.message;
    outcome.id = result.ingestId;
    outcome.relatedId = result.packId;
    RememberCommand( commandId, CAssetServiceOperations::Ingest, outcome );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ResumeIngest( const CORE::CString& commandId, const CORE::CString& ingestId, CAssetIngestResult& result )
{GUCEF_TRACE;

    CAssetServiceResult replay;
    if ( ReplayCommand( commandId, CAssetServiceOperations::ResumeIngest, replay ) )
    {
        result.status = replay.status;
        result.message = replay.message;
        result.ingestId = replay.id;
        result.packId = replay.relatedId;
        return result.status;
    }
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
    {
        result.status = available;
        result.message = message;
        return available;
    }
    const TAssetStoreStatus status = m_store->ResumeIngest( ingestId, result );
    CAssetServiceResult outcome;
    outcome.status = status;
    outcome.message = result.message;
    outcome.id = result.ingestId;
    outcome.relatedId = result.packId;
    RememberCommand( commandId, CAssetServiceOperations::ResumeIngest, outcome );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetPendingIngests( CAssetServiceStringList& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::CString message;
    const TAssetStoreStatus available = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != available )
        return available;
    m_store->GetPendingIngests( result.values );
    result.revision = GetCatalogueRevision();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ListImportRoots( CAssetServiceStringList& result )
{GUCEF_TRACE;

    // in this process every folder of the VFS can be ingested by its path: there are no roots that are exported for it
    result.Clear();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetSourcePolicy( const CORE::CString& commandId, const CAssetSourcePolicy& policy, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SetSourcePolicy, result ) )
        return result.status;
    if ( policy.sourceId.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A policy needs a source", result );
    {
        MT::CScopeMutex lock( m_lock );
        m_config.primaryStore.SetPolicy( policy );
    }
    if ( GUCEF_NULL != m_store )
    {
        CPrimaryAssetStoreSettings settings = m_store->GetSettings();
        settings.SetPolicy( policy );
        m_store->SetSettings( settings );
    }
    result.status = ASSETSTORE_STATUS_OK;
    result.id = policy.sourceId;
    RememberCommand( commandId, CAssetServiceOperations::SetSourcePolicy, result );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalAssetStoreService::GetPackName( const CORE::CString& packId ) const
{GUCEF_TRACE;

    CAssetPackRecord pack;
    if ( IsCatalogueAvailable() && !packId.IsNULLOrEmpty() && ASSETSTORE_STATUS_OK == m_store->GetPack( packId, pack ) && !pack.title.IsNULLOrEmpty() )
        return pack.title;
    return "Pack " + packId.SubstrToIndex( packId.Length() > 8 ? 8 : packId.Length(), true );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::MakeDownloadEntry( const CAssetStoreDownloadJob& job, CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.jobId = job.jobId;
    entry.kind = "download";
    entry.name = job.itemTitle.IsNULLOrEmpty() ? job.itemId : job.itemTitle;
    entry.detail = job.providerId + ( job.formatId.IsNULLOrEmpty() ? CORE::CString::Empty : " " + job.formatId ) + ( job.forIngest ? ", for the primary store" : CORE::CString::Empty );
    entry.totalBytes = job.GetTotalBytes();
    entry.doneBytes = job.GetBytesDone();
    entry.state = StateOfDownload( job.state );
    if ( ASSETDOWNLOAD_STATE_PLANNING == job.state )
        entry.stateText = "Planning";
    entry.message = job.message;
    entry.status = job.status;
    entry.folder = job.destination;
    entry.providerId = job.providerId;
    entry.itemId = job.itemId;
    entry.forIngest = job.forIngest;
    for ( size_t f=0; f<job.files.size(); ++f )
        entry.files.push_back( job.GetFilePath( job.files[ f ] ) );
    entry.createdAtMs = job.createdAtMs;
    entry.updatedAtMs = job.updatedAtMs;
    TRateMap::const_iterator rate = m_rates.find( job.jobId );
    if ( rate != m_rates.end() && ASSETJOB_STATE_RUNNING == entry.state )
    {
        entry.bytesPerSecond = (*rate).second.bytesPerSecond;
        entry.secondsLeft = (*rate).second.secondsLeft;
    }
    for ( size_t i=0; i<m_addJobs.size(); ++i )
        if ( m_addJobs[ i ].jobId == job.jobId )
            entry.ingestRequested = m_addJobs[ i ].ingestRequested;
    const bool ended = entry.IsEnded();
    entry.SetActionAvailable( ASSETJOB_ACTION_PAUSE, ASSETJOB_STATE_QUEUED == entry.state || ASSETJOB_STATE_RUNNING == entry.state );
    entry.SetActionAvailable( ASSETJOB_ACTION_RESUME, ASSETJOB_STATE_PAUSED == entry.state );
    entry.SetActionAvailable( ASSETJOB_ACTION_CANCEL, !ended );
    entry.SetActionAvailable( ASSETJOB_ACTION_RETRY, ASSETJOB_STATE_FAILED == entry.state || ASSETJOB_STATE_CANCELLED == entry.state );
    entry.SetActionAvailable( ASSETJOB_ACTION_REMOVE, ended );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::MakeIngestEntry( const SIngest& ingest, CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.jobId = IngestJobPrefix + ingest.ingestId;
    entry.kind = "ingest";
    entry.name = ingest.name;
    entry.detail = "into the primary store";
    entry.state = ingest.state;
    entry.fraction = ASSETJOB_STATE_DONE == ingest.state ? 1.0f : (Float32) ingest.stepIndex / (Float32) IngestStepCount;
    if ( ASSETJOB_STATE_RUNNING == ingest.state && !ingest.step.IsNULLOrEmpty() )
        entry.stateText = ingest.step;
    entry.message = ingest.message;
    entry.packId = ingest.packId;
    entry.createdAtMs = ingest.createdAtMs;
    entry.updatedAtMs = ingest.updatedAtMs;
    entry.SetActionAvailable( ASSETJOB_ACTION_RETRY, ASSETJOB_STATE_FAILED == ingest.state && IsCatalogueAvailable() );
    entry.SetActionAvailable( ASSETJOB_ACTION_REMOVE, entry.IsEnded() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::MakeConversionEntry( const SConversion& conversion, CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.jobId = ConversionJobPrefix + conversion.key;
    entry.kind = "conversion";
    entry.name = conversion.name;
    entry.detail = "converted files of the pack";
    entry.state = conversion.state;
    entry.status = conversion.status;
    entry.fraction = ASSETJOB_STATE_DONE == conversion.state ? 1.0f : ( conversion.total > 0 ? (Float32) conversion.done / (Float32) conversion.total : 0.0f );
    entry.itemsDone = conversion.done;
    entry.itemsTotal = conversion.total;
    entry.message = conversion.message;
    entry.packId = conversion.packId;
    entry.createdAtMs = conversion.createdAtMs;
    entry.updatedAtMs = conversion.updatedAtMs;
    const bool running = ASSETJOB_STATE_RUNNING == conversion.state || ASSETJOB_STATE_QUEUED == conversion.state;
    entry.SetActionAvailable( ASSETJOB_ACTION_CANCEL, running );
    entry.SetActionAvailable( ASSETJOB_ACTION_RETRY, ( ASSETJOB_STATE_FAILED == conversion.state || ASSETJOB_STATE_CANCELLED == conversion.state ) && IsCatalogueAvailable() );
    entry.SetActionAvailable( ASSETJOB_ACTION_REMOVE, entry.IsEnded() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::MakePipelineEntry( const SPipelineJob& job, CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    entry.Clear();
    entry.jobId = PipelineJobPrefix + job.key;
    entry.kind = "pipeline";
    entry.name = job.name;
    entry.detail = job.request.operation;
    entry.state = job.state;
    entry.status = job.status;
    entry.fraction = ASSETJOB_STATE_DONE == job.state ? 1.0f : ( job.total > 0 ? (Float32) job.done / (Float32) job.total : 0.0f );
    entry.itemsDone = job.done;
    entry.itemsTotal = job.total;
    entry.message = job.message;
    entry.createdAtMs = job.createdAtMs;
    entry.updatedAtMs = job.updatedAtMs;
    entry.SetActionAvailable( ASSETJOB_ACTION_CANCEL, !entry.IsEnded() );
    entry.SetActionAvailable( ASSETJOB_ACTION_REMOVE, entry.IsEnded() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::GetAllJobs( TAssetJobEntryVector& jobs ) const
{GUCEF_TRACE;

    jobs.clear();
    TAssetStoreDownloadJobVector downloads;
    if ( GUCEF_NULL != m_manager )
        m_manager->GetJobs( downloads );
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<downloads.size(); ++i )
    {
        CAssetJobEntry entry;
        MakeDownloadEntry( downloads[ i ], entry );
        jobs.push_back( entry );
    }
    for ( size_t i=0; i<m_ingests.size(); ++i )
    {
        CAssetJobEntry entry;
        MakeIngestEntry( m_ingests[ i ], entry );
        jobs.push_back( entry );
    }
    for ( size_t i=0; i<m_conversions.size(); ++i )
    {
        CAssetJobEntry entry;
        MakeConversionEntry( m_conversions[ i ], entry );
        jobs.push_back( entry );
    }
    for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
    {
        CAssetJobEntry entry;
        MakePipelineEntry( m_pipelineJobs[ i ], entry );
        jobs.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::FindJob( const CORE::CString& jobId, CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    if ( jobId.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    if ( 0 == jobId.HasSubstr( IngestJobPrefix ) )
    {
        const CORE::CString ingestId = jobId.CutChars( IngestJobPrefix.Length(), true );
        for ( size_t i=0; i<m_ingests.size(); ++i )
            if ( m_ingests[ i ].ingestId == ingestId )
            {
                MakeIngestEntry( m_ingests[ i ], entry );
                return true;
            }
        return false;
    }
    if ( 0 == jobId.HasSubstr( ConversionJobPrefix ) )
    {
        const CORE::CString key = jobId.CutChars( ConversionJobPrefix.Length(), true );
        for ( size_t i=0; i<m_conversions.size(); ++i )
            if ( m_conversions[ i ].key == key )
            {
                MakeConversionEntry( m_conversions[ i ], entry );
                return true;
            }
        return false;
    }
    if ( 0 == jobId.HasSubstr( PipelineJobPrefix ) )
    {
        const CORE::CString key = jobId.CutChars( PipelineJobPrefix.Length(), true );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == key )
            {
                MakePipelineEntry( m_pipelineJobs[ i ], entry );
                return true;
            }
        return false;
    }
    CAssetStoreDownloadJob job;
    if ( GUCEF_NULL == m_manager || !m_manager->GetJob( jobId, job ) )
        return false;
    MakeDownloadEntry( job, entry );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NotifyJobChanged( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetJobEntry entry;
    if ( FindJob( jobId, entry ) )
        Raise( JobChangedEvent, entry );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NotifyJobRemoved( const CAssetJobEntry& entry )
{GUCEF_TRACE;

    Raise( JobRemovedEvent, entry );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ListJobs( const CORE::CString& pageToken, UInt32 maxEntries, CAssetJobPage& result )
{GUCEF_TRACE;

    result.Clear();
    TAssetJobEntryVector jobs;
    GetAllJobs( jobs );
    FillPage( jobs, pageToken, maxEntries, result );
    result.revision = GetCatalogueRevision();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetJob( const CORE::CString& jobId, CAssetJobEntry& result )
{GUCEF_TRACE;

    result.Clear();
    return FindJob( jobId, result ) ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ControlJob( const CORE::CString& commandId, const CORE::CString& jobId, TAssetJobAction action, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::ControlJob, result ) )
        return result.status;
    result.id = jobId;
    result.status = ASSETSTORE_STATUS_NOT_FOUND;

    if ( 0 == jobId.HasSubstr( ConversionJobPrefix ) )
    {
        const CORE::CString key = jobId.CutChars( ConversionJobPrefix.Length(), true );
        CAssetJobEntry removed;
        bool wasRemoved = false;
        bool changed = false;
        CORE::CString retryPackId;
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<m_conversions.size(); ++i )
            {
                if ( m_conversions[ i ].key != key )
                    continue;
                SConversion& conversion = m_conversions[ i ];
                const bool running = ASSETJOB_STATE_RUNNING == conversion.state || ASSETJOB_STATE_QUEUED == conversion.state;
                result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
                if ( ASSETJOB_ACTION_CANCEL == action && running )
                {
                    // the task stops at the next asset; its events say how it ended
                    conversion.message = "Cancelling";
                    const bool asked = conversion.future.HasAFuture() && conversion.future.RequestCancellation();
                    result.status = asked ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_FAILED;
                    changed = true;
                }
                else
                if ( ASSETJOB_ACTION_REMOVE == action && !running )
                {
                    MakeConversionEntry( conversion, removed );
                    m_conversions.erase( m_conversions.begin() + i );
                    wasRemoved = true;
                    result.status = ASSETSTORE_STATUS_OK;
                }
                else
                if ( ASSETJOB_ACTION_RETRY == action && !running && IsCatalogueAvailable() )
                {
                    retryPackId = conversion.packId;
                    MakeConversionEntry( conversion, removed );
                    m_conversions.erase( m_conversions.begin() + i );
                    wasRemoved = true;
                }
                break;
            }
        }
        if ( changed )
            NotifyJobChanged( jobId );
        if ( wasRemoved )
            NotifyJobRemoved( removed );
        if ( !retryPackId.IsNULLOrEmpty() )
        {
            CORE::CString newJobId;
            result.status = StartConversionJob( retryPackId, newJobId );
            result.relatedId = newJobId;
        }
    }
    else
    if ( 0 == jobId.HasSubstr( IngestJobPrefix ) )
    {
        const CORE::CString ingestId = jobId.CutChars( IngestJobPrefix.Length(), true );
        CAssetJobEntry removed;
        bool wasRemoved = false;
        bool resume = false;
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<m_ingests.size(); ++i )
            {
                if ( m_ingests[ i ].ingestId != ingestId )
                    continue;
                result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
                if ( ASSETJOB_ACTION_REMOVE == action && ASSETJOB_STATE_RUNNING != m_ingests[ i ].state )
                {
                    MakeIngestEntry( m_ingests[ i ], removed );
                    m_ingests.erase( m_ingests.begin() + i );
                    wasRemoved = true;
                    result.status = ASSETSTORE_STATUS_OK;
                }
                else
                if ( ASSETJOB_ACTION_RETRY == action && IsCatalogueAvailable() && ASSETJOB_STATE_FAILED == m_ingests[ i ].state )
                {
                    m_ingests[ i ].state = ASSETJOB_STATE_RUNNING;
                    m_ingests[ i ].message.Clear();
                    resume = true;
                    result.status = ASSETSTORE_STATUS_OK;
                }
                break;
            }
        }
        if ( resume )
        {
            m_store->ResumeIngestAsync( ingestId );
            NotifyJobChanged( jobId );
        }
        if ( wasRemoved )
            NotifyJobRemoved( removed );
    }
    else
    if ( 0 == jobId.HasSubstr( PipelineJobPrefix ) )
    {
        const CORE::CString key = jobId.CutChars( PipelineJobPrefix.Length(), true );
        CAssetJobEntry removed;
        bool wasRemoved = false;
        bool changed = false;
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            {
                if ( m_pipelineJobs[ i ].key != key )
                    continue;
                SPipelineJob& job = m_pipelineJobs[ i ];
                result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
                if ( ASSETJOB_ACTION_CANCEL == action && !AssetJobStateIsFinal( job.state ) )
                {
                    job.message = "Cancelling";
                    result.status = job.source.GetFuture().RequestCancellation() ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_FAILED;
                    changed = true;
                }
                else
                if ( ASSETJOB_ACTION_REMOVE == action && AssetJobStateIsFinal( job.state ) )
                {
                    MakePipelineEntry( job, removed );
                    m_pipelineJobs.erase( m_pipelineJobs.begin() + i );
                    wasRemoved = true;
                    result.status = ASSETSTORE_STATUS_OK;
                }
                break;
            }
        }
        if ( changed )
            NotifyJobChanged( jobId );
        if ( wasRemoved )
            NotifyJobRemoved( removed );
    }
    else
    if ( GUCEF_NULL != m_manager )
    {
        switch ( action )
        {
            case ASSETJOB_ACTION_PAUSE:  result.status = m_manager->Pause( jobId ); break;
            case ASSETJOB_ACTION_RESUME: result.status = m_manager->Resume( jobId ); break;
            case ASSETJOB_ACTION_CANCEL: result.status = m_manager->Cancel( jobId ); break;
            case ASSETJOB_ACTION_RETRY:  result.status = m_manager->Retry( jobId ); break;
            case ASSETJOB_ACTION_REMOVE: result.status = m_manager->Remove( jobId ); break;
            default:                     result.status = ASSETSTORE_STATUS_UNSUPPORTED; break;
        }
    }
    if ( ASSETSTORE_STATUS_OK != result.status && result.message.IsNULLOrEmpty() )
        result.message = "The job " + jobId + " cannot be asked to " + AssetJobActionToString( action ) + " now";
    RememberCommand( commandId, CAssetServiceOperations::ControlJob, result );
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetJobResult( const CORE::CString& jobId, const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    CAssetJobEntry entry;
    if ( !FindJob( jobId, entry ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    result.jobId = jobId;
    CORE::CDataNodeSerializableSettings settings;
    result.document.SetName( "result" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( 0 == jobId.HasSubstr( PipelineJobPrefix ) )
    {
        // the result the converter gave, as it gave it
        MT::CScopeMutex lock( m_lock );
        const CORE::CString key = jobId.CutChars( PipelineJobPrefix.Length(), true );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == key )
            {
                result = m_pipelineJobs[ i ].result;
                result.jobId = jobId;
            }
        return ASSETSTORE_STATUS_OK;
    }
    entry.Serialize( *result.document.AddChild( "job", GUCEF_DATATYPE_OBJECT ), settings );
    if ( 0 == jobId.HasSubstr( ConversionJobPrefix ) )
    {
        CORE::CFutureResult future( CORE::TASKSTATUS_UNDEFINED );
        {
            MT::CScopeMutex lock( m_lock );
            const CORE::CString key = jobId.CutChars( ConversionJobPrefix.Length(), true );
            for ( size_t i=0; i<m_conversions.size(); ++i )
                if ( m_conversions[ i ].key == key )
                    future = m_conversions[ i ].future;
        }
        CAssetConversionOutcome outcome;
        if ( TryGetFutureResult( future, outcome ) )
        {
            CORE::CDataNode* summary = result.document.AddChild( "summary", GUCEF_DATATYPE_OBJECT );
            summary->SetAttribute( "status", CORE::CString( AssetStoreStatusToString( outcome.status ) ) );
            summary->SetAttribute( "state", CORE::CString( AssetPackStateToString( outcome.state ) ) );
            summary->SetAttribute( "message", outcome.message );
            summary->SetAttribute( "packId", outcome.packId );
            summary->SetValuesOfChildByName( "problems", outcome.problems );
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnDownloadJobEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL != data )
        NotifyJobChanged( data->job.jobId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnDownloadJobProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL == data )
        return;
    {
        MT::CScopeMutex lock( m_lock );
        SRate rate;
        rate.bytesPerSecond = data->bytesPerSecond;
        rate.secondsLeft = data->secondsLeft;
        m_rates[ data->job.jobId ] = rate;
    }
    NotifyJobChanged( data->job.jobId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnDownloadJobRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL == data )
        return;
    CAssetJobEntry entry;
    {
        MT::CScopeMutex lock( m_lock );
        MakeDownloadEntry( data->job, entry );
        m_rates.erase( data->job.jobId );
    }
    NotifyJobRemoved( entry );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnDownloadJobFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL == data )
        return;
    IngestFinishedDownload( data->job );
    NotifyJobChanged( data->job.jobId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::IngestFinishedDownload( const CAssetStoreDownloadJob& job )
{GUCEF_TRACE;

    SAddJob add;
    bool found = false;
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_addJobs.size(); ++i )
        {
            if ( m_addJobs[ i ].jobId == job.jobId && !m_addJobs[ i ].ingestRequested )
            {
                add = m_addJobs[ i ];
                found = true;
                // a source with the automatic policy is ingested by the store itself
                if ( ASSETINGEST_MODE_AUTOMATIC != m_config.primaryStore.GetPolicy( job.providerId ).mode )
                    m_addJobs[ i ].ingestRequested = true;
                break;
            }
        }
    }
    if ( !found || !IsCatalogueAvailable() || ASSETINGEST_MODE_AUTOMATIC == m_config.primaryStore.GetPolicy( job.providerId ).mode )
        return;
    CAssetIngestRequest request = CPrimaryAssetStore::MakeIngestRequest( job );
    if ( !add.keep.IsNULLOrEmpty() )
        request.keep = add.keep;
    request.locationId = add.locationId;
    m_store->IngestAsync( request );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CLocalAssetStoreService: The download " << job.jobId << " is done, its ingest started" );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NoteIngest( CORE::CICloneable* eventData, TAssetJobState state )
{GUCEF_TRACE;

    const CAssetIngestEventData* data = static_cast< const CAssetIngestEventData* >( eventData );
    if ( GUCEF_NULL == data || data->ingestId.IsNULLOrEmpty() )
        return;
    // the name of the pack when the store knows it by now, until then the start of the id of the ingest
    CORE::CString packTitle;
    CAssetPackRecord pack;
    if ( !data->packId.IsNULLOrEmpty() && IsCatalogueAvailable() && ASSETSTORE_STATUS_OK == m_store->GetPack( data->packId, pack ) && !pack.title.IsNULLOrEmpty() )
        packTitle = pack.title;
    {
        MT::CScopeMutex lock( m_lock );
        SIngest* ingest = GUCEF_NULL;
        for ( size_t i=0; i<m_ingests.size(); ++i )
            if ( m_ingests[ i ].ingestId == data->ingestId )
                ingest = &m_ingests[ i ];
        if ( GUCEF_NULL == ingest )
        {
            SIngest created;
            created.ingestId = data->ingestId;
            created.createdAtMs = NowMs();
            m_ingests.push_back( created );
            ingest = &m_ingests.back();
        }
        ingest->state = state;
        ingest->step = data->step;
        ingest->message = data->message;
        ingest->updatedAtMs = NowMs();
        if ( !data->packId.IsNULLOrEmpty() )
            ingest->packId = data->packId;
        const TAssetIngestStep step = AssetIngestStepFromString( data->step );
        ingest->stepIndex = (UInt32) step > 0 ? (UInt32) step - 1 : 0;
        if ( !packTitle.IsNULLOrEmpty() )
            ingest->name = packTitle;
        if ( ingest->name.IsNULLOrEmpty() )
            ingest->name = "Ingest " + data->ingestId.SubstrToIndex( data->ingestId.Length() > 8 ? 8 : data->ingestId.Length(), true );
    }
    NotifyJobChanged( IngestJobPrefix + data->ingestId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnIngestProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteIngest( eventData, ASSETJOB_STATE_RUNNING );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnIngestFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteIngest( eventData, ASSETJOB_STATE_DONE );
    const CAssetIngestEventData* data = static_cast< const CAssetIngestEventData* >( eventData );
    // a duplicate adds nothing to the catalogue
    if ( GUCEF_NULL != data && !data->packId.IsNULLOrEmpty() && ASSETSTORE_STATUS_OK == AssetStoreStatusFromString( data->status ) )
        RecordChange( ASSETCHANGE_PACK_ADDED, data->packId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnIngestFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteIngest( eventData, ASSETJOB_STATE_FAILED );
}

/*-------------------------------------------------------------------------*/

CLocalAssetStoreService::SConversion*
CLocalAssetStoreService::FindConversionOfEventLocked( const CORE::CString& jobId, const CORE::CString& packId )
{GUCEF_TRACE;

    // the job of the event, else the conversion that was started for the pack and has not seen an event yet
    for ( size_t i=0; i<m_conversions.size(); ++i )
        if ( !jobId.IsNULLOrEmpty() && m_conversions[ i ].jobId == jobId )
            return &m_conversions[ i ];
    for ( size_t i=m_conversions.size(); i>0; --i )
    {
        SConversion& conversion = m_conversions[ i - 1 ];
        if ( conversion.jobId.IsNULLOrEmpty() && conversion.packId == packId && !AssetJobStateIsFinal( conversion.state ) )
        {
            conversion.jobId = jobId;
            return &conversion;
        }
    }
    SConversion created;
    created.key = "cv" + CORE::ToString( ++m_conversionCounter );
    created.jobId = jobId;
    created.packId = packId;
    created.createdAtMs = NowMs();
    m_conversions.push_back( created );
    return &m_conversions.back();
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NoteConversion( CORE::CICloneable* eventData, TAssetJobState state )
{GUCEF_TRACE;

    const CAssetConversionEventData* data = static_cast< const CAssetConversionEventData* >( eventData );
    if ( GUCEF_NULL == data || data->jobId.IsNULLOrEmpty() )
        return;
    if ( data->forIngest )
    {
        // the conversion step of an ingest: what the ingest says of itself
        bool changed = false;
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<m_ingests.size(); ++i )
            {
                if ( m_ingests[ i ].ingestId == data->jobId && ASSETJOB_STATE_RUNNING == m_ingests[ i ].state )
                {
                    m_ingests[ i ].step = "convert";
                    m_ingests[ i ].message = data->message;
                    changed = true;
                }
            }
        }
        if ( changed )
            NotifyJobChanged( IngestJobPrefix + data->jobId );
        return;
    }
    const CORE::CString packName = GetPackName( data->packId );
    CORE::CString key;
    {
        MT::CScopeMutex lock( m_lock );
        SConversion* conversion = FindConversionOfEventLocked( data->jobId, data->packId );
        if ( conversion->name.IsNULLOrEmpty() )
            conversion->name = packName;
        conversion->state = state;
        if ( data->total > 0 )
        {
            conversion->done = data->done;
            conversion->total = data->total;
        }
        const TAssetStoreStatus status = AssetStoreStatusFromString( data->status );
        conversion->status = status;
        if ( ASSETJOB_STATE_FAILED == state && ASSETSTORE_STATUS_CANCELLED == status )
            conversion->state = ASSETJOB_STATE_CANCELLED;
        conversion->message = data->message;
        if ( ASSETJOB_STATE_DONE == state && ASSETSTORE_STATUS_REFUSED == status )
            conversion->message = "Refused: " + data->message;
        conversion->updatedAtMs = NowMs();
        key = conversion->key;
    }
    NotifyJobChanged( ConversionJobPrefix + key );
    if ( ASSETJOB_STATE_DONE == state )
        RecordChange( ASSETCHANGE_PACK_UPDATED, data->packId );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnConversionProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteConversion( eventData, ASSETJOB_STATE_RUNNING );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnConversionFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteConversion( eventData, ASSETJOB_STATE_DONE );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnConversionFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NoteConversion( eventData, ASSETJOB_STATE_FAILED );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::StartConversionJob( const CORE::CString& packId, CORE::CString& jobId )
{GUCEF_TRACE;

    SConversion conversion;
    conversion.packId = packId;
    conversion.name = GetPackName( packId );
    conversion.state = ASSETJOB_STATE_QUEUED;
    conversion.message = "Waiting for its turn";
    conversion.createdAtMs = NowMs();
    conversion.updatedAtMs = conversion.createdAtMs;
    {
        MT::CScopeMutex lock( m_lock );
        conversion.key = "cv" + CORE::ToString( ++m_conversionCounter );
        // registered before the task runs, so its first event finds it
        m_conversions.push_back( conversion );
    }
    CORE::CFutureResult future = m_store->ConvertPackAsync( packId );
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_conversions.size(); ++i )
        {
            if ( m_conversions[ i ].key != conversion.key )
                continue;
            if ( future.HasNoFuture() )
                m_conversions.erase( m_conversions.begin() + i );
            else
                m_conversions[ i ].future = future;
            break;
        }
    }
    if ( future.HasNoFuture() )
        return ASSETSTORE_STATUS_FAILED;
    jobId = ConversionJobPrefix + conversion.key;
    NotifyJobChanged( jobId );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RequestConversion( const CORE::CString& commandId, const CAssetConversionJobRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::RequestConversion, result ) )
        return result.status;
    CORE::CString message;
    TAssetStoreStatus status = NeedCatalogue( message );
    if ( ASSETSTORE_STATUS_OK != status )
        return Refuse( status, message, result );
    if ( GUCEF_NULL == GetConverterHook() )
        return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The store has no converter", result );
    if ( !request.assetIds.empty() || !request.cookProfile.IsNULLOrEmpty() || !request.keep.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The conversion of a part of a pack, with another profile or another keep policy is not available in this version", result );
    if ( request.allPending )
    {
        CORE::CFutureResult future = m_store->ConvertPendingPacksAsync();
        if ( future.HasNoFuture() )
            return Refuse( ASSETSTORE_STATUS_FAILED, "The conversion could not be started", result );
        MT::CScopeMutex lock( m_lock );
        m_pendingConversions = future;
        result.status = ASSETSTORE_STATUS_OK;
    }
    else
    {
        if ( request.packId.IsNULLOrEmpty() )
            return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "No pack is given", result );
        result.status = StartConversionJob( request.packId, result.id );
        if ( ASSETSTORE_STATUS_OK != result.status )
            result.message = "The conversion could not be started";
        result.relatedId = request.packId;
    }
    RememberCommand( commandId, CAssetServiceOperations::RequestConversion, result );
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RunPipelineQuery( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    if ( CAssetPipelineOperations::IsRender( request.operation ) )
        return RenderSheet( request, result );
    if ( CAssetPipelineOperations::IsJob( request.operation ) )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "The operation " + request.operation + " is a job: it is started with StartPipelineJob()";
        return result.status;
    }
    if ( CAssetPipelineOperations::IsWrite( request.operation ) && request.apply )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "A write is applied with ApplyPipelineChange()";
        return result.status;
    }
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "The store has no converter";
        return result.status;
    }
    return hook->RunPipelineOperation( request, GUCEF_NULL, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ApplyPipelineChange( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    CAssetServiceResult replay;
    if ( ReplayCommand( commandId, CAssetServiceOperations::ApplyPipelineChange, replay ) )
    {
        result.status = replay.status;
        result.message = replay.message;
        return result.status;
    }
    if ( !CAssetPipelineOperations::IsWrite( request.operation ) )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "The operation " + request.operation + " writes nothing";
        return result.status;
    }
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "The store has no converter";
        return result.status;
    }
    CAssetPipelineRequest apply = request;
    apply.apply = true;
    const TAssetStoreStatus status = hook->RunPipelineOperation( apply, GUCEF_NULL, result );
    CAssetServiceResult outcome;
    outcome.status = status;
    outcome.message = result.message;
    RememberCommand( commandId, CAssetServiceOperations::ApplyPipelineChange, outcome );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::StartPipelineJob( const CORE::CString& commandId, const CAssetPipelineRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::StartPipelineJob, result ) )
        return result.status;
    if ( !CAssetPipelineOperations::IsJob( request.operation ) )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The operation " + request.operation + " is not a job", result );
    CIAssetConverterHook* hook = GetConverterHook();
    if ( GUCEF_NULL == hook )
        return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The store has no converter", result );
    if ( !hook->SupportsPipelineOperation( request.operation ) )
        return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The converter " + hook->GetName() + " does not carry out the operation \"" + request.operation + "\"", result );

    SPipelineJob job;
    job.request = request;
    job.name = request.packId.IsNULLOrEmpty() ? request.sourcePath : request.packId;
    job.message = "Waiting for its turn";
    job.createdAtMs = NowMs();
    job.updatedAtMs = job.createdAtMs;
    {
        MT::CScopeMutex lock( m_lock );
        job.key = "pj" + CORE::ToString( ++m_pipelineCounter );
        m_pipelineJobs.push_back( job );
    }
    result.id = PipelineJobPrefix + job.key;
    CAssetServiceCall call( CAssetServiceOperations::StartPipelineJob, commandId );
    CORE::CFutureResult future = QueueCall( call, job.key );
    if ( future.HasNoFuture() )
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == job.key )
            {
                m_pipelineJobs[ i ].state = ASSETJOB_STATE_FAILED;
                m_pipelineJobs[ i ].status = ASSETSTORE_STATUS_FAILED;
                m_pipelineJobs[ i ].message = "The job could not be queued";
            }
        result.status = ASSETSTORE_STATUS_FAILED;
        result.message = "The job could not be queued";
    }
    else
        result.status = ASSETSTORE_STATUS_OK;
    NotifyJobChanged( result.id );
    RememberCommand( commandId, CAssetServiceOperations::StartPipelineJob, result );
    return result.status;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::RunPipelineJob( const CORE::CString& key )
{GUCEF_TRACE;

    CAssetPipelineRequest request;
    CORE::CFutureResultSource source( CORE::TASKSTATUS_QUEUED );
    bool found = false;
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == key )
            {
                request = m_pipelineJobs[ i ].request;
                source = m_pipelineJobs[ i ].source;
                m_pipelineJobs[ i ].state = ASSETJOB_STATE_RUNNING;
                m_pipelineJobs[ i ].message.Clear();
                found = true;
            }
    }
    if ( !found )
        return;
    NotifyJobChanged( PipelineJobPrefix + key );
    CAssetServiceDocument document;
    TAssetStoreStatus status = ASSETSTORE_STATUS_UNSUPPORTED;
    CIAssetConverterHook* hook = GetConverterHook();
    CPipelineJobObserver observer( source, this, &CLocalAssetStoreService::NotePipelineProgress, key );
    if ( GUCEF_NULL != hook )
    {
        try
        {
            status = hook->RunPipelineOperation( request, &observer, document );
        }
        catch ( ... )
        {
            status = ASSETSTORE_STATUS_FAILED;
            document.message = "An exception ended the job";
        }
    }
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
        {
            if ( m_pipelineJobs[ i ].key != key )
                continue;
            SPipelineJob& job = m_pipelineJobs[ i ];
            job.status = status;
            job.result = document;
            job.result.status = status;
            job.message = document.message;
            job.state = ASSETSTORE_STATUS_OK == status ? ASSETJOB_STATE_DONE : ( ASSETSTORE_STATUS_CANCELLED == status ? ASSETJOB_STATE_CANCELLED : ASSETJOB_STATE_FAILED );
            job.updatedAtMs = NowMs();
        }
    }
    CAssetServiceCall call( CAssetServiceOperations::StartPipelineJob );
    CompleteCall( source, call, status, document.Clone(), document.message );
    NotifyJobChanged( PipelineJobPrefix + key );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NotePipelineProgress( const CORE::CString& key, UInt32 done, UInt32 total, const CORE::CString& assetPath )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pipelineJobs.size(); ++i )
            if ( m_pipelineJobs[ i ].key == key )
            {
                m_pipelineJobs[ i ].done = done;
                m_pipelineJobs[ i ].total = total;
                m_pipelineJobs[ i ].message = assetPath;
            }
    }
    NotifyJobChanged( PipelineJobPrefix + key );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RenderSheet( const CAssetPipelineRequest& request, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    CIAssetRenderHook* hook = GetRenderHook();
    if ( GUCEF_NULL == hook )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "No renderer is given to the service";
        return result.status;
    }
    return hook->Render( request, result );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::MakeStoreInfo( const TAssetStoreProviderPtr& provider, CAssetStoreInfo& info ) const
{GUCEF_TRACE;

    info.Clear();
    info.storeId = provider->GetProviderId();
    info.displayName = provider->GetDisplayName();
    info.iconName = provider->GetIconName();
    info.homePageUrl = provider->GetHomePageUrl();
    info.signInKind = provider->GetSignInKind();
    info.signedIn = provider->IsSignedIn();
    MT::CScopeMutex lock( m_lock );
    // what the store told of the end of its session holds until it is signed in again
    info.sessionEnded = m_endedSessions.find( info.storeId ) != m_endedSessions.end();
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::WatchProviders( void )
{GUCEF_TRACE;

    CAssetStoreRegistry& registry = CAssetStoreGlobal::Instance()->GetRegistry();
    CORE::CString::StringVector ids;
    registry.GetProviderIds( ids );
    for ( size_t i=0; i<ids.size(); ++i )
    {
        TAssetStoreProviderPtr provider;
        if ( !registry.TryGetProvider( ids[ i ], provider ) || provider.IsNULL() )
            continue;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_watchedProviders.find( ids[ i ] ) != m_watchedProviders.end() )
                continue;
            m_watchedProviders.insert( ids[ i ] );
        }
        // the sign in state follows the session of the provider
        TEventCallback callback( this, &CLocalAssetStoreService::OnSessionStateChanged );
        SubscribeTo( provider.GetPointerAlways(), CIAssetStoreProvider::SessionStateChangedEvent, callback );
        // a store that signs in on its web site tells when the session ended, as apart from a sign out
        CWebSessionSignIn* webSignIn = ASSETSTORE_SIGNIN_WEB_SESSION == provider->GetSignInKind() ? provider->GetWebSessionSignIn() : GUCEF_NULL;
        if ( GUCEF_NULL != webSignIn )
        {
            TEventCallback endedCallback( this, &CLocalAssetStoreService::OnWebSessionEnded );
            SubscribeTo( webSignIn, CWebSessionSignIn::SessionEndedEvent, endedCallback );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::NotifyStoreState( const CORE::CString& storeId )
{GUCEF_TRACE;

    TAssetStoreProviderPtr provider;
    if ( !CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId, provider ) || provider.IsNULL() )
        return;
    CAssetStoreInfo info;
    MakeStoreInfo( provider, info );
    Raise( StoreStateChangedEvent, info );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnSessionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == storeId )
        return;
    TAssetStoreProviderPtr provider;
    if ( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId->GetData(), provider ) && !provider.IsNULL() && provider->IsSignedIn() )
    {
        // a sign in ends what is known of the end of the session
        MT::CScopeMutex lock( m_lock );
        m_endedSessions.erase( storeId->GetData() );
    }
    NotifyStoreState( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnWebSessionEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == storeId )
        return;
    {
        MT::CScopeMutex lock( m_lock );
        m_endedSessions.insert( storeId->GetData() );
    }
    NotifyStoreState( storeId->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnProviderAdded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    WatchProviders();
    NotifyObservers( StoresChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnProviderRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != storeId )
    {
        MT::CScopeMutex lock( m_lock );
        m_watchedProviders.erase( storeId->GetData() );
    }
    NotifyObservers( StoresChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnLocationStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* locationId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == locationId )
        return;
    TAssetLocationInfoVector locations;
    GetLocationInfos( locations );
    for ( size_t i=0; i<locations.size(); ++i )
        if ( locations[ i ].location.locationId == locationId->GetData() )
            Raise( LocationStateChangedEvent, locations[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::OnImageReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreImageCache::TImageReadyEventData* ready = static_cast< const CAssetStoreImageCache::TImageReadyEventData* >( eventData );
    if ( GUCEF_NULL == ready )
        return;
    const CAssetStoreImageCache::SImageReadyData& data = ready->GetData();
    CAssetThumbnail image;
    image.url = data.url;
    image.size = data.maxSize;
    image.kind = "listing";
    image.pixels = data.pixels;
    image.status = data.status;
    image.message = data.message;
    Raise( ListingImageReadyEvent, image );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ListStores( CAssetStoreInfoPage& result )
{GUCEF_TRACE;

    result.Clear();
    WatchProviders();
    CAssetStoreRegistry& registry = CAssetStoreGlobal::Instance()->GetRegistry();
    CORE::CString::StringVector ids;
    registry.GetProviderIds( ids );
    for ( size_t i=0; i<ids.size(); ++i )
    {
        // the primary store is the catalogue of the service, not one of its online stores
        if ( CPrimaryAssetStore::ProviderId == ids[ i ] )
            continue;
        TAssetStoreProviderPtr provider;
        if ( !registry.TryGetProvider( ids[ i ], provider ) || provider.IsNULL() )
            continue;
        CAssetStoreInfo info;
        MakeStoreInfo( provider, info );
        result.entries.push_back( info );
    }
    result.total = result.entries.size();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::GetStoreProvider( const CORE::CString& storeId, TAssetStoreProviderPtr& provider )
{GUCEF_TRACE;

    return CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId, provider ) && !provider.IsNULL();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::QueryLibrary( const CORE::CString& storeId, const CAssetStoreQuery& query, CAssetStoreLibraryPage& result )
{GUCEF_TRACE;

    TAssetStoreProviderPtr provider;
    if ( !GetStoreProvider( storeId, provider ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    return provider->QueryLibrary( query, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetStoreItem( const CORE::CString& storeId, const CORE::CString& itemId, CAssetStoreItem& result )
{GUCEF_TRACE;

    TAssetStoreProviderPtr provider;
    if ( !GetStoreProvider( storeId, provider ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    return provider->GetItem( itemId, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::EnqueueDownload( const CORE::CString& commandId, const CAssetDownloadRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::EnqueueDownload, result ) )
        return result.status;
    if ( GUCEF_NULL == m_manager )
        return Refuse( ASSETSTORE_STATUS_UNAVAILABLE, "The asset store service is not open", result );
    if ( request.formatId.IsNULLOrEmpty() )
        return Refuse( ASSETSTORE_STATUS_INVALID_ARGUMENT, "No format is given", result );
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    if ( request.forIngest )
    {
        CORE::CString message;
        status = NeedCatalogue( message );
        if ( ASSETSTORE_STATUS_OK != status )
            return Refuse( status, message, result );
        status = m_store->EnqueueDownload( *m_manager, request.providerId, request.itemId, request.formatId, request.fileIds, result.id );
        if ( ASSETSTORE_STATUS_OK == status )
        {
            SAddJob add;
            add.jobId = result.id;
            add.keep = request.keep;
            add.locationId = request.locationId;
            add.ingestRequested = false;
            MT::CScopeMutex lock( m_lock );
            m_addJobs.push_back( add );
        }
    }
    else
    {
        status = m_manager->Enqueue( request.providerId, request.itemId, request.formatId, request.fileIds, request.destination, false, result.id, request.itemTitle );
    }
    result.status = status;
    if ( ASSETSTORE_STATUS_OK != status )
        result.message = "The download of " + ( request.itemTitle.IsNULLOrEmpty() ? request.itemId : request.itemTitle ) + " cannot be queued";
    RememberCommand( commandId, CAssetServiceOperations::EnqueueDownload, result );
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::TryGetListingImage( const CORE::CString& url, UInt32 size, CAssetThumbnail& image )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imageCache || url.IsNULLOrEmpty() )
        return false;
    IMAGE::TPixelMapPtr pixels;
    if ( !m_imageCache->TryGetImage( url, size, pixels ) )
        return false;
    image.Clear();
    image.url = url;
    image.size = size;
    image.kind = "listing";
    image.pixels = pixels;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalAssetStoreService::RequestListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imageCache || url.IsNULLOrEmpty() )
        return false;
    m_imageCache->RequestImage( url, size );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalAssetStoreService::CancelListingImage( const CORE::CString& url, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_imageCache )
        m_imageCache->CancelRequest( url, size );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SignOut( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( ReplayCommand( commandId, CAssetServiceOperations::SignOut, result ) )
        return result.status;
    TAssetStoreProviderPtr provider;
    if ( !GetStoreProvider( storeId, provider ) )
        return Refuse( ASSETSTORE_STATUS_NOT_FOUND, "The store " + storeId + " is not known", result );
    result.status = provider->SignOut() ? ASSETSTORE_STATUS_OK : ASSETSTORE_STATUS_FAILED;
    result.id = storeId;
    RememberCommand( commandId, CAssetServiceOperations::SignOut, result );
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetSignInPrompts( CAssetSignInPromptPage& result )
{GUCEF_TRACE;

    // the application of this process signs in to its stores itself: the service has no prompts of its own
    result.Clear();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetOfflinePin( const CORE::CString& commandId, const CORE::CString& packId, bool pinned, CAssetServiceResult& result )
{GUCEF_TRACE;

    // the files of a store in this process are always at hand
    result.Clear();
    result.id = packId;
    result.message = "The files of the store of this process are always available";
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetResourceStatus( CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    result.document.SetName( "resources" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    TAssetJobEntryVector jobs;
    GetAllJobs( jobs );
    UInt32 running = 0;
    for ( size_t i=0; i<jobs.size(); ++i )
        if ( !jobs[ i ].IsEnded() )
            ++running;
    result.document.SetAttribute( "pendingCalls", m_pendingCalls.Load() );
    result.document.SetAttribute( "jobs", (UInt32) jobs.size() );
    result.document.SetAttribute( "jobsNotEnded", running );
    result.document.SetAttribute( "catalogueRevision", GetCatalogueRevision() );
    {
        MT::CScopeMutex lock( m_lock );
        result.document.SetAttribute( "journalEntries", (UInt32) m_journal.size() );
        result.document.SetAttribute( "commandsKept", (UInt32) m_commands.size() );
    }
    if ( GUCEF_NULL != m_imageCache )
    {
        result.document.SetAttribute( "listingImagesInMemory", m_imageCache->GetMemoryEntryCount() );
        result.document.SetAttribute( "listingImageBytes", m_imageCache->GetMemoryBytes() );
    }
    result.document.SetAttribute( "catalogueAvailable", IsCatalogueAvailable() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::UpdatePackMetadata( const CORE::CString& commandId, const CAssetPackRecord& pack, UInt64 expectedRevision, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The edit of the metadata of a pack is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::SetClassification( const CORE::CString& commandId, const CORE::CString& objectId, const CAssetClassification& classification, const CORE::CString& source, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The classification of a single pack or asset is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RemoveCategoryMapping( const CORE::CString& commandId, const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The removal of a category mapping is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RemoveCategoryRule( const CORE::CString& commandId, const CORE::CString& ruleId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The removal of a category rule is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::Reclassify( const CORE::CString& commandId, const CORE::CString::StringVector& packIds, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A reclassification of packs is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::PutThumbnail( const CORE::CString& commandId, const CAssetThumbnail& thumbnail, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The thumbnail store of the catalogue is not available in this version: the thumbnails of the catalogue are rendered by the application from the files", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::BeginUpload( const CORE::CString& commandId, const CORE::CString& name, UInt64 totalBytes, UInt32 fileCount, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "An upload is a function of a remote service: in this process the files are ingested from where they are", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::WriteUploadBlock( const CORE::CString& uploadId, const CORE::CString& relativePath, UInt64 offset, const CORE::CDynamicBuffer& data, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "An upload is a function of a remote service: in this process the files are ingested from where they are", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::CommitUpload( const CORE::CString& commandId, const CORE::CString& uploadId, const CAssetIngestRequest& ingest, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "An upload is a function of a remote service: in this process the files are ingested from where they are", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::AbortUpload( const CORE::CString& commandId, const CORE::CString& uploadId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "An upload is a function of a remote service: in this process the files are ingested from where they are", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::BeginSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A sign in through the service is a function of a remote service: the application of this process signs in to its stores itself", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::CancelSignIn( const CORE::CString& commandId, const CORE::CString& storeId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A sign in through the service is a function of a remote service: the application of this process signs in to its stores itself", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::AnswerPrompt( const CORE::CString& commandId, const CORE::CString& promptId, const AUTH::CSecretString& answer, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A sign in through the service is a function of a remote service: the application of this process signs in to its stores itself", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RequestExport( const CORE::CString& commandId, const CORE::CString::StringVector& assetIds, const CORE::CString& profile, const CORE::CString& format, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "An export is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::RequestLocalCopy( const CORE::CString& commandId, const CAssetLocalCopyRequest& request, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A local copy is a function of a remote service: the files of the store of this process are local already", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::LockKeyring( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The keyring of the saved store passwords belongs to the application in this process", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::UnlockKeyring( const CORE::CString& commandId, const AUTH::CSecretString& code, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "The keyring of the saved store passwords belongs to the application in this process", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::ReindexFromStorage( const CORE::CString& commandId, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Refuse( ASSETSTORE_STATUS_UNSUPPORTED, "A reindex from storage is not available in the store of this process in this version", result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::CountRuleMatches( const CAssetCategoryRule& rule, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    result.status = ASSETSTORE_STATUS_UNSUPPORTED;
    result.message = "The preview of a category rule is not available in the store of this process in this version";
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetAuditRecords( const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result )
{GUCEF_TRACE;

    result.Clear();
    result.status = ASSETSTORE_STATUS_UNSUPPORTED;
    result.message = "The audit is a function of a remote service";
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetThumbnails( const CORE::CString::StringVector& assetIds, UInt32 size, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    // the thumbnail store of the catalogue is not available in this version
    result.Clear();
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::GetMissingThumbnails( const CORE::CString& rendererId, const CORE::CString& rendererVersion, UInt32 size, UInt32 maxEntries, CAssetThumbnailPage& result )
{GUCEF_TRACE;

    // the thumbnail store of the catalogue is not available in this version
    result.Clear();
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CLocalAssetStoreService::QueryUpload( const CORE::CString& uploadId, CAssetFileStatPage& result )
{GUCEF_TRACE;

    // uploads are a function of a remote service
    result.Clear();
    return ASSETSTORE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
