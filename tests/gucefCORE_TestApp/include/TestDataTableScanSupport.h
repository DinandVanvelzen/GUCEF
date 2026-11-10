/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCANSUPPORT_H
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCANSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <limits>

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#include "gucefCORE_CDataTablePruner.h"
#define GUCEF_CORE_CDATATABLEPRUNER_H
#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */

#include "TestDataTableSupport.h"
#include "TestDataTableTestCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

/** A seeded generator (xorshift64*) so property tests repeat exactly */
class CTestRandom
{
    public:

    explicit CTestRandom( GUCEF::UInt64 seed ) : m_state( 0 == seed ? 0x9E3779B97F4A7C15ULL : seed ) {}

    GUCEF::UInt64 Next( void )
    {
        m_state ^= m_state >> 12;
        m_state ^= m_state << 25;
        m_state ^= m_state >> 27;
        return m_state * 2685821657736338717ULL;
    }

    /** In [ 0, bound ) */
    GUCEF::UInt32 Below( GUCEF::UInt32 bound ) { return 0 == bound ? 0 : (GUCEF::UInt32) ( Next() % bound ); }

    /** In [ low, high ] */
    GUCEF::Int64 Between( GUCEF::Int64 low, GUCEF::Int64 high ) { return low + (GUCEF::Int64) ( Next() % (GUCEF::UInt64) ( high - low + 1 ) ); }

    bool Chance( GUCEF::UInt32 percent ) { return Below( 100 ) < percent; }

    private:

    GUCEF::UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

/** A random value of the column type in a small domain, so equal values and ranges hit; nulls at nullPercent */
inline GUCEF::CORE::CVariant
RandomCellValue( CTestRandom& random, const GUCEF::CORE::CDataTableColumnDef& column, GUCEF::UInt32 nullPercent )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    if ( column.nullable && random.Chance( nullPercent ) )
        return CVariant();
    switch ( column.type )
    {
        case DATATABLE_TYPE_BOOL:       { return CVariant( random.Chance( 50 ) ); }
        case DATATABLE_TYPE_INT8:       { return CVariant( (Int64) random.Between( -100, 100 ) ); }
        case DATATABLE_TYPE_INT16:      { return CVariant( (Int64) random.Between( -1000, 1000 ) ); }
        case DATATABLE_TYPE_INT32:      { return CVariant( (Int64) random.Between( -50, 50 ) ); }
        case DATATABLE_TYPE_INT64:      { return CVariant( (Int64) random.Between( -100000, 100000 ) * 1000003 ); }
        case DATATABLE_TYPE_UINT8:      { return CVariant( (UInt64) random.Below( 200 ) ); }
        case DATATABLE_TYPE_UINT16:     { return CVariant( (UInt64) random.Below( 60000 ) ); }
        case DATATABLE_TYPE_UINT32:     { return CVariant( (UInt64) random.Below( 100 ) ); }
        case DATATABLE_TYPE_UINT64:     { return CVariant( (UInt64) ( random.Below( 1000 ) * (UInt64) 18446744073709ULL ) ); }
        case DATATABLE_TYPE_FLOAT32:
        {
            if ( random.Chance( 3 ) )
                return CVariant( std::numeric_limits< Float32 >::quiet_NaN() );
            return CVariant( (Float32) ( (Float32) random.Between( -40, 40 ) * 0.25f ) );
        }
        case DATATABLE_TYPE_FLOAT64:
        {
            if ( random.Chance( 3 ) )
                return CVariant( std::numeric_limits< Float64 >::quiet_NaN() );
            if ( random.Chance( 3 ) )
                return CVariant( (Float64) -0.0 );
            return CVariant( (Float64) random.Between( -2000, 2000 ) * 0.125 );
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            CDecimal value;
            CDecimal::FromUnscaled( random.Between( -500000, 500000 ), column.decimalScale, value );
            return CDataTableValues::Decimal( value );
        }
        case DATATABLE_TYPE_DATE:       { return CDataTableValues::Date( (Int32) random.Between( -800, 800 ) ); }
        case DATATABLE_TYPE_TIMESTAMP:  { return CDataTableValues::Timestamp( random.Between( -5000, 5000 ) * 1000, column.timeUnit ); }
        case DATATABLE_TYPE_UTF8:
        {
            static const char* words[] = { "", "a", "ab", "abc", "b", "ba", "z", "KO", "XNYS", "\xC3\xA9t\xC3\xA9", "zz", "AAPL" };
            return CVariant( CUtf8String( words[ random.Below( sizeof( words ) / sizeof( words[ 0 ] ) ) ] ) );
        }
        case DATATABLE_TYPE_BINARY:
        {
            UInt8 bytes[ 4 ];
            const UInt32 length = random.Below( 4 );
            for ( UInt32 i=0; i<length; ++i )
                bytes[ i ] = (UInt8) random.Below( 3 ) * 0x7F;
            CVariant value;
            value.Set( bytes, length, GUCEF_DATATYPE_BINARY_BLOB, false );
            return value;
        }
        default:                        { return CVariant(); }
    }
}

