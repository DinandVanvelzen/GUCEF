/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#include <string.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "MyGUI_DataManager.h"

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "guidriverMyGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const GUI::CColour Black( 0, 0, 0, 255 );
const GUI::CColour Red( 255, 0, 0, 255 );
const GUI::CColour Green( 0, 255, 0, 255 );
const GUI::CColour Blue( 0, 0, 255, 255 );
const GUI::CColour White( 255, 255, 255, 255 );

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance = 4 )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance &&
           abs( (Int32) a.b - (Int32) b.b ) <= tolerance;
}

/** How many pixels of the rectangle are not black */
UInt32
CountLit( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect )
{
    UInt32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( !Near( PixelAt( pixels, x, y ), Black, 8 ) )
                ++count;
    return count;
}

/** How many pixels of the rectangle differ between two pictures */
UInt32
CountDifferent( const IMAGE::TPixelMapPtr& a, const IMAGE::TPixelMapPtr& b, const GUI::CRect& rect )
{
    UInt32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( !Near( PixelAt( a, x, y ), PixelAt( b, x, y ), 3 ) )
                ++count;
    return count;
}

GUI::CWidget*
Screen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels, UInt32 width = 640, UInt32 height = 480 )
{
    context->RenderFrame( 16 );
    bool captured = context->CaptureFrame( pixels );
    static UInt32 dumpCounter = 0;
    if ( captured && GUCEF_NULL != getenv( "MYGUI_TEST_DUMP" ) )
    {
        CORE::CString problem;
        IMAGE::CPngImageCodec::WritePixelMapToFile( *pixels, CORE::CString( "dump_" ) + CORE::UInt32ToString( dumpCounter++ ) + ".png", problem );
    }
    return captured && !pixels.IsNULL() && width == pixels->GetWidthInPixels() && height == pixels->GetHeightInPixels();
}

GUI::CWidget*
Make( GUI::CGUIContext* context, GUI::CWidget* parent, const char* type, const char* name, Int32 x, Int32 y, Int32 w, Int32 h )
{
    GUI::CWidget* widget = context->CreateWidget( type, name, parent );
    if ( GUCEF_NULL == widget )
        return GUCEF_NULL;
    widget->SetProperty( "Position", ( CORE::CString( "0," ) + CORE::Int32ToString( x ) + " 0," + CORE::Int32ToString( y ) ).C_String() );
    widget->SetProperty( "Size", ( CORE::CString( "0," ) + CORE::Int32ToString( w ) + " 0," + CORE::Int32ToString( h ) ).C_String() );
    return widget;
}

