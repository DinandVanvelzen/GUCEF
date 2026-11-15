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

#include <string.h>

#include "gucefWEB_CWebViewKeyCodes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The DOM KeyboardEvent.code names, indexed by TWebKeyCode */
const char* const DomCodeNames[ WEBKEYCODE_COUNT ] =
{
    "Unidentified",
    "Backquote",
    "Backslash",
    "BracketLeft",
    "BracketRight",
    "Comma",
    "Digit0",
    "Digit1",
    "Digit2",
    "Digit3",
    "Digit4",
    "Digit5",
    "Digit6",
    "Digit7",
    "Digit8",
    "Digit9",
    "Equal",
    "IntlBackslash",
    "IntlRo",
    "IntlYen",
    "KeyA",
    "KeyB",
    "KeyC",
    "KeyD",
    "KeyE",
    "KeyF",
    "KeyG",
    "KeyH",
    "KeyI",
    "KeyJ",
    "KeyK",
    "KeyL",
    "KeyM",
    "KeyN",
    "KeyO",
    "KeyP",
    "KeyQ",
    "KeyR",
    "KeyS",
    "KeyT",
    "KeyU",
    "KeyV",
    "KeyW",
    "KeyX",
    "KeyY",
    "KeyZ",
    "Minus",
    "Period",
    "Quote",
    "Semicolon",
    "Slash",
    "AltLeft",
    "AltRight",
    "Backspace",
    "CapsLock",
    "ContextMenu",
    "ControlLeft",
    "ControlRight",
    "Enter",
    "MetaLeft",
    "MetaRight",
    "ShiftLeft",
    "ShiftRight",
    "Space",
    "Tab",
    "Convert",
    "KanaMode",
    "Lang1",
    "Lang2",
    "Lang3",
    "Lang4",
    "Lang5",
    "NonConvert",
    "Delete",
    "End",
    "Help",
    "Home",
    "Insert",
    "PageDown",
    "PageUp",
    "ArrowDown",
    "ArrowLeft",
    "ArrowRight",
    "ArrowUp",
    "NumLock",
    "Numpad0",
    "Numpad1",
    "Numpad2",
    "Numpad3",
    "Numpad4",
    "Numpad5",
    "Numpad6",
    "Numpad7",
    "Numpad8",
    "Numpad9",
    "NumpadAdd",
    "NumpadBackspace",
    "NumpadClear",
    "NumpadClearEntry",
    "NumpadComma",
    "NumpadDecimal",
    "NumpadDivide",
    "NumpadEnter",
    "NumpadEqual",
    "NumpadHash",
    "NumpadMemoryAdd",
    "NumpadMemoryClear",
    "NumpadMemoryRecall",
    "NumpadMemoryStore",
    "NumpadMemorySubtract",
    "NumpadMultiply",
    "NumpadParenLeft",
    "NumpadParenRight",
    "NumpadStar",
    "NumpadSubtract",
    "Escape",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Fn",
    "FnLock",
    "PrintScreen",
    "ScrollLock",
    "Pause",
    "BrowserBack",
    "BrowserFavorites",
    "BrowserForward",
    "BrowserHome",
    "BrowserRefresh",
    "BrowserSearch",
    "BrowserStop",
    "Eject",
    "LaunchApp1",
    "LaunchApp2",
    "LaunchMail",
    "MediaPlayPause",
    "MediaSelect",
    "MediaStop",
    "MediaTrackNext",
    "MediaTrackPrevious",
    "Power",
    "Sleep",
    "AudioVolumeDown",
    "AudioVolumeMute",
    "AudioVolumeUp",
    "WakeUp",
    "Hyper",
    "Super",
    "Turbo",
    "Abort",
    "Resume",
    "Suspend",
    "Again",
    "Copy",
    "Cut",
    "Find",
    "Open",
    "Paste",
    "Props",
    "Select",
    "Undo",
    "Hiragana",
    "Katakana"
};

/** The DOM KeyboardEvent.key names, indexed by TWebNamedKey */
const char* const DomKeyNames[ WEBNAMEDKEY_COUNT ] =
{
    "Unidentified",
    "",                     /* WEBNAMEDKEY_CHARACTER: no name of its own, the key value is the text */
    "Alt",
    "AltGraph",
    "CapsLock",
    "Control",
    "Fn",
    "FnLock",
    "Meta",
    "NumLock",
    "ScrollLock",
    "Shift",
    "Symbol",
    "SymbolLock",
    "Hyper",
    "Super",
    "Enter",
    "Tab",
    "ArrowDown",
    "ArrowLeft",
    "ArrowRight",
    "ArrowUp",
    "End",
    "Home",
    "PageDown",
    "PageUp",
    "Backspace",
    "Clear",
    "Copy",
    "CrSel",
    "Cut",
    "Delete",
    "EraseEof",
    "ExSel",
    "Insert",
    "Paste",
    "Redo",
    "Undo",
    "Accept",
    "Again",
    "Attn",
    "Cancel",
    "ContextMenu",
    "Escape",
    "Execute",
    "Find",
    "Help",
    "Pause",
    "Play",
    "Props",
    "Select",
    "ZoomIn",
    "ZoomOut",
    "BrightnessDown",
    "BrightnessUp",
    "Eject",
    "LogOff",
    "Power",
    "PowerOff",
    "PrintScreen",
    "Hibernate",
    "Standby",
    "WakeUp",
    "AllCandidates",
    "Alphanumeric",
    "CodeInput",
    "Compose",
    "Convert",
    "Dead",
    "FinalMode",
    "GroupFirst",
    "GroupLast",
    "GroupNext",
    "GroupPrevious",
    "ModeChange",
    "NextCandidate",
    "NonConvert",
    "PreviousCandidate",
    "Process",
    "SingleCandidate",
    "HangulMode",
    "HanjaMode",
    "JunjaMode",
    "Eisu",
    "Hankaku",
    "Hiragana",
    "HiraganaKatakana",
    "KanaMode",
    "KanjiMode",
    "Katakana",
    "Romaji",
    "Zenkaku",
    "ZenkakuHankaku",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Soft1",
    "Soft2",
    "Soft3",
    "Soft4",
    "ChannelDown",
    "ChannelUp",
    "Close",
    "MailForward",
    "MailReply",
    "MailSend",
    "MediaClose",
    "MediaFastForward",
    "MediaPause",
    "MediaPlay",
    "MediaPlayPause",
    "MediaRecord",
    "MediaRewind",
    "MediaStop",
    "MediaTrackNext",
    "MediaTrackPrevious",
    "New",
    "Open",
    "Print",
    "Save",
    "SpellCheck",
    "Key11",
    "Key12",
    "AudioBalanceLeft",
    "AudioBalanceRight",
    "AudioBassBoostDown",
    "AudioBassBoostToggle",
    "AudioBassBoostUp",
    "AudioFaderFront",
    "AudioFaderRear",
    "AudioSurroundModeNext",
    "AudioTrebleDown",
    "AudioTrebleUp",
    "AudioVolumeDown",
    "AudioVolumeUp",
    "AudioVolumeMute",
    "MicrophoneToggle",
    "MicrophoneVolumeDown",
    "MicrophoneVolumeUp",
    "MicrophoneVolumeMute",
    "SpeechCorrectionList",
    "SpeechInputToggle",
    "LaunchApplication1",
    "LaunchApplication2",
    "LaunchCalendar",
    "LaunchContacts",
    "LaunchMail",
    "LaunchMediaPlayer",
    "LaunchMusicPlayer",
    "LaunchPhone",
    "LaunchScreenSaver",
    "LaunchSpreadsheet",
    "LaunchWebBrowser",
    "LaunchWebCam",
    "LaunchWordProcessor",
    "BrowserBack",
    "BrowserFavorites",
    "BrowserForward",
    "BrowserHome",
    "BrowserRefresh",
    "BrowserSearch",
    "BrowserStop",
    "AppSwitch",
    "Call",
    "Camera",
    "CameraFocus",
    "EndCall",
    "GoBack",
    "GoHome",
    "HeadsetHook",
    "LastNumberRedial",
    "Notification",
    "MannerMode",
    "VoiceDial"
};

