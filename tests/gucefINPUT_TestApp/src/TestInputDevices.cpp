/*
 *  gucefINPUT_TestApp: GUCEF test application for the INPUT module
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

#include "TestInputDevices.h"

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

/*-------------------------------------------------------------------------*/

/** A driver without hardware: the tests feed it input the way a real driver does */
class CTestInputDriver : public INPUT::CInputDriver
{
    public:

    virtual const CORE::TVersion* GetVersion( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        static CORE::TVersion version = { 1, 0, 0, 0 };
        return &version;
    }

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestInput"; }

    void Mouse( const Int32 deviceId ) { AddMouse( deviceId ); }
    void NoMouse( const Int32 deviceId ) { RemoveMouse( deviceId ); }
    void Keyboard( const Int32 deviceId ) { AddKeyboard( deviceId ); }
    void NoKeyboard( const Int32 deviceId ) { RemoveKeyboard( deviceId ); }

    void Button( const Int32 deviceId, const UInt32 button, const bool pressed ) { InjectMouseButtonChange( 1, deviceId, button, pressed ); }
    void Move( const Int32 deviceId, const Int32 x, const Int32 y ) { InjectMouseMove( 1, deviceId, x, y, 0, 0 ); }
    void Wheel( const Int32 deviceId, const UInt32 variable, const Int32 value, const Int32 delta ) { InjectMouseVariableChange( 1, deviceId, variable, value, delta ); }
    void Key( const Int32 deviceId, const INPUT::KeyCode key, const bool pressed ) { InjectKeyboardKeyChange( 1, deviceId, key, pressed ); }
    void Text( const Int32 deviceId, const CORE::CString& text ) { InjectKeyboardText( 1, deviceId, text ); }

    protected:

