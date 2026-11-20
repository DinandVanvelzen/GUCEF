/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWEBJSON_H
#include "gucefWEB_CWebJson.h"
#define GUCEF_WEB_CWEBJSON_H
#endif /* GUCEF_WEB_CWEBJSON_H ? */

#include "gucefWEB_CReactFlightPayload.h"

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

/** The deepest an element tree is followed for its text */
const UInt32 MaxTextDepth = 64;

/** The bytes of a text without the closing zero */
UInt32
ByteLength( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

bool
IsHexDigit( char c )
{GUCEF_TRACE;

    return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
}

/*-------------------------------------------------------------------------*/

UInt32
HexValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return (UInt32) ( c - '0' );
    if ( c >= 'a' && c <= 'f' )
        return (UInt32) ( c - 'a' + 10 );
    if ( c >= 'A' && c <= 'F' )
        return (UInt32) ( c - 'A' + 10 );
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Reads 'count' hexadecimal digits at 'pos'; false when there are not that many */
bool
ReadHex( const char* data, UInt32 size, UInt32 pos, UInt32 count, UInt32& value )
{GUCEF_TRACE;

    value = 0;
    if ( pos + count > size )
        return false;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !IsHexDigit( data[ pos + i ] ) )
            return false;
        value = ( value << 4 ) | HexValue( data[ pos + i ] );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
AppendUtf8( GUCEF::vector< char >& out, UInt32 codePoint )
{GUCEF_TRACE;

    if ( codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
        codePoint = 0xFFFD;         // a lone surrogate or out of range: the replacement character
    if ( codePoint < 0x80 )
    {
        out.push_back( (char) codePoint );
    }
    else
    if ( codePoint < 0x800 )
    {
        out.push_back( (char) ( 0xC0 | ( codePoint >> 6 ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    if ( codePoint < 0x10000 )
    {
        out.push_back( (char) ( 0xE0 | ( codePoint >> 12 ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    {
        out.push_back( (char) ( 0xF0 | ( codePoint >> 18 ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeString( const char* data, UInt32 size )
{GUCEF_TRACE;

    CORE::CString text;
    if ( size > 0 )
        text.Set( data, size );
    return text;
}

/*-------------------------------------------------------------------------*/

/** The first index of 'needle' in the bytes from 'from', or -1 */
Int32
FindBytes( const char* data, UInt32 size, UInt32 from, const char* needle )
{GUCEF_TRACE;

    const UInt32 needleSize = (UInt32) strlen( needle );
    if ( 0 == needleSize || needleSize > size )
        return -1;
    for ( UInt32 i=from; i+needleSize<=size; ++i )
    {
        if ( data[ i ] == needle[ 0 ] && 0 == memcmp( data + i, needle, needleSize ) )
            return (Int32) i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The row tags whose payload is length prefixed: text and the binary types of newer React */
bool
IsLengthTag( char c )
{GUCEF_TRACE;

    return 0 != c && GUCEF_NULL != strchr( "TAOoUSsLlGgMmV", c );
}

/*-------------------------------------------------------------------------*/

/** Whether a payload is a JSON value by its first character */
bool
StartsAsJson( const CORE::CString& payload )
{GUCEF_TRACE;

    if ( payload.IsNULLOrEmpty() )
        return false;
    const char c = payload.C_String()[ 0 ];
    return c == '[' || c == '{' || c == '"' || c == '-' || ( c >= '0' && c <= '9' ) || c == 't' || c == 'f' || c == 'n';
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReactFlightPayload::CReactFlightPayload( void )
    : m_rows()
    , m_skipped( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CReactFlightPayload::~CReactFlightPayload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::Clear( void )
{GUCEF_TRACE;

    m_rows.clear();
    m_skipped = 0;
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::DecodeStringLiteral( const CORE::CString& text, UInt32 start, CORE::CString& value, UInt32& end )
{GUCEF_TRACE;

    value.Clear();
    end = start;
    const char* data = text.C_String();
    const UInt32 size = ByteLength( text );
    if ( start >= size || ( data[ start ] != '"' && data[ start ] != '\'' ) )
        return false;
    const char quote = data[ start ];
    GUCEF::vector< char > out;
    out.reserve( 256 );
    UInt32 pos = start + 1;
    while ( pos < size )
    {
        const char c = data[ pos ];
        if ( c == quote )
        {
            end = pos + 1;
            if ( !out.empty() )
                value.Set( &out[ 0 ], (UInt32) out.size() );
            return true;
        }
        if ( c != '\\' )
        {
            out.push_back( c );
            ++pos;
            continue;
        }
        if ( pos + 1 >= size )
            break;
        const char e = data[ pos + 1 ];
        pos += 2;
        switch ( e )
        {
            case 'n': out.push_back( '\n' ); break;
            case 'r': out.push_back( '\r' ); break;
            case 't': out.push_back( '\t' ); break;
            case 'b': out.push_back( '\b' ); break;
            case 'f': out.push_back( '\f' ); break;
            case 'v': out.push_back( '\v' ); break;
            case '0': break;                    // a NUL character is dropped: the text is kept as a C string
            case '\r':
            {
                // a line continuation
                if ( pos < size && data[ pos ] == '\n' )
                    ++pos;
                break;
            }
            case '\n': break;
            case 'x':
            {
                UInt32 cp = 0;
                if ( !ReadHex( data, size, pos, 2, cp ) )
                    return false;
                pos += 2;
                if ( 0 != cp )
                    AppendUtf8( out, cp );
                break;
            }
            case 'u':
            {
                UInt32 cp = 0;
                if ( pos < size && data[ pos ] == '{' )
                {
                    // \u{1F600}
                    UInt32 close = pos + 1;
                    while ( close < size && IsHexDigit( data[ close ] ) && close - pos <= 6 )
                        ++close;
                    if ( close >= size || data[ close ] != '}' || close == pos + 1 )
                        return false;
                    ReadHex( data, size, pos + 1, close - pos - 1, cp );
                    pos = close + 1;
                }
                else
                {
                    if ( !ReadHex( data, size, pos, 4, cp ) )
                        return false;
                    pos += 4;
                    if ( cp >= 0xD800 && cp <= 0xDBFF && pos + 6 <= size && data[ pos ] == '\\' && data[ pos + 1 ] == 'u' )
                    {
                        UInt32 low = 0;
                        if ( ReadHex( data, size, pos + 2, 4, low ) && low >= 0xDC00 && low <= 0xDFFF )
                        {
                            cp = 0x10000 + ( ( cp - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                            pos += 6;
                        }
                    }
                }
                if ( 0 != cp )
                    AppendUtf8( out, cp );
                break;
            }
            default:
            {
                // \" \' \\ \/ and every other escaped character stand for themselves
                out.push_back( e );
                break;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::ExtractFromHtml( const CORE::CString& html, CORE::CString& flightText, UInt32* chunkCount, UInt32* skippedCount )
{GUCEF_TRACE;

    flightText.Clear();
    if ( GUCEF_NULL != chunkCount )
        *chunkCount = 0;
    if ( GUCEF_NULL != skippedCount )
        *skippedCount = 0;

    static const char* const marker = "__next_f.push([";
    const UInt32 markerSize = (UInt32) strlen( marker );
    const char* data = html.C_String();
    const UInt32 size = ByteLength( html );
    GUCEF::vector< char > out;
    UInt32 taken = 0;
    UInt32 skipped = 0;
    Int32 at = FindBytes( data, size, 0, marker );
    while ( at >= 0 )
    {
        UInt32 pos = (UInt32) at + markerSize;
        UInt32 type = 0;
        bool haveType = false;
        while ( pos < size && data[ pos ] >= '0' && data[ pos ] <= '9' )
        {
            type = type * 10 + (UInt32) ( data[ pos ] - '0' );
            haveType = true;
            ++pos;
        }
        while ( pos < size && ( data[ pos ] == ' ' || data[ pos ] == '\t' ) )
            ++pos;
        if ( haveType && pos < size && data[ pos ] == ',' )
        {
            ++pos;
            while ( pos < size && ( data[ pos ] == ' ' || data[ pos ] == '\t' || data[ pos ] == '\n' || data[ pos ] == '\r' ) )
                ++pos;
            CORE::CString chunk;
            UInt32 end = pos;
            if ( 1 == type && DecodeStringLiteral( html, pos, chunk, end ) )
            {
                const UInt32 chunkSize = ByteLength( chunk );
                const char* chunkData = chunk.C_String();
                out.insert( out.end(), chunkData, chunkData + chunkSize );
                ++taken;
                pos = end;
            }
            else
            {
                ++skipped;
            }
        }
        else
        if ( haveType && 0 != type )
        {
            ++skipped;
        }
        at = FindBytes( data, size, pos, marker );
    }
    if ( GUCEF_NULL != chunkCount )
        *chunkCount = taken;
    if ( GUCEF_NULL != skippedCount )
        *skippedCount = skipped;
    if ( 0 == taken )
        return false;
    if ( !out.empty() )
        flightText.Set( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::LooksLikePayload( const CORE::CString& text )
{GUCEF_TRACE;

    const char* data = text.C_String();
    const UInt32 size = ByteLength( text );
    UInt32 pos = 0;
    while ( pos < size && IsHexDigit( data[ pos ] ) )
        ++pos;
    return pos > 0 && pos < size && data[ pos ] == ':';
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::Parse( const CORE::CString& payloadText )
{GUCEF_TRACE;

    m_rows.clear();
    const char* data = payloadText.C_String();
    const UInt32 size = ByteLength( payloadText );
    UInt32 pos = 0;
    while ( pos < size )
    {
        if ( data[ pos ] == '\n' || data[ pos ] == '\r' )
        {
            ++pos;
            continue;
        }

        // the id, hexadecimal, and the colon
        const UInt32 idStart = pos;
        while ( pos < size && IsHexDigit( data[ pos ] ) )
            ++pos;
        if ( pos == idStart || pos >= size || data[ pos ] != ':' )
        {
            // not a row: the rest of the line is skipped
            while ( pos < size && data[ pos ] != '\n' )
                ++pos;
            ++m_skipped;
            continue;
        }
        CReactFlightRow row;
        row.id = MakeString( data + idStart, pos - idStart ).Lowercase();
        ++pos;

        // a length prefixed row: <tag><hex length>,<bytes>
        bool lengthRow = false;
        if ( pos < size && IsLengthTag( data[ pos ] ) )
        {
            UInt32 p = pos + 1;
            UInt32 length = 0;
            while ( p < size && IsHexDigit( data[ p ] ) && p - pos <= 8 )
            {
                length = ( length << 4 ) | HexValue( data[ p ] );
                ++p;
            }
            if ( p > pos + 1 && p < size && data[ p ] == ',' )
            {
                lengthRow = true;
                row.tag = MakeString( data + pos, 1 );
                ++p;
                const UInt32 available = size - p;
                const UInt32 taken = length < available ? length : available;
                row.text = MakeString( data + p, taken );
                pos = p + taken;
            }
        }
        if ( !lengthRow )
        {
            // a row that ends at the line end, with an upper case tag (a hint has its code letter after the H) or none
            if ( pos < size && ( ( data[ pos ] >= 'A' && data[ pos ] <= 'Z' ) || data[ pos ] == '#' || data[ pos ] == 'r' || data[ pos ] == 'x' ) )
            {
                UInt32 tagSize = 1;
                if ( data[ pos ] == 'H' && pos + 1 < size && ( ( data[ pos + 1 ] >= 'A' && data[ pos + 1 ] <= 'Z' ) || ( data[ pos + 1 ] >= 'a' && data[ pos + 1 ] <= 'z' ) ) )
                    tagSize = 2;
                row.tag = MakeString( data + pos, tagSize );
                pos += tagSize;
            }
            const UInt32 payloadStart = pos;
            while ( pos < size && data[ pos ] != '\n' )
                ++pos;
            UInt32 payloadEnd = pos;
            if ( payloadEnd > payloadStart && data[ payloadEnd - 1 ] == '\r' )
                --payloadEnd;
            row.text = MakeString( data + payloadStart, payloadEnd - payloadStart );
            if ( StartsAsJson( row.text ) )
                row.hasJson = CWebJson::FromText( row.text, row.json );
        }
        m_rows.push_back( row );
    }
    return !m_rows.empty();
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::ParseHtml( const CORE::CString& html )
{GUCEF_TRACE;

    Clear();
    CORE::CString flightText;
    UInt32 chunks = 0;
    UInt32 skipped = 0;
    const bool found = ExtractFromHtml( html, flightText, &chunks, &skipped );
    if ( !found )
    {
        m_skipped = skipped;
        return false;
    }
    const bool parsed = Parse( flightText );
    m_skipped += skipped;
    return parsed;
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::ParseAny( const CORE::CString& text )
{GUCEF_TRACE;

    const char* data = text.C_String();
    const UInt32 size = ByteLength( text );
    UInt32 pos = 0;
    while ( pos < size && ( data[ pos ] == ' ' || data[ pos ] == '\t' || data[ pos ] == '\n' || data[ pos ] == '\r' ) )
        ++pos;
    if ( pos < size && data[ pos ] == '<' )
        return ParseHtml( text );
    Clear();
    return Parse( text );
}

/*-------------------------------------------------------------------------*/

const TReactFlightRowVector&
CReactFlightPayload::GetRows( void ) const
{GUCEF_TRACE;

    return m_rows;
}

/*-------------------------------------------------------------------------*/

const CReactFlightRow*
CReactFlightPayload::FindRow( const CORE::CString& id ) const
{GUCEF_TRACE;

    const CORE::CString lower = id.Lowercase();
    for ( size_t i=0; i<m_rows.size(); ++i )
        if ( m_rows[ i ].id == lower )
            return &m_rows[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CReactFlightPayload::GetJsonRowCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_rows.size(); ++i )
        if ( m_rows[ i ].hasJson )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CReactFlightPayload::GetSkippedCount( void ) const
{GUCEF_TRACE;

    return m_skipped;
}

/*-------------------------------------------------------------------------*/

bool
CReactFlightPayload::IsRowReference( const CORE::CString& value, CORE::CString& rowId )
{GUCEF_TRACE;

    rowId.Clear();
    const char* data = value.C_String();
    const UInt32 size = ByteLength( value );
    if ( size < 2 || data[ 0 ] != '$' )
        return false;
    UInt32 pos = 1;
    if ( GUCEF_NULL != strchr( "L@FhQW", data[ 1 ] ) && 0 != data[ 1 ] )
        pos = 2;
    const UInt32 idStart = pos;
    while ( pos < size && IsHexDigit( data[ pos ] ) )
        ++pos;
    if ( pos == idStart || ( pos < size && data[ pos ] != ':' ) )
        return false;
    rowId = MakeString( data + idStart, pos - idStart ).Lowercase();
    return true;
}

/*-------------------------------------------------------------------------*/

const CReactFlightRow*
CReactFlightPayload::ResolveReference( const CORE::CString& value ) const
{GUCEF_TRACE;

    CORE::CString rowId;
    if ( !IsRowReference( value, rowId ) )
        return GUCEF_NULL;
    return FindRow( rowId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CReactFlightPayload::ValueAsText( const CORE::CString& value ) const
{GUCEF_TRACE;

    const char* data = value.C_String();
    const UInt32 size = ByteLength( value );
    if ( 0 == size || data[ 0 ] != '$' )
        return value;
    if ( size >= 2 && data[ 1 ] == '$' )
        return MakeString( data + 1, size - 1 );
    const CReactFlightRow* row = ResolveReference( value );
    if ( GUCEF_NULL != row && row->IsText() )
        return row->text;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::CollectObjects( const CORE::CDataNode& node, TConstNodeVector& objects )
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_OBJECT == node.GetNodeType() )
        objects.push_back( &node );
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        if ( GUCEF_NULL != (*i) )
            CollectObjects( *(*i), objects );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::CollectObjects( TConstNodeVector& objects ) const
{GUCEF_TRACE;

    for ( size_t r=0; r<m_rows.size(); ++r )
        if ( m_rows[ r ].hasJson )
            CollectObjects( m_rows[ r ].json, objects );
}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::AppendValueText( const CORE::CString& value, CORE::CString& text, UInt32 depth ) const
{GUCEF_TRACE;

    CORE::CString part = ValueAsText( value );
    if ( part.IsNULLOrEmpty() )
    {
        // a reference to a model row (a lazy element): its text
        const CReactFlightRow* row = ResolveReference( value );
        if ( GUCEF_NULL != row && row->hasJson && depth < MaxTextDepth )
            AppendText( row->json, text, depth + 1 );
        return;
    }
    part = part.Trim( true ).Trim( false );
    if ( part.IsNULLOrEmpty() )
        return;
    if ( !text.IsNULLOrEmpty() )
        text += " ";
    text += part;
}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::AppendText( const CORE::CDataNode& node, CORE::CString& text, UInt32 depth ) const
{GUCEF_TRACE;

    if ( depth > MaxTextDepth )
        return;
    if ( GUCEF_DATATYPE_ARRAY == node.GetNodeType() )
    {
        // an element ["$", type, key, props]: the children of its props; any other list: each entry
        GUCEF::vector< const CORE::CDataNode* > entries;
        CORE::CDataNode::const_iterator i = node.ConstBegin();
        while ( i != node.ConstEnd() )
        {
            if ( GUCEF_NULL != (*i) )
                entries.push_back( (*i) );
            ++i;
        }
        if ( !entries.empty() && entries[ 0 ]->HasValue() && entries[ 0 ]->GetValue().AsString() == "$" )
        {
            // the props are the object after the type and the key (the key may be null, which a reader may leave out)
            for ( size_t e=1; e<entries.size(); ++e )
            {
                if ( GUCEF_DATATYPE_OBJECT == entries[ e ]->GetNodeType() )
                {
                    AppendText( *entries[ e ], text, depth + 1 );
                    break;
                }
            }
            return;
        }
        for ( size_t e=0; e<entries.size(); ++e )
        {
            if ( entries[ e ]->GetNodeType() == GUCEF_DATATYPE_ARRAY || entries[ e ]->GetNodeType() == GUCEF_DATATYPE_OBJECT )
                AppendText( *entries[ e ], text, depth + 1 );
            else
            if ( entries[ e ]->HasValue() && entries[ e ]->GetValue().IsString() )
                AppendValueText( entries[ e ]->GetValue().AsString(), text, depth );
        }
        return;
    }
    if ( GUCEF_DATATYPE_OBJECT == node.GetNodeType() )
    {
        // props: their children only
        const CORE::CVariant& children = node.GetAttributeValue( "children" );
        if ( children.IsString() )
        {
            AppendValueText( children.AsString(), text, depth );
            return;
        }
        const CORE::CDataNode* childNode = node.FindChild( "children" );
        if ( GUCEF_NULL != childNode )
            AppendText( *childNode, text, depth + 1 );
        return;
    }
    if ( node.HasValue() && node.GetValue().IsString() )
        AppendValueText( node.GetValue().AsString(), text, depth );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CReactFlightPayload::GetElementText( const CORE::CDataNode& props ) const
{GUCEF_TRACE;

    CORE::CString text;
    AppendText( props, text, 0 );
    return text;
}

/*-------------------------------------------------------------------------*/

void
CReactFlightPayload::CollectLinks( TStringPairVector& links ) const
{GUCEF_TRACE;

    TConstNodeVector objects;
    CollectObjects( objects );
    for ( size_t o=0; o<objects.size(); ++o )
    {
        const CORE::CVariant& href = objects[ o ]->GetAttributeValue( "href" );
        if ( !href.IsString() )
            continue;
        const CORE::CString hrefText = ValueAsText( href.AsString() );
        if ( hrefText.IsNULLOrEmpty() )
            continue;
        links.push_back( TStringPair( hrefText, GetElementText( *objects[ o ] ) ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
