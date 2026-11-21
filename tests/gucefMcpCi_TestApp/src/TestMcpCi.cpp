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

#include <time.h>

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

#include "TestMcpCiSupport.h"
#include "TestCiServer.h"
#include "TestFakeForgejo.h"
#include "TestMcpCi.h"

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

/** The API token of the fake Forgejo: it must appear in no output of the server */
const char* Token = "tok-5f3a9c1e7b2d4f6a8c0e-NEVER-SHOWN";
const char* CiId = "WIDGET";
const char* Remote = "https://example.invalid/acme/widget.git";

/** What every server of the tests answered and logged, for the check that the token is never in it */
CORE::CString g_transcripts;

/*-------------------------------------------------------------------------*/

/** A scratch directory with a working repository, a bare scratch repository, the registry and the token file */
class CCiFixture
{
    public:

    explicit CCiFixture( const char* name )
        : m_scratch( name )
        , dir( m_scratch.GetPath() )
        , repo( CORE::CombinePath( dir, "work" ) )
        , bare( CORE::CombinePath( dir, "scratch.git" ) )
        , config( CORE::CombinePath( dir, "ci.json" ) )
        , tokenFile( CORE::CombinePath( dir, "forgejo.token" ) )
        , logDir( CORE::CombinePath( dir, "logs" ) )
    {
    }

    /** The working repository with one commit and the remote of the CI, and the bare repository */
    bool MakeRepos( void )
    {
        CORE::CreateDirs( repo );
        CORE::CreateDirs( bare );
        if ( 0 != Git( bare, "init", "-q", "--bare" ) || 0 != Git( repo, "init", "-q" ) )
            return false;
        Git( repo, "checkout", "-q", "-b", "main" );
        if ( !WriteFile( CORE::CombinePath( repo, "a.txt" ), "alpha\n" ) || !WriteFile( CORE::CombinePath( repo, "b.txt" ), "bravo\n" ) ||
             !WriteFile( CORE::CombinePath( repo, "c.txt" ), "charlie\n" ) || !WriteFile( CORE::CombinePath( repo, ".gitignore" ), "*.o\nbuild/\n" ) )
            return false;
        return 0 == Git( repo, "add", "-A" ) && GitCommit( repo, "first" ) && 0 == Git( repo, "remote", "add", "origin", Remote );
    }

    /** The registry for the fake Forgejo; the scratch repository is the bare one, as a path */
    bool WriteConfig( const CORE::CString& forgejoUrl, const CORE::CString& scratchUrl = CORE::CString::Empty, const CORE::CString& extra = CORE::CString::Empty )
    {
        const CORE::CString json = "{\"ci\":[{\"id\":\"WIDGET\",\"description\":\"the widget test project\",\"backend\":\"forgejo\",\"url\":" + Json( forgejoUrl ) +
                                   ",\"owner\":\"acme\",\"repo\":\"widget\",\"scratchOwner\":\"ci\",\"scratchRepo\":\"widget\",\"scratchUrl\":" +
                                   Json( scratchUrl.IsNULLOrEmpty() ? bare : scratchUrl ) + ",\"workflow\":\"gucef-ci.yml\",\"retentionDays\":0," +
                                   "\"remotes\":[\"https://example.invalid/acme/widget.git\",\"git@example.invalid:acme/widget.git\"]" + extra + "}]}";
        return WriteFile( config, json ) && WriteOwnerOnlyFile( tokenFile, CORE::CString( Token ) + "\n" );
    }

    CORE::CProcess::TStringVector ServerArgs( const char* extraName = GUCEF_NULL, const char* extraValue = GUCEF_NULL ) const
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "--mode" ); args.push_back( "direct" );
        args.push_back( "--ci-config" ); args.push_back( config );
        args.push_back( "--forgejo-token-file" ); args.push_back( tokenFile );
        args.push_back( "--lablink-ci-dir" ); args.push_back( "none" );
        args.push_back( "--log-dir" ); args.push_back( logDir );
        args.push_back( "--machine-name" ); args.push_back( "TestBox" );
        args.push_back( "--poll-interval-ms" ); args.push_back( "200" );
        args.push_back( "--max-wait-seconds" ); args.push_back( "30" );
        if ( GUCEF_NULL != extraName )
        {
            args.push_back( extraName );
            args.push_back( extraValue );
        }
        return args;
    }

    /** The scratch branches of the bare repository, one per line */
    CORE::CString Branches( void ) const
    {
        return GitOut( bare, "for-each-ref", "--format=%(refname:short)", "refs/heads" );
    }

    private:

    TESTFW::CScopedTestDir m_scratch;

    public:

    const CORE::CString dir;
    const CORE::CString repo;
    const CORE::CString bare;
    const CORE::CString config;
    const CORE::CString tokenFile;
    const CORE::CString logDir;
};

/*-------------------------------------------------------------------------*/

/** Stops a server and keeps what it said */
bool
StopServer( CTestCiServer& server )
{
    const bool stopped = server.Stop();
    g_transcripts += server.GetTranscript();
    return stopped;
}

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

/** The state of the source repository that a snapshot must leave as it is */
CORE::CString
RepoState( const CORE::CString& repo )
{
    return "status:\n" + GitOut( repo, "status", "--porcelain=v2", "--untracked-files=all" ) + "\nhead:\n" + GitOut( repo, "rev-parse", "HEAD" ) +
           "\nrefs:\n" + GitOut( repo, "for-each-ref" ) + "\nindex:\n" + GitOut( repo, "ls-files", "-s" ) + "\nstash:\n" + GitOut( repo, "stash", "list" ) +
           "\nconfig:\n" + GitOut( repo, "config", "--local", "--list" );
}

/*-------------------------------------------------------------------------*/

/** Content that does not compress: the size cap looks at what is sent */
CORE::CString
NoiseText( UInt32 size )
{
    // made in a buffer of its own: appending a character at a time to a string takes minutes for megabytes
    GUCEF::vector< char > buffer( size );
    UInt32 state = 12345;
    static const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245u + 12345u;
        buffer[ i ] = letters[ ( state >> 16 ) % 62 ];
    }
    return CORE::CString( &buffer[ 0 ], size );
}

/*-------------------------------------------------------------------------*/

/*
 *  Reports and logs as projects/CI/scripts/ci_cell.sh writes them: made by running the script with a stand-in for
 *  cmake (configure and build of the products badconf, badbuild, good and tested, whose build makes two test
 *  applications), with the python writer and, for ReportPlainWriterCell, the plain writer of a runner without python3.
 *  Only the path of the checkout was replaced by /work/GUCEF. The logs are the uncompressed <product>.log.gz.
 */
