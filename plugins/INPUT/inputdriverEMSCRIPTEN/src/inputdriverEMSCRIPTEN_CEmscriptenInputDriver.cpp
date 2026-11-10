/*
 *  inputdriverEMSCRIPTEN: GUCEF input driver for web browsers: mouse, wheel, keys and text of a page
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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#define INPUTDRIVEREMSCRIPTEN_IN_BROWSER
#endif

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTEXT_H
#include "CInputContext.h"
#define GUCEF_INPUT_CINPUTCONTEXT_H
#endif /* GUCEF_INPUT_CINPUTCONTEXT_H ? */

#ifndef GUCEF_INPUT_CMOUSE_H
#include "gucefINPUT_CMouse.h"
#define GUCEF_INPUT_CMOUSE_H
#endif /* GUCEF_INPUT_CMOUSE_H ? */

#include "inputdriverEMSCRIPTEN_CEmscriptenInputDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef INPUTDRIVEREMSCRIPTEN_IN_BROWSER

/* Keeps the browser's menu away from the canvas, the right button belongs to the application */
EM_JS( void, inputdriverEMSCRIPTEN_SetContextMenuBlocked, ( const char* selector, int blocked ), {
    var canvas = document.querySelector( UTF8ToString( selector ) );
    if ( !canvas ) return;
    if ( !canvas.__gucefNoMenu ) canvas.__gucefNoMenu = function( e ) { e.preventDefault(); };
    if ( blocked ) canvas.addEventListener( "contextmenu", canvas.__gucefNoMenu );
    else canvas.removeEventListener( "contextmenu", canvas.__gucefNoMenu );
} );

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUTDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SDomKey
{
    const char* code;
    INPUT::KeyCode key;
};

