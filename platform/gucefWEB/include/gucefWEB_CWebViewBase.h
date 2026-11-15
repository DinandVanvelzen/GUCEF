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

#ifndef GUCEF_WEB_CWEBVIEWBASE_H
#define GUCEF_WEB_CWEBVIEWBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#include "gucefWEB_CWebViewFrameExchange.h"
#define GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#endif /* GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMAND_H
#include "gucefWEB_CWebViewCommand.h"
#define GUCEF_WEB_CWEBVIEWCOMMAND_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMAND_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#include "gucefWEB_CWebViewCertificateError.h"
#define GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#endif /* GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H ? */

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

class CWebViewEngineBase;

/**
 *  The part of a view every engine shares, on top of which an engine implements its views.
 *
 *  The commands of IWebView become CWebViewCommands for the engine thread (CWebViewEngineBase posts them to its host).
 *  On the engine thread ExecuteCommand() applies what is common (the policy of a navigation, the script world,
 *  decisions and their time outs, frame capture, closing) and calls the On...() functions an engine implements. Those
 *  run on the engine thread only. The engine reports what its page does through the Notify...(), Request...() and
 *  Publish...() helpers, which keep the state snapshots, apply the policy and notify the events.
 *
 *  Futures: commands with a simple outcome complete when their On...() function returns. Navigate() completes when
 *  the engine reports the commit (NotifyLoadCommitted()) or the failure (NotifyLoadFailed()); a newer navigation
 *  cancels it. Scripts and cookies complete when the engine calls CompleteScript(), CompleteCookies() or
 *  FailCommand(), which may be later (a callback of the engine), still on the engine thread.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewBase : public IWebView
{
    public:

    CWebViewBase( CWebViewEngineBase& engine, UInt32 viewId, const CWebViewSettings& settings, const CWebViewCapabilities& capabilities );

    virtual ~CWebViewBase();

    /*
     *  IWebView: any thread
     */

    virtual CORE::CFutureResult Navigate( const CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult Reload( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult Stop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult GoBack( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult GoForward( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult EvaluateScript( const CString& script, const CString& world ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult GetCookies( const CString& urlFilter ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ClearCookies( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult SetCookie( const CWebViewCookie& cookie ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult SetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult SetVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult SetFocus( bool focused ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void InjectInput( const CWebViewInputEvent& inputEvent ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult SetZoom( Float32 factor ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ResolveDecision( UInt32 decisionId, bool allow, const CString& text = CString::Empty ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult ExecuteEditCommand( TWebViewEditCommand command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult CaptureFrame( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CFutureResult Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetUrl( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Float32 GetLoadProgress( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsLoading( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TWebViewSecurityState GetSecurityState( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TWebViewCursorShape GetCursorShape( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsClosed( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetSize( UInt32& logicalWidth, UInt32& logicalHeight, Float32& deviceScale ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CWebViewFrameExchange& GetFrameExchange( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CWebViewSettings& GetSettings( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CString& GetEngineName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CWebViewCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetViewId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  For CWebViewEngineBase
     */

    /** Engine thread: carries out a command for this view */
    void ExecuteCommand( CWebViewCommand& command );

    /** Engine thread: right after the engine created the view */
    void Created( void );

    /** Engine thread: deadlines of deferred decisions, and OnTick() */
    void Tick( UInt64 nowMs );

    /** Engine thread: how long the engine thread may sleep before Tick() has something to do; CIWebViewEngineLoop::WaitForever for nothing */
    UInt32 GetTickWaitMs( UInt64 nowMs ) const;

    /** Engine thread: closes the view (OnClose()), fails what is pending with 'reason', notifies ClosedEvent */
    void CloseOnEngineThread( TWebViewError reason );

    /** Any thread: the engine stalled; the view is closed for new commands and EngineStalledEvent is notified on this thread */
    void EngineStalled( void );

    /** Engine thread: the number of deferred decisions not answered yet */
    UInt32 GetOpenDecisionCount( void ) const;

    protected:

    /*
     *  What an engine implements: engine thread only
     */

    /** Starts loading; the policy allowed it. Report with NotifyLoadStarted() / NotifyLoadCommitted() / NotifyLoadFailed() */
    virtual void OnNavigate( const CString& url ) = 0;

    virtual bool OnReload( void ) = 0;

    virtual bool OnStop( void ) = 0;

    virtual bool OnGoBack( void ) = 0;

    virtual bool OnGoForward( void ) = 0;

    /** Evaluates in the world (not empty unless the policy allows the page's world); end with CompleteScript() or FailCommand() */
    virtual void OnEvaluateScript( const CString& script, const CString& world, CWebViewCommand& command ) = 0;

    /** End with CompleteCookies() or FailCommand() */
    virtual void OnGetCookies( const CString& urlFilter, CWebViewCommand& command ) = 0;

    /** End with CompleteCommand() or FailCommand() */
    virtual void OnClearCookies( CWebViewCommand& command ) = 0;

    /** End with CompleteCommand() or FailCommand() */
    virtual void OnSetCookie( const CWebViewCookie& cookie, CWebViewCommand& command ) = 0;

    /** The new size in logical pixels and the device scale (GetSize() gives it as well); render a frame of the new size */
    virtual void OnSetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale ) = 0;

    virtual void OnSetVisible( bool visible ) = 0;

    virtual void OnSetFocus( bool focused ) = 0;

    virtual void OnInjectInput( const CWebViewInputEvent& inputEvent ) = 0;

    /** False when the engine has no zoom */
    virtual bool OnSetZoom( Float32 factor ) = 0;

    /** A deferred decision was answered, or timed out (then 'allow' is false) */
    virtual void OnDecisionResolved( UInt32 decisionId, TWebViewDecisionKind kind, bool allow, const CString& text, bool timedOut ) = 0;

    /** False when the engine cannot; copy and cut give the text that was copied */
    virtual bool OnEditCommand( TWebViewEditCommand command, CString& copiedText ) = 0;

    /** The native view goes */
    virtual void OnClose( void ) = 0;

    /** Right after the view was created on the engine thread: a first frame, for example */
    virtual void OnCreated( void );

    /** Every engine thread cycle */
    virtual void OnTick( UInt64 nowMs );

    /*
     *  Helpers for engines: engine thread only
     */

    void CompleteCommand( CWebViewCommand& command, const CString& text = CString::Empty );

    void FailCommand( CWebViewCommand& command, TWebViewError error, const CString& message );

    /** Completes with the JSON of a result, or fails with WEBVIEWERROR_RESULT_TOO_LARGE when it is larger than allowed */
    void CompleteScript( CWebViewCommand& command, const CString& json );

    void CompleteCookies( CWebViewCommand& command, const TWebViewCookieVector& cookies );

    /**
     *  Applies the policy to a navigation and reports it (NavigationRequestedEvent; a refused main frame also as
     *  LoadFailedEvent). WEBVIEWDECISION_ALLOW or WEBVIEWDECISION_REFUSE
     */
    TWebViewDecision CheckNavigation( const CString& url, bool mainFrame, bool userGesture, bool byApplication );

    void NotifyLoadStarted( const CString& url );

    /** The page of the address is the page now: the address, the security state from the scheme, the pending navigation completes */
    void NotifyLoadCommitted( const CString& url );

    void NotifyLoadFinished( const CString& url );

    /** The pending navigation fails with the error */
    void NotifyLoadFailed( const CString& url, TWebViewError error, const CString& message );

    void NotifyTitleChanged( const CString& title );

    void NotifyUrlChanged( const CString& url );

    void NotifyProgress( Float32 progress );

    void NotifyCursorChanged( TWebViewCursorShape shape );

    void NotifySecurityState( TWebViewSecurityState state );

    /** Publishes the changed part of a complete RGBA frame of the page; FrameAvailableEvent when the reader took the previous one */
    bool PublishFrame( const IMAGE::CPixelMap& frame, const CWebViewRect& dirty );

    /** As PublishFrame() for BGRA premultiplied pixels */
    bool PublishFrameBgraPremultiplied( const UInt8* pixels, UInt32 strideInBytes, UInt32 width, UInt32 height, const CWebViewRect& dirty );

    /** Popups: decided by the policy; ASK defers (the id is in the event, the answer comes to OnDecisionResolved()) */
    TWebViewDecision RequestPopup( const CString& url, UInt32& decisionId );

    TWebViewDecision RequestDownload( const CString& url, const CString& suggestedName, UInt32& decisionId );

    TWebViewDecision RequestPermission( TWebViewPermissionKind kind, const CString& origin, UInt32& decisionId );

    /** Never allowed by itself; refused also marks the view WEBVIEWSECURITY_CERTIFICATE_ERROR */
    TWebViewDecision ReportCertificateError( const CWebViewCertificateError& certificateError, UInt32& decisionId );

    TWebViewDecision ShowScriptDialog( TWebViewScriptDialogKind kind, const CString& message, const CString& defaultPromptText, UInt32& decisionId );

    /** Only notified when the settings enable console messages */
    void ReportConsoleMessage( TWebViewConsoleLevel level, const CString& text, const CString& source, UInt32 line );

    void ReportProcessTerminated( TWebViewTermination reason );

    void RequestFocusLeave( bool forward );

    void RequestClose( void );

    /** Registers a deferred decision with the time out of the settings and gives its id */
    UInt32 DeferDecision( TWebViewDecisionKind kind );

    bool HasPendingNavigation( void ) const;

    CWebViewEngineBase& GetEngine( void );

    private:

    CORE::CFutureResult Post( CWebViewCommand& command );

    void PostWithoutResult( CWebViewCommand& command );

    void CancelPendingNavigation( TWebViewError error, const CString& message );

    void SetLoading( bool loading );

    CWebViewBase( const CWebViewBase& src );                /**< not implemented */
    CWebViewBase& operator=( const CWebViewBase& src );     /**< not implemented */

    struct SDecision
    {
        TWebViewDecisionKind kind;
        UInt64 deadlineMs;      /**< 0: never */
    };
    typedef GUCEF::map< UInt32, SDecision > TDecisionMap;

    private:

    CWebViewEngineBase* m_engine;           /**< NULL once closed; guarded by m_stateLock */
    const CString m_engineName;
    const UInt32 m_viewId;
    const CWebViewSettings m_settings;
    const CWebViewCapabilities m_capabilities;
    CWebViewFrameExchange m_frames;
    MT::CTAtomic< UInt32 > m_closed;

    mutable MT::CMutex m_stateLock;
    CString m_url;
    CString m_title;
    Float32 m_progress;
    bool m_loading;
    TWebViewSecurityState m_security;
    TWebViewCursorShape m_cursor;
    UInt32 m_width;
    UInt32 m_height;
    Float32 m_scale;

    // engine thread only
    CORE::CFutureResultSource* m_pendingNavigation;
    TDecisionMap m_decisions;
    UInt32 m_nextDecisionId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWBASE_H ? */
