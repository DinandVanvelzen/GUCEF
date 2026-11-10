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

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "AncestryBackup_CDataLayout.h"
#include "AncestryBackup_CUrlTemplate.h"
#include "AncestryBackup_CJsonShape.h"
#include "AncestryBackup_CEndpointCatalog.h"
#include "AncestryBackup_CDiscoveryJournal.h"
#include "AncestryBackup_CSettings.h"
#include "AncestryBackup_CChromeBrowser.h"
#include "AncestryBackup_CChromeNetworkRecorder.h"
#include "AncestryBackup_CGedcomReader.h"
#include "AncestryBackup_CGedcomSummary.h"
#include "AncestryBackup_CGedcomFile.h"
#include "AncestryBackup_CGrampsText.h"
#include "AncestryBackup_CGrampsDate.h"
#include "AncestryBackup_CGrampsExporter.h"
#include "AncestryBackup_CCommands.h"
#include "AncestryBackup_CExportFinder.h"
#include "AncestryBackup_CZipExtractor.h"
#include "AncestryBackup_CRequestPolicy.h"
#include "AncestryBackup_CRequestBudget.h"
#include "AncestryBackup_CChangeFeed.h"
#include "AncestryBackup_CMediaInventory.h"
#include "AncestryBackup_CDnaMatches.h"
#include "gucefCORE_CDateTime.h"

#include <algorithm>
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
#include <sys/utime.h>
#define GUCEF_UTIME _utime
#define GUCEF_UTIMBUF _utimbuf
#else
#include <utime.h>
#define GUCEF_UTIME utime
#define GUCEF_UTIMBUF utimbuf
#endif
#include <cstring>
#include <cstdlib>

#include "TestToolLogic.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::ANCESTRYBACKUP;

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
JsonAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const char* part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------*/