/** What is known of a physical key, indexed by TWebKeyCode */
struct SKeyCodeInfo
{
    TWebKeyLocation location;
    UInt16 evdevCode;               /**< Linux KEY_*, 0 for none */
    UInt16 windowsScanCode;         /**< set 1, 0xE0xx extended, 0 for none */
    UInt8 windowsVirtualKey;        /**< VK_* of the US layout, 0 for none */
    TWebNamedKey usKey;             /**< the key value on the US layout */
    const char* usText;             /**< its character when usKey is WEBNAMEDKEY_CHARACTER */
    TWebNamedKey usKeyShifted;      /**< the same with shift */
    const char* usTextShifted;
};

/*
 *  location, evdev code, Windows scan code, Windows virtual key, US key, US text, US key with shift, US text with shift.
 *  The evdev codes are those of linux/input-event-codes.h; the scan codes and virtual keys those of Windows. Keys only a
 *  few keyboards have, of which no code is established, have 0. Hiragana and Katakana are the legacy names of Lang4 and
 *  Lang3, which carry the codes.
 */
const SKeyCodeInfo KeyCodeTable[ WEBKEYCODE_COUNT ] =
{
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* Unidentified */
    { WEBKEYLOCATION_STANDARD, 41,  0x0029, 0xC0, WEBNAMEDKEY_CHARACTER,            "`",        WEBNAMEDKEY_CHARACTER,            "~"        },   /* Backquote */
    { WEBKEYLOCATION_STANDARD, 43,  0x002B, 0xDC, WEBNAMEDKEY_CHARACTER,            "\\",       WEBNAMEDKEY_CHARACTER,            "|"        },   /* Backslash */
    { WEBKEYLOCATION_STANDARD, 26,  0x001A, 0xDB, WEBNAMEDKEY_CHARACTER,            "[",        WEBNAMEDKEY_CHARACTER,            "{"        },   /* BracketLeft */
    { WEBKEYLOCATION_STANDARD, 27,  0x001B, 0xDD, WEBNAMEDKEY_CHARACTER,            "]",        WEBNAMEDKEY_CHARACTER,            "}"        },   /* BracketRight */
    { WEBKEYLOCATION_STANDARD, 51,  0x0033, 0xBC, WEBNAMEDKEY_CHARACTER,            ",",        WEBNAMEDKEY_CHARACTER,            "<"        },   /* Comma */
    { WEBKEYLOCATION_STANDARD, 11,  0x000B, 0x30, WEBNAMEDKEY_CHARACTER,            "0",        WEBNAMEDKEY_CHARACTER,            ")"        },   /* Digit0 */
    { WEBKEYLOCATION_STANDARD, 2,   0x0002, 0x31, WEBNAMEDKEY_CHARACTER,            "1",        WEBNAMEDKEY_CHARACTER,            "!"        },   /* Digit1 */
    { WEBKEYLOCATION_STANDARD, 3,   0x0003, 0x32, WEBNAMEDKEY_CHARACTER,            "2",        WEBNAMEDKEY_CHARACTER,            "@"        },   /* Digit2 */
    { WEBKEYLOCATION_STANDARD, 4,   0x0004, 0x33, WEBNAMEDKEY_CHARACTER,            "3",        WEBNAMEDKEY_CHARACTER,            "#"        },   /* Digit3 */
    { WEBKEYLOCATION_STANDARD, 5,   0x0005, 0x34, WEBNAMEDKEY_CHARACTER,            "4",        WEBNAMEDKEY_CHARACTER,            "$"        },   /* Digit4 */
    { WEBKEYLOCATION_STANDARD, 6,   0x0006, 0x35, WEBNAMEDKEY_CHARACTER,            "5",        WEBNAMEDKEY_CHARACTER,            "%"        },   /* Digit5 */
    { WEBKEYLOCATION_STANDARD, 7,   0x0007, 0x36, WEBNAMEDKEY_CHARACTER,            "6",        WEBNAMEDKEY_CHARACTER,            "^"        },   /* Digit6 */
    { WEBKEYLOCATION_STANDARD, 8,   0x0008, 0x37, WEBNAMEDKEY_CHARACTER,            "7",        WEBNAMEDKEY_CHARACTER,            "&"        },   /* Digit7 */
    { WEBKEYLOCATION_STANDARD, 9,   0x0009, 0x38, WEBNAMEDKEY_CHARACTER,            "8",        WEBNAMEDKEY_CHARACTER,            "*"        },   /* Digit8 */
    { WEBKEYLOCATION_STANDARD, 10,  0x000A, 0x39, WEBNAMEDKEY_CHARACTER,            "9",        WEBNAMEDKEY_CHARACTER,            "("        },   /* Digit9 */
    { WEBKEYLOCATION_STANDARD, 13,  0x000D, 0xBB, WEBNAMEDKEY_CHARACTER,            "=",        WEBNAMEDKEY_CHARACTER,            "+"        },   /* Equal */
    { WEBKEYLOCATION_STANDARD, 86,  0x0056, 0xE2, WEBNAMEDKEY_CHARACTER,            "\\",       WEBNAMEDKEY_CHARACTER,            "|"        },   /* IntlBackslash */
    { WEBKEYLOCATION_STANDARD, 89,  0x0073, 0xC1, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* IntlRo */
    { WEBKEYLOCATION_STANDARD, 124, 0x007D, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* IntlYen */
    { WEBKEYLOCATION_STANDARD, 30,  0x001E, 0x41, WEBNAMEDKEY_CHARACTER,            "a",        WEBNAMEDKEY_CHARACTER,            "A"        },   /* KeyA */
    { WEBKEYLOCATION_STANDARD, 48,  0x0030, 0x42, WEBNAMEDKEY_CHARACTER,            "b",        WEBNAMEDKEY_CHARACTER,            "B"        },   /* KeyB */
    { WEBKEYLOCATION_STANDARD, 46,  0x002E, 0x43, WEBNAMEDKEY_CHARACTER,            "c",        WEBNAMEDKEY_CHARACTER,            "C"        },   /* KeyC */
    { WEBKEYLOCATION_STANDARD, 32,  0x0020, 0x44, WEBNAMEDKEY_CHARACTER,            "d",        WEBNAMEDKEY_CHARACTER,            "D"        },   /* KeyD */
    { WEBKEYLOCATION_STANDARD, 18,  0x0012, 0x45, WEBNAMEDKEY_CHARACTER,            "e",        WEBNAMEDKEY_CHARACTER,            "E"        },   /* KeyE */
    { WEBKEYLOCATION_STANDARD, 33,  0x0021, 0x46, WEBNAMEDKEY_CHARACTER,            "f",        WEBNAMEDKEY_CHARACTER,            "F"        },   /* KeyF */
    { WEBKEYLOCATION_STANDARD, 34,  0x0022, 0x47, WEBNAMEDKEY_CHARACTER,            "g",        WEBNAMEDKEY_CHARACTER,            "G"        },   /* KeyG */
    { WEBKEYLOCATION_STANDARD, 35,  0x0023, 0x48, WEBNAMEDKEY_CHARACTER,            "h",        WEBNAMEDKEY_CHARACTER,            "H"        },   /* KeyH */
    { WEBKEYLOCATION_STANDARD, 23,  0x0017, 0x49, WEBNAMEDKEY_CHARACTER,            "i",        WEBNAMEDKEY_CHARACTER,            "I"        },   /* KeyI */
    { WEBKEYLOCATION_STANDARD, 36,  0x0024, 0x4A, WEBNAMEDKEY_CHARACTER,            "j",        WEBNAMEDKEY_CHARACTER,            "J"        },   /* KeyJ */
    { WEBKEYLOCATION_STANDARD, 37,  0x0025, 0x4B, WEBNAMEDKEY_CHARACTER,            "k",        WEBNAMEDKEY_CHARACTER,            "K"        },   /* KeyK */
    { WEBKEYLOCATION_STANDARD, 38,  0x0026, 0x4C, WEBNAMEDKEY_CHARACTER,            "l",        WEBNAMEDKEY_CHARACTER,            "L"        },   /* KeyL */
    { WEBKEYLOCATION_STANDARD, 50,  0x0032, 0x4D, WEBNAMEDKEY_CHARACTER,            "m",        WEBNAMEDKEY_CHARACTER,            "M"        },   /* KeyM */
    { WEBKEYLOCATION_STANDARD, 49,  0x0031, 0x4E, WEBNAMEDKEY_CHARACTER,            "n",        WEBNAMEDKEY_CHARACTER,            "N"        },   /* KeyN */
    { WEBKEYLOCATION_STANDARD, 24,  0x0018, 0x4F, WEBNAMEDKEY_CHARACTER,            "o",        WEBNAMEDKEY_CHARACTER,            "O"        },   /* KeyO */
    { WEBKEYLOCATION_STANDARD, 25,  0x0019, 0x50, WEBNAMEDKEY_CHARACTER,            "p",        WEBNAMEDKEY_CHARACTER,            "P"        },   /* KeyP */
    { WEBKEYLOCATION_STANDARD, 16,  0x0010, 0x51, WEBNAMEDKEY_CHARACTER,            "q",        WEBNAMEDKEY_CHARACTER,            "Q"        },   /* KeyQ */
    { WEBKEYLOCATION_STANDARD, 19,  0x0013, 0x52, WEBNAMEDKEY_CHARACTER,            "r",        WEBNAMEDKEY_CHARACTER,            "R"        },   /* KeyR */
    { WEBKEYLOCATION_STANDARD, 31,  0x001F, 0x53, WEBNAMEDKEY_CHARACTER,            "s",        WEBNAMEDKEY_CHARACTER,            "S"        },   /* KeyS */
    { WEBKEYLOCATION_STANDARD, 20,  0x0014, 0x54, WEBNAMEDKEY_CHARACTER,            "t",        WEBNAMEDKEY_CHARACTER,            "T"        },   /* KeyT */
    { WEBKEYLOCATION_STANDARD, 22,  0x0016, 0x55, WEBNAMEDKEY_CHARACTER,            "u",        WEBNAMEDKEY_CHARACTER,            "U"        },   /* KeyU */
    { WEBKEYLOCATION_STANDARD, 47,  0x002F, 0x56, WEBNAMEDKEY_CHARACTER,            "v",        WEBNAMEDKEY_CHARACTER,            "V"        },   /* KeyV */
    { WEBKEYLOCATION_STANDARD, 17,  0x0011, 0x57, WEBNAMEDKEY_CHARACTER,            "w",        WEBNAMEDKEY_CHARACTER,            "W"        },   /* KeyW */
    { WEBKEYLOCATION_STANDARD, 45,  0x002D, 0x58, WEBNAMEDKEY_CHARACTER,            "x",        WEBNAMEDKEY_CHARACTER,            "X"        },   /* KeyX */
    { WEBKEYLOCATION_STANDARD, 21,  0x0015, 0x59, WEBNAMEDKEY_CHARACTER,            "y",        WEBNAMEDKEY_CHARACTER,            "Y"        },   /* KeyY */
    { WEBKEYLOCATION_STANDARD, 44,  0x002C, 0x5A, WEBNAMEDKEY_CHARACTER,            "z",        WEBNAMEDKEY_CHARACTER,            "Z"        },   /* KeyZ */
    { WEBKEYLOCATION_STANDARD, 12,  0x000C, 0xBD, WEBNAMEDKEY_CHARACTER,            "-",        WEBNAMEDKEY_CHARACTER,            "_"        },   /* Minus */
    { WEBKEYLOCATION_STANDARD, 52,  0x0034, 0xBE, WEBNAMEDKEY_CHARACTER,            ".",        WEBNAMEDKEY_CHARACTER,            ">"        },   /* Period */
    { WEBKEYLOCATION_STANDARD, 40,  0x0028, 0xDE, WEBNAMEDKEY_CHARACTER,            "'",        WEBNAMEDKEY_CHARACTER,            "\""       },   /* Quote */
    { WEBKEYLOCATION_STANDARD, 39,  0x0027, 0xBA, WEBNAMEDKEY_CHARACTER,            ";",        WEBNAMEDKEY_CHARACTER,            ":"        },   /* Semicolon */
    { WEBKEYLOCATION_STANDARD, 53,  0x0035, 0xBF, WEBNAMEDKEY_CHARACTER,            "/",        WEBNAMEDKEY_CHARACTER,            "?"        },   /* Slash */
    { WEBKEYLOCATION_LEFT,     56,  0x0038, 0xA4, WEBNAMEDKEY_ALT,                  GUCEF_NULL, WEBNAMEDKEY_ALT,                  GUCEF_NULL },   /* AltLeft */
    { WEBKEYLOCATION_RIGHT,    100, 0xE038, 0xA5, WEBNAMEDKEY_ALT,                  GUCEF_NULL, WEBNAMEDKEY_ALT,                  GUCEF_NULL },   /* AltRight */
    { WEBKEYLOCATION_STANDARD, 14,  0x000E, 0x08, WEBNAMEDKEY_BACKSPACE,            GUCEF_NULL, WEBNAMEDKEY_BACKSPACE,            GUCEF_NULL },   /* Backspace */
    { WEBKEYLOCATION_STANDARD, 58,  0x003A, 0x14, WEBNAMEDKEY_CAPS_LOCK,            GUCEF_NULL, WEBNAMEDKEY_CAPS_LOCK,            GUCEF_NULL },   /* CapsLock */
    { WEBKEYLOCATION_STANDARD, 127, 0xE05D, 0x5D, WEBNAMEDKEY_CONTEXT_MENU,         GUCEF_NULL, WEBNAMEDKEY_CONTEXT_MENU,         GUCEF_NULL },   /* ContextMenu */
    { WEBKEYLOCATION_LEFT,     29,  0x001D, 0xA2, WEBNAMEDKEY_CONTROL,              GUCEF_NULL, WEBNAMEDKEY_CONTROL,              GUCEF_NULL },   /* ControlLeft */
    { WEBKEYLOCATION_RIGHT,    97,  0xE01D, 0xA3, WEBNAMEDKEY_CONTROL,              GUCEF_NULL, WEBNAMEDKEY_CONTROL,              GUCEF_NULL },   /* ControlRight */
    { WEBKEYLOCATION_STANDARD, 28,  0x001C, 0x0D, WEBNAMEDKEY_ENTER,                GUCEF_NULL, WEBNAMEDKEY_ENTER,                GUCEF_NULL },   /* Enter */
    { WEBKEYLOCATION_LEFT,     125, 0xE05B, 0x5B, WEBNAMEDKEY_META,                 GUCEF_NULL, WEBNAMEDKEY_META,                 GUCEF_NULL },   /* MetaLeft */
    { WEBKEYLOCATION_RIGHT,    126, 0xE05C, 0x5C, WEBNAMEDKEY_META,                 GUCEF_NULL, WEBNAMEDKEY_META,                 GUCEF_NULL },   /* MetaRight */
    { WEBKEYLOCATION_LEFT,     42,  0x002A, 0xA0, WEBNAMEDKEY_SHIFT,                GUCEF_NULL, WEBNAMEDKEY_SHIFT,                GUCEF_NULL },   /* ShiftLeft */
    { WEBKEYLOCATION_RIGHT,    54,  0x0036, 0xA1, WEBNAMEDKEY_SHIFT,                GUCEF_NULL, WEBNAMEDKEY_SHIFT,                GUCEF_NULL },   /* ShiftRight */
    { WEBKEYLOCATION_STANDARD, 57,  0x0039, 0x20, WEBNAMEDKEY_CHARACTER,            " ",        WEBNAMEDKEY_CHARACTER,            " "        },   /* Space */
    { WEBKEYLOCATION_STANDARD, 15,  0x000F, 0x09, WEBNAMEDKEY_TAB,                  GUCEF_NULL, WEBNAMEDKEY_TAB,                  GUCEF_NULL },   /* Tab */
    { WEBKEYLOCATION_STANDARD, 92,  0x0079, 0x1C, WEBNAMEDKEY_CONVERT,              GUCEF_NULL, WEBNAMEDKEY_CONVERT,              GUCEF_NULL },   /* Convert */
    { WEBKEYLOCATION_STANDARD, 93,  0x0070, 0x15, WEBNAMEDKEY_KANA_MODE,            GUCEF_NULL, WEBNAMEDKEY_KANA_MODE,            GUCEF_NULL },   /* KanaMode */
    { WEBKEYLOCATION_STANDARD, 122, 0x0072, 0x15, WEBNAMEDKEY_HANGUL_MODE,          GUCEF_NULL, WEBNAMEDKEY_HANGUL_MODE,          GUCEF_NULL },   /* Lang1 */
    { WEBKEYLOCATION_STANDARD, 123, 0x0071, 0x19, WEBNAMEDKEY_HANJA_MODE,           GUCEF_NULL, WEBNAMEDKEY_HANJA_MODE,           GUCEF_NULL },   /* Lang2 */
    { WEBKEYLOCATION_STANDARD, 90,  0x0000, 0x00, WEBNAMEDKEY_KATAKANA,             GUCEF_NULL, WEBNAMEDKEY_KATAKANA,             GUCEF_NULL },   /* Lang3 */
    { WEBKEYLOCATION_STANDARD, 91,  0x0000, 0x00, WEBNAMEDKEY_HIRAGANA,             GUCEF_NULL, WEBNAMEDKEY_HIRAGANA,             GUCEF_NULL },   /* Lang4 */
    { WEBKEYLOCATION_STANDARD, 85,  0x0000, 0x00, WEBNAMEDKEY_ZENKAKU_HANKAKU,      GUCEF_NULL, WEBNAMEDKEY_ZENKAKU_HANKAKU,      GUCEF_NULL },   /* Lang5 */
    { WEBKEYLOCATION_STANDARD, 94,  0x007B, 0x1D, WEBNAMEDKEY_NON_CONVERT,          GUCEF_NULL, WEBNAMEDKEY_NON_CONVERT,          GUCEF_NULL },   /* NonConvert */
    { WEBKEYLOCATION_STANDARD, 111, 0xE053, 0x2E, WEBNAMEDKEY_DELETE,               GUCEF_NULL, WEBNAMEDKEY_DELETE,               GUCEF_NULL },   /* Delete */
    { WEBKEYLOCATION_STANDARD, 107, 0xE04F, 0x23, WEBNAMEDKEY_END,                  GUCEF_NULL, WEBNAMEDKEY_END,                  GUCEF_NULL },   /* End */
    { WEBKEYLOCATION_STANDARD, 138, 0xE03B, 0x2F, WEBNAMEDKEY_HELP,                 GUCEF_NULL, WEBNAMEDKEY_HELP,                 GUCEF_NULL },   /* Help */
    { WEBKEYLOCATION_STANDARD, 102, 0xE047, 0x24, WEBNAMEDKEY_HOME,                 GUCEF_NULL, WEBNAMEDKEY_HOME,                 GUCEF_NULL },   /* Home */
    { WEBKEYLOCATION_STANDARD, 110, 0xE052, 0x2D, WEBNAMEDKEY_INSERT,               GUCEF_NULL, WEBNAMEDKEY_INSERT,               GUCEF_NULL },   /* Insert */
    { WEBKEYLOCATION_STANDARD, 109, 0xE051, 0x22, WEBNAMEDKEY_PAGE_DOWN,            GUCEF_NULL, WEBNAMEDKEY_PAGE_DOWN,            GUCEF_NULL },   /* PageDown */
    { WEBKEYLOCATION_STANDARD, 104, 0xE049, 0x21, WEBNAMEDKEY_PAGE_UP,              GUCEF_NULL, WEBNAMEDKEY_PAGE_UP,              GUCEF_NULL },   /* PageUp */
    { WEBKEYLOCATION_STANDARD, 108, 0xE050, 0x28, WEBNAMEDKEY_ARROW_DOWN,           GUCEF_NULL, WEBNAMEDKEY_ARROW_DOWN,           GUCEF_NULL },   /* ArrowDown */
    { WEBKEYLOCATION_STANDARD, 105, 0xE04B, 0x25, WEBNAMEDKEY_ARROW_LEFT,           GUCEF_NULL, WEBNAMEDKEY_ARROW_LEFT,           GUCEF_NULL },   /* ArrowLeft */
    { WEBKEYLOCATION_STANDARD, 106, 0xE04D, 0x27, WEBNAMEDKEY_ARROW_RIGHT,          GUCEF_NULL, WEBNAMEDKEY_ARROW_RIGHT,          GUCEF_NULL },   /* ArrowRight */
    { WEBKEYLOCATION_STANDARD, 103, 0xE048, 0x26, WEBNAMEDKEY_ARROW_UP,             GUCEF_NULL, WEBNAMEDKEY_ARROW_UP,             GUCEF_NULL },   /* ArrowUp */
    { WEBKEYLOCATION_STANDARD, 69,  0xE045, 0x90, WEBNAMEDKEY_NUM_LOCK,             GUCEF_NULL, WEBNAMEDKEY_NUM_LOCK,             GUCEF_NULL },   /* NumLock */
    { WEBKEYLOCATION_NUMPAD,   82,  0x0052, 0x60, WEBNAMEDKEY_CHARACTER,            "0",        WEBNAMEDKEY_CHARACTER,            "0"        },   /* Numpad0 */
    { WEBKEYLOCATION_NUMPAD,   79,  0x004F, 0x61, WEBNAMEDKEY_CHARACTER,            "1",        WEBNAMEDKEY_CHARACTER,            "1"        },   /* Numpad1 */
    { WEBKEYLOCATION_NUMPAD,   80,  0x0050, 0x62, WEBNAMEDKEY_CHARACTER,            "2",        WEBNAMEDKEY_CHARACTER,            "2"        },   /* Numpad2 */
    { WEBKEYLOCATION_NUMPAD,   81,  0x0051, 0x63, WEBNAMEDKEY_CHARACTER,            "3",        WEBNAMEDKEY_CHARACTER,            "3"        },   /* Numpad3 */
    { WEBKEYLOCATION_NUMPAD,   75,  0x004B, 0x64, WEBNAMEDKEY_CHARACTER,            "4",        WEBNAMEDKEY_CHARACTER,            "4"        },   /* Numpad4 */
    { WEBKEYLOCATION_NUMPAD,   76,  0x004C, 0x65, WEBNAMEDKEY_CHARACTER,            "5",        WEBNAMEDKEY_CHARACTER,            "5"        },   /* Numpad5 */
    { WEBKEYLOCATION_NUMPAD,   77,  0x004D, 0x66, WEBNAMEDKEY_CHARACTER,            "6",        WEBNAMEDKEY_CHARACTER,            "6"        },   /* Numpad6 */
    { WEBKEYLOCATION_NUMPAD,   71,  0x0047, 0x67, WEBNAMEDKEY_CHARACTER,            "7",        WEBNAMEDKEY_CHARACTER,            "7"        },   /* Numpad7 */
    { WEBKEYLOCATION_NUMPAD,   72,  0x0048, 0x68, WEBNAMEDKEY_CHARACTER,            "8",        WEBNAMEDKEY_CHARACTER,            "8"        },   /* Numpad8 */
    { WEBKEYLOCATION_NUMPAD,   73,  0x0049, 0x69, WEBNAMEDKEY_CHARACTER,            "9",        WEBNAMEDKEY_CHARACTER,            "9"        },   /* Numpad9 */
    { WEBKEYLOCATION_NUMPAD,   78,  0x004E, 0x6B, WEBNAMEDKEY_CHARACTER,            "+",        WEBNAMEDKEY_CHARACTER,            "+"        },   /* NumpadAdd */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_BACKSPACE,            GUCEF_NULL, WEBNAMEDKEY_BACKSPACE,            GUCEF_NULL },   /* NumpadBackspace */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x0C, WEBNAMEDKEY_CLEAR,                GUCEF_NULL, WEBNAMEDKEY_CLEAR,                GUCEF_NULL },   /* NumpadClear */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_CLEAR,                GUCEF_NULL, WEBNAMEDKEY_CLEAR,                GUCEF_NULL },   /* NumpadClearEntry */
    { WEBKEYLOCATION_NUMPAD,   121, 0x007E, 0x6C, WEBNAMEDKEY_CHARACTER,            ",",        WEBNAMEDKEY_CHARACTER,            ","        },   /* NumpadComma */
    { WEBKEYLOCATION_NUMPAD,   83,  0x0053, 0x6E, WEBNAMEDKEY_CHARACTER,            ".",        WEBNAMEDKEY_CHARACTER,            "."        },   /* NumpadDecimal */
    { WEBKEYLOCATION_NUMPAD,   98,  0xE035, 0x6F, WEBNAMEDKEY_CHARACTER,            "/",        WEBNAMEDKEY_CHARACTER,            "/"        },   /* NumpadDivide */
    { WEBKEYLOCATION_NUMPAD,   96,  0xE01C, 0x0D, WEBNAMEDKEY_ENTER,                GUCEF_NULL, WEBNAMEDKEY_ENTER,                GUCEF_NULL },   /* NumpadEnter */
    { WEBKEYLOCATION_NUMPAD,   117, 0x0059, 0x92, WEBNAMEDKEY_CHARACTER,            "=",        WEBNAMEDKEY_CHARACTER,            "="        },   /* NumpadEqual */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_CHARACTER,            "#",        WEBNAMEDKEY_CHARACTER,            "#"        },   /* NumpadHash */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* NumpadMemoryAdd */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* NumpadMemoryClear */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* NumpadMemoryRecall */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* NumpadMemoryStore */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* NumpadMemorySubtract */
    { WEBKEYLOCATION_NUMPAD,   55,  0x0037, 0x6A, WEBNAMEDKEY_CHARACTER,            "*",        WEBNAMEDKEY_CHARACTER,            "*"        },   /* NumpadMultiply */
    { WEBKEYLOCATION_NUMPAD,   179, 0x0000, 0x00, WEBNAMEDKEY_CHARACTER,            "(",        WEBNAMEDKEY_CHARACTER,            "("        },   /* NumpadParenLeft */
    { WEBKEYLOCATION_NUMPAD,   180, 0x0000, 0x00, WEBNAMEDKEY_CHARACTER,            ")",        WEBNAMEDKEY_CHARACTER,            ")"        },   /* NumpadParenRight */
    { WEBKEYLOCATION_NUMPAD,   0,   0x0000, 0x00, WEBNAMEDKEY_CHARACTER,            "*",        WEBNAMEDKEY_CHARACTER,            "*"        },   /* NumpadStar */
    { WEBKEYLOCATION_NUMPAD,   74,  0x004A, 0x6D, WEBNAMEDKEY_CHARACTER,            "-",        WEBNAMEDKEY_CHARACTER,            "-"        },   /* NumpadSubtract */
    { WEBKEYLOCATION_STANDARD, 1,   0x0001, 0x1B, WEBNAMEDKEY_ESCAPE,               GUCEF_NULL, WEBNAMEDKEY_ESCAPE,               GUCEF_NULL },   /* Escape */
    { WEBKEYLOCATION_STANDARD, 59,  0x003B, 0x70, WEBNAMEDKEY_F1,                   GUCEF_NULL, WEBNAMEDKEY_F1,                   GUCEF_NULL },   /* F1 */
    { WEBKEYLOCATION_STANDARD, 60,  0x003C, 0x71, WEBNAMEDKEY_F2,                   GUCEF_NULL, WEBNAMEDKEY_F2,                   GUCEF_NULL },   /* F2 */
    { WEBKEYLOCATION_STANDARD, 61,  0x003D, 0x72, WEBNAMEDKEY_F3,                   GUCEF_NULL, WEBNAMEDKEY_F3,                   GUCEF_NULL },   /* F3 */
    { WEBKEYLOCATION_STANDARD, 62,  0x003E, 0x73, WEBNAMEDKEY_F4,                   GUCEF_NULL, WEBNAMEDKEY_F4,                   GUCEF_NULL },   /* F4 */
    { WEBKEYLOCATION_STANDARD, 63,  0x003F, 0x74, WEBNAMEDKEY_F5,                   GUCEF_NULL, WEBNAMEDKEY_F5,                   GUCEF_NULL },   /* F5 */
    { WEBKEYLOCATION_STANDARD, 64,  0x0040, 0x75, WEBNAMEDKEY_F6,                   GUCEF_NULL, WEBNAMEDKEY_F6,                   GUCEF_NULL },   /* F6 */
    { WEBKEYLOCATION_STANDARD, 65,  0x0041, 0x76, WEBNAMEDKEY_F7,                   GUCEF_NULL, WEBNAMEDKEY_F7,                   GUCEF_NULL },   /* F7 */
    { WEBKEYLOCATION_STANDARD, 66,  0x0042, 0x77, WEBNAMEDKEY_F8,                   GUCEF_NULL, WEBNAMEDKEY_F8,                   GUCEF_NULL },   /* F8 */
    { WEBKEYLOCATION_STANDARD, 67,  0x0043, 0x78, WEBNAMEDKEY_F9,                   GUCEF_NULL, WEBNAMEDKEY_F9,                   GUCEF_NULL },   /* F9 */
    { WEBKEYLOCATION_STANDARD, 68,  0x0044, 0x79, WEBNAMEDKEY_F10,                  GUCEF_NULL, WEBNAMEDKEY_F10,                  GUCEF_NULL },   /* F10 */
    { WEBKEYLOCATION_STANDARD, 87,  0x0057, 0x7A, WEBNAMEDKEY_F11,                  GUCEF_NULL, WEBNAMEDKEY_F11,                  GUCEF_NULL },   /* F11 */
    { WEBKEYLOCATION_STANDARD, 88,  0x0058, 0x7B, WEBNAMEDKEY_F12,                  GUCEF_NULL, WEBNAMEDKEY_F12,                  GUCEF_NULL },   /* F12 */
    { WEBKEYLOCATION_STANDARD, 183, 0x0064, 0x7C, WEBNAMEDKEY_F13,                  GUCEF_NULL, WEBNAMEDKEY_F13,                  GUCEF_NULL },   /* F13 */
    { WEBKEYLOCATION_STANDARD, 184, 0x0065, 0x7D, WEBNAMEDKEY_F14,                  GUCEF_NULL, WEBNAMEDKEY_F14,                  GUCEF_NULL },   /* F14 */
    { WEBKEYLOCATION_STANDARD, 185, 0x0066, 0x7E, WEBNAMEDKEY_F15,                  GUCEF_NULL, WEBNAMEDKEY_F15,                  GUCEF_NULL },   /* F15 */
    { WEBKEYLOCATION_STANDARD, 186, 0x0067, 0x7F, WEBNAMEDKEY_F16,                  GUCEF_NULL, WEBNAMEDKEY_F16,                  GUCEF_NULL },   /* F16 */
    { WEBKEYLOCATION_STANDARD, 187, 0x0068, 0x80, WEBNAMEDKEY_F17,                  GUCEF_NULL, WEBNAMEDKEY_F17,                  GUCEF_NULL },   /* F17 */
    { WEBKEYLOCATION_STANDARD, 188, 0x0069, 0x81, WEBNAMEDKEY_F18,                  GUCEF_NULL, WEBNAMEDKEY_F18,                  GUCEF_NULL },   /* F18 */
    { WEBKEYLOCATION_STANDARD, 189, 0x006A, 0x82, WEBNAMEDKEY_F19,                  GUCEF_NULL, WEBNAMEDKEY_F19,                  GUCEF_NULL },   /* F19 */
    { WEBKEYLOCATION_STANDARD, 190, 0x006B, 0x83, WEBNAMEDKEY_F20,                  GUCEF_NULL, WEBNAMEDKEY_F20,                  GUCEF_NULL },   /* F20 */
    { WEBKEYLOCATION_STANDARD, 191, 0x006C, 0x84, WEBNAMEDKEY_F21,                  GUCEF_NULL, WEBNAMEDKEY_F21,                  GUCEF_NULL },   /* F21 */
    { WEBKEYLOCATION_STANDARD, 192, 0x006D, 0x85, WEBNAMEDKEY_F22,                  GUCEF_NULL, WEBNAMEDKEY_F22,                  GUCEF_NULL },   /* F22 */
    { WEBKEYLOCATION_STANDARD, 193, 0x006E, 0x86, WEBNAMEDKEY_F23,                  GUCEF_NULL, WEBNAMEDKEY_F23,                  GUCEF_NULL },   /* F23 */
    { WEBKEYLOCATION_STANDARD, 194, 0x0076, 0x87, WEBNAMEDKEY_F24,                  GUCEF_NULL, WEBNAMEDKEY_F24,                  GUCEF_NULL },   /* F24 */
    { WEBKEYLOCATION_STANDARD, 464, 0x0000, 0x00, WEBNAMEDKEY_FN,                   GUCEF_NULL, WEBNAMEDKEY_FN,                   GUCEF_NULL },   /* Fn */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_FN_LOCK,              GUCEF_NULL, WEBNAMEDKEY_FN_LOCK,              GUCEF_NULL },   /* FnLock */
    { WEBKEYLOCATION_STANDARD, 99,  0xE037, 0x2C, WEBNAMEDKEY_PRINT_SCREEN,         GUCEF_NULL, WEBNAMEDKEY_PRINT_SCREEN,         GUCEF_NULL },   /* PrintScreen */
    { WEBKEYLOCATION_STANDARD, 70,  0x0046, 0x91, WEBNAMEDKEY_SCROLL_LOCK,          GUCEF_NULL, WEBNAMEDKEY_SCROLL_LOCK,          GUCEF_NULL },   /* ScrollLock */
    { WEBKEYLOCATION_STANDARD, 119, 0x0045, 0x13, WEBNAMEDKEY_PAUSE,                GUCEF_NULL, WEBNAMEDKEY_PAUSE,                GUCEF_NULL },   /* Pause */
    { WEBKEYLOCATION_STANDARD, 158, 0xE06A, 0xA6, WEBNAMEDKEY_BROWSER_BACK,         GUCEF_NULL, WEBNAMEDKEY_BROWSER_BACK,         GUCEF_NULL },   /* BrowserBack */
    { WEBKEYLOCATION_STANDARD, 156, 0xE066, 0xAB, WEBNAMEDKEY_BROWSER_FAVORITES,    GUCEF_NULL, WEBNAMEDKEY_BROWSER_FAVORITES,    GUCEF_NULL },   /* BrowserFavorites */
    { WEBKEYLOCATION_STANDARD, 159, 0xE069, 0xA7, WEBNAMEDKEY_BROWSER_FORWARD,      GUCEF_NULL, WEBNAMEDKEY_BROWSER_FORWARD,      GUCEF_NULL },   /* BrowserForward */
    { WEBKEYLOCATION_STANDARD, 172, 0xE032, 0xAC, WEBNAMEDKEY_BROWSER_HOME,         GUCEF_NULL, WEBNAMEDKEY_BROWSER_HOME,         GUCEF_NULL },   /* BrowserHome */
    { WEBKEYLOCATION_STANDARD, 173, 0xE067, 0xA8, WEBNAMEDKEY_BROWSER_REFRESH,      GUCEF_NULL, WEBNAMEDKEY_BROWSER_REFRESH,      GUCEF_NULL },   /* BrowserRefresh */
    { WEBKEYLOCATION_STANDARD, 217, 0xE065, 0xAA, WEBNAMEDKEY_BROWSER_SEARCH,       GUCEF_NULL, WEBNAMEDKEY_BROWSER_SEARCH,       GUCEF_NULL },   /* BrowserSearch */
    { WEBKEYLOCATION_STANDARD, 128, 0xE068, 0xA9, WEBNAMEDKEY_BROWSER_STOP,         GUCEF_NULL, WEBNAMEDKEY_BROWSER_STOP,         GUCEF_NULL },   /* BrowserStop */
    { WEBKEYLOCATION_STANDARD, 161, 0x0000, 0x00, WEBNAMEDKEY_EJECT,                GUCEF_NULL, WEBNAMEDKEY_EJECT,                GUCEF_NULL },   /* Eject */
    { WEBKEYLOCATION_STANDARD, 157, 0xE06B, 0xB6, WEBNAMEDKEY_LAUNCH_APPLICATION1,  GUCEF_NULL, WEBNAMEDKEY_LAUNCH_APPLICATION1,  GUCEF_NULL },   /* LaunchApp1 */
    { WEBKEYLOCATION_STANDARD, 140, 0xE021, 0xB7, WEBNAMEDKEY_LAUNCH_APPLICATION2,  GUCEF_NULL, WEBNAMEDKEY_LAUNCH_APPLICATION2,  GUCEF_NULL },   /* LaunchApp2 */
    { WEBKEYLOCATION_STANDARD, 155, 0xE06C, 0xB4, WEBNAMEDKEY_LAUNCH_MAIL,          GUCEF_NULL, WEBNAMEDKEY_LAUNCH_MAIL,          GUCEF_NULL },   /* LaunchMail */
    { WEBKEYLOCATION_STANDARD, 164, 0xE022, 0xB3, WEBNAMEDKEY_MEDIA_PLAY_PAUSE,     GUCEF_NULL, WEBNAMEDKEY_MEDIA_PLAY_PAUSE,     GUCEF_NULL },   /* MediaPlayPause */
    { WEBKEYLOCATION_STANDARD, 171, 0xE06D, 0xB5, WEBNAMEDKEY_LAUNCH_MEDIA_PLAYER,  GUCEF_NULL, WEBNAMEDKEY_LAUNCH_MEDIA_PLAYER,  GUCEF_NULL },   /* MediaSelect */
    { WEBKEYLOCATION_STANDARD, 166, 0xE024, 0xB2, WEBNAMEDKEY_MEDIA_STOP,           GUCEF_NULL, WEBNAMEDKEY_MEDIA_STOP,           GUCEF_NULL },   /* MediaStop */
    { WEBKEYLOCATION_STANDARD, 163, 0xE019, 0xB0, WEBNAMEDKEY_MEDIA_TRACK_NEXT,     GUCEF_NULL, WEBNAMEDKEY_MEDIA_TRACK_NEXT,     GUCEF_NULL },   /* MediaTrackNext */
    { WEBKEYLOCATION_STANDARD, 165, 0xE010, 0xB1, WEBNAMEDKEY_MEDIA_TRACK_PREVIOUS, GUCEF_NULL, WEBNAMEDKEY_MEDIA_TRACK_PREVIOUS, GUCEF_NULL },   /* MediaTrackPrevious */
    { WEBKEYLOCATION_STANDARD, 116, 0xE05E, 0x00, WEBNAMEDKEY_POWER,                GUCEF_NULL, WEBNAMEDKEY_POWER,                GUCEF_NULL },   /* Power */
    { WEBKEYLOCATION_STANDARD, 142, 0xE05F, 0x5F, WEBNAMEDKEY_STANDBY,              GUCEF_NULL, WEBNAMEDKEY_STANDBY,              GUCEF_NULL },   /* Sleep */
    { WEBKEYLOCATION_STANDARD, 114, 0xE02E, 0xAE, WEBNAMEDKEY_AUDIO_VOLUME_DOWN,    GUCEF_NULL, WEBNAMEDKEY_AUDIO_VOLUME_DOWN,    GUCEF_NULL },   /* AudioVolumeDown */
    { WEBKEYLOCATION_STANDARD, 113, 0xE020, 0xAD, WEBNAMEDKEY_AUDIO_VOLUME_MUTE,    GUCEF_NULL, WEBNAMEDKEY_AUDIO_VOLUME_MUTE,    GUCEF_NULL },   /* AudioVolumeMute */
    { WEBKEYLOCATION_STANDARD, 115, 0xE030, 0xAF, WEBNAMEDKEY_AUDIO_VOLUME_UP,      GUCEF_NULL, WEBNAMEDKEY_AUDIO_VOLUME_UP,      GUCEF_NULL },   /* AudioVolumeUp */
    { WEBKEYLOCATION_STANDARD, 143, 0xE063, 0x00, WEBNAMEDKEY_WAKE_UP,              GUCEF_NULL, WEBNAMEDKEY_WAKE_UP,              GUCEF_NULL },   /* WakeUp */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_HYPER,                GUCEF_NULL, WEBNAMEDKEY_HYPER,                GUCEF_NULL },   /* Hyper */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_SUPER,                GUCEF_NULL, WEBNAMEDKEY_SUPER,                GUCEF_NULL },   /* Super */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* Turbo */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_CANCEL,               GUCEF_NULL, WEBNAMEDKEY_CANCEL,               GUCEF_NULL },   /* Abort */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* Resume */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL, WEBNAMEDKEY_UNIDENTIFIED,         GUCEF_NULL },   /* Suspend */
    { WEBKEYLOCATION_STANDARD, 129, 0x0000, 0x00, WEBNAMEDKEY_AGAIN,                GUCEF_NULL, WEBNAMEDKEY_AGAIN,                GUCEF_NULL },   /* Again */
    { WEBKEYLOCATION_STANDARD, 133, 0xE018, 0x00, WEBNAMEDKEY_COPY,                 GUCEF_NULL, WEBNAMEDKEY_COPY,                 GUCEF_NULL },   /* Copy */
    { WEBKEYLOCATION_STANDARD, 137, 0xE017, 0x00, WEBNAMEDKEY_CUT,                  GUCEF_NULL, WEBNAMEDKEY_CUT,                  GUCEF_NULL },   /* Cut */
    { WEBKEYLOCATION_STANDARD, 136, 0x0000, 0x00, WEBNAMEDKEY_FIND,                 GUCEF_NULL, WEBNAMEDKEY_FIND,                 GUCEF_NULL },   /* Find */
    { WEBKEYLOCATION_STANDARD, 134, 0x0000, 0x00, WEBNAMEDKEY_OPEN,                 GUCEF_NULL, WEBNAMEDKEY_OPEN,                 GUCEF_NULL },   /* Open */
    { WEBKEYLOCATION_STANDARD, 135, 0xE00A, 0x00, WEBNAMEDKEY_PASTE,                GUCEF_NULL, WEBNAMEDKEY_PASTE,                GUCEF_NULL },   /* Paste */
    { WEBKEYLOCATION_STANDARD, 130, 0x0000, 0x00, WEBNAMEDKEY_PROPS,                GUCEF_NULL, WEBNAMEDKEY_PROPS,                GUCEF_NULL },   /* Props */
    { WEBKEYLOCATION_STANDARD, 353, 0x0000, 0x29, WEBNAMEDKEY_SELECT,               GUCEF_NULL, WEBNAMEDKEY_SELECT,               GUCEF_NULL },   /* Select */
    { WEBKEYLOCATION_STANDARD, 131, 0xE008, 0x00, WEBNAMEDKEY_UNDO,                 GUCEF_NULL, WEBNAMEDKEY_UNDO,                 GUCEF_NULL },   /* Undo */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_HIRAGANA,             GUCEF_NULL, WEBNAMEDKEY_HIRAGANA,             GUCEF_NULL },   /* Hiragana */
    { WEBKEYLOCATION_STANDARD, 0,   0x0000, 0x00, WEBNAMEDKEY_KATAKANA,             GUCEF_NULL, WEBNAMEDKEY_KATAKANA,             GUCEF_NULL },   /* Katakana */
};

