/*
 *  authpluginEOS: gucefAUTH plugin for logging in with Epic Online Services
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "authpluginEOS_CEosAuthProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CEosAuthProvider::ProviderName = "epic";

// The ticks the SDK gets after a logout so that its callbacks run
static const UInt32 LogoutTickCount = 40;

// The largest id token the SDK is asked for: a JWT of an Epic account is a few kilobytes at most
static const UInt32 MaxIdTokenSize = 16384;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosAuthProvider::CPump::CPump( CEosAuthProvider& owner )
    : CORE::CObserver()
    , m_owner( owner )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosAuthProvider::CPump::~CPump()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosAuthProvider::CPump::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::EOS::CEosAuthProvider::CPump";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::CPump::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( PLUGINGLUE::EOS::CEosPlatform::TickEvent == eventId )
        m_owner.OnTimer();
}

/*-------------------------------------------------------------------------*/

CEosAuthProvider::CEosAuthProvider( void )
    : AUTH::CIAuthProvider()
    , m_lock()
    , m_settings()
    , m_sdk()
    , m_shim()
    , m_platform( GUCEF_NULL )
    , m_tickDemand( 0 )
    , m_lastRunInMs( 0 )
    , m_loginStateAccount()
    , m_logins()
    , m_productUserIds()
    , m_nextRequestId( 0 )
    , m_idleTicksLeft( 0 )
    , m_linkTarget()
    , m_pump( *this )
{GUCEF_TRACE;

    // Without the SDK in the build the table stays empty and the provider is not available
    m_sdk.FillShim( m_shim );
}

/*-------------------------------------------------------------------------*/

CEosAuthProvider::CEosAuthProvider( const TEosShim& shim )
    : AUTH::CIAuthProvider()
    , m_lock()
    , m_settings()
    , m_sdk()
    , m_shim( shim )
    , m_platform( GUCEF_NULL )
    , m_tickDemand( 0 )
    , m_lastRunInMs( 0 )
    , m_loginStateAccount()
    , m_logins()
    , m_productUserIds()
    , m_nextRequestId( 0 )
    , m_idleTicksLeft( 0 )
    , m_linkTarget()
    , m_pump( *this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosAuthProvider::~CEosAuthProvider()
{GUCEF_TRACE;

    m_pump.UnsubscribeAllFromObserver();

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_logins.size(); ++i )
        m_logins[ i ].source.Cancel();
    m_logins.clear();

    // The platform goes with the last one who holds it; whoever is logged in through this provider is not any more
    ReleasePlatform();
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::SetShim( const TEosShim& shim )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_shim = shim;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_DEVICE_CODE | AUTH::AUTHCAP_SERVER_VERIFIABLE | AUTH::AUTHCAP_REQUIRES_CLIENT_ID;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::IsSdkAvailable( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_shim.isSdkAvailable )
        return m_shim.isSdkAvailable( m_shim.context );

    // A table without the platform functions: the SDK that is compiled in, if any
    PLUGINGLUE::EOS::SEosPlatformShim platformShim;
    if ( !PLUGINGLUE::EOS::CEosGlue::Instance()->GetDefaultPlatformShim( platformShim ) )
        return false;
    return GUCEF_NULL != platformShim.isSdkAvailable && platformShim.isSdkAvailable( platformShim.context );
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return IsSdkAvailable() && m_settings.IsConfigured();
}

/*-------------------------------------------------------------------------*/

