/*
 *  guidriverImGui_TestApp: tests of the gucefGUI driver library of Dear ImGui
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

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "guidriverImGui_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;

namespace {

/** What a frame builder read of the input state of Dear ImGui */
struct SInputRecord
{
    GUCEF::vector< bool > keysDown;      /**< per mapped key code, in the order of the table */
    bool mouseDown[ 5 ];
    Float32 wheel;
    bool ctrl;
    bool shift;
    bool alt;
    bool super;
    ImGuiKeyChord keyMods;
    GUCEF::vector< UInt32 > characters;
    bool keyADown;
    ImVec2 mousePos;

    SInputRecord( void )
        : keysDown(), wheel( 0.0f ), ctrl( false ), shift( false ), alt( false ), super( false ), keyMods( 0 ), characters(),
          keyADown( false ), mousePos( 0.0f, 0.0f )
    {
        for ( UInt32 i=0; i<5; ++i )
            mouseDown[ i ] = false;
    }
};

void
RecordInput( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SInputRecord* record = static_cast< SInputRecord* >( userData );
    ImGuiIO& io = ImGui::GetIO();
    record->keysDown.clear();
    for ( UInt32 i=0; i<CImGuiInputMapper::GetMappedKeyCount(); ++i )
        record->keysDown.push_back( ImGui::IsKeyDown( CImGuiInputMapper::ToImGuiKey( CImGuiInputMapper::GetMappedKeyCode( i ) ) ) );
    for ( int b=0; b<5; ++b )
        record->mouseDown[ b ] = ImGui::IsMouseDown( b );
    record->wheel = io.MouseWheel;
    record->ctrl = io.KeyCtrl;
    record->shift = io.KeyShift;
    record->alt = io.KeyAlt;
    record->super = io.KeySuper;
    record->keyMods = io.KeyMods;
    record->characters.clear();
    for ( int c=0; c<io.InputQueueCharacters.Size; ++c )
        record->characters.push_back( (UInt32) io.InputQueueCharacters[ c ] );
    record->keyADown = ImGui::IsKeyDown( ImGuiKey_A );
    record->mousePos = io.MousePos;
}

/** A tools window over a part of the screen, optionally with a text field that takes the keyboard focus */
struct SToolsWindow
{
    bool withTextField;
    bool focusTextField;
    char text[ 64 ];
    ImGuiMouseCursor cursor;
    bool textFieldActive;

    SToolsWindow( void )
        : withTextField( false ), focusTextField( false ), cursor( ImGuiMouseCursor_COUNT ), textFieldActive( false )
    {
        memset( text, 0, sizeof( text ) );
    }
};

void
BuildToolsWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SToolsWindow* tools = static_cast< SToolsWindow* >( userData );
    ImGui::SetNextWindowPos( ImVec2( 100.0f, 100.0f ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( 200.0f, 150.0f ), ImGuiCond_Always );
    ImGui::Begin( "Tools", GUCEF_NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );
    if ( tools->withTextField )
    {
        if ( tools->focusTextField )
        {
            ImGui::SetKeyboardFocusHere();
            tools->focusTextField = false;
        }
        ImGui::InputText( "##text", tools->text, sizeof( tools->text ) );
        tools->textFieldActive = ImGui::IsItemActive();
    }
    ImGui::End();
    if ( ImGuiMouseCursor_COUNT != tools->cursor )
        ImGui::SetMouseCursor( tools->cursor );
}

/** A retained screen with one button that covers it */
GUI::CWidget*
MakeButtonScreen( GUI::CGUIContext& context )
{
    GUI::CWidget* screen = context.CreateWidget( "Panel", "Screen", GUCEF_NULL );
    screen->SetProperty( "Size", "1,0 1,0" );
    GUI::CWidget* button = context.CreateWidget( "Button", "Button", screen );
    button->SetProperty( "Position", "0,0 0,0" );
    button->SetProperty( "Size", "1,0 1,0" );
    button->SetProperty( "Text", "Below" );
    context.ShowRoot( screen, false );
    return button;
}

