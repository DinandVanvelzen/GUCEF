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

#ifndef GUCEF_AUTHGUI_CLOGINFORM_H
#define GUCEF_AUTHGUI_CLOGINFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

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
 *  "Log in with ...": the window in which a player logs in with an external identity (gucefAUTH) and sees which accounts
 *  are linked to the profile. It is bound to a CAuthManager (SetAuthManager(), not owned) and calls it directly.
 *
 *  The window has, from the top: a line with the login state ("Guest", or "Linked: Google, Epic Games"), a scrolling body and
 *  the strip of the buttons along the bottom edge.
 *  The body has the content of CAccountsPanel (a "Log in with <name>" button per provider the manager offers with the
 *  progress of that login, the linked accounts each with an Unlink button) and a check box "Log in when the game starts"
 *  for the setting autoLogin of the manager. The strip has the error line and Close.
 *
 *  It is a window like the others (moved by its title bar, folded up into it), not modal, nine tenths of the screen up to
 *  640 x 820 and in the middle. The content is built in code from what the manager says and made again, one frame after a
 *  change, when something changes; the window keeps the place the player put it at and stays folded when it was.
 *
 *  Events: ClosedEvent when it closes (the Close button, Escape), AutoLoginChangedEvent when the player switches the check
 *  box (the manager has the setting then; the application saves its configuration).
 */
class GUCEF_AUTHGUI_EXPORT_CPP CLoginForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent ClosedEvent;                  /**< no data */
    static const CORE::CEvent AutoLoginChangedEvent;        /**< no data: the player changed the setting autoLogin of the manager */

    static void RegisterEvents( void );

    /** Where a login with one provider is */
    typedef CAccountsPanel::ELoginProgress ELoginProgress;
    typedef CAccountsPanel::TLoginProgress TLoginProgress;
    static const TLoginProgress LOGINPROGRESS_IDLE = CAccountsPanel::LOGINPROGRESS_IDLE;
    static const TLoginProgress LOGINPROGRESS_WAITING_FOR_BROWSER = CAccountsPanel::LOGINPROGRESS_WAITING_FOR_BROWSER;
    static const TLoginProgress LOGINPROGRESS_SHOWING_DEVICE_CODE = CAccountsPanel::LOGINPROGRESS_SHOWING_DEVICE_CODE;
    static const TLoginProgress LOGINPROGRESS_DONE = CAccountsPanel::LOGINPROGRESS_DONE;
    static const TLoginProgress LOGINPROGRESS_FAILED = CAccountsPanel::LOGINPROGRESS_FAILED;

    /** Names longer than this (in characters) are cut off with "..." where the form shows them */
    static const UInt32 MaxShownTextLength = CAccountsPanel::MaxShownTextLength;

    CLoginForm( void );

    CLoginForm( GUI::CIGUIContext* context );

    virtual ~CLoginForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window (not modal) with the current state of the manager */
    bool Open( void );

    /** Hides the window and notifies ClosedEvent */
    void Close( void );

    bool IsOpen( void ) const;

    /** The manager the form shows and calls; not owned, it has to outlive the form or be taken away with GUCEF_NULL first */
    void SetAuthManager( AUTH::CAuthManager* manager );

    AUTH::CAuthManager* GetAuthManager( void ) const;

    /** The content the form shares with the account page */
    CAccountsPanel& GetPanel( void );

    /** Starts a login with the provider, as its button does. False when the manager did not start it (the form then shows why) */
    bool Login( const CORE::CString& providerName );

    /** Cancels the login with the provider, as its Cancel button does */
    bool CancelLogin( const CORE::CString& providerName );

    /** Removes the link of the account, as its Unlink button does */
    bool Unlink( const AUTH::CAccountRef& account );

    /** Opens the page of a device code in the browser, as the Open button does. False when there is no such code or the browser was not started */
    bool OpenDeviceCodePage( const CORE::CString& providerName );

    /** Logins ask for a code on another device instead of opening a browser, for a machine without one */
    void SetPreferDeviceCode( bool preferDeviceCode );

    bool GetPreferDeviceCode( void ) const;

    /** The setting autoLogin of the manager, as the check box shows it */
    bool GetAutoLogin( void ) const;

    /** Changes the setting autoLogin of the manager and the check box; notifies AutoLoginChangedEvent when it changed */
    void SetAutoLogin( bool autoLogin );

    /** Makes the content again from the state of the manager and the logins, at once (else it is done at the next frame) */
    void Refresh( void );

    /** The providers shown with a login button, in the order of the form */
    void GetShownProviders( CORE::CString::StringVector& providerNames ) const;

    TLoginProgress GetProgress( const CORE::CString& providerName ) const;

    /** What the form says of the login with the provider: the failure, the account that logged in; empty otherwise */
    CORE::CString GetProgressMessage( const CORE::CString& providerName ) const;

    /** The device code shown for the provider, empty when none is */
    CORE::CString GetDeviceCode( const CORE::CString& providerName ) const;

    /** The page of the device code shown for the provider, empty when none is */
    CORE::CString GetDeviceCodeUrl( const CORE::CString& providerName ) const;

    /** The line with the login state: "Guest", or "Linked: Google, Epic Games" */
    CORE::CString GetStateText( void ) const;

    /** The text at the bottom: what went wrong last, empty when nothing */
    CORE::CString GetErrorText( void ) const;

    /** The text of an account in the list: "Epic Games: 0123456789abcdef0123456789abcdef", cut off at MaxShownTextLength characters */
    static CORE::CString MakeAccountText( const AUTH::CAccountRef& account );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CLoginForm > TEventCallback;

    /** The panel has an error to show */
    void OnPanelErrorChanged( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    /** What the content of the panel shows changed: the form is made again at the next frame */
    void OnPanelChanged( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    /** A frame of the context starts: the form is made again when what it shows changed */
    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void OnCloseActivated( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    /** The check box that logs in when the game starts was changed by the person */
    void OnAutoLoginValueChanged( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    /** Escape closes */
    void OnNavigation( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    CLoginForm( const CLoginForm& src );
    CLoginForm& operator=( const CLoginForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    bool Rebuild( void );
    void MarkDirty( void );
    void SubscribeToPanel( void );
    void ShowError( const CORE::CString& text );

    private:

    CAccountsPanel m_panel;
    CORE::CString m_movedPosition;      /**< where the player put the window, empty when it was not moved */
    bool m_open;
    bool m_dirty;
    bool m_subscribedToFrames;
    bool m_rebuilding;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CLOGINFORM_H ? */
