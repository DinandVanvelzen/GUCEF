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

#ifndef GUCEF_COMCORE_CRPCCALLRESULT_H
#include "gucefCOMCORE_CRpcCallResult.h"
#define GUCEF_COMCORE_CRPCCALLRESULT_H
#endif /* GUCEF_COMCORE_CRPCCALLRESULT_H ? */

#ifndef GUCEF_COMCORE_RPCEVENTDATA_H
#include "gucefCOMCORE_RpcEventData.h"
#define GUCEF_COMCORE_RPCEVENTDATA_H
#endif /* GUCEF_COMCORE_RPCEVENTDATA_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#include "gucefDRNUTILS_CTaskManagerContracts.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#include "gucefDRNUTILS_CSchedulerClient.h"
#define GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCLIENT_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#include "gucefDRNUTILS_CDrnUtilsHostSupport.h"
#define GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSHOSTSUPPORT_H ? */

#include "gucefDRNUTILS_CTaskManagerClient.h"

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

const CORE::CString CTaskManagerClient::ClassTypeName = "GUCEF::DRNUTILS::CTaskManagerClient";
const CORE::CEvent CTaskManagerClient::InfoReceivedEvent = "GUCEF::DRNUTILS::CTaskManagerClient::InfoReceivedEvent";
const CORE::CEvent CTaskManagerClient::CommandCompletedEvent = "GUCEF::DRNUTILS::CTaskManagerClient::CommandCompletedEvent";
const CORE::CEvent CTaskManagerClient::ViewingChangedEvent = "GUCEF::DRNUTILS::CTaskManagerClient::ViewingChangedEvent";

/* field numbers of gucef.drnutils.tasks 1.0 */
enum { INFO_HOSTNAME = 1, INFO_INSTANCEID = 2, INFO_SERVERTIME = 3, INFO_CAPABILITIES = 4, INFO_CONTROLLABLEPOOLS = 5, INFO_TABLENAMES = 6 };
enum { WATCH_INTERVAL = 1, WATCH_THREADS = 2, WATCH_TASKS = 3, WATCH_SCHEDULEDJOBS = 4 };
enum { CONTROL_COMMANDID = 1, CONTROL_ACTION = 2, CONTROL_POOL = 3, CONTROL_TASKID = 4, CONTROL_MINWORKERS = 5, CONTROL_MAXTHREADS = 6,
       CONTROL_REASON = 7, CONTROL_DRYRUN = 8 };
