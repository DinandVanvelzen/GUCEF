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

#include <stdlib.h>

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CHttpProxyEnvironment.h"

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

/** reads a dotted IPv4 address, false for anything else */
static bool
ParseIPv4( const CORE::CString& text, UInt32& address )
{GUCEF_TRACE;

    CORE::CString::StringVector parts = text.ParseElements( '.', true );
    if ( 4 != parts.size() )
        return false;
    UInt32 result = 0;
    for ( size_t i=0; i<4; ++i )
    {
        const CORE::CString& part = parts[ i ];
        if ( part.IsNULLOrEmpty() || part.Length() > 3 )
            return false;
        for ( UInt32 c=0; c<part.Length(); ++c )
            if ( part[ c ] < '0' || part[ c ] > '9' )
                return false;
        int value = atoi( part.C_String() );
        if ( value > 255 )
            return false;
        result = ( result << 8 ) | (UInt32) value;
    }
    address = result;
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FirstEnv( const char* upper, const char* lower, bool allowUpper )
{GUCEF_TRACE;

    if ( allowUpper )
    {
        CORE::CString value = CORE::GetEnv( upper );
        if ( !value.IsNULLOrEmpty() )
            return value;
    }
    return CORE::GetEnv( lower );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CHttpProxyEnvironment::IsLoopback( const CORE::CString& host )
{GUCEF_TRACE;

    CORE::CString lower = host.Lowercase();
    if ( lower == "localhost" || lower == "::1" || lower == "[::1]" || lower.EndsWith( ".localhost" ) )
        return true;
    UInt32 address = 0;
    return ParseIPv4( lower, address ) && 127 == ( address >> 24 );
}

/*-------------------------------------------------------------------------*/

bool
CHttpProxyEnvironment::ParseProxyUrl( const CORE::CString& text, TProxy& proxy )
{GUCEF_TRACE;

    CORE::CString rest = text.Trim( true ).Trim( false );
    if ( rest.IsNULLOrEmpty() )
        return false;

    Int32 schemeEnd = rest.HasSubstr( "://" );
    if ( schemeEnd >= 0 )
    {
        CORE::CString scheme = rest.SubstrToIndex( (UInt32) schemeEnd, true ).Lowercase();
        if ( scheme != "http" )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "HttpProxyEnvironment: The proxy scheme \"" << scheme << "\" is not supported, only http:// proxies are, the proxy is ignored" );
            return false;
        }
        rest = rest.CutChars( (UInt32) schemeEnd + 3, true );
    }

    // the path and anything after it is not part of the proxy
    Int32 pathStart = rest.HasChar( '/' );
    if ( pathStart >= 0 )
        rest = rest.SubstrToIndex( (UInt32) pathStart, true );

    TProxy result;
    Int32 at = rest.HasChar( '@', false );
    if ( at >= 0 )
    {
        CORE::CString credentials = rest.SubstrToIndex( (UInt32) at, true );
        rest = rest.CutChars( (UInt32) at + 1, true );
        Int32 colon = credentials.HasChar( ':' );
        if ( colon >= 0 )
        {
            result.userName = credentials.SubstrToIndex( (UInt32) colon, true );
            result.password = credentials.CutChars( (UInt32) colon + 1, true );
        }
        else
            result.userName = credentials;
    }

    result.port = 1080;
    Int32 colon = rest.HasChar( ':', false );
    Int32 bracket = rest.HasChar( ']', false );
    if ( colon >= 0 && colon > bracket )
    {
        CORE::CString portText = rest.CutChars( (UInt32) colon + 1, true );
        if ( portText.IsNULLOrEmpty() || portText.Length() > 5 )
            return false;
        for ( UInt32 i=0; i<portText.Length(); ++i )
            if ( portText[ i ] < '0' || portText[ i ] > '9' )
                return false;
        int port = atoi( portText.C_String() );
        if ( port < 1 || port > 65535 )
            return false;
        result.port = (UInt16) port;
        rest = rest.SubstrToIndex( (UInt32) colon, true );
    }
    if ( rest.IsNULLOrEmpty() )
        return false;
    result.host = rest;
    proxy = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpProxyEnvironment::IsExcluded( const CORE::CString& host, UInt16 port, const CORE::CString& noProxyList )
{GUCEF_TRACE;

    if ( noProxyList.IsNULLOrEmpty() )
        return false;

    CORE::CString lowerHost = host.Lowercase();
    UInt32 hostAddress = 0;
    bool hostIsAddress = ParseIPv4( lowerHost, hostAddress );

    CORE::CString::StringVector entries = noProxyList.ParseElements( ',', false );
    for ( size_t i=0; i<entries.size(); ++i )
    {
        CORE::CString entry = entries[ i ].Trim( true ).Trim( false ).Lowercase();
        if ( entry.IsNULLOrEmpty() )
            continue;
        if ( entry == "*" )
            return true;

        // an optional port: host:port, not for an IPv6 literal
        UInt16 entryPort = 0;
        Int32 colon = entry.HasChar( ':', false );
        if ( colon >= 0 && entry.HasChar( ':', true ) == colon && entry.HasChar( ']', false ) < 0 )
        {
            entryPort = CORE::StringToUInt16( entry.CutChars( (UInt32) colon + 1, true ), 0 );
            entry = entry.SubstrToIndex( (UInt32) colon, true );
        }
        if ( 0 != entryPort && entryPort != port )
            continue;

        // an IPv4 network
        Int32 slash = entry.HasChar( '/' );
        if ( slash >= 0 )
        {
            UInt32 network = 0;
            int bits = atoi( entry.CutChars( (UInt32) slash + 1, true ).C_String() );
            if ( hostIsAddress && ParseIPv4( entry.SubstrToIndex( (UInt32) slash, true ), network ) && bits >= 0 && bits <= 32 )
            {
                UInt32 mask = 0 == bits ? 0 : ( 0xFFFFFFFFUL << ( 32 - bits ) );
                if ( ( hostAddress & mask ) == ( network & mask ) )
                    return true;
            }
            continue;
        }

        // a name: the host itself, or a subdomain of it
        if ( entry.StartsWith( "." ) )
            entry = entry.CutChars( 1, true );
        if ( lowerHost == entry )
            return true;
        if ( !hostIsAddress && lowerHost.EndsWith( "." + entry ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHttpProxyEnvironment::Lookup( const CORE::CString& scheme     ,
                               const CORE::CString& targetHost ,
                               UInt16 targetPort               ,
                               TProxy& proxy                   )
{GUCEF_TRACE;

    if ( targetHost.IsNULLOrEmpty() || IsLoopback( targetHost ) )
        return false;

    CORE::CString lowerScheme = scheme.Lowercase();
    CORE::CString value;
    if ( lowerScheme == "https" )
        value = FirstEnv( "HTTPS_PROXY", "https_proxy", true );
    else
    if ( lowerScheme == "http" )
    {
        // a CGI program would be handed a proxy by the client in the form of a header
        bool isCgi = !CORE::GetEnv( "REQUEST_METHOD" ).IsNULLOrEmpty();
        value = FirstEnv( "HTTP_PROXY", "http_proxy", !isCgi );
    }
    else
        return false;

    if ( value.IsNULLOrEmpty() )
        return false;

    if ( IsExcluded( targetHost, targetPort, FirstEnv( "NO_PROXY", "no_proxy", true ) ) )
        return false;

    TProxy result;
    if ( !ParseProxyUrl( value, result ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "HttpProxyEnvironment: The proxy of the environment for " << lowerScheme << " is not usable, going direct" );
        return false;
    }
    proxy = result;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHttpProxyEnvironment::GetBasicAuthorization( const TProxy& proxy )
{GUCEF_TRACE;

    if ( proxy.userName.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return "Basic " + CORE::Base64Encode( proxy.userName + ":" + proxy.password );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
