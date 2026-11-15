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

#ifndef GUCEF_WEB_WEBVIEWINPUTTYPES_H
#define GUCEF_WEB_WEBVIEWINPUTTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_ETYPES_H
#include "gucefWEB_ETypes.h"
#define GUCEF_WEB_ETYPES_H
#endif /* GUCEF_WEB_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace WEB {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The input of web views follows the web standard (W3C UI Events), as values of the enumerations below and not as
 *  strings: a key event carries the physical key (KeyboardEvent.code) and the key value (KeyboardEvent.key), the latter
 *  as a named key or, for a key that types a character, as that character in UTF-8. The DOM names of the values, and
 *  the codes of the platforms (evdev, X11, Windows), are given on demand by the lookup tables of CWebViewKeyCodes.
 *  Nothing here depends on an input or user interface module: those translate their own key codes to these.
 *  The values are stable: new ones are only ever appended (before the COUNT values).
 */

/**
 *  The physical keys of a keyboard: the values of KeyboardEvent.code of the W3C "UI Events KeyboardEvent code Values",
 *  in the order of its sections. A code names the position of a key, independent of the keyboard layout: KeyQ is the
 *  key left of KeyW, also where the layout puts an A on it.
 */
enum EWebKeyCode
{
    WEBKEYCODE_UNIDENTIFIED = 0 ,   /**< "Unidentified": a key without a code value */

    /* writing system keys */
    WEBKEYCODE_BACKQUOTE ,
    WEBKEYCODE_BACKSLASH ,
    WEBKEYCODE_BRACKET_LEFT ,
    WEBKEYCODE_BRACKET_RIGHT ,
    WEBKEYCODE_COMMA ,
    WEBKEYCODE_DIGIT0 ,
    WEBKEYCODE_DIGIT1 ,
    WEBKEYCODE_DIGIT2 ,
    WEBKEYCODE_DIGIT3 ,
    WEBKEYCODE_DIGIT4 ,
    WEBKEYCODE_DIGIT5 ,
    WEBKEYCODE_DIGIT6 ,
    WEBKEYCODE_DIGIT7 ,
    WEBKEYCODE_DIGIT8 ,
    WEBKEYCODE_DIGIT9 ,
    WEBKEYCODE_EQUAL ,
    WEBKEYCODE_INTL_BACKSLASH ,
    WEBKEYCODE_INTL_RO ,
    WEBKEYCODE_INTL_YEN ,
    WEBKEYCODE_KEY_A ,
    WEBKEYCODE_KEY_B ,
    WEBKEYCODE_KEY_C ,
    WEBKEYCODE_KEY_D ,
    WEBKEYCODE_KEY_E ,
    WEBKEYCODE_KEY_F ,
    WEBKEYCODE_KEY_G ,
    WEBKEYCODE_KEY_H ,
    WEBKEYCODE_KEY_I ,
    WEBKEYCODE_KEY_J ,
    WEBKEYCODE_KEY_K ,
    WEBKEYCODE_KEY_L ,
    WEBKEYCODE_KEY_M ,
    WEBKEYCODE_KEY_N ,
    WEBKEYCODE_KEY_O ,
    WEBKEYCODE_KEY_P ,
    WEBKEYCODE_KEY_Q ,
    WEBKEYCODE_KEY_R ,
    WEBKEYCODE_KEY_S ,
    WEBKEYCODE_KEY_T ,
    WEBKEYCODE_KEY_U ,
    WEBKEYCODE_KEY_V ,
    WEBKEYCODE_KEY_W ,
    WEBKEYCODE_KEY_X ,
    WEBKEYCODE_KEY_Y ,
    WEBKEYCODE_KEY_Z ,
    WEBKEYCODE_MINUS ,
    WEBKEYCODE_PERIOD ,
    WEBKEYCODE_QUOTE ,
    WEBKEYCODE_SEMICOLON ,
    WEBKEYCODE_SLASH ,

