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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#include "gucefCORE_CCsvTableReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline UInt32
TextLength( const CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** The date formats a date column is read with: its own, else the one the inference found, else the list of the options */
static void
ResolveDateFormats( const CDataTableColumnDef& column, UInt32 columnIndex, const CValueList& keyValues, const SCsvReadOptions& options,
                    SCsvReadOptions::TDateFormatVector& formats )
{GUCEF_TRACE;

    formats.clear();
    if ( columnIndex < options.schemaColumns.size() && options.schemaColumns[ columnIndex ].hasDateFormat )
    {
        formats.push_back( options.schemaColumns[ columnIndex ].dateFormat );
        return;
    }
    for ( size_t i = 0; i < options.columnOverrides.size(); ++i )
    {
        if ( options.columnOverrides[ i ].column.name == column.name && options.columnOverrides[ i ].hasDateFormat )
        {
            formats.push_back( options.columnOverrides[ i ].dateFormat );
            return;
        }
    }

    const CString key = "dateFormat." + column.name;
    if ( keyValues.HasKey( key ) )
    {
        CCsvTableText::SDateFormat inferred;
        CString ignored;
        if ( CCsvTableText::CompileDateFormat( keyValues.GetValueAlways( key ).AsString(), inferred, ignored ) )
        {
            formats.push_back( inferred );
            return;
        }
    }
    formats = options.dateFormats;
}

/*-------------------------------------------------------------------------*/

CCsvTableMetaData::CCsvTableMetaData( const CDataTableSchemaPtr& schema, const CValueList& keyValues )
    : CITableFileMetaData()
    , m_schema( schema )
    , m_keyValues( keyValues )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCsvTableMetaData::~CCsvTableMetaData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CDataTableSchemaPtr&
CCsvTableMetaData::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt64
CCsvTableMetaData::GetRowCount( void ) const
{GUCEF_TRACE;

    return GUCEF_UINT64MAX;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableMetaData::GetRowGroupCount( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableMetaData::GetRowGroupInfo( UInt32 rowGroup, CDataTableRowGroupInfo& info ) const
{GUCEF_TRACE;

    if ( 0 != rowGroup )
        return false;

    info.Clear();
    info.rowCount = GUCEF_UINT64MAX;
    if ( !m_schema.IsNULL() )
        info.columnStats.resize( m_schema->GetColumnCount() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CCsvTableMetaData::GetMemoryFootprint( void ) const
{GUCEF_TRACE;

    UInt64 bytes = sizeof( *this );
    if ( !m_schema.IsNULL() )
        bytes += (UInt64) m_schema->GetColumnCount() * 128;
    bytes += (UInt64) m_keyValues.GetKeyCount() * 64;
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CCsvTableMetaData::GetKeyValueMetaData( CValueList& keyValues ) const
{GUCEF_TRACE;

    keyValues = m_keyValues;
}

/*-------------------------------------------------------------------------*/

CCsvRecordReader::CCsvRecordReader( CIOAccess& source               ,
                                    const IOAccessPtr& sourceHolder ,
                                    CDataTableBufferPool& pool      ,
                                    const SCsvReadOptions& options  ,
                                    MT::CMutex* sharedFileLock      )
    : m_source( source )
    , m_shared( GUCEF_NULL != sharedFileLock )
    , m_options( options )
    , m_parser( source, sourceHolder, pool, options, sharedFileLock )
    , m_synthetic( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordReader::Start( CString& error )
{GUCEF_TRACE;

    // A file is read from its first byte. Input shared with other readers is positioned for every read by the parser
    if ( !m_shared && 0 != m_source.Tell() )
    {
        TDataTableStatus status = m_parser.Rewind( error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }
    return m_parser.SkipLines( m_options.skipLeadingLines, error );
}

/*-------------------------------------------------------------------------*/

bool
CCsvRecordReader::MatchesSkipPattern( void ) const
{GUCEF_TRACE;

    // The fields as the table sees them, joined with the delimiter
    CString line;
    for ( UInt32 i = 0; i < m_parser.GetFieldCount(); ++i )
    {
        if ( i > 0 )
            line += m_options.delimiter;
        const UInt32 length = m_parser.GetFieldLength( i );
        if ( length > 0 )
            line += CString( m_parser.GetFieldData( i ), length );
    }

    for ( size_t n = 0; n < m_options.skipLinesMatching.size(); ++n )
        if ( m_options.skipLinesMatching[ n ].IsMatch( line ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvRecordReader::Next( UInt32 columnCountHint, bool& gotRecord, CString& error )
{GUCEF_TRACE;

    m_synthetic = false;
    for ( ;; )
    {
        TDataTableStatus status = m_parser.NextRecord( gotRecord, error );
        if ( DATATABLE_STATUS_OK != status || !gotRecord )
            return status;

        if ( m_parser.IsBlank() )
        {
            // an empty line is a row in a table of one column, and nothing otherwise
            if ( 1 == columnCountHint )
            {
                m_synthetic = true;
                return DATATABLE_STATUS_OK;
            }
            continue;
        }

        if ( !m_options.skipLinesMatching.empty() && MatchesSkipPattern() )
            continue;
        return DATATABLE_STATUS_OK;
    }
}

/*-------------------------------------------------------------------------*/

CCsvTableReader::CCsvTableReader( const IOAccessPtr& source              ,
                                  const CITableFileMetaDataPtr& metaData ,
                                  const SCsvReadOptions& options         )
    : CITableReader()
    , m_source( source )
    , m_metaData( metaData )
    , m_options( options )
    , m_conversions()
    , m_fileSchema()
    , m_request()
    , m_pool( GUCEF_NULL )
    , m_records( GUCEF_NULL )
    , m_converter( GUCEF_NULL )
    , m_projectedSchema()
    , m_builder( GUCEF_NULL )
    , m_projection()
    , m_batchRowCount( DataTableDefaultBatchRowCount )
    , m_rowsRead( 0 )
    , m_valueErrors( 0 )
    , m_open( false )
    , m_ended( false )
    , m_failed( false )
    , m_failedStatus( DATATABLE_STATUS_OK )
    , m_lastError()
{GUCEF_TRACE;

    if ( !m_metaData.IsNULL() )
    {
        m_fileSchema = m_metaData->GetSchema();
        CValueList keyValues;
        m_metaData->GetKeyValueMetaData( keyValues );
        if ( !m_fileSchema.IsNULL() )
        {
            for ( UInt32 i = 0; i < m_fileSchema->GetColumnCount(); ++i )
            {
                SCsvColumnConversion conversion;
                conversion.column = m_fileSchema->GetColumn( i );
                if ( DATATABLE_TYPE_DATE == conversion.column.type )
                    ResolveDateFormats( conversion.column, i, keyValues, m_options, conversion.dateFormats );
                m_conversions.push_back( conversion );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CCsvTableReader::~CCsvTableReader()
{GUCEF_TRACE;

    GUCEF_DELETE m_builder;
    m_builder = GUCEF_NULL;
    GUCEF_DELETE m_converter;
    m_converter = GUCEF_NULL;
    GUCEF_DELETE m_records;
    m_records = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CITableFileMetaDataPtr&
CCsvTableReader::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

CString
CCsvTableReader::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableReader::Fail( TDataTableStatus status, const CString& message )
{GUCEF_TRACE;

    m_lastError = message;
    m_failed = true;
    m_failedStatus = status;
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableReader::ReadHeader( void )
{GUCEF_TRACE;

    bool gotRecord = false;
    CString error;
    const UInt32 columnCount = m_fileSchema->GetColumnCount();
    TDataTableStatus status = m_records->Next( columnCount, gotRecord, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    if ( !gotRecord )
        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "The file has no header row" );

    if ( m_records->GetFieldCount() != columnCount )
    {
        return Fail( DATATABLE_STATUS_SCHEMA_MISMATCH, "The header has " + UInt32ToString( m_records->GetFieldCount() ) + " columns where the schema has " +
                     UInt32ToString( columnCount ) + " (" + m_records->GetParser().DescribeLast() + ")" );
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableReader::OpenRowGroup( UInt32 rowGroup, const CDataTableReadRequest& request )
{GUCEF_TRACE;

    m_failed = false;
    m_failedStatus = DATATABLE_STATUS_OK;
    m_lastError.Clear();
    m_open = false;
    m_ended = false;

    if ( 0 != rowGroup )
        return Fail( DATATABLE_STATUS_NOT_FOUND, "A CSV file has one row group, not row group " + UInt32ToString( rowGroup ) );
    if ( m_source.IsNULL() || m_fileSchema.IsNULL() || m_fileSchema->GetColumnCount() != m_conversions.size() )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The reader has no source or no schema" );

    GUCEF_DELETE m_builder;
    m_builder = GUCEF_NULL;
    GUCEF_DELETE m_converter;
    m_converter = GUCEF_NULL;
    GUCEF_DELETE m_records;
    m_records = GUCEF_NULL;

    m_request = request;
    m_pool = GUCEF_NULL != request.bufferPool ? request.bufferPool : &CCoreGlobal::Instance()->GetDataTableBufferPool();
    m_batchRowCount = request.batchRowCount;
    if ( 0 == m_batchRowCount )
        m_batchRowCount = DataTableDefaultBatchRowCount;
    if ( m_batchRowCount > DataTableMaxBatchRowCount )
        m_batchRowCount = DataTableMaxBatchRowCount;

    // the projection: every column when none is asked for
    const UInt32 columnCount = m_fileSchema->GetColumnCount();
    m_projection = request.projection;
    if ( m_projection.empty() )
    {
        for ( UInt32 i = 0; i < columnCount; ++i )
            m_projection.push_back( i );
    }
    CDataTableSchema projected;
    for ( size_t p = 0; p < m_projection.size(); ++p )
    {
        if ( m_projection[ p ] >= columnCount )
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names column " + UInt32ToString( m_projection[ p ] ) + " of a file with " + UInt32ToString( columnCount ) + " columns" );
        if ( !projected.AddColumn( m_fileSchema->GetColumn( m_projection[ p ] ) ) )
            return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names a column twice" );
    }
    m_projectedSchema = CreateDataTableSchemaPtr( projected );

    m_records = GUCEF_NEW CCsvRecordReader( *m_source, m_source, *m_pool, m_options, request.sharedFileLock );
    m_converter = GUCEF_NEW CCsvColumnConverter( m_options );
    m_builder = GUCEF_NEW CDataTableBatchBuilder( m_projectedSchema, *m_pool );

    CString error;
    TDataTableStatus status = m_records->Start( error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    if ( m_options.hasHeader )
    {
        status = ReadHeader();
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }

    m_rowsRead = 0;
    m_valueErrors = 0;
    m_open = true;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableReader::AppendRecord( void )
{GUCEF_TRACE;

    const UInt32 columnCount = m_fileSchema->GetColumnCount();
    const UInt32 fieldCount = m_records->GetFieldCount();
    if ( fieldCount < columnCount && !m_options.allowMissingFields )
    {
        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A record has " + UInt32ToString( fieldCount ) + " fields where the file has " + UInt32ToString( columnCount ) +
                     " columns (" + m_records->GetParser().DescribeLast() + ")" );
    }
    if ( fieldCount > columnCount && !m_options.allowExtraFields )
    {
        return Fail( DATATABLE_STATUS_CORRUPT_DATA, "A record has " + UInt32ToString( fieldCount ) + " fields where the file has " + UInt32ToString( columnCount ) +
                     " columns (" + m_records->GetParser().DescribeLast() + ")" );
    }

    CString error;
    for ( size_t p = 0; p < m_projection.size(); ++p )
    {
        const UInt32 fileColumn = m_projection[ p ];
        CDataTableColumnBuilder& column = m_builder->GetColumn( (UInt32) p );
        TDataTableStatus status = DATATABLE_STATUS_OK;
        if ( fileColumn >= fieldCount )
            status = m_converter->ConvertMissing( m_conversions[ fileColumn ], column, error );
        else
            status = m_converter->Convert( m_conversions[ fileColumn ], m_records->GetFieldData( fileColumn ), m_records->GetFieldLength( fileColumn ),
                                           m_records->IsFieldQuoted( fileColumn ), column, m_valueErrors, error );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status, error + " (" + m_records->GetParser().DescribeLast() + ")" );
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableReader::ReadNextBatch( CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    if ( m_failed )
        return m_failedStatus;
    if ( !m_open )
        return Fail( DATATABLE_STATUS_INVALID_ARGUMENT, "ReadNextBatch before OpenRowGroup" );
    if ( m_ended )
        return DATATABLE_STATUS_END;

    const UInt64 consumedBefore = m_records->GetParser().GetBytesConsumed();
    const UInt64 valueErrorsBefore = m_valueErrors;
    const UInt32 columnCount = m_fileSchema->GetColumnCount();

    UInt32 rows = 0;
    UInt64 firstRow = m_rowsRead;
    UInt64 textBytes = 0;
    CString error;
    while ( rows < m_batchRowCount && textBytes < m_options.batchByteLimit )
    {
        bool gotRecord = false;
        TDataTableStatus status = m_records->Next( columnCount, gotRecord, error );
        if ( DATATABLE_STATUS_OK != status )
            return Fail( status, error );
        if ( !gotRecord )
        {
            m_ended = true;
            break;
        }

        status = AppendRecord();
        if ( DATATABLE_STATUS_OK != status )
            return status;
        for ( UInt32 f = 0; f < m_records->GetFieldCount(); ++f )
            textBytes += m_records->GetFieldLength( f );
        ++rows;
        ++m_rowsRead;
    }

    if ( GUCEF_NULL != m_request.stats )
    {
        m_request.stats->bytesRead += m_records->GetParser().GetBytesConsumed() - consumedBefore;
        m_request.stats->valueErrors += m_valueErrors - valueErrorsBefore;
    }

    if ( 0 == rows )
        return DATATABLE_STATUS_END;

    TDataTableStatus status = m_builder->Build( batch, error );
    if ( DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    // The batch is built over the projected schema; it is delivered with the schema of the file and the file's column indices
    CDataTableBatch* built = batch.GetWritable();
    built->SetSchema( m_fileSchema );
    for ( size_t p = 0; p < m_projection.size(); ++p )
        built->SetSchemaColumnIndex( (UInt32) p, m_projection[ p ] );
    CDataTableBatchOrigin origin;
    origin.fileOrdinal = m_request.fileOrdinal;
    origin.rowGroup = 0;
    origin.firstRowInRowGroup = firstRow;
    built->SetOrigin( origin );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
