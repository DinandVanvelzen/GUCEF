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

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#include "gucefCORE_CMemoryDataTable.h"

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

/** The rows of a memory table at one moment: immutable once made, shared by the table and its scans */
class CMemoryDataTableData
{
    public:

    typedef GUCEF::vector< CDataTableBatchPtr > TBatchVector;
    typedef GUCEF::vector< CDataTableRowGroupInfo > TRowGroupInfoVector;

    CDataTableSchemaPtr schema;
    TBatchVector batches;                   /**< full batches of the schema, one per row group */
    TRowGroupInfoVector rowGroups;          /**< their exact statistics */
    UInt64 rowCount;
    UInt64 byteCount;                       /**< bytes of the values held, for the partition statistics */
    bool canonical;                         /**< sorted, deduplicated and cut by the writer options */
    CDateTime modificationTime;

    CMemoryDataTableData( void ) : schema(), batches(), rowGroups(), rowCount( 0 ), byteCount( 0 ), canonical( true ), modificationTime() {}
};

/*-------------------------------------------------------------------------*/

namespace {

typedef CTSharedPtr< CMemoryDataTableData, MT::CMutex > TMemoryDataPtr;

/** The rows of a memory table as the metadata of the one "file" of the table */
class CMemoryDataTableMetaData : public CITableFileMetaData
{
    public:

    explicit CMemoryDataTableMetaData( const TMemoryDataPtr& data ) : CITableFileMetaData(), m_data( data ) {}

    virtual const CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_data->schema; }
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_data->rowCount; }
    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) m_data->batches.size(); }

    virtual bool GetRowGroupInfo( UInt32 rowGroup, CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( rowGroup >= (UInt32) m_data->rowGroups.size() )
            return false;
        info = m_data->rowGroups[ rowGroup ];
        return true;
    }

    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE { return sizeof( *this ); }
    virtual void GetKeyValueMetaData( CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE { keyValues.Clear(); }

    private:

    TMemoryDataPtr m_data;
};

/*-------------------------------------------------------------------------*/

/** Reads row groups of a memory table: batches are slices of the stored batches, no value is copied */
class CMemoryDataTableReader : public CITableReader
{
    public:

    CMemoryDataTableReader( const TMemoryDataPtr& data, const CITableFileMetaDataPtr& metaData )
        : CITableReader()
        , m_data( data )
        , m_metaData( metaData )
        , m_request( GUCEF_NULL )
        , m_rowGroup( 0 )
        , m_position( 0 )
        , m_error()
    {
    }

