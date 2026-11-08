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

#ifndef GUCEF_AUTH_CACCESSTOKENKEEPER_H
#define GUCEF_AUTH_CACCESSTOKENKEEPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H
#include "gucefAUTH_CAccessTokenKeeperSettings.h"
#define GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H
#endif /* GUCEF_AUTH_CACCESSTOKENKEEPERSETTINGS_H ? */

#ifndef GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H
#include "gucefAUTH_CAccessTokenKeeperEventData.h"
#define GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H
#endif /* GUCEF_AUTH_CACCESSTOKENKEEPEREVENTDATA_H ? */

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

/** A stable lower case name of a status, for logs */
GUCEF_AUTH_EXPORT_CPP const char*
AccessTokenStatusToString( TAccessTokenStatus status );

/*-------------------------------------------------------------------------*/

/** A stable lower case name of a reason, for logs and for an application that shows it */
GUCEF_AUTH_EXPORT_CPP const char*
ReauthReasonToString( TReauthReason reason );

/*-------------------------------------------------------------------------*/

/**
 *  Keeps a valid OAuth 2.0 access token of one account of a provider of a CAuthManager, for code that calls an API with it
 *  (a broker, a data provider, a streaming socket): the token of the session of the manager, renewed with the refresh token of
 *  the credential store before it expires, a rotated refresh token kept, a refused refresh token reported as the need to log in
 *  again, and, when the provider declares how long a refresh token lives, warnings before that time comes.
 *
 *  Threads. GetAccessToken(), RequestRefresh(), ReportTokenRejected(), WaitForAccessToken() and HasLogin() may be called from any
 *  thread; they take an internal lock and never call the manager or the provider. Everything else (the setters, RecordInteractiveLogin(),
 *  ForgetLogin(), Shutdown()) is called on the thread that pumps the pulse generator, which is also where the renewal runs
 *  (CIAuthProvider::Refresh()), where a timer of the keeper drives the proactive renewal and the warnings, and where the events
 *  are raised. A renewal asked for from any thread starts at the next tick of that timer (checkIntervalMs): the timer is
 *  not triggered from the caller's thread, as a pulse generator that is pumped by hand runs such a pulse on the calling thread.
 *
 *  Waiting. RequestRefresh() is the asynchronous form: it gives the future of the renewal that runs, or of one that starts at the
 *  next tick of the timer, so concurrent callers share one renewal. WaitForAccessToken() is the blocking form for a worker thread: it waits
 *  on that future with a timeout and reads the token. The pulse thread never waits: on that thread WaitForAccessToken() answers
 *  at once, as the renewal it would wait for runs on it.
 *
 *  Renewal. A token is renewed when it expires within refreshAheadMs, when a caller reports it refused (once for that token) and
 *  when a caller asks for a token that is gone. A renewal that fails because the network or the provider failed is tried again
 *  after retryDelayMs, doubled for every further failure up to maxRetryDelayMs; TokenRefreshFailedEvent says when. A refresh token
 *  the provider refuses (AUTHFAIL_INVALID_GRANT) is removed from the store, when it is still the one that was sent, and
 *  ReauthRequiredEvent asks for a new login. A new login of the account through the manager gives the keeper its token. No renewal
 *  starts while the manager has a login of the provider pending.
 *
 *  Life of the refresh token. RecordInteractiveLogin() keeps the time of an interactive login in the store (slot of the provider,
 *  account key "<account>#loginAtMs", which is not an account reference, so the startup policy of the manager never takes it for
 *  a login). With a known lifetime the keeper raises ReauthDueSoonEvent once when less than reauthWarnHours remain and
 *  ReauthRequiredEvent once when it has passed; renewals are still tried, as the lifetime is what the provider declares. A
 *  logout or an unlink of the account through the manager removes that slot.
 *
 *  Secrets. Tokens are CSecretString; the log has the provider and what happened, never a token. The manager, the store and
 *  the provider are not owned: they outlive the keeper, or are taken away with GUCEF_NULL first.
 */
