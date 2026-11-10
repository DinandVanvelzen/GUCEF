/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#include "gucefCOMCORE_CRpcCallTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CRpcCallTaskConsumer::TaskType = "GUCEF::COMCORE::CRpcCallTaskConsumer";

/*-------------------------------------------------------------------------*/

CRpcCallTaskData::CRpcCallTaskData( const CRpcServerCallPtr& serverCall )
    : CORE::CICloneable()
    , call( serverCall )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallTaskData::CRpcCallTaskData( const CRpcCallTaskData& src )
    : CORE::CICloneable( src )
    , call( src.call )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallTaskData::~CRpcCallTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcCallTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcCallTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CRpcCallTaskConsumer::CRpcCallTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCallTaskConsumer::~CRpcCallTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcCallTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CRpcCallTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    CRpcCallTaskData* data = GUCEF_NULL;
    if ( !task.IsNULL() )
        data = static_cast< CRpcCallTaskData* >( task->GetTaskData() );      // this task type only ever gets CRpcCallTaskData
    if ( GUCEF_NULL != data && !data->call.IsNULL() )
        data->call->RunHandler();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcCallTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcCallTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcCallTaskConsumer";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
