/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H
#include "tablecodecpluginXLSX_CXlsxSchemaResolver.h"
#define TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#include "tablecodecpluginXLSX_CXlsxColumnConverter.h"
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H ? */

#include "tablecodecpluginXLSX_CXlsxTableReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Builds the batches of the rows of the sheet as they are reported. A row is appended column by column: the cell goes
 *  into the builder of its column and the columns the row has no cell for get a null. A row without a cell with a value
 *  appends nothing, so it is not a row of the table.
 */
class CXlsxBatchSink : public CIXlsxRowSink
{
    public:

    CXlsxBatchSink( const SXlsxReadOptions& options, const CXlsxColumnConverter& converter, const SXlsxLayout& layout,
                    const CORE::CDataTableSchemaPtr& fileSchema, const CORE::CDataTableSchemaPtr& projectedSchema,
                    const CORE::CDataTableSchema::TColumnIndexVector& projection, CORE::CDataTableBufferPool& pool,
                    UInt32 batchRowCount, UInt32 fileOrdinal, GUCEF::vector< CORE::CDataTableBatchPtr >& batches )
        : CIXlsxRowSink()
        , m_options( options )
        , m_converter( converter )
        , m_layout( layout )
        , m_fileSchema( fileSchema )
        , m_projection( projection )
        , m_projectedPosition( layout.width, -1 )
        , m_builder( projectedSchema, pool )
        , m_batchRowCount( batchRowCount )
        , m_fileOrdinal( fileOrdinal )
        , m_batches( batches )
        , m_skip( true )
        , m_started( false )
        , m_nextFileColumn( 0 )
        , m_builderRows( 0 )
        , m_rowsRead( 0 )
        , m_valueErrors( 0 )
    {
        for ( size_t p = 0; p < projection.size(); ++p )
            m_projectedPosition[ projection[ p ] ] = (Int32) p;
    }

    UInt64 GetRowsRead( void ) const { return m_rowsRead; }
    UInt64 GetValueErrors( void ) const { return m_valueErrors; }

    virtual CORE::TDataTableStatus OnRowBegin( UInt32 row, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_skip = row < m_layout.dataStartRow;
        m_started = false;
        m_nextFileColumn = 0;
        return CORE::DATATABLE_STATUS_OK;
    }

