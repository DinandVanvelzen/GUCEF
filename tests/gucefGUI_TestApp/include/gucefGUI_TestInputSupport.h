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

#ifndef GUCEF_GUI_TESTINPUTSUPPORT_H
#define GUCEF_GUI_TESTINPUTSUPPORT_H

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

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUITEST {

/** A handler of widgets that does nothing, so a context without a driver has widgets that work */
class CTestWidgetHandler : public GUI::CWidgetHandler
{
    public:

    virtual bool CreateNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void DestroyNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE {}
};

/*-------------------------------------------------------------------------*/

/**
 *  A window without a screen that records what the GUI layers ask of the pointer of the system: whether it is shown and
 *  the shape, and that can be told to not support shapes (answering false, as the window drivers without them do)
 */
class CRecordingWindow : public GUI::CWindowContext
{
    public:

    Int32 pointerShown;                         /**< -1 when never told */
    UInt32 visibilityCalls;
    UInt32 shapeCalls;
    GUI::TCursorShape lastShape;
    bool supportsShapes;
    GUCEF::vector< GUI::TCursorShape > shapes;  /**< every shape the window was asked for, in order */

    CRecordingWindow( void ) : GUI::CWindowContext(), pointerShown( -1 ), visibilityCalls( 0 ), shapeCalls( 0 ), lastShape( GUI::CURSORSHAPE_ARROW ), supportsShapes( true ), shapes() {}

    virtual UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE { return 640; }
    virtual UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE { return 480; }
    virtual UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "RecordingWindow"; }
    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE { return GUI::TGuiContextPtr(); }
    virtual CORE::CString GetProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CString::Empty; }

    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE
    {
        pointerShown = visible ? 1 : 0;
        ++visibilityCalls;
        return true;
    }

    virtual bool SetSystemCursorShape( GUI::TCursorShape shape ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++shapeCalls;
        lastShape = shape;
        shapes.push_back( shape );
        return supportsShapes;
    }
};

/** A window that has no shapes and no visibility control: what a window driver gets that overrides nothing */
class CPlainWindow : public CRecordingWindow
{
    public:

    virtual bool SetSystemCursorVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE { return GUI::CWindowContext::SetSystemCursorVisible( visible ); }
    virtual bool SetSystemCursorShape( GUI::TCursorShape shape ) GUCEF_VIRTUAL_OVERRIDE { return GUI::CWindowContext::SetSystemCursorShape( shape ); }
};

/*-------------------------------------------------------------------------*/

/**
 *  A GUI context without a driver, with decisions of its own about the layer arbitration and the input previews:
 *  by default it behaves as every context (nothing consumed, a layer takes input with a visible root)
 */
class CLayerTestContext : public GUI::CGUIContext
{
    public:

    bool customArbitration;     /**< IsTaking...() answer with the flags below instead of the default */
    bool takesPointer;
    bool takesKeyboard;
    bool consumeMove;
    bool consumeButton;
    bool consumeWheel;
    bool consumeKey;
    bool consumeText;
    GUCEF::vector< CORE::CString >* previewLog;     /**< shared with the observers of the test, to see the order; NULL for none */

    CLayerTestContext( GUI::TWindowContextPtr window )
        : GUI::CGUIContext( GUCEF_NULL, window ), customArbitration( false ), takesPointer( false ), takesKeyboard( false )
        , consumeMove( false ), consumeButton( false ), consumeWheel( false ), consumeKey( false ), consumeText( false ), previewLog( GUCEF_NULL )
    {
        SetLogicalResolution( 640, 480 );
    }

    virtual ~CLayerTestContext() { DestroyAllWidgets(); }

    virtual bool IsTakingPointerInput( void ) const GUCEF_VIRTUAL_OVERRIDE { return customArbitration ? takesPointer : GUI::CGUIContext::IsTakingPointerInput(); }
    virtual bool IsTakingKeyboardInput( void ) const GUCEF_VIRTUAL_OVERRIDE { return customArbitration ? takesKeyboard : GUI::CGUIContext::IsTakingKeyboardInput(); }

