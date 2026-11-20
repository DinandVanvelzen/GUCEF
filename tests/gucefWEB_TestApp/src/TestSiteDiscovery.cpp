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

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#include "gucefWEB_CUrlPathTemplate.h"
#include "gucefWEB_CWebBodyShape.h"
#include "gucefWEB_CSiteDiscoverySettings.h"
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#include "gucefWEB_CUserActionTracker.h"
#include "gucefWEB_CSiteEndpointCatalog.h"
#include "gucefWEB_CSiteDiscoveryJournal.h"
#include "gucefWEB_CChromeNetworkRecorder.h"
#include "gucefWEB_CDevToolsBrowser.h"
#include "gucefWEB_CSiteDiscoverySession.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestFakeDevToolsBrowser.h"
#include "TestSiteDiscovery.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
#include <sys/stat.h>
#endif

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const PlantedSecrets[] = { "PLANTEDqueryTOKEN1", "PLANTEDcookieVALUE2", "PLANTEDbearerTOKEN3", "PLANTEDcsrfTOKEN4",
                                              "PLANTEDbodyPASSWORD5", "PLANTEDaccessTOKEN6", "PLANTEDresponseTOKEN7", "PLANTEDsigninPASSWORD8",
                                              "PLANTEDapiKEY9", "PLANTEDformSECRET10", GUCEF_NULL };

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const char* part )
{
    return !text.IsNULLOrEmpty() && GUCEF_NULL != strstr( text.C_String(), part );
}

/*-------------------------------------------------------------------------*/

static bool
ContainsAnyPlantedSecret( const CORE::CString& text )
{
    for ( UInt32 i=0; GUCEF_NULL != PlantedSecrets[ i ]; ++i )
        if ( Contains( text, PlantedSecrets[ i ] ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
JoinedContains( const CORE::CString::StringVector& lines, const char* part )
{
    for ( size_t i=0; i<lines.size(); ++i )
        if ( Contains( lines[ i ], part ) )
            return true;
    return false;
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
JsonAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

static bool
WebSocketAvailable( void )
{
    if ( !WEB::CWebSocketHandshake::IsAvailable() )
        LoadPlugin( "codecspluginSTBRUMMEHASH" );
    return WEB::CWebSocketHandshake::IsAvailable();
}

/*-------------------------------------------------------------------------*/

static void
Pump( UInt32 rounds, WEB::CChromeNetworkRecorder* recorder )
{
    for ( UInt32 i=0; i<rounds; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        if ( GUCEF_NULL != recorder )
            recorder->Process();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

static WEB::CObservedWebRequest
MakeObservation( UInt32 sequence, const char* method, const char* url, const char* type, Int32 status, const char* mime, UInt64 size )
{
    WEB::CObservedWebRequest o;
    o.sequence = sequence;
    o.timestamp = 100.0 + sequence;
    o.method = method;
    o.url = url;
    o.resourceType = type;
    o.status = status;
    o.mimeType = mime;
    o.encodedDataLength = size;
    o.requestHeaders[ "accept" ] = "application/json";
    return o;
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformUrlTemplateTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Identifiers in a path become placeholders" )
        try
        {
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/api/accounts/12345678/orders/987654321/lines" ) == "/api/accounts/{n}/orders/{n}/lines" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/media/3f2504e0-4f89-11d3-9a0c-0305e82c3301/original.jpg" ) == "/media/{guid}/original.jpg" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/media/3f2504e04f8911d39a0c0305e82c3301" ) == "/media/{guid}" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/img/12345678.jpg" ) == "/img/{n}.jpg" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/a/9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08" ) == "/a/{hex}" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/p/Ab3dE5gH7jK9" ) == "/p/{id}" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/rec/1030-7488" ) == "/rec/{n}" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/v1/assets/search" ) == "/v1/assets/search" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/api/2/x" ) == "/api/2/x" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/" ) == "/" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/a/" ) == "/a/" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "" ).IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/static/main.min.js" ) == "/static/main.min.js" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::TemplatePath( "/api/accounts/{n}/orders" ) == "/api/accounts/{n}/orders" );
            ASSERT_FALSE( WEB::CUrlPathTemplate::LooksLikeId( "assets" ) );
            ASSERT_TRUE( WEB::CUrlPathTemplate::LooksLikeId( "123456" ) );
            ASSERT_FALSE( WEB::CUrlPathTemplate::LooksLikeId( "{n}" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A URL is split in its parts" )
        try
        {
            WEB::CUrlPathTemplate parsed;
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "https://User:pw@WWW.Shop.Example:8443/a/12345/b?x=1&y=&z#frag", parsed ) );
            ASSERT_TRUE( parsed.scheme == "https" );
            ASSERT_TRUE( parsed.host == "www.shop.example:8443" );
            ASSERT_TRUE( parsed.path == "/a/12345/b" );
            ASSERT_TRUE( parsed.pathTemplate == "/a/{n}/b" );
            ASSERT_TRUE( parsed.GetKey() == "www.shop.example:8443/a/{n}/b" );
            ASSERT_TRUE( parsed.queryParams.size() == 3 );
            if ( parsed.queryParams.size() == 3 )
            {
                ASSERT_TRUE( parsed.queryParams[ 0 ].first == "x" && parsed.queryParams[ 0 ].second == "1" );
                ASSERT_TRUE( parsed.queryParams[ 1 ].first == "y" && parsed.queryParams[ 1 ].second.IsNULLOrEmpty() );
                ASSERT_TRUE( parsed.queryParams[ 2 ].first == "z" );
            }
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "http://host", parsed ) );
            ASSERT_TRUE( parsed.host == "host" && parsed.path.IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "http://host?a=b", parsed ) );
            ASSERT_TRUE( parsed.host == "host" && parsed.queryParams.size() == 1 );
            ASSERT_FALSE( WEB::CUrlPathTemplate::Parse( "data:text/plain,hi", parsed ) );
            ASSERT_FALSE( WEB::CUrlPathTemplate::Parse( "/relative/path", parsed ) );
            ASSERT_TRUE( WEB::CUrlPathTemplate::FindIdAfterSegment( "/account/123456/orders", "account" ) == "123456" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::FindIdAfterSegment( "/account/overview/123456", "account" ).IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CUrlPathTemplate::PercentDecode( "a%20b+c%2Fd" ) == "a b c/d" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::PercentDecode( "100%" ) == "100%" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::EscapeJson( "say \"hi\"\n\\" ) == "say \\\"hi\\\"\\n\\\\" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::EscapeJson( "\x01" ) == "\\u0001" );
            ASSERT_TRUE( WEB::CUrlPathTemplate::EscapeJson( "caf\xC3\xA9" ) == "caf\xC3\xA9" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBodyShapeTests( void )
{
    static const char* const document = "{\"total\":120,\"page\":1,\"next\":\"https://shop.example/api/x/99887766?page=2\",\"hasMore\":true,"
                                        "\"accessToken\":\"PLANTEDaccessTOKEN6\","
                                        "\"items\":[{\"id\":1,\"name\":\"Private Person Name\",\"image\":\"/media/55555555.jpg\",\"price\":1.5},"
                                        "{\"id\":2,\"name\":\"Another Name\",\"tags\":[\"a\",\"b\"]}],\"meta\":{\"ok\":true}}";

    GUCEF_TESTFW_TESTCASE( "Test: A body shape holds names and types and no values" )
        try
        {
            CORE::CDataNode root;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( document, root ) );
            WEB::CWebBodyShape shape;
            WEB::CWebBodyShape::Describe( root, shape );
            ASSERT_TRUE( JoinedContains( shape.lines, "items: array[2]" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "items[].name: string" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "items[].price: number" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "items[].tags[]: string" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "meta.ok: boolean" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "total: integer" ) );
            ASSERT_FALSE( JoinedContains( shape.lines, "120" ) );
            ASSERT_FALSE( JoinedContains( shape.lines, "Private Person Name" ) );
            ASSERT_FALSE( JoinedContains( shape.lines, "PLANTEDaccessTOKEN6" ) );
            ASSERT_TRUE( shape.paginationHints.size() >= 4 );
            ASSERT_FALSE( JoinedContains( shape.paginationHints, "=" ) );
            // a URL field is described by its template, never its value
            ASSERT_TRUE( JoinedContains( shape.urlFields, "next -> https://shop.example/api/x/{n}" ) );
            ASSERT_FALSE( JoinedContains( shape.urlFields, "page=2" ) );
            ASSERT_TRUE( JoinedContains( shape.urlFields, "items[].image -> /media/{n}.jpg" ) );

            // with sample values: paging and URL fields show them, secret fields never
            WEB::CWebBodyShape samples;
            WEB::CWebBodyShape::Describe( root, samples, 5, 150, true );
            ASSERT_TRUE( JoinedContains( samples.lines, "total: integer = 120" ) );
            ASSERT_TRUE( JoinedContains( samples.lines, "accessToken: string" ) );
            ASSERT_FALSE( JoinedContains( samples.lines, "PLANTEDaccessTOKEN6" ) );
            ASSERT_FALSE( JoinedContains( samples.lines, "Private Person Name" ) );

            WEB::CWebBodyShape limited;
            WEB::CWebBodyShape::Describe( root, limited, 5, 4 );
            ASSERT_TRUE( limited.lines.size() == 4 && limited.truncated );

            WEB::CWebBodyShape form;
            WEB::CWebBodyShape::DescribeFormUrlEncoded( "user=bob&count=12&ratio=0.5&agree=true&client_secret=PLANTEDformSECRET10", form );
            ASSERT_TRUE( JoinedContains( form.lines, "user: string" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "count: integer" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "ratio: number" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "agree: boolean" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "client_secret: string" ) );
            ASSERT_FALSE( JoinedContains( form.lines, "bob" ) || JoinedContains( form.lines, "PLANTEDformSECRET10" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSettingsTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Site discovery settings have private defaults and load from JSON" )
        try
        {
            WEB::CSiteDiscoverySettings defaults;
            ASSERT_FALSE( defaults.recordExampleValues );
            ASSERT_FALSE( defaults.saveResponseBodies );
            ASSERT_FALSE( defaults.captureClicks );
            ASSERT_FALSE( defaults.disableBrowserCache );
            ASSERT_TRUE( defaults.noteNeverRecorded );
            ASSERT_TRUE( defaults.secretValueMode == WEB::CSiteDiscoverySettings::SECRET_VALUE_HASH );
            ASSERT_TRUE( defaults.browserRunMode == WEB::CSiteDiscoverySettings::BROWSER_RUNMODE_ATTACH );
            ASSERT_TRUE( defaults.browserDebugPort == 9222 );
            ASSERT_TRUE( defaults.neverRecordPathPatterns.size() > 20 );
            ASSERT_TRUE( defaults.hostFilter.empty() );

            CORE::CDataNode config;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"hostFilter\":[\"schwab.com\",\"fab.com\"],\"neverRecordPathPatterns\":[\"/login\"],"
                "\"neverRecordHosts\":[],\"secretValueMode\":\"strip\",\"recordExampleValues\":true,\"saveResponseBodies\":true,"
                "\"maxResponseBodyBytes\":0,\"browserRunMode\":\"recordedSession\",\"browserDebugPort\":9333,\"actionWindowMs\":5000,"
                "\"outputDirectory\":\"/tmp/x\"}", config ) );
            WEB::CSiteDiscoverySettings settings;
            ASSERT_TRUE( settings.LoadConfig( config ) );
            ASSERT_TRUE( settings.hostFilter.size() == 2 && settings.hostFilter[ 1 ] == "fab.com" );
            ASSERT_TRUE( settings.neverRecordPathPatterns.size() == 1 && settings.neverRecordPathPatterns[ 0 ] == "/login" );
            ASSERT_TRUE( settings.neverRecordHosts.empty() );
            ASSERT_TRUE( settings.secretNamePatterns == defaults.secretNamePatterns );
            ASSERT_TRUE( settings.secretValueMode == WEB::CSiteDiscoverySettings::SECRET_VALUE_STRIP );
            ASSERT_TRUE( settings.recordExampleValues && settings.saveResponseBodies && 0 == settings.maxResponseBodyBytes );
            ASSERT_TRUE( settings.browserRunMode == WEB::CSiteDiscoverySettings::BROWSER_RUNMODE_RECORDED_SESSION );
            ASSERT_TRUE( settings.browserDebugPort == 9333 && settings.actionWindowMs == 5000 );
            ASSERT_TRUE( settings.outputDirectory == "/tmp/x" );

            // and what is saved loads back the same
            CORE::CDataNode saved;
            ASSERT_TRUE( settings.SaveConfig( saved ) );
            WEB::CSiteDiscoverySettings again;
            ASSERT_TRUE( again.LoadConfig( saved ) );
            ASSERT_TRUE( again.hostFilter == settings.hostFilter );
            ASSERT_TRUE( again.neverRecordPathPatterns == settings.neverRecordPathPatterns );
            ASSERT_TRUE( again.neverRecordHosts.empty() );
            ASSERT_TRUE( again.secretValueMode == settings.secretValueMode && again.browserRunMode == settings.browserRunMode );
            ASSERT_TRUE( again.browserDebugPort == 9333 && again.recordExampleValues );

            CORE::CDataNode wrong;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"browserRunMode\":\"somewhere\"}", wrong ) );
            WEB::CSiteDiscoverySettings refused;
            ASSERT_FALSE( refused.LoadConfig( wrong ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformRedactorTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: The redactor knows what is in scope, never recorded and secret" )
        try
        {
            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "Shop.Example" );
            settings.allowPathPrefixes.push_back( "/api/" );
            settings.allowPathPrefixes.push_back( "/orders" );
            WEB::CSiteDiscoveryRedactor redactor( settings );

            WEB::CUrlPathTemplate parsed;
            WEB::CUrlPathTemplate::Parse( "https://www.shop.example/api/items", parsed );
            ASSERT_TRUE( redactor.IsInScope( parsed ) );
            WEB::CUrlPathTemplate::Parse( "https://www.shop.example/static/app.js", parsed );
            ASSERT_FALSE( redactor.IsInScope( parsed ) );
            WEB::CUrlPathTemplate::Parse( "https://www.mybank.example/api/items", parsed );
            ASSERT_FALSE( redactor.IsInScope( parsed ) );

            CORE::CString::StringVector terms;
            ASSERT_TRUE( WEB::CSiteDiscoveryRedactor::MatchesHostFilter( terms, "anything.example" ) );
            terms.push_back( "shop" );
            ASSERT_TRUE( WEB::CSiteDiscoveryRedactor::MatchesHostFilter( terms, "WWW.SHOP.EXAMPLE" ) );
            ASSERT_FALSE( WEB::CSiteDiscoveryRedactor::MatchesHostFilter( terms, "" ) );

            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://www.shop.example/account/password/verify" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://www.shop.example/secure/SignIn?returnUrl=x" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://example.com/oauth2/token" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://accounts.google.com/o/whatever" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://login.shop.example/x" ) );
            ASSERT_FALSE( redactor.IsNeverRecordUrl( "https://www.shop.example/api/accounts/123/positions" ) );

            ASSERT_TRUE( redactor.IsSecretName( "cookie" ) && redactor.IsSecretName( "Authorization" ) && redactor.IsSecretName( "x-csrf-token" ) );
            ASSERT_TRUE( redactor.IsSecretName( "access_token" ) && redactor.IsSecretName( "password" ) && redactor.IsSecretName( "x-xsrf-token" ) );
            ASSERT_FALSE( redactor.IsSecretName( "accept" ) || redactor.IsSecretName( "page" ) || redactor.IsSecretName( "content-type" ) );

            // a secret header keeps its name only, and its value becomes known
            ASSERT_TRUE( redactor.CaptureHeaderValue( "cookie", "sid=PLANTEDcookieVALUE2; theme=ok" ) == WEB::CSiteDiscoveryRedactor::NotRecordedMarker );
            ASSERT_TRUE( redactor.CaptureHeaderValue( "authorization", "Bearer PLANTEDbearerTOKEN3" ) == WEB::CSiteDiscoveryRedactor::NotRecordedMarker );
            ASSERT_TRUE( redactor.CaptureHeaderValue( "accept", "application/json" ) == "application/json" );
            ASSERT_TRUE( redactor.ContainsKnownSecret( "x PLANTEDcookieVALUE2 y" ) );
            ASSERT_TRUE( redactor.ContainsKnownSecret( "PLANTEDbearerTOKEN3" ) );
            ASSERT_FALSE( redactor.ContainsKnownSecret( "ok" ) );         // shorter than a secret is looked for

            // URLs: without example values the template and the names; with them values, secrets hashed or stripped
            const CORE::CString url = "https://u:p@www.shop.example/api/accounts/12345678/orders?page=2&access_token=PLANTEDqueryTOKEN1#token=x";
            redactor.RegisterQuerySecrets( url );
            ASSERT_TRUE( redactor.RedactUrl( url, false ) == "https://www.shop.example/api/accounts/{n}/orders?page&access_token" );
            WEB::CSiteDiscoverySettings examples = settings;
            examples.recordExampleValues = true;
            WEB::CSiteDiscoveryRedactor exampleRedactor( examples );
            const CORE::CString hashed = exampleRedactor.RedactUrl( url, false );
            ASSERT_TRUE( Contains( hashed, "/api/accounts/12345678/orders?page=2&access_token={secret:" ) );
            ASSERT_FALSE( Contains( hashed, "PLANTEDqueryTOKEN1" ) || Contains( hashed, "#" ) || Contains( hashed, "u:p" ) );
            ASSERT_TRUE( exampleRedactor.RedactUrl( url, false ) == hashed );     // equal values hash equally within a session
            ASSERT_TRUE( exampleRedactor.RedactUrl( url, true ) == "https://www.shop.example/api/accounts/{n}/orders" );
            examples.secretValueMode = WEB::CSiteDiscoverySettings::SECRET_VALUE_STRIP;
            exampleRedactor.SetSettings( examples );
            ASSERT_TRUE( Contains( exampleRedactor.RedactUrl( url, false ), "access_token={secret}" ) );

            // bodies: values of secret fields go
            exampleRedactor.RegisterBodySecrets( "application/json", "{\"user\":\"bob\",\"password\":\"PLANTEDbodyPASSWORD5\"}" );
            const CORE::CString body = exampleRedactor.RedactBody( "{\"user\":\"bob\",\"password\":\"PLANTEDbodyPASSWORD5\"}" );
            ASSERT_TRUE( Contains( body, "bob" ) );
            ASSERT_FALSE( Contains( body, "PLANTEDbodyPASSWORD5" ) );
            exampleRedactor.RegisterBodySecrets( "application/x-www-form-urlencoded", "a=1&client_secret=PLANTEDformSECRET10" );
            ASSERT_TRUE( exampleRedactor.ContainsKnownSecret( "PLANTEDformSECRET10" ) );

            // labels: digits as {n}, at most 60 characters
            ASSERT_TRUE( redactor.RedactLabel( "Export 2026 orders" ) == "Export {n} orders" );
            ASSERT_TRUE( redactor.RedactLabel( "Account 1234-5678 history" ) == "Account {n} history" );
            ASSERT_TRUE( redactor.RedactLabel( "abcdefghij abcdefghij abcdefghij abcdefghij abcdefghij abcdefghij abcdefghij" ).Length() <= 60 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformActionTrackerTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Requests are grouped by the user action that preceded them" )
        try
        {
            WEB::CUserActionTracker tracker;
            tracker.SetTimes( 3000, 15000, 1500 );
            bool started = false;

            // the first request of a session follows a pause
            ASSERT_TRUE( 1 == tracker.AssignRequest( 1000, started ) && started );
            ASSERT_TRUE( tracker.GetLastAction().kind == WEB::CObservedUserAction::KindPause );
            ASSERT_TRUE( 1 == tracker.AssignRequest( 2000, started ) && !started );

            // a click, then a chain of requests with gaps under the window
            const WEB::CObservedUserAction& click = tracker.StartAction( 10000, WEB::CObservedUserAction::KindClick, "Export", "https://shop/orders" );
            ASSERT_TRUE( 2 == click.id );
            ASSERT_TRUE( 2 == tracker.AssignRequest( 10100, started ) && !started );
            ASSERT_TRUE( 2 == tracker.AssignRequest( 12500, started ) );
            ASSERT_TRUE( 2 == tracker.AssignRequest( 15400, started ) );        // 2900 after the previous one

            // a request long after the chain, after a quiet gap: a new implicit action
            ASSERT_TRUE( 3 == tracker.AssignRequest( 20000, started ) && started );

            // polling keeps no action open beyond the maximum duration
            tracker.StartAction( 30000, WEB::CObservedUserAction::KindNavigation, CORE::CString::Empty, "https://shop/" );
            UInt32 last = 0;
            for ( UInt64 t=30500; t<=46000; t+=1000 )
                last = tracker.AssignRequest( t, started );
            ASSERT_TRUE( 0 == last );       // over 15 s after the start, and no quiet gap between the polls
            ASSERT_TRUE( 4 == tracker.GetActionCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformCatalogTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Requests of the same kind become one endpoint, the report holds structure only" )
        try
        {
            WEB::CSiteEndpointCatalog catalog;
            ASSERT_TRUE( catalog.Add( MakeObservation( 1, "GET", "https://shop.example/api/accounts/111111/orders?page=1", "XHR", 200, "application/json", 1000 ) ) );
            ASSERT_FALSE( catalog.Add( MakeObservation( 2, "GET", "https://shop.example/api/accounts/222222/orders?page=2&size=25", "XHR", 404, "application/json", 3000 ) ) );
            ASSERT_TRUE( catalog.Add( MakeObservation( 3, "GET", "https://cdn.shop.example/image/99999999", "Image", 200, "image/jpeg", 50000 ) ) );
            ASSERT_FALSE( catalog.Add( MakeObservation( 4, "GET", "https://shop.example/static/app.js", "Script", 200, "text/javascript", 10 ) ) );
            WEB::CObservedWebRequest login = MakeObservation( 5, "POST", "https://shop.example/account/login", "Fetch", 200, "application/json", 10 );
            login.postData = "{\"password\":\"PLANTEDsigninPASSWORD8\"}";
            ASSERT_TRUE( catalog.Add( login ) );
            WEB::CObservedWebRequest export_ = MakeObservation( 6, "GET", "https://shop.example/api/export/12345", "Document", 200, "text/csv", 2048 );
            export_.isDownload = true;
            export_.contentDisposition = "attachment";
            export_.downloadFileName = "Orders_{n}.csv";
            ASSERT_TRUE( catalog.Add( export_ ) );

            ASSERT_TRUE( catalog.GetRequestCount() == 6 );
            ASSERT_TRUE( catalog.GetEndpointCount( WEB::CSiteEndpointCatalog::KIND_DATA ) == 2 );
            ASSERT_TRUE( catalog.GetEndpointCount( WEB::CSiteEndpointCatalog::KIND_MEDIA ) == 1 );
            ASSERT_TRUE( catalog.GetEndpointCount( WEB::CSiteEndpointCatalog::KIND_PAGE ) == 1 );
            const WEB::CDiscoveredEndpoint* orders = catalog.FindEntry( "GET", "https://shop.example/api/accounts/555555/orders" );
            ASSERT_TRUE( GUCEF_NULL != orders );
            if ( GUCEF_NULL != orders )
            {
                ASSERT_TRUE( orders->requestCount == 2 && orders->totalBytes == 4000 );
                ASSERT_TRUE( orders->queryParams.size() == 2 && orders->queryParams.find( "page" )->second.empty() );
                ASSERT_TRUE( orders->exampleUrls.empty() );
            }
            const WEB::CDiscoveredEndpoint* signIn = catalog.FindEntry( "POST", "https://shop.example/account/login" );
            ASSERT_TRUE( GUCEF_NULL != signIn && signIn->neverRecorded );

            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( Contains( report, "### GET shop.example/api/accounts/{n}/orders" ) );
            ASSERT_TRUE( Contains( report, "200 (1), 404 (1)" ) );
            ASSERT_TRUE( Contains( report, "  - `page` (paging)" ) );
            ASSERT_TRUE( Contains( report, "### POST shop.example/account/login" ) );
            ASSERT_TRUE( Contains( report, "content not recorded" ) );
            ASSERT_TRUE( Contains( report, "Orders_{n}.csv" ) );
            ASSERT_TRUE( Contains( report, "| cdn.shop.example | 1 |" ) );
            ASSERT_TRUE( Contains( report, "Script: 1" ) );
            ASSERT_FALSE( Contains( report, "111111" ) || Contains( report, "page=" ) || Contains( report, "PLANTEDsigninPASSWORD8" ) );

            // the same observations give the same report
            CORE::CString again;
            ASSERT_TRUE( catalog.RenderReport( again ) && again == report );

            ASSERT_TRUE( WEB::CSiteEndpointCatalog::Classify( "XHR", "text/plain" ) == WEB::CSiteEndpointCatalog::KIND_DATA );
            ASSERT_TRUE( WEB::CSiteEndpointCatalog::Classify( "Document", "text/html" ) == WEB::CSiteEndpointCatalog::KIND_PAGE );
            ASSERT_TRUE( WEB::CSiteEndpointCatalog::Classify( "Other", "application/pdf" ) == WEB::CSiteEndpointCatalog::KIND_MEDIA );
            ASSERT_TRUE( WEB::CSiteEndpointCatalog::Classify( "Preflight", "" ) == WEB::CSiteEndpointCatalog::KIND_OTHER );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The journal keeps every line, a report part that would hold a registered value is replaced by a placeholder" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Refuse" );
            WEB::CSiteDiscoveryRedactor redactor;
            redactor.RegisterSecret( "PLANTEDapiKEY9" );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Open( CORE::CombinePath( dir.GetPath(), "requests.jsonl" ) ) );
            WEB::CObservedWebRequest bad = MakeObservation( 1, "GET", "https://shop.example/api/x", "XHR", 0, "", 0 );
            bad.failed = true;
            bad.errorText = "net::ERR_FAILED for PLANTEDapiKEY9";
            ASSERT_TRUE( journal.Append( bad ) );           // written as observed: the registry is for the report
            ASSERT_TRUE( journal.Append( MakeObservation( 2, "GET", "https://shop.example/api/y", "XHR", 200, "application/json", 1 ) ) );
            ASSERT_TRUE( 3 == journal.GetNextSequence() );
            journal.Close();
            CORE::CString text;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journal.GetPath(), text ) );
            ASSERT_TRUE( Contains( text, "/api/x" ) && Contains( text, "ERR_FAILED" ) && Contains( text, "/api/y" ) );
            ASSERT_FALSE( Contains( text, "\"redacted\"" ) );

            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            catalog.Add( MakeObservation( 2, "GET", "https://shop.example/api/y", "XHR", 200, "application/json", 1 ) );
            CORE::CString::StringVector extra;
            extra.push_back( "a note with PLANTEDapiKEY9 in it" );
            extra.push_back( "a clean note" );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report, extra ) );       // the part is left out, the report is written
            ASSERT_FALSE( Contains( report, "PLANTEDapiKEY9" ) );
            ASSERT_TRUE( Contains( report, "A summary line was left out" ) && Contains( report, "a clean note" ) && Contains( report, "/api/y" ) );
            ASSERT_TRUE( 1 == catalog.GetRedactedEntryCount() && catalog.GetRedactionSummary() == "other: 1" );
            ASSERT_TRUE( Contains( report, "left out because they would have contained a secret value" ) );
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( 0 == catalog.GetRedactedEntryCount() );

            // the journal rebuilds whole
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Load( journal.GetPath(), rebuilt ) == 2 );
            ASSERT_TRUE( 0 == rebuilt.GetStubCount() );
            ASSERT_TRUE( 2 == rebuilt.GetRequestCount() && 2 == rebuilt.GetEntries().size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Registers what a recorder of a store registers on a real run: cookie values (also short, numeric and word-like ones),
 *  the value of a header, and the values of body fields whose name matched the secret rules, among them ordinary words
 *  (the name word rule is a substring rule: keyImages holds key, author holds auth, sessionTimeout holds session) */
static void
RegisterRealisticSecrets( WEB::CSiteDiscoveryRedactor& redactor )
{
    redactor.CaptureHeaderValue( "cookie", "EPIC_LOCALE_COOKIE=en-US; theme=dark; consent=true; page_size=25; flag=published; cart_count=1234567890; "
                                           "EPIC_ENV=unreal-engine-5; cf_clearance=Qz7HxK2mPw9LrT4vNb8yJc3DfGhA5sEu" );
    redactor.CaptureHeaderValue( "authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.REALISTICpayload0123456789.sigREALISTIC0123456789abcd" );
    redactor.RegisterBodySecrets( "application/json",
        "{\"keyImages\":[{\"url\":\"https://cdn1.epicgames.com/offer/img.jpg\",\"type\":\"Thumbnail\",\"md5\":\"9e107d9d372bb6826bd81d3542a419d6\"}],"
        "\"assetFormatType\":{\"code\":\"unreal-engine\"},\"author\":{\"displayName\":\"Megascans Studio\",\"listingsUrl\":\"https://www.fab.com/sellers/megascans\"},"
        "\"state\":\"published\",\"sessionTimeoutSeconds\":1800000000,\"chlToken\":\"/cdn-cgi/challenge-platform/h/g/orchestrate/chl_page/v1\"}" );
    redactor.RegisterQuerySecrets( "https://www.fab.com/i/listings/search?state=published&code=download-info&ticket=listings&pin=12345678" );
}

/*-------------------------------------------------------------------------*/

/** The ordinary traffic of a store: paths and shapes made of the same ordinary words and numbers */
static void
AddOrdinaryTraffic( WEB::CSiteDiscoveryJournal& journal, WEB::CSiteEndpointCatalog& catalog, UInt32& journalRefused, UInt32& catalogCount )
{
    WEB::CObservedWebRequest formats = MakeObservation( 1, "GET", "https://www.fab.com/i/listings/3f2504e0-4f89-41d3-9a0c-0305e82c3301/asset-formats/unreal-engine", "XHR", 200, "application/json", 900 );
    formats.hasResponseShape = true;
    formats.responseShape.lines.push_back( "published: boolean" );
    formats.responseShape.lines.push_back( "assetFormatType.code: string" );
    formats.responseShape.lines.push_back( "author.displayName: string" );
    formats.responseShape.lines.push_back( "keyImages[].type: string" );
    WEB::CObservedWebRequest challenge = MakeObservation( 2, "GET", "https://www.fab.com/cdn-cgi/challenge-platform/h/g/orchestrate/chl_page/v1?ray=1", "Fetch", 200, "application/json", 300 );
    WEB::CObservedWebRequest download = MakeObservation( 3, "GET", "https://content-download.example.test/Builds/Org/12345/source.zip", "Fetch", 200, "application/zip", 1234567890ULL );
    download.isDownload = true;
    download.downloadFileName = "source.zip";
    WEB::CObservedWebRequest listing = MakeObservation( 4, "GET", "https://www.fab.com/i/listings/search?state&code&ticket&pin", "XHR", 200, "application/json", 1200 );
    listing.requestHeaders[ "x-requested-with" ] = "XMLHttpRequest";
    WEB::CObservedUserAction click;
    click.id = 1;
    click.timestamp = 100.0;
    click.kind = WEB::CObservedUserAction::KindClick;
    click.label = "Megascans Studio";
    click.page = "https://www.fab.com/sellers";
    if ( !journal.AppendAction( click ) )
        ++journalRefused;
    catalog.AddAction( click );
    listing.actionId = 1;
    WEB::CObservedWebRequest* all[] = { &formats, &challenge, &download, &listing };
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( !journal.Append( *all[ i ] ) )
            ++journalRefused;
        catalog.Add( *all[ i ] );
    }
    catalogCount = catalog.GetRequestCount();
}

