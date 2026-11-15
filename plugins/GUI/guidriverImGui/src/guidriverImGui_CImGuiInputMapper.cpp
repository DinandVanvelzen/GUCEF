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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "guidriverImGui_CImGuiInputMapper.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SKeyMapping
{
    INPUT::KeyCode keyCode;
    ImGuiKey key;
};

/** Every key code that has a key of Dear ImGui; each appears once on either side */
const SKeyMapping g_keyTable[] =
{
    { INPUT::KEYCODE_ESCAPE, ImGuiKey_Escape },
    { INPUT::KEYCODE_1, ImGuiKey_1 },
    { INPUT::KEYCODE_2, ImGuiKey_2 },
    { INPUT::KEYCODE_3, ImGuiKey_3 },
    { INPUT::KEYCODE_4, ImGuiKey_4 },
    { INPUT::KEYCODE_5, ImGuiKey_5 },
    { INPUT::KEYCODE_6, ImGuiKey_6 },
    { INPUT::KEYCODE_7, ImGuiKey_7 },
    { INPUT::KEYCODE_8, ImGuiKey_8 },
    { INPUT::KEYCODE_9, ImGuiKey_9 },
    { INPUT::KEYCODE_0, ImGuiKey_0 },
    { INPUT::KEYCODE_MINUS, ImGuiKey_Minus },
    { INPUT::KEYCODE_EQUALS, ImGuiKey_Equal },
    { INPUT::KEYCODE_BACK, ImGuiKey_Backspace },
    { INPUT::KEYCODE_TAB, ImGuiKey_Tab },
    { INPUT::KEYCODE_Q, ImGuiKey_Q },
    { INPUT::KEYCODE_W, ImGuiKey_W },
    { INPUT::KEYCODE_E, ImGuiKey_E },
    { INPUT::KEYCODE_R, ImGuiKey_R },
    { INPUT::KEYCODE_T, ImGuiKey_T },
    { INPUT::KEYCODE_Y, ImGuiKey_Y },
    { INPUT::KEYCODE_U, ImGuiKey_U },
    { INPUT::KEYCODE_I, ImGuiKey_I },
    { INPUT::KEYCODE_O, ImGuiKey_O },
    { INPUT::KEYCODE_P, ImGuiKey_P },
    { INPUT::KEYCODE_LBRACKET, ImGuiKey_LeftBracket },
    { INPUT::KEYCODE_RBRACKET, ImGuiKey_RightBracket },
    { INPUT::KEYCODE_RETURN, ImGuiKey_Enter },
    { INPUT::KEYCODE_LCONTROL, ImGuiKey_LeftCtrl },
    { INPUT::KEYCODE_A, ImGuiKey_A },
    { INPUT::KEYCODE_S, ImGuiKey_S },
    { INPUT::KEYCODE_D, ImGuiKey_D },
    { INPUT::KEYCODE_F, ImGuiKey_F },
    { INPUT::KEYCODE_G, ImGuiKey_G },
    { INPUT::KEYCODE_H, ImGuiKey_H },
    { INPUT::KEYCODE_J, ImGuiKey_J },
    { INPUT::KEYCODE_K, ImGuiKey_K },
    { INPUT::KEYCODE_L, ImGuiKey_L },
    { INPUT::KEYCODE_SEMICOLON, ImGuiKey_Semicolon },
    { INPUT::KEYCODE_APOSTROPHE, ImGuiKey_Apostrophe },
    { INPUT::KEYCODE_GRAVE, ImGuiKey_GraveAccent },
    { INPUT::KEYCODE_LSHIFT, ImGuiKey_LeftShift },
    { INPUT::KEYCODE_BACKSLASH, ImGuiKey_Backslash },
    { INPUT::KEYCODE_Z, ImGuiKey_Z },
    { INPUT::KEYCODE_X, ImGuiKey_X },
    { INPUT::KEYCODE_C, ImGuiKey_C },
    { INPUT::KEYCODE_V, ImGuiKey_V },
    { INPUT::KEYCODE_B, ImGuiKey_B },
    { INPUT::KEYCODE_N, ImGuiKey_N },
    { INPUT::KEYCODE_M, ImGuiKey_M },
    { INPUT::KEYCODE_COMMA, ImGuiKey_Comma },
    { INPUT::KEYCODE_PERIOD, ImGuiKey_Period },
    { INPUT::KEYCODE_SLASH, ImGuiKey_Slash },
    { INPUT::KEYCODE_RSHIFT, ImGuiKey_RightShift },
    { INPUT::KEYCODE_MULTIPLY, ImGuiKey_KeypadMultiply },
    { INPUT::KEYCODE_LMENU, ImGuiKey_LeftAlt },
    { INPUT::KEYCODE_SPACE, ImGuiKey_Space },
    { INPUT::KEYCODE_CAPITAL, ImGuiKey_CapsLock },
    { INPUT::KEYCODE_F1, ImGuiKey_F1 },
    { INPUT::KEYCODE_F2, ImGuiKey_F2 },
    { INPUT::KEYCODE_F3, ImGuiKey_F3 },
    { INPUT::KEYCODE_F4, ImGuiKey_F4 },
    { INPUT::KEYCODE_F5, ImGuiKey_F5 },
    { INPUT::KEYCODE_F6, ImGuiKey_F6 },
    { INPUT::KEYCODE_F7, ImGuiKey_F7 },
    { INPUT::KEYCODE_F8, ImGuiKey_F8 },
    { INPUT::KEYCODE_F9, ImGuiKey_F9 },
    { INPUT::KEYCODE_F10, ImGuiKey_F10 },
    { INPUT::KEYCODE_NUMLOCK, ImGuiKey_NumLock },
    { INPUT::KEYCODE_SCROLL, ImGuiKey_ScrollLock },
    { INPUT::KEYCODE_NUMPAD7, ImGuiKey_Keypad7 },
    { INPUT::KEYCODE_NUMPAD8, ImGuiKey_Keypad8 },
    { INPUT::KEYCODE_NUMPAD9, ImGuiKey_Keypad9 },
    { INPUT::KEYCODE_SUBTRACT, ImGuiKey_KeypadSubtract },
    { INPUT::KEYCODE_NUMPAD4, ImGuiKey_Keypad4 },
    { INPUT::KEYCODE_NUMPAD5, ImGuiKey_Keypad5 },
    { INPUT::KEYCODE_NUMPAD6, ImGuiKey_Keypad6 },
    { INPUT::KEYCODE_ADD, ImGuiKey_KeypadAdd },
    { INPUT::KEYCODE_NUMPAD1, ImGuiKey_Keypad1 },
    { INPUT::KEYCODE_NUMPAD2, ImGuiKey_Keypad2 },
    { INPUT::KEYCODE_NUMPAD3, ImGuiKey_Keypad3 },
    { INPUT::KEYCODE_NUMPAD0, ImGuiKey_Keypad0 },
    { INPUT::KEYCODE_DECIMAL, ImGuiKey_KeypadDecimal },
    { INPUT::KEYCODE_OEM_102, ImGuiKey_Oem102 },
    { INPUT::KEYCODE_F11, ImGuiKey_F11 },
    { INPUT::KEYCODE_F12, ImGuiKey_F12 },
    { INPUT::KEYCODE_F13, ImGuiKey_F13 },
    { INPUT::KEYCODE_F14, ImGuiKey_F14 },
    { INPUT::KEYCODE_F15, ImGuiKey_F15 },
    { INPUT::KEYCODE_NUMPADEQUALS, ImGuiKey_KeypadEqual },
    { INPUT::KEYCODE_NUMPADENTER, ImGuiKey_KeypadEnter },
    { INPUT::KEYCODE_RCONTROL, ImGuiKey_RightCtrl },
    { INPUT::KEYCODE_DIVIDE, ImGuiKey_KeypadDivide },
    { INPUT::KEYCODE_SYSRQ, ImGuiKey_PrintScreen },
    { INPUT::KEYCODE_RMENU, ImGuiKey_RightAlt },
    { INPUT::KEYCODE_PAUSE, ImGuiKey_Pause },
    { INPUT::KEYCODE_HOME, ImGuiKey_Home },
    { INPUT::KEYCODE_UP, ImGuiKey_UpArrow },
    { INPUT::KEYCODE_PGUP, ImGuiKey_PageUp },
    { INPUT::KEYCODE_LEFT, ImGuiKey_LeftArrow },
    { INPUT::KEYCODE_RIGHT, ImGuiKey_RightArrow },
    { INPUT::KEYCODE_END, ImGuiKey_End },
    { INPUT::KEYCODE_DOWN, ImGuiKey_DownArrow },
    { INPUT::KEYCODE_PGDOWN, ImGuiKey_PageDown },
    { INPUT::KEYCODE_INSERT, ImGuiKey_Insert },
    { INPUT::KEYCODE_DELETE, ImGuiKey_Delete },
    { INPUT::KEYCODE_LWIN, ImGuiKey_LeftSuper },
    { INPUT::KEYCODE_RWIN, ImGuiKey_RightSuper },
    { INPUT::KEYCODE_APPS, ImGuiKey_Menu },
    { INPUT::KEYCODE_WEBBACK, ImGuiKey_AppBack },
    { INPUT::KEYCODE_WEBFORWARD, ImGuiKey_AppForward },
};

