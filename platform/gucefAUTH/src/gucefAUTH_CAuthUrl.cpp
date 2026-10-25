/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthUrl.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CAuthUrl::MaxUrlBytes;
const UInt32 CAuthUrl::MaxQueryBytes;
const UInt32 CAuthUrl::MaxQueryParameters;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsUnreservedChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c || '~' == c;
}

/*-------------------------------------------------------------------------*/

static int
HexValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return 10 + ( c - 'a' );
    if ( c >= 'A' && c <= 'F' )
        return 10 + ( c - 'A' );
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The percent encoding of the framework (CORE::CUrlEncoder): everything but A-Z a-z 0-9 - . _ ~ as %XX of every byte of its UTF-8 */
static CORE::CString
EncodeComponent( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString encoded;
    CORE::CUrlEncoder::Encode( text, encoded );
    return encoded;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::Decode( const CORE::CString& text, bool form, CORE::CString& decoded )
{GUCEF_TRACE;

    decoded.Clear();
    UInt32 size = GetByteLength( text );
    const char* p = text.C_String();

    // The bytes are collected as bytes: a string that holds half of a multi byte character at some moment is not something to build on
    CORE::CDynamicBuffer bytes;
    bytes.SetDataSize( 0 );
    UInt32 runStart = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = p[ i ];
        if ( '%' == c || ( form && '+' == c ) )
        {
            if ( i > runStart )
                bytes.Append( p + runStart, i - runStart );
            char byte = ' ';
            if ( '%' == c )
            {
                if ( i + 2 >= size )
                    return false;
                int hi = HexValue( p[ i + 1 ] );
                int lo = HexValue( p[ i + 2 ] );
                if ( hi < 0 || lo < 0 )
                    return false;
                byte = (char) ( ( hi << 4 ) | lo );
                if ( '\0' == byte )
                    return false;
                i += 2;
            }
            bytes.Append( &byte, 1 );
            runStart = i + 1;
        }
    }
    if ( size > runStart )
        bytes.Append( p + runStart, size - runStart );
    decoded = MakeString( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthUrl::EncodeForm( const CORE::CString::StringMap& values )
{GUCEF_TRACE;

    CORE::CString form;
    CORE::CString::StringMap::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        if ( !form.IsNULLOrEmpty() )
            form += '&';
        form += EncodeComponent( (*i).first ) + "=" + EncodeComponent( (*i).second );
        ++i;
    }
    return form;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::ParseQuery( const CORE::CString& query, CORE::CString::StringMap& values, bool& duplicate )
{GUCEF_TRACE;

    values.clear();
    duplicate = false;
    UInt32 size = GetByteLength( query );
    if ( size > MaxQueryBytes )
        return false;
    const char* p = query.C_String();

    UInt32 start = 0;
    UInt32 count = 0;
    while ( start <= size )
    {
        UInt32 end = start;
        while ( end < size && '&' != p[ end ] )
            ++end;
        if ( end > start )
        {
            if ( ++count > MaxQueryParameters )
            {
                values.clear();
                return false;
            }
            UInt32 eq = start;
            while ( eq < end && '=' != p[ eq ] )
                ++eq;
            CORE::CString name, value;
            if ( !Decode( MakeString( p + start, eq - start ), true, name ) ||
                 ( eq < end && !Decode( MakeString( p + eq + 1, end - eq - 1 ), true, value ) ) )
            {
                values.clear();
                return false;
            }
            if ( values.find( name ) != values.end() )
                duplicate = true;
            else
                values[ name ] = value;
        }
        start = end + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthUrl::AppendQuery( const CORE::CString& url, const CORE::CString::StringMap& values )
{GUCEF_TRACE;

    if ( values.empty() )
        return url;
    CORE::CString result = url;
    result += url.HasChar( '?' ) >= 0 ? "&" : "?";
    result += EncodeForm( values );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::Split( const CORE::CString& url  ,
                 CORE::CString& scheme     ,
                 CORE::CString& host       ,
                 UInt16& port              ,
                 CORE::CString& path       ,
                 CORE::CString& query      )
{GUCEF_TRACE;

    scheme.Clear();
    host.Clear();
    path.Clear();
    query.Clear();
    port = 0;

    UInt32 size = GetByteLength( url );
    if ( 0 == size || size > MaxUrlBytes )
        return false;
    const char* p = url.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( (unsigned char) p[ i ] <= 0x20 || 0x7F == (unsigned char) p[ i ] || '\\' == p[ i ] || '#' == p[ i ] )
            return false;

    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return false;
    scheme = request.requestProtocol.Lowercase();
    if ( scheme != "http" && scheme != "https" )
        return false;
    if ( request.requestHost.IsNULLOrEmpty() || request.requestHost.HasChar( '@' ) >= 0 )
        return false;
    host = request.GetHostName().Lowercase();
    if ( host.IsNULLOrEmpty() )
        return false;
    port = request.GetPort();
    path = request.requestUri.IsNULLOrEmpty() ? CORE::CString( "/" ) : request.requestUri;
    query = request.requestUriParams;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::IsHttpsUrl( const CORE::CString& url )
{GUCEF_TRACE;

    return GetByteLength( url ) >= 8 && url.SubstrToIndex( 8, true ).Equals( "https://", false );
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::IsLoopbackHost( const CORE::CString& host )
{GUCEF_TRACE;

    CORE::CString lower = host.Lowercase();
    if ( lower == "localhost" || lower == "::1" || lower == "[::1]" )
        return true;
    // 127.a.b.c with every part a number
    if ( !lower.StartsWith( "127." ) )
        return false;
    CORE::CString::StringVector parts = lower.ParseElements( '.', true );
    if ( 4 != parts.size() )
        return false;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        UInt32 size = GetByteLength( parts[ i ] );
        if ( 0 == size || size > 3 )
            return false;
        const char* d = parts[ i ].C_String();
        UInt32 value = 0;
        for ( UInt32 c=0; c<size; ++c )
        {
            if ( d[ c ] < '0' || d[ c ] > '9' )
                return false;
            value = value * 10 + (UInt32) ( d[ c ] - '0' );
        }
        if ( value > 255 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::IsAcceptableEndpoint( const CORE::CString& url, bool insecureLoopbackAllowed )
{GUCEF_TRACE;

    CORE::CString scheme, host, path, query;
    UInt16 port = 0;
    if ( !Split( url, scheme, host, port, path, query ) )
        return false;
    if ( scheme == "https" )
        return true;
    return insecureLoopbackAllowed && scheme == "http" && IsLoopbackHost( host );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthUrl::GetOrigin( const CORE::CString& url )
{GUCEF_TRACE;

    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return CORE::CString::Empty;
    return request.requestProtocol.Lowercase() + "://" + request.requestHost;
}

/*-------------------------------------------------------------------------*/

bool
CAuthUrl::IsUnreservedText( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( text );
    const char* p = text.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( !IsUnreservedChar( p[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
