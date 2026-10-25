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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#include "inputdriverWAYLAND_Xkb.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// the values of xkbcommon.h
static const int XkbKeymapFormatTextV1 = 1;
static const int XkbStateModsEffective = 1 << 3;
static const int XkbComposeStatusNothing = 0;
static const int XkbComposeStatusComposing = 1;
static const int XkbComposeStatusComposed = 2;
static const int XkbComposeStatusCancelled = 3;
static const UInt32 EvdevToXkb = 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

template< typename T >
void
Fill( T& slot, void* module, const char* name )
{
    slot = (T) CORE::GetFunctionAddress( module, name, 0 ).objPtr;
}

}

/*-------------------------------------------------------------------------*/

bool
LoadXkbApi( SXkbApi& api )
{GUCEF_TRACE;

    memset( &api, 0, sizeof( api ) );
    void* lib = CORE::LoadModuleDynamicly( "libxkbcommon.so.0" );
    if ( GUCEF_NULL == lib )
        return false;

    Fill( api.context_new, lib, "xkb_context_new" );
    Fill( api.context_unref, lib, "xkb_context_unref" );
    Fill( api.keymap_new_from_buffer, lib, "xkb_keymap_new_from_buffer" );
    Fill( api.keymap_unref, lib, "xkb_keymap_unref" );
    Fill( api.state_new, lib, "xkb_state_new" );
    Fill( api.state_unref, lib, "xkb_state_unref" );
    Fill( api.state_update_mask, lib, "xkb_state_update_mask" );
    Fill( api.state_key_get_one_sym, lib, "xkb_state_key_get_one_sym" );
    Fill( api.state_key_get_utf8, lib, "xkb_state_key_get_utf8" );
    Fill( api.state_key_get_layout, lib, "xkb_state_key_get_layout" );
    Fill( api.state_mod_name_is_active, lib, "xkb_state_mod_name_is_active" );
    Fill( api.keymap_num_layouts_for_key, lib, "xkb_keymap_num_layouts_for_key" );
    Fill( api.keymap_key_get_syms_by_level, lib, "xkb_keymap_key_get_syms_by_level" );
    Fill( api.keymap_key_repeats, lib, "xkb_keymap_key_repeats" );
    Fill( api.compose_table_new_from_locale, lib, "xkb_compose_table_new_from_locale" );
    Fill( api.compose_table_unref, lib, "xkb_compose_table_unref" );
    Fill( api.compose_state_new, lib, "xkb_compose_state_new" );
    Fill( api.compose_state_unref, lib, "xkb_compose_state_unref" );
    Fill( api.compose_state_feed, lib, "xkb_compose_state_feed" );
    Fill( api.compose_state_get_status, lib, "xkb_compose_state_get_status" );
    Fill( api.compose_state_get_utf8, lib, "xkb_compose_state_get_utf8" );
    Fill( api.compose_state_reset, lib, "xkb_compose_state_reset" );

    return GUCEF_NULL != api.context_new && GUCEF_NULL != api.context_unref && GUCEF_NULL != api.keymap_new_from_buffer && GUCEF_NULL != api.keymap_unref &&
           GUCEF_NULL != api.state_new && GUCEF_NULL != api.state_unref && GUCEF_NULL != api.state_update_mask && GUCEF_NULL != api.state_key_get_one_sym &&
           GUCEF_NULL != api.state_key_get_utf8 && GUCEF_NULL != api.state_key_get_layout && GUCEF_NULL != api.state_mod_name_is_active &&
           GUCEF_NULL != api.keymap_num_layouts_for_key && GUCEF_NULL != api.keymap_key_get_syms_by_level && GUCEF_NULL != api.keymap_key_repeats;
}

/*-------------------------------------------------------------------------*/

CXkbKeyboard::CXkbKeyboard( const SXkbApi& api )
    : m_api( api )
    , m_context( GUCEF_NULL )
    , m_keymap( GUCEF_NULL )
    , m_state( GUCEF_NULL )
    , m_composeTable( GUCEF_NULL )
    , m_composeState( GUCEF_NULL )
    , m_composeTried( false )
{GUCEF_TRACE;

    m_context = m_api.context_new( 0 );
}

/*-------------------------------------------------------------------------*/

CXkbKeyboard::~CXkbKeyboard()
{GUCEF_TRACE;

    ReleaseKeymap();
    if ( GUCEF_NULL != m_composeState ) m_api.compose_state_unref( m_composeState );
    if ( GUCEF_NULL != m_composeTable ) m_api.compose_table_unref( m_composeTable );
    if ( GUCEF_NULL != m_context ) m_api.context_unref( m_context );
}

/*-------------------------------------------------------------------------*/

void
CXkbKeyboard::ReleaseKeymap( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_state ) m_api.state_unref( m_state );
    if ( GUCEF_NULL != m_keymap ) m_api.keymap_unref( m_keymap );
    m_state = GUCEF_NULL;
    m_keymap = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CXkbKeyboard::LoadKeymap( const char* text, UInt32 size )
{GUCEF_TRACE;

    ReleaseKeymap();
    if ( GUCEF_NULL == m_context || GUCEF_NULL == text || 0 == size )
        return false;

    // the size of the keymap of the compositor includes the terminating zero
    UInt32 length = 0;
    while ( length < size && '\0' != text[ length ] )
        ++length;

    m_keymap = m_api.keymap_new_from_buffer( m_context, text, length, XkbKeymapFormatTextV1, 0 );
    if ( GUCEF_NULL == m_keymap )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CXkbKeyboard: The keymap of the compositor could not be compiled" );
        return false;
    }
    m_state = m_api.state_new( m_keymap );
    if ( GUCEF_NULL == m_state )
    {
        ReleaseKeymap();
        return false;
    }
    ResetCompose();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXkbKeyboard::HasKeymap( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_state;
}

