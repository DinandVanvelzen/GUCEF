/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_COM_CMCPJOBWAITER_H
#include "gucefCOM_CMcpJobWaiter.h"
#define GUCEF_COM_CMCPJOBWAITER_H
#endif /* GUCEF_COM_CMCPJOBWAITER_H ? */

#include "gucefCOM_CMcpJobBridge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 HandleBytes = 16;
const UInt32 MaxRequestKeyLength = 64;
const char* const SchedulerJobIdPrefix = "mcp.job.";

CORE::CString
RequestKeyCommandId( const CORE::CString& kind, const CORE::CString& requestKey )
{GUCEF_TRACE;

    return kind + "\n" + requestKey;
}

void
SaveRun( const CORE::SJobRunRecord& run, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "state", (Int32) run.state );
    node.SetAttribute( "queuedUtcMs", run.queuedUtcMs );
    node.SetAttribute( "startedUtcMs", run.startedUtcMs );
    node.SetAttribute( "endedUtcMs", run.endedUtcMs );
    node.SetAttribute( "progressDone", run.progressDone );
    node.SetAttribute( "progressTotal", run.progressTotal );
    node.SetAttribute( "note", run.note );
    node.SetAttribute( "outcomeMessage", run.outcomeMessage );
    node.SetAttribute( "resultDocument", run.resultDocument );
    node.SetAttribute( "requestedBy", run.requestedBy );
}

