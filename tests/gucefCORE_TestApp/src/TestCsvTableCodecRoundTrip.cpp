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
#include <stdlib.h>

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CSTOPWATCH_H
#include "gucefCORE_CStopwatch.h"
#define GUCEF_CORE_CSTOPWATCH_H
#endif /* GUCEF_CORE_CSTOPWATCH_H ? */

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
#include "TestCsvTableCodecRoundTrip.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records a failure with a text of our own */
#define FAIL_WITH( text ) \
    ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( ( text ).C_String(), __FILE__, __LINE__ )

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace CORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TextPieces[] = {
    "", "a", "word", "two words", " leading", "trailing ", "\t", ",", ";", "|", "\"", "\"\"", "'", "\r", "\n", "\r\n", "line1\nline2", "a,b,c", "say \"hi\"",
    "NULL", "N/A", "\\N", "1", "2024-01-02", "true", "\xC3\xA9\xC3\xA8", "\xE2\x82\xAC", "\xF0\x9F\x98\x80", "\xEF\xBB\xBF", "tab\there", "back\\slash", "quote'apostrophe" };

/*-------------------------------------------------------------------------*/

static CString
RandomText( CCsvTestRandom& random )
{
    CString text;
    const UInt32 pieces = random.Below( 6 );
    for ( UInt32 i = 0; i < pieces; ++i )
    {
        // appending the empty string to a CString counts one byte too many, so the empty piece is not appended
        const char* piece = TextPieces[ random.Below( sizeof( TextPieces ) / sizeof( TextPieces[ 0 ] ) ) ];
        if ( '\0' != piece[ 0 ] )
            text += piece;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

static Int64
Pow10( UInt32 n )
{
    Int64 result = 1;
    for ( UInt32 i = 0; i < n; ++i )
        result *= 10;
    return result;
}

/*-------------------------------------------------------------------------*/

static void
AppendRandom( CDataTableColumnBuilder& builder, const CDataTableColumnDef& def, CCsvTestRandom& random )
{
    if ( def.nullable && random.Coin( 5 ) )
    {
        builder.AppendNull();
        return;
    }

    switch ( def.type )
    {
        case DATATABLE_TYPE_BOOL: builder.AppendBool( random.Coin( 2 ) ); break;
        case DATATABLE_TYPE_INT8: builder.AppendInt64( (Int8) random.Next() ); break;
        case DATATABLE_TYPE_INT16: builder.AppendInt64( (Int16) random.Next() ); break;
        case DATATABLE_TYPE_INT32: builder.AppendInt64( (Int32) random.Next() ); break;
        case DATATABLE_TYPE_INT64:
        {
            const Int64 specials[] = { GUCEF_INT64MIN, GUCEF_INT64MAX, 0, -1, 1 };
            builder.AppendInt64( random.Coin( 8 ) ? specials[ random.Below( 5 ) ] : (Int64) random.Next() );
            break;
        }
        case DATATABLE_TYPE_UINT8: builder.AppendUInt64( (UInt8) random.Next() ); break;
        case DATATABLE_TYPE_UINT16: builder.AppendUInt64( (UInt16) random.Next() ); break;
        case DATATABLE_TYPE_UINT32: builder.AppendUInt64( (UInt32) random.Next() ); break;
        case DATATABLE_TYPE_UINT64: builder.AppendUInt64( random.Coin( 8 ) ? GUCEF_UINT64MAX : random.Next() ); break;
        case DATATABLE_TYPE_FLOAT32:
        {
            const UInt32 bits = (UInt32) random.Next();
            Float32 value;
            memcpy( &value, &bits, sizeof( value ) );
            builder.AppendFloat64( (Float64) value );
            break;
        }
        case DATATABLE_TYPE_FLOAT64:
        {
            const UInt64 bits = random.Next();
            Float64 value;
            memcpy( &value, &bits, sizeof( value ) );
            if ( random.Coin( 10 ) )
            {
                const Float64 specials[] = { 0.0, -0.0, 1.0, -1.0, 0.1, 5e-324, 1.7976931348623157e308, strtod( "inf", GUCEF_NULL ), -strtod( "inf", GUCEF_NULL ), strtod( "nan", GUCEF_NULL ) };
                value = specials[ random.Below( 10 ) ];
            }
            builder.AppendFloat64( value );
            break;
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            const Int64 limit = Pow10( def.decimalPrecision ) - 1;
            const UInt64 span = (UInt64) limit * 2 + 1;
            builder.AppendDecimalMantissa( (Int64) ( random.Next() % span ) - limit );
            break;
        }
        case DATATABLE_TYPE_DATE:
            builder.AppendDate( random.Coin( 4 ) ? (Int32) random.Next() : (Int32) ( random.Below( 200000 ) ) - 100000 );
            break;
        case DATATABLE_TYPE_TIMESTAMP:
            builder.AppendTimestamp( random.Coin( 4 ) ? (Int64) random.Next() : (Int64) ( random.Next() % 4000000000000000LL ) - 2000000000000000LL );
            break;
        case DATATABLE_TYPE_UTF8:
        {
            const CString text = RandomText( random );
            builder.AppendUtf8( text.C_String(), (UInt32) strlen( text.C_String() ) );
            break;
        }
        case DATATABLE_TYPE_BINARY:
        {
            unsigned char bytes[ 24 ];
            const UInt32 length = random.Below( 24 );
            for ( UInt32 i = 0; i < length; ++i )
                bytes[ i ] = (unsigned char) random.Next();
            builder.AppendBinary( bytes, length );
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

static CDataTableColumnDef
RandomColumn( CCsvTestRandom& random, UInt32 index )
{
    static const TDataTableLogicalType types[] = {
        DATATABLE_TYPE_BOOL, DATATABLE_TYPE_INT8, DATATABLE_TYPE_INT16, DATATABLE_TYPE_INT32, DATATABLE_TYPE_INT64, DATATABLE_TYPE_UINT8, DATATABLE_TYPE_UINT16,
        DATATABLE_TYPE_UINT32, DATATABLE_TYPE_UINT64, DATATABLE_TYPE_FLOAT32, DATATABLE_TYPE_FLOAT64, DATATABLE_TYPE_DECIMAL, DATATABLE_TYPE_DATE,
        DATATABLE_TYPE_TIMESTAMP, DATATABLE_TYPE_UTF8, DATATABLE_TYPE_BINARY };
    const TDataTableLogicalType type = types[ random.Below( sizeof( types ) / sizeof( types[ 0 ] ) ) ];
    const CString name = CString( "c" ) + UInt32ToString( index );
    const bool nullable = random.Coin( 3 ) ? false : true;
    if ( DATATABLE_TYPE_DECIMAL == type )
    {
        const UInt8 precision = (UInt8) ( 1 + random.Below( 18 ) );
        return CDataTableColumnDef::Decimal( name, precision, (UInt8) random.Below( precision + 1 ), nullable );
    }
    if ( DATATABLE_TYPE_TIMESTAMP == type )
        return CDataTableColumnDef::Timestamp( name, (TDataTableTimeUnit) random.Below( 3 ), random.Coin( 2 ), nullable );
    return CDataTableColumnDef( name, type, nullable );
}

/*-------------------------------------------------------------------------*/

/** The rows of a batch as cells that own their memory */
static void
CellsOf( const CDataTableBatch& batch, TCsvTestRows& rows )
{
    rows.clear();
    for ( UInt32 r = 0; r < batch.GetRowCount(); ++r )
    {
        TCsvTestRow cells;
        for ( UInt32 c = 0; c < batch.GetColumnCount(); ++c )
        {
            CVariant value;
            batch.GetValue( r, c, value, false );
            cells.push_back( value );
        }
        rows.push_back( cells );
    }
}

/*-------------------------------------------------------------------------*/

/** What is different between two sets of rows, "" when none */
static CString
Difference( const TCsvTestRows& expected, const TCsvTestRows& actual, const CDataTableSchemaPtr& schema )
{
    if ( expected.size() != actual.size() )
        return CString( "row count " ) + UInt32ToString( (UInt32) actual.size() ) + " and not " + UInt32ToString( (UInt32) expected.size() );
    for ( size_t r = 0; r < expected.size(); ++r )
    {
        for ( size_t c = 0; c < expected[ r ].size(); ++c )
        {
            if ( c >= actual[ r ].size() || !CsvSameCell( expected[ r ][ c ], actual[ r ][ c ] ) )
            {
                return CString( "row " ) + UInt32ToString( (UInt32) r ) + " column " + schema->GetColumn( (UInt32) c ).name + " (" + schema->GetColumn( (UInt32) c ).GetTypeText() + ") is [" +
                       ( c < actual[ r ].size() ? CsvCellText( actual[ r ][ c ] ) : CString( "missing" ) ) + "] and not [" + CsvCellText( expected[ r ][ c ] ) + "]";
            }
        }
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

/** Writes the batch with random choices of the writer, reads it back with the matching choices of the reader, and tells what differs */
static CString
RoundTrip( UInt32 seed, const CDataTableSchemaPtr& schema, const CDataTableBatchPtr& batch, bool useSchemaOption )
{
    CCsvTestRandom random( seed * 7919 + 13 );

    const char* const delimiters[] = { ",", ";", "\t", "|" };
    const char* const lineEnds[] = { "crlf", "lf", "cr" };
    const char* const delimiter = delimiters[ random.Below( 4 ) ];
    const char* const lineEnd = lineEnds[ random.Below( 3 ) ];
    const bool header = random.Coin( 4 ) ? false : true;
    const bool bom = random.Coin( 4 );
    const bool apostropheQuote = random.Coin( 6 );
    const char* const nullToken = random.Coin( 4 ) ? "NULL" : "";

    CDataTableWriterOptions writerOptions = CsvWriterOptions();
    SetCsvWriterOption( writerOptions, "delimiter", delimiter );
    SetCsvWriterOption( writerOptions, "lineEnd", lineEnd );
    SetCsvWriterOption( writerOptions, "header", header ? "true" : "false" );
    SetCsvWriterOption( writerOptions, "bom", bom ? "true" : "false" );
    SetCsvWriterOption( writerOptions, "nullToken", nullToken );
    if ( apostropheQuote )
        SetCsvWriterOption( writerOptions, "quote", "'" );

    CCsvMemoryFile file;
    CString error;
    TDataTableStatus status = WriteCsvBatch( schema, *batch, writerOptions, file, error );

    if ( DATATABLE_STATUS_OK != status )
        return CString( "write failed: " ) + error;

    CValueList options = CsvOptions();
    SetCsvOption( options, "delimiter", delimiter );
    SetCsvOption( options, "hasHeader", header ? "true" : "false" );
    if ( '\0' != nullToken[ 0 ] )
        CCsvTableCodec::AddListOption( options, "nullTokens", nullToken );
    if ( apostropheQuote )
        SetCsvOption( options, "quote", "'" );
    if ( useSchemaOption )
        CCsvTableCodec::SetSchemaOption( options, *schema );

    const UInt32 batchSizes[] = { 1, 3, 1000 };
    SCsvTestRead read;
    status = ReadCsvSource( file.Access(), options, read, batchSizes[ random.Below( 3 ) ], random.Coin( 2 ) );
    if ( DATATABLE_STATUS_OK != status )
    {
        return CString( "read failed: " ) + read.error + " in [" + file.Text() + "]";
    }

    TCsvTestRows expected;
    CellsOf( *batch, expected );
    CString difference = Difference( expected, read.rows, schema );
    if ( !difference.IsNULLOrEmpty() )
        difference += CString( " in [" ) + file.Text() + "]";
    return difference;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCsvTableCodecRoundTripTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.CsvTableCodecRoundTrip" );

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: random tables of every type written and read back are the same, whatever delimiter, line end, quote and null token" )
    try
    {
        for ( UInt32 seed = 1; seed <= 300; ++seed )
        {
            CCsvTestRandom random( seed );
            CDataTableSchema schema;
            const UInt32 columns = 1 + random.Below( 8 );
            for ( UInt32 c = 0; c < columns; ++c )
                schema.AddColumn( RandomColumn( random, c ) );
            const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );

            CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            const UInt32 rows = random.Coin( 10 ) ? 0 : random.Below( 80 );
            for ( UInt32 r = 0; r < rows; ++r )
                for ( UInt32 c = 0; c < columns; ++c )
                    AppendRandom( builder.GetColumn( c ), shared->GetColumn( c ), random );
            CDataTableBatchPtr batch;
            CString error;
            if ( DATATABLE_STATUS_OK != builder.Build( batch, error ) )
            {
                FAIL_WITH( CString( "the batch could not be built: " ) + error );
                continue;
            }

            const CString difference = RoundTrip( seed, shared, batch, true );
            if ( !difference.IsNULLOrEmpty() )
            {
                FAIL_WITH( CString( "seed " ) + UInt32ToString( seed ) + ": " + difference );
                break;
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: one table with every type, with and without nulls, is the same after a round trip" )
    try
    {
        for ( int nullable = 0; nullable < 2; ++nullable )
        {
            const CDataTableSchemaPtr schema = MakeAllTypesSchema( 0 != nullable );
            CCsvTestRandom random( 99 + nullable );
            CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            for ( UInt32 r = 0; r < 500; ++r )
                for ( UInt32 c = 0; c < schema->GetColumnCount(); ++c )
                    AppendRandom( builder.GetColumn( c ), schema->GetColumn( c ), random );
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
            const CString difference = RoundTrip( 1000 + nullable, schema, batch, true );
            if ( !difference.IsNULLOrEmpty() )
                FAIL_WITH( difference );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: floats of every bit pattern come back as the same value" )
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32, false ) );
        schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64, false ) );
        const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
        CCsvTestRandom random( 4242 );
        CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
        for ( UInt32 i = 0; i < 20000; ++i )
        {
            AppendRandom( builder.GetColumn( 0 ), shared->GetColumn( 0 ), random );
            AppendRandom( builder.GetColumn( 1 ), shared->GetColumn( 1 ), random );
        }
        // denormals, the largest and the smallest values and the neighbours of 1
        const Float64 hard[] = { 4.9406564584124654e-324, 2.2250738585072014e-308, 2.2250738585072009e-308, 1.7976931348623157e308, 1.0000000000000002, 0.9999999999999999,
                                 9007199254740993.0, 0.30000000000000004, 1e22, 1e23, 8.41e21, 5e-324, 123456789012345680000.0 };
        for ( size_t i = 0; i < sizeof( hard ) / sizeof( hard[ 0 ] ); ++i )
        {
            builder.GetColumn( 0 ).AppendFloat64( (Float64) (Float32) hard[ i ] > 3.4e38 ? 0.0 : (Float64) (Float32) hard[ i ] );
            builder.GetColumn( 1 ).AppendFloat64( hard[ i ] );
        }
        CDataTableBatchPtr batch;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
        const CString difference = RoundTrip( 2024, shared, batch, true );
        if ( !difference.IsNULLOrEmpty() )
            FAIL_WITH( difference );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: tables that can be told from their values come back with the same types and values when nothing is declared" )
    try
    {
        for ( UInt32 seed = 1; seed <= 150; ++seed )
        {
            CCsvTestRandom random( seed + 5000 );
            CDataTableSchema schema;
            const UInt32 columns = 1 + random.Below( 6 );
            for ( UInt32 c = 0; c < columns; ++c )
            {
                const CString name = CString( "c" ) + UInt32ToString( c );
                switch ( random.Below( 5 ) )
                {
                    case 0: schema.AddColumn( CDataTableColumnDef( name, DATATABLE_TYPE_INT64, false ) ); break;
                    case 1: schema.AddColumn( CDataTableColumnDef::Decimal( name, 18, (UInt8) ( 1 + random.Below( 9 ) ), false ) ); break;
                    case 2: schema.AddColumn( CDataTableColumnDef( name, DATATABLE_TYPE_BOOL, false ) ); break;
                    case 3: schema.AddColumn( CDataTableColumnDef( name, DATATABLE_TYPE_DATE, false ) ); break;
                    default: schema.AddColumn( CDataTableColumnDef::Timestamp( name, DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) ); break;
                }
            }
            const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
            CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            const UInt32 rows = 1 + random.Below( 40 );
            for ( UInt32 r = 0; r < rows; ++r )
            {
                for ( UInt32 c = 0; c < columns; ++c )
                {
                    const CDataTableColumnDef& def = shared->GetColumn( c );
                    if ( DATATABLE_TYPE_DECIMAL == def.type )
                    {
                        // values with the digits of the scale and a point in them; inference sees the precision of the largest
                        builder.GetColumn( c ).AppendDecimalMantissa( ( (Int64) ( random.Next() % 2000000000ULL ) - 1000000000LL ) + ( 0 == r ? Pow10( def.decimalScale ) : 0 ) );
                    }
                    else
                    {
                        AppendRandom( builder.GetColumn( c ), def, random );
                    }
                }
            }
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );

            CCsvMemoryFile file;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteCsvBatch( shared, *batch, CsvWriterOptions(), file, error ) );

            SCsvTestRead read;
            const TDataTableStatus status = ReadCsvSource( file.Access(), CsvOptions(), read );
            if ( DATATABLE_STATUS_OK != status )
            {
                FAIL_WITH( CString( "seed " ) + UInt32ToString( seed ) + " read failed: " + read.error + " in [" + file.Text() + "]" );
                break;
            }

            for ( UInt32 c = 0; c < columns; ++c )
            {
                const CDataTableColumnDef& expected = shared->GetColumn( c );
                const CDataTableColumnDef& found = CsvColumnOf( read, c );
                bool sameKind = expected.type == found.type;
                if ( sameKind && DATATABLE_TYPE_DECIMAL == expected.type )
                    sameKind = expected.decimalScale == found.decimalScale;
                if ( !sameKind )
                {
                    FAIL_WITH( CString( "seed " ) + UInt32ToString( seed ) + " column " + UInt32ToString( c ) + " is " + found.GetTypeText() + " and not " + expected.GetTypeText() + " in [" + file.Text() + "]" );
                    break;
                }
            }

            TCsvTestRows expectedRows;
            CellsOf( *batch, expectedRows );
            if ( expectedRows.size() != read.rows.size() )
            {
                FAIL_WITH( CString( "seed " ) + UInt32ToString( seed ) + " row count" );
                break;
            }
            for ( size_t r = 0; r < expectedRows.size(); ++r )
            {
                for ( size_t c = 0; c < expectedRows[ r ].size(); ++c )
                {
                    if ( expectedRows[ r ][ c ].AsString() != CsvCell( read, r, c ).AsString() )
                    {
                        FAIL_WITH( CString( "seed " ) + UInt32ToString( seed ) + " row " + UInt32ToString( (UInt32) r ) + " column " + UInt32ToString( (UInt32) c ) + " is [" +
                                   CsvCell( read, r, c ).AsString() + "] and not [" + expectedRows[ r ][ c ].AsString() + "]" );
                        r = expectedRows.size();
                        break;
                    }
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: input that is cut short or damaged is read or refused, never more than that" )
    try
    {
        const char* valid =
            "id,name,amount,when,flag\r\n"
            "1,\"Smith, John\",12.50,2024-01-31,true\r\n"
            "2,\"say \"\"hi\"\"\",-0.25,2023-12-01,false\r\n"
            "3,\"line1\nline2\",,2024-02-29,true\r\n"
            "4,\xC3\xA9t\xC3\xA9,1000000.00,1969-12-31,false\r\n";
        const UInt32 length = (UInt32) strlen( valid );

        // cut at every length
        for ( UInt32 cut = 0; cut <= length; ++cut )
        {
            SCsvTestRead read;
            const TDataTableStatus status = ReadCsvSource( MakeCsvSource( valid, cut ), CsvOptions(), read );
            const bool acceptable = DATATABLE_STATUS_OK == status || DATATABLE_STATUS_CORRUPT_DATA == status || DATATABLE_STATUS_TYPE_MISMATCH == status;
            if ( !acceptable )
            {
                FAIL_WITH( CString( "cut at " ) + UInt32ToString( cut ) + " gave status " + DataTableStatusToString( status ) + ": " + read.error );
                break;
            }
        }

        // a byte replaced by every value, at every place
        for ( UInt32 place = 0; place < length; ++place )
        {
            for ( UInt32 value = 0; value < 256; value += 51 )
            {
                vector< char > damaged( valid, valid + length );
                damaged[ place ] = (char) value;
                SCsvTestRead read;
                const TDataTableStatus status = ReadCsvSource( MakeCsvSource( damaged ), CsvOptions(), read, 2 );
                const bool acceptable = DATATABLE_STATUS_OK == status || DATATABLE_STATUS_CORRUPT_DATA == status || DATATABLE_STATUS_TYPE_MISMATCH == status ||
                                        DATATABLE_STATUS_UNSUPPORTED == status || DATATABLE_STATUS_SCHEMA_MISMATCH == status;
                if ( !acceptable )
                {
                    FAIL_WITH( CString( "byte " ) + UInt32ToString( place ) + " set to " + UInt32ToString( value ) + " gave status " + DataTableStatusToString( status ) + ": " + read.error );
                    place = length;
                    break;
                }
            }
        }

        // random bytes, and random bytes with the characters that matter
        CCsvTestRandom random( 777 );
        const char interesting[] = { '"', ',', '\r', '\n', '\0', ' ', '\t', '1', '.', '-', 'e', ':', 'T', 'Z', '\xC3', '\xBF', '\xEF', '\xBB' };
        for ( UInt32 trial = 0; trial < 3000; ++trial )
        {
            vector< char > bytes;
            const UInt32 count = random.Below( 200 );
            for ( UInt32 i = 0; i < count; ++i )
                bytes.push_back( random.Coin( 2 ) ? interesting[ random.Below( sizeof( interesting ) ) ] : (char) random.Next() );
            CValueList options = CsvOptions();
            if ( 0 == trial % 3 ) SetCsvOption( options, "hasHeader", "false" );
            if ( 1 == trial % 3 ) SetCsvOption( options, "inferRowCount", "2" );
            SCsvTestRead read;
            const TDataTableStatus status = ReadCsvSource( MakeCsvSource( bytes ), options, read, 3, 0 == trial % 2 );
            const bool acceptable = DATATABLE_STATUS_OK == status || DATATABLE_STATUS_CORRUPT_DATA == status || DATATABLE_STATUS_TYPE_MISMATCH == status ||
                                    DATATABLE_STATUS_UNSUPPORTED == status || DATATABLE_STATUS_SCHEMA_MISMATCH == status || DATATABLE_STATUS_LIMIT_EXCEEDED == status;
            if ( !acceptable )
            {
                FAIL_WITH( CString( "random input " ) + UInt32ToString( trial ) + " gave status " + DataTableStatusToString( status ) + ": " + read.error );
                break;
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecRoundTrip: a file of a million rows is written and read in batches (set GUCEF_TEST_CSV_BIG to run it)" )
    GUCEF_TESTFW_REQUIRE_ENV( big, "GUCEF_TEST_CSV_BIG" );
    try
    {
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
        schema.AddColumn( CDataTableColumnDef::Decimal( "price", 18, 4, false ) );
        schema.AddColumn( CDataTableColumnDef( "day", DATATABLE_TYPE_DATE, false ) );
        schema.AddColumn( CDataTableColumnDef( "note", DATATABLE_TYPE_UTF8, true ) );
        const CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );

        CString path = RelativePath( "$CURWORKDIR$" );
        AppendToPath( path, "gucefCORE_TestApp_csv_big.csv" );

        const UInt32 total = 1000000;
        CStopwatch stopwatch;
        stopwatch.Start();
        {
            IOAccessPtr destination( GUCEF_NEW CFileAccess( path, "wb" ) );
            ASSERT_TRUE( destination->IsValid() );
            CTableCodecRegistry::TTableCodecPtr codec;
            ASSERT_TRUE( GetCsvCodec( codec ) );
            CITableWriterPtr writer;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateWriter( destination, shared, CsvWriterOptions(), writer, error ) );

            CDataTableBatchBuilder builder( shared, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            for ( UInt32 i = 0; i < total; ++i )
            {
                builder.GetColumn( 0 ).AppendInt64( i );
                builder.GetColumn( 1 ).AppendDecimalMantissa( (Int64) i * 7 - 3000 );
                builder.GetColumn( 2 ).AppendDate( (Int32) ( i % 20000 ) );
                if ( 0 == i % 10 )
                    builder.GetColumn( 3 ).AppendNull();
                else
                    builder.GetColumn( 3 ).AppendUtf8( 0 == i % 7 ? "needs, quotes" : "plain text", 0 == i % 7 ? 13 : 10 );
                if ( 49999 == i % 50000 )
                {
                    CDataTableBatchPtr batch;
                    ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == writer->WriteBatch( *batch ) );
                }
            }
            CDataTableFileSummary summary;
            ASSERT_TRUE( DATATABLE_STATUS_OK == writer->Finish( summary ) );
            ASSERT_TRUE( total == summary.rowCount );
        }
        const Float64 writeSeconds = (Float64) stopwatch.GetElapsedMilliseconds() / 1000.0;

        stopwatch.Reset();
        stopwatch.Start();
        {
            IOAccessPtr source( GUCEF_NEW CFileAccess( path, "rb" ) );
            ASSERT_TRUE( source->IsValid() );
            CTableCodecRegistry::TTableCodecPtr codec;
            ASSERT_TRUE( GetCsvCodec( codec ) );
            CString error;
            CITableFileMetaDataPtr metaData;
            CValueList options = CsvOptions();
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( *source, options, metaData, error ) );
            ASSERT_TRUE( 4 == metaData->GetSchema()->GetColumnCount() );
            ASSERT_TRUE( DATATABLE_TYPE_DECIMAL == metaData->GetSchema()->GetColumn( 1 ).type && 4 == metaData->GetSchema()->GetColumn( 1 ).decimalScale );
            ASSERT_TRUE( DATATABLE_TYPE_DATE == metaData->GetSchema()->GetColumn( 2 ).type );

            CITableReaderPtr reader;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->CreateReader( source, metaData, options, reader, error ) );
            CDataTableBufferPool pool;
            CDataTableReadRequest request;
            request.batchRowCount = 16384;
            request.bufferPool = &pool;
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );

            UInt64 rows = 0, idSum = 0, peak = 0;
            for ( ;; )
            {
                CDataTableBatchPtr batch;
                const TDataTableStatus status = reader->ReadNextBatch( batch );
                if ( DATATABLE_STATUS_END == status )
                    break;
                ASSERT_TRUE( DATATABLE_STATUS_OK == status );
                const Int64* ids = batch->GetColumn( 0 ).GetInt64Data();
                ASSERT_TRUE( GUCEF_NULL != ids );
                for ( UInt32 r = 0; r < batch->GetRowCount(); ++r )
                    idSum += (UInt64) ids[ r ];
                rows += batch->GetRowCount();
                CDataTableBufferPoolStats stats;
                pool.GetStats( stats );
                if ( stats.bytesInUse > peak )
                    peak = stats.bytesInUse;
            }
            ASSERT_TRUE( total == rows );
            ASSERT_TRUE( (UInt64) total * ( total - 1 ) / 2 == idSum );
            ASSERT_TRUE( peak < 16 * 1024 * 1024 );

            const Float64 readSeconds = (Float64) stopwatch.GetElapsedMilliseconds() / 1000.0;
            GUCEF_LOG( LOGLEVEL_NORMAL, "CSV of a million rows: written in " + DoubleToString( writeSeconds ) + " s, read in " + DoubleToString( readSeconds ) + " s, peak pool use " + UInt64ToString( peak ) + " bytes" );
        }
        DeleteFile( path );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