/** The flags of MouseEvent.buttons, indexed by MouseEvent.button */
const UInt32 MouseButtonFlags[ WEBMOUSEBUTTON_COUNT ] =
{
    WEBMOUSEBUTTONFLAG_LEFT, WEBMOUSEBUTTONFLAG_MIDDLE, WEBMOUSEBUTTONFLAG_RIGHT, WEBMOUSEBUTTONFLAG_BACK, WEBMOUSEBUTTONFLAG_FORWARD
};

inline bool
IsValidCode( TWebKeyCode code )
{
    return (Int32) code >= 0 && (Int32) code < (Int32) WEBKEYCODE_COUNT;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
CWebViewKeyCodes::GetDomCodeName( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) ? DomCodeNames[ code ] : DomCodeNames[ WEBKEYCODE_UNIDENTIFIED ];
}

/*-------------------------------------------------------------------------*/

TWebKeyCode
CWebViewKeyCodes::GetKeyCodeFromDomCodeName( const char* domCodeName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == domCodeName )
        return WEBKEYCODE_UNIDENTIFIED;
    for ( Int32 i=0; i<(Int32) WEBKEYCODE_COUNT; ++i )
        if ( 0 == strcmp( DomCodeNames[ i ], domCodeName ) )
            return (TWebKeyCode) i;
    return WEBKEYCODE_UNIDENTIFIED;
}

