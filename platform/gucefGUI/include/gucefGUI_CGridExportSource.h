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

#ifndef GUCEF_GUI_CGRIDEXPORTSOURCE_H
#define GUCEF_GUI_CGRIDEXPORTSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  What an export reads: columns in the order they are written, their titles, and the text of the cells of every row.
 *
 *  Made from a grid, it takes the view as the user sees it at that moment: the shown columns in the order shown (hidden
 *  columns are left out), their titles, and the rows of the grid in its order, which is the order after the filter and
 *  the sort of the model (or after SortRows() of a string list). The text of a cell is what the grid shows and copies
 *  (CGridView::GetCellContent(), by the definition of the column), or with raw values the value of the model as plain
 *  text (CVariant::AsString(); a null is empty). A source of a grid with a string list reads the grid and is used on the
 *  thread of the grid only.
 *
 *  Made from a model and a list of columns it holds no grid (IsDetached()): it reads the model only and keeps it alive
 *  through the shared pointer, which is what an export on a pool thread needs; the model must then be safe to read from
 *  that thread while the export runs.
 */
class GUCEF_GUI_PUBLIC_CPP CGridExportSource : public CORE::CTSharedPtrCreator< CGridExportSource, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CGridExportSource, MT::CMutex > TGridExportSourcePtr;
    typedef GUCEF::vector< Int32 > TInt32Vector;
    typedef GUCEF::vector< CString > TStringVector;

    /** The view of the grid as it is now; with rawValues the values of the model instead of the text shown */
    CGridExportSource( const CGridView& grid, bool rawValues );

    /** The columns of a model (columns of the data, in the order written) with their titles; holds no grid */
    CGridExportSource( const TTableViewModelPtr& model, const TInt32Vector& columns, const TStringVector& titles, bool rawValues );

    virtual ~CGridExportSource();

    /** Reads a model only, no grid */
    bool IsDetached( void ) const;

    UInt32 GetColumnCount( void ) const;

    /** The columns of the data, in the order they are written */
    const TInt32Vector& GetColumns( void ) const;

    const TStringVector& GetTitles( void ) const;

    /** The rows there are now */
    UInt32 GetRowCount( void ) const;

    /** Announces the rows about to be read, so a model that fetches in pages reads them in one go */
    void PrepareRows( UInt32 firstRow, UInt32 rowCount );

    /**
     *  The texts of the cells of a row, one per column, with the marker of a cell (SCellContent::marker) where the grid draws it,
     *  after or before the value; false when the row is not there
     */
    bool GetRowTexts( UInt32 row, TStringVector& cells ) const;

    private:

    CGridExportSource( const CGridExportSource& src );
    CGridExportSource& operator=( const CGridExportSource& src );

    private:

    const CGridView* m_grid;            /**< a grid with a string list; NULL for a model */
    TTableViewModelPtr m_model;
    TInt32Vector m_columns;
    TStringVector m_titles;
    bool m_rawValues;
};

/*-------------------------------------------------------------------------*/

typedef CGridExportSource::TGridExportSourcePtr TGridExportSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDEXPORTSOURCE_H ? */
