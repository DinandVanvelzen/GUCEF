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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What is known of a column while its cells are looked at. A cell can only turn possibilities off, so the type that results
 *  only gets wider with more rows: bool; int64, decimal(p,s) and float64 for numbers (exact decimals first, a float64 only
 *  when the digits do not fit a decimal of 18 digits); date, and timestamp(us) when the dates have a time of day; utf8.
 *  Cells of different kinds in one column (numbers and text, dates and numbers) make the column utf8.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxColumnInference
{
    public:

    CXlsxColumnInference( void );

    /** Looks at one cell */
    void Feed( const SXlsxCell& cell, const SXlsxReadOptions& options );

    /** A row that has no cell for the column */
    void FeedBlank( void );

    /** The column that the cells seen allow. allNullable makes it nullable whatever was seen */
    CORE::CDataTableColumnDef GetColumn( const CORE::CString& name, bool allNullable ) const;

    /** The numbers with more than 15 digits in a column that is float64: a float64 does not keep them exactly */
    UInt64 GetLossyFloatValueCount( void ) const;

    private:

    bool m_sawBlank;
    bool m_sawBool;
    bool m_sawNumber;
    bool m_sawString;
    bool m_sawDate;
    bool m_sawTimeOfDay;        /**< a date with a time of day */
    bool m_allIntegral;
    bool m_allExact;            /**< every number is an exact decimal of at most 18 digits */
    UInt32 m_maxIntDigits;
    UInt32 m_maxFracDigits;
    UInt64 m_lossyValues;
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H ? */
