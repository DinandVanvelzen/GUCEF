/*
 *  assetpluginUNITY: the reader of the YAML dialect of Unity's text-serialised files
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "assetpluginUNITY_CUnityYamlReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< char > TCharVector;

/** A line of the text: where its content starts after the indentation, how long it is without the line end */
struct SLine
{
    const char* text;
    UInt32 length;
    UInt32 indent;
    UInt32 number;
};

typedef GUCEF::vector< SLine > TLineVector;

inline bool IsSpace( char c ) { return ' ' == c || '\t' == c; }

CORE::CString
MakeString( const char* text, UInt32 length )
{
    if ( 0 == length )
        return CORE::CString::Empty;
    return CORE::CString( text, length );
}

CORE::CString
MakeString( const TCharVector& chars )
{
    if ( chars.empty() )
        return CORE::CString::Empty;
    return CORE::CString( &chars[ 0 ], (UInt32) chars.size() );
}

/** Appends the UTF-8 encoding of a code point */
void
AppendUtf8( TCharVector& out, UInt32 cp )
{
    if ( cp < 0x80 )
    {
        out.push_back( (char) cp );
    }
    else
    if ( cp < 0x800 )
    {
        out.push_back( (char) ( 0xC0 | ( cp >> 6 ) ) );
        out.push_back( (char) ( 0x80 | ( cp & 0x3F ) ) );
    }
    else
    if ( cp < 0x10000 )
    {
        out.push_back( (char) ( 0xE0 | ( cp >> 12 ) ) );
        out.push_back( (char) ( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( cp & 0x3F ) ) );
    }
    else
    {
        out.push_back( (char) ( 0xF0 | ( cp >> 18 ) ) );
        out.push_back( (char) ( 0x80 | ( ( cp >> 12 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( cp & 0x3F ) ) );
    }
}

/** The value of 'digits' hexadecimal digits, false when one is not a digit */
bool
ReadHex( const char* text, UInt32 digits, UInt32& value )
{
    value = 0;
    for ( UInt32 i=0; i<digits; ++i )
    {
        const char c = text[ i ];
        UInt32 d = 0;
        if ( c >= '0' && c <= '9' ) d = (UInt32) ( c - '0' );
        else if ( c >= 'a' && c <= 'f' ) d = (UInt32) ( c - 'a' + 10 );
        else if ( c >= 'A' && c <= 'F' ) d = (UInt32) ( c - 'A' + 10 );
        else return false;
        value = value * 16 + d;
    }
    return true;
}

/**
 *  The parser of one document: recursive descent over the lines of the document, by indentation. Flow collections
 *  and quoted scalars may span lines; their text is gathered first and then read as one.
 */
class CDocumentParser
{
    public:

    CDocumentParser( TLineVector& lines, UInt32 first, UInt32 end )
        : m_lines( lines )
        , m_i( first )
        , m_end( end )
        , m_problem()
    {
    }

    bool ParseDocument( CORE::CDataNode& root )
    {
        SkipEmpty();
        if ( m_i >= m_end )
            return true;
        return ParseNode( root );
    }

    const CORE::CString& GetProblem( void ) const { return m_problem; }

    private:

    bool Fail( const CORE::CString& what )
    {
        const UInt32 line = m_i < m_end ? m_lines[ m_i ].number : ( m_end > 0 ? m_lines[ m_end-1 ].number : 0 );
        m_problem = "line " + CORE::UInt32ToString( line ) + ": " + what;
        return false;
    }

    static bool IsBlankOrComment( const SLine& line )
    {
        return 0 == line.length || '#' == line.text[ 0 ];
    }

    void SkipEmpty( void )
    {
        while ( m_i < m_end && IsBlankOrComment( m_lines[ m_i ] ) )
            ++m_i;
    }

    static bool IsSequenceText( const char* text, UInt32 length )
    {
        return length >= 1 && '-' == text[ 0 ] && ( 1 == length || IsSpace( text[ 1 ] ) );
    }

    static bool IsSequenceItem( const SLine& line )
    {
        return IsSequenceText( line.text, line.length );
    }

    /** The end of a plain key: the first ':' followed by a space or the end of the line; -1 when there is none */
    static Int32 FindKeyEnd( const char* text, UInt32 length )
    {
        if ( 0 == length )
            return -1;
        const char first = text[ 0 ];
        if ( '{' == first || '[' == first || '#' == first || '|' == first || '>' == first || '*' == first || '!' == first || '&' == first )
            return -1;
        if ( '"' == first || '\'' == first )
        {
            // a quoted key: the closing quote, then the colon
            UInt32 i = 1;
            while ( i < length )
            {
                if ( '"' == first && '\\' == text[ i ] ) { i += 2; continue; }
                if ( text[ i ] == first )
                {
                    if ( '\'' == first && i+1 < length && '\'' == text[ i+1 ] ) { i += 2; continue; }
                    break;
                }
                ++i;
            }
            if ( i >= length )
                return -1;
            ++i;
            while ( i < length && IsSpace( text[ i ] ) )
                ++i;
            if ( i < length && ':' == text[ i ] && ( i+1 == length || IsSpace( text[ i+1 ] ) ) )
                return (Int32) i;
            return -1;
        }
        for ( UInt32 i=0; i<length; ++i )
        {
            if ( ':' == text[ i ] && ( i+1 == length || IsSpace( text[ i+1 ] ) ) )
                return (Int32) i;
            if ( '#' == text[ i ] && i > 0 && IsSpace( text[ i-1 ] ) )
                return -1;
        }
        return -1;
    }

    /** The text of a key, unquoted */
    static CORE::CString KeyText( const char* text, UInt32 keyEnd )
    {
        UInt32 end = keyEnd;
        while ( end > 0 && IsSpace( text[ end-1 ] ) )
            --end;
        if ( end >= 2 && ( '"' == text[ 0 ] || '\'' == text[ 0 ] ) && text[ end-1 ] == text[ 0 ] )
        {
            TCharVector chars;
            ReadQuoted( text + 1, end - 2, text[ 0 ], chars );
            return MakeString( chars );
        }
        return MakeString( text, end );
    }

    /** Unescapes the content of a quoted scalar (without the quotes, on one line) */
    static void ReadQuoted( const char* text, UInt32 length, char quote, TCharVector& out )
    {
        for ( UInt32 i=0; i<length; ++i )
        {
            const char c = text[ i ];
            if ( '\'' == quote )
            {
                if ( '\'' == c && i+1 < length && '\'' == text[ i+1 ] )
                    ++i;
                out.push_back( c );
                continue;
            }
            if ( '\\' != c || i+1 >= length )
            {
                out.push_back( c );
                continue;
            }
            const char e = text[ ++i ];
            UInt32 cp = 0;
            switch ( e )
            {
                case 'n': out.push_back( '\n' ); break;
                case 't': out.push_back( '\t' ); break;
                case 'r': out.push_back( '\r' ); break;
                case '0': out.push_back( '\0' ); break;
                case 'a': out.push_back( '\a' ); break;
                case 'b': out.push_back( '\b' ); break;
                case 'e': out.push_back( (char) 0x1B ); break;
                case 'v': out.push_back( '\v' ); break;
                case 'f': out.push_back( '\f' ); break;
                case ' ': out.push_back( ' ' ); break;
                case '/': out.push_back( '/' ); break;
                case '"': out.push_back( '"' ); break;
                case '\\': out.push_back( '\\' ); break;
                case 'N': AppendUtf8( out, 0x85 ); break;
                case '_': AppendUtf8( out, 0xA0 ); break;
                case 'L': AppendUtf8( out, 0x2028 ); break;
                case 'P': AppendUtf8( out, 0x2029 ); break;
                case 'x':
                    if ( i+2 < length && ReadHex( text+i+1, 2, cp ) ) { AppendUtf8( out, cp ); i += 2; }
                    else out.push_back( e );
                    break;
                case 'u':
                    if ( i+4 < length && ReadHex( text+i+1, 4, cp ) ) { AppendUtf8( out, cp ); i += 4; }
                    else out.push_back( e );
                    break;
                case 'U':
                    if ( i+8 < length && ReadHex( text+i+1, 8, cp ) ) { AppendUtf8( out, cp ); i += 8; }
                    else out.push_back( e );
                    break;
                default:
                    out.push_back( e );
                    break;
            }
        }
    }

    /** Sets a scalar: an attribute of a mapping, or an item of a sequence */
    static void SetScalar( CORE::CDataNode& parent, const CORE::CString& key, bool inSequence, const CORE::CString& value )
    {
        if ( inSequence )
            parent.AddValueAsChild( value );
        else
            parent.SetAttribute( key, value );
    }

    /** A mapping or a sequence, whatever the current line starts */
    bool ParseNode( CORE::CDataNode& node )
    {
        SkipEmpty();
        if ( m_i >= m_end )
            return true;
        const SLine& line = m_lines[ m_i ];
        if ( IsSequenceItem( line ) )
        {
            node.SetNodeType( GUCEF_DATATYPE_ARRAY );
            return ParseSequence( line.indent, node );
        }
        node.SetNodeType( GUCEF_DATATYPE_OBJECT );
        return ParseMapping( line.indent, node );
    }

    bool ParseMapping( UInt32 indent, CORE::CDataNode& node )
    {
        while ( true )
        {
            SkipEmpty();
            if ( m_i >= m_end )
                return true;
            SLine& line = m_lines[ m_i ];
            if ( line.indent < indent )
                return true;
            if ( line.indent > indent )
                return Fail( "more indentation than the mapping has" );
            if ( IsSequenceItem( line ) )
                return true;    // a sequence at the indentation of its key ends that key's mapping level

            const Int32 keyEnd = FindKeyEnd( line.text, line.length );
            if ( keyEnd < 0 )
                return Fail( "a key is expected" );
            const CORE::CString key = KeyText( line.text, (UInt32) keyEnd );
            UInt32 pos = (UInt32) keyEnd + 1;
            while ( pos < line.length && IsSpace( line.text[ pos ] ) )
                ++pos;
            if ( pos >= line.length || '#' == line.text[ pos ] )
            {
                // the value is on the next lines: a nested block, a sequence at this indentation, or nothing
                ++m_i;
                SkipEmpty();
                if ( m_i < m_end && ( m_lines[ m_i ].indent > indent || ( m_lines[ m_i ].indent == indent && IsSequenceItem( m_lines[ m_i ] ) ) ) )
                {
                    CORE::CDataNode* child = node.AddChild( key, GUCEF_DATATYPE_OBJECT );
                    if ( !ParseNode( *child ) )
                        return false;
                }
                else
                {
                    node.SetAttribute( key, CORE::CString::Empty );
                }
                continue;
            }
            if ( !ParseValue( pos, indent, node, key, false ) )
                return false;
        }
    }

    bool ParseSequence( UInt32 indent, CORE::CDataNode& node )
    {
        while ( true )
        {
            SkipEmpty();
            if ( m_i >= m_end )
                return true;
            SLine& line = m_lines[ m_i ];
            if ( line.indent < indent )
                return true;
            if ( line.indent > indent )
                return Fail( "more indentation than the sequence has" );
            if ( !IsSequenceItem( line ) )
                return true;

            UInt32 offset = 1;
            while ( offset < line.length && IsSpace( line.text[ offset ] ) )
                ++offset;
            if ( offset >= line.length || '#' == line.text[ offset ] )
            {
                // the item is on the next lines
                ++m_i;
                SkipEmpty();
                if ( m_i < m_end && m_lines[ m_i ].indent > indent )
                {
                    CORE::CDataNode* child = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                    if ( !ParseNode( *child ) )
                        return false;
                }
                else
                {
                    node.AddValueAsChild( CORE::CString::Empty );
                }
                continue;
            }

            const char* content = line.text + offset;
            const UInt32 contentLength = line.length - offset;
            if ( IsSequenceText( content, contentLength ) || FindKeyEnd( content, contentLength ) >= 0 )
            {
                // "- key: value" or "- - item": the item is a block whose first line starts at the content
                line.indent += offset;
                line.text = content;
                line.length = contentLength;
                CORE::CDataNode* child = node.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                if ( !ParseNode( *child ) )
                    return false;
                continue;
            }
            if ( !ParseValue( offset, indent, node, CORE::CString::Empty, true ) )
                return false;
        }
    }

    /** Skips a tag ("!u!21", "!!str") or an anchor ("&id") before a value; the position after it */
    UInt32 SkipProperties( const SLine& line, UInt32 pos )
    {
        while ( pos < line.length && ( '!' == line.text[ pos ] || '&' == line.text[ pos ] ) )
        {
            while ( pos < line.length && !IsSpace( line.text[ pos ] ) )
                ++pos;
            while ( pos < line.length && IsSpace( line.text[ pos ] ) )
                ++pos;
        }
        return pos;
    }

    /** The value that starts on the current line at 'pos', of a key or a sequence item at 'indent' */
    bool ParseValue( UInt32 pos, UInt32 indent, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        const SLine& line = m_lines[ m_i ];
        pos = SkipProperties( line, pos );
        if ( pos >= line.length )
        {
            // only a tag or an anchor: the value is what follows on the next lines
            ++m_i;
            SkipEmpty();
            if ( m_i < m_end && m_lines[ m_i ].indent > indent )
            {
                CORE::CDataNode* child = parent.AddChild( inSequence ? CORE::CString::Empty : key, GUCEF_DATATYPE_OBJECT );
                return ParseNode( *child );
            }
            SetScalar( parent, key, inSequence, CORE::CString::Empty );
            return true;
        }
        const char c = line.text[ pos ];
        if ( '{' == c || '[' == c )
            return ParseFlowLines( pos, parent, key, inSequence );
        if ( '"' == c || '\'' == c )
            return ParseQuotedLines( pos, parent, key, inSequence );
        if ( '|' == c || '>' == c )
            return ParseBlockScalar( pos, indent, parent, key, inSequence );
        return ParsePlainLines( pos, indent, parent, key, inSequence );
    }

    /** A plain scalar: the rest of the line, and the more indented lines that follow it */
    bool ParsePlainLines( UInt32 pos, UInt32 indent, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        TCharVector chars;
        AppendPlain( m_lines[ m_i ].text + pos, m_lines[ m_i ].length - pos, chars );
        ++m_i;
        while ( m_i < m_end )
        {
            // blank lines inside the scalar are line breaks, if the scalar goes on after them
            UInt32 next = m_i;
            UInt32 blanks = 0;
            while ( next < m_end && 0 == m_lines[ next ].length )
            {
                ++blanks;
                ++next;
            }
            if ( next >= m_end || m_lines[ next ].indent <= indent || '#' == m_lines[ next ].text[ 0 ] )
                break;
            if ( 0 == blanks )
                chars.push_back( ' ' );
            for ( UInt32 b=0; b<blanks; ++b )
                chars.push_back( '\n' );
            AppendPlain( m_lines[ next ].text, m_lines[ next ].length, chars );
            m_i = next + 1;
        }
        SetScalar( parent, key, inSequence, MakeString( chars ) );
        return true;
    }

    /** The text of a plain scalar on one line: up to a comment, without the trailing spaces */
    static void AppendPlain( const char* text, UInt32 length, TCharVector& chars )
    {
        UInt32 end = length;
        for ( UInt32 i=1; i<length; ++i )
        {
            if ( '#' == text[ i ] && IsSpace( text[ i-1 ] ) )
            {
                end = i;
                break;
            }
        }
        while ( end > 0 && IsSpace( text[ end-1 ] ) )
            --end;
        chars.insert( chars.end(), text, text + end );
    }

    /** A quoted scalar that may go on over the next lines, folded as YAML folds them */
    bool ParseQuotedLines( UInt32 pos, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        const char quote = m_lines[ m_i ].text[ pos ];
        TCharVector raw;        // the content as it is written, lines folded
        UInt32 i = pos + 1;
        bool first = true;
        while ( m_i < m_end )
        {
            const SLine& line = m_lines[ m_i ];
            UInt32 start = first ? i : 0;
            if ( !first )
            {
                while ( start < line.length && IsSpace( line.text[ start ] ) )
                    ++start;
            }
            // find the closing quote on this line
            UInt32 j = start;
            bool closed = false;
            while ( j < line.length )
            {
                const char c = line.text[ j ];
                if ( '"' == quote && '\\' == c && j+1 < line.length )
                {
                    j += 2;
                    continue;
                }
                if ( c == quote )
                {
                    if ( '\'' == quote && j+1 < line.length && '\'' == line.text[ j+1 ] )
                    {
                        j += 2;
                        continue;
                    }
                    closed = true;
                    break;
                }
                ++j;
            }
            if ( closed )
            {
                raw.insert( raw.end(), line.text + start, line.text + j );
                ++m_i;
                TCharVector value;
                ReadQuoted( raw.empty() ? "" : &raw[ 0 ], (UInt32) raw.size(), quote, value );
                SetScalar( parent, key, inSequence, MakeString( value ) );
                return true;
            }

            // the line ends inside the scalar: fold it
            UInt32 end = line.length;
            bool escapedBreak = false;
            if ( '"' == quote && end > start && '\\' == line.text[ end-1 ] )
            {
                // an escaped line break joins the lines without a space (when the backslash itself is not escaped)
                UInt32 backslashes = 0;
                while ( backslashes < end - start && '\\' == line.text[ end-1-backslashes ] )
                    ++backslashes;
                if ( 1 == backslashes % 2 )
                {
                    escapedBreak = true;
                    --end;
                }
            }
            if ( !escapedBreak )
            {
                while ( end > start && IsSpace( line.text[ end-1 ] ) )
                    --end;
            }
            raw.insert( raw.end(), line.text + start, line.text + end );
            ++m_i;
            first = false;

            // blank lines are line breaks, otherwise the break is a space
            UInt32 blanks = 0;
            while ( m_i < m_end && 0 == m_lines[ m_i ].length )
            {
                ++blanks;
                ++m_i;
            }
            if ( escapedBreak )
                continue;
            if ( 0 == blanks )
            {
                raw.push_back( ' ' );
            }
            else
            {
                for ( UInt32 b=0; b<blanks; ++b )
                {
                    if ( '"' == quote ) { raw.push_back( '\\' ); raw.push_back( 'n' ); }
                    else raw.push_back( '\n' );
                }
            }
        }
        return Fail( "a quoted scalar is not closed" );
    }

    /** A literal (|) or folded (>) block scalar: the more indented lines that follow */
    bool ParseBlockScalar( UInt32 pos, UInt32 indent, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        const SLine& header = m_lines[ m_i ];
        const bool literal = '|' == header.text[ pos ];
        char chomping = 'c';
        for ( UInt32 i=pos+1; i<header.length; ++i )
        {
            if ( '-' == header.text[ i ] ) chomping = '-';
            else if ( '+' == header.text[ i ] ) chomping = '+';
        }
        ++m_i;
        GUCEF::vector< CORE::CString > lines;
        UInt32 blockIndent = 0;
        while ( m_i < m_end )
        {
            const SLine& line = m_lines[ m_i ];
            if ( 0 != line.length && line.indent <= indent )
                break;
            if ( 0 != line.length && 0 == blockIndent )
                blockIndent = line.indent;
            if ( 0 == line.length )
            {
                lines.push_back( CORE::CString::Empty );
            }
            else
            {
                // the indentation beyond that of the block is content
                const UInt32 extra = line.indent > blockIndent ? line.indent - blockIndent : 0;
                CORE::CString text;
                for ( UInt32 s=0; s<extra; ++s )
                    text += " ";
                text += MakeString( line.text, line.length );
                lines.push_back( text );
            }
            ++m_i;
        }
        // trailing empty lines go with the chomping
        UInt32 trailing = 0;
        while ( !lines.empty() && lines.back().IsNULLOrEmpty() )
        {
            lines.pop_back();
            ++trailing;
        }
        CORE::CString value;
        for ( size_t l=0; l<lines.size(); ++l )
        {
            if ( l > 0 )
                value += ( literal || lines[ l ].IsNULLOrEmpty() || lines[ l-1 ].IsNULLOrEmpty() ) ? "\n" : " ";
            value += lines[ l ];
        }
        if ( '-' != chomping && !lines.empty() )
            value += "\n";
        if ( '+' == chomping )
            for ( UInt32 t=0; t<trailing; ++t )
                value += "\n";
        SetScalar( parent, key, inSequence, value );
        return true;
    }

    /** A flow collection: its text is gathered over the lines until the brackets close, then read */
    bool ParseFlowLines( UInt32 pos, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        TCharVector text;
        Int32 depth = 0;
        char quote = 0;
        bool first = true;
        while ( m_i < m_end )
        {
            const SLine& line = m_lines[ m_i ];
            UInt32 i = first ? pos : 0;
            if ( !first )
            {
                text.push_back( ' ' );
                while ( i < line.length && IsSpace( line.text[ i ] ) )
                    ++i;
            }
            for ( ; i<line.length; ++i )
            {
                const char c = line.text[ i ];
                text.push_back( c );
                if ( 0 != quote )
                {
                    if ( '"' == quote && '\\' == c && i+1 < line.length )
                        text.push_back( line.text[ ++i ] );
                    else
                    if ( c == quote )
                        quote = 0;
                    continue;
                }
                if ( '"' == c || '\'' == c )
                    quote = c;
                else
                if ( '{' == c || '[' == c )
                    ++depth;
                else
                if ( '}' == c || ']' == c )
                {
                    --depth;
                    if ( 0 == depth )
                        break;
                }
            }
            ++m_i;
            first = false;
            if ( 0 == depth )
                break;
        }
        if ( 0 != depth )
            return Fail( "a flow collection is not closed" );
        const char* p = &text[ 0 ];
        const char* end = p + text.size();
        if ( !ParseFlowValue( p, end, parent, key, inSequence ) )
            return Fail( "a flow collection cannot be read" );
        return true;
    }

    static void SkipFlowSpace( const char*& p, const char* end )
    {
        while ( p < end && ( IsSpace( *p ) || '\n' == *p || '\r' == *p ) )
            ++p;
    }

    /** A plain scalar in a flow collection: up to a ',', a closing bracket, or (for a key) a ':' */
    static CORE::CString ReadFlowPlain( const char*& p, const char* end, bool isKey )
    {
        const char* start = p;
        while ( p < end )
        {
            const char c = *p;
            if ( ',' == c || '}' == c || ']' == c )
                break;
            if ( isKey && ':' == c && ( p+1 == end || IsSpace( p[ 1 ] ) || ',' == p[ 1 ] || '}' == p[ 1 ] ) )
                break;
            ++p;
        }
        const char* stop = p;
        while ( stop > start && IsSpace( stop[ -1 ] ) )
            --stop;
        return MakeString( start, (UInt32) ( stop - start ) );
    }

    static bool ReadFlowQuoted( const char*& p, const char* end, CORE::CString& value )
    {
        const char quote = *p;
        const char* start = ++p;
        while ( p < end )
        {
            if ( '"' == quote && '\\' == *p && p+1 < end )
            {
                p += 2;
                continue;
            }
            if ( *p == quote )
            {
                if ( '\'' == quote && p+1 < end && '\'' == p[ 1 ] )
                {
                    p += 2;
                    continue;
                }
                break;
            }
            ++p;
        }
        if ( p >= end )
            return false;
        TCharVector chars;
        ReadQuoted( start, (UInt32) ( p - start ), quote, chars );
        value = MakeString( chars );
        ++p;
        return true;
    }

    static bool ParseFlowValue( const char*& p, const char* end, CORE::CDataNode& parent, const CORE::CString& key, bool inSequence )
    {
        SkipFlowSpace( p, end );
        if ( p >= end )
            return false;
        if ( '{' == *p )
        {
            ++p;
            CORE::CDataNode* node = parent.AddChild( inSequence ? CORE::CString::Empty : key, GUCEF_DATATYPE_OBJECT );
            while ( true )
            {
                SkipFlowSpace( p, end );
                if ( p >= end )
                    return false;
                if ( '}' == *p )
                {
                    ++p;
                    return true;
                }
                CORE::CString memberKey;
                if ( '"' == *p || '\'' == *p )
                {
                    if ( !ReadFlowQuoted( p, end, memberKey ) )
                        return false;
                }
                else
                {
                    memberKey = ReadFlowPlain( p, end, true );
                }
                SkipFlowSpace( p, end );
                if ( p < end && ':' == *p )
                {
                    ++p;
                    SkipFlowSpace( p, end );
                    if ( p < end && ( ',' == *p || '}' == *p ) )
                        node->SetAttribute( memberKey, CORE::CString::Empty );
                    else
                    if ( !ParseFlowValue( p, end, *node, memberKey, false ) )
                        return false;
                }
                else
                {
                    node->SetAttribute( memberKey, CORE::CString::Empty );
                }
                SkipFlowSpace( p, end );
                if ( p < end && ',' == *p )
                    ++p;
                else
                if ( p >= end || '}' != *p )
                    return false;
            }
        }
        if ( '[' == *p )
        {
            ++p;
            CORE::CDataNode* node = parent.AddChild( inSequence ? CORE::CString::Empty : key, GUCEF_DATATYPE_ARRAY );
            while ( true )
            {
                SkipFlowSpace( p, end );
                if ( p >= end )
                    return false;
                if ( ']' == *p )
                {
                    ++p;
                    return true;
                }
                if ( !ParseFlowValue( p, end, *node, CORE::CString::Empty, true ) )
                    return false;
                SkipFlowSpace( p, end );
                if ( p < end && ',' == *p )
                    ++p;
                else
                if ( p >= end || ']' != *p )
                    return false;
            }
        }
        CORE::CString value;
        if ( '"' == *p || '\'' == *p )
        {
            if ( !ReadFlowQuoted( p, end, value ) )
                return false;
        }
        else
        {
            value = ReadFlowPlain( p, end, false );
        }
        SetScalar( parent, key, inSequence, value );
        return true;
    }

    private:

    TLineVector& m_lines;
    UInt32 m_i;
    UInt32 m_end;
    CORE::CString m_problem;
};

/*-------------------------------------------------------------------------*/

/** Whether the line is a document marker ("---" or "...") at the start of the line */
bool
IsDocumentLine( const SLine& line, const char* marker )
{
    return 0 == line.indent && line.length >= 3 && 0 == strncmp( line.text, marker, 3 ) && ( 3 == line.length || IsSpace( line.text[ 3 ] ) );
}

/*-------------------------------------------------------------------------*/

/** The header of a document after "---": the class ID, the file ID and "stripped" */
void
ReadHeader( const char* text, UInt32 length, Int32& classId, Int64& fileId, bool& stripped )
{
    classId = -1;
    fileId = 0;
    stripped = false;
    UInt32 i = 3;
    while ( i < length )
    {
        while ( i < length && IsSpace( text[ i ] ) )
            ++i;
        const UInt32 start = i;
        while ( i < length && !IsSpace( text[ i ] ) )
            ++i;
        const CORE::CString token = MakeString( text + start, i - start );
        if ( token.IsNULLOrEmpty() )
            continue;
        if ( 0 == token.HasSubstr( "!u!", true ) )
        {
            Int64 value = 0;
            if ( CUnityYamlReader::ParseInt64( token.CutChars( 3, true ), value ) )
                classId = (Int32) value;
        }
        else
        if ( '&' == token.C_String()[ 0 ] )
        {
            Int64 value = 0;
            if ( CUnityYamlReader::ParseInt64( token.CutChars( 1, true ), value ) )
                fileId = value;
        }
        else
        if ( "stripped" == token )
        {
            stripped = true;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CUnityYamlReader::Parse( const char* text, UInt32 byteSize, CUnityYamlFile& file, CORE::CString& problem, const ASSETCONVERTER::CConversionReport* cancelSource )
{GUCEF_TRACE;

    file.Clear();
    problem.Clear();
    if ( GUCEF_NULL == text )
        return 0 == byteSize;

    // the lines, without their line ends; the indentation counts spaces
    TLineVector lines;
    const char* p = text;
    const char* end = text + byteSize;
    if ( byteSize >= 3 && (UInt8) p[ 0 ] == 0xEF && (UInt8) p[ 1 ] == 0xBB && (UInt8) p[ 2 ] == 0xBF )
        p += 3;
    UInt32 number = 1;
    while ( p < end )
    {
        const char* lineEnd = p;
        while ( lineEnd < end && '\n' != *lineEnd )
            ++lineEnd;
        const char* contentEnd = lineEnd;
        while ( contentEnd > p && ( '\r' == contentEnd[ -1 ] ) )
            --contentEnd;
        SLine line;
        line.indent = 0;
        line.number = number++;
        const char* content = p;
        while ( content < contentEnd && ' ' == *content )
        {
            ++content;
            ++line.indent;
        }
        // a line of only spaces or tabs is blank
        const char* trimmed = contentEnd;
        while ( trimmed > content && IsSpace( trimmed[ -1 ] ) )
            --trimmed;
        line.text = content;
        line.length = trimmed == content ? 0 : (UInt32) ( contentEnd - content );
        lines.push_back( line );
        p = lineEnd < end ? lineEnd + 1 : end;
    }

    // the documents: from a "---" line (or the start) to the next
    bool ok = true;
    UInt32 i = 0;
    const UInt32 count = (UInt32) lines.size();
    UInt32 documentsRead = 0;
    while ( i < count )
    {
        if ( GUCEF_NULL != cancelSource && 0 == ( ++documentsRead & 63 ) && cancelSource->IsCancellationRequested() )
        {
            file.Clear();
            problem = "cancelled";
            return false;
        }
        Int32 classId = -1;
        Int64 fileId = 0;
        bool stripped = false;
        bool hasMarker = false;
        UInt32 first = i;
        while ( first < count && 0 == lines[ first ].indent && lines[ first ].length >= 1 && '%' == lines[ first ].text[ 0 ] )
            ++first;     // directives
        if ( first < count && IsDocumentLine( lines[ first ], "---" ) )
        {
            ReadHeader( lines[ first ].text, lines[ first ].length, classId, fileId, stripped );
            hasMarker = true;
            ++first;
        }
        UInt32 docEnd = first;
        while ( docEnd < count && !IsDocumentLine( lines[ docEnd ], "---" ) && !IsDocumentLine( lines[ docEnd ], "..." ) )
            ++docEnd;

        // a Unity header makes the document an object of a class; a document without content and header is none
        const bool unityHeader = classId >= 0;
        bool hasContent = false;
        for ( UInt32 l=first; l<docEnd && !hasContent; ++l )
            hasContent = 0 != lines[ l ].length && '#' != lines[ l ].text[ 0 ];
        if ( unityHeader || hasContent )
        {
            CUnityYamlDocument& document = file.AddDocument( unityHeader ? classId : -1, fileId, stripped );
            CORE::CDataNode root( GUCEF_DATATYPE_OBJECT );
            CDocumentParser parser( lines, first, docEnd );
            if ( !parser.ParseDocument( root ) )
            {
                ok = false;
                if ( problem.IsNULLOrEmpty() )
                    problem = parser.GetProblem() + ( unityHeader ? " (document &" + CORE::Int64ToString( fileId ) + ")" : CORE::CString::Empty );
            }
            if ( unityHeader && 1 == root.GetNrOfDirectChildNodes() && 0 == root.GetAttCount() )
            {
                // the single key of the document is the class name, its value the object
                const CORE::CDataNode* object = root.GetFirstChild();
                document.typeName = object->GetName();
                document.body = *object;
            }
            else
            if ( unityHeader && 0 == root.GetNrOfDirectChildNodes() && 1 == root.GetAttCount() )
            {
                document.typeName = root.GetAttribute( (UInt32) 0 )->first;
                document.body = CORE::CDataNode( document.typeName, GUCEF_DATATYPE_OBJECT );
            }
            else
            {
                document.body = root;
            }
        }

        i = docEnd;
        if ( i < count && IsDocumentLine( lines[ i ], "..." ) )
            ++i;
        if ( !hasMarker && i == first && i < count && !IsDocumentLine( lines[ i ], "---" ) )
            ++i;    // no progress
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::Parse( const CORE::CString& text, CUnityYamlFile& file, CORE::CString& problem )
{GUCEF_TRACE;

    return Parse( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, file, problem );
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::Load( const CORE::CString& vfsPath, CUnityYamlFile& file, CORE::CString& problem, const ASSETCONVERTER::CConversionReport* cancelSource )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( buffer, vfsPath, "rb" ) )
    {
        file.Clear();
        problem = "the file cannot be read";
        return false;
    }
    return Parse( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize(), file, problem, cancelSource );
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::ParseInt64( const CORE::CString& text, Int64& value )
{GUCEF_TRACE;

    const char* p = text.C_String();
    if ( GUCEF_NULL == p )
        return false;
    while ( IsSpace( *p ) )
        ++p;
    bool negative = false;
    if ( '-' == *p || '+' == *p )
    {
        negative = '-' == *p;
        ++p;
    }
    if ( *p < '0' || *p > '9' )
        return false;
    UInt64 magnitude = 0;
    while ( *p >= '0' && *p <= '9' )
    {
        const UInt64 digit = (UInt64) ( *p - '0' );
        if ( magnitude > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        magnitude = magnitude * 10 + digit;
        ++p;
    }
    while ( IsSpace( *p ) )
        ++p;
    if ( '\0' != *p )
        return false;
    const UInt64 limit = negative ? (UInt64) GUCEF_INT64MAX + 1 : (UInt64) GUCEF_INT64MAX;
    if ( magnitude > limit )
        return false;
    if ( negative )
        value = magnitude == (UInt64) GUCEF_INT64MAX + 1 ? GUCEF_INT64MIN : -(Int64) magnitude;
    else
        value = (Int64) magnitude;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::ParseFloat( const CORE::CString& text, Float64& value )
{GUCEF_TRACE;

    const CORE::CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    const CORE::CString lower = trimmed.Lowercase();
    if ( "infinity" == lower || "+infinity" == lower || ".inf" == lower || "+.inf" == lower || "inf" == lower )
    {
        value = HUGE_VAL;
        return true;
    }
    if ( "-infinity" == lower || "-.inf" == lower || "-inf" == lower )
    {
        value = -HUGE_VAL;
        return true;
    }
    if ( "nan" == lower || ".nan" == lower )
    {
        value = 0.0;
        value = value / value;
        return true;
    }
    const char* start = trimmed.C_String();
    char* stop = GUCEF_NULL;
    const double parsed = strtod( start, &stop );
    if ( stop == start || GUCEF_NULL == stop || '\0' != *stop )
        return false;
    value = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityYamlReader::GetString( const CORE::CDataNode& mapping, const CORE::CString& key, const CORE::CString& defaultValue )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = mapping.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return defaultValue;
    return attribute->second.AsString();
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::HasScalar( const CORE::CDataNode& mapping, const CORE::CString& key )
{GUCEF_TRACE;

    return GUCEF_NULL != mapping.GetAttribute( key );
}

/*-------------------------------------------------------------------------*/

Int64
CUnityYamlReader::GetInt64( const CORE::CDataNode& mapping, const CORE::CString& key, Int64 defaultValue )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = mapping.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return defaultValue;
    Int64 value = 0;
    if ( ParseInt64( attribute->second.AsString(), value ) )
        return value;
    Float64 real = 0.0;
    if ( ParseFloat( attribute->second.AsString(), real ) && real == floor( real ) && fabs( real ) < 9.0e18 )
        return (Int64) real;
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

Int32
CUnityYamlReader::GetInt32( const CORE::CDataNode& mapping, const CORE::CString& key, Int32 defaultValue )
{GUCEF_TRACE;

    const Int64 value = GetInt64( mapping, key, (Int64) defaultValue );
    if ( value > GUCEF_INT32MAX || value < GUCEF_INT32MIN )
        return defaultValue;
    return (Int32) value;
}

/*-------------------------------------------------------------------------*/

Float32
CUnityYamlReader::GetFloat( const CORE::CDataNode& mapping, const CORE::CString& key, Float32 defaultValue )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = mapping.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return defaultValue;
    Float64 value = 0.0;
    if ( !ParseFloat( attribute->second.AsString(), value ) )
        return defaultValue;
    return (Float32) value;
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::GetBool( const CORE::CDataNode& mapping, const CORE::CString& key, bool defaultValue )
{GUCEF_TRACE;

    const CORE::CDataNode::TKeyValuePair* attribute = mapping.GetAttribute( key );
    if ( GUCEF_NULL == attribute )
        return defaultValue;
    const CORE::CString text = attribute->second.AsString().Lowercase();
    if ( "true" == text || "yes" == text )
        return true;
    if ( "false" == text || "no" == text )
        return false;
    Float64 value = 0.0;
    if ( !ParseFloat( text, value ) )
        return defaultValue;
    return 0.0 != value;
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::GetComponents( const CORE::CDataNode* mapping, const char* components, Float32* values )
{GUCEF_TRACE;

    if ( GUCEF_NULL == mapping || GUCEF_NULL == components || GUCEF_NULL == values || GUCEF_DATATYPE_OBJECT != mapping->GetNodeType() )
        return false;
    for ( UInt32 c=0; '\0' != components[ c ]; ++c )
    {
        const char name[ 2 ] = { components[ c ], '\0' };
        values[ c ] = GetFloat( *mapping, name, values[ c ] );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityYamlReader::GetComponents( const CORE::CDataNode& mapping, const CORE::CString& key, const char* components, Float32* values )
{GUCEF_TRACE;

    return GetComponents( mapping.FindChild( key ), components, values );
}

/*-------------------------------------------------------------------------*/

void
CUnityYamlReader::GetItems( const CORE::CDataNode* sequence, TNodeList& items )
{GUCEF_TRACE;

    items.clear();
    if ( GUCEF_NULL == sequence || GUCEF_DATATYPE_ARRAY != sequence->GetNodeType() )
        return;
    CORE::CDataNode::const_iterator i = sequence->ConstBegin();
    while ( i != sequence->ConstEnd() )
    {
        items.push_back( *i );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityYamlReader::GetItemText( const CORE::CDataNode& item )
{GUCEF_TRACE;

    return item.GetValue().AsString();
}

/*-------------------------------------------------------------------------*/

void
CUnityYamlReader::GetKeyedEntries( const CORE::CDataNode* member, CORE::CString::StringMap& scalars, GUCEF::vector< std::pair< CORE::CString, const CORE::CDataNode* > >& nodes )
{GUCEF_TRACE;

    scalars.clear();
    nodes.clear();
    if ( GUCEF_NULL == member )
        return;

    TNodeList entries;
    if ( GUCEF_DATATYPE_ARRAY == member->GetNodeType() )
        GetItems( member, entries );
    else
        entries.push_back( member );

    for ( size_t e=0; e<entries.size(); ++e )
    {
        const CORE::CDataNode& entry = *entries[ e ];
        CORE::CDataNode::TAttributeMap::const_iterator a = entry.AttributeBegin();
        while ( a != entry.AttributeEnd() )
        {
            scalars[ (*a).first ] = (*a).second.AsString();
            ++a;
        }
        CORE::CDataNode::const_iterator c = entry.ConstBegin();
        while ( c != entry.ConstEnd() )
        {
            nodes.push_back( std::pair< CORE::CString, const CORE::CDataNode* >( (*c)->GetName(), *c ) );
            ++c;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