const char* ReportBuildCell =
    "{\n"
    "  \"schema\": 1,\n"
    "  \"cell\": \"gcc-linux-x64-cxx17-Release\",\n"
    "  \"platform\": \"linux64\",\n"
    "  \"compiler\": \"gcc\",\n"
    "  \"cppStandard\": \"17\",\n"
    "  \"buildType\": \"Release\",\n"
    "  \"mode\": \"build\",\n"
    "  \"runId\": \"4711\",\n"
    "  \"commit\": \"0123456789abcdef0123456789abcdef01234567\",\n"
    "  \"ref\": \"refs/heads/scratch/testbox-x\",\n"
    "  \"status\": \"failed\",\n"
    "  \"started\": \"2026-11-23T02:42:09Z\",\n"
    "  \"seconds\": 0,\n"
    "  \"products\": [\n"
    "    {\n"
    "      \"name\": \"badconf\",\n"
    "      \"status\": \"failed\",\n"
    "      \"phase\": \"configure\",\n"
    "      \"exitCode\": 1,\n"
    "      \"seconds\": 0,\n"
    "      \"logFile\": \"badconf.log.gz\",\n"
    "      \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\nCMake Error at CMakeLists.txt:12 (add_executable):\\n  Cannot find source file:\\n\\n    src/missing.cpp\\n\\n-- Configuring incomplete, errors occurred!\\n\"\n"
    "    },\n"
    "    {\n"
    "      \"name\": \"badbuild\",\n"
    "      \"status\": \"failed\",\n"
    "      \"phase\": \"build\",\n"
    "      \"exitCode\": 1,\n"
    "      \"seconds\": 0,\n"
    "      \"logFile\": \"badbuild.log.gz\",\n"
    "      \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\n-- Configuring done (0.4s)\\n-- Generating done (0.1s)\\n-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx17-Release/badbuild\\n[1/3] Building CXX object src/CMakeFiles/badbuild.dir/a.cpp.o\\nFAILED: src/CMakeFiles/badbuild.dir/a.cpp.o\\n/usr/bin/g++ -std=c++17 -c /work/GUCEF/src/a.cpp -o src/CMakeFiles/badbuild.dir/a.cpp.o\\n/work/GUCEF/src/a.cpp: In function 'int main()':\\n/work/GUCEF/src/a.cpp:10:5: error: 'foo' was not declared in this scope\\n/work/GUCEF/src/a.cpp:10:5: error: 'foo' was not declared in this scope\\n/work/GUCEF/src/b.cpp:3:1: warning: unused variable 'x' [-Wunused-variable]\\nninja: build stopped: subcommand failed.\\n\"\n"
    "    },\n"
    "    {\n"
    "      \"name\": \"good\",\n"
    "      \"status\": \"ok\",\n"
    "      \"phase\": \"build\",\n"
    "      \"exitCode\": 0,\n"
    "      \"seconds\": 0,\n"
    "      \"logFile\": \"good.log.gz\",\n"
    "      \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\n-- Configuring done (0.4s)\\n-- Generating done (0.1s)\\n-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx17-Release/good\\n[1/3] Building CXX object src/CMakeFiles/good.dir/a.cpp.o\\n[3/3] Linking CXX executable bin/good\\n\"\n"
    "    }\n"
    "  ]\n"
    "}\n";

const char* LogBadConf =
    "-- The C compiler identification is GNU 15.2.0\n"
    "-- The CXX compiler identification is GNU 15.2.0\n"
    "CMake Error at CMakeLists.txt:12 (add_executable):\n"
    "  Cannot find source file:\n"
    "\n"
    "    src/missing.cpp\n"
    "\n"
    "-- Configuring incomplete, errors occurred!\n";

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

const char* LogGood =
    "-- The C compiler identification is GNU 15.2.0\n"
    "-- The CXX compiler identification is GNU 15.2.0\n"
    "-- Configuring done (0.4s)\n"
    "-- Generating done (0.1s)\n"
    "-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx17-Release/good\n"
    "[1/3] Building CXX object src/CMakeFiles/good.dir/a.cpp.o\n"
    "[3/3] Linking CXX executable bin/good\n";

const char* ReportTestCell =
    "{\n"
    "  \"schema\": 1,\n"
    "  \"cell\": \"clang-linux-x64-cxx20-Release\",\n"
    "  \"platform\": \"linux64\",\n"
    "  \"compiler\": \"clang\",\n"
    "  \"cppStandard\": \"20\",\n"
    "  \"buildType\": \"Release\",\n"
    "  \"mode\": \"test\",\n"
    "  \"runId\": \"4711\",\n"
    "  \"commit\": \"0123456789abcdef0123456789abcdef01234567\",\n"
    "  \"ref\": \"refs/heads/scratch/testbox-x\",\n"
    "  \"status\": \"failed\",\n"
    "  \"started\": \"2026-11-23T02:42:09Z\",\n"
    "  \"seconds\": 0,\n"
    "  \"products\": [\n"
    "    {\n"
    "      \"name\": \"tested\",\n"
    "      \"status\": \"failed\",\n"
    "      \"phase\": \"test\",\n"
    "      \"exitCode\": 1,\n"
    "      \"seconds\": 0,\n"
    "      \"logFile\": \"tested.log.gz\",\n"
    "      \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\n-- Configuring done (0.4s)\\n-- Generating done (0.1s)\\n-- Build files have been written to: /work/GUCEF/common/bin/clang-linux-x64-cxx20-Release/tested\\n[1/3] Building CXX object src/CMakeFiles/tested.dir/a.cpp.o\\n[3/3] Linking CXX executable bin/tested\\n=== test other_TestApp\\n2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test END: Test: gamma - PASSED\\n=== test other_TestApp: ok (exit code 0, 0 s)\\n=== test widget_TestApp\\n2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test BEGIN: Test: alpha values\\n2026-11-22T20:00:00.000Z|1|ERROR|NORMAL|    ASSERTION FAILED: value == 3 at /work/GUCEF/tests/widget_TestApp/src/TestAlpha.cpp:88\\n2026-11-22T20:00:00.000Z|1|WARNING|NORMAL|  Test END: Test: alpha values - FAILED (1 failures)\\n2026-11-22T20:00:00.001Z|1|STANDARD|NORMAL|  Test END: Test: beta values - PASSED\\n=== test widget_TestApp: failed (exit code 1, 0 s)\\n\",\n"
    "      \"tests\": [\n"
    "        {\n"
    "          \"name\": \"other_TestApp\",\n"
    "          \"status\": \"ok\",\n"
    "          \"exitCode\": 0,\n"
    "          \"seconds\": 0,\n"
    "          \"tests\": 1,\n"
    "          \"failures\": 0,\n"
    "          \"skipped\": 0,\n"
    "          \"failedCases\": [],\n"
    "          \"resultsFile\": \"other_TestApp_Results.xml\"\n"
    "        },\n"
    "        {\n"
    "          \"name\": \"widget_TestApp\",\n"
    "          \"status\": \"failed\",\n"
    "          \"exitCode\": 1,\n"
    "          \"seconds\": 0,\n"
    "          \"tests\": 2,\n"
    "          \"failures\": 1,\n"
    "          \"skipped\": 0,\n"
    "          \"failedCases\": [\n"
    "            {\n"
    "              \"suite\": \"Widget\",\n"
    "              \"name\": \"Test: alpha values\",\n"
    "              \"message\": \"ASSERTION FAILED: value == 3 at tests/widget_TestApp/src/TestAlpha.cpp:88\"\n"
    "            }\n"
    "          ],\n"
    "          \"resultsFile\": \"widget_TestApp_Results.xml\",\n"
    "          \"logTail\": \"2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test BEGIN: Test: alpha values\\n2026-11-22T20:00:00.000Z|1|ERROR|NORMAL|    ASSERTION FAILED: value == 3 at /work/GUCEF/tests/widget_TestApp/src/TestAlpha.cpp:88\\n2026-11-22T20:00:00.000Z|1|WARNING|NORMAL|  Test END: Test: alpha values - FAILED (1 failures)\\n2026-11-22T20:00:00.001Z|1|STANDARD|NORMAL|  Test END: Test: beta values - PASSED\\n\"\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  ]\n"
    "}\n";

