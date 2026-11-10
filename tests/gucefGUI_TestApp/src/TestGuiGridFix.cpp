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

/** A grid of 800 x 300 (14 rows in view) with the model, in a context of 1280 x 960 */
GUI::CGridView*
MakeFixGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetModel( model );
    context->FlushChanges();
    return grid;
}

/** A model that draws its own text for the name column and the footer, and keeps its values typed */
class CContentModel : public CTestModel
{
    public:

    explicit CContentModel( UInt32 rows ) : CTestModel( rows ) {}

    virtual bool GetCellContent( UInt32 row, UInt32 column, GUI::SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COL_NAME == column && row < GetRowCount() )
        {
            content.kind = GUI::SCellContent::KIND_TEXT;
            content.text = CORE::CString( "N" ) + CORE::UInt32ToString( IdOf( row ) ) + "!";
            content.isNumber = false;
            return true;
        }
        return CTestModel::GetCellContent( row, column, content );
    }

    virtual bool GetFooterContent( UInt32 column, GUI::SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COL_NAME == column && HasFooter() )
        {
            content.kind = GUI::SCellContent::KIND_TEXT;
            content.text = CORE::CString( "Sum " ) + CORE::UInt32ToString( GetRowCount() );
            content.isNumber = false;
            return true;
        }
        return CTestModel::GetFooterContent( column, content );
    }
};

/** The time of the steady frames of a grid with the model, in microseconds per frame */
double
SteadyFrame( const GUI::TTableViewModelPtr& model, UInt32 frames )
{
    GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return 0.0;
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 1000, 600 );
    grid->SetModel( model );
    grid->SetIntProperty( "ScrollPosition", (Int32) model->GetRowCount() / 2 );
    PaintedCommands( context, grid );
    const UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<frames; ++i )
        PaintedCommands( context, grid );
    return Micros( MT::PrecisionTickCount() - start ) / (double) frames;
}

