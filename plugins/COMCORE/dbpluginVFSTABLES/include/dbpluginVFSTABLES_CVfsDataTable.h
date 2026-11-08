/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#define DBPLUGIN_VFSTABLES_CVFSDATATABLE_H

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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H
#include "dbpluginVFSTABLES_CVfsTableScanSource.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A table of a vfstables database: partition files in the VFS, one per set of partition key values, at the path its
 *  template gives (section 14 of the data tables design). An unpartitioned table is the one file
 *  "<table name>.<codec extension>" at the root.
 *
 *  Scans prune partitions on their key values while discovering them (CVfsTableScanSource), then row groups on their
 *  statistics (CDataTableScan). Partition files are immutable: every change of a partition reads its rows, combines
 *  them with CDataTableMerge (7.2), writes the result through the codec to a temporary file next to the partition and
 *  moves it over the partition in one atomic step. A failure at any point leaves the partition as it was; a partition
 *  left without rows is deleted, and a missing partition file is an empty partition. Every row written is checked
 *  against the key values of its partition, so a value that cannot be expressed in a path (empty text, a NUL byte) is
 *  refused instead of landing in another partition.
 *
 *  Each partition changes atomically; a write over several partitions is not one transaction: the result lists the
 *  outcome per partition and the other partitions of a write go ahead when one fails. Changes of one table are
 *  serialized (also between two table objects of one table); scans run beside them and see each partition as it was
 *  before or after a change, never in between. One process writes a database (8).
 *
 *  Thread safe; the last error message is kept per calling thread.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsDataTable : public CORE::CIDataTable
{
    public:

    typedef CVfsTablesStore::TTableCodecPtr TTableCodecPtr;
    typedef GUCEF::vector< CORE::CDataTableBatchPtr > TBatchVector;

    CVfsDataTable( const TVfsTablesStorePtr& store                 ,
                   const CORE::CDataTableDefinition& definition    ,
                   const TTableCodecPtr& codec                     ,
                   CORE::PulseGeneratorPtr pulseGenerator          );

    /** Waits for an async operation that is running; scans and batches handed out stay valid */
    virtual ~CVfsDataTable() GUCEF_VIRTUAL_OVERRIDE;

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

    /**
     *  Deletes the partition files whose key values may match the predicates, whole: the predicates must be on
     *  partition source columns (INVALID_ARGUMENT otherwise), and a partition whose period only overlaps a range goes
     *  with all its rows. No predicates deletes every partition
     */
    virtual CORE::TDataTableStatus DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Partitions whose key values may match; with predicates on other columns, or with file statistics, also those whose row group statistics may */
    virtual CORE::TDataTableStatus ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Rewrites the partitions in the canonical form (7.2), those whose layout metadata says they are in it only with force */
    virtual CORE::TDataTableStatus Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool = CORE::ThreadPoolPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    /** Forgets the cached listings and footers of the database */
    virtual void Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The file of an unpartitioned table, relative to the database root: "<name>.<extension>" */
    const CORE::CString& GetUnpartitionedFile( void ) const;

    /** The storage counters of the database the table is in */
    void GetStorageStats( CVfsTablesStats& stats ) const;

    TVfsTablesStorePtr GetStore( void ) const;

    private:

    struct SPartitionRows
    {
        CORE::CString relativePath;
        CORE::CValueList keyValues;
        CORE::CDataTableBatchPtr rows;      /**< a full batch of the rows of the partition */
    };
    typedef GUCEF::vector< SPartitionRows > TPartitionRowsVector;
    typedef GUCEF::map< TThreadId, CORE::CString > TLastErrorMap;

    struct SRowOrder;

    CVfsDataTable( const CVfsDataTable& src );              /**< not implemented */
    CVfsDataTable& operator=( const CVfsDataTable& src );   /**< not implemented */

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    CORE::TDataTableStatus CheckWritable( void ) const;
    CORE::TDataTableStatus Resolve( const CORE::CDataTableQuery& query, CORE::CDataTableResolvedQuery& resolved ) const;
    CORE::TDataTableStatus CheckPartitionFilter( const CORE::CDataTableQuery& partitionFilter ) const;
    CORE::CDataTableScanSourcePtr MakeSource( const CORE::CDataTableResolvedQuery& resolved ) const;

    /** The partition files the resolved query may need, in path order, without opening them */
    void FindPartitions( const CORE::CDataTableResolvedQuery& resolved, CVfsTableScanSource::TPartitionFileVector& partitions ) const;

    /** Splits normalized rows by partition, each a full batch of its rows in their order */
    CORE::TDataTableStatus RouteRows( const CORE::CDataTableBatchPtr& normalized, TPartitionRowsVector& partitions, CORE::CString& error ) const;

    /** Every row of the batches belongs to the partition at the path */
    CORE::TDataTableStatus CheckRowsOfPartition( const TBatchVector& batches, const CORE::CString& relativePath, CORE::CString& error ) const;

    /** The rows of one partition as full batches; none for a partition without a file */
    CORE::TDataTableStatus ReadPartition( const CVfsTableScanSource::SPartitionFile& partition, TBatchVector& batches, CORE::CString& error ) const;

    /** Puts the rows in place as the new file of the partition, or deletes the partition when there are none */
    CORE::TDataTableStatus WritePartitionFile( const CVfsTableScanSource::SPartitionFile& partition, const TBatchVector& batches,
                                               UInt64 rowsReplaced, UInt64 rowsDeleted, bool existed, CORE::CDataTableWriteResult& result );

    void AddFailedPartition( const CORE::CString& relativePath, CORE::TDataTableStatus status, const CORE::CString& message,
                             CORE::CDataTableWriteResult& result ) const;
    CORE::TDataTableStatus EndChange( CORE::CDataTableWriteResult& result ) const;

    CORE::CFutureResult QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool );

    mutable TVfsTablesStorePtr m_store;     /**< mutable: the store is thread safe, its calls are not const */
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    TTableCodecPtr m_codec;
    CORE::CValueList m_codecOptions;
    CORE::CString m_unpartitionedFile;
    CORE::CString m_layoutText;
    MT::CMutex& m_writeLock;
    CORE::CDataTableAsyncLinkPtr m_asyncLink;
    mutable MT::CMutex m_errorLock;
    mutable TLastErrorMap m_lastErrors;     /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CVfsDataTable, MT::CMutex >      CVfsDataTablePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSDATATABLE_H ? */
