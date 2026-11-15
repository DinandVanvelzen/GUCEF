/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_IWEBVIEW_H
#define GUCEF_WEB_IWEBVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#include "gucefWEB_CWebViewCapabilities.h"
#define GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#endif /* GUCEF_WEB_CWEBVIEWCAPABILITIES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSETTINGS_H
#include "gucefWEB_CWebViewSettings.h"
#define GUCEF_WEB_CWEBVIEWSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWSETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOOKIE_H
#include "gucefWEB_CWebViewCookie.h"
#define GUCEF_WEB_CWEBVIEWCOOKIE_H
#endif /* GUCEF_WEB_CWEBVIEWCOOKIE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#include "gucefWEB_CWebViewInputEvent.h"
#define GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#endif /* GUCEF_WEB_CWEBVIEWINPUTEVENT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWRECT_H
#include "gucefWEB_CWebViewRect.h"
#define GUCEF_WEB_CWEBVIEWRECT_H
#endif /* GUCEF_WEB_CWEBVIEWRECT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebViewFrameExchange;

/**
 *  One web view of an engine: a page that renders into a frame exchange (offscreen) or into a window of the engine's
 *  own, takes injected input and carries out commands.
 *
 *  Threading. Every object of the engine lives on the engine thread (CWebViewEngineHost). Commands may be called from
 *  any thread: they are marshalled to the engine thread in the order they are called and return a CORE::CFutureResult
 *  that completes there (result data: CWebViewCommandResult, see CWebViewCommandResult::TryGetFromFuture()). A thread
 *  that must block (a sign-in task) waits on the future; the GUI thread and the engine thread never do. Events are
 *  notified on the engine thread; an observer on another thread takes them through a mailbox of its own thread
 *  (CWebViewEventRelay) or a CORE::CPumpedObserver. The state getters are snapshots kept by the view and may be read
 *  from any thread. Frames do not travel as events: the frame exchange carries them (GetFrameExchange()).
 *
 *  Deferred decisions. The engine applies the policy of the view at once. Only what the policy marks 'ask' is deferred:
 *  the event carries a decision id and the owner answers with ResolveDecision(); an unanswered decision is refused
 *  after CWebViewSettings::decisionTimeoutMs. The engine thread never waits for the owner.
 *
 *  Privacy. Cookie values, script results, typed text and full addresses during a sign-in are not logged by the views.
 */
class GUCEF_WEB_PUBLIC_CPP IWebView : public CORE::CNotifier
{
    public:

    static const CORE::CEvent LoadStartedEvent;             /**< TUrlEventData */
    static const CORE::CEvent LoadCommittedEvent;           /**< TUrlEventData: the page of the address is the page of the view now */
    static const CORE::CEvent LoadFinishedEvent;            /**< TUrlEventData */
    static const CORE::CEvent LoadFailedEvent;              /**< TWebViewLoadErrorEventData */
    static const CORE::CEvent TitleChangedEvent;            /**< TTextEventData */
    static const CORE::CEvent UrlChangedEvent;              /**< TUrlEventData */
    static const CORE::CEvent ProgressChangedEvent;         /**< TProgressEventData: 0..1 */
    static const CORE::CEvent SecurityStateChangedEvent;    /**< TStateEventData: a TWebViewSecurityState */
    static const CORE::CEvent CursorChangedEvent;           /**< TStateEventData: a TWebViewCursorShape */
    static const CORE::CEvent FrameAvailableEvent;          /**< TWebViewRectEventData: what changed; coalesced, at most one until the frame was taken */
    static const CORE::CEvent NavigationRequestedEvent;     /**< TWebViewNavigationRequestEventData */
    static const CORE::CEvent PopupRequestedEvent;          /**< TWebViewPopupRequestEventData */
    static const CORE::CEvent DownloadRequestedEvent;       /**< TWebViewDownloadRequestEventData */
    static const CORE::CEvent PermissionRequestedEvent;     /**< TWebViewPermissionRequestEventData */
    static const CORE::CEvent CertificateErrorEvent;        /**< TWebViewCertificateErrorEventData */
    static const CORE::CEvent ScriptDialogEvent;            /**< TWebViewScriptDialogEventData */
    static const CORE::CEvent ConsoleMessageEvent;          /**< TWebViewConsoleMessageEventData; only when the settings enable it */
    static const CORE::CEvent ProcessTerminatedEvent;       /**< TStateEventData: a TWebViewTermination */
    static const CORE::CEvent FocusLeaveRequestedEvent;     /**< TFlagEventData: true when the page tabbed past its last element, false past its first */
    static const CORE::CEvent CloseRequestedEvent;          /**< the page called window.close(); no data */
    static const CORE::CEvent EngineStalledEvent;           /**< the engine thread did not answer within the watchdog time; notified on the thread that noticed */
    static const CORE::CEvent ClosedEvent;                  /**< the view is closed (Close(), engine shutdown); no data */

