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

#include <string.h>

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableSupport.h"
#include "TestCsvTableCodecSupport.h"
#include "TestCsvTableCodecWriter.h"

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

static CDataTableSchemaPtr
TwoTexts( void )
{
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_UTF8, true ) );
    schema.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_UTF8, true ) );
    return CreateDataTableSchemaPtr( schema );
}

/*-------------------------------------------------------------------------*/

/** Appends a text, or a null for the pointer NULL, to the column of a builder */
static void
AddText( CDataTableBatchBuilder& builder, UInt32 column, const char* text )
{
    if ( GUCEF_NULL == text )
        builder.GetColumn( column ).AppendNull();
    else
        builder.GetColumn( column ).AppendUtf8( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

static CDataTableBatchPtr
TextBatch( const CDataTableSchemaPtr& schema, const char* const* cells, UInt32 rowCount )
{
    CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 r = 0; r < rowCount; ++r )
        for ( UInt32 c = 0; c < schema->GetColumnCount(); ++c )
            AddText( builder, c, cells[ r * schema->GetColumnCount() + c ] );
    CDataTableBatchPtr batch;
    CString error;
    builder.Build( batch, error );
    return batch;
}

/*-------------------------------------------------------------------------*/

/** Writes the batch and gives the text; "<error>" when the write failed */
static CString
WriteText( const CDataTableSchemaPtr& schema, const CDataTableBatchPtr& batch, const CDataTableWriterOptions& options, TDataTableStatus* status = GUCEF_NULL )
{
    CCsvMemoryFile file;
    CString error;
    TDataTableStatus result = WriteCsvBatch( schema, *batch, options, file, error );
    if ( GUCEF_NULL != status )
        *status = result;
    return DATATABLE_STATUS_OK == result ? file.Text() : CString( "<error>" );
}

/*-------------------------------------------------------------------------*/

/** Read options with a text column per column: what the file says, as it is */
static CValueList
AllTextOptionsFor( UInt32 columns )
{
    CValueList options = CsvOptions();
    CDataTableSchema schema;
    for ( UInt32 i = 0; i < columns; ++i )
    {
        CString name;
        name += (char) ( 'a' + i );
        schema.AddColumn( CDataTableColumnDef( name, DATATABLE_TYPE_UTF8, true ) );
    }
    CCsvTableCodec::SetSchemaOption( options, schema );
    return options;
}

/*-------------------------------------------------------------------------*/

static CDataTableWriterOptions
Writer( const char* key = GUCEF_NULL, const char* value = GUCEF_NULL )
{
    CDataTableWriterOptions options = CsvWriterOptions();
    if ( GUCEF_NULL != key )
        SetCsvWriterOption( options, key, value );
    return options;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCsvTableCodecWriterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.CsvTableCodecWriter" );

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: fields are quoted when they hold the delimiter, a quote, CR or LF, or start or end with a blank" )
    try
    {
        const CDataTableSchemaPtr schema = TwoTexts();
        const char* cells[] = {
            "plain", "with,comma",
            "say \"hi\"", "line1\nline2",
            " lead", "trail ",
            "", GUCEF_NULL,
            "\tx", "y\t",
            "cr\rx", "crlf\r\nx",
            "in side", "semi;colon" };
        const CDataTableBatchPtr batch = TextBatch( schema, cells, 7 );
        ASSERT_FALSE( batch.IsNULL() );
        const CString expected =
            "a,b\r\n"
            "plain,\"with,comma\"\r\n"
            "\"say \"\"hi\"\"\",\"line1\nline2\"\r\n"
            "\" lead\",\"trail \"\r\n"
            "\"\",\r\n"
            "\"\tx\",\"y\t\"\r\n"
            "\"cr\rx\",\"crlf\r\nx\"\r\n"
            "in side,semi;colon\r\n";
        ASSERT_TRUE( WriteText( schema, batch, Writer() ) == expected );

        // with another delimiter the comma is free and the semicolon is not
        const CString semicolons =
            "a;b\r\n"
            "plain;with,comma\r\n"
            "\"say \"\"hi\"\"\";\"line1\nline2\"\r\n"
            "\" lead\";\"trail \"\r\n"
            "\"\";\r\n"
            "\"\tx\";\"y\t\"\r\n"
            "\"cr\rx\";\"crlf\r\nx\"\r\n"
            "in side;\"semi;colon\"\r\n";
        ASSERT_TRUE( WriteText( schema, batch, Writer( "delimiter", ";" ) ) == semicolons );

        // the quote is a choice too
        const char* oneCell[] = { "it's", "x" };
        const CString apostrophes = WriteText( schema, TextBatch( schema, oneCell, 1 ), Writer( "quote", "'" ) );
        ASSERT_TRUE( apostrophes == "a,b\r\n'it''s',x\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: the names in the header are quoted by the same rules" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "plain", DATATABLE_TYPE_UTF8 ) );
        schema.AddColumn( CDataTableColumnDef( "with,comma", DATATABLE_TYPE_UTF8 ) );
        schema.AddColumn( CDataTableColumnDef( "say \"hi\"", DATATABLE_TYPE_UTF8 ) );
        schema.AddColumn( CDataTableColumnDef( " padded ", DATATABLE_TYPE_UTF8 ) );
        const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
        const char* cells[] = { "1", "2", "3", "4" };
        ASSERT_TRUE( WriteText( shared, TextBatch( shared, cells, 1 ), Writer() ) == "plain,\"with,comma\",\"say \"\"hi\"\"\",\" padded \"\r\n1,2,3,4\r\n" );
        ASSERT_TRUE( WriteText( shared, TextBatch( shared, cells, 1 ), Writer( "header", "false" ) ) == "1,2,3,4\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: line ends, header and byte order mark are options" )
    try
    {
        const CDataTableSchemaPtr schema = TwoTexts();
        const char* cells[] = { "1", "2", "3", "4" };
        const CDataTableBatchPtr batch = TextBatch( schema, cells, 2 );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "lineEnd", "crlf" ) ) == "a,b\r\n1,2\r\n3,4\r\n" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "lineEnd", "lf" ) ) == "a,b\n1,2\n3,4\n" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "lineEnd", "cr" ) ) == "a,b\r1,2\r3,4\r" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "lineEnd", "LF" ) ) == "a,b\n1,2\n3,4\n" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "header", "false" ) ) == "1,2\r\n3,4\r\n" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "bom", "true" ) ) == "\xEF\xBB\xBF" "a,b\r\n1,2\r\n3,4\r\n" );

        TDataTableStatus status = DATATABLE_STATUS_OK;
        WriteText( schema, batch, Writer( "lineEnd", "windows" ), &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );
        WriteText( schema, batch, Writer( "delimiter", "ab" ), &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );
        WriteText( schema, batch, Writer( "header", "perhaps" ), &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );
        CDataTableWriterOptions sameChar = Writer( "delimiter", "'" );
        SetCsvWriterOption( sameChar, "quote", "'" );
        WriteText( schema, batch, sameChar, &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );

        // no rows: the header only, and nothing at all without a header
        CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        CDataTableBatchPtr empty;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( empty, error ) );
        ASSERT_TRUE( WriteText( schema, empty, Writer() ) == "a,b\r\n" );
        ASSERT_TRUE( WriteText( schema, empty, Writer( "header", "false" ) ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: null is an empty field, the empty text is quoted, and a null token is written for nulls and quoted for text" )
    try
    {
        const CDataTableSchemaPtr schema = TwoTexts();
        const char* cells[] = { GUCEF_NULL, "", "", GUCEF_NULL, "NULL", "x", "n", "NULL " };
        const CDataTableBatchPtr batch = TextBatch( schema, cells, 4 );
        ASSERT_TRUE( WriteText( schema, batch, Writer() ) == "a,b\r\n,\"\"\r\n\"\",\r\nNULL,x\r\nn,\"NULL \"\r\n" );
        ASSERT_TRUE( WriteText( schema, batch, Writer( "nullToken", "NULL" ) ) == "a,b\r\nNULL,\"\"\r\n\"\",NULL\r\n\"NULL\",x\r\nn,\"NULL \"\r\n" );

        // and both read back as they were written
        for ( int tokens = 0; tokens < 2; ++tokens )
        {
            const CString text = WriteText( schema, batch, 0 == tokens ? Writer() : Writer( "nullToken", "NULL" ) );
            CValueList options = CsvOptions();
            CCsvTableCodec::SetSchemaOption( options, *schema );
            if ( 0 != tokens )
                CCsvTableCodec::AddListOption( options, "nullTokens", "NULL" );
            SCsvTestRead read;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( text.C_String() ), options, read ) );
            ASSERT_TRUE( 4 == read.rows.size() );
            ASSERT_TRUE( CsvCell( read, 0, 0 ).IsNULL() && !CsvCell( read, 0, 1 ).IsNULL() && 0 == CsvCell( read, 0, 1 ).ByteSize( false ) );
            ASSERT_TRUE( !CsvCell( read, 1, 0 ).IsNULL() && CsvCell( read, 1, 1 ).IsNULL() );
            ASSERT_TRUE( CsvCell( read, 2, 0 ).AsString() == "NULL" && CsvCell( read, 2, 1 ).AsString() == "x" );
            ASSERT_TRUE( CsvCell( read, 3, 1 ).AsString() == "NULL " );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: every type has one text form: the digits, the dates and timestamps of ISO 8601, true and false, base64" )
    try
    {
        enum { B, I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, DEC, D, TS, TMS, TNS, S, BIN };
        const CDataTableSchemaPtr schema = MakeAllTypesSchema( true );
        CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );

        // a row of ordinary values
        Int32 leapDay = 0;
        ASSERT_TRUE( CDataTableValues::DaysFromCivil( 2024, 2, 29, leapDay ) );
        builder.GetColumn( B ).AppendBool( true );
        builder.GetColumn( I8 ).AppendInt64( -128 );
        builder.GetColumn( I16 ).AppendInt64( -32768 );
        builder.GetColumn( I32 ).AppendInt64( GUCEF_INT32MIN );
        builder.GetColumn( I64 ).AppendInt64( GUCEF_INT64MIN );
        builder.GetColumn( U8 ).AppendUInt64( 255 );
        builder.GetColumn( U16 ).AppendUInt64( 65535 );
        builder.GetColumn( U32 ).AppendUInt64( GUCEF_UINT32MAX );
        builder.GetColumn( U64 ).AppendUInt64( GUCEF_UINT64MAX );
        builder.GetColumn( F32 ).AppendFloat64( (Float64) 0.1f );
        builder.GetColumn( F64 ).AppendFloat64( 0.1 );
        builder.GetColumn( DEC ).AppendDecimal( CDecimal::Parse( "12.5" ) );
        builder.GetColumn( D ).AppendDate( leapDay );
        builder.GetColumn( TS ).AppendTimestamp( 1700000000123456LL );
        builder.GetColumn( TMS ).AppendTimestamp( 1700000000123LL );
        builder.GetColumn( TNS ).AppendTimestamp( 1700000000123456789LL );
        builder.GetColumn( S ).AppendUtf8( "text", 4 );
        const unsigned char bytes[] = { 0, 1, 2, 255 };
        builder.GetColumn( BIN ).AppendBinary( bytes, 4 );

        // a row of nulls
        for ( UInt32 c = 0; c < schema->GetColumnCount(); ++c )
            builder.GetColumn( c ).AppendNull();

        // a row of limits
        builder.GetColumn( B ).AppendBool( false );
        builder.GetColumn( I8 ).AppendInt64( 127 );
        builder.GetColumn( I16 ).AppendInt64( 32767 );
        builder.GetColumn( I32 ).AppendInt64( GUCEF_INT32MAX );
        builder.GetColumn( I64 ).AppendInt64( GUCEF_INT64MAX );
        builder.GetColumn( U8 ).AppendUInt64( 0 );
        builder.GetColumn( U16 ).AppendUInt64( 0 );
        builder.GetColumn( U32 ).AppendUInt64( 0 );
        builder.GetColumn( U64 ).AppendUInt64( 0 );
        builder.GetColumn( F32 ).AppendFloat64( -0.0 );
        builder.GetColumn( F64 ).AppendFloat64( strtod( "nan", GUCEF_NULL ) );
        builder.GetColumn( DEC ).AppendDecimalMantissa( -500 );
        builder.GetColumn( D ).AppendDate( -1 );
        builder.GetColumn( TS ).AppendTimestamp( -1 );
        builder.GetColumn( TMS ).AppendTimestamp( 0 );
        builder.GetColumn( TNS ).AppendTimestamp( -1 );
        builder.GetColumn( S ).AppendUtf8( "", 0 );
        builder.GetColumn( BIN ).AppendBinary( bytes, 0 );

        CDataTableBatchPtr batch;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );

        const CString text = WriteText( schema, batch, Writer() );
        const CString header = "b,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,dec,d,ts,tms,tns,s,bin\r\n";
        const CString row1 = "true,-128,-32768,-2147483648,-9223372036854775808,255,65535,4294967295,18446744073709551615,0.1,0.1,12.5000,2024-02-29,"
                             "2023-11-14T22:13:20.123456Z,2023-11-14T22:13:20.123,2023-11-14T22:13:20.123456789Z,text,AAEC/w==\r\n";
        const CString row2 = ",,,,,,,,,,,,,,,,,\r\n";
        const CString row3 = "false,127,32767,2147483647,9223372036854775807,0,0,0,0,-0,NaN,-0.0500,1969-12-31,"
                             "1969-12-31T23:59:59.999999Z,1970-01-01T00:00:00.000,1969-12-31T23:59:59.999999999Z,\"\",\"\"\r\n";
        ASSERT_TRUE( text == header + row1 + row2 + row3 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: floats are written in the shortest form that reads back as the same value" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "x", DATATABLE_TYPE_FLOAT64, true ) );
        const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
        CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const Float64 infinity = strtod( "inf", GUCEF_NULL );
        const Float64 values[] = { 0.1, 0.1 + 0.2, 1e300, -1e-300, 100000.0, 123456789012345678.0, infinity, -infinity, -0.0, 1.7976931348623157e308, 2.5, -3.0 };
        for ( size_t i = 0; i < sizeof( values ) / sizeof( values[ 0 ] ); ++i )
            builder.GetColumn( 0 ).AppendFloat64( values[ i ] );
        CDataTableBatchPtr batch;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
        const CString text = WriteText( shared, batch, Writer( "lineEnd", "lf" ) );
        ASSERT_TRUE( text == "x\n0.1\n0.30000000000000004\n1e+300\n-1e-300\n100000\n1.2345678901234568e+17\nInfinity\n-Infinity\n-0\n1.7976931348623157e+308\n2.5\n-3\n" );

        // float32 in its own shortest form
        CDataTableSchema schema32;
        schema32.AddColumn( CDataTableColumnDef( "x", DATATABLE_TYPE_FLOAT32, true ) );
        const CDataTableSchemaPtr shared32 = CreateDataTableSchemaPtr( schema32 );
        CDataTableBatchBuilder builder32( shared32, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const Float32 values32[] = { 0.1f, 16777216.0f, 3.4028235e38f, 1.0f / 3.0f };
        for ( size_t i = 0; i < sizeof( values32 ) / sizeof( values32[ 0 ] ); ++i )
            builder32.GetColumn( 0 ).AppendFloat64( (Float64) values32[ i ] );
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder32.Build( batch, error ) );
        ASSERT_TRUE( WriteText( shared32, batch, Writer( "lineEnd", "lf" ) ) == "x\n0.1\n16777216\n3.4028235e+38\n0.33333334\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: decimals are written with exactly the digits of their scale" )
    try
    {
        struct SCase { UInt8 precision; UInt8 scale; Int64 mantissa; const char* text; };
        const SCase cases[] = {
            { 10, 2, 0, "0.00" }, { 10, 2, 5, "0.05" }, { 10, 2, -5, "-0.05" }, { 10, 2, 100, "1.00" }, { 10, 2, -123456, "-1234.56" },
            { 18, 2, 999999999999999999LL, "9999999999999999.99" }, { 18, 2, -999999999999999999LL, "-9999999999999999.99" },
            { 10, 0, 42, "42" }, { 10, 0, -7, "-7" }, { 10, 0, 0, "0" },
            { 18, 18, 1, "0.000000000000000001" }, { 18, 18, -1, "-0.000000000000000001" }, { 18, 18, 999999999999999999LL, "0.999999999999999999" },
            { 18, 18, 0, "0.000000000000000000" }, { 5, 5, 12345, "0.12345" }, { 3, 1, 10, "1.0" }, { 3, 1, 9, "0.9" } };
        for ( size_t i = 0; i < sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
        {
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef::Decimal( "x", cases[ i ].precision, cases[ i ].scale, true ) );
            const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
            CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            ASSERT_TRUE( builder.GetColumn( 0 ).AppendDecimalMantissa( cases[ i ].mantissa ) );
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
            ASSERT_TRUE( WriteText( shared, batch, Writer( "lineEnd", "lf" ) ) == CString( "x\n" ) + cases[ i ].text + "\n" );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: binary is base64 of every length, and the text form is quoted when the delimiter needs it" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "x", DATATABLE_TYPE_BINARY, true ) );
        const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
        CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const unsigned char bytes[] = { 0, 1, 2, 255, 255, 255, 250 };
        const UInt32 offsets[] = { 0, 0, 0, 0, 3 };
        const UInt32 lengths[] = { 0, 1, 2, 3, 4 };
        for ( size_t i = 0; i < sizeof( lengths ) / sizeof( lengths[ 0 ] ); ++i )
            builder.GetColumn( 0 ).AppendBinary( bytes + offsets[ i ], lengths[ i ] );
        CDataTableBatchPtr batch;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
        ASSERT_TRUE( WriteText( shared, batch, Writer( "lineEnd", "lf" ) ) == "x\n\"\"\nAA==\nAAE=\nAAEC\n////+g==\n" );
        ASSERT_TRUE( WriteText( shared, batch, Writer( "delimiter", "/" ) ) == "x\r\n\"\"\r\nAA==\r\nAAE=\r\nAAEC\r\n\"////+g==\"\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: only the selected rows of a batch are written, and the summary says what was written" )
    try
    {
        const CDataTableSchemaPtr schema = TwoTexts();
        const char* cells[] = { "r0", "x", "r1", GUCEF_NULL, "r2", "z", "r3", GUCEF_NULL };
        CDataTableBatchPtr batch = TextBatch( schema, cells, 4 );

        CDataTableBufferPtr owner;
        ASSERT_TRUE( DATATABLE_STATUS_OK == CCoreGlobal::Instance()->GetDataTableBufferPool().Acquire( 16, owner ) );
        UInt32* selection = static_cast< UInt32* >( owner->GetData() );
        selection[ 0 ] = 1;
        selection[ 1 ] = 3;
        owner->SetDataSize( 8 );
        batch.GetWritable()->SetSelection( selection, 2, owner );
        ASSERT_TRUE( 2 == batch->GetSelectedRowCount() );

        CCsvMemoryFile file;
        CString error;
        CDataTableFileSummary summary;
        ASSERT_TRUE( DATATABLE_STATUS_OK == WriteCsvBatch( schema, *batch, Writer(), file, error, &summary ) );
        ASSERT_TRUE( file.Text() == "a,b\r\nr1,\r\nr3,\r\n" );
        ASSERT_TRUE( 2 == summary.rowCount && 1 == summary.rowGroupCount );
        ASSERT_TRUE( file.Size() == summary.byteCount );
        ASSERT_TRUE( 2 == summary.columnStats.size() );
        ASSERT_TRUE( summary.columnStats[ 0 ].hasNullCount && 0 == summary.columnStats[ 0 ].nullCount && 2 == summary.columnStats[ 0 ].valueCount );
        ASSERT_TRUE( summary.columnStats[ 1 ].hasNullCount && 2 == summary.columnStats[ 1 ].nullCount );
        ASSERT_FALSE( summary.columnStats[ 1 ].hasMinMax );

        // several batches make one file
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );
        CCsvMemoryFile several;
        CITableWriterPtr writer;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( several.Access(), schema, Writer(), writer, error ) );
        const char* first[] = { "1", "2" };
        const char* second[] = { "3", "4", "5", "6" };
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *TextBatch( schema, first, 1 ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->FlushRowGroup() );
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *TextBatch( schema, second, 2 ) ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->Finish( summary ) );
        ASSERT_TRUE( several.Text() == "a,b\r\n1,2\r\n3,4\r\n5,6\r\n" );
        ASSERT_TRUE( 3 == summary.rowCount );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: misuse is refused with a status: a batch of another shape, writing after the end, no columns" )
    try
    {
        const CDataTableSchemaPtr schema = TwoTexts();
        const char* cells[] = { "1", "2" };
        const CDataTableBatchPtr batch = TextBatch( schema, cells, 1 );
        CTableCodecRegistry::TTableCodecPtr codec;
        ASSERT_TRUE( GetCsvCodec( codec ) );
        CString error;

        // a batch of another column count and of other column types
        CDataTableSchema narrow;
        narrow.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_UTF8 ) );
        const CDataTableSchemaPtr narrowSchema = CreateDataTableSchemaPtr( narrow );
        const char* one[] = { "1" };
        CCsvMemoryFile file;
        CITableWriterPtr writer;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( file.Access(), schema, Writer(), writer, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == writer->WriteBatch( *TextBatch( narrowSchema, one, 1 ) ) );
        ASSERT_FALSE( writer->GetLastErrorMessage().IsNULLOrEmpty() );

        CDataTableSchema ints;
        ints.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64 ) );
        ints.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_INT64 ) );
        const CDataTableSchemaPtr intSchema = CreateDataTableSchemaPtr( ints );
        CDataTableBatchBuilder intBuilder( intSchema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        intBuilder.GetColumn( 0 ).AppendInt64( 1 );
        intBuilder.GetColumn( 1 ).AppendInt64( 2 );
        CDataTableBatchPtr intBatch;
        ASSERT_TRUE( DATATABLE_STATUS_OK == intBuilder.Build( intBatch, error ) );
        CCsvMemoryFile other;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( other.Access(), schema, Writer(), writer, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == writer->WriteBatch( *intBatch ) );

        // after Finish
        CCsvMemoryFile done;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( done.Access(), schema, Writer(), writer, error ) );
        CDataTableFileSummary summary;
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->Finish( summary ) );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == writer->WriteBatch( *batch ) );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == writer->Finish( summary ) );
        ASSERT_TRUE( done.Text() == "a,b\r\n" );

        // Abort stops the output: nothing is written after it
        CCsvMemoryFile aborted;
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( aborted.Access(), schema, Writer(), writer, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *batch ) );
        writer->Abort();
        ASSERT_TRUE( DATATABLE_STATUS_OK != writer->WriteBatch( *batch ) );
        ASSERT_TRUE( DATATABLE_STATUS_OK != writer->Finish( summary ) );
        ASSERT_TRUE( aborted.Text() == "a,b\r\n" );      // the header went out when the writer started, the row was still buffered

        // a writer that is dropped without Finish does no harm
        {
            CCsvMemoryFile dropped;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( dropped.Access(), schema, Writer(), writer, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *batch ) );
            writer.Unlink();
        }

        // no destination, no schema, no columns
        CITableWriterPtr none;
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == codec->CreateWriter( IOAccessPtr(), schema, Writer(), none, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == codec->CreateWriter( file.Access(), CDataTableSchemaPtr(), Writer(), none, error ) );
        CDataTableSchema empty;
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == codec->CreateWriter( file.Access(), CreateDataTableSchemaPtr( empty ), Writer(), none, error ) );
        ASSERT_TRUE( none.IsNULL() );

        // a destination that takes nothing
        CDynamicBuffer fixedBuffer( 4, false );
        fixedBuffer.SetDataSize( 0 );
        CDynamicBufferAccess* fixedAccess = GUCEF_NEW CDynamicBufferAccess( &fixedBuffer, false );
        IOAccessPtr tiny = fixedAccess->CreateSharedPtr();
        ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( tiny, schema, Writer(), writer, error ) );
        TDataTableStatus status = writer->WriteBatch( *batch );
        if ( DATATABLE_STATUS_OK == status )
            status = writer->Finish( summary );
        ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == status || DATATABLE_STATUS_OK == status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecWriter: a file that is read and written again keeps its line ends, byte order mark and delimiter" )
    try
    {
        struct SCase { const char* file; const char* delimiter; };
        const SCase cases[] = {
            { "a,b\r\n\"x\ny\",2\r\n3,4\r\n", "," },
            { "a;b\n1;\"q;r\"\n3;4\n", ";" },
            { "a,b\r1,2\r3,4\r", "," },
            { "\xEF\xBB\xBF" "a|b\r\n1|2\r\n", "|" },
            { "a,b\r\n\"line1\r\nline2\",\"\"\r\n", "," },
            { "a\tb\n\"cr\rx\"\t2\n", "\\t" } };
        for ( size_t i = 0; i < sizeof( cases ) / sizeof( cases[ 0 ] ); ++i )
        {
            CValueList options = AllTextOptionsFor( 2 );
            SetCsvOption( options, "delimiter", cases[ i ].delimiter );
            SCsvTestRead read;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( cases[ i ].file ), options, read ) );

            CDataTableBatchPtr batch;
            ASSERT_TRUE( BuildCsvTestBatch( read.schema, read.rows, batch ) );

            // what the reader reported goes into the writer options as they are
            CDataTableWriterOptions writerOptions = CsvWriterOptions();
            writerOptions.keyValueMetaData = read.keyValues;
            const CString written = WriteText( read.schema, batch, writerOptions );
            ASSERT_TRUE( written == cases[ i ].file );
        }

        // what the options say wins over what the file said
        CValueList options = AllTextOptionsFor( 2 );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "\xEF\xBB\xBF" "a,b\r\n1,2\r\n" ), options, read ) );
        CDataTableBatchPtr batch;
        ASSERT_TRUE( BuildCsvTestBatch( read.schema, read.rows, batch ) );
        CDataTableWriterOptions writerOptions = CsvWriterOptions();
        writerOptions.keyValueMetaData = read.keyValues;
        SetCsvWriterOption( writerOptions, "lineEnd", "lf" );
        SetCsvWriterOption( writerOptions, "bom", "false" );
        SetCsvWriterOption( writerOptions, "delimiter", ";" );
        ASSERT_TRUE( WriteText( read.schema, batch, writerOptions ) == "a;b\n1;2\n" );

        // a file of one line has no line end to keep
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a,b" ), options, read ) );
        ASSERT_TRUE( read.keyValues.GetValueAlways( "lineEnding" ).AsString() == "none" );
        CDataTableWriterOptions none = CsvWriterOptions();
        none.keyValueMetaData = read.keyValues;
        const char* cells[] = { "1", "2" };
        const CDataTableSchemaPtr schema = TwoTexts();
        ASSERT_TRUE( WriteText( schema, TextBatch( schema, cells, 1 ), none ) == "a,b\r\n1,2\r\n" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
