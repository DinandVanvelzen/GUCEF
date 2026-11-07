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

#include "gucefAUTH_CAccessTokenKeeper.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
AccessTokenStatusToString( TAccessTokenStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case TOKEN_READY : return "ready";
        case TOKEN_PENDING : return "pending";
        case TOKEN_FAILED : return "failed";
        default : return "no-login";
    }
}

/*-------------------------------------------------------------------------*/

const char*
ReauthReasonToString( TReauthReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case REAUTH_REFRESH_TOKEN_EXPIRED : return "refresh-token-expired";
        case REAUTH_REFRESH_TOKEN_REJECTED : return "refresh-token-rejected";
        case REAUTH_NO_REFRESH_TOKEN : return "no-refresh-token";
        default : return "none";
    }
}

/*-------------------------------------------------------------------------*/

/** The reason of a login future that ended without a result: the text before the first ':' of the extra information (CIAuthProvider::FailLogin()) */
static TAuthFailureReason
FailureReasonOfFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return AUTHFAIL_UNKNOWN;
    if ( CORE::TTaskStatus::TASKSTATUS_CANCELLED == task->GetTaskStatus() )
        return AUTHFAIL_CANCELLED;
    CORE::CString extraInfo = task->GetTaskStatusExtraInfo();
    CORE::CString name = extraInfo.SubstrToChar( ':', true );
    for ( Int32 r=(Int32) AUTHFAIL_UNKNOWN; r<=(Int32) AUTHFAIL_INTERNAL; ++r )
        if ( name == AuthFailureReasonToString( (TAuthFailureReason) r ) )
            return (TAuthFailureReason) r;
    return AUTHFAIL_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

/** A future that is over before anybody waits: the request was refused */
static CORE::CFutureResult
MakeRefusedFuture( const char* why )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, why );
    return source.GetFuture();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CAccessTokenKeeperSettings                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CAccessTokenKeeperSettings::DefaultRefreshAheadMs;
const UInt32 CAccessTokenKeeperSettings::DefaultExpirySkewMs;
const UInt32 CAccessTokenKeeperSettings::DefaultRetryDelayMs;
const UInt32 CAccessTokenKeeperSettings::DefaultMaxRetryDelayMs;
const UInt32 CAccessTokenKeeperSettings::DefaultReauthWarnHours;
const UInt32 CAccessTokenKeeperSettings::DefaultCheckIntervalMs;
const UInt32 CAccessTokenKeeperSettings::DefaultPollIntervalMs;
const UInt32 CAccessTokenKeeperSettings::DefaultRefreshTimeoutMs;