/*-------------------------------------------------------------------------*/

static void
PerformTripwireTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: The tripwire looks for tokens and not for words, numbers and paths, with a threshold that is a setting" )
        try
        {
            WEB::CSiteDiscoverySettings settings;
            ASSERT_TRUE( 12 == settings.knownSecretMinLength && 32 == settings.knownSecretMinEntropyBits );
            WEB::CSiteDiscoveryRedactor redactor( settings );

            // never secrets: words, identifiers, ordinary numbers, URLs and paths of words, short values, structural values
            ASSERT_FALSE( redactor.IsPlausibleSecret( "published" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "unreal-engine" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "download-info-format" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "1234567890" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "1700000000123" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "true" ) || redactor.IsPlausibleSecret( "en-US" ) || redactor.IsPlausibleSecret( "1" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "application/json" ) || redactor.IsPlausibleSecret( "multipart/form-data; boundary=x" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "/cdn-cgi/challenge-platform/h/g/orchestrate/chl_page/v1" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "https://www.fab.com/sellers/megascans" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "Megascans Studio" ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( WEB::CSiteDiscoveryRedactor::NotRecordedMarker ) || redactor.IsPlausibleSecret( WEB::CSiteDiscoveryRedactor::SecretMarker ) );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "a1a1a1a1a1a1a1" ) );      // a pattern: too little entropy
            ASSERT_FALSE( redactor.IsPlausibleSecret( "ab12cd3" ) );             // short
            // secrets: tokens, hashes, long numbers
            for ( UInt32 i=0; GUCEF_NULL != PlantedSecrets[ i ]; ++i )
                ASSERT_TRUE( redactor.IsPlausibleSecret( PlantedSecrets[ i ] ) );
            ASSERT_TRUE( redactor.IsPlausibleSecret( "9e107d9d372bb6826bd81d3542a419d6" ) );
            ASSERT_TRUE( redactor.IsPlausibleSecret( "Qz7HxK2mPw9LrT4vNb8yJc3DfGhA5sEu" ) );
            ASSERT_TRUE( redactor.IsPlausibleSecret( "4929123412341234" ) );     // 16 digits
            ASSERT_TRUE( redactor.IsPlausibleSecret( "eyJhbGciOiJIUzI1NiJ9.payload01234567.sigREALISTIC0123456789abcd" ) );

            // the threshold is a setting; below the floor it is refused
            settings.knownSecretMinLength = 20;
            redactor.SetSettings( settings );
            ASSERT_FALSE( redactor.IsPlausibleSecret( "PLANTEDapiKEY9" ) );
            ASSERT_TRUE( redactor.IsPlausibleSecret( "PLANTEDbodyPASSWORD5" ) );
            settings.knownSecretMinLength = 12;
            settings.knownSecretMinEntropyBits = 0;
            redactor.SetSettings( settings );
            ASSERT_TRUE( redactor.IsPlausibleSecret( "a1a1a1a1a1a1a1" ) );
            CORE::CDataNode cfg( GUCEF_DATATYPE_OBJECT );
            cfg.SetAttribute( "knownSecretMinLength", (UInt32) 4 );
            WEB::CSiteDiscoverySettings loaded;
            ASSERT_FALSE( loaded.LoadConfig( cfg ) );
            cfg.SetAttribute( "knownSecretMinLength", (UInt32) 16 );
            cfg.SetAttribute( "knownSecretMinEntropyBits", (UInt32) 40 );
            ASSERT_TRUE( loaded.LoadConfig( cfg ) && 16 == loaded.knownSecretMinLength && 40 == loaded.knownSecretMinEntropyBits );
            CORE::CDataNode saved( GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( loaded.SaveConfig( saved ) );
            WEB::CSiteDiscoverySettings reloaded;
            ASSERT_TRUE( reloaded.LoadConfig( saved ) && 16 == reloaded.knownSecretMinLength && 40 == reloaded.knownSecretMinEntropyBits );

            // registering follows the rule and counts what it did not accept
            WEB::CSiteDiscoveryRedactor registry;
            registry.RegisterSecret( "published" );
            registry.RegisterSecret( "1234567890" );
            registry.RegisterSecret( "PLANTEDapiKEY9" );
            ASSERT_TRUE( 1 == registry.GetKnownSecretCount() && 2 == registry.GetIgnoredSecretCount() );
            ASSERT_FALSE( registry.ContainsKnownSecret( "published 1234567890" ) );

            // a value shorter than 24 bytes counts as a whole token only; a long one is found anywhere
            ASSERT_TRUE( registry.ContainsKnownSecret( "a/PLANTEDapiKEY9?x" ) && registry.ContainsKnownSecret( "PLANTEDapiKEY9" ) && registry.ContainsKnownSecret( "x=PLANTEDapiKEY9&y" ) );
            ASSERT_TRUE( registry.ContainsKnownSecret( "q=a%3DPLANTEDapiKEY9" ) && registry.ContainsKnownSecret( "line\\nPLANTEDapiKEY9" ) );
            ASSERT_FALSE( registry.ContainsKnownSecret( "xPLANTEDapiKEY9y" ) || registry.ContainsKnownSecret( "PLANTEDapiKEY99" ) );
            ASSERT_TRUE( registry.ScrubKnownSecrets( "a PLANTEDapiKEY9, PLANTEDapiKEY9; xPLANTEDapiKEY9" ) == "a {secret}, {secret}; xPLANTEDapiKEY9" );
            registry.RegisterSecret( "Qz7HxK2mPw9LrT4vNb8yJc3DfGhA5sEu" );
            ASSERT_TRUE( registry.ContainsKnownSecret( "prefixQz7HxK2mPw9LrT4vNb8yJc3DfGhA5sEusuffix" ) );
            ASSERT_TRUE( registry.ScrubKnownSecrets( "aQz7HxK2mPw9LrT4vNb8yJc3DfGhA5sEub" ) == "a{secret}b" );

            // addresses are never looked for as a whole: host names, cookie domains, URLs and paths; a token in one is
            const UInt32 before = registry.GetKnownSecretCount();
            registry.RegisterSecret( "www.shop-example.test", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "token" );
            registry.RegisterSecret( ".shop-example.test", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "cookieDomain" );
            registry.RegisterSecret( "shop-example.myshopify.com", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "shop_token" );
            registry.RegisterSecret( "https://www.shop-example.test/collections/kits2026", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "auth_url" );
            registry.RegisterSecret( "/collections/kits2026", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "session_path" );
            ASSERT_TRUE( registry.GetKnownSecretCount() == before );           // the path segments are words or too short
            ASSERT_FALSE( registry.ContainsKnownSecret( "{\"url\":\"https://www.shop-example.test/collections/all\"}" ) );
            registry.RegisterSecret( "https://cdn.example.test/dl?sig=Zt8Qp3Lm9Xc2Vb7Nk4Hs&name=file", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "auth_url" );
            ASSERT_TRUE( registry.ContainsKnownSecret( "x=Zt8Qp3Lm9Xc2Vb7Nk4Hs" ) );

            // body field values only count as whole tokens, also long ones; credentials are found anywhere when long
            registry.RegisterSecret( "AbCdEfGhIjKlMnOpQrStUvWx12", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "secret" );
            ASSERT_FALSE( registry.ContainsKnownSecret( "blob=AbCdEfGhIjKlMnOpQrStUvWx12345xyz" ) );
            ASSERT_TRUE( registry.ContainsKnownSecret( "blob=AbCdEfGhIjKlMnOpQrStUvWx12&x" ) );
            registry.RegisterSecret( "Kq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Se", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_COOKIE, "sid" );
            ASSERT_TRUE( registry.ContainsKnownSecret( "zzKq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Sezz" ) );

            // a value seen in a request URL is public; a credential never is
            registry.RegisterSecret( "c1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "checkout_token" );
            ASSERT_TRUE( registry.ContainsKnownSecret( "/cart/c1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6/x" ) );
            registry.NotePublicUrl( "https://shop.example.test/cart/c1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6/update?sid=Kq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Se" );
            ASSERT_FALSE( registry.ContainsKnownSecret( "/cart/c1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6/x" ) );
            registry.NotePublicUrl( "https://shop.example.test/x?k=Kq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Se" );
            ASSERT_TRUE( registry.ContainsKnownSecret( "Kq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Se" ) );
            registry.RegisterSecret( "pk_live_PUBLICw1dg3tKEY4567", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_RESPONSE_BODY_FIELD, "api_key" );
            registry.NotePublicUrl( "https://tracker.widget.test/collect?k=pk_live_PUBLICw1dg3tKEY4567" );
            ASSERT_FALSE( registry.ContainsKnownSecret( "?k=pk_live_PUBLICw1dg3tKEY4567" ) );

            // the journal mode: credentials only, and the diagnostics name a value without showing it
            registry.SetJournalMode( true );
            ASSERT_FALSE( registry.ContainsKnownSecret( "blob=AbCdEfGhIjKlMnOpQrStUvWx12&x" ) );
            ASSERT_TRUE( registry.ScrubKnownSecrets( "id=Kq3Wm8Zp2Lx7Rv5Tn9Yc4Hb6Jd1Fg0Se" ) == "id={secret}" );
            registry.SetJournalMode( false );
            UInt32 index = 0;
            ASSERT_TRUE( registry.FindKnownSecretIndex( "blob=AbCdEfGhIjKlMnOpQrStUvWx12&x", index ) );
            ASSERT_TRUE( registry.DescribeSecret( index ) == "class response-body-field, field `secret`, length 26, entropy " + CORE::UInt32ToString( registry.GetSecretInfo( index ).entropyBits ) + " bits" );
            ASSERT_TRUE( 1 == registry.AssessJournal( "a AbCdEfGhIjKlMnOpQrStUvWx12 b\nc\nd\n" ) );     // one line of three: taken for public
            ASSERT_TRUE( registry.GetSecretInfo( index ).suspectedPublic && 1 == registry.GetSecretInfo( index ).journalLines );
            ASSERT_FALSE( registry.ContainsKnownSecret( "blob=AbCdEfGhIjKlMnOpQrStUvWx12&x" ) );
            ASSERT_TRUE( Contains( registry.DescribeSecretUse( index ), "in 1 of 3 journal lines (lines 1)" ) && Contains( registry.DescribeSecretUse( index ), "probably not a secret" ) );
            ASSERT_FALSE( Contains( registry.DescribeSecretUse( index ), "AbCdEf" ) );

            // ignoredSecretFields: never looked for; a password field always is
            WEB::CSiteDiscoverySettings ignoring;
            ignoring.ignoredSecretFields.push_back( "storefront*" );
            ignoring.ignoredSecretFields.push_back( "*password*" );
            WEB::CSiteDiscoveryRedactor ignorer( ignoring );
            ignorer.RegisterBodySecrets( "application/json", "{\"storefrontAccessToken\":\"Zt8Qp3Lm9Xc2Vb7Nk4Hs\",\"password\":\"Pw7Rt3Yx9Lk2Mn5Bq8\"}" );
            ASSERT_FALSE( ignorer.ContainsKnownSecret( "Zt8Qp3Lm9Xc2Vb7Nk4Hs" ) );
            ASSERT_TRUE( ignorer.ContainsKnownSecret( "Pw7Rt3Yx9Lk2Mn5Bq8" ) );
            ASSERT_TRUE( WEB::CSiteDiscoveryRedactor::IsPasswordFieldName( "currentPassword" ) && !WEB::CSiteDiscoveryRedactor::IsPasswordFieldName( "token" ) );
            CORE::CDataNode ignoreCfg( GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( ignoring.SaveConfig( ignoreCfg ) );
            WEB::CSiteDiscoverySettings ignoreLoaded;
            ASSERT_TRUE( ignoreLoaded.LoadConfig( ignoreCfg ) && 2 == ignoreLoaded.ignoredSecretFields.size() && 20 == ignoreLoaded.tripwireMaxLineShare );
            ignoreCfg.SetAttribute( "tripwireMaxLineShare", (UInt32) 101 );
            ASSERT_FALSE( ignoreLoaded.LoadConfig( ignoreCfg ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Ordinary traffic is not refused because look-alike values were registered" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Tripwire" );
            WEB::CSiteDiscoveryRedactor redactor;
            RegisterRealisticSecrets( redactor );
            ASSERT_TRUE( redactor.GetKnownSecretCount() > 0 );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Open( CORE::CombinePath( dir.GetPath(), "requests.jsonl" ) ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );

            UInt32 journalRefused = 0;
            UInt32 requests = 0;
            AddOrdinaryTraffic( journal, catalog, journalRefused, requests );
            journal.Close();
            ASSERT_TRUE( 4 == requests );

            // none of it holds a secret: nothing is replaced, and the report is written whole
            ASSERT_TRUE( 0 == journalRefused );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( 0 == catalog.GetRedactedEntryCount() );
            ASSERT_TRUE( Contains( report, "asset-formats/unreal-engine" ) );
            ASSERT_TRUE( Contains( report, "chl_page" ) );
            ASSERT_TRUE( Contains( report, "Megascans Studio" ) );
            ASSERT_FALSE( Contains( report, "left out" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The journal keeps every line with credential values masked, the report leaves out the parts with a registered value and names it without its value" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Degrade" );
            WEB::CSiteDiscoveryRedactor redactor;
            RegisterRealisticSecrets( redactor );
            redactor.RegisterSecret( "PLANTEDcookieVALUE2", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_COOKIE );
            redactor.RegisterSecret( "PLANTEDqueryTOKEN1", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_QUERY_VALUE );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );

            WEB::CObservedWebRequest first = MakeObservation( 1, "GET", "https://shop.example/api/a", "XHR", 200, "application/json", 10 );
            WEB::CObservedWebRequest second = MakeObservation( 2, "GET", "https://shop.example/api/b", "XHR", 200, "application/x-PLANTEDcookieVALUE2", 20 );
            WEB::CObservedWebRequest third = MakeObservation( 3, "GET", "https://shop.example/api/c", "XHR", 200, "application/json", 30 );
            third.isDownload = true;
            third.downloadFileName = "PLANTEDqueryTOKEN1.csv";
            WEB::CObservedWebRequest fourth = MakeObservation( 4, "GET", "https://shop.example/api/d", "XHR", 200, "application/json", 40 );
            WEB::CObservedWebRequest* all[] = { &first, &second, &third, &fourth };
            for ( UInt32 i=0; i<4; ++i )
            {
                journal.Append( *all[ i ] );
                catalog.Add( *all[ i ] );
            }
            WEB::CObservedUserAction action;
            action.id = 1;
            action.timestamp = 5.0;
            action.kind = WEB::CObservedUserAction::KindNavigation;
            action.page = "https://shop.example/PLANTEDcookieVALUE2/home";
            journal.AppendAction( action );
            WEB::CObservedUserAction fine;
            fine.id = 2;
            fine.timestamp = 6.0;
            fine.kind = WEB::CObservedUserAction::KindClick;
            fine.label = "Export";
            fine.page = "https://shop.example/home";
            journal.AppendAction( fine );
            journal.Close();

            // the journal: every line, the cookie value masked where it stood, the rest as observed
            ASSERT_TRUE( 5 == journal.GetNextSequence() );
            CORE::CString text;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, text ) );
            ASSERT_FALSE( Contains( text, "PLANTEDcookieVALUE2" ) );
            ASSERT_TRUE( Contains( text, "application/x-{secret}" ) && Contains( text, "https://shop.example/{secret}/home" ) );
            ASSERT_TRUE( Contains( text, "/api/a" ) && Contains( text, "/api/b" ) && Contains( text, "/api/c" ) && Contains( text, "/api/d" ) && Contains( text, "\"label\":\"Export\"" ) );
            ASSERT_TRUE( Contains( text, "PLANTEDqueryTOKEN1.csv" ) );        // not a credential: for the report to decide
            ASSERT_FALSE( Contains( text, "\"redacted\"" ) );

            // the report: the same two endpoints are replaced by a placeholder with their number and the class, the others are whole
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( report ) );
            ASSERT_TRUE( 2 == catalog.GetRedactedEntryCount() && catalog.GetRedactionSummary() == "cookie: 1, query-value: 1" );
            ASSERT_TRUE( Contains( report, "### Endpoint 2 of 4: left out" ) && Contains( report, "secret class cookie, left out 1" ) );
            ASSERT_TRUE( Contains( report, "### Endpoint 3 of 4: left out" ) && Contains( report, "secret class query-value, left out 2" ) );
            ASSERT_TRUE( Contains( report, "Host `shop.example`, method GET" ) );
            ASSERT_TRUE( Contains( report, "class cookie, field (unknown), length 19, entropy" ) && Contains( report, "class query-value, field (unknown), length 18" ) );
            ASSERT_TRUE( Contains( report, "### GET shop.example/api/a" ) && Contains( report, "### GET shop.example/api/d" ) );
            ASSERT_TRUE( Contains( report, "(cookie: 1, query-value: 1)" ) );
            ASSERT_FALSE( Contains( report, "/api/b" ) || Contains( report, "/api/c" ) );

            // a report rebuilt from the journal, without a registry as a later run has it, has every endpoint
            WEB::CSiteDiscoveryRedactor later;
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &later );
            ASSERT_TRUE( journal.Load( journalPath, rebuilt ) == 4 );
            ASSERT_TRUE( 0 == rebuilt.GetStubCount() );
            ASSERT_TRUE( 4 == rebuilt.GetRequestCount() && 4 == rebuilt.GetEntries().size() );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
            ASSERT_TRUE( Contains( rebuiltReport, "### GET shop.example/api/a" ) && Contains( rebuiltReport, "### GET shop.example/api/b" ) && Contains( rebuiltReport, "### GET shop.example/api/c" ) );
            ASSERT_TRUE( 0 == rebuilt.GetRedactedEntryCount() );
            ASSERT_FALSE( Contains( rebuiltReport, "PLANTEDcookieVALUE2" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The session rebuilds the report from an existing journal, and says what was left out" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Rebuild" );
            WEB::CSiteDiscoverySettings settings;
            settings.outputDirectory = dir.GetPath();

            // a first session has a journal, with a stub line that an older recorder wrote
            WEB::CSiteDiscoverySession writer;
            writer.SetSettings( settings );
            writer.GetRedactor().RegisterSecret( "PLANTEDcookieVALUE2", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_COOKIE );
            ASSERT_TRUE( writer.GetJournal().Open( writer.GetJournalPath() ) );
            ASSERT_TRUE( writer.GetJournal().Append( MakeObservation( 1, "GET", "https://shop.example/api/a", "XHR", 200, "application/json", 10 ) ) );
            ASSERT_TRUE( writer.GetJournal().Append( MakeObservation( 2, "GET", "https://shop.example/api/b", "XHR", 200, "application/x-PLANTEDcookieVALUE2", 20 ) ) );
            writer.GetJournal().Close();
            {
                CORE::CString journalText;
                ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( writer.GetJournalPath(), journalText ) );
                ASSERT_TRUE( Contains( journalText, "/api/b" ) && !ContainsAnyPlantedSecret( journalText ) );
                journalText += "{\"seq\":3,\"ts\":103,\"method\":\"\",\"url\":\"{redacted}\",\"redacted\":\"response-body-field\"}\n";
                CORE::CString problem;
                ASSERT_TRUE( CORE::DeleteFile( writer.GetJournalPath() ) );
                ASSERT_TRUE( CORE::CreateOwnerOnlyFile( writer.GetJournalPath(), journalText.C_String(), journalText.ByteSize() - 1, problem ) );
            }

            // a session that rebuilds knows no secret (they are not on disk) and reads the journal only
            CORE::CString journalBefore;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( writer.GetJournalPath(), journalBefore ) );
            WEB::CSiteDiscoverySession rebuild;
            rebuild.SetSettings( settings );
            CORE::CString problem;
            ASSERT_TRUE( rebuild.RebuildReportFromJournal( problem ) );
            CORE::CString journalAfter;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( rebuild.GetJournalPath(), journalAfter ) );
            ASSERT_TRUE( journalBefore == journalAfter );
            CORE::CString report;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( rebuild.GetReportPath(), report ) );
            ASSERT_TRUE( Contains( report, "### GET shop.example/api/a" ) && Contains( report, "### GET shop.example/api/b" ) );
            ASSERT_TRUE( Contains( report, "an older recorder reduced to a stub" ) && Contains( report, "cannot be recovered" ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( report ) );
            ASSERT_TRUE( 1 == rebuild.GetCatalog().GetStubCount() && 0 == rebuild.GetCatalog().GetRedactedEntryCount() );
            const CORE::CString notice = rebuild.GetRedactionNotice();
            ASSERT_TRUE( Contains( notice, "1 journal line(s) are stubs" ) && Contains( notice, "response-body-field: 1" ) && Contains( notice, "cannot be recovered" ) && !Contains( notice, "PLANTED" ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == ModeOf( rebuild.GetReportPath() ) );
            #endif

            // a live session that leaves a report part out says so, with the classes
            WEB::CSiteDiscoverySession live;
            live.SetSettings( settings );
            live.GetRedactor().RegisterSecret( "PLANTEDcookieVALUE2", WEB::CSiteDiscoveryRedactor::SECRET_CLASS_COOKIE );
            WEB::CObservedWebRequest bad = MakeObservation( 3, "GET", "https://shop.example/api/b", "XHR", 200, "application/x-PLANTEDcookieVALUE2", 20 );
            live.GetCatalog().Add( bad );
            ASSERT_TRUE( live.WriteReport( problem ) );
            ASSERT_TRUE( 1 == live.GetCatalog().GetRedactedEntryCount() );
            ASSERT_TRUE( Contains( live.GetRedactionNotice(), "1 part(s) of the report were left out" ) && Contains( live.GetRedactionNotice(), "cookie: 1" ) );
            ASSERT_TRUE( Contains( live.GetRedactionNotice(), "class cookie, field (unknown), length 19" ) && !Contains( live.GetRedactionNotice(), "PLANTED" ) );

            // no journal, nothing to rebuild; and a recording session is not rebuilt
            TESTFW::CScopedTestDir empty( "gucefWEB_SiteDiscovery_RebuildEmpty" );
            WEB::CSiteDiscoverySettings emptySettings;
            emptySettings.outputDirectory = empty.GetPath();
            WEB::CSiteDiscoverySession none;
            none.SetSettings( emptySettings );
            ASSERT_FALSE( none.RebuildReportFromJournal( problem ) );
            ASSERT_TRUE( Contains( problem, "no journal" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBrowserHelperTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: The browser helpers: active port file, arguments, environment, downloads" )
        try
        {
            UInt16 port = 0;
            CORE::CString path;
            ASSERT_TRUE( WEB::CDevToolsBrowser::ParseDevToolsActivePort( "9333\n/devtools/browser/abc-123\n", port, path ) );
            ASSERT_TRUE( 9333 == port && path == "/devtools/browser/abc-123" );
            ASSERT_TRUE( WEB::CDevToolsBrowser::ParseDevToolsActivePort( "9334\r\n/devtools/browser/x", port, path ) && 9334 == port );
            ASSERT_FALSE( WEB::CDevToolsBrowser::ParseDevToolsActivePort( "70000\n/devtools/browser/x", port, path ) );
            ASSERT_FALSE( WEB::CDevToolsBrowser::ParseDevToolsActivePort( "9333\nnot-a-path", port, path ) );
            ASSERT_FALSE( WEB::CDevToolsBrowser::ParseDevToolsActivePort( "", port, path ) );

            WEB::CDevToolsBrowser browser;
            browser.SetDebugPort( 9444 );
            browser.SetProfileDir( "/tmp/profile" );
            browser.SetHeadless( true );
            const CORE::CString::StringVector args = browser.BuildArguments( "https://shop.example/" );
            ASSERT_TRUE( !args.empty() && args[ 0 ] == "--remote-debugging-port=9444" );
            ASSERT_TRUE( args.size() >= 2 && args[ 1 ] == "--user-data-dir=/tmp/profile" );
            ASSERT_TRUE( args.back() == "https://shop.example/" );

            WEB::CDevToolsBrowser::TEnvironment environment;
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "SNAP", "/snap/code/1" ) );
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "SNAP_NAME", "code" ) );
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "GTK_PATH", "/snap/x" ) );
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "XDG_DATA_DIRS", "/snap/share" ) );
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "XDG_DATA_DIRS_VSCODE_SNAP_ORIG", "/usr/share" ) );
            environment.push_back( WEB::CDevToolsBrowser::TNameValue( "HOME", "/home/x" ) );
            CORE::CString::StringVector unsetNames;
            WEB::CDevToolsBrowser::TStringMap overrides;
            WEB::CDevToolsBrowser::BuildCleanEnvironment( environment, unsetNames, overrides );
            ASSERT_TRUE( overrides.size() == 1 && overrides[ "XDG_DATA_DIRS" ] == "/usr/share" );
            ASSERT_TRUE( unsetNames.size() == 4 );      // SNAP, SNAP_NAME, GTK_PATH and the saved original
            environment.erase( environment.begin() );
            WEB::CDevToolsBrowser::BuildCleanEnvironment( environment, unsetNames, overrides );
            ASSERT_TRUE( unsetNames.empty() && overrides.empty() );

            CORE::CString type;
            CORE::CString name;
            WEB::CChromeNetworkRecorder::ParseContentDisposition( "attachment; filename=\"Positions_20261121_1234.csv\"", type, name );
            ASSERT_TRUE( type == "attachment" && name == "Positions_{n}_{n}.csv" );
            WEB::CChromeNetworkRecorder::ParseContentDisposition( "inline; filename*=UTF-8''Report%202026.pdf", type, name );
            ASSERT_TRUE( type == "inline" && name == "Report {n}.pdf" );
            WEB::CChromeNetworkRecorder::ParseContentDisposition( "attachment; filename=\"../../etc/x99.txt\"", type, name );
            ASSERT_TRUE( name == "x{n}.txt" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Plays a browsing session on the fake browser: a navigation, a click on an export button with the requests behind it,
 *  a data request with a JSON response, a sign-in, a request whose body the browser keeps, a tracker of another host */
static void
PlayBrowsingSession( CFakeDevToolsBrowser& browser, WEB::CChromeNetworkRecorder& recorder, const CORE::CString& session )
{
    CFakeDevToolsBrowser::TStringMap headers;
    CFakeDevToolsBrowser::TStringMap none;

    headers[ "Cookie" ] = "sid=PLANTEDcookieVALUE2; theme=dark";
    headers[ "Authorization" ] = "Bearer PLANTEDbearerTOKEN3";
    headers[ "Accept" ] = "text/html";
    browser.EmitRequest( session, "R1", "GET", "http://shop.example.test/orders/12345678?sessionToken=PLANTEDqueryTOKEN1&page=2", "Document", "P1", headers, "", false, 100.0 );
    CFakeDevToolsBrowser::TStringMap htmlHeaders;
    htmlHeaders[ "Content-Type" ] = "text/html; charset=utf-8";
    htmlHeaders[ "Set-Cookie" ] = "refresh=PLANTEDresponseTOKEN7; HttpOnly";
    browser.EmitResponse( session, "R1", 200, "text/html", htmlHeaders, "Document" );
    browser.EmitFinished( session, "R1", 5000 );
    Pump( 30, &recorder );

    // the click listener reports a click on the export button
    CORE::CDataNode binding( GUCEF_DATATYPE_OBJECT );
    binding.SetAttribute( "name", CORE::CVariant( WEB::CChromeNetworkRecorder::ActionBindingName ) );
    binding.SetAttribute( "payload", CORE::CVariant( CORE::CString( "{\"kind\":\"click\",\"tag\":\"button\",\"label\":\"Export 2026 to CSV\"}" ) ) );
    binding.SetAttribute( "executionContextId", CORE::CVariant( (Int32) 1 ) );
    browser.Emit( session, "Runtime.bindingCalled", binding );
    Pump( 10, &recorder );

    headers.clear();
    headers[ "Content-Type" ] = "application/json";
    headers[ "X-CSRF-Token" ] = "PLANTEDcsrfTOKEN4";
    browser.EmitRequest( session, "R2", "POST", "http://shop.example.test/api/orders/12345678/export", "Fetch", "P1", headers,
                         "{\"format\":\"csv\",\"password\":\"PLANTEDbodyPASSWORD5\",\"accountId\":12345678,\"range\":{\"from\":\"2026-01-01\"}}", true, 101.0 );
    CFakeDevToolsBrowser::TStringMap csvHeaders;
    csvHeaders[ "Content-Type" ] = "text/csv";
    csvHeaders[ "Content-Disposition" ] = "attachment; filename=\"Orders_20261121.csv\"";
    browser.EmitResponse( session, "R2", 200, "text/csv", csvHeaders, "Fetch" );
    browser.EmitFinished( session, "R2", 20480 );

    browser.SetResponseBody( "R3", "{\"total\":120,\"page\":1,\"next\":\"/api/orders?page=2\",\"items\":[{\"id\":1,\"name\":\"Private Person Name\",\"price\":9.5}],"
                                   "\"refreshToken\":\"PLANTEDresponseTOKEN7\"}" );
    browser.EmitRequest( session, "R3", "GET", "http://shop.example.test/api/orders?page=1&limit=25&access_token=PLANTEDaccessTOKEN6", "Fetch", "P1", none, "", false, 102.0 );
    CFakeDevToolsBrowser::TStringMap jsonHeaders;
    jsonHeaders[ "content-type" ] = "application/json";
    browser.EmitResponse( session, "R3", 200, "application/json", jsonHeaders, "Fetch" );
    browser.EmitFinished( session, "R3", 900 );

    headers.clear();
    headers[ "Content-Type" ] = "application/json";
    browser.EmitRequest( session, "R4", "POST", "http://shop.example.test/login", "Fetch", "P1", headers,
                         "{\"username\":\"someone\",\"password\":\"PLANTEDsigninPASSWORD8\"}", true, 103.0 );
    browser.EmitResponse( session, "R4", 200, "application/json", jsonHeaders, "Fetch" );
    browser.EmitFinished( session, "R4", 100 );

    headers.clear();
    headers[ "Content-Type" ] = "application/x-www-form-urlencoded";
    browser.SetRequestPostData( "R5", "query=lamp&count=10&apiKey=PLANTEDapiKEY9&client_secret=PLANTEDformSECRET10" );
    browser.EmitRequest( session, "R5", "POST", "http://shop.example.test/api/search", "XHR", "P1", headers, "", true, 104.0 );
    browser.EmitResponse( session, "R5", 204, "", none, "XHR" );
    browser.EmitFinished( session, "R5", 0 );

    browser.EmitRequest( session, "R6", "GET", "http://tracker.other.test/pixel?id=PLANTEDcookieVALUE2", "Image", "P1", none, "", false, 105.0 );
    browser.EmitResponse( session, "R6", 200, "image/gif", none, "Image" );
    browser.EmitFinished( session, "R6", 40 );

    for ( UInt32 i=0; i<2000 && recorder.GetRecordedRequestCount() < 5; ++i )
        Pump( 1, &recorder );
    Pump( 30, &recorder );
}

/*-------------------------------------------------------------------------*/

static void
PerformRecorderTests( void )
{
    if ( !WebSocketAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Site discovery recorder TESTS skipped: the sha1 codec plugin for web sockets is not available" );
        return;
    }

    CFakeDevToolsBrowser browser;
    const bool started = browser.Start();
    browser.AddTarget( "P1", "page", "http://shop.example.test/" );
    browser.AddTarget( "P2", "page", "http://other.example.test/inbox" );
    browser.AddTarget( "W1", "service_worker", "http://shop.example.test/sw.js" );

    GUCEF_TESTFW_TESTCASE( "Test: The recorder records a browsing session of the fake browser and keeps every planted secret out" )
        try
        {
            ASSERT_TRUE( started );
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Recorder" );

            WEB::CChromeDevToolsClient client;
            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) );
            ASSERT_TRUE( client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.captureClicks = true;
            settings.saveResponseBodies = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );

            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.SetBodyDirectory( dir.GetPath() );
            recorder.AddSink( &journal );
            recorder.AddSink( &catalog );
            recorder.SetNextSequence( journal.GetNextSequence() );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );

            // only the tab of the site is watched, and set up with the commands that only listen
            ASSERT_TRUE( browser.WasAttachedTo( "P1" ) );
            ASSERT_FALSE( browser.WasAttachedTo( "P2" ) || browser.WasAttachedTo( "W1" ) );
            ASSERT_TRUE( recorder.GetSessionCount() == 1 );
            ASSERT_TRUE( browser.WasCommandSent( "Network.enable", "S-P1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Page.enable", "S-P1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Target.setAutoAttach", "S-P1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Runtime.addBinding", "S-P1" ) );
            ASSERT_FALSE( browser.WasCommandSent( "Network.setCacheDisabled", "S-P1" ) );

            PlayBrowsingSession( browser, recorder, "S-P1" );
            ASSERT_TRUE( recorder.GetRecordedRequestCount() == 5 );        // not the tracker of the other host
            ASSERT_TRUE( browser.WasCommandSent( "Network.getRequestPostData", "S-P1" ) );
            ASSERT_TRUE( browser.WasCommandSent( "Network.getResponseBody", "S-P1" ) );

            // the page, with its query names and no cookie or credential
            const WEB::CDiscoveredEndpoint* page = catalog.FindEntry( "GET", "http://shop.example.test/orders/1" + CORE::CString( "1111111" ) );
            ASSERT_TRUE( GUCEF_NULL != page );
            if ( GUCEF_NULL != page )
            {
                ASSERT_TRUE( page->kind == WEB::CSiteEndpointCatalog::KIND_PAGE );
                ASSERT_TRUE( page->queryParams.find( "sessionToken" ) != page->queryParams.end() );
                ASSERT_TRUE( page->requestHeaderNames.count( "cookie" ) == 1 && page->requestHeaderNames.count( "authorization" ) == 1 );
            }

            // the export: the shape of its body, the file it gave, the click behind it
            const WEB::CDiscoveredEndpoint* exporter = catalog.FindEntry( "POST", "http://shop.example.test/api/orders/99999999/export" );
            ASSERT_TRUE( GUCEF_NULL != exporter );
            if ( GUCEF_NULL != exporter )
            {
                ASSERT_TRUE( exporter->hasRequestBodyShape );
                ASSERT_TRUE( JoinedContains( exporter->requestBodyShape.lines, "format: string" ) );
                ASSERT_TRUE( JoinedContains( exporter->requestBodyShape.lines, "password: string" ) );
                ASSERT_TRUE( JoinedContains( exporter->requestBodyShape.lines, "range.from: string" ) );
                ASSERT_TRUE( exporter->downloadCount == 1 && exporter->downloadFileNames.count( "Orders_{n}.csv" ) == 1 );
                ASSERT_TRUE( exporter->requestContentTypes.count( "application/json" ) == 1 );
                ASSERT_TRUE( exporter->requestHeaderNames.count( "x-csrf-token" ) == 1 );
            }

            // the data request: its paging and shape, from the response the browser kept
            const WEB::CDiscoveredEndpoint* orders = catalog.FindEntry( "GET", "http://shop.example.test/api/orders" );
            ASSERT_TRUE( GUCEF_NULL != orders );
            if ( GUCEF_NULL != orders )
            {
                ASSERT_TRUE( orders->hasShape );
                ASSERT_TRUE( JoinedContains( orders->shape.lines, "items[].price: number" ) );
                ASSERT_TRUE( JoinedContains( orders->shape.paginationHints, "total" ) && JoinedContains( orders->shape.paginationHints, "next" ) );
                ASSERT_TRUE( orders->queryParams.size() == 3 );
                ASSERT_TRUE( orders->bodyFiles.empty() );      // the report names saved bodies only with example values
            }

            // the sign-in: only that it happened
            const WEB::CDiscoveredEndpoint* signIn = catalog.FindEntry( "POST", "http://shop.example.test/login" );
            ASSERT_TRUE( GUCEF_NULL != signIn && signIn->neverRecorded && signIn->requestHeaderNames.empty() && !signIn->hasRequestBodyShape );

            // the body the browser kept
            const WEB::CDiscoveredEndpoint* search = catalog.FindEntry( "POST", "http://shop.example.test/api/search" );
            ASSERT_TRUE( GUCEF_NULL != search && search->hasRequestBodyShape && JoinedContains( search->requestBodyShape.lines, "query: string" ) );

            // the actions: the navigation and the click, with the export behind the click
            ASSERT_TRUE( recorder.GetActionCount() >= 2 );
            bool clickWithExport = false;
            bool navigation = false;
            for ( size_t a=0; a<catalog.GetActions().size(); ++a )
            {
                const WEB::CSiteEndpointCatalog::SActionRecord& record = catalog.GetActions()[ a ];
                if ( record.action.kind == WEB::CObservedUserAction::KindNavigation )
                    navigation = true;
                if ( record.action.kind == WEB::CObservedUserAction::KindClick && record.action.label == "Export {n} to CSV" &&
                     !record.steps.empty() && Contains( record.steps[ 0 ], "/api/orders/{n}/export (download)" ) )
                    clickWithExport = true;
            }
            ASSERT_TRUE( navigation && clickWithExport );

            // saved bodies are owner-only
            CORE::CString bodiesDir = CORE::CombinePath( dir.GetPath(), "bodies" );
            ASSERT_TRUE( recorder.GetSavedBodyCount() == 1 );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0700 == ModeOf( bodiesDir ) );
            ASSERT_TRUE( 0600 == ModeOf( journalPath ) );
            CORE::CString bodyFile;
            UInt32 sequence = 0;
            for ( UInt32 s=1; s<10 && bodyFile.IsNULLOrEmpty(); ++s )
            {
                char name[ 32 ];
                snprintf( name, sizeof( name ), "%06u.json", (unsigned int) s );
                if ( CORE::FileExists( CORE::CombinePath( bodiesDir, name ) ) )
                {
                    bodyFile = CORE::CombinePath( bodiesDir, name );
                    sequence = s;
                }
            }
            ASSERT_TRUE( !bodyFile.IsNULLOrEmpty() && 0600 == ModeOf( bodyFile ) );
            #endif

            recorder.Stop();
            journal.Close();

            // no planted secret and no data value in what was written
            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( journalText ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( report ) );
            ASSERT_FALSE( Contains( journalText, "Private Person Name" ) || Contains( report, "Private Person Name" ) );
            ASSERT_FALSE( Contains( journalText, "12345678" ) || Contains( report, "12345678" ) );
            ASSERT_FALSE( Contains( report, "tracker.other.test" ) || Contains( journalText, "tracker.other.test" ) );
            ASSERT_TRUE( redactor.GetKnownSecretCount() >= 9 );
            ASSERT_TRUE( Contains( journalText, "\"label\":\"Export 2026 to CSV\"" ) );     // as observed; the report applies the value rules
            ASSERT_TRUE( Contains( report, "### POST shop.example.test/api/orders/{n}/export" ) );
            ASSERT_TRUE( Contains( report, "Export {n} to CSV" ) );

            // the report rebuilt from the journal is the same report
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &redactor );
            UInt32 next = 0;
            ASSERT_TRUE( journal.Load( journalPath, rebuilt, &next ) == 5 );
            ASSERT_TRUE( next == recorder.GetNextSequence() );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
            ASSERT_TRUE( rebuiltReport == report );
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Site discovery report of the fake browsing session:\n" << report << "\nJournal:\n" << journalText );
            if ( rebuiltReport != report )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Live report:\n" << report << "\nRebuilt report:\n" << rebuiltReport );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With example values on, the planted secrets still stay out of journal and report" )
        try
        {
            ASSERT_TRUE( started );
            browser.ForgetConnection();
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Examples" );

            WEB::CChromeDevToolsClient client;
            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) );
            ASSERT_TRUE( client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.captureClicks = true;
            settings.recordExampleValues = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &journal );
            recorder.AddSink( &catalog );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );
            PlayBrowsingSession( browser, recorder, "S-P1" );
            ASSERT_TRUE( recorder.GetRecordedRequestCount() == 5 );
            recorder.Stop();
            journal.Close();

            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( journalText ) );
            ASSERT_FALSE( ContainsAnyPlantedSecret( report ) );

            // the examples are there, with the secrets in them replaced
            ASSERT_TRUE( Contains( report, "`page` e.g. `1`" ) );
            ASSERT_TRUE( Contains( report, "access_token={secret:" ) );
            ASSERT_TRUE( Contains( report, "request body example" ) && Contains( report, "\"format\":\"csv\"" ) );
            ASSERT_TRUE( Contains( report, "total: integer = 120" ) );
            ASSERT_FALSE( Contains( report, "Private Person Name" ) );
            ASSERT_TRUE( Contains( journalText, "sessionToken={secret:" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The recorder keeps later response shapes, enum query values, form field names, cookie and redirect names and notes payment traffic only" )
        try
        {
            ASSERT_TRUE( started );
            browser.ForgetConnection();
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_GapSession" );

            WEB::CChromeDevToolsClient client;
            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) );
            ASSERT_TRUE( client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.hostFilter.push_back( "files.example.test" );
            settings.hostFilter.push_back( "payment-website-pci.example.test" );
            settings.recordEnumQueryValues = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &journal );
            recorder.AddSink( &catalog );
            recorder.SetNextSequence( journal.GetNextSequence() );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );

            const CORE::CString session = "S-P1";
            CFakeDevToolsBrowser::TStringMap none;
            CFakeDevToolsBrowser::TStringMap jsonHeaders;
            jsonHeaders[ "content-type" ] = "application/json";

            // the library search: an empty list first, items later, a new type and field after that, a repeat
            const char* const bodies[] = { "{\"results\":[],\"next\":null,\"total\":0}",
                                           "{\"results\":[{\"title\":\"First\",\"price\":null,\"seller\":{\"displayName\":\"x\"}}],\"next\":\"c1\",\"total\":3}",
                                           "{\"results\":[{\"title\":\"Second\",\"price\":null,\"seller\":{\"displayName\":\"y\"}}],\"next\":\"c2\",\"total\":3}",
                                           "{\"results\":[{\"title\":\"Third\",\"price\":\"9.50\",\"rating\":4.5,\"seller\":{\"displayName\":\"z\"}}],\"next\":\"c3\",\"total\":3}", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != bodies[ i ]; ++i )
            {
                const CORE::CString id = "L" + CORE::UInt32ToString( i );
                browser.SetResponseBody( id, bodies[ i ] );
                CFakeDevToolsBrowser::TStringMap headers;
                headers[ "Cookie" ] = "locale=en; gapsession=PLANTEDgapCOOKIEvalue11";
                browser.EmitRequest( session, id, "GET", "http://shop.example.test/i/library/search?count=24&sort_by=-created&source=web_library&listing_types=template&cursor=PLANTEDcursorABCD1234xyz&token=PLANTEDgapTOKENvalue14", "Fetch", "P1", headers, "", false, 200.0 + i );
                if ( 0 == i )
                {
                    // what the browser really sent and received, which comes as extra info
                    CORE::CDataNode extra( GUCEF_DATATYPE_OBJECT );
                    extra.SetAttribute( "requestId", CORE::CVariant( id ) );
                    CORE::CDataNode* extraHeaders = extra.AddChild( "headers", GUCEF_DATATYPE_OBJECT );
                    extraHeaders->SetAttribute( "Cookie", CORE::CVariant( CORE::CString( "locale=en; extracookie=PLANTEDgapEXTRAvalue12" ) ) );
                    extraHeaders->SetAttribute( "Authorization", CORE::CVariant( CORE::CString( "Bearer PLANTEDgapBEARERtoken13" ) ) );
                    extraHeaders->SetAttribute( "sec-fetch-mode", CORE::CVariant( CORE::CString( "cors" ) ) );
                    browser.Emit( session, "Network.requestWillBeSentExtraInfo", extra );
                    CORE::CDataNode responseExtra( GUCEF_DATATYPE_OBJECT );
                    responseExtra.SetAttribute( "requestId", CORE::CVariant( id ) );
                    CORE::CDataNode* responseExtraHeaders = responseExtra.AddChild( "headers", GUCEF_DATATYPE_OBJECT );
                    responseExtraHeaders->SetAttribute( "Set-Cookie", CORE::CVariant( CORE::CString( "newcookie=PLANTEDgapSETvalue15; Path=/; HttpOnly\nothercookie=abc" ) ) );
                    browser.Emit( session, "Network.responseReceivedExtraInfo", responseExtra );
                }
                browser.EmitResponse( session, id, 200, "application/json", jsonHeaders, "Fetch" );
                browser.EmitFinished( session, id, 300 );
                for ( UInt32 p=0; p<2000 && recorder.GetRecordedRequestCount() < i + 1; ++p )
                    Pump( 1, &recorder );
            }

            // a multipart upload
            CFakeDevToolsBrowser::TStringMap multipartHeaders;
            multipartHeaders[ "Content-Type" ] = "multipart/form-data; boundary=XYZ";
            browser.EmitRequest( session, "M1", "POST", "http://shop.example.test/api/upload", "Fetch", "P1", multipartHeaders,
                                 "--XYZ\r\nContent-Disposition: form-data; name=\"title\"\r\n\r\nMy Private Title PLANTEDmultipartVALUE16\r\n"
                                 "--XYZ\r\nContent-Disposition: form-data; name=\"csrfmiddlewaretoken\"\r\n\r\nPLANTEDmultiCSRFvalue17\r\n"
                                 "--XYZ\r\nContent-Disposition: form-data; name=\"password\"\r\n\r\nPLANTEDmultiPASSWORDv18\r\n"
                                 "--XYZ\r\nContent-Disposition: form-data; name=\"file\"; filename=\"private-name.png\"\r\nContent-Type: image/png\r\n\r\nBINARYDATAxxxx\r\n--XYZ--\r\n",
                                 false, 210.0 );
            browser.EmitResponse( session, "M1", 204, "", none, "Fetch" );
            browser.EmitFinished( session, "M1", 0 );

            // a download behind a redirect to another host
            browser.EmitRequest( session, "D1", "GET", "http://shop.example.test/api/assets/12345/download", "Fetch", "P1", none, "", false, 220.0 );
            CORE::CDataNode leg( GUCEF_DATATYPE_OBJECT );
            leg.SetAttribute( "requestId", CORE::CVariant( CORE::CString( "D1" ) ) );
            leg.SetAttribute( "frameId", CORE::CVariant( CORE::CString( "P1" ) ) );
            leg.SetAttribute( "type", CORE::CVariant( CORE::CString( "Fetch" ) ) );
            leg.SetAttribute( "timestamp", CORE::CVariant( (Float64) 221.0 ) );
            CORE::CDataNode* legRequest = leg.AddChild( "request", GUCEF_DATATYPE_OBJECT );
            legRequest->SetAttribute( "url", CORE::CVariant( CORE::CString( "http://files.example.test/blobs/abcdef0123456789abcdef/asset.zip?sig=PLANTEDgapSIGvalue19" ) ) );
            legRequest->SetAttribute( "method", CORE::CVariant( CORE::CString( "GET" ) ) );
            legRequest->AddChild( "headers", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* redirect = leg.AddChild( "redirectResponse", GUCEF_DATATYPE_OBJECT );
            redirect->SetAttribute( "url", CORE::CVariant( CORE::CString( "http://shop.example.test/api/assets/12345/download" ) ) );
            redirect->SetAttribute( "status", CORE::CVariant( (Int32) 302 ) );
            redirect->SetAttribute( "mimeType", CORE::CVariant( CORE::CString( "" ) ) );
            CORE::CDataNode* redirectHeaders = redirect->AddChild( "headers", GUCEF_DATATYPE_OBJECT );
            redirectHeaders->SetAttribute( "Location", CORE::CVariant( CORE::CString( "http://files.example.test/blobs/abcdef0123456789abcdef/asset.zip?sig=PLANTEDgapSIGvalue19" ) ) );
            redirectHeaders->SetAttribute( "Set-Cookie", CORE::CVariant( CORE::CString( "dlcookie=PLANTEDgapDLvalue20; Path=/" ) ) );
            browser.Emit( session, "Network.requestWillBeSent", leg );
            CFakeDevToolsBrowser::TStringMap zipHeaders;
            zipHeaders[ "Content-Type" ] = "application/zip";
            zipHeaders[ "Content-Disposition" ] = "attachment; filename=\"asset_123456.zip\"";
            browser.EmitResponse( session, "D1", 200, "application/zip", zipHeaders, "Fetch" );
            browser.EmitFinished( session, "D1", 4096 );

            // checkout and payment traffic: noted, never recorded
            CFakeDevToolsBrowser::TStringMap payHeaders;
            payHeaders[ "Content-Type" ] = "application/json";
            payHeaders[ "Cookie" ] = "paycookie=PLANTEDgapPAYvalue21";
            browser.EmitRequest( session, "C1", "POST", "http://payment-website-pci.example.test/v2/purchase/550e8400-e29b-41d4-a716-446655440000/confirm?step=2", "Fetch", "P1", payHeaders,
                                 "{\"cardNumber\":\"4111111111111111\",\"cardholder\":\"Private Person\"}", false, 230.0 );
            browser.SetResponseBody( "C1", "{\"orderStatus\":\"confirmedOrder\"}" );
            browser.EmitResponse( session, "C1", 200, "application/json", jsonHeaders, "Fetch" );
            browser.EmitFinished( session, "C1", 100 );
            browser.EmitRequest( session, "C2", "GET", "http://shop.example.test/api/checkout/cart", "Fetch", "P1", none, "", false, 231.0 );
            browser.SetResponseBody( "C2", "{\"cartTotal\":12.5}" );
            browser.EmitResponse( session, "C2", 200, "application/json", jsonHeaders, "Fetch" );
            browser.EmitFinished( session, "C2", 100 );

            for ( UInt32 p=0; p<3000 && recorder.GetRecordedRequestCount() < 9; ++p )
                Pump( 1, &recorder );
            Pump( 30, &recorder );
            ASSERT_TRUE( recorder.GetRecordedRequestCount() == 9 );     // 4 searches, the upload, both legs of the download, the two payment requests
            recorder.Stop();
            journal.Close();

            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Site discovery report of the gap session:\n" << report << "\nJournal:\n" << journalText );

            // 1. the shapes: the first (empty list), the items, the new type and field were kept; the repeat was not
            const WEB::CDiscoveredEndpoint* search = catalog.FindEntry( "GET", "http://shop.example.test/i/library/search" );
            ASSERT_TRUE( GUCEF_NULL != search && 4 == search->requestCount && 3 == search->shape.mergedCount );
            UInt32 shapeLines = 0;
            for ( const char* p = journalText.C_String(); GUCEF_NULL != ( p = strstr( p, "\"responseShape\"" ) ); ++p )
                ++shapeLines;
            ASSERT_TRUE( 3 == shapeLines );
            ASSERT_TRUE( Contains( report, "response shape (union of 3 responses)" ) );
            ASSERT_TRUE( Contains( report, "results[].title: string" ) && Contains( report, "results[].seller.displayName: string" ) );
            ASSERT_TRUE( Contains( report, "results[].price: null | string" ) );
            ASSERT_TRUE( Contains( report, "results[].rating: number (optional)" ) );
            ASSERT_FALSE( Contains( report, "always empty" ) || Contains( report, "array[0]" ) );
            ASSERT_FALSE( Contains( report, "First" ) || Contains( report, "Second" ) || Contains( report, "Third" ) );

            // 2. enum-like query values, never the token-like or secret ones
            ASSERT_TRUE( Contains( report, "`count` values: `24` (4)" ) && Contains( report, "`sort_by` values: `-created` (4)" ) );
            ASSERT_TRUE( Contains( report, "`source` values: `web_library` (4)" ) && Contains( report, "`listing_types` values: `template` (4)" ) );
            ASSERT_FALSE( Contains( report, "`cursor` values" ) || Contains( report, "`token` values" ) );

            // 3. the multipart body: names, kinds and sizes
            const WEB::CDiscoveredEndpoint* upload = catalog.FindEntry( "POST", "http://shop.example.test/api/upload" );
            ASSERT_TRUE( GUCEF_NULL != upload && upload->hasRequestBodyShape );
            if ( GUCEF_NULL != upload )
            {
                ASSERT_TRUE( JoinedContains( upload->requestBodyShape.lines, "multipart/form-data" ) );
                ASSERT_TRUE( JoinedContains( upload->requestBodyShape.lines, "title: text, " ) );
                ASSERT_TRUE( JoinedContains( upload->requestBodyShape.lines, "csrfmiddlewaretoken: csrf-like, value withheld" ) );
                ASSERT_TRUE( JoinedContains( upload->requestBodyShape.lines, "password: text, value withheld" ) );
                ASSERT_TRUE( JoinedContains( upload->requestBodyShape.lines, "file: file, image/png, 14 bytes" ) );
            }

            // 4. cookie and header names per host, redirects and the chain of the download
            ASSERT_TRUE( Contains( report, "- `shop.example.test`: request headers: " ) );
            ASSERT_TRUE( Contains( report, "cookies sent: " ) && Contains( report, "gapsession" ) && Contains( report, "extracookie" ) && Contains( report, "locale" ) );
            ASSERT_TRUE( Contains( report, "cookies set: " ) && Contains( report, "newcookie" ) && Contains( report, "othercookie" ) && Contains( report, "dlcookie" ) );
            ASSERT_TRUE( Contains( report, "sec-fetch-mode" ) );
            ASSERT_TRUE( Contains( report, "- redirects: 302 -> files.example.test/blobs/{hex}/asset.zip" ) );
            ASSERT_TRUE( Contains( report, "- download redirect chain (hosts): shop.example.test > files.example.test" ) );
            ASSERT_TRUE( Contains( journalText, "\"cookiesSent\":[" ) && Contains( journalText, "\"redirectTo\":\"files.example.test/blobs/{hex}/asset.zip\"" ) );

            // 5. payment traffic: noted with host and count, nothing else
            const WEB::CDiscoveredEndpoint* pay = catalog.FindEntry( "POST", "http://payment-website-pci.example.test/v2/purchase/550e8400-e29b-41d4-a716-446655440000/confirm" );
            ASSERT_TRUE( GUCEF_NULL != pay && pay->neverRecorded && !pay->hasRequestBodyShape && !pay->hasShape && pay->requestHeaderNames.empty() && pay->queryParams.empty() );
            const WEB::CDiscoveredEndpoint* cart = catalog.FindEntry( "GET", "http://shop.example.test/api/checkout/cart" );
            ASSERT_TRUE( GUCEF_NULL != cart && cart->neverRecorded && !cart->hasShape );
            ASSERT_TRUE( Contains( report, "| payment-website-pci.example.test | 1 |" ) );
            ASSERT_FALSE( Contains( report, "paycookie" ) || Contains( journalText, "paycookie" ) );
            ASSERT_FALSE( Contains( report + journalText, "cardNumber" ) || Contains( report + journalText, "4111111111111111" ) || Contains( report + journalText, "orderStatus" ) );
            ASSERT_FALSE( Contains( report + journalText, "cartTotal" ) || Contains( report + journalText, "step" ) );

            // no planted value anywhere
            static const char* const planted[] = { "PLANTEDgapCOOKIEvalue11", "PLANTEDgapEXTRAvalue12", "PLANTEDgapBEARERtoken13", "PLANTEDgapTOKENvalue14", "PLANTEDgapSETvalue15",
                                                   "PLANTEDmultipartVALUE16", "PLANTEDmultiCSRFvalue17", "PLANTEDmultiPASSWORDv18", "PLANTEDgapSIGvalue19", "PLANTEDgapDLvalue20",
                                                   "PLANTEDgapPAYvalue21", "PLANTEDcursorABCD1234xyz", "Private Title", "private-name", "BINARYDATA", GUCEF_NULL };
            for ( UInt32 i=0; GUCEF_NULL != planted[ i ]; ++i )
                ASSERT_FALSE( Contains( report, planted[ i ] ) || Contains( journalText, planted[ i ] ) );

            // the report rebuilt from the journal is the same report
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Load( journalPath, rebuilt ) == 9 );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
            ASSERT_TRUE( rebuiltReport == report );
            if ( rebuiltReport != report )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Live report:\n" << report << "\nRebuilt report:\n" << rebuiltReport );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A session attaches to a running browser, records and writes an owner-only report" )
        try
        {
            ASSERT_TRUE( started );
            browser.ForgetConnection();
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Session" );

            WEB::CSiteDiscoverySettings settings;
            settings.outputDirectory = CORE::CombinePath( dir.GetPath(), "out" );
            settings.hostFilter.push_back( "shop.example.test" );
            settings.browserRunMode = WEB::CSiteDiscoverySettings::BROWSER_RUNMODE_ATTACH;
            settings.browserDebugPort = browser.GetPort();
            settings.browserUserDataDirs.push_back( CORE::CombinePath( dir.GetPath(), "no-profile" ) );     // never the user's own browser
            settings.browserConnectTimeoutMs = 10000;

            WEB::CSiteDiscoverySession session;
            session.SetSettings( settings );
            session.SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            CORE::CString problem;
            ASSERT_TRUE( session.Open( problem ) );
            ASSERT_TRUE( session.IsOpen() && session.IsBrowserConnected() );
            ASSERT_FALSE( session.GetBrowser().WasLaunched() );
            for ( UInt32 i=0; i<50; ++i )
                session.Process();

            CFakeDevToolsBrowser::TStringMap none;
            CFakeDevToolsBrowser::TStringMap jsonHeaders;
            jsonHeaders[ "content-type" ] = "application/json";
            browser.SetResponseBody( "Q1", "{\"results\":[{\"sku\":\"A1\"}],\"cursor\":\"abc\"}" );
            browser.EmitRequest( "S-P1", "Q1", "GET", "http://shop.example.test/api/catalog/search?q=lamp", "Fetch", "P1", none, "", false, 200.0 );
            browser.EmitResponse( "S-P1", "Q1", 200, "application/json", jsonHeaders, "Fetch" );
            browser.EmitFinished( "S-P1", "Q1", 300 );
            for ( UInt32 i=0; i<2000 && session.GetCatalog().GetRequestCount() < 1; ++i )
            {
                session.Process();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( session.GetCatalog().GetRequestCount() == 1 );
            ASSERT_TRUE( session.Close( problem ) );
            ASSERT_FALSE( session.IsOpen() );

            CORE::CString report;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetReportPath(), report ) );
            ASSERT_TRUE( Contains( report, "### GET shop.example.test/api/catalog/search" ) );
            ASSERT_TRUE( Contains( report, "  - `cursor` (paging)" ) || Contains( report, "  - `cursor`" ) );
            ASSERT_FALSE( Contains( report, "lamp" ) || Contains( report, "abc" ) );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == ModeOf( session.GetReportPath() ) );
            ASSERT_TRUE( 0600 == ModeOf( session.GetJournalPath() ) );
            ASSERT_TRUE( 0700 == ModeOf( settings.outputDirectory ) );
            #endif

            // a second run goes on with the journal of the first: the report covers both
            WEB::CSiteDiscoverySession second;
            second.SetSettings( settings );
            second.SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            browser.ForgetConnection();
            ASSERT_TRUE( second.Open( problem ) );
            ASSERT_TRUE( second.GetCatalog().GetRequestCount() == 1 );
            ASSERT_TRUE( second.Close( problem ) );

            // attaching to a port where no browser is does not start one
            WEB::CSiteDiscoverySettings nobody = settings;
            nobody.browserDebugPort = TESTFW::FindFreeTcpPort();
            WEB::CSiteDiscoverySession refused;
            refused.SetSettings( nobody );
            refused.SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            ASSERT_FALSE( refused.Open( problem ) );
            ASSERT_FALSE( refused.GetBrowser().WasLaunched() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A storefront session keeps every journal line; public values from secret looking fields are not looked for, real secrets are" )
        try
        {
            ASSERT_TRUE( started );
            browser.ForgetConnection();
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Storefront" );

            WEB::CChromeDevToolsClient client;
            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", browser.GetPort(), discovery ) );
            ASSERT_TRUE( client.ConnectAndWait( discovery.browser.webSocketDebuggerUrl ) );

            WEB::CSiteDiscoverySettings settings;
            settings.hostFilter.push_back( "shop.example.test" );
            settings.recordExampleValues = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            WEB::CChromeNetworkRecorder recorder( client );
            recorder.SetRedactor( &redactor );
            recorder.AddSink( &journal );
            recorder.AddSink( &catalog );
            recorder.SetNextSequence( journal.GetNextSequence() );
            ASSERT_TRUE( recorder.Start() );
            Pump( 50, &recorder );

            // every request of the store sends the session cookie and an analytics header
            const CORE::CString session = "S-P1";
            const char* const sessionCookie = "Sess9fK2mQ7xL4pR8tV3wZ6yB1nC5dH0";
            const char* const analyticsId = "Sh0pAn4lyt1csId77";
            const char* const publicKey = "pk_live_PUBLICw1dg3tKEY4567";
            const char* const checkoutToken = "c1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6";
            const char* const realToken = "Rk9vYmFyVG9rZW4xMjM0NTY3ODkwQWJj";
            const char* const prefixValue = "AbCdEfGhIjKlMnOpQrStUvWx12";
            CFakeDevToolsBrowser::TStringMap headers;
            headers[ "Cookie" ] = CORE::CString( "_shopify_y=ab12; _secure_session_id=" ) + sessionCookie;
            headers[ "X-Shop-Analytics" ] = analyticsId;
            CFakeDevToolsBrowser::TStringMap jsonHeaders;
            jsonHeaders[ "content-type" ] = "application/json";
            CFakeDevToolsBrowser::TStringMap htmlHeaders;
            htmlHeaders[ "content-type" ] = "text/html";

            // the storefront configuration: values of fields with secret looking names, most of them public
            CORE::CString config = "{\"token\":\"shop.example.test\",\"auth_url\":\"https://shop.example.test/collections/all-kits?from=nav\","
                                   "\"api_key\":\"" + CORE::CString( publicKey ) + "\",\"session\":{\"path\":\"/collections/all-kits\",\"domain\":\".example.test\","
                                   "\"shop\":\"shop-example.myshopify.com\",\"hint\":\"" + analyticsId + "\"},\"csrf\":\"collections-and-kits\","
                                   "\"checkout_token\":\"" + checkoutToken + "\",\"secret_prefix\":\"" + prefixValue + "\","
                                   "\"auth_token\":\"" + realToken + "\"}";
            browser.SetResponseBody( "C1", config );
            browser.EmitRequest( session, "D1", "GET", "https://shop.example.test/collections/all-kits", "Document", "P1", headers, "", false, 200.0 );
            browser.EmitResponse( session, "D1", 200, "text/html", htmlHeaders, "Document" );
            browser.EmitFinished( session, "D1", 5000 );
            Pump( 20, &recorder );
            browser.EmitRequest( session, "C1", "GET", "https://shop.example.test/api/storefront/config.json", "Fetch", "P1", headers, "", false, 200.5 );
            browser.EmitResponse( session, "C1", 200, "application/json", jsonHeaders, "Fetch" );
            browser.EmitFinished( session, "C1", 600 );
            for ( UInt32 i=0; i<2000 && recorder.GetRecordedRequestCount() < 2; ++i )
                Pump( 1, &recorder );
            Pump( 20, &recorder );

            // the public key in tracking requests, the checkout token in a path, the cookie name in a query, a payload that starts like a registered value
            browser.EmitRequest( session, "T1", "GET", "https://tracker.widget.test/collect?k=" + CORE::CString( publicKey ), "Image", "P1", headers, "", false, 201.0 );
            browser.EmitResponse( session, "T1", 200, "image/gif", htmlHeaders, "Image" );
            browser.EmitFinished( session, "T1", 40 );
            const char* const paths[] = { "/collections/all-kits/products/neo-city", "/apps/widget/load", "/cart/update", "/search/suggest", "/api/events", "/api/payload" };
            for ( UInt32 i=0; i<6; ++i )
            {
                CORE::CString url = CORE::CString( "https://shop.example.test" ) + paths[ i ];
                if ( 1 == i )
                    url += "?k=" + CORE::CString( publicKey );
                if ( 2 == i )
                    url = "https://shop.example.test/cart/" + CORE::CString( checkoutToken ) + "/update";
                if ( 3 == i )
                    url += "?from=_secure_session_id";
                if ( 5 == i )
                    url += "?blob=" + CORE::CString( prefixValue ) + "345xyzPAYLOAD";
                const CORE::CString id = "Q" + CORE::UInt32ToString( i );
                browser.EmitRequest( session, id, "GET", url, "XHR", "P1", headers, "", false, 202.0 + i );
                browser.EmitResponse( session, id, 204, "", htmlHeaders, "XHR" );
                browser.EmitFinished( session, id, 0 );
            }
            // a download whose content type carries the real token: a place where a report cannot mask it in place
            CFakeDevToolsBrowser::TStringMap fileHeaders;
            fileHeaders[ "content-disposition" ] = "attachment; filename=\"Kit.zip\"";
            browser.EmitRequest( session, "Z1", "GET", "https://shop.example.test/downloads/kit", "Document", "P1", headers, "", false, 209.0 );
            browser.EmitResponse( session, "Z1", 200, "application/x-kit-" + CORE::CString( realToken ), fileHeaders, "Document" );
            browser.EmitFinished( session, "Z1", 1000 );

            // the real token goes back to the site in a body field whose name is not secret
            CFakeDevToolsBrowser::TStringMap postHeaders( headers );
            postHeaders[ "Content-Type" ] = "application/json";
            browser.EmitRequest( session, "N1", "POST", "https://shop.example.test/api/notes", "Fetch", "P1", postHeaders,
                                 "{\"note\":\"" + CORE::CString( realToken ) + "\"}", true, 210.0 );
            browser.EmitResponse( session, "N1", 204, "", htmlHeaders, "Fetch" );
            browser.EmitFinished( session, "N1", 0 );
            for ( UInt32 i=0; i<3000 && recorder.GetRecordedRequestCount() < 10; ++i )
                Pump( 1, &recorder );
            Pump( 30, &recorder );
            ASSERT_TRUE( recorder.GetRecordedRequestCount() == 10 );
            recorder.Stop();
            journal.Close();

            // the journal: one complete line per request, only the cookie value masked
            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            CORE::CString::StringVector lines = journalText.ParseElements( '\n', false );
            UInt32 requestLines = 0;
            for ( size_t i=0; i<lines.size(); ++i )
                if ( Contains( lines[ i ], "{\"seq\":" ) )
                    ++requestLines;
            ASSERT_TRUE( 10 == requestLines );
            ASSERT_FALSE( Contains( journalText, "\"redacted\"" ) );
            ASSERT_FALSE( Contains( journalText, sessionCookie ) );
            ASSERT_TRUE( Contains( journalText, "https://shop.example.test/collections/all-kits" ) );
            ASSERT_TRUE( Contains( journalText, ( "/cart/" + CORE::CString( checkoutToken ) + "/update" ).C_String() ) );
            ASSERT_TRUE( Contains( journalText, realToken ) );             // the journal is the user's record; the report decides

            // the report: hosts and public values shown, the real token left out with its description, the analytics id suspected public
            ASSERT_TRUE( 1 <= redactor.AssessJournal( journalText ) );
            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( Contains( report, "| shop.example.test |" ) );
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Storefront report:\n" << report );
            ASSERT_TRUE( Contains( report, "/collections/all-kits" ) && Contains( report, "/apps/widget/load" ) && Contains( report, "/search/{value}" ) );
            ASSERT_TRUE( Contains( report, "/api/payload" ) && Contains( report, "/cart/" ) && Contains( report, "/api/notes" ) );
            ASSERT_TRUE( Contains( report, "{secret}" ) );              // the token in the body example is masked in place
            ASSERT_TRUE( 1 == catalog.GetRedactedEntryCount() );       // the download whose content type holds it is left out
            ASSERT_TRUE( Contains( report, "class response-body-field, field `auth_token`, length 32" ) );
            ASSERT_TRUE( Contains( report, "Host `shop.example.test`, method GET" ) );
            ASSERT_FALSE( Contains( report, realToken ) || Contains( report, sessionCookie ) );
            ASSERT_TRUE( Contains( report, "taken for public values" ) && Contains( report, "field `session`" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static WEB::CObservedUserAction
CObservedActionFor( UInt32 id, const char* kind, const char* label, const char* page )
{
    WEB::CObservedUserAction action;
    action.id = id;
    action.timestamp = 50.0 + id;
    action.kind = kind;
    action.label = label;
    action.page = page;
    return action;
}

/*-------------------------------------------------------------------------*/

static WEB::CWebBodyShape
DescribeJson( const char* json )
{
    WEB::CWebBodyShape shape;
    CORE::CDataNode document;
    if ( COM::CJsonRpcEndpoint::ParseJsonText( json, document ) )
        WEB::CWebBodyShape::Describe( document, shape, 5, 150, false );
    return shape;
}

/*-------------------------------------------------------------------------*/

static void
PerformDiscoveryGapTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Response shapes of one endpoint merge: empty list first, items later, optional fields, null and typed variants" )
        try
        {
            const WEB::CWebBodyShape empty = DescribeJson( "{\"results\":[],\"next\":null,\"total\":0}" );
            const WEB::CWebBodyShape items = DescribeJson( "{\"results\":[{\"title\":\"a\",\"price\":null,\"seller\":{\"displayName\":\"x\"}}],\"next\":\"c1\",\"total\":3}" );
            const WEB::CWebBodyShape typed = DescribeJson( "{\"results\":[{\"title\":\"b\",\"price\":\"9.50\",\"rating\":4.5,\"seller\":{\"displayName\":\"y\"}}],\"next\":\"c2\",\"total\":3}" );
            ASSERT_TRUE( !empty.IsEmpty() && !items.IsEmpty() && !typed.IsEmpty() );

            WEB::CWebBodyShape merged;
            ASSERT_TRUE( empty.AddsToMerged( merged ) );
            merged.Merge( empty );
            ASSERT_TRUE( JoinedContains( merged.lines, "results: array (always empty so far)" ) );
            ASSERT_FALSE( JoinedContains( merged.lines, "array[" ) );
            ASSERT_FALSE( empty.AddsToMerged( merged ) );

            // an array that was empty before and has elements now is new
            ASSERT_TRUE( items.AddsToMerged( merged ) );
            merged.Merge( items );
            ASSERT_FALSE( items.AddsToMerged( merged ) );
            ASSERT_TRUE( JoinedContains( merged.lines, "results: array" ) && !JoinedContains( merged.lines, "always empty" ) );
            ASSERT_TRUE( JoinedContains( merged.lines, "results[].title: string" ) );
            ASSERT_TRUE( JoinedContains( merged.lines, "results[].seller.displayName: string" ) );
            ASSERT_TRUE( JoinedContains( merged.lines, "next: null | string" ) );

            // a type that was not seen for a path, and a path that was not seen at all, are new
            ASSERT_TRUE( typed.AddsToMerged( merged ) );
            merged.Merge( typed );
            ASSERT_TRUE( 3 == merged.mergedCount );
            ASSERT_TRUE( JoinedContains( merged.lines, "results[].price: null | string" ) );
            ASSERT_TRUE( JoinedContains( merged.lines, "results[].rating: number (optional)" ) );
            ASSERT_FALSE( JoinedContains( merged.lines, "results[].title: string (optional)" ) );
            ASSERT_FALSE( JoinedContains( merged.lines, "total: integer (optional)" ) );
            ASSERT_TRUE( JoinedContains( merged.paginationHints, "next" ) );
            ASSERT_FALSE( typed.AddsToMerged( merged ) );

            // a union that would be larger than its limit is cut and says so
            WEB::CWebBodyShape limited;
            limited.Merge( items, 3 );
            limited.Merge( typed, 3 );
            ASSERT_TRUE( limited.truncated );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The catalog keeps the union of the shapes of an endpoint, up to maxShapesPerEndpoint, and the journal gives the same report back" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_ShapeUnion" );
            WEB::CSiteDiscoverySettings settings;
            settings.maxShapesPerEndpoint = 3;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );

            const char* const bodies[] = { "{\"results\":[],\"next\":null}",
                                           "{\"results\":[{\"title\":\"a\"}],\"next\":\"c\"}",
                                           "{\"results\":[{\"title\":\"b\"}],\"next\":\"d\"}",
                                           "{\"results\":[{\"title\":\"c\",\"rating\":5}],\"next\":\"e\"}",
                                           "{\"results\":[{\"title\":\"d\",\"rating\":5,\"extra\":true}],\"next\":\"f\"}", GUCEF_NULL };
            UInt32 shaped = 0;
            for ( UInt32 i=0; GUCEF_NULL != bodies[ i ]; ++i )
            {
                WEB::CObservedWebRequest o = MakeObservation( i + 1, "GET", "https://shop.example/i/library/search?count=24", "Fetch", 200, "application/json", 100 );
                const WEB::CWebBodyShape shape = DescribeJson( bodies[ i ] );
                if ( catalog.WantsResponseShape( o ) && catalog.IsNewResponseShape( o, shape ) )
                {
                    o.responseShape = shape;
                    o.hasResponseShape = true;
                    ++shaped;
                }
                catalog.OnWebRequestObserved( o );
                journal.OnWebRequestObserved( o );
            }
            journal.Close();

            // the empty list, the first items and the response with a new field were kept; the repeat was not; the cap stops the rest
            ASSERT_TRUE( 3 == shaped );
            const WEB::CDiscoveredEndpoint* search = catalog.FindEntry( "GET", "https://shop.example/i/library/search" );
            ASSERT_TRUE( GUCEF_NULL != search && 5 == search->requestCount && 3 == search->shape.mergedCount );
            ASSERT_TRUE( GUCEF_NULL != search && !search->shape.lines.empty() && JoinedContains( search->shape.lines, "results[].title: string" ) );
            ASSERT_TRUE( GUCEF_NULL != search && JoinedContains( search->shape.lines, "results[].rating: integer (optional)" ) );
            ASSERT_FALSE( GUCEF_NULL != search && JoinedContains( search->shape.lines, "extra" ) );

            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            UInt32 shapeLines = 0;
            for ( const char* p = journalText.C_String(); GUCEF_NULL != ( p = strstr( p, "\"responseShape\"" ) ); ++p )
                ++shapeLines;
            ASSERT_TRUE( 3 == shapeLines );                 // one journal line per request, the shape only on some

            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( Contains( report, "response shape (union of 3 responses)" ) );
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Load( journalPath, rebuilt ) == 5 );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
            ASSERT_TRUE( rebuiltReport == report );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Enum-like query values are recorded with counts when asked for, tokens, ids and secret names never" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_EnumValues" );
            WEB::CSiteDiscoverySettings settings;
            settings.recordEnumQueryValues = true;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            ASSERT_TRUE( redactor.IsEnumLikeQueryValue( "sort_by", "-created" ) );
            ASSERT_TRUE( redactor.IsEnumLikeQueryValue( "listing_types", "template,graphical" ) );
            ASSERT_TRUE( redactor.IsEnumLikeQueryValue( "count", "100" ) );
            ASSERT_TRUE( redactor.IsEnumLikeQueryValue( "ratio", "1.5" ) );
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "q", "lamp shade" ) );                                    // not enum characters
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "q", "this-value-is-longer-than-32-characters" ) );      // too long
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "id", "550e8400-e29b-41d4-a716-446655440000" ) );        // a guid
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "id", "0123456789abcdef0123" ) );                         // long hex
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "ts", "1790000000" ) );                                   // a number of an id's size
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "cursor", "PLANTEDcursorABCD1234xyz" ) );                 // token like
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "cursor", "a1b2c3d4e5" ) );                               // token like, shorter than the tripwire's minimum
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "token", "web" ) );                                       // secret name
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "state", "active" ) );                                    // secret name pattern
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "", "web" ) );
            redactor.RegisterSecret( "Registered-Secret_Value1" );
            ASSERT_FALSE( redactor.IsEnumLikeQueryValue( "mode", "Registered-Secret_Value1" ) );

            WEB::CSiteEndpointCatalog catalog;
            catalog.SetRedactor( &redactor );
            const char* const urls[] = {
                "https://shop.example/i/library/search?count=24&sort_by=-created&source=web_library&listing_types=template&token=web&state=active&id=550e8400-e29b-41d4-a716-446655440000&q=lamp+shade&cursor=PLANTEDcursorABCD1234xyz",
                "https://shop.example/i/library/search?count=48&sort_by=-created&source=web_library&listing_types=template&token=web&state=active&id=11111111-e29b-41d4-a716-446655440000&q=desk+lamp&cursor=PLANTEDcursorEFGH5678abc",
                "https://shop.example/i/library/search?count=24&sort_by=relevance&source=web_library&listing_types=template&token=web&state=active&ts=1790000000", GUCEF_NULL };
            WEB::CSiteDiscoveryJournal journal;
            journal.SetRedactor( &redactor );
            const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
            ASSERT_TRUE( journal.Open( journalPath ) );
            for ( UInt32 i=0; GUCEF_NULL != urls[ i ]; ++i )
            {
                WEB::CObservedWebRequest o = MakeObservation( i + 1, "GET", urls[ i ], "Fetch", 200, "application/json", 100 );
                catalog.OnWebRequestObserved( o );
                journal.OnWebRequestObserved( o );
            }
            journal.Close();

            CORE::CString report;
            ASSERT_TRUE( catalog.RenderReport( report ) );
            ASSERT_TRUE( Contains( report, "`count` values: `24` (2), `48` (1)" ) );
            ASSERT_TRUE( Contains( report, "`sort_by` values: `-created` (2), `relevance` (1)" ) );
            ASSERT_TRUE( Contains( report, "`source` values: `web_library` (3)" ) );
            ASSERT_TRUE( Contains( report, "`listing_types` values: `template` (3)" ) );
            ASSERT_TRUE( Contains( report, "`token`" ) && Contains( report, "`state`" ) && Contains( report, "`cursor`" ) && Contains( report, "`q`" ) );
            ASSERT_FALSE( Contains( report, "`token` values" ) || Contains( report, "`state` values" ) || Contains( report, "`cursor` values" ) );
            ASSERT_FALSE( Contains( report, "`id` values" ) || Contains( report, "`q` values" ) || Contains( report, "`ts` values" ) );
            ASSERT_FALSE( Contains( report, "PLANTEDcursor" ) || Contains( report, "550e8400" ) || Contains( report, "lamp" ) || Contains( report, "1790000000" ) );

            // the journal holds the enum-like values only, and the report rebuilt from it is the same
            CORE::CString journalText;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
            ASSERT_TRUE( Contains( journalText, "\"queryValues\":[" ) );
            ASSERT_FALSE( Contains( journalText, "PLANTEDcursor" ) || Contains( journalText, "550e8400" ) || Contains( journalText, "lamp" ) || Contains( journalText, "\"web\"" ) );
            WEB::CSiteEndpointCatalog rebuilt;
            rebuilt.SetRedactor( &redactor );
            ASSERT_TRUE( journal.Load( journalPath, rebuilt ) == 3 );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
            ASSERT_TRUE( rebuiltReport == report );

            // off by default: names only
            WEB::CSiteEndpointCatalog plain;
            for ( UInt32 i=0; GUCEF_NULL != urls[ i ]; ++i )
                plain.Add( MakeObservation( i + 1, "GET", urls[ i ], "Fetch", 200, "application/json", 100 ) );
            CORE::CString plainReport;
            ASSERT_TRUE( plain.RenderReport( plainReport ) );
            ASSERT_TRUE( Contains( plainReport, "`count`" ) && !Contains( plainReport, "values:" ) );

            // a parameter with more than twelve distinct values keeps twelve and says so
            WEB::CSiteEndpointCatalog many;
            many.SetRedactor( &redactor );
            for ( UInt32 i=0; i<15; ++i )
                many.Add( MakeObservation( i + 1, "GET", ( "https://shop.example/i/list?mode=m" + CORE::UInt32ToString( i ) ).C_String(), "Fetch", 200, "application/json", 1 ) );
            const WEB::CDiscoveredEndpoint* list = many.FindEntry( "GET", "https://shop.example/i/list" );
            ASSERT_TRUE( GUCEF_NULL != list && 12 == list->queryEnumValues.find( "mode" )->second.size() && 1 == list->queryEnumOverflow.count( "mode" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Multipart and form bodies give field names, kinds and sizes, never values" )
        try
        {
            const CORE::CString body = "--XYZ\r\nContent-Disposition: form-data; name=\"title\"\r\n\r\nMy Private Title PLANTEDmultipartVALUE15\r\n"
                                       "--XYZ\r\nContent-Disposition: form-data; name=\"csrfmiddlewaretoken\"\r\n\r\nPLANTEDmultiCSRFvalue16\r\n"
                                       "--XYZ\r\nContent-Disposition: form-data; name=\"password\"\r\n\r\nPLANTEDmultiPASSWORDv17\r\n"
                                       "--XYZ\r\nContent-Disposition: form-data; name=\"file\"; filename=\"private-name.png\"\r\nContent-Type: image/png\r\n\r\nBINARYDATAxxxx\r\n"
                                       "--XYZ--\r\n";
            WEB::CWebBodyShape shape;
            WEB::CWebBodyShape::DescribeMultipart( body, shape );
            ASSERT_TRUE( 4 == shape.lines.size() );
            ASSERT_TRUE( JoinedContains( shape.lines, "title: text, 40 bytes" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "csrfmiddlewaretoken: csrf-like, value withheld" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "password: text, value withheld" ) );
            ASSERT_TRUE( JoinedContains( shape.lines, "file: file, image/png, 14 bytes" ) );
            for ( size_t i=0; i<shape.lines.size(); ++i )
                ASSERT_FALSE( Contains( shape.lines[ i ], "PLANTED" ) || Contains( shape.lines[ i ], "Private" ) || Contains( shape.lines[ i ], "private-name" ) || Contains( shape.lines[ i ], "BINARYDATA" ) );

            WEB::CWebBodyShape form;
            WEB::CWebBodyShape::DescribeFormUrlEncoded( "user=bob&_csrf=PLANTEDformSECRET10&apiKey=PLANTEDapiKEY9", form );
            ASSERT_TRUE( JoinedContains( form.lines, "user: string, 3 bytes" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "_csrf: csrf-like, value withheld" ) );
            ASSERT_TRUE( JoinedContains( form.lines, "apiKey: string, value withheld" ) );
            ASSERT_FALSE( JoinedContains( form.lines, "PLANTED" ) || JoinedContains( form.lines, "bob" ) );
            ASSERT_TRUE( WEB::CWebBodyShape::IsCsrfLikeName( "authenticity_token" ) && !WEB::CWebBodyShape::IsCsrfLikeName( "title" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Checkout and payment hosts and paths are never recorded by default, cookie names are parsed without values" )
        try
        {
            WEB::CSiteDiscoverySettings settings;
            WEB::CSiteDiscoveryRedactor redactor( settings );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://payment-website-pci.ol.epicgames.com/v2/purchase/550e8400-e29b-41d4-a716-446655440000/confirm" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://pay.shop.example/start" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://checkout.shop.example/" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://billing.shop.example/" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://pci-gateway.shop.example/" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/api/purchase/1" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/api/checkout/cart" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/x/payment/methods" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/x/billing" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/paypal/return" ) );
            ASSERT_TRUE( redactor.IsNeverRecordUrl( "https://shop.example/card/add" ) );
            ASSERT_FALSE( redactor.IsNeverRecordUrl( "https://shop.example/i/library/search" ) );
            ASSERT_FALSE( redactor.IsNeverRecordUrl( "https://www.shop.example/listings/12345" ) );

            CORE::CString::StringVector names;
            WEB::CSiteDiscoveryRedactor::ParseCookieNames( "sid=PLANTEDcookieVALUE2; theme=dark; flag; theme=light", names );
            ASSERT_TRUE( 3 == names.size() && names[ 0 ] == "sid" && names[ 1 ] == "theme" && names[ 2 ] == "flag" );
            names.clear();
            WEB::CSiteDiscoveryRedactor::ParseSetCookieNames( "a=PLANTEDx1234567; Path=/; HttpOnly\nb=2; Secure", names );
            ASSERT_TRUE( 2 == names.size() && names[ 0 ] == "a" && names[ 1 ] == "b" );
            ASSERT_TRUE( redactor.RedactCookieName( "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08" ) != "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Scrubbing a journal reduces the lines of checkout and payment requests, keeps a backup and the rest byte for byte" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_Scrub" );
            WEB::CSiteDiscoverySettings settings;
            settings.outputDirectory = dir.GetPath();

            // a journal as an older version wrote it: without the checkout and payment rules
            WEB::CSiteDiscoverySettings old = settings;
            old.neverRecordPathPatterns.clear();
            old.neverRecordPathPatterns.push_back( "login" );
            old.neverRecordHosts.clear();
            WEB::CSiteDiscoverySession writer;
            writer.SetSettings( old );
            ASSERT_TRUE( writer.GetJournal().Open( writer.GetJournalPath() ) );
            writer.GetJournal().AppendAction( CObservedActionFor( 1, "click", "Pay now", "https://payment-website-pci.example/v2/purchase/{guid}" ) );
            writer.GetJournal().AppendAction( CObservedActionFor( 2, "click", "Open item", "https://shop.example/listings/{n}" ) );
            WEB::CObservedWebRequest normal = MakeObservation( 1, "GET", "https://shop.example/i/library/search?count=24", "Fetch", 200, "application/json", 100 );
            WEB::CObservedWebRequest paying = MakeObservation( 2, "POST", "https://payment-website-pci.example/v2/purchase/550e8400-e29b-41d4-a716-446655440000/confirm?step=2", "Fetch", 200, "application/json", 50 );
            paying.requestContentType = "application/json";
            paying.requestBodyShape.lines.push_back( "cardholder: string" );
            paying.hasResponseShape = true;
            paying.responseShape.lines.push_back( "orderStatus: string" );
            paying.cookiesSent.push_back( "paycookie" );
            WEB::CObservedWebRequest cart = MakeObservation( 3, "GET", "https://shop.example/api/checkout/cart", "Fetch", 200, "application/json", 70 );
            cart.hasResponseShape = true;
            cart.responseShape.lines.push_back( "cartTotal: number" );
            WEB::CObservedWebRequest signIn = MakeObservation( 4, "POST", "https://shop.example/login", "Fetch", 200, "application/json", 10 );
            signIn.neverRecorded = true;
            writer.GetJournal().Append( normal );
            writer.GetJournal().Append( paying );
            writer.GetJournal().Append( cart );
            writer.GetJournal().Append( signIn );
            writer.GetJournal().Close();

            CORE::CString before;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( writer.GetJournalPath(), before ) );
            ASSERT_TRUE( Contains( before, "cardholder" ) && Contains( before, "cartTotal" ) && Contains( before, "paycookie" ) );

            // the session with the current rules scrubs it
            WEB::CSiteDiscoverySession scrubber;
            scrubber.SetSettings( settings );
            WEB::CSiteDiscoveryJournal::SScrubResult result;
            CORE::CString problem;
            ASSERT_TRUE( scrubber.ScrubJournal( result, problem ) );
            ASSERT_TRUE( 6 == result.lines );
            ASSERT_TRUE( 2 == result.stubbedRequests && 0 == result.droppedRequests && 1 == result.stubbedActions && 0 == result.unreadableLines );
            ASSERT_TRUE( Contains( result.backupPath, "requests.jsonl.bak" ) );

            // the copy is the journal as it was and owner-only
            CORE::CString backup;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( result.backupPath, backup ) );
            ASSERT_TRUE( backup == before );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
            ASSERT_TRUE( 0600 == ModeOf( result.backupPath ) && 0600 == ModeOf( scrubber.GetJournalPath() ) );
            #endif

            CORE::CString after;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( scrubber.GetJournalPath(), after ) );
            ASSERT_FALSE( Contains( after, "cardholder" ) || Contains( after, "orderStatus" ) || Contains( after, "cartTotal" ) || Contains( after, "paycookie" ) );
            ASSERT_FALSE( Contains( after, "step" ) || Contains( after, "Pay now" ) );
            ASSERT_TRUE( Contains( after, "payment-website-pci.example" ) );
            ASSERT_TRUE( Contains( after, "\"seq\":1," ) && Contains( after, "\"seq\":4," ) && Contains( after, "count" ) );

            // the lines that did not match are untouched
            const WEB::CSiteDiscoveryJournal plain;
            ASSERT_TRUE( Contains( after, plain.ToJournalLine( normal ).C_String() ) );
            ASSERT_TRUE( Contains( after, "Open item" ) );

            // the report was rebuilt: the payment endpoint is noted only
            CORE::CString report;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( scrubber.GetReportPath(), report ) );
            ASSERT_TRUE( Contains( report, "### POST payment-website-pci.example/v2/purchase/{guid}/confirm" ) && Contains( report, "content not recorded" ) );
            ASSERT_FALSE( Contains( report, "cardholder" ) || Contains( report, "cartTotal" ) || Contains( report, "paycookie" ) );

            // a second scrub has nothing left to do and keeps its own backup
            WEB::CSiteDiscoverySession again;
            again.SetSettings( settings );
            WEB::CSiteDiscoveryJournal::SScrubResult second;
            ASSERT_TRUE( again.ScrubJournal( second, problem ) );
            ASSERT_TRUE( 0 == second.stubbedRequests && 0 == second.stubbedActions );
            ASSERT_TRUE( second.backupPath != result.backupPath && CORE::FileExists( second.backupPath ) );
            CORE::CString afterAgain;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( again.GetJournalPath(), afterAgain ) );
            ASSERT_TRUE( afterAgain == after );

            // with noting off such lines are dropped
            WEB::CSiteDiscoverySettings quiet = settings;
            quiet.noteNeverRecorded = false;
            TESTFW::CScopedTestDir quietDir( "gucefWEB_SiteDiscovery_ScrubQuiet" );
            quiet.outputDirectory = quietDir.GetPath();
            WEB::CSiteDiscoverySession quietWriter;
            WEB::CSiteDiscoverySettings quietOld = old;
            quietOld.outputDirectory = quietDir.GetPath();
            quietWriter.SetSettings( quietOld );
            ASSERT_TRUE( quietWriter.GetJournal().Open( quietWriter.GetJournalPath() ) );
            quietWriter.GetJournal().Append( normal );
            quietWriter.GetJournal().Append( paying );
            quietWriter.GetJournal().Close();
            WEB::CSiteDiscoverySession quietScrubber;
            quietScrubber.SetSettings( quiet );
            WEB::CSiteDiscoveryJournal::SScrubResult dropped;
            ASSERT_TRUE( quietScrubber.ScrubJournal( dropped, problem ) );
            ASSERT_TRUE( 1 == dropped.droppedRequests && 0 == dropped.stubbedRequests );
            CORE::CString quietAfter;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( quietScrubber.GetJournalPath(), quietAfter ) );
            ASSERT_FALSE( Contains( quietAfter, "payment-website-pci" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Values of a user, as a site may put them where a report expects structure; none may reach a journal or report with the rules on */
static const char* const UserValues[] = { "ZXQ", "YYW", "73021945", "Jane Doe", "Joint_Tenant", "red+shoes", "red shoes", "red%20shoes",
                                          "aGVsbG8gd29ybGQgdGhpcyBpcw2Q", "acct12", "Rainy Day", GUCEF_NULL };

static bool
ContainsAnyUserValue( const CORE::CString& text )
{
    for ( UInt32 i=0; GUCEF_NULL != UserValues[ i ]; ++i )
    {
        if ( Contains( text, UserValues[ i ] ) )
        {
            // synthetic values only: the place is logged to tell which rule missed it
            const char* at = strstr( text.C_String(), UserValues[ i ] );
            const UInt32 offset = (UInt32) ( at - text.C_String() );
            const UInt32 from = offset > 80 ? offset - 80 : 0;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Value rules test: the text still holds the user value " << UserValues[ i ] << " in: " <<
                             CORE::CString( text.C_String() + from, offset - from + 40 < text.Length() - from ? offset - from + 40 : text.Length() - from ) );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The value rules as a site with codes and names of the user would configure them */
static WEB::CSiteDiscoverySettings
ValueRuleSettings( const CORE::CString& outputDirectory )
{
    WEB::CSiteDiscoverySettings settings;
    settings.outputDirectory = outputDirectory;
    settings.maskUppercaseTokens = true;
    settings.sensitiveValues.push_back( "Jane Doe" );
    settings.sensitiveValues.push_back( "Rainy Day" );
    settings.sensitiveValuePatterns.push_back( "acct*" );
    settings.valueQueryNames.push_back( "pick" );
    settings.valueQueryNames.push_back( "picks" );
    settings.recordEnumQueryValues = true;
    return settings;
}

/*-------------------------------------------------------------------------*/

static void
PerformValueRuleTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: The value rule settings have their defaults, and load and save" )
        try
        {
            WEB::CSiteDiscoverySettings defaults;
            ASSERT_TRUE( defaults.maskIdLikeMemberNames && defaults.maskTokenLikeParameterNames && defaults.maxParameterNameLength == 40 );
            ASSERT_FALSE( defaults.maskUppercaseTokens );
            ASSERT_TRUE( defaults.uppercaseTokenMaxLength == 5 && defaults.maskFileNameWords );
            ASSERT_TRUE( defaults.sensitiveValues.empty() && defaults.sensitiveValuePatterns.empty() && defaults.enumQueryNames.empty() );
            ASSERT_TRUE( JoinedContains( defaults.valueQueryNames, "query" ) && JoinedContains( defaults.valuePathSegmentsAfter, "search" ) );
            ASSERT_TRUE( JoinedContains( defaults.nonSensitiveValues, "PDF" ) );

            WEB::CSiteDiscoverySettings settings = ValueRuleSettings( "/out" );
            settings.enumQueryNames.push_back( "view" );
            settings.maxParameterNameLength = 33;
            settings.maskFileNameWords = false;
            CORE::CDataNode cfg;
            ASSERT_TRUE( settings.SaveConfig( cfg ) );
            WEB::CSiteDiscoverySettings again;
            ASSERT_TRUE( again.LoadConfig( cfg ) );
            ASSERT_TRUE( again.maskUppercaseTokens && again.maxParameterNameLength == 33 && !again.maskFileNameWords );
            ASSERT_TRUE( again.sensitiveValues.size() == 2 && again.sensitiveValues[ 0 ] == "Jane Doe" && again.sensitiveValuePatterns.size() == 1 );
            ASSERT_TRUE( again.enumQueryNames.size() == 1 && JoinedContains( again.valueQueryNames, "picks" ) );

            CORE::CDataNode json;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"maskUppercaseTokens\":true,\"uppercaseTokenMaxLength\":4,\"valuePathSegmentsAfter\":[],"
                                                                 "\"nonSensitiveValues\":[\"ETA\"],\"maskIdLikeMemberNames\":false}", json ) );
            WEB::CSiteDiscoverySettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( json ) );
            ASSERT_TRUE( fromJson.maskUppercaseTokens && fromJson.uppercaseTokenMaxLength == 4 && fromJson.valuePathSegmentsAfter.empty() );
            ASSERT_TRUE( fromJson.nonSensitiveValues.size() == 1 && !fromJson.maskIdLikeMemberNames );
            CORE::CDataNode zero;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"maxParameterNameLength\":0}", zero ) );
            ASSERT_FALSE( fromJson.LoadConfig( zero ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The value rules mask member names, parameter names, path segments, query values, labels and file names" )
        try
        {
            // the defaults: identifiers as member names, token-like parameter names, search terms
            WEB::CSiteDiscoveryRedactor plain;
            ASSERT_TRUE( plain.RedactMemberName( "73021945" ) == "{key}" );
            ASSERT_TRUE( plain.RedactMemberName( "0f8fad5b-d9cb-469f-a165-70867728950e" ) == "{key}" );
            ASSERT_TRUE( plain.RedactMemberName( "2026-01-31" ) == "{key}" );
            ASSERT_TRUE( plain.RedactMemberName( "ZXQ" ) == "ZXQ" );
            ASSERT_TRUE( plain.RedactMemberName( "unitPrice" ) == "unitPrice" && plain.RedactMemberName( "line2" ) == "line2" );
            ASSERT_TRUE( plain.RedactMemberName( "{key}" ) == "{key}" );
            ASSERT_TRUE( plain.IsTokenLikeParameterName( "aGVsbG8gd29ybGQgdGhpcyBpcw2Q" ) );
            ASSERT_TRUE( plain.IsTokenLikeParameterName( "dGVzdA==" ) );
            ASSERT_TRUE( plain.IsTokenLikeParameterName( "averyveryverylongparameternamethatnobodywouldwrite" ) );
            ASSERT_FALSE( plain.IsTokenLikeParameterName( "sort" ) || plain.IsTokenLikeParameterName( "pageSize" ) || plain.IsTokenLikeParameterName( "include_totals" ) );
            ASSERT_TRUE( plain.MaskUrl( "https://api.shop.test/v1/search/red%20shoes?q=red+shoes&page=2" ) == "https://api.shop.test/v1/search/{value}?q={value}&page=2" );
            ASSERT_TRUE( plain.MaskUrl( "https://api.shop.test/v1/items?aGVsbG8gd29ybGQgdGhpcyBpcw2Q=1&sort=asc" ) == "https://api.shop.test/v1/items?{token-like name}=1&sort=asc" );
            ASSERT_TRUE( plain.RedactPath( "/v1/quote/ZXQ" ) == "/v1/quote/ZXQ" );
            ASSERT_TRUE( plain.RedactLabel( "Sell ZXQ" ) == "Sell ZXQ" );

            // with the rules of a site that shows codes and names of the user
            WEB::CSiteDiscoveryRedactor redactor( ValueRuleSettings( CORE::CString::Empty ) );
            ASSERT_TRUE( redactor.RedactMemberName( "ZXQ" ) == "{key}" && redactor.RedactMemberName( "XY.Z" ) == "{key}" );
            ASSERT_TRUE( redactor.RedactMemberName( "Jane Doe" ) == "{key}" && redactor.RedactMemberName( "acct12" ) == "{key}" );
            ASSERT_TRUE( redactor.RedactMemberName( "ID" ) == "ID" && redactor.RedactMemberName( "price" ) == "price" );
            ASSERT_TRUE( redactor.RedactMemberName( "TOOLONGCODE" ) == "TOOLONGCODE" );
            ASSERT_TRUE( redactor.IsSensitiveToken( "ZXQ" ) && redactor.IsSensitiveToken( "acct99" ) && !redactor.IsSensitiveToken( "PDF" ) && !redactor.IsSensitiveToken( "Zxq" ) );

            ASSERT_TRUE( redactor.RedactShapeLine( "holdings.ZXQ.quantity: number" ) == "holdings.{key}.quantity: number" );
            ASSERT_TRUE( redactor.RedactShapeLine( "prefs.73021945.view: string" ) == "prefs.{key}.view: string" );
            ASSERT_TRUE( redactor.RedactShapeLine( "byCode.ZXQ[]: null | string (optional)" ) == "byCode.{key}[]: null | string (optional)" );
            ASSERT_TRUE( redactor.RedactShapeLine( "items[].name: string" ) == "items[].name: string" );
            ASSERT_TRUE( redactor.RedactShapeLine( "links.ZXQ -> https://api.shop.test/v1/quote/ZXQ" ) == "links.{key} -> https://api.shop.test/v1/quote/{value}" );
            ASSERT_TRUE( redactor.RedactShapeLine( "names.Jane Doe: string" ) == "names.{key}: string" );
            WEB::CWebBodyShape shape;
            shape.lines.push_back( "holdings.ZXQ.quantity: number" );
            shape.lines.push_back( "holdings.YYW.quantity: number" );
            shape.lines.push_back( "holdings.ZXQ.cost: number" );
            shape.paginationHints.push_back( "pages.73021945.next" );
            ASSERT_TRUE( redactor.RedactShape( shape ) );
            ASSERT_TRUE( shape.lines.size() == 2 && shape.lines[ 0 ] == "holdings.{key}.quantity: number" && shape.lines[ 1 ] == "holdings.{key}.cost: number" );
            ASSERT_TRUE( shape.paginationHints.size() == 1 && shape.paginationHints[ 0 ] == "pages.{key}.next" );
            ASSERT_FALSE( redactor.RedactShape( shape ) );

            ASSERT_TRUE( redactor.MaskUrl( "https://api.shop.test/v1/quote/ZXQ/detail?pick=ZXQ&type=ZXQ&page=2&q=red+shoes" ) ==
                         "https://api.shop.test/v1/quote/{value}/detail?pick={value}&type={value}&page=2&q={value}" );
            ASSERT_TRUE( redactor.MaskUrl( "https://api.shop.test/v1/API/items" ) == "https://api.shop.test/v1/API/items" );
            ASSERT_TRUE( redactor.RedactPath( "/v1/lookup/abc/x" ) == "/v1/lookup/{value}/x" );
            ASSERT_TRUE( redactor.RedactAddressTemplate( "www.shop.test/app/quote/ZXQ" ) == "www.shop.test/app/quote/{value}" );
            ASSERT_TRUE( redactor.RedactAddressTemplate( "https://www.shop.test/app/search/abc" ) == "https://www.shop.test/app/search/{value}" );

            ASSERT_TRUE( redactor.RedactLabel( "Sell 10 ZXQ for Jane Doe" ) == "Sell {n} {value} for {value}" );
            ASSERT_TRUE( redactor.RedactLabel( "Download PDF" ) == "Download PDF" );
            ASSERT_TRUE( redactor.RedactLabel( "Rainy Day account acct12" ) == "{value} account {value}" );
            ASSERT_TRUE( redactor.RedactLabel( redactor.RedactLabel( "Sell 10 ZXQ for Jane Doe" ) ) == "Sell {n} {value} for {value}" );

            ASSERT_TRUE( redactor.RedactFileName( "Positions_12345678_Joint_Tenant_Account_2026-01-31.csv" ) == "Positions_{n}_{text}_{n}-{n}-{n}.csv" );
            ASSERT_TRUE( redactor.RedactFileName( "Lot-Details.csv" ) == "Lot-Details.csv" );
            ASSERT_TRUE( redactor.RedactFileName( "Lot-Details 123.csv" ) == "Lot-Details {n}.csv" );
            ASSERT_TRUE( redactor.RedactFileName( "Report_ZXQ_2026.pdf" ) == "Report_{value}_{n}.pdf" );
            ASSERT_TRUE( redactor.RedactFileName( "Positions_{n}_{text}_{n}-{n}-{n}.csv" ) == "Positions_{n}_{text}_{n}-{n}-{n}.csv" );
            WEB::CSiteDiscoverySettings keepWords = ValueRuleSettings( CORE::CString::Empty );
            keepWords.maskFileNameWords = false;
            WEB::CSiteDiscoveryRedactor wordsKept( keepWords );
            ASSERT_TRUE( wordsKept.RedactFileName( "Positions_12345678_Joint_Tenant_Account_2026-01-31.csv" ) == "Positions_{n}_Joint_Tenant_Account_{n}-{n}-{n}.csv" );

            // enum values: never of value names, codes masked, only the configured names when there are any
            WEB::CUrlPathTemplate parsed;
            ASSERT_TRUE( WEB::CUrlPathTemplate::Parse( "https://api.shop.test/v1/items?view=compact&pick=ZXQ&type=ZXQ&sort=asc&q=red", parsed ) );
            GUCEF::vector< std::pair< CORE::CString, CORE::CString > > enums;
            redactor.ExtractEnumQueryValues( parsed, enums );
            ASSERT_TRUE( enums.size() == 3 && enums[ 0 ].first == "view" && enums[ 1 ].first == "type" && enums[ 1 ].second == "{value}" && enums[ 2 ].first == "sort" );
            WEB::CSiteDiscoverySettings onlyView = ValueRuleSettings( CORE::CString::Empty );
            onlyView.enumQueryNames.push_back( "view" );
            WEB::CSiteDiscoveryRedactor viewOnly( onlyView );
            viewOnly.ExtractEnumQueryValues( parsed, enums );
            ASSERT_TRUE( enums.size() == 1 && enums[ 0 ].first == "view" && enums[ 0 ].second == "compact" );

            // example values: the value names and the codes never, other values as they are
            ASSERT_TRUE( redactor.RedactQueryValue( "q", "red+shoes" ) == "{value}" && redactor.RedactQueryValue( "type", "ZXQ" ) == "{value}" );
            ASSERT_TRUE( redactor.RedactQueryValue( "page", "2" ) == "2" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The value rules apply to the report only: the journal keeps the values, a rebuild with other settings gives another report" )
        try
        {
            for ( UInt32 examples=0; examples<2; ++examples )
            {
                TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_ValueRules" );
                WEB::CSiteDiscoverySettings settings = ValueRuleSettings( dir.GetPath() );
                settings.recordExampleValues = ( examples == 1 );
                WEB::CSiteDiscoveryRedactor redactor( settings );
                WEB::CSiteDiscoveryJournal journal;
                journal.SetRedactor( &redactor );
                WEB::CSiteEndpointCatalog catalog;
                catalog.SetRedactor( &redactor );
                const CORE::CString journalPath = CORE::CombinePath( dir.GetPath(), "requests.jsonl" );
                ASSERT_TRUE( journal.Open( journalPath ) );

                WEB::CObservedUserAction action;
                action.id = 1;
                action.timestamp = 100.5;
                action.kind = WEB::CObservedUserAction::KindClick;
                action.label = "Sell 10 ZXQ for Jane Doe";
                action.page = "https://www.shop.test/app/quote/ZXQ";
                journal.AppendAction( action );
                catalog.AddAction( action );

                WEB::CObservedWebRequest search = MakeObservation( 1, "GET", "https://api.shop.test/v1/search/red%20shoes?q=red+shoes&pick=ZXQ&type=YYW&aGVsbG8gd29ybGQgdGhpcyBpcw2Q=1&page=2",
                                                                   "XHR", 200, "application/json", 100 );
                search.actionId = 1;
                search.requestHeaders[ "referer" ] = "https://www.shop.test/app/quote/ZXQ";
                search.queryEnumValues.push_back( std::make_pair( CORE::CString( "type" ), CORE::CString( "YYW" ) ) );
                search.queryEnumValues.push_back( std::make_pair( CORE::CString( "pick" ), CORE::CString( "ZXQ" ) ) );
                search.hasResponseShape = true;
                search.responseShape.lines.push_back( "holdings.ZXQ.quantity: number" );
                search.responseShape.lines.push_back( "holdings.YYW.quantity: number" );
                search.responseShape.lines.push_back( "prefs.73021945.view: string" );
                search.responseShape.lines.push_back( "names.Jane Doe: string" );
                search.responseShape.urlFields.push_back( "links.ZXQ -> https://api.shop.test/v1/quote/ZXQ" );

                WEB::CObservedWebRequest post = MakeObservation( 2, "POST", "https://api.shop.test/v1/lists/acct12/items", "XHR", 201, "application/json", 10 );
                post.requestContentType = "application/json";
                post.requestBodyShape.lines.push_back( "symbols.ZXQ: boolean" );
                post.postData = "{\"symbols\":{\"ZXQ\":true},\"owner\":\"Jane Doe\"}";

                WEB::CObservedWebRequest download = MakeObservation( 3, "GET", "https://files.shop.test/export/positions", "Document", 200, "text/csv", 1000 );
                download.isDownload = true;
                download.contentDisposition = "attachment";
                download.downloadFileName = "Positions_12345678_Joint_Tenant_Account_2026-01-31.csv";

                WEB::CObservedWebRequest redirect = MakeObservation( 4, "GET", "https://www.shop.test/app/go", "Document", 302, "text/html", 0 );
                redirect.redirectTarget = "www.shop.test/app/search/ZXQ";

                WEB::CObservedWebRequest* all[] = { &search, &post, &download, &redirect };
                for ( UInt32 i=0; i<4; ++i )
                {
                    ASSERT_TRUE( journal.Append( *all[ i ] ) );
                    catalog.Add( *all[ i ] );
                }
                journal.Close();

                CORE::CString journalText;
                ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( journalPath, journalText ) );
                CORE::CString report;
                ASSERT_TRUE( catalog.RenderReport( report ) );
                ASSERT_FALSE( ContainsAnyUserValue( report ) );
                ASSERT_TRUE( Contains( journalText, "Sell 10 ZXQ for Jane Doe" ) && Contains( journalText, "holdings.ZXQ.quantity" ) && Contains( journalText, "prefs.73021945.view" ) );
                ASSERT_TRUE( Contains( journalText, "Positions_12345678_Joint_Tenant_Account_2026-01-31.csv" ) && Contains( journalText, "aGVsbG8gd29ybGQgdGhpcyBpcw2Q" ) );
                ASSERT_FALSE( Contains( journalText, "{key}" ) || Contains( journalText, "{text}" ) || Contains( journalText, "{token-like name}" ) );
                ASSERT_TRUE( Contains( report, "holdings.{key}.quantity" ) && Contains( report, "prefs.{key}.view" ) );
                ASSERT_TRUE( Contains( report, "/v1/search/{value}" ) && Contains( report, "{token-like name}" ) );
                ASSERT_TRUE( Contains( report, "Positions_{n}_{text}_{n}-{n}-{n}.csv" ) );
                ASSERT_TRUE( Contains( report, "Sell {n} {value} for {value}" ) );
                ASSERT_TRUE( Contains( report, "/v1/lists/{value}/items" ) );
                ASSERT_TRUE( GUCEF_NULL != catalog.FindEntry( "GET", "https://api.shop.test/v1/search/other?q=x" ) );

                // the live catalog and one rebuilt from the journal render the same report
                WEB::CSiteEndpointCatalog rebuilt;
                rebuilt.SetRedactor( &redactor );
                WEB::CSiteDiscoveryJournal reader;
                reader.SetRedactor( &redactor );
                ASSERT_TRUE( 4 == reader.Load( journalPath, rebuilt ) );
                CORE::CString rebuiltReport;
                ASSERT_TRUE( rebuilt.RenderReport( rebuiltReport ) );
                ASSERT_TRUE( rebuiltReport == report );

                // the same journal with other value rules: another report, without recording again
                WEB::CSiteDiscoverySettings plain;
                plain.outputDirectory = dir.GetPath();
                plain.recordExampleValues = settings.recordExampleValues;
                plain.maskFileNameWords = false;
                WEB::CSiteDiscoveryRedactor plainRedactor( plain );
                WEB::CSiteEndpointCatalog plainCatalog;
                plainCatalog.SetRedactor( &plainRedactor );
                WEB::CSiteDiscoveryJournal plainReader;
                plainReader.SetRedactor( &plainRedactor );
                ASSERT_TRUE( 4 == plainReader.Load( journalPath, plainCatalog ) );
                CORE::CString plainReport;
                ASSERT_TRUE( plainCatalog.RenderReport( plainReport ) );
                ASSERT_TRUE( plainReport != report );
                ASSERT_TRUE( Contains( plainReport, "holdings.ZXQ.quantity" ) && Contains( plainReport, "Sell {n} ZXQ for Jane Doe" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A report rebuilt from an older journal, and a scrubbed journal, follow the value rules of today" )
        try
        {
            TESTFW::CScopedTestDir dir( "gucefWEB_SiteDiscovery_ValueRulesRebuild" );
            const WEB::CSiteDiscoverySettings settings = ValueRuleSettings( dir.GetPath() );

            // a journal as a recorder without the value rules wrote it
            const CORE::CString plainLine = "{\"seq\":3,\"ts\":103,\"method\":\"GET\",\"url\":\"https://api.shop.test/v1/items?page\",\"type\":\"XHR\",\"status\":200,"
                                            "\"mime\":\"application/json\",\"size\":5,\"failed\":false,\"error\":\"\",\"bodyFile\":\"\",\"postData\":\"\",\"requestHeaders\":{},"
                                            "\"action\":0,\"neverRecorded\":false}";
            const CORE::CString older =
                "{\"action\":1,\"ts\":100.5,\"kind\":\"click\",\"label\":\"Sell ZXQ for Jane Doe\",\"page\":\"https://www.shop.test/app/quote/ZXQ\"}\n"
                "{\"seq\":1,\"ts\":101,\"method\":\"GET\",\"url\":\"https://api.shop.test/v1/quote/ZXQ?aGVsbG8gd29ybGQgdGhpcyBpcw2Q&q&type\",\"type\":\"XHR\",\"status\":200,"
                "\"mime\":\"application/json\",\"size\":10,\"failed\":false,\"error\":\"\",\"bodyFile\":\"\",\"postData\":\"\",\"requestHeaders\":{\"accept\":\"application/json\"},"
                "\"action\":1,\"queryValues\":[\"type\",\"YYW\",\"q\",\"red shoes\"],\"responseShape\":[\"holdings.ZXQ.quantity: number\",\"prefs.73021945.view: string\"],"
                "\"responsePaging\":[],\"responseUrls\":[],\"responseShapeTruncated\":false,\"neverRecorded\":false}\n"
                "{\"seq\":2,\"ts\":102,\"method\":\"GET\",\"url\":\"https://files.shop.test/export/positions\",\"type\":\"Document\",\"status\":200,\"mime\":\"text/csv\","
                "\"size\":1000,\"failed\":false,\"error\":\"\",\"bodyFile\":\"\",\"postData\":\"\",\"requestHeaders\":{},\"action\":0,"
                "\"download\":{\"disposition\":\"attachment\",\"fileName\":\"Positions_{n}_Joint_Tenant_Account_{n}.csv\",\"contentType\":\"text/csv\",\"size\":1000},"
                "\"neverRecorded\":false}\n" + plainLine + "\n";

            WEB::CSiteDiscoverySession session;
            session.SetSettings( settings );
            CORE::CString problem;
            ASSERT_TRUE( CORE::CreateOwnerOnlyFile( session.GetJournalPath(), older.C_String(), older.ByteSize() - 1, problem ) );

            // the rebuild only reads the journal, and its report follows the rules of today
            ASSERT_TRUE( session.RebuildReportFromJournal( problem ) );
            CORE::CString journalAfterRebuild;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetJournalPath(), journalAfterRebuild ) );
            ASSERT_TRUE( journalAfterRebuild == older );
            CORE::CString rebuiltReport;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetReportPath(), rebuiltReport ) );
            ASSERT_FALSE( ContainsAnyUserValue( rebuiltReport ) );
            ASSERT_TRUE( Contains( rebuiltReport, "holdings.{key}.quantity" ) && Contains( rebuiltReport, "/v1/quote/{value}" ) && Contains( rebuiltReport, "{token-like name}" ) );
            ASSERT_TRUE( Contains( rebuiltReport, "Positions_{n}_{text}_{n}.csv" ) && Contains( rebuiltReport, "Sell {value} for {value}" ) );
            ASSERT_TRUE( Contains( rebuiltReport, "### GET api.shop.test/v1/items" ) );

            // the scrub rewrites the lines that held values, keeps the others byte for byte, and keeps the old journal as a backup
            WEB::CSiteDiscoveryJournal::SScrubResult result;
            ASSERT_TRUE( session.ScrubJournal( result, problem ) );
            ASSERT_TRUE( result.lines == 4 && result.maskedRequests == 2 && result.maskedActions == 1 && result.stubbedRequests == 0 && result.unreadableLines == 0 );
            ASSERT_TRUE( CORE::FileExists( result.backupPath ) );
            CORE::CString scrubbed;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetJournalPath(), scrubbed ) );
            ASSERT_FALSE( ContainsAnyUserValue( scrubbed ) );
            ASSERT_TRUE( Contains( scrubbed, plainLine.C_String() ) );
            ASSERT_TRUE( Contains( scrubbed, "\"seq\":1," ) && Contains( scrubbed, "\"seq\":2," ) );
            CORE::CString scrubbedReport;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetReportPath(), scrubbedReport ) );
            ASSERT_TRUE( scrubbedReport == rebuiltReport );

            // a second scrub finds nothing more to mask
            WEB::CSiteDiscoveryJournal::SScrubResult second;
            ASSERT_TRUE( session.ScrubJournal( second, problem ) );
            ASSERT_TRUE( second.maskedRequests == 0 && second.maskedActions == 0 );
            CORE::CString scrubbedTwice;
            ASSERT_TRUE( WEB::CSiteDiscoveryJournal::ReadWholeFile( session.GetJournalPath(), scrubbedTwice ) );
            ASSERT_TRUE( scrubbedTwice == scrubbed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSiteDiscoveryTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING site discovery TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "SiteDiscovery" );

    if ( !JsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Site discovery TESTS skipped: the json codec plugin is not available" );
        return;
    }

    PerformUrlTemplateTests();
    PerformBodyShapeTests();
    PerformSettingsTests();
    PerformRedactorTests();
    PerformActionTrackerTests();
    PerformCatalogTests();
    PerformTripwireTests();
    PerformBrowserHelperTests();
    PerformRecorderTests();
    PerformDiscoveryGapTests();
    PerformValueRuleTests();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL site discovery TESTS COMPLETED" );
}
