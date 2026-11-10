/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

/*-------------------------------------------------------------------------*/

/** An input driver without hardware: the tests feed it input the way a window's input driver does */
class CScriptedInputDriver : public INPUT::CInputDriver
{
    public:

    virtual const CORE::TVersion* GetVersion( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        static CORE::TVersion version = { 1, 0, 0, 0 };
        return &version;
    }

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ScriptedInput"; }

    void Mouse( const Int32 deviceId ) { AddMouse( deviceId ); }
    void NoMouse( const Int32 deviceId ) { RemoveMouse( deviceId ); }
    void Keyboard( const Int32 deviceId ) { AddKeyboard( deviceId ); }
    void NoKeyboard( const Int32 deviceId ) { RemoveKeyboard( deviceId ); }

    void Move( const Int32 contextId, const Int32 deviceId, const Int32 x, const Int32 y ) { InjectMouseMove( contextId, deviceId, x, y, 0, 0 ); }
    void Button( const Int32 contextId, const Int32 deviceId, const UInt32 button, const bool pressed ) { InjectMouseButtonChange( contextId, deviceId, button, pressed ); }
    void Wheel( const Int32 contextId, const Int32 deviceId, const Int32 value, const Int32 delta ) { InjectMouseVariableChange( contextId, deviceId, INPUT::CMouse::MOUSEVARIABLE_WHEEL, value, delta ); }
    void HorizontalWheel( const Int32 contextId, const Int32 deviceId, const Int32 value, const Int32 delta ) { InjectMouseVariableChange( contextId, deviceId, INPUT::CMouse::MOUSEVARIABLE_HORIZONTAL_WHEEL, value, delta ); }
    void Variable( const Int32 contextId, const Int32 deviceId, const UInt32 variable, const Int32 value, const Int32 delta ) { InjectMouseVariableChange( contextId, deviceId, variable, value, delta ); }
    void Key( const Int32 contextId, const Int32 deviceId, const INPUT::KeyCode key, const bool pressed ) { InjectKeyboardKeyChange( contextId, deviceId, key, pressed ); }
    void Text( const Int32 contextId, const Int32 deviceId, const CORE::CString& text ) { InjectKeyboardText( contextId, deviceId, text ); }

    void Click( const Int32 contextId, const Int32 deviceId, const Int32 x, const Int32 y )
    {
        Move( contextId, deviceId, x, y );
        Button( contextId, deviceId, 0, true );
        Button( contextId, deviceId, 0, false );
    }

    protected:

