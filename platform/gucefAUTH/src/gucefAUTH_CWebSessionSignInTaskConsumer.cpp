/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#include "gucefAUTH_CWebSessionSignIn.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNIN_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H
#include "gucefAUTH_CWebSessionSignInTaskConsumer.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWebSessionSignInTaskConsumer::TaskType = "GUCEF::AUTH::CWebSessionSignInTaskConsumer";

/*-------------------------------------------------------------------------*/

CWebSessionSignInTaskConsumer::CWebSessionSignInTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSessionSignInTaskConsumer::~CWebSessionSignInTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionSignInTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWebSessionSignInTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CWebSessionSignInTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignInTaskConsumer::FailSource( CORE::CFutureResultSource& source, WEB::TWebSignInStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    if ( WEB::WEBSIGNIN_STATUS_CANCELLED == status )
    {
        source.Cancel();
        return;
    }
    CORE::CString info = WEB::WebSignInStatusToString( status );
    if ( !message.IsNULLOrEmpty() )
        info += ": " + message;
    source.Fail( CORE::TASKSTATUS_EXCEPTION_DURING_RUN, info );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignInTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CWebSessionSignInTaskData* data = const_cast< CWebSessionSignInTaskData* >( CORE::CICloneable::CloneableAs< CWebSessionSignInTaskData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->signIn )
        return true;

    CWebSessionSignIn* signIn = data->signIn;
    data->signIn = GUCEF_NULL;      // the sign in is told once that the task ended
    try
    {
        WEB::CWebSignInResult result;
        const WEB::TWebSignInStatus status = signIn->RunSignIn( data->mode, true, result );
        signIn->EndRun();
        if ( WEB::WEBSIGNIN_STATUS_OK == status )
            data->source.Complete( result.Clone(), true );
        else
            CWebSessionSignInTaskConsumer::FailSource( data->source, status, result.message );
    }
    catch ( ... )
    {
        signIn->EndRun();
        CWebSessionSignInTaskConsumer::FailSource( data->source, WEB::WEBSIGNIN_STATUS_FAILED, "the sign in threw an exception" );
    }
    signIn->AsyncEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignInTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // a task that ended without running still lets go of the sign in and of its waiter
    if ( !task.IsNULL() )
    {
        CWebSessionSignInTaskData* data = const_cast< CWebSessionSignInTaskData* >( CORE::CICloneable::CloneableAs< CWebSessionSignInTaskData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->signIn )
        {
            CWebSessionSignIn* signIn = data->signIn;
            data->signIn = GUCEF_NULL;
            data->source.Cancel();
            signIn->EndRun();
            signIn->AsyncEnded();
        }
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
