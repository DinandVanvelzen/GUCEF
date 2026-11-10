/*
 *  inputdriverXWINMSG: GUCEF input driver module for X11
 *  Copyright (C) 2002 - 2012.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "inputdriverXWINMSG_CEventListner.h"

#define XK_MISCELLANY
#define XK_TECHNICAL
#define XK_LATIN1
#include "X11/keysymdef.h"
#include <X11/Xutil.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace XWINMSG {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CEventListner::CEventListner( ::Window window                  ,
                              const TInputCallbacks* callbacks )
    : CObserver()               ,
      m_window( window )        ,
      m_callbacks( *callbacks ) ,
      m_mouseX( 0 )             ,
      m_mouseY( 0 )             ,
      m_wheel( 0 )              ,
      m_horizontalWheel( 0 )    ,
      m_movesSinceButtonChange( 0 )   ,
      m_inputMethod( GUCEF_NULL ) ,
      m_inputContext( GUCEF_NULL )
{GUCEF_TRACE;

    CORE::CX11EventDispatcher* eventDispatcher = CORE::CX11EventDispatcher::Instance();
    ::Display* display = eventDispatcher->GetDisplay();

    eventDispatcher->SubscribeOnBehalfOfWindow( *this, window );

    ::XWindowAttributes windowAttributes;
    ::XGetWindowAttributes( display, window, &windowAttributes );

    long eventMask = windowAttributes.your_event_mask |
                          KeyPressMask | KeyReleaseMask |
                          ButtonPressMask | ButtonReleaseMask |
                          PointerMotionMask | Button1MotionMask |
                          EnterWindowMask | LeaveWindowMask | FocusChangeMask;
    ::XSelectInput( display, window, eventMask );

    // an input method makes text in the user's language and layout as UTF-8; without one XLookupString gives Latin-1
    ::XSetLocaleModifiers( "" );
    m_inputMethod = ::XOpenIM( display, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL );
    if ( GUCEF_NULL != m_inputMethod )
    {
        m_inputContext = ::XCreateIC( m_inputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                                      XNClientWindow, window, XNFocusWindow, window, (char*) GUCEF_NULL );
        if ( GUCEF_NULL != m_inputContext )
            ::XSetICFocus( m_inputContext );
    }
}

/*--------------------------------------------------------------------------*/

CEventListner::~CEventListner()
{GUCEF_TRACE;

    CORE::CX11EventDispatcher::Instance()->UnsubscribeOnBehalfOfWindow( *this, m_window );
    if ( GUCEF_NULL != m_inputContext )
        ::XDestroyIC( m_inputContext );
    if ( GUCEF_NULL != m_inputMethod )
        ::XCloseIM( m_inputMethod );
}

/*--------------------------------------------------------------------------*/

::Window
CEventListner::GetWindow( void ) const
{GUCEF_TRACE;

    return m_window;
}

/*--------------------------------------------------------------------------*/

const TInputCallbacks*
CEventListner::GetCallbacks( void ) const
{GUCEF_TRACE;

    return &m_callbacks;
}

/*--------------------------------------------------------------------------*/