    virtual const CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData; }

    virtual TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_request = GUCEF_NULL;
        if ( rowGroup >= (UInt32) m_data->batches.size() )
        {
            m_error = "Row group " + ToString( rowGroup ) + " does not exist";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        const UInt32 columnCount = m_data->schema->GetColumnCount();
        for ( UInt32 i=0; i<(UInt32) request.projection.size(); ++i )
        {
            if ( request.projection[ i ] >= columnCount )
            {
                m_error = "The projection names column " + ToString( request.projection[ i ] ) + ", which does not exist";
                return DATATABLE_STATUS_INVALID_ARGUMENT;
            }
        }
        if ( GUCEF_NULL == request.bufferPool )
        {
            m_error = "The read request has no buffer pool";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        m_request = &request;
        m_rowGroup = rowGroup;
        m_position = 0;
        return DATATABLE_STATUS_OK;
    }

    virtual TDataTableStatus ReadNextBatch( CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE
    {
        batch.Reset();
        if ( GUCEF_NULL == m_request )
        {
            m_error = "No row group is open";
            return DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        const CDataTableBatch& stored = *m_data->batches[ m_rowGroup ];
        const UInt32 rowCount = stored.GetRowCount();
        if ( m_position >= rowCount )
            return DATATABLE_STATUS_END;
        UInt32 count = rowCount - m_position;
        if ( count > m_request->batchRowCount )
            count = m_request->batchRowCount;

        CDataTableBufferPool& pool = *m_request->bufferPool;
        TDataTableStatus status = pool.AcquireBatch( batch );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        CDataTableBatch* writable = batch.GetWritable();
        const UInt32 columnCount = (UInt32) m_request->projection.size();
        if ( !writable->SetColumnCount( columnCount ) )
            return DATATABLE_STATUS_OUT_OF_MEMORY;
        writable->SetSchema( m_data->schema );
        writable->SetRowCount( count );
        for ( UInt32 i=0; i<columnCount; ++i )
        {
            const UInt32 column = m_request->projection[ i ];
            status = writable->GetMutableColumn( i ).SetSlice( stored.GetColumn( column ), m_position, count, pool );
            if ( DATATABLE_STATUS_OK != status )
            {
                m_error = "Slicing column " + ToString( column ) + " failed: " + DataTableStatusToString( status );
                batch.Reset();
                return status;
            }
            writable->SetSchemaColumnIndex( i, column );
        }
        CDataTableBatchOrigin origin;
        origin.fileOrdinal = m_request->fileOrdinal;
        origin.rowGroup = m_rowGroup;
        origin.firstRowInRowGroup = m_position;
        writable->SetOrigin( origin );
        if ( GUCEF_NULL != m_request->stats )
            m_request->stats->pagesInPlace += columnCount;
        m_position += count;
        return DATATABLE_STATUS_OK;
    }

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_error; }

    private:

    TMemoryDataPtr m_data;
    CITableFileMetaDataPtr m_metaData;
    const CDataTableReadRequest* m_request;
    UInt32 m_rowGroup;
    UInt32 m_position;
    CString m_error;
};

/*-------------------------------------------------------------------------*/

/** The one "file" of a memory table: the rows as they were when the scan was made */
class CMemoryDataTableScanSource : public CIDataTableScanSource
{
    public:

    CMemoryDataTableScanSource( const TMemoryDataPtr& data, const CString& name )
        : CIDataTableScanSource()
        , m_data( data )
        , m_metaData()
        , m_name( name )
        , m_listed( false )
    {
        CTSharedPtr< CMemoryDataTableMetaData, MT::CMutex > metaData( GUCEF_NEW CMemoryDataTableMetaData( data ) );
        m_metaData = CITableFileMetaDataPtr( metaData );
    }

    virtual TDataTableStatus NextFile( CDataTableScanFile& file ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_listed || 0 == m_data->rowCount )
            return DATATABLE_STATUS_END;
        m_listed = true;
        file.Clear();
        file.path = m_name;
        file.fileSize = m_data->byteCount;
        file.modificationTime = m_data->modificationTime;
        return DATATABLE_STATUS_OK;
    }

    virtual TDataTableStatus OpenFile( const CDataTableScanFile& file, CITableReaderPtr& reader ) GUCEF_VIRTUAL_OVERRIDE
    {
        reader = CITableReaderPtr( GUCEF_NEW CMemoryDataTableReader( m_data, m_metaData ) );
        return DATATABLE_STATUS_OK;
    }

    private:

    TMemoryDataPtr m_data;
    CITableFileMetaDataPtr m_metaData;
    CString m_name;
    bool m_listed;
};

/*-------------------------------------------------------------------------*/

/** Rows [ first, first + count ) of a full batch as a batch sharing its buffers */
TDataTableStatus
SliceBatch( const CDataTableBatch& source, UInt32 first, UInt32 count, CDataTableBufferPool& pool, CDataTableBatchPtr& slice )
{
    TDataTableStatus status = pool.AcquireBatch( slice );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableBatch* writable = slice.GetWritable();
    const UInt32 columnCount = source.GetColumnCount();
    if ( !writable->SetColumnCount( columnCount ) )
        return DATATABLE_STATUS_OUT_OF_MEMORY;
    writable->SetSchema( source.GetSchema() );
    writable->SetRowCount( count );
    for ( UInt32 c=0; c<columnCount; ++c )
    {
        status = writable->GetMutableColumn( c ).SetSlice( source.GetColumn( c ), first, count, pool );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        writable->SetSchemaColumnIndex( c, source.GetSchemaColumnIndex( c ) );
    }
    return DATATABLE_STATUS_OK;
}