static SObservedRequest
MakeObservation( UInt32 sequence, const char* method, const char* url, const char* type, Int32 status, const char* mime, UInt64 size )
{
    SObservedRequest o;
    o.sequence = sequence;
    o.timestamp = 100.0 + sequence;
    o.method = method;
    o.url = url;
    o.resourceType = type;
    o.status = status;
    o.mimeType = mime;
    o.encodedDataLength = size;
    o.requestHeaders[ "accept" ] = "application/json";
    o.requestHeaders[ "cookie" ] = "<not recorded>";
    return o;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformDataLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The data layout names the directories of a tree" )
        try
        {
            CDataLayout layout( "/backup", "1234" );
            ASSERT_TRUE( layout.IsValid() );
            ASSERT_TRUE( layout.GetTreeDir() == CORE::CombinePath( "/backup", "1234" ) );
            ASSERT_TRUE( layout.GetBrowserProfileDir() == CORE::CombinePath( layout.GetTreeDir(), "browser-profile" ) );
            ASSERT_TRUE( layout.GetDiscoveryJournalPath() == CORE::CombinePath( CORE::CombinePath( layout.GetTreeDir(), "discovery" ), "requests.jsonl" ) );
            ASSERT_TRUE( layout.GetEndpointsReportPath() == CORE::CombinePath( CORE::CombinePath( layout.GetTreeDir(), "discovery" ), "endpoints.md" ) );
            ASSERT_TRUE( layout.GetBaselineGedcomPath() == CORE::CombinePath( layout.GetTreeDir(), "baseline.ged" ) );
            ASSERT_FALSE( CDataLayout().IsValid() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A tree id cannot escape the data directory" )
        try
        {
            ASSERT_TRUE( CDataLayout::IsValidTreeId( "123456789" ) );
            ASSERT_TRUE( CDataLayout::IsValidTreeId( "my-tree_1.0" ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "" ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "." ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( ".." ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "../other" ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "a/b" ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "a\\b" ) );
            ASSERT_FALSE( CDataLayout::IsValidTreeId( "a b" ) );
            ASSERT_FALSE( CDataLayout( "/backup", "../x" ).IsValid() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformUrlTemplateTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Identifiers in a path become placeholders" )
        try
        {
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/api/tree/12345678/person/987654321/facts" ) == "/api/tree/{n}/person/{n}/facts" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/media/3f2504e0-4f89-11d3-9a0c-0305e82c3301/original.jpg" ) == "/media/{guid}/original.jpg" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/media/3f2504e04f8911d39a0c0305e82c3301" ) == "/media/{guid}" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/img/12345678.jpg" ) == "/img/{n}.jpg" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/a/9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08" ) == "/a/{hex}" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/p/Ab3dE5gH7jK9" ) == "/p/{id}" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/rec/1030-7488" ) == "/rec/{n}" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Words and short numbers in a path stay" )
        try
        {
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/v1/people/search" ) == "/v1/people/search" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/api/2/x" ) == "/api/2/x" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/" ) == "/" );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CUrlTemplate::TemplatePath( "/static/main.min.js" ) == "/static/main.min.js" );
            ASSERT_FALSE( CUrlTemplate::LooksLikeId( "people" ) );
            ASSERT_TRUE( CUrlTemplate::LooksLikeId( "123456" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A URL is split in its parts" )
        try
        {
            SUrlTemplate parsed;
            ASSERT_TRUE( CUrlTemplate::Parse( "https://User@WWW.Ancestry.com:8443/a/12345/b?x=1&y=&z#frag", parsed ) );
            ASSERT_TRUE( parsed.scheme == "https" );
            ASSERT_TRUE( parsed.host == "www.ancestry.com:8443" );
            ASSERT_TRUE( parsed.path == "/a/12345/b" );
            ASSERT_TRUE( parsed.pathTemplate == "/a/{n}/b" );
            ASSERT_TRUE( parsed.GetKey() == "www.ancestry.com:8443/a/{n}/b" );
            ASSERT_TRUE( parsed.queryParams.size() == 3 );
            ASSERT_TRUE( parsed.queryParams[ 0 ].first == "x" && parsed.queryParams[ 0 ].second == "1" );
            ASSERT_TRUE( parsed.queryParams[ 1 ].first == "y" && parsed.queryParams[ 1 ].second.IsNULLOrEmpty() );
            ASSERT_TRUE( parsed.queryParams[ 2 ].first == "z" );

            ASSERT_TRUE( CUrlTemplate::Parse( "http://host", parsed ) );
            ASSERT_TRUE( parsed.host == "host" && parsed.path.IsNULLOrEmpty() );
            ASSERT_TRUE( CUrlTemplate::Parse( "http://host?a=b", parsed ) );
            ASSERT_TRUE( parsed.host == "host" && parsed.queryParams.size() == 1 );

            ASSERT_FALSE( CUrlTemplate::Parse( "data:text/plain,hi", parsed ) );
            ASSERT_FALSE( CUrlTemplate::Parse( "/relative/path", parsed ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The id following a path segment is found" )
        try
        {
            ASSERT_TRUE( CUrlTemplate::FindIdAfterSegment( "/tree/123456/person/9", "tree" ) == "123456" );
            ASSERT_TRUE( CUrlTemplate::FindIdAfterSegment( "/api/trees/778899/x", "trees" ) == "778899" );
            ASSERT_TRUE( CUrlTemplate::FindIdAfterSegment( "/tree/overview/123456", "tree" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CUrlTemplate::FindIdAfterSegment( "/nothing/here", "tree" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CUrlTemplate::FindIdAfterSegment( "/a/tree", "tree" ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Percent decoding and JSON escaping" )
        try
        {
            ASSERT_TRUE( CUrlTemplate::PercentDecode( "a%20b+c%2Fd" ) == "a b c/d" );
            ASSERT_TRUE( CUrlTemplate::PercentDecode( "100%" ) == "100%" );
            ASSERT_TRUE( CUrlTemplate::PercentDecode( "%zz" ) == "%zz" );
            ASSERT_TRUE( CUrlTemplate::EscapeJson( "say \"hi\"\n\\" ) == "say \\\"hi\\\"\\n\\\\" );
            ASSERT_TRUE( CUrlTemplate::EscapeJson( "tab\t" ) == "tab\\t" );
            ASSERT_TRUE( CUrlTemplate::EscapeJson( "\x01" ) == "\\u0001" );
            ASSERT_TRUE( CUrlTemplate::EscapeJson( "caf\xC3\xA9" ) == "caf\xC3\xA9" );
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
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The command line is split in the command and the settings" )
        try
        {
            std::vector< std::string > args;
            args.push_back( "discover" ); args.push_back( "treeId=1234" ); args.push_back( "--headless" ); args.push_back( "--DebugPort=9333" );
            std::string command;
            std::map< std::string, std::string > options;
            CORE::CString problem;
            ASSERT_TRUE( CSettings::ParseCommandLine( args, command, options, problem ) );
            ASSERT_TRUE( command == "discover" );
            ASSERT_TRUE( options[ "treeid" ] == "1234" );
            ASSERT_TRUE( options.find( "headless" ) != options.end() && options[ "headless" ].empty() );
            ASSERT_TRUE( options[ "debugport" ] == "9333" );

            args.push_back( "login" );
            ASSERT_FALSE( CSettings::ParseCommandLine( args, command, options, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );

            args.clear();
            ASSERT_TRUE( CSettings::ParseCommandLine( args, command, options, problem ) );
            ASSERT_TRUE( command.empty() && options.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Settings have defaults and take values" )
        try
        {
            CSettings settings;
            ASSERT_TRUE( settings.debugPort == 9222 && settings.treeId == "default" && settings.cookieDomain == "ancestry.com" );
            ASSERT_TRUE( settings.attachToRunning && settings.chromeUserDataDirs.empty() );
            ASSERT_TRUE( settings.exportDirs.empty() && settings.exportMaxAgeDays == 7 );
            ASSERT_TRUE( settings.hostFilter.size() == 1 && settings.hostFilter[ 0 ] == "ancestry" );
            ASSERT_FALSE( settings.headless );

            std::map< std::string, std::string > options;
            options[ "debugport" ] = "9333";
            options[ "headless" ] = "";
            options[ "treeid" ] = "5566";
            options[ "hostfilter" ] = "ancestry; mediasvc , 127.0.0.1";
            options[ "maxbodymegabytes" ] = "2";
            options[ "discoverseconds" ] = "30";
            options[ "attachtorunning" ] = "false";
            options[ "chromeuserdatadir" ] = "/a/b;/c/d";
            options[ "gedcom" ] = "/exports/tree.ged";
            options[ "exportdirs" ] = "/a;/b";
            options[ "exportmaxagedays" ] = "14";
            CORE::CString problem;
            ASSERT_TRUE( settings.Load( options, problem ) );
            ASSERT_TRUE( settings.debugPort == 9333 && settings.headless && settings.treeId == "5566" );
            ASSERT_TRUE( settings.hostFilter.size() == 3 && settings.hostFilter[ 1 ] == "mediasvc" && settings.hostFilter[ 2 ] == "127.0.0.1" );
            ASSERT_TRUE( settings.maxBodyBytes == 2ULL * 1024 * 1024 && settings.discoverSeconds == 30 );
            ASSERT_TRUE( settings.GetLayout().GetTreeId() == "5566" );
            ASSERT_FALSE( settings.attachToRunning );
            ASSERT_TRUE( settings.baselineGedcom == "/exports/tree.ged" );
            ASSERT_TRUE( settings.exportDirs.size() == 2 && settings.exportMaxAgeDays == 14 );
            ASSERT_TRUE( settings.chromeUserDataDirs.size() == 2 && settings.chromeUserDataDirs[ 1 ] == "/c/d" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Bad settings are refused with an explanation" )
        try
        {
            CORE::CString problem;
            std::map< std::string, std::string > options;
            options[ "debugport" ] = "0";
            ASSERT_FALSE( CSettings().Load( options, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );

            options.clear();
            options[ "debugport" ] = "99999";
            ASSERT_FALSE( CSettings().Load( options, problem ) );

            options.clear();
            options[ "treeid" ] = "../escape";
            problem = CORE::CString::Empty;
            ASSERT_FALSE( CSettings().Load( options, problem ) );
            ASSERT_FALSE( problem.IsNULLOrEmpty() );

            ASSERT_TRUE( CSettings::SplitList( "" ).empty() );
            ASSERT_TRUE( CSettings::SplitList( " a ;; b," ).size() == 2 );
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
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Requests of the same kind become one endpoint" )
        try
        {
            CEndpointCatalog catalog;
            ASSERT_TRUE( catalog.Add( MakeObservation( 1, "GET", "https://www.ancestry.com/api/tree/111111/person/222222/facts?page=1", "XHR", 200, "application/json", 1000 ) ) );
            ASSERT_FALSE( catalog.Add( MakeObservation( 2, "GET", "https://www.ancestry.com/api/tree/111111/person/333333/facts?page=2&size=25", "XHR", 200, "application/json", 3000 ) ) );
            ASSERT_TRUE( catalog.Add( MakeObservation( 3, "GET", "https://mediasvc.ancestry.com/image/99999999", "Image", 200, "image/jpeg", 50000 ) ) );
            ASSERT_FALSE( catalog.Add( MakeObservation( 4, "GET", "https://www.ancestry.com/static/app.js", "Script", 200, "text/javascript", 10 ) ) );
            SObservedRequest post = MakeObservation( 5, "POST", "https://www.ancestry.com/graphql", "Fetch", 200, "application/json", 10 );
            post.postData = "{\"query\":\"{ person }\"}";
            ASSERT_TRUE( catalog.Add( post ) );

            ASSERT_TRUE( catalog.GetRequestCount() == 5 );
            ASSERT_TRUE( catalog.GetEndpointCount( CEndpointCatalog::KIND_DATA ) == 2 );
            ASSERT_TRUE( catalog.GetEndpointCount( CEndpointCatalog::KIND_MEDIA ) == 1 );
            ASSERT_TRUE( catalog.GetEndpointCount( CEndpointCatalog::KIND_PAGE ) == 0 );
            ASSERT_TRUE( catalog.GetEntries().size() == 3 );

            const SEndpointEntry* facts = catalog.FindEntry( "GET", "https://www.ancestry.com/api/tree/99/person/1000000/facts" );
            ASSERT_TRUE( GUCEF_NULL == facts );   // 99 is too short to be seen as an id, so this is a different endpoint
            facts = catalog.FindEntry( "GET", "https://www.ancestry.com/api/tree/555555/person/666666/facts?x=1" );
            ASSERT_TRUE( GUCEF_NULL != facts );
            if ( GUCEF_NULL != facts )
            {
                ASSERT_TRUE( facts->requestCount == 2 );
                ASSERT_TRUE( facts->totalBytes == 4000 );
                ASSERT_TRUE( facts->queryParams.size() == 2 );
                ASSERT_TRUE( facts->queryParams.find( "page" ) != facts->queryParams.end() && facts->queryParams.find( "page" )->second.size() == 2 );
                ASSERT_TRUE( facts->requestHeaderNames.count( "accept" ) == 1 );
                ASSERT_TRUE( facts->exampleUrls.size() == 2 );
            }

            ASSERT_TRUE( catalog.GetTreeIds().count( "111111" ) == 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sign in and account requests are recognized as sensitive" )
        try
        {
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/account/password/verify" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/account/security/step-up/api/send-email-code" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/secure/signin?returnUrl=x" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://example.com/oauth2/Token" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://example.com/api/Login" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/api/dna/matches/ABC/matches" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/api/treesdna/thrulines" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/discoveryui-geneticfamilyservice/api/person/1/family" ) );
            ASSERT_FALSE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/api/treesui-list/trees/86165115/persons?page=1" ) );
            ASSERT_FALSE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/family-tree/person/facts/user/x/tree/1/person/2/factsglue" ) );
            ASSERT_FALSE( CEndpointCatalog::IsSensitiveUrl( "https://www.ancestry.com/api/media/retrieval/v2/image/namespaces/1/media/a.jpg?token=notinpath" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What was sent to a sign in page never reaches the report" )
        try
        {
            CEndpointCatalog catalog;
            SObservedRequest verify = MakeObservation( 1, "POST", "https://www.ancestry.com/account/password/verify", "Fetch", 200, "application/json", 10 );
            verify.postData = "{\"password\":\"hunter2-NOT-REAL\"}";
            verify.bodyFile = "bodies/000001.json";
            catalog.Add( verify );
            const CORE::CString report = catalog.RenderReport();
            ASSERT_TRUE( Contains( report, "account/password/verify" ) );        // that it happened is noted
            ASSERT_FALSE( Contains( report, "hunter2-NOT-REAL" ) );
            ASSERT_FALSE( Contains( report, "bodies/000001.json" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: DNA requests are kept out of the report unless DNA recording is on, credentials never" )
        try
        {
            CEndpointCatalog off;
            CEndpointCatalog on;
            on.SetRecordDna( true );
            const std::string dna = "https://www.ancestry.com/api/dna/matches/ABC/matches";
            const std::string password = "https://www.ancestry.com/account/password/verify";
            ASSERT_TRUE( off.IsExcluded( dna ) && !on.IsExcluded( dna ) );
            ASSERT_TRUE( off.IsExcluded( password ) && on.IsExcluded( password ) );
            ASSERT_FALSE( on.IsExcluded( "https://www.ancestry.com/api/treesui-list/trees/86165115/persons" ) );

            SObservedRequest match = MakeObservation( 1, "GET", dna.c_str(), "Fetch", 200, "application/json", 100 );
            match.bodyFile = "bodies/000001.json";
            off.Add( match );
            on.Add( match );
            ASSERT_FALSE( Contains( off.RenderReport(), "bodies/000001.json" ) );
            ASSERT_TRUE( Contains( on.RenderReport(), "bodies/000001.json" ) );

            CSettings settings;
            ASSERT_FALSE( settings.recordDna );
            std::map< std::string, std::string > options;
            options[ "recorddna" ] = "";
            CORE::CString problem;
            ASSERT_TRUE( settings.Load( options, problem ) && settings.recordDna );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Responses are classified by their type" )
        try
        {
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "XHR", 200, "text/plain", 1 ) ) == CEndpointCatalog::KIND_DATA );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "Document", 200, "application/json", 1 ) ) == CEndpointCatalog::KIND_DATA );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "Document", 200, "text/html", 1 ) ) == CEndpointCatalog::KIND_PAGE );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "Other", 200, "application/pdf", 1 ) ) == CEndpointCatalog::KIND_MEDIA );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "Media", 206, "video/mp4", 1 ) ) == CEndpointCatalog::KIND_MEDIA );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "GET", "http://a/", "Stylesheet", 200, "text/css", 1 ) ) == CEndpointCatalog::KIND_OTHER );
            ASSERT_TRUE( CEndpointCatalog::Classify( MakeObservation( 1, "OPTIONS", "http://a/", "Preflight", 204, "", 1 ) ) == CEndpointCatalog::KIND_OTHER );
            ASSERT_TRUE( CEndpointCatalog::IsSecretHeader( "cookie" ) && CEndpointCatalog::IsSecretHeader( "authorization" ) );
            ASSERT_TRUE( CEndpointCatalog::IsSecretHeader( "x-csrf-token" ) && CEndpointCatalog::IsSecretHeader( "x-xsrf-token" ) && CEndpointCatalog::IsSecretHeader( "x-auth-token" ) );
            ASSERT_FALSE( CEndpointCatalog::IsSecretHeader( "accept" ) || CEndpointCatalog::IsSecretHeader( "ancestry-context-ube" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The report describes endpoints and keeps secrets out" )
        try
        {
            CEndpointCatalog catalog;
            SObservedRequest first = MakeObservation( 1, "GET", "https://www.ancestry.com/api/tree/111111/person/222222/facts?page=1", "XHR", 200, "application/json", 1000 );
            first.requestHeaders[ "authorization" ] = "<not recorded>";
            catalog.Add( first );
            catalog.Add( MakeObservation( 2, "GET", "https://www.ancestry.com/api/tree/111111/person/333333/facts?page=2", "XHR", 404, "application/json", 3000 ) );
            catalog.Add( MakeObservation( 3, "GET", "https://mediasvc.ancestry.com/image/99999999", "Image", 200, "image/jpeg", 50000 ) );
            catalog.Add( MakeObservation( 4, "GET", "https://www.ancestry.com/static/app.js", "Script", 200, "text/javascript", 10 ) );

            const CORE::CString report = catalog.RenderReport();
            ASSERT_TRUE( Contains( report, "# Endpoints observed during discovery" ) );
            ASSERT_TRUE( Contains( report, "### GET www.ancestry.com/api/tree/{n}/person/{n}/facts" ) );
            ASSERT_TRUE( Contains( report, "200 (1), 404 (1)" ) );
            ASSERT_TRUE( Contains( report, "`page` e.g. `1`, `2`" ) );
            ASSERT_TRUE( Contains( report, "### GET mediasvc.ancestry.com/image/{n}" ) );
            ASSERT_TRUE( Contains( report, "Tree ids seen in urls: 111111" ) );
            ASSERT_TRUE( Contains( report, "Script: 1" ) );
            ASSERT_TRUE( Contains( report, "| mediasvc.ancestry.com | 1 |" ) );
            ASSERT_TRUE( Contains( report, "accept" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformJsonTests( void )
{GUCEF_TRACE;

    if ( !JsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup tests of JSON handling skipped: the json codec is not available" );
        return;
    }

    static const char* const document = "{\"total\":120,\"page\":1,\"next\":\"/api/x?page=2\",\"hasMore\":true,"
                                        "\"items\":[{\"id\":1,\"name\":\"Secret Person Name\",\"media\":\"https://img.example.com/1.jpg\",\"score\":1.5},"
                                        "{\"id\":2,\"name\":\"Another Name\",\"tags\":[\"a\",\"b\"]}],\"meta\":{\"ok\":true,\"owner\":null}}";

    GUCEF_TESTFW_TESTCASE( "Test: The shape of a document shows the fields and not the data" )
        try
        {
            CORE::CDataNode root;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( document, root ) );
            SJsonShape shape;
            CJsonShape::Describe( root, shape );
            ASSERT_FALSE( shape.lines.empty() );

            CORE::CString all;
            for ( size_t i=0; i<shape.lines.size(); ++i )
                all += shape.lines[ i ] + "\n";

            ASSERT_TRUE( Contains( all, "items: array[2]" ) );
            ASSERT_TRUE( Contains( all, "items[].name: string" ) );
            ASSERT_TRUE( Contains( all, "items[].id: integer" ) );
            ASSERT_TRUE( Contains( all, "items[].tags: array[2]" ) );
            ASSERT_TRUE( Contains( all, "items[].tags[]: string" ) );
            ASSERT_TRUE( Contains( all, "meta: object" ) );
            ASSERT_TRUE( Contains( all, "meta.ok: boolean" ) );
            ASSERT_TRUE( Contains( all, "total: integer = 120" ) );
            ASSERT_TRUE( Contains( all, "next: string = /api/x?page=2" ) );

            // personal data is not repeated in the report
            ASSERT_FALSE( Contains( all, "Secret Person Name" ) );
            ASSERT_FALSE( Contains( all, "Another Name" ) );

            ASSERT_TRUE( shape.paginationHints.size() >= 4 );   // total, page, next, hasMore
            ASSERT_TRUE( shape.urlFields.size() == 2 );         // next and the media url
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The shape of an array document and the line limit" )
        try
        {
            CORE::CDataNode root;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "[{\"a\":1},{\"b\":\"x\"}]", root ) );
            SJsonShape shape;
            CJsonShape::Describe( root, shape );
            ASSERT_TRUE( shape.lines.size() == 3 );
            ASSERT_TRUE( shape.lines[ 0 ] == "(root): array[2]" );

            CORE::CDataNode wide;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( document, wide ) );
            CJsonShape::Describe( wide, shape, 5, 4 );
            ASSERT_TRUE( shape.lines.size() == 4 );
            ASSERT_TRUE( shape.truncated );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The journal rebuilds the catalog, including shapes of saved responses" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Journal" );
            CORE::CreateDirs( dir );
            CDataLayout layout( dir, "9988" );
            ASSERT_TRUE( layout.CreateDiscoveryDirs() );
            CORE::DeleteFile( layout.GetDiscoveryJournalPath() );

            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( layout.GetDiscoveryDir(), "bodies/000001.json" ), document ) );

            SObservedRequest first = MakeObservation( 1, "GET", "https://www.ancestry.com/api/tree/111111/person/222222/facts?page=1", "XHR", 200, "application/json", 1000 );
            first.bodyFile = "bodies/000001.json";
            first.postData = "with \"quotes\" and \\ backslash\nand newline \xC3\xA9";
            SObservedRequest second = MakeObservation( 7, "GET", "https://mediasvc.ancestry.com/image/99999999", "Image", 200, "image/jpeg", 50000 );

            {
                CDiscoveryJournal journal;
                ASSERT_TRUE( journal.Open( layout ) );
                ASSERT_TRUE( journal.GetNextSequence() == 1 );
                ASSERT_TRUE( journal.Append( first ) );
                ASSERT_TRUE( journal.Append( second ) );
                ASSERT_TRUE( journal.GetNextSequence() == 8 );
            }
            {
                // a second run goes on after the first
                CDiscoveryJournal journal;
                ASSERT_TRUE( journal.Open( layout ) );
                ASSERT_TRUE( journal.GetNextSequence() == 8 );
            }

            CEndpointCatalog catalog;
            UInt32 next = 0;
            ASSERT_TRUE( CDiscoveryJournal::Load( layout, catalog, &next ) == 2 );
            ASSERT_TRUE( next == 8 );
            ASSERT_TRUE( catalog.GetEntries().size() == 2 );
            const SEndpointEntry* facts = catalog.FindEntry( "GET", first.url.c_str() );
            ASSERT_TRUE( GUCEF_NULL != facts );
            if ( GUCEF_NULL != facts )
            {
                ASSERT_TRUE( facts->hasShape );
                ASSERT_TRUE( facts->bodyFiles.size() == 1 && facts->bodyFiles[ 0 ] == "bodies/000001.json" );
                ASSERT_TRUE( facts->postDataExamples.size() == 1 && facts->postDataExamples[ 0 ] == first.postData );
                ASSERT_TRUE( facts->requestHeaderNames.count( "cookie" ) == 1 );
            }

            // one more line that is damaged is skipped, the rest still loads
            CORE::CFileAccess file( layout.GetDiscoveryJournalPath(), "ab" );
            file.Write( "this is not json\n", 1, 17 );
            file.Close();
            CEndpointCatalog again;
            ASSERT_TRUE( CDiscoveryJournal::Load( layout, again ) == 2 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static const char* const SampleGedcom =
    "0 HEAD\n"
    "1 SOUR Ancestry.com Family Trees\n"
    "2 NAME Ancestry.com Member Trees\n"
    "2 VERS 2025.01\n"
    "1 GEDC\n"
    "2 VERS 5.5.1\n"
    "2 FORM LINEAGE-LINKED\n"
    "1 CHAR UTF-8\n"
    "1 DATE 15 OCT 2026\n"
    "0 @I1@ INDI\n"
    "1 NAME Jan /Test/\n"
    "1 SEX M\n"
    "1 BIRT\n"
    "2 DATE 1 JAN 1900\n"
    "2 SOUR @S1@\n"
    "3 _APID 1,9289::10143733\n"
    "1 NOTE A note that goes\n"
    "2 CONT on a second line\n"
    "1 OBJE\n"
    "2 FILE https://example.com/media/1.jpg\n"
    "1 FAMS @F1@\n"
    "0 @I2@ INDI\n"
    "1 NAME Marie /Test/\n"
    "1 FAMS @F1@\n"
    "0 @F1@ FAM\n"
    "1 HUSB @I1@\n"
    "1 WIFE @I2@\n"
    "1 MARR\n"
    "0 @S1@ SOUR\n"
    "1 TITL Census 1900\n"
    "0 @M1@ OBJE\n"
    "1 FILE photo.jpg\n"
    "0 TRLR\n";

class CLineCollector : public CIGedcomHandler
{
    public:
    virtual void OnLine( const SGedcomLine& line ) GUCEF_VIRTUAL_OVERRIDE
    {
        lines.push_back( CORE::ToString( line.level ) + "|" + CORE::CString( line.xref ? line.xref : "", line.xrefLength ) + "|" + CORE::CString( line.tag, line.tagLength ) + "|"
                         + CORE::CString( line.value ? line.value : "", line.valueLength ) + "|" + CORE::ToString( line.lineNumber ) );
    }
    virtual void OnMalformedLine( UInt32 lineNumber ) GUCEF_VIRTUAL_OVERRIDE { malformed.push_back( lineNumber ); }
    GUCEF::vector< CORE::CString > lines;
    GUCEF::vector< UInt32 > malformed;
};

/*-------------------------------------------------------------------------*/

static void
PerformGedcomTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The lines of a GEDCOM file are split in level, id, tag and value" )
        try
        {
            const CORE::CString text = "0 HEAD\n0 @I1@ INDI\n1 NAME Jan /Test/\n2 CONT   two spaces\n1 BIRT\n";
            CLineCollector collector;
            ASSERT_TRUE( CGedcomReader::Parse( text.C_String(), text.ByteSize() - 1, collector ) == 5 );    // ByteSize includes the null terminator
            ASSERT_TRUE( collector.malformed.empty() );
            ASSERT_TRUE( collector.lines[ 0 ] == "0||HEAD||1" );
            ASSERT_TRUE( collector.lines[ 1 ] == "0|I1|INDI||2" );
            ASSERT_TRUE( collector.lines[ 2 ] == "1||NAME|Jan /Test/|3" );
            ASSERT_TRUE( collector.lines[ 3 ] == "2||CONT|  two spaces|4" );     // only the one separating space goes
            ASSERT_TRUE( collector.lines[ 4 ] == "1||BIRT||5" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: All line ends and a byte order mark are accepted" )
        try
        {
            const CORE::CString crlf = "\xEF\xBB\xBF" "0 HEAD\r\n1 CHAR UTF-8\r\n0 TRLR\r\n";
            CLineCollector a;
            ASSERT_TRUE( CGedcomReader::Parse( crlf.C_String(), crlf.ByteSize() - 1, a ) == 3 );    // ByteSize includes the null terminator
            ASSERT_TRUE( a.lines[ 0 ] == "0||HEAD||1" && a.lines[ 2 ] == "0||TRLR||3" );

            const CORE::CString cr = "0 HEAD\r1 CHAR UTF-8\r0 TRLR";
            CLineCollector b;
            ASSERT_TRUE( CGedcomReader::Parse( cr.C_String(), cr.ByteSize() - 1, b ) == 3 );
            ASSERT_TRUE( b.lines[ 2 ] == "0||TRLR||3" );

            const CORE::CString blanks = "\n\n0 HEAD\n\n\n0 TRLR\n";
            CLineCollector c;
            ASSERT_TRUE( CGedcomReader::Parse( blanks.C_String(), blanks.ByteSize() - 1, c ) == 2 );
            ASSERT_TRUE( c.malformed.empty() );

            CLineCollector d;
            ASSERT_TRUE( CGedcomReader::Parse( "", 0, d ) == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Lines that are not GEDCOM are reported and skipped" )
        try
        {
            const CORE::CString text = "0 HEAD\nthis is not gedcom\n1 \n0 @I1 INDI\n1x NAME\n0 TRLR\n";
            CLineCollector collector;
            ASSERT_TRUE( CGedcomReader::Parse( text.C_String(), text.ByteSize() - 1, collector ) == 2 );
            ASSERT_TRUE( collector.malformed.size() == 4 );
            ASSERT_TRUE( collector.malformed[ 0 ] == 2 && collector.malformed[ 1 ] == 3 && collector.malformed[ 2 ] == 4 && collector.malformed[ 3 ] == 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The summary counts what the file holds" )
        try
        {
            CGedcomSummarizer summarizer;
            CGedcomReader::Parse( SampleGedcom, strlen( SampleGedcom ), summarizer );
            const SGedcomSummary& s = summarizer.GetSummary();
            ASSERT_TRUE( s.hasHeader && s.hasTrailer && s.malformedLines == 0 );
            ASSERT_TRUE( s.sourceSystem == "Ancestry.com Family Trees" && s.sourceName == "Ancestry.com Member Trees" && s.sourceVersion == "2025.01" );
            ASSERT_TRUE( s.gedcomVersion == "5.5.1" && s.gedcomForm == "LINEAGE-LINKED" && s.characterSet == "UTF-8" && s.headerDate == "15 OCT 2026" );
            ASSERT_TRUE( s.individuals == 2 && s.families == 1 && s.sources == 1 && s.mediaObjects == 1 && s.notes == 0 );
            ASSERT_TRUE( s.individualsWithNotes == 1 );
            ASSERT_TRUE( s.citations == 1 && s.ancestryIds == 1 );
            ASSERT_TRUE( s.mediaLinks == 1 && s.mediaFiles == 2 && s.mediaFilesThatAreUrls == 1 );
            ASSERT_TRUE( s.mediaFileNames.size() == 2 && s.mediaFileNames[ 0 ] == "https://example.com/media/1.jpg" && s.mediaFileNames[ 1 ] == "photo.jpg" );
            ASSERT_TRUE( s.individualIds.size() == 2 && s.individualIds[ 0 ] == "I1" && s.individualIds[ 1 ] == "I2" );
            ASSERT_TRUE( s.individualTags.find( "NAME" )->second == 2 && s.individualTags.find( "BIRT" )->second == 1 );
            ASSERT_TRUE( s.individualTags.find( "DATE" ) == s.individualTags.end() );     // below an event, not directly below the individual
            ASSERT_TRUE( s.familyTags.find( "MARR" )->second == 1 );
            ASSERT_TRUE( s.lines == 33 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The summary text says what the file holds and what it cannot hold" )
        try
        {
            CGedcomSummarizer summarizer;
            CGedcomReader::Parse( SampleGedcom, strlen( SampleGedcom ), summarizer );
            const CORE::CString text = CGedcomSummarizer::Render( summarizer.GetSummary() );
            ASSERT_TRUE( Contains( text, "individuals: 2" ) );
            ASSERT_TRUE( Contains( text, "Ancestry.com Family Trees" ) );
            ASSERT_TRUE( Contains( text, "does not hold comments, stories or record transcriptions" ) );
            ASSERT_FALSE( Contains( text, "no TRLR" ) );

            CGedcomSummarizer cut;
            const char* const broken = "0 HEAD\n1 CHAR ANSEL\n0 @I1@ INDI\n";
            CGedcomReader::Parse( broken, strlen( broken ), cut );
            const CORE::CString cutText = CGedcomSummarizer::Render( cut.GetSummary() );
            ASSERT_TRUE( Contains( cutText, "no TRLR" ) );
            ASSERT_TRUE( Contains( cutText, "ANSEL" ) );
            ASSERT_TRUE( Contains( cutText, "does not name any media files" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The baseline command takes the GEDCOM file in and is optional" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Baseline" );
            CORE::CreateDirs( dir );
            const CORE::CString gedcom = CORE::CombinePath( dir, "export.ged" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( gedcom, SampleGedcom ) );

            CSettings settings;
            settings.dataDir = CORE::CombinePath( dir, "data" );
            settings.treeId = "7788";
            CDataLayout layout = settings.GetLayout();
            CORE::DeleteFile( layout.GetBaselineGedcomPath() );

            // nothing given and nothing imported: a clear failure, not a crash
            ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_USAGE );

            // a file that is not there
            settings.baselineGedcom = CORE::CombinePath( dir, "missing.ged" );
            ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_USAGE );

            // a file that is not a gedcom file of a tree
            const CORE::CString nonsense = CORE::CombinePath( dir, "nonsense.ged" );
            CORE::WriteStringAsTextFile( nonsense, "hello\nworld\n" );
            settings.baselineGedcom = nonsense;
            ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_FAILED );
            ASSERT_FALSE( CORE::FileExists( layout.GetBaselineGedcomPath() ) );

            // the real thing
            settings.baselineGedcom = gedcom;
            ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_OK );
            ASSERT_TRUE( CORE::FileExists( layout.GetBaselineGedcomPath() ) );
            ASSERT_TRUE( CORE::FileSize( layout.GetBaselineGedcomPath() ) == strlen( SampleGedcom ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( layout.GetBaselineSnapshotsDir(), "export-" + CExportFinder::FormatTimestamp( CORE::GetFileModificationTime( gedcom ).ToUnixEpochBasedTicksInMillisecs() ) + ".ged" ) ) );
            CORE::CString summaryText;
            ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( layout.GetStateDir(), "baseline-summary.txt" ), summaryText ) );
            ASSERT_TRUE( Contains( summaryText, "individuals: 2" ) );
            CORE::CString ids;
            ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( layout.GetStateDir(), "baseline-individuals.txt" ), ids ) );
            ASSERT_TRUE( Contains( ids, "I1" ) && Contains( ids, "I2" ) );

            // later runs use the file that was imported
            settings.baselineGedcom = CORE::CString::Empty;
            ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_OK );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
SetAgeOfFile( const CORE::CString& path, UInt32 ageInDays )
{
    struct GUCEF_UTIMBUF times;
    times.actime = times.modtime = (time_t) ( CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() / 1000 ) - (time_t) ageInDays * 24 * 60 * 60;
    GUCEF_UTIME( path.C_String(), &times );
}

/*-------------------------------------------------------------------------*/

static void
PerformExportFinderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Exports are named by the time they were written" )
        try
        {
            // the downloads directory is a directory called Downloads, not the home directory
            const std::vector< CORE::CString > defaults = CExportFinder::GetDefaultDirs();
            for ( size_t i=0; i<defaults.size(); ++i )
                ASSERT_TRUE( CORE::DirExists( defaults[ i ] ) );
            ASSERT_TRUE( CExportFinder::FormatTimestamp( 0 ) == "19700101-000000" );
            ASSERT_TRUE( CExportFinder::FormatTimestamp( 1792106595000ULL ) == "20261015-232315" );
            ASSERT_TRUE( CExportFinder::FormatTimestamp( 1792106595999ULL ) == "20261015-232315" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only a recent GEDCOM file written by Ancestry is found" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Downloads" );
            CORE::CreateDirs( dir );
            const char* const names[] = { "old.ged", "other.ged", "new1.ged", "new2.GED", "Tree_export.zip", "notes.txt", "holiday.zip", GUCEF_NULL };
            for ( UInt32 i=0; names[ i ] != GUCEF_NULL; ++i )
                CORE::DeleteFile( CORE::CombinePath( dir, names[ i ] ) );

            const CORE::CString other = "0 HEAD\n1 SOUR SomeOtherProgram\n0 @I1@ INDI\n0 TRLR\n";
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "old.ged" ), SampleGedcom );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "other.ged" ), other );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "new1.ged" ), SampleGedcom );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "new2.GED" ), SampleGedcom );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "Tree_export.zip" ), "PK" );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "notes.txt" ), SampleGedcom );
            CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "holiday.zip" ), "PK" );
            SetAgeOfFile( CORE::CombinePath( dir, "old.ged" ), 30 );
            SetAgeOfFile( CORE::CombinePath( dir, "other.ged" ), 1 );
            SetAgeOfFile( CORE::CombinePath( dir, "new1.ged" ), 3 );
            SetAgeOfFile( CORE::CombinePath( dir, "new2.GED" ), 1 );

            ASSERT_TRUE( CExportFinder::LooksLikeAncestryExport( CORE::CombinePath( dir, "new1.ged" ) ) );
            ASSERT_FALSE( CExportFinder::LooksLikeAncestryExport( CORE::CombinePath( dir, "other.ged" ) ) );
            ASSERT_FALSE( CExportFinder::LooksLikeAncestryExport( CORE::CombinePath( dir, "missing.ged" ) ) );

            std::vector< CORE::CString > dirs;
            dirs.push_back( dir );
            dirs.push_back( CORE::CombinePath( dir, "no-such-directory" ) );
            std::vector< SExportCandidate > found;
            std::vector< CORE::CString > zips;
            const UInt64 now = CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
            CExportFinder::FindRecent( dirs, now, 7ULL * 24 * 60 * 60 * 1000, found, &zips );
            ASSERT_TRUE( found.size() == 2 );
            if ( found.size() == 2 )
            {
                ASSERT_TRUE( found[ 0 ].path == CORE::CombinePath( dir, "new2.GED" ) );      // newest first
                ASSERT_TRUE( found[ 1 ].path == CORE::CombinePath( dir, "new1.ged" ) );
                ASSERT_TRUE( found[ 0 ].sizeInBytes == strlen( SampleGedcom ) );
                ASSERT_TRUE( found[ 0 ].modifiedAtInMs > found[ 1 ].modifiedAtInMs );
            }
            ASSERT_TRUE( zips.size() == 1 && zips[ 0 ] == CORE::CombinePath( dir, "Tree_export.zip" ) );

            // a longer memory also finds the old one
            CExportFinder::FindRecent( dirs, now, 60ULL * 24 * 60 * 60 * 1000, found, GUCEF_NULL );
            ASSERT_TRUE( found.size() == 3 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The export command takes in the newest download once" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Downloads" );
            CSettings settings;
            settings.dataDir = CORE::CombinePath( CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_ExportCmd" ), "data" );
            settings.treeId = "5566";
            settings.exportDirs.push_back( dir.C_String() );
            const CDataLayout layout = settings.GetLayout();
            CORE::DeleteFile( layout.GetBaselineGedcomPath() );
            CORE::DeleteFile( CORE::CombinePath( layout.GetBaselineSnapshotsDir(), "export-" + CExportFinder::FormatTimestamp( CORE::GetFileModificationTime( CORE::CombinePath( dir, "new2.GED" ) ).ToUnixEpochBasedTicksInMillisecs() ) + ".ged" ) );

            ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_OK );
            ASSERT_TRUE( CORE::FileExists( layout.GetBaselineGedcomPath() ) );
            const CORE::CString snapshot = CORE::CombinePath( layout.GetBaselineSnapshotsDir(), "export-" + CExportFinder::FormatTimestamp( CORE::GetFileModificationTime( CORE::CombinePath( dir, "new2.GED" ) ).ToUnixEpochBasedTicksInMillisecs() ) + ".ged" );
            ASSERT_TRUE( CORE::FileExists( snapshot ) );
            ASSERT_TRUE( CORE::FileSize( snapshot ) == strlen( SampleGedcom ) );

            // the same download is not taken in twice
            SetAgeOfFile( layout.GetBaselineGedcomPath(), 5 );
            ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_OK );
            ASSERT_TRUE( CORE::GetFileModificationTime( layout.GetBaselineGedcomPath() ).ToUnixEpochBasedTicksInMillisecs() <
                         CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() - 24ULL * 60 * 60 * 1000 );

            // nothing recent: its own result, and nothing is changed
            settings.exportMaxAgeDays = 1;
            CORE::DeleteFile( CORE::CombinePath( dir, "new2.GED" ) );
            CORE::DeleteFile( CORE::CombinePath( dir, "other.ged" ) );
            ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_NO_RECENT_EXPORT );
            settings.exportDirs.clear();
            settings.exportDirs.push_back( CORE::CombinePath( dir, "no-such-directory" ).C_String() );
            ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_NO_RECENT_EXPORT );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

