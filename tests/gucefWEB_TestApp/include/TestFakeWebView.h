/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEW_H
#define GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#include "TestFakeWebSite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeWebViewEngine;

/**
 *  A view of the fake engine: loads pages of the fake site, applies input to them and draws them. Engine thread only,
 *  except the test accessors, which take a lock of their own.
 *
 *  Layout, in logical pixels: a title strip of 24 at the top, the lines of text below it (14 each), then the elements
 *  (fields, buttons, links) at x 16, 300 wide and 28 high, 36 apart. Text is drawn as blocks of 5 x 7 cells per
 *  character, its pattern taken from the character code: no font, the same picture everywhere.
 *
 *  Scripts: the helper expressions of the page queries of a sign-in (element state, focus and clear, value length,
 *  click, submit, contains text, location) by their text, and a few of the tests: "document.title",
 *  "fake:repeat(<n>)" (a string of n characters), "fake:nested(<n>)" (arrays nested n deep), "fake:value(<id>)",
 *  "fake:state", and anything that starts with "throw " (an exception). In the page's own world of the hostile page,
 *  whose scripts replace document.querySelector, the helpers find nothing.
 */
class CFakeWebView : public WEB::CWebViewBase
{
    public:

    CFakeWebView( CFakeWebViewEngine& engine, UInt32 viewId, const WEB::CWebViewSettings& settings, const WEB::CWebViewCapabilities& capabilities );

    virtual ~CFakeWebView();

    /*
     *  For tests: any thread
     */

    GUCEF::vector< WEB::CWebViewInputEvent > GetRecordedInput( void ) const;

    UInt32 GetRecordedInputCount( void ) const;

    /** Whether the challenge page was verified with trusted input */
    bool IsVerified( void ) const;

    /** The id of the element that has the focus, empty for none */
    CORE::CString GetFocusedElement( void ) const;

    /** Answers to the dialogs, decisions and downloads, in the order they came: "alert", "confirm:yes", "prompt:<text>" ... */
    CORE::CString::StringVector GetAnswers( void ) const;

    /** The rectangle of an element in logical pixels of the view, as last laid out */
    bool GetElementRect( const CORE::CString& id, WEB::CWebViewRect& rect ) const;

    /** How many frames the view drew */
    UInt32 GetRenderCount( void ) const;

    bool IsPageVisible( void ) const;

    bool HasPageFocus( void ) const;

    Float32 GetZoom( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnCreated( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnNavigate( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnReload( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnStop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnGoBack( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnGoForward( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnEvaluateScript( const CORE::CString& script, const CORE::CString& world, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnGetCookies( const CORE::CString& urlFilter, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClearCookies( WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetCookie( const WEB::CWebViewCookie& cookie, WEB::CWebViewCommand& command ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetVisible( bool visible ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSetFocus( bool focused ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnInjectInput( const WEB::CWebViewInputEvent& inputEvent ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnSetZoom( Float32 factor ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnDecisionResolved( UInt32 decisionId, WEB::TWebViewDecisionKind kind, bool allow, const CORE::CString& text, bool timedOut ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnEditCommand( WEB::TWebViewEditCommand command, CORE::CString& copiedText ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClose( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void NavigateInternal( const CORE::CString& url, bool fromHistory, bool userGesture );
    void CommitPage( const CORE::CString& url, const CFakeWebSite::TPage& page, bool fromHistory );
    void RunPageLoadActions( void );
    void ShowNextDialog( void );
    void Render( const WEB::CWebViewRect* logicalDirty );
    void Layout( void );
    Int32 HitTest( Int32 x, Int32 y ) const;
    Int32 FindElement( const CORE::CString& selector ) const;
    void Click( Int32 element, bool trusted );
    void Submit( Int32 element );
    void Focus( Int32 element );
    void AddAnswer( const CORE::CString& answer );
    void RedrawElement( Int32 element );
    bool HasSession( void ) const;
    WEB::TWebViewCookieVector& GetJar( void );
    CORE::CString GetPageText( void ) const;
    bool EvaluateHelper( const CORE::CString& script, bool pageWorld, CORE::CString& json, CORE::CString& error );

    CFakeWebView( const CFakeWebView& src );
    CFakeWebView& operator=( const CFakeWebView& src );

    typedef GUCEF::vector< WEB::CWebViewRect > TRectVector;
    typedef GUCEF::map< UInt32, CORE::CString > TDecisionActionMap;

    private:

    CFakeWebViewEngine& m_fakeEngine;
    CFakeWebSite::TPage m_page;
    CORE::CString m_url;
    CORE::CString::StringVector m_history;
    Int32 m_historyIndex;
    IMAGE::CPixelMap m_pixels;
    Int32 m_pressed;
    Int32 m_hovered;
    Int32 m_scrollY;
    UInt32 m_nextDialog;
    UInt32 m_openDialog;
    CORE::CString m_pendingCertificateUrl;
    UInt32 m_certificateDecision;
    TDecisionActionMap m_decisionActions;
    CORE::CString::StringVector m_afterLoad;
    WEB::TWebViewCookieVector m_ephemeralJar;

    mutable MT::CMutex m_testLock;
    GUCEF::vector< WEB::CWebViewInputEvent > m_recorded;
    bool m_verified;
    Int32 m_focused;
    CORE::CString::StringVector m_answers;
    TRectVector m_layout;
    UInt32 m_renders;
    bool m_visible;
    bool m_pageFocus;
    Float32 m_zoom;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTFAKEWEBVIEW_H ? */
