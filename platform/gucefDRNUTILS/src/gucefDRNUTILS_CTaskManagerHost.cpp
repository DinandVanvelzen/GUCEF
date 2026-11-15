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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#include "gucefDRNUTILS_CTaskManagerContracts.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H
#include "gucefDRNUTILS_CCommandIdempotencyCache.h"
#define GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_DRNUTILS_CCONTROLRATELIMITER_H
#include "gucefDRNUTILS_CControlRateLimiter.h"
#define GUCEF_DRNUTILS_CCONTROLRATELIMITER_H
#endif /* GUCEF_DRNUTILS_CCONTROLRATELIMITER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#include "gucefDRNUTILS_CTaskManagerSample.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLE_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H
#include "gucefDRNUTILS_CTaskManagerSampler.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H
#include "gucefDRNUTILS_CTaskManagerTablePublisher.h"
#define GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERTABLEPUBLISHER_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERWATCHSTREAM_H
#include "gucefDRNUTILS_CTaskManagerWatchStream.h"
#define GUCEF_DRNUTILS_CTASKMANAGERWATCHSTREAM_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERWATCHSTREAM_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H
#include "gucefDRNUTILS_CTaskManagerRowCodec.h"
#define GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERROWCODEC_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSDIGEST_H
#include "gucefDRNUTILS_CDrnUtilsDigest.h"
#define GUCEF_DRNUTILS_CDRNUTILSDIGEST_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSDIGEST_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsHostSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H ? */

#include "gucefDRNUTILS_CTaskManagerHost.h"

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

/* field numbers of gucef.drnutils.tasks 1.0 */
enum { INFO_HOSTNAME = 1, INFO_INSTANCEID = 2, INFO_SERVERTIME = 3, INFO_CAPABILITIES = 4, INFO_CONTROLLABLEPOOLS = 5, INFO_TABLENAMES = 6 };
enum { POOLLIST_POOLS = 1, POOLLIST_SAMPLEUTC = 2 };
enum { LISTTHREADS_POOL = 1 };
enum { THREADLIST_THREADS = 1 };
enum { LISTTASKS_POOL = 1, LISTTASKS_INCLUDEQUEUED = 2, LISTTASKS_INCLUDEENDED = 3, LISTTASKS_MAXROWS = 4 };
enum { TASKLIST_TASKS = 1, TASKLIST_TRUNCATED = 2 };
enum { SJLIST_JOBS = 1 };
enum { WATCH_INTERVAL = 1, WATCH_THREADS = 2, WATCH_TASKS = 3, WATCH_SCHEDULEDJOBS = 4 };
enum { SAMPLE_UTC = 1, SAMPLE_POOLS = 2, SAMPLE_THREADS = 3, SAMPLE_TASKS = 4, SAMPLE_TRUNCATED = 5, SAMPLE_SCHEDULEDJOBS = 6 };
enum { CONTROL_COMMANDID = 1, CONTROL_ACTION = 2, CONTROL_POOL = 3, CONTROL_TASKID = 4, CONTROL_MINWORKERS = 5, CONTROL_MAXTHREADS = 6,
       CONTROL_REASON = 7, CONTROL_DRYRUN = 8 };
enum { OUTCOME_REPLAYED = 1, OUTCOME_MESSAGES = 2 };

/** Pools nobody resizes or acts on remotely: RPC protocol tasks, DRN nodes, the scheduler dispatcher, the external pseudo pool */
static const char* BuiltInProtectedPools[] = { "gucef.rpc", "gucef.rpc.loopback", "gucef.rpc.tcp", "gucef.drn", "gucef.scheduler", "external" };

/*-------------------------------------------------------------------------*/

const CORE::CString CTaskManagerHost::ClassTypeName = "GUCEF::DRNUTILS::CTaskManagerHost";

/*-------------------------------------------------------------------------*/

