/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#include "gucefWEB_CMcpStreamableHttpTransport.h"
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */

#include "gucefLabLink_PasswordHash.h"
#include "gucefLabLink_WebAssets.h"
#include "gucefLabLink_WebJson.h"
#include "gucefLabLink_WebApi.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* WebCsrfHeader = "X-LabLink-Csrf";

/** The longest password that is hashed: a longer one is refused before any work is done for it */
static const UInt32 MaxPasswordLength = 1024;

/** The text of every failed login, whatever the reason: it tells nobody whether the user exists */
static const char* LoginRefusedText = "The user name or password is not correct.";

/** The policy of the page: only what the hub itself serves, no frames, no inline script or style */
static const char* ContentSecurityPolicy = "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; "
                                           "frame-ancestors 'none'; form-action 'self'; base-uri 'none'";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsLoopback( const CORE::CString& address )
{GUCEF_TRACE;

    return address.StartsWith( "127." ) || address == "::1" || address.Equals( "localhost", false );
}

/*-------------------------------------------------------------------------*/

/** The media type of a Content-Type, lower case and without its parameters */
static CORE::CString
MediaTypeOf( const CORE::CString& contentType )
{GUCEF_TRACE;

    const Int32 semicolon = contentType.HasChar( ';' );
    return StripText( semicolon >= 0 ? contentType.SubstrToIndex( (UInt32) semicolon, true ) : contentType ).Lowercase();
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HeaderOf( const WEB::CHttpRequestData& request, const char* lowerCaseName )
{GUCEF_TRACE;

    WEB::CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.find( lowerCaseName );
    return i == request.otherHeaders.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
AddressKey( const CORE::CString& peer )
{GUCEF_TRACE;

    return "ip:" + peer;
}

/*-------------------------------------------------------------------------*/

/** Writes over the value of a field of a form that held a secret */
static void
WipeField( CORE::CString::StringMap& fields, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringMap::iterator i = fields.find( name );
    if ( i != fields.end() )
        CPasswordHash::Wipe( (*i).second );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FieldOf( const CORE::CString::StringMap& fields, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringMap::const_iterator i = fields.find( name );
    return i == fields.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLabWebApi::CLabWebApi( const SWebSettings& settings, const CAddressRanges& allowedRanges, ILabHubView& hub, CWebUserStore& users,
                        bool tls, const CORE::CString& token )
    : m_settings( settings )
    , m_allowedRanges( allowedRanges )
    , m_hub( hub )
    , m_users( users )
    , m_sessions( settings.maxSessions, settings.maxSessionsPerUser )
    , m_addressThrottle( settings.maxThrottleEntries, 5, 1000, settings.maxAddressDelayInSecs * 1000, 3600 * 1000 )
    , m_userThrottle( settings.maxThrottleEntries, 3, 1000, settings.maxUserDelayInSecs * 1000, 3600 * 1000 )
    , m_tls( tls )
    , m_hasToken( !token.IsNULLOrEmpty() )
    , m_dummyRecord( CPasswordHash::MakeDummyRecord( settings.passwordParams ) )
    , m_listenAddress()
    , m_port( settings.port )
    , m_nextSweepInMs( 0 )
{GUCEF_TRACE;

    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
    if ( m_hasToken )
        CORE::CSha256::Hash( token.C_String(), token.Length(), m_tokenDigest );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::SetListening( const CORE::CString& address, UInt16 port )
{GUCEF_TRACE;

    m_listenAddress = address;
    m_port = port;
}

/*-------------------------------------------------------------------------*/

bool CLabWebApi::IsTls( void ) const { return m_tls; }
CWebSessionTable& CLabWebApi::GetSessions( void ) { return m_sessions; }
CLoginThrottle& CLabWebApi::GetAddressThrottle( void ) { return m_addressThrottle; }
CLoginThrottle& CLabWebApi::GetUserThrottle( void ) { return m_userThrottle; }
const SWebSettings& CLabWebApi::GetSettings( void ) const { return m_settings; }

/*-------------------------------------------------------------------------*/

CORE::CString
CLabWebApi::GetCookieName( void ) const
{GUCEF_TRACE;

    // __Host-: a cookie the browser only takes over https, for this host and every path, never set by a sub domain
    return m_tls ? "__Host-lablink-session" : "lablink-session";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLabWebApi::MakeCookie( const CORE::CString& name, const CORE::CString& id, bool tls, UInt64 maxAgeInSecs, bool clear )
{GUCEF_TRACE;

    CORE::CString cookie = name + "=" + ( clear ? CORE::CString::Empty : id ) + "; Path=/; HttpOnly; SameSite=Strict";
    if ( tls )
        cookie += "; Secure";
    if ( clear )
        cookie += "; Max-Age=0";
    else
    if ( maxAgeInSecs > 0 )
        cookie += "; Max-Age=" + CORE::ToString( maxAgeInSecs );
    return cookie;
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::ParseForm( const CORE::CString& body, CORE::CString::StringMap& fields )
{GUCEF_TRACE;

    fields.clear();
    CORE::CString::StringVector pairs = body.ParseElements( '&', false );
    for ( size_t i=0; i<pairs.size(); ++i )
    {
        const Int32 equals = pairs[ i ].HasChar( '=' );
        const CORE::CString name = WEB::CUrlPathTemplate::PercentDecode( equals >= 0 ? pairs[ i ].SubstrToIndex( (UInt32) equals, true ) : pairs[ i ] );
        if ( name.IsNULLOrEmpty() || fields.find( name ) != fields.end() )
            continue;
        fields[ name ] = equals >= 0 ? WEB::CUrlPathTemplate::PercentDecode( pairs[ i ].CutChars( (UInt32) equals + 1, true ) ) : CORE::CString::Empty;
        CPasswordHash::Wipe( pairs[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLabWebApi::UserKey( const CORE::CString& user )
{GUCEF_TRACE;

    return "user:" + ( user.Length() > 64 ? user.SubstrToIndex( 64, true ) : user ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLabWebApi::BodyOf( const WEB::CHttpRequestData& request )
{GUCEF_TRACE;

    const UInt32 size = request.content.GetDataSize();
    return size > 0 ? CORE::CString( static_cast< const char* >( request.content.GetConstBufferPtr() ), size ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CLabWebApi::SRoute*
CLabWebApi::FindRoute( const CORE::CString& path, bool& pathKnown )
{GUCEF_TRACE;

    static const SRoute routes[] =
    {
        { "/", "GET", false, false, false, ROUTE_PAGE },
        { "/index.html", "GET", false, false, false, ROUTE_PAGE },
        { "/app.js", "GET", false, false, false, ROUTE_SCRIPT },
        { "/app.css", "GET", false, false, false, ROUTE_STYLE },
        { "/api/v1/login", "POST", false, false, false, ROUTE_LOGIN },
        { "/api/v1/logout", "POST", false, true, false, ROUTE_LOGOUT },
        { "/api/v1/session", "GET", false, true, false, ROUTE_SESSION },
        { "/api/v1/nodes", "GET", false, true, false, ROUTE_NODES },
        { "/api/v1/nodes/", "GET", true, true, false, ROUTE_NODE },
        { "/api/v1/endpoints", "GET", false, true, false, ROUTE_ENDPOINTS },
        { "/api/v1/users", "GET", false, true, true, ROUTE_USERS },
        { "/api/v1/users/add", "POST", false, true, true, ROUTE_USERS_ADD },
        { "/api/v1/users/disable", "POST", false, true, true, ROUTE_USERS_DISABLE },
        { "/api/v1/users/enable", "POST", false, true, true, ROUTE_USERS_ENABLE }
    };
    pathKnown = false;
    for ( size_t i=0; i<sizeof( routes ) / sizeof( routes[ 0 ] ); ++i )
    {
        const SRoute& route = routes[ i ];
        if ( route.prefix ? ( path.StartsWith( route.path ) && path.Length() > CORE::CString( route.path ).Length() ) : path == route.path )
            return &route;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::IsTokenValid( const CORE::CString& given ) const
{GUCEF_TRACE;

    // both sides are hashed first, so the comparison takes the same time whatever the length of what is given
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( given.C_String(), given.Length(), digest );
    const bool equal = CORE::CSha256::ConstantTimeEquals( digest, m_tokenDigest, sizeof( digest ) );
    ::memset( digest, 0, sizeof( digest ) );
    return equal && m_hasToken && !given.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::Authenticate( const WEB::CHttpRequestData& request, UInt64 nowInMs, SCaller& caller )
{GUCEF_TRACE;

    caller = SCaller();

    // the bearer token: a request that carries one is decided by it alone
    const CORE::CString authorization = HeaderOf( request, "authorization" );
    if ( !authorization.IsNULLOrEmpty() )
    {
        if ( authorization.Length() <= 7 || !authorization.SubstrToIndex( 7, true ).Equals( "bearer ", false ) )
            return false;
        if ( !IsTokenValid( StripText( authorization.CutChars( 7, true ) ) ) )
            return false;
        caller.role = WebRoleAdmin;
        caller.viaToken = true;
        return true;
    }

    // the session cookie: CHTTPServer keeps the Cookie field in transactionID
    const CORE::CString cookieName = GetCookieName();
    const CORE::CString cookieField = request.transactionID.IsNULLOrEmpty() ? HeaderOf( request, "cookie" ) : request.transactionID;
    CORE::CString::StringVector cookies = cookieField.ParseElements( ';', false );
    CORE::CString id;
    for ( size_t i=0; i<cookies.size(); ++i )
    {
        const CORE::CString cookie = StripText( cookies[ i ] );
        const Int32 equals = cookie.HasChar( '=' );
        if ( equals > 0 && cookie.SubstrToIndex( (UInt32) equals, true ) == cookieName )
        {
            id = cookie.CutChars( (UInt32) equals + 1, true );
            break;
        }
    }
    SWebSession session;
    if ( id.IsNULLOrEmpty() || !m_sessions.Use( id, nowInMs, session ) )
        return false;
    if ( session.viaToken )
    {
        caller.role = WebRoleAdmin;
        caller.viaToken = true;
        caller.sessionId = id;
        return true;
    }
    // the user as it is now: removed or disabled ends the session, the role is the current one
    SWebUser user;
    if ( !m_users.Find( session.user, user ) || user.disabled )
    {
        m_sessions.Remove( id );
        return false;
    }
    caller.user = user.name;
    caller.role = user.role;
    caller.sessionId = id;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::RefreshUsers( UInt64 nowInMs, bool now )
{GUCEF_TRACE;

    CORE::CString::StringVector changed;
    if ( !m_users.Refresh( nowInMs, now ? 0 : 1000, changed ) )
        return;
    for ( size_t i=0; i<changed.size(); ++i )
    {
        const UInt32 ended = m_sessions.RemoveUser( changed[ i ] );
        if ( ended > 0 )
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: " << ended << " session(s) of " << changed[ i ] << " ended: the user was changed" );
    }
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::OnTimer( UInt64 nowInMs )
{GUCEF_TRACE;

    // the users file is read again with the requests that need it, not here: an idle site reads nothing
    if ( nowInMs < m_nextSweepInMs )
        return;
    m_nextSweepInMs = nowInMs + 60000;
    m_sessions.Sweep( nowInMs );
    m_addressThrottle.Sweep( nowInMs );
    m_userThrottle.Sweep( nowInMs );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::Respond( WEB::CHttpResponseData& response, int statusCode, const CORE::CString& json ) const
{GUCEF_TRACE;

    response.statusCode = statusCode;
    response.contentType = "application/json; charset=utf-8";
    response.cacheability = "no-store";
    response.content.Clear();
    response.content.Append( json.C_String(), json.Length() );
    response.otherHeaders[ "X-Content-Type-Options" ] = "nosniff";
    response.otherHeaders[ "X-Frame-Options" ] = "DENY";
    response.otherHeaders[ "Referrer-Policy" ] = "no-referrer";
    response.otherHeaders[ "Content-Security-Policy" ] = ContentSecurityPolicy;
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::RespondError( WEB::CHttpResponseData& response, int statusCode, const char* text ) const
{GUCEF_TRACE;

    Respond( response, statusCode, CWebJson::Error( text ) );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::SessionAnswer( WEB::CHttpResponseData& response, const SCaller& caller ) const
{GUCEF_TRACE;

    Respond( response, 200, "{\"user\":" + CWebJson::Quote( caller.user ) + ",\"role\":" + CWebJson::Quote( caller.role ) +
                            ",\"viaToken\":" + ( caller.viaToken ? "true" : "false" ) + ",\"hub\":" + CWebJson::Quote( m_hub.GetHubName() ) +
                            ",\"tls\":" + ( m_tls ? "true" : "false" ) + "}" );
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::CheckHead( const WEB::CHttpRequestData& request, const CORE::CString* peer, UInt64 nowInMs, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    RefreshUsers( nowInMs, false );

    // 1. who: the ranges of the lab, as on the other endpoints
    if ( GUCEF_NULL != peer && !m_allowedRanges.Allows( *peer ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: refused " << *peer << ": not in the allowed ranges" );
        RespondError( response, 403, "This address may not use the site." );
        return false;
    }

    // 2. the Host on plain HTTP over loopback
    if ( !m_tls && ( IsLoopback( m_listenAddress ) || ( GUCEF_NULL != peer && IsLoopback( *peer ) ) ) &&
         !WEB::CMcpStreamableHttpTransport::IsHostAllowed( request.requestHost, CORE::CString::StringVector(), m_port ) )
    {
        RespondError( response, 403, "The Host of the request is not this machine." );
        return false;
    }

    // 3. what
    bool pathKnown = false;
    const SRoute* route = FindRoute( request.requestUri, pathKnown );
    if ( GUCEF_NULL == route )
    {
        RespondError( response, 404, "There is no such page." );
        return false;
    }
    if ( request.requestType != route->method )
    {
        RespondError( response, 405, "That method is not offered here." );
        response.allowedMethods.push_back( route->method );
        return false;
    }

    // 4. a change has to come from the page: a custom header needs the consent of the site, which it never gives
    if ( request.requestType == "POST" )
    {
        if ( HeaderOf( request, "x-lablink-csrf" ) != "1" )
        {
            RespondError( response, 403, "The request lacks the header X-LabLink-Csrf." );
            return false;
        }
        const UInt32 length = CORE::StringToUInt32( HeaderOf( request, "content-length" ), 0 );
        if ( length > 0 && MediaTypeOf( request.contentRepresentation ) != "application/x-www-form-urlencoded" )
        {
            RespondError( response, 415, "The body has to be application/x-www-form-urlencoded." );
            response.acceptedTypes.push_back( "application/x-www-form-urlencoded" );
            return false;
        }
    }

    // 5. a client that failed to log in waits
    if ( ROUTE_LOGIN == route->route )
    {
        const UInt32 waitInMs = m_addressThrottle.GetWaitInMs( AddressKey( GUCEF_NULL != peer ? *peer : CORE::CString::Empty ), nowInMs );
        if ( waitInMs > 0 )
        {
            RespondError( response, 429, "Too many failed logins: try again later." );
            response.otherHeaders[ "Retry-After" ] = CORE::ToString( (UInt32) ( ( waitInMs + 999 ) / 1000 ) );
            return false;
        }
    }

    // 6. credentials
    if ( route->needsLogin )
    {
        SCaller caller;
        if ( !Authenticate( request, nowInMs, caller ) )
        {
            RespondError( response, 401, "Log in first." );
            response.otherHeaders[ "WWW-Authenticate" ] = "Bearer";
            return false;
        }
        if ( route->needsAdmin && caller.role != WebRoleAdmin )
        {
            RespondError( response, 403, "That is for admins." );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::Handle( const WEB::CHttpRequestData& request, const CORE::CString* peer, UInt64 nowInMs, WEB::CHttpResponseData& response,
                    SPasswordJob& job )
{GUCEF_TRACE;

    if ( !CheckHead( request, peer, nowInMs, response ) )
        return true;
    bool pathKnown = false;
    const SRoute* route = FindRoute( request.requestUri, pathKnown );
    const CORE::CString peerAddress = GUCEF_NULL != peer ? *peer : CORE::CString::Empty;
    SCaller caller;
    if ( route->needsLogin && !Authenticate( request, nowInMs, caller ) )
    {
        RespondError( response, 401, "Log in first." );
        return true;
    }

    switch ( route->route )
    {
        case ROUTE_PAGE:
        case ROUTE_SCRIPT:
        case ROUTE_STYLE:
        {
            const char* name = ROUTE_SCRIPT == route->route ? "app.js" : ( ROUTE_STYLE == route->route ? "app.css" : "index.html" );
            UInt32 length = 0;
            const char* mediaType = GUCEF_NULL;
            const char* content = CWebAssets::Find( name, length, mediaType );
            if ( GUCEF_NULL == content )
            {
                RespondError( response, 404, "There is no such page." );
                return true;
            }
            Respond( response, 200, CORE::CString::Empty );
            response.content.Clear();
            response.content.Append( content, length );
            response.contentType = mediaType;
            response.cacheability = "no-cache";
            return true;
        }
        case ROUTE_LOGIN:
        {
            return StartLogin( request, peerAddress, nowInMs, response, job );
        }
        case ROUTE_LOGOUT:
        {
            if ( !caller.sessionId.IsNULLOrEmpty() )
            {
                m_sessions.Remove( caller.sessionId );
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: " << ( caller.viaToken ? CORE::CString( "the token" ) : caller.user ) << " logged out (" << peerAddress << ")" );
            }
            Respond( response, 200, "{\"ok\":true}" );
            response.otherHeaders[ "Set-Cookie" ] = MakeCookie( GetCookieName(), CORE::CString::Empty, m_tls, 0, true );
            return true;
        }
        case ROUTE_SESSION:
        {
            SessionAnswer( response, caller );
            return true;
        }
        case ROUTE_NODES:
        {
            GUCEF::vector< SHubNode > nodes;
            if ( !m_hub.GetNodes( nodes ) )
            {
                RespondError( response, 503, "The hub does not answer." );
                return true;
            }
            Respond( response, 200, CWebJson::Nodes( m_hub.GetHubName(), m_hub.GetHubTimeInUnixSecs(), nodes ) );
            return true;
        }
        case ROUTE_NODE:
        {
            const CORE::CString id = WEB::CUrlPathTemplate::PercentDecode( request.requestUri.CutChars( CORE::CString( "/api/v1/nodes/" ).Length(), true ) );
            SHubNode node;
            bool found = false;
            if ( id.IsNULLOrEmpty() || id.Length() > 64 || id.HasChar( '/' ) >= 0 )
            {
                RespondError( response, 404, "There is no such node." );
                return true;
            }
            if ( !m_hub.GetNode( id, node, found ) )
            {
                RespondError( response, 503, "The hub does not answer." );
                return true;
            }
            if ( !found )
            {
                RespondError( response, 404, "There is no such node." );
                return true;
            }
            Respond( response, 200, CWebJson::Node( node ) );
            return true;
        }
        case ROUTE_ENDPOINTS:
        {
            GUCEF::vector< SHubEndpoint > endpoints;
            if ( !m_hub.GetEndpoints( endpoints ) )
            {
                RespondError( response, 503, "The hub does not answer." );
                return true;
            }
            Respond( response, 200, CWebJson::Endpoints( m_hub.GetHubName(), m_hub.GetHubTimeInUnixSecs(), endpoints ) );
            return true;
        }
        case ROUTE_USERS:
        {
            GUCEF::vector< SWebUser > users;
            m_users.GetAll( users );
            Respond( response, 200, CWebJson::Users( users ) );
            return true;
        }
        case ROUTE_USERS_ADD:
        case ROUTE_USERS_DISABLE:
        case ROUTE_USERS_ENABLE:
        {
            return ChangeUser( route->route, request, caller, peerAddress, response, job );
        }
        default:
        {
            RespondError( response, 404, "There is no such page." );
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::StartLogin( const WEB::CHttpRequestData& request, const CORE::CString& peer, UInt64 nowInMs, WEB::CHttpResponseData& response,
                        SPasswordJob& job )
{GUCEF_TRACE;

    CORE::CString body = BodyOf( request );
    CORE::CString::StringMap fields;
    ParseForm( body, fields );
    CPasswordHash::Wipe( body );
    const CORE::CString stay = FieldOf( fields, "stay" );
    const bool persistent = stay == "1" || stay == "on" || stay == "true";

    // the token, for scripts and for the owner
    CORE::CString token = FieldOf( fields, "token" );
    WipeField( fields, "token" );
    if ( !token.IsNULLOrEmpty() )
    {
        const bool valid = IsTokenValid( token );
        CPasswordHash::Wipe( token );
        WipeField( fields, "password" );
        if ( !valid )
        {
            LoginFailed( CORE::CString::Empty, "the token (not valid)", peer, nowInMs, response );
            return true;
        }
        m_addressThrottle.OnSuccess( AddressKey( peer ) );
        LoggedIn( CORE::CString::Empty, true, persistent, peer, nowInMs, response );
        return true;
    }

    const CORE::CString name = StripText( FieldOf( fields, "username" ) );
    CORE::CString password = FieldOf( fields, "password" );
    WipeField( fields, "password" );
    if ( name.IsNULLOrEmpty() || password.IsNULLOrEmpty() )
    {
        CPasswordHash::Wipe( password );
        RespondError( response, 400, "Enter a user name and a password." );
        return true;
    }
    if ( password.Length() > MaxPasswordLength )
    {
        CPasswordHash::Wipe( password );
        RespondError( response, 400, "That password is longer than any password of this site." );
        return true;
    }
    const UInt32 waitInMs = m_userThrottle.GetWaitInMs( UserKey( name ), nowInMs );
    if ( waitInMs > 0 )
    {
        CPasswordHash::Wipe( password );
        RespondError( response, 429, "Too many failed logins: try again later." );
        response.otherHeaders[ "Retry-After" ] = CORE::ToString( (UInt32) ( ( waitInMs + 999 ) / 1000 ) );
        return true;
    }

    // a user that is not there, or disabled, is checked against a record of the same cost: the time of the answer
    // does not tell which users exist. The users file is read again first: a user disabled a moment ago stays out
    RefreshUsers( nowInMs, true );
    SWebUser user;
    const bool known = CWebUserStore::IsValidName( name ) && m_users.Find( name, user ) && !user.disabled && !user.record.IsNULLOrEmpty();
    job = SPasswordJob();
    job.kind = SPasswordJob::JOB_LOGIN;
    job.user = name;
    job.password = password;
    job.record = known ? user.record : m_dummyRecord;
    job.knownUser = known;
    job.persistent = persistent;
    job.peer = peer;
    job.params = m_settings.passwordParams;
    CPasswordHash::Wipe( password );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::RunPasswordJob( SPasswordJob& job )
{GUCEF_TRACE;

    job.verified = false;
    job.newRecord.Clear();
    if ( SPasswordJob::JOB_LOGIN == job.kind )
    {
        bool needsUpgrade = false;
        job.verified = CPasswordHash::Verify( job.password, job.record, job.params, needsUpgrade );
        if ( job.verified && needsUpgrade && job.knownUser )
            CPasswordHash::MakeRecord( job.password, CPasswordHash::Effective( job.params ), job.newRecord );
    }
    else
    {
        job.verified = CPasswordHash::MakeRecord( job.password, CPasswordHash::Effective( job.params ), job.newRecord );
    }
    CPasswordHash::Wipe( job.password );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::CompletePasswordJob( SPasswordJob& job, UInt64 nowInMs, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    CPasswordHash::Wipe( job.password );
    if ( SPasswordJob::JOB_ADD_USER == job.kind )
    {
        if ( !job.verified )
        {
            RespondError( response, 500, "The password could not be hashed." );
            return;
        }
        SWebUser user;
        user.name = job.user;
        user.role = job.role;
        user.createdInUnixSecs = NowInUnixSecs();
        user.record = job.newRecord;
        CORE::CString problem;
        if ( !m_users.Add( user, problem ) )
        {
            Respond( response, 409, CWebJson::Error( problem ) );
            return;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web audit: " << job.actor << " added the user " << job.user << " (" << job.role << ") from " << job.peer );
        Respond( response, 200, "{\"ok\":true}" );
        return;
    }

    // a login: the user has to be as it was when the job started, the password unchanged since
    SWebUser user;
    const bool ok = job.verified && job.knownUser && m_users.Find( job.user, user ) && !user.disabled && user.record == job.record;
    if ( !ok )
    {
        LoginFailed( UserKey( job.user ), job.knownUser ? "the user " + job.user : CORE::CString( "a user name that is not known" ), job.peer, nowInMs, response );
        return;
    }
    m_addressThrottle.OnSuccess( AddressKey( job.peer ) );
    m_userThrottle.OnSuccess( UserKey( job.user ) );
    if ( !job.newRecord.IsNULLOrEmpty() )
    {
        CORE::CString problem;
        if ( m_users.SetRecord( job.user, job.newRecord, problem, &job.record ) )
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: the password hash of " << job.user << " was made again with the current parameters" );
    }
    LoggedIn( job.user, false, job.persistent, job.peer, nowInMs, response );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::RefusePasswordJob( SPasswordJob& job, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    CPasswordHash::Wipe( job.password );
    RespondError( response, 503, "The site is busy: try again in a moment." );
    response.otherHeaders[ "Retry-After" ] = "2";
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::LoggedIn( const CORE::CString& user, bool viaToken, bool persistent, const CORE::CString& peer, UInt64 nowInMs,
                      WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    const UInt64 lifetimeInSecs = persistent ? m_settings.staySignedInLifetimeInSecs : m_settings.sessionLifetimeInSecs;
    CORE::CString id;
    if ( !m_sessions.Create( user, viaToken, persistent, nowInMs, m_settings.sessionIdleTimeoutInSecs * 1000, lifetimeInSecs * 1000, id ) )
    {
        RespondError( response, 500, "No session could be made." );
        return;
    }
    SCaller caller;
    caller.user = user;
    caller.viaToken = viaToken;
    caller.role = WebRoleAdmin;
    SWebUser record;
    if ( !viaToken && m_users.Find( user, record ) )
        caller.role = record.role;
    SessionAnswer( response, caller );
    response.otherHeaders[ "Set-Cookie" ] = MakeCookie( GetCookieName(), id, m_tls, persistent ? lifetimeInSecs : 0, false );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: login of " << ( viaToken ? CORE::CString( "the token" ) : user ) << " (" << caller.role << ") from " << peer );
}

/*-------------------------------------------------------------------------*/

void
CLabWebApi::LoginFailed( const CORE::CString& userKey, const CORE::CString& logName, const CORE::CString& peer, UInt64 nowInMs,
                         WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    m_addressThrottle.OnFailure( AddressKey( peer ), nowInMs );
    if ( !userKey.IsNULLOrEmpty() )
        m_userThrottle.OnFailure( userKey, nowInMs );
    // the name given is only logged when it is a user: a name that is not one may be a password typed in the wrong field
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: failed login for " << logName << " from " << peer << " (" <<
                       m_addressThrottle.GetFailures( AddressKey( peer ) ) << " failures from that address)" );
    RespondError( response, 401, LoginRefusedText );
}

/*-------------------------------------------------------------------------*/

bool
CLabWebApi::ChangeUser( ERoute route, const WEB::CHttpRequestData& request, const SCaller& caller, const CORE::CString& peer,
                        WEB::CHttpResponseData& response, SPasswordJob& job )
{GUCEF_TRACE;

    CORE::CString body = BodyOf( request );
    CORE::CString::StringMap fields;
    ParseForm( body, fields );
    CPasswordHash::Wipe( body );
    const CORE::CString name = StripText( FieldOf( fields, "name" ) );
    CORE::CString password = FieldOf( fields, "password" );
    WipeField( fields, "password" );
    const CORE::CString actor = caller.viaToken ? CORE::CString( "the token" ) : caller.user;

    if ( !CWebUserStore::IsValidName( name ) )
    {
        CPasswordHash::Wipe( password );
        RespondError( response, 400, "A user name is 1 to 64 letters, digits, '.', '_', '-' or '@'." );
        return true;
    }

    if ( ROUTE_USERS_ADD == route )
    {
        const CORE::CString role = StripText( FieldOf( fields, "role" ) );
        if ( !CWebUserStore::IsValidRole( role ) )
        {
            CPasswordHash::Wipe( password );
            RespondError( response, 400, "The role is admin or viewer." );
            return true;
        }
        if ( password.Length() < m_settings.minPasswordLength || password.Length() > MaxPasswordLength )
        {
            CPasswordHash::Wipe( password );
            Respond( response, 400, CWebJson::Error( "A password has " + CORE::ToString( m_settings.minPasswordLength ) + " to " +
                                                     CORE::ToString( MaxPasswordLength ) + " characters." ) );
            return true;
        }
        SWebUser existing;
        if ( m_users.Find( name, existing ) )
        {
            CPasswordHash::Wipe( password );
            RespondError( response, 409, "There is a user of that name already." );
            return true;
        }
        job = SPasswordJob();
        job.kind = SPasswordJob::JOB_ADD_USER;
        job.user = name;
        job.role = role;
        job.password = password;
        job.peer = peer;
        job.actor = actor;
        job.params = m_settings.passwordParams;
        CPasswordHash::Wipe( password );
        return false;
    }
    CPasswordHash::Wipe( password );

    const bool disable = ROUTE_USERS_DISABLE == route;
    if ( disable && !caller.viaToken && name == caller.user )
    {
        RespondError( response, 409, "You cannot disable your own account." );
        return true;
    }
    CORE::CString problem;
    if ( !m_users.SetDisabled( name, disable, problem ) )
    {
        Respond( response, 404, CWebJson::Error( problem ) );
        return true;
    }
    if ( disable )
        m_sessions.RemoveUser( name );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web audit: " << actor << ( disable ? " disabled" : " enabled" ) << " the user " << name << " from " << peer );
    Respond( response, 200, "{\"ok\":true}" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
