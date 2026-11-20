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

#include <algorithm>

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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CForgejoReportLayout.h"
#include "gucefMcpCi_CForgejoBackend.h"

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

static const UInt32 ConnectTimeoutInMs = 10000;
static const UInt32 JsonTimeoutInMs = 60000;
static const UInt32 LogTimeoutInMs = 300000;
static const UInt64 MaxJsonBytes = (UInt64) 8 * 1024 * 1024;
static const UInt64 MaxLogBytes = (UInt64) 64 * 1024 * 1024;
static const UInt32 PageSize = 50;
static const UInt32 MaxTaskPages = 10;
static const UInt32 MaxListPages = 40;
static const UInt32 MaxLogsPerJob = 4;
static const UInt32 MaxBaseCommits = 16;
static const UInt32 GzipInputStepSize = 16384;
static const char* UserAgent = "gucefMcpCi/1.0";

/** The path after .../actions/runs/{id} that cancels a run; Forgejo versions without it answer 404 */
static const char* CancelPathSuffix = "/cancel";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
IsDigits( const CORE::CString& text )
{
    if ( text.IsNULLOrEmpty() )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; '\0' != c[ i ]; ++i )
        if ( c[ i ] < '0' || c[ i ] > '9' )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
BufferToString( const CORE::CDynamicBuffer& buffer )
{
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** The message of an error answer of Forgejo ({"message": "..."}), cut short */
static CORE::CString
ServerMessage( const CORE::CDataNode& json )
{
    const CORE::CString message = CCiUtils::Trimmed( json.GetAttributeValue( "message" ).AsString() );
    return message.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + CCiUtils::Bound( message, 300 );
}

/*-------------------------------------------------------------------------*/

/** Unix seconds of a time attribute; false for missing and zero times (Forgejo writes 0001-01-01 for "not yet") */
static bool
TimeAttribute( const CORE::CDataNode& node, const char* name, Int64& unixSeconds )
{
    return CCiUtils::ParseIsoToUnix( node.GetAttributeValue( name ).AsString(), unixSeconds ) && unixSeconds > 0;
}

/*-------------------------------------------------------------------------*/

/** Orders scratch branches newest first by the time of their commit */
class CBranchOrder
{
    public:

    Int64 time;
    CORE::CString commit;

    CBranchOrder( void ) : time( 0 ), commit() {}

    bool operator<( const CBranchOrder& other ) const { return time > other.time; }
};

/*-------------------------------------------------------------------------*/

/** Orders jobs by the id of their task */
class CTaskOrder
{
    public:

    Int64 taskId;
    CCiRunStatus::CJob job;

    CTaskOrder( void ) : taskId( 0 ), job() {}

    bool operator<( const CTaskOrder& other ) const { return taskId < other.taskId; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CForgejoBackend::CForgejoBackend( void )
    : ICiBackend()
    , m_token()
    , m_caBundlePath()
    , m_pusher( GUCEF_NULL )
    , m_defaultReportsOwner( CForgejoReportLayout::DefaultReportsOwner )
    , m_discoveryAttempts( 8 )
    , m_discoveryDelayInMs( 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CForgejoBackend::~CForgejoBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CForgejoBackend::SetToken( const CORE::CString& token )
{GUCEF_TRACE;

    m_token = token;
    if ( !m_token.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( m_token );
}

/*-------------------------------------------------------------------------*/

void
CForgejoBackend::SetCaBundlePath( const CORE::CString& caBundlePath )
{GUCEF_TRACE;

    m_caBundlePath = caBundlePath;
}

/*-------------------------------------------------------------------------*/

void
CForgejoBackend::SetSnapshotPusher( const CSnapshotBuilder* pusher )
{GUCEF_TRACE;

    m_pusher = pusher;
}

/*-------------------------------------------------------------------------*/

void
CForgejoBackend::SetDefaultReportsOwner( const CORE::CString& owner )
{GUCEF_TRACE;

    m_defaultReportsOwner = owner.IsNULLOrEmpty() ? CORE::CString( CForgejoReportLayout::DefaultReportsOwner ) : owner;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoBackend::GetReportsOwner( const CCiDefinition& ci ) const
{GUCEF_TRACE;

    return ci.reportsOwner.IsNULLOrEmpty() ? m_defaultReportsOwner : ci.reportsOwner;
}

/*-------------------------------------------------------------------------*/

void
CForgejoBackend::SetRunDiscovery( UInt32 attempts, UInt32 delayInMs )
{GUCEF_TRACE;

    m_discoveryAttempts = attempts > 0 ? attempts : 1;
    m_discoveryDelayInMs = delayInMs;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoBackend::GetBackendName( void ) const
{GUCEF_TRACE;

    return "forgejo";
}

/*-------------------------------------------------------------------------*/

CCiRunStatus::EState
CForgejoBackend::MapState( const CORE::CString& forgejoStatus )
{GUCEF_TRACE;

    const CORE::CString status = forgejoStatus.Lowercase();
    if ( status == "waiting" || status == "blocked" || status == "queued" || status == "pending" || status == "requested" )
        return CCiRunStatus::STATE_QUEUED;
    if ( status == "running" || status == "in_progress" )
        return CCiRunStatus::STATE_RUNNING;
    if ( status == "success" || status == "skipped" )
        return CCiRunStatus::STATE_SUCCEEDED;
    if ( status == "failure" || status == "error" )
        return CCiRunStatus::STATE_FAILED;
    if ( status == "cancelled" )
        return CCiRunStatus::STATE_CANCELLED;
    return CCiRunStatus::STATE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::Gunzip( const CORE::CDynamicBuffer& compressed, UInt64 maxSize, CORE::CDynamicBuffer& raw, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" );
    CORE::CICodecInstancePtr instance;
    if ( codec.IsNULL() || !codec->CreateInstance( CORE::CValueList(), instance ) || instance.IsNULL() )
    {
        problem = "no gzip codec is loaded (codecspluginZLIB)";
        return false;
    }

    raw.Clear();
    raw.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    const UInt8* data = static_cast< const UInt8* >( compressed.GetConstBufferPtr() );
    const UInt32 size = compressed.GetDataSize();
    UInt32 offset = 0;
    UInt32 flags = GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_DECODE;
    do
    {
        UInt32 step = size - offset;
        if ( step > GzipInputStepSize )
            step = GzipInputStepSize;
        if ( offset + step == size )
            flags |= GUCEF_CODEC_PROCESS_FINAL;
        if ( !instance->Process( flags, data + offset, step, GUCEF_NULL, raw ) )
        {
            problem = "the data is not valid gzip";
            return false;
        }
        if ( raw.GetDataSize() > maxSize )
        {
            problem = "the log expands to more than " + CCiUtils::FormatBytes( maxSize );
            return false;
        }
        offset += step;
        flags = GUCEF_CODEC_PROCESS_UPDATE | GUCEF_CODEC_PROCESS_DECODE;
    }
    while ( offset < size );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::Execute( const CORE::CString& method       ,
                          const CORE::CString& url          ,
                          const CORE::CString& jsonBody     ,
                          UInt64 maxResponseBytes           ,
                          UInt32 timeoutInMs                ,
                          Int32& status                     ,
                          CORE::CDynamicBuffer& body        ,
                          CORE::CString& error              ) const
{GUCEF_TRACE;

    status = 0;
    body.Clear();
    WEB::CHttpRequestData request;
    request.requestType = method;
    if ( !request.SetUrl( url ) )
    {
        error = "the CI address in the registry is not a valid URL";
        return false;
    }
    if ( !m_token.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = "token " + m_token;
    if ( !jsonBody.IsNULLOrEmpty() )
    {
        request.contentRepresentation = "application/json";
        request.content.CopyFrom( jsonBody.ByteSize() - 1, jsonBody.C_String() );
    }

    WEB::CHttpClientRequestOptions options;
    options.maxResponseBodySize = maxResponseBytes;
    options.connectTimeoutInMs = ConnectTimeoutInMs;
    options.totalTimeoutInMs = timeoutInMs;
    options.verifyPeer = true;
    options.caBundlePath = m_caBundlePath;
    options.followRedirects = false;            // the token is never sent elsewhere
    options.userAgent = UserAgent;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transferInfo;
    if ( !client.Execute( request, options, response, &transferInfo ) )
    {
        if ( transferInfo.responseBodyTooLarge )
            error = "the answer of the CI server is larger than " + CCiUtils::FormatBytes( maxResponseBytes );
        else
            error = "the CI server did not answer (" + CORE::CString( WEB::HttpTransferErrorClassToString( transferInfo.errorClass ) ) + ": " +
                    WEB::CHttpLogRedaction::RedactText( transferInfo.errorDescription ) + ")";
        return false;
    }
    status = response.statusCode;
    body = response.content;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::ExecuteJson( const CORE::CString& method       ,
                              const CORE::CString& url          ,
                              const CORE::CString& jsonBody     ,
                              Int32& status                     ,
                              CORE::CDataNode& json             ,
                              CORE::CString& error              ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer body;
    if ( !Execute( method, url, jsonBody, MaxJsonBytes, JsonTimeoutInMs, status, body, error ) )
        return false;
    json.Clear();
    if ( 0 == body.GetDataSize() )
        return true;
    if ( !CCiUtils::ParseJson( body, json ) )
    {
        if ( status >= 200 && status < 300 )
        {
            error = "the answer of the CI server is not JSON";
            return false;
        }
        json.Clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoBackend::ScratchApi( const CCiDefinition& ci ) const
{GUCEF_TRACE;

    return ci.GetBaseUrl() + "/api/v1/repos/" + CCiUtils::PercentEncode( ci.scratchOwner ) + "/" + CCiUtils::PercentEncode( ci.scratchRepo );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoBackend::PackageFileUrl( const CCiDefinition& ci, const CORE::CString& version, const CORE::CString& fileName ) const
{GUCEF_TRACE;

    return CForgejoReportLayout::FileUrl( ci.GetBaseUrl(), GetReportsOwner( ci ), version, fileName );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::Describe( const CCiDefinition& ci         ,
                           const CORE::CString& repoDir    ,
                           CCiDescription& description     ,
                           CORE::CString& error            )
{GUCEF_TRACE;

    description = CCiDescription();
    description.ciId = ci.id;

    if ( !repoDir.IsNULLOrEmpty() )
    {
        // the matrix files of the working tree: a snapshot of it runs with them
        bool found = false;
        if ( !description.LoadFromMatrixFiles( CORE::CombinePath( repoDir, ci.matrixDir ), found, error ) )
            return false;
        if ( found )
        {
            description.source = "the CI files of the repository (" + ci.matrixDir + ")";
            return true;
        }
    }

    // the options the registry gives
    for ( size_t i=0; i<ci.optionToolchains.size(); ++i )
    {
        CCiDescription::CToolchain toolchain;
        toolchain.id = ci.optionToolchains[ i ];
        description.toolchains.push_back( toolchain );
    }
    description.cppStandards = ci.optionCppStandards;
    description.products = ci.optionProducts;
    description.hasOptions = !ci.optionToolchains.empty() || !ci.optionCppStandards.empty() || !ci.optionProducts.empty();
    description.source = description.hasOptions ? "the CI registry" : "nothing: pass repo_dir to read the options from the CI files of the repository";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::SendSnapshot( const CCiDefinition& ci         ,
                               const CCiSnapshot& snapshot     ,
                               CORE::CString& error            )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pusher )
    {
        error = "The backend has nothing to send snapshots with";
        return false;
    }
    return m_pusher->Push( snapshot, ci.GetScratchGitUrl(), m_token.IsNULLOrEmpty() ? CORE::CString::Empty : "token " + m_token, error );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::FetchPublished( const CCiDefinition& ci         ,
                                 const CORE::CString& bareRepoDir ,
                                 CORE::CString& error            )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pusher )
    {
        error = "The backend has nothing to fetch snapshots with";
        return false;
    }
    return m_pusher->Fetch( bareRepoDir, ci.GetScratchGitUrl(), m_token.IsNULLOrEmpty() ? CORE::CString::Empty : "token " + m_token,
                            "+refs/heads/*:refs/remotes/scratch/*", error );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::ListBaseCommits( const CCiDefinition& ci                 ,
                                  CORE::CString::StringVector& commits    ,
                                  CORE::CString& error                    )
{GUCEF_TRACE;

    commits.clear();

    // the default branch of the scratch repository, the seed from the published repository
    CORE::CString defaultBranch;
    Int32 status = 0;
    CORE::CDataNode answer;
    if ( ExecuteJson( "GET", ScratchApi( ci ), CORE::CString::Empty, status, answer, error ) && 200 == status )
        defaultBranch = CCiUtils::Trimmed( answer.GetAttributeValue( "default_branch" ).AsString() );

    // the heads of the branches: the default branch first, then the scratch branches by the time of their commit
    CORE::CString defaultCommit;
    GUCEF::vector< CBranchOrder > scratch;
    for ( UInt32 page=1; page<=MaxListPages; ++page )
    {
        const CORE::CString url = ScratchApi( ci ) + "/branches?page=" + CORE::UInt32ToString( page ) + "&limit=" + CORE::UInt32ToString( PageSize );
        if ( !ExecuteJson( "GET", url, CORE::CString::Empty, status, answer, error ) )
            return false;
        if ( 200 != status )
        {
            error = "The CI did not list the branches of the scratch repository (HTTP " + CORE::Int32ToString( status ) + ")" + ServerMessage( answer );
            return false;
        }
        UInt32 count = 0;
        for ( CORE::CDataNode::const_iterator i = answer.ConstBegin(); i != answer.ConstEnd(); ++i )
        {
            ++count;
            const CORE::CString name = (*i)->GetAttributeValue( "name" ).AsString();
            const CORE::CDataNode* commit = (*i)->FindChild( "commit" );
            if ( GUCEF_NULL == commit )
                continue;
            const CORE::CString id = CCiUtils::Trimmed( commit->GetAttributeValue( "id" ).AsString() );
            if ( !CCiUtils::IsHex( id, 40 ) )
                continue;
            if ( name == defaultBranch || ( defaultBranch.IsNULLOrEmpty() && ( name == "main" || name == "master" ) && defaultCommit.IsNULLOrEmpty() ) )
            {
                defaultCommit = id;
                continue;
            }
            if ( !name.StartsWith( CSnapshotBuilder::BranchPrefix ) )
                continue;
            CBranchOrder entry;
            entry.commit = id;
            if ( !TimeAttribute( *commit, "timestamp", entry.time ) )
                entry.time = 0;
            scratch.push_back( entry );
        }
        if ( count < PageSize )
            break;
    }
    std::sort( scratch.begin(), scratch.end() );
    if ( !defaultCommit.IsNULLOrEmpty() )
        commits.push_back( defaultCommit );
    for ( size_t i=0; i<scratch.size() && commits.size() < MaxBaseCommits; ++i )
    {
        bool listed = false;
        for ( size_t c=0; c<commits.size() && !listed; ++c )
            listed = commits[ c ] == scratch[ i ].commit;
        if ( !listed )
            commits.push_back( scratch[ i ].commit );
    }
    error.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::StartRun( const CCiDefinition& ci             ,
                           const CCiSnapshot& snapshot         ,
                           const CCiSubmitOptions& options     ,
                           CORE::CString& runId                ,
                           CORE::CString& error                )
{GUCEF_TRACE;

    // empty filters are left out: the workflow takes them as everything
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "ref", snapshot.branchName );
    body.SetAttribute( "return_run_info", CORE::CVariant( true ) );
    CORE::CDataNode* inputs = body.AddChild( "inputs", GUCEF_DATATYPE_OBJECT );
    inputs->SetAttribute( "force", "true" );
    if ( !options.toolchains.empty() )
        inputs->SetAttribute( "toolchains", CCiUtils::Join( options.toolchains, "," ) );
    if ( !options.cppStandards.empty() )
        inputs->SetAttribute( "cppStandards", CCiUtils::Join( options.cppStandards, "," ) );
    if ( !options.products.empty() )
        inputs->SetAttribute( "products", CCiUtils::Join( options.products, "," ) );
    if ( !options.mode.IsNULLOrEmpty() && options.mode != "build" )
        inputs->SetAttribute( "mode", options.mode );
    CORE::CString bodyText;
    if ( !CCiUtils::ToJson( body, bodyText ) )
    {
        error = "The request could not be written as JSON";
        return false;
    }

    const CORE::CString url = ScratchApi( ci ) + "/actions/workflows/" + CCiUtils::PercentEncode( ci.workflow ) + "/dispatches";
    Int32 status = 0;
    CORE::CDataNode answer;
    if ( !ExecuteJson( "POST", url, bodyText, status, answer, error ) )
    {
        error = "The snapshot was sent but the run could not be started: " + error;
        return false;
    }
    if ( status < 200 || status >= 300 )
    {
        if ( 401 == status || 403 == status )
            error = "The CI refused the credentials of this server (HTTP " + CORE::Int32ToString( status ) + ")";
        else
        if ( 404 == status )
            error = "The CI has no workflow \"" + ci.workflow + "\" for the snapshot (HTTP 404); the CI files of the repository may be missing" + ServerMessage( answer );
        else
            error = "The CI did not start the run (HTTP " + CORE::Int32ToString( status ) + ")" + ServerMessage( answer );
        return false;
    }

    const Int64 id = answer.GetAttributeValue( "id" ).AsInt64( 0 );
    if ( id > 0 )
    {
        runId = CORE::Int64ToString( id );
        return true;
    }
    return DiscoverRun( ci, snapshot, runId, error );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::DiscoverRun( const CCiDefinition& ci, const CCiSnapshot& snapshot, CORE::CString& runId, CORE::CString& error ) const
{GUCEF_TRACE;

    // the run of the dispatch: event workflow_dispatch, the commit and the branch, newest first. The branch filter is
    // applied here, since the API wants the ref in its own form
    const CORE::CString url = ScratchApi( ci ) + "/actions/runs?event=workflow_dispatch&head_sha=" + CCiUtils::PercentEncode( snapshot.commit ) +
                              "&limit=" + CORE::UInt32ToString( PageSize );
    for ( UInt32 attempt=0; attempt<m_discoveryAttempts; ++attempt )
    {
        if ( attempt > 0 )
            MT::ThreadDelay( m_discoveryDelayInMs );
        Int32 status = 0;
        CORE::CDataNode answer;
        if ( !ExecuteJson( "GET", url, CORE::CString::Empty, status, answer, error ) || 200 != status )
            continue;
        const CORE::CDataNode* runs = answer.FindChild( "workflow_runs" );
        if ( GUCEF_NULL == runs )
            continue;
        Int64 newest = 0;
        for ( CORE::CDataNode::const_iterator i = runs->ConstBegin(); i != runs->ConstEnd(); ++i )
        {
            const CORE::CDataNode& run = *(*i);
            const CORE::CString ref = run.GetAttributeValue( "prettyref" ).AsString();
            const CORE::CString sha = run.GetAttributeValue( "commit_sha" ).AsString();
            const CORE::CString event = run.GetAttributeValue( "event" ).AsString();
            const bool refMatches = ref == snapshot.branchName || ref == "refs/heads/" + snapshot.branchName;
            if ( !refMatches || ( !sha.IsNULLOrEmpty() && sha != snapshot.commit ) || ( !event.IsNULLOrEmpty() && event != "workflow_dispatch" ) )
                continue;
            const Int64 id = run.GetAttributeValue( "id" ).AsInt64( 0 );
            if ( id > newest )
                newest = id;
        }
        if ( newest > 0 )
        {
            runId = CORE::Int64ToString( newest );
            return true;
        }
    }
    error = "The run was started but it could not be found afterwards; the snapshot is on branch " + snapshot.branchName +
            ". Try ci_submit again later.";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::LoadRun( const CCiDefinition& ci, const CORE::CString& runId, CCiRunStatus& status, bool& isUnknown, CORE::CString& error ) const
{GUCEF_TRACE;

    isUnknown = false;
    if ( !IsDigits( runId ) )
    {
        isUnknown = true;
        error = "Unknown run id";
        return false;
    }
    Int32 httpStatus = 0;
    CORE::CDataNode run;
    if ( !ExecuteJson( "GET", ScratchApi( ci ) + "/actions/runs/" + runId, CORE::CString::Empty, httpStatus, run, error ) )
        return false;
    if ( 404 == httpStatus )
    {
        isUnknown = true;
        error = "Unknown run id";
        return false;
    }
    if ( 200 != httpStatus )
    {
        error = "The CI did not give the run (HTTP " + CORE::Int32ToString( httpStatus ) + ")" + ServerMessage( run );
        return false;
    }

    status.state = MapState( run.GetAttributeValue( "status" ).AsString() );
    status.commit = run.GetAttributeValue( "commit_sha" ).AsString();
    status.title = run.GetAttributeValue( "title" ).AsString();
    Int64 start = 0;
    Int64 stop = 0;
    if ( TimeAttribute( run, "started", start ) || TimeAttribute( run, "created", start ) )
    {
        if ( !TimeAttribute( run, "stopped", stop ) || stop < start || !CCiRunStatus::IsFinal( status.state ) )
            stop = CCiUtils::NowUnix();
        status.elapsedSecs = stop > start ? (UInt64) ( stop - start ) : 0;
    }

    const Int64 runNumber = run.GetAttributeValue( "index_in_repo" ).AsInt64( 0 );
    status.jobs.clear();
    if ( runNumber > 0 )
        LoadJobs( ci, runNumber, status, error );
    if ( status.jobs.empty() && !status.commit.IsNULLOrEmpty() )
        LoadJobsFromStatuses( ci, status );

    // a run of which the state is not known yet takes it from its jobs
    if ( CCiRunStatus::STATE_UNKNOWN == status.state && !status.jobs.empty() )
    {
        bool allFinal = true;
        bool anyFailed = false;
        for ( size_t i=0; i<status.jobs.size(); ++i )
        {
            allFinal = allFinal && CCiRunStatus::IsFinal( status.jobs[ i ].state );
            anyFailed = anyFailed || CCiRunStatus::STATE_FAILED == status.jobs[ i ].state;
        }
        status.state = allFinal ? ( anyFailed ? CCiRunStatus::STATE_FAILED : CCiRunStatus::STATE_SUCCEEDED ) : CCiRunStatus::STATE_RUNNING;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::LoadJobs( const CCiDefinition& ci, Int64 runNumber, CCiRunStatus& status, CORE::CString& error ) const
{GUCEF_TRACE;

    // the tasks of the repository, newest first: the jobs of the run have its number and commit. A job that ran
    // more than once counts with its latest task
    typedef GUCEF::map< CORE::CString, CTaskOrder > TJobMap;
    TJobMap jobs;
    for ( UInt32 page=1; page<=MaxTaskPages; ++page )
    {
        Int32 httpStatus = 0;
        CORE::CDataNode answer;
        const CORE::CString url = ScratchApi( ci ) + "/actions/tasks?page=" + CORE::UInt32ToString( page ) + "&limit=" + CORE::UInt32ToString( PageSize );
        if ( !ExecuteJson( "GET", url, CORE::CString::Empty, httpStatus, answer, error ) || 200 != httpStatus )
            return false;
        const CORE::CDataNode* tasks = answer.FindChild( "workflow_runs" );
        if ( GUCEF_NULL == tasks || 0 == tasks->GetNrOfChildNodes() )
            break;
        bool olderSeen = false;
        for ( CORE::CDataNode::const_iterator i = tasks->ConstBegin(); i != tasks->ConstEnd(); ++i )
        {
            const CORE::CDataNode& task = *(*i);
            const Int64 number = task.GetAttributeValue( "run_number" ).AsInt64( 0 );
            if ( number < runNumber )
                olderSeen = true;
            if ( number != runNumber )
                continue;
            const CORE::CString sha = task.GetAttributeValue( "head_sha" ).AsString();
            if ( !sha.IsNULLOrEmpty() && !status.commit.IsNULLOrEmpty() && sha != status.commit )
                continue;
            CTaskOrder entry;
            entry.taskId = task.GetAttributeValue( "id" ).AsInt64( 0 );
            entry.job.id = CCiUtils::Trimmed( task.GetAttributeValue( "name" ).AsString() );
            // a matrix job is named "<job name> (<the values of the matrix cell>)": the job name alone is the cell id
            // that the reports are filed under
            {
                const Int32 matrixStart = entry.job.id.HasSubstr( " (" );
                if ( matrixStart > 0 )
                    entry.job.id = entry.job.id.SubstrToIndex( (UInt32) matrixStart, true );
            }
            entry.job.state = MapState( task.GetAttributeValue( "status" ).AsString() );
            Int64 start = 0;
            Int64 stop = 0;
            if ( TimeAttribute( task, "run_started_at", start ) )
            {
                if ( !CCiRunStatus::IsFinal( entry.job.state ) || !TimeAttribute( task, "updated_at", stop ) || stop < start )
                    stop = CCiUtils::NowUnix();
                entry.job.elapsedSecs = stop > start ? (UInt64) ( stop - start ) : 0;
            }
            if ( entry.job.id.IsNULLOrEmpty() )
                continue;
            TJobMap::iterator existing = jobs.find( entry.job.id );
            if ( existing == jobs.end() || (*existing).second.taskId < entry.taskId )
                jobs[ entry.job.id ] = entry;
        }
        if ( olderSeen || tasks->GetNrOfChildNodes() < PageSize )
            break;
    }

    GUCEF::vector< CTaskOrder > ordered;
    for ( TJobMap::iterator i = jobs.begin(); i != jobs.end(); ++i )
        ordered.push_back( (*i).second );
    std::sort( ordered.begin(), ordered.end() );
    for ( size_t i=0; i<ordered.size(); ++i )
        status.jobs.push_back( ordered[ i ].job );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::LoadJobsFromStatuses( const CCiDefinition& ci, CCiRunStatus& status ) const
{GUCEF_TRACE;

    // the commit statuses that Actions sets per job, newest first: "workflow / job (event)"
    Int32 httpStatus = 0;
    CORE::CDataNode answer;
    CORE::CString error;
    if ( !ExecuteJson( "GET", ScratchApi( ci ) + "/commits/" + CCiUtils::PercentEncode( status.commit ) + "/statuses?limit=" + CORE::UInt32ToString( PageSize ),
                       CORE::CString::Empty, httpStatus, answer, error ) || 200 != httpStatus )
        return false;
    CORE::CString::StringSet seen;
    for ( CORE::CDataNode::const_iterator i = answer.ConstBegin(); i != answer.ConstEnd(); ++i )
    {
        const CORE::CString context = CCiUtils::Trimmed( (*i)->GetAttributeValue( "context" ).AsString() );
        if ( context.IsNULLOrEmpty() || seen.find( context ) != seen.end() )
            continue;
        seen.insert( context );
        CCiRunStatus::CJob job;
        job.id = context;
        const CORE::CString state = (*i)->GetAttributeValue( "status" ).AsString().Lowercase();
        job.state = state == "pending" ? CCiRunStatus::STATE_RUNNING : MapState( state == "warning" ? CORE::CString( "success" ) : state );
        status.jobs.push_back( job );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::GetRun( const CCiDefinition& ci         ,
                         const CORE::CString& runId      ,
                         CCiRunStatus& status            ,
                         bool& isUnknown                 ,
                         CORE::CString& error            )
{GUCEF_TRACE;

    return LoadRun( ci, runId, status, isUnknown, error );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::LoadCellReport( const CCiDefinition& ci, const CORE::CString& runId, const CORE::CString& jobId, CCiReport::CCell& cell, bool& missing, CORE::CString& error ) const
{GUCEF_TRACE;

    missing = false;
    cell.jobId = jobId;
    Int32 status = 0;
    CORE::CDynamicBuffer body;
    const CORE::CString version = CForgejoReportLayout::MakeVersion( runId, jobId );
    if ( !Execute( "GET", PackageFileUrl( ci, version, CForgejoReportLayout::ReportFileName ), CORE::CString::Empty, MaxJsonBytes, JsonTimeoutInMs, status, body, error ) )
        return false;
    if ( 404 == status )
    {
        missing = true;
        return false;
    }
    if ( 200 != status )
    {
        error = "the report could not be fetched (HTTP " + CORE::Int32ToString( status ) + ")";
        return false;
    }
    CORE::CDataNode report;
    if ( !CCiUtils::ParseJson( body, report ) )
    {
        error = "the report is not JSON";
        return false;
    }
    return cell.LoadFromReport( report );
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::GetReport( const CCiDefinition& ci         ,
                            const CORE::CString& runId      ,
                            CCiReport& report               ,
                            bool& isUnknown                 ,
                            CORE::CString& error            )
{GUCEF_TRACE;

    report = CCiReport();
    if ( !LoadRun( ci, runId, report.status, isUnknown, error ) )
        return false;
    for ( size_t i=0; i<report.status.jobs.size(); ++i )
    {
        const CCiRunStatus::CJob& job = report.status.jobs[ i ];
        CCiReport::CCell cell;
        bool missing = false;
        CORE::CString problem;
        if ( !LoadCellReport( ci, runId, job.id, cell, missing, problem ) && !missing )
            cell.problem = "the report could not be read: " + problem;
        cell.jobId = job.id;
        cell.state = job.state;
        report.cells.push_back( cell );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::LoadLog( const CCiDefinition& ci, const CORE::CString& version, const CORE::CString& fileName,
                          CORE::CString& content, CORE::CString& note, CORE::CString& problem ) const
{GUCEF_TRACE;

    Int32 status = 0;
    CORE::CDynamicBuffer body;
    CORE::CString error;
    if ( !Execute( "GET", PackageFileUrl( ci, version, fileName ), CORE::CString::Empty, MaxLogBytes, LogTimeoutInMs, status, body, error ) )
    {
        problem = error;
        return false;
    }
    if ( 200 == status )
    {
        if ( !CForgejoReportLayout::IsCompressed( fileName ) )
        {
            content = BufferToString( body );
            return true;
        }
        CORE::CDynamicBuffer raw;
        CORE::CString gzipProblem;
        if ( Gunzip( body, MaxLogBytes, raw, gzipProblem ) )
        {
            content = BufferToString( raw );
            return true;
        }
        note = fileName + " could not be expanded (" + gzipProblem + "); the uncompressed log was asked for instead.";
    }
    else
    if ( 404 != status )
    {
        problem = "the log could not be fetched (HTTP " + CORE::Int32ToString( status ) + ")";
        return false;
    }

    const CORE::CString plain = CForgejoReportLayout::PlainLogFileName( fileName );
    if ( plain == fileName )
    {
        problem = "the job published no log " + fileName;
        return false;
    }
    if ( !Execute( "GET", PackageFileUrl( ci, version, plain ), CORE::CString::Empty, MaxLogBytes, LogTimeoutInMs, status, body, error ) )
    {
        problem = error;
        return false;
    }
    if ( 200 != status )
    {
        problem = 404 == status ? "the job published no log " + fileName + " or " + plain : "the log could not be fetched (HTTP " + CORE::Int32ToString( status ) + ")";
        return false;
    }
    content = BufferToString( body );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::GetJobDetail( const CCiDefinition& ci             ,
                               const CORE::CString& runId          ,
                               const CORE::CString& jobId          ,
                               const CORE::CString& product        ,
                               CCiJobDetail& detail                ,
                               bool& isUnknown                     ,
                               CORE::CString& error                )
{GUCEF_TRACE;

    detail = CCiJobDetail();
    detail.jobId = jobId;
    CCiRunStatus status;
    if ( !LoadRun( ci, runId, status, isUnknown, error ) )
        return false;
    bool found = false;
    for ( size_t i=0; i<status.jobs.size() && !found; ++i )
    {
        if ( status.jobs[ i ].id == jobId )
        {
            found = true;
            detail.state = status.jobs[ i ].state;
        }
    }
    if ( !found )
    {
        CORE::CString::StringVector ids;
        for ( size_t i=0; i<status.jobs.size(); ++i )
            ids.push_back( status.jobs[ i ].id );
        isUnknown = true;
        error = "Unknown job id \"" + jobId + "\". The jobs of the run: " + ( ids.empty() ? CORE::CString( "none yet" ) : CCiUtils::Join( ids, ", " ) );
        return false;
    }

    bool missing = false;
    CORE::CString problem;
    if ( !LoadCellReport( ci, runId, jobId, detail.cell, missing, problem ) )
    {
        detail.cell.jobId = jobId;
        detail.cell.state = detail.state;
        detail.note = missing ? CORE::CString( "The job published no report: it did not get to building (see its state), or it is still running." )
                              : "The report of the job could not be read: " + problem;
        return true;
    }
    detail.cell.state = detail.state;

    // the logs to fetch: of the product asked for (or of the product of the test application asked for, which holds
    // its output: test applications have no log file of their own), else of the products that failed
    const CORE::CString version = CForgejoReportLayout::MakeVersion( runId, jobId );
    const CCiReport::CProduct* productOfTest = GUCEF_NULL;
    const bool isTest = !product.IsNULLOrEmpty() && GUCEF_NULL != detail.cell.FindTest( product, &productOfTest );
    CCiJobDetail::TLogVector wanted;
    for ( size_t i=0; i<detail.cell.products.size(); ++i )
    {
        const CCiReport::CProduct& entry = detail.cell.products[ i ];
        const bool asked = product.IsNULLOrEmpty() ? entry.IsFailed() : ( isTest ? &entry == productOfTest : entry.name == product );
        if ( asked )
        {
            CCiJobDetail::CLog log;
            log.name = entry.name;
            log.fileName = entry.logFile.IsNULLOrEmpty() ? CForgejoReportLayout::DefaultLogFileName( entry.name ) : entry.logFile;
            wanted.push_back( log );
        }
    }
    if ( !product.IsNULLOrEmpty() && !isTest && wanted.empty() )
    {
        CCiJobDetail::CLog log;
        log.name = product;
        log.fileName = CForgejoReportLayout::DefaultLogFileName( product );
        wanted.push_back( log );
    }
    if ( wanted.size() > MaxLogsPerJob )
    {
        detail.note += "Only the logs of the first " + CORE::UInt32ToString( MaxLogsPerJob ) + " failures are fetched; name a product for another.\n";
        wanted.resize( MaxLogsPerJob );
    }

    for ( size_t i=0; i<wanted.size(); ++i )
    {
        CCiJobDetail::CLog& log = wanted[ i ];
        CORE::CString note;
        LoadLog( ci, version, log.fileName, log.content, note, log.problem );
        if ( !note.IsNULLOrEmpty() )
            detail.note += note + "\n";
        detail.logs.push_back( log );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::Cancel( const CCiDefinition& ci         ,
                         const CORE::CString& runId      ,
                         CORE::CString& message          ,
                         bool& isUnknown                 ,
                         CORE::CString& error            )
{GUCEF_TRACE;

    CCiRunStatus status;
    if ( !LoadRun( ci, runId, status, isUnknown, error ) )
        return false;
    if ( status.IsFinished() )
    {
        message = "The run is already finished (" + CORE::CString( CCiRunStatus::StateToString( status.state ) ) + "); there is nothing to cancel.";
        return true;
    }

    Int32 httpStatus = 0;
    CORE::CDataNode answer;
    if ( !ExecuteJson( "POST", ScratchApi( ci ) + "/actions/runs/" + runId + CancelPathSuffix, CORE::CString::Empty, httpStatus, answer, error ) )
        return false;
    if ( httpStatus >= 200 && httpStatus < 300 )
    {
        message = "Cancelling the run was asked for; ci_status shows when it has stopped.";
        return true;
    }
    if ( 404 == httpStatus || 405 == httpStatus || 501 == httpStatus )
    {
        error = "This CI cannot cancel a run through its interface (HTTP " + CORE::Int32ToString( httpStatus ) + "): the run goes on until it is " +
                "finished. Ignore its result, or submit again.";
        return false;
    }
    error = "The CI did not cancel the run (HTTP " + CORE::Int32ToString( httpStatus ) + ")" + ServerMessage( answer );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoBackend::Cleanup( const CCiDefinition& ci         ,
                          UInt32 olderThanDays            ,
                          CORE::CString& summary          ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    const Int64 cutoff = CCiUtils::NowUnix() - (Int64) olderThanDays * 86400;

    // the scratch branches: their age is in their name, or else in their commit
    CORE::CString::StringVector oldBranches;
    UInt32 keptBranches = 0;
    for ( UInt32 page=1; page<=MaxListPages; ++page )
    {
        Int32 status = 0;
        CORE::CDataNode answer;
        const CORE::CString url = ScratchApi( ci ) + "/branches?page=" + CORE::UInt32ToString( page ) + "&limit=" + CORE::UInt32ToString( PageSize );
        if ( !ExecuteJson( "GET", url, CORE::CString::Empty, status, answer, error ) )
            return false;
        if ( 200 != status )
        {
            error = "The CI did not list the scratch branches (HTTP " + CORE::Int32ToString( status ) + ")" + ServerMessage( answer );
            return false;
        }
        const UInt32 count = answer.GetNrOfChildNodes();
        for ( CORE::CDataNode::const_iterator i = answer.ConstBegin(); i != answer.ConstEnd(); ++i )
        {
            const CORE::CString name = (*i)->GetAttributeValue( "name" ).AsString();
            if ( !name.StartsWith( CSnapshotBuilder::BranchPrefix ) )
                continue;
            Int64 when = 0;
            bool known = false;
            CORE::CString::StringVector parts = name.ParseElements( '-', false );
            for ( size_t p=0; p<parts.size() && !known; ++p )
                known = CCiUtils::ParseCompactUtcToUnix( parts[ p ], when );
            if ( !known )
            {
                const CORE::CDataNode* commit = (*i)->FindChild( "commit" );
                known = GUCEF_NULL != commit && TimeAttribute( *commit, "timestamp", when );
            }
            if ( known && when < cutoff )
                oldBranches.push_back( name );
            else
                ++keptBranches;
        }
        if ( count < PageSize )
            break;
    }

    // the reports
    CORE::CString::StringVector oldReports;
    for ( UInt32 page=1; page<=MaxListPages; ++page )
    {
        Int32 status = 0;
        CORE::CDataNode answer;
        const CORE::CString url = CForgejoReportLayout::ListUrl( ci.GetBaseUrl(), GetReportsOwner( ci ), page, PageSize );
        if ( !ExecuteJson( "GET", url, CORE::CString::Empty, status, answer, error ) )
            return false;
        if ( 200 != status )
            break;          // without a package registry there are no reports to remove
        const UInt32 count = answer.GetNrOfChildNodes();
        for ( CORE::CDataNode::const_iterator i = answer.ConstBegin(); i != answer.ConstEnd(); ++i )
        {
            if ( (*i)->GetAttributeValue( "name" ).AsString() != CForgejoReportLayout::PackageName )
                continue;
            Int64 when = 0;
            if ( TimeAttribute( *(*i), "created_at", when ) && when < cutoff )
                oldReports.push_back( (*i)->GetAttributeValue( "version" ).AsString() );
        }
        if ( count < PageSize )
            break;
    }

    UInt32 deletedBranches = 0;
    UInt32 deletedReports = 0;
    UInt32 failures = 0;
    for ( size_t i=0; i<oldBranches.size(); ++i )
    {
        Int32 status = 0;
        CORE::CDynamicBuffer body;
        CORE::CString problem;
        if ( Execute( "DELETE", ScratchApi( ci ) + "/branches/" + CCiUtils::PercentEncodePath( oldBranches[ i ] ), CORE::CString::Empty, MaxJsonBytes, JsonTimeoutInMs, status, body, problem ) &&
             status >= 200 && status < 300 )
            ++deletedBranches;
        else
            ++failures;
    }
    for ( size_t i=0; i<oldReports.size(); ++i )
    {
        Int32 status = 0;
        CORE::CDynamicBuffer body;
        CORE::CString problem;
        const CORE::CString url = CForgejoReportLayout::VersionUrl( ci.GetBaseUrl(), GetReportsOwner( ci ), oldReports[ i ] );
        if ( Execute( "DELETE", url, CORE::CString::Empty, MaxJsonBytes, JsonTimeoutInMs, status, body, problem ) && status >= 200 && status < 300 )
            ++deletedReports;
        else
            ++failures;
    }

    summary = "Removed " + CORE::UInt32ToString( deletedBranches ) + " scratch branch(es) and " + CORE::UInt32ToString( deletedReports ) +
              " report(s) older than " + CORE::UInt32ToString( olderThanDays ) + " day(s); kept " + CORE::UInt32ToString( keptBranches ) + " newer scratch branch(es).";
    if ( failures > 0 )
        summary += " " + CORE::UInt32ToString( failures ) + " could not be removed.";
    return true;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
