/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#include <string.h>

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
#include <sys/stat.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#include "gucefWEB_CChromeNetworkRecorder.h"
#include "gucefWEB_CSiteDiscoveryJournal.h"
#include "gucefWEB_CNavigationRule.h"
#include "gucefWEB_CNavigationGuard.h"
#include "gucefWEB_CBrowserDownloadCapture.h"
#include "gucefWEB_CWebResponseTap.h"
#include "gucefWEB_CDevToolsPage.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestFakeDevToolsBrowser.h"
#include "TestBrowserAutomation.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const char* part )
{
    return !text.IsNULLOrEmpty() && GUCEF_NULL != strstr( text.C_String(), part );
}

/*-------------------------------------------------------------------------*/

static bool
LoadPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/*-------------------------------------------------------------------------*/

static bool
PluginsAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadPlugin( "dstorepluginJSONPARSER" );
    if ( !WEB::CWebSocketHandshake::IsAvailable() )
        LoadPlugin( "codecspluginSTBRUMMEHASH" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && WEB::CWebSocketHandshake::IsAvailable();
}

/*-------------------------------------------------------------------------*/

/** A node from JSON text, for scripted answers and events */
static CORE::CDataNode
Json( const CORE::CString& text )
{
    CORE::CDataNode node;
    COM::CJsonRpcEndpoint::ParseJsonText( text, node );
    return node;
}

/*-------------------------------------------------------------------------*/

