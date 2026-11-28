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

#ifndef TABLECODECPLUGIN_XLSX_TESTAPP_TESTXLSXSUPPORT_H
#define TABLECODECPLUGIN_XLSX_TESTAPP_TESTXLSXSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace XLSXTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< GUCEF::CORE::CDataTableBatchPtr > TBatchVector;

/**
 *  Builds the .xlsx files of the tests from XML text: a zip package with the parts of a minimal workbook, written with the
 *  zip archive writer of the VFS plugin vfspluginZIP. Nothing here is a file of a real source: every fixture is made in the test.
 */
class CXlsxBuilder
{
    public:

    CXlsxBuilder( void );

    /** A worksheet with the given sheet XML (see Sheet()) */
    CXlsxBuilder& AddSheet( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& sheetXml );

    /** A sheet that is a chart sheet: it holds no cells */
    CXlsxBuilder& AddChartSheet( const GUCEF::CORE::CString& name );

    /** xl/sharedStrings.xml with one plain string per entry */
    CXlsxBuilder& SetSharedStrings( const GUCEF::CORE::CString::StringVector& strings );

    /** xl/sharedStrings.xml as given */
    CXlsxBuilder& SetSharedStringsXml( const GUCEF::CORE::CString& xml );

    /** xl/styles.xml as given */
    CXlsxBuilder& SetStyles( const GUCEF::CORE::CString& xml );

    CXlsxBuilder& SetDate1904( bool date1904 );

    /** Adds a part, or replaces the one of the builder with the name ("xl/workbook.xml", "[Content_Types].xml", ...) */
    CXlsxBuilder& SetPart( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& content );

    /** Leaves a part of the builder out */
    CXlsxBuilder& RemovePart( const GUCEF::CORE::CString& name );

    /** A part of the zip with bytes that are not text: the content as is */
    CXlsxBuilder& SetBinaryPart( const GUCEF::CORE::CString& name, const GUCEF::CORE::CDynamicBuffer& content );

    /** Writes the package into the buffer */
    bool Build( GUCEF::CORE::CDynamicBuffer& xlsx ) const;

    private:

    struct SSheet
    {
        GUCEF::CORE::CString name;
        GUCEF::CORE::CString xml;
        bool isChart;
    };

    struct SPart
    {
        GUCEF::CORE::CString name;
        GUCEF::CORE::CDynamicBuffer content;
    };

    GUCEF::vector< SSheet > m_sheets;
    GUCEF::vector< SPart > m_parts;
    GUCEF::vector< GUCEF::CORE::CString > m_removed;
    GUCEF::CORE::CString m_sharedStringsXml;
    GUCEF::CORE::CString m_stylesXml;
    bool m_date1904;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads vfspluginZIP and dstorepluginPARSIFALXML once, from the directory of the module */
void LoadRuntimePlugins( void );

/** Whether the zip archive type of the VFS and the XML codec are there */
bool ArePluginsAvailable( void );

/** One xlsx codec object for the tests */
GUCEF::CORE::CITableCodec& GetXlsxCodec( void );

/** XML text of the data of a cell, with the special characters escaped */
GUCEF::CORE::CString Xml( const GUCEF::CORE::CString& text );

/** One cell: a style is optional (-1: none) */
GUCEF::CORE::CString CellShared( const char* ref, GUCEF::UInt32 index, int style = -1 );
GUCEF::CORE::CString CellNumber( const char* ref, const char* text, int style = -1 );
GUCEF::CORE::CString CellFormulaString( const char* ref, const char* formula, const char* cached );
GUCEF::CORE::CString CellFormulaNumber( const char* ref, const char* formula, const char* cached, int style = -1 );
GUCEF::CORE::CString CellFormulaOnly( const char* ref, const char* formula );
GUCEF::CORE::CString CellBool( const char* ref, bool value );
GUCEF::CORE::CString CellInline( const char* ref, const char* text );
GUCEF::CORE::CString CellIsoDate( const char* ref, const char* text );
GUCEF::CORE::CString CellError( const char* ref, const char* text );

/** A row with its cells (the XML of the cells) */
GUCEF::CORE::CString Row( GUCEF::UInt32 number, const GUCEF::CORE::CString& cells );

/** A worksheet with the rows and, behind sheetData, more XML (merge cells, ...) */
GUCEF::CORE::CString Sheet( const GUCEF::CORE::CString& rows, const GUCEF::CORE::CString& behindData = GUCEF::CORE::CString() );

/** A styles part: style 1 is a date, 2 a date with a time of day, 3 a plain number with a currency text, 4 a time of day, 5 a plain number format (0.00) */
GUCEF::CORE::CString DateStyles( void );

/**
 *  Reads the workbook as a table: the metadata, then every batch. The status of the first thing that fails, OK when
 *  all was read. error holds the message; the schema and the batches are the ones read.
 */
GUCEF::CORE::TDataTableStatus ReadXlsx( const GUCEF::CORE::CDynamicBuffer& xlsx, const GUCEF::CORE::CValueList& options, GUCEF::CORE::CString& error,
                                        GUCEF::CORE::CDataTableSchemaPtr* schema = GUCEF_NULL, TBatchVector* batches = GUCEF_NULL,
                                        GUCEF::UInt32 batchRowCount = 0, GUCEF::CORE::CITableFileMetaDataPtr* metaData = GUCEF_NULL );

/** The schema as text: name:type separated by | */
GUCEF::CORE::CString DescribeSchema( const GUCEF::CORE::CDataTableSchema& schema );

/** The rows as text, one line per row, cells separated by |, ~ for null: values are read from the raw column data (bool true/false, decimals as text, dates as yyyy-mm-dd) */
GUCEF::CORE::CString DumpRows( const GUCEF::CORE::CDataTableSchema& schema, const TBatchVector& batches );

/** Total rows of batches */
GUCEF::UInt64 CountRows( const TBatchVector& batches );

/** Options: one entry of a list option */
void AddOption( GUCEF::CORE::CValueList& options, const char* key, const char* value );

/** A worksheet of rows of two columns, the header and row count rows: for the tests of the batches and the limits */
GUCEF::CORE::CString BigSheet( GUCEF::UInt32 rows );

/** Whether a message holds the text */
bool Contains( const GUCEF::CORE::CString& text, const char* part );

}; /* namespace XLSXTEST */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_TESTAPP_TESTXLSXSUPPORT_H ? */
