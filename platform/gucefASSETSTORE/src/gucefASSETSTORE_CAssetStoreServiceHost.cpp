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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#include "gucefASSETSTORE_CAssetStoreRemoteContracts.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECONTRACTS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#include "gucefASSETSTORE_CAssetStoreRemoteCodec.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREMOTECODEC_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#include "gucefASSETSTORE_CAssetStoreServiceHost.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Keeps the drain open while a call of the host runs */
class CDrainHoldScope
{
    public:

    explicit CDrainHoldScope( DRNUTILS::CRemoteServiceHost& host ) : m_host( host ) { m_host.AddDrainHold(); }
    ~CDrainHoldScope() { m_host.ReleaseDrainHold(); }

    private:

    CDrainHoldScope( const CDrainHoldScope& );
    CDrainHoldScope& operator=( const CDrainHoldScope& );
    DRNUTILS::CRemoteServiceHost& m_host;
};

/** Counts a call that runs */
class CRunningScope
{
    public:

    explicit CRunningScope( MT::CTAtomic< UInt32 >& running ) : m_running( running ) { ++m_running; }
    ~CRunningScope() { m_running.FetchAdd( (UInt32) -1 ); }

    private:

    CRunningScope( const CRunningScope& );
    CRunningScope& operator=( const CRunningScope& );
    MT::CTAtomic< UInt32 >& m_running;
};

/** The length of the part of a file that arrived from its start: where an interrupted upload continues */
UInt64
ContiguousFromStart( const DRNUTILS::CUploadRangeSet& ranges )
{GUCEF_TRACE;

    const DRNUTILS::CUploadRangeSet::TRangeVector& held = ranges.GetRanges();
    if ( held.empty() || 0 != held[ 0 ].offset )
        return 0;
    return held[ 0 ].length;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreServiceHost::SJobThrottle::SJobThrottle( void )
    : state( ASSETJOB_STATE_QUEUED )
    , lastSentMs( 0 )
    , pending( false )
    , entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreServiceHost::CAssetStoreServiceHost( DRNUTILS::CRemoteServiceHost& host, const TAssetStoreServicePtr& service, const CAssetStoreServiceHostSettings& settings )
    : CORE::CObservingNotifier()
    , CORE::CIServiceResourceSource()
    , m_host( host )
    , m_service( service )
    , m_fileGate( const_cast< CIAssetStoreService* >( service.GetPointerAlways() ) )
    , m_settings( settings )
    , m_credentials( GUCEF_NULL )
    , m_serviceImpl()
    , m_filesImpl()
    , m_commands()
    , m_journal()
    , m_staging()
    , m_stagingMountName()
    , m_uploadIndexes()
    , m_jobs()
    , m_inFlight()
    , m_timer( host.GetPulseGenerator(), 100 )
    , m_pulses( 0 )
    , m_running( 0 )
    , m_calls( 0 )
    , m_commandCount( 0 )
    , m_replays( 0 )
    , m_refusals( 0 )
    , m_bytesRead( 0 )
    , m_bytesWritten( 0 )
    , m_eventsPublished( 0 )
    , m_lastExpiryMs( 0 )
    , m_hasJournalKey( false )
    , m_attached( false )
    , m_journalOpen( false )
    , m_stagingOpen( false )
    , m_lock()
{GUCEF_TRACE;

    m_serviceImpl.BindMethod( "Query", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnQuery ) );
    m_serviceImpl.BindMethod( "Command", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnCommand ) );
    m_serviceImpl.BindMethod( "Secret", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnSecret ) );
    m_filesImpl.BindMethod( "Query", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnFilesQuery ) );
    m_filesImpl.BindMethod( "Command", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnFilesCommand ) );
    m_filesImpl.BindMethod( "ReadBlock", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnReadBlock ) );
    m_filesImpl.BindMethod( "WriteBlock", GUCEF_NEW THandler( this, &CAssetStoreServiceHost::OnWriteBlock ) );
    // a mount name of its own per binding (several hosts in one process, tests)
    m_stagingMountName = "AssetStoreServiceStaging" + CORE::ToString( (UInt64) (size_t) this );
}

/*-------------------------------------------------------------------------*/

