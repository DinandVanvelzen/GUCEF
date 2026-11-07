/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestLogCapture.h"
#include "TestAccessTokenKeeper.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt64 HourMs = 3600 * 1000;
const UInt64 T0 = (UInt64) 1800000000 * 1000;          /**< the start of the clock of the tests, ms since 1970 */
const UInt64 LoginLifeMs = 1800 * 1000;                 /**< the life of the access token of a login */

const char* const LoginAccessToken = "SECRET-AT-login-5c1e";
const char* const LoginRefreshToken = "SECRET-RT-login-5c1e";

/** A wall clock that only moves when the test moves it */
class CTestEpochClock : public AUTH::IEpochClock
{
    public:

    explicit CTestEpochClock( UInt64 start ) : AUTH::IEpochClock(), m_lock(), m_now( start ) {}

    virtual UInt64 GetEpochTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        return m_now;
    }

    void Set( UInt64 now )
    {
        MT::CScopeMutex lock( m_lock );
        m_now = now;
    }

    private:

    mutable MT::CMutex m_lock;
    UInt64 m_now;
};

/*-------------------------------------------------------------------------*/

/**
 *  A provider whose renewals the test ends: each Refresh() waits until the test says how it ends, the way a token endpoint
 *  answers or fails. A login ends at once with the tokens the test set.
 */
class CScriptedRefreshProvider : public AUTH::CIAuthProvider
{
    public:

    UInt32 loginCount;
    UInt32 refreshCount;
    bool refuseRefresh;
    CORE::CString lastRefreshToken;
    CORE::CString loginAccessToken;
    CORE::CString loginRefreshToken;
    UInt64 loginExpiresAtMs;
    GUCEF::vector< CORE::CFutureResultSource > pending;

    CScriptedRefreshProvider( void )
        : AUTH::CIAuthProvider()
        , loginCount( 0 )
        , refreshCount( 0 )
        , refuseRefresh( false )
        , lastRefreshToken()
        , loginAccessToken( LoginAccessToken )
        , loginRefreshToken( LoginRefreshToken )
        , loginExpiresAtMs( T0 + LoginLifeMs )
        , pending()
        , m_account()
    {
    }

    virtual ~CScriptedRefreshProvider()
    {
        for ( size_t i=0; i<pending.size(); ++i )
            pending[ i ].Cancel();
    }

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "broker";
        return name;
    }

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_REFRESHABLE; }
    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++loginCount;
        CORE::CFutureResultSource source;
        future = source.GetFuture();
        AUTH::CAuthResult result;
        result.accountRef = AUTH::CAccountRef( "broker", "primary" );
        result.accessToken.Set( loginAccessToken );
        result.refreshToken.Set( loginRefreshToken );
        result.expiresAtMs = loginExpiresAtMs;
        return CompleteLogin( source, result );
    }

    virtual bool Refresh( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++refreshCount;
        lastRefreshToken = refreshToken.Reveal();
        if ( refuseRefresh )
            return false;
        m_account = account;
        CORE::CFutureResultSource source;
        pending.push_back( source );
        future = source.GetFuture();
        return true;
    }

    /** Ends the oldest renewal with the tokens; an empty refresh token is one that did not rotate */
    bool Succeed( const CORE::CString& accessToken, UInt64 expiresAtMs, const CORE::CString& refreshToken )
    {
        if ( pending.empty() )
            return false;
        CORE::CFutureResultSource source = pending.front();
        pending.erase( pending.begin() );
        AUTH::CAuthResult result;
        result.accountRef = m_account;
        result.accessToken.Set( accessToken );
        result.expiresAtMs = expiresAtMs;
        if ( !refreshToken.IsNULLOrEmpty() )
            result.refreshToken.Set( refreshToken );
        return CompleteLogin( source, result );
    }

    bool FailWith( AUTH::TAuthFailureReason reason )
    {
        if ( pending.empty() )
            return false;
        CORE::CFutureResultSource source = pending.front();
        pending.erase( pending.begin() );
        return FailLogin( source, reason, "scripted failure" );
    }

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CScriptedRefreshProvider";
        return classTypeName;
    }

    private:

    AUTH::CAccountRef m_account;
};

typedef CORE::CTFactory< AUTH::CIAuthProvider, CScriptedRefreshProvider, MT::CMutex > TScriptedFactory;

/*-------------------------------------------------------------------------*/

/** Counts the events of a keeper and keeps the data of the last one of each */
class CKeeperEvents : public CORE::CObserver
{
    public:

    UInt32 refreshed;
    UInt32 failed;
    UInt32 dueSoon;
    UInt32 required;
    AUTH::CAccessTokenKeeperEventData lastRefreshed;
    AUTH::CAccessTokenKeeperEventData lastFailed;
    AUTH::CAccessTokenKeeperEventData lastDueSoon;
    AUTH::CAccessTokenKeeperEventData lastRequired;

    CKeeperEvents( void ) : CORE::CObserver(), refreshed( 0 ), failed( 0 ), dueSoon( 0 ), required( 0 ) {}
    virtual ~CKeeperEvents() { UnsubscribeAllFromObserver(); }

