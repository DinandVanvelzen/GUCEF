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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H
#define TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H
#include "tablecodecpluginXLSX_CXlsxColumnInference.h"
#define TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXCOLUMNINFERENCE_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#include "tablecodecpluginXLSX_CXlsxFileMetaData.h"
#define TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H ? */

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
 *  Finds the schema of a worksheet: the header row (the first row with a value, or the row of the options), the columns,
 *  and their types from the schema or the type overrides of the options and else from the cells (inference over inferRowCount
 *  rows, all rows when it is 0). The whole sheet is read, so the row count is known afterwards.
 *
 *  Names of the columns: the text of the header cells, control characters as spaces, trimmed. A name that is empty becomes
 *  column_<n> (n counts the columns of the table from 1) and a name that is there twice gets _2, _3 and so on, as the CSV
 *  codec names its columns.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxSchemaResolver : public CIXlsxRowSink
{
    public:

    /** Opens the package, loads the workbook and selects the sheet of the options */
    static CORE::TDataTableStatus OpenSheet( CORE::CIOAccess& source, const SXlsxReadOptions& options, CXlsxPackage& package,
                                             CXlsxWorkbook& workbook, UInt32& sheetIndex, CORE::CString& error );

    static CORE::TDataTableStatus Resolve( CORE::CIOAccess& source, const SXlsxReadOptions& options,
                                           CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error );

    explicit CXlsxSchemaResolver( const SXlsxReadOptions& options );

    virtual ~CXlsxSchemaResolver() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OnRowBegin( UInt32 row, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus OnCell( UInt32 row, UInt32 column, const SXlsxCell& cell, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus OnRowEnd( UInt32 row, CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    /** Once the sheet is read: the metadata */
    CORE::TDataTableStatus Finish( const CORE::CString& sheetName, UInt32 sheetIndex, bool date1904, CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error );

    private:

    struct SStoredCell
    {
        UInt32 index;
        TXlsxCellKind kind;
        CORE::CString text;
        bool boolValue;
        TXlsxNumberKind numberKind;
    };

    enum ERowKind { ROW_SKIP = 0, ROW_HEADER, ROW_DATA };

    static SStoredCell Store( UInt32 index, const SXlsxCell& cell );
    static SXlsxCell Unstore( const SStoredCell& stored );
    CORE::TDataTableStatus InitColumns( UInt32 width, const CORE::CString::StringVector* headerNames, CORE::CString& error );
    void FeedCell( UInt32 index, const SXlsxCell& cell );

    const SXlsxReadOptions& m_options;
    UInt32 m_firstColumn;               /**< 0 based */
    UInt32 m_windowEnd;                 /**< 0 based, exclusive; the maximum when the table has no last column */
    bool m_waitingForHeader;
    bool m_widthKnown;
    UInt32 m_width;
    UInt32 m_headerRow;
    ERowKind m_rowKind;
    bool m_rowHasValue;
    bool m_rowFeeding;
    UInt64 m_rowCount;
    GUCEF::vector< SStoredCell > m_stored;
    GUCEF::vector< UInt8 > m_present;
    CORE::CString::StringVector m_names;
    GUCEF::vector< CXlsxColumnInference > m_inference;
    GUCEF::vector< const CORE::CDataTableColumnDef* > m_fixed;
    bool m_anyInference;
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H ? */
