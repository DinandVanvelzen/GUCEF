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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CTABLECELLPARSER_H
#include "gucefGUI_CTableCellParser.h"
#define GUCEF_GUI_CTABLECELLPARSER_H
#endif /* GUCEF_GUI_CTABLECELLPARSER_H ? */

#include "gucefGUI_CGridEditController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The height of the line that tells why a value was refused */
const Int32 MessagePadding = 6;

Int32
Clamp( Int32 value, Int32 low, Int32 high )
{
    return value < low ? low : ( value > high ? high : value );
}

bool
Inside( const CRect& rect, Int32 x, Int32 y )
{
    return x >= rect.left && x < rect.left + rect.width && y >= rect.top && y < rect.top + rect.height;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridEditController::CGridEditController( CGridView& grid )
    : m_grid( grid )
    , m_editor()
    , m_rowKey()
    , m_hasRowKey( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridEditController::~CGridEditController()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::IsEditing( void ) const
{GUCEF_TRACE;

    return m_editor.IsOpen();
}

/*-------------------------------------------------------------------------*/

const CGridCellEditor&
CGridEditController::GetEditor( void ) const
{GUCEF_TRACE;

    return m_editor;
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::Redraw( void )
{GUCEF_TRACE;

    m_grid.Touch( EWidgetProperty_Rows );
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::Report( const CORE::CEvent& eventId, CGridEditReport::EOperation operation, Int32 row, Int32 column, const CString& message )
{GUCEF_TRACE;

    CGridEditReport report( operation );
    report.row = row;
    report.column = column;
    report.message = message;
    if ( CGridView::CellEditCommittedEvent == eventId )
        report.cellsApplied = 1;
    if ( CGridView::CellEditRefusedEvent == eventId )
    {
        report.refused = true;
        report.cellsRefused = 1;
        report.problemRow = row;
        report.problemColumn = column;
    }
    m_grid.ReportEdit( eventId, report );
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::IsEditableCell( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    return m_grid.IsCellEditable( row, column );
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::IsToggleCell( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    if ( m_grid.m_model.IsNULL() || row < 0 || column < 0 )
        return false;
    const CTableViewColumn& definition = m_grid.m_model->GetColumn( (UInt32) column );
    return CORE::DATATABLE_TYPE_BOOL == definition.type && definition.choices.empty();
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::Open( Int32 row, Int32 column, EOpenMode mode, const CString& typed, bool report )
{GUCEF_TRACE;

    if ( m_editor.IsOpen() && !Commit() )
        return false;
    if ( m_grid.m_model.IsNULL() || !m_grid.IsEditingEnabled() )
        return false;
    if ( row < 0 || column < 0 || row >= (Int32) m_grid.GetRowCount() || m_grid.IsColumnHidden( (UInt32) column ) )
        return false;
    CITableViewModel& model = *m_grid.m_model;
    if ( !m_grid.IsCellEditable( row, column ) )
    {
        if ( report )
            Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_EDIT, row, column, "The cell is read only" );
        return false;
    }
    const CTableViewColumn& definition = model.GetColumn( (UInt32) column );
    if ( !CTableCellParser::IsEditableType( definition ) )
    {
        if ( report )
            Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_EDIT, row, column, "This kind of value cannot be edited" );
        return false;
    }
    if ( IsToggleCell( row, column ) )
        return false;

    CGridCellEditor::EKind kind = CGridCellEditor::KIND_TEXT;
    if ( !definition.choices.empty() )
        kind = CGridCellEditor::KIND_CHOICE;
    else
    if ( definition.IsNumeric() )
        kind = CGridCellEditor::KIND_NUMBER;
    else
    if ( definition.IsTemporal() )
        kind = CGridCellEditor::KIND_DATE;

    CORE::CVariant value;
    CString text;
    if ( model.GetCellValue( (UInt32) row, (UInt32) column, value ) )
        text = CTableCellParser::FormatEditText( value, definition );

    m_grid.ScrollToCell( row, column );
    if ( OPEN_TYPED == mode && CGridCellEditor::KIND_CHOICE != kind )
        text.Clear();
    m_editor.Open( kind, row, column, text, false );
    if ( CGridCellEditor::KIND_NUMBER == kind )
        m_editor.SetExtraCharacters( definition.prefix + definition.suffix );
    if ( CGridCellEditor::KIND_CHOICE == kind )
        m_editor.SetChoices( definition.choices );
    if ( OPEN_TYPED == mode )
    {
        const bool changed = CGridCellEditor::KIND_CHOICE == kind ? m_editor.TypeAheadChoice( typed ) : m_editor.InsertText( typed );
        if ( !changed )
        {
            m_editor.Close();
            return false;
        }
    }
    m_hasRowKey = model.GetRowKey( (UInt32) row, m_rowKey );
    if ( m_editor.IsModified() )
        Revalidate();
    Report( CGridView::CellEditStartedEvent, CGridEditReport::OPERATION_EDIT, row, column, CString::Empty );
    Redraw();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::BeginEdit( Int32 row, Int32 column )
{GUCEF_TRACE;

    return Open( row, column, OPEN_VALUE, CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::BeginEditWithText( Int32 row, Int32 column, const CString& typed )
{GUCEF_TRACE;

    return Open( row, column, OPEN_TYPED, typed, false );
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::Close( void )
{GUCEF_TRACE;

    m_editor.Close();
    m_hasRowKey = false;
    m_rowKey = CORE::CVariant();
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::Revalidate( void )
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() || m_grid.m_model.IsNULL() )
        return;
    if ( !m_editor.IsModified() )
    {
        m_editor.SetError( CString::Empty );
        return;
    }
    CITableViewModel& model = *m_grid.m_model;
    const UInt32 row = (UInt32) m_editor.GetRow();
    const UInt32 column = (UInt32) m_editor.GetColumn();
    CORE::CVariant current;
    const bool haveCurrent = model.GetCellValue( row, column, current );
    CORE::CVariant value;
    CString error;
    if ( !CTableCellParser::Parse( m_editor.GetText(), model.GetColumn( column ), haveCurrent ? &current : GUCEF_NULL, value, error ) )
    {
        m_editor.SetError( error );
        return;
    }
    const CTableEditResult result = model.ValidateCellEdit( row, column, value );
    if ( result.IsAccepted() )
        m_editor.SetError( CString::Empty );
    else
        m_editor.SetError( result.message.IsNULLOrEmpty() ? CString( "The value is not accepted" ) : result.message );
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::Commit( void )
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() )
        return true;
    const Int32 row = m_editor.GetRow();
    const Int32 column = m_editor.GetColumn();
    if ( m_grid.m_model.IsNULL() )
    {
        Cancel();
        return false;
    }
    CITableViewModel& model = *m_grid.m_model;
    if ( !m_editor.IsModified() )
    {
        Close();
        Report( CGridView::CellEditCancelledEvent, CGridEditReport::OPERATION_EDIT, row, column, CString::Empty );
        Redraw();
        return true;
    }

    CORE::CVariant current;
    const bool haveCurrent = model.GetCellValue( (UInt32) row, (UInt32) column, current );
    CORE::CVariant value;
    CString error;
    if ( !CTableCellParser::Parse( m_editor.GetText(), model.GetColumn( (UInt32) column ), haveCurrent ? &current : GUCEF_NULL, value, error ) )
    {
        m_editor.SetError( error );
        Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_EDIT, row, column, error );
        Redraw();
        return false;
    }
    const CTableEditResult result = m_grid.ApplyCellEdit( row, column, value );
    if ( !result.IsAccepted() )
    {
        const CString message = result.message.IsNULLOrEmpty() ? CString( "The value is not accepted" ) : result.message;
        m_editor.SetError( message );
        Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_EDIT, row, column, message );
        Redraw();
        return false;
    }
    Close();
    Report( CGridView::CellEditCommittedEvent, CGridEditReport::OPERATION_EDIT, row, column, CString::Empty );
    Redraw();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::Cancel( void )
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() )
        return;
    const Int32 row = m_editor.GetRow();
    const Int32 column = m_editor.GetColumn();
    Close();
    Report( CGridView::CellEditCancelledEvent, CGridEditReport::OPERATION_EDIT, row, column, CString::Empty );
    Redraw();
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::ToggleCell( Int32 row, Int32 column )
{GUCEF_TRACE;

    if ( !IsToggleCell( row, column ) || m_grid.m_model.IsNULL() )
        return false;
    if ( m_editor.IsOpen() && !Commit() )
        return false;
    if ( !m_grid.IsCellEditable( row, column ) )
    {
        Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_TOGGLE, row, column, "The cell is read only" );
        return false;
    }
    CORE::CVariant current;
    bool on = true;
    if ( m_grid.m_model->GetCellValue( (UInt32) row, (UInt32) column, current ) && !current.IsNULL() && !current.IsNIL() )
        on = !current.AsBool();
    const CTableEditResult result = m_grid.ApplyCellEdit( row, column, CORE::CVariant( on ) );
    if ( !result.IsAccepted() )
    {
        Report( CGridView::CellEditRefusedEvent, CGridEditReport::OPERATION_TOGGLE, row, column, result.message );
        return false;
    }
    Report( CGridView::CellEditCommittedEvent, CGridEditReport::OPERATION_TOGGLE, row, column, CString::Empty );
    Redraw();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::UseUndo( bool undo )
{GUCEF_TRACE;

    if ( m_grid.m_model.IsNULL() )
        return false;
    CITableViewModel& model = *m_grid.m_model;
    if ( undo ? !model.CanUndo() : !model.CanRedo() )
        return false;
    CGridEditReport report( undo ? CGridEditReport::OPERATION_UNDO : CGridEditReport::OPERATION_REDO );
    const bool done = undo ? model.Undo() : model.Redo();
    if ( done )
        report.cellsApplied = 1;
    else
    {
        report.refused = true;
        report.message = undo ? "Nothing could be undone" : "Nothing could be redone";
    }
    m_grid.ReportEdit( CGridView::RangeEditResultEvent, report );
    Redraw();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnKey( CKeyEventData& key )
{GUCEF_TRACE;

    if ( m_editor.IsOpen() )
        return OnEditorKey( key );
    return OnIdleKey( key );
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnIdleKey( CKeyEventData& key )
{GUCEF_TRACE;

    const bool ctrl = 0 != ( key.modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( key.modifiers & INPUT::KEYMOD_SHIFT );
    const bool alt = 0 != ( key.modifiers & INPUT::KEYMOD_ALT );
    const bool editing = m_grid.IsEditingEnabled();
    const bool clipboard = m_grid.GetBoolProperty( EWidgetProperty_CellClipboard );
    const Int32 row = m_grid.GetCursorRow();
    const Int32 column = m_grid.GetCursorColumn();
    if ( alt )
        return false;

    switch ( key.keyCode )
    {
        case INPUT::KEYCODE_F2:
        {
            if ( !editing || ctrl || shift || row < 0 || column < 0 )
                return false;
            BeginEdit( row, column );
            return true;
        }
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
        {
            if ( !editing || ctrl || shift || row < 0 || column < 0 )
                return false;

            // a block of cells is walked through, one cell is edited
            const CGridSelection& selection = m_grid.GetSelection();
            const bool single = 1 == selection.GetRangeCount() && !selection.GetRange( 0 ).wholeRows && !selection.GetRange( 0 ).wholeColumns &&
                                selection.GetRange( 0 ).firstRow == selection.GetRange( 0 ).lastRow && selection.GetRange( 0 ).firstColumn == selection.GetRange( 0 ).lastColumn;
            if ( !single || IsToggleCell( row, column ) || !IsEditableCell( row, column ) )
                return false;
            return BeginEdit( row, column );
        }
        case INPUT::KEYCODE_SPACE:
        {
            if ( !editing || ctrl || shift || row < 0 || column < 0 || !IsToggleCell( row, column ) )
                return false;
            ToggleCell( row, column );
            return true;
        }
        case INPUT::KEYCODE_DELETE:
        {
            if ( shift && !ctrl && clipboard )
            {
                m_grid.CopySelection( true );
                return true;
            }
            if ( !editing || ctrl || shift )
                return false;
            m_grid.ClearSelectedCells();
            return true;
        }
        case INPUT::KEYCODE_Z:
        {
            if ( !editing || !ctrl )
                return false;
            return UseUndo( !shift );
        }
        case INPUT::KEYCODE_Y:
        {
            if ( !editing || !ctrl )
                return false;
            return UseUndo( false );
        }
        case INPUT::KEYCODE_C:
        {
            if ( !clipboard || !ctrl || shift )
                return false;
            m_grid.CopySelection( false );
            return true;
        }
        case INPUT::KEYCODE_X:
        {
            if ( !clipboard || !ctrl || shift )
                return false;
            m_grid.CopySelection( true );
            return true;
        }
        case INPUT::KEYCODE_V:
        {
            if ( !clipboard || !ctrl || shift )
                return false;
            m_grid.PasteFromClipboard();
            return true;
        }
        case INPUT::KEYCODE_INSERT:
        {
            if ( !clipboard )
                return false;
            if ( ctrl && !shift )
                m_grid.CopySelection( false );
            else
            if ( shift && !ctrl )
                m_grid.PasteFromClipboard();
            else
                return false;
            return true;
        }
        case INPUT::KEYCODE_D:
        case INPUT::KEYCODE_R:
        {
            if ( !clipboard || !ctrl || shift )
                return false;
            m_grid.FillSelection( INPUT::KEYCODE_D == key.keyCode );
            return true;
        }
        case INPUT::KEYCODE_ESCAPE:
        {
            if ( !clipboard || GUCEF_NULL == m_grid.GetCopyOutline() )
                return false;
            m_grid.ClearCopyOutline();
            return true;
        }
        default:
            break;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnEditorKey( CKeyEventData& key )
{GUCEF_TRACE;

    const bool ctrl = 0 != ( key.modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( key.modifiers & INPUT::KEYMOD_SHIFT );
    const bool choice = CGridCellEditor::KIND_CHOICE == m_editor.GetKind();
    bool textChanged = false;
    bool handled = true;

    switch ( key.keyCode )
    {
        case INPUT::KEYCODE_ESCAPE:
        {
            Cancel();
            return true;
        }
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
        {
            if ( Commit() )
                m_grid.StepThroughSelection( shift, true );
            return true;
        }
        case INPUT::KEYCODE_TAB:
        {
            if ( Commit() )
                m_grid.StepThroughSelection( shift, false );
            return true;
        }
        case INPUT::KEYCODE_F2:
            return true;
        case INPUT::KEYCODE_UP:
        {
            if ( choice )
                textChanged = m_editor.MoveChoice( -1 );
            break;
        }
        case INPUT::KEYCODE_DOWN:
        {
            if ( choice )
                textChanged = m_editor.MoveChoice( 1 );
            break;
        }
        case INPUT::KEYCODE_PGUP:
        {
            if ( choice )
                textChanged = m_editor.MoveChoice( -(Int32) MAX_LIST_ITEMS );
            break;
        }
        case INPUT::KEYCODE_PGDOWN:
        {
            if ( choice )
                textChanged = m_editor.MoveChoice( (Int32) MAX_LIST_ITEMS );
            break;
        }
        case INPUT::KEYCODE_HOME:
        {
            if ( choice )
                textChanged = m_editor.SetChoiceIndex( 0 );
            else
                m_editor.MoveToStart( shift );
            break;
        }
        case INPUT::KEYCODE_END:
        {
            if ( choice )
                textChanged = m_editor.SetChoiceIndex( (Int32) m_editor.GetChoices().size() - 1 );
            else
                m_editor.MoveToEnd( shift );
            break;
        }
        case INPUT::KEYCODE_LEFT:
        {
            if ( !choice )
                m_editor.MoveCaret( -1, shift );
            break;
        }
        case INPUT::KEYCODE_RIGHT:
        {
            if ( !choice )
                m_editor.MoveCaret( 1, shift );
            break;
        }
        case INPUT::KEYCODE_BACK:
        {
            if ( !choice )
                textChanged = m_editor.Backspace();
            break;
        }
        case INPUT::KEYCODE_DELETE:
        {
            if ( shift && !ctrl && !choice )
            {
                CSecretInput::WriteClipboardText( m_editor.GetSelectedText() );
                textChanged = m_editor.DeleteSelection();
            }
            else
            if ( !choice )
                textChanged = m_editor.DeleteForward();
            break;
        }
        case INPUT::KEYCODE_A:
        {
            if ( ctrl && !choice )
                m_editor.SelectAll();
            else
                handled = false;
            break;
        }
        case INPUT::KEYCODE_C:
        case INPUT::KEYCODE_X:
        {
            if ( !ctrl || choice )
            {
                handled = false;
                break;
            }
            if ( m_editor.HasSelection() )
            {
                CSecretInput::WriteClipboardText( m_editor.GetSelectedText() );
                if ( INPUT::KEYCODE_X == key.keyCode )
                    textChanged = m_editor.DeleteSelection();
            }
            break;
        }
        case INPUT::KEYCODE_V:
        case INPUT::KEYCODE_INSERT:
        {
            const bool paste = INPUT::KEYCODE_V == key.keyCode ? ctrl : shift;
            if ( !paste || choice )
            {
                handled = false;
                break;
            }
            CString pasted;
            if ( CSecretInput::ReadClipboardText( pasted ) )
            {
                // a cell holds one line: the first line of what was copied
                const char* chars = pasted.C_String();
                CString line;
                for ( ; '\0' != *chars && '\n' != *chars && '\r' != *chars; ++chars )
                    line += *chars;
                textChanged = m_editor.InsertText( line );
            }
            break;
        }
        case INPUT::KEYCODE_Z:
        case INPUT::KEYCODE_Y:
        {
            handled = ctrl;
            break;
        }
        default:
            handled = false;
            break;
    }
    if ( textChanged )
        Revalidate();
    if ( handled )
        Redraw();
    return handled;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnText( const CString& text )
{GUCEF_TRACE;

    if ( m_editor.IsOpen() )
    {
        const bool changed = CGridCellEditor::KIND_CHOICE == m_editor.GetKind() ? m_editor.TypeAheadChoice( text ) : m_editor.InsertText( text );
        if ( changed )
            Revalidate();
        Redraw();
        return true;
    }
    if ( !m_grid.IsEditingEnabled() )
        return false;
    const Int32 row = m_grid.GetCursorRow();
    const Int32 column = m_grid.GetCursorColumn();
    if ( row < 0 || column < 0 || IsToggleCell( row, column ) || !IsEditableCell( row, column ) )
        return false;
    return BeginEditWithText( row, column, text );
}

/*-------------------------------------------------------------------------*/

Int32
CGridEditController::MeasureWidth( const CString& text ) const
{GUCEF_TRACE;

    CGUIContext* context = m_grid.GetContext();
    if ( GUCEF_NULL == context || text.IsNULLOrEmpty() )
        return 0;
    Int32 width = 0, height = 0;
    context->MeasureText( m_grid.GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height );
    return width;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::GetEditorLayout( SEditorLayout& layout ) const
{GUCEF_TRACE;

    layout = SEditorLayout();
    if ( !m_editor.IsOpen() )
        return false;
    const Int32 row = m_editor.GetRow();
    const Int32 column = m_editor.GetColumn();
    m_grid.EnsureLayout();
    const Int32 position = m_grid.m_layout.FindPosition( column );
    const Int32 top = m_grid.GetRowTop( row );
    if ( position < 0 || top < 0 )
        return false;
    CRect clip;
    if ( !m_grid.GetCellRect( row, column, clip ) )
        return false;
    const Int32 rowHeight = m_grid.GetRowHeight();
    layout.visible = true;
    layout.clip = clip;
    layout.cell = CRect( m_grid.m_layout.GetScreenLeft( position ), top, m_grid.m_layout.GetWidth( position ), rowHeight );

    // the text, moved to the left far enough that the caret is in the cell
    const Int32 room = layout.cell.width - 2 * (Int32) EDITOR_PADDING;
    const Int32 caretWidth = MeasureWidth( m_editor.GetTextBefore( m_editor.GetCaret() ) );
    const Int32 scroll = caretWidth > room ? caretWidth - room : 0;
    layout.textLeft = layout.cell.left + (Int32) EDITOR_PADDING - scroll;
    layout.caretX = layout.textLeft + caretWidth;
    layout.selectionLeft = layout.selectionRight = layout.caretX;
    if ( m_editor.HasSelection() )
    {
        UInt32 first = 0, last = 0;
        m_editor.GetSelection( first, last );
        layout.selectionLeft = layout.textLeft + MeasureWidth( m_editor.GetTextBefore( first ) );
        layout.selectionRight = layout.textLeft + MeasureWidth( m_editor.GetTextBefore( last ) );
    }

    const CRect& area = m_grid.GetRect();
    if ( CGridCellEditor::KIND_CHOICE == m_editor.GetKind() )
    {
        const Int32 items = (Int32) m_editor.GetChoices().size();
        if ( items > 0 )
        {
            layout.hasList = true;
            layout.itemHeight = rowHeight;
            layout.itemCount = items < (Int32) MAX_LIST_ITEMS ? items : (Int32) MAX_LIST_ITEMS;
            const Int32 current = m_editor.GetChoiceIndex();
            Int32 first = current < 0 ? 0 : current - layout.itemCount / 2;
            layout.firstItem = Clamp( first, 0, items - layout.itemCount );
            Int32 widest = layout.cell.width;
            for ( Int32 i=0; i<items; ++i )
            {
                const Int32 width = MeasureWidth( m_editor.GetChoices()[ i ] ) + 2 * (Int32) EDITOR_PADDING;
                if ( width > widest )
                    widest = width;
            }
            if ( widest > area.width )
                widest = area.width;
            Int32 left = layout.cell.left;
            if ( left + widest > area.width )
                left = area.width - widest;
            if ( left < 0 )
                left = 0;
            const Int32 height = layout.itemCount * rowHeight;
            Int32 listTop = layout.cell.top + rowHeight;
            if ( listTop + height > area.height && layout.cell.top - height >= rowHeight )
                listTop = layout.cell.top - height;
            layout.list = CRect( left, listTop, widest, height );
        }
    }
    else
    if ( m_editor.HasError() )
    {
        layout.hasMessage = true;
        Int32 width = MeasureWidth( m_editor.GetError() ) + 2 * (Int32) MessagePadding;
        if ( width > area.width )
            width = area.width;
        Int32 left = layout.cell.left;
        if ( left + width > area.width )
            left = area.width - width;
        if ( left < 0 )
            left = 0;
        Int32 messageTop = layout.cell.top + rowHeight;
        if ( messageTop + rowHeight > area.height && layout.cell.top - rowHeight >= rowHeight )
            messageTop = layout.cell.top - rowHeight;
        layout.message = CRect( left, messageTop, width, rowHeight );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CGridEditController::GetTooltipAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() || !m_editor.HasError() )
        return CString::Empty;
    SEditorLayout layout;
    if ( GetEditorLayout( layout ) && Inside( layout.clip, localX, localY ) )
        return m_editor.GetError();
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnMouseDown( Int32 localX, Int32 localY )
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() )
        return false;
    SEditorLayout layout;
    if ( !GetEditorLayout( layout ) )
    {
        if ( !Commit() )
        {
            m_grid.m_suppressClick = true;
            return true;
        }
        return false;
    }
    if ( layout.hasList && Inside( layout.list, localX, localY ) )
    {
        const Int32 index = layout.firstItem + ( localY - layout.list.top ) / layout.itemHeight;
        m_editor.SetChoiceIndex( index );
        m_grid.m_suppressClick = true;
        Commit();
        return true;
    }
    if ( Inside( layout.clip, localX, localY ) || ( layout.hasMessage && Inside( layout.message, localX, localY ) ) )
    {
        if ( CGridCellEditor::KIND_CHOICE != m_editor.GetKind() && Inside( layout.clip, localX, localY ) )
        {
            // the caret goes to the gap between characters that is nearest to the click
            const UInt32 count = m_editor.GetCharacterCount();
            UInt32 best = 0;
            Int32 bestDistance = 0x7FFFFFFF;
            for ( UInt32 i=0; i<=count; ++i )
            {
                const Int32 x = layout.textLeft + MeasureWidth( m_editor.GetTextBefore( i ) );
                const Int32 distance = x > localX ? x - localX : localX - x;
                if ( distance < bestDistance )
                {
                    bestDistance = distance;
                    best = i;
                }
            }
            m_editor.MoveCaretTo( best, false );
            Redraw();
        }
        m_grid.m_suppressClick = true;
        return true;
    }
    if ( !Commit() )
    {
        m_grid.m_suppressClick = true;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridEditController::OnCellClicked( Int32 row, Int32 column, UInt32 clickCount, UInt32 modifiers )
{GUCEF_TRACE;

    if ( !m_grid.IsEditingEnabled() || row < 0 || column < 0 )
        return false;
    if ( 0 != ( modifiers & ( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT ) ) )
        return false;
    if ( 1 == clickCount && IsToggleCell( row, column ) && IsEditableCell( row, column ) )
        return ToggleCell( row, column );
    if ( 2 == clickCount && !IsToggleCell( row, column ) && IsEditableCell( row, column ) )
        return BeginEdit( row, column );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::OnFocusLost( void )
{GUCEF_TRACE;

    if ( m_editor.IsOpen() && !Commit() )
        Cancel();
}

/*-------------------------------------------------------------------------*/

void
CGridEditController::OnModelChanged( void )
{GUCEF_TRACE;

    if ( !m_editor.IsOpen() )
        return;
    UInt32 row = 0;
    if ( m_grid.m_model.IsNULL() || !m_hasRowKey || !m_grid.m_model->FindRowByKey( m_rowKey, row ) ||
         (UInt32) m_editor.GetColumn() >= m_grid.m_model->GetColumnCount() )
    {
        Cancel();
        return;
    }
    m_editor.SetRow( (Int32) row );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