/** A zip file with one entry, MyTree.ged, which holds SampleGedcom */
static const unsigned char SampleZip[] = { 0x50,0x4b,0x03,0x04,0x14,0x00,0x00,0x00,0x08,0x00,0x58,0x9d,0x50,0x5d,0x30,0x2a,0x2f,0x5e,0x53,0x01,0x00,0x00,0x09,0x02,0x00,0x00,0x0a,0x00,0x00,0x00,0x4d,0x79,0x54,0x72,0x65,0x65,0x2e,0x67,0x65,0x64,0x55,0x50,0xdb,0x6e,0x83,0x30,0x0c,0x7d,0xe7,0x2b,0xfc,0x01,0x2b,0x24,0x30,0xb6,0xb6,0x4f,0xa4,0x10,0xda,0x74,0x5c,0xa6,0x90,0x6e,0x7b,0x9b,0x58,0x1b,0xb5,0x4c,0xe5,0x22,0x60,0xd2,0xfa,0xf7,0x73,0xa8,0xb6,0xa9,0x4f,0xb6,0x8f,0x8f,0xed,0x73,0x4c,0x60,0xc3,0x59,0x64,0x51,0x28,0xf2,0x9d,0x04,0xd6,0xec,0xf5,0x30,0xf6,0x17,0x7b,0xdf,0xd6,0x10,0x97,0x75,0x75,0xbe,0x80,0xea,0xb5,0x1e,0x2c,0x17,0x32,0x96,0xf2,0x5b,0x42,0xaa,0xeb,0x0f,0xdd,0xff,0x11,0x5e,0xb8,0x2c,0xc0,0x25,0xae,0x6f,0x13,0x8a,0x0b,0xd7,0x3c,0x0a,0x7f,0x51,0xdf,0xf6,0x6d,0x8a,0x45,0x9c,0xcb,0x14,0x12,0x91,0x71,0xb6,0xe6,0x33,0x8c,0x4f,0xdc,0x9c,0x0e,0x37,0x4c,0xc2,0x4e,0xc5,0xb3,0x39,0x16,0x11,0x53,0x1c,0xa8,0x0f,0x79,0xa8,0xcc,0xb2,0x07,0x8b,0x40,0x20,0x68,0x00,0x22,0x8b,0x04,0xb6,0x27,0x15,0xdb,0xb2,0x01,0x47,0xa1,0x10,0xc7,0x08,0xe7,0x6f,0x90,0x62,0x5c,0x09,0xa9,0xf0,0xc4,0x75,0x1e,0xb6,0x2c,0x03,0xba,0x20,0x04,0x91,0xc9,0x59,0x50,0xd0,0xc0,0xf2,0xe0,0x9d,0x3d,0x8b,0x08,0xe8,0xdd,0xc2,0x9d,0x2f,0x96,0x4b,0x4a,0xe8,0xbd,0xf7,0xe8,0x79,0x66,0x6f,0x8e,0x63,0x0c,0x9a,0x76,0xd4,0x30,0x9e,0xca,0x11,0x8e,0xed,0x64,0x2a,0xcc,0x33,0x05,0x6d,0x03,0x25,0x0c,0x7a,0xdf,0x36,0x07,0x38,0x57,0x8d,0x46,0x7e,0xbe,0xda,0x72,0x63,0x48,0x24,0x1c,0x4e,0xe3,0xd8,0x0d,0x4b,0xc7,0xd1,0xdf,0x65,0xdd,0x9d,0xb5,0xf9,0x8d,0x53,0xeb,0x43,0x55,0x3a,0xd4,0xfe,0xec,0x8e,0xc8,0x8e,0x59,0x5a,0x40,0x10,0xa3,0x04,0xe3,0xc6,0xbd,0x75,0x93,0x96,0x7d,0xa5,0xff,0xfd,0xdc,0x70,0x31,0x18,0x00,0xe1,0xcd,0xae,0x58,0x4d,0x9f,0xc0,0xfc,0x55,0xc4,0x7c,0xda,0x83,0x79,0xca,0xa4,0x34,0x4c,0xf4,0x37,0x39,0x45,0x48,0x09,0x95,0x40,0xa8,0x9b,0xe1,0x6b,0xb8,0xfe,0x00,0xdb,0x29,0xb6,0x27,0xcd,0xf4,0xaa,0xb9,0x3b,0xb5,0x63,0x3b,0xa9,0x23,0xa0,0x64,0x22,0xad,0x1f,0x50,0x4b,0x01,0x02,0x14,0x03,0x14,0x00,0x00,0x00,0x08,0x00,0x58,0x9d,0x50,0x5d,0x30,0x2a,0x2f,0x5e,0x53,0x01,0x00,0x00,0x09,0x02,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x00,0x00,0x4d,0x79,0x54,0x72,0x65,0x65,0x2e,0x67,0x65,0x64,0x50,0x4b,0x05,0x06,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x38,0x00,0x00,0x00,0x7b,0x01,0x00,0x00,0x00,0x00 };

