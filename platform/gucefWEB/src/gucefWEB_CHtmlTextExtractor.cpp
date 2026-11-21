/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CUTF8STRING_H
#include "gucefCORE_CUtf8String.h"
#define GUCEF_CORE_CUTF8STRING_H
#endif /* GUCEF_CORE_CUTF8STRING_H ? */

#include "gucefWEB_CHtmlTextExtractor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< char > TCharVector;

/** The longest name of a tag or a character reference that is looked at; longer ones are not known */
const UInt32 MaxNameLength = 15;
const UInt32 MaxReferenceLength = 32;
const UInt32 ReplacementCharacter = 0xFFFD;

struct SNamedReference
{
    const char* name;
    UInt32 codePoint;       /**< 0: the reference stands for nothing that is shown (a soft hyphen, a joiner) */
};

const SNamedReference NamedReferences[] =
{
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' },
    { "nbsp", ' ' }, { "ensp", ' ' }, { "emsp", ' ' }, { "thinsp", ' ' },
    { "shy", 0 }, { "zwj", 0 }, { "zwnj", 0 }, { "lrm", 0 }, { "rlm", 0 },
    { "ndash", 0x2013 }, { "mdash", 0x2014 }, { "hellip", 0x2026 }, { "bull", 0x2022 }, { "middot", 0xB7 },
    { "lsquo", 0x2018 }, { "rsquo", 0x2019 }, { "sbquo", 0x201A }, { "ldquo", 0x201C }, { "rdquo", 0x201D }, { "bdquo", 0x201E },
    { "laquo", 0xAB }, { "raquo", 0xBB }, { "lsaquo", 0x2039 }, { "rsaquo", 0x203A },
    { "copy", 0xA9 }, { "reg", 0xAE }, { "trade", 0x2122 }, { "sect", 0xA7 }, { "para", 0xB6 }, { "dagger", 0x2020 },
    { "euro", 0x20AC }, { "pound", 0xA3 }, { "yen", 0xA5 }, { "cent", 0xA2 }, { "curren", 0xA4 },
    { "deg", 0xB0 }, { "times", 0xD7 }, { "divide", 0xF7 }, { "plusmn", 0xB1 }, { "micro", 0xB5 },
    { "frac12", 0xBD }, { "frac14", 0xBC }, { "frac34", 0xBE }, { "sup1", 0xB9 }, { "sup2", 0xB2 }, { "sup3", 0xB3 },
    { "iexcl", 0xA1 }, { "iquest", 0xBF }, { "larr", 0x2190 }, { "rarr", 0x2192 }, { "uarr", 0x2191 }, { "darr", 0x2193 },
    { "hearts", 0x2665 }, { "star", 0x2606 }, { "check", 0x2713 },
    { GUCEF_NULL, 0 }
};

