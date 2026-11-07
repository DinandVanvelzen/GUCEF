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

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#define GUCEF_GUI_CITABLEVIEWMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CTABLEVIEWCOLUMN_H
#include "gucefGUI_CTableViewColumn.h"
#define GUCEF_GUI_CTABLEVIEWCOLUMN_H
#endif /* GUCEF_GUI_CTABLEVIEWCOLUMN_H ? */

#ifndef GUCEF_GUI_CTABLEVIEWRANGE_H
#include "gucefGUI_CTableViewRange.h"
#define GUCEF_GUI_CTABLEVIEWRANGE_H
#endif /* GUCEF_GUI_CTABLEVIEWRANGE_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#ifndef GUCEF_GUI_CTABLEEDITRESULT_H
#include "gucefGUI_CTableEditResult.h"
#define GUCEF_GUI_CTABLEEDITRESULT_H
#endif /* GUCEF_GUI_CTABLEEDITRESULT_H ? */

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
 *  The data of a grid, held by the application instead of in the grid: rows, columns and the typed value of every
 *  cell, read when the grid needs to show the cell. A grid with a model (CGridView::SetModel) asks only for the
 *  cells it shows, so a table of millions of rows costs the grid nothing more than a table of ten. The string list
 *  mode of the grid (the Rows property) is the other way to fill a grid and stays as it is.
 *
 *  Reading: the counts, the definition of a column and the value of a cell. The value is a CVariant of the type
 *  the definition says (a null cell is a variant of type GUCEF_DATATYPE_NULL). Every member that reads is const, so
 *  that whoever holds a model through a pointer to const (CGridView::GetModel()) can read it; a model that fetches
 *  or counts lazily keeps that state in mutable members. Before the grid reads the cells it shows it announces
 *  them with PrepareRange(), which is not const because it is the call that makes a model fetch. A column span can
 *  be read in bulk for the models behind a data table.
 *
 *  What a cell shows and what its text is: GetCellContent() is what the grid DRAWS (text, the distinct empty cell of
 *  a null, or a check box), GetCellText() is the text of the cell for everything that handles it as text (copying,
 *  finding, tooltips). The default of GetCellContent() formats the value by the definition of its column
 *  (CTableCellFormatter), and the default of GetCellText() is the text of GetCellContent(), so a model that gives
 *  values and column definitions only is drawn, copied and found by the same text. A model whose display rules
 *  cannot be said by a column definition (a minus before the currency sign, a plus for a gain, a mark per cell)
 *  overrides GetCellContent() and keeps GetCellValue() typed: the grid then draws and copies its text and still has
 *  the typed value for sorting, finding the edge of a run of data and editing. The footer has the same pair:
 *  GetFooterContent() and GetFooterText().
 *
 *  Identity: a row has a key that stays with the row when rows are inserted, removed or sorted. The grid keeps its
 *  cursor and selected rows by key, so they stay with the row when the model re-orders. Without a key of its own
 *  the row index is the key.
 *
 *  Row styles: IsRowEmphasized() lets the model say which rows stand out; the grid asks it for the rows in view
 *  when it paints, so the answer follows the rows through every change, re-order and filter, with no list of row
 *  places to keep up to date.
 *
 *  Changes are announced by events, with the range that changed, so the grid redraws what it has to and keeps the
 *  rest: CellsChangedEvent, RowsInsertedEvent, RowsRemovedEvent, ColumnsChangedEvent, FooterChangedEvent and
 *  ModelResetEvent. A model raises them after its state has changed, on the thread it is used from. Models are
 *  used from one thread. A model that applies many changes at once (a refill: remove the old rows, insert the new)
 *  brackets them with BeginBatch() and EndBatch(): the grid then keeps its selection by row key, and tells its
 *  observers about the selection once, when the batch ends.
 *
 *  Editing is optional and off for every model that does not override the members of it: a cell is read only
 *  (IsCellReadOnly() is the read only flag of its column, true by default), ValidateCellEdit() refuses and
 *  SetCellValue() says read only. The grid only reports an edit and asks the model to apply it; the model validates,
 *  stores and raises CellsChangedEvent, and it owns undo and redo (CanUndo(), Undo(), CanRedo(), Redo(); the helper
 *  CTableEditHistory keeps the steps). Several edits that belong together (a paste, a fill) are announced with
 *  BeginEditGroup() and EndEditGroup(), so that a model can make them one step of its history.
 */