const char* LogTested =
    "-- The C compiler identification is GNU 15.2.0\n"
    "-- The CXX compiler identification is GNU 15.2.0\n"
    "-- Configuring done (0.4s)\n"
    "-- Generating done (0.1s)\n"
    "-- Build files have been written to: /work/GUCEF/common/bin/clang-linux-x64-cxx20-Release/tested\n"
    "[1/3] Building CXX object src/CMakeFiles/tested.dir/a.cpp.o\n"
    "[3/3] Linking CXX executable bin/tested\n"
    "=== test other_TestApp\n"
    "2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test END: Test: gamma - PASSED\n"
    "=== test other_TestApp: ok (exit code 0, 0 s)\n"
    "=== test widget_TestApp\n"
    "2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test BEGIN: Test: alpha values\n"
    "2026-11-22T20:00:00.000Z|1|ERROR|NORMAL|    ASSERTION FAILED: value == 3 at /work/GUCEF/tests/widget_TestApp/src/TestAlpha.cpp:88\n"
    "2026-11-22T20:00:00.000Z|1|WARNING|NORMAL|  Test END: Test: alpha values - FAILED (1 failures)\n"
    "2026-11-22T20:00:00.001Z|1|STANDARD|NORMAL|  Test END: Test: beta values - PASSED\n"
    "=== test widget_TestApp: failed (exit code 1, 0 s)\n";

const char* ReportSucceededCell =
    "{\n"
    "  \"schema\": 1,\n"
    "  \"cell\": \"gcc-linux-x64-cxx98-Release\",\n"
    "  \"platform\": \"linux64\",\n"
    "  \"compiler\": \"gcc\",\n"
    "  \"cppStandard\": \"98\",\n"
    "  \"buildType\": \"Release\",\n"
    "  \"mode\": \"build\",\n"
    "  \"runId\": \"4711\",\n"
    "  \"commit\": \"0123456789abcdef0123456789abcdef01234567\",\n"
    "  \"ref\": \"refs/heads/scratch/testbox-x\",\n"
    "  \"status\": \"ok\",\n"
    "  \"started\": \"2026-11-23T02:42:09Z\",\n"
    "  \"seconds\": 0,\n"
    "  \"products\": [\n"
    "    {\n"
    "      \"name\": \"good\",\n"
    "      \"status\": \"ok\",\n"
    "      \"phase\": \"build\",\n"
    "      \"exitCode\": 0,\n"
    "      \"seconds\": 0,\n"
    "      \"logFile\": \"good.log.gz\",\n"
    "      \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\n-- Configuring done (0.4s)\\n-- Generating done (0.1s)\\n-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx98-Release/good\\n[1/3] Building CXX object src/CMakeFiles/good.dir/a.cpp.o\\n[3/3] Linking CXX executable bin/good\\n\"\n"
    "    }\n"
    "  ]\n"
    "}\n";

const char* ReportPlainWriterCell =
    "{\n"
    "  \"schema\": 1,\n"
    "  \"cell\": \"gcc-linux-x64-cxx11-Release\",\n"
    "  \"platform\": \"linux64\",\n"
    "  \"compiler\": \"gcc\",\n"
    "  \"cppStandard\": \"11\",\n"
    "  \"buildType\": \"Release\",\n"
    "  \"mode\": \"test\",\n"
    "  \"runId\": \"4711\",\n"
    "  \"commit\": \"0123456789abcdef0123456789abcdef01234567\",\n"
    "  \"ref\": \"refs/heads/scratch/testbox-x\",\n"
    "  \"status\": \"failed\",\n"
    "  \"started\": \"2026-11-23T02:42:24Z\",\n"
    "  \"seconds\": 0,\n"
    "  \"products\": [\n"
    "    { \"name\": \"tested\", \"status\": \"failed\", \"phase\": \"test\", \"exitCode\": 1, \"seconds\": 0, \"logFile\": \"tested.log.gz\", \"logTail\": \"-- The C compiler identification is GNU 15.2.0\\n-- The CXX compiler identification is GNU 15.2.0\\n-- Configuring done (0.4s)\\n-- Generating done (0.1s)\\n-- Build files have been written to: /work/GUCEF/common/bin/gcc-linux-x64-cxx11-Release/tested\\n[1/3] Building CXX object src/CMakeFiles/tested.dir/a.cpp.o\\n[3/3] Linking CXX executable bin/tested\\n=== test other_TestApp\\n2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test END: Test: gamma - PASSED\\n=== test other_TestApp: ok (exit code 0, 0 s)\\n=== test widget_TestApp\\n2026-11-22T20:00:00.000Z|1|STANDARD|NORMAL|  Test BEGIN: Test: alpha values\\n2026-11-22T20:00:00.000Z|1|ERROR|NORMAL|    ASSERTION FAILED: value == 3 at /work/GUCEF/tests/widget_TestApp/src/TestAlpha.cpp:88\\n2026-11-22T20:00:00.000Z|1|WARNING|NORMAL|  Test END: Test: alpha values - FAILED (1 failures)\\n2026-11-22T20:00:00.001Z|1|STANDARD|NORMAL|  Test END: Test: beta values - PASSED\\n=== test widget_TestApp: failed (exit code 1, 0 s)\", \"tests\": [{ \"name\": \"other_TestApp\", \"status\": \"ok\", \"exitCode\": 0, \"seconds\": 0, \"failedCases\": [] }, { \"name\": \"widget_TestApp\", \"status\": \"failed\", \"exitCode\": 1, \"seconds\": 0, \"failedCases\": [] }] }\n"
    "  ]\n"
    "}\n";

/*-------------------------------------------------------------------------*/

/** The stages of a run that ends with one of its two cells failed */
CFakeForgejo::TStageVector
FailingRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "waiting", 1 ).AddJob( "plan", "waiting" ) );
    stages.push_back( CFakeForgejo::CStage( "running", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-cxx17", "running" ).AddJob( "clang-cxx17", "waiting" ) );
    stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-cxx17", "failure" ).AddJob( "clang-cxx17", "success" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

/** Submits the working tree and gives the run id; empty when that failed */
CORE::CString
Submit( CTestCiServer& server, const CCiFixture& fixture, const CORE::CString& extraArguments, CORE::CString& text )
{
    bool isError = true;
    if ( !server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + extraArguments + "}", text, isError, 120000 ) || isError )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi_TestApp: ci_submit failed: " << text );
        return CORE::CString::Empty;
    }
    return After( text, "Run id: " );
}

/*-------------------------------------------------------------------------*/

/** Starts a server process that must refuse to start. @return its exit code, 0 when it did not end */
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

