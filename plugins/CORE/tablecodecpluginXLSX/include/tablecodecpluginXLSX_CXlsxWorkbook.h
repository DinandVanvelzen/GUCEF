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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H
#define TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#include "tablecodecpluginXLSX_CXlsxPackage.h"
#define TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#include "tablecodecpluginXLSX_CXlsxDates.h"
#define TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXDATES_H ? */

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

/** One sheet of the workbook */
struct TABLECODECPLUGIN_XLSX_PUBLIC_CPP SXlsxSheetInfo
{
    CORE::CString name;
    CORE::CString partName;         /**< the part of the package with the cells, empty for a sheet that was not resolved */
    bool isWorksheet;               /**< false for chart sheets and the like, which hold no cells */
    bool hidden;

    SXlsxSheetInfo( void );
};

/**
 *  What a worksheet needs of the rest of the workbook: the sheets, the date system (workbook.xml), the shared strings
 *  (xl/sharedStrings.xml) and the number formats of the cell styles (xl/styles.xml), found through the relationships of
 *  the package.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxWorkbook
{
    public:

    explicit CXlsxWorkbook( const SXlsxReadOptions& options );

    ~CXlsxWorkbook();

    /** Reads the sheet list and the date system, then the shared strings and the styles when the workbook has them */
    CORE::TDataTableStatus Load( CXlsxPackage& package, CORE::CString& error );

    bool IsDate1904( void ) const;

    const GUCEF::vector< SXlsxSheetInfo >& GetSheets( void ) const;

    /** The sheet the options select: by name, else by index */
    CORE::TDataTableStatus SelectSheet( UInt32& sheetIndex, CORE::CString& error ) const;

    /** The kind of number the cell style makes of a number; PLAIN for a style that is not known */
    TXlsxNumberKind GetStyleKind( UInt32 styleIndex ) const;

    UInt32 GetSharedStringCount( void ) const;

    /** False for an index that is out of range */
    bool GetSharedString( UInt32 index, const char*& text, UInt32& length ) const;

    /** Text of the XML of a part: "_xHHHH_" is the character with that code, "_x005F_" a literal underscore */
    static void DecodeEscapes( const char* text, UInt32 length, CORE::CString& decoded );

    private:

    CXlsxWorkbook( const CXlsxWorkbook& src );                  /**< not implemented */
    CXlsxWorkbook& operator=( const CXlsxWorkbook& src );       /**< not implemented */

    friend class CXlsxSharedStringsParser;
    friend class CXlsxStylesParser;

    SXlsxReadOptions m_options;
    bool m_date1904;
    GUCEF::vector< SXlsxSheetInfo > m_sheets;
    GUCEF::vector< char > m_stringBytes;
    GUCEF::vector< UInt32 > m_stringOffsets;        /**< the start of every string in m_stringBytes, and the end of the last one */
    GUCEF::vector< UInt8 > m_styleKinds;            /**< per cell format (xf) */
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H ? */