/** The keys by the "code" of the browser: the position on the keyboard, not what the layout prints on it */
const SDomKey g_domKeys[] =
{
    { "Escape", INPUT::KEYCODE_ESCAPE }, { "Digit1", INPUT::KEYCODE_1 }, { "Digit2", INPUT::KEYCODE_2 }, { "Digit3", INPUT::KEYCODE_3 },
    { "Digit4", INPUT::KEYCODE_4 }, { "Digit5", INPUT::KEYCODE_5 }, { "Digit6", INPUT::KEYCODE_6 }, { "Digit7", INPUT::KEYCODE_7 },
    { "Digit8", INPUT::KEYCODE_8 }, { "Digit9", INPUT::KEYCODE_9 }, { "Digit0", INPUT::KEYCODE_0 }, { "Minus", INPUT::KEYCODE_MINUS },
    { "Equal", INPUT::KEYCODE_EQUALS }, { "Backspace", INPUT::KEYCODE_BACK }, { "Tab", INPUT::KEYCODE_TAB },
    { "KeyQ", INPUT::KEYCODE_Q }, { "KeyW", INPUT::KEYCODE_W }, { "KeyE", INPUT::KEYCODE_E }, { "KeyR", INPUT::KEYCODE_R },
    { "KeyT", INPUT::KEYCODE_T }, { "KeyY", INPUT::KEYCODE_Y }, { "KeyU", INPUT::KEYCODE_U }, { "KeyI", INPUT::KEYCODE_I },
    { "KeyO", INPUT::KEYCODE_O }, { "KeyP", INPUT::KEYCODE_P }, { "BracketLeft", INPUT::KEYCODE_LBRACKET },
    { "BracketRight", INPUT::KEYCODE_RBRACKET }, { "Enter", INPUT::KEYCODE_RETURN }, { "ControlLeft", INPUT::KEYCODE_LCONTROL },
    { "KeyA", INPUT::KEYCODE_A }, { "KeyS", INPUT::KEYCODE_S }, { "KeyD", INPUT::KEYCODE_D }, { "KeyF", INPUT::KEYCODE_F },
    { "KeyG", INPUT::KEYCODE_G }, { "KeyH", INPUT::KEYCODE_H }, { "KeyJ", INPUT::KEYCODE_J }, { "KeyK", INPUT::KEYCODE_K },
    { "KeyL", INPUT::KEYCODE_L }, { "Semicolon", INPUT::KEYCODE_SEMICOLON }, { "Quote", INPUT::KEYCODE_APOSTROPHE },
    { "Backquote", INPUT::KEYCODE_GRAVE }, { "ShiftLeft", INPUT::KEYCODE_LSHIFT }, { "Backslash", INPUT::KEYCODE_BACKSLASH },
    { "KeyZ", INPUT::KEYCODE_Z }, { "KeyX", INPUT::KEYCODE_X }, { "KeyC", INPUT::KEYCODE_C }, { "KeyV", INPUT::KEYCODE_V },
    { "KeyB", INPUT::KEYCODE_B }, { "KeyN", INPUT::KEYCODE_N }, { "KeyM", INPUT::KEYCODE_M }, { "Comma", INPUT::KEYCODE_COMMA },
    { "Period", INPUT::KEYCODE_PERIOD }, { "Slash", INPUT::KEYCODE_SLASH }, { "ShiftRight", INPUT::KEYCODE_RSHIFT },
    { "NumpadMultiply", INPUT::KEYCODE_MULTIPLY }, { "AltLeft", INPUT::KEYCODE_LMENU }, { "Space", INPUT::KEYCODE_SPACE },
    { "CapsLock", INPUT::KEYCODE_CAPITAL }, { "F1", INPUT::KEYCODE_F1 }, { "F2", INPUT::KEYCODE_F2 }, { "F3", INPUT::KEYCODE_F3 },
    { "F4", INPUT::KEYCODE_F4 }, { "F5", INPUT::KEYCODE_F5 }, { "F6", INPUT::KEYCODE_F6 }, { "F7", INPUT::KEYCODE_F7 },
    { "F8", INPUT::KEYCODE_F8 }, { "F9", INPUT::KEYCODE_F9 }, { "F10", INPUT::KEYCODE_F10 }, { "NumLock", INPUT::KEYCODE_NUMLOCK },
    { "ScrollLock", INPUT::KEYCODE_SCROLL }, { "Numpad7", INPUT::KEYCODE_NUMPAD7 }, { "Numpad8", INPUT::KEYCODE_NUMPAD8 },
    { "Numpad9", INPUT::KEYCODE_NUMPAD9 }, { "NumpadSubtract", INPUT::KEYCODE_SUBTRACT }, { "Numpad4", INPUT::KEYCODE_NUMPAD4 },
    { "Numpad5", INPUT::KEYCODE_NUMPAD5 }, { "Numpad6", INPUT::KEYCODE_NUMPAD6 }, { "NumpadAdd", INPUT::KEYCODE_ADD },
    { "Numpad1", INPUT::KEYCODE_NUMPAD1 }, { "Numpad2", INPUT::KEYCODE_NUMPAD2 }, { "Numpad3", INPUT::KEYCODE_NUMPAD3 },
    { "Numpad0", INPUT::KEYCODE_NUMPAD0 }, { "NumpadDecimal", INPUT::KEYCODE_DECIMAL }, { "IntlBackslash", INPUT::KEYCODE_OEM_102 },
    { "F11", INPUT::KEYCODE_F11 }, { "F12", INPUT::KEYCODE_F12 }, { "F13", INPUT::KEYCODE_F13 }, { "F14", INPUT::KEYCODE_F14 },
    { "F15", INPUT::KEYCODE_F15 }, { "NumpadEqual", INPUT::KEYCODE_NUMPADEQUALS }, { "NumpadEnter", INPUT::KEYCODE_NUMPADENTER },
    { "ControlRight", INPUT::KEYCODE_RCONTROL }, { "NumpadComma", INPUT::KEYCODE_NUMPADCOMMA }, { "NumpadDivide", INPUT::KEYCODE_DIVIDE },
    { "PrintScreen", INPUT::KEYCODE_SYSRQ }, { "AltRight", INPUT::KEYCODE_RMENU }, { "Pause", INPUT::KEYCODE_PAUSE },
    { "Home", INPUT::KEYCODE_HOME }, { "ArrowUp", INPUT::KEYCODE_UP }, { "PageUp", INPUT::KEYCODE_PGUP },
    { "ArrowLeft", INPUT::KEYCODE_LEFT }, { "ArrowRight", INPUT::KEYCODE_RIGHT }, { "End", INPUT::KEYCODE_END },
    { "ArrowDown", INPUT::KEYCODE_DOWN }, { "PageDown", INPUT::KEYCODE_PGDOWN }, { "Insert", INPUT::KEYCODE_INSERT },
    { "Delete", INPUT::KEYCODE_DELETE }, { "MetaLeft", INPUT::KEYCODE_LWIN }, { "MetaRight", INPUT::KEYCODE_RWIN },
    { "ContextMenu", INPUT::KEYCODE_APPS }, { "AudioVolumeMute", INPUT::KEYCODE_MUTE }, { "AudioVolumeDown", INPUT::KEYCODE_VOLUMEDOWN },
    { "AudioVolumeUp", INPUT::KEYCODE_VOLUMEUP }, { "MediaPlayPause", INPUT::KEYCODE_PLAYPAUSE }, { "MediaStop", INPUT::KEYCODE_MEDIASTOP },
    { "MediaTrackNext", INPUT::KEYCODE_NEXTTRACK }, { "MediaTrackPrevious", INPUT::KEYCODE_PREVTRACK }
};

