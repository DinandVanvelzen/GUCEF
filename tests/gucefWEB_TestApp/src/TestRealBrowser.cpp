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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#include "gucefWEB_CDevToolsBrowser.h"
#include "gucefWEB_CChromeNetworkRecorder.h"
#include "gucefWEB_CNavigationGuard.h"
#include "gucefWEB_CBrowserDownloadCapture.h"
#include "gucefWEB_CWebResponseTap.h"
#include "gucefWEB_CDevToolsPage.h"
#include "gucefWEB_CWriteRequestGuard.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestRealBrowser.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The page's site and the site of its frame: different sites, so the frame is a cross-origin frame of its own process */
static const UInt16 MainSitePort = 47641;
static const UInt16 FrameSitePort = 47642;
static const UInt16 BrowserDebugPort = 47643;

/** Values of the page that must never show up in a label or a control list */
static const char* const PageSecrets[] = { "hunter2secret", "typed-value-77", "ACCT-9999", GUCEF_NULL };

static const char* const LotsCsv = "Symbol,Quantity,Cost\nAAA,3,12.50\nBBB,1,99.00\n";

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

static CORE::CString
Sha256Hex( const char* content )
{
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( content, (UInt32) strlen( content ), digest );
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

static CORE::CString
MainSite( void )
{
    return "http://site-a.test:" + CORE::UInt32ToString( MainSitePort );
}

/*-------------------------------------------------------------------------*/

/** The page of the main site: controls of every kind, values that must stay unseen, and a frame of the other site */
static CORE::CString
MainPage( void )
{
    return "<!DOCTYPE html><html><head><title>Automation test page</title></head><body>\n"
           "<button id=\"export\" aria-label=\"Export lots\" onclick=\"fetch('/api/accounts/12345678/positions?token=abc123').then(function(r){return r.json();})"
           ".then(function(){document.getElementById('out').textContent='loaded';});fetch('/api/login/status');\">E</button>\n"
           "<a id=\"section\" href=\"#lots\">Go to lots</a>\n"
           "<button id=\"route\" onclick=\"history.pushState({}, '', '/app/route/2')\">Route change</button>\n"
           "<a id=\"deny\" href=\"http://site-c.test:" + CORE::UInt32ToString( MainSitePort ) + "/trade/order\">Trade now</a>\n"
           "<a id=\"redir\" href=\"/redirect-out\">Redirect out</a>\n"
           "<a id=\"dl\" href=\"/download/lots\">Download lots</a>\n"
           "<button id=\"make-late\" onclick=\"setTimeout(function(){var b=document.createElement('button');b.id='late';b.textContent='Late button';"
           "document.body.appendChild(b);},400);\">Make late</button>\n"
           "<div role=\"tablist\"><div role=\"tab\" id=\"tab1\" tabindex=\"0\">Positions</div></div>\n"
           "<table><tr><td>ACCT-9999 <button id=\"cellbtn\">Row export</button></td></tr></table>\n"
           "<select id=\"acct\" aria-label=\"Account\"><option value=\"v1\">First acct</option><option value=\"v2\">Second acct</option></select>\n"
           "<label>Remember <input type=\"checkbox\" id=\"cb\"></label>\n"
           "<input id=\"pw\" type=\"password\" value=\"hunter2secret\">\n"
           "<input id=\"search\" type=\"text\" aria-label=\"Search\" value=\"typed-value-77\">\n"
           "<button id=\"hidden\" style=\"display:none\">Hidden one</button>\n"
           "<div id=\"out\"></div>\n"
           "<iframe id=\"frame\" src=\"http://site-b.test:" + CORE::UInt32ToString( FrameSitePort ) + "/frame\" width=\"400\" height=\"200\" style=\"border:0\"></iframe>\n"
           "<p id=\"lots\">lots</p>\n"
           "</body></html>";
}

/*-------------------------------------------------------------------------*/

/** The page of the write request guard test: requests of every kind, started by the test through the page's functions */
static CORE::CString
WritePage( void )
{
    return "<!DOCTYPE html><html><head><title>Write test page</title></head><body>\n"
           "<button id=\"popup\" onclick=\"window.open('/write-popup','_blank')\">Open popup</button>\n"
           "<iframe name=\"sink\" id=\"sink\" style=\"display:none\"></iframe>\n"
           "<form id=\"form\" method=\"POST\" action=\"/api/write/form\" target=\"sink\"><input name=\"a\" value=\"b\"></form>\n"
           "<script>\n"
           "function req(m,u,mode){return fetch(u,{method:m,body:(m==='GET'||m==='HEAD')?undefined:'x',mode:mode||'same-origin',cache:'no-store'})"
           ".then(function(r){return 'sent:'+r.status;},function(){return 'failed';});}\n"
           "function xhr(m,u){return new Promise(function(res){var x=new XMLHttpRequest();x.open(m,u);x.onload=function(){res('sent:'+x.status);};"
           "x.onerror=function(){res('failed');};x.send('x');});}\n"
           "function worker(){return new Promise(function(res){var w=new Worker('/worker.js');w.onmessage=function(e){res(e.data);};"
           "setTimeout(function(){res('timeout');},5000);});}\n"
           "function frame(){var f=document.createElement('iframe');f.src='http://site-b.test:" + CORE::UInt32ToString( FrameSitePort ) + "/frame-post';"
           "document.body.appendChild(f);return 'added';}\n"
           "</script></body></html>";
}

/*-------------------------------------------------------------------------*/

/** "METHOD path" of every request to /api/write/ the sites received, in order */
static CORE::CString::StringVector g_writesReceived;

static bool
WriteReceived( const char* methodAndPath )
{
    for ( size_t i=0; i<g_writesReceived.size(); ++i )
        if ( g_writesReceived[ i ] == methodAndPath )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static void
SetBody( WEB::CHttpResponseData& response, const CORE::CString& contentType, const CORE::CString& body )
{
    response.statusCode = 200;
    response.contentType = contentType;
    if ( !body.IsNULLOrEmpty() )
        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
}

/*-------------------------------------------------------------------------*/

namespace {

class CSiteHandlerBase : public WEB::CIHttpServerRequestHandler
{
    public:

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

namespace {

/** site-a.test (and site-c.test, which the guard refuses, on the same port) */
class CMainSiteHandler : public CSiteHandlerBase
{
    public:

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CString& path = request.requestUri;
        if ( path.StartsWith( "/api/write/" ) )
        {
            g_writesReceived.push_back( request.requestType + " " + path );
            if ( path == "/api/write/redirect" )
            {
                response.statusCode = 307;
                response.otherHeaders[ "Location" ] = "http://site-c.test:" + CORE::UInt32ToString( MainSitePort ) + "/api/write/redirected";
            }
            else
                SetBody( response, "text/plain", "ok" );
        }
        else
        if ( path == "/write" )
            SetBody( response, "text/html; charset=UTF-8", WritePage() );
        else
        if ( path == "/write-popup" )
            SetBody( response, "text/html; charset=UTF-8", "<!DOCTYPE html><html><body>popup<script>fetch('/api/write/frompopup',{method:'POST',body:'x'});</script></body></html>" );
        else
        if ( path == "/worker.js" )
            SetBody( response, "text/javascript", "fetch('/api/write/fromworker',{method:'POST',body:'x'}).then(function(){postMessage('sent');},function(){postMessage('failed');});" );
        else
        if ( path == "/" || path.StartsWith( "/app/" ) )
            SetBody( response, "text/html; charset=UTF-8", MainPage() );
        else
        if ( path == "/api/accounts/12345678/positions" )
            SetBody( response, "application/json", "{\"positions\":[{\"symbol\":\"AAA\",\"quantity\":3,\"marketValue\":\"PLANTEDbodyVALUE12\"}]}" );
        else
        if ( path == "/api/login/status" )
            SetBody( response, "application/json", "{\"signedIn\":true,\"sessionHint\":\"PLANTEDsigninVALUE13\"}" );
        else
        if ( path == "/redirect-out" )
        {
            response.statusCode = 302;
            response.otherHeaders[ "Location" ] = "http://site-c.test:" + CORE::UInt32ToString( MainSitePort ) + "/trade/redirected";
        }
        else
        if ( path == "/download/lots" )
        {
            SetBody( response, "text/csv", LotsCsv );
            response.otherHeaders[ "Content-Disposition" ] = "attachment; filename=\"Lot-Details 123.csv\"";
        }
        else
        if ( path.StartsWith( "/trade/" ) )
            SetBody( response, "text/html", "<html><body>TRADE PAGE</body></html>" );
        else
        if ( path == "/nocontent" )
            response.statusCode = 204;
        else
            response.statusCode = 404;
        return true;
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

namespace {

/** site-b.test: the frame, one big button */
class CFrameSiteHandler : public CSiteHandlerBase
{
    public:

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( request.requestUri == "/frame" )
            SetBody( response, "text/html; charset=UTF-8", "<!DOCTYPE html><html><body style=\"margin:0\">"
                     "<button id=\"fb\" style=\"width:400px;height:200px\">Frame action</button></body></html>" );
        else
        if ( request.requestUri == "/frame-post" )
            SetBody( response, "text/html; charset=UTF-8", "<!DOCTYPE html><html><body>frame<script>fetch('http://site-a.test:" + CORE::UInt32ToString( MainSitePort ) +
                     "/api/write/fromframe',{method:'POST',mode:'no-cors',body:'x'});</script></body></html>" );
        else
            response.statusCode = 404;
        return true;
    }
};

}; /* anonymous namespace */

typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CMainSiteHandler, MT::CMutex > TMainSiteHandlerFactory;
typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFrameSiteHandler, MT::CMutex > TFrameSiteHandlerFactory;

/*-------------------------------------------------------------------------*/

namespace {

class CActionCollector : public WEB::CIWebObservationSink
{
    public:

    GUCEF::vector< WEB::CObservedUserAction > actions;

    virtual void OnWebRequestObserved( const WEB::CObservedWebRequest& ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnUserActionObserved( const WEB::CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE { actions.push_back( action ); }

    bool HasClickLabel( const char* label ) const
    {
        for ( size_t i=0; i<actions.size(); ++i )
            if ( actions[ i ].kind == WEB::CObservedUserAction::KindClick && actions[ i ].label == label )
                return true;
        return false;
    }

    bool AnyLabelContains( const char* part ) const
    {
        for ( size_t i=0; i<actions.size(); ++i )
            if ( Contains( actions[ i ].label, part ) || Contains( actions[ i ].page, part ) )
                return true;
        return false;
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

namespace {

class CBodyCollector : public WEB::CIWebResponseSink
{
    public:

    CORE::CString::StringVector bodies;
    CORE::CString::StringVector endpoints;

    virtual void OnResponseBody( const WEB::CObservedResponseBody& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        bodies.push_back( CORE::CString( static_cast< const char* >( response.body.GetConstBufferPtr() ), response.body.GetDataSize() ) );
        endpoints.push_back( response.method + " " + response.host + response.pathTemplate + " " + CORE::Int32ToString( response.status ) + " " + response.mimeType );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

namespace {

/** Everything a test drives, pumped together on the application's pulse generator */
struct SRig
{
    WEB::CChromeDevToolsClient* client;
    WEB::CChromeNetworkRecorder* recorder;
    WEB::CWebResponseTap* tap;
    WEB::CBrowserDownloadCapture* capture;

    void Pump( UInt32 rounds )
    {
        for ( UInt32 i=0; i<rounds; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            recorder->Process();
            tap->Process();
            capture->Process();
            MT::ThreadDelay( 5 );
        }
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static void
PumpPlain( UInt32 rounds )
{
    for ( UInt32 i=0; i<rounds; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 5 );
    }
}

/*-------------------------------------------------------------------------*/

/** Runs an expression in the page and waits for its promise: the value as text */
static CORE::CString
EvaluateInPage( WEB::CChromeDevToolsClient& client, const CORE::CString& sessionId, const CORE::CString& expression )
{
    CORE::CDataNode result;
    client.SetSessionId( sessionId );
    const bool ok = client.RuntimeEvaluate( expression, true, result );
    client.SetSessionId( CORE::CString::Empty );
    const CORE::CString value = ok ? result.GetValue().AsString( CORE::CString::Empty, false ) : CORE::CString( "(not evaluated)" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: " << expression << " gave " << value );
    return value;
}

/*-------------------------------------------------------------------------*/

/** The refusal of the write request guard whose address holds the part, or GUCEF_NULL */
static const WEB::CWriteRequestRefusal*
FindRefusal( const WEB::CWriteRequestGuard& guard, const char* part )
{
    const WEB::TWriteRequestRefusalVector& refusals = guard.GetRecentRefusals();
    for ( size_t i=0; i<refusals.size(); ++i )
        if ( Contains( refusals[ i ].url, part ) )
            return &refusals[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static const WEB::CWriteRequestRefusal*
WaitForRefusal( const WEB::CWriteRequestGuard& guard, const char* part, UInt32 rounds = 1000 )
{
    for ( UInt32 i=0; i<rounds && GUCEF_NULL == FindRefusal( guard, part ); ++i )
        PumpPlain( 1 );
    return FindRefusal( guard, part );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRealBrowserTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING real browser TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "RealBrowser" );

    WEB::CDevToolsBrowser browser;
    const CORE::CString browserOverride = CORE::GetEnv( "GUCEF_TEST_BROWSER" );
    if ( !browserOverride.IsNULLOrEmpty() )
        browser.SetExecutablePath( browserOverride );
    if ( !PluginsAvailable() || browser.FindExecutable().IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Real browser TESTS skipped: no Chrome or Chromium found, or the json and hash codec plugins are not available" );
        return;
    }

    TMainSiteHandlerFactory mainFactory;
    TFrameSiteHandlerFactory frameFactory;
    WEB::CHttpServerSettings httpSettings;
    httpSettings.processRequestsAsync = false;
    httpSettings.keepAliveConnections = false;
    WEB::CHTTPServer mainSite( &mainFactory, &httpSettings );
    WEB::CHTTPServer frameSite( &frameFactory, &httpSettings );
    const bool listening = mainSite.ListenOnPort( MainSitePort ) && frameSite.ListenOnPort( FrameSitePort );

    TESTFW::CScopedTestDir dir( "gucefWEB_RealBrowser" );
    const CORE::CString profile = CORE::CombinePath( dir.GetPath(), "profile" );
    const CORE::CString run = CORE::CombinePath( dir.GetPath(), "run" );

    GUCEF_TESTFW_TESTCASE( "Test: A real browser reports labels, refuses navigations, gives downloads and hands over response bodies" )
        try
        {
            ASSERT_TRUE( listening );

            // headless, a profile of its own, never the network: the test names resolve to the loopback interface
            browser.SetAttachToRunning( false );
            browser.SetDebugPort( BrowserDebugPort );
            browser.SetProfileDir( profile );
            browser.SetHeadless( true );
            browser.AddArgument( "--host-resolver-rules=MAP site-a.test 127.0.0.1, MAP site-b.test 127.0.0.1, MAP site-c.test 127.0.0.1, EXCLUDE localhost" );
            browser.AddArgument( "--no-proxy-server" );
            browser.AddArgument( "--site-per-process" );
            browser.AddArgument( "--disable-background-networking" );
            browser.AddArgument( "--disable-component-update" );
            browser.AddArgument( "--disable-sync" );
            browser.AddArgument( "--disable-domain-reliability" );
            browser.AddArgument( "--no-pings" );
            browser.AddArgument( "--metrics-recording-only" );
            if ( !CORE::GetEnv( "GUCEF_TEST_CHROME_NO_SANDBOX" ).IsNULLOrEmpty() )
                browser.AddArgument( "--no-sandbox" );
            ASSERT_TRUE( browser.Launch( "about:blank" ) );

            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( browser.ConnectToBrowser( client ) );

            // the recorder with the click listener
            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "site-a.test" );
            settings.captureClicks = true;
            settings.recordResponseShapes = false;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            CActionCollector actions;
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &actions );
            ASSERT_TRUE( recorder.Start() );

            // the guard: the two sites, never a trade page
            WEB::CNavigationGuardSettings guardSettings;
            guardSettings.allow.push_back( "site-a.test" );
            guardSettings.allow.push_back( "site-b.test" );
            guardSettings.deny.push_back( "*.test/trade" );
            WEB::CNavigationGuard guard;
            ASSERT_TRUE( guard.SetSettings( guardSettings ) );
            ASSERT_TRUE( guard.Start( client ) );

            // downloads into an owner-only run folder
            WEB::CBrowserDownloadCapture capture( client );
            WEB::CBrowserDownloadSettings downloadSettings;
            downloadSettings.allowedExtensions.push_back( "csv" );
            downloadSettings.allowedMimeTypes.push_back( "text/csv" );
            capture.SetSettings( downloadSettings );
            ASSERT_TRUE( capture.Begin( run ) );

            // the response tap, on the positions and on everything of the API, sign-in included
            WEB::CWebResponseTap tap( client );
            CORE::CString::StringVector pages;
            pages.push_back( "site-a.test" );
            tap.SetPageHostFilter( pages );
            CBodyCollector bodies;
            const UInt32 positions = tap.AddTap( "GET", "site-a.test", "/api/accounts/{n}/positions", &bodies, 65536 );
            const UInt32 anyApi = tap.AddTap( "*", "site-a.test", "/api/**", &bodies, 65536 );
            ASSERT_TRUE( 0 != positions && 0 != anyApi );
            ASSERT_TRUE( tap.Start() );

            SRig rig;
            rig.client = &client;
            rig.recorder = &recorder;
            rig.tap = &tap;
            rig.capture = &capture;

            // the page driver on the open tab
            WEB::TChromeTargetVector targets;
            ASSERT_TRUE( client.TargetGetTargets( targets ) );
            CORE::CString targetId;
            for ( size_t i=0; i<targets.size(); ++i )
                if ( targets[ i ].type == "page" )
                    targetId = targets[ i ].id;
            ASSERT_FALSE( targetId.IsNULLOrEmpty() );
            WEB::CDevToolsPage page( client );
            ASSERT_TRUE( page.AttachToTarget( targetId ) );
            page.SetNavigationGuard( &guard );
            ASSERT_TRUE( capture.WatchSession( page.GetSessionId() ) );
            ASSERT_TRUE( page.Navigate( MainSite() + "/" ) );

            // the tab and its cross-origin frame are watched, with the listener in both
            for ( UInt32 i=0; i<600 && ( recorder.GetSessionCount() < 2 || tap.GetSessionCount() < 1 ); ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( page.WaitForElement( "#export", 10000 ) );
            for ( UInt32 i=0; i<600 && recorder.GetSessionCount() < 2; ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( recorder.GetSessionCount() >= 2 );
            rig.Pump( 40 );

            // a button by its aria-label; the page then calls its API: the tap hands the body over
            ASSERT_TRUE( tap.WaitForResponse( positions, 15000 ) );
            ASSERT_TRUE( page.Click( "#export" ) );
            for ( UInt32 i=0; i<600 && ( !actions.HasClickLabel( "Export lots" ) || tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_PENDING ); ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( actions.HasClickLabel( "Export lots" ) );
            ASSERT_TRUE( tap.GetWaitState( positions ) == WEB::CWebResponseTap::WAIT_RECEIVED );
            rig.Pump( 60 );
            ASSERT_TRUE( bodies.bodies.size() == 2 );     // the positions for both taps; never the sign-in status
            for ( size_t i=0; i<bodies.bodies.size(); ++i )
            {
                ASSERT_TRUE( Contains( bodies.bodies[ i ], "PLANTEDbodyVALUE12" ) );
                ASSERT_FALSE( Contains( bodies.bodies[ i ], "PLANTEDsigninVALUE13" ) );
                ASSERT_TRUE( Contains( bodies.endpoints[ i ], "GET site-a.test:" ) && Contains( bodies.endpoints[ i ], "/api/accounts/{n}/positions 200 application/json" ) );
                ASSERT_FALSE( Contains( bodies.endpoints[ i ], "abc123" ) );
            }

            // a link, an in-page navigation, a button of the cross-origin frame (the frame fills its element)
            ASSERT_TRUE( page.Click( "#section" ) );
            for ( UInt32 i=0; i<400 && !actions.HasClickLabel( "Go to lots" ); ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( actions.HasClickLabel( "Go to lots" ) );
            ASSERT_TRUE( page.Click( "#route" ) );
            for ( UInt32 i=0; i<400 && !actions.HasClickLabel( "Route change" ); ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( actions.HasClickLabel( "Route change" ) );
            ASSERT_TRUE( page.Click( "#frame" ) );
            for ( UInt32 i=0; i<400 && !actions.HasClickLabel( "Frame action" ); ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( actions.HasClickLabel( "Frame action" ) );

            // a field by its aria-label, a control in a table cell by its own name (never the text of the cell), never a value
            ASSERT_TRUE( page.Click( "#search" ) );
            ASSERT_TRUE( page.Click( "#cellbtn" ) );
            ASSERT_TRUE( page.Click( "#pw" ) );
            for ( UInt32 i=0; i<400 && !actions.HasClickLabel( "Row export" ); ++i )
                rig.Pump( 1 );
            rig.Pump( 40 );
            ASSERT_TRUE( actions.HasClickLabel( "Search" ) );
            ASSERT_TRUE( actions.HasClickLabel( "Row export" ) );
            for ( UInt32 s=0; GUCEF_NULL != PageSecrets[ s ]; ++s )
                ASSERT_FALSE( actions.AnyLabelContains( PageSecrets[ s ] ) );

            // the guard: a link to a trade page, a redirect to another site, a navigation of the program
            const UInt32 refusedBefore = guard.GetRefusedCount();
            ASSERT_TRUE( page.Click( "#deny" ) );
            for ( UInt32 i=0; i<400 && guard.GetRefusedCount() == refusedBefore; ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( guard.GetRefusedCount() == refusedBefore + 1 );
            if ( !guard.GetRecentRefusals().empty() )
            {
                const WEB::CNavigationRefusal& refused = guard.GetRecentRefusals().back();
                ASSERT_TRUE( refused.reason == "denied" && refused.source == "page" && refused.isMainFrame && refused.blocked );
                ASSERT_TRUE( Contains( refused.urlTemplate, "site-c.test" ) && Contains( refused.urlTemplate, "/trade/order" ) );
            }
            ASSERT_TRUE( page.Click( "#redir" ) );
            for ( UInt32 i=0; i<400 && guard.GetRefusedCount() == refusedBefore + 1; ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( guard.GetRefusedCount() == refusedBefore + 2 );
            if ( !guard.GetRecentRefusals().empty() )
                ASSERT_TRUE( guard.GetRecentRefusals().back().source == "redirect" );
            ASSERT_FALSE( page.Navigate( "http://site-c.test:" + CORE::UInt32ToString( MainSitePort ) + "/trade/direct" ) );
            ASSERT_TRUE( guard.GetRecentRefusals().back().source == "navigate" );
            rig.Pump( 40 );
            CORE::CString url;
            ASSERT_TRUE( page.GetUrl( url ) );
            ASSERT_TRUE( Contains( url, "site-a.test" ) && !Contains( url, "trade" ) );
            ASSERT_FALSE( page.ElementExists( "aria/TRADE PAGE" ) );

            // back to a fresh page for what follows
            ASSERT_TRUE( page.Navigate( MainSite() + "/" ) );
            ASSERT_TRUE( page.WaitForElement( "#dl", 10000 ) );
            rig.Pump( 20 );

            // a download: captured, checked, renamed, owner-only
            ASSERT_TRUE( capture.ExpectDownload( 15000 ) );
            ASSERT_TRUE( page.Click( "#dl" ) );
            for ( UInt32 i=0; i<1500 && capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_PENDING; ++i )
                rig.Pump( 1 );
            ASSERT_TRUE( capture.GetWaitState() == WEB::CBrowserDownloadCapture::WAIT_DONE );
            WEB::CBrowserDownloadResult download;
            ASSERT_TRUE( capture.PopResult( download ) );
            ASSERT_TRUE( download.state == WEB::CBrowserDownloadResult::STATE_COMPLETED );
            ASSERT_TRUE( download.finalPath == CORE::CombinePath( run, "Lot-Details_123.csv" ) && CORE::FileExists( download.finalPath ) );
            ASSERT_TRUE( download.suggestedFilename == "Lot-Details 123.csv" && download.bytes == strlen( LotsCsv ) );
            ASSERT_TRUE( download.mimeType == "text/csv" && download.mimeTypeFromServer );
            ASSERT_TRUE( download.sha256 == Sha256Hex( LotsCsv ) );
            ASSERT_TRUE( Contains( download.urlTemplate, "site-a.test" ) && Contains( download.urlTemplate, "/download/lots" ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( ModeOf( run ) == 0700 && ModeOf( download.finalPath ) == 0600 );
            #endif

            // the controls of the page: roles, labels, options; no values, no password field, no cell text
            WEB::TControlInfoVector controls;
            ASSERT_TRUE( page.ListControls( CORE::CString::Empty, controls ) );
            bool exportButton = false, link = false, tab = false, select = false, checkbox = false, cell = false, hidden = false;
            for ( size_t i=0; i<controls.size(); ++i )
            {
                const WEB::CControlInfo& c = controls[ i ];
                for ( UInt32 s=0; GUCEF_NULL != PageSecrets[ s ]; ++s )
                    ASSERT_FALSE( Contains( c.label, PageSecrets[ s ] ) );
                if ( c.role == "button" && c.label == "Export lots" && c.visible && c.enabled )
                    exportButton = true;
                if ( c.role == "link" && c.label == "Go to lots" )
                    link = true;
                if ( c.role == "tab" && c.label == "Positions" )
                    tab = true;
                if ( c.role == "select" && c.label == "Account" && c.optionLabels.size() == 2 && c.optionLabels[ 0 ] == "First acct" && c.optionLabels[ 1 ] == "Second acct" )
                    select = true;
                if ( c.role == "checkbox" && c.label == "Remember" )
                    checkbox = true;
                if ( c.role == "button" && c.inTableCell && c.label == "Row export" )
                    cell = true;
                if ( c.label == "Hidden one" && c.visible )
                    hidden = true;
            }
            ASSERT_TRUE( exportButton && link && tab && select && checkbox && cell );
            ASSERT_FALSE( hidden );

            // the names of the controls in table cells can be withheld
            page.SetReportTableCellControlNames( false );
            ASSERT_TRUE( page.ListControls( CORE::CString::Empty, controls ) );
            bool withheld = false;
            for ( size_t i=0; i<controls.size(); ++i )
                if ( controls[ i ].inTableCell && controls[ i ].label.IsNULLOrEmpty() && controls[ i ].role == "button" )
                    withheld = true;
            ASSERT_TRUE( withheld );
            page.SetReportTableCellControlNames( true );

            // an option, an attribute, an element that comes later
            ASSERT_TRUE( page.SelectOption( "#acct", "Second acct" ) );
            CORE::CDataNode chosen;
            client.SetSessionId( page.GetSessionId() );
            ASSERT_TRUE( client.RuntimeEvaluate( "document.getElementById('acct').value", false, chosen ) );
            client.SetSessionId( CORE::CString::Empty );
            ASSERT_TRUE( chosen.GetValue().AsString() == "v2" );
            ASSERT_FALSE( page.SelectOption( "#acct", "No such acct" ) );
            CORE::CString href;
            ASSERT_TRUE( page.GetElementAttribute( "#dl", "href", href ) && href == "/download/lots" );
            ASSERT_FALSE( page.GetElementAttribute( "#search", "value", href ) );
            ASSERT_TRUE( page.Click( "#make-late" ) );
            ASSERT_TRUE( page.WaitForElement( "#late", 10000 ) );
            ASSERT_TRUE( page.WaitForElement( "aria/Late button", 2000 ) );
            ASSERT_FALSE( page.WaitForElement( "#never", 300 ) );
            ASSERT_TRUE( page.TypeInto( "#search", "AAA" ) );
            ASSERT_FALSE( page.TypeInto( "#pw", "anything" ) );

            recorder.Stop();
            tap.Stop();
            guard.Stop();
            capture.End( true );
            rig.Pump( 20 );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A real browser sends no request other than GET, HEAD and OPTIONS that the write request guard does not name" )
        try
        {
            ASSERT_TRUE( listening && browser.WasLaunched() );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( browser.ConnectToBrowser( client ) );
            const CORE::CString port = CORE::UInt32ToString( MainSitePort );

            // installed and in force before the page is opened
            WEB::CNavigationGuardSettings navigationSettings;
            navigationSettings.allow.push_back( "site-a.test" );
            navigationSettings.allow.push_back( "site-b.test" );
            WEB::CNavigationGuard navigation;
            ASSERT_TRUE( navigation.SetSettings( navigationSettings ) );
            WEB::CWriteRequestGuardSettings writeSettings;
            writeSettings.allow.push_back( "POST http://site-a.test:" + port + "/api/write/allowed" );
            writeSettings.allow.push_back( "POST http://site-a.test:" + port + "/api/write/redirect" );
            WEB::CWriteRequestGuard writes;
            ASSERT_TRUE( writes.SetSettings( writeSettings ) );
            ASSERT_TRUE( navigation.SetWriteRequestGuard( &writes ) );
            ASSERT_TRUE( navigation.Start( client ) );
            ASSERT_TRUE( writes.IsEnforcing() );

            WEB::TChromeTargetVector targets;
            ASSERT_TRUE( client.TargetGetTargets( targets ) );
            CORE::CString targetId;
            for ( size_t i=0; i<targets.size() && targetId.IsNULLOrEmpty(); ++i )
                if ( targets[ i ].type == "page" )
                    targetId = targets[ i ].id;
            ASSERT_FALSE( targetId.IsNULLOrEmpty() );
            WEB::CDevToolsPage page( client );
            ASSERT_TRUE( page.AttachToTarget( targetId ) );
            page.SetNavigationGuard( &navigation );
            ASSERT_TRUE( page.Navigate( MainSite() + "/write" ) );
            ASSERT_TRUE( page.WaitForElement( "#popup", 10000 ) );
            const CORE::CString session = page.GetSessionId();

            // the proof on the page itself
            ASSERT_TRUE( writes.VerifyEnforcing( session, 10000 ) );
            g_writesReceived.clear();

            // reads and the one allowed write reach the site
            ASSERT_TRUE( EvaluateInPage( client, session, "req('GET','/api/accounts/12345678/positions')" ) == "sent:200" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/allowed')" ) == "sent:200" );
            ASSERT_TRUE( WriteReceived( "POST /api/write/allowed" ) );

            // writes no rule names never leave the browser: fetch, other methods, case, path and query differences, XHR
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/denied')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('PUT','/api/write/allowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('DELETE','/api/write/allowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('PATCH','/api/write/allowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('patch','/api/write/allowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/allowed/')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/allowed?x=1')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/%61llowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/API/write/allowed')" ) == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "xhr('POST','/api/write/xhr')" ) == "failed" );
            const WEB::CWriteRequestRefusal* refused = FindRefusal( writes, "/api/write/denied" );
            ASSERT_TRUE( GUCEF_NULL != refused );
            if ( GUCEF_NULL != refused )
                ASSERT_TRUE( refused->method == "POST" && refused->reason == "path-not-allowed" && ( refused->resourceType == "Fetch" || refused->resourceType == "XHR" ) && refused->isMainFrame );
            refused = FindRefusal( writes, "/api/write/xhr" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->resourceType == "XHR" );
            refused = FindRefusal( writes, "/api/write/%61llowed" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->reason == "ambiguous-path" );

            // dot segments: the browser resolves them before the request, the guard sees what the site would receive
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/x/../allowed')" ) == "sent:200" );

            // a beacon, a form post, a worker, a cross-origin frame and a window the page opens
            ASSERT_TRUE( EvaluateInPage( client, session, "navigator.sendBeacon('/api/write/beacon','x')" ) == "true" );
            refused = WaitForRefusal( writes, "/api/write/beacon" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->resourceType == "Ping" );
            EvaluateInPage( client, session, "document.getElementById('form').submit(),'submitted'" );
            refused = WaitForRefusal( writes, "/api/write/form" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->resourceType == "Document" && !refused->isMainFrame );
            const CORE::CString workerOutcome = EvaluateInPage( client, session, "worker()" );
            refused = WaitForRefusal( writes, "/api/write/fromworker", 400 );
            ASSERT_TRUE( GUCEF_NULL != refused );
            ASSERT_TRUE( workerOutcome == "failed" );
            ASSERT_TRUE( EvaluateInPage( client, session, "frame()" ) == "added" );
            refused = WaitForRefusal( writes, "/api/write/fromframe" );
            ASSERT_TRUE( GUCEF_NULL != refused && !refused->isMainFrame );
            ASSERT_TRUE( page.Click( "#popup" ) );
            refused = WaitForRefusal( writes, "/api/write/frompopup" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->isMainFrame && refused->frameId != targetId );

            // an allowed write redirected to another host: the first leg reaches the site, the second never leaves
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/redirect','no-cors')" ) == "failed" );
            refused = FindRefusal( writes, "/api/write/redirected" );
            ASSERT_TRUE( GUCEF_NULL != refused && refused->redirected && refused->reason == "host-not-allowed" && refused->host == "site-c.test:" + port );
            ASSERT_TRUE( WriteReceived( "POST /api/write/redirect" ) );

            // none of the refused requests reached a site
            PumpPlain( 40 );
            static const char* const neverReceived[] = { "POST /api/write/denied", "PUT /api/write/allowed", "DELETE /api/write/allowed", "PATCH /api/write/allowed",
                                                         "patch /api/write/allowed", "POST /api/write/allowed/", "POST /api/write/%61llowed", "POST /API/write/allowed",
                                                         "POST /api/write/xhr", "POST /api/write/beacon", "POST /api/write/form", "POST /api/write/fromworker",
                                                         "POST /api/write/fromframe", "POST /api/write/frompopup", "POST /api/write/redirected", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != neverReceived[ i ]; ++i )
            {
                if ( WriteReceived( neverReceived[ i ] ) )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: the site received " << neverReceived[ i ] );
                ASSERT_FALSE( WriteReceived( neverReceived[ i ] ) );
            }
            for ( size_t i=0; i<g_writesReceived.size(); ++i )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: the site received " << g_writesReceived[ i ] );

            // the control: without the guard the same write reaches the site
            ASSERT_TRUE( navigation.SetWriteRequestGuard( GUCEF_NULL ) );
            ASSERT_FALSE( writes.IsEnforcing() );
            ASSERT_FALSE( writes.VerifyEnforcing( session, 2000 ) );
            ASSERT_TRUE( EvaluateInPage( client, session, "req('POST','/api/write/denied')" ) == "sent:200" );
            ASSERT_TRUE( WriteReceived( "POST /api/write/denied" ) );

            navigation.Stop();
            PumpPlain( 20 );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: In a real browser Navigate returns once the new document is committed, also in a fresh tab" )
        try
        {
            ASSERT_TRUE( listening && browser.WasLaunched() );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( browser.ConnectToBrowser( client ) );
            const CORE::CString address = MainSite() + "/app/nav";

            // the old behaviour, for the record: back when the browser answers Page.navigate
            CORE::CString firstTab;
            ASSERT_TRUE( client.TargetCreate( "about:blank", firstTab ) );
            WEB::CDevToolsPage early( client );
            ASSERT_TRUE( early.AttachToTarget( firstTab ) );
            early.SetNavigationTimeout( 0 );
            CORE::CString url;
            ASSERT_TRUE( early.GetUrl( url ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: a fresh tab shows " << url );
            UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( early.Navigate( address + "?early=1" ) );
            // not asserted: the tab still shows the blank page, or is between two pages and does not answer at all
            const bool known = early.GetUrl( url );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: without the wait Navigate took " <<
                       (UInt64) ( ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 ) ) << " ms, the tab then showed " <<
                       ( known ? url : CORE::CString( "nothing (no answer)" ) ) );

            // with the wait: committed and loaded, the address is the new one right away and the controls are there
            CORE::CString secondTab;
            ASSERT_TRUE( client.TargetCreate( "about:blank", secondTab ) );
            WEB::CDevToolsPage page( client );
            ASSERT_TRUE( page.AttachToTarget( secondTab ) );
            ASSERT_TRUE( page.GetUrl( url ) && url == "about:blank" );
            start = MT::PrecisionTickCount();
            ASSERT_TRUE( page.Navigate( address ) );
            ASSERT_TRUE( page.GetUrl( url ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: with the wait Navigate took " <<
                       (UInt64) ( ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 ) ) << " ms, the tab then showed " << url );
            ASSERT_TRUE( url == address );
            ASSERT_TRUE( page.GetLastNavigationLoaded() && page.GetLastNavigationError().IsNULLOrEmpty() );
            WEB::TControlInfoVector controls;
            ASSERT_TRUE( page.ListControls( CORE::CString::Empty, controls ) );
            bool exportButton = false;
            for ( size_t i=0; i<controls.size(); ++i )
                if ( controls[ i ].label == "Export lots" )
                    exportButton = true;
            ASSERT_TRUE( exportButton );
            ASSERT_TRUE( page.GetLastListSkippedCount() == 0 );

            // within the document: a fragment
            start = MT::PrecisionTickCount();
            ASSERT_TRUE( page.Navigate( address + "#lots" ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: a navigation within the document took " <<
                       (UInt64) ( ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 ) ) << " ms" );
            ASSERT_TRUE( page.GetUrl( url ) && url == address + "#lots" );

            // a response without content: no commit, false without waiting out the time, the tab stays
            page.SetNavigationTimeout( 5000 );
            start = MT::PrecisionTickCount();
            ASSERT_FALSE( page.Navigate( MainSite() + "/nocontent" ) );
            const UInt64 noContentMs = (UInt64) ( ( MT::PrecisionTickCount() - start ) / ( MT::PrecisionTimerResolution() / 1000.0 ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RealBrowser test: a response without content gave false after " << noContentMs << " ms: " << page.GetLastNavigationError() );
            ASSERT_TRUE( noContentMs < 6000 );
            ASSERT_FALSE( page.GetLastNavigationError().IsNULLOrEmpty() );
            ASSERT_TRUE( page.GetUrl( url ) && url == address + "#lots" );

            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode result;
            params.SetAttribute( "targetId", CORE::CVariant( firstTab ) );
            client.SendCommandAndWait( "Target.closeTarget", &params, result, GUCEF_NULL, 10000 );
            params.SetAttribute( "targetId", CORE::CVariant( secondTab ) );
            client.SendCommandAndWait( "Target.closeTarget", &params, result, GUCEF_NULL, 10000 );
            PumpPlain( 20 );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The real browser that was started is ended" )
        try
        {
            ASSERT_TRUE( browser.WasLaunched() );
            ASSERT_TRUE( browser.Shutdown() );
            for ( UInt32 i=0; i<100 && browser.IsAnswering( 300 ); ++i )
                MT::ThreadDelay( 50 );
            ASSERT_FALSE( browser.IsAnswering( 500 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // the browser may still be writing its profile while it goes; whatever it left is removed with the folder
    if ( browser.WasLaunched() )
        browser.Shutdown();
    MT::ThreadDelay( 500 );

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL real browser TESTS COMPLETED" );
}