GUCEF::INPUT::KeyCode
CEventListner::MapKeyCode( int x11KeyCode )
{GUCEF_TRACE;

    switch ( x11KeyCode )
    {
        // XK_MISCELLANY
        case XK_BackSpace /* Back space, back char */ : return KEYCODE_BACK;
        case XK_Tab : return KEYCODE_TAB;
        case XK_Linefeed /* Linefeed, LF */ : return KEYCODE_UNASSIGNED;
        case XK_Clear : return KEYCODE_UNASSIGNED;
        case XK_Return /* Return, enter */ : return KEYCODE_RETURN;
        case XK_Pause : return KEYCODE_PAUSE;
        case XK_Scroll_Lock : return KEYCODE_SCROLL;
        case XK_Sys_Req : return KEYCODE_SYSRQ;
        case XK_Escape : return KEYCODE_ESCAPE;
        case XK_Delete /* Delete, rubout */ : return KEYCODE_DELETE;

        case XK_F1 : return KEYCODE_F1;
        case XK_F2 : return KEYCODE_F2;
        case XK_F3 : return KEYCODE_F3;
        case XK_F4 : return KEYCODE_F4;
        case XK_F5 : return KEYCODE_F5;
        case XK_F6 : return KEYCODE_F6;
        case XK_F7 : return KEYCODE_F7;
        case XK_F8 : return KEYCODE_F8;
        case XK_F9 : return KEYCODE_F9;
        case XK_F10 : return KEYCODE_F10;
        case XK_F11 : return KEYCODE_F11;
        case XK_F12 : return KEYCODE_F12;
        case XK_F13 : return KEYCODE_F13;
        case XK_F14 : return KEYCODE_F14;
        case XK_F15 : return KEYCODE_F15;

//        case XK_3270_Duplicate : return KEYCODE_UNASSIGNED;
//        case XK_3270_FieldMark : return KEYCODE_UNASSIGNED;
//        case XK_3270_Right2 : return KEYCODE_UNASSIGNED;
//        case XK_3270_Left2 : return KEYCODE_UNASSIGNED;
//        case XK_3270_BackTab : return KEYCODE_TAB;
//        case XK_3270_EraseEOF : return KEYCODE_UNASSIGNED;
//        case XK_3270_EraseInput : return KEYCODE_UNASSIGNED;
//        case XK_3270_Reset : return KEYCODE_UNASSIGNED;
//        case XK_3270_Quit : return KEYCODE_UNASSIGNED;
//        case XK_3270_PA1 : return KEYCODE_UNASSIGNED;
//        case XK_3270_PA2 : return KEYCODE_UNASSIGNED;
//        case XK_3270_PA3 : return KEYCODE_UNASSIGNED;
//        case XK_3270_Test : return KEYCODE_UNASSIGNED;
//        case XK_3270_Attn : return KEYCODE_UNASSIGNED;
//        case XK_3270_CursorBlink : return KEYCODE_UNASSIGNED;
//        case XK_3270_AltCursor : return KEYCODE_UNASSIGNED;
//        case XK_3270_KeyClick : return KEYCODE_UNASSIGNED;
//        case XK_3270_Jump : return KEYCODE_UNASSIGNED;
//        case XK_3270_Ident : return KEYCODE_UNASSIGNED;
//        case XK_3270_Rule : return KEYCODE_UNASSIGNED;
//        case XK_3270_Copy : return KEYCODE_UNASSIGNED;
//        case XK_3270_Play : return KEYCODE_PLAYPAUSE;
//        case XK_3270_Setup : return KEYCODE_UNASSIGNED;
//        case XK_3270_Record : return KEYCODE_UNASSIGNED;
//        case XK_3270_ChangeScreen : return KEYCODE_UNASSIGNED;
//        case XK_3270_DeleteWord : return KEYCODE_UNASSIGNED;
//        case XK_3270_ExSelect : return KEYCODE_UNASSIGNED;
//        case XK_3270_CursorSelect : return KEYCODE_UNASSIGNED;
//        case XK_3270_PrintScreen : return KEYCODE_UNASSIGNED;
//        case XK_3270_Enter : return KEYCODE_RETURN;

        // XK_LATIN1
        case XK_space /* U+0020 SPACE */ : return KEYCODE_SPACE;
        case XK_exclam /* U+0021 EXCLAMATION MARK */ : return KEYCODE_UNASSIGNED;
        case XK_quotedbl /* U+0022 QUOTATION MARK */ : return KEYCODE_UNASSIGNED;
        case XK_numbersign /* U+0023 NUMBER SIGN */ : return KEYCODE_UNASSIGNED;
        case XK_dollar /* U+0024 DOLLAR SIGN */ : return KEYCODE_UNASSIGNED;
        case XK_percent /* U+0025 PERCENT SIGN */ : return KEYCODE_UNASSIGNED;
        case XK_ampersand /* U+0026 AMPERSAND */ : return KEYCODE_UNASSIGNED;
        case XK_apostrophe /* U+0027 APOSTROPHE */ : return KEYCODE_UNASSIGNED;
//        case XK_quoteright /* deprecated */ : return KEYCODE_UNASSIGNED;
        case XK_parenleft /* U+0028 LEFT PARENTHESIS */ : return KEYCODE_UNASSIGNED;
        case XK_parenright /* U+0029 RIGHT PARENTHESIS */ : return KEYCODE_UNASSIGNED;
        case XK_asterisk /* U+002A ASTERISK */ : return KEYCODE_UNASSIGNED;
        case XK_plus /* U+002B PLUS SIGN */ : return KEYCODE_ADD;
        case XK_comma /* U+002C COMMA */ :  return KEYCODE_COMMA;
        case XK_minus /* U+002D HYPHEN-MINUS */ :  return KEYCODE_MINUS;
        case XK_period /* U+002E FULL STOP */ :  return KEYCODE_PERIOD;
        case XK_slash /* U+002F SOLIDUS */ :  return KEYCODE_SLASH;
        case XK_0 /* U+0030 DIGIT ZERO */ : return KEYCODE_0;
        case XK_1 /* U+0031 DIGIT ONE */ : return KEYCODE_1;
        case XK_2 /* U+0032 DIGIT TWO */ : return KEYCODE_2;
        case XK_3 /* U+0033 DIGIT THREE */ : return KEYCODE_3;
        case XK_4 /* U+0034 DIGIT FOUR */ : return KEYCODE_4;
        case XK_5 /* U+0035 DIGIT FIVE */ : return KEYCODE_5;
        case XK_6 /* U+0036 DIGIT SIX */ : return KEYCODE_6;
        case XK_7 /* U+0037 DIGIT SEVEN */ : return KEYCODE_7;
        case XK_8 /* U+0038 DIGIT EIGHT */ : return KEYCODE_8;
        case XK_9 /* U+0039 DIGIT NINE */ : return KEYCODE_9;
        case XK_colon /* U+003A COLON */ : return KEYCODE_COLON;
        case XK_semicolon /* U+003B SEMICOLON */ : return KEYCODE_SEMICOLON;
        case XK_less /* U+003C LESS-THAN SIGN */ : return KEYCODE_UNASSIGNED;
        case XK_equal /* U+003D EQUALS SIGN */ : return KEYCODE_EQUALS;
        case XK_greater /* U+003E GREATER-THAN SIGN */ : return KEYCODE_UNASSIGNED;
        case XK_question /* U+003F QUESTION MARK */ : return KEYCODE_ABNT_C1;
        case XK_at /* U+0040 COMMERCIAL AT */ : return KEYCODE_AT;
        case XK_A /* U+0041 LATIN CAPITAL LETTER A */ : return KEYCODE_A;
        case XK_B /* U+0042 LATIN CAPITAL LETTER B */ : return KEYCODE_B;
        case XK_C /* U+0043 LATIN CAPITAL LETTER C */ : return KEYCODE_C;
        case XK_D /* U+0044 LATIN CAPITAL LETTER D */ : return KEYCODE_D;
        case XK_E /* U+0045 LATIN CAPITAL LETTER E */ : return KEYCODE_E;
        case XK_F /* U+0046 LATIN CAPITAL LETTER F */ : return KEYCODE_F;
        case XK_G /* U+0047 LATIN CAPITAL LETTER G */ : return KEYCODE_G;
        case XK_H /* U+0048 LATIN CAPITAL LETTER H */ : return KEYCODE_H;
        case XK_I /* U+0049 LATIN CAPITAL LETTER I */ : return KEYCODE_I;
        case XK_J /* U+004A LATIN CAPITAL LETTER J */ : return KEYCODE_J;
        case XK_K /* U+004B LATIN CAPITAL LETTER K */ : return KEYCODE_K;
        case XK_L /* U+004C LATIN CAPITAL LETTER L */ : return KEYCODE_L;
        case XK_M /* U+004D LATIN CAPITAL LETTER M */ : return KEYCODE_M;
        case XK_N /* U+004E LATIN CAPITAL LETTER N */ : return KEYCODE_N;
        case XK_O /* U+004F LATIN CAPITAL LETTER O */ : return KEYCODE_O;
        case XK_P /* U+0050 LATIN CAPITAL LETTER P */ : return KEYCODE_P;
        case XK_Q /* U+0051 LATIN CAPITAL LETTER Q */ : return KEYCODE_Q;
        case XK_R /* U+0052 LATIN CAPITAL LETTER R */ : return KEYCODE_R;
        case XK_S /* U+0053 LATIN CAPITAL LETTER S */ : return KEYCODE_S;
        case XK_T /* U+0054 LATIN CAPITAL LETTER T */ : return KEYCODE_T;
        case XK_U /* U+0055 LATIN CAPITAL LETTER U */ : return KEYCODE_U;
        case XK_V /* U+0056 LATIN CAPITAL LETTER V */ : return KEYCODE_V;
        case XK_W /* U+0057 LATIN CAPITAL LETTER W */ : return KEYCODE_W;
        case XK_X /* U+0058 LATIN CAPITAL LETTER X */ : return KEYCODE_X;
        case XK_Y /* U+0059 LATIN CAPITAL LETTER Y */ : return KEYCODE_Y;
        case XK_Z /* U+005A LATIN CAPITAL LETTER Z */ : return KEYCODE_Z;
        case XK_bracketleft /* U+005B LEFT SQUARE BRACKET */ : return KEYCODE_LBRACKET;
        case XK_backslash /* U+005C REVERSE SOLIDUS */ : return KEYCODE_BACKSLASH;
        case XK_bracketright /* U+005D RIGHT SQUARE BRACKET */ : return KEYCODE_RBRACKET;
        case XK_asciicircum /* U+005E CIRCUMFLEX ACCENT */ : return KEYCODE_PREVTRACK;
        case XK_underscore /* U+005F LOW LINE */ : return KEYCODE_UNASSIGNED;
        case XK_grave /* U+0060 GRAVE ACCENT */ :  return KEYCODE_GRAVE;
//        case XK_quoteleft /* deprecated */ : return KEYCODE_UNASSIGNED;
        case XK_a /* U+0061 LATIN SMALL LETTER A */ : return KEYCODE_A;
        case XK_b /* U+0062 LATIN SMALL LETTER B */ : return KEYCODE_B;
        case XK_c /* U+0063 LATIN SMALL LETTER C */ : return KEYCODE_C;
        case XK_d /* U+0064 LATIN SMALL LETTER D */ : return KEYCODE_D;
        case XK_e /* U+0065 LATIN SMALL LETTER E */ : return KEYCODE_E;
        case XK_f /* U+0066 LATIN SMALL LETTER F */ : return KEYCODE_F;
        case XK_g /* U+0067 LATIN SMALL LETTER G */ : return KEYCODE_G;
        case XK_h /* U+0068 LATIN SMALL LETTER H */ : return KEYCODE_H;
        case XK_i /* U+0069 LATIN SMALL LETTER I */ : return KEYCODE_I;
        case XK_j /* U+006A LATIN SMALL LETTER J */ : return KEYCODE_J;
        case XK_k /* U+006B LATIN SMALL LETTER K */ : return KEYCODE_K;
        case XK_l /* U+006C LATIN SMALL LETTER L */ : return KEYCODE_L;
        case XK_m /* U+006D LATIN SMALL LETTER M */ : return KEYCODE_M;
        case XK_n /* U+006E LATIN SMALL LETTER N */ : return KEYCODE_N;
        case XK_o /* U+006F LATIN SMALL LETTER O */ : return KEYCODE_O;
        case XK_p /* U+0070 LATIN SMALL LETTER P */ : return KEYCODE_P;
        case XK_q /* U+0071 LATIN SMALL LETTER Q */ : return KEYCODE_Q;
        case XK_r /* U+0072 LATIN SMALL LETTER R */ : return KEYCODE_R;
        case XK_s /* U+0073 LATIN SMALL LETTER S */ : return KEYCODE_S;
        case XK_t /* U+0074 LATIN SMALL LETTER T */ : return KEYCODE_T;
        case XK_u /* U+0075 LATIN SMALL LETTER U */ : return KEYCODE_U;
        case XK_v /* U+0076 LATIN SMALL LETTER V */ : return KEYCODE_V;
        case XK_w /* U+0077 LATIN SMALL LETTER W */ : return KEYCODE_W;
        case XK_x /* U+0078 LATIN SMALL LETTER X */ : return KEYCODE_X;
        case XK_y /* U+0079 LATIN SMALL LETTER Y */ : return KEYCODE_Y;
        case XK_z /* U+007A LATIN SMALL LETTER Z */ : return KEYCODE_Z;
        case XK_braceleft /* U+007B LEFT CURLY BRACKET */ : return KEYCODE_UNASSIGNED;
        case XK_bar /* U+007C VERTICAL LINE */ : return KEYCODE_UNASSIGNED;
        case XK_braceright /* U+007D RIGHT CURLY BRACKET */ : return KEYCODE_UNASSIGNED;
        case XK_asciitilde /* U+007E TILDE */ : return KEYCODE_UNASSIGNED;

        case XK_leftarrow /* U+2190 LEFTWARDS ARROW */ : return KEYCODE_LEFT;
        case XK_uparrow /* U+2191 UPWARDS ARROW */ : return KEYCODE_UP;
        case XK_rightarrow /* U+2192 RIGHTWARDS ARROW */ : return KEYCODE_RIGHT;
        case XK_downarrow /* U+2193 DOWNWARDS ARROW */ : return KEYCODE_DOWN;
        // cursor control, editing and modifiers
        case XK_Left : return KEYCODE_LEFT;
        case XK_Up : return KEYCODE_UP;
        case XK_Right : return KEYCODE_RIGHT;
        case XK_Down : return KEYCODE_DOWN;
        case XK_Home : return KEYCODE_HOME;
        case XK_End : return KEYCODE_END;
        case XK_Prior : return KEYCODE_PGUP;
        case XK_Next : return KEYCODE_PGDOWN;
        case XK_Insert : return KEYCODE_INSERT;
        case XK_Menu : return KEYCODE_APPS;
        case XK_Print : return KEYCODE_SYSRQ;
        case XK_Shift_L : return KEYCODE_LSHIFT;
        case XK_Shift_R : return KEYCODE_RSHIFT;
        case XK_Control_L : return KEYCODE_LCONTROL;
        case XK_Control_R : return KEYCODE_RCONTROL;
        case XK_Alt_L : return KEYCODE_LMENU;
        case XK_Alt_R : return KEYCODE_RMENU;
        case XK_Super_L : return KEYCODE_LWIN;
        case XK_Super_R : return KEYCODE_RWIN;
        case XK_Caps_Lock : return KEYCODE_CAPITAL;
        case XK_Num_Lock : return KEYCODE_NUMLOCK;
        // the numeric keypad
        case XK_KP_Enter : return KEYCODE_NUMPADENTER;
        case XK_KP_Add : return KEYCODE_ADD;
        case XK_KP_Subtract : return KEYCODE_SUBTRACT;
        case XK_KP_Multiply : return KEYCODE_MULTIPLY;
        case XK_KP_Divide : return KEYCODE_DIVIDE;
        case XK_KP_Decimal : return KEYCODE_DECIMAL;
        case XK_KP_Delete : return KEYCODE_DELETE;
        case XK_KP_Insert : case XK_KP_0 : return KEYCODE_NUMPAD0;
        case XK_KP_End : case XK_KP_1 : return KEYCODE_NUMPAD1;
        case XK_KP_Down : case XK_KP_2 : return KEYCODE_NUMPAD2;
        case XK_KP_Next : case XK_KP_3 : return KEYCODE_NUMPAD3;
        case XK_KP_Left : case XK_KP_4 : return KEYCODE_NUMPAD4;
        case XK_KP_Begin : case XK_KP_5 : return KEYCODE_NUMPAD5;
        case XK_KP_Right : case XK_KP_6 : return KEYCODE_NUMPAD6;
        case XK_KP_Home : case XK_KP_7 : return KEYCODE_NUMPAD7;
        case XK_KP_Up : case XK_KP_8 : return KEYCODE_NUMPAD8;
        case XK_KP_Prior : case XK_KP_9 : return KEYCODE_NUMPAD9;
        default: return KEYCODE_UNASSIGNED;
    }
}

