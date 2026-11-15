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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CCRONSCHEDULE_H
#include "gucefCORE_CCronSchedule.h"
#define GUCEF_CORE_CCRONSCHEDULE_H
#endif /* GUCEF_CORE_CCRONSCHEDULE_H ? */

#ifndef GUCEF_CORE_CSCHEDULE_H
#include "gucefCORE_CSchedule.h"
#define GUCEF_CORE_CSCHEDULE_H
#endif /* GUCEF_CORE_CSCHEDULE_H ? */

#ifndef GUCEF_CORE_CDATETIMERANGE_H
#include "gucefCORE_CDateTimeRange.h"
#define GUCEF_CORE_CDATETIMERANGE_H
#endif /* GUCEF_CORE_CDATETIMERANGE_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#include "gucefDRNUTILS_CSchedulerContracts.h"
#define GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H
#include "gucefDRNUTILS_CSchedulerChangeJournal.h"
#define GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCHANGEJOURNAL_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H
#include "gucefDRNUTILS_CCommandIdempotencyCache.h"
#define GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_DRNUTILS_CCONTROLRATELIMITER_H
#include "gucefDRNUTILS_CControlRateLimiter.h"
#define GUCEF_DRNUTILS_CCONTROLRATELIMITER_H
#endif /* GUCEF_DRNUTILS_CCONTROLRATELIMITER_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H
#include "gucefDRNUTILS_CSchedulerWatchStream.h"
#define GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERWATCHSTREAM_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H
#include "gucefDRNUTILS_CSchedulerTablePublisher.h"
#define GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERTABLEPUBLISHER_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSDIGEST_H
#include "gucefDRNUTILS_CDrnUtilsDigest.h"
#define GUCEF_DRNUTILS_CDRNUTILSDIGEST_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSDIGEST_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsHostSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H ? */

#include "gucefDRNUTILS_CSchedulerHost.h"

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

/* field numbers of gucef.drnutils.scheduler 1.0 */
enum { HOSTINFO_HOSTNAME = 1, HOSTINFO_INSTANCEID = 2, HOSTINFO_REVISION = 3, HOSTINFO_SERVERTIME = 4, HOSTINFO_STATE = 5,
       HOSTINFO_ALLOWEDACTIONS = 6, HOSTINFO_TABLENAMES = 7, HOSTINFO_SUBMITTABLE = 8, HOSTINFO_MAXPAYLOAD = 9 };
enum { LIST_JOBID = 1, LIST_LANE = 2, LIST_MAXROWS = 3, LIST_PAGETOKEN = 4, LIST_INCLUDEENDED = 5 };
enum { JOBLIST_JOBS = 1, JOBLIST_NEXTPAGE = 2, JOBLIST_REVISION = 3 };
enum { LANELIST_LANES = 1, LANELIST_REVISION = 2 };
enum { RUNLIST_RUNS = 1, RUNLIST_NEXTPAGE = 2, RUNLIST_REVISION = 3 };
enum { GETJOB_JOBID = 1, GETJOB_RECENTRUNS = 2, GETJOB_SOURCESTATUS = 3 };
enum { DETAIL_JOB = 1, DETAIL_RECENTRUNS = 2, DETAIL_SOURCESTATUS = 3, DETAIL_DESCRIPTION = 4, DETAIL_PAYLOAD = 5 };
enum { WATCH_INSTANCEID = 1, WATCH_AFTERREVISION = 2, WATCH_INCLUDERUNS = 3, WATCH_MAXPERSECOND = 4 };
enum { CHANGE_KIND = 1, CHANGE_REVISION = 2, CHANGE_JOB = 3, CHANGE_LANE = 4, CHANGE_RUN = 5, CHANGE_INSTANCEID = 6, CHANGE_REMOVEDKEY = 7 };
enum { CHANGEKIND_JOB = 1, CHANGEKIND_LANE = 2, CHANGEKIND_RUN = 3, CHANGEKIND_RESET = 4, CHANGEKIND_SNAPSHOT_END = 5, CHANGEKIND_HOST = 6 };
enum { CONTROL_COMMANDID = 1, CONTROL_ACTION = 2, CONTROL_TARGETKIND = 3, CONTROL_TARGETID = 4, CONTROL_CUSTOMNAME = 5,
       CONTROL_ARGUMENTS = 6, CONTROL_PRIORITY = 8, CONTROL_MODE = 9, CONTROL_REASON = 10, CONTROL_EXPECTEDREVISION = 11,
       CONTROL_DRYRUN = 12, CONTROL_WAITFORRUN = 13 };
enum { OUTCOME_REPLAYED = 1, OUTCOME_REVISION = 2, OUTCOME_RUNID = 3, OUTCOME_MESSAGES = 4, OUTCOME_RUN = 5, OUTCOME_DETAIL = 6 };
enum { TRIGGER_KIND = 1, TRIGGER_CRON = 2, TRIGGER_RANGESTART = 3, TRIGGER_RANGEEND = 4, TRIGGER_PERIODMS = 5, TRIGGER_INTERVALFROM = 6,
       TRIGGER_ALIGNTO = 7, TRIGGER_ONCEAT = 8 };
enum { SUBMIT_COMMANDID = 1, SUBMIT_JOBID = 2, SUBMIT_TITLE = 3, SUBMIT_TASKTYPE = 4, SUBMIT_PAYLOAD = 5, SUBMIT_TRIGGER = 6,
       SUBMIT_EXECUTIONTARGET = 7, SUBMIT_THREADPOOL = 8, SUBMIT_LANE = 9, SUBMIT_PRIORITY = 10, SUBMIT_REPLACE = 11,
       SUBMIT_DRYRUN = 12, SUBMIT_REASON = 13, SUBMIT_DESCRIPTION = 14 };

static const UInt32 DefaultMaxRows = 100;
static const UInt32 MaxMaxRows = 1000;
static const UInt32 DefaultRecentRuns = 10;
static const UInt32 MaxRecentRuns = 200;

/*-------------------------------------------------------------------------*/

const CORE::CString CSchedulerHost::ClassTypeName = "GUCEF::DRNUTILS::CSchedulerHost";

/*-------------------------------------------------------------------------*/

static bool
IsJobTargetedAction( CORE::TJobControlAction action )
{
    return CORE::JOBCONTROLACTION_RUN_NOW == action || CORE::JOBCONTROLACTION_ENABLE == action ||
           CORE::JOBCONTROLACTION_DISABLE == action || CORE::JOBCONTROLACTION_SET_PRIORITY == action;
}

/*-------------------------------------------------------------------------*/

static CORE::TJobControlTargetKind
DefaultTargetKind( CORE::TJobControlAction action )
{
    switch ( action )
    {
        case CORE::JOBCONTROLACTION_CANCEL_RUN: return CORE::JOBCONTROLTARGET_RUN;
        case CORE::JOBCONTROLACTION_SET_LANE_MODE: return CORE::JOBCONTROLTARGET_LANE;
        default: return CORE::JOBCONTROLTARGET_JOB;
    }
}

/*-------------------------------------------------------------------------*/

static const char*
TargetKindToString( CORE::TJobControlTargetKind kind )
{
    switch ( kind )
    {
        case CORE::JOBCONTROLTARGET_JOB: return "JOB";
        case CORE::JOBCONTROLTARGET_LANE: return "LANE";
        case CORE::JOBCONTROLTARGET_RUN: return "RUN";
        case CORE::JOBCONTROLTARGET_HOST: return "HOST";
        default: return "UNSPECIFIED";
    }
}

/*-------------------------------------------------------------------------*/

static const CORE::SJobStatus*
FindJob( const CORE::CJobSchedulerSnapshot& model, const CORE::CString& jobId )
{
    return model.FindJob( jobId );
}

/*-------------------------------------------------------------------------*/

