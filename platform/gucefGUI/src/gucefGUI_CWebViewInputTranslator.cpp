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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBVIEWKEYCODES_H
#include "gucefWEB_CWebViewKeyCodes.h"
#define GUCEF_WEB_CWEBVIEWKEYCODES_H
#endif /* GUCEF_WEB_CWEBVIEWKEYCODES_H ? */

#include "gucefGUI_CWebViewInputTranslator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The physical key of the web standard of each INPUT::KeyCode, indexed by the key code (a scan code position) */
const WEB::TWebKeyCode WebKeyCodes[ INPUT::KEYCODE_MAX ] =
{
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x00 KEYCODE_UNASSIGNED */
    WEB::WEBKEYCODE_ESCAPE,                      /* 0x01 KEYCODE_ESCAPE */
    WEB::WEBKEYCODE_DIGIT1,                      /* 0x02 KEYCODE_1 */
    WEB::WEBKEYCODE_DIGIT2,                      /* 0x03 KEYCODE_2 */
    WEB::WEBKEYCODE_DIGIT3,                      /* 0x04 KEYCODE_3 */
    WEB::WEBKEYCODE_DIGIT4,                      /* 0x05 KEYCODE_4 */
    WEB::WEBKEYCODE_DIGIT5,                      /* 0x06 KEYCODE_5 */
    WEB::WEBKEYCODE_DIGIT6,                      /* 0x07 KEYCODE_6 */
    WEB::WEBKEYCODE_DIGIT7,                      /* 0x08 KEYCODE_7 */
    WEB::WEBKEYCODE_DIGIT8,                      /* 0x09 KEYCODE_8 */
    WEB::WEBKEYCODE_DIGIT9,                      /* 0x0A KEYCODE_9 */
    WEB::WEBKEYCODE_DIGIT0,                      /* 0x0B KEYCODE_0 */
    WEB::WEBKEYCODE_MINUS,                       /* 0x0C KEYCODE_MINUS */
    WEB::WEBKEYCODE_EQUAL,                       /* 0x0D KEYCODE_EQUALS */
    WEB::WEBKEYCODE_BACKSPACE,                   /* 0x0E KEYCODE_BACK */
    WEB::WEBKEYCODE_TAB,                         /* 0x0F KEYCODE_TAB */
    WEB::WEBKEYCODE_KEY_Q,                       /* 0x10 KEYCODE_Q */
    WEB::WEBKEYCODE_KEY_W,                       /* 0x11 KEYCODE_W */
    WEB::WEBKEYCODE_KEY_E,                       /* 0x12 KEYCODE_E */
    WEB::WEBKEYCODE_KEY_R,                       /* 0x13 KEYCODE_R */
    WEB::WEBKEYCODE_KEY_T,                       /* 0x14 KEYCODE_T */
    WEB::WEBKEYCODE_KEY_Y,                       /* 0x15 KEYCODE_Y */
    WEB::WEBKEYCODE_KEY_U,                       /* 0x16 KEYCODE_U */
    WEB::WEBKEYCODE_KEY_I,                       /* 0x17 KEYCODE_I */
    WEB::WEBKEYCODE_KEY_O,                       /* 0x18 KEYCODE_O */
    WEB::WEBKEYCODE_KEY_P,                       /* 0x19 KEYCODE_P */
    WEB::WEBKEYCODE_BRACKET_LEFT,                /* 0x1A KEYCODE_LBRACKET */
    WEB::WEBKEYCODE_BRACKET_RIGHT,               /* 0x1B KEYCODE_RBRACKET */
    WEB::WEBKEYCODE_ENTER,                       /* 0x1C KEYCODE_RETURN */
    WEB::WEBKEYCODE_CONTROL_LEFT,                /* 0x1D KEYCODE_LCONTROL */
    WEB::WEBKEYCODE_KEY_A,                       /* 0x1E KEYCODE_A */
    WEB::WEBKEYCODE_KEY_S,                       /* 0x1F KEYCODE_S */
    WEB::WEBKEYCODE_KEY_D,                       /* 0x20 KEYCODE_D */
    WEB::WEBKEYCODE_KEY_F,                       /* 0x21 KEYCODE_F */
    WEB::WEBKEYCODE_KEY_G,                       /* 0x22 KEYCODE_G */
    WEB::WEBKEYCODE_KEY_H,                       /* 0x23 KEYCODE_H */
    WEB::WEBKEYCODE_KEY_J,                       /* 0x24 KEYCODE_J */
    WEB::WEBKEYCODE_KEY_K,                       /* 0x25 KEYCODE_K */
    WEB::WEBKEYCODE_KEY_L,                       /* 0x26 KEYCODE_L */
    WEB::WEBKEYCODE_SEMICOLON,                   /* 0x27 KEYCODE_SEMICOLON */
    WEB::WEBKEYCODE_QUOTE,                       /* 0x28 KEYCODE_APOSTROPHE */
    WEB::WEBKEYCODE_BACKQUOTE,                   /* 0x29 KEYCODE_GRAVE */
    WEB::WEBKEYCODE_SHIFT_LEFT,                  /* 0x2A KEYCODE_LSHIFT */
    WEB::WEBKEYCODE_BACKSLASH,                   /* 0x2B KEYCODE_BACKSLASH */
    WEB::WEBKEYCODE_KEY_Z,                       /* 0x2C KEYCODE_Z */
    WEB::WEBKEYCODE_KEY_X,                       /* 0x2D KEYCODE_X */
    WEB::WEBKEYCODE_KEY_C,                       /* 0x2E KEYCODE_C */
    WEB::WEBKEYCODE_KEY_V,                       /* 0x2F KEYCODE_V */
    WEB::WEBKEYCODE_KEY_B,                       /* 0x30 KEYCODE_B */
    WEB::WEBKEYCODE_KEY_N,                       /* 0x31 KEYCODE_N */
    WEB::WEBKEYCODE_KEY_M,                       /* 0x32 KEYCODE_M */
    WEB::WEBKEYCODE_COMMA,                       /* 0x33 KEYCODE_COMMA */
    WEB::WEBKEYCODE_PERIOD,                      /* 0x34 KEYCODE_PERIOD */
    WEB::WEBKEYCODE_SLASH,                       /* 0x35 KEYCODE_SLASH */
    WEB::WEBKEYCODE_SHIFT_RIGHT,                 /* 0x36 KEYCODE_RSHIFT */
    WEB::WEBKEYCODE_NUMPAD_MULTIPLY,             /* 0x37 KEYCODE_MULTIPLY */
    WEB::WEBKEYCODE_ALT_LEFT,                    /* 0x38 KEYCODE_LMENU */
    WEB::WEBKEYCODE_SPACE,                       /* 0x39 KEYCODE_SPACE */
    WEB::WEBKEYCODE_CAPS_LOCK,                   /* 0x3A KEYCODE_CAPITAL */
    WEB::WEBKEYCODE_F1,                          /* 0x3B KEYCODE_F1 */
    WEB::WEBKEYCODE_F2,                          /* 0x3C KEYCODE_F2 */
    WEB::WEBKEYCODE_F3,                          /* 0x3D KEYCODE_F3 */
    WEB::WEBKEYCODE_F4,                          /* 0x3E KEYCODE_F4 */
    WEB::WEBKEYCODE_F5,                          /* 0x3F KEYCODE_F5 */
    WEB::WEBKEYCODE_F6,                          /* 0x40 KEYCODE_F6 */
    WEB::WEBKEYCODE_F7,                          /* 0x41 KEYCODE_F7 */
    WEB::WEBKEYCODE_F8,                          /* 0x42 KEYCODE_F8 */
    WEB::WEBKEYCODE_F9,                          /* 0x43 KEYCODE_F9 */
    WEB::WEBKEYCODE_F10,                         /* 0x44 KEYCODE_F10 */
    WEB::WEBKEYCODE_NUM_LOCK,                    /* 0x45 KEYCODE_NUMLOCK */
    WEB::WEBKEYCODE_SCROLL_LOCK,                 /* 0x46 KEYCODE_SCROLL */
    WEB::WEBKEYCODE_NUMPAD7,                     /* 0x47 KEYCODE_NUMPAD7 */
    WEB::WEBKEYCODE_NUMPAD8,                     /* 0x48 KEYCODE_NUMPAD8 */
    WEB::WEBKEYCODE_NUMPAD9,                     /* 0x49 KEYCODE_NUMPAD9 */
    WEB::WEBKEYCODE_NUMPAD_SUBTRACT,             /* 0x4A KEYCODE_SUBTRACT */
    WEB::WEBKEYCODE_NUMPAD4,                     /* 0x4B KEYCODE_NUMPAD4 */
    WEB::WEBKEYCODE_NUMPAD5,                     /* 0x4C KEYCODE_NUMPAD5 */
    WEB::WEBKEYCODE_NUMPAD6,                     /* 0x4D KEYCODE_NUMPAD6 */
    WEB::WEBKEYCODE_NUMPAD_ADD,                  /* 0x4E KEYCODE_ADD */
    WEB::WEBKEYCODE_NUMPAD1,                     /* 0x4F KEYCODE_NUMPAD1 */
    WEB::WEBKEYCODE_NUMPAD2,                     /* 0x50 KEYCODE_NUMPAD2 */
    WEB::WEBKEYCODE_NUMPAD3,                     /* 0x51 KEYCODE_NUMPAD3 */
    WEB::WEBKEYCODE_NUMPAD0,                     /* 0x52 KEYCODE_NUMPAD0 */
    WEB::WEBKEYCODE_NUMPAD_DECIMAL,              /* 0x53 KEYCODE_DECIMAL */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x54 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x55 */
    WEB::WEBKEYCODE_INTL_BACKSLASH,              /* 0x56 KEYCODE_OEM_102 */
    WEB::WEBKEYCODE_F11,                         /* 0x57 KEYCODE_F11 */
    WEB::WEBKEYCODE_F12,                         /* 0x58 KEYCODE_F12 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x59 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x5A */
    WEB::WEBKEYCODE_META_LEFT,                   /* 0x5B KEYCODE_LEFTWINKEY */
    WEB::WEBKEYCODE_META_RIGHT,                  /* 0x5C KEYCODE_RIGHTWINKEY */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x5D */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x5E */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x5F */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x60 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x61 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x62 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x63 */
    WEB::WEBKEYCODE_F13,                         /* 0x64 KEYCODE_F13 */
    WEB::WEBKEYCODE_F14,                         /* 0x65 KEYCODE_F14 */
    WEB::WEBKEYCODE_F15,                         /* 0x66 KEYCODE_F15 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x67 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x68 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x69 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6A */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6B */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6C */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6D */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6E */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x6F */
    WEB::WEBKEYCODE_KANA_MODE,                   /* 0x70 KEYCODE_KANA */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x71 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x72 */
    WEB::WEBKEYCODE_INTL_RO,                     /* 0x73 KEYCODE_ABNT_C1 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x74 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x75 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x76 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x77 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x78 */
    WEB::WEBKEYCODE_CONVERT,                     /* 0x79 KEYCODE_CONVERT */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x7A */
    WEB::WEBKEYCODE_NON_CONVERT,                 /* 0x7B KEYCODE_NOCONVERT */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x7C */
    WEB::WEBKEYCODE_INTL_YEN,                    /* 0x7D KEYCODE_YEN */
    WEB::WEBKEYCODE_NUMPAD_COMMA,                /* 0x7E KEYCODE_ABNT_C2 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x7F */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x80 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x81 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x82 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x83 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x84 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x85 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x86 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x87 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x88 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x89 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x8A */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x8B */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x8C */
    WEB::WEBKEYCODE_NUMPAD_EQUAL,                /* 0x8D KEYCODE_NUMPADEQUALS */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x8E */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x8F */
    WEB::WEBKEYCODE_MEDIA_TRACK_PREVIOUS,        /* 0x90 KEYCODE_PREVTRACK */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x91 KEYCODE_AT */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x92 KEYCODE_COLON */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x93 KEYCODE_UNDERLINE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x94 KEYCODE_KANJI */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x95 KEYCODE_STOP */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x96 KEYCODE_AX */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x97 KEYCODE_UNLABELED */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x98 */
    WEB::WEBKEYCODE_MEDIA_TRACK_NEXT,            /* 0x99 KEYCODE_NEXTTRACK */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x9A */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x9B */
    WEB::WEBKEYCODE_NUMPAD_ENTER,                /* 0x9C KEYCODE_NUMPADENTER */
    WEB::WEBKEYCODE_CONTROL_RIGHT,               /* 0x9D KEYCODE_RCONTROL */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x9E */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0x9F */
    WEB::WEBKEYCODE_AUDIO_VOLUME_MUTE,           /* 0xA0 KEYCODE_MUTE */
    WEB::WEBKEYCODE_LAUNCH_APP2,                 /* 0xA1 KEYCODE_CALCULATOR */
    WEB::WEBKEYCODE_MEDIA_PLAY_PAUSE,            /* 0xA2 KEYCODE_PLAYPAUSE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA3 */
    WEB::WEBKEYCODE_MEDIA_STOP,                  /* 0xA4 KEYCODE_MEDIASTOP */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA5 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA6 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA7 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA8 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xA9 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xAA */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xAB */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xAC */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xAD */
    WEB::WEBKEYCODE_AUDIO_VOLUME_DOWN,           /* 0xAE KEYCODE_VOLUMEDOWN */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xAF */
    WEB::WEBKEYCODE_AUDIO_VOLUME_UP,             /* 0xB0 KEYCODE_VOLUMEUP */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xB1 */
    WEB::WEBKEYCODE_BROWSER_HOME,                /* 0xB2 KEYCODE_WEBHOME */
    WEB::WEBKEYCODE_NUMPAD_COMMA,                /* 0xB3 KEYCODE_NUMPADCOMMA */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xB4 */
    WEB::WEBKEYCODE_NUMPAD_DIVIDE,               /* 0xB5 KEYCODE_DIVIDE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xB6 */
    WEB::WEBKEYCODE_PRINT_SCREEN,                /* 0xB7 KEYCODE_SYSRQ */
    WEB::WEBKEYCODE_ALT_RIGHT,                   /* 0xB8 KEYCODE_RMENU */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xB9 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBA */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBB */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBC */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBD */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xBF */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC0 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC1 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC2 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC3 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC4 */
    WEB::WEBKEYCODE_PAUSE,                       /* 0xC5 KEYCODE_PAUSE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xC6 */
    WEB::WEBKEYCODE_HOME,                        /* 0xC7 KEYCODE_HOME */
    WEB::WEBKEYCODE_ARROW_UP,                    /* 0xC8 KEYCODE_UP */
    WEB::WEBKEYCODE_PAGE_UP,                     /* 0xC9 KEYCODE_PGUP */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xCA */
    WEB::WEBKEYCODE_ARROW_LEFT,                  /* 0xCB KEYCODE_LEFT */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xCC */
    WEB::WEBKEYCODE_ARROW_RIGHT,                 /* 0xCD KEYCODE_RIGHT */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xCE */
    WEB::WEBKEYCODE_END,                         /* 0xCF KEYCODE_END */
    WEB::WEBKEYCODE_ARROW_DOWN,                  /* 0xD0 KEYCODE_DOWN */
    WEB::WEBKEYCODE_PAGE_DOWN,                   /* 0xD1 KEYCODE_PGDOWN */
    WEB::WEBKEYCODE_INSERT,                      /* 0xD2 KEYCODE_INSERT */
    WEB::WEBKEYCODE_DELETE,                      /* 0xD3 KEYCODE_DELETE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD4 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD5 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD6 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD7 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD8 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xD9 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xDA */
    WEB::WEBKEYCODE_META_LEFT,                   /* 0xDB KEYCODE_LWIN */
    WEB::WEBKEYCODE_META_RIGHT,                  /* 0xDC KEYCODE_RWIN */
    WEB::WEBKEYCODE_CONTEXT_MENU,                /* 0xDD KEYCODE_APPS */
    WEB::WEBKEYCODE_POWER,                       /* 0xDE KEYCODE_POWER */
    WEB::WEBKEYCODE_SLEEP,                       /* 0xDF KEYCODE_SLEEP */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xE0 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xE1 */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xE2 */
    WEB::WEBKEYCODE_WAKE_UP,                     /* 0xE3 KEYCODE_WAKE */
    WEB::WEBKEYCODE_UNIDENTIFIED,                /* 0xE4 */
    WEB::WEBKEYCODE_BROWSER_SEARCH,              /* 0xE5 KEYCODE_WEBSEARCH */
    WEB::WEBKEYCODE_BROWSER_FAVORITES,           /* 0xE6 KEYCODE_WEBFAVORITES */
    WEB::WEBKEYCODE_BROWSER_REFRESH,             /* 0xE7 KEYCODE_WEBREFRESH */
    WEB::WEBKEYCODE_BROWSER_STOP,                /* 0xE8 KEYCODE_WEBSTOP */
    WEB::WEBKEYCODE_BROWSER_FORWARD,             /* 0xE9 KEYCODE_WEBFORWARD */
    WEB::WEBKEYCODE_BROWSER_BACK,                /* 0xEA KEYCODE_WEBBACK */
    WEB::WEBKEYCODE_LAUNCH_APP1,                 /* 0xEB KEYCODE_MYCOMPUTER */
    WEB::WEBKEYCODE_LAUNCH_MAIL,                 /* 0xEC KEYCODE_MAIL */
    WEB::WEBKEYCODE_MEDIA_SELECT                 /* 0xED KEYCODE_MEDIASELECT */
};

