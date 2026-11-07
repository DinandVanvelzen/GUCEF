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

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMailOperationTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMailOperationTaskConsumer::TaskType = "GUCEF::COM::CMailOperationTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const CMailOperationTaskData*
GetEndedTaskData( const CORE::CFutureResult& future, EMailOperation operation )
{
    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return GUCEF_NULL;
    const CMailOperationTaskData* data = CORE::CICloneable::CloneableAs< CMailOperationTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data || data->operation != operation )
        return GUCEF_NULL;
    return data;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailOperationTaskData::CMailOperationTaskData( void )
    : CORE::CICloneable()
    , operation( MAILOPERATION_NONE )
    , smtpSettings()
    , message()
    , smtpResult()
    , pop3Settings()
    , pop3Options()
    , pop3Result()
    , imapSettings()
    , imapOptions()
    , imapResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailOperationTaskData::CMailOperationTaskData( const CMailOperationTaskData& src )
    : CORE::CICloneable( src )
    , operation( src.operation )
    , smtpSettings( src.smtpSettings )
    , message( src.message )
    , smtpResult( src.smtpResult )
    , pop3Settings( src.pop3Settings )
    , pop3Options( src.pop3Options )
    , pop3Result( src.pop3Result )
    , imapSettings( src.imapSettings )
    , imapOptions( src.imapOptions )
    , imapResult( src.imapResult )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailOperationTaskData::~CMailOperationTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailOperationTaskData&
CMailOperationTaskData::operator=( const CMailOperationTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        operation = src.operation;
        smtpSettings = src.smtpSettings;
        message = src.message;
        smtpResult = src.smtpResult;
        pop3Settings = src.pop3Settings;
        pop3Options = src.pop3Options;
        pop3Result = src.pop3Result;
        imapSettings = src.imapSettings;
        imapOptions = src.imapOptions;
        imapResult = src.imapResult;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* CMailOperationTaskData::Clone( void ) const { return GUCEF_NEW CMailOperationTaskData( *this ); }
const char* CMailOperationTaskData::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CMailOperationTaskData::CloneableTypeName( void ) { return "GUCEF::COM::CMailOperationTaskData"; }

/*-------------------------------------------------------------------------*/

CMailOperationTaskConsumer::CMailOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailOperationTaskConsumer::~CMailOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CMailOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CMailOperationTaskData* data = CORE::CICloneable::CloneableAs< CMailOperationTaskData >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailOperationTask: No task data to work on" );
        return true;
    }
    if ( task->IsCancellationRequested() )
    {
        data->smtpResult.errorClass = data->pop3Result.errorClass = data->imapResult.errorClass = MAILERR_CANCELLED;
        return true;
    }

    // the same blocking functions that a caller can call on its own thread, here on the thread of the pool
    switch ( data->operation )
    {
        case MAILOPERATION_SMTP_SEND:
            CSmtpClient::SendMessageSync( data->smtpSettings, data->message, data->smtpResult );
            break;
        case MAILOPERATION_POP3_FETCH:
            CPop3Client::FetchNewSync( data->pop3Settings, data->pop3Options, data->pop3Result );
            break;
        case MAILOPERATION_IMAP_FETCH:
            CImapClient::FetchNewSync( data->imapSettings, data->imapOptions, data->imapResult );
            break;
        default:
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailOperationTask: Unknown operation " << (Int32) data->operation );
            break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMailOperations::Queue( CMailOperationTaskData& data, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    try
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : threadPoolName );
        if ( !threadPool.IsNULL() )
        {
            CORE::CFutureResult future = threadPool->QueueTask( CMailOperationTaskConsumer::TaskType, &data );
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
CMailOperations::SendMessageAsync( const CSmtpClientSettings& settings, const CMailMessage& message, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CMailOperationTaskData data;
    data.operation = MAILOPERATION_SMTP_SEND;
    data.smtpSettings = settings;
    data.message = message;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMailOperations::FetchNewPop3Async( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CMailOperationTaskData data;
    data.operation = MAILOPERATION_POP3_FETCH;
    data.pop3Settings = settings;
    data.pop3Options = options;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CMailOperations::FetchNewImapAsync( const CImapClientSettings& settings, const CImapFetchOptions& options, const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    CMailOperationTaskData data;
    data.operation = MAILOPERATION_IMAP_FETCH;
    data.imapSettings = settings;
    data.imapOptions = options;
    return Queue( data, threadPoolName );
}

/*-------------------------------------------------------------------------*/

bool
CMailOperations::TryGetSendResult( const CORE::CFutureResult& future, CSmtpSendResult& result )
{GUCEF_TRACE;

    const CMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_SMTP_SEND );
    if ( GUCEF_NULL == data )
        return false;
    result = data->smtpResult;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailOperations::TryGetPop3FetchResult( const CORE::CFutureResult& future, CPop3FetchResult& result )
{GUCEF_TRACE;

    const CMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_POP3_FETCH );
    if ( GUCEF_NULL == data )
        return false;
    result = data->pop3Result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailOperations::TryGetImapFetchResult( const CORE::CFutureResult& future, CImapFetchResult& result )
{GUCEF_TRACE;

    const CMailOperationTaskData* data = GetEndedTaskData( future, MAILOPERATION_IMAP_FETCH );
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
