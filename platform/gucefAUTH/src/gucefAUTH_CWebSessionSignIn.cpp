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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */
#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */
#ifndef GUCEF_AUTH_CCHROMESIGNINBROWSER_H
#include "gucefAUTH_CChromeSignInBrowser.h"
#define GUCEF_AUTH_CCHROMESIGNINBROWSER_H
#endif /* GUCEF_AUTH_CCHROMESIGNINBROWSER_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H
#include "gucefAUTH_CWebSessionSignInTaskConsumer.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINTASKCONSUMER_H ? */
#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H
#include "gucefAUTH_CWebSessionSignInTaskData.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINTASKDATA_H ? */
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
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWebSessionSignIn::SignInStartingEvent = "GUCEF::AUTH::CWebSessionSignIn::SignInStartingEvent";
const CORE::CEvent CWebSessionSignIn::ProgressEvent = "GUCEF::AUTH::CWebSessionSignIn::ProgressEvent";
const CORE::CEvent CWebSessionSignIn::OwnerActionNeededEvent = "GUCEF::AUTH::CWebSessionSignIn::OwnerActionNeededEvent";
const CORE::CEvent CWebSessionSignIn::SignedInEvent = "GUCEF::AUTH::CWebSessionSignIn::SignedInEvent";
const CORE::CEvent CWebSessionSignIn::SignInFailedEvent = "GUCEF::AUTH::CWebSessionSignIn::SignInFailedEvent";
const CORE::CEvent CWebSessionSignIn::SessionEndedEvent = "GUCEF::AUTH::CWebSessionSignIn::SessionEndedEvent";
const CORE::CString CWebSessionSignIn::DefaultThreadPoolName = "WebSignIn";

/*-------------------------------------------------------------------------*/

namespace {

UInt64
NowInMs( void )
{
    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/** The condition of a check holds (1), does not (0), or the browser is gone (-1) */
const Int32 CONDITION_HOLDS = 1;
const Int32 CONDITION_DOES_NOT_HOLD = 0;
const Int32 CONDITION_NO_BROWSER = -1;

}

/*-------------------------------------------------------------------------*/

/** The state of one sign in while it runs */
struct CWebSessionSignIn::SRunContext
{
    CWebSessionSignInSettings settings;
    WEB::TWebSignInMode mode;
    CIWebSignInBrowser* browser;
    CWebSignInCredentials credentials;
    UInt64 deadline;
    GUCEF::vector< const WEB::CWebLoginScriptStep* > watchers;
    bool handedOver;
    bool signedIn;
    CORE::CString message;

