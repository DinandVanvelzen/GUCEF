/*
 *  gucefAUTHGUI: GUCEF module providing the user interface for logging in with external identities and the account page of settings
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

#ifndef GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H
#define GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_IACCOUNTPROFILE_H
#include "gucefAUTH_IAccountProfile.h"
#define GUCEF_AUTH_IACCOUNTPROFILE_H
#endif /* GUCEF_AUTH_IACCOUNTPROFILE_H ? */

#ifndef GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#include "gucefAUTHGUI_CAccountsPanel.h"
#define GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#endif /* GUCEF_AUTHGUI_CACCOUNTSPANEL_H ? */

#ifndef GUCEF_AUTHGUI_MACROS_H
#include "gucefAUTHGUI_macros.h"
#define GUCEF_AUTHGUI_MACROS_H
#endif /* GUCEF_AUTHGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The page "Account" of a settings form (GUI::CSettingsForm): who the person is and how the person logs in. It is there
 *  for any tool that links this module (the module registers the page type "Account", so
 *  settingsForm.AddPage( "Account" ) is all it takes) and works with what the tool has:
 *      SetAuthManager()    the manager (not owned) the logins go through; without it the page shows a guest and says
 *                          that no sign-in service is set up
 *      SetProfile()        the profile (not owned) of the person; without it the page shows the account state only
 *
 *  From the top, in a scroll panel that is centred and as wide as 720 pixels:
 *      the name shown for the person (edit box) and the id of the profile (read only), with a profile
 *      the state line: "Guest", or "Linked: Epic Games, Google"
 *      the content of CAccountsPanel: "Log in with <name>" for the providers the manager offers with the progress of the
 *      login (waiting with Cancel, a device code with the page and Open, done, failed), or the line "No sign-in services are
 *      set up in this build. You play as a guest." when it offers none; the linked accounts with Unlink
 *      the providers the manager knows and does not offer, disabled, with the reason ("Not set up in this build: needs a client id") and the key of the
 *      configuration to fill in as tooltip, when the settings of the manager show them (showUnavailableProviders)
 *      the button "Forget remembered account names", only while the manager remembers the name of an account (an action that happens at once)
 *      the check box "Log in when the game starts" (the setting autoLogin of the manager), when providers are offered
 *  and below the scroll panel the line with what went wrong.
 *
 *  Settings semantics: logging in and unlinking are actions that happen at once. The name and the check box are values of the
 *  page: Apply gives the name to the profile (SetDisplayName(); an empty name or one the profile refuses is an error and
 *  stays a change) and the check box to the manager and notifies AutoLoginChangedEvent when that changed it; Cancel (ReadSettings())
 *  puts the values of the profile and the manager back. A change of the profile or the logins while the page shows (the name is
 *  changed elsewhere, a platform login finishes) makes the page again one frame later, keeping what the person typed.
 */
class GUCEF_AUTHGUI_EXPORT_CPP CAccountSettingsForm : public GUI::CSettingsPageForm
{
    public:

    static const CORE::CEvent AutoLoginChangedEvent;    /**< no data: Apply changed the setting autoLogin of the manager, the tool saves its configuration */
    static const CORE::CEvent LoginRequestedEvent;      /**< CORE::TCloneableString with the provider name: a login is about to start (CAccountsPanel::LoginRequestedEvent), before the manager is asked */

    static void RegisterEvents( void );

    /** The line when the manager offers no provider */
    static const char* const NoProvidersText;

    /** Names longer than this are cut off where the page shows them */
    static const UInt32 MaxDisplayNameLength = 32;

    CAccountSettingsForm( void );

    virtual ~CAccountSettingsForm();

    /** The manager the logins go through; not owned, it has to outlive the page or be taken away with GUCEF_NULL first */
    void SetAuthManager( AUTH::CAuthManager* manager );

    AUTH::CAuthManager* GetAuthManager( void ) const;

    /** The profile of the person; not owned, it has to outlive the page or be taken away with GUCEF_NULL first */
    void SetProfile( AUTH::IAccountProfile* profile );

    AUTH::IAccountProfile* GetProfile( void ) const;

    /** Logins ask for a code on another device instead of opening a browser */
    void SetPreferDeviceCode( bool preferDeviceCode );

    /** The content the page shares with the login form */
    CAccountsPanel& GetPanel( void );

    /** Makes the widgets again from the manager and the profile, at once (else that happens at the next frame); what was typed stays unless 'keepChanges' is false */
    void Refresh( bool keepChanges = true );

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BuildPage( GUI::CWidget* parent ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The name in the edit box, empty without a profile */
    CORE::CString GetDisplayNameText( void );

    /** Puts a name in the edit box as the person typing it does (a change of the page); false without a profile */
    bool SetDisplayNameText( const CORE::CString& name );

    /** The id of the profile as shown, empty without a profile */
    CORE::CString GetProfileIdText( void );

    /** Whether the page has the fields of the profile */
    bool HasProfileFields( void );

    /** The line with the login state: "Guest", or "Linked: Google, Epic Games" */
    CORE::CString GetStateText( void );

    /** What went wrong last (a login, a name that could not be kept); empty when nothing */
    CORE::CString GetErrorText( void ) const;

    /** The check box, false when the page has none (no provider is offered) */
    bool HasAutoLoginBox( void );

    bool GetAutoLogin( void );

    /** Switches the check box as the person does (a change of the page) */
    bool SetAutoLogin( bool autoLogin );

    bool Login( const CORE::CString& providerName );

    bool CancelLogin( const CORE::CString& providerName );

    bool Unlink( const AUTH::CAccountRef& account );

    /** Whether the page has the button "Forget remembered account names" (the manager remembers names of accounts) */
    bool HasForgetLoginHintsButton( void );

    /** Presses that button: the names are forgotten at once; true when there were any */
    bool ForgetLoginHints( void );

    bool OpenDeviceCodePage( const CORE::CString& providerName );

    void GetShownProviders( CORE::CString::StringVector& providerNames ) const;

    /** The providers shown disabled, with the reason below their button */
    void GetShownUnavailableProviders( CORE::CString::StringVector& providerNames ) const;

    CAccountsPanel::TLoginProgress GetProgress( const CORE::CString& providerName ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Only the standard events: the profile that is destroyed is let go of */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAccountSettingsForm > TEventCallback;

    /** The panel has an error to show */
    void OnPanelErrorChanged( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    /** A login is about to start at a provider */
    void OnPanelLoginRequested( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    /** What the content of the panel shows changed: the page is made again at the next frame */
    void OnPanelChanged( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    /** The profile changed: what the page shows is made again at the next frame */
    void OnProfileChanged( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    /** A frame of the context starts: the page is made again when what it shows or its width changed */
    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    CAccountSettingsForm( const CAccountSettingsForm& src );
    CAccountSettingsForm& operator=( const CAccountSettingsForm& src );

    bool Rebuild( bool keepChanges );
    void ShowError( const CORE::CString& text );
    void ShowValues( void );
    bool IsNarrow( void );

    private:

    CAccountsPanel m_panel;
    AUTH::IAccountProfile* m_profile;
    CORE::CNotifier* m_profileNotifier;
    CORE::CString m_errorText;
    bool m_dirty;
    bool m_rebuilding;
    bool m_narrow;
    bool m_subscribedToFrames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H ? */
