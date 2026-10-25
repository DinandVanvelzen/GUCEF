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

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#define GUCEF_AUTH_CAUTHMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_IIDENTITYLINKSTORE_H
#include "gucefAUTH_IIdentityLinkStore.h"
#define GUCEF_AUTH_IIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_IIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What is known about the person at the start: nothing but the profile key, or at least one verified account */
enum EAuthLoginState
{
    AUTHLOGINSTATE_GUEST = 0,       /**< only the profile key */
    AUTHLOGINSTATE_LINKED = 1       /**< the profile has at least one account with a verified link record */
};

typedef enum EAuthLoginState TAuthLoginState;

/*-------------------------------------------------------------------------*/

/** Event data of CAuthManager::LoginStateChangedEvent and AutoLoginFinishedEvent */
class GUCEF_AUTH_EXPORT_CPP CAuthStateEventData : public CORE::CICloneable
{
    public:

    TAuthLoginState state;
    UInt32 linkedAccountCount;

    CAuthStateEventData( void );

    CAuthStateEventData( TAuthLoginState loginState, UInt32 linkedCount );

    CAuthStateEventData( const CAuthStateEventData& src );

    virtual ~CAuthStateEventData();

    CAuthStateEventData& operator=( const CAuthStateEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** Event data of CAuthManager::AccountLinkedEvent, AccountUnlinkedEvent and AccountLoggedOutEvent */
class GUCEF_AUTH_EXPORT_CPP CAuthAccountEventData : public CORE::CICloneable
{
    public:

    CAccountRef accountRef;
    CORE::CString providerName;

    CAuthAccountEventData( void );

    CAuthAccountEventData( const CAccountRef& account, const CORE::CString& provider );

    CAuthAccountEventData( const CAuthAccountEventData& src );

    virtual ~CAuthAccountEventData();

    CAuthAccountEventData& operator=( const CAuthAccountEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The login logic of an application: which providers are offered, logging in with them, keeping the logins across
 *  restarts, linking the accounts to the profile key and the startup policy that tries to log in without the user.
 *  CAuthGlobal owns the manager of the process; more managers can be made (tests do).
 *
 *  Everything the manager does is on the thread that pumps the pulse generator, which is also where its events are
 *  raised: provider events from other threads reach it through the gateway of the provider (CTSGNotifier), so the
 *  manager has no locks and its observers do not need any. None of its operations waits for a provider.
 *
 *  Startup policy, AutoLogin(): when autoLogin is on, (1) the providers that can log in silently (platform sessions),
 *  the last used first, (2) the stored refresh tokens of the last used providers, in that order; each step is
 *  given autoLoginStepTimeoutInMs, a step that fails or times out is skipped, and what the steps give is logged in
 *  and linked. When nothing logs in the person stays a guest: the profile key is the identity. AutoLoginFinishedEvent
 *  and the future end the sequence.
 *
 *  The profile is the one of the signer (ILinkSigner): without a signer there are no links, logins still work and the
 *  state stays GUEST. Without stores of its own the manager makes a CFileCredentialStore and a CLocalIdentityLinkStore
 *  from the settings the first time it needs them.
 *
 *  Config: the settings, see CAuthSettings; LoadConfig() also creates the providers of the factory and gives each its
 *  provider settings.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthManager : public CORE::CObservingNotifier ,
                                           public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent LoginSucceededEvent;          /**< CAuthResult: a login of this manager ended well, linked when it was to be */
    static const CORE::CEvent LoginFailedEvent;             /**< CAuthFailure */
    static const CORE::CEvent ActionRequiredEvent;          /**< CAuthActionRequired: forwarded from the provider, show the url or the device code */
    static const CORE::CEvent LoginStateChangedEvent;       /**< CAuthStateEventData */
    static const CORE::CEvent AccountLinkedEvent;           /**< CAuthAccountEventData */
    static const CORE::CEvent AccountUnlinkedEvent;         /**< CAuthAccountEventData */
    static const CORE::CEvent AccountLoggedOutEvent;        /**< CAuthAccountEventData */
    static const CORE::CEvent AutoLoginFinishedEvent;       /**< CAuthStateEventData */
    static const CORE::CEvent SettingsChangedEvent;         /**< the settings changed (the last used providers, the remembered account names): the application saves its configuration */

    static void RegisterEvents( void );

    CAuthManager( void );

    virtual ~CAuthManager();

    /** @name Collaborators, not owned: they have to outlive the manager or be taken away with GUCEF_NULL first. */
    ///@{
    void SetCredentialStore( ICredentialStore* store );
    void SetLinkStore( IIdentityLinkStore* store );
    void SetSigner( ILinkSigner* signer );
    /** The factory the providers come from, the one of CAuthGlobal by default */
    void SetProviderFactory( CAuthProviderFactory* factory );
    ///@}

    /** @name Settings */
    ///@{
    const CAuthSettings& GetSettings( void ) const;

    /** Replaces the settings and gives every provider its part of them */
    void SetSettings( const CAuthSettings& settings );
    ///@}

    /** @name Providers */
    ///@{

    /** Makes a provider of every name in the factory that the manager does not have yet and configures it. Returns how many were made */
    UInt32 RefreshProviders( void );

    /** Takes the provider, under its provider name; false when the name is empty or already there */
    bool AddProvider( const CIAuthProviderPtr& provider );

    bool RemoveProvider( const CORE::CString& providerName );

    CIAuthProvider* GetProvider( const CORE::CString& providerName ) const;

    void GetProviderNames( CORE::CString::StringVector& names ) const;

    /** Whether the provider exists, is available here and now, is enabled in the settings and has its client id when it needs one */
    bool IsProviderOffered( const CORE::CString& providerName ) const;

    /** The offered providers: the last used ones first in their order, then the others by name */
    void GetOfferedProviders( CORE::CString::StringVector& names ) const;

    /**
     *  The providers that exist but are not offered, by name, with the reason and the key of the configuration that fills the
     *  gap, for a screen that shows them disabled instead of leaving them out without a word. Listed are the providers that
     *  are enabled in the settings, are not offered and say why (CIAuthProvider::GetUnavailability(): the providers that are
     *  descriptors, a client id missing at the top of the list), and the descriptors of the registry that are a plugin
     *  (the protocol "platform") when no plugin made the provider. A provider that is disabled by enabledProviders is not
     *  listed. Empty when the setting showUnavailableProviders is off.
     */
    void GetUnavailableProviders( TAuthUnavailableProviderVector& providers ) const;

    /** The reason one provider is not offered; false when it is offered, does not exist, is disabled in the settings or does not say */
    bool GetUnavailableProvider( const CORE::CString& providerName, CAuthUnavailableProvider& unavailable ) const;
    ///@}

    /** @name Logging in */
    ///@{

    /**
     *  Starts an interactive login with the provider. The future ends when the manager is done with the outcome (the
     *  credentials are stored, the link is made) with a CAuthResult, see CAuthResult::TryGetFromFuture(). When it
     *  returns false the login did not start; the future then holds the failure and LoginFailedEvent was raised.
     *  The progress for the user comes as ActionRequiredEvent.
     *
     *  The name of the account is prefilled at the provider when it takes such a parameter (CAuthProviderDescriptor::supportsLoginHint):
     *  options.loginHint when the caller gave one, else the name of the account of the last login at this provider on this machine
     *  (the setting rememberLoginHints, on by default). A provider that takes none gets none.
     */
    bool Login( const CORE::CString& providerName, const CAuthOptions& options, CORE::CFutureResult& future );

    /** Whether there is a remembered name of an account that would be prefilled at a login (the setting rememberLoginHints is on and a login that worked left one) */
    bool HasLoginHints( void ) const;

    /**
     *  Forgets the remembered names of the accounts at all providers, as "Forget remembered account names" does. The next login
     *  starts with an empty field. SettingsChangedEvent is raised when there was something to forget: the application saves its
     *  configuration, which drops them from the file. False when there was nothing
     */
    bool ClearLoginHints( void );

    /** Like Login() and always linking the account to the profile, whatever the options say */
    bool Link( const CORE::CString& providerName, const CAuthOptions& options, CORE::CFutureResult& future );

    /** Asks the login with the provider to stop; it ends as cancelled (or as a timeout when the time was up). False when there is none */
    bool CancelLogin( const CORE::CString& providerName );

    bool IsLoginPending( const CORE::CString& providerName ) const;

    /** Runs the startup policy. The future ends when the sequence is over; a sequence that is running is not started again, its future is given */
    bool AutoLogin( CORE::CFutureResult& future );

    bool IsAutoLoginRunning( void ) const;

    /** Ends a running startup sequence after the step it is in */
    void CancelAutoLogin( void );

    /** Ends the session of the account: revokes the tokens as far as the provider can and forgets them, the link stays. False when the account has no session or stored login */
    bool Logout( const CAccountRef& account );

    /** Removes the link of the account to the profile and ends its session and stored login */
    bool Unlink( const CAccountRef& account );
    ///@}

    /** @name State */
    ///@{

    /** The id of the profile of the signer, empty without a signer */
    CORE::CString GetProfileKeyId( void ) const;

    TAuthLoginState GetLoginState( void ) const;

    /** The accounts with a verified link record to the profile */
    bool GetLinkedAccounts( TLinkRecordVector& links ) const;

    /** The accounts logged in during this run, with what they gave, tokens and proof included */
    void GetSessionAccounts( GUCEF::vector< CAuthResult >& sessions ) const;

    bool GetSession( const CAccountRef& account, CAuthResult& session ) const;

    /** Moves the accounts of another profile to this one, see IIdentityLinkStore::Merge(). The caller has made sure the user is entitled to both */
    TLinkResult MergeProfile( const CORE::CString& fromProfileKeyId, UInt32* mergedCount = GUCEF_NULL );
    ///@}

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAuthManager > TEventCallback;

    void OnPendingTimerUpdate( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void OnProviderActionRequired( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnProviderLoginSucceeded( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    void OnProviderLoginFailed( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    struct SPending
    {
        CORE::CFutureResultSource managerSource;    /**< what the requester waits on */
        CORE::CFutureResult providerFuture;
        UInt64 deadlineMs;                          /**< monotonic; 0 for none */
        UInt64 graceDeadlineMs;                     /**< monotonic, after a cancellation was asked: when the manager ends it itself; 0 before */
        UInt64 readySeenMs;                         /**< monotonic, when the provider's future was seen ended without an event; 0 before */
        bool hasProviderFuture;
        bool linkToProfile;
        bool isAutoStep;
        bool isSilent;
        bool isRefresh;
        bool timedOut;
        CORE::CString refreshAccountKey;

        SPending( void );
    };

    struct SAutoStep
    {
        bool silent;
        CORE::CString providerName;
        CORE::CString accountKey;

        SAutoStep( void );
    };

    typedef GUCEF::map< CORE::CString, SPending > TPendingMap;
    typedef GUCEF::map< CORE::CString, CIAuthProviderPtr > TProviderMap;
    typedef GUCEF::map< CAccountRef, CAuthResult > TSessionMap;
    typedef GUCEF::vector< SAutoStep > TAutoStepVector;

    CAuthManager( const CAuthManager& src );
    CAuthManager& operator=( const CAuthManager& src );

    void EnsureStores( void );
    ICredentialStore* GetCredentialStore( void );
    IIdentityLinkStore* GetLinkStore( void );
    void ConfigureProvider( CIAuthProvider& provider );

    /** Makes the settings that use an earlier name of a provider (CAuthProviderDescriptor::formerNames) the settings of the provider of today */
    void ApplyFormerProviderNames( void );

    bool StartLogin( const CORE::CString& providerName, const CAuthOptions& options, bool forceLink, CORE::CFutureResult& future );
    void ApplyRememberedLoginHint( const CORE::CString& providerName, CAuthOptions& options ) const;
    bool FailImmediately( const CORE::CString& providerName, TAuthFailureReason reason, const CORE::CString& message, CORE::CFutureResult& future );
    void OnProviderSucceeded( const CAuthResult& result );
    void OnProviderFailed( const CAuthFailure& failure );
    void FinishPending( const CORE::CString& providerName, const CAuthResult* result, const CAuthFailure* failure );
    bool LinkAccount( const CAuthResult& result, TAuthFailureReason& failureReason, CORE::CString& failureMessage );
    void CheckPending( void );
    void UpdateTimer( void );

    void RunAutoSteps( void );
    void FinishAutoLogin( void );
    void BuildAutoSteps( void );

    void RaiseStateChangeIfNeeded( TAuthLoginState previous );
    UInt32 CountLinks( void ) const;

    private:

    CAuthSettings m_settings;
    CAuthProviderFactory* m_providerFactory;
    TProviderMap m_providers;
    ICredentialStore* m_credentialStore;
    IIdentityLinkStore* m_linkStore;
    ILinkSigner* m_signer;
    ICredentialStore* m_ownedCredentialStore;
    CFileCredentialStore* m_ownedFileCredentialStore;
    CLocalIdentityLinkStore* m_ownedLinkStore;
    TPendingMap m_pending;
    TSessionMap m_sessions;
    CORE::CTimer m_timer;

    bool m_autoRunning;
    bool m_autoCancelled;
    bool m_autoWaiting;
    size_t m_autoIndex;
    UInt32 m_autoSucceeded;
    CORE::CString m_autoWaitingProvider;
    TAutoStepVector m_autoSteps;
    CORE::CString::StringSet m_autoSilentSucceeded;
    CORE::CFutureResultSource m_autoSource;
    TAuthLoginState m_lastKnownState;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */
