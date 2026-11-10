/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#define GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CLocalIdentityProvider.h"
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#include "gucefMULTIPLAY_CPlayerIdentitySigner.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The identity of the local player with the accounts at external identity providers linked to it: "Log in with ...".
 *  Registered as "sso". The key and the profile (the name others see) are those of a CLocalIdentityProvider, which this
 *  one wraps and which stays the root of the identity: the player is a guest with the key alone, and is linked
 *  once at least one account is. The accounts come from a CAuthManager of gucefAUTH, which the provider owns unless
 *  SetAuthManager() gives it another one (the manager of CAuthGlobal for example). The manager gets this provider's
 *  profile key as its signer, so the links it makes are links to this player, and the credential store of this provider,
 *  which is also where the private key of a new profile is kept (see CLocalIdentityProvider).
 *
 *  GetIdentity() is the identity of the local provider with:
 *      accounts    the verified linked accounts added to what the profile has: the provider namespace of the account
 *                  is the backend name and its subject the account id ("epic" -> "0123456789abcdef0123456789abcdef"), see CPlayerIdentity
 *      displayName the name of the first linked account that is logged in when the name of the profile is still the
 *                  default name (defaultDisplayName), else the name of the profile
 *      linkProofs  the proofs of the logins of the linked accounts (at most CLinkProof::MaxCount), for the services that
 *                  show them to a server; IdentityChangedEvent and public identities have none
 *  IdentityChangedEvent is raised when the name changes and when an account is linked, unlinked or logged in (at least once
 *  per change, the identity is the public one as it is then).
 *
 *  Login(), AutoLogin() start the logins of the manager: events and futures are those of CAuthManager, which is on the
 *  thread that pumps the pulse generator, like this provider. AutoLogin() honours the setting autoLogin and never blocks.
 *
 *  Config: that of CLocalIdentityProvider (appName, profilePath, defaultDisplayName, keyInCredentialStore) and the child
 *  "Auth" (CAuthSettings, section "Auth" of the application, which can also have keyInCredentialStore). LoadAuthConfig()
 *  takes that section from elsewhere, for applications that keep it in their own config.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CAuthIdentityProvider : public CIIdentityProvider
{
    public:

    static const CORE::CString ProviderName;                /**< "sso" */
    static const CORE::CEvent AuthSettingsChangedEvent;     /**< the settings of the manager changed (the last used providers): the application saves its config, see SaveAuthConfig() */

    static void RegisterEvents( void );

    CAuthIdentityProvider( void );

    virtual ~CAuthIdentityProvider();

    virtual bool GetIdentity( CPlayerIdentity& identity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Logs in with the provider of gucefAUTH of that name and links the account to the profile. False when the login cannot start */
    virtual bool Login( const CORE::CString& externalProviderName, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** The startup policy of the manager: silent logins, then stored ones. False when the setting autoLogin is off */
    virtual bool AutoLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** GUEST: the key alone, LINKED: at least one account is linked */
    AUTH::TAuthLoginState GetLoginState( void );

    /** The accounts linked to the profile, the one made first first */
    bool GetLinkedAccounts( AUTH::TAccountRefVector& accounts );

    /** The names of the providers a player can log in with now, the last used first; empty when none is configured */
    void GetOfferedProviders( CORE::CString::StringVector& providerNames );

    /** Removes the link of the account (and its stored login) */
    bool Unlink( const AUTH::CAccountRef& account );

    /** The account the profile name is made of when the profile still has the default name, empty when there is none */
    AUTH::CAccountRef GetPrimaryAccount( void );

    /** The accounts at services the local provider knows, see CLocalIdentityProvider::SetAccount() */
    bool SetAccount( const CORE::CString& backendName, const CORE::CString& accountId );

    /** The manager in use, with the signer and stores of this provider in place */
    AUTH::CAuthManager& GetAuthManager( void );

    /** Uses this manager instead of the own one; not owned, it has to outlive the provider or be given back with GUCEF_NULL first */
    void SetAuthManager( AUTH::CAuthManager* manager );

    /** The provider that keeps the profile, for settings of the key and its place */
    CLocalIdentityProvider& GetLocalProvider( void );

    /** The settings of the manager (section "Auth") from this node, the section or a node that has it as a child */
    bool LoadAuthConfig( const CORE::CDataNode& config );

    /** The settings of the manager as they are now, with the last used providers: what an application saves */
    bool SaveAuthConfig( CORE::CDataNode& config ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAuthIdentityProvider > TEventCallback;

    class CProfileSigner;
    friend class CProfileSigner;

    CAuthIdentityProvider( const CAuthIdentityProvider& src );
    CAuthIdentityProvider& operator=( const CAuthIdentityProvider& src );

    void EnsureWired( void );
    void RefreshSigner( void );
    void Detach( AUTH::CAuthManager* manager );
    void RaiseIdentityChanged( void );
    void SubscribeToManager( AUTH::CAuthManager* manager );

    void OnLocalIdentityChanged( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

    void OnManagerAccountsChanged( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnManagerSettingsChanged( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );
    bool BuildIdentity( CPlayerIdentity& identity, bool withProofs );

    private:

    CLocalIdentityProvider m_local;
    CPlayerIdentitySigner m_signer;
    CProfileSigner* m_profileSigner;
    AUTH::CAuthManager* m_ownedManager;
    AUTH::CAuthManager* m_manager;
    AUTH::CFileCredentialStore* m_credentialStore;
    CORE::CString m_credentialDirectory;
    CORE::CString m_defaultDisplayName;
    bool m_wired;
    bool m_raising;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIIdentityProvider, CAuthIdentityProvider, MT::CMutex > TAuthIdentityProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H ? */
