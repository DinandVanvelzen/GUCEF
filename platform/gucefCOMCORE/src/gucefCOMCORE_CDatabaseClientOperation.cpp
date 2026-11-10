/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CDatabaseClientOperation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDatabaseClientOperation::TaskType = "gucefCOMCORE::DatabaseClientOperation";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDatabaseClientOperationData::CDatabaseClientOperationData( void )
    : CORE::CICloneable()
    , client( GUCEF_NULL )
    , operation( CDatabaseClientBase::OPERATION_OPEN )
    , settings()
    , tableName()
    , definition()
    , deleteData( false )
    , source()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientOperationData::CDatabaseClientOperationData( const CDatabaseClientOperationData& src )
    : CORE::CICloneable( src )
    , client( src.client )
    , operation( src.operation )
    , settings( src.settings )
    , tableName( src.tableName )
    , definition( src.definition )
    , deleteData( src.deleteData )
    , source( src.source )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientOperationData::~CDatabaseClientOperationData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDatabaseClientOperationData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDatabaseClientOperationData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseClientOperationData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CDatabaseClientOperationData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COMCORE::CDatabaseClientOperationData";
}

/*-------------------------------------------------------------------------*/

CDatabaseClientOperation::CDatabaseClientOperation( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientOperation::~CDatabaseClientOperation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDatabaseClientOperation::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDatabaseClientOperation::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CDatabaseClientOperation";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientOperation::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientOperation::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;

    CDatabaseClientOperationData* data = const_cast< CDatabaseClientOperationData* >( CORE::CICloneable::CloneableAs< CDatabaseClientOperationData >( task->GetTaskData() ) );
    if ( GUCEF_NULL == data || GUCEF_NULL == data->client )
        return true;

    CDatabaseClientBase* client = data->client;
    data->client = GUCEF_NULL;      // from here on the client is told it is done, once
    client->RunOperation( data->operation, data->settings, data->tableName, data->definition, data->deleteData, data->source );
    client->OperationEnded();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDatabaseClientOperation::OnTaskEnded( CORE::CTaskPtr task ,
                                       bool wasForced      )
{GUCEF_TRACE;

    // A task that ended without running its operation (a pool that stopped) still has to let go of the client and of its waiter
    if ( !task.IsNULL() )
    {
        CDatabaseClientOperationData* data = const_cast< CDatabaseClientOperationData* >( CORE::CICloneable::CloneableAs< CDatabaseClientOperationData >( task->GetTaskData() ) );
        if ( GUCEF_NULL != data && GUCEF_NULL != data->client )
        {
            CDatabaseClientBase* client = data->client;
            data->client = GUCEF_NULL;
            data->source.Cancel();
            client->OperationEnded();
        }
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
