/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTFAKESTOREPROVIDER_H
#define TESTFAKESTOREPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */
#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H
#include "gucefASSETSTORE_CIAssetStoreInteractiveSignIn.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreSignInContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H ? */
#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The store provider of the fake store, as a store plugin would make it: library, items and plans over HTTP from
 *  the CFakeStoreServer, the session token as a secret in the Authorization header of downloads, renewed through the
 *  token endpoint of the store.
 */
class CFakeStoreProvider : public GUCEF::ASSETSTORE::CIAssetStoreProvider
{
    public:

    /** The server all fake providers talk to; set by the tests */
    static void SetServer( CFakeStoreServer* server );

    CFakeStoreProvider( void );
    virtual ~CFakeStoreProvider();

    void SetProviderId( const GUCEF::CORE::CString& providerId );

    GUCEF::UInt32 GetPlanCount( void ) const;
    GUCEF::UInt32 GetRenewCount( void ) const;
    void SetToken( const GUCEF::CORE::CString& token );
    /** Tells the observers that the session changed (a sign in happened), as a store does after a sign in */
    void RaiseSessionChanged( void );

    virtual const GUCEF::CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    /** The provider "fakestore2" has a home page ("https://www.Store-Two.example/shop"), the other ones have none */
    virtual GUCEF::CORE::CString GetHomePageUrl( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool RenewSession( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryLibrary( const GUCEF::ASSETSTORE::CAssetStoreQuery& query, GUCEF::ASSETSTORE::CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetItem( const GUCEF::CORE::CString& itemId, GUCEF::ASSETSTORE::CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus PlanDownload( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& formatId,
                                                               const GUCEF::CORE::CString::StringVector& fileIds, GUCEF::ASSETSTORE::CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AuthorizeTransferRequest( const GUCEF::ASSETSTORE::CAssetStoreTransfer& transfer, GUCEF::WEB::CHttpRequestData& request ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::ASSETSTORE::TAssetStoreStatus Get( const GUCEF::CORE::CString& path, GUCEF::CORE::CDataNode& json ) const;

    private:

    GUCEF::CORE::CString m_providerId;
    GUCEF::AUTH::CSecretString m_token;
    GUCEF::UInt32 m_planCount;
    GUCEF::UInt32 m_renewCount;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** Makes fake store providers with an id, as the concrete factory of a store plugin */
class CFakeStoreProviderFactory : public GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeStoreProvider, GUCEF::MT::CMutex >
{
    public:

    typedef GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeStoreProvider, GUCEF::MT::CMutex > TBase;

    explicit CFakeStoreProviderFactory( const GUCEF::CORE::CString& providerId );
    virtual ~CFakeStoreProviderFactory();

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CString m_providerId;
};

/*-------------------------------------------------------------------------*/

class CFakeSignInStoreProvider;

/**
 *  The sign in of a store with steps of a person, as a store plugin of the OAuth kind has it: an address to open and a code to
 *  paste. A well shaped code is "exchanged" at the store (two polls later): the right code (RightCode()) gives a session, any
 *  other well shaped code is refused by the store; a text that is no code is not taken at all. Everything of the store is in memory.
 */
class CFakeInteractiveSignIn : public GUCEF::ASSETSTORE::CIAssetStoreInteractiveSignIn
{
    public:

    static const char* RightCode( void );
    static const char* WrongCode( void );
    static const char* LoginUrl( void );

    explicit CFakeInteractiveSignIn( CFakeSignInStoreProvider& provider );
    virtual ~CFakeInteractiveSignIn();

    virtual bool Begin( GUCEF::ASSETSTORE::TAssetSignInPromptVector& prompts, GUCEF::CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Answer( const GUCEF::CORE::CString& promptKind, const GUCEF::AUTH::CSecretString& answer, GUCEF::CORE::CString& reason ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Cancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreSignInState Update( GUCEF::CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;

    GUCEF::UInt32 GetBeginCount( void ) const;
    GUCEF::UInt32 GetCancelCount( void ) const;

    private:

    CFakeSignInStoreProvider& m_provider;
    bool m_running;
    bool m_exchanging;
    GUCEF::UInt32 m_polls;
    GUCEF::CORE::CString m_received;
    GUCEF::UInt32 m_beginCount;
    GUCEF::UInt32 m_cancelCount;
};

/*-------------------------------------------------------------------------*/

/**
 *  A store that signs in with steps of a person (kind authProvider, CFakeInteractiveSignIn) or on its web site (kind webSession, no
 *  sign in object: the service has no browser). Its library answers NOT_SIGNED_IN without a session and SESSION_EXPIRED after
 *  RefuseSession(). ConfigureSignIn() wants the child of the store in the settings of the sign in context, as a store that reads its
 *  client credentials from a file would: without it the store says why it cannot sign in.
 */
class CFakeSignInStoreProvider : public GUCEF::ASSETSTORE::CIAssetStoreProvider
{
    public:

    CFakeSignInStoreProvider( void );
    virtual ~CFakeSignInStoreProvider();

    void SetProviderId( const GUCEF::CORE::CString& providerId );
    void SetWebSession( bool webSession );

    /** The session ends at the store: the provider is signed out and says so (the service learns it from the event) */
    void ExpireSession( void );
    /** The store refuses the session of the provider with 401 until a sign in: IsSignedIn() stays true */
    void RefuseSession( void );
    void SetSignedIn( bool signedIn );

    bool WasConfigured( void ) const;
    GUCEF::CORE::CString GetClientIdFile( void ) const;
    bool IsProvisioned( void ) const;
    CFakeInteractiveSignIn& GetFakeSignIn( void );

    virtual const GUCEF::CORE::CString& GetProviderId( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetAuthProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreSignInKind GetSignInKind( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::CIAssetStoreInteractiveSignIn* GetInteractiveSignIn( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ConfigureSignIn( const GUCEF::ASSETSTORE::CAssetStoreSignInContext& context, GUCEF::CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSignedIn( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SignOut( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus QueryLibrary( const GUCEF::ASSETSTORE::CAssetStoreQuery& query, GUCEF::ASSETSTORE::CAssetStoreLibraryPage& page ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus GetItem( const GUCEF::CORE::CString& itemId, GUCEF::ASSETSTORE::CAssetStoreItem& item ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus PlanDownload( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& formatId,
                                                               const GUCEF::CORE::CString::StringVector& fileIds, GUCEF::ASSETSTORE::CAssetStoreDownloadPlan& plan ) GUCEF_VIRTUAL_OVERRIDE;
    /** Why PlanDownload() refused: "the fake sign in store has no downloads (<item> <format>)" */
    virtual GUCEF::CORE::CString GetPlanProblem( const GUCEF::CORE::CString& itemId, const GUCEF::CORE::CString& formatId ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Raises the event of a change of the session, from the sign in */
    void SessionChanged( void );

    /** A web session store takes a handed over session: the cookies of the domain webstore.test; it is then signed in */
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ImportWebSession( const GUCEF::AUTH::CWebSession& session, GUCEF::CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;
    /** What the last ImportWebSession() kept: the number of cookies and the user agent */
    GUCEF::UInt32 GetImportedCookieCount( void ) const;
    GUCEF::CORE::CString GetImportedUserAgent( void ) const;

    private:

    GUCEF::CORE::CString m_providerId;
    bool m_webSession;
    bool m_signedIn;
    bool m_refused;
    bool m_configured;
    bool m_provisioned;
    GUCEF::CORE::CString m_clientIdFile;
    GUCEF::UInt32 m_importedCookies;
    GUCEF::CORE::CString m_importedUserAgent;
    CFakeInteractiveSignIn m_signIn;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

/** Makes fake sign in stores with an id and a kind, as the concrete factory of a store plugin */
class CFakeSignInStoreProviderFactory : public GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeSignInStoreProvider, GUCEF::MT::CMutex >
{
    public:

    typedef GUCEF::CORE::CTFactory< GUCEF::ASSETSTORE::CIAssetStoreProvider, CFakeSignInStoreProvider, GUCEF::MT::CMutex > TBase;

    CFakeSignInStoreProviderFactory( const GUCEF::CORE::CString& providerId, bool webSession );
    virtual ~CFakeSignInStoreProviderFactory();

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CString m_providerId;
    bool m_webSession;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKESTOREPROVIDER_H ? */
