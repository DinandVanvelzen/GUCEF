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

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#include "TestDatabaseClientTransactional.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Every row of the table as the batches of a scan */
CORE::TDataTableStatus
CopyRows( CORE::CIDataTable& table, GUCEF::vector< CORE::CDataTableBatchPtr >& batches )
{GUCEF_TRACE;

    batches.clear();
    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = table.Scan( CORE::CDataTableQuery(), scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    while ( true )
    {
        CORE::CDataTableBatchPtr batch;
        status = scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_END == status )
            return CORE::DATATABLE_STATUS_OK;
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        batches.push_back( batch );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CTestTransactionalDatabaseClient::BackendName = "faketransactional";

/*-------------------------------------------------------------------------*/

CTestTransactionalDatabaseClient::CTestTransactionalDatabaseClient( void )
    : CDatabaseClientBase()
    , m_lock()
    , m_tables()
    , m_tablesAtBegin()
    , m_rowsAtBegin()
    , m_errors()
    , m_lastTransactionThread( 0 )
    , m_commits( 0 )
    , m_rollbacks( 0 )
    , m_inTransaction( false )
    , m_open( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestTransactionalDatabaseClient::~CTestTransactionalDatabaseClient()
{GUCEF_TRACE;

    // The operations that are queued call into this object
    WaitForAsyncOperations();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    m_errors[ MT::GetCurrentThreadID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::Succeed( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    m_errors.erase( MT::GetCurrentThreadID() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CTestTransactionalDatabaseClient::RecordTransactionCall( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    m_lastTransactionThread = MT::GetCurrentThreadID();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestTransactionalDatabaseClient::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentThreadID() );
    return i == m_errors.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CTestTransactionalDatabaseClient::Open( const CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    CORE::CString error;
    if ( !settings.Validate( error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }
    SetSettings( settings );
    {
        MT::CObjectScopeLock lock( &m_lock );
        m_open = true;
    }
    Succeed();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestTransactionalDatabaseClient::Close( void )
{GUCEF_TRACE;

    WaitForAsyncOperations();
    MT::CObjectScopeLock lock( &m_lock );
    if ( m_inTransaction )
    {
        RestoreLocked();
        m_inTransaction = false;
        ++m_rollbacks;
    }
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CTestTransactionalDatabaseClient::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestTransactionalDatabaseClient::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestTransactionalDatabaseClient::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_DATABASE_CAPABILITY_WRITE | GUCEF_DATABASE_CAPABILITY_ASYNC | GUCEF_DATABASE_CAPABILITY_TRANSACTIONS;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::ListTables( CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    tableNames.clear();
    for ( TTableMap::const_iterator i = m_tables.begin(); i != m_tables.end(); ++i )
        tableNames.insert( (*i).first );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    TTableMap::const_iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    definition = (*i).second->GetDefinition();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    TTableMap::const_iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    table = (*i).second;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    CORE::CString error;
    if ( !definition.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    if ( m_tables.find( definition.name ) != m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_ALREADY_EXISTS, "The table \"" + definition.name + "\" exists" );
    table = CORE::CIDataTablePtr( GUCEF_NEW CORE::CMemoryDataTable( definition ) );
    m_tables[ definition.name ] = table;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::DropTable( const CORE::CString& tableName, bool deleteData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    TTableMap::iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    m_tables.erase( i );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::BeginTransaction( void )
{GUCEF_TRACE;

    RecordTransactionCall();
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    if ( m_inTransaction )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "A transaction is open already" );
    m_tablesAtBegin = m_tables;
    m_rowsAtBegin.clear();
    for ( TTableMap::iterator i = m_tables.begin(); i != m_tables.end(); ++i )
    {
        const CORE::TDataTableStatus status = CopyRows( *(*i).second, m_rowsAtBegin[ (*i).first ] );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, "Copying the rows of \"" + (*i).first + "\" failed" );
    }
    m_inTransaction = true;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::CommitTransaction( void )
{GUCEF_TRACE;

    RecordTransactionCall();
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_inTransaction )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "No transaction is open" );
    m_inTransaction = false;
    m_tablesAtBegin.clear();
    m_rowsAtBegin.clear();
    ++m_commits;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::RollbackTransaction( void )
{GUCEF_TRACE;

    RecordTransactionCall();
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_inTransaction )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "No transaction is open" );
    m_inTransaction = false;
    ++m_rollbacks;
    return RestoreLocked();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestTransactionalDatabaseClient::RestoreLocked( void )
{GUCEF_TRACE;

    // the tables and their rows as they were at the begin, in the same table objects
    m_tables = m_tablesAtBegin;
    CORE::TDataTableStatus result = CORE::DATATABLE_STATUS_OK;
    for ( TTableMap::iterator i = m_tables.begin(); i != m_tables.end(); ++i )
    {
        CORE::CIDataTable& table = *(*i).second;
        CORE::CDataTableWriteResult writeResult;
        CORE::TDataTableStatus status = table.DeleteRows( CORE::CDataTableQuery(), writeResult );
        const TBatchVector& rows = m_rowsAtBegin[ (*i).first ];
        for ( UInt32 b=0; b<(UInt32) rows.size() && CORE::DATATABLE_STATUS_OK == status; ++b )
            status = table.Write( *rows[ b ], CORE::DATATABLE_WRITE_INSERT, writeResult );
        if ( CORE::DATATABLE_STATUS_OK != status )
            result = status;
    }
    m_tablesAtBegin.clear();
    m_rowsAtBegin.clear();
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CTestTransactionalDatabaseClient::IsInTransaction( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_inTransaction;
}

/*-------------------------------------------------------------------------*/

TThreadId
CTestTransactionalDatabaseClient::GetThreadOfLastTransactionCall( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_lastTransactionThread;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestTransactionalDatabaseClient::GetCommitCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_commits;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestTransactionalDatabaseClient::GetRollbackCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_rollbacks;
}

/*-------------------------------------------------------------------------*/
