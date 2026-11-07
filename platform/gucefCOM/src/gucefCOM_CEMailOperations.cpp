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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H
#include "gucefCOM_CEMailOperationTaskConsumer.h"
#define GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H
#endif /* GUCEF_COM_CEMAILOPERATIONTASKCONSUMER_H ? */

#ifndef GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#include "gucefCOM_CEMailOperationTaskData.h"
#define GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#endif /* GUCEF_COM_CEMAILOPERATIONTASKDATA_H ? */

#include "gucefCOM_CEMailOperations.h"

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

const CEMailOperationTaskData*
GetEndedTaskData( const CORE::CFutureResult& future, EMailOperation operation )
{
    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return GUCEF_NULL;
    const CEMailOperationTaskData* data = CORE::CICloneable::CloneableAs< CEMailOperationTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->operation != operation )
        return GUCEF_NULL;
    return data;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CFutureResult
CEMailOperations::Queue( CEMailOperationTaskData& data, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    try
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName );
        if ( !threadPool.IsNULL() )
        {
            CORE::CFutureResult future = threadPool->QueueTask( CEMailOperationTaskConsumer::TaskType, &data );
            if ( future.HasAFuture() )
                return future;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "MailOperations: Failed to queue the operation: " << e.what() );
    }
    return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CEMailOperations::SendMessageAsync( const CSmtpClientSettings& settings, const CEMailMessage& message, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CEMailOperationTaskData data;
    data.operation = MAILOPERATION_SMTP_SEND;
    data.smtpSettings = settings;
    data.message = message;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CEMailOperations::FetchNewPop3Async( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CEMailOperationTaskData data;
    data.operation = MAILOPERATION_POP3_FETCH;
    data.pop3Settings = settings;
    data.pop3Options = options;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CEMailOperations::FetchNewImapAsync( const CImapClientSettings& settings, const CImapFetchOptions& options, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CEMailOperationTaskData data;
    data.operation = MAILOPERATION_IMAP_FETCH;
    data.imapSettings = settings;
    data.imapOptions = options;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

bool
CEMailOperations::TryGetSendResult( const CORE::CFutureResult& future, CSmtpSendResult& result )
{GUCEF_TRACE;

    const CEMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_SMTP_SEND );
    if ( GUCEF_NULL == data )
        return false;
    result = data->smtpResult;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailOperations::TryGetPop3FetchResult( const CORE::CFutureResult& future, CPop3FetchResult& result )
{GUCEF_TRACE;

    const CEMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_POP3_FETCH );
    if ( GUCEF_NULL == data )
        return false;
    result = data->pop3Result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEMailOperations::TryGetImapFetchResult( const CORE::CFutureResult& future, CImapFetchResult& result )
{GUCEF_TRACE;

    const CEMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_IMAP_FETCH );
    if ( GUCEF_NULL == data )
        return false;
    result = data->imapResult;
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