CAssetStoreServiceHost::~CAssetStoreServiceHost()
{GUCEF_TRACE;

    Detach();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::SetCredentialStore( AUTH::ICredentialStore* store )
{GUCEF_TRACE;

    m_credentials = store;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::SetJournalKey( const UInt8* key, UInt32 size )
{GUCEF_TRACE;

    m_journal.SetKey( key, size );
    m_hasJournalKey = true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::Attach( CORE::CString& error )
{GUCEF_TRACE;

    if ( m_attached )
        return true;
    if ( m_service.IsNULL() )
    {
        error = "no asset store service to serve";
        return false;
    }
    COMCORE::CRpcContractPtr control = CAssetStoreRemoteContracts::GetControlContract();
    COMCORE::CRpcContractPtr files = CAssetStoreRemoteContracts::GetFilesContract();
    if ( control.IsNULL() || files.IsNULL() )
    {
        error = "the contracts of the asset store service cannot be built";
        return false;
    }

    // the command journal: a repeat after a restart is answered from it
    if ( !m_settings.commandJournalDirectory.IsNULLOrEmpty() )
    {
        if ( !m_hasJournalKey && GUCEF_NULL != m_credentials )
        {
            CORE::CString problem;
            if ( !m_journal.UseKeyOfStore( *m_credentials, m_host.GetSettings().serviceName + DRNUTILS::CRemoteServiceHost::KeyProviderSuffix, problem ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHost: the key of the command journal is not kept (" << problem
                                   << "): repeats after a restart are not recognised" );
        }
        m_journal.SetRetention( m_settings.commandJournalKeepDays, 0, 0 );
        if ( !m_journal.Open( m_settings.commandJournalDirectory, error ) )
        {
            error = "the command journal cannot be opened: " + error;
            return false;
        }
        m_journalOpen = true;
    }

    // the staging of uploads, mounted so the ingest of a committed upload reads it through the VFS
    if ( !m_settings.stagingDirectory.IsNULLOrEmpty() )
    {
        DRNUTILS::CUploadStaging::SSettings staging;
        staging.directory = m_settings.stagingDirectory;
        staging.maxBytesPerSubject = m_settings.maxUploadBytesPerSubject;
        staging.maxTotalBytes = m_settings.maxUploadBytesTotal;
        staging.maxUploadsPerSubject = m_settings.maxUploadsPerSubject;
        staging.expiryMs = (UInt64) m_settings.uploadExpiryHours * 3600000ULL;
        staging.blockSize = CAssetStoreRemoteContracts::BlockSize;
        CORE::CreateDirs( m_settings.stagingDirectory );
        if ( !m_staging.Open( staging, error ) )
        {
            error = "the upload staging cannot be opened: " + error;
            return false;
        }
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        vfs.UnmountArchiveByName( m_stagingMountName );
        if ( !vfs.AddRoot( m_settings.stagingDirectory, m_stagingMountName, false, false, m_settings.stagingVfsRoot ) )
        {
            m_staging.Close();
            error = "the upload staging cannot be mounted at " + m_settings.stagingVfsRoot;
            return false;
        }
        m_stagingOpen = true;
    }

    if ( !m_host.RegisterService( control, "Service", &m_serviceImpl, COMCORE::RPC_EXECUTION_THREAD_POOL ) ||
         !m_host.RegisterService( files, "Files", &m_filesImpl, COMCORE::RPC_EXECUTION_THREAD_POOL ) )
    {
        error = "the services of the asset store cannot be registered on the host (is it started?)";
        Detach();
        return false;
    }
    CORE::CString rolesCsv;
    for ( CORE::CString::StringSet::const_iterator i = m_settings.roles.begin(); i != m_settings.roles.end(); ++i )
        rolesCsv += ( rolesCsv.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "," ) ) + (*i);
    m_host.GetAccessRules().AddRule( CORE::CString( CAssetStoreRemoteContracts::ControlContractName ) + ".*", rolesCsv );
    m_host.GetAccessRules().AddRule( CORE::CString( CAssetStoreRemoteContracts::FilesContractName ) + ".*", rolesCsv );

    // the events of the service go to every client session
    CIAssetStoreService* service = m_service.GetPointerAlways();
    TEventCallback onEvent( this, &CAssetStoreServiceHost::OnServiceEvent );
    SubscribeTo( service, CIAssetStoreService::CatalogueChangedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::JobChangedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::JobRemovedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::LocationStateChangedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::StoreStateChangedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::StoresChangedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::SignInPromptEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::SignInPromptClosedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::ThumbnailUpdatedEvent, onEvent );
    SubscribeTo( service, CIAssetStoreService::NoticeEvent, onEvent );
    CORE::PulseGeneratorPtr pulse = m_host.GetPulseGenerator();
    if ( !pulse.IsNULL() )
    {
        TEventCallback onPulse( this, &CAssetStoreServiceHost::OnPulse );
        SubscribeTo( pulse.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, onPulse );
    }
    TEventCallback onTimer( this, &CAssetStoreServiceHost::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, onTimer );
    m_timer.SetEnabled( true );
    m_attached = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: serving the " << m_service->GetType() << " asset store service to the devices with the roles "
               << rolesCsv << ( m_stagingOpen ? ", uploads staged in " + m_settings.stagingDirectory : CORE::CString( ", no uploads" ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::Detach( void )
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    // the implementations of the services are members: no call may reach them once this binding goes, also not one
    // that arrived before and waits in the thread pool. Unregistering waits for the handlers under way
    COMCORE::CRpcServiceSetPtr services = m_host.GetServiceSet();
    if ( !services.IsNULL() )
    {
        services->UnregisterService( CAssetStoreRemoteContracts::GetControlContract(), "Service" );
        services->UnregisterService( CAssetStoreRemoteContracts::GetFilesContract(), "Files" );
    }
    // the calls that run use the journal and the staging
    for ( UInt32 waited=0; m_running.Load() > 0 && waited<30000; waited+=2 )
        MT::ThreadDelay( 2 );
    if ( m_stagingOpen )
    {
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_stagingMountName );
        m_staging.Close();
        m_stagingOpen = false;
    }
    if ( m_journalOpen )
    {
        m_journal.Close();
        m_journalOpen = false;
    }
    MT::CScopeMutex lock( m_lock );
    m_jobs.clear();
    m_uploadIndexes.clear();
    m_attached = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::IsAttached( void ) const
{GUCEF_TRACE;

    return m_attached;
}

/*-------------------------------------------------------------------------*/

const TAssetStoreServicePtr&
CAssetStoreServiceHost::GetService( void ) const
{GUCEF_TRACE;

    return m_service;
}

/*-------------------------------------------------------------------------*/

DRNUTILS::CRemoteServiceHost&
CAssetStoreServiceHost::GetHost( void )
{GUCEF_TRACE;

    return m_host;
}

/*-------------------------------------------------------------------------*/

const CAssetStoreServiceHostSettings&
CAssetStoreServiceHost::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreServiceHost::GetRunningCallCount( void ) const
{GUCEF_TRACE;

    return m_running.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreServiceHost::NowMs( void )
{GUCEF_TRACE;

    return DRNUTILS::CRemoteServiceHost::NowMs();
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnQuery( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    return HandleCall( call, false, false );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnCommand( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    return HandleCall( call, true, false );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnFilesQuery( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    return HandleCall( call, false, true );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnFilesCommand( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    return HandleCall( call, true, true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::BeginCommand( const CORE::CString& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_inFlight.find( key ) != m_inFlight.end() )
        return false;
    m_inFlight.insert( key );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::EndCommand( const CORE::CString& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_inFlight.erase( key );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::AnswerKnownCommand( COMCORE::CRpcServerCall& call, const DRNUTILS::CRemoteServicePrincipal& principal,
                                            const CORE::CString& commandId, const CORE::CString& cacheHash, const CORE::CString& journalHash )
{GUCEF_TRACE;

    DRNUTILS::CCommandOutcomeCache::SOutcome kept;
    DRNUTILS::CCommandOutcomeCache::ELookup lookup = m_commands.Find( principal.deviceId, commandId, cacheHash, NowMs(), kept );
    if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == lookup )
    {
        ++m_replays;
        DRNUTILS::CCommandOutcomeCache::AnswerReplay( call, kept, CAssetStoreRemoteContracts::AnswerReplayedField );
        return true;
    }
    if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == lookup )
    {
        ++m_refusals;
        DRNUTILS::CCommandOutcomeCache::RefuseReused( call, commandId );
        return true;
    }
    if ( !m_journalOpen )
        return false;
    DRNUTILS::CCommandJournal::SEntry entry;
    lookup = m_journal.Lookup( principal.deviceId, commandId, journalHash, entry );
    if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REPLAY == lookup )
    {
        ++m_replays;
        COMCORE::CRpcMessage answer = call.NewResponse();
        answer.SetString( CAssetStoreRemoteContracts::AnswerStatusField, entry.outcome.GetAttributeValueOrChildValueByName( "status" ).AsString() );
        answer.SetString( CAssetStoreRemoteContracts::AnswerMessageField, entry.outcome.GetAttributeValueOrChildValueByName( "message" ).AsString() );
        const CORE::CDataNode* result = entry.outcome.FindChild( "result" );
        if ( GUCEF_NULL != result )
            answer.SetDataNode( CAssetStoreRemoteContracts::AnswerResultField, *result );
        answer.SetUInt64( CAssetStoreRemoteContracts::AnswerEventSeqField, GetNextEventSeq( principal ) );
        answer.SetBool( CAssetStoreRemoteContracts::AnswerReplayedField, true );
        call.Reply( answer );
        return true;
    }
    if ( DRNUTILS::CCommandOutcomeCache::COMMAND_REUSED == lookup )
    {
        ++m_refusals;
        DRNUTILS::CCommandOutcomeCache::RefuseReused( call, commandId );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::RememberCommand( const DRNUTILS::CRemoteServicePrincipal& principal, const CORE::CString& operation, const CORE::CString& commandId,
                                         const CORE::CString& cacheHash, const CORE::CString& journalHash, const COMCORE::CRpcMessage& answer,
                                         TAssetStoreStatus status, const CORE::CString& message, const CORE::CICloneable* result )
{GUCEF_TRACE;

    // refusals that changed nothing are not kept: a repeat may succeed
    if ( !CIAssetStoreService::IsOutcomeStatus( operation, status ) )
        return;
    DRNUTILS::CCommandOutcomeCache::SOutcome outcome;
    DRNUTILS::CCommandOutcomeCache::MakeOutcome( COMCORE::CRpcStatus::Ok, &answer, outcome );
    m_commands.Remember( principal.deviceId, commandId, cacheHash, NowMs(), outcome );
    if ( !m_journalOpen )
        return;
    DRNUTILS::CCommandJournal::SEntry entry;
    entry.atUtcMs = DRNUTILS::CRemoteServiceHost::NowUtcMs();
    entry.subject = principal.deviceId;
    entry.commandId = commandId;
    entry.requestHash = journalHash;
    entry.method = operation;
    entry.code = COMCORE::RPC_STATUS_OK;
    entry.outcome.SetName( "outcome" );
    entry.outcome.SetNodeType( GUCEF_DATATYPE_OBJECT );
    entry.outcome.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    entry.outcome.SetAttribute( "message", message );
    CORE::CDataNode resultNode;
    if ( answer.GetDataNode( CAssetStoreRemoteContracts::AnswerResultField, resultNode ) )
    {
        resultNode.SetName( "result" );
        entry.outcome.AddChild( resultNode );
    }
    CORE::CString error;
    if ( !m_journal.Record( entry, error ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetStoreServiceHost: a command was not written to the journal: " << error );
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::HandleCall( COMCORE::CRpcServerCall& call, bool isCommand, bool filesContract )
{GUCEF_TRACE;

    CRunningScope running( m_running );
    ++m_calls;
    DRNUTILS::CRemoteServicePrincipal principal;
    if ( !m_host.GetCallPrincipal( call, principal ) )
    {
        ++m_refusals;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::DeviceUnknown ) + ": not an active paired device" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CAssetServiceCall serviceCall;
    serviceCall.operation = request.GetString( CAssetStoreRemoteContracts::CallOperationField );
    serviceCall.commandId = request.GetString( CAssetStoreRemoteContracts::CallCommandIdField );
    request.GetDataNode( CAssetStoreRemoteContracts::CallArgsField, serviceCall.args );
    const CORE::CString& operation = serviceCall.operation;

    // the right method for the operation: reads and mutations apart, files on their contract, secrets and blocks on their own methods
    if ( IsSignInOperation( operation ) && !IsSignInPrincipal( principal ) )
    {
        RefuseSignIn( call, principal, operation );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( CAssetServiceOperations::PutThumbnail == operation && !principal.HasRole( "owner" ) )
    {
        // the pictures of the assets are written by the owner devices; the other devices keep their read access
        ++m_refusals;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: " << operation << " refused for the device " << principal.deviceId << ": a picture is put by the owner devices" );
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "the operation \"" + operation + "\" is for the devices with the owner role" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    {
        const CORE::CString syncRefusal = GetSyncRefusal( principal, serviceCall );
        if ( !syncRefusal.IsNULLOrEmpty() )
        {
            ++m_refusals;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: " << operation << " refused for the device " << principal.deviceId << ": " << syncRefusal );
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, syncRefusal ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
    }
    const bool known = CAssetServiceOperations::IsKnown( operation );
    const bool mutation = CAssetServiceOperations::IsMutation( operation );
    if ( !known || mutation != isCommand || CAssetStoreRemoteContracts::IsFilesOperation( operation ) != filesContract ||
         CAssetServiceOperations::AnswerPrompt == operation || CAssetServiceOperations::UnlockKeyring == operation ||
         CAssetServiceOperations::ReadFileBlock == operation || CAssetServiceOperations::WriteUploadBlock == operation )
    {
        ++m_refusals;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, known ? "the operation \"" + operation + "\" is not called this way"
                                                                                    : "the operation \"" + operation + "\" is not known" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }

    CORE::CString cacheHash;
    CORE::CString journalHash;
    CORE::CString inFlightKey;
    if ( isCommand )
    {
        ++m_commandCount;
        if ( m_host.RefuseIfDraining( call ) )
            return COMCORE::RPC_HANDLER_DONE;
        if ( !serviceCall.commandId.IsNULLOrEmpty() )
        {
            if ( !DRNUTILS::CCommandOutcomeCache::IsValidCommandId( serviceCall.commandId ) )
            {
                ++m_refusals;
                call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::BadCommandId ) +
                                                ": a command id has 1 to 64 characters of A-Z, a-z, 0-9, '.', '_' and '-'" ) );
                return COMCORE::RPC_HANDLER_DONE;
            }
            cacheHash = m_commands.HashRequest( request, CAssetStoreRemoteContracts::CallCommandIdField );
            if ( m_journalOpen )
                journalHash = m_journal.HashRequest( request, CAssetStoreRemoteContracts::CallCommandIdField );
            // a repeat that arrives while the first one runs waits for its outcome
            inFlightKey = principal.deviceId + "\n" + serviceCall.commandId;
            while ( !BeginCommand( inFlightKey ) )
            {
                if ( call.IsCancelled() )
                    return COMCORE::RPC_HANDLER_DONE;
                MT::ThreadDelay( 5 );
            }
            if ( AnswerKnownCommand( call, principal, serviceCall.commandId, cacheHash, journalHash ) )
            {
                EndCommand( inFlightKey );
                return COMCORE::RPC_HANDLER_DONE;
            }
        }
    }

    CDrainHoldScope hold( m_host );
    CORE::CICloneable* result = GUCEF_NULL;
    CORE::CString message;
    TAssetStoreStatus status = Execute( principal, serviceCall, result, message );
    if ( isCommand )
        WaitForEventBarrier();
    COMCORE::CRpcMessage answer = call.NewResponse();
    CAssetStoreRemoteCodec::FillAnswer( answer, status, message, CAssetStoreRemoteCodec::AsRecord( operation, result ), GetNextEventSeq( principal ), false );
    if ( isCommand && !serviceCall.commandId.IsNULLOrEmpty() )
    {
        RememberCommand( principal, operation, serviceCall.commandId, cacheHash, journalHash, answer, status, message, result );
        EndCommand( inFlightKey );
    }
    GUCEF_DELETE result;
    call.Reply( answer );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnSecret( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRunningScope running( m_running );
    ++m_calls;
    ++m_commandCount;
    DRNUTILS::CRemoteServicePrincipal principal;
    if ( !m_host.GetCallPrincipal( call, principal ) )
    {
        ++m_refusals;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::DeviceUnknown ) + ": not an active paired device" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( m_host.RefuseIfDraining( call ) )
        return COMCORE::RPC_HANDLER_DONE;
    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString operation = request.GetString( CAssetStoreRemoteContracts::SecretOperationField );
    const CORE::CString commandId = request.GetString( CAssetStoreRemoteContracts::SecretCommandIdField );
    if ( !IsSignInPrincipal( principal ) )
    {
        // the answer of a prompt and the keyring code: the device of a person only (the operation name is no secret)
        RefuseSignIn( call, principal, operation );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const CORE::CString scopedCommandId = commandId.IsNULLOrEmpty() ? CORE::CString::Empty : principal.deviceId + ":" + commandId;
    CDrainHoldScope hold( m_host );
    CAssetServiceResult result;
    TAssetStoreStatus status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
    {
        // the secret lives in this scope only; it is never logged
        AUTH::CSecretString secret( request.GetString( CAssetStoreRemoteContracts::SecretValueField ) );
        if ( CAssetServiceOperations::AnswerPrompt == operation )
            status = m_service->AnswerPrompt( scopedCommandId, request.GetString( CAssetStoreRemoteContracts::SecretTargetField ), secret, result );
        else
        if ( CAssetServiceOperations::UnlockKeyring == operation )
            status = m_service->UnlockKeyring( scopedCommandId, secret, result );
        else
        {
            result.status = status;
            result.message = "the operation \"" + operation + "\" carries no secret";
        }
    }
    WaitForEventBarrier();
    COMCORE::CRpcMessage answer = call.NewResponse();
    CAssetStoreRemoteCodec::FillAnswer( answer, status, result.message, &result, GetNextEventSeq( principal ), false );
    call.Reply( answer );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnReadBlock( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRunningScope running( m_running );
    DRNUTILS::CRemoteServicePrincipal principal;
    if ( !m_host.GetCallPrincipal( call, principal ) )
    {
        ++m_refusals;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::DeviceUnknown ) + ": not an active paired device" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString path = request.GetString( CAssetStoreRemoteContracts::ReadPathField );
    const UInt64 offset = request.GetUInt64( CAssetStoreRemoteContracts::ReadOffsetField );
    UInt32 length = request.GetUInt32( CAssetStoreRemoteContracts::ReadLengthField );
    if ( length > m_settings.maxReadBlockBytes )
        length = m_settings.maxReadBlockBytes;
    CAssetFileBlock block;
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    CORE::CString denial;
    const CAssetFileServingGate::SCaller gateCaller = MakeGateCaller( principal );
    if ( gateCaller.NeedsLook() && CAssetFileServingGate::ALLOWED != m_fileGate.CheckFile( gateCaller, path, denial ) )
    {
        // the licence gate: the blocks of a file are for the owner devices; the machine role sees metadata only
        ++m_refusals;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: a read of " << path << " is refused for the device " << principal.deviceId << ": " << denial );
        status = ASSETSTORE_STATUS_REFUSED;
    }
    else
        status = m_service->ReadFileBlock( path, offset, length, block );
    COMCORE::CRpcMessage answer = call.NewResponse();
    answer.SetString( CAssetStoreRemoteContracts::BlockStatusField, AssetStoreStatusToString( status ) );
    if ( ASSETSTORE_STATUS_OK == status )
    {
        answer.SetUInt64( CAssetStoreRemoteContracts::BlockFileSizeField, block.fileSize );
        answer.SetBool( CAssetStoreRemoteContracts::BlockEndOfFileField, block.endOfFile );
        answer.SetUInt64( CAssetStoreRemoteContracts::BlockOffsetField, block.offset );
        answer.SetBytes( CAssetStoreRemoteContracts::BlockDataField, block.data );
        m_bytesRead.FetchAdd( block.data.GetDataSize() );
    }
    else
    {
        answer.SetString( CAssetStoreRemoteContracts::BlockMessageField, denial.IsNULLOrEmpty() ? "the block of " + path + " cannot be read: " + AssetStoreStatusToString( status ) : denial );
    }
    call.Reply( answer );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CAssetStoreServiceHost::OnWriteBlock( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CRunningScope running( m_running );
    DRNUTILS::CRemoteServicePrincipal principal;
    if ( !m_host.GetCallPrincipal( call, principal ) )
    {
        ++m_refusals;
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::DeviceUnknown ) + ": not an active paired device" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    const CORE::CString uploadId = request.GetString( CAssetStoreRemoteContracts::WriteUploadIdField );
    const CORE::CString path = request.GetString( CAssetStoreRemoteContracts::WritePathField );
    const UInt64 offset = request.GetUInt64( CAssetStoreRemoteContracts::WriteOffsetField );
    CAssetServiceResult result;
    if ( !m_stagingOpen )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "this service takes no uploads";
    }
    else
    {
        // the index of the file in the upload, kept per upload so a block costs no look at the manifest
        UInt32 fileIndex = 0;
        bool found = false;
        {
            MT::CScopeMutex lock( m_lock );
            TUploadFileIndexMap::const_iterator u = m_uploadIndexes.find( uploadId );
            if ( u != m_uploadIndexes.end() )
            {
                GUCEF::map< CORE::CString, UInt32 >::const_iterator f = (*u).second.find( path );
                found = f != (*u).second.end();
                if ( found )
                    fileIndex = (*f).second;
            }
        }
        if ( !found )
        {
            DRNUTILS::CUploadStaging::SUploadInfo info;
            if ( m_staging.Query( principal.deviceId, uploadId, info ) )
            {
                MT::CScopeMutex lock( m_lock );
                GUCEF::map< CORE::CString, UInt32 >& index = m_uploadIndexes[ uploadId ];
                for ( UInt32 i=0; i<(UInt32) info.files.size(); ++i )
                    index[ info.files[ i ].relativePath ] = i;
                GUCEF::map< CORE::CString, UInt32 >::const_iterator f = index.find( path );
                found = f != index.end();
                if ( found )
                    fileIndex = (*f).second;
            }
        }
        CORE::CDynamicBuffer data;
        request.GetBytes( CAssetStoreRemoteContracts::WriteDataField, data );
        CORE::CString detail;
        if ( !found )
        {
            result.status = ASSETSTORE_STATUS_NOT_FOUND;
            result.message = CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::UploadUnknown ) + ": the upload has no file " + path;
        }
        else
        if ( !m_staging.WriteBlock( principal.deviceId, uploadId, fileIndex, offset, data.GetConstBufferPtr(), data.GetDataSize(),
                                    DRNUTILS::CRemoteServiceHost::NowUtcMs(), detail, result.message ) )
        {
            result.status = StatusOfStagingDetail( detail );
            result.message = detail + ": " + result.message;
        }
        else
        {
            result.status = ASSETSTORE_STATUS_OK;
            result.id = uploadId;
            m_bytesWritten.FetchAdd( data.GetDataSize() );
        }
    }
    COMCORE::CRpcMessage answer = call.NewResponse();
    CAssetStoreRemoteCodec::FillAnswer( answer, result.status, result.message, &result, 0, false );
    call.Reply( answer );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::StatusOfStagingDetail( const CORE::CString& detailCode )
{GUCEF_TRACE;

    if ( DRNUTILS::CRemoteServiceDetailCodes::UnsafePath == detailCode )
        return ASSETSTORE_STATUS_UNSAFE_NAME;
    if ( DRNUTILS::CRemoteServiceDetailCodes::UploadUnknown == detailCode )
        return ASSETSTORE_STATUS_NOT_FOUND;
    if ( DRNUTILS::CRemoteServiceDetailCodes::UploadDigestMismatch == detailCode )
        return ASSETSTORE_STATUS_DIGEST_MISMATCH;
    if ( DRNUTILS::CRemoteServiceDetailCodes::QuotaExceeded == detailCode || DRNUTILS::CRemoteServiceDetailCodes::StagingFull == detailCode ||
         DRNUTILS::CRemoteServiceDetailCodes::UploadLimit == detailCode )
        return ASSETSTORE_STATUS_REFUSED;
    if ( DRNUTILS::CRemoteServiceDetailCodes::UploadIncomplete == detailCode || DRNUTILS::CRemoteServiceDetailCodes::OutOfRange == detailCode )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    return ASSETSTORE_STATUS_IO_ERROR;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::Execute( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call,
                                 CORE::CICloneable*& result, CORE::CString& message )
{GUCEF_TRACE;

    typedef CAssetServiceOperations Op;
    const CORE::CString& operation = call.operation;
    result = GUCEF_NULL;
    message.Clear();

    // what the host carries out itself
    if ( Op::BeginUploadFiles == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        result = answer;
        const TAssetStoreStatus status = BeginUploadFiles( principal, call, *answer );
        message = answer->message;
        return status;
    }
    if ( Op::QueryUpload == operation )
    {
        CAssetFileStatPage* answer = GUCEF_NEW CAssetFileStatPage();
        result = answer;
        return QueryUpload( principal, call, *answer, message );
    }
    if ( Op::CommitUpload == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        result = answer;
        const TAssetStoreStatus status = CommitUpload( principal, call, *answer );
        message = answer->message;
        return status;
    }
    if ( Op::AbortUpload == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        result = answer;
        const TAssetStoreStatus status = AbortUpload( principal, call, *answer );
        message = answer->message;
        return status;
    }
    if ( Op::BeginUpload == operation || Op::RequestLocalCopy == operation || Op::SetOfflinePin == operation )
    {
        CAssetServiceResult* answer = GUCEF_NEW CAssetServiceResult();
        result = answer;
        answer->status = ASSETSTORE_STATUS_UNSUPPORTED;
        if ( Op::BeginUpload == operation )
            answer->message = "the service takes an upload with its file list (paths, sizes and SHA-256 digests): BeginUploadFiles";
        else
            answer->message = "local copies and offline pins are kept by a client; this version of the remote service has none";
        message = answer->message;
        return answer->status;
    }

    // the pictures of the assets are content: the licence gate applies as to the blocks of the files
    if ( Op::GetThumbnails == operation || Op::PutThumbnail == operation )
    {
        const CAssetFileServingGate::SCaller gateCaller = MakeGateCaller( principal );
        if ( gateCaller.mcp && !gateCaller.owner )
        {
            // the machine role sees the metadata of every pack; the pictures are content
            result = Op::GetThumbnails == operation ? static_cast< CORE::CICloneable* >( GUCEF_NEW CAssetThumbnailPage() ) : static_cast< CORE::CICloneable* >( GUCEF_NEW CAssetServiceResult() );
            m_fileGate.CheckThumbnailId( gateCaller, "pack:none", message );
            ++m_refusals;
            return ASSETSTORE_STATUS_REFUSED;
        }
    }

    // the rest through the service, the command id scoped to the device
    CAssetServiceCall scoped( call );
    if ( !scoped.commandId.IsNULLOrEmpty() )
        scoped.commandId = principal.deviceId + ":" + call.commandId;

    // the licence gate (pack index design 6.6, remote design 7.3): a caller of a learning role never sees a pack with noMachineLearning. The host sets
    // the flag itself, whatever the client sent; a caller that is not of a learning role keeps what it asked for
    if ( IsLearningPrincipal( principal ) )
    {
        if ( Op::GetPackIndex == operation || Op::GetResourceGroups == operation || Op::GetResources == operation )
            scoped.SetBool( "forLearning", true );
        else
        if ( Op::Search == operation )
        {
            CAssetSearchQuery query;
            scoped.GetRecord( "query", query );
            CPrimaryAssetStore::ExcludeRestricted( query, CAssetLicence::NoMachineLearning );
            scoped.SetRecord( "query", query );
        }
    }
    TAssetStoreStatus status = m_service->Dispatch( scoped, result, message );
    if ( Op::GetThumbnails == operation && GUCEF_NULL != result && ASSETSTORE_STATUS_OK == status )
    {
        // the pictures of packs that the device may not have are taken out; the others are remembered as given to it
        const CAssetFileServingGate::SCaller gateCaller = MakeGateCaller( principal );
        CAssetThumbnailPage* page = static_cast< CAssetThumbnailPage* >( result );
        for ( size_t i=0; i<page->entries.size(); ++i )
        {
            CAssetThumbnail& entry = page->entries[ i ];
            if ( ASSETSTORE_STATUS_OK != entry.status )
                continue;
            CORE::CString denial;
            if ( gateCaller.NeedsLook() && CAssetFileServingGate::ALLOWED != m_fileGate.CheckThumbnailId( gateCaller, entry.assetId, denial ) )
            {
                entry.status = ASSETSTORE_STATUS_REFUSED;
                entry.message = denial;
                entry.vfsPath.Clear();
                ++m_refusals;
            }
            else
            if ( gateCaller.NeedsLook() )
                m_fileGate.GrantThumbnail( gateCaller.deviceId, entry.vfsPath );
        }
    }
    if ( Op::GetServiceInfo == operation && GUCEF_NULL != result )
    {
        CAssetServiceInfo* info = static_cast< CAssetServiceInfo* >( result );
        info->roles = principal.roles;
        info->capabilities.insert( "remote" );
        if ( m_stagingOpen )
            info->capabilities.insert( "uploads" );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::IsSignInOperation( const CORE::CString& operation )
{GUCEF_TRACE;

    typedef CAssetServiceOperations Op;
    return Op::BeginSignIn == operation || Op::CancelSignIn == operation || Op::SignOut == operation || Op::GetSignInPrompts == operation ||
           Op::AnswerPrompt == operation || Op::UnlockKeyring == operation || Op::LockKeyring == operation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::IsSignInPrincipal( const DRNUTILS::CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    for ( CORE::CString::StringSet::const_iterator i = m_settings.signInRoles.begin(); i != m_settings.signInRoles.end(); ++i )
        if ( "mcp" != (*i) && principal.HasRole( (*i) ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::RefuseSignIn( COMCORE::CRpcServerCall& call, const DRNUTILS::CRemoteServicePrincipal& principal, const CORE::CString& operation )
{GUCEF_TRACE;

    ++m_refusals;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: " << operation << " refused for the device " << principal.deviceId << ": the sign in and the keyring are for the owner devices" );
    call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "the operation \"" + operation + "\" is for the devices with the owner role" ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreServiceHost::GetSyncRefusal( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call )
{GUCEF_TRACE;

    typedef CAssetServiceOperations Op;
    const CORE::CString& operation = call.operation;
    const bool owner = principal.HasRole( "owner" );
    const bool machine = principal.HasRole( "mcp" );
    if ( Op::SetSyncMarks == operation || Op::SetSyncProfile == operation )
    {
        if ( !owner || machine )
            return "the marks and the profiles of the library sync are set by the devices with the owner role";
        return CORE::CString::Empty;
    }
    if ( Op::StartPipelineJob == operation )
    {
        CAssetPipelineRequest request;
        call.GetRecord( "request", request );
        if ( CAssetPipelineOperations::SyncLibrary != request.operation )
            return CORE::CString::Empty;
        const CAssetLibrarySyncSpec spec = CAssetLibrarySyncSpec::FromRequest( request );
        if ( spec.IsPlanOnly() )
        {
            if ( !owner && !machine )
                return "a plan of the library sync is started by the devices with the owner role or the machine role";
            return CORE::CString::Empty;
        }
        if ( !owner || machine )
            return "a run of the library sync downloads and is started by the devices with the owner role only";
        return CORE::CString::Empty;
    }
    if ( Op::ControlJob == operation && !owner )
    {
        // a library sync is controlled by the owner devices; the machine role may stop one (a dry run it started)
        CAssetJobEntry job;
        if ( ASSETSTORE_STATUS_OK != m_service->GetJob( call.GetText( "jobId" ), job ) || "sync" != job.subKind )
            return CORE::CString::Empty;
        TAssetJobAction action = ASSETJOB_ACTION_RETRY;
        const bool parsed = AssetJobActionFromString( call.GetText( "action" ), action );
        if ( machine && parsed && ( ASSETJOB_ACTION_CANCEL == action || ASSETJOB_ACTION_REMOVE == action ) )
            return CORE::CString::Empty;
        return "a library sync is controlled by the devices with the owner role";
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::SCaller
CAssetStoreServiceHost::MakeGateCaller( const DRNUTILS::CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    CAssetFileServingGate::SCaller caller;
    caller.deviceId = principal.deviceId;
    caller.owner = principal.HasRole( "owner" );
    caller.mcp = principal.HasRole( "mcp" );
    caller.learning = IsLearningPrincipal( principal );
    return caller;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceHost::IsLearningPrincipal( const DRNUTILS::CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    for ( CORE::CString::StringSet::const_iterator i = m_settings.learningRoles.begin(); i != m_settings.learningRoles.end(); ++i )
        if ( principal.HasRole( (*i) ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::BeginUploadFiles( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    if ( !m_stagingOpen )
    {
        result.status = ASSETSTORE_STATUS_UNSUPPORTED;
        result.message = "this service takes no uploads";
        return result.status;
    }
    CAssetFileStatPage files;
    call.GetRecord( "files", files );
    DRNUTILS::CUploadStaging::TFileSpecVector specs;
    for ( size_t i=0; i<files.entries.size(); ++i )
    {
        DRNUTILS::CUploadStaging::SFileSpec spec;
        spec.relativePath = files.entries[ i ].path;
        spec.size = files.entries[ i ].sizeBytes < 0 ? 0 : (UInt64) files.entries[ i ].sizeBytes;
        spec.sha256Hex = files.entries[ i ].digest.Lowercase();
        specs.push_back( spec );
    }
    if ( specs.empty() )
    {
        result.status = ASSETSTORE_STATUS_INVALID_ARGUMENT;
        result.message = "an upload needs at least one file";
        return result.status;
    }
    CORE::CString uploadId;
    CORE::CString detail;
    if ( !m_staging.Begin( principal.deviceId, call.GetText( "name" ), specs, DRNUTILS::CRemoteServiceHost::NowUtcMs(), uploadId, detail, result.message ) )
    {
        result.status = StatusOfStagingDetail( detail );
        result.message = detail + ": " + result.message;
        return result.status;
    }
    result.status = ASSETSTORE_STATUS_OK;
    result.id = uploadId;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: upload " << uploadId << " of " << specs.size() << " files begun by device " << principal.deviceId );
    return result.status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::QueryUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetFileStatPage& result, CORE::CString& message )
{GUCEF_TRACE;

    result.entries.clear();
    DRNUTILS::CUploadStaging::SUploadInfo info;
    if ( !m_stagingOpen || !m_staging.Query( principal.deviceId, call.GetText( "uploadId" ), info ) )
    {
        message = CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::UploadUnknown ) + ": no upload " + call.GetText( "uploadId" ) + " of this device";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    // per file: the bytes from its start that arrived (where it continues), the declared digest and what arrived
    for ( size_t i=0; i<info.files.size(); ++i )
    {
        CAssetFileRecord entry;
        entry.path = info.files[ i ].relativePath;
        entry.digest = info.files[ i ].sha256Hex;
        const DRNUTILS::CUploadRangeSet& received = i < info.received.size() ? info.received[ i ] : DRNUTILS::CUploadRangeSet();
        entry.sizeBytes = (Int64) ContiguousFromStart( received );
        entry.role = received.IsComplete( info.files[ i ].size ) ? CORE::CString( "complete" ) : "received:" + received.ToString();
        result.entries.push_back( entry );
    }
    result.total = result.entries.size();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::CommitUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    const CORE::CString uploadId = call.GetText( "uploadId" );
    if ( !m_stagingOpen )
    {
        result.status = ASSETSTORE_STATUS_NOT_FOUND;
        result.message = "this service takes no uploads";
        return result.status;
    }
    // the name that the device gave the upload names the pack, unless the ingest says otherwise: the staged folder is always called "files"
    DRNUTILS::CUploadStaging::SUploadInfo uploadInfo;
    const bool hasUploadInfo = m_staging.Query( principal.deviceId, uploadId, uploadInfo );
    CORE::CString staged;
    CORE::CString detail;
    if ( !m_staging.Commit( principal.deviceId, uploadId, staged, detail, result.message ) )
    {
        result.status = StatusOfStagingDetail( detail );
        result.message = detail + ": " + result.message;
        return result.status;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_uploadIndexes.erase( uploadId );
    }

    // the digests are right: the files are ingested from the VFS mount of the staging, copied into the store
    CAssetIngestRequest request;
    call.GetRecord( "ingest", request );
    if ( hasUploadInfo && request.item.title.IsNULLOrEmpty() )
        request.item.title = uploadInfo.name;
    if ( request.sourceId.IsNULLOrEmpty() )
        request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
    request.contentPath = m_settings.stagingVfsRoot + "/" + uploadId + "/files";
    request.copyContent = 1;
    if ( request.locationId.IsNULLOrEmpty() )
        request.locationId = m_settings.uploadLocationId;
    CAssetIngestResult ingest;
    const CORE::CString commandId = call.commandId.IsNULLOrEmpty() ? CORE::CString::Empty : principal.deviceId + ":" + call.commandId;
    const TAssetStoreStatus status = m_service->Ingest( commandId, request, ingest );
    m_staging.Release( uploadId );
    result.status = status;
    result.id = ingest.ingestId;
    result.relatedId = ASSETSTORE_STATUS_DUPLICATE == status ? ingest.duplicateOfPackId : ingest.packId;
    result.message = ingest.message;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: upload " << uploadId << " committed and ingested: " << AssetStoreStatusToString( status ) );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreServiceHost::AbortUpload( const DRNUTILS::CRemoteServicePrincipal& principal, const CAssetServiceCall& call, CAssetServiceResult& result )
{GUCEF_TRACE;

    result.Clear();
    const CORE::CString uploadId = call.GetText( "uploadId" );
    if ( !m_stagingOpen || !m_staging.Abort( principal.deviceId, uploadId ) )
    {
        result.status = ASSETSTORE_STATUS_NOT_FOUND;
        result.message = CORE::CString( DRNUTILS::CRemoteServiceDetailCodes::UploadUnknown ) + ": no upload " + uploadId + " of this device";
        return result.status;
    }
    MT::CScopeMutex lock( m_lock );
    m_uploadIndexes.erase( uploadId );
    result.status = ASSETSTORE_STATUS_OK;
    result.id = uploadId;
    return result.status;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::WaitForEventBarrier( void )
{GUCEF_TRACE;

    // events raised on other threads are mailed to the host's pulse thread: two pulses later they are in the rings
    CORE::PulseGeneratorPtr pulse = m_host.GetPulseGenerator();
    if ( pulse.IsNULL() || 0 == m_settings.eventBarrierMs )
        return;
    const UInt64 target = m_pulses.Load() + 2;
    const UInt64 deadline = NowMs() + m_settings.eventBarrierMs;
    while ( m_pulses.Load() < target && NowMs() < deadline )
    {
        pulse->RequestImmediatePulseThroughDriver();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreServiceHost::GetNextEventSeq( const DRNUTILS::CRemoteServicePrincipal& principal ) const
{GUCEF_TRACE;

    if ( principal.sessionId.IsNULLOrEmpty() )
        return 0;
    return m_host.GetRingState( principal.sessionId ).nextEventSeq;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::Publish( const char* kind, const CORE::CIDataNodeSerializable* record, const CORE::CString::StringVector& ids, UInt64 revision, bool signInSessionsOnly )
{GUCEF_TRACE;

    COMCORE::CRpcMessage event = m_host.NewEvent( kind );
    if ( !event.IsBound() )
        return;
    for ( size_t i=0; i<ids.size(); ++i )
        event.Add( 4, CORE::CVariant( ids[ i ] ) );
    if ( 0 != revision )
        event.SetUInt64( 5, revision );
    if ( GUCEF_NULL != record )
    {
        CORE::CDataNode node( "data", GUCEF_DATATYPE_OBJECT );
        CORE::CDataNodeSerializableSettings settings;
        if ( record->Serialize( node, settings ) )
            event.SetDataNode( 6, node );
    }
    ++m_eventsPublished;
    if ( !signInSessionsOnly )
    {
        m_host.PublishEventToAll( event );
        return;
    }

    // the steps of a sign in (an address, a code to enter) go to the sessions of the devices that may drive a sign in, not to every client
    DRNUTILS::TRemoteSessionVector sessions;
    m_host.GetSessionCache().GetSessions( sessions );
    for ( size_t i=0; i<sessions.size(); ++i )
    {
        DRNUTILS::CDeviceRecord device;
        if ( !m_host.GetDeviceRegistry().Find( sessions[ i ]->GetDeviceId(), device ) || !device.IsActive() )
            continue;
        DRNUTILS::CRemoteServicePrincipal principal;
        principal.roles = device.roles;
        if ( IsSignInPrincipal( principal ) )
            m_host.PublishEvent( sessions[ i ]->GetSessionId(), event );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::OnServiceEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CString::StringVector ids;
    if ( CIAssetStoreService::CatalogueChangedEvent == eventId )
    {
        const CAssetCatalogueChange* change = CORE::CICloneable::CloneableAs< CAssetCatalogueChange >( eventData );
        Publish( CAssetStoreRemoteContracts::EventCatalogueChanged, change, GUCEF_NULL != change ? change->ids : ids, GUCEF_NULL != change ? change->revision : 0 );
        return;
    }
    if ( CIAssetStoreService::JobChangedEvent == eventId || CIAssetStoreService::JobRemovedEvent == eventId )
    {
        const CAssetJobEntry* job = CORE::CICloneable::CloneableAs< CAssetJobEntry >( eventData );
        if ( GUCEF_NULL == job )
            return;
        ids.push_back( job->jobId );
        if ( CIAssetStoreService::JobRemovedEvent == eventId )
        {
            {
                MT::CScopeMutex lock( m_lock );
                m_jobs.erase( job->jobId );
            }
            Publish( CAssetStoreRemoteContracts::EventJobRemoved, job, ids, 0 );
            return;
        }
        // progress at most every jobProgressIntervalMs per job; a change of state, and the end, at once
        const UInt64 now = NowMs();
        bool send = false;
        {
            MT::CScopeMutex lock( m_lock );
            SJobThrottle& throttle = m_jobs[ job->jobId ];
            send = throttle.state != job->state || 0 == throttle.lastSentMs || job->IsEnded() ||
                   now - throttle.lastSentMs >= m_settings.jobProgressIntervalMs;
            throttle.state = job->state;
            if ( send )
            {
                throttle.lastSentMs = now;
                throttle.pending = false;
            }
            else
            {
                throttle.pending = true;
                throttle.entry = *job;
            }
        }
        if ( send )
            Publish( CAssetStoreRemoteContracts::EventJobChanged, job, ids, 0 );
        return;
    }
    // the record of each event by the type its event id fixes (checked by type name, no RTTI)
    if ( CIAssetStoreService::LocationStateChangedEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventLocationState, CORE::CICloneable::CloneableAs< CAssetLocationInfo >( eventData ), ids, 0 );
    else if ( CIAssetStoreService::StoreStateChangedEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventStoreState, CORE::CICloneable::CloneableAs< CAssetStoreInfo >( eventData ), ids, 0 );
    else if ( CIAssetStoreService::StoresChangedEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventStoresChanged, GUCEF_NULL, ids, 0 );
    else if ( CIAssetStoreService::SignInPromptEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventSignInPrompt, CORE::CICloneable::CloneableAs< CAssetSignInPrompt >( eventData ), ids, 0, true );
    else if ( CIAssetStoreService::SignInPromptClosedEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventSignInPromptClosed, CORE::CICloneable::CloneableAs< CAssetSignInPrompt >( eventData ), ids, 0, true );
    else if ( CIAssetStoreService::ThumbnailUpdatedEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventThumbnailUpdated, CORE::CICloneable::CloneableAs< CAssetThumbnail >( eventData ), ids, 0 );
    else if ( CIAssetStoreService::NoticeEvent == eventId )
        Publish( CAssetStoreRemoteContracts::EventServiceNotice, CORE::CICloneable::CloneableAs< CAssetServiceNotice >( eventData ), ids, 0 );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ++m_pulses;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the progress held back goes out once its interval passed
    const UInt64 now = NowMs();
    GUCEF::vector< CAssetJobEntry > due;
    {
        MT::CScopeMutex lock( m_lock );
        for ( TJobThrottleMap::iterator i = m_jobs.begin(); i != m_jobs.end(); ++i )
        {
            SJobThrottle& throttle = (*i).second;
            if ( throttle.pending && now - throttle.lastSentMs >= m_settings.jobProgressIntervalMs )
            {
                throttle.pending = false;
                throttle.lastSentMs = now;
                due.push_back( throttle.entry );
            }
        }
    }
    for ( size_t i=0; i<due.size(); ++i )
    {
        CORE::CString::StringVector ids;
        ids.push_back( due[ i ].jobId );
        Publish( CAssetStoreRemoteContracts::EventJobChanged, &due[ i ], ids, 0 );
    }

    // uploads that were left go after their expiry, checked once a minute
    if ( m_stagingOpen && now - m_lastExpiryMs >= 60000 )
    {
        m_lastExpiryMs = now;
        const UInt32 expired = m_staging.ExpireOld( DRNUTILS::CRemoteServiceHost::NowUtcMs() );
        if ( expired > 0 )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreServiceHost: " << expired << " uploads expired" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::GetInfo( CORE::CValueList& info ) const
{GUCEF_TRACE;

    info.Set( "assetstore.calls", CORE::CVariant( m_calls.Load() ) );
    info.Set( "assetstore.commands", CORE::CVariant( m_commandCount.Load() ) );
    info.Set( "assetstore.replays", CORE::CVariant( m_replays.Load() ) );
    info.Set( "assetstore.refusals", CORE::CVariant( m_refusals.Load() ) );
    info.Set( "assetstore.running", CORE::CVariant( m_running.Load() ) );
    info.Set( "assetstore.bytesRead", CORE::CVariant( m_bytesRead.Load() ) );
    info.Set( "assetstore.bytesWritten", CORE::CVariant( m_bytesWritten.Load() ) );
    info.Set( "assetstore.events", CORE::CVariant( m_eventsPublished.Load() ) );
    if ( m_stagingOpen )
    {
        info.Set( "assetstore.uploadsOpen", CORE::CVariant( m_staging.GetOpenCount() ) );
        info.Set( "assetstore.stagedBytes", CORE::CVariant( m_staging.GetTotalUsedBytes() ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreServiceHost::GetResourceSourceName( void ) const
{GUCEF_TRACE;

    return "assetstore";
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreServiceHost::GetResourceCounters( CORE::CValueList& counters ) const
{GUCEF_TRACE;

    counters.Set( "calls", CORE::CVariant( m_calls.Load() ) );
    counters.Set( "running", CORE::CVariant( m_running.Load() ) );
    counters.Set( "events", CORE::CVariant( m_eventsPublished.Load() ) );
    if ( m_stagingOpen )
    {
        counters.Set( "uploads", CORE::CVariant( m_staging.GetOpenCount() ) );
        counters.Set( "stagedMB", CORE::CVariant( m_staging.GetTotalUsedBytes() / ( 1024 * 1024 ) ) );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreServiceHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreServiceHost";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
