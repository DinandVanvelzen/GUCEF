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

#include <stdio.h>

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

/** Rows of 'columns' cells that depend on the row and column only */
GUI::CGridView::TRowVector
MakeRows( UInt32 count, UInt32 columns )
{
    GUI::CGridView::TRowVector rows;
    rows.reserve( count );
    for ( UInt32 r=0; r<count; ++r )
    {
        GUI::CGridView::TStringVector cells;
        for ( UInt32 c=0; c<columns; ++c )
        {
            if ( 0 == c )
                cells.push_back( CORE::CString( "item " ) + CORE::UInt32ToString( r ) );
            else
            if ( 1 == c )
                cells.push_back( CORE::UInt32ToString( ( r * 7919 ) % 10007 ) + ".50" );
            else
                cells.push_back( CORE::CString( "note " ) + CORE::UInt32ToString( r ) + " of the list, a text that is wider than its column" );
        }
        rows.push_back( cells );
    }
    return rows;
}

/** The look of a grid that does not use any of the optional features: one hash per scene, to be equal to what the grid drew before they existed */
UInt64
GoldenScene( UInt32 scene )
{
    GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return 0;
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 100, 400, 130 );
    grid->SetProperty( "Columns", "Name\nValue\nFlags" );
    grid->SetProperty( "ColumnWidths", "100,100,200" );
    grid->SetProperty( "ColumnAlignments", "Left\nRight" );

    UInt64 hash = 0;
    switch ( scene )
    {
        case 0:
        {
            grid->SetRows( MakeRows( 6, 3 ) );
            break;
        }
        case 1:
        {
            grid->SetRows( MakeRows( 6, 3 ) );
            grid->SetProperty( "TextOverflow", "Ellipsis" );
            grid->SetIntProperty( "SortColumn", 1 );
            grid->SetBoolProperty( "SortAscending", false );
            grid->SetSelectedRow( 2 );
            grid->SetIntProperty( "HoverIndex", 3 );
            break;
        }
        case 2:
        {
            grid->SetRows( MakeRows( 12, 3 ) );
            grid->SetAllowMultiSelect( true );
            GUI::CGridView::TInt32Vector selected;
            selected.push_back( 1 ); selected.push_back( 3 ); selected.push_back( 4 );
            grid->SetSelectedRows( selected );
            GUI::CGridView::TInt32Vector emphasized;
            emphasized.push_back( 2 ); emphasized.push_back( 3 );
            grid->SetEmphasizedRows( emphasized );
            grid->SetIntProperty( "ScrollPosition", 1 );
            break;
        }
        case 3:
        {
            grid->SetProperty( "ColumnWidths", "1,2,1" );
            grid->SetBoolProperty( "FitColumns", true );
            grid->SetProperty( "ColumnAlignments", "Center\nRight\nLeft" );
            grid->SetProperty( "TextOverflow", "Ellipsis" );
            grid->SetRows( MakeRows( 5, 3 ) );
            break;
        }
        case 4:
        {
            grid->SetProperty( "Columns", "First\nSecond" );
            grid->ResetProperty( "ColumnWidths" );
            grid->ResetProperty( "ColumnAlignments" );
            grid->SetRows( MakeRows( 5, 4 ) );
            break;
        }
        case 5:
        {
            break;
        }
        case 6:
        {
            grid->SetRows( MakeRows( 8, 3 ) );
            grid->SetAllowMultiSelect( true );
            grid->SetProperty( "TextOverflow", "Ellipsis" );
            context->FlushChanges();
            const GUI::CRect& rect = grid->GetRect();
            const Int32 rowHeight = grid->GetRowHeight();
            context->InjectMouseMove( rect.left + 12, rect.top + rowHeight + rowHeight / 2 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 500 );
            context->InjectKey( INPUT::KEYCODE_LSHIFT, true, INPUT::KEYMOD_SHIFT );
            context->InjectMouseMove( rect.left + 12, rect.top + rowHeight * 4 + rowHeight / 2 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->InjectKey( INPUT::KEYCODE_LSHIFT, false, 0 );
            context->RenderFrame( 500 );
            context->InjectKey( INPUT::KEYCODE_DOWN, true, 0 );
            context->InjectKey( INPUT::KEYCODE_DOWN, false, 0 );
            hash = FnvText( hash, grid->GetProperty( "SelectedRows" ) );
            hash = FnvText( hash, grid->GetProperty( "SelectedRow" ) );
            break;
        }
        case 7:
        {
            grid->SetRows( MakeRows( 60, 3 ) );
            context->FlushChanges();
            const GUI::CRect& rect = grid->GetRect();
            context->InjectMouseMove( rect.left + 40, rect.top + 60 );
            for ( UInt32 i=0; i<5; ++i )
                context->InjectMouseWheel( -1 );
            hash = FnvText( hash, grid->GetProperty( "ScrollPosition" ) );
            break;
        }
        case 8:
        {
            GUI::CGridView::TRowVector rows = MakeRows( 4, 3 );
            rows[ 1 ][ 0 ] = CORE::CString::Empty;
            rows[ 2 ][ 1 ] = CORE::CString::Empty;
            rows[ 3 ].resize( 1 );
            grid->SetRows( rows );
            grid->SetProperty( "Sortable", "true" );
            break;
        }
        default:
            return 0;
    }
    return FnvInt( hash, (Int64) HashCommands( PaintedCommands( context, grid ) ) );
}

