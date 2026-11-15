/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H
#define GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGridView;

/**
 *  The list of the column chooser: a row per column of a grid, in the order of all its columns (CGridView::GetColumnOrder(),
 *  hidden columns at their place), with the columns Shown (a check box), Column (the title) and Note ("Always shown" for
 *  a required column, "Hidden by default" for a column the model offers hidden). Ticking Shown hides or shows the column
 *  of the grid at once; the check box of a column that cannot be hidden (required, or the last one shown) is read only.
 *  The key of a row is the column of the data, so a cursor stays on its column when the order changes. Refresh() reads
 *  the grid again and raises ModelResetEvent.
 */
class GUCEF_GUI_PUBLIC_CPP CGridColumnChooserModel : public CITableViewModel
{
    public:

    enum EColumn
    {
        COLUMN_SHOWN = 0    ,
        COLUMN_TITLE        ,
        COLUMN_NOTE         ,
        COLUMN_COUNT
    };

    CGridColumnChooserModel( void );

    virtual ~CGridColumnChooserModel() GUCEF_VIRTUAL_OVERRIDE;

    /** The grid listed; NULL for none */
    void SetGrid( CGridView* grid );

    CGridView* GetGrid( void ) const;

    /** Reads the order and the state of the columns of the grid again and raises ModelResetEvent */
    void Refresh( void );

    /** The column of the data of the grid a row stands for, -1 for none */
    Int32 GetGridColumn( UInt32 row ) const;

    /** The row of a column of the data of the grid, -1 for none */
    Int32 FindRow( Int32 gridColumn ) const;

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsCellReadOnly( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CTableEditResult ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CTableEditResult SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGridColumnChooserModel( const CGridColumnChooserModel& src );
    CGridColumnChooserModel& operator=( const CGridColumnChooserModel& src );

    private:

    CGridView* m_grid;
    GUCEF::vector< Int32 > m_order;         /**< the column of the data of every row */
    CTableViewColumn m_columns[ COLUMN_COUNT ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H ? */
