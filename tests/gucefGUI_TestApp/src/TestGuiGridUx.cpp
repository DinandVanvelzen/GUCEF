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
 *  The fit policy of grid columns, the overflow rules of numbers and text in cells and in the footer, cell statuses
 *  and markers, the emphasis bar and the selection outline, the compact form of numbers, the context menu of a grid
 *  and rows that fold open. Every width is computed by hand with the bitmap test font of 8 x 10 pixels.
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

typedef GUI::CGridColumnFitter TFitter;

/** A column of the fitter */
TFitter::TColumn
FitColumn( Int32 nominal, Int32 content, bool numeric, const GUI::CGridColumnFit& fit = GUI::CGridColumnFit() )
{
    TFitter::TColumn column;
    column.nominal = nominal;
    column.content = content;
    column.numeric = numeric;
    column.fit = fit;
    return column;
}

/**
 *  A model of three columns (Name, Value with "$", grouped digits and 2 decimals, Note), a footer, a status and a marker
 *  per row on one column, and an optional hierarchy whose parents get two child rows when they are opened
 */
class CUxModel : public GUI::CITableViewModel
{
    public:

    struct SRow
    {
        CORE::CString name;
        Float64 value;
        CORE::CString note;
        Int32 statusColumn;
        GUI::SCellContent::EStatus status;
        CORE::CString marker;
        bool markerLeading;
        UInt32 level;
        bool expandable;
        bool expanded;
    };

    GUCEF::vector< SRow > rows;
    GUCEF::vector< GUI::CTableViewColumn > columns;
    bool footer;
    Float64 footerValue;
    CORE::CString footerName;
    bool hierarchy;
    UInt32 expandCalls;

    CUxModel( void )
        : GUI::CITableViewModel(), rows(), columns(), footer( false ), footerValue( 0.0 ), footerName(), hierarchy( false ), expandCalls( 0 )
    {
        columns.push_back( GUI::CTableViewColumn( "Name", CORE::DATATABLE_TYPE_UTF8 ) );
        GUI::CTableViewColumn value( "Value", CORE::DATATABLE_TYPE_FLOAT64 );
        value.decimals = 2;
        value.thousandsSeparator = true;
        value.prefix = "$";
        columns.push_back( value );
        columns.push_back( GUI::CTableViewColumn( "Note", CORE::DATATABLE_TYPE_UTF8 ) );
    }

    static SRow MakeRow( const CORE::CString& name, Float64 value, const CORE::CString& note )
    {
        SRow row;
        row.name = name;
        row.value = value;
        row.note = note;
        row.statusColumn = -1;
        row.status = GUI::SCellContent::STATUS_NORMAL;
        row.markerLeading = false;
        row.level = 0;
        row.expandable = false;
        row.expanded = false;
        return row;
    }

    /** Says that the value of a cell changed */
    void ChangedCell( UInt32 row, UInt32 column )
    {
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( row, 1, column, 1 ) );
        NotifyObservers( CellsChangedEvent, &range );
    }

    /** Says that anything may have changed */
    void Reset( void )
    {
        NotifyObservers( ModelResetEvent );
    }

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) rows.size(); }

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return (UInt32) columns.size(); }

    virtual const GUI::CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUI::CTableViewColumn none;
        return column < (UInt32) columns.size() ? columns[ column ] : none;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= (UInt32) rows.size() || column >= (UInt32) columns.size() )
            return false;
        if ( 0 == column )
            value = CORE::CVariant( rows[ row ].name );
        else
        if ( 1 == column )
            value = CORE::CVariant( rows[ row ].value );
        else
            value = CORE::CVariant( rows[ row ].note );
        return true;
    }

    virtual bool GetCellContent( UInt32 row, UInt32 column, GUI::SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !GUI::CITableViewModel::GetCellContent( row, column, content ) )
            return false;
        if ( (Int32) column == rows[ row ].statusColumn )
        {
            content.status = rows[ row ].status;
            content.marker = rows[ row ].marker;
            content.markerLeading = rows[ row ].markerLeading;
        }
        return true;
    }

    virtual bool HasFooter( void ) const GUCEF_VIRTUAL_OVERRIDE { return footer; }

    virtual bool GetFooterValue( UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 == column && !footerName.IsNULLOrEmpty() )
        {
            value = CORE::CVariant( footerName );
            return true;
        }
        if ( 1 == column )
        {
            value = CORE::CVariant( footerValue );
            return true;
        }
        return false;
    }

    virtual bool HasRowHierarchy( void ) const GUCEF_VIRTUAL_OVERRIDE { return hierarchy; }

    virtual UInt32 GetRowLevel( UInt32 row ) const GUCEF_VIRTUAL_OVERRIDE { return row < (UInt32) rows.size() ? rows[ row ].level : 0; }

    virtual bool IsRowExpandable( UInt32 row ) const GUCEF_VIRTUAL_OVERRIDE { return row < (UInt32) rows.size() && rows[ row ].expandable; }

    virtual bool IsRowExpanded( UInt32 row ) const GUCEF_VIRTUAL_OVERRIDE { return row < (UInt32) rows.size() && rows[ row ].expanded; }

    virtual bool SetRowExpanded( UInt32 row, bool expanded ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( row >= (UInt32) rows.size() || !rows[ row ].expandable || rows[ row ].expanded == expanded )
            return false;
        ++expandCalls;
        rows[ row ].expanded = expanded;
        GUI::TTableViewRangeEventData range( GUI::CTableViewRange( row + 1, 2, 0, (UInt32) columns.size() ) );
        if ( expanded )
        {
            SRow first = MakeRow( "c1", 1.0, "" );
            SRow second = MakeRow( "c2", 2.0, "" );
            first.level = rows[ row ].level + 1;
            second.level = rows[ row ].level + 1;
            rows.insert( rows.begin() + ( row + 1 ), second );
            rows.insert( rows.begin() + ( row + 1 ), first );
            NotifyObservers( RowsInsertedEvent, &range );
        }
        else
        {
            rows.erase( rows.begin() + ( row + 1 ), rows.begin() + ( row + 3 ) );
            NotifyObservers( RowsRemovedEvent, &range );
        }
        return true;
    }
};

