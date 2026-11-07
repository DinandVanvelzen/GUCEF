/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_GAMEGUI_CMAILACCOUNTFORM_H
#define GUCEF_GAMEGUI_CMAILACCOUNTFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#include "gucefCORE_CTScopedValue.h"
#define GUCEF_CORE_CTSCOPEDVALUE_H
#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#include "gucefGUI_CSecretInputSettings.h"
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H
#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */

#ifndef GUCEF_COM_CMAILPROVIDERPRESETS_H
#include "gucefCOM_CMailProviderPresets.h"
#define GUCEF_COM_CMAILPROVIDERPRESETS_H
#endif /* GUCEF_COM_CMAILPROVIDERPRESETS_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a mailbox as the form shows them and hands them on (the data of CMailAccountForm::SaveRequestedEvent and TestRequestedEvent).
 *  The password is in it only as typed, for whoever keeps or uses it: never log this object. The password text of an object is overwritten
 *  when the object is destroyed or assigned to.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CMailAccountFormData : public CORE::CICloneable
{
    public:

    CORE::CString address;
    CORE::CString displayName;
    CORE::CString userName;             /**< empty: the address */
    CORE::CString password;
    bool useImap;                       /**< IMAP (true) or POP3 */
    CORE::CString incomingHost;
    UInt16 incomingPort;                /**< 0: the usual one */
    Int32 incomingSecurity;             /**< 0 SSL/TLS, 1 STARTTLS, 2 none */
    CORE::CString outgoingHost;
    UInt16 outgoingPort;
    Int32 outgoingSecurity;
    CORE::CString mailbox;
    UInt32 pollSeconds;
    bool useIdle;
    Int32 processedAction;              /**< what is done with a game mail once it was handled: 0 leave it in the inbox, 1 copy it to the folder, 2 move it to the folder (it is removed from the inbox) */
    CORE::CString processedFolder;      /**< the folder of processedAction, empty for none */
    bool pop3Delete;                    /**< POP3: a handled game mail is deleted from the server */
    bool autoAccept;                    /**< an invitation to a game the player agreed to be introduced to is accepted by itself */
    CORE::CString providerId;           /**< the id of the provider preset the person works with (CMailProviderPresets), "custom" for none; empty: the provider the address says */
    Int32 authMode;                     /**< 0: the password above, 1: the account is signed in with the provider named below (OAuth 2.0); the password is not used then */
    CORE::CString oauthProvider;        /**< the gucefAUTH provider of authMode 1, empty otherwise */

    CMailAccountFormData( void );
    CMailAccountFormData( const CMailAccountFormData& src );
    virtual ~CMailAccountFormData();
    CMailAccountFormData& operator=( const CMailAccountFormData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The mail account of the player: the address and the name the other players see, the servers to send and to receive (IMAP or POP3) with their
 *  port and security, the user name and the password, the folder that is watched and how often, what is done with a game mail once it was
 *  handled (leave, copy or move to a folder; delete on a POP3 server), and "Test connection". The player code (the
 *  public key that others need to invite the player) is shown to be copied to those who invite. A non-modal window that moves.
 *
 *  The form is guided by the address: it is the first field, and from its domain the form recognizes the provider (a table of
 *  COM::CMailProviderPresets, compiled in defaults that the application can replace), fills in the servers, ports and security, and says in
 *  plain words how that provider signs in (the steps for an app password, or "Sign in with Google / Microsoft" through the OAuth 2.0 client of
 *  gucefAUTH). The sign-in button only exists for a provider the application says it can sign in to (SetOAuthProviders(): a client id is
 *  configured); for the others the text says in plain words that it is not available in this build and what to do instead. An address the
 *  form does not know is the custom provider: the servers are fields to fill in. The servers and the other options are on a second page.
 *
 *  Save notifies SaveRequestedEvent, Test notifies TestRequestedEvent (both with CMailAccountFormData); the application checks the data and says
 *  what is wrong or the result of the test with SetStatus(). What is typed is checked first for what is hostile or malformed (control
 *  characters, a host name that is none, a port that is no number): the status says what to correct and nothing is notified. Empty fields
 *  are for the application to judge. The sign-in button notifies SignInRequestedEvent (the provider name as TCloneableString); the
 *  application starts the sign-in and tells the form with SetSignedIn(). The form does not log the data. Cancel and Escape notify CancelledEvent.
 *
 *  The password is typed in a GUI::CSecretInput: it is drawn as mask characters, kept in memory that is wiped, and wiped when the form is
 *  closed. SetPasswordSettings() changes how strict the field is (the defaults are the strictest); when the settings allow the password to be
 *  shown, a "Show" toggle appears next to the field.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CMailAccountForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent SaveRequestedEvent;       /**< CMailAccountFormData */
    static const CORE::CEvent TestRequestedEvent;       /**< CMailAccountFormData */
    static const CORE::CEvent CancelledEvent;           /**< no data */
    static const CORE::CEvent SignInRequestedEvent;     /**< CORE::TCloneableString: the name of the gucefAUTH provider to sign in to */
    static const CORE::CEvent SignOutRequestedEvent;    /**< no data: the person wants a password instead of the sign-in */

    static void RegisterEvents( void );

    CMailAccountForm( void );

    CMailAccountForm( GUI::CIGUIContext* context );

    virtual ~CMailAccountForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** Fills the fields */
    void SetData( const CMailAccountFormData& data );

    /** The fields as they are now */
    CMailAccountFormData GetData( void );

    /** How the password field treats the password (see GUI::CSecretInputSettings); the defaults are the most secure ones, nothing can be shown or copied */
    void SetPasswordSettings( const GUI::CSecretInputSettings& settings );

    const GUI::CSecretInputSettings& GetPasswordSettings( void ) const;

    /** Wipes the password the form holds and hides it; Close() does it too */
    void ClearSecrets( void );

    /** The public key of the player: what others fill in to invite the player */
    void SetPlayerCode( const CORE::CString& code );

    CORE::CString GetPlayerCode( void );

    /** The table that recognizes addresses; not owned and not changed by the form. NULL: the compiled in defaults */
    void SetProviderPresets( const COM::CMailProviderPresets* presets );

    const COM::CMailProviderPresets& GetProviderPresets( void ) const;

    /** The gucefAUTH providers the application can sign in to (the ones with a client id): "google", "microsoft". Others are shown as not available in this build */
    void SetOAuthProviders( const CORE::CString::StringVector& providers );

    bool IsOAuthAvailable( const CORE::CString& providerName ) const;

    /** The provider the person works with: recognized from the address, or picked in the list */
    CORE::CString GetProviderId( void ) const;

    /** Picks a provider as the person does with the list: fills in the servers and the text. False for an id the table does not have */
    bool SelectProvider( const CORE::CString& id );

    /** The plain words about signing in to the provider that are shown now */
    CORE::CString GetGuidance( void );

    /** Whether the "Sign in with" button is shown, and what it says */
    bool IsSignInShown( void );

    CORE::CString GetSignInText( void );

    /** The account is (or is no longer) signed in with the OAuth provider; 'text' is what is shown instead of the password ("Signed in with Google") */
    void SetSignedIn( bool signedIn, const CORE::CString& text );

    bool IsSignedIn( void ) const;

    /** The two pages: the account (address, provider, password or sign-in) and the servers with the options */
    void ShowServersPage( bool servers );

    bool IsServersPageShown( void ) const;

    /** Checks what is typed for what is hostile or malformed. False with a sentence for the person; empty fields are fine */
    bool Validate( CORE::CString& problem );

    /** As the sign-in button does */
    void RequestSignIn( void );

    /** A line about what happened (a field that is wrong, the result of the test); 'problem' draws it in red */
    void SetStatus( const CORE::CString& text, bool problem );

    CORE::CString GetStatus( void );

    /** While a test runs its button waits */
    void SetTesting( bool testing );

    bool IsTesting( void ) const;

    /** As the buttons do */
    void RequestSave( void );
    void RequestTest( void );
    void Cancel( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMailAccountForm > TEventCallback;

    CMailAccountForm( const CMailAccountForm& src );
    CMailAccountForm& operator=( const CMailAccountForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    void OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTestActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAddressChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderPicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSignOutActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAccountPageActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnServersPageActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRevealChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRevealUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPasswordRevealStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ApplyPasswordSettings( void );
    void FillFromData( const CMailAccountFormData& data );
    void FillProviderList( void );
    Int32 GetProviderIndex( const CORE::CString& id ) const;
    void ApplyPreset( const COM::CMailProviderPreset& preset, bool fillServers );
    void RefreshGuidance( void );
    void ApplyPage( void );
    const COM::CMailProviderPreset* GetSelectedPreset( void ) const;

    GUI::CSecretInputSettings m_passwordSettings;
    bool m_open;
    bool m_testing;
    const COM::CMailProviderPresets* m_presets;
    COM::CMailProviderPresets m_defaultPresets;
    CORE::CString::StringVector m_oauthProviders;
    CORE::CString m_providerId;
    bool m_providerPicked;                      /**< the person picked the provider: an address of an unknown domain does not take it back */
    bool m_signedIn;
    bool m_serversPage;
    bool m_applying;                            /**< the form changes its own fields: their events are not the person's */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CMAILACCOUNTFORM_H ? */
