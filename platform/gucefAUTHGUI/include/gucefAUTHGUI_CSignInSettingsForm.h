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

#ifndef GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H
#define GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H

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

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#include "gucefGUI_CSecretInputSettings.h"
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H
#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H
#include "gucefAUTHGUI_CSignInSettingsState.h"
#define GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H
#endif /* GUCEF_AUTHGUI_CSIGNINSETTINGSSTATE_H ? */

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
 *  The window with the sign in settings of one service that needs a login (a store, a site): whether the person is signed
 *  in, the buttons Sign in and Sign out, the credentials saved for an automatic sign in (a secret field for each slot the
 *  service needs, "Save credentials...", "Forget saved credentials") and the keyring code that protects them
 *  ("Set up keyring code..." or "Change keyring code...", "Lock now"). It knows nothing of a service: the application gives
 *  it a CSignInSettingsState with SetState() whenever something changes, and does the work in its handlers of the events
 *  below, one event for each button.
 *
 *  The credentials are typed in GUI::CSecretInput fields (SetCredentialSettings(): the secure defaults, reveal allowed so
 *  that a typo can be seen): the value is never a property of a widget, the handler of SaveCredentialsRequestedEvent takes
 *  it out of the fields with GetCredential() and the fields are wiped when the handler is done, when the window closes and
 *  when the form goes. Nothing of a credential is logged or shown. A save that has to wait for the keyring code (not set up,
 *  locked) is the application's to carry on: it keeps what it took in memory until the code is given, and wipes it when the
 *  person gives up.
 *
 *  The widths and heights follow the screen: a modal window in the middle, nine tenths of it up to 560 pixels wide, as high as
 *  the number of credential slots needs. Escape closes it.
 *
 *  Widgets: Status, SignIn, SignOut, CredentialsHeader, CredentialsNote, SlotLabel0.., Slot0.. (the fields), SaveCredentials,
 *  ForgetCredentials, KeyringHeader, KeyringNote, KeyringCode, LockNow, ButtonRow, Message, Close.
 */
class GUCEF_AUTHGUI_EXPORT_CPP CSignInSettingsForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent SignInRequestedEvent;             /**< no data: the button Sign in */
    static const CORE::CEvent SignOutRequestedEvent;            /**< no data: the button Sign out */
    static const CORE::CEvent SaveCredentialsRequestedEvent;    /**< no data: Save credentials with every field filled in; GetCredential() has the values until the handler is done */
    static const CORE::CEvent ForgetCredentialsRequestedEvent;  /**< no data: the button Forget saved credentials */
    static const CORE::CEvent SetUpCodeRequestedEvent;          /**< no data: the keyring button while no code is set up */
    static const CORE::CEvent ChangeCodeRequestedEvent;         /**< no data: the keyring button when a code is set up */
    static const CORE::CEvent LockNowRequestedEvent;            /**< no data: the button Lock now */
    static const CORE::CEvent ClosedEvent;                      /**< no data: Close or Escape; the window is closed */

    static void RegisterEvents( void );

    CSignInSettingsForm( void );
    CSignInSettingsForm( GUI::CIGUIContext* context );
    virtual ~CSignInSettingsForm();

    /** Makes the window of the state, hidden */
    bool Build( void );

    /** Shows the window (modal) with the fields empty */
    bool Open( void );

    /** Wipes the fields and hides the window; no event */
    void Close( void );

    bool IsOpen( void ) const;

    /** What the form shows; a change of the credential slots makes the window again (the fields are wiped), anything else only updates what is shown */
    void SetState( const CSignInSettingsState& state );
    const CSignInSettingsState& GetState( void ) const;

    /** The settings of the credential fields (length, mask, reveal, paste, ...) */
    void SetCredentialSettings( const GUI::CSecretInputSettings& settings );
    const GUI::CSecretInputSettings& GetCredentialSettings( void ) const;

    /** @name What the buttons do; each notifies its event when the button is enabled */
    ///@{
    bool SignIn( void );
    bool SignOut( void );
    bool SaveCredentials( void );
    bool ForgetCredentials( void );
    bool KeyringCode( void );
    bool LockNow( void );
    void CloseWindow( void );
    ///@}

    /** For the handler of SaveCredentialsRequestedEvent: the value of the field of the slot; false when there is none */
    bool GetCredential( const CORE::CString& slot, AUTH::CSecretString& value ) const;

    /** Wipes every credential field */
    void ClearSecrets( void );

    /** A line above the buttons, in the colour of an error or of a status */
    void ShowMessage( const CORE::CString& text, bool isError );
    CORE::CString GetMessageText( void ) const;

    /** @name What the form shows */
    ///@{
    CORE::CString GetStatusText( void ) const;                  /**< "Signed in", "Not signed in", "Signing in..." and the note */
    CORE::CString GetCredentialsNoteText( void ) const;
    CORE::CString GetKeyringNoteText( void ) const;
    bool IsActionEnabled( const CORE::CString& widgetName ) const;
    ///@}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CSignInSettingsForm > TEventCallback;

    CSignInSettingsForm( const CSignInSettingsForm& src );
    CSignInSettingsForm& operator=( const CSignInSettingsForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    bool Rebuild( void );
    void ApplyCredentialSettings( void );
    void UpdateState( void );
    static CORE::CString MakeSlotLabel( const CORE::CString& slot );

    void OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignOutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnForgetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyringActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLockActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSlotTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CSignInSettingsState m_state;
    GUI::CSecretInputSettings m_credentialSettings;
    CORE::CString m_messageText;
    bool m_messageIsError;
    bool m_built;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CSIGNINSETTINGSFORM_H ? */
