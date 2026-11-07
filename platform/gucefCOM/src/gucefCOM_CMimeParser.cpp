/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOM_CMimeParser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SDelimiter
{
    UInt32 lineStart;
    UInt32 nextLine;
    bool closing;
};

inline bool
IsWhite( char c )
{
    return ' ' == c || '\t' == c;
}

/*-------------------------------------------------------------------------*/

/** The end of the line that starts at 'pos': the offset of its line break (or the end of the data) and the offset where the next line starts */
void
FindLineEnd( const char* data, UInt32 size, UInt32 pos, UInt32& lineEnd, UInt32& nextLine )
{
    lineEnd = pos;
    while ( lineEnd < size && '\n' != data[ lineEnd ] && '\r' != data[ lineEnd ] )
        ++lineEnd;
    nextLine = lineEnd;
    if ( lineEnd < size )
    {
        if ( '\r' == data[ lineEnd ] && lineEnd + 1 < size && '\n' == data[ lineEnd + 1 ] )
            nextLine = lineEnd + 2;
        else
            nextLine = lineEnd + 1;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
LowerTrimmed( const CORE::CString& text )
{
    return text.Lowercase().Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

/** Text with CR LF and CR turned into LF, and one final line break taken off */
CORE::CString
NormalizeTextBody( const CORE::CString& text )
{
    const char* p = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    GUCEF::vector< char > out;
    out.reserve( size );
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\r' == p[ i ] )
        {
            if ( i + 1 < size && '\n' == p[ i + 1 ] )
                ++i;
            out.push_back( '\n' );
        }
        else
            out.push_back( p[ i ] );
    }
    if ( !out.empty() && '\n' == out.back() )
        out.pop_back();
    return out.empty() ? CORE::CString() : CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/** Adds the header field that was collected, unfolded. @return false when the number of fields exceeds the limit */
bool
FlushHeaderField( GUCEF::vector< char >& current, CMailHeaders& headers, CORE::CString::StringVector* warnings, UInt32& fieldCount, UInt32 maxCount )
{
    const char* c = current.empty() ? "" : &current[ 0 ];
    const UInt32 size = (UInt32) current.size();
    const char* colon = static_cast< const char* >( memchr( c, ':', size ) );
    bool ok = true;
    if ( GUCEF_NULL == colon )
    {
        if ( GUCEF_NULL != warnings && warnings->size() < 50 )
            warnings->push_back( "A header line without a colon was skipped" );
    }
    else
    {
        UInt32 nameEnd = (UInt32)( colon - c );
        while ( nameEnd > 0 && IsWhite( c[ nameEnd - 1 ] ) )
            --nameEnd;
        CORE::CString name( c, nameEnd );
        if ( !CMimeCodec::IsValidHeaderName( name ) )
        {
            if ( GUCEF_NULL != warnings && warnings->size() < 50 )
                warnings->push_back( "A header with an invalid name was skipped" );
        }
        else
        if ( ++fieldCount > maxCount )
            ok = false;
        else
            headers.Add( name, CMimeCodec::Unfold( colon + 1, size - (UInt32)( colon - c ) - 1 ) );
    }
    current.clear();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
HasHeaderOf( const CORE::CString& name, const char* const* names, size_t count )
{
    for ( size_t i=0; i<count; ++i )
        if ( name.Equals( names[ i ], false ) )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMimeLimits::CMimeLimits( void )
    : maxMessageSize( 64 * 1024 * 1024 )
    , maxHeaderBytes( 256 * 1024 )
    , maxHeaderCount( 1000 )
    , maxParts( 512 )
    , maxDepth( 16 )
    , maxDecodedPartSize( 64 * 1024 * 1024 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMimeParser::CMimeParser( void )
    : m_limits()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMimeParser::CMimeParser( const CMimeLimits& limits )
    : m_limits( limits )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMimeParser::SetLimits( const CMimeLimits& limits )
{GUCEF_TRACE;

    m_limits = limits;
}

/*-------------------------------------------------------------------------*/

const CMimeLimits&
CMimeParser::GetLimits( void ) const
{GUCEF_TRACE;

    return m_limits;
}

/*-------------------------------------------------------------------------*/

void
CMimeParser::AddWarning( SState& state, const char* text )
{GUCEF_TRACE;

    if ( GUCEF_NULL != state.warnings && state.warnings->size() < 50 )
        state.warnings->push_back( text );
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::ParseHeaders( const void* rawData, UInt32 size, CMailHeaders& headers, UInt32& bodyOffset, CORE::CString::StringVector* warnings, CORE::CString* error ) const
{GUCEF_TRACE;

    const char* data = static_cast< const char* >( rawData );
    headers.Clear();
    bodyOffset = size;
    if ( GUCEF_NULL == data && size > 0 )
        return false;

    UInt32 pos = 0;

    // an mbox style first line is not a header
    if ( size > 5 && 0 == memcmp( data, "From ", 5 ) )
    {
        UInt32 lineEnd = 0, nextLine = 0;
        FindLineEnd( data, size, 0, lineEnd, nextLine );
        pos = nextLine;
    }

    GUCEF::vector< char > current;
    bool haveCurrent = false;
    UInt32 fieldCount = 0;

    while ( pos < size )
    {
        if ( pos > m_limits.maxHeaderBytes )
        {
            if ( GUCEF_NULL != error )
                *error = "The header section is too large";
            headers.Clear();
            return false;
        }

        UInt32 lineEnd = 0, nextLine = 0;
        FindLineEnd( data, size, pos, lineEnd, nextLine );
        if ( nextLine > m_limits.maxHeaderBytes && lineEnd > pos )
        {
            if ( GUCEF_NULL != error )
                *error = "The header section is too large";
            headers.Clear();
            return false;
        }
        if ( lineEnd == pos )
        {
            // the empty line ends the header section
            if ( haveCurrent && !FlushHeaderField( current, headers, warnings, fieldCount, m_limits.maxHeaderCount ) )
            {
                if ( GUCEF_NULL != error )
                    *error = "The message has too many header fields";
                headers.Clear();
                return false;
            }
            bodyOffset = nextLine;
            return true;
        }

        if ( IsWhite( data[ pos ] ) )
        {
            if ( haveCurrent )
            {
                // the line break of the fold is dropped, the white space that follows it stays
                current.insert( current.end(), data + pos, data + lineEnd );
            }
            else
            if ( GUCEF_NULL != warnings && warnings->size() < 50 )
                warnings->push_back( "A continuation line without a header was skipped" );
        }
        else
        {
            if ( haveCurrent && !FlushHeaderField( current, headers, warnings, fieldCount, m_limits.maxHeaderCount ) )
            {
                if ( GUCEF_NULL != error )
                    *error = "The message has too many header fields";
                headers.Clear();
                return false;
            }
            current.assign( data + pos, data + lineEnd );
            haveCurrent = true;
        }
        pos = nextLine;
    }

    if ( haveCurrent && !FlushHeaderField( current, headers, warnings, fieldCount, m_limits.maxHeaderCount ) )
    {
        if ( GUCEF_NULL != error )
            *error = "The message has too many header fields";
        headers.Clear();
        return false;
    }
    bodyOffset = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::ParseParts( const void* data, UInt32 size, CMimePart& root, CORE::CString::StringVector* warnings, CORE::CString* error ) const
{GUCEF_TRACE;

    root.Clear();
    if ( size > m_limits.maxMessageSize )
    {
        if ( GUCEF_NULL != error )
            *error = "The message is larger than the limit";
        return false;
    }

    SState state;
    state.partCount = 0;
    state.warnings = warnings;
    if ( !ParseEntity( static_cast< const char* >( data ), size, root, 0, true, state ) )
    {
        if ( GUCEF_NULL != error )
            *error = state.error;
        root.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::ParseEntity( const char* data, UInt32 size, CMimePart& part, UInt32 depth, bool topLevel, SState& state ) const
{GUCEF_TRACE;

    if ( ++state.partCount > m_limits.maxParts )
    {
        state.error = "The message has too many parts";
        return false;
    }

    UInt32 bodyOffset = 0;
    CORE::CString headerError;
    if ( !ParseHeaders( data, size, part.headers, bodyOffset, state.warnings, &headerError ) )
    {
        state.error = headerError;
        return false;
    }
    part.ApplyHeaders();

    const char* body = data + bodyOffset;
    const UInt32 bodySize = size - bodyOffset;

    if ( part.IsMultipart() )
    {
        if ( part.GetBoundary().IsNULLOrEmpty() )
        {
            AddWarning( state, "A multipart has no boundary and is read as text" );
            part.contentType = "text/plain";
        }
        else
        if ( depth + 1 >= m_limits.maxDepth )
        {
            state.error = "The multiparts are nested too deep";
            return false;
        }
        else
            return ParseMultipart( body, bodySize, part, depth, state );
    }
    return DecodeBody( body, bodySize, part, state );
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::ParseMultipart( const char* data, UInt32 size, CMimePart& part, UInt32 depth, SState& state ) const
{GUCEF_TRACE;

    const CORE::CString delimiter = "--" + part.GetBoundary();
    const UInt32 delimiterSize = delimiter.ByteSize() - 1;
    const char* d = delimiter.C_String();

    // the lines that are delimiters
    GUCEF::vector< SDelimiter > found;

    UInt32 lineStart = 0;
    while ( lineStart < size )
    {
        UInt32 lineEnd = 0, nextLine = 0;
        FindLineEnd( data, size, lineStart, lineEnd, nextLine );
        if ( lineEnd - lineStart >= delimiterSize && 0 == memcmp( data + lineStart, d, delimiterSize ) )
        {
            UInt32 after = lineStart + delimiterSize;
            bool closing = after + 1 < lineEnd + 0 && '-' == data[ after ] && '-' == data[ after + 1 ];
            if ( after + 1 <= lineEnd && closing )
                after += 2;
            // only white space may follow on the line
            bool onlyWhite = true;
            for ( UInt32 i=after; i<lineEnd; ++i )
                if ( !IsWhite( data[ i ] ) )
                    onlyWhite = false;
            if ( onlyWhite )
            {
                SDelimiter delim;
                delim.lineStart = lineStart;
                delim.nextLine = nextLine;
                delim.closing = closing;
                found.push_back( delim );
                if ( closing )
                    break;
                if ( found.size() > m_limits.maxParts + 1 )
                {
                    state.error = "The message has too many parts";
                    return false;
                }
            }
        }
        lineStart = nextLine;
    }

    if ( found.empty() )
    {
        AddWarning( state, "A multipart has no delimiter and is read as text" );
        part.contentType = "text/plain";
        return DecodeBody( data, size, part, state );
    }
    if ( !found.back().closing )
        AddWarning( state, "A multipart has no closing delimiter" );

    for ( size_t i=0; i<found.size(); ++i )
    {
        if ( found[ i ].closing )
            break;
        const UInt32 partStart = found[ i ].nextLine;
        UInt32 partEnd = size;
        if ( i + 1 < found.size() )
        {
            partEnd = found[ i + 1 ].lineStart;
            // the line break in front of a delimiter belongs to the delimiter
            if ( partEnd > partStart && '\n' == data[ partEnd - 1 ] )
                --partEnd;
            if ( partEnd > partStart && '\r' == data[ partEnd - 1 ] )
                --partEnd;
        }
        if ( partEnd < partStart )
            partEnd = partStart;

        CMimePart* child = part.AddChild();
        if ( !ParseEntity( data + partStart, partEnd - partStart, *child, depth + 1, false, state ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::DecodeBody( const char* data, UInt32 size, CMimePart& part, SState& state ) const
{GUCEF_TRACE;

    part.body.Clear();
    const CORE::CString& encoding = part.transferEncoding;
    bool ok = true;
    if ( encoding == "base64" )
    {
        bool hadErrors = false;
        ok = CMimeCodec::DecodeBase64Lenient( data, size, part.body, m_limits.maxDecodedPartSize, &hadErrors );
        if ( hadErrors )
            AddWarning( state, "A base64 body had characters that are not base64" );
    }
    else
    if ( encoding == "quoted-printable" )
    {
        ok = CMimeCodec::DecodeQuotedPrintable( data, size, part.body, false, m_limits.maxDecodedPartSize );
    }
    else
    {
        if ( !encoding.IsNULLOrEmpty() && encoding != "7bit" && encoding != "8bit" && encoding != "binary" )
            AddWarning( state, "A part has an unknown transfer encoding and is taken as it is" );
        if ( size > m_limits.maxDecodedPartSize )
            ok = false;
        else
        if ( size > 0 )
            part.body.CopyFrom( size, data );
    }
    if ( !ok )
    {
        state.error = "A part is larger than the limit";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMimeParser::ExtractMessageIds( const CORE::CString& headerValue, CORE::CString::StringVector& ids, UInt32 maxIds )
{GUCEF_TRACE;

    const char* p = headerValue.C_String();
    const UInt32 size = headerValue.ByteSize() > 0 ? headerValue.ByteSize() - 1 : 0;
    UInt32 i = 0;
    while ( i < size && ids.size() < maxIds )
    {
        if ( '<' != p[ i ] )
        {
            ++i;
            continue;
        }
        UInt32 end = i + 1;
        while ( end < size && '>' != p[ end ] && '<' != p[ end ] )
            ++end;
        if ( end < size && '>' == p[ end ] && end - i > 1 && end - i <= 998 )
        {
            bool clean = true;
            for ( UInt32 k=i+1; k<end; ++k )
                if ( ( (UInt8) p[ k ] ) <= 0x20 )
                    clean = false;
            if ( clean )
                ids.push_back( CORE::CString( p + i, end - i + 1 ) );
        }
        i = end + 1;
    }
}

/*-------------------------------------------------------------------------*/

void
CMimeParser::ApplyHeaders( const CMailHeaders& headers, CMailMessage& message )
{GUCEF_TRACE;

    message.headers = headers;

    CORE::CString::StringVector values;
    headers.GetAll( "From", values );
    for ( size_t i=0; i<values.size(); ++i )
        CMailAddress::ParseList( values[ i ], message.from );
    values.clear();
    headers.GetAll( "To", values );
    for ( size_t i=0; i<values.size(); ++i )
        CMailAddress::ParseList( values[ i ], message.to );
    values.clear();
    headers.GetAll( "Cc", values );
    for ( size_t i=0; i<values.size(); ++i )
        CMailAddress::ParseList( values[ i ], message.cc );
    values.clear();
    headers.GetAll( "Bcc", values );
    for ( size_t i=0; i<values.size(); ++i )
        CMailAddress::ParseList( values[ i ], message.bcc );
    values.clear();
    headers.GetAll( "Reply-To", values );
    for ( size_t i=0; i<values.size(); ++i )
        CMailAddress::ParseList( values[ i ], message.replyTo );
    CMailAddress::Parse( headers.Get( "Sender" ), message.sender );

    message.subject = headers.GetDecoded( "Subject" );

    message.hasDate = CMimeCodec::ParseDate( headers.Get( "Date" ), message.date );

    CORE::CString::StringVector ids;
    ExtractMessageIds( headers.Get( "Message-ID" ), ids, 1 );
    if ( !ids.empty() )
        message.messageId = ids[ 0 ];
    ExtractMessageIds( headers.Get( "In-Reply-To" ), message.inReplyTo );
    CORE::CString::StringVector referenceFields;
    headers.GetAll( "References", referenceFields );
    for ( size_t i=0; i<referenceFields.size(); ++i )
        ExtractMessageIds( referenceFields[ i ], message.references );

    static const char* const typed[] =
    {
        "from", "sender", "to", "cc", "bcc", "reply-to", "subject", "date", "message-id", "in-reply-to", "references",
        "mime-version", "content-type", "content-transfer-encoding", "content-disposition", "content-id", "content-length"
    };
    for ( UInt32 i=0; i<headers.GetCount(); ++i )
    {
        const CMailHeaders::TField& field = headers.GetField( i );
        if ( !HasHeaderOf( field.name, typed, sizeof( typed ) / sizeof( typed[ 0 ] ) ) )
            message.extraHeaders.Add( field.name, field.value );
    }
}

/*-------------------------------------------------------------------------*/

void
CMimeParser::MapPart( const CMimePart& part, CMailMessage& message, bool& haveText, bool& haveHtml, UInt32& extraTextCount, bool topLevel ) const
{GUCEF_TRACE;

    if ( part.IsMultipart() )
    {
        for ( size_t i=0; i<part.children.size(); ++i )
            MapPart( *part.children[ i ], message, haveText, haveHtml, extraTextCount, false );
        return;
    }

    const CORE::CString fileName = part.GetFileName();
    const bool isAttachmentDisposition = part.disposition == "attachment";
    const bool isPlain = part.contentType == "text/plain";
    const bool isHtml = part.contentType == "text/html";

    if ( ( isPlain || isHtml ) && !isAttachmentDisposition )
    {
        bool lossy = false;
        const char* bytes = part.body.GetDataSize() > 0 ? static_cast< const char* >( part.body.GetConstBufferPtr() ) : "";
        const CORE::CString charset = part.GetCharset();
        CORE::CString text = NormalizeTextBody( CMimeCodec::ToUtf8( charset.IsNULLOrEmpty() ? CORE::CString( "utf-8" ) : charset, bytes, part.body.GetDataSize(), lossy ) );
        if ( lossy )
            message.warnings.push_back( "The character set of a text part is not supported and was read as UTF-8" );

        if ( fileName.IsNULLOrEmpty() )
        {
            if ( isPlain && !haveText )
            {
                message.textBody = text;
                haveText = true;
                return;
            }
            if ( isHtml && !haveHtml )
            {
                message.htmlBody = text;
                haveHtml = true;
                return;
            }
            // a further text part without a name: kept as an attachment instead of being lost
            message.warnings.push_back( "A further text part was kept as an attachment" );
        }
    }

    CMailAttachment attachment;
    attachment.fileName = fileName;
    if ( attachment.fileName.IsNULLOrEmpty() )
    {
        if ( part.contentType == "message/rfc822" )
            attachment.fileName = "message.eml";
        else
        if ( isPlain )
            attachment.fileName = "part" + CORE::UInt32ToString( ++extraTextCount ) + ".txt";
        else
        if ( isHtml )
            attachment.fileName = "part" + CORE::UInt32ToString( ++extraTextCount ) + ".html";
    }
    attachment.contentType = part.contentType;
    attachment.contentId = part.contentId;
    attachment.isInline = part.disposition == "inline";
    if ( part.body.GetDataSize() > 0 )
        attachment.data.CopyFrom( part.body.GetDataSize(), part.body.GetConstBufferPtr() );
    message.attachments.push_back( attachment );
}

/*-------------------------------------------------------------------------*/

bool
CMimeParser::ParseMessage( const void* data, UInt32 size, CMailMessage& message, CORE::CString* error ) const
{GUCEF_TRACE;

    message = CMailMessage();
    CMimePart root;
    CORE::CString::StringVector warnings;
    if ( !ParseParts( data, size, root, &warnings, error ) )
        return false;

    message.rawSize = size;
    message.warnings = warnings;
    ApplyHeaders( root.headers, message );

    bool haveText = false;
    bool haveHtml = false;
    UInt32 extraTextCount = 0;
    MapPart( root, message, haveText, haveHtml, extraTextCount, true );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
