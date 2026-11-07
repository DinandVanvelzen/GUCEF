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
#ifndef GUCEF_GUI_CGRIDEDITCONTROLLER_H
#define GUCEF_GUI_CGRIDEDITCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGRIDCELLEDITOR_H
#include "gucefGUI_CGridCellEditor.h"
#define GUCEF_GUI_CGRIDCELLEDITOR_H
#endif /* GUCEF_GUI_CGRIDCELLEDITOR_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

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
 *  The editing of the cells of a grid (CGridView, property CellEditing): opens the editor of a cell by the type of its
 *  column, takes the keys, the text and the clicks while it is open, and commits or cancels. The grid owns one when
 *  it first needs it. The controller only reports an edit and asks the model to apply it (CGridView::ApplyCellEdit):
 *  the text is read into a value of the type of the column (CTableCellParser), the model validates it while the user
 *  types (a refusal marks the editor and gives the reason) and again when the value is committed.
 *
 *  Keys while an editor is open: Enter commits and moves to the cell below (Shift: above), Tab commits and moves to the
 *  next cell (Shift: back), Escape cancels; the text editor takes the usual caret keys, Backspace, Delete, Ctrl+A and
 *  Ctrl+C, Ctrl+X, Ctrl+V on its own text; the list of choices takes Up, Down, Page Up, Page Down, Home, End and letters.
 *  A commit that the model refuses keeps the editor open and raises CellEditRefusedEvent.
 *
 *  Keys with no editor open (cell mode): F2 opens the editor of the cursor cell, Enter does it when one cell is selected,
 *  Space toggles a check box, Delete clears the selected cells, Ctrl+Z and Ctrl+Y ask the model to undo and redo, and
 *  when CellClipboard is set Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+D, Ctrl+R and Escape (which ends the outline of a copy).
 */
class GUCEF_GUI_PUBLIC_CPP CGridEditController
{
    public:

    /** Where the parts of the open editor stand, in the coordinates of the grid; the painter draws them, the mouse is placed by them */
    struct SEditorLayout
    {
        bool visible;               /**< the cell of the editor is in view */
        CRect cell;                 /**< the cell, not cut by the area it scrolls in */
        CRect clip;                 /**< the part of the cell that is in view */
        Int32 textLeft;             /**< where the first character stands, moved so that the caret is in the cell */
        Int32 caretX;
        Int32 selectionLeft;
        Int32 selectionRight;       /**< equal to selectionLeft when nothing is selected */
        bool hasList;               /**< a list of choices is open */
        CRect list;
        Int32 itemHeight;
        Int32 firstItem;            /**< the first choice shown */
        Int32 itemCount;            /**< the choices shown */
        bool hasMessage;            /**< a refusal is shown */
        CRect message;

        SEditorLayout( void ) : visible( false ), cell(), clip(), textLeft( 0 ), caretX( 0 ), selectionLeft( 0 ), selectionRight( 0 )
                              , hasList( false ), list(), itemHeight( 0 ), firstItem( 0 ), itemCount( 0 ), hasMessage( false ), message() {}
    };

    /** The room between the border of the editor and its text */
    enum ELayout
    {
        EDITOR_PADDING = 4,
        MAX_LIST_ITEMS = 8
    };

    explicit CGridEditController( CGridView& grid );

    ~CGridEditController();

    bool IsEditing( void ) const;

    const CGridCellEditor& GetEditor( void ) const;

    /** Opens the editor on the cell with its value as text (false for a cell that cannot be edited, or a check box) */
    bool BeginEdit( Int32 row, Int32 column );

    /** Opens the editor with the typed text instead of the value: what typing on a cell does */
    bool BeginEditWithText( Int32 row, Int32 column, const CString& typed );

    /** Gives the value to the model and closes; false (and the editor stays) when the text is no value of the column or the model refuses it */
    bool Commit( void );

    void Cancel( void );

    /** Flips a check box through the model */
    bool ToggleCell( Int32 row, Int32 column );

    /** Whether a value of the column is shown as a check box that a click flips */
    bool IsToggleCell( Int32 row, Int32 column ) const;

    /*
     *  Input; true when the controller used it
     */

    bool OnKey( CKeyEventData& key );

    bool OnText( const CString& text );

    /** A press: with an editor open it places the caret, picks a choice, or commits when it is outside the editor */
    bool OnMouseDown( Int32 localX, Int32 localY );

    /** A click on a cell: a check box is flipped, a double click opens the editor; the grid asks after it raised its own events */
    bool OnCellClicked( Int32 row, Int32 column, UInt32 clickCount, UInt32 modifiers );

    /** The grid lost the keyboard focus: the value is committed, or the editor closes when the model refuses it */
    void OnFocusLost( void );

    /** The model changed rows or columns: the editor follows its row by key, or closes when the row is gone */
    void OnModelChanged( void );

    bool GetEditorLayout( SEditorLayout& layout ) const;

    /** The reason of a refusal when the point is on the cell of the editor */
    CString GetTooltipAt( Int32 localX, Int32 localY ) const;

    private:

    enum EOpenMode
    {
        OPEN_VALUE = 0  ,
        OPEN_TYPED
    };

    bool Open( Int32 row, Int32 column, EOpenMode mode, const CString& typed, bool report );
    void Close( void );
    bool IsEditableCell( Int32 row, Int32 column ) const;
    void Revalidate( void );
    void Report( const CORE::CEvent& eventId, CGridEditReport::EOperation operation, Int32 row, Int32 column, const CString& message );
    bool OnEditorKey( CKeyEventData& key );
    bool OnIdleKey( CKeyEventData& key );
    bool UseUndo( bool undo );
    Int32 MeasureWidth( const CString& text ) const;
    void Redraw( void );

    CGridEditController( const CGridEditController& src );
    CGridEditController& operator=( const CGridEditController& src );

    CGridView& m_grid;
    CGridCellEditor m_editor;
    CORE::CVariant m_rowKey;
    bool m_hasRowKey;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDEDITCONTROLLER_H ? */
