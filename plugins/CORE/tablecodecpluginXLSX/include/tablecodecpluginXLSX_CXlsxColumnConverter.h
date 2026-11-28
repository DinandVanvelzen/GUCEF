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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H
#include "tablecodecpluginXLSX_CXlsxSheetParser.h"
#define TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A number text split in its parts, without a floating point number */
struct TABLECODECPLUGIN_XLSX_PUBLIC_CPP SXlsxNumberParts
{
    bool negative;
    UInt64 whole;                   /**< the digits in front of the decimal point */
    char fraction[ 16 ];            /**< the first 15 digits behind it */
    UInt32 fractionCount;
    bool fractionIsZero;            /**< every digit behind the decimal point is 0 */

    SXlsxNumberParts( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns a cell into the value of a column of a data table. Numbers are read as exact decimal text: a decimal column gets
 *  the digits of the text, an integer column the integer of the text, and only a float column goes through a floating point
 *  number. A cell that does not fit its column is a value error: the failure of the read, or a null when the options say so.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxColumnConverter
{
    public:

    CXlsxColumnConverter( const SXlsxReadOptions& options, bool date1904 );

    /**
     *  Appends the cell to the builder of the column. OK, or TYPE_MISMATCH for a cell that does not fit (with the reason in
     *  error) that the options do not turn into a null; the value errors that were made null are counted.
     */
    CORE::TDataTableStatus Convert( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder,
                                    UInt64& valueErrors, CORE::CString& error ) const;

    /** Appends a null: a cell the row has not got */
    CORE::TDataTableStatus ConvertBlank( const CORE::CDataTableColumnDef& column, CORE::CDataTableColumnBuilder& builder, CORE::CString& error ) const;

    /** Splits "-12.50", "1E-3" and the like; false for text that is no number */
    static bool SplitNumber( const char* text, UInt32 length, SXlsxNumberParts& parts );

    private:

    bool ConvertValue( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const;
    bool ConvertDate( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const;
    bool ConvertNumber( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const;
    bool ConvertText( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const;

    const SXlsxReadOptions& m_options;
    bool m_date1904;
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNCONVERTER_H ? */
