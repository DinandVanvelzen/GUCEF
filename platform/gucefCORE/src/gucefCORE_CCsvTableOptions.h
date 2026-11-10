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

#ifndef GUCEF_CORE_CCSVTABLEOPTIONS_H
#define GUCEF_CORE_CCSVTABLEOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CCSVTABLETEXT_H
#include "gucefCORE_CCsvTableText.h"
#define GUCEF_CORE_CCSVTABLETEXT_H
#endif /* GUCEF_CORE_CCSVTABLETEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The type of a column as the options give it: the column and the date format a date column is read with */
struct SCsvColumnSpec
{
    CDataTableColumnDef column;
    CCsvTableText::SDateFormat dateFormat;
    bool hasDateFormat;

    SCsvColumnSpec( void );
};

/**
 *  The read options of the CSV table codec (section 11.1 of the data tables design) taken from a CValueList.
 *  Keys that hold lists (nullTokens, skipLinesMatching, currencySymbols, dateFormats, schema) take one value per entry
 *  and need a value list that allows multiple values.
 */
struct SCsvReadOptions
{
    typedef GUCEF::vector< SCsvColumnSpec > TColumnSpecVector;
    typedef GUCEF::vector< CGlobPattern > TGlobPatternVector;
    typedef GUCEF::vector< CCsvTableText::SDateFormat > TDateFormatVector;

    char delimiter;
    char quote;                                 /**< '\0' for none: every byte is literal */
    bool hasHeader;
    UInt32 skipLeadingLines;
    TGlobPatternVector skipLinesMatching;
    CString::StringVector nullTokens;
    bool trimWhitespace;
    CCsvTableText::SNumberFormat numberFormat;
    TDateFormatVector dateFormats;
    TColumnSpecVector schemaColumns;            /**< a full schema: no inference */
    TColumnSpecVector columnOverrides;          /**< by name; the names are the column names of the file or of the schema */
    UInt64 inferRowCount;                       /**< 0: all rows */
    bool failOnValueError;                      /**< false: a value that does not fit its column becomes null */
    bool allowMissingFields;
    bool allowExtraFields;
    UInt32 maxFieldBytes;
    UInt32 maxRecordBytes;
    UInt32 maxColumns;
    UInt32 batchByteLimit;                      /**< a batch is cut short when its text values reach this size */

    SCsvReadOptions( void );

    /** False, with the reason, for an option that is not valid: INVALID_ARGUMENT to the caller */
    bool Parse( const CValueList& options, CString& error );

    /** True when the default null token list is in use: the empty string and nothing else */
    bool HasOnlyEmptyNullToken( void ) const;

    /** True when the unquoted text is a null token */
    bool IsNullToken( const char* text, UInt32 length ) const;
};

/**
 *  The write options of the CSV table codec (11.2) taken from the codec options of CDataTableWriterOptions. When
 *  lineEnd, bom or delimiter are not given they are taken from the key value metadata of the options under
 *  lineEnding, bom and delimiter, which is what the reader of a file reports: a file read and written again keeps its form.
 */
struct SCsvWriteOptions
{
    char delimiter;
    char quote;
    CString lineEnd;                            /**< "\r\n", "\n" or "\r" */
    bool header;
    bool bom;
    CString nullToken;

    SCsvWriteOptions( void );

    bool Parse( const CDataTableWriterOptions& options, CString& error );
};

/** Appends an entry of a list option, which needs a value list that allows multiple values: that is switched on */
GUCEF_HIDDEN void AddCsvOptionValue( CValueList& options, const CString& key, const CString& value );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVTABLEOPTIONS_H ? */