    protected:

    virtual GUI::CWidgetHandler* GetWidgetHandler( const CORE::CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE
    {
        static CTestWidgetHandler handler;
        return &handler;
    }

    virtual bool PreviewMouseMove( Int32 x, Int32 y ) GUCEF_VIRTUAL_OVERRIDE { Log( "preview:move" ); return consumeMove; }
    virtual bool PreviewMouseButton( GUI::TMouseButton button, bool down ) GUCEF_VIRTUAL_OVERRIDE { Log( down ? "preview:down" : "preview:up" ); return consumeButton; }
    virtual bool PreviewMouseWheel( Int32 delta ) GUCEF_VIRTUAL_OVERRIDE { Log( "preview:wheel" ); return consumeWheel; }
    virtual bool PreviewKey( INPUT::KeyCode keyCode, bool down, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE { Log( down ? "preview:keydown" : "preview:keyup" ); return consumeKey; }
    virtual bool PreviewText( const CORE::CString& text ) GUCEF_VIRTUAL_OVERRIDE { Log( "preview:text" ); return consumeText; }

    private:

    void Log( const char* what ) { if ( GUCEF_NULL != previewLog ) previewLog->push_back( what ); }
};

/*-------------------------------------------------------------------------*/

/** An input driver without hardware: the tests feed it input the way a window's input driver does */
class CScriptedDriver : public INPUT::CInputDriver
{
    public:

    virtual const CORE::TVersion* GetVersion( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        static CORE::TVersion version = { 1, 0, 0, 0 };
        return &version;
    }

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestScripted"; }

    void Mouse( const Int32 deviceId ) { AddMouse( deviceId ); }
    void NoMouse( const Int32 deviceId ) { RemoveMouse( deviceId ); }
    void Keyboard( const Int32 deviceId ) { AddKeyboard( deviceId ); }
    void NoKeyboard( const Int32 deviceId ) { RemoveKeyboard( deviceId ); }

    void Move( const Int32 contextId, const Int32 deviceId, const Int32 x, const Int32 y ) { InjectMouseMove( contextId, deviceId, x, y, 0, 0 ); }
    void Button( const Int32 contextId, const Int32 deviceId, const UInt32 button, const bool pressed ) { InjectMouseButtonChange( contextId, deviceId, button, pressed ); }
    void Key( const Int32 contextId, const Int32 deviceId, const INPUT::KeyCode key, const bool pressed ) { InjectKeyboardKeyChange( contextId, deviceId, key, pressed ); }
    void Text( const Int32 contextId, const Int32 deviceId, const CORE::CString& text ) { InjectKeyboardText( contextId, deviceId, text ); }

    protected:

    virtual bool OnUpdate( const UInt64 tickcount, const Float64 updateDeltaInMilliSecs, INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual INPUT::CInputContext* CreateContext( const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW INPUT::CInputContext( *this, params ); }
    virtual void DeleteContext( INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE { GUCEF_DELETE context; }
};

/*-------------------------------------------------------------------------*/

/** The modifiers the last wheel event of a widget carried */
class CWheelModifierObserver : public CORE::CObserver
{
    public:

    UInt32 lastModifiers;
    UInt32 events;

    CWheelModifierObserver( GUI::CWidget* observed ) : CORE::CObserver(), lastModifiers( 0 ), events( 0 ) { SubscribeTo( observed, GUI::CWidget::MouseWheelEvent ); }
    virtual ~CWheelModifierObserver() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        const GUI::CWheelEventData* wheel = CORE::CICloneable::CloneableAs< GUI::CWheelEventData >( eventData );
        if ( GUCEF_NULL != wheel && wheel->source == notifier )
        {
            lastModifiers = wheel->modifiers;
            ++events;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Records the routed events of a widget by name, in order, with the phase left out: the target phase only */
class CWidgetEventLog : public CORE::CObserver
{
    public:

    GUCEF::vector< CORE::CString >* log;
    GUI::CWidget* widget;

    CWidgetEventLog( GUI::CWidget* observed, GUCEF::vector< CORE::CString >* sharedLog ) : CORE::CObserver(), log( sharedLog ), widget( observed )
    {
        SubscribeTo( observed, GUI::CWidget::MouseMoveEvent );
        SubscribeTo( observed, GUI::CWidget::MouseDownEvent );
        SubscribeTo( observed, GUI::CWidget::MouseUpEvent );
        SubscribeTo( observed, GUI::CWidget::MouseWheelEvent );
        SubscribeTo( observed, GUI::CWidget::KeyDownEvent );
        SubscribeTo( observed, GUI::CWidget::KeyUpEvent );
        SubscribeTo( observed, GUI::CWidget::TextInputEvent );
        SubscribeTo( observed, GUI::CWidget::MouseCaptureLostEvent );
    }

    virtual ~CWidgetEventLog() { UnsubscribeAllFromObserver(); }

    UInt32 Count( const char* what ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<(UInt32) log->size(); ++i )
            if ( (*log)[ i ] == what )
                ++count;
        return count;
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        const GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
        if ( GUCEF_NULL != routed && routed->source != notifier )
            return;
        if ( GUI::CWidget::MouseMoveEvent == eventId ) log->push_back( "widget:move" );
        else if ( GUI::CWidget::MouseDownEvent == eventId ) log->push_back( "widget:down" );
        else if ( GUI::CWidget::MouseUpEvent == eventId ) log->push_back( "widget:up" );
        else if ( GUI::CWidget::MouseWheelEvent == eventId ) log->push_back( "widget:wheel" );
        else if ( GUI::CWidget::KeyDownEvent == eventId ) log->push_back( "widget:keydown" );
        else if ( GUI::CWidget::KeyUpEvent == eventId ) log->push_back( "widget:keyup" );
        else if ( GUI::CWidget::TextInputEvent == eventId ) log->push_back( "widget:text" );
        else if ( GUI::CWidget::MouseCaptureLostEvent == eventId ) log->push_back( "widget:capturelost" );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Records what the keyboards of gucefINPUT report as text, one token per event: "D<key>" and "U<key>" for a key
 *  going down and up, followed by "/<modifier bits in decimal>", "T<text>" for typed text. Observes the keyboard
 *  devices it is told about
 */
class CKeyboardRecorder : public CORE::CObserver
{
    public:

    GUCEF::vector< CORE::CString > events;

    CKeyboardRecorder( void ) : CORE::CObserver(), events() {}

    virtual ~CKeyboardRecorder() { UnsubscribeAllFromObserver(); }

    void Watch( Int32 deviceId )
    {
        INPUT::CKeyboard& keyboard = INPUT::CInputGlobal::Instance()->GetInputController().GetKeyboard( deviceId );
        SubscribeTo( &keyboard, INPUT::CKeyboard::KeyStateChangedEvent );
        SubscribeTo( &keyboard, INPUT::CKeyboard::TextInputEvent );
    }

    CORE::CString Joined( void ) const
    {
        CORE::CString result;
        for ( UInt32 i=0; i<(UInt32) events.size(); ++i )
        {
            if ( i > 0 )
                result += " ";
            result += events[ i ];
        }
        return result;
    }

    UInt32 Count( const CORE::CString& token ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<(UInt32) events.size(); ++i )
            if ( events[ i ] == token )
                ++count;
        return count;
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( INPUT::CKeyboard::KeyStateChangedEvent == eventId )
        {
            const INPUT::CKeyStateChangedEventData* data = static_cast< INPUT::CKeyStateChangedEventData* >( eventData );
            events.push_back( CORE::CString( data->GetKeyPressedState() ? "D" : "U" ) + INPUT::KeyCodeToString( data->GetKeyCode() ) + "/" + CORE::UInt32ToString( data->GetKeyModPressedStates() ) );
        }
        else
        if ( INPUT::CKeyboard::TextInputEvent == eventId )
        {
            events.push_back( "T" + static_cast< INPUT::CKeyboardTextEventData* >( eventData )->GetText() );
        }
    }
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_TESTINPUTSUPPORT_H ? */