bool
HasAllTools( const CORE::CString& names )
{
    const char* tools[] = { "ci_resolve_repo,", "ci_describe,", "ci_submit,", "ci_status,", "ci_report,", "ci_job_details,", "ci_cancel,", "ci_cleanup," };
    for ( int i=0; i<8; ++i )
        if ( !Contains( names, tools[ i ] ) )
            return false;
    return 8 == names.GetCharacterCount( ',' );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpCiTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "McpCi" );

    // the responses of the server are parsed with the JSON codec
    const bool hasServer = !FindServer().IsNULLOrEmpty() && McpTestJsonAvailable();
    const bool hasGit = GitAvailable();

    GUCEF_TESTFW_TESTCASE( "Test: The tool list: the eight tools with a registry and a token, from options or from the lablink directory; none without, the instructions say why" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Tools" );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            {
                CTestCiServer server;
                ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
                ASSERT_TRUE( HasAllTools( server.ToolNames() ) );
                const CORE::CString instructions = server.Instructions();
                ASSERT_TRUE( Contains( instructions, "ci_submit" ) && Contains( instructions, "ONCE" ) && Contains( instructions, "CI ids: WIDGET" ) );
                ASSERT_TRUE( StopServer( server ) );
            }
            {
                // auto without anything: no tools, and the instructions say what is missing
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--lablink-ci-dir" ); args.push_back( "none" );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( server.ToolNames().IsNULLOrEmpty() );
                ASSERT_TRUE( Contains( server.Instructions(), "No CI is set up for this machine" ) );
                ASSERT_TRUE( StopServer( server ) );
            }
            {
                // the files lablink provides, found in the directory given
                const CORE::CString lablinkDir = CORE::CombinePath( fixture.dir, "lablink" );
                CORE::CreateDirs( lablinkDir );
                CORE::CString registry;
                ASSERT_TRUE( ReadFile( fixture.config, registry ) );
                ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( lablinkDir, "ci.json" ), registry ) );
                ASSERT_TRUE( WriteOwnerOnlyFile( CORE::CombinePath( lablinkDir, "forgejo.token" ), Token ) );
                CTestCiServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--lablink-ci-dir" ); args.push_back( lablinkDir );
                args.push_back( "--log-dir" ); args.push_back( fixture.logDir );
                ASSERT_TRUE( server.Start( args ) );
                ASSERT_TRUE( HasAllTools( server.ToolNames() ) );
                ASSERT_TRUE( StopServer( server ) );
            }
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

    GUCEF_TESTFW_TESTCASE( "Test: Start options that cannot work are refused: gateway and proxy, direct without a token, a token file others can read, bad serving options" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Refused" );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );

            CORE::CProcess::TStringVector args = fixture.ServerArgs();
            args[ 1 ] = "gateway";
            ASSERT_TRUE( 0 != ExitCodeOf( args ) );
            args[ 1 ] = "proxy";
            ASSERT_TRUE( 0 != ExitCodeOf( args ) );

            // direct without a token
            CORE::CProcess::TStringVector noToken;
            noToken.push_back( "--mode" ); noToken.push_back( "direct" );
            noToken.push_back( "--ci-config" ); noToken.push_back( fixture.config );
            noToken.push_back( "--lablink-ci-dir" ); noToken.push_back( "none" );
            ASSERT_TRUE( 0 != ExitCodeOf( noToken ) );

            // a token file that others may read is refused, the token is never used
            const CORE::CString openToken = CORE::CombinePath( fixture.dir, "open.token" );
            ASSERT_TRUE( WriteFile( openToken, Token ) );
            ASSERT_TRUE( 0 != ExitCodeOf( fixture.ServerArgs( "--forgejo-token-file", openToken.C_String() ) ) );

            const char* sets[][ 4 ] = { { "--listen", "127.0.0.1:1", GUCEF_NULL, GUCEF_NULL },
                                        { "--listen-tls", "127.0.0.1:1", "--token-file", "/nonexistent/token" },
                                        { "--listen", "nowhere", "--token-file", "/nonexistent/token" },
                                        { "--no-such-option", GUCEF_NULL, GUCEF_NULL, GUCEF_NULL } };
            for ( int i=0; i<4; ++i )
            {
                CORE::CProcess::TStringVector serving = fixture.ServerArgs();
                for ( int a=0; a<4 && GUCEF_NULL != sets[ i ][ a ]; ++a )
                    serving.push_back( sets[ i ][ a ] );
                ASSERT_TRUE( 0 != ExitCodeOf( serving ) );
            }
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_resolve_repo: the remotes of a repository or given ones, in https, ssh and scp form; no CI for others, nothing is guessed" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Resolve" );
            ASSERT_TRUE( fixture.MakeRepos() );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;

            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "CI id: WIDGET (the widget test project)" ) && Contains( text, "Matched remote: " + CORE::CString( Remote ) ) );

            // the forms of one remote: case of the host, a port, user, no .git, a trailing /
            const char* forms[] = { "ssh://git@EXAMPLE.invalid:2222/acme/widget", "git@Example.Invalid:acme/widget.git", "https://user@example.invalid/acme/widget/",
                                    "http://example.invalid:8080/acme/widget.git" };
            for ( int i=0; i<4; ++i )
            {
                ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[" + Json( forms[ i ] ) + "]}", text, isError ) );
                ASSERT_FALSE( isError );
                ASSERT_TRUE( Contains( text, "CI id: WIDGET" ) );
            }

            // the path is compared as it is, and other repositories have no CI
            const char* others[] = { "https://example.invalid/acme/other.git", "https://example.invalid/ACME/widget.git", "https://elsewhere.invalid/acme/widget.git" };
            for ( int i=0; i<3; ++i )
            {
                ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"remotes\":[" + Json( others[ i ] ) + "]}", text, isError ) );
                ASSERT_TRUE( isError );
                ASSERT_TRUE( Contains( text, "No CI is set up for this repository" ) && Contains( text, others[ i ] ) );
            }

            // a repository without remotes, and a call without anything
            const CORE::CString lonely = CORE::CombinePath( fixture.dir, "lonely" );
            CORE::CreateDirs( lonely );
            ASSERT_TRUE( 0 == Git( lonely, "init", "-q" ) );
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{\"repo_dir\":" + Json( lonely ) + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "has no remotes" ) );
            ASSERT_TRUE( server.Call( "ci_resolve_repo", "{}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Give repo_dir or remotes" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_describe: the options from the CI files of the repository; without a repository the registry; unknown CI ids" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Describe" );
            ASSERT_TRUE( fixture.MakeRepos() );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/matrix.json" ),
                "{\"cppStandards\":[\"98\",\"17\"],\"buildTypes\":[\"Release\"],\"toolchains\":["
                "{\"id\":\"gcc-linux-x64\",\"enabled\":true},{\"id\":\"clang-linux-x64\",\"enabled\":true},{\"id\":\"msvc-win64\",\"enabled\":false}],"
                "\"cells\":[{\"id\":\"a\"},{\"id\":\"b\"},{\"id\":\"c\"},{\"id\":\"d\"}]}" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/products.json" ),
                "{\"products\":[{\"name\":\"platform\",\"platform\":\"linux64\"},{\"name\":\"platform\",\"platform\":\"win64\"},{\"name\":\"tools\"}]}" ) );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Toolchains: gcc-linux-x64, clang-linux-x64, msvc-win64 (no runner, not built)" ) );
            ASSERT_TRUE( Contains( text, "C++ standards: 98, 17" ) && Contains( text, "Products: platform, tools" ) && Contains( text, "= 4 jobs" ) );
            ASSERT_TRUE( Contains( text, "Modes: build, test" ) );

            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"WIDGET\"}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "pass repo_dir" ) );

            ASSERT_TRUE( server.Call( "ci_describe", "{\"ci_id\":\"NOPE\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown CI id \"NOPE\"" ) && Contains( text, "Known CI ids: WIDGET" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: The snapshot holds unpushed commits, modified, staged, deleted and untracked files and no ignored ones; dry_run sends nothing; the repository is left as it was" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Snapshot" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );

            // a second commit that is not pushed anywhere, then work that is not committed
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "d.txt" ), "delta\n" ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "d.txt" ) && GitCommit( fixture.repo, "second" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha changed\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "b.txt" ), "bravo staged\n" ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "b.txt" ) );
            ASSERT_TRUE( CORE::DeleteFile( CORE::CombinePath( fixture.repo, "c.txt" ) ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "new.txt" ), "new\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "sub/deep.txt" ), "deep\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "x.o" ), "object\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "build/out.txt" ), "built\n" ) );
            const CORE::CString head = GitOut( fixture.repo, "rev-parse", "HEAD" );
            const CORE::CString before = RepoState( fixture.repo );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + ",\"dry_run\":true}", text, isError, 120000 ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Dry run: nothing was sent" ) );
            ASSERT_TRUE( Contains( text, "2 unpublished commit(s)" ) );
            ASSERT_TRUE( Contains( text, "2 modified, 2 added (2 untracked), 1 deleted" ) );
            ASSERT_TRUE( Contains( text, "M a.txt" ) && Contains( text, "M b.txt" ) && Contains( text, "D c.txt" ) && Contains( text, "A new.txt" ) && Contains( text, "A sub/deep.txt" ) );
            ASSERT_FALSE( Contains( text, "x.o" ) || Contains( text, "build/out.txt" ) );
            ASSERT_TRUE( 0 == fake.GetDispatchCount() );
            ASSERT_TRUE( fixture.Branches().IsNULLOrEmpty() );

            CFakeForgejo::TStageVector stages;
            stages.push_back( CFakeForgejo::CStage( "waiting", 1000 ) );
            fake.ScriptNextRun( stages );
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_TRUE( runId == "WIDGET:" + CORE::Int64ToString( fake.GetLastRunId() ) );
            ASSERT_TRUE( Contains( text, "Next: ci_status with run_id \"" + runId + "\"" ) );

            // the branch in the scratch repository holds the working tree as it is, on top of HEAD
            const CORE::CString branch = fixture.Branches();
            ASSERT_TRUE( branch.StartsWith( "scratch/testbox-" ) && !Contains( branch, "\n" ) );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":a.txt" ).C_String() ) == "alpha changed" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":b.txt" ).C_String() ) == "bravo staged" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":d.txt" ).C_String() ) == "delta" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":new.txt" ).C_String() ) == "new" );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":sub/deep.txt" ).C_String() ) == "deep" );
            ASSERT_TRUE( 0 != Git( fixture.bare, "cat-file", "-e", ( branch + ":c.txt" ).C_String() ) );
            ASSERT_TRUE( 0 != Git( fixture.bare, "cat-file", "-e", ( branch + ":x.o" ).C_String() ) );
            ASSERT_TRUE( 0 != Git( fixture.bare, "cat-file", "-e", ( branch + ":build/out.txt" ).C_String() ) );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( branch + "^" ).C_String() ) == head );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-list", "--count", branch.C_String() ) == "3" );

            // the dispatch named the branch and built everything
            const CORE::CString body = fake.GetLastDispatchBody();
            ASSERT_TRUE( Contains( body, branch ) && Contains( body, "\"force\"" ) && Contains( body, "return_run_info" ) );
            ASSERT_FALSE( Contains( body, "toolchains" ) || Contains( body, "\"mode\"" ) );

            // nothing of the repository changed: working tree, index, refs, configuration
            ASSERT_TRUE( RepoState( fixture.repo ) == before );
            ASSERT_TRUE( 0 == fake.GetUnauthorizedCount() );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: Snapshot guards: untracked files that look like secrets are refused unless tracked; the size cap; HEAD only; filters and mode reach the CI" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Guards" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );

            // tracked on purpose: allowed, also when it changes
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "config/secrets.yaml" ), "public: yes\n" ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "add", "-A" ) && GitCommit( fixture.repo, "tracked" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "config/secrets.yaml" ), "public: still\n" ) );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( "--max-snapshot-mb", "1" ) ) );
            CORE::CString text;
            bool isError = false;
            const char* secrets[] = { "deploy.env", ".env", "keys/server.pem", "id_rsa", ".netrc", "api.token", "secrets.json" };
            for ( int i=0; i<7; ++i )
            {
                const CORE::CString path = CORE::CombinePath( fixture.repo, secrets[ i ] );
                ASSERT_TRUE( WriteFile( path, "secret\n" ) );
                ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 120000 ) );
                ASSERT_TRUE( isError && Contains( text, "look like secrets" ) && Contains( text, secrets[ i ] ) );
                ASSERT_TRUE( CORE::DeleteFile( path ) );
            }
            ASSERT_TRUE( 0 == fake.GetDispatchCount() && fixture.Branches().IsNULLOrEmpty() );

            // more than the cap
            const CORE::CString big = CORE::CombinePath( fixture.repo, "big.dat" );
            ASSERT_TRUE( WriteFile( big, NoiseText( 1536 * 1024 ) ) );
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 120000 ) );
            ASSERT_TRUE( isError && Contains( text, "more than the limit of 1.0 MiB" ) );
            ASSERT_TRUE( 0 == fake.GetDispatchCount() );

            // HEAD only leaves the working tree out, the big file too; the filters and the mode go to the CI
            const CORE::CString runId = Submit( server, fixture, ",\"include_uncommitted\":false,\"toolchains\":[\"gcc-linux-x64\"],\"cpp_standards\":[\"98\",\"17\"],"
                                                                 "\"products\":[\"platform\"],\"mode\":\"test\"", text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            ASSERT_TRUE( Contains( text, "uncommitted work left out" ) );
            const CORE::CString branch = fixture.Branches();
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", branch.C_String() ) == GitOut( fixture.repo, "rev-parse", "HEAD" ) );
            ASSERT_TRUE( GitOut( fixture.bare, "show", ( branch + ":config/secrets.yaml" ).C_String() ) == "public: yes" );
            const CORE::CString body = fake.GetLastDispatchBody();
            ASSERT_TRUE( Contains( body, "gcc-linux-x64" ) && Contains( body, "98,17" ) && Contains( body, "platform" ) && Contains( body, "\"test\"" ) );
            ASSERT_TRUE( CORE::DeleteFile( big ) );

            // the changed tracked secret goes with the working tree
            ASSERT_FALSE( Submit( server, fixture, CORE::CString::Empty, text ).IsNULLOrEmpty() );
            ASSERT_TRUE( Contains( text, "M config/secrets.yaml" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_status follows a run from queued to running to failed; wait_seconds ends with the run and is bounded; a run is found without run info; unknown run ids" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Status" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;
            fake.ScriptNextRun( FailingRun() );
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );

            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Run " + runId + ": queued" ) && Contains( text, "Not finished" ) );
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_TRUE( Contains( text, ": running" ) && Contains( text, "Jobs: 3 (1 succeeded, 0 failed, 1 running, 1 queued)" ) && Contains( text, "Running: gcc-cxx17" ) );
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + ",\"wait_seconds\":20}", text, isError ) );
            ASSERT_TRUE( ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution() < 10000 );
            ASSERT_TRUE( Contains( text, ": failed" ) && Contains( text, "Failed: gcc-cxx17" ) && Contains( text, "Next: ci_report" ) );

            // a run that does not finish: the wait ends at its bound
            CFakeForgejo::TStageVector stuck;
            stuck.push_back( CFakeForgejo::CStage( "running", 1000000 ).AddJob( "plan", "running" ) );
            fake.ScriptNextRun( stuck );
            fake.SetReturnRunInfo( false );         // found in the list of runs instead
            const CORE::CString stuckRun = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_TRUE( stuckRun == "WIDGET:" + CORE::Int64ToString( fake.GetLastRunId() ) );
            ASSERT_TRUE( fake.GetRunListCount() > 0 );
            const UInt64 waitStart = MT::PrecisionTickCount();
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( stuckRun ) + ",\"wait_seconds\":2}", text, isError ) );
            const UInt64 waited = ( MT::PrecisionTickCount() - waitStart ) * 1000 / MT::PrecisionTimerResolution();
            ASSERT_TRUE( waited >= 1800 && waited < 15000 );
            ASSERT_TRUE( Contains( text, ": running" ) && Contains( text, "Not finished" ) );

            // more than the maximum wait is refused by the schema
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( stuckRun ) + ",\"wait_seconds\":31}", text, isError ) );
            ASSERT_TRUE( isError );

            const char* unknown[] = { "WIDGET:999", "WIDGET:abc", "nonsense", "OTHER:1" };
            for ( int i=0; i<4; ++i )
            {
                ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( unknown[ i ] ) + "}", text, isError ) );
                ASSERT_TRUE( isError && Contains( text, "Unknown run id" ) );
            }
            ASSERT_TRUE( 0 == fake.GetUnauthorizedCount() );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_report reads the reports of ci_cell.sh: failed configure and build with their first errors, failed test cases, the plain writer, succeeded jobs, jobs without a report" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Report" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );

            CFakeForgejo::TStageVector stages;
            stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release (map[arch:x64 id:gcc-linux-x64-cxx17-Release toolchain:gcc-linux-x64])", "failure" )
                                                                  .AddJob( "clang-linux-x64-cxx20-Release", "failure" ).AddJob( "gcc-linux-x64-cxx98-Release", "success" )
                                                                  .AddJob( "gcc-linux-x64-cxx11-Release", "failure" ).AddJob( "clang-linux-x64-cxx98-Release", "failure" ) );
            fake.ScriptNextRun( stages );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );

            // the reports under the default owner of the report packages, ci-bot
            const CORE::CString run = CORE::Int64ToString( fake.GetLastRunId() );
            fake.AddPackageFile( "ci-bot", "ci-reports", run + "-gcc-linux-x64-cxx17-Release", "report.json", ReportBuildCell, CORE::CString::Empty );
            fake.AddPackageFile( "ci-bot", "ci-reports", run + "-clang-linux-x64-cxx20-Release", "report.json", ReportTestCell, CORE::CString::Empty );
            fake.AddPackageFile( "ci-bot", "ci-reports", run + "-gcc-linux-x64-cxx98-Release", "report.json", ReportSucceededCell, CORE::CString::Empty );
            fake.AddPackageFile( "ci-bot", "ci-reports", run + "-gcc-linux-x64-cxx11-Release", "report.json", ReportPlainWriterCell, CORE::CString::Empty );

            ASSERT_TRUE( server.Call( "ci_report", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, ": failed" ) && Contains( text, "Failed jobs:" ) );
            // a failed configure and a failed build, with the first errors relative to the checkout
            ASSERT_TRUE( Contains( text, "- gcc-linux-x64-cxx17-Release: (build)\n    product badconf: configure failed (exit 1)\n" ) );
            ASSERT_TRUE( Contains( text, "      CMakeLists.txt:12: Cannot find source file: src/missing.cpp" ) );
            ASSERT_TRUE( Contains( text, "    product badbuild: build failed (exit 1)\n      src/a.cpp:10:5: 'foo' was not declared in this scope\n" ) );
            ASSERT_FALSE( Contains( text, "product good" ) || Contains( text, "/work/GUCEF/src" ) );
            // test mode: the failed test case as suite: name - message, the test that succeeded left out
            ASSERT_TRUE( Contains( text, "- clang-linux-x64-cxx20-Release: (test)\n    product tested: tests failed (exit 1)\n"
                                         "      test widget_TestApp: failed (exit 1, 1 of 2 failed)\n"
                                         "        Widget: Test: alpha values - ASSERTION FAILED: value == 3 at tests/widget_TestApp/src/TestAlpha.cpp:88\n" ) );
            ASSERT_FALSE( Contains( text, "other_TestApp" ) );
            // the plain writer names no cases
            ASSERT_TRUE( Contains( text, "- gcc-linux-x64-cxx11-Release: (test)\n    product tested: tests failed (exit 1)\n      test widget_TestApp: failed (exit 1)\n"
                                         "        the report names no failed cases; its output is in the log of the product\n" ) );
            ASSERT_TRUE( Contains( text, "- clang-linux-x64-cxx98-Release: no report published" ) );
            ASSERT_TRUE( Contains( text, "Succeeded jobs (2, 1 product builds): plan, gcc-linux-x64-cxx98-Release" ) );
            ASSERT_TRUE( Contains( text, "Next: ci_job_details with run_id \"" + runId + "\" and job_id \"gcc-linux-x64-cxx17-Release\"" ) );

            ASSERT_TRUE( server.Call( "ci_report", "{\"run_id\":\"WIDGET:4242\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown run id" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_job_details on the reports of ci_cell.sh: the digest of gzip product logs with file:line, the plain log when there is no gzip one, log_tail, failed test cases from the report, the owner of the reports from ci.json" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CORE::CDynamicBuffer badBuildGz;
            CORE::CDynamicBuffer testedGz;
            GUCEF_TESTFW_SKIP_IF( !Gzip( LogBadBuild, badBuildGz ) || !Gzip( LogTested, testedGz ), "the gzip codec (codecspluginZLIB) is not next to the test application" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Details" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl(), CORE::CString::Empty, ",\"reportsOwner\":\"lab-reports\"" ) );

            CFakeForgejo::TStageVector stages;
            stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "failure" )
                                                                  .AddJob( "clang-linux-x64-cxx20-Release", "failure" ).AddJob( "gcc-linux-x64-cxx98-Release", "success" ) );
            fake.ScriptNextRun( stages );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            const CORE::CString run = CORE::Int64ToString( fake.GetLastRunId() );
            const CORE::CString build = run + "-gcc-linux-x64-cxx17-Release";
            const CORE::CString tested = run + "-clang-linux-x64-cxx20-Release";
            fake.AddPackageFile( "lab-reports", "ci-reports", build, "report.json", ReportBuildCell, CORE::CString::Empty );
            fake.AddPackageFile( "lab-reports", "ci-reports", build, "badbuild.log.gz", badBuildGz, CORE::CString::Empty );
            fake.AddPackageFile( "lab-reports", "ci-reports", build, "badconf.log", LogBadConf, CORE::CString::Empty );       // only uncompressed
            fake.AddPackageFile( "lab-reports", "ci-reports", tested, "report.json", ReportTestCell, CORE::CString::Empty );
            fake.AddPackageFile( "lab-reports", "ci-reports", tested, "tested.log.gz", testedGz, CORE::CString::Empty );
            fake.AddPackageFile( "lab-reports", "ci-reports", run + "-gcc-linux-x64-cxx98-Release", "report.json", ReportSucceededCell, CORE::CString::Empty );

            // the failed configure and build: their logs, digested
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"gcc-linux-x64-cxx17-Release\"}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Job gcc-linux-x64-cxx17-Release of run " + runId + ": failed" ) );
            ASSERT_TRUE( Contains( text, "In the report: badconf failed in configure, badbuild failed in build, good succeeded" ) );
            ASSERT_TRUE( Contains( text, "== product badconf (" ) && Contains( text, "CMakeLists.txt:12: Cannot find source file: src/missing.cpp" ) );
            ASSERT_TRUE( Contains( text, "== product badbuild (" ) && Contains( text, "Errors: 2 (1 distinct)" ) );
            ASSERT_TRUE( Contains( text, "src/a.cpp:10:5: 'foo' was not declared in this scope [x2]" ) && Contains( text, "src/b.cpp:3:1: unused variable 'x'" ) );
            ASSERT_FALSE( Contains( text, "== product good" ) );
            const CORE::CString logPath = After( text, "== product badbuild (", '\n' );
            const CORE::CString savedPath = After( logPath, "full log: ", ')' );
            ASSERT_TRUE( savedPath.StartsWith( "/" ) );
            CORE::CString saved;
            ASSERT_TRUE( ReadFile( savedPath, saved ) && saved == CORE::CString( LogBadBuild ) );

            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"gcc-linux-x64-cxx17-Release\",\"product\":\"badbuild\",\"part\":\"log_tail\",\"limit\":2,\"offset\":1}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "a.cpp:10:5: error: 'foo' was not declared in this scope\n/work/GUCEF/src/b.cpp:3:1: warning: unused variable 'x'" ) );
            ASSERT_FALSE( Contains( text, "ninja: build stopped" ) || Contains( text, "== product badconf" ) );

            // test mode: the cases and the end of the output of the test application from the report
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"clang-linux-x64-cxx20-Release\",\"part\":\"tests\"}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "test widget_TestApp: failed (exit 1, 1 of 2 failed)\n"
                                         "  Widget: Test: alpha values - ASSERTION FAILED: value == 3 at tests/widget_TestApp/src/TestAlpha.cpp:88\n" ) );
            ASSERT_TRUE( Contains( text, "  tests/widget_TestApp/src/TestAlpha.cpp:88: ASSERTION FAILED: value == 3" ) );
            ASSERT_TRUE( Contains( text, "== product tested (" ) && Contains( text, "full log: /" ) );
            ASSERT_FALSE( Contains( text, "test other_TestApp:" ) );

            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"clang-linux-x64-cxx20-Release\",\"product\":\"widget_TestApp\",\"part\":\"log_tail\",\"limit\":1}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "the end of its output from the report:\n2026-11-22T20:00:00.001Z|1|STANDARD|NORMAL|  Test END: Test: beta values - PASSED" ) );
            ASSERT_FALSE( Contains( text, "alpha values - FAILED" ) );

            // a job without failures, a job that does not exist, a run that does not exist
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"gcc-linux-x64-cxx98-Release\"}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "In the report: good succeeded" ) && Contains( text, "Nothing failed in this job" ) );
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"nope\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown job id \"nope\"" ) && Contains( text, "plan, gcc-linux-x64-cxx17-Release, clang-linux-x64-cxx20-Release" ) );
            ASSERT_TRUE( server.Call( "ci_job_details", "{\"run_id\":\"WIDGET:1\",\"job_id\":\"plan\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown run id" ) );

            // nothing was asked of the default owner: the owner of ci.json is used
            ASSERT_FALSE( Contains( fake.GetRequestLog(), "/api/packages/ci-bot/" ) );
            ASSERT_TRUE( Contains( fake.GetRequestLog(), "/api/packages/lab-reports/generic/ci-reports/" + build + "/report.json" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_cancel stops a running run, says so for a finished one, and explains a CI that cannot cancel" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Cancel" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = true;

            CFakeForgejo::TStageVector running;
            running.push_back( CFakeForgejo::CStage( "running", 1000000 ).AddJob( "plan", "success" ).AddJob( "gcc-cxx17", "running" ) );
            fake.ScriptNextRun( running );
            const CORE::CString runId = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            ASSERT_TRUE( server.Call( "ci_cancel", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Cancelling the run was asked for" ) );
            ASSERT_TRUE( server.Call( "ci_status", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_TRUE( Contains( text, ": cancelled" ) && Contains( text, "1 cancelled" ) );
            ASSERT_TRUE( server.Call( "ci_cancel", "{\"run_id\":" + Json( runId ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "already finished (cancelled)" ) );

            fake.SetCancelSupported( false );
            fake.ScriptNextRun( running );
            const CORE::CString other = Submit( server, fixture, CORE::CString::Empty, text );
            ASSERT_TRUE( server.Call( "ci_cancel", "{\"run_id\":" + Json( other ) + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "cannot cancel a run" ) );
            ASSERT_TRUE( server.Call( "ci_cancel", "{\"run_id\":\"WIDGET:777\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown run id" ) );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_cleanup removes scratch branches and reports older than the limit and keeps newer ones and other branches" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Cleanup" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );

            char now[ 32 ];
            time_t seconds = ::time( GUCEF_NULL );
            ::strftime( now, sizeof( now ), "%Y%m%dT%H%M%SZ", ::gmtime( &seconds ) );
            const CORE::CString oldBranch = "scratch/oldbox-20200101T000000Z-abcdef1234";
            const CORE::CString newBranch = "scratch/newbox-" + CORE::CString( now ) + "-abcdef1234";
            ASSERT_TRUE( 0 == Git( fixture.repo, "push", "-q", fixture.bare.C_String(), ( "HEAD:refs/heads/" + oldBranch ).C_String() ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "push", "-q", fixture.bare.C_String(), ( "HEAD:refs/heads/" + newBranch ).C_String() ) );
            ASSERT_TRUE( 0 == Git( fixture.repo, "push", "-q", fixture.bare.C_String(), "HEAD:refs/heads/main" ) );
            fake.AddPackageFile( "sweeper", "ci-reports", "11-gcc", "report.json", "{}", "2020-01-02T00:00:00Z" );
            fake.AddPackageFile( "sweeper", "ci-reports", "12-gcc", "report.json", "{}", CORE::CString::Empty );
            fake.AddPackageFile( "ci-bot", "ci-reports", "13-gcc", "report.json", "{}", "2020-01-02T00:00:00Z" );     // another owner: kept

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( "--reports-owner", "sweeper" ) ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "ci_cleanup", "{\"ci_id\":\"WIDGET\",\"older_than_days\":14}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Removed 1 scratch branch(es) and 1 report(s) older than 14 day(s); kept 1 newer scratch branch(es)." ) );
            const CORE::CString branches = fixture.Branches();
            ASSERT_FALSE( Contains( branches, oldBranch ) );
            ASSERT_TRUE( Contains( branches, newBranch ) && Contains( branches, "main" ) );
            ASSERT_FALSE( fake.HasPackageVersion( "sweeper", "ci-reports", "11-gcc" ) );
            ASSERT_TRUE( fake.HasPackageVersion( "sweeper", "ci-reports", "12-gcc" ) );
            ASSERT_TRUE( fake.HasPackageVersion( "ci-bot", "ci-reports", "13-gcc" ) );

            ASSERT_TRUE( server.Call( "ci_cleanup", "{\"ci_id\":\"NOPE\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown CI id" ) );
            ASSERT_TRUE( server.Call( "ci_cleanup", "{\"ci_id\":\"WIDGET\",\"older_than_days\":0}", text, isError ) );
            ASSERT_TRUE( isError );         // below the minimum of the schema
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: Wrong arguments are refused: missing and unknown ones, wrong types and values, filter values the CI does not have, a repository of another CI" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Arguments" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl() ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/matrix.json" ),
                "{\"cppStandards\":[\"98\",\"17\"],\"toolchains\":[{\"id\":\"gcc-linux-x64\",\"enabled\":true}]}" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "projects/CI/matrix/products.json" ), "{\"products\":[{\"name\":\"platform\"}]}" ) );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = false;
            const CORE::CString repo = Json( fixture.repo );
            const CORE::CString calls[][ 2 ] = { { "ci_submit", "{}" },
                                                 { "ci_submit", "{\"ci_id\":\"WIDGET\"}" },
                                                 { "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + repo + ",\"mode\":\"fast\"}" },
                                                 { "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + repo + ",\"dry_run\":\"yes\"}" },
                                                 { "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + repo + ",\"surprise\":1}" },
                                                 { "ci_status", "{\"run_id\":\"WIDGET:1\",\"wait_seconds\":\"x\"}" },
                                                 { "ci_job_details", "{\"run_id\":\"WIDGET:1\",\"job_id\":\"a\",\"part\":\"all\"}" },
                                                 { "ci_report", "{}" },
                                                 { "ci_resolve_repo", "{\"remotes\":\"https://example.invalid/acme/widget.git\"}" },
                                                 { "ci_no_such_tool", "{}" } };
            for ( int i=0; i<10; ++i )
            {
                ASSERT_TRUE( server.Call( calls[ i ][ 0 ], calls[ i ][ 1 ], text, isError ) );
                ASSERT_TRUE( isError );
            }

            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + repo + ",\"toolchains\":[\"msvc\"],\"cpp_standards\":[\"03\"],\"products\":[\"nope\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "toolchains msvc (known: gcc-linux-x64)" ) && Contains( text, "cpp_standards 03 (known: 98, 17)" ) &&
                         Contains( text, "products nope (known: platform)" ) );
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"NOPE\",\"repo_dir\":" + repo + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Unknown CI id" ) );
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( CORE::CombinePath( fixture.dir, "missing" ) ) + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "not an existing directory" ) );

            // a checkout of something else is never sent
            ASSERT_TRUE( 0 == Git( fixture.repo, "remote", "set-url", "origin", "https://example.invalid/acme/other.git" ) );
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + repo + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "is not a checkout of CI id WIDGET" ) && Contains( text, "No CI is set up for it" ) );
            ASSERT_TRUE( 0 == fake.GetDispatchCount() && fixture.Branches().IsNULLOrEmpty() );
            ASSERT_TRUE( StopServer( server ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: The token reaches git and the CI through headers only and never the output: a push over HTTP carries it, its failure does not show it" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Token" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo fake( Token, "ci", "widget" );
            fake.SetBareRepo( fixture.bare );
            ASSERT_TRUE( fake.Start() );
            // the scratch repository over HTTP at the fake, which has no git service: the push fails after sending the header
            ASSERT_TRUE( fixture.WriteConfig( fake.GetUrl(), fake.GetUrl() + "/ci/widget.git" ) );

            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs() ) );
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( server.Call( "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + "}", text, isError, 120000 ) );
            ASSERT_TRUE( isError && Contains( text, "Sending the snapshot failed (git push" ) );
            ASSERT_FALSE( Contains( text, Token ) );
            ASSERT_TRUE( Contains( fake.GetRequestLog(), "/ci/widget.git/info/refs" ) );
            ASSERT_TRUE( 0 == fake.GetUnauthorizedCount() );                     // git sent "Authorization: token ..."
            ASSERT_FALSE( Contains( GitOut( fixture.repo, "config", "--list" ), Token ) );
            ASSERT_TRUE( StopServer( server ) );

            // nothing any server of these tests answered or logged holds the token
            ASSERT_FALSE( g_transcripts.IsNULLOrEmpty() );
            ASSERT_FALSE( Contains( g_transcripts, Token ) );
            ASSERT_FALSE( Contains( g_transcripts, "5f3a9c1e7b2d4f6a8c0e" ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: Serving HTTP and HTTPS: clients with the token are served on both, without it refused; SIGTERM stops the server" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
            TESTFW::CTlsTestCerts certs;
            GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );
            CCiFixture fixture( "gucefMcpCi_TestApp_Http" );
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9" ) );
            const CORE::CString tokenFile = CORE::CombinePath( fixture.dir, "mcp.token" );

            const UInt16 plainPort = TESTFW::CTlsTestServer::FindFreePort();
            UInt16 tlsPort = TESTFW::CTlsTestServer::FindFreePort();
            for ( int i=0; i<10 && tlsPort == plainPort; ++i )
                tlsPort = TESTFW::CTlsTestServer::FindFreePort();
            CORE::CProcess::TStringVector args = fixture.ServerArgs();
            args.push_back( "--listen" ); args.push_back( "127.0.0.1:" + CORE::UInt16ToString( plainPort ) );
            args.push_back( "--listen-tls" ); args.push_back( "127.0.0.1:" + CORE::UInt16ToString( tlsPort ) );
            args.push_back( "--tls-cert" ); args.push_back( certs.GetPath( "good.crt" ) );
            args.push_back( "--tls-key" ); args.push_back( certs.GetPath( "good.key" ) );
            args.push_back( "--token-file" ); args.push_back( tokenFile );
            CORE::CProcess process;
            process.SetIsolateFromParent( true );
            ASSERT_TRUE( process.Launch( FindServer(), args, CORE::CString::Empty, ChildEnvironment() ) );
            bool listening = false;
            for ( int i=0; i<500 && !listening; ++i )
            {
                listening = TESTFW::CTlsTestServer::CanConnect( plainPort ) && TESTFW::CTlsTestServer::CanConnect( tlsPort );
                if ( !listening )
                    MT::ThreadDelay( 20 );
            }
            ASSERT_TRUE( listening );

            // the token file was made for the owner only
            CORE::CString token;
            ASSERT_TRUE( ReadFile( tokenFile, token ) );
            token = Trimmed( token );
            ASSERT_TRUE( token.Length() >= 32 );

            const CORE::CString initialize = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2025-11-25\","
                                             "\"capabilities\":{},\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}";
            const CORE::CString resolve = "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":{\"name\":\"ci_resolve_repo\",\"arguments\":"
                                          "{\"remotes\":[\"git@example.invalid:acme/widget.git\"]}}}";
            const CORE::CString urls[] = { "http://127.0.0.1:" + CORE::UInt16ToString( plainPort ) + "/mcp", "https://localhost:" + CORE::UInt16ToString( tlsPort ) + "/mcp" };
            for ( int u=0; u<2; ++u )
            {
                CORE::CString text;
                ASSERT_TRUE( 200 == McpHttpPost( urls[ u ], token, initialize, certs.GetPath( "ca.crt" ), text ) );
                ASSERT_TRUE( Contains( text, "The tools run on the machine TestBox" ) );
                ASSERT_TRUE( 200 == McpHttpPost( urls[ u ], token, resolve, certs.GetPath( "ca.crt" ), text ) );
                ASSERT_TRUE( Contains( text, "CI id: WIDGET" ) );
                ASSERT_TRUE( 401 == McpHttpPost( urls[ u ], "wrong", resolve, certs.GetPath( "ca.crt" ), text ) );
            }

            // a client that does not trust the certificate gets no answer over HTTPS
            CORE::CString text;
            ASSERT_TRUE( 200 != McpHttpPost( urls[ 1 ], token, resolve, certs.GetPath( "other_ca.crt" ), text ) );

            ASSERT_TRUE( process.Terminate( true, 15000 ) );
            ASSERT_FALSE( process.IsRunning() );
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
