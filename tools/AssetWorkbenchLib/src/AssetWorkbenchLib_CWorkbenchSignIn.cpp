/*
 *  AssetWorkbenchLib: the sign in of the workbench: the credential stores, the keyring code and the routing of the sign in of an online store
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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#include "gucefAUTH_CAuthEventData.h"
#define GUCEF_AUTH_CAUTHEVENTDATA_H
#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#include "AssetWorkbenchLib_CWorkbenchSignIn.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchSignIn::CWorkbenchSignIn( void )
    : CORE::CObserver()
    , m_context( GUCEF_NULL )
    , m_report( GUCEF_NULL )
    , m_credentials( GUCEF_NULL )
    , m_authManager( GUCEF_NULL )
    , m_flowEngine( GUCEF_NULL )
    , m_unlock( GUCEF_NULL )
    , m_codeEntry( GUCEF_NULL )
    , m_settings( GUCEF_NULL )
    , m_question( GUCEF_NULL )
    , m_notice( GUCEF_NULL )
    , m_questionKind( QUESTION_NONE )
    , m_questionStore()
    , m_keyringAction( KEYRINGACTION_NONE )
    , m_keyringStore()
    , m_settingsStore()
    , m_automaticStore()
    , m_oauthStore()
    , m_oauthProvider()
    , m_endedStores()
    , m_pending()
    , m_pendingStore()
    , m_watchedSignIns()
    , m_watchedProviders()
    , m_initialized( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchSignIn::~CWorkbenchSignIn()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::Initialize( GUI::CGUIContext& context, GUI::CReportViewForm* report, CWorkbenchCredentials* credentials )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    m_context = &context;
    m_report = report;
    m_credentials = credentials;

    m_unlock = GUCEF_NEW AUTHGUI::CKeyringUnlockForm( m_context );
    m_codeEntry = GUCEF_NEW AUTHGUI::COAuthCodeEntryForm( m_context );
    m_settings = GUCEF_NEW AUTHGUI::CSignInSettingsForm( m_context );
    m_question = GUCEF_NEW GUI::CMessageBoxForm( m_context );
    m_notice = GUCEF_NEW GUI::CMessageBoxForm( m_context );
    if ( !m_unlock->Build() || !m_codeEntry->Build() || !m_settings->Build() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSignIn: The forms of the sign in cannot be made" );
        Shutdown();
        return false;
    }

    if ( GUCEF_NULL != m_credentials )
    {
        AUTH::CKeyringSession& session = m_credentials->GetKeyringSession();
        m_unlock->SetSession( &session );
        TEventCallback changedCallback( this, &CWorkbenchSignIn::OnKeyringChanged );
        SubscribeTo( &session, AUTH::CKeyringSession::UnlockedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::LockedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeSetUpEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeChangedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::ForgottenEvent, changedCallback );
    }
    TEventCallback unlockDoneCallback( this, &CWorkbenchSignIn::OnUnlockDone );
    SubscribeTo( m_unlock, AUTHGUI::CKeyringUnlockForm::DoneEvent, unlockDoneCallback );
    TEventCallback unlockCancelledCallback( this, &CWorkbenchSignIn::OnUnlockCancelled );
    SubscribeTo( m_unlock, AUTHGUI::CKeyringUnlockForm::CancelledEvent, unlockCancelledCallback );

    TEventCallback codeDoneCallback( this, &CWorkbenchSignIn::OnCodeEntryDone );
    SubscribeTo( m_codeEntry, AUTHGUI::COAuthCodeEntryForm::DoneEvent, codeDoneCallback );
    TEventCallback codeCancelledCallback( this, &CWorkbenchSignIn::OnCodeEntryCancelled );
    SubscribeTo( m_codeEntry, AUTHGUI::COAuthCodeEntryForm::CancelledEvent, codeCancelledCallback );

    TEventCallback signInCallback( this, &CWorkbenchSignIn::OnSettingsSignIn );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SignInRequestedEvent, signInCallback );
    TEventCallback signOutCallback( this, &CWorkbenchSignIn::OnSettingsSignOut );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SignOutRequestedEvent, signOutCallback );
    TEventCallback saveCallback( this, &CWorkbenchSignIn::OnSettingsSave );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SaveCredentialsRequestedEvent, saveCallback );
    TEventCallback forgetCallback( this, &CWorkbenchSignIn::OnSettingsForget );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ForgetCredentialsRequestedEvent, forgetCallback );
    TEventCallback setUpCallback( this, &CWorkbenchSignIn::OnSettingsSetUpCode );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SetUpCodeRequestedEvent, setUpCallback );
    TEventCallback changeCallback( this, &CWorkbenchSignIn::OnSettingsChangeCode );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ChangeCodeRequestedEvent, changeCallback );
    TEventCallback lockCallback( this, &CWorkbenchSignIn::OnSettingsLockNow );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::LockNowRequestedEvent, lockCallback );
    TEventCallback closedCallback( this, &CWorkbenchSignIn::OnSettingsClosed );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ClosedEvent, closedCallback );

    TEventCallback questionCallback( this, &CWorkbenchSignIn::OnQuestionClosed );
    SubscribeTo( m_question, GUI::CMessageBoxForm::ClosedEvent, questionCallback );

    m_initialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::Shutdown( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    WipePending();
    if ( GUCEF_NULL != m_unlock )
        m_unlock->SetSession( GUCEF_NULL );
    if ( GUCEF_NULL != m_codeEntry )
        m_codeEntry->SetEngine( GUCEF_NULL );
    GUCEF_DELETE m_unlock;      m_unlock = GUCEF_NULL;
    GUCEF_DELETE m_codeEntry;   m_codeEntry = GUCEF_NULL;
    GUCEF_DELETE m_settings;    m_settings = GUCEF_NULL;
    GUCEF_DELETE m_question;    m_question = GUCEF_NULL;
    GUCEF_DELETE m_notice;      m_notice = GUCEF_NULL;
    m_watchedSignIns.clear();
    m_watchedProviders.clear();
    m_authManager = GUCEF_NULL;
    m_credentials = GUCEF_NULL;
    m_report = GUCEF_NULL;
    m_context = GUCEF_NULL;
    m_questionKind = QUESTION_NONE;
    m_keyringAction = KEYRINGACTION_NONE;
    m_settingsStore.Clear();
    m_automaticStore.Clear();
    m_oauthStore.Clear();
    m_initialized = false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    if ( manager == m_authManager )
        return;
    if ( GUCEF_NULL != m_authManager )
        UnsubscribeFrom( *m_authManager );
    m_authManager = manager;
    if ( GUCEF_NULL == m_authManager )
        return;
    TEventCallback actionCallback( this, &CWorkbenchSignIn::OnAuthActionRequired );
    SubscribeTo( m_authManager, AUTH::CAuthManager::ActionRequiredEvent, actionCallback );
    TEventCallback succeededCallback( this, &CWorkbenchSignIn::OnAuthLoginSucceeded );
    SubscribeTo( m_authManager, AUTH::CAuthManager::LoginSucceededEvent, succeededCallback );
    TEventCallback failedCallback( this, &CWorkbenchSignIn::OnAuthLoginFailed );
    SubscribeTo( m_authManager, AUTH::CAuthManager::LoginFailedEvent, failedCallback );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::SetFlowEngine( AUTH::IAuthFlowEngine* engine )
{GUCEF_TRACE;

    m_flowEngine = engine;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::FindProvider( const CORE::CString& storeId, ASSETSTORE::TAssetStoreProviderPtr& provider ) const
{GUCEF_TRACE;

    return ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId, provider ) && !provider.IsNULL();
}

/*-------------------------------------------------------------------------*/