    /* functional keys */
    WEBKEYCODE_ALT_LEFT ,
    WEBKEYCODE_ALT_RIGHT ,
    WEBKEYCODE_BACKSPACE ,
    WEBKEYCODE_CAPS_LOCK ,
    WEBKEYCODE_CONTEXT_MENU ,
    WEBKEYCODE_CONTROL_LEFT ,
    WEBKEYCODE_CONTROL_RIGHT ,
    WEBKEYCODE_ENTER ,
    WEBKEYCODE_META_LEFT ,
    WEBKEYCODE_META_RIGHT ,
    WEBKEYCODE_SHIFT_LEFT ,
    WEBKEYCODE_SHIFT_RIGHT ,
    WEBKEYCODE_SPACE ,
    WEBKEYCODE_TAB ,
    WEBKEYCODE_CONVERT ,
    WEBKEYCODE_KANA_MODE ,
    WEBKEYCODE_LANG1 ,
    WEBKEYCODE_LANG2 ,
    WEBKEYCODE_LANG3 ,
    WEBKEYCODE_LANG4 ,
    WEBKEYCODE_LANG5 ,
    WEBKEYCODE_NON_CONVERT ,

    /* control pad section */
    WEBKEYCODE_DELETE ,
    WEBKEYCODE_END ,
    WEBKEYCODE_HELP ,
    WEBKEYCODE_HOME ,
    WEBKEYCODE_INSERT ,
    WEBKEYCODE_PAGE_DOWN ,
    WEBKEYCODE_PAGE_UP ,

    /* arrow pad section */
    WEBKEYCODE_ARROW_DOWN ,
    WEBKEYCODE_ARROW_LEFT ,
    WEBKEYCODE_ARROW_RIGHT ,
    WEBKEYCODE_ARROW_UP ,

    /* numpad section */
    WEBKEYCODE_NUM_LOCK ,
    WEBKEYCODE_NUMPAD0 ,
    WEBKEYCODE_NUMPAD1 ,
    WEBKEYCODE_NUMPAD2 ,
    WEBKEYCODE_NUMPAD3 ,
    WEBKEYCODE_NUMPAD4 ,
    WEBKEYCODE_NUMPAD5 ,
    WEBKEYCODE_NUMPAD6 ,
    WEBKEYCODE_NUMPAD7 ,
    WEBKEYCODE_NUMPAD8 ,
    WEBKEYCODE_NUMPAD9 ,
    WEBKEYCODE_NUMPAD_ADD ,
    WEBKEYCODE_NUMPAD_BACKSPACE ,
    WEBKEYCODE_NUMPAD_CLEAR ,
    WEBKEYCODE_NUMPAD_CLEAR_ENTRY ,
    WEBKEYCODE_NUMPAD_COMMA ,
    WEBKEYCODE_NUMPAD_DECIMAL ,
    WEBKEYCODE_NUMPAD_DIVIDE ,
    WEBKEYCODE_NUMPAD_ENTER ,
    WEBKEYCODE_NUMPAD_EQUAL ,
    WEBKEYCODE_NUMPAD_HASH ,
    WEBKEYCODE_NUMPAD_MEMORY_ADD ,
    WEBKEYCODE_NUMPAD_MEMORY_CLEAR ,
    WEBKEYCODE_NUMPAD_MEMORY_RECALL ,
    WEBKEYCODE_NUMPAD_MEMORY_STORE ,
    WEBKEYCODE_NUMPAD_MEMORY_SUBTRACT ,
    WEBKEYCODE_NUMPAD_MULTIPLY ,
    WEBKEYCODE_NUMPAD_PAREN_LEFT ,
    WEBKEYCODE_NUMPAD_PAREN_RIGHT ,
    WEBKEYCODE_NUMPAD_STAR ,
    WEBKEYCODE_NUMPAD_SUBTRACT ,

