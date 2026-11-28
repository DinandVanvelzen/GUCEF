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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#include "tablecodecpluginXLSX_CXlsxDates.h"
#define TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXDATES_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H
#include "tablecodecpluginXLSX_CXlsxWorkbook.h"
#define TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H
#include "tablecodecpluginXLSX_CXlsxSheetParser.h"
#define TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#include "tablecodecpluginXLSX_CXlsxColumnConverter.h"
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#include "tablecodecpluginXLSX_CXlsxPackage.h"
#define TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H ? */

#include "TestXlsxSupport.h"
#include "TestXlsxDates.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

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

bool
SerialIs1900( Int64 serial, Int32 expectedDays )
{
    Int32 days = 0;
    return CXlsxDates::SerialToDays( serial, false, days ) && expectedDays == days;
}

bool
SerialIs1904( Int64 serial, Int32 expectedDays )
{
    Int32 days = 0;
    return CXlsxDates::SerialToDays( serial, true, days ) && expectedDays == days;
}

bool
IsoIs( const char* text, Int32 expectedDays, Int64 expectedMicros, bool expectedTime )
{
    Int32 days = 0;
    Int64 micros = 0;
    bool hasTime = false;
    return CXlsxDates::ParseIso( text, (UInt32) strlen( text ), days, micros, hasTime ) && expectedDays == days && expectedMicros == micros && expectedTime == hasTime;
}

bool
IsoFails( const char* text )
{
    Int32 days = 0;
    Int64 micros = 0;
    bool hasTime = false;
    return !CXlsxDates::ParseIso( text, (UInt32) strlen( text ), days, micros, hasTime );
}

CString
Decoded( const char* text )
{
    CString decoded;
    CXlsxWorkbook::DecodeEscapes( text, (UInt32) strlen( text ), decoded );
    return decoded;
}

bool
Target( const char* base, const char* target, const char* expected )
{
    CString part;
    return CXlsxPackage::ResolveTarget( base, target, part ) && part == expected;
}

bool
TargetFails( const char* base, const char* target )
{
    CString part;
    return !CXlsxPackage::ResolveTarget( base, target, part );
}


/** Records what the XML reader of the data store codecs reports of a document */
class CXmlRecorder : public CXlsxXmlCallbacks
{
    public:

    CString events;

    virtual void OnNodeBegin( const CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE { events += "<" + nodeId.AsString() + ">"; }
    virtual void OnNodeEnd( const CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE { events += "</" + nodeId.AsString() + ">"; }
    virtual void OnNodeAttribute( const CVariant& nodeId, const CVariant& attributeId, const CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        const char* text = attributeValue.AsCharPtr( "?" );
        events += "[" + attributeId.AsString() + "=" + CString( text, attributeValue.ByteSize( false ) ) + "]";
    }
    virtual void OnNodeValue( const CVariant& nodeId, const CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        const char* text = nodeValue.AsCharPtr( "?" );
        events += "{" + CString( text, nodeValue.ByteSize( false ) ) + "}";
    }
};

}

/*-------------------------------------------------------------------------*/

void
PerformXlsxHelperTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginXLSX.Helpers" );

