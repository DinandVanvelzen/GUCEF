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

#ifndef GUCEF_CORE_CCSVTABLECODEC_H
#define GUCEF_CORE_CCSVTABLECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

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

/**
 *  The CSV table codec, type "csv", file extensions csv and tsv. Built in: CCoreGlobal registers it at start up.
 *  Section 11 of the data tables design has the rules; the short form:
 *
 *  Reading (the options are the CValueList given to ReadMetaData() and CreateReader(); lists take one value per entry and
 *  need a value list that allows multiple values, see SetSchemaOption()):
 *    delimiter (","), quote ("\""), hasHeader (true), skipLeadingLines (0), skipLinesMatching (glob patterns, matched
 *    against the fields of a record joined with the delimiter), nullTokens (""), trimWhitespace (true),
 *    thousandsSeparator, decimalSeparator ("."), currencySymbols, negativeParentheses (false), dateFormats
 *    ("YYYY-MM-DD"; YYYY YY MM M DD D), schema (one "name|type" entry per column: no inference), columnTypes.<name>
 *    (a type text, "date:MM/DD/YYYY" for a date format), inferRowCount (0: all rows), onValueError (fail, null),
 *    allowMissingFields, allowExtraFields (false), maxFieldBytes, maxRecordBytes, maxColumns, batchByteLimit.
 *
 *  Writing (CDataTableWriterOptions::codecOptions): delimiter, quote, lineEnd (crlf, lf, cr), header (true), bom (false),
 *  nullToken (empty). Whatever is not given is taken from the key value metadata of the writer options under
 *  lineEnding, bom and delimiter, which is what the reader reports for a file: a file read and written again keeps its form.
 */
class GUCEF_CORE_PUBLIC_CPP CCsvTableCodec : public CITableCodec
{
    public:

    CCsvTableCodec( void );

    virtual ~CCsvTableCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileExtensions( CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus ReadMetaData( CIOAccess& source, const CValueList& options,
                                           CITableFileMetaDataPtr& metaData, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData,
                                           const CValueList& options, CITableReaderPtr& reader, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema,
                                           const CDataTableWriterOptions& options, CITableWriterPtr& writer, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Puts the schema in the read options as the "schema" option, one "name|type" entry per column (a trailing ! on the
     *  type marks a column that is not nullable), and switches the list to multiple values. The file is then read as that
     *  schema, without inference.
     */
    static void SetSchemaOption( CValueList& options, const CDataTableSchema& schema );

    /** Adds one entry to a list option such as nullTokens or dateFormats, switching the list to multiple values */
    static void AddListOption( CValueList& options, const CString& key, const CString& value );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVTABLECODEC_H ? */