    /* function section */
    WEBKEYCODE_ESCAPE ,
    WEBKEYCODE_F1 ,
    WEBKEYCODE_F2 ,
    WEBKEYCODE_F3 ,
    WEBKEYCODE_F4 ,
    WEBKEYCODE_F5 ,
    WEBKEYCODE_F6 ,
    WEBKEYCODE_F7 ,
    WEBKEYCODE_F8 ,
    WEBKEYCODE_F9 ,
    WEBKEYCODE_F10 ,
    WEBKEYCODE_F11 ,
    WEBKEYCODE_F12 ,
    WEBKEYCODE_F13 ,
    WEBKEYCODE_F14 ,
    WEBKEYCODE_F15 ,
    WEBKEYCODE_F16 ,
    WEBKEYCODE_F17 ,
    WEBKEYCODE_F18 ,
    WEBKEYCODE_F19 ,
    WEBKEYCODE_F20 ,
    WEBKEYCODE_F21 ,
    WEBKEYCODE_F22 ,
    WEBKEYCODE_F23 ,
    WEBKEYCODE_F24 ,
    WEBKEYCODE_FN ,
    WEBKEYCODE_FN_LOCK ,
    WEBKEYCODE_PRINT_SCREEN ,
    WEBKEYCODE_SCROLL_LOCK ,
    WEBKEYCODE_PAUSE ,

    /* media keys */
    WEBKEYCODE_BROWSER_BACK ,
    WEBKEYCODE_BROWSER_FAVORITES ,
    WEBKEYCODE_BROWSER_FORWARD ,
    WEBKEYCODE_BROWSER_HOME ,
    WEBKEYCODE_BROWSER_REFRESH ,
    WEBKEYCODE_BROWSER_SEARCH ,
    WEBKEYCODE_BROWSER_STOP ,
    WEBKEYCODE_EJECT ,
    WEBKEYCODE_LAUNCH_APP1 ,
    WEBKEYCODE_LAUNCH_APP2 ,
    WEBKEYCODE_LAUNCH_MAIL ,
    WEBKEYCODE_MEDIA_PLAY_PAUSE ,
    WEBKEYCODE_MEDIA_SELECT ,
    WEBKEYCODE_MEDIA_STOP ,
    WEBKEYCODE_MEDIA_TRACK_NEXT ,
    WEBKEYCODE_MEDIA_TRACK_PREVIOUS ,
    WEBKEYCODE_POWER ,
    WEBKEYCODE_SLEEP ,
    WEBKEYCODE_AUDIO_VOLUME_DOWN ,
    WEBKEYCODE_AUDIO_VOLUME_MUTE ,
    WEBKEYCODE_AUDIO_VOLUME_UP ,
    WEBKEYCODE_WAKE_UP ,

    /* legacy, non-standard and special keys */
    WEBKEYCODE_HYPER ,
    WEBKEYCODE_SUPER ,
    WEBKEYCODE_TURBO ,
    WEBKEYCODE_ABORT ,
    WEBKEYCODE_RESUME ,
    WEBKEYCODE_SUSPEND ,
    WEBKEYCODE_AGAIN ,
    WEBKEYCODE_COPY ,
    WEBKEYCODE_CUT ,
    WEBKEYCODE_FIND ,
    WEBKEYCODE_OPEN ,
    WEBKEYCODE_PASTE ,
    WEBKEYCODE_PROPS ,
    WEBKEYCODE_SELECT ,
    WEBKEYCODE_UNDO ,
    WEBKEYCODE_HIRAGANA ,
    WEBKEYCODE_KATAKANA ,

    WEBKEYCODE_COUNT                /**< the number of key codes, not a key */
};
typedef enum EWebKeyCode TWebKeyCode;

/**
 *  The key values of the W3C "UI Events KeyboardEvent key Values" that are not characters (KeyboardEvent.key), in the
 *  order of the sections of that specification; the television and media controller sections are not included. A key
 *  that types a character has WEBNAMEDKEY_CHARACTER and the character as UTF-8 text.
 */
