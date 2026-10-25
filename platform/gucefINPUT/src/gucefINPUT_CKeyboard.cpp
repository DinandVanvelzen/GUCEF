/*
 *  gucefINPUT: GUCEF module providing input device interaction
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"           /* C++ string utils */
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyModStateChangedEventData.h"
#define GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#include "gucefINPUT_CKeyboardTextEventData.h"
#include "gucefINPUT_CKeyboard.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CKeyboard::KeyStateChangedEvent = "GUCEF::INPUT::CKeyboard::KeyStateChangedEvent";
const CORE::CEvent CKeyboard::KeyModStateChangedEvent = "GUCEF::INPUT::CKeyboard::KeyModStateChangedEvent";
const CORE::CEvent CKeyboard::TextInputEvent = "GUCEF::INPUT::CKeyboard::TextInputEvent";

const char* CKeyboard::DeviceType = "GUCEF::INPUT::CKeyboard";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CKeyboard::RegisterEvents( void )
{GUCEF_TRACE;

    KeyStateChangedEvent.Initialize();
    KeyModStateChangedEvent.Initialize();
    TextInputEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CKeyboard::CKeyboard( CInputDriver& inputDriver ,
                      const UInt32 deviceID     )
    : CAbstractInputDevice()        ,
      m_deviceID( deviceID )        ,
      m_keyState()                  ,
      m_keyModStates( 0 )           ,
      m_inputDriver( &inputDriver )
{GUCEF_TRACE;

    RegisterEvents();
    //ResetKeyboardState();
}

/*-------------------------------------------------------------------------*/

CKeyboard::~CKeyboard()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CKeyboard::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::INPUT::CKeyboard";
    return typeName;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboard::IsKeyDown( const KeyCode key ) const
{GUCEF_TRACE;

    TKeyStates::const_iterator i = m_keyState.find( key );
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboard::IsKeyModifierActive( const KeyModifier keyMod ) const
{GUCEF_TRACE;

    return ( m_keyModStates & keyMod ) > 0;
}

/*-------------------------------------------------------------------------*/

const CKeyboard::TKeyStates&
CKeyboard::GetKeyStates( void ) const
{GUCEF_TRACE;

    return m_keyState;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboard::GetKeyModStates( void ) const
{GUCEF_TRACE;

    return m_keyModStates;
}

/*-------------------------------------------------------------------------*/

CInputDriver*
CKeyboard::GetInputDriver( void )
{GUCEF_TRACE;

    return m_inputDriver;
}

/*-------------------------------------------------------------------------*/

void
CKeyboard::SetKeyState( const Int32 contextId   ,
                        const KeyCode key       ,
                        const bool pressedState )
{GUCEF_TRACE;

    m_keyState[ key ] = pressedState;

    // the modifier keys keep the modifier state, which comes with every key event
    KeyModifier modifier = (KeyModifier) 0;
    bool otherSideDown = false;
    switch ( key )
    {
        case KEYCODE_LSHIFT: modifier = KEYMOD_SHIFT; otherSideDown = IsKeyDown( KEYCODE_RSHIFT ); break;
        case KEYCODE_RSHIFT: modifier = KEYMOD_SHIFT; otherSideDown = IsKeyDown( KEYCODE_LSHIFT ); break;
        case KEYCODE_LCONTROL: modifier = KEYMOD_CTRL; otherSideDown = IsKeyDown( KEYCODE_RCONTROL ); break;
        case KEYCODE_RCONTROL: modifier = KEYMOD_CTRL; otherSideDown = IsKeyDown( KEYCODE_LCONTROL ); break;
        case KEYCODE_LMENU: modifier = KEYMOD_ALT; otherSideDown = IsKeyDown( KEYCODE_RMENU ); break;
        case KEYCODE_RMENU: modifier = KEYMOD_ALT; otherSideDown = IsKeyDown( KEYCODE_LMENU ); break;
        case KEYCODE_LWIN: modifier = KEYMOD_META; otherSideDown = IsKeyDown( KEYCODE_RWIN ); break;
        case KEYCODE_RWIN: modifier = KEYMOD_META; otherSideDown = IsKeyDown( KEYCODE_LWIN ); break;
        default: break;
    }
    if ( 0 != modifier && ( pressedState || !otherSideDown ) && pressedState != IsKeyModifierActive( modifier ) )
        SetKeyModState( modifier, pressedState );

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "Setting state of key [" << KeyCodeToString( key ) << "] to " << CORE::BoolToString( pressedState ) << " on keyboard " << m_deviceID );

    CKeyStateChangedEventData eventData( contextId, m_deviceID, key, pressedState, m_keyModStates );
    NotifyObservers( KeyStateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CKeyboard::SetKeyModState( const KeyModifier keyMod ,
                           const bool pressedState  )
{GUCEF_TRACE;

    if ( pressedState )
        m_keyModStates = m_keyModStates | keyMod;
    else
        m_keyModStates = m_keyModStates & ~( (UInt32) keyMod );

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "Setting state of key mod [" << KeyModifierToString( keyMod ) << "] to " << CORE::BoolToString( pressedState ) << " on keyboard " << m_deviceID );

    CKeyModStateChangedEventData eventData( keyMod, pressedState, m_keyModStates );
    NotifyObservers( KeyModStateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CKeyboard::SetText( const Int32 contextId ,
                    const CString& text   )
{GUCEF_TRACE;

    CKeyboardTextEventData eventData( contextId, m_deviceID, text );
    NotifyObservers( TextInputEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CKeyboard::ResetKeyboardState( void )
{GUCEF_TRACE;

    for ( int i=0; i<KEYCODE_MAX; ++i )
    {
        m_keyState[ (KeyCode) i ] = false;
    }

    m_keyModStates = 0;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboard::GetUnicodeForKeyCode( const KeyCode keyCode     ,
                                 const UInt32 keyModifiers ,
                                 UInt32& unicode           ) const
{GUCEF_TRACE;

    return m_inputDriver->GetUnicodeForKeyCode( keyCode      ,
                                                keyModifiers ,
                                                unicode      );
}

/*-------------------------------------------------------------------------*/

const char*
CKeyboard::GetDeviceType( void ) const
{GUCEF_TRACE;

    return DeviceType;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboard::IsDeviceType( const char* deviceType ) const
{GUCEF_TRACE;

    return ( DeviceType == deviceType ) || ( 0 == strcmp( DeviceType, deviceType ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
