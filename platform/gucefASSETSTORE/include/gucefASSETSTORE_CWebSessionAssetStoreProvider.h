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

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H
#define GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */
#ifndef GUCEF_WEB_CHTTPCOOKIEJAR_H
#include "gucefWEB_CHttpCookieJar.h"
#define GUCEF_WEB_CHTTPCOOKIEJAR_H
#endif /* GUCEF_WEB_CHTTPCOOKIEJAR_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#include "gucefAUTH_CWebSessionSignIn.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNIN_H ? */

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
 *  The base of a store provider that signs in on the web site of its store (section 12.1 of the design): it answers
 *  ASSETSTORE_SIGNIN_WEB_SESSION, owns the AUTH::CWebSessionSignIn of the store and keeps the session in a cookie jar for
 *  the HTTP requests of the plugin. A plugin derives from it, calls ConfigureWebSignIn() with its login script
 *  (loaded from its data folder) and implements the library operations of CIAssetStoreProvider with requests that
 *  use GetCookieJar() (or AddSessionCookies()) and hand every answer to CheckResponse().
 *
 *  - IsSignedIn(): there is a session (saved or new).
 *  - SignOut(): forgets the session here, in the session store and in the browser profile (at the next sign in).
 *  - RenewSession(): an unattended sign in with saved credentials, when the settings allow it; the download manager
 *    calls it on a 401. Without saved credentials it is false and the owner signs in again.
 *  - AuthorizeTransferRequest(): adds the cookies of the session to the download requests.
 *  - SessionStateChangedEvent is raised when a sign in succeeded and when the session ended or was forgotten.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSessionAssetStoreProvider : public CIAssetStoreProvider
{
    public:

    CWebSessionAssetStoreProvider( void );
    virtual ~CWebSessionAssetStoreProvider() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Sets up the sign in: the store id of the settings becomes the provider id when it is empty; reads the session
     *  that was saved. The stores are not owned and must outlive the provider; GUCEF_NULL takes the store the
     *  application set with CAssetStoreGlobal::SetWebSignInStores() (none when it set none)
     */
    bool ConfigureWebSignIn( const AUTH::CWebSessionSignInSettings& settings        ,
                             AUTH::ICredentialStore* sessionStore                   ,
                             AUTH::ICredentialStore* credentialStore                ,
                             CORE::CString::StringVector* problems = GUCEF_NULL     );

    virtual TAssetStoreSignInKind GetSignInKind( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual AUTH::CWebSessionSignIn* GetWebSessionSignIn( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the credential stores of the service (sessions and saved passwords) for the sign in and reads a saved session; a store the context does not give is left as it was */
    virtual bool ConfigureSignIn( const CAssetStoreSignInContext& context, CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SignOut( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RenewSession( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AuthorizeTransferRequest( const CAssetStoreTransfer& transfer, WEB::CHttpRequestData& request ) GUCEF_VIRTUAL_OVERRIDE;

    /** The cookie jar with the session: give it to the requests of the plugin (CHttpClientRequestOptions::cookieJar) */
    WEB::CHttpCookieJarPtr GetCookieJar( void ) const;

    /** Adds a Cookie header with the cookies of the session for the address of the request; false when none apply */
    bool AddSessionCookies( WEB::CHttpRequestData& request ) const;

    /** Hand every answer of the store to this: true when the session ended (answer ASSETSTORE_STATUS_SESSION_EXPIRED then) */
    bool CheckResponse( const WEB::CHttpResponseData& response );

    /** Puts the current session into the cookie jar again, replacing what is there */
    void RefreshCookieJar( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    AUTH::CWebSessionSignIn& GetSignIn( void );

    private:

    class CSignInObserver : public CORE::CObserver
    {
        public:
        explicit CSignInObserver( CWebSessionAssetStoreProvider& provider ) : CORE::CObserver(), m_provider( provider ) {}
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;
        private:
        CWebSessionAssetStoreProvider& m_provider;
    };

    CWebSessionAssetStoreProvider( const CWebSessionAssetStoreProvider& src );              /**< not implemented */
    CWebSessionAssetStoreProvider& operator=( const CWebSessionAssetStoreProvider& src );   /**< not implemented */

    private:

    AUTH::CWebSessionSignIn m_signIn;
    WEB::CHttpCookieJarPtr m_cookieJar;
    CSignInObserver m_observer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSIONASSETSTOREPROVIDER_H ? */
