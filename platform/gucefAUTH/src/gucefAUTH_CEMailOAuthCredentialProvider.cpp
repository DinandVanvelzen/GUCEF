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

#include "gucefAUTH_CEMailOAuthCredentialProvider.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Looks at the timer that follows a renewal in flight: the provider is a notifier and cannot subscribe to the timer itself */
class CEMailOAuthCredentialProvider::CPollObserver : public CORE::CObserver
{
    public:

    explicit CPollObserver( CEMailOAuthCredentialProvider& owner, CORE::CTimer& timer )
        : CORE::CObserver()
        , m_owner( owner )
    {
        typedef CORE::CTEventHandlerFunctor< CPollObserver > TEventCallback;
        TEventCallback callback( this, &CPollObserver::OnTimer );
        SubscribeTo( &timer, CORE::CTimer::TimerUpdateEvent, callback );
    }

    virtual ~CPollObserver()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::AUTH::CEMailOAuthCredentialProvider::CPollObserver";
        return classTypeName;
    }

    private:

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( m_owner.m_refreshing && m_owner.m_refreshFuture.IsReady() )
            m_owner.FinishRefresh();
    }

    CEMailOAuthCredentialProvider& m_owner;
};

const UInt32 CEMailOAuthCredentialProvider::DefaultSkewMs;
const UInt32 CEMailOAuthCredentialProvider::DefaultRetryDelayMs;
const UInt32 CEMailOAuthCredentialProvider::PollIntervalMs;

CEMailOAuthCredentialProvider::CEMailOAuthCredentialProvider( void )
    : COM::CIEMailCredentialProvider()
    , m_manager( GUCEF_NULL )
    , m_store( GUCEF_NULL )
    , m_providerName()
    , m_account()
    , m_mailUserName()
    , m_accessToken()
    , m_expiresAtMs( 0 )
    , m_refreshFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , m_refreshing( false )
    , m_refreshFailed( false )
    , m_failedAtMs( 0 )
    , m_justRenewed( false )
    , m_sessionSuperseded( false )
    , m_waitingRequests()
    , m_pollTimer( PollIntervalMs )
    , m_poller( GUCEF_NULL )
    , m_skewMs( DefaultSkewMs )
    , m_retryDelayMs( DefaultRetryDelayMs )
    , m_refreshCount( 0 )
    , m_rejectionCount( 0 )
{GUCEF_TRACE;

    m_pollTimer.SetEnabled( false );
    m_poller = GUCEF_NEW CPollObserver( *this, m_pollTimer );
}

CEMailOAuthCredentialProvider::~CEMailOAuthCredentialProvider()
{GUCEF_TRACE;

    m_pollTimer.SetEnabled( false );
    GUCEF_DELETE m_poller;
    m_poller = GUCEF_NULL;
    if ( m_refreshing )
        m_refreshFuture.RequestCancellation();
}

/*-------------------------------------------------------------------------*/

void
CEMailOAuthCredentialProvider::SetManager( CAuthManager* manager )
{GUCEF_TRACE;

    m_manager = manager;
}

void
CEMailOAuthCredentialProvider::SetCredentialStore( ICredentialStore* store )
{GUCEF_TRACE;

    m_store = store;
}

void
CEMailOAuthCredentialProvider::SetSkewMs( UInt32 skewMs )
{GUCEF_TRACE;

    m_skewMs = skewMs;
}

void
CEMailOAuthCredentialProvider::SetRetryDelayMs( UInt32 delayMs )
{GUCEF_TRACE;

    m_retryDelayMs = delayMs;
}

void
CEMailOAuthCredentialProvider::Reset( void )
{GUCEF_TRACE;

    m_accessToken.Clear();
    m_expiresAtMs = 0;
    m_refreshFailed = false;
    m_failedAtMs = 0;
    m_justRenewed = false;
    m_sessionSuperseded = false;
}

void
CEMailOAuthCredentialProvider::SetAccount( const CORE::CString& providerName, const CAccountRef& account, const CORE::CString& mailUserName )
{GUCEF_TRACE;

    if ( providerName == m_providerName && account == m_account && mailUserName == m_mailUserName )
        return;
    m_providerName = providerName;
    m_account = account;
    m_mailUserName = mailUserName;
    Reset();
}

bool
CEMailOAuthCredentialProvider::HasLogin( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager || m_providerName.IsNULLOrEmpty() || !m_account.IsValid() )
        return false;
    if ( !m_accessToken.IsEmpty() )
        return true;
    CAuthResult session;
    if ( m_manager->GetSession( m_account, session ) && !session.accessToken.IsEmpty() )
        return true;
    CSecretString refreshToken;
    return GUCEF_NULL != m_store && m_store->Load( m_providerName, m_account.ToString(), refreshToken ) && !refreshToken.IsEmpty();
}

UInt32 CEMailOAuthCredentialProvider::GetRefreshCount( void ) const { return m_refreshCount; }
UInt32 CEMailOAuthCredentialProvider::GetRejectionCount( void ) const { return m_rejectionCount; }
bool CEMailOAuthCredentialProvider::HasRefreshFailed( void ) const { return m_refreshFailed; }

/*-------------------------------------------------------------------------*/

bool
CEMailOAuthCredentialProvider::HasUsableCachedToken( UInt64 nowMs ) const
{GUCEF_TRACE;

    return !m_accessToken.IsEmpty() && ( 0 == m_expiresAtMs || m_expiresAtMs > nowMs + m_skewMs );
}

