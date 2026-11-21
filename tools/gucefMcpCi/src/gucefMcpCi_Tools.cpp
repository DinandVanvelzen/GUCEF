/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpLocalLlm_BuildLogDigest.h"
#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiDeployImage.h"
#include "gucefMcpCi_Tools.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 MaxOutputChars = 16000;
static const UInt32 MaxReportChars = 12000;
static const UInt32 MaxListedJobs = 30;
static const UInt32 StatusChunkSeconds = 15;
static const UInt32 DefaultTailLines = 200;
static const UInt32 MaxTailLines = 2000;
static const UInt32 MaxFirstErrors = 3;
static const UInt32 MaxCaseChars = 240;
static const UInt32 MaxCasesInReport = 10;
static const UInt32 MaxCasesInDetails = 50;
static const UInt32 TestTailLines = 30;
static const UInt32 ImageTailLines = 12;
static const UInt32 MaxImageTailChars = 1500;
static const UInt32 DefaultRetentionDays = 14;
static const UInt32 ProgressIntervalInMs = 2000;

static const char* ResolveDescription =
    "Tells whether the lab CI can build a repository and gives its CI id. Pass repo_dir (its git remotes are used) or "
    "remotes. Only repositories the CI already knows can be built; for any other the answer is \"no CI is set up\". "
    "Call this first when you do not know the CI id.";

static const char* DescribeDescription =
    "Lists what a CI id can build: toolchains, C++ standards, products, modes, and how many jobs \"everything\" is. "
    "Pass repo_dir to read the options from the CI files of that working tree. You rarely need this: ci_submit builds "
    "everything by default; use it only when you must narrow a run down.";

static const char* SubmitDescription =
    "Builds a local repository on the lab CI: all toolchains, C++ standards and products (the full matrix) by default, "
    "with mode \"test\" also the tests. Sends everything not yet published (unpushed commits, modified, deleted and "
    "untracked files; .gitignore applies) as a scratch snapshot; your working tree, index and branches are not touched. "
    "Use instead of building variants yourself. Submit ONCE per change, then poll ci_status with wait_seconds; do not "
    "submit again while a run is going, and do not manage variants yourself. Filters are optional and only for a "
    "deliberate narrow run. dry_run shows what would be sent without sending it. images also builds container images "
    "(names, or [\"all\"]); only ask for them when an image is needed. commit builds a commit of the published repository "
    "instead of the working tree (nothing local is sent, repo_dir is not needed): use it with images to build what may be "
    "deployed to a target that takes published commits only. Returns the run id.";

static const char* StatusDescription =
    "The state of a CI run: queued, running, succeeded, failed or cancelled, with job counts and the failed jobs. "
    "Cheap. Pass wait_seconds (up to the maximum) to wait inside the call until the run is finished: poll with a long "
    "wait instead of many short calls. When it is finished, read ci_report.";

static const char* ReportDescription =
    "The outcome of a finished CI run: per job which products and tests succeeded or failed, the first errors with "
    "file:line and the failed test cases. Read this once the run is finished, then ask ci_job_details only for the "
    "failed jobs you need more of.";

static const char* JobDetailsDescription =
    "Details of ONE job of a run, for failed jobs only: part \"digest\" (default) gives the distinct errors with "
    "file:line, failed targets and failed tests of its failed products; \"log_tail\" the end of a log (limit lines, "
    "offset lines skipped from the end); \"tests\" the failed test cases. Name product to see another product or test "
    "application. The full logs are saved on this machine; the result gives their absolute paths to grep.";

static const char* CancelDescription =
    "Stops a CI run, for example one started on the wrong change. Not needed for a run that is finished.";

static const char* ImagesDescription =
    "The container images a CI run built: per image its tag, digest and whether it was built and pushed, and whether the "
    "run built a published commit or a scratch snapshot. Read-only. Use it to see what ci_deploy could roll out.";

static const char* DeployDescription =
    "Rolls an image that a green CI run built out to a target, a Kubernetes Deployment listed for the CI id. DESTRUCTIVE: "
    "it changes what runs. Only call it when the user explicitly asked for this deployment, never on your own initiative "
    "and never to try something out. First show the user what will change: ci_release_status for what runs now and "
    "ci_images for what the run built; then call it once with confirm=true. Targets that take published commits only "
    "(such as live) refuse scratch snapshots: build a published commit with ci_submit commit and images first. The run "
    "must be fully green. When the rollout fails or times out nothing is rolled back: report what the result says and "
    "let the user decide; ci_rollback restores the previous revision.";

static const char* ReleaseStatusDescription =
    "What a deployment target of a CI id runs now: per image its Deployment, the image and digest, who deployed which "
    "commit and run (the change cause), the replicas and the revision history. Read-only. Call it before ci_deploy or "
    "ci_rollback to show the user what will change, and afterwards to show the result.";

static const char* RollbackDescription =
    "Returns the Deployment of a target to its previous revision: the image it ran before the last deployment. "
    "DESTRUCTIVE. Only call it when the user explicitly asked for a rollback; show ci_release_status first, then call it "
    "once with confirm=true. Another call rolls back again (to the revision before that), so never repeat it to be sure.";

static const char* CleanupDescription =
    "Removes the scratch branches and reports of a CI id that are older than older_than_days (default 14). Submitting "
    "already does this now and then; call it only when asked to.";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CDataNode
MakeObjectSchema( void )
{
    CORE::CDataNode schema( GUCEF_DATATYPE_OBJECT );
    schema.SetAttribute( "type", "object" );
    schema.SetAttribute( "additionalProperties", false );
    return schema;
}

/*-------------------------------------------------------------------------*/

