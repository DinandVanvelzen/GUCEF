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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CHttpCookieJar.h"

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
TrimWhitespace( const CString& str )
{GUCEF_TRACE;

    return str.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

static bool
IsIpAddress( const CString& host )
{GUCEF_TRACE;

    if ( host.HasChar( ':' ) >= 0 )
        return true;    // IPv6
    for ( UInt32 i=0; i<host.Length(); ++i )
    {
        UInt32 c = host[ i ];
        if ( ( c < '0' || c > '9' ) && c != '.' )
            return false;
    }
    return host.Length() > 0;
}

/*-------------------------------------------------------------------------*/

static bool
DomainMatches( const CString& lowercaseHost, const CString& domain )
{GUCEF_TRACE;

    if ( lowercaseHost == domain )
        return true;
    if ( lowercaseHost.Length() <= domain.Length() || IsIpAddress( lowercaseHost ) )
        return false;
    UInt32 offset = lowercaseHost.Length() - domain.Length();
    return '.' == lowercaseHost[ offset-1 ] && lowercaseHost.CutChars( offset, true ) == domain;
}

/*-------------------------------------------------------------------------*/

static bool
PathMatches( const CString& requestPath, const CString& cookiePath )
{GUCEF_TRACE;

    if ( requestPath == cookiePath )
        return true;
    if ( requestPath.Length() <= cookiePath.Length() || 0 != requestPath.HasSubstr( cookiePath, true ) )
        return false;
    return '/' == cookiePath[ cookiePath.Length()-1 ] || '/' == requestPath[ cookiePath.Length() ];
}

/*-------------------------------------------------------------------------*/

/** the default path of a cookie as per RFC 6265 section 5.1.4 */
static CString
DefaultCookiePath( const CString& requestPath )
{GUCEF_TRACE;

    if ( requestPath.IsNULLOrEmpty() || '/' != requestPath[ 0 ] )
        return "/";
    Int32 lastSlash = requestPath.HasChar( '/', false );
    if ( lastSlash <= 0 )
        return "/";
    return requestPath.SubstrToIndex( (UInt32) lastSlash, true );
}

/*-------------------------------------------------------------------------*/

/**
 *  Parses a cookie date, RFC 6265 section 5.1.1: the tokens are found by their form rather than their position
 *  which handles the date formats of RFC 1123, RFC 850 and asctime
 */
static bool
ParseCookieDate( const CString& dateStr, CORE::CDateTime& dateTime )
{GUCEF_TRACE;

    static const char* months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };

    bool foundTime = false, foundDay = false, foundMonth = false, foundYear = false;
    int hours = 0, minutes = 0, seconds = 0, day = 0, month = 0, year = 0;

    CString::StringVector tokens = dateStr.Lowercase().ReplaceChar( ',', ' ' ).ReplaceChar( '-', ' ' ).ParseElements( ' ', false );
    for ( CString::StringVector::iterator i = tokens.begin(); i != tokens.end(); ++i )
    {
        const CString& token = (*i);
        int a = 0, b = 0, c = 0;
        if ( !foundTime && 3 == sscanf( token.C_String(), "%d:%d:%d", &a, &b, &c ) )
        {
            hours = a; minutes = b; seconds = c; foundTime = true;
            continue;
        }
        if ( !foundMonth && token.Length() >= 3 )
        {
            CString prefix = token.SubstrToIndex( 3, true );
            bool matched = false;
            for ( int m=0; m<12; ++m )
            {
                if ( prefix == months[ m ] )
                {
                    month = m+1; foundMonth = true; matched = true;
                    break;
                }
            }
            if ( matched )
                continue;
        }
        bool numeric = token.Length() > 0;
        for ( UInt32 n=0; n<token.Length() && numeric; ++n )
            numeric = token[ n ] >= '0' && token[ n ] <= '9';
        if ( !numeric )
            continue;
        if ( !foundDay && token.Length() <= 2 )
        {
            day = CORE::StringToInt32( token ); foundDay = true;
            continue;
        }
        if ( !foundYear && ( token.Length() == 2 || token.Length() == 4 ) )
        {
            year = CORE::StringToInt32( token ); foundYear = true;
            if ( year >= 70 && year <= 99 ) year += 1900;
            else if ( year < 70 ) year += 2000;
        }
    }

    if ( !foundTime || !foundDay || !foundMonth || !foundYear || day < 1 || day > 31 || year < 1601 || hours > 23 || minutes > 59 || seconds > 59 )
        return false;

    dateTime = CORE::CDateTime( (Int16) year, (UInt8) month, (UInt8) day, (Int8) hours, (Int8) minutes, (Int8) seconds, 0, 0 );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpCookie::CHttpCookie( void )
    : name()
    , value()
    , domain()
    , path( "/" )
    , expires()
    , isPersistent( false )
    , hostOnly( true )
    , secure( false )
    , httpOnly( false )
    , sameSite()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHttpCookie::IsExpired( const CORE::CDateTime& nowUtc ) const
{GUCEF_TRACE;

    return isPersistent && expires.ToUnixEpochBasedTicksInMillisecs() <= nowUtc.ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

bool
CHttpCookie::Matches( const CString& lowercaseHost, const CString& requestPath, bool isSecure ) const
{GUCEF_TRACE;

    if ( secure && !isSecure )
        return false;
    if ( hostOnly ? lowercaseHost != domain : !DomainMatches( lowercaseHost, domain ) )
        return false;
    return PathMatches( requestPath.IsNULLOrEmpty() ? CString( "/" ) : requestPath, path );
}

/*-------------------------------------------------------------------------*/

CHttpCookieJar::CHttpCookieJar( void )
    : m_cookiesByDomain()
    , m_cookieCount( 0 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpCookieJar::~CHttpCookieJar()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::SetCookieNoLock( const CHttpCookie& cookie )
{GUCEF_TRACE;

    TCookieVector& cookies = m_cookiesByDomain[ cookie.domain ];
    for ( TCookieVector::iterator i = cookies.begin(); i != cookies.end(); ++i )
    {
        if ( (*i).name == cookie.name && (*i).path == cookie.path )
        {
            if ( cookie.IsExpired( CORE::CDateTime::NowUTCDateTime() ) )
            {
                cookies.erase( i );
                --m_cookieCount;
            }
            else
            {
                (*i) = cookie;
            }
            return;
        }
    }
    if ( !cookie.IsExpired( CORE::CDateTime::NowUTCDateTime() ) )
    {
        cookies.push_back( cookie );
        ++m_cookieCount;
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::SetCookie( const CHttpCookie& cookie )
{GUCEF_TRACE;

    CHttpCookie normalized( cookie );
    normalized.domain = cookie.domain.Lowercase();
    if ( normalized.domain.Length() > 0 && '.' == normalized.domain[ 0 ] )
    {
        normalized.domain = normalized.domain.CutChars( 1, true );
        normalized.hostOnly = false;
    }
    if ( normalized.path.IsNULLOrEmpty() )
        normalized.path = "/";

    MT::CScopeMutex lock( m_lock );
    SetCookieNoLock( normalized );
}

/*-------------------------------------------------------------------------*/

bool
CHttpCookieJar::SetCookieFromHeader( const CString& setCookieValue ,
                                     const CString& requestHost    ,
                                     const CString& requestPath    ,
                                     bool requestIsSecure          )
{GUCEF_TRACE;

    CString::StringVector parts = setCookieValue.ParseElements( ';', false );
    if ( parts.empty() )
        return false;

    // name=value
    CString nameValue = parts[ 0 ];
    Int32 equals = nameValue.HasChar( '=', true );
    if ( equals <= 0 )
        return false;

    CHttpCookie cookie;
    cookie.name = TrimWhitespace( nameValue.SubstrToIndex( (UInt32) equals, true ) );
    cookie.value = TrimWhitespace( nameValue.CutChars( (UInt32) equals + 1, true ) );
    if ( cookie.name.IsNULLOrEmpty() )
        return false;

    CString lowercaseHost = requestHost.Lowercase();
    CString domainAttribute;
    CString pathAttribute;
    bool hasMaxAge = false;
    Int64 maxAge = 0;

    for ( size_t i=1; i<parts.size(); ++i )
    {
        CString attribute = TrimWhitespace( parts[ i ] );
        Int32 attrEquals = attribute.HasChar( '=', true );
        CString attrName = ( attrEquals >= 0 ? attribute.SubstrToIndex( (UInt32) attrEquals, true ) : attribute ).Lowercase();
        attrName = TrimWhitespace( attrName );
        CString attrValue = attrEquals >= 0 ? TrimWhitespace( attribute.CutChars( (UInt32) attrEquals + 1, true ) ) : CString();

        if ( attrName == "expires" )
        {
            CORE::CDateTime expires;
            if ( !hasMaxAge && ParseCookieDate( attrValue, expires ) )
            {
                cookie.expires = expires;
                cookie.isPersistent = true;
            }
        }
        else
        if ( attrName == "max-age" )
        {
            hasMaxAge = true;
            maxAge = CORE::StringToInt64( attrValue );
            cookie.isPersistent = true;
        }
        else
        if ( attrName == "domain" )
        {
            domainAttribute = attrValue.Lowercase();
            while ( domainAttribute.Length() > 0 && '.' == domainAttribute[ 0 ] )
                domainAttribute = domainAttribute.CutChars( 1, true );
        }
        else
        if ( attrName == "path" )
        {
            if ( attrValue.Length() > 0 && '/' == attrValue[ 0 ] )
                pathAttribute = attrValue;
        }
        else
        if ( attrName == "secure" )
        {
            cookie.secure = true;
        }
        else
        if ( attrName == "httponly" )
        {
            cookie.httpOnly = true;
        }
        else
        if ( attrName == "samesite" )
        {
            cookie.sameSite = attrValue;
        }
    }

    if ( hasMaxAge )
    {
        // Max-Age takes precedence over Expires, zero or negative expires the cookie
        UInt64 nowTicks = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
        Int64 expiryTicks = (Int64) nowTicks + maxAge * 1000;
        if ( maxAge <= 0 )
            expiryTicks = 0;
        cookie.expires.FromUnixEpochBasedTicksInMillisecs( expiryTicks > 0 ? (UInt64) expiryTicks : 0 );
    }

    if ( domainAttribute.IsNULLOrEmpty() )
    {
        cookie.domain = lowercaseHost;
        cookie.hostOnly = true;
    }
    else
    {
        // The host has to be within the domain, and a domain cookie for a top level domain is not accepted
        if ( !DomainMatches( lowercaseHost, domainAttribute ) )
            return false;
        if ( domainAttribute.HasChar( '.' ) < 0 && domainAttribute != lowercaseHost )
            return false;
        cookie.domain = domainAttribute;
        cookie.hostOnly = false;
    }

    // A secure cookie can only be set over a secure connection
    if ( cookie.secure && !requestIsSecure )
        return false;

    cookie.path = pathAttribute.IsNULLOrEmpty() ? DefaultCookiePath( requestPath ) : pathAttribute;

    MT::CScopeMutex lock( m_lock );
    SetCookieNoLock( cookie );
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CHttpCookieJar::GetCookieHeaderValue( const CString& requestHost ,
                                      const CString& requestPath ,
                                      bool requestIsSecure       ) const
{GUCEF_TRACE;

    CString lowercaseHost = requestHost.Lowercase();
    CORE::CDateTime nowUtc = CORE::CDateTime::NowUTCDateTime();

    // Collect the matching cookies, visiting only the host and its parent domains
    typedef std::vector< const CHttpCookie* > TCookiePtrVector;
    TCookiePtrVector matches;
    {
        MT::CScopeMutex lock( m_lock );
        CString domain = lowercaseHost;
        while ( !domain.IsNULLOrEmpty() )
        {
            TDomainCookieMap::const_iterator d = m_cookiesByDomain.find( domain );
            if ( d != m_cookiesByDomain.end() )
            {
                const TCookieVector& cookies = (*d).second;
                for ( TCookieVector::const_iterator c = cookies.begin(); c != cookies.end(); ++c )
                {
                    if ( !(*c).IsExpired( nowUtc ) && (*c).Matches( lowercaseHost, requestPath, requestIsSecure ) )
                        matches.push_back( &(*c) );
                }
            }
            Int32 dot = domain.HasChar( '.', true );
            if ( dot < 0 || IsIpAddress( lowercaseHost ) )
                break;
            domain = domain.CutChars( (UInt32) dot + 1, true );
        }

        // Cookies with longer paths are listed first, RFC 6265 section 5.4
        struct SLongerPathFirst
        {
            bool operator()( const CHttpCookie* a, const CHttpCookie* b ) const { return a->path.Length() > b->path.Length(); }
        };
        std::stable_sort( matches.begin(), matches.end(), SLongerPathFirst() );

        CString header;
        for ( TCookiePtrVector::const_iterator i = matches.begin(); i != matches.end(); ++i )
        {
            if ( !header.IsNULLOrEmpty() )
                header += "; ";
            header += (*i)->name + "=" + (*i)->value;
        }
        return header;
    }
}

/*-------------------------------------------------------------------------*/

bool
CHttpCookieJar::RemoveCookie( const CString& domain, const CString& path, const CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDomainCookieMap::iterator d = m_cookiesByDomain.find( domain.Lowercase() );
    if ( d == m_cookiesByDomain.end() )
        return false;
    TCookieVector& cookies = (*d).second;
    for ( TCookieVector::iterator i = cookies.begin(); i != cookies.end(); ++i )
    {
        if ( (*i).name == name && (*i).path == path )
        {
            cookies.erase( i );
            --m_cookieCount;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::RemoveExpiredCookies( void )
{GUCEF_TRACE;

    CORE::CDateTime nowUtc = CORE::CDateTime::NowUTCDateTime();
    MT::CScopeMutex lock( m_lock );
    for ( TDomainCookieMap::iterator d = m_cookiesByDomain.begin(); d != m_cookiesByDomain.end(); ++d )
    {
        TCookieVector& cookies = (*d).second;
        for ( TCookieVector::iterator i = cookies.begin(); i != cookies.end(); )
        {
            if ( (*i).IsExpired( nowUtc ) )
            {
                i = cookies.erase( i );
                --m_cookieCount;
            }
            else
                ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::RemoveSessionCookies( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TDomainCookieMap::iterator d = m_cookiesByDomain.begin(); d != m_cookiesByDomain.end(); ++d )
    {
        TCookieVector& cookies = (*d).second;
        for ( TCookieVector::iterator i = cookies.begin(); i != cookies.end(); )
        {
            if ( !(*i).isPersistent )
            {
                i = cookies.erase( i );
                --m_cookieCount;
            }
            else
                ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cookiesByDomain.clear();
    m_cookieCount = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CHttpCookieJar::GetCookieCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cookieCount;
}

/*-------------------------------------------------------------------------*/

void
CHttpCookieJar::GetAllCookies( TCookieVector& cookies ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    cookies.reserve( cookies.size() + m_cookieCount );
    for ( TDomainCookieMap::const_iterator d = m_cookiesByDomain.begin(); d != m_cookiesByDomain.end(); ++d )
        cookies.insert( cookies.end(), (*d).second.begin(), (*d).second.end() );
}

/*-------------------------------------------------------------------------*/

bool
CHttpCookieJar::LoadFromNetscapeFile( const CString& path )
{GUCEF_TRACE;

    CString content;
    if ( !CORE::LoadTextFileAsString( path, content, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpCookieJar: Unable to load cookies from \"" + path + "\"" );
        return false;
    }

    static const CString httpOnlyPrefix = "#HttpOnly_";
    CString::StringVector lines = content.RemoveChar( '\r' ).ParseElements( '\n', false );

    MT::CScopeMutex lock( m_lock );
    for ( CString::StringVector::iterator i = lines.begin(); i != lines.end(); ++i )
    {
        CString line = (*i);
        bool httpOnly = false;
        if ( 0 == line.HasSubstr( httpOnlyPrefix, true ) )
        {
            httpOnly = true;
            line = line.CutChars( httpOnlyPrefix.Length(), true );
        }
        if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
            continue;

        // domain, include sub domains, path, secure, expiry, name, value
        CString::StringVector fields = line.ParseElements( '\t', true );
        if ( fields.size() < 7 )
            continue;

        CHttpCookie cookie;
        cookie.domain = fields[ 0 ].Lowercase();
        cookie.hostOnly = !fields[ 1 ].Equals( "TRUE", false );
        if ( cookie.domain.Length() > 0 && '.' == cookie.domain[ 0 ] )
        {
            cookie.domain = cookie.domain.CutChars( 1, true );
            cookie.hostOnly = false;
        }
        cookie.path = fields[ 2 ].IsNULLOrEmpty() ? CString( "/" ) : fields[ 2 ];
        cookie.secure = fields[ 3 ].Equals( "TRUE", false );
        UInt64 expiry = CORE::StringToUInt64( fields[ 4 ] );
        cookie.isPersistent = 0 != expiry;
        if ( cookie.isPersistent )
            cookie.expires.FromUnixEpochBasedTicksInMillisecs( expiry * 1000 );
        cookie.name = fields[ 5 ];
        cookie.value = fields[ 6 ];
        cookie.httpOnly = httpOnly;
        SetCookieNoLock( cookie );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpCookieJar::SaveToNetscapeFile( const CString& path ) const
{GUCEF_TRACE;

    CString content = "# Netscape HTTP Cookie File\n";
    {
        MT::CScopeMutex lock( m_lock );
        for ( TDomainCookieMap::const_iterator d = m_cookiesByDomain.begin(); d != m_cookiesByDomain.end(); ++d )
        {
            const TCookieVector& cookies = (*d).second;
            for ( TCookieVector::const_iterator c = cookies.begin(); c != cookies.end(); ++c )
            {
                const CHttpCookie& cookie = (*c);
                UInt64 expiry = cookie.isPersistent ? cookie.expires.ToUnixEpochBasedTicksInMillisecs() / 1000 : 0;
                content += ( cookie.httpOnly ? CString( "#HttpOnly_" ) : CString() ) +
                           ( cookie.hostOnly ? cookie.domain : "." + cookie.domain ) + "\t" +
                           ( cookie.hostOnly ? "FALSE" : "TRUE" ) + "\t" +
                           cookie.path + "\t" +
                           ( cookie.secure ? "TRUE" : "FALSE" ) + "\t" +
                           CORE::ToString( expiry ) + "\t" +
                           cookie.name + "\t" +
                           cookie.value + "\n";
            }
        }
    }
    return CORE::WriteFileAtomically( path, content.C_String(), content.ByteSize()-1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
