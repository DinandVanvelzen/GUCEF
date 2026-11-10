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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNLAYOUT_H
#include "gucefGUI_CGridColumnLayout.h"
#define GUCEF_GUI_CGRIDCOLUMNLAYOUT_H
#endif /* GUCEF_GUI_CGRIDCOLUMNLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDSELECTION_H
#include "gucefGUI_CGridSelection.h"
#define GUCEF_GUI_CGRIDSELECTION_H
#endif /* GUCEF_GUI_CGRIDSELECTION_H ? */

#ifndef GUCEF_GUI_CGRIDEDITREPORT_H
#include "gucefGUI_CGridEditReport.h"
#define GUCEF_GUI_CGRIDEDITREPORT_H
#endif /* GUCEF_GUI_CGRIDEDITREPORT_H ? */

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

class CGridCellEditor;
class CGridEditController;
class CGridClipboard;

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
 *
 *  Everything below is optional and off by default; a grid that uses none of it is the grid described above.
 *
 *  A model (SetModel) holds the data instead of the Rows property: the grid asks it for the cells it shows only, so
 *  a table of millions of rows is as cheap as one of ten; it follows the changes the model announces and keeps its
 *  cursor and selection with the rows (by key) when the model re-orders. The cells are typed and shown by the
 *  definition of their column (numbers, dates, check marks, null).
 *
 *  SelectionMode Cell turns the row selection into a cell cursor with ranges of cells: a click selects a cell, a
 *  drag or Shift + click a block, Ctrl + click adds a block, the row header a whole row, the column header a whole
 *  column (when Sortable is off), the corner everything. Keys: the arrows move the cursor and with Shift extend the
 *  block, with Ctrl they go to the edge of the data; Home, End, Page Up and Page Down move along the row and the
 *  rows; Ctrl + Home / End go to the first and the last cell; Tab and Enter move through the block or the grid
 *  (Shift: backwards); Ctrl + A selects everything, Ctrl + Space the column and Shift + Space the row of the cursor.
 *
 *  Layout: ShowRowHeader (numbers, or the row titles of the model), HorizontalScroll (columns keep their widths, Shift
 *  + wheel scrolls), ResizableColumns (drag the border of a title, a double click fits the column to its content),
 *  MovableColumns (drag a title), ColumnOrder, HiddenColumns, FrozenColumns, FrozenRows, ShowFooter (the footer row of
 *  the model; not set: shown when the model has a footer), ZebraColour, GridLines and GridColour, ScrollBars.
 *
 *  Updates: BeginUpdate() and EndUpdate() (they nest) hold the notifications of the selection back and, with a model,
 *  the following of the changes by place: the selection stays with its rows by key, and SelectionChangedEvent is raised
 *  once when the update ends. A model that brackets its changes with BeginBatch() and EndBatch() does the same.
 *
 *  CellEditing (cell mode, with a model that has cells that are not read only): F2, Enter (one cell selected), a
 *  double click or typing on the cursor cell opens an editor for the type of the column: a text editor, a number editor
 *  that takes digits, signs and the separators only, a date editor, a list of the choices of the column; a boolean is
 *  toggled by a click or Space. Enter commits and goes to the cell below, Tab commits and goes to the next cell, Escape
 *  cancels; Delete clears the selected cells. The grid reports the edit and asks the model to apply it
 *  (ApplyCellEdit); the model validates while the user types (a refusal outlines the editor in red and says why, as a
 *  tooltip too), and owns undo and redo, which Ctrl+Z and Ctrl+Y ask for. CellClipboard: Ctrl+C, Ctrl+X, Ctrl+V (also
 *  Ctrl+Insert, Shift+Delete, Shift+Insert), Ctrl+D (fill down) and Ctrl+R (fill right) work on the selected cells as
 *  tab separated text, through the clipboard of CSecretInput, and report what they did with RangeEditResultEvent.
 *
 *  These features are drawn by the own painter of gucefGUI (CWidgetPainter), which is what the drivers that draw
 *  widgets themselves use; GetCapabilities() names them for the drivers that do.
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
    static const CORE::CEvent CellCursorChangedEvent;           /**< TCellCursorChangedEventData: the cursor moved to another cell (SelectionMode Cell); the column is the column of the data */
    static const CORE::CEvent CellSelectionChangedEvent;        /**< the ranges of selected cells changed (SelectionMode Cell); no data, read GetSelection() */
    static const CORE::CEvent ColumnResizedEvent;               /**< TColumnChangedEventData: the user (or SetColumnWidth) changed the width of a column; value is the new width */
    static const CORE::CEvent ColumnMovedEvent;                 /**< TColumnChangedEventData: a column was moved; value is its new place among the shown columns */
    static const CORE::CEvent ColumnVisibilityChangedEvent;     /**< TColumnChangedEventData: a column was hidden (value 0) or shown (value 1) */
    static const CORE::CEvent CellEditStartedEvent;             /**< TGridEditReportEventData: an editor opened on the cell (row, column of the data) */
    static const CORE::CEvent CellEditCommittedEvent;           /**< TGridEditReportEventData: the model took the value of the editor, or the toggle of a check box */
    static const CORE::CEvent CellEditCancelledEvent;           /**< TGridEditReportEventData: the editor closed without a change */
    static const CORE::CEvent CellEditRefusedEvent;             /**< TGridEditReportEventData: the cell is read only or the model refused the value; the message says why. The editor stays open */
    static const CORE::CEvent RangeEditResultEvent;             /**< TGridEditReportEventData: what a copy, cut, paste, fill or clear did: counts of the cells taken, refused, read only and left out, and the first reason */

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
    typedef TCloneableGridIndex TCellCursorChangedEventData;
    typedef CORE::CTCloneableObj< CGridEditReport > TGridEditReportEventData;

    /** A change of a column: which column of the data, and a width, a place or a flag by the event */
    struct SColumnChange
    {
        UInt32 column;
        Int32 value;
    };
    typedef struct SColumnChange TColumnChange;
    typedef CORE::CTCloneableObj< TColumnChange > TColumnChangedEventData;

    static void RegisterEvents( void );

    public:

    typedef GUCEF::vector< TGridIndex > TGridIndexVector;
    typedef GUCEF::vector< CString > TStringVector;
    typedef GUCEF::vector< TStringVector > TRowVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    /** What is under a point of the grid */
    enum EHitKind
    {
        HIT_NONE = 0        ,
        HIT_CORNER          ,   /**< above the row header, left of the column titles */
        HIT_COLUMN_HEADER   ,
        HIT_COLUMN_BORDER   ,   /**< the right edge of a column title, where ResizableColumns grabs it; column is the column at the left of it */
        HIT_ROW_HEADER      ,
        HIT_CELL            ,
        HIT_FOOTER
    };

    struct SHit
    {
        EHitKind kind;
        Int32 row;              /**< -1 when there is no row */
        Int32 column;           /**< the column of the data, -1 when there is none */
        Int32 position;         /**< the place of the column among the shown columns, -1 when there is none */
    };
    typedef struct SHit THit;

    /** The room around the text of a cell and of a title, the room of the sort arrow in its title, the indent of the first cell of a row that stands out */
    enum ECellLayout
    {
        CELL_PADDING_LEFT = 2,
        CELL_PADDING_RIGHT = 8,
        SORT_ARROW_ROOM = 16,
        EMPHASIS_INDENT = 6,
        MIN_COLUMN_WIDTH = 24,
        DEFAULT_COLUMN_WIDTH = 100,
        BORDER_GRAB_ROOM = 3
    };

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

    /** Removes the rows of the string list and the selection; with a model it clears the selection only, the model owns the rows */
    virtual void Clear( void );

    /** Every cell of the selected row */
    virtual void GetSelectedItems( TGridIndexVector& gridIndexList );

    UInt32 GetRowCount( void ) const;

    UInt32 GetColumnCount( void ) const;

    TStringVector GetRowCells( UInt32 rowIndex ) const;

    /** Widths of the columns in pixels, by column of the data (hidden columns included) */
    GUCEF::vector< Int32 > GetColumnWidths( void ) const;

    Int32 GetRowHeight( void ) const;

    /** The data row at a position relative to the grid (below the header), -1 when none */
    Int32 GetRowAt( Int32 localY ) const;

    /** The column of the data at a position relative to the grid, -1 when none */
    Int32 GetColumnAt( Int32 localX ) const;

    /** Replaces every row; the selection is cleared. Without effect while the grid has a model */
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

    /** The row stands out because it is in EmphasizedRows or because the model says so */
    bool IsRowEmphasized( Int32 row ) const;

    /** Only the model decides: its answer for the row, false without a model */
    bool IsRowEmphasizedByModel( Int32 row ) const;

    bool IsRowSelected( Int32 row ) const;

    /** How many rows fit below the header (and above the footer); a part of a row is not counted */
    Int32 GetVisibleRowCount( void ) const;

    /** Scrolls the least so that the row is visible */
    void ScrollToRow( Int32 row );

    /**
     *  Sorts the rows by the text of a column, case ignored, numbers by their value; equal rows keep their order.
     *  The selection stays with its rows. Sets SortColumn and SortAscending. With a model the model sorts: when it
     *  does not support it (IsSortSupported) nothing changes and false is returned
     */
    bool SortRows( UInt32 column, bool ascending );

    /** Where the title and the cells of a column stand (ColumnAlignments): "Left", "Center" or "Right"; the alignment of the type for a column of a model; Left without an entry */
    CString GetColumnAlignment( UInt32 column ) const;

    /** The pixels the text of a cell has in its column: the width of the column without the padding (and the indent of a row that stands out) */
    Int32 GetCellTextRoom( UInt32 column, Int32 row ) const;

    /** Whether the text of a cell is wider than its room: with TextOverflow Ellipsis it is drawn cut with "..." and its tooltip is the whole text */
    bool IsCellTextCut( UInt32 column, UInt32 row ) const;

    /** Over the cell of an editor that holds a refused value: the reason; over a cell whose text is cut: the whole text; else the Tooltip property */
    virtual CString GetTooltipAt( Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "GridView"; }

    static CWidget* Create( void );

    /*
     *  Model
     */

    /** The model holds the data from now on (NULL: back to the Rows property). The grid follows the events of the model and does not own it beyond the shared pointer */
    void SetModel( const TTableViewModelPtr& model );

    const TTableViewModelPtr& GetModel( void ) const;

    bool HasModel( void ) const;

    /** What a cell shows, by the model (typed, by the definition of the column) or by the text of the string list; false for a cell that is not there */
    bool GetCellContent( Int32 row, Int32 column, SCellContent& content ) const;

    /** Whether a cell has no value: not there, null, or text without characters */
    bool IsCellEmpty( Int32 row, Int32 column ) const;

    /** Announces the rows in view to the model (PrepareRange), so that a model that fetches in pages reads them in one go; the painter does it before it reads the cells */
    void PrepareVisibleCells( void ) const;

    /** The title of a column: of the model, or of the Columns property */
    CString GetColumnTitle( UInt32 column ) const;

    /** The text of the row header: the title of the row by the model, else its number from 1 */
    CString GetRowHeaderText( Int32 row ) const;

    /** The footer of the model in a column, false when there is no model, no footer or no value */
    bool GetFooterContent( UInt32 column, SCellContent& content ) const;

    /** The text of the footer cell as the model gives it for copying; empty without a footer */
    CString GetFooterText( UInt32 column ) const;

    /** Whether the footer row is there: the model has a footer, and ShowFooter is not set to false on the grid */
    bool IsFooterShown( void ) const;

    /*
     *  Updates
     */

    /** Holds back the notifications of the selection and the following of the model by place until the matching EndUpdate(); calls nest */
    void BeginUpdate( void );

    /** The outermost call finds the selection again by key when the model changed, and raises SelectionChangedEvent once if the selection changed */
    void EndUpdate( void );

    bool IsUpdating( void ) const;

    /*
     *  Editing (CellEditing) and clipboard (CellClipboard)
     */

    /** Asks the model whether it takes the value (ValidateCellEdit) and then applies it (SetCellValue); the answer of the model. Read only when there is no model */
    CTableEditResult ApplyCellEdit( Int32 row, Int32 column, const CORE::CVariant& value );

    /** CellEditing is set, the grid is in cell mode and has a model */
    bool IsEditingEnabled( void ) const;

    /** Editing is enabled and the model does not say the cell is read only */
    bool IsCellEditable( Int32 row, Int32 column ) const;

    bool IsEditing( void ) const;

    /** Opens the editor on the cell with the text of its value (a check box is toggled instead); false when the cell cannot be edited */
    bool BeginEdit( Int32 row, Int32 column );

    /** Gives the value of the editor to the model; false when the model refuses it, the editor then stays open */
    bool CommitEdit( void );

    void CancelEdit( void );

    /** The open editor, NULL when none is open */
    const CGridCellEditor* GetCellEditor( void ) const;

    /** The editor session, for the painter and for the geometry of the list of choices; NULL when no editor was ever needed */
    const CGridEditController* GetEditController( void ) const;

    /** Copies the selected cells as tab separated text to the clipboard; with cut the cells are cleared afterwards. False when the selection cannot be copied (the ranges do not line up) */
    bool CopySelection( bool cut );

    /** Pastes the text of the clipboard at the selection, cell by cell through the model */
    bool PasteFromClipboard( void );

    /** Copies the first row (or the row above, when one row is selected) of the selected ranges down, or the first column (the column at the left) to the right */
    bool FillSelection( bool down );

    /** Clears the selected cells (the empty value of the type of their column) */
    bool ClearSelectedCells( void );

    /** What the last edit operation did, also raised as an event */
    const CGridEditReport& GetLastEditReport( void ) const;

    /** The cells copied last, outlined while they are still the copy; NULL when there is none */
    const CGridSelection* GetCopyOutline( void ) const;

    void ClearCopyOutline( void );

    /*
     *  Cell mode (SelectionMode Cell)
     */

    bool IsCellMode( void ) const;

    /** The cell of the cursor, -1 for none; the column is the column of the data */
    Int32 GetCursorRow( void ) const;

    Int32 GetCursorColumn( void ) const;

    /** Selects the one cell and moves the cursor to it, bringing it into view */
    void SetCursor( Int32 row, Int32 column );

    /** The ranges of selected cells, as places among the shown columns */
    const CGridSelection& GetSelection( void ) const;

    bool IsCellSelected( Int32 row, Int32 column ) const;

    /** Selects the block between two cells (columns of the data, in the order they are shown), whole rows, whole columns, everything, nothing */
    void SelectRange( Int32 firstRow, Int32 firstColumn, Int32 lastRow, Int32 lastColumn );

    void SelectRows( Int32 firstRow, Int32 lastRow );

    void SelectColumns( Int32 firstColumn, Int32 lastColumn );

    void SelectAll( void );

    void ClearSelection( void );

    /*
     *  Layout
     */

    /** Where the shown columns stand, see CGridColumnLayout */
    const CGridColumnLayout& GetColumnLayout( void ) const;

    /** The width in pixels of a column of the data; a hidden column keeps its width */
    Int32 GetColumnWidth( UInt32 column ) const;

    /** Sets the width of a column (at least MIN_COLUMN_WIDTH); the widths of all columns become pixels. Raises ColumnResizedEvent */
    void SetColumnWidth( UInt32 column, Int32 width );

    /** Widens or narrows a column to the widest of its title and the cells shown now (the rows in view; a model is asked for those only) */
    void FitColumnToContent( UInt32 column );

    /** Puts a column at a place among the shown columns. Raises ColumnMovedEvent */
    void MoveColumn( UInt32 column, UInt32 position );

    void SetColumnHidden( UInt32 column, bool hidden );

    bool IsColumnHidden( UInt32 column ) const;

    /** The place the column being dragged would be dropped at, -1 when no column is being dragged */
    Int32 GetColumnDropPosition( void ) const;

    /** The number of rows that do not scroll, kept within the rows that fit */
    Int32 GetFrozenRowCount( void ) const;

    /** The first row of the rows that scroll */
    Int32 GetFirstScrolledRow( void ) const;

    /** The largest scroll position of the rows: the first row of the rows that scroll when the last row is in view */
    Int32 GetMaxScrollRow( void ) const;

    /** The room at the left for the row header, 0 without it */
    Int32 GetRowHeaderWidth( void ) const;

    /** The y of the top of the footer row, -1 without it */
    Int32 GetFooterTop( void ) const;

    /** The y a row is drawn at, -1 for a row that is scrolled out */
    Int32 GetRowTop( Int32 row ) const;

    /** The rectangle of a cell in the grid (column of the data), false when it is out of view; the rectangle is cut by the area it scrolls in */
    bool GetCellRect( Int32 row, Int32 column, CRect& rect ) const;

    /** Scrolls both ways so that the cell is in view */
    void ScrollToCell( Int32 row, Int32 column );

    THit HitTest( Int32 localX, Int32 localY ) const;

    /**
     *  Names the optional features of this grid that the own painter draws: GridModel, GridTypedCells, GridCellSelection, GridLayout,
     *  GridModelContent (the model gives the content of cells and footer cells and which rows stand out), GridBatchUpdate (updates and batches
     *  of the model), GridCellEditing and GridClipboard. A driver that draws widgets with CWidgetPainter adds them to its capabilities
     */
    static void GetCapabilities( GUCEF::set< CString >& capabilities );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CGridEditController;
    friend class CGridClipboard;

    typedef CORE::CTEventHandlerFunctor< CGridView > TEventCallback;

    /** What a mouse press that is held started */
    enum EDragMode
    {
        DRAG_NONE = 0   ,
        DRAG_CELLS      ,   /**< extends the block from the anchor to the cell under the pointer */
        DRAG_ROWS       ,
        DRAG_RESIZE     ,   /**< changes the width of a column */
        DRAG_MOVE           /**< moves a column; it starts when the pointer moved a few pixels */
    };

    /** The keys of the rows the user selected, for a model that re-orders */
    struct SKeyedSelection
    {
        bool hasCursor;
        CORE::CVariant cursorKey;
        bool hasAnchor;
        CORE::CVariant anchorKey;
        GUCEF::vector< CORE::CVariant > rowKeys;       /**< the selected rows (row mode), the rows selected whole (cell mode) */
        bool overflow;                                  /**< more rows than are kept: the selection does not survive a re-order */

        SKeyedSelection( void ) : hasCursor( false ), cursorKey(), hasAnchor( false ), anchorKey(), rowKeys(), overflow( false ) {}
    };

    void OnModelCellsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelRowsInserted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelRowsRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelColumnsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelFooterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelBatchStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelBatchFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelReset( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** Selects rows as the user did: the properties change and ValueChangedEvent is raised for each one that did */
    void SelectByUser( const TInt32Vector& rows, Int32 cursorRow );

    /** Moves the cursor to a row by a key: selects it, or with extend the rows from the anchor row to it */
    void MoveCursorByUser( Int32 row, bool extend );

    bool OnMouseDown( CPointerEventData& pointer );
    bool OnMouseMoved( CPointerEventData& pointer );
    bool OnMouseReleased( bool completed );
    bool OnMouseClicked( CPointerEventData& pointer );
    bool OnKeyPressed( CKeyEventData& key );
    bool OnCellKey( CKeyEventData& key );
    bool OnWheel( CWheelEventData& wheel );
    bool OnHeaderClicked( const THit& hit, CPointerEventData& pointer );

    /** Makes the cache of the string list and the layout again when something they depend on changed */
    void EnsureRows( void ) const;
    void EnsureLayout( void ) const;
    void InvalidateLayout( void );

    TInt32Vector ComputeBaseWidths( Int32 available, UInt32& columnCount ) const;
    Int32 MeasureRowHeaderWidth( void ) const;
    CString GetRowText( Int32 row ) const;

    /** Cell mode */
    Int32 PositionOfColumn( Int32 column ) const;
    Int32 ColumnOfPosition( Int32 position ) const;
    void SetCursorCell( Int32 row, Int32 column );
    void MoveSelectionTo( Int32 row, Int32 position, bool extend, bool add );
    void SyncCellSelection( bool byUser );
    void ApplySelectionText( const CString& text );
    bool FindDataEdge( Int32 row, Int32 position, Int32 stepRow, Int32 stepPosition, Int32& edgeRow, Int32& edgePosition ) const;
    bool StepThroughSelection( bool backwards, bool vertical );

    /** SelectionChangedEvent now, or once at the end of the update */
    void RaiseSelectionChanged( void );

    bool OnEditKey( CKeyEventData& key );

    /** Remembers what an edit operation did and raises the event for it */
    void ReportEdit( const CORE::CEvent& eventId, const CGridEditReport& report );
    CGridEditController* EditController( void );
    CGridClipboard* Clipboard( void );

    /** Keeps the selection with the rows of a model that changed */
    void CaptureKeys( void );
    void RestoreKeys( void );
    void ClampToModel( void );

    void FinishColumnChange( UInt32 column, const CORE::CEvent& eventId, Int32 value );
    void WriteColumnOrder( const TInt32Vector& order );

    CGridView( const CGridView& src );
    CGridView& operator=( const CGridView& src );

    private:

    mutable TTableViewModelPtr m_model;               /**< the model reads and fetches in lazy ways: using it from a const member is not a change of the grid */

    // the string list, split once per change
    mutable TStringVector m_rows;
    mutable UInt32 m_rowCellCount;                  /**< the most cells of a row */
    mutable bool m_rowsDirty;

    // the layout, made again per change
    mutable bool m_layoutDirty;
    mutable Int32 m_layoutWidth;
    mutable Int32 m_layoutHeight;
    mutable CString m_layoutFont;
    mutable Int32 m_layoutRowCount;
    mutable CGridColumnLayout m_layout;
    mutable TInt32Vector m_baseWidths;
    mutable TStringVector m_titles;
    mutable TStringVector m_alignments;
    mutable UInt32 m_columnCount;
    mutable Int32 m_rowHeaderWidth;

    // the cell mode
    CGridSelection m_selection;
    CGridSelection m_notifiedSelection;             /**< the selection the observers were last told about */
    Int32 m_cursorRow;
    Int32 m_cursorColumn;                           /**< a column of the data */
    Int32 m_notifiedCursorRow;
    Int32 m_notifiedCursorColumn;
    SKeyedSelection m_keys;
    bool m_syncing;                                 /**< the grid sets its own state properties: their change is not a change by somebody else */

    // a mouse press that is held
    EDragMode m_dragMode;
    Int32 m_dragColumn;
    Int32 m_dragStartX;
    Int32 m_dragStartWidth;
    Int32 m_dropPosition;
    bool m_suppressClick;

    // updates
    UInt32 m_updateDepth;
    bool m_selectionNotifyPending;
    bool m_modelChangedInUpdate;
    bool m_modelBatchOpen;

    // editing and clipboard, made when first needed
    CGridEditController* m_editing;
    CGridClipboard* m_clipboard;
    CGridEditReport m_lastReport;
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
