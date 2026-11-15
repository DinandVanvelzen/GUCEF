/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#include "gucefWEB_CWebViewHostAllowList.h"
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */

#include "gucefWEB_CWebViewCookie.h"

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

CWebViewCookie::CWebViewCookie( void )
    : name()
    , domain()
    , path( "/" )
    , expiresUtcSeconds( 0 )
    , secure( false )
    , httpOnly( false )
    , sameSite( WEBVIEWSAMESITE_UNSET )
    , m_value()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCookie::CWebViewCookie( const CWebViewCookie& src )
    : name( src.name )
    , domain( src.domain )
    , path( src.path )
    , expiresUtcSeconds( src.expiresUtcSeconds )
    , secure( src.secure )
    , httpOnly( src.httpOnly )
    , sameSite( src.sameSite )
    , m_value( src.m_value )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCookie::~CWebViewCookie()
{GUCEF_TRACE;

    CORE::SecureWipeString( m_value );
}

/*-------------------------------------------------------------------------*/

CWebViewCookie&
CWebViewCookie::operator=( const CWebViewCookie& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        name = src.name;
        domain = src.domain;
        path = src.path;
        expiresUtcSeconds = src.expiresUtcSeconds;
        secure = src.secure;
        httpOnly = src.httpOnly;
        sameSite = src.sameSite;
        CORE::SecureWipeString( m_value );
        m_value = src.m_value;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewCookie::GetValue( void ) const
{GUCEF_TRACE;

    return m_value;
}

/*-------------------------------------------------------------------------*/

void
CWebViewCookie::SetValue( const CString& value )
{GUCEF_TRACE;

    CORE::SecureWipeString( m_value );
    m_value = value;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCookie::IsSession( void ) const
{GUCEF_TRACE;

    return 0 == expiresUtcSeconds;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCookie::MatchesUrl( const CString& url ) const
{GUCEF_TRACE;

    CString scheme, host;
    UInt16 port = 0;
    bool hasUserInfo = false;
    if ( !CWebViewHostAllowList::SplitUrl( url, scheme, host, port, hasUserInfo ) )
        return false;
    if ( "http" != scheme && "https" != scheme )
        return false;
    if ( secure && "https" != scheme )
        return false;

    CString cookieDomain = domain.Lowercase();
    bool domainAndBelow = false;
    if ( cookieDomain.StartsWith( "." ) )
    {
        domainAndBelow = true;
        cookieDomain = cookieDomain.CutChars( 1, true );
    }
    if ( host != cookieDomain && !( domainAndBelow && host.EndsWith( "." + cookieDomain ) ) )
        return false;

    // the path of the address: after the authority up to the query or fragment
    CString urlPath = "/";
    const Int32 authorityStart = url.HasSubstr( "://" );
    if ( authorityStart >= 0 )
    {
        const Int32 pathStart = url.HasChar( '/', (UInt32) authorityStart + 3, true );
        if ( pathStart >= 0 )
        {
            urlPath = url.CutChars( (UInt32) pathStart, true );
            const Int32 query = urlPath.HasChar( '?' );
            if ( query >= 0 )
                urlPath = urlPath.SubstrToIndex( (UInt32) query, true );
            const Int32 fragment = urlPath.HasChar( '#' );
            if ( fragment >= 0 )
                urlPath = urlPath.SubstrToIndex( (UInt32) fragment, true );
        }
    }
    const CString cookiePath = path.IsNULLOrEmpty() ? CString( "/" ) : path;
    if ( urlPath == cookiePath )
        return true;
    if ( !urlPath.StartsWith( cookiePath ) )
        return false;
    return cookiePath.EndsWith( "/" ) || urlPath.C_String()[ cookiePath.ByteSize() - 1 ] == '/';
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCookie::MatchesFilter( const CString& urlFilter ) const
{GUCEF_TRACE;

    return urlFilter.IsNULLOrEmpty() || MatchesUrl( urlFilter );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCookie::IsSameCookie( const CWebViewCookie& other ) const
{GUCEF_TRACE;

    return name == other.name && domain.Lowercase() == other.domain.Lowercase() && path == other.path;
}

/*-------------------------------------------------------------------------*/

CString
CWebViewCookie::Describe( void ) const
{GUCEF_TRACE;

    CString text = name + " (" + domain + path;
    if ( secure )
        text += ", Secure";
    if ( httpOnly )
        text += ", HttpOnly";
    if ( WEBVIEWSAMESITE_UNSET != sameSite )
        text += CString( ", SameSite=" ) + WebViewSameSiteToString( sameSite );
    text += IsSession() ? ", session" : ", expires " + CORE::Int64ToString( expiresUtcSeconds );
    return text + ")";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
