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

#ifndef GUCEF_AUTH_CKEYRINGOPERATIONDATA_H
#include "gucefAUTH_CKeyringOperationData.h"
#define GUCEF_AUTH_CKEYRINGOPERATIONDATA_H
#endif /* GUCEF_AUTH_CKEYRINGOPERATIONDATA_H ? */

#ifndef GUCEF_AUTH_CKEYRINGSESSION_H
#include "gucefAUTH_CKeyringSession.h"
#define GUCEF_AUTH_CKEYRINGSESSION_H
#endif /* GUCEF_AUTH_CKEYRINGSESSION_H ? */

#include "gucefAUTH_CKeyringOperationTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKeyringOperationTaskConsumer::TaskType = "gucefAUTH::KeyringOperation";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyringOperationTaskConsumer::CKeyringOperationTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringOperationTaskConsumer::~CKeyringOperationTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CKeyringOperationTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKeyringOperationTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CKeyringOperationTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringOperationTaskConsumer::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringOperationTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CKeyringOperationData* data = const_cast< CKeyringOperationData* >( CORE::CICloneable::CloneableAs< CKeyringOperationData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->session )
        return true;
    CKeyringSession* session = data->session;
    data->session = GUCEF_NULL;     // from here on the session is told it is done, once
    session->RunOperation( *data );
    data->code.Clear();
    data->newCode.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CKeyringOperationTaskConsumer::OnTaskEnded( CORE::CTaskPtr task ,
                                            bool wasForced      )
{GUCEF_TRACE;

    // A task that ended without running its operation (a pool that stopped) still lets go of the session and of its waiter
    if ( !task.IsNULL() )
    {
        CKeyringOperationData* data = const_cast< CKeyringOperationData* >( CORE::CICloneable::CloneableAs< CKeyringOperationData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data )
        {
            data->code.Clear();
            data->newCode.Clear();
            if ( GUCEF_NULL != data->session )
            {
                CKeyringSession* session = data->session;
                data->session = GUCEF_NULL;
                session->AbandonOperation( *data );
            }
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
