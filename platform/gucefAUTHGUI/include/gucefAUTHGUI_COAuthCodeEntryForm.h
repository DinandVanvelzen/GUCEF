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

#ifndef GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H
#define GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H

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

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

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
 *  The window of a login whose provider shows the authorization code on a page instead of redirecting back to the
 *  application (gucefAUTH, redirect mode "code-entry" or "manual"): the person opens the authorization page, signs in
 *  and allows the application there, and the page shows a code that is pasted here.
 *
 *  - The address of the authorization page (Open() gets it from CAuthActionRequired::url) is shown in a read only field
 *    that selects all on focus. "Copy address" puts it on the clipboard of the GUI and "Open in browser" hands it to the
 *    system browser through CORE::OpenUrlInSystemBrowser(), which only takes http and https addresses without user
 *    information and without characters that break out of a command line; for any other address the button is disabled
 *    and the person copies it. The address is shown, never logged.
 *  - The code is typed or pasted in a GUI::CSecretInput (SetCodeSettings(): paste and reveal allowed by default, as a
 *    code is long and is copied from a page): the value is never a property of a widget, it is taken out of the field when
 *    the person submits and the field is wiped then, when the window closes and when the form goes. Nothing of a code is
 *    logged or shown. The text may also be the whole page the provider shows (a JSON object with the member
 *    authorizationCode), as IAuthFlowEngine::SubmitAuthorizationCode() takes it.
 *  - With an engine (SetEngine(), not owned) the form hands the code to IAuthFlowEngine::SubmitAuthorizationCode() and
 *    shows what became of it, each outcome in its own words: taken (the window closes with DoneEvent, the login goes on and
 *    ends through the events of the manager), no login waiting, more than one login waiting (nothing was sent anywhere),
 *    empty, not a code, a page without the member, refused. Without an engine an application takes the code in its handler
 *    of CodeSubmittedEvent (GetCode()) and reports with ShowCheck().
 *  - ShowMessage() lets the application say more afterwards, such as that the provider refused the code at its token
 *    endpoint; Open() again then asks for a new code.
 *
 *  It is a modal window in the middle of the screen, nine tenths of it up to 640 pixels wide and as high as it needs. Enter
 *  in the code field submits, Escape cancels.
 *
 *  Widgets: Prompt, Note, UrlLabel, Url, CopyUrl, OpenUrl, CodeLabel, Code, Paste, ButtonRow, Message, Submit, Cancel.
 */
class GUCEF_AUTHGUI_EXPORT_CPP COAuthCodeEntryForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent CodeSubmittedEvent;   /**< no data: the person submitted a code that is not empty, before anything is done with it; GetCode() has it */
    static const CORE::CEvent CancelledEvent;       /**< no data: Cancel or Escape; the window is closed */
    static const CORE::CEvent DoneEvent;            /**< no data: a login took the code; the window is closed */

    static void RegisterEvents( void );

    COAuthCodeEntryForm( void );
    COAuthCodeEntryForm( GUI::CIGUIContext* context );
    virtual ~COAuthCodeEntryForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window (modal) for the login with the provider, the code field empty; 'authorizationUrl' is the page the person goes to */
    bool Open( const CORE::CString& providerDisplayName, const CORE::CString& authorizationUrl );

    /** Wipes the field and hides the window; no event */
    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetProviderDisplayName( void ) const;
    const CORE::CString& GetAuthorizationUrl( void ) const;

    /** The engine the code is handed to; not owned, it has to outlive the form or be taken away with GUCEF_NULL first */
    void SetEngine( AUTH::IAuthFlowEngine* engine );
    AUTH::IAuthFlowEngine* GetEngine( void ) const;

    /** The settings of the code field (length, mask, reveal, paste, ...); the defaults of this form allow paste and reveal */
    void SetCodeSettings( const GUI::CSecretInputSettings& settings );
    const GUI::CSecretInputSettings& GetCodeSettings( void ) const;

    /** @name What the buttons do */
    ///@{

    /** As the button Submit: checks that a code was typed, notifies CodeSubmittedEvent and, with an engine, hands the code over and shows the outcome. False when nothing was sent */
    bool Submit( void );

    /** As the button Cancel: closes and notifies CancelledEvent */
    void Cancel( void );

    /** As the button Copy address: false when the clipboard is not available */
    bool CopyUrl( void );

    /** As the button Open in browser: false when the address is not one that may be handed to the browser, or the system refused */
    bool OpenUrl( void );

    /** As the button Paste: puts the text on the clipboard into the code field; false when there is none or it does not fit */
    bool PasteCode( void );
    ///@}

    /** @name For the handler of CodeSubmittedEvent of an application without an engine */
    ///@{

    /** The code typed, or the page pasted; false when there is none */
    bool GetCode( AUTH::CSecretString& code ) const;

    /** Shows what became of the code: CODECHECK_ACCEPTED closes the window with DoneEvent, the others show their message and keep it open */
    void ShowCheck( AUTH::TCodeCheck check );
    ///@}

    /** A line above the buttons, in the colour of an error or of a status, for what the application knows more */
    void ShowMessage( const CORE::CString& text, bool isError );

    /** The line above the buttons; empty for nothing */
    CORE::CString GetMessageText( void ) const;

    /** The words that tell the person what became of a code, empty for CODECHECK_ACCEPTED */
    static CORE::CString CheckToText( AUTH::TCodeCheck check );

    /** Whether "Open in browser" would take the address */
    bool CanOpenUrl( void ) const;

    /** Wipes the code field */
    void ClearSecrets( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< COAuthCodeEntryForm > TEventCallback;

    COAuthCodeEntryForm( const COAuthCodeEntryForm& src );
    COAuthCodeEntryForm& operator=( const COAuthCodeEntryForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    bool Rebuild( void );
    void ApplyCodeSettings( void );
    void UpdateState( void );
    bool ReadCode( AUTH::CSecretString& value ) const;
    void FinishDone( void );

    void OnSubmitActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCopyActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPasteActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCodeTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    AUTH::IAuthFlowEngine* m_engine;
    GUI::CSecretInputSettings m_codeSettings;
    CORE::CString m_providerName;
    CORE::CString m_url;
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

#endif /* GUCEF_AUTHGUI_COAUTHCODEENTRYFORM_H ? */