/*-------------------------------------------------------------------------*/

const char*
CWebViewKeyCodes::GetDomKeyName( TWebNamedKey key )
{GUCEF_TRACE;

    return ( (Int32) key >= 0 && (Int32) key < (Int32) WEBNAMEDKEY_COUNT ) ? DomKeyNames[ key ] : DomKeyNames[ WEBNAMEDKEY_UNIDENTIFIED ];
}

/*-------------------------------------------------------------------------*/

TWebNamedKey
CWebViewKeyCodes::GetNamedKeyFromDomKeyName( const char* domKeyName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == domKeyName || '\0' == *domKeyName )
        return WEBNAMEDKEY_UNIDENTIFIED;
    for ( Int32 i=0; i<(Int32) WEBNAMEDKEY_COUNT; ++i )
        if ( 0 == strcmp( DomKeyNames[ i ], domKeyName ) )
            return (TWebNamedKey) i;
    return WEBNAMEDKEY_UNIDENTIFIED;
}

/*-------------------------------------------------------------------------*/

TWebKeyLocation
CWebViewKeyCodes::GetLocation( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) ? KeyCodeTable[ code ].location : WEBKEYLOCATION_STANDARD;
}

/*-------------------------------------------------------------------------*/

UInt16
CWebViewKeyCodes::GetEvdevCode( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) ? KeyCodeTable[ code ].evdevCode : 0;
}