enum EWebNamedKey
{
    WEBNAMEDKEY_UNIDENTIFIED = 0 ,  /**< "Unidentified": the key value cannot be determined */
    WEBNAMEDKEY_CHARACTER ,         /**< not a named key: the key value is a character, carried as UTF-8 text */

    /* modifier keys */
    WEBNAMEDKEY_ALT ,
    WEBNAMEDKEY_ALT_GRAPH ,
    WEBNAMEDKEY_CAPS_LOCK ,
    WEBNAMEDKEY_CONTROL ,
    WEBNAMEDKEY_FN ,
    WEBNAMEDKEY_FN_LOCK ,
    WEBNAMEDKEY_META ,
    WEBNAMEDKEY_NUM_LOCK ,
    WEBNAMEDKEY_SCROLL_LOCK ,
    WEBNAMEDKEY_SHIFT ,
    WEBNAMEDKEY_SYMBOL ,
    WEBNAMEDKEY_SYMBOL_LOCK ,
    WEBNAMEDKEY_HYPER ,
    WEBNAMEDKEY_SUPER ,

    /* whitespace keys */
    WEBNAMEDKEY_ENTER ,
    WEBNAMEDKEY_TAB ,

    /* navigation keys */
    WEBNAMEDKEY_ARROW_DOWN ,
    WEBNAMEDKEY_ARROW_LEFT ,
    WEBNAMEDKEY_ARROW_RIGHT ,
    WEBNAMEDKEY_ARROW_UP ,
    WEBNAMEDKEY_END ,
    WEBNAMEDKEY_HOME ,
    WEBNAMEDKEY_PAGE_DOWN ,
    WEBNAMEDKEY_PAGE_UP ,

    /* editing keys */
    WEBNAMEDKEY_BACKSPACE ,
    WEBNAMEDKEY_CLEAR ,
    WEBNAMEDKEY_COPY ,
    WEBNAMEDKEY_CR_SEL ,
    WEBNAMEDKEY_CUT ,
    WEBNAMEDKEY_DELETE ,
    WEBNAMEDKEY_ERASE_EOF ,
    WEBNAMEDKEY_EX_SEL ,
    WEBNAMEDKEY_INSERT ,
    WEBNAMEDKEY_PASTE ,
    WEBNAMEDKEY_REDO ,
    WEBNAMEDKEY_UNDO ,

    /* UI keys */
    WEBNAMEDKEY_ACCEPT ,
    WEBNAMEDKEY_AGAIN ,
    WEBNAMEDKEY_ATTN ,
    WEBNAMEDKEY_CANCEL ,
    WEBNAMEDKEY_CONTEXT_MENU ,
    WEBNAMEDKEY_ESCAPE ,
    WEBNAMEDKEY_EXECUTE ,
    WEBNAMEDKEY_FIND ,
    WEBNAMEDKEY_HELP ,
    WEBNAMEDKEY_PAUSE ,
    WEBNAMEDKEY_PLAY ,
    WEBNAMEDKEY_PROPS ,
    WEBNAMEDKEY_SELECT ,
    WEBNAMEDKEY_ZOOM_IN ,
    WEBNAMEDKEY_ZOOM_OUT ,

    /* device keys */
    WEBNAMEDKEY_BRIGHTNESS_DOWN ,
    WEBNAMEDKEY_BRIGHTNESS_UP ,
    WEBNAMEDKEY_EJECT ,
    WEBNAMEDKEY_LOG_OFF ,
    WEBNAMEDKEY_POWER ,
    WEBNAMEDKEY_POWER_OFF ,
    WEBNAMEDKEY_PRINT_SCREEN ,
    WEBNAMEDKEY_HIBERNATE ,
    WEBNAMEDKEY_STANDBY ,
    WEBNAMEDKEY_WAKE_UP ,

