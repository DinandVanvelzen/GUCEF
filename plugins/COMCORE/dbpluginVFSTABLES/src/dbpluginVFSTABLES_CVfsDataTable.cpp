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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#include "gucefCORE_CFileDataTable.h"
#define GUCEF_CORE_CFILEDATATABLE_H
#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */

#ifndef GUCEF_CORE_CCSVTABLECODEC_H
#include "gucefCORE_CCsvTableCodec.h"
#define GUCEF_CORE_CCSVTABLECODEC_H
#endif /* GUCEF_CORE_CCSVTABLECODEC_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#include "dbpluginVFSTABLES_CVfsTablesDatabaseClient.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H ? */

#include "dbpluginVFSTABLES_CVfsDataTable.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A partition field and the column of a full batch it takes its value from */
struct SFieldColumn
{
    CORE::CString name;
    UInt32 column;
    CORE::TDataTablePartitionTransform transform;
    CORE::TDataTableLogicalType type;
    CORE::TDataTableTimeUnit timeUnit;
};
typedef GUCEF::vector< SFieldColumn > TFieldColumnVector;

/** The rows of one partition: a range of the sorted row order, and the path and key values of the partition */
struct SPartitionRun
{
    CORE::CString relativePath;
    CORE::CValueList keyValues;
    UInt32 begin;
    UInt32 end;
};
typedef GUCEF::vector< SPartitionRun > TPartitionRunVector;

/** The day of a date or timestamp cell, floor division for timestamps before 1970 */
bool
DayOfCell( const CORE::CDataTableColumnBatch& column, UInt32 row, const SFieldColumn& field, Int32& days )
{
    if ( CORE::DATATABLE_TYPE_DATE == field.type )
    {
        const Int32* values = column.GetInt32Data();
        if ( GUCEF_NULL == values )
            return false;
        days = values[ row ];
        return true;
    }
    const Int64* values = column.GetInt64Data();
    if ( GUCEF_NULL == values )
        return false;
    const Int64 ticksPerDay = CORE::CDataTableValues::TicksPerDay( field.timeUnit );
    Int64 day = values[ row ] / ticksPerDay;
    if ( ( values[ row ] % ticksPerDay ) < 0 )
        --day;
    if ( day < GUCEF_INT32MIN || day > GUCEF_INT32MAX )
        return false;
    days = (Int32) day;
    return true;
}

/** The period of a day under a YEAR, MONTH or DAY transform, as one number */
Int64
PeriodOfDay( Int32 days, CORE::TDataTablePartitionTransform transform )
{
    if ( CORE::DATATABLE_PARTITION_TRANSFORM_DAY == transform )
        return days;
    Int32 year = 0;
    UInt32 month = 0;
    UInt32 day = 0;
    CORE::CDataTableValues::CivilFromDays( days, year, month, day );
    if ( CORE::DATATABLE_PARTITION_TRANSFORM_YEAR == transform )
        return year;
    return (Int64) year * 12 + ( month - 1 );
}

/** Splits a path into its segments */
void
SplitSegments( const CORE::CString& path, CORE::CString::StringVector& segments )
{
    segments = path.ParseElements( '/', true );
}

}

/*-------------------------------------------------------------------------*/

/** Orders rows by the values of the partition fields: periods as numbers, identity values as the column orders them */
struct CVfsDataTable::SRowOrder
{
    const CORE::CDataTableBatch* batch;
    const TFieldColumnVector* fields;
    const GUCEF::vector< Int64 >* periods;

