/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefWEB_CHttpResponseData.h"

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpResponseData::CHttpResponseData( void )
    : CORE::CICloneable()         
    , allowedMethods()
    , cacheability()
    , eTag()
    , lastModified()
    , content()
    , contentEncoding()
    , statusCode()
    , contentType()
    , location()
    , acceptedTypes()
    , keepConnectionsAlive( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpResponseData::CHttpResponseData( const CHttpResponseData& src )
    : CORE::CICloneable( src )         
    , allowedMethods( src.allowedMethods )
    , cacheability( src.cacheability )
    , eTag( src.eTag )
    , lastModified( src.lastModified )
    , content( src.content )
    , contentEncoding( src.contentEncoding )
    , statusCode( src.statusCode )
    , contentType( src.contentType )
    , location( src.location )
    , acceptedTypes( src.acceptedTypes )
    , keepConnectionsAlive( src.keepConnectionsAlive )
    , otherHeaders( src.otherHeaders )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpResponseData::~CHttpResponseData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHttpResponseData::Clear( void )
{GUCEF_TRACE;

    allowedMethods.clear();
    cacheability.Clear();
    eTag.Clear();
    lastModified.Clear();
    content.SetDataSize( 0 );
    contentEncoding.Clear();
    statusCode = 0;
    contentType.Clear();
    location.Clear();
    acceptedTypes.clear();
    keepConnectionsAlive = true;
    otherHeaders.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CHttpResponseData::Clone( void ) const 
{GUCEF_TRACE;

    return GUCEF_NEW CHttpResponseData( *this );
}

/*-------------------------------------------------------------------------*/

/** A header field name is a token (RFC 9110 5.1, 5.6.2) */
static bool
IsHeaderFieldName( const CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    const char* text = name.C_String();
    const UInt32 size = name.ByteSize() - 1;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) )
            continue;
        if ( GUCEF_NULL == memchr( "!#$%&'*+-.^_`|~", (int) c, 15 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A header field value must not hold what ends a line or the message: CR, LF and NUL */
static bool
IsSafeHeaderFieldValue( const CString& value )
{GUCEF_TRACE;

    if ( value.IsNULLOrEmpty() )
        return true;
    const char* text = value.C_String();
    const UInt32 size = value.ByteSize() - 1;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\r' == text[ i ] || '\n' == text[ i ] || '\0' == text[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
AreSafeHeaderFieldValues( const CString::StringVector& values )
{GUCEF_TRACE;

    for ( CString::StringVector::const_iterator i = values.begin(); i != values.end(); ++i )
        if ( !IsSafeHeaderFieldValue( *i ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static CString
JoinListValue( const CString::StringVector& values )
{GUCEF_TRACE;

    CString joined;
    for ( CString::StringVector::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        if ( !joined.IsNULLOrEmpty() )
            joined += ", ";
        joined += (*i);
    }
    return joined;
}

/*-------------------------------------------------------------------------*/

const char*
CHttpResponseData::GetReasonPhrase( int statusCode )
{GUCEF_TRACE;

    switch ( statusCode )
    {
        case 100: return "Continue";
        case 101: return "Switching Protocols";
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 206: return "Partial Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Content Too Large";
        case 415: return "Unsupported Media Type";
        case 416: return "Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 426: return "Upgrade Required";
        case 429: return "Too Many Requests";
        case 431: return "Request Header Fields Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        default: return "";
    }
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::HasSafeHeaderFields( void ) const
{GUCEF_TRACE;

    if ( !IsSafeHeaderFieldValue( cacheability ) || !IsSafeHeaderFieldValue( eTag ) || !IsSafeHeaderFieldValue( lastModified ) ||
         !IsSafeHeaderFieldValue( contentEncoding ) || !IsSafeHeaderFieldValue( contentType ) || !IsSafeHeaderFieldValue( location ) ||
         !AreSafeHeaderFieldValues( allowedMethods ) || !AreSafeHeaderFieldValues( acceptedTypes ) )
        return false;
    for ( TStringMap::const_iterator i = otherHeaders.begin(); i != otherHeaders.end(); ++i )
    {
        if ( !IsHeaderFieldName( (*i).first ) || !IsSafeHeaderFieldValue( (*i).second ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::SerializeHead( CString& response, int connectionMode ) const
{GUCEF_TRACE;

    const bool streamed = ( 2 == connectionMode );

    if ( !HasSafeHeaderFields() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "HttpResponseData(" << CORE::PointerToString( this ) << "): A header field of the response has a name that is not a token or a value with a line break, the response is not written" );
        return false;
    }

    // the space after the status code is part of the status line also when there is no reason phrase
    response = "HTTP/1.1 " + CORE::Int32ToString( statusCode ) + " " + GetReasonPhrase( statusCode ) + "\r\nServer: gucefWEB/" + CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( false, true ) + "\r\n";

    // A Connection header explicitly provided by the caller takes precedence over the one we derive,
    // except for a stream, whose body ends with the connection
    if ( streamed || 0 == connectionMode )
    {
        response += "Connection: close\r\n";
    }
    else
    if ( 1 == connectionMode )
    {
        response += "Connection: keep-alive\r\n";
    }
    else
    if ( otherHeaders.find( "Connection" ) == otherHeaders.end() )
    {
        if ( keepConnectionsAlive )
            response += "Connection: keep-alive\r\n";
        else
            response += "Connection: close\r\n";
    }

    if ( !location.IsNULLOrEmpty() )
    {
        response += "Content-Location: " + location + "\r\n";
    }
    if ( !eTag.IsNULLOrEmpty() )
    {
        response += "ETag: " + eTag + "\r\n";
    }
    if ( !cacheability.IsNULLOrEmpty() )
    {
        response += "Cache-Control: " + cacheability + "\r\n";
    }
    if ( !lastModified.IsNULLOrEmpty() )
    {
        response += "Last-Modified: " + lastModified + "\r\n";
    }
    if ( content.GetDataSize() > 0 || ( streamed && !contentType.IsNULLOrEmpty() ) )
    {
        if ( !contentEncoding.IsNULLOrEmpty() )
            response += "Content-Encoding: " + contentEncoding + "\r\n";
        response += "Content-Type: " + contentType + "\r\n";
    }
    if ( 101 != statusCode && !streamed )
        response += "Content-Length: " + CORE::UInt32ToString( content.GetDataSize() ) + "\r\n";

    // You cannot send back accept types to the client using a standard HTTP header since
    // it is a HTTP request only header. We don't want the client to aimlessly retry different
    // representations either on PUT/POST so we do want to inform the client of the accepted types.
    // The client will need to have support for this operation to take advantage of it since it is a custom HTTP header
    if ( statusCode == 415 && !acceptedTypes.empty() )
    {
        response += "Accept: " + JoinListValue( acceptedTypes ) + "\r\n";
    }

    // If the operation was and a subset of allowed operations where specified
    // we will send those to the client.
    if ( statusCode == 405 && !allowedMethods.empty() )
    {
        response += "Allow: " + JoinListValue( allowedMethods ) + "\r\n";
    }

    // Add any other headers
    TStringMap::const_iterator i = otherHeaders.begin();
    while ( i != otherHeaders.end() )
    {
        if ( connectionMode < 0 || !(*i).first.Equals( "Connection", false ) )
            response += (*i).first + ": " + (*i).second + "\r\n";
        ++i;
    }

    // Add the end of HTTP header delimiter
    response += "\r\n";

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "HttpResponseData(" << CORE::PointerToString( this ) << "): Finished building response header: " << CHttpLogRedaction::RedactHeaders( response ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::Serialize( CORE::CDynamicBuffer& outputBuffer ) const
{GUCEF_TRACE;

    return SerializeWithConnectionMode( outputBuffer, -1 );
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::Serialize( CORE::CDynamicBuffer& outputBuffer, bool keepConnectionAlive ) const
{GUCEF_TRACE;

    return SerializeWithConnectionMode( outputBuffer, keepConnectionAlive ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::SerializeWithConnectionMode( CORE::CDynamicBuffer& outputBuffer, int connectionMode ) const
{GUCEF_TRACE;

    CString response;
    if ( !SerializeHead( response, connectionMode ) )
    {
        outputBuffer.SetDataSize( 0 );
        return false;
    }

    // Copy the HTTP header into the buffer
    outputBuffer.CopyFrom( ( response.ByteSize() > 0 ? response.ByteSize() - 1 : 0 ), response.C_String() );

    // Copy the HTTP message body into the buffer
    if ( content.GetDataSize() > 0 )
    {
        outputBuffer.CopyFrom( ( response.ByteSize() > 0 ? response.ByteSize() - 1 : 0 ), content.GetDataSize(), content.GetConstBufferPtr() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpResponseData::SerializeStreamHead( CORE::CDynamicBuffer& outputBuffer ) const
{GUCEF_TRACE;

    CString response;
    if ( !SerializeHead( response, 2 ) )
    {
        outputBuffer.SetDataSize( 0 );
        return false;
    }
    outputBuffer.CopyFrom( ( response.ByteSize() > 0 ? response.ByteSize() - 1 : 0 ), response.C_String() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
