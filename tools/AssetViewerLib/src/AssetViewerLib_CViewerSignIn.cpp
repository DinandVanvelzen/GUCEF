/*
 *  AssetViewerLib: the sign in of the viewer: the credential stores, the keyring code and the routing of the sign in of an online store
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

#ifndef ASSETVIEWERLIB_CVIEWERSIGNIN_H
#include "AssetViewerLib_CViewerSignIn.h"
#define ASSETVIEWERLIB_CVIEWERSIGNIN_H
#endif /* ASSETVIEWERLIB_CVIEWERSIGNIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CViewerSignIn::CViewerSignIn( void )
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

CViewerSignIn::~CViewerSignIn()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::Initialize( GUI::CGUIContext& context, GUI::CReportViewForm* report, CViewerCredentials* credentials )
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerSignIn: The forms of the sign in cannot be made" );
        Shutdown();
        return false;
    }

    if ( GUCEF_NULL != m_credentials )
    {
        AUTH::CKeyringSession& session = m_credentials->GetKeyringSession();
        m_unlock->SetSession( &session );
        TEventCallback changedCallback( this, &CViewerSignIn::OnKeyringChanged );
        SubscribeTo( &session, AUTH::CKeyringSession::UnlockedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::LockedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeSetUpEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeChangedEvent, changedCallback );
        SubscribeTo( &session, AUTH::CKeyringSession::ForgottenEvent, changedCallback );
    }
    TEventCallback unlockDoneCallback( this, &CViewerSignIn::OnUnlockDone );
    SubscribeTo( m_unlock, AUTHGUI::CKeyringUnlockForm::DoneEvent, unlockDoneCallback );
    TEventCallback unlockCancelledCallback( this, &CViewerSignIn::OnUnlockCancelled );
    SubscribeTo( m_unlock, AUTHGUI::CKeyringUnlockForm::CancelledEvent, unlockCancelledCallback );

    TEventCallback codeDoneCallback( this, &CViewerSignIn::OnCodeEntryDone );
    SubscribeTo( m_codeEntry, AUTHGUI::COAuthCodeEntryForm::DoneEvent, codeDoneCallback );
    TEventCallback codeCancelledCallback( this, &CViewerSignIn::OnCodeEntryCancelled );
    SubscribeTo( m_codeEntry, AUTHGUI::COAuthCodeEntryForm::CancelledEvent, codeCancelledCallback );

    TEventCallback signInCallback( this, &CViewerSignIn::OnSettingsSignIn );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SignInRequestedEvent, signInCallback );
    TEventCallback signOutCallback( this, &CViewerSignIn::OnSettingsSignOut );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SignOutRequestedEvent, signOutCallback );
    TEventCallback saveCallback( this, &CViewerSignIn::OnSettingsSave );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SaveCredentialsRequestedEvent, saveCallback );
    TEventCallback forgetCallback( this, &CViewerSignIn::OnSettingsForget );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ForgetCredentialsRequestedEvent, forgetCallback );
    TEventCallback setUpCallback( this, &CViewerSignIn::OnSettingsSetUpCode );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::SetUpCodeRequestedEvent, setUpCallback );
    TEventCallback changeCallback( this, &CViewerSignIn::OnSettingsChangeCode );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ChangeCodeRequestedEvent, changeCallback );
    TEventCallback lockCallback( this, &CViewerSignIn::OnSettingsLockNow );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::LockNowRequestedEvent, lockCallback );
    TEventCallback closedCallback( this, &CViewerSignIn::OnSettingsClosed );
    SubscribeTo( m_settings, AUTHGUI::CSignInSettingsForm::ClosedEvent, closedCallback );

    TEventCallback questionCallback( this, &CViewerSignIn::OnQuestionClosed );
    SubscribeTo( m_question, GUI::CMessageBoxForm::ClosedEvent, questionCallback );

    m_initialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::Shutdown( void )
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
CViewerSignIn::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    if ( manager == m_authManager )
        return;
    if ( GUCEF_NULL != m_authManager )
        UnsubscribeFrom( *m_authManager );
    m_authManager = manager;
    if ( GUCEF_NULL == m_authManager )
        return;
    TEventCallback actionCallback( this, &CViewerSignIn::OnAuthActionRequired );
    SubscribeTo( m_authManager, AUTH::CAuthManager::ActionRequiredEvent, actionCallback );
    TEventCallback succeededCallback( this, &CViewerSignIn::OnAuthLoginSucceeded );
    SubscribeTo( m_authManager, AUTH::CAuthManager::LoginSucceededEvent, succeededCallback );
    TEventCallback failedCallback( this, &CViewerSignIn::OnAuthLoginFailed );
    SubscribeTo( m_authManager, AUTH::CAuthManager::LoginFailedEvent, failedCallback );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::SetFlowEngine( AUTH::IAuthFlowEngine* engine )
{GUCEF_TRACE;

    m_flowEngine = engine;
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::FindProvider( const CORE::CString& storeId, ASSETSTORE::TAssetStoreProviderPtr& provider ) const
{GUCEF_TRACE;

    return ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId, provider ) && !provider.IsNULL();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::CWebSessionSignIn*
CViewerSignIn::GetWebSignIn( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( FindProvider( storeId, provider ) && ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION == provider->GetSignInKind() )
        return provider->GetWebSessionSignIn();
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CViewerSignIn::GetStoreName( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    return FindProvider( storeId, provider ) ? provider->GetDisplayName() : storeId;
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::Watch( ASSETSTORE::CIAssetStoreProvider* provider, ASSETSTORE::CWebSessionSignIn* signIn )
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
            TEventCallback callback( this, &CViewerSignIn::OnProviderSessionStateChanged );
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
            TEventCallback startingCallback( this, &CViewerSignIn::OnWebSignInStarting );
            SubscribeTo( signIn, ASSETSTORE::CWebSessionSignIn::SignInStartingEvent, startingCallback );
            TEventCallback ownerCallback( this, &CViewerSignIn::OnWebOwnerActionNeeded );
            SubscribeTo( signIn, ASSETSTORE::CWebSessionSignIn::OwnerActionNeededEvent, ownerCallback );
            TEventCallback signedInCallback( this, &CViewerSignIn::OnWebSignedIn );
            SubscribeTo( signIn, ASSETSTORE::CWebSessionSignIn::SignedInEvent, signedInCallback );
            TEventCallback failedCallback( this, &CViewerSignIn::OnWebSignInFailed );
            SubscribeTo( signIn, ASSETSTORE::CWebSessionSignIn::SignInFailedEvent, failedCallback );
            TEventCallback endedCallback( this, &CViewerSignIn::OnWebSessionEnded );
            SubscribeTo( signIn, ASSETSTORE::CWebSessionSignIn::SessionEndedEvent, endedCallback );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::Report( GUI::CReportViewForm::ESeverity severity, const CORE::CString& storeName, const CORE::CString& text, const CORE::CString& details )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_report )
        m_report->AddEntry( severity, storeName, text, details );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::ShowNotice( const CORE::CString& title, const CORE::CString& text )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_notice )
        m_notice->Open( title, text, GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_OK );
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CSignInSettingsState::EKeyringState
CViewerSignIn::GetKeyringState( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_credentials || !m_credentials->IsKeyringAvailable() )
        return AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE;
    const AUTH::CKeyringSession& session = m_credentials->GetKeyringSession();
    if ( !session.IsSetUp() )
        return AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP;
    return session.IsLocked() ? AUTHGUI::CSignInSettingsState::KEYRING_LOCKED : AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED;
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CSignInSettingsState
CViewerSignIn::MakeSettingsState( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    AUTHGUI::CSignInSettingsState state;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return state;
    state.serviceName = provider->GetDisplayName();
    state.signedIn = provider->IsSignedIn();
    state.keyringState = GetKeyringState();
    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
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
CViewerSignIn::RefreshSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_settings && !m_settingsStore.IsNULLOrEmpty() )
        m_settings->SetState( MakeSettingsState( m_settingsStore ) );
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::SignIn( const CORE::CString& storeId )
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;
    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerSignIn: The store " << storeId << " is not known: it cannot be signed in to" );
        return false;
    }
    switch ( provider->GetSignInKind() )
    {
        case ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION: return BeginWebSignIn( storeId, false );
        case ASSETSTORE::ASSETSTORE_SIGNIN_AUTH_PROVIDER: return BeginOAuthSignIn( storeId );
        default:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerSignIn: The store " << storeId << " has no sign in" );
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::OnSessionExpired( const CORE::CString& storeId )
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
CViewerSignIn::BeginWebSignIn( const CORE::CString& storeId, bool sessionEnded )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return false;
    ASSETSTORE::CWebSessionSignIn* signIn = provider->GetWebSessionSignIn();
    if ( GUCEF_NULL == signIn )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CViewerSignIn: The sign in to " << storeId << " is not available in this viewer" );
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
CViewerSignIn::StartAutomatic( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL == signIn )
        return false;
    Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( storeId ), "Signing in with the saved credentials, without a window" );
    m_automaticStore = storeId;
    signIn->SignInAsync( ASSETSTORE::WEBSIGNIN_MODE_UNATTENDED );
    RefreshSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::AskOpenWindow( const CORE::CString& storeId )
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
CViewerSignIn::StartWithWindow( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL == signIn )
        return false;
    if ( signIn->IsSignInRunning() )
        return false;
    m_automaticStore.Clear();
    signIn->SignInAsync( ASSETSTORE::WEBSIGNIN_MODE_INTERACTIVE );
    RefreshSettings();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnQuestionClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::AskKeyring( EKeyringAction action, const CORE::CString& storeId )
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
CViewerSignIn::ContinueAfterKeyring( EKeyringAction action, const CORE::CString& storeId )
{GUCEF_TRACE;

    switch ( action )
    {
        case KEYRINGACTION_AUTOMATIC_SIGNIN:
        {
            ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
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
CViewerSignIn::AbandonAfterKeyring( EKeyringAction action, const CORE::CString& storeId )
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
CViewerSignIn::OnUnlockDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const EKeyringAction action = m_keyringAction;
    const CORE::CString storeId = m_keyringStore;
    m_keyringAction = KEYRINGACTION_NONE;
    m_keyringStore.Clear();
    ContinueAfterKeyring( action, storeId );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnUnlockCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const EKeyringAction action = m_keyringAction;
    const CORE::CString storeId = m_keyringStore;
    m_keyringAction = KEYRINGACTION_NONE;
    m_keyringStore.Clear();
    AbandonAfterKeyring( action, storeId );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnKeyringChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnWebSignInStarting( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CWebSignInEventData* data = static_cast< const ASSETSTORE::CWebSignInEventData* >( eventData );
    if ( GUCEF_NULL != data )
        Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( data->storeId ), data->message );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnWebOwnerActionNeeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CWebSignInEventData* data = static_cast< const ASSETSTORE::CWebSignInEventData* >( eventData );
    if ( GUCEF_NULL == data )
        return;
    const CORE::CString name = GetStoreName( data->storeId );
    Report( GUI::CReportViewForm::SEVERITY_WARNING, name, "Owner action needed: " + data->message );
    // what to do in the browser window, where the owner can see it
    ShowNotice( "Sign in to " + name, data->message );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnWebSignedIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CWebSignInResult* result = static_cast< const ASSETSTORE::CWebSignInResult* >( eventData );
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
CViewerSignIn::OnWebSignInFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CWebSignInResult* result = static_cast< const ASSETSTORE::CWebSignInResult* >( eventData );
    if ( GUCEF_NULL == result )
        return;
    const bool automatic = m_automaticStore == result->storeId;
    if ( automatic )
        m_automaticStore.Clear();
    const CORE::CString name = GetStoreName( result->storeId );
    Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in did not succeed: " + CORE::CString( ASSETSTORE::AssetStoreStatusToString( result->status ) ), result->message );
    RefreshSettings();
    if ( ASSETSTORE::ASSETSTORE_STATUS_CANCELLED == result->status )
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
CViewerSignIn::OnWebSessionEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::OnProviderSessionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::SignOut( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !m_initialized || !FindProvider( storeId, provider ) )
        return false;
    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
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
CViewerSignIn::IsSignInRunning( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    if ( GUCEF_NULL != signIn )
        return signIn->IsSignInRunning();
    return !m_oauthStore.IsNULLOrEmpty() && m_oauthStore == storeId;
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::OpenSettings( const CORE::CString& storeId )
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
CViewerSignIn::OnSettingsSignIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_settingsStore.IsNULLOrEmpty() )
        SignIn( m_settingsStore );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnSettingsSignOut( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_settingsStore.IsNULLOrEmpty() )
        SignOut( m_settingsStore );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::WipePending( void )
{GUCEF_TRACE;

    m_pending.Clear();
    m_pendingStore.Clear();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::StartSaveCredentials( void )
{GUCEF_TRACE;

    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( m_settingsStore );
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
CViewerSignIn::OnSettingsSave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StartSaveCredentials();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::DoSaveCredentials( void )
{GUCEF_TRACE;

    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( m_pendingStore );
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
CViewerSignIn::OnSettingsForget( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::DoForgetCredentials( void )
{GUCEF_TRACE;

    const CORE::CString storeId = m_keyringStore.IsNULLOrEmpty() ? m_settingsStore : m_keyringStore;
    ASSETSTORE::CWebSessionSignIn* signIn = GetWebSignIn( storeId );
    const bool forgotten = GUCEF_NULL != signIn && signIn->ForgetCredentials();
    if ( GUCEF_NULL != m_settings )
        m_settings->ShowMessage( forgotten ? CORE::CString( "The saved credentials are forgotten." ) : CORE::CString( "The saved credentials could not be forgotten." ), !forgotten );
    Report( forgotten ? GUI::CReportViewForm::SEVERITY_INFO : GUI::CReportViewForm::SEVERITY_FAILURE, GetStoreName( storeId ),
            forgotten ? CORE::CString( "The saved credentials are forgotten" ) : CORE::CString( "The saved credentials could not be forgotten" ) );
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnSettingsSetUpCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_keyringAction = KEYRINGACTION_NONE;
    m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_SET_UP );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnSettingsChangeCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_keyringAction = KEYRINGACTION_NONE;
    m_unlock->Open( AUTHGUI::CKeyringUnlockForm::MODE_CHANGE_CODE );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnSettingsLockNow( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_credentials )
        m_credentials->GetKeyringSession().LockNow();
    RefreshSettings();
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnSettingsClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_settingsStore.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::IsCodeEntryProvider( const CORE::CString& authProviderName ) const
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings settings;
    if ( GUCEF_NULL == m_authManager || !m_authManager->GetSettings().GetProviderSettings( authProviderName, settings ) )
        return false;
    return settings.redirectMode == "code-entry" || settings.redirectMode == "manual";
}

/*-------------------------------------------------------------------------*/

