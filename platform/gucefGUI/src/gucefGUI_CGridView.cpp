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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CGRIDEDITCONTROLLER_H
#include "gucefGUI_CGridEditController.h"
#define GUCEF_GUI_CGRIDEDITCONTROLLER_H
#endif /* GUCEF_GUI_CGRIDEDITCONTROLLER_H ? */

#ifndef GUCEF_GUI_CGRIDCLIPBOARD_H
#include "gucefGUI_CGridClipboard.h"
#define GUCEF_GUI_CGRIDCLIPBOARD_H
#endif /* GUCEF_GUI_CGRIDCLIPBOARD_H ? */

#include "gucefGUI_CGridView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGridView::GridItemClickedEvent = "GUCEF::GUI::CGridView::GridItemClickedEvent";
const CORE::CEvent CGridView::GridItemAddedEvent = "GUCEF::GUI::CGridView::GridItemAddedEvent";
const CORE::CEvent CGridView::GridItemDeletedEvent = "GUCEF::GUI::CGridView::GridItemDeletedEvent";
const CORE::CEvent CGridView::GridRowAddedEvent = "GUCEF::GUI::CGridView::GridRowAddedEvent";
const CORE::CEvent CGridView::GridRowDeletedEvent = "GUCEF::GUI::CGridView::GridRowDeletedEvent";
const CORE::CEvent CGridView::GridColumnAddedEvent = "GUCEF::GUI::CGridView::GridColumnAddedEvent";
const CORE::CEvent CGridView::GridColumnDeletedEvent = "GUCEF::GUI::CGridView::GridColumnDeletedEvent";
const CORE::CEvent CGridView::ColumnClickedEvent = "GUCEF::GUI::CGridView::ColumnClickedEvent";
const CORE::CEvent CGridView::SelectionChangedEvent = "GUCEF::GUI::CGridView::SelectionChangedEvent";
const CORE::CEvent CGridView::CellCursorChangedEvent = "GUCEF::GUI::CGridView::CellCursorChangedEvent";
const CORE::CEvent CGridView::CellSelectionChangedEvent = "GUCEF::GUI::CGridView::CellSelectionChangedEvent";
const CORE::CEvent CGridView::ColumnResizedEvent = "GUCEF::GUI::CGridView::ColumnResizedEvent";
const CORE::CEvent CGridView::ColumnMovedEvent = "GUCEF::GUI::CGridView::ColumnMovedEvent";
const CORE::CEvent CGridView::ColumnVisibilityChangedEvent = "GUCEF::GUI::CGridView::ColumnVisibilityChangedEvent";
const CORE::CEvent CGridView::CellEditStartedEvent = "GUCEF::GUI::CGridView::CellEditStartedEvent";
const CORE::CEvent CGridView::CellEditCommittedEvent = "GUCEF::GUI::CGridView::CellEditCommittedEvent";
const CORE::CEvent CGridView::CellEditCancelledEvent = "GUCEF::GUI::CGridView::CellEditCancelledEvent";
const CORE::CEvent CGridView::CellEditRefusedEvent = "GUCEF::GUI::CGridView::CellEditRefusedEvent";
const CORE::CEvent CGridView::RangeEditResultEvent = "GUCEF::GUI::CGridView::RangeEditResultEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

/** The keys of the rows a grid keeps for a model that re-orders; more rows than this are not followed */
const UInt32 MaxKeyedRows = 1000;

/** The pixels the pointer has to move with a column title pressed before the column is dragged */
const Int32 MoveThreshold = 4;

/** The pixels a notch of the wheel moves the columns */
const Int32 WheelStepX = 48;

/** The room around the number of a row in the row header */
const Int32 RowHeaderPadding = 6;

const Int32 MinRowHeaderWidth = 24;

CString
JoinCells( const GUCEF::vector< CString >& cells )
{
    CString row;
    for ( UInt32 i=0; i<(UInt32)cells.size(); ++i )
    {
        if ( i > 0 )
            row += '\t';
        row += cells[ i ];
    }
    return row;
}

/*-------------------------------------------------------------------------*/

/** Whether a cell is a number (digits with at most one '.' and an optional sign), and its value */
bool
TryReadNumber( const CString& cell, Float64& value )
{
    if ( cell.IsNULLOrEmpty() )
        return false;
    const char* text = cell.C_String();
    bool digits = false;
    bool dot = false;
    for ( UInt32 i=0; '\0' != text[ i ]; ++i )
    {
        const char c = text[ i ];
        if ( c >= '0' && c <= '9' )
            digits = true;
        else
        if ( '.' == c && !dot )
            dot = true;
        else
        if ( ( '-' == c || '+' == c ) && 0 == i )
            continue;
        else
            return false;
    }
    if ( !digits )
        return false;
    value = CORE::StringToDouble( cell );
    return true;
}

/*-------------------------------------------------------------------------*/

