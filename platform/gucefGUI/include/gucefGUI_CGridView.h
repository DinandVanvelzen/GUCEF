/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#define GUCEF_GUI_CGRIDVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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

/**
 *  A table: a header row (Columns) above rows of cells. Rows holds one entry per row with the cells
 *  separated by a tab character. A click selects a row (SelectedRow) and raises GridItemClickedEvent with
 *  the cell; a double click or Enter activates the row. Properties: Columns, ColumnWidths (empty: equal), Rows,
 *  SelectedRow, MultiSelect, SelectedRows, Sortable, SortColumn, SortAscending, RowHeight, Font, TextColour,
 *  HeaderColour, SelectionColour, HoverColour, BackgroundColour, ScrollPosition. Runtime: HoverIndex, AnchorRow.
 *
 *  Keys: Up / Down, Home / End and Page Up / Page Down move the selection, with Shift they extend it (MultiSelect);
 *  typing jumps to the next row whose first cell starts with the text. With MultiSelect a click selects one row,
 *  Ctrl + click adds or removes a row and Shift + click selects the rows from the anchor row to the clicked one.
 *  A click on the header raises ColumnClickedEvent; with Sortable the grid sorts its rows by that column (again:
 *  the other way round). A form that sorts the rows itself leaves Sortable off and sets SortColumn and
 *  SortAscending for the arrow in the header.
 */
class GUCEF_GUI_PUBLIC_CPP CGridView : public CWidget
{
    public:

    static const CORE::CEvent GridItemClickedEvent;
    static const CORE::CEvent GridItemAddedEvent;
    static const CORE::CEvent GridItemDeletedEvent;
    static const CORE::CEvent GridRowAddedEvent;
    static const CORE::CEvent GridRowDeletedEvent;
    static const CORE::CEvent GridColumnAddedEvent;
    static const CORE::CEvent GridColumnDeletedEvent;
    static const CORE::CEvent ColumnClickedEvent;
    static const CORE::CEvent SelectionChangedEvent;

    struct SGridIndex
    {
        UInt32 column;
        UInt32 row;
    };
    typedef struct SGridIndex TGridIndex;
    typedef CORE::CTCloneableObj< TGridIndex > TCloneableGridIndex;

    typedef TCloneableGridIndex TGridItemClickedEventData;
    typedef TCloneableGridIndex TGridItemAddedEventData;
    typedef TCloneableGridIndex TGridItemDeletedEventData;
    typedef TCloneableGridIndex TGridRowAddedEventData;
    typedef TCloneableGridIndex TGridRowDeletedEventData;
    typedef TCloneableGridIndex TGridColumnAddedEventData;
    typedef TCloneableGridIndex TGridColumnDeletedEventData;
    typedef TCloneableGridIndex TColumnClickedEventData;

    static void RegisterEvents( void );

    public:

    typedef GUCEF::vector< TGridIndex > TGridIndexVector;
    typedef GUCEF::vector< CString > TStringVector;
    typedef GUCEF::vector< TStringVector > TRowVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    CGridView( void );

    virtual ~CGridView();

    virtual void SetText( const UInt32 columnIndex ,
                          const UInt32 rowIndex    ,
                          const CString& text      );

    virtual CString GetText( const UInt32 columnIndex ,
                             const UInt32 rowIndex    ) const;

    /** Adds a row with the text in the given column */
    virtual void AppendText( const UInt32 columnIndex ,
                             const CString& text      );

    /** Adds a row with the text in the first column */
    virtual void AppendText( const CString& text );

    virtual void Clear( void );

    /** Every cell of the selected row */
    virtual void GetSelectedItems( TGridIndexVector& gridIndexList );

    UInt32 GetRowCount( void ) const;

    UInt32 GetColumnCount( void ) const;

    TStringVector GetRowCells( UInt32 rowIndex ) const;

    /** Widths of the columns in pixels */
    GUCEF::vector< Int32 > GetColumnWidths( void ) const;

    Int32 GetRowHeight( void ) const;

    /** The data row at a position relative to the grid (below the header), -1 when none */
    Int32 GetRowAt( Int32 localY ) const;

    Int32 GetColumnAt( Int32 localX ) const;

    /** Replaces every row; the selection is cleared */
    void SetRows( const TRowVector& rows );

    void SetAllowMultiSelect( bool allowMultiSelect );

    bool GetAllowMultiSelect( void ) const;

    /** The row of the cursor (the last one selected), -1 for none */
    Int32 GetSelectedRow( void ) const;

    void SetSelectedRow( Int32 row );

    /** The selected rows in ascending order; without MultiSelect the selected row only */
    TInt32Vector GetSelectedRows( void ) const;

    void SetSelectedRows( const TInt32Vector& rows );

    /**
     *  The rows that stand out (EmphasizedRows, with the colour EmphasisColour): the rows that need the player, such as the games where it is the player's turn. Only rows that
     *  are there are kept, in ascending order, each once
     */
    void SetEmphasizedRows( const TInt32Vector& rows );

    TInt32Vector GetEmphasizedRows( void ) const;

    bool IsRowEmphasized( Int32 row ) const;

    bool IsRowSelected( Int32 row ) const;

    /** How many rows fit below the header */
    Int32 GetVisibleRowCount( void ) const;

    /** Scrolls the least so that the row is visible */
    void ScrollToRow( Int32 row );

    /**
     *  Sorts the rows by the text of a column, case ignored, numbers by their value; equal rows keep their order.
     *  The selection stays with its rows. Sets SortColumn and SortAscending
     */
    void SortRows( UInt32 column, bool ascending );

    /** The room around the text of a cell and of a title, the room of the sort arrow in its title, the indent of the first cell of a row that stands out */
    enum ECellLayout
    {
        CELL_PADDING_LEFT = 2,
        CELL_PADDING_RIGHT = 8,
        SORT_ARROW_ROOM = 16,
        EMPHASIS_INDENT = 6
    };

    /** Where the title and the cells of a column stand (ColumnAlignments): "Left", "Center" or "Right"; Left without an entry */
    CString GetColumnAlignment( UInt32 column ) const;

    /** The pixels the text of a cell has in its column: the width of the column without the padding (and the indent of a row that stands out) */
    Int32 GetCellTextRoom( UInt32 column, Int32 row ) const;

    /** Whether the text of a cell is wider than its room: with TextOverflow Ellipsis it is drawn cut with "..." and its tooltip is the whole text */
    bool IsCellTextCut( UInt32 column, UInt32 row ) const;

    /** Over a cell whose text is cut: the whole text; else the Tooltip property */
    virtual CString GetTooltipAt( Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "GridView"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Selects rows as the user did: the properties change and ValueChangedEvent is raised for each one that did */
    void SelectByUser( const TInt32Vector& rows, Int32 cursorRow );

    /** Moves the cursor to a row by a key: selects it, or with extend the rows from the anchor row to it */
    void MoveCursorByUser( Int32 row, bool extend );

    CGridView( const CGridView& src );
    CGridView& operator=( const CGridView& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDVIEW_H ? */