/** Each round is about 5 ms: the fake browser's small writes can wait for the client's delayed acknowledgement */
static void
Pump( UInt32 rounds                                 ,
      WEB::CChromeNetworkRecorder* recorder = GUCEF_NULL ,
      WEB::CWebResponseTap* tap = GUCEF_NULL             ,
      WEB::CBrowserDownloadCapture* capture = GUCEF_NULL )
{
    for ( UInt32 i=0; i<rounds*5; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        if ( GUCEF_NULL != recorder )
            recorder->Process();
        if ( GUCEF_NULL != tap )
            tap->Process();
        if ( GUCEF_NULL != capture )
            capture->Process();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
ModeOf( const CORE::CString& path )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct stat info;
    if ( 0 != stat( path.C_String(), &info ) )
        return 0xFFFF;
    return (UInt32) ( info.st_mode & 0777 );
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Writes a file as the browser would leave it in the incoming folder; also an empty one */
static bool
WriteFile( const CORE::CString& path, const CORE::CString& content )
{
    CORE::CString problem;
    return CORE::CreateOwnerOnlyFile( path, content.C_String(), content.IsNULLOrEmpty() ? 0 : content.ByteSize() - 1, problem );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Sha256Hex( const CORE::CString& content )
{
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( content.C_String(), content.IsNULLOrEmpty() ? 0 : content.ByteSize() - 1, digest );
    static const char* const hexDigits = "0123456789abcdef";
    CORE::CString hex;
    for ( UInt32 i=0; i<CORE::CSha256::DigestSize; ++i )
    {
        hex += hexDigits[ digest[ i ] >> 4 ];
        hex += hexDigits[ digest[ i ] & 15 ];
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Collects what a recorder hands over */
class CActionCollector : public WEB::CIWebObservationSink
{
    public:

    GUCEF::vector< WEB::CObservedUserAction > actions;
    GUCEF::vector< WEB::CObservedWebRequest > requests;

    virtual void OnWebRequestObserved( const WEB::CObservedWebRequest& observation ) GUCEF_VIRTUAL_OVERRIDE { requests.push_back( observation ); }
    virtual void OnUserActionObserved( const WEB::CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE { actions.push_back( action ); }

    bool HasLabel( const char* label ) const
    {
        for ( size_t i=0; i<actions.size(); ++i )
            if ( actions[ i ].label == label )
                return true;
        return false;
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

namespace {

/** Keeps copies of the bodies a response tap delivers */
class CBodyCollector : public WEB::CIWebResponseSink
{
    public:

    struct SBody
    {
        CORE::CString method;
        CORE::CString host;
        CORE::CString pathTemplate;
        Int32 status;
        CORE::CString mimeType;
        UInt32 requestSeq;
        UInt32 tapId;
        CORE::CString text;
    };

    GUCEF::vector< SBody > bodies;

    virtual void OnResponseBody( const WEB::CObservedResponseBody& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        SBody body;
        body.method = response.method;
        body.host = response.host;
        body.pathTemplate = response.pathTemplate;
        body.status = response.status;
        body.mimeType = response.mimeType;
        body.requestSeq = response.requestSeq;
        body.tapId = response.tapId;
        body.text = CORE::CString( static_cast< const char* >( response.body.GetConstBufferPtr() ), response.body.GetDataSize() );
        bodies.push_back( body );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static bool
Connect( CFakeDevToolsBrowser& browser, WEB::CChromeDevToolsClient& client )
{
    WEB::SChromeDiscoveryResult discovery;
    return client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) && client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl );
}

/*-------------------------------------------------------------------------*/

static UInt32
IndexOfCommand( const CFakeDevToolsBrowser& browser, const CORE::CString& entry )
{
    for ( size_t i=0; i<browser.commands.size(); ++i )
        if ( browser.commands[ i ] == entry )
            return (UInt32) i;
    return 0xFFFFFFFF;
}

/*-------------------------------------------------------------------------*/

static void
EmitRequestCycle( CFakeDevToolsBrowser& browser, const CORE::CString& session, const CORE::CString& requestId, const CORE::CString& method,
                  const CORE::CString& url, const CORE::CString& mime, UInt32 size )
{
    CFakeDevToolsBrowser::TStringMap none;
    browser.EmitRequest( session, requestId, method, url, "Fetch", "P1", none, "", false, 10.0 );
    browser.EmitResponse( session, requestId, 200, mime, none, "Fetch" );
    browser.EmitFinished( session, requestId, size );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformClickListenerTests( void )
{
    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "http://shop.example.test/" );

    GUCEF_TESTFW_TESTCASE( "Test: The click listener reaches every page and frame session and survives navigations" )
        try
        {
            ASSERT_TRUE( started );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.captureClicks = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            CActionCollector collector;
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &collector );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );

            // the runtime is enabled before the binding, so the binding reaches the documents that come later
            const UInt32 enable = IndexOfCommand( browser, "Runtime.enable@S-P1" );
            const UInt32 binding = IndexOfCommand( browser, "Runtime.addBinding@S-P1" );
            const UInt32 script = IndexOfCommand( browser, "Page.addScriptToEvaluateOnNewDocument@S-P1" );
            ASSERT_TRUE( enable != 0xFFFFFFFF && binding != 0xFFFFFFFF && script != 0xFFFFFFFF );
            ASSERT_TRUE( enable < binding && binding < script );

            // in an isolated world, in the documents that are there and in every later one
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Runtime.addBinding", "S-P1", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "name" ).AsString() == WEB::CChromeNetworkRecorder::ActionBindingName );
            ASSERT_TRUE( params.GetAttributeValue( "executionContextName" ).AsString() == WEB::CChromeNetworkRecorder::ClickListenerWorldName );
            ASSERT_TRUE( browser.GetLastCommandParams( "Page.addScriptToEvaluateOnNewDocument", "S-P1", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "worldName" ).AsString() == WEB::CChromeNetworkRecorder::ClickListenerWorldName );
            ASSERT_TRUE( params.GetAttributeValue( "runImmediately" ).AsBool( false ) );
            ASSERT_FALSE( browser.WasCommandSent( "Runtime.evaluate", "S-P1" ) );

            // the script reports labels and roles, never values, and keeps table cells and password fields out
            const CORE::CString& source = WEB::CChromeNetworkRecorder::GetClickListenerScript();
            ASSERT_TRUE( Contains( source, "aria-label" ) && Contains( source, "innerText" ) && Contains( source, "composedPath" ) );
            ASSERT_TRUE( Contains( source, "password" ) && Contains( source, "inTable" ) && Contains( source, "[role=gridcell]" ) );
            ASSERT_FALSE( Contains( source, "el.value" ) );

            // a cross-origin frame of the tab comes with its own session, which gets the listener too
            browser.Emit( "S-P1", "Target.attachedToTarget",
                          Json( "{\"sessionId\":\"S-F1\",\"targetInfo\":{\"targetId\":\"F1\",\"type\":\"iframe\",\"url\":\"http://frames.example.test/widget\"},\"waitingForDebugger\":false}" ) );
            Pump( 50, &recorder );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.enable", "S-F1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.addBinding", "S-F1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Page.addScriptToEvaluateOnNewDocument", "S-F1" ) );
            ASSERT_TRUE( recorder.GetSessionCount() == 2 );

            // the tab navigates; a click in the frame is an action on the tab's page, with its label templated
            CFakeDevToolsBrowser::TStringMap none;
            browser.EmitRequest( "S-P1", "D1", "GET", "http://shop.example.test/orders/12345678", "Document", "P1", none, "", false, 1.0 );
            Pump( 10, &recorder );
            browser.Emit( "S-F1", "Runtime.bindingCalled",
                          Json( "{\"name\":\"__gucefSiteDiscoveryAction\",\"executionContextId\":3,"
                                "\"payload\":\"{\\\"kind\\\":\\\"click\\\",\\\"tag\\\":\\\"button\\\",\\\"role\\\":\\\"button\\\",\\\"label\\\":\\\"Pay later 42\\\",\\\"inTable\\\":false}\"}" ) );
            Pump( 10, &recorder );
            ASSERT_TRUE( collector.HasLabel( "Pay later {n}" ) );
            bool onPage = false;
            for ( size_t i=0; i<collector.actions.size(); ++i )
                if ( collector.actions[ i ].label == "Pay later {n}" )
                    onPage = collector.actions[ i ].page == "http://shop.example.test/orders/{n}" && collector.actions[ i ].kind == "click";
            ASSERT_TRUE( onPage );

            // a control in a table cell: its role only, even if the page sent a label
            browser.Emit( "S-P1", "Runtime.bindingCalled",
                          Json( "{\"name\":\"__gucefSiteDiscoveryAction\",\"executionContextId\":1,"
                                "\"payload\":\"{\\\"kind\\\":\\\"click\\\",\\\"tag\\\":\\\"a\\\",\\\"role\\\":\\\"link\\\",\\\"label\\\":\\\"ACCT 9999\\\",\\\"inTable\\\":true}\"}" ) );
            Pump( 10, &recorder );
            ASSERT_TRUE( collector.HasLabel( "<link in table>" ) );
            ASSERT_FALSE( collector.HasLabel( "ACCT {n}" ) );

            // a role that is not a plain word is not shown
            browser.Emit( "S-P1", "Runtime.bindingCalled",
                          Json( "{\"name\":\"__gucefSiteDiscoveryAction\",\"executionContextId\":1,"
                                "\"payload\":\"{\\\"kind\\\":\\\"submit\\\",\\\"tag\\\":\\\"form\\\",\\\"role\\\":\\\"<img src=x>\\\",\\\"label\\\":\\\"\\\"}\"}" ) );
            Pump( 10, &recorder );
            for ( size_t i=0; i<collector.actions.size(); ++i )
                ASSERT_FALSE( Contains( collector.actions[ i ].label, "img" ) );

            // events of a session someone else set up are not the recorder's
            const size_t actionsBefore = collector.actions.size();
            browser.Emit( "S-OTHER", "Runtime.bindingCalled",
                          Json( "{\"name\":\"__gucefSiteDiscoveryAction\",\"executionContextId\":1,"
                                "\"payload\":\"{\\\"kind\\\":\\\"click\\\",\\\"tag\\\":\\\"button\\\",\\\"label\\\":\\\"Foreign\\\"}\"}" ) );
            EmitRequestCycle( browser, "S-OTHER", "X1", "GET", "http://shop.example.test/api/foreign", "application/json", 10 );
            Pump( 20, &recorder );
            ASSERT_TRUE( collector.actions.size() == actionsBefore );
            for ( size_t i=0; i<collector.requests.size(); ++i )
                ASSERT_FALSE( Contains( collector.requests[ i ].url, "foreign" ) );
            recorder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A browser that refuses runImmediately gets the listener in the page's own world" )
        try
        {
            browser.ForgetConnection();
            browser.SetCommandError( "Page.addScriptToEvaluateOnNewDocument", "Invalid parameters" );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );
            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.captureClicks = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.enable", "S-P1" ) );
            ASSERT_TRUE( browser.CountCommands( "Runtime.addBinding", "S-P1" ) == 2 );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.evaluate", "S-P1" ) );
            recorder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformNavigationRuleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Navigation rules parse, normalise paths and match hosts, ports, prefixes and globs" )
        try
        {
            WEB::CNavigationRule rule;
            ASSERT_TRUE( WEB::CNavigationRule::Parse( " https://App.Example.TEST/Accounts/Positions ", rule ) );
            ASSERT_TRUE( rule.hostPattern == "app.example.test" && rule.pathPattern == "/Accounts/Positions" );
            ASSERT_TRUE( WEB::CNavigationRule::Parse( "*.example.test", rule ) && rule.pathPattern.IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CNavigationRule::Parse( "example.test/", rule ) && rule.pathPattern.IsNULLOrEmpty() );
            ASSERT_FALSE( WEB::CNavigationRule::Parse( "", rule ) );
            ASSERT_FALSE( WEB::CNavigationRule::Parse( "/only/a/path", rule ) );
            ASSERT_FALSE( WEB::CNavigationRule::Parse( "bad host/x", rule ) );
            ASSERT_FALSE( WEB::CNavigationRule::Parse( "a.test/x?y=1", rule ) );
            ASSERT_FALSE( WEB::CNavigationRule::Parse( "a.test/x\\y", rule ) );

            bool ambiguous = false;
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/a/./b/../c", ambiguous ) == "/a/c" && !ambiguous );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/a/%62%2e%63", ambiguous ) == "/a/b.c" && !ambiguous );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/a/%3f", ambiguous ) == "/a/%3F" && !ambiguous );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "", ambiguous ) == "/" );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/a/b/", ambiguous ) == "/a/b/" );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/a/..", ambiguous ) == "/" );
            ASSERT_TRUE( WEB::CNavigationRule::NormalisePath( "/../../etc", ambiguous ) == "/etc" );
            WEB::CNavigationRule::NormalisePath( "/a%2Fb", ambiguous );
            ASSERT_TRUE( ambiguous );
            WEB::CNavigationRule::NormalisePath( "/a%5cb", ambiguous );
            ASSERT_TRUE( ambiguous );
            WEB::CNavigationRule::NormalisePath( "/a\\b", ambiguous );
            ASSERT_TRUE( ambiguous );

            WEB::CNavigationRule prefix( "app.example.test", "/app/positions" );
            ASSERT_TRUE( prefix.MatchesPath( "/app/positions", true ) && prefix.MatchesPath( "/app/positions/lots", true ) );
            ASSERT_FALSE( prefix.MatchesPath( "/app/positionsX", true ) || prefix.MatchesPath( "/app", true ) );
            ASSERT_FALSE( prefix.MatchesPath( "/APP/positions", true ) );
            ASSERT_TRUE( prefix.MatchesPath( "/APP/positions", false ) );
            WEB::CNavigationRule glob( "*.example.test", "/reports/**/*.csv" );
            ASSERT_TRUE( glob.MatchesPath( "/reports/2026/lots.csv", true ) && glob.MatchesPath( "/reports/lots.csv", true ) );
            ASSERT_FALSE( glob.MatchesPath( "/reports/lots.pdf", true ) );
            WEB::CNavigationRule segment( "*.example.test", "/reports/*.csv" );
            ASSERT_TRUE( segment.MatchesPath( "/reports/lots.csv", true ) && !segment.MatchesPath( "/reports/2026/lots.csv", true ) );
            WEB::CNavigationRule braces( "*.example.test", "/accounts/{n}/*" );
            ASSERT_TRUE( braces.MatchesPath( "/accounts/{n}/lots", true ) && !braces.MatchesPath( "/accounts/n/lots", true ) );
            ASSERT_TRUE( glob.MatchesHost( "www.example.test", "www.example.test" ) && !glob.MatchesHost( "example.test.evil", "example.test.evil" ) );
            WEB::CNavigationRule port( "127.0.0.1:8080", "" );
            ASSERT_TRUE( port.MatchesHost( "127.0.0.1", "127.0.0.1:8080" ) && !port.MatchesHost( "127.0.0.1", "127.0.0.1:9090" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The guard allows, denies and refuses sign-in addresses as its settings say" )
        try
        {
            WEB::CNavigationGuardSettings settings;
            settings.allow.push_back( "*.bank.test/app" );
            settings.allow.push_back( "login.bank.test" );
            settings.deny.push_back( "*.bank.test/app/trade" );
            WEB::CNavigationGuard guard;
            ASSERT_TRUE( guard.SetSettings( settings ) );
            CORE::CString reason;
            ASSERT_TRUE( guard.IsAllowed( "https://www.bank.test/app/positions?acct=1#top", true, &reason ) && reason.IsNULLOrEmpty() );
            ASSERT_TRUE( guard.IsAllowed( "https://www.bank.test/app", true, &reason ) );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/app/trade/new", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonDenied );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/app/TRADE", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonDenied );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/app/x/../trade", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonDenied );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/app%2Ftrade", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonDenied );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/app%2Fpositions", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonNotAllowed );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/App/positions", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonNotAllowed );
            ASSERT_FALSE( guard.IsAllowed( "https://www.bank.test/application", true, &reason ) );
            ASSERT_FALSE( guard.IsAllowed( "https://other.test/app", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonNotAllowed );
            ASSERT_FALSE( guard.IsAllowed( "ftp://www.bank.test/app", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonScheme );
            ASSERT_FALSE( guard.IsAllowed( "javascript:alert(1)", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonScheme );
            ASSERT_FALSE( guard.IsAllowed( "not a url", true, &reason ) );
            ASSERT_TRUE( guard.IsAllowed( "about:blank", true, &reason ) && guard.IsAllowed( "about:srcdoc", false, &reason ) );
            ASSERT_FALSE( guard.IsAllowed( "about:config", true, &reason ) );

            // a sign-in host that an allow rule names may be visited
            ASSERT_TRUE( guard.IsAllowed( "https://login.bank.test/signin", true, &reason ) );

            // without allow rules: everything but the deny rules and the sign-in and payment addresses
            WEB::CNavigationGuardSettings open;
            WEB::CNavigationGuard openGuard;
            ASSERT_TRUE( openGuard.SetSettings( open ) );
            ASSERT_TRUE( openGuard.IsAllowed( "https://shop.test/home", true, &reason ) );
            ASSERT_FALSE( openGuard.IsAllowed( "https://accounts.google.com/o/oauth2", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonNeverRecord );
            ASSERT_FALSE( openGuard.IsAllowed( "https://shop.test/checkout/12345", true, &reason ) );
            ASSERT_TRUE( reason == WEB::CNavigationRefusal::ReasonNeverRecord );
            open.refuseNeverRecordUnlessAllowed = false;
            ASSERT_TRUE( openGuard.SetSettings( open ) );
            ASSERT_TRUE( openGuard.IsAllowed( "https://shop.test/checkout/12345", true, &reason ) );

            // frames only when guarded
            settings.guardSubframes = false;
            ASSERT_TRUE( guard.SetSettings( settings ) );
            ASSERT_TRUE( guard.IsAllowed( "https://ads.other.test/frame", false, &reason ) );
            ASSERT_FALSE( guard.IsAllowed( "https://ads.other.test/frame", true, &reason ) );

            // a rule that does not parse leaves the guard as it was
            WEB::CNavigationGuardSettings broken;
            broken.allow.push_back( "no spaces allowed" );
            ASSERT_FALSE( guard.SetSettings( broken ) );
            ASSERT_TRUE( guard.IsAllowed( "https://www.bank.test/app/positions", true, &reason ) );

            ASSERT_TRUE( WEB::CNavigationGuard::MakeUrlTemplate( "https://www.bank.test/app/accounts/12345678/lots?token=PLANTEDqueryTOKEN" ) == "https://www.bank.test/app/accounts/{n}/lots" );
            ASSERT_TRUE( WEB::CNavigationGuard::MakeUrlTemplate( "javascript:alert(document.cookie)" ) == "javascript:" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Navigation guard settings load and save their rules and refuse a broken one" )
        try
        {
            CORE::CDataNode cfg = Json( "{\"allow\":[\"*.bank.test/app\",\"login.bank.test\"],\"deny\":\"*.bank.test/app/trade\",\"guardSubframes\":false,"
                                        "\"refuseNeverRecordUnlessAllowed\":false,\"neverRecordHosts\":[]}" );
            WEB::CNavigationGuardSettings settings;
            ASSERT_TRUE( settings.LoadConfig( cfg ) );
            ASSERT_TRUE( settings.allow.size() == 2 && settings.deny.size() == 1 && settings.deny[ 0 ] == "*.bank.test/app/trade" );
            ASSERT_FALSE( settings.guardSubframes || settings.refuseNeverRecordUnlessAllowed );
            ASSERT_TRUE( settings.neverRecordHosts.empty() && !settings.neverRecordPathPatterns.empty() );
            CORE::CDataNode saved;
            ASSERT_TRUE( settings.SaveConfig( saved ) );
            WEB::CNavigationGuardSettings again;
            ASSERT_TRUE( again.LoadConfig( saved ) );
            ASSERT_TRUE( again.allow == settings.allow && again.deny == settings.deny && !again.guardSubframes );

            CORE::CDataNode bad = Json( "{\"allow\":[\"ok.test\",\"not ok\"]}" );
            ASSERT_FALSE( again.LoadConfig( bad ) );
            ASSERT_TRUE( again.allow == settings.allow );

            WEB::CNavigationGuardSettings defaults;
            ASSERT_TRUE( defaults.allow.empty() && defaults.deny.empty() && defaults.guardSubframes && defaults.refuseNeverRecordUnlessAllowed );
            ASSERT_TRUE( defaults.neverRecordHosts == WEB::CSiteDiscoverySettings::GetDefaultNeverRecordHosts() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Counts the refusals a guard reports */
class CRefusalCounter : public CORE::CObserver
{
    public:

    UInt32 count;

    CRefusalCounter( void ) : CORE::CObserver(), count( 0 ) {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == WEB::CNavigationGuard::NavigationRefusedEvent && GUCEF_NULL != eventData )
            ++count;
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
PerformNavigationGuardTests( void )
{
    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "https://www.bank.test/app/home" );

    GUCEF_TESTFW_TESTCASE( "Test: The guard answers every paused document request and reports the refused ones" )
        try
        {
            ASSERT_TRUE( started );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CNavigationGuardSettings settings;
            settings.allow.push_back( "*.bank.test/app" );
            settings.deny.push_back( "*.bank.test/app/trade" );
            WEB::CNavigationGuard guard;
            ASSERT_TRUE( guard.SetSettings( settings ) );
            CRefusalCounter counter;
            guard.Subscribe( &counter );
            ASSERT_TRUE( guard.Start( client ) );
            ASSERT_TRUE( guard.IsStarted() );

            // document requests of every tab and frame, on the browser target
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.enable", "", params ) );
            const CORE::CDataNode* patterns = params.FindChild( "patterns" );
            ASSERT_TRUE( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 );
            if ( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 )
            {
                const CORE::CDataNode& pattern = *( *patterns->ConstBegin() );
                ASSERT_TRUE( pattern.GetAttributeValue( "resourceType" ).AsString() == "Document" );
                ASSERT_TRUE( pattern.GetAttributeValue( "requestStage" ).AsString() == "Request" );
            }
            ASSERT_TRUE( browser.WasCommandSent( "Target.setDiscoverTargets", "" ) );

            // allowed: continued
            browser.Emit( "", "Fetch.requestPaused", Json( "{\"requestId\":\"F1\",\"request\":{\"url\":\"https://www.bank.test/app/positions\",\"method\":\"GET\"},"
                                                           "\"frameId\":\"P1\",\"resourceType\":\"Document\"}" ) );
            Pump( 20 );
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.continueRequest", "", params ) && params.GetAttributeValue( "requestId" ).AsString() == "F1" );
            ASSERT_TRUE( guard.GetAllowedCount() == 1 && guard.GetRefusedCount() == 0 );

            // denied: aborted, so the tab stays where it was, and reported without the query
            browser.Emit( "", "Fetch.requestPaused", Json( "{\"requestId\":\"F2\",\"request\":{\"url\":\"https://www.bank.test/app/trade/order?token=PLANTEDqueryTOKEN1\",\"method\":\"GET\"},"
                                                           "\"frameId\":\"P1\",\"resourceType\":\"Document\"}" ) );
            Pump( 20 );
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.failRequest", "", params ) && params.GetAttributeValue( "requestId" ).AsString() == "F2" );
            ASSERT_TRUE( params.GetAttributeValue( "errorReason" ).AsString() == "Aborted" );
            ASSERT_TRUE( guard.GetRefusedCount() == 1 && counter.count == 1 );
            const WEB::CNavigationRefusal& denied = guard.GetRecentRefusals().back();
            ASSERT_TRUE( denied.urlTemplate == "https://www.bank.test/app/trade/order" && denied.reason == "denied" && denied.source == "page" );
            ASSERT_TRUE( denied.isMainFrame && denied.blocked && denied.frameId == "P1" && denied.host == "www.bank.test" );

            // a redirect of a frame to another site
            browser.Emit( "", "Fetch.requestPaused", Json( "{\"requestId\":\"F3\",\"request\":{\"url\":\"https://evil.test/collect\",\"method\":\"GET\"},"
                                                           "\"frameId\":\"SUB1\",\"resourceType\":\"Document\",\"redirectedRequestId\":\"F1\"}" ) );
            Pump( 20 );
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.failRequest", "", params ) && params.GetAttributeValue( "requestId" ).AsString() == "F3" );
            const WEB::CNavigationRefusal& redirect = guard.GetRecentRefusals().back();
            ASSERT_TRUE( redirect.source == "redirect" && redirect.reason == "not-allowed" && !redirect.isMainFrame );

            // an address reached without a request: reported once, not blocked
            browser.Emit( "", "Target.targetInfoChanged", Json( "{\"targetInfo\":{\"targetId\":\"P1\",\"type\":\"page\",\"url\":\"https://www.bank.test/app/trade/inpage\",\"attached\":true}}" ) );
            browser.Emit( "", "Target.targetInfoChanged", Json( "{\"targetInfo\":{\"targetId\":\"P1\",\"type\":\"page\",\"url\":\"https://www.bank.test/app/trade/inpage\",\"attached\":true}}" ) );
            Pump( 20 );
            ASSERT_TRUE( guard.GetRefusedCount() == 3 );
            const WEB::CNavigationRefusal& committed = guard.GetRecentRefusals().back();
            ASSERT_TRUE( committed.source == "committed" && !committed.blocked );

            // a paused request of a page session is not the guard's
            browser.Emit( "S-P1", "Fetch.requestPaused", Json( "{\"requestId\":\"F4\",\"request\":{\"url\":\"https://evil.test/\",\"method\":\"GET\"},\"frameId\":\"P1\",\"resourceType\":\"Document\"}" ) );
            Pump( 20 );
            ASSERT_TRUE( guard.GetRefusedCount() == 3 );

            // the page driver asks the guard first and sends nothing for a refused address
            WEB::CDevToolsPage page( client, "S-P1" );
            page.SetNavigationGuard( &guard );
            ASSERT_FALSE( page.Navigate( "https://www.bank.test/app/trade" ) );
            ASSERT_TRUE( browser.CountCommands( "Page.navigate", "*" ) == 0 );
            ASSERT_TRUE( guard.GetRecentRefusals().back().source == "navigate" );
            ASSERT_TRUE( page.Navigate( "https://www.bank.test/app/positions" ) );
            ASSERT_TRUE( browser.CountCommands( "Page.navigate", "S-P1" ) == 1 );

            guard.Stop();
            Pump( 10 );
            ASSERT_TRUE( browser.WasCommandSent( "Fetch.disable", "" ) );
            ASSERT_FALSE( guard.IsStarted() );
            guard.Unsubscribe( &counter );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDownloadCaptureTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Download file names are sanitised and content types sniffed" )
        try
        {
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "Lot-Details 123.csv", 100 ) == "Lot-Details_123.csv" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "../../etc/passwd", 100 ) == "passwd" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "..\\x\\.hidden.csv", 100 ) == "hidden.csv" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "CON.txt", 100 ) == "file-CON.txt" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "", 100 ) == "download" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "...", 100 ) == "download" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "a  b\t$c.csv", 100 ) == "a_b_c.csv" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SanitizeFileName( "r\xC3\xA9sum\xC3\xA9.pdf", 100 ) == "r_sum_.pdf" );
            const CORE::CString longName = WEB::CBrowserDownloadCapture::SanitizeFileName( "abcdefghijklmnopqrstuvwxyz0123456789abcdefghij.csv", 20 );
            ASSERT_TRUE( longName.Length() == 20 && longName.EndsWith( ".csv" ) );

            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.pdf", "%PDF-1.7", 8 ) == "application/pdf" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.csv", "\n <!DOCTYPE html><html>", 23 ) == "text/html" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.csv", "a,b\n1,2\n", 8 ) == "text/csv" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.json", "{\"a\":1}", 7 ) == "application/json" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.xlsx", "PK\x03\x04rest", 8 ) == "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.bin", "\x01\x02\x00\x7f", 4 ) == "application/octet-stream" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::SniffMimeType( "x.txt", "plain words", 11 ) == "text/plain" );

            CORE::CString::StringVector allowed;
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::IsMimeTypeAllowed( allowed, "anything/at-all" ) );
            allowed.push_back( "text/csv" );
            allowed.push_back( "application/*" );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::IsMimeTypeAllowed( allowed, "Text/CSV; charset=utf-8" ) );
            ASSERT_TRUE( WEB::CBrowserDownloadCapture::IsMimeTypeAllowed( allowed, "application/pdf" ) );
            ASSERT_FALSE( WEB::CBrowserDownloadCapture::IsMimeTypeAllowed( allowed, "text/html" ) );

            WEB::CBrowserDownloadSettings settings;
            ASSERT_TRUE( settings.LoadConfig( Json( "{\"allowedExtensions\":[\".CSV\",\"pdf\"],\"maxBytes\":1000,\"minBytes\":2}" ) ) );
            ASSERT_TRUE( settings.allowedExtensions.size() == 2 && settings.allowedExtensions[ 0 ] == "csv" && settings.maxBytes == 1000 && settings.minBytes == 2 );
            ASSERT_FALSE( settings.LoadConfig( Json( "{\"maxFileNameLength\":4}" ) ) );
            ASSERT_FALSE( settings.LoadConfig( Json( "{\"minBytes\":5000}" ) ) );
            ASSERT_TRUE( settings.maxFileNameLength == 100 && settings.minBytes == 2 );
            CORE::CDataNode saved;
            ASSERT_TRUE( settings.SaveConfig( saved ) );
            WEB::CBrowserDownloadSettings again;
            ASSERT_TRUE( again.LoadConfig( saved ) && again.allowedExtensions == settings.allowedExtensions && again.maxBytes == 1000 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "https://www.bank.test/app/home" );

    GUCEF_TESTFW_TESTCASE( "Test: The download capture checks, hashes and renames downloads into an owner-only run folder" )
        try
        {
            ASSERT_TRUE( started );
            TESTFW::CScopedTestDir dir( "gucefWEB_BrowserAutomation_Download" );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CBrowserDownloadCapture capture( client );
            WEB::CBrowserDownloadSettings settings;
            settings.allowedExtensions.push_back( "csv" );
            settings.maxBytes = 1000;
            capture.SetSettings( settings );
            ASSERT_FALSE( capture.ExpectDownload( 100 ) );       // not capturing yet

            const CORE::CString run = CORE::CombinePath( dir.GetPath(), "run" );
            const CORE::CString incoming = CORE::CombinePath( run, "incoming" );
            ASSERT_TRUE( capture.Begin( run ) );
            ASSERT_TRUE( capture.IsCapturing() && capture.GetRunDirectory() == run );
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Browser.setDownloadBehavior", "", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "behavior" ).AsString() == "allowAndName" );
            ASSERT_TRUE( params.GetAttributeValue( "downloadPath" ).AsString() == incoming );
            ASSERT_TRUE( params.GetAttributeValue( "eventsEnabled" ).AsBool( false ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( ModeOf( run ) == 0700 && ModeOf( incoming ) == 0700 );
            #endif

            // the server's content type, seen on a watched page session
            ASSERT_TRUE( capture.WatchSession( "S-P1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Network.enable", "S-P1" ) );
            browser.Emit( "S-P1", "Network.responseReceived", Json( "{\"requestId\":\"R9\",\"type\":\"Document\",\"response\":{\"url\":\"https://www.bank.test/app/export?acct=PLANTEDqueryTOKEN1\","
                                                                     "\"status\":200,\"mimeType\":\"text/csv\",\"headers\":{}}}" ) );
            ASSERT_TRUE( capture.ExpectDownload( 5000 ) );
            ASSERT_TRUE( capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_PENDING );
            browser.Emit( "", "Browser.downloadWillBegin", Json( "{\"frameId\":\"P1\",\"guid\":\"G1\",\"url\":\"https://www.bank.test/app/export?acct=PLANTEDqueryTOKEN1\","
                                                                  "\"suggestedFilename\":\"Lot Details 2026.csv\"}" ) );
            Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.GetInProgressCount() == 1 );
            const CORE::CString content = "Symbol,Quantity\nAAA,1\n";
            ASSERT_TRUE( WriteFile( CORE::CombinePath( incoming, "G1" ), content ) );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G1\",\"totalBytes\":22,\"receivedBytes\":22,\"state\":\"inProgress\"}" ) );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G1\",\"totalBytes\":22,\"receivedBytes\":22,\"state\":\"completed\",\"filePath\":\"" +
                                                                 CORE::CombinePath( incoming, "G1" ) + "\"}" ) );
            Pump( 20, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_DONE );
            WEB::CBrowserDownloadResult result;
            ASSERT_TRUE( capture.PopResult( result ) );
            ASSERT_TRUE( result.state == WEB::CBrowserDownloadResult::STATE_COMPLETED );
            ASSERT_TRUE( result.finalPath == CORE::CombinePath( run, "Lot_Details_2026.csv" ) && CORE::FileExists( result.finalPath ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( incoming, "G1" ) ) );
            ASSERT_TRUE( result.bytes == 22 && result.suggestedFilename == "Lot Details 2026.csv" );
            ASSERT_TRUE( result.mimeType == "text/csv" && result.mimeTypeFromServer && result.sniffedMimeType == "text/csv" );
            ASSERT_TRUE( result.urlTemplate == "https://www.bank.test/app/export" && result.frameId == "P1" );
            ASSERT_TRUE( result.sha256 == Sha256Hex( content ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( ModeOf( result.finalPath ) == 0600 );
            #endif

            // the same name again gets a number
            browser.Emit( "", "Browser.downloadWillBegin", Json( "{\"frameId\":\"P1\",\"guid\":\"G2\",\"url\":\"blob:https://www.bank.test/5f0c\",\"suggestedFilename\":\"Lot Details 2026.csv\"}" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( incoming, "G2" ), content ) );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G2\",\"totalBytes\":22,\"receivedBytes\":22,\"state\":\"completed\"}" ) );
            Pump( 20, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.PopResult( result ) && result.state == WEB::CBrowserDownloadResult::STATE_COMPLETED );
            ASSERT_TRUE( result.finalPath == CORE::CombinePath( run, "Lot_Details_2026-2.csv" ) );
            ASSERT_TRUE( result.urlTemplate == "blob:" && !result.mimeTypeFromServer && result.mimeType == "text/csv" );

            // too large: canceled while it comes in
            browser.Emit( "", "Browser.downloadWillBegin", Json( "{\"frameId\":\"P1\",\"guid\":\"G3\",\"url\":\"https://www.bank.test/app/big\",\"suggestedFilename\":\"big.csv\"}" ) );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G3\",\"totalBytes\":5000,\"receivedBytes\":100,\"state\":\"inProgress\"}" ) );
            Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( browser.GetLastCommandParams( "Browser.cancelDownload", "", params ) && params.GetAttributeValue( "guid" ).AsString() == "G3" );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G3\",\"totalBytes\":5000,\"receivedBytes\":100,\"state\":\"canceled\"}" ) );
            Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.PopResult( result ) && result.state == WEB::CBrowserDownloadResult::STATE_REFUSED && result.failureReason == "too-large" );
            ASSERT_TRUE( result.finalPath.IsNULLOrEmpty() );

            // an HTML page instead of the file, a wrong extension, an empty file: refused and deleted
            const char* const cases[][ 3 ] = { { "G4", "positions.csv", "<!DOCTYPE html><html><body>Sign in</body></html>" },
                                               { "G5", "report.pdf", "%PDF-1.7 not much" },
                                               { "G6", "empty.csv", "" } };
            const char* const reasons[] = { "html-instead-of-file", "extension-not-allowed", "too-small" };
            for ( UInt32 c=0; c<3; ++c )
            {
                browser.Emit( "", "Browser.downloadWillBegin", Json( CORE::CString( "{\"frameId\":\"P1\",\"guid\":\"" ) + cases[ c ][ 0 ] +
                                                                      "\",\"url\":\"https://www.bank.test/app/x\",\"suggestedFilename\":\"" + cases[ c ][ 1 ] + "\"}" ) );
                const CORE::CString staged = CORE::CombinePath( incoming, cases[ c ][ 0 ] );
                ASSERT_TRUE( WriteFile( staged, cases[ c ][ 2 ] ) );
                browser.Emit( "", "Browser.downloadProgress", Json( CORE::CString( "{\"guid\":\"" ) + cases[ c ][ 0 ] + "\",\"state\":\"completed\"}" ) );
                Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
                ASSERT_TRUE( capture.PopResult( result ) );
                ASSERT_TRUE( result.state == WEB::CBrowserDownloadResult::STATE_REFUSED && result.failureReason == reasons[ c ] );
                ASSERT_FALSE( CORE::FileExists( staged ) );
            }

            // only the allowed content types
            settings.allowedMimeTypes.push_back( "text/csv" );
            capture.SetSettings( settings );
            browser.Emit( "S-P1", "Network.responseReceived", Json( "{\"requestId\":\"R10\",\"type\":\"Document\",\"response\":{\"url\":\"https://www.bank.test/app/bin\","
                                                                     "\"status\":200,\"mimeType\":\"application/octet-stream\",\"headers\":{}}}" ) );
            browser.Emit( "", "Browser.downloadWillBegin", Json( "{\"frameId\":\"P1\",\"guid\":\"G7\",\"url\":\"https://www.bank.test/app/bin\",\"suggestedFilename\":\"x.csv\"}" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( incoming, "G7" ), content ) );
            browser.Emit( "", "Browser.downloadProgress", Json( "{\"guid\":\"G7\",\"state\":\"completed\"}" ) );
            Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.PopResult( result ) && result.failureReason == "mime-type-not-allowed" );

            // a wait without a download times out
            ASSERT_TRUE( capture.ExpectDownload( 30 ) );
            for ( UInt32 i=0; i<100 && capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_PENDING; ++i )
                Pump( 1, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_TIMED_OUT );

            // afterwards downloads are refused by the browser
            ASSERT_TRUE( capture.End( true ) );
            ASSERT_TRUE( browser.GetLastCommandParams( "Browser.setDownloadBehavior", "", params ) && params.GetAttributeValue( "behavior" ).AsString() == "deny" );
            ASSERT_FALSE( capture.IsCapturing() );
            browser.Emit( "", "Browser.downloadWillBegin", Json( "{\"frameId\":\"P1\",\"guid\":\"G8\",\"url\":\"https://www.bank.test/app/x\",\"suggestedFilename\":\"late.csv\"}" ) );
            Pump( 10, GUCEF_NULL, GUCEF_NULL, &capture );
            ASSERT_TRUE( capture.GetInProgressCount() == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformResponseTapTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Response taps match method, host and path template, and refuse unusable arguments" )
        try
        {
            WEB::CUrlPathTemplate parsed;
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "https://api.bank.test/api/accounts/123456/positions?token=x", parsed ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "GET", "*.bank.test", "/api/accounts/{n}/positions", "GET", parsed ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "get", "api.bank.test", "/api/**", "GET", parsed ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "*", "API.BANK.TEST", "/api/accounts/**", "POST", parsed ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "", "api.bank.test", "/api/accounts/{n}/positions", "PUT", parsed ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "POST", "*.bank.test", "/api/accounts/{n}/positions", "GET", parsed ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "GET", "*.bank.test", "/api/accounts/123456/positions", "GET", parsed ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "GET", "*.bank.test", "/API/accounts/{n}/positions", "GET", parsed ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "GET", "other.test", "/api/**", "GET", parsed ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "GET", "api.bank.test", "/api/*", "GET", parsed ) );     // one segment only
            WEB::CUrlPathTemplate withPort;
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "https://api.bank.test:8443/api/x", withPort ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "GET", "api.bank.test", "/api/x", "GET", withPort ) );
            ASSERT_TRUE( WEB::CWebResponseTap::IsMatch( "GET", "api.bank.test:8443", "/api/x", "GET", withPort ) );
            ASSERT_FALSE( WEB::CWebResponseTap::IsMatch( "GET", "api.bank.test:9", "/api/x", "GET", withPort ) );

            WEB::CChromeDevToolsClient client;
            WEB::CWebResponseTap tap( client );
            CBodyCollector sink;
            ASSERT_TRUE( 0 == tap.AddTap( "GET", "api.bank.test", "/api/*", GUCEF_NULL, 100 ) );
            ASSERT_TRUE( 0 == tap.AddTap( "GET", "", "/api/*", &sink, 100 ) );
            ASSERT_TRUE( 0 == tap.AddTap( "GET", "api.bank.test", "/api/*", &sink, 0 ) );
            ASSERT_TRUE( 0 == tap.AddTap( "GET", "api.bank.test", "/api/*", &sink, WEB::CWebResponseTap::MaxBodyBytesCeiling + 1 ) );
            ASSERT_TRUE( 0 == tap.AddTap( "GET", "api.bank.test", "/api/[", &sink, 100 ) );
            const UInt32 id = tap.AddTap( "GET", "api.bank.test", "/api/*", &sink, 100 );
            ASSERT_TRUE( 0 != id && tap.GetTapCount() == 1 );
            ASSERT_FALSE( tap.Start() );        // no connection
            ASSERT_TRUE( tap.RemoveTap( id ) && !tap.RemoveTap( id ) && tap.GetTapCount() == 0 );
            ASSERT_FALSE( tap.WaitForResponse( id, 10 ) );
            ASSERT_TRUE( tap.GetWaitState( id ) == WEB::CWebResponseTap::WAIT_NONE );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "https://client.bank.test/app/positions" );
    browser.AddTarget( "P2", "page", "https://mail.other.test/inbox" );

    GUCEF_TESTFW_TESTCASE( "Test: A response tap hands over the bodies the page received, in memory, and nothing else" )
        try
        {
            ASSERT_TRUE( started );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CWebResponseTap tap( client );
            CORE::CString::StringVector pages;
            pages.push_back( "bank.test" );
            tap.SetPageHostFilter( pages );
            CBodyCollector sink;
            const UInt32 positions = tap.AddTap( "GET", "api.bank.test", "/api/accounts/{n}/positions", &sink, 1024 );
            const UInt32 lots = tap.AddTap( "POST", "*.bank.test", "/api/lots**", &sink, 16 );
            const UInt32 anything = tap.AddTap( "*", "api.bank.test", "/api/**", &sink, 1024 );
            ASSERT_TRUE( 0 != positions && 0 != lots && 0 != anything );
            ASSERT_TRUE( tap.Start() );
            Pump( 50, GUCEF_NULL, &tap );

            // only the tab of the site, with network events and its frames
            ASSERT_TRUE( browser.WasAttachedTo( "P1" ) && !browser.WasAttachedTo( "P2" ) );
            ASSERT_TRUE( tap.GetSessionCount() == 1 );
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Network.enable", "S-P1", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "maxResourceBufferSize" ).AsUInt32( 0 ) == WEB::CWebResponseTap::MaxBodyBytesCeiling );
            ASSERT_TRUE( browser.WasCommandSent( "Target.setAutoAttach", "S-P1" ) );

            ASSERT_TRUE( tap.WaitForResponse( positions, 5000 ) );
            ASSERT_TRUE( tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_PENDING );
            const CORE::CString body = "{\"accounts\":[{\"marketValue\":\"PLANTEDbodyVALUE11\"}]}";
            browser.SetResponseBody( "R1", body );
            EmitRequestCycle( browser, "S-P1", "R1", "GET", "https://api.bank.test/api/accounts/12345678/positions?token=PLANTEDqueryTOKEN1", "application/json", 40 );
            Pump( 30, GUCEF_NULL, &tap );

            // both taps that match get it, the path template carries no query
            ASSERT_TRUE( sink.bodies.size() == 2 );
            ASSERT_TRUE( browser.CountCommands( "Network.getResponseBody", "S-P1" ) == 1 );
            if ( sink.bodies.size() == 2 )
            {
                ASSERT_TRUE( sink.bodies[ 0 ].text == body && sink.bodies[ 1 ].text == body );
                ASSERT_TRUE( sink.bodies[ 0 ].method == "GET" && sink.bodies[ 0 ].host == "api.bank.test" );
                ASSERT_TRUE( sink.bodies[ 0 ].pathTemplate == "/api/accounts/{n}/positions" && sink.bodies[ 0 ].status == 200 );
                ASSERT_TRUE( sink.bodies[ 0 ].mimeType == "application/json" && sink.bodies[ 0 ].requestSeq == 1 );
                ASSERT_TRUE( sink.bodies[ 0 ].tapId == positions && sink.bodies[ 1 ].tapId == anything );
            }
            ASSERT_TRUE( tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_RECEIVED && tap.GetDeliveredCount( positions ) == 1 );

            // what no tap names is not fetched, nor is anything of a sign-in
            EmitRequestCycle( browser, "S-P1", "R2", "GET", "https://cdn.bank.test/api/fonts", "application/json", 40 );
            browser.SetResponseBody( "R3", "{\"session\":\"PLANTEDsigninPASSWORD8\"}" );
            EmitRequestCycle( browser, "S-P1", "R3", "GET", "https://api.bank.test/api/login/status", "application/json", 40 );
            Pump( 30, GUCEF_NULL, &tap );
            ASSERT_TRUE( browser.CountCommands( "Network.getResponseBody", "S-P1" ) == 1 );
            ASSERT_TRUE( sink.bodies.size() == 2 );

            // too large: not fetched at all, or fetched and not delivered
            ASSERT_TRUE( tap.WaitForResponse( lots, 5000 ) );
            EmitRequestCycle( browser, "S-P1", "R4", "POST", "https://api.bank.test/api/lots/export", "text/csv", 5000 );
            Pump( 30, GUCEF_NULL, &tap );
            ASSERT_TRUE( tap.GetWaitState( lots ) == WEB::CWebResponseTap::WAIT_TOO_LARGE );
            ASSERT_TRUE( tap.GetDeliveredCount( lots ) == 0 && tap.GetDeliveredCount( anything ) == 1 );
            ASSERT_TRUE( tap.WaitForResponse( lots, 5000 ) );
            browser.SetResponseBody( "R5", "0123456789012345678901234567890123456789" );
            EmitRequestCycle( browser, "S-P1", "R5", "POST", "https://api.bank.test/api/lots", "text/csv", 10 );
            Pump( 30, GUCEF_NULL, &tap );
            ASSERT_TRUE( tap.GetWaitState( lots ) == WEB::CWebResponseTap::WAIT_TOO_LARGE && tap.GetDeliveredCount( lots ) == 0 );

            // a wait with nothing new times out; a body that came before the wait counts for it
            ASSERT_TRUE( tap.WaitForResponse( positions, 30 ) );
            for ( UInt32 i=0; i<100 && tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_PENDING; ++i )
                Pump( 1, GUCEF_NULL, &tap );
            ASSERT_TRUE( tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_TIMED_OUT );
            browser.SetResponseBody( "R6", "{}" );
            EmitRequestCycle( browser, "S-P1", "R6", "GET", "https://api.bank.test/api/accounts/87654321/positions", "application/json", 2 );
            Pump( 30, GUCEF_NULL, &tap );
            ASSERT_TRUE( tap.WaitForResponse( positions, 5000 ) );
            Pump( 2, GUCEF_NULL, &tap );
            ASSERT_TRUE( tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_RECEIVED );

            // the events of another session are not the tap's
            const size_t before = sink.bodies.size();
            browser.SetResponseBody( "X1", "{}" );
            EmitRequestCycle( browser, "S-OTHER", "X1", "GET", "https://api.bank.test/api/accounts/1234/positions", "application/json", 2 );
            Pump( 30, GUCEF_NULL, &tap );
            ASSERT_TRUE( sink.bodies.size() == before );
            tap.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A response tap works next to the recorder and its bodies never reach the journal" )
        try
        {
            browser.ForgetConnection();
            TESTFW::CScopedTestDir dir( "gucefWEB_BrowserAutomation_TapAndRecorder" );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "bank.test" );
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &journal );
            ASSERT_TRUE( recorder.Start() );

            WEB::CWebResponseTap tap( client );
            CBodyCollector sink;
            const UInt32 id = tap.AddTap( "GET", "api.bank.test", "/api/accounts/{n}/positions", &sink, 1024 );
            ASSERT_TRUE( tap.Start() );
            for ( UInt32 i=0; i<50; ++i )
                Pump( 1, &recorder, &tap );

            const CORE::CString body = "{\"accounts\":[{\"marketValue\":\"PLANTEDbodyVALUE11\"}]}";
            browser.SetResponseBody( "T1", body );
            EmitRequestCycle( browser, "S-P1", "T1", "GET", "https://api.bank.test/api/accounts/12345678/positions", "application/json", 40 );
            for ( UInt32 i=0; i<50; ++i )
                Pump( 1, &recorder, &tap );
            ASSERT_TRUE( tap.GetDeliveredCount( id ) == 1 && sink.bodies.size() == 1 );
            ASSERT_TRUE( recorder.GetRecordedRequestCount() >= 1 );
            recorder.Stop();
            tap.Stop();
            journal.Close();

            // the recorder keeps its shape-only rules; the value the tap handed over is in no file
            CORE::CString journalText;
            ASSERT_TRUE( CORE::LoadTextFileAsString( journalPath, journalText ) );
            ASSERT_TRUE( Contains( journalText, "/api/accounts/{n}/positions" ) );
            ASSERT_FALSE( Contains( journalText, "PLANTEDbodyVALUE11" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPageDriverTests( void )
{
    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "https://www.bank.test/app/home" );

    GUCEF_TESTFW_TESTCASE( "Test: The page driver clicks, types, chooses options and reads attributes through the protocol" )
        try
        {
            ASSERT_TRUE( started );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );
            WEB::CDevToolsPage page( client );
            ASSERT_TRUE( page.AttachToTarget( "P1" ) && page.GetSessionId() == "S-P1" );

            browser.SetCommandResult( "DOM.getDocument", Json( "{\"root\":{\"nodeId\":1,\"backendNodeId\":1,\"nodeName\":\"#document\"}}" ) );
            browser.SetCommandResult( "DOM.querySelector##export", Json( "{\"nodeId\":5}" ) );
            browser.SetCommandResult( "DOM.querySelector##missing", Json( "{\"nodeId\":0}" ) );
            browser.SetCommandResult( "DOM.getContentQuads", Json( "{\"quads\":[[10,20,30,20,30,40,10,40]]}" ) );

            // a trusted click at the centre of the element
            ASSERT_TRUE( page.Click( "#export" ) );
            ASSERT_TRUE( browser.CountCommands( "Input.dispatchMouseEvent", "S-P1" ) == 3 );
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Input.dispatchMouseEvent", "S-P1", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "type" ).AsString() == "mouseReleased" && params.GetAttributeValue( "button" ).AsString() == "left" );
            ASSERT_TRUE( params.GetAttributeValue( "x" ).AsFloat64( 0.0 ) == 20.0 && params.GetAttributeValue( "y" ).AsFloat64( 0.0 ) == 30.0 );
            ASSERT_TRUE( browser.WasCommandSent( "DOM.scrollIntoViewIfNeeded", "S-P1" ) );
            ASSERT_FALSE( page.Click( "#missing" ) );
            ASSERT_TRUE( browser.CountCommands( "Input.dispatchMouseEvent", "S-P1" ) == 3 );
            ASSERT_TRUE( client.GetSessionId().IsNULLOrEmpty() );     // the client's session is put back

            // an accessible name finds the control, not the text in it
            browser.SetCommandResult( "Accessibility.queryAXTree#Export lots",
                                      Json( "{\"nodes\":[{\"nodeId\":\"9\",\"ignored\":false,\"role\":{\"type\":\"role\",\"value\":\"StaticText\"},\"backendDOMNodeId\":77},"
                                            "{\"nodeId\":\"8\",\"ignored\":false,\"role\":{\"type\":\"role\",\"value\":\"button\"},\"backendDOMNodeId\":55}]}" ) );
            browser.SetCommandResult( "DOM.pushNodesByBackendIdsToFrontend", Json( "{\"nodeIds\":[5]}" ) );
            ASSERT_TRUE( page.Click( "aria/Export lots" ) );
            ASSERT_TRUE( browser.GetLastCommandParams( "DOM.pushNodesByBackendIdsToFrontend", "S-P1", params ) );
            const CORE::CDataNode* ids = params.FindChild( "backendNodeIds" );
            ASSERT_TRUE( GUCEF_NULL != ids && ids->GetNrOfChildNodes() == 1 && ( *ids->ConstBegin() )->GetValue().AsInt64( 0 ) == 55 );

            // attributes, bounded; never the value of a field
            browser.SetCommandResult( "DOM.describeNode", Json( "{\"node\":{\"nodeId\":5,\"nodeName\":\"A\",\"attributes\":[\"href\",\"/app/export?id=1\",\"class\",\"button\"]}}" ) );
            CORE::CString value;
            ASSERT_TRUE( page.GetElementAttribute( "#export", "HREF", value ) && value == "/app/export?id=1" );
            ASSERT_FALSE( page.GetElementAttribute( "#export", "href", value, 5 ) );
            ASSERT_FALSE( page.GetElementAttribute( "#export", "title", value ) );
            browser.SetCommandResult( "DOM.describeNode", Json( "{\"node\":{\"nodeId\":5,\"nodeName\":\"INPUT\",\"attributes\":[\"type\",\"text\",\"value\",\"typed\"]}}" ) );
            ASSERT_FALSE( page.GetElementAttribute( "#export", "value", value ) );

            // typing replaces the content of a field, never of a password field
            browser.SetCommandResult( "DOM.resolveNode", Json( "{\"object\":{\"type\":\"object\",\"objectId\":\"O1\"}}" ) );
            browser.SetCommandResult( "Runtime.callFunctionOn", Json( "{\"result\":{\"type\":\"string\",\"value\":\"ok\"}}" ) );
            ASSERT_TRUE( page.TypeInto( "#export", "12/31/2026" ) );
            ASSERT_TRUE( browser.GetLastCommandParams( "Input.insertText", "S-P1", params ) && params.GetAttributeValue( "text" ).AsString() == "12/31/2026" );
            ASSERT_TRUE( browser.WasCommandSent( "DOM.focus", "S-P1" ) );
            browser.SetCommandResult( "DOM.describeNode", Json( "{\"node\":{\"nodeId\":5,\"nodeName\":\"INPUT\",\"attributes\":[\"type\",\"Password\"]}}" ) );
            const UInt32 typed = browser.CountCommands( "Input.insertText", "S-P1" );
            ASSERT_FALSE( page.TypeInto( "#export", "hunter2" ) );
            ASSERT_TRUE( browser.CountCommands( "Input.insertText", "S-P1" ) == typed );

            // an option by its label: the label travels as an argument, never inside the function
            ASSERT_TRUE( page.SelectOption( "#export", "Second acct'); alert('x" ) );
            ASSERT_TRUE( browser.GetLastCommandParams( "Runtime.callFunctionOn", "S-P1", params ) );
            ASSERT_FALSE( Contains( params.GetAttributeValue( "functionDeclaration" ).AsString(), "Second acct" ) );
            const CORE::CDataNode* arguments = params.FindChild( "arguments" );
            ASSERT_TRUE( GUCEF_NULL != arguments && arguments->GetNrOfChildNodes() == 1 &&
                         ( *arguments->ConstBegin() )->GetAttributeValue( "value" ).AsString() == "Second acct'); alert('x" );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.releaseObjectGroup", "S-P1" ) );
            browser.SetCommandResult( "Runtime.callFunctionOn", Json( "{\"result\":{\"type\":\"string\",\"value\":\"no-such-option\"}}" ) );
            ASSERT_FALSE( page.SelectOption( "#export", "Third acct" ) );

            // the address of the tab
            browser.SetCommandResult( "Page.getNavigationHistory", Json( "{\"currentIndex\":1,\"entries\":[{\"id\":1,\"url\":\"about:blank\"},{\"id\":2,\"url\":\"https://www.bank.test/app/now\"}]}" ) );
            CORE::CString url;
            ASSERT_TRUE( page.GetUrl( url ) && url == "https://www.bank.test/app/now" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The page driver waits for elements on changes of the DOM and lists controls without values or cell text" )
        try
        {
            browser.ForgetConnection();
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );
            WEB::CDevToolsPage page( client, "S-P1" );

            // waiting: an isolated world, a promise on the next change of the DOM, until the time is up
            browser.SetCommandResult( "DOM.querySelector##late", Json( "{\"nodeId\":0}" ) );
            browser.SetCommandResult( "Page.getFrameTree", Json( "{\"frameTree\":{\"frame\":{\"id\":\"P1\",\"url\":\"https://www.bank.test/app\"}}}" ) );
            browser.SetCommandResult( "Page.createIsolatedWorld", Json( "{\"executionContextId\":7}" ) );
            browser.SetCommandResult( "Runtime.evaluate", Json( "{\"result\":{\"type\":\"boolean\",\"value\":true}}" ) );
            ASSERT_FALSE( page.WaitForElement( "#late", 200 ) );
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Runtime.evaluate", "S-P1", params ) );
            ASSERT_TRUE( params.GetAttributeValue( "awaitPromise" ).AsBool( false ) && params.GetAttributeValue( "contextId" ).AsInt64( 0 ) == 7 );
            ASSERT_TRUE( Contains( params.GetAttributeValue( "expression" ).AsString(), "MutationObserver" ) );
            ASSERT_TRUE( browser.CountCommands( "Page.createIsolatedWorld", "S-P1" ) == 1 );
            browser.SetCommandResult( "DOM.querySelector##late", Json( "{\"nodeId\":9}" ) );
            ASSERT_TRUE( page.WaitForElement( "#late", 200 ) );
            ASSERT_TRUE( page.ElementExists( "#late" ) );

            // the controls: by role, in document order, nothing of a table cell, options of a select
            browser.SetCommandResult( "Accessibility.queryAXTree#button", Json(
                "{\"nodes\":[{\"nodeId\":\"a1\",\"ignored\":false,\"role\":{\"value\":\"button\"},\"name\":{\"value\":\"Export   lots\"},\"backendDOMNodeId\":101,\"properties\":[]},"
                "{\"nodeId\":\"a2\",\"ignored\":false,\"role\":{\"value\":\"button\"},\"name\":{\"value\":\"ACCT-9999\"},\"backendDOMNodeId\":102},"
                "{\"nodeId\":\"a3\",\"ignored\":false,\"role\":{\"value\":\"button\"},\"name\":{\"value\":\"Disabled one\"},\"backendDOMNodeId\":103,"
                "\"properties\":[{\"name\":\"disabled\",\"value\":{\"type\":\"boolean\",\"value\":true}}]}]}" ) );
            browser.SetCommandResult( "Accessibility.queryAXTree#link", Json(
                "{\"nodes\":[{\"nodeId\":\"a4\",\"ignored\":false,\"role\":{\"value\":\"link\"},\"name\":{\"value\":\"Second page\"},\"backendDOMNodeId\":104}]}" ) );
            browser.SetCommandResult( "Accessibility.queryAXTree#combobox", Json(
                "{\"nodes\":[{\"nodeId\":\"a5\",\"ignored\":false,\"role\":{\"value\":\"combobox\"},\"name\":{\"value\":\"Account\"},\"value\":{\"value\":\"First acct\"},\"backendDOMNodeId\":105}]}" ) );
            browser.SetCommandResult( "Accessibility.queryAXTree#checkbox", Json(
                "{\"nodes\":[{\"nodeId\":\"a6\",\"ignored\":true,\"role\":{\"value\":\"checkbox\"},\"name\":{\"value\":\"Remember\"},\"backendDOMNodeId\":106}]}" ) );
            browser.SetCommandResult( "Accessibility.queryAXTree#option", Json(
                "{\"nodes\":[{\"nodeId\":\"o1\",\"role\":{\"value\":\"option\"},\"name\":{\"value\":\"First acct\"},\"backendDOMNodeId\":111},"
                "{\"nodeId\":\"o2\",\"role\":{\"value\":\"option\"},\"name\":{\"value\":\"Second label\"},\"backendDOMNodeId\":112}]}" ) );
            browser.SetCommandResult( "Accessibility.queryAXTree", Json( "{\"nodes\":[]}" ) );
            browser.SetCommandResult( "DOM.pushNodesByBackendIdsToFrontend", Json( "{\"nodeIds\":[201,202,203,204,205,206]}" ) );
            browser.SetCommandResult( "DOM.querySelectorAll#*", Json( "{\"nodeIds\":[206,201,202,203,204,205]}" ) );
            browser.SetCommandResult( "DOM.querySelectorAll", Json( "{\"nodeIds\":[202]}" ) );
            browser.SetCommandResult( "DOM.describeNode", Json( "{\"node\":{\"nodeId\":205,\"nodeName\":\"SELECT\",\"attributes\":[\"aria-label\",\"Account\"]}}" ) );
            browser.SetCommandResult( "DOM.getContentQuads", Json( "{\"quads\":[[0,0,10,0,10,10,0,10]]}" ) );

            WEB::TControlInfoVector controls;
            ASSERT_TRUE( page.ListControls( CORE::CString::Empty, controls ) );
            ASSERT_TRUE( controls.size() == 6 );
            if ( controls.size() == 6 )
            {
                ASSERT_TRUE( controls[ 0 ].role == "checkbox" && controls[ 0 ].label == "Remember" && !controls[ 0 ].visible );
                ASSERT_TRUE( controls[ 1 ].role == "button" && controls[ 1 ].label == "Export lots" && controls[ 1 ].visible && controls[ 1 ].enabled );
                ASSERT_TRUE( controls[ 2 ].role == "button" && controls[ 2 ].label.IsNULLOrEmpty() && controls[ 2 ].inTableCell );
                ASSERT_TRUE( controls[ 3 ].label == "Disabled one" && !controls[ 3 ].enabled );
                ASSERT_TRUE( controls[ 4 ].role == "link" && controls[ 4 ].label == "Second page" );
                ASSERT_TRUE( controls[ 5 ].role == "select" && controls[ 5 ].label == "Account" && controls[ 5 ].optionLabels.size() == 2 );
                if ( controls[ 5 ].optionLabels.size() == 2 )
                    ASSERT_TRUE( controls[ 5 ].optionLabels[ 0 ] == "First acct" && controls[ 5 ].optionLabels[ 1 ] == "Second label" );
            }
            for ( size_t i=0; i<controls.size(); ++i )
                ASSERT_FALSE( Contains( controls[ i ].label, "9999" ) );

            // a scope that is not there
            browser.SetCommandResult( "DOM.querySelector##nowhere", Json( "{\"nodeId\":0}" ) );
            ASSERT_FALSE( page.ListControls( "#nowhere", controls ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformBrowserAutomationTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING browser automation TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "BrowserAutomation" );

    if ( !PluginsAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Browser automation TESTS skipped: the json or the sha1 codec plugin is not available" );
        return;
    }

    PerformClickListenerTests();
    PerformNavigationRuleTests();
    PerformNavigationGuardTests();
    PerformDownloadCaptureTests();
    PerformResponseTapTests();
    PerformPageDriverTests();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL browser automation TESTS COMPLETED" );
}
