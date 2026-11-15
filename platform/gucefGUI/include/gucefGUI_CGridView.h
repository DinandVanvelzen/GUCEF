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

#ifndef GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#include "gucefGUI_CIGridPreferenceStore.h"
#define GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#endif /* GUCEF_GUI_CIGRIDPREFERENCESTORE_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#include "gucefGUI_CGridCsvExportReport.h"
#define GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTREPORT_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNFIT_H
#include "gucefGUI_CGridColumnFit.h"
#define GUCEF_GUI_CGRIDCOLUMNFIT_H
#endif /* GUCEF_GUI_CGRIDCOLUMNFIT_H ? */

#ifndef GUCEF_GUI_CCELLSTATUSCOLOURS_H
#include "gucefGUI_CCellStatusColours.h"
#define GUCEF_GUI_CCELLSTATUSCOLOURS_H
#endif /* GUCEF_GUI_CCELLSTATUSCOLOURS_H ? */

#ifndef GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H
#include "gucefGUI_CIGridContextMenuProvider.h"
#define GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H
#endif /* GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H ? */

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
class CGridColumnChooserForm;
class CGridCsvExportController;
class CGridCsvOptions;
class CIGridHelpProvider;
class CPopupMenu;

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
 *  Columns and export (ShowColumnChooser, ShowCsvExport): buttons at the top right of the grid, in a strip at its
 *  right edge that the columns leave free. The column chooser lists every column the grid has (also the hidden ones, and
 *  the columns of the model that are hidden by default) to show, hide and order them and to reset them to the default;
 *  required columns cannot be hidden. The CSV export saves the table as it is shown (the shown columns in their order,
 *  the rows after filter and sort, the text of the cells) to a file chosen in the file save dialog. With a preference
 *  store (SetPreferenceStore) the choices of the user are kept per grid id and applied again the next time.
 *
 *  Fit policy (FitMode Fill or Content): every column gets its nominal or content width within the bounds of its
 *  CGridColumnFit, numbers never below their content; Fill shares the room left among the growing columns, Content
 *  leaves it free; when the columns do not fit they shrink by priority and the grid scrolls sideways. In every mode a
 *  number that does not fit its cell is drawn compact or as "###", never cut, and text is never cut on the left.
 *
 *  Cell statuses and markers (SCellContent::status, marker): the text of a cell in the colour of its status
 *  (StatusColours), with a marker in a fixed slot (CTableViewColumn::markerSlot) or before the value. EmphasisStyle Bar
 *  marks a row that stands out with the bar only; SelectionOutline outlines the selected rows.
 *
 *  Context menu (SetContextMenuProvider): a right click, the menu key or Shift+F10 opens the items of the provider
 *  for the cell. Rows that fold open: a model with a row hierarchy gets an indent and a fold control in the first
 *  column; a click on it, Right / Left (row selection) and numpad + / - open and close the row through the model.
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
    static const CORE::CEvent ColumnsResetEvent;                /**< no data: the order, the shown columns and the widths went back to the defaults (ResetColumnsToDefault) */
    static const CORE::CEvent PreferencesSaveFailedEvent;       /**< no data: the preference store did not keep the choices of the user (the log says why); the grid works on */
    static const CORE::CEvent CsvExportFinishedEvent;           /**< TGridCsvExportReportEventData: a CSV export that was started ended, written or not */
    static const CORE::CEvent CsvExportCancelledEvent;          /**< no data: the user cancelled the file save dialog of a CSV export; nothing was written */
    static const CORE::CEvent ContextMenuItemChosenEvent;       /**< TContextMenuChoiceEventData: the user chose an item of the context menu (after the provider was told) */
    static const CORE::CEvent RowExpansionChangedEvent;         /**< TCloneableGridIndex (the row, column 0): the model opened or closed a row that folds open */

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
    typedef CORE::CTCloneableObj< CGridCsvExportReport > TGridCsvExportReportEventData;

    /** A change of a column: which column of the data, and a width, a place or a flag by the event */
    struct SColumnChange
    {
        UInt32 column;
        Int32 value;
    };
    typedef struct SColumnChange TColumnChange;
    typedef CORE::CTCloneableObj< TColumnChange > TColumnChangedEventData;

    /** A choice in the context menu: the cell the menu was opened for (-1 outside a cell) and the id of the item */
    struct SContextMenuChoice
    {
        Int32 row;
        Int32 column;
        CString itemId;
    };
    typedef struct SContextMenuChoice TContextMenuChoice;
    typedef CORE::CTCloneableObj< TContextMenuChoice > TContextMenuChoiceEventData;

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
        HIT_FOOTER          ,
        HIT_COLUMN_CHOOSER_BUTTON   ,   /**< the column chooser button of the header strip */
        HIT_CSV_EXPORT_BUTTON       ,   /**< the CSV export button of the header strip */
        HIT_FOLD_BUTTON                 /**< the fold control of a row that folds open (a model with a row hierarchy) */
    };

    /** The buttons of the header strip */
    enum EHeaderButton
    {
        HEADERBUTTON_COLUMN_CHOOSER = 0 ,
        HEADERBUTTON_CSV_EXPORT
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

    /** The indent per level of a row of a hierarchy, and the room of the fold control in front of the text of its first column */
    enum EFoldLayout
    {
        FOLD_INDENT = 16,
        FOLD_BUTTON_ROOM = 16
    };

    /** The rows from the top whose cells the content width of a column is measured over, besides the rows in view */
    enum EContentSample
    {
        CONTENT_SAMPLE_ROWS = 100
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

    /** The colour ItemColours gives the row (a marker at its left and its text in that colour); false for a plain row */
    bool GetRowColour( Int32 row, CColour& colour ) const;

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

    /**
     *  Over the cell of an editor that holds a refused value: the reason; over a cell whose text is cut: the whole text; over a column
     *  title or a cell with a help topic: nothing (the tooltip service shows the brief of the topic); else the Tooltip property
     */
    virtual CString GetTooltipAt( Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  Help topics of the parts of the grid
     */

    /** Where the topics of the columns and cells come from, asked when needed; not owned, it must outlive the grid or be set to NULL first */
    void SetHelpProvider( CIGridHelpProvider* provider );

    CIGridHelpProvider* GetHelpProvider( void ) const;

    /** The topic of the title of a column of the data: of the provider, else the helpTopic of the model column; empty for none */
    CString GetColumnHelpTopic( UInt32 column ) const;

    /** The topic of a cell (row in the order of the grid, column of the data): of the provider; empty for none */
    CString GetCellHelpTopic( UInt32 row, UInt32 column ) const;

    /** Over a column title: the topic of the column; over a cell: the topic of the cell; when the part has none, the HelpTopic property */
    virtual CString GetHelpTopicAt( Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

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

    /**
     *  Hides or shows a column (HiddenColumns). A required column is not hidden and neither is the last column that is
     *  shown: false then. True when the column is as asked afterwards. Raises ColumnVisibilityChangedEvent when it changed
     */
    bool SetColumnHidden( UInt32 column, bool hidden );

    bool IsColumnHidden( UInt32 column ) const;

    /*
     *  Columns: keys, defaults, required columns, the order of all columns
     */

    /**
     *  The key of a column, which the preferences name it by: the name of the column of the model, else its title (the
     *  title of Columns without a model), else "#" and its number from 1; a key that an earlier column has already gets
     *  "#2", "#3" ... appended
     */
    CString GetColumnKey( UInt32 column ) const;

    /** The keys of all columns, by column of the data */
    TStringVector GetColumnKeys( void ) const;

    /** The column with the key, -1 when none has it */
    Int32 FindColumnByKey( const CString& key ) const;

    /** Whether the column is always shown: required by the model's column definition, or named by SetRequiredColumns() */
    bool IsColumnRequired( UInt32 column ) const;

    /** Columns of the data that are always shown, besides the ones the model says are required (for a grid without a model) */
    void SetRequiredColumns( const TInt32Vector& columns );

    /** Whether the column is not shown by default: the defaults recorded by CaptureColumnDefaults(), else hiddenByDefault of the model's column */
    bool IsColumnHiddenByDefault( UInt32 column ) const;

    /** Whether the column may be hidden: it is not required and is not the last column shown (a hidden column: true) */
    bool CanHideColumn( UInt32 column ) const;

    /** Every column of the data in the order they stand, hidden columns at their place */
    TInt32Vector GetColumnOrder( void ) const;

    /** Puts a column at a place in the order of all columns (GetColumnOrder()), hidden or not. Raises ColumnMovedEvent with its place among the shown columns (-1 for a hidden one). False when nothing changed */
    bool MoveColumnInOrder( UInt32 column, UInt32 index );

    /**
     *  Records what the application set up as the default of the columns (ColumnOrder, HiddenColumns, ColumnWidths,
     *  each with whether it was set). Done by SetPreferenceStore() and at the first opening of the column chooser when
     *  not done before, so an application sets its columns up before either
     */
    void CaptureColumnDefaults( void );

    bool HasColumnDefaults( void ) const;

    /** Back to the recorded defaults (the defaults of the model for what was not set): order, shown columns, widths; the sort stays. Raises ColumnsResetEvent */
    void ResetColumnsToDefault( void );

    /*
     *  Preferences
     */

    /**
     *  Keeps the choices of the user for the columns in the store under the id (stable across runs, unique per grid with
     *  columns of its own): records the defaults, loads the record of the id and applies it (now, and again when the
     *  model or its columns change), and from then on saves every change of the order, the shown columns, the widths and
     *  the sort made through the members of the grid, the mouse or the column chooser (a resize by dragging once, when
     *  it ends). A NULL store stops it
     */
    void SetPreferenceStore( const TGridPreferenceStorePtr& store, const CString& gridId );

    const TGridPreferenceStorePtr& GetPreferenceStore( void ) const;

    const CString& GetPreferenceId( void ) const;

    /**
     *  The revision of the default columns the application gives the grid, written into every record the grid saves
     *  (CGridPreferences::defaultsRevision): an application that changes its defaults tells the records made against
     *  the earlier ones from those made against the current ones. Empty by default
     */
    void SetDefaultsRevision( const CString& revision );

    const CString& GetDefaultsRevision( void ) const;

    /** Loads the record of the id again and applies it; false when the store has none */
    bool LoadPreferences( void );

    /** Saves the state of the columns now; false without a store or when the store refused it (PreferencesSaveFailedEvent) */
    bool SavePreferences( void );

    /** The state of the columns as a record (the export location of the record last loaded or saved is kept) */
    void CapturePreferences( CGridPreferences& preferences ) const;

    /** Applies a record: keys the grid does not have are ignored, columns the record does not name keep their defaults, required columns stay shown; false when the grid has no columns yet */
    bool ApplyPreferences( const CGridPreferences& preferences );

    /** The record last loaded, saved or applied */
    const CGridPreferences& GetPreferences( void ) const;

    /** Where the last CSV export of this grid went; kept in the record, saved with a store */
    void SetExportLocation( const CString& rootName, const CString& directory );

    /*
     *  Header strip (ShowColumnChooser, ShowCsvExport)
     */

    /** The width of the strip at the right edge that the buttons stand in: a header row height per button shown, 0 without buttons */
    Int32 GetHeaderStripWidth( void ) const;

    /** The rectangle of a button in the grid, false when it is not shown */
    bool GetHeaderButtonRect( EHeaderButton button, CRect& rect ) const;

    /** The button below the pointer, -1 for none */
    Int32 GetHoveredHeaderButton( void ) const;

    /** Opens the column chooser of the grid (what its button does); false without a context */
    bool OpenColumnChooser( void );

    /** The column chooser, NULL until it was opened once */
    CGridColumnChooserForm* GetColumnChooser( void );

    bool IsColumnChooserOpen( void ) const;

    /** Starts a CSV export (what its button does): the file save dialog, then the file; false when it cannot start */
    bool OpenCsvExport( void );

    /** The CSV export of the grid: options, targets, state */
    CGridCsvExportController& GetCsvExportController( void );

    /** The options of the CSV export of the grid */
    CGridCsvOptions& GetCsvExportOptions( void );

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

    /*
     *  Fit policy (FitMode Fill and Content), overflow of cells
     */

    /** The fit policy of a column of the data for this grid, over the one of the model column (for a grid without a model) */
    void SetColumnFit( UInt32 column, const CGridColumnFit& fit );

    void ClearColumnFit( UInt32 column );

    /** The fit policy of a column: set on the grid, else the fit of the model column, else the default */
    CGridColumnFit GetColumnFit( UInt32 column ) const;

    /** The width the content of a column needs, padding included: the widest of its title, its footer and the cells measured (the first CONTENT_SAMPLE_ROWS rows and the rows in view); 0 without a context */
    Int32 GetColumnContentWidth( UInt32 column ) const;

    /** Measures the content of the columns afresh at the next layout (the measured widths otherwise only grow while the data change) */
    void ResetContentWidths( void );

    /** Whether the columns scroll sideways when they are wider than the grid: HorizontalScroll, or a FitMode of Fill or Content */
    bool IsHorizontallyScrollable( void ) const;

    /** The pixels of a column kept for the trailing markers of its cells (CTableViewColumn::markerSlot); 0 without a model */
    Int32 GetColumnMarkerSlot( UInt32 column ) const;

    /**
     *  The text drawn for a content in a room of 'room' pixels, and whether it is not the whole text: the text when it
     *  fits; a number: its compact form (the compactText of the content, else CTableCellFormatter::FormatCompact() of
     *  the value of the cell, row -1 for the footer) when that fits, else '#' characters; other text: cut with "..." when
     *  TextOverflow is Ellipsis or for a footer cell, else the whole text (it is clipped at the end of its cell)
     */
    CString FitContentText( const SCellContent& content, Int32 room, Int32 row, Int32 column, bool footer, bool& cut ) const;

    /** Whether the text of the footer cell of a column is wider than its room (the tooltip then shows the whole text) */
    bool IsFooterTextCut( UInt32 column ) const;

    /*
     *  Cell statuses
     */

    /** The colours of the statuses of cells: the defaults with the StatusColours property over them */
    const CCellStatusColours& GetStatusColours( void ) const;

    /*
     *  Context menu
     */

    /** Where the items of the context menu come from; not owned, it must outlive the grid or be set to NULL first. NULL: no menu, a right click does what it did */
    void SetContextMenuProvider( CIGridContextMenuProvider* provider );

    CIGridContextMenuProvider* GetContextMenuProvider( void ) const;

    /** Opens the context menu of a cell (row and column of the data, -1 outside a cell) at a point of the context; false without a provider, a context or items */
    bool OpenContextMenu( Int32 row, Int32 column, Int32 x, Int32 y );

    bool IsContextMenuOpen( void ) const;

    void CloseContextMenu( void );

    /** Chooses the item with the id in the menu that is open, as a click on it does; false when no menu is open or the item cannot be chosen */
    bool ChooseContextMenuItem( const CString& itemId );

    /** The items of the menu that was opened last */
    const TContextMenuItemVector& GetContextMenuItems( void ) const;

    /** The cell the menu that was opened last is for */
    Int32 GetContextMenuRow( void ) const;

    Int32 GetContextMenuColumn( void ) const;

    /*
     *  Rows that fold open (a model with a row hierarchy)
     */

    bool HasRowHierarchy( void ) const;

    UInt32 GetRowLevel( Int32 row ) const;

    bool IsRowExpandable( Int32 row ) const;

    bool IsRowExpanded( Int32 row ) const;

    /** Asks the model to open or close the row; raises RowExpansionChangedEvent when the model took it */
    bool SetRowExpanded( Int32 row, bool expanded );

    bool ToggleRowExpanded( Int32 row );

    /** The rectangle of the fold control of a row in the grid; false when the row has none or is out of view */
    bool GetFoldControlRect( Int32 row, CRect& rect ) const;

    /** The pixels in front of the text of a cell for the indent and the fold control: for the first shown column of a grid with a row hierarchy, else 0 */
    Int32 GetFoldRoom( Int32 row, UInt32 column ) const;

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
    friend class CGridCsvExportController;      /**< raises the export events of the grid */

    typedef CORE::CTEventHandlerFunctor< CGridView > TEventCallback;

    /** The help topic of the column title or the cell at the point, empty elsewhere and for a part without one */
    CString GetPartHelpTopicAt( Int32 localX, Int32 localY ) const;

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

    /** The widths of all columns; the shown ones share the room ('hidden' are not shown: they keep a width but take no room) */
    TInt32Vector ComputeBaseWidths( Int32 available, UInt32& columnCount, const TInt32Vector& hidden ) const;
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

    /** The hidden columns the layout uses: HiddenColumns when set, else the defaults of the model; never a required column */
    TInt32Vector GetEffectiveHiddenColumns( void ) const;

    /** A block selection is of places: after a change of the shown columns it collapses to the cursor */
    void CollapseSelectionToCursor( void );

    /** Saves the preferences when a change is pending and no resize drag is in progress */
    void FlushPreferences( void );

    /** The header strip: which button an x of the header row is on, -1 for none */
    Int32 HeaderButtonAt( Int32 localX ) const;

    /** The fit policy is used: FitMode Fill or Content */
    bool IsFitByPolicy( void ) const;

    /** The widths of the columns by the fit policy, see CGridColumnFitter */
    TInt32Vector ComputePolicyWidths( Int32 available, UInt32 columns, const TInt32Vector& hidden ) const;

    /** Measures the content of every column when it changed since the last time, the widths only growing unless reset; uses no layout, so the layout can call it */
    void EnsureContentWidths( UInt32 columns ) const;

    /** The data changed: the content widths are measured again (and, with 'reset', may shrink) at the next layout of a grid that fits by policy */
    void OnContentChanged( bool reset );

    /** The first shown column, -1 when none is shown */
    Int32 GetFirstShownColumn( void ) const;

    /** Opens the context menu at the cell under a pointer or of the cursor, selecting the row first */
    bool OnContextMenuRequest( Int32 row, Int32 column, Int32 x, Int32 y );

    bool OnContextMenuKey( void );

    void OnContextMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    bool OnFoldKey( CKeyEventData& key );

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
    mutable bool m_layoutHiddenSet;                 /**< HiddenColumns was set when the layout was made */
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

    /** What the application set up for the columns, for ResetColumnsToDefault() */
    struct SColumnDefaults
    {
        bool captured;
        bool orderSet;
        TInt32Vector order;
        bool hiddenSet;
        TInt32Vector hidden;
        bool widthsSet;
        TInt32Vector widths;

        SColumnDefaults( void ) : captured( false ), orderSet( false ), order(), hiddenSet( false ), hidden(), widthsSet( false ), widths() {}
    };

    // columns, preferences and the header strip
    GUCEF::set< Int32 > m_requiredColumns;
    SColumnDefaults m_columnDefaults;
    TGridPreferenceStorePtr m_preferenceStore;
    CString m_preferenceId;
    CString m_defaultsRevision;
    CGridPreferences m_preferences;
    bool m_hasPreferences;                          /**< m_preferences holds a record that was loaded, applied or saved */
    bool m_applyingPreferences;                     /**< the grid sets the properties of the columns itself: no save */
    bool m_preferencesDirty;                        /**< a change of the columns is not saved yet */
    bool m_preferencesPending;                      /**< a record was loaded but the grid had no columns to apply it to: no save until it is applied */
    Int32 m_hoverButton;
    CGridColumnChooserForm* m_columnChooser;        /**< made when first opened, owned */
    CGridCsvExportController* m_csvExport;          /**< made when first needed, owned */
    CIGridHelpProvider* m_helpProvider;             /**< the topics of the columns and cells, not owned */

    // fit policy and statuses
    GUCEF::map< UInt32, CGridColumnFit > m_columnFits;  /**< fit policies set on the grid, by column of the data */
    mutable TInt32Vector m_contentWidths;           /**< measured content widths, by column of the data */
    mutable bool m_contentDirty;                    /**< the data changed since the content was measured */
    mutable CCellStatusColours m_statusColours;
    mutable CString m_statusColoursText;            /**< the StatusColours value m_statusColours was parsed from */
    mutable bool m_statusColoursParsed;

    // context menu
    CIGridContextMenuProvider* m_contextMenuProvider;   /**< not owned */
    CPopupMenu* m_contextMenu;                      /**< made in the context when first opened; destroyed with the grid */
    TContextMenuItemVector m_contextMenuItems;
    Int32 m_contextMenuRow;
    Int32 m_contextMenuColumn;
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
