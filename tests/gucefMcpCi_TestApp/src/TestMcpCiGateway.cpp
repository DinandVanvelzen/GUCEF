/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#include "TestMcpCiSupport.h"
#include "TestCiServer.h"
#include "TestCiGateway.h"
#include "TestFakeForgejo.h"
#include "TestFakeGatewayFront.h"
#include "TestMcpCiGateway.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace MCPCITEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The secrets of these tests: none of them may appear in any output or log */
const char* ForgejoToken = "fjtok-8d2c4e6a0b1f3d5c7e9a-SECRET";
const char* ProxyToken = "gwtok-3b5d7f9a1c2e4f6b8d0a-SECRET";
const char* AdminToken = "admtok-6e8a0c2d4f1b3d5f7a9c-SECRET";
const char* Remote = "https://example.invalid/acme/widget.git";

/** What every server of these tests answered and logged */
CORE::CString g_transcripts;

/** The example of the lablink hub, as projects/k3s/lablink/ci.json.example has it; the file is read when it is found */
const char* LablinkExample =
    "{\n"
    "  \"ci\": [\n"
    "    {\n"
    "      \"id\": \"GUCEF\",\n"
    "      \"description\": \"GUCEF platform\",\n"
    "      \"backend\": \"forgejo\",\n"
    "      \"url\": \"http://172.16.1.125:32001\",\n"
    "      \"owner\": \"forgejo-admin\",\n"
    "      \"repo\": \"GUCEF\",\n"
    "      \"scratchOwner\": \"ci\",\n"
    "      \"scratchRepo\": \"GUCEF\",\n"
    "      \"workflow\": \"gucef-ci.yml\",\n"
    "      \"matrixDir\": \"projects/CI/matrix\",\n"
    "      \"reportsOwner\": \"ci-bot\",\n"
    "      \"retentionDays\": 14,\n"
    "      \"remotes\": [\n"
    "        \"https://github.com/DinandVanvelzen/GUCEF.git\",\n"
    "        \"git@github.com:DinandVanvelzen/GUCEF.git\",\n"
    "        \"https://github.com/LiberatorUSA/GUCEF.git\",\n"
    "        \"git@github.com:LiberatorUSA/GUCEF.git\",\n"
    "        \"http://172.16.1.125:32001/forgejo-admin/GUCEF.git\",\n"
    "        \"ssh://git@172.16.1.125:32002/forgejo-admin/GUCEF.git\"\n"
    "      ],\n"
    "      \"defaults\": { \"mode\": \"build\", \"toolchains\": [], \"cppStandards\": [], \"products\": [] }\n"
    "    }\n"
    "  ]\n"
    "}\n";

const char* LogBadBuild =
    "-- The C compiler identification is GNU 15.2.0\n"
    "-- The CXX compiler identification is GNU 15.2.0\n"
    "-- Configuring done (0.4s)\n"
    "-- Generating done (0.1s)\n"
    "-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx17-Release/badbuild\n"
    "[1/3] Building CXX object src/CMakeFiles/badbuild.dir/a.cpp.o\n"
    "FAILED: src/CMakeFiles/badbuild.dir/a.cpp.o\n"
    "/usr/bin/g++ -std=c++17 -c /work/GUCEF/src/a.cpp -o src/CMakeFiles/badbuild.dir/a.cpp.o\n"
    "/work/GUCEF/src/a.cpp: In function 'int main()':\n"
    "/work/GUCEF/src/a.cpp:10:5: error: 'foo' was not declared in this scope\n"
    "/work/GUCEF/src/a.cpp:10:5: error: 'foo' was not declared in this scope\n"
    "/work/GUCEF/src/b.cpp:3:1: warning: unused variable 'x' [-Wunused-variable]\n"
    "ninja: build stopped: subcommand failed.\n";

/*-------------------------------------------------------------------------*/

/** The part of a text after 'marker' up to the end of the line or the given character */
CORE::CString
After( const CORE::CString& text, const CORE::CString& marker, char end = '\n' )
{
    const Int32 at = text.HasSubstr( marker, true );
    if ( at < 0 )
        return CORE::CString::Empty;
    CORE::CString rest = text.CutChars( (UInt32) at + marker.Length(), true );
    const Int32 stop = rest.HasChar( end, true );
    const Int32 newline = rest.HasChar( '\n', true );
    Int32 cut = stop;
    if ( newline >= 0 && ( cut < 0 || newline < cut ) )
        cut = newline;
    return cut >= 0 ? rest.SubstrToIndex( (UInt32) cut, true ) : rest;
}

/*-------------------------------------------------------------------------*/

CORE::CString
RepoState( const CORE::CString& repo )
{
    return "status:\n" + GitOut( repo, "status", "--porcelain=v2", "--untracked-files=all" ) + "\nhead:\n" + GitOut( repo, "rev-parse", "HEAD" ) +
           "\nrefs:\n" + GitOut( repo, "for-each-ref" ) + "\nindex:\n" + GitOut( repo, "ls-files", "-s" ) + "\nconfig:\n" + GitOut( repo, "config", "--local", "--list" );
}

/*-------------------------------------------------------------------------*/

/** Content that does not compress */
CORE::CString
NoiseText( UInt32 size, UInt32 seed )
{
    GUCEF::vector< char > buffer( size );
    UInt32 state = seed;
    static const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245u + 12345u;
        buffer[ i ] = letters[ ( state >> 16 ) % 62 ];
    }
    return CORE::CString( &buffer[ 0 ], size );
}

/*-------------------------------------------------------------------------*/

bool
HasAllTools( const CORE::CString& names )
{
    const char* tools[] = { "ci_resolve_repo,", "ci_describe,", "ci_submit,", "ci_status,", "ci_report,", "ci_job_details,", "ci_cancel,", "ci_cleanup," };
    for ( int i=0; i<8; ++i )
        if ( !Contains( names, tools[ i ] ) )
            return false;
    return 8 == names.GetCharacterCount( ',' );
}

/*-------------------------------------------------------------------------*/

bool
StopServer( CTestCiServer& server )
{
    const bool stopped = server.Stop();
    g_transcripts += server.GetTranscript();
    return stopped;
}

/*-------------------------------------------------------------------------*/

bool
StopGateway( CTestCiGateway& gateway )
{
    const bool stopped = gateway.Stop();
    g_transcripts += gateway.GetLog();
    return stopped;
}

/*-------------------------------------------------------------------------*/

Int32
ExitCodeOf( const CORE::CProcess::TStringVector& args )
{
    CORE::CProcess process;
    process.SetIsolateFromParent( true );
    if ( !process.Launch( FindServer(), args, CORE::CString::Empty, ChildEnvironment() ) )
        return -1;
    Int32 exitCode = 0;
    if ( !process.WaitForExit( 30000, exitCode ) )
    {
        process.Terminate( false, 0 );
        return 0;
    }
    return exitCode;
}