CAccessTokenKeeperSettings::CAccessTokenKeeperSettings( void )
    : CORE::CIConfigurable()
    , refreshAheadMs( DefaultRefreshAheadMs )
    , expirySkewMs( DefaultExpirySkewMs )
    , retryDelayMs( DefaultRetryDelayMs )
    , maxRetryDelayMs( DefaultMaxRetryDelayMs )
    , refreshTokenLifetimeHours( 0 )
    , reauthWarnHours( DefaultReauthWarnHours )
    , checkIntervalMs( DefaultCheckIntervalMs )
    , pollIntervalMs( DefaultPollIntervalMs )
    , refreshTimeoutMs( DefaultRefreshTimeoutMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeperSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "refreshAheadMs", refreshAheadMs );
    ok = config.SetAttribute( "expirySkewMs", expirySkewMs ) && ok;
    ok = config.SetAttribute( "retryDelayMs", retryDelayMs ) && ok;
    ok = config.SetAttribute( "maxRetryDelayMs", maxRetryDelayMs ) && ok;
    ok = config.SetAttribute( "refreshTokenLifetimeHours", refreshTokenLifetimeHours ) && ok;
    ok = config.SetAttribute( "reauthWarnHours", reauthWarnHours ) && ok;
    ok = config.SetAttribute( "checkIntervalMs", checkIntervalMs ) && ok;
    ok = config.SetAttribute( "pollIntervalMs", pollIntervalMs ) && ok;
    ok = config.SetAttribute( "refreshTimeoutMs", refreshTimeoutMs ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeperSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    static const UInt32 OneDayMs = 24 * 3600 * 1000;

    refreshAheadMs = config.GetAttributeValueOrChildValueByName( "refreshAheadMs" ).AsUInt32( refreshAheadMs, true );
    expirySkewMs = config.GetAttributeValueOrChildValueByName( "expirySkewMs" ).AsUInt32( expirySkewMs, true );
    retryDelayMs = config.GetAttributeValueOrChildValueByName( "retryDelayMs" ).AsUInt32( retryDelayMs, true );
    maxRetryDelayMs = config.GetAttributeValueOrChildValueByName( "maxRetryDelayMs" ).AsUInt32( maxRetryDelayMs, true );
    refreshTokenLifetimeHours = config.GetAttributeValueOrChildValueByName( "refreshTokenLifetimeHours" ).AsUInt32( refreshTokenLifetimeHours, true );
    reauthWarnHours = config.GetAttributeValueOrChildValueByName( "reauthWarnHours" ).AsUInt32( reauthWarnHours, true );
    checkIntervalMs = config.GetAttributeValueOrChildValueByName( "checkIntervalMs" ).AsUInt32( checkIntervalMs, true );
    pollIntervalMs = config.GetAttributeValueOrChildValueByName( "pollIntervalMs" ).AsUInt32( pollIntervalMs, true );
    refreshTimeoutMs = config.GetAttributeValueOrChildValueByName( "refreshTimeoutMs" ).AsUInt32( refreshTimeoutMs, true );

    if ( refreshAheadMs > OneDayMs )
        refreshAheadMs = OneDayMs;
    if ( expirySkewMs > OneDayMs )
        expirySkewMs = OneDayMs;
    if ( maxRetryDelayMs < retryDelayMs )
        maxRetryDelayMs = retryDelayMs;
    if ( 0 == checkIntervalMs )
        checkIntervalMs = 1;
    if ( 0 == pollIntervalMs )
        pollIntervalMs = 1;
    if ( 0 == refreshTimeoutMs )
        refreshTimeoutMs = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccessTokenKeeperSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAccessTokenKeeperSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CAccessTokenKeeperEventData                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData::CAccessTokenKeeperEventData( void )
    : CORE::CICloneable()
    , providerName()
    , accountRef()
    , accessTokenExpiresAtMs( 0 )
    , failureReason( AUTHFAIL_UNKNOWN )
    , retryAtMs( 0 )
    , reauthReason( REAUTH_NONE )
    , refreshTokenExpiresAtMs( 0 )
    , remainingMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData::CAccessTokenKeeperEventData( const CAccessTokenKeeperEventData& src )
    : CORE::CICloneable( src )
    , providerName( src.providerName )
    , accountRef( src.accountRef )
    , accessTokenExpiresAtMs( src.accessTokenExpiresAtMs )
    , failureReason( src.failureReason )
    , retryAtMs( src.retryAtMs )
    , reauthReason( src.reauthReason )
    , refreshTokenExpiresAtMs( src.refreshTokenExpiresAtMs )
    , remainingMs( src.remainingMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData::~CAccessTokenKeeperEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData&
CAccessTokenKeeperEventData::operator=( const CAccessTokenKeeperEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        accountRef = src.accountRef;
        accessTokenExpiresAtMs = src.accessTokenExpiresAtMs;
        failureReason = src.failureReason;
        retryAtMs = src.retryAtMs;
        reauthReason = src.reauthReason;
        refreshTokenExpiresAtMs = src.refreshTokenExpiresAtMs;
        remainingMs = src.remainingMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAccessTokenKeeperEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAccessTokenKeeperEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CAccessTokenKeeper                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAccessTokenKeeper::TokenRefreshedEvent = "GUCEF::AUTH::CAccessTokenKeeper::TokenRefreshedEvent";
const CORE::CEvent CAccessTokenKeeper::TokenRefreshFailedEvent = "GUCEF::AUTH::CAccessTokenKeeper::TokenRefreshFailedEvent";
const CORE::CEvent CAccessTokenKeeper::ReauthDueSoonEvent = "GUCEF::AUTH::CAccessTokenKeeper::ReauthDueSoonEvent";
const CORE::CEvent CAccessTokenKeeper::ReauthRequiredEvent = "GUCEF::AUTH::CAccessTokenKeeper::ReauthRequiredEvent";

const char* const CAccessTokenKeeper::LoginTimeKeySuffix = "#loginAtMs";

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::RegisterEvents( void )
{GUCEF_TRACE;

    TokenRefreshedEvent.Initialize();
    TokenRefreshFailedEvent.Initialize();
    ReauthDueSoonEvent.Initialize();
    ReauthRequiredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeper::CAccessTokenKeeper( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_lock()
    , m_manager( GUCEF_NULL )
    , m_store( GUCEF_NULL )
    , m_clock( &CSystemEpochClock::Instance() )
    , m_settings()
    , m_providerName()
    , m_account()
    , m_timer( CAccessTokenKeeperSettings::DefaultCheckIntervalMs )
    , m_shutDown( false )
    , m_accessToken()
    , m_expiresAtMs( 0 )
    , m_obtainedAtMs( 0 )
    , m_hasRefreshToken( false )
    , m_refreshRequested( false )
    , m_refreshing( false )
    , m_hasWaitSource( false )
    , m_waitSource()
    , m_providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , m_sentRefreshToken()
    , m_refreshStartedAtMonotonicMs( 0 )
    , m_consecutiveFailures( 0 )
    , m_retryAtMs( 0 )
    , m_loginAtMs( 0 )
    , m_dueSoonRaised( false )
    , m_expiredRaised( false )
    , m_reauthReason( REAUTH_NONE )
    , m_refreshTokenLifetimeMs( 0 )
    , m_refreshCount( 0 )
    , m_failedRefreshCount( 0 )
    , m_pulseThreadId( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_timer.SetEnabled( false );
    TEventCallback timerCallback( this, &CAccessTokenKeeper::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeper::~CAccessTokenKeeper()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::SetManager( CAuthManager* manager )
{GUCEF_TRACE;

    if ( manager == m_manager )
        return;
    if ( GUCEF_NULL != m_manager )
        UnsubscribeFrom( m_manager );
    m_manager = manager;
    if ( GUCEF_NULL != m_manager )
    {
        TEventCallback loginCallback( this, &CAccessTokenKeeper::OnManagerLoginSucceeded );
        SubscribeTo( m_manager, CAuthManager::LoginSucceededEvent, loginCallback );
        TEventCallback logoutCallback( this, &CAccessTokenKeeper::OnManagerAccountGone );
        SubscribeTo( m_manager, CAuthManager::AccountLoggedOutEvent, logoutCallback );
        TEventCallback unlinkCallback( this, &CAccessTokenKeeper::OnManagerAccountGone );
        SubscribeTo( m_manager, CAuthManager::AccountUnlinkedEvent, unlinkCallback );
        TakeSessionToken();
    }
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::SetCredentialStore( ICredentialStore* store )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_store = store;
    }
    LoadLoginTime();
    bool hasRefreshToken = CheckStoreForRefreshToken();
    MT::CScopeMutex lock( m_lock );
    m_hasRefreshToken = hasRefreshToken;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::SetClock( const IEpochClock* clock )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_clock = GUCEF_NULL != clock ? clock : &CSystemEpochClock::Instance();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::SetSettings( const CAccessTokenKeeperSettings& settings )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_settings = settings;
    }
    UpdateLifetime();
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperSettings
CAccessTokenKeeper::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::SetAccount( const CORE::CString& providerName, const CAccountRef& account )
{GUCEF_TRACE;

    CORE::CFutureResultSource released;
    bool releaseWaiters = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_pulseThreadId = MT::GetCurrentTaskID();
        if ( m_refreshing )
            m_providerFuture.RequestCancellation();
        releaseWaiters = m_hasWaitSource;
        released = m_waitSource;
        m_hasWaitSource = false;
        m_refreshing = false;
        m_refreshRequested = false;
        m_providerName = providerName;
        m_account = account;
        m_shutDown = false;
        ClearTokenLocked();
        m_consecutiveFailures = 0;
        m_retryAtMs = 0;
        m_loginAtMs = 0;
        m_dueSoonRaised = false;
        m_expiredRaised = false;
        m_reauthReason = REAUTH_NONE;
        m_hasRefreshToken = false;
    }
    if ( releaseWaiters )
        released.Cancel();

    UpdateLifetime();
    LoadLoginTime();
    bool hasRefreshToken = CheckStoreForRefreshToken();
    {
        MT::CScopeMutex lock( m_lock );
        m_hasRefreshToken = hasRefreshToken;
    }
    TakeSessionToken();
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::Reset( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ClearTokenLocked();
    m_consecutiveFailures = 0;
    m_retryAtMs = 0;
    m_dueSoonRaised = false;
    m_expiredRaised = false;
    m_reauthReason = REAUTH_NONE;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::Shutdown( void )
{GUCEF_TRACE;

    CORE::CFutureResultSource released;
    bool releaseWaiters = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_shutDown = true;
        if ( m_refreshing )
            m_providerFuture.RequestCancellation();
        m_refreshing = false;
        m_refreshRequested = false;
        releaseWaiters = m_hasWaitSource;
        released = m_waitSource;
        m_hasWaitSource = false;
        m_sentRefreshToken.Clear();
        ClearTokenLocked();
    }
    m_timer.SetEnabled( false );
    if ( releaseWaiters )
        released.Cancel();
}

/*-------------------------------------------------------------------------*/

UInt64
CAccessTokenKeeper::Now( void ) const
{GUCEF_TRACE;

    return m_clock->GetEpochTimeInMs();
}

/*-------------------------------------------------------------------------*/

UInt64
CAccessTokenKeeper::EffectiveMarginLocked( UInt32 marginMs ) const
{GUCEF_TRACE;

    // A margin larger than half the life of the token would make every token look expired the moment it arrives, and renew it in a loop
    if ( 0 == m_expiresAtMs )
        return 0;
    UInt64 life = m_expiresAtMs > m_obtainedAtMs ? m_expiresAtMs - m_obtainedAtMs : 0;
    UInt64 half = life / 2;
    return (UInt64) marginMs < half ? (UInt64) marginMs : half;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::IsUsableLocked( UInt64 nowMs ) const
{GUCEF_TRACE;

    if ( m_accessToken.IsEmpty() )
        return false;
    if ( 0 == m_expiresAtMs )
        return true;
    return nowMs + EffectiveMarginLocked( m_settings.expirySkewMs ) < m_expiresAtMs;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::NeedsProactiveRefreshLocked( UInt64 nowMs ) const
{GUCEF_TRACE;

    if ( m_accessToken.IsEmpty() || 0 == m_expiresAtMs )
        return false;
    return nowMs + EffectiveMarginLocked( m_settings.refreshAheadMs ) >= m_expiresAtMs;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::IsInRetryDelayLocked( UInt64 nowMs ) const
{GUCEF_TRACE;

    return 0 != m_retryAtMs && nowMs < m_retryAtMs;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::IsBlockedLocked( void ) const
{GUCEF_TRACE;

    // Renewing cannot work until the person logs in again
    return REAUTH_REFRESH_TOKEN_REJECTED == m_reauthReason || REAUTH_NO_REFRESH_TOKEN == m_reauthReason;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::ClearTokenLocked( void )
{GUCEF_TRACE;

    m_accessToken.Clear();
    m_expiresAtMs = 0;
    m_obtainedAtMs = 0;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::TakeTokenLocked( const CAuthResult& result, UInt64 nowMs )
{GUCEF_TRACE;

    m_accessToken = result.accessToken;
    m_expiresAtMs = result.expiresAtMs;
    m_obtainedAtMs = nowMs;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::CheckStoreForRefreshToken( void ) const
{GUCEF_TRACE;

    ICredentialStore* store = GUCEF_NULL;
    CORE::CString providerName;
    CORE::CString accountKey;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_store;
        providerName = m_providerName;
        accountKey = m_account.IsValid() ? m_account.ToString() : CORE::CString::Empty;
    }
    if ( GUCEF_NULL == store || providerName.IsNULLOrEmpty() || accountKey.IsNULLOrEmpty() )
        return false;
    CSecretString refreshToken;
    return store->Load( providerName, accountKey, refreshToken ) && !refreshToken.IsEmpty();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::UpdateLifetime( void )
{GUCEF_TRACE;

    CORE::CString providerName;
    UInt64 lifetimeMs = 0;
    {
        MT::CScopeMutex lock( m_lock );
        providerName = m_providerName;
        lifetimeMs = (UInt64) m_settings.refreshTokenLifetimeHours * 3600 * 1000;
    }
    if ( 0 == lifetimeMs && !providerName.IsNULLOrEmpty() )
    {
        // What the provider declares, as data of its descriptor
        CAuthGlobal* global = CAuthGlobal::Instance();
        CAuthProviderDescriptor descriptor;
        if ( GUCEF_NULL != global && global->GetDescriptorRegistry().GetDescriptor( providerName, descriptor ) )
            lifetimeMs = (UInt64) descriptor.refreshTokenLifetimeInSeconds * 1000;
    }
    MT::CScopeMutex lock( m_lock );
    m_refreshTokenLifetimeMs = lifetimeMs;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::LoadLoginTime( void )
{GUCEF_TRACE;

    ICredentialStore* store = GUCEF_NULL;
    CORE::CString providerName;
    CORE::CString key;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_store;
        providerName = m_providerName;
        if ( m_account.IsValid() )
            key = m_account.ToString() + LoginTimeKeySuffix;
    }
    UInt64 loginAtMs = 0;
    CSecretString text;
    if ( GUCEF_NULL != store && !providerName.IsNULLOrEmpty() && !key.IsNULLOrEmpty() && store->Load( providerName, key, text ) )
        loginAtMs = CORE::StringToUInt64( text.Reveal(), 0 );
    MT::CScopeMutex lock( m_lock );
    m_loginAtMs = loginAtMs;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::TakeSessionToken( void )
{GUCEF_TRACE;

    // The manager has no locks: it is asked on the pulse thread only, which is where this runs
    CAccountRef account;
    {
        MT::CScopeMutex lock( m_lock );
        account = m_account;
    }
    CAuthResult session;
    if ( GUCEF_NULL == m_manager || !account.IsValid() || !m_manager->GetSession( account, session ) || session.accessToken.IsEmpty() )
        return;

    MT::CScopeMutex lock( m_lock );
    UInt64 nowMs = Now();
    if ( session.providerName != m_providerName || ( 0 != session.expiresAtMs && session.expiresAtMs <= nowMs ) )
        return;
    // A token of a renewal is newer than the one the session started with
    if ( !m_accessToken.IsEmpty() && ( 0 == session.expiresAtMs || session.expiresAtMs <= m_expiresAtMs ) )
        return;
    TakeTokenLocked( session, nowMs );
}

/*-------------------------------------------------------------------------*/

TAccessTokenStatus
CAccessTokenKeeper::GetAccessToken( CSecretString& token, UInt64& expiresAtMs )
{GUCEF_TRACE;

    token.Clear();
    expiresAtMs = 0;

    MT::CScopeMutex lock( m_lock );
    if ( m_shutDown || m_providerName.IsNULLOrEmpty() || !m_account.IsValid() )
        return NO_LOGIN;
    UInt64 nowMs = Now();
    if ( IsUsableLocked( nowMs ) )
    {
        token = m_accessToken;
        expiresAtMs = m_expiresAtMs;
        return TOKEN_READY;
    }
    if ( IsBlockedLocked() )
        return NO_LOGIN;
    if ( m_refreshing || m_refreshRequested )
        return TOKEN_PENDING;
    if ( IsInRetryDelayLocked( nowMs ) )
        return TOKEN_FAILED;
    if ( !m_hasRefreshToken )
    {
        // The store may have been given a login since the keeper looked; it can be used from any thread
        lock.EarlyUnlock();
        bool hasRefreshToken = CheckStoreForRefreshToken();
        MT::CScopeMutex relock( m_lock );
        m_hasRefreshToken = hasRefreshToken;
        if ( !hasRefreshToken || m_shutDown )
            return NO_LOGIN;
        if ( m_refreshing || m_refreshRequested )
            return TOKEN_PENDING;
        m_refreshRequested = true;
        return TOKEN_PENDING;
    }
    // The timer of the keeper starts it on the pulse thread. It is not triggered from here: a pulse generator that is pumped by
    // hand would run that pulse on the thread of the caller, which may be any thread
    m_refreshRequested = true;
    return TOKEN_PENDING;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::RequestRefreshLocked( CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( m_shutDown || m_providerName.IsNULLOrEmpty() || !m_account.IsValid() )
    {
        future = MakeRefusedFuture( "no-login" );
        return false;
    }
    if ( IsBlockedLocked() )
    {
        future = MakeRefusedFuture( ReauthReasonToString( m_reauthReason ) );
        return false;
    }
    if ( !m_refreshing && !m_hasRefreshToken )
    {
        future = MakeRefusedFuture( "no-login" );
        return false;
    }
    if ( !m_refreshing && !m_refreshRequested && IsInRetryDelayLocked( Now() ) )
    {
        future = MakeRefusedFuture( "retry-delay" );
        return false;
    }
    if ( !m_hasWaitSource )
    {
        m_waitSource = CORE::CFutureResultSource();
        m_hasWaitSource = true;
    }
    if ( !m_refreshing )
        m_refreshRequested = true;
    future = m_waitSource.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::RequestRefresh( CORE::CFutureResult& future )
{GUCEF_TRACE;

    RefreshStoreKnowledge();
    MT::CScopeMutex lock( m_lock );
    return RequestRefreshLocked( future );
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::RefreshStoreKnowledge( void )
{GUCEF_TRACE;

    // The store may have been given a login since the keeper looked; it can be used from any thread
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_hasRefreshToken || m_refreshing )
            return;
    }
    bool hasRefreshToken = CheckStoreForRefreshToken();
    MT::CScopeMutex lock( m_lock );
    m_hasRefreshToken = hasRefreshToken;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::ReportTokenRejected( const CSecretString& refusedToken, CORE::CFutureResult& future )
{GUCEF_TRACE;

    RefreshStoreKnowledge();
    MT::CScopeMutex lock( m_lock );
    if ( !m_accessToken.IsEmpty() && !refusedToken.IsEmpty() && m_accessToken != refusedToken && IsUsableLocked( Now() ) )
    {
        // Renewed since the caller took its token: the new one is the answer
        CORE::CFutureResultSource done;
        done.Complete();
        future = done.GetFuture();
        return true;
    }
    if ( !m_accessToken.IsEmpty() && m_accessToken == refusedToken )
        ClearTokenLocked();
    return RequestRefreshLocked( future );
}

/*-------------------------------------------------------------------------*/

TAccessTokenStatus
CAccessTokenKeeper::WaitForAccessToken( CSecretString& token, UInt64& expiresAtMs, UInt32 timeoutInMs )
{GUCEF_TRACE;

    TAccessTokenStatus status = GetAccessToken( token, expiresAtMs );
    if ( TOKEN_PENDING != status )
        return status;

    // The renewal runs on the pulse thread: that thread cannot wait for it
    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 != m_pulseThreadId && MT::GetCurrentTaskID() == m_pulseThreadId )
            return status;
    }

    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !RequestRefresh( future ) )
        return GetAccessToken( token, expiresAtMs );
    try
    {
        future.Await( (Int32) ( timeoutInMs > 0x7FFFFFFF ? 0x7FFFFFFF : timeoutInMs ) );
    }
    catch ( const timeout_exception& )
    {
        return TOKEN_PENDING;
    }
    return GetAccessToken( token, expiresAtMs );
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::HasLogin( void ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shutDown || m_providerName.IsNULLOrEmpty() || !m_account.IsValid() )
            return false;
        if ( !m_accessToken.IsEmpty() )
            return true;
    }
    bool hasRefreshToken = CheckStoreForRefreshToken();
    MT::CScopeMutex lock( m_lock );
    m_hasRefreshToken = hasRefreshToken;
    return hasRefreshToken;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::RecordInteractiveLogin( UInt64 loginAtEpochMs )
{GUCEF_TRACE;

    ICredentialStore* store = GUCEF_NULL;
    CORE::CString providerName;
    CORE::CString key;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_account.IsValid() || m_providerName.IsNULLOrEmpty() )
            return false;
        if ( 0 == loginAtEpochMs )
            loginAtEpochMs = Now();
        m_loginAtMs = loginAtEpochMs;
        m_dueSoonRaised = false;
        m_expiredRaised = false;
        m_reauthReason = REAUTH_NONE;
        m_consecutiveFailures = 0;
        m_retryAtMs = 0;
        store = m_store;
        providerName = m_providerName;
        key = m_account.ToString() + LoginTimeKeySuffix;
    }
    bool kept = GUCEF_NULL != store && store->Store( providerName, key, CSecretString( CORE::UInt64ToString( loginAtEpochMs ) ) );
    if ( !kept )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the time of the login cannot be kept for the next start" );
    bool hasRefreshToken = CheckStoreForRefreshToken();
    {
        MT::CScopeMutex lock( m_lock );
        m_hasRefreshToken = hasRefreshToken;
    }
    TakeSessionToken();
    CheckReauth();
    UpdateTimer();
    return kept;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::ForgetLogin( void )
{GUCEF_TRACE;

    ICredentialStore* store = GUCEF_NULL;
    CORE::CString providerName;
    CORE::CString key;
    {
        MT::CScopeMutex lock( m_lock );
        ClearTokenLocked();
        m_loginAtMs = 0;
        m_dueSoonRaised = false;
        m_expiredRaised = false;
        m_reauthReason = REAUTH_NONE;
        m_consecutiveFailures = 0;
        m_retryAtMs = 0;
        m_hasRefreshToken = false;
        store = m_store;
        providerName = m_providerName;
        if ( m_account.IsValid() )
            key = m_account.ToString() + LoginTimeKeySuffix;
    }
    if ( GUCEF_NULL == store || providerName.IsNULLOrEmpty() || key.IsNULLOrEmpty() )
        return false;
    return store->Remove( providerName, key );
}

/*-------------------------------------------------------------------------*/

UInt64
CAccessTokenKeeper::GetRefreshTokenExpiresAtMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( 0 == m_loginAtMs || 0 == m_refreshTokenLifetimeMs )
        return 0;
    return m_loginAtMs + m_refreshTokenLifetimeMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CAccessTokenKeeper::GetRefreshTokenLifetimeMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refreshTokenLifetimeMs;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::IsRefreshing( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refreshing || m_refreshRequested;
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::IsReauthRequired( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return REAUTH_NONE != m_reauthReason;
}

/*-------------------------------------------------------------------------*/

TReauthReason
CAccessTokenKeeper::GetReauthReason( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_reauthReason;
}

/*-------------------------------------------------------------------------*/

UInt32
CAccessTokenKeeper::GetRefreshCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refreshCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAccessTokenKeeper::GetFailedRefreshCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_failedRefreshCount;
}

/*-------------------------------------------------------------------------*/

CAccessTokenKeeperEventData
CAccessTokenKeeper::MakeEventDataLocked( void ) const
{GUCEF_TRACE;

    CAccessTokenKeeperEventData data;
    data.providerName = m_providerName;
    data.accountRef = m_account;
    data.accessTokenExpiresAtMs = m_expiresAtMs;
    data.reauthReason = m_reauthReason;
    if ( 0 != m_loginAtMs && 0 != m_refreshTokenLifetimeMs )
        data.refreshTokenExpiresAtMs = m_loginAtMs + m_refreshTokenLifetimeMs;
    return data;
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::RaiseEvent( const CORE::CEvent& eventId, const CAccessTokenKeeperEventData& data )
{GUCEF_TRACE;

    CAccessTokenKeeperEventData copy( data );
    NotifyObservers( eventId, &copy );
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::UpdateTimer( void )
{GUCEF_TRACE;

    bool enabled = false;
    UInt32 interval = 0;
    {
        MT::CScopeMutex lock( m_lock );
        enabled = !m_shutDown && !m_providerName.IsNULLOrEmpty() && m_account.IsValid();
        interval = m_refreshing ? m_settings.pollIntervalMs : m_settings.checkIntervalMs;
    }
    if ( !enabled )
    {
        m_timer.SetEnabled( false );
        return;
    }
    if ( m_timer.GetInterval() != interval )
        m_timer.SetInterval( interval );
    if ( !m_timer.GetEnabled() )
        m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    bool refreshing = false;
    bool finish = false;
    bool timedOut = false;
    bool start = false;
    bool refuseWaiters = false;
    CORE::CFutureResultSource refused;
    {
        MT::CScopeMutex lock( m_lock );
        m_pulseThreadId = MT::GetCurrentTaskID();
        if ( m_shutDown )
            return;
        refreshing = m_refreshing;
        if ( refreshing )
        {
            if ( m_providerFuture.IsReady() )
                finish = true;
            else
            if ( GetMonotonicTimeInMs() - m_refreshStartedAtMonotonicMs >= m_settings.refreshTimeoutMs )
            {
                m_providerFuture.RequestCancellation();
                finish = true;
                timedOut = true;
            }
        }
        else
        {
            UInt64 nowMs = Now();
            bool wanted = m_refreshRequested || NeedsProactiveRefreshLocked( nowMs );
            if ( wanted && !IsBlockedLocked() && !IsInRetryDelayLocked( nowMs ) )
            {
                start = true;
            }
            else
            if ( m_refreshRequested )
            {
                // asked for while nothing can be done: whoever waits is told now
                m_refreshRequested = false;
                refuseWaiters = m_hasWaitSource;
                refused = m_waitSource;
                m_hasWaitSource = false;
            }
        }
    }

    if ( refuseWaiters )
        refused.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "retry-delay" );
    if ( finish )
        FinishRefresh( timedOut );
    else
    if ( start )
        StartRefresh();
    CheckReauth();
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::StartRefresh( void )
{GUCEF_TRACE;

    // A login of the provider that the manager waits for would take the answer of a renewal for its own: the renewal waits for it
    {
        CORE::CString name;
        {
            MT::CScopeMutex lock( m_lock );
            name = m_providerName;
        }
        if ( GUCEF_NULL != m_manager && m_manager->IsLoginPending( name ) )
        {
            MT::CScopeMutex lock( m_lock );
            m_refreshRequested = true;
            return;
        }
    }

    CORE::CString providerName;
    CAccountRef account;
    ICredentialStore* store = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        m_refreshRequested = false;
        if ( !m_hasWaitSource )
        {
            m_waitSource = CORE::CFutureResultSource();
            m_hasWaitSource = true;
        }
        providerName = m_providerName;
        account = m_account;
        store = m_store;
    }

    CIAuthProvider* provider = GUCEF_NULL != m_manager ? m_manager->GetProvider( providerName ) : GUCEF_NULL;
    CSecretString refreshToken;
    bool haveToken = GUCEF_NULL != store && store->Load( providerName, account.ToString(), refreshToken ) && !refreshToken.IsEmpty();

    CORE::CFutureResult providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    bool started = false;
    if ( haveToken && GUCEF_NULL != provider )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_refreshCount;
    }
    if ( haveToken && GUCEF_NULL != provider )
        started = provider->Refresh( account, refreshToken, providerFuture );

    if ( started )
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shutDown )
        {
            providerFuture.RequestCancellation();
            return;
        }
        m_providerFuture = providerFuture;
        m_sentRefreshToken = refreshToken;
        m_refreshing = true;
        m_refreshStartedAtMonotonicMs = GetMonotonicTimeInMs();
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AccessTokenKeeper: " << providerName << ": the access token is being renewed" );
        return;
    }

    // Nothing could be started: there is nothing to renew with, or the provider cannot do it now
    CAccessTokenKeeperEventData failed;
    CAccessTokenKeeperEventData reauth;
    bool raiseReauth = false;
    CORE::CFutureResultSource waiters;
    bool releaseWaiters = false;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_failedRefreshCount;
        releaseWaiters = m_hasWaitSource;
        waiters = m_waitSource;
        m_hasWaitSource = false;
        if ( !haveToken )
        {
            m_hasRefreshToken = false;
            raiseReauth = REAUTH_NO_REFRESH_TOKEN != m_reauthReason;
            m_reauthReason = REAUTH_NO_REFRESH_TOKEN;
            m_retryAtMs = 0;
            failed = MakeEventDataLocked();
            failed.failureReason = AUTHFAIL_INVALID_GRANT;
            reauth = failed;
        }
        else
        {
            ++m_consecutiveFailures;
            UInt64 delay = m_settings.retryDelayMs;
            for ( UInt32 i=1; i<m_consecutiveFailures && delay < m_settings.maxRetryDelayMs; ++i )
                delay *= 2;
            if ( delay > m_settings.maxRetryDelayMs )
                delay = m_settings.maxRetryDelayMs;
            m_retryAtMs = Now() + delay;
            failed = MakeEventDataLocked();
            failed.failureReason = AUTHFAIL_NOT_AVAILABLE;
            failed.retryAtMs = m_retryAtMs;
        }
    }
    if ( !haveToken )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": there is no stored login to renew the access token with" );
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the provider cannot renew the access token now" );
    }
    if ( releaseWaiters )
        waiters.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, haveToken ? "not-available" : ReauthReasonToString( REAUTH_NO_REFRESH_TOKEN ) );
    RaiseEvent( TokenRefreshFailedEvent, failed );
    if ( raiseReauth )
        RaiseEvent( ReauthRequiredEvent, reauth );
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::FinishRefresh( bool timedOut )
{GUCEF_TRACE;

    CORE::CFutureResult providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    CSecretString sentRefreshToken;
    CORE::CString providerName;
    CAccountRef account;
    ICredentialStore* store = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        providerFuture = m_providerFuture;
        sentRefreshToken = m_sentRefreshToken;
        m_sentRefreshToken.Clear();
        m_providerFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        providerName = m_providerName;
        account = m_account;
        store = m_store;
    }

    CAuthResult result;
    bool ok = !timedOut && CAuthResult::TryGetFromFuture( providerFuture, result ) && !result.accessToken.IsEmpty() && result.accountRef == account;
    TAuthFailureReason reason = AUTHFAIL_NETWORK;
    if ( !ok && !timedOut )
        reason = FailureReasonOfFuture( providerFuture );

    // A refresh token that rotated replaces the one in the store
    if ( ok && !result.refreshToken.IsEmpty() && result.refreshToken != sentRefreshToken && GUCEF_NULL != store )
    {
        if ( !store->Store( providerName, account.ToString(), result.refreshToken ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the renewed refresh token cannot be kept" );
    }
    // A refresh token the provider refuses is of no use; it is removed when it is still the one that was sent, not one of a login that happened meanwhile
    bool rejected = !ok && AUTHFAIL_INVALID_GRANT == reason;
    if ( rejected && GUCEF_NULL != store )
    {
        CSecretString stored;
        if ( store->Load( providerName, account.ToString(), stored ) && stored == sentRefreshToken )
            store->Remove( providerName, account.ToString() );
    }

    CAccessTokenKeeperEventData data;
    bool raiseReauth = false;
    CORE::CFutureResultSource waiters;
    bool releaseWaiters = false;
    bool cancelled = false;
    {
        MT::CScopeMutex lock( m_lock );
        m_refreshing = false;
        releaseWaiters = m_hasWaitSource;
        waiters = m_waitSource;
        m_hasWaitSource = false;
        cancelled = m_shutDown;
        if ( ok )
        {
            TakeTokenLocked( result, Now() );
            m_consecutiveFailures = 0;
            m_retryAtMs = 0;
            m_hasRefreshToken = true;
            data = MakeEventDataLocked();
        }
        else
        {
            ++m_failedRefreshCount;
            if ( rejected )
            {
                m_hasRefreshToken = false;
                m_retryAtMs = 0;
                raiseReauth = REAUTH_REFRESH_TOKEN_REJECTED != m_reauthReason;
                m_reauthReason = REAUTH_REFRESH_TOKEN_REJECTED;
            }
            else
            {
                ++m_consecutiveFailures;
                UInt64 delay = m_settings.retryDelayMs;
                for ( UInt32 i=1; i<m_consecutiveFailures && delay < m_settings.maxRetryDelayMs; ++i )
                    delay *= 2;
                if ( delay > m_settings.maxRetryDelayMs )
                    delay = m_settings.maxRetryDelayMs;
                m_retryAtMs = Now() + delay;
            }
            data = MakeEventDataLocked();
            data.failureReason = reason;
            data.retryAtMs = m_retryAtMs;
        }
    }
    sentRefreshToken.Clear();
    result.WipeSecrets();

    if ( cancelled )
    {
        if ( releaseWaiters )
            waiters.Cancel();
        return;
    }
    if ( ok )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the access token was renewed" );
        if ( releaseWaiters )
            waiters.Complete();
        RaiseEvent( TokenRefreshedEvent, data );
        return;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the access token could not be renewed (" << AuthFailureReasonToString( reason ) << ")" );
    if ( releaseWaiters )
        waiters.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, AuthFailureReasonToString( reason ) );
    RaiseEvent( TokenRefreshFailedEvent, data );
    if ( raiseReauth )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << providerName << ": the refresh token was refused, a new login is needed" );
        RaiseEvent( ReauthRequiredEvent, data );
    }
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::CheckReauth( void )
{GUCEF_TRACE;

    CAccessTokenKeeperEventData data;
    bool dueSoon = false;
    bool expired = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shutDown || 0 == m_loginAtMs || 0 == m_refreshTokenLifetimeMs )
            return;
        UInt64 nowMs = Now();
        UInt64 expiresAtMs = m_loginAtMs + m_refreshTokenLifetimeMs;
        if ( nowMs >= expiresAtMs )
        {
            if ( m_expiredRaised )
                return;
            m_expiredRaised = true;
            if ( REAUTH_NONE == m_reauthReason )
                m_reauthReason = REAUTH_REFRESH_TOKEN_EXPIRED;
            expired = true;
            data = MakeEventDataLocked();
            data.reauthReason = REAUTH_REFRESH_TOKEN_EXPIRED;
        }
        else
        {
            UInt64 remainingMs = expiresAtMs - nowMs;
            if ( m_dueSoonRaised || remainingMs > (UInt64) m_settings.reauthWarnHours * 3600 * 1000 )
                return;
            m_dueSoonRaised = true;
            dueSoon = true;
            data = MakeEventDataLocked();
            data.remainingMs = remainingMs;
        }
    }
    if ( expired )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << data.providerName << ": the refresh token is past its declared lifetime, a new login is needed" );
        RaiseEvent( ReauthRequiredEvent, data );
    }
    else
    if ( dueSoon )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AccessTokenKeeper: " << data.providerName << ": the refresh token expires in " << ( data.remainingMs / 60000 ) << " minutes" );
        RaiseEvent( ReauthDueSoonEvent, data );
    }
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::OnManagerLoginSucceeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAuthResult* result = static_cast< const CAuthResult* >( eventData );
    if ( GUCEF_NULL == result )
        return;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shutDown || result->accountRef != m_account || result->providerName != m_providerName || result->accessToken.IsEmpty() )
            return;
        TakeTokenLocked( *result, Now() );
        m_consecutiveFailures = 0;
        m_retryAtMs = 0;
        if ( !result->refreshToken.IsEmpty() )
            m_hasRefreshToken = true;
        // A login that worked: whatever made renewing impossible is over. The count of the life of the refresh token is RecordInteractiveLogin()'s
        if ( IsBlockedLocked() )
            m_reauthReason = REAUTH_NONE;
    }
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

void
CAccessTokenKeeper::OnManagerAccountGone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAuthAccountEventData* data = static_cast< const CAuthAccountEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    {
        MT::CScopeMutex lock( m_lock );
        if ( data->accountRef != m_account )
            return;
    }
    ForgetLogin();
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return GetSettings().SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CAccessTokenKeeper::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CAccessTokenKeeperSettings settings = GetSettings();
    if ( !settings.LoadConfig( config ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAccessTokenKeeper::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAccessTokenKeeper";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
