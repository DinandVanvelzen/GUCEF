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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#include "gucefCORE_CDataTableAsyncOperation.h"
#define GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#endif /* GUCEF_CORE_CDATATABLEASYNCOPERATION_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE { class CDataTableBufferPool; }
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A table of the postgresql backend: a table of the schema of the database, reached through the connection of the
 *  client that opened it. Unpartitioned: the whole table is one partition with an empty path.
 *
 *  Scans send the projection and the predicates of the query to the server (a parameterised WHERE clause and the limit)
 *  and go through the shared CDataTableScan, which filters the rows again exactly; the rows of a scan are held in
 *  memory while it is read. Writes follow 7.1 of the data tables design within one unit of work, applied whole or
 *  not at all: INSERT adds the rows (CONFLICT for a primary key that exists or is given twice), UPSERT replaces rows
 *  by primary key with the last incoming row of a key winning (INVALID_ARGUMENT without a primary key), and
 *  REPLACE_PARTITIONS replaces all rows of the table. Unlike file backends, rowsWritten of the write result is the
 *  number of rows the call added or replaced, not the rows of the table after it (a count would cost a pass over the
 *  table). DeleteRows deletes the matching rows in one statement, DeletePartitions without predicates every row.
 *  Compact does nothing: PostgreSQL keeps its own storage (autovacuum).
 *
 *  Thread safe: statements are serialized on the connection of the client. Inside a transaction of the client the
 *  changes belong to it (see CIDatabaseClient); a failed change then undoes only itself (a savepoint).
 *  The last error message is kept per calling thread.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlDataTable : public CORE::CIDataTable
{
    public:

    CPostgreSqlDataTable( const CPostgreSqlConnectionPtr& connection    ,
                          const CORE::CDataTableDefinition& definition  ,
                          bool readOnly                                 ,
                          CORE::PulseGeneratorPtr pulseGenerator        );

    /** Waits for an async operation that is running; scans and batches handed out stay valid */
    virtual ~CPostgreSqlDataTable() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CDataTableDefinition& GetDefinition( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CDataTableSchemaPtr GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadOnly( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** The one partition is deleted (all rows) unless the filter has a predicate, which an unpartitioned table refuses with INVALID_ARGUMENT */
    virtual CORE::TDataTableStatus DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** The one partition when a row matches the filter; with file statistics its row count and the bytes of the table with its indexes */
    virtual CORE::TDataTableStatus ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Nothing to do: OK without partitions in the result */
    virtual CORE::TDataTableStatus Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Nothing is cached: does nothing */
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< TThreadId, CORE::CString > TErrorMap;

    CPostgreSqlDataTable( const CPostgreSqlDataTable& src );                /**< not implemented */
    CPostgreSqlDataTable& operator=( const CPostgreSqlDataTable& src );     /**< not implemented */

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    CORE::TDataTableStatus CheckWritable( void ) const;
    CORE::TDataTableStatus Resolve( const CORE::CDataTableQuery& query, CORE::CDataTableResolvedQuery& resolved ) const;
    CORE::TDataTableStatus InsertRows( const CORE::CDataTableBatch& rows, const GUCEF::vector< UInt32 >& rowIndices, bool upsert,
                                       UInt64& rowsReplaced, CORE::CString& error );
    CORE::CFutureResult QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool );
    CORE::CDataTableScanPtr MakeScan( const CORE::CDataTableResolvedQuery& resolved );

    CPostgreSqlConnectionPtr m_connection;
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    bool m_readOnly;
    CORE::CDataTableBufferPool* m_pool;
    CORE::CDataTableAsyncLinkPtr m_asyncLink;
    mutable MT::CMutex m_errorLock;
    mutable TErrorMap m_errors;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H ? */
