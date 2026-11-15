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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#include "gucefGUI_CIHelpSource.h"
#define GUCEF_GUI_CIHELPSOURCE_H
#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */

#include "gucefGUI_CHelpMarkupParser.h"

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

typedef GUCEF::vector< char > TBytes;

/** A line of the UTF-8 text: where it starts and how many bytes it has, without its line end */
struct SLine
{
    const char* p;
    UInt32 n;
};
typedef GUCEF::vector< SLine > TLineVector;

/** The text as UTF-8 bytes, kept alive by 'holder' */
const char*
Utf8Bytes( const CString& text, CORE::CUtf8String& holder, UInt32& byteCount )
{
    holder = CORE::ToUtf8String( text );
    const char* bytes = holder.C_String();
    byteCount = GUCEF_NULL != bytes ? (UInt32) strlen( bytes ) : 0;
    if ( GUCEF_NULL == bytes )
        bytes = "";
    // a byte order mark at the start is not text
    if ( byteCount >= 3 && (unsigned char) bytes[ 0 ] == 0xEF && (unsigned char) bytes[ 1 ] == 0xBB && (unsigned char) bytes[ 2 ] == 0xBF )
    {
        bytes += 3;
        byteCount -= 3;
    }
    return bytes;
}

CString
FromBytes( const char* bytes, UInt32 byteCount )
{
    CORE::CUtf8String utf8;
    if ( GUCEF_NULL != bytes && byteCount > 0 )
        utf8.Set( bytes, byteCount );
    return CORE::ToString( utf8 );
}

CString
FromBytes( const TBytes& bytes )
{
    if ( bytes.empty() )
        return CString();
    return FromBytes( &bytes[ 0 ], (UInt32) bytes.size() );
}

void
AppendBytes( TBytes& out, const char* bytes, UInt32 byteCount )
{
    if ( byteCount > 0 )
        out.insert( out.end(), bytes, bytes + byteCount );
}

void
AppendText( TBytes& out, const CString& text )
{
    CORE::CUtf8String holder;
    UInt32 byteCount = 0;
    const char* bytes = Utf8Bytes( text, holder, byteCount );
    AppendBytes( out, bytes, byteCount );
}

bool
IsBlank( char c )
{
    return ' ' == c || '\t' == c;
}

/** The range without the blanks around it */
void
Trim( const char*& p, UInt32& n )
{
    while ( n > 0 && IsBlank( p[ 0 ] ) )
    {
        ++p;
        --n;
    }
    while ( n > 0 && IsBlank( p[ n - 1 ] ) )
        --n;
}

CString
TrimmedString( const char* p, UInt32 n )
{
    Trim( p, n );
    return FromBytes( p, n );
}

bool
StartsWithBytes( const char* p, UInt32 n, const char* prefix )
{
    const UInt32 length = (UInt32) strlen( prefix );
    return n >= length && 0 == memcmp( p, prefix, length );
}

/** Where 'what' occurs in the range from 'from' on, -1 when it does not */
Int32
FindBytes( const char* p, UInt32 n, UInt32 from, const char* what )
{
    const UInt32 length = (UInt32) strlen( what );
    if ( 0 == length || n < length )
        return -1;
    for ( UInt32 i=from; i+length<=n; ++i )
        if ( 0 == memcmp( p + i, what, length ) )
            return (Int32) i;
    return -1;
}

void
SplitLines( const char* p, UInt32 n, TLineVector& lines )
{
    UInt32 start = 0;
    for ( UInt32 i=0; i<=n; ++i )
    {
        if ( i == n || '\n' == p[ i ] )
        {
            SLine line;
            line.p = p + start;
            line.n = i - start;
            if ( line.n > 0 && '\r' == line.p[ line.n - 1 ] )
                --line.n;
            if ( i < n || line.n > 0 )
                lines.push_back( line );
            start = i + 1;
        }
    }
}

/** The block being gathered from several lines */
enum EPending
{
    PENDING_NONE = 0    ,
    PENDING_PARAGRAPH   ,
    PENDING_BULLET      ,
    PENDING_NOTE
};

void
Flush( EPending& pending, TBytes& text, CHelpDocument::TBlockVector& blocks )
{
    if ( PENDING_NONE != pending && !text.empty() )
    {
        CHelpDocument::EBlockKind kind = CHelpDocument::BLOCK_PARAGRAPH;
        if ( PENDING_BULLET == pending )
            kind = CHelpDocument::BLOCK_BULLET;
        else
        if ( PENDING_NOTE == pending )
            kind = CHelpDocument::BLOCK_NOTE;
        blocks.push_back( CHelpDocument::TBlock( kind, FromBytes( text ) ) );
    }
    pending = PENDING_NONE;
    text.clear();
}