class GUCEF_AUTH_EXPORT_CPP CAccessTokenKeeper : public CORE::CObservingNotifier ,
                                                 public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent TokenRefreshedEvent;          /**< CAccessTokenKeeperEventData: a renewal gave a new access token */
    static const CORE::CEvent TokenRefreshFailedEvent;      /**< CAccessTokenKeeperEventData: a renewal failed, with the reason and when it may be tried again */
    static const CORE::CEvent ReauthDueSoonEvent;           /**< CAccessTokenKeeperEventData: the refresh token expires within reauthWarnHours; raised once per login */
    static const CORE::CEvent ReauthRequiredEvent;          /**< CAccessTokenKeeperEventData: the person has to log in again; raised once per login and reason */

    static const char* const LoginTimeKeySuffix;            /**< "#loginAtMs" */

    static void RegisterEvents( void );

    CAccessTokenKeeper( void );

    virtual ~CAccessTokenKeeper() GUCEF_VIRTUAL_OVERRIDE;

    /** @name Collaborators and settings: called on the pulse thread */
    ///@{
    /** The manager that has the provider and the session of the account; its logins, logouts and unlinks of the account reach the keeper */
    void SetManager( CAuthManager* manager );

    /** Where the refresh token of the account is (the store of the manager), where a rotated one is put and where the time of the login is kept */
    void SetCredentialStore( ICredentialStore* store );

    /** The login mechanism ("schwab") and the account. Forgets the token and the failures of the account before; takes the session token of the manager */
    void SetAccount( const CORE::CString& providerName, const CAccountRef& account );

    /** The clock of the expiry times (wall clock, ms since 1970 UTC); GUCEF_NULL for the system clock. For tests */
    void SetClock( const IEpochClock* clock );

    void SetSettings( const CAccessTokenKeeperSettings& settings );
    CAccessTokenKeeperSettings GetSettings( void ) const;

    /** Forgets the token, the failures and the warnings that were given; the account stays */
    void Reset( void );

    /** Ends what runs: a renewal is cancelled, waiters are released (their future ends as cancelled), the token is wiped. Afterwards everything answers NO_LOGIN until SetAccount() */
    void Shutdown( void );
    ///@}

    /** @name The token: any thread */
    ///@{
    /**
     *  The access token when one is ready, with its expiry (ms since 1970 UTC, 0 when not known). A token that is gone starts a renewal at
     *  the next tick of the timer and gives TOKEN_PENDING; within the retry delay of a failed renewal TOKEN_FAILED; no account or nothing to renew
     *  with NO_LOGIN. The token is left empty unless the answer is TOKEN_READY.
     */
    TAccessTokenStatus GetAccessToken( CSecretString& token, UInt64& expiresAtMs );

    /**
     *  The future of the renewal that runs, or of one that starts at the next tick of the timer: callers at the same time share it. It ends as
     *  finished when the renewal gave a token and as failed or cancelled when not; then GetAccessToken() says what there is.
     *  False when no renewal can be started (no account, the retry delay, the refresh token was refused); the future has no future then.
     */
    bool RequestRefresh( CORE::CFutureResult& future );

    /**
     *  The API refused the token (HTTP 401): when it is still the token in hand it is dropped and renewed, once; when the keeper has
     *  another one by now nothing is renewed and the future is finished at once. Returns what RequestRefresh() returns.
     */
    bool ReportTokenRejected( const CSecretString& refusedToken, CORE::CFutureResult& future );

    /**
     *  The blocking form, for worker threads: GetAccessToken(), and while it gives TOKEN_PENDING, waits on the renewal for at most
     *  timeoutInMs and asks again. On the pulse thread it does not wait.
     */
    TAccessTokenStatus WaitForAccessToken( CSecretString& token, UInt64& expiresAtMs, UInt32 timeoutInMs );

    /** Whether an account is set that can get a token: a token is in hand or the store has a refresh token for it */
    bool HasLogin( void ) const;
    ///@}

    /** @name The life of the refresh token: called on the pulse thread */
    ///@{
    /** Keeps the time of an interactive login of the account (0: now) in the store and starts the count of the life of its refresh token again */
    bool RecordInteractiveLogin( UInt64 loginAtEpochMs = 0 );

    /** Removes the time of the login from the store and forgets the token (a logout that went around the manager) */
    bool ForgetLogin( void );

    /** When the refresh token expires by the declared lifetime, ms since 1970 UTC; 0 when the lifetime or the time of the login is not known */
    UInt64 GetRefreshTokenExpiresAtMs( void ) const;

    /** The life of a refresh token in ms that is used: the setting, else the one the descriptor of the provider declares; 0 when not known */
    UInt64 GetRefreshTokenLifetimeMs( void ) const;
    ///@}

    /** @name State, any thread */
    ///@{
    bool IsRefreshing( void ) const;
    bool IsReauthRequired( void ) const;
    TReauthReason GetReauthReason( void ) const;
    UInt32 GetRefreshCount( void ) const;           /**< renewals that were started */
    UInt32 GetFailedRefreshCount( void ) const;     /**< renewals that failed */
    ///@}

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAccessTokenKeeper > TEventCallback;

    CAccessTokenKeeper( const CAccessTokenKeeper& src );
    CAccessTokenKeeper& operator=( const CAccessTokenKeeper& src );

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnManagerLoginSucceeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnManagerAccountGone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    UInt64 Now( void ) const;
    UInt64 EffectiveMarginLocked( UInt32 marginMs ) const;
    bool IsUsableLocked( UInt64 nowMs ) const;
    bool NeedsProactiveRefreshLocked( UInt64 nowMs ) const;
    bool IsInRetryDelayLocked( UInt64 nowMs ) const;
    bool IsBlockedLocked( void ) const;
    bool RequestRefreshLocked( CORE::CFutureResult& future );
    void TakeTokenLocked( const CAuthResult& result, UInt64 nowMs );
    void ClearTokenLocked( void );
    void TakeSessionToken( void );
    void LoadLoginTime( void );
    void UpdateLifetime( void );
    bool CheckStoreForRefreshToken( void ) const;
    void RefreshStoreKnowledge( void );

    void StartRefresh( void );
    void FinishRefresh( bool timedOut );
    void CheckReauth( void );
    void UpdateTimer( void );
    void RaiseEvent( const CORE::CEvent& eventId, const CAccessTokenKeeperEventData& data );
    CAccessTokenKeeperEventData MakeEventDataLocked( void ) const;

    private:

    mutable MT::CMutex m_lock;
    CAuthManager* m_manager;
    ICredentialStore* m_store;
    const IEpochClock* m_clock;
    CAccessTokenKeeperSettings m_settings;
    CORE::CString m_providerName;
    CAccountRef m_account;
    CORE::CTimer m_timer;
    bool m_shutDown;

    CSecretString m_accessToken;
    UInt64 m_expiresAtMs;               /**< of the token in hand, 0 when not known */
    UInt64 m_obtainedAtMs;              /**< when the token in hand was taken, for the margins capped at half its life */
    mutable bool m_hasRefreshToken;     /**< what the store said last */

    bool m_refreshRequested;            /**< a renewal is to start at the next tick of the timer */
    bool m_refreshing;                  /**< the provider renews right now */
    bool m_hasWaitSource;
    CORE::CFutureResultSource m_waitSource;         /**< what the callers of RequestRefresh() wait on */
    CORE::CFutureResult m_providerFuture;
    CSecretString m_sentRefreshToken;
    UInt64 m_refreshStartedAtMonotonicMs;

    UInt32 m_consecutiveFailures;
    UInt64 m_retryAtMs;                 /**< no renewal before this time, 0 for none */

    UInt64 m_loginAtMs;                 /**< the last interactive login, 0 when not known */
    bool m_dueSoonRaised;
    bool m_expiredRaised;
    TReauthReason m_reauthReason;       /**< REAUTH_NONE while no new login is needed */
    UInt64 m_refreshTokenLifetimeMs;    /**< the setting, else what the descriptor declares; 0 when not known */

    UInt32 m_refreshCount;
    UInt32 m_failedRefreshCount;
    TThreadId m_pulseThreadId;             /**< the thread the timer of the keeper runs on, 0 before its first pulse */
    GUCEF_SYSTEM_THREAD_ID_MEMBER( m_pulseSystemThreadId )   /**< display only, recorded with m_pulseThreadId */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CACCESSTOKENKEEPER_H ? */
