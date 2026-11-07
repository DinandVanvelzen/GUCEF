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

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#define GUCEF_CORE_CMEMORYDATATABLE_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDataTableBufferPool;
class CMemoryDataTableData;

/**
 *  A table held in memory as full batches of its schema (section 4.5 of the data tables design): the reference
 *  implementation of CIDataTable, for tests, small derived results and as a cache.
 *
 *  The whole table is one partition, without key values and with an empty path. Each batch is a row group of at most
 *  storage.rowGroupRowCount rows of the definition, with exact column statistics made when it is stored, so scans
 *  prune row groups like any backend; scans go through the shared CDataTableScan and slice the stored batches without
 *  copying them. Writes follow 7.1: INSERT appends (refusing an existing primary key with CONFLICT), UPSERT replaces
 *  rows by primary key, REPLACE_PARTITIONS replaces all rows; with a primary key or a sort key the rows are kept
 *  sorted by the sort key and the batches are made anew, without them an INSERT only adds batches.
 *
 *  Thread safe: a scan works on the rows as they were when it started, writes are serialized and replace the rows
 *  at once. The last error message is kept per calling thread.
 */
class GUCEF_CORE_PUBLIC_CPP CMemoryDataTable : public CIDataTable
{
    public:

    /**
     *  A table of the definition; buffers and batches come from the given pool, the global one of CCoreGlobal when
     *  NULL. The definition is checked for its name, schema and writer options (the codec type is not used); a table
     *  of an invalid definition refuses every call with INVALID_ARGUMENT.
     */
    explicit CMemoryDataTable( const CDataTableDefinition& definition           ,
                               CDataTableBufferPool* bufferPool = GUCEF_NULL    ,
                               PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );

    /** Waits for an async operation that is running; scans and batches handed out stay valid */
    virtual ~CMemoryDataTable() GUCEF_VIRTUAL_OVERRIDE;

    /** False when the definition was refused, see GetLastErrorMessage() */
    bool IsValid( void ) const;

    /** The rows held */
    UInt64 GetRowCount( void ) const;

    /** The stored batches (row groups) */
    UInt32 GetBatchCount( void ) const;

    /** A table that refuses changes with READ_ONLY */
    void SetReadOnly( bool readOnly );

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CDataTableDefinition& GetDefinition( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CDataTableSchemaPtr GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadOnly( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus Scan( const CDataTableQuery& query, CDataTableScanPtr& scan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult ScanAsync( const CDataTableQuery& query, CDataTableScanPtr& scan, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus Write( const CDataTableBatch& rows, TDataTableWriteMode mode, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult WriteAsync( const CDataTableBatchPtr& rows, TDataTableWriteMode mode, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus DeleteRows( const CDataTableQuery& predicates, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult DeleteRowsAsync( const CDataTableQuery& predicates, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** The one partition is deleted (all rows) unless a predicate is on a column that is not a partition source column: INVALID_ARGUMENT */
    virtual TDataTableStatus DeletePartitions( const CDataTableQuery& partitionFilter, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult DeletePartitionsAsync( const CDataTableQuery& partitionFilter, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** The one partition when it holds rows that may match the filter; file statistics are the bytes held and the row groups */
    virtual TDataTableStatus ListPartitions( const CDataTableQuery& partitionFilter, bool withFileStats, CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult ListPartitionsAsync( const CDataTableQuery& partitionFilter, bool withFileStats, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the rows canonical (7.2) when an INSERT appended batches since, or always with force */
    virtual TDataTableStatus Compact( const CDataTableQuery& partitionFilter, bool force, CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CFutureResult CompactAsync( const CDataTableQuery& partitionFilter, bool force, ThreadPoolPtr threadPool = ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Nothing is cached: does nothing */
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CTSharedPtr< CMemoryDataTableData, MT::CMutex > TDataPtr;
    typedef GUCEF::map< UInt32, CString > TLastErrorMap;

    CMemoryDataTable( const CMemoryDataTable& src );                /**< not implemented */
    CMemoryDataTable& operator=( const CMemoryDataTable& src );     /**< not implemented */

    TDataTableStatus Fail( TDataTableStatus status, const CString& message ) const;
    TDataTableStatus Succeed( void ) const;
    TDataTableStatus CheckWritable( void ) const;
    TDataPtr GetData( void ) const;
    TDataTableStatus StoreRows( const GUCEF::vector< CDataTableBatchPtr >& batches, bool canonical, UInt64 rowsReplaced, UInt64 rowsDeleted, CDataTableWriteResult& result );
    CFutureResult QueueOperation( CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool );

    CDataTableDefinition m_definition;
    CDataTableSchemaPtr m_schema;
    CDataTableBufferPool* m_pool;
    bool m_valid;
    bool m_readOnly;
    MT::CMutex m_writeLock;                 /**< serializes the changes */
    mutable MT::CMutex m_dataLock;          /**< guards the swap of m_data */
    TDataPtr m_data;
    CDataTableAsyncLinkPtr m_asyncLink;
    mutable MT::CMutex m_errorLock;
    mutable TLastErrorMap m_lastErrors;     /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CMemoryDataTable, MT::CMutex >  CMemoryDataTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */
