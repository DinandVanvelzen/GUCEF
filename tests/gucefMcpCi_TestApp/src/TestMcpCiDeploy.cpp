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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestMcpSupport.h"
#include "TestMcpCiSupport.h"
#include "TestCiServer.h"
#include "TestCiGateway.h"
#include "TestFakeForgejo.h"
#include "TestFakeRegistry.h"
#include "TestMcpCiDeploy.h"

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

/** The secrets of these tests: none of them may appear in any output, log or audit line */
const char* ForgejoToken = "dptok-1a3c5e7a9b2d4f6a8c0e-SECRET";
const char* DeployerToken = "dpgw-7c9e1a3b5d7f9a2c4e6a-SECRET";
const char* ViewerToken = "dpview-2b4d6f8a0c1e3a5c7e9b-SECRET";
const char* OddToken = "dpodd-5e7a9c1b3d5f7a9c2e4a-SECRET";
const char* KubeSecret = "kubeconfig-content-9f8e7d6c5b4a-NEVER-SHOWN";
const char* Remote = "https://example.invalid/acme/widget.git";
const char* ImageName = "widget-service";
const char* Repository = "ci/widget-service";

/** What every server of these tests answered and logged */
CORE::CString g_transcripts;

/*-------------------------------------------------------------------------*/

/** A stand-in for kubectl: checks every call against the ServiceAccount of the deployer and answers from state files */
const char* FakeKubectlScript =
    "#!/bin/sh\n"
    "S='@STATE@'\n"
    "K='@KUBECONFIG@'\n"
    "for a in \"$@\"; do printf '%s\\t' \"$a\"; done >> \"$S/argv.log\"\n"
    "printf '\\n' >> \"$S/argv.log\"\n"
    "fail() { echo \"$*\" >> \"$S/violations.log\"; echo \"fake kubectl: $*\" >&2; exit 3; }\n"
    "[ \"$KUBECONFIG\" = \"$K\" ] || fail \"KUBECONFIG is $KUBECONFIG\"\n"
    "kc=''; ctx=''; ns=''\n"
    "while [ $# -gt 0 ]; do\n"
    "  case \"$1\" in\n"
    "    --kubeconfig) kc=\"$2\"; shift 2 ;;\n"
    "    --context) ctx=\"$2\"; shift 2 ;;\n"
    "    -n) ns=\"$2\"; shift 2 ;;\n"
    "    --request-timeout=*) shift ;;\n"
    "    -*) fail \"unknown flag $1\" ;;\n"
    "    *) break ;;\n"
    "  esac\n"
    "done\n"
    "[ \"$kc\" = \"$K\" ] && [ -f \"$kc\" ] || fail \"kubeconfig $kc\"\n"
    "[ -n \"$ns\" ] && [ \"$ctx\" = \"$ns\" ] || fail \"context $ctx for namespace $ns\"\n"
    "D=\"$S/$ns\"\n"
    "[ -d \"$D\" ] || { echo \"Error from server (Forbidden): namespace $ns\"; exit 1; }\n"
    "dep=$(cat \"$D/deployment\")\n"
    "verb=\"$1\"; shift\n"
    "case \"$verb\" in\n"
    "  get)\n"
    "    [ \"$1\" = deployment ] && [ \"$2\" = \"$dep\" ] && [ \"$3\" = -o ] && [ $# -eq 4 ] || fail \"get $*\"\n"
    "    case \"$4\" in jsonpath=*) ;; *) fail \"get output $4\" ;; esac\n"
    "    printf '%s\\t%s\\t%s\\t%s\\t%s\\t1\\t%s\\t1\\t1\\n' \"$(cat \"$D/image\")\" \"$(cat \"$D/policy\")\" \"$(cat \"$D/revision\")\" \"$(cat \"$D/cause\")\" \"$(cat \"$D/strategy\")\" \"$(cat \"$D/ready\")\"\n"
    "    ;;\n"
    "  patch)\n"
    "    [ \"$1\" = \"deployment/$dep\" ] && [ \"$2\" = --type=strategic ] && [ \"$3\" = -p ] && [ $# -eq 4 ] || fail \"patch $*\"\n"
    "    c=\"$(printf '%s' \"$4\" | sed -n 's/.*\"containers\":\\[{\"name\":\"\\([^\"]*\\)\",\"image\":\"[^\"]*\"}\\].*/\\1/p')\"\n"
    "    v=\"$(printf '%s' \"$4\" | sed -n 's/.*\"image\":\"\\([^\"]*\\)\"}\\].*/\\1/p')\"\n"
    "    k=\"$(printf '%s' \"$4\" | sed -n 's/.*\"kubernetes.io\\/change-cause\":\"\\([^\"]*\\)\".*/\\1/p')\"\n"
    "    [ -n \"$v\" ] && [ -n \"$k\" ] || fail \"patch body $4\"\n"
    "    [ \"$c\" = \"$(cat \"$D/container\")\" ] || { echo \"error: unable to find container named $c\"; exit 1; }\n"
    "    cp \"$D/image\" \"$D/prev_image\"; cp \"$D/cause\" \"$D/prev_cause\"\n"
    "    printf '%s' \"$v\" > \"$D/image\"; printf '%s' \"$k\" > \"$D/cause\"\n"
    "    echo $(( $(cat \"$D/revision\") + 1 )) > \"$D/revision\"\n"
    "    echo \"deployment.apps/$dep patched\"\n"
    "    ;;\n"
    "  rollout)\n"
    "    sub=\"$1\"; shift\n"
    "    [ \"$1\" = \"deployment/$dep\" ] || fail \"rollout target $1\"\n"
    "    case \"$sub\" in\n"
    "      status)\n"
    "        case \"$2\" in --timeout=*s) ;; *) fail \"rollout status $*\" ;; esac\n"
    "        [ -s \"$D/rollout_sleep\" ] && sleep \"$(cat \"$D/rollout_sleep\")\"\n"
    "        if [ -s \"$D/rollout_output\" ]; then cat \"$D/rollout_output\"; else echo \"deployment \\\"$dep\\\" successfully rolled out\"; fi\n"
    "        exit \"$(cat \"$D/rollout_exit\" 2>/dev/null || echo 0)\"\n"
    "        ;;\n"
    "      undo)\n"
    "        [ -s \"$D/prev_image\" ] || { echo \"error: no rollout history found for deployment $dep\"; exit 1; }\n"
    "        cp \"$D/image\" \"$D/next_image\"; cp \"$D/cause\" \"$D/next_cause\"\n"
    "        cp \"$D/prev_image\" \"$D/image\"; cp \"$D/prev_cause\" \"$D/cause\"\n"
    "        mv \"$D/next_image\" \"$D/prev_image\"; mv \"$D/next_cause\" \"$D/prev_cause\"\n"
    "        echo $(( $(cat \"$D/revision\") + 1 )) > \"$D/revision\"\n"
    "        echo \"deployment.apps/$dep rolled back\"\n"
    "        ;;\n"
    "      history)\n"
    "        echo \"deployment.apps/$dep\"; echo \"REVISION  CHANGE-CAUSE\"; echo \"$(cat \"$D/revision\")  $(cat \"$D/cause\")\"\n"
    "        ;;\n"
    "      *) fail \"rollout $sub\" ;;\n"
    "    esac\n"
    "    ;;\n"
    "  *) fail \"verb $verb\" ;;\n"
    "esac\n";

/*-------------------------------------------------------------------------*/

CORE::CString
After( const CORE::CString& text, const CORE::CString& marker )
{
    const Int32 at = text.HasSubstr( marker, true );
    if ( at < 0 )
        return CORE::CString::Empty;
    CORE::CString rest = text.CutChars( (UInt32) at + marker.Length(), true );
    const Int32 newline = rest.HasChar( '\n', true );
    return newline >= 0 ? rest.SubstrToIndex( (UInt32) newline, true ) : rest;
}

/*-------------------------------------------------------------------------*/

/** sha256:<64 hexadecimal digits> made from a text */
CORE::CString
DigestOf( const CORE::CString& text )
{
    return "sha256:" + Sha256Hex( text.C_String(), text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::TStageVector
GreenRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "success", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "success" ).AddJob( "images", "success" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::TStageVector
RedRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "failure" ).AddJob( "images", "skipped" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ImagesJson( const CORE::CString& run, const CORE::CString& commit, bool scratch, const CORE::CString& imageStatus, const CORE::CString& digest )
{
    const CORE::CString tag = "sha-" + commit.SubstrToIndex( 12, true );
    return "{\"schema\":1,\"run\":" + Json( run ) + ",\"commit\":" + Json( commit ) + ",\"ref\":\"refs/heads/scratch/release-x\",\"scratch\":" +
           ( scratch ? "true" : "false" ) + ",\"status\":" + Json( imageStatus ) + ",\"images\":[{\"name\":\"widget-service\",\"registryName\":\"widget-service\",\"tag\":" +
           Json( tag ) + ",\"alsoTags\":[\"candidate\"],\"digest\":" + Json( imageStatus == "ok" ? digest : CORE::CString::Empty ) + ",\"status\":" + Json( imageStatus ) +
           ",\"seconds\":42,\"logTail\":" + Json( imageStatus == "ok" ? CORE::CString( "pushed" ) : CORE::CString( "#5 ERROR: failed to solve: exit code 2\n" ) ) + "}]}";
}

/*-------------------------------------------------------------------------*/

/** The log of an image build that failed, as BuildKit and build_image.sh write it: no file:line in it */
const char* FailedImageLog =
    "#1 [internal] load build definition from gucef_service.dockerfile\n"
    "#1 DONE 0.0s\n"
    "#2 [internal] load metadata for 172.16.1.125:32001/ci/gucef-dev-base:latest\n"
    "#2 ERROR: failed to do request: Head \"https://172.16.1.125:32001/v2/ci/gucef-dev-base/manifests/latest\": http: server gave HTTP response to HTTPS client\n"
    "------\n"
    " > [internal] load metadata for 172.16.1.125:32001/ci/gucef-dev-base:latest:\n"
    "------\n"
    "error: failed to solve: 172.16.1.125:32001/ci/gucef-dev-base:latest: failed to resolve source metadata for 172.16.1.125:32001/ci/gucef-dev-base:latest: "
    "failed to do request: Head \"https://172.16.1.125:32001/v2/ci/gucef-dev-base/manifests/latest\": http: server gave HTTP response to HTTPS client\n"
    "build_image: the BuildKit daemon (buildkitd) could not read the development image 172.16.1.125:32001/ci/gucef-dev-base:latest; is the registry marked as plain HTTP in buildkitd.toml?\n";

/** images.json of a run with one image that failed and one that was pushed */
CORE::CString
FailedImagesJson( const CORE::CString& run, const CORE::CString& commit, const CORE::CString& okDigest )
{
    const CORE::CString tag = "sha-" + commit.SubstrToIndex( 12, true );
    return "{\"schema\":1,\"run\":" + Json( run ) + ",\"commit\":" + Json( commit ) + ",\"ref\":\"refs/heads/scratch/x\",\"scratch\":true,\"status\":\"failed\",\"images\":["
           "{\"name\":\"widget-service\",\"registryName\":\"widget-service\",\"tag\":" + Json( tag ) + ",\"alsoTags\":[],\"digest\":\"\",\"status\":\"failed\","
           "\"seconds\":17,\"logTail\":" + Json( FailedImageLog ) + "},"
           "{\"name\":\"helper-service\",\"registryName\":\"helper-service\",\"tag\":" + Json( tag ) + ",\"alsoTags\":[],\"digest\":" + Json( okDigest ) +
           ",\"status\":\"ok\",\"seconds\":31,\"logTail\":\"pushed\"}]}";
}

/*-------------------------------------------------------------------------*/

/** Many lines before the failure: the saved log is all of it, the answer only its end */
CORE::CString
NoisyPrefix( void )
{
    CORE::CString text;
    for ( UInt32 i=0; i<300; ++i )
        text += "#3 sha256:" + CORE::UInt32ToString( i ) + " 1.2MB / 40MB 0.4s\n";
    return text;
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::TStageVector
FailedImagesRun( void )
{
    CFakeForgejo::TStageVector stages;
    stages.push_back( CFakeForgejo::CStage( "failure", 1 ).AddJob( "plan", "success" ).AddJob( "gcc-linux-x64-cxx17-Release", "success" ).AddJob( "images", "failure" ) );
    return stages;
}

/*-------------------------------------------------------------------------*/

/** The lines of a file */
CORE::CString::StringVector
Lines( const CORE::CString& path )
{
    CORE::CString text;
    ReadFile( path, text );
    return text.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
}

/*-------------------------------------------------------------------------*/

/**
 *  A scratch directory with a working repository, a bare scratch repository, a bare published repository (the pull
 *  mirror), the registry, the CI token, a kubeconfig and the fake kubectl with its state per namespace
 */
class CDeployFixture
{
    public:

    explicit CDeployFixture( const char* name )
        : m_scratch( name )
        , dir( m_scratch.GetPath() )
        , repo( CORE::CombinePath( dir, "work" ) )
        , bare( CORE::CombinePath( dir, "scratch.git" ) )
        , published( CORE::CombinePath( dir, "published.git" ) )
        , config( CORE::CombinePath( dir, "ci.json" ) )
        , tokenFile( CORE::CombinePath( dir, "forgejo.token" ) )
        , logDir( CORE::CombinePath( dir, "logs" ) )
        , workDir( CORE::CombinePath( dir, "work-dir" ) )
        , kubeconfig( CORE::CombinePath( dir, "kubeconfig" ) )
        , kubectl( CORE::CombinePath( dir, "kubectl" ) )
        , kubeState( CORE::CombinePath( dir, "kube" ) )
    {
    }

    /** The repositories: one published commit on main of the published repository, which is also the seed of the scratch one */
    bool MakeRepos( void )
    {
        CORE::CreateDirs( repo );
        CORE::CreateDirs( bare );
        CORE::CreateDirs( published );
        if ( 0 != Git( bare, "init", "-q", "--bare" ) || 0 != Git( published, "init", "-q", "--bare" ) || 0 != Git( repo, "init", "-q" ) )
            return false;
        Git( published, "symbolic-ref", "HEAD", "refs/heads/main" );
        Git( repo, "checkout", "-q", "-b", "main" );
        if ( !WriteFile( CORE::CombinePath( repo, "a.txt" ), "alpha\n" ) )
            return false;
        return 0 == Git( repo, "add", "-A" ) && GitCommit( repo, "first" ) && 0 == Git( repo, "remote", "add", "origin", Remote ) &&
               0 == Git( repo, "push", "-q", published.C_String(), "HEAD:refs/heads/main" ) && 0 == Git( repo, "push", "-q", bare.C_String(), "HEAD:refs/heads/main" );
    }

    /** A commit of the working repository, pushed to a branch of the published repository unless branch is empty */
    CORE::CString Commit( const char* file, const char* content, const char* branch )
    {
        if ( !WriteFile( CORE::CombinePath( repo, file ), content ) || 0 != Git( repo, "add", "-A" ) || !GitCommit( repo, file ) )
            return CORE::CString::Empty;
        const CORE::CString commit = GitOut( repo, "rev-parse", "HEAD" );
        if ( GUCEF_NULL != branch && 0 != Git( repo, "push", "-q", "-f", published.C_String(), ( "HEAD:refs/heads/" + CORE::CString( branch ) ).C_String() ) )
            return CORE::CString::Empty;
        return commit;
    }

    CORE::CString Entry( const CORE::CString& forgejoUrl, const CORE::CString& registryHost ) const
    {
        return "{\"id\":\"WIDGET\",\"description\":\"the widget test project\",\"backend\":\"forgejo\",\"url\":" + Json( forgejoUrl ) +
               ",\"owner\":\"acme\",\"repo\":\"widget\",\"publishedUrl\":" + Json( published ) + ",\"scratchOwner\":\"ci\",\"scratchRepo\":\"widget\",\"scratchUrl\":" +
               Json( bare ) + ",\"workflow\":\"gucef-ci.yml\",\"retentionDays\":0,\"remotes\":[\"https://example.invalid/acme/widget.git\"]," +
               "\"registryHost\":" + Json( registryHost ) + ",\"registryUser\":\"ci-bot\"," +
               "\"deploy\":{\"images\":[{\"name\":\"widget-service\",\"targets\":{" +
               "\"live\":{\"namespace\":\"widgets\",\"deployment\":\"widget\",\"container\":\"widget\",\"allowScratch\":false}," +
               "\"dev\":{\"namespace\":\"widgets-dev\",\"deployment\":\"widget\",\"container\":\"widget\",\"allowScratch\":true}}}]}}";
    }

    bool WriteConfig( const CORE::CString& forgejoUrl, const CORE::CString& registryHost, const CORE::CString& otherEntries = CORE::CString::Empty )
    {
        return WriteFile( config, "{\"ci\":[" + Entry( forgejoUrl, registryHost ) + otherEntries + "]}" ) &&
               WriteOwnerOnlyFile( tokenFile, CORE::CString( ForgejoToken ) + "\n" );
    }

    /** The kubeconfig, the script and the Deployments widgets/widget and widgets-dev/widget, both on a registry image */
    bool WriteKubectl( void )
    {
        CORE::CreateDirs( kubeState );
        const CORE::CString script = CORE::CString( FakeKubectlScript ).ReplaceSubstr( "@STATE@", kubeState ).ReplaceSubstr( "@KUBECONFIG@", kubeconfig );
        CORE::CString output;
        CORE::CProcess::TStringVector chmod;
        chmod.push_back( "+x" );
        chmod.push_back( kubectl );
        if ( !WriteOwnerOnlyFile( kubeconfig, "apiVersion: v1\nkind: Config\nusers:\n- name: deployer\n  user:\n    token: " + CORE::CString( KubeSecret ) + "\n" ) ||
             !WriteFile( kubectl, script ) || 0 != RunCapture( "chmod", chmod, output ) )
            return false;
        const char* namespaces[] = { "widgets", "widgets-dev" };
        for ( int i=0; i<2; ++i )
        {
            const CORE::CString ns = namespaces[ i ];
            CORE::CreateDirs( CORE::CombinePath( kubeState, ns ) );
            SetState( ns, "deployment", "widget" );
            SetState( ns, "container", "widget" );
            SetState( ns, "image", "127.0.0.1:1/ci/widget-service@" + DigestOf( "the image before" ) );
            SetState( ns, "policy", "IfNotPresent" );
            SetState( ns, "revision", "1" );
            SetState( ns, "cause", "set up by hand" );
            SetState( ns, "strategy", "RollingUpdate" );
            SetState( ns, "ready", "1" );
        }
        return true;
    }

    void SetState( const CORE::CString& ns, const char* name, const CORE::CString& value ) const
    {
        WriteFile( CORE::CombinePath( CORE::CombinePath( kubeState, ns ), name ), value );
    }

    CORE::CString State( const CORE::CString& ns, const char* name ) const
    {
        CORE::CString text;
        ReadFile( CORE::CombinePath( CORE::CombinePath( kubeState, ns ), name ), text );
        return Trimmed( text );
    }

    CORE::CString KubectlLog( void ) const
    {
        CORE::CString text;
        ReadFile( CORE::CombinePath( kubeState, "argv.log" ), text );
        return text;
    }

    CORE::CString Violations( void ) const
    {
        CORE::CString text;
        ReadFile( CORE::CombinePath( kubeState, "violations.log" ), text );
        return text;
    }

    void ClearKubectlLog( void ) const
    {
        WriteFile( CORE::CombinePath( kubeState, "argv.log" ), CORE::CString::Empty );
    }

    CORE::CString AuditPath( void ) const
    {
        return CORE::CombinePath( CORE::CombinePath( workDir, "audit" ), "deploy.jsonl" );
    }

    CORE::CProcess::TStringVector ServerArgs( bool allowDeploy ) const
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "--mode" ); args.push_back( "direct" );
        args.push_back( "--ci-config" ); args.push_back( config );
        args.push_back( "--forgejo-token-file" ); args.push_back( tokenFile );
        args.push_back( "--lablink-ci-dir" ); args.push_back( "none" );
        args.push_back( "--log-dir" ); args.push_back( logDir );
        args.push_back( "--work-dir" ); args.push_back( workDir );
        args.push_back( "--machine-name" ); args.push_back( "TestBox" );
        args.push_back( "--poll-interval-ms" ); args.push_back( "200" );
        args.push_back( "--max-wait-seconds" ); args.push_back( "30" );
        args.push_back( "--kubectl" ); args.push_back( kubectl );
        args.push_back( "--kubeconfig" ); args.push_back( kubeconfig );
        args.push_back( "--deploy-timeout-seconds" ); args.push_back( "20" );
        if ( allowDeploy )
            args.push_back( "--allow-deploy" );
        return args;
    }

    private:

    TESTFW::CScopedTestDir m_scratch;

    public:

    const CORE::CString dir;
    const CORE::CString repo;
    const CORE::CString bare;
    const CORE::CString published;
    const CORE::CString config;
    const CORE::CString tokenFile;
    const CORE::CString logDir;
    const CORE::CString workDir;
    const CORE::CString kubeconfig;
    const CORE::CString kubectl;
    const CORE::CString kubeState;
};

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

/** Keeps the audit and the calls of the fake kubectl for the check that no secret is in them */
void
Keep( const CDeployFixture& fixture )
{
    CORE::CString audit;
    ReadFile( fixture.AuditPath(), audit );
    g_transcripts += audit + fixture.KubectlLog() + fixture.Violations();
}

/*-------------------------------------------------------------------------*/

/** A tool call that must succeed. @return its text, empty when it failed */
CORE::CString
CallOk( CTestCiServer& server, const char* tool, const CORE::CString& arguments, UInt32 timeoutInMs = 120000 )
{
    CORE::CString text;
    bool isError = true;
    if ( !server.Call( tool, arguments, text, isError, timeoutInMs ) || isError )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi_TestApp: " << tool << " failed: " << text );
        return CORE::CString::Empty;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** A tool call that must fail. @return its text, empty when it did not fail */
CORE::CString
CallError( CTestCiServer& server, const char* tool, const CORE::CString& arguments, UInt32 timeoutInMs = 120000 )
{
    CORE::CString text;
    bool isError = false;
    if ( !server.Call( tool, arguments, text, isError, timeoutInMs ) || !isError )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi_TestApp: " << tool << " did not fail: " << text );
        return CORE::CString::Empty;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
DeployArguments( const CORE::CString& image, const CORE::CString& target, const CORE::CString& runId, const char* confirm )
{
    return "{\"ci_id\":\"WIDGET\",\"image\":" + Json( image ) + ",\"target\":" + Json( target ) + ",\"run_id\":" + Json( runId ) +
           ( GUCEF_NULL == confirm ? CORE::CString::Empty : ",\"confirm\":" + CORE::CString( confirm ) ) + "}";
}

/*-------------------------------------------------------------------------*/

/**
 *  Builds a published commit with the image through ci_submit commit, publishes its image report and puts the image
 *  in the registry. @return the run id, empty when that failed
 */
CORE::CString
BuildRelease( CTestCiServer& server, CFakeForgejo& forgejo, CFakeRegistry& registry, const CORE::CString& commit, const CORE::CString& digest,
              bool green = true, const char* imageStatus = "ok" )
{
    forgejo.ScriptNextRun( green ? GreenRun() : RedRun() );
    const CORE::CString text = CallOk( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( commit ) + ",\"images\":[\"widget-service\"]}" );
    const CORE::CString runId = After( text, "Run id: " );
    if ( runId.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString run = CORE::Int64ToString( forgejo.GetLastRunId() );
    forgejo.AddPackageFile( "ci-bot", "ci-reports", run + "-images", "images.json", ImagesJson( run, commit, false, imageStatus, digest ), CORE::CString::Empty );
    registry.AddManifest( Repository, "sha-" + commit.SubstrToIndex( 12, true ), digest );
    return runId;
}

/*-------------------------------------------------------------------------*/

/** Whether every call of the fake kubectl named the kubeconfig, a context equal to its namespace, and nothing it may not do */
bool
KubectlCallsAllowed( const CDeployFixture& fixture )
{
    if ( !fixture.Violations().IsNULLOrEmpty() )
        return false;
    const CORE::CString::StringVector lines = fixture.KubectlLog().ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString& line = lines[ i ];
        const bool widgets = line.StartsWith( "--kubeconfig\t" + fixture.kubeconfig + "\t--context\twidgets\t-n\twidgets\t" );
        const bool dev = line.StartsWith( "--kubeconfig\t" + fixture.kubeconfig + "\t--context\twidgets-dev\t-n\twidgets-dev\t" );
        if ( !widgets && !dev )
            return false;
        if ( Contains( line, "\tdelete\t" ) || Contains( line, "\tcreate\t" ) || Contains( line, "\tapply\t" ) || Contains( line, "secret" ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The position of the first line of the kubectl log that holds the text, -1 when none does */
Int32
KubectlLine( const CDeployFixture& fixture, const CORE::CString& part )
{
    const CORE::CString::StringVector lines = fixture.KubectlLog().ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
        if ( Contains( lines[ i ], part ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The audit lines with the result, and whether every line is a JSON object */
UInt32
CountAudit( const CDeployFixture& fixture, const CORE::CString& part, bool& allJson )
{
    const CORE::CString::StringVector lines = Lines( fixture.AuditPath() );
    UInt32 count = 0;
    allJson = !lines.empty();
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CDataNode node;
        if ( !McpTestParse( lines[ i ], node ) || node.GetAttributeValue( "time" ).AsString().IsNULLOrEmpty() )
            allJson = false;
        if ( Contains( lines[ i ], part ) )
            ++count;
    }
    return count;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpCiDeployTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "McpCiDeploy" );

    const bool hasServer = !FindServer().IsNULLOrEmpty() && McpTestJsonAvailable();
    const bool hasGit = GitAvailable();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    const bool hasShell = CORE::FileExists( "/bin/sh" );
    #else
    const bool hasShell = false;
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: ci_submit commit builds a published commit: sent as scratch/release-<sha12> with the images as a dispatch input; unpublished and unknown commits are refused" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployCommit" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo forgejo( ForgejoToken, "ci", "widget" );
            forgejo.SetBareRepo( fixture.bare );
            ASSERT_TRUE( forgejo.Start() );
            ASSERT_TRUE( fixture.WriteConfig( forgejo.GetUrl(), "127.0.0.1:1" ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( false ) ) );

            const CORE::CString commit = fixture.Commit( "b.txt", "bravo\n", "main" );
            ASSERT_FALSE( commit.IsNULLOrEmpty() );
            const CORE::CString before = GitOut( fixture.repo, "status", "--porcelain" );
            forgejo.ScriptNextRun( GreenRun() );
            CORE::CString text = CallOk( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( commit ) + ",\"images\":[\"widget-service\"],\"mode\":\"test\"}" );
            ASSERT_TRUE( Contains( text, "was sent to the CI as scratch/release-" + commit.SubstrToIndex( 12, true ) ) && Contains( text, "then ci_images" ) );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( "refs/heads/scratch/release-" + commit.SubstrToIndex( 12, true ) ).C_String() ) == commit );
            const CORE::CString dispatch = forgejo.GetLastDispatchBody();
            ASSERT_TRUE( Contains( dispatch, "\"images\"" ) && Contains( dispatch, "widget-service" ) && Contains( dispatch, "\"mode\"" ) );
            ASSERT_TRUE( GitOut( fixture.repo, "status", "--porcelain" ) == before );

            // all images, and a commit on another branch of the published repository
            const CORE::CString feature = fixture.Commit( "c.txt", "charlie\n", "feature" );
            text = CallOk( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( feature ) + ",\"images\":[\"all\"]}" );
            ASSERT_TRUE( Contains( forgejo.GetLastDispatchBody(), "\"all\"" ) );

            // a commit that was never pushed, a short one, an unknown one, images that are no names
            const CORE::CString local = fixture.Commit( "d.txt", "delta\n", GUCEF_NULL );
            const UInt32 dispatches = forgejo.GetDispatchCount();
            text = CallError( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( local ) + "}" );
            ASSERT_TRUE( Contains( text, "is not in the published repository" ) );
            text = CallError( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( commit.SubstrToIndex( 10, true ) ) + "}" );
            ASSERT_TRUE( Contains( text, "full 40 character" ) );
            text = CallError( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":\"0123456789abcdef0123456789abcdef01234567\"}" );
            ASSERT_TRUE( Contains( text, "published repository" ) );
            text = CallError( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( commit ) + ",\"images\":[\"Bad Name\"]}" );
            ASSERT_TRUE( Contains( text, "is not an image name" ) );
            text = CallError( server, "ci_submit", "{\"ci_id\":\"WIDGET\"}" );
            ASSERT_TRUE( Contains( text, "Give repo_dir" ) );
            ASSERT_TRUE( dispatches == forgejo.GetDispatchCount() );
            ASSERT_TRUE( 0 == forgejo.GetUnauthorizedCount() );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_deploy guards: not listed, scope, missing confirm, run not green, image failed, scratch to live, unpublished commit, pull policy Never; each refusal changes nothing and is audited" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit || !hasShell, "needs tools/gucefMcpCi, git and /bin/sh" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployGuards" );
            ASSERT_TRUE( fixture.MakeRepos() && fixture.WriteKubectl() );
            CFakeForgejo forgejo( ForgejoToken, "ci", "widget" );
            forgejo.SetBareRepo( fixture.bare );
            ASSERT_TRUE( forgejo.Start() );
            CFakeRegistry registry( CFakeRegistry::MODE_BEARER, "ci-bot", ForgejoToken );
            ASSERT_TRUE( registry.Start() );
            ASSERT_TRUE( fixture.WriteConfig( forgejo.GetUrl(), registry.GetHost() ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( true ) ) );

            const CORE::CString commit = fixture.Commit( "b.txt", "bravo\n", "main" );
            const CORE::CString digest = DigestOf( "release one" );
            const CORE::CString runId = BuildRelease( server, forgejo, registry, commit, digest );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );

            // ci_images from the report of the run
            CORE::CString text = CallOk( server, "ci_images", "{\"run_id\":" + Json( runId ) + "}" );
            ASSERT_TRUE( Contains( text, "- widget-service: ok, tag sha-" + commit.SubstrToIndex( 12, true ) + " (also candidate), digest " + digest ) );
            ASSERT_TRUE( Contains( text, "a build of a published branch" ) && Contains( text, "only when the user asks" ) );
            text = CallError( server, "ci_images", "{\"run_id\":\"WIDGET:4242\"}" );
            ASSERT_TRUE( Contains( text, "Unknown run id" ) );

            // not listed: another image, another target, another CI id
            text = CallError( server, "ci_deploy", DeployArguments( "other-service", "live", runId, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (not_listed)" ) && Contains( text, "Deployable: widget-service (live, dev)" ) );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "prod", runId, "true" ) );
            ASSERT_TRUE( Contains( text, "has no target \"prod\"; its targets: live, dev" ) );
            text = CallError( server, "ci_deploy", "{\"ci_id\":\"NOPE\",\"image\":\"widget-service\",\"target\":\"live\",\"run_id\":" + Json( runId ) + ",\"confirm\":true}" );
            ASSERT_TRUE( Contains( text, "Unknown CI id" ) );

            // confirm: missing, false, and not the boolean true
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", runId, GUCEF_NULL ) );
            ASSERT_TRUE( Contains( text, "Refused (confirm)" ) && Contains( text, "explicitly" ) && Contains( text, "ci_release_status" ) );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", runId, "false" ) );
            ASSERT_TRUE( Contains( text, "Refused (confirm)" ) );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", runId, "\"true\"" ) );
            ASSERT_FALSE( text.IsNULLOrEmpty() );
            text = CallError( server, "ci_deploy", "{\"ci_id\":\"WIDGET\",\"image\":\"widget-service\",\"target\":\"live\",\"confirm\":true}" );
            ASSERT_TRUE( Contains( text, "Give run_id or commit" ) );

            // a server without --allow-deploy has the scope build only
            {
                CTestCiServer reader;
                ASSERT_TRUE( reader.Start( fixture.ServerArgs( false ) ) );
                text = CallError( reader, "ci_deploy", DeployArguments( ImageName, "live", runId, "true" ) );
                ASSERT_TRUE( Contains( text, "Refused (scope)" ) && Contains( text, "TestBox" ) );
                text = CallError( reader, "ci_rollback", "{\"ci_id\":\"WIDGET\",\"target\":\"live\",\"confirm\":true}" );
                ASSERT_TRUE( Contains( text, "Refused (scope)" ) );
                text = CallOk( reader, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
                ASSERT_TRUE( Contains( text, "runs 127.0.0.1:1/ci/widget-service@" + DigestOf( "the image before" ) ) );
                ASSERT_TRUE( StopServer( reader ) );
            }

            // a run that is not green, and a green run whose image failed
            const CORE::CString redCommit = fixture.Commit( "e.txt", "echo\n", "main" );
            const CORE::CString redRun = BuildRelease( server, forgejo, registry, redCommit, DigestOf( "red" ), false );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", redRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (not_green)" ) && Contains( text, "gcc-linux-x64-cxx17-Release (failed)" ) );
            const CORE::CString failedCommit = fixture.Commit( "f.txt", "foxtrot\n", "main" );
            const CORE::CString failedRun = BuildRelease( server, forgejo, registry, failedCommit, DigestOf( "failed" ), true, "failed" );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", failedRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (image_failed)" ) && Contains( text, "failed to solve" ) );

            // a scratch snapshot: not to live, but to dev, which takes scratch builds
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha, not committed\n" ) );
            forgejo.ScriptNextRun( GreenRun() );
            text = CallOk( server, "ci_submit", "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + ",\"images\":[\"widget-service\"]}" );
            const CORE::CString scratchRun = After( text, "Run id: " );
            ASSERT_FALSE( scratchRun.IsNULLOrEmpty() );
            const CORE::CString scratchBranch = GitOut( fixture.bare, "for-each-ref", "--format=%(refname:short)", "refs/heads/scratch/testbox-*" );
            const CORE::CString scratchCommit = GitOut( fixture.bare, "rev-parse", scratchBranch.C_String() );
            ASSERT_TRUE( 40 == scratchCommit.Length() );
            const CORE::CString scratchDigest = DigestOf( "scratch build" );
            const CORE::CString backendRun = CORE::Int64ToString( forgejo.GetLastRunId() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", backendRun + "-images", "images.json", ImagesJson( backendRun, scratchCommit, true, "ok", scratchDigest ), CORE::CString::Empty );
            registry.AddManifest( Repository, "sha-" + scratchCommit.SubstrToIndex( 12, true ), scratchDigest );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", scratchRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (unpublished)" ) && Contains( text, "not in the published repository at all" ) );
            fixture.ClearKubectlLog();
            text = CallOk( server, "ci_deploy", DeployArguments( ImageName, "dev", scratchRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Deployed: widgets-dev/widget runs " + registry.GetHost() + "/ci/widget-service@" + scratchDigest ) );
            ASSERT_TRUE( fixture.State( "widgets-dev", "image" ) == registry.GetHost() + "/ci/widget-service@" + scratchDigest );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == "127.0.0.1:1/ci/widget-service@" + DigestOf( "the image before" ) );

            // a published commit that is not on the default branch
            ASSERT_TRUE( 0 == Git( fixture.repo, "checkout", "-q", "-b", "side", ( commit + "~1" ).C_String() ) );
            const CORE::CString side = fixture.Commit( "g.txt", "golf\n", "side" );
            ASSERT_TRUE( 0 == Git( fixture.repo, "checkout", "-q", "main" ) );
            const CORE::CString sideRun = BuildRelease( server, forgejo, registry, side, DigestOf( "side" ) );
            ASSERT_FALSE( sideRun.IsNULLOrEmpty() );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", sideRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (unpublished)" ) && Contains( text, "not on the default branch" ) );

            // the registry does not have the digest of the report
            const CORE::CString movedCommit = fixture.Commit( "h.txt", "hotel\n", "main" );
            const CORE::CString movedRun = BuildRelease( server, forgejo, registry, movedCommit, DigestOf( "reported" ) );
            registry.AddManifest( Repository, "sha-" + movedCommit.SubstrToIndex( 12, true ), DigestOf( "moved" ) );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", movedRun, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (registry)" ) && Contains( text, "nothing is deployed" ) );

            // a Deployment that still runs an image imported by hand
            fixture.SetState( "widgets", "policy", "Never" );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", runId, "true" ) );
            ASSERT_TRUE( Contains( text, "Refused (pull_policy)" ) && Contains( text, "IMAGE_REGISTRY" ) );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == "127.0.0.1:1/ci/widget-service@" + DigestOf( "the image before" ) );
            ASSERT_TRUE( !Contains( fixture.KubectlLog(), "\"image\":\"" + registry.GetHost() + "/ci/widget-service@" + digest + "\"" ) );

            ASSERT_TRUE( KubectlCallsAllowed( fixture ) );
            bool allJson = false;
            const char* codes[] = { "\"not_listed\"", "\"confirm\"", "\"scope\"", "\"not_green\"", "\"image_failed\"", "\"unpublished\"", "\"registry\"", "\"pull_policy\"" };
            for ( int i=0; i<8; ++i )
                ASSERT_TRUE( CountAudit( fixture, codes[ i ], allJson ) > 0 );
            ASSERT_TRUE( allJson );
            ASSERT_TRUE( 1 == CountAudit( fixture, "\"result\":\"deployed\"", allJson ) + CountAudit( fixture, "\"result\": \"deployed\"", allJson ) );
            ASSERT_TRUE( registry.GetTokenRequestCount() > 0 );
            Keep( fixture );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci_deploy to live: the exact kubectl calls (change cause, image by digest, rollout status), unchanged when asked again, release status, a failed and a timed out rollout reported without undo, rollback, the audit" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit || !hasShell, "needs tools/gucefMcpCi, git and /bin/sh" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployLive" );
            ASSERT_TRUE( fixture.MakeRepos() && fixture.WriteKubectl() );
            CFakeForgejo forgejo( ForgejoToken, "ci", "widget" );
            forgejo.SetBareRepo( fixture.bare );
            ASSERT_TRUE( forgejo.Start() );
            CFakeRegistry registry( CFakeRegistry::MODE_BEARER, "ci-bot", ForgejoToken );
            ASSERT_TRUE( registry.Start() );
            ASSERT_TRUE( fixture.WriteConfig( forgejo.GetUrl(), registry.GetHost() ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( true ) ) );

            const CORE::CString before = "127.0.0.1:1/ci/widget-service@" + DigestOf( "the image before" );
            const CORE::CString commit = fixture.Commit( "b.txt", "bravo\n", "main" );
            const CORE::CString digest = DigestOf( "release one" );
            const CORE::CString runId = BuildRelease( server, forgejo, registry, commit, digest );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            CORE::CString text = CallOk( server, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "- image widget-service: Deployment widgets/widget, container widget (published commits only)" ) && Contains( text, "runs " + before ) );

            // by commit instead of run id: the newest run of the commit
            fixture.ClearKubectlLog();
            text = CallOk( server, "ci_deploy", "{\"ci_id\":\"WIDGET\",\"image\":\"widget-service\",\"target\":\"live\",\"commit\":" + Json( commit ) + ",\"confirm\":true}" );
            const CORE::CString reference = registry.GetHost() + "/ci/widget-service@" + digest;
            ASSERT_TRUE( Contains( text, "Deployed: widgets/widget runs " + reference + " (run " + runId + ", commit " + commit + "), revision 2" ) );
            ASSERT_TRUE( Contains( text, "It ran before: " + before ) );
            const Int32 get = KubectlLine( fixture, "\tget\tdeployment\twidget\t-o\tjsonpath=" );
            const Int32 patch = KubectlLine( fixture, "\tpatch\tdeployment/widget\t--type=strategic\t-p\t{\"metadata\":{\"annotations\":{\"kubernetes.io/change-cause\":\"ci WIDGET run " + runId + " commit " + commit +
                                                      " image widget-service by TestBox at " );
            const Int32 rollout = KubectlLine( fixture, "\trollout\tstatus\tdeployment/widget\t--timeout=20s\t" );
            ASSERT_TRUE( get >= 0 && patch > get && rollout > patch );
            ASSERT_TRUE( Contains( fixture.KubectlLog(), "\"containers\":[{\"name\":\"widget\",\"image\":\"" + reference + "\"}]" ) );
            ASSERT_TRUE( KubectlLine( fixture, "\tannotate\t" ) < 0 && KubectlLine( fixture, "\tset\timage\t" ) < 0 );
            ASSERT_TRUE( KubectlLine( fixture, ":sha-" ) < 0 && KubectlLine( fixture, "undo" ) < 0 );
            ASSERT_TRUE( Contains( fixture.KubectlLog(), "\t--request-timeout=60s\tget\t" ) && !Contains( fixture.KubectlLog(), "--request-timeout=60s\trollout\tstatus" ) );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == reference );
            ASSERT_TRUE( Contains( fixture.State( "widgets", "cause" ), "commit " + commit ) );

            // again: nothing changes
            fixture.ClearKubectlLog();
            text = CallOk( server, "ci_deploy", DeployArguments( ImageName, "live", runId, "true" ) );
            ASSERT_TRUE( Contains( text, "Unchanged: widgets/widget already runs " + reference ) );
            ASSERT_TRUE( KubectlLine( fixture, "\tpatch\t" ) < 0 && KubectlLine( fixture, "\tannotate\t" ) < 0 );

            // what runs now
            text = CallOk( server, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "runs " + reference ) && Contains( text, "pull policy IfNotPresent, revision 2, strategy RollingUpdate, 1 of 1 ready" ) );
            ASSERT_TRUE( Contains( text, "last change: ci WIDGET run " + runId + " commit " + commit ) && Contains( text, "history:" ) );
            text = CallError( server, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"prod\"}" );
            ASSERT_TRUE( Contains( text, "has the target \"prod\"" ) );

            // a rollout that fails: reported with the state, nothing undone
            const CORE::CString second = fixture.Commit( "c.txt", "charlie\n", "main" );
            const CORE::CString secondDigest = DigestOf( "release two" );
            const CORE::CString secondRun = BuildRelease( server, forgejo, registry, second, secondDigest );
            fixture.SetState( "widgets", "rollout_output", "Waiting for deployment \"widget\" rollout to finish: 0 of 1 updated replicas are available...\n"
                                                           "error: deployment \"widget\" exceeded its progress deadline" );
            fixture.SetState( "widgets", "rollout_exit", "1" );
            fixture.SetState( "widgets", "ready", "0" );
            fixture.ClearKubectlLog();
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", secondRun, "true" ) );
            ASSERT_TRUE( Contains( text, "did not complete (rollout_failed)" ) && Contains( text, "exceeded its progress deadline" ) );
            ASSERT_TRUE( Contains( text, "keep serving" ) && Contains( text, "Nothing was rolled back" ) && Contains( text, "0 of 1 ready" ) );
            ASSERT_TRUE( KubectlLine( fixture, "undo" ) < 0 );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == registry.GetHost() + "/ci/widget-service@" + secondDigest );

            // a rollout that times out, with the strategy Recreate
            const CORE::CString third = fixture.Commit( "d.txt", "delta\n", "main" );
            const CORE::CString thirdRun = BuildRelease( server, forgejo, registry, third, DigestOf( "release three" ) );
            fixture.SetState( "widgets", "rollout_output", "error: timed out waiting for the condition" );
            fixture.SetState( "widgets", "strategy", "Recreate" );
            text = CallError( server, "ci_deploy", DeployArguments( ImageName, "live", thirdRun, "true" ) );
            ASSERT_TRUE( Contains( text, "did not complete (rollout_timeout)" ) && Contains( text, "Recreate" ) && Contains( text, "may be down" ) );

            // rollback: the revision before
            fixture.SetState( "widgets", "rollout_output", CORE::CString::Empty );
            fixture.SetState( "widgets", "rollout_exit", "0" );
            fixture.SetState( "widgets", "ready", "1" );
            fixture.SetState( "widgets", "strategy", "RollingUpdate" );
            text = CallError( server, "ci_rollback", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "Refused (confirm)" ) );
            fixture.ClearKubectlLog();
            text = CallOk( server, "ci_rollback", "{\"ci_id\":\"WIDGET\",\"target\":\"live\",\"confirm\":true}" );
            ASSERT_TRUE( Contains( text, "Rolled back: widgets/widget was returned to its previous revision" ) );
            ASSERT_TRUE( KubectlLine( fixture, "\trollout\tundo\tdeployment/widget\t" ) >= 0 && KubectlLine( fixture, "\trollout\tstatus\tdeployment/widget\t" ) >= 0 );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == registry.GetHost() + "/ci/widget-service@" + secondDigest );
            ASSERT_TRUE( Contains( text, "runs " + registry.GetHost() + "/ci/widget-service@" + secondDigest ) );

            ASSERT_TRUE( KubectlCallsAllowed( fixture ) );
            bool allJson = false;
            ASSERT_TRUE( CountAudit( fixture, "deployed", allJson ) >= 1 && CountAudit( fixture, "unchanged", allJson ) >= 1 );
            ASSERT_TRUE( CountAudit( fixture, "rollout_failed", allJson ) == 1 && CountAudit( fixture, "rollout_timeout", allJson ) == 1 );
            ASSERT_TRUE( CountAudit( fixture, "rolled_back", allJson ) == 1 && CountAudit( fixture, "TestBox", allJson ) >= 6 );
            ASSERT_TRUE( CountAudit( fixture, digest, allJson ) >= 1 && CountAudit( fixture, DigestOf( "the image before" ), allJson ) >= 1 );
            ASSERT_TRUE( allJson );
            Keep( fixture );
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

    GUCEF_TESTFW_TESTCASE( "Test: A failed images job: ci_report, ci_job_details and ci_images describe it from images.json, with the status words, the end of the log and the full log saved (gzip or plain); without images.json it says so" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit, "needs tools/gucefMcpCi and git" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployImageFailure" );
            ASSERT_TRUE( fixture.MakeRepos() );
            CFakeForgejo forgejo( ForgejoToken, "ci", "widget" );
            forgejo.SetBareRepo( fixture.bare );
            ASSERT_TRUE( forgejo.Start() );
            ASSERT_TRUE( fixture.WriteConfig( forgejo.GetUrl(), "127.0.0.1:1" ) );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( fixture.ServerArgs( false ) ) );
            const CORE::CString submit = "{\"ci_id\":\"WIDGET\",\"repo_dir\":" + Json( fixture.repo ) + ",\"images\":[\"all\"]}";
            const CORE::CString okDigest = DigestOf( "helper image" );
            const CORE::CString fullLog = "#0 building with the BuildKit daemon\n" + NoisyPrefix() + FailedImageLog;

            // the run, its images.json and the gzip compressed log of the image that failed
            forgejo.ScriptNextRun( FailedImagesRun() );
            CORE::CString text = CallOk( server, "ci_submit", submit );
            const CORE::CString runId = After( text, "Run id: " );
            ASSERT_FALSE( runId.IsNULLOrEmpty() );
            const CORE::CString run = CORE::Int64ToString( forgejo.GetLastRunId() );
            const CORE::CString commit = GitOut( fixture.bare, "rev-parse", GitOut( fixture.bare, "for-each-ref", "--format=%(refname)", "refs/heads/scratch/testbox-*" ).C_String() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", run + "-images", "images.json", FailedImagesJson( run, commit, okDigest ), CORE::CString::Empty );
            CORE::CDynamicBuffer compressed;
            const bool hasGzip = Gzip( fullLog, compressed );
            if ( hasGzip )
                forgejo.AddPackageFile( "ci-bot", "ci-reports", run + "-images", "widget-service.log.gz", compressed, CORE::CString::Empty );
            else
                forgejo.AddPackageFile( "ci-bot", "ci-reports", run + "-images", "widget-service.log", fullLog, CORE::CString::Empty );

            text = CallOk( server, "ci_report", "{\"run_id\":" + Json( runId ) + "}" );
            ASSERT_TRUE( Contains( text, "- images: (images)\n    image widget-service: failed, tag sha-" + commit.SubstrToIndex( 12, true ) + ", 17s\n" ) );
            ASSERT_TRUE( Contains( text, "server gave HTTP response to HTTPS client" ) && Contains( text, "the end of its log:" ) );
            ASSERT_TRUE( Contains( text, "build_image: the BuildKit daemon (buildkitd) could not read the development image" ) );
            ASSERT_FALSE( Contains( text, "no report published" ) || Contains( text, "image helper-service" ) );
            ASSERT_TRUE( Contains( text, "Next: ci_job_details with run_id \"" + runId + "\" and job_id \"images\"" ) );

            text = CallOk( server, "ci_job_details", "{\"run_id\":" + Json( runId ) + ",\"job_id\":\"images\"}" );
            ASSERT_TRUE( Contains( text, "In the report: image widget-service failed, image helper-service ok" ) );
            ASSERT_TRUE( Contains( text, "image helper-service: ok, tag sha-" + commit.SubstrToIndex( 12, true ) + ", digest " + okDigest ) );
            ASSERT_TRUE( Contains( text, "== image widget-service (" ) && Contains( text, "server gave HTTP response to HTTPS client" ) );
            const CORE::CString savedPath = After( After( text, "== image widget-service (" ), "full log: " ).ReplaceSubstr( ")", "" );
            CORE::CString saved;
            ASSERT_TRUE( savedPath.StartsWith( fixture.logDir ) && ReadFile( savedPath, saved ) && saved == fullLog );

            text = CallOk( server, "ci_images", "{\"run_id\":" + Json( runId ) + "}" );
            ASSERT_TRUE( Contains( text, "Images of run " + runId + ": failed" ) );
            ASSERT_TRUE( Contains( text, "- widget-service: failed, tag sha-" + commit.SubstrToIndex( 12, true ) + ", 17s\n" ) && Contains( text, "could not read the development image" ) );
            ASSERT_TRUE( Contains( text, "- helper-service: ok, tag sha-" + commit.SubstrToIndex( 12, true ) + ", digest " + okDigest + ", 31s" ) );
            ASSERT_TRUE( Contains( text, "job_id \"images\" gives the full log" ) );

            // only the plain log: it is fetched instead of the gzip one
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha two\n" ) );
            forgejo.ScriptNextRun( FailedImagesRun() );
            text = CallOk( server, "ci_submit", submit );
            const CORE::CString plainRunId = After( text, "Run id: " );
            const CORE::CString plainRun = CORE::Int64ToString( forgejo.GetLastRunId() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", plainRun + "-images", "images.json", FailedImagesJson( plainRun, commit, okDigest ), CORE::CString::Empty );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", plainRun + "-images", "widget-service.log", fullLog + "plain\n", CORE::CString::Empty );
            text = CallOk( server, "ci_job_details", "{\"run_id\":" + Json( plainRunId ) + ",\"job_id\":\"images\",\"product\":\"widget-service\"}" );
            const CORE::CString plainPath = After( After( text, "== image widget-service (" ), "full log: " ).ReplaceSubstr( ")", "" );
            ASSERT_TRUE( ReadFile( plainPath, saved ) && saved == fullLog + "plain\n" );

            // an images job that failed without images.json
            ASSERT_TRUE( WriteFile( CORE::CombinePath( fixture.repo, "a.txt" ), "alpha three\n" ) );
            forgejo.ScriptNextRun( FailedImagesRun() );
            text = CallOk( server, "ci_submit", submit );
            const CORE::CString bareRunId = After( text, "Run id: " );
            text = CallOk( server, "ci_report", "{\"run_id\":" + Json( bareRunId ) + "}" );
            ASSERT_TRUE( Contains( text, "- images: the image job published no report (images.json)" ) );
            text = CallOk( server, "ci_job_details", "{\"run_id\":" + Json( bareRunId ) + ",\"job_id\":\"images\"}" );
            ASSERT_TRUE( Contains( text, "The image job published no report" ) );
            text = CallError( server, "ci_images", "{\"run_id\":" + Json( bareRunId ) + "}" );
            ASSERT_TRUE( Contains( text, "published no image report" ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: ci.json: an entry without a deploy block loads and deploys nothing; a broken deploy block refuses only its entry, with a log line; the example of the lablink hub loads with its deploy block" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpCi is not next to the test application" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployConfig" );
            const CORE::CString nowhere = CORE::CombinePath( fixture.dir, "no-such-kubectl" );
            const CORE::CString noKubeconfig = CORE::CombinePath( fixture.dir, "no-such-kubeconfig" );
            const CORE::CString old = "{\"id\":\"OLD\",\"backend\":\"forgejo\",\"url\":\"http://127.0.0.1:9\",\"owner\":\"acme\",\"repo\":\"old\",\"scratchOwner\":\"ci\","
                                      "\"scratchRepo\":\"old\",\"workflow\":\"gucef-ci.yml\",\"remotes\":[\"https://example.invalid/acme/old.git\"]}";
            const CORE::CString broken = "{\"id\":\"BROKEN\",\"backend\":\"forgejo\",\"url\":\"http://127.0.0.1:9\",\"owner\":\"acme\",\"repo\":\"broken\",\"scratchOwner\":\"ci\","
                                         "\"scratchRepo\":\"broken\",\"workflow\":\"gucef-ci.yml\",\"remotes\":[\"https://example.invalid/acme/broken.git\"],"
                                         "\"deploy\":{\"images\":[{\"name\":\"x\",\"targets\":{\"live\":{\"deployment\":\"x\",\"container\":\"x\"}}}]}}";
            ASSERT_TRUE( fixture.WriteConfig( "http://127.0.0.1:9", "127.0.0.1:1", "," + old + "," + broken ) );
            CORE::CProcess::TStringVector args = fixture.ServerArgs( true );
            args.push_back( "--kubectl" ); args.push_back( nowhere );
            args.push_back( "--kubeconfig" ); args.push_back( noKubeconfig );
            CTestCiServer server;
            ASSERT_TRUE( server.Start( args ) );
            ASSERT_TRUE( Contains( server.Instructions(), "CI ids: WIDGET, OLD." ) );
            CORE::CString text = CallOk( server, "ci_resolve_repo", "{\"remotes\":[\"https://example.invalid/acme/old.git\"]}" );
            ASSERT_TRUE( Contains( text, "CI id: OLD" ) );
            text = CallError( server, "ci_release_status", "{\"ci_id\":\"OLD\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "has no deploy block" ) );
            text = CallError( server, "ci_deploy", "{\"ci_id\":\"OLD\",\"image\":\"x\",\"target\":\"live\",\"run_id\":\"OLD:1\",\"confirm\":true}" );
            ASSERT_TRUE( Contains( text, "Refused (not_listed)" ) && Contains( text, "has no deploy block" ) );
            text = CallError( server, "ci_describe", "{\"ci_id\":\"BROKEN\"}" );
            ASSERT_TRUE( Contains( text, "Unknown CI id" ) );
            // no kubeconfig: the state cannot be read, and nothing is started
            text = CallOk( server, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "its state could not be read" ) && Contains( text, "no kubeconfig at " + noKubeconfig ) );
            ASSERT_TRUE( StopServer( server ) );
            ASSERT_TRUE( Contains( server.GetTranscript(), "The CI registry entry \"BROKEN\" is refused" ) && Contains( server.GetTranscript(), "has no namespace" ) );

            // the example of the hub, with its deploy block
            const char* candidates[] = { "$MODULEDIR$/../../../../projects/k3s/lablink/ci.json.example", "$MODULEDIR$/../../../../../projects/k3s/lablink/ci.json.example" };
            CORE::CString example;
            for ( int i=0; i<2 && example.IsNULLOrEmpty(); ++i )
                ReadFile( CORE::RelativePath( candidates[ i ] ), example );
            GUCEF_TESTFW_SKIP_IF( example.IsNULLOrEmpty(), "projects/k3s/lablink/ci.json.example is not found from the test application" );
            const CORE::CString examplePath = CORE::CombinePath( fixture.dir, "example.json" );
            ASSERT_TRUE( WriteFile( examplePath, example ) );
            CORE::CProcess::TStringVector exampleArgs = args;
            for ( size_t i=0; i+1<exampleArgs.size(); ++i )
                if ( exampleArgs[ i ] == "--ci-config" )
                    exampleArgs[ i+1 ] = examplePath;
            CTestCiServer exampleServer;
            ASSERT_TRUE( exampleServer.Start( exampleArgs ) );
            text = CallOk( exampleServer, "ci_release_status", "{\"ci_id\":\"GUCEF\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "- image asset-store-service: Deployment assetstore/assetstore, container service (published commits only)" ) );
            ASSERT_TRUE( StopServer( exampleServer ) );
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

    GUCEF_TESTFW_TESTCASE( "Test: Proxy to gateway: submit of a published commit, images, release status and deploy through the gateway; scopes from the tokens file (none is build, unknown ignored), 403 scope, one deploy per target at a time (409), a revoked token" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasGit || !hasShell, "needs tools/gucefMcpCi, git and /bin/sh" );
            CDeployFixture fixture( "gucefMcpCi_TestApp_DeployGateway" );
            ASSERT_TRUE( fixture.MakeRepos() && fixture.WriteKubectl() );
            CFakeForgejo forgejo( ForgejoToken, "ci", "widget" );
            forgejo.SetBareRepo( fixture.bare );
            ASSERT_TRUE( forgejo.Start() );
            CFakeRegistry registry( CFakeRegistry::MODE_BASIC, "ci-bot", ForgejoToken );
            ASSERT_TRUE( registry.Start() );
            ASSERT_TRUE( fixture.WriteConfig( forgejo.GetUrl(), registry.GetHost() ) );

            const CORE::CString tokensFile = CORE::CombinePath( fixture.dir, "gateway-tokens" );
            const CORE::CString deployerTokenFile = CORE::CombinePath( fixture.dir, "deployer.token" );
            const CORE::CString tokens = "# machines of the lab\n" + Sha256Hex( DeployerToken, CORE::CString( DeployerToken ).Length() ) + "\ttestbox\tbuild,deploy\n" +
                                         TokenLine( ViewerToken, "viewer" ) + Sha256Hex( OddToken, CORE::CString( OddToken ).Length() ) + "\toddbox\tbuild,launch\n";
            ASSERT_TRUE( WriteOwnerOnlyFile( tokensFile, tokens ) && WriteOwnerOnlyFile( deployerTokenFile, CORE::CString( DeployerToken ) + "\n" ) );
            CORE::CProcess::TStringVector gatewayArgs;
            gatewayArgs.push_back( "--mode" ); gatewayArgs.push_back( "gateway" );
            gatewayArgs.push_back( "--ci-config" ); gatewayArgs.push_back( fixture.config );
            gatewayArgs.push_back( "--forgejo-token-file" ); gatewayArgs.push_back( fixture.tokenFile );
            gatewayArgs.push_back( "--gateway-tokens-file" ); gatewayArgs.push_back( tokensFile );
            gatewayArgs.push_back( "--lablink-ci-dir" ); gatewayArgs.push_back( "none" );
            gatewayArgs.push_back( "--log-dir" ); gatewayArgs.push_back( CORE::CombinePath( fixture.dir, "gateway-logs" ) );
            gatewayArgs.push_back( "--work-dir" ); gatewayArgs.push_back( fixture.workDir );
            gatewayArgs.push_back( "--poll-interval-ms" ); gatewayArgs.push_back( "200" );
            gatewayArgs.push_back( "--max-wait-seconds" ); gatewayArgs.push_back( "30" );
            gatewayArgs.push_back( "--kubectl" ); gatewayArgs.push_back( fixture.kubectl );
            gatewayArgs.push_back( "--kubeconfig" ); gatewayArgs.push_back( fixture.kubeconfig );
            gatewayArgs.push_back( "--deploy-timeout-seconds" ); gatewayArgs.push_back( "20" );
            CTestCiGateway gateway;
            ASSERT_TRUE( gateway.Start( gatewayArgs, CORE::CombinePath( fixture.dir, "gateway.log" ) ) );
            const CORE::CString url = gateway.GetUrl();

            CORE::CProcess::TStringVector proxyArgs;
            proxyArgs.push_back( "--mode" ); proxyArgs.push_back( "proxy" );
            proxyArgs.push_back( "--gateway-url" ); proxyArgs.push_back( url );
            proxyArgs.push_back( "--gateway-token-file" ); proxyArgs.push_back( deployerTokenFile );
            proxyArgs.push_back( "--lablink-ci-dir" ); proxyArgs.push_back( "none" );
            proxyArgs.push_back( "--log-dir" ); proxyArgs.push_back( fixture.logDir );
            proxyArgs.push_back( "--machine-name" ); proxyArgs.push_back( "TestBox" );
            proxyArgs.push_back( "--max-wait-seconds" ); proxyArgs.push_back( "30" );
            CTestCiServer proxy;
            ASSERT_TRUE( proxy.Start( proxyArgs ) );

            // the gateway sends the published commit from its own clone; the proxy sends nothing
            const CORE::CString commit = fixture.Commit( "b.txt", "bravo\n", "main" );
            const CORE::CString digest = DigestOf( "gateway release" );
            forgejo.ScriptNextRun( GreenRun() );
            CORE::CString text = CallOk( proxy, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( commit ) + ",\"images\":[\"widget-service\"]}" );
            const CORE::CString runId = After( text, "Run id: " );
            ASSERT_TRUE( runId == "WIDGET:" + CORE::Int64ToString( forgejo.GetLastRunId() ) );
            ASSERT_TRUE( GitOut( fixture.bare, "rev-parse", ( "refs/heads/scratch/release-" + commit.SubstrToIndex( 12, true ) ).C_String() ) == commit );
            ASSERT_TRUE( Contains( forgejo.GetLastDispatchBody(), "widget-service" ) );
            ASSERT_TRUE( GitOut( CORE::CombinePath( fixture.workDir, "repos/widget.git" ), "rev-parse", "refs/ci/published-head" ) == commit );
            const CORE::CString run = CORE::Int64ToString( forgejo.GetLastRunId() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", run + "-images", "images.json", ImagesJson( run, commit, false, "ok", digest ), CORE::CString::Empty );
            registry.AddManifest( Repository, "sha-" + commit.SubstrToIndex( 12, true ), digest );

            text = CallOk( proxy, "ci_images", "{\"run_id\":" + Json( runId ) + "}" );
            ASSERT_TRUE( Contains( text, "digest " + digest ) );
            text = CallOk( proxy, "ci_release_status", "{\"ci_id\":\"WIDGET\",\"target\":\"live\"}" );
            ASSERT_TRUE( Contains( text, "Deployment widgets/widget" ) );
            text = CallError( proxy, "ci_deploy", DeployArguments( ImageName, "live", runId, "false" ) );
            ASSERT_TRUE( Contains( text, "Refused (confirm)" ) );
            text = CallOk( proxy, "ci_deploy", DeployArguments( ImageName, "live", runId, "true" ), 180000 );
            const CORE::CString reference = registry.GetHost() + "/ci/widget-service@" + digest;
            ASSERT_TRUE( Contains( text, "Deployed: widgets/widget runs " + reference ) );
            ASSERT_TRUE( Contains( fixture.State( "widgets", "cause" ), "by testbox at " ) );
            ASSERT_TRUE( 0 == registry.GetTokenRequestCount() && registry.GetUnauthorizedCount() > 0 );

            // a failed image build, described through the gateway with its log kept on this machine
            const CORE::CString failedCommit = fixture.Commit( "f.txt", "foxtrot\n", "main" );
            forgejo.ScriptNextRun( FailedImagesRun() );
            text = CallOk( proxy, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( failedCommit ) + ",\"images\":[\"all\"]}" );
            const CORE::CString failedRunId = After( text, "Run id: " );
            const CORE::CString failedRun = CORE::Int64ToString( forgejo.GetLastRunId() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", failedRun + "-images", "images.json", FailedImagesJson( failedRun, failedCommit, DigestOf( "helper" ) ), CORE::CString::Empty );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", failedRun + "-images", "widget-service.log", FailedImageLog, CORE::CString::Empty );
            text = CallOk( proxy, "ci_report", "{\"run_id\":" + Json( failedRunId ) + "}" );
            ASSERT_TRUE( Contains( text, "    image widget-service: failed, tag sha-" + failedCommit.SubstrToIndex( 12, true ) ) && Contains( text, "server gave HTTP response to HTTPS client" ) );
            text = CallOk( proxy, "ci_job_details", "{\"run_id\":" + Json( failedRunId ) + ",\"job_id\":\"images\"}" );
            ASSERT_TRUE( Contains( text, "image helper-service: ok" ) && Contains( text, "== image widget-service (" ) );
            ASSERT_TRUE( After( After( text, "== image widget-service (" ), "full log: " ).StartsWith( fixture.logDir ) );
            text = CallOk( proxy, "ci_images", "{\"run_id\":" + Json( failedRunId ) + "}" );
            ASSERT_TRUE( Contains( text, "- widget-service: failed," ) && Contains( text, "could not read the development image" ) );

            // the scopes of the tokens file: without the column build, an unknown one ignored
            CORE::CString answer;
            const CORE::CString body = "{\"ci\":\"WIDGET\",\"image\":\"widget-service\",\"target\":\"live\",\"run\":" + Json( runId ) + ",\"confirm\":true}";
            ASSERT_TRUE( 403 == GatewayRequest( url, ViewerToken, "POST", "/v1/deploy", body, answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "scope" && Contains( JsonValue( answer, "error" ), "viewer" ) );
            ASSERT_TRUE( 403 == GatewayRequest( url, OddToken, "POST", "/v1/rollback", "{\"ci\":\"WIDGET\",\"target\":\"live\",\"confirm\":true}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "scope" );
            ASSERT_TRUE( 200 == GatewayRequest( url, ViewerToken, "GET", "/v1/release?ci=WIDGET&target=live", CORE::CString::Empty, answer ) );
            ASSERT_TRUE( Contains( answer, digest ) );
            ASSERT_TRUE( 200 == GatewayRequest( url, ViewerToken, "GET", "/v1/images?id=" + runId, CORE::CString::Empty, answer ) );
            ASSERT_TRUE( 400 == GatewayRequest( url, DeployerToken, "POST", "/v1/deploy",
                                                "{\"ci\":\"WIDGET\",\"image\":\"widget-service\",\"target\":\"live\",\"run\":" + Json( runId ) + ",\"confirm\":\"true\"}", answer ) );
            ASSERT_TRUE( JsonValue( answer, "code" ) == "confirm" );
            ASSERT_TRUE( 404 == GatewayRequest( url, DeployerToken, "GET", "/v1/release?ci=WIDGET&target=prod", CORE::CString::Empty, answer ) );

            // one deployment per target at a time
            const CORE::CString second = fixture.Commit( "c.txt", "charlie\n", "main" );
            const CORE::CString secondDigest = DigestOf( "gateway release two" );
            forgejo.ScriptNextRun( GreenRun() );
            text = CallOk( proxy, "ci_submit", "{\"ci_id\":\"WIDGET\",\"commit\":" + Json( second ) + ",\"images\":[\"widget-service\"]}" );
            const CORE::CString secondRun = After( text, "Run id: " );
            const CORE::CString secondBackend = CORE::Int64ToString( forgejo.GetLastRunId() );
            forgejo.AddPackageFile( "ci-bot", "ci-reports", secondBackend + "-images", "images.json", ImagesJson( secondBackend, second, false, "ok", secondDigest ), CORE::CString::Empty );
            registry.AddManifest( Repository, "sha-" + second.SubstrToIndex( 12, true ), secondDigest );
            fixture.SetState( "widgets", "rollout_sleep", "4" );
            const CORE::CString secondBody = "{\"ci\":\"WIDGET\",\"image\":\"widget-service\",\"target\":\"live\",\"run\":" + Json( secondRun ) + ",\"confirm\":true}";
            WEB::CHTTPClient slowClient;
            slowClient.SetThreadPoolName( "gucefMcpCi_TestApp.deploy" );
            CORE::CFutureResult slow = GatewayRequestAsync( slowClient, url, DeployerToken, "POST", "/v1/deploy", secondBody, 60000 );
            // the first is in its rollout of four seconds after its checks, the second is refused at once
            MT::ThreadDelay( 2000 );
            const UInt64 askedAt = MT::PrecisionTickCount();
            Int32 status = GatewayRequest( url, DeployerToken, "POST", "/v1/deploy", secondBody, answer );
            if ( 409 != status )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi_TestApp: The second deployment answered " << status << ": " << answer );
            ASSERT_TRUE( 409 == status && JsonValue( answer, "code" ) == "busy" );
            ASSERT_TRUE( ( MT::PrecisionTickCount() - askedAt ) * 1000 / MT::PrecisionTimerResolution() < 2000 );
            ASSERT_TRUE( 200 == GatewayResult( slow, 60000 ) );
            ASSERT_TRUE( fixture.State( "widgets", "image" ) == registry.GetHost() + "/ci/widget-service@" + secondDigest );
            fixture.SetState( "widgets", "rollout_sleep", CORE::CString::Empty );

            // rollback through the proxy, then the deployer is revoked
            text = CallOk( proxy, "ci_rollback", "{\"ci_id\":\"WIDGET\",\"target\":\"live\",\"confirm\":true}", 180000 );
            ASSERT_TRUE( Contains( text, "Rolled back" ) && fixture.State( "widgets", "image" ) == reference );
            ASSERT_TRUE( WriteOwnerOnlyFile( tokensFile, TokenLine( ViewerToken, "viewer" ) ) );
            status = 200;
            for ( int i=0; i<100 && 401 != status; ++i )
            {
                MT::ThreadDelay( 100 );
                status = GatewayRequest( url, DeployerToken, "GET", "/v1/release?ci=WIDGET&target=live", CORE::CString::Empty, answer );
            }
            ASSERT_TRUE( 401 == status );
            text = CallError( proxy, "ci_deploy", DeployArguments( ImageName, "live", runId, "true" ) );
            ASSERT_TRUE( Contains( text, "refused the token of this machine" ) );

            ASSERT_TRUE( KubectlCallsAllowed( fixture ) );
            Keep( fixture );
            ASSERT_TRUE( StopServer( proxy ) );
            ASSERT_TRUE( StopGateway( gateway ) );
            const CORE::CString log = gateway.GetLog();
            ASSERT_TRUE( Contains( log, "unknown scope(s) launch of machine oddbox" ) && Contains( log, "1 of them may deploy" ) );
            bool allJson = false;
            ASSERT_TRUE( CountAudit( fixture, "\"testbox\"", allJson ) >= 3 && CountAudit( fixture, "\"viewer\"", allJson ) >= 1 && CountAudit( fixture, "busy", allJson ) >= 1 );
            ASSERT_TRUE( allJson );
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

    GUCEF_TESTFW_TESTCASE( "Test: No CI token, gateway token, registry bearer or kubeconfig content of the deploy tests appears in any answer, log or audit" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || g_transcripts.IsNULLOrEmpty(), "no server of the deploy tests ran" );
            const char* secrets[] = { ForgejoToken, DeployerToken, ViewerToken, OddToken, KubeSecret, "issued-bearer-4c7e1a9d", "1a3c5e7a9b2d4f6a8c0e", "9f8e7d6c5b4a" };
            for ( int i=0; i<8; ++i )
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
