/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H
#define GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#include "gucefWEB_CWebViewInputEvent.h"
#define GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#endif /* GUCEF_WEB_CWEBVIEWINPUTEVENT_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Translates the input of the GUI (INPUT::KeyCode, INPUT::KeyModifier flags, TMouseButton) into the input of web
 *  views, which follows the web standard (gucefWEB_WebViewInputTypes.h): the physical key, the key value, modifier
 *  flags and mouse buttons numbered as in the DOM. Constant lookup arrays: no string compares, no allocation, any
 *  thread. Every INPUT::KeyCode below KEYCODE_MAX has a web key code, WEBKEYCODE_UNIDENTIFIED for the few keys the web
 *  standard has no code for (NEC PC98 and J3100 keys).
 *
 *  The key value of a key event is that of the key on the US layout (CWebViewKeyCodes::GetUsLayoutKey()): what a
 *  user types arrives at the page as text events, after the keyboard layout and input method of the GUI, which is how
 *  characters outside the US layout arrive correctly. Used by CWebPageView.
 */
class GUCEF_GUI_PUBLIC_CPP CWebViewInputTranslator
{
    public:

    /** The physical key of an INPUT::KeyCode; WEBKEYCODE_UNIDENTIFIED for one without a web key code or out of range */
    static WEB::TWebKeyCode GetWebKeyCode( INPUT::KeyCode keyCode );

    /** The TWebModifier flags of INPUT::KeyModifier flags */
    static UInt32 GetWebModifiers( UInt32 inputModifiers );

    /** The DOM number of a mouse button of the GUI; WEBMOUSEBUTTON_NONE for MOUSEBUTTON_NONE or one out of range */
    static WEB::TWebMouseButton GetWebMouseButton( TMouseButton button );

    /** The DOM MouseEvent.buttons flags of the held buttons of the GUI (bit 1 << TMouseButton for each) */
    static UInt32 GetWebMouseButtonStates( UInt32 guiButtonStates );

    /** The key event of a view for a key of the GUI: the physical key, its key value on the US layout, the modifiers */
    static WEB::CWebViewInputEvent KeyEvent( INPUT::KeyCode keyCode, bool down, UInt32 inputModifiers, bool isRepeat = false );

    private:

    CWebViewInputTranslator( void );                                /**< not implemented: only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H ? */
