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

#ifndef ASSETVIEWERLIB_CVIEWERSIGNIN_H
#define ASSETVIEWERLIB_CVIEWERSIGNIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H
#include "gucefAUTHGUI_CKeyringUnlockForm.h"
#define GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H
#endif /* GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H ? */

#ifndef GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H
#include "gucefAUTHGUI_COAuthCodeEntryForm.h"
#define GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H
#endif /* GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H ? */

#ifndef GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H
#include "gucefAUTHGUI_CSignInSettingsForm.h"
#define GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H
#endif /* GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#include "gucefASSETSTORE_CWebSessionSignIn.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNIN_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H
#include "gucefASSETSTORE_CWebSignInCredentials.h"
#define GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H
#endif /* GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERCREDENTIALS_H
#include "AssetViewerLib_CViewerCredentials.h"
#define ASSETVIEWERLIB_CVIEWERCREDENTIALS_H
#endif /* ASSETVIEWERLIB_CVIEWERCREDENTIALS_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The sign in of the online stores in the viewer: what the owner sees and does from the button "Sign in" of the sources page
 *  to a session, and the sign in settings of a store. The forms are those of gucefAUTHGUI and gucefGUI; this class routes.
 *
 *  A store that signs in on its web site (ASSETSTORE_SIGNIN_WEB_SESSION, ASSETSTORE::CWebSessionSignIn):
 *  - The owner presses Sign in. When the keyring code is set up and locked, the keyring form asks for it (a store may have saved
 *    credentials); then, with saved credentials, the sign in runs by itself without a window (WEBSIGNIN_MODE_UNATTENDED). When the
 *    owner gives up the keyring form, or there are no saved credentials, a message box announces that a browser window opens and
 *    asks to go on; then the sign in runs with the window (WEBSIGNIN_MODE_INTERACTIVE), also with saved credentials filled in.
 *  - An automatic sign in that ends because a person has to act (a captcha, a code by e-mail, a second factor), or fails, is offered
 *    again with the window.
 *  - What the sign in asks of the owner in the window ("Solve the puzzle in the browser window") is shown in a message box and put
 *    in the report as a warning; the outcome is in the report and, when it is a failure, in a message box. The notice that a window
 *    opens is in the report as well (the starting event of the sign in).
 *  - Everything runs on the pool of the sign in: the interface never waits for the network, the browser or the derivation of the key.
 *
 *  A store that signs in with a provider of gucefAUTH (ASSETSTORE_SIGNIN_AUTH_PROVIDER): CAuthManager::Login() of the manager that
 *  SetAuthManager() gave. The provider of the redirect mode "code-entry" or "manual" shows the authorization code on a page: the
 *  address is shown in the form COAuthCodeEntryForm, copied or opened in the browser, the code is pasted and handed to the flow
 *  engine (IAuthFlowEngine::SubmitAuthorizationCode(); each outcome that is not taken is said in its own words); the login ends
 *  through the events of the manager. A device code is shown in a message box.
 *
 *  The sign in settings of a store (OpenSettings()): CSignInSettingsForm: Sign in, Sign out, Save credentials (needs the keyring
 *  code: set up first when there is none, unlock when locked), Forget saved credentials, Set up or Change the keyring code, Lock now.
 *  What is typed is taken out of the fields, kept in memory only until the code is given, written to the credential store (sealed)
 *  and wiped. Nothing secret is logged, shown after it was submitted, or written to a setting.
 *
 *  The session of a store that ended is signed in again once by the automatic route (OnSessionExpired(), called by the sources page
 *  when an action of the library finds the session over).
 *
 *  The forms and the stores are not owned; they have to outlive this object or be taken away with Shutdown().
 */
