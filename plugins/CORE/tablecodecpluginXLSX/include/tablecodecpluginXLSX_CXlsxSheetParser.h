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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H
#define TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H
#include "tablecodecpluginXLSX_CXlsxWorkbook.h"
#define TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXWORKBOOK_H ? */

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

/** What a cell with a value holds */
enum EXlsxCellKind
{
    XLSX_CELL_NUMBER = 0        ,   /**< the text of a number as the file states it, exact: "12.5", "1E-3"; numberKind tells whether the cell format makes it a date */
    XLSX_CELL_STRING            ,   /**< text: a shared string, an inline string or the cached result of a formula */
    XLSX_CELL_BOOL              ,   /**< boolValue */
    XLSX_CELL_ISODATE           ,   /**< a date or date time as ISO 8601 text (cell type d) */
    XLSX_CELL_ERROR             ,   /**< an error value of the sheet: #DIV/0!, #N/A and so on */
    XLSX_CELL_UNCACHED_FORMULA      /**< a formula that has no cached value: the workbook was never calculated */
};
typedef enum EXlsxCellKind TXlsxCellKind;

/** One cell with a value. The text is valid during the call that gets it */
struct TABLECODECPLUGIN_XLSX_PUBLIC_CPP SXlsxCell
{
    TXlsxCellKind kind;
    const char* text;
    UInt32 length;
    bool boolValue;
    TXlsxNumberKind numberKind;

    SXlsxCell( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Receives the rows of a worksheet in order. Rows and the cells of a row come in increasing order; a row without values
 *  gets its begin and end only. A call that gives an error stops the sheet: the rest of it is not read.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CIXlsxRowSink
{
    public:

    CIXlsxRowSink( void );
    virtual ~CIXlsxRowSink();

    /** row is the 1 based row number of the sheet */
    virtual CORE::TDataTableStatus OnRowBegin( UInt32 row, CORE::CString& error ) = 0;

    /** column is 0 based, from column A */
    virtual CORE::TDataTableStatus OnCell( UInt32 row, UInt32 column, const SXlsxCell& cell, CORE::CString& error ) = 0;

    virtual CORE::TDataTableStatus OnRowEnd( UInt32 row, CORE::CString& error ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads the XML of a worksheet and gives the cells to a row sink: the cell types s (shared string), n (number, also when
 *  the type is not stated), str (the cached text of a formula), b, inlineStr, d and e. The cached value of a formula is the
 *  value of its cell. Merged cells (mergeCells) are not looked at: the cells of a merged range other than its first hold no
 *  value, so they are empty. The limits of the options (rows, columns, cells, text of a cell) are held here.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxSheetParser : public CXlsxXmlCallbacks
{
    public:

    CXlsxSheetParser( const CXlsxWorkbook& workbook, const SXlsxReadOptions& options, CIXlsxRowSink& sink, CXlsxPackage& package );

    virtual ~CXlsxSheetParser() GUCEF_VIRTUAL_OVERRIDE;

    /** The 1 based number of the last row that was read, 0 for none */
    UInt32 GetLastRow( void ) const;

    /** The cells with a value that were read */
    UInt64 GetCellCount( void ) const;

    virtual void OnFailed( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeBegin( const CORE::CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE;

    /** Column number of a cell reference such as "BC12": 0 based, the row number in the digits; false for text that is no reference */
    static bool ParseCellReference( const char* text, UInt32 length, UInt32& column, UInt32& row );

    /** "C12" for column 2 (0 based) and row 12 */
    static CORE::CString FormatCellReference( UInt32 column, UInt32 row );

    private:

    CXlsxSheetParser( const CXlsxSheetParser& src );                /**< not implemented */
    CXlsxSheetParser& operator=( const CXlsxSheetParser& src );     /**< not implemented */

    enum EElement
    {
        ELEMENT_OTHER = 0   ,
        ELEMENT_SHEETDATA   ,
        ELEMENT_ROW         ,
        ELEMENT_CELL        ,
        ELEMENT_VALUE       ,
        ELEMENT_FORMULA     ,
        ELEMENT_INLINE      ,
        ELEMENT_TEXT        ,
        ELEMENT_PHONETIC
    };

    void BeginRow( void );
    void EndCell( void );
    bool Check( CORE::TDataTableStatus status, const CORE::CString& error );

    const CXlsxWorkbook& m_workbook;
    const SXlsxReadOptions& m_options;
    CIXlsxRowSink& m_sink;
    CXlsxPackage& m_package;

    GUCEF::vector< UInt8 > m_stack;
    UInt32 m_phoneticDepth;
    bool m_inRow;
    bool m_inCell;
    bool m_inInline;
    bool m_rowBegun;
    UInt32 m_rowNumber;             /**< 1 based */
    UInt32 m_lastRow;
    bool m_hasCellColumn;
    UInt32 m_nextColumn;            /**< 0 based: the column of the next cell that has no reference */
    UInt32 m_cellColumn;
    UInt32 m_cellStyle;
    CORE::CString m_cellType;
    bool m_hasValue;
    bool m_hasFormula;
    bool m_hasInline;
    GUCEF::vector< char > m_text;   /**< the value or inline text of the cell, reused for every cell */
    UInt64 m_cellCount;
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

#endif /* TABLECODECPLUGIN_XLSX_CXLSXSHEETPARSER_H ? */