/*-------------------------------------------------------------------------*/

/** Orders a cell and a literal of the column's own kind: false when unordered (NaN). Brute force, independent of the pruner */
inline bool
CompareCellToLiteral( const GUCEF::CORE::CDataTableColumnDef& column, const GUCEF::CORE::CVariant& cell, const GUCEF::CORE::CVariant& literal, GUCEF::Int32& order )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    order = 0;
    switch ( column.type )
    {
        case DATATABLE_TYPE_BOOL:
        {
            const int a = cell.AsBool() ? 1 : 0;
            const int b = literal.AsBool() ? 1 : 0;
            order = a < b ? -1 : ( a > b ? 1 : 0 );
            return true;
        }
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:
        {
            const UInt64 a = cell.AsUInt64();
            const UInt64 b = literal.AsUInt64();
            order = a < b ? -1 : ( a > b ? 1 : 0 );
            return true;
        }
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_FLOAT64:
        {
            const Float64 a = cell.AsFloat64();
            const Float64 b = literal.AsFloat64();
            if ( a != a || b != b )
                return false;
            order = a < b ? -1 : ( a > b ? 1 : 0 );
            return true;
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            CDecimal a;
            CDecimal b;
            cell.GetDecimal( a );
            literal.GetDecimal( b );
            order = a.Compare( b );
            return true;
        }
        case DATATABLE_TYPE_UTF8:
        case DATATABLE_TYPE_BINARY:
        {
            UInt32 aLength = 0;
            UInt32 bLength = 0;
            const char* a = CDataTablePruner::GetNativeBytes( cell, aLength );
            const char* b = CDataTablePruner::GetNativeBytes( literal, bLength );
            const UInt32 common = aLength < bLength ? aLength : bLength;
            const int result = common > 0 ? memcmp( a, b, common ) : 0;
            order = 0 != result ? ( result < 0 ? -1 : 1 ) : ( aLength < bLength ? -1 : ( aLength > bLength ? 1 : 0 ) );
            return true;
        }
        default:
        {
            // signed integers, dates and timestamps of the column unit
            CVariant left( cell );
            CVariant right( literal );
            left.ToNativeEndian();
            right.ToNativeEndian();
            const Int64 a = left.AsInt64();
            const Int64 b = right.AsInt64();
            order = a < b ? -1 : ( a > b ? 1 : 0 );
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Whether a cell satisfies a predicate with a literal of the column's own kind: SQL nulls, IEEE NaN */
inline bool
BruteForceMatches( const GUCEF::CORE::CDataTableColumnDef& column, const GUCEF::CORE::CVariant& cell, const GUCEF::CORE::CDataTablePredicate& predicate )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    const bool isNull = !cell.IsInitialized() || cell.IsNULL();
    if ( DATATABLE_PREDICATE_IS_NULL == predicate.op )
        return isNull;
    if ( DATATABLE_PREDICATE_IS_NOT_NULL == predicate.op )
        return !isNull;
    if ( isNull )
        return false;

    Int32 order = 0;
    switch ( predicate.op )
    {
        case DATATABLE_PREDICATE_EQUALS:            { return CompareCellToLiteral( column, cell, predicate.low, order ) && 0 == order; }
        case DATATABLE_PREDICATE_NOT_EQUALS:        { return !CompareCellToLiteral( column, cell, predicate.low, order ) || 0 != order; }
        case DATATABLE_PREDICATE_LESS:              { return CompareCellToLiteral( column, cell, predicate.low, order ) && order < 0; }
        case DATATABLE_PREDICATE_LESS_OR_EQUAL:     { return CompareCellToLiteral( column, cell, predicate.low, order ) && order <= 0; }
        case DATATABLE_PREDICATE_GREATER:           { return CompareCellToLiteral( column, cell, predicate.low, order ) && order > 0; }
        case DATATABLE_PREDICATE_GREATER_OR_EQUAL:  { return CompareCellToLiteral( column, cell, predicate.low, order ) && order >= 0; }
        case DATATABLE_PREDICATE_RANGE:
        {
            if ( predicate.low.IsInitialized() && !predicate.low.IsNULL() )
            {
                if ( !CompareCellToLiteral( column, cell, predicate.low, order ) || order < 0 || ( 0 == order && !predicate.lowInclusive ) )
                    return false;
            }
            if ( predicate.high.IsInitialized() && !predicate.high.IsNULL() )
            {
                if ( !CompareCellToLiteral( column, cell, predicate.high, order ) || order > 0 || ( 0 == order && !predicate.highInclusive ) )
                    return false;
            }
            return true;
        }
        case DATATABLE_PREDICATE_IN:
        {
            for ( UInt32 i=0; i<(UInt32) predicate.values.size(); ++i )
                if ( CompareCellToLiteral( column, cell, predicate.values[ i ], order ) && 0 == order )
                    return true;
            return false;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The text of a cell, for comparing rows as strings */
inline GUCEF::CORE::CString
CellText( const GUCEF::CORE::CDataTableBatch& batch, GUCEF::UInt32 row, GUCEF::UInt32 column )
{
    using namespace GUCEF::CORE;
    CVariant cell;
    if ( !batch.GetValue( row, column, cell, false ) )
        return "<error>";
    if ( cell.IsNULL() )
        return "<null>";
    if ( cell.IsFloat() )
    {
        // the shortest text that reads back as the value: 0.5, 33, -1.25
        const Float64 value = cell.AsFloat64();
        CDecimal shortest;
        if ( value != value )
            return "NaN";
        if ( DecimalStatusIsError( CDecimal::FromFloat64Shortest( value, shortest ) ) )
            return value < 0.0 ? "-Inf" : "Inf";
        return shortest.ToString();
    }
    if ( GUCEF_DATATYPE_BINARY_BLOB == cell.GetTypeId() )
    {
        CString hex;
        const GUCEF::UInt8* bytes = static_cast< const GUCEF::UInt8* >( cell.AsVoidPtr() );
        for ( GUCEF::UInt32 i=0; i<cell.ByteSize(); ++i )
            hex += ToString( (GUCEF::UInt32) bytes[ i ] ) + ".";
        return hex;
    }
    return cell.AsString();
}

/** Every selected row of the batch as text, columns separated by '|' */
inline void
AppendBatchRows( const GUCEF::CORE::CDataTableBatch& batch, GUCEF::CORE::CString::StringVector& rows )
{
    const GUCEF::UInt32* selection = batch.GetSelection();
    for ( GUCEF::UInt32 i=0; i<batch.GetSelectedRowCount(); ++i )
    {
        const GUCEF::UInt32 row = GUCEF_NULL == selection ? i : selection[ i ];
        GUCEF::CORE::CString text;
        for ( GUCEF::UInt32 c=0; c<batch.GetColumnCount(); ++c )
            text += CellText( batch, row, c ) + "|";
        rows.push_back( text );
    }
}

/** Pulls every batch of a scan into rows of text; the status that ended the scan */
inline GUCEF::CORE::TDataTableStatus
CollectScanRows( GUCEF::CORE::CDataTableScan& scan, GUCEF::CORE::CString::StringVector& rows, GUCEF::UInt32* batchCount = GUCEF_NULL )
{
    using namespace GUCEF::CORE;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    while ( true )
    {
        CDataTableBatchPtr batch;
        status = scan.NextBatch( batch );
        if ( DATATABLE_STATUS_OK != status )
            break;
        if ( GUCEF_NULL != batchCount )
            ++(*batchCount);
        AppendBatchRows( *batch, rows );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** A random full batch of the schema; column "id", when present, numbers the rows from firstId */
inline GUCEF::CORE::CDataTableBatchPtr
MakeRandomBatch( const GUCEF::CORE::CDataTableSchemaPtr& schema, GUCEF::CORE::CDataTableBufferPool& pool, CTestRandom& random,
                 GUCEF::UInt32 rowCount, GUCEF::Int64 firstId, GUCEF::UInt32 nullPercent )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    CDataTableBatchBuilder builder( schema, pool );
    const UInt32 columnCount = schema->GetColumnCount();
    const Int32 idColumn = schema->FindColumn( "id" );
    GUCEF::vector< CVariant > cells( columnCount );
    for ( UInt32 r=0; r<rowCount; ++r )
    {
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            if ( (Int32) c == idColumn )
                cells[ c ] = CVariant( firstId + (Int64) r );
            else
                cells[ c ] = RandomCellValue( random, schema->GetColumn( c ), nullPercent );
        }
        builder.AppendRow( &cells[ 0 ], columnCount );
    }
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/*-------------------------------------------------------------------------*/

/** Counters shared by the readers of a CTestFileListSource */
struct STestSourceCounters
{
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > batchesRead;      /**< ReadNextBatch calls that gave a batch */
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > readersOpened;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > readersAlive;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > concurrentReads;  /**< ReadNextBatch calls in progress */
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > maxConcurrentReads;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > concurrentOpens;  /**< CIDataTableScanSource::OpenFile calls in progress */
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > maxConcurrentOpens;

    STestSourceCounters( void ) : batchesRead( 0 ), readersOpened( 0 ), readersAlive( 0 ), concurrentReads( 0 ), maxConcurrentReads( 0 ),
                                  concurrentOpens( 0 ), maxConcurrentOpens( 0 ) {}
};

/** Forwards to a reader and counts what it does */
class CCountingTableReader : public GUCEF::CORE::CITableReader
{
    public:

    CCountingTableReader( const GUCEF::CORE::CITableReaderPtr& reader, STestSourceCounters& counters )
        : GUCEF::CORE::CITableReader(), m_reader( reader ), m_counters( counters )
    {
        m_counters.readersOpened.FetchAdd( 1 );
        m_counters.readersAlive.FetchAdd( 1 );
    }

    virtual ~CCountingTableReader() GUCEF_VIRTUAL_OVERRIDE { m_counters.readersAlive.FetchSub( 1 ); }

    virtual const GUCEF::CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_reader->GetMetaData(); }

    virtual GUCEF::CORE::TDataTableStatus OpenRowGroup( GUCEF::UInt32 rowGroup, const GUCEF::CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE
    {
        return m_reader->OpenRowGroup( rowGroup, request );
    }

    virtual GUCEF::CORE::TDataTableStatus ReadNextBatch( GUCEF::CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE
    {
        const GUCEF::UInt32 concurrent = m_counters.concurrentReads.FetchAdd( 1 ) + 1;
        GUCEF::UInt32 seen = m_counters.maxConcurrentReads.Load();
        while ( concurrent > seen && !m_counters.maxConcurrentReads.CompareExchangeWeak( seen, concurrent ) ) {}
        GUCEF::CORE::TDataTableStatus status = m_reader->ReadNextBatch( batch );
        m_counters.concurrentReads.FetchSub( 1 );
        if ( GUCEF::CORE::DATATABLE_STATUS_OK == status )
            m_counters.batchesRead.FetchAdd( 1 );
        return status;
    }

    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_reader->GetLastErrorMessage(); }

    private:

    GUCEF::CORE::CITableReaderPtr m_reader;
    STestSourceCounters& m_counters;
};

/*-------------------------------------------------------------------------*/

/** Test table files held in memory, listed in order; readers made by CTestTableCodec, counted, with faults on request */
class CTestFileListSource : public GUCEF::CORE::CIDataTableScanSource
{
    public:

    struct STestFile
    {
        GUCEF::CORE::IOAccessPtr access;
        GUCEF::CORE::CITableFileMetaDataPtr metaData;
        GUCEF::MT::CMutex* lock;
    };
    typedef GUCEF::vector< STestFile > TFileVector;

    CTestFileListSource( const TFileVector& files, const GUCEF::CORE::CValueList& codecOptions, STestSourceCounters& counters )
        : GUCEF::CORE::CIDataTableScanSource()
        , m_files( files )
        , m_codecOptions( codecOptions )
        , m_counters( counters )
        , m_next( 0 )
        , m_nextFileDelayMs( codecOptions.GetValueAlways( "nextFileDelayMs", GUCEF::CORE::CVariant( (GUCEF::Int64) 0 ) ).AsUInt32() )
        , m_openFileDelayMs( codecOptions.GetValueAlways( "openFileDelayMs", GUCEF::CORE::CVariant( (GUCEF::Int64) 0 ) ).AsUInt32() )
        , m_failNextFile( codecOptions.GetValueAlways( "failNextFile", GUCEF::CORE::CVariant( (GUCEF::Int64) -1 ) ).AsInt64() )
        , m_failOpenFile( codecOptions.GetValueAlways( "failOpenFile", GUCEF::CORE::CVariant( (GUCEF::Int64) -1 ) ).AsInt64() )
    {
    }

    virtual GUCEF::CORE::TDataTableStatus NextFile( GUCEF::CORE::CDataTableScanFile& file ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        if ( m_nextFileDelayMs > 0 )
            GUCEF::MT::PrecisionDelay( m_nextFileDelayMs );
        if ( (GUCEF::Int64) m_next == m_failNextFile )
            return DATATABLE_STATUS_IO_ERROR;
        if ( m_next >= (GUCEF::UInt32) m_files.size() )
            return DATATABLE_STATUS_END;
        file.Clear();
        file.path = "file" + ToString( m_next );
        file.backendData = m_next;
        file.sharedFileLock = m_files[ m_next ].lock;
        if ( !m_files[ m_next ].metaData.IsNULL() && 0 != ( m_next & 1 ) )
            file.metaData = m_files[ m_next ].metaData;     // odd files come with cached metadata
        ++m_next;
        return DATATABLE_STATUS_OK;
    }

    virtual GUCEF::CORE::TDataTableStatus OpenFile( const GUCEF::CORE::CDataTableScanFile& file, GUCEF::CORE::CITableReaderPtr& reader ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF::CORE;
        if ( (GUCEF::Int64) file.backendData == m_failOpenFile )
            return DATATABLE_STATUS_IO_ERROR;
        if ( m_openFileDelayMs > 0 )
        {
            // the latency of opening a file: the scan may open several at once
            const GUCEF::UInt32 concurrent = m_counters.concurrentOpens.FetchAdd( 1 ) + 1;
            GUCEF::UInt32 seen = m_counters.maxConcurrentOpens.Load();
            while ( concurrent > seen && !m_counters.maxConcurrentOpens.CompareExchangeWeak( seen, concurrent ) ) {}
            GUCEF::MT::PrecisionDelay( m_openFileDelayMs );
            m_counters.concurrentOpens.FetchSub( 1 );
        }
        const STestFile& testFile = m_files[ (GUCEF::UInt32) file.backendData ];
        CTestTableCodec codec;
        CITableReaderPtr inner;
        CString error;
        TDataTableStatus status = codec.CreateReader( testFile.access, testFile.metaData, m_codecOptions, inner, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        reader = CITableReaderPtr( GUCEF_NEW CCountingTableReader( inner, m_counters ) );
        return DATATABLE_STATUS_OK;
    }

    private:

    TFileVector m_files;
    GUCEF::CORE::CValueList m_codecOptions;
    STestSourceCounters& m_counters;
    GUCEF::UInt32 m_next;
    GUCEF::UInt32 m_nextFileDelayMs;
    GUCEF::UInt32 m_openFileDelayMs;
    GUCEF::Int64 m_failNextFile;
    GUCEF::Int64 m_failOpenFile;
};

/*-------------------------------------------------------------------------*/

/**
 *  Test files of a schema: fileCount files of rowGroupsPerFile row groups of rowsPerRowGroup rows, column "id"
 *  numbering all rows in order. The locks are owned by the holder and outlive the scans.
 */
class CTestFileSet
{
    public:

    CTestFileSet( void ) : files(), schema(), locks() {}

    ~CTestFileSet()
    {
        files.clear();
        for ( GUCEF::UInt32 i=0; i<(GUCEF::UInt32) locks.size(); ++i )
            GUCEF_DELETE locks[ i ];
    }

    bool Make( const GUCEF::CORE::CDataTableSchemaPtr& tableSchema, GUCEF::CORE::CDataTableBufferPool& pool, GUCEF::UInt64 seed,
               GUCEF::UInt32 fileCount, GUCEF::UInt32 rowGroupsPerFile, GUCEF::UInt32 rowsPerRowGroup, GUCEF::UInt32 nullPercent )
    {
        using namespace GUCEF::CORE;
        schema = tableSchema;
        CTestRandom random( seed );
        GUCEF::Int64 id = 0;
        for ( GUCEF::UInt32 f=0; f<fileCount; ++f )
        {
            GUCEF::vector< CDataTableBatchPtr > batches;
            for ( GUCEF::UInt32 g=0; g<rowGroupsPerFile; ++g )
            {
                batches.push_back( MakeRandomBatch( schema, pool, random, rowsPerRowGroup, id, nullPercent ) );
                id += rowsPerRowGroup;
            }
            CTestFileListSource::STestFile file;
            if ( DATATABLE_STATUS_OK != WriteTestTableFile( batches, schema, file.access ) )
                return false;
            CTestTableCodec codec;
            CString error;
            if ( DATATABLE_STATUS_OK != codec.ReadMetaData( *file.access, CValueList(), file.metaData, error ) )
                return false;
            file.lock = GUCEF_NEW GUCEF::MT::CMutex();
            locks.push_back( file.lock );
            files.push_back( file );
        }
        return true;
    }

    /** A scan over the files with the codec options (faults, delays) */
    GUCEF::CORE::CDataTableScanPtr MakeScan( GUCEF::CORE::CDataTableBufferPool& pool, STestSourceCounters& counters,
                                             const GUCEF::CORE::CValueList& options = GUCEF::CORE::CValueList(),
                                             GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::PulseGeneratorPtr() ) const
    {
        using namespace GUCEF::CORE;
        CDataTableScanSourcePtr source( GUCEF_NEW CTestFileListSource( files, options, counters ) );
        return CDataTableScanPtr( GUCEF_NEW CDataTableScan( source, schema, pool, pulseGenerator ) );
    }

    CTestFileListSource::TFileVector files;
    GUCEF::CORE::CDataTableSchemaPtr schema;
    GUCEF::vector< GUCEF::MT::CMutex* > locks;

    private:

    CTestFileSet( const CTestFileSet& src );
    CTestFileSet& operator=( const CTestFileSet& src );
};

/*-------------------------------------------------------------------------*/

/** A thread pool of its own for a test, with the given number of worker threads */
inline GUCEF::CORE::ThreadPoolPtr
GetDataTableTestThreadPool( const char* name, GUCEF::UInt32 threads )
{
    GUCEF::CORE::ThreadPoolPtr pool = GUCEF::CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( name );
    if ( !pool.IsNULL() && pool->GetDesiredMinNrOfWorkerThreads() < threads )
        pool->SetDesiredMinNrOfWorkerThreads( threads );
    return pool;
}

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCANSUPPORT_H ? */