/** Whether the accent bar of a row that stands out is drawn at the left edge of the row */
bool
HasEmphasisBar( const GUI::CDrawSurface::TCommandVector& commands, GUI::CGridView* grid, Int32 row )
{
    const Int32 top = grid->GetRowTop( row );
    if ( top < 0 )
        return false;
    GUI::CColour colour;
    return HasFill( commands, GUI::CRect( 0, top, 4, grid->GetRowHeight() ), &colour ) && 74 == colour.r && 232 == colour.b;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridFixTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridFix" );

    GUCEF_TESTFW_TESTCASE( "GridFix 1: The content of a cell and of a footer cell comes from the model, the grid draws it, and the text of a cell is the text that is drawn" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CContentModel* raw = GUCEF_NEW CContentModel( 100 );
        raw->footer = true;
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );

        // the grid draws what the model says for the name column, and what the column definition says for the others
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "N0!" ) && GUCEF_NULL != FindText( commands, "N12!" ) && GUCEF_NULL == FindText( commands, "name 0" ) );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "$0.25" ) );

        // the text of a cell is the text that is drawn, for every way to read it; the typed value is still the value
        ASSERT_TRUE( "N3!" == raw->GetCellText( 3, CTestModel::COL_NAME ) && "N3!" == grid->GetText( CTestModel::COL_NAME, 3 ) );
        ASSERT_TRUE( "$0.25" == raw->GetCellText( 1, CTestModel::COL_PRICE ) && "$0.25" == grid->GetText( CTestModel::COL_PRICE, 1 ) );
        CORE::CVariant value;
        ASSERT_TRUE( raw->GetCellValue( 3, CTestModel::COL_NAME, value ) && "name 3" == value.AsString() );
        GUI::SCellContent content;
        ASSERT_TRUE( grid->GetCellContent( 5, CTestModel::COL_NAME, content ) && "N5!" == content.text && GUI::SCellContent::KIND_TEXT == content.kind );
        ASSERT_TRUE( (UInt32) CTestModel::COLUMNS == grid->GetRowCells( 2 ).size() && "N2!" == grid->GetRowCells( 2 )[ CTestModel::COL_NAME ] );

        // the footer: drawn from the content, its text is the same text
        ASSERT_TRUE( grid->IsFooterShown() );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Sum 100" ) && "Sum 100" == raw->GetFooterText( CTestModel::COL_NAME ) && "Sum 100" == grid->GetFooterText( CTestModel::COL_NAME ) );
        ASSERT_TRUE( "100" == raw->GetFooterText( CTestModel::COL_ID ) && GUCEF_NULL != FindText( commands, "100" ) );
        ASSERT_TRUE( raw->GetFooterText( CTestModel::COL_DATE ).IsNULLOrEmpty() );
        GUI::SCellContent footerContent;
        ASSERT_TRUE( grid->GetFooterContent( CTestModel::COL_NAME, footerContent ) && "Sum 100" == footerContent.text && !grid->GetFooterContent( CTestModel::COL_DATE, footerContent ) );

        // a model that gives values and definitions only is drawn and copied by the same text as before
        CTestModel* plain = GUCEF_NEW CTestModel( 10 );
        GUI::TTableViewModelPtr plainModel( plain );
        ASSERT_TRUE( "name 4" == plain->GetCellText( 4, CTestModel::COL_NAME ) && "true" == plain->GetCellText( 5, CTestModel::COL_FLAG ) && "$1.25" == plain->GetCellText( 5, CTestModel::COL_PRICE ) );
        plain->footer = true;
        ASSERT_TRUE( "10" == plain->GetFooterText( CTestModel::COL_ID ) && "Total" == plain->GetFooterText( CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 1: A frame of a grid whose model gives the content costs what a frame of a plain model costs, whatever the number of rows (the numbers are in the log)" )
    try
    {
        double small = 0.0, large = 0.0;
        {
            GUI::TTableViewModelPtr model( GUCEF_NEW CContentModel( 1000 ) );
            small = SteadyFrame( model, 20 );
        }
        {
            GUI::TTableViewModelPtr model( GUCEF_NEW CContentModel( 100000 ) );
            large = SteadyFrame( model, 20 );
        }
        double plain = 0.0;
        {
            GUI::TTableViewModelPtr model( GUCEF_NEW CTestModel( 100000 ) );
            plain = SteadyFrame( model, 20 );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: model content frame, 1000 rows " << (UInt64) small << " us, 100000 rows " << (UInt64) large << " us, plain model 100000 rows " << (UInt64) plain << " us" );
        ASSERT_TRUE( small > 0.0 && large > 0.0 && plain > 0.0 );
        ASSERT_TRUE( large < small * 4.0 + 3000.0 );
        ASSERT_TRUE( large < plain * 3.0 + 3000.0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 2 and 3: The footer shows when the model has one, ShowFooter set on the grid wins, and a footer event has a meaning of its own" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 100 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );
        ASSERT_TRUE( !grid->IsFooterShown() && -1 == grid->GetFooterTop() && !grid->IsPropertySet( "ShowFooter" ) && 14 == grid->GetVisibleRowCount() );

        // the model has a footer: nobody sets ShowFooter
        raw->footer = true;
        raw->RaiseFooterChanged( 0, raw->GetColumnCount() );
        ASSERT_TRUE( grid->IsFooterShown() && 280 == grid->GetFooterTop() && 13 == grid->GetVisibleRowCount() );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Total" ) && GUI::CGridView::HIT_FOOTER == grid->HitTest( 10, 290 ).kind );

        // the footer takes a row from the rows that scroll: when it goes the position of the scroll follows (the last row stays the last row)
        grid->SetIntProperty( "ScrollPosition", grid->GetMaxScrollRow() );
        ASSERT_TRUE( 100 - 13 == grid->GetIntProperty( "ScrollPosition" ) );
        raw->footer = false;
        raw->RaiseFooterChanged( 0, raw->GetColumnCount() );
        ASSERT_TRUE( !grid->IsFooterShown() && 100 - 14 == grid->GetMaxScrollRow() && grid->GetIntProperty( "ScrollPosition" ) <= grid->GetMaxScrollRow() );
        raw->footer = true;
        raw->RaiseFooterChanged( 0, raw->GetColumnCount() );
        ASSERT_TRUE( grid->IsFooterShown() );

        // a footer cell that changed: the event names the columns, the grid draws the new text
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Total" ) );
        CEventCount redraws( grid, GUI::CGridView::ValueChangedEvent );
        raw->RaiseFooterChanged( CTestModel::COL_NAME, 1 );
        ASSERT_TRUE( grid->IsFooterShown() );

        // what was set on the grid wins, and a reset gives the decision back to the model
        grid->SetBoolProperty( "ShowFooter", false );
        ASSERT_TRUE( !grid->IsFooterShown() && -1 == grid->GetFooterTop() );
        grid->SetBoolProperty( "ShowFooter", true );
        ASSERT_TRUE( grid->IsFooterShown() );
        grid->ResetProperty( "ShowFooter" );
        ASSERT_TRUE( grid->IsFooterShown() );
        raw->footer = false;
        ASSERT_TRUE( !grid->IsFooterShown() );
        grid->SetBoolProperty( "ShowFooter", true );
        ASSERT_TRUE( !grid->IsFooterShown() );      // set to true: the model has none to show

        // the convention of the first version, cells changed at the row count, is still a redraw and does no harm
        raw->footer = true;
        raw->RaiseCells( raw->GetRowCount(), 0, 0, raw->GetColumnCount() );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Total" ) );

        // the text of a footer cell is a member of the model: the grid and the model say the same
        ASSERT_TRUE( "Total" == grid->GetFooterText( CTestModel::COL_NAME ) && "100" == grid->GetFooterText( CTestModel::COL_ID ) );
        raw->footer = false;
        ASSERT_TRUE( grid->GetFooterText( CTestModel::COL_NAME ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 4: A model says which rows stand out, and they stand out wherever the rows go; EmphasizedRows works as it did" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 100 );
        raw->emphasizedIds.insert( 3 );
        raw->emphasizedIds.insert( 40 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasEmphasisBar( commands, grid, 3 ) && !HasEmphasisBar( commands, grid, 2 ) && !HasEmphasisBar( commands, grid, 4 ) );
        ASSERT_TRUE( grid->IsRowEmphasized( 3 ) && grid->IsRowEmphasizedByModel( 3 ) && !grid->IsRowEmphasized( 4 ) && !grid->IsRowEmphasizedByModel( 40 + 1 ) );
        ASSERT_TRUE( grid->GetEmphasizedRows().empty() );

        // inserted rows: the row that stands out is the same row, now further down
        raw->InsertRows( 0, 5 );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasEmphasisBar( commands, grid, 8 ) && !HasEmphasisBar( commands, grid, 3 ) );

        // a new order: by key, there is nothing to carry over
        raw->Reverse();
        const Int32 reversed = 104 - 8;
        ASSERT_TRUE( 3 == raw->IdOf( (UInt32) reversed ) );
        grid->ScrollToRow( reversed );
        commands = PaintedCommands( context, grid );
        UInt32 stands = 0;
        for ( Int32 row = grid->GetFirstScrolledRow(); row < grid->GetFirstScrolledRow() + grid->GetVisibleRowCount() && row < (Int32) raw->GetRowCount(); ++row )
        {
            const bool bar = HasEmphasisBar( commands, grid, row );
            ASSERT_TRUE( bar == raw->IsRowEmphasized( (UInt32) row ) );
            if ( bar )
                ++stands;
        }
        ASSERT_TRUE( 1 == stands );

        // a refill of the rows: the ids that stand out are the ones that stay
        GUCEF::vector< UInt32 > ids;
        for ( UInt32 i=0; i<50; ++i )
            ids.push_back( 100 + i );
        ids.push_back( 40 );
        raw->Refill( ids, false );
        grid->ScrollToRow( 50 );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasEmphasisBar( commands, grid, 50 ) && raw->IsRowEmphasized( 50 ) && !raw->IsRowEmphasized( 49 ) );

        // the property works as before in the string list mode, and moves with the rows of a model that inserts and removes
        GUI::CGridView* listGrid = MakeGrid( context, grid->GetParentWidget(), 10, 400, 400, 200 );
        listGrid->SetProperty( "Columns", "A\nB" );
        GUI::CGridView::TRowVector rows;
        for ( UInt32 r=0; r<5; ++r )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( CORE::CString( "a" ) + CORE::UInt32ToString( r ) );
            cells.push_back( CORE::CString( "b" ) + CORE::UInt32ToString( r ) );
            rows.push_back( cells );
        }
        listGrid->SetRows( rows );
        GUI::CGridView::TInt32Vector emphasized;
        emphasized.push_back( 1 );
        emphasized.push_back( 3 );
        listGrid->SetEmphasizedRows( emphasized );
        commands = PaintedCommands( context, listGrid );
        ASSERT_TRUE( listGrid->IsRowEmphasized( 1 ) && listGrid->IsRowEmphasized( 3 ) && !listGrid->IsRowEmphasized( 2 ) && !listGrid->IsRowEmphasizedByModel( 1 ) );
        ASSERT_TRUE( HasEmphasisBar( commands, listGrid, 1 ) && HasEmphasisBar( commands, listGrid, 3 ) && !HasEmphasisBar( commands, listGrid, 2 ) );

        CTestModel* second = GUCEF_NEW CTestModel( 20 );
        GUI::TTableViewModelPtr secondModel( second );
        GUI::CGridView* modelGrid = MakeGrid( context, grid->GetParentWidget(), 420, 400, 400, 200 );
        modelGrid->SetModel( secondModel );
        GUI::CGridView::TInt32Vector property;
        property.push_back( 2 );
        modelGrid->SetEmphasizedRows( property );
        second->InsertRows( 0, 3 );
        ASSERT_TRUE( 1 == modelGrid->GetEmphasizedRows().size() && 5 == modelGrid->GetEmphasizedRows()[ 0 ] );
        second->RemoveRows( 0, 1 );
        ASSERT_TRUE( 1 == modelGrid->GetEmphasizedRows().size() && 4 == modelGrid->GetEmphasizedRows()[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 5: The read members of a model are const, so a model reached through the grid (CGridView::GetModel) can be read without a cast" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 30 );
        raw->footer = true;
        raw->titles = true;
        raw->emphasizedIds.insert( 2 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );

        const GUI::CGridView* constGrid = grid;
        const GUI::TTableViewModelPtr& held = constGrid->GetModel();
        const GUI::CITableViewModel& reader = *held;
        ASSERT_TRUE( 30 == reader.GetRowCount() && CTestModel::COLUMNS == reader.GetColumnCount() && "Name" == reader.GetColumn( CTestModel::COL_NAME ).title );
        CORE::CVariant value;
        ASSERT_TRUE( reader.GetCellValue( 7, CTestModel::COL_ID, value ) && 7 == value.AsInt32() && "7" == reader.GetCellText( 7, CTestModel::COL_ID ) );
        GUI::SCellContent content;
        ASSERT_TRUE( reader.GetCellContent( 7, CTestModel::COL_NAME, content ) && "name 7" == content.text );
        CORE::CVariant::VariantVector span;
        ASSERT_TRUE( reader.GetColumnSpan( CTestModel::COL_ID, 5, 3, span ) && 3 == span.size() );
        CORE::CVariant key;
        UInt32 row = 0;
        ASSERT_TRUE( reader.GetRowKey( 9, key ) && reader.FindRowByKey( key, row ) && 9 == row );
        CORE::CString title;
        ASSERT_TRUE( reader.GetRowTitle( 4, title ) && "R4" == title );
        ASSERT_TRUE( reader.IsRowEmphasized( 2 ) && !reader.IsRowEmphasized( 3 ) );
        ASSERT_TRUE( reader.HasFooter() && reader.GetFooterValue( CTestModel::COL_ID, value ) && "30" == reader.GetFooterText( CTestModel::COL_ID ) );
        ASSERT_TRUE( reader.GetFooterContent( CTestModel::COL_NAME, content ) && "Total" == content.text );
        ASSERT_TRUE( !reader.IsSortSupported() && reader.IsCellReadOnly( 1, 1 ) && !reader.CanUndo() && !reader.CanRedo() && !reader.IsInBatch() );
        ASSERT_TRUE( !reader.ValidateCellEdit( 1, 1, value ).IsAccepted() && GUI::CTableEditResult::STATUS_READ_ONLY == reader.ValidateCellEdit( 1, 1, value ).status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 6: A refill of 10,000 rows in one batch keeps the selection by row key and tells the observers of the selection once" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const UInt32 rowCount = 10000;
        CTestModel* raw = GUCEF_NEW CTestModel( rowCount );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );
        grid->SetAllowMultiSelect( true );
        GUI::CGridView::TInt32Vector selected;
        selected.push_back( 100 ); selected.push_back( 2000 ); selected.push_back( 9000 );
        grid->SetSelectedRows( selected );
        ASSERT_TRUE( 100 == grid->GetSelectedRow() && 3 == grid->GetSelectedRows().size() );
        CSelectionProbe probe( grid );

        // without a batch the refill takes the selection with the rows that go
        const GUCEF::vector< UInt32 > before = raw->GetIds();
        raw->Refill( before, false );
        ASSERT_TRUE( -1 == grid->GetSelectedRow() && grid->GetSelectedRows().empty() );
        ASSERT_TRUE( probe.snapshots.size() >= 1 );

        // in a batch: 50 rows come in front, every row that was selected is 50 places further down, and one notification says so
        grid->SetSelectedRows( selected );
        probe.snapshots.clear();
        GUCEF::vector< UInt32 > refill;
        for ( UInt32 i=0; i<50; ++i )
            refill.push_back( rowCount + i );
        refill.insert( refill.end(), before.begin(), before.end() );
        const UInt64 start = MT::PrecisionTickCount();
        raw->Refill( refill, true );
        const double refillMicros = Micros( MT::PrecisionTickCount() - start );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: refill of " << rowCount << " rows in one batch with a selection: " << (UInt64) refillMicros << " us" );
        ASSERT_TRUE( rowCount + 50 == grid->GetRowCount() && 150 == grid->GetSelectedRow() );
        GUI::CGridView::TInt32Vector now = grid->GetSelectedRows();
        ASSERT_TRUE( 3 == now.size() && 150 == now[ 0 ] && 2050 == now[ 1 ] && 9050 == now[ 2 ] );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( 1 == probe.snapshots.size() )
            ASSERT_TRUE( 150 == probe.snapshots[ 0 ].selectedRow && 3 == probe.snapshots[ 0 ].selectedRows.size() && 9050 == probe.snapshots[ 0 ].selectedRows[ 2 ] );
        ASSERT_TRUE( refillMicros < 5000000.0 );

        // the same rows in the same places: nothing changed for the selection, and nobody is told
        probe.snapshots.clear();
        raw->Refill( refill, true );
        ASSERT_TRUE( 150 == grid->GetSelectedRow() && 3 == grid->GetSelectedRows().size() && probe.snapshots.empty() );

        // one of the rows that was selected is gone and one more row comes: the others stay, one notification
        probe.snapshots.clear();
        GUCEF::vector< UInt32 > fewer = refill;
        fewer.erase( fewer.begin() + 2050 );
        raw->Refill( fewer, true );
        now = grid->GetSelectedRows();
        ASSERT_TRUE( 2 == now.size() && 150 == now[ 0 ] && 9049 == now[ 1 ] && 150 == grid->GetSelectedRow() );
        ASSERT_TRUE( 1 == probe.snapshots.size() );

        // nested batches are one batch; the cursor row that is gone leaves no cursor
        probe.snapshots.clear();
        GUCEF::vector< UInt32 > withoutCursor = fewer;
        withoutCursor.erase( withoutCursor.begin() + 150 );
        raw->Refill( withoutCursor, true );
        ASSERT_TRUE( -1 == grid->GetSelectedRow() && 1 == grid->GetSelectedRows().size() && 9048 == grid->GetSelectedRows()[ 0 ] );
        ASSERT_TRUE( 1 == probe.snapshots.size() );

        // the consumer can bracket the changes too, with no help of the model
        raw->Refill( refill, true );
        grid->SetSelectedRows( selected );
        ASSERT_TRUE( 100 == grid->GetSelectedRow() );
        probe.snapshots.clear();
        grid->BeginUpdate();
        grid->BeginUpdate();
        ASSERT_TRUE( grid->IsUpdating() );
        std::reverse( refill.begin(), refill.begin() + 50 );
        GUCEF::vector< UInt32 > again;
        for ( UInt32 i=0; i<20; ++i )
            again.push_back( 20000 + i );
        again.insert( again.end(), refill.begin(), refill.end() );
        raw->Refill( again, false );
        grid->EndUpdate();
        ASSERT_TRUE( grid->IsUpdating() && probe.snapshots.empty() );
        grid->EndUpdate();
        ASSERT_TRUE( !grid->IsUpdating() );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        ASSERT_TRUE( 120 == grid->GetSelectedRow() && 3 == grid->GetSelectedRows().size() && 2020 == grid->GetSelectedRows()[ 1 ] && 9020 == grid->GetSelectedRows()[ 2 ] );
        grid->EndUpdate();
        ASSERT_TRUE( !grid->IsUpdating() );      // more ends than begins change nothing
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 6: A batch keeps the cursor and the rows selected whole in the cell mode, and the places of the selection follow the rows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 500 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeFixGrid( context, model );
        grid->SetProperty( "SelectionMode", "Cell" );
        grid->SetCursor( 20, CTestModel::COL_NAME );
        grid->SelectRows( 30, 32 );
        ASSERT_TRUE( grid->GetSelection().ContainsWholeRow( 31 ) && 30 == grid->GetCursorRow() );
        CSelectionProbe probe( grid );
        CEventCount cursorEvents( grid, GUI::CGridView::CellCursorChangedEvent );
        CEventCount selectionEvents( grid, GUI::CGridView::CellSelectionChangedEvent );
        const Int32 cursorRow = grid->GetCursorRow();
        const Int32 cursorColumn = grid->GetCursorColumn();

        GUCEF::vector< UInt32 > ids = raw->GetIds();
        GUCEF::vector< UInt32 > refill;
        for ( UInt32 i=0; i<7; ++i )
            refill.push_back( 1000 + i );
        refill.insert( refill.end(), ids.begin(), ids.end() );
        raw->Refill( refill, true );
        ASSERT_TRUE( 507 == grid->GetRowCount() && cursorRow + 7 == grid->GetCursorRow() && cursorColumn == grid->GetCursorColumn() );
        ASSERT_TRUE( grid->GetSelection().ContainsWholeRow( 37 ) && grid->GetSelection().ContainsWholeRow( 38 ) && grid->GetSelection().ContainsWholeRow( 39 ) );
        ASSERT_TRUE( !grid->GetSelection().ContainsWholeRow( 36 ) && !grid->GetSelection().ContainsWholeRow( 40 ) );
        ASSERT_TRUE( 1 == probe.snapshots.size() && 1 == selectionEvents.count && 1 == cursorEvents.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridFix 7: Whoever is told that the selection changed finds the whole new selection in the first notification, from every setter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 600, 300 );
        grid->SetProperty( "Columns", "A\nB" );
        GUI::CGridView::TRowVector rows;
        for ( UInt32 r=0; r<12; ++r )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( CORE::CString( "k" ) + CORE::UInt32ToString( ( r * 5 ) % 12 ) );
            cells.push_back( CORE::CString( "v" ) + CORE::UInt32ToString( r ) );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
        CSelectionProbe probe( grid );

        // one row selected, then none
        grid->SetSelectedRow( 3 );
        ASSERT_TRUE( 1 == probe.snapshots.size() && 3 == probe.snapshots[ 0 ].selectedRow );
        probe.snapshots.clear();
        grid->SetSelectedRow( -1 );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( -1 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].selectedRows.empty() );

        // several rows: the first notification has the cursor and the list
        grid->SetAllowMultiSelect( true );
        GUI::CGridView::TInt32Vector two;
        two.push_back( 1 ); two.push_back( 2 );
        grid->SetSelectedRows( two );
        probe.snapshots.clear();
        grid->SetSelectedRow( -1 );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( -1 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].selectedRows.empty() );
        grid->SetSelectedRows( two );
        GUI::CGridView::TInt32Vector other;
        other.push_back( 4 ); other.push_back( 5 );
        probe.snapshots.clear();
        grid->SetSelectedRows( other );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( 4 == probe.snapshots[ 0 ].selectedRow && 2 == probe.snapshots[ 0 ].selectedRows.size() && 4 == probe.snapshots[ 0 ].selectedRows[ 0 ] );
        probe.snapshots.clear();
        grid->SetSelectedRows( GUI::CGridView::TInt32Vector() );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( -1 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].selectedRows.empty() );

        // clear
        grid->SetSelectedRows( two );
        probe.snapshots.clear();
        grid->Clear();
        ASSERT_TRUE( !probe.snapshots.empty() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( -1 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].selectedRows.empty() );

        // sorting moves the selected rows: the first notification has the new place of both
        grid->SetRows( rows );
        grid->SetSelectedRows( two );
        probe.snapshots.clear();
        ASSERT_TRUE( grid->SortRows( 0, true ) );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
        {
            const CSelectionProbe::SSnapshot& snapshot = probe.snapshots[ 0 ];
            ASSERT_TRUE( 2 == snapshot.selectedRows.size() && 2 == snapshot.selectedRows[ 0 ] && 7 == snapshot.selectedRows[ 1 ] && 7 == snapshot.selectedRow );
            ASSERT_TRUE( grid->GetText( 1, 2 ) == "v2" && grid->GetText( 1, 7 ) == "v1" );
        }

        // the cell mode: the cursor, the column and the ranges all say the same in the first notification
        grid->SetProperty( "SelectionMode", "Cell" );
        probe.snapshots.clear();
        grid->SetCursor( 6, 1 );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( 6 == probe.snapshots[ 0 ].selectedRow && 1 == probe.snapshots[ 0 ].selectedColumn && probe.snapshots[ 0 ].ranges == "6,1,6,1" );
        probe.snapshots.clear();
        grid->SetSelectedRow( 2 );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( 2 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].ranges == "2,1,2,1" );
        probe.snapshots.clear();
        grid->SetSelectedRow( -1 );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( -1 == probe.snapshots[ 0 ].selectedRow && probe.snapshots[ 0 ].ranges.IsNULLOrEmpty() && -1 == probe.snapshots[ 0 ].selectedColumn );

        // a click and a key: the same
        grid->SetProperty( "SelectionMode", "Row" );
        grid->SetSelectedRows( GUI::CGridView::TInt32Vector() );
        probe.snapshots.clear();
        ASSERT_TRUE( ClickCell( context, grid, 4, 0 ) );
        ASSERT_TRUE( !probe.snapshots.empty() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( 4 == probe.snapshots.back().selectedRow && 1 == probe.snapshots.back().selectedRows.size() && 4 == probe.snapshots.back().selectedRows[ 0 ] );
        for ( size_t i=0; i<probe.snapshots.size(); ++i )
            ASSERT_TRUE( probe.snapshots[ i ].selectedRow == probe.snapshots[ i ].selectedRows[ 0 ] );
        probe.snapshots.clear();
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 1 == probe.snapshots.size() );
        if ( !probe.snapshots.empty() )
            ASSERT_TRUE( 5 == probe.snapshots[ 0 ].selectedRow && 1 == probe.snapshots[ 0 ].selectedRows.size() && 5 == probe.snapshots[ 0 ].selectedRows[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
