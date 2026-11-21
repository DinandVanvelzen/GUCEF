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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <string.h>

#include "gucefGUI_CKeyboardShortcutMap.h"

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

const CORE::CEvent CKeyboardShortcutMap::ShortcutActivatedEvent = "GUCEF::GUI::CKeyboardShortcutMap::ShortcutActivatedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SKeyName
{
    INPUT::KeyCode key;
    const char* name;
};

/** A chord read from a settings node */
struct SRead
{
    CString actionId;
    INPUT::KeyCode key;
    UInt32 modifiers;
};

const SKeyName KeyNames[] =
{
    { INPUT::KEYCODE_A, "A" }, { INPUT::KEYCODE_B, "B" }, { INPUT::KEYCODE_C, "C" }, { INPUT::KEYCODE_D, "D" },
    { INPUT::KEYCODE_E, "E" }, { INPUT::KEYCODE_F, "F" }, { INPUT::KEYCODE_G, "G" }, { INPUT::KEYCODE_H, "H" },
    { INPUT::KEYCODE_I, "I" }, { INPUT::KEYCODE_J, "J" }, { INPUT::KEYCODE_K, "K" }, { INPUT::KEYCODE_L, "L" },
    { INPUT::KEYCODE_M, "M" }, { INPUT::KEYCODE_N, "N" }, { INPUT::KEYCODE_O, "O" }, { INPUT::KEYCODE_P, "P" },
    { INPUT::KEYCODE_Q, "Q" }, { INPUT::KEYCODE_R, "R" }, { INPUT::KEYCODE_S, "S" }, { INPUT::KEYCODE_T, "T" },
    { INPUT::KEYCODE_U, "U" }, { INPUT::KEYCODE_V, "V" }, { INPUT::KEYCODE_W, "W" }, { INPUT::KEYCODE_X, "X" },
    { INPUT::KEYCODE_Y, "Y" }, { INPUT::KEYCODE_Z, "Z" },
    { INPUT::KEYCODE_1, "1" }, { INPUT::KEYCODE_2, "2" }, { INPUT::KEYCODE_3, "3" }, { INPUT::KEYCODE_4, "4" },
    { INPUT::KEYCODE_5, "5" }, { INPUT::KEYCODE_6, "6" }, { INPUT::KEYCODE_7, "7" }, { INPUT::KEYCODE_8, "8" },
    { INPUT::KEYCODE_9, "9" }, { INPUT::KEYCODE_0, "0" },
    { INPUT::KEYCODE_F1, "F1" }, { INPUT::KEYCODE_F2, "F2" }, { INPUT::KEYCODE_F3, "F3" }, { INPUT::KEYCODE_F4, "F4" },
    { INPUT::KEYCODE_F5, "F5" }, { INPUT::KEYCODE_F6, "F6" }, { INPUT::KEYCODE_F7, "F7" }, { INPUT::KEYCODE_F8, "F8" },
    { INPUT::KEYCODE_F9, "F9" }, { INPUT::KEYCODE_F10, "F10" }, { INPUT::KEYCODE_F11, "F11" }, { INPUT::KEYCODE_F12, "F12" },
    { INPUT::KEYCODE_LEFT, "Left" }, { INPUT::KEYCODE_RIGHT, "Right" }, { INPUT::KEYCODE_UP, "Up" }, { INPUT::KEYCODE_DOWN, "Down" },
    { INPUT::KEYCODE_HOME, "Home" }, { INPUT::KEYCODE_END, "End" }, { INPUT::KEYCODE_PGUP, "PageUp" }, { INPUT::KEYCODE_PGDOWN, "PageDown" },
    { INPUT::KEYCODE_INSERT, "Insert" }, { INPUT::KEYCODE_DELETE, "Delete" }, { INPUT::KEYCODE_BACK, "Backspace" },
    { INPUT::KEYCODE_TAB, "Tab" }, { INPUT::KEYCODE_RETURN, "Enter" }, { INPUT::KEYCODE_NUMPADENTER, "Enter" },
    { INPUT::KEYCODE_ESCAPE, "Escape" }, { INPUT::KEYCODE_SPACE, "Space" }, { INPUT::KEYCODE_APPS, "Menu" },
    { INPUT::KEYCODE_MINUS, "-" }, { INPUT::KEYCODE_EQUALS, "=" }, { INPUT::KEYCODE_COMMA, "," }, { INPUT::KEYCODE_PERIOD, "." },
    { INPUT::KEYCODE_SLASH, "/" }, { INPUT::KEYCODE_ADD, "Num+" }, { INPUT::KEYCODE_SUBTRACT, "Num-" },
    { INPUT::KEYCODE_GRAVE, "Grave" }, { INPUT::KEYCODE_BACKSLASH, "\\" }, { INPUT::KEYCODE_LBRACKET, "[" }, { INPUT::KEYCODE_RBRACKET, "]" },
    { INPUT::KEYCODE_SEMICOLON, ";" }, { INPUT::KEYCODE_APOSTROPHE, "'" },
    { INPUT::KEYCODE_WEBBACK, "Back" }, { INPUT::KEYCODE_WEBFORWARD, "Forward" }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CKeyboardShortcutMap::RegisterEvents( void )
{GUCEF_TRACE;

    ShortcutActivatedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CKeyboardShortcutMap::CKeyboardShortcutMap( void )
    : CORE::CNotifier()
    , m_bindings()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CKeyboardShortcutMap::~CKeyboardShortcutMap()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardShortcutMap::NormalizeModifiers( UInt32 modifiers )
{GUCEF_TRACE;

    return modifiers & ( (UInt32) INPUT::KEYMOD_SHIFT | (UInt32) INPUT::KEYMOD_CTRL | (UInt32) INPUT::KEYMOD_ALT | (UInt32) INPUT::KEYMOD_META );
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::Bind( const CString& actionId, INPUT::KeyCode key, UInt32 modifiers )
{GUCEF_TRACE;

    if ( actionId.IsNULLOrEmpty() || INPUT::KEYCODE_UNASSIGNED == key )
        return false;
    const UInt32 normalized = NormalizeModifiers( modifiers );
    TBindingVector kept;
    for ( UInt32 i=0; i<(UInt32)m_bindings.size(); ++i )
    {
        const TBinding& binding = m_bindings[ i ];
        if ( binding.actionId == actionId || ( binding.key == key && binding.modifiers == normalized ) )
            continue;
        kept.push_back( binding );
    }
    TBinding binding;
    binding.actionId = actionId;
    binding.key = key;
    binding.modifiers = normalized;
    kept.push_back( binding );
    m_bindings = kept;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::Unbind( const CString& actionId )
{GUCEF_TRACE;

    for ( TBindingVector::iterator i=m_bindings.begin(); i!=m_bindings.end(); ++i )
    {
        if ( (*i).actionId == actionId )
        {
            m_bindings.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CKeyboardShortcutMap::Clear( void )
{GUCEF_TRACE;

    m_bindings.clear();
}

/*-------------------------------------------------------------------------*/

CString
CKeyboardShortcutMap::FindAction( INPUT::KeyCode key, UInt32 modifiers ) const
{GUCEF_TRACE;

    const UInt32 normalized = NormalizeModifiers( modifiers );
    for ( UInt32 i=0; i<(UInt32)m_bindings.size(); ++i )
        if ( m_bindings[ i ].key == key && m_bindings[ i ].modifiers == normalized )
            return m_bindings[ i ].actionId;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::GetChord( const CString& actionId, INPUT::KeyCode& key, UInt32& modifiers ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_bindings.size(); ++i )
    {
        if ( m_bindings[ i ].actionId == actionId )
        {
            key = m_bindings[ i ].key;
            modifiers = m_bindings[ i ].modifiers;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardShortcutMap::GetBindingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_bindings.size();
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::HandleKey( INPUT::KeyCode key, UInt32 modifiers )
{GUCEF_TRACE;

    const CString action = FindAction( key, modifiers );
    if ( action.IsNULLOrEmpty() )
        return false;
    CORE::TCloneableString data( action );
    NotifyObservers( ShortcutActivatedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::HandleKeyEvent( const CKeyEventData& key )
{GUCEF_TRACE;

    return HandleKey( key.keyCode, key.modifiers );
}

/*-------------------------------------------------------------------------*/

CString
CKeyboardShortcutMap::GetShortcutText( const CString& actionId ) const
{GUCEF_TRACE;

    INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
    UInt32 modifiers = 0;
    if ( !GetChord( actionId, key, modifiers ) )
        return CString::Empty;
    return DescribeChord( key, modifiers );
}

/*-------------------------------------------------------------------------*/

CString
CKeyboardShortcutMap::DescribeKey( INPUT::KeyCode key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)( sizeof( KeyNames ) / sizeof( KeyNames[ 0 ] ) ); ++i )
        if ( KeyNames[ i ].key == key )
            return KeyNames[ i ].name;
    return "Key" + CORE::UInt32ToString( (UInt32) key );
}

/*-------------------------------------------------------------------------*/

CString
CKeyboardShortcutMap::DescribeChord( INPUT::KeyCode key, UInt32 modifiers )
{GUCEF_TRACE;

    CString text;
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) )
        text += "Ctrl+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_ALT ) )
        text += "Alt+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) )
        text += "Shift+";
    if ( 0 != ( modifiers & INPUT::KEYMOD_META ) )
        text += "Meta+";
    text += DescribeKey( key );
    return text;
}

/*-------------------------------------------------------------------------*/

void
CKeyboardShortcutMap::GetActionIds( GUCEF::vector< CString >& actionIds ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_bindings.size(); ++i )
        actionIds.push_back( m_bindings[ i ].actionId );
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::ParseChord( const CString& text, INPUT::KeyCode& key, UInt32& modifiers )
{GUCEF_TRACE;

    key = INPUT::KEYCODE_UNASSIGNED;
    modifiers = 0;

    // the modifiers come first, each followed by a plus; what remains is the key (which may be a plus itself)
    const char* begin = text.C_String();
    UInt32 length = (UInt32) strlen( begin );
    while ( length > 0 && ' ' == begin[ 0 ] ) { ++begin; --length; }
    while ( length > 0 && ' ' == begin[ length - 1 ] ) { --length; }
    for ( ;; )
    {
        UInt32 plus = 1;    // a plus at the first place is the key, not a separator
        while ( plus < length && '+' != begin[ plus ] )
            ++plus;
        if ( plus + 1 >= length )
            break;
        UInt32 flag = 0;
        if ( 4 == plus && 0 == strncmp( begin, "Ctrl", 4 ) )
            flag = INPUT::KEYMOD_CTRL;
        else
        if ( 3 == plus && 0 == strncmp( begin, "Alt", 3 ) )
            flag = INPUT::KEYMOD_ALT;
        else
        if ( 5 == plus && 0 == strncmp( begin, "Shift", 5 ) )
            flag = INPUT::KEYMOD_SHIFT;
        else
        if ( 4 == plus && 0 == strncmp( begin, "Meta", 4 ) )
            flag = INPUT::KEYMOD_META;
        else
            return false;
        modifiers |= flag;
        begin += plus + 1;
        length -= plus + 1;
    }
    for ( UInt32 i=0; i<(UInt32)( sizeof( KeyNames ) / sizeof( KeyNames[ 0 ] ) ); ++i )
    {
        if ( strlen( KeyNames[ i ].name ) == length && 0 == strncmp( begin, KeyNames[ i ].name, length ) )
        {
            key = KeyNames[ i ].key;
            return true;
        }
    }
    modifiers = 0;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardShortcutMap::SaveBindings( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_bindings.size(); ++i )
        node.SetAttribute( m_bindings[ i ].actionId, DescribeChord( m_bindings[ i ].key, m_bindings[ i ].modifiers ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardShortcutMap::LoadBindings( const CORE::CDataNode& node )
{GUCEF_TRACE;

    // what is read first, bound after: the actions of the file may take each other's chords
    GUCEF::vector< SRead > reads;
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
    {
        SRead read;
        read.actionId = (*a).first;
        read.key = INPUT::KEYCODE_UNASSIGNED;
        read.modifiers = 0;
        if ( ParseChord( (*a).second.AsString( CString::Empty, true ), read.key, read.modifiers ) )
            reads.push_back( read );
    }
    for ( UInt32 i=0; i<(UInt32)reads.size(); ++i )
        Unbind( reads[ i ].actionId );
    UInt32 bound = 0;
    for ( UInt32 i=0; i<(UInt32)reads.size(); ++i )
        if ( Bind( reads[ i ].actionId, reads[ i ].key, reads[ i ].modifiers ) )
            ++bound;
    return bound;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