bool
CEMailOAuthCredentialProvider::TakeSessionToken( UInt64 nowMs )
{GUCEF_TRACE;

    CAuthResult session;
    if ( m_sessionSuperseded || GUCEF_NULL == m_manager || !m_manager->GetSession( m_account, session ) || session.accessToken.IsEmpty() )
        return false;
    if ( session.IsExpired( nowMs, m_skewMs ) )
        return false;
    m_accessToken = session.accessToken;
    m_expiresAtMs = session.expiresAtMs;
    return true;
}

void
CEMailOAuthCredentialProvider::FillCredential( COM::CEMailCredential& credential ) const
{GUCEF_TRACE;

    credential.type = COM::MAILCRED_BEARER_TOKEN;
    credential.userName = m_mailUserName;
    credential.secret = m_accessToken.Reveal();
}

/*-------------------------------------------------------------------------*/

COM::CIEMailCredentialProvider::EResult
CEMailOAuthCredentialProvider::GetCredential( const COM::CEMailCredentialRequest& request, COM::CEMailCredential& credential )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager || m_providerName.IsNULLOrEmpty() || !m_account.IsValid() )
        return CRED_FAILED;

    const UInt64 nowMs = GetEpochTimeInMs();
    if ( request.previousAttemptFailed )
    {
        // the server refused the token it was given: a token that was renewed since is the answer, else it is renewed now, whatever the old one says about its own life
        if ( m_justRenewed && !m_accessToken.IsEmpty() )
        {
            FillCredential( credential );
            return CRED_READY;
        }
        m_accessToken.Clear();
        m_expiresAtMs = 0;
    }
    else
    if ( ( m_justRenewed && !m_accessToken.IsEmpty() ) || HasUsableCachedToken( nowMs ) || TakeSessionToken( nowMs ) )
    {
        m_justRenewed = false;
        FillCredential( credential );
        return CRED_READY;
    }

    if ( m_refreshing )
    {
        if ( request.requestId != 0 )
            m_waitingRequests.push_back( request.requestId );
        return CRED_PENDING;
    }
    if ( m_refreshFailed && nowMs < m_failedAtMs + m_retryDelayMs )
        return CRED_FAILED;
    if ( !StartRefresh() )
        return CRED_FAILED;
    if ( request.requestId != 0 )
        m_waitingRequests.push_back( request.requestId );
    return CRED_PENDING;
}

void
CEMailOAuthCredentialProvider::ReportRejected( const COM::CEMailCredentialRequest& request )
{GUCEF_TRACE;

    ++m_rejectionCount;
    m_accessToken.Clear();
    m_expiresAtMs = 0;
    m_justRenewed = false;
}

/*-------------------------------------------------------------------------*/

bool
CEMailOAuthCredentialProvider::StartRefresh( void )
{GUCEF_TRACE;

    CIAuthProvider* provider = GUCEF_NULL != m_manager ? m_manager->GetProvider( m_providerName ) : GUCEF_NULL;
    CSecretString refreshToken;
    if ( GUCEF_NULL == provider || GUCEF_NULL == m_store || !m_store->Load( m_providerName, m_account.ToString(), refreshToken ) || refreshToken.IsEmpty() )
    {
        m_refreshFailed = true;
        m_failedAtMs = GetEpochTimeInMs();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOAuthCredentialProvider: There is no login to renew the access token of the mailbox with" );
        return false;
    }
    ++m_refreshCount;
    m_refreshFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !provider->Refresh( m_account, refreshToken, m_refreshFuture ) )
    {
        m_refreshFailed = true;
        m_failedAtMs = GetEpochTimeInMs();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOAuthCredentialProvider: The provider cannot renew the access token of the mailbox" );
        return false;
    }
    m_refreshing = true;
    m_pollTimer.SetInterval( PollIntervalMs );
    m_pollTimer.SetEnabled( true );
    return true;
}

void
CEMailOAuthCredentialProvider::FinishRefresh( void )
{GUCEF_TRACE;

    m_refreshing = false;
    m_pollTimer.SetEnabled( false );
    CAuthResult result;
    if ( CAuthResult::TryGetFromFuture( m_refreshFuture, result ) && !result.accessToken.IsEmpty() )
    {
        m_accessToken = result.accessToken;
        m_expiresAtMs = result.expiresAtMs;
        m_refreshFailed = false;
        m_justRenewed = true;
        m_sessionSuperseded = true;         // the session of the manager has the token before this one
        // a refresh token that rotated replaces the one in the store
        if ( !result.refreshToken.IsEmpty() && GUCEF_NULL != m_store && !m_store->Store( m_providerName, m_account.ToString(), result.refreshToken ) )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOAuthCredentialProvider: The renewed refresh token of the mailbox could not be kept" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOAuthCredentialProvider: The access token of the mailbox was renewed" );
    }
    else
    {
        m_refreshFailed = true;
        m_failedAtMs = GetEpochTimeInMs();
        m_accessToken.Clear();
        m_expiresAtMs = 0;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOAuthCredentialProvider: The access token of the mailbox could not be renewed" );
    }
    m_refreshFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

    // the clients that wait ask again: they get the new token, or no credential
    GUCEF::vector< UInt32 > waiting;
    waiting.swap( m_waitingRequests );
    for ( size_t i=0; i<waiting.size(); ++i )
    {
        TCredentialReadyEventData data( waiting[ i ] );
        NotifyObservers( CredentialReadyEvent, &data );
    }
}

const CORE::CString&
CEMailOAuthCredentialProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CEMailOAuthCredentialProvider";
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
