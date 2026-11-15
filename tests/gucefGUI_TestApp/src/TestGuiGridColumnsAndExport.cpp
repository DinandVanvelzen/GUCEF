/*
 *  gucefGUI_TestApp: test application for the gucefGUI module
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

/*
 *  The column chooser, the preferences of grids and the CSV export: hidden by default and required columns, the order
 *  of all columns, reset, the header strip, the chooser form, preference records and the local store, the CSV text,
 *  streaming of a large table, the file save dialog flow, sliced and asynchronous exports.
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

#include "CDynamicBufferAccess.h"
#include "gucefGUI_TestGridSupport.h"
#include "gucefGUI_TestGridExportSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GridTest;
using namespace GridExportTest;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A model of five text columns: A, B (hidden by default), C (required), D, E */
CTextTableModel*
MakeFiveColumns( UInt32 rows )
{
    CTextTableModel* model = GUCEF_NEW CTextTableModel();
    model->AddColumn( "A", CORE::DATATABLE_TYPE_UTF8, "a" );
    model->AddColumn( "B", CORE::DATATABLE_TYPE_UTF8, "b" ).hiddenByDefault = true;
    model->AddColumn( "C", CORE::DATATABLE_TYPE_UTF8, "c" ).required = true;
    model->AddColumn( "D", CORE::DATATABLE_TYPE_UTF8, "d" );
    model->AddColumn( "E", CORE::DATATABLE_TYPE_UTF8, "e" );
    for ( UInt32 r=0; r<rows; ++r )
    {
        GUCEF::vector< CORE::CVariant > values;
        for ( UInt32 c=0; c<5; ++c )
            values.push_back( CORE::CVariant( CORE::CString( (char) ( 'a' + c ) ) + CORE::UInt32ToString( r ) ) );
        model->AddRow( values );
    }
    return model;
}

/*-------------------------------------------------------------------------*/

/** The data columns in the order they are shown */
CORE::CString
ShownColumns( GUI::CGridView* grid )
{
    const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
    CORE::CString text;
    for ( UInt32 p=0; p<layout.GetPositionCount(); ++p )
        text += (char) ( 'A' + layout.GetColumn( (Int32) p ) );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
OrderText( GUI::CGridView* grid )
{
    const GUI::CGridView::TInt32Vector order = grid->GetColumnOrder();
    CORE::CString text;
    for ( UInt32 i=0; i<(UInt32)order.size(); ++i )
        text += (char) ( 'A' + order[ i ] );
    return text;
}

/*-------------------------------------------------------------------------*/

/** The model of the text tests: Name, Note, Count (grouped digits); five rows of what quoting has to handle */
CTextTableModel*
MakeQuotingModel( void )
{
    CTextTableModel* model = GUCEF_NEW CTextTableModel();
    model->AddColumn( "Name", CORE::DATATABLE_TYPE_UTF8, "name" );
    model->AddColumn( "Note", CORE::DATATABLE_TYPE_UTF8, "note" );
    model->AddColumn( "Count", CORE::DATATABLE_TYPE_INT64, "count" ).thousandsSeparator = true;
    model->AddTextRow( "plain", "a,b", CORE::CVariant( (Int64) 1234567 ) );
    model->AddTextRow( "say \"hi\"", "line1\nline2", CORE::CVariant( (Int64) 5 ) );
    model->AddTextRow( " lead", "trail ", NullValue() );
    model->AddTextRow( "", "cr\rx", CORE::CVariant( (Int64) 0 ) );
    model->AddTextRow( "\xC3\xA9t\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC", "tab\tin", CORE::CVariant( (Int64) 42 ) );
    return model;
}

/*-------------------------------------------------------------------------*/

/** A grid of 800 x 300 at 10,10 in a context of the null driver */
GUI::CGridView*
MakeExportGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetProperty( "FitColumns", "true" );
    if ( !model.IsNULL() )
        grid->SetModel( model );
    context->FlushChanges();
    return grid;
}

/*-------------------------------------------------------------------------*/

/** Exports the grid as it is shown to memory */
CORE::CString
ExportText( GUI::CGridView* grid, const GUI::CGridCsvOptions& options, GUI::CGridCsvExportReport* report = GUCEF_NULL )
{
    CMemoryOutput output;
    GUI::CGridExportSource source( *grid, options.rawValues );
    GUI::CGridCsvExportReport result;
    if ( !GUI::CGridCsvExporter::Export( source, output.Access(), options, result ) )
        return "<failed: " + result.message + ">";
    if ( GUCEF_NULL != report )
        *report = result;
    return output.Text();
}

/*-------------------------------------------------------------------------*/

CORE::CString
ScratchDir( const char* name )
{
    const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), name );
    CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** The names of the files in a directory */
