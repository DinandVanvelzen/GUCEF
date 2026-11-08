/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

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

#include "gucefMcpLocalLlm_ProcessRun.h"
#include "gucefMcpLocalLlm_BuildLogDigest.h"
#include "gucefMcpLocalLlm_Tools.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 ProgressIntervalInMs = 15000;
static const UInt32 PumpIntervalInMs = 200;
static const size_t AgentOutputChars = 3000;
static const size_t BatchTaskOutputChars = 600;
static const UInt32 MaxBatchTasks = 50;
static const UInt32 MaxAskPaths = 50;
static const size_t MaxGlobFiles = 200;
static const UInt32 PromptReserveTokens = 1000;     /**< ask_local_llm: room in the context for the instructions and the question */
static const UInt32 CharsPerToken = 3;              /**< conservative for source code */
static const UInt32 MaxFailovers = 2;               /**< other servers a request moves to when its server is gone */

/*-------------------------------------------------------------------------*/

static const char* DelegateDescription =
    "Runs a coding agent on a local LLM to edit files in repo_dir; no Anthropic tokens are spent on its work. It can read "
    "and edit files only: no shell, builds or tests. Returns its summary and the change (the diff when small).\n"
    "Use for mechanical, fully specified changes that touch many lines or files: formatting, include guards, renames, "
    "build file lists, applying one pattern, boilerplate. The files must have no uncommitted changes. Do small edits, "
    "design work and debugging yourself.\n"
    "Task: one precise instruction naming the files and the rule. Always review the change; the local model is less "
    "reliable than you. Takes tens of seconds to minutes. For several independent tasks use delegate_batch.";

static const char* BatchDescription =
    "Runs several delegate_to_local_llm tasks in one call, in parallel on all free local LLM servers. Use instead of "
    "repeated delegate_to_local_llm calls: one call, one review. Each task must touch different files than the others. "
    "Returns per task ok/failed with a short summary, then the combined change.";

static const char* RunDescription =
    "Runs an allowed build or test command and returns only what matters: distinct errors and warnings with file:line, "
    "failed targets, failed tests, the test summary. The full output is saved to a log file you can grep. Use instead of "
    "running builds or tests through the shell and reading their output. Allowed commands: ";

static const char* AskDescription =
    "Answers a question about files using a local LLM that reads them, so you don't have to. Returns a short answer "
    "citing file:line. Use for questions that need many files or long files read: where something is used, how a flow "
    "works, what a file does. Check the cited lines before relying on an answer. paths: files, path:START-END for a "
    "line range, or wildcards: * within a directory, ** across directories (src/**/*.cpp), so no listing is needed "
    "first. deep: let the model think first (slower, for hard questions).";

static const char* StatusDescription =
    "Reports the local LLM servers and the models they run: reachable, model, context size, free request slots. Use "
    "when unsure whether delegation is available or which models there are.";

static const char* ModelPropertyDescription =
    "The model to use, as local_llm_status lists them; only servers running it are used. Default: any model";

static const char* AskSystemPrompt =
    "You answer questions about the files below. Each line starts with its line number. Use only these files. Cite "
    "file:line for every claim. If the files do not answer the question, say so plainly. Be brief: at most 15 lines "
    "unless a list is asked for.";

static const char* FailureSystemPrompt =
    "You read the end of the output of a failed build or test command. State the root cause in at most 8 lines: the "
    "first real error, with file:line when present. Skip follow-on errors and noise.";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
WildcardMatch( const char* pattern, const char* text )
{
    const char* star = GUCEF_NULL;
    const char* resume = GUCEF_NULL;
    while ( '\0' != *text )
    {
        if ( '?' == *pattern || *pattern == *text )
        {
            ++pattern;
            ++text;
        }
        else
        if ( '*' == *pattern )
        {
            star = pattern++;
            resume = text;
        }
        else
        if ( GUCEF_NULL != star )
        {
            pattern = star + 1;
            text = ++resume;
        }
        else
            return false;
    }
    while ( '*' == *pattern )
        ++pattern;
    return '\0' == *pattern;
}

/*-------------------------------------------------------------------------*/

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
AddStringArrayProperty( CORE::CDataNode& schema, const char* name, const char* description, UInt32 maxItems )
{
    CORE::CDataNode* property = AddProperty( schema, name, "array", description, true );
    CORE::CDataNode* items = property->AddChild( "items", GUCEF_DATATYPE_OBJECT );
    items->SetAttribute( "type", "string" );
    property->SetAttribute( "minItems", (Int32) 1 );
    property->SetAttribute( "maxItems", (Int32) maxItems );
}

/*-------------------------------------------------------------------------*/

static std::vector< CORE::CString >
GetStringArray( const CORE::CDataNode& arguments, const char* name )
{
    std::vector< CORE::CString > values;
    const CORE::CDataNode* array = arguments.FindChild( name );
    if ( GUCEF_NULL == array )
        return values;
    CORE::CDataNode::TVariantVector items = array->GetChildrenValues();
    for ( size_t i=0; i<items.size(); ++i )
        values.push_back( items[ i ].AsString() );
    return values;
}

/*-------------------------------------------------------------------------*/

static bool
IsBlank( const CORE::CString& text )
{
    return text.Trim( true ).Trim( false ).IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

/** The output of the coding agent without the lines about the local setup of Claude Code, the end of it at most */
static CORE::CString
CleanAgentOutput( const std::string& output, size_t maxChars )
{
    static const char* noise[] = { "claude.ai connectors are disabled", "[claude-code:", "doesn't recognize the model",
                                   "no stdin data received", "Update Claude Code, or see" };
    std::string clean;
    size_t start = 0;
    while ( start < output.size() )
    {
        size_t end = output.find( '\n', start );
        if ( std::string::npos == end )
            end = output.size();
        const std::string line = output.substr( start, end - start );
        bool isNoise = false;
        for ( size_t i=0; i<sizeof( noise ) / sizeof( noise[ 0 ] ) && !isNoise; ++i )
            isNoise = std::string::npos != line.find( noise[ i ] );
        if ( !isNoise )
            clean += line + "\n";
        start = end + 1;
    }
    while ( !clean.empty() && ( '\n' == clean[ clean.size()-1 ] || ' ' == clean[ clean.size()-1 ] ) )
        clean.erase( clean.size()-1 );
    while ( !clean.empty() && '\n' == clean[ 0 ] )
        clean.erase( 0, 1 );
    if ( clean.size() > maxChars )
        clean = "[...]\n" + clean.substr( clean.size() - maxChars );
    return clean.empty() ? CORE::CString( "(no output)" ) : CORE::CString( clean );
}

/*-------------------------------------------------------------------------*/

static UInt32
CountLines( const CORE::CString& text )
{
    UInt32 lines = 0;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<text.ByteSize(); ++i )
        if ( '\n' == c[ i ] )
            ++lines;
    return lines;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GitStatus( const CORE::CString& repoDir )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "status" );
    args.push_back( "--porcelain" );
    CORE::CString output;
    RunGit( repoDir, args, output );
    return output;
}