CTaskManagerHost::CTaskManagerHost( CITaskManagerSource& source                              ,
                                    const CTaskManagerHostSettings& settings                 ,
                                    const CORE::PulseGeneratorPtr& notificationPulseGenerator )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , m_source( &source )
    , m_settings( settings )
    , m_defaultAuditSink()
    , m_auditSink( &m_defaultAuditSink )
    , m_lock()
    , m_service( GUCEF_NEW COMCORE::CRpcServiceImpl() )
    , m_services()
    , m_contract()
    , m_commands( GUCEF_NEW CCommandIdempotencyCache() )
    , m_rateLimiter( GUCEF_NEW CControlRateLimiter() )
    , m_sampler( GUCEF_NEW CTaskManagerSampler( source, settings.GetSamplerPoolName() ) )
    , m_tablePublisher( GUCEF_NEW CTaskManagerTablePublisher() )
    , m_current( GUCEF_NEW CTaskManagerSample() )
    , m_listPrevious( GUCEF_NEW CTaskManagerSample() )
    , m_streams()
    , m_releasedCalls()
    , m_releaseTimer( GUCEF_NEW CORE::CTimer( notificationPulseGenerator, 10 ) )
    , m_graceTimer( GUCEF_NEW CORE::CTimer( notificationPulseGenerator, settings.GetViewingGraceMs() > 0 ? settings.GetViewingGraceMs() : 1 ) )
    , m_viewing( false )
{GUCEF_TRACE;

    m_commands->SetBounds( m_settings.GetIdempotencyTtlMs(), m_settings.GetMaxCommandsPerSubject(), m_settings.GetMaxCommands() );
    m_rateLimiter->SetMaxPerMinute( m_settings.GetMaxControlCommandsPerMinutePerSubject() );

    typedef COMCORE::CTRpcMethodHandler< CTaskManagerHost > THandler;
    m_service->BindMethod( "GetInfo", GUCEF_NEW THandler( this, &CTaskManagerHost::OnGetInfo ) );
    m_service->BindMethod( "ListPools", GUCEF_NEW THandler( this, &CTaskManagerHost::OnListPools ) );
    m_service->BindMethod( "ListThreads", GUCEF_NEW THandler( this, &CTaskManagerHost::OnListThreads ) );
    m_service->BindMethod( "ListTasks", GUCEF_NEW THandler( this, &CTaskManagerHost::OnListTasks ) );
    m_service->BindMethod( "Watch", GUCEF_NEW THandler( this, &CTaskManagerHost::OnWatch ) );
    m_service->BindMethod( "Control", GUCEF_NEW THandler( this, &CTaskManagerHost::OnControl ) );
    m_service->BindMethod( "ListScheduledJobs", GUCEF_NEW THandler( this, &CTaskManagerHost::OnListScheduledJobs ) );

    TEventCallback releaseTimer( this, &CTaskManagerHost::OnReleaseTimer );
    SubscribeTo( m_releaseTimer, CORE::CTimer::TimerUpdateEvent, releaseTimer );
    TEventCallback grace( this, &CTaskManagerHost::OnGraceTimer );
    SubscribeTo( m_graceTimer, CORE::CTimer::TimerUpdateEvent, grace );
}

/*-------------------------------------------------------------------------*/

CTaskManagerHost::~CTaskManagerHost()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    Withdraw();
    EndViewing();
    StopPublishing();
    m_graceTimer->SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_releaseTimer->SetEnabled( false );
    m_releasedCalls.clear();
    GUCEF_DELETE m_releaseTimer;
    GUCEF_DELETE m_graceTimer;
    GUCEF_DELETE m_sampler;
    GUCEF_DELETE m_tablePublisher;
    GUCEF_DELETE m_current;
    GUCEF_DELETE m_listPrevious;
    GUCEF_DELETE m_rateLimiter;
    GUCEF_DELETE m_commands;
    GUCEF_DELETE m_service;
}

/*-------------------------------------------------------------------------*/

