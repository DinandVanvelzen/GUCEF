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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreProviderOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#include "gucefASSETSTORE_CWebSessionSignIn.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H
#include "gucefASSETSTORE_CWebSessionSignInTaskConsumer.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWebSessionSignInTaskConsumer::TaskType = "GUCEF::ASSETSTORE::CWebSessionSignInTaskConsumer";

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

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CWebSessionSignInTaskConsumer";
    return classTypeName;
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
        CWebSignInResult result;
        const TAssetStoreStatus status = signIn->RunSignIn( data->mode, true, result );
        signIn->EndRun();
        if ( ASSETSTORE_STATUS_OK == status )
            data->source.Complete( result.Clone(), true );
        else
            CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, status, result.message );
    }
    catch ( ... )
    {
        signIn->EndRun();
        CAssetStoreProviderOperationTaskConsumer::FailSource( data->source, ASSETSTORE_STATUS_FAILED, "the sign in threw an exception" );
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

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
