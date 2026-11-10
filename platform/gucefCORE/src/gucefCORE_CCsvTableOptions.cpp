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

#include "gucefCORE_CCsvTableOptions.h"

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

/** The text of the value; text of a variant that does not hold text is its string form */
static CString
OptionText( const CVariant& value )
{
    return value.AsString();
}

/*-------------------------------------------------------------------------*/

static bool
ParseBoolOption( const CString& key, const CString& text, bool& value, CString& error )
{
    if ( CCsvTableText::ParseBool( text.C_String(), TextLength( text ), true, value ) )
        return true;
    error = "CSV option \"" + key + "\" is not a boolean: \"" + text + "\"";
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
ParseUInt64Option( const CString& key, const CString& text, UInt64& value, CString& error )
{
    const UInt32 length = TextLength( text );
    const char* chars = text.C_String();
    if ( 0 == length || length > 20 )
    {
        error = "CSV option \"" + key + "\" is not a count: \"" + text + "\"";
        return false;
    }
    UInt64 result = 0;
    for ( UInt32 i = 0; i < length; ++i )
    {
        if ( chars[ i ] < '0' || chars[ i ] > '9' )
        {
            error = "CSV option \"" + key + "\" is not a count: \"" + text + "\"";
            return false;
        }
        const UInt64 digit = (UInt64) ( chars[ i ] - '0' );
        if ( result > ( GUCEF_UINT64MAX - digit ) / 10 )
        {
            error = "CSV option \"" + key + "\" is too large: \"" + text + "\"";
            return false;
        }
        result = result * 10 + digit;
    }
    value = result;
    return true;
}

/*-------------------------------------------------------------------------*/

/** One byte: the byte itself, or the names tab, comma, semicolon, pipe, space and the escape \t */
static bool
ParseSeparatorOption( const CString& key, const CString& text, bool allowNone, char& value, CString& error )
{
    const UInt32 length = TextLength( text );
    const char* chars = text.C_String();
    if ( 0 == length )
    {
        if ( allowNone )
        {
            value = '\0';
            return true;
        }
        error = "CSV option \"" + key + "\" is empty";
        return false;
    }

    if ( 1 == length )
        value = chars[ 0 ];
    else
    if ( text == "\\t" || text.Equals( "tab", false ) ) value = '\t';
    else if ( text.Equals( "comma", false ) ) value = ',';
    else if ( text.Equals( "semicolon", false ) ) value = ';';
    else if ( text.Equals( "pipe", false ) ) value = '|';
    else if ( text.Equals( "space", false ) ) value = ' ';
    else if ( allowNone && text.Equals( "none", false ) ) value = '\0';
    else
    {
        error = "CSV option \"" + key + "\" is not a single byte: \"" + text + "\"";
        return false;
    }

    if ( '\0' != value && ( (unsigned char) value >= 0x80 || '\r' == value || '\n' == value ) )
    {
        error = "CSV option \"" + key + "\" is not a usable ASCII character";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** typeText[!][:format], the ! makes the column not nullable */
static bool
ParseColumnSpec( const CString& name, const CString& spec, bool allowNotNull, SCsvColumnSpec& result, CString& error )
{
    const UInt32 length = TextLength( spec );
    const char* chars = spec.C_String();

    UInt32 colon = length;
    for ( UInt32 i = 0; i < length; ++i )
    {
        if ( ':' == chars[ i ] )
        {
            colon = i;
            break;
        }
    }

    CString typeText( chars, colon );
    CString formatText;
    if ( colon < length )
        formatText = CString( chars + colon + 1, length - colon - 1 );

    bool notNull = false;
    UInt32 typeLength = TextLength( typeText );
    if ( typeLength > 0 && '!' == typeText.C_String()[ typeLength - 1 ] )
    {
        if ( !allowNotNull )
        {
            error = "Column \"" + name + "\": a type override cannot be marked not null";
            return false;
        }
        notNull = true;
        typeText = CString( typeText.C_String(), typeLength - 1 );
    }

    result = SCsvColumnSpec();
    result.column = CDataTableColumnDef( name, DATATABLE_TYPE_UTF8, !notNull );
    if ( !result.column.SetTypeFromText( typeText.Trim( true ).Trim( false ) ) )
    {
        error = "Column \"" + name + "\": unknown type \"" + typeText + "\"";
        return false;
    }
    if ( DATATABLE_TYPE_UNSUPPORTED == result.column.type )
    {
        error = "Column \"" + name + "\": the type cannot be used";
        return false;
    }

    if ( colon < length )
    {
        if ( DATATABLE_TYPE_DATE != result.column.type )
        {
            error = "Column \"" + name + "\": only date columns take a format";
            return false;
        }
        if ( !CCsvTableText::CompileDateFormat( formatText, result.dateFormat, error ) )
            return false;
        result.hasDateFormat = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

SCsvColumnSpec::SCsvColumnSpec( void )
    : column()
    , dateFormat()
    , hasDateFormat( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SCsvReadOptions::SCsvReadOptions( void )
    : delimiter( ',' )
    , quote( '"' )
    , hasHeader( true )
    , skipLeadingLines( 0 )
    , skipLinesMatching()
    , nullTokens()
    , trimWhitespace( true )
    , numberFormat()
    , dateFormats()
    , schemaColumns()
    , columnOverrides()
    , inferRowCount( 0 )
    , failOnValueError( true )
    , allowMissingFields( false )
    , allowExtraFields( false )
    , maxFieldBytes( 16 * 1024 * 1024 )
    , maxRecordBytes( 64 * 1024 * 1024 )
    , maxColumns( 10000 )
    , batchByteLimit( 128 * 1024 * 1024 )
{GUCEF_TRACE;

    nullTokens.push_back( CString() );
}

/*-------------------------------------------------------------------------*/

bool
SCsvReadOptions::HasOnlyEmptyNullToken( void ) const
{GUCEF_TRACE;

    return 1 == nullTokens.size() && 0 == TextLength( nullTokens[ 0 ] );
}

/*-------------------------------------------------------------------------*/

bool
SCsvReadOptions::IsNullToken( const char* text, UInt32 length ) const
{GUCEF_TRACE;

    for ( size_t i = 0; i < nullTokens.size(); ++i )
    {
        if ( TextLength( nullTokens[ i ] ) == length && 0 == memcmp( nullTokens[ i ].C_String(), text, length ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
SCsvReadOptions::Parse( const CValueList& options, CString& error )
{GUCEF_TRACE;

    CString text;

    if ( options.HasKey( "delimiter" ) )
        if ( !ParseSeparatorOption( "delimiter", OptionText( options.GetValueAlways( "delimiter" ) ), false, delimiter, error ) )
            return false;

    if ( options.HasKey( "quote" ) )
        if ( !ParseSeparatorOption( "quote", OptionText( options.GetValueAlways( "quote" ) ), true, quote, error ) )
            return false;

    if ( '\0' != quote && quote == delimiter )
    {
        error = "CSV options \"delimiter\" and \"quote\" are the same character";
        return false;
    }

    if ( options.HasKey( "hasHeader" ) )
        if ( !ParseBoolOption( "hasHeader", OptionText( options.GetValueAlways( "hasHeader" ) ), hasHeader, error ) )
            return false;

    if ( options.HasKey( "trimWhitespace" ) )
        if ( !ParseBoolOption( "trimWhitespace", OptionText( options.GetValueAlways( "trimWhitespace" ) ), trimWhitespace, error ) )
            return false;

    if ( options.HasKey( "negativeParentheses" ) )
        if ( !ParseBoolOption( "negativeParentheses", OptionText( options.GetValueAlways( "negativeParentheses" ) ), numberFormat.negativeParentheses, error ) )
            return false;

    if ( options.HasKey( "allowMissingFields" ) )
        if ( !ParseBoolOption( "allowMissingFields", OptionText( options.GetValueAlways( "allowMissingFields" ) ), allowMissingFields, error ) )
            return false;

    if ( options.HasKey( "allowExtraFields" ) )
        if ( !ParseBoolOption( "allowExtraFields", OptionText( options.GetValueAlways( "allowExtraFields" ) ), allowExtraFields, error ) )
            return false;

    UInt64 count = 0;
    if ( options.HasKey( "skipLeadingLines" ) )
    {
        if ( !ParseUInt64Option( "skipLeadingLines", OptionText( options.GetValueAlways( "skipLeadingLines" ) ), count, error ) || count > GUCEF_UINT32MAX )
        {
            if ( error.IsNULLOrEmpty() ) error = "CSV option \"skipLeadingLines\" is too large";
            return false;
        }
        skipLeadingLines = (UInt32) count;
    }

    if ( options.HasKey( "inferRowCount" ) )
        if ( !ParseUInt64Option( "inferRowCount", OptionText( options.GetValueAlways( "inferRowCount" ) ), inferRowCount, error ) )
            return false;

    struct SLimit { const char* key; UInt32* target; UInt32 minimum; UInt32 maximum; };
    const SLimit limits[] = { { "maxFieldBytes", &maxFieldBytes, 1, GUCEF_UINT32MAX - 1 }, { "maxRecordBytes", &maxRecordBytes, 1, GUCEF_UINT32MAX - 1 },
                              { "maxColumns", &maxColumns, 1, 1000000 }, { "batchByteLimit", &batchByteLimit, 1, GUCEF_UINT32MAX - 1 } };
    for ( size_t i = 0; i < sizeof( limits ) / sizeof( limits[ 0 ] ); ++i )
    {
        if ( options.HasKey( limits[ i ].key ) )
        {
            if ( !ParseUInt64Option( limits[ i ].key, OptionText( options.GetValueAlways( limits[ i ].key ) ), count, error ) )
                return false;
            if ( count < limits[ i ].minimum || count > limits[ i ].maximum )
            {
                error = CString( "CSV option \"" ) + limits[ i ].key + "\" is out of range";
                return false;
            }
            *limits[ i ].target = (UInt32) count;
        }
    }

    if ( options.HasKey( "onValueError" ) )
    {
        text = OptionText( options.GetValueAlways( "onValueError" ) );
        if ( text.Equals( "fail", false ) ) failOnValueError = true;
        else if ( text.Equals( "null", false ) ) failOnValueError = false;
        else
        {
            error = "CSV option \"onValueError\" is fail or null, not \"" + text + "\"";
            return false;
        }
    }

    if ( options.HasKey( "thousandsSeparator" ) )
        if ( !ParseSeparatorOption( "thousandsSeparator", OptionText( options.GetValueAlways( "thousandsSeparator" ) ), true, numberFormat.thousandsSeparator, error ) )
            return false;
    if ( options.HasKey( "decimalSeparator" ) )
        if ( !ParseSeparatorOption( "decimalSeparator", OptionText( options.GetValueAlways( "decimalSeparator" ) ), false, numberFormat.decimalSeparator, error ) )
            return false;
    if ( '\0' != numberFormat.thousandsSeparator && numberFormat.thousandsSeparator == numberFormat.decimalSeparator )
    {
        error = "CSV options \"thousandsSeparator\" and \"decimalSeparator\" are the same character";
        return false;
    }
    // the list options
    CValueList::TVariantVector values = options.GetValueVectorAlways( "nullTokens" );
    if ( !values.empty() )
    {
        nullTokens.clear();
        for ( size_t i = 0; i < values.size(); ++i )
            nullTokens.push_back( OptionText( values[ i ] ) );
    }

    values = options.GetValueVectorAlways( "skipLinesMatching" );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        CGlobPattern pattern( OptionText( values[ i ] ), true );
        if ( !pattern.IsValid() )
        {
            error = "CSV option \"skipLinesMatching\" has an invalid pattern \"" + OptionText( values[ i ] ) + "\"";
            return false;
        }
        skipLinesMatching.push_back( pattern );
    }

    values = options.GetValueVectorAlways( "currencySymbols" );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        CString symbol = OptionText( values[ i ] );
        if ( 0 == TextLength( symbol ) )
            continue;
        const char* chars = symbol.C_String();
        for ( UInt32 n = 0; n < TextLength( symbol ); ++n )
        {
            if ( ( chars[ n ] >= '0' && chars[ n ] <= '9' ) || chars[ n ] == delimiter || chars[ n ] == '\r' || chars[ n ] == '\n' )
            {
                error = "CSV option \"currencySymbols\" has a symbol with a digit, the delimiter or a line break: \"" + symbol + "\"";
                return false;
            }
        }
        numberFormat.currencySymbols.push_back( symbol );
    }

    values = options.GetValueVectorAlways( "dateFormats" );
    if ( values.empty() )
    {
        CCsvTableText::SDateFormat iso;
        CCsvTableText::CompileDateFormat( "YYYY-MM-DD", iso, error );
        dateFormats.push_back( iso );
    }
    else
    {
        if ( values.size() > 32 )
        {
            error = "CSV option \"dateFormats\" has more than 32 formats";
            return false;
        }
        for ( size_t i = 0; i < values.size(); ++i )
        {
            CCsvTableText::SDateFormat compiled;
            if ( !CCsvTableText::CompileDateFormat( OptionText( values[ i ] ), compiled, error ) )
                return false;
            dateFormats.push_back( compiled );
        }
    }

    values = options.GetValueVectorAlways( "schema" );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        // name|typeText[!][:format]: the name is what is in front of the last '|'
        const CString entry = OptionText( values[ i ] );
        const char* chars = entry.C_String();
        const UInt32 length = TextLength( entry );
        Int32 bar = -1;
        for ( UInt32 n = 0; n < length; ++n )
            if ( '|' == chars[ n ] )
                bar = (Int32) n;
        if ( bar < 0 )
        {
            error = "CSV option \"schema\" entries are name|type, not \"" + entry + "\"";
            return false;
        }
        SCsvColumnSpec spec;
        if ( !ParseColumnSpec( CString( chars, (UInt32) bar ), CString( chars + bar + 1, length - (UInt32) bar - 1 ), true, spec, error ) )
            return false;
        schemaColumns.push_back( spec );
    }

    CValueList::TStringVector overrideKeys = options.GetKeysWithWildcardKeyMatch( "columnTypes.*" );
    static const UInt32 prefixLength = 12;     // "columnTypes."
    for ( size_t i = 0; i < overrideKeys.size(); ++i )
    {
        const CString& key = overrideKeys[ i ];
        if ( TextLength( key ) <= prefixLength )
            continue;
        SCsvColumnSpec spec;
        if ( !ParseColumnSpec( CString( key.C_String() + prefixLength, TextLength( key ) - prefixLength ),
                               OptionText( options.GetValueAlways( key ) ), false, spec, error ) )
            return false;
        columnOverrides.push_back( spec );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

SCsvWriteOptions::SCsvWriteOptions( void )
    : delimiter( ',' )
    , quote( '"' )
    , lineEnd( "\r\n" )
    , header( true )
    , bom( false )
    , nullToken()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
SCsvWriteOptions::Parse( const CDataTableWriterOptions& writerOptions, CString& error )
{GUCEF_TRACE;

    const CValueList& options = writerOptions.codecOptions;
    const CValueList& fileMetaData = writerOptions.keyValueMetaData;

    if ( options.HasKey( "delimiter" ) )
    {
        if ( !ParseSeparatorOption( "delimiter", OptionText( options.GetValueAlways( "delimiter" ) ), false, delimiter, error ) )
            return false;
    }
    else
    if ( fileMetaData.HasKey( "delimiter" ) )
    {
        // from what the reader reported; a value that does not fit is not an error of the writer
        char fromMetaData = '\0';
        CString ignored;
        if ( ParseSeparatorOption( "delimiter", OptionText( fileMetaData.GetValueAlways( "delimiter" ) ), false, fromMetaData, ignored ) )
            delimiter = fromMetaData;
    }

    if ( options.HasKey( "quote" ) )
        if ( !ParseSeparatorOption( "quote", OptionText( options.GetValueAlways( "quote" ) ), false, quote, error ) )
            return false;
    if ( quote == delimiter )
    {
        error = "CSV options \"delimiter\" and \"quote\" are the same character";
        return false;
    }

    CString lineEndText;
    if ( options.HasKey( "lineEnd" ) )
        lineEndText = OptionText( options.GetValueAlways( "lineEnd" ) );
    else
    if ( fileMetaData.HasKey( "lineEnding" ) )
        lineEndText = OptionText( fileMetaData.GetValueAlways( "lineEnding" ) );
    if ( !lineEndText.IsNULLOrEmpty() )
    {
        if ( lineEndText.Equals( "crlf", false ) ) lineEnd = "\r\n";
        else if ( lineEndText.Equals( "lf", false ) ) lineEnd = "\n";
        else if ( lineEndText.Equals( "cr", false ) ) lineEnd = "\r";
        else if ( options.HasKey( "lineEnd" ) )
        {
            error = "CSV option \"lineEnd\" is crlf, lf or cr, not \"" + lineEndText + "\"";
            return false;
        }
    }

    if ( options.HasKey( "header" ) )
        if ( !ParseBoolOption( "header", OptionText( options.GetValueAlways( "header" ) ), header, error ) )
            return false;

    if ( options.HasKey( "bom" ) )
    {
        if ( !ParseBoolOption( "bom", OptionText( options.GetValueAlways( "bom" ) ), bom, error ) )
            return false;
    }
    else
    if ( fileMetaData.HasKey( "bom" ) )
    {
        bool fromMetaData = false;
        CString ignored;
        if ( ParseBoolOption( "bom", OptionText( fileMetaData.GetValueAlways( "bom" ) ), fromMetaData, ignored ) )
            bom = fromMetaData;
    }

    if ( options.HasKey( "nullToken" ) )
        nullToken = OptionText( options.GetValueAlways( "nullToken" ) );

    return true;
}

/*-------------------------------------------------------------------------*/

void
AddCsvOptionValue( CValueList& options, const CString& key, const CString& value )
{GUCEF_TRACE;

    options.SetAllowMultipleValues( true );
    options.Set( key, CVariant( value ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