enum { OUTCOME_REPLAYED = 1, OUTCOME_MESSAGES = 2 };

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::RegisterEvents( void )
{GUCEF_TRACE;

    InfoReceivedEvent.Initialize();
    CommandCompletedEvent.Initialize();
    ViewingChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskManagerClient::CTaskManagerClient( const CORE::PulseGeneratorPtr& eventPulseGenerator )
    : CORE::CTSGNotifier( eventPulseGenerator, true, false )
    , m_endpoint()
    , m_lastInfo()
    , m_pending()
    , m_pollTimer( GUCEF_NEW CORE::CTimer( eventPulseGenerator, 10 ) )
    , m_connected( false )
    , m_watch()
    , m_samplesReceived( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback poll( this, &CTaskManagerClient::OnPollTimer );
    SubscribeTo( m_pollTimer, CORE::CTimer::TimerUpdateEvent, poll );
}

/*-------------------------------------------------------------------------*/

CTaskManagerClient::~CTaskManagerClient()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_pollTimer->SetEnabled( false );
    if ( !m_watch.IsNULL() )
    {
        UnsubscribeFrom( m_watch.GetPointerAlways() );
        m_watch->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "the viewer went" ) );
        m_watch.Unlink();
    }
    UnsubscribeAllFromObserver();
    DropPendingCalls();
    GUCEF_DELETE m_pollTimer;
    m_endpoint.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::DropPendingCalls( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_pending.size(); ++i )
        GUCEF_DELETE m_pending[ i ].future;
    m_pending.clear();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::SetEndpoint( const COMCORE::CRpcEndpointPtr& endpoint )
{GUCEF_TRACE;

    StopViewing();
    if ( !m_endpoint.IsNULL() )
        UnsubscribeFrom( m_endpoint.GetPointerAlways() );
    m_endpoint = endpoint;
    if ( !m_endpoint.IsNULL() )
    {
        m_endpoint->AddWantedContract( CTaskManagerContracts::ContractName, 1 );
        TEventCallback changed( this, &CTaskManagerClient::OnEndpointEvent );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionEstablishedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionClosedEvent, changed );
        SubscribeTo( m_endpoint.GetPointerAlways(), COMCORE::CRpcEndpoint::ContractBoundEvent, changed );
    }
    CheckConnection();
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcEndpointPtr
CTaskManagerClient::GetEndpoint( void ) const
{GUCEF_TRACE;

    return m_endpoint;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::IsConnected( void ) const
{GUCEF_TRACE;

    return !m_endpoint.IsNULL() && COMCORE::RPC_SESSIONSTATE_ESTABLISHED == m_endpoint->GetState() &&
           m_endpoint->IsContractBound( CTaskManagerContracts::ContractName, 1 );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::CheckConnection( void )
{GUCEF_TRACE;

    m_connected = IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::OnEndpointEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CheckConnection();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::NewRequest( const CORE::CString& methodName, COMCORE::CRpcMessage& request ) const
{GUCEF_TRACE;

    if ( m_endpoint.IsNULL() )
        return false;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    CORE::CString path = CORE::CString( CTaskManagerContracts::ContractName ) + "." + CTaskManagerContracts::ServiceName + "/" + methodName;
    if ( !m_endpoint->ResolveMethod( path, ref, status ) )
        return false;
    request = m_endpoint->NewRequest( ref );
    return request.IsBound();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::Send( const CORE::CString& methodName, const COMCORE::CRpcMessage& request, Int32 kind, const CORE::CString& commandId )
{GUCEF_TRACE;

    CORE::CString path = CORE::CString( CTaskManagerContracts::ContractName ) + "." + CTaskManagerContracts::ServiceName + "/" + methodName;
    CORE::CFutureResult future = m_endpoint->CallAsync( path, request );
    if ( future.HasNoFuture() )
        return false;
    SPendingCall call;
    call.kind = kind;
    call.commandId = commandId;
    call.future = GUCEF_NEW CORE::CFutureResult( future );
    m_pending.push_back( call );
    if ( !m_pollTimer->GetEnabled() )
        m_pollTimer->SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::RequestInfo( void )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( !NewRequest( "GetInfo", request ) )
        return false;
    return Send( "GetInfo", request, CALL_INFO, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

const STaskManagerHostInfo&
CTaskManagerClient::GetLastInfo( void ) const
{GUCEF_TRACE;

    return m_lastInfo;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerClient::Control( const STaskManagerControlCommand& command )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request;
    if ( !NewRequest( "Control", request ) )
        return CORE::CString::Empty;
    CORE::CString commandId = CSchedulerClient::NewCommandId();
    request.SetString( CONTROL_COMMANDID, commandId );
    request.SetEnum( CONTROL_ACTION, (Int32) command.action );
    if ( !command.poolName.IsNULLOrEmpty() )
        request.SetString( CONTROL_POOL, command.poolName );
    request.SetUInt32( CONTROL_TASKID, command.taskId );
    request.SetUInt32( CONTROL_MINWORKERS, command.minWorkers );
    request.SetInt32( CONTROL_MAXTHREADS, command.maxThreads );
    if ( !command.reason.IsNULLOrEmpty() )
        request.SetString( CONTROL_REASON, command.reason );
    request.SetBool( CONTROL_DRYRUN, command.dryRun );
    if ( !Send( "Control", request, CALL_COMMAND, commandId ) )
        return CORE::CString::Empty;
    return commandId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerClient::ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, const CORE::CString& reason )
{GUCEF_TRACE;

    STaskManagerControlCommand command;
    command.action = DRNUTILS_TASKACTION_RESIZE_POOL;
    command.poolName = poolName;
    command.minWorkers = minWorkers;
    command.maxThreads = maxThreads;
    command.reason = reason;
    return Control( command );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerClient::CancelTask( UInt32 taskId, const CORE::CString& reason )
{GUCEF_TRACE;

    STaskManagerControlCommand command;
    command.action = DRNUTILS_TASKACTION_CANCEL_TASK;
    command.taskId = taskId;
    command.reason = reason;
    return Control( command );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerClient::StopTask( UInt32 taskId, const CORE::CString& reason )
{GUCEF_TRACE;

    STaskManagerControlCommand command;
    command.action = DRNUTILS_TASKACTION_STOP_TASK;
    command.taskId = taskId;
    command.reason = reason;
    return Control( command );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::StartViewing( UInt32 sampleIntervalMs, bool includeThreads, bool includeTasks )
{GUCEF_TRACE;

    // a second call replaces the stream (with the new interval and parts)
    if ( !m_watch.IsNULL() )
        StopViewing();
    if ( m_endpoint.IsNULL() )
        return false;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    CORE::CString path = CORE::CString( CTaskManagerContracts::ContractName ) + "." + CTaskManagerContracts::ServiceName + "/Watch";
    if ( !m_endpoint->ResolveMethod( path, ref, status ) )
        return false;
    COMCORE::CRpcMessage request = m_endpoint->NewRequest( ref );
    request.SetUInt32( WATCH_INTERVAL, sampleIntervalMs < 1000 ? 1000 : sampleIntervalMs );
    request.SetBool( WATCH_THREADS, includeThreads );
    request.SetBool( WATCH_TASKS, includeTasks );
    request.SetBool( WATCH_SCHEDULEDJOBS, true );
    COMCORE::CIRpcStreamPtr stream = m_endpoint->OpenStream( ref, &request );
    if ( stream.IsNULL() )
        return false;
    m_watch = stream;
    // the samples are consumed by their events, so the credit of the stream keeps flowing; the forms read the tables
    TEventCallback streamEvent( this, &CTaskManagerClient::OnStreamEvent );
    SubscribeTo( m_watch.GetPointerAlways(), COMCORE::CIRpcStream::MessageEvent, streamEvent );
    SubscribeTo( m_watch.GetPointerAlways(), COMCORE::CIRpcStream::StatusEvent, streamEvent );
    NotifyObservers( ViewingChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::EndViewing( void )
{GUCEF_TRACE;

    if ( m_watch.IsNULL() )
        return;
    UnsubscribeFrom( m_watch.GetPointerAlways() );
    m_watch.Unlink();
    NotifyObservers( ViewingChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::StopViewing( void )
{GUCEF_TRACE;

    if ( m_watch.IsNULL() )
        return;
    m_watch->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "the viewer stopped viewing" ) );
    EndViewing();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerClient::IsViewing( void ) const
{GUCEF_TRACE;

    return !m_watch.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::OnStreamEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( COMCORE::CIRpcStream::MessageEvent == eventId )
    {
        ++m_samplesReceived;
        return;
    }
    if ( COMCORE::CIRpcStream::StatusEvent == eventId )
        EndViewing();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerClient::OnPollTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

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

        if ( CALL_INFO == done[ d ].kind )
        {
            if ( !result.status.IsOk() )
                continue;
            const COMCORE::CRpcMessage& response = result.response;
            STaskManagerHostInfo info;
            info.hostName = response.GetString( INFO_HOSTNAME );
            info.hostInstanceId = response.GetUInt64( INFO_INSTANCEID );
            info.serverTimeUtcMs = response.GetTimestamp( INFO_SERVERTIME ) / 1000;
            UInt32 count = response.GetCount( INFO_CAPABILITIES );
            for ( UInt32 c=0; c<count; ++c )
            {
                CORE::CVariant value;
                response.GetAt( INFO_CAPABILITIES, c, value );
                info.capabilities.insert( value.AsString() );
            }
            count = response.GetCount( INFO_CONTROLLABLEPOOLS );
            for ( UInt32 c=0; c<count; ++c )
            {
                CORE::CVariant value;
                response.GetAt( INFO_CONTROLLABLEPOOLS, c, value );
                info.controllablePools.insert( value.AsString() );
            }
            COMCORE::CRpcMessage::TVariantVector keys;
            response.GetMapKeys( INFO_TABLENAMES, keys );
            for ( UInt32 k=0; k<keys.size(); ++k )
            {
                CORE::CVariant value;
                response.GetMapEntry( INFO_TABLENAMES, keys[ k ], value );
                info.tableNames[ keys[ k ].AsString() ] = value.AsString();
            }
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
            outcome.replayed = result.response.GetBool( OUTCOME_REPLAYED );
            UInt32 count = result.response.GetCount( OUTCOME_MESSAGES );
            for ( UInt32 m=0; m<count; ++m )
            {
                CORE::CVariant value;
                result.response.GetAt( OUTCOME_MESSAGES, m, value );
                outcome.messages.push_back( value.AsString() );
            }
        }
        TCommandCompletedEventData data( outcome );
        NotifyObservers( CommandCompletedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerClient::GetClassTypeName( void ) const
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
