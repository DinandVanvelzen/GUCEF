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

#ifndef GUCEF_WEB_CWEBVIEWKEYCODES_H
#define GUCEF_WEB_CWEBVIEWKEYCODES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWINPUTTYPES_H
#include "gucefWEB_WebViewInputTypes.h"
#define GUCEF_WEB_WEBVIEWINPUTTYPES_H
#endif /* GUCEF_WEB_WEBVIEWINPUTTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Constant lookup tables of the web standard input values (gucefWEB_WebViewInputTypes.h), indexed by the value: no
 *  string compares, no allocation, any thread. They give what engines, a remote view server or a test want for a key
 *  and only when they want it:
 *  - the DOM names of key codes and named keys ("KeyA", "ArrowLeft"), for engines driven by name (a DevTools protocol);
 *  - the location of a key (KeyboardEvent.location);
 *  - the Linux evdev code of the physical key, and the X11 hardware key code (evdev code + 8) of an evdev keyboard;
 *  - the Windows scan code (set 1, 0xE0xx for an extended key) and the Windows virtual key of the key on the US layout.
 *  These are engine neutral conversions of the physical key: the keysyms of X11 or GDK depend on the keyboard layout
 *  and are left to the engine that needs them, which has the key value (named key or text) for that.
 *  - the key value of a key on the US layout, with shift and caps lock: a fallback for sources that only know the
 *    physical key. A source that knows the layout (a GUI with text input) sends its own key values and text.
 *  A value of 0 in a code table means the key has no such code (it is not on those keyboards).
 *  The reverse lookups by name are searches, for settings, protocols and tests, not for the input path.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewKeyCodes
{
    public:

    /** The DOM KeyboardEvent.code name: "KeyA", "Enter"; "Unidentified" for WEBKEYCODE_UNIDENTIFIED or a value out of range */
    static const char* GetDomCodeName( TWebKeyCode code );

    /** The key code of a DOM code name (case sensitive, as in the DOM); WEBKEYCODE_UNIDENTIFIED for an unknown name */
    static TWebKeyCode GetKeyCodeFromDomCodeName( const char* domCodeName );

    /** The DOM KeyboardEvent.key name: "Enter", "ArrowLeft"; "" for WEBNAMEDKEY_CHARACTER, "Unidentified" out of range */
    static const char* GetDomKeyName( TWebNamedKey key );

    /** The named key of a DOM key name (case sensitive); WEBNAMEDKEY_UNIDENTIFIED for an unknown name or a character */
    static TWebNamedKey GetNamedKeyFromDomKeyName( const char* domKeyName );

    static TWebKeyLocation GetLocation( TWebKeyCode code );

    /** The Linux input event code (KEY_*) of the key; 0 when there is none */
    static UInt16 GetEvdevCode( TWebKeyCode code );

    /** The X11 hardware key code of the key on an evdev keyboard (the evdev code + 8); 0 when there is none */
    static UInt16 GetX11HardwareKeycode( TWebKeyCode code );

    /** The Windows scan code of the key: set 1, 0xE0xx for an extended key; 0 when there is none */
    static UInt16 GetWindowsScanCode( TWebKeyCode code );

    /** The Windows virtual key (VK_*) of the key on the US layout, left and right modifiers apart (VK_LSHIFT); 0 when there is none */
    static UInt16 GetWindowsVirtualKey( TWebKeyCode code );

    /**
     *  The key value of the key on the US layout with 'modifiers' (TWebModifier flags: shift, and caps lock for letters):
     *  a named key, or WEBNAMEDKEY_CHARACTER with the character in 'text' (UTF-8, a constant). 'text' is GUCEF_NULL for
     *  a named key.
     */
    static TWebNamedKey GetUsLayoutKey( TWebKeyCode code, UInt32 modifiers, const char*& text );

    /** Whether the key types a character on the US layout */
    static bool IsPrintable( TWebKeyCode code );

    /** Whether the key is one of the modifier keys (shift, control, alt, meta on either side) */
    static bool IsModifierKey( TWebKeyCode code );

    /** The flag of MouseEvent.buttons for a button of MouseEvent.button; 0 for WEBMOUSEBUTTON_NONE */
    static UInt32 GetMouseButtonFlag( TWebMouseButton button );

    private:

    CWebViewKeyCodes( void );                                       /**< not implemented: only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWKEYCODES_H ? */