/** Bytes of the values of a full batch, roughly: fixed width values, bits, offsets and text */
UInt64
BatchByteCount( const CDataTableBatch& batch )
{
    UInt64 bytes = 0;
    for ( UInt32 c=0; c<batch.GetColumnCount(); ++c )
    {
        const CDataTableColumnBatch& column = batch.GetColumn( c );
        const UInt32 rows = column.GetRowCount();
        switch ( column.GetStorage() )
        {
            case DATATABLE_STORAGE_FIXED:       { bytes += (UInt64) rows * DataTableLogicalTypeByteWidth( column.GetLogicalType() ); break; }
            case DATATABLE_STORAGE_BITS:        { bytes += ( rows + 7 ) / 8; break; }
            case DATATABLE_STORAGE_VARIABLE:    { bytes += (UInt64) ( rows + 1 ) * 4 + ( rows > 0 ? column.GetOffsets()[ rows ] - column.GetOffsets()[ 0 ] : 0 ); break; }
            case DATATABLE_STORAGE_DICTIONARY:  { bytes += (UInt64) rows * 4; break; }
            default:                            { break; }
        }
        if ( column.HasNulls() )
            bytes += ( rows + 7 ) / 8;
    }
    return bytes;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMemoryDataTable::CMemoryDataTable( const CDataTableDefinition& definition ,
                                    CDataTableBufferPool* bufferPool       ,
                                    PulseGeneratorPtr pulseGenerator       )
    : CIDataTable( pulseGenerator )
    , m_definition( definition )
    , m_schema()
    , m_pool( GUCEF_NULL != bufferPool ? bufferPool : &CCoreGlobal::Instance()->GetDataTableBufferPool() )
    , m_valid( false )
    , m_readOnly( false )
    , m_writeLock()
    , m_dataLock()
    , m_data()
    , m_asyncLink()
    , m_errorLock()
    , m_lastErrors()
{GUCEF_TRACE;

    m_asyncLink = CDataTableAsyncLinkPtr( GUCEF_NEW CDataTableAsyncLink( this ) );
    m_schema = m_definition.CreateSchemaPtr();

    CString error;
    if ( !m_definition.name.IsNULLOrEmpty() && !CDataTableDefinition::IsValidTableName( m_definition.name ) )
        error = "\"" + m_definition.name + "\" is not a valid table name";
    else
    if ( !m_definition.schema.Validate( error ) )
        error = "The schema is not valid: " + error;
    else
    if ( !m_definition.partitioning.Validate( m_definition.schema, error ) )
        error = "The partitioning is not valid: " + error;
    else
    if ( !m_definition.storage.Validate( error ) )
        error = "The storage options are not valid: " + error;
    else
        m_valid = true;
    if ( !m_valid )
        Fail( DATATABLE_STATUS_INVALID_ARGUMENT, error );

    TDataPtr data( GUCEF_NEW CMemoryDataTableData() );
    data->schema = m_schema;
    data->modificationTime = CDateTime::NowUTCDateTime();
    m_data = data;
}

/*-------------------------------------------------------------------------*/

CMemoryDataTable::~CMemoryDataTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_asyncLink->Detach();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::Fail( TDataTableStatus status, const CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    m_lastErrors[ MT::GetCurrentThreadID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TLastErrorMap::iterator i = m_lastErrors.find( MT::GetCurrentThreadID() );
    if ( i != m_lastErrors.end() )
        m_lastErrors.erase( i );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CString
CMemoryDataTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TLastErrorMap::const_iterator i = m_lastErrors.find( MT::GetCurrentThreadID() );
    return i != m_lastErrors.end() ? (*i).second : CString::Empty;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::CheckWritable( void ) const
{GUCEF_TRACE;

    if ( !m_valid )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table definition is not valid" );
    if ( m_readOnly )
        return Fail( DATATABLE_STATUS_READ_ONLY, "Table \"" + m_definition.name + "\" is read only" );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CMemoryDataTable::TDataPtr
CMemoryDataTable::GetData( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_data;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryDataTable::IsValid( void ) const
{GUCEF_TRACE;

    return m_valid;
}

/*-------------------------------------------------------------------------*/

UInt64
CMemoryDataTable::GetRowCount( void ) const
{GUCEF_TRACE;

    return GetData()->rowCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CMemoryDataTable::GetBatchCount( void ) const
{GUCEF_TRACE;

    return (UInt32) GetData()->batches.size();
}

/*-------------------------------------------------------------------------*/

void
CMemoryDataTable::SetReadOnly( bool readOnly )
{GUCEF_TRACE;

    m_readOnly = readOnly;
}

/*-------------------------------------------------------------------------*/

const CString&
CMemoryDataTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CDataTableDefinition&
CMemoryDataTable::GetDefinition( void ) const
{GUCEF_TRACE;

    return m_definition;
}

/*-------------------------------------------------------------------------*/

CDataTableSchemaPtr
CMemoryDataTable::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryDataTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return m_readOnly;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::Scan( const CDataTableQuery& query, CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    if ( !m_valid )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table definition is not valid" );

    CDataTableScanSourcePtr source( GUCEF_NEW CMemoryDataTableScanSource( GetData(), m_definition.name ) );
    scan = CDataTableScanPtr( GUCEF_NEW CDataTableScan( source, m_schema, *m_pool, GetPulseGenerator() ) );
    const TDataTableStatus status = scan->Start( query );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, scan->GetLastErrorMessage() );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::ScanAsync( const CDataTableQuery& query, CDataTableScanPtr& scan, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    scan.Unlink();
    if ( !m_valid )
    {
        Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table definition is not valid" );
        return CFutureResult( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }
    CDataTableScanSourcePtr source( GUCEF_NEW CMemoryDataTableScanSource( GetData(), m_definition.name ) );
    scan = CDataTableScanPtr( GUCEF_NEW CDataTableScan( source, m_schema, *m_pool, GetPulseGenerator() ) );
    return scan->StartAsync( query, threadPool );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::StoreRows( const GUCEF::vector< CDataTableBatchPtr >& batches, bool canonical, UInt64 rowsReplaced, UInt64 rowsDeleted, CDataTableWriteResult& result )
{GUCEF_TRACE;

    // the new rows replace the old at once: scans that started before keep the old ones
    TDataPtr old = GetData();
    TDataPtr data( GUCEF_NEW CMemoryDataTableData() );
    data->schema = m_schema;
    data->canonical = canonical;
    data->modificationTime = CDateTime::NowUTCDateTime();
    data->rowGroups.resize( batches.size() );
    for ( UInt32 i=0; i<(UInt32) batches.size(); ++i )
    {
        if ( batches[ i ].IsNULL() || 0 == batches[ i ]->GetRowCount() )
            continue;
        const UInt32 index = (UInt32) data->batches.size();
        data->batches.push_back( batches[ i ] );

        // statistics of batches kept from before are taken over
        bool reused = false;
        for ( UInt32 o=0; o<(UInt32) old->batches.size() && !reused; ++o )
        {
            if ( old->batches[ o ] == batches[ i ] )
            {
                data->rowGroups[ index ] = old->rowGroups[ o ];
                reused = true;
            }
        }
        if ( !reused )
            CDataTableMerge::ComputeStats( *batches[ i ], data->rowGroups[ index ] );
        data->rowCount += batches[ i ]->GetRowCount();
        data->byteCount += BatchByteCount( *batches[ i ] );
    }
    data->rowGroups.resize( data->batches.size() );
    {
        MT::CScopeMutex lock( m_dataLock );
        m_data = data;
    }

    CDataTableWriteResult::SPartitionResult partition;
    partition.path = CString::Empty;
    partition.status = DATATABLE_STATUS_OK;
    partition.rowsWritten = data->rowCount;
    partition.rowsReplaced = rowsReplaced;
    partition.rowsDeleted = rowsDeleted;
    result.partitions.push_back( partition );

    CDataTablePartitionInfo info;
    info.path = CString::Empty;
    info.hasFileStats = true;
    info.fileSize = data->byteCount;
    info.modificationTime = data->modificationTime;
    info.rowCount = data->rowCount;
    info.rowGroupCount = (UInt32) data->batches.size();
    if ( 0 == data->rowCount )
    {
        // a partition left empty is deleted
        if ( old->rowCount > 0 )
            NotifyObserversFromThread( PartitionDeletedEvent, &info );
    }
    else
    {
        NotifyObserversFromThread( PartitionWrittenEvent, &info );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::Write( const CDataTableBatch& rows, TDataTableWriteMode mode, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableBatchPtr normalized;
    status = CDataTableMerge::Normalize( m_schema, rows, *m_pool, normalized, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    TDataPtr data = GetData();
    const UInt32 rowGroupRowCount = m_definition.storage.rowGroupRowCount;
    GUCEF::vector< CDataTableBatchPtr > batches;
    UInt64 rowsReplaced = 0;
    bool canonical = true;
    if ( DATATABLE_WRITE_INSERT == mode && m_schema->GetPrimaryKey().empty() && m_schema->GetSortKey().empty() )
    {
        // nothing to keep in order: the rows are added as batches of their own, sharing their buffers
        batches = data->batches;
        canonical = data->batches.empty() && data->canonical;
        const UInt32 rowCount = normalized->GetRowCount();
        for ( UInt32 first=0; first<rowCount; first+=rowGroupRowCount )
        {
            const UInt32 count = rowCount - first < rowGroupRowCount ? rowCount - first : rowGroupRowCount;
            if ( 0 == first && count == rowCount )
            {
                batches.push_back( normalized );
                break;
            }
            CDataTableBatchPtr slice;
            status = SliceBatch( *normalized, first, count, *m_pool, slice );
            if ( DATATABLE_STATUS_OK != status )
                return Fail( status, "Cutting the rows into row groups failed" );
            batches.push_back( slice );
        }
    }
    else
    {
        GUCEF::vector< CDataTableBatchPtr > incoming;
        incoming.push_back( normalized );
        status = CDataTableMerge::Merge( m_schema, data->batches, incoming, mode, rowGroupRowCount, *m_pool, batches, rowsReplaced, error );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status, error );
    }
    return StoreRows( batches, canonical, rowsReplaced, 0, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::DeleteRows( const CDataTableQuery& predicates, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !predicates.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TDataPtr data = GetData();
    GUCEF::vector< CDataTableBatchPtr > batches;
    UInt64 removed = 0;
    status = CDataTableMerge::RemoveMatchingRows( resolved, data->batches, *m_pool, batches, removed, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    if ( 0 == removed )
    {
        CDataTableWriteResult::SPartitionResult partition;
        partition.status = DATATABLE_STATUS_OK;
        partition.rowsWritten = data->rowCount;
        result.partitions.push_back( partition );
        return Succeed();
    }
    return StoreRows( batches, data->canonical, 0, removed, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::DeletePartitions( const CDataTableQuery& partitionFilter, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    // predicates may only name partition source columns
    const CDataTablePartitionSpec::TFieldVector& fields = m_definition.partitioning.GetFields();
    const CDataTableQuery::TPredicateVector& predicates = partitionFilter.GetPredicates();
    for ( UInt32 p=0; p<(UInt32) predicates.size(); ++p )
    {
        bool isSource = false;
        for ( UInt32 f=0; f<(UInt32) fields.size() && !isSource; ++f )
            isSource = fields[ f ].sourceColumn == predicates[ p ].columnName;
        if ( !isSource )
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "Column \"" + predicates[ p ].columnName + "\" is not a partition source column" );
    }
    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !partitionFilter.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    // the one partition has no key values, so it is selected unless the filter matches nothing
    TDataPtr data = GetData();
    if ( resolved.matchesNothing || !m_definition.partitioning.MayMatch( CValueList(), resolved ) || 0 == data->rowCount )
        return Succeed();
    const GUCEF::vector< CDataTableBatchPtr > none;
    return StoreRows( none, true, 0, data->rowCount, result );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::ListPartitions( const CDataTableQuery& partitionFilter, bool withFileStats, CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    if ( !m_valid )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The table definition is not valid" );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !partitionFilter.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TDataPtr data = GetData();
    bool mayMatch = false;
    const CDataTableSchema::TColumnIndexVector sameColumns;
    for ( UInt32 i=0; i<(UInt32) data->rowGroups.size() && !mayMatch; ++i )
        mayMatch = CDataTablePruner::RowGroupMayMatch( resolved, data->rowGroups[ i ], sameColumns );
    if ( !mayMatch )
        return Succeed();

    CDataTablePartitionInfo info;
    info.path = CString::Empty;
    info.hasFileStats = withFileStats;
    if ( withFileStats )
    {
        info.fileSize = data->byteCount;
        info.modificationTime = data->modificationTime;
        info.rowCount = data->rowCount;
        info.rowGroupCount = (UInt32) data->batches.size();
    }
    partitions.push_back( info );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CMemoryDataTable::Compact( const CDataTableQuery& partitionFilter, bool force, CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    TDataTableStatus status = CheckWritable();
    if ( DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CString error;
    CDataTableResolvedQuery resolved;
    resolved.schema = m_schema;
    if ( !partitionFilter.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );

    TDataPtr data = GetData();
    if ( ( data->canonical && !force ) || 0 == data->rowCount || resolved.matchesNothing )
        return Succeed();

    GUCEF::vector< CDataTableBatchPtr > batches;
    UInt64 removed = 0;
    status = CDataTableMerge::Canonicalize( m_schema, data->batches, m_definition.storage.rowGroupRowCount, *m_pool, batches, removed, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    return StoreRows( batches, true, removed, 0, result );
}

/*-------------------------------------------------------------------------*/

void
CMemoryDataTable::Refresh( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::QueueOperation( CDataTableAsyncOperationData& data, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    data.table = m_asyncLink;
    return CDataTableAsyncOperation::Queue( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::WriteAsync( const CDataTableBatchPtr& rows, TDataTableWriteMode mode, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_WRITE;
    data.rows = rows;
    data.writeMode = mode;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::DeleteRowsAsync( const CDataTableQuery& predicates, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_DELETE_ROWS;
    data.query = predicates;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::DeletePartitionsAsync( const CDataTableQuery& partitionFilter, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS;
    data.query = partitionFilter;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::ListPartitionsAsync( const CDataTableQuery& partitionFilter, bool withFileStats, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS;
    data.query = partitionFilter;
    data.flag = withFileStats;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CMemoryDataTable::CompactAsync( const CDataTableQuery& partitionFilter, bool force, ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CDataTableAsyncOperationData data;
    data.operation = CDataTableAsyncOperationData::OPERATION_COMPACT;
    data.query = partitionFilter;
    data.flag = force;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

const CString&
CMemoryDataTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CMemoryDataTable";
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