void
LoadRun( const CORE::CDataNode& node, CORE::SJobRunRecord& run )
{GUCEF_TRACE;

    run.Clear();
    run.state = (CORE::TJobRunState) node.GetAttributeValueOrChildValueByName( "state" ).AsInt32( 0, true );
    run.queuedUtcMs = node.GetAttributeValueOrChildValueByName( "queuedUtcMs" ).AsInt64( 0, true );
    run.startedUtcMs = node.GetAttributeValueOrChildValueByName( "startedUtcMs" ).AsInt64( 0, true );
    run.endedUtcMs = node.GetAttributeValueOrChildValueByName( "endedUtcMs" ).AsInt64( 0, true );
    run.progressDone = node.GetAttributeValueOrChildValueByName( "progressDone" ).AsUInt64( 0, true );
    run.progressTotal = node.GetAttributeValueOrChildValueByName( "progressTotal" ).AsUInt64( 0, true );
    run.note = node.GetAttributeValueOrChildValueByName( "note" ).AsString( CORE::CString::Empty, true );
    run.outcomeMessage = node.GetAttributeValueOrChildValueByName( "outcomeMessage" ).AsString( CORE::CString::Empty, true );
    run.resultDocument = node.GetAttributeValueOrChildValueByName( "resultDocument" ).AsString( CORE::CString::Empty, true );
    run.requestedBy = node.GetAttributeValueOrChildValueByName( "requestedBy" ).AsString( CORE::CString::Empty, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpJobBridge::CMcpJobBridge( const CORE::JobSchedulerPtr& scheduler )
    : m_scheduler( scheduler )
    , m_lock()
    , m_settings()
    , m_systemUtcClock()
    , m_utcClock( GUCEF_NULL )
    , m_monotonicClock( GUCEF_NULL )
    , m_jobs()
    , m_expired()
    , m_nextSequence( 1 )
    , m_requestKeys()
    , m_waiters()
{GUCEF_TRACE;

    m_requestKeys.SetBounds( m_settings.requestKeyTtlMs, m_settings.maxRequestKeysPerPrincipal, m_settings.maxRequestKeysTotal );
}

/*-------------------------------------------------------------------------*/

CMcpJobBridge::~CMcpJobBridge()
{GUCEF_TRACE;

    // calls that still wait get the job as it is now, so that their clients have the handle
    TWaiterVector waiters;
    {
        MT::CScopeMutex lock( m_lock );
        waiters.swap( m_waiters );
    }
    for ( size_t i=0; i<waiters.size(); ++i )
    {
        CMcpJobWaiter* waiter = waiters[ i ];
        if ( waiter->IsDone() )
        {
            waiter->Abandon( GUCEF_NULL );
        }
        else
        {
            CMcpJobInfo job;
            bool found = false;
            {
                MT::CScopeMutex lock( m_lock );
                TJobMap::iterator j = m_jobs.find( waiter->GetJobId() );
                if ( j != m_jobs.end() )
                {
                    job = (*j).second.info;
                    found = true;
                }
            }
            CMcpToolResult result;
            if ( found )
                BuildDefaultResult( job, result );
            else
                BuildErrorResult( "job-unknown", "The job " + waiter->GetJobId() + " is no longer known: the server stops its job bridge", false, result );
            waiter->Abandon( &result );
        }
        GUCEF_DELETE waiter;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::JobSchedulerPtr&
CMcpJobBridge::GetScheduler( void ) const
{GUCEF_TRACE;

    return m_scheduler;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::SetSettings( const CMcpJobBridgeSettings& settings )
{GUCEF_TRACE;

    if ( 0 == settings.pollIntervalMs || settings.defaultWaitMs > settings.maxWaitMs || 0 == settings.maxActivePerPrincipal ||
         0 == settings.maxJobs || 0 == settings.resultTtlMs )
        return false;
    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
    m_requestKeys.SetBounds( m_settings.requestKeyTtlMs, m_settings.maxRequestKeysPerPrincipal, m_settings.maxRequestKeysTotal );
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpJobBridgeSettings
CMcpJobBridge::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::SetClocks( const CORE::CIUtcClock* utcClock, const CORE::CIMonotonicClock* monotonicClock )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_utcClock = utcClock;
    m_monotonicClock = monotonicClock;
}

/*-------------------------------------------------------------------------*/

Int64
CMcpJobBridge::NowUtcMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_utcClock ? m_utcClock->GetUtcTimeInMs() : m_systemUtcClock.GetUtcTimeInMs();
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpJobBridge::NowMonotonicMs( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_monotonicClock ? m_monotonicClock->GetMonotonicTimeInMs() : CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::MakeHandle( CORE::CString& handle, CORE::CString& schedulerJobId )
{GUCEF_TRACE;

    UInt8 bytes[ HandleBytes ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, HandleBytes ) )
        return false;
    if ( !CORE::Base64UrlEncode( bytes, HandleBytes, handle, false ) )
        return false;
    static const char* hexDigits = "0123456789abcdef";
    schedulerJobId = SchedulerJobIdPrefix;
    for ( UInt32 i=0; i<HandleBytes; ++i )
    {
        schedulerJobId += CORE::CString( hexDigits + ( bytes[ i ] >> 4 ), 1 );
        schedulerJobId += CORE::CString( hexDigits + ( bytes[ i ] & 0xF ), 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::MaySee( const CMcpPrincipal& principal, const CORE::CString& subject ) const
{GUCEF_TRACE;

    if ( principal.subject == subject )
        return true;
    return !m_settings.operatorScope.IsNULLOrEmpty() && principal.HasScope( m_settings.operatorScope );
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::RefreshLocked( SJobEntry& entry, Int64 nowUtcMs, CORE::CString::StringVector& jobsToRemove )
{GUCEF_TRACE;

    CMcpJobInfo& info = entry.info;
    if ( info.IsFinal() )
        return;

    if ( m_scheduler.IsNULL() || entry.hostInstanceId != m_scheduler->GetHostInstanceId() )
    {
        // the run belonged to an earlier process of the server
        info.state = CMcpJobInfo::StateInterrupted;
        if ( info.run.outcomeMessage.IsNULLOrEmpty() )
            info.run.outcomeMessage = "the server restarted before the run ended";
    }
    else
    {
        CORE::SJobRunRecord run;
        if ( m_scheduler->GetRun( entry.runId, run ) )
        {
            info.run = run;
            info.hasRun = true;
            info.state = CMcpJobInfo::StateOfRun( run.state );
        }
        else
        {
            info.state = CMcpJobInfo::StateFailed;
            info.run.outcomeMessage = "the scheduler no longer knows the run of the job";
            if ( 0 == info.run.endedUtcMs )
                info.run.endedUtcMs = nowUtcMs;
        }
    }

    if ( info.IsFinal() )
    {
        info.expiresUtcMs = nowUtcMs + (Int64) m_settings.resultTtlMs;
        if ( !entry.schedulerJobId.IsNULLOrEmpty() )
        {
            jobsToRemove.push_back( entry.schedulerJobId );
            entry.schedulerJobId.Clear();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::ExpireLocked( Int64 nowUtcMs, CORE::CString::StringVector& jobsToRemove )
{GUCEF_TRACE;

    TJobMap::iterator j = m_jobs.begin();
    while ( j != m_jobs.end() )
    {
        SJobEntry& entry = (*j).second;
        RefreshLocked( entry, nowUtcMs, jobsToRemove );
        if ( entry.info.IsFinal() && entry.info.expiresUtcMs > 0 && entry.info.expiresUtcMs <= nowUtcMs )
        {
            m_expired[ (*j).first ] = entry.info.subject;
            m_jobs.erase( j++ );
            continue;
        }
        ++j;
    }

    // bounds: the oldest final handles go first; the expired list is kept as long
    while ( m_jobs.size() > m_settings.maxJobs )
    {
        TJobMap::iterator oldest = m_jobs.end();
        for ( TJobMap::iterator i = m_jobs.begin(); i != m_jobs.end(); ++i )
        {
            if ( (*i).second.info.IsFinal() && ( oldest == m_jobs.end() || (*i).second.sequence < (*oldest).second.sequence ) )
                oldest = i;
        }
        if ( oldest == m_jobs.end() )
            break;
        m_expired[ (*oldest).first ] = (*oldest).second.info.subject;
        m_jobs.erase( oldest );
    }
    while ( m_expired.size() > m_settings.maxJobs )
        m_expired.erase( m_expired.begin() );
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::RemoveSchedulerJobs( const CORE::CString::StringVector& jobsToRemove )
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return;
    for ( size_t i=0; i<jobsToRemove.size(); ++i )
        m_scheduler->RemoveJob( jobsToRemove[ i ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpJobBridge::CountActiveLocked( const CORE::CString& subject ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
        if ( (*j).second.info.subject == subject && !(*j).second.info.IsFinal() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::Expire( void )
{GUCEF_TRACE;

    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        ExpireLocked( NowUtcMs(), removals );
    }
    RemoveSchedulerJobs( removals );
    PruneWaiters();
}

/*-------------------------------------------------------------------------*/

CMcpJobBridge::EStartResult
CMcpJobBridge::StartJob( const CMcpPrincipal& principal          ,
                         const CORE::CString& kind               ,
                         const CORE::CJobDefinition& definition  ,
                         const CORE::CString& requestKey         ,
                         const CORE::CString& argumentsDigest    ,
                         CMcpJobInfo& job                        ,
                         CORE::CString& error                    )
{GUCEF_TRACE;

    error.Clear();
    if ( m_scheduler.IsNULL() )
    {
        error = "there is no job scheduler";
        return JOBSTART_REFUSED;
    }
    if ( requestKey.Length() > MaxRequestKeyLength )
    {
        error = "the request key is longer than " + CORE::ToString( MaxRequestKeyLength ) + " characters";
        return JOBSTART_REFUSED;
    }

    // one start at a time: a repeat of the same request key never starts twice, also when the repeats overlap
    CORE::CString::StringVector removals;
    EStartResult outcome = JOBSTART_REFUSED;
    {
        MT::CScopeMutex lock( m_lock );
        const Int64 nowUtcMs = NowUtcMs();
        ExpireLocked( nowUtcMs, removals );

        bool decided = false;
        if ( !requestKey.IsNULLOrEmpty() )
        {
            CORE::CDataNode stored;
            CORE::TCommandIdempotencyLookup lookup = m_requestKeys.Find( principal.subject, RequestKeyCommandId( kind, requestKey ), argumentsDigest, NowMonotonicMs(), stored );
            if ( CORE::COMMANDIDEMPOTENCY_CONFLICT == lookup )
            {
                error = "the request key was used before with other arguments";
                outcome = JOBSTART_KEY_CONFLICT;
                decided = true;
            }
            else
            if ( CORE::COMMANDIDEMPOTENCY_HIT == lookup )
            {
                TJobMap::iterator j = m_jobs.find( stored.GetAttributeValueOrChildValueByName( "jobId" ).AsString() );
                if ( j != m_jobs.end() )
                {
                    job = (*j).second.info;
                    outcome = JOBSTART_REPEATED;
                    decided = true;
                }
            }
        }

        if ( !decided && CountActiveLocked( principal.subject ) >= m_settings.maxActivePerPrincipal )
        {
            error = "there are " + CORE::ToString( m_settings.maxActivePerPrincipal ) + " jobs of yours that did not end yet";
            outcome = JOBSTART_BUSY;
            decided = true;
        }

        if ( !decided )
        {
            CORE::CString handle;
            CORE::CString schedulerJobId;
            if ( !MakeHandle( handle, schedulerJobId ) )
            {
                error = "no random bytes for a job handle";
            }
            else
            {
                CORE::SJobSubmission submission;
                submission.definition = definition;
                submission.definition.SetId( schedulerJobId );
                submission.definition.SetTrigger( CORE::CJobTrigger::Manual() );
                submission.submittedBy = principal.subject;
                submission.replaceExisting = false;
                submission.reason = "MCP job " + kind;
                CORE::SJobControlResult submitted;
                CORE::TJobControlStatus status = m_scheduler->SubmitJob( submission, submitted );
                if ( CORE::JOBCONTROLSTATUS_OK != status )
                {
                    error = "the job scheduler refused the job";
                    for ( size_t m=0; m<submitted.messages.size(); ++m )
                        error += ": " + submitted.messages[ m ];
                }
                else
                {
                    CORE::SJobControlCommand command;
                    command.action = CORE::JOBCONTROLACTION_RUN_NOW;
                    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
                    command.targetId = schedulerJobId;
                    command.requestedBy = principal.subject;
                    command.reason = submission.reason;
                    CORE::SJobControlResult started;
                    status = m_scheduler->Control( command, started );
                    if ( CORE::JOBCONTROLSTATUS_OK != status || 0 == started.runId )
                    {
                        error = "the job scheduler did not start the run";
                        for ( size_t m=0; m<started.messages.size(); ++m )
                            error += ": " + started.messages[ m ];
                        removals.push_back( schedulerJobId );
                    }
                    else
                    {
                        SJobEntry& entry = m_jobs[ handle ];
                        entry.info.jobId = handle;
                        entry.info.kind = kind;
                        entry.info.subject = principal.subject;
                        entry.info.createdUtcMs = nowUtcMs;
                        entry.hostInstanceId = m_scheduler->GetHostInstanceId();
                        entry.runId = started.runId;
                        entry.schedulerJobId = schedulerJobId;
                        entry.sequence = m_nextSequence++;
                        if ( started.hasRun )
                        {
                            entry.info.run = started.run;
                            entry.info.hasRun = true;
                            entry.info.state = CMcpJobInfo::StateOfRun( started.run.state );
                        }
                        RefreshLocked( entry, nowUtcMs, removals );
                        job = entry.info;
                        if ( !requestKey.IsNULLOrEmpty() )
                        {
                            CORE::CDataNode stored( "outcome", GUCEF_DATATYPE_OBJECT );
                            stored.SetAttribute( "jobId", handle );
                            m_requestKeys.Store( principal.subject, RequestKeyCommandId( kind, requestKey ), argumentsDigest, NowMonotonicMs(), stored );
                        }
                        outcome = JOBSTART_STARTED;
                    }
                }
            }
        }
    }
    RemoveSchedulerJobs( removals );
    if ( JOBSTART_STARTED == outcome )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpJobBridge: Job " << job.jobId << " of kind " << kind << " started for " << principal.subject );
    }
    else
    if ( JOBSTART_REFUSED == outcome )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpJobBridge: A job of kind " << kind << " for " << principal.subject << " did not start: " << error );
    }
    return outcome;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::AdoptRun( const CMcpPrincipal& principal, const CORE::CString& kind, UInt64 runId, CMcpJobInfo& job )
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return false;
    CORE::SJobRunRecord run;
    if ( !m_scheduler->GetRun( runId, run ) )
        return false;
    CORE::CString handle;
    CORE::CString schedulerJobId;
    if ( !MakeHandle( handle, schedulerJobId ) )
        return false;

    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        const Int64 nowUtcMs = NowUtcMs();
        SJobEntry& entry = m_jobs[ handle ];
        entry.info.jobId = handle;
        entry.info.kind = kind;
        entry.info.subject = principal.subject;
        entry.info.createdUtcMs = nowUtcMs;
        entry.info.run = run;
        entry.info.hasRun = true;
        entry.info.state = CMcpJobInfo::StateOfRun( run.state );
        entry.hostInstanceId = m_scheduler->GetHostInstanceId();
        entry.runId = runId;
        entry.sequence = m_nextSequence++;
        if ( entry.info.IsFinal() )
            entry.info.expiresUtcMs = nowUtcMs + (Int64) m_settings.resultTtlMs;
        job = entry.info;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpJobBridge::ELookup
CMcpJobBridge::GetJob( const CMcpPrincipal& principal, const CORE::CString& jobId, CMcpJobInfo& job )
{GUCEF_TRACE;

    ELookup lookup = JOB_UNKNOWN;
    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        ExpireLocked( NowUtcMs(), removals );
        TJobMap::iterator j = m_jobs.find( jobId );
        if ( j != m_jobs.end() )
        {
            if ( MaySee( principal, (*j).second.info.subject ) )
            {
                job = (*j).second.info;
                lookup = JOB_FOUND;
            }
        }
        else
        {
            TStringMap::const_iterator e = m_expired.find( jobId );
            if ( e != m_expired.end() && MaySee( principal, (*e).second ) )
                lookup = JOB_EXPIRED;
        }
    }
    RemoveSchedulerJobs( removals );
    return lookup;
}

/*-------------------------------------------------------------------------*/

CMcpJobBridge::ELookup
CMcpJobBridge::CancelJob( const CMcpPrincipal& principal, const CORE::CString& jobId, CMcpJobInfo& job, CORE::CString& error )
{GUCEF_TRACE;

    error.Clear();
    ELookup lookup = GetJob( principal, jobId, job );
    if ( JOB_FOUND != lookup || job.IsFinal() || m_scheduler.IsNULL() )
        return lookup;

    UInt64 runId = 0;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator j = m_jobs.find( jobId );
        if ( j == m_jobs.end() )
            return JOB_UNKNOWN;
        runId = (*j).second.runId;
    }

    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
    command.targetKind = CORE::JOBCONTROLTARGET_RUN;
    command.targetId = CORE::ToString( runId );
    command.requestedBy = principal.subject;
    command.reason = "cancelled through MCP";
    CORE::SJobControlResult result;
    if ( CORE::JOBCONTROLSTATUS_OK != m_scheduler->Control( command, result ) )
    {
        for ( size_t m=0; m<result.messages.size(); ++m )
            error += ( error.IsNULLOrEmpty() ? "" : ": " ) + result.messages[ m ];
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpJobBridge: Job " << jobId << " is asked to cancel by " << principal.subject );
    }
    GetJob( principal, jobId, job );
    return JOB_FOUND;
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::ListJobs( const CMcpPrincipal& principal, TMcpJobInfoVector& jobs, const CORE::CString& kindFilter, const CORE::CString& stateFilter )
{GUCEF_TRACE;

    jobs.clear();
    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        ExpireLocked( NowUtcMs(), removals );
        GUCEF::map< UInt64, const CMcpJobInfo* > newestFirst;
        for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
        {
            const CMcpJobInfo& info = (*j).second.info;
            if ( !MaySee( principal, info.subject ) )
                continue;
            if ( !kindFilter.IsNULLOrEmpty() && kindFilter != info.kind )
                continue;
            if ( !stateFilter.IsNULLOrEmpty() && stateFilter != info.state )
                continue;
            newestFirst[ (*j).second.sequence ] = &info;
        }
        GUCEF::map< UInt64, const CMcpJobInfo* >::const_reverse_iterator i = newestFirst.rbegin();
        while ( i != newestFirst.rend() )
        {
            jobs.push_back( *(*i).second );
            ++i;
        }
    }
    RemoveSchedulerJobs( removals );
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::BuildDefaultResult( const CMcpJobInfo& job, CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CDataNode jobObject;
    job.ToDataNode( jobObject );
    result.SetStructuredContent( jobObject, true );
    if ( !job.IsFinal() )
        result.AddText( "The job " + job.jobId + " is " + job.state + ": ask for its status with its jobId until its state is final" );
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::BuildErrorResult( const CORE::CString& code, const CORE::CString& message, bool retryable, CMcpToolResult& result )
{GUCEF_TRACE;

    CORE::CDataNode content( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* error = content.AddChild( "error", GUCEF_DATATYPE_OBJECT );
    error->SetAttribute( "code", code );
    error->SetAttribute( "message", message );
    error->SetAttribute( "retryable", retryable );
    result.SetError( message );
    result.structuredContent = content;
    result.hasStructuredContent = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpJobBridge::PruneWaiters( void )
{GUCEF_TRACE;

    TWaiterVector done;
    {
        MT::CScopeMutex lock( m_lock );
        TWaiterVector::iterator i = m_waiters.begin();
        while ( i != m_waiters.end() )
        {
            if ( (*i)->IsDone() )
            {
                done.push_back( *i );
                i = m_waiters.erase( i );
                continue;
            }
            ++i;
        }
    }
    for ( size_t i=0; i<done.size(); ++i )
    {
        done[ i ]->Abandon( GUCEF_NULL );
        GUCEF_DELETE done[ i ];
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpJobBridge::GetWaitingCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( size_t i=0; i<m_waiters.size(); ++i )
        if ( !m_waiters[ i ]->IsDone() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpJobBridge::GetJobCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_jobs.size();
}

/*-------------------------------------------------------------------------*/

EMcpExecuteResult
CMcpJobBridge::Wait( CMcpRequestContext& context                  ,
                     const CORE::CString& jobId                   ,
                     UInt32 waitMs                                ,
                     CMcpToolResult& result                       ,
                     const CIMcpJobResultBuilderPtr& builder      )
{GUCEF_TRACE;

    PruneWaiters();

    CMcpJobInfo job;
    ELookup lookup = GetJob( context.GetPrincipal(), jobId, job );
    if ( JOB_UNKNOWN == lookup )
    {
        BuildErrorResult( "job-unknown", "There is no job " + jobId + " of yours: list your jobs, or start the job again", false, result );
        return MCP_EXECUTE_COMPLETE;
    }
    if ( JOB_EXPIRED == lookup )
    {
        BuildErrorResult( "job-expired", "The job " + jobId + " ended longer ago than its results are kept", false, result );
        return MCP_EXECUTE_COMPLETE;
    }

    CORE::PulseGeneratorPtr pulseGenerator = context.GetPulseGenerator();
    if ( job.IsFinal() || 0 == waitMs || pulseGenerator.IsNULL() )
    {
        CIMcpJobResultBuilderPtr resultBuilder = builder;
        if ( resultBuilder.IsNULL() )
            BuildDefaultResult( job, result );
        else
            resultBuilder->BuildJobResult( job, result );
        return MCP_EXECUTE_COMPLETE;
    }

    CMcpJobBridgeSettings settings = GetSettings();
    const UInt32 waitFor = waitMs > settings.maxWaitMs ? settings.maxWaitMs : waitMs;
    CMcpJobWaiter* waiter = GUCEF_NEW CMcpJobWaiter( this, context.GetSharedPtr(), jobId, NowMonotonicMs() + waitFor, builder, pulseGenerator, settings.pollIntervalMs );
    if ( job.hasRun && job.run.progressDone > 0 )
    {
        waiter->lastProgressDone = job.run.progressDone;
        context.ReportProgress( (Float64) job.run.progressDone, job.run.progressTotal > 0 ? (Float64) job.run.progressTotal : -1.0, job.run.note );
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_waiters.push_back( waiter );
    }
    waiter->Start();
    return MCP_EXECUTE_DEFERRED;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::OnWaiterTick( CMcpJobWaiter& waiter )
{GUCEF_TRACE;

    // on the thread of the request's pulse generator, with the waiter's lock held
    CMcpRequestContextPtr context = waiter.GetContext();
    if ( context.IsNULL() || context->IsCompleted() || context->IsEnded() )
        return true;
    if ( context->IsCancellationRequested() )
        return true;            // the call went away; the job goes on

    CMcpJobInfo job;
    bool found = false;
    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator j = m_jobs.find( waiter.GetJobId() );
        if ( j != m_jobs.end() )
        {
            RefreshLocked( (*j).second, NowUtcMs(), removals );
            job = (*j).second.info;
            found = true;
        }
    }
    RemoveSchedulerJobs( removals );

    if ( !found )
    {
        CMcpToolResult result;
        BuildErrorResult( "job-unknown", "The job " + waiter.GetJobId() + " is no longer known", false, result );
        context->CompleteWithToolResult( result );
        return true;
    }

    if ( job.hasRun && job.run.progressDone > waiter.lastProgressDone )
    {
        waiter.lastProgressDone = job.run.progressDone;
        context->ReportProgress( (Float64) job.run.progressDone, job.run.progressTotal > 0 ? (Float64) job.run.progressTotal : -1.0, job.run.note );
    }

    if ( job.IsFinal() || NowMonotonicMs() >= waiter.GetDeadlineMonotonicMs() )
    {
        CMcpToolResult result;
        CIMcpJobResultBuilderPtr builder = waiter.GetBuilder();
        if ( builder.IsNULL() )
            BuildDefaultResult( job, result );
        else
            builder->BuildJobResult( job, result );
        context->CompleteWithToolResult( result );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::SaveState( CORE::CDataNode& state ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    state.Clear();
    state.SetName( "McpJobBridgeState" );
    state.SetNodeType( GUCEF_DATATYPE_OBJECT );
    state.SetAttribute( "version", (UInt32) 1 );
    CORE::CDataNode* handles = state.AddChild( "jobs", GUCEF_DATATYPE_ARRAY );
    for ( TJobMap::const_iterator j = m_jobs.begin(); j != m_jobs.end(); ++j )
    {
        const SJobEntry& entry = (*j).second;
        CORE::CDataNode* node = handles->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "jobId", entry.info.jobId );
        node->SetAttribute( "kind", entry.info.kind );
        node->SetAttribute( "subject", entry.info.subject );
        node->SetAttribute( "state", entry.info.state );
        node->SetAttribute( "createdUtcMs", entry.info.createdUtcMs );
        node->SetAttribute( "expiresUtcMs", entry.info.expiresUtcMs );
        node->SetAttribute( "hostInstanceId", entry.hostInstanceId );
        node->SetAttribute( "runId", entry.runId );
        node->SetAttribute( "schedulerJobId", entry.schedulerJobId );
        node->SetAttribute( "hasRun", entry.info.hasRun );
        if ( entry.info.hasRun )
            SaveRun( entry.info.run, *node->AddChild( "run", GUCEF_DATATYPE_OBJECT ) );
    }
    CORE::CDataNode* expired = state.AddChild( "expired", GUCEF_DATATYPE_OBJECT );
    for ( TStringMap::const_iterator e = m_expired.begin(); e != m_expired.end(); ++e )
        expired->SetAttribute( (*e).first, (*e).second );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::LoadState( const CORE::CDataNode& state )
{GUCEF_TRACE;

    const CORE::CDataNode* handles = state.FindChild( "jobs" );
    if ( GUCEF_NULL == handles )
        return false;

    CORE::CString::StringVector removals;
    {
        MT::CScopeMutex lock( m_lock );
        const Int64 nowUtcMs = NowUtcMs();
        for ( CORE::CDataNode::const_iterator i = handles->ConstBegin(); i != handles->ConstEnd(); ++i )
        {
            const CORE::CDataNode& node = *(*i);
            CORE::CString jobId = node.GetAttributeValueOrChildValueByName( "jobId" ).AsString( CORE::CString::Empty, true );
            CORE::CString subject = node.GetAttributeValueOrChildValueByName( "subject" ).AsString( CORE::CString::Empty, true );
            if ( jobId.IsNULLOrEmpty() || subject.IsNULLOrEmpty() )
                continue;
            SJobEntry& entry = m_jobs[ jobId ];
            entry.info.jobId = jobId;
            entry.info.kind = node.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true );
            entry.info.subject = subject;
            entry.info.state = node.GetAttributeValueOrChildValueByName( "state" ).AsString( CMcpJobInfo::StateQueued, true );
            entry.info.createdUtcMs = node.GetAttributeValueOrChildValueByName( "createdUtcMs" ).AsInt64( 0, true );
            entry.info.expiresUtcMs = node.GetAttributeValueOrChildValueByName( "expiresUtcMs" ).AsInt64( 0, true );
            entry.hostInstanceId = node.GetAttributeValueOrChildValueByName( "hostInstanceId" ).AsUInt64( 0, true );
            entry.runId = node.GetAttributeValueOrChildValueByName( "runId" ).AsUInt64( 0, true );
            entry.schedulerJobId = node.GetAttributeValueOrChildValueByName( "schedulerJobId" ).AsString( CORE::CString::Empty, true );
            entry.info.hasRun = node.GetAttributeValueOrChildValueByName( "hasRun" ).AsBool( false, true );
            const CORE::CDataNode* run = node.FindChild( "run" );
            if ( GUCEF_NULL != run )
                LoadRun( *run, entry.info.run );
            entry.sequence = m_nextSequence++;
            RefreshLocked( entry, nowUtcMs, removals );
        }
        const CORE::CDataNode* expired = state.FindChild( "expired" );
        if ( GUCEF_NULL != expired )
        {
            CORE::CDataNode::TAttributeMap::const_iterator a = expired->AttributeBegin();
            while ( a != expired->AttributeEnd() )
            {
                m_expired[ (*a).first ] = (*a).second.AsString();
                ++a;
            }
        }
        ExpireLocked( nowUtcMs, removals );
    }
    RemoveSchedulerJobs( removals );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::SaveStateToFile( const CORE::CString& path, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CDataNode state;
    SaveState( state );
    CORE::CString text;
    if ( !CJsonRpcEndpoint::ToJsonText( state, text ) )
    {
        error = "the state cannot be written as JSON (is the JSON codec plugin loaded?)";
        return false;
    }
    if ( CORE::FileExists( path ) )
    {
        // the file keeps its permissions (owner only, as it was made) when it is replaced
        if ( !CORE::WriteFileAtomically( path, text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 ) )
        {
            error = "the state file cannot be replaced";
            return false;
        }
        CORE::SetOwnerOnlyAccess( path );
        return true;
    }
    return CORE::CreateOwnerOnlyFile( path, text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, error );
}

/*-------------------------------------------------------------------------*/

bool
CMcpJobBridge::LoadStateFromFile( const CORE::CString& path, CORE::CString& error )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return true;
    CORE::CDynamicBuffer content;
    if ( !CORE::ReadOwnerOnlyFile( path, content, error ) )
        return false;
    CORE::CString text( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    CORE::CDataNode state;
    if ( !CJsonRpcEndpoint::ParseJsonText( text, state ) )
    {
        error = "the state file holds no JSON this bridge can read";
        return false;
    }
    if ( !LoadState( state ) )
    {
        error = "the state file holds no job handles";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
