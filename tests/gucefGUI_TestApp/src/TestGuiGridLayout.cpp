/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestGridSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GridTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A grid of 800 x 300 over a model of 'rows' rows with 8 columns of 'columnWidth' pixels; 14 rows in view */
GUI::CGridView*
MakeLayoutGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model, Int32 columnWidth )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetModel( model );
    if ( columnWidth > 0 )
    {
        CORE::CString widths;
        for ( UInt32 c=0; c<8; ++c )
            widths += ( c > 0 ? "," : "" ) + CORE::Int32ToString( columnWidth );
        grid->SetProperty( "ColumnWidths", widths );
    }
    context->FlushChanges();
    context->SetFocus( grid );
    return grid;
}

/** Whether a push clip with this rectangle is among the commands */
bool
HasClip( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_PUSHCLIP == commands[ i ].type && commands[ i ].rect == rect )
            return true;
    return false;
}

/** The number of fills of a colour */
UInt32
CountFills( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CColour& colour )
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ i ].type && commands[ i ].colour.r == colour.r && commands[ i ].colour.g == colour.g &&
             commands[ i ].colour.b == colour.b && commands[ i ].colour.a == colour.a )
            ++count;
    return count;
}

/** The lines of a colour, horizontal and vertical */
void
CountLines( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CColour& colour, UInt32& horizontal, UInt32& vertical )
{
    horizontal = 0;
    vertical = 0;
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& c = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_LINE != c.type || c.colour.r != colour.r || c.colour.g != colour.g || c.colour.b != colour.b )
            continue;
        if ( c.y0 == c.y1 )
            ++horizontal;
        else
        if ( c.x0 == c.x1 )
            ++vertical;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridLayout" );

    GUCEF_TESTFW_TESTCASE( "GridLayout: The layout of columns: order, hidden columns, frozen columns, scroll and the place of a point" )
    try
    {
        GUI::CGridColumnLayout layout;
        GUI::CGridColumnLayout::TInt32Vector widths, order, hidden;
        for ( UInt32 c=0; c<6; ++c )
            widths.push_back( 100 );
        widths[ 2 ] = 200;

        // nothing special: the plain row
        layout.Compute( widths, order, hidden, 0, 0, 500, 0 );
        ASSERT_TRUE( 6 == layout.GetPositionCount() && 700 == layout.GetContentWidth() && 200 == layout.GetMaxScrollX() );
        ASSERT_TRUE( 0 == layout.GetScreenLeft( 0 ) && 200 == layout.GetScreenLeft( 2 ) && 400 == layout.GetScreenLeft( 3 ) && 2 == layout.GetColumn( 2 ) && 2 == layout.FindPosition( 2 ) );
        ASSERT_TRUE( 0 == layout.PositionAt( 0 ) && 1 == layout.PositionAt( 199 ) && 2 == layout.PositionAt( 200 ) && 3 == layout.PositionAt( 400 ) && -1 == layout.PositionAt( 600 ) );

        // scrolled: the columns move to the left, the position of a point follows
        layout.Compute( widths, order, hidden, 0, 0, 500, 250 );
        ASSERT_TRUE( 200 == layout.GetScrollX() );                                      // held at the end
        ASSERT_TRUE( -200 == layout.GetScreenLeft( 0 ) );
        ASSERT_TRUE( 2 == layout.PositionAt( 0 ) && 5 == layout.PositionAt( 499 ) );
        Int32 first = 0, last = 0;
        layout.GetVisibleScrolledPositions( first, last );
        ASSERT_TRUE( 2 == first && 5 == last );
        layout.Compute( widths, order, hidden, 0, 0, 500, 120 );
        layout.GetVisibleScrolledPositions( first, last );
        ASSERT_TRUE( 1 == first && 5 == last );
        ASSERT_TRUE( 200 == layout.GetScrollToShow( 5 ) && 0 == layout.GetScrollToShow( 0 ) && 120 == layout.GetScrollToShow( 2 ) );

        // a row header, frozen columns that stay where they are
        layout.Compute( widths, order, hidden, 2, 40, 540, 100 );
        ASSERT_TRUE( 2 == layout.GetFrozenCount() && 200 == layout.GetFrozenWidth() && 40 == layout.GetRowHeaderWidth() );
        ASSERT_TRUE( 40 == layout.GetScreenLeft( 0 ) && 140 == layout.GetScreenLeft( 1 ) );
        ASSERT_TRUE( 40 + 200 - 100 == layout.GetScreenLeft( 2 ) );
        ASSERT_TRUE( 300 == layout.GetScrollViewWidth() );
        ASSERT_TRUE( layout.GetMaxScrollX() == ( 700 - 200 ) - 300 );
        ASSERT_TRUE( -1 == layout.PositionAt( 10 ) && 0 == layout.PositionAt( 50 ) && 1 == layout.PositionAt( 150 ) );
        // just right of the frozen columns: the columns that scroll, from where the scroll has them
        ASSERT_TRUE( 2 == layout.PositionAt( 240 ) && layout.IsFrozen( 1 ) && !layout.IsFrozen( 2 ) );
        // all but one column can freeze
        layout.Compute( widths, order, hidden, 10, 0, 500, 0 );
        ASSERT_TRUE( 5 == layout.GetFrozenCount() );

        // order and hidden columns
        order.push_back( 3 ); order.push_back( 1 );
        hidden.push_back( 0 ); hidden.push_back( 9 );
        layout.Compute( widths, order, hidden, 0, 0, 800, 0 );
        ASSERT_TRUE( 5 == layout.GetPositionCount() && 3 == layout.GetColumn( 0 ) && 1 == layout.GetColumn( 1 ) && 2 == layout.GetColumn( 2 ) && 4 == layout.GetColumn( 3 ) && 5 == layout.GetColumn( 4 ) );
        ASSERT_TRUE( -1 == layout.FindPosition( 0 ) && 0 == layout.FindPosition( 3 ) && 2 == layout.FindPosition( 2 ) );
        ASSERT_TRUE( 0 == layout.GetContentLeft( 0 ) && 100 == layout.GetContentLeft( 1 ) && 200 == layout.GetContentLeft( 2 ) && 400 == layout.GetContentLeft( 3 ) );
        // an order that repeats and names columns that are not there does no harm
        order.clear();
        order.push_back( 4 ); order.push_back( 4 ); order.push_back( 77 ); order.push_back( -3 );
        hidden.clear();
        layout.Compute( widths, order, hidden, 0, 0, 800, 0 );
        ASSERT_TRUE( 6 == layout.GetPositionCount() && 4 == layout.GetColumn( 0 ) && 0 == layout.GetColumn( 1 ) );
        // nothing at all
        layout.Compute( GUI::CGridColumnLayout::TInt32Vector(), order, hidden, 3, 0, 800, 40 );
        ASSERT_TRUE( 0 == layout.GetPositionCount() && -1 == layout.PositionAt( 5 ) && 0 == layout.GetMaxScrollX() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: Wider than the grid, the columns scroll sideways with Shift and the wheel, and the cursor brings its column into view" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 150 );

        // without the option the columns are cut or fit, nothing scrolls sideways
        const GUI::CRect& rect = grid->GetRect();
        context->InjectMouseMove( rect.left + 40, rect.top + 60 );
        PressModifiers( context, INPUT::KEYMOD_SHIFT );
        context->InjectMouseWheel( -1 );
        ReleaseModifiers( context, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == grid->GetIntProperty( "ScrollPositionX" ) && 1 == grid->GetIntProperty( "ScrollPosition" ) );          // the wheel is for the rows, as ever
        grid->SetIntProperty( "ScrollPosition", 0 );

        grid->SetBoolProperty( "HorizontalScroll", true );
        const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
        ASSERT_TRUE( 1200 == layout.GetContentWidth() && 400 == layout.GetMaxScrollX() && 150 == grid->GetColumnWidth( 3 ) );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Id" ) && GUCEF_NULL != FindText( commands, "Stamp" ) && GUCEF_NULL == FindText( commands, "Maybe" ) && GUCEF_NULL == FindText( commands, "Amount" ) );

        // Shift + wheel: 48 pixels a notch, in the direction of the wheel, held at both ends
        PressModifiers( context, INPUT::KEYMOD_SHIFT );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( 48 == grid->GetIntProperty( "ScrollPositionX" ) && 48 == grid->GetColumnLayout().GetScrollX() && 0 == grid->GetIntProperty( "ScrollPosition" ) );
        context->InjectMouseWheel( -20 );
        ASSERT_TRUE( 400 == grid->GetColumnLayout().GetScrollX() );
        context->InjectMouseWheel( 1 );
        ASSERT_TRUE( 352 == grid->GetColumnLayout().GetScrollX() );
        context->InjectMouseWheel( 50 );
        ReleaseModifiers( context, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == grid->GetColumnLayout().GetScrollX() );
        // the wheel without Shift is still for the rows
        context->InjectMouseWheel( -2 );
        ASSERT_TRUE( 2 == grid->GetIntProperty( "ScrollPosition" ) && 0 == grid->GetIntProperty( "ScrollPositionX" ) );
        grid->SetIntProperty( "ScrollPosition", 0 );

        // at the far end: the last columns show, the first do not, a point means the column that is there
        grid->SetIntProperty( "ScrollPositionX", 400 );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Maybe" ) && GUCEF_NULL != FindText( commands, "Amount" ) && GUCEF_NULL == FindText( commands, "Id" ) && GUCEF_NULL == FindText( commands, "Price" ) );
        ASSERT_TRUE( 2 == grid->GetColumnAt( 10 ) && 7 == grid->GetColumnAt( 799 ) && -1 == grid->GetColumnAt( 800 ) );
        GUI::CRect cell;
        ASSERT_TRUE( grid->GetCellRect( 0, 7, cell ) && 650 == cell.left && 150 == cell.width );
        ASSERT_TRUE( grid->GetCellRect( 0, 2, cell ) && 0 == cell.left && 50 == cell.width );          // the part of column 2 that is in view: 300 to 450 less 400 scrolled... cut at the left edge
        ASSERT_TRUE( !grid->GetCellRect( 0, 0, cell ) );

        // the cells are those under the pointer: select in cell mode
        grid->SetProperty( "SelectionMode", "Cell" );
        ASSERT_TRUE( ClickCell( context, grid, 4, 6 ) );
        ASSERT_TRUE( 4 == grid->GetCursorRow() && 6 == grid->GetCursorColumn() && "4,6,4,6" == CORE::CString( grid->GetSelection().ToText()[ 0 ] ) );

        // the cursor: its column comes into view whichever way it goes
        grid->SetIntProperty( "ScrollPositionX", 0 );
        grid->SetCursor( 0, 0 );
        for ( UInt32 i=0; i<7; ++i )
            PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 7 == grid->GetCursorColumn() && 400 == grid->GetColumnLayout().GetScrollX() );
        PressKey( context, INPUT::KEYCODE_HOME );
        ASSERT_TRUE( 0 == grid->GetCursorColumn() && 0 == grid->GetColumnLayout().GetScrollX() );
        grid->ScrollToCell( 30, 5 );
        ASSERT_TRUE( grid->GetColumnLayout().GetScrollX() > 0 && grid->GetIntProperty( "ScrollPosition" ) > 0 && grid->GetCellRect( 30, 5, cell ) );
        ASSERT_TRUE( cell.width == 150 );

        // a scroll position that is too large is held at the end, and switching the option off brings the columns back
        grid->SetIntProperty( "ScrollPositionX", 9999 );
        ASSERT_TRUE( 400 == grid->GetColumnLayout().GetScrollX() );
        grid->SetBoolProperty( "HorizontalScroll", false );
        ASSERT_TRUE( 0 == grid->GetColumnLayout().GetScrollX() && 150 == grid->GetColumnWidth( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: The border of a title resizes the column, a double click fits it to its content, and without the option nothing of the kind happens" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 120 );
        grid->SetBoolProperty( "HorizontalScroll", true );
        CEventCount resized( grid, GUI::CGridView::ColumnResizedEvent );
        CEventCount clicked( grid, GUI::CGridView::ColumnClickedEvent );

        // not allowed: a drag on a border does nothing, the border is just the title (a press and a release on it is a click on the title)
        Int32 x = 0, y = 0;
        ASSERT_TRUE( BorderPoint( context, grid, 1, x, y ) );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_HEADER == grid->HitTest( x - grid->GetRect().left, y - grid->GetRect().top ).kind );
        DragFrom( context, x, y, x + 40, y );
        ASSERT_TRUE( 120 == grid->GetColumnWidth( 1 ) && 0 == resized.count && 1 == clicked.count );

        grid->SetBoolProperty( "ResizableColumns", true );
        GUI::CGridView::THit hit = grid->HitTest( x - grid->GetRect().left, y - grid->GetRect().top );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_BORDER == hit.kind && 1 == hit.column && 1 == hit.position );
        // the left edge of the next title is the same border
        hit = grid->HitTest( x - grid->GetRect().left + 2, y - grid->GetRect().top );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_BORDER == hit.kind && 1 == hit.column );
        // the middle of a title is not
        hit = grid->HitTest( x - grid->GetRect().left - 60, y - grid->GetRect().top );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_HEADER == hit.kind && 1 == hit.column );

        // wider
        DragFrom( context, x, y, x + 30, y );
        ASSERT_TRUE( 150 == grid->GetColumnWidth( 1 ) && resized.count >= 1 );
        ASSERT_TRUE( grid->GetColumnLayout().GetScreenLeft( 2 ) == 120 + 150 && 1 == clicked.count );
        ASSERT_TRUE( "120,150,120,120,120,120,120,120" == grid->GetProperty( "ColumnWidths" ) );
        // the cells of the columns after it moved along
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* nameTitle = FindText( commands, "Name" );
        ASSERT_TRUE( GUCEF_NULL != nameTitle && nameTitle->x0 == 120 + 150 + GUI::CGridView::CELL_PADDING_LEFT );
        // narrower, and not narrower than a column can be
        ASSERT_TRUE( BorderPoint( context, grid, 1, x, y ) );
        DragFrom( context, x, y, x - 100, y );
        ASSERT_TRUE( 50 == grid->GetColumnWidth( 1 ) );
        ASSERT_TRUE( BorderPoint( context, grid, 1, x, y ) );
        DragFrom( context, x, y, x - 300, y );
        ASSERT_TRUE( GUI::CGridView::MIN_COLUMN_WIDTH == grid->GetColumnWidth( 1 ) );
        // the drag did not click the title, a click after it is a click
        ASSERT_TRUE( 1 == clicked.count );
        ASSERT_TRUE( TitlePoint( context, grid, 3, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( 2 == clicked.count );

        // a double click on the border fits the column to what is in view: its title and the cells
        ASSERT_TRUE( BorderPoint( context, grid, 2, x, y ) );
        context->FlushChanges();
        ClickAt( context, x, y, 0, false );
        ClickAt( context, x, y );
        const Int32 expected = 7 * 8 + GUI::CGridView::CELL_PADDING_LEFT + GUI::CGridView::CELL_PADDING_RIGHT;                          // "name 14" is the widest
        ASSERT_TRUE( expected == grid->GetColumnWidth( 2 ) );
        // Id: the title and the sort arrow are wider than the numbers
        ASSERT_TRUE( BorderPoint( context, grid, 0, x, y ) );
        ClickAt( context, x, y, 0, false );
        ClickAt( context, x, y );
        ASSERT_TRUE( 2 * 8 + GUI::CGridView::SORT_ARROW_ROOM + GUI::CGridView::CELL_PADDING_LEFT + GUI::CGridView::CELL_PADDING_RIGHT == grid->GetColumnWidth( 0 ) );
        // and the same by the call
        grid->SetColumnWidth( 5, 300 );
        ASSERT_TRUE( 300 == grid->GetColumnWidth( 5 ) );
        grid->FitColumnToContent( 5 );
        ASSERT_TRUE( 19 * 8 + 10 == grid->GetColumnWidth( 5 ) );                                  // "2023-11-14 22:13:20" and its 19 characters
        grid->SetColumnWidth( 99, 100 );
        ASSERT_TRUE( 0 == grid->GetColumnWidth( 99 ) );

        // the widths are still the widths of the grid after the grid is made again from its properties
        const CORE::CString widths = grid->GetProperty( "ColumnWidths" );
        GUI::CWidget* screen2 = context->CreateWidget( "Panel", "Screen2", GUCEF_NULL );
        GUI::CGridView* other = GUI::CWidget::As< GUI::CGridView >( context->CreateWidget( "GridView", "Other", screen2 ) );
        other->SetModel( model );
        other->SetProperty( "ColumnWidths", widths );
        other->SetBoolProperty( "HorizontalScroll", true );
        context->FlushChanges();
        ASSERT_TRUE( 2 * 8 + GUI::CGridView::SORT_ARROW_ROOM + 10 == other->GetColumnWidth( 0 ) && 24 == other->GetColumnWidth( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: A title dragged to another place moves its column, columns hide and show, the cells follow, and a click on a title that did not move is a click" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 0 );             // fit: 100 pixels each
        CEventCount moved( grid, GUI::CGridView::ColumnMovedEvent );
        CEventCount clicked( grid, GUI::CGridView::ColumnClickedEvent );
        ASSERT_TRUE( 100 == grid->GetColumnWidth( 0 ) );

        // not allowed: nothing moves, the title is clicked
        Int32 x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        ASSERT_TRUE( TitlePoint( context, grid, 1, x0, y0 ) && TitlePoint( context, grid, 4, x1, y1 ) );
        DragFrom( context, x0, y0, x1, y1 );
        ASSERT_TRUE( 0 == moved.count && 1 == grid->GetColumnLayout().FindPosition( 1 ) && 1 == clicked.count );

        grid->SetBoolProperty( "MovableColumns", true );
        // a press that does not move is a click
        ClickAt( context, x0, y0 );
        ASSERT_TRUE( 0 == moved.count && 2 == clicked.count && -1 == grid->GetColumnDropPosition() );

        // during the drag the place is marked
        context->FlushChanges();
        context->InjectMouseMove( x0, y0 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( x1 + 30, y1 );
        ASSERT_TRUE( 5 == grid->GetColumnDropPosition() );                                        // the right half of position 4: after it
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( CountFills( commands, GUI::CColour( 200, 200, 90, 255 ) ) >= 1 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->RenderFrame( 500 );
        ASSERT_TRUE( -1 == grid->GetColumnDropPosition() && 1 == moved.count && 2 == clicked.count );
        ASSERT_TRUE( "0,2,3,4,1,5,6,7" == grid->GetProperty( "ColumnOrder" ) );
        const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
        ASSERT_TRUE( 4 == layout.FindPosition( 1 ) && 1 == layout.GetColumn( 0 + 4 ) && 2 == layout.GetColumn( 1 ) );
        commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* priceTitle = FindText( commands, "Price" );
        ASSERT_TRUE( GUCEF_NULL != priceTitle && priceTitle->x0 == 400 + 100 - GUI::CGridView::CELL_PADDING_RIGHT - TextWidth( context, "Price" ) );        // numbers and their titles end at the right of the column
        const GUI::CDrawSurface::TCommand* nameCell = FindText( commands, "name 3" );
        ASSERT_TRUE( GUCEF_NULL != nameCell && nameCell->x0 == 100 + GUI::CGridView::CELL_PADDING_LEFT );

        // the data column under a point, and a cell in cell mode, are the data column that is there
        grid->SetProperty( "SelectionMode", "Cell" );
        ASSERT_TRUE( 1 == grid->GetColumnAt( 450 ) && 2 == grid->GetColumnAt( 150 ) );
        ASSERT_TRUE( ClickCell( context, grid, 2, 1 ) );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() && "2,4,2,4" == CORE::CString( grid->GetSelection().ToText()[ 0 ] ) );

        // to the far left and the far right
        ASSERT_TRUE( TitlePoint( context, grid, 5, x0, y0 ) );
        DragFrom( context, x0, y0, grid->GetRect().left + 5, y0 );
        ASSERT_TRUE( 0 == grid->GetColumnLayout().FindPosition( 5 ) && "5,0,2,3,4,1,6,7" == grid->GetProperty( "ColumnOrder" ) );
        ASSERT_TRUE( TitlePoint( context, grid, 5, x0, y0 ) );
        DragFrom( context, x0, y0, grid->GetRect().left + 799, y0 );
        ASSERT_TRUE( 7 == grid->GetColumnLayout().FindPosition( 5 ) && 0 == grid->GetColumnLayout().GetColumn( 0 ) );
        // the same by the call
        grid->MoveColumn( 7, 0 );
        ASSERT_TRUE( 0 == grid->GetColumnLayout().FindPosition( 7 ) && 1 == grid->GetColumnLayout().FindPosition( 0 ) );

        // hide and show: the column goes, the others close the gap, the place of the column is kept
        CEventCount visibility( grid, GUI::CGridView::ColumnVisibilityChangedEvent );
        grid->SetProperty( "ColumnOrder", "" );
        grid->SetColumnHidden( 2, true );
        ASSERT_TRUE( grid->IsColumnHidden( 2 ) && 7 == grid->GetColumnLayout().GetPositionCount() && -1 == grid->GetColumnLayout().FindPosition( 2 ) && "2" == grid->GetProperty( "HiddenColumns" ) );
        ASSERT_TRUE( 1 == visibility.count && 3 == grid->GetColumnLayout().GetColumn( 2 ) );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "Name" ) && GUCEF_NULL == FindText( commands, "name 3" ) && GUCEF_NULL != FindText( commands, "Flag" ) );
        ASSERT_TRUE( 3 == grid->GetColumnAt( 250 ) );
        grid->SetColumnHidden( 2, true );                                                         // already hidden: nothing
        ASSERT_TRUE( 1 == visibility.count );
        grid->MoveColumn( 0, 3 );
        grid->SetColumnHidden( 2, false );
        ASSERT_TRUE( !grid->IsColumnHidden( 2 ) && 8 == grid->GetColumnLayout().GetPositionCount() && 2 == visibility.count && grid->GetProperty( "HiddenColumns" ).IsNULLOrEmpty() );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Name" ) );
        // the cursor on a column that is hidden moves to the first shown
        grid->SetCursor( 1, 4 );
        grid->SetColumnHidden( 4, true );
        ASSERT_TRUE( 1 == grid->GetCursorRow() && grid->GetCursorColumn() == grid->GetColumnLayout().GetColumn( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: Frozen columns and rows stay where they are while the rest scrolls under them" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 150 );
        grid->SetBoolProperty( "HorizontalScroll", true );
        grid->SetIntProperty( "FrozenColumns", 2 );
        grid->SetIntProperty( "FrozenRows", 2 );
        grid->SetIntProperty( "ScrollPositionX", 200 );
        grid->SetIntProperty( "ScrollPosition", 10 );
        ASSERT_TRUE( 2 == grid->GetFrozenRowCount() && 10 == grid->GetFirstScrolledRow() && 300 == grid->GetColumnLayout().GetFrozenWidth() );
        ASSERT_TRUE( 1000 - 12 == grid->GetMaxScrollRow() );

        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* id = FindText( commands, "Id" );
        const GUI::CDrawSurface::TCommand* price = FindText( commands, "Price" );
        ASSERT_TRUE( GUCEF_NULL != id && id->x0 == 150 - GUI::CGridView::CELL_PADDING_RIGHT - TextWidth( context, "Id" ) && GUCEF_NULL != price && price->x0 == 300 - GUI::CGridView::CELL_PADDING_RIGHT - TextWidth( context, "Price" ) );
        // the columns under the frozen ones are not drawn there; the next one is cut off where the frozen ones end
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "Name" ) && GUCEF_NULL != FindText( commands, "Flag" ) );
        ASSERT_TRUE( HasClip( commands, GUI::CRect( 300, 0, 150 - 50, 20 ) ) );                    // the title of column 3: 450 less 200 scrolled = 250, cut at 300

        // the rows: the frozen ones first, then the ones that scroll
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "name 0" ) );                               // column 2 is under the frozen columns
        const GUI::CDrawSurface::TCommand* first = FindText( commands, "0" );
        const GUI::CDrawSurface::TCommand* second = FindText( commands, "1" );
        const GUI::CDrawSurface::TCommand* scrolled = FindText( commands, "10" );
        ASSERT_TRUE( GUCEF_NULL != first && 20 == first->y0 && GUCEF_NULL != second && 40 == second->y0 && GUCEF_NULL != scrolled && 60 == scrolled->y0 );
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "5" ) || FindText( commands, "5" )->y0 != 100 );
        ASSERT_TRUE( 0 == grid->GetRowAt( 30 ) && 1 == grid->GetRowAt( 50 ) && 10 == grid->GetRowAt( 70 ) && 11 == grid->GetRowAt( 90 ) );
        ASSERT_TRUE( 20 == grid->GetRowTop( 0 ) && 40 == grid->GetRowTop( 1 ) && 60 == grid->GetRowTop( 10 ) && -1 == grid->GetRowTop( 5 ) );

        // the wheel moves the rows that scroll
        const GUI::CRect& rect = grid->GetRect();
        context->InjectMouseMove( rect.left + 40, rect.top + 100 );
        context->InjectMouseWheel( -3 );
        ASSERT_TRUE( 13 == grid->GetIntProperty( "ScrollPosition" ) && 13 == grid->GetRowAt( 70 ) && 0 == grid->GetRowAt( 30 ) );
        context->InjectMouseWheel( 50 );
        ASSERT_TRUE( 2 == grid->GetIntProperty( "ScrollPosition" ) && 2 == grid->GetRowAt( 70 ) );                                  // not above the frozen rows
        // a row above the first that scrolls comes into view
        grid->SetIntProperty( "ScrollPosition", 10 );
        grid->ScrollToRow( 5 );
        ASSERT_TRUE( 5 == grid->GetIntProperty( "ScrollPosition" ) );
        grid->ScrollToRow( 1 );
        ASSERT_TRUE( 5 == grid->GetIntProperty( "ScrollPosition" ) );                                                               // a frozen row is in view
        grid->ScrollToRow( 100 );
        ASSERT_TRUE( 100 - 12 + 1 == grid->GetIntProperty( "ScrollPosition" ) );

        // a click hits the right row and the right column
        grid->SetProperty( "SelectionMode", "Cell" );
        grid->SetIntProperty( "ScrollPosition", 10 );
        ASSERT_TRUE( ClickCell( context, grid, 1, 1 ) && 1 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
        ASSERT_TRUE( ClickCell( context, grid, 12, 4 ) && 12 == grid->GetCursorRow() && 4 == grid->GetCursorColumn() );
        GUI::CRect cell;
        ASSERT_TRUE( grid->GetCellRect( 12, 2, cell ) == false || cell.left >= 300 );
        ASSERT_TRUE( !grid->GetCellRect( 5, 4, cell ) );

        // more frozen rows than fit: at least one row scrolls
        grid->SetIntProperty( "FrozenRows", 100 );
        ASSERT_TRUE( 13 == grid->GetFrozenRowCount() );
        grid->SetIntProperty( "FrozenRows", 0 );
        grid->SetIntProperty( "FrozenColumns", 0 );
        ASSERT_TRUE( 0 == grid->GetFrozenRowCount() && 10 == grid->GetFirstScrolledRow() && 0 == grid->GetColumnLayout().GetFrozenCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: Zebra stripes, grid lines, the footer row of the model and the scroll indicators are there when asked for, and only then" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 1000 );
        raw->footer = false;
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 0 );
        const GUI::CColour zebra( 10, 20, 30, 255 );
        const GUI::CColour lineColour( 70, 70, 76, 255 );
        const GUI::CColour thumbColour( 255, 255, 255, 110 );

        // nothing asked for: none of it
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        UInt32 horizontal = 0, vertical = 0;
        CountLines( commands, lineColour, horizontal, vertical );
        ASSERT_TRUE( 0 == CountFills( commands, zebra ) && 0 == horizontal && 0 == vertical && GUCEF_NULL == FindText( commands, "Total" ) && 0 == CountFills( commands, thumbColour ) );
        ASSERT_TRUE( -1 == grid->GetFooterTop() && 14 == grid->GetVisibleRowCount() && 0 == grid->GetRowHeaderWidth() );

        // zebra: every second row
        grid->SetProperty( "ZebraColour", "10,20,30,255" );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 0, grid->GetRowTop( 1 ), 800, 20 ), GUCEF_NULL ) && HasFill( commands, GUI::CRect( 0, grid->GetRowTop( 3 ), 800, 20 ), GUCEF_NULL ) );
        ASSERT_TRUE( CountFills( commands, zebra ) >= 7 && CountFills( commands, zebra ) <= 8 );
        grid->SetIntProperty( "ScrollPosition", 3 );                                              // the stripes belong to the rows
        commands = PaintedCommands( context, grid );
        GUI::CColour found;
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 0, grid->GetRowTop( 3 ), 800, 20 ), &found ) && 10 == found.r );
        ASSERT_TRUE( !( HasFill( commands, GUI::CRect( 0, grid->GetRowTop( 4 ), 800, 20 ), &found ) && 10 == found.r ) );
        grid->SetIntProperty( "ScrollPosition", 0 );
        grid->ResetProperty( "ZebraColour" );

        // grid lines
        grid->SetProperty( "GridLines", "Horizontal" );
        commands = PaintedCommands( context, grid );
        CountLines( commands, lineColour, horizontal, vertical );
        ASSERT_TRUE( 14 == horizontal && 0 == vertical );
        grid->SetProperty( "GridLines", "Vertical" );
        commands = PaintedCommands( context, grid );
        CountLines( commands, lineColour, horizontal, vertical );
        ASSERT_TRUE( 0 == horizontal && 8 == vertical );
        grid->SetProperty( "GridLines", "Both" );
        grid->SetProperty( "GridColour", "255,0,0,255" );
        commands = PaintedCommands( context, grid );
        CountLines( commands, GUI::CColour( 255, 0, 0, 255 ), horizontal, vertical );
        ASSERT_TRUE( 14 == horizontal && 8 == vertical );
        grid->SetProperty( "GridLines", "None" );

        // the footer: the rows of the model, the totals at the bottom, one row less for the rows; a model that has a footer shows it with no ShowFooter set,
        // ShowFooter set to true is the same, and set to false hides it
        raw->footer = true;
        ASSERT_TRUE( !grid->IsPropertySet( "ShowFooter" ) && 280 == grid->GetFooterTop() && 13 == grid->GetVisibleRowCount() );
        grid->SetBoolProperty( "ShowFooter", true );
        ASSERT_TRUE( 280 == grid->GetFooterTop() && 13 == grid->GetVisibleRowCount() );
        commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* total = FindText( commands, "Total" );
        const GUI::CDrawSurface::TCommand* count = FindText( commands, "1,000" );
        ASSERT_TRUE( GUCEF_NULL != total && 280 == total->y0 && GUCEF_NULL != count && 280 == count->y0 );
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "name 13" ) && GUCEF_NULL != FindText( commands, "name 12" ) );
        ASSERT_TRUE( -1 == grid->GetRowAt( 285 ) && 12 == grid->GetRowAt( 265 ) && GUI::CGridView::HIT_FOOTER == grid->HitTest( 10, 290 ).kind );
        ASSERT_TRUE( 1000 - 13 == grid->GetMaxScrollRow() );
        // a model without a footer shows none
        raw->footer = false;
        ASSERT_TRUE( -1 == grid->GetFooterTop() && 14 == grid->GetVisibleRowCount() );
        raw->footer = true;
        grid->SetBoolProperty( "ShowFooter", false );
        ASSERT_TRUE( -1 == grid->GetFooterTop() );

        // the scroll indicators: a thumb where the rows in view are among all rows
        grid->SetBoolProperty( "ScrollBars", true );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 800 - 6, 20, 6, 280 ), GUCEF_NULL ) && CountFills( commands, thumbColour ) == 1 );
        UInt32 thumbTop = 0;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ i ].type && commands[ i ].colour.a == 110 )
                thumbTop = (UInt32) commands[ i ].rect.top;
        ASSERT_TRUE( 20 == thumbTop );
        grid->SetIntProperty( "ScrollPosition", grid->GetMaxScrollRow() );
        commands = PaintedCommands( context, grid );
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_FILLRECT == commands[ i ].type && commands[ i ].colour.a == 110 )
                ASSERT_TRUE( commands[ i ].rect.Bottom() == 300 );
        // with columns that scroll there is one at the bottom as well
        grid->SetBoolProperty( "HorizontalScroll", true );
        grid->SetProperty( "ColumnWidths", "200,200,200,200,200,200,200,200" );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 0, 300 - 6, 800, 6 ), GUCEF_NULL ) && 2 == CountFills( commands, thumbColour ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: The row header numbers the rows, or gives the titles of the model, and its cells are the place to click for a row" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 1000 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeLayoutGrid( context, model, 0 );
        grid->SetBoolProperty( "ShowRowHeader", true );
        const Int32 headerWidth = grid->GetRowHeaderWidth();
        ASSERT_TRUE( headerWidth == 4 * 8 + 12 );
        // the columns share the room that is left
        ASSERT_TRUE( (800 - headerWidth) / 8 == grid->GetColumnWidth( 0 ) && grid->GetColumnLayout().GetScreenLeft( 0 ) == headerWidth );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* one = FindText( commands, "1" );
        ASSERT_TRUE( GUCEF_NULL != one && 20 == one->y0 && one->x0 + 8 == headerWidth - 6 );        // numbers end at the padding at the right
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "14" ) );
        // the first column starts after the header
        const GUI::CDrawSurface::TCommand* title = FindText( commands, "Id" );
        ASSERT_TRUE( GUCEF_NULL != title && title->x0 == headerWidth + grid->GetColumnWidth( 0 ) - GUI::CGridView::CELL_PADDING_RIGHT - TextWidth( context, "Id" ) );
        ASSERT_TRUE( GUI::CGridView::HIT_ROW_HEADER == grid->HitTest( 5, 45 ).kind && 1 == grid->HitTest( 5, 45 ).row && GUI::CGridView::HIT_CORNER == grid->HitTest( 5, 5 ).kind && -1 == grid->GetColumnAt( 5 ) );

        // the click on the number of a row of a grid in row mode selects the row
        ClickAt( context, grid->GetRect().left + 5, grid->GetRect().top + 45 );
        ASSERT_TRUE( 1 == grid->GetSelectedRow() );

        // the titles of the model; the header is as wide as the widest in view
        raw->titles = true;
        grid->SetProperty( "Font", "Test" );
        grid->SetBoolProperty( "ShowRowHeader", false );
        grid->SetBoolProperty( "ShowRowHeader", true );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "R3" ) && GUCEF_NULL != FindText( commands, "R13" ) );
        ASSERT_TRUE( grid->GetRowHeaderWidth() == 4 * 8 + 12 || grid->GetRowHeaderWidth() == 3 * 8 + 12 );
        grid->SetIntProperty( "RowHeaderWidth", 90 );
        ASSERT_TRUE( 90 == grid->GetRowHeaderWidth() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridLayout: Every option is off by default and the properties of them are registered with their defaults" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
        static const char* const off[] = { "ShowRowHeader", "HorizontalScroll", "ResizableColumns", "MovableColumns", "ShowFooter", "ScrollBars" };
        for ( UInt32 i=0; i<6; ++i )
            ASSERT_TRUE( !grid->GetBoolProperty( off[ i ] ) );
        ASSERT_TRUE( "Row" == grid->GetProperty( "SelectionMode" ) && "None" == grid->GetProperty( "GridLines" ) && grid->GetProperty( "ZebraColour" ).IsNULLOrEmpty() );
        ASSERT_TRUE( 0 == grid->GetIntProperty( "FrozenColumns" ) && 0 == grid->GetIntProperty( "FrozenRows" ) && 0 == grid->GetIntProperty( "ScrollPositionX" ) && 0 == grid->GetIntProperty( "RowHeaderWidth" ) );
        ASSERT_TRUE( grid->GetProperty( "ColumnOrder" ).IsNULLOrEmpty() && grid->GetProperty( "HiddenColumns" ).IsNULLOrEmpty() && -1 == grid->GetIntProperty( "SelectedColumn" ) && grid->GetProperty( "SelectedRanges" ).IsNULLOrEmpty() );
        ASSERT_TRUE( !grid->HasModel() && !grid->IsCellMode() && -1 == grid->GetColumnDropPosition() );

        // the registry describes them
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        static const char* const names[] = { "SelectionMode", "SelectedColumn", "SelectedRanges", "ShowRowHeader", "RowHeaderWidth", "HorizontalScroll", "ScrollPositionX", "ResizableColumns", "MovableColumns",
                                             "ColumnOrder", "HiddenColumns", "FrozenColumns", "FrozenRows", "ShowFooter", "ScrollBars", "ZebraColour", "GridLines", "GridColour" };
        for ( UInt32 i=0; i<18; ++i )
            ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "GridView", names[ i ] ) );
        CORE::CString normalized;
        ASSERT_TRUE( registry.FindProperty( "GridView", "SelectionMode" )->Validate( "Cell", normalized ) && !registry.FindProperty( "GridView", "SelectionMode" )->Validate( "Block", normalized ) );
        ASSERT_TRUE( registry.FindProperty( "GridView", "GridLines" )->Validate( "Both", normalized ) && !registry.FindProperty( "GridView", "GridLines" )->Validate( "Dotted", normalized ) );

        // the border of a title is a title; Shift + wheel is the wheel; Ctrl + A is nothing; without columns nothing breaks
        grid->SetProperty( "Columns", "A\nB\nC" );
        context->FlushChanges();
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_HEADER == grid->HitTest( 266, 5 ).kind );
        GUI::CGridView::THit hit = grid->HitTest( 790, 100 );
        ASSERT_TRUE( GUI::CGridView::HIT_NONE == hit.kind );
        GUI::CWidget* empty = context->CreateWidget( "GridView", "Empty", screen );
        context->FlushChanges();
        GUI::CGridView* emptyGrid = GUI::CWidget::As< GUI::CGridView >( empty );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, emptyGrid );
        ASSERT_TRUE( 0 == emptyGrid->GetRowCount() && 0 == emptyGrid->GetColumnCount() );
        emptyGrid->SetProperty( "SelectionMode", "Cell" );
        emptyGrid->SelectAll();
        emptyGrid->SetCursor( 0, 0 );
        ASSERT_TRUE( emptyGrid->GetSelection().IsEmpty() && -1 == emptyGrid->GetCursorRow() );
        emptyGrid->SetBoolProperty( "ShowRowHeader", true );
        emptyGrid->SetBoolProperty( "HorizontalScroll", true );
        emptyGrid->SetIntProperty( "FrozenColumns", 3 );
        commands = PaintedCommands( context, emptyGrid );
        ASSERT_TRUE( 0 == emptyGrid->GetColumnLayout().GetPositionCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
