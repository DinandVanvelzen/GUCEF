/*
 *  tablecodecpluginXLSX_TestApp: tests of the xlsx table codec plugin
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#include "tablecodecpluginXLSX_CXlsxPackage.h"
#define TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#include "tablecodecpluginXLSX_CXlsxFileMetaData.h"
#define TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H ? */

#include "TestXlsxSupport.h"
#include "TestXlsxCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** An assertion that the rest of the test case cannot do without: the failure is recorded and the case ends (the catch records it once more) */
#define REQUIRE(t)      do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw (int) 0; } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::TABLECODECPLUGIN::XLSX;
using namespace XLSXTEST;

namespace {

void
SetOption( CValueList& options, const char* key, const char* value )
{
    options.Set( CString( key ), CVariant( CString( value ) ) );
}

/** Reads the workbook and compares the schema and the rows with the expected text */
bool
CheckTable( const char* label, const CDynamicBuffer& file, const CValueList& options, const char* expectedSchema, const char* expectedRows, UInt32 batchRowCount = 0 )
{
    CString error;
    CDataTableSchemaPtr schema;
    TBatchVector batches;
    const TDataTableStatus status = ReadXlsx( file, options, error, &schema, &batches, batchRowCount );
    if ( DATATABLE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "XlsxTest " << label << ": the read failed with " << DataTableStatusToString( status ) << ": " << error );
        return false;
    }
    const CString actualSchema = DescribeSchema( *schema );
    const CString actualRows = DumpRows( *schema, batches );
    bool ok = true;
    if ( GUCEF_NULL != expectedSchema && actualSchema != expectedSchema )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "XlsxTest " << label << ": the schema is\n" << actualSchema << "\nexpected\n" << expectedSchema );
        ok = false;
    }
    if ( GUCEF_NULL != expectedRows && !( actualRows.IsNULLOrEmpty() && 0 == expectedRows[ 0 ] ) && actualRows != expectedRows )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "XlsxTest " << label << ": the rows are\n" << actualRows << "expected\n" << expectedRows );
        ok = false;
    }
    return ok;
}

/** The read has to fail with the status and a message that holds the text */
bool
CheckFailure( const char* label, const CDynamicBuffer& file, const CValueList& options, TDataTableStatus expected, const char* messagePart )
{
    CString error;
    const TDataTableStatus status = ReadXlsx( file, options, error );
    if ( expected != status || !Contains( error, messagePart ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "XlsxTest " << label << ": expected " << DataTableStatusToString( expected ) << " with \"" << messagePart << "\", got " <<
                         DataTableStatusToString( status ) << " with \"" << error << "\"" );
        return false;
    }
    return true;
}

CDynamicBuffer
MakeFile( CXlsxBuilder& builder )
{
    CDynamicBuffer file;
    if ( !builder.Build( file ) )
        file.Clear( true );
    return file;
}

CString::StringVector
Strings( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL, const char* e = GUCEF_NULL,
         const char* f = GUCEF_NULL, const char* g = GUCEF_NULL, const char* h = GUCEF_NULL )
{
    CString::StringVector strings;
    const char* all[] = { a, b, c, d, e, f, g, h };
    for ( UInt32 i = 0; i < 8; ++i )
        if ( GUCEF_NULL != all[ i ] )
            strings.push_back( all[ i ] );
    return strings;
}

}

/*-------------------------------------------------------------------------*/