    /* IME and composition keys */
    WEBNAMEDKEY_ALL_CANDIDATES ,
    WEBNAMEDKEY_ALPHANUMERIC ,
    WEBNAMEDKEY_CODE_INPUT ,
    WEBNAMEDKEY_COMPOSE ,
    WEBNAMEDKEY_CONVERT ,
    WEBNAMEDKEY_DEAD ,
    WEBNAMEDKEY_FINAL_MODE ,
    WEBNAMEDKEY_GROUP_FIRST ,
    WEBNAMEDKEY_GROUP_LAST ,
    WEBNAMEDKEY_GROUP_NEXT ,
    WEBNAMEDKEY_GROUP_PREVIOUS ,
    WEBNAMEDKEY_MODE_CHANGE ,
    WEBNAMEDKEY_NEXT_CANDIDATE ,
    WEBNAMEDKEY_NON_CONVERT ,
    WEBNAMEDKEY_PREVIOUS_CANDIDATE ,
    WEBNAMEDKEY_PROCESS ,
    WEBNAMEDKEY_SINGLE_CANDIDATE ,

    /* Korean keyboards */
    WEBNAMEDKEY_HANGUL_MODE ,
    WEBNAMEDKEY_HANJA_MODE ,
    WEBNAMEDKEY_JUNJA_MODE ,

    /* Japanese keyboards */
    WEBNAMEDKEY_EISU ,
    WEBNAMEDKEY_HANKAKU ,
    WEBNAMEDKEY_HIRAGANA ,
    WEBNAMEDKEY_HIRAGANA_KATAKANA ,
    WEBNAMEDKEY_KANA_MODE ,
    WEBNAMEDKEY_KANJI_MODE ,
    WEBNAMEDKEY_KATAKANA ,
    WEBNAMEDKEY_ROMAJI ,
    WEBNAMEDKEY_ZENKAKU ,
    WEBNAMEDKEY_ZENKAKU_HANKAKU ,

    /* general purpose function keys */
    WEBNAMEDKEY_F1 ,
    WEBNAMEDKEY_F2 ,
    WEBNAMEDKEY_F3 ,
    WEBNAMEDKEY_F4 ,
    WEBNAMEDKEY_F5 ,
    WEBNAMEDKEY_F6 ,
    WEBNAMEDKEY_F7 ,
    WEBNAMEDKEY_F8 ,
    WEBNAMEDKEY_F9 ,
    WEBNAMEDKEY_F10 ,
    WEBNAMEDKEY_F11 ,
    WEBNAMEDKEY_F12 ,
    WEBNAMEDKEY_F13 ,
    WEBNAMEDKEY_F14 ,
    WEBNAMEDKEY_F15 ,
    WEBNAMEDKEY_F16 ,
    WEBNAMEDKEY_F17 ,
    WEBNAMEDKEY_F18 ,
    WEBNAMEDKEY_F19 ,
    WEBNAMEDKEY_F20 ,
    WEBNAMEDKEY_F21 ,
    WEBNAMEDKEY_F22 ,
    WEBNAMEDKEY_F23 ,
    WEBNAMEDKEY_F24 ,
    WEBNAMEDKEY_SOFT1 ,
    WEBNAMEDKEY_SOFT2 ,
    WEBNAMEDKEY_SOFT3 ,
    WEBNAMEDKEY_SOFT4 ,

    /* multimedia keys */
    WEBNAMEDKEY_CHANNEL_DOWN ,
    WEBNAMEDKEY_CHANNEL_UP ,
    WEBNAMEDKEY_CLOSE ,
    WEBNAMEDKEY_MAIL_FORWARD ,
    WEBNAMEDKEY_MAIL_REPLY ,
    WEBNAMEDKEY_MAIL_SEND ,
    WEBNAMEDKEY_MEDIA_CLOSE ,
    WEBNAMEDKEY_MEDIA_FAST_FORWARD ,
    WEBNAMEDKEY_MEDIA_PAUSE ,
    WEBNAMEDKEY_MEDIA_PLAY ,
    WEBNAMEDKEY_MEDIA_PLAY_PAUSE ,
    WEBNAMEDKEY_MEDIA_RECORD ,
    WEBNAMEDKEY_MEDIA_REWIND ,
    WEBNAMEDKEY_MEDIA_STOP ,
    WEBNAMEDKEY_MEDIA_TRACK_NEXT ,
    WEBNAMEDKEY_MEDIA_TRACK_PREVIOUS ,
    WEBNAMEDKEY_NEW ,
    WEBNAMEDKEY_OPEN ,
    WEBNAMEDKEY_PRINT ,
    WEBNAMEDKEY_SAVE ,
    WEBNAMEDKEY_SPELL_CHECK ,