/*--------------------------------------------------------------------------*/

/** Latin-1 text (what XLookupString gives without an input context) as UTF-8 */
static CORE::CString
Latin1ToUtf8( const char* text, int length )
{
    CORE::CString result;
    for ( int i=0; i<length; ++i )
        result.Append( (UInt32) (unsigned char) text[ i ] );
    return result;
}

/*--------------------------------------------------------------------------*/

void
CEventListner::SendText( ::XKeyEvent& keyEvent )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_callbacks.onKeyboardText )
        return;

    // the text the key makes, after the keyboard layout and input method
    char buffer[ 64 ];
    CORE::CString text;
    if ( GUCEF_NULL != m_inputContext )
    {
        ::KeySym keySym = 0;
        Status status = 0;
        int length = ::Xutf8LookupString( m_inputContext, &keyEvent, buffer, (int) sizeof( buffer ) - 1, &keySym, &status );
        if ( ( XLookupChars == status || XLookupBoth == status ) && length > 0 )
        {
            buffer[ length ] = '\0';
            text = buffer;
        }
    }
    else
    {
        int length = ::XLookupString( &keyEvent, buffer, (int) sizeof( buffer ) - 1, GUCEF_NULL, GUCEF_NULL );
        if ( length > 0 )
            text = Latin1ToUtf8( buffer, length );
    }

    // control characters (Return, Backspace, Escape, Tab, Delete) are keys, not text
    if ( text.IsNULLOrEmpty() )
        return;
    const char* bytes = text.C_String();
    if ( 1 == text.ByteSize() - 1 && ( (unsigned char) bytes[ 0 ] < 32 || 127 == (unsigned char) bytes[ 0 ] ) )
        return;
    m_callbacks.onKeyboardText( m_callbacks.userData, 0, bytes );
}

