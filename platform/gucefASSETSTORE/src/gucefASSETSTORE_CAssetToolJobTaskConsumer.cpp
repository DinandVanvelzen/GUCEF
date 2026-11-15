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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#include "gucefASSETSTORE_CAssetToolContext.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#include "gucefASSETSTORE_CAssetToolJobTaskData.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H ? */

#include "gucefASSETSTORE_CAssetToolJobTaskConsumer.h"

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

const CORE::CString CAssetToolJobTaskConsumer::ResultReferencePrefix = "assetjob:";
const CORE::CString CAssetToolJobTaskConsumer::FailureReferencePrefix = "assetjob!";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 CycleDelayInMs = 10;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolJobTaskConsumer::CAssetToolJobTaskConsumer( void )
    : CORE::CTaskConsumer()
    , m_context( GUCEF_NULL )
    , m_taskTypeName()
    , m_kind()
    , m_commandId()
    , m_serviceJobId()
    , m_isIngest( false )
    , m_reported( false )
    , m_cancelSent( false )
    , m_serviceEnded( false )
    , m_future( CORE::TASKSTATUS_UNDEFINED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskConsumer::CAssetToolJobTaskConsumer( CAssetToolContext* context, const CORE::CString& taskTypeName )
    : CORE::CTaskConsumer()
    , m_context( context )
    , m_taskTypeName( taskTypeName )
    , m_kind()
    , m_commandId()
    , m_serviceJobId()
    , m_isIngest( false )
    , m_reported( false )
    , m_cancelSent( false )
    , m_serviceEnded( false )
    , m_future( CORE::TASKSTATUS_UNDEFINED )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskConsumer::~CAssetToolJobTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolJobTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return m_taskTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // a pool may hand this consumer the next task: every task starts from nothing
    m_kind.Clear();
    m_commandId.Clear();
    m_serviceJobId.Clear();
    m_isIngest = false;
    m_reported = false;
    m_cancelSent = false;
    m_serviceEnded = false;
    m_future = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );

    if ( !CORE::CTaskConsumer::OnTaskStart( task ) )
        return false;
    const CAssetToolJobTaskData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CAssetToolJobTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data || GUCEF_NULL == m_context )
    {
        if ( !task.IsNULL() )
            task->SetTaskStatusExtraInfo( "failed: the job has no data or no tool set" );
        return false;
    }
    m_kind = data->kind;
    m_commandId = data->commandId;
    CIAssetStoreService& service = m_context->GetService();
    CORE::CDataNodeSerializableSettings settings;

    if ( "store_ingest" == m_kind )
    {
        CAssetIngestRequest request;
        request.Deserialize( data->request, settings );
        m_future = service.IngestAsync( m_commandId, request );
        m_isIngest = true;
        return true;
    }
    if ( "store_ingest_resume" == m_kind )
    {
        const CORE::CString ingestId = data->request.GetAttributeValueOrChildValueByName( "ingestId" ).AsString( CORE::CString::Empty, true );
        m_future = service.ResumeIngestAsync( m_commandId, ingestId );
        m_isIngest = true;
        return true;
    }

    CAssetPipelineRequest request;
    request.Deserialize( data->request, settings );
    CAssetServiceResult started;
    const TAssetStoreStatus status = service.StartPipelineJob( m_commandId, request, started );
    if ( ASSETSTORE_STATUS_OK != status || started.id.IsNULLOrEmpty() )
    {
        task->SetTaskStatusExtraInfo( m_context->GetRedaction().ForResult( CAssetToolContext::GetErrorCodeOfStatus( status ) + ": " +
            ( started.message.IsNULLOrEmpty() ? CORE::CString( "the service did not start the job" ) : started.message ) ) );
        return false;
    }
    m_serviceJobId = started.id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskConsumer::CycleIngest( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( !m_future.IsReady() && !m_future.HasNoFuture() )
    {
        MT::ThreadDelay( CycleDelayInMs );
        return false;
    }
    CAssetIngestResult outcome;
    const bool hasOutcome = CIAssetStoreService::TryGetFutureResult( m_future, outcome );
    const TAssetStoreStatus status = hasOutcome ? outcome.status : CIAssetStoreService::GetStatusOfFuture( m_future );
    if ( hasOutcome && !outcome.ingestId.IsNULLOrEmpty() )
    {
        m_serviceEnded = true;
        ReportRunResultDocument( ResultReferencePrefix + "ingest:" + outcome.ingestId );
        task->SetTaskStatusExtraInfo( m_context->GetRedaction().ForResult( CORE::CString( AssetStoreStatusToString( status ) ) + ": " + outcome.message ) );
        return true;
    }
    m_serviceEnded = true;
    const CORE::CString message = hasOutcome ? outcome.message : CIAssetStoreService::GetMessageOfFuture( m_future );
    const CORE::CString redacted = m_context->GetRedaction().ForResult( message ).SubstrToIndex( 800, true );
    ReportRunResultDocument( FailureReferencePrefix + CAssetToolContext::GetErrorCodeOfStatus( status ) + ":" + redacted );
    task->SetTaskStatusExtraInfo( redacted );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return true;
    CIAssetStoreService& service = m_context->GetService();

    // asked to cancel: the job of the service is told, and the task is not done (see the class description)
    if ( task->IsCancellationRequested() )
    {
        CancelServiceJob();
        return false;
    }

    if ( m_isIngest )
        return CycleIngest( task );

    if ( !m_reported )
    {
        ReportRunResultDocument( ResultReferencePrefix + m_serviceJobId );
        m_reported = true;
    }
    CAssetJobEntry job;
    if ( ASSETSTORE_STATUS_OK != service.GetJob( m_serviceJobId, job ) )
    {
        m_serviceEnded = true;
        task->SetTaskStatusExtraInfo( "not-found: the service no longer knows the job" );
        return true;
    }
    ReportRunProgress( job.itemsDone, job.itemsTotal, m_context->GetRedaction().ForResult( job.message ).SubstrToIndex( 400, true ) );
    if ( job.IsEnded() )
    {
        m_serviceEnded = true;
        task->SetTaskStatusExtraInfo( m_context->GetRedaction().ForResult( CORE::CString( AssetJobStateToString( job.state ) ) +
            ( job.message.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + job.message ) ).SubstrToIndex( 400, true ) );
        return true;
    }
    MT::ThreadDelay( CycleDelayInMs );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobTaskConsumer::CancelServiceJob( void )
{GUCEF_TRACE;

    if ( m_cancelSent || m_serviceEnded || GUCEF_NULL == m_context )
        return;
    m_cancelSent = true;
    if ( !m_serviceJobId.IsNULLOrEmpty() )
    {
        CAssetServiceResult cancelled;
        m_context->GetService().ControlJob( m_commandId.IsNULLOrEmpty() ? CORE::CString::Empty : m_commandId + ":cancel", m_serviceJobId, ASSETJOB_ACTION_CANCEL, cancelled );
    }
    if ( !m_future.HasNoFuture() )
        m_future.RequestCancellation();
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // a task that ends before the job of the service did (a cancellation the delegator acted on before a cycle of
    // this consumer saw it, a stop of the scheduler) leaves no job of the service running
    CancelServiceJob();
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolJobTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetToolJobTaskConsumer";
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
