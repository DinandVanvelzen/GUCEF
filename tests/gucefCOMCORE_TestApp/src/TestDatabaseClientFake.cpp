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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#include "TestDatabaseClientFake.h"

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A table without rows: it holds the definition and refuses the rest */
class CTestFakeDataTable : public CORE::CIDataTable
{
    public:

    explicit CTestFakeDataTable( const CORE::CDataTableDefinition& definition, bool readOnly )
        : CORE::CIDataTable( CORE::PulseGeneratorPtr() )
        , m_definition( definition )
        , m_readOnly( readOnly )
    {GUCEF_TRACE;

    }

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_definition.name; }
    virtual const CORE::CDataTableDefinition& GetDefinition( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_definition; }
    virtual CORE::CDataTableSchemaPtr GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_definition.CreateSchemaPtr(); }
    virtual bool IsReadOnly( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_readOnly; }

    virtual CORE::TDataTableStatus Scan( const CORE::CDataTableQuery&, CORE::CDataTableScanPtr& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult ScanAsync( const CORE::CDataTableQuery&, CORE::CDataTableScanPtr&, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual CORE::TDataTableStatus Write( const CORE::CDataTableBatch&, CORE::TDataTableWriteMode, CORE::CDataTableWriteResult& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult WriteAsync( const CORE::CDataTableBatchPtr&, CORE::TDataTableWriteMode, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual CORE::TDataTableStatus DeleteRows( const CORE::CDataTableQuery&, CORE::CDataTableWriteResult& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult DeleteRowsAsync( const CORE::CDataTableQuery&, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual CORE::TDataTableStatus DeletePartitions( const CORE::CDataTableQuery&, CORE::CDataTableWriteResult& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult DeletePartitionsAsync( const CORE::CDataTableQuery&, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual CORE::TDataTableStatus ListPartitions( const CORE::CDataTableQuery&, bool, CORE::CDataTablePartitionInfo::TPartitionInfoVector& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult ListPartitionsAsync( const CORE::CDataTableQuery&, bool, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual CORE::TDataTableStatus Compact( const CORE::CDataTableQuery&, bool, CORE::CDataTableWriteResult& ) GUCEF_VIRTUAL_OVERRIDE { return CORE::DATATABLE_STATUS_UNSUPPORTED; }
    virtual CORE::CFutureResult CompactAsync( const CORE::CDataTableQuery&, bool, CORE::ThreadPoolPtr ) GUCEF_VIRTUAL_OVERRIDE { return Refused(); }
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CString(); }

    private:

    static CORE::CFutureResult Refused( void )
    {GUCEF_TRACE;

        return CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }

    CORE::CDataTableDefinition m_definition;
    bool m_readOnly;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CTestFakeDatabaseClient::BackendName = "fake";

/*-------------------------------------------------------------------------*/

CTestFakeDatabaseClient::CTestFakeDatabaseClient( void )
    : CDatabaseClientBase()
    , delayInMs( 0 )
    , m_lock()
    , m_tables()
    , m_errors()
    , m_callCount( 0 )
    , m_open( false )
{GUCEF_TRACE;

    for ( int i = 0; i < 5; ++i )
        m_threads[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

CTestFakeDatabaseClient::~CTestFakeDatabaseClient()
{GUCEF_TRACE;

    // The operations that are queued call into this object
    WaitForAsyncOperations();
}

/*-------------------------------------------------------------------------*/

void
CTestFakeDatabaseClient::Record( UInt32 kind )
{GUCEF_TRACE;

    {
        MT::CObjectScopeLock lock( &m_lock );
        m_threads[ kind ] = MT::GetCurrentTaskID();
        ++m_callCount;
        m_errors.erase( MT::GetCurrentTaskID() );
    }
    if ( delayInMs > 0 && 0 != kind )
        MT::ThreadDelay( delayInMs );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    {
        MT::CObjectScopeLock lock( &m_lock );
        m_errors[ MT::GetCurrentTaskID() ] = message;
    }
    CORE::TCloneableString eventData( message );
    NotifyObservers( DatabaseErrorEvent, &eventData );
    return status;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestFakeDatabaseClient::GetThreadOfLastCall( UInt32 kind ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return kind < 5 ? m_threads[ kind ] : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestFakeDatabaseClient::GetCallCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_callCount;
}

/*-------------------------------------------------------------------------*/

bool
CTestFakeDatabaseClient::Open( const CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    Record( 0 );
    CORE::CString error;
    if ( !settings.Validate( error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }
    if ( settings.backendName != BackendName )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The settings are for backend \"" + settings.backendName + "\"" );
        return false;
    }
    if ( settings.database == "unavailable" )
    {
        Fail( CORE::DATATABLE_STATUS_IO_ERROR, "The database \"unavailable\" cannot be opened" );
        return false;
    }

    SetSettings( settings );
    {
        MT::CObjectScopeLock lock( &m_lock );
        m_open = true;
    }
    NotifyObservers( DatabaseOpenedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestFakeDatabaseClient::Close( void )
{GUCEF_TRACE;

    WaitForAsyncOperations();
    bool wasOpen = false;
    {
        MT::CObjectScopeLock lock( &m_lock );
        wasOpen = m_open;
        m_open = false;
    }
    if ( wasOpen )
        NotifyObservers( DatabaseClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CTestFakeDatabaseClient::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestFakeDatabaseClient::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestFakeDatabaseClient::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_DATABASE_CAPABILITY_WRITE | GUCEF_DATABASE_CAPABILITY_ASYNC;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::ListTables( CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    Record( 1 );
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );

    tableNames.clear();
    for ( TDefinitionMap::const_iterator i = m_tables.begin(); i != m_tables.end(); ++i )
        tableNames.insert( (*i).first );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    Record( 1 );
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );

    TDefinitionMap::const_iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    definition = (*i).second;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    Record( 2 );
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );

    TDefinitionMap::const_iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    table = CORE::CIDataTablePtr( GUCEF_NEW CTestFakeDataTable( (*i).second, GetSettings().readOnly ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    Record( 3 );
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    if ( GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database is open read only" );

    CORE::CString error;
    if ( !definition.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    if ( m_tables.find( definition.name ) != m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_ALREADY_EXISTS, "The table \"" + definition.name + "\" exists" );

    m_tables[ definition.name ] = definition;
    table = CORE::CIDataTablePtr( GUCEF_NEW CTestFakeDataTable( definition, false ) );
    lock.EarlyUnlock();

    CORE::TCloneableString eventData( definition.name );
    NotifyObservers( TableCreatedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CTestFakeDatabaseClient::DropTable( const CORE::CString& tableName, bool deleteData )
{GUCEF_TRACE;

    Record( 4 );
    MT::CObjectScopeLock lock( &m_lock );
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    if ( GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database is open read only" );

    TDefinitionMap::iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\"" );
    m_tables.erase( i );
    lock.EarlyUnlock();

    CORE::TCloneableString eventData( tableName );
    NotifyObservers( TableDroppedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestFakeDatabaseClient::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( &m_lock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentTaskID() );
    return i == m_errors.end() ? CORE::CString() : (*i).second;
}

/*-------------------------------------------------------------------------*/
