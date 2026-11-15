/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#ifndef GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H
#define GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "imgui.h"

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The translations between the input of gucefGUI and gucefINPUT and that of Dear ImGui:
 *
 *  - key codes: positional INPUT::KeyCode values to ImGuiKey, one to one (letters, digits, punctuation, function keys,
 *    arrows, editing keys, keypad, modifiers, lock keys); the letters of shortcuts follow the US layout as gucefGUI's do;
 *  - modifiers: KEYMOD_SHIFT, KEYMOD_CTRL, KEYMOD_ALT and KEYMOD_META to ImGuiMod_Shift, _Ctrl, _Alt and _Super;
 *  - mouse buttons: MOUSEBUTTON_LEFT .. MOUSEBUTTON_X2 (1 .. 5) to the buttons 0 .. 4 of Dear ImGui;
 *  - the mouse cursor Dear ImGui asks for to a TCursorShape.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiInputMapper
{
    public:

    /** The key of Dear ImGui for a key code, ImGuiKey_None for a key code that has none */
    static ImGuiKey ToImGuiKey( INPUT::KeyCode keyCode );

    /** The key code for a key of Dear ImGui, KEYCODE_UNASSIGNED for a key that has none */
    static INPUT::KeyCode ToKeyCode( ImGuiKey key );

    /** How many key codes have a key of Dear ImGui, and the n-th of them: the table, for tests and tools */
    static UInt32 GetMappedKeyCount( void );
    static INPUT::KeyCode GetMappedKeyCode( UInt32 index );

    /** The modifiers of Dear ImGui (ImGuiMod_* flags) for the modifiers of gucefINPUT (INPUT::KEYMOD_* flags) */
    static ImGuiKeyChord ToImGuiModifiers( UInt32 modifiers );

    /** Queues the state of the four modifiers into Dear ImGui as key events, as it wants them before every key event */
    static void QueueModifiers( ImGuiIO& io, UInt32 modifiers );

    /** The button of Dear ImGui (0 .. 4) for a mouse button, -1 for MOUSEBUTTON_NONE and unknown values */
    static Int32 ToImGuiMouseButton( GUI::TMouseButton button );

    /** The shape of the pointer of the system for the cursor Dear ImGui asks for; the arrow for none and unknown values */
    static GUI::TCursorShape ToCursorShape( ImGuiMouseCursor cursor );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H ? */