/** INPUT::KeyModifier flags and the TWebModifier flags they are */
struct SModifierPair
{
    UInt32 inputFlag;
    UInt32 webFlag;
};

const SModifierPair ModifierPairs[] =
{
    { INPUT::KEYMOD_SHIFT,      WEB::WEBMODIFIER_SHIFT },
    { INPUT::KEYMOD_CTRL,       WEB::WEBMODIFIER_CONTROL },
    { INPUT::KEYMOD_ALT,        WEB::WEBMODIFIER_ALT },
    { INPUT::KEYMOD_META,       WEB::WEBMODIFIER_META },
    { INPUT::KEYMOD_CAPSLOCK,   WEB::WEBMODIFIER_CAPSLOCK },
    { INPUT::KEYMOD_NUMLOCK,    WEB::WEBMODIFIER_NUMLOCK },
    { INPUT::KEYMOD_SCROLLLOCK, WEB::WEBMODIFIER_SCROLLLOCK }
};

const UInt32 ModifierPairCount = (UInt32) ( sizeof( ModifierPairs ) / sizeof( ModifierPairs[ 0 ] ) );

/** The DOM number of each TMouseButton of the GUI, indexed by it */
const WEB::TWebMouseButton WebMouseButtons[ MOUSEBUTTON_X2 + 1 ] =
{
    WEB::WEBMOUSEBUTTON_NONE,           /* MOUSEBUTTON_NONE */
    WEB::WEBMOUSEBUTTON_LEFT,           /* MOUSEBUTTON_LEFT */
    WEB::WEBMOUSEBUTTON_RIGHT,          /* MOUSEBUTTON_RIGHT */
    WEB::WEBMOUSEBUTTON_MIDDLE,         /* MOUSEBUTTON_MIDDLE */
    WEB::WEBMOUSEBUTTON_BACK,           /* MOUSEBUTTON_X1 */
    WEB::WEBMOUSEBUTTON_FORWARD         /* MOUSEBUTTON_X2 */
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

WEB::TWebKeyCode
CWebViewInputTranslator::GetWebKeyCode( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    return ( (Int32) keyCode >= 0 && (Int32) keyCode < (Int32) INPUT::KEYCODE_MAX ) ? WebKeyCodes[ keyCode ] : WEB::WEBKEYCODE_UNIDENTIFIED;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewInputTranslator::GetWebModifiers( UInt32 inputModifiers )
{GUCEF_TRACE;

    UInt32 webModifiers = 0;
    for ( UInt32 i=0; i<ModifierPairCount; ++i )
        if ( 0 != ( inputModifiers & ModifierPairs[ i ].inputFlag ) )
            webModifiers |= ModifierPairs[ i ].webFlag;
    return webModifiers;
}

/*-------------------------------------------------------------------------*/

WEB::TWebMouseButton
CWebViewInputTranslator::GetWebMouseButton( TMouseButton button )
{GUCEF_TRACE;

    return ( (Int32) button >= 0 && (Int32) button <= (Int32) MOUSEBUTTON_X2 ) ? WebMouseButtons[ button ] : WEB::WEBMOUSEBUTTON_NONE;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewInputTranslator::GetWebMouseButtonStates( UInt32 guiButtonStates )
{GUCEF_TRACE;

    UInt32 webButtonStates = 0;
    for ( Int32 button=MOUSEBUTTON_LEFT; button<=MOUSEBUTTON_X2; ++button )
        if ( 0 != ( guiButtonStates & ( 1u << (UInt32) button ) ) )
            webButtonStates |= WEB::CWebViewKeyCodes::GetMouseButtonFlag( WebMouseButtons[ button ] );
    return webButtonStates;
}

/*-------------------------------------------------------------------------*/

WEB::CWebViewInputEvent
CWebViewInputTranslator::KeyEvent( INPUT::KeyCode keyCode, bool down, UInt32 inputModifiers, bool isRepeat )
{GUCEF_TRACE;

    return WEB::CWebViewInputEvent::UsLayoutKey( GetWebKeyCode( keyCode ), down, GetWebModifiers( inputModifiers ), isRepeat );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
