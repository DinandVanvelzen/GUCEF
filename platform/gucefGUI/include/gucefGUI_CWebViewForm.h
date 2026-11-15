/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CWEBVIEWFORM_H
#define GUCEF_GUI_CWEBVIEWFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWEVENTRELAY_H
#include "gucefWEB_CWebViewEventRelay.h"
#define GUCEF_WEB_CWEBVIEWEVENTRELAY_H
#endif /* GUCEF_WEB_CWEBVIEWEVENTRELAY_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPRESENTATION_H
#include "gucefWEB_CWebViewPresentation.h"
#define GUCEF_WEB_CWEBVIEWPRESENTATION_H
#endif /* GUCEF_WEB_CWEBVIEWPRESENTATION_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebPageView;

/**
 *  A browser surface: a web view (WEB::IWebView) in the middle, the chrome a person needs around it.
 *
 *      navigation bar      Back, Forward, Reload / Stop, the address, the security indicator   (showNavigationBar)
 *      banner              the prompt of the application: "Sign in to ...", "Solve the puzzle to continue"
 *      page                a CWebPageView; a loading line over it until the first frame of a navigation, an error
 *                          panel (the reason, Retry, Back) instead of it when a load failed
 *      status bar          the link or load state (cut with "..." and the whole text as tooltip), the progress  (showStatusBar)
 *      buttons             Cancel                                                                (showDialogButtons)
 *
 *  Build( title ) makes it a window of the context, Build( placeholder ) puts it in the place of a widget. The form owns
 *  no engine: Attach() binds a view, Detach() lets go of it, so a view can move between forms.
 *
 *  The events of the view arrive on the engine thread; the form relays them (WEB::CWebViewEventRelay) at every
 *  FrameStartedEvent of its context and notifies its own events on the GUI thread with the data of the view's events.
 *  Deferred decisions (popups, downloads, permissions, certificate errors, script dialogs) are answered by the owner
 *  of the form with ResolveDecision() while the event is notified; when the owner does not answer, the form asks the
 *  person with a CMessageBoxForm (one at a time) and answers with that.
 *
 *  Keyboard: F6 moves the focus between the page and the chrome, as browsers do; when the page tabs past its last
 *  element the focus goes to the chrome; Escape on the chrome is Cancel (Escape in the page is the page's).
 *  Nothing typed into the page is logged; the address of a page shows no query while the address cannot be edited.
 *  The page asks at least 320 x 240 logical pixels; a window of the form is never smaller than that plus its chrome.
 */
class GUCEF_GUI_PUBLIC_CPP CWebViewForm : public CFormEx
{
    public:

    static const CORE::CEvent TitleChangedEvent;            /**< TCloneableString: the title of the page */
    static const CORE::CEvent UrlChangedEvent;              /**< TCloneableString: the address of the page */
    static const CORE::CEvent LoadStateChangedEvent;        /**< TCloneableBool: loading */
    static const CORE::CEvent LoadFailedEvent;              /**< WEB::TWebViewLoadErrorEventData */
    static const CORE::CEvent PopupRequestedEvent;          /**< WEB::TWebViewPopupRequestEventData */
    static const CORE::CEvent DownloadRequestedEvent;       /**< WEB::TWebViewDownloadRequestEventData */
    static const CORE::CEvent PermissionRequestedEvent;     /**< WEB::TWebViewPermissionRequestEventData */
    static const CORE::CEvent CertificateErrorEvent;        /**< WEB::TWebViewCertificateErrorEventData */
    static const CORE::CEvent ScriptDialogEvent;            /**< WEB::TWebViewScriptDialogEventData */
    static const CORE::CEvent ConsoleMessageEvent;          /**< WEB::TWebViewConsoleMessageEventData */
    static const CORE::CEvent CloseRequestedEvent;          /**< the page called window.close(); the owner closes the form, not in the notification */
    static const CORE::CEvent CancelledEvent;               /**< Cancel, or Escape on the chrome */

    static void RegisterEvents( void );

    static const Int32 NavigationBarHeight = 34;
    static const Int32 StatusBarHeight = 24;
    static const Int32 DialogButtonsHeight = 44;
    static const Int32 WindowTitleHeight = 28;

    CWebViewForm( void );

    explicit CWebViewForm( CIGUIContext* context );

    virtual ~CWebViewForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Web page" );

    /** Makes the form in the place of 'placeholder', which it fills */
    bool Build( CWidget* placeholder );