    static void RegisterEvents( void );

    typedef CORE::TCloneableString TUrlEventData;
    typedef CORE::TCloneableString TTextEventData;
    typedef CORE::TCloneableFloat32 TProgressEventData;
    typedef CORE::TCloneableInt32 TStateEventData;
    typedef CORE::TCloneableBool TFlagEventData;
    typedef CORE::CTCloneableObj< CWebViewRect > TWebViewRectEventData;

    IWebView( void );

    virtual ~IWebView();

    /*
     *  Commands: any thread, carried out on the engine thread in order
     */

    /** Completes on commit with the final address (text), fails with the load error. Only http, https, about:blank and data:, checked against the policy */
    virtual CORE::CFutureResult Navigate( const CString& url ) = 0;

    virtual CORE::CFutureResult Reload( void ) = 0;

    virtual CORE::CFutureResult Stop( void ) = 0;

    virtual CORE::CFutureResult GoBack( void ) = 0;

    virtual CORE::CFutureResult GoForward( void ) = 0;

    /** The JSON text of the result (bounded by maxScriptResultBytes) or the exception message (WEBVIEWERROR_SCRIPT). 'world' names an isolated script world; empty is the page's world and is refused unless the policy allows it */
    virtual CORE::CFutureResult EvaluateScript( const CString& script, const CString& world ) = 0;

    /** The cookies of the profile (HttpOnly ones included) that match an address; an empty filter gives all */
    virtual CORE::CFutureResult GetCookies( const CString& urlFilter ) = 0;

    virtual CORE::CFutureResult ClearCookies( void ) = 0;

    virtual CORE::CFutureResult SetCookie( const CWebViewCookie& cookie ) = 0;

    /** The size in logical pixels and the device scale, from the layout of the widget that shows the view */
    virtual CORE::CFutureResult SetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale ) = 0;

    /** An invisible view throttles its rendering */
    virtual CORE::CFutureResult SetVisible( bool visible ) = 0;

    virtual CORE::CFutureResult SetFocus( bool focused ) = 0;

    /** Fire and forget, in order with the other commands */
    virtual void InjectInput( const CWebViewInputEvent& inputEvent ) = 0;

    /** Page zoom: text scale and the fraction of a device scale the image density does not cover */
    virtual CORE::CFutureResult SetZoom( Float32 factor ) = 0;

    /** Answers a deferred decision; 'text' is the answer of a prompt dialog */
    virtual CORE::CFutureResult ResolveDecision( UInt32 decisionId, bool allow, const CString& text = CString::Empty ) = 0;

    /** Cut, copy, paste, select all, undo, redo; copy and cut give the text that was copied (for engines without a clipboard of their own) */
    virtual CORE::CFutureResult ExecuteEditCommand( TWebViewEditCommand command ) = 0;

    /** A copy of the last frame (CWebViewCommandResult::frame) */
    virtual CORE::CFutureResult CaptureFrame( void ) = 0;

    /** Completes when the native view is gone; pending commands fail with WEBVIEWERROR_VIEW_CLOSED */
    virtual CORE::CFutureResult Close( void ) = 0;

    /*
     *  State snapshots: any thread
     */

    virtual CString GetUrl( void ) const = 0;

    virtual CString GetTitle( void ) const = 0;

    /** 0..1 */
    virtual Float32 GetLoadProgress( void ) const = 0;

    virtual bool IsLoading( void ) const = 0;

    virtual TWebViewSecurityState GetSecurityState( void ) const = 0;

    virtual TWebViewCursorShape GetCursorShape( void ) const = 0;

    virtual bool IsClosed( void ) const = 0;

    /** The logical size and scale the view renders at, as of the last SetSize() it carried out */
    virtual void GetSize( UInt32& logicalWidth, UInt32& logicalHeight, Float32& deviceScale ) const = 0;

    /*
     *  Fixed for the life of the view: any thread
     */

    virtual CWebViewFrameExchange& GetFrameExchange( void ) = 0;

    virtual const CWebViewSettings& GetSettings( void ) const = 0;

    virtual const CString& GetEngineName( void ) const = 0;

    virtual CWebViewCapabilities GetCapabilities( void ) const = 0;

    /** Unique within the engine */
    virtual UInt32 GetViewId( void ) const = 0;

    private:

    IWebView( const IWebView& src );                /**< not implemented */
    IWebView& operator=( const IWebView& src );     /**< not implemented */
};

typedef CORE::CTSharedPtr< IWebView, MT::CMutex > TWebViewPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_IWEBVIEW_H ? */