class ASSETVIEWERLIB_PUBLIC_CPP CViewerSignIn : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CViewerSignIn > TEventCallback;

    CViewerSignIn( void );

    virtual ~CViewerSignIn() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes the forms in the context. 'report' (may be NULL) gets the notices and the outcomes; 'credentials' (may be NULL: then there
     *  is no saved credential and no automatic sign in) has the stores and the keyring session
     */
    bool Initialize( GUI::CGUIContext& context, GUI::CReportViewForm* report, CViewerCredentials* credentials );

    /** Closes the forms and lets go of everything */
    void Shutdown( void );

    bool IsInitialized( void ) const;

    /** The manager the stores that sign in with a provider of gucefAUTH use (not owned); GUCEF_NULL for none */
    void SetAuthManager( AUTH::CAuthManager* manager );

    /** The engine the authorization code is handed to; not owned, the engine of CAuthGlobal when none is given */
    void SetFlowEngine( AUTH::IAuthFlowEngine* engine );

    /** @name What the owner asks for */
    ///@{

    /** The button Sign in of a store: the route of its kind; false when nothing could be started (the reason is in the report) */
    bool SignIn( const CORE::CString& storeId );

    /** The session of the store ended and an action found it out: signs in again by the automatic route when that can be, else asks the owner */
    bool OnSessionExpired( const CORE::CString& storeId );

    /** Forgets the session of the store */
    bool SignOut( const CORE::CString& storeId );

    /** The window with the sign in settings of the store */
    bool OpenSettings( const CORE::CString& storeId );
    ///@}

    /** @name State, for tests and for applications that go beyond this */
    ///@{
    bool IsSignInRunning( const CORE::CString& storeId ) const;
    const CORE::CString& GetSettingsStoreId( void ) const;
    AUTHGUI::CKeyringUnlockForm* GetUnlockForm( void ) const;
    AUTHGUI::COAuthCodeEntryForm* GetCodeEntryForm( void ) const;
    AUTHGUI::CSignInSettingsForm* GetSettingsForm( void ) const;
    GUI::CMessageBoxForm* GetQuestionBox( void ) const;     /**< the questions: a window opens, go on? sign in with the window instead? */
    GUI::CMessageBoxForm* GetNoticeBox( void ) const;       /**< what the owner has to do in the browser window and how a sign in failed */
    CViewerCredentials* GetCredentials( void ) const;

    /** The state of the sign in settings of a store as the form shows it */
    AUTHGUI::CSignInSettingsState MakeSettingsState( const CORE::CString& storeId ) const;
    ///@}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** What a keyring code that is asked for goes on with */
    enum EKeyringAction
    {
        KEYRINGACTION_NONE = 0          ,
        KEYRINGACTION_AUTOMATIC_SIGNIN  ,
        KEYRINGACTION_SAVE_CREDENTIALS  ,
        KEYRINGACTION_FORGET_CREDENTIALS
    };

    /** What the question box asks */
    enum EQuestion
    {
        QUESTION_NONE = 0           ,
        QUESTION_OPEN_WINDOW        ,   /**< a window opens: go on? (OK, Cancel) */
        QUESTION_OFFER_WINDOW           /**< the automatic sign in did not do it: sign in with the window? (Yes, No) */
    };

    CViewerSignIn( const CViewerSignIn& src );              /**< not implemented */
    CViewerSignIn& operator=( const CViewerSignIn& src );   /**< not implemented */

    bool FindProvider( const CORE::CString& storeId, ASSETSTORE::TAssetStoreProviderPtr& provider ) const;
    ASSETSTORE::CWebSessionSignIn* GetWebSignIn( const CORE::CString& storeId ) const;
    void Watch( ASSETSTORE::CIAssetStoreProvider* provider, ASSETSTORE::CWebSessionSignIn* signIn );
    void Report( GUI::CReportViewForm::ESeverity severity, const CORE::CString& storeName, const CORE::CString& text, const CORE::CString& details = CORE::CString::Empty );
    void ShowNotice( const CORE::CString& title, const CORE::CString& text );
    void RefreshSettings( void );
    AUTHGUI::CSignInSettingsState::EKeyringState GetKeyringState( void ) const;

    bool BeginWebSignIn( const CORE::CString& storeId, bool sessionEnded );
    bool StartAutomatic( const CORE::CString& storeId );
    bool AskOpenWindow( const CORE::CString& storeId );
    bool StartWithWindow( const CORE::CString& storeId );
    void AskKeyring( EKeyringAction action, const CORE::CString& storeId );
    void ContinueAfterKeyring( EKeyringAction action, const CORE::CString& storeId );
    void AbandonAfterKeyring( EKeyringAction action, const CORE::CString& storeId );
    bool BeginOAuthSignIn( const CORE::CString& storeId );
    void DoSaveCredentials( void );
    void DoForgetCredentials( void );
    void StartSaveCredentials( void );
    void WipePending( void );
    bool IsCodeEntryProvider( const CORE::CString& authProviderName ) const;
    CORE::CString GetStoreName( const CORE::CString& storeId ) const;

    void OnWebSignInStarting( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnWebOwnerActionNeeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnWebSignedIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnWebSignInFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnWebSessionEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderSessionStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnQuestionClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnUnlockDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnUnlockCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyringChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsSignIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsSignOut( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsSave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsForget( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsSetUpCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsChangeCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsLockNow( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingsClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCodeEntryDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCodeEntryCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAuthActionRequired( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAuthLoginSucceeded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAuthLoginFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    GUI::CGUIContext* m_context;
    GUI::CReportViewForm* m_report;
    CViewerCredentials* m_credentials;
    AUTH::CAuthManager* m_authManager;
    AUTH::IAuthFlowEngine* m_flowEngine;
    AUTHGUI::CKeyringUnlockForm* m_unlock;
    AUTHGUI::COAuthCodeEntryForm* m_codeEntry;
    AUTHGUI::CSignInSettingsForm* m_settings;
    GUI::CMessageBoxForm* m_question;
    GUI::CMessageBoxForm* m_notice;
    EQuestion m_questionKind;
    CORE::CString m_questionStore;
    EKeyringAction m_keyringAction;
    CORE::CString m_keyringStore;
    CORE::CString m_settingsStore;
    CORE::CString m_automaticStore;         /**< the store whose automatic sign in runs */
    CORE::CString m_oauthStore;             /**< the store whose login with a provider of gucefAUTH runs */
    CORE::CString m_oauthProvider;
    CORE::CString::StringVector m_endedStores;
    ASSETSTORE::CWebSignInCredentials m_pending;
    CORE::CString m_pendingStore;
    GUCEF::vector< ASSETSTORE::CWebSessionSignIn* > m_watchedSignIns;
    GUCEF::vector< ASSETSTORE::CIAssetStoreProvider* > m_watchedProviders;
    bool m_initialized;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CVIEWERSIGNIN_H ? */