/** The hash as 0x and sixteen hexadecimal digits */
CORE::CString
HexText( UInt64 hash )
{
    static const char digits[] = "0123456789abcdef";
    CORE::CString text( "0x" );
    for ( Int32 shift = 60; shift >= 0; shift -= 4 )
        text += digits[ ( hash >> shift ) & 0xF ];
    return text;
}

/** The scenes of GoldenScene() and the hashes the grid gave before the optional features were added */
const UInt32 GoldenSceneCount = 9;
const UInt64 GoldenHashes[ GoldenSceneCount ] =
{
    0xb8667adfc79129cfULL, 0x477a05783c293c78ULL, 0xd3ceee2252c9eb51ULL, 0x42ca060042f35758ULL, 0x508c793a61d6a79fULL,
    0x33cc017583a6c40aULL, 0xabf84f0f273cb333ULL, 0x02e3001e512c7d2dULL, 0xf6db12aaa3a6c2bbULL
};

/** A grid of 'rowCount' rows in a context of 1280 x 960: the time per frame of painting it, in microseconds */
void
MeasurePaint( UInt32 rowCount, UInt32 frames, double& firstFrame, double& steadyFrame )
{
    GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    firstFrame = 0.0;
    steadyFrame = 0.0;
    if ( GUCEF_NULL == context )
        return;
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 10, 10, 1000, 600 );
    grid->SetProperty( "Columns", "Name\nValue\nFlags" );
    grid->SetProperty( "ColumnWidths", "200,100,600" );
    UInt64 setupStart = MT::PrecisionTickCount();
    grid->SetRows( MakeRows( rowCount, 3 ) );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: SetRows " << rowCount << ": " << (UInt64) Micros( MT::PrecisionTickCount() - setupStart ) << " us" );
    grid->SetIntProperty( "ScrollPosition", (Int32) ( rowCount / 2 ) );
    context->FlushChanges();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: setup " << rowCount << ": " << (UInt64) Micros( MT::PrecisionTickCount() - setupStart ) << " us" );

    GUI::CDrawSurface::TCommandVector commands;
    UInt64 start = MT::PrecisionTickCount();
    GUI::CWidgetPainter::Paint( *grid, *context, commands );
    firstFrame = Micros( MT::PrecisionTickCount() - start );

    start = MT::PrecisionTickCount();
    for ( UInt32 f=0; f<frames; ++f )
    {
        commands.clear();
        GUI::CWidgetPainter::Paint( *grid, *context, commands );
    }
    steadyFrame = Micros( MT::PrecisionTickCount() - start ) / (double) frames;
}