    virtual bool OnUpdate( const UInt64 tickcount, const Float64 updateDeltaInMilliSecs, INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual INPUT::CInputContext* CreateContext( const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW INPUT::CInputContext( *this, params ); }

    virtual void DeleteContext( INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { ++deletedContexts; GUCEF_DELETE context; }

    public:

    UInt32 deletedContexts;

    CTestInputDriver( void ) : INPUT::CInputDriver(), deletedContexts( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** Keeps the last event data of each kind a device sends */
class CInputRecorder : public CORE::CObserver
{
    public:

    UInt32 buttonEvents;
    UInt32 lastButton;
    bool lastButtonPressed;
    UInt32 variableEvents;
    UInt32 lastVariable;
    Int32 lastValue;
    Int32 lastDelta;
    UInt32 keyEvents;
    UInt32 lastKeyModifiers;
    UInt32 modifierEvents;
    UInt32 textEvents;
    CORE::CString text;

    CInputRecorder( void )
        : CORE::CObserver(), buttonEvents( 0 ), lastButton( 0 ), lastButtonPressed( false ), variableEvents( 0 ), lastVariable( 0 ),
          lastValue( 0 ), lastDelta( 0 ), keyEvents( 0 ), lastKeyModifiers( 0 ), modifierEvents( 0 ), textEvents( 0 )
    {
    }

    virtual ~CInputRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( INPUT::CMouse::MouseButtonEvent == eventId )
        {
            const INPUT::CMouseButtonEventData* data = static_cast< INPUT::CMouseButtonEventData* >( eventData );
            ++buttonEvents;
            lastButton = data->GetButtonIndex();
            lastButtonPressed = data->GetPressedState();
        }
        else
        if ( INPUT::CMouse::MouseVariableChangedEvent == eventId )
        {
            const INPUT::CMouseVariableChangedEventData* data = static_cast< INPUT::CMouseVariableChangedEventData* >( eventData );
            ++variableEvents;
            lastVariable = data->GetVariableIndex();
            lastValue = data->GetValue();
            lastDelta = data->GetValueDelta();
        }
        else
        if ( INPUT::CKeyboard::KeyStateChangedEvent == eventId )
        {
            const INPUT::CKeyStateChangedEventData* data = static_cast< INPUT::CKeyStateChangedEventData* >( eventData );
            ++keyEvents;
            lastKeyModifiers = data->GetKeyModPressedStates();
        }
        else
        if ( INPUT::CKeyboard::KeyModStateChangedEvent == eventId )
        {
            ++modifierEvents;
        }
        else
        if ( INPUT::CKeyboard::TextInputEvent == eventId )
        {
            const INPUT::CKeyboardTextEventData* data = static_cast< INPUT::CKeyboardTextEventData* >( eventData );
            ++textEvents;
            text += data->GetText();
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformInputDeviceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefINPUT.Devices" );

    const Int32 mouseId = 9001;
    const Int32 keyboardId = 9002;

    INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();

    GUCEF_TESTFW_TESTCASE( "Mouse: buttons beyond the first ones keep their state and report their index" )
    try
    {
        CTestInputDriver driver;
        driver.Mouse( mouseId );
        INPUT::CMouse& mouse = controller.GetMouse( mouseId );
        CInputRecorder recorder;
        recorder.SubscribeTo( &mouse );

        driver.Move( mouseId, 10, 20 );
        driver.Button( mouseId, 4, true );
        ASSERT_TRUE( mouse.IsButtonPressed( 4 ) );
        ASSERT_FALSE( mouse.IsButtonPressed( 0 ) );
        ASSERT_TRUE( 1 == recorder.buttonEvents );
        ASSERT_TRUE( 4 == recorder.lastButton && recorder.lastButtonPressed );

        driver.Button( mouseId, 4, false );
        ASSERT_FALSE( mouse.IsButtonPressed( 4 ) );
        ASSERT_TRUE( 2 == recorder.buttonEvents && !recorder.lastButtonPressed );

        UInt32 x = 0, y = 0;
        mouse.GetMousePos( x, y );
        ASSERT_TRUE( 10 == x && 20 == y );

        recorder.UnsubscribeAllFromObserver();
        driver.NoMouse( mouseId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Mouse: the wheels arrive as variable changes with value and notches" )
    try
    {
        CTestInputDriver driver;
        driver.Mouse( mouseId );
        INPUT::CMouse& mouse = controller.GetMouse( mouseId );
        CInputRecorder recorder;
        recorder.SubscribeTo( &mouse );

        driver.Wheel( mouseId, INPUT::CMouse::MOUSEVARIABLE_WHEEL, 1, 1 );
        driver.Wheel( mouseId, INPUT::CMouse::MOUSEVARIABLE_WHEEL, -2, -3 );
        ASSERT_TRUE( 2 == recorder.variableEvents );
        ASSERT_TRUE( INPUT::CMouse::MOUSEVARIABLE_WHEEL == recorder.lastVariable );
        ASSERT_TRUE( -2 == recorder.lastValue && -3 == recorder.lastDelta );

        driver.Wheel( mouseId, INPUT::CMouse::MOUSEVARIABLE_HORIZONTAL_WHEEL, 1, 1 );
        ASSERT_TRUE( INPUT::CMouse::MOUSEVARIABLE_HORIZONTAL_WHEEL == recorder.lastVariable );

        // the wheel is no button: the button states stay untouched
        ASSERT_TRUE( 0 == recorder.buttonEvents );

        recorder.UnsubscribeAllFromObserver();
        driver.NoMouse( mouseId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keyboard: the modifier keys keep the modifier state, left and right together" )
    try
    {
        CTestInputDriver driver;
        driver.Keyboard( keyboardId );
        INPUT::CKeyboard& keyboard = controller.GetKeyboard( keyboardId );
        CInputRecorder recorder;
        recorder.SubscribeTo( &keyboard );

        driver.Key( keyboardId, INPUT::KEYCODE_LSHIFT, true );
        ASSERT_TRUE( keyboard.IsKeyModifierActive( INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( 1 == recorder.modifierEvents );

        // a key pressed with shift held carries the modifier
        driver.Key( keyboardId, INPUT::KEYCODE_A, true );
        ASSERT_TRUE( 0 != ( recorder.lastKeyModifiers & INPUT::KEYMOD_SHIFT ) );
        driver.Key( keyboardId, INPUT::KEYCODE_A, false );

        // the other shift key down and the first one up: shift stays active
        driver.Key( keyboardId, INPUT::KEYCODE_RSHIFT, true );
        driver.Key( keyboardId, INPUT::KEYCODE_LSHIFT, false );
        ASSERT_TRUE( keyboard.IsKeyModifierActive( INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( 1 == recorder.modifierEvents );

        // both up: shift is released, once
        driver.Key( keyboardId, INPUT::KEYCODE_RSHIFT, false );
        ASSERT_FALSE( keyboard.IsKeyModifierActive( INPUT::KEYMOD_SHIFT ) );
        ASSERT_TRUE( 2 == recorder.modifierEvents );

        driver.Key( keyboardId, INPUT::KEYCODE_RCONTROL, true );
        driver.Key( keyboardId, INPUT::KEYCODE_LMENU, true );
        ASSERT_TRUE( ( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT ) == keyboard.GetKeyModStates() );
        driver.Key( keyboardId, INPUT::KEYCODE_RCONTROL, false );
        driver.Key( keyboardId, INPUT::KEYCODE_LMENU, false );
        ASSERT_TRUE( 0 == keyboard.GetKeyModStates() );

        recorder.UnsubscribeAllFromObserver();
        driver.NoKeyboard( keyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keyboard: typed text arrives as UTF-8, apart from the key events" )
    try
    {
        CTestInputDriver driver;
        driver.Keyboard( keyboardId );
        INPUT::CKeyboard& keyboard = controller.GetKeyboard( keyboardId );
        CInputRecorder recorder;
        recorder.SubscribeTo( &keyboard );

        driver.Text( keyboardId, "a" );
        driver.Text( keyboardId, "\xC3\xA9" );          // e with acute
        driver.Text( keyboardId, "\xE2\x82\xAC" );      // the euro sign
        ASSERT_TRUE( 3 == recorder.textEvents );
        ASSERT_TRUE( recorder.text == "a\xC3\xA9\xE2\x82\xAC" );
        ASSERT_TRUE( 0 == recorder.keyEvents );

        recorder.UnsubscribeAllFromObserver();
        driver.NoKeyboard( keyboardId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Driver: input for a device that is not attached is ignored" )
    try
    {
        CTestInputDriver driver;
        UInt32 mice = controller.GetMouseCount();
        driver.Button( mouseId, 0, true );
        driver.Wheel( mouseId, INPUT::CMouse::MOUSEVARIABLE_WHEEL, 1, 1 );
        driver.Text( keyboardId, "x" );
        ASSERT_TRUE( mice == controller.GetMouseCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Driver: When a driver is unregistered its contexts go with it" )
    try
    {
        CTestInputDriver* driver = GUCEF_NEW CTestInputDriver();
        INPUT::TInputDriverPtr driverPtr( driver );
        controller.RegisterDriver( driverPtr );
        const UInt32 before = controller.GetContextCount();
        CORE::CValueList params;
        INPUT::CInputContext* first = controller.CreateContext( "TestInput", params );
        INPUT::CInputContext* second = controller.CreateContext( "TestInput", params );
        ASSERT_TRUE( GUCEF_NULL != first && GUCEF_NULL != second );
        ASSERT_TRUE( before + 2 == controller.GetContextCount() );

        // the plugin of the driver is unloaded next: nothing may use its contexts any more
        controller.UnregisterDriver( "TestInput" );
        ASSERT_TRUE( before == controller.GetContextCount() );
        ASSERT_TRUE( 2 == driver->deletedContexts );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