/** The menu of the tests: Open (with a key), a separator, Delete (not selectable); remembers what it was asked and told */
class CMenuProvider : public GUI::CIGridContextMenuProvider
{
    public:

    UInt32 asked;
    Int32 askedRow;
    Int32 askedColumn;
    bool giveItems;
    CORE::CString chosen;
    Int32 chosenRow;
    Int32 chosenColumn;

    CMenuProvider( void )
        : GUI::CIGridContextMenuProvider(), asked( 0 ), askedRow( -9 ), askedColumn( -9 ), giveItems( true ), chosen(), chosenRow( -9 ), chosenColumn( -9 )
    {
    }

    virtual bool GetContextMenuItems( const GUI::CGridView& grid, Int32 row, Int32 column, GUI::TContextMenuItemVector& items ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++asked;
        askedRow = row;
        askedColumn = column;
        if ( !giveItems )
            return false;
        items.push_back( GUI::CContextMenuItem( "open", "Open", "Enter" ) );
        items.push_back( GUI::CContextMenuItem::Separator() );
        items.push_back( GUI::CContextMenuItem( "delete", "Delete", CORE::CString::Empty, false ) );
        return true;
    }

    virtual void OnContextMenuItem( GUI::CGridView& grid, Int32 row, Int32 column, const CORE::CString& itemId ) GUCEF_VIRTUAL_OVERRIDE
    {
        chosen = itemId;
        chosenRow = row;
        chosenColumn = column;
    }
};

/** A model grid of the given size at (10,10) with the rows Alpha 1234.5 short and B 99 "a longer note text" */
GUI::CGridView*
MakeUxGrid( GUI::CGUIContext* context, CUxModel*& raw, Int32 width, Int32 height )
{
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, width, height );
    raw = GUCEF_NEW CUxModel();
    raw->rows.push_back( CUxModel::MakeRow( "Alpha", 1234.5, "short" ) );
    raw->rows.push_back( CUxModel::MakeRow( "B", 99.0, "a longer note text" ) );
    grid->SetModel( GUI::TTableViewModelPtr( raw ) );
    context->FlushChanges();
    return grid;
}

/** The text command with the text at the y, NULL when none */
const GUI::CDrawSurface::TCommand*
FindTextAt( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& text, Int32 y )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && text == commands[ i ].text && y == commands[ i ].y0 )
            return &commands[ i ];
    return GUCEF_NULL;
}

/** The clip the text command at the index of the text is drawn in: the last clip pushed before it */
bool
ClipOfText( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& text, GUI::CRect& clip )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        if ( GUI::CDrawSurface::COMMAND_TEXT != commands[ i ].type || !( text == commands[ i ].text ) )
            continue;
        for ( size_t j=i; j>0; --j )
        {
            if ( GUI::CDrawSurface::COMMAND_PUSHCLIP == commands[ j - 1 ].type )
            {
                clip = commands[ j - 1 ].rect;
                return true;
            }
        }
        return false;
    }
    return false;
}

bool
HasTextContaining( const GUI::CDrawSurface::TCommandVector& commands, const CORE::CString& part )
{
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type && commands[ i ].text.HasSubstr( part ) >= 0 )
            return true;
    return false;
}

bool
HasLine( const GUI::CDrawSurface::TCommandVector& commands, Int32 x0, Int32 y0, Int32 x1, Int32 y1 )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& c = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_LINE == c.type && c.x0 == x0 && c.y0 == y0 && c.x1 == x1 && c.y1 == y1 )
            return true;
    }
    return false;
}

bool
HasFillOfColour( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect, const GUI::CColour& colour )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& c = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_FILLRECT == c.type && c.rect == rect && c.colour == colour )
            return true;
    }
    return false;
}

bool
HasStroke( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& rect, const GUI::CColour& colour, Int32 thickness )
{
    for ( size_t i=0; i<commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& c = commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_STROKERECT == c.type && c.rect == rect && c.colour == colour && c.thickness == thickness )
            return true;
    }
    return false;
}

/** A click with the right button at a point of the context */
void
RightClickAt( GUI::CGUIContext* context, Int32 x, Int32 y )
{
    context->FlushChanges();
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, false );
    context->RenderFrame( 500 );
}

