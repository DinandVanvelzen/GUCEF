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


#ifndef GUCEF_INPUT_WAYLAND_XKB_H
#define GUCEF_INPUT_WAYLAND_XKB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The functions of libxkbcommon.so.0 that the driver uses, loaded when asked for so the driver builds without the
 *  development package of the library. The handles are opaque pointers. The values of the constants are those of xkbcommon.h.
 */
struct SXkbApi
{
    void* ( *context_new )( int flags );
    void ( *context_unref )( void* context );
    void* ( *keymap_new_from_buffer )( void* context, const char* buffer, size_t length, int format, int flags );
    void ( *keymap_unref )( void* keymap );
    void* ( *state_new )( void* keymap );
    void ( *state_unref )( void* state );
    int ( *state_update_mask )( void* state, UInt32 depressedMods, UInt32 latchedMods, UInt32 lockedMods, UInt32 depressedLayout, UInt32 latchedLayout, UInt32 lockedLayout );
    UInt32 ( *state_key_get_one_sym )( void* state, UInt32 key );
    int ( *state_key_get_utf8 )( void* state, UInt32 key, char* buffer, size_t size );
    UInt32 ( *state_key_get_layout )( void* state, UInt32 key );
    int ( *state_mod_name_is_active )( void* state, const char* name, int type );
    UInt32 ( *keymap_num_layouts_for_key )( void* keymap, UInt32 key );
    int ( *keymap_key_get_syms_by_level )( void* keymap, UInt32 key, UInt32 layout, UInt32 level, const UInt32** syms );
    int ( *keymap_key_repeats )( void* keymap, UInt32 key );

    // the dead keys and compose sequences: optional
    void* ( *compose_table_new_from_locale )( void* context, const char* locale, int flags );
    void ( *compose_table_unref )( void* table );
    void* ( *compose_state_new )( void* table, int flags );
    void ( *compose_state_unref )( void* state );
    int ( *compose_state_feed )( void* state, UInt32 keysym );
    int ( *compose_state_get_status )( void* state );
    int ( *compose_state_get_utf8 )( void* state, char* buffer, size_t size );
    void ( *compose_state_reset )( void* state );
};

/** Loads libxkbcommon.so.0; false when it is not installed or lacks the functions every keyboard needs */
GUCEF_INPUT_PLUGIN_PUBLIC_CPP bool
LoadXkbApi( SXkbApi& api );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The keymap the compositor sent, its state and the compose state, over the functions of xkbcommon. Keys are the evdev
 *  codes of wl_keyboard.key (the code of xkbcommon is that plus 8, which is done here).
 */
class GUCEF_INPUT_PLUGIN_PUBLIC_CPP CXkbKeyboard
{
    public:

    explicit CXkbKeyboard( const SXkbApi& api );

    ~CXkbKeyboard();

    /** Takes the keymap text of wl_keyboard.keymap (XKB text format 1), replacing the one before; false when it cannot be read */
    bool LoadKeymap( const char* text, UInt32 size );

    bool HasKeymap( void ) const;

    void UpdateMask( UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group );

    /**
     *  The keysyms that name the key without modifiers, the layout in use first and then the others: what a key is called
     *  independent of the shift state and, on a layout that has no Latin letters, of the letters it has. Returns the number found.
     */
    UInt32 GetBaseKeysyms( UInt32 key, UInt32* keysyms, UInt32 maxCount ) const;

    /**
     *  The text the key types now (after the layout, the modifiers and the compose sequences in progress) as UTF-8 in 'buffer'.
     *  False when it types nothing: modifier and cursor keys, the middle of a compose sequence, control characters
     */
    bool GetText( UInt32 key, char* buffer, UInt32 size );

    bool KeyRepeats( UInt32 key ) const;

    /** The KEYMOD_ bits of gucefINPUT for the modifiers that are active */
    UInt32 GetModifierMask( void ) const;

    void ResetCompose( void );

    private:

    CXkbKeyboard( const CXkbKeyboard& src );
    CXkbKeyboard& operator=( const CXkbKeyboard& src );

    void ReleaseKeymap( void );

    private:

    SXkbApi m_api;
    void* m_context;
    void* m_keymap;
    void* m_state;
    void* m_composeTable;
    void* m_composeState;
    bool m_composeTried;
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

#endif /* GUCEF_INPUT_WAYLAND_XKB_H ? */
