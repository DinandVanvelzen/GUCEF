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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "inputdriverWAYLAND_CKeyMapping.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

KeyCode
CKeyMapping::MapKeysym( UInt32 keysym )
{GUCEF_TRACE;

    switch ( keysym )
    {
        case 0xFF08 /* XK_BackSpace */ : return KEYCODE_BACK;
        case 0xFF09 /* XK_Tab */ : return KEYCODE_TAB;
        case 0xFF0A /* XK_Linefeed */ : return KEYCODE_UNASSIGNED;
        case 0xFF0B /* XK_Clear */ : return KEYCODE_UNASSIGNED;
        case 0xFF0D /* XK_Return */ : return KEYCODE_RETURN;
        case 0xFF13 /* XK_Pause */ : return KEYCODE_PAUSE;
        case 0xFF14 /* XK_Scroll_Lock */ : return KEYCODE_SCROLL;
        case 0xFF15 /* XK_Sys_Req */ : return KEYCODE_SYSRQ;
        case 0xFF1B /* XK_Escape */ : return KEYCODE_ESCAPE;
        case 0xFFFF /* XK_Delete */ : return KEYCODE_DELETE;
        case 0xFFBE /* XK_F1 */ : return KEYCODE_F1;
        case 0xFFBF /* XK_F2 */ : return KEYCODE_F2;
        case 0xFFC0 /* XK_F3 */ : return KEYCODE_F3;
        case 0xFFC1 /* XK_F4 */ : return KEYCODE_F4;
        case 0xFFC2 /* XK_F5 */ : return KEYCODE_F5;
        case 0xFFC3 /* XK_F6 */ : return KEYCODE_F6;
        case 0xFFC4 /* XK_F7 */ : return KEYCODE_F7;
        case 0xFFC5 /* XK_F8 */ : return KEYCODE_F8;
        case 0xFFC6 /* XK_F9 */ : return KEYCODE_F9;
        case 0xFFC7 /* XK_F10 */ : return KEYCODE_F10;
        case 0xFFC8 /* XK_F11 */ : return KEYCODE_F11;
        case 0xFFC9 /* XK_F12 */ : return KEYCODE_F12;
        case 0xFFCA /* XK_F13 */ : return KEYCODE_F13;
        case 0xFFCB /* XK_F14 */ : return KEYCODE_F14;
        case 0xFFCC /* XK_F15 */ : return KEYCODE_F15;
        case 0x0020 /* XK_space */ : return KEYCODE_SPACE;
        case 0x0021 /* XK_exclam */ : return KEYCODE_UNASSIGNED;
        case 0x0022 /* XK_quotedbl */ : return KEYCODE_UNASSIGNED;
        case 0x0023 /* XK_numbersign */ : return KEYCODE_UNASSIGNED;
        case 0x0024 /* XK_dollar */ : return KEYCODE_UNASSIGNED;
        case 0x0025 /* XK_percent */ : return KEYCODE_UNASSIGNED;
        case 0x0026 /* XK_ampersand */ : return KEYCODE_UNASSIGNED;
        case 0x0027 /* XK_apostrophe */ : return KEYCODE_UNASSIGNED;
        case 0x0028 /* XK_parenleft */ : return KEYCODE_UNASSIGNED;
        case 0x0029 /* XK_parenright */ : return KEYCODE_UNASSIGNED;
        case 0x002A /* XK_asterisk */ : return KEYCODE_UNASSIGNED;
        case 0x002B /* XK_plus */ : return KEYCODE_ADD;
        case 0x002C /* XK_comma */ :  return KEYCODE_COMMA;
        case 0x002D /* XK_minus */ :  return KEYCODE_MINUS;
        case 0x002E /* XK_period */ :  return KEYCODE_PERIOD;
        case 0x002F /* XK_slash */ :  return KEYCODE_SLASH;
        case 0x0030 /* XK_0 */ : return KEYCODE_0;
        case 0x0031 /* XK_1 */ : return KEYCODE_1;
        case 0x0032 /* XK_2 */ : return KEYCODE_2;
        case 0x0033 /* XK_3 */ : return KEYCODE_3;
        case 0x0034 /* XK_4 */ : return KEYCODE_4;
        case 0x0035 /* XK_5 */ : return KEYCODE_5;
        case 0x0036 /* XK_6 */ : return KEYCODE_6;
        case 0x0037 /* XK_7 */ : return KEYCODE_7;
        case 0x0038 /* XK_8 */ : return KEYCODE_8;
        case 0x0039 /* XK_9 */ : return KEYCODE_9;
        case 0x003A /* XK_colon */ : return KEYCODE_COLON;
        case 0x003B /* XK_semicolon */ : return KEYCODE_SEMICOLON;
        case 0x003C /* XK_less */ : return KEYCODE_UNASSIGNED;
        case 0x003D /* XK_equal */ : return KEYCODE_EQUALS;
        case 0x003E /* XK_greater */ : return KEYCODE_UNASSIGNED;
        case 0x003F /* XK_question */ : return KEYCODE_ABNT_C1;
        case 0x0040 /* XK_at */ : return KEYCODE_AT;
        case 0x0041 /* XK_A */ : return KEYCODE_A;
        case 0x0042 /* XK_B */ : return KEYCODE_B;
        case 0x0043 /* XK_C */ : return KEYCODE_C;
        case 0x0044 /* XK_D */ : return KEYCODE_D;
        case 0x0045 /* XK_E */ : return KEYCODE_E;
        case 0x0046 /* XK_F */ : return KEYCODE_F;
        case 0x0047 /* XK_G */ : return KEYCODE_G;
        case 0x0048 /* XK_H */ : return KEYCODE_H;
        case 0x0049 /* XK_I */ : return KEYCODE_I;
        case 0x004A /* XK_J */ : return KEYCODE_J;
        case 0x004B /* XK_K */ : return KEYCODE_K;
        case 0x004C /* XK_L */ : return KEYCODE_L;
        case 0x004D /* XK_M */ : return KEYCODE_M;
        case 0x004E /* XK_N */ : return KEYCODE_N;
        case 0x004F /* XK_O */ : return KEYCODE_O;
        case 0x0050 /* XK_P */ : return KEYCODE_P;
        case 0x0051 /* XK_Q */ : return KEYCODE_Q;
        case 0x0052 /* XK_R */ : return KEYCODE_R;
        case 0x0053 /* XK_S */ : return KEYCODE_S;
        case 0x0054 /* XK_T */ : return KEYCODE_T;
        case 0x0055 /* XK_U */ : return KEYCODE_U;
        case 0x0056 /* XK_V */ : return KEYCODE_V;
        case 0x0057 /* XK_W */ : return KEYCODE_W;
        case 0x0058 /* XK_X */ : return KEYCODE_X;
        case 0x0059 /* XK_Y */ : return KEYCODE_Y;
        case 0x005A /* XK_Z */ : return KEYCODE_Z;
        case 0x005B /* XK_bracketleft */ : return KEYCODE_LBRACKET;
        case 0x005C /* XK_backslash */ : return KEYCODE_BACKSLASH;
        case 0x005D /* XK_bracketright */ : return KEYCODE_RBRACKET;
        case 0x005E /* XK_asciicircum */ : return KEYCODE_PREVTRACK;
        case 0x005F /* XK_underscore */ : return KEYCODE_UNASSIGNED;
        case 0x0060 /* XK_grave */ :  return KEYCODE_GRAVE;
        case 0x0061 /* XK_a */ : return KEYCODE_A;
        case 0x0062 /* XK_b */ : return KEYCODE_B;
        case 0x0063 /* XK_c */ : return KEYCODE_C;
        case 0x0064 /* XK_d */ : return KEYCODE_D;
        case 0x0065 /* XK_e */ : return KEYCODE_E;
        case 0x0066 /* XK_f */ : return KEYCODE_F;
        case 0x0067 /* XK_g */ : return KEYCODE_G;
        case 0x0068 /* XK_h */ : return KEYCODE_H;
        case 0x0069 /* XK_i */ : return KEYCODE_I;
        case 0x006A /* XK_j */ : return KEYCODE_J;
        case 0x006B /* XK_k */ : return KEYCODE_K;
        case 0x006C /* XK_l */ : return KEYCODE_L;
        case 0x006D /* XK_m */ : return KEYCODE_M;
        case 0x006E /* XK_n */ : return KEYCODE_N;
        case 0x006F /* XK_o */ : return KEYCODE_O;
        case 0x0070 /* XK_p */ : return KEYCODE_P;
        case 0x0071 /* XK_q */ : return KEYCODE_Q;
        case 0x0072 /* XK_r */ : return KEYCODE_R;
        case 0x0073 /* XK_s */ : return KEYCODE_S;
        case 0x0074 /* XK_t */ : return KEYCODE_T;
        case 0x0075 /* XK_u */ : return KEYCODE_U;
        case 0x0076 /* XK_v */ : return KEYCODE_V;
        case 0x0077 /* XK_w */ : return KEYCODE_W;
        case 0x0078 /* XK_x */ : return KEYCODE_X;
        case 0x0079 /* XK_y */ : return KEYCODE_Y;
        case 0x007A /* XK_z */ : return KEYCODE_Z;
        case 0x007B /* XK_braceleft */ : return KEYCODE_UNASSIGNED;
        case 0x007C /* XK_bar */ : return KEYCODE_UNASSIGNED;
        case 0x007D /* XK_braceright */ : return KEYCODE_UNASSIGNED;
        case 0x007E /* XK_asciitilde */ : return KEYCODE_UNASSIGNED;
        case 0x08FB /* XK_leftarrow */ : return KEYCODE_LEFT;
        case 0x08FC /* XK_uparrow */ : return KEYCODE_UP;
        case 0x08FD /* XK_rightarrow */ : return KEYCODE_RIGHT;
        case 0x08FE /* XK_downarrow */ : return KEYCODE_DOWN;
        case 0xFF51 /* XK_Left */ : return KEYCODE_LEFT;
        case 0xFF52 /* XK_Up */ : return KEYCODE_UP;
        case 0xFF53 /* XK_Right */ : return KEYCODE_RIGHT;
        case 0xFF54 /* XK_Down */ : return KEYCODE_DOWN;
        case 0xFF50 /* XK_Home */ : return KEYCODE_HOME;
        case 0xFF57 /* XK_End */ : return KEYCODE_END;
        case 0xFF55 /* XK_Prior */ : return KEYCODE_PGUP;
        case 0xFF56 /* XK_Next */ : return KEYCODE_PGDOWN;
        case 0xFF63 /* XK_Insert */ : return KEYCODE_INSERT;
        case 0xFF67 /* XK_Menu */ : return KEYCODE_APPS;
        case 0xFF61 /* XK_Print */ : return KEYCODE_SYSRQ;
        case 0xFFE1 /* XK_Shift_L */ : return KEYCODE_LSHIFT;
        case 0xFFE2 /* XK_Shift_R */ : return KEYCODE_RSHIFT;
        case 0xFFE3 /* XK_Control_L */ : return KEYCODE_LCONTROL;
        case 0xFFE4 /* XK_Control_R */ : return KEYCODE_RCONTROL;
        case 0xFFE9 /* XK_Alt_L */ : return KEYCODE_LMENU;
        case 0xFFEA /* XK_Alt_R */ : return KEYCODE_RMENU;
        case 0xFFEB /* XK_Super_L */ : return KEYCODE_LWIN;
        case 0xFFEC /* XK_Super_R */ : return KEYCODE_RWIN;
        case 0xFFE5 /* XK_Caps_Lock */ : return KEYCODE_CAPITAL;
        case 0xFF7F /* XK_Num_Lock */ : return KEYCODE_NUMLOCK;
        case 0xFF8D /* XK_KP_Enter */ : return KEYCODE_NUMPADENTER;
        case 0xFFAB /* XK_KP_Add */ : return KEYCODE_ADD;
        case 0xFFAD /* XK_KP_Subtract */ : return KEYCODE_SUBTRACT;
        case 0xFFAA /* XK_KP_Multiply */ : return KEYCODE_MULTIPLY;
        case 0xFFAF /* XK_KP_Divide */ : return KEYCODE_DIVIDE;
        case 0xFFAE /* XK_KP_Decimal */ : return KEYCODE_DECIMAL;
        case 0xFF9F /* XK_KP_Delete */ : return KEYCODE_DELETE;
        case 0xFF9E /* XK_KP_Insert */ : case 0xFFB0 /* XK_KP_0 */ : return KEYCODE_NUMPAD0;
        case 0xFF9C /* XK_KP_End */ : case 0xFFB1 /* XK_KP_1 */ : return KEYCODE_NUMPAD1;
        case 0xFF99 /* XK_KP_Down */ : case 0xFFB2 /* XK_KP_2 */ : return KEYCODE_NUMPAD2;
        case 0xFF9B /* XK_KP_Next */ : case 0xFFB3 /* XK_KP_3 */ : return KEYCODE_NUMPAD3;
        case 0xFF96 /* XK_KP_Left */ : case 0xFFB4 /* XK_KP_4 */ : return KEYCODE_NUMPAD4;
        case 0xFF9D /* XK_KP_Begin */ : case 0xFFB5 /* XK_KP_5 */ : return KEYCODE_NUMPAD5;
        case 0xFF98 /* XK_KP_Right */ : case 0xFFB6 /* XK_KP_6 */ : return KEYCODE_NUMPAD6;
        case 0xFF95 /* XK_KP_Home */ : case 0xFFB7 /* XK_KP_7 */ : return KEYCODE_NUMPAD7;
        case 0xFF97 /* XK_KP_Up */ : case 0xFFB8 /* XK_KP_8 */ : return KEYCODE_NUMPAD8;
        case 0xFF9A /* XK_KP_Prior */ : case 0xFFB9 /* XK_KP_9 */ : return KEYCODE_NUMPAD9;
        default: return KEYCODE_UNASSIGNED;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CKeyMapping::MapButton( UInt32 evdevButton )
{GUCEF_TRACE;

    // linux/input-event-codes.h: BTN_LEFT 0x110 ... BTN_TASK 0x117
    switch ( evdevButton )
    {
        case 0x110 /* BTN_LEFT */ :    return 0;
        case 0x111 /* BTN_RIGHT */ :   return 1;
        case 0x112 /* BTN_MIDDLE */ :  return 2;
        case 0x113 /* BTN_SIDE */ :    return 3;
        case 0x114 /* BTN_EXTRA */ :   return 4;
        case 0x115 /* BTN_FORWARD */ : return 5;
        case 0x116 /* BTN_BACK */ :    return 6;
        case 0x117 /* BTN_TASK */ :    return 7;
        default :                      return -1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CKeyMapping::IsModifierKey( KeyCode code )
{GUCEF_TRACE;

    switch ( code )
    {
        case KEYCODE_LSHIFT : case KEYCODE_RSHIFT : case KEYCODE_LCONTROL : case KEYCODE_RCONTROL :
        case KEYCODE_LMENU : case KEYCODE_RMENU : case KEYCODE_LWIN : case KEYCODE_RWIN :
        case KEYCODE_CAPITAL : case KEYCODE_NUMLOCK : case KEYCODE_SCROLL :
            return true;
        default :
            return false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
