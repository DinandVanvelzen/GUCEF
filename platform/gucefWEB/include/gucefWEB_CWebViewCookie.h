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

#ifndef GUCEF_WEB_CWEBVIEWCOOKIE_H
#define GUCEF_WEB_CWEBVIEWCOOKIE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

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
 *  A cookie of a profile, HttpOnly ones included. The value is a secret: it is kept apart (GetValue(), SetValue()),
 *  wiped from memory when the cookie goes, and never written to a log or a report (Describe() leaves it out).
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewCookie
{
    public:

    CString name;
    CString domain;                 /**< "example.com" for the host only, ".example.com" for the domain and below */
    CString path;                   /**< "/" when the cookie did not say */
    Int64 expiresUtcSeconds;        /**< seconds since 1970 UTC; 0 for a session cookie */
    bool secure;
    bool httpOnly;
    TWebViewSameSite sameSite;

    CWebViewCookie( void );

    CWebViewCookie( const CWebViewCookie& src );

    ~CWebViewCookie();

    CWebViewCookie& operator=( const CWebViewCookie& src );

    const CString& GetValue( void ) const;

    void SetValue( const CString& value );

    bool IsSession( void ) const;

    /** Whether the cookie would be sent to the address: domain, path and Secure; false for an address that is not http(s) */
    bool MatchesUrl( const CString& url ) const;

    /** An empty filter matches every cookie, otherwise MatchesUrl() */
    bool MatchesFilter( const CString& urlFilter ) const;

    /** Whether the cookie is the same cookie as 'other': name, domain and path */
    bool IsSameCookie( const CWebViewCookie& other ) const;

    /** Name, domain, path and flags for logs and reports; never the value */
    CString Describe( void ) const;

    private:

    CString m_value;
};

typedef GUCEF::vector< CWebViewCookie > TWebViewCookieVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWCOOKIE_H ? */
