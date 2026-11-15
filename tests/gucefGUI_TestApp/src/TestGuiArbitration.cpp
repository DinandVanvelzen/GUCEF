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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"
#include "gucefGUI_TestInputSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::GUITEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const Int32 ArbitrationMouseId = 9301;
const Int32 ArbitrationKeyboardId = 9302;

typedef CORE::CTSharedPtr< CRecordingWindow, MT::CMutex > TRecordingWindowPtr;

/** A full screen panel with a button at 10,10 (80 x 30) and an edit box at 10,100 (200 x 20) in the context */
struct SScene
{
    GUI::CWidget* root;
    GUI::CWidget* button;
    GUI::CEditbox* edit;

    SScene( GUI::CGUIContext& context )
        : root( GUCEF_NULL ), button( GUCEF_NULL ), edit( GUCEF_NULL )
    {
        root = context.CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        button = context.CreateWidget( "Button", "Ok", root );
        button->SetProperty( "Position", "0,10 0,10" );
        button->SetProperty( "Size", "0,80 0,30" );
        edit = static_cast< GUI::CEditbox* >( context.CreateWidget( "Editbox", "Name", root ) );
        edit->SetProperty( "Position", "0,10 0,100" );
        edit->SetProperty( "Size", "0,200 0,20" );
        context.ShowRoot( root, false );
        context.FlushChanges();
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiArbitrationTests( void )
{GUCEF_TRACE;

    {
    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.InputArbitration" );

    GUCEF_TESTFW_TESTCASE( "A2: By default a layer takes the pointer and the keyboard when it shows something, as before" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        ASSERT_FALSE( bottom.HasHigherLayerTakingInput() || bottom.HasHigherLayerTakingPointerInput() || bottom.HasHigherLayerTakingKeyboardInput() );

        SScene topScene( top );
        ASSERT_TRUE( top.IsTakingPointerInput() && top.IsTakingKeyboardInput() && top.HasVisibleRoots() );
        ASSERT_TRUE( bottom.HasHigherLayerTakingPointerInput() && bottom.HasHigherLayerTakingKeyboardInput() && bottom.HasHigherLayerTakingInput() );
        ASSERT_FALSE( top.HasHigherLayerTakingInput() );

        top.HideRoot( topScene.root );
        ASSERT_FALSE( top.IsTakingPointerInput() || top.IsTakingKeyboardInput() );
        ASSERT_FALSE( bottom.HasHigherLayerTakingInput() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A2: A layer that passes the keyboard on takes the pointer only" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        SScene topScene( top );

        ASSERT_FALSE( top.IsKeyboardInputPassThrough() );
        top.SetKeyboardInputPassThrough( true );
        ASSERT_TRUE( top.IsKeyboardInputPassThrough() && top.IsTakingPointerInput() && !top.IsTakingKeyboardInput() );
        ASSERT_TRUE( bottom.HasHigherLayerTakingPointerInput() );
        ASSERT_FALSE( bottom.HasHigherLayerTakingKeyboardInput() );
        ASSERT_TRUE( bottom.HasHigherLayerTakingInput() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A2: The input adapter decides the pointer and the keyboard apart: the lower layer gets the pointer but not the keys while the higher layer takes only the keyboard, and the other way round" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        SScene topScene( top );
        bottom.SetFocus( bottomScene.edit );
        top.SetFocus( topScene.edit );

        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog clicks( bottomScene.button, &log );

        CScriptedDriver driver;
        driver.Mouse( ArbitrationMouseId );
        driver.Keyboard( ArbitrationKeyboardId );
        GUI::CGUIInputAdapter bottomAdapter( bottom );
        GUI::CGUIInputAdapter topAdapter( top );
        bottomAdapter.Attach();
        topAdapter.Attach();

        // the higher layer takes the keyboard only (a layer of an immediate mode GUI with a text field in use)
        top.customArbitration = true;
        top.takesPointer = false;
        top.takesKeyboard = true;
        ASSERT_FALSE( bottom.HasHigherLayerTakingPointerInput() );
        ASSERT_TRUE( bottom.HasHigherLayerTakingKeyboardInput() );
        driver.Move( 1, ArbitrationMouseId, 20, 20 );
        driver.Button( 1, ArbitrationMouseId, 0, true );
        driver.Button( 1, ArbitrationMouseId, 0, false );
        ASSERT_TRUE( 1 == clicks.Count( "widget:down" ) && 1 == clicks.Count( "widget:up" ) );
        bottom.SetFocus( bottomScene.edit );
        top.SetFocus( topScene.edit );
        driver.Text( 1, ArbitrationKeyboardId, "ab" );
        ASSERT_TRUE( bottomScene.edit->GetText().IsNULLOrEmpty() );
        ASSERT_TRUE( "ab" == topScene.edit->GetText() );

        // the higher layer takes the pointer only (the pointer is over one of its windows, no text field is in use)
        top.takesPointer = true;
        top.takesKeyboard = false;
        driver.Button( 1, ArbitrationMouseId, 0, true );
        driver.Button( 1, ArbitrationMouseId, 0, false );
        ASSERT_TRUE( 1 == clicks.Count( "widget:down" ) );
        bottom.SetFocus( bottomScene.edit );
        driver.Text( 1, ArbitrationKeyboardId, "cd" );
        ASSERT_TRUE( "cd" == bottomScene.edit->GetText() );

        // it takes both: the lower layer gets nothing; takes none: everything
        top.takesKeyboard = true;
        driver.Text( 1, ArbitrationKeyboardId, "x" );
        ASSERT_TRUE( "cd" == bottomScene.edit->GetText() );
        top.takesPointer = false;
        top.takesKeyboard = false;
        driver.Button( 1, ArbitrationMouseId, 0, true );
        driver.Button( 1, ArbitrationMouseId, 0, false );
        bottom.SetFocus( bottomScene.edit );
        driver.Text( 1, ArbitrationKeyboardId, "e" );
        ASSERT_TRUE( 2 == clicks.Count( "widget:down" ) && "cde" == bottomScene.edit->GetText() );

        bottomAdapter.Detach();
        topAdapter.Detach();
        driver.NoMouse( ArbitrationMouseId );
        driver.NoKeyboard( ArbitrationKeyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A2: A press in progress is cancelled when a layer above takes the pointer, not when it takes only the keyboard" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        SScene topScene( top );
        top.customArbitration = true;

        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog events( bottomScene.button, &log );
        bottom.InjectMouseMove( 20, 20 );
        bottom.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( 1 == events.Count( "widget:down" ) );

        top.takesKeyboard = true;
        bottom.RenderFrame( 16 );
        ASSERT_TRUE( 0 == events.Count( "widget:capturelost" ) );

        top.takesPointer = true;
        bottom.RenderFrame( 16 );
        ASSERT_TRUE( 1 == events.Count( "widget:capturelost" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A2: The pointer of the system is the one of the layer that takes the pointer" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        SScene topScene( top );
        top.customArbitration = true;
        top.takesKeyboard = true;

        // the lower layer has a picture as the pointer; the higher layer takes the keyboard only, so the pointer is still the lower layer's
        bottom.SetCursor( "Arrow", 0, 0 );
        window->UpdateSystemCursor();
        ASSERT_TRUE( 0 == windowImpl->pointerShown );

        top.takesPointer = true;
        window->UpdateSystemCursor();
        ASSERT_TRUE( 1 == windowImpl->pointerShown );
        top.takesPointer = false;
        window->UpdateSystemCursor();
        ASSERT_TRUE( 0 == windowImpl->pointerShown );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    }

    {
    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.InputPreview" );

    GUCEF_TESTFW_TESTCASE( "A3: A context that consumes nothing by default delivers input as before, and the previews come first" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );
        GUCEF::vector< CORE::CString > log;
        context.previewLog = &log;
        CWidgetEventLog events( scene.button, &log );

        context.InjectMouseMove( 20, 20 );
        ASSERT_TRUE( log.size() >= 2 && "preview:move" == log[ 0 ] );
        ASSERT_TRUE( 1 == events.Count( "widget:move" ) && context.GetHovered() == scene.button );

        log.clear();
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( !log.empty() && "preview:down" == log[ 0 ] && 1 == events.Count( "widget:down" ) );
        log.clear();
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( !log.empty() && "preview:up" == log[ 0 ] && 1 == events.Count( "widget:up" ) );
        log.clear();
        context.InjectMouseWheel( 1 );
        ASSERT_TRUE( "preview:wheel" == log[ 0 ] && 1 == events.Count( "widget:wheel" ) );

        log.clear();
        context.SetFocus( scene.button );
        context.InjectKey( INPUT::KEYCODE_A, true, 0 );
        ASSERT_TRUE( "preview:keydown" == log[ 0 ] && 1 == events.Count( "widget:keydown" ) );
        context.InjectKey( INPUT::KEYCODE_A, false, 0 );
        ASSERT_TRUE( 1 == events.Count( "widget:keyup" ) );

        context.SetFocus( scene.edit );
        log.clear();
        context.InjectText( "t" );
        ASSERT_TRUE( "preview:text" == log[ 0 ] && "t" == scene.edit->GetText() );

        // navigation is not previewed: Tab moves the focus and makes no preview of its own
        log.clear();
        context.InjectNavigation( GUI::NAVIGATION_NEXT );
        ASSERT_TRUE( log.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A3: A consumed move takes the hover from the widgets but the pointer position is kept" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );
        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog events( scene.button, &log );

        context.InjectMouseMove( 20, 20 );
        ASSERT_TRUE( context.GetHovered() == scene.button && 1 == events.Count( "widget:move" ) );

        context.consumeMove = true;
        context.InjectMouseMove( 30, 25 );
        ASSERT_TRUE( 1 == events.Count( "widget:move" ) );
        ASSERT_TRUE( GUCEF_NULL == context.GetHovered() );
        ASSERT_TRUE( 30 == context.GetPointerX() && 25 == context.GetPointerY() );

        context.consumeMove = false;
        context.InjectMouseMove( 31, 25 );
        ASSERT_TRUE( context.GetHovered() == scene.button && 2 == events.Count( "widget:move" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A3: A consumed press is the preview's alone, its release is not delivered either, and the next press works" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );
        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog events( scene.button, &log );
        context.InjectMouseMove( 20, 20 );

        context.consumeButton = true;
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context.consumeButton = false;
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 0 == events.Count( "widget:down" ) && 0 == events.Count( "widget:up" ) );
        ASSERT_TRUE( 0 == ( scene.button->GetState() & GUI::CWidget::STATE_PRESSED ) );

        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == events.Count( "widget:down" ) && 1 == events.Count( "widget:up" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A3: A press of a widget in progress still gets its moves and its release when the preview consumes them" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );
        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog events( scene.button, &log );

        context.InjectMouseMove( 20, 20 );
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( 0 != ( scene.button->GetState() & GUI::CWidget::STATE_PRESSED ) );

        context.consumeMove = true;
        context.consumeButton = true;
        context.InjectMouseMove( 25, 22 );
        ASSERT_TRUE( 2 == events.Count( "widget:move" ) );
        context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == events.Count( "widget:up" ) );
        ASSERT_TRUE( 0 == ( scene.button->GetState() & GUI::CWidget::STATE_PRESSED ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A3: Consumed wheel, key and text do not reach the widgets, the modifiers of a consumed key are kept" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );
        GUCEF::vector< CORE::CString > log;
        CWidgetEventLog events( scene.button, &log );
        CWheelModifierObserver wheels( scene.button );
        context.InjectMouseMove( 20, 20 );

        context.consumeWheel = true;
        context.InjectMouseWheel( 1 );
        ASSERT_TRUE( 0 == events.Count( "widget:wheel" ) );
        context.consumeWheel = false;

        context.SetFocus( scene.button );
        context.consumeKey = true;
        context.InjectKey( INPUT::KEYCODE_LSHIFT, true, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == events.Count( "widget:keydown" ) );
        context.consumeKey = false;
        context.InjectMouseWheel( 1 );
        ASSERT_TRUE( 1 == wheels.events && INPUT::KEYMOD_SHIFT == wheels.lastModifiers );

        context.SetFocus( scene.edit );
        context.consumeText = true;
        context.InjectText( "no" );
        ASSERT_TRUE( scene.edit->GetText().IsNULLOrEmpty() );
        context.consumeText = false;
        context.InjectText( "yes" );
        ASSERT_TRUE( "yes" == scene.edit->GetText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    }

    {
    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.CursorShapes" );

    GUCEF_TESTFW_TESTCASE( "A4: A window that does not override the shapes says it cannot, and the layers cope" )
    try
    {
        CORE::CTSharedPtr< CPlainWindow, MT::CMutex > windowImpl( GUCEF_NEW CPlainWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        ASSERT_FALSE( windowImpl->GUI::CWindowContext::SetSystemCursorShape( GUI::CURSORSHAPE_HAND ) );

        CLayerTestContext context( window );
        SScene scene( context );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == context.GetSystemCursorShape() );
        context.SetSystemCursorShape( GUI::CURSORSHAPE_TEXT );
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == context.GetSystemCursorShape() );
        window->UpdateSystemCursor();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A4: The shape of the layer that takes the pointer is applied once per change" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );

        context.SetSystemCursorShape( GUI::CURSORSHAPE_TEXT );
        ASSERT_TRUE( 1 == windowImpl->shapeCalls && GUI::CURSORSHAPE_TEXT == windowImpl->lastShape );

        // the same shape again, and a window asked to update: the window is not asked again
        context.SetSystemCursorShape( GUI::CURSORSHAPE_TEXT );
        window->UpdateSystemCursor();
        context.RenderWindowLayers( 16 );
        ASSERT_TRUE( 1 == windowImpl->shapeCalls );

        context.SetSystemCursorShape( GUI::CURSORSHAPE_RESIZE_EW );
        ASSERT_TRUE( 2 == windowImpl->shapeCalls && GUI::CURSORSHAPE_RESIZE_EW == windowImpl->lastShape );
        context.SetSystemCursorShape( GUI::CURSORSHAPE_ARROW );
        ASSERT_TRUE( 3 == windowImpl->shapeCalls && GUI::CURSORSHAPE_ARROW == windowImpl->lastShape );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A4: A window that cannot show a shape is not asked for it again until the shape changes" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        windowImpl->supportsShapes = false;
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );

        context.SetSystemCursorShape( GUI::CURSORSHAPE_HAND );
        ASSERT_TRUE( 1 == windowImpl->shapeCalls );
        for ( UInt32 i=0; i<5; ++i )
        {
            window->UpdateSystemCursor();
            context.RenderWindowLayers( 16 );
        }
        ASSERT_TRUE( 1 == windowImpl->shapeCalls );
        context.SetSystemCursorShape( GUI::CURSORSHAPE_WAIT );
        ASSERT_TRUE( 2 == windowImpl->shapeCalls );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A4: The shape is the one of the layer that takes the pointer, and follows it from layer to layer" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext bottom( window );
        CLayerTestContext top( window );
        top.SetLayer( 1 );
        SScene bottomScene( bottom );
        bottom.SetSystemCursorShape( GUI::CURSORSHAPE_TEXT );
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == windowImpl->lastShape );

        // a higher layer shows something: its shape; nothing shows: the lower layer's again
        top.SetSystemCursorShape( GUI::CURSORSHAPE_HAND );
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == windowImpl->lastShape );
        SScene topScene( top );
        window->UpdateSystemCursor();
        ASSERT_TRUE( GUI::CURSORSHAPE_HAND == windowImpl->lastShape );
        top.HideRoot( topScene.root );
        window->UpdateSystemCursor();
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == windowImpl->lastShape );

        // it shows something but takes no pointer: the pointer, and so the shape, is the lower layer's
        top.ShowRoot( topScene.root, false );
        top.customArbitration = true;
        top.takesKeyboard = true;
        window->UpdateSystemCursor();
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == windowImpl->lastShape );
        top.takesPointer = true;
        window->UpdateSystemCursor();
        ASSERT_TRUE( GUI::CURSORSHAPE_HAND == windowImpl->lastShape );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A4: No shape is set while the GUI draws its own pointer, and the shape comes back with the system pointer" )
    try
    {
        TRecordingWindowPtr windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext context( window );
        SScene scene( context );

        context.SetCursor( "Arrow", 0, 0 );
        ASSERT_TRUE( 0 == windowImpl->pointerShown );
        const UInt32 before = windowImpl->shapeCalls;
        context.SetSystemCursorShape( GUI::CURSORSHAPE_HAND );
        ASSERT_TRUE( before == windowImpl->shapeCalls );

        context.SetCursor( CORE::CString::Empty, 0, 0 );
        ASSERT_TRUE( 1 == windowImpl->pointerShown );
        ASSERT_TRUE( before + 1 == windowImpl->shapeCalls && GUI::CURSORSHAPE_HAND == windowImpl->lastShape );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    }
}

/*-------------------------------------------------------------------------*/