void
PerformXlsxCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginXLSX.Codec" );

    GUCEF_TESTFW_TESTCASE( "Codec: identity, read only, and the runtime plugins" )
    try
    {
        ASSERT_TRUE( ArePluginsAvailable() );
        CITableCodec& codec = GetXlsxCodec();
        ASSERT_TRUE( "xlsx" == codec.GetTypeName() );
        CString::StringSet extensions;
        codec.GetFileExtensions( extensions );
        ASSERT_TRUE( 1 == extensions.size() && extensions.find( "xlsx" ) != extensions.end() );
        ASSERT_TRUE( 0 != ( codec.GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_READ ) );
        ASSERT_TRUE( 0 == ( codec.GetCapabilities() & GUCEF_TABLECODEC_CAPABILITY_WRITE ) );

        CITableWriterPtr writer;
        CString error;
        CDataTableSchema schema;
        schema.AddColumn( CDataTableColumnDef( "a", DATATABLE_TYPE_INT64 ) );
        CDynamicBuffer sink;
        ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == codec.CreateWriter( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( sink ) ), CreateDataTableSchemaPtr( schema ), CDataTableWriterOptions(), writer, error ) );
        ASSERT_TRUE( writer.IsNULL() && Contains( error, "only reads" ) );
        CICloneable* clone = codec.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone );
        GUCEF_DELETE clone;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: shared strings, numbers as exact decimals, integers and booleans" )
    try
    {
        CXlsxBuilder builder;
        builder.SetSharedStrings( Strings( "Name", "Qty", "Price", "Active", "Widget", "Gadget", "Doohickey" ) );
        CString rows;
        rows += Row( 1, CellShared( "A1", 0 ) + CellShared( "B1", 1 ) + CellShared( "C1", 2 ) + CellShared( "D1", 3 ) );
        rows += Row( 2, CellShared( "A2", 4 ) + CellNumber( "B2", "5" ) + CellNumber( "C2", "19.99" ) + CellBool( "D2", true ) );
        rows += Row( 3, CellShared( "A3", 5 ) + CellNumber( "B3", "12" ) + CellNumber( "C3", "0.05" ) + CellBool( "D3", false ) );
        rows += Row( 4, CellShared( "A4", 6 ) + CellNumber( "B4", "100" ) + CellNumber( "C4", "1234.5" ) + CellBool( "D4", true ) );
        builder.AddSheet( "Sheet1", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );
        ASSERT_TRUE( file.GetDataSize() > 0 );

        ASSERT_TRUE( CheckTable( "basic", file, CValueList(), "Name:utf8|Qty:int64|Price:decimal(6,2)|Active:bool",
                                 "Widget|5|19.99|true\nGadget|12|0.05|false\nDoohickey|100|1234.50|true\n" ) );

        // the metadata knows the rows and where the table is
        CString error;
        CITableFileMetaDataPtr metaData;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, GUCEF_NULL, GUCEF_NULL, 0, &metaData ) && !metaData.IsNULL() );
        ASSERT_TRUE( 3 == metaData->GetRowCount() && 1 == metaData->GetRowGroupCount() );
        CValueList keyValues;
        metaData->GetKeyValueMetaData( keyValues );
        ASSERT_TRUE( "Sheet1" == keyValues.GetValueAlways( "sheetName" ).AsString() && "1900" == keyValues.GetValueAlways( "dateSystem" ).AsString() );
        ASSERT_TRUE( 1 == keyValues.GetValueAlways( "headerRow" ).AsUInt32() );
        CDataTableRowGroupInfo info;
        ASSERT_TRUE( metaData->GetRowGroupInfo( 0, info ) && 3 == info.rowCount );
        ASSERT_FALSE( metaData->GetRowGroupInfo( 1, info ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: decimals are the digits of the text, never a floating point number" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "Amount" ) + CellInline( "B1", "Big" ) );
        rows += Row( 2, CellNumber( "A2", "0.1" ) + CellNumber( "B2", "12345678901234567" ) );
        rows += Row( 3, CellNumber( "A3", "0.2" ) + CellNumber( "B3", "9007199254740993" ) );
        rows += Row( 4, CellNumber( "A4", "0.30000000000000004" ) + CellNumber( "B4", "-5" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );

        // 0.30000000000000004 has 17 decimals: exactly a decimal(18,17), and 2^53 + 1 is exactly an int64
        ASSERT_TRUE( CheckTable( "exact", file, CValueList(), "Amount:decimal(18,17)|Big:int64",
                                 "0.10000000000000000|12345678901234567\n0.20000000000000000|9007199254740993\n0.30000000000000004|-5\n" ) );

        // a column typed decimal(18,2) does not take the digits it cannot hold, unless the options round
        CValueList typed;
        SetOption( typed, "columnTypes.Amount", "decimal(18,2)" );
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == ReadXlsx( file, typed, error ) );
        ASSERT_TRUE( Contains( error, "0.30000000000000004" ) && Contains( error, "decimalRounding" ) && Contains( error, "A4" ) );

        SetOption( typed, "decimalRounding", "halfUp" );
        ASSERT_TRUE( CheckTable( "rounded", file, typed, "Amount:decimal(18,2)|Big:int64", "0.10|12345678901234567\n0.20|9007199254740993\n0.30|-5\n" ) );

        // with a number that does not fit a decimal of 18 digits the column falls back to a float64 and says so
        CXlsxBuilder wide;
        CString wideRows;
        wideRows += Row( 1, CellInline( "A1", "v" ) );
        wideRows += Row( 2, CellNumber( "A2", "0.30000000000000004" ) );
        wideRows += Row( 3, CellNumber( "A3", "1234.56" ) );
        wide.AddSheet( "S", Sheet( wideRows ) );
        CDynamicBuffer wideFile = MakeFile( wide );
        CDataTableSchemaPtr schema;
        CITableFileMetaDataPtr metaData;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( wideFile, CValueList(), error, &schema, GUCEF_NULL, 0, &metaData ) && !metaData.IsNULL() );
        ASSERT_TRUE( DATATABLE_TYPE_FLOAT64 == schema->GetColumn( 0 ).type );
        CValueList keyValues;
        metaData->GetKeyValueMetaData( keyValues );
        ASSERT_TRUE( 1 == keyValues.GetValueAlways( CString( "lossyInference" ) ).AsUInt32() && 1 == keyValues.GetValueAlways( "lossyInference.v" ).AsUInt32() );

        // exponent notation is read exactly too
        CXlsxBuilder exponent;
        exponent.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "v" ) ) + Row( 2, CellNumber( "A2", "1.5E-3" ) ) + Row( 3, CellNumber( "A3", "2E2" ) ) ) );
        ASSERT_TRUE( CheckTable( "exponent", MakeFile( exponent ), CValueList(), "v:decimal(7,4)", "0.0015\n200.0000\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: strings, inline strings, formula strings, rich text, phonetic text and escapes" )
    try
    {
        CXlsxBuilder builder;
        builder.SetSharedStringsXml( CString( "<?xml version=\"1.0\"?><sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                                              "<si><t>h</t></si>"                                                         // 0
                                              "<si><r><t>Hello</t></r><r><t>World</t></r></si>"                           // 1 rich text
                                              "<si><t>Kanji</t><rPh sb=\"0\" eb=\"1\"><t>ignored</t></rPh></si>"          // 2 phonetic run
                                              "<si><t>x_x000D_y</t></si>"                                                 // 3 escape
                                              "<si><t></t></si>"                                                          // 4 empty: no value
                                              "<si><t xml:space=\"preserve\">  a   b  </t></si>"                          // 5 white space
                                              "<si><t>a &amp; b &lt;c&gt;</t></si>"                                       // 6 entities
                                              "</sst>" ) );
        CString rows;
        rows += Row( 1, CellShared( "A1", 0 ) );
        rows += Row( 2, CellShared( "A2", 1 ) );
        rows += Row( 3, CellShared( "A3", 2 ) );
        rows += Row( 4, CellShared( "A4", 3 ) );
        rows += Row( 5, CellShared( "A5", 4 ) );
        rows += Row( 6, CellInline( "A6", "inline" ) );
        rows += Row( 7, CellFormulaString( "A7", "UPPER(\"x\")", "X" ) );
        rows += Row( 8, CellInline( "A8", "tab_x0009_end" ) );
        rows += Row( 9, CellShared( "A9", 5 ) );
        rows += Row( 10, CellShared( "A10", 6 ) );
        builder.AddSheet( "S", Sheet( rows ) );
        // the text of an element goes through the XML reader of the data store codecs, which normalizes its white space:
        // "  a   b  " is read as "a b"
        ASSERT_TRUE( CheckTable( "strings", MakeFile( builder ), CValueList(), "h:utf8",
                                 "HelloWorld\nKanji\nx\ry\ninline\nX\ntab\tend\na b\na & b <c>\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: dates of the 1900 system, with a leap day that does not exist, and the time of day" )
    try
    {
        CXlsxBuilder builder;
        builder.SetStyles( DateStyles() );
        CString rows;
        rows += Row( 1, CellInline( "A1", "when" ) + CellInline( "B1", "at" ) + CellInline( "C1", "clock" ) );
        rows += Row( 2, CellNumber( "A2", "44927", 1 ) + CellNumber( "B2", "44927.5", 2 ) + CellNumber( "C2", "0.5", 4 ) );
        rows += Row( 3, CellNumber( "A3", "1", 1 ) + CellNumber( "B3", "44927.708333333336", 2 ) + CellNumber( "C3", "0.25", 4 ) );
        rows += Row( 4, CellNumber( "A4", "59", 1 ) + CellNumber( "B4", "44927", 2 ) );
        rows += Row( 5, CellNumber( "A5", "61", 1 ) + CellNumber( "B5", "44927.999999999999", 2 ) );
        rows += Row( 6, CellNumber( "A6", "25569", 1 ) );
        rows += Row( 7, CellNumber( "A7", "2958465", 1 ) );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );

        // A: dates; B: dates with a time of day, to the microsecond; C: times of a day are numbers, not dates
        ASSERT_TRUE( CheckTable( "dates", file, CValueList(), "when:date|at:timestamp(us)|clock:decimal(3,2)",
                                 "2023-01-01|2023-01-01T12:00:00|0.50\n1900-01-01|2023-01-01T17:00:00|0.25\n1900-02-28|2023-01-01T00:00:00|~\n"
                                 "1900-03-01|2023-01-02T00:00:00|~\n1970-01-01|~|~\n9999-12-31|~|~\n" ) );

        // the date value is the days since 1970 and the timestamp the microseconds, as stored
        CString error;
        CDataTableSchemaPtr schema;
        TBatchVector batches;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, &schema, &batches ) );
        REQUIRE( 1 == batches.size() );
        const Int32* dateValues = batches[ 0 ]->GetColumn( 0 ).GetInt32Data();
        const Int64* timeValues = batches[ 0 ]->GetColumn( 1 ).GetInt64Data();
        REQUIRE( GUCEF_NULL != dateValues && GUCEF_NULL != timeValues && batches[ 0 ]->GetRowCount() >= 2 );
        ASSERT_TRUE( 19358 == dateValues[ 0 ] && -25567 == dateValues[ 1 ] );
        ASSERT_TRUE( 19358LL * 86400000000LL + 43200000000LL == timeValues[ 0 ] );
        ASSERT_FALSE( schema->GetColumn( 1 ).utcAdjusted );

        // the 29th of February 1900 is no date, it is a value error
        CXlsxBuilder leap;
        leap.SetStyles( DateStyles() );
        leap.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "d" ) ) + Row( 2, CellNumber( "A2", "44927", 1 ) ) + Row( 3, CellNumber( "A3", "60", 1 ) ) ) );
        CDynamicBuffer leapFile = MakeFile( leap );
        ASSERT_TRUE( CheckFailure( "leap day", leapFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "A3" ) );
        ASSERT_TRUE( CheckFailure( "leap day text", leapFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "1900 date system" ) );
        CValueList lenient;
        SetOption( lenient, "onValueError", "null" );
        ASSERT_TRUE( CheckTable( "leap day null", leapFile, lenient, "d:date", "2023-01-01\n~\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the 1904 date system: the same numbers are other dates" )
    try
    {
        CString rows;
        rows += Row( 1, CellInline( "A1", "d" ) );
        rows += Row( 2, CellNumber( "A2", "1462", 1 ) );
        rows += Row( 3, CellNumber( "A3", "0", 1 ) );

        CXlsxBuilder system1900;
        system1900.SetStyles( DateStyles() );
        system1900.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "d" ) ) + Row( 2, CellNumber( "A2", "1462", 1 ) ) ) );
        ASSERT_TRUE( CheckTable( "1900", MakeFile( system1900 ), CValueList(), "d:date", "1904-01-01\n" ) );

        CXlsxBuilder system1904;
        system1904.SetDate1904( true ).SetStyles( DateStyles() );
        system1904.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( system1904 );
        ASSERT_TRUE( CheckTable( "1904", file, CValueList(), "d:date", "1908-01-02\n1904-01-01\n" ) );

        CString error;
        CITableFileMetaDataPtr metaData;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, GUCEF_NULL, GUCEF_NULL, 0, &metaData ) && !metaData.IsNULL() );
        CValueList keyValues;
        metaData->GetKeyValueMetaData( keyValues );
        ASSERT_TRUE( "1904" == keyValues.GetValueAlways( "dateSystem" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: dates of type d (ISO text), and dates read as text and numbers in typed columns" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "iso" ) + CellInline( "B1", "stamp" ) );
        rows += Row( 2, CellIsoDate( "A2", "2023-01-01" ) + CellIsoDate( "B2", "2023-01-01T10:20:30" ) );
        rows += Row( 3, CellIsoDate( "A3", "2024-02-29" ) + CellIsoDate( "B3", "2024-02-29T00:00:00.5" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        ASSERT_TRUE( CheckTable( "iso", MakeFile( builder ), CValueList(), "iso:date|stamp:timestamp(us)",
                                 "2023-01-01|2023-01-01T10:20:30\n2024-02-29|2024-02-29T00:00:00.500000\n" ) );

        // dates given as text, in a column that is declared a date; a number in a column declared a date is a serial date
        CXlsxBuilder typed;
        CString typedRows;
        typedRows += Row( 1, CellInline( "A1", "d" ) );
        typedRows += Row( 2, CellInline( "A2", "2023-05-17" ) );
        typedRows += Row( 3, CellNumber( "A3", "44927" ) );
        typed.AddSheet( "S", Sheet( typedRows ) );
        CValueList options;
        SetOption( options, "columnTypes.d", "date" );
        ASSERT_TRUE( CheckTable( "typed", MakeFile( typed ), options, "d:date", "2023-05-17\n2023-01-01\n" ) );

        // a column that mixes dates and text is text
        CXlsxBuilder mixed;
        mixed.SetStyles( DateStyles() );
        mixed.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "m" ) ) + Row( 2, CellNumber( "A2", "44927", 1 ) ) + Row( 3, CellInline( "A3", "n/a" ) ) + Row( 4, CellNumber( "A4", "44927.5", 2 ) ) ) );
        ASSERT_TRUE( CheckTable( "mixed", MakeFile( mixed ), CValueList(), "m:utf8", "2023-01-01\nn/a\n2023-01-01T12:00:00\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: blank cells, rows without cells, gaps between rows and ragged rows" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "a" ) + CellInline( "B1", "b" ) + CellInline( "C1", "c" ) );
        rows += Row( 2, CellNumber( "A2", "1" ) );
        rows += Row( 3, "" );
        rows += Row( 5, CellNumber( "A5", "2" ) + CellNumber( "C5", "7" ) );
        rows += Row( 6, "<c r=\"A6\"/>" + CellNumber( "B6", "3" ) );
        rows += Row( 8, "<c r=\"A8\"/><c r=\"B8\"/>" );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );
        ASSERT_TRUE( CheckTable( "ragged", file, CValueList(), "a:int64|b:int64|c:int64", "1|~|~\n2|~|7\n~|3|~\n" ) );

        CString error;
        CITableFileMetaDataPtr metaData;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, GUCEF_NULL, GUCEF_NULL, 0, &metaData ) && !metaData.IsNULL() );
        ASSERT_TRUE( 3 == metaData->GetRowCount() );
        ASSERT_TRUE( metaData->GetSchema()->GetColumn( 0 ).nullable && metaData->GetSchema()->GetColumn( 2 ).nullable );

        // a header and no rows is a table of no rows
        CXlsxBuilder headerOnly;
        headerOnly.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) + CellInline( "B1", "b" ) ) ) );
        ASSERT_TRUE( CheckTable( "header only", MakeFile( headerOnly ), CValueList(), "a:utf8|b:utf8", "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: several sheets by name or index, and a sheet that holds no cells" )
    try
    {
        CXlsxBuilder builder;
        builder.AddSheet( "First", Sheet( Row( 1, CellInline( "A1", "one" ) ) + Row( 2, CellNumber( "A2", "1" ) ) ) );
        builder.AddSheet( "Second", Sheet( Row( 1, CellInline( "A1", "two" ) ) + Row( 2, CellNumber( "A2", "2" ) ) ) );
        builder.AddSheet( "Third & more", Sheet( Row( 1, CellInline( "A1", "three" ) ) + Row( 2, CellNumber( "A2", "3" ) ) ) );
        builder.AddChartSheet( "Chart" );
        CDynamicBuffer file = MakeFile( builder );

        ASSERT_TRUE( CheckTable( "default", file, CValueList(), "one:int64", "1\n" ) );
        CValueList options;
        SetOption( options, "sheetName", "Second" );
        ASSERT_TRUE( CheckTable( "by name", file, options, "two:int64", "2\n" ) );
        CValueList byIndex;
        SetOption( byIndex, "sheetIndex", "2" );
        ASSERT_TRUE( CheckTable( "by index", file, byIndex, "three:int64", "3\n" ) );
        CValueList escaped;
        SetOption( escaped, "sheetName", "Third & more" );
        ASSERT_TRUE( CheckTable( "name with an ampersand", file, escaped, "three:int64", "3\n" ) );

        CValueList missing;
        SetOption( missing, "sheetName", "Nope" );
        ASSERT_TRUE( CheckFailure( "no such name", file, missing, DATATABLE_STATUS_NOT_FOUND, "no sheet named \"Nope\"" ) );
        CValueList beyond;
        SetOption( beyond, "sheetIndex", "4" );
        ASSERT_TRUE( CheckFailure( "no such index", file, beyond, DATATABLE_STATUS_NOT_FOUND, "4 sheets" ) );
        CValueList chart;
        SetOption( chart, "sheetIndex", "3" );
        ASSERT_TRUE( CheckFailure( "chart sheet", file, chart, DATATABLE_STATUS_UNSUPPORTED, "no worksheet" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: merged cells are ignored: only the first cell of the range has a value" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "h1" ) + CellInline( "B1", "h2" ) + CellInline( "C1", "h3" ) );
        rows += Row( 2, CellInline( "A2", "merged" ) + "<c r=\"B2\" s=\"0\"/>" + CellNumber( "C2", "1" ) );
        rows += Row( 3, CellInline( "A3", "x" ) + CellInline( "B3", "y" ) + CellNumber( "C3", "2" ) );
        builder.AddSheet( "S", Sheet( rows, "<mergeCells count=\"1\"><mergeCell ref=\"A2:B2\"/></mergeCells>" ) );
        ASSERT_TRUE( CheckTable( "merged", MakeFile( builder ), CValueList(), "h1:utf8|h2:utf8|h3:int64", "merged|~|1\nx|y|2\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: formulas give their cached value; a formula without one, and an error value, are value errors" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "a" ) + CellInline( "B1", "b" ) + CellInline( "C1", "product" ) + CellInline( "D1", "word" ) );
        rows += Row( 2, CellNumber( "A2", "2" ) + CellNumber( "B2", "3" ) + CellFormulaNumber( "C2", "A2*B2", "6" ) + CellFormulaString( "D2", "\"six\"", "six" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        ASSERT_TRUE( CheckTable( "cached", MakeFile( builder ), CValueList(), "a:int64|b:int64|product:int64|word:utf8", "2|3|6|six\n" ) );

        CXlsxBuilder uncached;
        CString uncachedRows;
        uncachedRows += Row( 1, CellInline( "A1", "a" ) + CellInline( "B1", "b" ) );
        uncachedRows += Row( 2, CellNumber( "A2", "1" ) + CellFormulaNumber( "B2", "A2+1", "2" ) );
        uncachedRows += Row( 3, CellNumber( "A3", "4" ) + CellFormulaOnly( "B3", "A3+1" ) );
        uncached.AddSheet( "S", Sheet( uncachedRows ) );
        CDynamicBuffer uncachedFile = MakeFile( uncached );
        ASSERT_TRUE( CheckFailure( "no cached value", uncachedFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "never calculated" ) );
        ASSERT_TRUE( CheckFailure( "no cached value cell", uncachedFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "B3" ) );
        CValueList lenient;
        SetOption( lenient, "onValueError", "null" );
        ASSERT_TRUE( CheckTable( "no cached value null", uncachedFile, lenient, "a:int64|b:int64", "1|2\n4|~\n" ) );

        CXlsxBuilder errors;
        CString errorRows;
        errorRows += Row( 1, CellInline( "A1", "a" ) );
        errorRows += Row( 2, CellNumber( "A2", "1" ) );
        errorRows += Row( 3, CellError( "A3", "#DIV/0!" ) );
        errors.AddSheet( "S", Sheet( errorRows ) );
        CDynamicBuffer errorFile = MakeFile( errors );
        ASSERT_TRUE( CheckFailure( "error value", errorFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "#DIV/0!" ) );
        ASSERT_TRUE( CheckTable( "error value null", errorFile, lenient, "a:int64", "1\n~\n" ) );

        // an error value is never text: also not in a column of text
        CXlsxBuilder textErrors;
        textErrors.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "t" ) ) + Row( 2, CellInline( "A2", "word" ) ) + Row( 3, CellError( "A3", "#N/A" ) ) ) );
        CDynamicBuffer textErrorFile = MakeFile( textErrors );
        ASSERT_TRUE( CheckFailure( "error value in text", textErrorFile, CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "error value" ) );
        ASSERT_TRUE( CheckTable( "error value in text null", textErrorFile, lenient, "t:utf8", "word\n~\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the header row: the first row with a value, or the row, the first row or the columns of the options" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "Report of the quarter" ) );
        rows += Row( 3, CellInline( "A3", "x" ) + CellInline( "B3", "y" ) );
        rows += Row( 4, CellNumber( "A4", "1" ) + CellNumber( "B4", "2" ) );
        rows += Row( 5, CellNumber( "A5", "3" ) + CellNumber( "B5", "4" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );

        // the first row with a value is the header, and the cells right of it belong to no column
        ASSERT_TRUE( CheckFailure( "title is the header", file, CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "allowExtraCells" ) );

        CValueList headerRow;
        SetOption( headerRow, "headerRow", "3" );
        ASSERT_TRUE( CheckTable( "headerRow", file, headerRow, "x:int64|y:int64", "1|2\n3|4\n" ) );
        CValueList firstRow;
        SetOption( firstRow, "firstRow", "2" );
        ASSERT_TRUE( CheckTable( "firstRow", file, firstRow, "x:int64|y:int64", "1|2\n3|4\n" ) );

        CValueList emptyHeader;
        SetOption( emptyHeader, "headerRow", "2" );
        ASSERT_TRUE( CheckFailure( "empty header row", file, emptyHeader, DATATABLE_STATUS_CORRUPT_DATA, "header row (row 2) has no values" ) );

        CValueList extra;
        SetOption( extra, "allowExtraCells", "true" );
        ASSERT_TRUE( CheckTable( "extra cells", file, extra, "Report of the quarter:utf8", "x\n1\n3\n" ) );

        CValueList noHeader;
        SetOption( noHeader, "hasHeader", "false" );
        SetOption( noHeader, "firstRow", "4" );
        ASSERT_TRUE( CheckTable( "no header", file, noHeader, "column_1:int64|column_2:int64", "1|2\n3|4\n" ) );

        // rows at the top that have no value are not the header: the first row with a value is
        CXlsxBuilder blankTop;
        blankTop.AddSheet( "S", Sheet( Row( 1, "" ) + Row( 2, "<c r=\"A2\"/>" ) + Row( 3, CellInline( "A3", "x" ) + CellInline( "B3", "y" ) ) + Row( 4, CellNumber( "A4", "1" ) + CellNumber( "B4", "2" ) ) ) );
        ASSERT_TRUE( CheckTable( "blank rows on top", MakeFile( blankTop ), CValueList(), "x:int64|y:int64", "1|2\n" ) );

        // the columns of the table: C to D
        CXlsxBuilder shifted;
        CString shiftedRows;
        shiftedRows += Row( 1, CellInline( "C1", "p" ) + CellInline( "D1", "q" ) + CellInline( "E1", "r" ) );
        shiftedRows += Row( 2, CellNumber( "B2", "99" ) + CellNumber( "C2", "1" ) + CellNumber( "D2", "2" ) + CellNumber( "E2", "3" ) );
        shifted.AddSheet( "S", Sheet( shiftedRows ) );
        CDynamicBuffer shiftedFile = MakeFile( shifted );
        CValueList columns;
        SetOption( columns, "firstColumn", "C" );
        ASSERT_TRUE( CheckTable( "firstColumn", shiftedFile, columns, "p:int64|q:int64|r:int64", "1|2|3\n" ) );
        SetOption( columns, "lastColumn", "D" );
        ASSERT_TRUE( CheckTable( "lastColumn", shiftedFile, columns, "p:int64|q:int64", "1|2\n" ) );

        // empty and repeated names are named as the CSV codec names them
        CXlsxBuilder names;
        names.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) + CellInline( "C1", "a" ) + CellInline( "D1", "  b\tc " ) ) + Row( 2, CellNumber( "A2", "1" ) + CellNumber( "B2", "2" ) + CellNumber( "C2", "3" ) + CellNumber( "D2", "4" ) ) ) );
        ASSERT_TRUE( CheckTable( "names", MakeFile( names ), CValueList(), "a:int64|column_2:int64|a_2:int64|b c:int64", "1|2|3|4\n" ) );

        // a sheet without cells
        CXlsxBuilder empty;
        empty.AddSheet( "S", Sheet( "" ) );
        ASSERT_TRUE( CheckFailure( "empty sheet", MakeFile( empty ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "no header row" ) );
        CXlsxBuilder emptyNoHeader;
        emptyNoHeader.AddSheet( "S", Sheet( "" ) );
        CValueList noHeaderOption;
        SetOption( noHeaderOption, "hasHeader", "false" );
        ASSERT_TRUE( CheckFailure( "empty sheet, no header", MakeFile( emptyNoHeader ), noHeaderOption, DATATABLE_STATUS_CORRUPT_DATA, "no rows" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: the schema of the options, type overrides, null tokens and the rows that are inferred" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "id" ) + CellInline( "B1", "name" ) + CellInline( "C1", "qty" ) );
        rows += Row( 2, CellNumber( "A2", "1" ) + CellInline( "B2", "one" ) + CellNumber( "C2", "5" ) );
        rows += Row( 3, CellNumber( "A3", "2" ) + CellInline( "B3", "N/A" ) + CellInline( "C3", "-" ) );
        rows += Row( 4, CellNumber( "A4", "3" ) + CellInline( "B4", "three" ) + CellNumber( "C4", "42" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );

        // inferred: qty has a text in it, so it is all text
        ASSERT_TRUE( CheckTable( "inferred", file, CValueList(), "id:int64|name:utf8|qty:utf8", "1|one|5\n2|N/A|-\n3|three|42\n" ) );

        // the null tokens are nulls, and qty is a number again
        CValueList tokens;
        AddOption( tokens, "nullTokens", "N/A" );
        AddOption( tokens, "nullTokens", "-" );
        ASSERT_TRUE( CheckTable( "null tokens", file, tokens, "id:int64|name:utf8|qty:int64", "1|one|5\n2|~|~\n3|three|42\n" ) );

        // an override: the numbers are read as decimals of the column
        CValueList typed;
        SetOption( typed, "columnTypes.qty", "decimal(10,2)" );
        AddOption( typed, "nullTokens", "-" );
        ASSERT_TRUE( CheckTable( "override", file, typed, "id:int64|name:utf8|qty:decimal(10,2)", "1|one|5.00\n2|N/A|~\n3|three|42.00\n" ) );

        // the schema of the options: no inference, a column that is not nullable
        CValueList schema;
        AddOption( schema, "schema", "id|int64!" );
        AddOption( schema, "schema", "name|utf8" );
        AddOption( schema, "schema", "qty|float64" );
        AddOption( schema, "nullTokens", "-" );
        ASSERT_TRUE( CheckTable( "schema", file, schema, "id:int64|name:utf8|qty:float64", "1|one|5\n2|N/A|~\n3|three|42\n" ) );
        CString error;
        CDataTableSchemaPtr read;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, schema, error, &read ) && !read.IsNULL() );
        ASSERT_FALSE( read->GetColumn( 0 ).nullable );
        ASSERT_TRUE( read->GetColumn( 1 ).nullable );

        CValueList notNull;
        AddOption( notNull, "schema", "id|int64!" );
        AddOption( notNull, "schema", "name|utf8!" );
        AddOption( notNull, "schema", "qty|utf8" );
        CXlsxBuilder blank;
        blank.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "id" ) + CellInline( "B1", "name" ) + CellInline( "C1", "qty" ) ) + Row( 2, CellNumber( "A2", "1" ) + CellNumber( "C2", "5" ) ) ) );
        ASSERT_TRUE( CheckFailure( "not nullable", MakeFile( blank ), notNull, DATATABLE_STATUS_TYPE_MISMATCH, "not nullable" ) );
        ASSERT_TRUE( CheckFailure( "not nullable cell", MakeFile( blank ), notNull, DATATABLE_STATUS_TYPE_MISMATCH, "B2" ) );

        CValueList tooFew;
        AddOption( tooFew, "schema", "id|int64" );
        ASSERT_TRUE( CheckFailure( "schema columns", file, tooFew, DATATABLE_STATUS_SCHEMA_MISMATCH, "3 columns where the schema of the options has 1" ) );

        // a type override of a column that is not there
        CValueList unknown;
        SetOption( unknown, "columnTypes.nothing", "int64" );
        ASSERT_TRUE( CheckFailure( "override of an unknown column", file, unknown, DATATABLE_STATUS_INVALID_ARGUMENT, "nothing" ) );

        // inference over the first rows only: what comes later has to fit
        CXlsxBuilder late;
        late.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "v" ) ) + Row( 2, CellNumber( "A2", "1" ) ) + Row( 3, CellNumber( "A3", "2" ) ) + Row( 4, CellInline( "A4", "abc" ) ) ) );
        CDynamicBuffer lateFile = MakeFile( late );
        ASSERT_TRUE( CheckTable( "all rows", lateFile, CValueList(), "v:utf8", "1\n2\nabc\n" ) );
        CValueList limited;
        SetOption( limited, "inferRowCount", "2" );
        ASSERT_TRUE( CheckFailure( "inferRowCount", lateFile, limited, DATATABLE_STATUS_TYPE_MISMATCH, "abc" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: booleans, and columns that mix kinds of cells are text" )
    try
    {
        CXlsxBuilder builder;
        CString rows;
        rows += Row( 1, CellInline( "A1", "flag" ) + CellInline( "B1", "mixed" ) + CellInline( "C1", "yesno" ) );
        rows += Row( 2, CellBool( "A2", true ) + CellBool( "B2", true ) + CellInline( "C2", "TRUE" ) );
        rows += Row( 3, CellBool( "A3", false ) + CellNumber( "B3", "1" ) + CellInline( "C3", "false" ) );
        builder.AddSheet( "S", Sheet( rows ) );
        CDynamicBuffer file = MakeFile( builder );
        ASSERT_TRUE( CheckTable( "booleans", file, CValueList(), "flag:bool|mixed:utf8|yesno:utf8", "true|true|TRUE\nfalse|1|false\n" ) );

        CValueList typed;
        SetOption( typed, "columnTypes.yesno", "bool" );
        ASSERT_TRUE( CheckTable( "text booleans", file, typed, "flag:bool|mixed:utf8|yesno:bool", "true|true|true\nfalse|1|false\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: XML with namespace prefixes and sheets without a shared strings or a styles part" )
    try
    {
        const CString sheet = "<?xml version=\"1.0\"?><x:worksheet xmlns:x=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><x:sheetData>"
                              "<x:row r=\"1\"><x:c r=\"A1\" t=\"inlineStr\"><x:is><x:t>h</x:t></x:is></x:c></x:row>"
                              "<x:row r=\"2\"><x:c r=\"A2\"><x:v>7.5</x:v></x:c></x:row>"
                              "</x:sheetData></x:worksheet>";
        CXlsxBuilder builder;
        builder.AddSheet( "S", sheet );
        ASSERT_TRUE( CheckTable( "prefixed", MakeFile( builder ), CValueList(), "h:decimal(2,1)", "7.5\n" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: batches of a sheet of thousands of rows, the projection, the origin and the bytes read" )
    try
    {
        CXlsxBuilder builder;
        builder.AddSheet( "Big", BigSheet( 5000 ) );
        CDynamicBuffer file = MakeFile( builder );

        CString error;
        CDataTableSchemaPtr schema;
        TBatchVector batches;
        CITableFileMetaDataPtr metaData;
        REQUIRE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, &schema, &batches, 1024, &metaData ) && !metaData.IsNULL() );
        ASSERT_TRUE( "id:int64|amount:decimal(5,2)" == DescribeSchema( *schema ) );
        REQUIRE( 5 == batches.size() && 5000 == CountRows( batches ) && 5000 == metaData->GetRowCount() );
        ASSERT_TRUE( 1024 == batches[ 0 ]->GetRowCount() && 1024 == batches[ 3 ]->GetRowCount() && 904 == batches[ 4 ]->GetRowCount() );
        ASSERT_TRUE( 0 == batches[ 0 ]->GetOrigin().firstRowInRowGroup && 1024 == batches[ 1 ]->GetOrigin().firstRowInRowGroup && 4096 == batches[ 4 ]->GetOrigin().firstRowInRowGroup );
        REQUIRE( GUCEF_NULL != batches[ 1 ]->GetColumn( 0 ).GetInt64Data() && GUCEF_NULL != batches[ 4 ]->GetColumn( 0 ).GetInt64Data() &&
                 GUCEF_NULL != batches[ 1 ]->GetColumn( 1 ).GetInt64Data() && GUCEF_NULL != batches[ 4 ]->GetColumn( 1 ).GetInt64Data() && batches[ 4 ]->GetRowCount() > 903 );
        ASSERT_TRUE( 1024 == batches[ 1 ]->GetColumn( 0 ).GetInt64Data()[ 0 ] && 4999 == batches[ 4 ]->GetColumn( 0 ).GetInt64Data()[ 903 ] );
        ASSERT_TRUE( 2425 == batches[ 1 ]->GetColumn( 1 ).GetInt64Data()[ 0 ] );      // 24.25 as a mantissa of scale 2
        ASSERT_TRUE( 99925 == batches[ 4 ]->GetColumn( 1 ).GetInt64Data()[ 903 ] );

        // the projection: only the second column, in the delivery order asked for
        CITableReaderPtr reader;
        REQUIRE( DATATABLE_STATUS_OK == GetXlsxCodec().CreateReader( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( file ) ), metaData, CValueList(), reader, error ) );
        CDataTableReadRequest request;
        request.batchRowCount = 2048;
        request.projection.push_back( 1 );
        CDataTableScanStats stats;
        request.stats = &stats;
        REQUIRE( DATATABLE_STATUS_OK == reader->OpenRowGroup( 0, request ) );
        UInt32 batchCount = 0;
        UInt64 rows = 0;
        CDataTableBatchPtr batch;
        while ( DATATABLE_STATUS_OK == reader->ReadNextBatch( batch ) )
        {
            ASSERT_TRUE( 1 == batch->GetColumnCount() && 1 == batch->GetSchemaColumnIndex( 0 ) );
            ASSERT_TRUE( DATATABLE_TYPE_DECIMAL == batch->GetColumn( 0 ).GetLogicalType() );
            rows += batch->GetRowCount();
            ++batchCount;
        }
        ASSERT_TRUE( 3 == batchCount && 5000 == rows );
        ASSERT_TRUE( stats.bytesRead > 0 );
        ASSERT_TRUE( DATATABLE_STATUS_END == reader->ReadNextBatch( batch ) );

        // a second row group does not exist, and a batch cannot be read before a row group is opened
        CITableReaderPtr second;
        REQUIRE( DATATABLE_STATUS_OK == GetXlsxCodec().CreateReader( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( file ) ), metaData, CValueList(), second, error ) );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == second->ReadNextBatch( batch ) );
        ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == second->OpenRowGroup( 1, CDataTableReadRequest() ) );
        CDataTableReadRequest bad;
        bad.projection.push_back( 7 );
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == second->OpenRowGroup( 0, bad ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Codec: options that are not valid are refused with the reason" )
    try
    {
        CXlsxBuilder builder;
        builder.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "A2", "1" ) ) ) );
        CDynamicBuffer file = MakeFile( builder );

        CValueList options;
        SetOption( options, "sheetName", "S" );
        SetOption( options, "sheetIndex", "0" );
        ASSERT_TRUE( CheckFailure( "name and index", file, options, DATATABLE_STATUS_INVALID_ARGUMENT, "select the sheet both" ) );

        CValueList headerWithout;
        SetOption( headerWithout, "hasHeader", "false" );
        SetOption( headerWithout, "headerRow", "1" );
        ASSERT_TRUE( CheckFailure( "headerRow without header", file, headerWithout, DATATABLE_STATUS_INVALID_ARGUMENT, "headerRow" ) );

        CValueList column;
        SetOption( column, "firstColumn", "A1" );
        ASSERT_TRUE( CheckFailure( "column", file, column, DATATABLE_STATUS_INVALID_ARGUMENT, "firstColumn" ) );

        CValueList order;
        SetOption( order, "firstColumn", "C" );
        SetOption( order, "lastColumn", "B" );
        ASSERT_TRUE( CheckFailure( "column order", file, order, DATATABLE_STATUS_INVALID_ARGUMENT, "lastColumn" ) );

        CValueList type;
        AddOption( type, "schema", "a|foo" );
        ASSERT_TRUE( CheckFailure( "unknown type", file, type, DATATABLE_STATUS_INVALID_ARGUMENT, "unknown type" ) );

        CValueList binary;
        SetOption( binary, "columnTypes.a", "binary" );
        ASSERT_TRUE( CheckFailure( "binary", file, binary, DATATABLE_STATUS_INVALID_ARGUMENT, "cannot be read from a worksheet" ) );

        CValueList both;
        AddOption( both, "schema", "a|int64" );
        SetOption( both, "columnTypes.a", "int64" );
        ASSERT_TRUE( CheckFailure( "schema and types", file, both, DATATABLE_STATUS_INVALID_ARGUMENT, "give one" ) );

        const char* const badValues[][2] = { { "maxRows", "0" }, { "maxRows", "x" }, { "maxCells", "0" }, { "onValueError", "maybe" }, { "decimalRounding", "sideways" },
                                             { "hasHeader", "perhaps" }, { "sheetIndex", "-1" }, { "inferRowCount", "1.5" } };
        for ( UInt32 i = 0; i < sizeof( badValues ) / sizeof( badValues[ 0 ] ); ++i )
        {
            CValueList bad;
            SetOption( bad, badValues[ i ][ 0 ], badValues[ i ][ 1 ] );
            ASSERT_TRUE( CheckFailure( badValues[ i ][ 0 ], file, bad, DATATABLE_STATUS_INVALID_ARGUMENT, badValues[ i ][ 0 ] ) );
        }

        // the reader refuses metadata of another codec and a missing source
        CITableReaderPtr reader;
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == GetXlsxCodec().CreateReader( IOAccessPtr(), CITableFileMetaDataPtr(), CValueList(), reader, error ) );

        // nothing was left behind by any of the reads of this suite
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
