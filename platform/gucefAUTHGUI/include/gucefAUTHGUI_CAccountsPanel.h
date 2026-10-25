/*
 *  gucefAUTHGUI: GUCEF module providing prebaked account and login user interface forms
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


#ifndef GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#define GUCEF_AUTHGUI_CACCOUNTSPANEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_CFORM_H
#include "gucefGUI_CForm.h"
#define GUCEF_GUI_CFORM_H
#endif /* GUCEF_GUI_CFORM_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#include "gucefAUTH_CAuthEventData.h"
#define GUCEF_AUTH_CAUTHEVENTDATA_H
#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */

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
 *  What the forms that log a player in with external identities (gucefAUTH) share: the content of "Log in with ..." and
 *  "Linked accounts" and everything behind it. A host form (CLoginForm, CAccountSettingsForm) owns a panel, gives it the
 *  manager (SetAuthManager(), not owned) and
 *      puts its content in the layout it builds with AddContent(),
 *      binds the widgets that layout made with Bind() when the layout is loaded,
 *      makes its layout again when the panel notifies ChangedEvent (the widgets of the content are new then).
 *
 *  The content has, below each other in the list it is given:
 *      a header "Log in with" and a button "Log in with <name>" for every provider the manager offers
 *      (CAuthManager::GetOfferedProviders(), the names are those of the descriptors of CAuthGlobal's registry, else the name of
 *      the provider), with the progress of that login next to it:
 *          idle                          nothing
 *          waiting for the browser       "Waiting..." and a Cancel button
 *          a device code                 the page to go to and the code in two read only fields to copy from, an Open button for the page
 *                                        and Cancel
 *          done                          "Logged in"
 *          failed                        "Failed" in red, the reason is the error text
 *      for a provider that detected something on this machine (CIAuthProvider::GetDetectedAccountHint(), the client of a store that is
 *      installed) the name it found in the button, "Log in with <provider> (<name>)", and a line below it, "<provider> is installed and
 *      running" or "<provider> is installed". The hint only makes the offer concrete: the login is the one that proves who the person is
 *      the line SetNoProvidersText() says instead when the manager offers none (or there is no manager)
 *      the providers the manager knows and does not offer (CAuthManager::GetUnavailableProviders(), when its settings show them)
 *      after those: a disabled button "Log in with <name>", the reason as a line below it ("Not set up in this build: needs a
 *      client id") and the reason with the key of the configuration to fill in as the tooltip of both
 *      a header "Linked accounts" and the linked accounts (CAuthManager::GetLinkedAccounts()), each with an Unlink button,
 *      "None. You play as a guest." without.
 *      the button "Forget remembered account names" while the manager remembers any (CAuthManager::HasLoginHints(): the name of
 *      the account of the last login at a provider, which the next login prefills); nothing of that is shown otherwise.
 *  The progress comes from the events of the manager (ActionRequiredEvent, LoginSucceededEvent, LoginFailedEvent and the
 *  others), nothing waits. Tokens and proofs are never shown.
 *
 *  A login that starts here first raises LoginRequestedEvent, with the provider name, so that a host that has to prepare for a
 *  login (an application that keeps a plain profile until the person logs in with something) can do that before the manager is called.
 *
 *  Widget names: ProvidersHeader, NoProviders, Row<n>, Login<n>, Status<n>, Hint<n>, Cancel<n>, UrlLabel<n>, Url<n>, Open<n>, CodeLabel<n>,
 *  Code<n>, UnavailableRow<n>, UnavailableLogin<n>, UnavailableReason<n>, AccountsHeader, NoAccounts, AccountRow<n>, AccountText<n>, Unlink<n>,
 *  ForgetHints.
 */
class GUCEF_AUTHGUI_EXPORT_CPP CAccountsPanel : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ChangedEvent;                 /**< no data: what the content shows changed, the host makes its layout again */
    static const CORE::CEvent ErrorChangedEvent;            /**< no data: GetErrorText() changed */
    static const CORE::CEvent AutoLoginChangedEvent;        /**< no data: SetAutoLogin() changed the setting autoLogin of the manager */
    static const CORE::CEvent LoginRequestedEvent;          /**< CORE::TCloneableString with the provider name: a login is about to start, raised before the manager is asked */

    static void RegisterEvents( void );

    /** Where a login with one provider is */
    enum ELoginProgress
    {
        LOGINPROGRESS_IDLE = 0                  ,
        LOGINPROGRESS_WAITING_FOR_BROWSER = 1   ,   /**< the browser has the login, the panel waits for it */
        LOGINPROGRESS_SHOWING_DEVICE_CODE = 2   ,   /**< the player is to enter a code at a page */
        LOGINPROGRESS_DONE = 3                  ,
        LOGINPROGRESS_FAILED = 4
    };
    typedef enum ELoginProgress TLoginProgress;

    /** Names longer than this (in characters) are cut off with "..." where the panel shows them */
    static const UInt32 MaxShownTextLength = 44;

    /** The name a provider detected is cut off at this many characters in the button */
    static const UInt32 MaxHintNameLength = 20;

    CAccountsPanel( void );

    virtual ~CAccountsPanel();

    /** The manager the panel shows and calls; not owned, it has to outlive the panel or be taken away with GUCEF_NULL first */
    void SetAuthManager( AUTH::CAuthManager* manager );

    AUTH::CAuthManager* GetAuthManager( void ) const;

    /** What the content says when there is no provider to log in with; "No login is set up for this game." at first */
    void SetNoProvidersText( const CORE::CString& text );

    const CORE::CString& GetNoProvidersText( void ) const;

    /** Logins ask for a code on another device instead of opening a browser, for a machine without one */
    void SetPreferDeviceCode( bool preferDeviceCode );

    bool GetPreferDeviceCode( void ) const;

    /** Adds the content to 'list', a container of a layout under construction (a VBox, whose spacing places the parts); forgets the widgets of the last layout */
    void AddContent( CORE::CDataNode& list );

    /** Subscribes to the widgets of the content as they are in the form, to be called from the form's OnPostLayoutLoad() */
    void Bind( GUI::CForm& form );

    /** Starts a login with the provider, as its button does. False when the manager did not start it (the panel then has the reason as its error) */
    bool Login( const CORE::CString& providerName );

    /** Cancels the login with the provider, as its Cancel button does */
    bool CancelLogin( const CORE::CString& providerName );

    /** Removes the link of the account, as its Unlink button does */
    bool Unlink( const AUTH::CAccountRef& account );

    /** Whether the manager remembers names of accounts, which is when the button to forget them is shown */
    bool HasRememberedLoginHints( void ) const;

    /** Forgets the remembered account names, as the button "Forget remembered account names" does; true when there were any */
    bool ForgetLoginHints( void );

    /** Opens the page of a device code in the browser, as the Open button does. False when there is no such code or the browser was not started */
    bool OpenDeviceCodePage( const CORE::CString& providerName );

    /** The setting autoLogin of the manager */
    bool GetAutoLogin( void ) const;

    /** Changes the setting autoLogin of the manager; notifies AutoLoginChangedEvent and returns true when it changed */
    bool SetAutoLogin( bool autoLogin );

    /** The providers shown with a login button by the last AddContent(), in order */
    void GetShownProviders( CORE::CString::StringVector& providerNames ) const;

    /** The providers the manager offers now */
    void GetOfferedProviders( CORE::CString::StringVector& providerNames ) const;

    /** The providers shown disabled by the last AddContent(), in order */
    void GetShownUnavailableProviders( CORE::CString::StringVector& providerNames ) const;

    /** The text of the button of an offered provider: "Log in with <provider>", or "Log in with <provider> (<name>)" when the provider detected a name; the name is cut off at MaxHintNameLength characters */
    CORE::CString GetLoginButtonText( const CORE::CString& providerName ) const;

    /** What the status next to the button of a login says while it waits: "Waiting, up to 10 min" (the time the login is given), "Waiting..." when that is not known */
    CORE::CString GetWaitingText( const CORE::CString& providerName ) const;

    /** The line below the button of an offered provider that detected something: "<provider> is installed and running", "<provider> is installed"; empty when nothing was detected */
    CORE::CString GetHintText( const CORE::CString& providerName ) const;

    /** The reason line of a provider shown disabled ("Not set up in this build: needs a client id"), empty when it is not one */
    CORE::CString GetUnavailableText( const CORE::CString& providerName ) const;

    /** The tooltip of a provider shown disabled: the reason and the key of the configuration to fill in; empty when it is not one */
    CORE::CString GetUnavailableTooltip( const CORE::CString& providerName ) const;

    TLoginProgress GetProgress( const CORE::CString& providerName ) const;

    /** What the panel says of the login with the provider: the failure, the account that logged in; empty otherwise */
    CORE::CString GetProgressMessage( const CORE::CString& providerName ) const;

    /** The device code shown for the provider, empty when none is */
    CORE::CString GetDeviceCode( const CORE::CString& providerName ) const;

    /** The page of the device code shown for the provider, empty when none is */
    CORE::CString GetDeviceCodeUrl( const CORE::CString& providerName ) const;

    /** The login state: "Guest", or "Linked: Google, Epic Games" (at most three names and a count of the others) */
    CORE::CString GetStateText( void ) const;

    /** What went wrong last, empty when nothing */
    CORE::CString GetErrorText( void ) const;

    void ClearError( void );

    /** The text of an account in the list: "Epic Games: 0123456789abcdef0123456789abcdef", cut off at MaxShownTextLength characters */
    static CORE::CString MakeAccountText( const AUTH::CAccountRef& account );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Only the standard events: the manager that is destroyed is let go of */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAccountsPanel > TEventCallback;

    enum EAction
    {
        ACTION_LOGIN        ,
        ACTION_CANCEL       ,
        ACTION_OPEN_PAGE    ,
        ACTION_UNLINK       ,
        ACTION_FORGET_HINTS
    };

    /** A widget of the content and what it does */
    struct SBinding
    {
        CORE::CString widgetName;
        EAction action;
        CORE::CString providerName;
        AUTH::CAccountRef account;
        SBinding( void ) : widgetName(), action( ACTION_LOGIN ), providerName(), account() {}
    };
    typedef GUCEF::vector< SBinding > TBindingVector;

    /** What the panel knows of the login with one provider */
    struct SProgress
    {
        TLoginProgress progress;
        CORE::CString message;
        AUTH::CAuthActionRequired action;
        UInt32 minutesToWait;           /**< how long the login waits for the person, from the moment the browser was asked for, rounded up; 0 when not known */
        SProgress( void ) : progress( LOGINPROGRESS_IDLE ), message(), action(), minutesToWait( 0 ) {}
    };
    typedef GUCEF::map< CORE::CString, SProgress > TProgressMap;

    CAccountsPanel( const CAccountsPanel& src );
    CAccountsPanel& operator=( const CAccountsPanel& src );

    void AddProviderRow( CORE::CDataNode& list, const CORE::CString& providerName, UInt32 index );
    void AddUnavailableRow( CORE::CDataNode& list, const AUTH::CAuthUnavailableProvider& unavailable, UInt32 index );
    void AddAccountRow( CORE::CDataNode& list, const AUTH::CAccountRef& account, UInt32 index );
    void ShowError( const CORE::CString& text );

    /** A button of the content was activated: does what its binding says */
    void OnWidgetActivated( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    /** The manager asks the person to do something (a page to go to, a code to enter) */
    void OnManagerActionRequired( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void OnManagerLoginSucceeded( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void OnManagerLoginFailed( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    /** The state of the logins and links of the manager changed: the content is made again by the host */
    void OnManagerStateChanged( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    SProgress& GetOrMakeProgress( const CORE::CString& providerName );
    static Int32 GetRowHeight( TLoginProgress progress, bool hasHint );

    private:

    AUTH::CAuthManager* m_manager;
    GUI::CForm* m_form;
    CORE::CString m_noProvidersText;
    TProgressMap m_progress;
    TBindingVector m_bindings;
    CORE::CString::StringVector m_shownProviders;
    CORE::CString::StringVector m_shownUnavailable;
    AUTH::TAuthUnavailableProviderVector m_unavailable;
    CORE::CString m_error;
    bool m_preferDeviceCode;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CACCOUNTSPANEL_H ? */
