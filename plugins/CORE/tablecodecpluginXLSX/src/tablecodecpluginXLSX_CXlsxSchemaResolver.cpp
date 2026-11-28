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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "tablecodecpluginXLSX_CXlsxSchemaResolver.h"

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

/** The text of a header cell: control characters as spaces, trimmed */
static CORE::CString
HeaderName( const char* data, UInt32 length )
{GUCEF_TRACE;

    CORE::CString name;
    for ( UInt32 i = 0; i < length; ++i )
    {
        const char c = data[ i ];
        name += ( (unsigned char) c < 0x20 || 0x7F == c ) ? ' ' : c;
    }
    return name.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

/** Empty names become column_<n>, repeated names get _2, _3 and so on; the result is unique */
static void
MakeUniqueNames( CORE::CString::StringVector& names )
{GUCEF_TRACE;

    GUCEF::set< CORE::CString > taken;
    for ( size_t i = 0; i < names.size(); ++i )
        if ( !names[ i ].IsNULLOrEmpty() )
            taken.insert( names[ i ] );

    GUCEF::set< CORE::CString > used;
    for ( size_t i = 0; i < names.size(); ++i )
    {
        CORE::CString candidate = names[ i ];
        if ( candidate.IsNULLOrEmpty() )
        {
            candidate = "column_" + CORE::UInt32ToString( (UInt32) i + 1 );
            UInt32 suffix = 2;
            while ( used.find( candidate ) != used.end() || taken.find( candidate ) != taken.end() )
                candidate = "column_" + CORE::UInt32ToString( (UInt32) i + 1 ) + "_" + CORE::UInt32ToString( suffix++ );
        }
        else
        if ( used.find( candidate ) != used.end() )
        {
            UInt32 suffix = 2;
            CORE::CString attempt = names[ i ] + "_" + CORE::UInt32ToString( suffix );
            while ( used.find( attempt ) != used.end() || taken.find( attempt ) != taken.end() )
                attempt = names[ i ] + "_" + CORE::UInt32ToString( ++suffix );
            candidate = attempt;
        }
        used.insert( candidate );
        names[ i ] = candidate;
    }
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HeaderTextOf( const CORE::CString& text, TXlsxCellKind kind, bool boolValue )
{
    if ( XLSX_CELL_BOOL == kind )
        return boolValue ? "true" : "false";
    if ( XLSX_CELL_UNCACHED_FORMULA == kind )
        return CORE::CString();
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CXlsxSchemaResolver::CXlsxSchemaResolver( const SXlsxReadOptions& options )
    : CIXlsxRowSink()
    , m_options( options )
    , m_firstColumn( options.firstColumn - 1 )
    , m_windowEnd( 0 != options.lastColumn ? options.lastColumn : GUCEF_UINT32MAX )
    , m_waitingForHeader( options.hasHeader )
    , m_widthKnown( false )
    , m_width( 0 )
    , m_headerRow( 0 )
    , m_rowKind( ROW_SKIP )
    , m_rowHasValue( false )
    , m_rowFeeding( false )
    , m_rowCount( 0 )
    , m_stored()
    , m_present()
    , m_names()
    , m_inference()
    , m_fixed()
    , m_anyInference( false )
{GUCEF_TRACE;

    if ( !options.hasHeader )
    {
        // without a header the width is known when the schema or the last column gives it
        UInt32 width = 0;
        if ( !options.schemaColumns.empty() )
            width = (UInt32) options.schemaColumns.size();
        else
        if ( 0 != options.lastColumn )
            width = options.lastColumn - options.firstColumn + 1;
        if ( 0 != width )
        {
            CORE::CString ignored;
            m_width = width;
            if ( CORE::DATATABLE_STATUS_OK == InitColumns( width, GUCEF_NULL, ignored ) )
                m_widthKnown = true;
        }
    }
}

/*-------------------------------------------------------------------------*/

CXlsxSchemaResolver::~CXlsxSchemaResolver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::OpenSheet( CORE::CIOAccess& source, const SXlsxReadOptions& options, CXlsxPackage& package,
                                CXlsxWorkbook& workbook, UInt32& sheetIndex, CORE::CString& error )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = package.Open( source, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = workbook.Load( package, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    return workbook.SelectSheet( sheetIndex, error );
}

/*-------------------------------------------------------------------------*/

CXlsxSchemaResolver::SStoredCell
CXlsxSchemaResolver::Store( UInt32 index, const SXlsxCell& cell )
{GUCEF_TRACE;

    SStoredCell stored;
    stored.index = index;
    stored.kind = cell.kind;
    stored.text = CORE::CString( cell.text, cell.length );
    stored.boolValue = cell.boolValue;
    stored.numberKind = cell.numberKind;
    return stored;
}

/*-------------------------------------------------------------------------*/

SXlsxCell
CXlsxSchemaResolver::Unstore( const SStoredCell& stored )
{GUCEF_TRACE;

    SXlsxCell cell;
    cell.kind = stored.kind;
    cell.text = stored.text.C_String();
    cell.length = stored.text.ByteSize() > 0 ? stored.text.ByteSize() - 1 : 0;
    cell.boolValue = stored.boolValue;
    cell.numberKind = stored.numberKind;
    return cell;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::InitColumns( UInt32 width, const CORE::CString::StringVector* headerNames, CORE::CString& error )
{GUCEF_TRACE;

    m_width = width;
    m_fixed.assign( width, (const CORE::CDataTableColumnDef*) GUCEF_NULL );

    if ( !m_options.schemaColumns.empty() )
    {
        m_names.clear();
        for ( size_t i = 0; i < m_options.schemaColumns.size() && i < width; ++i )
        {
            m_names.push_back( m_options.schemaColumns[ i ].name );
            m_fixed[ i ] = &m_options.schemaColumns[ i ];
        }
    }
    else
    {
        m_names.clear();
        m_names.resize( width );
        if ( GUCEF_NULL != headerNames )
            for ( size_t i = 0; i < headerNames->size() && i < width; ++i )
                m_names[ i ] = (*headerNames)[ i ];
        MakeUniqueNames( m_names );
        for ( size_t o = 0; o < m_options.columnOverrides.size(); ++o )
        {
            bool found = false;
            for ( UInt32 i = 0; i < width; ++i )
            {
                if ( m_names[ i ] == m_options.columnOverrides[ o ].name )
                {
                    m_fixed[ i ] = &m_options.columnOverrides[ o ];
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                error = "The option columnTypes names a column that the sheet does not have: \"" + m_options.columnOverrides[ o ].name + "\"";
                return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
            }
        }
    }

    m_anyInference = false;
    for ( UInt32 i = 0; i < width; ++i )
        if ( GUCEF_NULL == m_fixed[ i ] )
            m_anyInference = true;
    m_inference.assign( width, CXlsxColumnInference() );
    m_present.assign( width, 0 );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CXlsxSchemaResolver::FeedCell( UInt32 index, const SXlsxCell& cell )
{GUCEF_TRACE;

    m_present[ index ] = 1;
    if ( m_rowFeeding && GUCEF_NULL == m_fixed[ index ] )
        m_inference[ index ].Feed( cell, m_options );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::OnRowBegin( UInt32 row, CORE::CString& error )
{GUCEF_TRACE;

    m_rowKind = ROW_SKIP;
    m_rowHasValue = false;
    if ( row < m_options.firstRow )
        return CORE::DATATABLE_STATUS_OK;

    if ( m_waitingForHeader )
    {
        if ( 0 != m_options.headerRow )
        {
            if ( row < m_options.headerRow )
                return CORE::DATATABLE_STATUS_OK;
            if ( row > m_options.headerRow )
            {
                error = "The header row (row " + CORE::UInt32ToString( m_options.headerRow ) + ") has no values";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
        }
        m_rowKind = ROW_HEADER;
        m_stored.clear();
        return CORE::DATATABLE_STATUS_OK;
    }

    m_rowKind = ROW_DATA;
    m_rowFeeding = 0 == m_options.inferRowCount || m_rowCount < m_options.inferRowCount;
    if ( m_widthKnown )
        m_present.assign( m_width, 0 );
    else
        m_stored.clear();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::OnCell( UInt32 row, UInt32 column, const SXlsxCell& cell, CORE::CString& error )
{GUCEF_TRACE;

    if ( ROW_SKIP == m_rowKind || column < m_firstColumn || column >= m_windowEnd )
        return CORE::DATATABLE_STATUS_OK;
    const UInt32 index = column - m_firstColumn;

    if ( ROW_HEADER == m_rowKind )
    {
        m_stored.push_back( Store( index, cell ) );
        return CORE::DATATABLE_STATUS_OK;
    }

    // a data row
    if ( !m_widthKnown )
    {
        m_stored.push_back( Store( index, cell ) );
        m_rowHasValue = true;
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( index >= m_width )
    {
        if ( !m_options.allowExtraCells )
        {
            error = "Row " + CORE::UInt32ToString( row ) + " has a value in " + CXlsxSheetParser::FormatCellReference( column, row ) +
                    ", right of the table of " + CORE::UInt32ToString( m_width ) + " columns (allowExtraCells)";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        return CORE::DATATABLE_STATUS_OK;
    }
    m_rowHasValue = true;
    FeedCell( index, cell );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::OnRowEnd( UInt32 row, CORE::CString& error )
{GUCEF_TRACE;

    if ( ROW_HEADER == m_rowKind )
    {
        if ( m_stored.empty() )
        {
            if ( 0 != m_options.headerRow )
            {
                error = "The header row (row " + CORE::UInt32ToString( row ) + ") has no values";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
            return CORE::DATATABLE_STATUS_OK;
        }

        UInt32 extent = 0;
        for ( size_t i = 0; i < m_stored.size(); ++i )
            if ( m_stored[ i ].index + 1 > extent )
                extent = m_stored[ i ].index + 1;

        UInt32 width = extent;
        if ( !m_options.schemaColumns.empty() )
        {
            if ( extent != m_options.schemaColumns.size() )
            {
                error = "The header has " + CORE::UInt32ToString( extent ) + " columns where the schema of the options has " + CORE::UInt32ToString( (UInt32) m_options.schemaColumns.size() );
                return CORE::DATATABLE_STATUS_SCHEMA_MISMATCH;
            }
        }
        else
        if ( 0 != m_options.lastColumn )
            width = m_options.lastColumn - m_options.firstColumn + 1;

        CORE::CString::StringVector names( width );
        for ( size_t i = 0; i < m_stored.size(); ++i )
        {
            const CORE::CString text = HeaderTextOf( m_stored[ i ].text, m_stored[ i ].kind, m_stored[ i ].boolValue );
            names[ m_stored[ i ].index ] = HeaderName( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
        }
        CORE::TDataTableStatus status = InitColumns( width, &names, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        m_stored.clear();
        m_headerRow = row;
        m_widthKnown = true;
        m_waitingForHeader = false;
        return CORE::DATATABLE_STATUS_OK;
    }

    if ( ROW_DATA != m_rowKind || !m_rowHasValue )
        return CORE::DATATABLE_STATUS_OK;

    if ( !m_widthKnown )
    {
        // no header and nothing that gives the width: the first row with a value does
        UInt32 extent = 0;
        for ( size_t i = 0; i < m_stored.size(); ++i )
            if ( m_stored[ i ].index + 1 > extent )
                extent = m_stored[ i ].index + 1;
        CORE::TDataTableStatus status = InitColumns( extent, GUCEF_NULL, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        m_widthKnown = true;
        for ( size_t i = 0; i < m_stored.size(); ++i )
            FeedCell( m_stored[ i ].index, Unstore( m_stored[ i ] ) );
        m_stored.clear();
    }

    if ( m_rowFeeding )
        for ( UInt32 i = 0; i < m_width; ++i )
            if ( 0 == m_present[ i ] && GUCEF_NULL == m_fixed[ i ] )
                m_inference[ i ].FeedBlank();
    ++m_rowCount;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::Finish( const CORE::CString& sheetName, UInt32 sheetIndex, bool date1904, CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error )
{GUCEF_TRACE;

    metaData.Unlink();
    if ( m_waitingForHeader )
    {
        error = "The sheet \"" + sheetName + "\" has no header row: it has no cells with a value";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( !m_widthKnown )
    {
        error = "The sheet \"" + sheetName + "\" has no rows with a value";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    CORE::CDataTableSchema schema;
    CORE::CValueList keyValues;
    UInt32 lossyColumns = 0;
    // a value that does not fit its column becomes null when the options say so, and the rows after the inferred ones can hold nulls
    const bool allNullable = 0 != m_options.inferRowCount || !m_options.failOnValueError;

    for ( UInt32 i = 0; i < m_width; ++i )
    {
        CORE::CDataTableColumnDef column;
        if ( GUCEF_NULL != m_fixed[ i ] )
        {
            column = *m_fixed[ i ];
            column.name = m_names[ i ];
            if ( m_options.schemaColumns.empty() )
                column.nullable = true;
        }
        else
        {
            column = m_inference[ i ].GetColumn( m_names[ i ], allNullable );
            const UInt64 lossyValues = m_inference[ i ].GetLossyFloatValueCount();
            if ( CORE::DATATABLE_TYPE_FLOAT64 == column.type && lossyValues > 0 )
            {
                keyValues.Set( "lossyInference." + m_names[ i ], CORE::CVariant( lossyValues ) );
                ++lossyColumns;
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "XlsxTableCodec: Column \"" << m_names[ i ] << "\" was inferred as float64 and " << lossyValues <<
                        " of its numbers have more than 15 digits, which a float64 rounds. Declare the column as decimal or text if the digits matter" );
            }
        }
        if ( !schema.AddColumn( column ) )
        {
            error = "Column \"" + m_names[ i ] + "\" cannot be a column of a table";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
    }

    keyValues.Set( CORE::CString( "lossyInference" ), CORE::CVariant( (UInt64) lossyColumns ) );
    keyValues.Set( "sheetName", CORE::CVariant( sheetName ) );
    keyValues.Set( "sheetIndex", CORE::CVariant( (UInt32) sheetIndex ) );
    keyValues.Set( "headerRow", CORE::CVariant( (UInt32) m_headerRow ) );
    keyValues.Set( "dateSystem", CORE::CVariant( CORE::CString( date1904 ? "1904" : "1900" ) ) );

    SXlsxLayout layout;
    layout.sheetIndex = sheetIndex;
    layout.sheetName = sheetName;
    layout.headerRow = m_options.hasHeader ? m_headerRow : 0;
    layout.dataStartRow = m_options.hasHeader ? m_headerRow + 1 : m_options.firstRow;
    layout.firstColumn = m_firstColumn;
    layout.width = m_width;
    layout.date1904 = date1904;

    // made as a pointer to the mutable type, whose destructor does the release, then shared as immutable
    CORE::CTSharedPtr< CXlsxFileMetaData, MT::CMutex > result( GUCEF_NEW CXlsxFileMetaData( CORE::CreateDataTableSchemaPtr( schema ), keyValues, layout, m_rowCount ) );
    metaData = CORE::CITableFileMetaDataPtr( result );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxSchemaResolver::Resolve( CORE::CIOAccess& source, const SXlsxReadOptions& options, CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error )
{GUCEF_TRACE;

    metaData.Unlink();
    CXlsxPackage package( options );
    CXlsxWorkbook workbook( options );
    UInt32 sheetIndex = 0;
    CORE::TDataTableStatus status = OpenSheet( source, options, package, workbook, sheetIndex, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CXlsxSchemaResolver resolver( options );
    CXlsxSheetParser parser( workbook, options, resolver, package );
    const SXlsxSheetInfo& sheet = workbook.GetSheets()[ sheetIndex ];
    status = package.ParseXmlPart( sheet.partName, parser, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    return resolver.Finish( sheet.name, sheetIndex, workbook.IsDate1904(), metaData, error );
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
