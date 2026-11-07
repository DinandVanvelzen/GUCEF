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
#include "gucefGUI_TestGridEditSupport.h"

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

/** The number of dashes of the outline of copied cells that are in the commands */
UInt32
CountOutlineDashes( const GUI::CDrawSurface::TCommandVector& commands )
{
    UInt32 count = 0;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_LINE == commands[ i ].type && 200 == commands[ i ].colour.r && 200 == commands[ i ].colour.g && 90 == commands[ i ].colour.b )
            ++count;
    return count;
}

const GUI::CGridEditReport*
LastResult( const CEditEvents& events )
{
    const CEditEvents::SEvent* last = events.Last( GUI::CGridView::RangeEditResultEvent );
    return GUCEF_NULL != last ? &last->report : GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridClipboardTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridClipboard" );

    GUCEF_TESTFW_TESTCASE( "GridClipboard: Copy, cut, paste and fill do nothing unless the grid asks for them, and the clipboard of the system is not touched by the tests" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene off( true, false );
        ASSERT_TRUE( off.IsReady() );
        if ( !off.IsReady() )
            return;
        CEditEvents events( off.grid );
        CFakeClipboard::Text() = "kept";
        ASSERT_TRUE( off.Select( 2, CTestModel::COL_NAME ) );
        Chord( off.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_R, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_INSERT, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_INSERT, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "kept" == CFakeClipboard::Text() && 0 == CFakeClipboard::Writes() && 0 == off.model->setCalls && events.events.empty() );
        ASSERT_TRUE( GUCEF_NULL == off.grid->GetCopyOutline() && "name 2" == Shown( off.model, 2, CTestModel::COL_NAME ) );
        ASSERT_TRUE( !off.grid->GetBoolProperty( "CellClipboard" ) );

        // the registry describes the two properties, with off as their default
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "GridView", "CellEditing" ) && GUCEF_NULL != registry.FindProperty( "GridView", "CellClipboard" ) );
        CORE::CString normalized;
        ASSERT_TRUE( registry.FindProperty( "GridView", "CellClipboard" )->Validate( "true", normalized ) && !registry.FindProperty( "GridView", "CellClipboard" )->Validate( "perhaps", normalized ) );
        ASSERT_TRUE( GUI::EWidgetProperty_CellClipboard == GUI::StringToWidgetProperty( "CellClipboard" ) && GUI::EWidgetProperty_CellEditing == GUI::StringToWidgetProperty( "CellEditing" ) );
        ASSERT_TRUE( GUI::EWidgetProperty_CellClipboard < GUI::EWidgetProperty_CellEditing && GUI::EWidgetProperty_CaretVisible < GUI::EWidgetProperty_CellClipboard && GUI::EWidgetProperty_CellEditing < GUI::EWidgetProperty_Checked );

        // the same through the calls: no model, no row mode, and no selection give a refusal, not a crash
        GUI::CGridView* bare = GUI::CWidget::As< GUI::CGridView >( off.context->CreateWidget( "GridView", "Bare", off.grid->GetParentWidget() ) );
        off.context->FlushChanges();
        CEditEvents bareEvents( bare );
        ASSERT_TRUE( !bare->CopySelection( false ) && !bare->PasteFromClipboard() && !bare->FillSelection( true ) && !bare->ClearSelectedCells() );
        ASSERT_TRUE( 4 == bareEvents.Count( GUI::CGridView::RangeEditResultEvent ) && bare->GetLastEditReport().refused && !bare->GetLastEditReport().message.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: A range is copied as the text it shows, rows by line, cells by tab, with the cells that need it quoted" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // a cell
        ASSERT_TRUE( scene.Select( 3, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 3" == CFakeClipboard::Text() && 1 == CFakeClipboard::Writes() );
        const GUI::CGridEditReport* report = LastResult( events );
        ASSERT_TRUE( GUCEF_NULL != report && GUI::CGridEditReport::OPERATION_COPY == report->operation && 1 == report->cellsApplied && !report->refused && 3 == report->row && CTestModel::COL_NAME == report->column );

        // a block, as it is shown: the format of the column, the check boxes as true and false
        grid->SelectRange( 2, CTestModel::COL_ID, 3, CTestModel::COL_FLAG );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "2\t$0.50\tname 2\tfalse\n3\t$0.75\tname 3\ttrue" == CFakeClipboard::Text() );
        ASSERT_TRUE( 8 == LastResult( events )->cellsApplied );

        // cells with tabs, quotes and line ends are quoted
        scene.model->ChangeName( 4, "a\tb" );
        scene.model->ChangeName( 5, "say \"x\"" );
        scene.model->ChangeName( 6, "l1\nl2" );
        grid->SelectRange( 4, CTestModel::COL_NAME, 6, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "\"a\tb\"\n\"say \"\"x\"\"\"\n\"l1\nl2\"" == CFakeClipboard::Text() );

        // a column that is hidden is not copied, the order of the columns is the order shown
        grid->SelectRange( 2, CTestModel::COL_ID, 2, CTestModel::COL_NAME );
        grid->SetColumnHidden( CTestModel::COL_PRICE, true );
        grid->SelectRange( 2, CTestModel::COL_ID, 2, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "2\tname 2" == CFakeClipboard::Text() );
        grid->SetColumnHidden( CTestModel::COL_PRICE, false );
        grid->MoveColumn( CTestModel::COL_NAME, 0 );
        grid->SelectRange( 2, CTestModel::COL_NAME, 2, CTestModel::COL_PRICE );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 2\t2\t$0.50" == CFakeClipboard::Text() );
        grid->MoveColumn( CTestModel::COL_NAME, 2 );

        // a whole row, a whole column, null cells as their text
        grid->SelectRows( 8, 8 );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == CORE::CString( CFakeClipboard::Text() ).HasSubstr( "8\t$2.00\tname 8\ttrue" ) || CFakeClipboard::Text().HasSubstr( "8\t$2.00\tname 8\tfalse" ) >= 0 );
        ASSERT_TRUE( 8 == LastResult( events )->cellsApplied );
        grid->SelectColumns( CTestModel::COL_MAYBE, CTestModel::COL_MAYBE );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        GUI::CTabularText::TTable lines;
        GUI::CTabularText::Parse( CFakeClipboard::Text(), lines );
        ASSERT_TRUE( 100 == lines.size() && lines[ 0 ][ 0 ].IsNULLOrEmpty() && "1" == lines[ 1 ][ 0 ] && lines[ 3 ][ 0 ].IsNULLOrEmpty() );

        // other keys for the same: Ctrl + Insert copies
        ASSERT_TRUE( scene.Select( 9, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_INSERT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 9" == CFakeClipboard::Text() );

        // without a model of the grid there is nothing to ask the clipboard of: the clipboard is not available
        CFakeClipboard::Available() = false;
        const UInt32 results = events.Count( GUI::CGridView::RangeEditResultEvent );
        ASSERT_TRUE( !grid->CopySelection( false ) && results + 1 == events.Count( GUI::CGridView::RangeEditResultEvent ) && LastResult( events )->refused && "The clipboard is not available" == LastResult( events )->message );
        CFakeClipboard::Available() = true;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: Several ranges are copied the way a spreadsheet does it: side by side when they have the same rows, under each other when they have the same columns, else refused" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // the same rows: the ranges go side by side, in the order of their columns whatever the order they were selected in
        grid->SetProperty( "SelectedRanges", "2,3,3,3\n2,0,3,1" );
        ASSERT_TRUE( 2 == grid->GetSelection().GetRangeCount() );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "2\t$0.50\tfalse\n3\t$0.75\ttrue" == CFakeClipboard::Text() && 6 == LastResult( events )->cellsApplied );

        // the same columns: under each other, in the order of their rows
        grid->SetProperty( "SelectedRanges", "6,1,7,2\n2,1,3,2" );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "$0.50\tname 2\n$0.75\tname 3\n$1.50\tname 6\n$1.75\tname 7" == CFakeClipboard::Text() );

        // not the same rows and not the same columns: refused, with a reason, and the clipboard keeps what it had
        const CORE::CString before = CFakeClipboard::Text();
        const UInt32 writes = CFakeClipboard::Writes();
        grid->SetProperty( "SelectedRanges", "2,1,3,2\n6,1,6,3" );
        ASSERT_TRUE( !grid->CopySelection( false ) );
        ASSERT_TRUE( !grid->CopySelection( false ) );
        ASSERT_TRUE( before == CFakeClipboard::Text() && writes == CFakeClipboard::Writes() );
        const GUI::CGridEditReport* report = LastResult( events );
        ASSERT_TRUE( GUCEF_NULL != report && report->refused && GUI::CGridEditReport::OPERATION_COPY == report->operation && 0 == report->cellsApplied && report->message.HasSubstr( "do not line up" ) >= 0 );

        // two ranges with a different number of rows and other columns; the same columns with other rows are a stack
        grid->SetProperty( "SelectedRanges", "2,1,3,1\n6,2,8,2" );
        ASSERT_TRUE( !grid->CopySelection( false ) && before == CFakeClipboard::Text() );
        grid->SetProperty( "SelectedRanges", "2,1,3,1\n6,1,8,1" );
        ASSERT_TRUE( grid->CopySelection( false ) && "$0.50\n$0.75\n$1.50\n$1.75\n$2.00" == CFakeClipboard::Text() );
        // three ranges on the same rows
        grid->SetProperty( "SelectedRanges", "4,0,5,0\n4,2,5,2\n4,3,5,3" );
        ASSERT_TRUE( grid->CopySelection( false ) && "4\tname 4\tfalse\n5\tname 5\ttrue" == CFakeClipboard::Text() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: Cut copies and clears the cells, and counts the cells that are read only" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        ASSERT_TRUE( scene.Select( 3, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 3" == CFakeClipboard::Text() && Shown( scene.model, 3, CTestModel::COL_NAME ).IsNULLOrEmpty() );
        ASSERT_TRUE( GUCEF_NULL != LastResult( events ) && GUI::CGridEditReport::OPERATION_CUT == LastResult( events )->operation && 1 == LastResult( events )->cellsApplied );

        // the id column is read only: copied, not cleared
        grid->SelectRange( 4, CTestModel::COL_ID, 5, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "4\t$1.00\tname 4\n5\t$1.25\tname 5" == CFakeClipboard::Text() );
        const GUI::CGridEditReport* report = LastResult( events );
        ASSERT_TRUE( 2 == report->cellsReadOnly && 4 == report->cellsApplied && "4" == Shown( scene.model, 4, CTestModel::COL_ID ) );
        ASSERT_TRUE( scene.model->Current( 4, CTestModel::COL_PRICE ).IsNULL() && Shown( scene.model, 5, CTestModel::COL_NAME ).IsNULLOrEmpty() );
        ASSERT_TRUE( report->problemRow == 4 && report->problemColumn == CTestModel::COL_ID );

        // Shift + Delete is a cut too
        ASSERT_TRUE( scene.Select( 7, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_DELETE, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "name 7" == CFakeClipboard::Text() && Shown( scene.model, 7, CTestModel::COL_NAME ).IsNULLOrEmpty() );

        // cut is one step for undo
        grid->SelectRange( 10, CTestModel::COL_PRICE, 13, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( scene.model->Current( 12, CTestModel::COL_PRICE ).IsNULL() );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 3.0 == scene.model->Current( 12, CTestModel::COL_PRICE ).AsFloat64() && "name 13" == Shown( scene.model, 13, CTestModel::COL_NAME ) && "name 10" == Shown( scene.model, 10, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: A paste is read by the type of every column and applied cell by cell through the model, which says what it refuses; the summary says what happened" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // a block of 2 x 3 cells at the cursor
        CFakeClipboard::Text() = "1.5\tpasted a\ttrue\n2.5\tpasted b\tfalse";
        ASSERT_TRUE( scene.Select( 10, CTestModel::COL_PRICE ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1.5 == scene.model->Current( 10, CTestModel::COL_PRICE ).AsFloat64() && "pasted a" == Shown( scene.model, 10, CTestModel::COL_NAME ) && scene.model->Current( 10, CTestModel::COL_FLAG ).AsBool() );
        ASSERT_TRUE( 2.5 == scene.model->Current( 11, CTestModel::COL_PRICE ).AsFloat64() && "pasted b" == Shown( scene.model, 11, CTestModel::COL_NAME ) && !scene.model->Current( 11, CTestModel::COL_FLAG ).AsBool() );
        const GUI::CGridEditReport* report = LastResult( events );
        ASSERT_TRUE( GUCEF_NULL != report && GUI::CGridEditReport::OPERATION_PASTE == report->operation && 6 == report->cellsApplied && 0 == report->cellsRefused && 0 == report->cellsReadOnly && 0 == report->cellsSkipped && !report->refused );
        ASSERT_TRUE( 10 == report->row && CTestModel::COL_PRICE == report->column );

        // the pasted cells are the selection, the cursor is at the first
        ASSERT_TRUE( 10 == grid->GetCursorRow() && CTestModel::COL_PRICE == grid->GetCursorColumn() && grid->IsCellSelected( 11, CTestModel::COL_FLAG ) && !grid->IsCellSelected( 12, CTestModel::COL_FLAG ) );
        ASSERT_TRUE( 1 == scene.model->groupsBegun && scene.model->groupsBegun == scene.model->groupsEnded );

        // cells that do not parse, and one the model refuses (a negative price), are counted; the others go in
        CFakeClipboard::Text() = "abc\tfine\n-5\tfine too\n";
        ASSERT_TRUE( scene.Select( 20, CTestModel::COL_PRICE ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( 2 == report->cellsApplied && 2 == report->cellsRefused && !report->refused );
        ASSERT_TRUE( 20 == report->problemRow && CTestModel::COL_PRICE == report->problemColumn && "Not a number" == report->message );
        ASSERT_TRUE( "fine" == Shown( scene.model, 20, CTestModel::COL_NAME ) && "fine too" == Shown( scene.model, 21, CTestModel::COL_NAME ) && 5.0 == scene.model->Current( 20, CTestModel::COL_PRICE ).AsFloat64() );
        ASSERT_TRUE( scene.model->Current( 21, CTestModel::COL_PRICE ).AsFloat64() == 21 * 0.25 );

        // the refusal of the model is the message when the first cell that went wrong is one it refused
        CFakeClipboard::Text() = "-5";
        ASSERT_TRUE( scene.Select( 22, CTestModel::COL_PRICE ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( 0 == report->cellsApplied && 1 == report->cellsRefused && report->refused && "The price must not be negative" == report->message );

        // a column that is read only: counted; the others are applied
        CFakeClipboard::Text() = "777\t9.5";
        ASSERT_TRUE( scene.Select( 23, CTestModel::COL_ID ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( 1 == report->cellsApplied && 1 == report->cellsReadOnly && "23" == Shown( scene.model, 23, CTestModel::COL_ID ) && 9.5 == scene.model->Current( 23, CTestModel::COL_PRICE ).AsFloat64() );
        scene.model->readOnlyIds.insert( 24 );
        CFakeClipboard::Text() = "1.5\n2.5";
        ASSERT_TRUE( scene.Select( 24, CTestModel::COL_PRICE ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( 1 == report->cellsApplied && 1 == report->cellsReadOnly && 2.5 == scene.model->Current( 25, CTestModel::COL_PRICE ).AsFloat64() );

        // the end of the grid: what does not fit is left out and counted
        CFakeClipboard::Text() = "1\ta\n2\tb\n3\tc";
        ASSERT_TRUE( scene.Select( 98, CTestModel::COL_MAYBE ) );
        grid->SetCursor( 98, CTestModel::COL_AMOUNT );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( 2 == report->cellsApplied && 4 == report->cellsSkipped && 0 == report->cellsRefused );          // the second column and the third row are not there

        // types: whole numbers, dates, booleans in several spellings, a null from an empty cell
        CFakeClipboard::Text() = "12\t2022-01-02\tyes\t\t3.5";
        grid->SetCursor( 30, CTestModel::COL_MAYBE );
        grid->SelectRange( 30, CTestModel::COL_MAYBE, 30, CTestModel::COL_MAYBE );
        grid->SetCursor( 30, CTestModel::COL_FLAG );
        CFakeClipboard::Text() = "yes";
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( scene.model->Current( 30, CTestModel::COL_FLAG ).AsBool() );
        grid->SetCursor( 31, CTestModel::COL_DATE );
        CFakeClipboard::Text() = "2022-01-02";
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "2022-01-02" == Shown( scene.model, 31, CTestModel::COL_DATE ) );
        CFakeClipboard::Text() = "02/01/2022";
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "2022-01-02" == Shown( scene.model, 31, CTestModel::COL_DATE ) && 1 == LastResult( events )->cellsRefused );
        grid->SetCursor( 32, CTestModel::COL_MAYBE );
        CFakeClipboard::Text() = "12";
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 12 == scene.model->Current( 32, CTestModel::COL_MAYBE ).AsInt32() );
        CFakeClipboard::Text() = "\t";
        grid->SetCursor( 32, CTestModel::COL_MAYBE );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( scene.model->Current( 32, CTestModel::COL_MAYBE ).IsNULL() && !LastResult( events )->refused );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: One cell of text fills the selection, a block is repeated over a range that is a multiple of it, and a block does not go into several ranges" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        CFakeClipboard::Text() = "7.75";
        grid->SetProperty( "SelectedRanges", "2,1,4,1\n8,1,9,1" );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 5 == LastResult( events )->cellsApplied );
        for ( UInt32 row = 2; row <= 9; ++row )
        {
            const bool filled = row <= 4 || row >= 8;
            ASSERT_TRUE( filled == ( 7.75 == scene.model->Current( row, CTestModel::COL_PRICE ).AsFloat64() ) );
        }

        // a block of 2 rows by 1 column over 4 by 2: repeated
        CFakeClipboard::Text() = "p\nq";
        grid->SelectRange( 20, CTestModel::COL_NAME, 23, CTestModel::COL_FLAG );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "p" == Shown( scene.model, 20, CTestModel::COL_NAME ) && "q" == Shown( scene.model, 21, CTestModel::COL_NAME ) && "p" == Shown( scene.model, 22, CTestModel::COL_NAME ) && "q" == Shown( scene.model, 23, CTestModel::COL_NAME ) );
        ASSERT_TRUE( 4 == LastResult( events )->cellsApplied && 4 == LastResult( events )->cellsRefused );     // the flag column: "p" is no boolean, the name column took the text

        // a range that is not a multiple takes the block once, at its top left cell
        CFakeClipboard::Text() = "r\ns";
        grid->SelectRange( 30, CTestModel::COL_NAME, 32, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "r" == Shown( scene.model, 30, CTestModel::COL_NAME ) && "s" == Shown( scene.model, 31, CTestModel::COL_NAME ) && "name 32" == Shown( scene.model, 32, CTestModel::COL_NAME ) );

        // a block into more than one range: refused
        grid->SetProperty( "SelectedRanges", "40,2,40,2\n44,2,44,2" );
        const UInt32 calls = scene.model->setCalls;
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( calls == scene.model->setCalls && LastResult( events )->refused && LastResult( events )->message.HasSubstr( "several ranges" ) >= 0 );

        // quoted cells with line ends, lines ending in CRLF
        CFakeClipboard::Text() = "\"l1\r\nl2\"\tx\r\n";
        ASSERT_TRUE( scene.Select( 50, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "l1\r\nl2" == Shown( scene.model, 50, CTestModel::COL_NAME ) && 2 == LastResult( events )->cellsApplied + LastResult( events )->cellsRefused );

        // nothing on the clipboard, or nothing to paste at
        CFakeClipboard::Text().Clear();
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( LastResult( events )->refused && LastResult( events )->message.HasSubstr( "holds no text" ) >= 0 );
        CFakeClipboard::Text() = "x";
        grid->ClearSelection();
        ASSERT_TRUE( !grid->PasteFromClipboard() && LastResult( events )->refused );

        // the copy of a grid is what another grid takes
        ASSERT_TRUE( scene.Select( 60, CTestModel::COL_NAME ) );
        grid->SelectRange( 60, CTestModel::COL_NAME, 62, CTestModel::COL_NAME );
        ASSERT_TRUE( grid->CopySelection( false ) );
        ASSERT_TRUE( scene.Select( 70, CTestModel::COL_NAME ) );
        ASSERT_TRUE( grid->PasteFromClipboard() && "name 60" == Shown( scene.model, 70, CTestModel::COL_NAME ) && "name 62" == Shown( scene.model, 72, CTestModel::COL_NAME ) );
        ASSERT_TRUE( grid->IsCellSelected( 71, CTestModel::COL_NAME ) && !grid->IsCellSelected( 73, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: A paste or a fill is one step of the history of the model" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CFakeClipboard::Text() = "a\tb\nc\td";
        ASSERT_TRUE( scene.Select( 3, CTestModel::COL_NAME ) );
        grid->SetCursor( 3, CTestModel::COL_NAME );
        grid->SelectRange( 3, CTestModel::COL_NAME, 3, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == scene.model->GetHistory().GetUndoStepCount() );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 3" == Shown( scene.model, 3, CTestModel::COL_NAME ) && "name 4" == Shown( scene.model, 4, CTestModel::COL_NAME ) && !scene.model->CanUndo() );
        Chord( scene.context, INPUT::KEYCODE_Y, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "a" == Shown( scene.model, 3, CTestModel::COL_NAME ) && "c" == Shown( scene.model, 4, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: Fill down copies the first row over the others, fill right the first column; one row or column takes what is before it" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // down: the first row of the range, typed values
        grid->SelectRange( 30, CTestModel::COL_PRICE, 33, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 7.5 == scene.model->Current( 33, CTestModel::COL_PRICE ).AsFloat64() && "name 30" == Shown( scene.model, 33, CTestModel::COL_NAME ) && "name 30" == Shown( scene.model, 31, CTestModel::COL_NAME ) );
        const GUI::CGridEditReport* report = LastResult( events );
        ASSERT_TRUE( GUI::CGridEditReport::OPERATION_FILL_DOWN == report->operation && 6 == report->cellsApplied && !report->refused );
        ASSERT_TRUE( 7.5 == scene.model->Current( 30, CTestModel::COL_PRICE ).AsFloat64() );

        // one row: the row above
        ASSERT_TRUE( scene.Select( 41, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 40" == Shown( scene.model, 41, CTestModel::COL_NAME ) && 1 == LastResult( events )->cellsApplied );
        // the first row has no row above it
        ASSERT_TRUE( scene.Select( 0, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 0" == Shown( scene.model, 0, CTestModel::COL_NAME ) && 1 == LastResult( events )->cellsSkipped && 0 == LastResult( events )->cellsApplied );

        // read only cells are counted and stay
        grid->SelectRange( 50, CTestModel::COL_ID, 52, CTestModel::COL_PRICE );
        Chord( scene.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == LastResult( events )->cellsReadOnly && 2 == LastResult( events )->cellsApplied && "51" == Shown( scene.model, 51, CTestModel::COL_ID ) );
        ASSERT_TRUE( scene.model->Current( 52, CTestModel::COL_PRICE ).AsFloat64() == 50 * 0.25 );

        // right: the first column over the others; a text column into a text column is the text, a number into a boolean is a refusal
        grid->SelectRange( 60, CTestModel::COL_PRICE, 61, CTestModel::COL_FLAG );
        Chord( scene.context, INPUT::KEYCODE_R, INPUT::KEYMOD_CTRL );
        report = LastResult( events );
        ASSERT_TRUE( GUI::CGridEditReport::OPERATION_FILL_RIGHT == report->operation && 2 == report->cellsApplied && 2 == report->cellsRefused );
        ASSERT_TRUE( "15" == Shown( scene.model, 60, CTestModel::COL_NAME ) && "15.25" == Shown( scene.model, 61, CTestModel::COL_NAME ) && "Not true or false" == report->message );

        // one column: the column at its left; the first column has none
        ASSERT_TRUE( scene.Select( 70, CTestModel::COL_NAME ) );
        scene.model->ChangeName( 70, "left" );
        grid->SetCursor( 70, CTestModel::COL_FLAG );
        grid->SelectRange( 70, CTestModel::COL_NAME, 70, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_R, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == LastResult( events )->cellsApplied && "17.5" == Shown( scene.model, 70, CTestModel::COL_NAME ) );
        grid->SelectRange( 71, CTestModel::COL_ID, 71, CTestModel::COL_ID );
        Chord( scene.context, INPUT::KEYCODE_R, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == LastResult( events )->cellsSkipped );

        // fill is one step of the history
        grid->SelectRange( 80, CTestModel::COL_NAME, 83, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_D, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 80" == Shown( scene.model, 83, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 83" == Shown( scene.model, 83, CTestModel::COL_NAME ) && "name 81" == Shown( scene.model, 81, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: The cells that were copied are outlined until Escape, a new copy, or a change of the rows" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( scene.context, grid );
        ASSERT_TRUE( GUCEF_NULL == grid->GetCopyOutline() && 0 == CountOutlineDashes( commands ) );

        grid->SelectRange( 2, CTestModel::COL_PRICE, 4, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( GUCEF_NULL != grid->GetCopyOutline() && 1 == grid->GetCopyOutline()->GetRangeCount() );
        commands = PaintedCommands( scene.context, grid );
        const UInt32 dashes = CountOutlineDashes( commands );
        ASSERT_TRUE( dashes > 8 && dashes < 200 );

        // the outline is round the copied cells: the line at the top is at the top of the first row
        bool atTop = false;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_LINE == commands[ i ].type && 200 == commands[ i ].colour.r && commands[ i ].y0 == commands[ i ].y1 && commands[ i ].y0 == grid->GetRowTop( 2 ) )
                atTop = true;
        ASSERT_TRUE( atTop );

        // the outline of copied whole columns of a long table is cut to the grid, not drawn all along
        grid->SelectColumns( CTestModel::COL_PRICE, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        commands = PaintedCommands( scene.context, grid );
        ASSERT_TRUE( CountOutlineDashes( commands ) > 8 && CountOutlineDashes( commands ) < 400 );

        // Escape ends it, once
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( GUCEF_NULL == grid->GetCopyOutline() );
        commands = PaintedCommands( scene.context, grid );
        ASSERT_TRUE( 0 == CountOutlineDashes( commands ) );

        // rows inserted: the cells it was round have moved
        grid->SelectRange( 2, CTestModel::COL_PRICE, 4, CTestModel::COL_NAME );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( GUCEF_NULL != grid->GetCopyOutline() );
        scene.model->InsertRows( 0, 2 );
        ASSERT_TRUE( GUCEF_NULL == grid->GetCopyOutline() );

        // a cut and a copy replace each other; a grid that has other keys for Escape keeps them
        grid->SelectRange( 2, CTestModel::COL_PRICE, 2, CTestModel::COL_PRICE );
        Chord( scene.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( GUCEF_NULL != grid->GetCopyOutline() );
        grid->ClearCopyOutline();
        ASSERT_TRUE( GUCEF_NULL == grid->GetCopyOutline() );
        scene.model->RaiseCells( 0, 5, 0, 3 );
        grid->SelectRange( 2, CTestModel::COL_PRICE, 2, CTestModel::COL_PRICE );
        grid->CopySelection( false );
        scene.model->RaiseCells( 0, 5, 0, 3 );                  // a value that changed does not move the cells
        ASSERT_TRUE( GUCEF_NULL != grid->GetCopyOutline() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridClipboard: A copy and a paste of thousands of rows go through the model at the pace of the model (the numbers are in the log)" )
    try
    {
        CFakeClipboard clipboard;
        SEditScene scene( true, true, 10000 );
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        grid->SelectColumns( CTestModel::COL_NAME, CTestModel::COL_NAME );
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( grid->CopySelection( false ) );
        const double copyMicros = Micros( MT::PrecisionTickCount() - start );
        GUI::CTabularText::TTable lines;
        GUI::CTabularText::Parse( CFakeClipboard::Text(), lines );
        ASSERT_TRUE( 10000 == lines.size() && "name 9999" == lines[ 9999 ][ 0 ] && 10000 == LastResult( events )->cellsApplied );

        // paste it two columns over, at the top: the Id column is read only, the price takes text it cannot read
        CORE::CString block;
        for ( UInt32 i=0; i<5000; ++i )
            block += CORE::UInt32ToString( i ) + ".5\tn" + CORE::UInt32ToString( i ) + "\n";
        CFakeClipboard::Text() = block;
        ASSERT_TRUE( scene.Select( 0, CTestModel::COL_PRICE ) );
        start = MT::PrecisionTickCount();
        ASSERT_TRUE( grid->PasteFromClipboard() );
        const double pasteMicros = Micros( MT::PrecisionTickCount() - start );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: copy of 10000 cells " << (UInt64) copyMicros << " us, paste of 5000 rows by 2 columns (10000 cells through the model) " << (UInt64) pasteMicros << " us" );
        ASSERT_TRUE( "n4999" == Shown( scene.model, 4999, CTestModel::COL_NAME ) && 4999.5 == scene.model->Current( 4999, CTestModel::COL_PRICE ).AsFloat64() );
        ASSERT_TRUE( 10000 == LastResult( events )->cellsApplied && 1 == scene.model->GetHistory().GetUndoStepCount() );
        ASSERT_TRUE( copyMicros < 5000000.0 && pasteMicros < 20000000.0 );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 4999" == Shown( scene.model, 4999, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