/*-------------------------------------------------------------------------*/

/** The number of bytes of the UTF-8 sequence that starts with the given byte, 0 for a byte that cannot start one */
size_t
Utf8SequenceLength( const unsigned char lead )
{
    if ( lead < 0x80 ) return 1;
    if ( ( lead & 0xE0 ) == 0xC0 ) return 2;
    if ( ( lead & 0xF0 ) == 0xE0 ) return 3;
    if ( ( lead & 0xF8 ) == 0xF0 ) return 4;
    return 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The input of one canvas */
class CEmscriptenInputContext : public INPUT::CInputContext
{
    public:

    CEmscriptenInputContext( CEmscriptenInputDriver& driver, const CORE::CValueList& params );

    virtual ~CEmscriptenInputContext();

    bool Listen( void );

    void StopListening( void );

    /** Lets go of the keys and buttons that are held, when the page loses the focus */
    void ReleaseAll( void );

    /* What the browser reports, passed on to the driver */
    void MouseButton( const CORE::UInt32 button, const bool pressed ) { driver.OnMouseButton( *this, button, pressed ); }
    void MouseMove( const CORE::Int32 x, const CORE::Int32 y ) { driver.OnMouseMove( *this, x, y ); }
    void Wheel( const CORE::UInt32 variable, const CORE::Int32 notches ) { driver.OnWheel( *this, variable, notches ); }
    void Key( const INPUT::KeyCode key, const bool pressed ) { driver.OnKey( *this, key, pressed ); }
    void Text( const CORE::CString& utf8Text ) { driver.OnText( *this, utf8Text ); }

    CEmscriptenInputDriver& driver;
    CORE::CString canvas;
    CORE::Int32 lastX;
    CORE::Int32 lastY;
    CORE::Int32 wheel;
    CORE::Int32 horizontalWheel;
    CORE::Float64 wheelRest;
    CORE::Float64 horizontalWheelRest;
    bool buttons[ 5 ];
    bool keys[ INPUT::KEYCODE_MAX ];
    bool listening;
};

/*-------------------------------------------------------------------------*/

#ifdef INPUTDRIVEREMSCRIPTEN_IN_BROWSER

namespace {

/** Mouse positions in pixels of the canvas' drawing buffer, which the page may show larger or smaller */
void
ToCanvasPixels( const CEmscriptenInputContext& context, const double cssX, const double cssY, CORE::Int32& x, CORE::Int32& y )
{
    int width = 0, height = 0;
    double cssWidth = 0.0, cssHeight = 0.0;
    ::emscripten_get_canvas_element_size( context.canvas.C_String(), &width, &height );
    ::emscripten_get_element_css_size( context.canvas.C_String(), &cssWidth, &cssHeight );
    x = (CORE::Int32) ( cssWidth > 0.0 ? cssX * width / cssWidth : cssX );
    y = (CORE::Int32) ( cssHeight > 0.0 ? cssY * height / cssHeight : cssY );
}

/*-------------------------------------------------------------------------*/

EM_BOOL
OnMouseEvent( int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData )
{
    CEmscriptenInputContext& context = *static_cast< CEmscriptenInputContext* >( userData );
    if ( EMSCRIPTEN_EVENT_MOUSEMOVE == eventType )
    {
        CORE::Int32 x = 0, y = 0;
        ToCanvasPixels( context, (double) mouseEvent->targetX, (double) mouseEvent->targetY, x, y );
        context.MouseMove( x, y );
        return EM_FALSE;
    }

    const CORE::UInt32 button = CEmscriptenInputDriver::ButtonFromDomButton( (CORE::UInt32) mouseEvent->button );
    if ( EMSCRIPTEN_EVENT_MOUSEDOWN == eventType )
    {
        // where the button went down, the pointer may not have moved over the canvas before
        CORE::Int32 x = 0, y = 0;
        ToCanvasPixels( context, (double) mouseEvent->targetX, (double) mouseEvent->targetY, x, y );
        if ( x != context.lastX || y != context.lastY )
            context.MouseMove( x, y );
        context.MouseButton( button, true );
        return EM_TRUE;
    }
    if ( EMSCRIPTEN_EVENT_MOUSEUP == eventType )
    {
        // let go anywhere on the page: only buttons that went down on the canvas
        if ( button < 5 && context.buttons[ button ] )
            context.MouseButton( button, false );
        return EM_FALSE;
    }
    return EM_FALSE;
}

/*-------------------------------------------------------------------------*/

EM_BOOL
OnWheelEvent( int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData )
{
    CEmscriptenInputContext& context = *static_cast< CEmscriptenInputContext* >( userData );

    // a notch is 100 pixels in the browsers that report pixels, 3 lines or a page
    double perNotch = 100.0;
    if ( DOM_DELTA_LINE == wheelEvent->deltaMode )
        perNotch = 3.0;
    else
    if ( DOM_DELTA_PAGE == wheelEvent->deltaMode )
        perNotch = 1.0;

    // the browser counts down as positive, gucefINPUT up
    context.wheelRest += -wheelEvent->deltaY / perNotch;
    context.horizontalWheelRest += wheelEvent->deltaX / perNotch;
    const CORE::Int32 notches = (CORE::Int32) context.wheelRest;
    const CORE::Int32 horizontalNotches = (CORE::Int32) context.horizontalWheelRest;
    if ( 0 != notches )
    {
        context.wheelRest -= notches;
        context.Wheel( INPUT::CMouse::MOUSEVARIABLE_WHEEL, notches );
    }
    if ( 0 != horizontalNotches )
    {
        context.horizontalWheelRest -= horizontalNotches;
        context.Wheel( INPUT::CMouse::MOUSEVARIABLE_HORIZONTAL_WHEEL, horizontalNotches );
    }
    return EM_TRUE;
}

/*-------------------------------------------------------------------------*/

EM_BOOL
OnKeyEvent( int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData )
{
    CEmscriptenInputContext& context = *static_cast< CEmscriptenInputContext* >( userData );
    const INPUT::KeyCode key = CEmscriptenInputDriver::KeyCodeFromDomCode( keyEvent->code );
    const bool pressed = EMSCRIPTEN_EVENT_KEYDOWN == eventType;

    // a held key repeats its text, not its press
    if ( INPUT::KEYCODE_UNASSIGNED != key && ( !pressed || !keyEvent->repeat || !context.keys[ key ] ) )
        context.Key( key, pressed );

    // the text of the key: a single character, not a name like "Enter" or "Dead"
    const bool shortcut = keyEvent->ctrlKey || keyEvent->metaKey;
    if ( pressed && !shortcut )
    {
        const size_t length = ::strlen( keyEvent->key );
        if ( length > 0 && length == Utf8SequenceLength( (unsigned char) keyEvent->key[ 0 ] ) &&
             !( 1 == length && ( (unsigned char) keyEvent->key[ 0 ] < 32 || 127 == (unsigned char) keyEvent->key[ 0 ] ) ) )
        {
            context.Text( CORE::CString( keyEvent->key ) );
        }
    }

    // the browser keeps its shortcuts, reload and developer tools
    if ( shortcut || INPUT::KEYCODE_F5 == key || INPUT::KEYCODE_F12 == key )
        return EM_FALSE;
    return EM_TRUE;
}

/*-------------------------------------------------------------------------*/

EM_BOOL
OnFocusEvent( int eventType, const EmscriptenFocusEvent* focusEvent, void* userData )
{
    static_cast< CEmscriptenInputContext* >( userData )->ReleaseAll();
    return EM_FALSE;
}

}

#endif /* INPUTDRIVEREMSCRIPTEN_IN_BROWSER ? */

/*-------------------------------------------------------------------------*/

CEmscriptenInputContext::CEmscriptenInputContext( CEmscriptenInputDriver& inputDriver, const CORE::CValueList& params )
    : INPUT::CInputContext( inputDriver, params )
    , driver( inputDriver )
    , canvas( params.GetValueAlways( "WINDOW", "#canvas" ).AsString() )
    , lastX( 0 )
    , lastY( 0 )
    , wheel( 0 )
    , horizontalWheel( 0 )
    , wheelRest( 0.0 )
    , horizontalWheelRest( 0.0 )
    , listening( false )
{GUCEF_TRACE;

    if ( canvas.IsNULLOrEmpty() )
        canvas = "#canvas";
    memset( buttons, 0, sizeof( buttons ) );
    memset( keys, 0, sizeof( keys ) );
}

/*-------------------------------------------------------------------------*/

CEmscriptenInputContext::~CEmscriptenInputContext()
{GUCEF_TRACE;

    StopListening();
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenInputContext::Listen( void )
{GUCEF_TRACE;

    #ifdef INPUTDRIVEREMSCRIPTEN_IN_BROWSER

    const char* target = canvas.C_String();
    if ( EMSCRIPTEN_RESULT_SUCCESS != ::emscripten_set_mousedown_callback( target, this, EM_TRUE, &OnMouseEvent ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenInputDriver: The page has no canvas \"" + canvas + "\" to read the input of" );
        return false;
    }
    ::emscripten_set_mousemove_callback( target, this, EM_TRUE, &OnMouseEvent );
    ::emscripten_set_wheel_callback( target, this, EM_TRUE, &OnWheelEvent );
    ::emscripten_set_mouseup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &OnMouseEvent );
    ::emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &OnKeyEvent );
    ::emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &OnKeyEvent );
    ::emscripten_set_blur_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &OnFocusEvent );
    inputdriverEMSCRIPTEN_SetContextMenuBlocked( target, 1 );
    listening = true;
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputContext::StopListening( void )
{GUCEF_TRACE;

    #ifdef INPUTDRIVEREMSCRIPTEN_IN_BROWSER

    if ( !listening )
        return;
    const char* target = canvas.C_String();
    ::emscripten_set_mousedown_callback( target, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_mousemove_callback( target, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_wheel_callback( target, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_mouseup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    ::emscripten_set_blur_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, GUCEF_NULL, EM_TRUE, GUCEF_NULL );
    inputdriverEMSCRIPTEN_SetContextMenuBlocked( target, 0 );
    listening = false;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputContext::ReleaseAll( void )
{GUCEF_TRACE;

    for ( CORE::UInt32 i=0; i<5; ++i )
        if ( buttons[ i ] )
            driver.OnMouseButton( *this, i, false );
    for ( CORE::UInt32 i=0; i<(CORE::UInt32)INPUT::KEYCODE_MAX; ++i )
        if ( keys[ i ] )
            driver.OnKey( *this, (INPUT::KeyCode) i, false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEmscriptenInputDriver::DriverName = "EMSCRIPTEN";

/*-------------------------------------------------------------------------*/

CEmscriptenInputDriver::CEmscriptenInputDriver( void )
    : INPUT::CInputDriver()
    , m_contextCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEmscriptenInputDriver::~CEmscriptenInputDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::TVersion*
CEmscriptenInputDriver::GetVersion( void )
{GUCEF_TRACE;

    static CORE::TVersion version = { 1, 0, 0, 0 };
    return &version;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEmscriptenInputDriver::GetName( void ) const
{GUCEF_TRACE;

    return DriverName;
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
CEmscriptenInputDriver::KeyCodeFromDomCode( const char* domCode )
{GUCEF_TRACE;

    if ( GUCEF_NULL == domCode )
        return INPUT::KEYCODE_UNASSIGNED;
    for ( size_t i=0; i<sizeof( g_domKeys ) / sizeof( g_domKeys[ 0 ] ); ++i )
        if ( 0 == ::strcmp( g_domKeys[ i ].code, domCode ) )
            return g_domKeys[ i ].key;
    return INPUT::KEYCODE_UNASSIGNED;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CEmscriptenInputDriver::ButtonFromDomButton( const CORE::UInt32 domButton )
{GUCEF_TRACE;

    // the browser: 0 left, 1 middle, 2 right, 3 back, 4 forward. gucefINPUT: 0 left, 1 right, 2 middle
    switch ( domButton )
    {
        case 1: return 2;
        case 2: return 1;
        default: return domButton;
    }
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenInputDriver::OnUpdate( const CORE::UInt64 tickcount               ,
                                  const CORE::Float64 updateDeltaInMilliSecs ,
                                  INPUT::CInputContext* context              )
{GUCEF_TRACE;

    // the browser calls the event handlers, there is nothing to poll
    return true;
}

/*-------------------------------------------------------------------------*/

INPUT::CInputContext*
CEmscriptenInputDriver::CreateContext( const CORE::CValueList& params )
{GUCEF_TRACE;

    CEmscriptenInputContext* context = GUCEF_NEW CEmscriptenInputContext( *this, params );
    if ( !context->Listen() )
    {
        GUCEF_DELETE context;
        return GUCEF_NULL;
    }
    if ( 0 == m_contextCount++ )
    {
        AddMouse( 0 );
        AddKeyboard( 0 );
    }
    return context;
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::DeleteContext( INPUT::CInputContext* context )
{GUCEF_TRACE;

    CEmscriptenInputContext* emscriptenContext = static_cast< CEmscriptenInputContext* >( context );
    if ( GUCEF_NULL == emscriptenContext )
        return;
    emscriptenContext->StopListening();
    GUCEF_DELETE emscriptenContext;
    if ( m_contextCount > 0 && 0 == --m_contextCount )
    {
        RemoveMouse( 0 );
        RemoveKeyboard( 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::OnMouseButton( CEmscriptenInputContext& context, const CORE::UInt32 button, const bool pressed )
{GUCEF_TRACE;

    if ( button < 5 )
        context.buttons[ button ] = pressed;
    InjectMouseButtonChange( context.GetID(), 0, button, pressed );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::OnMouseMove( CEmscriptenInputContext& context, const CORE::Int32 x, const CORE::Int32 y )
{GUCEF_TRACE;

    const CORE::Int32 deltaX = x - context.lastX;
    const CORE::Int32 deltaY = y - context.lastY;
    context.lastX = x;
    context.lastY = y;
    InjectMouseMove( context.GetID(), 0, x, y, deltaX, deltaY );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::OnWheel( CEmscriptenInputContext& context, const CORE::UInt32 variable, const CORE::Int32 notches )
{GUCEF_TRACE;

    CORE::Int32& value = INPUT::CMouse::MOUSEVARIABLE_WHEEL == variable ? context.wheel : context.horizontalWheel;
    value += notches;
    InjectMouseVariableChange( context.GetID(), 0, variable, value, notches );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::OnKey( CEmscriptenInputContext& context, const INPUT::KeyCode key, const bool pressed )
{GUCEF_TRACE;

    if ( key < INPUT::KEYCODE_MAX )
        context.keys[ key ] = pressed;
    InjectKeyboardKeyChange( context.GetID(), 0, key, pressed );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenInputDriver::OnText( CEmscriptenInputContext& context, const CORE::CString& utf8Text )
{GUCEF_TRACE;

    InjectKeyboardText( context.GetID(), 0, utf8Text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUTDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