    /* multimedia numpad keys */
    WEBNAMEDKEY_KEY11 ,
    WEBNAMEDKEY_KEY12 ,

    /* audio keys */
    WEBNAMEDKEY_AUDIO_BALANCE_LEFT ,
    WEBNAMEDKEY_AUDIO_BALANCE_RIGHT ,
    WEBNAMEDKEY_AUDIO_BASS_BOOST_DOWN ,
    WEBNAMEDKEY_AUDIO_BASS_BOOST_TOGGLE ,
    WEBNAMEDKEY_AUDIO_BASS_BOOST_UP ,
    WEBNAMEDKEY_AUDIO_FADER_FRONT ,
    WEBNAMEDKEY_AUDIO_FADER_REAR ,
    WEBNAMEDKEY_AUDIO_SURROUND_MODE_NEXT ,
    WEBNAMEDKEY_AUDIO_TREBLE_DOWN ,
    WEBNAMEDKEY_AUDIO_TREBLE_UP ,
    WEBNAMEDKEY_AUDIO_VOLUME_DOWN ,
    WEBNAMEDKEY_AUDIO_VOLUME_UP ,
    WEBNAMEDKEY_AUDIO_VOLUME_MUTE ,
    WEBNAMEDKEY_MICROPHONE_TOGGLE ,
    WEBNAMEDKEY_MICROPHONE_VOLUME_DOWN ,
    WEBNAMEDKEY_MICROPHONE_VOLUME_UP ,
    WEBNAMEDKEY_MICROPHONE_VOLUME_MUTE ,

    /* speech keys */
    WEBNAMEDKEY_SPEECH_CORRECTION_LIST ,
    WEBNAMEDKEY_SPEECH_INPUT_TOGGLE ,

    /* application keys */
    WEBNAMEDKEY_LAUNCH_APPLICATION1 ,
    WEBNAMEDKEY_LAUNCH_APPLICATION2 ,
    WEBNAMEDKEY_LAUNCH_CALENDAR ,
    WEBNAMEDKEY_LAUNCH_CONTACTS ,
    WEBNAMEDKEY_LAUNCH_MAIL ,
    WEBNAMEDKEY_LAUNCH_MEDIA_PLAYER ,
    WEBNAMEDKEY_LAUNCH_MUSIC_PLAYER ,
    WEBNAMEDKEY_LAUNCH_PHONE ,
    WEBNAMEDKEY_LAUNCH_SCREEN_SAVER ,
    WEBNAMEDKEY_LAUNCH_SPREADSHEET ,
    WEBNAMEDKEY_LAUNCH_WEB_BROWSER ,
    WEBNAMEDKEY_LAUNCH_WEB_CAM ,
    WEBNAMEDKEY_LAUNCH_WORD_PROCESSOR ,

    /* browser keys */
    WEBNAMEDKEY_BROWSER_BACK ,
    WEBNAMEDKEY_BROWSER_FAVORITES ,
    WEBNAMEDKEY_BROWSER_FORWARD ,
    WEBNAMEDKEY_BROWSER_HOME ,
    WEBNAMEDKEY_BROWSER_REFRESH ,
    WEBNAMEDKEY_BROWSER_SEARCH ,
    WEBNAMEDKEY_BROWSER_STOP ,

