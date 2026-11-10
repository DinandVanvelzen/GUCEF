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

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#include "gucefINPUT_CInputDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CInputDriver::CInputDriver( void )
    : m_controller( &CInputGlobal::Instance()->GetInputController() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputDriver::~CInputDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CInputDriver::InjectMouseButtonChange( const Int32 contextId    ,
                                       const Int32 deviceIndex  ,
                                       const UInt32 buttonIndex ,
                                       const bool buttonPressed )
{GUCEF_TRACE;

    m_controller->SetMouseButtonState( contextId     ,
                                       deviceIndex   ,
                                       buttonIndex   ,
                                       buttonPressed );
}

/*-------------------------------------------------------------------------*/

void
CInputDriver::InjectMouseMove( const Int32 contextId   ,
                               const Int32 deviceIndex ,
                               const Int32 xPos        ,
                               const Int32 yPos        ,
                               const Int32 xDelta      ,
                               const Int32 yDelta      )
{GUCEF_TRACE;

    m_controller->SetMousePos( contextId   ,
                               deviceIndex ,
                               xPos        ,
                               yPos        ,
                               xDelta      ,
                               yDelta      );
}

/*-------------------------------------------------------------------------*/

void
CInputDriver::InjectKeyboardKeyChange( const Int32 contextId   ,
                                       const Int32 deviceIndex ,
                                       const KeyCode keyCode   ,
                                       const bool keyPressed   )
{GUCEF_TRACE;

    m_controller->SetKeyboardKeyState( contextId   ,
                                       deviceIndex ,
                                       keyCode     ,
                                       keyPressed  );    
}

/*-------------------------------------------------------------------------*/

void
CInputDriver::InjectMouseVariableChange( const Int32 contextId      ,
                                         const Int32 deviceIndex    ,
                                         const UInt32 variableIndex ,
                                         const Int32 value          ,
                                         const Int32 valueDelta     )
{GUCEF_TRACE;

    m_controller->SetMouseVariable( contextId, deviceIndex, variableIndex, value, valueDelta );
}

/*-------------------------------------------------------------------------*/

void
CInputDriver::InjectKeyboardText( const Int32 contextId   ,
                                  const Int32 deviceIndex ,
                                  const CString& utf8Text )
{GUCEF_TRACE;

    m_controller->SetKeyboardText( contextId, deviceIndex, utf8Text );
}

/*-------------------------------------------------------------------------*/

void
CInputDriver::AddMouse( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->AddMouse( this, deviceID );
}

/*-------------------------------------------------------------------------*/
    
void
CInputDriver::RemoveMouse( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->RemoveMouse( deviceID );
}

/*-------------------------------------------------------------------------*/
    
void
CInputDriver::AddKeyboard( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->AddKeyboard( this, deviceID );
}

/*-------------------------------------------------------------------------*/
    
void
CInputDriver::RemoveKeyboard( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->RemoveKeyboard( deviceID );
}

/*-------------------------------------------------------------------------*/
    
void
CInputDriver::AddDevice( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->AddDevice( this, deviceID );
}

/*-------------------------------------------------------------------------*/
    
void
CInputDriver::RemoveDevice( const Int32 deviceID )
{GUCEF_TRACE;

    m_controller->RemoveDevice( deviceID );
}

/*-------------------------------------------------------------------------*/

bool
CInputDriver::GetUnicodeForKeyCode( const KeyCode keyCode     ,
                                    const UInt32 keyModifiers ,
                                    UInt32& unicode           ) const
{GUCEF_TRACE;

    // Very basic implementation for a unicode conversion
    // Descending classes can override with better versions if so desired
    // A possible enhancement would be localization by using a different 
    // character set and supporting all keys

    // First we check to see if the key code given is that of a character
    if ( ( ( keyCode >= KEYCODE_Q ) && ( keyCode <= KEYCODE_P ) ) ||
         ( ( keyCode >= KEYCODE_A ) && ( keyCode <= KEYCODE_L ) ) ||
         ( ( keyCode >= KEYCODE_Z ) && ( keyCode <= KEYCODE_M ) )  )
    {
        // Now that we know we are dealing with a character we can apply
        // the shift state to the character
        bool isUppercase = keyModifiers & GUCEF::INPUT::KEYMOD_SHIFT;
        
        switch ( keyCode )
        {
            // Note that the unicode values used come from the 'Basic-Latin' character set
            case KEYCODE_Q : { if ( isUppercase ) unicode = 0x51; else unicode = 0x71; return true; }
            case KEYCODE_W : { if ( isUppercase ) unicode = 0x57; else unicode = 0x77; return true; }
            case KEYCODE_E : { if ( isUppercase ) unicode = 0x45; else unicode = 0x65; return true; }
            case KEYCODE_R : { if ( isUppercase ) unicode = 0x52; else unicode = 0x72; return true; }
            case KEYCODE_T : { if ( isUppercase ) unicode = 0x54; else unicode = 0x74; return true; }
            case KEYCODE_Y : { if ( isUppercase ) unicode = 0x59; else unicode = 0x79; return true; }
            case KEYCODE_U : { if ( isUppercase ) unicode = 0x55; else unicode = 0x75; return true; }
            case KEYCODE_I : { if ( isUppercase ) unicode = 0x49; else unicode = 0x69; return true; }
            case KEYCODE_O : { if ( isUppercase ) unicode = 0x4F; else unicode = 0x6F; return true; }
            case KEYCODE_P : { if ( isUppercase ) unicode = 0x50; else unicode = 0x70; return true; }
            case KEYCODE_A : { if ( isUppercase ) unicode = 0x41; else unicode = 0x61; return true; }
            case KEYCODE_S : { if ( isUppercase ) unicode = 0x53; else unicode = 0x73; return true; }
            case KEYCODE_D : { if ( isUppercase ) unicode = 0x44; else unicode = 0x64; return true; }
            case KEYCODE_F : { if ( isUppercase ) unicode = 0x46; else unicode = 0x66; return true; }
            case KEYCODE_G : { if ( isUppercase ) unicode = 0x47; else unicode = 0x67; return true; }
            case KEYCODE_H : { if ( isUppercase ) unicode = 0x48; else unicode = 0x68; return true; }
            case KEYCODE_J : { if ( isUppercase ) unicode = 0x4A; else unicode = 0x6A; return true; }
            case KEYCODE_K : { if ( isUppercase ) unicode = 0x4B; else unicode = 0x6B; return true; }
            case KEYCODE_L : { if ( isUppercase ) unicode = 0x4C; else unicode = 0x6C; return true; }
            case KEYCODE_Z : { if ( isUppercase ) unicode = 0x5A; else unicode = 0x7A; return true; }
            case KEYCODE_X : { if ( isUppercase ) unicode = 0x58; else unicode = 0x78; return true; }
            case KEYCODE_C : { if ( isUppercase ) unicode = 0x43; else unicode = 0x63; return true; }
            case KEYCODE_V : { if ( isUppercase ) unicode = 0x56; else unicode = 0x76; return true; }
            case KEYCODE_B : { if ( isUppercase ) unicode = 0x42; else unicode = 0x62; return true; }
            case KEYCODE_N : { if ( isUppercase ) unicode = 0x4E; else unicode = 0x6E; return true; }
            case KEYCODE_M : { if ( isUppercase ) unicode = 0x4D; else unicode = 0x6D; return true; }
            default: { unicode = 0; return false; } // <- we should not get here
        }
    }
    
    // Try the symbols
    // Note that the unicode values used come from the 'Basic-Latin' character set
    switch ( keyCode )
    {
        case KEYCODE_0 : unicode = 0x30; return true;
        case KEYCODE_1 : unicode = 0x31; return true;
        case KEYCODE_2 : unicode = 0x32; return true;
        case KEYCODE_3 : unicode = 0x33; return true;
        case KEYCODE_4 : unicode = 0x34; return true;
        case KEYCODE_5 : unicode = 0x35; return true;
        case KEYCODE_6 : unicode = 0x36; return true;
        case KEYCODE_7 : unicode = 0x37; return true;
        case KEYCODE_8 : unicode = 0x38; return true;
        case KEYCODE_9 : unicode = 0x39; return true;
        case KEYCODE_SPACE : unicode = 0x20; return true;
        default: { unicode = 0; return false; }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