GUI::CGUIContext*
AsContext( GUI::TGuiContextPtr& pointer )
{
    return pointer.IsNULL() ? GUCEF_NULL : pointer->AsGUIContext();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMyGUIRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MyGUI OpenGL driver" );

    bool haveDriver = false;
    GUCEF_TESTFW_TESTCASE( "The driver makes a context in an offscreen window, one at a time, and reports what it draws with" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        haveDriver = GUCEF_NULL != context;
        if ( GUCEF_NULL != context )
        {
            GUI::CGUIDriver* driver = context->GetDriver();
            ASSERT_TRUE( "MyGUIOpenGL" == driver->GetDriverName() );
            ASSERT_TRUE( driver->HasCapability( "CaptureFrame" ) && driver->HasCapability( "NativeWidgets" ) );
            ASSERT_TRUE( "3.5.4" == driver->GetDriverProperty( "MyGUIVersion" ) );
            ASSERT_TRUE( "OpenGL3" == driver->GetDriverProperty( "Renderer" ) );
            ASSERT_TRUE( 1 == driver->GetContextCount() );
            ASSERT_TRUE( GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "MyGUIOpenGL" ) == driver );

            // MyGUI has one Gui per process
            GUI::TGuiContextPtr second = CreateMyGUIContext();
            ASSERT_TRUE( second.IsNULL() );
            ASSERT_TRUE( 1 == driver->GetContextCount() );

            // its skins, fonts and shaders come out of the VFS
            ASSERT_TRUE( MyGUI::DataManager::getInstance().isDataExist( "MyGUI_Core.xml" ) );
            ASSERT_TRUE( MyGUI::DataManager::getInstance().isDataExist( "MyGUI_OpenGL3_VP.glsl" ) );
            ASSERT_TRUE( GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "MyGUIOpenGL" ) != GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !haveDriver )
        return;

    GUCEF_TESTFW_TESTCASE( "A filled panel is drawn where it is laid out, with the top left of the picture at the top" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* panel = Make( context, screen, "Panel", "Panel", 10, 20, 100, 50 );
        panel->SetColourProperty( "BackgroundColour", Red );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 60, 45 ), Red, 8 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 5, 5 ), Black ) && Near( PixelAt( pixels, 120, 45 ), Black ) && Near( PixelAt( pixels, 60, 80 ), Black ) );
        // not mirrored: nothing at the bottom
        ASSERT_TRUE( Near( PixelAt( pixels, 60, 480 - 45 ), Black ) );
        ASSERT_TRUE( CountLit( pixels, GUI::CRect( 10, 20, 100, 50 ) ) > 4500 );
        ASSERT_TRUE( 0 == CountLit( pixels, GUI::CRect( 0, 100, 640, 380 ) ) );

        // moved
        panel->SetProperty( "Position", "0,300 0,200" );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 350, 225 ), Red, 8 ) && Near( PixelAt( pixels, 60, 45 ), Black ) );

        // hidden, and shown again
        panel->SetVisibility( false );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 350, 225 ), Black ) );
        panel->SetVisibility( true );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 350, 225 ), Red, 8 ) );

        // half see through over black
        panel->SetFloatProperty( "Opacity", 0.5f );
        ASSERT_TRUE( Capture( context, pixels ) );
        GUI::CColour half = PixelAt( pixels, 350, 225 );
        ASSERT_TRUE( half.r > 100 && half.r < 160 && half.g < 8 && half.b < 8 );

        // the root is hidden: everything goes
        panel->SetFloatProperty( "Opacity", 1.0f );
        context->HideRoot( screen );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 0 == CountLit( pixels, GUI::CRect( 0, 0, 640, 480 ) ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets are stacked in the order of the forms and widgets, and brought to the front" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* a = Make( context, screen, "Panel", "A", 50, 50, 100, 100 );
        a->SetColourProperty( "BackgroundColour", Red );
        GUI::CWidget* b = Make( context, screen, "Panel", "B", 100, 100, 100, 100 );
        b->SetColourProperty( "BackgroundColour", Blue );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 70, 70 ), Red, 8 ) && Near( PixelAt( pixels, 180, 180 ), Blue, 8 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 120 ), Blue, 8 ) );

        a->BringToFront();
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 120 ), Red, 8 ) );

        // a child is above its parent
        GUI::CWidget* child = Make( context, a, "Panel", "Child", 20, 20, 30, 30 );
        child->SetColourProperty( "BackgroundColour", Green );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 85, 85 ), Green, 8 ) && Near( PixelAt( pixels, 65, 65 ), Red, 8 ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Labels draw their text in the colour asked, and nothing when they have none" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* label = Make( context, screen, "Label", "Label", 20, 20, 200, 30 );
        GUI::CWidget* empty = Make( context, screen, "Label", "Empty", 20, 120, 200, 30 );
        label->SetProperty( "Text", "Hello MyGUI" );
        label->SetColourProperty( "TextColour", GUI::CColour( 255, 255, 0, 255 ) );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        UInt32 lit = CountLit( pixels, GUI::CRect( 20, 20, 200, 30 ) );
        ASSERT_TRUE( lit > 40 && lit < 3000 );
        ASSERT_TRUE( 0 == CountLit( pixels, GUI::CRect( 20, 120, 200, 30 ) ) );

        // the text is yellow: no blue in it
        UInt32 yellow = 0, other = 0;
        for ( Int32 y=20; y<50; ++y )
            for ( Int32 x=20; x<220; ++x )
            {
                GUI::CColour c = PixelAt( pixels, x, y );
                if ( c.r > 200 && c.g > 200 && c.b < 40 ) ++yellow;
                else if ( c.b > 40 ) ++other;
            }
        ASSERT_TRUE( yellow > 10 && 0 == other );

        // more text, more pixels; the text colour changes
        label->SetProperty( "Text", "Hello MyGUI, a longer line of text" );
        IMAGE::TPixelMapPtr longer;
        ASSERT_TRUE( Capture( context, longer ) );
        ASSERT_TRUE( CountLit( longer, GUI::CRect( 20, 20, 200, 30 ) ) > lit );
        label->SetColourProperty( "TextColour", GUI::CColour( 255, 0, 0, 255 ) );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( CountDifferent( pixels, longer, GUI::CRect( 20, 20, 200, 30 ) ) > 10 );

        // text is measured with the MyGUI font
        Int32 w1 = 0, h1 = 0, w2 = 0, h2 = 0, w3 = 0, h3 = 0;
        ASSERT_TRUE( context->MeasureText( "", "Hello", 1000, w1, h1 ) );
        ASSERT_TRUE( context->MeasureText( "", "Hello World", 1000, w2, h2 ) );
        ASSERT_TRUE( context->MeasureText( "", "Hello\nWorld", 1000, w3, h3 ) );
        ASSERT_TRUE( w1 > 10 && w2 > w1 && h1 > 6 && h2 == h1 && h3 > h1 );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Buttons look different hovered and pressed, check boxes when checked, from the state gucefGUI keeps" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* button = Make( context, screen, "Button", "Button", 40, 40, 120, 40 );
        button->SetProperty( "Text", "Press" );
        GUI::CWidget* check = Make( context, screen, "Checkbox", "Check", 40, 120, 150, 24 );
        check->SetProperty( "Text", "Option" );
        const GUI::CRect buttonArea( 40, 40, 120, 40 );
        const GUI::CRect checkArea( 40, 120, 150, 24 );

        IMAGE::TPixelMapPtr normal, hovered, pressed;
        ASSERT_TRUE( Capture( context, normal ) );
        ASSERT_TRUE( CountLit( normal, buttonArea ) > 3000 && CountLit( normal, checkArea ) > 20 );

        context->InjectMouseMove( 100, 60 );
        ASSERT_TRUE( Capture( context, hovered ) );
        ASSERT_TRUE( button->IsHovered() );
        ASSERT_TRUE( CountDifferent( normal, hovered, buttonArea ) > 100 );
        ASSERT_TRUE( 0 == CountDifferent( normal, hovered, checkArea ) );

        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( Capture( context, pressed ) );
        ASSERT_TRUE( button->IsPressed() );
        ASSERT_TRUE( CountDifferent( hovered, pressed, buttonArea ) > 100 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->InjectMouseMove( 500, 400 );
        IMAGE::TPixelMapPtr released;
        ASSERT_TRUE( Capture( context, released ) );
        // the button keeps the keyboard focus it got from the click, which MyGUI shows as the highlight
        ASSERT_TRUE( CountDifferent( pressed, released, buttonArea ) > 100 );

        // checked
        IMAGE::TPixelMapPtr checked;
        check->SetBoolProperty( "Checked", true );
        ASSERT_TRUE( Capture( context, checked ) );
        ASSERT_TRUE( CountDifferent( normal, checked, checkArea ) > 20 );

        // disabled
        IMAGE::TPixelMapPtr disabled;
        button->SetEnabled( false );
        ASSERT_TRUE( Capture( context, disabled ) );
        ASSERT_TRUE( CountDifferent( normal, disabled, buttonArea ) > 50 );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Progress bars, edit boxes, lists, combo boxes, sliders and windows draw what they hold" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );

        GUI::CWidget* progress = Make( context, screen, "ProgressBar", "Progress", 20, 20, 200, 20 );
        const GUI::CRect progressArea( 20, 20, 200, 20 );
        progress->SetFloatProperty( "Value", 0.0f );
        IMAGE::TPixelMapPtr zero, half, full;
        ASSERT_TRUE( Capture( context, zero ) );
        progress->SetFloatProperty( "Value", 50.0f );
        ASSERT_TRUE( Capture( context, half ) );
        progress->SetFloatProperty( "Value", 100.0f );
        ASSERT_TRUE( Capture( context, full ) );
        UInt32 d1 = CountDifferent( zero, half, progressArea );
        UInt32 d2 = CountDifferent( zero, full, progressArea );
        ASSERT_TRUE( d1 > 20 && d2 > d1 );

        GUI::CWidget* edit = Make( context, screen, "Editbox", "Edit", 20, 60, 200, 26 );
        const GUI::CRect editArea( 20, 60, 200, 26 );
        IMAGE::TPixelMapPtr emptyEdit, filledEdit;
        ASSERT_TRUE( Capture( context, emptyEdit ) );
        edit->SetProperty( "Text", "typed text" );
        ASSERT_TRUE( Capture( context, filledEdit ) );
        ASSERT_TRUE( CountDifferent( emptyEdit, filledEdit, editArea ) > 20 );

        GUI::CWidget* list = Make( context, screen, "Listbox", "List", 20, 100, 200, 100 );
        const GUI::CRect listArea( 20, 100, 200, 100 );
        IMAGE::TPixelMapPtr emptyList, filledList, selectedList;
        ASSERT_TRUE( Capture( context, emptyList ) );
        GUI::CWidget::TStringVector items;
        items.push_back( "one" ); items.push_back( "two" ); items.push_back( "three" );
        list->SetStringListProperty( "Items", items );
        ASSERT_TRUE( Capture( context, filledList ) );
        ASSERT_TRUE( CountDifferent( emptyList, filledList, listArea ) > 40 );
        list->SetIntProperty( "SelectedIndex", 1 );
        ASSERT_TRUE( Capture( context, selectedList ) );
        ASSERT_TRUE( CountDifferent( filledList, selectedList, listArea ) > 100 );

        GUI::CWidget* combo = Make( context, screen, "Combobox", "Combo", 20, 220, 200, 26 );
        const GUI::CRect comboArea( 20, 220, 200, 26 );
        IMAGE::TPixelMapPtr emptyCombo, filledCombo;
        ASSERT_TRUE( Capture( context, emptyCombo ) );
        combo->SetStringListProperty( "Items", items );
        combo->SetIntProperty( "SelectedIndex", 2 );
        ASSERT_TRUE( Capture( context, filledCombo ) );
        ASSERT_TRUE( CountDifferent( emptyCombo, filledCombo, comboArea ) > 10 );

        GUI::CWidget* slider = Make( context, screen, "Slider", "Slider", 20, 270, 200, 20 );
        const GUI::CRect sliderArea( 20, 270, 200, 20 );
        IMAGE::TPixelMapPtr low, high;
        slider->SetFloatProperty( "Value", 0.0f );
        ASSERT_TRUE( Capture( context, low ) );
        slider->SetFloatProperty( "Value", 100.0f );
        ASSERT_TRUE( Capture( context, high ) );
        ASSERT_TRUE( CountLit( low, sliderArea ) > 100 && CountDifferent( low, high, sliderArea ) > 20 );

        GUI::CWidget* window = Make( context, screen, "Window", "Window", 300, 40, 200, 150 );
        const GUI::CRect windowArea( 300, 40, 200, 150 );
        IMAGE::TPixelMapPtr untitled, titled;
        ASSERT_TRUE( Capture( context, untitled ) );
        ASSERT_TRUE( CountLit( untitled, windowArea ) > 5000 );
        window->SetProperty( "Title", "A window" );
        ASSERT_TRUE( Capture( context, titled ) );
        ASSERT_TRUE( CountDifferent( untitled, titled, windowArea ) > 20 );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A picture of gucefGUI becomes a texture of MyGUI and is drawn the right way up" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );

        // 4 x 4 pixels: red top left, green top right, blue bottom left, white bottom right
        IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( picture->GetDataPtr() );
        for ( UInt32 y=0; y<4; ++y )
            for ( UInt32 x=0; x<4; ++x )
            {
                const GUI::CColour& c = y < 2 ? ( x < 2 ? Red : Green ) : ( x < 2 ? Blue : White );
                UInt8* p = data + ( y * 4 + x ) * 4;
                p[ 0 ] = c.r; p[ 1 ] = c.g; p[ 2 ] = c.b; p[ 3 ] = 255;
            }
        ASSERT_TRUE( context->RegisterImage( "quad", picture ) );

        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* frame = Make( context, screen, "ImageFrame", "Frame", 100, 100, 80, 80 );
        frame->SetProperty( "Image", "quad" );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 120 ), Red, 12 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 160, 120 ), Green, 12 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 160 ), Blue, 12 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 160, 160 ), White, 12 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 90, 90 ), Black ) );

        // changed pixels: the texture follows
        UInt8* p = static_cast< UInt8* >( picture->GetDataPtr() );
        for ( UInt32 i=0; i<16; ++i ) { p[ i*4 ] = 0; p[ i*4+1 ] = 0; p[ i*4+2 ] = 255; }
        ASSERT_TRUE( context->UpdateImage( "quad" ) );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 120 ), Blue, 12 ) && Near( PixelAt( pixels, 160, 160 ), Blue, 12 ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A TrueType font of the VFS is loaded through MyGUI and used by name" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CGUIDriver* driver = context->GetDriver();
        ASSERT_TRUE( driver->LoadFontFromAsset( "MyGUI.Engine/DejaVuSans.ttf" ) );
        ASSERT_TRUE( driver->GetAvailableFonts().count( "DejaVuSans" ) > 0 );
        ASSERT_FALSE( driver->LoadFontFromAsset( "MyGUI.Engine/no_such_font.ttf" ) );

        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* plain = Make( context, screen, "Label", "Plain", 20, 20, 300, 40 );
        GUI::CWidget* custom = Make( context, screen, "Label", "Custom", 20, 80, 300, 40 );
        GUI::CWidget* unknown = Make( context, screen, "Label", "Unknown", 20, 140, 300, 40 );
        plain->SetProperty( "Text", "Font test" );
        custom->SetProperty( "Text", "Font test" );
        unknown->SetProperty( "Text", "Font test" );
        custom->SetProperty( "Font", "DejaVuSans" );
        unknown->SetProperty( "Font", "NoSuchFont" );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( CountLit( pixels, GUI::CRect( 20, 20, 300, 40 ) ) > 20 );
        ASSERT_TRUE( CountLit( pixels, GUI::CRect( 20, 80, 300, 40 ) ) > 20 );
        // a font that does not exist is the default font
        ASSERT_TRUE( CountLit( pixels, GUI::CRect( 20, 140, 300, 40 ) ) == CountLit( pixels, GUI::CRect( 20, 20, 300, 40 ) ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "At a render scale of 2 the picture is twice as large and the widgets are placed at the scale" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* panel = Make( context, screen, "Panel", "Panel", 10, 20, 100, 50 );
        panel->SetColourProperty( "BackgroundColour", Red );
        context->SetRenderScale( 2 );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels, 1280, 960 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 90 ), Red, 8 ) && Near( PixelAt( pixels, 215, 135 ), Red, 8 ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 225, 135 ), Black ) && Near( PixelAt( pixels, 10, 90 ), Black ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The picture is shown in a window of another size the way the present mode says" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContextInWindow( 800, 600 );
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* panel = Make( context, screen, "Panel", "Panel", 0, 0, 640, 480 );
        panel->SetColourProperty( "BackgroundColour", Blue );
        context->RenderFrame( 16 );
        IMAGE::TPixelMapPtr presented;
        ASSERT_TRUE( context->CapturePresentedFrame( presented ) );
        ASSERT_TRUE( !presented.IsNULL() && 800 == presented->GetWidthInPixels() && 600 == presented->GetHeightInPixels() );
        if ( !presented.IsNULL() && 800 == presented->GetWidthInPixels() )
        {
            GUI::CRect area = context->GetPresentRect();
            ASSERT_TRUE( area.width > 0 && area.height > 0 );
            ASSERT_TRUE( Near( PixelAt( presented, area.left + area.width / 2, area.top + area.height / 2 ), Blue, 8 ) );
            // black outside of the picture, when the picture does not fill the window
            if ( area.left > 4 )
                ASSERT_TRUE( Near( PixelAt( presented, 1, 300 ), Black ) );
            if ( area.top > 4 )
                ASSERT_TRUE( Near( PixelAt( presented, 400, 1 ), Black ) );
        }
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A context that goes lets the driver make a new one, which draws again" )
    try
    {
        GUI::CGUIDriver* driver = GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "MyGUIOpenGL" );
        ASSERT_TRUE( GUCEF_NULL != driver );
        ASSERT_TRUE( GUCEF_NULL != driver && 0 == driver->GetContextCount() );

        for ( UInt32 round=0; round<2; ++round )
        {
            GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
            GUI::CGUIContext* context = AsContext( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                break;
            GUI::CWidget* screen = Screen( context );
            GUI::CWidget* panel = Make( context, screen, "Panel", "Panel", 10, 20, 100, 50 );
            panel->SetColourProperty( "BackgroundColour", Green );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( context, pixels ) );
            ASSERT_TRUE( Near( PixelAt( pixels, 60, 45 ), Green, 8 ) );
            // the widgets are left for the context to clean up
        }
        ASSERT_TRUE( GUCEF_NULL != driver && 0 == driver->GetContextCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
