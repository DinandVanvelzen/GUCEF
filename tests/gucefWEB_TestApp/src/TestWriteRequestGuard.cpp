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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#include "gucefWEB_CChromeDevToolsClient.h"
#include "gucefWEB_CNavigationGuard.h"
#include "gucefWEB_CWriteRequestGuard.h"

#include "gucefWEB_CWebSocketHandshake.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestFakeDevToolsBrowser.h"
#include "TestWriteRequestGuard.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/** The json codec for the protocol and the hash codec for the web socket handshake */
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

static CORE::CDataNode
Json( const CORE::CString& text )
{
    CORE::CDataNode node;
    COM::CJsonRpcEndpoint::ParseJsonText( text, node );
    return node;
}

/*-------------------------------------------------------------------------*/

/** Each round is about 5 ms */
static void
Pump( UInt32 rounds )
{
    for ( UInt32 i=0; i<rounds*5; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

static bool
Connect( CFakeDevToolsBrowser& browser, WEB::CChromeDevToolsClient& client )
{
    WEB::SChromeDiscoveryResult discovery;
    return client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) && client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl );
}

/*-------------------------------------------------------------------------*/

/** A paused request as the browser reports it on the browser target */
static void
EmitPaused( CFakeDevToolsBrowser& browser, const char* requestId, const char* method, const CORE::CString& url, const char* resourceType,
            const char* frameId, const char* redirectedFrom = GUCEF_NULL )
{
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "requestId", CORE::CVariant( CORE::CString( requestId ) ) );
    CORE::CDataNode* request = params.AddChild( "request", GUCEF_DATATYPE_OBJECT );
    request->SetAttribute( "url", CORE::CVariant( url ) );
    request->SetAttribute( "method", CORE::CVariant( CORE::CString( method ) ) );
    params.SetAttribute( "frameId", CORE::CVariant( CORE::CString( frameId ) ) );
    params.SetAttribute( "resourceType", CORE::CVariant( CORE::CString( resourceType ) ) );
    if ( GUCEF_NULL != redirectedFrom )
        params.SetAttribute( "redirectedRequestId", CORE::CVariant( CORE::CString( redirectedFrom ) ) );
    browser.Emit( CORE::CString::Empty, "Fetch.requestPaused", params );
}

/*-------------------------------------------------------------------------*/