bool
IsAsciiLetter( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

bool
IsAsciiDigit( char c )
{
    return c >= '0' && c <= '9';
}

bool
IsHtmlSpace( char c )
{
    return ' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c;
}

char
LowerAscii( char c )
{
    return ( c >= 'A' && c <= 'Z' ) ? (char) ( c - 'A' + 'a' ) : c;
}

void
AppendCodePoint( TCharVector& out, UInt32 codePoint )
{
    char encoded[ 8 ];
    const Int32 size = CORE::CUtf8String::EncodeUtf32CodePointToUtf8( codePoint, encoded, (UInt32) sizeof( encoded ) );
    if ( size > 0 )
        out.insert( out.end(), encoded, encoded + size );
}

/** The code point of a numeric reference ("#169", "#xA9"); false when it is not one */
bool
ParseNumericReference( const char* name, UInt32 length, UInt32& codePoint )
{
    if ( length < 2 || '#' != name[ 0 ] )
        return false;
    const bool hex = 'x' == name[ 1 ] || 'X' == name[ 1 ];
    UInt32 i = hex ? 2 : 1;
    if ( i >= length )
        return false;
    UInt32 value = 0;
    for ( ; i<length; ++i )
    {
        const char c = name[ i ];
        UInt32 digit = 0;
        if ( IsAsciiDigit( c ) )
            digit = (UInt32) ( c - '0' );
        else
        if ( hex && c >= 'a' && c <= 'f' )
            digit = (UInt32) ( c - 'a' + 10 );
        else
        if ( hex && c >= 'A' && c <= 'F' )
            digit = (UInt32) ( c - 'A' + 10 );
        else
            return false;
        value = value * ( hex ? 16 : 10 ) + digit;
        if ( value > 0x10FFFF )
            value = 0x110000;       // stays out of range, without overflow
    }
    if ( 0 == value || value > 0x10FFFF || ( value >= 0xD800 && value <= 0xDFFF ) )
        value = ReplacementCharacter;
    codePoint = value;
    return true;
}

/** The code point of a named reference (case sensitive, as HTML has it); false when it is not known */
bool
FindNamedReference( const char* name, UInt32 length, UInt32& codePoint )
{
    for ( UInt32 i=0; GUCEF_NULL != NamedReferences[ i ].name; ++i )
    {
        if ( length == (UInt32) strlen( NamedReferences[ i ].name ) && 0 == strncmp( NamedReferences[ i ].name, name, length ) )
        {
            codePoint = NamedReferences[ i ].codePoint;
            return true;
        }
    }
    return false;
}

/**
 *  Decodes the character references of text[ start, end ) into 'out'; characters that are not shown (controls other than
 *  white space) are left out
 */
void
DecodeInto( const char* text, UInt32 start, UInt32 end, TCharVector& out )
{
    UInt32 i = start;
    while ( i < end )
    {
        const char c = text[ i ];
        if ( '&' == c )
        {
            UInt32 semicolon = i + 1;
            while ( semicolon < end && semicolon - i <= MaxReferenceLength && ';' != text[ semicolon ] && '&' != text[ semicolon ] && !IsHtmlSpace( text[ semicolon ] ) )
                ++semicolon;
            if ( semicolon < end && ';' == text[ semicolon ] && semicolon > i + 1 )
            {
                UInt32 codePoint = 0;
                const char* name = text + i + 1;
                const UInt32 length = semicolon - i - 1;
                if ( ParseNumericReference( name, length, codePoint ) || FindNamedReference( name, length, codePoint ) )
                {
                    if ( codePoint >= 0x20 && !( codePoint >= 0x7F && codePoint <= 0x9F ) )
                        AppendCodePoint( out, codePoint );
                    else
                    if ( '\t' == codePoint || '\n' == codePoint || '\r' == codePoint )
                        out.push_back( (char) codePoint );
                    i = semicolon + 1;
                    continue;
                }
            }
            out.push_back( c );
            ++i;
            continue;
        }
        if ( (unsigned char) c < 0x20 && !IsHtmlSpace( c ) )
        {
            ++i;
            continue;
        }
        out.push_back( c );
        ++i;
    }
}

/** Writes the text with the white space and the breaks of the blocks: what the extractor writes goes through it */
class CPlainTextWriter
{
    public:

    TCharVector out;
    UInt32 preDepth;

    CPlainTextWriter( void )
        : out(), preDepth( 0 ), m_pendingBreaks( 0 ), m_pendingSpace( false ), m_pendingPrefix(), m_pendingSeparator( false ), m_atLineStart( true )
    {
    }

    /** At least 'breaks' line ends before the next text (1: a new line, 2: an empty line between) */
    void RequestBreak( UInt32 breaks )
    {
        if ( breaks > m_pendingBreaks )
            m_pendingBreaks = breaks;
        m_pendingSpace = false;
        m_pendingSeparator = false;
    }

    /** One more line end before the next text, at most an empty line */
    void AddLineBreak( void )
    {
        RequestBreak( m_pendingBreaks >= 2 ? 2 : m_pendingBreaks + 1 );
    }

    /** What the next line starts with (the marker of an item of a list) */
    void SetLinePrefix( const TCharVector& prefix )
    {
        m_pendingPrefix = prefix;
    }

    /** " | " before the next text when it is on the same line (the cells of a row) */
    void RequestSeparator( void )
    {
        m_pendingSeparator = true;
    }

    /** Text that is decoded already; white space collapses outside "pre" */
    void Write( const TCharVector& text )
    {
        for ( size_t i=0; i<text.size(); ++i )
        {
            const char c = text[ i ];
            if ( preDepth > 0 )
            {
                if ( '\r' == c )
                    continue;
                if ( '\n' == c )
                {
                    Flush();
                    out.push_back( '\n' );
                    m_atLineStart = true;
                    continue;
                }
                Flush();
                out.push_back( c );
                m_atLineStart = false;
                continue;
            }
            if ( IsHtmlSpace( c ) )
            {
                if ( !m_atLineStart || m_pendingBreaks > 0 )
                    m_pendingSpace = true;
                continue;
            }
            Flush();
            out.push_back( c );
            m_atLineStart = false;
        }
    }

    void WriteText( const char* text )
    {
        TCharVector chars( text, text + strlen( text ) );
        Write( chars );
    }

    /** The place in the output where the next text starts: what is pending is written first (not the space) */
    size_t MarkTextStart( void )
    {
        if ( m_pendingBreaks > 0 || !m_pendingPrefix.empty() )
        {
            m_pendingSpace = false;
            Flush();
        }
        return out.size() + ( m_pendingSpace && !m_atLineStart ? 1 : 0 );
    }

    CORE::CString ToString( void ) const
    {
        // no white space at the ends of the lines or of the whole
        TCharVector result;
        result.reserve( out.size() );
        for ( size_t i=0; i<out.size(); ++i )
        {
            if ( '\n' == out[ i ] )
            {
                while ( !result.empty() && ( ' ' == result.back() || '\t' == result.back() ) )
                    result.pop_back();
            }
            result.push_back( out[ i ] );
        }
        while ( !result.empty() && IsHtmlSpace( result.back() ) )
            result.pop_back();
        size_t first = 0;
        while ( first < result.size() && IsHtmlSpace( result[ first ] ) )
            ++first;
        if ( first >= result.size() )
            return CORE::CString::Empty;
        return CORE::CString( &result[ first ], (UInt32) ( result.size() - first ) );
    }

    private:

    void Flush( void )
    {
        if ( m_pendingBreaks > 0 )
        {
            if ( !out.empty() )
            {
                // the breaks that are there already count
                UInt32 have = 0;
                for ( size_t i=out.size(); i>0 && '\n' == out[ i-1 ]; --i )
                    ++have;
                for ( UInt32 b=have; b<m_pendingBreaks; ++b )
                    out.push_back( '\n' );
            }
            m_pendingBreaks = 0;
            m_atLineStart = true;
            m_pendingSpace = false;
            m_pendingSeparator = false;
        }
        if ( !m_pendingPrefix.empty() )
        {
            if ( !m_atLineStart )
                out.push_back( '\n' );
            out.insert( out.end(), m_pendingPrefix.begin(), m_pendingPrefix.end() );
            m_pendingPrefix.clear();
            m_atLineStart = false;
            m_pendingSpace = false;
            m_pendingSeparator = false;
        }
        if ( m_pendingSeparator )
        {
            if ( !m_atLineStart )
            {
                const char separator[] = " | ";
                out.insert( out.end(), separator, separator + 3 );
            }
            m_pendingSeparator = false;
            m_pendingSpace = false;
        }
        if ( m_pendingSpace )
        {
            if ( !m_atLineStart )
                out.push_back( ' ' );
            m_pendingSpace = false;
        }
    }

    UInt32 m_pendingBreaks;
    bool m_pendingSpace;
    TCharVector m_pendingPrefix;
    bool m_pendingSeparator;
    bool m_atLineStart;
};

/** The tags whose content is no text: left out with what is in them */
bool
IsSkippedElement( const char* name )
{
    static const char* const skipped[] = { "script", "style", "template", "noscript", "iframe", "object", "svg", "head", "math", "canvas", "audio", "video", "select", "textarea", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != skipped[ i ]; ++i )
        if ( 0 == strcmp( skipped[ i ], name ) )
            return true;
    return false;
}

/** The tags that are blocks of their own: an empty line before and after them */
bool
IsParagraphElement( const char* name )
{
    static const char* const blocks[] = { "p", "div", "h1", "h2", "h3", "h4", "h5", "h6", "ul", "ol", "dl", "table", "blockquote", "section", "article",
                                          "header", "footer", "nav", "aside", "main", "figure", "figcaption", "form", "fieldset", "address", "hr", "details",
                                          "summary", "body", "html", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != blocks[ i ]; ++i )
        if ( 0 == strcmp( blocks[ i ], name ) )
            return true;
    return false;
}

/** The tags that start a line of their own */
bool
IsLineElement( const char* name )
{
    static const char* const lines[] = { "tr", "dt", "dd", "caption", "thead", "tbody", "tfoot", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != lines[ i ]; ++i )
        if ( 0 == strcmp( lines[ i ], name ) )
            return true;
    return false;
}

/**
 *  The address of a link: the reference as a CORE::CUri (resolved against the base when there is one) and rendered again.
 *  False when it is no valid URI (a relative reference without a base, a reference CUri cannot parse): the link then keeps
 *  its text only. The scheme is not judged here: whether a link may be opened is up to the code that would open it
 */
bool
MakeLinkAddress( const CORE::CString& reference, const CORE::CUri* base, CORE::CString& address )
{
    address.Clear();
    if ( reference.IsNULLOrEmpty() )
        return false;
    CORE::CUri uri;
    const bool valid = GUCEF_NULL != base ? base->ResolveReference( reference, uri ) : uri.ParseUriInStringForm( reference );
    if ( !valid )
        return false;
    address = uri.ToUriInStringForm();
    return !address.IsNULLOrEmpty();
}

/** The text of a part of the input, as a string */
CORE::CString
MakeString( const TCharVector& chars, size_t start )
{
    if ( start >= chars.size() )
        return CORE::CString::Empty;
    return CORE::CString( &chars[ start ], (UInt32) ( chars.size() - start ) );
}

/**
 *  Reads the tag that starts at text[ start ] ('<'): its lower case name (empty when it is too long), whether it closes,
 *  whether it closes itself, the value of the attribute 'href', and where the text after it starts. False when there is
 *  no '>' that ends it
 */
bool
ReadTag( const char* text, UInt32 start, UInt32 end, char* name, bool& closing, bool& selfClosing, CORE::CString& href, UInt32& next )
{
    UInt32 i = start + 1;
    closing = false;
    selfClosing = false;
    href.Clear();
    if ( i < end && '/' == text[ i ] )
    {
        closing = true;
        ++i;
    }
    UInt32 length = 0;
    bool tooLong = false;
    while ( i < end && ( IsAsciiLetter( text[ i ] ) || IsAsciiDigit( text[ i ] ) || '-' == text[ i ] || ':' == text[ i ] ) )
    {
        if ( length < MaxNameLength )
            name[ length++ ] = LowerAscii( text[ i ] );
        else
            tooLong = true;
        ++i;
    }
    name[ tooLong ? 0 : length ] = '\0';

    // the attributes: name, name=value, name="value", name='value'
    while ( i < end )
    {
        const char c = text[ i ];
        if ( '>' == c )
        {
            next = i + 1;
            return true;
        }
        if ( '/' == c && i + 1 < end && '>' == text[ i + 1 ] )
        {
            selfClosing = true;
            next = i + 2;
            return true;
        }
        if ( IsHtmlSpace( c ) || '/' == c )
        {
            ++i;
            continue;
        }
        const UInt32 nameStart = i;
        while ( i < end && !IsHtmlSpace( text[ i ] ) && '=' != text[ i ] && '>' != text[ i ] && '/' != text[ i ] )
            ++i;
        const UInt32 nameEnd = i;
        while ( i < end && IsHtmlSpace( text[ i ] ) )
            ++i;
        UInt32 valueStart = i;
        UInt32 valueEnd = i;
        bool hasValue = false;
        if ( i < end && '=' == text[ i ] )
        {
            ++i;
            while ( i < end && IsHtmlSpace( text[ i ] ) )
                ++i;
            if ( i < end && ( '"' == text[ i ] || '\'' == text[ i ] ) )
            {
                const char quote = text[ i ];
                valueStart = ++i;
                while ( i < end && quote != text[ i ] )
                    ++i;
                if ( i >= end )
                    return false;
                valueEnd = i;
                ++i;
            }
            else
            {
                valueStart = i;
                while ( i < end && !IsHtmlSpace( text[ i ] ) && '>' != text[ i ] )
                    ++i;
                valueEnd = i;
            }
            hasValue = true;
        }
        if ( hasValue && 4 == nameEnd - nameStart && 'h' == LowerAscii( text[ nameStart ] ) && 'r' == LowerAscii( text[ nameStart + 1 ] ) &&
             'e' == LowerAscii( text[ nameStart + 2 ] ) && 'f' == LowerAscii( text[ nameStart + 3 ] ) )
        {
            TCharVector value;
            DecodeInto( text, valueStart, valueEnd, value );
            href = MakeString( value, 0 ).Trim( true ).Trim( false );
        }
        if ( i == nameStart )
            ++i;
    }
    return false;
}

/** Where the closing tag of the element starts after 'start' ("</name", any case), 'end' when there is none */
UInt32
FindClosingTag( const char* text, UInt32 start, UInt32 end, const char* name )
{
    const UInt32 nameLength = (UInt32) strlen( name );
    for ( UInt32 i=start; i + 1 + nameLength < end; ++i )
    {
        if ( '<' != text[ i ] || '/' != text[ i + 1 ] )
            continue;
        bool same = true;
        for ( UInt32 n=0; n<nameLength && same; ++n )
            same = LowerAscii( text[ i + 2 + n ] ) == name[ n ];
        if ( !same )
            continue;
        const UInt32 after = i + 2 + nameLength;
        if ( after >= end || IsHtmlSpace( text[ after ] ) || '>' == text[ after ] || '/' == text[ after ] )
            return i;
    }
    return end;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHtmlTextExtractor::SLink::SLink( void )
    : text()
    , address()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CHtmlTextExtractor::ToPlainText( const CORE::CString& html, TLinkVector* links, bool inlineAddresses, const CORE::CUri* baseAddress )
{GUCEF_TRACE;

    if ( GUCEF_NULL != links )
        links->clear();
    const char* text = html.C_String();
    if ( GUCEF_NULL == text )
        return CORE::CString::Empty;
    const UInt32 end = (UInt32) strlen( text );

    CPlainTextWriter writer;
    writer.out.reserve( end );
    UInt32 listDepth = 0;
    UInt32 cellsInRow = 0;
    bool inLink = false;
    size_t linkStart = 0;
    CORE::CString linkReference;
    CORE::CString linkAddress;
    TCharVector decoded;
    char name[ MaxNameLength + 1 ];

    UInt32 i = 0;
    while ( i < end )
    {
        // the text up to the next tag
        UInt32 textEnd = i;
        while ( textEnd < end && '<' != text[ textEnd ] )
            ++textEnd;
        if ( textEnd > i )
        {
            decoded.clear();
            DecodeInto( text, i, textEnd, decoded );
            writer.Write( decoded );
            i = textEnd;
            continue;
        }

        // a comment, a declaration, a processing instruction, CDATA
        if ( i + 3 < end && 0 == strncmp( text + i, "<!--", 4 ) )
        {
            const char* close = strstr( text + i + 4, "-->" );
            i = GUCEF_NULL == close ? end : (UInt32) ( close - text ) + 3;
            continue;
        }
        if ( i + 8 < end && 0 == strncmp( text + i, "<![CDATA[", 9 ) )
        {
            const char* close = strstr( text + i + 9, "]]>" );
            const UInt32 contentEnd = GUCEF_NULL == close ? end : (UInt32) ( close - text );
            TCharVector content( text + i + 9, text + contentEnd );
            writer.Write( content );
            i = GUCEF_NULL == close ? end : contentEnd + 3;
            continue;
        }
        const char after = i + 1 < end ? text[ i + 1 ] : '\0';
        if ( '!' == after || '?' == after )
        {
            const char* close = strchr( text + i + 2, '>' );
            i = GUCEF_NULL == close ? end : (UInt32) ( close - text ) + 1;
            continue;
        }

        // a "<" that starts no tag is text
        if ( !IsAsciiLetter( after ) && !( '/' == after && i + 2 < end && IsAsciiLetter( text[ i + 2 ] ) ) )
        {
            TCharVector lessThan( 1, '<' );
            writer.Write( lessThan );
            ++i;
            continue;
        }

        bool closing = false;
        bool selfClosing = false;
        CORE::CString href;
        UInt32 next = end;
        if ( !ReadTag( text, i, end, name, closing, selfClosing, href, next ) )
            break;      // a tag that is not closed ends the input
        i = next;

        if ( !closing && !selfClosing && IsSkippedElement( name ) )
        {
            const UInt32 close = FindClosingTag( text, i, end, name );
            if ( close >= end )
                break;
            const char* tagEnd = strchr( text + close, '>' );
            i = GUCEF_NULL == tagEnd ? end : (UInt32) ( tagEnd - text ) + 1;
            continue;
        }

        if ( 0 == strcmp( name, "a" ) )
        {
            // a link: its text is kept, its address is written after it and listed
            if ( inLink || closing )
            {
                if ( inLink )
                {
                    const CORE::CString linkText = MakeString( writer.out, linkStart ).Trim( true ).Trim( false );
                    if ( MakeLinkAddress( linkReference, baseAddress, linkAddress ) )
                    {
                        if ( inlineAddresses && linkText != linkAddress )
                        {
                            CORE::CString suffix = linkText.IsNULLOrEmpty() ? linkAddress : " (" + linkAddress + ")";
                            writer.WriteText( suffix.C_String() );
                        }
                        if ( GUCEF_NULL != links )
                        {
                            SLink link;
                            link.text = linkText.IsNULLOrEmpty() ? linkAddress : linkText;
                            link.address = linkAddress;
                            links->push_back( link );
                        }
                    }
                }
                inLink = false;
                linkReference.Clear();
            }
            if ( !closing && !selfClosing )
            {
                inLink = true;
                linkReference = href;
                linkStart = writer.MarkTextStart();
            }
            continue;
        }

        if ( 0 == strcmp( name, "pre" ) )
        {
            writer.RequestBreak( 2 );
            if ( closing )
            {
                if ( writer.preDepth > 0 )
                    --writer.preDepth;
            }
            else
            if ( !selfClosing )
                ++writer.preDepth;
            continue;
        }

        if ( 0 == strcmp( name, "ul" ) || 0 == strcmp( name, "ol" ) )
        {
            // a list of its own is a block, a list in an item of a list starts on the next line
            if ( closing )
            {
                if ( listDepth > 0 )
                    --listDepth;
                writer.RequestBreak( listDepth > 0 ? 1 : 2 );
            }
            else
            {
                writer.RequestBreak( listDepth > 0 ? 1 : 2 );
                if ( !selfClosing )
                    ++listDepth;
            }
            continue;
        }

        if ( 0 == strcmp( name, "li" ) )
        {
            writer.RequestBreak( 1 );
            if ( !closing )
            {
                TCharVector prefix;
                for ( UInt32 level=1; level<listDepth; ++level )
                {
                    prefix.push_back( ' ' );
                    prefix.push_back( ' ' );
                }
                prefix.push_back( '-' );
                prefix.push_back( ' ' );
                writer.SetLinePrefix( prefix );
            }
            continue;
        }

        if ( 0 == strcmp( name, "td" ) || 0 == strcmp( name, "th" ) )
        {
            if ( !closing )
            {
                if ( cellsInRow > 0 )
                    writer.RequestSeparator();
                ++cellsInRow;
            }
            continue;
        }

        if ( 0 == strcmp( name, "tr" ) )
            cellsInRow = 0;

        if ( IsParagraphElement( name ) )
        {
            writer.RequestBreak( 2 );
            continue;
        }
        if ( 0 == strcmp( name, "br" ) )
        {
            // each line break counts: two of them leave an empty line
            writer.AddLineBreak();
            continue;
        }
        if ( IsLineElement( name ) )
        {
            writer.RequestBreak( 1 );
            continue;
        }
        // any other tag (span, b, i, u, em, strong, img, font, ...) is dropped and its text kept
    }

    if ( inLink && MakeLinkAddress( linkReference, baseAddress, linkAddress ) )
    {
        const CORE::CString linkText = MakeString( writer.out, linkStart ).Trim( true ).Trim( false );
        if ( inlineAddresses && linkText != linkAddress )
            writer.WriteText( ( linkText.IsNULLOrEmpty() ? linkAddress : " (" + linkAddress + ")" ).C_String() );
        if ( GUCEF_NULL != links )
        {
            SLink link;
            link.text = linkText.IsNULLOrEmpty() ? linkAddress : linkText;
            link.address = linkAddress;
            links->push_back( link );
        }
    }
    return writer.ToString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHtmlTextExtractor::ToPlainTextIfHtml( const CORE::CString& text, TLinkVector* links, bool inlineAddresses, const CORE::CUri* baseAddress )
{GUCEF_TRACE;

    if ( !LooksLikeHtml( text ) )
    {
        if ( GUCEF_NULL != links )
            links->clear();
        return text;
    }
    return ToPlainText( text, links, inlineAddresses, baseAddress );
}

/*-------------------------------------------------------------------------*/

bool
CHtmlTextExtractor::LooksLikeHtml( const CORE::CString& text )
{GUCEF_TRACE;

    const char* chars = text.C_String();
    if ( GUCEF_NULL == chars )
        return false;
    for ( UInt32 i=0; '\0' != chars[ i ]; ++i )
    {
        if ( '<' == chars[ i ] )
        {
            const char after = chars[ i + 1 ];
            const bool opens = IsAsciiLetter( after ) || ( '/' == after && IsAsciiLetter( chars[ i + 2 ] ) ) || ( '!' == after && '-' == chars[ i + 2 ] );
            if ( opens && GUCEF_NULL != strchr( chars + i + 1, '>' ) )
            {
                // a tag name followed by the end of the tag, white space or an attribute
                UInt32 n = i + 1 + ( '/' == after ? 1 : 0 );
                while ( IsAsciiLetter( chars[ n ] ) || IsAsciiDigit( chars[ n ] ) )
                    ++n;
                if ( '!' == after || '>' == chars[ n ] || '/' == chars[ n ] || IsHtmlSpace( chars[ n ] ) )
                    return true;
            }
        }
        else
        if ( '&' == chars[ i ] )
        {
            UInt32 n = i + 1;
            while ( '\0' != chars[ n ] && n - i <= MaxReferenceLength && ';' != chars[ n ] && !IsHtmlSpace( chars[ n ] ) && '&' != chars[ n ] )
                ++n;
            UInt32 codePoint = 0;
            if ( ';' == chars[ n ] && n > i + 1 && ( ParseNumericReference( chars + i + 1, n - i - 1, codePoint ) || FindNamedReference( chars + i + 1, n - i - 1, codePoint ) ) )
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHtmlTextExtractor::DecodeCharacterReferences( const CORE::CString& text )
{GUCEF_TRACE;

    const char* chars = text.C_String();
    if ( GUCEF_NULL == chars )
        return CORE::CString::Empty;
    TCharVector decoded;
    DecodeInto( chars, 0, (UInt32) strlen( chars ), decoded );
    return MakeString( decoded, 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
