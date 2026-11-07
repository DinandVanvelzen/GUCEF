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

#include "gucefCORE_CCsvTableCodec.h"
#include "gucefCORE_CDataTableBatchBuilder.h"
#include "gucefCORE_CDataTableBufferPool.h"
#include "CDynamicBufferAccess.h"
#include "gucefCORE_CFileDataTable.h"
#include "gucefCORE_CMemoryDataTable.h"

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

/** A grid of 800 x 300 (14 rows in view) with the model, in a context of 1280 x 960 */
GUI::CGridView*
MakeModelGrid( GUI::CGUIContext* context, const GUI::TTableViewModelPtr& model )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 800, 300 );
    grid->SetModel( model );
    context->FlushChanges();
    return grid;
}

/** The commands of a grid for the cells in view, and what the model was asked for them */
GUI::CDrawSurface::TCommandVector
PaintModelGrid( GUI::CGUIContext* context, GUI::CGridView* grid )
{
    return PaintedCommands( context, grid );
}

/** A memory table that can tell its observers that rows changed, as a table that follows a replicated source does */
class CNotifyingTable : public CORE::CMemoryDataTable
{
    public:

    CNotifyingTable( const CORE::CDataTableDefinition& definition, CORE::CDataTableBufferPool* pool )
        : CORE::CMemoryDataTable( definition, pool )
    {
    }

    void RaiseRowsChanged( void )
    {
        CORE::TDataTableRowsChangedEventData changed;
        NotifyObservers( CORE::CIDataTable::RowsChangedEvent, &changed );
    }
};

/** A table of 'rows' rows in memory: id int64, name utf8, price float64, flag bool, stamp timestamp (ms), tag utf8 (null for every fifth row) */
CORE::CIDataTablePtr
MakeMemoryTable( CORE::CDataTableBufferPool& pool, UInt32 rows, UInt32 rowGroup )
{
    CORE::CDataTableDefinition definition;
    definition.name = "gridtest";
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "name", CORE::DATATABLE_TYPE_UTF8 ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "price", CORE::DATATABLE_TYPE_FLOAT64 ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "flag", CORE::DATATABLE_TYPE_BOOL ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef::Timestamp( "stamp", CORE::DATATABLE_TIMEUNIT_MILLISECONDS ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "tag", CORE::DATATABLE_TYPE_UTF8 ) );
    definition.storage.rowGroupRowCount = rowGroup;
    definition.storage.pageRowCount = rowGroup;
    CORE::CIDataTablePtr table( GUCEF_NEW CNotifyingTable( definition, &pool ) );
    CORE::CDataTableSchemaPtr schema = table->GetSchema();

    CORE::CDataTableBatchBuilder builder( schema, pool );
    for ( UInt32 i=0; i<rows; ++i )
    {
        CORE::CVariant cells[ 6 ];
        cells[ 0 ] = (Int64) i;
        cells[ 1 ] = CORE::ToUtf8String( CORE::CString( "row " ) + CORE::UInt32ToString( i ) );
        cells[ 2 ] = (Float64) ( ( i * 37 ) % 1000 ) / 4.0;
        cells[ 3 ] = 0 == i % 2;
        cells[ 4 ] = CORE::CDataTableValues::Timestamp( (Int64) 1700000000000LL + (Int64) i * 1000, CORE::DATATABLE_TIMEUNIT_MILLISECONDS );
        if ( 0 != i % 5 )
            cells[ 5 ] = CORE::ToUtf8String( CORE::CString( "tag" ) + CORE::UInt32ToString( i % 10 ) );
        builder.AppendRow( cells, 6 );
    }
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    CORE::CDataTableWriteResult result;
    if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) || CORE::DATATABLE_STATUS_OK != table->Write( *batch, CORE::DATATABLE_WRITE_INSERT, result ) )
        return CORE::CIDataTablePtr();
    return table;
}

