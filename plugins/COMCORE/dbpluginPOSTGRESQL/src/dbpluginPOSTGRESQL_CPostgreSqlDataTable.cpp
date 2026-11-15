/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#include "gucefCORE_CDataTableMerge.h"
#define GUCEF_CORE_CDATATABLEMERGE_H
#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H
#include "dbpluginPOSTGRESQL_CPostgreSqlScanSource.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlDataTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The text of the primary key of a row, for finding rows of the same key */
CORE::CString
KeyOfRow( const CORE::CDataTableBatch& rows, const CORE::CDataTableSchema& schema, UInt32 row )
{
    CPostgreSqlParameters parameters;
    CORE::CString key;
    const CORE::CDataTableSchema::TColumnIndexVector& primaryKey = schema.GetPrimaryKey();
    for ( UInt32 k=0; k<(UInt32) primaryKey.size(); ++k )
    {
        const UInt32 number = CPostgreSqlValues::AddCellParameter( schema.GetColumn( primaryKey[ k ] ), rows.GetColumn( primaryKey[ k ] ), row, parameters );
        key += CORE::ToString( parameters.GetDisplayText( number ).Length() ) + ":" + parameters.GetDisplayText( number ) + ";";
    }
    return key;
}

/*-------------------------------------------------------------------------*/

