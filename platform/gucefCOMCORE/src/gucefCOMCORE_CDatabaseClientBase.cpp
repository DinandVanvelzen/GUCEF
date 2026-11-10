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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#include "gucefCOMCORE_CDatabaseTableResult.h"
#define GUCEF_COMCORE_CDATABASETABLERESULT_H
#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTOPERATION_H
#include "gucefCOMCORE_CDatabaseClientOperation.h"
#define GUCEF_COMCORE_CDATABASECLIENTOPERATION_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTOPERATION_H ? */

#include "gucefCOMCORE_CDatabaseClientBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a failed sync call ends its future, with an error that is an end state (CFutureResultSource::Fail() refuses the
 *  others): what was asked cannot be done with the data or the state it was asked in (invalid arguments, a table that is
 *  not there or is there already, a mismatch, a conflict, a read only database) as TASKSTATUS_TASKDATA_INVALID, and what
 *  went wrong while it ran (storage, corrupt data, memory, limits) as TASKSTATUS_EXCEPTION_DURING_RUN. The extra info of
 *  the future is "STATUSNAME: message".
 */
static CORE::TTaskStatus
FailureStatusFor( CORE::TDataTableStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case CORE::DATATABLE_STATUS_INVALID_ARGUMENT:
        case CORE::DATATABLE_STATUS_NOT_FOUND:
        case CORE::DATATABLE_STATUS_ALREADY_EXISTS:
        case CORE::DATATABLE_STATUS_SCHEMA_MISMATCH:
        case CORE::DATATABLE_STATUS_TYPE_MISMATCH:
        case CORE::DATATABLE_STATUS_CONFLICT:
        case CORE::DATATABLE_STATUS_UNSUPPORTED:
        case CORE::DATATABLE_STATUS_READ_ONLY:
            return CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID;
        default:
            return CORE::TTaskStatus::TASKSTATUS_EXCEPTION_DURING_RUN;
    }
}

/*-------------------------------------------------------------------------*/

CDatabaseClientBase::CDatabaseClientBase( void )
    : CIDatabaseClient()
    , m_pendingOperations( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientBase::~CDatabaseClientBase()
{GUCEF_TRACE;

    // The backend waited already. When it did not, the operations still call into a client that is going away
    if ( m_pendingOperations.Load() > 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DatabaseClientBase: Destroyed with async operations pending, the backend must call WaitForAsyncOperations()" );
        WaitForAsyncOperations();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDatabaseClientBase::GetPendingAsyncOperationCount( void ) const
{GUCEF_TRACE;

    return m_pendingOperations.Load();
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientBase::WaitForAsyncOperations( UInt32 timeoutInMs )
{GUCEF_TRACE;

    UInt32 waited = 0;
    while ( m_pendingOperations.Load() > 0 )
    {
        if ( waited >= timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
        ++waited;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDatabaseClientBase::OperationEnded( void )
{GUCEF_TRACE;

    --m_pendingOperations;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::QueueOperation( EOperation operation, const CDatabaseClientSettings& settings, const CORE::CString& tableName,
                                     const CORE::CDataTableDefinition& definition, bool deleteData )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr threadPool = settings.threadPool;
    if ( threadPool.IsNULL() )
        threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( settings.threadPoolName.IsNULLOrEmpty() ? CORE::CString( "DataTables" ) : settings.threadPoolName );
    if ( threadPool.IsNULL() )
        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED );

    CORE::CFutureResultSource source;
    CDatabaseClientOperationData* data = GUCEF_NEW CDatabaseClientOperationData();
    data->client = this;
    data->operation = operation;
    data->settings = settings;
    data->tableName = tableName;
    data->definition = definition;
    data->deleteData = deleteData;
    data->source = source;

    ++m_pendingOperations;
    CORE::CFutureResult queued = threadPool->QueueTask( CDatabaseClientOperation::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        --m_pendingOperations;
        source.Fail( CORE::TTaskStatus::TASKSTATUS_QUEUEING_FAILED, "The operation could not be queued on the thread pool" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::OpenAsync( const CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    return QueueOperation( OPERATION_OPEN, settings, CORE::CString::Empty, CORE::CDataTableDefinition(), false );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::ListTablesAsync( void )
{GUCEF_TRACE;

    return QueueOperation( OPERATION_LIST_TABLES, GetSettings(), CORE::CString::Empty, CORE::CDataTableDefinition(), false );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::OpenTableAsync( const CORE::CString& tableName )
{GUCEF_TRACE;

    return QueueOperation( OPERATION_OPEN_TABLE, GetSettings(), tableName, CORE::CDataTableDefinition(), false );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::CreateTableAsync( const CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    return QueueOperation( OPERATION_CREATE_TABLE, GetSettings(), CORE::CString::Empty, definition, false );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CDatabaseClientBase::DropTableAsync( const CORE::CString& tableName, bool deleteData )
{GUCEF_TRACE;

    return QueueOperation( OPERATION_DROP_TABLE, GetSettings(), tableName, CORE::CDataTableDefinition(), deleteData );
}

/*-------------------------------------------------------------------------*/

void
CDatabaseClientBase::RunOperation( EOperation operation, const CDatabaseClientSettings& settings, const CORE::CString& tableName,
                                   const CORE::CDataTableDefinition& definition, bool deleteData, CORE::CFutureResultSource& source )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CORE::CICloneable* resultData = GUCEF_NULL;
    CORE::CString failure;

    try
    {
        switch ( operation )
        {
            case OPERATION_OPEN:
            {
                if ( !Open( settings ) )
                    status = CORE::DATATABLE_STATUS_FAILED;
                break;
            }
            case OPERATION_LIST_TABLES:
            {
                CORE::CString::StringSet names;
                status = ListTables( names );
                if ( CORE::DATATABLE_STATUS_OK == status )
                    resultData = GUCEF_NEW CDatabaseTableNamesResult( names );
                break;
            }
            case OPERATION_OPEN_TABLE:
            {
                CORE::CIDataTablePtr table;
                status = OpenTable( tableName, table );
                if ( CORE::DATATABLE_STATUS_OK == status )
                    resultData = GUCEF_NEW CDatabaseTableResult( table );
                break;
            }
            case OPERATION_CREATE_TABLE:
            {
                CORE::CIDataTablePtr table;
                status = CreateTable( definition, table );
                if ( CORE::DATATABLE_STATUS_OK == status )
                    resultData = GUCEF_NEW CDatabaseTableResult( table );
                break;
            }
            case OPERATION_DROP_TABLE:
            {
                status = DropTable( tableName, deleteData );
                break;
            }
        }
    }
    catch ( const std::exception& e )
    {
        failure = CORE::CString( "EXCEPTION: " ) + e.what();
        status = CORE::DATATABLE_STATUS_FAILED;
    }
    catch ( ... )
    {
        failure = "EXCEPTION: an unknown exception";
        status = CORE::DATATABLE_STATUS_FAILED;
    }

    if ( CORE::DATATABLE_STATUS_OK == status )
    {
        if ( GUCEF_NULL != resultData )
            source.Complete( resultData, true );
        else
            source.Complete();
        return;
    }

    if ( failure.IsNULLOrEmpty() )
        failure = CORE::CString( CORE::DataTableStatusToString( status ) ) + ": " + GetLastErrorMessage();
    GUCEF_DELETE resultData;
    if ( CORE::DATATABLE_STATUS_CANCELLED == status )
        source.Cancel();
    else
        source.Fail( FailureStatusFor( status ), failure );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
