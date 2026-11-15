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
#include <string.h>

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWebViewTypeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "WebView.Types" );

    GUCEF_TESTFW_TESTCASE( "Rect: union, intersection, clipping and containment" )
    try
    {
        CWebViewRect a( 10, 10, 20, 20 );
        CWebViewRect b( 25, 5, 10, 10 );
        CWebViewRect u = a;
        u.Unite( b );
        ASSERT_TRUE( CWebViewRect( 10, 5, 25, 25 ) == u );
        ASSERT_TRUE( CWebViewRect( 25, 10, 5, 5 ) == a.Intersection( b ) );
        ASSERT_TRUE( a.Intersection( CWebViewRect( 100, 100, 5, 5 ) ).IsEmpty() );
        CWebViewRect empty;
        u = empty;
        u.Unite( a );
        ASSERT_TRUE( u == a );
        u.Unite( empty );
        ASSERT_TRUE( u == a );
        CWebViewRect c( -5, -5, 20, 20 );
        ASSERT_TRUE( c.ClipTo( 10, 8 ) && CWebViewRect( 0, 0, 10, 8 ) == c );
        CWebViewRect d( 50, 50, 5, 5 );
        ASSERT_FALSE( d.ClipTo( 10, 10 ) );
        ASSERT_TRUE( a.Contains( 10, 10 ) && !a.Contains( 30, 30 ) && a.Contains( CWebViewRect( 12, 12, 2, 2 ) ) && !a.Contains( b ) );
        ASSERT_TRUE( 400 == a.GetArea() && 0 == empty.GetArea() );
        ASSERT_TRUE( "10,10 20x20" == a.ToString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Capabilities: flags, what is missing, names both ways" )
    try
    {
        typedef CWebViewCapabilities C;
        C caps( C::CAP_COOKIES | C::CAP_SCRIPT_RESULT | C::CAP_OFFSCREEN_FRAMES );
        ASSERT_TRUE( caps.Has( C::CAP_COOKIES | C::CAP_SCRIPT_RESULT ) );
        ASSERT_FALSE( caps.Has( C::CAP_COOKIES | C::CAP_ISOLATED_WORLDS ) );
        ASSERT_TRUE( C::CAP_ISOLATED_WORLDS == caps.GetMissing( C::CAP_COOKIES | C::CAP_ISOLATED_WORLDS ) );
        ASSERT_TRUE( "OffscreenFrames,Cookies,ScriptResult" == caps.ToString() );
        UInt32 unknown = 99;
        ASSERT_TRUE( caps.GetFlags() == C::FlagsFromString( " cookies, ScriptResult ,OffscreenFrames", &unknown ) && 0 == unknown );
        ASSERT_TRUE( C::CAP_ZOOM == C::FlagsFromString( "Zoom,Teleport", &unknown ) && 1 == unknown );
        caps.Set( C::CAP_COOKIES, false );
        ASSERT_FALSE( caps.Has( C::CAP_COOKIES ) );
        UInt32 all = 0;
        for ( UInt32 i=0; i<C::CapabilityCount; ++i )
        {
            ASSERT_FALSE( CORE::CString( C::GetFlagName( 1u << i ) ).IsNULLOrEmpty() );
            all |= ( 1u << i );
        }
        ASSERT_TRUE( all == C::FlagsFromString( C::FlagsToString( all ) ) );
        ASSERT_TRUE( CORE::CString( C::GetFlagName( 3 ) ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Host allow-list: exact hosts, domains and below, schemes, ports" )
    try
    {
        CWebViewHostAllowList list;
        ASSERT_TRUE( list.AddEntry( "accounts.example.com" ) );
        ASSERT_TRUE( list.AddEntry( ".store.test" ) );
        ASSERT_TRUE( list.AddEntry( "https://secure.test" ) );
        ASSERT_TRUE( list.AddEntry( "http://127.0.0.1:41234" ) );
        ASSERT_TRUE( list.AddEntry( "ports.test:8443" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://accounts.example.com/login?x=1" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "http://accounts.example.com/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https://www.accounts.example.com/" ) );           // exact: no hosts below
        ASSERT_FALSE( list.IsUrlAllowed( "https://example.com/" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://store.test/" ) );                           // the domain itself
        ASSERT_TRUE( list.IsUrlAllowed( "https://a.b.store.test/x" ) );                      // and below
        ASSERT_FALSE( list.IsUrlAllowed( "https://evilstore.test/" ) );                      // no suffix tricks
        ASSERT_FALSE( list.IsUrlAllowed( "https://store.test.evil.test/" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://secure.test/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "http://secure.test/" ) );                          // https only
        ASSERT_TRUE( list.IsUrlAllowed( "http://127.0.0.1:41234/callback?code=1" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "http://127.0.0.1:41235/callback" ) );              // another port
        ASSERT_FALSE( list.IsUrlAllowed( "http://127.0.0.1/callback" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://ports.test:8443/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https://ports.test/" ) );                           // no port: the default port only
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.example.com:444/" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://accounts.example.com:443/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "ftp://accounts.example.com/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "javascript:alert(1)" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "data:text/html,hello" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Host allow-list: normalisation and hostile addresses (case, trailing dot, user information, IDN, wildcards)" )
    try
    {
        CWebViewHostAllowList list;
        ASSERT_TRUE( list.AddEntry( "Accounts.Example.COM." ) );
        ASSERT_TRUE( list.IsUrlAllowed( "HTTPS://ACCOUNTS.example.com/" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://accounts.example.com./" ) );               // the root dot
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.example.com../" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https://user:pass@accounts.example.com/" ) );     // user information: refused
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.example.com@evil.test/" ) );     // the host is evil.test
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.ex\xC3\xA4mple.com/" ) );        // not the ASCII form of the engine
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts%2eexample.com/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https:/accounts.example.com/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.example.com:99999/" ) );
        ASSERT_FALSE( list.IsUrlAllowed( "https://accounts.example.com:/" ) );
        ASSERT_TRUE( list.IsUrlAllowed( "https://accounts.example.com\\@evil.test" ) );            // a backslash ends the host, as browsers read it

        // patterns that are no patterns
        ASSERT_FALSE( list.AddEntry( "*.example.com" ) );
        ASSERT_FALSE( list.AddEntry( "example.*" ) );
        ASSERT_FALSE( list.AddEntry( "user@example.com" ) );
        ASSERT_FALSE( list.AddEntry( "https://example.com/path" ) );
        ASSERT_FALSE( list.AddEntry( "ftp://example.com" ) );
        ASSERT_FALSE( list.AddEntry( "a..b" ) );
        ASSERT_FALSE( list.AddEntry( ".127.0.0.1" ) );                                       // no "and below" for addresses
        ASSERT_FALSE( list.AddEntry( "" ) );
        ASSERT_TRUE( 1 == list.GetEntries().size() );

        CWebViewHostAllowList ipv6;
        ASSERT_TRUE( ipv6.AddEntry( "http://[::1]:8080" ) );
        ASSERT_TRUE( ipv6.IsUrlAllowed( "http://[::1]:8080/x" ) );
        ASSERT_FALSE( ipv6.IsUrlAllowed( "http://[::2]:8080/x" ) );

        CString scheme, host;
        UInt16 port = 0;
        bool userInfo = false;
        ASSERT_TRUE( CWebViewHostAllowList::SplitUrl( "HTTP://Example.COM:8080/a?b#c", scheme, host, port, userInfo ) );
        ASSERT_TRUE( "http" == scheme && "example.com" == host && 8080 == port && !userInfo );
        ASSERT_TRUE( CWebViewHostAllowList::SplitUrl( "https://example.com", scheme, host, port, userInfo ) && 443 == port );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Host allow-list: saved and loaded as patterns, invalid ones left out" )
    try
    {
        CWebViewHostAllowList list;
        list.AddEntry( "a.test" );
        list.AddEntry( ".b.test" );
        list.AddEntry( "https://c.test:8443" );
        CORE::CDataNode node;
        ASSERT_TRUE( list.SaveConfig( node ) );
        CWebViewHostAllowList loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( list.GetPatterns() == loaded.GetPatterns() );
        ASSERT_TRUE( loaded.IsUrlAllowed( "https://x.b.test/" ) && loaded.IsUrlAllowed( "https://c.test:8443/" ) );
        CORE::CDataNode bad;
        bad.SetAttribute( "hosts", "good.test,*.bad.test,also-good.test" );
        loaded.LoadConfig( bad );
        ASSERT_TRUE( 2 == loaded.GetEntries().size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Policy: navigation by scheme, allow-list and frames; decisions; sanitised for sign-in" )
    try
    {
        CWebViewPolicy policy;
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "https://anything.test/", true, false ) );
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "about:blank", true, false ) );
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "data:text/html,x", true, true ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "data:text/html,x", true, false ) );   // only the application's own
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "data:text/html,x", false, true ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "file:///etc/passwd", true, true ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "javascript:alert(1)", true, true ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "https://u:p@anything.test/", true, true ) );

        policy.navigation = WEBVIEWNAVIGATION_ALLOWLIST;
        policy.allowList.AddEntry( "app.test" );
        policy.subframeHosts.AddEntry( "challenge.test" );
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "https://app.test/login", true, false ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "https://other.test/", true, false ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideNavigation( "https://challenge.test/frame", true, false ) );  // not as a page
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "https://challenge.test/frame", false, false ) ); // as a frame
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecideNavigation( "https://app.test/frame", false, false ) );

        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecidePopup( "https://app.test/x" ) );
        policy.popups = WEBVIEWPOPUPS_SAMEVIEW;
        ASSERT_TRUE( WEBVIEWDECISION_ALLOW == policy.DecidePopup( "https://app.test/x" ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecidePopup( "https://other.test/x" ) );
        policy.popups = WEBVIEWPOPUPS_ASK;
        ASSERT_TRUE( WEBVIEWDECISION_ASK == policy.DecidePopup( "https://app.test/x" ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecidePopup( "https://other.test/x" ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideDownload() && WEBVIEWDECISION_REFUSE == policy.DecidePermission( WEBVIEWPERMISSION_CAMERA ) );
        policy.downloads = WEBVIEWDOWNLOADS_ASK;
        policy.permissions = WEBVIEWPERMISSIONS_ASK;
        ASSERT_TRUE( WEBVIEWDECISION_ASK == policy.DecideDownload() && WEBVIEWDECISION_ASK == policy.DecidePermission( WEBVIEWPERMISSION_CAMERA ) );
        ASSERT_TRUE( WEBVIEWDECISION_REFUSE == policy.DecideCertificateError() );
        policy.certificateErrors = WEBVIEWCERTERRORS_ASK;
        ASSERT_TRUE( WEBVIEWDECISION_ASK == policy.DecideCertificateError() );
        ASSERT_TRUE( WEBVIEWDECISION_ASK == policy.DecideScriptDialog() );
        ASSERT_FALSE( policy.IsScriptWorldAllowed( "" ) );
        ASSERT_TRUE( policy.IsScriptWorldAllowed( "gucef-1" ) );

        policy.defaultWorldScripts = true;
        ASSERT_TRUE( policy.Sanitize( false ) );
        ASSERT_FALSE( policy.Sanitize( true ) );                 // a sign-in: no asking about certificates, no page world
        ASSERT_TRUE( WEBVIEWCERTERRORS_REFUSE == policy.certificateErrors && !policy.defaultWorldScripts );
        ASSERT_TRUE( policy.Sanitize( true ) );

        CORE::CDataNode node;
        ASSERT_TRUE( policy.SaveConfig( node ) );
        CWebViewPolicy loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( WEBVIEWNAVIGATION_ALLOWLIST == loaded.navigation && WEBVIEWPOPUPS_ASK == loaded.popups && WEBVIEWDOWNLOADS_ASK == loaded.downloads );
        ASSERT_TRUE( loaded.allowList.GetPatterns() == policy.allowList.GetPatterns() && loaded.subframeHosts.GetPatterns() == policy.subframeHosts.GetPatterns() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: profile ids, sizes and scales sanitised, sign-in safe, folders per engine, saved and loaded" )
    try
    {
        ASSERT_TRUE( CWebViewProfileSettings::IsValidProfileId( "signin-fab.store_1" ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( "" ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( ".." ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( "../etc" ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( "a/b" ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( "with space" ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( "\xC3\xA4" ) );
        CORE::CString longest;
        for ( UInt32 i=0; i<64; ++i )
            longest += "a";
        ASSERT_TRUE( CWebViewProfileSettings::IsValidProfileId( longest ) );
        ASSERT_FALSE( CWebViewProfileSettings::IsValidProfileId( longest + "b" ) );

        CWebViewProfileSettings profile;
        profile.profileId = "../escape";
        profile.proxyMode = WEBVIEWPROXY_EXPLICIT;
        ASSERT_FALSE( profile.Sanitize() );
        ASSERT_TRUE( "default" == profile.profileId && WEBVIEWPROXY_NONE == profile.proxyMode );
        profile.profileId = "signin-store";
        const CString folder = profile.ResolveDataDirectory( "WebKitGTK", "AppName" );
        ASSERT_TRUE( folder.HasSubstr( "WebKitGTK" ) > 0 && folder.HasSubstr( "signin-store" ) > 0 && folder.HasSubstr( "AppName" ) > 0 && folder.HasChar( '$' ) < 0 );
        profile.ephemeral = true;
        ASSERT_TRUE( profile.ResolveDataDirectory( "WebKitGTK", "AppName" ).IsNULLOrEmpty() );

        CWebViewSettings settings;
        ASSERT_TRUE( !settings.inspector && !settings.consoleMessages && WEBVIEWHWACCEL_NEVER == settings.hardwareAcceleration );
        ASSERT_TRUE( CWebViewSettings::DefaultMaxScriptResultBytes == settings.maxScriptResultBytes );
        settings.width = 0;
        settings.height = 99999;
        settings.deviceScale = 20.0f;
        ASSERT_FALSE( settings.Sanitize( false ) );
        ASSERT_TRUE( 1 == settings.width && CWebViewSettings::MaxSize == settings.height && 8.0f == settings.deviceScale );
        settings.deviceScale = (Float32) sqrt( -1.0 );
        settings.Sanitize( false );
        ASSERT_TRUE( 1.0f == settings.deviceScale );
        settings.deviceScale = 0.01f;
        settings.Sanitize( false );
        ASSERT_TRUE( 0.25f == settings.deviceScale );
        settings.inspector = true;
        settings.consoleMessages = true;
        settings.policy.certificateErrors = WEBVIEWCERTERRORS_ASK;
        ASSERT_TRUE( settings.Sanitize( false ) && settings.inspector );        // allowed outside sign-in
        ASSERT_FALSE( settings.Sanitize( true ) );
        ASSERT_TRUE( !settings.inspector && !settings.consoleMessages && WEBVIEWCERTERRORS_REFUSE == settings.policy.certificateErrors );

        UInt32 w = 0, h = 0;
        CWebViewSettings::ToDeviceSize( 320, 240, 1.25f, w, h );
        ASSERT_TRUE( 400 == w && 300 == h );
        CWebViewSettings::ToDeviceSize( 333, 101, 1.5f, w, h );
        ASSERT_TRUE( 500 == w && 152 == h );

        settings.width = 800;
        settings.height = 600;
        settings.deviceScale = 2.0f;
        settings.userAgent = "Compatible/1.0";
        settings.maxScriptResultBytes = 0;
        settings.profile.profileId = "p1";
        settings.policy.navigation = WEBVIEWNAVIGATION_ALLOWLIST;
        settings.policy.allowList.AddEntry( "app.test" );
        CORE::CDataNode node;
        ASSERT_TRUE( settings.SaveConfig( node ) );
        CWebViewSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( 800 == loaded.width && 600 == loaded.height && 2.0f == loaded.deviceScale && "Compatible/1.0" == loaded.userAgent );
        ASSERT_TRUE( 0 == loaded.maxScriptResultBytes && "p1" == loaded.profile.profileId && loaded.policy.allowList.IsUrlAllowed( "https://app.test/" ) );

        CWebViewGlobalSettings global;
        global.engineOrder.push_back( "WPE" );
        global.engineOrder.push_back( "WebKitGTK" );
        global.engineWatchdogMs = 0;
        CORE::CDataNode globalNode;
        ASSERT_TRUE( global.SaveConfig( globalNode ) );
        CWebViewGlobalSettings globalLoaded;
        ASSERT_TRUE( globalLoaded.LoadConfig( globalNode ) );
        ASSERT_TRUE( 2 == globalLoaded.engineOrder.size() && "WPE" == globalLoaded.engineOrder[ 0 ] && 0 == globalLoaded.engineWatchdogMs );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Cookie: the value apart and wiped, never described; domain, path and Secure matching" )
    try
    {
        CWebViewCookie cookie;
        cookie.name = "sid";
        cookie.SetValue( "fake-secret-value" );
        cookie.domain = ".app.test";
        cookie.path = "/account";
        cookie.secure = true;
        cookie.httpOnly = true;
        ASSERT_TRUE( "fake-secret-value" == cookie.GetValue() );
        ASSERT_TRUE( cookie.Describe().HasSubstr( "fake-secret-value" ) < 0 && cookie.Describe().HasSubstr( "HttpOnly" ) > 0 );
        ASSERT_TRUE( cookie.MatchesUrl( "https://app.test/account" ) );
        ASSERT_TRUE( cookie.MatchesUrl( "https://www.app.test/account/settings?x=1" ) );
        ASSERT_FALSE( cookie.MatchesUrl( "http://app.test/account" ) );           // Secure
        ASSERT_FALSE( cookie.MatchesUrl( "https://app.test/accounts" ) );         // not a path below
        ASSERT_FALSE( cookie.MatchesUrl( "https://app.test/" ) );
        ASSERT_FALSE( cookie.MatchesUrl( "https://evilapp.test/account" ) );
        ASSERT_TRUE( cookie.MatchesFilter( "" ) );
        CWebViewCookie hostOnly( cookie );
        hostOnly.domain = "app.test";
        hostOnly.path = "/";
        ASSERT_TRUE( "fake-secret-value" == hostOnly.GetValue() );
        ASSERT_TRUE( hostOnly.MatchesUrl( "https://app.test/x" ) && !hostOnly.MatchesUrl( "https://www.app.test/x" ) );
        CWebViewCookie assigned;
        assigned = cookie;
        ASSERT_TRUE( assigned.IsSameCookie( cookie ) && !assigned.IsSameCookie( hostOnly ) && "fake-secret-value" == assigned.GetValue() );
        ASSERT_TRUE( cookie.IsSession() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Input events: makers, kinds, typed text left out of descriptions" )
    try
    {
        CWebViewInputEvent down = CWebViewInputEvent::PointerButton( 10, 20, WEBMOUSEBUTTON_LEFT, true, 2, WEBMODIFIER_SHIFT );
        ASSERT_TRUE( WEBVIEWINPUT_POINTER_DOWN == down.kind && 10 == down.x && 20 == down.y && 2 == down.clickCount && down.IsPointerEvent() && !down.IsKeyEvent() );
        ASSERT_TRUE( WEBMOUSEBUTTON_NONE == CWebViewInputEvent::PointerMove( 1, 1 ).button );
        CWebViewInputEvent wheel = CWebViewInputEvent::Wheel( 1, 2, 0.0f, -1.5f );
        ASSERT_TRUE( WEBVIEWINPUT_WHEEL == wheel.kind && -1.5f == wheel.wheelNotchesY );
        CWebViewInputEvent key = CWebViewInputEvent::UsLayoutKey( WEBKEYCODE_KEY_A, true, WEBMODIFIER_SHIFT, true );
        ASSERT_TRUE( WEBVIEWINPUT_KEY_DOWN == key.kind && key.isRepeat && key.IsKeyEvent() && key.IsCharacterKey() );
        ASSERT_TRUE( WEBKEYCODE_KEY_A == key.code && WEBNAMEDKEY_CHARACTER == key.namedKey && CString( "A" ) == key.keyText );
        ASSERT_TRUE( key.Describe().HasSubstr( "\"A\"" ) < 0 && key.Describe( true ).HasSubstr( "\"A\"" ) > 0 && key.Describe().HasSubstr( "KeyA" ) > 0 );
        CWebViewInputEvent enter = CWebViewInputEvent::Key( WEBKEYCODE_NUMPAD_ENTER, WEBNAMEDKEY_ENTER, "ignored", false );
        ASSERT_TRUE( WEBVIEWINPUT_KEY_UP == enter.kind && !enter.IsCharacterKey() && '\0' == enter.keyText[ 0 ] && enter.Describe().HasSubstr( "Enter" ) > 0 );
        CWebViewInputEvent text = CWebViewInputEvent::Text( "hunter2" );
        ASSERT_TRUE( text.Describe().HasSubstr( "hunter2" ) < 0 && text.Describe( true ).HasSubstr( "hunter2" ) > 0 );
        ASSERT_TRUE( WEBVIEWINPUT_FOCUS_OUT == CWebViewInputEvent::Focus( false ).kind && WEBVIEWINPUT_POINTER_LEAVE == CWebViewInputEvent::PointerLeave().kind );

        // the key value never overruns its room, and is never cut inside a character
        CWebViewInputEvent cut;
        cut.SetKeyText( "\xE2\x82\xAC\xE2\x82\xAC\xE2\x82\xAC" );     // three euro signs of three bytes: two fit in seven
        ASSERT_TRUE( 6 == strlen( cut.keyText ) );
        cut.SetKeyText( GUCEF_NULL );
        ASSERT_TRUE( '\0' == cut.keyText[ 0 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Key codes: every EWebKeyCode has its DOM code name, and the name gives the code back" )
    try
    {
        ASSERT_TRUE( CString( "KeyA" ) == CWebViewKeyCodes::GetDomCodeName( WEBKEYCODE_KEY_A ) );
        ASSERT_TRUE( CString( "NumpadEnter" ) == CWebViewKeyCodes::GetDomCodeName( WEBKEYCODE_NUMPAD_ENTER ) );
        ASSERT_TRUE( CString( "IntlBackslash" ) == CWebViewKeyCodes::GetDomCodeName( WEBKEYCODE_INTL_BACKSLASH ) );
        ASSERT_TRUE( CString( "Unidentified" ) == CWebViewKeyCodes::GetDomCodeName( WEBKEYCODE_UNIDENTIFIED ) );
        ASSERT_TRUE( CString( "Unidentified" ) == CWebViewKeyCodes::GetDomCodeName( (TWebKeyCode) 100000 ) );
        ASSERT_TRUE( CString( "Unidentified" ) == CWebViewKeyCodes::GetDomCodeName( (TWebKeyCode) -1 ) );
        ASSERT_TRUE( WEBKEYCODE_UNIDENTIFIED == CWebViewKeyCodes::GetKeyCodeFromDomCodeName( "keya" ) );       // names are case sensitive
        ASSERT_TRUE( WEBKEYCODE_UNIDENTIFIED == CWebViewKeyCodes::GetKeyCodeFromDomCodeName( GUCEF_NULL ) );
        bool roundTrips = true, unique = true;
        for ( Int32 i=0; i<(Int32) WEBKEYCODE_COUNT; ++i )
        {
            const char* name = CWebViewKeyCodes::GetDomCodeName( (TWebKeyCode) i );
            if ( GUCEF_NULL == name || '\0' == *name || i != (Int32) CWebViewKeyCodes::GetKeyCodeFromDomCodeName( name ) )
            {
                roundTrips = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Key code " << i << " does not round trip through its DOM name" );
            }
            for ( Int32 j=i+1; j<(Int32) WEBKEYCODE_COUNT; ++j )
                if ( 0 == strcmp( name, CWebViewKeyCodes::GetDomCodeName( (TWebKeyCode) j ) ) )
                    unique = false;
        }
        ASSERT_TRUE( roundTrips && unique );
        ASSERT_TRUE( WEBKEYCODE_COUNT > 180 );                                    // the whole W3C list
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Key codes: every named key has its DOM key name, and the name gives the key back" )
    try
    {
        ASSERT_TRUE( CString( "ArrowLeft" ) == CWebViewKeyCodes::GetDomKeyName( WEBNAMEDKEY_ARROW_LEFT ) );
        ASSERT_TRUE( CString( "Process" ) == CWebViewKeyCodes::GetDomKeyName( WEBNAMEDKEY_PROCESS ) );
        ASSERT_TRUE( CString( "Dead" ) == CWebViewKeyCodes::GetDomKeyName( WEBNAMEDKEY_DEAD ) );
        ASSERT_TRUE( CString( "F24" ) == CWebViewKeyCodes::GetDomKeyName( WEBNAMEDKEY_F24 ) );
        ASSERT_TRUE( '\0' == *CWebViewKeyCodes::GetDomKeyName( WEBNAMEDKEY_CHARACTER ) );
        ASSERT_TRUE( CString( "Unidentified" ) == CWebViewKeyCodes::GetDomKeyName( (TWebNamedKey) 100000 ) );
        ASSERT_TRUE( WEBNAMEDKEY_UNIDENTIFIED == CWebViewKeyCodes::GetNamedKeyFromDomKeyName( "a" ) );        // a character is no named key
        ASSERT_TRUE( WEBNAMEDKEY_UNIDENTIFIED == CWebViewKeyCodes::GetNamedKeyFromDomKeyName( "" ) );
        bool roundTrips = true, unique = true;
        for ( Int32 i=0; i<(Int32) WEBNAMEDKEY_COUNT; ++i )
        {
            if ( WEBNAMEDKEY_CHARACTER == i )
                continue;
            const char* name = CWebViewKeyCodes::GetDomKeyName( (TWebNamedKey) i );
            if ( GUCEF_NULL == name || '\0' == *name || i != (Int32) CWebViewKeyCodes::GetNamedKeyFromDomKeyName( name ) )
            {
                roundTrips = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Named key " << i << " does not round trip through its DOM name" );
            }
            for ( Int32 j=i+1; j<(Int32) WEBNAMEDKEY_COUNT; ++j )
                if ( 0 == strcmp( name, CWebViewKeyCodes::GetDomKeyName( (TWebNamedKey) j ) ) )
                    unique = false;
        }
        ASSERT_TRUE( roundTrips && unique );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Key codes: the evdev and X11 hardware key codes of the physical keys" )
    try
    {
        // known values of an evdev keyboard (X11 hardware key code = evdev code + 8)
        ASSERT_TRUE( 30 == CWebViewKeyCodes::GetEvdevCode( WEBKEYCODE_KEY_A ) && 38 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_KEY_A ) );
        ASSERT_TRUE( 113 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_ARROW_LEFT ) && 72 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_F6 ) );
        ASSERT_TRUE( 104 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_NUMPAD_ENTER ) && 94 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_INTL_BACKSLASH ) );
        ASSERT_TRUE( 0 == CWebViewKeyCodes::GetEvdevCode( WEBKEYCODE_UNIDENTIFIED ) && 0 == CWebViewKeyCodes::GetX11HardwareKeycode( WEBKEYCODE_UNIDENTIFIED ) );
        ASSERT_TRUE( 0 == CWebViewKeyCodes::GetX11HardwareKeycode( (TWebKeyCode) 100000 ) );

        // no two keys share a code
        UInt32 withCode = 0;
        bool unique = true;
        for ( Int32 i=0; i<(Int32) WEBKEYCODE_COUNT; ++i )
        {
            const UInt16 evdev = CWebViewKeyCodes::GetEvdevCode( (TWebKeyCode) i );
            if ( 0 == evdev )
                continue;
            ++withCode;
            if ( evdev + 8 != CWebViewKeyCodes::GetX11HardwareKeycode( (TWebKeyCode) i ) )
                unique = false;
            for ( Int32 j=i+1; j<(Int32) WEBKEYCODE_COUNT; ++j )
                if ( evdev == CWebViewKeyCodes::GetEvdevCode( (TWebKeyCode) j ) )
                    unique = false;
        }
        ASSERT_TRUE( unique );
        // the keys of the main block, the control and arrow pads and the function row all have one
        bool standardComplete = true;
        for ( Int32 i=WEBKEYCODE_BACKQUOTE; i<=WEBKEYCODE_ARROW_UP; ++i )
            if ( 0 == CWebViewKeyCodes::GetEvdevCode( (TWebKeyCode) i ) )
                standardComplete = false;
        for ( Int32 i=WEBKEYCODE_ESCAPE; i<=WEBKEYCODE_F24; ++i )
            if ( 0 == CWebViewKeyCodes::GetEvdevCode( (TWebKeyCode) i ) )
                standardComplete = false;
        ASSERT_TRUE( standardComplete && withCode > 150 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Key codes: the Windows scan codes and virtual keys of the physical keys" )
    try
    {
        ASSERT_TRUE( 0x1E == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_KEY_A ) && 0x41 == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_KEY_A ) );
        ASSERT_TRUE( 0xE01C == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_NUMPAD_ENTER ) && 0x1C == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_ENTER ) );
        ASSERT_TRUE( 0x0D == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_ENTER ) && 0x0D == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_NUMPAD_ENTER ) );
        ASSERT_TRUE( 0xA0 == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_SHIFT_LEFT ) && 0xA1 == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_SHIFT_RIGHT ) );
        ASSERT_TRUE( 0xE048 == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_ARROW_UP ) && 0x26 == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_ARROW_UP ) );
        ASSERT_TRUE( 0x87 == CWebViewKeyCodes::GetWindowsVirtualKey( WEBKEYCODE_F24 ) && 0x76 == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_F24 ) );
        ASSERT_TRUE( 0 == CWebViewKeyCodes::GetWindowsScanCode( WEBKEYCODE_UNIDENTIFIED ) && 0 == CWebViewKeyCodes::GetWindowsVirtualKey( (TWebKeyCode) 100000 ) );

        // no two keys share a scan code; every key with a scan code also has an evdev code (they are the same keys)
        bool unique = true, covered = true;
        UInt32 withVirtualKey = 0;
        for ( Int32 i=0; i<(Int32) WEBKEYCODE_COUNT; ++i )
        {
            const UInt16 scanCode = CWebViewKeyCodes::GetWindowsScanCode( (TWebKeyCode) i );
            if ( 0 != CWebViewKeyCodes::GetWindowsVirtualKey( (TWebKeyCode) i ) )
                ++withVirtualKey;
            if ( 0 == scanCode )
                continue;
            if ( 0 == CWebViewKeyCodes::GetEvdevCode( (TWebKeyCode) i ) )
                covered = false;
            for ( Int32 j=i+1; j<(Int32) WEBKEYCODE_COUNT; ++j )
                if ( scanCode == CWebViewKeyCodes::GetWindowsScanCode( (TWebKeyCode) j ) )
                    unique = false;
        }
        ASSERT_TRUE( unique && covered && withVirtualKey > 140 );
        // the keys of the main block, the function keys, the arrows and the numpad digits all have a virtual key
        for ( Int32 i=WEBKEYCODE_KEY_A; i<=WEBKEYCODE_KEY_Z; ++i )
            ASSERT_TRUE( 0x41 + ( i - WEBKEYCODE_KEY_A ) == CWebViewKeyCodes::GetWindowsVirtualKey( (TWebKeyCode) i ) );
        for ( Int32 i=WEBKEYCODE_F1; i<=WEBKEYCODE_F24; ++i )
            ASSERT_TRUE( 0x70 + ( i - WEBKEYCODE_F1 ) == CWebViewKeyCodes::GetWindowsVirtualKey( (TWebKeyCode) i ) );
        for ( Int32 i=WEBKEYCODE_NUMPAD0; i<=WEBKEYCODE_NUMPAD9; ++i )
            ASSERT_TRUE( 0x60 + ( i - WEBKEYCODE_NUMPAD0 ) == CWebViewKeyCodes::GetWindowsVirtualKey( (TWebKeyCode) i ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Key codes: key values of the US layout with shift and caps lock, locations, modifier keys, mouse button flags" )
    try
    {
        const char* text = GUCEF_NULL;
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_KEY_A, 0, text ) && CString( "a" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_KEY_A, WEBMODIFIER_SHIFT, text ) && CString( "A" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_KEY_A, WEBMODIFIER_CAPSLOCK, text ) && CString( "A" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_KEY_A, WEBMODIFIER_CAPSLOCK | WEBMODIFIER_SHIFT, text ) && CString( "a" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_DIGIT1, WEBMODIFIER_CAPSLOCK, text ) && CString( "1" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_DIGIT1, WEBMODIFIER_SHIFT, text ) && CString( "!" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_CHARACTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_QUOTE, WEBMODIFIER_SHIFT, text ) && CString( "\"" ) == text );
        ASSERT_TRUE( WEBNAMEDKEY_ARROW_LEFT == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_ARROW_LEFT, WEBMODIFIER_SHIFT, text ) && GUCEF_NULL == text );
        ASSERT_TRUE( WEBNAMEDKEY_ENTER == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_NUMPAD_ENTER, 0, text ) && GUCEF_NULL == text );
        ASSERT_TRUE( WEBNAMEDKEY_SHIFT == CWebViewKeyCodes::GetUsLayoutKey( WEBKEYCODE_SHIFT_RIGHT, WEBMODIFIER_SHIFT, text ) );
        ASSERT_TRUE( WEBNAMEDKEY_UNIDENTIFIED == CWebViewKeyCodes::GetUsLayoutKey( (TWebKeyCode) 100000, 0, text ) && GUCEF_NULL == text );
        ASSERT_TRUE( CWebViewKeyCodes::IsPrintable( WEBKEYCODE_SPACE ) && !CWebViewKeyCodes::IsPrintable( WEBKEYCODE_ESCAPE ) );

        // every key has a key value: a character with its text, or a named key without
        bool consistent = true;
        for ( Int32 i=0; i<(Int32) WEBKEYCODE_COUNT; ++i )
        {
            for ( UInt32 shift=0; shift<2; ++shift )
            {
                const TWebNamedKey key = CWebViewKeyCodes::GetUsLayoutKey( (TWebKeyCode) i, 0 == shift ? 0 : WEBMODIFIER_SHIFT, text );
                if ( ( WEBNAMEDKEY_CHARACTER == key ) != ( GUCEF_NULL != text && '\0' != *text ) || (Int32) key < 0 || (Int32) key >= (Int32) WEBNAMEDKEY_COUNT )
                    consistent = false;
            }
        }
        ASSERT_TRUE( consistent );

        ASSERT_TRUE( WEBKEYLOCATION_LEFT == CWebViewKeyCodes::GetLocation( WEBKEYCODE_SHIFT_LEFT ) && WEBKEYLOCATION_RIGHT == CWebViewKeyCodes::GetLocation( WEBKEYCODE_META_RIGHT ) );
        ASSERT_TRUE( WEBKEYLOCATION_NUMPAD == CWebViewKeyCodes::GetLocation( WEBKEYCODE_NUMPAD7 ) && WEBKEYLOCATION_STANDARD == CWebViewKeyCodes::GetLocation( WEBKEYCODE_KEY_Q ) );
        ASSERT_TRUE( CWebViewKeyCodes::IsModifierKey( WEBKEYCODE_CONTROL_RIGHT ) && !CWebViewKeyCodes::IsModifierKey( WEBKEYCODE_CAPS_LOCK ) );

        // MouseEvent.button to MouseEvent.buttons: right is 2 in both, middle is 1 and 4
        ASSERT_TRUE( WEBMOUSEBUTTONFLAG_LEFT == CWebViewKeyCodes::GetMouseButtonFlag( WEBMOUSEBUTTON_LEFT ) );
        ASSERT_TRUE( WEBMOUSEBUTTONFLAG_MIDDLE == CWebViewKeyCodes::GetMouseButtonFlag( WEBMOUSEBUTTON_MIDDLE ) );
        ASSERT_TRUE( WEBMOUSEBUTTONFLAG_RIGHT == CWebViewKeyCodes::GetMouseButtonFlag( WEBMOUSEBUTTON_RIGHT ) );
        ASSERT_TRUE( WEBMOUSEBUTTONFLAG_FORWARD == CWebViewKeyCodes::GetMouseButtonFlag( WEBMOUSEBUTTON_FORWARD ) );
        ASSERT_TRUE( 0 == CWebViewKeyCodes::GetMouseButtonFlag( WEBMOUSEBUTTON_NONE ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Type names: the names of settings both ways, unknown names keep the fallback" )
    try
    {
        ASSERT_TRUE( WEBVIEWPOPUPS_SAMEVIEW == WebViewPopupPolicyFromString( WebViewPopupPolicyToString( WEBVIEWPOPUPS_SAMEVIEW ), WEBVIEWPOPUPS_REFUSE ) );
        ASSERT_TRUE( WEBVIEWPOPUPS_REFUSE == WebViewPopupPolicyFromString( "whatever", WEBVIEWPOPUPS_REFUSE ) );
        ASSERT_TRUE( WEBVIEWNAVIGATION_ALLOWLIST == WebViewNavigationPolicyFromString( "ALLOWLIST", WEBVIEWNAVIGATION_ANY ) );
        ASSERT_TRUE( WEBVIEWMODE_NATIVEWINDOW == WebViewModeFromString( "nativeWindow", WEBVIEWMODE_OFFSCREEN ) );
        ASSERT_TRUE( CString( "refusedByPolicy" ) == WebViewErrorToString( WEBVIEWERROR_REFUSED_BY_POLICY ) );
        ASSERT_TRUE( CString( "unknown" ) == WebViewErrorToString( (TWebViewError) 999 ) );
        ASSERT_TRUE( WEBVIEWSAMESITE_STRICT == WebViewSameSiteFromString( "strict", WEBVIEWSAMESITE_UNSET ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
