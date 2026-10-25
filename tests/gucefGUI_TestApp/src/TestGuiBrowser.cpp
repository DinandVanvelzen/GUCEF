/*
 *  gucefGUI_TestApp: GUCEF test application for the GUI module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
#include <emscripten.h>
#include "guidriverRmlUIOpenGL_pluginAPI.h"
#include "guidriverEmscripten_pluginAPI.h"
#include "inputdriverEMSCRIPTEN_pluginAPI.h"
#include "inputdriverEMSCRIPTEN_CEmscriptenInputDriver.h"
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* What the user does, as the browser reports it: events of the page, positions in CSS pixels of the canvas */

EM_JS( void, TestBrowser_Mouse, ( const char* type, int x, int y, int button ), {
    var canvas = document.querySelector( "#canvas" );
    var rect = canvas.getBoundingClientRect();
    var name = UTF8ToString( type );
    var target = "mouseup" == name ? window : canvas;
    target.dispatchEvent( new MouseEvent( name, { clientX: rect.left + x, clientY: rect.top + y, button: button, bubbles: true, cancelable: true } ) );
} );

EM_JS( void, TestBrowser_Wheel, ( double deltaY, int deltaMode ), {
    var canvas = document.querySelector( "#canvas" );
    var rect = canvas.getBoundingClientRect();
    canvas.dispatchEvent( new WheelEvent( "wheel", { clientX: rect.left + 1, clientY: rect.top + 1, deltaY: deltaY, deltaMode: deltaMode, bubbles: true, cancelable: true } ) );
} );

EM_JS( int, TestBrowser_Key, ( const char* type, const char* code, const char* key, int ctrl ), {
    var event = new KeyboardEvent( UTF8ToString( type ), { code: UTF8ToString( code ), key: UTF8ToString( key ), ctrlKey: 0 != ctrl, bubbles: true, cancelable: true } );
    window.dispatchEvent( event );
    return event.defaultPrevented ? 1 : 0;
} );

EM_JS( void, TestBrowser_Blur, ( void ), {
    window.dispatchEvent( new FocusEvent( "blur" ) );
} );

EM_JS( void, TestBrowser_SetCanvasCssSize, ( int width, int height ), {
    var canvas = document.querySelector( "#canvas" );
    canvas.style.width = width + "px";
    canvas.style.height = height + "px";
} );

EM_JS( int, TestBrowser_CanvasWidth, ( void ), {
    return document.querySelector( "#canvas" ).width;
} );

EM_JS( int, TestBrowser_CursorHidden, ( void ), {
    return "none" == document.querySelector( "#canvas" ).style.cursor ? 1 : 0;
} );

/*-------------------------------------------------------------------------*/

namespace {

/** Counts the target phase of one routed event of one widget */
class CTargetCount : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;
    Int32 lastWheelDelta;
    GUI::TMouseButton lastButton;

    CTargetCount( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 ), lastWheelDelta( 0 ), lastButton( GUI::MOUSEBUTTON_NONE )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CTargetCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != eventId )
            return;
        GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
        if ( GUCEF_NULL != routed && routed->source != notifier )
            return;
        ++count;
        GUI::CWheelEventData* wheel = CORE::CICloneable::CloneableAs< GUI::CWheelEventData >( eventData );
        if ( GUCEF_NULL != wheel )
            lastWheelDelta = wheel->delta;
        GUI::CPointerEventData* pointer = CORE::CICloneable::CloneableAs< GUI::CPointerEventData >( eventData );
        if ( GUCEF_NULL != pointer )
            lastButton = pointer->button;
    }
};

/*-------------------------------------------------------------------------*/

GUI::TWindowManagerBackendPtr
BrowserWindows( void )
{
    GUI::CWindowManager& windows = GUI::CGuiGlobal::Instance()->GetWindowManager();
    if ( windows.GetBackend( "Emscripten" ).IsNULL() )
        GUIDRIVEREMSCRIPTEN::guidriverEmscripten_GUCEFPlugin_Load( 0, GUCEF_NULL );
    return windows.GetBackend( "Emscripten" );
}

}

