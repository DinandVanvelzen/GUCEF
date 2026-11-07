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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCsvTableCodecSupport.h"
#include "TestCsvTableCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace CORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The rows as text: cells joined by '|', rows by ';', null as <null> */
static CString
RowsText( const SCsvTestRead& read )
{
    CString text;
    for ( size_t r = 0; r < read.rows.size(); ++r )
    {
        if ( r > 0 )
            text += ';';
        for ( size_t c = 0; c < read.rows[ r ].size(); ++c )
        {
            if ( c > 0 )
                text += '|';
            text += CsvCell( read, r, c ).IsNULL() ? CString( "<null>" ) : CsvCell( read, r, c ).AsString();
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** Reads text and gives the rows as text; the status in the second argument */
static CString
Read( const char* text, TDataTableStatus* status = GUCEF_NULL, const CValueList* options = GUCEF_NULL )
{
    SCsvTestRead read;
    TDataTableStatus result = ReadCsvSource( MakeCsvSource( text ), GUCEF_NULL != options ? *options : CsvOptions(), read );
    if ( GUCEF_NULL != status )
        *status = result;
    return DATATABLE_STATUS_OK == result ? RowsText( read ) : CString( "<error>" );
}

/*-------------------------------------------------------------------------*/

/** Appends text to bytes: a CString that grows by appending copies its whole content at every append in some builds, which large files cannot afford */
static void
AppendTo( vector< char >& bytes, const char* text )
{
    bytes.insert( bytes.end(), text, text + strlen( text ) );
}

static void
AppendTo( vector< char >& bytes, UInt32 number )
{
    char digits[ 16 ];
    snprintf( digits, sizeof( digits ), "%u", (unsigned) number );
    AppendTo( bytes, digits );
}

/*-------------------------------------------------------------------------*/

/** Reads text with the options and gives the rows as text */
static CString
ReadWith( const char* text, const CValueList& options )
{
    return Read( text, GUCEF_NULL, &options );
}

/*-------------------------------------------------------------------------*/

static TDataTableStatus
StatusOfBytes( const char* bytes, UInt32 length, const CValueList& options, CString* error = GUCEF_NULL )
{
    SCsvTestRead read;
    TDataTableStatus status = ReadCsvSource( MakeCsvSource( bytes, length ), options, read );
    if ( GUCEF_NULL != error )
        *error = read.error;
    return status;
}

/*-------------------------------------------------------------------------*/

static TDataTableStatus
StatusOf( const char* text, const CValueList& options = CsvOptions(), CString* error = GUCEF_NULL )
{
    return StatusOfBytes( text, (UInt32) strlen( text ), options, error );
}

/*-------------------------------------------------------------------------*/

static CValueList
Options( const char* key, const char* value )
{
    CValueList options = CsvOptions();
    SetCsvOption( options, key, value );
    return options;
}

/*-------------------------------------------------------------------------*/

/** A text column on every column: what the file says, as it is */
static CValueList
AllText( UInt32 columns )
{
    CValueList options = CsvOptions();
    CDataTableSchema schema;
    for ( UInt32 i = 0; i < columns; ++i )
        schema.AddColumn( CDataTableColumnDef( "c" + UInt32ToString( i + 1 ), DATATABLE_TYPE_UTF8, true ) );
    CCsvTableCodec::SetSchemaOption( options, schema );
    return options;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCsvTableCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.CsvTableCodec" );

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: the codec is registered under csv and lists its extensions and capabilities" )
    try
    {
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );
        ASSERT_TRUE( codec->GetTypeName() == "csv" );
        CString::StringSet extensions;
        codec->GetFileExtensions( extensions );
        ASSERT_TRUE( 2 == extensions.size() && extensions.count( "csv" ) && extensions.count( "tsv" ) );
        const UInt32 capabilities = codec->GetCapabilities();
        ASSERT_TRUE( 0 != ( capabilities & GUCEF_TABLECODEC_CAPABILITY_READ ) );
        ASSERT_TRUE( 0 != ( capabilities & GUCEF_TABLECODEC_CAPABILITY_WRITE ) );
        ASSERT_TRUE( 0 != ( capabilities & GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS ) );
        ASSERT_TRUE( 0 == ( capabilities & GUCEF_TABLECODEC_CAPABILITY_EMBEDDED_SCHEMA ) );
        ASSERT_TRUE( 0 == ( capabilities & GUCEF_TABLECODEC_CAPABILITY_ROW_GROUPS ) );
        ASSERT_TRUE( 0 == ( capabilities & GUCEF_TABLECODEC_CAPABILITY_STATISTICS ) );

        CTableCodecRegistry::TTableCodecPtr byExtension;
        ASSERT_TRUE( CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCodecForFileExtension( "CSV", byExtension ) );
        ASSERT_TRUE( byExtension.GetPointer() == codec.GetPointer() );
        ASSERT_TRUE( CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCodecForFileExtension( ".tsv", byExtension ) );
        ASSERT_TRUE( byExtension.GetPointer() == codec.GetPointer() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a header and rows become a table with the types of the values" )
    try
    {
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "id,name,price\r\n1,apple,2.50\r\n2,pear,3.75\r\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "id,name,price" );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,utf8,decimal(3,2)" );
        ASSERT_TRUE( RowsText( read ) == "1|apple|2.50;2|pear|3.75" );
        ASSERT_TRUE( 1 == read.batchRowCounts.size() && 2 == CsvAt( read.batchRowCounts, 0 ) );
        ASSERT_FALSE( CsvColumnOf( read, 0 ).nullable );
        ASSERT_FALSE( CsvColumnOf( read, 2 ).nullable );

        // the same through a reader that works the schema out itself
        SCsvTestRead direct;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "id,name,price\r\n1,apple,2.50\r\n2,pear,3.75\r\n", CsvOptions(), direct, 16384, false ) );
        ASSERT_TRUE( RowsText( direct ) == RowsText( read ) );
        ASSERT_TRUE( CsvTypeTexts( direct ) == CsvTypeTexts( read ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: records end at CRLF, LF or a lone CR, the last record needs no line break, and the file reports its line ending" )
    try
    {
        const char* crlf = "a,b\r\n1,2\r\n3,4\r\n";
        const char* lf = "a,b\n1,2\n3,4\n";
        const char* cr = "a,b\r1,2\r3,4\r";
        const char* noFinal = "a,b\n1,2\n3,4";
        const char* mixed = "a,b\r\n1,2\n3,4\r5,6\r\n";
        ASSERT_TRUE( Read( crlf ) == "1|2;3|4" );
        ASSERT_TRUE( Read( lf ) == "1|2;3|4" );
        ASSERT_TRUE( Read( cr ) == "1|2;3|4" );
        ASSERT_TRUE( Read( noFinal ) == "1|2;3|4" );
        ASSERT_TRUE( Read( mixed ) == "1|2;3|4;5|6" );

        SCsvTestRead read;
        ReadCsvText( crlf, CsvOptions(), read );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "crlf" );
        ReadCsvText( lf, CsvOptions(), read );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "lf" );
        ReadCsvText( cr, CsvOptions(), read );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "cr" );
        ReadCsvText( mixed, CsvOptions(), read );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "mixed" );
        ReadCsvText( "a,b", CsvOptions(), read );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "none" );
        ASSERT_TRUE( read.rows.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a CRLF that straddles two records is one line break, a CR that is followed by anything else is one too" )
    try
    {
        ASSERT_TRUE( Read( "a,b\r\n\r\n1,2\r\n\r\n\r\n3,4\r\n" ) == "1|2;3|4" );
        ASSERT_TRUE( Read( "a,b\n\n\n1,2\n" ) == "1|2" );
        ASSERT_TRUE( Read( "a,b\r\r1,2\r\r" ) == "1|2" );
        ASSERT_TRUE( Read( "a,b\r\n1,2\n\r3,4" ) == "1|2;3|4" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: an empty line is a row in a table of one column and nothing in other tables" )
    try
    {
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "name\na\n\nb\n\n", CsvOptions(), read ) );
        ASSERT_TRUE( RowsText( read ) == "a;<null>;b;<null>" );
        ASSERT_TRUE( CsvTypeTexts( read ) == "utf8" );
        ASSERT_TRUE( CsvColumnOf( read, 0 ).nullable );

        // a quoted empty field in the same table is the empty text
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "name\na\n\"\"\nb\n", CsvOptions(), read ) );
        ASSERT_TRUE( RowsText( read ) == "a;;b" );
        ASSERT_FALSE( CsvColumnOf( read, 0 ).nullable );

        // not so with more columns
        ASSERT_TRUE( Read( "a,b\n\n1,2\n\n" ) == "1|2" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: quoted fields hold delimiters, doubled quotes and line breaks, which are kept byte for byte" )
    try
    {
        const CValueList options = AllText( 3 );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\r\n\"a,b\",\"say \"\"hi\"\"\",\"line1\nline2\"\r\n\"x\r\ny\",\"\"\"\",\"\"\r\n" ), options, read ) );
        ASSERT_TRUE( 2 == read.rows.size() );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == "a,b" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ) == "say \"hi\"" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 2 ) ) == "line1\nline2" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 0 ) ) == "x\r\ny" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 1 ) ) == "\"" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 2 ) ).IsNULLOrEmpty() );
        ASSERT_TRUE( !CsvCell( read, 1, 2 ).IsNULL() );

        // a lone CR inside quotes, and a field that is only a line break
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\n\"a\rb\",\"\n\",\"\r\n\"\n" ), options, read ) );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == "a\rb" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ) == "\n" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 2 ) ) == "\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a quote in an unquoted field is literal, blanks around a quoted field are dropped" )
    try
    {
        const CValueList options = AllText( 3 );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\nab\"c,5\" tall,  \"q\"  \n" ), options, read ) );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == "ab\"c" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ) == "5\" tall" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 2 ) ) == "q" );

        // blanks around unquoted text go with trimWhitespace, quoted blanks stay
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\n  x y  ,\"  z  \",\t w\t\n" ), options, read ) );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == "x y" );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ) == "  z  " );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 2 ) ) == "w" );

        CValueList keep = AllText( 3 );
        SetCsvOption( keep, "trimWhitespace", "false" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\n  x y  ,\"  z  \",\t w\t\n" ), keep, read ) );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == "  x y  " );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ) == "  z  " );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 2 ) ) == "\t w\t" );

        // without trimming a blank in front of a quote makes the field an unquoted one that holds quotes
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "h1,h2,h3\n \"q\",b,c\n" ), keep, read ) );
        ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 0 ) ) == " \"q\"" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: the delimiter and the quote can be chosen" )
    try
    {
        ASSERT_TRUE( ReadWith( "a;b\n1;x,y\n", Options( "delimiter", ";" ) ) == "1|x,y" );
        ASSERT_TRUE( ReadWith( "a\tb\n1\tx y\n", Options( "delimiter", "\\t" ) ) == "1|x y" );
        ASSERT_TRUE( ReadWith( "a\tb\n1\tx y\n", Options( "delimiter", "tab" ) ) == "1|x y" );
        ASSERT_TRUE( ReadWith( "a|b\n1|x\n", Options( "delimiter", "pipe" ) ) == "1|x" );
        ASSERT_TRUE( ReadWith( "a b\n1 x\n", Options( "delimiter", " " ) ) == "1|x" );
        ASSERT_TRUE( ReadWith( "a;b\n1;\"x;y\"\n", Options( "delimiter", ";" ) ) == "1|x;y" );

        CValueList apostrophe = Options( "quote", "'" );
        ASSERT_TRUE( ReadWith( "a,b\n1,'x,y'\n", apostrophe ) == "1|x,y" );
        ASSERT_TRUE( ReadWith( "a,b\n1,'it''s'\n", apostrophe ) == "1|it's" );
        ASSERT_TRUE( ReadWith( "a,b\n1,\"q\"\n", apostrophe ) == "1|\"q\"" );

        CValueList none = Options( "quote", "none" );
        ASSERT_TRUE( ReadWith( "a,b\n1,\"q\"\n", none ) == "1|\"q\"" );
        ASSERT_TRUE( ReadWith( "a,b\n1,x\"y\n", none ) == "1|x\"y" );

        // a delimiter that is a decimal comma of the numbers
        CValueList european = Options( "delimiter", ";" );
        SetCsvOption( european, "decimalSeparator", "," );
        SetCsvOption( european, "thousandsSeparator", "." );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a;b\n1.234,50;7,25\n" ), european, read ) );
        ASSERT_TRUE( CsvTypeTexts( read ) == "decimal(6,2),decimal(3,2)" );
        ASSERT_TRUE( RowsText( read ) == "1234.50|7.25" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a byte order mark is removed and reported, UTF-16 is refused" )
    try
    {
        SCsvTestRead read;
        const char withBom[] = "\xEF\xBB\xBF" "id,name\r\n1,\xC3\xA9\r\n";
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( withBom, sizeof( withBom ) - 1 ), CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "id,name" );
        ASSERT_TRUE( RowsText( read ) == "1|\xC3\xA9" );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "bom" ).AsString() == "true" );

        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "id,name\r\n1,a\r\n", CsvOptions(), read ) );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "bom" ).AsString() == "false" );

        // the mark of a file of three bytes only: no header
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( "\xEF\xBB\xBF", 3, CsvOptions() ) );

        const char utf16le[] = "\xFF\xFE" "i\0d\0,\0n\0\r\0\n\0";
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == StatusOfBytes( utf16le, sizeof( utf16le ) - 1, CsvOptions(), &error ) );
        ASSERT_TRUE( error.HasSubstr( "UTF-16" ) >= 0 );
        const char utf16be[] = "\xFE\xFF\0i\0d";
        ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == StatusOfBytes( utf16be, sizeof( utf16be ) - 1, CsvOptions() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a quote that is not closed, text after a closing quote and NUL bytes are corrupt data" )
    try
    {
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n1,\"never closed\n2,3\n", AllText( 2 ), &error ) );
        ASSERT_TRUE( error.HasSubstr( "quote" ) >= 0 );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n1,\"x", AllText( 2 ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n\"", AllText( 2 ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "\"a,b\n1,2\n", AllText( 2 ) ) );

        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n1,\"x\"y\n", AllText( 2 ), &error ) );
        ASSERT_TRUE( error.HasSubstr( "stray quote" ) >= 0 );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n\"x\" y,2\n", AllText( 2 ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( "a,b\n\"x\" ,2\n", AllText( 2 ) ) );      // blanks are fine, trimmed
        CValueList noTrim = AllText( 2 );
        SetCsvOption( noTrim, "trimWhitespace", "false" );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n\"x\" ,2\n", noTrim ) );

        const char nulUnquoted[] = "a,b\n1,x\0y\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulUnquoted, sizeof( nulUnquoted ) - 1, AllText( 2 ), &error ) );
        ASSERT_TRUE( error.HasSubstr( "NUL" ) >= 0 );
        const char nulQuoted[] = "a,b\n1,\"x\0y\"\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulQuoted, sizeof( nulQuoted ) - 1, AllText( 2 ) ) );
        const char nulHeader[] = "a\0,b\n1,2\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulHeader, sizeof( nulHeader ) - 1, CsvOptions() ) );
        const char nulFirst[] = "a,b\n\0,2\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulFirst, sizeof( nulFirst ) - 1, AllText( 2 ) ) );
        const char nulLast[] = "a,b\n1,2\n3,4\0";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulLast, sizeof( nulLast ) - 1, AllText( 2 ) ) );
        const char nulAfterQuote[] = "a,b\n1,\"x\"\0\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulAfterQuote, sizeof( nulAfterQuote ) - 1, AllText( 2 ) ) );
        const char nulInNumber[] = "a,b\n1,2\0\n";
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( nulInNumber, sizeof( nulInNumber ) - 1, CsvOptions() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: text that is not valid UTF-8 is corrupt data in a text column, valid text of every length is kept" )
    try
    {
        const CValueList options = AllText( 2 );
        const char* invalid[] = {
            "\xC3",                         // truncated two byte sequence
            "\xC3\x28",                     // not a continuation byte
            "\x80",                         // a continuation byte alone
            "\xC0\xAF",                     // overlong slash
            "\xC1\xBF",                     // overlong
            "\xE0\x80\xAF",                 // overlong three byte
            "\xED\xA0\x80",                 // a surrogate
            "\xF0\x80\x80\x80",             // overlong four byte
            "\xF4\x90\x80\x80",             // above U+10FFFF
            "\xF5\x80\x80\x80",             // not a lead byte
            "\xFF",                         // never in UTF-8
            "\xE2\x82",                     // truncated three byte sequence
            "\xF0\x9F\x98",                 // truncated four byte sequence
            "ok \xE2\x82\xAC \xFE",         // valid text followed by garbage
        };
        for ( size_t i = 0; i < sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
        {
            // raw bytes: a CString is not made from text that is not UTF-8
            vector< char > text;
            AppendTo( text, "a,b\n1," );
            AppendTo( text, invalid[ i ] );
            AppendTo( text, "\n" );
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( &text[ 0 ], (UInt32) text.size(), options, &error ) );
            ASSERT_TRUE( error.HasSubstr( "UTF-8" ) >= 0 );

            // in quotes as well, and in the header
            vector< char > quoted;
            AppendTo( quoted, "a,b\n1,\"" );
            AppendTo( quoted, invalid[ i ] );
            AppendTo( quoted, "\"\n" );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( &quoted[ 0 ], (UInt32) quoted.size(), options ) );
            vector< char > header;
            AppendTo( header, "a" );
            AppendTo( header, invalid[ i ] );
            AppendTo( header, ",b\n1,2\n" );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( &header[ 0 ], (UInt32) header.size(), CsvOptions() ) );
        }

        // valid text: two, three and four byte sequences and the limits of each range
        SCsvTestRead read;
        const char valid[] = "a,b\n1,\xC2\x80\xDF\xBF\xE0\xA0\x80\xED\x9F\xBF\xEE\x80\x80\xEF\xBF\xBF\xF0\x90\x80\x80\xF4\x8F\xBF\xBF\n";
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( valid, sizeof( valid ) - 1 ), options, read ) );
        ASSERT_TRUE( 1 == read.rows.size() );
        ASSERT_TRUE( 24 == CsvCell( read, 0, 1 ).ByteSize( false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: records with too few or too many fields are corrupt data unless the options let them through" )
    try
    {
        const char* ragged = "a,b,c\n1,2\n3,4,5,6\n7,8,9\n";
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( ragged, CsvOptions(), &error ) );
        ASSERT_TRUE( error.HasSubstr( "2 fields where the file has 3 columns" ) >= 0 );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( ragged, Options( "allowMissingFields", "true" ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( ragged, Options( "allowExtraFields", "true" ) ) );

        CValueList both = Options( "allowMissingFields", "true" );
        SetCsvOption( both, "allowExtraFields", "true" );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( ragged ), both, read ) );
        ASSERT_TRUE( RowsText( read ) == "1|2|<null>;3|4|5;7|8|9" );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,int64,int64" );
        ASSERT_TRUE( CsvColumnOf( read, 2 ).nullable );
        ASSERT_FALSE( CsvColumnOf( read, 0 ).nullable );

        // a column that does not take null refuses the missing field
        CDataTableSchema strict;
        strict.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64, false ) );
        strict.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_INT64, false ) );
        strict.AddColumn( CDataTableColumnDef( "c", DATATABLE_TYPE_INT64, false ) );
        CValueList strictOptions = Options( "allowMissingFields", "true" );
        SetCsvOption( strictOptions, "allowExtraFields", "true" );
        CCsvTableCodec::SetSchemaOption( strictOptions, strict );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == StatusOf( ragged, strictOptions, &error ) );
        ASSERT_TRUE( error.HasSubstr( "no field" ) >= 0 );

        // a trailing delimiter is a field
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "a,b\n1,2,\n" ) );
        ASSERT_TRUE( ReadWith( "a,b\n1,2,\n", Options( "allowExtraFields", "true" ) ) == "1|2" );
        ASSERT_TRUE( Read( "a,b\n1,\n" ) == "1|<null>" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: header names that are empty or repeated are made up, and names are text" )
    try
    {
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a,b,a,a\n1,2,3,4\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "a,b,a_2,a_3" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( ",x,\n1,2,3\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "column_1,x,column_3" );

        // a made up name never collides with a name of the file
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( ",a,a,a,a_2,\"\",x\n1,2,3,4,5,6,7\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "column_1,a,a_3,a_4,a_2,column_6,x" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "column_2,,x\n1,2,3\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "column_2,column_2_2,x" );

        // quoted names, names with delimiters, quotes, blanks and non-ASCII letters, control characters
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "\" first col \",\"x,y\",\"say \"\"hi\"\"\",\xC3\xA9t\xC3\xA9,\"tab\there\"\n1,2,3,4,5\n", CsvOptions(), read ) );
        ASSERT_TRUE( 5 == CsvColumnCountOf( read ) );
        ASSERT_TRUE( CsvColumnOf( read, 0 ).name == "first col" );
        ASSERT_TRUE( CsvColumnOf( read, 1 ).name == "x,y" );
        ASSERT_TRUE( CsvColumnOf( read, 2 ).name == "say \"hi\"" );
        ASSERT_TRUE( CsvColumnOf( read, 3 ).name == "\xC3\xA9t\xC3\xA9" );
        ASSERT_TRUE( CsvColumnOf( read, 4 ).name == "tab here" );

        // names are case sensitive
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "A,a\n1,2\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "A,a" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a file with a header only has no rows, a file without a header row or without records is corrupt" )
    try
    {
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a,b,c\n", CsvOptions(), read ) );
        ASSERT_TRUE( read.rows.empty() );
        ASSERT_TRUE( read.batchRowCounts.empty() );
        ASSERT_TRUE( CsvTypeTexts( read ) == "utf8,utf8,utf8" );
        ASSERT_TRUE( CsvColumnOf( read, 0 ).nullable );

        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "", CsvOptions(), &error ) );
        ASSERT_TRUE( error.HasSubstr( "header" ) >= 0 );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "\n\n\r\n" ) );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOf( "", Options( "hasHeader", "false" ), &error ) );
        ASSERT_TRUE( error.HasSubstr( "no records" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: without a header the columns are numbered, and a given schema names and types them" )
    try
    {
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "1,x\n2,y\n" ), Options( "hasHeader", "false" ), read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "column_1,column_2" );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,utf8" );
        ASSERT_TRUE( RowsText( read ) == "1|x;2|y" );

        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
        schema.AddColumn( CDataTableColumnDef::Decimal( "price", 10, 3, true ) );
        schema.AddColumn( CDataTableColumnDef( "day", DATATABLE_TYPE_DATE, true ) );
        CValueList options = Options( "hasHeader", "false" );
        CCsvTableCodec::SetSchemaOption( options, schema );
        const char* data = "7,1.5,2024-02-29\n8,,\n";
        for ( int viaMetaData = 0; viaMetaData < 2; ++viaMetaData )
        {
            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( data ), options, read, 16384, 0 != viaMetaData ) );
            ASSERT_TRUE( CsvColumnNames( read ) == "id,price,day" );
            ASSERT_TRUE( CsvTypeTexts( read ) == "int64,decimal(10,3),date" );
            ASSERT_FALSE( CsvColumnOf( read, 0 ).nullable );
            ASSERT_TRUE( CsvColumnOf( read, 1 ).nullable );
            ASSERT_TRUE( RowsText( read ) == "7|1.500|2024-02-29;8|<null>|<null>" );
        }

        // with a header row in the file the schema still decides: the header is skipped, its names are not used
        CValueList withHeader = CsvOptions();
        CCsvTableCodec::SetSchemaOption( withHeader, schema );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "A,B,C\n7,1.5,2024-02-29\n" ), withHeader, read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "id,price,day" );
        ASSERT_TRUE( RowsText( read ) == "7|1.500|2024-02-29" );

        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == StatusOf( "A,B\n7,1.5\n", withHeader, &error ) );
        ASSERT_TRUE( error.HasSubstr( "header has 2 columns" ) >= 0 );

        // the schema decides over what the values look like: a number column of a text
        CDataTableSchema texts;
        texts.AddColumn( CDataTableColumnDef( "zip", DATATABLE_TYPE_UTF8, false ) );
        CValueList textOptions = Options( "hasHeader", "false" );
        CCsvTableCodec::SetSchemaOption( textOptions, texts );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "02134\n00501\n" ), textOptions, read ) );
        ASSERT_TRUE( RowsText( read ) == "02134;00501" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: title lines are skipped and lines that match a pattern are not rows" )
    try
    {
        ASSERT_TRUE( ReadWith( "Report title\nGenerated 2024\nid,name\n1,a\n", Options( "skipLeadingLines", "2" ) ) == "1|a" );
        ASSERT_TRUE( ReadWith( "t1\n", Options( "skipLeadingLines", "5" ) ) == "<error>" );

        // title lines are lines, quotes and delimiters in them mean nothing
        ASSERT_TRUE( ReadWith( "Title \"with a quote, and more\nid,name\n1,a\n", Options( "skipLeadingLines", "1" ) ) == "1|a" );

        CValueList options = Options( "currencySymbols", "$" );
        SetCsvOption( options, "thousandsSeparator", "," );
        SetCsvOption( options, "skipLinesMatching", "Transactions Total*" );
        CCsvTableCodec::AddListOption( options, "skipLinesMatching", "*Subtotal" );
        CCsvTableCodec::AddListOption( options, "skipLinesMatching", "Account*" );
        const char* statement = "Account Report 1234\nDate,Amount\n2024-01-01,\"$1,234.50\"\n\"Transactions Total\",\"$1,234.50\"\n2024-01-02,$5.00\nQ1 Subtotal\n";
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( statement ), options, read ) );
        ASSERT_TRUE( CsvColumnNames( read ) == "Date,Amount" );
        ASSERT_TRUE( CsvTypeTexts( read ) == "date,decimal(6,2)" );
        ASSERT_TRUE( RowsText( read ) == "2024-01-01|1234.50;2024-01-02|5.00" );

        // the patterns are case sensitive and match the whole line
        ASSERT_TRUE( ReadWith( "a,b\n1,2\nTOTAL,x\n", Options( "skipLinesMatching", "total*" ) ) == "1|2;TOTAL|x" );
        ASSERT_TRUE( ReadWith( "a,b\n1,2\nTOTAL,x\n", Options( "skipLinesMatching", "TOTAL*" ) ) == "1|2" );
        ASSERT_TRUE( ReadWith( "a,b\n1,2\nTOTAL,x\n", Options( "skipLinesMatching", "TOTAL" ) ) == "1|2;TOTAL|x" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: null tokens decide what is null, and quoted text is never null" )
    try
    {
        
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a,b\n1,x\n--,N/A\n,y\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvTypeTexts( read ) == "utf8,utf8" );
        ASSERT_TRUE( RowsText( read ) == "1|x;--|N/A;<null>|y" );

        CValueList tokens = CsvOptions();
        CCsvTableCodec::AddListOption( tokens, "nullTokens", "" );
        CCsvTableCodec::AddListOption( tokens, "nullTokens", "--" );
        CCsvTableCodec::AddListOption( tokens, "nullTokens", "N/A" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a,b\n1,x\n--,N/A\n,y\n" ), tokens, read ) );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,utf8" );
        ASSERT_TRUE( RowsText( read ) == "1|x;<null>|<null>;<null>|y" );
        ASSERT_TRUE( CsvColumnOf( read, 0 ).nullable && CsvColumnOf( read, 1 ).nullable );

        // tokens are case sensitive and match the whole field
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a\nn/a\nN/A\nN/A \n--x\n" ), tokens, read ) );
        ASSERT_TRUE( RowsText( read ) == "n/a;<null>;<null>;--x" );

        // the empty text is a null only when it is a token
        CValueList onlyDash = CsvOptions();
        CCsvTableCodec::AddListOption( onlyDash, "nullTokens", "--" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a,b\n1,\n--,x\n" ), onlyDash, read ) );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,utf8" );
        ASSERT_TRUE( RowsText( read ) == "1|;<null>|x" );

        // quoted: in a text column the text itself, in a column of numbers a missing value
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a,b\n\"\",x\n1,\"\"\n2,y\n", CsvOptions(), read ) );
        ASSERT_TRUE( CsvTypeTexts( read ) == "int64,utf8" );
        ASSERT_TRUE( RowsText( read ) == "<null>|x;1|;2|y" );
        ASSERT_TRUE( CsvColumnOf( read, 0 ).nullable );
        ASSERT_FALSE( CsvColumnOf( read, 1 ).nullable );
        ASSERT_TRUE( !CsvCell( read, 1, 1 ).IsNULL() );

        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "x\nN/A\n\"N/A\"\nabc\n" ), tokens, read ) );
        ASSERT_TRUE( RowsText( read ) == "<null>;N/A;abc" );

        // a column that does not take null refuses it
        CDataTableSchema strict;
        strict.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64, false ) );
        CValueList strictOptions = CsvOptions();
        CCsvTableCodec::SetSchemaOption( strictOptions, strict );
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == StatusOf( "a\n1\n\n", strictOptions, &error ) );
        ASSERT_TRUE( error.HasSubstr( "does not take null" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: very long fields are read whole, whatever the size of the blocks of the reader" )
    try
    {
        const UInt32 length = 1024 * 1024 + 3;
        vector< char > file;
        const char* head = "a,b\n1,";
        file.insert( file.end(), head, head + strlen( head ) );
        for ( UInt32 i = 0; i < length; ++i )
            file.push_back( (char) ( 'a' + i % 26 ) );
        file.push_back( '\n' );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( file ), AllText( 2 ), read ) );
        ASSERT_TRUE( 1 == read.rows.size() );
        ASSERT_TRUE( length == CsvCell( read, 0, 1 ).ByteSize( false ) );
        const char* value = CsvCell( read, 0, 1 ).AsCharPtr();
        ASSERT_TRUE( 'a' == value[ 0 ] && ( 'a' + ( length - 1 ) % 26 ) == value[ length - 1 ] );

        // quoted, with quotes and line breaks all through it
        vector< char > quoted;
        quoted.insert( quoted.end(), head, head + strlen( head ) );
        quoted.push_back( '"' );
        UInt32 expected = 0;
        for ( UInt32 i = 0; i < 100000; ++i )
        {
            const char* piece = 0 == i % 3 ? "x\"\"y" : ( 1 == i % 3 ? "p\r\nq" : "r,s" );
            quoted.insert( quoted.end(), piece, piece + strlen( piece ) );
            expected += 1 == i % 3 ? 4 : 3;
        }
        quoted.push_back( '"' );
        quoted.push_back( '\n' );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( quoted ), AllText( 2 ), read ) );
        ASSERT_TRUE( 1 == read.rows.size() );
        ASSERT_TRUE( expected == CsvCell( read, 0, 1 ).ByteSize( false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a doubled quote, a CRLF or a delimiter that falls on the edge of a block of the reader is read as if it did not" )
    try
    {
        // blocks are 64 KiB: move the text so that every kind of sequence straddles the edge
        for ( UInt32 pad = 65536 - 12; pad <= 65536 + 4; ++pad )
        {
            vector< char > file;
            const char* head = "a,b\r\n1,\"";
            file.insert( file.end(), head, head + strlen( head ) );
            const UInt32 prefix = (UInt32) file.size();
            for ( UInt32 i = prefix; i < pad; ++i )
                file.push_back( 'p' );
            const char* tail = "\"\"z\"\r\n2,\"\r\n\"\r\n3,w\n";
            file.insert( file.end(), tail, tail + strlen( tail ) );

            SCsvTestRead read;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( file ), AllText( 2 ), read ) );
            ASSERT_TRUE( 3 == read.rows.size() );
            ASSERT_TRUE( CsvCellText( CsvCell( read, 0, 1 ) ).ByteSize() - 1 == ( pad > prefix ? pad - prefix : 0 ) + 2 );
            ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 0 ) ) == "2" );
            ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 1 ) ) == "\r\n" );
            ASSERT_TRUE( CsvCellText( CsvCell( read, 2, 1 ) ) == "w" );
        }

        for ( UInt32 length = 65536 - 8; length <= 65536 + 2; ++length )
        {
            vector< char > file;
            const char* head = "a,b\r\n1,";
            file.insert( file.end(), head, head + strlen( head ) );
            for ( UInt32 i = (UInt32) file.size(); i < length; ++i )
                file.push_back( 'x' );
            const char* tail = "\r\n2,y\r\n";
            file.insert( file.end(), tail, tail + strlen( tail ) );
            SCsvTestRead read;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( file ), AllText( 2 ), read ) );
            ASSERT_TRUE( 2 == read.rows.size() );
            ASSERT_TRUE( CsvCellText( CsvCell( read, 1, 0 ) ) == "2" && CsvCellText( CsvCell( read, 1, 1 ) ) == "y" );
        }

        // a NUL byte on the edge is found as well
        vector< char > file;
        const char* head = "a,b\n1,";
        file.insert( file.end(), head, head + strlen( head ) );
        while ( file.size() < 65536 )
            file.push_back( 'x' );
        file.push_back( '\0' );
        file.push_back( '\n' );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == StatusOfBytes( &file[ 0 ], (UInt32) file.size(), AllText( 2 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: fields, records and columns are bounded, a hostile file does not use memory beyond the bounds" )
    try
    {
        CString field100;
        for ( int i = 0; i < 100; ++i ) field100 += 'x';
        CString text = "a,b\n1," + field100 + "\n";
        CValueList limit = AllText( 2 );
        SetCsvOption( limit, "maxFieldBytes", "100" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( text.C_String(), limit ) );
        text = "a,b\n1," + field100 + "y\n";
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == StatusOf( text.C_String(), limit, &error ) );
        ASSERT_TRUE( error.HasSubstr( "field" ) >= 0 );

        // the limit counts the bytes of the field as the table gets them: doubled quotes count once
        CString quotes = "a,b\n1,\"";
        for ( int i = 0; i < 100; ++i ) quotes += "\"\"";
        quotes += "\"\n";
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( quotes.C_String(), limit ) );

        CValueList recordLimit = AllText( 3 );
        SetCsvOption( recordLimit, "maxRecordBytes", "50" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( "a,b,c\n1234567890,1234567890,1234567890\n", recordLimit ) );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == StatusOf( "a,b,c\n12345678901234567890,12345678901234567890,12345678901234567890\n", recordLimit, &error ) );
        ASSERT_TRUE( error.HasSubstr( "record" ) >= 0 );

        // a quote that is never closed in ten megabytes of data stops at the bound of the record
        vector< char > hostile;
        hostile.push_back( '"' );
        for ( UInt32 i = 0; i < 10 * 1024 * 1024; ++i )
            hostile.push_back( 'q' );
        CValueList small = CsvOptions();
        SetCsvOption( small, "maxRecordBytes", "1048576" );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == StatusOfBytes( &hostile[ 0 ], (UInt32) hostile.size(), small ) );

        CValueList columnLimit = CsvOptions();
        SetCsvOption( columnLimit, "maxColumns", "3" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( "a,b,c\n1,2,3\n", columnLimit ) );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == StatusOf( "a,b,c,d\n1,2,3,4\n", columnLimit, &error ) );
        ASSERT_TRUE( error.HasSubstr( "fields" ) >= 0 );

        // many columns are fine up to the default bound, beyond it not
        CString wide, wideRow;
        for ( int i = 0; i < 5000; ++i )
        {
            wide += ( i > 0 ? "," : "" ) + CString( "c" ) + UInt32ToString( i );
            wideRow += ( i > 0 ? "," : "" ) + UInt32ToString( i );
        }
        text = wide + "\n" + wideRow + "\n";
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( text.C_String() ), CsvOptions(), read ) );
        ASSERT_TRUE( 5000 == CsvColumnCountOf( read ) );
        ASSERT_TRUE( 4999 == CsvCell( read, 0, 4999 ).AsInt32() );

        CString tooWide;
        for ( int i = 0; i < 10001; ++i )
            tooWide += ( i > 0 ? "," : "" ) + CString( "c" ) + UInt32ToString( i );
        tooWide += "\n";
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == StatusOf( tooWide.C_String() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a file is delivered in batches of the size that is asked for, in order, with their position in the file" )
    try
    {
        vector< char > text;
        AppendTo( text, "id,value\n" );
        const UInt32 total = 50000;
        for ( UInt32 i = 0; i < total; ++i )
        {
            AppendTo( text, i );
            AppendTo( text, "," );
            AppendTo( text, i * 3 );
            AppendTo( text, ".25\n" );
        }

        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( text ), CsvOptions(), read, 1024 ) );
        ASSERT_TRUE( total == read.rows.size() );
        ASSERT_TRUE( 49 == read.batchRowCounts.size() );
        for ( size_t b = 0; b < read.batchRowCounts.size(); ++b )
        {
            ASSERT_TRUE( ( b + 1 < read.batchRowCounts.size() ? 1024 : total - 48 * 1024 ) == CsvAt( read.batchRowCounts, b ) );
            ASSERT_TRUE( b * 1024 == CsvAt( read.batchFirstRows, b ) );
        }
        ASSERT_TRUE( 0 == CsvCell( read, 0, 0 ).AsInt32() && 49999 == CsvCell( read, total - 1, 0 ).AsInt32() );
        ASSERT_TRUE( CsvCell( read, 12345, 1 ).AsString() == "37035.25" );

        // batches of one row, and a batch size that divides the rows
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a\n1\n2\n3\n", CsvOptions(), read, 1 ) );
        ASSERT_TRUE( 3 == read.batchRowCounts.size() && 1 == CsvAt( read.batchRowCounts, 2 ) );
        ASSERT_TRUE( 2 == CsvAt( read.batchFirstRows, 2 ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvText( "a\n1\n2\n3\n4\n", CsvOptions(), read, 2 ) );
        ASSERT_TRUE( 2 == read.batchRowCounts.size() && 2 == CsvAt( read.batchRowCounts, 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: reading a large file takes memory for a block, a record and a batch, not for the file" )
    try
    {
        // about 12 MB of text
        vector< char > text;
        AppendTo( text, "id,amount,note\n" );
        const UInt32 total = 400000;
        for ( UInt32 i = 0; i < total; ++i )
        {
            AppendTo( text, i );
            AppendTo( text, "," );
            AppendTo( text, i % 1000 );
            AppendTo( text, ".50,note number " );
            AppendTo( text, i );
            AppendTo( text, "\n" );
        }
        const UInt64 fileSize = text.size();
        ASSERT_TRUE( fileSize > 10 * 1024 * 1024 );

        CCsvMemoryFile file( &text[ 0 ], (UInt32) fileSize );
        CDataTableBufferPool pool;
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );

        CString error;
        CITableFileMetaDataPtr metaData;
        CValueList options = CsvOptions();
        SetCsvOption( options, "inferRowCount", "1000" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( CsvIo( file.Access() ), options, metaData, error ) );
        CITableReaderPtr reader;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateReader( file.Access(), metaData, options, reader, error ) );

        CDataTableReadRequest request;
        request.batchRowCount = 4096;
        request.bufferPool = &pool;
        ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );

        UInt64 rows = 0, sum = 0, peak = 0;
        for ( ;; )
        {
            CDataTableBatchPtr batch;
            TDataTableStatus status = reader->ReadNextBatch( batch );
            if ( DATATABLE_STATUS_END == status )
                break;
            ASSERT_TRUE( DATATABLE_STATUS_OK == status );
            const Int64* ids = batch->GetColumn( 0 ).GetInt64Data();
            ASSERT_TRUE( GUCEF_NULL != ids );
            for ( UInt32 r = 0; r < batch->GetRowCount(); ++r )
                sum += (UInt64) ids[ r ];
            rows += batch->GetRowCount();

            CDataTableBufferPoolStats stats;
            pool.GetStats( stats );
            if ( stats.bytesInUse > peak )
                peak = stats.bytesInUse;
        }
        ASSERT_TRUE( total == rows );
        ASSERT_TRUE( (UInt64) total * ( total - 1 ) / 2 == sum );
        // the block, the batch with its text, and the size classes that round up: far below the size of the file
        ASSERT_TRUE( peak < 4 * 1024 * 1024 );
        ASSERT_TRUE( peak < fileSize / 3 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: a row group can be opened again, a projection picks and orders the columns" )
    try
    {
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );
        const char* fileText = "a,b,c\n1,x,1.5\n2,y,2.5\n3,z,3.5\n";
        CCsvMemoryFile file( fileText, (UInt32) strlen( fileText ) );
        const CValueList options = CsvOptions();
        CString error;
        CITableFileMetaDataPtr metaData;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( CsvIo( file.Access() ), options, metaData, error ) );
        CITableReaderPtr reader;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateReader( file.Access(), metaData, options, reader, error ) );

        ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == reader->OpenRowGroup( 1, CDataTableReadRequest() ) );
        CDataTableBatchPtr batch;
        ASSERT_TRUE( DATATABLE_STATUS_OK != reader->ReadNextBatch( batch ) && DATATABLE_STATUS_END != reader->ReadNextBatch( batch ) );

        for ( int pass = 0; pass < 3; ++pass )
        {
            CDataTableReadRequest request;
            request.batchRowCount = 2;
            request.fileOrdinal = 7;
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->ReadNextBatch( batch ) );
            ASSERT_TRUE( 2 == batch->GetRowCount() && 3 == batch->GetColumnCount() );
            ASSERT_TRUE( 7 == batch->GetOrigin().fileOrdinal && 0 == batch->GetOrigin().rowGroup && 0 == batch->GetOrigin().firstRowInRowGroup );
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->ReadNextBatch( batch ) );
            ASSERT_TRUE( 1 == batch->GetRowCount() && 2 == batch->GetOrigin().firstRowInRowGroup );
            ASSERT_TRUE( DATATABLE_STATUS_END == reader->ReadNextBatch( batch ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == reader->ReadNextBatch( batch ) );
        }

        // columns c and a, in that order
        CDataTableReadRequest projected;
        projected.projection.push_back( 2 );
        projected.projection.push_back( 0 );
        ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, projected ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == reader->ReadNextBatch( batch ) );
        ASSERT_TRUE( 2 == batch->GetColumnCount() && 3 == batch->GetRowCount() );
        ASSERT_TRUE( 2 == batch->GetSchemaColumnIndex( 0 ) && 0 == batch->GetSchemaColumnIndex( 1 ) );
        ASSERT_TRUE( batch->GetSchema()->GetColumn( 2 ).name == "c" );
        CVariant value;
        ASSERT_TRUE( batch->GetValue( 1, 0, value, false ) && value.AsString() == "2.5" );
        ASSERT_TRUE( batch->GetValue( 1, 1, value, false ) && 2 == value.AsInt32() );

        CDataTableReadRequest invalid;
        invalid.projection.push_back( 3 );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == reader->OpenRowGroup( 0, invalid ) );
        CDataTableReadRequest twice;
        twice.projection.push_back( 1 );
        twice.projection.push_back( 1 );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == reader->OpenRowGroup( 0, twice ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: readers that share one input under a lock each get their own rows" )
    try
    {
        vector< char > text;
        AppendTo( text, "n\n" );
        for ( UInt32 i = 0; i < 20000; ++i )
        {
            AppendTo( text, i );
            AppendTo( text, "\n" );
        }
        CCsvMemoryFile file( &text[ 0 ], (UInt32) text.size() );
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );
        CString error;
        CITableFileMetaDataPtr metaData;
        const CValueList options = CsvOptions();
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( CsvIo( file.Access() ), options, metaData, error ) );

        MT::CMutex lock;
        CITableReaderPtr first, second;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateReader( file.Access(), metaData, options, first, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateReader( file.Access(), metaData, options, second, error ) );
        CDataTableReadRequest request;
        request.batchRowCount = 100;
        request.sharedFileLock = &lock;
        ASSERT_TRUE( DATATABLE_STATUS_OK == first->OpenRowGroup( 0, request ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == second->OpenRowGroup( 0, request ) );

        UInt64 sumFirst = 0, sumSecond = 0, rowsFirst = 0, rowsSecond = 0;
        bool firstDone = false, secondDone = false;
        while ( !firstDone || !secondDone )
        {
            CDataTableBatchPtr batch;
            if ( !firstDone )
            {
                TDataTableStatus status = first->ReadNextBatch( batch );
                if ( DATATABLE_STATUS_END == status ) firstDone = true;
                else
                {
                    ASSERT_TRUE( DATATABLE_STATUS_OK == status );
                    for ( UInt32 r = 0; r < batch->GetRowCount(); ++r ) sumFirst += (UInt64) batch->GetColumn( 0 ).GetInt64Data()[ r ];
                    rowsFirst += batch->GetRowCount();
                }
            }
            if ( !secondDone )
            {
                TDataTableStatus status = second->ReadNextBatch( batch );
                if ( DATATABLE_STATUS_END == status ) secondDone = true;
                else
                {
                    ASSERT_TRUE( DATATABLE_STATUS_OK == status );
                    for ( UInt32 r = 0; r < batch->GetRowCount(); ++r ) sumSecond += (UInt64) batch->GetColumn( 0 ).GetInt64Data()[ r ];
                    rowsSecond += batch->GetRowCount();
                }
            }
        }
        ASSERT_TRUE( 20000 == rowsFirst && 20000 == rowsSecond );
        ASSERT_TRUE( 199990000 == sumFirst && sumFirst == sumSecond );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodec: options that are not valid are refused as invalid arguments" )
    try
    {
        const char* keys[] = { "delimiter", "delimiter", "quote", "hasHeader", "skipLeadingLines", "dateFormats", "dateFormats", "dateFormats", "inferRowCount",
                               "maxFieldBytes", "maxColumns", "onValueError", "trimWhitespace", "negativeParentheses", "thousandsSeparator", "decimalSeparator",
                               "columnTypes.a", "columnTypes.a", "schema", "schema", "currencySymbols", "skipLinesMatching", "lineEnd" };
        const char* values[] = { "ab", "\r", "\"\"x", "maybe", "-1", "YYYY", "MM/DD", "YYYY-MM-DD-DD", "many", "0", "0", "sometimes", "2", "x", "ab", "xy",
                                 "bogus", "date:YYYY", "nodelimiter", "x|", "a1", "[", "crlf" };
        for ( size_t i = 0; i < sizeof( keys ) / sizeof( keys[ 0 ] ); ++i )
        {
            CString error;
            CValueList options = Options( keys[ i ], values[ i ] );
            if ( 0 == strcmp( keys[ i ], "lineEnd" ) )
                continue;           // a write option, ignored when reading
            if ( 0 == strcmp( keys[ i ], "columnTypes.a" ) && 0 == strcmp( values[ i ], "date:YYYY" ) )
                continue;
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == StatusOf( "a,b\n1,2\n", options, &error ) );
            ASSERT_FALSE( error.IsNULLOrEmpty() );
        }

        CValueList same = Options( "thousandsSeparator", "," );
        SetCsvOption( same, "decimalSeparator", "," );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == StatusOf( "a,b\n1,2\n", same ) );
        CValueList sameQuote = Options( "quote", ";" );
        SetCsvOption( sameQuote, "delimiter", ";" );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == StatusOf( "a,b\n1,2\n", sameQuote ) );

        // a type for a column the file does not have
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == StatusOf( "a,b\n1,2\n", Options( "columnTypes.nope", "int64" ), &error ) );
        ASSERT_TRUE( error.HasSubstr( "nope" ) >= 0 );

        // options other codecs know are ignored
        ASSERT_TRUE( DATATABLE_STATUS_OK == StatusOf( "a,b\n1,2\n", Options( "someOtherCodecOption", "1" ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
