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

/** A grid of 800 x 300 in cell mode over a model; 14 rows in view, 100 pixels per column */
GUI::CGridView*
MakeCellGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model, bool rowHeader )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetModel( model );
    grid->SetProperty( "SelectionMode", "Cell" );
    if ( rowHeader )
        grid->SetBoolProperty( "ShowRowHeader", true );
    context->FlushChanges();
    context->SetFocus( grid );
    return grid;
}

/** The ranges of the selection as text: "firstRow,firstColumn,lastRow,lastColumn" for each, separated by a semicolon */
CORE::CString
Ranges( const GUI::CGridView* grid )
{
    CORE::CString text;
    const GUCEF::vector< CORE::CString > lines = grid->GetSelection().ToText();
    for ( size_t i=0; i<lines.size(); ++i )
    {
        if ( i > 0 )
            text += ';';
        text += lines[ i ];
    }
    return text;
}

/** A grid of text with holes, in the string list, in cell mode */
GUI::CGridView*
MakeHolesGrid( GUI::CGUIContext* context )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetProperty( "SelectionMode", "Cell" );
    grid->SetProperty( "Columns", "A\nB\nC\nD\nE\nF" );
    grid->SetProperty( "ColumnWidths", "100,100,100,100,100,100" );
    const char* rows[ 4 ][ 6 ] = { { "a", "b", "c", "", "e", "f" },
                                   { "", "", "", "", "", "" },
                                   { "g", "", "h", "i", "j", "k" },
                                   { "l", "m", "n", "o", "p", "q" } };
    GUI::CGridView::TRowVector data;
    for ( UInt32 r=0; r<4; ++r )
    {
        GUI::CGridView::TStringVector cells;
        for ( UInt32 c=0; c<6; ++c )
            cells.push_back( rows[ r ][ c ] );
        data.push_back( cells );
    }
    grid->SetRows( data );
    context->FlushChanges();
    context->SetFocus( grid );
    return grid;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridSelectionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridSelection" );

    GUCEF_TESTFW_TESTCASE( "GridSelection: A click selects a cell, Shift + click and a drag a block, Ctrl + click another block, and the events tell" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeCellGrid( context, model, false );
        ASSERT_TRUE( grid->IsCellMode() && -1 == grid->GetCursorRow() && grid->GetSelection().IsEmpty() );
        CEventCount cursorEvents( grid, GUI::CGridView::CellCursorChangedEvent );
        CEventCount selectionEvents( grid, GUI::CGridView::CellSelectionChangedEvent );
        CEventCount rowEvents( grid, GUI::CGridView::SelectionChangedEvent );
        CEventCount valueEvents( grid, GUI::CWidget::ValueChangedEvent );

        // a click: the cell, the cursor, the rows and columns of the properties
        ASSERT_TRUE( ClickCell( context, grid, 3, 2 ) );
        ASSERT_TRUE( 3 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() && "3,2,3,2" == Ranges( grid ) );
        ASSERT_TRUE( 3 == grid->GetSelectedRow() && 2 == grid->GetIntProperty( "SelectedColumn" ) );
        ASSERT_TRUE( "3,2,3,2" == grid->GetProperty( "SelectedRanges" ) );
        ASSERT_TRUE( grid->IsCellSelected( 3, 2 ) && !grid->IsCellSelected( 3, 3 ) && !grid->IsCellSelected( 4, 2 ) );
        ASSERT_TRUE( 1 == cursorEvents.count && 1 == selectionEvents.count && 1 == rowEvents.count && valueEvents.count >= 1 );

        // Shift + click: the block from the anchor, the cursor stays
        ASSERT_TRUE( ClickCell( context, grid, 6, 4, INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( "3,2,6,4" == Ranges( grid ) && 3 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() );
        ASSERT_TRUE( grid->IsCellSelected( 5, 3 ) && grid->IsCellSelected( 6, 4 ) && grid->IsCellSelected( 3, 2 ) && !grid->IsCellSelected( 7, 3 ) && !grid->IsCellSelected( 5, 5 ) && !grid->IsCellSelected( 2, 2 ) );
        ASSERT_TRUE( 1 == cursorEvents.count && 2 == selectionEvents.count );
        // the other way round
        ASSERT_TRUE( ClickCell( context, grid, 1, 0, INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( "1,0,3,2" == Ranges( grid ) );

        // Ctrl + click: a block of its own; the cursor moves to it; the first block stays
        ASSERT_TRUE( ClickCell( context, grid, 10, 1, INPUT::KEYMOD_CTRL ) );
        ASSERT_TRUE( "1,0,3,2;10,1,10,1" == Ranges( grid ) && 10 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
        ASSERT_TRUE( grid->IsCellSelected( 2, 1 ) && grid->IsCellSelected( 10, 1 ) && !grid->IsCellSelected( 10, 2 ) );
        // Shift + click goes on from the new block
        ASSERT_TRUE( ClickCell( context, grid, 12, 3, INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( "1,0,3,2;10,1,12,3" == Ranges( grid ) );
        // a plain click is one cell again
        ASSERT_TRUE( ClickCell( context, grid, 7, 7 ) );
        ASSERT_TRUE( "7,7,7,7" == Ranges( grid ) );

        // a drag
        Int32 x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        ASSERT_TRUE( CellPoint( context, grid, 2, 1, x0, y0 ) && CellPoint( context, grid, 5, 3, x1, y1 ) );
        DragFrom( context, x0, y0, x1, y1 );
        ASSERT_TRUE( "2,1,5,3" == Ranges( grid ) && 2 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
        // and the other way, with a click that comes after the drag not undoing it
        ASSERT_TRUE( CellPoint( context, grid, 8, 6, x0, y0 ) && CellPoint( context, grid, 6, 4, x1, y1 ) );
        DragFrom( context, x0, y0, x1, y1 );
        ASSERT_TRUE( "6,4,8,6" == Ranges( grid ) && 8 == grid->GetCursorRow() && 6 == grid->GetCursorColumn() );

        // a drag below the grid pulls the view along
        ASSERT_TRUE( CellPoint( context, grid, 2, 1, x0, y0 ) );
        DragFrom( context, x0, y0, x0 + 5, grid->GetRect().top + grid->GetRect().height + 60 );
        ASSERT_TRUE( grid->GetIntProperty( "ScrollPosition" ) >= 1 );
        ASSERT_TRUE( 2 == grid->GetSelection().GetRange( 0 ).firstRow && grid->GetSelection().GetRange( 0 ).lastRow >= 14 && 1 == grid->GetSelection().GetRange( 0 ).firstColumn );

        // the selection set from outside
        grid->SetProperty( "SelectedRanges", "4,2,6,5\n20,0,21,0" );
        ASSERT_TRUE( "4,2,6,5;20,0,21,0" == Ranges( grid ) && grid->IsCellSelected( 5, 4 ) && grid->IsCellSelected( 21, 0 ) );
        grid->SelectRange( 1, 1, 2, 2 );
        ASSERT_TRUE( "1,1,2,2" == Ranges( grid ) && 1 == grid->GetCursorRow() );
        grid->SetCursor( 9, 3 );
        ASSERT_TRUE( "9,3,9,3" == Ranges( grid ) && 9 == grid->GetSelectedRow() );
        grid->ClearSelection();
        ASSERT_TRUE( grid->GetSelection().IsEmpty() && -1 == grid->GetCursorRow() && -1 == grid->GetSelectedRow() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: The row header selects whole rows, the column header whole columns, the corner everything; the sort of a sortable header stays" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 1000 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeCellGrid( context, model, true );
        const Int32 headerWidth = grid->GetRowHeaderWidth();
        ASSERT_TRUE( headerWidth > 0 && headerWidth < 80 );
        CEventCount clickedColumns( grid, GUI::CGridView::ColumnClickedEvent );

        // a row
        Int32 x = 0, y = 0;
        ASSERT_TRUE( RowHeaderPoint( context, grid, 4, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( "4,0,4,-1" == Ranges( grid ) && 4 == grid->GetCursorRow() );
        ASSERT_TRUE( grid->IsCellSelected( 4, 0 ) && grid->IsCellSelected( 4, 7 ) && !grid->IsCellSelected( 5, 0 ) && grid->GetSelection().ContainsWholeRow( 4 ) );
        // Shift + click: the rows from the anchor
        ASSERT_TRUE( RowHeaderPoint( context, grid, 8, x, y ) );
        ClickAt( context, x, y, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "4,0,8,-1" == Ranges( grid ) );
        // Ctrl + click: other rows as well
        ASSERT_TRUE( RowHeaderPoint( context, grid, 12, x, y ) );
        ClickAt( context, x, y, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "4,0,8,-1;12,0,12,-1" == Ranges( grid ) && grid->IsCellSelected( 6, 3 ) && grid->IsCellSelected( 12, 5 ) && !grid->IsCellSelected( 10, 5 ) );
        // a drag over the row header
        Int32 x1 = 0, y1 = 0;
        ASSERT_TRUE( RowHeaderPoint( context, grid, 2, x, y ) && RowHeaderPoint( context, grid, 6, x1, y1 ) );
        DragFrom( context, x, y, x1, y1 );
        ASSERT_TRUE( "2,0,6,-1" == Ranges( grid ) );

        // a column: the title is clicked, the event is raised first
        ASSERT_TRUE( TitlePoint( context, grid, 2, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( "0,2,-1,2" == Ranges( grid ) && 1 == clickedColumns.count && grid->IsCellSelected( 0, 2 ) && grid->IsCellSelected( 900, 2 ) && !grid->IsCellSelected( 5, 3 ) );
        ASSERT_TRUE( grid->GetSelection().ContainsWholeColumn( 2 ) );
        ASSERT_TRUE( TitlePoint( context, grid, 4, x, y ) );
        ClickAt( context, x, y, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "0,2,-1,4" == Ranges( grid ) );
        ASSERT_TRUE( TitlePoint( context, grid, 6, x, y ) );
        ClickAt( context, x, y, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "0,2,-1,4;0,6,-1,6" == Ranges( grid ) );

        // the corner: everything
        context->FlushChanges();
        ClickAt( context, grid->GetRect().left + 3, grid->GetRect().top + 3 );
        ASSERT_TRUE( "0,0,-1,-1" == Ranges( grid ) && grid->IsCellSelected( 999, 7 ) && grid->IsCellSelected( 0, 0 ) );

        // a sortable grid: the title sorts, as it always did, and selects nothing
        raw->SetSortable( true );
        grid->SetProperty( "Sortable", "true" );
        grid->ClearSelection();
        ASSERT_TRUE( TitlePoint( context, grid, 1, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( 1 == grid->GetIntProperty( "SortColumn" ) && grid->GetSelection().IsEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: The arrow keys move the cursor and with Shift extend the block, Home, End, Page Up and Page Down, Ctrl + Home and End, Ctrl + A" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 1000 ) );
        GUI::CGridView* grid = MakeCellGrid( context, model, false );
        grid->SetCursor( 5, 3 );
        ASSERT_TRUE( "5,3,5,3" == Ranges( grid ) );

        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 6 == grid->GetCursorRow() && 3 == grid->GetCursorColumn() && "6,3,6,3" == Ranges( grid ) );
        PressKey( context, INPUT::KEYCODE_UP );
        PressKey( context, INPUT::KEYCODE_UP );
        ASSERT_TRUE( 4 == grid->GetCursorRow() );
        PressKey( context, INPUT::KEYCODE_LEFT );
        PressKey( context, INPUT::KEYCODE_LEFT );
        ASSERT_TRUE( 1 == grid->GetCursorColumn() );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 2 == grid->GetCursorColumn() && 4 == grid->GetCursorRow() );
        // the edges stop the cursor
        PressKey( context, INPUT::KEYCODE_HOME );
        ASSERT_TRUE( 0 == grid->GetCursorColumn() && 4 == grid->GetCursorRow() );
        PressKey( context, INPUT::KEYCODE_LEFT );
        ASSERT_TRUE( 0 == grid->GetCursorColumn() );
        PressKey( context, INPUT::KEYCODE_END );
        ASSERT_TRUE( 7 == grid->GetCursorColumn() && 4 == grid->GetCursorRow() );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 7 == grid->GetCursorColumn() );

        // the pages: one less than the rows in view
        PressKey( context, INPUT::KEYCODE_PGDOWN );
        ASSERT_TRUE( 17 == grid->GetCursorRow() );
        PressKey( context, INPUT::KEYCODE_PGUP );
        PressKey( context, INPUT::KEYCODE_PGUP );
        ASSERT_TRUE( 0 == grid->GetCursorRow() );

        // Ctrl + Home and End; the cell is brought into view
        Chord( context, INPUT::KEYCODE_END, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 999 == grid->GetCursorRow() && 7 == grid->GetCursorColumn() && grid->GetIntProperty( "ScrollPosition" ) == grid->GetMaxScrollRow() );
        CORE::CString text = Ranges( grid );
        ASSERT_TRUE( "999,7,999,7" == text );
        Chord( context, INPUT::KEYCODE_HOME, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == grid->GetCursorRow() && 0 == grid->GetCursorColumn() && 0 == grid->GetIntProperty( "ScrollPosition" ) );

        // Shift extends the block from the anchor; the cursor stays
        grid->SetCursor( 5, 3 );
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_SHIFT );
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_SHIFT );
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "5,3,7,4" == Ranges( grid ) && 5 == grid->GetCursorRow() && 3 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_UP, INPUT::KEYMOD_SHIFT );
        Chord( context, INPUT::KEYCODE_UP, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "5,3,5,4" == Ranges( grid ) );
        Chord( context, INPUT::KEYCODE_UP, INPUT::KEYMOD_SHIFT );
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_SHIFT );
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "4,2,5,3" == Ranges( grid ) );
        Chord( context, INPUT::KEYCODE_HOME, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "4,0,5,3" == Ranges( grid ) );
        // a plain arrow collapses the block to the cursor moved
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( "6,3,6,3" == Ranges( grid ) );
        // Ctrl + Shift + End: everything from here to the last cell
        Chord( context, INPUT::KEYCODE_END, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "6,3,999,7" == Ranges( grid ) && 6 == grid->GetCursorRow() );

        // Ctrl + A: everything; the cursor stays where it is
        grid->SetCursor( 8, 2 );
        Chord( context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "0,0,-1,-1" == Ranges( grid ) && 8 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() && grid->IsCellSelected( 500, 6 ) );
        // Ctrl + Space the column, Shift + Space the row
        Chord( context, INPUT::KEYCODE_SPACE, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "0,2,-1,2" == Ranges( grid ) );
        Chord( context, INPUT::KEYCODE_SPACE, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "8,0,8,-1" == Ranges( grid ) );

        // a key the grid has no use for is not taken
        grid->SetCursor( 8, 2 );
        PressKey( context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( 8 == grid->GetCursorRow() && "8,2,8,2" == Ranges( grid ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: Ctrl + arrow goes to the edge of the data as a sheet does: the end of a run, the next value, or the edge of the grid" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CGridView* grid = MakeHolesGrid( context );
        ASSERT_TRUE( 4 == grid->GetRowCount() && 6 == grid->GetColumnCount() );

        // along row 0: a b c _ e f
        grid->SetCursor( 0, 0 );
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() );                    // the end of the run
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 4 == grid->GetCursorColumn() );                                                  // over the hole to the next value
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 5 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 5 == grid->GetCursorColumn() );                                                  // at the edge
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 4 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == grid->GetCursorColumn() );

        // down column 0: a _ g l
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 0 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 3 == grid->GetCursorRow() );
        Chord( context, INPUT::KEYCODE_UP, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == grid->GetCursorRow() );
        Chord( context, INPUT::KEYCODE_UP, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == grid->GetCursorRow() );

        // from a hole, and into a row of holes only
        grid->SetCursor( 0, 3 );
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 3 == grid->GetCursorColumn() );
        grid->SetCursor( 1, 4 );
        Chord( context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == grid->GetCursorRow() && 0 == grid->GetCursorColumn() );                     // nothing but holes: the edge

        // with Shift the block goes to the edge
        grid->SetCursor( 3, 0 );
        Chord( context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "3,0,3,5" == Ranges( grid ) && 0 == grid->GetCursorColumn() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: Tab and Enter move through the grid, or through the block when there is one, and let go of the focus at the end" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 20 ) );
        GUI::CGridView* grid = MakeCellGrid( context, model, false );

        grid->SetCursor( 0, 0 );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 0 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() && "0,1,0,1" == Ranges( grid ) );
        grid->SetCursor( 0, 7 );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 1 == grid->GetCursorRow() && 0 == grid->GetCursorColumn() );                    // the next row
        Chord( context, INPUT::KEYCODE_TAB, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == grid->GetCursorRow() && 7 == grid->GetCursorColumn() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == grid->GetCursorRow() && 7 == grid->GetCursorColumn() );
        Chord( context, INPUT::KEYCODE_RETURN, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == grid->GetCursorRow() );
        PressKey( context, INPUT::KEYCODE_NUMPADENTER );
        ASSERT_TRUE( 1 == grid->GetCursorRow() );

        // the end of the grid: Enter stays, Tab is not taken (the focus goes on)
        grid->SetCursor( 19, 7 );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 19 == grid->GetCursorRow() && 7 == grid->GetCursorColumn() );
        CORE::CString before = Ranges( grid );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 19 == grid->GetCursorRow() && 7 == grid->GetCursorColumn() && before == Ranges( grid ) );

        // through a block: the selection stays and the cursor walks in it, a row at a time for Tab, a column for Enter
        grid->SelectRange( 2, 1, 4, 3 );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() && "2,1,4,3" == Ranges( grid ) );
        PressKey( context, INPUT::KEYCODE_TAB );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 3 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() && "2,1,4,3" == Ranges( grid ) );
        grid->SelectRange( 2, 1, 4, 3 );
        PressKey( context, INPUT::KEYCODE_RETURN );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 4 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 2 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() && "2,1,4,3" == Ranges( grid ) );
        Chord( context, INPUT::KEYCODE_RETURN, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 4 == grid->GetCursorRow() && 1 == grid->GetCursorColumn() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: In 100,000 rows the cursor and the block go to the end at once, and a sort or a change of rows keeps cursor and whole rows with their rows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 100000 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeCellGrid( context, model, true );
        grid->SetCursor( 10, 0 );
        raw->ResetCounts();
        Chord( context, INPUT::KEYCODE_DOWN, INPUT::KEYMOD_CTRL );                                    // every cell has a value: the end
        ASSERT_TRUE( 99999 == grid->GetCursorRow() );
        Chord( context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( grid->IsCellSelected( 77777, 5 ) && "0,0,-1,-1" == Ranges( grid ) );
        Chord( context, INPUT::KEYCODE_HOME, INPUT::KEYMOD_CTRL );

        // whole rows and the cursor follow their rows when the model is reversed
        grid->SetCursor( 3, 2 );
        Int32 x = 0, y = 0;
        ASSERT_TRUE( RowHeaderPoint( context, grid, 5, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( 5 == grid->GetCursorRow() && "5,0,5,-1" == Ranges( grid ) );
        ASSERT_TRUE( RowHeaderPoint( context, grid, 8, x, y ) );
        ClickAt( context, x, y, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "5,0,5,-1;8,0,8,-1" == Ranges( grid ) && 8 == grid->GetCursorRow() );
        raw->Reverse();
        ASSERT_TRUE( 99999 - 8 == grid->GetCursorRow() && 2 == grid->GetCursorColumn() );
        ASSERT_TRUE( grid->GetSelection().ContainsWholeRow( 99999 - 5 ) && grid->GetSelection().ContainsWholeRow( 99999 - 8 ) && !grid->GetSelection().ContainsWholeRow( 5 ) );
        ASSERT_TRUE( 99999 - 8 == grid->GetSelectedRow() );

        // rows inserted above, rows removed
        const Int32 cursorBefore = grid->GetCursorRow();
        raw->InsertRows( 0, 10 );
        ASSERT_TRUE( cursorBefore + 10 == grid->GetCursorRow() && grid->GetSelection().ContainsWholeRow( 99999 - 5 + 10 ) );
        raw->RemoveRows( 0, 4 );
        ASSERT_TRUE( cursorBefore + 6 == grid->GetCursorRow() );
        raw->RemoveRows( (UInt32) grid->GetCursorRow(), 1 );
        ASSERT_TRUE( grid->GetRowCount() == raw->GetRowCount() && grid->GetCursorRow() < (Int32) grid->GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridSelection: A grid in row mode (the default) selects, moves and sorts as before: the cell cursor, blocks, Ctrl + A and Tab are not there" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
        grid->SetProperty( "Columns", "A\nB" );
        GUI::CGridView::TRowVector rows;
        for ( UInt32 r=0; r<30; ++r )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( CORE::CString( "a" ) + CORE::UInt32ToString( r ) );
            cells.push_back( CORE::CString( "b" ) + CORE::UInt32ToString( r ) );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
        grid->SetAllowMultiSelect( true );
        context->FlushChanges();
        context->SetFocus( grid );
        ASSERT_TRUE( !grid->IsCellMode() && "Row" == grid->GetProperty( "SelectionMode" ) );
        CEventCount cellEvents( grid, GUI::CGridView::CellSelectionChangedEvent );
        CEventCount cursorEvents( grid, GUI::CGridView::CellCursorChangedEvent );

        ClickAt( context, grid->GetRect().left + 30, grid->GetRect().top + grid->GetRowTop( 4 ) + 5 );
        ASSERT_TRUE( 4 == grid->GetSelectedRow() && grid->IsRowSelected( 4 ) );
        ClickAt( context, grid->GetRect().left + 30, grid->GetRect().top + grid->GetRowTop( 7 ) + 5, INPUT::KEYMOD_SHIFT );
        GUI::CGridView::TInt32Vector selected = grid->GetSelectedRows();
        ASSERT_TRUE( 4 == selected.size() && 4 == selected.front() && 7 == selected.back() );
        // keys: Down moves the row, Ctrl + A does nothing, Tab is not taken, Enter activates
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 8 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() );
        Chord( context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 8 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() );
        PressKey( context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( 8 == grid->GetSelectedRow() );
        Chord( context, INPUT::KEYCODE_SPACE, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 8 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() );
        CEventCount activated( grid, GUI::CWidget::ItemActivatedEvent );
        PressKey( context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == activated.count && 8 == grid->GetSelectedRow() );
        // the header with Shift is the header
        ASSERT_TRUE( 0 == cellEvents.count && 0 == cursorEvents.count && grid->GetSelection().IsEmpty() && -1 == grid->GetCursorRow() );
        // a click on a cell in row mode raises the event of the click, with the column
        CEventCount clicks( grid, GUI::CGridView::GridItemClickedEvent );
        ClickAt( context, grid->GetRect().left + 150, grid->GetRect().top + grid->GetRowTop( 2 ) + 5 );
        ASSERT_TRUE( 1 == clicks.count && 2 == grid->GetSelectedRow() );

        // switched to the cell mode and back
        grid->SetProperty( "SelectionMode", "Cell" );
        ASSERT_TRUE( grid->IsCellMode() && 2 == grid->GetCursorRow() && "2,0,2,0" == Ranges( grid ) );
        grid->SetProperty( "SelectionMode", "Row" );
        ASSERT_TRUE( !grid->IsCellMode() && grid->GetSelection().IsEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
