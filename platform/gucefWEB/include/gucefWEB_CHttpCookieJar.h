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

#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#define GUCEF_WEB_CHTTPCOOKIEJAR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <vector>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A HTTP cookie as defined by RFC 6265
 */
class GUCEF_WEB_PUBLIC_CPP CHttpCookie
{
    public:

    CString name;
    CString value;
    CString domain;             /**< lowercase, without a leading dot */
    CString path;
    CORE::CDateTime expires;    /**< only meaningful if isPersistent is set */
    bool isPersistent;          /**< false for session cookies which have no expiry */
    bool hostOnly;              /**< only sent to exactly the domain host, not its sub domains */
    bool secure;                /**< only sent over https */
    bool httpOnly;
    CString sameSite;

    CHttpCookie( void );

    bool IsExpired( const CORE::CDateTime& nowUtc ) const;

    /** whether the cookie is to be sent with a request to the given host, path and protocol */
    bool Matches( const CString& lowercaseHost, const CString& requestPath, bool isSecure ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  Thread-safe store of HTTP cookies, a subset of RFC 6265: Set-Cookie parsing, domain and path matching,
 *  Secure, HttpOnly, expiry. Cookies can also be set directly, for example from a browser session, and the jar
 *  can be loaded from and saved to the Netscape cookies.txt format used by curl, wget and browser extensions.
 *  There is no public suffix list, as such a Set-Cookie for a domain without a dot is rejected but registrable
 *  domain boundaries like co.uk are not checked.
 *
 *  Lookups only visit the cookies of the domains the request host is part of, not the whole jar.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpCookieJar
{
    public:

    typedef std::vector< CHttpCookie > TCookieVector;

    CHttpCookieJar( void );
    virtual ~CHttpCookieJar();

    /**
     *  Stores the cookie, replacing a cookie with the same name, domain and path. Expired cookies remove it instead.
     */
    void SetCookie( const CHttpCookie& cookie );

    /**
     *  Processes the value of a Set-Cookie response header received for the given request
     *  @return whether a cookie was stored or removed, false if the header was invalid or not allowed for the host
     */
    bool SetCookieFromHeader( const CString& setCookieValue ,
                              const CString& requestHost    ,
                              const CString& requestPath    ,
                              bool requestIsSecure          );

    /**
     *  @return the value for a Cookie request header for the given request, empty if no cookies apply
     */
    CString GetCookieHeaderValue( const CString& requestHost ,
                                  const CString& requestPath ,
                                  bool requestIsSecure       ) const;

    bool RemoveCookie( const CString& domain, const CString& path, const CString& name );

    void RemoveExpiredCookies( void );

    /** removes the session cookies, as a browser would when closed */
    void RemoveSessionCookies( void );

    void Clear( void );

    UInt32 GetCookieCount( void ) const;

    void GetAllCookies( TCookieVector& cookies ) const;

    /** loads cookies from a Netscape cookies.txt file, adding to the cookies already present */
    bool LoadFromNetscapeFile( const CString& path );

    /** saves all cookies, including session cookies, in the Netscape cookies.txt format */
    bool SaveToNetscapeFile( const CString& path ) const;

    private:

    CHttpCookieJar( const CHttpCookieJar& src );            /**< not supported */
    CHttpCookieJar& operator=( const CHttpCookieJar& src ); /**< not supported */

    void SetCookieNoLock( const CHttpCookie& cookie );

    typedef std::map< CString, TCookieVector > TDomainCookieMap;

    TDomainCookieMap m_cookiesByDomain;
    UInt32 m_cookieCount;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CHttpCookieJar, MT::CMutex > CHttpCookieJarPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