void
Continue( TBytes& text, const char* p, UInt32 n )
{
    if ( !text.empty() && n > 0 )
        text.push_back( ' ' );
    AppendBytes( text, p, n );
}

void
ParseBlockLines( const TLineVector& lines, UInt32 first, CHelpDocument::TBlockVector& blocks )
{
    EPending pending = PENDING_NONE;
    TBytes text;
    for ( UInt32 l=first; l<(UInt32)lines.size(); ++l )
    {
        const char* p = lines[ l ].p;
        UInt32 n = lines[ l ].n;
        const bool indented = n > 0 && IsBlank( p[ 0 ] );
        Trim( p, n );
        if ( 0 == n )
        {
            Flush( pending, text, blocks );
            continue;
        }

        // a heading: one to three '#', a blank, the text
        if ( '#' == p[ 0 ] && !indented )
        {
            UInt32 hashes = 0;
            while ( hashes < n && '#' == p[ hashes ] )
                ++hashes;
            if ( hashes <= 3 && ( hashes == n || IsBlank( p[ hashes ] ) ) )
            {
                Flush( pending, text, blocks );
                blocks.push_back( CHelpDocument::TBlock( CHelpDocument::BLOCK_HEADING, TrimmedString( p + hashes, n - hashes ), hashes ) );
                continue;
            }
        }

        // a bullet
        if ( n >= 2 && ( '-' == p[ 0 ] || '*' == p[ 0 ] ) && IsBlank( p[ 1 ] ) )
        {
            Flush( pending, text, blocks );
            pending = PENDING_BULLET;
            const char* rest = p + 2;
            UInt32 restCount = n - 2;
            Trim( rest, restCount );
            AppendBytes( text, rest, restCount );
            continue;
        }

        // a note, and the lines of a note that follow
        if ( '>' == p[ 0 ] && ( 1 == n || IsBlank( p[ 1 ] ) ) )
        {
            if ( PENDING_NOTE != pending )
            {
                Flush( pending, text, blocks );
                pending = PENDING_NOTE;
            }
            const char* rest = p + 1;
            UInt32 restCount = n - 1;
            Trim( rest, restCount );
            Continue( text, rest, restCount );
            continue;
        }

        // a row of a table
        if ( n >= 2 && '|' == p[ 0 ] && '|' == p[ n - 1 ] )
        {
            Flush( pending, text, blocks );
            CHelpDocument::TBlock row( CHelpDocument::BLOCK_TABLE_ROW, CString::Empty );
            UInt32 start = 1;
            bool inLink = false;    // the '|' of a link with a label is not a cell border
            for ( UInt32 i=1; i<n; ++i )
            {
                if ( i + 1 < n && '[' == p[ i ] && '[' == p[ i + 1 ] )
                    inLink = true;
                else
                if ( inLink && i + 1 < n && ']' == p[ i ] && ']' == p[ i + 1 ] )
                    inLink = false;
                else
                if ( '|' == p[ i ] && !inLink )
                {
                    row.cells.push_back( TrimmedString( p + start, i - start ) );
                    start = i + 1;
                }
            }
            blocks.push_back( row );
            continue;
        }

        // a definition
        const Int32 separator = FindBytes( p, n, 0, " :: " );
        if ( separator > 0 && !( PENDING_BULLET == pending && indented ) )
        {
            Flush( pending, text, blocks );
            CHelpDocument::TBlock definition( CHelpDocument::BLOCK_DEFINITION, TrimmedString( p + separator + 4, n - (UInt32) separator - 4 ) );
            definition.term = TrimmedString( p, (UInt32) separator );
            blocks.push_back( definition );
            continue;
        }

        // text: it continues the paragraph or the note, or a bullet when it is indented; else it starts a paragraph
        if ( PENDING_PARAGRAPH == pending || PENDING_NOTE == pending || ( PENDING_BULLET == pending && indented ) )
        {
            Continue( text, p, n );
            continue;
        }
        Flush( pending, text, blocks );
        pending = PENDING_PARAGRAPH;
        AppendBytes( text, p, n );
    }
    Flush( pending, text, blocks );
}