/*-------------------------------------------------------------------------*/

static bool
IsGitRepo( const CORE::CString& repoDir )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "rev-parse" );
    args.push_back( "--is-inside-work-tree" );
    CORE::CString output;
    return RunGit( repoDir, args, output );
}

/*-------------------------------------------------------------------------*/

/**
 *  What changed in the repository since 'statusBefore': the diff itself when it is small, saving the reviewer a
 *  call to git diff, the statistics otherwise. New files are listed by git status.
 */
static CORE::CString
DescribeChanges( const CORE::CString& repoDir, const CORE::CString& statusBefore, UInt32 maxDiffLines )
{
    const CORE::CString statusAfter = GitStatus( repoDir );
    if ( statusAfter == statusBefore )
        return "No files were changed.";

    CORE::CString text = "git status --porcelain:\n" + statusAfter;
    CORE::CProcess::TStringVector diffArgs;
    diffArgs.push_back( "diff" );
    CORE::CString diff;
    if ( maxDiffLines > 0 && RunGit( repoDir, diffArgs, diff ) && !diff.IsNULLOrEmpty() && CountLines( diff ) <= maxDiffLines )
    {
        text += "git diff:\n" + diff;
        return text;
    }
    diffArgs.push_back( "--stat" );
    CORE::CString diffStat;
    if ( RunGit( repoDir, diffArgs, diffStat ) && !diffStat.IsNULLOrEmpty() )
        text += "git diff --stat (the diff is over " + CORE::UInt32ToString( maxDiffLines ) + " lines, review it with git diff):\n" + diffStat;
    return text;
}

/*-------------------------------------------------------------------------*/

/** bash for local-agent.sh, PowerShell for local-agent.ps1 */
static void
BuildAgentCommand( const SToolSettings& settings          ,
                   const CORE::CString& repoDir           ,
                   const CORE::CString& task              ,
                   CORE::CString& exe                     ,
                   CORE::CProcess::TStringVector& args    )
{
    args.clear();
    if ( settings.agentScript.Lowercase().EndsWith( ".ps1" ) )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        exe = "powershell.exe";
        #else
        exe = "pwsh";
        #endif
        args.push_back( "-NoProfile" );
        args.push_back( "-ExecutionPolicy" );
        args.push_back( "Bypass" );
        args.push_back( "-File" );
        args.push_back( settings.agentScript );
        args.push_back( "-Dir" );
        args.push_back( repoDir );
        args.push_back( task );
    }
    else
    {
        exe = "bash";
        args.push_back( settings.agentScript );
        args.push_back( "-C" );
        args.push_back( repoDir );
        args.push_back( task );
    }
}

/*-------------------------------------------------------------------------*/