/** A context of the given size with the bitmap font "Test" and, when the font plugin is there, the UI font "UI" */
GUI::TGuiContextPtr
MakeRenderContext( Int32 width, Int32 height, CORE::CString& font )
{
    GUI::TGuiContextPtr contextPtr = MakeContext( width, height );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    font = "Test";
    if ( GUCEF_NULL != context && LoadFreeTypeFontPlugin() && GUI::CGuiGlobal::Instance()->MountData() )
    {
        GUI::TBitmapFontPtr ui = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().RasterizeFont( GUI::CGuiGlobal::DefaultUiFontPath, 14, GUI::CFontRasterizer::GetLatin1Ranges() );
        if ( !ui.IsNULL() && context->RegisterFont( "UI", ui ) )
            font = "UI";
    }
    return contextPtr;
}

bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance && abs( (Int32) a.b - (Int32) b.b ) <= tolerance;
}

/** Whether every pixel of the rows and columns outside the rectangle is the colour: nothing is drawn outside the widget */
bool
NothingOutside( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect, const GUI::CColour& colour )
{
    const Int32 width = (Int32) pixels->GetWidthInPixels();
    const Int32 height = (Int32) pixels->GetHeightInPixels();
    for ( Int32 y=0; y<height; ++y )
    {
        for ( Int32 x=0; x<width; ++x )
        {
            if ( x >= rect.left && x < rect.Right() && y >= rect.top && y < rect.Bottom() )
            {
                x = rect.Right() - 1;
                continue;
            }
            if ( !Near( PixelAt( pixels, x, y ), colour, 0 ) )
                return false;
        }
    }
    return true;
}

const GUI::CColour SheetBackground( 24, 24, 28, 255 );

/** The sheet scene: a table of 100,000 rows with the features of a sheet, to the full size of the window */
GUI::CGridView*
MakeSheet( GUI::CGUIContext* context, const CORE::CString& font, Int32 width, Int32 height, UInt32 variant, CTestModel*& model )
{
    GUI::CWidget* screen = MakeScreen( context );
    screen->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, GUI::CColour( 13, 13, 13, 255 ) );
    GUI::CGridView* grid = MakeGrid( context, screen, 8, 8, width - 16, height - 16 );
    grid->SetProperty( "Font", font );
    grid->SetIntProperty( "RowHeight", 22 );
    grid->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, SheetBackground );
    model = GUCEF_NEW CTestModel( 100000 );
    GUI::TTableViewModelPtr modelPtr( model );
    grid->SetModel( modelPtr );
    grid->SetProperty( "SelectionMode", "Cell" );
    grid->SetBoolProperty( "ShowRowHeader", true );
    grid->SetBoolProperty( "ScrollBars", true );
    grid->SetBoolProperty( "ResizableColumns", true );
    grid->SetBoolProperty( "MovableColumns", true );
    grid->SetProperty( "TextOverflow", "Ellipsis" );
    model->SetSortable( true );
    context->FlushChanges();
    context->SetFocus( grid );

    if ( 0 == variant )
    {
        // narrow: the columns are wider than the window, the first two stay while the others scroll
        grid->SetBoolProperty( "HorizontalScroll", true );
        grid->SetProperty( "ColumnWidths", "150,180,260,100,180,280,160,200" );
        grid->SetIntProperty( "FrozenColumns", 2 );
        grid->SetIntProperty( "FrozenRows", 2 );
        grid->SetProperty( "ZebraColour", "30,32,40,255" );
        grid->SetProperty( "GridLines", "Horizontal" );
        grid->SetIntProperty( "ScrollPosition", 40 );
        grid->SetIntProperty( "ScrollPositionX", 230 );
        grid->SetIntProperty( "SortColumn", 1 );
        grid->SetBoolProperty( "SortAscending", true );
        grid->SelectRange( 44, 3, 49, 5 );
    }
    else
    if ( 1 == variant )
    {
        // wide: the columns share the window, a footer, lines both ways, rows and columns selected whole
        grid->SetBoolProperty( "FitColumns", true );
        grid->SetProperty( "ColumnWidths", "90,110,160,60,110,180,100,120" );
        grid->SetProperty( "ZebraColour", "30,32,40,255" );
        grid->SetProperty( "GridLines", "Both" );
        model->footer = true;
        grid->SetBoolProperty( "ShowFooter", true );
        grid->SetIntProperty( "ScrollPosition", 500 );
        grid->MoveColumn( 1, 6 );
        grid->SetProperty( "SelectedRanges", "512,1,515,3\n520,0,520,-1\n0,5,-1,5" );
        grid->SetCursor( 512, 2 );
        grid->SetProperty( "SelectedRanges", "512,1,515,3\n520,0,520,-1\n0,5,-1,5" );
    }
    else
    {
        // ultrawide and low: titles for the rows, a column hidden, a column wider, rows that stand out
        grid->SetProperty( "ColumnWidths", "100,120,320,70,130,200,110,130" );
        model->titles = true;
        grid->SetBoolProperty( "ShowRowHeader", false );
        grid->SetBoolProperty( "ShowRowHeader", true );
        grid->SetColumnHidden( 3, true );
        grid->SetProperty( "GridLines", "Vertical" );
        GUI::CGridView::TInt32Vector emphasized;
        emphasized.push_back( 4 ); emphasized.push_back( 9 );
        grid->SetEmphasizedRows( emphasized );
        grid->SetIntProperty( "FrozenColumns", 1 );
        grid->SetIntProperty( "ScrollPosition", 3 );
        grid->SelectRange( 6, 2, 12, 6 );
        grid->SetCursor( 6, 2 );
        grid->SelectRange( 6, 2, 12, 6 );
    }
    context->FlushChanges();
    return grid;
}

