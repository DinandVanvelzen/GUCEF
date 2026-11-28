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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H
#define TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

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

/** Where the table is in the sheet that the metadata was read from */
struct TABLECODECPLUGIN_XLSX_PUBLIC_CPP SXlsxLayout
{
    UInt32 sheetIndex;
    CORE::CString sheetName;
    UInt32 headerRow;           /**< 1 based sheet row of the header, 0 when the table has none */
    UInt32 dataStartRow;        /**< 1 based: the rows from here on are the rows of the table */
    UInt32 firstColumn;         /**< 0 based: the column of the first column of the table */
    UInt32 width;               /**< the columns of the table */
    bool date1904;

    SXlsxLayout( void );
};

/**
 *  The file metadata of a worksheet read as a table: the schema (given or inferred), the number of rows (known: the sheet
 *  was read to its end), key values and the layout. One row group, without statistics.
 *
 *  Key values: sheetName, sheetIndex, headerRow, dateSystem (1900 or 1904), lossyInference (the number of columns that were
 *  inferred as float64 with numbers of more than 15 digits) and lossyInference.<column name> with the number of such values.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxFileMetaData : public CORE::CITableFileMetaData
{
    public:

    CXlsxFileMetaData( const CORE::CDataTableSchemaPtr& schema, const CORE::CValueList& keyValues, const SXlsxLayout& layout, UInt64 rowCount );

    virtual ~CXlsxFileMetaData() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetKeyValueMetaData( CORE::CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE;

    const SXlsxLayout& GetLayout( void ) const;

    private:

    CORE::CDataTableSchemaPtr m_schema;
    CORE::CValueList m_keyValues;
    SXlsxLayout m_layout;
    UInt64 m_rowCount;
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXFILEMETADATA_H ? */