    GUCEF_TESTFW_TESTCASE( "Dates: serial dates of the 1900 system, with the 29th of February 1900 that never existed" )
    try
    {
        ASSERT_TRUE( SerialIs1900( 25569, 0 ) );             // 1970-01-01
        ASSERT_TRUE( SerialIs1900( 44927, 19358 ) );         // 2023-01-01
        ASSERT_TRUE( SerialIs1900( 1, -25567 ) );            // 1900-01-01
        ASSERT_TRUE( SerialIs1900( 59, -25509 ) );           // 1900-02-28
        ASSERT_TRUE( SerialIs1900( 61, -25508 ) );           // 1900-03-01, the day after the day that does not exist
        ASSERT_TRUE( SerialIs1900( 2958465, CXlsxDates::MaxDays ) );    // 9999-12-31
        Int32 days = 0;
        ASSERT_FALSE( CXlsxDates::SerialToDays( 60, false, days ) );    // 1900-02-29
        ASSERT_FALSE( CXlsxDates::SerialToDays( 0, false, days ) );
        ASSERT_FALSE( CXlsxDates::SerialToDays( -1, false, days ) );
        ASSERT_FALSE( CXlsxDates::SerialToDays( 2958466, false, days ) );
        ASSERT_TRUE( "1900-01-01" == CXlsxDates::ToIsoText( -25567, 0 ) );
        ASSERT_TRUE( "1900-03-01" == CXlsxDates::ToIsoText( -25508, 0 ) );
        ASSERT_TRUE( "2023-01-01" == CXlsxDates::ToIsoText( 19358, 0 ) );
        ASSERT_TRUE( "9999-12-31" == CXlsxDates::ToIsoText( CXlsxDates::MaxDays, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Dates: serial dates of the 1904 system" )
    try
    {
        ASSERT_TRUE( SerialIs1904( 0, -24107 ) );            // 1904-01-01
        ASSERT_TRUE( SerialIs1904( 24107, 0 ) );             // 1970-01-01
        ASSERT_TRUE( SerialIs1904( 2957003, CXlsxDates::MaxDays ) );
        Int32 days = 0;
        ASSERT_FALSE( CXlsxDates::SerialToDays( -1, true, days ) );
        ASSERT_FALSE( CXlsxDates::SerialToDays( 2957004, true, days ) );
        ASSERT_TRUE( "1904-01-01" == CXlsxDates::ToIsoText( -24107, 0 ) );

        // the same moment is 1462 apart in the two systems
        Int32 days1900 = 0, days1904 = 0;
        ASSERT_TRUE( CXlsxDates::SerialToDays( 44927, false, days1900 ) && CXlsxDates::SerialToDays( 44927 - 1462, true, days1904 ) && days1900 == days1904 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Dates: the calendar, the fraction of a day and ISO 8601 text" )
    try
    {
        Int32 days = 0;
        ASSERT_TRUE( CXlsxDates::CivilToDays( 2000, 2, 29, days ) && 11016 == days );
        ASSERT_TRUE( CXlsxDates::CivilToDays( 2024, 2, 29, days ) );
        ASSERT_FALSE( CXlsxDates::CivilToDays( 1900, 2, 29, days ) );
        ASSERT_FALSE( CXlsxDates::CivilToDays( 2023, 2, 29, days ) );
        ASSERT_FALSE( CXlsxDates::CivilToDays( 2023, 13, 1, days ) );
        ASSERT_FALSE( CXlsxDates::CivilToDays( 2023, 4, 31, days ) );
        Int32 year = 0;
        UInt32 month = 0, day = 0;
        CXlsxDates::DaysToCivil( 11016, year, month, day );
        ASSERT_TRUE( 2000 == year && 2 == month && 29 == day );

        ASSERT_TRUE( 43200000000LL == CXlsxDates::FractionToMicros( "5", 1 ) );
        ASSERT_TRUE( 64800000000LL == CXlsxDates::FractionToMicros( "75", 2 ) );
        ASSERT_TRUE( 0 == CXlsxDates::FractionToMicros( "0", 1 ) );
        ASSERT_TRUE( 61200000000LL == CXlsxDates::FractionToMicros( "708333333336", 12 ) );      // 17:00, the double of Excel is 0.7083333333333334
        ASSERT_TRUE( CXlsxDates::MicrosPerDay == CXlsxDates::FractionToMicros( "999999999999999", 15 ) );    // rounds to the next midnight

        ASSERT_TRUE( IsoIs( "2023-01-01", 19358, 0, false ) );
        ASSERT_TRUE( IsoIs( "2023-01-01T10:20:30", 19358, 37230000000LL, true ) );
        ASSERT_TRUE( IsoIs( "2023-01-01T10:20:30.5Z", 19358, 37230500000LL, true ) );
        ASSERT_TRUE( IsoIs( "2023-01-01 10:20", 19358, 37200000000LL, true ) );
        ASSERT_TRUE( IsoFails( "2023-02-30" ) );
        ASSERT_TRUE( IsoFails( "2023-01-01T25:00:00" ) );
        ASSERT_TRUE( IsoFails( "2023-01-01T10:20:30+01:00" ) );
        ASSERT_TRUE( IsoFails( "23-01-01" ) );
        ASSERT_TRUE( IsoFails( "" ) );
        ASSERT_TRUE( IsoFails( "10000-01-01" ) );
        ASSERT_TRUE( "2023-01-01T10:20:30.500000" == CXlsxDates::ToIsoText( 19358, 37230500000LL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Number formats: built in ids and custom format codes that make a number a date" )
    try
    {
        ASSERT_TRUE( XLSX_NUMBERKIND_DATE == CXlsxDates::KindOfBuiltInFormat( 14 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATETIME == CXlsxDates::KindOfBuiltInFormat( 22 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_TIME == CXlsxDates::KindOfBuiltInFormat( 18 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_TIME == CXlsxDates::KindOfBuiltInFormat( 46 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfBuiltInFormat( 0 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfBuiltInFormat( 2 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfBuiltInFormat( 9 ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfBuiltInFormat( 49 ) );

        ASSERT_TRUE( XLSX_NUMBERKIND_DATE == CXlsxDates::KindOfFormatCode( "yyyy-mm-dd" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATE == CXlsxDates::KindOfFormatCode( "m/d/yyyy" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATE == CXlsxDates::KindOfFormatCode( "mmm-yy" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATE == CXlsxDates::KindOfFormatCode( "[$-409]d-mmm-yy;@" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATETIME == CXlsxDates::KindOfFormatCode( "yyyy\\-mm\\-dd\\ hh:mm:ss" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_DATETIME == CXlsxDates::KindOfFormatCode( "dd/mm/yyyy hh:mm" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_TIME == CXlsxDates::KindOfFormatCode( "hh:mm:ss" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_TIME == CXlsxDates::KindOfFormatCode( "[h]:mm:ss" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_TIME == CXlsxDates::KindOfFormatCode( "h:mm AM/PM" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "0.00" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "General" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "0.00E+00" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "[Red]#,##0.00" ) );
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "#,##0.00\" USD, day\"" ) );     // the d and y of text in quotes are no tokens
        ASSERT_TRUE( XLSX_NUMBERKIND_PLAIN == CXlsxDates::KindOfFormatCode( "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Numbers: the text of a number is split without a floating point number" )
    try
    {
        SXlsxNumberParts parts;
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "12.50", 5, parts ) );
        ASSERT_TRUE( !parts.negative && 12 == parts.whole && 2 == parts.fractionCount && '5' == parts.fraction[ 0 ] && !parts.fractionIsZero );
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "-0.25", 5, parts ) && parts.negative && 0 == parts.whole );
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "1E3", 3, parts ) && 1000 == parts.whole && parts.fractionIsZero );
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "1.5E-3", 6, parts ) && 0 == parts.whole && 4 == parts.fractionCount && 0 == memcmp( parts.fraction, "0015", 4 ) );
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "44927.708333333336", 18, parts ) && 44927 == parts.whole && 12 == parts.fractionCount );
        ASSERT_TRUE( CXlsxColumnConverter::SplitNumber( "9007199254740993", 16, parts ) && 9007199254740993ULL == parts.whole );     // 2^53 + 1: a double cannot hold it
        ASSERT_FALSE( CXlsxColumnConverter::SplitNumber( "abc", 3, parts ) );
        ASSERT_FALSE( CXlsxColumnConverter::SplitNumber( "", 0, parts ) );
        ASSERT_FALSE( CXlsxColumnConverter::SplitNumber( "1.2.3", 5, parts ) );
        ASSERT_FALSE( CXlsxColumnConverter::SplitNumber( "99999999999999999999999", 23, parts ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Text: the escapes _xHHHH_ of the strings of a workbook" )
    try
    {
        ASSERT_TRUE( CString( "a\rb" ) == Decoded( "a_x000D_b" ) );
        ASSERT_TRUE( CString( "_x0041_" ) == Decoded( "_x005F_x0041_" ) );      // an escaped underscore is not the start of an escape
        ASSERT_TRUE( CString( "\xC3\xA9" ) == Decoded( "_x00e9_" ) );
        ASSERT_TRUE( CString( "\xF0\x9F\x98\x80" ) == Decoded( "_xD83D__xDE00_" ) );     // a surrogate pair
        ASSERT_TRUE( CString( "_xZZZZ_" ) == Decoded( "_xZZZZ_" ) );
        ASSERT_TRUE( CString( "plain_text" ) == Decoded( "plain_text" ) );
        ASSERT_TRUE( CString( "" ) == Decoded( "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "References: cells and the targets of relationships" )
    try
    {
        UInt32 column = 0, row = 0;
        ASSERT_TRUE( CXlsxSheetParser::ParseCellReference( "A1", 2, column, row ) && 0 == column && 1 == row );
        ASSERT_TRUE( CXlsxSheetParser::ParseCellReference( "BC12", 4, column, row ) && 54 == column && 12 == row );
        ASSERT_TRUE( CXlsxSheetParser::ParseCellReference( "XFD1048576", 10, column, row ) && 16383 == column && 1048576 == row );
        ASSERT_FALSE( CXlsxSheetParser::ParseCellReference( "1A", 2, column, row ) );
        ASSERT_FALSE( CXlsxSheetParser::ParseCellReference( "A", 1, column, row ) );
        ASSERT_FALSE( CXlsxSheetParser::ParseCellReference( "A0", 2, column, row ) );
        ASSERT_TRUE( "A1" == CXlsxSheetParser::FormatCellReference( 0, 1 ) );
        ASSERT_TRUE( "BC12" == CXlsxSheetParser::FormatCellReference( 54, 12 ) );
        ASSERT_TRUE( "Z3" == CXlsxSheetParser::FormatCellReference( 25, 3 ) );
        ASSERT_TRUE( "AA1" == CXlsxSheetParser::FormatCellReference( 26, 1 ) );
        ASSERT_TRUE( "XFD5" == CXlsxSheetParser::FormatCellReference( 16383, 5 ) );

        ASSERT_TRUE( Target( "xl/workbook.xml", "worksheets/sheet1.xml", "xl/worksheets/sheet1.xml" ) );
        ASSERT_TRUE( Target( "xl/workbook.xml", "/xl/sharedStrings.xml", "xl/sharedStrings.xml" ) );
        ASSERT_TRUE( Target( "xl/worksheets/sheet1.xml", "../styles.xml", "xl/styles.xml" ) );
        ASSERT_TRUE( Target( "", "xl/workbook.xml", "xl/workbook.xml" ) );
        ASSERT_TRUE( Target( "xl/workbook.xml", "./a/./b.xml", "xl/a/b.xml" ) );
        ASSERT_TRUE( TargetFails( "xl/workbook.xml", "../../outside.xml" ) );
        ASSERT_TRUE( TargetFails( "xl/workbook.xml", "../.." ) );
        ASSERT_TRUE( TargetFails( "", "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limited access: what is really read is held against the budget, whatever the archive states" )
    try
    {
        CDynamicBuffer data;
        for ( UInt32 i = 0; i < 100; ++i )
            data.Append( "x", 1 );
        char buffer[ 128 ];

        {
            UInt64 budget = 60;
            CXlsxLimitedAccess access( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( data ) ), &budget );
            ASSERT_TRUE( 40 == access.Read( buffer, 1, 40 ) && 20 == budget && !access.IsExceeded() );
            ASSERT_TRUE( 20 == access.Read( buffer, 1, 40 ) );          // cut at the budget
            ASSERT_TRUE( access.IsExceeded() && 0 == budget );          // there was more behind it
            ASSERT_TRUE( 0 == access.Read( buffer, 1, 10 ) && access.Eof() );
            ASSERT_TRUE( 60 == access.GetBytesRead() );
        }
        {
            UInt64 budget = 100;
            CXlsxLimitedAccess access( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( data ) ), &budget );
            ASSERT_TRUE( 100 == access.Read( buffer, 1, 100 ) );
            ASSERT_TRUE( 0 == access.Read( buffer, 1, 1 ) );
            ASSERT_FALSE( access.IsExceeded() );                        // exactly the budget is no excess
        }
        {
            UInt64 budget = 1000;
            CXlsxLimitedAccess access( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( data ) ), &budget );
            ASSERT_TRUE( 10 == access.Read( buffer, 1, 10 ) );
            access.Abort();
            ASSERT_TRUE( 0 == access.Read( buffer, 1, 10 ) && access.Eof() && !access.IsExceeded() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "XML reader: what the data store codec reports of a document: attributes, text at the end of its element, entities, prefixes" )
    try
    {
        CXlsxBuilder builder;
        builder.AddSheet( "S", Sheet( "" ) );
        builder.SetPart( "xl/test.xml", "<?xml version=\"1.0\"?><a x=\"1\" y:z=\"two\" xmlns:y=\"urn:y\"><y:b> padded   text </y:b><c>&lt;&amp;&gt;</c><d/><e>1</e></a>" );
        CDynamicBuffer file;
        ASSERT_TRUE( builder.Build( file ) );
        SXlsxReadOptions options;
        CXlsxPackage package( options );
        CDynamicBufferAccess source( file );
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == package.Open( source, error ) );
        CXmlRecorder recorder;
        ASSERT_TRUE( DATATABLE_STATUS_OK == package.ParseXmlPart( "xl/test.xml", recorder, error ) );
        ASSERT_TRUE( recorder.events == "<a>[x=1][y:z=two]<y:b>{padded text}</y:b><c>{<&>}</c><d></d><e>{1}</e></a>" );

        // what the parsers of the package make of the names and values of the reader
        CXlsxWorkbook workbook( options );
        ASSERT_TRUE( DATATABLE_STATUS_OK == workbook.Load( package, error ) );
        ASSERT_TRUE( 1 == workbook.GetSheets().size() );
        ASSERT_TRUE( "S" == workbook.GetSheets()[ 0 ].name && "xl/worksheets/sheet1.xml" == workbook.GetSheets()[ 0 ].partName && workbook.GetSheets()[ 0 ].isWorksheet );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Package: the temporary file and the mount exist while the package is open and are gone when it is destroyed" )
    try
    {
        ASSERT_TRUE( ArePluginsAvailable() );
        CXlsxBuilder builder;
        builder.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "x" ) ) ) );
        CDynamicBuffer file;
        ASSERT_TRUE( builder.Build( file ) );

        CString path;
        CString mountName;
        {
            SXlsxReadOptions options;
            CXlsxPackage package( options );
            CDynamicBufferAccess source( file );
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == package.Open( source, error ) );
            path = package.GetFilePath();
            mountName = package.GetMountName();
            ASSERT_FALSE( path.IsNULLOrEmpty() );
            ASSERT_TRUE( FileExists( path ) );
            ASSERT_TRUE( 1 == CXlsxPackage::GetOpenPackageCount() );
            ASSERT_TRUE( package.HasPart( "xl/workbook.xml" ) && package.HasPart( "[Content_Types].xml" ) && !package.HasPart( "xl/nothing.xml" ) );
            ASSERT_TRUE( file.GetDataSize() == package.GetFileBytes() );
        }
        ASSERT_FALSE( FileExists( path ) );
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
        ASSERT_FALSE( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( "/" + mountName + "/xl/workbook.xml" ) );

        // a package that failed to open leaves nothing either
        CDynamicBuffer garbage;
        garbage.Append( "not a zip file at all", 21 );
        {
            SXlsxReadOptions options;
            CXlsxPackage package( options );
            CDynamicBufferAccess source( garbage );
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == package.Open( source, error ) );
            ASSERT_TRUE( package.GetFilePath().IsNULLOrEmpty() );
        }
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