/*-------------------------------------------------------------------------*/

UInt16
CWebViewKeyCodes::GetX11HardwareKeycode( TWebKeyCode code )
{GUCEF_TRACE;

    const UInt16 evdevCode = GetEvdevCode( code );
    return 0 != evdevCode ? (UInt16) ( evdevCode + 8 ) : 0;
}

/*-------------------------------------------------------------------------*/

UInt16
CWebViewKeyCodes::GetWindowsScanCode( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) ? KeyCodeTable[ code ].windowsScanCode : 0;
}

/*-------------------------------------------------------------------------*/

UInt16
CWebViewKeyCodes::GetWindowsVirtualKey( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) ? KeyCodeTable[ code ].windowsVirtualKey : 0;
}

/*-------------------------------------------------------------------------*/

TWebNamedKey
CWebViewKeyCodes::GetUsLayoutKey( TWebKeyCode code, UInt32 modifiers, const char*& text )
{GUCEF_TRACE;

    if ( !IsValidCode( code ) )
    {
        text = GUCEF_NULL;
        return WEBNAMEDKEY_UNIDENTIFIED;
    }

    // caps lock shifts the letters only, and shift with caps lock gives them in lower case again
    bool shifted = 0 != ( modifiers & WEBMODIFIER_SHIFT );
    if ( code >= WEBKEYCODE_KEY_A && code <= WEBKEYCODE_KEY_Z && 0 != ( modifiers & WEBMODIFIER_CAPSLOCK ) )
        shifted = !shifted;

    const SKeyCodeInfo& info = KeyCodeTable[ code ];
    text = shifted ? info.usTextShifted : info.usText;
    return shifted ? info.usKeyShifted : info.usKey;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewKeyCodes::IsPrintable( TWebKeyCode code )
{GUCEF_TRACE;

    return IsValidCode( code ) && WEBNAMEDKEY_CHARACTER == KeyCodeTable[ code ].usKey;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewKeyCodes::IsModifierKey( TWebKeyCode code )
{GUCEF_TRACE;

    switch ( code )
    {
        case WEBKEYCODE_SHIFT_LEFT:
        case WEBKEYCODE_SHIFT_RIGHT:
        case WEBKEYCODE_CONTROL_LEFT:
        case WEBKEYCODE_CONTROL_RIGHT:
        case WEBKEYCODE_ALT_LEFT:
        case WEBKEYCODE_ALT_RIGHT:
        case WEBKEYCODE_META_LEFT:
        case WEBKEYCODE_META_RIGHT:
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewKeyCodes::GetMouseButtonFlag( TWebMouseButton button )
{GUCEF_TRACE;

    return ( (Int32) button >= 0 && (Int32) button < (Int32) WEBMOUSEBUTTON_COUNT ) ? MouseButtonFlags[ button ] : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