static bool
WriteBytes( const CORE::CString& path, const unsigned char* data, size_t size )
{
    CORE::CFileAccess file( path, "wb" );
    if ( !file.Opened() )
        return false;
    const bool ok = file.Write( data, 1, (UInt32) size ) == size;
    file.Close();
    return ok;
}

/*-------------------------------------------------------------------------*/

static void
PerformZipTests( void )
{GUCEF_TRACE;

    const bool zipPlugin = LoadPlugin( "vfspluginLIBARCHIVE" );

    GUCEF_TESTFW_TESTCASE( "Test: A file is taken out of a zip through the virtual file system" )
        try
        {
            if ( !zipPlugin )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "zip tests skipped: vfspluginLIBARCHIVE is not available" );
            }
            else
            {
                ASSERT_TRUE( CZipExtractor::IsAvailable() );
                const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Zip" );
                CORE::CreateDirs( dir );
                const CORE::CString zipPath = CORE::CombinePath( dir, "export.zip" );
                ASSERT_TRUE( WriteBytes( zipPath, SampleZip, sizeof( SampleZip ) ) );

                CORE::CString extracted;
                ASSERT_TRUE( CZipExtractor::ExtractFirst( zipPath, ".ged", CORE::CombinePath( dir, "out" ), extracted ) );
                ASSERT_TRUE( extracted == CORE::CombinePath( CORE::CombinePath( dir, "out" ), "MyTree.ged" ) );
                ASSERT_TRUE( CORE::FileSize( extracted ) == strlen( SampleGedcom ) );
                ASSERT_TRUE( CExportFinder::LooksLikeAncestryExport( extracted ) );

                // asking again works, the archive is not left mounted
                CORE::CString again;
                ASSERT_TRUE( CZipExtractor::ExtractFirst( zipPath, ".GED", CORE::CombinePath( dir, "out2" ), again ) );

                // nothing like that in there, or not a zip at all
                CORE::CString none;
                ASSERT_FALSE( CZipExtractor::ExtractFirst( zipPath, ".xyz", CORE::CombinePath( dir, "out3" ), none ) );
                ASSERT_TRUE( none.IsNULLOrEmpty() );
                const CORE::CString notZip = CORE::CombinePath( dir, "notzip.zip" );
                CORE::WriteStringAsTextFile( notZip, "this is not a zip file" );
                ASSERT_FALSE( CZipExtractor::ExtractFirst( notZip, ".ged", CORE::CombinePath( dir, "out4" ), none ) );
                ASSERT_FALSE( CZipExtractor::ExtractFirst( CORE::CombinePath( dir, "missing.zip" ), ".ged", CORE::CombinePath( dir, "out5" ), none ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The baseline and export commands take in a zip with the export in it" )
        try
        {
            if ( zipPlugin )
            {
                const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_ZipCmd" );
                CORE::CreateDirs( CORE::CombinePath( dir, "downloads" ) );
                const CORE::CString zipPath = CORE::CombinePath( CORE::CombinePath( dir, "downloads" ), "MyTree.zip" );
                CORE::DeleteFile( zipPath );
                ASSERT_TRUE( WriteBytes( zipPath, SampleZip, sizeof( SampleZip ) ) );

                CSettings settings;
                settings.dataDir = CORE::CombinePath( dir, "data" );
                settings.treeId = "4321";
                const CDataLayout layout = settings.GetLayout();
                CORE::DeleteFile( layout.GetBaselineGedcomPath() );

                // by hand
                settings.baselineGedcom = zipPath;
                ASSERT_TRUE( CCommands::Baseline( settings ) == EXIT_CODE_OK );
                ASSERT_TRUE( CORE::FileExists( layout.GetBaselineGedcomPath() ) );
                ASSERT_TRUE( CORE::FileSize( layout.GetBaselineGedcomPath() ) == strlen( SampleGedcom ) );
                ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( CORE::CombinePath( layout.GetStateDir(), "unzipped" ), "MyTree.ged" ) ) );    // not left behind

                // and as a download, for another tree so that nothing was taken in yet
                settings.treeId = "4322";
                const CDataLayout layout2 = settings.GetLayout();
                settings.baselineGedcom = CORE::CString::Empty;
                settings.exportDirs.push_back( CORE::CombinePath( dir, "downloads" ).STL_String() );
                ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_OK );
                ASSERT_TRUE( CORE::FileExists( layout2.GetBaselineGedcomPath() ) );

                // once is enough
                CORE::DeleteFile( layout2.GetBaselineGedcomPath() );
                ASSERT_TRUE( CCommands::Export( settings ) == EXIT_CODE_OK );
                ASSERT_FALSE( CORE::FileExists( layout2.GetBaselineGedcomPath() ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPolicyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Only reads of the tool's own tree on the allowlist get through the gate" )
        try
        {
            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            policy.AddDefaultAllowed();
            const std::string T = "https://www.ancestry.com";

            ASSERT_TRUE( policy.Check( "GET", T + "/api/treesui-list/trees/86165115/persons?page=1&limit=100" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/treesui-list/trees/86165115/persons/count?fn=&ln=" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/tree-change-event/api/change-events?limit=20&treeId=86165115" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/treeviewer/tree/newfamilyview/86165115?focusPersonId=40537067793&genup=4" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/family-tree/person/facts/user/01f64673-0006-0000-0000-000000000000/tree/86165115/person/40537187384/factsglue" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/media/retrieval/v2/image/namespaces/1093/media/e2f33858-c1b7-4314-b190-c8b4fcda3ac3?client=trees-mediaservice&imageQuality=hq" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/media/retrieval/v2/image/namespaces/60898/media/44637_01s%5E%5E45%5E1841-00086.jpg?client=imageviewer-widgets" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/media/retrieval/v2/stream/namespaces/61515/media/0b9262c4-9dd5-40ac-8c93-54edf2e8b25c.ged?Client=Ancestry.Trees&filename=Tree" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/treesui-list/trees/86165115/export" ).allowed );

            // never a write
            ASSERT_FALSE( policy.Check( "POST", T + "/api/treesui-list/trees/86165115/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "PUT", T + "/api/media/viewer/api/trees/86165115/persons/40537067793?client=ugcgallery-fe" ).allowed );
            ASSERT_FALSE( policy.Check( "DELETE", T + "/api/treesui-list/trees/86165115/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "get", T + "/api/treesui-list/trees/86165115/persons" ).allowed );

            // a GET of the site that changes something is not on the list
            ASSERT_FALSE( policy.Check( "GET", T + "/api/coco/actions/SavePersonToTree" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/family-tree/person/addedit/user/01f64673-0006-0000-0000-000000000000/tree/86165115/person/40537067793/savepersontotree" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/api/hintsui-api/trees/86165115/merge/job/status?location=x" ).allowed );

            // other trees
            ASSERT_FALSE( policy.Check( "GET", T + "/api/treesui-list/trees/86362151/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/api/treeviewer/tree/newfamilyview/86362151?focusPersonId=1" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/tree-change-event/api/change-events?limit=20&treeId=86362151" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/family-tree/person/facts/user/01f64673-0006-0000-0000-000000000000/tree/86362151/person/32541871831/factsglue" ).allowed );

            // other hosts, plain http
            ASSERT_FALSE( policy.Check( "GET", "https://www.example.com/api/treesui-list/trees/86165115/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", "http://www.ancestry.com/api/treesui-list/trees/86165115/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", "https://www.ancestry.com.evil.example/api/treesui-list/trees/86165115/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", "/api/treesui-list/trees/86165115/persons" ).allowed );

            // credentials and DNA, even where the url would otherwise fit
            ASSERT_FALSE( policy.Check( "GET", T + "/account/password/verify" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/account/security/step-up/api/status" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/api/dna/matches/53F46AE4-480D-4848-8E12-C3CA3666D26C/matches" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/api/treesdna/thrulines?sampleid=1" ).allowed );

            // starting an export is only done when asked for
            ASSERT_FALSE( policy.Check( "GET", T + "/api/treesui-list/trees/86165115/export?start=true" ).allowed );
            policy.SetExportStartAllowed( true );
            ASSERT_TRUE( policy.Check( "GET", T + "/api/treesui-list/trees/86165115/export?start=true" ).allowed );

            // no tree set: nothing that names a tree passes
            CRequestPolicy noTree;
            noTree.AddDefaultAllowed();
            ASSERT_FALSE( noTree.Check( "GET", T + "/api/treesui-list/trees/86165115/persons" ).allowed );

            // there is always a reason
            ASSERT_FALSE( policy.Check( "POST", T + "/x" ).reason.empty() );
            ASSERT_FALSE( policy.Check( "GET", T + "/not/on/the/list" ).reason.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The POST requests of the DNA facility are the only ones allowed, and only with ids" )
        try
        {
            const std::string T = "https://www.ancestry.com";
            const std::string sample = "53F46AE4-480D-4848-8E12-C3CA3666D26C";
            const std::string a = "69909A05-B93F-45BD-8AFC-8155DA610BCC";
            const std::string b = "b9be6eac-4cf8-41fc-b7eb-60aec3542491";
            const std::string good = "{\"matchSampleIds\":[\"" + a + "\",\"" + b + "\"]}";

            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            policy.AddDefaultAllowed();

            // not in DNA mode: no POST at all, and no DNA
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", good ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/api/dna/matches/" + sample + "/matches" ).allowed );

            policy.SetDnaAllowed( true );
            policy.AddDnaAllowed();
            ASSERT_TRUE( policy.Check( "GET", T + "/api/dna/matches/" + sample + "/matches?page=2" ).allowed );
            ASSERT_TRUE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", good ).allowed );
            ASSERT_TRUE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/clusters/profileData", good ).allowed );
            ASSERT_TRUE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/commonAncestors", "{\"sampleIds\":[\"" + a + "\"]}" ).allowed );
            ASSERT_TRUE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/trees/inTree", "[ \"" + a + "\" ]" ).allowed );

            // writes are still not allowed, also not in DNA mode
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/tags/forMatches", good ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/ignore", good ).allowed );
            ASSERT_FALSE( policy.Check( "PUT", T + "/api/dna/matches/" + sample + "/relationships/data", good ).allowed );
            ASSERT_FALSE( policy.Check( "DELETE", T + "/api/dna/matches/" + sample + "/matches", good ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/treesui-list/trees/86165115/persons", good ).allowed );

            // a body that is more than ids
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"matchSampleIds\":[]}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"matchSampleIds\":[\"" + a + "\"],\"isIgnored\":true}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"matchSampleIds\":[\"John Smith\"]}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"note\":[\"" + a + "\"]}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"matchSampleIds\":[\"" + a + "\",]}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", T + "/api/dna/matches/" + sample + "/relationships/data", "{\"matchSampleIds\":[\"" + a + "x\"]}" ).allowed );

            // credentials and other trees stay out in DNA mode as well
            ASSERT_FALSE( policy.Check( "POST", T + "/account/password/verify", "{\"matchSampleIds\":[\"" + a + "\"]}" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", T + "/account/security/step-up/api/status" ).allowed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The allowlist can be extended from a file" )
        try
        {
            const CORE::CString file = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_allow.txt" );
            CORE::WriteStringAsTextFile( file, "# extra endpoints\nwww.ancestry.com/api/extra/{tree}/things\n\n  www.ancestry.com/api/other/*/x  \n" );
            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            ASSERT_TRUE( policy.LoadAllowedFile( file ) == 2 );
            ASSERT_TRUE( policy.GetAllowedCount() == 2 );
            ASSERT_TRUE( policy.Check( "GET", "https://www.ancestry.com/api/extra/86165115/things" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", "https://www.ancestry.com/api/extra/86362151/things" ).allowed );
            ASSERT_TRUE( policy.Check( "GET", "https://www.ancestry.com/api/other/anything/x" ).allowed );
            ASSERT_TRUE( policy.LoadAllowedFile( CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_nofile.txt" ) ) == 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
PerformPacingAndFeedTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The budget limits a run and a day, and varies the pause between requests" )
        try
        {
            CRequestBudget budget;
            budget.Configure( 1000, 40, 3, 5 );
            const UInt64 t0 = 1792195200000ULL;      // 2026-10-17T00:00:00Z
            std::string reason;
            ASSERT_TRUE( budget.CanRequest( t0, reason ) );
            ASSERT_TRUE( budget.GetDelayBeforeNextInMs( t0 ) == 0 );       // the first one goes right away

            // the pause is 60% to 140% of the delay and is not the same every time
            GUCEF::set< UInt32 > pauses;
            for ( UInt32 i=0; i<3; ++i )
            {
                ASSERT_TRUE( budget.CanRequest( t0, reason ) );
                budget.RecordRequest( t0 );
                const UInt32 pause = budget.GetDelayBeforeNextInMs( t0 );
                ASSERT_TRUE( pause >= 600 && pause <= 1400 );
                pauses.insert( pause );
                // later on the pause is over
                ASSERT_TRUE( budget.GetDelayBeforeNextInMs( t0 + 5000 ) == 0 );
            }
            ASSERT_FALSE( budget.CanRequest( t0, reason ) );               // 3 per run
            ASSERT_FALSE( reason.empty() );
            ASSERT_TRUE( budget.GetRunCount() == 3 );

            // and the same pause is not used every time
            CRequestBudget varied;
            varied.Configure( 1000, 40, 0, 0 );
            GUCEF::set< UInt32 > seen;
            for ( UInt32 i=0; i<30; ++i )
            {
                varied.RecordRequest( t0 );
                seen.insert( varied.GetDelayBeforeNextInMs( t0 ) );
            }
            ASSERT_TRUE( seen.size() > 5 );

            // no jitter: always the same
            CRequestBudget fixed;
            fixed.Configure( 1000, 0, 0, 0 );
            fixed.RecordRequest( t0 );
            ASSERT_TRUE( fixed.GetDelayBeforeNextInMs( t0 ) == 1000 );
            ASSERT_TRUE( fixed.GetDelayBeforeNextInMs( t0 + 400 ) == 600 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The count of the day survives a restart and starts again the next day" )
        try
        {
            const CORE::CString file = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_budget.txt" );
            CORE::DeleteFile( file );
            const UInt64 t0 = 1792195200000ULL + 3600000ULL;      // 2026-10-17T01:00:00Z
            {
                CRequestBudget budget;
                budget.Configure( 1000, 0, 0, 4 );
                budget.SetStateFile( file, t0 );
                ASSERT_TRUE( budget.GetTodayCount() == 0 );
                budget.RecordRequest( t0 );
                budget.RecordRequest( t0 + 1 );
                budget.RecordRequest( t0 + 2 );
            }
            CRequestBudget again;
            again.Configure( 1000, 0, 0, 4 );
            again.SetStateFile( file, t0 + 60000 );
            ASSERT_TRUE( again.GetTodayCount() == 3 );
            ASSERT_TRUE( again.GetRunCount() == 0 );
            std::string reason;
            ASSERT_TRUE( again.CanRequest( t0 + 60000, reason ) );
            again.RecordRequest( t0 + 60000 );
            ASSERT_FALSE( again.CanRequest( t0 + 61000, reason ) );           // 4 per day
            ASSERT_TRUE( reason.find( "day" ) != std::string::npos );

            // the next day the count starts again, in the same run
            ASSERT_TRUE( again.CanRequest( t0 + 24ULL * 3600 * 1000, reason ) );

            // another day in the file: counted from zero
            CRequestBudget nextDay;
            nextDay.Configure( 1000, 0, 0, 4 );
            nextDay.SetStateFile( file, t0 + 24ULL * 3600 * 1000 );
            ASSERT_TRUE( nextDay.GetTodayCount() == 0 );
            ASSERT_TRUE( CRequestBudget::DayOf( t0 ) == "20261017" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The tool stops for good when the site shows unhappiness" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Halt" );
            CORE::CreateDirs( dir );
            CHaltState::Clear( dir );
            std::string reason;
            ASSERT_FALSE( CHaltState::IsHalted( dir, reason ) );
            ASSERT_TRUE( CHaltState::Halt( dir, "the site answered 429" ) );
            ASSERT_TRUE( CHaltState::IsHalted( dir, reason ) );
            ASSERT_TRUE( reason == "the site answered 429" );
            CHaltState::Clear( dir );
            ASSERT_FALSE( CHaltState::IsHalted( dir, reason ) );

            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 429, "application/json", "", "{}", true, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 403, "text/html", "", "", true, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 401, "application/json", "", "", true, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 503, "text/html", "", "", true, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 200, "application/json", "challenge", "{}", true, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 200, "text/html; charset=UTF-8", "", "<html><title>Just a moment...</title>", false, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 200, "text/html", "", "<html><body>please solve this CAPTCHA</body>", false, reason ) );
            ASSERT_TRUE( CSiteResponseCheck::IsTrouble( 200, "text/html", "", "<html>a login page</html>", true, reason ) );       // html where json was asked for

            ASSERT_FALSE( CSiteResponseCheck::IsTrouble( 200, "application/json", "", "{\"a\":1}", true, reason ) );
            ASSERT_FALSE( CSiteResponseCheck::IsTrouble( 200, "text/html", "", "<html>a person page</html>", false, reason ) );
            ASSERT_FALSE( CSiteResponseCheck::IsTrouble( 404, "application/json", "", "{}", true, reason ) );             // not found is not a complaint
            ASSERT_FALSE( CSiteResponseCheck::IsTrouble( 204, "", "", "", true, reason ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Times of the site and of a GEDCOM header become the same moment" )
        try
        {
            UInt64 t = 0;
            ASSERT_TRUE( CChangeFeed::ParseTime( "2026-10-16T21:27:48Z", t ) );
            ASSERT_TRUE( t == 1792186068000ULL );
            ASSERT_TRUE( CChangeFeed::FormatTime( t ) == "2026-10-16T21:27:48Z" );
            ASSERT_TRUE( CChangeFeed::ParseTime( "2026-10-16T21:27:48.466Z", t ) && t == 1792186068466ULL );
            ASSERT_TRUE( CChangeFeed::ParseTime( "2026-10-16T21:27:48.4Z", t ) && t == 1792186068400ULL );
            ASSERT_TRUE( CChangeFeed::ParseTime( "1970-01-01T00:00:00Z", t ) && t == 0 );
            ASSERT_TRUE( CChangeFeed::ParseTime( "2024-02-29T12:00:00Z", t ) && t == 1709208000000ULL );
            ASSERT_FALSE( CChangeFeed::ParseTime( "", t ) );
            ASSERT_FALSE( CChangeFeed::ParseTime( "yesterday", t ) );
            ASSERT_FALSE( CChangeFeed::ParseTime( "2026-13-01T00:00:00Z", t ) );
            ASSERT_FALSE( CChangeFeed::ParseTime( "2026-10-16T21:27:48+02:00", t ) );    // an offset is not guessed at

            ASSERT_TRUE( CChangeFeed::ParseGedcomDateTime( "16 Oct 2026", "23:14:06", t ) );
            ASSERT_TRUE( CChangeFeed::FormatTime( t ) == "2026-10-16T23:14:06Z" );
            ASSERT_TRUE( CChangeFeed::ParseGedcomDateTime( "1 JAN 2020", "", t ) && CChangeFeed::FormatTime( t ) == "2020-01-01T00:00:00Z" );
            ASSERT_FALSE( CChangeFeed::ParseGedcomDateTime( "sometime", "", t ) );
            ASSERT_FALSE( CChangeFeed::ParseGedcomDateTime( "16 Xyz 2026", "", t ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( !JsonAvailable() )
        return;

    GUCEF_TESTFW_TESTCASE( "Test: The id of the tree on the site is a number and is found in the export header" )
        try
        {
            CRequestPolicy policy;
            ASSERT_FALSE( policy.SetTreeId( "default" ) );        // the name of a directory is not the id of a tree
            ASSERT_FALSE( policy.SetTreeId( "" ) );
            ASSERT_FALSE( policy.SetTreeId( "86165115x" ) );
            policy.AddDefaultAllowed();
            ASSERT_FALSE( policy.Check( "GET", "https://www.ancestry.com/api/treesui-list/trees/default/persons" ).allowed );
            ASSERT_FALSE( policy.Check( "GET", "https://www.ancestry.com/api/treesui-list/trees/86165115/persons" ).allowed );   // no tree is set, nothing about a tree
            ASSERT_TRUE( policy.SetTreeId( "86165115" ) );
            ASSERT_TRUE( policy.Check( "GET", "https://www.ancestry.com/api/treesui-list/trees/86165115/persons" ).allowed );

            const char* const header = "0 HEAD\n1 SOUR Ancestry.com Family Trees\n2 NAME Ancestry.com Member Trees\n2 _TREE My Family\n3 RIN 86165115\n3 _ENV prd\n"
                                       "1 DATE 16 Oct 2026\n2 TIME 23:14:06\n1 CHAR UTF-8\n0 @I1@ INDI\n0 TRLR\n";
            CGedcomSummarizer summarizer;
            CGedcomReader::Parse( header, strlen( header ), summarizer );
            ASSERT_TRUE( summarizer.GetSummary().treeId == "86165115" && summarizer.GetSummary().treeName == "My Family" );
            ASSERT_TRUE( summarizer.GetSummary().headerTime == "23:14:06" && summarizer.GetSummary().headerDate == "16 Oct 2026" );
            ASSERT_TRUE( Contains( CGedcomSummarizer::Render( summarizer.GetSummary() ), "id 86165115" ) );

            CSettings settings;
            std::map< std::string, std::string > options;
            options[ "sitetreeid" ] = "not-a-number";
            CORE::CString problem;
            ASSERT_FALSE( settings.Load( options, problem ) );
            options[ "sitetreeid" ] = "86165115";
            ASSERT_TRUE( settings.Load( options, problem ) && settings.siteTreeId == "86165115" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A page of the change feed is read into changes and a cursor" )
        try
        {
            const char* const page =
                "{\"cursor\":\"bmV4dA\",\"changes\":["
                "{\"title\":\"edited\",\"action\":\"edit_fact\",\"author\":{\"displayName\":\"Jan\",\"photoUrl\":\"x\",\"userId\":\"u-1\"},\"time\":\"2026-10-16T21:27:48Z\","
                "\"treeId\":\"86165115\",\"subGroupId\":null,\"hasChildren\":false,\"referenceId\":\"ref-1\",\"personNoLongerExists\":false,"
                "\"person\":{\"gender\":\"M\",\"name\":\"X\",\"personId\":40537163709},\"fact\":{\"id\":\"f1\",\"type\":\"NAME\",\"year\":null}},"
                "{\"action\":\"add_citation\",\"author\":{\"userId\":\"u-1\"},\"time\":\"2026-10-16T21:17:06Z\",\"referenceId\":\"ref-2\",\"personNoLongerExists\":true,\"person\":{\"personId\":40537163709}},"
                "{\"action\":\"add_fact\",\"time\":\"not a time\",\"person\":{\"personId\":1}},"
                "{\"action\":\"edit_fact\",\"time\":\"2026-10-16T20:00:00Z\",\"person\":{\"personId\":40537000001}}]}";
            CORE::CDataNode document;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( page, document ) );
            SChangePage parsed;
            ASSERT_TRUE( CChangeFeed::ParsePage( document, parsed ) );
            ASSERT_TRUE( parsed.nextCursor == "bmV4dA" );
            ASSERT_TRUE( parsed.events.size() == 3 );               // the one without a proper time is skipped
            if ( parsed.events.size() == 3 )
            {
                ASSERT_TRUE( parsed.events[ 0 ].action == "edit_fact" && parsed.events[ 0 ].personId == "40537163709" );
                ASSERT_TRUE( parsed.events[ 0 ].factId == "f1" && parsed.events[ 0 ].factType == "NAME" && parsed.events[ 0 ].authorId == "u-1" );
                ASSERT_TRUE( parsed.events[ 0 ].timeInMs == 1792186068000ULL && parsed.events[ 0 ].referenceId == "ref-1" );
                ASSERT_TRUE( parsed.events[ 1 ].personNoLongerExists );
            }
            const std::vector< std::string > people = CChangeFeed::GetChangedPeople( parsed.events );
            ASSERT_TRUE( people.size() == 2 && people[ 0 ] == "40537163709" && people[ 1 ] == "40537000001" );      // each once

            const std::string line = CChangeFeed::ToJournalLine( parsed.events[ 0 ] );
            CORE::CDataNode lineDocument;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( line.c_str(), lineDocument ) );
            ASSERT_TRUE( lineDocument.GetAttributeValue( "action" ).AsString() == "edit_fact" );

            // not a feed at all
            CORE::CDataNode other;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"hello\":1}", other ) );
            ASSERT_FALSE( CChangeFeed::ParsePage( other, parsed ) );

            ASSERT_TRUE( CChangeFeed::BuildUrl( "86165115", 20, "" ) == "https://www.ancestry.com/tree-change-event/api/change-events?useMockData=false&limit=20&treeId=86165115" );
            ASSERT_TRUE( CChangeFeed::BuildUrl( "86165115", 20, "abc" ).find( "&cursor=abc" ) != std::string::npos );

            // and the gate lets the feed through, for this tree only
            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            policy.AddDefaultAllowed();
            ASSERT_TRUE( policy.Check( "GET", CChangeFeed::BuildUrl( "86165115", 20, "abc" ) ).allowed );
            ASSERT_FALSE( policy.Check( "GET", CChangeFeed::BuildUrl( "99999999", 20, "" ) ).allowed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static bool
TestSerialize( const CORE::CDataNode& node, CORE::CString& json )
{
    return COM::CJsonRpcEndpoint::ToJsonText( node, json );
}

/*-------------------------------------------------------------------------*/

static void
PerformMediaTests( void )
{GUCEF_TRACE;

    if ( !JsonAvailable() )
        return;

    GUCEF_TESTFW_TESTCASE( "Test: A page of the media list is read and the items are kept whole" )
        try
        {
            const char* const page =
                "{\"treeRights\":{},\"mediaCount\":1593,\"hasMedia\":true,\"objects\":["
                "{\"title\":\"Riek Coort\",\"type\":\"p\",\"ext\":\"jpeg\",\"width\":\"1127\",\"height\":\"845\",\"category\":\"photo\",\"date\":\"1950\",\"description\":\"d\",\"location\":\"Utrecht\","
                "\"id\":\"5e5621c0-7b5c-440a-a239-620b05a13810\",\"collectionId\":\"1093\",\"url\":\"/api/media/retrieval/v2/image/namespaces/1093/media/5e5621c0.jpg?client=trees-mediaservice\",\"ms_lookup_id\":\"99ef7ec8-95d1-4f45-bf07-9358077f0fdf\",\"createdDate\":\"Tue, 13 Jan 2026 19:52:03 GMT\",\"tags\":[{\"pid\":\"1\"}]},"
                "{\"title\":\"A story\",\"category\":\"story\",\"id\":\"story-1\",\"collectionId\":\"1093\",\"ms_lookup_id\":\"s1\"},"
                "{\"title\":\"no id\"},"
                "{\"title\":\"A scan\",\"ext\":\"PDF\",\"category\":\"document\",\"id\":\"doc/../1\",\"collectionId\":\"1093\",\"ms_lookup_id\":\"d1\"}]}";
            CORE::CDataNode document;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( page, document ) );
            SMediaPage parsed;
            ASSERT_TRUE( CMediaInventory::ParsePage( document, parsed, &TestSerialize ) );
            ASSERT_TRUE( parsed.totalCount == 1593 );
            ASSERT_TRUE( parsed.items.size() == 3 );            // the one without an id is skipped
            const SMediaItem firstItem = parsed.items.empty() ? SMediaItem() : parsed.items[ 0 ];
            if ( parsed.items.size() == 3 )
            {
                const SMediaItem& photo = parsed.items[ 0 ];
                ASSERT_TRUE( photo.id == "5e5621c0-7b5c-440a-a239-620b05a13810" && photo.lookupId == "99ef7ec8-95d1-4f45-bf07-9358077f0fdf" && photo.namespaceId == "1093" );
                ASSERT_TRUE( photo.title == "Riek Coort" && photo.date == "1950" && photo.location == "Utrecht" && photo.width == "1127" );
                ASSERT_TRUE( photo.json.find( "\"pid\"" ) != std::string::npos );     // nothing the site said is dropped, the tags included
                ASSERT_TRUE( CMediaInventory::IsDownloadable( photo ) );
                ASSERT_TRUE( CMediaInventory::BuildSiteUrl( photo ) == "https://www.ancestry.com/api/media/retrieval/v2/image/namespaces/1093/media/5e5621c0.jpg?client=trees-mediaservice" );
                ASSERT_TRUE( CMediaInventory::BuildSiteUrl( parsed.items[ 1 ] ).empty() );
                ASSERT_TRUE( CMediaInventory::FileNameOf( photo ) == "5e5621c0-7b5c-440a-a239-620b05a13810.jpg" );
                ASSERT_TRUE( CMediaInventory::BuildDownloadUrl( photo ) == "https://www.ancestry.com/api/media/retrieval/v2/image/namespaces/1093/media/99ef7ec8-95d1-4f45-bf07-9358077f0fdf?client=trees-mediaservice&imageQuality=hq" );

                ASSERT_FALSE( CMediaInventory::IsDownloadable( parsed.items[ 1 ] ) );        // a story is not a picture
                ASSERT_TRUE( CMediaInventory::IsDownloadable( parsed.items[ 2 ] ) );
                // a file name never leaves the media directory
                ASSERT_TRUE( CMediaInventory::FileNameOf( parsed.items[ 2 ] ) == "doc1.pdf" );
            }

            const std::string line = CMediaInventory::ToInventoryLine( firstItem );
            CORE::CDataNode lineDocument;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( line.c_str(), lineDocument ) );
            ASSERT_TRUE( lineDocument.GetAttributeValue( "lookupId" ).AsString() == "99ef7ec8-95d1-4f45-bf07-9358077f0fdf" && lineDocument.Find( "item" ) != GUCEF_NULL );

            CORE::CDataNode other;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"hello\":1}", other ) );
            ASSERT_FALSE( CMediaInventory::ParsePage( other, parsed, &TestSerialize ) );

            // and the gate lets the list and the file through
            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            policy.AddDefaultAllowed();
            ASSERT_TRUE( policy.Check( "GET", CMediaInventory::BuildListUrl( "86165115", "Photo", 3, 28 ) ).allowed );
            ASSERT_FALSE( policy.Check( "GET", CMediaInventory::BuildListUrl( "86362151", "Photo", 3, 28 ) ).allowed );
            ASSERT_TRUE( policy.Check( "GET", CMediaInventory::BuildDownloadUrl( firstItem ) ).allowed );
            ASSERT_TRUE( CMediaInventory::GetFilters().size() == 5 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static void
PerformDnaTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: CSV fields are quoted when they need to be" )
        try
        {
            ASSERT_TRUE( CDnaMatches::CsvField( "plain" ) == "plain" );
            ASSERT_TRUE( CDnaMatches::CsvField( "a,b" ) == "\"a,b\"" );
            ASSERT_TRUE( CDnaMatches::CsvField( "say \"hi\"" ) == "\"say \"\"hi\"\"\"" );
            ASSERT_TRUE( CDnaMatches::CsvField( "two\nlines" ) == "\"two\nlines\"" );
            ASSERT_TRUE( CDnaMatches::CsvField( "" ) == "" );
            ASSERT_TRUE( CDnaMatches::DateOf( 1432533600000ULL ) == "2015-05-25" );
            ASSERT_TRUE( CDnaMatches::LooksLikeSampleId( "53F46AE4-480D-4848-8E12-C3CA3666D26C" ) );
            ASSERT_FALSE( CDnaMatches::LooksLikeSampleId( "53F46AE4-480D-4848-8E12-C3CA3666D26" ) );
            ASSERT_FALSE( CDnaMatches::LooksLikeSampleId( "53F46AE4_480D_4848_8E12_C3CA3666D26C" ) );
            ASSERT_TRUE( CDnaMatches::FindTestId( "x https://www.ancestry.com/api/dna/matches/53f46ae4-480d-4848-8e12-c3ca3666d26c/matches?currentPage=2 y" ) == "53F46AE4-480D-4848-8E12-C3CA3666D26C" );
            ASSERT_TRUE( CDnaMatches::FindTestId( "nothing about dna here" ).empty() );
            ASSERT_TRUE( CDnaMatches::FindTestId( "/api/dna/matches/not-a-guid/matches" ).empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( !JsonAvailable() )
        return;

    const std::string test = "53F46AE4-480D-4848-8E12-C3CA3666D26C";
    const std::string a = "69909A05-B93F-45BD-8AFC-8155DA610BCC";
    const std::string b = "B9BE6EAC-4CF8-41FC-B7EB-60AEC3542491";

    GUCEF_TESTFW_TESTCASE( "Test: A page of the DNA match list and the details of its matches make one record each" )
        try
        {
            const std::string list = "{\"currentPage\":1,\"matchCount\":4311,\"apiPage\":1,\"isLastPage\":false,\"totalPages\":216,\"matchList\":["
                "{\"sampleId\":\"" + a + "\",\"ignored\":false,\"tags\":{\"1\":null,\"5\":\"PM\",\"9\":\"[{\\\"r\\\":\\\"PM\\\"}]\"},\"relationship\":{\"meiosis\":3,\"sharedCentimorgans\":2014,\"numSharedSegments\":36},"
                "\"matchClusterCode\":\"blocked\",\"createdDate\":1432533600000},"
                "{\"sampleId\":\"" + b + "\",\"ignored\":true,\"tags\":{},\"relationship\":{\"meiosis\":9,\"sharedCentimorgans\":51,\"numSharedSegments\":4},\"matchClusterCode\":\"c1\",\"createdDate\":1700000000000},"
                "{\"ignored\":false}]}";
            CORE::CDataNode document;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( list.c_str(), document ) );
            SDnaMatchPage page;
            ASSERT_TRUE( CDnaMatches::ParseListPage( document, test, page ) );
            ASSERT_TRUE( page.currentPage == 1 && page.totalPages == 216 && page.matchCount == 4311 && !page.isLastPage );
            ASSERT_TRUE( page.matches.size() == 2 );        // the one without an id is skipped
            if ( page.matches.size() == 2 )
            {
                const SDnaMatch& first = page.matches[ 0 ];
                ASSERT_TRUE( first.matchId == a && first.testId == test && first.provider == "ancestry" );
                ASSERT_TRUE( first.sharedCm == 2014.0 && first.sharedSegments == 36 && first.meiosis == 3 && first.clusterCode == "blocked" && !first.ignored );
                ASSERT_TRUE( first.matchDate == "2015-05-25" );
                ASSERT_TRUE( first.tags.find( "5=PM" ) != std::string::npos );
                ASSERT_TRUE( first.tags.find( "1=" ) == std::string::npos );                   // tags without a value are not listed
                ASSERT_TRUE( first.profileUrl == "https://www.ancestry.com/discoveryui-matches/compare/" + test + "/with/" + a );
                ASSERT_TRUE( page.matches[ 1 ].ignored && page.matches[ 1 ].sharedCm == 51.0 );
            }

            const std::string relationships = "{\"" + a + "\":{\"assignment\":\"1st cousin 1x removed\",\"predictionSource\":\"STATISTICAL_MODEL\",\"assignmentType\":\"PREDICTED\","
                "\"sharedCm\":2014,\"numSharedSegments\":36,\"meiosis\":3,\"relationshipGroup\":\"EXTENDED_FAMILY\",\"percentDna\":\"29\",\"parentSideFlag\":\"PATERNAL\"},"
                "\"" + b + "\":{\"assignment\":\"4th cousin, with, a comma\",\"relationshipGroup\":\"DISTANT_FAMILY\",\"percentDna\":\"< 1\",\"parentSideFlag\":\"BLOCKED\"},"
                "\"11111111-1111-1111-1111-111111111111\":{\"assignment\":\"not in the list\"}}";
            const std::string profiles = "{\"" + a + "\":{\"matchUcdmid\":\"u1\",\"matchName\":\"Jan Test\",\"matchNameInitials\":\"JT\",\"displayGender\":\"M\"},"
                "\"" + b + "\":{\"matchName\":\"Say \\\"Hi\\\"\",\"matchNameInitials\":\"SH\",\"displayGender\":\"F\"}}";
            CORE::CDataNode relationshipDocument, profileDocument;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( relationships.c_str(), relationshipDocument ) );
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( profiles.c_str(), profileDocument ) );
            CDnaMatches::MergeRelationships( relationshipDocument, page.matches );
            CDnaMatches::MergeProfiles( profileDocument, page.matches );

            const SDnaMatch& merged = page.matches[ 0 ];
            ASSERT_TRUE( merged.predictedRelationship == "1st cousin 1x removed" && merged.relationshipGroup == "EXTENDED_FAMILY" && merged.parentSide == "paternal" && merged.percentShared == "29" );
            ASSERT_TRUE( merged.name == "Jan Test" && merged.initials == "JT" && merged.sex == "M" && merged.predictionSource == "STATISTICAL_MODEL" );
            ASSERT_TRUE( page.matches[ 1 ].predictedRelationship == "4th cousin, with, a comma" && page.matches[ 1 ].percentShared == "< 1" && page.matches[ 1 ].parentSide == "blocked" );

            // the CSV: one row a match, quoted where needed
            ASSERT_TRUE( CDnaMatches::CsvHeader().compare( 0, 40, "provider,test_id,match_id,name,initials," ) == 0 );
            const std::string row = CDnaMatches::ToCsvRow( merged );
            ASSERT_TRUE( row.compare( 0, 17, "ancestry," + test.substr( 0, 8 ) ) == 0 );
            ASSERT_TRUE( row.find( ",Jan Test,JT,M,2014,36," ) != std::string::npos );
            ASSERT_TRUE( CDnaMatches::ToCsvRow( page.matches[ 1 ] ).find( "\"4th cousin, with, a comma\"" ) != std::string::npos );
            ASSERT_TRUE( CDnaMatches::ToCsvRow( page.matches[ 1 ] ).find( "\"Say \"\"Hi\"\"\"" ) != std::string::npos );

            // and the record survives being written down and read back
            CORE::CDataNode lineDocument;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( CDnaMatches::ToJsonLine( page.matches[ 1 ] ).c_str(), lineDocument ) );
            SDnaMatch back;
            ASSERT_TRUE( CDnaMatches::FromJsonLine( lineDocument, back ) );
            ASSERT_TRUE( back.matchId == b && back.name == "Say \"Hi\"" && back.sharedCm == 51.0 && back.ignored && back.predictedRelationship == page.matches[ 1 ].predictedRelationship );

            // a page that is not a match list
            CORE::CDataNode other;
            ASSERT_TRUE( COM::CJsonRpcEndpoint::ParseJsonText( "{\"hello\":1}", other ) );
            ASSERT_FALSE( CDnaMatches::ParseListPage( other, test, page ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The requests of the DNA facility pass the gate and nothing else about DNA does" )
        try
        {
            CRequestPolicy policy;
            policy.SetTreeId( "86165115" );
            policy.AddDefaultAllowed();
            policy.SetDnaAllowed( true );
            policy.AddDnaAllowed();
            ASSERT_TRUE( policy.Check( "GET", CDnaMatches::BuildListUrl( test, 1 ) ).allowed );
            ASSERT_TRUE( policy.Check( "GET", CDnaMatches::BuildListUrl( test, 216 ) ).allowed );
            ASSERT_TRUE( CDnaMatches::BuildListUrl( test, 1 ).find( "currentPage" ) == std::string::npos );
            ASSERT_TRUE( CDnaMatches::BuildListUrl( test, 2 ).find( "?currentPage=2" ) != std::string::npos );
            std::vector< std::string > ids;
            ids.push_back( a );
            ids.push_back( b );
            const std::string body = CDnaMatches::BuildIdsBody( ids );
            ASSERT_TRUE( body == "{\"matchSampleIds\":[\"" + a + "\",\"" + b + "\"]}" );
            ASSERT_TRUE( policy.Check( "POST", CDnaMatches::BuildRelationshipsUrl( test ), body ).allowed );
            ASSERT_TRUE( policy.Check( "POST", CDnaMatches::BuildProfilesUrl( test ), body ).allowed );
            ASSERT_FALSE( policy.Check( "POST", "https://www.ancestry.com/api/dna/matches/" + test + "/tags/match/" + a + "/1", "{\"tagValue\":\"x\"}" ).allowed );
            ASSERT_FALSE( policy.Check( "POST", "https://www.ancestry.com/api/dna/matches/" + test + "/parents/clusterCodes", "{\"matchingSampleIds\":[\"" + a + "\"]}" ).allowed );

            // DNA is closed unless asked for
            CRequestPolicy closed;
            closed.SetTreeId( "86165115" );
            closed.AddDefaultAllowed();
            closed.AddDnaAllowed();
            ASSERT_FALSE( closed.Check( "GET", CDnaMatches::BuildListUrl( test, 1 ) ).allowed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformBrowserTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The browser is started with a debugging port and a profile" )
        try
        {
            CChromeBrowser browser;
            browser.SetDebugPort( 9333 );
            browser.SetProfileDir( "/data/1234/browser-profile" );
            browser.AddArgument( "--lang=nl" );
            std::vector< CORE::CString > args = browser.BuildArguments( "https://www.ancestry.com/" );
            ASSERT_TRUE( args.size() >= 5 );
            ASSERT_TRUE( args[ 0 ] == "--remote-debugging-port=9333" );
            ASSERT_TRUE( args[ 1 ] == "--user-data-dir=/data/1234/browser-profile" );
            ASSERT_TRUE( args.back() == "https://www.ancestry.com/" );
            bool hasLang = false, hasHeadless = false;
            for ( size_t i=0; i<args.size(); ++i )
            {
                if ( args[ i ] == "--lang=nl" ) hasLang = true;
                if ( Contains( args[ i ], "--headless" ) ) hasHeadless = true;
            }
            ASSERT_TRUE( hasLang );
            ASSERT_FALSE( hasHeadless );

            browser.SetHeadless( true );
            args = browser.BuildArguments( "" );
            hasHeadless = false;
            for ( size_t i=0; i<args.size(); ++i )
                if ( Contains( args[ i ], "--headless" ) ) hasHeadless = true;
            ASSERT_TRUE( hasHeadless );
            ASSERT_FALSE( args.back() == "" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The browser is started without the environment of a snap package" )
        try
        {
            std::vector< std::pair< std::string, std::string > > env;
            env.push_back( std::make_pair( "HOME", "/home/me" ) );
            env.push_back( std::make_pair( "SNAP", "/snap/code/269" ) );
            env.push_back( std::make_pair( "SNAP_NAME", "code" ) );
            env.push_back( std::make_pair( "GSETTINGS_SCHEMA_DIR", "/home/me/snap/code/269/schemas" ) );
            env.push_back( std::make_pair( "XDG_DATA_DIRS", "/snap/code/269/usr/share" ) );
            env.push_back( std::make_pair( "XDG_DATA_DIRS_VSCODE_SNAP_ORIG", "/usr/share:/usr/local/share" ) );
            env.push_back( std::make_pair( "LANG", "en_US.UTF-8" ) );

            std::vector< std::string > unsetNames;
            std::map< std::string, std::string > overrides;
            CChromeBrowser::BuildCleanEnvironment( env, unsetNames, overrides );

            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "GSETTINGS_SCHEMA_DIR" ) != unsetNames.end() );
            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "SNAP_NAME" ) != unsetNames.end() );
            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "SNAP" ) != unsetNames.end() );
            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "HOME" ) == unsetNames.end() );
            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "LANG" ) == unsetNames.end() );
            ASSERT_TRUE( std::find( unsetNames.begin(), unsetNames.end(), "XDG_DATA_DIRS" ) == unsetNames.end() );
            ASSERT_TRUE( overrides.size() == 1 && overrides[ "XDG_DATA_DIRS" ] == "/usr/share:/usr/local/share" );

            // an ordinary environment is left alone
            env.clear();
            env.push_back( std::make_pair( "HOME", "/home/me" ) );
            env.push_back( std::make_pair( "GSETTINGS_SCHEMA_DIR", "/usr/share/glib-2.0/schemas" ) );
            CChromeBrowser::BuildCleanEnvironment( env, unsetNames, overrides );
            ASSERT_TRUE( unsetNames.empty() && overrides.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The DevToolsActivePort file of a running browser tells where to connect" )
        try
        {
            UInt16 port = 0;
            CORE::CString path;
            ASSERT_TRUE( CChromeBrowser::ParseDevToolsActivePort( "9222\n/devtools/browser/2c6f8888-7a42-4149-98b4-df16bc9b81e3", port, path ) );
            ASSERT_TRUE( port == 9222 && path == "/devtools/browser/2c6f8888-7a42-4149-98b4-df16bc9b81e3" );
            ASSERT_TRUE( CChromeBrowser::ParseDevToolsActivePort( "45123\r\n/devtools/browser/abc\r\n", port, path ) );
            ASSERT_TRUE( port == 45123 && path == "/devtools/browser/abc" );

            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "9222", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "9222\n", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "abc\n/devtools/browser/x", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "0\n/devtools/browser/x", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "70000\n/devtools/browser/x", port, path ) );
            ASSERT_FALSE( CChromeBrowser::ParseDevToolsActivePort( "9222\nnot-a-path", port, path ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The default profile locations of the browsers are known" )
        try
        {
            ASSERT_FALSE( CChromeBrowser::GetDefaultUserDataDirs().empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A configured browser path is used as it is" )
        try
        {
            CChromeBrowser browser;
            ASSERT_TRUE( browser.FindExecutable().IsNULLOrEmpty() || CORE::FileExists( browser.FindExecutable() ) );
            browser.SetExecutablePath( "/some/where/chrome" );
            ASSERT_TRUE( browser.FindExecutable() == "/some/where/chrome" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_TESTCASE( "Test: Chrome is preferred and Chromium is used when there is no Chrome" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Path" );
            CORE::CreateDirs( dir );
            const CORE::CString chromium = CORE::CombinePath( dir, "chromium" );
            const CORE::CString chrome = CORE::CombinePath( dir, "google-chrome-stable" );
            CORE::DeleteFile( chromium );
            CORE::DeleteFile( chrome );

            const CORE::CString originalPath = getenv( "PATH" ) != GUCEF_NULL ? getenv( "PATH" ) : "";
            setenv( "PATH", dir.C_String(), 1 );

            // only chromium: chromium is used
            CORE::WriteStringAsTextFile( chromium, "#!/bin/sh\n" );
            CChromeBrowser browser;
            ASSERT_TRUE( browser.FindExecutable() == chromium );

            // both: chrome wins
            CORE::WriteStringAsTextFile( chrome, "#!/bin/sh\n" );
            ASSERT_TRUE( browser.FindExecutable() == chrome );

            // a configured path wins over both
            browser.SetExecutablePath( "/my/own/browser" );
            ASSERT_TRUE( browser.FindExecutable() == "/my/own/browser" );

            setenv( "PATH", originalPath.C_String(), 1 );
            CORE::DeleteFile( chromium );
            CORE::DeleteFile( chrome );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: The host filter decides which hosts are recorded" )
        try
        {
            std::vector< std::string > terms;
            ASSERT_TRUE( CChromeNetworkRecorder::MatchesFilter( terms, "anything.example.com" ) );
            terms.push_back( "Ancestry" );
            terms.push_back( "127.0.0.1" );
            ASSERT_TRUE( CChromeNetworkRecorder::MatchesFilter( terms, "www.ancestry.com" ) );
            ASSERT_TRUE( CChromeNetworkRecorder::MatchesFilter( terms, "MEDIASVC.ANCESTRY.COM" ) );
            ASSERT_TRUE( CChromeNetworkRecorder::MatchesFilter( terms, "127.0.0.1:8080" ) );
            ASSERT_FALSE( CChromeNetworkRecorder::MatchesFilter( terms, "www.mybank.example" ) );
            ASSERT_FALSE( CChromeNetworkRecorder::MatchesFilter( terms, "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}


/*-------------------------------------------------------------------------*/

static bool
XmlHas( const std::string& text, const std::string& part )
{
    return text.find( part ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

static std::string
DateXmlOf( const char* date )
{
    std::string xml;
    CGrampsDate::ToXml( date, xml );
    return xml;
}

/*-------------------------------------------------------------------------*/

static const char* const GrampsSampleGedcom =
    "0 HEAD\n"
    "1 SOUR Ancestry.com Family Trees\n"
    "2 _TREE Test\n"
    "3 RIN 12345\n"
    "1 DATE 15 Oct 2026\n"
    "2 TIME 10:20:30\n"
    "0 @I1@ INDI\n"
    "1 NAME Jan Willem /van Test/\n"
    "2 GIVN Jan Willem\n"
    "2 SURN van Test\n"
    "1 SEX M\n"
    "1 BIRT\n"
    "2 DATE 1 JAN 1900\n"
    "2 PLAC Meerssen, Limburg, Nederland\n"
    "2 SOUR @S1@\n"
    "3 PAGE Page 1\n"
    "3 _APID 1,2::3\n"
    "1 DEAT\n"
    "2 DATE Abt. 1950\n"
    "2 SOUR @S1@\n"
    "3 PAGE Page 1\n"
    "3 _APID 1,2::3\n"
    "1 SOUR @S404@\n"
    "1 EVEN\n"
    "2 TYPE _NEW\n"
    "2 DATE 25 Feb 2015\n"
    "1 NOTE Fish &amp; chips\n"
    "2 CONT second line\n"
    "1 OBJE @O1@\n"
    "2 _PRIM Y\n"
    "2 _CROP\n"
    "3 _LEFT 50\n"
    "3 _TOP 0\n"
    "3 _WDTH 50\n"
    "3 _HGHT 100\n"
    "1 OBJE @O2@\n"
    "1 FAMS @F1@\n"
    "0 @I2@ INDI\n"
    "1 NAME Marie /Test/\n"
    "1 SEX F\n"
    "1 FAMC @F2@\n"
    "1 FAMS @F1@\n"
    "0 @F1@ FAM\n"
    "1 HUSB @I1@\n"
    "1 WIFE @I2@\n"
    "1 MARR\n"
    "2 DATE 2 Feb 1925\n"
    "2 PLAC Meerssen, Limburg, Nederland\n"
    "0 @S1@ SOUR\n"
    "1 TITL Census 1900\n"
    "0 @O1@ OBJE\n"
    "1 FILE\n"
    "2 FORM jpg\n"
    "3 _WDTH 200\n"
    "3 _HGHT 100\n"
    "2 TITL A photo\n"
    "1 _OID abc-123\n"
    "0 @O2@ OBJE\n"
    "1 FILE\n"
    "2 FORM png\n"
    "2 TITL Not downloaded\n"
    "1 _OID def-456\n"
    "0 TRLR\n";

/*-------------------------------------------------------------------------*/

static void
PerformGrampsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Text is made safe for XML and the HTML escapes of the export are undone" )
        try
        {
            ASSERT_TRUE( CGrampsText::XmlEscape( "a & b < \"c\"" ) == "a &amp; b &lt; &quot;c&quot;" );
            // line breaks have to be escaped in an attribute and can stay in element content
            ASSERT_TRUE( CGrampsText::XmlEscape( "x\ny" ) == "x&#10;y" );
            ASSERT_TRUE( CGrampsText::XmlEscape( "x\ny", false ) == "x\ny" );
            // control characters are not XML, a stray byte is not UTF-8, a good multi byte character stays
            ASSERT_TRUE( CGrampsText::XmlEscape( std::string( "a\x01" "b", 3 ) ) == "ab" );
            ASSERT_TRUE( CGrampsText::XmlEscape( "a\xFF" "b" ) == "a\xEF\xBF\xBD" "b" );
            ASSERT_TRUE( CGrampsText::XmlEscape( "\xC3\xBB" ) == "\xC3\xBB" );
            ASSERT_TRUE( CGrampsText::XmlEscape( "\xC0\x80" ).find( '\0' ) == std::string::npos );    // overlong encoding

            ASSERT_TRUE( CGrampsText::DecodeHtmlEntities( "Fish &amp; chips &lt;b&gt; &#39;x&#39; &quot;" ) == "Fish & chips <b> 'x' \"" );
            ASSERT_TRUE( CGrampsText::DecodeHtmlEntities( "a&nbsp;b" ) == "a b" );
            ASSERT_TRUE( CGrampsText::DecodeHtmlEntities( "AT&T and &unknown; and 5 & 6" ) == "AT&T and &unknown; and 5 & 6" );
            ASSERT_TRUE( CGrampsText::DecodeHtmlEntities( "&#233;" ) == "\xC3\xA9" );

            ASSERT_TRUE( CGrampsText::HtmlToPlainText( "&lt;p&gt;One&lt;/p&gt;&lt;p&gt;Two&lt;/p&gt;" ) == "One\nTwo" );
            ASSERT_TRUE( CGrampsText::HtmlToPlainText( "a &lt; b" ) == "a < b" );
            ASSERT_TRUE( CGrampsText::HtmlToPlainText( "<line></line>" ) == "" );

            ASSERT_TRUE( CGrampsText::HashHex( "x" ).size() == 32 );
            ASSERT_TRUE( CGrampsText::HashHex( "x" ) == CGrampsText::HashHex( "x" ) );
            ASSERT_TRUE( CGrampsText::HashHex( "x" ) != CGrampsText::HashHex( "y" ) );
            ASSERT_TRUE( CGrampsText::MakeId( "S_4/3 0" ) == "S_4_3_0" );
            ASSERT_TRUE( CGrampsText::DaysSinceEpoch( 1970, 1, 1 ) == 0 );
            ASSERT_TRUE( CGrampsText::DaysSinceEpoch( 2000, 3, 1 ) == 11017 );
            ASSERT_TRUE( CGrampsText::DaysSinceEpoch( 1969, 12, 31 ) == -1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: GEDCOM dates become the date elements of Gramps" )
        try
        {
            ASSERT_TRUE( DateXmlOf( "20 Sep 1980" ) == "<dateval val=\"1980-09-20\"/>" );
            ASSERT_TRUE( DateXmlOf( "20 SEPTEMBER 1980" ) == "<dateval val=\"1980-09-20\"/>" );
            ASSERT_TRUE( DateXmlOf( "Sep 1980" ) == "<dateval val=\"1980-09\"/>" );
            ASSERT_TRUE( DateXmlOf( "1980" ) == "<dateval val=\"1980\"/>" );
            ASSERT_TRUE( DateXmlOf( "  1 jan 1900 " ) == "<dateval val=\"1900-01-01\"/>" );
            ASSERT_TRUE( DateXmlOf( "Abt. 1950" ) == "<dateval val=\"1950\" type=\"about\"/>" );
            ASSERT_TRUE( DateXmlOf( "ABT 5 Feb 1950" ) == "<dateval val=\"1950-02-05\" type=\"about\"/>" );
            ASSERT_TRUE( DateXmlOf( "Bef. Nov 1900" ) == "<dateval val=\"1900-11\" type=\"before\"/>" );
            ASSERT_TRUE( DateXmlOf( "Aft. 1900" ) == "<dateval val=\"1900\" type=\"after\"/>" );
            ASSERT_TRUE( DateXmlOf( "EST 1900" ) == "<dateval val=\"1900\" type=\"about\" quality=\"estimated\"/>" );
            ASSERT_TRUE( DateXmlOf( "CAL 1900" ) == "<dateval val=\"1900\" type=\"about\" quality=\"calculated\"/>" );
            ASSERT_TRUE( DateXmlOf( "Bet. 1921\xE2\x80\x93" "1940" ) == "<daterange start=\"1921\" stop=\"1940\"/>" );
            ASSERT_TRUE( DateXmlOf( "BET 1 JAN 1900 AND 5 FEB 1901" ) == "<daterange start=\"1900-01-01\" stop=\"1901-02-05\"/>" );
            ASSERT_TRUE( DateXmlOf( "FROM 1900 TO 1910" ) == "<datespan start=\"1900\" stop=\"1910\"/>" );
            ASSERT_TRUE( DateXmlOf( "44 BC" ) == "<dateval val=\"-0044\"/>" );
            ASSERT_TRUE( DateXmlOf( "25-12-1900" ) == "<dateval val=\"1900-12-25\"/>" );
            ASSERT_TRUE( DateXmlOf( "1900-12-25" ) == "<dateval val=\"1900-12-25\"/>" );
            ASSERT_TRUE( DateXmlOf( "29 Feb 1904" ) == "<dateval val=\"1904-02-29\"/>" );

            // what is not a date stays what it was, as text, and day and month which can be mixed up are not guessed
            ASSERT_TRUE( DateXmlOf( "(ja)" ) == "<datestr val=\"(ja)\"/>" );
            ASSERT_TRUE( DateXmlOf( "30 Feb 1900" ) == "<datestr val=\"30 Feb 1900\"/>" );
            ASSERT_TRUE( DateXmlOf( "3-4-1900" ) == "<datestr val=\"3-4-1900\"/>" );
            ASSERT_TRUE( DateXmlOf( "FROM 1932" ) == "<datestr val=\"FROM 1932\"/>" );
            ASSERT_TRUE( DateXmlOf( "Abt. &amp; x" ) == "<datestr val=\"Abt. &amp; x\"/>" );
            ASSERT_TRUE( DateXmlOf( "" ).empty() );

            std::string xml;
            ASSERT_TRUE( CGrampsDate::ToXml( "", xml ) == GRAMPSDATE_NONE );
            ASSERT_TRUE( CGrampsDate::ToXml( "1900", xml ) == GRAMPSDATE_VALUE );
            ASSERT_TRUE( CGrampsDate::ToXml( "Bet. 1900-1910", xml ) == GRAMPSDATE_RANGE );
            ASSERT_TRUE( CGrampsDate::ToXml( "nonsense", xml ) == GRAMPSDATE_TEXT );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A GEDCOM file is indexed by record and a record is read as a tree" )
        try
        {
            CGedcomFile file;
            file.LoadFromText( std::string( "\xEF\xBB\xBF" "0 HEAD\n1 CHAR UTF-8\n0 @I1@ INDI\n1 NAME Jan /Test/\n1 NOTE one\n2 CONC , two\n2 CONT three\n1 EMAIL a@@b.c\n0 @F1@ FAM\r\n1 HUSB @I1@\r\n0 TRLR\n" ) );
            const std::vector< SGedcomRecordRef >& records = file.GetRecords();
            ASSERT_TRUE( records.size() == 4 );
            ASSERT_TRUE( records[ 0 ].type == "HEAD" && records[ 1 ].type == "INDI" && records[ 1 ].xref == "I1" && records[ 2 ].type == "FAM" && records[ 3 ].type == "TRLR" );
            ASSERT_TRUE( file.FindRecord( "FAM", "F1" ) == &records[ 2 ] );
            ASSERT_TRUE( file.FindRecord( "FAM", "F9" ) == GUCEF_NULL );

            SGedcomNode node;
            ASSERT_TRUE( file.ParseRecord( records[ 1 ], node ) );
            ASSERT_TRUE( node.tag == "INDI" && node.xref == "I1" );
            ASSERT_TRUE( node.children.size() == 3 );
            ASSERT_TRUE( node.ChildValue( "NAME" ) == "Jan /Test/" );
            ASSERT_TRUE( node.ChildValue( "NOTE" ) == "one, two\nthree" );        // CONC joins, CONT breaks
            ASSERT_TRUE( node.ChildValue( "EMAIL" ) == "a@b.c" );                  // @@ is an @
            ASSERT_TRUE( node.Child( "BIRT" ) == GUCEF_NULL && node.ChildValue( "BIRT" ).empty() );

            // the record after it is read on its own, the one before does not leak in
            ASSERT_TRUE( file.ParseRecord( records[ 2 ], node ) );
            ASSERT_TRUE( node.tag == "FAM" && node.children.size() == 1 && node.ChildValue( "HUSB" ) == "@I1@" );
            ASSERT_TRUE( file.ParseRecord( records[ 0 ], node ) );
            ASSERT_TRUE( node.tag == "HEAD" && node.ChildValue( "CHAR" ) == "UTF-8" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The Gramps export holds the people, events, places, citations and media of a GEDCOM file" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_TestApp_Gramps" );
            CORE::CreateDirs( CORE::CombinePath( dir, "media" ) );
            const CORE::CString gedcom = CORE::CombinePath( dir, "tree.ged" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( gedcom, GrampsSampleGedcom ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( CORE::CombinePath( dir, "media" ), "abc-123.jpg" ), "not really a picture" ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( CORE::CombinePath( dir, "media" ), "abc-123.jpg.json" ), "{}" ) );

            SGrampsExportOptions options;
            options.gedcomPath = gedcom;
            options.mediaDir = CORE::CombinePath( dir, "media" );
            options.outputPath = CORE::CombinePath( dir, "out.gramps" );
            options.mediaBasePath = "/data/media";
            options.researcherName = "Jan & Marie";
            SGrampsExportResult result;
            ASSERT_TRUE( CGrampsExporter::Export( options, result ) );

            ASSERT_TRUE( result.people == 2 && result.families == 1 && result.sources == 1 && result.mediaObjects == 2 );
            ASSERT_TRUE( result.mediaFilesPresent == 1 && result.mediaFilesMissing == 1 );
            ASSERT_TRUE( result.events == 3 );                  // birth, death, marriage. The 'added to the tree' marker is an attribute
            ASSERT_TRUE( result.citations == 1 );              // the same citation on birth and death is one citation
            ASSERT_TRUE( result.places == 3 );                 // Nederland, Limburg, Meerssen
            ASSERT_TRUE( result.notes == 1 );
            ASSERT_TRUE( result.danglingReferences == 2 );     // the source S404 and the family F2 are not in the file
            ASSERT_TRUE( result.homePerson == "I1" && result.personIds.size() == 2 && result.gedcomTime == "2026-10-15 10:20:30" );

            std::string xml;
            CORE::CString xmlText;
            ASSERT_TRUE( CORE::LoadTextFileAsString( options.outputPath, xmlText ) );
            xml = xmlText.STL_String();
            ASSERT_TRUE( XmlHas( xml, "<!DOCTYPE database PUBLIC \"-//GRAMPS//DTD GRAMPS XML V1.7.2//EN\"" ) );
            ASSERT_TRUE( XmlHas( xml, "<created date=\"2026-10-15\"" ) );
            ASSERT_TRUE( XmlHas( xml, "<resname>Jan &amp; Marie</resname>" ) );
            ASSERT_TRUE( XmlHas( xml, "<mediapath>/data/media</mediapath>" ) );

            // the order of the sections is not up to us
            size_t last = 0;
            const char* const sections[] = { "<tags>", "<events>", "<people home=", "<families>", "<citations>", "<sources>", "<places>", "<objects>", "<notes>", "</database>" };
            for ( int i=0; i<10; ++i )
            {
                const size_t at = xml.find( sections[ i ] );
                ASSERT_TRUE( at != std::string::npos && at > last );
                last = at;
            }
            ASSERT_TRUE( XmlHas( xml, "id=\"I1\"" ) && XmlHas( xml, "id=\"F1\"" ) && XmlHas( xml, "id=\"S1\"" ) );
            ASSERT_TRUE( XmlHas( xml, "<gender>M</gender><name type=\"Birth Name\"><first>Jan Willem</first><surname>van Test</surname></name>" ) );
            ASSERT_TRUE( XmlHas( xml, "<dateval val=\"1900-01-01\"/>" ) && XmlHas( xml, "<dateval val=\"1950\" type=\"about\"/>" ) && XmlHas( xml, "<dateval val=\"1925-02-02\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<pname value=\"Meerssen\"/><placeref hlink=" ) );
            ASSERT_TRUE( XmlHas( xml, "type=\"Country\"><pname value=\"Nederland\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<page>Page 1</page>" ) && XmlHas( xml, "<srcattribute type=\"Ancestry record id\" value=\"1,2::3\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<attribute type=\"Ancestry: added to the tree\" value=\"25 Feb 2015\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "href=\"https://www.ancestry.com/family-tree/person/tree/12345/person/1/facts\"" ) );
            ASSERT_TRUE( XmlHas( xml, "<text>Fish &amp; chips\nsecond line</text>" ) );
            ASSERT_TRUE( XmlHas( xml, "<rel type=\"Married\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<childof" ) == false );        // the only FAMC points to a family which is not there

            // the media: the primary photo is the first, with the cut out as percentages, and the one without a file is tagged
            ASSERT_TRUE( XmlHas( xml, "<region corner1_x=\"25\" corner1_y=\"0\" corner2_x=\"50\" corner2_y=\"100\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<file src=\"abc-123.jpg\" mime=\"image/jpeg\" description=\"A photo\"/>" ) );
            ASSERT_TRUE( XmlHas( xml, "<file src=\"def-456.png\" mime=\"image/png\" description=\"Not downloaded\"/>" ) );
            const size_t missing = xml.find( "def-456.png" );
            ASSERT_TRUE( xml.find( "<tagref", missing ) != std::string::npos && xml.find( "</object>", missing ) > xml.find( "<tagref", missing ) );
            ASSERT_TRUE( xml.find( "<tagref" ) > xml.find( "<objects>" ) );        // the photo that is there has no tag

            // the same file again gives the same result, byte for byte, and leaves no working files behind
            const CORE::CString second = CORE::CombinePath( dir, "out2.gramps" );
            options.outputPath = second;
            SGrampsExportResult result2;
            ASSERT_TRUE( CGrampsExporter::Export( options, result2 ) );
            CORE::CString xml2Text;
            ASSERT_TRUE( CORE::LoadTextFileAsString( second, xml2Text ) );
            const std::string xml2 = xml2Text.STL_String();
            ASSERT_TRUE( xml == xml2 );
            ASSERT_TRUE( !CORE::FileExists( second + ".people.part" ) && !CORE::FileExists( second + ".tmp" ) );

            // a file that is not there is a failure, not a crash
            options.gedcomPath = CORE::CombinePath( dir, "missing.ged" );
            ASSERT_TRUE( !CGrampsExporter::Export( options, result2 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformToolLogicTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AncestryDotComBackup tool logic TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ToolLogic" );

    PerformDataLayoutTests();
    PerformUrlTemplateTests();
    PerformSettingsTests();
    PerformCatalogTests();
    PerformJsonTests();
    PerformGedcomTests();
    PerformGrampsTests();
    PerformExportFinderTests();
    PerformZipTests();
    PerformPolicyTests();
    PerformPacingAndFeedTests();
    PerformMediaTests();
    PerformDnaTests();
    PerformBrowserTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL AncestryDotComBackup tool logic TESTS COMPLETED" );
}

/*---------------------------------------------------------------------------*/