    bool operator()( UInt32 left, UInt32 right ) const
    {
        const UInt32 fieldCount = (UInt32) fields->size();
        for ( UInt32 f=0; f<fieldCount; ++f )
        {
            const SFieldColumn& field = (*fields)[ f ];
            if ( CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY != field.transform )
            {
                const Int64 a = (*periods)[ (size_t) left * fieldCount + f ];
                const Int64 b = (*periods)[ (size_t) right * fieldCount + f ];
                if ( a != b )
                    return a < b;
                continue;
            }
            const CORE::CDataTableColumnBatch& column = batch->GetColumn( field.column );
            const Int32 order = CORE::CDataTableMerge::CompareCells( column, left, column, right );
            if ( 0 != order )
                return order < 0;
        }
        return false;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsDataTable::CVfsDataTable( const TVfsTablesStorePtr& store                 ,
                              const CORE::CDataTableDefinition& definition    ,
                              const TTableCodecPtr& codec                     ,
                              CORE::PulseGeneratorPtr pulseGenerator          )
    : CORE::CIDataTable( pulseGenerator )
    , m_store( store )
    , m_definition( definition )
    , m_schema( definition.CreateSchemaPtr() )
    , m_codec( codec )
    , m_codecOptions( store->GetSettings().codecOptions )
    , m_unpartitionedFile()
    , m_layoutText( CORE::CFileDataTable::MakeLayoutText( definition ) )
    , m_writeLock( m_store->GetTableWriteLock( definition.name ) )
    , m_asyncLink()
    , m_errorLock()
    , m_lastErrors()
{GUCEF_TRACE;

    m_asyncLink = CORE::CDataTableAsyncLinkPtr( GUCEF_NEW CORE::CDataTableAsyncLink( this ) );

    // an unpartitioned table is one file named after the table, with the extension of its codec
    CORE::CString extension = m_definition.codecType;
    if ( !m_codec.IsNULL() )
    {
        CORE::CString::StringSet extensions;
        m_codec->GetFileExtensions( extensions );
        if ( !extensions.empty() && extensions.find( m_definition.codecType ) == extensions.end() )
            extension = *extensions.begin();

        // a codec whose files do not state their types reads them with the types of the table
        if ( 0 == ( m_codec->GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_EMBEDDED_SCHEMA ) && "csv" == m_codec->GetTypeName() )
            CORE::CCsvTableCodec::SetSchemaOption( m_codecOptions, m_definition.schema );
    }
    m_unpartitionedFile = m_definition.name + "." + extension;

    CVfsTablesDatabaseClient::OnObjectCreated( "table \"" + m_definition.name + "\" of \"" + m_store->GetRoot() + "\"" );
}

/*-------------------------------------------------------------------------*/

CVfsDataTable::~CVfsDataTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_asyncLink->Detach();
    CVfsTablesDatabaseClient::OnObjectDestroyed( "table \"" + m_definition.name + "\" of \"" + m_store->GetRoot() + "\"" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    m_lastErrors[ MT::GetCurrentTaskID() ] = message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    m_lastErrors.erase( MT::GetCurrentTaskID() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsDataTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_errorLock );
    TLastErrorMap::const_iterator i = m_lastErrors.find( MT::GetCurrentTaskID() );
    return i != m_lastErrors.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsDataTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableDefinition&
CVfsDataTable::GetDefinition( void ) const
{GUCEF_TRACE;

    return m_definition;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableSchemaPtr
CVfsDataTable::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CVfsDataTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return m_store->IsReadOnly();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsDataTable::GetUnpartitionedFile( void ) const
{GUCEF_TRACE;

    return m_unpartitionedFile;
}

/*-------------------------------------------------------------------------*/

void
CVfsDataTable::GetStorageStats( CVfsTablesStats& stats ) const
{GUCEF_TRACE;

    m_store->GetStats( stats );
}

/*-------------------------------------------------------------------------*/

TVfsTablesStorePtr
CVfsDataTable::GetStore( void ) const
{GUCEF_TRACE;

    return m_store;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::CheckWritable( void ) const
{GUCEF_TRACE;

    if ( m_store->IsReadOnly() )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database \"" + m_store->GetRoot() + "\" is open read only" );
    if ( m_codec.IsNULL() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The table has no codec" );
    if ( 0 == ( m_codec->GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_WRITE ) )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Codec \"" + m_codec->GetTypeName() + "\" does not write files" );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Resolve( const CORE::CDataTableQuery& query, CORE::CDataTableResolvedQuery& resolved ) const
{GUCEF_TRACE;

    CORE::CString error;
    resolved.schema = m_schema;
    if ( !query.Resolve( *m_schema, resolved, error ) )
        return Fail( resolved.status, error );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::CheckPartitionFilter( const CORE::CDataTableQuery& partitionFilter ) const
{GUCEF_TRACE;

    const CORE::CDataTablePartitionSpec::TFieldVector& fields = m_definition.partitioning.GetFields();
    const CORE::CDataTableQuery::TPredicateVector& predicates = partitionFilter.GetPredicates();
    for ( UInt32 p=0; p<(UInt32) predicates.size(); ++p )
    {
        bool isSource = false;
        for ( UInt32 f=0; f<(UInt32) fields.size() && !isSource; ++f )
            isSource = fields[ f ].sourceColumn == predicates[ p ].columnName;
        if ( !isSource )
            return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "Column \"" + predicates[ p ].columnName + "\" is not a partition source column" );
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableScanSourcePtr
CVfsDataTable::MakeSource( const CORE::CDataTableResolvedQuery& resolved ) const
{GUCEF_TRACE;

    return CORE::CDataTableScanSourcePtr( GUCEF_NEW CVfsTableScanSource( m_store, m_definition, m_schema, m_unpartitionedFile,
                                                                         m_codec, m_codecOptions, resolved ) );
}

/*-------------------------------------------------------------------------*/

void
CVfsDataTable::FindPartitions( const CORE::CDataTableResolvedQuery& resolved, CVfsTableScanSource::TPartitionFileVector& partitions ) const
{GUCEF_TRACE;

    partitions.clear();
    CVfsTableScanSource source( m_store, m_definition, m_schema, m_unpartitionedFile, m_codec, m_codecOptions, resolved );
    CVfsTableScanSource::SPartitionFile partition;
    while ( CORE::DATATABLE_STATUS_OK == source.NextPartition( partition ) )
        partitions.push_back( partition );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan )
{GUCEF_TRACE;

    scan.Unlink();
    CORE::CDataTableResolvedQuery resolved;
    CORE::TDataTableStatus status = Resolve( query, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    scan = CORE::CDataTableScanPtr( GUCEF_NEW CORE::CDataTableScan( MakeSource( resolved ), m_schema, m_store->GetBufferPool(), GetPulseGenerator() ) );
    status = scan->Start( query );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, scan->GetLastErrorMessage() );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    // a query that does not resolve reaches the scan, whose future then fails with TASKDATA_INVALID
    CORE::CDataTableResolvedQuery resolved;
    if ( CORE::DATATABLE_STATUS_OK != Resolve( query, resolved ) )
    {
        resolved = CORE::CDataTableResolvedQuery();
        resolved.matchesNothing = true;
    }
    scan = CORE::CDataTableScanPtr( GUCEF_NEW CORE::CDataTableScan( MakeSource( resolved ), m_schema, m_store->GetBufferPool(), GetPulseGenerator() ) );
    if ( threadPool.IsNULL() )
        threadPool = m_store->GetThreadPool();
    return scan->StartAsync( query, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::RouteRows( const CORE::CDataTableBatchPtr& normalized, TPartitionRowsVector& partitions, CORE::CString& error ) const
{GUCEF_TRACE;

    partitions.clear();
    const CORE::CDataTableBatch& batch = *normalized;
    const UInt32 rowCount = batch.GetRowCount();
    if ( !m_definition.partitioning.IsPartitioned() )
    {
        SPartitionRows all;
        all.relativePath = m_unpartitionedFile;
        all.rows = normalized;
        partitions.push_back( all );
        return CORE::DATATABLE_STATUS_OK;
    }

    // the partition fields and their columns (a full batch: projected column i is schema column i)
    const CORE::CDataTablePartitionSpec::TFieldVector& specFields = m_definition.partitioning.GetFields();
    TFieldColumnVector fields;
    for ( UInt32 f=0; f<(UInt32) specFields.size(); ++f )
    {
        const Int32 column = m_schema->FindColumn( specFields[ f ].sourceColumn );
        if ( column < 0 )
        {
            error = "Partition source column \"" + specFields[ f ].sourceColumn + "\" is not in the table";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        SFieldColumn field;
        field.name = specFields[ f ].name;
        field.column = (UInt32) column;
        field.transform = specFields[ f ].transform;
        field.type = m_schema->GetColumn( (UInt32) column ).type;
        field.timeUnit = m_schema->GetColumn( (UInt32) column ).timeUnit;
        fields.push_back( field );
    }

    // the periods of the YEAR, MONTH and DAY fields per row
    const UInt32 fieldCount = (UInt32) fields.size();
    GUCEF::vector< Int64 > periods( (size_t) rowCount * fieldCount, 0 );
    for ( UInt32 f=0; f<fieldCount; ++f )
    {
        if ( CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY == fields[ f ].transform )
            continue;
        const CORE::CDataTableColumnBatch& column = batch.GetColumn( fields[ f ].column );
        for ( UInt32 row=0; row<rowCount; ++row )
        {
            Int32 days = 0;
            if ( !DayOfCell( column, row, fields[ f ], days ) )
            {
                error = "Row " + CORE::ToString( row ) + ": column \"" + m_schema->GetColumn( fields[ f ].column ).name + "\" has no date of a partition";
                return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            }
            periods[ (size_t) row * fieldCount + f ] = PeriodOfDay( days, fields[ f ].transform );
        }
    }

    // rows grouped by partition, in their order within each partition
    GUCEF::vector< UInt32 > order( rowCount );
    for ( UInt32 row=0; row<rowCount; ++row )
        order[ row ] = row;
    SRowOrder less;
    less.batch = &batch;
    less.fields = &fields;
    less.periods = &periods;
    std::stable_sort( order.begin(), order.end(), less );

    TPartitionRunVector runs;
    UInt32 begin = 0;
    while ( begin < rowCount )
    {
        UInt32 end = begin + 1;
        while ( end < rowCount && !less( order[ begin ], order[ end ] ) )
            ++end;

        // the key values of the partition, and its path, which must read back as exactly those values
        const UInt32 row = order[ begin ];
        SPartitionRun run;
        run.begin = begin;
        run.end = end;
        run.keyValues.SetAllowMultipleValues( false );
        for ( UInt32 f=0; f<fieldCount; ++f )
        {
            const CORE::CDataTableColumnBatch& column = batch.GetColumn( fields[ f ].column );
            if ( CORE::DATATABLE_TYPE_UTF8 == fields[ f ].type && CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY == fields[ f ].transform )
            {
                UInt32 length = 0;
                const char* bytes = column.GetVariableValue( row, length );
                if ( GUCEF_NULL == bytes || 0 == length || GUCEF_NULL != memchr( bytes, 0, length ) )
                {
                    error = "Row " + CORE::ToString( row ) + ": the value of partition field \"" + fields[ f ].name + "\" is empty or holds a NUL byte";
                    return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
                }
            }
            CORE::CVariant value;
            column.GetValue( row, value, false );
            run.keyValues.Set( fields[ f ].name, value );
        }
        if ( !m_definition.partitioning.BuildPath( run.keyValues, run.relativePath ) )
        {
            error = "Row " + CORE::ToString( row ) + ": the values of the partition fields cannot form a path";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        CORE::CString::StringVector segments;
        SplitSegments( run.relativePath, segments );
        if ( !segments.empty() && CVfsTablesStore::TablesDirName == segments[ 0 ] )
        {
            error = "Row " + CORE::ToString( row ) + ": the partition path \"" + run.relativePath + "\" would be in the directory of the table definitions";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        CORE::CValueList parsed;
        bool readsBack = segments.size() == m_definition.partitioning.GetSegmentCount();
        for ( UInt32 s=0; s<(UInt32) segments.size() && readsBack; ++s )
            readsBack = m_definition.partitioning.ParseSegment( s, segments[ s ], parsed );
        for ( UInt32 f=0; f<fieldCount && readsBack; ++f )
        {
            if ( CORE::DATATABLE_PARTITION_TRANSFORM_IDENTITY != fields[ f ].transform )
                continue;
            UInt32 length = 0;
            const CORE::CVariant parsedValue = parsed.GetValueAlways( fields[ f ].name );
            if ( CORE::DATATABLE_TYPE_UTF8 == fields[ f ].type )
            {
                const char* bytes = batch.GetColumn( fields[ f ].column ).GetVariableValue( row, length );
                const CORE::CUtf8String text = parsedValue.AsUtf8String();
                readsBack = text.ByteSize() == length + 1 && 0 == memcmp( text.C_String(), bytes, length );
            }
        }
        if ( !readsBack )
        {
            error = "Row " + CORE::ToString( row ) + ": the values of the partition fields do not read back from the path \"" + run.relativePath + "\"";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        runs.push_back( run );
        begin = end;
    }

    if ( 1 == runs.size() )
    {
        // one partition: the rows as they are
        SPartitionRows only;
        only.relativePath = runs[ 0 ].relativePath;
        only.keyValues = runs[ 0 ].keyValues;
        only.rows = normalized;
        partitions.push_back( only );
        return CORE::DATATABLE_STATUS_OK;
    }

    CORE::CDataTableBufferPool& pool = m_store->GetBufferPool();
    for ( UInt32 r=0; r<(UInt32) runs.size(); ++r )
    {
        // a view of the rows of the partition, a selection over the shared columns, made into a full batch
        const UInt32 count = runs[ r ].end - runs[ r ].begin;
        CORE::CDataTableBufferPtr selection;
        CORE::TDataTableStatus status = pool.Acquire( count * (UInt32) sizeof( UInt32 ), selection );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            error = "No memory for the rows of a partition";
            return status;
        }
        memcpy( selection->GetData(), &order[ runs[ r ].begin ], count * sizeof( UInt32 ) );
        selection->SetDataSize( count * (UInt32) sizeof( UInt32 ) );

        CORE::CDataTableBatchPtr view;
        status = pool.AcquireBatch( view );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            error = "No memory for the rows of a partition";
            return status;
        }
        CORE::CDataTableBatch* writable = view.GetWritable();
        writable->SetSchema( batch.GetSchema() );
        if ( !writable->SetColumnCount( batch.GetColumnCount() ) )
        {
            error = "No memory for the rows of a partition";
            return CORE::DATATABLE_STATUS_OUT_OF_MEMORY;
        }
        writable->SetRowCount( rowCount );
        for ( UInt32 c=0; c<batch.GetColumnCount(); ++c )
        {
            status = writable->GetMutableColumn( c ).SetSlice( batch.GetColumn( c ), 0, rowCount, pool );
            if ( CORE::DATATABLE_STATUS_OK != status )
            {
                error = "The rows of a partition could not be taken from the batch";
                return status;
            }
            writable->SetSchemaColumnIndex( c, batch.GetSchemaColumnIndex( c ) );
        }
        writable->SetSelection( static_cast< const UInt32* >( selection->GetData() ), count, selection );

        SPartitionRows rows;
        rows.relativePath = runs[ r ].relativePath;
        rows.keyValues = runs[ r ].keyValues;
        status = CORE::CDataTableMerge::Normalize( m_schema, *view, pool, rows.rows, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        partitions.push_back( rows );
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::CheckRowsOfPartition( const TBatchVector& batches, const CORE::CString& relativePath, CORE::CString& error ) const
{GUCEF_TRACE;

    for ( UInt32 b=0; b<(UInt32) batches.size(); ++b )
    {
        if ( 0 == batches[ b ]->GetRowCount() )
            continue;
        TPartitionRowsVector partitions;
        const CORE::TDataTableStatus status = RouteRows( batches[ b ], partitions, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
        {
            if ( partitions[ p ].relativePath != relativePath )
            {
                error = "Partition \"" + relativePath + "\" holds rows of partition \"" + partitions[ p ].relativePath + "\"";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
        }
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::ReadPartition( const CVfsTableScanSource::SPartitionFile& partition, TBatchVector& batches, CORE::CString& error ) const
{GUCEF_TRACE;

    batches.clear();
    CVfsTableScanSource::TPartitionFileVector files;
    files.push_back( partition );
    CORE::CDataTableScanSourcePtr source( GUCEF_NEW CVfsTableScanSource( m_store, m_definition, m_schema, m_codec, m_codecOptions, files ) );
    CORE::CDataTableScan scan( source, m_schema, m_store->GetBufferPool(), GetPulseGenerator() );
    CORE::TDataTableStatus status = scan.Start( CORE::CDataTableQuery() );
    while ( CORE::DATATABLE_STATUS_OK == status )
    {
        CORE::CDataTableBatchPtr batch;
        status = scan.NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_OK == status )
            batches.push_back( batch );
    }
    if ( CORE::DATATABLE_STATUS_END != status )
    {
        error = "Reading the rows of \"" + partition.relativePath + "\" failed: " + scan.GetLastErrorMessage();
        batches.clear();
        return status;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CVfsDataTable::AddFailedPartition( const CORE::CString& relativePath, CORE::TDataTableStatus status, const CORE::CString& message,
                                   CORE::CDataTableWriteResult& result ) const
{GUCEF_TRACE;

    CORE::CDataTableWriteResult::SPartitionResult partition;
    partition.path = relativePath;
    partition.status = status;
    partition.message = message;
    result.partitions.push_back( partition );
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VfsTables: table \"" << m_definition.name << "\": " << message );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::EndChange( CORE::CDataTableWriteResult& result ) const
{GUCEF_TRACE;

    const CORE::TDataTableStatus status = result.GetOverallStatus();
    if ( CORE::DATATABLE_STATUS_OK == status )
        return Succeed();
    for ( UInt32 i=0; i<(UInt32) result.partitions.size(); ++i )
    {
        if ( CORE::DATATABLE_STATUS_OK != result.partitions[ i ].status )
            return Fail( status, result.partitions[ i ].message );
    }
    return Fail( status, "The change failed" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::WritePartitionFile( const CVfsTableScanSource::SPartitionFile& partition, const TBatchVector& batches,
                                   UInt64 rowsReplaced, UInt64 rowsDeleted, bool existed, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    UInt64 rowCount = 0;
    for ( UInt32 i=0; i<(UInt32) batches.size(); ++i )
        rowCount += batches[ i ]->GetRowCount();

    const CORE::CString path = m_store->MakePath( partition.relativePath );
    CORE::CDataTableWriteResult::SPartitionResult outcome;
    outcome.path = partition.relativePath;
    outcome.rowsWritten = rowCount;
    outcome.rowsReplaced = rowsReplaced;
    outcome.rowsDeleted = rowsDeleted;

    CORE::CDataTablePartitionInfo info;
    info.path = partition.relativePath;
    info.keyValues = partition.keyValues;
    info.hasFileStats = true;
    info.rowCount = rowCount;
    info.rowGroupCount = (UInt32) batches.size();

    CORE::CString error;
    if ( 0 == rowCount )
    {
        // a partition without rows has no file
        if ( existed )
        {
            const CORE::TDataTableStatus status = m_store->DeleteFile( path, error );
            if ( CORE::DATATABLE_STATUS_OK != status )
            {
                AddFailedPartition( partition.relativePath, status, error, result );
                return status;
            }
        }
        outcome.status = CORE::DATATABLE_STATUS_OK;
        result.partitions.push_back( outcome );
        if ( existed )
        {
            info.fileSize = 0;
            NotifyObserversFromThread( PartitionDeletedEvent, &info );
        }
        return CORE::DATATABLE_STATUS_OK;
    }

    // the new file is written next to the partition and moved over it: readers see the old file or the new one
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString temporaryPath = m_store->MakeTemporaryPath( path );
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CORE::CDataTableFileSummary summary;
    {
        VFS::TBasicVfsResourcePtr resource = vfs.GetFile( temporaryPath, "wb", true );
        CORE::IOAccessPtr output;
        if ( !resource.IsNULL() )
            output = resource->GetAccess();
        if ( output.IsNULL() )
        {
            error = "File \"" + temporaryPath + "\" cannot be created";
            AddFailedPartition( partition.relativePath, CORE::DATATABLE_STATUS_IO_ERROR, error, result );
            return CORE::DATATABLE_STATUS_IO_ERROR;
        }

        CORE::CDataTableWriterOptions options( m_definition.storage );
        options.keyValueMetaData.Set( CORE::CFileDataTable::LayoutMetaDataKey, m_layoutText );
        CORE::CITableWriterPtr writer;
        status = m_codec->CreateWriter( output, m_schema, options, writer, error );
        for ( UInt32 i=0; i<(UInt32) batches.size() && CORE::DATATABLE_STATUS_OK == status; ++i )
        {
            status = writer->WriteBatch( *batches[ i ] );
            if ( CORE::DATATABLE_STATUS_OK == status )
                status = writer->FlushRowGroup();
        }
        if ( CORE::DATATABLE_STATUS_OK == status )
            status = writer->Finish( summary );
        if ( CORE::DATATABLE_STATUS_OK != status && !writer.IsNULL() )
        {
            error = writer->GetLastErrorMessage();
            writer->Abort();
        }
        writer.Unlink();
        if ( !m_store->FinishTemporaryFile( output ) && CORE::DATATABLE_STATUS_OK == status )
        {
            status = CORE::DATATABLE_STATUS_IO_ERROR;
            error = "File \"" + temporaryPath + "\" could not be made durable";
        }
        resource.Unlink();
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        m_store->RemoveTemporaryFile( temporaryPath );
        AddFailedPartition( partition.relativePath, status, "Writing \"" + partition.relativePath + "\" failed: " + error, result );
        return status;
    }

    status = m_store->ReplaceFile( temporaryPath, path, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        m_store->RemoveTemporaryFile( temporaryPath );
        AddFailedPartition( partition.relativePath, status, error, result );
        return status;
    }

    outcome.status = CORE::DATATABLE_STATUS_OK;
    result.partitions.push_back( outcome );
    info.fileSize = summary.byteCount;
    NotifyObserversFromThread( PartitionWrittenEvent, &info );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CORE::CDataTableBufferPool& pool = m_store->GetBufferPool();
    CORE::CString error;
    CORE::CDataTableBatchPtr normalized;
    status = CORE::CDataTableMerge::Normalize( m_schema, rows, pool, normalized, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    if ( CORE::DATATABLE_WRITE_UPSERT == mode && m_schema->GetPrimaryKey().empty() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "UPSERT needs a primary key" );
    if ( 0 == normalized->GetRowCount() )
        return Succeed();

    TPartitionRowsVector partitions;
    status = RouteRows( normalized, partitions, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
    {
        CVfsTableScanSource::SPartitionFile partition;
        partition.relativePath = partitions[ p ].relativePath;
        partition.keyValues = partitions[ p ].keyValues;
        const bool existed = m_store->FileExists( m_store->MakePath( partition.relativePath ) );

        TBatchVector existing;
        if ( CORE::DATATABLE_WRITE_REPLACE_PARTITIONS != mode && existed )
        {
            status = ReadPartition( partition, existing, error );
            if ( CORE::DATATABLE_STATUS_OK != status )
            {
                AddFailedPartition( partition.relativePath, status, error, result );
                continue;
            }
        }

        TBatchVector incoming;
        incoming.push_back( partitions[ p ].rows );
        TBatchVector merged;
        UInt64 rowsReplaced = 0;
        status = CORE::CDataTableMerge::Merge( m_schema, existing, incoming, mode, m_definition.storage.rowGroupRowCount, pool, merged, rowsReplaced, error );
        if ( CORE::DATATABLE_STATUS_OK == status )
            status = CheckRowsOfPartition( merged, partition.relativePath, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partition.relativePath, status, "Partition \"" + partition.relativePath + "\": " + error, result );
            continue;
        }
        WritePartitionFile( partition, merged, rowsReplaced, 0, existed, result );
    }
    return EndChange( result );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CORE::CDataTableResolvedQuery resolved;
    status = Resolve( predicates, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CVfsTableScanSource::TPartitionFileVector partitions;
    FindPartitions( resolved, partitions );
    CORE::CString error;
    for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
    {
        // the rows of a partition whose key values may match
        TBatchVector existing;
        status = ReadPartition( partitions[ p ], existing, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partitions[ p ].relativePath, status, error, result );
            continue;
        }
        TBatchVector kept;
        UInt64 removed = 0;
        status = CORE::CDataTableMerge::RemoveMatchingRows( resolved, existing, m_store->GetBufferPool(), kept, removed, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partitions[ p ].relativePath, status, error, result );
            continue;
        }
        if ( 0 == removed )
            continue;
        WritePartitionFile( partitions[ p ], kept, 0, removed, true, result );
    }
    return EndChange( result );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = CheckPartitionFilter( partitionFilter );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CORE::CDataTableResolvedQuery resolved;
    status = Resolve( partitionFilter, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CVfsTableScanSource::TPartitionFileVector partitions;
    FindPartitions( resolved, partitions );
    for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
    {
        const CORE::CString path = m_store->MakePath( partitions[ p ].relativePath );
        CORE::CDataTablePartitionInfo info;
        info.path = partitions[ p ].relativePath;
        info.keyValues = partitions[ p ].keyValues;

        // the rows that go, from the metadata (usually in the footer cache)
        UInt64 rowsDeleted = 0;
        {
            TDataTableSharedFilePtr shared;
            bool fromCache = false;
            CORE::CString error;
            if ( CORE::DATATABLE_STATUS_OK == m_store->OpenSharedFile( path, m_codec, m_codecOptions, shared, fromCache, error ) )
            {
                const UInt64 rows = shared->metaData->GetRowCount();
                rowsDeleted = GUCEF_UINT64MAX == rows ? 0 : rows;
                info.hasFileStats = true;
                info.fileSize = shared->size;
                info.rowCount = rows;
                info.rowGroupCount = shared->metaData->GetRowGroupCount();
            }
        }

        CORE::CString error;
        status = m_store->DeleteFile( path, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partitions[ p ].relativePath, status, error, result );
            continue;
        }
        CORE::CDataTableWriteResult::SPartitionResult outcome;
        outcome.path = partitions[ p ].relativePath;
        outcome.status = CORE::DATATABLE_STATUS_OK;
        outcome.rowsDeleted = rowsDeleted;
        result.partitions.push_back( outcome );
        NotifyObserversFromThread( PartitionDeletedEvent, &info );
    }
    return EndChange( result );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    partitions.clear();
    CORE::CDataTableResolvedQuery resolved;
    CORE::TDataTableStatus status = Resolve( partitionFilter, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // predicates on columns that are not partition source columns are tested against the row group statistics
    bool otherColumns = false;
    const CORE::CDataTablePartitionSpec::TFieldVector& fields = m_definition.partitioning.GetFields();
    for ( UInt32 p=0; p<(UInt32) resolved.predicates.size() && !otherColumns; ++p )
    {
        const CORE::CString& column = m_schema->GetColumn( resolved.predicates[ p ].columnIndex ).name;
        bool isSource = false;
        for ( UInt32 f=0; f<(UInt32) fields.size() && !isSource; ++f )
            isSource = fields[ f ].sourceColumn == column;
        otherColumns = !isSource;
    }

    CVfsTableScanSource::TPartitionFileVector files;
    FindPartitions( resolved, files );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 i=0; i<(UInt32) files.size(); ++i )
    {
        CORE::CDataTablePartitionInfo info;
        info.path = files[ i ].relativePath;
        info.keyValues = files[ i ].keyValues;
        if ( withFileStats || otherColumns )
        {
            const CORE::CString path = m_store->MakePath( files[ i ].relativePath );
            TDataTableSharedFilePtr shared;
            bool fromCache = false;
            CORE::CString error;
            status = m_store->OpenSharedFile( path, m_codec, m_codecOptions, shared, fromCache, error );
            if ( CORE::DATATABLE_STATUS_NOT_FOUND == status )
                continue;   // deleted since it was listed
            if ( CORE::DATATABLE_STATUS_OK != status )
                return Fail( status, error );

            const CORE::CITableFileMetaData& metaData = *shared->metaData;
            if ( otherColumns )
            {
                const CORE::CDataTableSchema& fileSchema = *metaData.GetSchema();
                CORE::CDataTableSchema::TColumnIndexVector fileColumnOfTableColumn( m_schema->GetColumnCount(), GUCEF_UINT32MAX );
                for ( UInt32 t=0; t<m_schema->GetColumnCount(); ++t )
                {
                    const Int32 fileColumn = fileSchema.FindColumn( m_schema->GetColumn( t ).name );
                    if ( fileColumn >= 0 )
                        fileColumnOfTableColumn[ t ] = (UInt32) fileColumn;
                }
                bool mayMatch = false;
                CORE::CDataTableRowGroupInfo rowGroup;
                for ( UInt32 rg=0; rg<metaData.GetRowGroupCount() && !mayMatch; ++rg )
                    mayMatch = !metaData.GetRowGroupInfo( rg, rowGroup ) || CORE::CDataTablePruner::RowGroupMayMatch( resolved, rowGroup, fileColumnOfTableColumn, &fileSchema );
                if ( !mayMatch )
                    continue;
            }
            if ( withFileStats )
            {
                info.hasFileStats = true;
                info.fileSize = shared->size;
                info.modificationTime = vfs.GetFileModificationTime( path );
                info.rowCount = metaData.GetRowCount();
                info.rowGroupCount = metaData.GetRowGroupCount();
            }
        }
        partitions.push_back( info );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsDataTable::Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    result.Clear();
    CORE::TDataTableStatus status = CheckWritable();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    MT::CScopeMutex writeLock( m_writeLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CORE::CDataTableResolvedQuery resolved;
    status = Resolve( partitionFilter, resolved );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CVfsTableScanSource::TPartitionFileVector partitions;
    FindPartitions( resolved, partitions );
    CORE::CString error;
    for ( UInt32 p=0; p<(UInt32) partitions.size(); ++p )
    {
        if ( !force )
        {
            // a file in the layout of the definition is canonical already
            TDataTableSharedFilePtr shared;
            bool fromCache = false;
            status = m_store->OpenSharedFile( m_store->MakePath( partitions[ p ].relativePath ), m_codec, m_codecOptions, shared, fromCache, error );
            if ( CORE::DATATABLE_STATUS_NOT_FOUND == status )
                continue;
            if ( CORE::DATATABLE_STATUS_OK != status )
            {
                AddFailedPartition( partitions[ p ].relativePath, status, error, result );
                continue;
            }
            CORE::CValueList keyValues;
            shared->metaData->GetKeyValueMetaData( keyValues );
            if ( keyValues.GetValueAlways( CORE::CFileDataTable::LayoutMetaDataKey ).AsString() == m_layoutText )
                continue;
        }

        TBatchVector existing;
        status = ReadPartition( partitions[ p ], existing, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partitions[ p ].relativePath, status, error, result );
            continue;
        }
        TBatchVector canonical;
        UInt64 removed = 0;
        status = CORE::CDataTableMerge::Canonicalize( m_schema, existing, m_definition.storage.rowGroupRowCount, m_store->GetBufferPool(), canonical, removed, error );
        if ( CORE::DATATABLE_STATUS_OK == status )
            status = CheckRowsOfPartition( canonical, partitions[ p ].relativePath, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            AddFailedPartition( partitions[ p ].relativePath, status, "Partition \"" + partitions[ p ].relativePath + "\": " + error, result );
            continue;
        }
        WritePartitionFile( partitions[ p ], canonical, removed, 0, true, result );
    }
    return EndChange( result );
}

/*-------------------------------------------------------------------------*/

void
CVfsDataTable::Refresh( void )
{GUCEF_TRACE;

    m_store->ClearCaches();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::QueueOperation( CORE::CDataTableAsyncOperationData& data, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    data.table = m_asyncLink;
    if ( threadPool.IsNULL() )
        threadPool = m_store->GetThreadPool();
    return CORE::CDataTableAsyncOperation::Queue( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_WRITE;
    data.rows = rows;
    data.writeMode = mode;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_ROWS;
    data.query = predicates;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_DELETE_PARTITIONS;
    data.query = partitionFilter;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_LIST_PARTITIONS;
    data.query = partitionFilter;
    data.flag = withFileStats;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CVfsDataTable::CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    CORE::CDataTableAsyncOperationData data;
    data.operation = CORE::CDataTableAsyncOperationData::OPERATION_COMPACT;
    data.query = partitionFilter;
    data.flag = force;
    return QueueOperation( data, threadPool );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsDataTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DBPLUGIN::VFSTABLES::CVfsDataTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
