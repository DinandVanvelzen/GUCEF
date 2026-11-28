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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H
#define TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

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
 *  The xlsx table codec, type "xlsx", file extension xlsx: reads one worksheet of a Microsoft Excel workbook as a data
 *  table. Read only: CreateWriter() answers UNSUPPORTED. Shared; every member is reentrant.
 *
 *  The workbook is a zip package of XML parts. It is read through the zip archive type of the VFS (the plugin vfspluginZIP)
 *  and the XML codec of the data store codec registry (the plugin dstorepluginPARSIFALXML): both must be loaded, or the
 *  read answers UNSUPPORTED with the name of the plugin that is missing. See CXlsxPackage for how the package is opened.
 *
 *  What is read: the sheet of the options (sheetName or sheetIndex); cells of the types shared string, inline string, number,
 *  boolean, date (type d), the cached result of a formula (a formula without a cached value is a value error) and error
 *  values (a value error). Merged cells are not looked at: only the first cell of a merged range has a value. Numbers are
 *  read as the exact decimal text of the file. Dates are numbers with a date number format of xl/styles.xml, in the 1900 date
 *  system (with the 29th of February 1900 that Excel has and the calendar has not) or the 1904 one of the workbook.
 *
 *  The text of cells goes through the XML reader of the data store codecs, which normalizes the white space of the text of an
 *  element: leading and trailing white space is dropped and a run of white space becomes one space. Strings come out so.
 *
 *  The header is the first row with a value, or the row of the options; empty and repeated header names are named like the
 *  CSV codec names them. Rows without a value are not rows of the table; a row that has fewer cells than the header has null
 *  for the others. Types: from the options (schema, columnTypes.*), else inferred over the cells (bool, int64, decimal(p,s),
 *  float64 only where the digits do not fit a decimal, date, timestamp(us), utf8). Read options: see SXlsxReadOptions.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxTableCodec : public CORE::CITableCodec
{
    public:

    static const char* const TypeName;

    CXlsxTableCodec( void );

    virtual ~CXlsxTableCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetFileExtensions( CORE::CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ReadMetaData( CORE::CIOAccess& source, const CORE::CValueList& options,
                                                 CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The metadata must be NULL (it is read then) or have been made by this codec with the same options */
    virtual CORE::TDataTableStatus CreateReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData,
                                                 const CORE::CValueList& options, CORE::CITableReaderPtr& reader, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: UNSUPPORTED, workbooks are only read */
    virtual CORE::TDataTableStatus CreateWriter( const CORE::IOAccessPtr& destination, const CORE::CDataTableSchemaPtr& schema,
                                                 const CORE::CDataTableWriterOptions& options, CORE::CITableWriterPtr& writer, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CXlsxTableCodec( const CXlsxTableCodec& src );                /**< not implemented: Clone() makes a new codec */
    CXlsxTableCodec& operator=( const CXlsxTableCodec& src );     /**< not implemented */
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H ? */
