/*
 *  AncestryDotComBackup_TestApp: tests of the logic of the Ancestry.com backup tool
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "AncestryBackup_CDataLayout.h"
#include "AncestryBackup_CEndpointCatalog.h"
#include "AncestryBackup_CDiscoveryJournal.h"
#include "AncestryBackup_CChromeBrowser.h"
#include "AncestryBackup_CChromeNetworkRecorder.h"

#include "TestDiscoveryEndToEnd.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::ANCESTRYBACKUP;

static const UInt16 TEST_SITE_PORT = 47610;
static const UInt16 TEST_BROWSER_PORT = 47620;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

static void
Pump( UInt32 delayInMs )
{
    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
    MT::ThreadDelay( delayInMs );
}

/*-------------------------------------------------------------------------*/

/** The site the browser visits: a page that does what a single page application does, a JSON API, an image */
class CTestSiteHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString body;
        response.statusCode = 200;
        response.contentType = "application/json; charset=UTF-8";

        if ( request.requestUri == "/" )
        {
            response.contentType = "text/html; charset=UTF-8";
            body = "<html><head><title>test site</title></head><body><script>\n"
                   "document.cookie = 'sid=secret123; path=/';\n"
                   "setTimeout( function() {\n"
                   "fetch('/api/tree/1234567/person/7654321/facts?page=1').then(function(r){return r.json();}).then(function(){\n"
                   "  return fetch('/api/tree/1234567/person/7654322/facts?page=2'); });\n"
                   "fetch('/api/tree/1234567/query', {method:'POST', headers:{'content-type':'application/json','x-csrf-token':'t0k3n'}, body: JSON.stringify({q:'x'})});\n"
                   "var i = new Image(); i.src = '/media/55555555.jpg';\n"
                   "}, 1500 );\n"
                   "</script></body></html>";
        }
        else
        if ( request.requestUri.HasSubstr( "/facts" ) >= 0 )
        {
            body = "{\"total\":2,\"page\":1,\"next\":\"/api/tree/1234567/person/7654321/facts?page=2\","
                   "\"items\":[{\"id\":1,\"name\":\"Test Person\",\"media\":\"/media/55555555.jpg\"}]}";
        }
        else
        if ( request.requestUri == "/api/tree/1234567/query" )
            body = "{\"result\":\"ok\"}";
        else
        if ( request.requestUri.HasSubstr( "/media/" ) >= 0 )
        {
            response.contentType = "image/jpeg";
            body = "not really a jpeg";
        }
        else
        {
            response.statusCode = 404;
            return true;
        }

        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CTestSiteHandler, MT::CMutex > TTestSiteHandlerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDiscoveryEndToEndTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AncestryDotComBackup discovery end to end TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DiscoveryEndToEnd" );

    // GUCEF_TEST_BROWSER=<path> makes the test run with another browser, such as Chromium
    CChromeBrowser browser;
    const CORE::CString browserOverride = CORE::GetEnv( "GUCEF_TEST_BROWSER" );
    if ( !browserOverride.IsNULLOrEmpty() )
        browser.SetExecutablePath( browserOverride );
    if ( !PluginsAvailable() || browser.FindExecutable().IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup end to end TESTS skipped: no Chrome or Chromium found, or the json and hash codec plugins are not available" );
        return;
    }

    TTestSiteHandlerFactory handlerFactory;
    WEB::CHttpServerSettings httpSettings;
    httpSettings.processRequestsAsync = false;
    httpSettings.keepAliveConnections = false;
    WEB::CHTTPServer site( &handlerFactory, &httpSettings );
    const bool listening = site.ListenOnPort( TEST_SITE_PORT );

    CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_E2E" );
    CORE::CreateDirs( dir );
    CDataLayout layout( dir, "e2e" );
    CORE::DeleteFile( layout.GetDiscoveryJournalPath() );

    GUCEF_TESTFW_TESTCASE( "Test: Discovery records what a page in a real browser requests" )
        try
        {
            ASSERT_TRUE( listening );
            ASSERT_TRUE( layout.CreateDiscoveryDirs() );

            // never the browser of whoever runs the test, which may have remote debugging switched on
            browser.SetAttachToRunning( false );
            browser.SetDebugPort( TEST_BROWSER_PORT );
            browser.SetProfileDir( layout.GetBrowserProfileDir() );
            browser.SetHeadless( true );
            if ( !CORE::GetEnv( "GUCEF_TEST_CHROME_NO_SANDBOX" ).IsNULLOrEmpty() )
                browser.AddArgument( "--no-sandbox" );
            ASSERT_TRUE( browser.EnsureRunning( "about:blank" ) );

            WEB::CChromeDevToolsClient client;
            ASSERT_TRUE( browser.ConnectToBrowser( client ) );

            CEndpointCatalog catalog;
            CDiscoveryJournal journal;
            ASSERT_TRUE( journal.Open( layout ) );

            CChromeNetworkRecorder recorder( client, layout, catalog, journal );
            std::vector< std::string > filter;
            filter.push_back( "127.0.0.1" );
            recorder.SetHostFilter( filter );
            ASSERT_TRUE( recorder.Start() );

            // the blank page that is open is not the site, so it is not watched
            for ( UInt32 i=0; i<30; ++i )
            {
                Pump( 10 );
                recorder.Process();
            }
            ASSERT_TRUE( recorder.GetSessionCount() == 0 );

            CORE::CDataNode targets;
            ASSERT_TRUE( client.SendCommandAndWait( "Target.getTargets", GUCEF_NULL, targets ) );
            CORE::CString targetId;
            const CORE::CDataNode* infos = targets.Find( "targetInfos" );
            ASSERT_TRUE( GUCEF_NULL != infos );
            for ( CORE::CDataNode::const_iterator i = infos->ConstBegin(); i != infos->ConstEnd(); ++i )
                if ( (*i)->GetAttributeValue( "type" ).AsString() == "page" )
                    targetId = (*i)->GetAttributeValue( "targetId" ).AsString();
            ASSERT_FALSE( targetId.IsNULLOrEmpty() );

            CORE::CString sessionId;
            ASSERT_TRUE( client.AttachToTarget( targetId, sessionId ) );
            client.SetSessionId( sessionId );
            ASSERT_TRUE( client.PageNavigate( "http://127.0.0.1:" + CORE::UInt32ToString( TEST_SITE_PORT ) + "/" ) );
            client.SetSessionId( CORE::CString::Empty );

            // the tab is watched from the moment it shows the site on. The page waits a moment before it does its requests
            for ( UInt32 i=0; i<500 && recorder.GetSessionCount() == 0; ++i )
            {
                Pump( 10 );
                recorder.Process();
            }
            ASSERT_TRUE( recorder.GetSessionCount() >= 1 );

            const CORE::CString base = "http://127.0.0.1:" + CORE::UInt32ToString( TEST_SITE_PORT );
            bool done = false;
            for ( UInt32 i=0; i<2000 && !done; ++i )
            {
                Pump( 10 );
                recorder.Process();
                const SEndpointEntry* facts = catalog.FindEntry( "GET", base + "/api/tree/1234567/person/7654321/facts" );
                done = GUCEF_NULL != facts && facts->requestCount >= 2 &&
                       GUCEF_NULL != catalog.FindEntry( "POST", base + "/api/tree/1234567/query" ) &&
                       GUCEF_NULL != catalog.FindEntry( "GET", base + "/media/55555555.jpg" );
            }
            for ( UInt32 i=0; i<50; ++i )
            {
                Pump( 10 );
                recorder.Process();
            }
            ASSERT_TRUE( done );

            // the two fact requests are one endpoint
            const SEndpointEntry* facts = catalog.FindEntry( "GET", base + "/api/tree/1234567/person/7654321/facts" );
            ASSERT_TRUE( GUCEF_NULL != facts );
            if ( GUCEF_NULL != facts )
            {
                ASSERT_TRUE( facts->requestCount == 2 );
                ASSERT_TRUE( facts->pathTemplate == "/api/tree/{n}/person/{n}/facts" );
                ASSERT_TRUE( facts->statusCounts.find( 200 ) != facts->statusCounts.end() );
                ASSERT_TRUE( facts->queryParams.find( "page" ) != facts->queryParams.end() && facts->queryParams.find( "page" )->second.size() == 2 );
                ASSERT_TRUE( facts->resourceTypes.count( "Fetch" ) == 1 );

                // and the response was saved and analyzed
                ASSERT_TRUE( facts->bodyFiles.size() >= 1 );
                if ( !facts->bodyFiles.empty() )
                    ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( layout.GetDiscoveryDir(), facts->bodyFiles[ 0 ].c_str() ) ) );
                ASSERT_TRUE( facts->hasShape );
                ASSERT_TRUE( facts->shape.paginationHints.size() >= 3 );
                ASSERT_TRUE( facts->shape.urlFields.size() >= 1 );
            }

            // the POST with its body and the headers the page set
            const SEndpointEntry* query = catalog.FindEntry( "POST", base + "/api/tree/1234567/query" );
            ASSERT_TRUE( GUCEF_NULL != query );
            if ( GUCEF_NULL != query )
            {
                ASSERT_TRUE( query->postDataExamples.size() == 1 && query->postDataExamples[ 0 ] == "{\"q\":\"x\"}" );
                ASSERT_TRUE( query->requestHeaderNames.count( "x-csrf-token" ) == 1 );
            }

            // images are noted but their content is not saved
            const SEndpointEntry* image = catalog.FindEntry( "GET", base + "/media/55555555.jpg" );
            ASSERT_TRUE( GUCEF_NULL != image );
            if ( GUCEF_NULL != image )
                ASSERT_TRUE( image->bodyFiles.empty() );
            ASSERT_TRUE( catalog.GetEndpointCount( CEndpointCatalog::KIND_MEDIA ) >= 1 );

            ASSERT_TRUE( catalog.GetTreeIds().count( "1234567" ) == 1 );

            // secrets stay out of everything that was written
            recorder.Stop();
            journal.Close();
            CORE::CString journalText;
            ASSERT_TRUE( CORE::LoadTextFileAsString( layout.GetDiscoveryJournalPath(), journalText ) );
            ASSERT_FALSE( journalText.HasSubstr( "secret123", true ) >= 0 );
            ASSERT_FALSE( catalog.RenderReport().HasSubstr( "secret123", true ) >= 0 );

            // and the catalog can be rebuilt from the journal
            CEndpointCatalog rebuilt;
            ASSERT_TRUE( CDiscoveryJournal::Load( layout, rebuilt ) == catalog.GetRequestCount() );
            ASSERT_TRUE( rebuilt.GetEntries().size() == catalog.GetEntries().size() );
            const SEndpointEntry* rebuiltFacts = rebuilt.FindEntry( "GET", base + "/api/tree/1234567/person/7654321/facts" );
            ASSERT_TRUE( GUCEF_NULL != rebuiltFacts && rebuiltFacts->hasShape );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A started browser can be shut down" )
        try
        {
            ASSERT_TRUE( browser.WasLaunched() );
            ASSERT_TRUE( browser.Shutdown() );
            ASSERT_FALSE( browser.WasLaunched() );
            for ( UInt32 i=0; i<100 && browser.IsAnswering( 300 ); ++i )
                Pump( 50 );
            ASSERT_FALSE( browser.IsAnswering( 500 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL AncestryDotComBackup discovery end to end TESTS COMPLETED" );
}

/*---------------------------------------------------------------------------*/
