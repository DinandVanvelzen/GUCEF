/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H
#define GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A cookie of a web session (CWebSession). Its value is a secret: it is what signs in to the store, so it is kept as
 *  AUTH::CSecretString and never logged; the other fields are not secret.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSessionCookie
{
    public:

    CORE::CString name;
    AUTH::CSecretString value;
    CORE::CString domain;               /**< as the browser gives it: a leading dot for a cookie of the domain and below */
    CORE::CString path;
    Float64 expires;                    /**< seconds since the epoch (UTC), below 0 for a session cookie */
    bool httpOnly;
    bool secure;
    bool session;                       /**< ends with the browser session: no expiry */
    CORE::CString sameSite;

    CWebSessionCookie( void );

    /** Whether a cookie with an expiry is past it */
    bool IsExpired( UInt64 nowUtcInMs ) const;

    /** The cookie for a cookie jar of gucefWEB, for the HTTP requests of a store plugin */
    void ToHttpCookie( WEB::CHttpCookie& cookie ) const;

    static CWebSessionCookie FromChromeCookie( const WEB::SChromeCookie& cookie );

    static CWebSessionCookie FromHttpCookie( const WEB::CHttpCookie& cookie );

    /** Writes the fields, the value included: only for a node that is kept as a secret (CWebSession::ToSecret()) */
    void WriteTo( CORE::CDataNode& node ) const;

    /** False when the name is missing */
    bool ReadFrom( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CWebSessionCookie > TWebSessionCookieVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H ? */