static CORE::CProcess::TStringMap
AgentEnvironment( const SEndpointState& endpoint, const CLlmEndpointPool& pool, const SToolSettings& settings )
{
    CORE::CProcess::TStringMap env;
    env[ "LOCAL_LLM_URL" ] = endpoint.url;
    if ( !pool.GetApiKey().IsNULLOrEmpty() )
        env[ "LOCAL_LLM_API_KEY" ] = pool.GetApiKey();
    const CORE::CString& model = endpoint.model.IsNULLOrEmpty() ? settings.model : endpoint.model;
    if ( !model.IsNULLOrEmpty() )
        env[ "LOCAL_LLM_MODEL" ] = model;
    return env;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Seconds( UInt64 ms )
{
    return CORE::UInt64ToString( ms / 1000 ) + "s";
}

/*-------------------------------------------------------------------------*/

/** Reports progress at most every ProgressIntervalInMs */
class CProgressReporter
{
    public:

    explicit CProgressReporter( COM::CMcpRequestContext& context )
        : m_context( context )
        , m_startTicks( MT::PrecisionTickCount() )
        , m_lastMs( 0 )
    {
    }

    void Report( Float64 progress, Float64 total, const CORE::CString& message )
    {
        const UInt64 elapsed = MilliSecsSince( m_startTicks );
        if ( elapsed - m_lastMs < ProgressIntervalInMs )
            return;
        m_lastMs = elapsed;
        m_context.ReportProgress( progress, total, message + ", " + Seconds( elapsed ) );
    }

    UInt64 GetElapsedMs( void ) const
    {
        return MilliSecsSince( m_startTicks );
    }

    private:

    COM::CMcpRequestContext& m_context;
    UInt64 m_startTicks;
    UInt64 m_lastMs;
};

/*-------------------------------------------------------------------------*/

static bool
ReadTextFile( const CORE::CString& path, std::string& content )
{
    std::ifstream file( path.C_String(), std::ios::in | std::ios::binary );
    if ( !file )
        return false;
    content.assign( ( std::istreambuf_iterator< char >( file ) ), std::istreambuf_iterator< char >() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** path, or path:START-END for lines START to END */
static void
ParsePathSpec( const CORE::CString& spec, CORE::CString& path, UInt32& firstLine, UInt32& lastLine )
{
    path = spec;
    firstLine = 1;
    lastLine = GUCEF_UINT32MAX;
    const std::string text = spec.STL_String();
    const size_t colon = text.rfind( ':' );
    if ( std::string::npos == colon || colon < 2 )
        return;
    const std::string range = text.substr( colon + 1 );
    const size_t dash = range.find( '-' );
    if ( std::string::npos == dash || 0 == dash || range.find_first_not_of( "0123456789-" ) != std::string::npos )
        return;
    path = text.substr( 0, colon );
    firstLine = CORE::StringToUInt32( range.substr( 0, dash ), 1 );
    lastLine = CORE::StringToUInt32( range.substr( dash + 1 ), GUCEF_UINT32MAX );
    if ( 0 == firstLine )
        firstLine = 1;
}

/*-------------------------------------------------------------------------*/

/** Lines firstLine to lastLine, each prefixed with its number. @return false for files that are not text */
static bool
NumberLines( const std::string& content, UInt32 firstLine, UInt32 lastLine, std::string& numbered, UInt32& lineCount )
{
    if ( std::string::npos != content.find( '\0' ) )
        return false;
    numbered.clear();
    lineCount = 0;
    UInt32 number = 1;
    size_t start = 0;
    while ( start < content.size() && number <= lastLine )
    {
        size_t end = content.find( '\n', start );
        if ( std::string::npos == end )
            end = content.size();
        if ( number >= firstLine )
        {
            numbered += CORE::UInt32ToString( number ).STL_String() + ": " + content.substr( start, end - start ) + "\n";
            ++lineCount;
        }
        ++number;
        start = end + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Glob match on paths with / separators: * and ? within a directory, ** across directories */
static bool
GlobMatch( const char* pattern, const char* text, const char* textStart )
{
    while ( '\0' != *pattern )
    {
        if ( '*' == pattern[ 0 ] && '*' == pattern[ 1 ] )
        {
            pattern += 2;
            if ( '/' == *pattern )
                ++pattern;
            // zero or more whole directories: try at the start of every path segment
            for ( const char* at = text; ; ++at )
            {
                if ( ( at == textStart || '/' == at[ -1 ] ) && GlobMatch( pattern, at, textStart ) )
                    return true;
                if ( '\0' == *at )
                    return false;
            }
        }
        if ( '*' == *pattern )
        {
            ++pattern;
            for ( const char* at = text; ; ++at )
            {
                if ( GlobMatch( pattern, at, textStart ) )
                    return true;
                if ( '\0' == *at || '/' == *at )
                    return false;
            }
        }
        if ( '\0' == *text )
            return false;
        if ( '?' == *pattern ? '/' == *text : *pattern != *text )
            return false;
        ++pattern;
        ++text;
    }
    return '\0' == *text;
}

/*-------------------------------------------------------------------------*/

/** The DI_ functions of gucefCORE are not thread-safe; tool calls run in parallel */
static MT::CMutex g_directoryLock;

/** Files below 'dir' as paths relative to it, without hidden files and directories (.git and such) */
static void
ListFiles( const std::string& dir, const std::string& relative, std::vector< std::string >& files, UInt32 depth )
{
    if ( depth > 32 || files.size() >= MaxGlobFiles * 10 )
        return;
    std::vector< std::string > subDirs;
    struct CORE::SDI_Data* entry = CORE::DI_First_Dir_Entry( dir.c_str() );
    if ( GUCEF_NULL == entry )
        return;
    do
    {
        const std::string name = CORE::DI_Name( entry );
        if ( name.empty() || '.' == name[ 0 ] )
            continue;
        const std::string path = relative.empty() ? name : relative + "/" + name;
        if ( 1 == CORE::DI_Is_It_A_File( entry ) )
            files.push_back( path );
        else
            subDirs.push_back( name );
    }
    while ( 0 != CORE::DI_Next_Dir_Entry( entry ) );
    CORE::DI_Cleanup( entry );

    for ( size_t i=0; i<subDirs.size(); ++i )
        ListFiles( dir + "/" + subDirs[ i ], relative.empty() ? subDirs[ i ] : relative + "/" + subDirs[ i ], files, depth + 1 );
}

/*-------------------------------------------------------------------------*/

static bool
HasWildcard( const std::string& text )
{
    return std::string::npos != text.find_first_of( "*?" );
}

/*-------------------------------------------------------------------------*/

/**
 *  The files a pattern matches, sorted. 'shown' receives each path as the pattern spells it (relative stays
 *  relative), 'full' the path to open
 */
static void
ExpandGlob( const CORE::CString& baseDir                ,
            const CORE::CString& pattern                ,
            std::vector< std::string >& shown           ,
            std::vector< std::string >& full            )
{
    std::string normalized = pattern.STL_String();
    std::replace( normalized.begin(), normalized.end(), '\\', '/' );

    // the directories before the first wildcard are where the search starts
    size_t wildcard = normalized.find_first_of( "*?" );
    size_t split = normalized.rfind( '/', wildcard );
    const std::string prefix = std::string::npos == split ? std::string() : normalized.substr( 0, split );
    const std::string rest = std::string::npos == split ? normalized : normalized.substr( split + 1 );

    std::string root = prefix;
    if ( !CORE::IsAbsolutePath( prefix ) && !baseDir.IsNULLOrEmpty() )
        root = prefix.empty() ? baseDir.STL_String() : CORE::CombinePath( baseDir, prefix ).STL_String();
    std::replace( root.begin(), root.end(), '\\', '/' );
    if ( root.empty() )
        root = ".";

    std::vector< std::string > files;
    {
        MT::CScopeMutex lock( g_directoryLock );
        ListFiles( root, std::string(), files, 0 );
    }
    std::sort( files.begin(), files.end() );
    for ( size_t i=0; i<files.size() && shown.size() < MaxGlobFiles; ++i )
    {
        if ( !GlobMatch( rest.c_str(), files[ i ].c_str(), files[ i ].c_str() ) )
            continue;
        shown.push_back( prefix.empty() ? files[ i ] : prefix + "/" + files[ i ] );
        full.push_back( root + "/" + files[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Timestamp( void )
{
    char buffer[ 32 ];
    time_t now = time( GUCEF_NULL );
    strftime( buffer, sizeof( buffer ), "%Y%m%d-%H%M%S", localtime( &now ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

/**
 *  Picks an endpoint. A model named in the call ('model') is required. Without one the configured model is preferred
 *  and any other model is used when no server runs it.
 *  @return false with the reason in 'error'
 */
static bool
AcquireForModel( CLlmEndpointPool& pool                       ,
                 const std::vector< SEndpointState >& states  ,
                 const CORE::CString& model                   ,
                 const SToolSettings& settings                ,
                 SEndpointState& chosen                       ,
                 CORE::CString& error                         )
{
    if ( !model.IsNULLOrEmpty() )
    {
        if ( pool.AcquireFrom( states, chosen, model ) )
            return true;
        const CORE::CString models = pool.DescribeModels( states );
        error = "No reachable LLM server runs model '" + model + "'. " +
                ( models.IsNULLOrEmpty() ? CORE::CString( "No model is reachable." ) : "Reachable models: " + models + ". Name one of those, or leave model out." );
        return false;
    }
    if ( !settings.model.IsNULLOrEmpty() && pool.AcquireFrom( states, chosen, settings.model ) )
        return true;
    if ( pool.AcquireFrom( states, chosen ) )
        return true;
    error = "No local LLM server is usable.";
    return false;
}

/*-------------------------------------------------------------------------*/

/** As AcquireForModel(), after probing the endpoints; the reason includes their states */
static bool
AcquireForModel( CLlmEndpointPool& pool        ,
                 const CORE::CString& model    ,
                 const SToolSettings& settings ,
                 SEndpointState& chosen        ,
                 CORE::CString& error          )
{
    std::vector< SEndpointState > states;
    pool.ProbeAll( states );
    if ( AcquireForModel( pool, states, model, settings, chosen, error ) )
        return true;
    error += "\n" + pool.Describe( states );
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  A chat completion on 'endpoint', which the caller acquired, naming the model the server runs. When the server does
 *  not respond and is gone, the request goes to the next usable endpoint for 'model' (as AcquireForModel()) with a
 *  context of at least 'minContextSize' tokens (0: any). Releases every endpoint it used; 'endpoint' is the last one
 *  tried.
 */
static bool
ChatWithFailover( CLlmEndpointPool& pool            ,
                  SEndpointState& endpoint          ,
                  SChatRequest request              ,
                  const CORE::CString& model        ,
                  const SToolSettings& settings     ,
                  UInt32 minContextSize             ,
                  CORE::CString& answer             ,
                  CORE::CString& error              )
{
    for ( UInt32 attempt=0; ; ++attempt )
    {
        if ( !endpoint.model.IsNULLOrEmpty() )
            request.model = endpoint.model;
        else
            request.model = settings.model;
        bool connectionFailed = false;
        const bool answered = ChatCompletion( endpoint.url, pool.GetApiKey(), request, answer, error, &connectionFailed );
        pool.Release( endpoint.url );
        if ( answered || !connectionFailed || attempt >= MaxFailovers || pool.Recheck( endpoint.url ) )
            return answered;

        const CORE::CString goneUrl = endpoint.url;
        CORE::CString reason;
        if ( !AcquireForModel( pool, model, settings, endpoint, reason ) )
            return false;
        if ( endpoint.url == goneUrl || ( 0 != minContextSize && 0 != endpoint.contextSize && endpoint.contextSize < minContextSize ) )
        {
            pool.Release( endpoint.url );
            return false;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: " + goneUrl + " is gone (" + error + "), asking " + endpoint.url );
    }
}

/*-------------------------------------------------------------------------*/

/** Why a task on 'url' failed when the server went away during it; empty when it is still there */
static CORE::CString
GoneNote( CLlmEndpointPool& pool, const CORE::CString& url, bool isEphemeral )
{
    if ( pool.Recheck( url ) )
        return CORE::CString::Empty;
    return "The LLM server " + url + " went away during the task" +
           ( isEphemeral ? CORE::CString( " (its machine is not always on)" ) : CORE::CString::Empty ) + ".\n";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE TOOLS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class CStatusTool : public COM::CIMcpTool
{
    public:

    explicit CStatusTool( CLlmEndpointPool& pool )
        : m_pool( pool )
    {
    }

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "local_llm_status";
        descriptor.title = "Local LLM status";
        descriptor.description = StatusDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        descriptor.SetNoArguments();
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        std::vector< SEndpointState > states;
        m_pool.ProbeAll( states, true );
        bool anyUsable = false;
        for ( size_t i=0; i<states.size(); ++i )
            anyUsable = anyUsable || m_pool.IsUsable( states[ i ] );

        const CORE::CString models = m_pool.DescribeModels( states );
        result.AddText( "LLM servers, in order of preference:\n" + m_pool.Describe( states ) +
                        ( models.IsNULLOrEmpty() ? CORE::CString::Empty : "Models: " + models + ". Name one as model to use it.\n" ) +
                        ( anyUsable ? "Delegation is available." : "Delegation is not available: no LLM server is usable." ) );
        result.isError = !anyUsable;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CLlmEndpointPool& m_pool;
};

/*-------------------------------------------------------------------------*/

class CDelegateTool : public COM::CIMcpTool
{
    public:

    CDelegateTool( CLlmEndpointPool& pool, const SToolSettings& settings )
        : m_pool( pool )
        , m_settings( settings )
    {
    }

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "delegate_to_local_llm";
        descriptor.title = "Delegate to a local LLM";
        descriptor.description = DelegateDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = false;
        descriptor.maxDurationMs = ( m_settings.agentTimeoutInSecs + 120 ) * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "repo_dir", "string", "Absolute path of the directory to work in, usually the repository root", true );
        AddProperty( schema, "task", "string", "Precise instruction: the files and the exact rule to apply", true );
        AddProperty( schema, "model", "string", ModelPropertyDescription, false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString repoDir = arguments.GetAttributeValue( "repo_dir" ).AsString();
        const CORE::CString task = arguments.GetAttributeValue( "task" ).AsString();
        if ( repoDir.IsNULLOrEmpty() || !CORE::DirExists( repoDir ) )
        {
            result.SetError( "repo_dir is not an existing directory: " + repoDir );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( IsBlank( task ) )
        {
            result.SetError( "task is empty" );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        const CORE::CString model = arguments.GetAttributeValue( "model" ).AsString();
        SEndpointState endpoint;
        CORE::CString reason;
        if ( !AcquireForModel( m_pool, model, m_settings, endpoint, reason ) )
        {
            result.SetError( reason + "Do the task yourself, or pick another model." );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: Delegating a task in " + repoDir + " to " + endpoint.url + " (" + endpoint.model + ")" );

        const bool isGit = IsGitRepo( repoDir );
        const CORE::CString statusBefore = isGit ? GitStatus( repoDir ) : CORE::CString::Empty;

        CORE::CString exe;
        CORE::CProcess::TStringVector args;
        BuildAgentCommand( m_settings, repoDir, task, exe, args );
        CProgressReporter progress( context );
        CORE::CString text;
        CProcessRun* run = GUCEF_NULL;
        for ( UInt32 attempt=0; ; ++attempt )
        {
            GUCEF_DELETE run;
            run = GUCEF_NEW CProcessRun();
            const bool started = run->Start( exe, args, repoDir, AgentEnvironment( endpoint, m_pool, m_settings ), m_settings.agentTimeoutInSecs * 1000 );
            while ( started && run->Pump( PumpIntervalInMs ) )
            {
                if ( context.IsCancellationRequested() )
                    run->Stop( true );
                else
                    progress.Report( (Float64) ( progress.GetElapsedMs() / 1000 ), -1.0, "local agent running" );
            }
            m_pool.Release( endpoint.url );

            if ( !started )
            {
                GUCEF_DELETE run;
                result.SetError( "The coding agent could not be started: " + exe + " " + m_settings.agentScript );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            if ( 0 == run->GetExitCode() || run->WasCancelled() || run->TimedOut() )
                break;
            const CORE::CString gone = GoneNote( m_pool, endpoint.url, endpoint.isEphemeral );
            if ( gone.IsNULLOrEmpty() )
                break;
            text += gone;

            // the task starts again elsewhere only when it changed nothing yet
            if ( attempt >= MaxFailovers || !isGit || GitStatus( repoDir ) != statusBefore )
                break;
            const CORE::CString goneUrl = endpoint.url;
            if ( !AcquireForModel( m_pool, model, m_settings, endpoint, reason ) )
                break;
            if ( endpoint.url == goneUrl )
            {
                m_pool.Release( endpoint.url );
                break;
            }
            text += "It changed nothing; started again on " + endpoint.url + ".\n";
        }

        if ( run->WasCancelled() )
            text += "Cancelled; the agent was stopped.\n";
        if ( run->TimedOut() )
            text += "Stopped after the timeout of " + CORE::UInt32ToString( m_settings.agentTimeoutInSecs ) + "s.\n";
        text += "Agent (" + endpoint.url + ( endpoint.model.IsNULLOrEmpty() ? CORE::CString::Empty : ", " + endpoint.model ) + ", " + Seconds( run->GetElapsedMs() ) + ", exit " + CORE::Int32ToString( run->GetExitCode() ) + "):\n";
        text += CleanAgentOutput( run->GetOutput(), AgentOutputChars ) + "\n\n";
        text += isGit ? DescribeChanges( repoDir, statusBefore, m_settings.maxDiffLines ) : CORE::CString( "Not a git repository: review the changed files." );

        result.AddText( text );
        result.isError = run->WasCancelled() || run->TimedOut() || 0 != run->GetExitCode();
        GUCEF_DELETE run;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CLlmEndpointPool& m_pool;
    SToolSettings m_settings;
};

/*-------------------------------------------------------------------------*/

class CBatchTool : public COM::CIMcpTool
{
    public:

    CBatchTool( CLlmEndpointPool& pool, const SToolSettings& settings )
        : m_pool( pool )
        , m_settings( settings )
    {
    }

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "delegate_batch";
        descriptor.title = "Delegate a batch to local LLMs";
        descriptor.description = BatchDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;
        descriptor.idempotent = false;
        // the tasks may all run one after the other on a single server; each has its own timeout
        descriptor.maxDurationMs = 24 * 3600 * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "repo_dir", "string", "Absolute path of the directory to work in, usually the repository root", true );
        AddStringArrayProperty( schema, "tasks", "Precise instructions, one per task; tasks must touch different files", MaxBatchTasks );
        CORE::CDataNode* maxParallel = AddProperty( schema, "max_parallel", "integer", "At most this many tasks at the same time; default: the free LLM slots", false );
        maxParallel->SetAttribute( "minimum", (Int32) 1 );
        AddProperty( schema, "model", "string", ModelPropertyDescription, false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString repoDir = arguments.GetAttributeValue( "repo_dir" ).AsString();
        const std::vector< CORE::CString > tasks = GetStringArray( arguments, "tasks" );
        if ( repoDir.IsNULLOrEmpty() || !CORE::DirExists( repoDir ) )
        {
            result.SetError( "repo_dir is not an existing directory: " + repoDir );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( tasks.empty() || tasks.size() > MaxBatchTasks )
        {
            result.SetError( "tasks must hold 1 to " + CORE::UInt32ToString( MaxBatchTasks ) + " instructions" );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        for ( size_t i=0; i<tasks.size(); ++i )
        {
            if ( IsBlank( tasks[ i ] ) )
            {
                result.SetError( "task " + CORE::UInt32ToString( (UInt32) i+1 ) + " is empty" );
                return COM::MCP_EXECUTE_COMPLETE;
            }
        }

        // the model of all tasks: the one named, else the configured one when a server runs it, else any
        std::vector< SEndpointState > states;
        m_pool.ProbeAll( states );
        CORE::CString model = arguments.GetAttributeValue( "model" ).AsString();
        if ( model.IsNULLOrEmpty() && !m_settings.model.IsNULLOrEmpty() )
        {
            SEndpointState probe;
            if ( m_pool.AcquireFrom( states, probe, m_settings.model ) )
            {
                m_pool.Release( probe.url );
                model = m_settings.model;
            }
        }
        SEndpointState check;
        CORE::CString reason;
        if ( !AcquireForModel( m_pool, states, model, m_settings, check, reason ) )
        {
            result.SetError( reason + "\n" + m_pool.Describe( states ) + "Do the tasks yourself, or pick another model." );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        m_pool.Release( check.url );

        UInt32 parallel = m_pool.CountFreeSlots( states, model );
        if ( 0 == parallel )
            parallel = 1;
        const UInt32 requested = arguments.GetAttributeValue( "max_parallel" ).AsUInt32( 0 );
        if ( requested > 0 && requested < parallel )
            parallel = requested;
        if ( parallel > tasks.size() )
            parallel = (UInt32) tasks.size();

        const bool isGit = IsGitRepo( repoDir );
        const CORE::CString statusBefore = isGit ? GitStatus( repoDir ) : CORE::CString::Empty;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpLocalLlm: Batch of " + CORE::UInt32ToString( (UInt32) tasks.size() ) +
                          " tasks in " + repoDir + ", " + CORE::UInt32ToString( parallel ) + " at a time" );

        std::vector< CProcessRun* > runs( tasks.size(), GUCEF_NULL );
        std::vector< CORE::CString > endpoints( tasks.size() );
        std::vector< bool > active( tasks.size(), false );
        size_t next = 0;
        UInt32 running = 0;
        UInt32 done = 0;
        bool cancelled = false;
        CProgressReporter progress( context );

        while ( done < tasks.size() )
        {
            // start tasks while there is room
            while ( !cancelled && next < tasks.size() && running < parallel )
            {
                std::vector< SEndpointState > current;
                m_pool.ProbeAll( current );
                SEndpointState endpoint;
                runs[ next ] = GUCEF_NEW CProcessRun();
                if ( !m_pool.AcquireFrom( current, endpoint, model ) )
                {
                    ++done;         // no server: the task fails without starting
                    ++next;
                    continue;
                }
                endpoints[ next ] = endpoint.url;
                CORE::CString exe;
                CORE::CProcess::TStringVector args;
                BuildAgentCommand( m_settings, repoDir, tasks[ next ], exe, args );
                if ( runs[ next ]->Start( exe, args, repoDir, AgentEnvironment( endpoint, m_pool, m_settings ), m_settings.agentTimeoutInSecs * 1000 ) )
                {
                    active[ next ] = true;
                    ++running;
                }
                else
                {
                    m_pool.Release( endpoint.url );
                    ++done;
                }
                ++next;
            }

            if ( 0 == running )
            {
                if ( cancelled || next >= tasks.size() )
                    break;
                continue;
            }

            // a short wait per running task keeps the loop responsive
            const UInt32 wait = PumpIntervalInMs / running > 10 ? PumpIntervalInMs / running : 10;
            for ( size_t i=0; i<tasks.size(); ++i )
            {
                if ( !active[ i ] )
                    continue;
                if ( !runs[ i ]->Pump( wait ) )
                {
                    active[ i ] = false;
                    m_pool.Release( endpoints[ i ] );
                    --running;
                    ++done;
                }
            }

            if ( !cancelled && context.IsCancellationRequested() )
            {
                cancelled = true;
                for ( size_t i=0; i<tasks.size(); ++i )
                    if ( active[ i ] )
                        runs[ i ]->Stop( true );
            }
            progress.Report( (Float64) done, (Float64) tasks.size(), CORE::UInt32ToString( done ) + " of " + CORE::UInt32ToString( (UInt32) tasks.size() ) + " tasks done" );
        }

        CORE::CString text;
        UInt32 succeeded = 0;
        std::map< CORE::CString, CORE::CString > goneNotes;
        for ( size_t i=0; i<tasks.size(); ++i )
        {
            CProcessRun* run = runs[ i ];
            text += "[" + CORE::UInt32ToString( (UInt32) i+1 ) + "] ";
            if ( GUCEF_NULL == run || !run->WasStarted() )
            {
                text += cancelled ? "not started (cancelled)\n" : "failed: the agent could not be started\n";
                continue;
            }
            const bool ok = !run->WasCancelled() && !run->TimedOut() && 0 == run->GetExitCode();
            if ( ok )
                ++succeeded;
            text += ( ok ? "ok" : run->WasCancelled() ? "cancelled" : run->TimedOut() ? "timed out" : "failed" );
            text += " (" + endpoints[ i ] + ", " + Seconds( run->GetElapsedMs() ) + "): ";
            if ( !ok && !run->WasCancelled() && !run->TimedOut() )
            {
                if ( goneNotes.find( endpoints[ i ] ) == goneNotes.end() )
                    goneNotes[ endpoints[ i ] ] = GoneNote( m_pool, endpoints[ i ], false );
                text += goneNotes[ endpoints[ i ] ];
            }
            text += CleanAgentOutput( run->GetOutput(), BatchTaskOutputChars ) + "\n";
        }
        for ( size_t i=0; i<runs.size(); ++i )
            GUCEF_DELETE runs[ i ];

        text = CORE::UInt32ToString( succeeded ) + " of " + CORE::UInt32ToString( (UInt32) tasks.size() ) + " tasks succeeded in " +
               Seconds( progress.GetElapsedMs() ) + ", " + CORE::UInt32ToString( parallel ) + " at a time.\n" + text + "\n";
        text += isGit ? DescribeChanges( repoDir, statusBefore, m_settings.maxDiffLines ) : CORE::CString( "Not a git repository: review the changed files." );
        result.AddText( text );
        result.isError = cancelled || 0 == succeeded;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CLlmEndpointPool& m_pool;
    SToolSettings m_settings;
};

/*-------------------------------------------------------------------------*/

class CRunTool : public COM::CIMcpTool
{
    public:

    CRunTool( CLlmEndpointPool& pool, const SToolSettings& settings )
        : m_pool( pool )
        , m_settings( settings )
        , m_lock()
        , m_runCounter( 0 )
    {
    }

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "run_and_digest";
        descriptor.title = "Run a build or test and digest the output";
        CORE::CString allowed;
        for ( size_t i=0; i<m_settings.allowedCommands.size(); ++i )
            allowed += ( i > 0 ? ", " : "" ) + m_settings.allowedCommands[ i ];
        descriptor.description = CORE::CString( RunDescription ) + allowed;
        descriptor.effect = COM::MCP_TOOLEFFECT_DESTRUCTIVE;    // builds write files
        descriptor.idempotent = false;
        descriptor.maxDurationMs = ( m_settings.commandTimeoutInSecs + 120 ) * 1000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "command", "string", "The command line; it must match an allowed pattern. Not run by a shell: no pipes or redirection", true );
        AddProperty( schema, "working_dir", "string", "Absolute path of the directory to run it in", true );
        CORE::CDataNode* timeout = AddProperty( schema, "timeout_seconds", "integer", "Stop the command after this long; default and maximum: the server setting", false );
        timeout->SetAttribute( "minimum", (Int32) 1 );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString command = arguments.GetAttributeValue( "command" ).AsString().Trim( true ).Trim( false );
        const CORE::CString workingDir = arguments.GetAttributeValue( "working_dir" ).AsString();
        if ( workingDir.IsNULLOrEmpty() || !CORE::DirExists( workingDir ) )
        {
            result.SetError( "working_dir is not an existing directory: " + workingDir );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        bool allowed = false;
        for ( size_t i=0; i<m_settings.allowedCommands.size() && !allowed; ++i )
            allowed = WildcardMatch( m_settings.allowedCommands[ i ].C_String(), command.C_String() );
        if ( !allowed )
        {
            result.SetError( "The command is not allowed. Allowed patterns: " + JoinAllowed() );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CORE::CProcess::TStringVector args = CORE::CProcess::SplitCommandLine( command );
        if ( args.empty() )
        {
            result.SetError( "command is empty" );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        const CORE::CString exe = args[ 0 ];
        args.erase( args.begin() );

        UInt32 timeoutInSecs = arguments.GetAttributeValue( "timeout_seconds" ).AsUInt32( 0 );
        if ( 0 == timeoutInSecs || timeoutInSecs > m_settings.commandTimeoutInSecs )
            timeoutInSecs = m_settings.commandTimeoutInSecs;

        CProcessRun run;
        if ( !run.Start( exe, args, workingDir, CORE::CProcess::TStringMap(), timeoutInSecs * 1000 ) )
        {
            result.SetError( "The command could not be started: " + exe );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        CProgressReporter progress( context );
        while ( run.Pump( PumpIntervalInMs ) )
        {
            if ( context.IsCancellationRequested() )
                run.Stop( true );
            else
                progress.Report( (Float64) ( progress.GetElapsedMs() / 1000 ), -1.0, "running" );
        }

        const std::string& output = run.GetOutput();
        const CORE::CString logPath = SaveLog( command, output );
        CBuildLogDigest digest( workingDir );
        digest.Parse( output );

        CORE::CString text;
        const bool succeeded = !run.WasCancelled() && !run.TimedOut() && 0 == run.GetExitCode();
        text += ( succeeded ? "Succeeded" : run.WasCancelled() ? "Cancelled" : run.TimedOut() ? "Stopped at the timeout" : "Failed" );
        text += " (exit " + CORE::Int32ToString( run.GetExitCode() ) + ", " + Seconds( run.GetElapsedMs() ) + ", " +
                CORE::UInt32ToString( CountLines( output ) ) + " lines of output";
        text += logPath.IsNULLOrEmpty() ? CORE::CString( ")\n" ) : ", full log: " + logPath + ")\n";
        text += digest.Format( 25, digest.HasFailures() ? 5 : 10 );

        if ( !succeeded && !run.WasCancelled() && !digest.HasFailures() )
            text += ExplainFailure( output );
        result.AddText( text );
        result.isError = !succeeded;
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CORE::CString JoinAllowed( void ) const
    {
        CORE::CString allowed;
        for ( size_t i=0; i<m_settings.allowedCommands.size(); ++i )
            allowed += ( i > 0 ? ", " : "" ) + m_settings.allowedCommands[ i ];
        return allowed;
    }

    /** Keeps the full output, so the caller can search it instead of running the command again */
    CORE::CString SaveLog( const CORE::CString& command, const std::string& output )
    {
        if ( m_settings.logDir.IsNULLOrEmpty() )
            return CORE::CString::Empty;
        UInt32 number = 0;
        {
            MT::CScopeMutex lock( m_lock );
            number = ++m_runCounter;
        }
        CORE::CreateDirs( m_settings.logDir );
        const CORE::CString path = CORE::CombinePath( m_settings.logDir, "run-" + Timestamp() + "-" + CORE::UInt32ToString( number ) + ".log" );
        std::ofstream file( path.C_String(), std::ios::out | std::ios::binary );
        if ( !file )
            return CORE::CString::Empty;
        file << "$ " << command.C_String() << "\n" << output;
        return path;
    }

    /** No error was recognised: the local model reads the end of the output, or the end is shown */
    CORE::CString ExplainFailure( const std::string& output )
    {
        const std::string tail = CBuildLogDigest::Tail( output, 150, 12000 );
        SEndpointState endpoint;
        CORE::CString reason;
        if ( !tail.empty() && AcquireForModel( m_pool, CORE::CString::Empty, m_settings, endpoint, reason ) )
        {
            SChatRequest request;
            request.system = FailureSystemPrompt;
            request.user = "Output:\n" + tail;
            request.maxTokens = 600;
            request.timeoutInMs = 120000;
            CORE::CString answer;
            CORE::CString error;
            const bool answered = ChatWithFailover( m_pool, endpoint, request, CORE::CString::Empty, m_settings, 0, answer, error );
            if ( answered )
                return "No known error format found. The local LLM read the end of the output:\n" + answer + "\n";
        }
        return "No known error format found. The end of the output:\n" + CBuildLogDigest::Tail( output, 30, 3000 ) + "\n";
    }

    CLlmEndpointPool& m_pool;
    SToolSettings m_settings;
    MT::CMutex m_lock;
    UInt32 m_runCounter;
};

/*-------------------------------------------------------------------------*/

class CAskTool : public COM::CIMcpTool
{
    public:

    CAskTool( CLlmEndpointPool& pool, const SToolSettings& settings )
        : m_pool( pool )
        , m_settings( settings )
    {
    }

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "ask_local_llm";
        descriptor.title = "Ask a local LLM about files";
        descriptor.description = AskDescription;
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.idempotent = true;
        descriptor.maxDurationMs = 900000;
        CORE::CDataNode schema = MakeObjectSchema();
        AddProperty( schema, "question", "string", "The question", true );
        AddStringArrayProperty( schema, "paths", "Files to read: absolute, or relative to base_dir; path:START-END for a line range; * and ** wildcards", MaxAskPaths );
        AddProperty( schema, "base_dir", "string", "Directory relative paths are resolved against", false );
        AddProperty( schema, "deep", "boolean", "Think before answering: slower, for hard questions; default false", false );
        AddProperty( schema, "model", "string", ModelPropertyDescription, false );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_THREAD_POOL;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context       ,
                                            const CORE::CDataNode& arguments       ,
                                            COM::CMcpToolResult& result            ,
                                            COM::CMcpInputRequired& inputRequired  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString question = arguments.GetAttributeValue( "question" ).AsString();
        const CORE::CString baseDir = arguments.GetAttributeValue( "base_dir" ).AsString();
        const bool deep = arguments.GetAttributeValue( "deep" ).AsBool( false );
        const std::vector< CORE::CString > specs = GetStringArray( arguments, "paths" );
        if ( IsBlank( question ) )
        {
            result.SetError( "question is empty" );
            return COM::MCP_EXECUTE_COMPLETE;
        }
        if ( specs.empty() || specs.size() > MaxAskPaths )
        {
            result.SetError( "paths must hold 1 to " + CORE::UInt32ToString( MaxAskPaths ) + " files" );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        const CORE::CString model = arguments.GetAttributeValue( "model" ).AsString();
        SEndpointState endpoint;
        CORE::CString reason;
        if ( !AcquireForModel( m_pool, model, m_settings, endpoint, reason ) )
        {
            result.SetError( reason + "Read the files yourself, or pick another model." );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        // what fits in the context of the server, with room for the question and the answer
        const UInt32 answerTokens = deep ? 8192 : 2048;
        const UInt32 reserveTokens = answerTokens + PromptReserveTokens + (UInt32) ( question.Length() / CharsPerToken );
        const UInt32 contextSize = 0 != endpoint.contextSize ? endpoint.contextSize : 32768;
        const size_t budgetChars = contextSize > reserveTokens ? (size_t) ( contextSize - reserveTokens ) * CharsPerToken : 0;

        // wildcards become the files they match; a line range applies to a single file
        std::vector< CORE::CString > names;
        std::vector< CORE::CString > paths;
        std::vector< UInt32 > firstLines;
        std::vector< UInt32 > lastLines;
        CORE::CString notRead;
        for ( size_t i=0; i<specs.size(); ++i )
        {
            if ( HasWildcard( specs[ i ].STL_String() ) )
            {
                std::vector< std::string > shown, full;
                ExpandGlob( baseDir, specs[ i ], shown, full );
                if ( shown.empty() )
                    notRead += "  " + specs[ i ] + ": matches no files\n";
                for ( size_t f=0; f<shown.size(); ++f )
                {
                    names.push_back( shown[ f ] );
                    paths.push_back( full[ f ] );
                    firstLines.push_back( 1 );
                    lastLines.push_back( GUCEF_UINT32MAX );
                }
                continue;
            }
            CORE::CString path;
            UInt32 firstLine = 1;
            UInt32 lastLine = GUCEF_UINT32MAX;
            ParsePathSpec( specs[ i ], path, firstLine, lastLine );
            names.push_back( specs[ i ] );
            paths.push_back( !baseDir.IsNULLOrEmpty() && !CORE::IsAbsolutePath( path ) ? CORE::CombinePath( baseDir, path ) : path );
            firstLines.push_back( firstLine );
            lastLines.push_back( lastLine );
        }

        std::string files;
        UInt32 fileCount = 0;
        UInt32 lineCount = 0;
        for ( size_t i=0; i<names.size(); ++i )
        {
            const CORE::CString& path = paths[ i ];
            const UInt32 firstLine = firstLines[ i ];
            const UInt32 lastLine = lastLines[ i ];

            std::string content;
            std::string numbered;
            UInt32 lines = 0;
            if ( !ReadTextFile( path, content ) )
            {
                notRead += "  " + names[ i ] + ": cannot be read\n";
                continue;
            }
            if ( !NumberLines( content, firstLine, lastLine, numbered, lines ) )
            {
                notRead += "  " + names[ i ] + ": not a text file\n";
                continue;
            }
            const std::string block = "=== " + names[ i ].STL_String() + " ===\n" + numbered + "\n";
            if ( files.size() + block.size() > budgetChars )
            {
                notRead += "  " + names[ i ] + ": does not fit in the context of the model\n";
                continue;
            }
            files += block;
            ++fileCount;
            lineCount += lines;
        }

        if ( 0 == fileCount )
        {
            m_pool.Release( endpoint.url );
            result.SetError( "None of the files could be used:\n" + notRead );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        SChatRequest request;
        request.system = AskSystemPrompt;
        request.user = CORE::CString( files ) + "Question: " + question;
        request.thinking = deep;
        request.maxTokens = answerTokens;
        request.timeoutInMs = 840000;

        CORE::CString answer;
        CORE::CString error;
        const UInt64 startTicks = MT::PrecisionTickCount();
        const bool answered = ChatWithFailover( m_pool, endpoint, request, model, m_settings, endpoint.contextSize, answer, error );
        if ( !answered )
        {
            result.SetError( "The local LLM gave no answer (" + error + "); read the files yourself." );
            return COM::MCP_EXECUTE_COMPLETE;
        }

        CORE::CString text = answer.Trim( true ).Trim( false ) + "\n\n(local LLM" +
                             ( endpoint.model.IsNULLOrEmpty() ? CORE::CString::Empty : " " + endpoint.model ) + ", " + CORE::UInt32ToString( fileCount ) + " file(s), " +
                             CORE::UInt32ToString( lineCount ) + " lines, " + Seconds( MilliSecsSince( startTicks ) ) + "; check citations before relying on them)";
        if ( !notRead.IsNULLOrEmpty() )
            text += "\nNot read:\n" + notRead;
        result.AddText( text );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    private:

    CLlmEndpointPool& m_pool;
    SToolSettings m_settings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SToolSettings::SToolSettings( void )
    : agentScript()
    , model()
    , agentTimeoutInSecs( 900 )
    , maxDiffLines( 150 )
    , allowedCommands()
    , commandTimeoutInSecs( 3600 )
    , logDir()
{
}

/*-------------------------------------------------------------------------*/

bool
RegisterTools( COM::CMcpServer& server, CLlmEndpointPool& pool, const SToolSettings& settings )
{GUCEF_TRACE;

    CORE::CString error;
    bool allRegistered = true;
    COM::CMcpToolRegistry& registry = server.GetToolRegistry();
    if ( !settings.agentScript.IsNULLOrEmpty() && !pool.IsEmpty() )
    {
        allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CDelegateTool( pool, settings ) ), error ) && allRegistered;
        allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CBatchTool( pool, settings ) ), error ) && allRegistered;
    }
    if ( !pool.IsEmpty() )
        allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CAskTool( pool, settings ) ), error ) && allRegistered;
    allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CStatusTool( pool ) ), error ) && allRegistered;
    if ( !settings.allowedCommands.empty() )
        allRegistered = registry.Register( COM::CIMcpToolPtr( GUCEF_NEW CRunTool( pool, settings ) ), error ) && allRegistered;
    if ( !allRegistered )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpLocalLlm: A tool could not be registered: " + error );
    return allRegistered;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/