CORE::CString
Compact( const CORE::CVariant& value, const CORE::CString& prefix, const CORE::CString& suffix )
{
    GUI::CTableViewColumn column( "Value", CORE::DATATABLE_TYPE_FLOAT64 );
    column.prefix = prefix;
    column.suffix = suffix;
    CORE::CString text;
    if ( !GUI::CTableCellFormatter::FormatCompact( value, column, text ) )
        return "<none>";
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridUxTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridUx" );

    GUCEF_TESTFW_TESTCASE( "GridUx: The fitter grows the text columns or the Fill columns, keeps numbers at their content, caps at the maximum, shrinks by priority and reports the width that has to scroll" )
    try
    {
        TFitter::TColumnVector columns;
        columns.push_back( FitColumn( 100, 0, false ) );       // A: text with a width of its own
        columns.push_back( FitColumn( 0, 80, true ) );         // B: a number of 80 pixels of content
        columns.push_back( FitColumn( 50, 0, false ) );        // C: text
        TFitter::TInt32Vector widths;

        // minimum and nominal widths
        ASSERT_TRUE( 24 == TFitter::GetMinimumWidth( columns[ 0 ] ) && 80 == TFitter::GetMinimumWidth( columns[ 1 ] ) && 24 == TFitter::GetMinimumWidth( columns[ 2 ] ) );
        ASSERT_TRUE( 100 == TFitter::GetNominalWidth( columns[ 0 ] ) && 80 == TFitter::GetNominalWidth( columns[ 1 ] ) && 50 == TFitter::GetNominalWidth( columns[ 2 ] ) );

        // Fill, 400 pixels: 170 to spare for A and C by their widths: 170*100/150 = 113, 170*50/150 = 56, the last pixel to A
        ASSERT_TRUE( 400 == TFitter::Fit( columns, 400, true, widths ) );
        ASSERT_TRUE( 3 == widths.size() && 214 == widths[ 0 ] && 80 == widths[ 1 ] && 106 == widths[ 2 ] );

        // Content: the room to spare stays free
        ASSERT_TRUE( 230 == TFitter::Fit( columns, 400, false, widths ) );
        ASSERT_TRUE( 100 == widths[ 0 ] && 80 == widths[ 1 ] && 50 == widths[ 2 ] );

        // a maximum of 150 for A: A takes 50, C 56, and the 64 left go to C
        columns[ 0 ].fit.maxWidth = 150;
        ASSERT_TRUE( 400 == TFitter::Fit( columns, 400, true, widths ) );
        ASSERT_TRUE( 150 == widths[ 0 ] && 80 == widths[ 1 ] && 170 == widths[ 2 ] );
        columns[ 0 ].fit.maxWidth = 0;

        // a Fill column takes all the room to spare
        columns[ 2 ].fit.mode = GUI::CGridColumnFit::MODE_FILL;
        ASSERT_TRUE( 400 == TFitter::Fit( columns, 400, true, widths ) );
        ASSERT_TRUE( 100 == widths[ 0 ] && 80 == widths[ 1 ] && 220 == widths[ 2 ] );
        columns[ 2 ].fit.mode = GUI::CGridColumnFit::MODE_AUTO;

        // too little room, C has priority 1: A (priority 0) goes to its minimum first (76 of the 80), then C gives 4; the number keeps 80
        columns[ 2 ].fit.priority = 1;
        ASSERT_TRUE( 150 == TFitter::Fit( columns, 150, true, widths ) );
        ASSERT_TRUE( 24 == widths[ 0 ] && 80 == widths[ 1 ] && 46 == widths[ 2 ] );

        // far too little room: everything at its minimum, the total is what has to scroll
        ASSERT_TRUE( 128 == TFitter::Fit( columns, 100, true, widths ) );
        ASSERT_TRUE( 24 == widths[ 0 ] && 80 == widths[ 1 ] && 24 == widths[ 2 ] );

        // one priority, 30 pixels short of the 230 nominal: by the room above the minimum, 30*76/102 = 22 and 30*26/102 = 7, the last pixel from A
        columns[ 2 ].fit.priority = 0;
        ASSERT_TRUE( 200 == TFitter::Fit( columns, 200, true, widths ) );
        ASSERT_TRUE( 77 == widths[ 0 ] && 80 == widths[ 1 ] && 43 == widths[ 2 ] );

        // a number wider than its maximum keeps its content; a fixed column keeps its width
        TFitter::TColumn number = FitColumn( 0, 80, true, GUI::CGridColumnFit( GUI::CGridColumnFit::MODE_AUTO, 0, 60 ) );
        ASSERT_TRUE( 80 == TFitter::GetMaximumWidth( number ) && 80 == TFitter::GetNominalWidth( number ) );
        TFitter::TColumn fixed = FitColumn( 0, 300, false, GUI::CGridColumnFit( GUI::CGridColumnFit::MODE_FIXED ) );
        ASSERT_TRUE( 100 == TFitter::GetMinimumWidth( fixed ) && 100 == TFitter::GetNominalWidth( fixed ) );
        TFitter::TColumn content = FitColumn( 40, 70, false, GUI::CGridColumnFit( GUI::CGridColumnFit::MODE_CONTENT ) );
        ASSERT_TRUE( 70 == TFitter::GetMinimumWidth( content ) && 70 == TFitter::GetNominalWidth( content ) );

        // the names of the modes
        GUI::CGridColumnFit::EMode mode = GUI::CGridColumnFit::MODE_AUTO;
        ASSERT_TRUE( GUI::CGridColumnFit::StringToMode( "fill", mode ) && GUI::CGridColumnFit::MODE_FILL == mode && "Fill" == GUI::CGridColumnFit::ModeToString( mode ) );
        ASSERT_FALSE( GUI::CGridColumnFit::StringToMode( "stretch", mode ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: FitMode Content gives every column its content width, Fill shares the room at 1920 and 3440 pixels, maxima keep columns readable on an ultrawide grid, and a narrow grid scrolls instead of cutting the number" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 3600, 1500 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        ASSERT_TRUE( 40 == TextWidth( context, "Alpha" ) );        // the test font: 8 pixels a character

        CUxModel* raw = GUCEF_NULL;
        GUI::CGridView* grid = MakeUxGrid( context, raw, 1920, 300 );

        // the content: Name max( "Name" 32, "Alpha" 40 ) + 10; Value "$1,234.50" 72 + 10; Note "a longer note text" 144 + 10
        ASSERT_TRUE( 50 == grid->GetColumnContentWidth( 0 ) && 82 == grid->GetColumnContentWidth( 1 ) && 154 == grid->GetColumnContentWidth( 2 ) );

        // Proportional (the default): as before, the share of the room
        ASSERT_FALSE( grid->IsHorizontallyScrollable() );

        grid->SetProperty( "FitMode", "Content" );
        context->FlushChanges();
        GUCEF::vector< Int32 > widths = grid->GetColumnWidths();
        ASSERT_TRUE( 3 == widths.size() && 50 == widths[ 0 ] && 82 == widths[ 1 ] && 154 == widths[ 2 ] );
        ASSERT_TRUE( 286 == grid->GetColumnLayout().GetContentWidth() && grid->IsHorizontallyScrollable() );

        // Fill at 1920: 1634 to spare for the text columns, 1634*50/204 = 400 and 1634*154/204 = 1233, the last pixel to Name
        grid->SetProperty( "FitMode", "Fill" );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 451 == widths[ 0 ] && 82 == widths[ 1 ] && 1387 == widths[ 2 ] );

        // Fill at 3440: 3154 to spare, 773 and 2380, the last pixel to Name
        grid->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, 3440 ), GUI::CUDim( 0.0f, 300 ) ) );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 824 == widths[ 0 ] && 82 == widths[ 1 ] && 2534 == widths[ 2 ] );

        // with maxima the columns stay readable and the rest of the width stays free for something else
        grid->SetColumnFit( 0, GUI::CGridColumnFit( GUI::CGridColumnFit::MODE_AUTO, 0, 160 ) );
        grid->SetColumnFit( 2, GUI::CGridColumnFit( GUI::CGridColumnFit::MODE_AUTO, 0, 320 ) );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 160 == widths[ 0 ] && 82 == widths[ 1 ] && 320 == widths[ 2 ] );
        ASSERT_TRUE( 320 == grid->GetColumnFit( 2 ).maxWidth && 0 == grid->GetColumnFit( 1 ).maxWidth );
        grid->ClearColumnFit( 0 );
        grid->ClearColumnFit( 2 );

        // narrow, 200 pixels: 86 too many, taken from Name (26 above its minimum) and Note (130): 14 + 1 and 71
        grid->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, 200 ), GUI::CUDim( 0.0f, 300 ) ) );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 35 == widths[ 0 ] && 82 == widths[ 1 ] && 83 == widths[ 2 ] );

        // 120 pixels: the text columns at their minimum, the number whole; the grid scrolls the 10 pixels left over
        grid->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, 120 ), GUI::CUDim( 0.0f, 300 ) ) );
        context->FlushChanges();
        widths = grid->GetColumnWidths();
        ASSERT_TRUE( 24 == widths[ 0 ] && 82 == widths[ 1 ] && 24 == widths[ 2 ] );
        ASSERT_TRUE( 10 == grid->GetColumnLayout().GetMaxScrollX() );
        grid->ScrollToCell( 0, 2 );
        ASSERT_TRUE( 10 == grid->GetColumnLayout().GetScrollX() );

        // a value that grows widens its column, measured again when the data change
        raw->rows[ 0 ].value = 1234567.0;
        raw->ChangedCell( 0, 1 );
        context->FlushChanges();
        ASSERT_TRUE( 114 == grid->GetColumnContentWidth( 1 ) && 114 == grid->GetColumnWidths()[ 1 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: A footer number wider than its column is drawn compact or as '#' inside its cell, never clipped on the left; footer text is cut with an ellipsis; every footer cell is clipped to its own rectangle" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CUxModel* raw = GUCEF_NULL;
        GUI::CGridView* grid = MakeUxGrid( context, raw, 400, 200 );
        raw->footer = true;
        raw->footerValue = 1234567.0;
        raw->footerName = "Total of all positions here";
        grid->SetProperty( "ColumnWidths", "100,60,100" );
        context->FlushChanges();
        ASSERT_TRUE( grid->IsFooterShown() && 180 == grid->GetFooterTop() );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );

        // "$1,234,567.00" is 104 pixels, the room 50: "$1.23M" (48), right aligned inside the cell: 100 + 60 - 8 - 48
        const GUI::CDrawSurface::TCommand* total = FindTextAt( commands, "$1.23M", 180 );
        ASSERT_TRUE( GUCEF_NULL != total && 104 == total->x0 );
        ASSERT_FALSE( HasTextContaining( commands, "1,234,567" ) );
        GUI::CRect clip;
        ASSERT_TRUE( ClipOfText( commands, "$1.23M", clip ) && GUI::CRect( 100, 180, 60, 20 ) == clip );

        // the text of the footer: cut at its end with "...", in its own cell
        const GUI::CDrawSurface::TCommand* name = FindTextAt( commands, "Total of...", 180 );
        ASSERT_TRUE( GUCEF_NULL != name && 2 == name->x0 );
        ASSERT_TRUE( ClipOfText( commands, "Total of...", clip ) && GUI::CRect( 0, 180, 100, 20 ) == clip );

        // a cell number that does not fit: "$1,234.50" (72) in 50 is "$1.23K"; "$99.00" fits and is drawn as it is
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "$1.23K", 20 ) && 104 == FindTextAt( commands, "$1.23K", 20 )->x0 );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "$99.00", 40 ) && 100 + 60 - 8 - 48 == FindTextAt( commands, "$99.00", 40 )->x0 );

        // the tooltips have the whole numbers
        ASSERT_TRUE( "$1,234.50" == grid->GetTooltipAt( 130, 30 ) );
        ASSERT_TRUE( grid->IsFooterTextCut( 1 ) && "$1,234,567.00" == grid->GetTooltipAt( 130, 190 ) );
        ASSERT_TRUE( "Total of all positions here" == grid->GetTooltipAt( 50, 190 ) );

        // 40 pixels: the compact form does not fit the room of 30 either: three '#' (24 pixels), right aligned
        grid->SetProperty( "ColumnWidths", "100,40,100" );
        commands = PaintedCommands( context, grid );
        const GUI::CDrawSurface::TCommand* marks = FindTextAt( commands, "###", 180 );
        ASSERT_TRUE( GUCEF_NULL != marks && 100 + 40 - 8 - 24 == marks->x0 );
        ASSERT_FALSE( HasTextContaining( commands, "$1.2" ) && GUCEF_NULL != FindTextAt( commands, "$1.23M", 180 ) );

        // no text of a footer cell starts left of its cell or is drawn outside the clip of its cell
        for ( size_t i=0; i<commands.size(); ++i )
        {
            if ( GUI::CDrawSurface::COMMAND_TEXT != commands[ i ].type || 180 != commands[ i ].y0 )
                continue;
            GUI::CRect cellClip;
            ASSERT_TRUE( ClipOfText( commands, commands[ i ].text, cellClip ) );
            ASSERT_TRUE( commands[ i ].x0 >= cellClip.left && cellClip.Right() <= 240 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: Right aligned text wider than its cell starts at the left of its room; text that fits and ellipsized text keep their alignment" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 400, 200 );
        grid->SetProperty( "Columns", "A\nB" );
        grid->SetProperty( "ColumnWidths", "60,60" );
        grid->SetProperty( "ColumnAlignments", "Right\nRight" );
        GUI::CGridView::TRowVector rows;
        GUI::CGridView::TStringVector cells;
        cells.push_back( "ABCDEFGHIJ" );
        cells.push_back( "XY" );
        rows.push_back( cells );
        grid->SetRows( rows );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );

        // 80 pixels in a room of 50: from the left of the room (it used to start 28 pixels left of the cell)
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "ABCDEFGHIJ", 20 ) && 2 == FindTextAt( commands, "ABCDEFGHIJ", 20 )->x0 );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "XY", 20 ) && 60 + 60 - 8 - 16 == FindTextAt( commands, "XY", 20 )->x0 );

        // with an ellipsis the cut text fits and keeps the alignment: "ABCD..." is 56 > 50, "ABC..." 48: 60 - 8 - 48
        grid->SetProperty( "TextOverflow", "Ellipsis" );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "ABC...", 20 ) && 60 - 8 - 48 == FindTextAt( commands, "ABC...", 20 )->x0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: A cell status gives the text and the marker its colour token; a marker follows the value, stands in a fixed slot that keeps the digits aligned, or leads; a link is underlined; StatusColours overrides a token" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CUxModel* raw = GUCEF_NULL;
        GUI::CGridView* grid = MakeUxGrid( context, raw, 400, 200 );
        raw->rows.push_back( CUxModel::MakeRow( "Link", 5.0, "go" ) );
        raw->rows[ 0 ].statusColumn = 1;
        raw->rows[ 0 ].status = GUI::SCellContent::STATUS_ESTIMATE;
        raw->rows[ 0 ].marker = "est.";
        raw->rows[ 1 ].statusColumn = 0;
        raw->rows[ 1 ].status = GUI::SCellContent::STATUS_ATTENTION;
        raw->rows[ 1 ].marker = "!";
        raw->rows[ 1 ].markerLeading = true;
        raw->rows[ 2 ].statusColumn = 2;
        raw->rows[ 2 ].status = GUI::SCellContent::STATUS_LINK;
        raw->Reset();
        grid->SetProperty( "ColumnWidths", "100,140,100" );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const GUI::CColour estimate( 150, 190, 240, 255 );
        const GUI::CColour attention( 240, 176, 72, 255 );
        const GUI::CColour link( 120, 170, 255, 255 );

        // the marker follows the value: the room is 140 - 10 - 40, the value right aligned before it: 100 + 100 - 8 - 72
        const GUI::CDrawSurface::TCommand* value = FindTextAt( commands, "$1,234.50", 20 );
        const GUI::CDrawSurface::TCommand* marker = FindTextAt( commands, " est.", 20 );
        ASSERT_TRUE( GUCEF_NULL != value && GUCEF_NULL != marker && 120 == value->x0 && 192 == marker->x0 );
        ASSERT_TRUE( GUCEF_NULL != value && GUCEF_NULL != marker && estimate == value->colour && estimate == marker->colour );
        ASSERT_TRUE( "$1,234.50" == grid->GetText( 1, 0 ) );          // the text of the cell is the value: the marker is not part of it

        // a leading marker at the start of the cell, the value after it
        const GUI::CDrawSurface::TCommand* sign = FindTextAt( commands, "!", 40 );
        const GUI::CDrawSurface::TCommand* name = FindTextAt( commands, "B", 40 );
        ASSERT_TRUE( GUCEF_NULL != sign && GUCEF_NULL != name && 2 == sign->x0 && 12 == name->x0 && attention == sign->colour && attention == name->colour );

        // a link is underlined below its text, one line height down
        const GUI::CDrawSurface::TCommand* go = FindTextAt( commands, "go", 60 );
        ASSERT_TRUE( GUCEF_NULL != go && link == go->colour && 242 == go->x0 );
        const Int32 underline = 60 + context->GetLineHeight( "Test" );
        ASSERT_TRUE( HasLine( commands, 242, underline, 242 + 16 - 1, underline ) );

        // a slot of 40 pixels for the markers of the column: the digits of every row end at the same place
        raw->columns[ 1 ].markerSlot = 40;
        commands = PaintedCommands( context, grid );
        value = FindTextAt( commands, "$1,234.50", 20 );
        marker = FindTextAt( commands, "est.", 20 );
        const GUI::CDrawSurface::TCommand* plain = FindTextAt( commands, "$99.00", 40 );
        ASSERT_TRUE( GUCEF_NULL != value && GUCEF_NULL != marker && GUCEF_NULL != plain );
        if ( GUCEF_NULL != value && GUCEF_NULL != marker && GUCEF_NULL != plain )
        {
            ASSERT_TRUE( 120 == value->x0 && 240 - 40 + 2 == marker->x0 && value->x0 + 72 == plain->x0 + 48 );
        }
        ASSERT_TRUE( 40 == grid->GetColumnMarkerSlot( 1 ) && 140 - 10 - 40 == grid->GetCellTextRoom( 1, 1 ) );

        // the theme or the application sets the tokens
        grid->SetProperty( "StatusColours", "estimate=1,2,3,255" );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "est.", 20 ) && GUI::CColour( 1, 2, 3, 255 ) == FindTextAt( commands, "est.", 20 )->colour );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "!", 40 ) && attention == FindTextAt( commands, "!", 40 )->colour );

        // the colours and their names
        GUI::CCellStatusColours colours;
        GUI::CColour colour;
        ASSERT_FALSE( colours.GetColour( GUI::SCellContent::STATUS_NORMAL, colour ) );
        ASSERT_TRUE( colours.GetColour( GUI::SCellContent::STATUS_NEGATIVE, colour ) && GUI::CColour( 232, 110, 100, 255 ) == colour );
        ASSERT_FALSE( colours.Parse( "attention=1,2,3,255\nbogus=1,1,1\nlink=#0a0b0c" ) );
        ASSERT_TRUE( colours.GetColour( GUI::SCellContent::STATUS_ATTENTION, colour ) && GUI::CColour( 1, 2, 3, 255 ) == colour );
        ASSERT_TRUE( colours.GetColour( GUI::SCellContent::STATUS_LINK, colour ) && GUI::CColour( 10, 11, 12, 255 ) == colour );
        ASSERT_TRUE( colours.ToString().HasSubstr( "attention=1,2,3,255" ) >= 0 );
        GUI::SCellContent::EStatus status = GUI::SCellContent::STATUS_NORMAL;
        ASSERT_TRUE( GUI::CCellStatusColours::StringToStatus( "Estimate", status ) && GUI::SCellContent::STATUS_ESTIMATE == status );
        ASSERT_TRUE( CORE::CString( "unknown" ) == GUI::CCellStatusColours::StatusToString( GUI::SCellContent::STATUS_UNKNOWN ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: EmphasisStyle Bar marks a row with the bar only and SelectionOutline outlines the selected row; at their defaults the grid draws what it drew" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 300, 200 );
        grid->SetProperty( "Columns", "Name\nValue" );
        grid->SetProperty( "ColumnWidths", "100,100" );
        GUI::CGridView::TRowVector rows;
        for ( UInt32 r=0; r<3; ++r )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( CORE::CString( "row " ) + CORE::UInt32ToString( r ) );
            cells.push_back( CORE::UInt32ToString( r ) );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
        GUI::CGridView::TInt32Vector emphasized;
        emphasized.push_back( 1 );
        grid->SetEmphasizedRows( emphasized );
        grid->SetSelectedRow( 1 );
        const UInt64 before = HashCommands( PaintedCommands( context, grid ) );

        // the defaults written out draw the same
        grid->SetProperty( "EmphasisStyle", "Row" );
        grid->SetBoolProperty( "SelectionOutline", false );
        grid->SetProperty( "FitMode", "Proportional" );
        grid->SetProperty( "StatusColours", "" );
        ASSERT_TRUE( before == HashCommands( PaintedCommands( context, grid ) ) );

        // Bar: no tint over the row, the bar stays, the text keeps the text colour; the outline in the text colour
        grid->SetProperty( "EmphasisStyle", "Bar" );
        grid->SetBoolProperty( "SelectionOutline", true );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        const GUI::CRect rowRect( 0, 40, 300, 20 );
        const GUI::CColour textColour = grid->GetColourProperty( GUI::EWidgetProperty_TextColour );
        ASSERT_FALSE( HasFillOfColour( commands, rowRect, GUI::CColour( 74, 136, 232, 56 ) ) );
        ASSERT_TRUE( HasFillOfColour( commands, GUI::CRect( 0, 40, 4, 20 ), GUI::CColour( 74, 136, 232, 255 ) ) );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "row 1", 40 ) && textColour == FindTextAt( commands, "row 1", 40 )->colour );
        ASSERT_TRUE( HasStroke( commands, rowRect, textColour, 2 ) );
        ASSERT_FALSE( HasStroke( commands, GUI::CRect( 0, 20, 300, 20 ), textColour, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: The compact form of a number has three significant digits with K, M, B or T, the sign before the prefix and the suffix after it" )
    try
    {
        ASSERT_TRUE( "$1.23M" == Compact( CORE::CVariant( (Float64) 1234567.0 ), "$", "" ) );
        ASSERT_TRUE( "-$45.2K" == Compact( CORE::CVariant( (Float64) -45210.0 ), "$", "" ) );
        ASSERT_TRUE( "$156" == Compact( CORE::CVariant( (Float64) 155.78 ), "$", "" ) );
        ASSERT_TRUE( "$1M" == Compact( CORE::CVariant( (Float64) 999999.0 ), "$", "" ) );
        ASSERT_TRUE( "100" == Compact( CORE::CVariant( (Float64) 99.96 ), "", "" ) );
        ASSERT_TRUE( "0.5%" == Compact( CORE::CVariant( (Float64) 0.5 ), "", "%" ) );
        ASSERT_TRUE( "1.5K" == Compact( CORE::CVariant( (Int64) 1500 ), "", "" ) );
        ASSERT_TRUE( "2B" == Compact( CORE::CVariant( (UInt64) 2000000000ULL ), "", "" ) );
        ASSERT_TRUE( "3.2T" == Compact( CORE::CVariant( (Float64) 3.2e12 ), "", "" ) );
        ASSERT_TRUE( "1000T" == Compact( CORE::CVariant( (Float64) 1.0e15 ), "", "" ) );
        ASSERT_TRUE( "0" == Compact( CORE::CVariant( (Float64) -0.001 ), "", "" ) );
        ASSERT_TRUE( "<none>" == Compact( CORE::CVariant( CORE::CString( "text" ) ), "", "" ) );
        ASSERT_TRUE( "<none>" == Compact( CORE::CVariant( true ), "", "" ) );
        ASSERT_TRUE( "<none>" == Compact( CORE::CVariant(), "", "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: With a provider a right click, the menu key or Shift+F10 opens the context menu of the cell with the provider's items; a choice reaches the provider and the event; without one nothing opens" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 300, 200 );
        grid->SetProperty( "Columns", "Name\nValue" );
        grid->SetProperty( "ColumnWidths", "100,100" );
        GUI::CGridView::TRowVector rows;
        for ( UInt32 r=0; r<5; ++r )
        {
            GUI::CGridView::TStringVector cells;
            cells.push_back( CORE::CString( "row " ) + CORE::UInt32ToString( r ) );
            cells.push_back( CORE::UInt32ToString( r ) );
            rows.push_back( cells );
        }
        grid->SetRows( rows );
        CEventCount chosenEvents( grid, GUI::CGridView::ContextMenuItemChosenEvent );
        CMenuProvider provider;
        Int32 x = 0, y = 0;

        // no provider: a right click does nothing of the grid's own
        ASSERT_TRUE( CellPoint( context, grid, 2, 1, x, y ) );
        RightClickAt( context, x, y );
        ASSERT_TRUE( GUCEF_NULL == context->GetPopup() && -1 == grid->GetSelectedRow() && 0 == provider.asked );

        // with one: the row is selected and the menu of the cell opens with the items
        grid->SetContextMenuProvider( &provider );
        RightClickAt( context, x, y );
        ASSERT_TRUE( 1 == provider.asked && 2 == provider.askedRow && 1 == provider.askedColumn );
        ASSERT_TRUE( 2 == grid->GetSelectedRow() && grid->IsContextMenuOpen() && 2 == grid->GetContextMenuRow() && 1 == grid->GetContextMenuColumn() );
        GUI::CPopupMenu* menu = GUI::CWidget::As< GUI::CPopupMenu >( context->GetPopup() );
        ASSERT_TRUE( GUCEF_NULL != menu );
        if ( GUCEF_NULL != menu )
        {
            const GUI::CPopupMenu::TStringVector items = menu->GetItems();
            ASSERT_TRUE( 3 == items.size() && "Open   Enter" == items[ 0 ] && "-" == items[ 1 ] && "Delete" == items[ 2 ] );
            ASSERT_TRUE( menu->IsItemSelectable( 0 ) && !menu->IsItemSelectable( 1 ) && !menu->IsItemSelectable( 2 ) );
        }
        ASSERT_TRUE( 3 == grid->GetContextMenuItems().size() && "delete" == grid->GetContextMenuItems()[ 2 ].id );

        // a disabled item cannot be chosen; Open can
        ASSERT_FALSE( grid->ChooseContextMenuItem( "delete" ) );
        ASSERT_TRUE( grid->ChooseContextMenuItem( "open" ) );
        ASSERT_TRUE( "open" == provider.chosen && 2 == provider.chosenRow && 1 == provider.chosenColumn && 1 == chosenEvents.count );
        ASSERT_TRUE( !grid->IsContextMenuOpen() && GUCEF_NULL == context->GetPopup() );
        ASSERT_FALSE( grid->ChooseContextMenuItem( "open" ) );          // no menu is open

        // the menu key: the menu of the selected row, for the first shown column
        context->SetFocus( grid );
        grid->SetSelectedRow( 3 );
        PressKey( context, INPUT::KEYCODE_APPS );
        ASSERT_TRUE( 2 == provider.asked && 3 == provider.askedRow && 0 == provider.askedColumn && grid->IsContextMenuOpen() );
        grid->CloseContextMenu();
        ASSERT_FALSE( grid->IsContextMenuOpen() );

        // Shift+F10 does the same
        context->SetFocus( grid );
        Chord( context, INPUT::KEYCODE_F10, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 3 == provider.asked && 3 == provider.askedRow && grid->IsContextMenuOpen() );
        grid->CloseContextMenu();

        // a provider without items for the cell: no menu
        provider.giveItems = false;
        RightClickAt( context, x, y );
        ASSERT_TRUE( 4 == provider.asked && !grid->IsContextMenuOpen() );
        grid->SetContextMenuProvider( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridUx: Rows of a model with a hierarchy get an indent and a fold control; a click on it, Right, Left and numpad - open and close the row through the model and raise the event" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 400, 200 );
        CUxModel* raw = GUCEF_NEW CUxModel();
        raw->hierarchy = true;
        raw->rows.push_back( CUxModel::MakeRow( "Parent", 10.0, "" ) );
        raw->rows.push_back( CUxModel::MakeRow( "L", 20.0, "" ) );
        raw->rows[ 0 ].expandable = true;
        grid->SetModel( GUI::TTableViewModelPtr( raw ) );
        grid->SetProperty( "ColumnWidths", "200,100,100" );
        context->FlushChanges();
        CEventCount expansions( grid, GUI::CGridView::RowExpansionChangedEvent );
        CEventCount clicks( grid, GUI::CGridView::GridItemClickedEvent );

        // the control of the closed parent: a triangle to the right in the 16 pixels after the padding; the text after it, the leaf aligned with it
        ASSERT_TRUE( grid->HasRowHierarchy() && grid->IsRowExpandable( 0 ) && !grid->IsRowExpandable( 1 ) );
        GUI::CRect fold;
        ASSERT_TRUE( grid->GetFoldControlRect( 0, fold ) && GUI::CRect( 2, 20, 16, 20 ) == fold && !grid->GetFoldControlRect( 1, fold ) );
        GUI::CDrawSurface::TCommandVector commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasLine( commands, 9, 27, 9, 33 ) );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "Parent", 20 ) && 18 == FindTextAt( commands, "Parent", 20 )->x0 );
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "L", 40 ) && 18 == FindTextAt( commands, "L", 40 )->x0 );
        ASSERT_TRUE( 200 - 10 - 16 == grid->GetCellTextRoom( 0, 0 ) );
        ASSERT_TRUE( GUI::CGridView::HIT_FOLD_BUTTON == grid->HitTest( 10, 30 ).kind && GUI::CGridView::HIT_CELL == grid->HitTest( 60, 30 ).kind );

        // the content width counts the control: "Parent" 48 + 10 + 16
        grid->SetProperty( "FitMode", "Content" );
        ASSERT_TRUE( 74 == grid->GetColumnContentWidth( 0 ) );
        grid->SetProperty( "FitMode", "Proportional" );

        // a click on the control opens the row: two child rows one level deeper, no click on the row
        ClickAt( context, 10 + 10, 10 + 30 );
        ASSERT_TRUE( 1 == raw->expandCalls && 4 == grid->GetRowCount() && grid->IsRowExpanded( 0 ) && 1 == expansions.count && 0 == clicks.count );
        ASSERT_TRUE( 1 == grid->GetRowLevel( 1 ) && 0 == grid->GetRowLevel( 3 ) );
        commands = PaintedCommands( context, grid );
        ASSERT_TRUE( HasLine( commands, 7, 29, 13, 29 ) );                                             // the triangle points down now
        ASSERT_TRUE( GUCEF_NULL != FindTextAt( commands, "c1", 40 ) && 34 == FindTextAt( commands, "c1", 40 )->x0 );

        // keys on the selected row: Left closes, Right opens, numpad - closes
        grid->SetSelectedRow( 0 );
        context->SetFocus( grid );
        PressKey( context, INPUT::KEYCODE_LEFT );
        ASSERT_TRUE( 2 == grid->GetRowCount() && !grid->IsRowExpanded( 0 ) && 2 == expansions.count );
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( 4 == grid->GetRowCount() && 3 == expansions.count );
        PressKey( context, INPUT::KEYCODE_SUBTRACT );
        ASSERT_TRUE( 2 == grid->GetRowCount() && 4 == expansions.count );

        // a leaf cannot be opened
        ASSERT_FALSE( grid->SetRowExpanded( 1, true ) );
        ASSERT_TRUE( 4 == raw->expandCalls );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
