/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#include "authpluginSTEAM_CSteamKeyValues.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CSteamKeyValues::TIndex CSteamKeyValues::RootIndex;
const CSteamKeyValues::TIndex CSteamKeyValues::NoIndex;
const UInt32 CSteamKeyValues::MaxBytes;
const UInt32 CSteamKeyValues::MaxDepth;
const UInt32 CSteamKeyValues::MaxEntries;
const UInt32 CSteamKeyValues::MaxTokenBytes;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

enum ETokenKind
{
    TOKEN_END = 0       ,
    TOKEN_TEXT          ,       /**< a quoted or a bare string */
    TOKEN_OPEN          ,       /**< { */
    TOKEN_CLOSE         ,       /**< } */
    TOKEN_BAD                   /**< a string that is too long or not closed */
};

/** One pass over the text, a token at a time */
class CReader
{
    public:

    CReader( const char* text, size_t size ) : m_text( text ), m_size( size ), m_pos( 0 ) {}

    ETokenKind Next( CORE::CString& token )
    {
        token.Clear();
        SkipBlanksAndComments();
        if ( m_pos >= m_size )
            return TOKEN_END;

        const char c = m_text[ m_pos ];
        if ( '{' == c ) { ++m_pos; return TOKEN_OPEN; }
        if ( '}' == c ) { ++m_pos; return TOKEN_CLOSE; }
        if ( '"' == c )
            return ReadQuoted( token );
        return ReadBare( token );
    }

    /** Skips a conditional such as [$WIN32] that follows a value; leaves the text alone when there is none */
    void SkipConditional( void )
    {
        SkipBlanksAndComments();
        if ( m_pos < m_size && '[' == m_text[ m_pos ] )
        {
            while ( m_pos < m_size && ']' != m_text[ m_pos ] && '\n' != m_text[ m_pos ] )
                ++m_pos;
            if ( m_pos < m_size && ']' == m_text[ m_pos ] )
                ++m_pos;
        }
    }

    private:

    static bool IsBlank( char c ) { return ' ' == c || '\t' == c || '\r' == c || '\n' == c || '\0' == c; }

    void SkipBlanksAndComments( void )
    {
        while ( m_pos < m_size )
        {
            const char c = m_text[ m_pos ];
            if ( IsBlank( c ) )
            {
                ++m_pos;
            }
            else
            if ( '/' == c && m_pos + 1 < m_size && '/' == m_text[ m_pos + 1 ] )
            {
                while ( m_pos < m_size && '\n' != m_text[ m_pos ] )
                    ++m_pos;
            }
            else
                break;
        }
    }

    ETokenKind ReadQuoted( CORE::CString& token )
    {
        ++m_pos;    // the opening quote
        const size_t begin = m_pos;
        bool escapes = false;
        // the end of the string first, the escapes are undone in a second step
        while ( m_pos < m_size && '"' != m_text[ m_pos ] )
        {
            if ( '\\' == m_text[ m_pos ] && m_pos + 1 < m_size )
            {
                escapes = true;
                ++m_pos;
            }
            ++m_pos;
            if ( m_pos - begin > CSteamKeyValues::MaxTokenBytes * 2 )
                return TOKEN_BAD;
        }
        if ( m_pos >= m_size )
            return TOKEN_BAD;
        const size_t end = m_pos;
        ++m_pos;    // the closing quote

        if ( end - begin > CSteamKeyValues::MaxTokenBytes )
            return TOKEN_BAD;
        CORE::CString text;
        if ( !escapes )
        {
            text = CORE::CString( m_text + begin, (UInt32)( end - begin ) );
        }
        else
        {
            char buffer[ CSteamKeyValues::MaxTokenBytes + 1 ];
            size_t out = 0;
            for ( size_t i=begin; i<end && out < CSteamKeyValues::MaxTokenBytes; ++i )
            {
                char c = m_text[ i ];
                if ( '\\' == c && i + 1 < end )
                {
                    const char e = m_text[ i + 1 ];
                    if ( 'n' == e )  { c = '\n'; ++i; }
                    else if ( 't' == e )  { c = '\t'; ++i; }
                    else if ( '\\' == e ) { c = '\\'; ++i; }
                    else if ( '"' == e )  { c = '"'; ++i; }
                }
                buffer[ out++ ] = c;
            }
            text = CORE::CString( buffer, (UInt32) out );
        }
        token = text;
        return TOKEN_TEXT;
    }

    ETokenKind ReadBare( CORE::CString& token )
    {
        const size_t begin = m_pos;
        while ( m_pos < m_size )
        {
            const char c = m_text[ m_pos ];
            if ( IsBlank( c ) || '{' == c || '}' == c || '"' == c )
                break;
            ++m_pos;
            if ( m_pos - begin > CSteamKeyValues::MaxTokenBytes )
                return TOKEN_BAD;
        }
        token = CORE::CString( m_text + begin, (UInt32)( m_pos - begin ) );
        return TOKEN_TEXT;
    }