    void Attach( AUTH::CAccessTokenKeeper& keeper )
    {
        typedef CORE::CTEventHandlerFunctor< CKeeperEvents > TCallback;
        TCallback refreshedCallback( this, &CKeeperEvents::OnRefreshed );
        SubscribeTo( &keeper, AUTH::CAccessTokenKeeper::TokenRefreshedEvent, refreshedCallback );
        TCallback failedCallback( this, &CKeeperEvents::OnFailed );
        SubscribeTo( &keeper, AUTH::CAccessTokenKeeper::TokenRefreshFailedEvent, failedCallback );
        TCallback dueSoonCallback( this, &CKeeperEvents::OnDueSoon );
        SubscribeTo( &keeper, AUTH::CAccessTokenKeeper::ReauthDueSoonEvent, dueSoonCallback );
        TCallback requiredCallback( this, &CKeeperEvents::OnRequired );
        SubscribeTo( &keeper, AUTH::CAccessTokenKeeper::ReauthRequiredEvent, requiredCallback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CKeeperEvents";
        return classTypeName;
    }

    private:

    static void Take( CORE::CICloneable* eventData, AUTH::CAccessTokenKeeperEventData& into )
    {
        const AUTH::CAccessTokenKeeperEventData* data = static_cast< const AUTH::CAccessTokenKeeperEventData* >( eventData );
        if ( GUCEF_NULL != data )
            into = *data;
    }

    void OnRefreshed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++refreshed; Take( eventData, lastRefreshed ); }
    void OnFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++failed; Take( eventData, lastFailed ); }
    void OnDueSoon( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++dueSoon; Take( eventData, lastDueSoon ); }
    void OnRequired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++required; Take( eventData, lastRequired ); }
};

/*-------------------------------------------------------------------------*/

/** A worker thread that asks the keeper for a token with the blocking form */
class CTokenWorker : public MT::CActiveObject
{
    public:

    CTokenWorker( AUTH::CAccessTokenKeeper& keeper, UInt32 timeoutInMs )
        : MT::CActiveObject(), m_keeper( keeper ), m_timeoutInMs( timeoutInMs ), m_lock(), m_done( false ), m_status( AUTH::NO_LOGIN ), m_token()
    {
    }

    bool IsDone( void ) const { MT::CScopeMutex lock( m_lock ); return m_done; }
    AUTH::TAccessTokenStatus GetStatus( void ) const { MT::CScopeMutex lock( m_lock ); return m_status; }
    CORE::CString GetToken( void ) const { MT::CScopeMutex lock( m_lock ); return m_token; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        AUTH::CSecretString token;
        UInt64 expiresAtMs = 0;
        AUTH::TAccessTokenStatus status = m_keeper.WaitForAccessToken( token, expiresAtMs, m_timeoutInMs );
        MT::CScopeMutex lock( m_lock );
        m_status = status;
        m_token = token.Reveal();
        m_done = true;
        return true;
    }

    private:

    AUTH::CAccessTokenKeeper& m_keeper;
    UInt32 m_timeoutInMs;
    mutable MT::CMutex m_lock;
    bool m_done;
    AUTH::TAccessTokenStatus m_status;
    CORE::CString m_token;
};

/*-------------------------------------------------------------------------*/