bool
CViewerSignIn::BeginOAuthSignIn( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return false;
    const CORE::CString name = provider->GetDisplayName();
    const CORE::CString authProvider = provider->GetAuthProviderName();
    Watch( provider.GetPointerAlways(), GUCEF_NULL );
    if ( GUCEF_NULL == m_authManager )
    {
        Report( GUI::CReportViewForm::SEVERITY_FAILURE, name, "The sign in service of this viewer is not set up for this store" );
        ShowNotice( "Sign in to " + name, "Signing in to " + name + " needs a sign in service that this viewer has not set up." );
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
CViewerSignIn::OnAuthActionRequired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::OnAuthLoginSucceeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::OnAuthLoginFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::OnCodeEntryDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the login goes on at the token endpoint of the provider and ends through the events of the manager
    if ( !m_oauthStore.IsNULLOrEmpty() )
        Report( GUI::CReportViewForm::SEVERITY_INFO, GetStoreName( m_oauthStore ), "The code was accepted: finishing the sign in" );
}

/*-------------------------------------------------------------------------*/

void
CViewerSignIn::OnCodeEntryCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
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
CViewerSignIn::GetSettingsStoreId( void ) const
{GUCEF_TRACE;

    return m_settingsStore;
}

/*-------------------------------------------------------------------------*/

AUTHGUI::CKeyringUnlockForm* CViewerSignIn::GetUnlockForm( void ) const { return m_unlock; }
AUTHGUI::COAuthCodeEntryForm* CViewerSignIn::GetCodeEntryForm( void ) const { return m_codeEntry; }
AUTHGUI::CSignInSettingsForm* CViewerSignIn::GetSettingsForm( void ) const { return m_settings; }
GUI::CMessageBoxForm* CViewerSignIn::GetQuestionBox( void ) const { return m_question; }
GUI::CMessageBoxForm* CViewerSignIn::GetNoticeBox( void ) const { return m_notice; }
CViewerCredentials* CViewerSignIn::GetCredentials( void ) const { return m_credentials; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CViewerSignIn::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETVIEWER::CViewerSignIn";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