    virtual bool OnUpdate( const UInt64 tickcount, const Float64 updateDeltaInMilliSecs, INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual INPUT::CInputContext* CreateContext( const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW INPUT::CInputContext( *this, params ); }

    virtual void DeleteContext( INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { GUCEF_DELETE context; }
};

/*-------------------------------------------------------------------------*/

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

const Int32 TestMouseId = 9101;
const Int32 TestKeyboardId = 9102;

/*-------------------------------------------------------------------------*/

/** A window without a screen that remembers whether the pointer of the system is shown */
class CTestWindow : public GUI::CWindowContext
{
    public:

    Int32 systemCursorShown;        /**< -1 when never told */
    UInt32 systemCursorChanges;

    CTestWindow( void ) : GUI::CWindowContext(), systemCursorShown( -1 ), systemCursorChanges( 0 ) {}

    virtual UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE { return 640; }
    virtual UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE { return 480; }
    virtual UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestWindow"; }
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE { return GUI::TGuiContextPtr(); }
    virtual CORE::CString GetProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CString::Empty; }

    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE
    {
        systemCursorShown = visible ? 1 : 0;
        ++systemCursorChanges;
        return true;
    }
};

/** Whether the pixel of a frame is red, the colour of the cursor picture of the test */
bool
IsRed( const IMAGE::TPixelMapPtr& frame, UInt32 x, UInt32 y )
{
    const UInt8* pixel = static_cast< const UInt8* >( frame->GetDataPtr() ) + ( (size_t) y * frame->GetWidthInPixels() + x ) * 4;
    return 255 == pixel[ 0 ] && 0 == pixel[ 1 ] && 0 == pixel[ 2 ];
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiInputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Input" );

    GUCEF_TESTFW_TESTCASE( "Input: The adapter maps the gucefINPUT button indices to GUI buttons" )
    try
    {
        ASSERT_TRUE( GUI::MOUSEBUTTON_LEFT == GUI::CGUIInputAdapter::MapMouseButton( 0 ) );
        ASSERT_TRUE( GUI::MOUSEBUTTON_RIGHT == GUI::CGUIInputAdapter::MapMouseButton( 1 ) );
        ASSERT_TRUE( GUI::MOUSEBUTTON_MIDDLE == GUI::CGUIInputAdapter::MapMouseButton( 2 ) );
        ASSERT_TRUE( GUI::MOUSEBUTTON_X1 == GUI::CGUIInputAdapter::MapMouseButton( 3 ) );
        ASSERT_TRUE( GUI::MOUSEBUTTON_X2 == GUI::CGUIInputAdapter::MapMouseButton( 4 ) );
        ASSERT_TRUE( GUI::MOUSEBUTTON_NONE == GUI::CGUIInputAdapter::MapMouseButton( 9 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: Mouse and keyboard input from a driver reaches the widgets through the adapter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
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

        CScriptedInputDriver driver;
        driver.Keyboard( TestKeyboardId );
        GUI::CGUIInputAdapter adapter( *context );
        adapter.Attach();
        ASSERT_TRUE( adapter.IsAttached() );

        // a mouse attached after the adapter is picked up too
        driver.Mouse( TestMouseId );

        driver.Click( 1, TestMouseId, 20, 15 );
        ASSERT_TRUE( 20 == context->GetPointerX() && 15 == context->GetPointerY() );
        ASSERT_TRUE( edit->HasFocus() );

        driver.Text( 1, TestKeyboardId, "Ab\xC3\xA9" );
        ASSERT_TRUE( "Ab\xC3\xA9" == edit->GetText() );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_BACK, true );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_BACK, false );
        ASSERT_TRUE( "Ab" == edit->GetText() );

        driver.Wheel( 1, TestMouseId, -1, -1 );
        ASSERT_TRUE( 1 == wheeled.count && -1 == wheeled.lastWheelDelta );

        driver.Click( 1, TestMouseId, 30, 110 );
        ASSERT_TRUE( 1 == clicked.count && GUI::MOUSEBUTTON_LEFT == clicked.lastButton );

        // gucefINPUT button 1 is the right button
        driver.Button( 1, TestMouseId, 1, true );
        driver.Button( 1, TestMouseId, 1, false );
        ASSERT_TRUE( 2 == clicked.count && GUI::MOUSEBUTTON_RIGHT == clicked.lastButton );

        // after Detach nothing arrives any more
        adapter.Detach();
        driver.Text( 1, TestKeyboardId, "x" );
        ASSERT_TRUE( "Ab" == edit->GetText() );

        driver.NoMouse( TestMouseId );
        driver.NoKeyboard( TestKeyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: A scroll panel scrolls with the wheel of a mouse (a notch towards the user is down), over a disabled button too, and with its thumb and track, through the adapter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
        panel->SetProperty( "Position", "0,10 0,20" );
        panel->SetProperty( "Size", "0,200 0,100" );
        GUI::CWidget* first = context->CreateWidget( "Button", "First", panel );
        first->SetProperty( "Size", "1,0 0,40" );
        GUI::CWidget* second = context->CreateWidget( "Button", "Second", panel );
        second->SetProperty( "Position", "0,0 0,60" );
        second->SetProperty( "Size", "1,0 0,300" );
        second->SetEnabled( false );
        ASSERT_TRUE( context->ShowRoot( root, false ) );
        context->FlushChanges();
        ASSERT_TRUE( panel->IsScrollable() );

        CScriptedInputDriver driver;
        driver.Mouse( TestMouseId );
        GUI::CGUIInputAdapter adapter( *context );
        adapter.Attach();

        // over the enabled button, then over the disabled one
        driver.Move( 1, TestMouseId, 40, 40 );
        driver.Wheel( 1, TestMouseId, -1, -1 );
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );
        driver.Move( 1, TestMouseId, 40, 100 );
        driver.Wheel( 1, TestMouseId, -2, -1 );
        ASSERT_TRUE( 96 == panel->GetScrollPosition() );
        driver.Wheel( 1, TestMouseId, -1, 1 );
        ASSERT_TRUE( 48 == panel->GetScrollPosition() );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();

        // the thumb dragged, then a click on the track
        const GUI::CRect track = panel->GetTrackRect();
        const GUI::CRect thumb = panel->GetThumbRect();
        const Int32 x = 10 + track.left + track.width / 2;
        driver.Move( 1, TestMouseId, x, 20 + thumb.top + thumb.height / 2 );
        driver.Button( 1, TestMouseId, 0, true );
        driver.Move( 1, TestMouseId, x, 20 + thumb.top + thumb.height / 2 + 10 );
        ASSERT_TRUE( panel->IsThumbDragged() && panel->GetScrollPosition() > 0 );
        driver.Button( 1, TestMouseId, 0, false );
        ASSERT_FALSE( panel->IsThumbDragged() );
        panel->SetScrollPosition( 0 );
        context->FlushChanges();
        driver.Move( 1, TestMouseId, x, 20 + track.Bottom() - 3 );
        driver.Button( 1, TestMouseId, 0, true );
        driver.Button( 1, TestMouseId, 0, false );
        ASSERT_TRUE( panel->GetScrollPosition() >= 90 );
        driver.NoMouse( TestMouseId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: The wheel as a window system driver sends it in bursts (a wheel flung, both axes, a mouse that never moved, huge counts, unknown variables) never breaks the adapter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CScrollPanel* panel = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", root ) );
        panel->SetProperty( "Position", "0,10 0,20" );
        panel->SetProperty( "Size", "0,200 0,100" );
        GUI::CWidget* content = context->CreateWidget( "Button", "Content", panel );
        content->SetProperty( "Size", "1,0 0,400" );
        ASSERT_TRUE( context->ShowRoot( root, false ) );
        context->FlushChanges();

        CScriptedInputDriver driver;
        driver.Mouse( TestMouseId );
        GUI::CGUIInputAdapter adapter( *context );
        adapter.Attach();

        // a mouse that was never moved: the wheel goes to whatever is at the origin, which is nothing of the panel
        driver.Wheel( 1, TestMouseId, -1, -1 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // a wheel flung: dozens of groups at once, in both directions, with the horizontal one in between
        driver.Move( 1, TestMouseId, 40, 40 );
        for ( Int32 i=0; i<40; ++i )
        {
            driver.Wheel( 1, TestMouseId, -( i + 1 ), -1 );
            driver.HorizontalWheel( 1, TestMouseId, i + 1, 1 );
        }
        ASSERT_TRUE( panel->GetScrollPosition() > 0 );
        for ( Int32 i=0; i<80; ++i )
            driver.Wheel( 1, TestMouseId, i, 1 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // counts that no wheel has: they scroll as far as there is to scroll and no further
        driver.Wheel( 1, TestMouseId, -2147483647, -2147483647 );
        const Int32 end = panel->GetScrollPosition();
        ASSERT_TRUE( end > 0 );
        driver.Wheel( 1, TestMouseId, 2147483647, 2147483647 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
        driver.Wheel( 1, TestMouseId, 0, 0 );
        driver.Variable( 1, TestMouseId, 7, 5, 5 );
        driver.Variable( 1, TestMouseId, 0xFFFFFFFFu, 5, 5 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );

        // a device that is gone: the input has nowhere to go and says so, without a fuss
        driver.NoMouse( TestMouseId );
        driver.Wheel( 1, TestMouseId, -1, -1 );
        ASSERT_TRUE( 0 == panel->GetScrollPosition() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: The highest layer of a window that shows something takes the input" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !IsInBrowser() )
        GUCEF_TESTFW_SKIP( "layers need a window, there is no web page outside a browser" );
    #endif
    try
    {
        GUI::TGuiContextPtr bottomPtr = CreateRmlUiContext();
        GUI::CGUIContext* bottom = static_cast< GUI::CGUIContext* >( bottomPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != bottom );
        if ( GUCEF_NULL != bottom )
        {
            GUI::TGuiContextPtr topPtr = GUI::CGuiGlobal::Instance()->GetGuiManager().CreateGUIContext( "RmlUi", bottom->GetWindowContext() );
            GUI::CGUIContext* top = static_cast< GUI::CGUIContext* >( topPtr.GetPointerAlways() );
            top->SetLogicalResolution( 640, 480 );
            top->SetLayer( 1 );

            GUI::CWidget* bottomButton = bottom->CreateWidget( "Button", "Below", GUCEF_NULL );
            bottomButton->SetProperty( "Size", "1,0 1,0" );
            bottom->ShowRoot( bottomButton, false );
            GUI::CWidget* topButton = top->CreateWidget( "Button", "Above", GUCEF_NULL );
            topButton->SetProperty( "Size", "1,0 1,0" );
            CTargetCount belowClicks( bottomButton, GUI::CWidget::MouseClickEvent );
            CTargetCount aboveClicks( topButton, GUI::CWidget::MouseClickEvent );

            CScriptedInputDriver driver;
            driver.Mouse( TestMouseId );
            GUI::CGUIInputAdapter bottomAdapter( *bottom );
            GUI::CGUIInputAdapter topAdapter( *top );
            bottomAdapter.Attach();
            topAdapter.Attach();

            // the higher layer shows nothing: the lower one gets the click
            driver.Click( 1, TestMouseId, 20, 20 );
            ASSERT_TRUE( 1 == belowClicks.count && 0 == aboveClicks.count );

            // it shows something: it takes the click, the lower one gets none
            top->ShowRoot( topButton, false );
            ASSERT_TRUE( bottom->HasHigherLayerTakingInput() && !top->HasHigherLayerTakingInput() );
            driver.Click( 1, TestMouseId, 20, 20 );
            ASSERT_TRUE( 1 == belowClicks.count && 1 == aboveClicks.count );

            top->HideRoot( topButton );
            driver.Click( 1, TestMouseId, 20, 20 );
            ASSERT_TRUE( 2 == belowClicks.count && 1 == aboveClicks.count );

            bottomAdapter.Detach();
            topAdapter.Detach();
            driver.NoMouse( TestMouseId );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: The pointer is the one of the layer that takes the input: a picture of its own or the system's" )
    try
    {
        // the driver
        GUI::TGuiContextPtr loader = CreateNullContext();
        ASSERT_TRUE( !loader.IsNULL() );

        CORE::CTSharedPtr< CTestWindow, MT::CMutex > testWindow( GUCEF_NEW CTestWindow() );
        GUI::TWindowContextPtr window( testWindow );
        GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
        GUI::TGuiContextPtr bottomPtr = manager.CreateGUIContext( "Null", window );
        GUI::TGuiContextPtr topPtr = manager.CreateGUIContext( "Null", window );
        GUI::CGUIContext* bottom = ( bottomPtr.IsNULL() ? GUCEF_NULL : bottomPtr->AsGUIContext() );
        GUI::CGUIContext* top = ( topPtr.IsNULL() ? GUCEF_NULL : topPtr->AsGUIContext() );
        ASSERT_TRUE( GUCEF_NULL != bottom && GUCEF_NULL != top );
        if ( GUCEF_NULL != bottom && GUCEF_NULL != top )
        {
            bottom->SetLogicalResolution( 640, 480 );
            top->SetLogicalResolution( 640, 480 );
            top->SetLayer( 1 );

            // the lower layer draws a picture as the pointer: no pointer of the system
            ASSERT_TRUE( bottom->RegisterImage( "Arrow", MakeSolidImage( 8, 8, 255, 0, 0, 255 ) ) );
            bottom->SetCursor( "Arrow", 0, 0 );
            bottom->InjectMouseMove( 100, 100 );
            ASSERT_TRUE( 0 == testWindow->systemCursorShown );
            IMAGE::TPixelMapPtr frame;
            ASSERT_TRUE( bottom->CaptureFrame( frame ) && !frame.IsNULL() && IsRed( frame, 102, 102 ) );

            // the higher layer shows something and takes the input: the pointer of the system, no picture below it
            GUI::CWidget* panel = top->CreateWidget( "Panel", "Settings", GUCEF_NULL );
            panel->SetProperty( "Size", "1,0 1,0" );
            top->ShowRoot( panel, false );
            window->UpdateSystemCursor();
            ASSERT_TRUE( 1 == testWindow->systemCursorShown );
            ASSERT_TRUE( bottom->CaptureFrame( frame ) && !frame.IsNULL() && !IsRed( frame, 102, 102 ) );

            // the window is only told when that changes
            const UInt32 changes = testWindow->systemCursorChanges;
            window->UpdateSystemCursor();
            ASSERT_TRUE( changes == testWindow->systemCursorChanges );

            // gone again: the picture of the lower layer
            top->HideRoot( panel );
            window->UpdateSystemCursor();
            ASSERT_TRUE( 0 == testWindow->systemCursorShown );
            ASSERT_TRUE( bottom->CaptureFrame( frame ) && !frame.IsNULL() && IsRed( frame, 102, 102 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: The developer console drops down on its key and goes back up on it, when it is switched on" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );

        GUI::CDeveloperConsoleForm console( context );
        console.SetSlideTimeInMs( 100 );
        ASSERT_TRUE( console.Build() );
        ASSERT_FALSE( console.IsVisible() || console.IsOpen() || context->HasVisibleRoots() );
        CTargetCount opened( &console, GUI::CDeveloperConsoleForm::OpenedEvent );
        CTargetCount closed( &console, GUI::CDeveloperConsoleForm::ClosedEvent );
        CTargetCount entered( &console, GUI::CDeveloperConsoleForm::CommandEnteredEvent );

        CScriptedInputDriver driver;
        driver.Keyboard( TestKeyboardId );
        GUI::CGUIInputAdapter adapter( *context );
        adapter.Attach();

        // switched off the key does nothing
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_FALSE( console.IsOpen() );

        // switched on it drops down from above the top, in the slide time, frame by frame
        console.SetEnabled( true );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Text( 1, TestKeyboardId, "`" );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_TRUE( console.IsOpen() && console.IsVisible() && 1 == opened.count );
        context->FlushChanges();
        ASSERT_TRUE( console.GetRootWidget()->GetRect().Bottom() <= 0 );
        context->RenderFrame( 50 );
        ASSERT_TRUE( console.GetSlidePosition() > 0.4f && console.GetSlidePosition() < 0.6f );
        context->RenderFrame( 60 );
        context->FlushChanges();
        ASSERT_TRUE( 1.0f == console.GetSlidePosition() && 0 == console.GetRootWidget()->GetRect().top );

        // the character of the key is not typed, what is typed after it is; Enter gives the command
        GUI::CEditbox* commandLine = console.GetWidgetAs< GUI::CEditbox >( "CommandLine" );
        ASSERT_TRUE( GUCEF_NULL != commandLine && commandLine->HasFocus() && commandLine->GetText().IsNULLOrEmpty() );
        driver.Text( 1, TestKeyboardId, "help" );
        ASSERT_TRUE( "help" == commandLine->GetText() );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_RETURN, true );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_RETURN, false );
        ASSERT_TRUE( 1 == entered.count && commandLine->GetText().IsNULLOrEmpty() );
        GUI::CTextbox* output = console.GetWidgetAs< GUI::CTextbox >( "Output" );
        ASSERT_TRUE( GUCEF_NULL != output && output->GetTextAsString().HasSubstr( "> help" ) >= 0 );

        // the key again: it goes back up and is gone when the slide is done
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Text( 1, TestKeyboardId, "`" );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_FALSE( console.IsOpen() );
        ASSERT_TRUE( console.IsVisible() && 0 == closed.count );
        context->RenderFrame( 120 );
        ASSERT_FALSE( console.IsVisible() || context->HasVisibleRoots() );
        ASSERT_TRUE( 1 == closed.count && commandLine->GetText().IsNULLOrEmpty() );

        // only keys of its input context, when it is told one
        console.SetInputContextId( 2 );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Key( 1, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_FALSE( console.IsOpen() );
        driver.Key( 2, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Key( 2, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_TRUE( console.IsOpen() );

        // switched off it closes, and the key does nothing anymore
        console.SetEnabled( false );
        ASSERT_FALSE( console.IsOpen() );
        context->RenderFrame( 200 );
        ASSERT_FALSE( console.IsVisible() );
        driver.Key( 2, TestKeyboardId, INPUT::KEYCODE_GRAVE, true );
        driver.Key( 2, TestKeyboardId, INPUT::KEYCODE_GRAVE, false );
        ASSERT_FALSE( console.IsOpen() );

        adapter.Detach();
        driver.NoKeyboard( TestKeyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: An adapter for one input context ignores the input of other windows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CEditbox* edit = static_cast< GUI::CEditbox* >( context->CreateWidget( "Editbox", "Name", root ) );
        edit->SetProperty( "Size", "0,200 0,20" );
        ASSERT_TRUE( context->ShowRoot( root, false ) );
        context->SetFocus( edit );

        CScriptedInputDriver driver;
        driver.Mouse( TestMouseId );
        driver.Keyboard( TestKeyboardId );
        GUI::CGUIInputAdapter adapter( *context, 7 );
        adapter.Attach();

        driver.Text( 3, TestKeyboardId, "other" );
        driver.Move( 3, TestMouseId, 300, 300 );
        ASSERT_TRUE( edit->GetText().IsNULLOrEmpty() );
        ASSERT_FALSE( 300 == context->GetPointerX() );

        driver.Text( 7, TestKeyboardId, "ours" );
        driver.Move( 7, TestMouseId, 300, 300 );
        ASSERT_TRUE( "ours" == edit->GetText() );
        ASSERT_TRUE( 300 == context->GetPointerX() && 300 == context->GetPointerY() );

        adapter.Detach();
        driver.NoMouse( TestMouseId );
        driver.NoKeyboard( TestKeyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: A pointer move in a window of hundreds of widgets under a theme of dozens of styles is cheap, a mouse that reports a thousand times a second must not starve the application" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );

        // a theme the size of a game's: styles by type, class, name and state, none of which says anything about the clipping or the input of a widget
        CORE::CDataNode theme( "GUITheme" );
        for ( UInt32 n=0; n<80; ++n )
        {
            CORE::CDataNode* style = theme.AddChild( "Style" );
            style->SetAttribute( "type", 0 == n % 2 ? "Button" : "Panel" );
            if ( 0 == n % 3 )
                style->SetAttribute( "state", "hover" );
            if ( 0 == n % 5 )
                style->SetAttribute( "class", "Class" + CORE::UInt32ToString( n ) );
            for ( UInt32 p=0; p<4; ++p )
            {
                CORE::CDataNode* property = style->AddChild( "Property" );
                property->SetAttribute( "name", 0 == p ? "TextColour" : "Other" + CORE::UInt32ToString( p ) );
                property->SetAttribute( "value", "10,10,10,255" );
            }
        }
        ASSERT_TRUE( context->SetTheme( theme ) );

        // 20 rows of 10 buttons
        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        for ( UInt32 row=0; row<20; ++row )
        {
            GUI::CWidget* group = context->CreateWidget( "Panel", "Row" + CORE::UInt32ToString( row ), root );
            group->SetProperty( "Position", "0,0 0," + CORE::UInt32ToString( row * 20 ) );
            group->SetProperty( "Size", "1,0 0,20" );
            for ( UInt32 column=0; column<10; ++column )
            {
                GUI::CWidget* button = context->CreateWidget( "Button", "Button" + CORE::UInt32ToString( column ), group );
                button->SetProperty( "Position", "0," + CORE::UInt32ToString( column * 60 ) + " 0,0" );
                button->SetProperty( "Size", "0,56 0,18" );
            }
        }
        context->ShowRoot( root, false );
        context->FlushChanges();
        ASSERT_TRUE( context->GetOwnedWidgets().size() > 200 );

        const UInt64 start = MT::PrecisionTickCount();
        const UInt32 moves = 200;
        for ( UInt32 i=0; i<moves; ++i )
            context->InjectMouseMove( (Int32) ( ( i * 7 ) % 600 ), (Int32) ( ( i * 3 ) % 400 ) );
        const UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();

        // the whole run, in a debug build too: 1.5 ms a move is more than ten times what it needs
        ASSERT_TRUE( elapsedMs < 300 );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input: A theme can still set the clipping of a widget, which the hit test honours" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );

        CORE::CDataNode theme( "GUITheme" );
        CORE::CDataNode* style = theme.AddChild( "Style" );
        style->SetAttribute( "name", "Clipped" );
        CORE::CDataNode* property = style->AddChild( "Property" );
        property->SetAttribute( "name", "Clip" );
        property->SetAttribute( "value", "true" );
        style = theme.AddChild( "Style" );
        style->SetAttribute( "name", "Ghost" );
        property = style->AddChild( "Property" );
        property->SetAttribute( "name", "InputTransparent" );
        property->SetAttribute( "value", "true" );
        ASSERT_TRUE( context->SetTheme( theme ) );

        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* clipped = context->CreateWidget( "Panel", "Clipped", root );
        clipped->SetProperty( "Position", "0,10 0,10" );
        clipped->SetProperty( "Size", "0,100 0,100" );
        GUI::CWidget* sticksOut = context->CreateWidget( "Button", "SticksOut", clipped );
        sticksOut->SetProperty( "Position", "0,150 0,0" );
        sticksOut->SetProperty( "Size", "0,100 0,50" );
        GUI::CWidget* ghost = context->CreateWidget( "Panel", "Ghost", root );
        ghost->SetProperty( "Position", "0,300 0,300" );
        ghost->SetProperty( "Size", "0,50 0,50" );
        context->ShowRoot( root, false );
        context->FlushChanges();

        ASSERT_TRUE( clipped->GetBoolProperty( "Clip" ) );
        ASSERT_TRUE( context->HitTest( 20, 20 ) == clipped );
        // outside the clipping panel: not reachable through the child that sticks out of it
        ASSERT_TRUE( context->HitTest( 170, 20 ) == root );
        ASSERT_TRUE( context->HitTest( 320, 320 ) == root );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