/** The settings of the tests: the clock is the test's, the timers run fast */
AUTH::CAccessTokenKeeperSettings
TestKeeperSettings( void )
{GUCEF_TRACE;

    AUTH::CAccessTokenKeeperSettings settings;
    settings.refreshAheadMs = 120000;
    settings.expirySkewMs = 30000;
    settings.retryDelayMs = 1000;
    settings.maxRetryDelayMs = 4000;
    settings.reauthWarnHours = 24;
    settings.checkIntervalMs = 2;
    settings.pollIntervalMs = 1;
    settings.refreshTimeoutMs = 60000;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** A manager with the scripted provider "broker", a store in memory, a clock of the test and a keeper on them */
class CKeeperFixture
{
    public:

    CTestEpochClock clock;
    AUTH::CMemoryCredentialStore store;
    AUTH::CAuthManager manager;
    CScriptedRefreshProvider* provider;
    AUTH::CAccessTokenKeeper keeper;
    CKeeperEvents events;
    AUTH::CAccountRef account;

    CKeeperFixture( void )
        : clock( T0 ), store(), manager(), provider( GUCEF_NULL ), keeper(), events(), account( "broker", "primary" )
    {
        manager.SetCredentialStore( &store );
        static TScriptedFactory factory;        // outlives the providers it made
        TScriptedFactory::TProductPtr product = factory.Create();
        provider = static_cast< CScriptedRefreshProvider* >( product.GetPointerAlways() );
        manager.AddProvider( product );
        keeper.SetClock( &clock );
        keeper.SetSettings( TestKeeperSettings() );
        keeper.SetManager( &manager );
        keeper.SetCredentialStore( &store );
        events.Attach( keeper );
    }

    ~CKeeperFixture()
    {
        keeper.Shutdown();
        keeper.SetManager( GUCEF_NULL );
    }

    /** An interactive login with the provider through the manager: the manager keeps the refresh token in the store */
    bool LogIn( void )
    {
        AUTH::CAuthOptions options;
        options.linkToProfile = false;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        if ( !manager.Login( "broker", options, future ) )
            return false;
        for ( UInt32 waited=0; waited<5000 && !future.IsReady(); ++waited )
            PumpFor( 1 );
        PumpFor( 5 );
        AUTH::CAuthResult result;
        return AUTH::CAuthResult::TryGetFromFuture( future, result ) && result.accountRef == account;
    }

    void UseAccount( void )
    {
        keeper.SetAccount( "broker", account );
    }

    AUTH::TAccessTokenStatus Ask( CORE::CString& token )
    {
        AUTH::CSecretString secret;
        UInt64 expiresAtMs = 0;
        AUTH::TAccessTokenStatus status = keeper.GetAccessToken( secret, expiresAtMs );
        token = secret.Reveal();
        return status;
    }

    AUTH::TAccessTokenStatus Ask( void )
    {
        CORE::CString token;
        return Ask( token );
    }

    /** Pumps until the provider was asked to renew the given number of times */
    bool PumpUntilRefreshes( UInt32 count, UInt32 timeoutInMs = 3000 )
    {
        return PumpUntilAtLeast( provider->refreshCount, count, timeoutInMs );
    }

    CORE::CString StoredRefreshToken( void )
    {
        AUTH::CSecretString secret;
        store.Load( "broker", account.ToString(), secret );
        return secret.Reveal();
    }

    private:

    CKeeperFixture( const CKeeperFixture& src );
    CKeeperFixture& operator=( const CKeeperFixture& src );
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformKeeperBasicTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The keeper gives the token of the session of the manager without a renewal; without an account or anything stored there is no login" )
        try
        {
            CKeeperFixture f;
            CORE::CString token;
            ASSERT_TRUE( AUTH::NO_LOGIN == f.Ask( token ) && token.IsNULLOrEmpty() );
            ASSERT_FALSE( f.keeper.HasLogin() );
            f.UseAccount();
            ASSERT_TRUE( AUTH::NO_LOGIN == f.Ask( token ) );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.keeper.RequestRefresh( future ) );
            ASSERT_TRUE( future.HasNoFuture() );

            // the login of the account reaches the keeper through the manager
            ASSERT_TRUE( f.LogIn() );
            AUTH::CSecretString secret;
            UInt64 expiresAtMs = 0;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.keeper.GetAccessToken( secret, expiresAtMs ) );
            ASSERT_TRUE( LoginAccessToken == secret.Reveal() && T0 + LoginLifeMs == expiresAtMs );
            ASSERT_TRUE( f.keeper.HasLogin() );
            PumpFor( 30 );
            ASSERT_TRUE( 0 == f.provider->refreshCount && 0 == f.events.refreshed );

            // a keeper set up after the login takes the session as well
            AUTH::CAccessTokenKeeper later;
            later.SetClock( &f.clock );
            later.SetManager( &f.manager );
            later.SetCredentialStore( &f.store );
            later.SetAccount( "broker", f.account );
            ASSERT_TRUE( AUTH::TOKEN_READY == later.GetAccessToken( secret, expiresAtMs ) && LoginAccessToken == secret.Reveal() );
            // another account of the provider is not this one
            later.SetAccount( "broker", AUTH::CAccountRef( "broker", "other" ) );
            ASSERT_TRUE( AUTH::NO_LOGIN == later.GetAccessToken( secret, expiresAtMs ) && secret.IsEmpty() );
            later.SetManager( GUCEF_NULL );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A renewal starts exactly refreshAheadMs before the expiry, the old token is handed out meanwhile and the new one after" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            const UInt64 expiry = T0 + LoginLifeMs;

            f.clock.Set( expiry - 120001 );
            PumpFor( 30 );
            ASSERT_TRUE( 0 == f.provider->refreshCount && !f.keeper.IsRefreshing() );

            f.clock.Set( expiry - 120000 );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            ASSERT_TRUE( f.keeper.IsRefreshing() && LoginRefreshToken == f.provider->lastRefreshToken );
            CORE::CString token;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && LoginAccessToken == token );

            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-2", expiry + LoginLifeMs, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
            AUTH::CSecretString secret;
            UInt64 expiresAtMs = 0;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.keeper.GetAccessToken( secret, expiresAtMs ) && "SECRET-AT-2" == secret.Reveal() && expiry + LoginLifeMs == expiresAtMs );
            ASSERT_TRUE( expiry + LoginLifeMs == f.events.lastRefreshed.accessTokenExpiresAtMs && f.account == f.events.lastRefreshed.accountRef && "broker" == f.events.lastRefreshed.providerName );
            ASSERT_FALSE( f.keeper.IsRefreshing() );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == f.provider->refreshCount && 1 == f.keeper.GetRefreshCount() && 0 == f.keeper.GetFailedRefreshCount() );
            ASSERT_TRUE( LoginRefreshToken == f.StoredRefreshToken() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token is not handed out within expirySkewMs of its expiry; the margins are capped at half the life of a short token; a token without an expiry never expires" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            const UInt64 expiry = T0 + LoginLifeMs;
            CORE::CString token;

            f.clock.Set( expiry - 30001 );
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && LoginAccessToken == token );
            f.clock.Set( expiry - 30000 );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask( token ) && token.IsNULLOrEmpty() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );

            // a token that lives 40 seconds: both margins become 20 seconds, else it would be renewed the moment it arrives
            const UInt64 now = expiry - 30000;
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-short", now + 40000, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
            f.clock.Set( now + 19999 );
            PumpFor( 30 );
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-short" == token );
            ASSERT_TRUE( 1 == f.provider->refreshCount );
            f.clock.Set( now + 20000 );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask( token ) );
            ASSERT_TRUE( f.PumpUntilRefreshes( 2 ) );

            // an expiry that is not known: the token is good until it is refused
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-forever", 0, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 2, 3000 ) );
            f.clock.Set( now + 400 * HourMs );
            PumpFor( 30 );
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-forever" == token && 2 == f.provider->refreshCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Callers on several threads at once share one renewal and get the same token; RequestRefresh gives them one future; the pulse thread does not wait" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            f.clock.Set( T0 + LoginLifeMs );

            // the renewal runs on this thread, so this thread does not wait for it
            AUTH::CSecretString secret;
            UInt64 expiresAtMs = 0;
            UInt64 startedAt = AUTH::GetMonotonicTimeInMs();
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.keeper.WaitForAccessToken( secret, expiresAtMs, 5000 ) );
            ASSERT_TRUE( AUTH::GetMonotonicTimeInMs() - startedAt < 1000 );

            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.keeper.RequestRefresh( first ) && f.keeper.RequestRefresh( second ) );
            ASSERT_TRUE( 0 != first.GetTaskId() && first.GetTaskId() == second.GetTaskId() );

            const UInt32 WorkerCount = 8;
            CTokenWorker* workers[ WorkerCount ];
            for ( UInt32 i=0; i<WorkerCount; ++i )
            {
                workers[ i ] = GUCEF_NEW CTokenWorker( f.keeper, 20000 );
                ASSERT_TRUE( workers[ i ]->Activate( GUCEF_NULL, 1, 1 ) );
            }
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            PumpFor( 100 );
            ASSERT_TRUE( 1 == f.provider->refreshCount && 1 == f.provider->pending.size() );
            for ( UInt32 i=0; i<WorkerCount; ++i )
                ASSERT_FALSE( workers[ i ]->IsDone() );

            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-shared", T0 + 2 * LoginLifeMs, CORE::CString::Empty ) );
            bool allDone = false;
            for ( UInt32 waited=0; waited<10000 && !allDone; ++waited )
            {
                PumpFor( 1 );
                allDone = true;
                for ( UInt32 i=0; i<WorkerCount; ++i )
                    allDone = allDone && workers[ i ]->IsDone();
            }
            ASSERT_TRUE( allDone );
            for ( UInt32 i=0; i<WorkerCount; ++i )
            {
                ASSERT_TRUE( AUTH::TOKEN_READY == workers[ i ]->GetStatus() && "SECRET-AT-shared" == workers[ i ]->GetToken() );
                workers[ i ]->Deactivate( false, true );
                GUCEF_DELETE workers[ i ];
            }
            ASSERT_TRUE( first.IsReady() && !first.HasNoFuture() );
            ASSERT_TRUE( 1 == f.provider->refreshCount && 1 == f.events.refreshed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A rotated refresh token replaces the stored one and is the one sent next; one that did not rotate stays" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            ASSERT_TRUE( LoginRefreshToken == f.StoredRefreshToken() );

            f.clock.Set( T0 + LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) && LoginRefreshToken == f.provider->lastRefreshToken );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-2", T0 + 2 * LoginLifeMs, "SECRET-RT-2" ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
            ASSERT_TRUE( "SECRET-RT-2" == f.StoredRefreshToken() );

            f.clock.Set( T0 + 2 * LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 2 ) && "SECRET-RT-2" == f.provider->lastRefreshToken );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-3", T0 + 3 * LoginLifeMs, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 2, 3000 ) );
            ASSERT_TRUE( "SECRET-RT-2" == f.StoredRefreshToken() );

            f.clock.Set( T0 + 3 * LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 3 ) );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-4", T0 + 4 * LoginLifeMs, "SECRET-RT-2" ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 3, 3000 ) );
            ASSERT_TRUE( "SECRET-RT-2" == f.StoredRefreshToken() );
            CORE::CString token;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-4" == token );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A refused refresh token is removed from the store, raises the failure and the need to log in again once, and nothing is renewed until a new login" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            f.clock.Set( T0 + LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_INVALID_GRANT ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.required, 1, 3000 ) );
            ASSERT_TRUE( 1 == f.events.failed && AUTH::AUTHFAIL_INVALID_GRANT == f.events.lastFailed.failureReason && 0 == f.events.lastFailed.retryAtMs );
            ASSERT_TRUE( AUTH::REAUTH_REFRESH_TOKEN_REJECTED == f.events.lastRequired.reauthReason );
            ASSERT_TRUE( f.keeper.IsReauthRequired() && AUTH::REAUTH_REFRESH_TOKEN_REJECTED == f.keeper.GetReauthReason() );
            ASSERT_TRUE( f.StoredRefreshToken().IsNULLOrEmpty() );
            ASSERT_TRUE( AUTH::NO_LOGIN == f.Ask() );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.keeper.RequestRefresh( future ) );
            ASSERT_TRUE( future.HasNoFuture() );
            PumpFor( 50 );
            ASSERT_TRUE( 1 == f.provider->refreshCount && 1 == f.events.required && 1 == f.keeper.GetFailedRefreshCount() );

            // a new login of the account through the manager ends it
            f.provider->loginAccessToken = "SECRET-AT-again";
            f.provider->loginExpiresAtMs = T0 + 3 * LoginLifeMs;
            ASSERT_TRUE( f.LogIn() );
            CORE::CString token;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-again" == token );
            ASSERT_FALSE( f.keeper.IsReauthRequired() );

            // a refusal of a refresh token that a new login replaced meanwhile does not remove the new one
            f.clock.Set( T0 + 3 * LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 2 ) );
            ASSERT_TRUE( f.store.Store( "broker", f.account.ToString(), AUTH::CSecretString( "SECRET-RT-newer" ) ) );
            ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_INVALID_GRANT ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.required, 2, 3000 ) );
            ASSERT_TRUE( "SECRET-RT-newer" == f.StoredRefreshToken() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A renewal that fails on the network is tried again after the retry delay, doubled up to the cap and reset by a success; meanwhile the token is failed" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            UInt64 now = T0 + LoginLifeMs;
            f.clock.Set( now );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );

            const UInt64 delays[] = { 1000, 2000, 4000, 4000 };
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_NETWORK ) );
                ASSERT_TRUE( PumpUntilAtLeast( f.events.failed, i + 1, 3000 ) );
                ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == f.events.lastFailed.failureReason && now + delays[ i ] == f.events.lastFailed.retryAtMs );
                ASSERT_TRUE( AUTH::TOKEN_FAILED == f.Ask() );
                CORE::CFutureResult refused( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( f.keeper.RequestRefresh( refused ) );
                f.clock.Set( now + delays[ i ] - 1 );
                PumpFor( 20 );
                ASSERT_TRUE( AUTH::TOKEN_FAILED == f.Ask() && i + 1 == f.provider->refreshCount );
                // the timer tries again by itself when the delay is over
                now += delays[ i ];
                f.clock.Set( now );
                ASSERT_TRUE( f.PumpUntilRefreshes( i + 2 ) );
                ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            }
            ASSERT_FALSE( f.keeper.IsReauthRequired() );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-back", now + LoginLifeMs, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask() );
            // a success starts the count again
            now += LoginLifeMs;
            f.clock.Set( now );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 6 ) );
            ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_NETWORK ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.failed, 5, 3000 ) && now + 1000 == f.events.lastFailed.retryAtMs );

            // a provider that cannot start a renewal is a failure with the delay as well
            f.provider->refuseRefresh = true;
            now += 1000;
            f.clock.Set( now );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.failed, 6, 3000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == f.events.lastFailed.failureReason && now + 2000 == f.events.lastFailed.retryAtMs );
            ASSERT_FALSE( f.keeper.IsReauthRequired() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token the API refused is renewed once: a second report of it joins that renewal, a report of a token renewed meanwhile gives the new one at once" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            CORE::CString token;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && LoginAccessToken == token );

            CORE::CFutureResult first( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.keeper.ReportTokenRejected( AUTH::CSecretString( LoginAccessToken ), first ) );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.keeper.ReportTokenRejected( AUTH::CSecretString( LoginAccessToken ), second ) && first.GetTaskId() == second.GetTaskId() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-2", T0 + 2 * LoginLifeMs, CORE::CString::Empty ) );
            for ( UInt32 waited=0; waited<3000 && !first.IsReady(); ++waited )
                PumpFor( 1 );
            ASSERT_TRUE( first.IsReady() && !first.HasNoFuture() );

            CORE::CFutureResult late( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( f.keeper.ReportTokenRejected( AUTH::CSecretString( LoginAccessToken ), late ) );
            ASSERT_TRUE( late.IsReady() && !late.HasNoFuture() );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == f.provider->refreshCount );
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-2" == token );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformKeeperLifetimeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The life of a refresh token: due soon once within reauthWarnHours, required once at its end, counted from the interactive login kept in the store, and a logout removes it" )
        try
        {
            CKeeperFixture f;
            AUTH::CAccessTokenKeeperSettings settings = TestKeeperSettings();
            settings.refreshTokenLifetimeHours = 168;
            f.keeper.SetSettings( settings );
            f.provider->loginExpiresAtMs = 0;               // the access token does not get in the way here
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            ASSERT_TRUE( 168 * HourMs == f.keeper.GetRefreshTokenLifetimeMs() );
            ASSERT_TRUE( 0 == f.keeper.GetRefreshTokenExpiresAtMs() );         // no login time known: nothing to warn about
            ASSERT_TRUE( f.keeper.RecordInteractiveLogin( T0 ) );
            AUTH::CSecretString kept;
            ASSERT_TRUE( f.store.Load( "broker", "broker:primary#loginAtMs", kept ) && CORE::UInt64ToString( T0 ) == kept.Reveal() );
            ASSERT_TRUE( T0 + 168 * HourMs == f.keeper.GetRefreshTokenExpiresAtMs() );
            // the slot is no account: the startup policy of the manager never takes it for a login
            AUTH::CAccountRef notAnAccount;
            ASSERT_FALSE( AUTH::CAccountRef::TryParse( "broker:primary#loginAtMs", notAnAccount ) );

            f.clock.Set( T0 + 144 * HourMs - 1 );
            PumpFor( 30 );
            ASSERT_TRUE( 0 == f.events.dueSoon && 0 == f.events.required );
            f.clock.Set( T0 + 144 * HourMs );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.dueSoon, 1, 3000 ) );
            ASSERT_TRUE( 24 * HourMs == f.events.lastDueSoon.remainingMs && T0 + 168 * HourMs == f.events.lastDueSoon.refreshTokenExpiresAtMs );
            f.clock.Set( T0 + 168 * HourMs - 1 );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == f.events.dueSoon && 0 == f.events.required && !f.keeper.IsReauthRequired() );
            f.clock.Set( T0 + 168 * HourMs );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.required, 1, 3000 ) );
            ASSERT_TRUE( AUTH::REAUTH_REFRESH_TOKEN_EXPIRED == f.events.lastRequired.reauthReason && T0 + 168 * HourMs == f.events.lastRequired.refreshTokenExpiresAtMs );
            ASSERT_TRUE( f.keeper.IsReauthRequired() && AUTH::REAUTH_REFRESH_TOKEN_EXPIRED == f.keeper.GetReauthReason() );
            // the declared life is what the provider says: the token in hand is still given, renewals are still tried
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask() );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == f.events.dueSoon && 1 == f.events.required );

            // another keeper, as after a restart, reads the time of the login from the store
            AUTH::CAccessTokenKeeper restarted;
            restarted.SetClock( &f.clock );
            restarted.SetSettings( settings );
            restarted.SetCredentialStore( &f.store );
            restarted.SetAccount( "broker", f.account );
            ASSERT_TRUE( T0 + 168 * HourMs == restarted.GetRefreshTokenExpiresAtMs() );
            restarted.Shutdown();

            // a new interactive login starts the count again
            const UInt64 again = T0 + 170 * HourMs;
            f.clock.Set( again );
            ASSERT_TRUE( f.keeper.RecordInteractiveLogin() );
            ASSERT_TRUE( !f.keeper.IsReauthRequired() && again + 168 * HourMs == f.keeper.GetRefreshTokenExpiresAtMs() );
            PumpFor( 30 );
            ASSERT_TRUE( 1 == f.events.dueSoon && 1 == f.events.required );
            f.clock.Set( again + 150 * HourMs );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.dueSoon, 2, 3000 ) && 18 * HourMs == f.events.lastDueSoon.remainingMs );

            // a logout through the manager takes the time of the login away with the refresh token
            ASSERT_TRUE( f.manager.Logout( f.account ) );
            PumpFor( 10 );
            ASSERT_FALSE( f.store.Load( "broker", "broker:primary#loginAtMs", kept ) );
            ASSERT_TRUE( 0 == f.keeper.GetRefreshTokenExpiresAtMs() && AUTH::NO_LOGIN == f.Ask() && !f.keeper.HasLogin() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Without a setting the life of a refresh token is the one the descriptor of the provider declares; a lifetime that is not known gives no warnings" )
        try
        {
            CKeeperFixture f;
            f.provider->loginExpiresAtMs = 0;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            ASSERT_TRUE( 0 == f.keeper.GetRefreshTokenLifetimeMs() );
            ASSERT_TRUE( f.keeper.RecordInteractiveLogin( T0 ) && 0 == f.keeper.GetRefreshTokenExpiresAtMs() );
            f.clock.Set( T0 + 1000 * HourMs );
            PumpFor( 30 );
            ASSERT_TRUE( 0 == f.events.dueSoon && 0 == f.events.required );

            AUTH::CAuthProviderDescriptor descriptor;
            descriptor.name = "broker";
            descriptor.displayName = "Broker";
            descriptor.accountProviderId = "broker";
            descriptor.protocol = AUTH::AUTHPROTOCOL_OAUTH2;
            descriptor.authorizationEndpoint = "https://broker.example/authorize";
            descriptor.tokenEndpoint = "https://broker.example/token";
            descriptor.accountSource = AUTH::AUTHACCOUNTSOURCE_PARAMETERS;
            descriptor.subjectTemplate = "{accountAlias}";
            descriptor.defaultParameters[ "accountAlias" ] = "primary";
            descriptor.refreshTokenLifetimeInSeconds = 7200;
            AUTH::CAuthProviderRegistry& registry = AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry();
            ASSERT_TRUE( registry.RegisterDescriptor( descriptor ) );
            f.UseAccount();
            ASSERT_TRUE( 2 * HourMs == f.keeper.GetRefreshTokenLifetimeMs() );
            ASSERT_TRUE( f.keeper.RecordInteractiveLogin() );
            ASSERT_TRUE( f.clock.GetEpochTimeInMs() + 2 * HourMs == f.keeper.GetRefreshTokenExpiresAtMs() );
            // within the warning time from the start: due soon at once
            ASSERT_TRUE( PumpUntilAtLeast( f.events.dueSoon, 1, 3000 ) && 2 * HourMs == f.events.lastDueSoon.remainingMs );
            // the setting wins over the descriptor
            AUTH::CAccessTokenKeeperSettings settings = TestKeeperSettings();
            settings.refreshTokenLifetimeHours = 5;
            f.keeper.SetSettings( settings );
            ASSERT_TRUE( 5 * HourMs == f.keeper.GetRefreshTokenLifetimeMs() );
            registry.UnregisterDescriptor( "broker" );
        }
        catch ( ... )
        {
            AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "broker" );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformKeeperShutdownTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Shutdown while a renewal runs releases the waiting threads, cancels the renewal at the provider and drops its late answer; a keeper destroyed mid renewal leaves nothing behind" )
        try
        {
            CKeeperFixture f;
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            f.clock.Set( T0 + LoginLifeMs );
            CTokenWorker worker( f.keeper, 30000 );
            ASSERT_TRUE( worker.Activate( GUCEF_NULL, 1, 1 ) );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            PumpFor( 50 );
            ASSERT_FALSE( worker.IsDone() );
            ASSERT_TRUE( 1 == f.provider->pending.size() );

            f.keeper.Shutdown();
            UInt64 startedAt = AUTH::GetMonotonicTimeInMs();
            for ( UInt32 waited=0; waited<5000 && !worker.IsDone(); ++waited )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( worker.IsDone() && AUTH::GetMonotonicTimeInMs() - startedAt < 2000 );
            ASSERT_TRUE( AUTH::TOKEN_READY != worker.GetStatus() && worker.GetToken().IsNULLOrEmpty() );
            worker.Deactivate( false, true );
            ASSERT_TRUE( f.provider->pending.front().IsCancellationRequested() );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-late", T0 + 2 * LoginLifeMs, "SECRET-RT-late" ) );
            PumpFor( 30 );
            ASSERT_TRUE( 0 == f.events.refreshed && 0 == f.events.failed );
            ASSERT_TRUE( AUTH::NO_LOGIN == f.Ask() && !f.keeper.HasLogin() );
            ASSERT_TRUE( LoginRefreshToken == f.StoredRefreshToken() );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( f.keeper.RequestRefresh( future ) );

            // set up again it works again
            f.UseAccount();
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 2 ) );
            ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-revived", T0 + 2 * LoginLifeMs, CORE::CString::Empty ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
            CORE::CString token;
            ASSERT_TRUE( AUTH::TOKEN_READY == f.Ask( token ) && "SECRET-AT-revived" == token );

            // a keeper that goes away in the middle of a renewal (the one of the fixture is out of the way)
            f.keeper.Shutdown();
            AUTH::CAccessTokenKeeper* gone = GUCEF_NEW AUTH::CAccessTokenKeeper();
            gone->SetClock( &f.clock );
            gone->SetSettings( TestKeeperSettings() );
            gone->SetManager( &f.manager );
            gone->SetCredentialStore( &f.store );
            gone->SetAccount( "broker", f.account );
            f.clock.Set( T0 + 3 * LoginLifeMs );
            CORE::CFutureResult waiting( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( gone->RequestRefresh( waiting ) );
            ASSERT_TRUE( f.PumpUntilRefreshes( 3 ) );
            size_t index = f.provider->pending.size() - 1;
            GUCEF_DELETE gone;
            gone = GUCEF_NULL;
            ASSERT_TRUE( waiting.IsReady() );
            ASSERT_TRUE( f.provider->pending[ index ].IsCancellationRequested() );
            while ( !f.provider->pending.empty() )
                f.provider->Succeed( "SECRET-AT-nobody", T0 + 4 * LoginLifeMs, CORE::CString::Empty );
            PumpFor( 30 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A renewal that does not end within refreshTimeoutMs is cancelled and counts as a failure of the network" )
        try
        {
            CKeeperFixture f;
            AUTH::CAccessTokenKeeperSettings settings = TestKeeperSettings();
            settings.refreshTimeoutMs = 50;
            f.keeper.SetSettings( settings );
            ASSERT_TRUE( f.LogIn() );
            f.UseAccount();
            f.clock.Set( T0 + LoginLifeMs );
            ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
            ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
            ASSERT_TRUE( PumpUntilAtLeast( f.events.failed, 1, 3000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == f.events.lastFailed.failureReason && T0 + LoginLifeMs + 1000 == f.events.lastFailed.retryAtMs );
            ASSERT_TRUE( f.provider->pending.front().IsCancellationRequested() );
            ASSERT_TRUE( AUTH::TOKEN_FAILED == f.Ask() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings of the keeper come from a config and go back to it, with the margins capped and the intervals above zero" )
        try
        {
            AUTH::CAccessTokenKeeperSettings settings;
            ASSERT_TRUE( 120000 == settings.refreshAheadMs && 30000 == settings.expirySkewMs && 30000 == settings.retryDelayMs && 0 == settings.refreshTokenLifetimeHours && 24 == settings.reauthWarnHours );
            settings.refreshAheadMs = 90000;
            settings.refreshTokenLifetimeHours = 168;
            settings.reauthWarnHours = 12;
            CORE::CDataNode node( "TokenKeeper", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( node ) );
            AUTH::CAccessTokenKeeperSettings back;
            ASSERT_TRUE( back.LoadConfig( node ) );
            ASSERT_TRUE( 90000 == back.refreshAheadMs && 168 == back.refreshTokenLifetimeHours && 12 == back.reauthWarnHours && back.retryDelayMs == settings.retryDelayMs );

            CORE::CDataNode odd( "TokenKeeper", GUCEF_DATATYPE_OBJECT );
            odd.SetAttribute( "refreshAheadMs", "999999999" );
            odd.SetAttribute( "pollIntervalMs", "0" );
            odd.SetAttribute( "checkIntervalMs", "0" );
            odd.SetAttribute( "retryDelayMs", "5000" );
            odd.SetAttribute( "maxRetryDelayMs", "10" );
            ASSERT_TRUE( back.LoadConfig( odd ) );
            ASSERT_TRUE( 24 * HourMs == back.refreshAheadMs && 1 == back.pollIntervalMs && 1 == back.checkIntervalMs && 5000 == back.maxRetryDelayMs );

            AUTH::CAccessTokenKeeper keeper;
            ASSERT_TRUE( keeper.LoadConfig( node ) && 168 == keeper.GetSettings().refreshTokenLifetimeHours );
            CORE::CDataNode saved( "TokenKeeper", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( keeper.SaveConfig( saved ) && "90000" == saved.GetAttributeValueOrChildValueByName( "refreshAheadMs" ).AsString() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No access token or refresh token reaches the log in renewals that work, rotate, fail and are refused" )
        try
        {
            CLogCapture log;
            {
                CKeeperFixture f;
                AUTH::CAccessTokenKeeperSettings settings = TestKeeperSettings();
                settings.refreshTokenLifetimeHours = 2;          // a warning is in the log too
                f.keeper.SetSettings( settings );
                ASSERT_TRUE( f.LogIn() );
                f.UseAccount();
                ASSERT_TRUE( f.keeper.RecordInteractiveLogin( T0 ) );
                f.clock.Set( T0 + LoginLifeMs );
                ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
                ASSERT_TRUE( f.PumpUntilRefreshes( 1 ) );
                ASSERT_TRUE( f.provider->Succeed( "SECRET-AT-logged-2", T0 + 2 * LoginLifeMs, "SECRET-RT-logged-2" ) );
                ASSERT_TRUE( PumpUntilAtLeast( f.events.refreshed, 1, 3000 ) );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( f.keeper.ReportTokenRejected( AUTH::CSecretString( "SECRET-AT-logged-2" ), future ) );
                ASSERT_TRUE( f.PumpUntilRefreshes( 2 ) );
                ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_NETWORK ) );
                ASSERT_TRUE( PumpUntilAtLeast( f.events.failed, 1, 3000 ) );
                f.clock.Set( T0 + 2 * LoginLifeMs );
                ASSERT_TRUE( AUTH::TOKEN_PENDING == f.Ask() );
                ASSERT_TRUE( f.PumpUntilRefreshes( 3 ) );
                ASSERT_TRUE( f.provider->FailWith( AUTH::AUTHFAIL_INVALID_GRANT ) );
                ASSERT_TRUE( PumpUntilAtLeast( f.events.required, 1, 3000 ) );
            }
            CORE::CString text = log.Text();
            ASSERT_TRUE( text.HasSubstr( "AccessTokenKeeper: broker: the access token was renewed" ) >= 0 );
            ASSERT_TRUE( text.HasSubstr( "AccessTokenKeeper: broker: the refresh token was refused" ) >= 0 );
            ASSERT_TRUE( text.HasSubstr( "AccessTokenKeeper: broker: the refresh token expires in" ) >= 0 );
            ASSERT_TRUE( text.HasSubstr( "SECRET-" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAccessTokenKeeperTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING access token keeper TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AccessTokenKeeper" );

    PerformKeeperBasicTests();
    PerformKeeperLifetimeTests();
    PerformKeeperShutdownTests();
}

/*-------------------------------------------------------------------------*/