/*-------------------------------------------------------------------------*/

void
CXkbKeyboard::UpdateMask( UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_state )
        m_api.state_update_mask( m_state, depressed, latched, locked, 0, 0, group );
}

/*-------------------------------------------------------------------------*/

UInt32
CXkbKeyboard::GetBaseKeysyms( UInt32 key, UInt32* keysyms, UInt32 maxCount ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_state || GUCEF_NULL == keysyms || 0 == maxCount )
        return 0;
    const UInt32 xkbKey = key + EvdevToXkb;
    const UInt32 layouts = m_api.keymap_num_layouts_for_key( m_keymap, xkbKey );
    if ( 0 == layouts )
        return 0;

    UInt32 found = 0;
    const UInt32 active = m_api.state_key_get_layout( m_state, xkbKey );
    for ( UInt32 i=0; i<layouts && found<maxCount; ++i )
    {
        // the active layout first, then the others in their order
        const UInt32 layout = 0 == i ? active : ( i - 1 < active ? i - 1 : i );
        const UInt32* syms = GUCEF_NULL;
        const int count = m_api.keymap_key_get_syms_by_level( m_keymap, xkbKey, layout, 0, &syms );
        if ( count > 0 && GUCEF_NULL != syms )
            keysyms[ found++ ] = syms[ 0 ];
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CXkbKeyboard::GetText( UInt32 key, char* buffer, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_state || GUCEF_NULL == buffer || size < 2 )
        return false;
    const UInt32 xkbKey = key + EvdevToXkb;

    // dead keys and compose sequences: the middle of a sequence types nothing, the end types the result
    if ( !m_composeTried )
    {
        m_composeTried = true;
        if ( GUCEF_NULL != m_api.compose_table_new_from_locale && GUCEF_NULL != m_api.compose_state_new && GUCEF_NULL != m_api.compose_state_feed &&
             GUCEF_NULL != m_api.compose_state_get_status && GUCEF_NULL != m_api.compose_state_get_utf8 && GUCEF_NULL != m_api.compose_state_reset )
        {
            const char* locale = ::getenv( "LC_ALL" );
            if ( GUCEF_NULL == locale || '\0' == locale[ 0 ] ) locale = ::getenv( "LC_CTYPE" );
            if ( GUCEF_NULL == locale || '\0' == locale[ 0 ] ) locale = ::getenv( "LANG" );
            if ( GUCEF_NULL == locale || '\0' == locale[ 0 ] ) locale = "C";
            m_composeTable = m_api.compose_table_new_from_locale( m_context, locale, 0 );
            if ( GUCEF_NULL != m_composeTable )
                m_composeState = m_api.compose_state_new( m_composeTable, 0 );
        }
    }
    if ( GUCEF_NULL != m_composeState )
    {
        const UInt32 sym = m_api.state_key_get_one_sym( m_state, xkbKey );
        if ( 0 != m_api.compose_state_feed( m_composeState, sym ) )
        {
            const int status = m_api.compose_state_get_status( m_composeState );
            if ( XkbComposeStatusComposing == status )
                return false;
            if ( XkbComposeStatusCancelled == status )
            {
                m_api.compose_state_reset( m_composeState );
                return false;
            }
            if ( XkbComposeStatusComposed == status )
            {
                const int length = m_api.compose_state_get_utf8( m_composeState, buffer, size );
                m_api.compose_state_reset( m_composeState );
                if ( length > 0 && (UInt32) length < size )
                    return true;
                return false;
            }
        }
    }

    const int length = m_api.state_key_get_utf8( m_state, xkbKey, buffer, size );
    if ( length <= 0 || (UInt32) length >= size )
        return false;

    // control characters (Return, Backspace, Escape, Tab, Delete) are keys, not text
    if ( 1 == length && ( (unsigned char) buffer[ 0 ] < 32 || 127 == (unsigned char) buffer[ 0 ] ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXkbKeyboard::KeyRepeats( UInt32 key ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_keymap && 0 != m_api.keymap_key_repeats( m_keymap, key + EvdevToXkb );
}

/*-------------------------------------------------------------------------*/

UInt32
CXkbKeyboard::GetModifierMask( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_state )
        return 0;
    UInt32 mask = 0;
    if ( m_api.state_mod_name_is_active( m_state, "Shift", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_SHIFT;
    if ( m_api.state_mod_name_is_active( m_state, "Control", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_CTRL;
    if ( m_api.state_mod_name_is_active( m_state, "Mod1", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_ALT;
    if ( m_api.state_mod_name_is_active( m_state, "Mod4", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_META;
    if ( m_api.state_mod_name_is_active( m_state, "Lock", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_CAPSLOCK;
    if ( m_api.state_mod_name_is_active( m_state, "Mod2", XkbStateModsEffective ) > 0 ) mask |= KEYMOD_NUMLOCK;
    return mask;
}

/*-------------------------------------------------------------------------*/

void
CXkbKeyboard::ResetCompose( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_composeState )
        m_api.compose_state_reset( m_composeState );
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