const CTaskManagerHostSettings&
CTaskManagerHost::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::SetAuditSink( CIDrnUtilsAuditSink* sink )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_auditSink = GUCEF_NULL != sink ? sink : &m_defaultAuditSink;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::Expose( const COMCORE::CRpcServiceSetPtr& services, CORE::CString& error )
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_NONE == m_settings.GetExposure() )
    {
        error = "the settings expose nothing (exposure NONE)";
        return false;
    }
    if ( services.IsNULL() )
    {
        error = "no service set";
        return false;
    }
    if ( !m_services.IsNULL() )
    {
        error = "the host is exposed already";
        return false;
    }
    COMCORE::CRpcContractPtr contract = CTaskManagerContracts::GetContract();
    if ( contract.IsNULL() )
    {
        error = "the contract gucef.drnutils.tasks 1.0 could not be built";
        return false;
    }
    COMCORE::CRpcServiceSetPtr serviceSet( services );
    if ( !serviceSet->RegisterService( contract, CTaskManagerContracts::ServiceName, m_service, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) )
    {
        error = "the service TaskManager could not be registered";
        return false;
    }
    m_contract = contract;
    m_services = services;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::Withdraw( void )
{GUCEF_TRACE;

    if ( m_services.IsNULL() )
        return;
    m_services->UnregisterService( m_contract, CTaskManagerContracts::ServiceName );
    m_services.Unlink();
    RemoveAllStreams( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the task manager host was withdrawn" ) );
    m_graceTimer->SetEnabled( false );
    EndViewing();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::IsExposed( void ) const
{GUCEF_TRACE;

    return !m_services.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::PublishTables( const CORE::CString& drnSessionName, CORE::CString& error )
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_NONE == m_settings.GetExposure() )
    {
        error = "the settings expose nothing (exposure NONE)";
        return false;
    }
    StopPublishing();
    if ( !m_tablePublisher->Start( drnSessionName, m_settings.GetTablePrefix(), m_source->GetHostName(), error ) )
        return false;
    if ( m_viewing && 0 != m_current->sampleUtcMs )
        m_tablePublisher->Publish( *m_current, m_source->GetHostName(), m_source->GetHostInstanceId() );
    UpdateSampler();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::StopPublishing( void )
{GUCEF_TRACE;

    m_tablePublisher->Stop();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::IsInViewingMode( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_viewing;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::GetWatchStatistics( UInt32& openStreams, UInt64& samplesWritten, UInt64& samplesReplaced ) const
{GUCEF_TRACE;

    // the streams are kept by the thread of the event pulse generator only
    openStreams = (UInt32) m_streams.size();
    samplesWritten = 0;
    samplesReplaced = 0;
    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        samplesWritten += m_streams[ s ]->samplesWritten;
        samplesReplaced += m_streams[ s ]->samplesSkipped;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::GetTableNames( CORE::CString::StringMap& tableNames ) const
{GUCEF_TRACE;

    tableNames.clear();
    if ( !m_tablePublisher->IsStarted() )
        return;
    tableNames[ "pools" ] = m_tablePublisher->GetGridName( CTaskManagerTablePublisher::TABLE_POOLS );
    tableNames[ "threads" ] = m_tablePublisher->GetGridName( CTaskManagerTablePublisher::TABLE_THREADS );
    tableNames[ "tasks" ] = m_tablePublisher->GetGridName( CTaskManagerTablePublisher::TABLE_TASKS );
    tableNames[ "scheduledjobs" ] = m_tablePublisher->GetGridName( CTaskManagerTablePublisher::TABLE_SCHEDULEDJOBS );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::IsProtectedPool( const CORE::CString& poolName ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( BuiltInProtectedPools ) / sizeof( BuiltInProtectedPools[ 0 ] ); ++i )
        if ( poolName == BuiltInProtectedPools[ i ] )
            return true;
    if ( poolName == m_settings.GetSamplerPoolName() )
        return true;
    return m_settings.GetProtectedPools().find( poolName ) != m_settings.GetProtectedPools().end();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::GetHostInfo( const COMCORE::CRpcPrincipal& principal, STaskManagerHostInfo& info ) const
{GUCEF_TRACE;

    info = STaskManagerHostInfo();
    info.hostName = m_source->GetHostName();
    info.hostInstanceId = m_source->GetHostInstanceId();
    info.serverTimeUtcMs = m_source->GetUtcTimeInMs();
    info.capabilities.insert( "taskTimes" );
    info.capabilities.insert( "threadNames" );
    info.capabilities.insert( "scheduledJobs" );
    if ( DRNUTILS_EXPOSURE_CONTROL == m_settings.GetExposure() &&
         m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::TasksControlRole ) )
    {
        const CORE::CString::StringSet& controllable = m_settings.GetControllablePools();
        CORE::CString::StringSet::const_iterator i = controllable.begin();
        while ( i != controllable.end() )
        {
            if ( !IsProtectedPool( (*i) ) )
                info.controllablePools.insert( (*i) );
            ++i;
        }
        if ( !info.controllablePools.empty() )
        {
            info.capabilities.insert( "resizePool" );
            if ( !m_settings.GetCancellableTaskTypes().empty() )
            {
                info.capabilities.insert( "cancelTask" );
                info.capabilities.insert( "stopTask" );
            }
        }
    }
    GetTableNames( info.tableNames );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerHost::CheckViewAccess( const COMCORE::CRpcPrincipal& principal, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_NONE == m_settings.GetExposure() )
    {
        reason = "the task manager host exposes nothing";
        return false;
    }
    if ( !m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::TasksViewRole ) )
    {
        reason = "the role " + CORE::CString( CDrnUtilsRoleMap::TasksViewRole ) + " is needed";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::Audit( const SDrnUtilsAuditRecord& record )
{GUCEF_TRACE;

    CIDrnUtilsAuditSink* sink = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        sink = m_auditSink;
    }
    if ( GUCEF_NULL != sink )
        sink->OnAuditRecord( record );
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CTaskManagerHost::Control( const COMCORE::CRpcPrincipal& principal        ,
                           const CORE::CString& commandId                 ,
                           const STaskManagerControlCommand& command      ,
                           CORE::CString::StringVector& messages          ,
                           bool& replayed                                 )
{GUCEF_TRACE;

    replayed = false;
    messages.clear();

    CORE::CString::StringMap arguments;
    if ( DRNUTILS_TASKACTION_RESIZE_POOL == command.action )
    {
        arguments[ "minWorkers" ] = CORE::ToString( command.minWorkers );
        arguments[ "maxThreads" ] = CORE::ToString( command.maxThreads );
    }
    SDrnUtilsAuditRecord record;
    record.timeUtcMs = CDrnUtilsHostSupport::GetUtcMs();
    record.hostName = m_source->GetHostName();
    record.subject = principal.subject;
    record.transport = principal.transport;
    record.sessionId = principal.attributes.GetValueAlways( CDrnUtilsHostSupport::SessionIdAttribute ).AsString();
    record.contract = CTaskManagerContracts::ContractName;
    record.action = DrnUtilsTaskControlActionToString( command.action );
    record.targetKind = DRNUTILS_TASKACTION_RESIZE_POOL == command.action ? "POOL" : "TASK";
    record.targetId = DRNUTILS_TASKACTION_RESIZE_POOL == command.action ? command.poolName : CORE::ToString( command.taskId );
    record.argumentsDigest = CDrnUtilsDigest::ArgumentsDigest( arguments );
    record.reason = command.reason;
    record.commandId = commandId;
    record.dryRun = command.dryRun;

    CORE::CString refusal;
    TDrnUtilsCommandStatus status = DRNUTILS_STATUS_OK;
    CORE::CString poolName;
    CORE::CString taskTypeName;
    if ( !principal.isAuthenticated )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "control needs an authenticated principal";
    }
    else
    if ( !m_settings.GetAllowInsecureControl() && !CDrnUtilsHostSupport::IsSecure( principal ) )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "control needs a secure transport (" + principal.transport + " is not)";
    }
    else
    if ( !m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::TasksControlRole ) )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "the role " + CORE::CString( CDrnUtilsRoleMap::TasksControlRole ) + " is needed";
    }
    else
    if ( DRNUTILS_TASKACTION_RESIZE_POOL == command.action )
    {
        poolName = command.poolName;
        if ( m_settings.GetControllablePools().find( poolName ) == m_settings.GetControllablePools().end() || IsProtectedPool( poolName ) )
        {
            status = DRNUTILS_STATUS_PERMISSION_DENIED;
            refusal = "the pool \"" + poolName + "\" is not controllable remotely";
        }
        else
        if ( command.minWorkers < m_settings.GetMinAllowedWorkers() ||
             ( m_settings.GetMaxAllowedThreads() >= 0 && ( command.maxThreads < 0 || command.maxThreads > m_settings.GetMaxAllowedThreads() ) ) ||
             ( command.maxThreads >= 0 && (UInt32) command.maxThreads < command.minWorkers ) )
        {
            status = DRNUTILS_STATUS_INVALID_ARGUMENT;
            refusal = "the sizes are outside the allowed bounds (minimum " + CORE::ToString( m_settings.GetMinAllowedWorkers() ) +
                      " workers, maximum " + CORE::ToString( m_settings.GetMaxAllowedThreads() ) + " threads)";
        }
    }
    else
    if ( DRNUTILS_TASKACTION_CANCEL_TASK == command.action || DRNUTILS_TASKACTION_STOP_TASK == command.action )
    {
        if ( !m_source->GetTaskPoolAndType( command.taskId, poolName, taskTypeName ) )
        {
            status = DRNUTILS_STATUS_NOT_FOUND;
            refusal = "no task " + CORE::ToString( command.taskId );
        }
        else
        if ( m_settings.GetControllablePools().find( poolName ) == m_settings.GetControllablePools().end() || IsProtectedPool( poolName ) )
        {
            status = DRNUTILS_STATUS_PERMISSION_DENIED;
            refusal = "the pool \"" + poolName + "\" of the task is not controllable remotely";
        }
        else
        if ( m_settings.GetCancellableTaskTypes().find( taskTypeName ) == m_settings.GetCancellableTaskTypes().end() )
        {
            status = DRNUTILS_STATUS_PERMISSION_DENIED;
            refusal = "tasks of type \"" + taskTypeName + "\" cannot be cancelled or stopped remotely";
        }
        record.taskTypeName = taskTypeName;
    }
    else
    {
        status = DRNUTILS_STATUS_UNIMPLEMENTED;
        refusal = "the action " + CORE::ToString( (Int32) command.action ) + " is not supported in this version";
    }
    if ( DRNUTILS_STATUS_OK == status )
        status = CDrnUtilsHostSupport::CheckCommandId( commandId, command.dryRun, refusal );

    UInt64 nowMs = CDrnUtilsHostSupport::GetMonotonicMs();
    if ( DRNUTILS_STATUS_OK == status && !command.dryRun )
    {
        TDrnUtilsCommandStatus storedStatus = DRNUTILS_STATUS_OK;
        CORE::SJobControlResult stored;
        if ( m_commands->Find( principal.subject, commandId, nowMs, storedStatus, stored ) )
        {
            replayed = true;
            messages = stored.messages;
            record.replayed = true;
            record.outcome = DrnUtilsCommandStatusToString( storedStatus );
            Audit( record );
            return storedStatus;
        }
    }
    if ( DRNUTILS_STATUS_OK == status && !m_rateLimiter->Admit( principal.subject, nowMs ) )
    {
        status = DRNUTILS_STATUS_RESOURCE_EXHAUSTED;
        refusal = "the control rate limit of " + CORE::ToString( m_settings.GetMaxControlCommandsPerMinutePerSubject() ) + " commands per minute was reached";
    }
    if ( DRNUTILS_STATUS_OK != status )
    {
        messages.push_back( refusal );
        record.outcome = DrnUtilsCommandStatusToString( status );
        record.reason = refusal;
        Audit( record );
        return status;
    }

    CORE::CString error;
    if ( command.dryRun )
    {
        messages.push_back( "dry run: the command would be applied" );
    }
    else
    {
        bool applied = false;
        // on the caller's thread with no lock of the host held (rule 9.1)
        if ( DRNUTILS_TASKACTION_RESIZE_POOL == command.action )
            applied = m_source->ResizePool( command.poolName, command.minWorkers, command.maxThreads, error );
        else
        if ( DRNUTILS_TASKACTION_CANCEL_TASK == command.action )
            applied = m_source->CancelTask( command.taskId, error );
        else
            applied = m_source->StopTask( command.taskId, error );
        if ( applied )
        {
            messages.push_back( CORE::CString( DrnUtilsTaskControlActionToString( command.action ) ) + " applied" );
        }
        else
        {
            status = DRNUTILS_STATUS_FAILED_PRECONDITION;
            messages.push_back( error.IsNULLOrEmpty() ? CORE::CString( "the task manager refused the command" ) : error );
        }
        CORE::SJobControlResult stored;
        stored.messages = messages;
        m_commands->Store( principal.subject, commandId, CDrnUtilsHostSupport::GetMonotonicMs(), status, stored );
    }
    record.outcome = DrnUtilsCommandStatusToString( status );
    if ( DRNUTILS_STATUS_OK != status )
        record.reason = messages.empty() ? CORE::CString::Empty : messages[ 0 ];
    Audit( record );
    return status;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::UpdateSampler( void )
{GUCEF_TRACE;

    bool want = !m_streams.empty() || m_graceTimer->GetEnabled();
    {
        MT::CScopeMutex lock( m_lock );
        m_viewing = want;
    }
    if ( !want )
    {
        m_sampler->Stop( *this );
        return;
    }
    CTaskManagerSamplerTask::SConfig config;
    config.intervalMs = m_settings.GetSampleIntervalMs();
    config.taskIntervalMs = m_settings.GetTaskSampleIntervalMs();
    config.minIntervalMs = m_settings.GetMinSampleIntervalMs();
    config.options.maxTaskRows = m_settings.GetMaxTaskRows();
    config.options.maxThreadRows = m_settings.GetMaxThreadRows();
    config.options.finishedTaskRows = m_settings.GetFinishedTaskRows();
    bool publishing = m_tablePublisher->IsStarted();
    config.options.includeThreads = publishing;
    config.options.includeTasks = publishing;
    config.options.includeScheduledJobs = true;
    if ( !m_streams.empty() )
    {
        UInt32 interval = 0;
        for ( UInt32 s=0; s<m_streams.size(); ++s )
        {
            const CTaskManagerWatchStream& stream = *m_streams[ s ];
            if ( 0 == interval || stream.intervalMs < interval )
                interval = stream.intervalMs;
            config.options.includeThreads = config.options.includeThreads || stream.includeThreads;
            config.options.includeTasks = config.options.includeTasks || stream.includeTasks;
        }
        config.intervalMs = interval;
    }
    if ( config.intervalMs < m_settings.GetMinSampleIntervalMs() )
        config.intervalMs = m_settings.GetMinSampleIntervalMs();
    if ( m_sampler->IsRunning() )
    {
        m_sampler->Update( config );
    }
    else
    {
        TEventCallback ready( this, &CTaskManagerHost::OnSampleReady );
        m_sampler->Start( config, *this, ready );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::EndViewing( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_viewing = false;
    }
    m_sampler->Stop( *this );
    m_current->Clear();
    // the task tables are not kept current for nobody: they are emptied
    if ( m_tablePublisher->IsStarted() )
        m_tablePublisher->Clear();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::OnGraceTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_graceTimer->SetEnabled( false );
    if ( m_streams.empty() )
        EndViewing();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::BuildSampleMessage( const CTaskManagerWatchStream& stream, COMCORE::CRpcMessage& message ) const
{GUCEF_TRACE;

    const CTaskManagerSample& sample = *m_current;
    message.SetTimestamp( SAMPLE_UTC, sample.sampleUtcMs * 1000 );
    for ( UInt32 i=0; i<sample.pools.size(); ++i )
        CTaskManagerRowCodec::PoolToMessage( sample.pools[ i ], *message.AddMessage( SAMPLE_POOLS ) );
    if ( stream.includeThreads )
        for ( UInt32 i=0; i<sample.threads.size(); ++i )
            CTaskManagerRowCodec::ThreadToMessage( sample.threads[ i ], *message.AddMessage( SAMPLE_THREADS ) );
    if ( stream.includeTasks )
        for ( UInt32 i=0; i<sample.tasks.size(); ++i )
            CTaskManagerRowCodec::TaskToMessage( sample.tasks[ i ], *message.AddMessage( SAMPLE_TASKS ) );
    message.SetBool( SAMPLE_TRUNCATED, sample.truncated );
    if ( stream.includeScheduledJobs )
        for ( UInt32 i=0; i<sample.scheduledJobs.size(); ++i )
            CTaskManagerRowCodec::ScheduledJobToMessage( sample.scheduledJobs[ i ], *message.AddMessage( SAMPLE_SCHEDULEDJOBS ) );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::OnSampleReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CTaskManagerSample latest;
    if ( !m_sampler->GetLatest( latest ) )
        return;
    // threads and tasks of the latest sample that had them are kept between task samples
    CTaskManagerSample::TThreadVector threads;
    CTaskManagerSample::TTaskVector tasks;
    bool hadThreads = m_current->hasThreads;
    bool hadTasks = m_current->hasTasks;
    if ( !latest.hasThreads )
        threads.swap( m_current->threads );
    if ( !latest.hasTasks )
        tasks.swap( m_current->tasks );
    *m_current = latest;
    if ( !latest.hasThreads )
    {
        m_current->threads.swap( threads );
        m_current->hasThreads = hadThreads;
    }
    if ( !latest.hasTasks )
    {
        m_current->tasks.swap( tasks );
        m_current->hasTasks = hadTasks;
    }

    UInt64 nowMs = CDrnUtilsHostSupport::GetMonotonicMs();
    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CTaskManagerWatchStream& stream = *m_streams[ s ];
        if ( 0 != stream.lastSentMs && nowMs - stream.lastSentMs + 250 < stream.intervalMs )
            continue;
        COMCORE::CRpcMessage message = stream.call->NewResponse();
        BuildSampleMessage( stream, message );
        stream.lastSentMs = nowMs;
        // no lock held while writing; without credit the latest sample waits, older ones are dropped
        if ( stream.hasPending || !stream.call->Write( message ) )
        {
            if ( stream.hasPending )
                ++stream.samplesSkipped;
            stream.pending = message;
            stream.hasPending = true;
            if ( stream.call->Write( stream.pending ) )
            {
                stream.hasPending = false;
                ++stream.samplesWritten;
            }
        }
        else
        {
            ++stream.samplesWritten;
        }
    }
    if ( m_tablePublisher->IsStarted() )
        m_tablePublisher->Publish( latest, m_source->GetHostName(), m_source->GetHostInstanceId() );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::OnCallWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CTaskManagerWatchStream& stream = *m_streams[ s ];
        if ( static_cast< CORE::CNotifier* >( stream.call.GetPointerAlways() ) != notifier )
            continue;
        if ( stream.hasPending && stream.call->Write( stream.pending ) )
        {
            stream.hasPending = false;
            ++stream.samplesWritten;
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::OnCallCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RemoveStream( notifier );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::RemoveStream( CORE::CNotifier* callNotifier )
{GUCEF_TRACE;

    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CTaskManagerWatchStream* stream = m_streams[ s ];
        if ( static_cast< CORE::CNotifier* >( stream->call.GetPointerAlways() ) != callNotifier )
            continue;
        m_streams.erase( m_streams.begin() + s );
        UnsubscribeFrom( callNotifier );
        ReleaseLater( stream->call );
        GUCEF_DELETE stream;
        if ( m_streams.empty() )
        {
            // viewing mode ends ViewingGraceMs after the last stream closed
            if ( 0 == m_settings.GetViewingGraceMs() )
            {
                EndViewing();
                return;
            }
            m_graceTimer->SetInterval( m_settings.GetViewingGraceMs() );
            m_graceTimer->SetEnabled( true );
        }
        UpdateSampler();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::RemoveAllStreams( const COMCORE::CRpcStatus& status )
{GUCEF_TRACE;

    TWatchStreamVector streams;
    streams.swap( m_streams );
    for ( UInt32 s=0; s<streams.size(); ++s )
    {
        UnsubscribeFrom( streams[ s ]->call.GetPointerAlways() );
        streams[ s ]->call->Finish( status );
        GUCEF_DELETE streams[ s ];
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::GetListSample( bool includeThreads, bool includeTasks, bool includeScheduledJobs, CTaskManagerSample& sample )
{GUCEF_TRACE;

    if ( m_viewing && 0 != m_current->sampleUtcMs && ( !includeThreads || m_current->hasThreads ) && ( !includeTasks || m_current->hasTasks ) )
    {
        sample = *m_current;
        return;
    }
    CTaskManagerSample::SOptions options;
    options.includeThreads = includeThreads;
    options.includeTasks = includeTasks;
    options.includeScheduledJobs = includeScheduledJobs;
    options.maxTaskRows = m_settings.GetMaxTaskRows();
    options.maxThreadRows = m_settings.GetMaxThreadRows();
    options.finishedTaskRows = 0;
    sample.Take( *m_source, 0 != m_listPrevious->sampleMonotonicMs ? m_listPrevious : GUCEF_NULL, GUCEF_NULL, options );
    m_listPrevious->Clear();
    m_listPrevious->sampleUtcMs = sample.sampleUtcMs;
    m_listPrevious->sampleMonotonicMs = sample.sampleMonotonicMs;
    m_listPrevious->pools = sample.pools;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnGetInfo( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    STaskManagerHostInfo info;
    GetHostInfo( call.GetPrincipal(), info );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( INFO_HOSTNAME, info.hostName );
    response.SetUInt64( INFO_INSTANCEID, info.hostInstanceId );
    response.SetTimestamp( INFO_SERVERTIME, info.serverTimeUtcMs * 1000 );
    CORE::CString::StringSet::const_iterator i = info.capabilities.begin();
    while ( i != info.capabilities.end() )
    {
        response.Add( INFO_CAPABILITIES, CORE::CVariant( (*i) ) );
        ++i;
    }
    i = info.controllablePools.begin();
    while ( i != info.controllablePools.end() )
    {
        response.Add( INFO_CONTROLLABLEPOOLS, CORE::CVariant( (*i) ) );
        ++i;
    }
    CORE::CString::StringMap::const_iterator t = info.tableNames.begin();
    while ( t != info.tableNames.end() )
    {
        response.SetMapEntry( INFO_TABLENAMES, CORE::CVariant( (*t).first ), CORE::CVariant( (*t).second ) );
        ++t;
    }
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnListPools( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CTaskManagerSample sample;
    GetListSample( false, false, false, sample );
    COMCORE::CRpcMessage response = call.NewResponse();
    for ( UInt32 i=0; i<sample.pools.size(); ++i )
        CTaskManagerRowCodec::PoolToMessage( sample.pools[ i ], *response.AddMessage( POOLLIST_POOLS ) );
    response.SetTimestamp( POOLLIST_SAMPLEUTC, sample.sampleUtcMs * 1000 );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnListThreads( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CORE::CString pool = call.GetRequest().GetString( LISTTHREADS_POOL );
    CTaskManagerSample sample;
    GetListSample( true, false, false, sample );
    COMCORE::CRpcMessage response = call.NewResponse();
    for ( UInt32 i=0; i<sample.threads.size(); ++i )
        if ( pool.IsNULLOrEmpty() || pool == sample.threads[ i ].pool )
            CTaskManagerRowCodec::ThreadToMessage( sample.threads[ i ], *response.AddMessage( THREADLIST_THREADS ) );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnListTasks( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString pool = request.GetString( LISTTASKS_POOL );
    bool includeQueued = request.GetBool( LISTTASKS_INCLUDEQUEUED );
    bool includeEnded = request.GetBool( LISTTASKS_INCLUDEENDED );
    UInt32 maxRows = request.GetUInt32( LISTTASKS_MAXROWS );
    UInt32 bound = m_settings.GetMaxTaskRows() > 0 ? m_settings.GetMaxTaskRows() : 1000;
    maxRows = 0 == maxRows || maxRows > bound ? bound : maxRows;
    CORE::CString queuedText = CORE::TaskStatusToTaskStatusString( CORE::TASKSTATUS_QUEUED );

    CTaskManagerSample sample;
    GetListSample( false, true, false, sample );
    COMCORE::CRpcMessage response = call.NewResponse();
    UInt32 count = 0;
    bool truncated = sample.truncated;
    for ( UInt32 i=0; i<sample.tasks.size(); ++i )
    {
        const CTaskManagerSample::STask& task = sample.tasks[ i ];
        if ( !pool.IsNULLOrEmpty() && pool != task.pool )
            continue;
        if ( !includeQueued && task.status == queuedText )
            continue;
        if ( !includeEnded && task.ended )
            continue;
        if ( count >= maxRows )
        {
            truncated = true;
            break;
        }
        CTaskManagerRowCodec::TaskToMessage( task, *response.AddMessage( TASKLIST_TASKS ) );
        ++count;
    }
    response.SetBool( TASKLIST_TRUNCATED, truncated );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnListScheduledJobs( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CTaskManagerSample sample;
    GetListSample( false, false, true, sample );
    COMCORE::CRpcMessage response = call.NewResponse();
    for ( UInt32 i=0; i<sample.scheduledJobs.size(); ++i )
        CTaskManagerRowCodec::ScheduledJobToMessage( sample.scheduledJobs[ i ], *response.AddMessage( SJLIST_JOBS ) );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnWatch( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CTaskManagerWatchStream* stream = GUCEF_NEW CTaskManagerWatchStream();
    stream->call = call.GetSharedPtr();
    stream->sessionKey = CDrnUtilsHostSupport::GetSessionKey( call );
    UInt32 interval = request.GetUInt32( WATCH_INTERVAL );
    stream->intervalMs = 0 == interval ? m_settings.GetSampleIntervalMs() : interval;
    if ( stream->intervalMs < m_settings.GetMinSampleIntervalMs() )
        stream->intervalMs = m_settings.GetMinSampleIntervalMs();
    stream->includeThreads = request.GetBool( WATCH_THREADS );
    stream->includeTasks = request.GetBool( WATCH_TASKS );
    stream->includeScheduledJobs = request.GetBool( WATCH_SCHEDULEDJOBS );
    m_streams.push_back( stream );
    m_graceTimer->SetEnabled( false );

    TEventCallback writable( this, &CTaskManagerHost::OnCallWritable );
    TEventCallback cancelled( this, &CTaskManagerHost::OnCallCancelled );
    SubscribeTo( stream->call.GetPointerAlways(), COMCORE::CRpcServerCall::WritableEvent, writable );
    SubscribeTo( stream->call.GetPointerAlways(), COMCORE::CRpcServerCall::CancelledEvent, cancelled );

    // viewing mode: the sampler runs while a stream is open (its first sample is taken at once)
    UpdateSampler();
    return COMCORE::RPC_HANDLER_DEFERRED;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CTaskManagerHost::OnControl( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_CONTROL != m_settings.GetExposure() )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "control is not exposed by this host" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString commandId;
    CORE::CString problem;
    if ( !CDrnUtilsHostSupport::GetCommandId( call, request.GetString( CONTROL_COMMANDID ), commandId, problem ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, problem ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    STaskManagerControlCommand command;
    command.action = (TDrnUtilsTaskControlAction) request.GetEnum( CONTROL_ACTION );
    command.poolName = request.GetString( CONTROL_POOL );
    command.taskId = request.GetUInt32( CONTROL_TASKID );
    command.minWorkers = request.GetUInt32( CONTROL_MINWORKERS );
    command.maxThreads = request.GetInt32( CONTROL_MAXTHREADS );
    command.reason = request.GetString( CONTROL_REASON );
    command.dryRun = request.GetBool( CONTROL_DRYRUN );

    COMCORE::CRpcPrincipal principal( call.GetPrincipal() );
    principal.attributes.Set( CDrnUtilsHostSupport::SessionIdAttribute, CORE::CVariant( CDrnUtilsHostSupport::GetSessionKey( call ) ) );
    CORE::CString::StringVector messages;
    bool replayed = false;
    TDrnUtilsCommandStatus status = Control( principal, commandId, command, messages, replayed );
    if ( DRNUTILS_STATUS_OK != status )
    {
        call.Fail( COMCORE::CRpcStatus( CDrnUtilsHostSupport::ToRpcStatusCode( status ), CDrnUtilsHostSupport::JoinMessages( messages ) ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage outcome = call.NewResponse();
    outcome.SetBool( OUTCOME_REPLAYED, replayed );
    for ( UInt32 i=0; i<messages.size(); ++i )
        outcome.Add( OUTCOME_MESSAGES, CORE::CVariant( messages[ i ] ) );
    call.Reply( outcome );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::ReleaseLater( const COMCORE::CRpcServerCallPtr& call )
{GUCEF_TRACE;

    // the reference may be the last one, and this may run inside an event the call raises: the call would go while
    // it is still delivering. It goes at the next pulse instead, on this same (notification) thread
    m_releasedCalls.push_back( call );
    if ( !m_releaseTimer->GetEnabled() )
        m_releaseTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerHost::OnReleaseTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_releaseTimer->SetEnabled( false );
    GUCEF::vector< COMCORE::CRpcServerCallPtr > released;
    released.swap( m_releasedCalls );
    released.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
