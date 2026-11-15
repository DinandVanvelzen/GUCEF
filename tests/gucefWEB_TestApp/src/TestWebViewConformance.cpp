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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#include "TestWebViewSupport.h"
#include "TestWebViewEventLog.h"
#include "TestFakeWebView.h"
#include "TestFakeWebViewEngine.h"
#include "TestWebViewSuites.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::WEB;
using namespace GUCEF::WEBVIEWTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Whether the engine can load the pages of the test site: the fake engine has them in memory; real engines get them
 *  over HTTP on the loopback once the test site is served (the step of the first real engine) */
bool
LoadsTestSite( const TWebViewEnginePtr& engine )
{
    return engine->GetEngineName().StartsWith( CFakeWebViewEngine::EngineName );
}

CString
SiteUrl( const CString& path )
{
    return CFakeWebSite::Url( path );
}

CFakeWebView*
FakeOf( const TWebViewPtr& view )
{
    return static_cast< CFakeWebView* >( const_cast< TWebViewPtr& >( view ).GetPointerAlways() );
}

CString
Script( const TWebViewPtr& view, const CString& script )
{
    return AwaitText( const_cast< TWebViewPtr& >( view )->EvaluateScript( script, TestWorld() ) );
}

/** Clicks the middle of an element of a fake page with injected (trusted) input */
void
ClickElement( const TWebViewPtr& view, const CString& id, UInt32 clicks = 1 )
{
    CWebViewRect rect;
    if ( !FakeOf( view )->GetElementRect( id, rect ) )
        return;
    const Int32 x = rect.left + rect.width / 2, y = rect.top + rect.height / 2;
    TWebViewPtr v = view;
    v->InjectInput( CWebViewInputEvent::PointerMove( x, y ) );
    for ( UInt32 i=1; i<=clicks; ++i )
    {
        v->InjectInput( CWebViewInputEvent::PointerButton( x, y, WEBMOUSEBUTTON_LEFT, true, i ) );
        v->InjectInput( CWebViewInputEvent::PointerButton( x, y, WEBMOUSEBUTTON_LEFT, false, i ) );
    }
}

/** Fills a field the way a sign-in does: focus and clear by query, the value as injected text, the length checked */
bool
FillField( const TWebViewPtr& view, const CString& selector, const CString& value )
{
    TWebViewPtr v = view;
    if ( "true" != Script( v, FocusAndClearExpression( selector ) ) )
        return false;
    v->InjectInput( CWebViewInputEvent::Text( value ) );
    return CORE::UInt32ToString( value.Length() ) == Script( v, ValueLengthExpression( selector ) );
}

bool
SignIn( const TWebViewPtr& view )
{
    TWebViewPtr v = view;
    if ( !AwaitOk( v->Navigate( SiteUrl( "/login" ) ) ) )
        return false;
    if ( !FillField( v, "#user", "tester" ) || !FillField( v, "#password", "fake-password" ) )
        return false;
    return "true" == Script( v, SubmitExpression( "#submit" ) );
}