static CORE::CDataNode*
AddProperty( CORE::CDataNode& schema, const char* name, const char* type, const char* description, bool required )
{
    CORE::CDataNode* properties = schema.FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        properties = schema.AddChild( "properties", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* property = properties->AddChild( name, GUCEF_DATATYPE_OBJECT );
    property->SetAttribute( "type", type );
    property->SetAttribute( "description", description );
    if ( required )
    {
        CORE::CDataNode* requiredList = schema.FindChild( "required" );
        if ( GUCEF_NULL == requiredList )
            requiredList = schema.AddChild( "required", GUCEF_DATATYPE_ARRAY );
        requiredList->AddValueAsChild( CORE::CVariant( name ) );
    }
    return property;
}

/*-------------------------------------------------------------------------*/

static void
AddStringArrayProperty( CORE::CDataNode& schema, const char* name, const char* description )
{
    CORE::CDataNode* property = AddProperty( schema, name, "array", description, false );
    CORE::CDataNode* items = property->AddChild( "items", GUCEF_DATATYPE_OBJECT );
    items->SetAttribute( "type", "string" );
    items->SetAttribute( "minLength", (Int32) 1 );
    property->SetAttribute( "maxItems", (Int32) 64 );
}

/*-------------------------------------------------------------------------*/

static void
AddEnumProperty( CORE::CDataNode& schema, const char* name, const char* description, const char* a, const char* b, const char* c = GUCEF_NULL )
{
    CORE::CDataNode* property = AddProperty( schema, name, "string", description, false );
    CORE::CDataNode* values = property->AddChild( "enum", GUCEF_DATATYPE_ARRAY );
    values->AddValueAsChild( CORE::CVariant( a ) );
    values->AddValueAsChild( CORE::CVariant( b ) );
    if ( GUCEF_NULL != c )
        values->AddValueAsChild( CORE::CVariant( c ) );
}

/*-------------------------------------------------------------------------*/

static void
AddIntegerProperty( CORE::CDataNode& schema, const char* name, const char* description, Int32 minimum, Int32 maximum )
{
    CORE::CDataNode* property = AddProperty( schema, name, "integer", description, false );
    property->SetAttribute( "minimum", minimum );
    property->SetAttribute( "maximum", maximum );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Argument( const CORE::CDataNode& arguments, const char* name )
{
    return CCiUtils::Trimmed( arguments.GetAttributeValue( name ).AsString() );
}

/*-------------------------------------------------------------------------*/

/** A run id in a sentence: "GUCEF:12" */
static CORE::CString
Quote( const CORE::CString& text )
{
    return "\"" + text + "\"";
}

/*-------------------------------------------------------------------------*/

/** The first distinct errors a log tail holds, as lines of text with an indent */
static CORE::CString
FirstErrors( const CORE::CString& tail, const CORE::CString& workspace, const CORE::CString& indent )
{
    if ( tail.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    GUCEFMcpLocalLlm::CBuildLogDigest digest( workspace );
    digest.Parse( tail.STL_String() );
    CORE::CString text;
    const GUCEFMcpLocalLlm::CBuildLogDigest::TIssueVector& errors = digest.GetErrors();
    for ( size_t i=0; i<errors.size() && i<MaxFirstErrors; ++i )
    {
        const CORE::CString location( errors[ i ].location );
        text += indent + ( location.IsNULLOrEmpty() ? CORE::CString::Empty : location + ": " ) + CORE::CString( errors[ i ].message ) + "\n";
    }
    if ( errors.size() > MaxFirstErrors )
        text += indent + "(" + CORE::UInt32ToString( (UInt32) ( errors.size() - MaxFirstErrors ) ) + " more distinct errors)\n";
    return text;
}

/*-------------------------------------------------------------------------*/

/** The counts of a test application, when the report has them: ", 1 of 40 failed" */
static CORE::CString
TestCounts( const CCiReport::CTest& test )
{
    if ( test.testCount < 0 || test.failureCount < 0 )
        return CORE::CString::Empty;
    return ", " + CORE::Int32ToString( test.failureCount ) + " of " + CORE::Int32ToString( test.testCount ) + " failed";
}

/*-------------------------------------------------------------------------*/

/**
 *  A failed test application: its status, its failed cases as "suite: name - message" lines, and when the report
 *  names none, the errors or the end of its output. Test applications have no log file: this is all the report has.
 */
static CORE::CString
DescribeFailedTest( const CCiReport::CTest& test, const CORE::CString& workspace, const CORE::CString& indent, UInt32 maxCases, bool withTail )
{
    CORE::CString text = indent + "test " + test.name + ": " + CCiReport::DescribeStatus( test.status ) + " (exit " + CORE::Int32ToString( test.exitCode ) +
                         TestCounts( test ) + ")\n";
    for ( size_t f=0; f<test.failedCases.size() && f<maxCases; ++f )
        text += indent + "  " + test.failedCases[ f ].Describe( MaxCaseChars ) + "\n";
    if ( test.failedCases.size() > maxCases )
        text += indent + "  (" + CORE::UInt32ToString( (UInt32) test.failedCases.size() - maxCases ) + " more failed cases)\n";
    if ( test.failedCases.empty() || withTail )
    {
        const CORE::CString errors = FirstErrors( test.logTail, workspace, indent + "  " );
        if ( !errors.IsNULLOrEmpty() )
            text += errors;
        else
        if ( !test.logTail.IsNULLOrEmpty() && withTail )
        {
            UInt32 lines = 0;
            text += indent + "  the end of its output:\n" + CCiUtils::TailLines( test.logTail, TestTailLines, 0, lines );
        }
        else
        if ( test.failedCases.empty() && test.logTail.IsNULLOrEmpty() )
            text += indent + "  the report names no failed cases; its output is in the log of the product\n";
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/**
 *  An image build as one line with its status word as the report gives it, its tag, digest and time; for an image that
 *  was not built and pushed the errors the digest finds in the end of its log, and always the end of that log, bounded:
 *  the errors of image builds seldom have the file:line form
 */
static CORE::CString
DescribeImageBuild( const CORE::CString& name, const CORE::CString& status, const CORE::CString& tag, const CORE::CString::StringVector& alsoTags,
                    const CORE::CString& digest, UInt64 seconds, const CORE::CString& logTail, const CORE::CString& workspace, const CORE::CString& indent )
{
    const bool ok = status == "ok" && CCiImageReport::IsDigest( digest );
    CORE::CString text = indent + name + ": " + ( status.IsNULLOrEmpty() ? CORE::CString( "no status" ) : status ) +
                         ( status == "ok" && !ok ? CORE::CString( " but no digest, so not pushed" ) : CORE::CString::Empty ) +
                         ( tag.IsNULLOrEmpty() ? CORE::CString::Empty : ", tag " + tag ) +
                         ( alsoTags.empty() ? CORE::CString::Empty : " (also " + CCiUtils::Join( alsoTags, ", " ) + ")" ) +
                         ( digest.IsNULLOrEmpty() ? CORE::CString::Empty : ", digest " + digest ) +
                         ( seconds > 0 ? ", " + CCiUtils::FormatDuration( seconds ) : CORE::CString::Empty ) + "\n";
    if ( ok )
        return text;
    if ( logTail.IsNULLOrEmpty() )
        return text + indent + "  the report has no end of its log\n";
    text += FirstErrors( logTail, workspace, indent + "  " );
    UInt32 lines = 0;
    text += indent + "  the end of its log:\n" + CCiUtils::Bound( CCiUtils::TailLines( logTail, ImageTailLines, 0, lines ), MaxImageTailChars );
    if ( !text.EndsWith( "\n" ) )
        text += "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

/** Counts and lists of a run as a few lines */
static CORE::CString
DescribeStatus( const CCiRunStatus& status )
{
    CORE::CString text = "Run " + status.runId + ": " + CCiRunStatus::StateToString( status.state ) + ", elapsed " + CCiUtils::FormatDuration( status.elapsedSecs ) + ".";
    if ( status.jobs.empty() )
    {
        text += " No jobs yet.\n";
        return text;
    }
    const UInt32 succeeded = status.CountJobs( CCiRunStatus::STATE_SUCCEEDED );
    const UInt32 failed = status.CountJobs( CCiRunStatus::STATE_FAILED );
    const UInt32 running = status.CountJobs( CCiRunStatus::STATE_RUNNING );
    const UInt32 queued = status.CountJobs( CCiRunStatus::STATE_QUEUED );
    const UInt32 cancelled = status.CountJobs( CCiRunStatus::STATE_CANCELLED );
    text += " Jobs: " + CORE::UInt32ToString( (UInt32) status.jobs.size() ) + " (" + CORE::UInt32ToString( succeeded ) + " succeeded, " +
            CORE::UInt32ToString( failed ) + " failed, " + CORE::UInt32ToString( running ) + " running, " + CORE::UInt32ToString( queued ) + " queued" +
            ( cancelled > 0 ? ", " + CORE::UInt32ToString( cancelled ) + " cancelled" : CORE::CString::Empty ) + ").\n";
    if ( failed > 0 )
        text += "Failed: " + status.ListJobs( CCiRunStatus::STATE_FAILED, MaxListedJobs ) + "\n";
    if ( running > 0 )
        text += "Running: " + status.ListJobs( CCiRunStatus::STATE_RUNNING, MaxListedJobs ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

/** Sets the result, with the secrets taken out */
static void
SetText( COM::CMcpToolResult& result, const SToolSettings& settings, const CORE::CString& text, bool isError )
{
    const CORE::CString clean = settings.redactor.RedactText( text );
    if ( isError )
        result.SetError( clean );
    else
        result.AddText( clean );
}

/*-------------------------------------------------------------------------*/

/** The progress of a long submit as MCP progress notifications, at most one per interval, and its cancellation */
class CContextProgress : public ICiProgress
{
    public:

    explicit CContextProgress( COM::CMcpRequestContext& context )
        : ICiProgress()
        , m_context( context )
        , m_startTicks( MT::PrecisionTickCount() )
        , m_lastMs( 0 )
        , m_reported( false )
    {
    }

    virtual void OnProgress( Float64 done, Float64 total, const CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE
    {
        const UInt64 elapsed = CCiUtils::MilliSecsSince( m_startTicks );
        if ( m_reported && elapsed - m_lastMs < ProgressIntervalInMs )
            return;
        m_reported = true;
        m_lastMs = elapsed;
        m_context.ReportProgress( done, total, message + ", " + CCiUtils::FormatDuration( elapsed / 1000 ) );
    }

    virtual bool IsCancelled( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_context.IsCancellationRequested();
    }

    private:

    COM::CMcpRequestContext& m_context;
    UInt64 m_startTicks;
    UInt64 m_lastMs;
    bool m_reported;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE TOOLS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the tools share: the service and the settings, and the thread pool as the place to run */
class CCiTool : public COM::CIMcpTool
{
    public:

    CCiTool( ICiService& service, const SToolSettings& settings )
        : COM::CIMcpTool()
        , m_service( service )
        , m_settings( settings )
    {
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    protected:

    ICiService& m_service;
    SToolSettings m_settings;
};

/*-------------------------------------------------------------------------*/

class CResolveTool : public CCiTool
{
    public:

    CResolveTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_resolve_repo";
        descriptor.title = "CI id of a repository";
        descriptor.description = ResolveDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "repo_dir", "string", "Absolute path of a directory in the working tree of the repository", false );
        AddStringArrayProperty( schema, "remotes", "Remote URLs of the repository (https, ssh or scp form), when there is no local working tree" );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString repoDir = Argument( arguments, "repo_dir" );
        const CORE::CString::StringVector remotes = CCiUtils::GetStringArray( arguments, "remotes" );
        CORE::CString ciId;
        CORE::CString description;
        CORE::CString matched;
        CORE::CString::StringVector examined;
        CORE::CString error;
        if ( !m_service.Resolve( repoDir, remotes, ciId, description, matched, examined, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CString text = "CI id: " + ciId + ( description.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + description + ")" ) + ".\n" +
                             "Matched remote: " + matched + "\n" +
                             "Next: ci_submit with ci_id " + Quote( ciId ) + " and repo_dir builds everything; then ci_status with wait_seconds.";
        SetText( result, m_settings, text, false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CDescribeTool : public CCiTool
{
    public:

    CDescribeTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_describe";
        descriptor.title = "Options of a CI id";
        descriptor.description = DescribeDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddProperty( schema, "repo_dir", "string", "Absolute path of the working tree, to read the options from its CI files", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiDescription description;
        CORE::CString error;
        if ( !m_service.Describe( Argument( arguments, "ci_id" ), Argument( arguments, "repo_dir" ), description, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        CORE::CString text = "CI id " + description.ciId + ", options from " + description.source + ".\n";
        if ( !description.toolchains.empty() )
        {
            text += "Toolchains:";
            for ( size_t i=0; i<description.toolchains.size(); ++i )
                text += " " + description.toolchains[ i ].id + ( description.toolchains[ i ].enabled ? CORE::CString::Empty : CORE::CString( " (no runner, not built)" ) ) +
                        ( i+1 < description.toolchains.size() ? "," : "" );
            text += "\n";
        }
        if ( !description.cppStandards.empty() )
            text += "C++ standards: " + CCiUtils::Join( description.cppStandards, ", " ) + "\n";
        if ( !description.buildTypes.empty() )
            text += "Build types: " + CCiUtils::Join( description.buildTypes, ", " ) + "\n";
        if ( !description.products.empty() )
            text += "Products: " + CCiUtils::Join( description.products, ", " ) + "\n";
        text += "Modes: " + CCiUtils::Join( description.modes, ", " ) + " (test also runs the test applications)\n";
        text += "Everything (no filters): every enabled toolchain x every C++ standard x every product" +
                ( description.defaultCellCount > 0 ? " = " + CORE::UInt32ToString( description.defaultCellCount ) + " jobs" : CORE::CString::Empty ) + ".\n";
        text += "Filters of ci_submit: toolchains, cpp_standards, products; leave them out to build everything.";
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CSubmitTool : public CCiTool
{
    public:

    CSubmitTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_submit";
        descriptor.title = "Build a repository on the CI";
        descriptor.description = SubmitDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_ADDITIVE;
        descriptor.idempotent = false;
        descriptor.openWorld = true;
        descriptor.maxDurationMs = 45 * 60 * 1000;      // the first push of a large history
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddProperty( schema, "repo_dir", "string", "Absolute path of the working tree to build; not used with commit", false );
        AddStringArrayProperty( schema, "toolchains", "Only these toolchains; default: all enabled" );
        AddStringArrayProperty( schema, "cpp_standards", "Only these C++ standards (\"98\", \"17\"); default: all" );
        AddStringArrayProperty( schema, "products", "Only these products; default: all" );
        AddEnumProperty( schema, "mode", "build (default) or test (build and run the tests)", "build", "test" );
        AddProperty( schema, "include_uncommitted", "boolean", "Also send modified, deleted and untracked files (default true); false sends HEAD only", false );
        AddProperty( schema, "dry_run", "boolean", "Only show what would be sent (default false)", false );
        AddStringArrayProperty( schema, "images", "Also build these container images (their names), or [\"all\"]; default none" );
        AddProperty( schema, "commit", "string", "Build this commit (40 hexadecimal digits) of the published repository instead of the working tree", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiSubmitOptions options;
        options.toolchains = CCiUtils::GetStringArray( arguments, "toolchains" );
        options.cppStandards = CCiUtils::GetStringArray( arguments, "cpp_standards" );
        options.products = CCiUtils::GetStringArray( arguments, "products" );
        options.mode = Argument( arguments, "mode" ).Lowercase();
        options.includeUncommitted = arguments.GetAttributeValue( "include_uncommitted" ).AsBool( true );
        options.dryRun = arguments.GetAttributeValue( "dry_run" ).AsBool( false );
        options.images = CCiUtils::GetStringArray( arguments, "images" );
        options.commit = Argument( arguments, "commit" ).Lowercase();
        const CORE::CString ciId = Argument( arguments, "ci_id" );
        const CORE::CString repoDir = Argument( arguments, "repo_dir" );
        CORE::CString problem;
        if ( options.commit.IsNULLOrEmpty() && repoDir.IsNULLOrEmpty() )
            problem = "Give repo_dir (the working tree to build) or commit (a commit of the published repository)";
        for ( size_t i=0; i<options.images.size() && problem.IsNULLOrEmpty(); ++i )
        {
            if ( options.images[ i ] == "all" && options.images.size() > 1 )
                problem = "images is [\"all\"] or a list of image names, not both";
            else
            if ( options.images[ i ] != "all" && !CCiDeployImage::IsImageName( options.images[ i ] ) )
                problem = "\"" + options.images[ i ] + "\" is not an image name (lower case letters, digits, - _ . /)";
        }
        if ( !problem.IsNULLOrEmpty() )
        {
            SetText( result, m_settings, problem, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        CCiSnapshot snapshot;
        CORE::CString runId;
        CORE::CString error;
        CContextProgress progress( context );
        if ( !m_service.Submit( ciId, repoDir, options, snapshot, runId, error, &progress ) )
        {
            SetText( result, m_settings, CCiUtils::Bound( error, MaxOutputChars ), true );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        if ( !options.commit.IsNULLOrEmpty() )
        {
            CORE::CString text = "The published commit " + options.commit + ( options.dryRun ? CORE::CString( " is on a branch of the published repository; dry run: nothing was sent." )
                                                                                             : " was sent to the CI as " + snapshot.branchName + ".\nRun id: " + runId + "\n" ) +
                                 ( options.images.empty() ? CORE::CString::Empty : "Images: " + CCiUtils::Join( options.images, ", " ) + ".\n" );
            if ( !options.dryRun )
                text += "Next: ci_status with run_id " + Quote( runId ) + " and wait_seconds " + CORE::UInt32ToString( m_settings.maxWaitSeconds ) + ", repeated until it is finished" +
                        ( options.images.empty() ? CORE::CString( ", then ci_report." ) : CORE::CString( ", then ci_images." ) );
            SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        CORE::CString text = snapshot.Describe();
        if ( options.dryRun )
        {
            text += "Dry run: nothing was sent. Without dry_run this goes to the CI as " + snapshot.branchName.SubstrToIndex( 8, true ) + "... and builds " +
                    ( options.toolchains.empty() && options.cppStandards.empty() && options.products.empty() ? CORE::CString( "everything" ) : CORE::CString( "the filtered jobs" ) ) + ".";
        }
        else
        {
            text += "Run id: " + runId + "\n" +
                    "Next: ci_status with run_id " + Quote( runId ) + " and wait_seconds " + CORE::UInt32ToString( m_settings.maxWaitSeconds ) +
                    ", repeated until it is finished, then ci_report. Do not submit again for this change.";
        }
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CStatusTool : public CCiTool
{
    public:

    CStatusTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_status";
        descriptor.title = "State of a CI run";
        descriptor.description = StatusDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        descriptor.maxDurationMs = ( m_settings.maxWaitSeconds + 120 ) * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "run_id", "string", "The run id, as ci_submit gives it", true );
        AddIntegerProperty( schema, "wait_seconds", "Wait up to this long for the run to finish; 0 (default) answers at once", 0, (Int32) m_settings.maxWaitSeconds );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString runId = Argument( arguments, "run_id" );
        UInt32 waitSeconds = arguments.GetAttributeValue( "wait_seconds" ).AsUInt32( 0 );
        if ( waitSeconds > m_settings.maxWaitSeconds )
            waitSeconds = m_settings.maxWaitSeconds;

        // the wait in short steps, so that a cancelled request ends soon and progress is reported
        const UInt64 startTicks = MT::PrecisionTickCount();
        CCiRunStatus status;
        CORE::CString error;
        for ( ;; )
        {
            const UInt64 waited = CCiUtils::MilliSecsSince( startTicks ) / 1000;
            const UInt32 left = waited >= waitSeconds ? 0 : waitSeconds - (UInt32) waited;
            const UInt32 step = left < StatusChunkSeconds ? left : StatusChunkSeconds;
            if ( !m_service.Status( runId, step, status, error ) )
            {
                SetText( result, m_settings, error, true );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            if ( status.IsFinished() || 0 == left || context.IsCancellationRequested() )
                break;
            context.ReportProgress( (Float64) ( waited + step ), (Float64) waitSeconds, CORE::CString( "run " ) + CCiRunStatus::StateToString( status.state ) );
        }

        CORE::CString text = DescribeStatus( status );
        if ( status.IsFinished() )
        {
            if ( CCiRunStatus::STATE_SUCCEEDED == status.state )
                text += "Finished: everything succeeded. ci_report gives the summary per product if you need it.";
            else
                text += "Finished. Next: ci_report with run_id " + Quote( status.runId ) + ".";
        }
        else
        {
            text += "Not finished. Next: ci_status again with wait_seconds " + CORE::UInt32ToString( m_settings.maxWaitSeconds ) + ". Do not submit again meanwhile.";
        }
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CReportTool : public CCiTool
{
    public:

    CReportTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_report";
        descriptor.title = "Report of a CI run";
        descriptor.description = ReportDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "run_id", "string", "The run id, as ci_submit gives it", true );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiReport report;
        CORE::CString error;
        if ( !m_service.Report( Argument( arguments, "run_id" ), report, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        CORE::CString text = DescribeStatus( report.status );
        if ( !report.status.IsFinished() )
            text += "The run is not finished: this covers the jobs that are.\n";

        CORE::CString failedText;
        CORE::CString::StringVector succeeded;
        CORE::CString::StringVector pending;
        UInt32 builtProducts = 0;
        CORE::CString firstFailedJob;
        for ( size_t i=0; i<report.cells.size(); ++i )
        {
            const CCiReport::CCell& cell = report.cells[ i ];
            const bool failed = CCiRunStatus::STATE_FAILED == cell.state || ( cell.hasReport && cell.IsFailed() );
            if ( !failed )
            {
                if ( CCiRunStatus::STATE_SUCCEEDED == cell.state )
                {
                    succeeded.push_back( cell.jobId );
                    builtProducts += (UInt32) cell.products.size();
                }
                else
                    pending.push_back( cell.jobId + " (" + CCiRunStatus::StateToString( cell.state ) + ")" );
                continue;
            }
            if ( firstFailedJob.IsNULLOrEmpty() )
                firstFailedJob = cell.jobId;
            failedText += "- " + cell.jobId + ":";
            if ( !cell.hasReport )
            {
                failedText += " " + ( cell.problem.IsNULLOrEmpty() ? CORE::CString( "no report published (it failed before or outside the build)" ) : cell.problem ) + "\n";
                continue;
            }
            failedText += cell.mode.IsNULLOrEmpty() ? "\n" : " (" + cell.mode + ")\n";
            for ( size_t p=0; p<cell.products.size(); ++p )
            {
                const CCiReport::CProduct& product = cell.products[ p ];
                if ( !product.IsFailed() )
                    continue;
                if ( product.IsImage() )
                {
                    failedText += DescribeImageBuild( "image " + product.name, product.status, product.tag, CORE::CString::StringVector(), product.digest,
                                                      product.seconds, product.logTail, cell.workspace, "    " );
                    continue;
                }
                const bool inTests = product.phase == "test" || product.CountFailedTests() > 0;
                failedText += "    product " + product.name + ": " + ( inTests ? CORE::CString( "tests" ) : ( product.phase.IsNULLOrEmpty() ? CORE::CString( "build" ) : product.phase ) ) +
                              " failed (exit " + CORE::Int32ToString( product.exitCode ) + ")\n";
                if ( !inTests )
                    failedText += FirstErrors( product.logTail, cell.workspace, "      " );
                for ( size_t t=0; t<product.tests.size(); ++t )
                    if ( product.tests[ t ].IsFailed() )
                        failedText += DescribeFailedTest( product.tests[ t ], cell.workspace, "      ", MaxCasesInReport, false );
            }
            if ( 0 == cell.CountFailedProducts() )
                failedText += "    the job failed outside the " + ( cell.mode == "images" ? CORE::CString( "image builds" ) : CORE::CString( "products" ) ) +
                              " (the report says " + CCiReport::DescribeStatus( cell.status ) + ")\n";
        }

        if ( !failedText.IsNULLOrEmpty() )
            text += "Failed jobs:\n" + failedText;
        if ( !succeeded.empty() )
            text += "Succeeded jobs (" + CORE::UInt32ToString( (UInt32) succeeded.size() ) + ", " + CORE::UInt32ToString( builtProducts ) + " product builds): " +
                    CCiUtils::Join( succeeded, ", " ) + "\n";
        if ( !pending.empty() )
            text += "Not finished: " + CCiUtils::Join( pending, ", " ) + "\n";
        if ( !firstFailedJob.IsNULLOrEmpty() )
            text += "Next: ci_job_details with run_id " + Quote( report.status.runId ) + " and job_id " + Quote( firstFailedJob ) +
                    " for the full errors of a failed job; only for the failures you need more of.";
        else
        if ( report.status.IsFinished() )
            text += "Nothing failed.";
        SetText( result, m_settings, CCiUtils::Bound( text, MaxReportChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CJobDetailsTool : public CCiTool
{
    public:

    CJobDetailsTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_job_details";
        descriptor.title = "Details of a CI job";
        descriptor.description = JobDetailsDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "run_id", "string", "The run id, as ci_submit gives it", true );
        AddProperty( schema, "job_id", "string", "The job, as ci_status and ci_report name it", true );
        AddProperty( schema, "product", "string", "A product, or a test application, of the job; default: the products that failed", false );
        AddEnumProperty( schema, "part", "digest (default), log_tail or tests", "digest", "log_tail", "tests" );
        AddIntegerProperty( schema, "limit", "log_tail: the number of lines (default 200)", 1, (Int32) MaxTailLines );
        AddIntegerProperty( schema, "offset", "log_tail: lines skipped from the end first (default 0)", 0, 100000000 );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString runId = Argument( arguments, "run_id" );
        const CORE::CString jobId = Argument( arguments, "job_id" );
        CORE::CString part = Argument( arguments, "part" ).Lowercase();
        if ( part.IsNULLOrEmpty() )
            part = "digest";
        UInt32 limit = arguments.GetAttributeValue( "limit" ).AsUInt32( DefaultTailLines );
        if ( 0 == limit || limit > MaxTailLines )
            limit = DefaultTailLines;
        const UInt32 offset = arguments.GetAttributeValue( "offset" ).AsUInt32( 0 );

        CCiJobDetail detail;
        CORE::CString error;
        if ( !m_service.JobDetails( runId, jobId, Argument( arguments, "product" ), detail, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        const CCiReport::CCell& cell = detail.cell;
        CORE::CString text = "Job " + jobId + " of run " + detail.runId + ": " + CCiRunStatus::StateToString( detail.state ) + ".\n";
        if ( cell.hasReport )
        {
            CORE::CString::StringVector products;
            for ( size_t i=0; i<cell.products.size(); ++i )
            {
                const CCiReport::CProduct& entry = cell.products[ i ];
                if ( entry.IsImage() )
                {
                    products.push_back( "image " + entry.name + " " + entry.status );
                    continue;
                }
                products.push_back( entry.name + " " + ( entry.IsFailed() ? "failed in " + ( entry.phase.IsNULLOrEmpty() ? CORE::CString( "build" ) : entry.phase )
                                                                            : CCiReport::DescribeStatus( entry.status ) ) );
                for ( size_t t=0; t<entry.tests.size(); ++t )
                    products.push_back( "test " + entry.tests[ t ].name + " " + CCiReport::DescribeStatus( entry.tests[ t ].status ) );
            }
            if ( !products.empty() )
                text += "In the report: " + CCiUtils::Join( products, ", " ) + "\n";
        }
        if ( !detail.note.IsNULLOrEmpty() )
            text += detail.note + ( detail.note.EndsWith( "\n" ) ? "" : "\n" );

        // images: the status, tag and digest of each, the reason of those that failed; the logs follow
        for ( size_t i=0; i<cell.products.size(); ++i )
        {
            const CCiReport::CProduct& entry = cell.products[ i ];
            if ( entry.IsImage() )
                text += DescribeImageBuild( "image " + entry.name, entry.status, entry.tag, CORE::CString::StringVector(), entry.digest, entry.seconds,
                                            entry.logTail, cell.workspace, CORE::CString::Empty );
        }

        // test applications: no log of their own, the report has their failed cases and the end of their output
        const CORE::CString product = Argument( arguments, "product" );
        const CCiReport::CTest* namedTest = product.IsNULLOrEmpty() ? GUCEF_NULL : cell.FindTest( product, GUCEF_NULL );
        if ( GUCEF_NULL != namedTest )
        {
            if ( part == "log_tail" )
            {
                UInt32 lines = 0;
                text += "test " + namedTest->name + ": " + CCiReport::DescribeStatus( namedTest->status ) + "; the end of its output from the report" +
                        ( namedTest->logTail.IsNULLOrEmpty() ? CORE::CString( ": none (the report has it only for tests that did not succeed)\n" ) : CORE::CString( ":\n" ) ) +
                        CCiUtils::TailLines( namedTest->logTail, limit, offset, lines );
            }
            else
                text += DescribeFailedTest( *namedTest, cell.workspace, CORE::CString::Empty, MaxCasesInDetails, true );
        }
        else
        if ( part == "tests" )
        {
            bool any = false;
            bool ran = false;
            for ( size_t p=0; p<cell.products.size(); ++p )
            {
                for ( size_t t=0; t<cell.products[ p ].tests.size(); ++t )
                {
                    const CCiReport::CTest& test = cell.products[ p ].tests[ t ];
                    ran = true;
                    if ( !test.IsFailed() )
                        continue;
                    any = true;
                    text += DescribeFailedTest( test, cell.workspace, CORE::CString::Empty, MaxCasesInDetails, true );
                }
            }
            if ( !any )
                text += ran ? "No test failed.\n" : "The job ran no tests (mode " + ( cell.mode.IsNULLOrEmpty() ? CORE::CString( "build" ) : cell.mode ) + ").\n";
        }

        for ( size_t i=0; i<detail.logs.size(); ++i )
        {
            const CCiJobDetail::CLog& log = detail.logs[ i ];
            const bool isImageLog = cell.mode == "images";
            text += ( isImageLog ? "\n== image " : "\n== product " ) + log.name;
            if ( log.content.IsNULLOrEmpty() )
            {
                text += ": no log (" + ( log.problem.IsNULLOrEmpty() ? CORE::CString( "empty" ) : log.problem ) + ")\n";
                continue;
            }
            const CORE::CString path = SaveLog( detail.runId, jobId, log );
            UInt32 totalLines = 0;
            CCiUtils::TailLines( log.content, 0, 0, totalLines );
            if ( log.truncated )
                text += " (the last " + CORE::UInt32ToString( totalLines ) + " lines of " + CCiUtils::FormatBytes( log.totalBytes ) +
                        ( path.IsNULLOrEmpty() ? CORE::CString::Empty : ", end of the log: " + path ) + ")\n";
            else
                text += " (" + CORE::UInt32ToString( totalLines ) + " lines" + ( path.IsNULLOrEmpty() ? CORE::CString::Empty : ", full log: " + path ) + ")\n";
            if ( part == "tests" || ( GUCEF_NULL != namedTest && part == "log_tail" ) )
                continue;           // the log holds the output of the tests too: grep it
            if ( part == "log_tail" )
            {
                text += CCiUtils::TailLines( log.content, limit, offset, totalLines );
                continue;
            }
            text += log.digest.IsNULLOrEmpty() ? FormatLogDigest( log.content, cell.workspace ) : log.digest;
        }
        if ( detail.logs.empty() && part != "tests" && cell.hasReport && GUCEF_NULL == namedTest )
            text += "Nothing failed in this job; name a product to see its log.\n";
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    /** Keeps the full log, so that it can be searched without asking again. @return its absolute path, empty when it could not be written */
    CORE::CString SaveLog( const CORE::CString& runId, const CORE::CString& jobId, const CCiJobDetail::CLog& log ) const
    {
        if ( m_settings.logDir.IsNULLOrEmpty() )
            return CORE::CString::Empty;
        CORE::CreateDirs( m_settings.logDir );
        const CORE::CString name = "ci-" + CCiUtils::SafeName( runId, 60 ) + "-" + CCiUtils::SafeName( jobId, 80 ) + "-" + CCiUtils::SafeName( log.name, 80 ) + ".log";
        const CORE::CString path = CCiUtils::MakeAbsolute( CORE::CombinePath( m_settings.logDir, name ) );
        if ( !CORE::WriteFileAtomically( path, log.content.C_String(), log.content.ByteSize() > 0 ? log.content.ByteSize() - 1 : 0 ) )
            return CORE::CString::Empty;
        return path;
    }
};

/*-------------------------------------------------------------------------*/

class CCancelTool : public CCiTool
{
    public:

    CCancelTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_cancel";
        descriptor.title = "Cancel a CI run";
        descriptor.description = CancelDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "run_id", "string", "The run id, as ci_submit gives it", true );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString message;
        CORE::CString error;
        if ( !m_service.Cancel( Argument( arguments, "run_id" ), message, error ) )
            SetText( result, m_settings, error, true );
        else
            SetText( result, m_settings, message, false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CCleanupTool : public CCiTool
{
    public:

    CCleanupTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_cleanup";
        descriptor.title = "Remove old CI snapshots and reports";
        descriptor.description = CleanupDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = true;
        descriptor.maxDurationMs = 15 * 60 * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddIntegerProperty( schema, "older_than_days", "Remove what is older than this many days (default 14)", 1, 3650 );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UInt32 days = arguments.GetAttributeValue( "older_than_days" ).AsUInt32( DefaultRetentionDays );
        if ( 0 == days )
            days = DefaultRetentionDays;
        CORE::CString summary;
        CORE::CString error;
        if ( !m_service.Cleanup( Argument( arguments, "ci_id" ), days, summary, error ) )
            SetText( result, m_settings, error, true );
        else
            SetText( result, m_settings, summary, false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

/** Only the boolean true confirms: not "true", not 1 */
static bool
IsConfirmed( const CORE::CDataNode& arguments )
{
    const CORE::CVariant value = arguments.GetAttributeValue( "confirm" );
    return value.IsBoolean() && value.AsBool( false );
}

/*-------------------------------------------------------------------------*/

/** A deployment or rollback for the reader: what happened, the state, what to do next */
static CORE::CString
DescribeDeployResult( const CCiDeployResult& deploy, const CORE::CString& error )
{
    CORE::CString text;
    if ( deploy.IsSuccess() )
        text = ( deploy.outcome == "unchanged" ? CORE::CString( "Unchanged: " ) : ( deploy.action == "rollback" ? CORE::CString( "Rolled back: " ) : CORE::CString( "Deployed: " ) ) ) +
               deploy.message + "\n";
    else
    if ( deploy.IsRefused() )
        text = "Refused (" + deploy.code + "), nothing was changed: " + ( error.IsNULLOrEmpty() ? deploy.message : error ) + "\n";
    else
        text = "The " + ( deploy.action.IsNULLOrEmpty() ? CORE::CString( "deployment" ) : ( deploy.action == "rollback" ? CORE::CString( "rollback" ) : CORE::CString( "deployment" ) ) ) +
               " did not complete (" + ( deploy.outcome.IsNULLOrEmpty() ? CORE::CString( "failed" ) : deploy.outcome ) + "): " + ( error.IsNULLOrEmpty() ? deploy.message : error ) + "\n";
    if ( !deploy.nameSpace.IsNULLOrEmpty() )
        text += "Target " + deploy.target + ": Deployment " + deploy.nameSpace + "/" + deploy.deployment + ", container " + deploy.container + ".\n";
    if ( !deploy.runId.IsNULLOrEmpty() && deploy.action == "deploy" )
        text += "Run " + deploy.runId + ( deploy.commit.IsNULLOrEmpty() ? CORE::CString::Empty : ", commit " + deploy.commit ) +
                ( deploy.digest.IsNULLOrEmpty() ? CORE::CString::Empty : ", digest " + deploy.digest ) + ".\n";
    if ( !deploy.previousImage.IsNULLOrEmpty() && !deploy.IsRefused() )
        text += "It ran before: " + deploy.previousImage + "\n";
    if ( !deploy.IsRefused() )
        text += "Next: ci_release_status with ci_id " + Quote( deploy.ciId ) + " and target " + Quote( deploy.target ) + " shows the state.";
    return text;
}

/*-------------------------------------------------------------------------*/

class CImagesTool : public CCiTool
{
    public:

    CImagesTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_images";
        descriptor.title = "Images a CI run built";
        descriptor.description = ImagesDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "run_id", "string", "The run id, as ci_submit gives it", true );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiImageReport report;
        CORE::CString error;
        const CORE::CString runId = Argument( arguments, "run_id" );
        if ( !m_service.Images( runId, report, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CString text = "Images of run " + runId + ": " + ( report.status.IsNULLOrEmpty() ? CORE::CString( "no status" ) : report.status ) +
                             ", commit " + ( report.commit.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : report.commit ) +
                             ( report.ref.IsNULLOrEmpty() ? CORE::CString::Empty : " on " + report.ref ) + ", " +
                             ( report.scratch ? CORE::CString( "a scratch build" ) : CORE::CString( "a build of a published branch" ) ) + ".\n";
        UInt32 ok = 0;
        for ( size_t i=0; i<report.images.size(); ++i )
        {
            const CCiImageReport::CImage& image = report.images[ i ];
            text += DescribeImageBuild( "- " + image.name + ( image.registryName != image.name ? " (" + image.registryName + ")" : CORE::CString::Empty ),
                                        image.status, image.tag, image.alsoTags, image.digest, image.seconds, image.logTail, CORE::CString::Empty, CORE::CString::Empty );
            if ( image.IsOk() )
                ++ok;
        }
        if ( report.images.empty() )
            text += "The report names no images.\n";
        if ( ok < report.images.size() )
            text += "ci_job_details with run_id " + Quote( runId ) + " and job_id \"images\" gives the full log of an image that failed.\n";
        if ( ok > 0 )
            text += "ci_deploy can roll an image that is ok out, only when the user asks for it; ci_release_status shows what runs now.";
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CDeployTool : public CCiTool
{
    public:

    CDeployTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_deploy";
        descriptor.title = "Deploy an image to a target";
        descriptor.description = DeployDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = true;           // the same image again changes nothing
        descriptor.openWorld = true;
        descriptor.maxDurationMs = 65 * 60 * 1000;      // the longest rollout and the checks before it
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddProperty( schema, "image", "string", "The image, as ci_images names it", true );
        AddProperty( schema, "target", "string", "The target, as ci_release_status names it (for example live)", true );
        AddProperty( schema, "run_id", "string", "The green run that built the image, as ci_submit gives it", false );
        AddProperty( schema, "commit", "string", "Instead of run_id: the full commit whose newest run built the image", false );
        AddProperty( schema, "confirm", "boolean", "Must be true: the user explicitly asked for this deployment", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString runOrCommit = Argument( arguments, "run_id" );
        const CORE::CString commit = Argument( arguments, "commit" );
        if ( runOrCommit.IsNULLOrEmpty() == commit.IsNULLOrEmpty() )
        {
            SetText( result, m_settings, "Give run_id or commit, one of them", true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( runOrCommit.IsNULLOrEmpty() )
            runOrCommit = commit;
        CCiDeployResult deploy;
        CORE::CString error;
        CContextProgress progress( context );
        const bool deployed = m_service.Deploy( Argument( arguments, "ci_id" ), Argument( arguments, "image" ), Argument( arguments, "target" ), runOrCommit,
                                                IsConfirmed( arguments ), m_settings.caller, deploy, error, &progress );
        const CORE::CString text = deploy.outcome.IsNULLOrEmpty() && !deployed ? error : DescribeDeployResult( deploy, error );
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), !deployed );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CReleaseStatusTool : public CCiTool
{
    public:

    CReleaseStatusTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_release_status";
        descriptor.title = "What a deployment target runs";
        descriptor.description = ReleaseStatusDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddProperty( schema, "target", "string", "The target, for example live", true );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiReleaseStatus status;
        CORE::CString error;
        if ( !m_service.ReleaseStatus( Argument( arguments, "ci_id" ), Argument( arguments, "target" ), status, error ) )
        {
            SetText( result, m_settings, error, true );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CString text = "Target " + status.target + " of CI id " + status.ciId + ":\n";
        for ( size_t i=0; i<status.entries.size(); ++i )
        {
            const CCiReleaseStatus::CEntry& entry = status.entries[ i ];
            text += "- image " + entry.image + ": Deployment " + entry.nameSpace + "/" + entry.deployment + ", container " + entry.container +
                    ( entry.allowScratch ? CORE::CString( " (takes scratch builds too)" ) : CORE::CString( " (published commits only)" ) ) + "\n";
            if ( !entry.problem.IsNULLOrEmpty() )
            {
                text += "  its state could not be read: " + entry.problem + "\n";
                continue;
            }
            text += "  runs " + entry.imageReference + "\n";
            text += "  pull policy " + entry.imagePullPolicy + ", revision " + ( entry.revision.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : entry.revision ) +
                    ", strategy " + ( entry.strategy.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : entry.strategy ) + ", " + entry.replicas + "\n";
            if ( entry.imagePullPolicy == "Never" )
                text += "  it pulls nothing (Never): it cannot be deployed from here until its service is switched to the registry once\n";
            if ( entry.digest.IsNULLOrEmpty() )
                text += "  it runs a tag, not a digest: not deployed by the CI\n";
            text += "  last change: " + ( entry.changeCause.IsNULLOrEmpty() ? CORE::CString( "not recorded" ) : entry.changeCause ) + "\n";
            if ( !entry.history.IsNULLOrEmpty() )
                text += "  history:\n" + entry.history + "\n";
        }
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), false );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------*/

class CRollbackTool : public CCiTool
{
    public:

    CRollbackTool( ICiService& service, const SToolSettings& settings ) : CCiTool( service, settings ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ci_rollback";
        descriptor.title = "Roll a target back";
        descriptor.description = RollbackDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = false;          // a second rollback goes back further
        descriptor.openWorld = true;
        descriptor.maxDurationMs = 65 * 60 * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "ci_id", "string", "The CI id, as ci_resolve_repo gives it", true );
        AddProperty( schema, "target", "string", "The target, for example live", true );
        AddProperty( schema, "image", "string", "The image; only needed when more than one image has the target", false );
        AddProperty( schema, "confirm", "boolean", "Must be true: the user explicitly asked for this rollback", false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CCiDeployResult rollback;
        CORE::CString error;
        CContextProgress progress( context );
        const bool rolledBack = m_service.Rollback( Argument( arguments, "ci_id" ), Argument( arguments, "image" ), Argument( arguments, "target" ),
                                                    IsConfirmed( arguments ), m_settings.caller, rollback, error, &progress );
        const CORE::CString text = rollback.outcome.IsNULLOrEmpty() && !rolledBack ? error : DescribeDeployResult( rollback, error );
        SetText( result, m_settings, CCiUtils::Bound( text, MaxOutputChars ), !rolledBack );
        return COM::MCP_EXECUTE_COMPLETE;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SToolSettings::SToolSettings( void )
    : logDir()
    , maxWaitSeconds( 600 )
    , redactor()
    , caller()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
FormatLogDigest( const CORE::CString& log, const CORE::CString& workspace )
{GUCEF_TRACE;

    GUCEFMcpLocalLlm::CBuildLogDigest digest( workspace );
    digest.Parse( log.STL_String() );
    CORE::CString text = CORE::CString( digest.Format( 25, 5 ) );
    if ( !digest.HasFailures() )
        text += "No known error format found. The end of the log:\n" + CORE::CString( GUCEFMcpLocalLlm::CBuildLogDigest::Tail( log.STL_String(), 30, 3000 ) ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
GetToolNames( void )
{GUCEF_TRACE;

    static const char* names[] = { "ci_resolve_repo", "ci_describe", "ci_submit", "ci_status", "ci_report", "ci_job_details", "ci_cancel", "ci_cleanup",
                                   "ci_images", "ci_deploy", "ci_release_status", "ci_rollback" };
    CORE::CString::StringVector result;
    for ( size_t i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
        result.push_back( names[ i ] );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
RegisterTools( COM::CMcpServer& server, ICiService& service, const SToolSettings& settings )
{GUCEF_TRACE;

    CORE::CString error;
    bool allRegistered = true;
    COM::CMcpToolRegistry& registry = server.GetToolRegistry();
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CResolveTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CDescribeTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CSubmitTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CStatusTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CReportTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CJobDetailsTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CCancelTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CCleanupTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CImagesTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CDeployTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CReleaseStatusTool( service, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CRollbackTool( service, settings ) ), error ) && allRegistered;
    if ( !allRegistered )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: A tool could not be registered: " << error );
    return allRegistered;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