/*--------------------------------------------------------------------------*/

void
CEventListner::OnX11Event( ::XEvent& event )
{GUCEF_TRACE;

    // We only care about input related events
    switch ( event.type )
    {
        case KeyRelease:
        {
            m_callbacks.onKeyboardKeyUp( m_callbacks.userData, 0, MapKeyCode( (int) ::XLookupKeysym( &event.xkey, 0 ) ), 0 );
            return;
        }
        case KeyPress:
        {
            m_callbacks.onKeyboardKeyDown( m_callbacks.userData, 0, MapKeyCode( (int) ::XLookupKeysym( &event.xkey, 0 ) ), 0 );
            SendText( event.xkey );
            return;
        }
        case ButtonPress:
        case ButtonRelease:
        {
            bool pressed = ButtonPress == event.type;
            if ( event.xbutton.button < Button4 || event.xbutton.button > 7 )
            {
                const char* direction = pressed ? " down" : " up";
                GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "X11 input: button " << event.xbutton.button << direction << " at " << event.xbutton.x << "," << event.xbutton.y
                                << ", state " << event.xbutton.state << ", " << m_movesSinceButtonChange << " moves since the last button change" );
                m_movesSinceButtonChange = 0;
            }
            Int32 deltaX = event.xbutton.x - m_mouseX;
            Int32 deltaY = event.xbutton.y - m_mouseY;
            m_mouseX = event.xbutton.x;
            m_mouseY = event.xbutton.y;
            m_callbacks.onMouseMove( m_callbacks.userData, 0, m_mouseX, m_mouseY, deltaX, deltaY );

            // X11 numbers the buttons from 1: left, middle, right, then the wheel (4 up, 5 down, 6 left, 7 right), then the side buttons;
            // gucefINPUT numbers them 0 left, 1 right, 2 middle, 3 and 4 the side buttons
            UInt32 index = 0;
            switch ( event.xbutton.button )
            {
                case Button1: index = 0; break;
                case Button2: index = 2; break;
                case Button3: index = 1; break;
                case Button4:
                case Button5:
                {
                    // a notch of the wheel is a press and a release: count the press only
                    if ( pressed )
                    {
                        Int32 delta = Button4 == event.xbutton.button ? 1 : -1;
                        m_wheel += delta;
                        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "X11 input: wheel notch " << delta << " at " << event.xbutton.x << "," << event.xbutton.y );
                        m_callbacks.onMouseVarChanged( m_callbacks.userData, 0, 0, m_wheel, delta );
                    }
                    return;
                }
                case 6:
                case 7:
                {
                    if ( pressed )
                    {
                        Int32 delta = 7 == event.xbutton.button ? 1 : -1;
                        m_horizontalWheel += delta;
                        m_callbacks.onMouseVarChanged( m_callbacks.userData, 0, 1, m_horizontalWheel, delta );
                    }
                    return;
                }
                default:
                {
                    index = event.xbutton.button >= 8 ? event.xbutton.button - 5 : event.xbutton.button;
                    break;
                }
            }
            if ( pressed )
                m_callbacks.onMouseButtonDown( m_callbacks.userData, 0, index );
            else
                m_callbacks.onMouseButtonUp( m_callbacks.userData, 0, index );
            return;
        }
        case LeaveNotify :
        case EnterNotify :
        {
            const char* movement = EnterNotify == event.type ? "enters" : "leaves";
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "X11 input: the pointer " << movement << " the window at " << event.xcrossing.x << "," << event.xcrossing.y
                            << " (mode " << event.xcrossing.mode << ", detail " << event.xcrossing.detail << ", state " << event.xcrossing.state << ")" );
            Int32 deltaX = event.xcrossing.x - m_mouseX;
            Int32 deltaY = event.xcrossing.y - m_mouseY;

            m_mouseX = event.xcrossing.x;
            m_mouseY = event.xcrossing.y;

            m_callbacks.onMouseMove( m_callbacks.userData, 0, m_mouseX, m_mouseY, deltaX, deltaY );
            return;
        }
        case FocusIn :
        case FocusOut :
        {
            const char* focus = FocusIn == event.type ? "gets" : "loses";
            GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "X11 input: the window " << focus << " the keyboard focus (mode " << event.xfocus.mode << ", detail " << event.xfocus.detail << ")" );
            return;
        }
        case MotionNotify:
        {
            ++m_movesSinceButtonChange;
            Int32 deltaX = event.xmotion.x - m_mouseX;
            Int32 deltaY = event.xmotion.y - m_mouseY;

            m_mouseX = event.xmotion.x;
            m_mouseY = event.xmotion.y;

            m_callbacks.onMouseMove( m_callbacks.userData, 0, m_mouseX, m_mouseY, deltaX, deltaY );
            return;
        }
    }
}

