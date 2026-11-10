/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include "gucefCORE_CDataTableAsyncOperation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CDataTableAsyncOperation::TaskType = "DataTableAsyncOperation";
const CString CDataTableAsyncOperation::DefaultThreadPoolName = "DataTables";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableAsyncLink::CDataTableAsyncLink( CIDataTable* table )
    : m_lock()
    , m_table( table )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableAsyncLink::~CDataTableAsyncLink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataTableAsyncLink::Detach( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    m_table = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CIDataTable*
CDataTableAsyncLink::Lock( void )
{GUCEF_TRACE;

    m_lock.Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return m_table;
}

/*-------------------------------------------------------------------------*/

void
CDataTableAsyncLink::Unlock( void )
{GUCEF_TRACE;

    m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperationData::CDataTableAsyncOperationData( void )
    : CICloneable()
    , table()
    , operation( OPERATION_WRITE )
    , rows()
    , writeMode( DATATABLE_WRITE_INSERT )
    , query()
    , flag( false )
    , result()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperationData::CDataTableAsyncOperationData( const CDataTableAsyncOperationData& src )
    : CICloneable( src )
    , table( src.table )
    , operation( src.operation )
    , rows( src.rows )
    , writeMode( src.writeMode )
    , query( src.query )
    , flag( src.flag )
    , result( src.result )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperationData::~CDataTableAsyncOperationData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperationData&
CDataTableAsyncOperationData::operator=( const CDataTableAsyncOperationData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        table = src.table;
        operation = src.operation;
        rows = src.rows;
        writeMode = src.writeMode;
        query = src.query;
        flag = src.flag;
        result = src.result;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableAsyncOperationData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableAsyncOperationData( *this );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CDataTableAsyncOperation::Queue( const CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CFutureResultSource result( data.result );
    if ( threadPool.IsNULL() )
        threadPool = CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( DefaultThreadPoolName );
    if ( threadPool.IsNULL() )
    {
        result.Fail( TTaskStatus::TASKSTATUS_QUEUEING_FAILED, "No thread pool for the data table operation" );
        return result.GetFuture();
    }

    CDataTableAsyncOperationData taskData( data );
    CFutureResult queued = threadPool->QueueTask( TaskType, &taskData, GUCEF_NULL, false );
    if ( queued.HasNoFuture() )
        result.Fail( TTaskStatus::TASKSTATUS_QUEUEING_FAILED, "The data table operation could not be queued on thread pool \"" + threadPool->GetThreadPoolName() + "\"" );
    return result.GetFuture();
}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperation::CDataTableAsyncOperation( void )
    : CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableAsyncOperation::~CDataTableAsyncOperation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataTableAsyncOperation::OnTaskCycle( CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CDataTableAsyncOperationData* data = static_cast< CDataTableAsyncOperationData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return true;
    if ( data->table.IsNULL() )
    {
        data->result.Fail( TTaskStatus::TASKSTATUS_TASKDATA_INVALID, "The operation names no table" );
        return true;
    }
    if ( data->result.IsCancellationRequested() )
    {
        data->result.Cancel();
        return true;
    }

    CIDataTable* table = data->table->Lock();
    if ( GUCEF_NULL == table )
    {
        data->table->Unlock();
        data->result.Fail( TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The table was destroyed before the operation ran" );
        return true;
    }

    TDataTableStatus status = DATATABLE_STATUS_OK;
    CDataTableWriteResult writeResult;
    CDataTablePartitionInfoList partitions;
    bool isList = false;
    switch ( data->operation )
    {
        case CDataTableAsyncOperationData::OPERATION_WRITE:
        {
            if ( data->rows.IsNULL() )
                status = DATATABLE_STATUS_INVALID_ARGUMENT;
            else
                status = table->Write( *data->rows, data->writeMode, writeResult );
            break;
        }
        case CDataTableAsyncOperationData::OPERATION_DELETE_ROWS:
        {
            status = table->DeleteRows( data->query, writeResult );
            break;
        }
        case CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS:
        {
            status = table->DeletePartitions( data->query, writeResult );
            break;
        }
        case CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS:
        {
            isList = true;
            status = table->ListPartitions( data->query, data->flag, partitions.partitions );
            break;
        }
        case CDataTableAsyncOperationData::OPERATION_COMPACT:
        {
            status = table->Compact( data->query, data->flag, writeResult );
            break;
        }
        default:
        {
            status = DATATABLE_STATUS_INVALID_ARGUMENT;
            break;
        }
    }
    const CString message = DATATABLE_STATUS_OK == status ? CString::Empty : table->GetLastErrorMessage();
    data->table->Unlock();

    if ( DATATABLE_STATUS_OK != status )
    {
        data->result.Fail( TTaskStatus::TASKSTATUS_STARTUP_FAILED, CString( DataTableStatusToString( status ) ) + ": " + message );
        return true;
    }
    if ( isList )
        data->result.Complete( partitions.Clone(), true );
    else
        data->result.Complete( writeResult.Clone(), true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDataTableAsyncOperation::OnTaskEnded( CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // a task that never ran its cycle (stopped or killed) still ends its future
    if ( task.IsNULL() )
        return;
    CDataTableAsyncOperationData* data = static_cast< CDataTableAsyncOperationData* >( task->GetTaskData() );
    if ( GUCEF_NULL != data && !data->result.IsCompleted() )
        data->result.Cancel();
}

/*-------------------------------------------------------------------------*/

CString
CDataTableAsyncOperation::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CDataTableAsyncOperation::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CDataTableAsyncOperation";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
