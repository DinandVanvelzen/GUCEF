/*
 *  inputdriverWAYLAND: GUCEF input driver for Wayland: the pointer and keyboard of the seat for the windows of guidriverWAYLAND
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


#ifndef GUCEF_INPUT_WAYLAND_CKEYMAPPING_H
#define GUCEF_INPUT_WAYLAND_CKEYMAPPING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The translation of what the compositor sends into what gucefINPUT knows, without any library: keysyms and buttons.
 *  The keysym table is that of the X11 input driver, so a key has the same code under both.
 */
class GUCEF_INPUT_PLUGIN_PUBLIC_CPP CKeyMapping
{
    public:

    /** The key code for a keysym (the values of xkbcommon are those of X11); KEYCODE_UNASSIGNED for keys without one */
    static KeyCode MapKeysym( UInt32 keysym );

    /** The index of a mouse button for an evdev button code (BTN_LEFT ...): 0 left, 1 right, 2 middle, 3 and 4 the side buttons; -1 for a button without an index */
    static Int32 MapButton( UInt32 evdevButton );

    /** The key code is a modifier key (Shift, Control, Alt, Super, Caps Lock, Num Lock): they do not repeat */
    static bool IsModifierKey( KeyCode code );

    private:

    CKeyMapping( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_WAYLAND_CKEYMAPPING_H ? */