void
ParseInlineBytes( const char* p, UInt32 n, CHelpMarkupParser::TSpanVector& spans )
{
    TBytes plain;
    UInt32 i = 0;
    while ( i < n )
    {
        if ( i + 1 < n && '*' == p[ i ] && '*' == p[ i + 1 ] )
        {
            const Int32 close = FindBytes( p, n, i + 2, "**" );
            if ( close > (Int32) ( i + 2 ) )
            {
                if ( !plain.empty() )
                {
                    spans.push_back( CHelpMarkupParser::TSpan( CHelpMarkupParser::SPAN_TEXT, FromBytes( plain ) ) );
                    plain.clear();
                }
                spans.push_back( CHelpMarkupParser::TSpan( CHelpMarkupParser::SPAN_BOLD, FromBytes( p + i + 2, (UInt32) close - i - 2 ) ) );
                i = (UInt32) close + 2;
                continue;
            }
            AppendBytes( plain, p + i, 2 );
            i += 2;
            continue;
        }
        if ( i + 1 < n && '[' == p[ i ] && '[' == p[ i + 1 ] )
        {
            const Int32 close = FindBytes( p, n, i + 2, "]]" );
            if ( close > (Int32) ( i + 2 ) )
            {
                const char* inner = p + i + 2;
                const UInt32 innerCount = (UInt32) close - i - 2;
                const Int32 bar = FindBytes( inner, innerCount, 0, "|" );
                const CString target = bar >= 0 ? TrimmedString( inner, (UInt32) bar ) : TrimmedString( inner, innerCount );
                const CString label = bar >= 0 ? TrimmedString( inner + bar + 1, innerCount - (UInt32) bar - 1 ) : CString::Empty;
                if ( CHelpMarkupParser::IsValidTopicId( target ) )
                {
                    if ( !plain.empty() )
                    {
                        spans.push_back( CHelpMarkupParser::TSpan( CHelpMarkupParser::SPAN_TEXT, FromBytes( plain ) ) );
                        plain.clear();
                    }
                    spans.push_back( CHelpMarkupParser::TSpan( CHelpMarkupParser::SPAN_LINK, label, target ) );
                    i = (UInt32) close + 2;
                    continue;
                }
            }
            AppendBytes( plain, p + i, 2 );
            i += 2;
            continue;
        }
        plain.push_back( p[ i ] );
        ++i;
    }
    if ( !plain.empty() )
        spans.push_back( CHelpMarkupParser::TSpan( CHelpMarkupParser::SPAN_TEXT, FromBytes( plain ) ) );
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHelpMarkupParser::SSpan::SSpan( void )
    : kind( SPAN_TEXT )
    , text()
    , target()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHelpMarkupParser::SSpan::SSpan( ESpanKind spanKind, const CString& spanText, const CString& spanTarget )
    : kind( spanKind )
    , text( spanText )
    , target( spanTarget )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHelpMarkupParser::IsValidTopicId( const CString& id )
{GUCEF_TRACE;

    CORE::CUtf8String holder;
    UInt32 n = 0;
    const char* p = Utf8Bytes( id, holder, n );
    if ( 0 == n || n > (UInt32) MaxTopicIdLength )
        return false;
    for ( UInt32 i=0; i<n; ++i )
    {
        const char c = p[ i ];
        const bool letterOrDigit = ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' );
        if ( 0 == i && !letterOrDigit )
            return false;
        if ( !letterOrDigit && '.' != c && '_' != c && '-' != c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpMarkupParser::ParseDocument( const CString& text, CHelpDocument& document, CString& error )
{GUCEF_TRACE;

    document.Clear();
    error.Clear();

    CORE::CUtf8String holder;
    UInt32 byteCount = 0;
    const char* bytes = Utf8Bytes( text, holder, byteCount );
    TLineVector lines;
    SplitLines( bytes, byteCount, lines );

    // the header fields: lines starting with '@' before the first other line that is not blank
    UInt32 l = 0;
    for ( ; l<(UInt32)lines.size(); ++l )
    {
        const char* p = lines[ l ].p;
        UInt32 n = lines[ l ].n;
        Trim( p, n );
        if ( 0 == n )
            continue;
        if ( '@' != p[ 0 ] )
            break;
        UInt32 nameEnd = 1;
        while ( nameEnd < n && !IsBlank( p[ nameEnd ] ) )
            ++nameEnd;
        const CString name = FromBytes( p + 1, nameEnd - 1 );
        const CString value = TrimmedString( p + nameEnd, n - nameEnd );
        if ( "id" == name )
            document.id = value;
        else
        if ( "title" == name )
            document.title = value;
        else
        if ( "group" == name )
            document.group = value;
        else
        if ( "brief" == name )
            document.brief = value;
        else
        if ( "version" == name )
            document.version = value;
        else
        if ( "reviewed" == name )
            document.reviewed = value;
        else
        if ( "seealso" == name )
        {
            CORE::CUtf8String valueHolder;
            UInt32 valueCount = 0;
            const char* v = Utf8Bytes( value, valueHolder, valueCount );
            UInt32 start = 0;
            for ( UInt32 i=0; i<=valueCount; ++i )
            {
                if ( i == valueCount || ',' == v[ i ] )
                {
                    const CString item = TrimmedString( v + start, i - start );
                    if ( !item.IsNULLOrEmpty() )
                        document.seeAlso.push_back( item );
                    start = i + 1;
                }
            }
        }
        else
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "HelpMarkupParser: the header field \"" << name << "\" is not known and is left out" );
    }
    ParseBlockLines( lines, l, document.blocks );

    if ( document.id.IsNULLOrEmpty() )
    {
        error = "the document has no @id";
        return false;
    }
    if ( !IsValidTopicId( document.id ) )
    {
        error = "the id \"" + document.id + "\" is not a valid topic id";
        return false;
    }
    if ( document.title.IsNULLOrEmpty() )
    {
        error = "the document " + document.id + " has no @title";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHelpMarkupParser::ParseBlocks( const CString& body, CHelpDocument::TBlockVector& blocks )
{GUCEF_TRACE;

    CORE::CUtf8String holder;
    UInt32 byteCount = 0;
    const char* bytes = Utf8Bytes( body, holder, byteCount );
    TLineVector lines;
    SplitLines( bytes, byteCount, lines );
    ParseBlockLines( lines, 0, blocks );
}

/*-------------------------------------------------------------------------*/

void
CHelpMarkupParser::ParseInline( const CString& text, TSpanVector& spans )
{GUCEF_TRACE;

    CORE::CUtf8String holder;
    UInt32 byteCount = 0;
    const char* bytes = Utf8Bytes( text, holder, byteCount );
    ParseInlineBytes( bytes, byteCount, spans );
}

/*-------------------------------------------------------------------------*/

CString
CHelpMarkupParser::ToPlainText( const CString& text, const CIHelpSource* source )
{GUCEF_TRACE;

    TSpanVector spans;
    ParseInline( text, spans );
    TBytes out;
    for ( UInt32 i=0; i<(UInt32)spans.size(); ++i )
    {
        const TSpan& span = spans[ i ];
        if ( SPAN_LINK != span.kind || !span.text.IsNULLOrEmpty() )
            AppendText( out, span.text );
        else
            AppendText( out, GUCEF_NULL != source ? source->GetTopicTitle( span.target ) : span.target );
    }
    return FromBytes( out );
}

/*-------------------------------------------------------------------------*/

void
CHelpMarkupParser::CollectLinks( const CString& text, TStringVector& targets )
{GUCEF_TRACE;

    TSpanVector spans;
    ParseInline( text, spans );
    for ( UInt32 i=0; i<(UInt32)spans.size(); ++i )
        if ( SPAN_LINK == spans[ i ].kind )
            targets.push_back( spans[ i ].target );
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpMarkupParser::ParseBriefTable( const CString& text, TBriefEntryVector& entries, TStringVector& problems )
{GUCEF_TRACE;

    CORE::CUtf8String holder;
    UInt32 byteCount = 0;
    const char* bytes = Utf8Bytes( text, holder, byteCount );
    TLineVector lines;
    SplitLines( bytes, byteCount, lines );

    UInt32 read = 0;
    for ( UInt32 l=0; l<(UInt32)lines.size(); ++l )
    {
        const char* p = lines[ l ].p;
        UInt32 n = lines[ l ].n;
        Trim( p, n );
        if ( 0 == n || '#' == p[ 0 ] )
            continue;
        const CString lineNumber = CORE::ToString( l + 1 );
        const Int32 first = FindBytes( p, n, 0, "|" );
        const Int32 second = first >= 0 ? FindBytes( p, n, (UInt32) first + 1, "|" ) : -1;
        if ( first < 0 || second < 0 )
        {
            problems.push_back( "line " + lineNumber + ": not of the form \"id | topic | text\"" );
            continue;
        }
        TBriefEntry entry;
        entry.id = TrimmedString( p, (UInt32) first );
        entry.topic = TrimmedString( p + first + 1, (UInt32) ( second - first - 1 ) );
        entry.text = TrimmedString( p + second + 1, n - (UInt32) second - 1 );
        if ( !IsValidTopicId( entry.id ) )
        {
            problems.push_back( "line " + lineNumber + ": the id \"" + entry.id + "\" is not valid" );
            continue;
        }
        if ( !entry.topic.IsNULLOrEmpty() && !IsValidTopicId( entry.topic ) )
        {
            problems.push_back( "line " + lineNumber + ": the topic \"" + entry.topic + "\" of " + entry.id + " is not valid" );
            continue;
        }
        if ( entry.text.IsNULLOrEmpty() )
        {
            problems.push_back( "line " + lineNumber + ": " + entry.id + " has no text" );
            continue;
        }
        entries.push_back( entry );
        ++read;
    }
    return read;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
