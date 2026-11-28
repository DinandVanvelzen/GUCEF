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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "tablecodecpluginXLSX_CXlsxSheetParser.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SXlsxCell::SXlsxCell( void )
    : kind( XLSX_CELL_NUMBER )
    , text( GUCEF_NULL )
    , length( 0 )
    , boolValue( false )
    , numberKind( XLSX_NUMBERKIND_PLAIN )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIXlsxRowSink::CIXlsxRowSink( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIXlsxRowSink::~CIXlsxRowSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxSheetParser::CXlsxSheetParser( const CXlsxWorkbook& workbook, const SXlsxReadOptions& options, CIXlsxRowSink& sink, CXlsxPackage& package )
    : CXlsxXmlCallbacks()
    , m_workbook( workbook )
    , m_options( options )
    , m_sink( sink )
    , m_package( package )
    , m_stack()
    , m_phoneticDepth( 0 )
    , m_inRow( false )
    , m_inCell( false )
    , m_inInline( false )
    , m_rowBegun( false )
    , m_rowNumber( 0 )
    , m_lastRow( 0 )
    , m_hasCellColumn( false )
    , m_nextColumn( 0 )
    , m_cellColumn( 0 )
    , m_cellStyle( 0 )
    , m_cellType()
    , m_hasValue( false )
    , m_hasFormula( false )
    , m_hasInline( false )
    , m_text()
    , m_cellCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxSheetParser::~CXlsxSheetParser()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CXlsxSheetParser::GetLastRow( void ) const
{GUCEF_TRACE;

    return m_lastRow;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxSheetParser::GetCellCount( void ) const
{GUCEF_TRACE;

    return m_cellCount;
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::OnFailed( void )
{GUCEF_TRACE;

    // nothing of the rest of the sheet is of use
    m_package.AbortParse();
}

/*-------------------------------------------------------------------------*/

bool
CXlsxSheetParser::ParseCellReference( const char* text, UInt32 length, UInt32& column, UInt32& row )
{GUCEF_TRACE;

    UInt32 i = 0;
    UInt64 col = 0;
    while ( i < length && ( ( text[ i ] >= 'A' && text[ i ] <= 'Z' ) || ( text[ i ] >= 'a' && text[ i ] <= 'z' ) ) )
    {
        const char c = text[ i ] >= 'a' ? (char) ( text[ i ] - 'a' + 'A' ) : text[ i ];
        col = col * 26 + (UInt64) ( c - 'A' + 1 );
        if ( col > 1000000 )
            return false;
        ++i;
    }
    if ( 0 == i || i == length )
        return false;
    UInt64 rowNumber = 0;
    for ( ; i < length; ++i )
    {
        if ( text[ i ] < '0' || text[ i ] > '9' )
            return false;
        rowNumber = rowNumber * 10 + (UInt64) ( text[ i ] - '0' );
        if ( rowNumber > GUCEF_UINT32MAX )
            return false;
    }
    if ( 0 == rowNumber )
        return false;
    column = (UInt32) ( col - 1 );
    row = (UInt32) rowNumber;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxSheetParser::FormatCellReference( UInt32 column, UInt32 row )
{GUCEF_TRACE;

    char letters[ 12 ];
    UInt32 count = 0;
    UInt32 value = column + 1;
    while ( value > 0 && count < 10 )
    {
        --value;
        letters[ count++ ] = (char) ( 'A' + value % 26 );
        value /= 26;
    }
    CORE::CString text;
    while ( count > 0 )
    {
        char one[ 2 ] = { letters[ --count ], '\0' };
        text += one;
    }
    return text + CORE::UInt32ToString( row );
}

/*-------------------------------------------------------------------------*/

bool
CXlsxSheetParser::Check( CORE::TDataTableStatus status, const CORE::CString& error )
{GUCEF_TRACE;

    if ( CORE::DATATABLE_STATUS_OK == status )
        return true;
    Fail( status, error );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::BeginRow( void )
{GUCEF_TRACE;

    if ( m_rowBegun || IsFailed() )
        return;
    m_rowBegun = true;
    if ( m_rowNumber > m_options.maxRows )
    {
        Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The sheet has a row " + CORE::UInt32ToString( m_rowNumber ) + ", beyond the limit of " +
              CORE::UInt32ToString( m_options.maxRows ) + " rows (maxRows)" );
        return;
    }
    CORE::CString error;
    Check( m_sink.OnRowBegin( m_rowNumber, error ), error );
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::OnNodeBegin( const CORE::CVariant& nodeId, int nodeType )
{GUCEF_TRACE;

    UInt32 length = 0;
    const char* name = LocalName( nodeId, length );
    UInt8 code = ELEMENT_OTHER;

    if ( IsName( name, length, "sheetData" ) )
        code = ELEMENT_SHEETDATA;
    else
    if ( IsName( name, length, "row" ) )
    {
        code = ELEMENT_ROW;
        m_inRow = true;
        m_rowBegun = false;
        m_rowNumber = m_lastRow + 1;
        m_nextColumn = 0;
    }
    else
    if ( IsName( name, length, "c" ) && m_inRow )
    {
        code = ELEMENT_CELL;
        m_inCell = true;
        m_cellColumn = m_nextColumn;
        m_cellStyle = 0;
        m_cellType.Clear();
        m_hasValue = false;
        m_hasFormula = false;
        m_hasInline = false;
        m_text.clear();
    }
    else
    if ( IsName( name, length, "v" ) && m_inCell )
        code = ELEMENT_VALUE;
    else
    if ( IsName( name, length, "f" ) && m_inCell )
    {
        code = ELEMENT_FORMULA;
        m_hasFormula = true;
    }
    else
    if ( IsName( name, length, "is" ) && m_inCell )
    {
        code = ELEMENT_INLINE;
        m_inInline = true;
        m_hasInline = true;
    }
    else
    if ( IsName( name, length, "t" ) && m_inInline )
        code = ELEMENT_TEXT;
    else
    if ( IsName( name, length, "rPh" ) && m_inInline )
    {
        code = ELEMENT_PHONETIC;
        ++m_phoneticDepth;
    }
    m_stack.push_back( code );
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue )
{GUCEF_TRACE;

    if ( IsFailed() || m_stack.empty() )
        return;
    const UInt8 element = m_stack.back();
    if ( ELEMENT_ROW != element && ELEMENT_CELL != element )
        return;

    UInt32 length = 0;
    const char* attribute = LocalName( attributeId, length );
    CORE::CString holder;
    const char* text = GUCEF_NULL;
    UInt32 textLength = 0;
    TextOf( attributeValue, holder, text, textLength );

    if ( ELEMENT_ROW == element && IsName( attribute, length, "r" ) )
    {
        UInt64 number = 0;
        for ( UInt32 i = 0; i < textLength; ++i )
        {
            if ( text[ i ] < '0' || text[ i ] > '9' || number > GUCEF_UINT32MAX )
            {
                number = 0;
                break;
            }
            number = number * 10 + (UInt64) ( text[ i ] - '0' );
        }
        if ( 0 == number || number > GUCEF_UINT32MAX )
        {
            Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A row has a row number that is no number: \"" + CORE::CString( text, textLength ) + "\"" );
            return;
        }
        if ( number <= m_lastRow )
        {
            Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The rows are not in order: row " + CORE::UInt32ToString( (UInt32) number ) + " follows row " + CORE::UInt32ToString( m_lastRow ) );
            return;
        }
        m_rowNumber = (UInt32) number;
    }
    else
    if ( ELEMENT_CELL == element )
    {
        if ( IsName( attribute, length, "r" ) )
        {
            UInt32 column = 0, row = 0;
            if ( !ParseCellReference( text, textLength, column, row ) )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A cell has a reference that is not one: \"" + CORE::CString( text, textLength ) + "\"" );
                return;
            }
            if ( row != m_rowNumber )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The cell " + CORE::CString( text, textLength ) + " is in row " + CORE::UInt32ToString( m_rowNumber ) );
                return;
            }
            if ( column < m_nextColumn )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The cells of row " + CORE::UInt32ToString( row ) + " are not in order: " + CORE::CString( text, textLength ) + " is not right of the cell before it" );
                return;
            }
            m_cellColumn = column;
        }
        else
        if ( IsName( attribute, length, "t" ) )
            m_cellType = CORE::CString( text, textLength );
        else
        if ( IsName( attribute, length, "s" ) )
        {
            UInt64 number = 0;
            for ( UInt32 i = 0; i < textLength; ++i )
            {
                if ( text[ i ] < '0' || text[ i ] > '9' || number > GUCEF_UINT32MAX )
                {
                    Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A cell has a style that is no number: \"" + CORE::CString( text, textLength ) + "\"" );
                    return;
                }
                number = number * 10 + (UInt64) ( text[ i ] - '0' );
            }
            m_cellStyle = (UInt32) number;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue )
{GUCEF_TRACE;

    if ( IsFailed() || m_stack.empty() )
        return;
    const UInt8 element = m_stack.back();
    if ( ELEMENT_VALUE != element && !( ELEMENT_TEXT == element && 0 == m_phoneticDepth ) )
        return;

    CORE::CString holder;
    const char* text = GUCEF_NULL;
    UInt32 textLength = 0;
    TextOf( nodeValue, holder, text, textLength );
    if ( (UInt64) m_text.size() + textLength > m_options.maxCellBytes )
    {
        Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The text of a cell is longer than " + CORE::UInt32ToString( m_options.maxCellBytes ) +
              " bytes (maxCellBytes), in row " + CORE::UInt32ToString( m_rowNumber ) );
        return;
    }
    m_text.insert( m_text.end(), text, text + textLength );
    if ( ELEMENT_VALUE == element )
        m_hasValue = true;
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::EndCell( void )
{GUCEF_TRACE;

    m_inCell = false;
    const UInt32 column = m_cellColumn;
    m_nextColumn = column + 1;
    if ( IsFailed() )
        return;
    BeginRow();
    if ( IsFailed() )
        return;

    const char* text = m_text.empty() ? "" : &m_text[ 0 ];
    const UInt32 textLength = (UInt32) m_text.size();
    const CORE::CString where = "cell " + FormatCellReference( column, m_rowNumber );

    SXlsxCell cell;
    CORE::CString decoded;
    bool blank = false;

    if ( m_cellType.IsNULLOrEmpty() || m_cellType.Equals( "n", true ) )
    {
        if ( m_hasValue && textLength > 0 )
        {
            cell.kind = XLSX_CELL_NUMBER;
            cell.text = text;
            cell.length = textLength;
            cell.numberKind = m_workbook.GetStyleKind( m_cellStyle );
        }
        else
        if ( m_hasFormula )
            cell.kind = XLSX_CELL_UNCACHED_FORMULA;
        else
            blank = true;
    }
    else
    if ( m_cellType.Equals( "s", true ) )
    {
        if ( !m_hasValue )
            blank = true;
        else
        {
            UInt64 index = 0;
            bool valid = textLength > 0;
            for ( UInt32 i = 0; i < textLength && valid; ++i )
            {
                valid = text[ i ] >= '0' && text[ i ] <= '9' && index <= GUCEF_UINT32MAX;
                if ( valid )
                    index = index * 10 + (UInt64) ( text[ i ] - '0' );
            }
            if ( !valid || index > GUCEF_UINT32MAX )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The " + where + " has a shared string index that is no number: \"" + CORE::CString( text, textLength ) + "\"" );
                return;
            }
            cell.kind = XLSX_CELL_STRING;
            if ( !m_workbook.GetSharedString( (UInt32) index, cell.text, cell.length ) )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The " + where + " has shared string " + CORE::UInt64ToString( index ) + " where the workbook has " +
                      CORE::UInt32ToString( m_workbook.GetSharedStringCount() ) );
                return;
            }
            if ( 0 == cell.length )
                blank = true;
        }
    }
    else
    if ( m_cellType.Equals( "str", true ) || m_cellType.Equals( "inlineStr", true ) )
    {
        const bool inlineText = m_cellType.Equals( "inlineStr", true );
        if ( inlineText ? !m_hasInline : !m_hasValue )
        {
            if ( !inlineText && m_hasFormula )
                cell.kind = XLSX_CELL_UNCACHED_FORMULA;
            else
                blank = true;
        }
        else
        {
            CXlsxWorkbook::DecodeEscapes( text, textLength, decoded );
            cell.kind = XLSX_CELL_STRING;
            cell.text = decoded.C_String();
            cell.length = decoded.ByteSize() > 0 ? decoded.ByteSize() - 1 : 0;
            if ( 0 == cell.length )
                blank = true;
        }
    }
    else
    if ( m_cellType.Equals( "b", true ) )
    {
        if ( !m_hasValue )
            blank = true;
        else
        {
            cell.kind = XLSX_CELL_BOOL;
            if ( 1 == textLength && '1' == text[ 0 ] )
                cell.boolValue = true;
            else
            if ( 1 == textLength && '0' == text[ 0 ] )
                cell.boolValue = false;
            else
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The " + where + " is a boolean with the value \"" + CORE::CString( text, textLength ) + "\" instead of 0 or 1" );
                return;
            }
        }
    }
    else
    if ( m_cellType.Equals( "d", true ) )
    {
        if ( !m_hasValue || 0 == textLength )
            blank = true;
        else
        {
            cell.kind = XLSX_CELL_ISODATE;
            cell.text = text;
            cell.length = textLength;
        }
    }
    else
    if ( m_cellType.Equals( "e", true ) )
    {
        cell.kind = XLSX_CELL_ERROR;
        cell.text = text;
        cell.length = textLength;
    }
    else
    {
        Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The " + where + " has the unknown cell type \"" + m_cellType + "\"" );
        return;
    }

    if ( blank )
        return;

    if ( column >= m_options.maxColumns )
    {
        Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The sheet has a cell in column " + CORE::UInt32ToString( column + 1 ) + ", beyond the limit of " +
              CORE::UInt32ToString( m_options.maxColumns ) + " columns (maxColumns)" );
        return;
    }
    if ( ++m_cellCount > m_options.maxCells )
    {
        Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The sheet has more than " + CORE::UInt64ToString( m_options.maxCells ) + " cells with a value (maxCells)" );
        return;
    }
    if ( cell.length > m_options.maxCellBytes )
    {
        Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The text of the " + where + " is longer than " + CORE::UInt32ToString( m_options.maxCellBytes ) + " bytes (maxCellBytes)" );
        return;
    }

    CORE::CString error;
    Check( m_sink.OnCell( m_rowNumber, column, cell, error ), error );
}

/*-------------------------------------------------------------------------*/

void
CXlsxSheetParser::OnNodeEnd( const CORE::CVariant& nodeId )
{GUCEF_TRACE;

    if ( m_stack.empty() )
        return;
    const UInt8 element = m_stack.back();
    m_stack.pop_back();

    switch ( element )
    {
        case ELEMENT_CELL:
        {
            EndCell();
            break;
        }
        case ELEMENT_ROW:
        {
            if ( !IsFailed() )
            {
                BeginRow();
                CORE::CString error;
                if ( !IsFailed() )
                    Check( m_sink.OnRowEnd( m_rowNumber, error ), error );
                m_lastRow = m_rowNumber;
            }
            m_inRow = false;
            break;
        }
        case ELEMENT_INLINE:
        {
            m_inInline = false;
            break;
        }
        case ELEMENT_PHONETIC:
        {
            if ( m_phoneticDepth > 0 )
                --m_phoneticDepth;
            break;
        }
        default:
            break;
    }
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