AUTH::CWebSessionSignIn*
CWorkbenchSignIn::GetWebSignIn( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( FindProvider( storeId, provider ) && ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION == provider->GetSignInKind() )
        return provider->GetWebSessionSignIn();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSignIn::GetStoreName( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    return FindProvider( storeId, provider ) ? provider->GetDisplayName() : storeId;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::Watch( ASSETSTORE::CIAssetStoreProvider* provider, AUTH::CWebSessionSignIn* signIn )
{GUCEF_TRACE;

    // every notifier is subscribed to once, whatever the number of sign ins that were asked for
    if ( GUCEF_NULL != provider )
    {
        bool known = false;
        for ( size_t i=0; i<m_watchedProviders.size() && !known; ++i )
            known = m_watchedProviders[ i ] == provider;
        if ( !known )
        {
            m_watchedProviders.push_back( provider );
            TEventCallback callback( this, &CWorkbenchSignIn::OnProviderSessionStateChanged );
            SubscribeTo( provider, ASSETSTORE::CIAssetStoreProvider::SessionStateChangedEvent, callback );
        }
    }
    if ( GUCEF_NULL != signIn )
    {
        bool known = false;
        for ( size_t i=0; i<m_watchedSignIns.size() && !known; ++i )
            known = m_watchedSignIns[ i ] == signIn;
        if ( !known )
        {
            m_watchedSignIns.push_back( signIn );
            TEventCallback startingCallback( this, &CWorkbenchSignIn::OnWebSignInStarting );
            SubscribeTo( signIn, AUTH::CWebSessionSignIn::SignInStartingEvent, startingCallback );
            TEventCallback ownerCallback( this, &CWorkbenchSignIn::OnWebOwnerActionNeeded );
            SubscribeTo( signIn, AUTH::CWebSessionSignIn::OwnerActionNeededEvent, ownerCallback );
            TEventCallback signedInCallback( this, &CWorkbenchSignIn::OnWebSignedIn );
            SubscribeTo( signIn, AUTH::CWebSessionSignIn::SignedInEvent, signedInCallback );
            TEventCallback failedCallback( this, &CWorkbenchSignIn::OnWebSignInFailed );
            SubscribeTo( signIn, AUTH::CWebSessionSignIn::SignInFailedEvent, failedCallback );
            TEventCallback endedCallback( this, &CWorkbenchSignIn::OnWebSessionEnded );
            SubscribeTo( signIn, AUTH::CWebSessionSignIn::SessionEndedEvent, endedCallback );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::Report( GUI::CReportViewForm::ESeverity severity, const CORE::CString& storeName, const CORE::CString& text, const CORE::CString& details )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_report )
        m_report->AddEntry( severity, storeName, text, details );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::ShowNotice( const CORE::CString& title, const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_notice )
        m_notice->Open( title, text, GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_OK );
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CSignInSettingsState::EKeyringState
CWorkbenchSignIn::GetKeyringState( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_credentials || !m_credentials->IsKeyringAvailable() )
        return AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE;
    const AUTH::CKeyringSession& session = m_credentials->GetKeyringSession();
    if ( !session.IsSetUp() )
        return AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP;
    return session.IsLocked() ? AUTHGUI::CSignInSettingsState::KEYRING_LOCKED : AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED;
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CSignInSettingsState::EKeyringState
CWorkbenchSignIn::GetKeyringStatus( void ) const
{GUCEF_TRACE;

    return GetKeyringState();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::OpenKeyring( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_unlock )
        return false;
    const AUTHGUI::CSignInSettingsState::EKeyringState state = GetKeyringState();
    if ( AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP != state && AUTHGUI::CSignInSettingsState::KEYRING_LOCKED != state )
        return false;
    m_keyringAction = KEYRINGACTION_NONE;
    return m_unlock->Open( AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP == state ? AUTHGUI::CKeyringUnlockForm::MODE_SET_UP : AUTHGUI::CKeyringUnlockForm::MODE_UNLOCK );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::OpenKeyringChangeCode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_unlock || AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED != GetKeyringState() )
        return false;
    m_keyringAction = KEYRINGACTION_NONE;
    return m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_CHANGE_CODE );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::LockKeyring( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_credentials && AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == GetKeyringState() )
        m_credentials->GetKeyringSession().LockNow();
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CSignInSettingsState
CWorkbenchSignIn::MakeSettingsState( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    AUTHGUI::CSignInSettingsState state;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return state;
    state.serviceName = provider->GetDisplayName();
    state.signedIn = provider->IsSignedIn();
    state.keyringState = GetKeyringState();
    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL != signIn )
    {
        state.signInRunning = signIn->IsSignInRunning();
        state.credentialSlots = signIn->GetSettings().script.credentialSlots;
        state.hasSavedCredentials = AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == state.keyringState && !state.credentialSlots.empty() && signIn->HasCredentials();
    }
    else
    {
        state.signInRunning = !m_oauthStore.IsNULLOrEmpty() && m_oauthStore == storeId;
    }
    for ( size_t i=0; i<m_endedStores.size(); ++i )
        if ( m_endedStores[ i ] == storeId && !state.signedIn )
            state.statusNote = "The session ended.";
    return state;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::RefreshSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_settings && !m_settingsStore.IsNULLOrEmpty() )
        m_settings->SetState( MakeSettingsState( m_settingsStore ) );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::SignIn( const CORE::CString& storeId )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSignIn: The store " << storeId << " is not known: it cannot be signed in to" );
        return false;
    }
    switch ( provider->GetSignInKind() )
    {
        case ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION: return BeginWebSignIn( storeId, false );
        case ASSETSTORE::ASSETSTORE_SIGNIN_AUTH_PROVIDER: return BeginOAuthSignIn( storeId );
        default:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSignIn: The store " << storeId << " has no sign in" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::OnSessionExpired( const CORE::CString& storeId )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return false;
    if ( ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION == provider->GetSignInKind() )
        return BeginWebSignIn( storeId, true );
    Report( GUI::CReportViewForm::SEVERITY_WARNING, provider->GetDisplayName(), "The session ended: sign in again" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::BeginWebSignIn( const CORE::CString& storeId, bool sessionEnded )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return false;
    AUTH::CWebSessionSignIn* signIn = provider->GetWebSessionSignIn();
    if ( GUCEF_NULL == signIn )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CWorkbenchSignIn: The sign in to " << storeId << " is not available in this workbench" );
        return false;
    }
    Watch( provider.GetPointerAlways(), signIn );
    const CORE::CString name = provider->GetDisplayName();
    if ( signIn->IsSignInRunning() )
    {
        Report( GUI::CReportViewForm::SEVERITY_INFO, name, "A sign in runs already: finish it in the browser window" );
        return false;
    }
    if ( m_unlock->IsOpen() || m_question->IsOpen() )
    {
        Report( GUI::CReportViewForm::SEVERITY_INFO, name, "A step of a sign in waits for an answer: answer it first" );
        return false;
    }
    if ( sessionEnded )
        Report( GUI::CReportViewForm::SEVERITY_WARNING, name, "The session ended: signing in again" );

    // saved credentials make an automatic sign in; they are behind the keyring code, which is asked for once
    if ( GUCEF_NULL != m_credentials && !signIn->GetSettings().script.credentialSlots.empty() && m_credentials->IsKeyringAvailable() )
    {
        AUTH::CKeyringSession& session = m_credentials->GetKeyringSession();
        if ( session.IsSetUp() && session.IsLocked() )
        {
            AskKeyring( KEYRINGACTION_AUTOMATIC_SIGNIN, storeId );
            return true;
        }
        if ( signIn->HasCredentials() )
            return StartAutomatic( storeId );
    }
    return AskOpenWindow( storeId );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::StartAutomatic( const CORE::CString& storeId )
{GUCEF_TRACE;

    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL == signIn )
        return false;
    Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( storeId ), "Signing in with the saved credentials, without a window" );
    m_automaticStore = storeId;
    signIn->SignInAsync( WEB::WEBSIGNIN_MODE_UNATTENDED );
    RefreshSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::AskOpenWindow( const CORE::CString& storeId )
{GUCEF_TRACE;

    const CORE::CString name = GetStoreName( storeId );
    m_questionKind = QUESTION_OPEN_WINDOW;
    m_questionStore = storeId;
    // the notice comes before the window: the owner decides, then the browser opens
    return m_question->Open( "Sign in to " + name,
                             "A browser window opens so that you can sign in to " + name + ". Sign in there: the password and any code go to the page of the store, not to this program.",
                             GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_OK_CANCEL );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::StartWithWindow( const CORE::CString& storeId )
{GUCEF_TRACE;

    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL == signIn )
        return false;
    if ( signIn->IsSignInRunning() )
        return false;
    m_automaticStore.Clear();
    signIn->SignInAsync( WEB::WEBSIGNIN_MODE_INTERACTIVE );
    RefreshSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnQuestionClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableInt32* result = static_cast< const CORE::TCloneableInt32* >( eventData );
    const GUI::CMessageBoxForm::EMessageBoxResult button = GUCEF_NULL != result ? (GUI::CMessageBoxForm::EMessageBoxResult) result->GetData() : GUI::CMessageBoxForm::MESSAGEBOXRESULT_CANCEL;
    const EQuestion kind = m_questionKind;
    const CORE::CString storeId = m_questionStore;
    m_questionKind = QUESTION_NONE;
    m_questionStore.Clear();
    const bool yes = GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK == button || GUI::CMessageBoxForm::MESSAGEBOXRESULT_YES == button;
    if ( QUESTION_OPEN_WINDOW == kind || QUESTION_OFFER_WINDOW == kind )
    {
        if ( yes )
            StartWithWindow( storeId );
        else
            Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( storeId ), "The sign in was not started" );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::AskKeyring( EKeyringAction action, const CORE::CString& storeId )
{GUCEF_TRACE;

    m_keyringAction = action;
    m_keyringStore = storeId;
    if ( !m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_UNLOCK ) )
    {
        // no form to ask with: what waited for the code is given up
        const EKeyringAction given = m_keyringAction;
        m_keyringAction = KEYRINGACTION_NONE;
        AbandonAfterKeyring( given, storeId );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::ContinueAfterKeyring( EKeyringAction action, const CORE::CString& storeId )
{GUCEF_TRACE;

    switch ( action )
    {
        case KEYRINGACTION_AUTOMATIC_SIGNIN:
        {
            AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
            if ( GUCEF_NULL != signIn && signIn->HasCredentials() )
                StartAutomatic( storeId );
            else
                AskOpenWindow( storeId );
            break;
        }
        case KEYRINGACTION_SAVE_CREDENTIALS: DoSaveCredentials(); break;
        case KEYRINGACTION_FORGET_CREDENTIALS: DoForgetCredentials(); break;
        default: break;
    }
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::AbandonAfterKeyring( EKeyringAction action, const CORE::CString& storeId )
{GUCEF_TRACE;

    switch ( action )
    {
        case KEYRINGACTION_AUTOMATIC_SIGNIN:
        {
            // the saved credentials are not used: the owner can sign in with the window
            AskOpenWindow( storeId );
            break;
        }
        case KEYRINGACTION_SAVE_CREDENTIALS:
        {
            WipePending();
            if ( GUCEF_NULL != m_settings )
                m_settings->ShowMessage( "Not saved: the keyring code was not given.", true );
            break;
        }
        case KEYRINGACTION_FORGET_CREDENTIALS:
        {
            if ( GUCEF_NULL != m_settings )
                m_settings->ShowMessage( "Nothing was forgotten: the keyring code was not given.", true );
            break;
        }
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnUnlockDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const EKeyringAction action = m_keyringAction;
    const CORE::CString storeId = m_keyringStore;
    m_keyringAction = KEYRINGACTION_NONE;
    m_keyringStore.Clear();
    ContinueAfterKeyring( action, storeId );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnUnlockCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const EKeyringAction action = m_keyringAction;
    const CORE::CString storeId = m_keyringStore;
    m_keyringAction = KEYRINGACTION_NONE;
    m_keyringStore.Clear();
    AbandonAfterKeyring( action, storeId );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnKeyringChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnWebSignInStarting( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::CWebSignInEventData* data = static_cast< const WEB::CWebSignInEventData* >( eventData );
    if ( GUCEF_NULL != data )
        Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( data->storeId ), data->message );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnWebOwnerActionNeeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::CWebSignInEventData* data = static_cast< const WEB::CWebSignInEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const CORE::CString name = GetStoreName( data->storeId );
    Report( GUI::CReportViewForm::SEVERITY_WARNING, name, "Owner action needed: " + data->message );
    // what to do in the browser window, where the owner can see it
    ShowNotice( "Sign in to " + name, data->message );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnWebSignedIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::CWebSignInResult* result = static_cast< const WEB::CWebSignInResult* >( eventData );
    if ( GUCEF_NULL == result )
        return;
    if ( m_automaticStore == result->storeId )
        m_automaticStore.Clear();
    for ( size_t i=0; i<m_endedStores.size(); ++i )
    {
        if ( m_endedStores[ i ] == result->storeId )
        {
            m_endedStores.erase( m_endedStores.begin() + i );
            break;
        }
    }
    // the prompt of a step of a person is done with
    if ( GUCEF_NULL != m_notice && m_notice->IsOpen() )
        m_notice->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
    Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( result->storeId ), "Signed in", result->message );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnWebSignInFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const WEB::CWebSignInResult* result = static_cast< const WEB::CWebSignInResult* >( eventData );
    if ( GUCEF_NULL == result )
        return;
    const bool automatic = m_automaticStore == result->storeId;
    if ( automatic )
        m_automaticStore.Clear();
    const CORE::CString name = GetStoreName( result->storeId );
    Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in did not succeed: " + CORE::CString( WEB::WebSignInStatusToString( result->status ) ), result->message );
    RefreshSettings();
    if ( WEB::WEBSIGNIN_STATUS_CANCELLED == result->status )
        return;
    if ( GUCEF_NULL != m_notice && m_notice->IsOpen() )
        m_notice->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK );
    if ( automatic && GUCEF_NULL != m_question && !m_question->IsOpen() )
    {
        // a person has to act, or the page did not do what the script expected: the owner can do it in a window
        m_questionKind = QUESTION_OFFER_WINDOW;
        m_questionStore = result->storeId;
        m_question->Open( "Sign in to " + name, "The sign in to " + name + " could not finish by itself: " + result->message + ". Sign in with a browser window?", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO );
        return;
    }
    ShowNotice( "Sign in to " + name, "The sign in did not succeed: " + result->message );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnWebSessionEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* storeId = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL == storeId )
        return;
    bool known = false;
    for ( size_t i=0; i<m_endedStores.size() && !known; ++i )
        known = m_endedStores[ i ] == storeId->GetData();
    if ( !known )
        m_endedStores.push_back( storeId->GetData() );
    Report( GUI::CReportViewForm::SEVERITY_WARNING, GetStoreName( storeId->GetData() ), "The session ended" );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnProviderSessionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::SignOut( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !m_initialized || !FindProvider( storeId, provider ) )
        return false;
    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL != signIn && signIn->IsSignInRunning() )
        signIn->Cancel();
    const bool done = provider->SignOut();
    for ( size_t i=0; i<m_endedStores.size(); ++i )
    {
        if ( m_endedStores[ i ] == storeId )
        {
            m_endedStores.erase( m_endedStores.begin() + i );
            break;
        }
    }
    Report( GUI::CReportViewForm::SEVERITY_INFO, provider->GetDisplayName(), done ? "Signed out" : "The store has no session to sign out of" );
    RefreshSettings();
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::IsSignInRunning( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL != signIn )
        return signIn->IsSignInRunning();
    return !m_oauthStore.IsNULLOrEmpty() && m_oauthStore == storeId;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::OpenSettings( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !m_initialized || !FindProvider( storeId, provider ) || ASSETSTORE::ASSETSTORE_SIGNIN_NONE == provider->GetSignInKind() )
        return false;
    Watch( provider.GetPointerAlways(), provider->GetWebSessionSignIn() );
    m_settingsStore = storeId;
    m_settings->SetState( MakeSettingsState( storeId ) );
    return m_settings->Open();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsSignIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_settingsStore.IsNULLOrEmpty() )
        SignIn( m_settingsStore );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsSignOut( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_settingsStore.IsNULLOrEmpty() )
        SignOut( m_settingsStore );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::WipePending( void )
{GUCEF_TRACE;

    m_pending.Clear();
    m_pendingStore.Clear();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::StartSaveCredentials( void )
{GUCEF_TRACE;

    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( m_settingsStore );
    if ( GUCEF_NULL == signIn )
    {
        m_settings->ShowMessage( "This store has no credentials to save.", true );
        return;
    }
    // what was typed is taken out of the fields now (the form wipes them); it waits in memory for the keyring code when that is needed
    WipePending();
    m_pendingStore = m_settingsStore;
    const CORE::CString::StringVector& slots = m_settings->GetState().credentialSlots;
    for ( size_t i=0; i<slots.size(); ++i )
    {
        AUTH::CSecretString value;
        if ( m_settings->GetCredential( slots[ i ], value ) )
            m_pending.Set( slots[ i ], value );
        value.Clear();
    }
    switch ( GetKeyringState() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE:
        {
            WipePending();
            m_settings->ShowMessage( "Saved credentials are not available in this build.", true );
            break;
        }
        case AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP:
        {
            // a keyring code first: the form for a new code, then the save
            m_keyringAction = KEYRINGACTION_SAVE_CREDENTIALS;
            m_keyringStore = m_pendingStore;
            if ( !m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_SET_UP ) )
            {
                m_keyringAction = KEYRINGACTION_NONE;
                WipePending();
            }
            break;
        }
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:
        {
            AskKeyring( KEYRINGACTION_SAVE_CREDENTIALS, m_pendingStore );
            break;
        }
        default:
        {
            DoSaveCredentials();
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsSave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StartSaveCredentials();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::DoSaveCredentials( void )
{GUCEF_TRACE;

    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( m_pendingStore );
    const CORE::CString storeId = m_pendingStore;
    const bool saved = GUCEF_NULL != signIn && signIn->SaveCredentials( m_pending );
    WipePending();
    if ( GUCEF_NULL != m_settings )
        m_settings->ShowMessage( saved ? CORE::CString( "Credentials saved." ) : CORE::CString( "The credentials could not be saved." ), !saved );
    if ( saved )
        Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( storeId ), "Credentials saved for the automatic sign in" );
    else
        Report( GUI::CReportViewForm::SEVERITY_FAILURE, GetStoreName( storeId ), "The credentials could not be saved" );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsForget( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    switch ( GetKeyringState() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:
        {
            AskKeyring( KEYRINGACTION_FORGET_CREDENTIALS, m_settingsStore );
            break;
        }
        case AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED:
        {
            m_keyringStore = m_settingsStore;
            DoForgetCredentials();
            m_keyringStore.Clear();
            break;
        }
        default:
        {
            m_settings->ShowMessage( "No credentials are saved.", false );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::DoForgetCredentials( void )
{GUCEF_TRACE;

    const CORE::CString storeId = m_keyringStore.IsNULLOrEmpty() ? m_settingsStore : m_keyringStore;
    AUTH::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    const bool forgotten = GUCEF_NULL != signIn && signIn->ForgetCredentials();
    if ( GUCEF_NULL != m_settings )
        m_settings->ShowMessage( forgotten ? CORE::CString( "The saved credentials are forgotten." ) : CORE::CString( "The saved credentials could not be forgotten." ), !forgotten );
    Report( forgotten ? GUI::CReportViewForm::SEVERITY_INFO : GUI::CReportViewForm::SEVERITY_FAILURE, GetStoreName( storeId ),
            forgotten ? CORE::CString( "The saved credentials are forgotten" ) : CORE::CString( "The saved credentials could not be forgotten" ) );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsSetUpCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_keyringAction = KEYRINGACTION_NONE;
    m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_SET_UP );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsChangeCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_keyringAction = KEYRINGACTION_NONE;
    m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_CHANGE_CODE );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsLockNow( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_credentials )
        m_credentials->GetKeyringSession().LockNow();
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnSettingsClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_settingsStore.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::IsCodeEntryProvider( const CORE::CString& authProviderName ) const
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings settings;
    if ( GUCEF_NULL == m_authManager || !m_authManager->GetSettings().GetProviderSettings( authProviderName, settings ) )
        return false;
    return settings.redirectMode == "code-entry" || settings.redirectMode == "manual";
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSignIn::BeginOAuthSignIn( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return false;
    const CORE::CString name = provider->GetDisplayName();
    const CORE::CString authProvider = provider->GetAuthProviderName();
    Watch( provider.GetPointerAlways(), GUCEF_NULL );
    if ( GUCEF_NULL == m_authManager )
    {
        Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in service of this workbench is not set up for this store" );
        ShowNotice( "Sign in to " + name, "Signing in to " + name + " needs a sign in service that this workbench has not set up." );
        return false;
    }
    if ( m_authManager->IsLoginPending( authProvider ) )
    {
        Report( GUI::CReportViewForm::SEVERITY_INFO, name, "A sign in runs already: finish it in the browser" );
        return false;
    }
    m_oauthStore = storeId;
    m_oauthProvider = authProvider;
    CORE::CFutureResult future( CORE::TASKSTATUS_UNDEFINED );
    AUTH::CAuthOptions options;
    options.forceInteractive = true;
    if ( !m_authManager->Login( authProvider, options, future ) )
    {
        // the manager raised LoginFailedEvent: the report and the notice come from there
        if ( !m_oauthStore.IsNULLOrEmpty() )
        {
            m_oauthStore.Clear();
            m_oauthProvider.Clear();
            Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in could not be started" );
        }
        return false;
    }
    Report( GUI::CReportViewForm::SEVERITY_INFO, name, "Signing in" );
    RefreshSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnAuthActionRequired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const AUTH::CAuthActionRequired* action = static_cast< const AUTH::CAuthActionRequired* >( eventData );
    if ( GUCEF_NULL == action || m_oauthStore.IsNULLOrEmpty() || action->providerName != m_oauthProvider )
        return;
    const CORE::CString name = GetStoreName( m_oauthStore );
    if ( AUTH::AUTHACTION_SHOW_DEVICE_CODE == action->kind )
    {
        Report( GUI::CReportViewForm::SEVERITY_WARNING, name, "Owner action needed: enter the code " + action->userCode + " at " + action->url );
        ShowNotice( "Sign in to " + name, "Go to " + action->url + " and enter the code " + action->userCode + "." );
        return;
    }
    if ( IsCodeEntryProvider( m_oauthProvider ) )
    {
        // the page shows a code: the address to go to and the field for the code
        m_codeEntry->SetEngine( GUCEF_NULL != m_flowEngine ? m_flowEngine : AUTH::CAuthGlobal::Instance()->GetFlowEngine() );
        Report( GUI::CReportViewForm::SEVERITY_WARNING, name, "Owner action needed: sign in on the page of the store and enter the code it shows" );
        m_codeEntry->Open( name, action->url );
        return;
    }
    Report( GUI::CReportViewForm::SEVERITY_INFO, name, "Finish the sign in in the browser" );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnAuthLoginSucceeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_oauthStore.IsNULLOrEmpty() )
        return;
    const CORE::CString name = GetStoreName( m_oauthStore );
    if ( m_codeEntry->IsOpen() )
        m_codeEntry->Close();
    Report( GUI::CReportViewForm::SEVERITY_INFO, name, "Signed in" );
    m_oauthStore.Clear();
    m_oauthProvider.Clear();
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnAuthLoginFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const AUTH::CAuthFailure* failure = static_cast< const AUTH::CAuthFailure* >( eventData );
    if ( GUCEF_NULL == failure || m_oauthStore.IsNULLOrEmpty() || failure->providerName != m_oauthProvider )
        return;
    const CORE::CString name = GetStoreName( m_oauthStore );
    if ( m_codeEntry->IsOpen() )
        m_codeEntry->Close();
    m_oauthStore.Clear();
    m_oauthProvider.Clear();
    RefreshSettings();
    if ( AUTH::AUTHFAIL_CANCELLED == failure->reason )
    {
        Report( GUI::CReportViewForm::SEVERITY_INFO, name, "The sign in was cancelled" );
        return;
    }
    Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in did not succeed", failure->message );
    ShowNotice( "Sign in to " + name, "The sign in did not succeed: " + failure->message + ". Press Sign in to try again." );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnCodeEntryDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the login goes on at the token endpoint of the provider and ends through the events of the manager
    if ( !m_oauthStore.IsNULLOrEmpty() )
        Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( m_oauthStore ), "The code was accepted: finishing the sign in" );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSignIn::OnCodeEntryCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_oauthStore.IsNULLOrEmpty() )
        return;
    const CORE::CString provider = m_oauthProvider;
    Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( m_oauthStore ), "The sign in was cancelled" );
    m_oauthStore.Clear();
    m_oauthProvider.Clear();
    if ( GUCEF_NULL != m_authManager )
        m_authManager->CancelLogin( provider );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchSignIn::GetSettingsStoreId( void ) const
{GUCEF_TRACE;

    return m_settingsStore;
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CKeyringUnlockForm* CWorkbenchSignIn::GetUnlockForm( void ) const { return m_unlock; }
AUTHGUI::COAuthCodeEntryForm* CWorkbenchSignIn::GetCodeEntryForm( void ) const { return m_codeEntry; }
AUTHGUI::CSignInSettingsForm* CWorkbenchSignIn::GetSettingsForm( void ) const { return m_settings; }
GUI::CMessageBoxForm* CWorkbenchSignIn::GetQuestionBox( void ) const { return m_question; }
GUI::CMessageBoxForm* CWorkbenchSignIn::GetNoticeBox( void ) const { return m_notice; }
CWorkbenchCredentials* CWorkbenchSignIn::GetCredentials( void ) const { return m_credentials; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchSignIn::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchSignIn";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
