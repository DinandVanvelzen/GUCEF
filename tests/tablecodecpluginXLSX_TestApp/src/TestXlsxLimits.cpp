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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#include "tablecodecpluginXLSX_CXlsxPackage.h"
#define TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H ? */

#include "TestXlsxSupport.h"
#include "TestXlsxLimits.h"

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

void
SetOption( CValueList& options, const char* key, const char* value )
{
    options.Set( CString( key ), CVariant( CString( value ) ) );
}

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

/** A part of megabytes of zeros: a few kilobytes in the archive */
CDynamicBuffer
Zeros( UInt32 megabytes )
{
    CDynamicBuffer zeros;
    static const char block[ 65536 ] = { 0 };
    for ( UInt32 i = 0; i < megabytes * 16; ++i )
        zeros.Append( block, sizeof( block ) );
    return zeros;
}

CXlsxBuilder
SimpleBuilder( void )
{
    CXlsxBuilder builder;
    builder.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "A2", "1" ) ) ) );
    return builder;
}

}

/*-------------------------------------------------------------------------*/

void
PerformXlsxLimitsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "tablecodecpluginXLSX.Limits" );

    GUCEF_TESTFW_TESTCASE( "Limits: a sheet of 20000 rows is read, and refused when the rows or the cells pass the limits" )
    try
    {
        CXlsxBuilder builder;
        builder.AddSheet( "Big", BigSheet( 20000 ) );
        CDynamicBuffer file = MakeFile( builder );

        CString error;
        TBatchVector batches;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadXlsx( file, CValueList(), error, GUCEF_NULL, &batches ) );
        ASSERT_TRUE( 20000 == CountRows( batches ) );

        // 20001 rows in the sheet, 40002 cells with a value
        CValueList rows;
        SetOption( rows, "maxRows", "10000" );
        ASSERT_TRUE( CheckFailure( "maxRows", file, rows, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxRows" ) );
        CValueList cells;
        SetOption( cells, "maxCells", "1000" );
        ASSERT_TRUE( CheckFailure( "maxCells", file, cells, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxCells" ) );

        // at the limit it is read
        CValueList exact;
        SetOption( exact, "maxRows", "20001" );
        SetOption( exact, "maxCells", "40002" );
        TBatchVector exactBatches;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadXlsx( file, exact, error, GUCEF_NULL, &exactBatches ) );
        ASSERT_TRUE( 20000 == CountRows( exactBatches ) );
        CValueList oneRowShort;
        SetOption( oneRowShort, "maxRows", "20000" );
        ASSERT_TRUE( CheckFailure( "maxRows one short", file, oneRowShort, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxRows" ) );
        CValueList oneCellShort;
        SetOption( oneCellShort, "maxCells", "40001" );
        ASSERT_TRUE( CheckFailure( "maxCells one short", file, oneCellShort, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxCells" ) );

        // the limit also holds for the metadata alone
        CITableFileMetaDataPtr metaData;
        CDynamicBufferAccess source( file );
        ASSERT_TRUE( DATATABLE_STATUS_LIMIT_EXCEEDED == GetXlsxCodec().ReadMetaData( source, rows, metaData, error ) );
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: columns, the text of a cell, shared strings, sheets, entries and the size of the file" )
    try
    {
        CXlsxBuilder wide;
        wide.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) + CellInline( "E1", "e" ) ) ) );
        CValueList columns;
        SetOption( columns, "maxColumns", "3" );
        ASSERT_TRUE( CheckFailure( "maxColumns", MakeFile( wide ), columns, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxColumns" ) );

        CXlsxBuilder longText;
        longText.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellInline( "A2", "this text is longer than ten bytes" ) ) ) );
        CValueList bytes;
        SetOption( bytes, "maxCellBytes", "10" );
        ASSERT_TRUE( CheckFailure( "maxCellBytes", MakeFile( longText ), bytes, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxCellBytes" ) );

        CXlsxBuilder shared;
        shared.SetSharedStrings( CString::StringVector( 1, "this shared string is longer than ten bytes" ) );
        shared.AddSheet( "S", Sheet( Row( 1, CellShared( "A1", 0 ) ) ) );
        CDynamicBuffer sharedFile = MakeFile( shared );
        ASSERT_TRUE( CheckFailure( "maxCellBytes of a shared string", sharedFile, bytes, DATATABLE_STATUS_LIMIT_EXCEEDED, "shared string" ) );
        CValueList sharedBytes;
        SetOption( sharedBytes, "maxSharedStringBytes", "10" );
        ASSERT_TRUE( CheckFailure( "maxSharedStringBytes", sharedFile, sharedBytes, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxSharedStringBytes" ) );

        CXlsxBuilder sheets;
        sheets.AddSheet( "A", Sheet( "" ) ).AddSheet( "B", Sheet( "" ) ).AddSheet( "C", Sheet( "" ) );
        CValueList sheetLimit;
        SetOption( sheetLimit, "maxSheets", "2" );
        ASSERT_TRUE( CheckFailure( "maxSheets", MakeFile( sheets ), sheetLimit, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxSheets" ) );

        CXlsxBuilder simple = SimpleBuilder();
        CDynamicBuffer simpleFile = MakeFile( simple );
        CValueList entries;
        SetOption( entries, "maxEntries", "3" );
        ASSERT_TRUE( CheckFailure( "maxEntries", simpleFile, entries, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxEntries" ) );
        CValueList fileBytes;
        SetOption( fileBytes, "maxFileBytes", "200" );
        ASSERT_TRUE( CheckFailure( "maxFileBytes", simpleFile, fileBytes, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxFileBytes" ) );
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Limits: a zip bomb is refused by what it states, and by how much it expands, before any of it is read" )
    try
    {
        // a part of 8 MiB of zeros: a few kilobytes in the archive
        CXlsxBuilder bomb = SimpleBuilder();
        bomb.SetBinaryPart( "xl/media/bomb.bin", Zeros( 8 ) );
        CDynamicBuffer bombFile = MakeFile( bomb );
        ASSERT_TRUE( bombFile.GetDataSize() > 0 && bombFile.GetDataSize() < 100 * 1024 );

        CValueList total;
        SetOption( total, "maxUncompressedBytes", "1048576" );
        ASSERT_TRUE( CheckFailure( "states too much", bombFile, total, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxUncompressedBytes" ) );
        ASSERT_TRUE( CheckFailure( "states too much, zip bomb", bombFile, total, DATATABLE_STATUS_LIMIT_EXCEEDED, "zip bomb" ) );

        CValueList ratio;
        SetOption( ratio, "maxExpansionRatio", "100" );
        ASSERT_TRUE( CheckFailure( "expansion", bombFile, ratio, DATATABLE_STATUS_LIMIT_EXCEEDED, "maxExpansionRatio" ) );

        // the same file without the ratio limit is read: the limits are what refuses it
        CValueList noRatio;
        SetOption( noRatio, "maxExpansionRatio", "0" );
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadXlsx( bombFile, noRatio, error ) );

        // a sheet that is a bomb itself: 6 MiB of comment in the XML of the sheet
        CDynamicBuffer sheetXml;
        const CString open = CString( "<?xml version=\"1.0\"?><worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>" ) +
                             Row( 1, CellInline( "A1", "a" ) ) + "</sheetData><!--";
        sheetXml.Append( open.C_String(), open.ByteSize() - 1 );
        const CDynamicBuffer filler = Zeros( 6 );
        sheetXml.Append( filler.GetConstBufferPtr(), filler.GetDataSize() );
        sheetXml.Append( "--></worksheet>", 15 );
        CXlsxBuilder sheetBomb = SimpleBuilder();
        sheetBomb.SetBinaryPart( "xl/worksheets/sheet1.xml", sheetXml );
        CDynamicBuffer sheetBombFile = MakeFile( sheetBomb );
        CValueList small;
        SetOption( small, "maxUncompressedBytes", "2097152" );
        ASSERT_TRUE( CheckFailure( "sheet bomb", sheetBombFile, small, DATATABLE_STATUS_LIMIT_EXCEEDED, "zip bomb" ) );
        ASSERT_TRUE( CheckFailure( "sheet bomb states", sheetBombFile, small, DATATABLE_STATUS_LIMIT_EXCEEDED, "bytes of content" ) );
        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Malformed: files that are no workbook, or no zip archive, are refused with the reason" )
    try
    {
        CDynamicBuffer text;
        text.Append( "this is a text file and no zip archive", 38 );
        ASSERT_TRUE( CheckFailure( "text", text, CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "no zip archive" ) );

        CDynamicBuffer empty;
        ASSERT_TRUE( CheckFailure( "empty", empty, CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "empty" ) );

        CDynamicBuffer fake;
        fake.Append( "PK\x03\x04", 4 );
        for ( UInt32 i = 0; i < 200; ++i )
            fake.Append( "\x7F", 1 );
        ASSERT_TRUE( CheckFailure( "fake zip", fake, CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not a readable zip archive" ) );

        CXlsxBuilder simple = SimpleBuilder();
        CDynamicBuffer valid = MakeFile( simple );
        CDynamicBuffer truncated;
        truncated.Append( valid.GetConstBufferPtr(), valid.GetDataSize() - 40 );
        ASSERT_TRUE( CheckFailure( "truncated", truncated, CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "zip" ) );

        CXlsxBuilder noTypes = SimpleBuilder();
        noTypes.RemovePart( "[Content_Types].xml" );
        ASSERT_TRUE( CheckFailure( "no content types", MakeFile( noTypes ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "[Content_Types].xml" ) );

        CXlsxBuilder noWorkbook = SimpleBuilder();
        noWorkbook.RemovePart( "xl/workbook.xml" );
        ASSERT_TRUE( CheckFailure( "no workbook", MakeFile( noWorkbook ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "no workbook part" ) );

        CXlsxBuilder noSheet = SimpleBuilder();
        noSheet.RemovePart( "xl/worksheets/sheet1.xml" );
        ASSERT_TRUE( CheckFailure( "no sheet part", MakeFile( noSheet ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "has no part xl/worksheets/sheet1.xml" ) );

        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Malformed: XML that is not well formed in any part is refused with the reason" )
    try
    {
        CXlsxBuilder unclosed = SimpleBuilder();
        unclosed.SetPart( "xl/worksheets/sheet1.xml", "<?xml version=\"1.0\"?><worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData><row r=\"1\">" );
        ASSERT_TRUE( CheckFailure( "unclosed sheet", MakeFile( unclosed ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not well formed XML" ) );

        const CString sheet = Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "A2", "1" ) ) );
        CXlsxBuilder cut = SimpleBuilder();
        cut.SetPart( "xl/worksheets/sheet1.xml", CString( sheet.C_String(), sheet.ByteSize() / 2 ) );
        ASSERT_TRUE( CheckFailure( "sheet cut in two", MakeFile( cut ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not well formed XML" ) );

        CXlsxBuilder mismatch = SimpleBuilder();
        mismatch.SetPart( "xl/worksheets/sheet1.xml", "<worksheet><sheetData><row r=\"1\"></sheetData></row></worksheet>" );
        ASSERT_TRUE( CheckFailure( "tags that cross", MakeFile( mismatch ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not well formed XML" ) );

        CXlsxBuilder binary = SimpleBuilder();
        CDynamicBuffer noise;
        for ( UInt32 i = 0; i < 300; ++i )
            noise.Append( i % 2 ? "\x01\xFE" : "<<", 2 );
        binary.SetBinaryPart( "xl/worksheets/sheet1.xml", noise );
        ASSERT_TRUE( CheckFailure( "noise as a sheet", MakeFile( binary ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not well formed XML" ) );

        CXlsxBuilder workbook = SimpleBuilder();
        workbook.SetPart( "xl/workbook.xml", "<workbook><sheets><sheet name=\"S\"" );
        ASSERT_TRUE( CheckFailure( "workbook", MakeFile( workbook ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "xl/workbook.xml" ) );

        CXlsxBuilder strings = SimpleBuilder();
        strings.SetSharedStringsXml( "<sst><si><t>unclosed</si></sst>" );
        ASSERT_TRUE( CheckFailure( "shared strings", MakeFile( strings ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "xl/sharedStrings.xml" ) );

        CXlsxBuilder styles = SimpleBuilder();
        styles.SetStyles( "<styleSheet><cellXfs><xf numFmtId=\"14\"></cellXfs>" );
        ASSERT_TRUE( CheckFailure( "styles", MakeFile( styles ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "xl/styles.xml" ) );

        CXlsxBuilder badFormat = SimpleBuilder();
        badFormat.SetStyles( "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><cellXfs><xf numFmtId=\"abc\"/></cellXfs></styleSheet>" );
        ASSERT_TRUE( CheckFailure( "format id", MakeFile( badFormat ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "number format id" ) );

        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Malformed: cells and rows that contradict the format, and relationships that leave the package" )
    try
    {
        CXlsxBuilder index;
        index.SetSharedStrings( CString::StringVector( 1, "only one" ) );
        index.AddSheet( "S", Sheet( Row( 1, CellShared( "A1", 0 ) ) + Row( 2, CellShared( "A2", 99 ) ) ) );
        ASSERT_TRUE( CheckFailure( "shared string index", MakeFile( index ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "shared string 99" ) );

        CXlsxBuilder type;
        type.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, "<c r=\"A2\" t=\"zz\"><v>1</v></c>" ) ) );
        ASSERT_TRUE( CheckFailure( "cell type", MakeFile( type ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "unknown cell type \"zz\"" ) );

        CXlsxBuilder boolean;
        boolean.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, "<c r=\"A2\" t=\"b\"><v>7</v></c>" ) ) );
        ASSERT_TRUE( CheckFailure( "boolean", MakeFile( boolean ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "instead of 0 or 1" ) );

        CXlsxBuilder rows;
        rows.AddSheet( "S", Sheet( Row( 3, CellInline( "A3", "a" ) ) + Row( 2, CellNumber( "A2", "1" ) ) ) );
        ASSERT_TRUE( CheckFailure( "rows out of order", MakeFile( rows ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not in order" ) );

        CXlsxBuilder cells;
        cells.AddSheet( "S", Sheet( Row( 1, CellInline( "B1", "b" ) + CellInline( "A1", "a" ) ) ) );
        ASSERT_TRUE( CheckFailure( "cells out of order", MakeFile( cells ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not in order" ) );

        CXlsxBuilder twice;
        twice.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) + CellInline( "A1", "again" ) ) ) );
        ASSERT_TRUE( CheckFailure( "a cell twice", MakeFile( twice ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not in order" ) );

        CXlsxBuilder wrongRow;
        wrongRow.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "A3", "1" ) ) ) );
        ASSERT_TRUE( CheckFailure( "cell of another row", MakeFile( wrongRow ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "is in row 2" ) );

        CXlsxBuilder reference;
        reference.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "7", "1" ) ) ) );
        ASSERT_TRUE( CheckFailure( "cell reference", MakeFile( reference ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "not one" ) );

        CXlsxBuilder number;
        number.AddSheet( "S", Sheet( Row( 1, CellInline( "A1", "a" ) ) + Row( 2, CellNumber( "A2", "abc" ) ) ) );
        ASSERT_TRUE( CheckFailure( "text in a number cell", MakeFile( number ), CValueList(), DATATABLE_STATUS_TYPE_MISMATCH, "is no number" ) );

        // relationships: a target that leaves the package is refused, the same for a sheet without one
        CXlsxBuilder escape = SimpleBuilder();
        escape.SetPart( "xl/_rels/workbook.xml.rels", "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"../../../outside.xml\"/></Relationships>" );
        ASSERT_TRUE( CheckFailure( "target outside the package", MakeFile( escape ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "outside of the package" ) );

        CXlsxBuilder external = SimpleBuilder();
        external.SetPart( "xl/_rels/workbook.xml.rels", "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                          "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"https://example.org/x.xml\" TargetMode=\"External\"/></Relationships>" );
        ASSERT_TRUE( CheckFailure( "external target", MakeFile( external ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "no relationship to a part" ) );

        CXlsxBuilder norel = SimpleBuilder();
        norel.SetPart( "xl/_rels/workbook.xml.rels", "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"></Relationships>" );
        ASSERT_TRUE( CheckFailure( "sheet without a relationship", MakeFile( norel ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "no relationship to a part" ) );

        CXlsxBuilder rootEscape = SimpleBuilder();
        rootEscape.SetPart( "_rels/.rels", "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                            "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"../evil.xml\"/></Relationships>" );
        ASSERT_TRUE( CheckFailure( "workbook outside the package", MakeFile( rootEscape ), CValueList(), DATATABLE_STATUS_CORRUPT_DATA, "outside of the package" ) );

        // a worksheet without a workbook date system, strings or styles is fine
        CXlsxBuilder minimal = SimpleBuilder();
        CString error;
        ASSERT_TRUE( DATATABLE_STATUS_OK == ReadXlsx( MakeFile( minimal ), CValueList(), error ) );

        ASSERT_TRUE( 0 == CXlsxPackage::GetOpenPackageCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