/** Renders the sheet at a resolution, writes the picture and checks what the picture must show */
bool
RenderSheetAt( Int32 width, Int32 height, UInt32 variant, const char* name )
{
    CORE::CString font;
    GUI::TGuiContextPtr contextPtr = MakeRenderContext( width, height, font );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return false;
    CTestModel* model = GUCEF_NULL;
    GUI::CGridView* grid = MakeSheet( context, font, width, height, variant, model );
    IMAGE::TPixelMapPtr pixels;
    if ( !Capture( context, pixels ) || (Int32) pixels->GetWidthInPixels() != width || (Int32) pixels->GetHeightInPixels() != height )
        return false;
    WriteShot( pixels, name );

    const GUI::CRect& rect = grid->GetRect();
    bool ok = true;
    // the widget fills the window but for its margin, and nothing is drawn outside it
    ok = ok && rect.left == 8 && rect.top == 8 && rect.width == width - 16 && rect.height == height - 16;
    ok = ok && NothingOutside( pixels, rect, GUI::CColour( 13, 13, 13, 255 ) );

    // the header has its colour, the selection its colour, the cursor its outline
    ok = ok && Near( PixelAt( pixels, rect.left + grid->GetRowHeaderWidth() + 2, rect.top + 2 ), GUI::CColour( 50, 50, 50, 255 ), 12 );
    GUI::CRect cell;
    const Int32 firstSelected = 0 == variant ? 44 : ( 1 == variant ? 513 : 7 );
    const Int32 selectedColumn = 0 == variant ? 4 : ( 1 == variant ? 2 : 4 );
    if ( grid->GetCellRect( firstSelected + 1, selectedColumn, cell ) )
        ok = ok && Near( PixelAt( pixels, rect.left + cell.left + cell.width - 3, rect.top + cell.top + cell.height - 3 ), GUI::CColour( 60, 90, 160, 255 ), 20 );
    else
        ok = false;
    if ( grid->GetCellRect( grid->GetCursorRow(), grid->GetCursorColumn(), cell ) )
        ok = ok && Near( PixelAt( pixels, rect.left + cell.left + cell.width / 2, rect.top + cell.top ), GUI::CColour( 74, 136, 232, 255 ), 24 );
    else
        ok = false;
    return ok;
}
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridRender" );

    GUCEF_TESTFW_TESTCASE( "GridRender: A grid that uses none of the optional features draws what it drew before they existed (a hash of every drawing command of nine scenes)" )
    try
    {
        for ( UInt32 scene=0; scene<GoldenSceneCount; ++scene )
        {
            const UInt64 hash = GoldenScene( scene );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridGolden: scene " << scene << " " << HexText( hash ) );
            ASSERT_TRUE( 0 != hash );
            ASSERT_TRUE( GoldenHashes[ scene ] == hash );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridRender: A frame of a grid of 100,000 rows in the string list costs what a frame of 1,000 rows costs (the numbers are in the log)" )
    try
    {
        static const UInt32 sizes[ 3 ] = { 1000, 10000, 100000 };
        static const UInt32 frames[ 3 ] = { 40, 10, 3 };
        double steady[ 3 ] = { 0.0, 0.0, 0.0 };
        for ( UInt32 i=0; i<3; ++i )
        {
            double first = 0.0;
            MeasurePaint( sizes[ i ], frames[ i ], first, steady[ i ] );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridBenchmark: string rows " << sizes[ i ] << ": first frame after the change " << (UInt64) first << " us, steady frame " << (UInt64) steady[ i ] << " us" );
            ASSERT_TRUE( steady[ i ] > 0.0 );
        }

        // the cost of a frame follows the rows in view, not the rows there are: 100 times the rows must not be 100 times the cost (it was, before the rows were split once per change)
        ASSERT_TRUE( steady[ 2 ] < steady[ 0 ] * 4.0 + 3000.0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridRender: The sheet at 1242 x 952: columns wider than the window scroll under two frozen ones (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderSheetAt( 1242, 952, 0, "grid_1242x952" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridRender: The sheet at 3440 x 1440: an ultrawide window shows many rows and every column, with a footer and whole rows and columns selected (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderSheetAt( 3440, 1440, 1, "grid_3440x1440" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridRender: The sheet at 3840 x 1080: a wide and low window, a column hidden, titles for the rows, rows that stand out (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderSheetAt( 3840, 1080, 2, "grid_3840x1080" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridRender: A grid of the string list, as the dialogs have it, still looks as it did (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        CORE::CString font;
        GUI::TGuiContextPtr contextPtr = MakeRenderContext( 1242, 952, font );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GUI::CWidget* screen = MakeScreen( context );
        screen->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, GUI::CColour( 13, 13, 13, 255 ) );
        GUI::CGridView* grid = MakeGrid( context, screen, 8, 8, 1242 - 16, 400 );
        grid->SetProperty( "Font", font );
        grid->SetIntProperty( "RowHeight", 22 );
        grid->SetProperty( "Columns", "Name\nSize\nModified\nFlags" );
        grid->SetProperty( "ColumnWidths", "5,1,2,3" );
        grid->SetBoolProperty( "FitColumns", true );
        grid->SetProperty( "ColumnAlignments", "Left\nRight" );
        grid->SetProperty( "TextOverflow", "Ellipsis" );
        grid->SetProperty( "Sortable", "true" );
        grid->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, SheetBackground );
        grid->SetAllowMultiSelect( true );
        grid->SetRows( MakeRows( 40, 4 ) );
        GUI::CGridView::TInt32Vector selected;
        selected.push_back( 3 ); selected.push_back( 4 ); selected.push_back( 8 );
        grid->SetSelectedRows( selected );
        grid->SetSelectedRows( selected );
        GUI::CGridView::TInt32Vector emphasized;
        emphasized.push_back( 6 );
        grid->SetEmphasizedRows( emphasized );
        grid->SetIntProperty( "SortColumn", 1 );
        context->FlushChanges();
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) && 1242 == (Int32) pixels->GetWidthInPixels() && 952 == (Int32) pixels->GetHeightInPixels() );
        WriteShot( pixels, "grid_legacy_1242x952" );
        ASSERT_TRUE( NothingOutside( pixels, grid->GetRect(), GUI::CColour( 13, 13, 13, 255 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
