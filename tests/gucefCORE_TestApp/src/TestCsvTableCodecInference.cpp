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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCsvTableCodecSupport.h"
#include "TestCsvTableCodecInference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the difference of two texts as the failure, which names both */
#define CHECK_TEXT( actual, expected ) \
    do { \
        const GUCEF::CORE::CString checkActual_ = ( actual ); \
        const GUCEF::CORE::CString checkExpected_ = ( expected ); \
        if ( checkActual_ != checkExpected_ ) \
        { \
            const GUCEF::CORE::CString checkMessage_ = GUCEF::CORE::CString( #actual " is [" ) + checkActual_ + "] and not [" + checkExpected_ + "]"; \
            ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( checkMessage_.C_String(), __FILE__, __LINE__ ); \
        } \
    } while ( 0 )

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace CORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CValueList
Opts( const char* key = GUCEF_NULL, const char* value = GUCEF_NULL, const char* key2 = GUCEF_NULL, const char* value2 = GUCEF_NULL, const char* key3 = GUCEF_NULL, const char* value3 = GUCEF_NULL )
{
    CValueList options = CsvOptions();
    if ( GUCEF_NULL != key ) SetCsvOption( options, key, value );
    if ( GUCEF_NULL != key2 ) SetCsvOption( options, key2, value2 );
    if ( GUCEF_NULL != key3 ) SetCsvOption( options, key3, value3 );
    return options;
}

/*-------------------------------------------------------------------------*/

static CString
RowsOf( const SCsvTestRead& read )
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
            text += read.rows[ r ][ c ].IsNULL() ? CString( "<null>" ) : read.rows[ r ][ c ].AsString();
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** The type texts of the columns, then '/' and a character per column: n for nullable and r for required; "<status>" when the read failed */
static CString
TypesOf( const char* csv, const CValueList& options = CsvOptions(), bool viaMetaData = true )
{
    SCsvTestRead read;
    TDataTableStatus status = ReadCsvSource( MakeCsvSource( csv ), options, read, 16384, viaMetaData );
    if ( DATATABLE_STATUS_OK != status )
        return CString( "<" ) + DataTableStatusToString( status ) + ": " + read.error + ">";
    CString nullability;
    for ( UInt32 i = 0; i < CsvColumnCountOf( read ); ++i )
        nullability += CsvColumnOf( read, i ).nullable ? 'n' : 'r';
    return CsvTypeTexts( read ) + "/" + nullability;
}

/*-------------------------------------------------------------------------*/

/** A file of one column x with the values, one per line */
static CString
OneColumn( const char* values )
{
    return CString( "x\n" ) + values;
}

/*-------------------------------------------------------------------------*/

static CString
ReadRows( const char* csv, const CValueList& options, TDataTableStatus* status = GUCEF_NULL, CString* error = GUCEF_NULL, UInt64* valueErrors = GUCEF_NULL )
{
    SCsvTestRead read;
    TDataTableStatus result = ReadCsvSource( MakeCsvSource( csv ), options, read );
    if ( GUCEF_NULL != status ) *status = result;
    if ( GUCEF_NULL != error ) *error = read.error;
    if ( GUCEF_NULL != valueErrors ) *valueErrors = read.stats.valueErrors;
    return DATATABLE_STATUS_OK == result ? RowsOf( read ) : CString( "<error>" );
}

/*-------------------------------------------------------------------------*/

/** The rank of an inferred type in the order the doc gives; -1 for text and for types outside the chain */
static int
Rank( const CDataTableColumnDef& column )
{
    switch ( column.type )
    {
        case DATATABLE_TYPE_BOOL: return 0;
        case DATATABLE_TYPE_INT64: return 1;
        case DATATABLE_TYPE_DECIMAL: return 2;
        case DATATABLE_TYPE_FLOAT64: return 3;
        default: return -1;
    }
}

/** Whether the type of the later, longer inference is the same as the earlier or wider */
static bool
IsWiderOrSame( const CDataTableColumnDef& earlier, const CDataTableColumnDef& later )
{
    if ( DATATABLE_TYPE_UTF8 == later.type )
        return true;
    if ( earlier.type == later.type )
    {
        if ( DATATABLE_TYPE_DECIMAL == earlier.type )
            return earlier.decimalScale <= later.decimalScale && ( earlier.decimalPrecision - earlier.decimalScale ) <= ( later.decimalPrecision - later.decimalScale );
        if ( DATATABLE_TYPE_TIMESTAMP == earlier.type )
            return earlier.timeUnit == later.timeUnit && earlier.utcAdjusted == later.utcAdjusted;
        return true;
    }
    // along the chain int64 < decimal < float64; bool and the rest do not widen into numbers
    if ( DATATABLE_TYPE_INT64 == earlier.type )
        return DATATABLE_TYPE_DECIMAL == later.type || DATATABLE_TYPE_FLOAT64 == later.type;
    if ( DATATABLE_TYPE_DECIMAL == earlier.type )
        return DATATABLE_TYPE_FLOAT64 == later.type;
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCsvTableCodecInferenceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.CsvTableCodecInference" );

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: whole numbers are int64 up to the limits of Int64, beyond them float64" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "1\n2\n-3\n0\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "9223372036854775807\n-9223372036854775808\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "999999999999999999\n-999999999999999999\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "9223372036854775808\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-9223372036854775809\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "12345678901234567890\n1\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-0\n0\n" ).C_String() ), "int64/r" );

        // the values themselves stay exact
        CHECK_TEXT( ReadRows( OneColumn( "9223372036854775807\n-9223372036854775808\n" ).C_String(), CsvOptions() ), "9223372036854775807;-9223372036854775808" );
        CHECK_TEXT( ReadRows( OneColumn( "-0\n" ).C_String(), CsvOptions() ), "0" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: a column that became float64 from values of more than 15 digits reports how many values may have lost digits" )
    try
    {
        // (keys are looked up as CStrings: a character literal is an ASCII variant, which compares equal to a longer UTF-8 key that starts with it)
        // identifiers of 20 digits do not fit Int64: float64, silently rounded to 17 digits unless the metadata says so
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "id,amount,small\n12345678901234567890,1.5,7\n98765432109876543210,2.25,8\n5,3.5,9\n" ), CsvOptions(), read ) );
        ASSERT_TRUE( DATATABLE_TYPE_FLOAT64 == CsvColumnOf( read, 0 ).type );
        ASSERT_TRUE( read.keyValues.HasKey( CString( "lossyInference" ) ) );
        CHECK_TEXT( read.keyValues.GetValueAlways( CString( "lossyInference" ) ).AsString(), "1" );
        ASSERT_TRUE( read.keyValues.HasKey( CString( "lossyInference.id" ) ) );
        CHECK_TEXT( read.keyValues.GetValueAlways( CString( "lossyInference.id" ) ).AsString(), "2" );
        ASSERT_FALSE( read.keyValues.HasKey( CString( "lossyInference.amount" ) ) );
        ASSERT_FALSE( read.keyValues.HasKey( CString( "lossyInference.small" ) ) );

        // a decimal beyond 18 digits is a float64 as well
        SCsvTestRead decimals;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "x\n0.1234567890123456789\n1.5\n" ), CsvOptions(), decimals ) );
        ASSERT_TRUE( DATATABLE_TYPE_FLOAT64 == CsvColumnOf( decimals, 0 ).type );
        CHECK_TEXT( decimals.keyValues.GetValueAlways( CString( "lossyInference.x" ) ).AsString(), "1" );

        // nothing is lost, nothing is reported: up to 15 digits, exponents, exact types, text
        SCsvTestRead exact;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a,b,c,d\n12345678901234.5,1e300,1234567890123456789,x\n2.5e3,0.1,5,y\n" ), CsvOptions(), exact ) );
        CHECK_TEXT( CsvTypeTexts( exact ), "float64,float64,int64,utf8" );
        CHECK_TEXT( exact.keyValues.GetValueAlways( CString( "lossyInference" ) ).AsString(), "0" );
        ASSERT_FALSE( exact.keyValues.HasKey( CString( "lossyInference.a" ) ) );
        ASSERT_FALSE( exact.keyValues.HasKey( CString( "lossyInference.b" ) ) );
        ASSERT_FALSE( exact.keyValues.HasKey( CString( "lossyInference.c" ) ) );

        // identifiers declared as text stay as they are and there is nothing to report
        SCsvTestRead given;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "id,amount\n12345678901234567890,1.5\n" ), Opts( "columnTypes.id", "utf8" ), given ) );
        CHECK_TEXT( CsvTypeTexts( given ), "utf8,decimal(2,1)" );
        ASSERT_TRUE( 1 == given.rows.size() && given.rows[ 0 ][ 0 ].AsString() == "12345678901234567890" );
        CHECK_TEXT( given.keyValues.GetValueAlways( CString( "lossyInference" ) ).AsString(), "0" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: numbers with a fraction are decimals with the most fraction digits and the most integer digits seen" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "1.5\n2\n" ).C_String() ), "decimal(2,1)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0.5\n" ).C_String() ), "decimal(1,1)/r" );
        CHECK_TEXT( TypesOf( OneColumn( ".5\n" ).C_String() ), "decimal(1,1)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "12.345\n1\n" ).C_String() ), "decimal(5,3)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0.00\n" ).C_String() ), "decimal(2,2)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1234567890123456.78\n" ).C_String() ), "decimal(18,2)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "99999999999999999\n0.5\n" ).C_String() ), "decimal(18,1)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-12.50\n7.125\n" ).C_String() ), "decimal(5,3)/r" );

        // more than 18 digits do not fit a decimal: float64
        CHECK_TEXT( TypesOf( OneColumn( "999999999999999999\n0.5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "123456789012345678.5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0.1234567890123456789\n" ).C_String() ), "float64/r" );

        // a number is not a decimal when there is nothing behind the point
        CHECK_TEXT( TypesOf( OneColumn( "5.\n" ).C_String() ), "utf8/r" );

        // the digits are kept exactly: no detour through a double
        CHECK_TEXT( ReadRows( OneColumn( "9007199254740993.5\n0.1\n" ).C_String(), CsvOptions() ), "9007199254740993.5;0.1" );
        CHECK_TEXT( ReadRows( OneColumn( "0.10000000000000001\n" ).C_String(), CsvOptions() ), "0.10000000000000001" );
        CHECK_TEXT( ReadRows( OneColumn( "123456789.123456789\n" ).C_String(), CsvOptions() ), "123456789.123456789" );
        CHECK_TEXT( ReadRows( OneColumn( "-0.001\n12\n" ).C_String(), CsvOptions() ), "-0.001;12.000" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: an exponent or too many digits make a float64, nan and inf belong to float columns only" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "1e5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1.5E-3\n2\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-2.5e+10\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1e400\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1e\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "e5\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "nan\n1.5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "NaN\n5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "inf\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-Infinity\n2.5\n" ).C_String() ), "float64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "nan\nabc\n" ).C_String() ), "utf8/r" );

        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( OneColumn( "1e3\n0.25\n-7\nnan\n-inf\n" ).C_String() ), CsvOptions(), read ) );
        ASSERT_TRUE( 5 == read.rows.size() );
        ASSERT_TRUE( 1000.0 == CsvCell( read, 0, 0 ).AsFloat64() && 0.25 == CsvCell( read, 1, 0 ).AsFloat64() && -7.0 == CsvCell( read, 2, 0 ).AsFloat64() );
        ASSERT_TRUE( CsvCell( read, 3, 0 ).AsFloat64() != CsvCell( read, 3, 0 ).AsFloat64() );
        ASSERT_TRUE( CsvCell( read, 4, 0 ).AsFloat64() < -1e308 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: true and false in any case are bool, nothing else is" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "true\nfalse\nTRUE\nFalse\n" ).C_String() ), "bool/r" );
        CHECK_TEXT( TypesOf( OneColumn( "true\n1\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "yes\nno\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "t\nf\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1\n0\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "true\nfalsey\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( ReadRows( OneColumn( "true\nFALSE\nTrue\n" ).C_String(), CsvOptions() ), "true;false;true" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: dates are recognised by the formats of the options, the first format that fits every value decides" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31\n2023-12-01\n" ).C_String() ), "date/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-02-29\n" ).C_String() ), "date/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2023-02-29\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-04-31\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-13-01\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-00-10\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-1-5\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "20240131\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1900-02-29\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2000-02-29\n" ).C_String() ), "date/r" );
        CHECK_TEXT( ReadRows( OneColumn( "2024-02-29\n1970-01-01\n1969-12-31\n" ).C_String(), CsvOptions() ), "2024-02-29;1970-01-01;1969-12-31" );

        CValueList us = Opts();
        CCsvTableCodec::AddListOption( us, "dateFormats", "MM/DD/YYYY" );
        CHECK_TEXT( TypesOf( OneColumn( "01/31/2024\n12/01/2023\n" ).C_String(), us ), "date/r" );
        CHECK_TEXT( ReadRows( OneColumn( "01/31/2024\n12/01/2023\n" ).C_String(), us ), "2024-01-31;2023-12-01" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31\n" ).C_String(), us ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "02/30/2024\n" ).C_String(), us ), "utf8/r" );

        // two formats that both fit some values: the first that fits all of them
        CValueList both = Opts();
        CCsvTableCodec::AddListOption( both, "dateFormats", "DD/MM/YYYY" );
        CCsvTableCodec::AddListOption( both, "dateFormats", "MM/DD/YYYY" );
        CHECK_TEXT( TypesOf( OneColumn( "01/02/2024\n13/01/2024\n" ).C_String(), both ), "date/r" );
        CHECK_TEXT( ReadRows( OneColumn( "01/02/2024\n13/01/2024\n" ).C_String(), both ), "2024-02-01;2024-01-13" );
        CHECK_TEXT( ReadRows( OneColumn( "01/02/2024\n01/13/2024\n" ).C_String(), both ), "2024-01-02;2024-01-13" );
        CHECK_TEXT( ReadRows( OneColumn( "01/02/2024\n" ).C_String(), both ), "2024-02-01" );
        CHECK_TEXT( TypesOf( OneColumn( "13/01/2024\n01/13/2024\n" ).C_String(), both ), "utf8/r" );

        // formats with one digit parts, two digit years and other separators
        CValueList loose = Opts();
        CCsvTableCodec::AddListOption( loose, "dateFormats", "M/D/YY" );
        CHECK_TEXT( ReadRows( OneColumn( "1/2/24\n12/31/70\n3/4/69\n" ).C_String(), loose ), "2024-01-02;1970-12-31;2069-03-04" );
        CValueList dots = Opts();
        CCsvTableCodec::AddListOption( dots, "dateFormats", "DD.MM.YYYY" );
        CHECK_TEXT( ReadRows( OneColumn( "31.01.2024\n" ).C_String(), dots ), "2024-01-31" );
        CValueList compact = Opts();
        CCsvTableCodec::AddListOption( compact, "dateFormats", "YYYYMMDD" );
        CHECK_TEXT( TypesOf( OneColumn( "abc\n" ).C_String(), compact ), "utf8/r" );
        CHECK_TEXT( ReadRows( OneColumn( "x\n" ).C_String(), compact ), "x" );

        // the formats of the options have to make sense
        TDataTableStatus status = DATATABLE_STATUS_OK;
        CValueList noDay = Opts();
        CCsvTableCodec::AddListOption( noDay, "dateFormats", "YYYY-MM" );
        ReadRows( OneColumn( "1\n" ).C_String(), noDay, &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );
        CValueList ambiguous = Opts();
        CCsvTableCodec::AddListOption( ambiguous, "dateFormats", "YYYYMD" );
        ReadRows( OneColumn( "1\n" ).C_String(), ambiguous, &status );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: date times with a zone are timestamps in UTC, without a zone they are text" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30Z\n2024-01-31T10:20:30.123456+02:00\n" ).C_String() ), "timestamp(us,utc)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31 10:20:30Z\n" ).C_String() ), "timestamp(us,utc)/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30.1234567Z\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30Z\n2024-01-31\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T25:20:30Z\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-02-30T10:20:30Z\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20Z\n" ).C_String() ), "timestamp(us,utc)/r" );

        CHECK_TEXT( ReadRows( OneColumn( "2024-01-31T10:20:30Z\n2024-01-31T10:20:30.123456+02:00\n1969-12-31T23:59:59.999999Z\n" ).C_String(), CsvOptions() ),
                    "2024-01-31T10:20:30.000000Z;2024-01-31T08:20:30.123456Z;1969-12-31T23:59:59.999999Z" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: columns with only nulls are nullable text, columns with nulls are nullable, the others are not" )
    try
    {
        CHECK_TEXT( TypesOf( "a,b\n,\n,\n" ), "utf8,utf8/nn" );
        CHECK_TEXT( TypesOf( "a,b,c\n" ), "utf8,utf8,utf8/nnn" );
        CHECK_TEXT( TypesOf( "a,b\n1,2\n3,\n" ), "int64,int64/rn" );
        CHECK_TEXT( TypesOf( "a,b\n1,x\n,y\n" ), "int64,utf8/nr" );
        CHECK_TEXT( TypesOf( "a,b\n1,x\n\"\",y\n" ), "int64,utf8/nr" );
        CHECK_TEXT( TypesOf( "a\n1\n\n" ), "int64/n" );

        // only part of the rows is looked at: the others may hold anything, so nothing is required
        CValueList few = Opts( "inferRowCount", "2" );
        CHECK_TEXT( TypesOf( "a,b\n1,2\n3,4\n5,6\n", few ), "int64,int64/nn" );
        CHECK_TEXT( TypesOf( "a,b\n1,2\n3,4\n5,6\n", Opts( "inferRowCount", "0" ) ), "int64,int64/rr" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: numbers with leading zeros or a plus are identifiers, not numbers" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( "007\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "00\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-007\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "00.5\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "02134\n10001\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "+5\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "+15551234567\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0\n10\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0.5\n0\n" ).C_String() ), "decimal(1,1)/r" );

        // but a column that is declared a number parses them
        CValueList declared = Opts( "columnTypes.x", "int64" );
        CHECK_TEXT( ReadRows( OneColumn( "007\n+5\n-007\n" ).C_String(), declared ), "7;5;-7" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: thousands separators group the integer digits by three, currency symbols and parentheses are stripped" )
    try
    {
        CValueList thousands = Opts( "thousandsSeparator", "," );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,234\"\n\"12,345,678\"\n\"999\"\n", thousands ), "int64/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,23\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1234,567\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,234,56\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\",123\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"123,\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,,234\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,234.50\"\n", thousands ), "decimal(6,2)/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,234.5,6\"\n", thousands ), "utf8/r" );
        CHECK_TEXT( TypesOf( "\"x\"\n\"1,234\"\n" ), "utf8/r" );
        CHECK_TEXT( ReadRows( "\"x\"\n\"1,234,567\"\n\"-12,000.25\"\n", thousands ), "1234567.00;-12000.25" );

        CValueList dollars = Opts( "currencySymbols", "$" );
        CHECK_TEXT( TypesOf( OneColumn( "$1.50\n$20.00\n" ).C_String(), dollars ), "decimal(4,2)/r" );
        CHECK_TEXT( ReadRows( OneColumn( "$1.50\n-$5.00\n$-6.25\n7.00$\n$ 8.00\n" ).C_String(), dollars ), "1.50;-5.00;-6.25;7.00;8.00" );
        CHECK_TEXT( TypesOf( OneColumn( "$$5\n" ).C_String(), dollars ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "$\n" ).C_String(), dollars ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "$1.50\n" ).C_String() ), "utf8/r" );

        CValueList euro = Opts( "currencySymbols", "\xE2\x82\xAC", "decimalSeparator", ",", "delimiter", ";" );
        CCsvTableCodec::AddListOption( euro, "currencySymbols", "EUR" );
        CHECK_TEXT( ReadRows( "x\n\xE2\x82\xAC 5,00\nEUR7,25\n-3,50\xE2\x82\xAC\n", euro ), "5.00;7.25;-3.50" );

        CValueList parentheses = Opts( "negativeParentheses", "true" );
        CHECK_TEXT( TypesOf( OneColumn( "(12.34)\n5.00\n" ).C_String(), parentheses ), "decimal(4,2)/r" );
        CHECK_TEXT( ReadRows( OneColumn( "(12.34)\n5.00\n( 7 )\n" ).C_String(), parentheses ), "-12.34;5.00;-7.00" );
        CHECK_TEXT( TypesOf( OneColumn( "(12.34\n" ).C_String(), parentheses ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-(12.34)\n" ).C_String(), parentheses ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "(-12.34)\n" ).C_String(), parentheses ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "()\n" ).C_String(), parentheses ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "(12.34)\n" ).C_String() ), "utf8/r" );

        // all of it together: a statement amount
        CValueList statement = Opts( "currencySymbols", "$", "thousandsSeparator", ",", "negativeParentheses", "true" );
        CHECK_TEXT( ReadRows( "\"x\"\n\"$1,234.50\"\n\"($1,000.00)\"\n\"-$0.05\"\n", statement ), "1234.50;-1000.00;-0.05" );

        // a decimal comma
        CValueList comma = Opts( "decimalSeparator", ",", "delimiter", ";" );
        CHECK_TEXT( ReadRows( "x\n1,5\n2,25\n", comma ), "1.50;2.25" );
        CHECK_TEXT( TypesOf( "x\n1.5\n", comma ), "utf8/r" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: blanks around a number are ignored, a number with other text is text" )
    try
    {
        CHECK_TEXT( TypesOf( OneColumn( " 12 \n\"  13  \"\n" ).C_String() ), "int64/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1 2\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "12abc\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "--5\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "-\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1\nabc\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1\n2.5\nabc\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "0x10\n" ).C_String() ), "utf8/r" );
        CHECK_TEXT( TypesOf( OneColumn( "1_000\n" ).C_String() ), "utf8/r" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: the types of the options override what the values say, names are matched as they are" )
    try
    {
        // text that looks like a number
        CValueList zip = Opts( "columnTypes.zip", "utf8" );
        CHECK_TEXT( TypesOf( "zip,n\n02134,1\n10001,2\n", zip ), "utf8,int64/nr" );
        CHECK_TEXT( ReadRows( "zip,n\n02134,1\n10001,2\n", zip ), "02134|1;10001|2" );

        // numbers as decimals of a chosen shape
        CValueList money = Opts( "columnTypes.amount", "decimal(18,2)" );
        CHECK_TEXT( TypesOf( "amount\n1.5\n2\n", money ), "decimal(18,2)/n" );
        CHECK_TEXT( ReadRows( "amount\n1.5\n2\n12.500\n", money ), "1.50;2.00;12.50" );

        // dates in a format of their own, or the formats of the options
        CValueList dated = Opts( "columnTypes.d", "date:MM/DD/YYYY" );
        CHECK_TEXT( TypesOf( "d,e\n01/31/2024,2024-01-31\n", dated ), "date,date/nr" );
        CHECK_TEXT( ReadRows( "d,e\n01/31/2024,2024-01-31\n", dated ), "2024-01-31|2024-01-31" );
        CValueList dateWithoutFormat = Opts( "columnTypes.d", "date" );
        CCsvTableCodec::AddListOption( dateWithoutFormat, "dateFormats", "DD-MM-YYYY" );
        CHECK_TEXT( ReadRows( "d\n31-01-2024\n", dateWithoutFormat ), "2024-01-31" );

        // a type for the whole table without inference
        CValueList many = Opts( "columnTypes.a", "int8", "columnTypes.b", "uint16", "columnTypes.c", "float32" );
        CHECK_TEXT( TypesOf( "a,b,c\n-128,65535,0.1\n", many ), "int8,uint16,float32/nnn" );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "a,b,c\n-128,65535,0.1\n127,0,-3.4028235e38\n" ), many, read ) );
        ASSERT_TRUE( -128 == CsvCell( read, 0, 0 ).AsInt32() && 65535 == CsvCell( read, 0, 1 ).AsUInt32() );
        ASSERT_TRUE( 0.1f == CsvCell( read, 0, 2 ).AsFloat32() );
        ASSERT_TRUE( 127 == CsvCell( read, 1, 0 ).AsInt32() && -3.4028235e38f == CsvCell( read, 1, 2 ).AsFloat32() );

        // names are exact; an override names a column of the file
        CHECK_TEXT( TypesOf( "A\n5\n", Opts( "columnTypes.a", "utf8" ) ), "<INVALID_ARGUMENT: The option columnTypes names a column that the file does not have: \"a\">" );
        CHECK_TEXT( TypesOf( "A\n5\n", Opts( "columnTypes.A", "utf8" ) ), "utf8/n" );

        // the override applies to made up names as well
        CHECK_TEXT( TypesOf( "a,\n1,2\n", Opts( "columnTypes.column_2", "utf8" ) ), "int64,utf8/rn" );

        // headerless
        CHECK_TEXT( TypesOf( "1,2\n3,4\n", Opts( "hasHeader", "false", "columnTypes.column_2", "float64" ) ), "int64,float64/rn" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: types given as text read booleans, timestamps and binary as the type says" )
    try
    {
        CValueList flags = Opts( "columnTypes.x", "bool" );
        CHECK_TEXT( ReadRows( OneColumn( "true\nFALSE\nyes\nno\n1\n0\ny\nN\nt\nF\n" ).C_String(), flags ), "true;false;true;false;true;false;true;false;true;false" );
        TDataTableStatus status = DATATABLE_STATUS_OK;
        ReadRows( OneColumn( "maybe\n" ).C_String(), flags, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "2\n" ).C_String(), flags, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

        CValueList ms = Opts( "columnTypes.x", "timestamp(ms,utc)" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30.123Z\n" ).C_String(), ms ), "timestamp(ms,utc)/n" );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( OneColumn( "2024-01-31T10:20:30.123Z\n2024-01-31T12:20:30.123+02:00\n1970-01-01T00:00:00Z\n2024-01-31\n" ).C_String() ), ms, read ) );
        Int64 expected = 0;
        ASSERT_TRUE( CDataTableValues::ParseTimestamp( "2024-01-31T10:20:30.123Z", 24, DATATABLE_TIMEUNIT_MILLISECONDS, expected ) );
        ASSERT_TRUE( expected == CsvCell( read, 0, 0 ).AsInt64() && expected == CsvCell( read, 1, 0 ).AsInt64() );
        ASSERT_TRUE( 0 == CsvCell( read, 2, 0 ).AsInt64() );
        ASSERT_TRUE( 0 == ( CsvCell( read, 3, 0 ).AsInt64() % 86400000 ) );       // a date is midnight

        // finer than the unit is refused, a unit finer than the file is fine
        ReadRows( OneColumn( "2024-01-31T10:20:30.1234Z\n" ).C_String(), ms, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        CValueList ns = Opts( "columnTypes.x", "timestamp(ns)" );
        CHECK_TEXT( TypesOf( OneColumn( "2024-01-31T10:20:30\n" ).C_String(), ns ), "timestamp(ns)/n" );

        CValueList binary = Opts( "columnTypes.x", "binary" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( OneColumn( "AAEC/w==\n\"\"\nAA==\nAAE=\nAAEC\n" ).C_String() ), binary, read ) );
        ASSERT_TRUE( 5 == read.rows.size() );
        ASSERT_TRUE( 4 == CsvCell( read, 0, 0 ).ByteSize() && 0 == memcmp( CsvCell( read, 0, 0 ).AsVoidPtr(), "\x00\x01\x02\xff", 4 ) );
        ASSERT_TRUE( !CsvCell( read, 1, 0 ).IsNULL() && 0 == CsvCell( read, 1, 0 ).ByteSize() );
        ASSERT_TRUE( 1 == CsvCell( read, 2, 0 ).ByteSize() && 2 == CsvCell( read, 3, 0 ).ByteSize() && 3 == CsvCell( read, 4, 0 ).ByteSize() );
        const char* invalidBinary[] = { "A\n", "AAE\n", "AA=A\n", "AAE=x\n", "A*E=\n", "====\n" };
        for ( size_t i = 0; i < sizeof( invalidBinary ) / sizeof( invalidBinary[ 0 ] ); ++i )
        {
            ReadRows( ( OneColumn( "" ) + invalidBinary[ i ] ).C_String(), binary, &status );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: numbers must fit the column, decimals are never rounded" )
    try
    {
        TDataTableStatus status = DATATABLE_STATUS_OK;
        CString error;

        CValueList small = Opts( "columnTypes.x", "int8" );
        CHECK_TEXT( ReadRows( OneColumn( "-128\n127\n" ).C_String(), small ), "-128;127" );
        ReadRows( OneColumn( "128\n" ).C_String(), small, &status, &error );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ASSERT_TRUE( error.HasSubstr( "Column \"x\"" ) >= 0 && error.HasSubstr( "128" ) >= 0 && error.HasSubstr( "record 2" ) >= 0 );
        ReadRows( OneColumn( "-129\n" ).C_String(), small, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "1.0\n" ).C_String(), small, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "1e2\n" ).C_String(), small, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

        CValueList unsignedBig = Opts( "columnTypes.x", "uint64" );
        CHECK_TEXT( ReadRows( OneColumn( "18446744073709551615\n0\n-0\n" ).C_String(), unsignedBig ), "18446744073709551615;0;0" );
        ReadRows( OneColumn( "18446744073709551616\n" ).C_String(), unsignedBig, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "-1\n" ).C_String(), unsignedBig, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

        CValueList decimal = Opts( "columnTypes.x", "decimal(18,2)" );
        CHECK_TEXT( ReadRows( OneColumn( "12.500\n12.5\n-0.00\n1000000000000000.99\n" ).C_String(), decimal ), "12.50;12.50;0.00;1000000000000000.99" );
        ReadRows( OneColumn( "12.505\n" ).C_String(), decimal, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "-0.001\n" ).C_String(), decimal, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "10000000000000000.00\n" ).C_String(), decimal, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "1e2\n" ).C_String(), decimal, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ReadRows( OneColumn( "abc\n" ).C_String(), decimal, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

        CValueList narrow = Opts( "columnTypes.x", "decimal(5,2)" );
        CHECK_TEXT( ReadRows( OneColumn( "123.45\n-999.99\n" ).C_String(), narrow ), "123.45;-999.99" );
        ReadRows( OneColumn( "1234.56\n" ).C_String(), narrow, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );

        CValueList wide = Opts( "columnTypes.x", "decimal(18,17)" );
        CHECK_TEXT( ReadRows( OneColumn( "0.10000000000000001\n9.99999999999999999\n" ).C_String(), wide ), "0.10000000000000001;9.99999999999999999" );

        // floats are as near as the type allows, and not beyond it
        CValueList f64 = Opts( "columnTypes.x", "float64" );
        SCsvTestRead read;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( OneColumn( "0.1\n1e308\n-1e-308\n4.9e-324\n123456789012345678\n" ).C_String() ), f64, read ) );
        ASSERT_TRUE( 0.1 == CsvCell( read, 0, 0 ).AsFloat64() && 1e308 == CsvCell( read, 1, 0 ).AsFloat64() && -1e-308 == CsvCell( read, 2, 0 ).AsFloat64() );
        ASSERT_TRUE( 4.9e-324 == CsvCell( read, 3, 0 ).AsFloat64() && 123456789012345678.0 == CsvCell( read, 4, 0 ).AsFloat64() );
        ReadRows( OneColumn( "1e309\n" ).C_String(), f64, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        CValueList f32 = Opts( "columnTypes.x", "float32" );
        ReadRows( OneColumn( "3.5e38\n" ).C_String(), f32, &status );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: a value that does not fit is an error or a counted null, as onValueError says" )
    try
    {
        TDataTableStatus status = DATATABLE_STATUS_OK;
        CString error;
        UInt64 valueErrors = 0;

        CValueList failing = Opts( "columnTypes.n", "int64" );
        CHECK_TEXT( ReadRows( "n,t\n1,a\nabc,b\n3,c\n", failing, &status, &error, &valueErrors ), "<error>" );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ASSERT_TRUE( error.HasSubstr( "Column \"n\"" ) >= 0 && error.HasSubstr( "\"abc\"" ) >= 0 && error.HasSubstr( "record 3" ) >= 0 );

        CValueList nulling = Opts( "columnTypes.n", "int64", "onValueError", "null" );
        CHECK_TEXT( ReadRows( "n,t\n1,a\nabc,b\n3,c\n4x,d\n", nulling, &status, &error, &valueErrors ), "1|a;<null>|b;3|c;<null>|d" );
        ASSERT_TRUE( DATATABLE_STATUS_OK == status );
        ASSERT_TRUE( 2 == valueErrors );

        // with a column that cannot be null the substitute is refused
        CDataTableSchema required;
        required.AddColumn( CDataTableColumnDef( "n", DATATABLE_TYPE_INT64, false ) );
        CValueList refusing = Opts( "onValueError", "null" );
        CCsvTableCodec::SetSchemaOption( refusing, required );
        CHECK_TEXT( ReadRows( "n\n1\nabc\n", refusing, &status, &error ), "<error>" );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ASSERT_TRUE( error.HasSubstr( "does not take null" ) >= 0 );

        // text that is not valid UTF-8 is no value error: the file is corrupt whatever the option says
        const char badText[] = "n\n\xC3\x28\n";
        SCsvTestRead read;
        CValueList text = Opts( "onValueError", "null", "columnTypes.n", "utf8" );
        ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == ReadCsvSource( MakeCsvSource( badText, sizeof( badText ) - 1 ), text, read ) );

        // the counts are for the scan: they come with the batches that were read
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadCsvSource( MakeCsvSource( "n\n1\nx\ny\n4\nz\n" ), nulling, read, 2 ) );
        ASSERT_TRUE( 3 == read.stats.valueErrors );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: the types come out the same through the metadata and through the reader alone, and a part of the rows decides when asked" )
    try
    {
        const char* csv = "id,price,day,flag,note\n1,1.5,2024-01-31,true,a\n2,2.25,2024-02-01,false,\n3,3,2024-03-01,true,c\n";
        CString viaMetaData = TypesOf( csv, CsvOptions(), true );
        CString direct = TypesOf( csv, CsvOptions(), false );
        CHECK_TEXT( viaMetaData, "int64,decimal(3,2),date,bool,utf8/rrrrn" );
        CHECK_TEXT( direct, viaMetaData );

        // the rows after the ones that decide may not fit: an error that names the value
        const char* late = "n\n1\n2\n3\nabc\n";
        {
            CTableCodecRegistry::TTableCodecPtr codec;
            ASSERT_TRUE( GetCsvCodec( codec ) );
            CCsvMemoryFile file( late, (UInt32) strlen( late ) );
            CITableFileMetaDataPtr metaData;
            CString metaError;
            ASSERT_TRUE( DATATABLE_STATUS_OK == codec->ReadMetaData( CsvIo( file.Access() ), Opts( "inferRowCount", "3" ), metaData, metaError ) );
            ASSERT_TRUE( !metaData.IsNULL() && 1 == metaData->GetSchema()->GetColumnCount() );
            CHECK_TEXT( metaData->GetSchema()->GetColumn( 0 ).GetTypeText(), "int64" );
            ASSERT_TRUE( metaData->GetSchema()->GetColumn( 0 ).nullable );
            ASSERT_TRUE( GUCEF_UINT64MAX == metaData->GetRowCount() && 1 == metaData->GetRowGroupCount() );
            CDataTableRowGroupInfo info;
            ASSERT_TRUE( metaData->GetRowGroupInfo( 0, info ) && !metaData->GetRowGroupInfo( 1, info ) );
        }
        TDataTableStatus status = DATATABLE_STATUS_OK;
        CString error;
        ReadRows( late, Opts( "inferRowCount", "3" ), &status, &error );
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == status );
        ASSERT_TRUE( error.HasSubstr( "abc" ) >= 0 );
        CHECK_TEXT( TypesOf( late, Opts( "inferRowCount", "4" ) ), "utf8/n" );
        CHECK_TEXT( TypesOf( late ), "utf8/r" );
        CHECK_TEXT( ReadRows( late, Opts( "inferRowCount", "3", "onValueError", "null" ) ), "1;2;3;<null>" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CsvTableCodecInference: with more rows the type only gets wider, and every value parses in the type that was found" )
    try
    {
        const char* pool[] = {
            "1", "-5", "42", "0", "2.50", "0.125", "-0.5", ".75", "1e3", "-2.5E-2", "abc", "", "2024-01-02", "2024-12-31", "2024-01-02T03:04:05Z",
            "true", "FALSE", "007", "12345678901234567890", "nan", "999999999999999999", "1234567.891", "x y", "\"quoted, text\"", "\"\"", "+5", "9223372036854775807" };
        const UInt32 poolSize = sizeof( pool ) / sizeof( pool[ 0 ] );

        for ( UInt32 seed = 1; seed <= 400; ++seed )
        {
            CCsvTestRandom random( seed );
            const UInt32 count = 1 + random.Below( 10 );
            // draw from a few kinds so that long runs of one kind occur
            const UInt32 kindStart = random.Below( poolSize );
            CString values[ 16 ];
            for ( UInt32 i = 0; i < count; ++i )
                values[ i ] = random.Coin( 3 ) ? CString( pool[ random.Below( poolSize ) ] ) : CString( pool[ ( kindStart + random.Below( 4 ) ) % poolSize ] );

            CDataTableColumnDef previous;
            bool havePrevious = false;
            for ( UInt32 prefix = 1; prefix <= count; ++prefix )
            {
                CString csv = "x\n";
                bool anyValue = false;
                for ( UInt32 i = 0; i < prefix; ++i )
                {
                    csv += values[ i ] + "\n";
                    if ( !values[ i ].IsNULLOrEmpty() && values[ i ] != "\"\"" )
                        anyValue = true;
                }

                SCsvTestRead read;
                const TDataTableStatus status = ReadCsvSource( MakeCsvSource( csv.C_String() ), CsvOptions(), read );
                if ( DATATABLE_STATUS_OK != status )
                {
                    // the only way a file of one column fails is text that is not a field
                    ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( ( CString( "read failed: " ) + read.error + " for [" + csv + "]" ).C_String(), __FILE__, __LINE__ );
                    break;
                }
                ASSERT_TRUE( prefix == read.rows.size() );

                if ( !anyValue )
                    continue;
                const CDataTableColumnDef& column = CsvColumnOf( read, 0 );
                if ( havePrevious && !IsWiderOrSame( previous, column ) )
                {
                    ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( ( CString( "the type got narrower from " ) + previous.GetTypeText() + " to " + column.GetTypeText() +
                                                                              " for [" + csv + "]" ).C_String(), __FILE__, __LINE__ );
                }
                previous = column;
                havePrevious = true;
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