    /** Shows the view in the form; a view shown before is let go */
    bool Attach( const WEB::TWebViewPtr& view );

    void Detach( void );

    WEB::TWebViewPtr GetView( void ) const;

    /** Navigates the view; a future that has ended with WEBVIEWERROR_VIEW_CLOSED without a view */
    CORE::CFutureResult Navigate( const CString& url );

    /** The banner text; empty hides the banner */
    void SetPrompt( const CString& prompt );

    const CString& GetPrompt( void ) const;

    /** Shows the error panel instead of the page; Retry when 'canRetry' */
    void ShowError( const CString& message, bool canRetry );

    void HideError( void );

    bool IsErrorShown( void ) const;

    bool IsLoadingOverlayShown( void ) const;

    void SetShowNavigationBar( bool show );
    bool GetShowNavigationBar( void ) const;

    void SetShowStatusBar( bool show );
    bool GetShowStatusBar( void ) const;

    void SetShowDialogButtons( bool show );
    bool GetShowDialogButtons( void ) const;

    /** Whether the person may type an address (never in sign-in use) */
    void SetAddressEditable( bool editable );
    bool GetAddressEditable( void ) const;

    /** Whether the page gets the keyboard focus when a view is attached */
    void SetFocusPageOnShow( bool focusPage );
    bool GetFocusPageOnShow( void ) const;

    /** Whether the page zoom follows the text scale of the GUI (kept for when gucefGUI has a text scale) */
    void SetFollowTextScale( bool follow );
    bool GetFollowTextScale( void ) const;

    /** The chrome, banner and prompt of a presentation */
    void ApplyPresentation( const WEB::CWebViewPresentation& presentation );

    /** As Cancel: CancelledEvent */
    void Cancel( void );

    /** Answers a deferred decision of the view; during the form's event the form then does not ask the person */
    bool ResolveDecision( UInt32 decisionId, bool allow, const CString& text = CString::Empty );

    /** The message box the form asks the person with, NULL while it asks nothing */
    CMessageBoxForm* GetOpenPrompt( void );

    /** Decisions waiting for the person, the one asked included */
    UInt32 GetPendingPromptCount( void ) const;

    /** Relays the events of the view and brings the chrome up to date; the form does it every frame itself */
    void Update( void );

    /** The status text in full (the label may show it cut) */
    const CString& GetStatusText( void ) const;

    CWebPageView* GetPageView( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWebViewForm > TEventCallback;

    struct SPrompt
    {
        UInt32 decisionId;
        WEB::TWebViewDecisionKind kind;
        WEB::TWebViewScriptDialogKind dialogKind;
        CString title;
        CString text;
        CString defaultText;
    };
    typedef GUCEF::vector< SPrompt > TPromptVector;
    typedef GUCEF::set< UInt32 > TUInt32Set;

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewLoadStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewLoadCommitted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewLoadFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewLoadFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewTitleChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewUrlChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewProgressChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewSecurityChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewFrameAvailable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewDecisionEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewConsoleMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewCloseRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewFocusLeave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBack( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnForward( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReloadStop( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnAddressEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRetry( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelButton( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPromptClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void SubscribeRelay( void );
    void Relayout( void );
    void SetLoading( bool loading );
    void SetStatusText( const CString& text );
    void FitStatusText( void );
    void ShowAddress( const CString& url );
    void ShowSecurity( WEB::TWebViewSecurityState state );
    void EnsureSecurityImages( void );
    void ShowNextPrompt( void );
    bool FocusChrome( bool forward );
    void SetWidgetVisible( const CString& name, bool visible );

    CWebViewForm( const CWebViewForm& src );
    CWebViewForm& operator=( const CWebViewForm& src );

    private:

    WEB::CWebViewEventRelay m_relay;
    WEB::TWebViewPtr m_view;
    bool m_showNavigationBar;
    bool m_showStatusBar;
    bool m_showDialogButtons;
    bool m_addressEditable;
    bool m_focusPageOnShow;
    bool m_followTextScale;
    CString m_prompt;
    CString m_statusText;
    CString m_failedUrl;
    bool m_loading;
    bool m_frameSinceLoad;
    bool m_errorShown;
    bool m_frameSubscribed;
    Int32 m_laidOutWidth;
    TUInt32Set m_answered;
    TPromptVector m_prompts;
    CMessageBoxForm* m_promptForm;
    bool m_promptOpen;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWEBVIEWFORM_H ? */