    SRunContext( void )
        : settings()
        , mode( WEB::WEBSIGNIN_MODE_INTERACTIVE )
        , browser( GUCEF_NULL )
        , credentials()
        , deadline( 0 )
        , watchers()
        , handedOver( false )
        , signedIn( false )
        , message()
    {}
};

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::RegisterEvents( void )
{GUCEF_TRACE;

    SignInStartingEvent.Initialize();
    ProgressEvent.Initialize();
    OwnerActionNeededEvent.Initialize();
    SignedInEvent.Initialize();
    SignInFailedEvent.Initialize();
    SessionEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebSessionSignIn::CWebSessionSignIn( void )
    : CORE::CTSGNotifier()
    , m_dataLock()
    , m_settings()
    , m_browser( GUCEF_NULL )
    , m_credentialStore( GUCEF_NULL )
    , m_sessionStore( GUCEF_NULL )
    , m_sessionEndedCheck( GUCEF_NULL )
    , m_defaultSessionEndedCheck()
    , m_credentials()
    , m_session()
    , m_clearBrowserCookies( false )
    , m_running( false )
    , m_threadPoolName( DefaultThreadPoolName )
    , m_cancelRequested( 0 )
    , m_pendingAsync( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWebSessionSignIn::CWebSessionSignIn( const CWebSessionSignInSettings& settings )
    : CORE::CTSGNotifier()
    , m_dataLock()
    , m_settings()
    , m_browser( GUCEF_NULL )
    , m_credentialStore( GUCEF_NULL )
    , m_sessionStore( GUCEF_NULL )
    , m_sessionEndedCheck( GUCEF_NULL )
    , m_defaultSessionEndedCheck()
    , m_credentials()
    , m_session()
    , m_clearBrowserCookies( false )
    , m_running( false )
    , m_threadPoolName( DefaultThreadPoolName )
    , m_cancelRequested( 0 )
    , m_pendingAsync( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

CWebSessionSignIn::~CWebSessionSignIn()
{GUCEF_TRACE;

    Cancel();
    if ( !WaitForAsyncSignIn( 60000 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "WebSessionSignIn: Destroyed while an async sign in still runs" );
    }
    SignalUpcomingDestruction();
    m_credentials.Clear();
    m_session.Clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWebSessionSignIn::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CWebSessionSignIn";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::SetSettings( const CWebSessionSignInSettings& settings, CORE::CString::StringVector* problems )
{GUCEF_TRACE;

    CORE::CString::StringVector found;
    if ( !settings.Validate( found ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn: The settings of " << settings.storeId << " are refused:\n" << WEB::CWebLoginScript::DescribeProblems( found ) );
        if ( GUCEF_NULL != problems )
            problems->insert( problems->end(), found.begin(), found.end() );
        return false;
    }

    MT::CScopeMutex lock( m_dataLock );
    if ( m_running )
    {
        if ( GUCEF_NULL != problems )
            problems->push_back( "a sign in runs" );
        return false;
    }
    if ( m_settings.storeId != settings.storeId )
        m_session.Clear();
    m_settings = settings;
    m_defaultSessionEndedCheck.Configure( settings.script );
    return true;
}

/*-------------------------------------------------------------------------*/

CWebSessionSignInSettings
CWebSessionSignIn::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionSignIn::GetStoreId( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_settings.storeId;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::LoadLoginScript( const CORE::CString& path, CORE::CString::StringVector& problems )
{GUCEF_TRACE;

    WEB::CWebLoginScript script;
    if ( !script.LoadFromFile( path, problems ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn: The login script " << path << " cannot be used:\n" << WEB::CWebLoginScript::DescribeProblems( problems ) );
        return false;
    }
    CWebSessionSignInSettings settings = GetSettings();
    settings.script = script;
    return SetSettings( settings, &problems );
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetBrowser( CIWebSignInBrowser* browser )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_browser = browser;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetCredentialStore( AUTH::ICredentialStore* credentialStore )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_credentialStore = credentialStore;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetSessionStore( AUTH::ICredentialStore* sessionStore )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_sessionStore = sessionStore;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetSessionEndedCheck( const WEB::CIWebSessionEndedCheck* check )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_sessionEndedCheck = check;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetCredentials( const CWebSignInCredentials& credentials )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_credentials = credentials;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::ClearCredentials( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_credentials.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::SaveCredentials( const CWebSignInCredentials& credentials )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( GUCEF_NULL == m_credentialStore || m_settings.storeId.IsNULLOrEmpty() )
        return false;
    const bool saved = credentials.SaveTo( *m_credentialStore, m_settings.storeId );
    if ( saved )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn: Saved " << credentials.GetCount() << " credentials of " << m_settings.storeId );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn: The credentials of " << m_settings.storeId << " could not be saved" );
    }
    return saved;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::LoadCredentials( const CWebSessionSignInSettings& settings, CWebSignInCredentials& credentials ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    credentials = m_credentials;
    if ( GUCEF_NULL == m_credentialStore )
        return;

    // the slots not given for this object come from the saved ones
    CORE::CString::StringVector missing;
    for ( size_t i=0; i<settings.script.credentialSlots.size(); ++i )
        if ( !credentials.Has( settings.script.credentialSlots[ i ] ) )
            missing.push_back( settings.script.credentialSlots[ i ] );
    if ( missing.empty() )
        return;
    CWebSignInCredentials saved;
    saved.LoadFrom( *m_credentialStore, settings.storeId, missing );
    for ( size_t i=0; i<missing.size(); ++i )
    {
        AUTH::CSecretString value;
        if ( saved.Get( missing[ i ], value ) )
            credentials.Set( missing[ i ], value );
        value.Clear();
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::HasCredentials( void ) const
{GUCEF_TRACE;

    const CWebSessionSignInSettings settings = GetSettings();
    if ( settings.script.credentialSlots.empty() )
        return false;
    CWebSignInCredentials credentials;
    LoadCredentials( settings, credentials );
    return credentials.HasAll( settings.script.credentialSlots );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::ForgetCredentials( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_credentials.Clear();
    if ( GUCEF_NULL == m_credentialStore )
        return true;
    return CWebSignInCredentials::RemoveFrom( *m_credentialStore, m_settings.storeId, m_settings.script.credentialSlots );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::TryBeginRun( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( m_running )
        return false;
    m_running = true;
    m_cancelRequested.Store( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::EndRun( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_running = false;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::AsyncEnded( void )
{GUCEF_TRACE;

    --m_pendingAsync;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::IsSignInRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_running;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::Cancel( void )
{GUCEF_TRACE;

    m_cancelRequested.Store( 1 );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::WaitForAsyncSignIn( UInt32 timeoutInMs )
{GUCEF_TRACE;

    const UInt64 deadline = NowInMs() + timeoutInMs;
    while ( m_pendingAsync.Load() > 0 )
    {
        if ( NowInMs() >= deadline )
            return false;
        MT::ThreadDelay( 5 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::SetThreadPoolName( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_threadPoolName = threadPoolName.IsNULLOrEmpty() ? DefaultThreadPoolName : threadPoolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionSignIn::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::Notify( const CORE::CEvent& eventId, CORE::CICloneable& eventData )
{GUCEF_TRACE;

    // on the thread of the pulse generator directly, else through its mailbox
    CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() && pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentThreadID() )
        NotifyObservers( eventId, &eventData );
    else
        NotifyObserversFromThread( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

WEB::CWebSignInEventData
CWebSessionSignIn::MakeEventData( const CWebSessionSignInSettings& settings, WEB::TWebSignInMode mode ) const
{GUCEF_TRACE;

    WEB::CWebSignInEventData data;
    data.storeId = settings.storeId;
    data.storeName = settings.GetDisplayName();
    data.mode = mode;
    data.headless = WEB::WEBSIGNIN_MODE_UNATTENDED == mode || settings.browser.headless;
    return data;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::Progress( SRunContext& context, UInt32 stepNumber, const CORE::CString& message )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << context.settings.storeId << "): " << message );
    WEB::CWebSignInEventData data = MakeEventData( context.settings, context.mode );
    data.stepNumber = stepNumber;
    data.message = message;
    Notify( ProgressEvent, data );
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::SignIn( WEB::TWebSignInMode mode, CORE::CString* message )
{GUCEF_TRACE;

    if ( !TryBeginRun() )
    {
        if ( GUCEF_NULL != message )
            *message = "A sign in of this store runs already";
        return WEB::WEBSIGNIN_STATUS_ALREADY_EXISTS;
    }
    WEB::CWebSignInResult result;
    WEB::TWebSignInStatus status = WEB::WEBSIGNIN_STATUS_FAILED;
    try
    {
        status = RunSignIn( mode, false, result );
    }
    catch ( ... )
    {
        result.message = "The sign in threw an exception";
        status = WEB::WEBSIGNIN_STATUS_FAILED;
    }
    EndRun();
    if ( GUCEF_NULL != message )
        *message = result.message;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebSessionSignIn::SignInAsync( WEB::TWebSignInMode mode )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    if ( GUCEF_NULL == CAuthGlobal::Instance() )
    {
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "failed: the module is not initialised" );
        return source.GetFuture();
    }
    if ( !TryBeginRun() )
    {
        CWebSessionSignInTaskConsumer::FailSource( source, WEB::WEBSIGNIN_STATUS_ALREADY_EXISTS, "a sign in of this store runs already" );
        return source.GetFuture();
    }

    // the notice comes before the task is queued, so the application can say a window opens before it does
    const CWebSessionSignInSettings settings = GetSettings();
    WEB::CWebSignInEventData notice = MakeEventData( settings, mode );
    notice.message = notice.headless ? "Signing in to " + settings.GetDisplayName() + " without a window" :
                                       "A browser window opens to sign in to " + settings.GetDisplayName();
    Notify( SignInStartingEvent, notice );

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( GetThreadPoolName() );
    if ( threadPool.IsNULL() )
    {
        EndRun();
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "failed: no thread pool for the sign in" );
        return source.GetFuture();
    }

    CWebSessionSignInTaskData* data = GUCEF_NEW CWebSessionSignInTaskData();
    data->signIn = this;
    data->mode = mode;
    data->source = source;
    ++m_pendingAsync;
    CORE::CFutureResult queued = threadPool->QueueTask( CWebSessionSignInTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        --m_pendingAsync;
        EndRun();
        GUCEF_DELETE data;
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "failed: the sign in could not be queued" );
    }
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::RunSignIn( WEB::TWebSignInMode mode, bool startNoticeRaised, WEB::CWebSignInResult& result )
{GUCEF_TRACE;

    SRunContext context;
    context.mode = mode;
    CIWebSignInBrowser* injectedBrowser = GUCEF_NULL;
    bool clearBrowserCookies = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        context.settings = m_settings;
        injectedBrowser = m_browser;
        clearBrowserCookies = m_clearBrowserCookies;
    }
    result.storeId = context.settings.storeId;

    CORE::CString::StringVector problems;
    if ( !context.settings.Validate( problems ) )
    {
        result.status = WEB::WEBSIGNIN_STATUS_INVALID_ARGUMENT;
        result.message = "The sign in settings of " + context.settings.storeId + " cannot be used: " + ( problems.empty() ? CORE::CString::Empty : problems[ 0 ] );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn: " << result.message );
        Notify( SignInFailedEvent, result );
        return result.status;
    }

    LoadCredentials( context.settings, context.credentials );
    const WEB::CWebLoginScript& script = context.settings.script;
    if ( WEB::WEBSIGNIN_MODE_UNATTENDED == mode && script.NeedsCredentials() && !context.credentials.HasAll( script.credentialSlots ) )
    {
        context.credentials.Clear();
        result.status = WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN;
        result.message = "There are no saved credentials to sign in to " + context.settings.GetDisplayName() + " without the owner: sign in again";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn: " << result.message );
        Notify( SignInFailedEvent, result );
        return result.status;
    }

    WEB::CWebSignInBrowserSettings browserSettings = context.settings.browser;
    browserSettings.profileDirectory = context.settings.GetProfileDirectory();
    if ( WEB::WEBSIGNIN_MODE_UNATTENDED == mode )
    {
        browserSettings.headless = true;
        browserSettings.keepOpen = false;
    }
    context.settings.browser = browserSettings;

    if ( !startNoticeRaised )
    {
        WEB::CWebSignInEventData notice = MakeEventData( context.settings, mode );
        notice.message = notice.headless ? "Signing in to " + context.settings.GetDisplayName() + " without a window" :
                                           "A browser window opens to sign in to " + context.settings.GetDisplayName();
        Notify( SignInStartingEvent, notice );
    }

    CChromeSignInBrowser* ownBrowser = GUCEF_NULL;
    if ( GUCEF_NULL == injectedBrowser )
    {
        ownBrowser = GUCEF_NEW CChromeSignInBrowser();
        context.browser = ownBrowser;
    }
    else
        context.browser = injectedBrowser;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << context.settings.storeId << "): Starting a " << WEB::WebSignInModeToString( mode ) <<
        " sign in, " << ( context.credentials.HasAll( script.credentialSlots ) && script.NeedsCredentials() ? "with" : "without" ) << " credentials" );

    CORE::CString problem;
    WEB::TWebSignInStatus status = WEB::WEBSIGNIN_STATUS_OK;
    if ( !context.browser->Start( browserSettings, problem ) )
    {
        status = WEB::WEBSIGNIN_STATUS_BROWSER_UNAVAILABLE;
        context.message = problem;
    }
    else
    {
        const UInt64 timeout = context.settings.signInTimeoutInMs;
        context.deadline = NowInMs() + timeout;
        if ( clearBrowserCookies )
        {
            // a session that ended is not taken again from the cookies of the profile
            if ( context.browser->ClearCookies() )
            {
                MT::CScopeMutex lock( m_dataLock );
                m_clearBrowserCookies = false;
            }
        }
        status = RunScript( context );

        CWebSession session;
        if ( WEB::WEBSIGNIN_STATUS_OK == status && !CaptureSession( context, session ) )
        {
            status = WEB::WEBSIGNIN_STATUS_FAILED;
            context.message = "The store showed the signed in page but set no cookie of its domains";
        }

        if ( browserSettings.keepOpen && WEB::WEBSIGNIN_MODE_INTERACTIVE == mode )
            context.browser->Release();
        else
            context.browser->Stop();

        if ( WEB::WEBSIGNIN_STATUS_OK == status )
        {
            result.cookieCount = session.GetCookieCount();
            AUTH::ICredentialStore* sessionStore = GUCEF_NULL;
            {
                MT::CScopeMutex lock( m_dataLock );
                m_session = session;
                sessionStore = context.settings.saveSession ? m_sessionStore : GUCEF_NULL;
            }
            if ( GUCEF_NULL != sessionStore && !session.SaveTo( *sessionStore ) )
            {
                context.message = "Signed in, but the session could not be saved: the next start of the application signs in again";
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn(" << context.settings.storeId << "): The session could not be saved in the session store" );
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << context.settings.storeId << "): Signed in, " << session );
            session.Clear();
        }
    }
    context.credentials.Clear();
    if ( GUCEF_NULL != ownBrowser )
        GUCEF_DELETE ownBrowser;

    result.status = status;
    result.message = context.message;
    result.handedOver = context.handedOver;
    if ( WEB::WEBSIGNIN_STATUS_OK == status )
    {
        Notify( SignedInEvent, result );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSessionSignIn(" << context.settings.storeId << "): The sign in ended with " <<
            WEB::WebSignInStatusToString( status ) << ( result.message.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + result.message ) );
        Notify( SignInFailedEvent, result );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::CheckRunning( SRunContext& context )
{GUCEF_TRACE;

    if ( 0 != m_cancelRequested.Load() )
    {
        context.message = "The sign in was cancelled";
        return WEB::WEBSIGNIN_STATUS_CANCELLED;
    }
    if ( NowInMs() >= context.deadline )
    {
        context.message = "The sign in did not finish in time";
        return WEB::WEBSIGNIN_STATUS_TIMED_OUT;
    }
    if ( !context.browser->IsAlive() )
    {
        context.message = "The browser window was closed";
        return WEB::WEBSIGNIN_STATUS_CANCELLED;
    }
    return WEB::WEBSIGNIN_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionSignIn::Pause( SRunContext& context, UInt32 milliseconds )
{GUCEF_TRACE;

    const UInt64 until = NowInMs() + milliseconds;
    while ( 0 == m_cancelRequested.Load() && NowInMs() < until )
        MT::ThreadDelay( milliseconds < 20 ? milliseconds : 20 );
}

/*-------------------------------------------------------------------------*/

Int32
CWebSessionSignIn::CheckCondition( SRunContext& context, const WEB::CWebLoginScriptCondition& condition )
{GUCEF_TRACE;

    CIWebSignInBrowser& browser = *context.browser;
    if ( !condition.selector.IsNULLOrEmpty() )
    {
        const WEB::TWebElementState state = browser.GetElementState( condition.selector );
        if ( WEB::WEBELEMENT_STATE_NO_ANSWER == state && !browser.IsAlive() )
            return CONDITION_NO_BROWSER;
        if ( WEB::WEBELEMENT_STATE_VISIBLE != state )
            return CONDITION_DOES_NOT_HOLD;
    }
    if ( !condition.urlPattern.IsNULLOrEmpty() )
    {
        CORE::CString location;
        if ( !browser.GetLocation( location ) )
            return browser.IsAlive() ? CONDITION_DOES_NOT_HOLD : CONDITION_NO_BROWSER;
        if ( !WEB::CWebLoginScriptCondition::MatchesPattern( location, condition.urlPattern ) )
            return CONDITION_DOES_NOT_HOLD;
    }
    if ( !condition.text.IsNULLOrEmpty() )
    {
        bool contains = false;
        if ( !browser.ContainsText( condition.text, contains ) )
            return browser.IsAlive() ? CONDITION_DOES_NOT_HOLD : CONDITION_NO_BROWSER;
        if ( !contains )
            return CONDITION_DOES_NOT_HOLD;
    }
    if ( !condition.cookie.IsNULLOrEmpty() )
    {
        TWebSessionCookieVector cookies;
        if ( !browser.GetCookies( cookies ) )
            return browser.IsAlive() ? CONDITION_DOES_NOT_HOLD : CONDITION_NO_BROWSER;
        bool found = false;
        for ( size_t i=0; i<cookies.size(); ++i )
        {
            if ( !found && cookies[ i ].name == condition.cookie && !cookies[ i ].value.IsEmpty() && context.settings.script.IsStoreCookieDomain( cookies[ i ].domain ) )
                found = true;
            cookies[ i ].value.Clear();
        }
        if ( !found )
            return CONDITION_DOES_NOT_HOLD;
    }
    return CONDITION_HOLDS;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::IsSignedInNow( SRunContext& context, Int32& state )
{GUCEF_TRACE;

    state = CheckCondition( context, context.settings.script.signedIn );
    return CONDITION_HOLDS == state;
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::CheckWatchers( SRunContext& context )
{GUCEF_TRACE;

    for ( size_t i=0; i<context.watchers.size(); ++i )
    {
        const WEB::CWebLoginScriptStep& watcher = *context.watchers[ i ];
        const Int32 state = CheckCondition( context, watcher.condition );
        if ( CONDITION_NO_BROWSER == state )
        {
            context.message = "The browser window was closed";
            return WEB::WEBSIGNIN_STATUS_CANCELLED;
        }
        if ( CONDITION_HOLDS != state )
            continue;
        if ( WEB::CWebLoginScriptStep::ACTION_FAIL_CHECK == watcher.action )
        {
            context.message = watcher.reason;
            return WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN;
        }
        return HandOver( context, watcher.prompt, watcher.kind );
    }
    return WEB::WEBSIGNIN_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::HandOver( SRunContext& context, const CORE::CString& prompt, const CORE::CString& kind )
{GUCEF_TRACE;

    if ( WEB::WEBSIGNIN_MODE_UNATTENDED == context.mode )
    {
        context.message = "Only a person can go on: " + prompt;
        return WEB::WEBSIGNIN_STATUS_OWNER_ACTION_NEEDED;
    }

    context.handedOver = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << context.settings.storeId << "): Owner action needed (" << kind << "): " << prompt );
    WEB::CWebSignInEventData data = MakeEventData( context.settings, context.mode );
    data.message = prompt;
    data.personCheckKind = kind;
    Notify( OwnerActionNeededEvent, data );

    // the owner acts in the window; the sign in goes on when the signed in condition appears
    const UInt64 handOverEnd = NowInMs() + context.settings.handOverTimeoutInMs;
    if ( handOverEnd < context.deadline )
        context.deadline = handOverEnd;
    for ( ;; )
    {
        const WEB::TWebSignInStatus running = CheckRunning( context );
        if ( WEB::WEBSIGNIN_STATUS_OK != running )
        {
            if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == running )
                context.message = "The sign in was not finished in the browser window in time";
            return running;
        }
        Int32 state = CONDITION_DOES_NOT_HOLD;
        if ( IsSignedInNow( context, state ) )
        {
            context.signedIn = true;
            return WEB::WEBSIGNIN_STATUS_OK;
        }
        if ( CONDITION_NO_BROWSER == state )
        {
            context.message = "The browser window was closed";
            return WEB::WEBSIGNIN_STATUS_CANCELLED;
        }
        Pause( context, context.settings.pollIntervalInMs );
    }
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::Stuck( SRunContext& context, WEB::TWebSignInStatus unattendedStatus, const CORE::CString& reason )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << context.settings.storeId << "): " << reason );
    if ( WEB::WEBSIGNIN_MODE_UNATTENDED == context.mode )
    {
        context.message = reason;
        return unattendedStatus;
    }
    return HandOver( context, "Finish signing in to " + context.settings.GetDisplayName() + " in the browser window", "signIn" );
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::WaitForCondition( SRunContext& context, const WEB::CWebLoginScriptCondition& condition, UInt32 timeoutInMs, bool checkWatchers )
{GUCEF_TRACE;

    const UInt64 end = NowInMs() + timeoutInMs;
    for ( ;; )
    {
        const WEB::TWebSignInStatus running = CheckRunning( context );
        if ( WEB::WEBSIGNIN_STATUS_OK != running )
            return running;
        const Int32 state = CheckCondition( context, condition );
        if ( CONDITION_HOLDS == state )
            return WEB::WEBSIGNIN_STATUS_OK;
        if ( CONDITION_NO_BROWSER == state )
        {
            context.message = "The browser window was closed";
            return WEB::WEBSIGNIN_STATUS_CANCELLED;
        }
        if ( checkWatchers )
        {
            // a watcher that holds ends the wait: a failure, or a hand over that ends signed in
            const bool wasSignedIn = context.signedIn;
            const WEB::TWebSignInStatus watched = CheckWatchers( context );
            if ( WEB::WEBSIGNIN_STATUS_OK != watched || context.signedIn != wasSignedIn )
                return watched;
        }
        if ( NowInMs() >= end )
            return WEB::WEBSIGNIN_STATUS_TIMED_OUT;
        Pause( context, context.settings.pollIntervalInMs );
    }
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::WaitForSignedIn( SRunContext& context, UInt32 timeoutInMs )
{GUCEF_TRACE;

    const WEB::TWebSignInStatus status = WaitForCondition( context, context.settings.script.signedIn, timeoutInMs, true );
    if ( WEB::WEBSIGNIN_STATUS_OK == status )
        context.signedIn = true;
    return status;
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::RunStep( SRunContext& context, UInt32 stepIndex, bool& signedIn )
{GUCEF_TRACE;

    const WEB::CWebLoginScript& script = context.settings.script;
    const WEB::CWebLoginScriptStep& step = script.steps[ stepIndex ];
    const UInt32 stepNumber = stepIndex + 1;
    const UInt32 timeout = script.GetStepTimeout( step );
    signedIn = false;

    switch ( step.action )
    {
        case WEB::CWebLoginScriptStep::ACTION_OPEN:
        {
            if ( !script.IsUrlAllowed( step.url ) )
            {
                context.message = "The login script opens an address outside the domains of the store";
                return WEB::WEBSIGNIN_STATUS_INVALID_ARGUMENT;
            }
            Progress( context, stepNumber, "Opening " + WEB::CHttpLogRedaction::RedactUrl( step.url ) );
            if ( !context.browser->Navigate( step.url ) )
            {
                if ( !context.browser->IsAlive() )
                {
                    context.message = "The browser window was closed";
                    return WEB::WEBSIGNIN_STATUS_CANCELLED;
                }
                return Stuck( context, WEB::WEBSIGNIN_STATUS_NETWORK_ERROR, "The page " + WEB::CHttpLogRedaction::RedactUrl( step.url ) + " could not be opened" );
            }
            return WEB::WEBSIGNIN_STATUS_OK;
        }
        case WEB::CWebLoginScriptStep::ACTION_WAIT_FOR:
        {
            Progress( context, stepNumber, "Waiting for " + step.condition.Describe() );
            const bool wasSignedIn = context.signedIn;
            const WEB::TWebSignInStatus status = WaitForCondition( context, step.condition, timeout, true );
            signedIn = context.signedIn && !wasSignedIn;
            if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == status && NowInMs() < context.deadline )
            {
                if ( step.optional )
                    return WEB::WEBSIGNIN_STATUS_OK;
                return Stuck( context, WEB::WEBSIGNIN_STATUS_TIMED_OUT, "The login page did not show " + step.condition.Describe() + " in time" );
            }
            return status;
        }
        case WEB::CWebLoginScriptStep::ACTION_FILL:
        case WEB::CWebLoginScriptStep::ACTION_CLICK:
        case WEB::CWebLoginScriptStep::ACTION_SUBMIT:
        {
            // a profile that is still signed in needs no form
            Int32 state = CONDITION_DOES_NOT_HOLD;
            if ( IsSignedInNow( context, state ) )
            {
                signedIn = true;
                return WEB::WEBSIGNIN_STATUS_OK;
            }

            AUTH::CSecretString value;
            if ( WEB::CWebLoginScriptStep::ACTION_FILL == step.action && !context.credentials.Get( step.credential, value ) )
            {
                // without the credential the owner types it, from here on the sign in is the owner's
                if ( WEB::WEBSIGNIN_MODE_UNATTENDED == context.mode )
                {
                    context.message = "There is no saved value for '" + step.credential + "'";
                    return WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN;
                }
                return HandOver( context, "Sign in to " + context.settings.GetDisplayName() + " in the browser window", "signIn" );
            }

            WEB::CWebLoginScriptCondition target;
            target.selector = step.selector;
            Progress( context, stepNumber, WEB::CWebLoginScriptStep::ACTION_FILL == step.action ? "Filling in " + step.credential :
                                           ( WEB::CWebLoginScriptStep::ACTION_CLICK == step.action ? "Clicking " + step.selector : "Submitting " + step.selector ) );
            const bool wasSignedIn = context.signedIn;
            WEB::TWebSignInStatus status = WaitForCondition( context, target, timeout, true );
            if ( context.signedIn != wasSignedIn )
            {
                value.Clear();
                signedIn = true;
                return status;
            }
            if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == status && NowInMs() < context.deadline )
            {
                value.Clear();
                if ( step.optional )
                    return WEB::WEBSIGNIN_STATUS_OK;
                return Stuck( context, WEB::WEBSIGNIN_STATUS_FAILED, "The login page does not show " + step.selector );
            }
            if ( WEB::WEBSIGNIN_STATUS_OK != status )
            {
                value.Clear();
                return status;
            }

            bool done = false;
            if ( WEB::CWebLoginScriptStep::ACTION_FILL == step.action )
            {
                // a credential is only ever typed on a page of the store
                CORE::CString location;
                const CORE::CString host = context.browser->GetLocation( location ) ? WEB::CWebLoginScript::HostOfUrl( location ) : CORE::CString::Empty;
                if ( host.IsNULLOrEmpty() || !script.IsHostAllowed( host ) )
                {
                    value.Clear();
                    context.message = "A credential is not typed on a page of " + ( host.IsNULLOrEmpty() ? CORE::CString( "an unknown host" ) : host ) +
                                      ", which is not a domain of the store";
                    return WEB::WEBSIGNIN_STATUS_FAILED;
                }
                done = context.browser->FillField( step.selector, value );
            }
            else
            if ( WEB::CWebLoginScriptStep::ACTION_CLICK == step.action )
                done = context.browser->Click( step.selector );
            else
                done = context.browser->Submit( step.selector );
            value.Clear();

            if ( !done )
            {
                if ( !context.browser->IsAlive() )
                {
                    context.message = "The browser window was closed";
                    return WEB::WEBSIGNIN_STATUS_CANCELLED;
                }
                return Stuck( context, WEB::WEBSIGNIN_STATUS_FAILED, "The login page did not take step " + CORE::ToString( stepNumber ) + " (" +
                              WEB::CWebLoginScriptStep::ActionToString( step.action ) + " " + step.selector + ")" );
            }
            return WEB::WEBSIGNIN_STATUS_OK;
        }
        case WEB::CWebLoginScriptStep::ACTION_PERSON_CHECK:
        case WEB::CWebLoginScriptStep::ACTION_FAIL_CHECK:
        {
            context.watchers.push_back( &step );
            const bool wasSignedIn = context.signedIn;
            const WEB::TWebSignInStatus status = CheckWatchers( context );
            signedIn = context.signedIn && !wasSignedIn;
            return status;
        }
        case WEB::CWebLoginScriptStep::ACTION_DELAY:
        {
            Pause( context, step.delayInMs );
            return CheckRunning( context );
        }
        case WEB::CWebLoginScriptStep::ACTION_WAIT_FOR_SIGNED_IN:
        {
            Progress( context, stepNumber, "Waiting for the store to confirm the sign in" );
            const WEB::TWebSignInStatus status = WaitForSignedIn( context, timeout );
            signedIn = context.signedIn;
            if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == status && NowInMs() < context.deadline )
                return Stuck( context, WEB::WEBSIGNIN_STATUS_TIMED_OUT, "The store did not confirm the sign in in time" );
            return status;
        }
        default:
        {
            context.message = "The login script has a step that is not an action";
            return WEB::WEBSIGNIN_STATUS_INVALID_ARGUMENT;
        }
    }
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::RunScript( SRunContext& context )
{GUCEF_TRACE;

    const WEB::CWebLoginScript& script = context.settings.script;

    // every sign in begins on the start page
    Progress( context, 0, "Opening the login page of " + context.settings.GetDisplayName() );
    if ( !context.browser->Navigate( script.startUrl ) )
    {
        if ( !context.browser->IsAlive() )
        {
            context.message = "The browser window was closed";
            return WEB::WEBSIGNIN_STATUS_CANCELLED;
        }
        WEB::TWebSignInStatus status = Stuck( context, WEB::WEBSIGNIN_STATUS_NETWORK_ERROR, "The login page " + WEB::CHttpLogRedaction::RedactUrl( script.startUrl ) + " could not be opened" );
        return status;
    }

    for ( UInt32 i=0; i<(UInt32) script.steps.size() && !context.signedIn; ++i )
    {
        const WEB::TWebSignInStatus running = CheckRunning( context );
        if ( WEB::WEBSIGNIN_STATUS_OK != running )
            return running;
        bool signedIn = false;
        const WEB::TWebSignInStatus status = RunStep( context, i, signedIn );
        if ( WEB::WEBSIGNIN_STATUS_OK != status )
            return status;
        if ( signedIn || context.signedIn )
        {
            context.signedIn = true;
            break;
        }
    }
    if ( context.signedIn )
        return WEB::WEBSIGNIN_STATUS_OK;

    // a script without steps is a manual sign in; after the last step the store confirms it
    if ( script.steps.empty() || !script.NeedsCredentials() )
    {
        Int32 state = CONDITION_DOES_NOT_HOLD;
        if ( IsSignedInNow( context, state ) )
            return WEB::WEBSIGNIN_STATUS_OK;
        if ( WEB::WEBSIGNIN_MODE_UNATTENDED == context.mode )
        {
            const WEB::TWebSignInStatus status = WaitForSignedIn( context, script.GetStepTimeout( WEB::CWebLoginScriptStep() ) );
            if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == status )
                context.message = "The store did not confirm the sign in without the owner";
            return WEB::WEBSIGNIN_STATUS_TIMED_OUT == status ? WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN : status;
        }
        return HandOver( context, "Sign in to " + context.settings.GetDisplayName() + " in the browser window", "signIn" );
    }
    Progress( context, 0, "Waiting for the store to confirm the sign in" );
    const WEB::TWebSignInStatus status = WaitForSignedIn( context, script.GetStepTimeout( WEB::CWebLoginScriptStep() ) );
    if ( WEB::WEBSIGNIN_STATUS_TIMED_OUT == status && NowInMs() < context.deadline )
        return Stuck( context, WEB::WEBSIGNIN_STATUS_TIMED_OUT, "The store did not confirm the sign in in time" );
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::CaptureSession( SRunContext& context, CWebSession& session )
{GUCEF_TRACE;

    session.Clear();
    session.storeId = context.settings.storeId;
    session.capturedAtUtcInMs = CWebSession::NowUtcInMs();
    TWebSessionCookieVector cookies;
    if ( !context.browser->GetCookies( cookies ) )
        return false;
    for ( size_t i=0; i<cookies.size(); ++i )
    {
        if ( !cookies[ i ].value.IsEmpty() && context.settings.script.IsStoreCookieDomain( cookies[ i ].domain ) )
            session.cookies.push_back( cookies[ i ] );
        cookies[ i ].value.Clear();
    }
    return !session.IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::HasSession( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return !m_session.IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::GetSession( CWebSession& session ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    session = m_session;
    return !session.IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::LoadSavedSession( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( GUCEF_NULL == m_sessionStore || m_settings.storeId.IsNULLOrEmpty() )
        return false;
    CWebSession session;
    if ( !session.LoadFrom( *m_sessionStore, m_settings.storeId ) )
        return false;
    m_session = session;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << m_settings.storeId << "): Using the saved " << session );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSessionSignIn::ApplySessionTo( WEB::CHttpCookieJar& jar ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_session.ApplyTo( jar );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::ForgetSession( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_session.Clear();
    m_clearBrowserCookies = true;
    if ( GUCEF_NULL == m_sessionStore || m_settings.storeId.IsNULLOrEmpty() )
        return true;
    return CWebSession::RemoveFrom( *m_sessionStore, m_settings.storeId );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionSignIn::CheckResponse( const WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    CORE::CString storeId;
    {
        MT::CScopeMutex lock( m_dataLock );
        const WEB::CIWebSessionEndedCheck* check = GUCEF_NULL != m_sessionEndedCheck ? m_sessionEndedCheck : &m_defaultSessionEndedCheck;
        if ( !check->IsSessionEnded( response ) )
            return false;
        storeId = m_settings.storeId;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSessionSignIn(" << storeId << "): The store answered " << response.statusCode << ": the session ended" );
    ForgetSession();
    CORE::TCloneableString eventData( storeId );
    Notify( SessionEndedEvent, eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

WEB::TWebSignInStatus
CWebSessionSignIn::RenewSession( CORE::CString* message )
{GUCEF_TRACE;

    const UInt64 askedAtUtcInMs = CWebSession::NowUtcInMs();
    const CWebSessionSignInSettings settings = GetSettings();

    // several requests can find the session ended at once: one signs in, the others wait for it and use its session
    for ( int attempt=0; attempt<2; ++attempt )
    {
        const UInt64 waitUntil = NowInMs() + settings.signInTimeoutInMs;
        while ( IsSignInRunning() && NowInMs() < waitUntil )
            MT::ThreadDelay( 20 );
        {
            MT::CScopeMutex lock( m_dataLock );
            if ( !m_session.IsEmpty() && m_session.capturedAtUtcInMs >= askedAtUtcInMs )
                return WEB::WEBSIGNIN_STATUS_OK;
        }
        if ( attempt > 0 )
            break;

        // the store refused the session: it is not used again, also not from the cookies of the browser profile
        ForgetSession();
        if ( !settings.allowUnattendedRenewal || !HasCredentials() )
            break;
        const WEB::TWebSignInStatus status = SignIn( WEB::WEBSIGNIN_MODE_UNATTENDED, message );
        if ( WEB::WEBSIGNIN_STATUS_ALREADY_EXISTS != status )
            return status;
    }
    if ( GUCEF_NULL != message )
        *message = "The session of " + settings.GetDisplayName() + " ended: sign in again";
    return WEB::WEBSIGNIN_STATUS_NOT_SIGNED_IN;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
