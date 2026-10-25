/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

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
}

/*-------------------------------------------------------------------------*/

CGridView::CGridView( void )
    : CWidget( "GridView" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CGridView::~CGridView()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CGridView::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CGridView();
}

/*-------------------------------------------------------------------------*/

UInt32
CGridView::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) GetStringListProperty( EWidgetProperty_Rows ).size();
}

/*-------------------------------------------------------------------------*/

UInt32
CGridView::GetColumnCount( void ) const
{GUCEF_TRACE;

    UInt32 columns = (UInt32) GetStringListProperty( EWidgetProperty_Columns ).size();
    TStringVector rows = GetStringListProperty( EWidgetProperty_Rows );
    for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
    {
        UInt32 cells = (UInt32) rows[ i ].ParseElements( '\t', true ).size();
        if ( cells > columns )
            columns = cells;
    }
    return columns;
}

/*-------------------------------------------------------------------------*/

CGridView::TStringVector
CGridView::GetRowCells( UInt32 rowIndex ) const
{GUCEF_TRACE;

    TStringVector rows = GetStringListProperty( EWidgetProperty_Rows );
    TStringVector cells;
    if ( rowIndex >= (UInt32) rows.size() )
        return cells;
    CString::StringVector parts = rows[ rowIndex ].ParseElements( '\t', true );
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
        cells.push_back( parts[ i ] );
    return cells;
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetText( const UInt32 columnIndex ,
                    const UInt32 rowIndex    ,
                    const CString& text      )
{GUCEF_TRACE;

    TStringVector rows = GetStringListProperty( EWidgetProperty_Rows );
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

    TStringVector cells = GetRowCells( rowIndex );
    return columnIndex < (UInt32) cells.size() ? cells[ columnIndex ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CGridView::AppendText( const UInt32 columnIndex ,
                       const CString& text      )
{GUCEF_TRACE;

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

    SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    SetProperty( EWidgetProperty_SelectedRows, CString::Empty );
    SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    SetProperty( EWidgetProperty_Rows, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SetRows( const TRowVector& rows )
{GUCEF_TRACE;

    TStringVector joined;
    for ( size_t i=0; i<rows.size(); ++i )
        joined.push_back( JoinCells( rows[ i ] ) );
    SetIntProperty( EWidgetProperty_SelectedRow, -1 );
    SetProperty( EWidgetProperty_SelectedRows, CString::Empty );
    SetIntProperty( EWidgetProperty_AnchorRow, -1 );
    SetIntProperty( EWidgetProperty_ScrollPosition, 0 );
    SetStringListProperty( EWidgetProperty_Rows, joined );
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
    SetIntListProperty( EWidgetProperty_SelectedRows, valid );
    SetIntProperty( EWidgetProperty_AnchorRow, valid.empty() ? -1 : valid.front() );
    SetIntProperty( EWidgetProperty_SelectedRow, valid.empty() ? -1 : valid.front() );
}

/*-------------------------------------------------------------------------*/

bool
CGridView::IsRowSelected( Int32 row ) const
{GUCEF_TRACE;

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
    Int32 visible = GetRect().height / rowHeight - 1;
    return visible > 0 ? visible : 0;
}

/*-------------------------------------------------------------------------*/

void
CGridView::ScrollToRow( Int32 row )
{GUCEF_TRACE;

    if ( row < 0 )
        return;
    Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition );
    Int32 visible = GetVisibleRowCount();
    if ( row < scroll )
        scroll = row;
    else
    if ( visible > 0 && row >= scroll + visible )
        scroll = row - visible + 1;
    if ( scroll != GetIntProperty( EWidgetProperty_ScrollPosition ) )
        SetIntProperty( EWidgetProperty_ScrollPosition, scroll );
}

/*-------------------------------------------------------------------------*/

void
CGridView::SortRows( UInt32 column, bool ascending )
{GUCEF_TRACE;

    TStringVector rows = GetStringListProperty( EWidgetProperty_Rows );
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
    SetStringListProperty( EWidgetProperty_Rows, sorted );
    SetIntProperty( EWidgetProperty_SortColumn, (Int32) column );
    SetBoolProperty( EWidgetProperty_SortAscending, ascending );

    // the selection stays with its rows
    TInt32Vector moved;
    for ( size_t i=0; i<selected.size(); ++i )
        moved.push_back( newIndexOf[ selected[ i ] ] );
    SetIntListProperty( EWidgetProperty_SelectedRows, SortedUnique( moved, (Int32) sorted.size() ) );
    SetIntProperty( EWidgetProperty_SelectedRow, cursor >= 0 ? newIndexOf[ cursor ] : -1 );
    Int32 anchor = GetIntProperty( EWidgetProperty_AnchorRow );
    SetIntProperty( EWidgetProperty_AnchorRow, anchor >= 0 && anchor < (Int32) newIndexOf.size() ? newIndexOf[ anchor ] : -1 );
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
CGridView::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    CWidget::OnPropertyChanged( property, oldValue, newValue );
    if ( property == EWidgetProperty_SelectedRow || property == EWidgetProperty_SelectedRows )
        NotifyObservers( SelectionChangedEvent );
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

GUCEF::vector< Int32 >
CGridView::GetColumnWidths( void ) const
{GUCEF_TRACE;

    GUCEF::vector< Int32 > widths = GetIntListProperty( EWidgetProperty_ColumnWidths );
    UInt32 columns = GetColumnCount();
    if ( widths.size() >= columns && !widths.empty() )
    {
        // the widths are relative to each other: the columns fill the grid, whatever its size
        if ( GetBoolProperty( EWidgetProperty_FitColumns ) )
        {
            Int64 total = 0;
            for ( UInt32 i=0; i<columns; ++i )
                total += widths[ i ] > 0 ? widths[ i ] : 0;
            Int32 available = GetRect().width;
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
    Int32 width = GetRect().width / (Int32) columns;
    for ( UInt32 i=0; i<columns; ++i )
        widths.push_back( width );
    return widths;
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
    Int32 row = GetIntProperty( EWidgetProperty_ScrollPosition ) + ( localY - rowHeight ) / rowHeight;
    return row < (Int32) GetRowCount() ? row : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CGridView::GetColumnAt( Int32 localX ) const
{GUCEF_TRACE;

    GUCEF::vector< Int32 > widths = GetColumnWidths();
    Int32 x = 0;
    for ( UInt32 i=0; i<(UInt32)widths.size(); ++i )
    {
        if ( localX >= x && localX < x + widths[ i ] )
            return (Int32) i;
        x += widths[ i ];
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CGridView::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, GetRowAt( static_cast< CPointerEventData* >( &eventData )->localY ) );
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, -1 );
        return true;
    }
    if ( MouseClickEvent == eventId && MOUSEBUTTON_LEFT == static_cast< CPointerEventData* >( &eventData )->button )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        Int32 column = GetColumnAt( pointer->localX );

        // the header: the column was clicked, a sortable grid sorts by it
        if ( pointer->localY >= 0 && pointer->localY < GetRowHeight() )
        {
            if ( column < 0 )
                return false;
            TColumnClickedEventData clickedColumn;
            clickedColumn.GetData().column = (UInt32) column;
            clickedColumn.GetData().row = 0;
            NotifyObservers( ColumnClickedEvent, &clickedColumn );
            if ( GetBoolProperty( EWidgetProperty_Sortable ) )
            {
                bool ascending = column == GetIntProperty( EWidgetProperty_SortColumn ) ? !GetBoolProperty( EWidgetProperty_SortAscending ) : true;
                SortRows( (UInt32) column, ascending );
            }
            return true;
        }

        Int32 row = GetRowAt( pointer->localY );
        if ( row < 0 )
            return false;
        TGridItemClickedEventData clicked;
        clicked.GetData().column = column < 0 ? 0 : (UInt32) column;
        clicked.GetData().row = (UInt32) row;
        NotifyObservers( GridItemClickedEvent, &clicked );
        if ( 2 == pointer->clickCount )
        {
            ActivateItem( row, GetStringListProperty( EWidgetProperty_Rows )[ row ] );
            return true;
        }

        const bool ctrl = 0 != ( pointer->modifiers & INPUT::KEYMOD_CTRL );
        const bool shift = 0 != ( pointer->modifiers & INPUT::KEYMOD_SHIFT );
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
    if ( KeyDownEvent == eventId )
    {
        CKeyEventData* key = static_cast< CKeyEventData* >( &eventData );
        Int32 count = (Int32) GetRowCount();
        Int32 current = GetSelectedRow();
        Int32 page = GetVisibleRowCount() > 1 ? GetVisibleRowCount() - 1 : 1;
        const bool shift = 0 != ( key->modifiers & INPUT::KEYMOD_SHIFT );
        switch ( key->keyCode )
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
                ActivateItem( current, GetStringListProperty( EWidgetProperty_Rows )[ current ] );
                return true;
            }
            default:
                return false;
        }
    }
    if ( TextInputEvent == eventId )
    {
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
    {
        Int32 visible = GetRowHeight() > 0 ? GetRect().height / GetRowHeight() - 1 : 0;
        Int32 maxScroll = (Int32) GetRowCount() - visible;
        if ( maxScroll < 0 ) maxScroll = 0;
        Int32 scroll = GetIntProperty( EWidgetProperty_ScrollPosition ) - static_cast< CWheelEventData* >( &eventData )->delta;
        if ( scroll < 0 ) scroll = 0;
        if ( scroll > maxScroll ) scroll = maxScroll;

        // with nothing left to scroll the wheel is for what the grid is in
        if ( scroll == GetIntProperty( EWidgetProperty_ScrollPosition ) )
            return false;
        return ChangeValueByUser( EWidgetProperty_ScrollPosition, CORE::Int32ToString( scroll ) );
    }
    return false;
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