/** The answer the guard gave to a paused request: "continue", "fail:<reason>" or "" when it gave none */
static CORE::CString
AnswerTo( const CFakeDevToolsBrowser& browser, const char* requestId )
{
    CORE::CString answer;
    UInt32 answers = 0;
    for ( size_t i=0; i<browser.commands.size(); ++i )
    {
        const CORE::CDataNode& params = browser.commandParams[ i ];
        if ( params.GetAttributeValue( "requestId" ).AsString( CORE::CString::Empty, false ) != requestId )
            continue;
        if ( browser.commands[ i ] == "Fetch.continueRequest@" )
        {
            answer = "continue";
            ++answers;
        }
        else
        if ( browser.commands[ i ] == "Fetch.failRequest@" )
        {
            answer = "fail:" + params.GetAttributeValue( "errorReason" ).AsString( CORE::CString::Empty, false );
            ++answers;
        }
    }
    return answers == 1 ? answer : ( answers > 1 ? CORE::CString( "answered twice" ) : CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

static bool
AllowedWithReason( const WEB::CWriteRequestGuard& guard, const char* method, const char* url, const char* expectedReason )
{
    CORE::CString reason;
    const bool allowed = guard.IsAllowed( method, url, &reason );
    if ( GUCEF_NULL == expectedReason )
        return allowed && reason.IsNULLOrEmpty();
    if ( allowed || reason != expectedReason )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuard test: " << method << " " << url << " gave " << ( allowed ? "allowed" : "refused" ) << " (" << reason << "), expected " << expectedReason );
    return !allowed && reason == expectedReason;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Keeps the refusals a write request guard reports */
class CWriteRefusalCollector : public CORE::CObserver
{
    public:

    GUCEF::vector< WEB::CWriteRequestRefusal > refusals;

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventId == WEB::CWriteRequestGuard::WriteRequestRefusedEvent && GUCEF_NULL != eventData )
            refusals.push_back( static_cast< WEB::TWriteRequestRefusalEventData* >( eventData )->GetData() );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformWriteRequestRuleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Write request rules parse strictly and read paths without room for normalisation tricks" )
        try
        {
            WEB::CWriteRequestRule rule;
            ASSERT_TRUE( WEB::CWriteRequestRule::Parse( "POST api.shop.test/v1/lists/{n}/items", rule ) );
            ASSERT_TRUE( rule.method == "POST" && rule.scheme == "https" && rule.host == "api.shop.test" && rule.path == "/v1/lists/{n}/items" );
            ASSERT_TRUE( rule.segments.size() == 4 && rule.segments[ 2 ] == "{n}" && rule.queryMode == WEB::CWriteRequestRule::QUERY_NONE );
            ASSERT_TRUE( rule.ToString() == "POST api.shop.test/v1/lists/{n}/items" );

            ASSERT_TRUE( WEB::CWriteRequestRule::Parse( "  DELETE http://API.Shop.test:8443/v1/lists/{n}/items/{any}?*  ", rule ) );
            ASSERT_TRUE( rule.method == "DELETE" && rule.scheme == "http" && rule.host == "api.shop.test:8443" && rule.queryMode == WEB::CWriteRequestRule::QUERY_ANY );
            ASSERT_TRUE( rule.ToString() == "DELETE http://api.shop.test:8443/v1/lists/{n}/items/{any}?*" );

            ASSERT_TRUE( WEB::CWriteRequestRule::Parse( "PUT api.shop.test/v1/prefs?view=compact&page=2", rule ) );
            ASSERT_TRUE( rule.queryMode == WEB::CWriteRequestRule::QUERY_EXACT && rule.query == "view=compact&page=2" );
            ASSERT_TRUE( WEB::CWriteRequestRule::Parse( "POST https://api.shop.test/", rule ) && rule.segments.size() == 1 && rule.segments[ 0 ].IsNULLOrEmpty() );

            // what is not a rule
            static const char* const broken[] = { "", "/v1/items", "POST", "POST api.shop.test", "POST /v1/items", "POST *.shop.test/v1/items",
                                                  "POST api.*/v1/items", "POST user@api.shop.test/v1/items", "POST ftp://api.shop.test/v1/items",
                                                  "POST api.shop.test/v1/../admin", "POST api.shop.test/v1/./items", "POST api.shop.test/v1//items",
                                                  "POST api.shop.test/v1/%69tems", "POST api.shop.test/v1\\items", "POST api.shop.test/v1/items;x=1",
                                                  "POST api.shop.test/v1/{bad}", "POST api.shop.test/v1/{n", "POST api.shop.test/v1/items?",
                                                  "POST api.shop.test/v1/items?a=b c", "P0ST api.shop.test/v1/items", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != broken[ i ]; ++i )
            {
                CORE::CString problem;
                ASSERT_FALSE( WEB::CWriteRequestRule::Parse( broken[ i ], rule, &problem ) );
                ASSERT_FALSE( problem.IsNULLOrEmpty() );
            }

            // request paths: only plain ones can match a rule
            CORE::CString::StringVector segments;
            ASSERT_TRUE( WEB::CWriteRequestRule::SplitStrictPath( "/a/b/", segments ) && segments.size() == 3 && segments[ 2 ].IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CWriteRequestRule::SplitStrictPath( "/a-b_c.d~e/x:y@z/!$&'()*+,=", segments ) && segments.size() == 3 );
            static const char* const tricks[] = { "", "a/b", "/a/./b", "/a/../b", "/a/%2e%2e/b", "/a%2Fb", "/a%2fb", "/a\\b", "/a;x/b", "/a//b",
                                                  "/a/b c", "/a/\xC3\xA9", "/a/{n}", "/a/\t", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != tricks[ i ]; ++i )
                ASSERT_FALSE( WEB::CWriteRequestRule::SplitStrictPath( tricks[ i ], segments ) );

            // placeholders
            ASSERT_TRUE( WEB::CWriteRequestRule::SegmentMatches( "{n}", "7" ) && WEB::CWriteRequestRule::SegmentMatches( "{n}", "12345678901234567890" ) );
            ASSERT_FALSE( WEB::CWriteRequestRule::SegmentMatches( "{n}", "123456789012345678901" ) || WEB::CWriteRequestRule::SegmentMatches( "{n}", "12a" ) );
            ASSERT_TRUE( WEB::CWriteRequestRule::SegmentMatches( "{guid}", "0f8fad5b-d9cb-469f-a165-70867728950e" ) );
            ASSERT_FALSE( WEB::CWriteRequestRule::SegmentMatches( "{guid}", "0f8fad5b" ) );
            ASSERT_TRUE( WEB::CWriteRequestRule::SegmentMatches( "{any}", "AB-12.x_y~z" ) );
            ASSERT_FALSE( WEB::CWriteRequestRule::SegmentMatches( "{any}", "a:b" ) || WEB::CWriteRequestRule::SegmentMatches( "{any}", "" ) );
            ASSERT_FALSE( WEB::CWriteRequestRule::SegmentMatches( "items", "Items" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The write request guard passes every read and refuses every write that no rule names exactly" )
        try
        {
            WEB::CWriteRequestGuardSettings settings;
            settings.allow.push_back( "POST api.shop.test/v1/lists/{n}/items" );
            settings.allow.push_back( "DELETE api.shop.test/v1/lists/{n}/items/{any}" );
            settings.allow.push_back( "PUT api.shop.test/v1/prefs?view=compact" );
            settings.allow.push_back( "POST http://localhost:8080/form" );
            WEB::CWriteRequestGuard guard;
            ASSERT_TRUE( guard.SetSettings( settings ) );

            // reads pass, on any host
            ASSERT_TRUE( AllowedWithReason( guard, "GET", "https://anything.test/any/path?x=1", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "HEAD", "https://anything.test/", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "OPTIONS", "https://anything.test/v1", GUCEF_NULL ) );

            // the allowed writes, exactly as named
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://API.shop.test/v1/lists/42/items", GUCEF_NULL ) );     // a host has no case
            ASSERT_TRUE( AllowedWithReason( guard, "DELETE", "https://api.shop.test/v1/lists/7/items/AB-12", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PUT", "https://api.shop.test/v1/prefs?view=compact", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "http://localhost:8080/form", GUCEF_NULL ) );

            // methods: exact, with case
            ASSERT_TRUE( AllowedWithReason( guard, "post", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "Post", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PATCH", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "get", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PUT", "https://api.shop.test/v1/lists/42/items", "path-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "DELETE", "https://api.shop.test/v1/lists/42/items", "path-not-allowed" ) );

            // scheme, host and port
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "http://api.shop.test/v1/lists/42/items", "scheme-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test:8443/v1/lists/42/items", "host-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://evil.test/v1/lists/42/items", "host-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test.evil.test/v1/lists/42/items", "host-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://www.api.shop.test/v1/lists/42/items", "host-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://user@api.shop.test/v1/lists/42/items", "malformed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://evil.test\\@api.shop.test/v1/lists/42/items", "malformed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "data:text/plain,x", "malformed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "blob:https://api.shop.test/123", "malformed" ) );

            // path normalisation tricks, trailing slash, case, placeholders
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items/", "path-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/Items", "path-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/4x2/items", "path-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items/extra", "path-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/../42/items", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/./items", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/%69tems", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists%2F42/items", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42%2Fitems", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1//lists/42/items", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items;jsessionid=1", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1\\lists/42/items", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "DELETE", "https://api.shop.test/v1/lists/7/items/..", "ambiguous-path" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "DELETE", "https://api.shop.test/v1/lists/7/items/a%20b", "ambiguous-path" ) );

            // queries: none, exact, any
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items?x=1", "query-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items?", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items#frag", GUCEF_NULL ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PUT", "https://api.shop.test/v1/prefs?view=full", "query-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PUT", "https://api.shop.test/v1/prefs?view=compact&x=1", "query-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( guard, "PUT", "https://api.shop.test/v1/prefs", "query-not-allowed" ) );

            // the opening of a web socket is a GET (the browser does not pause it at all)
            ASSERT_TRUE( AllowedWithReason( guard, "GET", "wss://api.shop.test/stream", GUCEF_NULL ) );

            // no rules: every write refused
            WEB::CWriteRequestGuard empty;
            ASSERT_TRUE( AllowedWithReason( empty, "POST", "https://api.shop.test/v1/lists/42/items", "method-not-allowed" ) );
            ASSERT_TRUE( AllowedWithReason( empty, "GET", "https://api.shop.test/v1/lists/42/items", GUCEF_NULL ) );

            // a broken rule leaves the rules as they were
            WEB::CWriteRequestGuardSettings broken;
            broken.allow.push_back( "POST *.shop.test/v1" );
            ASSERT_FALSE( guard.SetSettings( broken ) );
            ASSERT_TRUE( guard.GetSettings().allow.size() == 4 );
            ASSERT_TRUE( AllowedWithReason( guard, "POST", "https://api.shop.test/v1/lists/42/items", GUCEF_NULL ) );

            // the address of a refusal: the path as a template, the query values never
            CORE::CString host, pathTemplate;
            ASSERT_TRUE( WEB::CWriteRequestGuard::MakeRedactedUrl( "https://api.shop.test/v1/lists/12345/items?token=PLANTEDqueryTOKEN1&b=2", &host, &pathTemplate ) ==
                         "https://api.shop.test/v1/lists/{n}/items?token={redacted}&b={redacted}" );
            ASSERT_TRUE( host == "api.shop.test" && pathTemplate == "/v1/lists/{n}/items" );
            ASSERT_TRUE( WEB::CWriteRequestGuard::MakeRedactedUrl( "javascript:alert(1)" ) == "javascript:" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Write request guard settings load and save their rules and refuse a broken one" )
        try
        {
            WEB::CWriteRequestGuardSettings defaults;
            ASSERT_TRUE( defaults.allow.empty() );

            WEB::CWriteRequestGuardSettings settings;
            settings.allow.push_back( "POST api.shop.test/v1/lists/{n}/items" );
            CORE::CDataNode cfg;
            ASSERT_TRUE( settings.SaveConfig( cfg ) );
            WEB::CWriteRequestGuardSettings again;
            ASSERT_TRUE( again.LoadConfig( cfg ) );
            ASSERT_TRUE( again.allow.size() == 1 && again.allow[ 0 ] == "POST api.shop.test/v1/lists/{n}/items" );

            CORE::CDataNode loaded = Json( "{\"allow\":[\"DELETE api.shop.test/v1/lists/{n}\",\"PUT http://localhost:9000/x?*\"]}" );
            WEB::CWriteRequestGuardSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( loaded ) );
            ASSERT_TRUE( fromJson.allow.size() == 2 );

            CORE::CDataNode bad = Json( "{\"allow\":[\"DELETE api.shop.test/v1/lists/{n}\",\"POST api.shop.test/a/../b\"]}" );
            ASSERT_FALSE( fromJson.LoadConfig( bad ) );
            ASSERT_TRUE( fromJson.allow.size() == 2 );
            CORE::CString invalid, problem;
            WEB::CWriteRequestGuardSettings check;
            check.allow.push_back( "POST api.shop.test/a/../b" );
            ASSERT_FALSE( check.AreRulesValid( &invalid, &problem ) );
            ASSERT_TRUE( invalid == "POST api.shop.test/a/../b" && !problem.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformWriteRequestGuardBrowserTests( void )
{
    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "https://www.shop.test/app" );

    GUCEF_TESTFW_TESTCASE( "Test: With a write request guard every request is intercepted and refused writes are failed and reported before they leave" )
        try
        {
            ASSERT_TRUE( started );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( browser, client ) );

            WEB::CNavigationGuardSettings navigationSettings;
            navigationSettings.allow.push_back( "*.shop.test" );
            WEB::CNavigationGuard navigation;
            ASSERT_TRUE( navigation.SetSettings( navigationSettings ) );

            WEB::CWriteRequestGuardSettings writeSettings;
            writeSettings.allow.push_back( "POST api.shop.test/v1/lists/{n}/items" );
            WEB::CWriteRequestGuard writes;
            ASSERT_TRUE( writes.SetSettings( writeSettings ) );
            CWriteRefusalCollector collector;
            writes.Subscribe( &collector );

            // installed before the start: in force only once the browser accepted the interception
            ASSERT_TRUE( navigation.SetWriteRequestGuard( &writes ) );
            ASSERT_TRUE( navigation.GetWriteRequestGuard() == &writes );
            ASSERT_FALSE( writes.IsEnforcing() );
            ASSERT_FALSE( writes.VerifyEnforcing( "S-P1", 1000 ) );
            ASSERT_TRUE( navigation.Start( client ) );
            ASSERT_TRUE( writes.IsEnforcing() );

            // every request of every target, at the request stage: no resource type in the pattern
            CORE::CDataNode params;
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.enable", "", params ) );
            const CORE::CDataNode* patterns = params.FindChild( "patterns" );
            ASSERT_TRUE( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 );
            if ( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 )
            {
                const CORE::CDataNode& pattern = *( *patterns->ConstBegin() );
                ASSERT_FALSE( pattern.IsAttribute( "resourceType" ) );
                ASSERT_TRUE( pattern.GetAttributeValue( "urlPattern" ).AsString() == "*" );
                ASSERT_TRUE( pattern.GetAttributeValue( "requestStage" ).AsString() == "Request" );
            }

            // an allowed write and reads of every kind continue
            EmitPaused( browser, "W1", "POST", "https://api.shop.test/v1/lists/42/items", "XHR", "P1" );
            EmitPaused( browser, "R1", "GET", "https://cdn.elsewhere.test/logo.png", "Image", "P1" );
            EmitPaused( browser, "R2", "GET", "https://api.shop.test/v1/lists/42/items?page=2", "Fetch", "P1" );
            EmitPaused( browser, "R3", "OPTIONS", "https://api.shop.test/v1/lists/42/items", "Preflight", "P1" );
            EmitPaused( browser, "R4", "HEAD", "https://api.shop.test/v1/ping", "Fetch", "P1" );
            EmitPaused( browser, "D1", "GET", "https://www.shop.test/app/lists", "Document", "P1" );
            Pump( 30 );
            ASSERT_TRUE( AnswerTo( browser, "W1" ) == "continue" );
            ASSERT_TRUE( AnswerTo( browser, "R1" ) == "continue" && AnswerTo( browser, "R2" ) == "continue" );
            ASSERT_TRUE( AnswerTo( browser, "R3" ) == "continue" && AnswerTo( browser, "R4" ) == "continue" );
            ASSERT_TRUE( AnswerTo( browser, "D1" ) == "continue" );
            ASSERT_TRUE( writes.GetAllowedWriteCount() == 1 && writes.GetPassedReadCount() == 5 && writes.GetRefusedCount() == 0 );

            // writes no rule names: failed as blocked by the client, whatever kind of request carries them
            EmitPaused( browser, "X1", "POST", "https://api.shop.test/v1/orders?token=PLANTEDqueryTOKEN1", "XHR", "P1" );
            EmitPaused( browser, "X2", "PUT", "https://api.shop.test/v1/lists/42/items", "Fetch", "P1" );
            EmitPaused( browser, "X3", "DELETE", "https://api.shop.test/v1/lists/42/items", "Fetch", "P1" );
            EmitPaused( browser, "X4", "PATCH", "https://api.shop.test/v1/lists/42/items", "Fetch", "P1" );
            EmitPaused( browser, "X5", "POST", "https://metrics.elsewhere.test/collect", "Ping", "P1" );                  // sendBeacon
            EmitPaused( browser, "X6", "POST", "https://www.shop.test/app/settings/save", "Document", "P1" );             // a form post
            EmitPaused( browser, "X7", "POST", "https://api.shop.test/v1/lists/42/items/", "XHR", "P1" );                // trailing slash
            EmitPaused( browser, "X8", "POST", "https://api.shop.test/v1/lists/42/../42/items", "XHR", "P1" );
            EmitPaused( browser, "X9", "post", "https://api.shop.test/v1/lists/42/items", "XHR", "P1" );
            EmitPaused( browser, "X10", "POST", "https://other.test/v1/lists/42/items", "XHR", "P1", "W1" );            // a redirect to another host
            EmitPaused( browser, "X11", "POST", "https://api.shop.test/v1/lists/42/items", "XHR", "SUB1" );             // allowed from a frame too
            Pump( 40 );
            static const char* const refused[] = { "X1", "X2", "X3", "X4", "X5", "X6", "X7", "X8", "X9", "X10", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != refused[ i ]; ++i )
                ASSERT_TRUE( AnswerTo( browser, refused[ i ] ) == "fail:BlockedByClient" );
            ASSERT_TRUE( AnswerTo( browser, "X11" ) == "continue" );
            ASSERT_TRUE( writes.GetRefusedCount() == 10 && collector.refusals.size() == 10 && writes.GetRecentRefusals().size() == 10 );
            ASSERT_TRUE( navigation.GetRefusedCount() == 0 );       // the form post is the write guard's

            if ( collector.refusals.size() == 10 )
            {
                const WEB::CWriteRequestRefusal& first = collector.refusals[ 0 ];
                ASSERT_TRUE( first.method == "POST" && first.url == "https://api.shop.test/v1/orders?token={redacted}" && first.reason == "path-not-allowed" );
                ASSERT_TRUE( first.host == "api.shop.test" && first.pathTemplate == "/v1/orders" && first.resourceType == "XHR" && first.isMainFrame && !first.redirected );
                ASSERT_TRUE( collector.refusals[ 1 ].method == "PUT" && collector.refusals[ 1 ].reason == "method-not-allowed" );
                ASSERT_TRUE( collector.refusals[ 3 ].method == "PATCH" );
                ASSERT_TRUE( collector.refusals[ 4 ].resourceType == "Ping" && collector.refusals[ 4 ].reason == "host-not-allowed" );
                ASSERT_TRUE( collector.refusals[ 5 ].resourceType == "Document" && collector.refusals[ 5 ].reason == "host-not-allowed" );
                ASSERT_TRUE( collector.refusals[ 6 ].reason == "path-not-allowed" && collector.refusals[ 7 ].reason == "ambiguous-path" );
                ASSERT_TRUE( collector.refusals[ 8 ].method == "post" && collector.refusals[ 8 ].reason == "method-not-allowed" );
                ASSERT_TRUE( collector.refusals[ 9 ].redirected && collector.refusals[ 9 ].reason == "host-not-allowed" );
                for ( size_t i=0; i<collector.refusals.size(); ++i )
                    ASSERT_FALSE( Contains( collector.refusals[ i ].url, "PLANTED" ) );
            }

            // a document the navigation rules refuse is still aborted by the navigation guard
            EmitPaused( browser, "N1", "GET", "https://evil.test/", "Document", "P1" );
            Pump( 20 );
            ASSERT_TRUE( AnswerTo( browser, "N1" ) == "fail:Aborted" );
            ASSERT_TRUE( navigation.GetRefusedCount() == 1 && writes.GetRefusedCount() == 10 );

            // a tab the page opens: its requests are paused on the browser target as well
            browser.Emit( "", "Target.targetCreated", Json( "{\"targetInfo\":{\"targetId\":\"P2\",\"type\":\"page\",\"url\":\"about:blank\",\"attached\":false,\"openerId\":\"P1\"}}" ) );
            Pump( 10 );
            EmitPaused( browser, "T1", "POST", "https://api.shop.test/v1/transfer", "Fetch", "P2" );
            EmitPaused( browser, "T2", "GET", "https://www.shop.test/app/popup", "Document", "P2" );
            Pump( 20 );
            ASSERT_TRUE( AnswerTo( browser, "T1" ) == "fail:BlockedByClient" && AnswerTo( browser, "T2" ) == "continue" );
            ASSERT_TRUE( collector.refusals.back().frameId == "P2" && collector.refusals.back().isMainFrame );

            // the proof on the page: its probe is seen, failed and not reported as a refusal
            const UInt32 refusedBefore = writes.GetRefusedCount();
            browser.SetEmulateProbeInterception( true );
            ASSERT_TRUE( writes.VerifyEnforcing( "S-P1", 5000 ) );
            Pump( 10 );
            ASSERT_TRUE( AnswerTo( browser, "PROBE1" ) == "fail:BlockedByClient" );
            ASSERT_TRUE( writes.GetRefusedCount() == refusedBefore );
            ASSERT_TRUE( browser.GetLastCommandParams( "Runtime.evaluate", "S-P1", params ) );
            ASSERT_TRUE( Contains( params.GetAttributeValue( "expression" ).AsString(), "write-guard-probe.invalid" ) );
            ASSERT_FALSE( writes.VerifyEnforcing( CORE::CString::Empty, 1000 ) );

            // a browser that lets the probe through, or a page that never sends it: not proven
            browser.SetEmulateProbeInterception( false );
            ASSERT_FALSE( writes.VerifyEnforcing( "S-P1", 2000 ) );

            // a stale probe address, or anything else sent to the probe host, never passes
            EmitPaused( browser, "P9", "GET", "https://write-guard-probe.invalid/0123456789abcdef01234567", "Fetch", "P1" );
            Pump( 20 );
            ASSERT_TRUE( AnswerTo( browser, "P9" ) == "fail:BlockedByClient" );
            ASSERT_TRUE( writes.GetRefusedCount() == refusedBefore + 1 );

            // disarmed on the started guard: documents only again, the write guard no longer in force
            ASSERT_TRUE( navigation.SetWriteRequestGuard( GUCEF_NULL ) );
            ASSERT_FALSE( writes.IsEnforcing() );
            ASSERT_TRUE( browser.GetLastCommandParams( "Fetch.enable", "", params ) );
            patterns = params.FindChild( "patterns" );
            ASSERT_TRUE( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 );
            if ( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 )
                ASSERT_TRUE( ( *patterns->ConstBegin() )->GetAttributeValue( "resourceType" ).AsString() == "Document" );
            EmitPaused( browser, "U1", "POST", "https://api.shop.test/v1/orders", "XHR", "P1" );
            Pump( 20 );
            ASSERT_TRUE( AnswerTo( browser, "U1" ) == "continue" );

            // armed again
            ASSERT_TRUE( navigation.SetWriteRequestGuard( &writes ) );
            ASSERT_TRUE( writes.IsEnforcing() );
            EmitPaused( browser, "U2", "POST", "https://api.shop.test/v1/orders", "XHR", "P1" );
            Pump( 20 );
            ASSERT_TRUE( AnswerTo( browser, "U2" ) == "fail:BlockedByClient" );

            // one navigation guard at a time
            WEB::CNavigationGuard other;
            ASSERT_FALSE( other.SetWriteRequestGuard( &writes ) );
            ASSERT_TRUE( navigation.GetWriteRequestGuard() == &writes );

            // a browser that refuses the change keeps the guard armed
            browser.SetCommandError( "Fetch.enable", "Fetch domain refused" );
            ASSERT_FALSE( navigation.SetWriteRequestGuard( GUCEF_NULL ) );
            ASSERT_TRUE( navigation.GetWriteRequestGuard() == &writes && writes.IsEnforcing() );

            // the end of the connection ends the enforcement, which the guard then tells
            client.Disconnect();
            Pump( 20 );
            ASSERT_FALSE( writes.IsEnforcing() );
            ASSERT_FALSE( navigation.IsStarted() );
            writes.Unsubscribe( &collector );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A write request guard that goes away leaves its navigation guard with document interception only" )
        try
        {
            ASSERT_TRUE( started );
            browser.ForgetConnection();
            CFakeDevToolsBrowser fresh;
            ASSERT_TRUE( fresh.Start() );
            fresh.AddTarget( "P1", "page", "https://www.shop.test/app" );
            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( Connect( fresh, client ) );
            WEB::CNavigationGuard navigation;
            ASSERT_TRUE( navigation.Start( client ) );
            {
                WEB::CWriteRequestGuard writes;
                ASSERT_TRUE( navigation.SetWriteRequestGuard( &writes ) );
                ASSERT_TRUE( writes.IsEnforcing() );
            }
            ASSERT_TRUE( navigation.GetWriteRequestGuard() == GUCEF_NULL );
            Pump( 20 );
            CORE::CDataNode params;
            ASSERT_TRUE( fresh.GetLastCommandParams( "Fetch.enable", "", params ) );
            const CORE::CDataNode* patterns = params.FindChild( "patterns" );
            ASSERT_TRUE( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 );
            if ( GUCEF_NULL != patterns && patterns->GetNrOfChildNodes() == 1 )
                ASSERT_TRUE( ( *patterns->ConstBegin() )->GetAttributeValue( "resourceType" ).AsString() == "Document" );

            // and a navigation guard that goes away first leaves the write guard not in force
            WEB::CWriteRequestGuard survivor;
            {
                WEB::CNavigationGuard shortLived;
                ASSERT_TRUE( shortLived.SetWriteRequestGuard( &survivor ) );
            }
            ASSERT_FALSE( survivor.IsEnforcing() );
            WEB::CNavigationGuard next;
            ASSERT_TRUE( next.SetWriteRequestGuard( &survivor ) );
            ASSERT_TRUE( next.SetWriteRequestGuard( GUCEF_NULL ) );
            navigation.Stop();
            Pump( 10 );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWriteRequestGuardTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING write request guard TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "WriteRequestGuard" );

    PerformWriteRequestRuleTests();

    if ( !PluginsAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Write request guard browser TESTS skipped: the json or the hash codec plugin is not available" );
        return;
    }
    PerformWriteRequestGuardBrowserTests();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL write request guard TESTS COMPLETED" );
}