    const char* m_text;
    size_t m_size;
    size_t m_pos;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSteamKeyValues::CSteamKeyValues( void )
    : m_entries()
{GUCEF_TRACE;

    m_entries.push_back( SEntry() );
    m_entries[ 0 ].isBlock = true;
}

/*-------------------------------------------------------------------------*/

CSteamKeyValues::CSteamKeyValues( const CSteamKeyValues& src )
    : m_entries( src.m_entries )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamKeyValues::~CSteamKeyValues()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSteamKeyValues&
CSteamKeyValues::operator=( const CSteamKeyValues& src )
{GUCEF_TRACE;

    if ( this != &src )
        m_entries = src.m_entries;
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CSteamKeyValues::Clear( void )
{GUCEF_TRACE;

    m_entries.clear();
    m_entries.push_back( SEntry() );
    m_entries[ 0 ].isBlock = true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamKeyValues::Parse( const CORE::CString& text )
{GUCEF_TRACE;

    Clear();

    const char* chars = text.C_String();
    if ( GUCEF_NULL == chars )
        return true;
    size_t size = strlen( chars );
    bool withinLimits = true;
    if ( size > MaxBytes )
    {
        size = MaxBytes;
        withinLimits = false;
    }
    // a byte order mark before the text
    if ( size >= 3 && (unsigned char) chars[ 0 ] == 0xEF && (unsigned char) chars[ 1 ] == 0xBB && (unsigned char) chars[ 2 ] == 0xBF )
    {
        chars += 3;
        size -= 3;
    }

    CReader reader( chars, size );
    TIndexVector open;                  // the blocks that are open, the innermost last
    open.push_back( RootIndex );

    CORE::CString key, value;
    for ( ;; )
    {
        ETokenKind kind = reader.Next( key );
        if ( TOKEN_END == kind )
            return withinLimits && 1 == open.size();    // a block that is not closed is text that was cut short
        if ( TOKEN_BAD == kind || TOKEN_OPEN == kind )
            return false;
        if ( TOKEN_CLOSE == kind )
        {
            if ( open.size() <= 1 )
                return false;                           // a closing brace too many
            open.pop_back();
            continue;
        }

        // a key: a block or a value follows
        kind = reader.Next( value );
        if ( TOKEN_OPEN == kind )
        {
            if ( open.size() > MaxDepth || m_entries.size() > MaxEntries )
                return false;
            SEntry block;
            block.key = key;
            block.isBlock = true;
            block.parent = open.back();
            m_entries.push_back( block );
            open.push_back( (TIndex)( m_entries.size() - 1 ) );
        }
        else
        if ( TOKEN_TEXT == kind )
        {
            if ( m_entries.size() > MaxEntries )
                return false;
            SEntry entry;
            entry.key = key;
            entry.value = value;
            entry.parent = open.back();
            m_entries.push_back( entry );
            reader.SkipConditional();
        }
        else
            return false;                               // a key without anything, a value that is too long, the end of the text
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CSteamKeyValues::GetEntryCount( void ) const
{GUCEF_TRACE;

    return (UInt32)( m_entries.size() - 1 );
}

/*-------------------------------------------------------------------------*/

static bool
KeysAreEqual( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    return a.Lowercase() == b.Lowercase();
}

/*-------------------------------------------------------------------------*/

CSteamKeyValues::TIndex
CSteamKeyValues::FindChild( TIndex parent, const CORE::CString& key ) const
{GUCEF_TRACE;

    if ( parent >= m_entries.size() )
        return NoIndex;
    const CORE::CString wanted = key.Lowercase();
    for ( size_t i=1; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].parent == parent && m_entries[ i ].key.Lowercase() == wanted )
            return (TIndex) i;
    }
    return NoIndex;
}

/*-------------------------------------------------------------------------*/

void
CSteamKeyValues::GetChildren( TIndex parent, TIndexVector& children ) const
{GUCEF_TRACE;

    children.clear();
    for ( size_t i=1; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].parent == parent )
            children.push_back( (TIndex) i );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSteamKeyValues::GetChildValue( TIndex parent, const CORE::CString& key, const CORE::CString& defaultValue ) const
{GUCEF_TRACE;

    const TIndex child = FindChild( parent, key );
    if ( NoIndex == child || m_entries[ child ].isBlock )
        return defaultValue;
    return m_entries[ child ].value;
}

/*-------------------------------------------------------------------------*/

CSteamKeyValues::TIndex
CSteamKeyValues::FindBlock( TIndex parent, const CORE::CString::StringVector& path ) const
{GUCEF_TRACE;

    TIndex current = parent;
    for ( size_t i=0; i<path.size(); ++i )
    {
        current = FindChild( current, path[ i ] );
        if ( NoIndex == current || !m_entries[ current ].isBlock )
            return NoIndex;
    }
    return current;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamKeyValues::GetKey( TIndex index ) const
{GUCEF_TRACE;

    return index < m_entries.size() ? m_entries[ index ].key : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSteamKeyValues::GetValue( TIndex index ) const
{GUCEF_TRACE;

    return index < m_entries.size() ? m_entries[ index ].value : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CSteamKeyValues::IsBlock( TIndex index ) const
{GUCEF_TRACE;

    return index < m_entries.size() && m_entries[ index ].isBlock;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