/** The first value of a one row result as a count */
UInt64
CountOfResult( const CPostgreSqlResult& result )
{
    UInt64 count = 0;
    if ( result.GetRowCount() > 0 && !result.IsNull( 0, 0 ) )
        CPostgreSqlValues::ParseUInt64( result.GetValue( 0, 0 ), result.GetLength( 0, 0 ), count );
    return count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlDataTable::CPostgreSqlDataTable( const CPostgreSqlConnectionPtr& connection    ,
                                            const CORE::CDataTableDefinition& definition  ,
                                            bool readOnly                                 ,
                                            CORE::PulseGeneratorPtr pulseGenerator        )
    : CORE::CIDataTable( pulseGenerator )
    , m_connection( connection )
    , m_definition( definition )
    , m_schema( definition.CreateSchemaPtr() )
    , m_readOnly( readOnly )
    , m_pool( &CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() )
    , m_asyncLink()
    , m_errorLock()
    , m_errors()
{GUCEF_TRACE;

    m_asyncLink = CORE::CDataTableAsyncLinkPtr( GUCEF_NEW CORE::CDataTableAsyncLink( this ) );
    CPostgreSqlClient::OnObjectCreated( "table" );
}

/*-------------------------------------------------------------------------*/

CPostgreSqlDataTable::~CPostgreSqlDataTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_asyncLink->Detach();
    CPostgreSqlClient::OnObjectDestroyed( "table" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    m_errors[ MT::GetCurrentThreadID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TErrorMap::iterator i = m_errors.find( MT::GetCurrentThreadID() );
    if ( i != m_errors.end() )
        m_errors.erase( i );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlDataTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentThreadID() );
    return i != m_errors.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPostgreSqlDataTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableDefinition&
CPostgreSqlDataTable::GetDefinition( void ) const
{GUCEF_TRACE;

    return m_definition;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableSchemaPtr
CPostgreSqlDataTable::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlDataTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return m_readOnly;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::CheckWritable( void ) const
{GUCEF_TRACE;

    if ( m_readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Table \"" + m_definition.name + "\" is read only" );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Resolve( const CORE::CDataTableQuery& query, CORE::CDataTableResolvedQuery& resolved ) const
{GUCEF_TRACE;

    resolved.schema = m_schema;
    CORE::CString error;
    if ( !query.Resolve( *m_schema, resolved, error ) )
        return Fail( CORE::DATATABLE_STATUS_OK != resolved.status ? resolved.status : CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableScanPtr
CPostgreSqlDataTable::MakeScan( const CORE::CDataTableResolvedQuery& resolved )
{GUCEF_TRACE;

    CORE::CDataTableScanSourcePtr source( GUCEF_NEW CPostgreSqlScanSource( m_connection, m_definition, m_schema, resolved ) );
    return CORE::CDataTableScanPtr( GUCEF_NEW CORE::CDataTableScan( source, m_schema, *m_pool, GetPulseGenerator() ) );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    CORE::CDataTableResolvedQuery resolved;
    CORE::TDataTableStatus status = Resolve( query, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    scan = MakeScan( resolved );
    status = scan->Start( query );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, scan->GetLastErrorMessage() );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    scan.Unlink();
    CORE::CDataTableResolvedQuery resolved;
    const CORE::TDataTableStatus status = Resolve( query, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        CORE::CFutureResultSource source;
        source.Fail( CORE::TASKSTATUS_TASKDATA_INVALID, CORE::CString( CORE::DataTableStatusToString( status ) ) + ": " + GetLastErrorMessage() );
        return source.GetFuture();
    }
    scan = MakeScan( resolved );
    return scan->StartAsync( query, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::InsertRows( const CORE::CDataTableBatch& rows, const GUCEF::vector< UInt32 >& rowIndices, bool upsert,
                                  UInt64& rowsReplaced, CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CDataTableSchema& schema = *m_schema;
    const UInt32 columnCount = schema.GetColumnCount();
    UInt32 rowsPerStatement = CPostgreSqlParameters::MaxCount / columnCount;
    if ( rowsPerStatement > m_connection->GetSettings().maxRowsPerStatement )
        rowsPerStatement = m_connection->GetSettings().maxRowsPerStatement;
    if ( 0 == rowsPerStatement )
        rowsPerStatement = 1;

    const CORE::CString prefix = CPostgreSqlSql::MakeInsertPrefix( m_definition, m_connection->GetSettings().schema );
    const CORE::CString suffix = upsert ? CPostgreSqlSql::MakeUpsertSuffix( m_definition ) : CORE::CString::Empty;
    const bool keyOnly = schema.GetPrimaryKey().size() == columnCount;

    UInt32 next = 0;
    while ( next < (UInt32) rowIndices.size() )
    {
        UInt32 count = (UInt32) rowIndices.size() - next;
        if ( count > rowsPerStatement )
            count = rowsPerStatement;

        CPostgreSqlParameters parameters;
        CORE::CString sql = prefix;
        for ( UInt32 r=0; r<count; ++r )
        {
            const UInt32 row = rowIndices[ next + r ];
            const UInt32 first = parameters.GetCount() + 1;
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                if ( 0 == CPostgreSqlValues::AddCellParameter( schema.GetColumn( c ), rows.GetColumn( c ), row, parameters ) )
                {
                    error = "The value of column \"" + schema.GetColumn( c ).name + "\" in row " + CORE::ToString( row ) + " cannot be sent to PostgreSQL (text with a NUL byte)";
                    return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
                }
            }
            if ( r > 0 )
                sql += ", ";
            sql += CPostgreSqlSql::MakeInsertRow( m_definition, first );
        }
        sql += suffix;

        CPostgreSqlResult result;
        const CORE::TDataTableStatus status = m_connection->Execute( sql, parameters, result, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        if ( upsert )
        {
            if ( keyOnly )
            {
                // DO NOTHING returns only the rows it added
                rowsReplaced += count - result.GetRowCount();
            }
            else
            {
                for ( UInt32 r=0; r<result.GetRowCount(); ++r )
                {
                    if ( 't' == result.GetValue( r, 0 )[ 0 ] )
                        ++rowsReplaced;
                }
            }
        }
        next += count;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( CORE::DATATABLE_WRITE_UPSERT == mode && m_schema->GetPrimaryKey().empty() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "Table \"" + m_definition.name + "\" has no primary key to upsert on" );

    CORE::CString error;
    CORE::CDataTableBatchPtr normalized;
    status = CORE::CDataTableMerge::Normalize( m_schema, rows, *m_pool, normalized, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    // the rows to send: for an upsert the last row of each key
    const UInt32 rowCount = normalized->GetRowCount();
    GUCEF::vector< UInt32 > rowIndices;
    rowIndices.reserve( rowCount );
    if ( CORE::DATATABLE_WRITE_UPSERT == mode )
    {
        GUCEF::map< CORE::CString, UInt32 > lastOfKey;
        for ( UInt32 r=0; r<rowCount; ++r )
            lastOfKey[ KeyOfRow( *normalized, *m_schema, r ) ] = r;
        for ( UInt32 r=0; r<rowCount; ++r )
        {
            if ( lastOfKey[ KeyOfRow( *normalized, *m_schema, r ) ] == r )
                rowIndices.push_back( r );
        }
    }
    else
    {
        for ( UInt32 r=0; r<rowCount; ++r )
            rowIndices.push_back( r );
    }

    UInt64 rowsReplaced = 0;
    UInt64 rowsDeleted = 0;
    {
        MT::CScopeMutex lock( m_connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = m_connection->BeginWork( error );
        if ( CORE::DATATABLE_STATUS_OK == status )
        {
            if ( CORE::DATATABLE_WRITE_REPLACE_PARTITIONS == mode )
            {
                CPostgreSqlParameters none;
                CPostgreSqlResult deleted;
                status = m_connection->Execute( "DELETE FROM " + CPostgreSqlSql::QualifiedName( m_connection->GetSettings().schema, m_definition.name ), none, deleted, error );
                rowsDeleted = deleted.GetAffectedRowCount();
            }
            if ( CORE::DATATABLE_STATUS_OK == status )
                status = InsertRows( *normalized, rowIndices, CORE::DATATABLE_WRITE_UPSERT == mode, rowsReplaced, error );
            CORE::CString endError;
            const CORE::TDataTableStatus endStatus = m_connection->EndWork( CORE::DATATABLE_STATUS_OK == status, endError );
            if ( CORE::DATATABLE_STATUS_OK == status && CORE::DATATABLE_STATUS_OK != endStatus )
            {
                status = endStatus;
                error = endError;
            }
        }
    }

    CORE::CDataTableWriteResult::SPartitionResult partition;
    partition.path = CORE::CString::Empty;
    partition.status = status;
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        partition.message = error;
        result.partitions.push_back( partition );
        return Fail( status, "Writing to table \"" + m_definition.name + "\" failed: " + error );
    }
    partition.rowsWritten = (UInt64) rowIndices.size();
    partition.rowsReplaced = rowsReplaced;
    partition.rowsDeleted = rowsDeleted;
    result.partitions.push_back( partition );

    CORE::CDataTablePartitionInfo info;
    info.path = CORE::CString::Empty;
    info.rowCount = GUCEF_UINT64MAX;
    NotifyObserversFromThread( PartitionWrittenEvent, &info );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CDataTableResolvedQuery resolved;
    status = Resolve( predicates, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CString error;
    CPostgreSqlParameters parameters;
    CORE::CString sql;
    status = CPostgreSqlSql::MakeDelete( m_definition, m_connection->GetSettings().schema, resolved, parameters, sql, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    CPostgreSqlResult deleted;
    {
        MT::CScopeMutex lock( m_connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = m_connection->Execute( sql, parameters, deleted, error );
    }
    CORE::CDataTableWriteResult::SPartitionResult partition;
    partition.path = CORE::CString::Empty;
    partition.status = status;
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        partition.message = error;
        result.partitions.push_back( partition );
        return Fail( status, "Deleting rows of table \"" + m_definition.name + "\" failed: " + error );
    }
    partition.rowsDeleted = deleted.GetAffectedRowCount();
    result.partitions.push_back( partition );
    if ( partition.rowsDeleted > 0 )
    {
        CORE::CDataTablePartitionInfo info;
        info.path = CORE::CString::Empty;
        info.rowCount = GUCEF_UINT64MAX;
        NotifyObserversFromThread( PartitionWrittenEvent, &info );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( !partitionFilter.GetPredicates().empty() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "Table \"" + m_definition.name + "\" is not partitioned: a partition filter cannot name a column" );

    CORE::CString error;
    CPostgreSqlResult deleted;
    {
        MT::CScopeMutex lock( m_connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = m_connection->Execute( "DELETE FROM " + CPostgreSqlSql::QualifiedName( m_connection->GetSettings().schema, m_definition.name ),
                                        CPostgreSqlParameters(), deleted, error );
    }
    CORE::CDataTableWriteResult::SPartitionResult partition;
    partition.path = CORE::CString::Empty;
    partition.status = status;
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        partition.message = error;
        result.partitions.push_back( partition );
        return Fail( status, "Deleting the rows of table \"" + m_definition.name + "\" failed: " + error );
    }
    partition.rowsDeleted = deleted.GetAffectedRowCount();
    result.partitions.push_back( partition );
    if ( partition.rowsDeleted > 0 )
    {
        CORE::CDataTablePartitionInfo info;
        info.path = CORE::CString::Empty;
        NotifyObserversFromThread( PartitionDeletedEvent, &info );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    CORE::CDataTableResolvedQuery resolved;
    CORE::TDataTableStatus status = Resolve( partitionFilter, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CString error;
    CPostgreSqlParameters parameters;
    CORE::CString sql;
    status = CPostgreSqlSql::MakeCount( m_definition, m_connection->GetSettings().schema, resolved, parameters, sql, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    MT::CScopeMutex lock( m_connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CPostgreSqlResult counted;
    status = m_connection->Execute( sql, parameters, counted, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Counting the rows of table \"" + m_definition.name + "\" failed: " + error );
    const UInt64 matching = CountOfResult( counted );
    if ( 0 == matching )
        return Succeed();

    CORE::CDataTablePartitionInfo info;
    info.path = CORE::CString::Empty;
    info.rowGroupCount = 1;
    info.rowCount = GUCEF_UINT64MAX;
    if ( withFileStats )
    {
        UInt64 total = matching;
        if ( !resolved.predicates.empty() )
        {
            CPostgreSqlResult all;
            status = m_connection->Execute( "SELECT count(*) FROM " + CPostgreSqlSql::QualifiedName( m_connection->GetSettings().schema, m_definition.name ),
                                            CPostgreSqlParameters(), all, error );
            if ( CORE::DATATABLE_STATUS_OK != status )
                return Fail( status, "Counting the rows of table \"" + m_definition.name + "\" failed: " + error );
            total = CountOfResult( all );
        }
        CPostgreSqlParameters sizeParameters;
        sizeParameters.AddText( CPostgreSqlSql::QualifiedName( m_connection->GetSettings().schema, m_definition.name ) );
        CPostgreSqlResult size;
        status = m_connection->Execute( "SELECT pg_total_relation_size(to_regclass($1::text))", sizeParameters, size, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, "The size of table \"" + m_definition.name + "\" cannot be read: " + error );
        info.hasFileStats = true;
        info.rowCount = total;
        info.fileSize = CountOfResult( size );
        info.modificationTime = CORE::CDateTime::NowUTCDateTime();
    }
    partitions.push_back( info );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlDataTable::Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    return Succeed();
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlDataTable::Refresh( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    data.table = m_asyncLink;
    return CORE::CDataTableAsyncOperation::Queue( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_WRITE;
    data.rows = rows;
    data.writeMode = mode;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_ROWS;
    data.query = predicates;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS;
    data.query = partitionFilter;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS;
    data.query = partitionFilter;
    data.flag = withFileStats;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CPostgreSqlDataTable::CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_COMPACT;
    data.query = partitionFilter;
    data.flag = force;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPostgreSqlDataTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DBPLUGIN::POSTGRESQL::CPostgreSqlDataTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
