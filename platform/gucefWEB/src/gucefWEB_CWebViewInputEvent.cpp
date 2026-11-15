/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWKEYCODES_H
#include "gucefWEB_CWebViewKeyCodes.h"
#define GUCEF_WEB_CWEBVIEWKEYCODES_H
#endif /* GUCEF_WEB_CWEBVIEWKEYCODES_H ? */

#include "gucefWEB_CWebViewInputEvent.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewInputEvent::CWebViewInputEvent( void )
    : kind( WEBVIEWINPUT_NONE )
    , x( 0 )
    , y( 0 )
    , button( WEBMOUSEBUTTON_NONE )
    , buttonStates( 0 )
    , clickCount( 0 )
    , wheelNotchesX( 0.0f )
    , wheelNotchesY( 0.0f )
    , wheelPixelsX( 0 )
    , wheelPixelsY( 0 )
    , modifiers( 0 )
    , code( WEBKEYCODE_UNIDENTIFIED )
    , namedKey( WEBNAMEDKEY_UNIDENTIFIED )
    , isRepeat( false )
    , text()
    , timestampMs( 0 )
{GUCEF_TRACE;

    keyText[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

bool
CWebViewInputEvent::IsPointerEvent( void ) const
{GUCEF_TRACE;

    return WEBVIEWINPUT_POINTER_MOVE == kind || WEBVIEWINPUT_POINTER_DOWN == kind || WEBVIEWINPUT_POINTER_UP == kind ||
           WEBVIEWINPUT_POINTER_LEAVE == kind || WEBVIEWINPUT_WHEEL == kind;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewInputEvent::IsKeyEvent( void ) const
{GUCEF_TRACE;

    return WEBVIEWINPUT_KEY_DOWN == kind || WEBVIEWINPUT_KEY_UP == kind;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewInputEvent::IsCharacterKey( void ) const
{GUCEF_TRACE;

    return IsKeyEvent() && WEBNAMEDKEY_CHARACTER == namedKey && '\0' != keyText[ 0 ];
}

/*-------------------------------------------------------------------------*/

void
CWebViewInputEvent::SetKeyText( const char* utf8Text )
{GUCEF_TRACE;

    UInt32 length = 0;
    if ( GUCEF_NULL != utf8Text )
    {
        while ( '\0' != utf8Text[ length ] && length < (UInt32) KEY_TEXT_CAPACITY - 1 )
            ++length;
        // a cut never splits a character: continuation bytes of UTF-8 are 10xxxxxx
        if ( '\0' != utf8Text[ length ] )
            while ( length > 0 && 0x80 == ( ( (UInt8) utf8Text[ length ] ) & 0xC0 ) )
                --length;
        for ( UInt32 i=0; i<length; ++i )
            keyText[ i ] = utf8Text[ i ];
    }
    keyText[ length ] = '\0';
}

/*-------------------------------------------------------------------------*/

CString
CWebViewInputEvent::Describe( bool includeText ) const
{GUCEF_TRACE;

    CString text = WebViewInputKindToString( kind );
    if ( IsPointerEvent() && WEBVIEWINPUT_POINTER_LEAVE != kind )
        text += " at " + CORE::Int32ToString( x ) + "," + CORE::Int32ToString( y );
    if ( WEBVIEWINPUT_POINTER_DOWN == kind || WEBVIEWINPUT_POINTER_UP == kind )
        text += " button " + CORE::Int32ToString( (Int32) button ) + " clicks " + CORE::UInt32ToString( clickCount );
    if ( WEBVIEWINPUT_WHEEL == kind )
        text += " notches " + CORE::FloatToString( wheelNotchesX ) + "," + CORE::FloatToString( wheelNotchesY );
    if ( IsKeyEvent() )
    {
        // the code of the key is no secret, its character is text typed into the page
        text += CString( " code " ) + CWebViewKeyCodes::GetDomCodeName( code );
        if ( WEBNAMEDKEY_CHARACTER == namedKey )
            text += includeText ? CString( " key \"" ) + keyText + "\"" : CString( " key (a character)" );
        else
            text += CString( " key " ) + CWebViewKeyCodes::GetDomKeyName( namedKey );
        if ( isRepeat )
            text += " (repeat)";
    }
    if ( 0 != modifiers )
        text += " modifiers " + CORE::UInt32ToString( modifiers );
    if ( WEBVIEWINPUT_TEXT == kind )
        text += includeText ? " \"" + this->text + "\"" : " (" + CORE::UInt32ToString( this->text.Length() ) + " characters)";
    return text;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::PointerMove( Int32 x, Int32 y, UInt32 modifiers, UInt32 buttonStates )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = WEBVIEWINPUT_POINTER_MOVE;
    e.x = x;
    e.y = y;
    e.modifiers = modifiers;
    e.buttonStates = buttonStates;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::PointerButton( Int32 x, Int32 y, TWebMouseButton button, bool down, UInt32 clickCount, UInt32 modifiers )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = down ? WEBVIEWINPUT_POINTER_DOWN : WEBVIEWINPUT_POINTER_UP;
    e.x = x;
    e.y = y;
    e.button = button;
    e.clickCount = clickCount;
    e.modifiers = modifiers;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::PointerLeave( void )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = WEBVIEWINPUT_POINTER_LEAVE;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::Wheel( Int32 x, Int32 y, Float32 notchesX, Float32 notchesY, UInt32 modifiers )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = WEBVIEWINPUT_WHEEL;
    e.x = x;
    e.y = y;
    e.wheelNotchesX = notchesX;
    e.wheelNotchesY = notchesY;
    e.modifiers = modifiers;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::Key( TWebKeyCode code, TWebNamedKey namedKey, const char* keyText, bool down, UInt32 modifiers, bool isRepeat )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = down ? WEBVIEWINPUT_KEY_DOWN : WEBVIEWINPUT_KEY_UP;
    e.code = code;
    e.namedKey = namedKey;
    if ( WEBNAMEDKEY_CHARACTER == namedKey )
        e.SetKeyText( keyText );
    e.modifiers = modifiers;
    e.isRepeat = isRepeat;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::UsLayoutKey( TWebKeyCode code, bool down, UInt32 modifiers, bool isRepeat )
{GUCEF_TRACE;

    const char* keyText = GUCEF_NULL;
    const TWebNamedKey namedKey = CWebViewKeyCodes::GetUsLayoutKey( code, modifiers, keyText );
    return Key( code, namedKey, keyText, down, modifiers, isRepeat );
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::Text( const CString& text )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = WEBVIEWINPUT_TEXT;
    e.text = text;
    return e;
}

/*-------------------------------------------------------------------------*/

CWebViewInputEvent
CWebViewInputEvent::Focus( bool focusIn )
{GUCEF_TRACE;

    CWebViewInputEvent e;
    e.kind = focusIn ? WEBVIEWINPUT_FOCUS_IN : WEBVIEWINPUT_FOCUS_OUT;
    return e;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
