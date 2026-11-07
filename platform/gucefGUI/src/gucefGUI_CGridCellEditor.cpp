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

#include "gucefGUI_CGridCellEditor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsControl( UInt32 codePoint )
{
    return codePoint < 32 || 127 == codePoint || ( codePoint >= 0x80 && codePoint < 0xA0 );
}

UInt32
ToLower( UInt32 codePoint )
{
    return codePoint >= 'A' && codePoint <= 'Z' ? codePoint + 32 : codePoint;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridCellEditor::CGridCellEditor( void )
    : m_kind( KIND_NONE )
    , m_row( -1 )
    , m_column( -1 )
    , m_codePoints()
    , m_text()
    , m_textValid( true )
    , m_original()
    , m_caret( 0 )
    , m_anchor( 0 )
    , m_extraCharacters()
    , m_choices()
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::Open( EKind kind, Int32 row, Int32 column, const CString& text, bool selectAll )
{GUCEF_TRACE;

    m_kind = kind;
    m_row = row;
    m_column = column;
    m_error.Clear();
    m_extraCharacters.Clear();
    m_choices.clear();
    m_codePoints.clear();
    DecodeUtf8( text, m_codePoints );
    if ( m_codePoints.size() > (size_t) MAX_CHARACTERS )
        m_codePoints.resize( MAX_CHARACTERS );
    m_text = text;
    m_textValid = false;
    m_original = GetText();
    m_caret = (UInt32) m_codePoints.size();
    m_anchor = selectAll ? 0 : m_caret;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::Close( void )
{GUCEF_TRACE;

    m_kind = KIND_NONE;
    m_row = -1;
    m_column = -1;
    m_codePoints.clear();
    m_text.Clear();
    m_textValid = true;
    m_original.Clear();
    m_caret = 0;
    m_anchor = 0;
    m_choices.clear();
    m_error.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::IsOpen( void ) const
{GUCEF_TRACE;

    return KIND_NONE != m_kind;
}

/*-------------------------------------------------------------------------*/

CGridCellEditor::EKind
CGridCellEditor::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

Int32
CGridCellEditor::GetRow( void ) const
{GUCEF_TRACE;

    return m_row;
}

/*-------------------------------------------------------------------------*/

Int32
CGridCellEditor::GetColumn( void ) const
{GUCEF_TRACE;

    return m_column;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::SetRow( Int32 row )
{GUCEF_TRACE;

    m_row = row;
}

/*-------------------------------------------------------------------------*/

const CString&
CGridCellEditor::GetText( void ) const
{GUCEF_TRACE;

    if ( !m_textValid )
    {
        m_text = GetTextBetween( 0, (UInt32) m_codePoints.size() );
        m_textValid = true;
    }
    return m_text;
}

/*-------------------------------------------------------------------------*/

const CString&
CGridCellEditor::GetOriginalText( void ) const
{GUCEF_TRACE;

    return m_original;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::IsModified( void ) const
{GUCEF_TRACE;

    return GetText() != m_original;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridCellEditor::GetCharacterCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_codePoints.size();
}

/*-------------------------------------------------------------------------*/

CString
CGridCellEditor::GetTextBefore( UInt32 index ) const
{GUCEF_TRACE;

    return GetTextBetween( 0, index );
}

/*-------------------------------------------------------------------------*/

CString
CGridCellEditor::GetTextBetween( UInt32 first, UInt32 last ) const
{GUCEF_TRACE;

    CString text;
    if ( last > (UInt32) m_codePoints.size() )
        last = (UInt32) m_codePoints.size();
    for ( UInt32 i=first; i<last; ++i )
        text.Append( m_codePoints[ i ] );
    return text;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridCellEditor::GetCaret( void ) const
{GUCEF_TRACE;

    return m_caret;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::HasSelection( void ) const
{GUCEF_TRACE;

    return m_caret != m_anchor;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::GetSelection( UInt32& first, UInt32& last ) const
{GUCEF_TRACE;

    first = m_caret < m_anchor ? m_caret : m_anchor;
    last = m_caret < m_anchor ? m_anchor : m_caret;
}

/*-------------------------------------------------------------------------*/

CString
CGridCellEditor::GetSelectedText( void ) const
{GUCEF_TRACE;

    UInt32 first = 0, last = 0;
    GetSelection( first, last );
    return GetTextBetween( first, last );
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::SelectAll( void )
{GUCEF_TRACE;

    m_anchor = 0;
    m_caret = (UInt32) m_codePoints.size();
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::MoveCaretTo( UInt32 index, bool extend )
{GUCEF_TRACE;

    if ( index > (UInt32) m_codePoints.size() )
        index = (UInt32) m_codePoints.size();
    m_caret = index;
    if ( !extend )
        m_anchor = index;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::MoveCaret( Int32 delta, bool extend )
{GUCEF_TRACE;

    // without Shift, a selection ends at the side the caret moves to
    if ( !extend && HasSelection() )
    {
        UInt32 first = 0, last = 0;
        GetSelection( first, last );
        MoveCaretTo( delta < 0 ? first : last, false );
        return;
    }
    Int32 target = (Int32) m_caret + delta;
    if ( target < 0 )
        target = 0;
    MoveCaretTo( (UInt32) target, extend );
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::MoveToStart( bool extend )
{GUCEF_TRACE;

    MoveCaretTo( 0, extend );
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::MoveToEnd( bool extend )
{GUCEF_TRACE;

    MoveCaretTo( (UInt32) m_codePoints.size(), extend );
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::AcceptsCharacter( UInt32 codePoint ) const
{GUCEF_TRACE;

    if ( IsControl( codePoint ) )
        return false;
    if ( KIND_NUMBER != m_kind )
        return true;
    if ( ( codePoint >= '0' && codePoint <= '9' ) || '-' == codePoint || '+' == codePoint || '.' == codePoint || ',' == codePoint || 'e' == codePoint || 'E' == codePoint || ' ' == codePoint )
        return true;
    if ( !m_extraCharacters.IsNULLOrEmpty() )
    {
        TCodePointVector extra;
        DecodeUtf8( m_extraCharacters, extra );
        for ( size_t i=0; i<extra.size(); ++i )
            if ( extra[ i ] == codePoint )
                return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::TextChanged( void )
{GUCEF_TRACE;

    m_textValid = false;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::RemoveRange( UInt32 first, UInt32 last )
{GUCEF_TRACE;

    if ( last > (UInt32) m_codePoints.size() )
        last = (UInt32) m_codePoints.size();
    if ( first >= last )
        return false;
    m_codePoints.erase( m_codePoints.begin() + first, m_codePoints.begin() + last );
    m_caret = first;
    m_anchor = first;
    TextChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::InsertText( const CString& text )
{GUCEF_TRACE;

    if ( KIND_NONE == m_kind )
        return false;
    TCodePointVector typed;
    DecodeUtf8( text, typed );
    TCodePointVector accepted;
    for ( size_t i=0; i<typed.size(); ++i )
        if ( AcceptsCharacter( typed[ i ] ) )
            accepted.push_back( typed[ i ] );
    if ( accepted.empty() )
        return false;

    UInt32 first = 0, last = 0;
    GetSelection( first, last );
    const size_t remaining = m_codePoints.size() - ( last - first );
    if ( remaining + accepted.size() > (size_t) MAX_CHARACTERS )
        accepted.resize( remaining >= (size_t) MAX_CHARACTERS ? 0 : (size_t) MAX_CHARACTERS - remaining );
    if ( accepted.empty() )
        return false;
    m_codePoints.erase( m_codePoints.begin() + first, m_codePoints.begin() + last );
    m_codePoints.insert( m_codePoints.begin() + first, accepted.begin(), accepted.end() );
    m_caret = first + (UInt32) accepted.size();
    m_anchor = m_caret;
    TextChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::Backspace( void )
{GUCEF_TRACE;

    if ( HasSelection() )
        return DeleteSelection();
    if ( 0 == m_caret )
        return false;
    return RemoveRange( m_caret - 1, m_caret );
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::DeleteForward( void )
{GUCEF_TRACE;

    if ( HasSelection() )
        return DeleteSelection();
    return RemoveRange( m_caret, m_caret + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::DeleteSelection( void )
{GUCEF_TRACE;

    UInt32 first = 0, last = 0;
    GetSelection( first, last );
    return RemoveRange( first, last );
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::SetText( const CString& text )
{GUCEF_TRACE;

    if ( text == GetText() )
    {
        m_caret = (UInt32) m_codePoints.size();
        m_anchor = m_caret;
        return false;
    }
    m_codePoints.clear();
    DecodeUtf8( text, m_codePoints );
    if ( m_codePoints.size() > (size_t) MAX_CHARACTERS )
        m_codePoints.resize( MAX_CHARACTERS );
    m_caret = (UInt32) m_codePoints.size();
    m_anchor = m_caret;
    TextChanged();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::SetExtraCharacters( const CString& characters )
{GUCEF_TRACE;

    m_extraCharacters = characters;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::SetChoices( const GUCEF::vector< CString >& choices )
{GUCEF_TRACE;

    m_choices = choices;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< CString >&
CGridCellEditor::GetChoices( void ) const
{GUCEF_TRACE;

    return m_choices;
}

/*-------------------------------------------------------------------------*/

Int32
CGridCellEditor::GetChoiceIndex( void ) const
{GUCEF_TRACE;

    const CString& text = GetText();
    for ( size_t i=0; i<m_choices.size(); ++i )
        if ( m_choices[ i ] == text )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::SetChoiceIndex( Int32 index )
{GUCEF_TRACE;

    if ( index < 0 || index >= (Int32) m_choices.size() )
        return false;
    SetText( m_choices[ index ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::MoveChoice( Int32 step )
{GUCEF_TRACE;

    if ( m_choices.empty() )
        return false;
    const Int32 current = GetChoiceIndex();
    Int32 target = current < 0 ? ( step > 0 ? 0 : (Int32) m_choices.size() - 1 ) : current + step;
    if ( target < 0 )
        target = 0;
    if ( target >= (Int32) m_choices.size() )
        target = (Int32) m_choices.size() - 1;
    return SetChoiceIndex( target );
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::TypeAheadChoice( const CString& typed )
{GUCEF_TRACE;

    TCodePointVector wanted;
    DecodeUtf8( typed, wanted );
    if ( wanted.empty() || m_choices.empty() )
        return false;
    const Int32 count = (Int32) m_choices.size();
    const Int32 start = GetChoiceIndex();
    for ( Int32 step=1; step<=count; ++step )
    {
        const Int32 index = ( start + step + count ) % count;
        TCodePointVector choice;
        DecodeUtf8( m_choices[ index ], choice );
        if ( choice.size() < wanted.size() )
            continue;
        bool same = true;
        for ( size_t i=0; i<wanted.size() && same; ++i )
            same = ToLower( choice[ i ] ) == ToLower( wanted[ i ] );
        if ( same )
            return SetChoiceIndex( index );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CGridCellEditor::SetError( const CString& message )
{GUCEF_TRACE;

    m_error = message;
}

/*-------------------------------------------------------------------------*/

bool
CGridCellEditor::HasError( void ) const
{GUCEF_TRACE;

    return !m_error.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CString&
CGridCellEditor::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