class GUCEF_GUI_PUBLIC_CPP CITableViewModel : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent CellsChangedEvent;        /**< TTableViewRangeEventData: the values of these cells changed; counts and keys did not */
    static const CORE::CEvent RowsInsertedEvent;        /**< TTableViewRangeEventData (firstRow, rowCount): rows were inserted; the rows from firstRow on moved down */
    static const CORE::CEvent RowsRemovedEvent;         /**< TTableViewRangeEventData (firstRow, rowCount): rows were removed; the rows after them moved up */
    static const CORE::CEvent ColumnsChangedEvent;      /**< titles, types or the number of columns changed */
    static const CORE::CEvent FooterChangedEvent;       /**< TTableViewRangeEventData (firstColumn, columnCount): the footer cells of these columns changed; the rows of the range are not used. Also raised for all columns when HasFooter() changed */
    static const CORE::CEvent ModelResetEvent;          /**< anything may have changed, rows may have been re-ordered or filtered; keys still identify the rows that remain */
    static const CORE::CEvent BatchStartedEvent;        /**< no data: many changes follow; they end with BatchFinishedEvent */
    static const CORE::CEvent BatchFinishedEvent;       /**< no data: the changes of the batch are all announced */

    static void RegisterEvents( void );

    public:

    CITableViewModel( void );

    virtual ~CITableViewModel() GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetRowCount( void ) const = 0;

    virtual UInt32 GetColumnCount( void ) const = 0;

    /** The definition of a column; for a column that is not there a default definition */
    virtual const CTableViewColumn& GetColumn( UInt32 column ) const = 0;

    /**
     *  The value of a cell. False when the model has no value for it (an index out of range): the cell is then empty,
     *  not null. A null cell gives true and a variant of type GUCEF_DATATYPE_NULL
     */
    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const = 0;

    /**
     *  What the grid draws in a cell. The default formats the value of GetCellValue() by the definition of the column
     *  (CTableCellFormatter::Format). False when the cell is not there. Called for every cell in view at every
     *  paint: keep it cheap
     */
    virtual bool GetCellContent( UInt32 row, UInt32 column, SCellContent& content ) const;

    /**
     *  The text of a cell for copying, finding and tooltips. The default is the text of GetCellContent(): what the
     *  grid draws, "true" or "false" for a check box and the null text of the column for a null
     */
    virtual CString GetCellText( UInt32 row, UInt32 column ) const;

    /**
     *  Announces the cells the grid is about to read, the rows and columns it shows. A model that fetches in pages
     *  reads them here in one go. The default does nothing
     */
    virtual void PrepareRange( const CTableViewRange& visible );

    /**
     *  The values of 'rowCount' rows of one column from 'firstRow', for the models behind a data table that hold
     *  a column as a typed span. The default reads the cells one by one. False when the column is out of range
     */
    virtual bool GetColumnSpan( UInt32 column, UInt32 firstRow, UInt32 rowCount, CORE::CVariant::VariantVector& values ) const;

    /** The key of a row, which stays with the row; the default is the row index as a UInt32 variant. False for a row that is not there */
    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const;

    /** The row that has the key now; the default reads the key as a row index. False when no row has it (it was removed or filtered out) */
    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const;

    /** The text of the row header; false for the model that has none, the grid then numbers the rows from 1 */
    virtual bool GetRowTitle( UInt32 row, CString& title ) const;

    /** Whether the row stands out (the colour of EmphasisColour, a bar at its left): asked for the rows in view at every paint. Default false */
    virtual bool IsRowEmphasized( UInt32 row ) const;

    /**
     *  Whether the model has a footer row (totals). The grid shows it when ShowFooter is not set on the grid, or is
     *  true: a ShowFooter that is set explicitly (also to false) wins over the model. When this answer changes the
     *  model raises FooterChangedEvent
     */
    virtual bool HasFooter( void ) const;

    /** The typed value of a footer cell; false when the column has none */
    virtual bool GetFooterValue( UInt32 column, CORE::CVariant& value ) const;

    /** What the grid draws in a footer cell; the default formats GetFooterValue() by the definition of the column. False when the column has none */
    virtual bool GetFooterContent( UInt32 column, SCellContent& content ) const;

    /** The text of a footer cell for copying and tooltips; the default is the text of GetFooterContent() */
    virtual CString GetFooterText( UInt32 column ) const;

    /**
     *  Whether the model orders its rows by a column when asked to, and the ask. A model that does raises
     *  ModelResetEvent after it did. The default does neither, and the grid reports that the sort is not supported
     */
    virtual bool IsSortSupported( void ) const;

    virtual bool SortByColumn( UInt32 column, bool ascending );

    /*
     *  Editing
     */

    /** Whether the user may not change the cell. The default is the read only flag of the column (true unless the column says otherwise) */
    virtual bool IsCellReadOnly( UInt32 row, UInt32 column ) const;

    /**
     *  Whether the model would take the value in the cell, without taking it: the grid asks while the user types (a
     *  refusal marks the editor, with the message) and for every cell of a paste. The default is read only for a cell
     *  that is read only, else accepted
     */
    virtual CTableEditResult ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const;

    /**
     *  Applies the edit: the value is of the logical type of the column (or null). The model stores it, raises
     *  CellsChangedEvent for the cell and records the edit in its history. The default says read only
     */
    virtual CTableEditResult SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value );

    /** The edits between these two calls belong together: a paste or a fill is one step of the history. Nested calls count. The defaults do nothing */
    virtual void BeginEditGroup( void );

    virtual void EndEditGroup( void );

    /** Undo and redo are a service of the model; the grid routes Ctrl+Z and Ctrl+Y to them when CanUndo() and CanRedo() say so. The defaults say no */
    virtual bool CanUndo( void ) const;

    virtual bool CanRedo( void ) const;

    virtual bool Undo( void );

    virtual bool Redo( void );

    /** Whether a batch of changes is open (BeginBatch() without its EndBatch()) */
    bool IsInBatch( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Opens a batch of changes: BatchStartedEvent at the outermost call. Calls nest */
    void BeginBatch( void );

    /** Closes a batch: BatchFinishedEvent at the outermost call */
    void EndBatch( void );

    private:

    CITableViewModel( const CITableViewModel& src );
    CITableViewModel& operator=( const CITableViewModel& src );

    UInt32 m_batchDepth;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CITableViewModel, MT::CMutex > TTableViewModelPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */
