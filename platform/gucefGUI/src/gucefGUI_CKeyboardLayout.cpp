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

#include "gucefGUI_CKeyboardLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyboardLayout::SKey::SKey( void )
    : keyCode( INPUT::KEYCODE_UNASSIGNED )
    , label()
    , shiftLabel()
    , text()
    , shiftText()
    , width( 1.0f )
    , capsAffected( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout::CKeyboardLayout( void )
    : m_name()
    , m_rows()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout::CKeyboardLayout( const CString& name )
    : m_name( name )
    , m_rows()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout::CKeyboardLayout( const CKeyboardLayout& src )
    : m_name( src.m_name )
    , m_rows( src.m_rows )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout::~CKeyboardLayout()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout&
CKeyboardLayout::operator=( const CKeyboardLayout& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_name = src.m_name;
        m_rows = src.m_rows;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CKeyboardLayout::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CKeyboardLayout::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardLayout::AddRow( void )
{GUCEF_TRACE;

    m_rows.push_back( TKeyRow() );
    return (UInt32) m_rows.size() - 1;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::AddKey( UInt32 row, const TKey& key )
{GUCEF_TRACE;

    if ( row >= (UInt32) m_rows.size() )
        return false;
    m_rows[ row ].push_back( key );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::AddKey( UInt32 row, INPUT::KeyCode keyCode, const CString& label, const CString& shiftLabel,
                         const CString& text, const CString& shiftText, Float32 width, bool capsAffected )
{GUCEF_TRACE;

    TKey key;
    key.keyCode = keyCode;
    key.label = label;
    key.shiftLabel = shiftLabel;
    key.text = text;
    key.shiftText = shiftText;
    key.width = width;
    key.capsAffected = capsAffected;
    return AddKey( row, key );
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::AddKey( UInt32 row, INPUT::KeyCode keyCode, const CString& label, Float32 width )
{GUCEF_TRACE;

    TKey key;
    key.keyCode = keyCode;
    key.label = label;
    key.width = width;
    return AddKey( row, key );
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::AddGap( UInt32 row, Float32 width )
{GUCEF_TRACE;

    TKey key;
    key.width = width;
    return AddKey( row, key );
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardLayout::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_rows.size();
}

/*-------------------------------------------------------------------------*/

const CKeyboardLayout::TKeyRow&
CKeyboardLayout::GetRow( UInt32 row ) const
{GUCEF_TRACE;

    static const TKeyRow noRow;
    return row < (UInt32) m_rows.size() ? m_rows[ row ] : noRow;
}

/*-------------------------------------------------------------------------*/

const CKeyboardLayout::TKeyRows&
CKeyboardLayout::GetRows( void ) const
{GUCEF_TRACE;

    return m_rows;
}

/*-------------------------------------------------------------------------*/

Float32
CKeyboardLayout::GetRowWidth( UInt32 row ) const
{GUCEF_TRACE;

    Float32 width = 0.0f;
    const TKeyRow& keys = GetRow( row );
    for ( TKeyRow::const_iterator i=keys.begin(); i!=keys.end(); ++i )
        width += (*i).width;
    return width;
}

/*-------------------------------------------------------------------------*/

Float32
CKeyboardLayout::GetWidth( void ) const
{GUCEF_TRACE;

    Float32 width = 0.0f;
    for ( UInt32 i=0; i<(UInt32) m_rows.size(); ++i )
    {
        const Float32 rowWidth = GetRowWidth( i );
        if ( rowWidth > width )
            width = rowWidth;
    }
    return width;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::FindKey( INPUT::KeyCode keyCode, UInt32& row, UInt32& index ) const
{GUCEF_TRACE;

    if ( INPUT::KEYCODE_UNASSIGNED == keyCode )
        return false;
    for ( UInt32 r=0; r<(UInt32) m_rows.size(); ++r )
    {
        const TKeyRow& keys = m_rows[ r ];
        for ( UInt32 k=0; k<(UInt32) keys.size(); ++k )
        {
            if ( keys[ k ].keyCode == keyCode )
            {
                row = r;
                index = k;
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyboardLayout::GetKeyCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TKeyRows::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r )
        for ( TKeyRow::const_iterator k=(*r).begin(); k!=(*r).end(); ++k )
            if ( INPUT::KEYCODE_UNASSIGNED != (*k).keyCode )
                ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::IsValid( void ) const
{GUCEF_TRACE;

    if ( m_rows.empty() || GetKeyCount() == 0 )
        return false;
    for ( TKeyRows::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r )
    {
        if ( (*r).empty() )
            return false;
        for ( TKeyRow::const_iterator k=(*r).begin(); k!=(*r).end(); ++k )
            if ( !( (*k).width > 0.0f ) || (Int32) (*k).keyCode < 0 || (Int32) (*k).keyCode >= (Int32) INPUT::KEYCODE_MAX )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CKeyboardLayout::Save( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    tree.Clear();
    tree.SetName( "KeyboardLayout" );
    tree.SetAttribute( "name", m_name );
    for ( TKeyRows::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r )
    {
        CORE::CDataNode* rowNode = tree.AddChild( "Row" );
        for ( TKeyRow::const_iterator k=(*r).begin(); k!=(*r).end(); ++k )
        {
            const TKey& key = (*k);
            CORE::CDataNode* keyNode = rowNode->AddChild( "Key" );
            if ( INPUT::KEYCODE_UNASSIGNED != key.keyCode )
                keyNode->SetAttribute( "code", INPUT::KeyCodeToString( key.keyCode ) );
            if ( !key.label.IsNULLOrEmpty() )
                keyNode->SetAttribute( "label", key.label );
            if ( !key.shiftLabel.IsNULLOrEmpty() )
                keyNode->SetAttribute( "shiftLabel", key.shiftLabel );
            if ( !key.text.IsNULLOrEmpty() )
                keyNode->SetAttribute( "text", key.text );
            if ( !key.shiftText.IsNULLOrEmpty() )
                keyNode->SetAttribute( "shiftText", key.shiftText );
            if ( 1.0f != key.width )
                keyNode->SetAttribute( "width", key.width );
            if ( key.capsAffected )
                keyNode->SetAttribute( "caps", true );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CKeyboardLayout::Load( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    if ( tree.GetName() != "KeyboardLayout" )
        return false;

    CKeyboardLayout loaded( tree.GetAttributeValue( "name" ).AsString() );
    for ( CORE::CDataNode::const_iterator r=tree.ConstBegin(); r!=tree.ConstEnd(); ++r )
    {
        const CORE::CDataNode* rowNode = (*r);
        if ( rowNode->GetName() != "Row" )
            continue;
        const UInt32 row = loaded.AddRow();
        for ( CORE::CDataNode::const_iterator k=rowNode->ConstBegin(); k!=rowNode->ConstEnd(); ++k )
        {
            const CORE::CDataNode* keyNode = (*k);
            if ( keyNode->GetName() != "Key" )
                continue;

            TKey key;
            const CString code = keyNode->GetAttributeValue( "code" ).AsString();
            if ( !code.IsNULLOrEmpty() )
            {
                key.keyCode = INPUT::StringToKeyCode( code );
                if ( INPUT::KEYCODE_UNASSIGNED == key.keyCode )
                    return false;
            }
            key.label = keyNode->GetAttributeValue( "label" ).AsString();
            key.shiftLabel = keyNode->GetAttributeValue( "shiftLabel" ).AsString();
            key.text = keyNode->GetAttributeValue( "text" ).AsString();
            key.shiftText = keyNode->GetAttributeValue( "shiftText" ).AsString();
            key.width = keyNode->GetAttributeValue( "width" ).AsFloat32( 1.0f );
            key.capsAffected = keyNode->GetAttributeValue( "caps" ).AsBool( false );
            loaded.AddKey( row, key );
        }
    }
    if ( !loaded.IsValid() )
        return false;

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

/** A key that types text, without a separate shifted label: the label is the text */
void
AddTextKey( CKeyboardLayout& layout, UInt32 row, INPUT::KeyCode keyCode, const char* text, const char* shiftText, bool letter )
{
    layout.AddKey( row, keyCode, text, shiftText, text, shiftText, 1.0f, letter );
}

}

/*-------------------------------------------------------------------------*/

CKeyboardLayout
CKeyboardLayout::MakeUS( void )
{GUCEF_TRACE;

    CKeyboardLayout layout( "US" );

    // the function row: 15 key widths, as the rows below
    UInt32 row = layout.AddRow();
    layout.AddKey( row, INPUT::KEYCODE_ESCAPE, "Esc" );
    layout.AddKey( row, INPUT::KEYCODE_F1, "F1" );
    layout.AddKey( row, INPUT::KEYCODE_F2, "F2" );
    layout.AddKey( row, INPUT::KEYCODE_F3, "F3" );
    layout.AddKey( row, INPUT::KEYCODE_F4, "F4" );
    layout.AddKey( row, INPUT::KEYCODE_F5, "F5" );
    layout.AddKey( row, INPUT::KEYCODE_F6, "F6" );
    layout.AddKey( row, INPUT::KEYCODE_F7, "F7" );
    layout.AddKey( row, INPUT::KEYCODE_F8, "F8" );
    layout.AddKey( row, INPUT::KEYCODE_F9, "F9" );
    layout.AddKey( row, INPUT::KEYCODE_F10, "F10" );
    layout.AddKey( row, INPUT::KEYCODE_F11, "F11" );
    layout.AddKey( row, INPUT::KEYCODE_F12, "F12" );
    layout.AddKey( row, INPUT::KEYCODE_INSERT, "Ins" );
    layout.AddKey( row, INPUT::KEYCODE_DELETE, "Del" );

    row = layout.AddRow();
    AddTextKey( layout, row, INPUT::KEYCODE_GRAVE, "`", "~", false );
    AddTextKey( layout, row, INPUT::KEYCODE_1, "1", "!", false );
    AddTextKey( layout, row, INPUT::KEYCODE_2, "2", "@", false );
    AddTextKey( layout, row, INPUT::KEYCODE_3, "3", "#", false );
    AddTextKey( layout, row, INPUT::KEYCODE_4, "4", "$", false );
    AddTextKey( layout, row, INPUT::KEYCODE_5, "5", "%", false );
    AddTextKey( layout, row, INPUT::KEYCODE_6, "6", "^", false );
    AddTextKey( layout, row, INPUT::KEYCODE_7, "7", "&", false );
    AddTextKey( layout, row, INPUT::KEYCODE_8, "8", "*", false );
    AddTextKey( layout, row, INPUT::KEYCODE_9, "9", "(", false );
    AddTextKey( layout, row, INPUT::KEYCODE_0, "0", ")", false );
    AddTextKey( layout, row, INPUT::KEYCODE_MINUS, "-", "_", false );
    AddTextKey( layout, row, INPUT::KEYCODE_EQUALS, "=", "+", false );
    layout.AddKey( row, INPUT::KEYCODE_BACK, "Bksp", 2.0f );

    row = layout.AddRow();
    layout.AddKey( row, INPUT::KEYCODE_TAB, "Tab", 1.5f );
    AddTextKey( layout, row, INPUT::KEYCODE_Q, "q", "Q", true );
    AddTextKey( layout, row, INPUT::KEYCODE_W, "w", "W", true );
    AddTextKey( layout, row, INPUT::KEYCODE_E, "e", "E", true );
    AddTextKey( layout, row, INPUT::KEYCODE_R, "r", "R", true );
    AddTextKey( layout, row, INPUT::KEYCODE_T, "t", "T", true );
    AddTextKey( layout, row, INPUT::KEYCODE_Y, "y", "Y", true );
    AddTextKey( layout, row, INPUT::KEYCODE_U, "u", "U", true );
    AddTextKey( layout, row, INPUT::KEYCODE_I, "i", "I", true );
    AddTextKey( layout, row, INPUT::KEYCODE_O, "o", "O", true );
    AddTextKey( layout, row, INPUT::KEYCODE_P, "p", "P", true );
    AddTextKey( layout, row, INPUT::KEYCODE_LBRACKET, "[", "{", false );
    AddTextKey( layout, row, INPUT::KEYCODE_RBRACKET, "]", "}", false );
    layout.AddKey( row, INPUT::KEYCODE_BACKSLASH, "\\", "|", "\\", "|", 1.5f, false );

    row = layout.AddRow();
    layout.AddKey( row, INPUT::KEYCODE_CAPITAL, "Caps", 1.75f );
    AddTextKey( layout, row, INPUT::KEYCODE_A, "a", "A", true );
    AddTextKey( layout, row, INPUT::KEYCODE_S, "s", "S", true );
    AddTextKey( layout, row, INPUT::KEYCODE_D, "d", "D", true );
    AddTextKey( layout, row, INPUT::KEYCODE_F, "f", "F", true );
    AddTextKey( layout, row, INPUT::KEYCODE_G, "g", "G", true );
    AddTextKey( layout, row, INPUT::KEYCODE_H, "h", "H", true );
    AddTextKey( layout, row, INPUT::KEYCODE_J, "j", "J", true );
    AddTextKey( layout, row, INPUT::KEYCODE_K, "k", "K", true );
    AddTextKey( layout, row, INPUT::KEYCODE_L, "l", "L", true );
    AddTextKey( layout, row, INPUT::KEYCODE_SEMICOLON, ";", ":", false );
    AddTextKey( layout, row, INPUT::KEYCODE_APOSTROPHE, "'", "\"", false );
    layout.AddKey( row, INPUT::KEYCODE_RETURN, "Enter", 2.25f );

    row = layout.AddRow();
    layout.AddKey( row, INPUT::KEYCODE_LSHIFT, "Shift", 2.25f );
    AddTextKey( layout, row, INPUT::KEYCODE_Z, "z", "Z", true );
    AddTextKey( layout, row, INPUT::KEYCODE_X, "x", "X", true );
    AddTextKey( layout, row, INPUT::KEYCODE_C, "c", "C", true );
    AddTextKey( layout, row, INPUT::KEYCODE_V, "v", "V", true );
    AddTextKey( layout, row, INPUT::KEYCODE_B, "b", "B", true );
    AddTextKey( layout, row, INPUT::KEYCODE_N, "n", "N", true );
    AddTextKey( layout, row, INPUT::KEYCODE_M, "m", "M", true );
    AddTextKey( layout, row, INPUT::KEYCODE_COMMA, ",", "<", false );
    AddTextKey( layout, row, INPUT::KEYCODE_PERIOD, ".", ">", false );
    AddTextKey( layout, row, INPUT::KEYCODE_SLASH, "/", "?", false );
    layout.AddKey( row, INPUT::KEYCODE_RSHIFT, "Shift", 2.75f );

    row = layout.AddRow();
    layout.AddKey( row, INPUT::KEYCODE_LCONTROL, "Ctrl", 1.5f );
    layout.AddKey( row, INPUT::KEYCODE_LMENU, "Alt", 1.5f );
    layout.AddKey( row, INPUT::KEYCODE_SPACE, "Space", "Space", " ", " ", 6.5f, false );
    layout.AddKey( row, INPUT::KEYCODE_RMENU, "Alt", 1.5f );
    layout.AddKey( row, INPUT::KEYCODE_RCONTROL, "Ctrl", 1.5f );
    layout.AddKey( row, INPUT::KEYCODE_LEFT, "<", 0.625f );
    layout.AddKey( row, INPUT::KEYCODE_UP, "^", 0.625f );
    layout.AddKey( row, INPUT::KEYCODE_DOWN, "v", 0.625f );
    layout.AddKey( row, INPUT::KEYCODE_RIGHT, ">", 0.625f );
    return layout;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