    /* mobile phone keys */
    WEBNAMEDKEY_APP_SWITCH ,
    WEBNAMEDKEY_CALL ,
    WEBNAMEDKEY_CAMERA ,
    WEBNAMEDKEY_CAMERA_FOCUS ,
    WEBNAMEDKEY_END_CALL ,
    WEBNAMEDKEY_GO_BACK ,
    WEBNAMEDKEY_GO_HOME ,
    WEBNAMEDKEY_HEADSET_HOOK ,
    WEBNAMEDKEY_LAST_NUMBER_REDIAL ,
    WEBNAMEDKEY_NOTIFICATION ,
    WEBNAMEDKEY_MANNER_MODE ,
    WEBNAMEDKEY_VOICE_DIAL ,

    WEBNAMEDKEY_COUNT               /**< the number of named keys, not a key */
};
typedef enum EWebNamedKey TWebNamedKey;

/** Where a key is on the keyboard: KeyboardEvent.location */
enum EWebKeyLocation
{
    WEBKEYLOCATION_STANDARD             = 0 ,
    WEBKEYLOCATION_LEFT                 = 1 ,   /**< the left one of a key that is there twice (ShiftLeft) */
    WEBKEYLOCATION_RIGHT                = 2 ,
    WEBKEYLOCATION_NUMPAD               = 3
};
typedef enum EWebKeyLocation TWebKeyLocation;

/**
 *  The modifier state of an input event, as flags (KeyboardEvent.getModifierState()). The flags do not tell the left
 *  and the right key apart: the key code of a key event does (ShiftLeft, ShiftRight).
 */
enum EWebModifier
{
    WEBMODIFIER_NONE                    = 0x0000 ,
    WEBMODIFIER_SHIFT                   = 0x0001 ,
    WEBMODIFIER_CONTROL                 = 0x0002 ,
    WEBMODIFIER_ALT                     = 0x0004 ,
    WEBMODIFIER_META                    = 0x0008 ,
    WEBMODIFIER_CAPSLOCK                = 0x0010 ,
    WEBMODIFIER_NUMLOCK                 = 0x0020 ,
    WEBMODIFIER_SCROLLLOCK              = 0x0040 ,
    WEBMODIFIER_ALTGRAPH                = 0x0080
};
typedef enum EWebModifier TWebModifier;

/** A mouse button, numbered as MouseEvent.button of the DOM; WEBMOUSEBUTTON_NONE (-1) for events without a button */
enum EWebMouseButton
{
    WEBMOUSEBUTTON_NONE                 = -1 ,
    WEBMOUSEBUTTON_LEFT                 = 0  ,  /**< the main button */
    WEBMOUSEBUTTON_MIDDLE               = 1  ,  /**< the auxiliary button, usually the wheel */
    WEBMOUSEBUTTON_RIGHT                = 2  ,  /**< the secondary button */
    WEBMOUSEBUTTON_BACK                 = 3  ,  /**< the fourth button, usually browser back */
    WEBMOUSEBUTTON_FORWARD              = 4  ,  /**< the fifth button, usually browser forward */

    WEBMOUSEBUTTON_COUNT                = 5     /**< the number of buttons, not a button */
};
typedef enum EWebMouseButton TWebMouseButton;

/** The buttons held down, as flags numbered as MouseEvent.buttons of the DOM (note: right is 2, middle is 4) */
enum EWebMouseButtonFlag
{
    WEBMOUSEBUTTONFLAG_NONE             = 0x00 ,
    WEBMOUSEBUTTONFLAG_LEFT             = 0x01 ,
    WEBMOUSEBUTTONFLAG_RIGHT            = 0x02 ,
    WEBMOUSEBUTTONFLAG_MIDDLE           = 0x04 ,
    WEBMOUSEBUTTONFLAG_BACK             = 0x08 ,
    WEBMOUSEBUTTONFLAG_FORWARD          = 0x10
};
typedef enum EWebMouseButtonFlag TWebMouseButtonFlag;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace WEB */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_WEBVIEWINPUTTYPES_H ? */