/*-------------------------------------------------------------------------*/

UInt64
NowInMs( void )
{
    return MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000 );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
ReadBytes( const CORE::CString& path )
{
    CORE::CDynamicBuffer buffer;
    buffer.LoadContentFromFile( path );
    return buffer;
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
Slice( const CORE::CDynamicBuffer& buffer, UInt32 offset, UInt32 size )
{
    CORE::CDynamicBuffer slice;
    slice.CopyFrom( size, static_cast< const char* >( buffer.GetConstBufferPtr() ) + offset );
    return slice;
}

/*-------------------------------------------------------------------------*/

/** A run that ends with its build cell failed */
CFakeForgejo::TStageVector
FailingRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "waiting", 1 ).AddJob( "plan", "waiting" ) );
    stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "failure" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::TStageVector
EndlessRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "running", 100000000 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "running" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

/** A scratch directory with a working repository, a bare scratch repository, the registry, the tokens of the gateway */
class CGatewayFixture
{
    public:

    explicit CGatewayFixture( const char* name )
        : m_scratch( name )
        , dir( m_scratch.GetPath() )
        , repo( CORE::CombinePath( dir, "work" ) )
        , bare( CORE::CombinePath( dir, "scratch.git" ) )
        , config( CORE::CombinePath( dir, "ci.json" ) )
        , forgejoTokenFile( CORE::CombinePath( dir, "forgejo.token" ) )
        , tokensFile( CORE::CombinePath( dir, "gateway-tokens" ) )
        , proxyTokenFile( CORE::CombinePath( dir, "proxy.token" ) )
        , adminTokenFile( CORE::CombinePath( dir, "admin.token" ) )
        , gatewayLog( CORE::CombinePath( dir, "gateway.log" ) )
        , workDir( CORE::CombinePath( dir, "gateway" ) )
        , logDir( CORE::CombinePath( dir, "logs" ) )
    {
    }

    bool MakeRepos( void )
    {
        CORE::CreateDirs( repo );
        CORE::CreateDirs( bare );
        if ( 0 != Git( bare, "init", "-q", "--bare" ) || 0 != Git( repo, "init", "-q" ) )
            return false;
        Git( repo, "checkout", "-q", "-b", "main" );
        if ( !WriteFile( CORE::CombinePath( repo, "a.txt" ), "alpha\n" ) || !WriteFile( CORE::CombinePath( repo, "b.txt" ), "bravo\n" ) ||
             !WriteFile( CORE::CombinePath( repo, ".gitignore" ), "*.o\n" ) )
            return false;
        return 0 == Git( repo, "add", "-A" ) && GitCommit( repo, "first" ) && 0 == Git( repo, "remote", "add", "origin", Remote );
    }

    /** The registry for the fake Forgejo, the CI token, the tokens file with the proxy as testbox, the tokens of proxy and admin */
    bool WriteConfig( const CORE::CString& forgejoUrl )
    {
        const CORE::CString json = "{\"ci\":[{\"id\":\"WIDGET\",\"description\":\"the widget test project\",\"backend\":\"forgejo\",\"url\":" + Json( forgejoUrl ) +
                                   ",\"owner\":\"acme\",\"repo\":\"widget\",\"scratchOwner\":\"ci\",\"scratchRepo\":\"widget\",\"scratchUrl\":" + Json( bare ) +
                                   ",\"workflow\":\"gucef-ci.yml\",\"retentionDays\":0," +
                                   "\"remotes\":[\"https://example.invalid/acme/widget.git\",\"git@example.invalid:acme/widget.git\"]}]}";
        return WriteFile( config, json ) && WriteOwnerOnlyFile( forgejoTokenFile, CORE::CString( ForgejoToken ) + "\n" ) &&
               WriteOwnerOnlyFile( tokensFile, "# machines of the lab\n" + TokenLine( ProxyToken, "testbox" ) ) &&
               WriteOwnerOnlyFile( proxyTokenFile, CORE::CString( ProxyToken ) + "\n" ) && WriteOwnerOnlyFile( adminTokenFile, CORE::CString( AdminToken ) + "\n" );
    }

    CORE::CProcess::TStringVector GatewayArgs( void ) const
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "--mode" ); args.push_back( "gateway" );
        args.push_back( "--ci-config" ); args.push_back( config );
        args.push_back( "--forgejo-token-file" ); args.push_back( forgejoTokenFile );
        args.push_back( "--gateway-tokens-file" ); args.push_back( tokensFile );
        args.push_back( "--token-file" ); args.push_back( adminTokenFile );
        args.push_back( "--lablink-ci-dir" ); args.push_back( "none" );
        args.push_back( "--log-dir" ); args.push_back( CORE::CombinePath( dir, "gateway-logs" ) );
        args.push_back( "--work-dir" ); args.push_back( workDir );
        args.push_back( "--poll-interval-ms" ); args.push_back( "200" );
        args.push_back( "--max-wait-seconds" ); args.push_back( "30" );
        args.push_back( "--upload-chunk-kb" ); args.push_back( "64" );
        return args;
    }

    CORE::CProcess::TStringVector ProxyArgs( const CORE::CString& gatewayUrl ) const
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "--mode" ); args.push_back( "proxy" );
        args.push_back( "--gateway-url" ); args.push_back( gatewayUrl );
        args.push_back( "--gateway-token-file" ); args.push_back( proxyTokenFile );
        args.push_back( "--lablink-ci-dir" ); args.push_back( "none" );
        args.push_back( "--log-dir" ); args.push_back( logDir );
        args.push_back( "--machine-name" ); args.push_back( "TestBox" );
        args.push_back( "--max-wait-seconds" ); args.push_back( "30" );
        return args;
    }

    /** The branches of the bare scratch repository, one per line */
    CORE::CString Branches( void ) const
    {
        return GitOut( bare, "for-each-ref", "--format=%(refname:short)", "refs/heads/scratch" );
    }

    /** The scratch branch that is there now and was not in 'before' (a list of Branches()); empty when there is none */
    CORE::CString NewBranch( const CORE::CString& before ) const
    {
        const CORE::CString::StringVector old = before.ParseElements( '\n', false );
        const CORE::CString::StringVector now = Branches().ParseElements( '\n', false );
        for ( size_t i=0; i<now.size(); ++i )
        {
            bool known = false;
            for ( size_t o=0; o<old.size() && !known; ++o )
                known = Trimmed( old[ o ] ) == Trimmed( now[ i ] );
            if ( !known )
                return Trimmed( now[ i ] );
        }
        return CORE::CString::Empty;
    }

    /** A bundle of the commit of 'ref' of the working repository as a snapshot bundle carries it, without 'excluded' */
    bool MakeBundle( const CORE::CString& path, const char* ref, const char* excluded = GUCEF_NULL ) const
    {
        Git( repo, "branch", "-f", "snapshot", ref );
        const bool made = GUCEF_NULL == excluded ? 0 == Git( repo, "bundle", "create", "-q", path.C_String(), "refs/heads/snapshot" )
                                                 : 0 == Git( repo, "bundle", "create", "-q", path.C_String(), "refs/heads/snapshot", ( CORE::CString( "^" ) + excluded ).C_String() );
        Git( repo, "branch", "-D", "-q", "snapshot" );
        return made;
    }

    private:

    TESTFW::CScopedTestDir m_scratch;

    public:

    const CORE::CString dir;
    const CORE::CString repo;
    const CORE::CString bare;
    const CORE::CString config;
    const CORE::CString forgejoTokenFile;
    const CORE::CString tokensFile;
    const CORE::CString proxyTokenFile;
    const CORE::CString adminTokenFile;
    const CORE::CString gatewayLog;
    const CORE::CString workDir;
    const CORE::CString logDir;
};