static const CORE::SJobRunRecord*
FindRun( const CORE::CJobSchedulerSnapshot& model, UInt64 runId )
{
    const CORE::TJobRunRecordVector& runs = model.GetRuns();
    for ( UInt32 i=0; i<runs.size(); ++i )
        if ( runs[ i ].runId == runId )
            return &runs[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CSchedulerHost::CSchedulerHost( CISchedulerSource& source                              ,
                                const CSchedulerHostSettings& settings                 ,
                                const CORE::PulseGeneratorPtr& notificationPulseGenerator )
    : CORE::CTSGNotifier( notificationPulseGenerator, true, false )
    , m_source( &source )
    , m_settings( settings )
    , m_defaultAuditSink()
    , m_auditSink( &m_defaultAuditSink )
    , m_customActionHandler( GUCEF_NULL )
    , m_lock()
    , m_service( GUCEF_NEW COMCORE::CRpcServiceImpl() )
    , m_services()
    , m_contract()
    , m_model( GUCEF_NEW CORE::CJobSchedulerSnapshot() )
    , m_journal( GUCEF_NEW CSchedulerChangeJournal( settings.GetJournalSize() ) )
    , m_commands( GUCEF_NEW CCommandIdempotencyCache() )
    , m_rateLimiter( GUCEF_NEW CControlRateLimiter() )
    , m_tablePublisher( GUCEF_NEW CSchedulerTablePublisher() )
    , m_streams()
    , m_runWaits()
    , m_releasedCalls()
    , m_releaseTimer( GUCEF_NEW CORE::CTimer( notificationPulseGenerator, 10 ) )
    , m_streamTimer( GUCEF_NEW CORE::CTimer( notificationPulseGenerator, 100 ) )
    , m_publishTimer( GUCEF_NEW CORE::CTimer( notificationPulseGenerator, settings.GetPublishIntervalMs() > 0 ? settings.GetPublishIntervalMs() : 1000 ) )
    , m_publishDirty( false )
{GUCEF_TRACE;

    m_commands->SetBounds( m_settings.GetIdempotencyTtlMs(), m_settings.GetMaxCommandsPerSubject(), m_settings.GetMaxCommands() );
    m_rateLimiter->SetMaxPerMinute( m_settings.GetMaxControlCommandsPerMinutePerSubject() );

    typedef COMCORE::CTRpcMethodHandler< CSchedulerHost > THandler;
    m_service->BindMethod( "GetInfo", GUCEF_NEW THandler( this, &CSchedulerHost::OnGetInfo ) );
    m_service->BindMethod( "ListJobs", GUCEF_NEW THandler( this, &CSchedulerHost::OnListJobs ) );
    m_service->BindMethod( "GetJob", GUCEF_NEW THandler( this, &CSchedulerHost::OnGetJob ) );
    m_service->BindMethod( "ListLanes", GUCEF_NEW THandler( this, &CSchedulerHost::OnListLanes ) );
    m_service->BindMethod( "ListRuns", GUCEF_NEW THandler( this, &CSchedulerHost::OnListRuns ) );
    m_service->BindMethod( "Watch", GUCEF_NEW THandler( this, &CSchedulerHost::OnWatch ) );
    m_service->BindMethod( "Control", GUCEF_NEW THandler( this, &CSchedulerHost::OnControl ) );
    m_service->BindMethod( "SubmitJob", GUCEF_NEW THandler( this, &CSchedulerHost::OnSubmitJob ) );

    TEventCallback changed( this, &CSchedulerHost::OnSourceChanged );
    if ( GUCEF_NULL != m_source->GetChangeNotifier() )
        SubscribeTo( m_source->GetChangeNotifier(), CISchedulerSource::ChangedEvent, changed );
    TEventCallback releaseTimer( this, &CSchedulerHost::OnReleaseTimer );
    SubscribeTo( m_releaseTimer, CORE::CTimer::TimerUpdateEvent, releaseTimer );
    TEventCallback streamTimer( this, &CSchedulerHost::OnStreamTimer );
    SubscribeTo( m_streamTimer, CORE::CTimer::TimerUpdateEvent, streamTimer );
    TEventCallback publishTimer( this, &CSchedulerHost::OnPublishTimer );
    SubscribeTo( m_publishTimer, CORE::CTimer::TimerUpdateEvent, publishTimer );

    // the journal covers everything after the first model
    CORE::CJobSchedulerSnapshot first;
    m_source->GetSnapshot( first, 0 );
    SanitizeSnapshot( first );
    *m_model = first;
    m_journal->Reset( m_settings.GetJournalSize(), first.GetRevision() );
}

/*-------------------------------------------------------------------------*/

CSchedulerHost::~CSchedulerHost()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    Withdraw();
    StopPublishing();
    m_streamTimer->SetEnabled( false );
    m_publishTimer->SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_releaseTimer->SetEnabled( false );
    m_releasedCalls.clear();
    GUCEF_DELETE m_releaseTimer;
    GUCEF_DELETE m_streamTimer;
    GUCEF_DELETE m_publishTimer;
    GUCEF_DELETE m_tablePublisher;
    GUCEF_DELETE m_rateLimiter;
    GUCEF_DELETE m_commands;
    GUCEF_DELETE m_journal;
    GUCEF_DELETE m_model;
    GUCEF_DELETE m_service;
}

/*-------------------------------------------------------------------------*/

const CSchedulerHostSettings&
CSchedulerHost::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CISchedulerSource&
CSchedulerHost::GetSource( void )
{GUCEF_TRACE;

    return *m_source;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::SetAuditSink( CIDrnUtilsAuditSink* sink )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_auditSink = GUCEF_NULL != sink ? sink : &m_defaultAuditSink;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::SetCustomActionHandler( CISchedulerCustomActionHandler* handler )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_customActionHandler = handler;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHost::Expose( const COMCORE::CRpcServiceSetPtr& services, CORE::CString& error )
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
    COMCORE::CRpcContractPtr contract = CSchedulerContracts::GetContract();
    if ( contract.IsNULL() )
    {
        error = "the contract gucef.drnutils.scheduler 1.0 could not be built";
        return false;
    }
    COMCORE::CRpcServiceSetPtr serviceSet( services );
    if ( !serviceSet->RegisterService( contract, CSchedulerContracts::ServiceName, m_service, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) )
    {
        error = "the service Scheduler could not be registered";
        return false;
    }
    m_contract = contract;
    m_services = services;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SchedulerHost(" << m_source->GetHostName() << "): exposed, exposure " << (Int32) m_settings.GetExposure() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::Withdraw( void )
{GUCEF_TRACE;

    if ( m_services.IsNULL() )
        return;
    m_services->UnregisterService( m_contract, CSchedulerContracts::ServiceName );
    m_services.Unlink();
    RemoveAllStreams( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the scheduler host was withdrawn" ) );
    CompleteAllRunWaits( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the scheduler host was withdrawn" ) );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHost::IsExposed( void ) const
{GUCEF_TRACE;

    return !m_services.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHost::PublishTables( const CORE::CString& drnSessionName, CORE::CString& error )
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_NONE == m_settings.GetExposure() )
    {
        error = "the settings expose nothing (exposure NONE)";
        return false;
    }
    StopPublishing();
    if ( !m_tablePublisher->Start( drnSessionName, m_settings.GetTablePrefix(), m_source->GetHostName(), error ) )
        return false;
    CORE::CJobSchedulerSnapshot copy;
    {
        MT::CScopeMutex lock( m_lock );
        copy = *m_model;
    }
    m_tablePublisher->Publish( copy, m_settings.GetPublishedRuns() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::StopPublishing( void )
{GUCEF_TRACE;

    m_publishTimer->SetEnabled( false );
    m_publishDirty = false;
    m_tablePublisher->Stop();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::GetTableNames( CORE::CString::StringMap& tableNames ) const
{GUCEF_TRACE;

    tableNames.clear();
    if ( !m_tablePublisher->IsStarted() )
        return;
    tableNames[ "jobs" ] = m_tablePublisher->GetGridName( CSchedulerTablePublisher::TABLE_JOBS );
    tableNames[ "lanes" ] = m_tablePublisher->GetGridName( CSchedulerTablePublisher::TABLE_LANES );
    tableNames[ "runs" ] = m_tablePublisher->GetGridName( CSchedulerTablePublisher::TABLE_RUNS );
}

/*-------------------------------------------------------------------------*/

UInt64
CSchedulerHost::GetMonotonicMs( void ) const
{GUCEF_TRACE;

    return CDrnUtilsHostSupport::GetMonotonicMs();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHost::IsSecure( const COMCORE::CRpcPrincipal& principal ) const
{GUCEF_TRACE;

    return CDrnUtilsHostSupport::IsSecure( principal );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerHost::CheckViewAccess( const COMCORE::CRpcPrincipal& principal, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_NONE == m_settings.GetExposure() )
    {
        reason = "the scheduler host exposes nothing";
        return false;
    }
    if ( !m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerViewRole ) )
    {
        reason = "the role " + CORE::CString( CDrnUtilsRoleMap::SchedulerViewRole ) + " is needed";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::GetDeclaredCustomActions( TSchedulerActionInfoVector& declared ) const
{GUCEF_TRACE;

    declared.clear();
    TSchedulerActionInfoVector fromSource;
    m_source->GetDeclaredActions( fromSource );
    for ( UInt32 i=0; i<fromSource.size(); ++i )
        if ( CORE::JOBCONTROLACTION_CUSTOM == fromSource[ i ].action )
            declared.push_back( fromSource[ i ] );
    CISchedulerCustomActionHandler* handler = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        handler = m_customActionHandler;
    }
    if ( GUCEF_NULL != handler )
    {
        TSchedulerActionInfoVector fromHandler;
        handler->GetCustomActions( fromHandler );
        for ( UInt32 i=0; i<fromHandler.size(); ++i )
        {
            fromHandler[ i ].action = CORE::JOBCONTROLACTION_CUSTOM;
            declared.push_back( fromHandler[ i ] );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::GetAllowedActions( const COMCORE::CRpcPrincipal& principal, const CORE::SJobStatus* job,
                                   const TSchedulerActionInfoVector& declared, TSchedulerActionInfoVector& actions ) const
{GUCEF_TRACE;

    actions.clear();
    if ( DRNUTILS_EXPOSURE_CONTROL != m_settings.GetExposure() )
        return;
    const CDrnUtilsRoleMap& roles = m_settings.GetRoleMap();
    bool hasControl = roles.HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerControlRole );
    bool hasSubmit = roles.HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerSubmitRole );
    static const CORE::TJobControlAction genericActions[ 6 ] = { CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLACTION_CANCEL_RUN,
                                                                 CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLACTION_DISABLE,
                                                                 CORE::JOBCONTROLACTION_SET_PRIORITY, CORE::JOBCONTROLACTION_SET_LANE_MODE };
    const CORE::TJobControlActionSet& allowed = m_settings.GetAllowedActions();
    if ( hasControl )
    {
        for ( UInt32 a=0; a<6; ++a )
        {
            CORE::TJobControlAction action = genericActions[ a ];
            if ( allowed.find( (Int32) action ) == allowed.end() )
                continue;
            CORE::TJobControlTargetKind targetKind = DefaultTargetKind( action );
            if ( GUCEF_NULL != job )
            {
                if ( CORE::JOBCONTROLTARGET_LANE == targetKind || job->remoteActions.find( (Int32) action ) == job->remoteActions.end() )
                    continue;
            }
            if ( CORE::JOBCONTROLACTION_RUN_NOW != action && !CDrnUtilsHostSupport::HasAnyRole( principal.roles, roles.GetControlRolesForAction( action ), true ) )
                continue;
            SSchedulerActionInfo info;
            info.action = action;
            info.targetKind = targetKind;
            info.title = CORE::JobControlActionToString( action );
            actions.push_back( info );
        }
    }
    if ( hasSubmit && !m_settings.GetSubmittableTaskTypes().empty() )
    {
        if ( GUCEF_NULL == job || ( CORE::JOBORIGIN_SUBMITTED == job->origin &&
             m_settings.GetSubmittableTaskTypes().find( job->taskTypeName ) != m_settings.GetSubmittableTaskTypes().end() ) )
        {
            SSchedulerActionInfo info;
            info.action = CORE::JOBCONTROLACTION_REMOVE_JOB;
            info.targetKind = CORE::JOBCONTROLTARGET_JOB;
            info.title = CORE::JobControlActionToString( CORE::JOBCONTROLACTION_REMOVE_JOB );
            actions.push_back( info );
        }
    }
    if ( hasControl )
    {
        TSchedulerActionInfoVector customs( declared );
        if ( GUCEF_NULL != job )
        {
            for ( UInt32 c=0; c<job->customActions.size(); ++c )
            {
                SSchedulerActionInfo info;
                info.action = CORE::JOBCONTROLACTION_CUSTOM;
                info.targetKind = job->customActions[ c ].targetKind;
                info.customName = job->customActions[ c ].name;
                info.title = job->customActions[ c ].title;
                info.argumentNames = job->customActions[ c ].argumentNames;
                info.privileged = job->customActions[ c ].privileged;
                customs.push_back( info );
            }
        }
        CORE::CString::StringSet listed;
        const CORE::CString::StringSet& allowedCustom = m_settings.GetAllowedCustomActions();
        for ( UInt32 c=0; c<customs.size(); ++c )
        {
            const SSchedulerActionInfo& info = customs[ c ];
            if ( allowedCustom.find( info.customName ) == allowedCustom.end() || listed.find( info.customName ) != listed.end() )
                continue;
            if ( GUCEF_NULL != job && ( CORE::JOBCONTROLTARGET_JOB != info.targetKind ||
                                        job->remoteCustomActions.find( info.customName ) == job->remoteCustomActions.end() ) )
                continue;
            if ( info.privileged && !CDrnUtilsHostSupport::HasAnyRole( principal.roles, roles.GetControlRolesForAction( CORE::JOBCONTROLACTION_CUSTOM ), true ) )
                continue;
            listed.insert( info.customName );
            actions.push_back( info );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::GetHostInfo( const COMCORE::CRpcPrincipal& principal, SSchedulerHostInfo& info ) const
{GUCEF_TRACE;

    TSchedulerActionInfoVector declared;
    GetDeclaredCustomActions( declared );
    CORE::CString::StringSet sourceSubmittable;
    m_source->GetSubmittableTaskTypes( sourceSubmittable );

    info = SSchedulerHostInfo();
    info.hostName = m_source->GetHostName();
    info.serverTimeUtcMs = CDrnUtilsHostSupport::GetUtcMs();
    {
        MT::CScopeMutex lock( m_lock );
        info.hostInstanceId = m_model->GetHostInstanceId();
        info.revision = m_model->GetRevision();
        info.state = CORE::JobSchedulerHostStateToString( m_model->GetHostState() );
    }
    GetAllowedActions( principal, GUCEF_NULL, declared, info.allowedActions );
    GetTableNames( info.tableNames );
    if ( DRNUTILS_EXPOSURE_CONTROL == m_settings.GetExposure() &&
         m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerSubmitRole ) )
    {
        const CORE::CString::StringSet& hostSubmittable = m_settings.GetSubmittableTaskTypes();
        CORE::CString::StringSet::const_iterator i = hostSubmittable.begin();
        while ( i != hostSubmittable.end() )
        {
            if ( sourceSubmittable.find( (*i) ) != sourceSubmittable.end() )
                info.submittableTaskTypes.insert( (*i) );
            ++i;
        }
        info.maxSubmitPayloadBytes = m_settings.GetMaxSubmitPayloadBytes();
    }
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CSchedulerHost::CheckControlAccess( const COMCORE::CRpcPrincipal& principal, const CORE::SJobControlCommand& command,
                                    const TSchedulerActionInfoVector& declared, const CORE::CJobSchedulerSnapshot* fresh,
                                    CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !principal.isAuthenticated )
    {
        reason = "control needs an authenticated principal";
        return DRNUTILS_STATUS_PERMISSION_DENIED;
    }
    if ( !m_settings.GetAllowInsecureControl() && !IsSecure( principal ) )
    {
        reason = "control needs a secure transport (" + principal.transport + " is not)";
        return DRNUTILS_STATUS_PERMISSION_DENIED;
    }
    const CDrnUtilsRoleMap& roles = m_settings.GetRoleMap();
    CORE::TJobControlAction action = command.action;

    MT::CScopeMutex lock( m_lock );
    // a fresh copy of the source when the model of the host does not know the target yet (its change is on the way)
    const CORE::CJobSchedulerSnapshot& model = GUCEF_NULL != fresh ? *fresh : *m_model;
    const CORE::SJobStatus* job = GUCEF_NULL;
    if ( CORE::JOBCONTROLTARGET_JOB == command.targetKind )
        job = FindJob( model, command.targetId );
    else
    if ( CORE::JOBCONTROLTARGET_RUN == command.targetKind )
    {
        const CORE::SJobRunRecord* run = FindRun( model, CORE::StringToUInt64( command.targetId ) );
        if ( GUCEF_NULL == run )
        {
            reason = "no run " + command.targetId + " is known";
            return DRNUTILS_STATUS_NOT_FOUND;
        }
        job = FindJob( model, run->jobId );
    }

    if ( CORE::JOBCONTROLACTION_REMOVE_JOB == action )
    {
        if ( !roles.HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerSubmitRole ) )
        {
            reason = "the role " + CORE::CString( CDrnUtilsRoleMap::SchedulerSubmitRole ) + " is needed";
            return DRNUTILS_STATUS_PERMISSION_DENIED;
        }
        if ( GUCEF_NULL == job )
        {
            reason = "no job " + command.targetId + " is known";
            return DRNUTILS_STATUS_NOT_FOUND;
        }
        if ( CORE::JOBORIGIN_SUBMITTED != job->origin )
        {
            reason = "only jobs submitted as data can be removed";
            return DRNUTILS_STATUS_FAILED_PRECONDITION;
        }
        if ( m_settings.GetSubmittableTaskTypes().find( job->taskTypeName ) == m_settings.GetSubmittableTaskTypes().end() )
        {
            reason = "the task type " + job->taskTypeName + " is not submittable on this host";
            return DRNUTILS_STATUS_PERMISSION_DENIED;
        }
        return DRNUTILS_STATUS_OK;
    }

    if ( !roles.HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerControlRole ) )
    {
        reason = "the role " + CORE::CString( CDrnUtilsRoleMap::SchedulerControlRole ) + " is needed";
        return DRNUTILS_STATUS_PERMISSION_DENIED;
    }

    if ( CORE::JOBCONTROLACTION_CUSTOM == action )
    {
        if ( m_settings.GetAllowedCustomActions().find( command.customName ) == m_settings.GetAllowedCustomActions().end() )
        {
            reason = "the custom action " + command.customName + " is not allowed remotely";
            return DRNUTILS_STATUS_PERMISSION_DENIED;
        }
        const SSchedulerActionInfo* info = GUCEF_NULL;
        for ( UInt32 i=0; i<declared.size(); ++i )
            if ( declared[ i ].customName == command.customName )
                info = &declared[ i ];
        SSchedulerActionInfo jobDeclared;
        if ( GUCEF_NULL == info && GUCEF_NULL != job )
        {
            for ( UInt32 c=0; c<job->customActions.size(); ++c )
            {
                if ( job->customActions[ c ].name == command.customName )
                {
                    jobDeclared.customName = job->customActions[ c ].name;
                    jobDeclared.targetKind = job->customActions[ c ].targetKind;
                    jobDeclared.privileged = job->customActions[ c ].privileged;
                    info = &jobDeclared;
                }
            }
        }
        if ( GUCEF_NULL == info )
        {
            reason = "nobody declares the custom action " + command.customName;
            return DRNUTILS_STATUS_UNIMPLEMENTED;
        }
        if ( CORE::JOBCONTROLTARGET_JOB == command.targetKind )
        {
            if ( GUCEF_NULL == job )
            {
                reason = "no job " + command.targetId + " is known";
                return DRNUTILS_STATUS_NOT_FOUND;
            }
            if ( job->remoteCustomActions.find( command.customName ) == job->remoteCustomActions.end() )
            {
                reason = "the custom action " + command.customName + " is not allowed on job " + job->jobId;
                return DRNUTILS_STATUS_PERMISSION_DENIED;
            }
        }
        if ( info->privileged && !CDrnUtilsHostSupport::HasAnyRole( principal.roles, roles.GetControlRolesForAction( CORE::JOBCONTROLACTION_CUSTOM ), true ) )
        {
            reason = "the privileged custom action " + command.customName + " needs one of the roles of its refinement";
            return DRNUTILS_STATUS_PERMISSION_DENIED;
        }
        return DRNUTILS_STATUS_OK;
    }

    if ( action < CORE::JOBCONTROLACTION_RUN_NOW || action > CORE::JOBCONTROLACTION_SET_LANE_MODE )
    {
        reason = "the action " + CORE::ToString( (Int32) action ) + " is not supported in this version";
        return DRNUTILS_STATUS_UNIMPLEMENTED;
    }
    if ( m_settings.GetAllowedActions().find( (Int32) action ) == m_settings.GetAllowedActions().end() )
    {
        reason = CORE::CString( "the action " ) + CORE::JobControlActionToString( action ) + " is not allowed remotely on this host";
        return DRNUTILS_STATUS_PERMISSION_DENIED;
    }
    if ( CORE::JOBCONTROLTARGET_JOB == command.targetKind || CORE::JOBCONTROLTARGET_RUN == command.targetKind )
    {
        if ( GUCEF_NULL == job )
        {
            reason = "no job " + command.targetId + " is known";
            return DRNUTILS_STATUS_NOT_FOUND;
        }
        if ( job->remoteActions.find( (Int32) action ) == job->remoteActions.end() )
        {
            reason = CORE::CString( "the action " ) + CORE::JobControlActionToString( action ) + " is not allowed on job " + job->jobId;
            return DRNUTILS_STATUS_PERMISSION_DENIED;
        }
    }
    bool refined = CORE::JOBCONTROLACTION_RUN_NOW != action || ( command.hasPriority && 0 == command.priority );
    if ( refined && !CDrnUtilsHostSupport::HasAnyRole( principal.roles, roles.GetControlRolesForAction( action ), true ) )
    {
        reason = CORE::CString( "the action " ) + CORE::JobControlActionToString( action ) + " needs one of the roles of its refinement";
        return DRNUTILS_STATUS_PERMISSION_DENIED;
    }
    return DRNUTILS_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::Audit( const SDrnUtilsAuditRecord& record )
{GUCEF_TRACE;

    CIDrnUtilsAuditSink* sink = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        sink = m_auditSink;
    }
    // outside every lock of the host: a sink may call back into the host
    if ( GUCEF_NULL != sink )
        sink->OnAuditRecord( record );
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CSchedulerHost::Control( const COMCORE::CRpcPrincipal& principal       ,
                         const CORE::CString& commandId                ,
                         const CORE::SJobControlCommand& command       ,
                         CORE::SJobControlResult& result               ,
                         bool& replayed                                )
{GUCEF_TRACE;

    replayed = false;
    result = CORE::SJobControlResult();

    SDrnUtilsAuditRecord record;
    record.timeUtcMs = CDrnUtilsHostSupport::GetUtcMs();
    record.hostName = m_source->GetHostName();
    record.subject = principal.subject;
    record.transport = principal.transport;
    record.sessionId = principal.attributes.GetValueAlways( CDrnUtilsHostSupport::SessionIdAttribute ).AsString();
    record.contract = CSchedulerContracts::ContractName;
    record.action = CORE::JobControlActionToString( command.action );
    record.targetKind = TargetKindToString( command.targetKind );
    record.targetId = command.targetId;
    record.customName = command.customName;
    record.argumentsDigest = CDrnUtilsDigest::ArgumentsDigest( command.arguments );
    record.reason = command.reason;
    record.commandId = commandId;
    record.dryRun = command.dryRun;

    TSchedulerActionInfoVector declared;
    GetDeclaredCustomActions( declared );
    bool known = true;
    {
        MT::CScopeMutex lock( m_lock );
        if ( CORE::JOBCONTROLTARGET_JOB == command.targetKind )
            known = GUCEF_NULL != FindJob( *m_model, command.targetId );
        else
        if ( CORE::JOBCONTROLTARGET_RUN == command.targetKind )
            known = GUCEF_NULL != FindRun( *m_model, CORE::StringToUInt64( command.targetId ) );
    }
    CORE::CJobSchedulerSnapshot fresh;
    if ( !known )
    {
        // taken with no lock of the host held
        m_source->GetSnapshot( fresh, 0 );
        SanitizeSnapshot( fresh );
    }
    CORE::CString refusal;
    TDrnUtilsCommandStatus status = CheckControlAccess( principal, command, declared, known ? GUCEF_NULL : &fresh, refusal );
    if ( DRNUTILS_STATUS_OK == status )
        status = CDrnUtilsHostSupport::CheckCommandId( commandId, command.dryRun, refusal );
    UInt64 nowMs = GetMonotonicMs();
    if ( DRNUTILS_STATUS_OK == status && !command.dryRun )
    {
        TDrnUtilsCommandStatus storedStatus = DRNUTILS_STATUS_OK;
        if ( m_commands->Find( principal.subject, commandId, nowMs, storedStatus, result ) )
        {
            replayed = true;
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
        result.messages.push_back( refusal );
        record.outcome = DrnUtilsCommandStatusToString( status );
        record.reason = refusal;
        Audit( record );
        return status;
    }

    CORE::SJobControlCommand applied( command );
    applied.requestedBy = principal.subject;
    if ( CORE::JOBCONTROLTARGET_UNSPECIFIED == applied.targetKind )
        applied.targetKind = DefaultTargetKind( applied.action );

    CISchedulerCustomActionHandler* handler = GUCEF_NULL;
    bool byHandler = false;
    if ( CORE::JOBCONTROLACTION_CUSTOM == applied.action )
    {
        MT::CScopeMutex lock( m_lock );
        handler = m_customActionHandler;
    }
    if ( GUCEF_NULL != handler )
    {
        TSchedulerActionInfoVector handlerActions;
        handler->GetCustomActions( handlerActions );
        for ( UInt32 i=0; i<handlerActions.size(); ++i )
            if ( handlerActions[ i ].customName == applied.customName )
                byHandler = true;
    }
    CORE::TJobControlStatus applyStatus = CORE::JOBCONTROLSTATUS_OK;
    if ( byHandler )
    {
        CORE::CString message;
        applyStatus = handler->ApplyCustomAction( applied, result.detail, message );
        if ( !message.IsNULLOrEmpty() )
            result.messages.push_back( message );
    }
    else
    {
        // on the caller's thread with no lock of the host held (rule 9.1)
        applyStatus = m_source->Apply( applied, result );
    }
    status = (TDrnUtilsCommandStatus) applyStatus;
    if ( !command.dryRun )
        m_commands->Store( principal.subject, commandId, GetMonotonicMs(), status, result );
    record.outcome = DrnUtilsCommandStatusToString( status );
    if ( DRNUTILS_STATUS_OK != status && !result.messages.empty() )
        record.reason = result.messages[ 0 ];
    Audit( record );
    return status;
}

/*-------------------------------------------------------------------------*/

TDrnUtilsCommandStatus
CSchedulerHost::SubmitJob( const COMCORE::CRpcPrincipal& principal     ,
                           const CORE::CString& commandId              ,
                           const CORE::SJobSubmission& submission      ,
                           CORE::SJobControlResult& result             ,
                           bool& replayed                              )
{GUCEF_TRACE;

    replayed = false;
    result = CORE::SJobControlResult();
    const CORE::CJobDefinition& definition = submission.definition;

    SDrnUtilsAuditRecord record;
    record.timeUtcMs = CDrnUtilsHostSupport::GetUtcMs();
    record.hostName = m_source->GetHostName();
    record.subject = principal.subject;
    record.transport = principal.transport;
    record.sessionId = principal.attributes.GetValueAlways( CDrnUtilsHostSupport::SessionIdAttribute ).AsString();
    record.contract = CSchedulerContracts::ContractName;
    record.action = "SUBMIT_JOB";
    record.targetKind = "JOB";
    record.targetId = definition.GetId();
    record.taskTypeName = definition.GetTaskTypeName();
    UInt32 payloadBytes = 0;
    record.argumentsDigest = CDrnUtilsDigest::DataNodeDigest( definition.GetPayload(), payloadBytes );
    record.reason = submission.reason;
    record.commandId = commandId;
    record.dryRun = submission.dryRun;

    CORE::CString refusal;
    TDrnUtilsCommandStatus status = DRNUTILS_STATUS_OK;
    CORE::CString::StringSet sourceSubmittable;
    m_source->GetSubmittableTaskTypes( sourceSubmittable );
    if ( !principal.isAuthenticated )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "submission needs an authenticated principal";
    }
    else
    if ( !m_settings.GetAllowInsecureControl() && !IsSecure( principal ) )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "submission needs a secure transport (" + principal.transport + " is not)";
    }
    else
    if ( !m_settings.GetRoleMap().HasRole( principal.roles, CDrnUtilsRoleMap::SchedulerSubmitRole ) )
    {
        status = DRNUTILS_STATUS_PERMISSION_DENIED;
        refusal = "the role " + CORE::CString( CDrnUtilsRoleMap::SchedulerSubmitRole ) + " is needed";
    }
    else
    if ( m_settings.GetSubmittableTaskTypes().find( definition.GetTaskTypeName() ) == m_settings.GetSubmittableTaskTypes().end() ||
         sourceSubmittable.find( definition.GetTaskTypeName() ) == sourceSubmittable.end() )
    {
        status = DRNUTILS_STATUS_FAILED_PRECONDITION;
        refusal = "the task type \"" + definition.GetTaskTypeName() + "\" is not submittable on this host";
    }
    else
    if ( m_settings.GetMaxSubmitPayloadBytes() > 0 && payloadBytes > m_settings.GetMaxSubmitPayloadBytes() )
    {
        status = DRNUTILS_STATUS_RESOURCE_EXHAUSTED;
        refusal = "the payload of " + CORE::ToString( payloadBytes ) + " bytes is above the bound of " + CORE::ToString( m_settings.GetMaxSubmitPayloadBytes() );
    }
    if ( DRNUTILS_STATUS_OK == status )
        status = CDrnUtilsHostSupport::CheckCommandId( commandId, submission.dryRun, refusal );
    UInt64 nowMs = GetMonotonicMs();
    if ( DRNUTILS_STATUS_OK == status && !submission.dryRun )
    {
        TDrnUtilsCommandStatus storedStatus = DRNUTILS_STATUS_OK;
        if ( m_commands->Find( principal.subject, commandId, nowMs, storedStatus, result ) )
        {
            replayed = true;
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
        result.messages.push_back( refusal );
        record.outcome = DrnUtilsCommandStatusToString( status );
        record.reason = refusal;
        Audit( record );
        return status;
    }

    CORE::SJobSubmission applied( submission );
    applied.submittedBy = principal.subject;
    status = (TDrnUtilsCommandStatus) m_source->Submit( applied, result );
    if ( !submission.dryRun )
        m_commands->Store( principal.subject, commandId, GetMonotonicMs(), status, result );
    record.outcome = DrnUtilsCommandStatusToString( status );
    if ( DRNUTILS_STATUS_OK != status && !result.messages.empty() )
        record.reason = result.messages[ 0 ];
    Audit( record );
    return status;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::SanitizeSnapshot( CORE::CJobSchedulerSnapshot& snapshot ) const
{GUCEF_TRACE;

    // hidden jobs and their runs never leave the process; sensitive notes and messages are redacted; item keys of
    // work sources are private
    CORE::CString::StringSet hidden;
    CORE::CString::StringSet sensitive;
    CORE::TJobStatusVector& jobs = snapshot.GetJobs();
    for ( UInt32 i=0; i<jobs.size(); )
    {
        if ( 0 != ( jobs[ i ].flags & CORE::JOBFLAG_HIDDEN ) )
        {
            hidden.insert( jobs[ i ].jobId );
            jobs.erase( jobs.begin() + i );
            continue;
        }
        if ( 0 != ( jobs[ i ].flags & CORE::JOBFLAG_SENSITIVE ) )
            sensitive.insert( jobs[ i ].jobId );
        if ( 0 == ( jobs[ i ].flags & CORE::JOBFLAG_EXPOSE_PAYLOAD ) )
            jobs[ i ].payload.Clear();
        ++i;
    }
    CORE::TJobRunRecordVector& runs = snapshot.GetRuns();
    for ( UInt32 i=0; i<runs.size(); )
    {
        if ( hidden.find( runs[ i ].jobId ) != hidden.end() )
        {
            runs.erase( runs.begin() + i );
            continue;
        }
        runs[ i ].itemKey.Clear();
        if ( sensitive.find( runs[ i ].jobId ) != sensitive.end() )
        {
            if ( !runs[ i ].note.IsNULLOrEmpty() )
                runs[ i ].note = CSchedulerRowCodec::RedactedText;
            if ( !runs[ i ].outcomeMessage.IsNULLOrEmpty() )
                runs[ i ].outcomeMessage = CSchedulerRowCodec::RedactedText;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnSourceChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshModel();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::RefreshModel( void )
{GUCEF_TRACE;

    // the copy is taken with no lock of the host held
    CORE::CJobSchedulerSnapshot fresh;
    m_source->GetSnapshot( fresh, 0 );
    SanitizeSnapshot( fresh );

    TChangeVector changes;
    bool reset = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( fresh.GetHostInstanceId() != m_model->GetHostInstanceId() || fresh.GetRevision() < m_model->GetRevision() )
        {
            reset = true;
            m_journal->Reset( m_settings.GetJournalSize(), fresh.GetRevision() );
        }
        else
        {
            typedef GUCEF::map< CORE::CString, UInt64 > TRevisionMap;
            TRevisionMap old;
            const CORE::TJobStatusVector& oldJobs = m_model->GetJobs();
            for ( UInt32 i=0; i<oldJobs.size(); ++i )
                old[ oldJobs[ i ].jobId ] = oldJobs[ i ].revision;
            const CORE::TJobStatusVector& jobs = fresh.GetJobs();
            for ( UInt32 i=0; i<jobs.size(); ++i )
            {
                TRevisionMap::iterator found = old.find( jobs[ i ].jobId );
                if ( found == old.end() || (*found).second != jobs[ i ].revision )
                {
                    SChange change = { CSchedulerChangeJournal::KIND_JOB, jobs[ i ].jobId, jobs[ i ].revision, false };
                    changes.push_back( change );
                }
                if ( found != old.end() )
                    old.erase( found );
            }
            for ( TRevisionMap::iterator i=old.begin(); i!=old.end(); ++i )
            {
                SChange change = { CSchedulerChangeJournal::KIND_JOB, (*i).first, fresh.GetRevision(), false };
                changes.push_back( change );
            }

            old.clear();
            const CORE::TJobLaneStatusVector& oldLanes = m_model->GetLanes();
            for ( UInt32 i=0; i<oldLanes.size(); ++i )
                old[ oldLanes[ i ].laneName ] = oldLanes[ i ].revision;
            const CORE::TJobLaneStatusVector& lanes = fresh.GetLanes();
            for ( UInt32 i=0; i<lanes.size(); ++i )
            {
                TRevisionMap::iterator found = old.find( lanes[ i ].laneName );
                if ( found == old.end() || (*found).second != lanes[ i ].revision )
                {
                    SChange change = { CSchedulerChangeJournal::KIND_LANE, lanes[ i ].laneName, lanes[ i ].revision, false };
                    changes.push_back( change );
                }
                if ( found != old.end() )
                    old.erase( found );
            }
            for ( TRevisionMap::iterator i=old.begin(); i!=old.end(); ++i )
            {
                SChange change = { CSchedulerChangeJournal::KIND_LANE, (*i).first, fresh.GetRevision(), false };
                changes.push_back( change );
            }

            typedef GUCEF::map< UInt64, UInt64 > TRunRevisionMap;
            TRunRevisionMap oldRuns;
            const CORE::TJobRunRecordVector& previousRuns = m_model->GetRuns();
            for ( UInt32 i=0; i<previousRuns.size(); ++i )
                oldRuns[ previousRuns[ i ].runId ] = previousRuns[ i ].revision;
            const CORE::TJobRunRecordVector& runs = fresh.GetRuns();
            for ( UInt32 i=0; i<runs.size(); ++i )
            {
                TRunRevisionMap::iterator found = oldRuns.find( runs[ i ].runId );
                if ( found == oldRuns.end() || (*found).second != runs[ i ].revision )
                {
                    SChange change = { CSchedulerChangeJournal::KIND_RUN, CORE::ToString( runs[ i ].runId ), runs[ i ].revision,
                                       CORE::JobRunStateIsAnEndState( runs[ i ].state ) };
                    changes.push_back( change );
                }
            }
            if ( fresh.GetHostState() != m_model->GetHostState() )
            {
                SChange change = { CHANGEKIND_HOST, fresh.GetHostName(), fresh.GetRevision(), false };
                changes.push_back( change );
            }
            for ( UInt32 i=0; i<changes.size(); ++i )
                if ( CHANGEKIND_HOST != changes[ i ].kind )
                    m_journal->Add( changes[ i ].revision, changes[ i ].kind, changes[ i ].key );
        }
        *m_model = fresh;
    }

    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CSchedulerWatchStream& stream = *m_streams[ s ];
        if ( reset )
        {
            stream.needsSnapshot = true;
            stream.pending.clear();
        }
        else
        {
            for ( UInt32 c=0; c<changes.size(); ++c )
            {
                if ( CSchedulerChangeJournal::KIND_RUN == changes[ c ].kind && !stream.includeRuns )
                    continue;
                stream.AddPending( changes[ c ].kind, changes[ c ].key, changes[ c ].revision );
            }
            UInt32 maxPending = m_settings.GetMaxPendingPerStream();
            if ( maxPending > 0 && stream.pending.size() > maxPending )
            {
                // a viewer that falls this far behind starts over from a snapshot: bounded memory for any viewer
                stream.pending.clear();
                stream.outbox.clear();
                stream.outboxPosition = 0;
                stream.needsSnapshot = true;
            }
        }
        FlushStream( stream );
    }

    CompleteRunWaits( changes );
    if ( m_tablePublisher->IsStarted() )
    {
        m_publishDirty = true;
        StartPublishTimerIfNeeded();
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::StartPublishTimerIfNeeded( void )
{GUCEF_TRACE;

    if ( m_publishDirty && !m_publishTimer->GetEnabled() )
        m_publishTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnPublishTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_publishTimer->SetEnabled( false );
    if ( !m_publishDirty || !m_tablePublisher->IsStarted() )
        return;
    m_publishDirty = false;
    CORE::CJobSchedulerSnapshot copy;
    {
        MT::CScopeMutex lock( m_lock );
        copy = *m_model;
    }
    // the update scope takes the node lock: no lock of the host is held here
    m_tablePublisher->Publish( copy, m_settings.GetPublishedRuns() );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::BuildJobRow( const CORE::SJobStatus& job, const COMCORE::CRpcPrincipal& principal,
                             const TSchedulerActionInfoVector& declared, COMCORE::CRpcMessage& row ) const
{GUCEF_TRACE;

    CSchedulerRowCodec::JobToMessage( job, row );
    TSchedulerActionInfoVector actions;
    GetAllowedActions( principal, &job, declared, actions );
    for ( UInt32 a=0; a<actions.size(); ++a )
    {
        COMCORE::CRpcMessage* action = row.AddMessage( CSchedulerRowCodec::JOBROW_ALLOWEDACTIONS );
        if ( GUCEF_NULL != action )
            CSchedulerRowCodec::ActionToMessage( actions[ a ], *action );
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::BuildStreamMessages( CSchedulerWatchStream& stream, const TSchedulerActionInfoVector& declared )
{GUCEF_TRACE;

    // under the host lock: rows are read from the model (copies go into the messages)
    const CORE::CJobSchedulerSnapshot& model = *m_model;
    if ( stream.needsSnapshot )
    {
        stream.needsSnapshot = false;
        stream.pending.clear();
        COMCORE::CRpcMessage reset = stream.call->NewResponse();
        reset.SetEnum( CHANGE_KIND, CHANGEKIND_RESET );
        reset.SetUInt64( CHANGE_REVISION, model.GetRevision() );
        reset.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
        stream.outbox.push_back( reset );
        const CORE::TJobStatusVector& jobs = model.GetJobs();
        for ( UInt32 i=0; i<jobs.size(); ++i )
        {
            COMCORE::CRpcMessage change = stream.call->NewResponse();
            change.SetEnum( CHANGE_KIND, CHANGEKIND_JOB );
            change.SetUInt64( CHANGE_REVISION, jobs[ i ].revision );
            change.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
            BuildJobRow( jobs[ i ], stream.principal, declared, *change.GetMutableMessage( CHANGE_JOB ) );
            stream.outbox.push_back( change );
        }
        const CORE::TJobLaneStatusVector& lanes = model.GetLanes();
        for ( UInt32 i=0; i<lanes.size(); ++i )
        {
            COMCORE::CRpcMessage change = stream.call->NewResponse();
            change.SetEnum( CHANGE_KIND, CHANGEKIND_LANE );
            change.SetUInt64( CHANGE_REVISION, lanes[ i ].revision );
            change.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
            CSchedulerRowCodec::LaneToMessage( lanes[ i ], *change.GetMutableMessage( CHANGE_LANE ) );
            stream.outbox.push_back( change );
        }
        if ( stream.includeRuns )
        {
            const CORE::TJobRunRecordVector& runs = model.GetRuns();
            for ( UInt32 i=0; i<runs.size(); ++i )
            {
                COMCORE::CRpcMessage change = stream.call->NewResponse();
                change.SetEnum( CHANGE_KIND, CHANGEKIND_RUN );
                change.SetUInt64( CHANGE_REVISION, runs[ i ].revision );
                change.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
                CSchedulerRowCodec::RunToMessage( runs[ i ], *change.GetMutableMessage( CHANGE_RUN ) );
                stream.outbox.push_back( change );
            }
        }
        COMCORE::CRpcMessage end = stream.call->NewResponse();
        end.SetEnum( CHANGE_KIND, CHANGEKIND_SNAPSHOT_END );
        end.SetUInt64( CHANGE_REVISION, model.GetRevision() );
        end.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
        stream.outbox.push_back( end );
        return;
    }

    // pending changes in revision order, the rows as they are now (latest wins)
    typedef GUCEF::map< UInt64, GUCEF::vector< const CSchedulerWatchStream::SPending* > > TByRevision;
    TByRevision ordered;
    CSchedulerWatchStream::TPendingMap::const_iterator p = stream.pending.begin();
    while ( p != stream.pending.end() )
    {
        ordered[ (*p).second.revision ].push_back( &(*p).second );
        ++p;
    }
    for ( TByRevision::iterator r = ordered.begin(); r != ordered.end(); ++r )
    {
        for ( UInt32 i=0; i<(*r).second.size(); ++i )
        {
            const CSchedulerWatchStream::SPending& pending = *(*r).second[ i ];
            COMCORE::CRpcMessage change = stream.call->NewResponse();
            change.SetEnum( CHANGE_KIND, pending.kind );
            change.SetUInt64( CHANGE_REVISION, pending.revision );
            change.SetUInt64( CHANGE_INSTANCEID, model.GetHostInstanceId() );
            if ( CSchedulerChangeJournal::KIND_JOB == pending.kind )
            {
                const CORE::SJobStatus* job = model.FindJob( pending.key );
                if ( GUCEF_NULL != job )
                    BuildJobRow( *job, stream.principal, declared, *change.GetMutableMessage( CHANGE_JOB ) );
                else
                    change.SetString( CHANGE_REMOVEDKEY, pending.key );
            }
            else
            if ( CSchedulerChangeJournal::KIND_LANE == pending.kind )
            {
                const CORE::SJobLaneStatus* lane = model.FindLane( pending.key );
                if ( GUCEF_NULL != lane )
                    CSchedulerRowCodec::LaneToMessage( *lane, *change.GetMutableMessage( CHANGE_LANE ) );
                else
                    change.SetString( CHANGE_REMOVEDKEY, pending.key );
            }
            else
            if ( CSchedulerChangeJournal::KIND_RUN == pending.kind )
            {
                const CORE::SJobRunRecord* run = FindRun( model, CORE::StringToUInt64( pending.key ) );
                if ( GUCEF_NULL == run )
                    continue;   // aged out of the history: nothing to tell
                CSchedulerRowCodec::RunToMessage( *run, *change.GetMutableMessage( CHANGE_RUN ) );
            }
            stream.outbox.push_back( change );
        }
    }
    stream.pending.clear();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::FlushStream( CSchedulerWatchStream& stream )
{GUCEF_TRACE;

    bool haveDeclared = false;
    TSchedulerActionInfoVector declared;
    while ( true )
    {
        while ( stream.outboxPosition < stream.outbox.size() )
        {
            if ( stream.maxMessagesPerSecond > 0 )
            {
                UInt64 nowMs = GetMonotonicMs();
                if ( nowMs - stream.windowStartMs >= 1000 )
                {
                    stream.windowStartMs = nowMs;
                    stream.sentInWindow = 0;
                }
                if ( stream.sentInWindow >= stream.maxMessagesPerSecond )
                {
                    stream.throttled = true;
                    if ( !m_streamTimer->GetEnabled() )
                        m_streamTimer->SetEnabled( true );
                    return;
                }
            }
            // no lock of the host is held while writing (rule 9.1); without credit WritableEvent resumes the flush
            if ( !stream.call->Write( stream.outbox[ stream.outboxPosition ] ) )
                return;
            ++stream.outboxPosition;
            ++stream.sentInWindow;
            ++stream.messagesWritten;
        }
        stream.outbox.clear();
        stream.outboxPosition = 0;
        if ( !stream.needsSnapshot && stream.pending.empty() )
            return;
        if ( !haveDeclared )
        {
            GetDeclaredCustomActions( declared );
            haveDeclared = true;
        }
        MT::CScopeMutex lock( m_lock );
        BuildStreamMessages( stream, declared );
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnCallWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        if ( static_cast< CORE::CNotifier* >( m_streams[ s ]->call.GetPointerAlways() ) == notifier )
        {
            if ( !m_streams[ s ]->throttled )
                FlushStream( *m_streams[ s ] );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnCallCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RemoveStream( notifier );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnStreamTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool anyThrottled = false;
    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CSchedulerWatchStream& stream = *m_streams[ s ];
        if ( !stream.throttled )
            continue;
        stream.throttled = false;
        FlushStream( stream );
        anyThrottled = anyThrottled || stream.throttled;
    }
    if ( !anyThrottled )
        m_streamTimer->SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::RemoveStream( CORE::CNotifier* callNotifier )
{GUCEF_TRACE;

    for ( UInt32 s=0; s<m_streams.size(); ++s )
    {
        CSchedulerWatchStream* stream = m_streams[ s ];
        if ( static_cast< CORE::CNotifier* >( stream->call.GetPointerAlways() ) == callNotifier )
        {
            m_streams.erase( m_streams.begin() + s );
            UnsubscribeFrom( callNotifier );
            // the last reference of the call goes at the next pulse, on the notification thread
            ReleaseLater( stream->call );
            GUCEF_DELETE stream;
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::RemoveAllStreams( const COMCORE::CRpcStatus& status )
{GUCEF_TRACE;

    TWatchStreamVector streams;
    streams.swap( m_streams );
    for ( UInt32 s=0; s<streams.size(); ++s )
    {
        UnsubscribeFrom( streams[ s ]->call.GetPointerAlways() );
        streams[ s ]->call->Finish( status );
        GUCEF_DELETE streams[ s ];
    }
    m_streamTimer->SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::FillOutcome( const CORE::SJobControlResult& result, bool replayed, COMCORE::CRpcMessage& outcome ) const
{GUCEF_TRACE;

    outcome.SetBool( OUTCOME_REPLAYED, replayed );
    outcome.SetUInt64( OUTCOME_REVISION, result.revision );
    outcome.SetUInt64( OUTCOME_RUNID, result.runId );
    for ( UInt32 i=0; i<result.messages.size(); ++i )
        outcome.Add( OUTCOME_MESSAGES, CORE::CVariant( CSchedulerRowCodec::Bound( result.messages[ i ], CSchedulerRowCodec::MaxTextBytes ) ) );
    if ( result.hasRun )
        CSchedulerRowCodec::RunToMessage( result.run, *outcome.GetMutableMessage( OUTCOME_RUN ) );
    if ( result.detail.GetNrOfChildNodes() > 0 || result.detail.GetAttCount() > 0 || result.detail.GetValue().IsInitialized() )
        outcome.SetDataNode( OUTCOME_DETAIL, result.detail );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::CompleteRunWaits( const TChangeVector& changes )
{GUCEF_TRACE;

    if ( m_runWaits.empty() )
        return;
    for ( UInt32 c=0; c<changes.size(); ++c )
    {
        if ( CSchedulerChangeJournal::KIND_RUN != changes[ c ].kind || !changes[ c ].endedRun )
            continue;
        UInt64 runId = CORE::StringToUInt64( changes[ c ].key );
        TRunWaitMap::iterator found = m_runWaits.find( runId );
        if ( found == m_runWaits.end() )
            continue;
        TServerCallVector calls;
        calls.swap( (*found).second );
        m_runWaits.erase( found );

        CORE::SJobControlResult result;
        {
            MT::CScopeMutex lock( m_lock );
            const CORE::SJobRunRecord* run = FindRun( *m_model, runId );
            if ( GUCEF_NULL != run )
            {
                result.hasRun = true;
                result.run = *run;
            }
            result.revision = m_model->GetRevision();
        }
        result.runId = runId;
        for ( UInt32 i=0; i<calls.size(); ++i )
        {
            UnsubscribeFrom( calls[ i ].GetPointerAlways() );
            COMCORE::CRpcMessage outcome = calls[ i ]->NewResponse();
            FillOutcome( result, false, outcome );
            calls[ i ]->CompleteResult( outcome );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::CompleteAllRunWaits( const COMCORE::CRpcStatus& status )
{GUCEF_TRACE;

    TRunWaitMap waits;
    waits.swap( m_runWaits );
    for ( TRunWaitMap::iterator i = waits.begin(); i != waits.end(); ++i )
    {
        for ( UInt32 c=0; c<(*i).second.size(); ++c )
        {
            UnsubscribeFrom( (*i).second[ c ].GetPointerAlways() );
            (*i).second[ c ]->CompleteResult( status );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnRunWaitCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    for ( TRunWaitMap::iterator i = m_runWaits.begin(); i != m_runWaits.end(); ++i )
    {
        TServerCallVector& calls = (*i).second;
        for ( UInt32 c=0; c<calls.size(); ++c )
        {
            if ( static_cast< CORE::CNotifier* >( calls[ c ].GetPointerAlways() ) == notifier )
            {
                UnsubscribeFrom( notifier );
                ReleaseLater( calls[ c ] );
                calls.erase( calls.begin() + c );
                if ( calls.empty() )
                    m_runWaits.erase( i );
                return;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnGetInfo( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    SSchedulerHostInfo info;
    GetHostInfo( call.GetPrincipal(), info );
    COMCORE::CRpcMessage response = call.NewResponse();
    response.SetString( HOSTINFO_HOSTNAME, info.hostName );
    response.SetUInt64( HOSTINFO_INSTANCEID, info.hostInstanceId );
    response.SetUInt64( HOSTINFO_REVISION, info.revision );
    response.SetTimestamp( HOSTINFO_SERVERTIME, info.serverTimeUtcMs * 1000 );
    response.SetString( HOSTINFO_STATE, info.state );
    for ( UInt32 a=0; a<info.allowedActions.size(); ++a )
        CSchedulerRowCodec::ActionToMessage( info.allowedActions[ a ], *response.AddMessage( HOSTINFO_ALLOWEDACTIONS ) );
    CORE::CString::StringMap::const_iterator t = info.tableNames.begin();
    while ( t != info.tableNames.end() )
    {
        response.SetMapEntry( HOSTINFO_TABLENAMES, CORE::CVariant( (*t).first ), CORE::CVariant( (*t).second ) );
        ++t;
    }
    CORE::CString::StringSet::const_iterator s = info.submittableTaskTypes.begin();
    while ( s != info.submittableTaskTypes.end() )
    {
        response.Add( HOSTINFO_SUBMITTABLE, CORE::CVariant( (*s) ) );
        ++s;
    }
    response.SetUInt32( HOSTINFO_MAXPAYLOAD, info.maxSubmitPayloadBytes );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnListJobs( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString lane = request.GetString( LIST_LANE );
    UInt32 maxRows = request.GetUInt32( LIST_MAXROWS );
    maxRows = 0 == maxRows ? DefaultMaxRows : ( maxRows > MaxMaxRows ? MaxMaxRows : maxRows );
    CORE::CString after = request.GetString( LIST_PAGETOKEN );
    bool includeEnded = request.GetBool( LIST_INCLUDEENDED );

    TSchedulerActionInfoVector declared;
    GetDeclaredCustomActions( declared );
    COMCORE::CRpcMessage response = call.NewResponse();
    {
        MT::CScopeMutex lock( m_lock );
        // jobs in id order; the page token is the last id of the previous page
        typedef GUCEF::map< CORE::CString, const CORE::SJobStatus* > TJobIndex;
        TJobIndex index;
        const CORE::TJobStatusVector& jobs = m_model->GetJobs();
        for ( UInt32 i=0; i<jobs.size(); ++i )
        {
            if ( !lane.IsNULLOrEmpty() && jobs[ i ].laneName != lane )
                continue;
            if ( !includeEnded && CORE::JOBSTATE_ENDED == jobs[ i ].state )
                continue;
            if ( !after.IsNULLOrEmpty() && !( after < jobs[ i ].jobId ) )
                continue;
            index[ jobs[ i ].jobId ] = &jobs[ i ];
        }
        UInt32 count = 0;
        TJobIndex::iterator i = index.begin();
        while ( i != index.end() && count < maxRows )
        {
            BuildJobRow( *(*i).second, call.GetPrincipal(), declared, *response.AddMessage( JOBLIST_JOBS ) );
            ++count;
            ++i;
        }
        if ( i != index.end() && count > 0 )
            response.SetString( JOBLIST_NEXTPAGE, (*(--i)).first );
        response.SetUInt64( JOBLIST_REVISION, m_model->GetRevision() );
    }
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnGetJob( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString jobId = request.GetString( GETJOB_JOBID );
    UInt32 recentRuns = request.GetUInt32( GETJOB_RECENTRUNS );
    recentRuns = 0 == recentRuns ? DefaultRecentRuns : ( recentRuns > MaxRecentRuns ? MaxRecentRuns : recentRuns );
    bool includeSourceStatus = request.GetBool( GETJOB_SOURCESTATUS );

    UInt32 flags = 0;
    {
        MT::CScopeMutex lock( m_lock );
        const CORE::SJobStatus* job = m_model->FindJob( jobId );
        if ( GUCEF_NULL == job )
        {
            // hidden jobs are not in the model: they are not found either
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_NOT_FOUND, "no job " + jobId ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        flags = job->flags;
    }
    SSchedulerJobDetail detail;
    if ( !m_source->GetJobDetail( jobId, recentRuns, includeSourceStatus, detail ) || 0 != ( detail.job.flags & CORE::JOBFLAG_HIDDEN ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_NOT_FOUND, "no job " + jobId ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    flags |= detail.job.flags;
    TSchedulerActionInfoVector declared;
    GetDeclaredCustomActions( declared );
    COMCORE::CRpcMessage response = call.NewResponse();
    BuildJobRow( detail.job, call.GetPrincipal(), declared, *response.GetMutableMessage( DETAIL_JOB ) );
    for ( UInt32 r=0; r<detail.recentRuns.size() && r<recentRuns; ++r )
    {
        CORE::SJobRunRecord run = detail.recentRuns[ r ];
        run.itemKey.Clear();
        if ( 0 != ( flags & CORE::JOBFLAG_SENSITIVE ) )
        {
            if ( !run.note.IsNULLOrEmpty() )
                run.note = CSchedulerRowCodec::RedactedText;
            if ( !run.outcomeMessage.IsNULLOrEmpty() )
                run.outcomeMessage = CSchedulerRowCodec::RedactedText;
        }
        CSchedulerRowCodec::RunToMessage( run, *response.AddMessage( DETAIL_RECENTRUNS ) );
    }
    if ( includeSourceStatus && ( detail.sourceStatus.GetNrOfChildNodes() > 0 || detail.sourceStatus.GetAttCount() > 0 ) )
        response.SetDataNode( DETAIL_SOURCESTATUS, detail.sourceStatus );
    if ( !detail.job.description.IsNULLOrEmpty() )
        response.SetString( DETAIL_DESCRIPTION, CSchedulerRowCodec::Bound( detail.job.description, 4096 ) );
    if ( 0 != ( flags & CORE::JOBFLAG_EXPOSE_PAYLOAD ) && ( detail.job.payload.GetNrOfChildNodes() > 0 || detail.job.payload.GetAttCount() > 0 ) )
        response.SetDataNode( DETAIL_PAYLOAD, detail.job.payload );
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnListLanes( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CORE::CString lane = call.GetRequest().GetString( LIST_LANE );
    COMCORE::CRpcMessage response = call.NewResponse();
    {
        MT::CScopeMutex lock( m_lock );
        const CORE::TJobLaneStatusVector& lanes = m_model->GetLanes();
        for ( UInt32 i=0; i<lanes.size(); ++i )
        {
            if ( lane.IsNULLOrEmpty() || lane == lanes[ i ].laneName )
                CSchedulerRowCodec::LaneToMessage( lanes[ i ], *response.AddMessage( LANELIST_LANES ) );
        }
        response.SetUInt64( LANELIST_REVISION, m_model->GetRevision() );
    }
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnListRuns( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString jobId = request.GetString( LIST_JOBID );
    CORE::CString lane = request.GetString( LIST_LANE );
    UInt32 maxRows = request.GetUInt32( LIST_MAXROWS );
    maxRows = 0 == maxRows ? DefaultMaxRows : ( maxRows > MaxMaxRows ? MaxMaxRows : maxRows );
    CORE::CString pageToken = request.GetString( LIST_PAGETOKEN );
    UInt64 before = pageToken.IsNULLOrEmpty() ? 0 : CORE::StringToUInt64( pageToken );
    bool includeEnded = request.GetBool( LIST_INCLUDEENDED );

    COMCORE::CRpcMessage response = call.NewResponse();
    {
        MT::CScopeMutex lock( m_lock );
        // newest first: the runs by id descending; the page token is the last id of the previous page
        typedef GUCEF::map< UInt64, const CORE::SJobRunRecord* > TRunIndex;
        TRunIndex index;
        const CORE::TJobRunRecordVector& runs = m_model->GetRuns();
        for ( UInt32 i=0; i<runs.size(); ++i )
        {
            const CORE::SJobRunRecord& run = runs[ i ];
            if ( !jobId.IsNULLOrEmpty() && run.jobId != jobId )
                continue;
            if ( !lane.IsNULLOrEmpty() )
            {
                const CORE::SJobStatus* job = m_model->FindJob( run.jobId );
                if ( GUCEF_NULL == job || job->laneName != lane )
                    continue;
            }
            if ( !includeEnded && CORE::JobRunStateIsAnEndState( run.state ) )
                continue;
            if ( 0 != before && run.runId >= before )
                continue;
            index[ run.runId ] = &run;
        }
        UInt32 count = 0;
        UInt64 lastId = 0;
        TRunIndex::reverse_iterator i = index.rbegin();
        while ( i != index.rend() && count < maxRows )
        {
            CSchedulerRowCodec::RunToMessage( *(*i).second, *response.AddMessage( RUNLIST_RUNS ) );
            lastId = (*i).first;
            ++count;
            ++i;
        }
        if ( i != index.rend() )
            response.SetString( RUNLIST_NEXTPAGE, CORE::ToString( lastId ) );
        response.SetUInt64( RUNLIST_REVISION, m_model->GetRevision() );
    }
    call.Reply( response );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnWatch( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !CheckViewAccess( call.GetPrincipal(), reason ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, reason ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    CORE::CString sessionKey = CDrnUtilsHostSupport::GetSessionKey( call );
    UInt32 maxPerHost = m_settings.GetMaxWatchStreamsPerHost();
    UInt32 maxPerSession = m_settings.GetMaxWatchStreamsPerSession();
    UInt32 inSession = 0;
    for ( UInt32 s=0; s<m_streams.size(); ++s )
        if ( m_streams[ s ]->sessionKey == sessionKey )
            ++inSession;
    if ( ( maxPerHost > 0 && m_streams.size() >= maxPerHost ) || ( maxPerSession > 0 && inSession >= maxPerSession ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, "the limit of watch streams was reached" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }

    const COMCORE::CRpcMessage& request = call.GetRequest();
    CSchedulerWatchStream* stream = GUCEF_NEW CSchedulerWatchStream();
    stream->call = call.GetSharedPtr();
    stream->principal = call.GetPrincipal();
    stream->sessionKey = sessionKey;
    stream->includeRuns = request.GetBool( WATCH_INCLUDERUNS );
    stream->maxMessagesPerSecond = request.GetUInt32( WATCH_MAXPERSECOND );
    stream->windowStartMs = GetMonotonicMs();

    UInt64 hostInstanceId = request.GetUInt64( WATCH_INSTANCEID );
    UInt64 afterRevision = request.GetUInt64( WATCH_AFTERREVISION );
    {
        MT::CScopeMutex lock( m_lock );
        CSchedulerChangeJournal::TRecordVector records;
        if ( 0 != hostInstanceId && hostInstanceId == m_model->GetHostInstanceId() && afterRevision <= m_model->GetRevision() &&
             m_journal->Collect( afterRevision, records ) )
        {
            // resumed from the host revision: only what changed after it
            for ( UInt32 r=0; r<records.size(); ++r )
            {
                if ( CSchedulerChangeJournal::KIND_RUN == records[ r ].kind && !stream->includeRuns )
                    continue;
                stream->AddPending( records[ r ].kind, records[ r ].key, records[ r ].revision );
            }
        }
        else
        {
            stream->needsSnapshot = true;
        }
    }
    m_streams.push_back( stream );

    TEventCallback writable( this, &CSchedulerHost::OnCallWritable );
    TEventCallback cancelled( this, &CSchedulerHost::OnCallCancelled );
    SubscribeTo( stream->call.GetPointerAlways(), COMCORE::CRpcServerCall::WritableEvent, writable );
    SubscribeTo( stream->call.GetPointerAlways(), COMCORE::CRpcServerCall::CancelledEvent, cancelled );
    FlushStream( *stream );
    return COMCORE::RPC_HANDLER_DEFERRED;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnControl( COMCORE::CRpcServerCall& call )
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
    CORE::SJobControlCommand command;
    command.action = (CORE::TJobControlAction) request.GetEnum( CONTROL_ACTION );
    command.targetKind = (CORE::TJobControlTargetKind) request.GetEnum( CONTROL_TARGETKIND );
    command.targetId = request.GetString( CONTROL_TARGETID );
    command.customName = request.GetString( CONTROL_CUSTOMNAME );
    COMCORE::CRpcMessage::TVariantVector keys;
    request.GetMapKeys( CONTROL_ARGUMENTS, keys );
    for ( UInt32 k=0; k<keys.size(); ++k )
    {
        CORE::CVariant value;
        request.GetMapEntry( CONTROL_ARGUMENTS, keys[ k ], value );
        command.arguments[ keys[ k ].AsString() ] = value.AsString();
    }
    command.hasPriority = request.Has( CONTROL_PRIORITY );
    command.priority = request.GetUInt32( CONTROL_PRIORITY );
    command.mode = request.GetString( CONTROL_MODE );
    command.reason = request.GetString( CONTROL_REASON );
    command.expectedRevision = request.GetUInt64( CONTROL_EXPECTEDREVISION );
    command.dryRun = request.GetBool( CONTROL_DRYRUN );
    bool waitForRun = request.GetBool( CONTROL_WAITFORRUN );

    COMCORE::CRpcPrincipal principal( call.GetPrincipal() );
    principal.attributes.Set( CDrnUtilsHostSupport::SessionIdAttribute, CORE::CVariant( CDrnUtilsHostSupport::GetSessionKey( call ) ) );
    CORE::SJobControlResult result;
    bool replayed = false;
    TDrnUtilsCommandStatus status = Control( principal, commandId, command, result, replayed );
    if ( DRNUTILS_STATUS_OK != status )
    {
        call.Fail( COMCORE::CRpcStatus( CDrnUtilsHostSupport::ToRpcStatusCode( status ), CDrnUtilsHostSupport::JoinMessages( result.messages ) ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    if ( waitForRun && CORE::JOBCONTROLACTION_RUN_NOW == command.action && !command.dryRun && !replayed && 0 != result.runId )
    {
        // ACCEPTED now with request id = run id, the RESULT when the run ended (6.4.2)
        if ( !call.Accept( COMCORE::CRpcRequestId( result.runId ), false, 0 ) )
            return COMCORE::RPC_HANDLER_DONE;
        m_runWaits[ result.runId ].push_back( call.GetSharedPtr() );
        TEventCallback cancelled( this, &CSchedulerHost::OnRunWaitCancelled );
        SubscribeTo( &call, COMCORE::CRpcServerCall::CancelledEvent, cancelled );

        // the run may have ended already (a source that raises its change on this thread)
        bool ended = false;
        {
            MT::CScopeMutex lock( m_lock );
            const CORE::SJobRunRecord* run = FindRun( *m_model, result.runId );
            ended = GUCEF_NULL != run && CORE::JobRunStateIsAnEndState( run->state );
        }
        if ( ended )
        {
            TChangeVector changes;
            SChange change = { CSchedulerChangeJournal::KIND_RUN, CORE::ToString( result.runId ), 0, true };
            changes.push_back( change );
            CompleteRunWaits( changes );
        }
        return COMCORE::RPC_HANDLER_DEFERRED;
    }
    COMCORE::CRpcMessage outcome = call.NewResponse();
    FillOutcome( result, replayed, outcome );
    call.Reply( outcome );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcHandlerResult
CSchedulerHost::OnSubmitJob( COMCORE::CRpcServerCall& call )
{GUCEF_TRACE;

    if ( DRNUTILS_EXPOSURE_CONTROL != m_settings.GetExposure() )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_PERMISSION_DENIED, "submission is not exposed by this host" ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    const COMCORE::CRpcMessage& request = call.GetRequest();
    CORE::CString commandId;
    CORE::CString problem;
    if ( !CDrnUtilsHostSupport::GetCommandId( call, request.GetString( SUBMIT_COMMANDID ), commandId, problem ) )
    {
        call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, problem ) );
        return COMCORE::RPC_HANDLER_DONE;
    }

    CORE::SJobSubmission submission;
    CORE::CJobDefinition& definition = submission.definition;
    definition.SetId( request.GetString( SUBMIT_JOBID ) );
    definition.SetTitle( request.GetString( SUBMIT_TITLE ) );
    definition.SetDescription( request.GetString( SUBMIT_DESCRIPTION ) );
    definition.SetTaskTypeName( request.GetString( SUBMIT_TASKTYPE ) );
    CORE::CDataNode payload;
    request.GetDataNode( SUBMIT_PAYLOAD, payload );
    definition.SetPayload( payload );
    Int32 target = request.GetEnum( SUBMIT_EXECUTIONTARGET );
    definition.SetExecutionTarget( 0 == target ? CORE::JOBEXECUTIONTARGET_POOL : (CORE::TJobExecutionTarget) target );
    definition.SetThreadPoolName( request.GetString( SUBMIT_THREADPOOL ) );
    definition.SetLaneName( request.GetString( SUBMIT_LANE ) );
    if ( request.Has( SUBMIT_PRIORITY ) )
        definition.SetPriority( request.GetUInt32( SUBMIT_PRIORITY ) );
    submission.replaceExisting = request.GetBool( SUBMIT_REPLACE );
    submission.dryRun = request.GetBool( SUBMIT_DRYRUN );
    submission.reason = request.GetString( SUBMIT_REASON );

    CORE::CJobTrigger trigger;
    const COMCORE::CRpcMessage* spec = request.GetMessage( SUBMIT_TRIGGER );
    if ( GUCEF_NULL != spec )
    {
        trigger.SetKind( (CORE::TJobTriggerKind) spec->GetEnum( TRIGGER_KIND ) );
        if ( CORE::JOBTRIGGER_SCHEDULE == trigger.GetKind() )
        {
            CORE::CCronSchedule cron;
            if ( !cron.FromCronString( spec->GetString( TRIGGER_CRON ) ) )
            {
                call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_INVALID_ARGUMENT, "the cron expression of the trigger is not valid" ) );
                return COMCORE::RPC_HANDLER_DONE;
            }
            CORE::CSchedule schedule;
            schedule.SetCronSchedule( cron );
            if ( spec->Has( TRIGGER_RANGESTART ) || spec->Has( TRIGGER_RANGEEND ) )
            {
                CORE::CDateTimeRange range;
                if ( spec->Has( TRIGGER_RANGESTART ) )
                {
                    CORE::CDateTime start;
                    start.FromUnixEpochBasedTicksInMillisecs( (UInt64) ( spec->GetTimestamp( TRIGGER_RANGESTART ) / 1000 ) );
                    range.SetStart( start );
                }
                if ( spec->Has( TRIGGER_RANGEEND ) )
                {
                    CORE::CDateTime end;
                    end.FromUnixEpochBasedTicksInMillisecs( (UInt64) ( spec->GetTimestamp( TRIGGER_RANGEEND ) / 1000 ) );
                    range.SetEnd( end );
                }
                schedule.SetTimeRange( range );
            }
            trigger.SetSchedule( schedule );
        }
        trigger.SetPeriodMs( spec->GetUInt64( TRIGGER_PERIODMS ) );
        Int32 intervalFrom = spec->GetEnum( TRIGGER_INTERVALFROM );
        if ( 0 != intervalFrom )
            trigger.SetIntervalFrom( (CORE::TJobIntervalFrom) intervalFrom );
        if ( spec->Has( TRIGGER_ALIGNTO ) )
        {
            trigger.SetHasAlignment( true );
            trigger.SetAlignToUtcMs( spec->GetTimestamp( TRIGGER_ALIGNTO ) / 1000 );
        }
        trigger.SetOnceAtUtcMs( spec->GetTimestamp( TRIGGER_ONCEAT ) / 1000 );
    }
    else
    {
        trigger.SetKind( CORE::JOBTRIGGER_MANUAL );
    }
    definition.SetTrigger( trigger );

    COMCORE::CRpcPrincipal principal( call.GetPrincipal() );
    principal.attributes.Set( CDrnUtilsHostSupport::SessionIdAttribute, CORE::CVariant( CDrnUtilsHostSupport::GetSessionKey( call ) ) );
    CORE::SJobControlResult result;
    bool replayed = false;
    TDrnUtilsCommandStatus status = SubmitJob( principal, commandId, submission, result, replayed );
    if ( DRNUTILS_STATUS_OK != status )
    {
        call.Fail( COMCORE::CRpcStatus( CDrnUtilsHostSupport::ToRpcStatusCode( status ), CDrnUtilsHostSupport::JoinMessages( result.messages ) ) );
        return COMCORE::RPC_HANDLER_DONE;
    }
    COMCORE::CRpcMessage outcome = call.NewResponse();
    FillOutcome( result, replayed, outcome );
    call.Reply( outcome );
    return COMCORE::RPC_HANDLER_DONE;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::ReleaseLater( const COMCORE::CRpcServerCallPtr& call )
{GUCEF_TRACE;

    // the reference may be the last one, and this may run inside an event the call raises: the call would go while
    // it is still delivering. It goes at the next pulse instead, on this same (notification) thread
    m_releasedCalls.push_back( call );
    if ( !m_releaseTimer->GetEnabled() )
        m_releaseTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerHost::OnReleaseTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_releaseTimer->SetEnabled( false );
    GUCEF::vector< COMCORE::CRpcServerCallPtr > released;
    released.swap( m_releasedCalls );
    released.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerHost::GetClassTypeName( void ) const
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