const UInt32 g_keyTableSize = (UInt32) ( sizeof( g_keyTable ) / sizeof( g_keyTable[ 0 ] ) );

/** The table by key code: built once, the key codes are below KEYCODE_MAX */
class CKeyLookup
{
    public:

    CKeyLookup( void )
    {
        for ( UInt32 i=0; i<(UInt32)INPUT::KEYCODE_MAX; ++i )
            m_keys[ i ] = ImGuiKey_None;
        for ( UInt32 i=0; i<g_keyTableSize; ++i )
            m_keys[ (UInt32) g_keyTable[ i ].keyCode ] = g_keyTable[ i ].key;
    }

    ImGuiKey Get( INPUT::KeyCode keyCode ) const
    {
        UInt32 index = (UInt32) keyCode;
        return index < (UInt32) INPUT::KEYCODE_MAX ? m_keys[ index ] : ImGuiKey_None;
    }

    private:

    ImGuiKey m_keys[ INPUT::KEYCODE_MAX ];
};

const CKeyLookup&
KeyLookup( void )
{
    static const CKeyLookup lookup;
    return lookup;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ImGuiKey
CImGuiInputMapper::ToImGuiKey( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    return KeyLookup().Get( keyCode );
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
CImGuiInputMapper::ToKeyCode( ImGuiKey key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<g_keyTableSize; ++i )
        if ( g_keyTable[ i ].key == key )
            return g_keyTable[ i ].keyCode;
    return INPUT::KEYCODE_UNASSIGNED;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiInputMapper::GetMappedKeyCount( void )
{GUCEF_TRACE;

    return g_keyTableSize;
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
CImGuiInputMapper::GetMappedKeyCode( UInt32 index )
{GUCEF_TRACE;

    return index < g_keyTableSize ? g_keyTable[ index ].keyCode : INPUT::KEYCODE_UNASSIGNED;
}

/*-------------------------------------------------------------------------*/

ImGuiKeyChord
CImGuiInputMapper::ToImGuiModifiers( UInt32 modifiers )
{GUCEF_TRACE;

    ImGuiKeyChord chord = ImGuiMod_None;
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) )
        chord |= ImGuiMod_Shift;
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
        chord |= ImGuiMod_Ctrl;
    if ( 0 != ( modifiers & INPUT::KEYMOD_ALT ) )
        chord |= ImGuiMod_Alt;
    if ( 0 != ( modifiers & INPUT::KEYMOD_META ) )
        chord |= ImGuiMod_Super;
    return chord;
}

/*-------------------------------------------------------------------------*/

void
CImGuiInputMapper::QueueModifiers( ImGuiIO& io, UInt32 modifiers )
{GUCEF_TRACE;

    io.AddKeyEvent( ImGuiMod_Ctrl, 0 != ( modifiers & INPUT::KEYMOD_CTRL ) );
    io.AddKeyEvent( ImGuiMod_Shift, 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) );
    io.AddKeyEvent( ImGuiMod_Alt, 0 != ( modifiers & INPUT::KEYMOD_ALT ) );
    io.AddKeyEvent( ImGuiMod_Super, 0 != ( modifiers & INPUT::KEYMOD_META ) );
}

