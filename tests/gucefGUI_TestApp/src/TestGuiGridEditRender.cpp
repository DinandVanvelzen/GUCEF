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

#include <stdlib.h>

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

const GUI::CColour SheetBackground( 24, 24, 28, 255 );

bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance && abs( (Int32) a.b - (Int32) b.b ) <= tolerance;
}

/** Whether a pixel in the run of the row is near the colour */
bool
RunHasColour( const IMAGE::TPixelMapPtr& pixels, Int32 x0, Int32 x1, Int32 y, const GUI::CColour& colour, Int32 tolerance )
{
    for ( Int32 x=x0; x<x1; ++x )
        if ( Near( PixelAt( pixels, x, y ), colour, tolerance ) )
            return true;
    return false;
}

/** A context of the given size with the bitmap font "Test" and, when the font plugin is there, the UI font "UI" */
GUI::TGuiContextPtr
MakeEditRenderContext( Int32 width, Int32 height, CORE::CString& font )
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

/**
 *  An editable sheet filling the window, with an editor open: state 0 a text editor with a range outlined as copied, 1 a number
 *  editor with a value the model refuses, 2 a list of choices. Writes the picture and checks what it must show.
 */
bool
RenderEditAt( Int32 width, Int32 height, UInt32 state, const char* name )
{
    CORE::CString font;
    GUI::TGuiContextPtr contextPtr = MakeEditRenderContext( width, height, font );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return false;
    CFakeClipboard clipboard;
    GUI::CWidget* screen = MakeScreen( context );
    screen->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, GUI::CColour( 13, 13, 13, 255 ) );
    GUI::CGridView* grid = MakeGrid( context, screen, 8, 8, width - 16, height - 16 );
    grid->SetProperty( "Font", font );
    grid->SetIntProperty( "RowHeight", 24 );
    grid->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, SheetBackground );
    CEditableTestModel* model = GUCEF_NEW CEditableTestModel( 2000 );
    GUI::TTableViewModelPtr modelPtr( model );
    grid->SetModel( modelPtr );
    grid->SetProperty( "SelectionMode", "Cell" );
    grid->SetBoolProperty( "ShowRowHeader", true );
    grid->SetBoolProperty( "CellEditing", true );
    grid->SetBoolProperty( "CellClipboard", true );
    grid->SetProperty( "ZebraColour", "30,32,40,255" );
    grid->SetProperty( "GridLines", "Horizontal" );
    grid->SetProperty( "TextOverflow", "Ellipsis" );
    grid->SetProperty( "ColumnWidths", "90,120,220,70,130,200,110,130" );
    grid->SetBoolProperty( "FitColumns", true );
    grid->SetIntProperty( "ScrollPosition", 40 );
    model->EditColumn( CTestModel::COL_NAME ).choices.push_back( "alpha" );
    model->EditColumn( CTestModel::COL_NAME ).choices.push_back( "beta" );
    model->EditColumn( CTestModel::COL_NAME ).choices.push_back( "gamma" );
    model->EditColumn( CTestModel::COL_NAME ).choices.push_back( "delta" );
    context->FlushChanges();
    context->SetFocus( grid );
    if ( 2 != state )
        model->EditColumn( CTestModel::COL_NAME ).choices.clear();

    // the cells that were copied, and the cell being edited
    grid->SelectRange( 44, CTestModel::COL_PRICE, 48, CTestModel::COL_NAME );
    if ( !grid->CopySelection( false ) )
        return false;
    Int32 editRow = 52, editColumn = CTestModel::COL_NAME;
    if ( 0 == state )
    {
        grid->SetCursor( editRow, editColumn );
        if ( !grid->BeginEdit( editRow, editColumn ) )
            return false;
        context->InjectText( " (edited)" );
    }
    else
    if ( 1 == state )
    {
        editColumn = CTestModel::COL_PRICE;
        grid->SetCursor( editRow, editColumn );
        context->InjectText( "-12.5" );
    }
    else
    {
        grid->SetCursor( editRow, editColumn );
        if ( !grid->BeginEdit( editRow, editColumn ) )
            return false;
        PressKey( context, INPUT::KEYCODE_DOWN );
        PressKey( context, INPUT::KEYCODE_DOWN );
    }
    context->FlushChanges();
    if ( !grid->IsEditing() )
        return false;

    IMAGE::TPixelMapPtr pixels;
    if ( !Capture( context, pixels ) || (Int32) pixels->GetWidthInPixels() != width || (Int32) pixels->GetHeightInPixels() != height )
        return false;
    WriteShot( pixels, name );

    GUI::CGridEditController::SEditorLayout layout;
    if ( !grid->GetEditController()->GetEditorLayout( layout ) )
        return false;
    const GUI::CRect& rect = grid->GetRect();
    const Int32 left = rect.left + layout.cell.left;
    const Int32 top = rect.top + layout.cell.top;
    bool ok = true;

    // the copied cells: a dashed outline in its colour along the top of the first row of them
    GUI::CRect copied;
    ok = ok && grid->GetCellRect( 44, CTestModel::COL_PRICE, copied );
    ok = ok && RunHasColour( pixels, rect.left + copied.left, rect.left + copied.left + 4 * ( copied.width / 2 ), rect.top + copied.top, GUI::CColour( 200, 200, 90, 255 ), 30 );

    if ( 0 == state )
    {
        // the editor: the colour of the accent on its border, the field inside, the caret in the text colour
        ok = ok && Near( PixelAt( pixels, left + layout.cell.width / 2, top ), GUI::CColour( 74, 136, 232, 255 ), 24 );
        ok = ok && Near( PixelAt( pixels, left + layout.cell.width - 6, top + layout.cell.height / 2 ), SheetBackground, 6 );
        ok = ok && RunHasColour( pixels, rect.left + layout.caretX - 1, rect.left + layout.caretX + 2, top + layout.cell.height / 2, GUI::CColour( 255, 255, 255, 255 ), 30 );
        ok = ok && !layout.hasMessage && !layout.hasList;
    }
    else
    if ( 1 == state )
    {
        // refused: a red outline and the reason under it
        ok = ok && layout.hasMessage && Near( PixelAt( pixels, left + layout.cell.width / 2, top ), GUI::CColour( 224, 64, 64, 255 ), 24 );
        ok = ok && Near( PixelAt( pixels, rect.left + layout.message.left + layout.message.width - 3, rect.top + layout.message.top + layout.message.height / 2 ), GUI::CColour( 86, 22, 26, 255 ), 12 );
        ok = ok && "The price must not be negative" == grid->GetCellEditor()->GetError();
    }
    else
    {
        // the list: a field under the cell, with the choice that is picked tinted
        ok = ok && layout.hasList && 4 == layout.itemCount;
        const Int32 listLeft = rect.left + layout.list.left;
        const Int32 listTop = rect.top + layout.list.top;
        ok = ok && Near( PixelAt( pixels, listLeft + layout.list.width - 4, listTop + 3 * layout.itemHeight + layout.itemHeight / 2 ), GUI::CColour( 36, 38, 44, 255 ), 6 );
        ok = ok && !Near( PixelAt( pixels, listLeft + layout.list.width - 4, listTop + 1 * layout.itemHeight + layout.itemHeight / 2 ), GUI::CColour( 36, 38, 44, 255 ), 20 );
    }
    return ok;
}