CORE::CString::StringSet
FilesIn( const CORE::CString& dir )
{
    CORE::CString::StringSet names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( dir ) )
    {
        do
        {
            if ( iterator.IsAFile() )
                names.insert( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

CORE::CString
FileText( const CORE::CString& path )
{
    CORE::CString text;
    if ( !CORE::LoadTextFileAsString( path, text ) )
        return "<no file>";
    return text;
}

/*-------------------------------------------------------------------------*/

/** The resident memory of the process in KiB; 0 where it cannot be read */
UInt64
ResidentKiB( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    CORE::CString status;
    if ( !CORE::LoadTextFileAsString( "/proc/self/status", status ) )
        return 0;
    const Int32 at = status.HasSubstr( "VmRSS:" );
    if ( at < 0 )
        return 0;
    const char* text = status.C_String() + at + 6;
    while ( '\0' != *text && ( *text < '0' || *text > '9' ) )
        ++text;
    UInt64 kib = 0;
    while ( *text >= '0' && *text <= '9' )
        kib = kib * 10 + (UInt64) ( *text++ - '0' );
    return kib;
    #else
    return 0;
    #endif
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridColumnsAndExportTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridColumnsAndExport" );

    GUCEF_TESTFW_TESTCASE( "GridColumns: Columns hidden by default stay hidden until HiddenColumns is set, required columns are always shown, the last column is never hidden" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTextTableModel* model = MakeFiveColumns( 3 );
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );

        // the defaults of the model
        ASSERT_TRUE( "ACDE" == ShownColumns( grid ) );
        ASSERT_TRUE( grid->IsColumnHidden( 1 ) && grid->IsColumnHiddenByDefault( 1 ) && !grid->IsColumnHiddenByDefault( 0 ) );
        ASSERT_TRUE( grid->IsColumnRequired( 2 ) && !grid->IsColumnRequired( 0 ) && !grid->IsColumnHiddenByDefault( 2 ) );
        ASSERT_FALSE( grid->IsPropertySet( "HiddenColumns" ) );

        // a required column is not hidden: not by the member, not by the property
        CEventCount visibility( grid, GUI::CGridView::ColumnVisibilityChangedEvent );
        ASSERT_FALSE( grid->CanHideColumn( 2 ) );
        ASSERT_FALSE( grid->SetColumnHidden( 2, true ) );
        ASSERT_TRUE( !grid->IsColumnHidden( 2 ) && 0 == visibility.count );
        grid->SetProperty( "HiddenColumns", "2,3" );          // set, the property is the whole truth: B shows, C is required
        ASSERT_TRUE( "ABCE" == ShownColumns( grid ) );

        // set, the property is the whole truth: an empty one shows the column hidden by default; reset, the model decides again
        grid->SetProperty( "HiddenColumns", "" );
        ASSERT_TRUE( "ABCDE" == ShownColumns( grid ) );
        grid->ResetProperty( "HiddenColumns" );
        ASSERT_TRUE( "ACDE" == ShownColumns( grid ) );

        // hiding through the member keeps what the defaults hide
        ASSERT_TRUE( grid->SetColumnHidden( 0, true ) );
        ASSERT_TRUE( "CDE" == ShownColumns( grid ) && 1 == visibility.count );
        ASSERT_TRUE( grid->SetColumnHidden( 0, true ) && 1 == visibility.count );      // already so: true, no event
        ASSERT_TRUE( grid->SetColumnHidden( 1, false ) && "BCDE" == ShownColumns( grid ) );

        // the last column that is shown stays
        CTextTableModel* plain = GUCEF_NEW CTextTableModel();
        plain->AddColumn( "X", CORE::DATATABLE_TYPE_UTF8, "x" );
        plain->AddColumn( "Y", CORE::DATATABLE_TYPE_UTF8, "y" );
        grid->SetModel( GUI::TTableViewModelPtr( plain ) );
        grid->ResetProperty( "HiddenColumns" );
        ASSERT_TRUE( grid->SetColumnHidden( 0, true ) );
        ASSERT_FALSE( grid->CanHideColumn( 1 ) );
        ASSERT_FALSE( grid->SetColumnHidden( 1, true ) );
        ASSERT_TRUE( "B" == ShownColumns( grid ) );

        // without a model: SetRequiredColumns
        grid->SetModel( GUI::TTableViewModelPtr() );
        grid->ResetProperty( "HiddenColumns" );
        grid->SetProperty( "Columns", "First\nSecond\nThird" );
        GUI::CGridView::TInt32Vector required;
        required.push_back( 0 );
        grid->SetRequiredColumns( required );
        grid->SetProperty( "HiddenColumns", "0,1" );
        ASSERT_TRUE( "AC" == ShownColumns( grid ) );
        ASSERT_TRUE( grid->IsColumnRequired( 0 ) && !grid->IsColumnRequired( 1 ) );
        ASSERT_FALSE( grid->SetColumnHidden( 0, true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridColumns: The order of all columns includes the hidden ones, a hidden column moves too, keys name the columns and repeats are numbered, reset goes back to the defaults" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTextTableModel* model = MakeFiveColumns( 3 );
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );
        CEventCount moved( grid, GUI::CGridView::ColumnMovedEvent );
        CEventCount reset( grid, GUI::CGridView::ColumnsResetEvent );

        ASSERT_TRUE( "ABCDE" == OrderText( grid ) );
        ASSERT_TRUE( grid->MoveColumnInOrder( 1, 0 ) );     // B is hidden: it moves in the order of all columns
        ASSERT_TRUE( "BACDE" == OrderText( grid ) && "ACDE" == ShownColumns( grid ) && 1 == moved.count );
        ASSERT_FALSE( grid->MoveColumnInOrder( 1, 0 ) );    // already there
        ASSERT_TRUE( grid->SetColumnHidden( 1, false ) && "BACDE" == ShownColumns( grid ) );
        ASSERT_TRUE( grid->MoveColumnInOrder( 4, 1 ) && "BEACD" == OrderText( grid ) && "BEACD" == ShownColumns( grid ) );
        ASSERT_TRUE( grid->MoveColumnInOrder( 0, 99 ) && "BECDA" == OrderText( grid ) );   // past the end: the last place

        // keys: the names of the model columns
        GUI::CGridView::TStringVector keys = grid->GetColumnKeys();
        ASSERT_TRUE( 5 == keys.size() && "a" == keys[ 0 ] && "e" == keys[ 4 ] && 3 == grid->FindColumnByKey( "d" ) && -1 == grid->FindColumnByKey( "z" ) );

        // reset: what the application set up when the defaults were captured
        grid->CaptureColumnDefaults();
        const CORE::CString defaultOrder = OrderText( grid );
        const CORE::CString defaultShown = ShownColumns( grid );
        grid->SetColumnHidden( 3, true );
        grid->MoveColumnInOrder( 3, 0 );
        grid->SetColumnWidth( 0, 222 );
        ASSERT_TRUE( defaultShown != ShownColumns( grid ) && !grid->GetIntListProperty( "ColumnWidths" ).empty() && 222 == grid->GetIntListProperty( "ColumnWidths" )[ 0 ] );
        grid->ResetColumnsToDefault();
        ASSERT_TRUE( defaultOrder == OrderText( grid ) && defaultShown == ShownColumns( grid ) && 1 == reset.count );
        ASSERT_FALSE( grid->IsPropertySet( "ColumnWidths" ) );

        // defaults recorded before anything was set: the model decides again
        CTextTableModel* fresh = MakeFiveColumns( 1 );
        GUI::CGridView* other = MakeExportGrid( context, GUI::TTableViewModelPtr( fresh ) );
        other->CaptureColumnDefaults();
        other->SetColumnHidden( 1, false );
        other->MoveColumnInOrder( 4, 0 );
        other->ResetColumnsToDefault();
        ASSERT_TRUE( "ABCDE" == OrderText( other ) && "ACDE" == ShownColumns( other ) && !other->IsPropertySet( "HiddenColumns" ) );

        // without a model the keys are the titles, repeats numbered, an empty title by its place
        other->SetModel( GUI::TTableViewModelPtr() );
        other->SetProperty( "Columns", "Price\nPrice\n\nQty" );
        keys = other->GetColumnKeys();
        ASSERT_TRUE( 4 == keys.size() && "Price" == keys[ 0 ] && "Price#2" == keys[ 1 ] && "#3" == keys[ 2 ] && "Qty" == keys[ 3 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridColumns: The header strip is off by default and draws nothing; on, it keeps a strip free at the right, its buttons hit, draw and open the chooser and the export" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTextTableModel* model = MakeFiveColumns( 4 );
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );

        // off: no strip, the drawing is that of a grid without the feature, also with the properties set to false
        ASSERT_TRUE( 0 == grid->GetHeaderStripWidth() && 800 == grid->GetColumnLayout().GetContentWidth() );
        GUI::CRect rect;
        ASSERT_FALSE( grid->GetHeaderButtonRect( GUI::CGridView::HEADERBUTTON_COLUMN_CHOOSER, rect ) );
        const UInt64 before = HashCommands( PaintedCommands( context, grid ) );
        grid->SetProperty( "ShowColumnChooser", "false" );
        grid->SetProperty( "ShowCsvExport", "false" );
        ASSERT_TRUE( before == HashCommands( PaintedCommands( context, grid ) ) );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_HEADER == grid->HitTest( 790, 5 ).kind );

        // on: a row height per button at the right, the columns in the rest
        grid->SetProperty( "ShowColumnChooser", "true" );
        ASSERT_TRUE( 20 == grid->GetHeaderStripWidth() && 780 == grid->GetColumnLayout().GetContentWidth() );
        ASSERT_TRUE( grid->GetHeaderButtonRect( GUI::CGridView::HEADERBUTTON_COLUMN_CHOOSER, rect ) && GUI::CRect( 780, 0, 20, 20 ) == rect );
        grid->SetProperty( "ShowCsvExport", "true" );
        ASSERT_TRUE( 40 == grid->GetHeaderStripWidth() && 760 == grid->GetColumnLayout().GetContentWidth() );
        ASSERT_TRUE( grid->GetHeaderButtonRect( GUI::CGridView::HEADERBUTTON_COLUMN_CHOOSER, rect ) && GUI::CRect( 760, 0, 20, 20 ) == rect );
        ASSERT_TRUE( grid->GetHeaderButtonRect( GUI::CGridView::HEADERBUTTON_CSV_EXPORT, rect ) && GUI::CRect( 780, 0, 20, 20 ) == rect );
        ASSERT_TRUE( GUI::CGridView::HIT_COLUMN_CHOOSER_BUTTON == grid->HitTest( 770, 5 ).kind );
        ASSERT_TRUE( GUI::CGridView::HIT_CSV_EXPORT_BUTTON == grid->HitTest( 790, 5 ).kind );
        ASSERT_TRUE( GUI::CGridView::HIT_NONE == grid->HitTest( 790, 50 ).kind );         // the strip below the header: nothing
        ASSERT_TRUE( GUI::CGridView::HIT_CELL == grid->HitTest( 740, 50 ).kind );

        // drawn: the strip in the header colour over the right end, and lines for the icons inside the buttons
        const GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( before != HashCommands( commands ) );
        ASSERT_TRUE( HasFill( commands, GUI::CRect( 760, 0, 40, 20 ) ) );
        UInt32 linesInButtons = 0;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_LINE == commands[ i ].type && commands[ i ].x0 >= 760 && commands[ i ].y0 < 20 )
                ++linesInButtons;
        ASSERT_TRUE( linesInButtons >= 6 );

        // only the export: one button at the right
        grid->SetProperty( "ShowColumnChooser", "false" );
        ASSERT_TRUE( 20 == grid->GetHeaderStripWidth() && GUI::CGridView::HIT_CSV_EXPORT_BUTTON == grid->HitTest( 790, 5 ).kind );
        ASSERT_TRUE( grid->GetHeaderButtonRect( GUI::CGridView::HEADERBUTTON_CSV_EXPORT, rect ) && GUI::CRect( 780, 0, 20, 20 ) == rect );
        grid->SetProperty( "ShowColumnChooser", "true" );

        // a click on a button: the chooser opens, the export opens the file save dialog; the click does not sort or select
        grid->SetProperty( "Sortable", "true" );
        CEventCount clicked( grid, GUI::CGridView::ColumnClickedEvent );
        ClickAt( context, 10 + 770, 10 + 10 );
        ASSERT_TRUE( grid->IsColumnChooserOpen() && 0 == clicked.count );
        GUI::CGridColumnChooserForm* chooser = grid->GetColumnChooser();
        ASSERT_TRUE( GUCEF_NULL != chooser && chooser->IsOpen() );
        if ( GUCEF_NULL != chooser )
            chooser->Close();
        ASSERT_FALSE( grid->IsColumnChooserOpen() );
        grid->GetCsvExportController().ClearTargets();
        grid->GetCsvExportController().AddTarget( ( GUCEF_NEW GUI::CNativeGridExportTarget( "Scratch", CORE::TempDir() ) )->CreateSharedPtr() );
        ClickAt( context, 10 + 790, 10 + 10 );
        ASSERT_TRUE( grid->GetCsvExportController().IsDialogOpen() && 0 == clicked.count );
        grid->GetCsvExportController().GetDialog()->Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridColumnChooser: Lists every column in order with its state, applies ticks, moves and reset at once, keeps required ticks and follows changes made elsewhere" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTextTableModel* model = MakeFiveColumns( 4 );
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );
        grid->SetProperty( "ShowColumnChooser", "true" );
        CEventCount visibility( grid, GUI::CGridView::ColumnVisibilityChangedEvent );
        CEventCount moved( grid, GUI::CGridView::ColumnMovedEvent );

        ASSERT_TRUE( grid->OpenColumnChooser() );
        GUI::CGridColumnChooserForm* chooser = grid->GetColumnChooser();
        ASSERT_TRUE( GUCEF_NULL != chooser );
        if ( GUCEF_NULL == chooser )
            return;
        CEventCount closed( chooser, GUI::CGridColumnChooserForm::ClosedEvent );
        GUI::CGridView* list = chooser->GetColumnGrid();
        ASSERT_TRUE( GUCEF_NULL != list && chooser->IsOpen() && grid->HasColumnDefaults() );
        if ( GUCEF_NULL == list )
            return;

        // every column, the hidden one too, with its tick, title and note
        ASSERT_TRUE( 5 == chooser->GetRowCount() && 5 == list->GetRowCount() );
        ASSERT_TRUE( "true" == CellTextOf( list, 0, 0 ) && "false" == CellTextOf( list, 1, 0 ) && "B" == CellTextOf( list, 1, 1 ) );
        ASSERT_TRUE( "Hidden by default" == CellTextOf( list, 1, 2 ) && "Always shown" == CellTextOf( list, 2, 2 ) && CellTextOf( list, 0, 2 ).IsNULLOrEmpty() );
        ASSERT_TRUE( !list->IsCellEditable( 2, 0 ) && list->IsCellEditable( 0, 0 ) && list->IsCellEditable( 1, 0 ) );

        // a tick shows the column at once
        ASSERT_TRUE( chooser->SetColumnShown( 1, true ) );
        ASSERT_TRUE( !grid->IsColumnHidden( 1 ) && 1 == visibility.count && "true" == CellTextOf( list, 1, 0 ) );
        ASSERT_FALSE( chooser->SetColumnShown( 2, false ) );        // required
        ASSERT_TRUE( !grid->IsColumnHidden( 2 ) && 1 == visibility.count );

        // a click on the check box of a row (the list is a grid in cell mode with editing)
        ASSERT_TRUE( ClickCell( context, list, 0, 0 ) );
        ASSERT_TRUE( grid->IsColumnHidden( 0 ) && 2 == visibility.count && "false" == CellTextOf( list, 0, 0 ) );

        // moves: the order of the grid, hidden columns too, the cursor goes with the row
        ASSERT_TRUE( chooser->MoveRow( 3, true ) );
        ASSERT_TRUE( "ABDCE" == OrderText( grid ) && 1 == moved.count && "D" == CellTextOf( list, 2, 1 ) && 2 == list->GetCursorRow() );
        ASSERT_TRUE( chooser->MoveRow( 0, false ) && "BADCE" == OrderText( grid ) );   // A is hidden and moves
        ASSERT_FALSE( chooser->MoveRow( 0, true ) );
        ASSERT_FALSE( chooser->MoveRow( 4, false ) );

        // the buttons: Move up with the cursor on D, then Reset to default
        list->SetCursor( 2, 0 );
        ClickAt( context, chooser->GetWidget( "MoveUp" )->GetRect().left + 10, chooser->GetWidget( "MoveUp" )->GetRect().top + 10 );
        ASSERT_TRUE( "BDACE" == OrderText( grid ) );
        ClickAt( context, chooser->GetWidget( "Reset" )->GetRect().left + 10, chooser->GetWidget( "Reset" )->GetRect().top + 10 );
        ASSERT_TRUE( "ABCDE" == OrderText( grid ) && "ACDE" == ShownColumns( grid ) );
        ASSERT_TRUE( "A" == CellTextOf( list, 0, 1 ) && "false" == CellTextOf( list, 1, 0 ) );

        // a change made elsewhere shows in the list
        grid->SetColumnHidden( 3, true );
        ASSERT_TRUE( "false" == CellTextOf( list, 3, 0 ) );

        // Escape closes
        context->SetFocus( list );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !chooser->IsOpen() && 1 == closed.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridPreferences: A record goes through a DataNode and back; no version is refused, a newer version is read for what is known, unknown parts are ignored" )
    try
    {
        GUI::CGridPreferences record;
        record.gridId = "Test.Grid";
        record.columns.push_back( GUI::CGridPreferences::SColumn( "symbol", true, 120 ) );
        record.columns.push_back( GUI::CGridPreferences::SColumn( "cost", false, 0 ) );
        record.columns.push_back( GUI::CGridPreferences::SColumn( "na\xC3\xAFve, \"quoted\"", true, 64 ) );
        record.hasWidths = true;
        record.hasSort = true;
        record.sortKey = "symbol";
        record.sortAscending = false;
        record.exportRoot = "This computer";
        record.exportDirectory = "/home/user/exports";
        record.defaultsRevision = "2";

        CORE::CDataNode tree;
        ASSERT_TRUE( record.SaveTo( tree ) );
        ASSERT_TRUE( "2" == tree.FindChild( "GridPreferences" )->GetAttributeValue( "defaultsRevision" ).AsString() );
        GUI::CGridPreferences read;
        ASSERT_TRUE( read.LoadFrom( tree ) && read == record );             // from the parent
        ASSERT_TRUE( "2" == read.defaultsRevision );
        GUI::CGridPreferences other = record;
        other.defaultsRevision = "3";
        ASSERT_FALSE( other == record );                                    // the revision is part of the record
        ASSERT_TRUE( read.LoadFrom( *tree.FindChild( "GridPreferences" ) ) && read == record );
        ASSERT_TRUE( GUI::CGridPreferences::CurrentVersion == read.version );

        // no version, version 0, no record at all: refused and cleared
        CORE::CDataNode* node = tree.FindChild( "GridPreferences" );
        node->DelAttribute( "version" );
        ASSERT_FALSE( read.LoadFrom( tree ) );
        ASSERT_TRUE( read.columns.empty() && !read.hasSort );
        node->SetAttribute( "version", (UInt32) 0 );
        ASSERT_FALSE( read.LoadFrom( tree ) );
        CORE::CDataNode empty;
        ASSERT_FALSE( read.LoadFrom( empty ) );

        // a newer version with things this one does not know: read for what it knows
        node->SetAttribute( "version", (UInt32) 7 );
        node->SetAttribute( "futureSetting", "x" );
        CORE::CDataNode* extra = node->AddChild( "Filter" );
        extra->SetAttribute( "column", "cost" );
        extra->SetAttribute( "op", "gt" );
        CORE::CDataNode* repeated = node->AddChild( "Column" );
        repeated->SetAttribute( "key", "symbol" );          // a repeated key counts once
        repeated->SetAttribute( "shown", false );
        repeated->SetAttribute( "width", (Int32) 5 );
        CORE::CDataNode* keyless = node->AddChild( "Column" );
        keyless->SetAttribute( "shown", true );
        keyless->SetAttribute( "width", (Int32) 5 );
        ASSERT_TRUE( read.LoadFrom( tree ) );
        ASSERT_TRUE( 7 == read.version && 3 == read.columns.size() && read.columns[ 0 ].shown && 120 == read.columns[ 0 ].width );
        ASSERT_TRUE( "/home/user/exports" == read.exportDirectory && read.hasSort && !read.sortAscending );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridPreferences: The local store keeps a file per grid written atomically, reads it with either codec, refuses damaged files and keeps hostile ids in its directory" )
    try
    {
        const CORE::CString dir = ScratchDir( "gucefGUI_TestApp_GridPreferences" );
        GUI::CGridPreferences record;
        record.columns.push_back( GUI::CGridPreferences::SColumn( "a", true, 80 ) );
        record.columns.push_back( GUI::CGridPreferences::SColumn( "b", false, 0 ) );
        record.hasWidths = true;
        record.exportRoot = "Data";

        // the built in binary codec
        GUI::CLocalGridPreferenceStore binary( dir, "dnc" );
        ASSERT_TRUE( "dnc" == binary.GetEffectiveCodecName() );
        GUI::CGridPreferences read;
        ASSERT_FALSE( binary.Load( "Test.Grid", read ) );                 // nothing yet
        ASSERT_TRUE( binary.Save( "Test.Grid", record ) );
        ASSERT_TRUE( CORE::FileExists( binary.GetRecordPath( "Test.Grid" ) ) && "Test.Grid.dnc" == CORE::ExtractFilename( binary.GetRecordPath( "Test.Grid" ) ) );
        ASSERT_TRUE( binary.Load( "Test.Grid", read ) );
        record.gridId = "Test.Grid";
        ASSERT_TRUE( read == record );
        ASSERT_TRUE( binary.Save( "Test.Grid", record ) );                 // replaced in one step: one file, no temporary left
        ASSERT_TRUE( 1 == FilesIn( dir ).size() );

        // damaged: garbage, an empty file, another tree; not loaded, and the next save replaces it
        const CORE::CString path = binary.GetRecordPath( "Test.Grid" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, "this is not a data node tree \x01\x02\x03" ) );
        ASSERT_FALSE( binary.Load( "Test.Grid", read ) );
        ASSERT_TRUE( read.columns.empty() );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, "" ) );
        ASSERT_FALSE( binary.Load( "Test.Grid", read ) );
        {
            CORE::CDataNode other;
            other.AddChild( "SomethingElse" )->SetAttribute( "version", (UInt32) 1 );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "dnc", codec, false ) );
            ASSERT_TRUE( codec->StoreDataTree( &other, path ) );
        }
        ASSERT_FALSE( binary.Load( "Test.Grid", read ) );
        ASSERT_TRUE( binary.Save( "Test.Grid", record ) && binary.Load( "Test.Grid", read ) && read == record );

        // too large to be a record
        {
            CORE::CString big;
            for ( UInt32 i=0; i<1100; ++i )
                big += "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123";
            ASSERT_TRUE( CORE::WriteStringAsTextFile( path, big ) && CORE::FileSize( path ) > GUI::CLocalGridPreferenceStore::MaxRecordBytes );
            ASSERT_FALSE( binary.Load( "Test.Grid", read ) );
        }

        // hostile ids: no separators, no leading dot, no device names, a long id cut with a hash; each one a file of its own in the directory
        const char* const hostile[] = { "../../etc/passwd", "a/b\\c:d", "CON", "nul", ".hidden", "", "\xC3\xA9t\xC3\xA9 grid", "x%2Fy" };
        CORE::CString longId;
        for ( UInt32 i=0; i<300; ++i )
            longId += 'q';
        CORE::CString::StringVector ids( hostile, hostile + ( sizeof( hostile ) / sizeof( hostile[ 0 ] ) ) );
        ids.push_back( longId );
        ids.push_back( longId + "r" );
        CORE::CString::StringSet names;
        for ( UInt32 i=0; i<(UInt32)ids.size(); ++i )
        {
            const CORE::CString name = GUI::CLocalGridPreferenceStore::GetRecordFileName( ids[ i ] );
            ASSERT_TRUE( name.HasChar( '/' ) < 0 && name.HasChar( '\\' ) < 0 && name.HasChar( ':' ) < 0 && !name.IsNULLOrEmpty() && '.' != name[ 0 ] );
            ASSERT_TRUE( name.Length() <= GUI::CLocalGridPreferenceStore::MaxFileNameBytes + 17 );
            names.insert( name );
            const CORE::CString recordPath = binary.GetRecordPath( ids[ i ] );
            ASSERT_TRUE( CORE::CombinePath( dir, CORE::ExtractFilename( recordPath ) ) == recordPath );
            ASSERT_TRUE( binary.Save( ids[ i ], record ) && binary.Load( ids[ i ], read ) );
        }
        ASSERT_TRUE( "CON_" == GUI::CLocalGridPreferenceStore::GetRecordFileName( "CON" ) && "%2Ehidden" == GUI::CLocalGridPreferenceStore::GetRecordFileName( ".hidden" ) );
        ASSERT_TRUE( names.size() == ids.size() );                          // no two ids share a file
        ASSERT_TRUE( FilesIn( dir ).size() == ids.size() + 1 );
        ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( CORE::TempDir(), "etc/passwd.dnc" ) ) );

        // remove
        ASSERT_TRUE( binary.Remove( "Test.Grid" ) && !binary.Load( "Test.Grid", read ) );

        // JSON when its codec plugin is there; a store of the other codec still finds the record, and its save leaves one file
        if ( LoadLayoutCodecs() )
        {
            GUI::CLocalGridPreferenceStore json( dir );
            ASSERT_TRUE( "json" == json.GetEffectiveCodecName() );
            ASSERT_TRUE( json.Save( "Json.Grid", record ) && CORE::FileExists( CORE::CombinePath( dir, "Json.Grid.json" ) ) );
            ASSERT_TRUE( json.Load( "Json.Grid", read ) );
            record.gridId = "Json.Grid";
            ASSERT_TRUE( read == record );
            ASSERT_TRUE( binary.Load( "Json.Grid", read ) && read == record );
            ASSERT_TRUE( binary.Save( "Json.Grid", record ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, "Json.Grid.dnc" ) ) && !CORE::FileExists( CORE::CombinePath( dir, "Json.Grid.json" ) ) );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridPreferences: the JSON codec plugin is not there; the JSON round trip was not run" );
        }
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridPreferences: A grid applies the record of its id, ignores unknown keys, adds new columns, keeps required ones shown, saves each change once and a resize drag once" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CMemoryPreferenceStore* memory = GUCEF_NEW CMemoryPreferenceStore();
        GUI::TGridPreferenceStorePtr store = memory->CreateSharedPtr();

        GUI::CGridPreferences stored;
        stored.columns.push_back( GUI::CGridPreferences::SColumn( "e", true, 150 ) );
        stored.columns.push_back( GUI::CGridPreferences::SColumn( "gone", true, 90 ) );     // a column this version no longer has
        stored.columns.push_back( GUI::CGridPreferences::SColumn( "a", false, 0 ) );
        stored.columns.push_back( GUI::CGridPreferences::SColumn( "c", false, 0 ) );        // required: shown anyway
        stored.hasWidths = true;
        stored.hasSort = true;
        stored.sortKey = "d";
        stored.sortAscending = false;
        memory->records[ "Test.Columns" ] = stored;

        CTextTableModel* model = MakeFiveColumns( 6 );
        model->sortable = true;
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );
        grid->SetProperty( "FitColumns", "false" );     // the widths of the record stay pixels
        grid->SetProperty( "ResizableColumns", "true" );
        grid->SetPreferenceStore( store, "Test.Columns" );

        // applied: the order of the record, then b and d in the order of the data; b hidden by default, c required
        ASSERT_TRUE( "EACBD" == OrderText( grid ) && "ECD" == ShownColumns( grid ) );
        ASSERT_TRUE( 150 == grid->GetColumnWidth( 4 ) );
        ASSERT_TRUE( 3 == model->sortedColumn && !model->sortedAscending && 3 == grid->GetIntProperty( "SortColumn" ) );
        ASSERT_TRUE( 0 == memory->saves );

        // every change is saved, once
        ASSERT_TRUE( grid->SetColumnHidden( 3, true ) );
        ASSERT_TRUE( 1 == memory->saves && !memory->records[ "Test.Columns" ].columns[ 4 ].shown && "d" == memory->records[ "Test.Columns" ].columns[ 4 ].key );
        grid->MoveColumn( 2, 0 );
        ASSERT_TRUE( 2 == memory->saves && "c" == memory->records[ "Test.Columns" ].columns[ 0 ].key );
        ASSERT_TRUE( grid->SortRows( 0, true ) );
        ASSERT_TRUE( 3 == memory->saves && "a" == memory->records[ "Test.Columns" ].sortKey && memory->records[ "Test.Columns" ].sortAscending );
        ASSERT_TRUE( -1 == memory->records[ "Test.Columns" ].FindColumn( "gone" ) );

        // a resize by dragging: saved once, when the button goes up
        Int32 x = 0, y = 0;
        ASSERT_TRUE( BorderPoint( context, grid, 2, x, y ) );
        const Int32 widthBefore = grid->GetColumnWidth( 2 );
        DragFrom( context, x, y, x + 60, y );
        ASSERT_TRUE( grid->GetColumnWidth( 2 ) > widthBefore && 4 == memory->saves );
        ASSERT_TRUE( memory->records[ "Test.Columns" ].hasWidths && grid->GetColumnWidth( 2 ) == memory->records[ "Test.Columns" ].columns[ 0 ].width );

        // a refused save: the event, the grid works on
        CEventCount failed( grid, GUI::CGridView::PreferencesSaveFailedEvent );
        memory->refuse = true;
        ASSERT_TRUE( grid->SetColumnHidden( 3, false ) && !grid->IsColumnHidden( 3 ) );
        ASSERT_TRUE( 1 == failed.count );
        memory->refuse = false;

        // the store before the model: applied when the model comes, nothing saved before
        memory->saves = 0;
        GUI::CGridView* later = MakeExportGrid( context, GUI::TTableViewModelPtr() );
        later->SetPreferenceStore( store, "Test.Columns" );
        later->SetProperty( "SortColumn", "1" );        // a change before there are columns is not saved over the record
        ASSERT_TRUE( 0 == memory->saves );
        CTextTableModel* six = MakeFiveColumns( 2 );
        six->AddColumn( "F", CORE::DATATABLE_TYPE_UTF8, "f" );
        later->SetModel( GUI::TTableViewModelPtr( six ) );
        ASSERT_TRUE( "CAEBDF" == OrderText( later ) && 0 == memory->saves );      // the new column f at the end, shown
        ASSERT_TRUE( !later->IsColumnHidden( 5 ) && !later->IsColumnHidden( 2 ) );

        // reset saves the defaults
        grid->ResetColumnsToDefault();
        ASSERT_TRUE( 1 == memory->saves && "a" == memory->records[ "Test.Columns" ].columns[ 0 ].key );

        // without a store nothing is saved and the members still work
        grid->SetPreferenceStore( GUI::TGridPreferenceStorePtr(), CORE::CString::Empty );
        grid->SetColumnHidden( 0, true );
        ASSERT_TRUE( 1 == memory->saves && grid->IsColumnHidden( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridCsvExport: The text of the view: quoting of delimiters, quotes, line breaks and blanks, UTF-8, empty and null cells, header, column order, hidden columns, sort, filter, raw values and the options" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTextTableModel* model = MakeQuotingModel();
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr( model ) );
        GUI::CGridCsvOptions options;

        const CORE::CString expected =
            "Name,Note,Count\r\n"
            "plain,\"a,b\",\"1,234,567\"\r\n"
            "\"say \"\"hi\"\"\",\"line1\nline2\",5\r\n"
            "\" lead\",\"trail \",\r\n"
            ",\"cr\rx\",0\r\n"
            "\xC3\xA9t\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC,tab\tin,42\r\n";
        GUI::CGridCsvExportReport report;
        ASSERT_TRUE( expected == ExportText( grid, options, &report ) );
        ASSERT_TRUE( report.success && 5 == report.rowCount && 3 == report.columnCount && (UInt64) expected.ByteSize() - 1 == report.byteCount );

        // the order shown and without the hidden columns
        grid->MoveColumn( 2, 0 );
        ASSERT_TRUE( grid->SetColumnHidden( 1, true ) );
        ASSERT_TRUE( "Count,Name\r\n\"1,234,567\",plain\r\n5,\"say \"\"hi\"\"\"\r\n,\" lead\"\r\n0,\r\n42,\xC3\xA9t\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC\r\n" == ExportText( grid, options ) );

        // sorted and filtered by the model: the rows as the grid has them
        model->sortable = true;
        ASSERT_TRUE( grid->SortRows( 2, false ) );
        model->SetFilter( 2, true );        // rows with a count
        ASSERT_TRUE( "Count,Name\r\n\"1,234,567\",plain\r\n42,\xC3\xA9t\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC\r\n5,\"say \"\"hi\"\"\"\r\n0,\r\n" == ExportText( grid, options ) );

        // raw values: the value of the model without the grouping
        options.rawValues = true;
        ASSERT_TRUE( "Count,Name\r\n1234567,plain\r\n42,\xC3\xA9t\xC3\xA9 \xE6\x97\xA5\xE6\x9C\xAC\r\n5,\"say \"\"hi\"\"\"\r\n0,\r\n" == ExportText( grid, options ) );
        options.rawValues = false;
        model->SetFilter( -1, false );
        grid->ResetProperty( "HiddenColumns" );
        grid->ResetProperty( "ColumnOrder" );
        model->ClearSort();

        // delimiter, line ending, byte order mark, no header
        options.delimiter = ';';
        ASSERT_TRUE( ExportText( grid, options ).StartsWith( "Name;Note;Count\r\nplain;a,b;1,234,567\r\n" ) );
        options.delimiter = '\t';
        const CORE::CString tabbed = ExportText( grid, options );
        ASSERT_TRUE( tabbed.StartsWith( "Name\tNote\tCount\r\nplain\ta,b\t1,234,567\r\n" ) && tabbed.HasSubstr( "\"tab\tin\"" ) >= 0 );
        options.delimiter = ',';
        options.lineEnding = GUI::CGridCsvOptions::LINEENDING_LF;
        ASSERT_TRUE( ExportText( grid, options ).StartsWith( "Name,Note,Count\nplain,\"a,b\",\"1,234,567\"\n\"say \"\"hi\"\"\",\"line1\nline2\",5\n" ) );
        options.lineEnding = GUI::CGridCsvOptions::LINEENDING_CRLF;
        options.byteOrderMark = true;
        const CORE::CString withBom = ExportText( grid, options );
        ASSERT_TRUE( withBom.StartsWith( "\xEF\xBB\xBFName,Note,Count\r\n" ) );
        options.byteOrderMark = false;
        options.includeHeader = false;
        ASSERT_TRUE( ExportText( grid, options ).StartsWith( "plain,\"a,b\",\"1,234,567\"\r\n" ) );
        options.includeHeader = true;
        options.delimiter = '"';
        CORE::CString error;
        ASSERT_FALSE( options.Validate( error ) );
        ASSERT_TRUE( ExportText( grid, options ).StartsWith( "<failed" ) );
        options.delimiter = ',';

        // a string list: the text of its cells, an empty title, check boxes of a model as true and false
        GUI::CGridView* list = MakeExportGrid( context, GUI::TTableViewModelPtr() );
        list->SetProperty( "Columns", "\nB" );
        list->SetProperty( "Rows", "x,1\ty\nz\t" );
        ASSERT_TRUE( ",B\r\n\"x,1\",y\r\nz,\r\n" == ExportText( list, options ) );
        CTextTableModel* flags = GUCEF_NEW CTextTableModel();
        flags->AddColumn( "On", CORE::DATATABLE_TYPE_BOOL, "on" );
        GUCEF::vector< CORE::CVariant > row;
        row.push_back( CORE::CVariant( true ) );
        flags->AddRow( row );
        row[ 0 ] = CORE::CVariant( false );
        flags->AddRow( row );
        list->SetModel( GUI::TTableViewModelPtr( flags ) );
        ASSERT_TRUE( "On\r\ntrue\r\nfalse\r\n" == ExportText( list, options ) );

        // no rows: the header only
        CTextTableModel* none = GUCEF_NEW CTextTableModel();
        none->AddColumn( "Only", CORE::DATATABLE_TYPE_UTF8, "only" );
        list->SetModel( GUI::TTableViewModelPtr( none ) );
        ASSERT_TRUE( "Only\r\n" == ExportText( list, options ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridCsvExport: 200,000 rows are streamed in batches: the rows are read in order a batch at a time, the output is written as it goes, time and memory stay bounded" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const UInt32 rows = 200000;
        CGeneratedModel* model = GUCEF_NEW CGeneratedModel( rows );
        GUI::TTableViewModelPtr modelPtr( model );
        GUI::CGridView* grid = MakeExportGrid( context, modelPtr );

        GUI::CGridCsvOptions options;
        CCountingOutput* sink = GUCEF_NEW CCountingOutput();
        CORE::IOAccessPtr output( sink );
        GUI::CGridExportSource source( *grid, false );
        model->ResetCounters();
        const UInt64 memoryBefore = ResidentKiB();
        const UInt64 start = MT::PrecisionTickCount();
        GUI::CGridCsvExportReport report;
        ASSERT_TRUE( GUI::CGridCsvExporter::Export( source, output, options, report ) );
        const double seconds = Micros( MT::PrecisionTickCount() - start ) / 1000000.0;
        const UInt64 memoryAfter = ResidentKiB();
        const UInt64 grown = memoryAfter > memoryBefore ? memoryAfter - memoryBefore : 0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExportBenchmark: " << rows << " rows of " << model->GetColumnCount() << " columns, " << sink->bytes << " bytes in "
            << sink->writes << " writes (largest " << sink->largestWrite << " bytes), " << seconds << " s, resident memory grew " << grown << " KiB, "
            << model->prepares << " prepares of at most " << model->largestPrepare << " rows" );

        ASSERT_TRUE( report.success && rows == report.rowCount && sink->bytes == report.byteCount );
        ASSERT_TRUE( model->inOrder && rows == model->rowsRead );                  // every row once, in order
        ASSERT_TRUE( model->largestPrepare <= options.batchRowCount && model->prepares >= rows / options.batchRowCount );
        ASSERT_TRUE( sink->writes > 10 && sink->largestWrite < 4 * 1024 * 1024 && sink->largestWrite * 4 < sink->bytes );   // written as it goes, not at the end
        ASSERT_TRUE( 0 == memoryBefore || grown < 64 * 1024 );                     // no copy of the table: the output alone is larger
        ASSERT_TRUE( seconds < 120.0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridCsvExport: The save dialog flow writes the file chosen, asks before a file is replaced, cancel writes nothing, a failing target reports an error and leaves no file" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const CORE::CString dir = ScratchDir( "gucefGUI_TestApp_GridCsvExport" );
        GUI::CGridView* grid = MakeExportGrid( context, GUI::TTableViewModelPtr() );
        grid->SetProperty( "Columns", "Symbol\nQty" );
        grid->SetProperty( "Rows", "AAA\t10\nBBB\t20\nCCC\t30" );
        grid->SetProperty( "ShowCsvExport", "true" );
        CEventCount finished( grid, GUI::CGridView::CsvExportFinishedEvent );
        CEventCount cancelled( grid, GUI::CGridView::CsvExportCancelledEvent );
        CReportProbe probe( grid );
        CMemoryPreferenceStore* memory = GUCEF_NEW CMemoryPreferenceStore();
        grid->SetPreferenceStore( memory->CreateSharedPtr(), "Test.Export" );

        GUI::CGridCsvExportController& controller = grid->GetCsvExportController();
        controller.ClearTargets();
        controller.AddTarget( ( GUCEF_NEW GUI::CNativeGridExportTarget( "Scratch", dir ) )->CreateSharedPtr() );

        // the suggested name: the name of the grid, made safe; the options can give another
        ASSERT_TRUE( "Grid.csv" == controller.GetSuggestedFileName() );
        controller.GetOptions().suggestedFileName = " My: report/2026? ";
        ASSERT_TRUE( "My_ report_2026_.csv" == controller.GetSuggestedFileName() );
        ASSERT_TRUE( "table" == GUI::CGridCsvExportController::SanitizeFileName( "..." ) && "a_b" == GUI::CGridCsvExportController::SanitizeFileName( "a\nb" ) );
        controller.GetOptions().suggestedFileName = "positions";

        // the dialog: the file type, the name, Accept writes the file and tells
        ASSERT_TRUE( grid->OpenCsvExport() && controller.IsDialogOpen() );
        GUI::CFileSaveDialogForm* dialog = controller.GetDialog();
        ASSERT_TRUE( "positions.csv" == dialog->GetName() && 1 == dialog->GetFileTypes().size() && dir == dialog->GetCurrentDirectory() );
        ASSERT_TRUE( dialog->Accept() );
        const CORE::CString file = CORE::CombinePath( dir, "positions.csv" );
        ASSERT_TRUE( "Symbol,Qty\r\nAAA,10\r\nBBB,20\r\nCCC,30\r\n" == FileText( file ) );
        ASSERT_TRUE( 1 == finished.count && probe.last.success && 3 == probe.last.rowCount && file == probe.last.path && "Scratch" == probe.last.rootName );
        ASSERT_TRUE( GUI::CGridCsvExportController::EXPORTMODE_IMMEDIATE == controller.GetExportMode() );
        ASSERT_TRUE( GUCEF_NULL != controller.GetMessageBox() && controller.GetMessageBox()->IsOpen() );
        ASSERT_TRUE( probe.last.message.HasSubstr( "Exported 3 rows to" ) >= 0 );
        controller.GetMessageBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        ASSERT_TRUE( 1 == FilesIn( dir ).size() );                                  // no temporary file left
        ASSERT_TRUE( "Scratch" == grid->GetPreferences().exportRoot && dir == grid->GetPreferences().exportDirectory && "Scratch" == memory->records[ "Test.Export" ].exportRoot );

        // cancel: nothing written, the event
        ASSERT_TRUE( grid->OpenCsvExport() );
        controller.GetDialog()->SetName( "other.csv" );
        controller.GetDialog()->Cancel();
        ASSERT_TRUE( 1 == cancelled.count && 1 == finished.count && !CORE::FileExists( CORE::CombinePath( dir, "other.csv" ) ) );

        // replacing asks first: No leaves the file, Yes replaces it
        grid->SetProperty( "Rows", "DDD\t40" );
        ASSERT_TRUE( grid->OpenCsvExport() );
        dialog = controller.GetDialog();
        ASSERT_FALSE( dialog->Accept() );
        ASSERT_TRUE( dialog->GetMessageBox().IsOpen() );
        dialog->GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_NO );
        ASSERT_TRUE( 1 == finished.count && "Symbol,Qty\r\nAAA,10\r\nBBB,20\r\nCCC,30\r\n" == FileText( file ) );
        ASSERT_FALSE( dialog->Accept() );
        dialog->GetMessageBox().Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES );
        ASSERT_TRUE( 2 == finished.count && "Symbol,Qty\r\nDDD,40\r\n" == FileText( file ) );
        controller.GetMessageBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );

        // failing targets: at the open, while writing, at the commit; an error is told, no file is made, nothing temporary stays
        const CFailingTarget::EFailure failures[] = { CFailingTarget::FAIL_OPEN, CFailingTarget::FAIL_WRITE, CFailingTarget::FAIL_COMMIT };
        for ( UInt32 i=0; i<3; ++i )
        {
            CFailingTarget* failing = GUCEF_NEW CFailingTarget( dir, failures[ i ] );
            GUI::TGridExportTargetPtr failingPtr = failing->CreateSharedPtr();
            controller.ClearTargets();
            controller.AddTarget( failingPtr );
            ASSERT_TRUE( grid->OpenCsvExport() );
            controller.GetDialog()->SetName( "failed.csv" );
            ASSERT_TRUE( controller.GetDialog()->Accept() );
            ASSERT_TRUE( 3 + i == finished.count && !probe.last.success && probe.last.message.HasSubstr( "could not be exported" ) >= 0 );
            ASSERT_TRUE( controller.GetMessageBox()->IsOpen() && !CORE::FileExists( CORE::CombinePath( dir, "failed.csv" ) ) );
            ASSERT_TRUE( 1 == FilesIn( dir ).size() && !controller.IsExporting() );
            if ( CFailingTarget::FAIL_WRITE == failures[ i ] )
                ASSERT_TRUE( 1 == failing->abandoned );
            controller.GetMessageBox()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
        }

        // without the dialog, and without the message
        controller.ClearTargets();
        GUI::TGridExportTargetPtr scratch = ( GUCEF_NEW GUI::CNativeGridExportTarget( "Scratch", dir ) )->CreateSharedPtr();
        controller.AddTarget( scratch );
        controller.GetOptions().showResultMessage = false;
        ASSERT_TRUE( controller.ExportTo( scratch, CORE::CombinePath( dir, "direct.csv" ) ) );
        ASSERT_TRUE( "Symbol,Qty\r\nDDD,40\r\n" == FileText( CORE::CombinePath( dir, "direct.csv" ) ) && !controller.GetMessageBox()->IsOpen() );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridCsvExport: A large table is written in slices over frames; a change of the rows during it ends it with an error and no file; an export on a pool gives the same file" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1024, 768 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const CORE::CString dir = ScratchDir( "gucefGUI_TestApp_GridCsvSliced" );
        const UInt32 rows = 30000;
        CGeneratedModel* model = GUCEF_NEW CGeneratedModel( rows );
        GUI::TTableViewModelPtr modelPtr( model );
        GUI::CGridView* grid = MakeExportGrid( context, modelPtr );
        model->counting = false;            // the pool reads it too: the model is not changed by a read
        CReportProbe probe( grid );
        GUI::CGridCsvExportController& controller = grid->GetCsvExportController();
        GUI::TGridExportTargetPtr scratch = ( GUCEF_NEW GUI::CNativeGridExportTarget( "Scratch", dir ) )->CreateSharedPtr();
        controller.ClearTargets();
        controller.AddTarget( scratch );
        controller.GetOptions().showResultMessage = false;
        controller.GetOptions().immediateCellLimit = 1000;
        controller.GetOptions().cellsPerFrame = 20000;

        // the same text at once, for comparison
        CMemoryOutput reference;
        GUI::CGridExportSource referenceSource( *grid, false );
        GUI::CGridCsvExportReport referenceReport;
        ASSERT_TRUE( GUI::CGridCsvExporter::Export( referenceSource, reference.Access(), controller.GetOptions(), referenceReport ) );

        // sliced: the export runs over frames
        const CORE::CString sliced = CORE::CombinePath( dir, "sliced.csv" );
        ASSERT_TRUE( controller.ExportTo( scratch, sliced ) );
        ASSERT_TRUE( controller.IsExporting() && GUI::CGridCsvExportController::EXPORTMODE_SLICED == controller.GetExportMode() && !CORE::FileExists( sliced ) );
        UInt32 frames = 0;
        while ( controller.IsExporting() && frames < 1000 )
        {
            context->RenderFrame( 16 );
            ++frames;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExport: the sliced export of " << rows << " rows took " << frames << " frames" );
        ASSERT_TRUE( !controller.IsExporting() && frames > 1 && probe.last.success && rows == probe.last.rowCount );
        ASSERT_TRUE( reference.Text() == FileText( sliced ) );

        // a change of the rows while it runs
        const CORE::CString changed = CORE::CombinePath( dir, "changed.csv" );
        ASSERT_TRUE( controller.ExportTo( scratch, changed ) );
        context->RenderFrame( 16 );
        ASSERT_TRUE( controller.IsExporting() );
        model->InsertRows( 0, 1 );
        frames = 0;
        while ( controller.IsExporting() && frames < 1000 )
        {
            context->RenderFrame( 16 );
            ++frames;
        }
        ASSERT_TRUE( !probe.last.success && probe.last.message.HasSubstr( "changed" ) >= 0 && !CORE::FileExists( changed ) );
        ASSERT_TRUE( FilesIn( dir ).size() == 1 );
        model->RemoveRows( 0, 1 );

        // cancelled
        ASSERT_TRUE( controller.ExportTo( scratch, changed ) );
        controller.CancelExport();
        ASSERT_TRUE( !controller.IsExporting() && probe.last.cancelled && !CORE::FileExists( changed ) );

        // on a pool, straight: a source of the model with its columns, the same text
        GUI::CGridExportSource::TInt32Vector columns;
        GUI::CGridExportSource::TStringVector titles;
        for ( UInt32 c=0; c<model->GetColumnCount(); ++c )
        {
            columns.push_back( (Int32) c );
            titles.push_back( model->GetColumn( c ).title );
        }
        GUI::TGridExportSourcePtr detached = ( GUCEF_NEW GUI::CGridExportSource( modelPtr, columns, titles, false ) )->CreateSharedPtr();
        ASSERT_TRUE( detached->IsDetached() );
        CMemoryOutput pooled;
        CORE::CFutureResult future = GUI::CGridCsvExporter::ExportAsync( detached, pooled.Access(), controller.GetOptions() );
        ASSERT_FALSE( future.HasNoFuture() );
        future.GetResult();
        UInt32 waits = 0;
        while ( !future.IsReady() && waits < 6000 )
        {
            MT::ThreadDelay( 10 );
            ++waits;
        }
        GUI::CGridCsvExportReport pooledReport;
        ASSERT_TRUE( GUI::CGridCsvExporter::GetAsyncResult( future, pooledReport ) && pooledReport.success && rows == pooledReport.rowCount );
        ASSERT_TRUE( reference.Text() == pooled.Text() );

        // on a pool through the flow of the grid (the model is read only and may be read from a thread)
        controller.GetOptions().useThreadPool = true;
        const CORE::CString onPool = CORE::CombinePath( dir, "pool.csv" );
        ASSERT_TRUE( controller.ExportTo( scratch, onPool ) );
        ASSERT_TRUE( GUI::CGridCsvExportController::EXPORTMODE_THREADPOOL == controller.GetExportMode() );
        frames = 0;
        while ( controller.IsExporting() && frames < 6000 )
        {
            context->RenderFrame( 16 );
            MT::ThreadDelay( 1 );
            ++frames;
        }
        ASSERT_TRUE( !controller.IsExporting() && probe.last.success && reference.Text() == FileText( onPool ) );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