const char* g_clipboardText = "from the clipboard";
CString g_clipboardWritten;

bool
TestClipboardReader( char* dest, UInt32 size, UInt32* written )
{
    UInt32 length = (UInt32) strlen( g_clipboardText );
    if ( length + 1 > size )
        length = size - 1;
    memcpy( dest, g_clipboardText, length );
    dest[ length ] = '\0';
    *written = length;
    return true;
}

bool
TestClipboardWriter( const char* text )
{
    g_clipboardWritten = text;
    return true;
}

struct SClipboardUse
{
    CString read;
    bool done;
};

void
UseClipboard( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SClipboardUse* use = static_cast< SClipboardUse* >( userData );
    if ( use->done )
        return;
    const char* text = ImGui::GetClipboardText();
    use->read = GUCEF_NULL != text ? text : "";
    ImGui::SetClipboardText( "to the clipboard" );
    use->done = true;
}

struct SIniWindow
{
    bool place;
    ImVec2 position;
};

void
BuildIniWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SIniWindow* window = static_cast< SIniWindow* >( userData );
    if ( window->place )
        ImGui::SetNextWindowPos( ImVec2( 33.0f, 44.0f ), ImGuiCond_Always );
    ImGui::Begin( "Remembered" );
    window->position = ImGui::GetWindowPos();
    ImGui::End();
}

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiInputTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Input" );

    GUCEF_TESTFW_TESTCASE( "Input: the key table maps every key code it has to its own key of Dear ImGui and back" )
    try
    {
        const UInt32 count = CImGuiInputMapper::GetMappedKeyCount();
        ASSERT_TRUE( count >= 100 );
        GUCEF::set< Int32 > keys;
        GUCEF::set< Int32 > keyCodes;
        for ( UInt32 i=0; i<count; ++i )
        {
            INPUT::KeyCode keyCode = CImGuiInputMapper::GetMappedKeyCode( i );
            ImGuiKey key = CImGuiInputMapper::ToImGuiKey( keyCode );
            ASSERT_TRUE( ImGuiKey_None != key );
            ASSERT_TRUE( keyCode == CImGuiInputMapper::ToKeyCode( key ) );
            keys.insert( (Int32) key );
            keyCodes.insert( (Int32) keyCode );
        }
        ASSERT_TRUE( count == (UInt32) keys.size() );
        ASSERT_TRUE( count == (UInt32) keyCodes.size() );
        ASSERT_TRUE( ImGuiKey_A == CImGuiInputMapper::ToImGuiKey( INPUT::KEYCODE_A ) );
        ASSERT_TRUE( ImGuiKey_KeypadEnter == CImGuiInputMapper::ToImGuiKey( INPUT::KEYCODE_NUMPADENTER ) );
        ASSERT_TRUE( ImGuiKey_LeftSuper == CImGuiInputMapper::ToImGuiKey( INPUT::KEYCODE_LWIN ) );
        ASSERT_TRUE( ImGuiKey_None == CImGuiInputMapper::ToImGuiKey( INPUT::KEYCODE_MUTE ) );
        ASSERT_TRUE( ImGuiKey_None == CImGuiInputMapper::ToImGuiKey( INPUT::KEYCODE_MAX ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: every mapped key goes down and up in Dear ImGui, modifiers go with the key events" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SInputRecord record;
            CTestFrameBuilder builder( &RecordInput, &record );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );

            const UInt32 count = CImGuiInputMapper::GetMappedKeyCount();
            for ( UInt32 i=0; i<count; ++i )
                context->InjectKey( CImGuiInputMapper::GetMappedKeyCode( i ), true, 0 );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            UInt32 down = 0;
            for ( UInt32 i=0; i<(UInt32)record.keysDown.size(); ++i )
                if ( record.keysDown[ i ] )
                    ++down;
            ASSERT_TRUE( count == down );
            for ( UInt32 i=0; i<count; ++i )
                context->InjectKey( CImGuiInputMapper::GetMappedKeyCode( i ), false, 0 );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            down = 0;
            for ( UInt32 i=0; i<(UInt32)record.keysDown.size(); ++i )
                if ( record.keysDown[ i ] )
                    ++down;
            ASSERT_TRUE( 0 == down );

            // Ctrl and Shift held with a key
            context->InjectKey( INPUT::KEYCODE_A, true, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT );
            context->RenderFrame( 16 );
            ASSERT_TRUE( record.ctrl && record.shift && !record.alt && !record.super );
            ASSERT_TRUE( ( ImGuiMod_Ctrl | ImGuiMod_Shift ) == record.keyMods );
            ASSERT_TRUE( record.keyADown );
            context->InjectKey( INPUT::KEYCODE_A, false, INPUT::KEYMOD_ALT | INPUT::KEYMOD_META );
            context->RenderFrame( 16 );
            ASSERT_TRUE( !record.ctrl && !record.shift && record.alt && record.super );
            ASSERT_FALSE( record.keyADown );
            ASSERT_TRUE( ( ImGuiMod_Alt | ImGuiMod_Super ) == CImGuiInputMapper::ToImGuiModifiers( INPUT::KEYMOD_ALT | INPUT::KEYMOD_META ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: mouse buttons 1 to 5 are buttons 0 to 4 of Dear ImGui, the wheel keeps its sign, text keeps code points above U+FFFF" )
    try
    {
        ASSERT_TRUE( -1 == CImGuiInputMapper::ToImGuiMouseButton( GUI::MOUSEBUTTON_NONE ) );
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SInputRecord record;
            CTestFrameBuilder builder( &RecordInput, &record );
            builder.Attach( contextPtr );
            context->InjectMouseMove( 600, 20 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 600.0f == record.mousePos.x && 20.0f == record.mousePos.y );

            const GUI::TMouseButton buttons[ 5 ] = { GUI::MOUSEBUTTON_LEFT, GUI::MOUSEBUTTON_RIGHT, GUI::MOUSEBUTTON_MIDDLE, GUI::MOUSEBUTTON_X1, GUI::MOUSEBUTTON_X2 };
            for ( UInt32 b=0; b<5; ++b )
            {
                ASSERT_TRUE( (Int32) b == CImGuiInputMapper::ToImGuiMouseButton( buttons[ b ] ) );
                context->InjectMouseButton( buttons[ b ], true );
                context->RenderFrame( 16 );
                ASSERT_TRUE( record.mouseDown[ b ] );
                context->InjectMouseButton( buttons[ b ], false );
                context->RenderFrame( 16 );
                ASSERT_FALSE( record.mouseDown[ b ] );
            }

            context->InjectMouseWheel( 2 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 2.0f == record.wheel );
            context->InjectMouseWheel( -1 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( -1.0f == record.wheel );

            // "a", U+00E9 and U+1F600 in UTF-8
            context->InjectText( "a\xC3\xA9\xF0\x9F\x98\x80" );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 3 == record.characters.size() );
            if ( 3 == record.characters.size() )
            {
                ASSERT_TRUE( 'a' == record.characters[ 0 ] );
                ASSERT_TRUE( 0xE9 == record.characters[ 1 ] );
                ASSERT_TRUE( 0x1F600 == record.characters[ 2 ] );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: when the window loses the focus Dear ImGui lets go of the keys held" )
    try
    {
        CRecordingWindow* window = GUCEF_NEW CRecordingWindow();
        GUI::TWindowContextPtr windowPtr( window );
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext( windowPtr );
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SInputRecord record;
            CTestFrameBuilder builder( &RecordInput, &record );
            builder.Attach( contextPtr );
            context->InjectKey( INPUT::KEYCODE_A, true, 0 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( record.keyADown );
            window->SetActive( false );
            context->RenderFrame( 16 );
            ASSERT_FALSE( record.keyADown );
            window->SetActive( true );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: a click on an ImGui window does not reach the retained button below it, a click beside the window does" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* button = MakeButtonScreen( *context );
            CEventCounter counter;
            button->Subscribe( &counter );
            SToolsWindow tools;
            CTestFrameBuilder builder( &BuildToolsWindow, &tools );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );

            // over the ImGui window: Dear ImGui wants the mouse
            context->InjectMouseMove( 150, 150 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( context->IsTakingPointerInput() );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 0 == counter.Count( GUI::CWidget::MouseDownEvent ) );
            ASSERT_TRUE( 0 == counter.Count( GUI::CWidget::MouseClickEvent ) );

            // beside it: the button gets the click. Dear ImGui trickles a press and a release of one frame over two frames
            // (io.ConfigInputTrickleEventQueue), so the release above is seen one frame later
            context->InjectMouseMove( 500, 400 );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_FALSE( context->GetImGuiContext()->IO.WantCaptureMouse );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWidget::MouseDownEvent ) );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWidget::MouseClickEvent ) );
            button->Unsubscribe( &counter );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: a press of a retained widget that is released over an ImGui window still ends" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* button = MakeButtonScreen( *context );
            CEventCounter counter;
            button->Subscribe( &counter );
            SToolsWindow tools;
            CTestFrameBuilder builder( &BuildToolsWindow, &tools );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->InjectMouseMove( 500, 400 );
            context->RenderFrame( 16 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            ASSERT_TRUE( button->IsPressed() );
            context->InjectMouseMove( 150, 150 );
            context->RenderFrame( 16 );
            context->InjectMouseMove( 160, 160 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 16 );
            ASSERT_FALSE( button->IsPressed() );
            ASSERT_TRUE( 1 == counter.Count( GUI::CWidget::MouseUpEvent ) );
            button->Unsubscribe( &counter );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: typing into an ImGui text field does not reach the retained edit box that has the focus, the text input data is recorded" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
            screen->SetProperty( "Size", "1,0 1,0" );
            GUI::CWidget* edit = context->CreateWidget( "Editbox", "Edit", screen );
            edit->SetProperty( "Position", "0,400 0,400" );
            edit->SetProperty( "Size", "0,200 0,30" );
            context->ShowRoot( screen, false );
            context->RenderFrame( 16 );
            ASSERT_TRUE( context->SetFocus( edit ) );
            context->InjectText( "x" );
            ASSERT_TRUE( "x" == edit->GetProperty( "Text" ) );

            SToolsWindow tools;
            tools.withTextField = true;
            tools.focusTextField = true;
            CTestFrameBuilder builder( &BuildToolsWindow, &tools );
            builder.Attach( contextPtr );
            const UInt32 imeUpdates = context->GetTextInputState().updates;
            for ( UInt32 i=0; i<3; ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( tools.textFieldActive );
            ASSERT_TRUE( context->IsTakingKeyboardInput() );
            ASSERT_TRUE( context->GetTextInputState().wantTextInput );
            ASSERT_TRUE( context->GetTextInputState().updates > imeUpdates );

            context->InjectText( "abc" );
            context->RenderFrame( 16 );
            ASSERT_TRUE( "x" == edit->GetProperty( "Text" ) );
            ASSERT_TRUE( 0 == strcmp( "abc", tools.text ) );

            // a layer that passes the keyboard on takes none
            context->SetKeyboardInputPassThrough( true );
            ASSERT_TRUE( context->IsTakingPointerInput() );
            ASSERT_FALSE( context->IsTakingKeyboardInput() );
            context->SetKeyboardInputPassThrough( false );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: Dear ImGui reads and writes the clipboard of gucefGUI" )
    try
    {
        GUI::CSecretInput::SetClipboardReader( &TestClipboardReader );
        GUI::CSecretInput::SetClipboardWriter( &TestClipboardWriter );
        g_clipboardWritten.Clear();
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            SClipboardUse use;
            use.done = false;
            CTestFrameBuilder builder( &UseClipboard, &use );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            ASSERT_TRUE( use.done );
            ASSERT_TRUE( "from the clipboard" == use.read );
            ASSERT_TRUE( "to the clipboard" == g_clipboardWritten );
        }
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: the cursor Dear ImGui asks for becomes the shape of the pointer of the window while the layer takes the pointer" )
    try
    {
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_Arrow ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_None ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_TextInput ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_ALL == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_ResizeAll ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_NS == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_ResizeNS ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_EW == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_ResizeEW ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_NESW == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_ResizeNESW ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_NWSE == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_ResizeNWSE ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_HAND == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_Hand ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_WAIT == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_Wait ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_Progress ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_NOT_ALLOWED == CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor_NotAllowed ) );

        CRecordingWindow* window = GUCEF_NEW CRecordingWindow();
        GUI::TWindowContextPtr windowPtr( window );
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext( windowPtr );
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( 0 != ( context->GetImGuiContext()->IO.BackendFlags & ImGuiBackendFlags_HasMouseCursors ) );
            SToolsWindow tools;
            tools.cursor = ImGuiMouseCursor_Hand;
            CTestFrameBuilder builder( &BuildToolsWindow, &tools );
            builder.Attach( contextPtr );
            context->RenderWindowLayers( 16 );
            ASSERT_TRUE( GUI::CURSORSHAPE_HAND == context->GetSystemCursorShape() );
            context->RenderWindowLayers( 16 );
            ASSERT_TRUE( GUI::CURSORSHAPE_HAND == window->lastShape );
            const UInt32 calls = window->shapeCalls;
            context->RenderWindowLayers( 16 );
            ASSERT_TRUE( calls == window->shapeCalls );
            tools.cursor = ImGuiMouseCursor_ResizeEW;
            context->RenderWindowLayers( 16 );
            context->RenderWindowLayers( 16 );
            ASSERT_TRUE( GUI::CURSORSHAPE_RESIZE_EW == window->lastShape );
        }
        contextPtr.Unlink();

        // a window without shapes: Dear ImGui is told there are none
        CRecordingWindow* plain = GUCEF_NEW CRecordingWindow();
        plain->supportsShapes = false;
        GUI::TWindowContextPtr plainPtr( plain );
        GUI::TGuiContextPtr plainContextPtr = CreateImGuiContext( plainPtr );
        CImGuiContext* plainContext = AsImGui( plainContextPtr );
        ASSERT_TRUE( GUCEF_NULL != plainContext );
        if ( GUCEF_NULL != plainContext )
            ASSERT_TRUE( 0 == ( plainContext->GetImGuiContext()->IO.BackendFlags & ImGuiBackendFlags_HasMouseCursors ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: the settings of Dear ImGui go out and come back in through the API" )
    try
    {
        CString settings;
        {
            GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
            CImGuiContext* context = AsImGui( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL != context )
            {
                SIniWindow window;
                window.place = true;
                CTestFrameBuilder builder( &BuildIniWindow, &window );
                builder.Attach( contextPtr );
                context->RenderFrame( 16 );
                context->RenderFrame( 16 );
                ASSERT_TRUE( context->GetIniSettings( settings ) );
                ASSERT_TRUE( settings.HasSubstr( "[Window][Remembered]" ) >= 0 );
                ASSERT_TRUE( settings.HasSubstr( "Pos=33,44" ) >= 0 );
            }
        }
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( context->SetIniSettings( settings ) );
            SIniWindow window;
            window.place = false;
            CTestFrameBuilder builder( &BuildIniWindow, &window );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 33.0f == window.position.x && 44.0f == window.position.y );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