/** A sheet with check boxes, in a window: the boxes of a column that is edited are drawn and flip with a click */
bool
RenderCheckBoxesAt( Int32 width, Int32 height, const char* name )
{
    CORE::CString font;
    GUI::TGuiContextPtr contextPtr = MakeEditRenderContext( width, height, font );
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return false;
    GUI::CWidget* screen = MakeScreen( context );
    GUI::CGridView* grid = MakeGrid( context, screen, 8, 8, width - 16, height - 16 );
    grid->SetProperty( "Font", font );
    grid->SetIntProperty( "RowHeight", 24 );
    grid->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, SheetBackground );
    CEditableTestModel* model = GUCEF_NEW CEditableTestModel( 500 );
    GUI::TTableViewModelPtr modelPtr( model );
    grid->SetModel( modelPtr );
    grid->SetProperty( "SelectionMode", "Cell" );
    grid->SetBoolProperty( "CellEditing", true );
    context->FlushChanges();
    context->SetFocus( grid );
    IMAGE::TPixelMapPtr before, after;
    if ( !Capture( context, before ) )
        return false;
    const bool wasOn = model->Current( 5, CTestModel::COL_FLAG ).AsBool();
    if ( !ClickCell( context, grid, 5, CTestModel::COL_FLAG ) || wasOn == model->Current( 5, CTestModel::COL_FLAG ).AsBool() )
        return false;
    grid->SetCursor( 9, CTestModel::COL_NAME );
    context->FlushChanges();
    if ( !Capture( context, after ) )
        return false;
    WriteShot( after, name );

    // the box of the cell changed: a tick is there or gone
    GUI::CRect cell;
    if ( !grid->GetCellRect( 5, CTestModel::COL_FLAG, cell ) )
        return false;
    const Int32 x = grid->GetRect().left + cell.left;
    const Int32 y = grid->GetRect().top + cell.top;
    bool differs = false;
    for ( Int32 dy=0; dy<cell.height && !differs; ++dy )
        for ( Int32 dx=0; dx<cell.width && !differs; ++dx )
            if ( !Near( PixelAt( before, x + dx, y + dy ), PixelAt( after, x + dx, y + dy ), 0 ) )
                differs = true;
    return differs;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridEditRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridEditRender" );

    GUCEF_TESTFW_TESTCASE( "GridEditRender: A text editor open in the sheet at 1242 x 952, the copied cells outlined (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderEditAt( 1242, 952, 0, "gridedit_text_1242x952" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditRender: A number editor with a value the model refuses at 1242 x 952: a red outline and the reason (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderEditAt( 1242, 952, 1, "gridedit_invalid_1242x952" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditRender: A list of choices open at 1242 x 952 (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderEditAt( 1242, 952, 2, "gridedit_choice_1242x952" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditRender: The three editors at 3440 x 1440, an ultrawide window (pictures with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderEditAt( 3440, 1440, 0, "gridedit_text_3440x1440" ) );
        ASSERT_TRUE( RenderEditAt( 3440, 1440, 1, "gridedit_invalid_3440x1440" ) );
        ASSERT_TRUE( RenderEditAt( 3440, 1440, 2, "gridedit_choice_3440x1440" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditRender: The three editors at 3840 x 1080, a wide and low window (pictures with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderEditAt( 3840, 1080, 0, "gridedit_text_3840x1080" ) );
        ASSERT_TRUE( RenderEditAt( 3840, 1080, 1, "gridedit_invalid_3840x1080" ) );
        ASSERT_TRUE( RenderEditAt( 3840, 1080, 2, "gridedit_choice_3840x1080" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditRender: The check boxes of an editable column are drawn and a click on one flips it (pictures with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        ASSERT_TRUE( RenderCheckBoxesAt( 1242, 952, "gridedit_checkbox_1242x952" ) );
        ASSERT_TRUE( RenderCheckBoxesAt( 3840, 1080, "gridedit_checkbox_3840x1080" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