#endif /* GUCEF_PLATFORM_WASM_EMSCRIPTEN ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiBrowserTests( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Browser" );

    GUCEF_TESTFW_TESTCASE( "Browser: The input driver EMSCRIPTEN knows the keys by their code and the mouse buttons of the browser" )
    try
    {
        typedef INPUTDRIVEREMSCRIPTEN::CEmscriptenInputDriver TDriver;
        ASSERT_TRUE( INPUT::KEYCODE_A == TDriver::KeyCodeFromDomCode( "KeyA" ) );
        ASSERT_TRUE( INPUT::KEYCODE_UP == TDriver::KeyCodeFromDomCode( "ArrowUp" ) );
        ASSERT_TRUE( INPUT::KEYCODE_NUMPADENTER == TDriver::KeyCodeFromDomCode( "NumpadEnter" ) );
        ASSERT_TRUE( INPUT::KEYCODE_F11 == TDriver::KeyCodeFromDomCode( "F11" ) );
        ASSERT_TRUE( INPUT::KEYCODE_UNASSIGNED == TDriver::KeyCodeFromDomCode( "NoSuchKey" ) );
        ASSERT_TRUE( INPUT::KEYCODE_UNASSIGNED == TDriver::KeyCodeFromDomCode( GUCEF_NULL ) );

        // the browser: 0 left, 1 middle, 2 right; gucefINPUT: 0 left, 1 right, 2 middle
        ASSERT_TRUE( 0 == TDriver::ButtonFromDomButton( 0 ) );
        ASSERT_TRUE( 2 == TDriver::ButtonFromDomButton( 1 ) );
        ASSERT_TRUE( 1 == TDriver::ButtonFromDomButton( 2 ) );
        ASSERT_TRUE( 3 == TDriver::ButtonFromDomButton( 3 ) && 4 == TDriver::ButtonFromDomButton( 4 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Browser: The window driver Emscripten makes a WebGL 2 window in the canvas of the page" )
    if ( !IsInBrowser() )
        GUCEF_TESTFW_SKIP( "there is no web page outside a browser" );
    try
    {
        GUI::TWindowManagerBackendPtr backend = BrowserWindows();
        ASSERT_FALSE( backend.IsNULL() );

        // a canvas the page does not have gives no window
        CORE::CValueList params;
        params.Set( "CANVAS", "#noSuchCanvas" );
        GUI::CVideoSettings settings;
        settings.SetResolution( 320, 200, 32 );
        settings.SetFrequency( 0 );
        ASSERT_TRUE( backend->CreateWindowContext( "Test", settings, &params ).IsNULL() );

        GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", settings, GUCEF_NULL );
        ASSERT_FALSE( window.IsNULL() );
        ASSERT_TRUE( 320 == window->GetWidth() && 200 == window->GetHeight() );
        ASSERT_TRUE( 320 == TestBrowser_CanvasWidth() );
        ASSERT_TRUE( "#canvas" == window->GetProperty( "WINDOW" ) );
        ASSERT_TRUE( "GLES" == window->GetProperty( "GL_API" ) && "3.0" == window->GetProperty( "GL_VERSION" ) );
        ASSERT_TRUE( "0" == window->GetProperty( "GL_FRAMEBUFFER" ) );
        ASSERT_FALSE( window->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );

        // a new size is the size of the drawing buffer of the canvas
        settings.SetResolution( 640, 480, 32 );
        ASSERT_TRUE( backend->ApplyVideoSettings( window, settings ) );
        ASSERT_TRUE( 640 == window->GetWidth() && 640 == TestBrowser_CanvasWidth() );

        // the GUI can draw its own pointer
        ASSERT_TRUE( window->SetSystemCursorVisible( false ) );
        ASSERT_TRUE( 1 == TestBrowser_CursorHidden() );
        ASSERT_TRUE( window->SetSystemCursorVisible( true ) );
        ASSERT_TRUE( 0 == TestBrowser_CursorHidden() );

        backend->DestroyWindowContext( window );
        ASSERT_FALSE( window->IsActive() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Browser: Mouse, wheel, keys and text of the page reach the widgets through the input driver EMSCRIPTEN" )
    if ( !IsInBrowser() )
        GUCEF_TESTFW_SKIP( "there is no web page outside a browser" );
    try
    {
        GUI::TWindowManagerBackendPtr backend = BrowserWindows();
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        if ( GUCEF_NULL == manager.GetGuiDriver( "RmlUi" ) )
            GUIDRIVERRMLUIGL::guidriverRmlUIOpenGL_GUCEFPlugin_Load( 0, GUCEF_NULL );
        INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();
        INPUTDRIVEREMSCRIPTEN::inputdriverEMSCRIPTEN_GUCEFPlugin_Load( 0, GUCEF_NULL );
        ASSERT_FALSE( controller.GetDriverByName( "EMSCRIPTEN" ).IsNULL() );

        GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", 640, 480, false );
        ASSERT_FALSE( window.IsNULL() );
        GUI::TGuiContextPtr contextPtr = manager.CreateGUIContext( "RmlUi", window );
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        context->SetLogicalResolution( 640, 480 );

        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CEditbox* edit = static_cast< GUI::CEditbox* >( context->CreateWidget( "Editbox", "Name", root ) );
        edit->SetProperty( "Position", "0,10 0,10" );
        edit->SetProperty( "Size", "0,200 0,20" );
        GUI::CWidget* button = context->CreateWidget( "Button", "Ok", root );
        button->SetProperty( "Position", "0,10 0,100" );
        button->SetProperty( "Size", "0,80 0,30" );
        ASSERT_TRUE( context->ShowRoot( root, false ) );
        CTargetCount clicked( button, GUI::CWidget::MouseClickEvent );
        CTargetCount wheeled( edit, GUI::CWidget::MouseWheelEvent );

        // the page shows the canvas twice as large as its drawing buffer
        TestBrowser_SetCanvasCssSize( 1280, 960 );

        CORE::CValueList params;
        params.Set( "WINDOW", window->GetProperty( "WINDOW" ) );
        INPUT::CInputContext* input = controller.CreateContext( "EMSCRIPTEN", params );
        ASSERT_TRUE( GUCEF_NULL != input );
        GUI::CGUIInputAdapter adapter( *context, input->GetID() );
        adapter.Attach();

        // pointer positions are pixels of the drawing buffer
        TestBrowser_Mouse( "mousemove", 40, 30, 0 );
        ASSERT_TRUE( 20 == context->GetPointerX() && 15 == context->GetPointerY() );
        TestBrowser_Mouse( "mousedown", 40, 30, 0 );
        TestBrowser_Mouse( "mouseup", 40, 30, 0 );
        ASSERT_TRUE( edit->HasFocus() );

        // keys type their text, the browser does not act on them; with Ctrl they are shortcuts, not text
        ASSERT_TRUE( 1 == TestBrowser_Key( "keydown", "KeyA", "A", 0 ) );
        TestBrowser_Key( "keyup", "KeyA", "A", 0 );
        TestBrowser_Key( "keydown", "Digit2", "\xC3\xA9", 0 );
        TestBrowser_Key( "keyup", "Digit2", "\xC3\xA9", 0 );
        ASSERT_TRUE( "A\xC3\xA9" == edit->GetText() );
        TestBrowser_Key( "keydown", "Backspace", "Backspace", 0 );
        TestBrowser_Key( "keyup", "Backspace", "Backspace", 0 );
        ASSERT_TRUE( "A" == edit->GetText() );
        ASSERT_TRUE( 0 == TestBrowser_Key( "keydown", "KeyR", "r", 1 ) );
        TestBrowser_Key( "keyup", "KeyR", "r", 1 );
        ASSERT_TRUE( "A" == edit->GetText() );

        // the wheel in notches: 100 pixels each, up is positive
        TestBrowser_Mouse( "mousemove", 40, 30, 0 );
        TestBrowser_Wheel( 100.0, 0 );
        ASSERT_TRUE( 1 == wheeled.count && -1 == wheeled.lastWheelDelta );
        TestBrowser_Wheel( -50.0, 0 );
        ASSERT_TRUE( 1 == wheeled.count );
        TestBrowser_Wheel( -50.0, 0 );
        ASSERT_TRUE( 2 == wheeled.count && 1 == wheeled.lastWheelDelta );

        // the right button of the browser is the right button of the GUI
        TestBrowser_Mouse( "mousemove", 60, 220, 0 );
        TestBrowser_Mouse( "mousedown", 60, 220, 2 );
        TestBrowser_Mouse( "mouseup", 60, 220, 2 );
        ASSERT_TRUE( 1 == clicked.count && GUI::MOUSEBUTTON_RIGHT == clicked.lastButton );

        // when the page loses the focus the held keys are let go
        TestBrowser_Key( "keydown", "ShiftLeft", "Shift", 0 );
        ASSERT_TRUE( controller.GetKeyboard( 0 ).IsKeyDown( INPUT::KEYCODE_LSHIFT ) );
        TestBrowser_Blur();
        ASSERT_FALSE( controller.GetKeyboard( 0 ).IsKeyDown( INPUT::KEYCODE_LSHIFT ) );

        // a frame draws in the canvas
        IMAGE::TPixelMapPtr pixels;
        context->RenderFrame( 16 );
        ASSERT_TRUE( context->CaptureFrame( pixels ) && !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() );

        adapter.Detach();
        controller.DestroyContext( input );
        TestBrowser_SetCanvasCssSize( 640, 480 );

        // without the context the page's input goes nowhere
        TestBrowser_Key( "keydown", "KeyB", "b", 0 );
        ASSERT_TRUE( "A" == edit->GetText() );
        contextPtr.Unlink();
        backend->DestroyWindowContext( window );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #endif /* GUCEF_PLATFORM_WASM_EMSCRIPTEN ? */
}

/*-------------------------------------------------------------------------*/