/*-------------------------------------------------------------------------*/

/** Submits through a proxy and gives the run id; empty when that failed */
CORE::CString
Submit( CTestCiServer& server, const CGatewayFixture& fixture, const CORE::CString& extraArguments, CORE::CString& text )
{
    bool isError = true;
    if ( !server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + extraArguments + "}", text, isError, 180000 ) || isError )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi_TestApp: ci_submit through the gateway failed: " << text );
        return CORE::CString::Empty;
    }
    return After( text, "Run id: " );
}

/*-------------------------------------------------------------------------*/

/** The text of the lablink example: the file of the repository when it is found, else the copy */
CORE::CString
LablinkExampleText( void )
{
    const char* candidates[] = { "$MODULEDIR$/../../../../projects/k3s/lablink/ci.json.example", "$MODULEDIR$/../../../../../projects/k3s/lablink/ci.json.example" };
    for ( int i=0; i<2; ++i )
    {
        CORE::CString text;
        if ( ReadFile( CORE::RelativePath( candidates[ i ] ), text ) && Contains( text, "\"ci\"" ) )
            return text;
    }
    return LablinkExample;
}

/*-------------------------------------------------------------------------*/

/** ci.json as the hub writes it: the gateway object first, then the registry unchanged */
CORE::CString
HubCiJson( const CORE::CString& registry, const CORE::CString& gatewayUrl )
{
    const Int32 brace = registry.HasChar( '{', true );
    return "{\n  \"gateway\": { \"url\": " + Json( gatewayUrl ) + ", \"caFile\": \"ca.crt\" },\n" + registry.CutChars( (UInt32) brace + 1, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpCiGatewayTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "McpCiGateway" );

    const bool hasServer = !FindServer().IsNULLOrEmpty() && McpTestJsonAvailable();
    const bool hasGit = GitAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: ci.json of the lablink hub with its gateway object loads in direct, proxy and auto mode; auto picks proxy, direct or no tools from the lablink files; unsafe gateways are refused" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayConfig" );
            const CORE::CString gatewayUrl = "http://127.0.0.1:" + CORE::UInt16ToString( TESTFW::CTlsTestServer::FindFreePort() );
            const CORE::CString hubJson = HubCiJson( LablinkExampleText(), gatewayUrl );

            // the files of a machine in proxy mode: ci.json, gateway.token and ca.crt, no CI token
            const CORE::CString proxyDir = CORE::CombinePath( fixture.dir, "lablink-proxy" );
            CORE::CreateDirs( proxyDir );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( proxyDir, "ci.json" ), hubJson ) );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( proxyDir, "gateway.token" ), Sha256Hex( "x", 1 ) + "\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( proxyDir, "ca.crt" ), "not used over http\n" ) );
            const char* modes[] = { "auto", "proxy" };
            for ( int m=0; m<2; ++m )
            {
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--mode" ); args.push_back( modes[ m ] );
                args.push_back( "--lablink-ci-dir" ); args.push_back( proxyDir );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( HasAllTools( server.ToolNames() ) );
                const CORE::CString instructions = server.Instructions();
                ASSERT_TRUE( Contains( instructions, "CI ids: GUCEF" ) );
                ASSERT_TRUE( Contains( instructions, "The CI gateway at " + gatewayUrl + " cannot be reached at the moment" ) );
                ASSERT_TRUE( StopServer( server ) );
            }

            // the same ci.json with a CI token: direct when asked for, or when there is no gateway token
            const CORE::CString directDir = CORE::CombinePath( fixture.dir, "lablink-direct" );
            CORE::CreateDirs( directDir );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( directDir, "ci.json" ), hubJson ) );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( directDir, "forgejo.token" ), CORE::CString( ForgejoToken ) + "\n" ) );
            const char* directModes[] = { "auto", "direct" };
            for ( int m=0; m<2; ++m )
            {
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--mode" ); args.push_back( directModes[ m ] );
                args.push_back( "--lablink-ci-dir" ); args.push_back( directDir );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( HasAllTools( server.ToolNames() ) );
                const CORE::CString instructions = server.Instructions();
                ASSERT_TRUE( Contains( instructions, "CI ids: GUCEF." ) && !Contains( instructions, "gateway" ) );
                CORE::CString text;
                bool isError = true;
                ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[\"git@github.com:DinandVanvelzen/GUCEF.git\"]}", text, isError ) );
                ASSERT_TRUE( !isError && Contains( text, "CI id: GUCEF (GUCEF platform)" ) );
                ASSERT_TRUE( StopServer( server ) );
            }

            // the proxy mode with the same directory names the CI token nowhere: proxy wins over direct in auto mode
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( directDir, "gateway.token" ), Sha256Hex( "y", 1 ) + "\n" ) );
            {
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--lablink-ci-dir" ); args.push_back( directDir );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( Contains( server.Instructions(), "The CI gateway at " + gatewayUrl ) );
                ASSERT_TRUE( StopServer( server ) );
            }

            // nothing: no tools, and the instructions say what is missing
            const CORE::CString emptyDir = CORE::CombinePath( fixture.dir, "lablink-empty" );
            CORE::CreateDirs( emptyDir );
            {
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--lablink-ci-dir" ); args.push_back( emptyDir );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( server.ToolNames().IsNULLOrEmpty() );
                const CORE::CString instructions = server.Instructions();
                ASSERT_TRUE( Contains( instructions, "No CI is set up for this machine" ) && Contains( instructions, "no CI gateway" ) );
                ASSERT_TRUE( StopServer( server ) );
            }

            // refused: plain http to another machine, a gateway token others can read, proxy without a gateway
            const CORE::CString unsafeDir = CORE::CombinePath( fixture.dir, "lablink-unsafe" );
            CORE::CreateDirs( unsafeDir );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( unsafeDir, "ci.json" ), HubCiJson( LablinkExampleText(), "http://ci-gateway.invalid:8792" ) ) );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( unsafeDir, "gateway.token" ), Sha256Hex( "z", 1 ) + "\n" ) );
            CORE::CProcess::TStringVector unsafe;
            unsafe.push_back( "--lablink-ci-dir" ); unsafe.push_back( unsafeDir );
            ASSERT_TRUE( 0 != ExitCodeOf( unsafe ) );
            const CORE::CString openToken = CORE::CombinePath( fixture.dir, "open.token" );
            ASSERT_TRUE( WriteFile( openToken, ProxyToken ) );
            CORE::CProcess::TStringVector open;
            open.push_back( "--mode" ); open.push_back( "proxy" );
            open.push_back( "--gateway-url" ); open.push_back( gatewayUrl );
            open.push_back( "--gateway-token-file" ); open.push_back( openToken );
            open.push_back( "--lablink-ci-dir" ); open.push_back( "none" );
            ASSERT_TRUE( 0 != ExitCodeOf( open ) );
            CORE::CProcess::TStringVector noGateway;
            noGateway.push_back( "--mode" ); noGateway.push_back( "proxy" );
            noGateway.push_back( "--lablink-ci-dir" ); noGateway.push_back( directDir + "-missing" );
            ASSERT_TRUE( 0 != ExitCodeOf( noGateway ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Proxy to gateway to the fake Forgejo over loopback HTTP: unpushed, modified and untracked work as a full bundle, then incremental ones against have commits; status, report, job details with the log saved here, cancel, cleanup; the repository stays as it was" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayChain" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( ForgejoToken, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( fixture.GatewayArgs(), fixture.gatewayLog ) );

            // a commit that is not pushed, with the CI files; then work that is not committed
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "d.txt" ), "delta\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/matrix.json" ),
                "{\"cppStandards\":[\"98\",\"17\"],\"toolchains\":[{\"id\":\"gcc-linux-x64\",\"enabled\":true}],\"cells\":[{\"id\":\"a\"},{\"id\":\"b\"}]}" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/products.json" ), "{\"products\":[{\"name\":\"platform\"}]}" ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "-A" ) && GitCommit( fixture.repo, "second" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha changed\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "new.txt" ), "new\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "x.o" ), "object\n" ) );
            const CORE::CString head = GitOut( fixture.repo, "rev-parse", "HEAD" );
            const CORE::CString before = RepoState( fixture.repo );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ProxyArgs( gateway.GetUrl() ) ) );
            ASSERT_TRUE( HasAllTools( server.ToolNames() ) );
            ASSERT_TRUE( Contains( server.Instructions(), "CI ids: WIDGET. The CI is reached through the gateway " + gateway.GetUrl() ) );
            CORE::CString text;
            bool isError = true;

            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "CI id: WIDGET (the widget test project)" ) && Contains( text, "Matched remote: " + CORE::CString( Remote ) ) );
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[\"https://example.invalid/acme/other.git\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "No CI is set up for this repository" ) );

            // the options from the CI files of this working tree, which the gateway cannot see
            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "C++ standards: 98, 17" ) && Contains( text, "Products: platform" ) && Contains( text, "= 2 jobs" ) );
            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"WIDGET\"}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "pass repo_dir" ) );
            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"NOPE\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown CI id \"NOPE\"" ) );

            // filters are checked against the CI files here, a dry run sends nothing
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + ",\"cpp_standards\":[\"03\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "cpp_standards 03 (known: 98, 17)" ) );
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + ",\"dry_run\":true}", text, isError, 120000 ) );
            ASSERT_TRUE( !isError && Contains( text, "Dry run: nothing was sent" ) && Contains( text, "1 modified, 1 added (1 untracked)" ) );
            ASSERT_TRUE( fixture.Branches().IsNULLOrEmpty() && 0 == fake.GetDispatchCount() );

            // the first snapshot: nothing to build on, so with its whole history
            fake.ScriptNextRun( FailingRun() );
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_TRUE( runId == "WIDGET:" + CORE::Int64ToString( fake.GetLastRunId() ) );
            ASSERT_TRUE( Contains( text, "Next: ci_status with run_id \"" + runId + "\"" ) );
            const CORE::CString first = fixture.NewBranch( CORE::CString::Empty );
            ASSERT_TRUE( first.StartsWith( "scratch/testbox-" ) );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( first + ":a.txt" ).C_String() ) == "alpha changed" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( first + ":new.txt" ).C_String() ) == "new" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( first + ":d.txt" ).C_String() ) == "delta" );
            ASSERT_TRUE( 0 != Git( fixture.bare, "cat-file", "-e", ( first + ":x.o" ).C_String() ) );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( first + "^" ).C_String() ) == head );
            ASSERT_TRUE( Contains( fake.GetLastDispatchBody(), first ) );
            ASSERT_TRUE( RepoState( fixture.repo ) == before );

            // status with a wait, the report and the details of the failed job, its log saved on this machine
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + ",\"wait_seconds\":20}", text, isError, 60000 ) );
            ASSERT_TRUE( !isError && Contains( text, ": failed" ) && Contains( text, "Failed: gcc-linux-x64-cxx17-Release" ) );
            const CORE::CString version = CORE::Int64ToString( fake.GetLastRunId() ) + "-gcc-linux-x64-cxx17-Release";
            const CORE::CString report = "{\"schema\":1,\"cell\":\"gcc-linux-x64-cxx17-Release\",\"compiler\":\"gcc\",\"cppStandard\":\"17\",\"mode\":\"build\",\"status\":\"failed\","
                                         "\"products\":[{\"name\":\"badbuild\",\"status\":\"failed\",\"phase\":\"build\",\"exitCode\":1,\"logFile\":\"badbuild.log.gz\",\"logTail\":" +
                                         Json( LogBadBuild ) + "}]}";
            fake.AddPackageFile( "ci-bot", "ci-reports", version, "report.json", report, CORE::CString::Empty );
            fake.AddPackageFile( "ci-bot", "ci-reports", version, "badbuild.log", LogBadBuild, CORE::CString::Empty );
            ASSERT_TRUE( server.Call( "ci_report", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "    product badbuild: build failed (exit 1)\n      src/a.cpp:10:5: 'foo' was not declared in this scope\n" ) );
            ASSERT_TRUE( Contains( text, "Next: ci_job_details with run_id \"" + runId + "\" and job_id \"gcc-linux-x64-cxx17-Release\"" ) );
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"gcc-linux-x64-cxx17-Release\"}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "Errors: 2 (1 distinct)" ) && Contains( text, "src/a.cpp:10:5: 'foo' was not declared in this scope [x2]" ) );
            const CORE::CString savedPath = After( After( text, "== product badbuild (", '\n' ), "full log: ", ')' );
            ASSERT_TRUE( savedPath.StartsWith( fixture.logDir ) );
            CORE::CString saved;
            ASSERT_TRUE( ReadFile( savedPath, saved ) && saved == CORE::CString( LogBadBuild ) );
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"gcc-linux-x64-cxx17-Release\",\"part\":\"log_tail\",\"limit\":1}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "ninja: build stopped: subcommand failed." ) && !Contains( text, "Configuring done" ) );
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"nope\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown job id \"nope\"" ) );
            const char* unknown[] = { "WIDGET:999", "nonsense", "OTHER:1" };
            for ( int i=0; i<3; ++i )
            {
                ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( unknown[ i ] ) + "}", text, isError ) );
                ASSERT_TRUE( isError && Contains( text, "Unknown run id" ) );
            }

            // the scratch repository has the published commit as its default branch: the next snapshot builds on it
            ASSERT_TRUE( 0 == Git( fixture.repo, "push", "-q", fixture.bare.C_String(), "HEAD:refs/heads/main" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha again\n" ) );
            fake.ScriptNextRun( EndlessRun() );
            const CORE::CString branchesBefore = fixture.Branches();
            const CORE::CString second = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( second.IsNULLOrEmpty() || second == runId );
            const CORE::CString secondBranch = fixture.NewBranch( branchesBefore );
            ASSERT_TRUE( secondBranch != first && GitOut( fixture.bare, "show", ( secondBranch + ":a.txt" ).C_String() ) == "alpha again" );
            ASSERT_TRUE( server.Call( "ci_cancel", "{\"run_id\":" + Json( second ) + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "Cancelling the run was asked for" ) );

            // a published commit the gateway has never seen: it fetches it from the scratch repository to take the bundle in
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "e.txt" ), "echo\n" ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "e.txt" ) && GitCommit( fixture.repo, "third" ) );
            const CORE::CString published = GitOut( fixture.repo, "rev-parse", "HEAD" );
            ASSERT_TRUE( 0 == Git( fixture.repo, "push", "-q", "-f", fixture.bare.C_String(), "HEAD:refs/heads/main" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "b.txt" ), "bravo changed\n" ) );
            const CORE::CString branchesBeforeThird = fixture.Branches();
            const CORE::CString third = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( third.IsNULLOrEmpty() );
            const CORE::CString thirdBranch = fixture.NewBranch( branchesBeforeThird );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( thirdBranch + ":b.txt" ).C_String() ) == "bravo changed" );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( thirdBranch + "^" ).C_String() ) == published );
            ASSERT_TRUE( GitOut( CORE::CombinePath( fixture.workDir, "repos/widget.git" ), "rev-parse", "refs/remotes/scratch/main" ) == published );

            ASSERT_TRUE( server.Call( "ci_cleanup", "{\"ci_id\":\"WIDGET\",\"older_than_days\":30}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "Removed 0 scratch branch(es)" ) );
            ASSERT_TRUE( server.Call( "ci_cleanup", "{\"ci_id\":\"NOPE\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown CI id" ) );

            ASSERT_TRUE( 0 == fake.GetUnauthorizedCount() );
            ASSERT_TRUE( StopServer( server ) );
            ASSERT_TRUE( StopGateway( gateway ) );

            // the first bundle had the whole history, the others were deltas against commits the gateway named
            ASSERT_TRUE( Contains( g_transcripts, "with its whole history" ) && Contains( g_transcripts, "against 1 base commit(s)" ) );
            ASSERT_TRUE( Contains( gateway.GetLog(), "is on scratch/testbox-" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The gateway over HTTPS with the test certificates: lablink files with a relative caFile; a proxy that does not trust the certificate gets a clear error" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
            TESTFW::CTlsTestCerts certs;
            GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayTls" );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            CORE::CProcess::TStringVector args = fixture.GatewayArgs();
            args.push_back( "--tls-cert" ); args.push_back( certs.GetPath( "good.crt" ) );
            args.push_back( "--tls-key" ); args.push_back( certs.GetPath( "good.key" ) );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( args, fixture.gatewayLog, true ) );
            ASSERT_TRUE( gateway.GetUrl().StartsWith( "https://localhost:" ) );

            // the files lablink writes: ci.json with only the gateway and no CI entries, the token, the CA
            const CORE::CString lablinkDir = CORE::CombinePath( fixture.dir, "lablink" );
            CORE::CreateDirs( lablinkDir );
            CORE::CString ca;
            ASSERT_TRUE( ReadFile( certs.GetPath( "ca.crt" ), ca ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( lablinkDir, "ca.crt" ), ca ) );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( lablinkDir, "ci.json" ), HubCiJson( "{\"ci\":[]}", gateway.GetUrl() ) ) );
            ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( lablinkDir, "gateway.token" ), CORE::CString( ProxyToken ) + "\n" ) );
            CTestCiServer server;
            CORE::CProcess::TStringVector proxyArgs;
            proxyArgs.push_back( "--lablink-ci-dir" ); proxyArgs.push_back( lablinkDir );
            proxyArgs.push_back( "--log-dir" ); proxyArgs.push_back( fixture.logDir );
            ASSERT_TRUE( server.Start( proxyArgs ) );
            ASSERT_TRUE( Contains( server.Instructions(), "CI ids: WIDGET. The CI is reached through the gateway " + gateway.GetUrl() ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "CI id: WIDGET" ) );
            ASSERT_TRUE( StopServer( server ) );

            // an authority that did not sign the certificate of the gateway
            CTestCiServer untrusting;
            CORE::CProcess::TStringVector otherArgs = fixture.ProxyArgs( gateway.GetUrl() );
            otherArgs.push_back( "--ca-file" ); otherArgs.push_back( certs.GetPath( "other_ca.crt" ) );
            ASSERT_TRUE( untrusting.Start( otherArgs ) );
            ASSERT_TRUE( Contains( untrusting.Instructions(), "cannot be reached at the moment" ) );
            const UInt64 start = NowInMs();
            ASSERT_TRUE( untrusting.Call( "ci_resolve_repo", "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "cannot be reached (tls" ) );
            ASSERT_TRUE( NowInMs() - start < 20000 );
            ASSERT_TRUE( StopServer( untrusting ) );

            // the gateway speaks no plain HTTP on its TLS port
            CORE::CString answer;
            ASSERT_TRUE( 200 != GatewayRequest( "http://127.0.0.1:" + CORE::UInt16ToString( gateway.GetPort() ), ProxyToken, "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 200 == GatewayRequest( gateway.GetUrl(), ProxyToken, "GET", "/v1/ci", CORE::CDynamicBuffer(), answer, certs.GetPath( "ca.crt" ) ) );
            ASSERT_TRUE( StopGateway( gateway ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Gateway tokens: none, a wrong one, the static one, one in the query, a machine revoked while the gateway runs; unknown calls and methods" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayTokens" );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( fixture.GatewayArgs(), fixture.gatewayLog ) );
            const CORE::CString url = gateway.GetUrl();
            CORE::CString answer;
            ASSERT_TRUE( 401 == GatewayRequest( url, CORE::CString::Empty, "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "token" );
            ASSERT_TRUE( 401 == GatewayRequest( url, "wrong-token", "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 401 == GatewayRequest( url, Sha256Hex( ProxyToken, CORE::CString( ProxyToken ).Length() ), "GET", "/v1/ci", CORE::CString::Empty, answer ) );    // the digest is no token
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( Contains( answer, "WIDGET" ) && Contains( answer, "chunkBytes" ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, AdminToken, "GET", "/v1/ci?id=WIDGET", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( Contains( answer, "\"backend\"" ) );
            ASSERT_TRUE( 400 == GatewayRequest( url, ProxyToken, "GET", "/v1/ci?access_token=abc", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 404 == GatewayRequest( url, ProxyToken, "GET", "/v1/ci?id=NOPE", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "unknown_ci" );
            ASSERT_TRUE( 404 == GatewayRequest( url, ProxyToken, "GET", "/v1/nothing", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 405 == GatewayRequest( url, ProxyToken, "DELETE", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 400 == GatewayRequest( url, ProxyToken, "POST", "/v1/resolve", "not json", answer ) );

            // revoked: its line leaves the tokens file, and the gateway refuses it soon after
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ProxyArgs( url ) ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( WriteOwnerOnlyFile( fixture.tokensFile, "# testbox was revoked\n" + TokenLine( "another-token-of-another-machine", "otherbox" ) ) );
            Int32 status = 200;
            for ( int i=0; i<100 && 401 != status; ++i )
            {
                MT::ThreadDelay( 100 );
                status = GatewayRequest( url, ProxyToken, "GET", "/v1/ci", CORE::CString::Empty, answer );
            }
            ASSERT_TRUE( 401 == status );
            ASSERT_TRUE( 200 == GatewayRequest( url, "another-token-of-another-machine", "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, AdminToken, "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "refused the token of this machine" ) );
            ASSERT_TRUE( StopServer( server ) );
            ASSERT_TRUE( StopGateway( gateway ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Uploads: too large, a bad SHA-256, truncated, chunks out of order or too large, a bundle against an unknown base, a damaged bundle, the most uploads at a time, abandoned uploads removed with their files" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayUploads" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( ForgejoToken, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            CORE::CProcess::TStringVector args = fixture.GatewayArgs();
            args.push_back( "--max-snapshot-mb" ); args.push_back( "1" );
            args.push_back( "--max-uploads" ); args.push_back( "2" );
            args.push_back( "--upload-idle-seconds" ); args.push_back( "2" );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( args, fixture.gatewayLog ) );
            const CORE::CString url = gateway.GetUrl();
            CORE::CString answer;

            // a snapshot of three chunks
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "noise.dat" ), NoiseText( 200 * 1024, 7 ) ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "noise.dat" ) && GitCommit( fixture.repo, "noise" ) );
            const CORE::CString bundlePath = CORE::CombinePath( fixture.dir, "snapshot.bundle" );
            ASSERT_TRUE( fixture.MakeBundle( bundlePath, "HEAD" ) );
            const CORE::CDynamicBuffer bundle = ReadBytes( bundlePath );
            const UInt32 size = bundle.GetDataSize();
            ASSERT_TRUE( size > 2 * 65536 && size < 3 * 65536 );
            const CORE::CString sha = Sha256Hex( bundle.GetConstBufferPtr(), size );
            const CORE::CString sizeText = CORE::UInt32ToString( size );

            ASSERT_TRUE( 413 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=2097152&sha256=" + sha, "{}", answer ) );
            ASSERT_TRUE( Contains( JsonValue( answer, "error" ), "more than the limit of the gateway of 1.0 MiB" ) );
            ASSERT_TRUE( 400 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + sizeText + "&sha256=xyz", "{}", answer ) );
            ASSERT_TRUE( 404 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=NOPE&size=" + sizeText + "&sha256=" + sha, "{}", answer ) );

            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + sizeText + "&sha256=" + sha, "{}", answer ) );
            const CORE::CString upload = JsonValue( answer, "id" );
            ASSERT_TRUE( upload.Length() == 32 && JsonValue( answer, "chunkBytes" ) == "65536" );
            const CORE::CString chunkPath = "/v1/uploads?id=" + upload + "&offset=";
            ASSERT_TRUE( 409 == GatewayRequest( url, ProxyToken, "PUT", chunkPath + "65536", Slice( bundle, 65536, 65536 ), answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "offset" && JsonValue( answer, "received" ) == "0" );
            ASSERT_TRUE( 413 == GatewayRequest( url, ProxyToken, "PUT", chunkPath + "0", Slice( bundle, 0, 65537 ), answer ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", chunkPath + "0", Slice( bundle, 0, 65536 ), answer ) );
            ASSERT_TRUE( JsonValue( answer, "received" ) == "65536" );
            ASSERT_TRUE( 404 == GatewayRequest( url, AdminToken, "GET", "/v1/uploads?id=" + upload, CORE::CString::Empty, answer ) );     // another machine
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "GET", "/v1/uploads?id=" + upload, CORE::CString::Empty, answer ) );
            ASSERT_TRUE( JsonValue( answer, "received" ) == "65536" && JsonValue( answer, "size" ) == sizeText );

            // truncated: the commit waits for all of it
            ASSERT_TRUE( 409 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads/commit?id=" + upload, "{}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "incomplete" && Contains( JsonValue( answer, "error" ), "65536 of " + sizeText ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", chunkPath + "65536", Slice( bundle, 65536, 65536 ), answer ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", chunkPath + "131072", Slice( bundle, 131072, size - 131072 ), answer ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads/commit?id=" + upload, "{}", answer ) );
            ASSERT_TRUE( Contains( answer, GitOut( fixture.repo, "rev-parse", "HEAD" ) ) && Contains( answer, "scratch/testbox-" ) );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", fixture.NewBranch( CORE::CString::Empty ).C_String() ) == GitOut( fixture.repo, "rev-parse", "HEAD" ) );
            ASSERT_TRUE( 404 == GatewayRequest( url, ProxyToken, "GET", "/v1/uploads?id=" + upload, CORE::CString::Empty, answer ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( fixture.workDir, "uploads/" + upload + ".bundle" ) ) );

            // a run on the snapshot, once: asked again it is the same run
            fake.ScriptNextRun( EndlessRun() );
            const CORE::CString runBody = "{\"snapshot\":" + Json( upload ) + ",\"options\":{\"mode\":\"build\"}}";
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/runs", runBody, answer ) );
            const CORE::CString runId = JsonValue( answer, "runId" );
            ASSERT_TRUE( runId == "WIDGET:" + CORE::Int64ToString( fake.GetLastRunId() ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "POST", "/v1/runs", runBody, answer ) );
            ASSERT_TRUE( JsonValue( answer, "runId" ) == runId && 1 == fake.GetDispatchCount() );
            ASSERT_TRUE( 404 == GatewayRequest( url, AdminToken, "POST", "/v1/runs", runBody, answer ) );         // not the snapshot of the admin
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "GET", "/v1/runs?id=" + runId, CORE::CString::Empty, answer ) );
            ASSERT_TRUE( JsonValue( answer, "state" ) == "running" );

            // the bytes are not those of the SHA-256 the upload announced
            const CORE::CString zeros = "0000000000000000000000000000000000000000000000000000000000000000";
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=100&sha256=" + zeros, "{}", answer ) );
            const CORE::CString badSha = JsonValue( answer, "id" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", "/v1/uploads?id=" + badSha + "&offset=0", Slice( bundle, 0, 100 ), answer ) );
            ASSERT_TRUE( 422 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads/commit?id=" + badSha, "{}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "bad_sha256" );

            // a bundle against a commit nobody has
            const CORE::CString other = CORE::CombinePath( fixture.dir, "other" );
            CORE::CreateDirs( other );
            ASSERT_TRUE( 0 == Git( other, "init", "-q" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( other, "x.txt" ), "x\n" ) && 0 == Git( other, "add", "-A" ) && GitCommit( other, "x" ) );
            const CORE::CString unknownBase = GitOut( other, "rev-parse", "HEAD" );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( other, "y.txt" ), "y\n" ) && 0 == Git( other, "add", "-A" ) && GitCommit( other, "y" ) );
            ASSERT_TRUE( 0 == Git( other, "branch", "snapshot" ) );
            const CORE::CString deltaPath = CORE::CombinePath( fixture.dir, "delta.bundle" );
            ASSERT_TRUE( 0 == Git( other, "bundle", "create", "-q", deltaPath.C_String(), "refs/heads/snapshot", ( "^" + unknownBase ).C_String() ) );
            const CORE::CDynamicBuffer delta = ReadBytes( deltaPath );
            const CORE::CString deltaSha = Sha256Hex( delta.GetConstBufferPtr(), delta.GetDataSize() );
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + CORE::UInt32ToString( delta.GetDataSize() ) + "&sha256=" + deltaSha, "{}", answer ) );
            const CORE::CString deltaUpload = JsonValue( answer, "id" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", "/v1/uploads?id=" + deltaUpload + "&offset=0", delta, answer ) );
            ASSERT_TRUE( 422 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads/commit?id=" + deltaUpload, "{}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "unknown_base" && Contains( JsonValue( answer, "error" ), "unknown base" ) );

            // bytes that are no bundle
            const CORE::CString junk = NoiseText( 5000, 3 );
            CORE::CDynamicBuffer junkBytes;
            junkBytes.CopyFrom( junk.Length(), junk.C_String() );
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=5000&sha256=" + Sha256Hex( junk.C_String(), 5000 ), "{}", answer ) );
            const CORE::CString junkUpload = JsonValue( answer, "id" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", "/v1/uploads?id=" + junkUpload + "&offset=0", junkBytes, answer ) );
            ASSERT_TRUE( 422 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads/commit?id=" + junkUpload, "{}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "bad_bundle" );

            // at most two at a time; one given up, one abandoned and removed with its file
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + sizeText + "&sha256=" + sha, "{}", answer ) );
            const CORE::CString given = JsonValue( answer, "id" );
            ASSERT_TRUE( 201 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + sizeText + "&sha256=" + sha, "{}", answer ) );
            const CORE::CString abandoned = JsonValue( answer, "id" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "PUT", "/v1/uploads?id=" + abandoned + "&offset=0", Slice( bundle, 0, 65536 ), answer ) );
            ASSERT_TRUE( 503 == GatewayRequest( url, ProxyToken, "POST", "/v1/uploads?ci=WIDGET&size=" + sizeText + "&sha256=" + sha, "{}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "uploads_full" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ProxyToken, "DELETE", "/v1/uploads?id=" + given, CORE::CString::Empty, answer ) );
            const CORE::CString abandonedFile = CORE::CombinePath( fixture.workDir, "uploads/" + abandoned + ".bundle" );
            ASSERT_TRUE( CORE::FileExists( abandonedFile ) );
            Int32 status = 200;
            for ( int i=0; i<100 && 404 != status; ++i )
            {
                MT::ThreadDelay( 100 );
                if ( !CORE::FileExists( abandonedFile ) )
                    status = 404;
            }
            ASSERT_TRUE( 404 == status );
            ASSERT_TRUE( 404 == GatewayRequest( url, ProxyToken, "GET", "/v1/uploads?id=" + abandoned, CORE::CString::Empty, answer ) );
            ASSERT_TRUE( StopGateway( gateway ) );
            ASSERT_TRUE( Contains( gateway.GetLog(), "was idle for 2 s and is removed" ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A chunk that fails is resumed from where the gateway is: a lost answer and a lost chunk between proxy and gateway; a snapshot larger than the gateway takes is refused before it is sent" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayResume" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( ForgejoToken, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            CTestCiGateway gateway;
            CORE::CProcess::TStringVector args = fixture.GatewayArgs();
            args.push_back( "--max-snapshot-mb" ); args.push_back( "1" );
            ASSERT_TRUE( gateway.Start( args, fixture.gatewayLog ) );
            CFakeGatewayFront front( gateway.GetUrl() );
            ASSERT_TRUE( front.Start() );
            front.FailChunk( 2, true );         // it arrives, its answer is lost
            front.FailChunk( 4, false );        // it is lost

            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "noise.dat" ), NoiseText( 600 * 1024, 11 ) ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "noise.dat" ) && GitCommit( fixture.repo, "noise" ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ProxyArgs( front.GetUrl() ) ) );
            fake.ScriptNextRun( EndlessRun() );
            CORE::CString text;
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            ASSERT_TRUE( 2 == front.GetFailedChunkCount() && front.GetChunkCount() >= 6 );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( fixture.NewBranch( CORE::CString::Empty ) + ":noise.dat" ).C_String() ) ==
                         GitOut( fixture.repo, "rev-parse", "HEAD:noise.dat" ) );

            // more than the gateway takes: refused here, with the size
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "big.dat" ), NoiseText( 2000 * 1024, 13 ) ) );
            bool isError = false;
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 120000 ) );
            ASSERT_TRUE( isError && Contains( text, "more than the limit of the gateway of 1.0 MiB" ) );
            ASSERT_TRUE( StopServer( server ) );
            ASSERT_TRUE( Contains( g_transcripts, "resuming from where the gateway is" ) );
            front.Stop();
            ASSERT_TRUE( StopGateway( gateway ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A gateway that is down or answers garbage: the instructions say so, the tools give a clear error in bounded time" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayDown" );
            ASSERT_TRUE( fixture.MakeRepos() );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            const CORE::CString downUrl = "http://127.0.0.1:" + CORE::UInt16ToString( TESTFW::CTlsTestServer::FindFreePort() );
            CTestCiServer down;
            ASSERT_TRUE( down.Start( fixture.ProxyArgs( downUrl ) ) );
            ASSERT_TRUE( HasAllTools( down.ToolNames() ) );
            ASSERT_TRUE( Contains( down.Instructions(), "The CI gateway at " + downUrl + " cannot be reached at the moment" ) );
            CORE::CString text;
            bool isError = false;
            const char* calls[][ 2 ] = { { "ci_status", "{\"run_id\":\"WIDGET:1\",\"wait_seconds\":10}" },
                                         { "ci_resolve_repo", "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}" },
                                         { "ci_report", "{\"run_id\":\"WIDGET:1\"}" } };
            for ( int i=0; i<3; ++i )
            {
                const UInt64 start = NowInMs();
                ASSERT_TRUE( down.Call( calls[ i ][ 0 ], calls[ i ][ 1 ], text, isError ) );
                ASSERT_TRUE( isError && Contains( text, "The CI gateway at " + downUrl + " cannot be reached" ) );
                ASSERT_TRUE( NowInMs() - start < 15000 );
            }
            const UInt64 start = NowInMs();
            ASSERT_TRUE( down.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 60000 ) );
            ASSERT_TRUE( isError && Contains( text, "cannot be reached" ) && NowInMs() - start < 15000 );
            ASSERT_TRUE( StopServer( down ) );

            CFakeGatewayFront garbage( CORE::CString::Empty );
            garbage.SetMode( CFakeGatewayFront::MODE_GARBAGE );
            ASSERT_TRUE( garbage.Start() );
            CTestCiServer confused;
            ASSERT_TRUE( confused.Start( fixture.ProxyArgs( garbage.GetUrl() ) ) );
            ASSERT_TRUE( Contains( confused.Instructions(), "cannot be reached at the moment (The CI gateway at " + garbage.GetUrl() + " answered with something that is not its API" ) );
            for ( int i=0; i<3; ++i )
            {
                ASSERT_TRUE( confused.Call( calls[ i ][ 0 ], calls[ i ][ 1 ], text, isError ) );
                ASSERT_TRUE( isError && Contains( text, "answered with something that is not its API (HTTP 200)" ) );
            }
            ASSERT_TRUE( confused.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 60000 ) );
            ASSERT_TRUE( isError && Contains( text, "not its API" ) );
            ASSERT_TRUE( StopServer( confused ) );
            garbage.Stop();
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Waiting statuses are bounded and hold no thread: beyond the limit 503 at once, other requests are served meanwhile, the waits end with their bound" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CGatewayFixture fixture( "gucefMcpCi_TestApp_GatewayPolls" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( ForgejoToken, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            CORE::CProcess::TStringVector args = fixture.GatewayArgs();
            args.push_back( "--max-pending-polls" ); args.push_back( "2" );
            args.push_back( "--gateway-threads" ); args.push_back( "1" );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( args, fixture.gatewayLog ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ProxyArgs( gateway.GetUrl() ) ) );
            fake.ScriptNextRun( EndlessRun() );
            CORE::CString text;
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );

            // two wait, the third is refused at once
            const CORE::CString pollPath = "/v1/runs?id=" + runId + "&wait=4";
            const UInt64 start = NowInMs();
            // each on a pool of its own, so that they run at the same time
            WEB::CHTTPClient firstClient;
            WEB::CHTTPClient secondClient;
            firstClient.SetThreadPoolName( "gucefMcpCi_TestApp.poll1" );
            secondClient.SetThreadPoolName( "gucefMcpCi_TestApp.poll2" );
            CORE::CFutureResult first = GatewayRequestAsync( firstClient, gateway.GetUrl(), ProxyToken, pollPath, 30000 );
            CORE::CFutureResult second = GatewayRequestAsync( secondClient, gateway.GetUrl(), ProxyToken, pollPath, 30000 );
            MT::ThreadDelay( 1000 );
            CORE::CString answer;
            ASSERT_TRUE( 503 == GatewayRequest( gateway.GetUrl(), ProxyToken, "GET", pollPath, CORE::CString::Empty, answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "polls_full" );

            // one thread, two waiting statuses, and still other calls are served
            const UInt64 askedAt = NowInMs();
            ASSERT_TRUE( 200 == GatewayRequest( gateway.GetUrl(), ProxyToken, "GET", "/v1/ci", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( NowInMs() - askedAt < 2500 );
            ASSERT_TRUE( 200 == GatewayResult( first, 30000 ) && 200 == GatewayResult( second, 30000 ) );
            const UInt64 waited = NowInMs() - start;
            ASSERT_TRUE( waited >= 3500 && waited < 15000 );

            // through the proxy the bounded wait of ci_status
            bool isError = true;
            const UInt64 statusStart = NowInMs();
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + ",\"wait_seconds\":2}", text, isError, 60000 ) );
            const UInt64 statusWaited = NowInMs() - statusStart;
            ASSERT_TRUE( !isError && Contains( text, ": running" ) && Contains( text, "Not finished" ) );
            ASSERT_TRUE( statusWaited >= 1800 && statusWaited < 15000 );
            ASSERT_TRUE( StopServer( server ) );
            ASSERT_TRUE( StopGateway( gateway ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: No token of the gateway tests appears in any answer, notification or log of a proxy or a gateway" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || g_transcripts.IsNULLOrEmpty(), "no server of the gateway tests ran" );
            const char* secrets[] = { ForgejoToken, ProxyToken, AdminToken, "8d2c4e6a0b1f3d5c7e9a", "3b5d7f9a1c2e4f6b8d0a", "6e8a0c2d4f1b3d5f7a9c" };
            for ( int i=0; i<6; ++i )
                ASSERT_FALSE( Contains( g_transcripts, secrets[ i ] ) );
        }
        catch ( const TESTFW::STestSkipRequest& )
        {
            throw;      // a skip, handled by the test case
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
