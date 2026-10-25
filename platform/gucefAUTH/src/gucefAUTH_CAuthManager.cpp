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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_COAUTH2ENGINE_H
#include "gucefAUTH_COAuth2Engine.h"
#define GUCEF_AUTH_COAUTH2ENGINE_H
#endif /* GUCEF_AUTH_COAUTH2ENGINE_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The time a provider has to end a login that was cancelled before the manager ends it itself */
static const UInt32 CancelGraceInMs = 2000;

/** A provider that ended the future without raising the event gets this long before the manager goes by the future */
static const UInt32 EventGraceInMs = 500;

static const UInt32 PendingCheckIntervalInMs = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAuthManager::LoginSucceededEvent = "GUCEF::AUTH::CAuthManager::LoginSucceededEvent";
const CORE::CEvent CAuthManager::LoginFailedEvent = "GUCEF::AUTH::CAuthManager::LoginFailedEvent";
const CORE::CEvent CAuthManager::ActionRequiredEvent = "GUCEF::AUTH::CAuthManager::ActionRequiredEvent";
const CORE::CEvent CAuthManager::LoginStateChangedEvent = "GUCEF::AUTH::CAuthManager::LoginStateChangedEvent";
const CORE::CEvent CAuthManager::AccountLinkedEvent = "GUCEF::AUTH::CAuthManager::AccountLinkedEvent";
const CORE::CEvent CAuthManager::AccountUnlinkedEvent = "GUCEF::AUTH::CAuthManager::AccountUnlinkedEvent";
const CORE::CEvent CAuthManager::AccountLoggedOutEvent = "GUCEF::AUTH::CAuthManager::AccountLoggedOutEvent";
const CORE::CEvent CAuthManager::AutoLoginFinishedEvent = "GUCEF::AUTH::CAuthManager::AutoLoginFinishedEvent";
const CORE::CEvent CAuthManager::SettingsChangedEvent = "GUCEF::AUTH::CAuthManager::SettingsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthStateEventData::CAuthStateEventData( void )
    : CORE::CICloneable()
    , state( AUTHLOGINSTATE_GUEST )
    , linkedAccountCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthStateEventData::CAuthStateEventData( TAuthLoginState loginState, UInt32 linkedCount )
    : CORE::CICloneable()
    , state( loginState )
    , linkedAccountCount( linkedCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthStateEventData::CAuthStateEventData( const CAuthStateEventData& src )
    : CORE::CICloneable( src )
    , state( src.state )
    , linkedAccountCount( src.linkedAccountCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthStateEventData::~CAuthStateEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthStateEventData&
CAuthStateEventData::operator=( const CAuthStateEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        state = src.state;
        linkedAccountCount = src.linkedAccountCount;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthStateEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthStateEventData( *this );
}

/*-------------------------------------------------------------------------*/

CAuthAccountEventData::CAuthAccountEventData( void )
    : CORE::CICloneable()
    , accountRef()
    , providerName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthAccountEventData::CAuthAccountEventData( const CAccountRef& account, const CORE::CString& provider )
    : CORE::CICloneable()
    , accountRef( account )
    , providerName( provider )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthAccountEventData::CAuthAccountEventData( const CAuthAccountEventData& src )
    : CORE::CICloneable( src )
    , accountRef( src.accountRef )
    , providerName( src.providerName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthAccountEventData::~CAuthAccountEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthAccountEventData&
CAuthAccountEventData::operator=( const CAuthAccountEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        accountRef = src.accountRef;
        providerName = src.providerName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthAccountEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthAccountEventData( *this );
}

/*-------------------------------------------------------------------------*/

CAuthManager::SPending::SPending( void )
    : managerSource()
    , providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , deadlineMs( 0 )
    , graceDeadlineMs( 0 )
    , readySeenMs( 0 )
    , hasProviderFuture( false )
    , linkToProfile( false )
    , isAutoStep( false )
    , isSilent( false )
    , isRefresh( false )
    , timedOut( false )
    , refreshAccountKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthManager::SAutoStep::SAutoStep( void )
    : silent( false )
    , providerName()
    , accountKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAuthManager::RegisterEvents( void )
{GUCEF_TRACE;

    LoginSucceededEvent.Initialize();
    LoginFailedEvent.Initialize();
    ActionRequiredEvent.Initialize();
    LoginStateChangedEvent.Initialize();
    AccountLinkedEvent.Initialize();
    AccountUnlinkedEvent.Initialize();
    AccountLoggedOutEvent.Initialize();
    AutoLoginFinishedEvent.Initialize();
    SettingsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAuthManager::CAuthManager( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_settings()
    , m_providerFactory( GUCEF_NULL )
    , m_providers()
    , m_credentialStore( GUCEF_NULL )
    , m_linkStore( GUCEF_NULL )
    , m_signer( GUCEF_NULL )
    , m_ownedCredentialStore( GUCEF_NULL )
    , m_ownedFileCredentialStore( GUCEF_NULL )
    , m_ownedLinkStore( GUCEF_NULL )
    , m_pending()
    , m_sessions()
    , m_timer( PendingCheckIntervalInMs )
    , m_autoRunning( false )
    , m_autoCancelled( false )
    , m_autoWaiting( false )
    , m_autoIndex( 0 )
    , m_autoSucceeded( 0 )
    , m_autoWaitingProvider()
    , m_autoSteps()
    , m_autoSilentSucceeded()
    , m_autoSource()
    , m_lastKnownState( AUTHLOGINSTATE_GUEST )
{GUCEF_TRACE;

    RegisterEvents();
    CIAuthProvider::RegisterEvents();
    TEventCallback timerCallback( this, &CAuthManager::OnPendingTimerUpdate );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CAuthManager::~CAuthManager()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_timer.SetEnabled( false );

    // Whoever waits on a login or on the startup sequence is told it is over
    TPendingMap::iterator i = m_pending.begin();
    while ( i != m_pending.end() )
    {
        if ( (*i).second.hasProviderFuture )
            (*i).second.providerFuture.RequestCancellation();
        (*i).second.managerSource.Cancel();
        ++i;
    }
    m_pending.clear();
    if ( m_autoRunning )
        m_autoSource.Cancel();

    TSessionMap::iterator s = m_sessions.begin();
    while ( s != m_sessions.end() )
    {
        (*s).second.WipeSecrets();
        ++s;
    }
    m_sessions.clear();

    // Providers go before the stores they were using and the factory that made them
    m_providers.clear();
    GUCEF_DELETE m_ownedLinkStore;
    m_ownedLinkStore = GUCEF_NULL;
    GUCEF_DELETE m_ownedCredentialStore;
    m_ownedCredentialStore = GUCEF_NULL;
    m_ownedFileCredentialStore = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::SetCredentialStore( ICredentialStore* store )
{GUCEF_TRACE;

    m_credentialStore = store;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::SetLinkStore( IIdentityLinkStore* store )
{GUCEF_TRACE;

    m_linkStore = store;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::SetSigner( ILinkSigner* signer )
{GUCEF_TRACE;

    m_signer = signer;
    if ( GUCEF_NULL != m_ownedLinkStore )
        m_ownedLinkStore->SetSigner( signer );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::SetProviderFactory( CAuthProviderFactory* factory )
{GUCEF_TRACE;

    m_providerFactory = factory;
}

/*-------------------------------------------------------------------------*/

const CAuthSettings&
CAuthManager::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::SetSettings( const CAuthSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    ApplyFormerProviderNames();
    // names of accounts are only kept while the setting says they may be
    if ( !m_settings.rememberLoginHints )
        m_settings.ClearLoginHints();
    if ( GUCEF_NULL != m_ownedFileCredentialStore )
        m_ownedFileCredentialStore->SetDirectory( m_settings.GetCredentialDirectory() );
    if ( GUCEF_NULL != m_ownedLinkStore )
        m_ownedLinkStore->SetPath( m_settings.GetLinkStorePath() );

    TProviderMap::iterator i = m_providers.begin();
    while ( i != m_providers.end() )
    {
        ConfigureProvider( *(*i).second.GetPointerAlways() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::ApplyFormerProviderNames( void )
{GUCEF_TRACE;

    CAuthProviderRegistry& registry = CAuthGlobal::Instance()->GetDescriptorRegistry();
    CORE::CString::StringVector names;
    registry.GetNames( names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        CAuthProviderDescriptor descriptor;
        if ( !registry.GetDescriptor( names[ i ], descriptor ) )
            continue;
        for ( size_t n=0; n<descriptor.formerNames.size(); ++n )
            if ( m_settings.RenameProvider( descriptor.formerNames[ n ], descriptor.name ) )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: The settings of the provider \"" << descriptor.formerNames[ n ] << "\" are the settings of \"" << descriptor.name << "\" now" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::ConfigureProvider( CIAuthProvider& provider )
{GUCEF_TRACE;

    CAuthProviderSettings providerSettings;
    if ( !m_settings.GetProviderSettings( provider.GetProviderName(), providerSettings ) )
        return;
    CORE::CDataNode node( "Provider", GUCEF_DATATYPE_OBJECT );
    providerSettings.SaveConfig( node );
    provider.LoadConfig( node );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::EnsureStores( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_credentialStore )
    {
        if ( CFileCredentialStore::IsAvailable() )
        {
            m_ownedFileCredentialStore = GUCEF_NEW CFileCredentialStore( m_settings.GetCredentialDirectory() );
            m_ownedCredentialStore = m_ownedFileCredentialStore;
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: There is no \"" << CAuthCrypto::AeadCipher << "\" codec, logins are not kept across restarts" );
            m_ownedCredentialStore = GUCEF_NEW CMemoryCredentialStore();
        }
        m_credentialStore = m_ownedCredentialStore;
    }
    if ( GUCEF_NULL == m_linkStore )
    {
        m_ownedLinkStore = GUCEF_NEW CLocalIdentityLinkStore( m_settings.GetLinkStorePath() );
        m_ownedLinkStore->SetSigner( m_signer );
        m_linkStore = m_ownedLinkStore;
    }
}

/*-------------------------------------------------------------------------*/

ICredentialStore*
CAuthManager::GetCredentialStore( void )
{GUCEF_TRACE;

    EnsureStores();
    return m_credentialStore;
}

/*-------------------------------------------------------------------------*/

IIdentityLinkStore*
CAuthManager::GetLinkStore( void )
{GUCEF_TRACE;

    EnsureStores();
    return m_linkStore;
}

/*-------------------------------------------------------------------------*/

UInt32
CAuthManager::RefreshProviders( void )
{GUCEF_TRACE;

    CAuthProviderFactory* factory = m_providerFactory;
    if ( GUCEF_NULL == factory )
        factory = &CAuthGlobal::Instance()->GetProviderFactory();

    // A descriptor that a plugin registered since the settings were set may be what an earlier name in them refers to
    ApplyFormerProviderNames();

    CAuthProviderFactory::TKeySet keys;
    factory->ObtainKeySet( keys );
    UInt32 made = 0;
    CAuthProviderFactory::TKeySet::const_iterator i = keys.begin();
    while ( i != keys.end() )
    {
        if ( m_providers.find( (*i) ) == m_providers.end() )
        {
            CIAuthProviderPtr provider = factory->Create( (*i) );
            if ( !provider.IsNULL() && AddProvider( provider ) )
                ++made;
        }
        ++i;
    }
    return made;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::AddProvider( const CIAuthProviderPtr& provider )
{GUCEF_TRACE;

    if ( provider.IsNULL() )
        return false;
    CIAuthProvider* raw = const_cast< CIAuthProviderPtr& >( provider ).GetPointerAlways();
    CORE::CString name = raw->GetProviderName();
    if ( name.IsNULLOrEmpty() || m_providers.find( name ) != m_providers.end() )
        return false;

    m_providers[ name ] = provider;
    TEventCallback actionCallback( this, &CAuthManager::OnProviderActionRequired );
    TEventCallback succeededCallback( this, &CAuthManager::OnProviderLoginSucceeded );
    TEventCallback failedCallback( this, &CAuthManager::OnProviderLoginFailed );
    SubscribeTo( raw, CIAuthProvider::ActionRequiredEvent, actionCallback );
    SubscribeTo( raw, CIAuthProvider::LoginSucceededEvent, succeededCallback );
    SubscribeTo( raw, CIAuthProvider::LoginFailedEvent, failedCallback );
    ConfigureProvider( *raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::RemoveProvider( const CORE::CString& providerName )
{GUCEF_TRACE;

    TProviderMap::iterator i = m_providers.find( providerName );
    if ( i == m_providers.end() )
        return false;

    TPendingMap::iterator p = m_pending.find( providerName );
    if ( p != m_pending.end() )
    {
        if ( (*p).second.hasProviderFuture )
            (*p).second.providerFuture.RequestCancellation();
        (*p).second.managerSource.Cancel();
        m_pending.erase( p );
        UpdateTimer();
    }
    UnsubscribeFrom( (*i).second.GetPointerAlways() );
    m_providers.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

CIAuthProvider*
CAuthManager::GetProvider( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    TProviderMap::const_iterator i = m_providers.find( providerName );
    if ( i == m_providers.end() )
        return GUCEF_NULL;
    return const_cast< CIAuthProviderPtr& >( (*i).second ).GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::GetProviderNames( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    TProviderMap::const_iterator i = m_providers.begin();
    while ( i != m_providers.end() )
    {
        names.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::IsProviderOffered( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    CIAuthProvider* provider = GetProvider( providerName );
    if ( GUCEF_NULL == provider )
        return false;
    if ( !m_settings.IsProviderOffered( providerName, 0 != ( provider->GetCapabilities() & AUTHCAP_REQUIRES_CLIENT_ID ) ) )
        return false;
    return provider->IsAvailable();
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::GetUnavailableProvider( const CORE::CString& providerName, CAuthUnavailableProvider& unavailable ) const
{GUCEF_TRACE;

    unavailable.Clear();
    if ( !m_settings.IsProviderEnabled( providerName ) )
        return false;

    CIAuthProvider* provider = GetProvider( providerName );
    if ( GUCEF_NULL == provider )
    {
        // a provider that only a plugin makes: the descriptor is there, the plugin is not
        CAuthProviderDescriptor descriptor;
        if ( CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( providerName, descriptor ) && AUTHPROTOCOL_PLATFORM == descriptor.protocol )
        {
            unavailable.providerName = providerName;
            unavailable.reason = AUTHUNAVAILABLE_MISSING_PLUGIN;
            unavailable.message = "Not available in this build: the plugin for this login is not loaded";
            return true;
        }
        return false;
    }
    if ( IsProviderOffered( providerName ) )
        return false;
    if ( !provider->GetUnavailability( unavailable ) )
    {
        unavailable.Clear();
        return false;
    }
    unavailable.providerName = providerName;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::GetUnavailableProviders( TAuthUnavailableProviderVector& providers ) const
{GUCEF_TRACE;

    providers.clear();
    if ( !m_settings.showUnavailableProviders )
        return;

    CORE::CString::StringVector names;
    TProviderMap::const_iterator i = m_providers.begin();
    while ( i != m_providers.end() )
    {
        names.push_back( (*i).first );
        ++i;
    }
    // descriptors that no provider is registered for: the plugin that makes them is missing
    CORE::CString::StringVector descriptorNames;
    CAuthGlobal::Instance()->GetDescriptorRegistry().GetNames( descriptorNames );
    for ( size_t d=0; d<descriptorNames.size(); ++d )
    {
        if ( m_providers.find( descriptorNames[ d ] ) == m_providers.end() )
            names.push_back( descriptorNames[ d ] );
    }

    for ( size_t n=0; n<names.size(); ++n )
    {
        CAuthUnavailableProvider unavailable;
        if ( GetUnavailableProvider( names[ n ], unavailable ) )
            providers.push_back( unavailable );
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::GetOfferedProviders( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    for ( size_t i=0; i<m_settings.lastUsedProviders.size(); ++i )
        if ( IsProviderOffered( m_settings.lastUsedProviders[ i ] ) && !ContainsString( names, m_settings.lastUsedProviders[ i ] ) )
            names.push_back( m_settings.lastUsedProviders[ i ] );

    TProviderMap::const_iterator i = m_providers.begin();
    while ( i != m_providers.end() )
    {
        if ( !ContainsString( names, (*i).first ) && IsProviderOffered( (*i).first ) )
            names.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::FailImmediately( const CORE::CString& providerName     ,
                               TAuthFailureReason reason             ,
                               const CORE::CString& message          ,
                               CORE::CFutureResult& future           )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    future = source.GetFuture();
    if ( AUTHFAIL_CANCELLED == reason )
    {
        source.Cancel();
    }
    else
    {
        CORE::CString extraInfo = AuthFailureReasonToString( reason );
        extraInfo += ": ";
        extraInfo += message;
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, extraInfo );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Login with " << providerName << " refused (" << AuthFailureReasonToString( reason ) << "): " << message );
    CAuthFailure failure( providerName, reason, message );
    NotifyObservers( LoginFailedEvent, &failure );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::ApplyRememberedLoginHint( const CORE::CString& providerName, CAuthOptions& options ) const
{GUCEF_TRACE;

    if ( !options.loginHint.IsNULLOrEmpty() )
        return;
    CORE::CString remembered;
    if ( !m_settings.GetLoginHint( providerName, remembered ) )
        return;

    // only a provider that is known to take the parameter is told: the others have no use for the name of the account
    CAuthProviderDescriptor descriptor;
    if ( CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( providerName, descriptor ) && descriptor.supportsLoginHint )
        options.loginHint = remembered;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::HasLoginHints( void ) const
{GUCEF_TRACE;

    return m_settings.HasLoginHints();
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::ClearLoginHints( void )
{GUCEF_TRACE;

    if ( !m_settings.ClearLoginHints() )
        return false;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: The remembered account names were forgotten" );
    NotifyObservers( SettingsChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::StartLogin( const CORE::CString& providerName, const CAuthOptions& options, bool forceLink, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CIAuthProvider* provider = GetProvider( providerName );
    if ( GUCEF_NULL == provider )
        return FailImmediately( providerName, AUTHFAIL_NOT_AVAILABLE, "There is no such provider", future );
    if ( !IsProviderOffered( providerName ) )
        return FailImmediately( providerName, AUTHFAIL_NOT_AVAILABLE, "The provider is not offered: it is disabled, not configured or not available here", future );
    if ( m_pending.find( providerName ) != m_pending.end() )
        return FailImmediately( providerName, AUTHFAIL_IN_PROGRESS, "A login with the provider is already under way", future );

    // the explicit hint of the caller wins over the one remembered, and a login that says the default time gets the one of the settings
    CAuthOptions effectiveOptions( options );
    ApplyRememberedLoginHint( providerName, effectiveOptions );
    if ( CAuthOptions::DefaultTimeoutInMs == effectiveOptions.timeoutInMs )
    {
        // a device code lasts as long as the provider says, up to a ceiling; the browser gets the time of the settings
        effectiveOptions.timeoutInMs = effectiveOptions.preferDeviceCode ? CAuthOptions::MaxDeviceCodeTimeoutInMs
                                                                         : CAuthSettings::ClampLoginTimeoutInSeconds( m_settings.loginTimeoutInSeconds ) * 1000;
    }

    SPending pending;
    pending.linkToProfile = forceLink || options.linkToProfile;
    if ( 0 != effectiveOptions.timeoutInMs )
        pending.deadlineMs = GetMonotonicTimeInMs() + effectiveOptions.timeoutInMs + m_settings.loginDeadlineSlackInMs;
    m_pending[ providerName ] = pending;
    future = pending.managerSource.GetFuture();
    UpdateTimer();

    CORE::CFutureResult providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( !provider->BeginLogin( effectiveOptions, providerFuture ) )
    {
        CAuthFailure failure( providerName, AUTHFAIL_NOT_AVAILABLE, "The provider could not start the login" );
        FinishPending( providerName, GUCEF_NULL, &failure );
        return false;
    }
    TPendingMap::iterator i = m_pending.find( providerName );
    if ( i != m_pending.end() )
    {
        (*i).second.providerFuture = providerFuture;
        (*i).second.hasProviderFuture = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::Login( const CORE::CString& providerName, const CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return StartLogin( providerName, options, false, future );
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::Link( const CORE::CString& providerName, const CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return StartLogin( providerName, options, true, future );
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::CancelLogin( const CORE::CString& providerName )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_pending.find( providerName );
    if ( i == m_pending.end() )
        return false;
    if ( 0 == (*i).second.graceDeadlineMs )
        (*i).second.graceDeadlineMs = GetMonotonicTimeInMs() + CancelGraceInMs;
    if ( (*i).second.hasProviderFuture )
        (*i).second.providerFuture.RequestCancellation();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::IsLoginPending( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return m_pending.find( providerName ) != m_pending.end();
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::UpdateTimer( void )
{GUCEF_TRACE;

    bool needed = !m_pending.empty();
    if ( needed != m_timer.GetEnabled() )
        m_timer.SetEnabled( needed );
}

/*-------------------------------------------------------------------------*/

UInt32
CAuthManager::CountLinks( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_signer )
        return 0;
    CAuthManager* self = const_cast< CAuthManager* >( this );
    IIdentityLinkStore* store = self->GetLinkStore();
    TLinkRecordVector links;
    if ( GUCEF_NULL == store || !store->GetLinks( m_signer->GetKeyId(), links ) )
        return 0;
    return (UInt32) links.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthManager::GetProfileKeyId( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_signer ? m_signer->GetKeyId() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

TAuthLoginState
CAuthManager::GetLoginState( void ) const
{GUCEF_TRACE;

    return CountLinks() > 0 ? AUTHLOGINSTATE_LINKED : AUTHLOGINSTATE_GUEST;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::GetLinkedAccounts( TLinkRecordVector& links ) const
{GUCEF_TRACE;

    links.clear();
    if ( GUCEF_NULL == m_signer )
        return false;
    CAuthManager* self = const_cast< CAuthManager* >( this );
    IIdentityLinkStore* store = self->GetLinkStore();
    return GUCEF_NULL != store && store->GetLinks( m_signer->GetKeyId(), links );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::GetSessionAccounts( GUCEF::vector< CAuthResult >& sessions ) const
{GUCEF_TRACE;

    sessions.clear();
    TSessionMap::const_iterator i = m_sessions.begin();
    while ( i != m_sessions.end() )
    {
        sessions.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::GetSession( const CAccountRef& account, CAuthResult& session ) const
{GUCEF_TRACE;

    TSessionMap::const_iterator i = m_sessions.find( account );
    if ( i == m_sessions.end() )
        return false;
    session = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::RaiseStateChangeIfNeeded( TAuthLoginState previous )
{GUCEF_TRACE;

    UInt32 count = CountLinks();
    TAuthLoginState now = count > 0 ? AUTHLOGINSTATE_LINKED : AUTHLOGINSTATE_GUEST;
    m_lastKnownState = now;
    if ( now != previous )
    {
        CAuthStateEventData data( now, count );
        NotifyObservers( LoginStateChangedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::LinkAccount( const CAuthResult& result, TAuthFailureReason& failureReason, CORE::CString& failureMessage )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_signer )
        return true;
    IIdentityLinkStore* store = GetLinkStore();
    if ( GUCEF_NULL == store )
        return true;

    CORE::CString profile = m_signer->GetKeyId();
    CORE::CString existing;
    if ( store->Resolve( result.accountRef, existing ) )
    {
        if ( existing == profile )
            return true;
        failureReason = AUTHFAIL_ACCOUNT_LINKED_ELSEWHERE;
        failureMessage = "The account is linked to another profile";
        return false;
    }

    CLinkRecord record;
    if ( !CLinkRecord::Create( *m_signer, result.accountRef, result.proof.GetDigestHex(), GetEpochTimeInMs(), record ) )
    {
        failureReason = AUTHFAIL_INTERNAL;
        failureMessage = "The link record could not be signed, is there an \"ed25519\" signature codec?";
        return false;
    }
    TLinkResult linked = store->Link( record );
    if ( LINKRESULT_CONFLICT == linked )
    {
        failureReason = AUTHFAIL_ACCOUNT_LINKED_ELSEWHERE;
        failureMessage = "The account is linked to another profile";
        return false;
    }
    if ( LINKRESULT_OK != linked )
    {
        failureReason = AUTHFAIL_INTERNAL;
        failureMessage = "The link could not be kept";
        return false;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Linked " << result.accountRef.ToString() << " to the profile " << profile );
    CAuthAccountEventData data( result.accountRef, result.providerName );
    NotifyObservers( AccountLinkedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::FinishPending( const CORE::CString& providerName, const CAuthResult* result, const CAuthFailure* failure )
{GUCEF_TRACE;

    TPendingMap::iterator i = m_pending.find( providerName );
    if ( i == m_pending.end() )
        return;
    SPending pending = (*i).second;
    m_pending.erase( i );
    UpdateTimer();

    CAuthFailure outcome;
    bool succeeded = GUCEF_NULL != result;
    TAuthLoginState previousState = GetLoginState();
    if ( succeeded )
    {
        TAuthFailureReason reason = AUTHFAIL_UNKNOWN;
        CORE::CString message;
        // The account is linked before anything of it is kept: an account of another profile leaves no trace
        if ( pending.linkToProfile && !LinkAccount( *result, reason, message ) )
        {
            succeeded = false;
            outcome = CAuthFailure( providerName, reason, message );
        }
    }
    else
    if ( GUCEF_NULL != failure )
    {
        outcome = *failure;
        // A login that was cancelled because its time was up is a timeout
        if ( AUTHFAIL_CANCELLED == outcome.reason && pending.timedOut )
            outcome.reason = AUTHFAIL_TIMEOUT;
    }
    else
    {
        outcome = CAuthFailure( providerName, AUTHFAIL_UNKNOWN, "The login ended without an outcome" );
    }

    if ( succeeded )
    {
        CAuthResult kept( *result );
        kept.providerName = providerName;
        if ( !kept.refreshToken.IsEmpty() )
        {
            ICredentialStore* store = GetCredentialStore();
            if ( GUCEF_NULL == store || !store->Store( providerName, kept.accountRef.ToString(), kept.refreshToken ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: The login of " << kept.accountRef.ToString() << " cannot be kept for the next start" );
        }
        m_sessions[ kept.accountRef ] = kept;
        // what names the account at the provider is remembered for the next login, after a login that worked and only then
        bool settingsChanged = m_settings.TouchLastUsed( providerName );
        if ( !kept.loginHint.IsNULLOrEmpty() && m_settings.SetLoginHint( providerName, kept.loginHint ) )
            settingsChanged = true;
        if ( settingsChanged )
            NotifyObservers( SettingsChangedEvent );

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: " << kept.accountRef.ToString() << " is logged in with " << providerName );
        pending.managerSource.Complete( kept.Clone(), true );
        NotifyObservers( LoginSucceededEvent, &kept );
        kept.WipeSecrets();
        if ( pending.isAutoStep )
        {
            ++m_autoSucceeded;
            if ( pending.isSilent )
                m_autoSilentSucceeded.insert( providerName );
        }
    }
    else
    {
        // A refresh token the provider does not accept anymore is of no use: the next start does not try it again
        if ( AUTHFAIL_INVALID_GRANT == outcome.reason && pending.isRefresh && !pending.refreshAccountKey.IsNULLOrEmpty() )
        {
            ICredentialStore* store = GetCredentialStore();
            if ( GUCEF_NULL != store )
                store->Remove( providerName, pending.refreshAccountKey );
        }
        if ( AUTHFAIL_CANCELLED == outcome.reason )
        {
            pending.managerSource.Cancel();
        }
        else
        {
            CORE::CString extraInfo = AuthFailureReasonToString( outcome.reason );
            extraInfo += ": ";
            extraInfo += outcome.message;
            pending.managerSource.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, extraInfo );
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Login with " << providerName << " failed (" << AuthFailureReasonToString( outcome.reason ) << "): " << outcome.message );
        NotifyObservers( LoginFailedEvent, &outcome );
    }

    RaiseStateChangeIfNeeded( previousState );

    if ( pending.isAutoStep )
    {
        m_autoWaiting = false;
        m_autoWaitingProvider.Clear();
        RunAutoSteps();
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnProviderSucceeded( const CAuthResult& result )
{GUCEF_TRACE;

    FinishPending( result.providerName, &result, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnProviderFailed( const CAuthFailure& failure )
{GUCEF_TRACE;

    FinishPending( failure.providerName, GUCEF_NULL, &failure );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::CheckPending( void )
{GUCEF_TRACE;

    UInt64 now = GetMonotonicTimeInMs();
    CORE::CString::StringVector names;
    TPendingMap::const_iterator i = m_pending.begin();
    while ( i != m_pending.end() )
    {
        names.push_back( (*i).first );
        ++i;
    }

    for ( size_t n=0; n<names.size(); ++n )
    {
        TPendingMap::iterator p = m_pending.find( names[ n ] );
        if ( p == m_pending.end() )
            continue;
        SPending& pending = (*p).second;
        if ( !pending.hasProviderFuture )
            continue;

        // A provider that ended its future and never raised the event: the future is what counts
        if ( pending.providerFuture.IsReady() )
        {
            if ( 0 == pending.readySeenMs )
            {
                pending.readySeenMs = now;
            }
            else
            if ( now - pending.readySeenMs >= EventGraceInMs )
            {
                CAuthResult result;
                if ( CAuthResult::TryGetFromFuture( pending.providerFuture, result ) )
                {
                    result.providerName = names[ n ];
                    FinishPending( names[ n ], &result, GUCEF_NULL );
                }
                else
                {
                    CORE::CTaskPtr task = pending.providerFuture.GetResult();
                    bool cancelled = !task.IsNULL() && CORE::TTaskStatus::TASKSTATUS_CANCELLED == task->GetTaskStatus();
                    CAuthFailure failure( names[ n ], cancelled ? AUTHFAIL_CANCELLED : AUTHFAIL_INTERNAL, "The provider ended the login without saying how" );
                    FinishPending( names[ n ], GUCEF_NULL, &failure );
                }
            }
            continue;
        }

        if ( 0 != pending.graceDeadlineMs )
        {
            // The cancellation was asked and the provider did not end the login in time
            if ( now >= pending.graceDeadlineMs )
            {
                CAuthFailure failure( names[ n ], AUTHFAIL_CANCELLED, "The provider did not end the cancelled login in time" );
                FinishPending( names[ n ], GUCEF_NULL, &failure );
            }
        }
        else
        if ( 0 != pending.deadlineMs && now >= pending.deadlineMs )
        {
            pending.timedOut = true;
            pending.graceDeadlineMs = now + CancelGraceInMs;
            pending.providerFuture.RequestCancellation();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::BuildAutoSteps( void )
{GUCEF_TRACE;

    m_autoSteps.clear();

    // The last used providers in their order, then the others
    CORE::CString::StringVector ordered;
    for ( size_t i=0; i<m_settings.lastUsedProviders.size(); ++i )
        if ( m_providers.find( m_settings.lastUsedProviders[ i ] ) != m_providers.end() && !ContainsString( ordered, m_settings.lastUsedProviders[ i ] ) )
            ordered.push_back( m_settings.lastUsedProviders[ i ] );
    size_t lastUsedCount = ordered.size();
    TProviderMap::const_iterator p = m_providers.begin();
    while ( p != m_providers.end() )
    {
        if ( !ContainsString( ordered, (*p).first ) )
            ordered.push_back( (*p).first );
        ++p;
    }

    for ( size_t i=0; i<ordered.size(); ++i )
    {
        CIAuthProvider* provider = GetProvider( ordered[ i ] );
        if ( GUCEF_NULL != provider && 0 != ( provider->GetCapabilities() & AUTHCAP_SILENT ) && IsProviderOffered( ordered[ i ] ) )
        {
            SAutoStep step;
            step.silent = true;
            step.providerName = ordered[ i ];
            m_autoSteps.push_back( step );
        }
    }

    ICredentialStore* store = GetCredentialStore();
    for ( size_t i=0; i<lastUsedCount && GUCEF_NULL != store; ++i )
    {
        CIAuthProvider* provider = GetProvider( ordered[ i ] );
        if ( GUCEF_NULL == provider || 0 == ( provider->GetCapabilities() & AUTHCAP_REFRESHABLE ) || !IsProviderOffered( ordered[ i ] ) )
            continue;
        CORE::CString::StringVector accountKeys;
        store->ListAccounts( ordered[ i ], accountKeys );
        for ( size_t a=0; a<accountKeys.size(); ++a )
        {
            SAutoStep step;
            step.silent = false;
            step.providerName = ordered[ i ];
            step.accountKey = accountKeys[ a ];
            m_autoSteps.push_back( step );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::AutoLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( m_autoRunning )
    {
        future = m_autoSource.GetFuture();
        return true;
    }

    m_autoSource = CORE::CFutureResultSource();
    future = m_autoSource.GetFuture();
    m_autoRunning = true;
    m_autoCancelled = false;
    m_autoWaiting = false;
    m_autoIndex = 0;
    m_autoSucceeded = 0;
    m_autoSilentSucceeded.clear();
    m_autoSteps.clear();
    m_autoWaitingProvider.Clear();

    if ( !m_settings.autoLogin )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Auto login is off" );
        FinishAutoLogin();
        return true;
    }

    BuildAutoSteps();
    RunAutoSteps();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::IsAutoLoginRunning( void ) const
{GUCEF_TRACE;

    return m_autoRunning;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::CancelAutoLogin( void )
{GUCEF_TRACE;

    if ( !m_autoRunning )
        return;
    m_autoCancelled = true;
    if ( m_autoWaiting )
        CancelLogin( m_autoWaitingProvider );
    else
        FinishAutoLogin();
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::FinishAutoLogin( void )
{GUCEF_TRACE;

    m_autoRunning = false;
    m_autoWaiting = false;
    m_autoWaitingProvider.Clear();
    m_autoSteps.clear();

    UInt32 count = CountLinks();
    const CORE::CString outcome = count > 0 ? "linked" : "guest";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Auto login finished, " << m_autoSucceeded << " logins, " << outcome );
    m_autoSource.Complete();
    CAuthStateEventData data( count > 0 ? AUTHLOGINSTATE_LINKED : AUTHLOGINSTATE_GUEST, count );
    NotifyObservers( AutoLoginFinishedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::RunAutoSteps( void )
{GUCEF_TRACE;

    if ( !m_autoRunning || m_autoWaiting )
        return;

    while ( m_autoIndex < m_autoSteps.size() && !m_autoCancelled )
    {
        SAutoStep step = m_autoSteps[ m_autoIndex ];
        ++m_autoIndex;

        CIAuthProvider* provider = GetProvider( step.providerName );
        // A provider the user is already logging in with is left to the user
        if ( GUCEF_NULL == provider || m_pending.find( step.providerName ) != m_pending.end() )
            continue;

        SPending pending;
        pending.isAutoStep = true;
        pending.isSilent = step.silent;
        pending.isRefresh = !step.silent;
        pending.refreshAccountKey = step.accountKey;
        pending.linkToProfile = true;
        if ( m_settings.autoLoginStepTimeoutInMs > 0 )
            pending.deadlineMs = GetMonotonicTimeInMs() + m_settings.autoLoginStepTimeoutInMs;

        CORE::CFutureResult providerFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        bool started = false;
        m_pending[ step.providerName ] = pending;
        if ( step.silent )
        {
            started = provider->TrySilentLogin( providerFuture );
        }
        else
        {
            if ( m_autoSilentSucceeded.find( step.providerName ) != m_autoSilentSucceeded.end() )
            {
                // the session of the platform has the account already
                m_pending.erase( step.providerName );
                continue;
            }
            CAccountRef account;
            CSecretString secret;
            ICredentialStore* store = GetCredentialStore();
            if ( CAccountRef::TryParse( step.accountKey, account ) && GUCEF_NULL != store && store->Load( step.providerName, step.accountKey, secret ) )
                started = provider->Refresh( account, secret, providerFuture );
        }

        if ( !started )
        {
            m_pending.erase( step.providerName );
            continue;
        }

        TPendingMap::iterator i = m_pending.find( step.providerName );
        if ( i != m_pending.end() )
        {
            (*i).second.providerFuture = providerFuture;
            (*i).second.hasProviderFuture = true;
        }
        m_autoWaiting = true;
        m_autoWaitingProvider = step.providerName;
        UpdateTimer();
        return;
    }

    UpdateTimer();
    FinishAutoLogin();
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::Logout( const CAccountRef& account )
{GUCEF_TRACE;

    if ( !account.IsValid() )
        return false;
    CORE::CString accountKey = account.ToString();
    CORE::CString providerName;
    CSecretString refreshToken;

    TSessionMap::iterator s = m_sessions.find( account );
    if ( s != m_sessions.end() )
    {
        providerName = (*s).second.providerName;
        refreshToken = (*s).second.refreshToken;
    }

    ICredentialStore* store = GetCredentialStore();
    if ( providerName.IsNULLOrEmpty() && GUCEF_NULL != store )
    {
        TProviderMap::const_iterator p = m_providers.begin();
        while ( p != m_providers.end() && providerName.IsNULLOrEmpty() )
        {
            CORE::CString::StringVector keys;
            store->ListAccounts( (*p).first, keys );
            if ( ContainsString( keys, accountKey ) )
                providerName = (*p).first;
            ++p;
        }
    }
    if ( providerName.IsNULLOrEmpty() )
        return false;

    if ( refreshToken.IsEmpty() && GUCEF_NULL != store )
        store->Load( providerName, accountKey, refreshToken );
    CIAuthProvider* provider = GetProvider( providerName );
    if ( GUCEF_NULL != provider )
        provider->Logout( account, refreshToken );
    if ( GUCEF_NULL != store )
        store->Remove( providerName, accountKey );
    if ( s != m_sessions.end() )
    {
        (*s).second.WipeSecrets();
        m_sessions.erase( s );
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: " << accountKey << " is logged out of " << providerName );
    CAuthAccountEventData data( account, providerName );
    NotifyObservers( AccountLoggedOutEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::Unlink( const CAccountRef& account )
{GUCEF_TRACE;

    if ( !account.IsValid() || GUCEF_NULL == m_signer )
        return false;
    IIdentityLinkStore* store = GetLinkStore();
    if ( GUCEF_NULL == store )
        return false;

    TAuthLoginState previousState = GetLoginState();
    CORE::CString profile;
    bool wasLinked = store->Resolve( account, profile ) && profile == m_signer->GetKeyId();
    if ( !store->Unlink( account, m_signer->GetKeyId() ) )
        return false;

    CORE::CString providerName;
    TSessionMap::const_iterator s = m_sessions.find( account );
    if ( s != m_sessions.end() )
        providerName = (*s).second.providerName;
    Logout( account );

    if ( wasLinked )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthManager: Unlinked " << account.ToString() << " from the profile " << m_signer->GetKeyId() );
        CAuthAccountEventData data( account, providerName );
        NotifyObservers( AccountUnlinkedEvent, &data );
    }
    RaiseStateChangeIfNeeded( previousState );
    return true;
}

/*-------------------------------------------------------------------------*/

TLinkResult
CAuthManager::MergeProfile( const CORE::CString& fromProfileKeyId, UInt32* mergedCount )
{GUCEF_TRACE;

    if ( GUCEF_NULL != mergedCount )
        *mergedCount = 0;
    if ( GUCEF_NULL == m_signer )
        return LINKRESULT_INVALID_RECORD;
    IIdentityLinkStore* store = GetLinkStore();
    if ( GUCEF_NULL == store )
        return LINKRESULT_STORE_ERROR;

    TAuthLoginState previousState = GetLoginState();
    TLinkResult merged = store->Merge( fromProfileKeyId, *m_signer, mergedCount );
    RaiseStateChangeIfNeeded( previousState );
    return merged;
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnPendingTimerUpdate( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CheckPending();
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnProviderActionRequired( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the event id says what the provider attached
    if ( GUCEF_NULL != eventData )
        NotifyObservers( ActionRequiredEvent, static_cast< CAuthActionRequired* >( eventData ) );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnProviderLoginSucceeded( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the event id says what the provider attached
    if ( GUCEF_NULL != eventData )
        OnProviderSucceeded( *static_cast< CAuthResult* >( eventData ) );
}

/*-------------------------------------------------------------------------*/

void
CAuthManager::OnProviderLoginFailed( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the event id says what the provider attached
    if ( GUCEF_NULL != eventData )
        OnProviderFailed( *static_cast< CAuthFailure* >( eventData ) );
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CAuthManager::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CAuthSettings settings( m_settings );
    if ( !settings.LoadConfig( config ) )
        return false;
    SetSettings( settings );

    // The settings of the built in flow engine are part of the same section of the configuration
    const CORE::CDataNode* section = config.GetName() == "Auth" ? &config : config.FindChild( "Auth" );
    if ( GUCEF_NULL != section && GUCEF_NULL != section->FindChild( "Engine" ) )
    {
        COAuth2Engine* engine = CAuthGlobal::Instance()->GetOAuth2Engine();
        if ( GUCEF_NULL != engine )
        {
            COAuth2EngineSettings engineSettings( engine->GetSettings() );
            if ( engineSettings.LoadConfig( *section ) )
                engine->SetSettings( engineSettings );
        }
    }

    RefreshProviders();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthManager";
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
