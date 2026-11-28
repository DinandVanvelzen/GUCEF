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

#include "tablecodecpluginXLSX_CXlsxReadOptions.h"

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

static inline UInt32
TextLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
OptionText( const CORE::CVariant& value )
{
    return value.AsString();
}

/*-------------------------------------------------------------------------*/

static bool
ParseBoolOption( const char* key, const CORE::CString& text, bool& value, CORE::CString& error )
{GUCEF_TRACE;

    if ( text.Equals( "true", false ) || text.Equals( "1", false ) || text.Equals( "yes", false ) )
        value = true;
    else
    if ( text.Equals( "false", false ) || text.Equals( "0", false ) || text.Equals( "no", false ) )
        value = false;
    else
    {
        error = CORE::CString( "xlsx option \"" ) + key + "\" is not a boolean: \"" + text + "\"";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ParseUInt64Option( const char* key, const CORE::CString& text, UInt64 maximum, UInt64& value, CORE::CString& error )
{GUCEF_TRACE;

    const UInt32 length = TextLength( text );
    if ( 0 == length || length > 20 )
    {
        error = CORE::CString( "xlsx option \"" ) + key + "\" is not a count: \"" + text + "\"";
        return false;
    }
    UInt64 result = 0;
    for ( UInt32 i = 0; i < length; ++i )
    {
        const char c = text.C_String()[ i ];
        if ( c < '0' || c > '9' )
        {
            error = CORE::CString( "xlsx option \"" ) + key + "\" is not a count: \"" + text + "\"";
            return false;
        }
        const UInt64 digit = (UInt64) ( c - '0' );
        if ( result > ( GUCEF_UINT64MAX - digit ) / 10 )
        {
            error = CORE::CString( "xlsx option \"" ) + key + "\" is too large: \"" + text + "\"";
            return false;
        }
        result = result * 10 + digit;
    }
    if ( result > maximum )
    {
        error = CORE::CString( "xlsx option \"" ) + key + "\" is out of range: \"" + text + "\"";
        return false;
    }
    value = result;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Letters ("C", "AB") or a number: the 1 based column */
static bool
ParseColumnOption( const char* key, const CORE::CString& text, UInt32& value, CORE::CString& error )
{GUCEF_TRACE;

    const UInt32 length = TextLength( text );
    if ( 0 == length )
    {
        error = CORE::CString( "xlsx option \"" ) + key + "\" is empty";
        return false;
    }
    const char first = text.C_String()[ 0 ];
    if ( ( first >= 'A' && first <= 'Z' ) || ( first >= 'a' && first <= 'z' ) )
    {
        UInt32 result = 0;
        for ( UInt32 i = 0; i < length; ++i )
        {
            char c = text.C_String()[ i ];
            if ( c >= 'a' && c <= 'z' )
                c = (char) ( c - 'a' + 'A' );
            if ( c < 'A' || c > 'Z' || i >= 3 )
            {
                error = CORE::CString( "xlsx option \"" ) + key + "\" is not a column: \"" + text + "\"";
                return false;
            }
            result = result * 26 + (UInt32) ( c - 'A' + 1 );
        }
        value = result;
        return true;
    }
    UInt64 number = 0;
    if ( !ParseUInt64Option( key, text, 16384, number, error ) )
        return false;
    value = (UInt32) number;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ParseRoundingMode( const CORE::CString& text, CORE::TRoundingMode& mode )
{GUCEF_TRACE;

    if ( text.Equals( "halfEven", false ) ) mode = CORE::ROUNDINGMODE_HALF_EVEN;
    else if ( text.Equals( "halfUp", false ) ) mode = CORE::ROUNDINGMODE_HALF_UP;
    else if ( text.Equals( "halfDown", false ) ) mode = CORE::ROUNDINGMODE_HALF_DOWN;
    else if ( text.Equals( "towardZero", false ) ) mode = CORE::ROUNDINGMODE_TOWARD_ZERO;
    else if ( text.Equals( "awayFromZero", false ) ) mode = CORE::ROUNDINGMODE_AWAY_FROM_ZERO;
    else if ( text.Equals( "floor", false ) ) mode = CORE::ROUNDINGMODE_FLOOR;
    else if ( text.Equals( "ceiling", false ) ) mode = CORE::ROUNDINGMODE_CEILING;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** typeText[!] */
static bool
ParseColumnSpec( const CORE::CString& name, const CORE::CString& spec, bool allowNotNull, CORE::CDataTableColumnDef& result, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString typeText = spec.Trim( true ).Trim( false );
    bool notNull = false;
    const UInt32 typeLength = TextLength( typeText );
    if ( typeLength > 0 && '!' == typeText.C_String()[ typeLength - 1 ] )
    {
        if ( !allowNotNull )
        {
            error = "Column \"" + name + "\": a type override cannot be marked not null";
            return false;
        }
        notNull = true;
        typeText = CORE::CString( typeText.C_String(), typeLength - 1 );
    }

    result = CORE::CDataTableColumnDef( name, CORE::DATATABLE_TYPE_UTF8, !notNull );
    if ( !result.SetTypeFromText( typeText.Trim( true ).Trim( false ) ) )
    {
        error = "Column \"" + name + "\": unknown type \"" + typeText + "\"";
        return false;
    }
    if ( CORE::DATATABLE_TYPE_UNSUPPORTED == result.type || CORE::DATATABLE_TYPE_BINARY == result.type )
    {
        error = "Column \"" + name + "\": the type cannot be read from a worksheet";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

SXlsxReadOptions::SXlsxReadOptions( void )
    : sheetName()
    , hasSheetName( false )
    , sheetIndex( 0 )
    , hasHeader( true )
    , headerRow( 0 )
    , firstRow( 1 )
    , firstColumn( 1 )
    , lastColumn( 0 )
    , allowExtraCells( false )
    , schemaColumns()
    , columnOverrides()
    , inferRowCount( 0 )
    , failOnValueError( true )
    , roundDecimals( false )
    , decimalRounding( CORE::ROUNDINGMODE_HALF_EVEN )
    , nullTokens()
    , maxFileBytes( 256ULL * 1024 * 1024 )
    , maxEntries( 4096 )
    , maxUncompressedBytes( 512ULL * 1024 * 1024 )
    , maxExpansionRatio( 1000 )
    , maxSheets( 1024 )
    , maxRows( 1048576 )
    , maxColumns( 16384 )
    , maxCells( 20000000ULL )
    , maxCellBytes( 65536 )
    , maxSharedStringBytes( 128ULL * 1024 * 1024 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
SXlsxReadOptions::Parse( const CORE::CValueList& options, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString text;
    UInt64 number = 0;

    if ( options.HasKey( "sheetName" ) )
    {
        sheetName = OptionText( options.GetValueAlways( "sheetName" ) );
        hasSheetName = true;
        if ( sheetName.IsNULLOrEmpty() )
        {
            error = "xlsx option \"sheetName\" is empty";
            return false;
        }
    }
    if ( options.HasKey( "sheetIndex" ) )
    {
        if ( hasSheetName )
        {
            error = "xlsx options \"sheetName\" and \"sheetIndex\" select the sheet both: give one";
            return false;
        }
        if ( !ParseUInt64Option( "sheetIndex", OptionText( options.GetValueAlways( "sheetIndex" ) ), GUCEF_UINT32MAX - 1, number, error ) )
            return false;
        sheetIndex = (UInt32) number;
    }
    if ( options.HasKey( "hasHeader" ) )
        if ( !ParseBoolOption( "hasHeader", OptionText( options.GetValueAlways( "hasHeader" ) ), hasHeader, error ) )
            return false;
    if ( options.HasKey( "allowExtraCells" ) )
        if ( !ParseBoolOption( "allowExtraCells", OptionText( options.GetValueAlways( "allowExtraCells" ) ), allowExtraCells, error ) )
            return false;

    if ( options.HasKey( "headerRow" ) )
    {
        if ( !hasHeader )
        {
            error = "xlsx option \"headerRow\" needs a header: hasHeader is false";
            return false;
        }
        if ( !ParseUInt64Option( "headerRow", OptionText( options.GetValueAlways( "headerRow" ) ), 1048576, number, error ) || 0 == number )
        {
            if ( error.IsNULLOrEmpty() )
                error = "xlsx option \"headerRow\" is a row number from 1";
            return false;
        }
        headerRow = (UInt32) number;
    }
    if ( options.HasKey( "firstRow" ) )
    {
        if ( !ParseUInt64Option( "firstRow", OptionText( options.GetValueAlways( "firstRow" ) ), 1048576, number, error ) || 0 == number )
        {
            if ( error.IsNULLOrEmpty() )
                error = "xlsx option \"firstRow\" is a row number from 1";
            return false;
        }
        firstRow = (UInt32) number;
    }
    if ( options.HasKey( "firstColumn" ) )
        if ( !ParseColumnOption( "firstColumn", OptionText( options.GetValueAlways( "firstColumn" ) ), firstColumn, error ) )
            return false;
    if ( options.HasKey( "lastColumn" ) )
        if ( !ParseColumnOption( "lastColumn", OptionText( options.GetValueAlways( "lastColumn" ) ), lastColumn, error ) )
            return false;
    if ( 0 != lastColumn && lastColumn < firstColumn )
    {
        error = "xlsx option \"lastColumn\" is left of \"firstColumn\"";
        return false;
    }
    if ( 0 != headerRow && headerRow < firstRow )
    {
        error = "xlsx option \"headerRow\" is above \"firstRow\"";
        return false;
    }

    if ( options.HasKey( "inferRowCount" ) )
        if ( !ParseUInt64Option( "inferRowCount", OptionText( options.GetValueAlways( "inferRowCount" ) ), GUCEF_UINT64MAX, inferRowCount, error ) )
            return false;

    if ( options.HasKey( "onValueError" ) )
    {
        text = OptionText( options.GetValueAlways( "onValueError" ) );
        if ( text.Equals( "fail", false ) ) failOnValueError = true;
        else if ( text.Equals( "null", false ) ) failOnValueError = false;
        else
        {
            error = "xlsx option \"onValueError\" is fail or null, not \"" + text + "\"";
            return false;
        }
    }
    if ( options.HasKey( "decimalRounding" ) )
    {
        text = OptionText( options.GetValueAlways( "decimalRounding" ) );
        if ( text.Equals( "none", false ) )
            roundDecimals = false;
        else
        if ( ParseRoundingMode( text, decimalRounding ) )
            roundDecimals = true;
        else
        {
            error = "xlsx option \"decimalRounding\" is none, halfEven, halfUp, halfDown, towardZero, awayFromZero, floor or ceiling, not \"" + text + "\"";
            return false;
        }
    }

    struct SLimit32 { const char* key; UInt32* target; UInt64 minimum; UInt64 maximum; };
    const SLimit32 limits32[] = { { "maxEntries", &maxEntries, 1, GUCEF_UINT32MAX - 1 }, { "maxSheets", &maxSheets, 1, 65535 },
                                  { "maxRows", &maxRows, 1, 1048576 }, { "maxColumns", &maxColumns, 1, 16384 },
                                  { "maxCellBytes", &maxCellBytes, 1, GUCEF_UINT32MAX - 1 }, { "maxExpansionRatio", &maxExpansionRatio, 0, GUCEF_UINT32MAX } };
    for ( size_t i = 0; i < sizeof( limits32 ) / sizeof( limits32[ 0 ] ); ++i )
    {
        if ( options.HasKey( limits32[ i ].key ) )
        {
            if ( !ParseUInt64Option( limits32[ i ].key, OptionText( options.GetValueAlways( limits32[ i ].key ) ), GUCEF_UINT64MAX, number, error ) )
                return false;
            if ( number < limits32[ i ].minimum || number > limits32[ i ].maximum )
            {
                error = CORE::CString( "xlsx option \"" ) + limits32[ i ].key + "\" is out of range";
                return false;
            }
            *limits32[ i ].target = (UInt32) number;
        }
    }
    struct SLimit64 { const char* key; UInt64* target; };
    const SLimit64 limits64[] = { { "maxFileBytes", &maxFileBytes }, { "maxUncompressedBytes", &maxUncompressedBytes },
                                  { "maxCells", &maxCells }, { "maxSharedStringBytes", &maxSharedStringBytes } };
    for ( size_t i = 0; i < sizeof( limits64 ) / sizeof( limits64[ 0 ] ); ++i )
    {
        if ( options.HasKey( limits64[ i ].key ) )
        {
            if ( !ParseUInt64Option( limits64[ i ].key, OptionText( options.GetValueAlways( limits64[ i ].key ) ), GUCEF_UINT64MAX, number, error ) )
                return false;
            if ( 0 == number )
            {
                error = CORE::CString( "xlsx option \"" ) + limits64[ i ].key + "\" is out of range";
                return false;
            }
            *limits64[ i ].target = number;
        }
    }

    CORE::CValueList::TVariantVector values = options.GetValueVectorAlways( "nullTokens" );
    for ( size_t i = 0; i < values.size(); ++i )
        nullTokens.push_back( OptionText( values[ i ] ) );

    values = options.GetValueVectorAlways( "schema" );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        // name|typeText[!]: the name is what is in front of the last '|'
        const CORE::CString entry = OptionText( values[ i ] );
        const char* chars = entry.C_String();
        const UInt32 length = TextLength( entry );
        Int32 bar = -1;
        for ( UInt32 n = 0; n < length; ++n )
            if ( '|' == chars[ n ] )
                bar = (Int32) n;
        if ( bar < 0 )
        {
            error = "xlsx option \"schema\" entries are name|type, not \"" + entry + "\"";
            return false;
        }
        CORE::CDataTableColumnDef column;
        if ( !ParseColumnSpec( CORE::CString( chars, (UInt32) bar ), CORE::CString( chars + bar + 1, length - (UInt32) bar - 1 ), true, column, error ) )
            return false;
        schemaColumns.push_back( column );
    }

    CORE::CValueList::TStringVector overrideKeys = options.GetKeysWithWildcardKeyMatch( "columnTypes.*" );
    static const UInt32 prefixLength = 12;     // "columnTypes."
    for ( size_t i = 0; i < overrideKeys.size(); ++i )
    {
        const CORE::CString& key = overrideKeys[ i ];
        if ( TextLength( key ) <= prefixLength )
            continue;
        CORE::CDataTableColumnDef column;
        if ( !ParseColumnSpec( CORE::CString( key.C_String() + prefixLength, TextLength( key ) - prefixLength ),
                               OptionText( options.GetValueAlways( key ) ), false, column, error ) )
            return false;
        columnOverrides.push_back( column );
    }

    if ( !schemaColumns.empty() && !columnOverrides.empty() )
    {
        error = "xlsx options \"schema\" and \"columnTypes.*\" both fix the types: give one";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
SXlsxReadOptions::IsNullToken( const char* text, UInt32 length ) const
{GUCEF_TRACE;

    for ( size_t i = 0; i < nullTokens.size(); ++i )
        if ( TextLength( nullTokens[ i ] ) == length && 0 == memcmp( nullTokens[ i ].C_String(), text, length ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
SXlsxReadOptions::AddOption( CORE::CValueList& options, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    options.SetAllowMultipleValues( true );
    options.Set( key, CORE::CVariant( value ) );
}

/*-------------------------------------------------------------------------*/

void
SXlsxReadOptions::SetSchemaOption( CORE::CValueList& options, const CORE::CDataTableSchema& schema )
{GUCEF_TRACE;

    options.Delete( "schema" );
    for ( UInt32 i = 0; i < schema.GetColumnCount(); ++i )
    {
        const CORE::CDataTableColumnDef& column = schema.GetColumn( i );
        AddOption( options, "schema", column.name + "|" + column.GetTypeText() + ( column.nullable ? "" : "!" ) );
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
