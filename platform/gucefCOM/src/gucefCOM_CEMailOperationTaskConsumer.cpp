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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#include "gucefCOM_CEMailOperationTaskData.h"
#define GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#endif /* GUCEF_COM_CEMAILOPERATIONTASKDATA_H ? */

#include "gucefCOM_CEMailOperationTaskConsumer.h"

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

const CORE::CString CEMailOperationTaskConsumer::TaskType = "GUCEF::COM::CEMailOperationTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailOperationTaskConsumer::CEMailOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailOperationTaskConsumer::~CEMailOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CEMailOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CEMailOperationTaskData* data = CORE::CICloneable::CloneableAs< CEMailOperationTaskData >( task->GetTaskData() );
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
CEMailOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