/*-------------------------------------------------------------------------*/

Int32
CImGuiInputMapper::ToImGuiMouseButton( GUI::TMouseButton button )
{GUCEF_TRACE;

    switch ( button )
    {
        case GUI::MOUSEBUTTON_LEFT: return 0;
        case GUI::MOUSEBUTTON_RIGHT: return 1;
        case GUI::MOUSEBUTTON_MIDDLE: return 2;
        case GUI::MOUSEBUTTON_X1: return 3;
        case GUI::MOUSEBUTTON_X2: return 4;
        default: return -1;
    }
}

/*-------------------------------------------------------------------------*/

GUI::TCursorShape
CImGuiInputMapper::ToCursorShape( ImGuiMouseCursor cursor )
{GUCEF_TRACE;

    switch ( cursor )
    {
        case ImGuiMouseCursor_TextInput: return GUI::CURSORSHAPE_TEXT;
        case ImGuiMouseCursor_ResizeAll: return GUI::CURSORSHAPE_RESIZE_ALL;
        case ImGuiMouseCursor_ResizeNS: return GUI::CURSORSHAPE_RESIZE_NS;
        case ImGuiMouseCursor_ResizeEW: return GUI::CURSORSHAPE_RESIZE_EW;
        case ImGuiMouseCursor_ResizeNESW: return GUI::CURSORSHAPE_RESIZE_NESW;
        case ImGuiMouseCursor_ResizeNWSE: return GUI::CURSORSHAPE_RESIZE_NWSE;
        case ImGuiMouseCursor_Hand: return GUI::CURSORSHAPE_HAND;
        case ImGuiMouseCursor_Wait: return GUI::CURSORSHAPE_WAIT;
        case ImGuiMouseCursor_Progress: return GUI::CURSORSHAPE_PROGRESS;
        case ImGuiMouseCursor_NotAllowed: return GUI::CURSORSHAPE_NOT_ALLOWED;
        case ImGuiMouseCursor_Arrow:
        default: return GUI::CURSORSHAPE_ARROW;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