PLUGINGLUE::EOS::CEosPlatform*
CEosAuthProvider::GetPlatform( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_platform;
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::ReleasePlatform( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_platform )
        return;

    m_pump.UnsubscribeAllFromObserver();
    if ( 0 != m_tickDemand )
        m_platform->RemoveTickDemand( m_tickDemand );
    m_tickDemand = 0;
    if ( !m_loginStateAccount.IsNULLOrEmpty() && m_platform->GetLoginState().epicAccountId == m_loginStateAccount )
        m_platform->ClearLoginState();
    m_loginStateAccount.Clear();
    PLUGINGLUE::EOS::CEosGlue::Instance()->ReleasePlatform( m_platform );
    m_platform = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::EnsurePlatform( CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsSdkAvailable() )
    {
        reason = "The Epic Online Services SDK is not part of this build";
        return false;
    }
    if ( !m_settings.IsConfigured() )
    {
        reason = "The settings clientId, clientSecret, productId, sandboxId and deploymentId are needed";
        return false;
    }
    if ( GUCEF_NULL != m_platform )
        return true;

    // The platform is the one of the glue, shared with the other EOS plugins: it is made by the first holder
    PLUGINGLUE::EOS::CEosPlatformSettings platformSettings = m_settings.GetPlatformSettings();
    const PLUGINGLUE::EOS::SEosPlatformShim* platformShim = &m_shim;
    PLUGINGLUE::EOS::CEosPlatform* platform = PLUGINGLUE::EOS::CEosGlue::Instance()->AcquirePlatform( platformShim, &platformSettings, reason );
    if ( GUCEF_NULL == platform )
        return false;

    m_platform = platform;
    m_pump.SubscribeTo( m_platform, PLUGINGLUE::EOS::CEosPlatform::TickEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    UInt32 timeout = 0;
    {
        MT::CScopeMutex lock( m_lock );
        timeout = m_settings.silentTimeoutInMs;
    }
    return Start( EOSLOGIN_PERSISTENT, false, timeout, future );
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::BeginLogin( const AUTH::CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    EEosLoginType type = options.preferDeviceCode ? EOSLOGIN_DEVICE_CODE : EOSLOGIN_ACCOUNT_PORTAL;
    return Start( type, true, options.timeoutInMs > 0 ? options.timeoutInMs : AUTH::CAuthOptions::DefaultTimeoutInMs, future );
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::Start( EEosLoginType type, bool interactive, UInt32 timeoutInMs, CORE::CFutureResult& future )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    CORE::CString reason;
    bool ready = EnsurePlatform( reason );
    if ( ready && GUCEF_NULL == m_shim.authLogin )
    {
        reason = "The Epic Online Services SDK cannot log in";
        ready = false;
    }

    // The SDK takes one login at a time
    bool busy = ready && !m_logins.empty();

    if ( !ready || busy )
    {
        if ( !interactive )
        {
            const CORE::CString whyNot = busy ? CORE::CString( "a login is under way" ) : reason;
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosAuthProvider: No silent login: " << whyNot );
            return false;
        }
        CORE::CFutureResultSource failed;
        future = failed.GetFuture();
        if ( busy )
            FailLogin( failed, AUTH::AUTHFAIL_IN_PROGRESS, "A login with Epic Online Services is under way" );
        else
            FailLogin( failed, AUTH::AUTHFAIL_NOT_AVAILABLE, reason );
        return true;
    }

    CORE::CFutureResultSource source;
    future = source.GetFuture();

    SLogin login;
    login.requestId = ++m_nextRequestId;
    login.source = source;
    login.stage = STAGE_AUTH;
    login.deadlineInMs = AUTH::GetMonotonicTimeInMs() + timeoutInMs;
    login.linkTarget = m_linkTarget;

    if ( !m_shim.authLogin( m_shim.context, login.requestId, type ) )
    {
        FailLogin( source, AUTH::AUTHFAIL_INTERNAL, "The Epic Online Services SDK did not start the login" );
        return true;
    }

    m_logins.push_back( login );
    UpdateTickDemand();
    return true;
}

/*-------------------------------------------------------------------------*/

AUTH::TAuthFailureReason
CEosAuthProvider::ToFailureReason( EEosOutcome outcome )
{GUCEF_TRACE;

    switch ( outcome )
    {
        case EOSOUTCOME_CANCELLED :             return AUTH::AUTHFAIL_CANCELLED;
        case EOSOUTCOME_DENIED :                return AUTH::AUTHFAIL_DENIED;
        case EOSOUTCOME_INVALID_CREDENTIALS :   return AUTH::AUTHFAIL_INVALID_GRANT;
        case EOSOUTCOME_NO_CONNECTION :         return AUTH::AUTHFAIL_NETWORK;
        case EOSOUTCOME_TIMEOUT :               return AUTH::AUTHFAIL_TIMEOUT;
        default :                               return AUTH::AUTHFAIL_INTERNAL;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::UpdateTickDemand( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_platform )
        return;

    bool needed = !m_logins.empty() || m_idleTicksLeft > 0;
    if ( needed )
    {
        UInt32 interval = m_settings.tickIntervalInMs > 0 ? m_settings.tickIntervalInMs : CEosAuthSettings::DefaultTickIntervalInMs;
        if ( 0 == m_tickDemand )
            m_tickDemand = m_platform->AddTickDemand( interval );
        else
            m_platform->SetTickDemandInterval( m_tickDemand, interval );
    }
    else
    if ( 0 != m_tickDemand )
    {
        m_platform->RemoveTickDemand( m_tickDemand );
        m_tickDemand = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::EndLogin( SLogin& login, const CORE::CString& productUserId )
{GUCEF_TRACE;

    AUTH::CAccountRef account( AUTH::CAuthProviderIds::Epic, login.epicAccountId );

    AUTH::CAuthResult result;
    result.accountRef = account;
    result.idToken = login.idToken;
    result.proof = AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, login.idToken.Reveal() );

    if ( !productUserId.IsNULLOrEmpty() && account.IsValid() )
        m_productUserIds[ account ] = productUserId;

    // The other EOS plugins find the user through the platform
    if ( GUCEF_NULL != m_platform && account.IsValid() )
    {
        m_platform->SetLoginState( login.epicAccountId, productUserId );
        m_loginStateAccount = login.epicAccountId;
    }
    CompleteLogin( login.source, result );
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::HandleEvent( const SEosEvent& event )
{GUCEF_TRACE;

    size_t index = 0;
    for ( ; index<m_logins.size(); ++index )
        if ( m_logins[ index ].requestId == event.requestId )
            break;

    if ( index >= m_logins.size() )
    {
        // The end of a login that was cancelled or timed out, or of a logout: nobody waits for it
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosAuthProvider: An event arrived for a request that is not waited for any more" );
        return;
    }

    SLogin login = m_logins[ index ];

    if ( EOSEVENT_PIN_GRANT == event.kind )
    {
        AUTH::CAuthActionRequired action;
        action.kind = AUTH::AUTHACTION_SHOW_DEVICE_CODE;
        action.url = event.verificationUri;
        action.urlWithCode = event.verificationUriComplete;
        action.userCode = event.userCode;
        if ( event.expiresInSeconds > 0 )
            action.expiresAtMs = AUTH::GetEpochTimeInMs() + (UInt64) event.expiresInSeconds * 1000;
        RaiseActionRequired( action );
        return;
    }

    bool ended = false;

    if ( EOSEVENT_AUTH_LOGIN == event.kind && STAGE_AUTH == login.stage )
    {
        if ( EOSOUTCOME_SUCCESS != event.outcome )
        {
            CORE::CString message = "Epic Online Services did not log in (EOS_EResult ";
            message += CORE::Int32ToString( event.sdkResultCode );
            message += ")";
            FailLogin( login.source, ToFailureReason( event.outcome ), message );
            ended = true;
        }
        else
        {
            login.epicAccountId = event.epicAccountId;
            AUTH::CAccountRef account( AUTH::CAuthProviderIds::Epic, login.epicAccountId );
            CORE::CDynamicBuffer tokenBuffer( MaxIdTokenSize, true );
            char* token = static_cast< char* >( tokenBuffer.GetBufferPtr() );
            token[ 0 ] = '\0';
            bool haveToken = account.IsValid() && GUCEF_NULL != m_shim.authCopyIdToken &&
                             m_shim.authCopyIdToken( m_shim.context, login.epicAccountId.C_String(), token, MaxIdTokenSize ) && '\0' != token[ 0 ];
            if ( haveToken )
                login.idToken.Set( token );
            // The token is a credential: the buffer does not keep it
            AUTH::CAuthCrypto::Wipe( token, MaxIdTokenSize );

            if ( !haveToken )
            {
                FailLogin( login.source, AUTH::AUTHFAIL_INVALID_RESPONSE, "Epic Online Services gave no usable account id and id token" );
                ended = true;
            }
            else
            if ( m_settings.useConnect && GUCEF_NULL != m_shim.connectLogin &&
                 m_shim.connectLogin( m_shim.context, login.requestId, login.idToken.Reveal().C_String() ) )
            {
                m_logins[ index ] = login;
                m_logins[ index ].stage = STAGE_CONNECT;
                return;
            }
            else
            {
                EndLogin( login, CORE::CString::Empty );
                ended = true;
            }
        }
    }
    else
    if ( EOSEVENT_CONNECT_LOGIN == event.kind && STAGE_CONNECT == login.stage )
    {
        if ( EOSOUTCOME_SUCCESS == event.outcome )
        {
            EndLogin( login, event.productUserId );
            ended = true;
        }
        else
        if ( EOSOUTCOME_NEEDS_USER_CREATION == event.outcome && !login.linkTarget.IsNULLOrEmpty() && GUCEF_NULL != m_shim.connectLinkAccount &&
             m_shim.connectLinkAccount( m_shim.context, login.requestId, login.linkTarget.C_String() ) )
        {
            m_logins[ index ].stage = STAGE_LINK_ACCOUNT;
            return;
        }
        else
        if ( EOSOUTCOME_NEEDS_USER_CREATION == event.outcome && m_settings.createConnectUser && GUCEF_NULL != m_shim.connectCreateUser &&
             m_shim.connectCreateUser( m_shim.context, login.requestId ) )
        {
            m_logins[ index ].stage = STAGE_CREATE_USER;
            return;
        }
        else
        {
            // The account is logged in; what is missing is the product user id, which is extra
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: The Connect login did not give a product user id (EOS_EResult " << event.sdkResultCode << ")" );
            EndLogin( login, CORE::CString::Empty );
            ended = true;
        }
    }
    else
    if ( EOSEVENT_CONNECT_CREATE_USER == event.kind && STAGE_CREATE_USER == login.stage )
    {
        if ( EOSOUTCOME_SUCCESS != event.outcome )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: The product user could not be created (EOS_EResult " << event.sdkResultCode << ")" );
        EndLogin( login, EOSOUTCOME_SUCCESS == event.outcome ? CORE::CString( event.productUserId ) : CORE::CString::Empty );
        ended = true;
    }
    else
    if ( EOSEVENT_CONNECT_LINK_ACCOUNT == event.kind && STAGE_LINK_ACCOUNT == login.stage )
    {
        if ( EOSOUTCOME_SUCCESS != event.outcome )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: The account could not be linked to the product user (EOS_EResult " << event.sdkResultCode << ")" );
        // The account is linked to the product user that was named: that is its product user id
        EndLogin( login, EOSOUTCOME_SUCCESS == event.outcome ? login.linkTarget : CORE::CString::Empty );
        ended = true;
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosAuthProvider: An event arrived that does not fit the stage of the login" );
        return;
    }

    if ( ended )
    {
        // The target of a link is for one login
        if ( !login.linkTarget.IsNULLOrEmpty() && m_linkTarget == login.linkTarget )
            m_linkTarget.Clear();

        // The secret does not stay in the list
        m_logins[ index ].idToken.Clear();
        m_logins.erase( m_logins.begin() + index );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::OnTimer( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    // The platform is ticked while a login is under way or a logout has callbacks to come; when the other plugins
    // want it ticked faster than this provider asked for, the provider keeps its own pace
    if ( GUCEF_NULL != m_platform )
    {
        UInt32 interval = m_settings.tickIntervalInMs > 0 ? m_settings.tickIntervalInMs : CEosAuthSettings::DefaultTickIntervalInMs;
        if ( m_platform->GetTickInterval() > 0 && m_platform->GetTickInterval() < interval )
        {
            UInt64 nowInMs = AUTH::GetMonotonicTimeInMs();
            if ( nowInMs < m_lastRunInMs + interval )
                return;
            m_lastRunInMs = nowInMs;
        }
    }
    if ( GUCEF_NULL != m_shim.pollEvent )
    {
        SEosEvent event;
        while ( m_shim.pollEvent( m_shim.context, &event ) )
            HandleEvent( event );
    }
    if ( m_logins.empty() && m_idleTicksLeft > 0 )
        --m_idleTicksLeft;

    UInt64 now = AUTH::GetMonotonicTimeInMs();
    size_t i = 0;
    while ( i < m_logins.size() )
    {
        bool cancelled = m_logins[ i ].source.IsCancellationRequested();
        bool timedOut = now >= m_logins[ i ].deadlineInMs;
        if ( !cancelled && !timedOut )
        {
            ++i;
            continue;
        }

        SLogin ended = m_logins[ i ];
        m_logins[ i ].idToken.Clear();
        m_logins.erase( m_logins.begin() + i );
        if ( cancelled )
            FailLogin( ended.source, AUTH::AUTHFAIL_CANCELLED, "The login was cancelled" );
        else
            FailLogin( ended.source, AUTH::AUTHFAIL_TIMEOUT, "Epic Online Services did not finish the login in time" );
    }

    UpdateTickDemand();
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::Logout( const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_productUserIds.erase( account );
    if ( account.GetProvider() != AUTH::CAuthProviderIds::Epic || GUCEF_NULL == m_shim.authLogout )
        return true;
    if ( GUCEF_NULL == m_platform || !m_platform->IsReady() )
        return true;

    // The user is not logged in at the platform any more
    if ( m_platform->GetLoginState().epicAccountId == account.GetSubject() )
        m_platform->ClearLoginState();
    if ( m_loginStateAccount == account.GetSubject() )
        m_loginStateAccount.Clear();

    // The end of the logout is not waited for: the SDK forgets the local session when its callback has run
    m_shim.authLogout( m_shim.context, ++m_nextRequestId, account.GetSubject().C_String(), m_settings.deletePersistentAuthOnLogout );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: Logout of " << account.ToString() << " requested" );

    // The ticks that deliver the callback of the logout
    m_idleTicksLeft = LogoutTickCount;
    UpdateTickDemand();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::IsVerifySupported( void ) const
{GUCEF_TRACE;

    AUTH::IAuthFlowEngine* engine = AUTH::CAuthGlobal::Instance()->GetFlowEngine();
    if ( GUCEF_NULL == engine )
        return false;

    AUTH::CAuthProviderDescriptor descriptor;
    if ( !AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::EpicName, descriptor ) )
        return false;

    AUTH::CAuthProviderSettings engineSettings;
    {
        MT::CScopeMutex lock( m_lock );
        engineSettings = m_settings.GetEngineSettings();
    }
    return engine->IsSupported( descriptor, engineSettings );
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::Verify( const AUTH::CAuthProof& proof, AUTH::CAuthResult& result )
{GUCEF_TRACE;

    if ( proof.kind != AUTH::CAuthProof::KindOidcIdToken || proof.data.IsEmpty() )
        return false;

    AUTH::IAuthFlowEngine* engine = AUTH::CAuthGlobal::Instance()->GetFlowEngine();
    if ( GUCEF_NULL == engine )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: An Epic id token cannot be verified, there is no OIDC engine" );
        return false;
    }

    AUTH::CAuthProviderDescriptor descriptor;
    if ( !AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( AUTH::CAuthProviderRegistry::EpicName, descriptor ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: An Epic id token cannot be verified, there is no descriptor for it" );
        return false;
    }

    AUTH::CAuthProviderSettings engineSettings;
    {
        MT::CScopeMutex lock( m_lock );
        engineSettings = m_settings.GetEngineSettings();
    }
    if ( !engine->IsSupported( descriptor, engineSettings ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: The OIDC engine cannot verify Epic id tokens with these settings" );
        return false;
    }

    AUTH::CAuthResult verified;
    if ( !engine->Verify( descriptor, engineSettings, proof, verified ) )
        return false;

    // Only an Epic account is the account of this provider
    if ( !verified.accountRef.IsValid() || verified.accountRef.GetProvider() != AUTH::CAuthProviderIds::Epic )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthProvider: The verified token is not for an Epic account" );
        return false;
    }

    result = verified;
    result.providerName = ProviderName;
    return true;
}

/*-------------------------------------------------------------------------*/

CEosAuthSettings
CEosAuthProvider::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CEosAuthProvider::SetLinkTargetProductUserId( const CORE::CString& productUserId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_linkTarget = productUserId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosAuthProvider::GetProductUserId( const AUTH::CAccountRef& account ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TProductUserIdMap::const_iterator i = m_productUserIds.find( account );
    return i == m_productUserIds.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosAuthProvider::GetPendingLoginCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_logins.size();
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::EOS::CEosAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
