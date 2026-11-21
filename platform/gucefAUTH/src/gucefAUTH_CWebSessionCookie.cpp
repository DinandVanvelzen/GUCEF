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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONCOOKIE_H
#include "gucefAUTH_CWebSessionCookie.h"
#define GUCEF_AUTH_CWEBSESSIONCOOKIE_H
#endif /* GUCEF_AUTH_CWEBSESSIONCOOKIE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebSessionCookie::CWebSessionCookie( void )
    : name()
    , value()
    , domain()
    , path( "/" )
    , expires( -1.0 )
    , httpOnly( false )
    , secure( false )
    , session( true )
    , sameSite()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebSessionCookie::IsExpired( UInt64 nowUtcInMs ) const
{GUCEF_TRACE;

    if ( session || expires < 0.0 )
        return false;
    return expires * 1000.0 <= (Float64) nowUtcInMs;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionCookie::ToHttpCookie( WEB::CHttpCookie& cookie ) const
{GUCEF_TRACE;

    cookie.name = name;
    cookie.value = value.Reveal();
    CORE::CString lowerDomain = domain.Lowercase();
    cookie.hostOnly = !lowerDomain.StartsWith( "." );
    if ( !cookie.hostOnly )
        lowerDomain = lowerDomain.CutChars( 1, true, 0 );
    cookie.domain = lowerDomain;
    cookie.path = path.IsNULLOrEmpty() ? CORE::CString( "/" ) : path;
    cookie.isPersistent = !session && expires >= 0.0;
    if ( cookie.isPersistent )
        cookie.expires.FromUnixEpochBasedTicksInMillisecs( (UInt64) ( expires * 1000.0 ) );
    cookie.secure = secure;
    cookie.httpOnly = httpOnly;
    cookie.sameSite = sameSite;
}

/*-------------------------------------------------------------------------*/

CWebSessionCookie
CWebSessionCookie::FromChromeCookie( const WEB::SChromeCookie& cookie )
{GUCEF_TRACE;

    CWebSessionCookie result;
    result.name = cookie.name;
    result.value.Set( cookie.value );
    result.domain = cookie.domain;
    result.path = cookie.path;
    result.expires = cookie.session ? -1.0 : cookie.expires;
    result.httpOnly = cookie.httpOnly;
    result.secure = cookie.secure;
    result.session = cookie.session || cookie.expires < 0.0;
    result.sameSite = cookie.sameSite;
    return result;
}

/*-------------------------------------------------------------------------*/

CWebSessionCookie
CWebSessionCookie::FromHttpCookie( const WEB::CHttpCookie& cookie )
{GUCEF_TRACE;

    CWebSessionCookie result;
    result.name = cookie.name;
    result.value.Set( cookie.value );
    result.domain = cookie.hostOnly ? cookie.domain : "." + cookie.domain;
    result.path = cookie.path;
    result.session = !cookie.isPersistent;
    result.expires = cookie.isPersistent ? (Float64) cookie.expires.ToUnixEpochBasedTicksInMillisecs() / 1000.0 : -1.0;
    result.httpOnly = cookie.httpOnly;
    result.secure = cookie.secure;
    result.sameSite = cookie.sameSite;
    return result;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionCookie::WriteTo( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "name", name );
    node.SetAttribute( "value", value.Reveal() );
    node.SetAttribute( "domain", domain );
    node.SetAttribute( "path", path );
    node.SetAttribute( "expires", CORE::CVariant( expires ) );
    node.SetAttribute( "httpOnly", httpOnly );
    node.SetAttribute( "secure", secure );
    node.SetAttribute( "session", session );
    if ( !sameSite.IsNULLOrEmpty() )
        node.SetAttribute( "sameSite", sameSite );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionCookie::ReadFrom( const CORE::CDataNode& node )
{GUCEF_TRACE;

    name = node.GetAttributeValue( "name" ).AsString();
    value.Set( node.GetAttributeValue( "value" ).AsString() );
    domain = node.GetAttributeValue( "domain" ).AsString();
    path = node.GetAttributeValue( "path" ).AsString( "/" );
    expires = node.GetAttributeValue( "expires" ).AsFloat64( -1.0 );
    httpOnly = node.GetAttributeValue( "httpOnly" ).AsBool( false );
    secure = node.GetAttributeValue( "secure" ).AsBool( false );
    session = node.GetAttributeValue( "session" ).AsBool( expires < 0.0 );
    sameSite = node.GetAttributeValue( "sameSite" ).AsString();
    return !name.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