    virtual CORE::TDataTableStatus OnCell( UInt32 row, UInt32 column, const SXlsxCell& cell, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_skip || column < m_layout.firstColumn || ( 0 != m_options.lastColumn && column >= m_options.lastColumn ) )
            return CORE::DATATABLE_STATUS_OK;
        const UInt32 index = column - m_layout.firstColumn;
        if ( index >= m_layout.width )
        {
            if ( !m_options.allowExtraCells )
            {
                error = "Row " + CORE::UInt32ToString( row ) + " has a value in " + CXlsxSheetParser::FormatCellReference( column, row ) +
                        ", right of the table of " + CORE::UInt32ToString( m_layout.width ) + " columns (allowExtraCells)";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
            return CORE::DATATABLE_STATUS_OK;
        }
        m_started = true;

        CORE::TDataTableStatus status = PadBlanks( row, m_nextFileColumn, index, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        m_nextFileColumn = index + 1;

        const Int32 position = m_projectedPosition[ index ];
        if ( position < 0 )
            return CORE::DATATABLE_STATUS_OK;
        CORE::CString reason;
        status = m_converter.Convert( m_fileSchema->GetColumn( index ), cell, m_builder.GetColumn( (UInt32) position ), m_valueErrors, reason );
        if ( CORE::DATATABLE_STATUS_OK != status )
            error = Describe( row, column, index, reason );
        return status;
    }

    virtual CORE::TDataTableStatus OnRowEnd( UInt32 row, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_skip || !m_started )
            return CORE::DATATABLE_STATUS_OK;
        CORE::TDataTableStatus status = PadBlanks( row, m_nextFileColumn, m_layout.width, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        ++m_builderRows;
        ++m_rowsRead;
        if ( m_builderRows >= m_batchRowCount )
            return Flush( error );
        return CORE::DATATABLE_STATUS_OK;
    }

    /** Hands out the rows that are left as the last batch */
    CORE::TDataTableStatus Flush( CORE::CString& error )
    {
        if ( 0 == m_builderRows )
            return CORE::DATATABLE_STATUS_OK;

        CORE::CDataTableBatchPtr batch;
        CORE::TDataTableStatus status = m_builder.Build( batch, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;

        // The batch is built over the projected schema; it is delivered with the schema of the file and the file's column indices
        CORE::CDataTableBatch* built = batch.GetWritable();
        built->SetSchema( m_fileSchema );
        for ( size_t p = 0; p < m_projection.size(); ++p )
            built->SetSchemaColumnIndex( (UInt32) p, m_projection[ p ] );
        CORE::CDataTableBatchOrigin origin;
        origin.fileOrdinal = m_fileOrdinal;
        origin.rowGroup = 0;
        origin.firstRowInRowGroup = m_rowsRead - m_builderRows;
        built->SetOrigin( origin );
        m_batches.push_back( batch );
        m_builderRows = 0;
        return CORE::DATATABLE_STATUS_OK;
    }

    private:

    CORE::CString Describe( UInt32 row, UInt32 column, UInt32 index, const CORE::CString& reason ) const
    {
        return "Row " + CORE::UInt32ToString( row ) + ", " + CXlsxSheetParser::FormatCellReference( column, row ) + " (column \"" +
               m_fileSchema->GetColumn( index ).name + "\"): " + reason;
    }

    CORE::TDataTableStatus PadBlanks( UInt32 row, UInt32 from, UInt32 to, CORE::CString& error )
    {
        for ( UInt32 index = from; index < to; ++index )
        {
            const Int32 position = m_projectedPosition[ index ];
            if ( position < 0 )
                continue;
            CORE::CString reason;
            const CORE::TDataTableStatus status = m_converter.ConvertBlank( m_fileSchema->GetColumn( index ), m_builder.GetColumn( (UInt32) position ), reason );
            if ( CORE::DATATABLE_STATUS_OK != status )
            {
                error = Describe( row, m_layout.firstColumn + index, index, reason );
                return status;
            }
        }
        return CORE::DATATABLE_STATUS_OK;
    }

    const SXlsxReadOptions& m_options;
    const CXlsxColumnConverter& m_converter;
    SXlsxLayout m_layout;
    CORE::CDataTableSchemaPtr m_fileSchema;
    CORE::CDataTableSchema::TColumnIndexVector m_projection;
    GUCEF::vector< Int32 > m_projectedPosition;     /**< per file column its place in the projection, -1 when it is not delivered */
    CORE::CDataTableBatchBuilder m_builder;
    UInt32 m_batchRowCount;
    UInt32 m_fileOrdinal;
    GUCEF::vector< CORE::CDataTableBatchPtr >& m_batches;
    bool m_skip;
    bool m_started;
    UInt32 m_nextFileColumn;
    UInt32 m_builderRows;
    UInt64 m_rowsRead;
    UInt64 m_valueErrors;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CXlsxTableReader::CXlsxTableReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData, const SXlsxReadOptions& options )
    : CORE::CITableReader()
    , m_source( source )
    , m_metaData( metaData )
    , m_options( options )
    , m_batches()
    , m_nextBatch( 0 )
    , m_open( false )
    , m_failed( false )
    , m_failedStatus( CORE::DATATABLE_STATUS_OK )
    , m_lastError()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxTableReader::~CXlsxTableReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CITableFileMetaDataPtr&
CXlsxTableReader::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxTableReader::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableReader::Fail( CORE::TDataTableStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_lastError = message;
    m_failed = true;
    m_failedStatus = status;
    m_batches.clear();
    m_nextBatch = 0;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableReader::OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request )
{GUCEF_TRACE;

    m_failed = false;
    m_failedStatus = CORE::DATATABLE_STATUS_OK;
    m_lastError.Clear();
    m_open = false;
    m_batches.clear();
    m_nextBatch = 0;

    if ( 0 != rowGroup )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "A worksheet has one row group, not row group " + CORE::UInt32ToString( rowGroup ) );
    const CXlsxFileMetaData* xlsxMetaData = GUCEF_NULL;
    if ( !m_metaData.IsNULL() )
    {
        #ifdef GUCEF_RTTI_IS_SUPPORTED
        xlsxMetaData = dynamic_cast< const CXlsxFileMetaData* >( m_metaData.GetPointerAlways() );
        #else
        xlsxMetaData = static_cast< const CXlsxFileMetaData* >( m_metaData.GetPointerAlways() );
        #endif
    }
    if ( m_source.IsNULL() || GUCEF_NULL == xlsxMetaData || m_metaData->GetSchema().IsNULL() )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The reader has no source or no metadata of the xlsx codec" );

    const CORE::CDataTableSchemaPtr fileSchema = m_metaData->GetSchema();
    const SXlsxLayout& layout = xlsxMetaData->GetLayout();
    CORE::CDataTableBufferPool& pool = GUCEF_NULL != request.bufferPool ? *request.bufferPool : CORE::CCoreGlobal::Instance()->GetDataTableBufferPool();

    UInt32 batchRowCount = request.batchRowCount;
    if ( 0 == batchRowCount )
        batchRowCount = CORE::DataTableDefaultBatchRowCount;
    if ( batchRowCount > CORE::DataTableMaxBatchRowCount )
        batchRowCount = CORE::DataTableMaxBatchRowCount;

    // the projection: every column when none is asked for
    const UInt32 columnCount = fileSchema->GetColumnCount();
    CORE::CDataTableSchema::TColumnIndexVector projection = request.projection;
    if ( projection.empty() )
        for ( UInt32 i = 0; i < columnCount; ++i )
            projection.push_back( i );
    CORE::CDataTableSchema projected;
    for ( size_t p = 0; p < projection.size(); ++p )
    {
        if ( projection[ p ] >= columnCount )
            return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names column " + CORE::UInt32ToString( projection[ p ] ) + " of a file with " + CORE::UInt32ToString( columnCount ) + " columns" );
        if ( !projected.AddColumn( fileSchema->GetColumn( projection[ p ] ) ) )
            return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The projection names a column twice" );
    }

    CORE::CString error;
    CXlsxPackage package( m_options );
    CXlsxWorkbook workbook( m_options );
    UInt32 sheetIndex = 0;
    CORE::TDataTableStatus status = CXlsxSchemaResolver::OpenSheet( *m_source, m_options, package, workbook, sheetIndex, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );
    const SXlsxSheetInfo& sheet = workbook.GetSheets()[ sheetIndex ];
    if ( sheet.name != layout.sheetName || workbook.IsDate1904() != layout.date1904 )
        return Fail( CORE::DATATABLE_STATUS_SCHEMA_MISMATCH, "The metadata was read from the sheet \"" + layout.sheetName + "\", the options select the sheet \"" + sheet.name + "\"" );

    CXlsxColumnConverter converter( m_options, workbook.IsDate1904() );
    CXlsxBatchSink sink( m_options, converter, layout, fileSchema, CORE::CreateDataTableSchemaPtr( projected ), projection, pool,
                         batchRowCount, request.fileOrdinal, m_batches );
    CXlsxSheetParser parser( workbook, m_options, sink, package );
    status = package.ParseXmlPart( sheet.partName, parser, error );
    if ( CORE::DATATABLE_STATUS_OK == status )
        status = sink.Flush( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    if ( GUCEF_NULL != request.stats )
    {
        request.stats->bytesRead += package.GetBytesRead();
        request.stats->valueErrors += sink.GetValueErrors();
    }
    m_open = true;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableReader::ReadNextBatch( CORE::CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    if ( m_failed )
        return m_failedStatus;
    if ( !m_open )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "ReadNextBatch before OpenRowGroup" );
    if ( m_nextBatch >= m_batches.size() )
        return CORE::DATATABLE_STATUS_END;

    batch = m_batches[ m_nextBatch ];
    m_batches[ m_nextBatch ].Reset();
    ++m_nextBatch;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