/** The text of the table cell at a view row */
CORE::CString
TextOf( const GUI::CITableViewModel& model, UInt32 row, UInt32 column )
{
    return model.GetCellText( row, column );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridModelTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridModel" );

    GUCEF_TESTFW_TESTCASE( "GridModel: A grid with a model asks it for the cells in view only, of 100,000 rows as of ten, and scrolling moves the window" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 100000 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeModelGrid( context, model );
        ASSERT_TRUE( grid->HasModel() && 100000 == grid->GetRowCount() && 8 == grid->GetColumnCount() );
        ASSERT_TRUE( 14 == grid->GetVisibleRowCount() );

        // the first window: at most the visible rows (and one partly visible) of the 8 columns
        raw->ResetCounts();
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( raw->valueCalls > 0 && raw->valueCalls <= 15 * 8 );
        ASSERT_TRUE( !raw->rowsFetched.empty() && *raw->rowsFetched.begin() == 0 && *raw->rowsFetched.rbegin() <= 14 );
        ASSERT_TRUE( raw->prepareCalls >= 1 && 0 == raw->lastPrepared.firstRow && raw->lastPrepared.rowCount >= 14 );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Id" ) && GUCEF_NULL != FindText( commands, "Price" ) );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "name 3" ) && GUCEF_NULL == FindText( commands, "name 40" ) );

        // the text asked for is never the text of the whole table
        ASSERT_TRUE( 0 == raw->textCalls );

        // scrolled to the middle: a window there, nothing from the top
        grid->SetIntProperty( "ScrollPosition", 50000 );
        raw->ResetCounts();
        commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( raw->valueCalls <= 15 * 8 );
        ASSERT_TRUE( !raw->rowsFetched.empty() && *raw->rowsFetched.begin() >= 50000 && *raw->rowsFetched.rbegin() <= 50014 );
        ASSERT_TRUE( 50000 == raw->lastPrepared.firstRow );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "name 50003" ) );

        // the wheel scrolls through the model as through a list, and stops at the end
        context->FlushChanges();
        const GUI::CRect& rect = grid->GetRect();
        context->InjectMouseMove( rect.left + 40, rect.top + 60 );
        context->InjectMouseWheel( -3 );
        ASSERT_TRUE( 50003 == grid->GetIntProperty( "ScrollPosition" ) );
        grid->SetIntProperty( "ScrollPosition", 99990 );
        context->InjectMouseWheel( -50 );
        ASSERT_TRUE( grid->GetMaxScrollRow() == grid->GetIntProperty( "ScrollPosition" ) && 99986 == grid->GetMaxScrollRow() );
        raw->ResetCounts();
        commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( *raw->rowsFetched.rbegin() == 99999 && GUCEF_NULL != FindText( commands, "name 99999" ) );

        // a click selects the row under the pointer, the model is asked for its key
        ClickAt( context, rect.left + 20, rect.top + grid->GetRowTop( 99990 ) + 5 );
        ASSERT_TRUE( 99990 == grid->GetSelectedRow() );

        // a frame costs the same for any number of rows
        grid->SetIntProperty( "ScrollPosition", 0 );
        UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<20; ++i )
            PaintModelGrid( context, grid );
        const double hugeFrame = Micros( MT::PrecisionTickCount() - start ) / 20.0;
        CTestModel* smallRaw = GUCEF_NEW CTestModel( 10 );
        GUI::TTableViewModelPtr small( smallRaw );
        grid->SetModel( small );
        start = MT::PrecisionTickCount();
        for ( UInt32 i=0; i<20; ++i )
            PaintModelGrid( context, grid );
        const double smallFrame = Micros( MT::PrecisionTickCount() - start ) / 20.0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: model frame of 100000 rows " << (UInt64) hugeFrame << " us, of 10 rows " << (UInt64) smallFrame << " us" );
        ASSERT_TRUE( hugeFrame < smallFrame * 3.0 + 2000.0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: Numbers, dates, booleans and nulls are shown by the definition of their column" )
    try
    {
        // the formatter
        GUI::CTableViewColumn integer( "n", CORE::DATATABLE_TYPE_INT64 );
        integer.thousandsSeparator = true;
        GUI::SCellContent content;
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Int64) 1234567 ), integer, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_TEXT == content.kind && "1,234,567" == content.text && content.isNumber );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Int64) -1234 ), integer, content );
        ASSERT_TRUE( "-1,234" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (UInt64) 18446744073709551615ULL ), integer, content );
        ASSERT_TRUE( "18,446,744,073,709,551,615" == content.text );

        GUI::CTableViewColumn price( "p", CORE::DATATABLE_TYPE_FLOAT64 );
        price.decimals = 2;
        price.prefix = "$";
        price.suffix = " USD";
        price.thousandsSeparator = true;
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 1234.5 ), price, content );
        ASSERT_TRUE( "$1,234.50 USD" == content.text && content.isNumber );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 0.125 ), price, content );        // ties go away from zero
        ASSERT_TRUE( "$0.13 USD" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) -0.125 ), price, content );
        ASSERT_TRUE( "$-0.13 USD" == content.text );
        price.decimals = 0;
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 2.5 ), price, content );
        ASSERT_TRUE( "$3 USD" == content.text );
        price.decimals = -1;
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 1234.5 ), price, content );        // the digits the value has
        ASSERT_TRUE( "$1,234.5 USD" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 3.0 ), price, content );
        ASSERT_TRUE( "$3 USD" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CVariant( (Float64) 0.1 ), price, content );
        ASSERT_TRUE( "$0.1 USD" == content.text );

        GUI::CTableViewColumn amount( "a", CORE::DATATABLE_TYPE_DECIMAL );
        CORE::CDecimal decimal;
        CORE::CDecimal::FromUnscaled( 1234525, 2, decimal );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Decimal( decimal ), amount, content );
        ASSERT_TRUE( "12345.25" == content.text );
        amount.decimals = 1;
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Decimal( decimal ), amount, content );
        ASSERT_TRUE( "12345.3" == content.text );
        amount.decimals = 4;
        amount.thousandsSeparator = true;
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Decimal( decimal ), amount, content );
        ASSERT_TRUE( "12,345.2500" == content.text );

        // booleans: a check mark or text
        GUI::CTableViewColumn flag( "f", CORE::DATATABLE_TYPE_BOOL );
        GUI::CTableCellFormatter::Format( CORE::CVariant( true ), flag, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_CHECK_ON == content.kind && "true" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CVariant( false ), flag, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_CHECK_OFF == content.kind );
        flag.booleanDisplay = GUI::CTableViewColumn::BOOLEAN_TEXT;
        GUI::CTableCellFormatter::Format( CORE::CVariant( true ), flag, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_TEXT == content.kind && "true" == content.text );

        // null is a cell of its own, with the text the column says
        CORE::CVariant empty;
        empty.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL, false );
        GUI::CTableCellFormatter::Format( empty, integer, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_NULL == content.kind && content.text.IsNULLOrEmpty() );
        integer.nullText = "n/a";
        GUI::CTableCellFormatter::Format( empty, integer, content );
        ASSERT_TRUE( GUI::SCellContent::KIND_NULL == content.kind && "n/a" == content.text );

        // dates and timestamps are UTC, before 1970 too
        GUI::CTableViewColumn date( "d", CORE::DATATABLE_TYPE_DATE );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Date( 0 ), date, content );
        ASSERT_TRUE( "1970-01-01" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Date( 18000 ), date, content );
        ASSERT_TRUE( "2019-04-14" == content.text );
        date.dateFormat = "dd/MM/yyyy";
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Date( 18000 ), date, content );
        ASSERT_TRUE( "14/04/2019" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Date( -1 ), GUI::CTableViewColumn( "d", CORE::DATATABLE_TYPE_DATE ), content );
        ASSERT_TRUE( "1969-12-31" == content.text );

        GUI::CTableViewColumn stamp( "t", CORE::DATATABLE_TYPE_TIMESTAMP );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Timestamp( 1700000000123LL, CORE::DATATABLE_TIMEUNIT_MILLISECONDS ), stamp, content );
        ASSERT_TRUE( "2023-11-14 22:13:20" == content.text );
        stamp.dateFormat = "yyyy-MM-dd HH:mm:ss.fff";
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Timestamp( 1700000000123LL, CORE::DATATABLE_TIMEUNIT_MILLISECONDS ), stamp, content );
        ASSERT_TRUE( "2023-11-14 22:13:20.123" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Timestamp( 1700000000123456LL, CORE::DATATABLE_TIMEUNIT_MICROSECONDS ), stamp, content );
        ASSERT_TRUE( "2023-11-14 22:13:20.123" == content.text );
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Timestamp( -1000, CORE::DATATABLE_TIMEUNIT_MILLISECONDS ), stamp, content );
        ASSERT_TRUE( "1969-12-31 23:59:59.000" == content.text );
        stamp.dateFormat = "HH:mm";
        GUI::CTableCellFormatter::Format( CORE::CDataTableValues::Timestamp( 1700000000123LL, CORE::DATATABLE_TIMEUNIT_MILLISECONDS ), stamp, content );
        ASSERT_TRUE( "22:13" == content.text );

        ASSERT_TRUE( "1,234,567.891" == GUI::CTableCellFormatter::GroupDigits( "1234567.891" ) && "123" == GUI::CTableCellFormatter::GroupDigits( "123" ) && "-1,000" == GUI::CTableCellFormatter::GroupDigits( "-1000" ) );

        // the alignment follows the type
        ASSERT_TRUE( "Right" == integer.GetEffectiveAlignment() && "Right" == date.GetEffectiveAlignment() && "Right" == price.GetEffectiveAlignment() );
        ASSERT_TRUE( "Center" == GUI::CTableViewColumn( "f", CORE::DATATABLE_TYPE_BOOL ).GetEffectiveAlignment() && "Left" == GUI::CTableViewColumn( "s", CORE::DATATABLE_TYPE_UTF8 ).GetEffectiveAlignment() );
        GUI::CTableViewColumn forced( "x", CORE::DATATABLE_TYPE_INT32 );
        forced.alignment = "Left";
        ASSERT_TRUE( "Left" == forced.GetEffectiveAlignment() );

        // in the grid
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 100 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeModelGrid( context, model );
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        // id 4: a price of 1.00 with its sign, an amount of 4.25, a number right aligned in its column
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "$1.00" ) && GUCEF_NULL != FindText( commands, "4.25" ) && GUCEF_NULL != FindText( commands, "2019-04-18" ) );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "2023-11-14 22:17:20" ) );
        const GUI::CDrawSurface::TCommand* priceCell = FindText( commands, "$1.00" );
        const GUI::CGridColumnLayout& layout = grid->GetColumnLayout();
        ASSERT_TRUE( GUCEF_NULL != priceCell && priceCell->x0 + TextWidth( context, "$1.00" ) == layout.GetScreenLeft( 1 ) + layout.GetWidth( 1 ) - GUI::CGridView::CELL_PADDING_RIGHT );
        // the null of id 3 is a tinted cell without text, the check marks are lines, not text
        ASSERT_TRUE( HasFill( commands, GUI::CRect( layout.GetScreenLeft( 6 ), grid->GetRowTop( 3 ), layout.GetWidth( 6 ), 20 ) ) );
        UInt32 lines = 0;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_LINE == commands[ i ].type && commands[ i ].y0 > 20 && commands[ i ].x0 >= layout.GetScreenLeft( 3 ) && commands[ i ].x0 < layout.GetScreenLeft( 4 ) )
                ++lines;
        ASSERT_TRUE( lines >= 4 * 7 && lines <= 4 * 8 + 2 );          // the 7 or 8 odd ids in view, four lines of a tick each
        ASSERT_TRUE( GUCEF_NULL == FindText( commands, "true" ) );

        // the boolean as text, when the column says so
        raw->ResetCounts();
        GUI::CTableViewColumn textFlag = CTestModel::MakeColumn( CTestModel::COL_FLAG );
        textFlag.booleanDisplay = GUI::CTableViewColumn::BOOLEAN_TEXT;
        ASSERT_TRUE( "true" == GUI::CTableCellFormatter::FormatText( CORE::CVariant( true ), textFlag ) );

        // the text of a cell of the model is the formatted value
        ASSERT_TRUE( "name 7" == TextOf( *model, 7, CTestModel::COL_NAME ) );
        ASSERT_TRUE( "$1.75" == TextOf( *model, 7, CTestModel::COL_PRICE ) );
        ASSERT_TRUE( TextOf( *model, 9, CTestModel::COL_MAYBE ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: Changes of the model are followed by range: cells redraw, inserted and removed rows keep the selection on its row, a new order is followed by key" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 50 );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeModelGrid( context, model );
        grid->SetAllowMultiSelect( true );
        grid->SetSelectedRow( 10 );
        grid->SetIntProperty( "ScrollPosition", 4 );
        CEventCount selectionChanges( grid, GUI::CGridView::SelectionChangedEvent );

        // cells: only what changed is drawn again; the selection and the scroll position stay
        raw->ChangeName( 12, "renamed" );
        ASSERT_TRUE( 10 == grid->GetSelectedRow() && 4 == grid->GetIntProperty( "ScrollPosition" ) );
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "renamed" ) && GUCEF_NULL == FindText( commands, "name 12" ) );
        ASSERT_TRUE( "renamed" == grid->GetText( CTestModel::COL_NAME, 12 ) );

        // rows inserted above: the selected row is the same row, now further down, and the same rows stay in view
        raw->InsertRows( 0, 5 );
        ASSERT_TRUE( 55 == grid->GetRowCount() && 15 == grid->GetSelectedRow() && "name 10" == grid->GetText( CTestModel::COL_NAME, 15 ) );
        ASSERT_TRUE( 9 == grid->GetIntProperty( "ScrollPosition" ) );
        // rows inserted below it do not move it
        raw->InsertRows( 40, 2 );
        ASSERT_TRUE( 57 == grid->GetRowCount() && 15 == grid->GetSelectedRow() );

        // rows removed above, and the selected row itself
        raw->RemoveRows( 0, 3 );
        ASSERT_TRUE( 12 == grid->GetSelectedRow() && "name 10" == grid->GetText( CTestModel::COL_NAME, 12 ) );
        raw->RemoveRows( 12, 1 );
        ASSERT_TRUE( -1 == grid->GetSelectedRow() && 53 == grid->GetRowCount() );
        ASSERT_TRUE( selectionChanges.count >= 3 );

        // a column title and the number of columns
        raw->SetColumnTitle( 2, "Label" );
        commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "Label" ) && GUCEF_NULL == FindText( commands, "Name" ) );
        CTestModel* narrowRaw = GUCEF_NEW CTestModel( 20, 3 );
        GUI::TTableViewModelPtr narrow( narrowRaw );
        grid->SetModel( narrow );
        ASSERT_TRUE( 3 == grid->GetColumnCount() );
        narrowRaw->AddColumn();
        ASSERT_TRUE( 4 == grid->GetColumnCount() );
        grid->SetModel( model );

        // a new order: the selected rows, the cursor and the anchor follow their rows by key
        raw->SetSortable( true );
        grid->SetSelectedRows( GUI::CGridView::TInt32Vector() );
        GUI::CGridView::TInt32Vector rows;
        rows.push_back( 3 ); rows.push_back( 8 ); rows.push_back( 20 );
        grid->SetSelectedRows( rows );
        grid->SetSelectedRow( 8 );
        grid->SetSelectedRows( rows );
        const GUI::CGridView::TInt32Vector selectedBefore = grid->GetSelectedRows();
        GUCEF::vector< UInt32 > idsBefore;
        for ( size_t i=0; i<selectedBefore.size(); ++i )
            idsBefore.push_back( raw->IdOf( (UInt32) selectedBefore[ i ] ) );
        raw->Reverse();
        const GUI::CGridView::TInt32Vector after = grid->GetSelectedRows();
        ASSERT_TRUE( 3 == after.size() );
        bool followed = after.size() == idsBefore.size();
        for ( size_t i=0; followed && i<after.size(); ++i )
        {
            bool found = false;
            for ( size_t j=0; j<idsBefore.size(); ++j )
                found = found || raw->IdOf( (UInt32) after[ i ] ) == idsBefore[ j ];
            followed = followed && found;
        }
        ASSERT_TRUE( followed );
        GUI::CGridView::TInt32Vector expected;
        expected.push_back( (Int32) raw->GetRowCount() - 1 - 20 );
        expected.push_back( (Int32) raw->GetRowCount() - 1 - 8 );
        expected.push_back( (Int32) raw->GetRowCount() - 1 - 3 );
        ASSERT_TRUE( expected == after );
        ASSERT_TRUE( (Int32) raw->GetRowCount() - 1 - 3 == grid->GetSelectedRow() );

        // the sort of the header: by the model when it can, and the arrow follows
        grid->SetProperty( "Sortable", "true" );
        grid->SetSelectedRows( GUI::CGridView::TInt32Vector() );
        grid->SetSelectedRow( 7 );
        const UInt32 selectedId = raw->IdOf( 7 );
        Int32 x = 0, y = 0;
        ASSERT_TRUE( TitlePoint( context, grid, CTestModel::COL_ID, x, y ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( 0 == grid->GetIntProperty( "SortColumn" ) && grid->GetBoolProperty( "SortAscending" ) );
        ASSERT_TRUE( 0 == raw->IdOf( 0 ) && selectedId == raw->IdOf( (UInt32) grid->GetSelectedRow() ) );
        ClickAt( context, x, y );
        ASSERT_TRUE( !grid->GetBoolProperty( "SortAscending" ) && selectedId == raw->IdOf( (UInt32) grid->GetSelectedRow() ) );
        // a model that does not sort says so: nothing changes
        raw->SetSortable( false );
        grid->SetProperty( "SortColumn", "-1" );
        const UInt32 firstId = raw->IdOf( 0 );
        ClickAt( context, x, y );
        ASSERT_TRUE( -1 == grid->GetIntProperty( "SortColumn" ) && firstId == raw->IdOf( 0 ) );
        ASSERT_FALSE( grid->SortRows( 1, true ) );

        // without a model the string list is back
        grid->SetModel( GUI::TTableViewModelPtr() );
        ASSERT_TRUE( !grid->HasModel() && 0 == grid->GetRowCount() );
        grid->SetText( 0, 0, "text" );
        ASSERT_TRUE( 1 == grid->GetRowCount() && "text" == grid->GetText( 0, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: A model has rows with titles and a footer, and the string list setters leave a grid with a model alone" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CTestModel* raw = GUCEF_NEW CTestModel( 30 );
        raw->titles = true;
        raw->footer = true;
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeModelGrid( context, model );
        grid->SetBoolProperty( "ShowRowHeader", true );
        grid->SetBoolProperty( "ShowFooter", true );
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "R3" ) && GUCEF_NULL != FindText( commands, "Total" ) && GUCEF_NULL != FindText( commands, "30" ) );
        ASSERT_TRUE( "R4" == grid->GetRowHeaderText( 4 ) && grid->GetFooterTop() == 300 - 20 );
        // the footer takes a row from the rows: 13 rows fit
        ASSERT_TRUE( 13 == grid->GetVisibleRowCount() );
        raw->titles = false;
        ASSERT_TRUE( "5" == grid->GetRowHeaderText( 4 ) );

        grid->SetText( 0, 0, "ignored" );
        grid->AppendText( "ignored" );
        GUI::CGridView::TRowVector rows;
        grid->SetRows( rows );
        ASSERT_TRUE( 30 == grid->GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: A data table is shown with no glue code: typed columns, a bounded cache of batches, a filter given to the table, a sort when the table is small enough" )
    try
    {
        CORE::CDataTableBufferPool pool;
        CORE::CIDataTablePtr table = MakeMemoryTable( pool, 10000, 1024 );
        ASSERT_TRUE( !table.IsNULL() );
        if ( table.IsNULL() )
            return;
        GUI::CDataTableViewModel* raw = GUCEF_NEW GUI::CDataTableViewModel( table );
        GUI::TTableViewModelPtr model( raw );
        ASSERT_TRUE( raw->IsValid() && 6 == raw->GetColumnCount() && 10000 == raw->GetRowCount() );
        ASSERT_TRUE( "id" == raw->GetColumn( 0 ).name && CORE::DATATABLE_TYPE_INT64 == raw->GetColumn( 0 ).type && CORE::DATATABLE_TYPE_TIMESTAMP == raw->GetColumn( 4 ).type );
        raw->SetMaxCachedBatches( 3 );

        // typed values, the cache never holds more than it may
        CORE::CVariant value;
        bool right = true;
        UInt32 largestCache = 0;
        for ( UInt32 row=0; row<10000; row += 7 )
        {
            right = right && raw->GetCellValue( row, 0, value ) && (Int64) row == value.AsInt64();
            right = right && raw->GetCellValue( row, 1, value ) && ( CORE::CString( "row " ) + CORE::UInt32ToString( row ) ) == value.AsString();
            if ( 0 == row % 5 )
                right = right && raw->GetCellValue( row, 5, value ) && value.IsNULL();
            if ( raw->GetCachedBatchCount() > largestCache )
                largestCache = raw->GetCachedBatchCount();
        }
        ASSERT_TRUE( right );
        ASSERT_TRUE( largestCache <= 3 && largestCache >= 1 );
        const UInt32 scansForward = raw->GetScansStarted();
        // reading forward is one pass: the count pass, and the scan the first read after the cache moved on
        ASSERT_TRUE( scansForward <= 3 );

        // a row that left the cache: the scan goes back and starts again, the value is right
        ASSERT_TRUE( raw->GetCellValue( 3, 0, value ) && 3 == value.AsInt64() );
        ASSERT_TRUE( raw->GetScansStarted() >= scansForward && raw->GetCachedBatchCount() <= 3 );

        // in the grid
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CGridView* grid = MakeModelGrid( context, model );
        grid->SetIntProperty( "ScrollPosition", 5000 );
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "id" ) && GUCEF_NULL != FindText( commands, "row 5003" ) && GUCEF_NULL != FindText( commands, "5003" ) );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "2023-11-14 23:36:43" ) );          // 1700000000 + 5003 s

        // the filter goes to the table: only the rows that match are seen
        CORE::CDataTableQuery::TPredicateVector filter;
        filter.push_back( CORE::CDataTablePredicate( "id", CORE::CVariant( (Int64) 100 ), true, CORE::CVariant( (Int64) 199 ), true ) );
        CEventCount resets( raw, GUI::CITableViewModel::ModelResetEvent );
        ASSERT_TRUE( raw->SetFilter( filter ) && 1 == resets.count );
        ASSERT_TRUE( 100 == raw->GetRowCount() && grid->GetRowCount() == 100 );
        ASSERT_TRUE( raw->GetCellValue( 0, 0, value ) && 100 == value.AsInt64() && raw->GetCellValue( 99, 0, value ) && 199 == value.AsInt64() );
        // a filter that does not fit the table is refused and changes nothing
        CORE::CDataTableQuery::TPredicateVector bad;
        bad.push_back( CORE::CDataTablePredicate( "nothere", CORE::DATATABLE_PREDICATE_EQUALS, CORE::CVariant( (Int64) 1 ) ) );
        ASSERT_FALSE( raw->SetFilter( bad ) );
        ASSERT_TRUE( 100 == raw->GetRowCount() );
        // a filter on a text column, and a filter that matches nothing
        CORE::CDataTableQuery::TPredicateVector textFilter;
        textFilter.push_back( CORE::CDataTablePredicate( "tag", CORE::DATATABLE_PREDICATE_EQUALS, CORE::CVariant( CORE::ToUtf8String( CORE::CString( "tag3" ) ) ) ) );
        ASSERT_TRUE( raw->SetFilter( textFilter ) && 1000 == raw->GetRowCount() );
        CORE::CDataTableQuery::TPredicateVector none;
        none.push_back( CORE::CDataTablePredicate( "id", CORE::DATATABLE_PREDICATE_LESS, CORE::CVariant( (Int64) 0 ) ) );
        ASSERT_TRUE( raw->SetFilter( none ) && 0 == raw->GetRowCount() && 0 == grid->GetRowCount() );
        raw->ClearFilter();
        ASSERT_TRUE( 10000 == raw->GetRowCount() );

        // the sort: the model orders by a column of a table of ordinary size; the keys keep following their rows
        ASSERT_TRUE( raw->IsSortSupported() );
        grid->SetSelectedRow( 6 );
        CORE::CVariant key;
        ASSERT_TRUE( raw->GetRowKey( 6, key ) && 6 == key.AsUInt64() );
        ASSERT_TRUE( raw->SortByColumn( 2, true ) );
        CORE::CVariant::VariantVector prices;
        ASSERT_TRUE( raw->GetColumnSpan( 2, 0, 10000, prices ) && 10000 == prices.size() );
        bool ascending = true;
        for ( size_t i=1; i<prices.size(); ++i )
            ascending = ascending && prices[ i - 1 ].AsFloat64() <= prices[ i ].AsFloat64();
        ASSERT_TRUE( ascending );
        ASSERT_TRUE( raw->GetCellValue( (UInt32) grid->GetSelectedRow(), 0, value ) && 6 == value.AsInt64() );
        UInt32 foundRow = 0;
        ASSERT_TRUE( raw->FindRowByKey( key, foundRow ) && (Int32) foundRow == grid->GetSelectedRow() );
        ASSERT_FALSE( raw->FindRowByKey( CORE::CVariant( (UInt64) 10000 ), foundRow ) );
        // nulls come last, whichever way
        ASSERT_TRUE( raw->SortByColumn( 5, false ) );
        ASSERT_TRUE( raw->GetCellValue( 9999, 5, value ) && value.IsNULL() && raw->GetCellValue( 0, 5, value ) && !value.IsNULL() );
        raw->ClearSort();
        ASSERT_TRUE( raw->GetCellValue( 0, 0, value ) && 0 == value.AsInt64() );

        // too large to sort: reported, nothing silently different
        raw->SetMaxSortRows( 100 );
        ASSERT_FALSE( raw->IsSortSupported() );
        ASSERT_FALSE( raw->SortByColumn( 2, true ) );
        ASSERT_TRUE( raw->GetCellValue( 1, 0, value ) && 1 == value.AsInt64() );
        ASSERT_FALSE( grid->SortRows( 2, true ) );

        // the columns that are shown
        CORE::CString::StringVector names;
        names.push_back( "price" );
        names.push_back( "id" );
        ASSERT_TRUE( raw->SetColumns( names ) && 2 == raw->GetColumnCount() && "price" == raw->GetColumn( 0 ).name );
        ASSERT_TRUE( raw->GetCellValue( 4, 1, value ) && 4 == value.AsInt64() );
        names.push_back( "nothere" );
        ASSERT_FALSE( raw->SetColumns( names ) );
        ASSERT_TRUE( 2 == raw->GetColumnCount() );

        // the table changes: the model is told with Invalidate, and shows the new rows
        raw->SetMaxSortRows( 1000000 );
        CORE::CDataTableBatchBuilder builder( table->GetSchema(), pool );
        for ( UInt32 i=10000; i<10010; ++i )
        {
            CORE::CVariant cells[ 6 ];
            cells[ 0 ] = (Int64) i;
            cells[ 1 ] = CORE::ToUtf8String( CORE::CString( "new" ) );
            builder.AppendRow( cells, 6 );
        }
        CORE::CDataTableBatchPtr more;
        CORE::CString error;
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == builder.Build( more, error ) && CORE::DATATABLE_STATUS_OK == table->Write( *more, CORE::DATATABLE_WRITE_INSERT, result ) );
        const UInt32 resetsBefore = resets.count;
        raw->Invalidate();
        ASSERT_TRUE( resets.count == resetsBefore + 1 && 10010 == raw->GetRowCount() && 10010 == grid->GetRowCount() );

        // a table that follows a replicated source tells the model by event
        static_cast< CNotifyingTable* >( table.GetPointerAlways() )->RaiseRowsChanged();
        ASSERT_TRUE( resets.count == resetsBefore + 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: A file table (a CSV file with its schema read from the file) is shown as it is, and a model without a table is not valid" )
    try
    {
        CORE::CString text = "id,name,price,flag\n";
        for ( UInt32 i=0; i<500; ++i )
            text += CORE::UInt32ToString( i ) + ",item" + CORE::UInt32ToString( i ) + "," + CORE::UInt32ToString( i ) + ".50," + ( 0 == i % 2 ? "true" : "false" ) + "\n";
        CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( GUCEF_NEW CORE::CDynamicBuffer(), true );
        access->Write( text.C_String(), 1, text.Length() );
        access->Setpos( 0 );
        CORE::IOAccessPtr input( access->CreateSharedPtr() );
        CORE::CTSharedPtr< CORE::CITableCodec, MT::CMutex > codec( GUCEF_NEW CORE::CCsvTableCodec() );
        CORE::CDataTableBufferPool pool;
        CORE::CIDataTablePtr table( GUCEF_NEW CORE::CFileDataTable( input, codec, CORE::CValueList(), "csvgrid", &pool ) );

        GUI::CDataTableViewModel* raw = GUCEF_NEW GUI::CDataTableViewModel( table );
        GUI::TTableViewModelPtr model( raw );
        ASSERT_TRUE( raw->IsValid() && 4 == raw->GetColumnCount() && 500 == raw->GetRowCount() );
        ASSERT_TRUE( "id" == raw->GetColumn( 0 ).name && "name" == raw->GetColumn( 1 ).name && "price" == raw->GetColumn( 2 ).name );
        ASSERT_TRUE( "item7" == TextOf( *model, 7, 1 ) && "7" == TextOf( *model, 7, 0 ) && "7.50" == TextOf( *model, 7, 2 ) );
        CORE::CVariant value;
        ASSERT_TRUE( raw->GetCellValue( 7, 3, value ) && !value.AsBool( true ) );
        ASSERT_TRUE( raw->GetCellValue( 8, 3, value ) && value.AsBool( false ) );

        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CGridView* grid = MakeModelGrid( context, model );
        GUI::CDrawSurface::TCommandVector commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindText( commands, "item3" ) && GUCEF_NULL != FindText( commands, "3.50" ) && GUCEF_NULL != FindText( commands, "name" ) );

        // no table: not valid, and an empty grid
        GUI::CDataTableViewModel* empty = GUCEF_NEW GUI::CDataTableViewModel( CORE::CIDataTablePtr() );
        GUI::TTableViewModelPtr emptyModel( empty );
        ASSERT_TRUE( !empty->IsValid() && 0 == empty->GetRowCount() && 0 == empty->GetColumnCount() );
        grid->SetModel( emptyModel );
        ASSERT_TRUE( 0 == grid->GetRowCount() );
        commands = PaintModelGrid( context, grid );
        ASSERT_TRUE( !commands.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridModel: The drivers that draw with the own painter say what the grid can do" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 640, 480 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CGUIDriver* driver = context->GetDriver();
        ASSERT_TRUE( GUCEF_NULL != driver );
        if ( GUCEF_NULL == driver )
            return;
        ASSERT_TRUE( driver->HasCapability( "GridModel" ) && driver->HasCapability( "GridTypedCells" ) && driver->HasCapability( "GridCellSelection" ) && driver->HasCapability( "GridLayout" ) );
        GUCEF::set< CORE::CString > capabilities;
        GUI::CGridView::GetCapabilities( capabilities );
        ASSERT_TRUE( 8 == capabilities.size() );
        for ( GUCEF::set< CORE::CString >::const_iterator i = capabilities.begin(); i != capabilities.end(); ++i )
            ASSERT_TRUE( driver->HasCapability( *i ) );
        ASSERT_TRUE( driver->HasCapability( "GridModelContent" ) && driver->HasCapability( "GridBatchUpdate" ) && driver->HasCapability( "GridCellEditing" ) && driver->HasCapability( "GridClipboard" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
