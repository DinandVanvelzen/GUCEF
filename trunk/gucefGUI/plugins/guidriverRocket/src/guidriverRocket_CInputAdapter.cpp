/*
 *  guidriverRocket: GUI backend using Rocket
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#include "guidriverRocket_CInputAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERROCKET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CInputAdapter::CInputAdapter( void )
    : CORE::CObserver()           ,
      m_rocketContext( NULL )     ,
      m_lastKeyModifierState( 0 ) ,
      m_inputContext( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputAdapter::~CInputAdapter()
{GUCEF_TRACE;

    m_rocketContext = NULL;
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::SetRocketContext( Rocket::Core::Context* rocketContext )
{GUCEF_TRACE;

    m_rocketContext = rocketContext;
}

/*-------------------------------------------------------------------------*/

Rocket::Core::Context*
CInputAdapter::GetRocketContext( void )
{GUCEF_TRACE;

    return m_rocketContext;
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::SetInputContext( INPUT::CInputContext* inputContext )
{GUCEF_TRACE;
    
    m_inputContext = inputContext;
}

/*-------------------------------------------------------------------------*/

INPUT::CInputContext*
CInputAdapter::GetInputContext( void ) const
{GUCEF_TRACE;

    return m_inputContext;
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::StartListningForInputEvents( void )
{GUCEF_TRACE;

    StopListningForInputEvents();

    INPUT::CInputController& inputController = INPUT::CInputGlobal::Instance()->GetInputController();
    inputController.SubscribeToAllKeyboards( this );
    inputController.SubscribeToAllMice( this );
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::StopListningForInputEvents( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_lastKeyModifierState = 0;
}

/*-------------------------------------------------------------------------*/

int
CInputAdapter::MapKeyModifierState( const INPUT::UInt32 keyModifierState )
{GUCEF_TRACE;

    int resultKeymodState = 0;
    if ( keyModifierState & INPUT::KEYMOD_SHIFT ) resultKeymodState |= Rocket::Core::Input::KM_SHIFT;
    if ( keyModifierState & INPUT::KEYMOD_CTRL ) resultKeymodState |= Rocket::Core::Input::KM_CTRL;
    if ( keyModifierState & INPUT::KEYMOD_ALT ) resultKeymodState |= Rocket::Core::Input::KM_ALT;
    if ( keyModifierState & INPUT::KEYMOD_META ) resultKeymodState |= Rocket::Core::Input::KM_META;
    if ( keyModifierState & INPUT::KEYMOD_CAPSLOCK ) resultKeymodState |= Rocket::Core::Input::KM_CAPSLOCK;
    if ( keyModifierState & INPUT::KEYMOD_NUMLOCK ) resultKeymodState |= Rocket::Core::Input::KM_NUMLOCK;
    if ( keyModifierState & INPUT::KEYMOD_SCROLLLOCK ) resultKeymodState |= Rocket::Core::Input::KM_SCROLLLOCK;
    return resultKeymodState;
}

/*-------------------------------------------------------------------------*/

Rocket::Core::Input::KeyIdentifier
CInputAdapter::MapKeyIdentifier( const INPUT::KeyCode keyCode )
{
    switch ( keyCode )
    {
        case INPUT::KEYCODE_UNASSIGNED : return Rocket::Core::Input::KI_UNKNOWN;
        case INPUT::KEYCODE_ESCAPE : return Rocket::Core::Input::KI_ESCAPE;
        case INPUT::KEYCODE_1 : return Rocket::Core::Input::KI_1;
        case INPUT::KEYCODE_2 : return Rocket::Core::Input::KI_2;
        case INPUT::KEYCODE_3 : return Rocket::Core::Input::KI_3;
        case INPUT::KEYCODE_4 : return Rocket::Core::Input::KI_4;
        case INPUT::KEYCODE_5 : return Rocket::Core::Input::KI_5;
        case INPUT::KEYCODE_6 : return Rocket::Core::Input::KI_6;
        case INPUT::KEYCODE_7 : return Rocket::Core::Input::KI_7;
        case INPUT::KEYCODE_8 : return Rocket::Core::Input::KI_8;
        case INPUT::KEYCODE_9 : return Rocket::Core::Input::KI_9;
        case INPUT::KEYCODE_0 : return Rocket::Core::Input::KI_0;
        case INPUT::KEYCODE_MINUS : return Rocket::Core::Input::KI_OEM_MINUS;
        case INPUT::KEYCODE_EQUALS : return Rocket::Core::Input::KI_OEM_PLUS;
        case INPUT::KEYCODE_BACK : return Rocket::Core::Input::KI_BACK;
        case INPUT::KEYCODE_TAB : return Rocket::Core::Input::KI_TAB;
        case INPUT::KEYCODE_Q : return Rocket::Core::Input::KI_Q;
        case INPUT::KEYCODE_W : return Rocket::Core::Input::KI_W;
        case INPUT::KEYCODE_E : return Rocket::Core::Input::KI_E;
        case INPUT::KEYCODE_R : return Rocket::Core::Input::KI_R;
        case INPUT::KEYCODE_T : return Rocket::Core::Input::KI_T;
        case INPUT::KEYCODE_Y : return Rocket::Core::Input::KI_Y;
        case INPUT::KEYCODE_U : return Rocket::Core::Input::KI_U;
        case INPUT::KEYCODE_I : return Rocket::Core::Input::KI_I;
        case INPUT::KEYCODE_O : return Rocket::Core::Input::KI_O;
        case INPUT::KEYCODE_P : return Rocket::Core::Input::KI_P;
        case INPUT::KEYCODE_LBRACKET : return Rocket::Core::Input::KI_OEM_4;
        case INPUT::KEYCODE_RBRACKET : return Rocket::Core::Input::KI_OEM_6;
        case INPUT::KEYCODE_RETURN : return Rocket::Core::Input::KI_RETURN;
        case INPUT::KEYCODE_LCONTROL : return Rocket::Core::Input::KI_LCONTROL;
        case INPUT::KEYCODE_A : return Rocket::Core::Input::KI_A;
        case INPUT::KEYCODE_S : return Rocket::Core::Input::KI_S;
        case INPUT::KEYCODE_D : return Rocket::Core::Input::KI_D;
        case INPUT::KEYCODE_F : return Rocket::Core::Input::KI_F;
        case INPUT::KEYCODE_G : return Rocket::Core::Input::KI_G;
        case INPUT::KEYCODE_H : return Rocket::Core::Input::KI_H;
        case INPUT::KEYCODE_J : return Rocket::Core::Input::KI_J;
        case INPUT::KEYCODE_K : return Rocket::Core::Input::KI_K;
        case INPUT::KEYCODE_L : return Rocket::Core::Input::KI_L;
        case INPUT::KEYCODE_SEMICOLON : return Rocket::Core::Input::KI_OEM_1;
        case INPUT::KEYCODE_APOSTROPHE : return Rocket::Core::Input::KI_OEM_7;
        case INPUT::KEYCODE_GRAVE : return Rocket::Core::Input::KI_OEM_7;
        case INPUT::KEYCODE_LSHIFT : return Rocket::Core::Input::KI_LSHIFT;
        case INPUT::KEYCODE_BACKSLASH : return Rocket::Core::Input::KI_OEM_5;
        case INPUT::KEYCODE_Z : return Rocket::Core::Input::KI_Z;
        case INPUT::KEYCODE_X : return Rocket::Core::Input::KI_X;
        case INPUT::KEYCODE_C : return Rocket::Core::Input::KI_C;
        case INPUT::KEYCODE_V : return Rocket::Core::Input::KI_V;
        case INPUT::KEYCODE_B : return Rocket::Core::Input::KI_B;
        case INPUT::KEYCODE_N : return Rocket::Core::Input::KI_N;
        case INPUT::KEYCODE_M : return Rocket::Core::Input::KI_M;
        case INPUT::KEYCODE_COMMA : return Rocket::Core::Input::KI_OEM_COMMA;
        case INPUT::KEYCODE_PERIOD : return Rocket::Core::Input::KI_OEM_PERIOD;
        case INPUT::KEYCODE_SLASH : return Rocket::Core::Input::KI_OEM_2;
        case INPUT::KEYCODE_RSHIFT : return Rocket::Core::Input::KI_RSHIFT;
        case INPUT::KEYCODE_MULTIPLY : return Rocket::Core::Input::KI_MULTIPLY;
        case INPUT::KEYCODE_LMENU : return Rocket::Core::Input::KI_LMENU;
        case INPUT::KEYCODE_SPACE : return Rocket::Core::Input::KI_SPACE;
        case INPUT::KEYCODE_CAPITAL : return Rocket::Core::Input::KI_CAPITAL;
        case INPUT::KEYCODE_F1 : return Rocket::Core::Input::KI_F1;
        case INPUT::KEYCODE_F2 : return Rocket::Core::Input::KI_F2;
        case INPUT::KEYCODE_F3 : return Rocket::Core::Input::KI_F3;
        case INPUT::KEYCODE_F4 : return Rocket::Core::Input::KI_F4;
        case INPUT::KEYCODE_F5 : return Rocket::Core::Input::KI_F5;
        case INPUT::KEYCODE_F6 : return Rocket::Core::Input::KI_F6;
        case INPUT::KEYCODE_F7 : return Rocket::Core::Input::KI_F7;
        case INPUT::KEYCODE_F8 : return Rocket::Core::Input::KI_F8;
        case INPUT::KEYCODE_F9 : return Rocket::Core::Input::KI_F9;
        case INPUT::KEYCODE_F10 : return Rocket::Core::Input::KI_F10;
        case INPUT::KEYCODE_NUMLOCK : return Rocket::Core::Input::KI_NUMLOCK;
        case INPUT::KEYCODE_SCROLL : return Rocket::Core::Input::KI_SCROLL;
        case INPUT::KEYCODE_NUMPAD7 : return Rocket::Core::Input::KI_NUMPAD7;
        case INPUT::KEYCODE_NUMPAD8 : return Rocket::Core::Input::KI_NUMPAD8;
        case INPUT::KEYCODE_NUMPAD9 : return Rocket::Core::Input::KI_NUMPAD9;
        case INPUT::KEYCODE_SUBTRACT : return Rocket::Core::Input::KI_SUBTRACT;
        case INPUT::KEYCODE_NUMPAD4 : return Rocket::Core::Input::KI_NUMPAD4;
        case INPUT::KEYCODE_NUMPAD5 : return Rocket::Core::Input::KI_NUMPAD5;
        case INPUT::KEYCODE_NUMPAD6 : return Rocket::Core::Input::KI_NUMPAD6;
        case INPUT::KEYCODE_ADD : return Rocket::Core::Input::KI_ADD;
        case INPUT::KEYCODE_NUMPAD1 : return Rocket::Core::Input::KI_NUMPAD1;
        case INPUT::KEYCODE_NUMPAD2 : return Rocket::Core::Input::KI_NUMPAD2;
        case INPUT::KEYCODE_NUMPAD3 : return Rocket::Core::Input::KI_NUMPAD3;
        case INPUT::KEYCODE_NUMPAD0 : return Rocket::Core::Input::KI_NUMPAD0;
        case INPUT::KEYCODE_DECIMAL : return Rocket::Core::Input::KI_DECIMAL;
        case INPUT::KEYCODE_LEFTWINKEY : return Rocket::Core::Input::KI_LWIN;
        case INPUT::KEYCODE_RIGHTWINKEY : return Rocket::Core::Input::KI_RWIN;
        case INPUT::KEYCODE_OEM_102 : return Rocket::Core::Input::KI_OEM_102;
        case INPUT::KEYCODE_F11 : return Rocket::Core::Input::KI_F11;
        case INPUT::KEYCODE_F12 : return Rocket::Core::Input::KI_F12;
        case INPUT::KEYCODE_F13 : return Rocket::Core::Input::KI_F13;
        case INPUT::KEYCODE_F14 : return Rocket::Core::Input::KI_F14;
        case INPUT::KEYCODE_F15 : return Rocket::Core::Input::KI_F15;
        case INPUT::KEYCODE_KANA : return Rocket::Core::Input::KI_KANA;
        case INPUT::KEYCODE_ABNT_C1 : return Rocket::Core::Input::KI_OEM_2;
        case INPUT::KEYCODE_CONVERT : return Rocket::Core::Input::KI_CONVERT;
        case INPUT::KEYCODE_NOCONVERT : return Rocket::Core::Input::KI_NONCONVERT;
        //case INPUT::KEYCODE_YEN : return Rocket::Core::Input:: ;
        case INPUT::KEYCODE_ABNT_C2 : return Rocket::Core::Input::KI_OEM_PERIOD;
        case INPUT::KEYCODE_NUMPADEQUALS : return Rocket::Core::Input::KI_OEM_NEC_EQUAL;
        case INPUT::KEYCODE_PREVTRACK : return Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
        // case INPUT::KEYCODE_AT : return Rocket::Core::Input:: ;
        case INPUT::KEYCODE_COLON : return Rocket::Core::Input::KI_OEM_COMMA;
        case INPUT::KEYCODE_UNDERLINE : return Rocket::Core::Input::KI_OEM_MINUS;
        case INPUT::KEYCODE_KANJI : return Rocket::Core::Input::KI_KANJI;
        case INPUT::KEYCODE_STOP : return Rocket::Core::Input::KI_MEDIA_STOP;
        case INPUT::KEYCODE_AX : return Rocket::Core::Input::KI_OEM_AX;
        // case INPUT::KEYCODE_UNLABELED : return Rocket::Core::Input:: ;
        case INPUT::KEYCODE_NEXTTRACK : return Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
        case INPUT::KEYCODE_NUMPADENTER : return Rocket::Core::Input::KI_NUMPADENTER;
        case INPUT::KEYCODE_RCONTROL : return Rocket::Core::Input::KI_RCONTROL;
        case INPUT::KEYCODE_MUTE : return Rocket::Core::Input::KI_VOLUME_MUTE;
        // case INPUT::KEYCODE_CALCULATOR : return Rocket::Core::Input:: ;
        case INPUT::KEYCODE_PLAYPAUSE : return Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
        case INPUT::KEYCODE_MEDIASTOP : return Rocket::Core::Input::KI_MEDIA_STOP;
        case INPUT::KEYCODE_VOLUMEDOWN : return Rocket::Core::Input::KI_VOLUME_DOWN;
        case INPUT::KEYCODE_VOLUMEUP : return Rocket::Core::Input::KI_VOLUME_UP;
        case INPUT::KEYCODE_WEBHOME : return Rocket::Core::Input::KI_BROWSER_HOME;
        case INPUT::KEYCODE_NUMPADCOMMA : return Rocket::Core::Input::KI_DECIMAL;
        case INPUT::KEYCODE_DIVIDE : return Rocket::Core::Input::KI_DIVIDE;
        case INPUT::KEYCODE_SYSRQ : return Rocket::Core::Input::KI_SNAPSHOT;
        case INPUT::KEYCODE_RMENU : return Rocket::Core::Input::KI_RMENU;
        case INPUT::KEYCODE_PAUSE : return Rocket::Core::Input::KI_PAUSE;
        case INPUT::KEYCODE_HOME : return Rocket::Core::Input::KI_HOME;
        case INPUT::KEYCODE_UP : return Rocket::Core::Input::KI_UP;
        case INPUT::KEYCODE_PGUP : return Rocket::Core::Input::KI_PRIOR;
        case INPUT::KEYCODE_LEFT : return Rocket::Core::Input::KI_LEFT;
        case INPUT::KEYCODE_RIGHT : return Rocket::Core::Input::KI_RIGHT;
        case INPUT::KEYCODE_END : return Rocket::Core::Input::KI_END;
        case INPUT::KEYCODE_DOWN : return Rocket::Core::Input::KI_DOWN;
        case INPUT::KEYCODE_PGDOWN : return Rocket::Core::Input::KI_NEXT;
        case INPUT::KEYCODE_INSERT : return Rocket::Core::Input::KI_INSERT;
        case INPUT::KEYCODE_DELETE : return Rocket::Core::Input::KI_DELETE;
        case INPUT::KEYCODE_LWIN : return Rocket::Core::Input::KI_LWIN;
        case INPUT::KEYCODE_RWIN : return Rocket::Core::Input::KI_RWIN;
        case INPUT::KEYCODE_APPS : return Rocket::Core::Input::KI_APPS;
        case INPUT::KEYCODE_POWER : return Rocket::Core::Input::KI_POWER;
        case INPUT::KEYCODE_SLEEP : return Rocket::Core::Input::KI_SLEEP;
        case INPUT::KEYCODE_WAKE : return Rocket::Core::Input::KI_WAKE;
        case INPUT::KEYCODE_WEBSEARCH : return Rocket::Core::Input::KI_BROWSER_SEARCH;
        case INPUT::KEYCODE_WEBFAVORITES : return Rocket::Core::Input::KI_BROWSER_FAVORITES;
        case INPUT::KEYCODE_WEBREFRESH : return Rocket::Core::Input::KI_BROWSER_REFRESH;
        case INPUT::KEYCODE_WEBSTOP : return Rocket::Core::Input::KI_BROWSER_STOP;
        case INPUT::KEYCODE_WEBFORWARD : return Rocket::Core::Input::KI_BROWSER_FORWARD;
        case INPUT::KEYCODE_WEBBACK : return Rocket::Core::Input::KI_BROWSER_BACK;
        case INPUT::KEYCODE_MYCOMPUTER : return Rocket::Core::Input::KI_LAUNCH_APP1;
        case INPUT::KEYCODE_MAIL : return Rocket::Core::Input::KI_LAUNCH_MAIL;
        case INPUT::KEYCODE_MEDIASELECT : return Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;
        default :
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GUIDRIVERROCKET::CInputAdapter: Unable to map keyCode " + CORE::Int32ToString( keyCode ) );
            return Rocket::Core::Input::KI_UNKNOWN;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::OnMouseEvent( INPUT::CMouse* mouse         ,
                             const CORE::CEvent& eventid  ,
                             CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid == INPUT::CMouse::MouseButtonEvent )
    {
        // send when a mouse button is pressed or released
        INPUT::CMouseButtonEventData* mouseEventData = static_cast< INPUT::CMouseButtonEventData* >( eventdata );
        if ( m_inputContext->GetID() == mouseEventData->GetContextId() )
        {
            if ( mouseEventData->GetPressedState() )
            {
                m_rocketContext->ProcessMouseButtonDown( mouseEventData->GetButtonIndex(), m_lastKeyModifierState );
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "GUIDRIVERROCKET::CInputAdapter: injected mouse button down buttonIndex=" + CORE::Int32ToString( mouseEventData->GetButtonIndex() ) +
                                                                                                                        " key modifier states=" + CORE::Int32ToString( m_lastKeyModifierState )   );
            }
            else
            {
                m_rocketContext->ProcessMouseButtonUp( mouseEventData->GetButtonIndex(), m_lastKeyModifierState );
            }
        }
    }
    else
    if ( eventid == INPUT::CMouse::MouseMovedEvent )
    {
        // send when the mouse moved
        INPUT::CMouseMovedEventData* mouseEventData = static_cast< INPUT::CMouseMovedEventData* >( eventdata );
        if ( m_inputContext->GetID() == mouseEventData->GetContextId() )
        {
            m_rocketContext->ProcessMouseMove( mouseEventData->GetXPos(), mouseEventData->GetYPos(), m_lastKeyModifierState );
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "GUIDRIVERROCKET::CInputAdapter: injected mouse move x=" + CORE::Int32ToString( mouseEventData->GetXPos() )                 +
                                                                                                             " y=" + CORE::Int32ToString( mouseEventData->GetYPos() )                 +
                                                                                                             " key modifier states=" + CORE::UInt32ToString( m_lastKeyModifierState ) );
        }
    }
    else
    if ( eventid == INPUT::CMouse::MouseClickedEvent )
    {
        // send when a mouse button is clicked and released within a set interval
    }
    else
    if ( eventid == INPUT::CMouse::MouseDoubleClickedEvent )
    {
        // send when a mouse button is clicked and released twice within a set interval
    }
    else
    if ( eventid == INPUT::CMouse::MouseTrippleClickedEvent )
    {
        // send when a mouse button is clicked and released trice within a set interval
    }
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::OnKeyboardEvent( INPUT::CKeyboard* keyboard   ,
                                const CORE::CEvent& eventid  ,
                                CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( eventid == INPUT::CKeyboard::KeyStateChangedEvent )
    {
        // event signaling that the state of a key changed
        INPUT::CKeyStateChangedEventData* keyboardEventData = static_cast< INPUT::CKeyStateChangedEventData* >( eventdata );
        m_lastKeyModifierState = MapKeyModifierState( keyboardEventData->GetKeyModPressedStates() );
        Rocket::Core::Input::KeyIdentifier rocketKeyIdentifier = MapKeyIdentifier( keyboardEventData->GetKeyCode() );

        if ( keyboardEventData->GetKeyPressedState() )
        {
            m_rocketContext->ProcessKeyDown( rocketKeyIdentifier, m_lastKeyModifierState );
        }
        else
        {
            m_rocketContext->ProcessKeyUp( rocketKeyIdentifier, m_lastKeyModifierState );
        }
    }
    else
    if ( eventid == INPUT::CKeyboard::KeyModStateChangedEvent )
    {
        // event signaling that the state of a key modifier changed
        INPUT::CKeyModStateChangedEventData* keyboardEventData = static_cast< INPUT::CKeyModStateChangedEventData* >( eventdata );
        m_lastKeyModifierState = MapKeyModifierState( keyboardEventData->GetAllKeyModStates() );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputAdapter::OnNotify( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventid  ,
                         CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( NULL != m_rocketContext )
    {
        INPUT::CAbstractInputDevice* inputDevice = static_cast< INPUT::CAbstractInputDevice* >( notifier );
        if ( inputDevice->IsDeviceType( INPUT::CMouse::DeviceType ) )
        {
            OnMouseEvent( static_cast< INPUT::CMouse* >( notifier ) ,
                          eventid                                   ,
                          eventdata                                 );
        }
        else
        if ( inputDevice->IsDeviceType( INPUT::CKeyboard::DeviceType ) )
        {
            OnKeyboardEvent( static_cast< INPUT::CKeyboard* >( notifier ) ,
                             eventid                                      ,
                             eventdata                                    );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERROCKET */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