//
//typedef struct {
//	int type;		/* of event */
//	unsigned long serial;	/* # of last request processed by server */
//	Bool send_event;	/* true if this came from a SendEvent request */
//	Display *display;	/* Display the event was read from */
//	Window window;	        /* "event" window it is reported relative to */
//	Window root;	        /* root window that the event occurred on */
//	Window subwindow;	/* child window */
//	Time time;		/* milliseconds */
//	int x, y;		/* pointer x, y coordinates in event window */
//	int x_root, y_root;	/* coordinates relative to root */
//	unsigned int state;	/* key or button mask */
//	unsigned int button;	/* detail */
//	Bool same_screen;	/* same screen flag */
//} XButtonEvent;
//typedef XButtonEvent XButtonPressedEvent;
//typedef XButtonEvent XButtonReleasedEvent;
//
//typedef struct {
//	int type;		/* of event */
//	unsigned long serial;	/* # of last request processed by server */
//	Bool send_event;	/* true if this came from a SendEvent request */
//	Display *display;	/* Display the event was read from */
//	Window window;	        /* "event" window reported relative to */
//	Window root;	        /* root window that the event occurred on */
//	Window subwindow;	/* child window */
//	Time time;		/* milliseconds */
//	int x, y;		/* pointer x, y coordinates in event window */
//	int x_root, y_root;	/* coordinates relative to root */
//	unsigned int state;	/* key or button mask */
//	char is_hint;		/* detail */
//	Bool same_screen;	/* same screen flag */
//} XMotionEvent;
//typedef XMotionEvent XPointerMovedEvent;

/*--------------------------------------------------------------------------*/

void
CEventListner::OnNotify( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventid  ,
                         CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( CORE::CX11EventDispatcher::X11Event == eventid )
    {
        CORE::CX11EventDispatcher::TX11EventData* x11data = (CORE::CX11EventDispatcher::TX11EventData*) eventdata;
        OnX11Event( x11data->GetData() );
    }
    else
    if ( CORE::CX11EventDispatcher::SubscribeEvent == eventid )
    {
        // Register our devices
        m_callbacks.onKeyboardAttached( m_callbacks.userData, 0 );
        m_callbacks.onMouseAttached( m_callbacks.userData, 0 );
    }
    else
    if ( CORE::CX11EventDispatcher::UnsubscribeEvent == eventid )
    {
        // Register our devices
        m_callbacks.onKeyboardDetached( m_callbacks.userData, 0 );
        m_callbacks.onMouseDetached( m_callbacks.userData, 0 );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XWINMSG */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
