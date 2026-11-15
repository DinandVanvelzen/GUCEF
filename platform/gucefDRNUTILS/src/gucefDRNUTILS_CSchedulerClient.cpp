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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#include "gucefDRNUTILS_CSchedulerContracts.h"
#define GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsHostSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H ? */

#include "gucefDRNUTILS_CSchedulerClient.h"

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

const CORE::CString CSchedulerClient::ClassTypeName = "GUCEF::DRNUTILS::CSchedulerClient";
const CORE::CEvent CSchedulerClient::InfoReceivedEvent = "GUCEF::DRNUTILS::CSchedulerClient::InfoReceivedEvent";
const CORE::CEvent CSchedulerClient::CommandCompletedEvent = "GUCEF::DRNUTILS::CSchedulerClient::CommandCompletedEvent";
const CORE::CEvent CSchedulerClient::ConnectionChangedEvent = "GUCEF::DRNUTILS::CSchedulerClient::ConnectionChangedEvent";
const CORE::CEvent CSchedulerClient::JobDetailReceivedEvent = "GUCEF::DRNUTILS::CSchedulerClient::JobDetailReceivedEvent";

/* field numbers of gucef.drnutils.scheduler 1.0 */
enum { HOSTINFO_HOSTNAME = 1, HOSTINFO_INSTANCEID = 2, HOSTINFO_REVISION = 3, HOSTINFO_SERVERTIME = 4, HOSTINFO_STATE = 5,
       HOSTINFO_ALLOWEDACTIONS = 6, HOSTINFO_TABLENAMES = 7, HOSTINFO_SUBMITTABLE = 8, HOSTINFO_MAXPAYLOAD = 9 };
enum { CONTROL_COMMANDID = 1, CONTROL_ACTION = 2, CONTROL_TARGETKIND = 3, CONTROL_TARGETID = 4, CONTROL_CUSTOMNAME = 5,
       CONTROL_ARGUMENTS = 6, CONTROL_PRIORITY = 8, CONTROL_MODE = 9, CONTROL_REASON = 10, CONTROL_EXPECTEDREVISION = 11,
       CONTROL_DRYRUN = 12, CONTROL_WAITFORRUN = 13 };
enum { OUTCOME_REPLAYED = 1, OUTCOME_REVISION = 2, OUTCOME_RUNID = 3, OUTCOME_MESSAGES = 4, OUTCOME_RUN = 5, OUTCOME_DETAIL = 6 };
enum { GETJOB_JOBID = 1, GETJOB_RECENTRUNS = 2, GETJOB_SOURCESTATUS = 3 };
enum { DETAIL_JOB = 1, DETAIL_RECENTRUNS = 2, DETAIL_SOURCESTATUS = 3, DETAIL_DESCRIPTION = 4, DETAIL_PAYLOAD = 5 };
enum { TRIGGER_KIND = 1, TRIGGER_CRON = 2, TRIGGER_RANGESTART = 3, TRIGGER_RANGEEND = 4, TRIGGER_PERIODMS = 5, TRIGGER_INTERVALFROM = 6,
       TRIGGER_ALIGNTO = 7, TRIGGER_ONCEAT = 8 };
enum { SUBMIT_COMMANDID = 1, SUBMIT_JOBID = 2, SUBMIT_TITLE = 3, SUBMIT_TASKTYPE = 4, SUBMIT_PAYLOAD = 5, SUBMIT_TRIGGER = 6,
       SUBMIT_EXECUTIONTARGET = 7, SUBMIT_THREADPOOL = 8, SUBMIT_LANE = 9, SUBMIT_PRIORITY = 10, SUBMIT_REPLACE = 11,
       SUBMIT_DRYRUN = 12, SUBMIT_REASON = 13, SUBMIT_DESCRIPTION = 14 };

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::RegisterEvents( void )
{GUCEF_TRACE;

    InfoReceivedEvent.Initialize();
    CommandCompletedEvent.Initialize();
    ConnectionChangedEvent.Initialize();
    JobDetailReceivedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CSchedulerClient::CSchedulerClient( const CORE::PulseGeneratorPtr& eventPulseGenerator )
    : CORE::CTSGNotifier( eventPulseGenerator, true, false )
    , m_endpoint()
    , m_lastInfo()
    , m_pending()
    , m_pollTimer( GUCEF_NEW CORE::CTimer( eventPulseGenerator, 10 ) )
    , m_connected( false )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback poll( this, &CSchedulerClient::OnPollTimer );
    SubscribeTo( m_pollTimer, CORE::CTimer::TimerUpdateEvent, poll );
}

