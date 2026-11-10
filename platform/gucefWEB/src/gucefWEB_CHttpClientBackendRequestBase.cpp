/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CHttpClientBackendRequestBase.h"

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

static CString
JoinWithComma( const CHttpRequestData::TStringVector& values )
{
    CString result;
    for ( size_t i=0; i<values.size(); ++i )
    {
        if ( i > 0 )
            result += ", ";
        result += values[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** the length of the text without trailing line ends */
static UInt32
LengthWithoutLineEnd( const char* line, UInt32 length )
{
    while ( length > 0 && ( '\n' == line[ length-1 ] || '\r' == line[ length-1 ] ) )
        --length;
    return length;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpClientBackendRequestBase::CHttpClientBackendRequestBase( const CHttpRequestData& request          ,
                                                              const CHttpClientRequestOptions& options ,
                                                              CHttpResponseData& response              ,
                                                              CHttpClientTransferInfo& transferInfo    ,
                                                              CIHttpClientBackendObserver& observer    )
    : CIHttpClientBackendRequest()
    , m_request( request )
    , m_options( options )
    , m_response( response )
    , m_transferInfo( transferInfo )
    , m_observer( observer )
    , m_method( request.requestType.IsNULLOrEmpty() ? CString( "GET" ) : request.requestType.Uppercase() )
    , m_isHeadRequest( false )
    , m_connected( false )
    , m_finished( false )
    , m_headersComplete( false )
    , m_discardBody( false )
    , m_hasContentLength( false )
    , m_chunked( false )
    , m_contentLength( 0 )
    , m_responseBodySize( 0 )
    , m_lastHeaderName()
    , m_bodySource( options.requestBody )
    , m_bodySizeKnown( true )
    , m_bodySize( 0 )
    , m_bodyOffset( 0 )
    , m_bodyStartPos( 0 )
{GUCEF_TRACE;

    m_isHeadRequest = m_method == "HEAD";

    if ( !m_bodySource.IsNULL() )
    {
        // A source which does not know its size, such as a stream, is sent chunked
        UInt64 size = m_bodySource->GetSize();
        UInt64 pos = m_bodySource->Tell();
        m_bodyStartPos = pos;
        m_bodySizeKnown = size > 0 && size >= pos;
        m_bodySize = m_bodySizeKnown ? size - pos : 0;
    }
    else
    {
        m_bodySize = request.content.GetDataSize();
    }
}

/*-------------------------------------------------------------------------*/

CHttpClientBackendRequestBase::~CHttpClientBackendRequestBase()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsRedirectStatusCode( int statusCode )
{GUCEF_TRACE;

    return 301 == statusCode || 302 == statusCode || 303 == statusCode || 307 == statusCode || 308 == statusCode;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsFinished( void ) const
{GUCEF_TRACE;

    return m_finished;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::Cancel( void )
{GUCEF_TRACE;

    if ( !m_finished )
    {
        m_transferInfo.wasCancelled = true;
        Finish( false, "The request was cancelled" );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::Finish( bool success, const CString& errorDescription, Int32 errorCode )
{GUCEF_TRACE;

    if ( m_finished )
        return;
    m_finished = true;

    m_transferInfo.success = success;
    if ( !success )
    {
        m_transferInfo.errorCode = errorCode;
        m_transferInfo.errorDescription = errorDescription;
    }
    OnFinish();
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnFinish( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CHttpClientBackendRequestBase::GetRequestMethod( void ) const
{GUCEF_TRACE;

    return m_method;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsHeadRequest( void ) const
{GUCEF_TRACE;

    return m_isHeadRequest;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::HasRequestBody( void ) const
{GUCEF_TRACE;

    return !m_bodySource.IsNULL() || m_request.content.GetDataSize() > 0 ||
           m_method == "POST" || m_method == "PUT" || m_method == "PATCH";
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsRequestBodyStreamed( void ) const
{GUCEF_TRACE;

    return !m_bodySource.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsRequestBodySizeKnown( void ) const
{GUCEF_TRACE;

    return m_bodySizeKnown;
}

/*-------------------------------------------------------------------------*/

UInt64
CHttpClientBackendRequestBase::GetRequestBodySize( void ) const
{GUCEF_TRACE;

    return m_bodySize;
}

/*-------------------------------------------------------------------------*/

UInt32
CHttpClientBackendRequestBase::ReadRequestBody( void* dest, UInt32 maxBytes )
{GUCEF_TRACE;

    UInt32 toRead = maxBytes;
    if ( m_bodySizeKnown )
    {
        UInt64 left = m_bodySize - m_bodyOffset;
        if ( left < toRead )
            toRead = (UInt32) left;
    }
    if ( 0 == toRead )
        return 0;

    UInt32 bytesRead = 0;
    if ( !m_bodySource.IsNULL() )
    {
        // the pointer is copied since its const form only gives access to a const object
        CORE::IOAccessPtr source = m_bodySource;
        bytesRead = source->Read( dest, 1, toRead );
    }
    else
    {
        memcpy( dest, m_request.content.AsConstTypePtr< char >() + m_bodyOffset, toRead );
        bytesRead = toRead;
    }
    m_bodyOffset += bytesRead;
    return bytesRead;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::RewindRequestBody( void )
{GUCEF_TRACE;

    if ( !m_bodySource.IsNULL() )
    {
        // the pointer is copied since its const form only gives access to a const object
        CORE::IOAccessPtr source = m_bodySource;
        if ( 0 != source->Setpos( (UInt32) m_bodyStartPos ) )
            return false;
    }
    m_bodyOffset = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CHttpClientBackendRequestBase::GetRequestBodyBytesRead( void ) const
{GUCEF_TRACE;

    return m_bodyOffset;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::GetRequestHeaderFields( CString::StringVector& fields ) const
{GUCEF_TRACE;

    fields.reserve( fields.size() + m_request.otherHeaders.size() + 4 );

    bool hasUserAgent = false;
    CHttpRequestData::TStringMap::const_iterator i = m_request.otherHeaders.begin();
    while ( i != m_request.otherHeaders.end() )
    {
        const CString& name = (*i).first;
        if ( !name.Equals( "Host", false ) && !name.Equals( "Connection", false ) &&
             !name.Equals( "Content-Length", false ) && !name.Equals( "Transfer-Encoding", false ) )
        {
            if ( name.Equals( "User-Agent", false ) )
                hasUserAgent = true;
            fields.push_back( name + ": " + (*i).second );
        }
        ++i;
    }
    if ( !hasUserAgent && !m_options.userAgent.IsNULLOrEmpty() )
        fields.push_back( "User-Agent: " + m_options.userAgent );

    fields.push_back( "Accept: " + ( m_request.resourceRepresentations.empty() ? CString( "*/*" ) : JoinWithComma( m_request.resourceRepresentations ) ) );
    if ( !m_request.encodingRepresentations.empty() )
        fields.push_back( "Accept-Encoding: " + JoinWithComma( m_request.encodingRepresentations ) );
    if ( HasRequestBody() && !m_request.contentRepresentation.IsNULLOrEmpty() )
        fields.push_back( "Content-Type: " + m_request.contentRepresentation );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnConnected( void )
{GUCEF_TRACE;

    if ( !m_connected )
    {
        m_connected = true;
        m_observer.OnHttpClientConnected();
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnDataSent( UInt32 bytesSent )
{GUCEF_TRACE;

    if ( bytesSent > 0 )
    {
        m_transferInfo.bytesSent += bytesSent;
        m_observer.OnHttpClientDataSent( bytesSent );
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnDataReceived( UInt32 bytesReceived )
{GUCEF_TRACE;

    m_transferInfo.bytesReceived += bytesReceived;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::OnResponseStatusLine( const char* line, UInt32 length )
{GUCEF_TRACE;

    // HTTP/x.y SSS reason
    length = LengthWithoutLineEnd( line, length );
    if ( length < 10 || 0 != memcmp( line, "HTTP/", 5 ) )
        return false;
    const char* space = (const char*) memchr( line, ' ', length );
    if ( GUCEF_NULL == space || space + 4 > line + length )
        return false;

    m_response.Clear();
    m_response.statusCode = atoi( space + 1 );
    m_headersComplete = false;
    m_discardBody = false;
    m_hasContentLength = false;
    m_chunked = false;
    m_contentLength = 0;
    m_responseBodySize = 0;
    m_lastHeaderName.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnResponseHeaderLine( const char* line, UInt32 length )
{GUCEF_TRACE;

    length = LengthWithoutLineEnd( line, length );
    if ( 0 == length )
        return;

    if ( ' ' == line[ 0 ] || '\t' == line[ 0 ] )
    {
        // obsolete line folding: the line continues the value of the previous field
        if ( m_lastHeaderName.IsNULLOrEmpty() )
            return;
        UInt32 start = 0;
        while ( start < length && ( ' ' == line[ start ] || '\t' == line[ start ] ) )
            ++start;
        CString& value = m_response.otherHeaders[ m_lastHeaderName ];
        value += ' ';
        value.Append( line + start, length - start );
        return;
    }

    const char* colon = (const char*) memchr( line, ':', length );
    if ( GUCEF_NULL == colon || colon == line )
        return;

    UInt32 valueStart = (UInt32) ( colon - line ) + 1;
    while ( valueStart < length && ( ' ' == line[ valueStart ] || '\t' == line[ valueStart ] ) )
        ++valueStart;
    UInt32 valueEnd = length;
    while ( valueEnd > valueStart && ( ' ' == line[ valueEnd-1 ] || '\t' == line[ valueEnd-1 ] ) )
        --valueEnd;

    OnResponseHeader( CString( line, (UInt32) ( colon - line ) ), CString( line + valueStart, valueEnd - valueStart ) );
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnResponseHeader( const CString& name, const CString& value )
{GUCEF_TRACE;

    m_observer.OnHttpClientResponseHeader( name, value );

    CString lowerName = name.Lowercase();
    if ( lowerName == "content-length" )
    {
        m_hasContentLength = true;
        m_contentLength = CORE::StringToUInt64( value );
    }
    else
    if ( lowerName == "transfer-encoding" )
        m_chunked = value.Lowercase().HasSubstr( "chunked", true ) >= 0;
    else
    if ( lowerName == "content-type" )
        m_response.contentType = value;
    else
    if ( lowerName == "content-encoding" )
        m_response.contentEncoding = value;
    else
    if ( lowerName == "etag" )
        m_response.eTag = value;
    else
    if ( lowerName == "last-modified" )
        m_response.lastModified = value;
    else
    if ( lowerName == "location" )
        m_response.location = value;
    else
    if ( lowerName == "cache-control" )
        m_response.cacheability = value;
    else
    if ( lowerName == "allow" )
    {
        CString::StringVector methods = value.ParseElements( ',', false );
        for ( size_t m=0; m<methods.size(); ++m )
            m_response.allowedMethods.push_back( methods[ m ].Trim( true ).Trim( false ) );
    }
    else
    if ( lowerName == "connection" )
        m_response.keepConnectionsAlive = value.Equals( "keep-alive", false );

    // All fields are available by their name as received, repeated fields are combined.
    // Set-Cookie values can contain commas so they are kept apart by line ends.
    CHttpResponseData::TStringMap::iterator existing = m_response.otherHeaders.find( name );
    if ( existing == m_response.otherHeaders.end() )
        m_response.otherHeaders[ name ] = value;
    else
        (*existing).second += ( lowerName == "set-cookie" ? "\n" : ", " ) + value;
    m_lastHeaderName = name;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::OnResponseHeadersComplete( void )
{GUCEF_TRACE;

    m_headersComplete = true;
    m_discardBody = m_options.followRedirects && IsRedirectStatusCode( m_response.statusCode ) && !m_response.location.IsNULLOrEmpty();
    m_observer.OnHttpClientResponseHeadersComplete( m_response );

    // A body that is announced to be larger than the limit is not read at all. A HEAD request, 204 and 304 have no body
    // whatever their Content-Length says
    if ( m_options.maxResponseBodySize > 0 && m_hasContentLength && m_contentLength > m_options.maxResponseBodySize &&
         !m_discardBody && !m_isHeadRequest && 204 != m_response.statusCode && 304 != m_response.statusCode )
    {
        FinishResponseBodyTooLarge( m_contentLength );
    }
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::OnResponseBody( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( m_discardBody || 0 == dataSize || m_finished )
        return !m_finished;

    // The block that would take the body over the limit is not taken: nothing more is read
    if ( m_options.maxResponseBodySize > 0 && m_responseBodySize + dataSize > m_options.maxResponseBodySize )
    {
        FinishResponseBodyTooLarge( m_responseBodySize + dataSize );
        return false;
    }

    if ( !m_options.responseBodySink.IsNULL() )
    {
        // the pointer is copied since its const form only gives access to a const object
        CORE::IOAccessPtr sink = m_options.responseBodySink;
        if ( dataSize != sink->Write( data, 1, dataSize ) )
        {
            Finish( false, "Unable to write the response body to the sink" );
            return false;
        }
    }
    else
    {
        m_response.content.Append( data, dataSize, true );
    }
    m_responseBodySize += dataSize;
    m_observer.OnHttpClientBodyData( data, dataSize );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendRequestBase::FinishResponseBodyTooLarge( UInt64 bodySize )
{GUCEF_TRACE;

    m_transferInfo.responseBodyTooLarge = true;
    m_transferInfo.errorClass = HTTPERRORCLASS_OTHER;
    Finish( false, "The response body of at least " + CORE::ToString( bodySize ) + " bytes is larger than the limit of " +
                   CORE::ToString( m_options.maxResponseBodySize ) + " bytes" );
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::HaveResponseHeadersCompleted( void ) const
{GUCEF_TRACE;

    return m_headersComplete;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::HasResponseContentLength( void ) const
{GUCEF_TRACE;

    return m_hasContentLength;
}

/*-------------------------------------------------------------------------*/

UInt64
CHttpClientBackendRequestBase::GetResponseContentLength( void ) const
{GUCEF_TRACE;

    return m_contentLength;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsResponseChunked( void ) const
{GUCEF_TRACE;

    return m_chunked;
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendRequestBase::IsResponseBodyDiscarded( void ) const
{GUCEF_TRACE;

    return m_discardBody;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