/** -1, 0 or 1: numbers by their value, other text without regard to case */
Int32
CompareCells( const CString& a, const CString& b )
{
    Float64 numberA = 0, numberB = 0;
    if ( TryReadNumber( a, numberA ) && TryReadNumber( b, numberB ) )
        return numberA < numberB ? -1 : ( numberA > numberB ? 1 : 0 );
    const CString lowerA = a.Lowercase();
    const CString lowerB = b.Lowercase();
    return lowerA < lowerB ? -1 : ( lowerB < lowerA ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< Int32 >
SortedUnique( const GUCEF::vector< Int32 >& rows, Int32 rowCount )
{
    GUCEF::set< Int32 > unique;
    for ( size_t i=0; i<rows.size(); ++i )
        if ( rows[ i ] >= 0 && rows[ i ] < rowCount )
            unique.insert( rows[ i ] );
    return GUCEF::vector< Int32 >( unique.begin(), unique.end() );
}

/*-------------------------------------------------------------------------*/

Int32
Clamp( Int32 value, Int32 low, Int32 high )
{
    return value < low ? low : ( value > high ? high : value );
}

/** Holds the notifications of the selection back while the properties of the selection are set one by one, so that the first one tells the whole new state */
class CUpdateScope
{
    public:

    explicit CUpdateScope( CGridView& grid ) : m_grid( grid ) { m_grid.BeginUpdate(); }

    ~CUpdateScope() { m_grid.EndUpdate(); }

    private:

    CUpdateScope( const CUpdateScope& src );
    CUpdateScope& operator=( const CUpdateScope& src );

    CGridView& m_grid;
};

}

void
CGridView::RegisterEvents( void )
{GUCEF_TRACE;

    GridItemClickedEvent.Initialize();
    GridItemAddedEvent.Initialize();
    GridItemDeletedEvent.Initialize();
    GridRowAddedEvent.Initialize();
    GridRowDeletedEvent.Initialize();
    GridColumnAddedEvent.Initialize();
    GridColumnDeletedEvent.Initialize();
    ColumnClickedEvent.Initialize();
    SelectionChangedEvent.Initialize();
    CellCursorChangedEvent.Initialize();
    CellSelectionChangedEvent.Initialize();
    ColumnResizedEvent.Initialize();
    ColumnMovedEvent.Initialize();
    ColumnVisibilityChangedEvent.Initialize();
    CellEditStartedEvent.Initialize();
    CellEditCommittedEvent.Initialize();
    CellEditCancelledEvent.Initialize();
    CellEditRefusedEvent.Initialize();
    RangeEditResultEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGridView::CGridView( void )
    : CWidget( "GridView" )
    , m_model()
    , m_rows()
    , m_rowCellCount( 0 )
    , m_rowsDirty( true )
    , m_layoutDirty( true )
    , m_layoutWidth( -1 )
    , m_layoutHeight( -1 )
    , m_layoutFont()
    , m_layoutRowCount( -1 )
    , m_layout()
    , m_baseWidths()
    , m_titles()
    , m_alignments()
    , m_columnCount( 0 )
    , m_rowHeaderWidth( 0 )
    , m_selection()
    , m_notifiedSelection()
    , m_cursorRow( -1 )
    , m_cursorColumn( -1 )
    , m_notifiedCursorRow( -1 )
    , m_notifiedCursorColumn( -1 )
    , m_keys()
    , m_syncing( false )
    , m_dragMode( DRAG_NONE )
    , m_dragColumn( -1 )
    , m_dragStartX( 0 )
    , m_dragStartWidth( 0 )
    , m_dropPosition( -1 )
    , m_suppressClick( false )
    , m_updateDepth( 0 )
    , m_selectionNotifyPending( false )
    , m_modelChangedInUpdate( false )
    , m_modelBatchOpen( false )
    , m_editing( GUCEF_NULL )
    , m_clipboard( GUCEF_NULL )
    , m_lastReport()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CGridView::~CGridView()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_editing;
    m_editing = GUCEF_NULL;
    GUCEF_DELETE m_clipboard;
    m_clipboard = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CGridView::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CGridView();
}

/*-------------------------------------------------------------------------*/

void
CGridView::GetCapabilities( GUCEF::set< CString >& capabilities )
{GUCEF_TRACE;

    capabilities.insert( "GridModel" );
    capabilities.insert( "GridTypedCells" );
    capabilities.insert( "GridCellSelection" );
    capabilities.insert( "GridLayout" );
    capabilities.insert( "GridModelContent" );
    capabilities.insert( "GridBatchUpdate" );
    capabilities.insert( "GridCellEditing" );
    capabilities.insert( "GridClipboard" );
}

/*-------------------------------------------------------------------------*/

void
CGridView::EnsureRows( void ) const
{GUCEF_TRACE;

    if ( !m_rowsDirty )
        return;
    m_rows = SplitStringList( GetPropertyRef( EWidgetProperty_Rows ) );
    UInt32 most = 0;
    for ( size_t r=0; r<m_rows.size(); ++r )
    {
        const char* text = m_rows[ r ].C_String();
        if ( '\0' == text[ 0 ] )
            continue;
        UInt32 cells = 1;
        for ( ; '\0' != *text; ++text )
            if ( '\t' == *text )
                ++cells;
        if ( cells > most )
            most = cells;
    }
    m_rowCellCount = most;
    m_rowsDirty = false;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridView::GetRowCount( void ) const
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
        return m_model->GetRowCount();
    EnsureRows();
    return (UInt32) m_rows.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CGridView::GetColumnCount( void ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_columnCount;
}

/*-------------------------------------------------------------------------*/

CGridView::TStringVector
CGridView::GetRowCells( UInt32 rowIndex ) const
{GUCEF_TRACE;

    TStringVector cells;
    if ( !m_model.IsNULL() )
    {
        if ( rowIndex >= m_model->GetRowCount() )
            return cells;
        const UInt32 columns = m_model->GetColumnCount();
        for ( UInt32 c=0; c<columns; ++c )
            cells.push_back( m_model->GetCellText( rowIndex, c ) );
        return cells;
    }
    EnsureRows();
    if ( rowIndex >= (UInt32) m_rows.size() )
        return cells;
    CString::StringVector parts = m_rows[ rowIndex ].ParseElements( '\t', true );
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
        cells.push_back( parts[ i ] );
    return cells;
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetRowText( Int32 row ) const
{GUCEF_TRACE;

    if ( row < 0 )
        return CString::Empty;
    if ( !m_model.IsNULL() )
        return JoinCells( GetRowCells( (UInt32) row ) );
    EnsureRows();
    return row < (Int32) m_rows.size() ? m_rows[ row ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetText( const UInt32 columnIndex ,
                    const UInt32 rowIndex    ,
                    const CString& text      )
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
        return;
    EnsureRows();
    TStringVector rows = m_rows;
    while ( rows.size() <= rowIndex )
        rows.push_back( CString::Empty );
    TStringVector cells = GetRowCells( rowIndex );
    while ( cells.size() <= columnIndex )
        cells.push_back( CString::Empty );
    cells[ columnIndex ] = text;
    rows[ rowIndex ] = JoinCells( cells );
    SetStringListProperty( EWidgetProperty_Rows, rows );
    TGridItemAddedEventData eventData;
    eventData.GetData().column = columnIndex;
    eventData.GetData().row = rowIndex;
    NotifyObservers( GridItemAddedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetText( const UInt32 columnIndex ,
                   const UInt32 rowIndex    ) const
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
    {
        SCellContent content;
        GetCellContent( (Int32) rowIndex, (Int32) columnIndex, content );
        return content.text;
    }
    TStringVector cells = GetRowCells( rowIndex );
    return columnIndex < (UInt32) cells.size() ? cells[ columnIndex ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CGridView::AppendText( const UInt32 columnIndex ,
                       const CString& text      )
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
        return;
    UInt32 row = GetRowCount();
    SetText( columnIndex, row, text );
    TGridRowAddedEventData eventData;
    eventData.GetData().column = columnIndex;
    eventData.GetData().row = row;
    NotifyObservers( GridRowAddedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CGridView::AppendText( const CString& text )
{GUCEF_TRACE;

    AppendText( 0, text );
}

/*-------------------------------------------------------------------------*/

void
CGridView::Clear( void )
{GUCEF_TRACE;

    CUpdateScope scope( *this );
    SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    SetProperty( EWidgetProperty_SelectedRows, CString::Empty );
    SetProperty( EWidgetProperty_EmphasizedRows, CString::Empty );
    SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    if ( !m_model.IsNULL() )
    {
        ClearSelection();
        return;
    }
    SetProperty( EWidgetProperty_Rows, CString::Empty );
    ClearSelection();
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetRows( const TRowVector& rows )
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
        return;
    TStringVector joined;
    joined.reserve( rows.size() );
    for ( size_t i=0; i<rows.size(); ++i )
        joined.push_back( JoinCells( rows[ i ] ) );
    CUpdateScope scope( *this );
    SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    SetProperty( EWidgetProperty_SelectedRows, CString::Empty );
    SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    SetStringListProperty( EWidgetProperty_Rows, joined );
    ClearSelection();
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetAllowMultiSelect( bool allowMultiSelect )
{GUCEF_TRACE;

    SetBoolProperty( EWidgetProperty_MultiSelect, allowMultiSelect );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::GetAllowMultiSelect( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_MultiSelect );
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetSelectedRow( void ) const
{GUCEF_TRACE;

    Int32 row = GetIntProperty( EWidgetProperty_SelectedRow );
    return row >= 0 && row < (Int32) GetRowCount() ? row : -1;
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetSelectedRow( Int32 row )
{GUCEF_TRACE;

    CUpdateScope scope( *this );
    if ( IsCellMode() )
    {
        if ( row >= 0 && row < (Int32) GetRowCount() )
        {
            EnsureLayout();
            SetCursor( row, m_cursorColumn >= 0 ? m_cursorColumn : ColumnOfPosition( 0 ) );
        }
        else
            ClearSelection();
        return;
    }
    TInt32Vector rows;
    if ( row >= 0 && row < (Int32) GetRowCount() )
        rows.push_back( row );
    SetSelectedRows( rows );
}

/*-------------------------------------------------------------------------*/

CGridView::TInt32Vector
CGridView::GetSelectedRows( void ) const
{GUCEF_TRACE;

    if ( !GetAllowMultiSelect() )
    {
        TInt32Vector rows;
        Int32 row = GetSelectedRow();
        if ( row >= 0 )
            rows.push_back( row );
        return rows;
    }
    return SortedUnique( GetIntListProperty( EWidgetProperty_SelectedRows ), (Int32) GetRowCount() );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetSelectedRows( const TInt32Vector& rows )
{GUCEF_TRACE;

    TInt32Vector valid = SortedUnique( rows, (Int32) GetRowCount() );
    if ( !GetAllowMultiSelect() && valid.size() > 1 )
        valid.resize( 1 );

    // the row of the cursor first, then the list: whoever is told of the change finds the whole new state
    CUpdateScope scope( *this );
    SetIntProperty( EWidgetProperty_SelectedRow, valid.empty() ? -1 : valid.front() );
    SetIntProperty( EWidgetProperty_AnchorRow, valid.empty() ? -1 : valid.front() );
    SetIntListProperty( EWidgetProperty_SelectedRows, valid );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetEmphasizedRows( const TInt32Vector& rows )
{GUCEF_TRACE;

    SetIntListProperty( EWidgetProperty_EmphasizedRows, SortedUnique( rows, (Int32) GetRowCount() ) );
}

/*-------------------------------------------------------------------------*/

CGridView::TInt32Vector
CGridView::GetEmphasizedRows( void ) const
{GUCEF_TRACE;

    return SortedUnique( GetIntListProperty( EWidgetProperty_EmphasizedRows ), (Int32) GetRowCount() );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsRowEmphasized( Int32 row ) const
{GUCEF_TRACE;

    if ( IsRowEmphasizedByModel( row ) )
        return true;
    const TInt32Vector rows = GetEmphasizedRows();
    return std::binary_search( rows.begin(), rows.end(), row );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsRowEmphasizedByModel( Int32 row ) const
{GUCEF_TRACE;

    return !m_model.IsNULL() && row >= 0 && m_model->IsRowEmphasized( (UInt32) row );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsRowSelected( Int32 row ) const
{GUCEF_TRACE;

    if ( IsCellMode() )
        return m_selection.ContainsWholeRow( row );
    TInt32Vector rows = GetSelectedRows();
    for ( size_t i=0; i<rows.size(); ++i )
        if ( rows[ i ] == row )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetVisibleRowCount( void ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    if ( rowHeight <= 0 )
        return 0;
    const Int32 footer = GetFooterTop() >= 0 ? rowHeight : 0;
    Int32 visible = ( GetRect().height - footer ) / rowHeight - 1;
    return visible > 0 ? visible : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetFrozenRowCount( void ) const
{GUCEF_TRACE;

    Int32 frozen = GetIntProperty( EWidgetProperty_FrozenRows );
    if ( frozen <= 0 )
        return 0;
    const Int32 visible = GetVisibleRowCount();
    if ( frozen > visible - 1 )
        frozen = visible > 1 ? visible - 1 : 0;
    const Int32 count = (Int32) GetRowCount();
    return frozen > count ? count : frozen;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetFirstScrolledRow( void ) const
{GUCEF_TRACE;

    const Int32 frozen = GetFrozenRowCount();
    const Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition );
    return scroll > frozen ? scroll : frozen;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetMaxScrollRow( void ) const
{GUCEF_TRACE;

    const Int32 frozen = GetFrozenRowCount();
    const Int32 slots = GetVisibleRowCount() - frozen;
    Int32 max = (Int32) GetRowCount() - slots;
    return max > frozen ? max : frozen;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsFooterShown( void ) const
{GUCEF_TRACE;

    if ( m_model.IsNULL() || !m_model->HasFooter() )
        return false;

    // a ShowFooter that was set on the grid wins; else the model decides
    return !IsPropertySet( EWidgetProperty_ShowFooter ) || GetBoolProperty( EWidgetProperty_ShowFooter );
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetFooterTop( void ) const
{GUCEF_TRACE;

    if ( !IsFooterShown() )
        return -1;
    const Int32 rowHeight = GetRowHeight();
    return GetRect().height - rowHeight;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetRowTop( Int32 row ) const
{GUCEF_TRACE;

    const Int32 rowHeight = GetRowHeight();
    const Int32 frozen = GetFrozenRowCount();
    if ( row < 0 )
        return -1;
    if ( row < frozen )
        return rowHeight + row * rowHeight;
    const Int32 first = GetFirstScrolledRow();
    if ( row < first )
        return -1;
    return rowHeight + ( frozen + row - first ) * rowHeight;
}

/*-------------------------------------------------------------------------*/

void
CGridView::ScrollToRow( Int32 row )
{GUCEF_TRACE;

    if ( row < 0 )
        return;
    const Int32 frozen = GetFrozenRowCount();
    if ( row < frozen )
        return;
    Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition );
    const Int32 first = GetFirstScrolledRow();
    const Int32 visible = GetVisibleRowCount() - frozen;
    if ( row < first )
        scroll = row;
    else
    if ( visible > 0 && row >= first + visible )
        scroll = row - visible + 1;
    if ( scroll != GetIntProperty( EWidgetProperty_ScrollPosition ) )
        SetIntProperty( EWidgetProperty_ScrollPosition, scroll );
}

/*-------------------------------------------------------------------------*/

void
CGridView::ScrollToCell( Int32 row, Int32 column )
{GUCEF_TRACE;

    ScrollToRow( row );
    EnsureLayout();
    const Int32 position = m_layout.FindPosition( column );
    if ( position < 0 )
        return;
    const Int32 scroll = m_layout.GetScrollToShow( position );
    if ( scroll != m_layout.GetScrollX() )
        SetIntProperty( EWidgetProperty_ScrollPositionX, scroll );
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetColumnAlignment( UInt32 column ) const
{GUCEF_TRACE;

    EnsureLayout();
    if ( column < (UInt32) m_alignments.size() && ( "Right" == m_alignments[ column ] || "Center" == m_alignments[ column ] || "Left" == m_alignments[ column ] ) )
    {
        // the legacy meaning of an entry of Left: the same as none
        return m_alignments[ column ];
    }
    if ( !m_model.IsNULL() )
        return m_model->GetColumn( column ).GetEffectiveAlignment();
    return "Left";
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetCellTextRoom( UInt32 column, Int32 row ) const
{GUCEF_TRACE;

    const GUCEF::vector< Int32 > widths = GetColumnWidths();
    if ( column >= (UInt32) widths.size() )
        return 0;
    Int32 room = widths[ column ] - CELL_PADDING_LEFT - CELL_PADDING_RIGHT;
    if ( 0 == column && row >= 0 && IsRowEmphasized( row ) )
        room -= EMPHASIS_INDENT;
    return room > 0 ? room : 0;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsCellTextCut( UInt32 column, UInt32 row ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return false;
    const CString text = GetText( column, row );
    if ( text.IsNULLOrEmpty() )
        return false;
    Int32 width = 0, height = 0;
    context->MeasureText( GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height );
    return width > GetCellTextRoom( column, (Int32) row );
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetTooltipAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_editing && m_editing->IsEditing() )
    {
        const CString refusal = m_editing->GetTooltipAt( localX, localY );
        if ( !refusal.IsNULLOrEmpty() )
            return refusal;
    }
    if ( "Ellipsis" == GetPropertyRef( EWidgetProperty_TextOverflow ) )
    {
        const Int32 row = GetRowAt( localY );
        const Int32 column = GetColumnAt( localX );
        if ( row >= 0 && column >= 0 && IsCellTextCut( (UInt32) column, (UInt32) row ) )
            return GetText( (UInt32) column, (UInt32) row );
    }
    return CWidget::GetTooltipAt( localX, localY );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::SortRows( UInt32 column, bool ascending )
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
    {
        if ( !m_model->IsSortSupported() || !m_model->SortByColumn( column, ascending ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CGridView: The model does not sort by column " << column << ": the rows stay as they are" );
            return false;
        }
        SetIntProperty( EWidgetProperty_SortColumn, (Int32) column );
        SetBoolProperty( EWidgetProperty_SortAscending, ascending );
        return true;
    }

    EnsureRows();
    TStringVector rows = m_rows;
    TInt32Vector selected = GetSelectedRows();
    Int32 cursor = GetSelectedRow();

    // a stable insertion sort of the row indices: the grids of a GUI are not large
    TInt32Vector order;
    for ( Int32 i=0; i<(Int32)rows.size(); ++i )
        order.push_back( i );
    for ( size_t i=1; i<order.size(); ++i )
    {
        Int32 current = order[ i ];
        const CString key = GetText( column, (UInt32) current );
        size_t j = i;
        while ( j > 0 )
        {
            Int32 compared = CompareCells( GetText( column, (UInt32) order[ j-1 ] ), key );
            if ( ascending ? compared <= 0 : compared >= 0 )
                break;
            order[ j ] = order[ j-1 ];
            --j;
        }
        order[ j ] = current;
    }

    TStringVector sorted;
    TInt32Vector newIndexOf( rows.size(), -1 );
    for ( size_t i=0; i<order.size(); ++i )
    {
        sorted.push_back( rows[ order[ i ] ] );
        newIndexOf[ order[ i ] ] = (Int32) i;
    }
    CUpdateScope scope( *this );
    SetStringListProperty( EWidgetProperty_Rows, sorted );
    SetIntProperty( EWidgetProperty_SortColumn, (Int32) column );
    SetBoolProperty( EWidgetProperty_SortAscending, ascending );

    // the selection stays with its rows; the row of the cursor first, as in SetSelectedRows
    TInt32Vector moved;
    for ( size_t i=0; i<selected.size(); ++i )
        moved.push_back( newIndexOf[ selected[ i ] ] );
    SetIntProperty( EWidgetProperty_SelectedRow, cursor >= 0 ? newIndexOf[ cursor ] : -1 );
    SetIntListProperty( EWidgetProperty_SelectedRows, SortedUnique( moved, (Int32) sorted.size() ) );
    Int32 anchor = GetIntProperty( EWidgetProperty_AnchorRow );
    SetIntProperty( EWidgetProperty_AnchorRow, anchor >= 0 && anchor < (Int32) newIndexOf.size() ? newIndexOf[ anchor ] : -1 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridView::SelectByUser( const TInt32Vector& rows, Int32 cursorRow )
{GUCEF_TRACE;

    if ( !GetAllowMultiSelect() )
    {
        // one row: only SelectedRow changes, as it always did
        ChangeValueByUser( EWidgetProperty_SelectedRow, CORE::Int32ToString( cursorRow ) );
        return;
    }
    TInt32Vector valid = SortedUnique( rows, (Int32) GetRowCount() );
    CUpdateScope scope( *this );
    CString oldValue = GetProperty( EWidgetProperty_SelectedRows );
    SetIntListProperty( EWidgetProperty_SelectedRows, valid );
    CString newValue = GetProperty( EWidgetProperty_SelectedRows );
    if ( oldValue != newValue )
    {
        CValueChangedEventData eventData;
        eventData.property = EWidgetProperty_SelectedRows;
        eventData.oldValue = oldValue;
        eventData.newValue = newValue;
        DispatchEvent( ValueChangedEvent, eventData );
    }
    ChangeValueByUser( EWidgetProperty_SelectedRow, CORE::Int32ToString( cursorRow ) );
}

/*-------------------------------------------------------------------------*/

void
CGridView::MoveCursorByUser( Int32 row, bool extend )
{GUCEF_TRACE;

    Int32 count = (Int32) GetRowCount();
    if ( 0 == count )
        return;
    if ( row < 0 ) row = 0;
    if ( row >= count ) row = count - 1;

    TInt32Vector rows;
    Int32 anchor = GetIntProperty( EWidgetProperty_AnchorRow );
    if ( extend && GetAllowMultiSelect() && anchor >= 0 && anchor < count )
    {
        for ( Int32 r = ( anchor < row ? anchor : row ); r <= ( anchor < row ? row : anchor ); ++r )
            rows.push_back( r );
    }
    else
    {
        rows.push_back( row );
        SetIntProperty( EWidgetProperty_AnchorRow, row );
    }
    SelectByUser( rows, row );
    ScrollToRow( row );
}

/*-------------------------------------------------------------------------*/

void
CGridView::InvalidateLayout( void )
{GUCEF_TRACE;

    m_layoutDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    CWidget::OnPropertyChanged( property, oldValue, newValue );
    switch ( property )
    {
        case EWidgetProperty_Rows:
        {
            m_rowsDirty = true;
            m_layoutDirty = true;
            break;
        }
        case EWidgetProperty_Columns:
        case EWidgetProperty_ColumnWidths:
        case EWidgetProperty_ColumnAlignments:
        case EWidgetProperty_FitColumns:
        case EWidgetProperty_ColumnOrder:
        case EWidgetProperty_HiddenColumns:
        case EWidgetProperty_FrozenColumns:
        case EWidgetProperty_ShowRowHeader:
        case EWidgetProperty_RowHeaderWidth:
        case EWidgetProperty_HorizontalScroll:
        case EWidgetProperty_ScrollPositionX:
        case EWidgetProperty_Font:
        case EWidgetProperty_RowHeight:
        {
            m_layoutDirty = true;
            break;
        }
        case EWidgetProperty_SelectionMode:
        {
            m_selection.Clear();
            m_cursorRow = -1;
            m_cursorColumn = -1;
            if ( IsCellMode() )
            {
                const Int32 row = GetSelectedRow();
                if ( row >= 0 )
                {
                    EnsureLayout();
                    SetCursorCell( row, ColumnOfPosition( 0 ) );
                    m_selection.SelectCell( row, 0 );
                    SyncCellSelection( false );
                }
            }
            break;
        }
        case EWidgetProperty_SelectedRanges:
        {
            if ( !m_syncing && IsCellMode() )
                ApplySelectionText( newValue );
            break;
        }
        case EWidgetProperty_SelectedRow:
        {
            RaiseSelectionChanged();
            if ( !m_syncing )
            {
                if ( IsCellMode() )
                {
                    const Int32 row = GetSelectedRow();
                    if ( row != m_cursorRow )
                    {
                        EnsureLayout();
                        SetCursor( row, m_cursorColumn >= 0 ? m_cursorColumn : ColumnOfPosition( 0 ) );
                    }
                }
                else
                    CaptureKeys();
            }
            break;
        }
        case EWidgetProperty_SelectedRows:
        case EWidgetProperty_AnchorRow:
        {
            if ( EWidgetProperty_SelectedRows == property )
                RaiseSelectionChanged();
            if ( !m_syncing && !IsCellMode() )
                CaptureKeys();
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CGridView::GetSelectedItems( TGridIndexVector& gridIndexList )
{GUCEF_TRACE;

    Int32 row = GetIntProperty( EWidgetProperty_SelectedRow );
    if ( row < 0 || row >= (Int32) GetRowCount() )
        return;
    UInt32 columns = GetColumnCount();
    for ( UInt32 c=0; c<columns; ++c )
    {
        TGridIndex index;
        index.column = c;
        index.row = (UInt32) row;
        gridIndexList.push_back( index );
    }
}

/*-------------------------------------------------------------------------*/

CGridView::TInt32Vector
CGridView::ComputeBaseWidths( Int32 available, UInt32& columns ) const
{GUCEF_TRACE;

    TInt32Vector widths = GetIntListProperty( EWidgetProperty_ColumnWidths );
    const bool fit = GetBoolProperty( EWidgetProperty_FitColumns );
    const bool scrolls = GetBoolProperty( EWidgetProperty_HorizontalScroll );

    if ( scrolls || !m_model.IsNULL() )
    {
        // columns that have a width of their own (the property, else the model) keep it, the others share the room that is left, or have the default width when the grid scrolls
        TInt32Vector result( columns, 0 );
        Int64 fixedTotal = 0;
        UInt32 unspecified = 0;
        for ( UInt32 c=0; c<columns; ++c )
        {
            Int32 width = c < (UInt32) widths.size() && widths[ c ] > 0 ? widths[ c ] : 0;
            if ( 0 == width && !m_model.IsNULL() && m_model->GetColumn( c ).width > 0 )
                width = m_model->GetColumn( c ).width;
            result[ c ] = width;
            if ( width > 0 )
                fixedTotal += width;
            else
                ++unspecified;
        }
        Int32 shared = (Int32) DEFAULT_COLUMN_WIDTH;
        if ( !scrolls && unspecified > 0 )
        {
            const Int64 left = (Int64) available - fixedTotal;
            shared = (Int32) ( left / (Int64) unspecified );
            if ( shared < (Int32) MIN_COLUMN_WIDTH )
                shared = (Int32) MIN_COLUMN_WIDTH;
        }
        Int64 total = 0;
        for ( UInt32 c=0; c<columns; ++c )
        {
            if ( 0 == result[ c ] )
                result[ c ] = shared;
            total += result[ c ];
        }
        if ( !scrolls && fit && total > 0 && available > 0 )
        {
            Int32 used = 0;
            for ( UInt32 c=0; c<columns; ++c )
            {
                Int32 width = c + 1 == columns ? available - used : (Int32) ( (Int64) result[ c ] * available / total );
                result[ c ] = width > 0 ? width : 0;
                used += result[ c ];
            }
        }
        return result;
    }

    if ( widths.size() >= columns && !widths.empty() )
    {
        // the widths are relative to each other: the columns fill the grid, whatever its size
        if ( fit )
        {
            Int64 total = 0;
            for ( UInt32 i=0; i<columns; ++i )
                total += widths[ i ] > 0 ? widths[ i ] : 0;
            if ( total > 0 && available > 0 )
            {
                Int32 used = 0;
                for ( UInt32 i=0; i<columns; ++i )
                {
                    Int32 width = i + 1 == columns ? available - used : (Int32) ( (Int64) ( widths[ i ] > 0 ? widths[ i ] : 0 ) * available / total );
                    widths[ i ] = width > 0 ? width : 0;
                    used += widths[ i ];
                }
            }
        }
        return widths;
    }
    widths.clear();
    if ( 0 == columns )
        return widths;
    Int32 width = available / (Int32) columns;
    for ( UInt32 i=0; i<columns; ++i )
        widths.push_back( width );
    return widths;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::MeasureRowHeaderWidth( void ) const
{GUCEF_TRACE;

    const Int32 configured = GetIntProperty( EWidgetProperty_RowHeaderWidth );
    if ( configured > 0 )
        return configured;
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return MinRowHeaderWidth;
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    UInt32 rows = GetRowCount();
    UInt32 digits = 2;
    for ( UInt32 n = rows; n >= 100; n /= 10 )
        ++digits;
    CString widest;
    for ( UInt32 i=0; i<digits; ++i )
        widest += '9';
    Int32 width = 0, height = 0;
    context->MeasureText( font, widest, 0, width, height );

    // the titles of the model, of the rows in view
    if ( !m_model.IsNULL() )
    {
        const Int32 first = GetIntProperty( EWidgetProperty_ScrollPosition );
        for ( Int32 r = first < 0 ? 0 : first; r < first + 64 && r < (Int32) rows; ++r )
        {
            CString title;
            if ( m_model->GetRowTitle( (UInt32) r, title ) && !title.IsNULLOrEmpty() )
            {
                Int32 titleWidth = 0;
                context->MeasureText( font, title, 0, titleWidth, height );
                if ( titleWidth > width )
                    width = titleWidth;
            }
        }
    }
    width += 2 * RowHeaderPadding;
    return width < MinRowHeaderWidth ? MinRowHeaderWidth : width;
}

/*-------------------------------------------------------------------------*/

void
CGridView::EnsureLayout( void ) const
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    const bool showRowHeader = GetBoolProperty( EWidgetProperty_ShowRowHeader );
    const Int32 rowCount = showRowHeader ? (Int32) GetRowCount() : 0;
    if ( !m_layoutDirty && rect.width == m_layoutWidth && rect.height == m_layoutHeight && rowCount == m_layoutRowCount && font == m_layoutFont )
        return;

    EnsureRows();
    m_titles = SplitStringList( GetPropertyRef( EWidgetProperty_Columns ) );
    m_alignments = SplitStringList( GetPropertyRef( EWidgetProperty_ColumnAlignments ) );
    if ( !m_model.IsNULL() )
        m_columnCount = m_model->GetColumnCount();
    else
    {
        m_columnCount = (UInt32) m_titles.size();
        if ( m_rowCellCount > m_columnCount )
            m_columnCount = m_rowCellCount;
    }
    m_rowHeaderWidth = showRowHeader ? MeasureRowHeaderWidth() : 0;
    UInt32 columns = m_columnCount;
    m_baseWidths = ComputeBaseWidths( rect.width - m_rowHeaderWidth, columns );
    m_layout.Compute( m_baseWidths, GetIntListProperty( EWidgetProperty_ColumnOrder ), GetIntListProperty( EWidgetProperty_HiddenColumns ),
                      GetIntProperty( EWidgetProperty_FrozenColumns ), m_rowHeaderWidth, rect.width,
                      GetBoolProperty( EWidgetProperty_HorizontalScroll ) ? GetIntProperty( EWidgetProperty_ScrollPositionX ) : 0 );
    m_layoutDirty = false;
    m_layoutWidth = rect.width;
    m_layoutHeight = rect.height;
    m_layoutRowCount = rowCount;
    m_layoutFont = font;
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< Int32 >
CGridView::GetColumnWidths( void ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_baseWidths;
}

/*-------------------------------------------------------------------------*/

const CGridColumnLayout&
CGridView::GetColumnLayout( void ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_layout;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetColumnWidth( UInt32 column ) const
{GUCEF_TRACE;

    EnsureLayout();
    return column < (UInt32) m_baseWidths.size() ? m_baseWidths[ column ] : 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetRowHeaderWidth( void ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_rowHeaderWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetRowHeight( void ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetIntProperty( EWidgetProperty_RowHeight );
    if ( rowHeight > 0 )
        return rowHeight;
    return GUCEF_NULL != GetContext() ? GetContext()->GetLineHeight( GetProperty( EWidgetProperty_Font ) ) : 16;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetRowAt( Int32 localY ) const
{GUCEF_TRACE;

    Int32 rowHeight = GetRowHeight();
    if ( rowHeight <= 0 || localY < rowHeight )
        return -1;
    const Int32 footerTop = GetFooterTop();
    if ( footerTop >= 0 && localY >= footerTop )
        return -1;
    const Int32 slot = ( localY - rowHeight ) / rowHeight;
    const Int32 frozen = GetFrozenRowCount();
    Int32 row = slot < frozen ? slot : GetFirstScrolledRow() + ( slot - frozen );
    return row < (Int32) GetRowCount() ? row : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetColumnAt( Int32 localX ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_layout.GetColumn( m_layout.PositionAt( localX ) );
}

/*-------------------------------------------------------------------------*/

CGridView::THit
CGridView::HitTest( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    THit hit;
    hit.kind = HIT_NONE;
    hit.row = -1;
    hit.column = -1;
    hit.position = -1;

    EnsureLayout();
    const CRect& rect = GetRect();
    if ( localX < 0 || localY < 0 || localX >= rect.width || localY >= rect.height )
        return hit;
    const Int32 rowHeight = GetRowHeight();
    const Int32 headerWidth = m_layout.GetRowHeaderWidth();
    const Int32 position = m_layout.PositionAt( localX );

    if ( localY < rowHeight )
    {
        if ( localX < headerWidth )
        {
            hit.kind = HIT_CORNER;
            return hit;
        }
        if ( GetBoolProperty( EWidgetProperty_ResizableColumns ) )
        {
            // the right edge of a title, or the left edge of the next one: the border of the column at its left
            Int32 borderPosition = -1;
            if ( position >= 0 )
            {
                const Int32 right = m_layout.GetScreenLeft( position ) + m_layout.GetWidth( position );
                const Int32 left = m_layout.GetScreenLeft( position );
                if ( localX >= right - BORDER_GRAB_ROOM )
                    borderPosition = position;
                else
                if ( localX < left + BORDER_GRAB_ROOM && position > 0 && ( !m_layout.IsFrozen( position ) || m_layout.IsFrozen( position - 1 ) ) )
                    borderPosition = position - 1;
            }
            else
            if ( m_layout.GetPositionCount() > 0 )
            {
                // just past the last column
                const Int32 last = (Int32) m_layout.GetPositionCount() - 1;
                const Int32 right = m_layout.GetScreenLeft( last ) + m_layout.GetWidth( last );
                if ( localX >= right && localX < right + BORDER_GRAB_ROOM )
                    borderPosition = last;
            }
            if ( borderPosition >= 0 )
            {
                const Int32 edge = m_layout.GetScreenLeft( borderPosition ) + m_layout.GetWidth( borderPosition );
                const bool visible = m_layout.IsFrozen( borderPosition ) || edge >= headerWidth + m_layout.GetFrozenWidth();
                if ( visible )
                {
                    hit.kind = HIT_COLUMN_BORDER;
                    hit.position = borderPosition;
                    hit.column = m_layout.GetColumn( borderPosition );
                    return hit;
                }
            }
        }
        if ( position >= 0 )
        {
            hit.kind = HIT_COLUMN_HEADER;
            hit.position = position;
            hit.column = m_layout.GetColumn( position );
        }
        return hit;
    }

    const Int32 footerTop = GetFooterTop();
    if ( footerTop >= 0 && localY >= footerTop )
    {
        if ( position >= 0 )
        {
            hit.kind = HIT_FOOTER;
            hit.position = position;
            hit.column = m_layout.GetColumn( position );
        }
        return hit;
    }

    hit.row = GetRowAt( localY );
    if ( hit.row < 0 )
        return hit;
    if ( localX < headerWidth )
    {
        hit.kind = HIT_ROW_HEADER;
        return hit;
    }
    if ( position >= 0 )
    {
        hit.kind = HIT_CELL;
        hit.position = position;
        hit.column = m_layout.GetColumn( position );
    }
    else
        hit.row = -1;
    return hit;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::GetCellRect( Int32 row, Int32 column, CRect& rect ) const
{GUCEF_TRACE;

    EnsureLayout();
    const Int32 position = m_layout.FindPosition( column );
    const Int32 top = GetRowTop( row );
    if ( position < 0 || top < 0 )
        return false;
    const Int32 rowHeight = GetRowHeight();
    Int32 left = m_layout.GetScreenLeft( position );
    Int32 right = left + m_layout.GetWidth( position );
    Int32 bottom = top + rowHeight;
    Int32 viewTop = top;

    if ( !m_layout.IsFrozen( position ) )
    {
        const Int32 edge = m_layout.GetRowHeaderWidth() + m_layout.GetFrozenWidth();
        if ( left < edge ) left = edge;
        if ( right > GetRect().width ) right = GetRect().width;
    }
    if ( row >= GetFrozenRowCount() )
    {
        const Int32 edge = rowHeight + GetFrozenRowCount() * rowHeight;
        if ( viewTop < edge ) viewTop = edge;
        const Int32 footerTop = GetFooterTop();
        const Int32 limit = footerTop >= 0 ? footerTop : GetRect().height;
        if ( bottom > limit ) bottom = limit;
    }
    if ( right <= left || bottom <= viewTop )
        return false;
    rect = CRect( left, viewTop, right - left, bottom - viewTop );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridView::PrepareVisibleCells( void ) const
{GUCEF_TRACE;

    if ( m_model.IsNULL() )
        return;
    EnsureLayout();

    // the frozen rows and the rows that scroll, one more for a part of a row
    const Int32 frozen = GetFrozenRowCount();
    if ( frozen > 0 )
        m_model->PrepareRange( CTableViewRange( 0, (UInt32) frozen, 0, m_columnCount ) );
    const Int32 slots = GetVisibleRowCount() - frozen + 1;
    if ( slots > 0 )
        m_model->PrepareRange( CTableViewRange( (UInt32) GetFirstScrolledRow(), (UInt32) slots, 0, m_columnCount ) );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::GetCellContent( Int32 row, Int32 column, SCellContent& content ) const
{GUCEF_TRACE;

    content.kind = SCellContent::KIND_TEXT;
    content.text.Clear();
    content.isNumber = false;
    if ( row < 0 || column < 0 )
        return false;
    if ( !m_model.IsNULL() )
        return m_model->GetCellContent( (UInt32) row, (UInt32) column, content );
    EnsureRows();
    if ( row >= (Int32) m_rows.size() )
        return false;
    const CString::StringVector cells = m_rows[ row ].ParseElements( '\t', true );
    if ( column >= (Int32) cells.size() )
        return false;
    content.text = cells[ column ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsCellEmpty( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    if ( !m_model.IsNULL() )
    {
        CORE::CVariant value;
        if ( row < 0 || column < 0 || !m_model->GetCellValue( (UInt32) row, (UInt32) column, value ) )
            return true;
        return value.IsNULL() || value.IsNIL() || ( value.IsString() && value.IsNULLOrEmpty() );
    }
    return GetText( (UInt32) column, (UInt32) row ).IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetColumnTitle( UInt32 column ) const
{GUCEF_TRACE;

    EnsureLayout();
    if ( !m_model.IsNULL() )
        return m_model->GetColumn( column ).GetDisplayTitle();
    return column < (UInt32) m_titles.size() ? m_titles[ column ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetRowHeaderText( Int32 row ) const
{GUCEF_TRACE;

    CString title;
    if ( !m_model.IsNULL() && row >= 0 && m_model->GetRowTitle( (UInt32) row, title ) )
        return title;
    return CORE::Int32ToString( row + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::GetFooterContent( UInt32 column, SCellContent& content ) const
{GUCEF_TRACE;

    content.kind = SCellContent::KIND_TEXT;
    content.text.Clear();
    content.isNumber = false;
    if ( m_model.IsNULL() || !m_model->HasFooter() )
        return false;
    return m_model->GetFooterContent( column, content );
}

/*-------------------------------------------------------------------------*/

CString
CGridView::GetFooterText( UInt32 column ) const
{GUCEF_TRACE;

    if ( m_model.IsNULL() || !m_model->HasFooter() )
        return CString::Empty;
    return m_model->GetFooterText( column );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetModel( const TTableViewModelPtr& model )
{GUCEF_TRACE;

    if ( m_model.GetPointerAlways() == model.GetPointerAlways() )
        return;
    if ( !m_model.IsNULL() )
        UnsubscribeFrom( m_model.GetPointerAlways() );

    // an editor, an outline of a copy and a batch of the model that was open belong to the model that goes
    if ( GUCEF_NULL != m_editing )
        m_editing->Cancel();
    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->ClearOutline();
    if ( m_modelBatchOpen )
    {
        m_modelBatchOpen = false;
        EndUpdate();
    }
    m_model = model;
    if ( !m_model.IsNULL() )
    {
        TEventCallback cellsCallback( this, &CGridView::OnModelCellsChanged );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::CellsChangedEvent, cellsCallback );
        TEventCallback insertedCallback( this, &CGridView::OnModelRowsInserted );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::RowsInsertedEvent, insertedCallback );
        TEventCallback removedCallback( this, &CGridView::OnModelRowsRemoved );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::RowsRemovedEvent, removedCallback );
        TEventCallback columnsCallback( this, &CGridView::OnModelColumnsChanged );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::ColumnsChangedEvent, columnsCallback );
        TEventCallback resetCallback( this, &CGridView::OnModelReset );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::ModelResetEvent, resetCallback );
        TEventCallback footerCallback( this, &CGridView::OnModelFooterChanged );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::FooterChangedEvent, footerCallback );
        TEventCallback batchStartedCallback( this, &CGridView::OnModelBatchStarted );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::BatchStartedEvent, batchStartedCallback );
        TEventCallback batchFinishedCallback( this, &CGridView::OnModelBatchFinished );
        SubscribeTo( m_model.GetPointerAlways(), CITableViewModel::BatchFinishedEvent, batchFinishedCallback );
        if ( m_model->IsInBatch() )
        {
            m_modelBatchOpen = true;
            BeginUpdate();
        }
    }

    // a new set of rows: nothing is selected or scrolled
    m_keys = SKeyedSelection();
    m_syncing = true;
    SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    SetProperty( EWidgetProperty_SelectedRows, CString::Empty );
    SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    SetIntProperty( EWidgetProperty_ScrollPositionX, 0 );
    SetIntProperty( EWidgetProperty_SelectedColumn, -1 );
    SetStringListProperty( EWidgetProperty_SelectedRanges, TStringVector() );
    m_syncing = false;
    m_selection.Clear();
    m_notifiedSelection.Clear();
    m_cursorRow = -1;
    m_cursorColumn = -1;
    m_notifiedCursorRow = -1;
    m_notifiedCursorColumn = -1;
    m_rowsDirty = true;
    m_layoutDirty = true;
    Touch( EWidgetProperty_Rows );
}

/*-------------------------------------------------------------------------*/

const TTableViewModelPtr&
CGridView::GetModel( void ) const
{GUCEF_TRACE;

    return m_model;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::HasModel( void ) const
{GUCEF_TRACE;

    return !m_model.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelCellsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Touch( EWidgetProperty_Rows );
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelFooterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the footer takes a row from the rows that scroll, so the scroll position may have to follow
    if ( 0 == m_updateDepth )
        ClampToModel();
    else
        m_modelChangedInUpdate = true;
    Touch( EWidgetProperty_Rows );
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelBatchStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_modelBatchOpen )
        return;
    m_modelBatchOpen = true;
    BeginUpdate();
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelBatchFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_modelBatchOpen )
        return;
    m_modelBatchOpen = false;
    EndUpdate();
}

/*-------------------------------------------------------------------------*/

void
CGridView::BeginUpdate( void )
{GUCEF_TRACE;

    ++m_updateDepth;
}

/*-------------------------------------------------------------------------*/

void
CGridView::EndUpdate( void )
{GUCEF_TRACE;

    if ( 0 == m_updateDepth )
        return;
    if ( m_updateDepth > 1 )
    {
        --m_updateDepth;
        return;
    }

    // the outermost end: the selection is found again by key while the notifications are still held back
    if ( m_modelChangedInUpdate )
    {
        m_modelChangedInUpdate = false;
        InvalidateLayout();
        if ( !m_model.IsNULL() )
        {
            RestoreKeys();
            ClampToModel();
        }
        Touch( EWidgetProperty_Rows );
        if ( GUCEF_NULL != m_editing )
            m_editing->OnModelChanged();
        if ( GUCEF_NULL != m_clipboard )
            m_clipboard->OnModelChanged();
    }
    m_updateDepth = 0;
    if ( m_selectionNotifyPending )
    {
        m_selectionNotifyPending = false;
        NotifyObservers( SelectionChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsUpdating( void ) const
{GUCEF_TRACE;

    return m_updateDepth > 0;
}

/*-------------------------------------------------------------------------*/

void
CGridView::RaiseSelectionChanged( void )
{GUCEF_TRACE;

    if ( m_updateDepth > 0 )
        m_selectionNotifyPending = true;
    else
        NotifyObservers( SelectionChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelRowsInserted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const TTableViewRangeEventData* range = static_cast< const TTableViewRangeEventData* >( eventData );
    if ( GUCEF_NULL == range )
        return;
    const Int32 first = (Int32) range->GetData().firstRow;
    const Int32 count = (Int32) range->GetData().rowCount;

    // rows before the first one in view push it down, so that the same rows stay in view
    const Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition );
    if ( scroll > 0 && first < scroll )
        SetIntProperty( EWidgetProperty_ScrollPosition, scroll + count );

    TInt32Vector emphasized = GetIntListProperty( EWidgetProperty_EmphasizedRows );
    for ( size_t i=0; i<emphasized.size(); ++i )
        if ( emphasized[ i ] >= first )
            emphasized[ i ] += count;
    if ( !emphasized.empty() )
        SetIntListProperty( EWidgetProperty_EmphasizedRows, emphasized );

    if ( m_updateDepth > 0 )
    {
        // inside an update the selection is found again by key at the end; the places are not followed one change at a time
        m_modelChangedInUpdate = true;
        Touch( EWidgetProperty_Rows );
        return;
    }

    CUpdateScope scope( *this );
    m_syncing = true;
    TInt32Vector selected = GetIntListProperty( EWidgetProperty_SelectedRows );
    for ( size_t i=0; i<selected.size(); ++i )
        if ( selected[ i ] >= first )
            selected[ i ] += count;
    SetIntListProperty( EWidgetProperty_SelectedRows, selected );
    Int32 row = GetIntProperty( EWidgetProperty_SelectedRow );
    if ( row >= first )
        SetIntProperty( EWidgetProperty_SelectedRow, row + count );
    Int32 anchor = GetIntProperty( EWidgetProperty_AnchorRow );
    if ( anchor >= first )
        SetIntProperty( EWidgetProperty_AnchorRow, anchor + count );
    m_syncing = false;

    m_selection.OnRowsInserted( first, count );
    if ( m_cursorRow >= first )
        m_cursorRow += count;
    InvalidateLayout();
    if ( IsCellMode() )
        SyncCellSelection( false );
    CaptureKeys();
    Touch( EWidgetProperty_Rows );
    if ( GUCEF_NULL != m_editing )
        m_editing->OnModelChanged();
    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->OnModelChanged();
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelRowsRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const TTableViewRangeEventData* range = static_cast< const TTableViewRangeEventData* >( eventData );
    if ( GUCEF_NULL == range )
        return;
    const Int32 first = (Int32) range->GetData().firstRow;
    const Int32 count = (Int32) range->GetData().rowCount;
    const Int32 end = first + count;

    TInt32Vector emphasized = GetIntListProperty( EWidgetProperty_EmphasizedRows );
    if ( !emphasized.empty() )
    {
        TInt32Vector emphasizedKept;
        for ( size_t i=0; i<emphasized.size(); ++i )
        {
            if ( emphasized[ i ] >= first && emphasized[ i ] < end )
                continue;
            emphasizedKept.push_back( emphasized[ i ] >= end ? emphasized[ i ] - count : emphasized[ i ] );
        }
        SetIntListProperty( EWidgetProperty_EmphasizedRows, emphasizedKept );
    }

    if ( m_updateDepth > 0 )
    {
        m_modelChangedInUpdate = true;
        Touch( EWidgetProperty_Rows );
        return;
    }

    CUpdateScope scope( *this );
    m_syncing = true;
    TInt32Vector selected = GetIntListProperty( EWidgetProperty_SelectedRows );
    TInt32Vector kept;
    for ( size_t i=0; i<selected.size(); ++i )
    {
        if ( selected[ i ] >= first && selected[ i ] < end )
            continue;
        kept.push_back( selected[ i ] >= end ? selected[ i ] - count : selected[ i ] );
    }
    SetIntListProperty( EWidgetProperty_SelectedRows, kept );
    Int32 row = GetIntProperty( EWidgetProperty_SelectedRow );
    if ( row >= first && row < end )
        SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    else
    if ( row >= end )
        SetIntProperty( EWidgetProperty_SelectedRow, row - count );
    Int32 anchor = GetIntProperty( EWidgetProperty_AnchorRow );
    if ( anchor >= first && anchor < end )
        SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    else
    if ( anchor >= end )
        SetIntProperty( EWidgetProperty_AnchorRow, anchor - count );
    m_syncing = false;

    m_selection.OnRowsRemoved( first, count );
    if ( m_cursorRow >= first && m_cursorRow < end )
        m_cursorRow = m_selection.IsEmpty() ? -1 : m_cursorRow;
    else
    if ( m_cursorRow >= end )
        m_cursorRow -= count;
    InvalidateLayout();
    ClampToModel();
    CaptureKeys();
    Touch( EWidgetProperty_Rows );
    if ( GUCEF_NULL != m_editing )
        m_editing->OnModelChanged();
    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->OnModelChanged();
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelColumnsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    InvalidateLayout();
    if ( m_updateDepth > 0 )
        m_modelChangedInUpdate = true;
    else
        ClampToModel();
    Touch( EWidgetProperty_Rows );
    if ( GUCEF_NULL != m_editing )
        m_editing->OnModelChanged();
    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->OnModelChanged();
}

/*-------------------------------------------------------------------------*/

void
CGridView::OnModelReset( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    InvalidateLayout();
    if ( m_updateDepth > 0 )
        m_modelChangedInUpdate = true;
    else
    {
        CUpdateScope scope( *this );
        RestoreKeys();
        ClampToModel();
    }
    Touch( EWidgetProperty_Rows );
    if ( GUCEF_NULL != m_editing )
        m_editing->OnModelChanged();
    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->OnModelChanged();
}

/*-------------------------------------------------------------------------*/

void
CGridView::ClampToModel( void )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    EnsureLayout();
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    if ( IsCellMode() )
    {
        m_selection.Clamp( rows, positions );
        if ( m_cursorRow >= rows )
            m_cursorRow = rows - 1;
        if ( m_cursorColumn >= 0 && m_layout.FindPosition( m_cursorColumn ) < 0 )
            m_cursorColumn = positions > 0 ? ColumnOfPosition( positions - 1 ) : -1;
        if ( rows <= 0 )
        {
            m_cursorRow = -1;
            m_cursorColumn = -1;
            m_selection.Clear();
        }
        SyncCellSelection( false );
    }
    else
    {
        m_syncing = true;
        const Int32 row = GetIntProperty( EWidgetProperty_SelectedRow );
        if ( row >= rows )
            SetIntProperty( EWidgetProperty_SelectedRow, -1 );
        TInt32Vector selected = GetIntListProperty( EWidgetProperty_SelectedRows );
        TInt32Vector valid = SortedUnique( selected, rows );
        if ( valid.size() != selected.size() )
            SetIntListProperty( EWidgetProperty_SelectedRows, valid );
        m_syncing = false;
    }

    // never scroll past the end
    const Int32 maxScroll = GetMaxScrollRow();
    if ( GetIntProperty( EWidgetProperty_ScrollPosition ) > maxScroll )
        SetIntProperty( EWidgetProperty_ScrollPosition, maxScroll );
}

/*-------------------------------------------------------------------------*/

void
CGridView::CaptureKeys( void )
{GUCEF_TRACE;

    if ( m_model.IsNULL() )
        return;
    m_keys = SKeyedSelection();
    const Int32 rows = (Int32) GetRowCount();
    Int32 cursor = -1, anchor = -1;
    GUCEF::vector< Int32 > selectedRows;
    if ( IsCellMode() )
    {
        cursor = m_cursorRow;
        anchor = m_selection.GetAnchorRow();
        UInt32 count = 0;
        for ( UInt32 i=0; i<m_selection.GetRangeCount() && !m_keys.overflow; ++i )
        {
            const CGridSelection::SRange& range = m_selection.GetRange( i );
            if ( !range.wholeRows || range.wholeColumns )
                continue;
            for ( Int32 r=range.firstRow; r<=range.lastRow; ++r )
            {
                if ( ++count > MaxKeyedRows )
                {
                    m_keys.overflow = true;
                    break;
                }
                selectedRows.push_back( r );
            }
        }
    }
    else
    {
        cursor = GetIntProperty( EWidgetProperty_SelectedRow );
        anchor = GetIntProperty( EWidgetProperty_AnchorRow );
        if ( GetAllowMultiSelect() )
        {
            selectedRows = GetIntListProperty( EWidgetProperty_SelectedRows );
            if ( selectedRows.size() > MaxKeyedRows )
            {
                selectedRows.clear();
                m_keys.overflow = true;
            }
        }
    }
    if ( cursor >= 0 && cursor < rows )
        m_keys.hasCursor = m_model->GetRowKey( (UInt32) cursor, m_keys.cursorKey );
    if ( anchor >= 0 && anchor < rows )
        m_keys.hasAnchor = m_model->GetRowKey( (UInt32) anchor, m_keys.anchorKey );
    for ( size_t i=0; i<selectedRows.size(); ++i )
    {
        CORE::CVariant key;
        if ( selectedRows[ i ] >= 0 && selectedRows[ i ] < rows && m_model->GetRowKey( (UInt32) selectedRows[ i ], key ) )
            m_keys.rowKeys.push_back( key );
    }
}

/*-------------------------------------------------------------------------*/

void
CGridView::RestoreKeys( void )
{GUCEF_TRACE;

    if ( m_model.IsNULL() )
        return;
    UInt32 row = 0;
    Int32 cursor = -1, anchor = -1;
    if ( m_keys.hasCursor && m_model->FindRowByKey( m_keys.cursorKey, row ) )
        cursor = (Int32) row;
    if ( m_keys.hasAnchor && m_model->FindRowByKey( m_keys.anchorKey, row ) )
        anchor = (Int32) row;
    TInt32Vector rows;
    for ( size_t i=0; i<m_keys.rowKeys.size(); ++i )
        if ( m_model->FindRowByKey( m_keys.rowKeys[ i ], row ) )
            rows.push_back( (Int32) row );
    std::sort( rows.begin(), rows.end() );

    if ( IsCellMode() )
    {
        EnsureLayout();
        m_selection.Clear();
        if ( !rows.empty() )
        {
            // rows that were selected whole stay selected whole; neighbours become one block
            size_t i = 0;
            bool add = false;
            while ( i < rows.size() )
            {
                size_t j = i;
                while ( j + 1 < rows.size() && rows[ j + 1 ] == rows[ j ] + 1 )
                    ++j;
                m_selection.SelectRows( rows[ i ], rows[ j ], add );
                add = true;
                i = j + 1;
            }
        }
        else
        if ( cursor >= 0 && m_cursorColumn >= 0 && m_layout.FindPosition( m_cursorColumn ) >= 0 )
            m_selection.SelectCell( cursor, m_layout.FindPosition( m_cursorColumn ) );
        m_cursorRow = cursor;
        if ( cursor < 0 )
            m_selection.Clear();
        SyncCellSelection( false );
        return;
    }

    m_syncing = true;
    if ( GetAllowMultiSelect() )
        SetIntListProperty( EWidgetProperty_SelectedRows, rows );
    SetIntProperty( EWidgetProperty_AnchorRow, anchor );
    SetIntProperty( EWidgetProperty_SelectedRow, cursor );
    m_syncing = false;
    CaptureKeys();
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsCellMode( void ) const
{GUCEF_TRACE;

    return "Cell" == GetPropertyRef( EWidgetProperty_SelectionMode );
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetCursorRow( void ) const
{GUCEF_TRACE;

    return m_cursorRow;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetCursorColumn( void ) const
{GUCEF_TRACE;

    return m_cursorColumn;
}

/*-------------------------------------------------------------------------*/

const CGridSelection&
CGridView::GetSelection( void ) const
{GUCEF_TRACE;

    return m_selection;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::PositionOfColumn( Int32 column ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_layout.FindPosition( column );
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::ColumnOfPosition( Int32 position ) const
{GUCEF_TRACE;

    EnsureLayout();
    return m_layout.GetColumn( position );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsCellSelected( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    return m_selection.Contains( row, PositionOfColumn( column ) );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetCursorCell( Int32 row, Int32 column )
{GUCEF_TRACE;

    m_cursorRow = row;
    m_cursorColumn = column;
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetCursor( Int32 row, Int32 column )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    const Int32 position = PositionOfColumn( column );
    if ( row < 0 || row >= rows || position < 0 )
        return;
    SetCursorCell( row, column );
    m_selection.SelectCell( row, position );
    ScrollToCell( row, column );
    SyncCellSelection( false );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SelectRange( Int32 firstRow, Int32 firstColumn, Int32 lastRow, Int32 lastColumn )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    const Int32 a = PositionOfColumn( firstColumn );
    const Int32 b = PositionOfColumn( lastColumn );
    if ( rows <= 0 || a < 0 || b < 0 )
        return;
    firstRow = Clamp( firstRow, 0, rows - 1 );
    lastRow = Clamp( lastRow, 0, rows - 1 );
    m_selection.SelectRect( firstRow, a, lastRow, b );
    SetCursorCell( firstRow, firstColumn );
    SyncCellSelection( false );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SelectRows( Int32 firstRow, Int32 lastRow )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    if ( rows <= 0 )
        return;
    EnsureLayout();
    firstRow = Clamp( firstRow, 0, rows - 1 );
    lastRow = Clamp( lastRow, 0, rows - 1 );
    m_selection.SelectRows( firstRow, lastRow, false );
    SetCursorCell( firstRow, m_cursorColumn >= 0 ? m_cursorColumn : ColumnOfPosition( 0 ) );
    SyncCellSelection( false );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SelectColumns( Int32 firstColumn, Int32 lastColumn )
{GUCEF_TRACE;

    const Int32 a = PositionOfColumn( firstColumn );
    const Int32 b = PositionOfColumn( lastColumn );
    if ( a < 0 || b < 0 || GetRowCount() == 0 )
        return;
    m_selection.SelectColumns( a, b, false );
    SetCursorCell( m_cursorRow >= 0 ? m_cursorRow : 0, firstColumn );
    SyncCellSelection( false );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SelectAll( void )
{GUCEF_TRACE;

    if ( GetRowCount() == 0 || GetColumnLayout().GetPositionCount() == 0 )
        return;
    m_selection.SelectAll();
    if ( m_cursorRow < 0 )
        SetCursorCell( 0, ColumnOfPosition( 0 ) );
    SyncCellSelection( false );
}

/*-------------------------------------------------------------------------*/

void
CGridView::ClearSelection( void )
{GUCEF_TRACE;

    if ( IsCellMode() )
    {
        m_selection.Clear();
        m_cursorRow = -1;
        m_cursorColumn = -1;
        SyncCellSelection( false );
    }
    else
    {
        m_selection.Clear();
        m_cursorRow = -1;
        m_cursorColumn = -1;
    }
}

/*-------------------------------------------------------------------------*/

void
CGridView::SyncCellSelection( bool byUser )
{GUCEF_TRACE;

    // SelectedRow, SelectedColumn, the anchor and the ranges are set one after the other: the first notification has them all
    CUpdateScope scope( *this );
    m_syncing = true;
    if ( byUser )
        ChangeValueByUser( EWidgetProperty_SelectedRow, CORE::Int32ToString( m_cursorRow ) );
    else
        SetIntProperty( EWidgetProperty_SelectedRow, m_cursorRow );
    SetIntProperty( EWidgetProperty_SelectedColumn, m_cursorColumn );
    SetIntProperty( EWidgetProperty_AnchorRow, m_selection.GetAnchorRow() );
    SetStringListProperty( EWidgetProperty_SelectedRanges, m_selection.ToText() );
    m_syncing = false;
    Touch( EWidgetProperty_SelectedRanges );

    if ( m_cursorRow != m_notifiedCursorRow || m_cursorColumn != m_notifiedCursorColumn )
    {
        m_notifiedCursorRow = m_cursorRow;
        m_notifiedCursorColumn = m_cursorColumn;
        if ( m_cursorRow >= 0 && m_cursorColumn >= 0 )
        {
            TCellCursorChangedEventData cursor;
            cursor.GetData().column = (UInt32) m_cursorColumn;
            cursor.GetData().row = (UInt32) m_cursorRow;
            NotifyObservers( CellCursorChangedEvent, &cursor );
        }
    }
    if ( m_selection != m_notifiedSelection )
    {
        m_notifiedSelection = m_selection;
        NotifyObservers( CellSelectionChangedEvent );
    }
    CaptureKeys();
}

/*-------------------------------------------------------------------------*/

void
CGridView::ApplySelectionText( const CString& text )
{GUCEF_TRACE;

    const TStringVector lines = SplitStringList( text );
    const Int32 rows = (Int32) GetRowCount();
    EnsureLayout();
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    m_selection.Clear();
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const TInt32Vector numbers = SplitIntList( lines[ i ] );
        if ( numbers.size() != 4 )
            continue;
        const bool wholeRows = -1 == numbers[ 3 ];
        const bool wholeColumns = -1 == numbers[ 2 ];
        const Int32 rowA = numbers[ 0 ];
        const Int32 columnA = numbers[ 1 ];
        const Int32 rowB = wholeColumns ? 0 : numbers[ 2 ];
        const Int32 columnB = wholeRows ? 0 : numbers[ 3 ];
        if ( rows <= 0 || positions <= 0 )
            continue;
        if ( wholeRows && wholeColumns )
            m_selection.SelectAll();
        else
        if ( wholeRows )
            m_selection.SelectRows( Clamp( rowA, 0, rows - 1 ), Clamp( rowB, 0, rows - 1 ), !m_selection.IsEmpty() );
        else
        if ( wholeColumns )
            m_selection.SelectColumns( Clamp( columnA, 0, positions - 1 ), Clamp( columnB, 0, positions - 1 ), !m_selection.IsEmpty() );
        else
        if ( m_selection.IsEmpty() )
            m_selection.SelectRect( Clamp( rowA, 0, rows - 1 ), Clamp( columnA, 0, positions - 1 ), Clamp( rowB, 0, rows - 1 ), Clamp( columnB, 0, positions - 1 ) );
        else
        {
            m_selection.AddCell( Clamp( rowA, 0, rows - 1 ), Clamp( columnA, 0, positions - 1 ) );
            m_selection.ExtendTo( Clamp( rowB, 0, rows - 1 ), Clamp( columnB, 0, positions - 1 ) );
        }
    }
    if ( !m_selection.IsEmpty() && m_cursorRow < 0 )
        SetCursorCell( m_selection.GetAnchorRow(), ColumnOfPosition( m_selection.GetAnchorPosition() ) );
    const bool wasSyncing = m_syncing;
    SyncCellSelection( false );
    m_syncing = wasSyncing;
}

/*-------------------------------------------------------------------------*/

void
CGridView::MoveSelectionTo( Int32 row, Int32 position, bool extend, bool add )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    EnsureLayout();
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    if ( rows <= 0 || positions <= 0 )
        return;
    row = Clamp( row, 0, rows - 1 );
    position = Clamp( position, 0, positions - 1 );
    if ( extend && !m_selection.IsEmpty() )
        m_selection.ExtendTo( row, position );
    else
    {
        if ( add && !m_selection.IsEmpty() )
            m_selection.AddCell( row, position );
        else
            m_selection.SelectCell( row, position );
        SetCursorCell( row, ColumnOfPosition( position ) );
    }
    ScrollToCell( row, ColumnOfPosition( position ) );
    SyncCellSelection( true );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::FindDataEdge( Int32 row, Int32 position, Int32 stepRow, Int32 stepPosition, Int32& edgeRow, Int32& edgePosition ) const
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    edgeRow = row;
    edgePosition = position;
    Int32 nextRow = row + stepRow;
    Int32 nextPosition = position + stepPosition;
    if ( nextRow < 0 || nextRow >= rows || nextPosition < 0 || nextPosition >= positions )
        return false;

    const bool currentEmpty = IsCellEmpty( row, m_layout.GetColumn( position ) );
    const bool nextEmpty = IsCellEmpty( nextRow, m_layout.GetColumn( nextPosition ) );
    if ( !currentEmpty && !nextEmpty )
    {
        // the end of the run of cells that have a value
        Int32 r = nextRow, p = nextPosition;
        for (;;)
        {
            const Int32 followRow = r + stepRow;
            const Int32 followPosition = p + stepPosition;
            if ( followRow < 0 || followRow >= rows || followPosition < 0 || followPosition >= positions || IsCellEmpty( followRow, m_layout.GetColumn( followPosition ) ) )
                break;
            r = followRow;
            p = followPosition;
        }
        edgeRow = r;
        edgePosition = p;
        return true;
    }

    // the next cell that has a value, else the last cell in that direction
    Int32 r = nextRow, p = nextPosition;
    while ( IsCellEmpty( r, m_layout.GetColumn( p ) ) )
    {
        const Int32 followRow = r + stepRow;
        const Int32 followPosition = p + stepPosition;
        if ( followRow < 0 || followRow >= rows || followPosition < 0 || followPosition >= positions )
            break;
        r = followRow;
        p = followPosition;
    }
    edgeRow = r;
    edgePosition = p;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::StepThroughSelection( bool backwards, bool vertical )
{GUCEF_TRACE;

    const Int32 rows = (Int32) GetRowCount();
    EnsureLayout();
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    if ( rows <= 0 || positions <= 0 )
        return false;
    Int32 row = m_cursorRow >= 0 ? m_cursorRow : 0;
    Int32 position = m_cursorColumn >= 0 ? m_layout.FindPosition( m_cursorColumn ) : 0;
    if ( position < 0 )
        position = 0;
    const Int32 step = backwards ? -1 : 1;

    // a block of more than one cell is walked through; the selection stays
    Int32 firstRow = 0, lastRow = rows - 1, firstPosition = 0, lastPosition = positions - 1;
    bool inside = false;
    if ( 1 == m_selection.GetRangeCount() )
    {
        const CGridSelection::SRange& range = m_selection.GetRange( 0 );
        const Int32 rangeFirstRow = range.wholeColumns ? 0 : range.firstRow;
        const Int32 rangeLastRow = range.wholeColumns ? rows - 1 : range.lastRow;
        const Int32 rangeFirstPosition = range.wholeRows ? 0 : range.firstColumn;
        const Int32 rangeLastPosition = range.wholeRows ? positions - 1 : range.lastColumn;
        if ( rangeLastRow > rangeFirstRow || rangeLastPosition > rangeFirstPosition )
        {
            firstRow = rangeFirstRow; lastRow = rangeLastRow;
            firstPosition = rangeFirstPosition; lastPosition = rangeLastPosition;
            inside = row >= firstRow && row <= lastRow && position >= firstPosition && position <= lastPosition;
        }
    }

    if ( !inside )
    {
        // no block to walk through: the whole grid is the room
        firstRow = 0; lastRow = rows - 1;
        firstPosition = 0; lastPosition = positions - 1;
    }
    if ( vertical )
    {
        row += step;
        if ( row > lastRow || row < firstRow )
        {
            if ( !inside )
            {
                // the end of the rows of the grid: Enter stays
                row = Clamp( row, firstRow, lastRow );
            }
            else
            {
                row = backwards ? lastRow : firstRow;
                position += step;
                if ( position > lastPosition || position < firstPosition )
                    position = backwards ? lastPosition : firstPosition;
            }
        }
    }
    else
    {
        position += step;
        if ( position > lastPosition || position < firstPosition )
        {
            position = backwards ? lastPosition : firstPosition;
            row += step;
            if ( row > lastRow || row < firstRow )
            {
                // the end of the grid: the key is for the next widget
                if ( !inside && ( ( !backwards && row > lastRow ) || ( backwards && row < firstRow ) ) )
                    return false;
                row = backwards ? lastRow : firstRow;
            }
        }
    }

    if ( inside )
    {
        SetCursorCell( row, ColumnOfPosition( position ) );
        ScrollToCell( row, ColumnOfPosition( position ) );
        SyncCellSelection( true );
        return true;
    }
    MoveSelectionTo( row, position, false, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnCellKey( CKeyEventData& key )
{GUCEF_TRACE;

    const bool ctrl = 0 != ( key.modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( key.modifiers & INPUT::KEYMOD_SHIFT );
    const Int32 rows = (Int32) GetRowCount();
    EnsureLayout();
    const Int32 positions = (Int32) m_layout.GetPositionCount();

    if ( INPUT::KEYCODE_A == key.keyCode && ctrl )
    {
        if ( rows <= 0 || positions <= 0 )
            return false;
        SelectAll();
        SyncCellSelection( true );
        return true;
    }
    if ( rows <= 0 || positions <= 0 )
        return false;

    const Int32 cursorRow = m_cursorRow >= 0 ? m_cursorRow : 0;
    Int32 cursorPosition = m_cursorColumn >= 0 ? m_layout.FindPosition( m_cursorColumn ) : 0;
    if ( cursorPosition < 0 )
        cursorPosition = 0;

    // the corner that moves: the far corner of the block when it is extended
    Int32 fromRow = cursorRow, fromPosition = cursorPosition;
    if ( shift && !m_selection.IsEmpty() && m_selection.GetExtentRow() >= 0 )
    {
        fromRow = m_selection.GetExtentRow();
        fromPosition = m_selection.GetExtentPosition();
    }
    const Int32 page = GetVisibleRowCount() > 1 ? GetVisibleRowCount() - 1 : 1;
    Int32 toRow = fromRow, toPosition = fromPosition;

    switch ( key.keyCode )
    {
        case INPUT::KEYCODE_UP:
        case INPUT::KEYCODE_DOWN:
        case INPUT::KEYCODE_LEFT:
        case INPUT::KEYCODE_RIGHT:
        {
            const Int32 stepRow = INPUT::KEYCODE_UP == key.keyCode ? -1 : ( INPUT::KEYCODE_DOWN == key.keyCode ? 1 : 0 );
            const Int32 stepPosition = INPUT::KEYCODE_LEFT == key.keyCode ? -1 : ( INPUT::KEYCODE_RIGHT == key.keyCode ? 1 : 0 );
            if ( ctrl )
                FindDataEdge( fromRow, fromPosition, stepRow, stepPosition, toRow, toPosition );
            else
            {
                toRow = fromRow + stepRow;
                toPosition = fromPosition + stepPosition;
            }
            break;
        }
        case INPUT::KEYCODE_HOME:
        {
            toPosition = 0;
            if ( ctrl )
                toRow = 0;
            break;
        }
        case INPUT::KEYCODE_END:
        {
            toPosition = positions - 1;
            if ( ctrl )
                toRow = rows - 1;
            break;
        }
        case INPUT::KEYCODE_PGUP:   toRow = fromRow - page; break;
        case INPUT::KEYCODE_PGDOWN: toRow = fromRow + page; break;
        case INPUT::KEYCODE_TAB:    return StepThroughSelection( shift, false );
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
            return StepThroughSelection( shift, true );
        case INPUT::KEYCODE_SPACE:
        {
            if ( ctrl )
            {
                m_selection.SelectColumns( cursorPosition, cursorPosition, false );
                SetCursorCell( cursorRow, ColumnOfPosition( cursorPosition ) );
                SyncCellSelection( true );
                return true;
            }
            if ( shift )
            {
                m_selection.SelectRows( cursorRow, cursorRow, false );
                SetCursorCell( cursorRow, ColumnOfPosition( cursorPosition ) );
                SyncCellSelection( true );
                return true;
            }
            return false;
        }
        default:
            return false;
    }
    MoveSelectionTo( toRow, toPosition, shift, false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridView::FinishColumnChange( UInt32 column, const CORE::CEvent& eventId, Int32 value )
{GUCEF_TRACE;

    TColumnChangedEventData eventData;
    eventData.GetData().column = column;
    eventData.GetData().value = value;
    NotifyObservers( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetColumnWidth( UInt32 column, Int32 width )
{GUCEF_TRACE;

    EnsureLayout();
    if ( column >= m_columnCount )
        return;
    if ( width < (Int32) MIN_COLUMN_WIDTH )
        width = (Int32) MIN_COLUMN_WIDTH;
    TInt32Vector widths = m_baseWidths;
    widths.resize( m_columnCount, (Int32) DEFAULT_COLUMN_WIDTH );
    if ( widths[ column ] == width )
        return;
    widths[ column ] = width;
    SetIntListProperty( EWidgetProperty_ColumnWidths, widths );
    FinishColumnChange( column, ColumnResizedEvent, width );
}

/*-------------------------------------------------------------------------*/

void
CGridView::FitColumnToContent( UInt32 column )
{GUCEF_TRACE;

    EnsureLayout();
    CGUIContext* context = GetContext();
    if ( column >= m_columnCount || GUCEF_NULL == context )
        return;
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    Int32 widest = 0, height = 0, width = 0;
    context->MeasureText( font, GetColumnTitle( column ), 0, widest, height );
    widest += (Int32) SORT_ARROW_ROOM;

    // the rows in view
    PrepareVisibleCells();
    const Int32 rows = (Int32) GetRowCount();
    const Int32 frozen = GetFrozenRowCount();
    const Int32 first = GetFirstScrolledRow();
    const Int32 slots = GetVisibleRowCount() - frozen + 1;
    for ( Int32 i=0; i<frozen + slots; ++i )
    {
        const Int32 row = i < frozen ? i : first + ( i - frozen );
        if ( row >= rows )
            break;
        SCellContent content;
        if ( !GetCellContent( row, (Int32) column, content ) )
            continue;
        if ( SCellContent::KIND_CHECK_ON == content.kind || SCellContent::KIND_CHECK_OFF == content.kind )
            width = 16;
        else
            context->MeasureText( font, content.text, 0, width, height );
        if ( width > widest )
            widest = width;
    }
    SetColumnWidth( column, widest + (Int32) CELL_PADDING_LEFT + (Int32) CELL_PADDING_RIGHT );
}

/*-------------------------------------------------------------------------*/

void
CGridView::WriteColumnOrder( const TInt32Vector& order )
{GUCEF_TRACE;

    SetIntListProperty( EWidgetProperty_ColumnOrder, order );
}

/*-------------------------------------------------------------------------*/

void
CGridView::MoveColumn( UInt32 column, UInt32 position )
{GUCEF_TRACE;

    EnsureLayout();
    const Int32 current = m_layout.FindPosition( (Int32) column );
    const Int32 positions = (Int32) m_layout.GetPositionCount();
    if ( current < 0 || positions <= 0 )
        return;
    Int32 target = (Int32) position;
    if ( target >= positions )
        target = positions - 1;
    if ( target == current )
        return;

    // the shown columns in their new order, in the places the shown columns had in the order of all columns
    TInt32Vector visible;
    for ( Int32 p=0; p<positions; ++p )
        visible.push_back( m_layout.GetColumn( p ) );
    visible.erase( visible.begin() + current );
    visible.insert( visible.begin() + target, (Int32) column );

    TInt32Vector sequence = GetIntListProperty( EWidgetProperty_ColumnOrder );
    GUCEF::vector< bool > listed( m_columnCount, false );
    TInt32Vector all;
    for ( size_t i=0; i<sequence.size(); ++i )
        if ( sequence[ i ] >= 0 && sequence[ i ] < (Int32) m_columnCount && !listed[ sequence[ i ] ] )
        {
            listed[ sequence[ i ] ] = true;
            all.push_back( sequence[ i ] );
        }
    for ( Int32 c=0; c<(Int32) m_columnCount; ++c )
        if ( !listed[ c ] )
            all.push_back( c );
    size_t next = 0;
    for ( size_t i=0; i<all.size(); ++i )
        if ( m_layout.FindPosition( all[ i ] ) >= 0 )
            all[ i ] = visible[ next++ ];
    WriteColumnOrder( all );

    // block selections are of places: they stay with the cursor only
    if ( IsCellMode() && !m_selection.IsEmpty() )
    {
        EnsureLayout();
        if ( m_cursorRow >= 0 && m_cursorColumn >= 0 )
            m_selection.SelectCell( m_cursorRow, m_layout.FindPosition( m_cursorColumn ) );
        else
            m_selection.Clear();
        SyncCellSelection( false );
    }
    FinishColumnChange( column, ColumnMovedEvent, target );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetColumnHidden( UInt32 column, bool hidden )
{GUCEF_TRACE;

    EnsureLayout();
    if ( column >= m_columnCount || IsColumnHidden( column ) == hidden )
        return;
    TInt32Vector list = GetIntListProperty( EWidgetProperty_HiddenColumns );
    TInt32Vector result;
    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] != (Int32) column )
            result.push_back( list[ i ] );
    if ( hidden )
        result.push_back( (Int32) column );
    SetIntListProperty( EWidgetProperty_HiddenColumns, result );
    EnsureLayout();
    if ( IsCellMode() )
    {
        if ( m_cursorColumn == (Int32) column && hidden )
        {
            const Int32 positions = (Int32) m_layout.GetPositionCount();
            m_cursorColumn = positions > 0 ? ColumnOfPosition( 0 ) : -1;
        }
        if ( m_cursorRow >= 0 && m_cursorColumn >= 0 )
            m_selection.SelectCell( m_cursorRow, m_layout.FindPosition( m_cursorColumn ) );
        else
            m_selection.Clear();
        SyncCellSelection( false );
    }
    FinishColumnChange( column, ColumnVisibilityChangedEvent, hidden ? 0 : 1 );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsColumnHidden( UInt32 column ) const
{GUCEF_TRACE;

    EnsureLayout();
    return column < m_columnCount && m_layout.FindPosition( (Int32) column ) < 0;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetColumnDropPosition( void ) const
{GUCEF_TRACE;

    return DRAG_MOVE == m_dragMode ? m_dropPosition : -1;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnMouseDown( CPointerEventData& pointer )
{GUCEF_TRACE;

    m_suppressClick = false;
    m_dragMode = DRAG_NONE;
    const THit hit = HitTest( pointer.localX, pointer.localY );
    const bool ctrl = 0 != ( pointer.modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( pointer.modifiers & INPUT::KEYMOD_SHIFT );

    if ( HIT_COLUMN_BORDER == hit.kind )
    {
        m_dragMode = DRAG_RESIZE;
        m_dragColumn = hit.column;
        m_dragStartX = pointer.localX;
        m_dragStartWidth = GetColumnWidth( (UInt32) hit.column );
        return true;
    }
    if ( HIT_COLUMN_HEADER == hit.kind && GetBoolProperty( EWidgetProperty_MovableColumns ) )
    {
        m_dragMode = DRAG_MOVE;
        m_dragColumn = hit.column;
        m_dragStartX = pointer.localX;
        m_dropPosition = -1;
        // a press that does not move is a click on the title
        return false;
    }
    if ( !IsCellMode() )
        return false;

    const Int32 rows = (Int32) GetRowCount();
    if ( rows <= 0 )
        return false;
    EnsureLayout();
    switch ( hit.kind )
    {
        case HIT_CELL:
        {
            m_dragMode = DRAG_CELLS;
            MoveSelectionTo( hit.row, hit.position, shift, ctrl && !shift );
            return true;
        }
        case HIT_ROW_HEADER:
        {
            m_dragMode = DRAG_ROWS;
            const Int32 anchorPosition = m_selection.GetAnchorPosition() >= 0 ? m_selection.GetAnchorPosition() : 0;
            if ( shift && !m_selection.IsEmpty() && m_selection.GetAnchorRow() >= 0 )
            {
                if ( m_selection.GetRange( m_selection.GetRangeCount() - 1 ).wholeRows )
                    m_selection.ExtendTo( hit.row, anchorPosition );
                else
                    m_selection.SelectRows( m_selection.GetAnchorRow(), hit.row, false );
            }
            else
                m_selection.SelectRows( hit.row, hit.row, ctrl );
            if ( !shift )
                SetCursorCell( hit.row, m_cursorColumn >= 0 ? m_cursorColumn : ColumnOfPosition( 0 ) );
            SyncCellSelection( true );
            return true;
        }
        case HIT_CORNER:
        {
            m_selection.SelectAll();
            if ( m_cursorRow < 0 )
                SetCursorCell( 0, ColumnOfPosition( 0 ) );
            SyncCellSelection( true );
            return true;
        }
        default:
            break;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnMouseMoved( CPointerEventData& pointer )
{GUCEF_TRACE;

    const bool pressed = 0 != ( pointer.buttonStates & ( 1u << (UInt32) MOUSEBUTTON_LEFT ) );
    if ( DRAG_NONE == m_dragMode || !pressed )
    {
        if ( DRAG_NONE != m_dragMode )
            OnMouseReleased( false );
        return false;
    }
    EnsureLayout();
    switch ( m_dragMode )
    {
        case DRAG_RESIZE:
        {
            SetColumnWidth( (UInt32) m_dragColumn, m_dragStartWidth + ( pointer.localX - m_dragStartX ) );
            m_suppressClick = true;
            return true;
        }
        case DRAG_MOVE:
        {
            if ( m_dropPosition < 0 && ( pointer.localX - m_dragStartX < MoveThreshold && m_dragStartX - pointer.localX < MoveThreshold ) )
                return false;
            // the place between two columns that is nearest to the pointer
            const Int32 positions = (Int32) m_layout.GetPositionCount();
            Int32 position = m_layout.PositionAt( pointer.localX );
            Int32 drop;
            if ( position < 0 )
                drop = pointer.localX < m_layout.GetRowHeaderWidth() + m_layout.GetFrozenWidth() ? 0 : positions;
            else
            {
                const Int32 middle = m_layout.GetScreenLeft( position ) + m_layout.GetWidth( position ) / 2;
                drop = pointer.localX < middle ? position : position + 1;
            }
            if ( drop != m_dropPosition )
            {
                m_dropPosition = drop;
                Touch( EWidgetProperty_ColumnOrder );
            }
            m_suppressClick = true;
            return true;
        }
        case DRAG_CELLS:
        case DRAG_ROWS:
        {
            // outside the cells the pointer pulls the view along, one row or column per move
            const Int32 rows = (Int32) GetRowCount();
            const Int32 positions = (Int32) m_layout.GetPositionCount();
            if ( rows <= 0 || positions <= 0 )
                return false;
            const Int32 rowHeight = GetRowHeight();
            Int32 row = GetRowAt( pointer.localY );
            if ( row < 0 )
            {
                if ( pointer.localY < rowHeight + GetFrozenRowCount() * rowHeight )
                    row = GetFirstScrolledRow() - 1;
                else
                    row = GetFirstScrolledRow() + GetVisibleRowCount() - GetFrozenRowCount();
            }
            Int32 position = m_layout.PositionAt( pointer.localX );
            if ( position < 0 )
            {
                Int32 firstVisible = 0, lastVisible = -1;
                m_layout.GetVisibleScrolledPositions( firstVisible, lastVisible );
                position = pointer.localX < m_layout.GetRowHeaderWidth() + m_layout.GetFrozenWidth() ? firstVisible - 1 : lastVisible + 1;
            }
            if ( DRAG_ROWS == m_dragMode )
                position = m_selection.GetAnchorPosition() >= 0 ? m_selection.GetAnchorPosition() : 0;
            row = Clamp( row, 0, rows - 1 );
            position = Clamp( position, 0, positions - 1 );
            if ( row != m_selection.GetExtentRow() || position != m_selection.GetExtentPosition() )
            {
                if ( !m_selection.IsEmpty() )
                    m_selection.ExtendTo( row, position );
                ScrollToCell( row, ColumnOfPosition( position ) );
                SyncCellSelection( true );
            }
            m_suppressClick = true;
            return true;
        }
        default:
            break;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnMouseReleased( bool completed )
{GUCEF_TRACE;

    const EDragMode mode = m_dragMode;
    const Int32 column = m_dragColumn;
    const Int32 drop = m_dropPosition;
    m_dragMode = DRAG_NONE;
    m_dragColumn = -1;
    m_dropPosition = -1;
    if ( DRAG_MOVE == mode )
    {
        Touch( EWidgetProperty_ColumnOrder );
        if ( completed && drop >= 0 && column >= 0 )
        {
            // the drop place counts the columns including the one that moves
            const Int32 current = PositionOfColumn( column );
            Int32 target = drop > current ? drop - 1 : drop;
            MoveColumn( (UInt32) column, (UInt32) ( target < 0 ? 0 : target ) );
        }
        return true;
    }
    return DRAG_NONE != mode;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnHeaderClicked( const THit& hit, CPointerEventData& pointer )
{GUCEF_TRACE;

    if ( hit.column < 0 )
        return false;
    TColumnClickedEventData clickedColumn;
    clickedColumn.GetData().column = (UInt32) hit.column;
    clickedColumn.GetData().row = 0;
    NotifyObservers( ColumnClickedEvent, &clickedColumn );
    if ( GetBoolProperty( EWidgetProperty_Sortable ) )
    {
        bool ascending = hit.column == GetIntProperty( EWidgetProperty_SortColumn ) ? !GetBoolProperty( EWidgetProperty_SortAscending ) : true;
        SortRows( (UInt32) hit.column, ascending );
        return true;
    }
    if ( IsCellMode() && GetRowCount() > 0 )
    {
        // a click on the title selects the whole column
        const bool ctrl = 0 != ( pointer.modifiers & INPUT::KEYMOD_CTRL );
        const bool shift = 0 != ( pointer.modifiers & INPUT::KEYMOD_SHIFT );
        if ( shift && !m_selection.IsEmpty() && m_selection.GetAnchorPosition() >= 0 )
        {
            if ( m_selection.GetRange( m_selection.GetRangeCount() - 1 ).wholeColumns )
                m_selection.ExtendTo( m_selection.GetAnchorRow() >= 0 ? m_selection.GetAnchorRow() : 0, hit.position );
            else
                m_selection.SelectColumns( m_selection.GetAnchorPosition(), hit.position, false );
        }
        else
        {
            m_selection.SelectColumns( hit.position, hit.position, ctrl );
            SetCursorCell( m_cursorRow >= 0 ? m_cursorRow : 0, hit.column );
        }
        SyncCellSelection( true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnMouseClicked( CPointerEventData& pointer )
{GUCEF_TRACE;

    if ( m_suppressClick )
    {
        m_suppressClick = false;
        return true;
    }
    const THit hit = HitTest( pointer.localX, pointer.localY );

    // the header: the column was clicked, a sortable grid sorts by it
    if ( pointer.localY >= 0 && pointer.localY < GetRowHeight() )
    {
        if ( HIT_COLUMN_BORDER == hit.kind )
        {
            if ( 2 == pointer.clickCount && hit.column >= 0 )
                FitColumnToContent( (UInt32) hit.column );
            return true;
        }
        if ( HIT_CORNER == hit.kind )
            return IsCellMode();
        return OnHeaderClicked( hit, pointer );
    }
    if ( HIT_FOOTER == hit.kind )
        return true;

    Int32 row = GetRowAt( pointer.localY );
    if ( row < 0 )
        return false;
    Int32 column = GetColumnAt( pointer.localX );
    TGridItemClickedEventData clicked;
    clicked.GetData().column = column < 0 ? 0 : (UInt32) column;
    clicked.GetData().row = (UInt32) row;
    NotifyObservers( GridItemClickedEvent, &clicked );
    if ( IsCellMode() && GetBoolProperty( EWidgetProperty_CellEditing ) && column >= 0 && EditController()->OnCellClicked( row, column, pointer.clickCount, pointer.modifiers ) )
        return true;
    if ( 2 == pointer.clickCount )
    {
        ActivateItem( row, GetRowText( row ) );
        return true;
    }
    if ( IsCellMode() )
        return true;

    const bool ctrl = 0 != ( pointer.modifiers & INPUT::KEYMOD_CTRL );
    const bool shift = 0 != ( pointer.modifiers & INPUT::KEYMOD_SHIFT );
    if ( GetAllowMultiSelect() && ctrl )
    {
        // one row in or out of the selection
        TInt32Vector rows = GetSelectedRows();
        TInt32Vector toggled;
        bool wasSelected = false;
        for ( size_t i=0; i<rows.size(); ++i )
        {
            if ( rows[ i ] == row )
                wasSelected = true;
            else
                toggled.push_back( rows[ i ] );
        }
        if ( !wasSelected )
            toggled.push_back( row );
        SetIntProperty( EWidgetProperty_AnchorRow, row );
        SelectByUser( toggled, row );
    }
    else
        MoveCursorByUser( row, shift );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnKeyPressed( CKeyEventData& key )
{GUCEF_TRACE;

    if ( IsCellMode() )
    {
        if ( OnEditKey( key ) )
            return true;
        return OnCellKey( key );
    }

    Int32 count = (Int32) GetRowCount();
    Int32 current = GetSelectedRow();
    Int32 page = GetVisibleRowCount() > 1 ? GetVisibleRowCount() - 1 : 1;
    const bool shift = 0 != ( key.modifiers & INPUT::KEYMOD_SHIFT );
    switch ( key.keyCode )
    {
        case INPUT::KEYCODE_UP:     if ( count > 0 ) MoveCursorByUser( current < 0 ? 0 : current - 1, shift ); return true;
        case INPUT::KEYCODE_DOWN:   if ( count > 0 ) MoveCursorByUser( current < 0 ? 0 : current + 1, shift ); return true;
        case INPUT::KEYCODE_HOME:   if ( count > 0 ) MoveCursorByUser( 0, shift ); return true;
        case INPUT::KEYCODE_END:    if ( count > 0 ) MoveCursorByUser( count - 1, shift ); return true;
        case INPUT::KEYCODE_PGUP:   if ( count > 0 ) MoveCursorByUser( current < 0 ? 0 : current - page, shift ); return true;
        case INPUT::KEYCODE_PGDOWN: if ( count > 0 ) MoveCursorByUser( current < 0 ? 0 : current + page, shift ); return true;
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
        {
            if ( current < 0 )
                return false;
            ActivateItem( current, GetRowText( current ) );
            return true;
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnWheel( CWheelEventData& wheel )
{GUCEF_TRACE;

    EnsureLayout();
    if ( 0 != ( wheel.modifiers & INPUT::KEYMOD_SHIFT ) && GetBoolProperty( EWidgetProperty_HorizontalScroll ) )
    {
        const Int32 current = m_layout.GetScrollX();
        const Int32 scroll = Clamp( current - wheel.delta * WheelStepX, 0, m_layout.GetMaxScrollX() );
        if ( scroll == current )
            return false;
        return ChangeValueByUser( EWidgetProperty_ScrollPositionX, CORE::Int32ToString( scroll ) );
    }

    Int32 maxScroll = GetMaxScrollRow();
    Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition ) - wheel.delta;
    const Int32 frozen = GetFrozenRowCount();
    if ( scroll < frozen ) scroll = frozen;
    if ( scroll > maxScroll ) scroll = maxScroll;

    // with nothing left to scroll the wheel is for what the grid is in
    if ( scroll == GetIntProperty( EWidgetProperty_ScrollPosition ) )
        return false;
    return ChangeValueByUser( EWidgetProperty_ScrollPosition, CORE::Int32ToString( scroll ) );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( DRAG_NONE != m_dragMode && OnMouseMoved( *pointer ) )
            return true;
        SetIntProperty( EWidgetProperty_HoverIndex, GetRowAt( pointer->localY ) );
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, -1 );
        return true;
    }
    if ( MouseDownEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        if ( GUCEF_NULL != m_editing && m_editing->IsEditing() && m_editing->OnMouseDown( pointer->localX, pointer->localY ) )
            return true;
        return OnMouseDown( *pointer );
    }
    if ( MouseUpEvent == eventId )
        return OnMouseReleased( true ) && m_suppressClick;
    if ( MouseCaptureLostEvent == eventId )
        return OnMouseReleased( false );
    if ( MouseClickEvent == eventId && MOUSEBUTTON_LEFT == static_cast< CPointerEventData* >( &eventData )->button )
        return OnMouseClicked( *static_cast< CPointerEventData* >( &eventData ) );
    if ( KeyDownEvent == eventId )
        return OnKeyPressed( *static_cast< CKeyEventData* >( &eventData ) );
    if ( KeyboardFocusLostEvent == eventId )
    {
        if ( GUCEF_NULL != m_editing )
            m_editing->OnFocusLost();
        return false;
    }
    if ( TextInputEvent == eventId )
    {
        if ( IsCellMode() )
        {
            if ( GetBoolProperty( EWidgetProperty_CellEditing ) )
                return EditController()->OnText( static_cast< CTextEventData* >( &eventData )->text );
            return false;
        }

        // type to find: the next row whose first cell starts with the text
        const CString typed = static_cast< CTextEventData* >( &eventData )->text.Lowercase();
        Int32 count = (Int32) GetRowCount();
        if ( typed.IsNULLOrEmpty() || 0 == count )
            return false;
        Int32 start = GetSelectedRow();
        for ( Int32 step=1; step<=count; ++step )
        {
            Int32 row = ( start + step + count ) % count;
            if ( GetText( 0, (UInt32) row ).Lowercase().StartsWith( typed ) )
            {
                MoveCursorByUser( row, false );
                return true;
            }
        }
        return false;
    }
    if ( MouseWheelEvent == eventId )
        return OnWheel( *static_cast< CWheelEventData* >( &eventData ) );
    return false;
}

/*-------------------------------------------------------------------------*/

CGridEditController*
CGridView::EditController( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_editing )
        m_editing = GUCEF_NEW CGridEditController( *this );
    return m_editing;
}

/*-------------------------------------------------------------------------*/

CGridClipboard*
CGridView::Clipboard( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_clipboard )
        m_clipboard = GUCEF_NEW CGridClipboard( *this );
    return m_clipboard;
}

/*-------------------------------------------------------------------------*/

void
CGridView::ReportEdit( const CORE::CEvent& eventId, const CGridEditReport& report )
{GUCEF_TRACE;

    m_lastReport = report;
    TGridEditReportEventData eventData( report );
    NotifyObservers( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

const CGridEditReport&
CGridView::GetLastEditReport( void ) const
{GUCEF_TRACE;

    return m_lastReport;
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CGridView::ApplyCellEdit( Int32 row, Int32 column, const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( m_model.IsNULL() )
        return CTableEditResult::ReadOnly();
    if ( row < 0 || column < 0 || row >= (Int32) m_model->GetRowCount() || column >= (Int32) m_model->GetColumnCount() )
        return CTableEditResult::Refused( "The cell is not there" );
    const CTableEditResult validation = m_model->ValidateCellEdit( (UInt32) row, (UInt32) column, value );
    if ( !validation.IsAccepted() )
        return validation;
    const CTableEditResult result = m_model->SetCellValue( (UInt32) row, (UInt32) column, value );
    Touch( EWidgetProperty_Rows );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsEditingEnabled( void ) const
{GUCEF_TRACE;

    return !m_model.IsNULL() && IsCellMode() && GetBoolProperty( EWidgetProperty_CellEditing );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsCellEditable( Int32 row, Int32 column ) const
{GUCEF_TRACE;

    return IsEditingEnabled() && row >= 0 && column >= 0 && row < (Int32) m_model->GetRowCount() && column < (Int32) m_model->GetColumnCount() &&
           !m_model->IsCellReadOnly( (UInt32) row, (UInt32) column );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsEditing( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_editing && m_editing->IsEditing();
}

/*-------------------------------------------------------------------------*/

bool
CGridView::BeginEdit( Int32 row, Int32 column )
{GUCEF_TRACE;

    return EditController()->BeginEdit( row, column );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::CommitEdit( void )
{GUCEF_TRACE;

    return GUCEF_NULL == m_editing || m_editing->Commit();
}

/*-------------------------------------------------------------------------*/

void
CGridView::CancelEdit( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_editing )
        m_editing->Cancel();
}

/*-------------------------------------------------------------------------*/

const CGridCellEditor*
CGridView::GetCellEditor( void ) const
{GUCEF_TRACE;

    return IsEditing() ? &m_editing->GetEditor() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CGridEditController*
CGridView::GetEditController( void ) const
{GUCEF_TRACE;

    return m_editing;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnEditKey( CKeyEventData& key )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_editing && m_editing->IsEditing() )
        return m_editing->OnKey( key );
    if ( !GetBoolProperty( EWidgetProperty_CellEditing ) && !GetBoolProperty( EWidgetProperty_CellClipboard ) )
        return false;
    return EditController()->OnKey( key );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::CopySelection( bool cut )
{GUCEF_TRACE;

    return Clipboard()->Copy( cut );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::PasteFromClipboard( void )
{GUCEF_TRACE;

    return Clipboard()->Paste();
}

/*-------------------------------------------------------------------------*/

bool
CGridView::FillSelection( bool down )
{GUCEF_TRACE;

    return Clipboard()->Fill( down );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::ClearSelectedCells( void )
{GUCEF_TRACE;

    return Clipboard()->Clear();
}

/*-------------------------------------------------------------------------*/

const CGridSelection*
CGridView::GetCopyOutline( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_clipboard ? m_clipboard->GetOutline() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGridView::ClearCopyOutline( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_clipboard )
        m_clipboard->ClearOutline();
}

/*-------------------------------------------------------------------------*/

const CString&
CGridView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGridView";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