/*-------------------------------------------------------------------------*/

CSchedulerClient::~CSchedulerClient()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_pollTimer->SetEnabled( false );
    UnsubscribeAllFromObserver();
    DropPendingCalls();
    GUCEF_DELETE m_pollTimer;
    m_endpoint.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::DropPendingCalls( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_pending.size(); ++i )
        GUCEF_DELETE m_pending[ i ].future;
    m_pending.clear();
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::SetEndpoint( const COMCORE::CRpcEndpointPtr& endpoint )
{GUCEF_TRACE;

    if ( !m_endpoint.IsNULL() )
        UnsubscribeFrom( m_endpoint.GetPointerAlways() );
    m_endpoint = endpoint;
    if ( !m_endpoint.IsNULL() )
    {
        // the contract is wanted in the handshake: give the endpoint to the client before the session starts
        m_endpoint->AddWantedContract( CSchedulerContracts::ContractName, 1 );
        TEventCallback changed( this, &CSchedulerClient::OnEndpointEvent );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionEstablishedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionResumedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionSuspendedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionClosedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::ContractBoundEvent, changed );
    }
    CheckConnection();
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcEndpointPtr
CSchedulerClient::GetEndpoint( void ) const
{GUCEF_TRACE;

    return m_endpoint;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerClient::IsConnected( void ) const
{GUCEF_TRACE;

    return !m_endpoint.IsNULL() && COMCORE::RPC_SESSIONSTATE_ESTABLISHED == m_endpoint->GetState() &&
           m_endpoint->IsContractBound( CSchedulerContracts::ContractName, 1 );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::CheckConnection( void )
{GUCEF_TRACE;

    bool connected = IsConnected();
    if ( connected != m_connected )
    {
        m_connected = connected;
        NotifyObservers( ConnectionChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::OnEndpointEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CheckConnection();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerClient::NewRequest( const CORE::CString& methodName, COMCORE::CRpcMessage& request ) const
{GUCEF_TRACE;

    if ( m_endpoint.IsNULL() )
        return false;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    CORE::CString path = CORE::CString( CSchedulerContracts::ContractName ) + "." + CSchedulerContracts::ServiceName + "/" + methodName;
    if ( !m_endpoint->ResolveMethod( path, ref, status ) )
        return false;
    request = m_endpoint->NewRequest( ref );
    return request.IsBound();
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerClient::Send( const CORE::CString& methodName, const COMCORE::CRpcMessage& request, Int32 kind, const CORE::CString& commandId,
                        const CORE::CString& jobId )
{GUCEF_TRACE;

    CORE::CString path = CORE::CString( CSchedulerContracts::ContractName ) + "." + CSchedulerContracts::ServiceName + "/" + methodName;
    // asynchronous: nothing blocks the thread of the caller (the GUI thread); the answer is taken when the future is ready
    CORE::CFutureResult future = m_endpoint->CallAsync( path, request );
    if ( future.HasNoFuture() )
        return false;
    SPendingCall call;
    call.kind = kind;
    call.commandId = commandId;
    call.jobId = jobId;
    call.future = GUCEF_NEW CORE::CFutureResult( future );
    m_pending.push_back( call );
    if ( !m_pollTimer->GetEnabled() )
        m_pollTimer->SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerClient::RequestInfo( void )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( !NewRequest( "GetInfo", request ) )
        return false;
    return Send( "GetInfo", request, CALL_INFO, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

const SSchedulerHostInfo&
CSchedulerClient::GetLastInfo( void ) const
{GUCEF_TRACE;

    return m_lastInfo;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::Control( const CORE::SJobControlCommand& command, bool waitForRun )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( !NewRequest( "Control", request ) )
        return CORE::CString::Empty;
    CORE::CString commandId = NewCommandId();
    request.SetString( CONTROL_COMMANDID, commandId );
    request.SetEnum( CONTROL_ACTION, (Int32) command.action );
    request.SetEnum( CONTROL_TARGETKIND, (Int32) command.targetKind );
    request.SetString( CONTROL_TARGETID, command.targetId );
    if ( !command.customName.IsNULLOrEmpty() )
        request.SetString( CONTROL_CUSTOMNAME, command.customName );
    CORE::SJobControlCommand::TArgumentMap::const_iterator a = command.arguments.begin();
    while ( a != command.arguments.end() )
    {
        request.SetMapEntry( CONTROL_ARGUMENTS, CORE::CVariant( (*a).first ), CORE::CVariant( (*a).second ) );
        ++a;
    }
    if ( command.hasPriority )
        request.SetUInt32( CONTROL_PRIORITY, command.priority );
    if ( !command.mode.IsNULLOrEmpty() )
        request.SetString( CONTROL_MODE, command.mode );
    if ( !command.reason.IsNULLOrEmpty() )
        request.SetString( CONTROL_REASON, command.reason );
    if ( 0 != command.expectedRevision )
        request.SetUInt64( CONTROL_EXPECTEDREVISION, command.expectedRevision );
    request.SetBool( CONTROL_DRYRUN, command.dryRun );
    request.SetBool( CONTROL_WAITFORRUN, waitForRun );
    if ( !Send( "Control", request, CALL_COMMAND, commandId ) )
        return CORE::CString::Empty;
    return commandId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::RunNow( const CORE::CString& jobId, const CORE::CString& reason, bool waitForRun )
{GUCEF_TRACE;

    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_RUN_NOW;
    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
    command.targetId = jobId;
    command.reason = reason;
    return Control( command, waitForRun );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::CancelRun( UInt64 runId, const CORE::CString& reason )
{GUCEF_TRACE;

    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_CANCEL_RUN;
    command.targetKind = CORE::JOBCONTROLTARGET_RUN;
    command.targetId = CORE::ToString( runId );
    command.reason = reason;
    return Control( command, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::SetJobEnabled( const CORE::CString& jobId, bool enabled, const CORE::CString& reason )
{GUCEF_TRACE;

    CORE::SJobControlCommand command;
    command.action = enabled ? CORE::JOBCONTROLACTION_ENABLE : CORE::JOBCONTROLACTION_DISABLE;
    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
    command.targetId = jobId;
    command.reason = reason;
    return Control( command, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::SubmitJob( const CORE::SJobSubmission& submission )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( !NewRequest( "SubmitJob", request ) )
        return CORE::CString::Empty;
    const CORE::CJobDefinition& definition = submission.definition;
    CORE::CString commandId = NewCommandId();
    request.SetString( SUBMIT_COMMANDID, commandId );
    request.SetString( SUBMIT_JOBID, definition.GetId() );
    request.SetString( SUBMIT_TITLE, definition.GetTitle() );
    request.SetString( SUBMIT_TASKTYPE, definition.GetTaskTypeName() );
    request.SetDataNode( SUBMIT_PAYLOAD, definition.GetPayload() );
    const CORE::CJobTrigger& trigger = definition.GetTrigger();
    COMCORE::CRpcMessage* spec = request.GetMutableMessage( SUBMIT_TRIGGER );
    if ( GUCEF_NULL != spec )
    {
        spec->SetEnum( TRIGGER_KIND, (Int32) trigger.GetKind() );
        if ( CORE::JOBTRIGGER_SCHEDULE == trigger.GetKind() )
        {
            spec->SetString( TRIGGER_CRON, trigger.GetSchedule().GetCronSchedule().ToCronString() );
            const CORE::CDateTimeRange& range = trigger.GetSchedule().GetTimeRange();
            if ( range.GetStart() != CORE::CDateTime::Empty )
                spec->SetTimestamp( TRIGGER_RANGESTART, (Int64) range.GetStart().ToUnixEpochBasedTicksInMillisecs() * 1000 );
            if ( range.GetEnd() != CORE::CDateTime::Empty )
                spec->SetTimestamp( TRIGGER_RANGEEND, (Int64) range.GetEnd().ToUnixEpochBasedTicksInMillisecs() * 1000 );
        }
        spec->SetUInt64( TRIGGER_PERIODMS, trigger.GetPeriodMs() );
        spec->SetEnum( TRIGGER_INTERVALFROM, (Int32) trigger.GetIntervalFrom() );
        if ( trigger.GetHasAlignment() )
            spec->SetTimestamp( TRIGGER_ALIGNTO, trigger.GetAlignToUtcMs() * 1000 );
        if ( 0 != trigger.GetOnceAtUtcMs() )
            spec->SetTimestamp( TRIGGER_ONCEAT, trigger.GetOnceAtUtcMs() * 1000 );
    }
    request.SetEnum( SUBMIT_EXECUTIONTARGET, (Int32) definition.GetExecutionTarget() );
    if ( !definition.GetThreadPoolName().IsNULLOrEmpty() )
        request.SetString( SUBMIT_THREADPOOL, definition.GetThreadPoolName() );
    if ( !definition.GetLaneName().IsNULLOrEmpty() )
        request.SetString( SUBMIT_LANE, definition.GetLaneName() );
    request.SetUInt32( SUBMIT_PRIORITY, definition.GetPriority() );
    request.SetBool( SUBMIT_REPLACE, submission.replaceExisting );
    request.SetBool( SUBMIT_DRYRUN, submission.dryRun );
    if ( !submission.reason.IsNULLOrEmpty() )
        request.SetString( SUBMIT_REASON, submission.reason );
    if ( !definition.GetDescription().IsNULLOrEmpty() )
        request.SetString( SUBMIT_DESCRIPTION, definition.GetDescription() );
    if ( !Send( "SubmitJob", request, CALL_COMMAND, commandId ) )
        return CORE::CString::Empty;
    return commandId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::RequestJobDetail( const CORE::CString& jobId, UInt32 recentRuns, bool includeSourceStatus )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( jobId.IsNULLOrEmpty() || !NewRequest( "GetJob", request ) )
        return CORE::CString::Empty;
    request.SetString( GETJOB_JOBID, jobId );
    request.SetUInt32( GETJOB_RECENTRUNS, recentRuns );
    request.SetBool( GETJOB_SOURCESTATUS, includeSourceStatus );
    CORE::CString requestId = NewCommandId();
    if ( !Send( "GetJob", request, CALL_JOBDETAIL, requestId, jobId ) )
        return CORE::CString::Empty;
    return requestId;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::OnJobDetailAnswer( const SPendingCall& call, const COMCORE::CRpcCallResult& result )
{GUCEF_TRACE;

    SSchedulerJobDetailAnswer answer;
    answer.requestId = call.commandId;
    answer.jobId = call.jobId;
    answer.status = CDrnUtilsHostSupport::FromRpcStatusCode( result.status.code );
    answer.statusMessage = result.status.message;
    if ( result.status.IsOk() )
    {
        const COMCORE::CRpcMessage& response = result.response;
        const COMCORE::CRpcMessage* job = response.GetMessage( DETAIL_JOB );
        if ( GUCEF_NULL != job )
        {
            CSchedulerRowCodec::JobFromMessage( *job, answer.detail.job );
            UInt32 count = job->GetCount( CSchedulerRowCodec::JOBROW_ALLOWEDACTIONS );
            for ( UInt32 a=0; a<count; ++a )
            {
                SSchedulerActionInfo action;
                CSchedulerRowCodec::ActionFromMessage( *job->GetMessageAt( CSchedulerRowCodec::JOBROW_ALLOWEDACTIONS, a ), action );
                answer.allowedActions.push_back( action );
            }
        }
        UInt32 count = response.GetCount( DETAIL_RECENTRUNS );
        for ( UInt32 r=0; r<count; ++r )
        {
            CORE::SJobRunRecord run;
            CSchedulerRowCodec::RunFromMessage( *response.GetMessageAt( DETAIL_RECENTRUNS, r ), run );
            answer.detail.recentRuns.push_back( run );
        }
        response.GetDataNode( DETAIL_SOURCESTATUS, answer.detail.sourceStatus );
        answer.detail.job.description = response.GetString( DETAIL_DESCRIPTION );
        response.GetDataNode( DETAIL_PAYLOAD, answer.detail.job.payload );
    }
    TJobDetailReceivedEventData data( answer );
    NotifyObservers( JobDetailReceivedEvent, &data );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerClient::NewCommandId( void )
{GUCEF_TRACE;

    static const char* hexDigits = "0123456789abcdef";
    UInt8 bytes[ 16 ];
    CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) );
    char text[ 33 ];
    for ( UInt32 i=0; i<16; ++i )
    {
        text[ i*2 ] = hexDigits[ ( bytes[ i ] >> 4 ) & 0x0F ];
        text[ i*2+1 ] = hexDigits[ bytes[ i ] & 0x0F ];
    }
    text[ 32 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerClient::OnPollTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the calls that ended are taken first: an event handler may issue new calls
    TPendingCallVector done;
    for ( UInt32 i=0; i<m_pending.size(); )
    {
        if ( m_pending[ i ].future->IsReady() || m_pending[ i ].future->HasNoFuture() )
        {
            done.push_back( m_pending[ i ] );
            m_pending.erase( m_pending.begin() + i );
        }
        else
        {
            ++i;
        }
    }
    if ( m_pending.empty() )
        m_pollTimer->SetEnabled( false );

    for ( UInt32 d=0; d<done.size(); ++d )
    {
        COMCORE::CRpcCallResult result;
        if ( !COMCORE::CRpcEndpoint::TryGetCallResult( *done[ d ].future, result ) )
            result.status.Set( COMCORE::RPC_STATUS_UNAVAILABLE, "the call ended without an answer" );
        GUCEF_DELETE done[ d ].future;
        done[ d ].future = GUCEF_NULL;

        if ( CALL_JOBDETAIL == done[ d ].kind )
        {
            OnJobDetailAnswer( done[ d ], result );
            continue;
        }

        if ( CALL_INFO == done[ d ].kind )
        {
            if ( !result.status.IsOk() )
                continue;
            const COMCORE::CRpcMessage& response = result.response;
            SSchedulerHostInfo info;
            info.hostName = response.GetString( HOSTINFO_HOSTNAME );
            info.hostInstanceId = response.GetUInt64( HOSTINFO_INSTANCEID );
            info.revision = response.GetUInt64( HOSTINFO_REVISION );
            info.serverTimeUtcMs = response.GetTimestamp( HOSTINFO_SERVERTIME ) / 1000;
            info.state = response.GetString( HOSTINFO_STATE );
            UInt32 count = response.GetCount( HOSTINFO_ALLOWEDACTIONS );
            for ( UInt32 a=0; a<count; ++a )
            {
                SSchedulerActionInfo action;
                CSchedulerRowCodec::ActionFromMessage( *response.GetMessageAt( HOSTINFO_ALLOWEDACTIONS, a ), action );
                info.allowedActions.push_back( action );
            }
            COMCORE::CRpcMessage::TVariantVector keys;
            response.GetMapKeys( HOSTINFO_TABLENAMES, keys );
            for ( UInt32 k=0; k<keys.size(); ++k )
            {
                CORE::CVariant value;
                response.GetMapEntry( HOSTINFO_TABLENAMES, keys[ k ], value );
                info.tableNames[ keys[ k ].AsString() ] = value.AsString();
            }
            count = response.GetCount( HOSTINFO_SUBMITTABLE );
            for ( UInt32 t=0; t<count; ++t )
            {
                CORE::CVariant value;
                response.GetAt( HOSTINFO_SUBMITTABLE, t, value );
                info.submittableTaskTypes.insert( value.AsString() );
            }
            info.maxSubmitPayloadBytes = response.GetUInt32( HOSTINFO_MAXPAYLOAD );
            m_lastInfo = info;
            TInfoReceivedEventData data( info );
            NotifyObservers( InfoReceivedEvent, &data );
            continue;
        }

        SDrnUtilsCommandOutcome outcome;
        outcome.commandId = done[ d ].commandId;
        outcome.status = CDrnUtilsHostSupport::FromRpcStatusCode( result.status.code );
        outcome.statusMessage = result.status.message;
        if ( result.status.IsOk() )
        {
            const COMCORE::CRpcMessage& response = result.response;
            outcome.replayed = response.GetBool( OUTCOME_REPLAYED );
            outcome.revision = response.GetUInt64( OUTCOME_REVISION );
            outcome.runId = response.GetUInt64( OUTCOME_RUNID );
            UInt32 count = response.GetCount( OUTCOME_MESSAGES );
            for ( UInt32 m=0; m<count; ++m )
            {
                CORE::CVariant value;
                response.GetAt( OUTCOME_MESSAGES, m, value );
                outcome.messages.push_back( value.AsString() );
            }
            const COMCORE::CRpcMessage* run = response.GetMessage( OUTCOME_RUN );
            if ( GUCEF_NULL != run )
            {
                outcome.hasRun = true;
                CSchedulerRowCodec::RunFromMessage( *run, outcome.run );
            }
            response.GetDataNode( OUTCOME_DETAIL, outcome.detail );
        }
        TCommandCompletedEventData data( outcome );
        NotifyObservers( CommandCompletedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerClient::GetClassTypeName( void ) const
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
