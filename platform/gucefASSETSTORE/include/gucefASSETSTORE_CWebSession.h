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

#ifndef GUCEF_ASSETSTORE_CWEBSESSION_H
#define GUCEF_ASSETSTORE_CWEBSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */
#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */
#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H
#include "gucefASSETSTORE_CWebSessionCookie.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H ? */

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
 *  The web session of a store: the cookies of the store's domains that a browser had once the owner was signed in
 *  (section 12.1 of the design). It is a secret as a whole: it is kept in a gucefAUTH credential store as one
 *  AUTH::CSecretString (ToSecret(), provider SessionProviderName, account the store id) and goes into a gucefWEB
 *  cookie jar for the HTTP requests of the store plugin (ApplyTo()). It is never logged: what is streamed to a log
 *  (operator<<, Describe()) is the store, the number of cookies and when it was captured.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSession
{
    public:

    static const CORE::CString SessionProviderName;     /**< "assetstore-web-session" */
    static const CORE::CString FormatName;              /**< "gucef.webSession" */

    CORE::CString storeId;
    UInt64 capturedAtUtcInMs;
    TWebSessionCookieVector cookies;

    CWebSession( void );
    ~CWebSession();

    bool IsEmpty( void ) const;

    UInt32 GetCookieCount( void ) const;

    /** Whether a cookie of the name has a value */
    bool HasCookie( const CORE::CString& name ) const;

    /** Wipes the values and forgets everything */
    void Clear( void );

    /** Drops the cookies with an expiry in the past; the number dropped */
    UInt32 RemoveExpiredCookies( UInt64 nowUtcInMs );

    /** The session as one secret (JSON); false when the json codec is not there */
    bool ToSecret( AUTH::CSecretString& secret ) const;

    /** Reads a session that ToSecret() made; drops expired cookies */
    bool FromSecret( const AUTH::CSecretString& secret );

    /** Keeps the session in the credential store under the store id */
    bool SaveTo( AUTH::ICredentialStore& store ) const;

    /** Reads the session of the store from the credential store; false when there is none or it holds no cookie that is still valid */
    bool LoadFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId );

    /** Forgets the session of the store in the credential store */
    static bool RemoveFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId );

    /** Puts the cookies into the jar, replacing cookies of the same name, domain and path; the number put in */
    UInt32 ApplyTo( WEB::CHttpCookieJar& jar ) const;

    /** "web session of <store>: <n> cookies, captured <time>": never a value */
    CORE::CString Describe( void ) const;

    /** Now as UTC milliseconds since the epoch */
    static UInt64 NowUtcInMs( void );
};

/*-------------------------------------------------------------------------*/

/** Streams Describe(): a session in a log line never shows a cookie value */
GUCEF_ASSETSTORE_PUBLIC_CPP CORE::CVariantStream&
operator<<( CORE::CVariantStream& stream, const CWebSession& session );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSION_H ? */