/** The session cookie of a view, false when it has none */
bool
GetSessionCookie( const TWebViewPtr& view, CWebViewCookie& cookie )
{
    CWebViewCommandResult result;
    if ( !AwaitResult( const_cast< TWebViewPtr& >( view )->GetCookies( SiteUrl( "/" ) ), result ) )
        return false;
    for ( UInt32 i=0; i<(UInt32)result.cookies.size(); ++i )
    {
        if ( "sid" == result.cookies[ i ].name )
        {
            cookie = result.cookies[ i ];
            return true;
        }
    }
    return false;
}

}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
RunWebViewConformanceSuite( const TWebViewEnginePtr& engineArgument )
{GUCEF_TRACE;

    TWebViewEnginePtr engine( engineArgument );

    const CString suiteName = "WebView.Conformance." + engine->GetEngineName();
    GUCEF_TESTFW_SUITE_SCOPE( suiteName.C_String() );
    typedef CWebViewCapabilities C;
    const CWebViewCapabilities caps = engine->GetCapabilities();
    const bool site = LoadsTestSite( engine );
    const char* noSite = "the engine cannot load the pages of the test site (served over HTTP from the step of the first real engine on)";

    GUCEF_TESTFW_TESTCASE( "Conformance: a view is created on the engine thread with its settings, its capabilities and a first frame of its size" )
    try
    {
        if ( !engine->IsAvailable() )
            GUCEF_TESTFW_SKIP( "the engine is not available here" );
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings( "conformance", 320, 240 ) );
        ASSERT_TRUE( !view.IsNULL() );
        if ( !view.IsNULL() )
        {
            ASSERT_TRUE( view->GetEngineName() == engine->GetEngineName() && view->GetCapabilities() == caps );
            ASSERT_TRUE( "conformance" == view->GetSettings().profile.profileId && 0 != view->GetViewId() );
            UInt32 w = 0, h = 0;
            Float32 s = 0.0f;
            view->GetSize( w, h, s );
            ASSERT_TRUE( 320 == w && 240 == h && 1.0f == s );
            ASSERT_FALSE( view->IsClosed() || view->IsLoading() );
            if ( caps.Has( C::CAP_OFFSCREEN_FRAMES ) )
            {
                CWebViewCommandResult frame;
                ASSERT_TRUE( AwaitResult( view->CaptureFrame(), frame ) && !frame.frame.IsNULL() );
                ASSERT_TRUE( !frame.frame.IsNULL() && 320 == frame.frame->GetWidthInPixels() && 240 == frame.frame->GetHeightInPixels() );
            }
            ASSERT_TRUE( AwaitOk( view->Close() ) );
        }
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: about:blank loads with its events in order and no security state" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( "about:blank" == AwaitText( view->Navigate( "about:blank" ) ) );
        ASSERT_TRUE( log.WaitFor( IWebView::LoadFinishedEvent ) );
        CORE::CString::StringSet loadEvents;
        loadEvents.insert( "NavigationRequestedEvent" );
        loadEvents.insert( "LoadStartedEvent" );
        loadEvents.insert( "LoadCommittedEvent" );
        loadEvents.insert( "LoadFinishedEvent" );
        CORE::CString::StringVector order = log.Names( loadEvents );
        ASSERT_TRUE( 4 == order.size() && "NavigationRequestedEvent" == order[ 0 ] && "LoadStartedEvent" == order[ 1 ] &&
                     "LoadCommittedEvent" == order[ 2 ] && "LoadFinishedEvent" == order[ 3 ] );
        ASSERT_TRUE( "about:blank" == view->GetUrl() && WEBVIEWSECURITY_NONE == view->GetSecurityState() && !view->IsLoading() );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: navigations the policy refuses fail before they reach the engine, and are reported" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeSignInSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->Navigate( "https://elsewhere.example/" ) ) );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->Navigate( "javascript:alert(1)" ) ) );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->Navigate( "file:///etc/hosts" ) ) );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->Navigate( "https://user:password@app.test/" ) ) );
        ASSERT_TRUE( log.WaitFor( IWebView::LoadFailedEvent, 4 ) );
        CWebViewLoadError failure;
        ASSERT_TRUE( log.GetData< CWebViewLoadError >( IWebView::LoadFailedEvent, 0, failure ) && WEBVIEWERROR_REFUSED_BY_POLICY == failure.error );
        CWebViewNavigationRequest request;
        ASSERT_TRUE( log.GetData< CWebViewNavigationRequest >( IWebView::NavigationRequestedEvent, 0, request ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == request.decision && request.byApplication && request.mainFrame );
        ASSERT_TRUE( 0 == log.Count( IWebView::LoadStartedEvent ) );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: no request leaves the machine: a host outside the loopback and the test site fails" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        const TWebViewError error = AwaitError( view->Navigate( "https://example.com/" ) );
        ASSERT_TRUE( WEBVIEWERROR_NONE != error && WEBVIEWERROR_REFUSED_BY_POLICY != error );
        ASSERT_TRUE( !site || WEBVIEWERROR_NETWORK == error );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: scripts run in an isolated world only; the page's world is refused unless the policy allows it" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->EvaluateScript( "document.title", "" ) ) );
        if ( !caps.Has( C::CAP_SCRIPT_RESULT ) )
            ASSERT_TRUE( WEBVIEWERROR_NOT_SUPPORTED == AwaitError( view->EvaluateScript( "document.title", TestWorld() ) ) );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: script results, exceptions, size bounds, and helpers the page cannot tamper with" )
    try
    {
        if ( !caps.Has( C::CAP_SCRIPT_RESULT | C::CAP_ISOLATED_WORLDS ) )
            GUCEF_TESTFW_SKIP( "the engine has no script results in isolated worlds" );
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        CWebViewSettings settings = MakeTestSettings();
        settings.maxScriptResultBytes = 1024;
        settings.policy.defaultWorldScripts = true;          // to show what the page's world does to the helpers
        TWebViewPtr view = CreateTestView( *engine, settings );
        ASSERT_TRUE( !view.IsNULL() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/login" ) ) ) );
        ASSERT_TRUE( "\"Sign in\"" == Script( view, "document.title" ) );
        ASSERT_TRUE( "\"" + SiteUrl( "/login" ) + "\"" == Script( view, LocationExpression() ) );
        ASSERT_TRUE( "2" == Script( view, ElementStateExpression( "#user" ) ) && "0" == Script( view, ElementStateExpression( "#nothing" ) ) );
        ASSERT_TRUE( "true" == Script( view, ContainsTextExpression( "Sign in to the test site" ) ) && "false" == Script( view, ContainsTextExpression( "absent \"text\"" ) ) );
        CWebViewCommandResult result;
        ASSERT_FALSE( AwaitResult( view->EvaluateScript( "throw new Error('boom')", TestWorld() ), result ) );
        ASSERT_TRUE( WEBVIEWERROR_SCRIPT == result.error && result.text.HasSubstr( "boom" ) >= 0 );
        ASSERT_TRUE( 1000 + 2 == Script( view, "fake:repeat(1000)" ).Length() );
        ASSERT_TRUE( WEBVIEWERROR_RESULT_TOO_LARGE == AwaitError( view->EvaluateScript( "fake:repeat(5000)", TestWorld() ) ) );
        ASSERT_TRUE( 400 == Script( view, "fake:nested(200)" ).Length() );      // deep results come back intact; who parses them bounds the depth
        AwaitOk( view->Close() );

        // no bound
        settings.maxScriptResultBytes = 0;
        view = CreateTestView( *engine, settings );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/login" ) ) ) );
        ASSERT_TRUE( 200000 + 2 == Script( view, "fake:repeat(200000)" ).Length() );

        // the page replaces document.querySelector in its own world: the helpers in the isolated world are not affected
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/hostile" ) ) ) );
        ASSERT_TRUE( "2" == Script( view, ElementStateExpression( "#field" ) ) );
        ASSERT_TRUE( "0" == AwaitText( view->EvaluateScript( ElementStateExpression( "#field" ), "" ) ) );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: cookies set by the site (HttpOnly, Secure, SameSite), read, filtered, cleared; profiles kept apart" )
    try
    {
        if ( !caps.Has( C::CAP_COOKIES ) )
        {
            TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
            CWebViewCommandResult result;
            ASSERT_TRUE( AwaitResult( view->GetCookies( "" ), result ) && result.cookies.empty() );
            CWebViewCookie cookie;
            cookie.name = "x";
            cookie.domain = "app.test";
            ASSERT_TRUE( WEBVIEWERROR_NOT_SUPPORTED == AwaitError( view->SetCookie( cookie ) ) );
            AwaitOk( view->Close() );
            GUCEF_TESTFW_SKIP( "the engine keeps no cookies" );
        }
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        TWebViewPtr view = CreateTestView( *engine, MakeSignInSettings( "cookies-a" ) );
        ASSERT_TRUE( !view.IsNULL() );
        ASSERT_TRUE( SignIn( view ) );
        ASSERT_TRUE( "\"" + SiteUrl( "/account" ) + "\"" == Script( view, LocationExpression() ) );
        CWebViewCookie session;
        ASSERT_TRUE( GetSessionCookie( view, session ) );
        ASSERT_TRUE( session.httpOnly && session.secure && WEBVIEWSAMESITE_LAX == session.sameSite && session.GetValue().StartsWith( "fake-session-" ) );
        CWebViewCommandResult result;
        ASSERT_TRUE( AwaitResult( view->GetCookies( "https://elsewhere.example/" ), result ) && result.cookies.empty() );
        ASSERT_TRUE( AwaitResult( view->GetCookies( "" ), result ) && 1 == result.cookies.size() );

        // another profile has none
        TWebViewPtr other = CreateTestView( *engine, MakeSignInSettings( "cookies-b" ) );
        ASSERT_FALSE( GetSessionCookie( other, session ) );
        ASSERT_TRUE( AwaitOk( other->Navigate( SiteUrl( "/account" ) ) ) );
        ASSERT_TRUE( SiteUrl( "/login" ) == other->GetUrl() );          // no session: sent to sign in
        AwaitOk( other->Close() );

        // the profile keeps its cookies for the next view of it
        AwaitOk( view->Close() );
        view = CreateTestView( *engine, MakeSignInSettings( "cookies-a" ) );
        ASSERT_TRUE( GetSessionCookie( view, session ) );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/account" ) ) ) && SiteUrl( "/account" ) == view->GetUrl() );

        // set by the application, then all cleared
        CWebViewCookie mine;
        mine.name = "preference";
        mine.SetValue( "fake-value" );
        mine.domain = "app.test";
        ASSERT_TRUE( AwaitOk( view->SetCookie( mine ) ) );
        ASSERT_TRUE( AwaitResult( view->GetCookies( "" ), result ) && 2 == result.cookies.size() );
        CWebViewCookie nameless;
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->SetCookie( nameless ) ) );
        ASSERT_TRUE( AwaitOk( view->ClearCookies() ) );
        ASSERT_TRUE( AwaitResult( view->GetCookies( "" ), result ) && result.cookies.empty() );
        AwaitOk( view->Close() );

        // an ephemeral profile leaves nothing behind
        CWebViewSettings ephemeral = MakeSignInSettings( "cookies-e" );
        ephemeral.profile.ephemeral = true;
        view = CreateTestView( *engine, ephemeral );
        ASSERT_TRUE( SignIn( view ) && GetSessionCookie( view, session ) );
        AwaitOk( view->Close() );
        view = CreateTestView( *engine, ephemeral );
        ASSERT_FALSE( GetSessionCookie( view, session ) );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: input is taken in order; injected input is trusted where script clicks are not" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        if ( !site )
        {
            // what the engine cannot show must still be taken without harm, in order with the commands
            view->InjectInput( CWebViewInputEvent::PointerMove( 5, 5 ) );
            view->InjectInput( CWebViewInputEvent::UsLayoutKey( WEBKEYCODE_KEY_A, true ) );
            view->InjectInput( CWebViewInputEvent::Text( "h\xC3\xA9llo \xE2\x82\xAC" ) );
            view->InjectInput( CWebViewInputEvent::Wheel( 5, 5, 0.0f, -1.0f ) );
            ASSERT_TRUE( AwaitOk( view->SetFocus( true ) ) );
            AwaitOk( view->Close() );
            GUCEF_TESTFW_SKIP( noSite );
        }
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/challenge" ) ) ) );
        ASSERT_TRUE( "true" == Script( view, ClickExpression( "#verify" ) ) );
        ASSERT_TRUE( "false" == Script( view, ContainsTextExpression( "Verified" ) ) );        // a script click is not trusted
        ClickElement( view, "verify" );
        ASSERT_TRUE( "true" == Script( view, ContainsTextExpression( "Verified" ) ) );
        ASSERT_TRUE( FakeOf( view )->IsVerified() );

        // text from outside the keyboard layout arrives as typed
        ClickElement( view, "code" );
        const CString typed = "h\xC3\xA9llo \xE2\x82\xAC";
        view->InjectInput( CWebViewInputEvent::Text( typed ) );
        ASSERT_TRUE( "\"" + typed + "\"" == Script( view, "fake:value(#code)" ) );
        view->InjectInput( CWebViewInputEvent::Key( WEBKEYCODE_BACKSPACE, WEBNAMEDKEY_BACKSPACE, GUCEF_NULL, true ) );
        view->InjectInput( CWebViewInputEvent::Key( WEBKEYCODE_BACKSPACE, WEBNAMEDKEY_BACKSPACE, GUCEF_NULL, false ) );
        ASSERT_TRUE( "\"h\xC3\xA9llo \"" == Script( view, "fake:value(#code)" ) );

        // double clicks keep their click counts, the wheel its notches
        ClickElement( view, "code", 2 );
        view->InjectInput( CWebViewInputEvent::Wheel( 10, 10, 0.0f, -2.0f ) );
        view->InjectInput( CWebViewInputEvent::Focus( false ) );
        ASSERT_TRUE( AwaitOk( view->SetFocus( false ) ) );
        GUCEF::vector< CWebViewInputEvent > recorded = FakeOf( view )->GetRecordedInput();
        UInt32 doubleDown = 0, wheel = 0, focusOut = 0;
        for ( UInt32 i=0; i<(UInt32)recorded.size(); ++i )
        {
            if ( WEBVIEWINPUT_POINTER_DOWN == recorded[ i ].kind && 2 == recorded[ i ].clickCount )
                ++doubleDown;
            if ( WEBVIEWINPUT_WHEEL == recorded[ i ].kind && -2.0f == recorded[ i ].wheelNotchesY )
                ++wheel;
            if ( WEBVIEWINPUT_FOCUS_OUT == recorded[ i ].kind )
                ++focusOut;
        }
        ASSERT_TRUE( 1 == doubleDown && 1 == wheel && 1 == focusOut );

        // the cursor follows what is under the pointer
        CWebViewRect rect;
        ASSERT_TRUE( FakeOf( view )->GetElementRect( "verify", rect ) );
        view->InjectInput( CWebViewInputEvent::PointerMove( rect.left + 2, rect.top + 2 ) );
        ASSERT_TRUE( AwaitOk( view->SetFocus( true ) ) && WEBVIEWCURSOR_HAND == view->GetCursorShape() );
        view->InjectInput( CWebViewInputEvent::PointerLeave() );
        ASSERT_TRUE( AwaitOk( view->SetFocus( true ) ) && WEBVIEWCURSOR_ARROW == view->GetCursorShape() );

        // tabbing past the last element asks to give the focus back
        ClickElement( view, "verify" );
        view->InjectInput( CWebViewInputEvent::Key( WEBKEYCODE_TAB, WEBNAMEDKEY_TAB, GUCEF_NULL, true ) );
        ASSERT_TRUE( log.WaitFor( IWebView::FocusLeaveRequestedEvent ) );
        bool forward = false;
        ASSERT_TRUE( log.GetData< bool >( IWebView::FocusLeaveRequestedEvent, 0, forward ) && forward );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: redirects and frames against the allow-list; popups, downloads and permissions by policy" )
    try
    {
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        TWebViewPtr view = CreateTestView( *engine, MakeSignInSettings() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( SiteUrl( "/landing" ) == AwaitText( view->Navigate( SiteUrl( "/redirect" ) ) ) );
        ASSERT_TRUE( WEBVIEWERROR_REFUSED_BY_POLICY == AwaitError( view->Navigate( SiteUrl( "/redirect-out" ) ) ) );   // the site sends it elsewhere
        ASSERT_TRUE( SiteUrl( "/landing" ) == view->GetUrl() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/challenge" ) ) ) );
        ASSERT_TRUE( "true" == Script( view, ContainsTextExpression( "Frame refused" ) ) );
        CWebViewNavigationRequest frameRequest;
        bool foundFrame = false;
        for ( UInt32 i=0; i<log.Count( IWebView::NavigationRequestedEvent ) && !foundFrame; ++i )
            if ( log.GetData< CWebViewNavigationRequest >( IWebView::NavigationRequestedEvent, i, frameRequest ) && !frameRequest.mainFrame )
                foundFrame = true;
        ASSERT_TRUE( foundFrame && WEBVIEWDECISION_REFUSE == frameRequest.decision && "https://challenge.test/frame" == frameRequest.url );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/popup" ) ) ) && AwaitOk( view->Navigate( SiteUrl( "/download" ) ) ) && AwaitOk( view->Navigate( SiteUrl( "/permissions" ) ) ) );
        CORE::CString::StringVector answers = FakeOf( view )->GetAnswers();
        ASSERT_TRUE( 4 == answers.size() && "popup:refused" == answers[ 0 ] && "download:refused" == answers[ 1 ] &&
                     "permission:notifications:denied" == answers[ 2 ] && "permission:geolocation:denied" == answers[ 3 ] );
        AwaitOk( view->Close() );

        // frames of a challenge provider when the policy names it; popups in the same view; asked downloads
        CWebViewSettings settings = MakeSignInSettings();
        settings.policy.subframeHosts.AddEntry( "challenge.test" );
        settings.policy.popups = WEBVIEWPOPUPS_SAMEVIEW;
        settings.policy.downloads = WEBVIEWDOWNLOADS_ASK;
        view = CreateTestView( *engine, settings );
        CTestEventLog log2;
        log2.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/challenge" ) ) ) && "true" == Script( view, ContainsTextExpression( "Frame loaded" ) ) );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/popup" ) ) ) );
        ASSERT_TRUE( log2.WaitFor( IWebView::LoadFinishedEvent, 3 ) && SiteUrl( "/landing" ) == view->GetUrl() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/download" ) ) ) && log2.WaitFor( IWebView::DownloadRequestedEvent ) );
        CWebViewDownloadRequest download;
        ASSERT_TRUE( log2.GetData< CWebViewDownloadRequest >( IWebView::DownloadRequestedEvent, 0, download ) );
        ASSERT_TRUE( WEBVIEWDECISION_ASK == download.decision && "file.bin" == download.suggestedName && 0 != download.decisionId );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( download.decisionId, true ) ) );
        answers = FakeOf( view )->GetAnswers();
        ASSERT_TRUE( !answers.empty() && "download:allowed" == answers.back() );
        AwaitOk( view->Close() );

        // a popup that is asked about
        settings.policy.popups = WEBVIEWPOPUPS_ASK;
        view = CreateTestView( *engine, settings );
        CTestEventLog log3;
        log3.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/popup" ) ) ) && log3.WaitFor( IWebView::PopupRequestedEvent ) );
        CWebViewPopupRequest popup;
        ASSERT_TRUE( log3.GetData< CWebViewPopupRequest >( IWebView::PopupRequestedEvent, 0, popup ) && WEBVIEWDECISION_ASK == popup.decision );
        ASSERT_TRUE( SiteUrl( "/popup" ) == view->GetUrl() );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( popup.decisionId, true ) ) && SiteUrl( "/landing" ) == view->GetUrl() );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: certificate errors are never accepted by themselves; sign-in profiles do not even ask" )
    try
    {
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( WEBVIEWERROR_CERTIFICATE == AwaitError( view->Navigate( "https://bad-cert.test/" ) ) );
        ASSERT_TRUE( WEBVIEWSECURITY_CERTIFICATE_ERROR == view->GetSecurityState() );
        CWebViewCertificateError report;
        ASSERT_TRUE( log.GetData< CWebViewCertificateError >( IWebView::CertificateErrorEvent, 0, report ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == report.decision && "bad-cert.test" == report.host && !report.issuer.IsNULLOrEmpty() );
        AwaitOk( view->Close() );

        CWebViewSettings asking = MakeTestSettings();
        asking.policy.certificateErrors = WEBVIEWCERTERRORS_ASK;
        view = CreateTestView( *engine, asking );
        CTestEventLog log2;
        log2.Watch( view.GetPointerAlways() );
        CORE::CFutureResult navigation = view->Navigate( "https://bad-cert.test/" );
        ASSERT_TRUE( log2.WaitFor( IWebView::CertificateErrorEvent ) );
        ASSERT_TRUE( log2.GetData< CWebViewCertificateError >( IWebView::CertificateErrorEvent, 0, report ) && WEBVIEWDECISION_ASK == report.decision );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( report.decisionId, true ) ) );
        ASSERT_TRUE( AwaitOk( navigation ) && "\"Bad certificate\"" == Script( view, "document.title" ) );
        ASSERT_TRUE( WEBVIEWSECURITY_CERTIFICATE_ERROR == view->GetSecurityState() );
        AwaitOk( view->Close() );

        asking.Sanitize( true );
        view = CreateTestView( *engine, asking );
        ASSERT_TRUE( WEBVIEWERROR_CERTIFICATE == AwaitError( view->Navigate( "https://bad-cert.test/" ) ) );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: script dialogs are deferred to the owner one by one, or refused; console messages only when enabled" )
    try
    {
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        CWebViewSettings settings = MakeTestSettings();
        TWebViewPtr view = CreateTestView( *engine, settings );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/dialogs" ) ) ) );
        CWebViewScriptDialog dialog;
        ASSERT_TRUE( log.WaitFor( IWebView::ScriptDialogEvent, 1 ) && log.GetData< CWebViewScriptDialog >( IWebView::ScriptDialogEvent, 0, dialog ) );
        ASSERT_TRUE( WEBVIEWDIALOG_ALERT == dialog.kind && WEBVIEWDECISION_ASK == dialog.decision );
        ASSERT_TRUE( 1 == log.Count( IWebView::ScriptDialogEvent ) );                 // the next comes after the answer
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( dialog.decisionId, true ) ) );
        ASSERT_TRUE( log.WaitFor( IWebView::ScriptDialogEvent, 2 ) && log.GetData< CWebViewScriptDialog >( IWebView::ScriptDialogEvent, 1, dialog ) && WEBVIEWDIALOG_CONFIRM == dialog.kind );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( dialog.decisionId, false ) ) );
        ASSERT_TRUE( log.WaitFor( IWebView::ScriptDialogEvent, 3 ) && log.GetData< CWebViewScriptDialog >( IWebView::ScriptDialogEvent, 2, dialog ) && WEBVIEWDIALOG_PROMPT == dialog.kind );
        ASSERT_TRUE( "suggested" == dialog.defaultPromptText );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( dialog.decisionId, true, "typed answer" ) ) );
        CORE::CString::StringVector answers = FakeOf( view )->GetAnswers();
        ASSERT_TRUE( 3 == answers.size() && "alert" == answers[ 0 ] && "confirm:refused" == answers[ 1 ] && "prompt:typed answer" == answers[ 2 ] );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/console" ) ) ) && 0 == log.Count( IWebView::ConsoleMessageEvent ) );
        AwaitOk( view->Close() );

        settings.policy.scriptDialogs = WEBVIEWSCRIPTDIALOGS_REFUSE;
        settings.consoleMessages = true;
        view = CreateTestView( *engine, settings );
        CTestEventLog log2;
        log2.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/dialogs" ) ) ) );
        answers = FakeOf( view )->GetAnswers();
        ASSERT_TRUE( 3 == answers.size() && "alert:refused" == answers[ 0 ] && "prompt:refused" == answers[ 2 ] );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/console" ) ) ) && log2.WaitFor( IWebView::ConsoleMessageEvent ) );
        CWebViewConsoleMessage message;
        ASSERT_TRUE( log2.GetData< CWebViewConsoleMessage >( IWebView::ConsoleMessageEvent, 0, message ) && "A message of the page" == message.text );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/close" ) ) ) && log2.WaitFor( IWebView::CloseRequestedEvent ) );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: frames follow size and scale; dirty rectangles inside the frame; at most three buffers for a slow reader" )
    try
    {
        if ( !caps.Has( C::CAP_OFFSCREEN_FRAMES ) )
            GUCEF_TESTFW_SKIP( "the engine has no offscreen frames" );
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        CWebViewCommandResult frame;
        ASSERT_TRUE( AwaitOk( view->SetSize( 320, 240, 2.0f ) ) && AwaitResult( view->CaptureFrame(), frame ) );
        ASSERT_TRUE( 640 == frame.frame->GetWidthInPixels() && 480 == frame.frame->GetHeightInPixels() );
        ASSERT_TRUE( AwaitOk( view->SetSize( 320, 240, 1.25f ) ) && AwaitResult( view->CaptureFrame(), frame ) );
        ASSERT_TRUE( 400 == frame.frame->GetWidthInPixels() && 300 == frame.frame->GetHeightInPixels() );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->SetSize( 0, 240, 1.0f ) ) );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->SetSize( 320, 240, 100.0f ) ) );
        ASSERT_TRUE( AwaitOk( view->SetSize( 320, 240, 1.0f ) ) );

        CWebViewFrameExchange& frames = view->GetFrameExchange();
        CWebViewFrameInfo info;
        frames.TakeLatest( info );
        ASSERT_TRUE( 320 == info.width && 240 == info.height && 1.0f == info.deviceScale );
        ASSERT_TRUE( CWebViewRect::Whole( 320, 240 ).Contains( info.dirty ) );

        // frames the reader does not take: replaced, never queued
        if ( site )
            ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/landing" ) ) ) );
        const UInt32 allocations = frames.GetAllocationCount();
        const UInt32 published = frames.GetPublishedCount();
        for ( UInt32 i=0; i<40; ++i )
            ASSERT_TRUE( AwaitOk( view->Reload() ) );
        ASSERT_TRUE( frames.GetAllocationCount() <= allocations + 3 );           // a reader that never takes costs no memory
        ASSERT_TRUE( frames.GetPublishedCount() >= published + 40 );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: golden frame probes of the colour page within 2 per channel, at scale 1 and 2" )
    try
    {
        if ( !caps.Has( C::CAP_OFFSCREEN_FRAMES ) )
            GUCEF_TESTFW_SKIP( "the engine has no offscreen frames" );
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/colours" ) ) ) );
        const Float32 scales[ 2 ] = { 1.0f, 2.0f };
        for ( UInt32 s=0; s<2; ++s )
        {
            CWebViewCommandResult frame;
            ASSERT_TRUE( AwaitOk( view->SetSize( 320, 240, scales[ s ] ) ) && AwaitResult( view->CaptureFrame(), frame ) );
            const UInt32 w = frame.frame->GetWidthInPixels(), h = frame.frame->GetHeightInPixels();
            const UInt32 probes[ 4 ][ 2 ] = { { w / 4, h / 4 }, { 3 * w / 4, h / 4 }, { w / 4, 3 * h / 4 }, { 3 * w / 4, 3 * h / 4 } };
            for ( UInt32 q=0; q<4; ++q )
            {
                const UInt8* c = CFakeWebSite::QuadrantColours[ q ];
                ASSERT_TRUE( IsColour( PixelOf( *frame.frame, probes[ q ][ 0 ], probes[ q ][ 1 ] ), c[ 0 ], c[ 1 ], c[ 2 ] ) );
            }
        }
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: history back and forward, reload, stop; a crashed web process is reported" )
    try
    {
        if ( !site )
            GUCEF_TESTFW_SKIP( noSite );
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        ASSERT_TRUE( AwaitOk( view->Navigate( SiteUrl( "/" ) ) ) && AwaitOk( view->Navigate( SiteUrl( "/landing" ) ) ) );
        ASSERT_TRUE( AwaitOk( view->GoBack() ) && SiteUrl( "/" ) == view->GetUrl() );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->GoBack() ) );
        ASSERT_TRUE( AwaitOk( view->GoForward() ) && SiteUrl( "/landing" ) == view->GetUrl() );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->GoForward() ) );
        ASSERT_TRUE( AwaitOk( view->Reload() ) && AwaitOk( view->Stop() ) && !view->IsLoading() );
        ASSERT_TRUE( "\"Landing\"" == Script( view, "document.title" ) && "Landing" == view->GetTitle() && 1.0f == view->GetLoadProgress() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( WEBVIEWERROR_PROCESS_ENDED == AwaitError( view->Navigate( SiteUrl( "/crash" ) ) ) );
        Int32 reason = -1;
        ASSERT_TRUE( log.GetData< Int32 >( IWebView::ProcessTerminatedEvent, 0, reason ) && WEBVIEWTERMINATION_CRASH == reason );
        AwaitOk( view->Close() );
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Conformance: closing with commands pending ends them; commands after the close fail; 500 create and close cycles" )
    try
    {
        TWebViewPtr view = CreateTestView( *engine, MakeTestSettings() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        CORE::CFutureResult before = view->SetFocus( true );
        CORE::CFutureResult close = view->Close();
        ASSERT_TRUE( AwaitOk( close ) && AwaitOk( before ) );
        ASSERT_TRUE( view->IsClosed() && 1 == log.Count( IWebView::ClosedEvent ) );
        ASSERT_TRUE( WEBVIEWERROR_VIEW_CLOSED == AwaitError( view->Reload() ) );
        ASSERT_TRUE( WEBVIEWERROR_VIEW_CLOSED == AwaitError( view->Close() ) );
        view->InjectInput( CWebViewInputEvent::PointerMove( 1, 1 ) );            // dropped quietly

        const UInt32 viewsBefore = engine->GetViewCount();
        const UInt64 start = MT::PrecisionTickCount();
        UInt32 created = 0;
        for ( UInt32 i=0; i<500; ++i )
        {
            TWebViewPtr cycle = CreateTestView( *engine, MakeTestSettings( "cycles", 64, 48 ) );
            if ( cycle.IsNULL() )
                break;
            ++created;
            AwaitOk( cycle->Close() );
        }
        ASSERT_TRUE( 500 == created );
        ASSERT_TRUE( viewsBefore == engine->GetViewCount() );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Conformance " << engine->GetEngineName() << ": 500 views created and closed in " << MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) << " ms" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWebViewConformanceTests( void )
{GUCEF_TRACE;

    // every engine the plugins registered (absent engines are named in the log, as the PostgreSQL tests do)
    CWebViewEngineRegistry::TEngineVector engines = CWebViewGlobal::Instance()->GetEngineRegistry().GetEngines();
    const char* expected[] = { "Null", "WebKitGTK", "WPE", "WebView2", "CEF", "WKWebView" };
    for ( UInt32 i=0; i<(UInt32)( sizeof( expected ) / sizeof( expected[ 0 ] ) ); ++i )
        if ( !CWebViewGlobal::Instance()->GetEngineRegistry().IsRegistered( expected[ i ] ) )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebView." << expected[ i ] << ": skipped, engine not loaded" );
    for ( UInt32 i=0; i<(UInt32)engines.size(); ++i )
        RunWebViewConformanceSuite( engines[ i ] );

    // and the fake engine, a fresh one
    TWebViewEnginePtr fake( GUCEF_NEW CFakeWebViewEngine() );
    RunWebViewConformanceSuite( fake );
    fake->Shutdown( 5000 );
}

/*-------------------------------------------------------------------------*/
