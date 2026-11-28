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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#define TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_ROUNDINGMODE_H
#include "gucefCORE_RoundingMode.h"
#define GUCEF_CORE_ROUNDINGMODE_H
#endif /* GUCEF_CORE_ROUNDINGMODE_H ? */

#ifndef TABLECODECPLUGIN_XLSX_MACROS_H
#include "tablecodecpluginXLSX_macros.h"
#define TABLECODECPLUGIN_XLSX_MACROS_H
#endif /* TABLECODECPLUGIN_XLSX_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The read options of the xlsx table codec taken from a CValueList. The keys that hold lists (nullTokens, schema)
 *  take one value per entry and need a value list that allows multiple values, see AddOption().
 *
 *  Sheet:         sheetName (the sheet with that name) or sheetIndex (0 based, in workbook order, default 0)
 *  Layout:        hasHeader (true), headerRow (1 based sheet row of the header, 0: the first row with a value), firstRow (1 based,
 *                 the rows above it are skipped, default 1), firstColumn (1 based or letters: "C", default 1), lastColumn (0:
 *                 the last cell of the header row), allowExtraCells (false: a value right of the table is an error)
 *  Types:         schema (entries "name|type", a trailing ! on the type marks a column that is not nullable: no inference),
 *                 columnTypes.<column name> (a type for one column, the others are inferred), inferRowCount (0: all rows),
 *                 onValueError ("fail" or "null", default fail), decimalRounding (none, halfEven, halfUp, halfDown, towardZero,
 *                 awayFromZero, floor, ceiling: how a number with more digits than its decimal column is rounded, default none: an error)
 *  Text:          nullTokens (text cells that read as null)
 *  Limits:        maxFileBytes, maxEntries, maxUncompressedBytes, maxExpansionRatio, maxSheets, maxRows, maxColumns, maxCells,
 *                 maxCellBytes, maxSharedStringBytes
 */
struct TABLECODECPLUGIN_XLSX_PUBLIC_CPP SXlsxReadOptions
{
    typedef GUCEF::vector< CORE::CDataTableColumnDef > TColumnDefVector;

    CORE::CString sheetName;
    bool hasSheetName;
    UInt32 sheetIndex;
    bool hasHeader;
    UInt32 headerRow;
    UInt32 firstRow;
    UInt32 firstColumn;                 /**< 1 based */
    UInt32 lastColumn;                  /**< 1 based, 0: the last cell of the header row */
    bool allowExtraCells;
    TColumnDefVector schemaColumns;     /**< a full schema: no inference */
    TColumnDefVector columnOverrides;   /**< by name */
    UInt64 inferRowCount;               /**< 0: all rows */
    bool failOnValueError;
    bool roundDecimals;
    CORE::TRoundingMode decimalRounding;
    CORE::CString::StringVector nullTokens;

    UInt64 maxFileBytes;                /**< the size of the .xlsx file */
    UInt32 maxEntries;                  /**< entries in the zip archive */
    UInt64 maxUncompressedBytes;        /**< the sizes of all entries together, and the bytes that are actually read */
    UInt32 maxExpansionRatio;           /**< uncompressed over compressed size, 0: no limit; applies above 4 MiB uncompressed */
    UInt32 maxSheets;
    UInt32 maxRows;                     /**< highest row number of the sheet */
    UInt32 maxColumns;                  /**< highest column number of the sheet */
    UInt64 maxCells;                    /**< cells with a value in the sheet */
    UInt32 maxCellBytes;                /**< the text of one cell */
    UInt64 maxSharedStringBytes;        /**< the text of all shared strings together */

    SXlsxReadOptions( void );

    /** False, with the reason, for an option that is not valid: INVALID_ARGUMENT to the caller */
    bool Parse( const CORE::CValueList& options, CORE::CString& error );

    /** True when the text cell is one of the null tokens */
    bool IsNullToken( const char* text, UInt32 length ) const;

    /** Adds one entry of a list option, switching the list to multiple values */
    static void AddOption( CORE::CValueList& options, const CORE::CString& key, const CORE::CString& value );

    /** Puts the schema in the options as the "schema" entries, one "name|type" entry per column, ! marks not nullable */
    static void SetSchemaOption( CORE::CValueList& options, const CORE::CDataTableSchema& schema );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H ? */
