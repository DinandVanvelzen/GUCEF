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

#ifndef GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#define GUCEF_WEB_CWEBVIEWINPUTEVENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

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
 *  One input event for a view: pointer, wheel, key, text or focus, in the terms of the web standard (W3C UI Events,
 *  gucefWEB_WebViewInputTypes.h). Positions are logical pixels of the view (its top left is 0,0). A key event has the
 *  physical key ('code') and the key value: a named key, or WEBNAMEDKEY_CHARACTER with the character in 'keyText'.
 *  Modifiers are TWebModifier flags, mouse buttons are numbered as in the DOM. Text events carry UTF-8 after keyboard
 *  layout and input method. A plain value, copied freely; key events need no allocation. CWebViewKeyCodes gives the
 *  names and platform codes an engine wants for the values. Text typed into a page may be a secret: Describe() leaves
 *  it out unless asked.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewInputEvent
{
    public:

    /** The room for the UTF-8 key value of a key event, with its terminating zero */
    enum { KEY_TEXT_CAPACITY = 8 };

    TWebViewInputKind kind;
    Int32 x;                        /**< pointer events: logical pixels of the view */
    Int32 y;
    TWebMouseButton button;         /**< down and up: MouseEvent.button; WEBMOUSEBUTTON_NONE otherwise */
    UInt32 buttonStates;            /**< MouseEvent.buttons: TWebMouseButtonFlag flags of the buttons held */
    UInt32 clickCount;              /**< down: 1 single, 2 double, 3 triple */
    Float32 wheelNotchesX;          /**< wheel: notches, positive is right */
    Float32 wheelNotchesY;          /**< wheel: notches, positive is away from the user (up) */
    Int32 wheelPixelsX;             /**< wheel: precise deltas in logical pixels where the device has them, else 0 */
    Int32 wheelPixelsY;
    UInt32 modifiers;               /**< TWebModifier flags */
    TWebKeyCode code;               /**< key down and up: the physical key (KeyboardEvent.code) */
    TWebNamedKey namedKey;          /**< key down and up: the key value (KeyboardEvent.key) when it is no character */
    char keyText[ KEY_TEXT_CAPACITY ]; /**< key down and up: the key value when namedKey is WEBNAMEDKEY_CHARACTER, UTF-8 */
    bool isRepeat;                  /**< key down: repeated by holding the key */
    CString text;                   /**< text: UTF-8 */
    UInt64 timestampMs;             /**< when the event happened, MT::PrecisionTickCount based milliseconds; 0 when not known */

    CWebViewInputEvent( void );

    bool IsPointerEvent( void ) const;

    bool IsKeyEvent( void ) const;

    /** Whether the key value of a key event is a character (the text in keyText) */
    bool IsCharacterKey( void ) const;

    /** Sets the key value to a character, UTF-8 (cut at the capacity, at a character boundary) */
    void SetKeyText( const char* utf8Text );

    /** For logs: the kind, positions and keys; the text only when 'includeText' (never for pages that may hold secrets) */
    CString Describe( bool includeText = false ) const;

    static CWebViewInputEvent PointerMove( Int32 x, Int32 y, UInt32 modifiers = 0, UInt32 buttonStates = 0 );

    static CWebViewInputEvent PointerButton( Int32 x, Int32 y, TWebMouseButton button, bool down, UInt32 clickCount = 1, UInt32 modifiers = 0 );

    static CWebViewInputEvent PointerLeave( void );

    static CWebViewInputEvent Wheel( Int32 x, Int32 y, Float32 notchesX, Float32 notchesY, UInt32 modifiers = 0 );

    /** A key event with its key value: a named key, or WEBNAMEDKEY_CHARACTER and the character in 'keyText' */
    static CWebViewInputEvent Key( TWebKeyCode code, TWebNamedKey namedKey, const char* keyText, bool down, UInt32 modifiers = 0, bool isRepeat = false );

    /** A key event with the key value of the key on the US layout (CWebViewKeyCodes::GetUsLayoutKey()) */
    static CWebViewInputEvent UsLayoutKey( TWebKeyCode code, bool down, UInt32 modifiers = 0, bool isRepeat = false );

    static CWebViewInputEvent Text( const CString& text );

    static CWebViewInputEvent Focus( bool focusIn );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWINPUTEVENT_H ? */
