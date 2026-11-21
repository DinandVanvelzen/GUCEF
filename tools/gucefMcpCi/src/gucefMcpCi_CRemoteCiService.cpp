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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

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
#include "gucefMcpCi_CCiJson.h"
#include "gucefMcpCi_CDirectCiService.h"
#include "gucefMcpCi_CRemoteCiService.h"

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

static const UInt32 JsonTimeoutInMs = 60000;
static const UInt32 ProbeTimeoutInMs = 5000;
static const UInt32 ChunkTimeoutInMs = 300000;
static const UInt32 CommitTimeoutInMs = 1800000;          /**< the gateway may fetch and push a large history */
static const UInt32 StatusMarginInMs = 60000;
static const UInt32 DetailTimeoutInMs = 300000;
static const UInt32 DeployTimeoutInMs = 3900000;            /**< the longest rollout the gateway waits for, and some */
static const UInt64 MaxJsonAnswerBytes = (UInt64) 8 * 1024 * 1024;
static const UInt64 MaxDetailAnswerBytes = (UInt64) 96 * 1024 * 1024;
static const UInt32 MaxChunkTries = 5;
static const UInt32 MaxBaseCommits = 16;
static const UInt32 GitTimeoutInMs = 120000;
static const UInt32 HashStepBytes = 1024 * 1024;
static const UInt32 DefaultChunkBytes = 8 * 1024 * 1024;
static const char* UserAgent = "gucefMcpCi/1.0";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Removes a file when it goes out of scope */
class CFileRemover
{
    public:

    explicit CFileRemover( const CORE::CString& path ) : m_path( path ) {}

    ~CFileRemover()
    {
        if ( !m_path.IsNULLOrEmpty() && CORE::FileExists( m_path ) )
            CORE::DeleteFile( m_path );
    }

    private:

    CORE::CString m_path;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
Encode( const CORE::CString& text )
{
    return CCiUtils::PercentEncode( text );
}

/*-------------------------------------------------------------------------*/

/** The SHA-256 of a file as 64 lower case hexadecimal digits, read in steps; empty when it cannot be read */
static CORE::CString
FileSha256( const CORE::CString& path )
{
    CORE::CFileAccess file;
    if ( !file.Open( path, "rb" ) || !file.Opened() )
        return CORE::CString::Empty;
    CORE::CSha256 hasher;
    CORE::CDynamicBuffer buffer( HashStepBytes, true );
    for ( ;; )
    {
        const UInt32 got = file.Read( buffer.GetBufferPtr(), 1, HashStepBytes );
        if ( got > 0 )
            hasher.Update( buffer.GetConstBufferPtr(), got );
        if ( got < HashStepBytes )
            break;
    }
    file.Close();
    UInt8 digest[ CORE::CSha256::DigestSize ];
    hasher.Final( digest );
    static const char* digits = "0123456789abcdef";
    CORE::CString text;
    for ( UInt32 i=0; i<CORE::CSha256::DigestSize; ++i )
    {
        text += digits[ digest[ i ] >> 4 ];
        text += digits[ digest[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/** The top level directory of the working tree that holds 'repoDir' */
static bool
TopLevel( const CORE::CString& repoDir, CORE::CString& topLevel, CORE::CString& error )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "rev-parse" );
    args.push_back( "--show-toplevel" );
    CORE::CString output;
    if ( !CORE::DirExists( repoDir ) || !CCiUtils::RunGit( repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "repo_dir is not a git working tree: " + repoDir;
        return false;
    }
    CORE::CString::StringVector lines = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    topLevel = lines.empty() ? repoDir : CCiUtils::Trimmed( lines[ lines.size()-1 ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteCiService::CRemoteCiService( const CSnapshotBuilder& snapshotBuilder )
    : ICiService()
    , m_snapshotBuilder( snapshotBuilder )
    , m_url()
    , m_token()
    , m_caFile()
    , m_machineName()
    , m_workDir()
    , m_connectTimeoutInMs( 10000 )
    , m_ciIds()
    , m_bundleCounter( 0 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteCiService::~CRemoteCiService()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteCiService::SetGateway( const CORE::CString& url, const CORE::CString& token, const CORE::CString& caFile )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_url = url;
    while ( m_url.EndsWith( "/" ) )
        m_url = m_url.CutChars( 1, false );
    m_token = token;
    m_caFile = caFile;
    if ( !m_token.IsNULLOrEmpty() )
        WEB::CHttpLogRedaction::AddSecret( m_token );
}

/*-------------------------------------------------------------------------*/

void
CRemoteCiService::SetMachineName( const CORE::CString& machineName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_machineName = machineName;
}

/*-------------------------------------------------------------------------*/

void
CRemoteCiService::SetWorkDir( const CORE::CString& workDir )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_workDir = workDir;
}

/*-------------------------------------------------------------------------*/

void
CRemoteCiService::SetConnectTimeout( UInt32 timeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_connectTimeoutInMs = timeoutInMs > 0 ? timeoutInMs : 10000;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteCiService::GetUrl( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_url;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteCiService::ListCiIds( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_ciIds;
}

/*-------------------------------------------------------------------------*/

void
CRemoteCiService::RememberIds( const CORE::CDataNode& json ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == json.FindChild( "ids" ) )
        return;
    const CORE::CString ids = CCiUtils::Join( CCiUtils::GetStringArray( json, "ids" ), ", " );
    MT::CScopeMutex lock( m_lock );
    m_ciIds = ids;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Execute( const CORE::CString& method             ,
                           const CORE::CString& pathAndQuery       ,
                           const CORE::CDynamicBuffer* body        ,
                           const char* contentType                 ,
                           UInt32 timeoutInMs                      ,
                           UInt64 maxAnswerBytes                   ,
                           Int32& status                           ,
                           CORE::CDynamicBuffer& answer            ,
                           CORE::CString& error                    ) const
{GUCEF_TRACE;

    CORE::CString url;
    CORE::CString token;
    CORE::CString caFile;
    UInt32 connectTimeout = 0;
    {
        MT::CScopeMutex lock( m_lock );
        url = m_url;
        token = m_token;
        caFile = m_caFile;
        connectTimeout = m_connectTimeoutInMs;
    }
    status = 0;
    answer.Clear();
    WEB::CHttpRequestData request;
    request.requestType = method;
    if ( url.IsNULLOrEmpty() || !request.SetUrl( url + pathAndQuery ) )
    {
        error = "The address of the CI gateway is not a valid URL: " + url;
        return false;
    }
    request.otherHeaders[ "Authorization" ] = "Bearer " + token;
    request.resourceRepresentations.push_back( "application/json" );
    if ( GUCEF_NULL != body )
    {
        request.contentRepresentation = GUCEF_NULL != contentType ? contentType : "application/json";
        request.content = *body;
    }

    WEB::CHttpClientRequestOptions options;
    options.maxResponseBodySize = maxAnswerBytes;
    options.connectTimeoutInMs = connectTimeout < timeoutInMs ? connectTimeout : timeoutInMs;
    options.totalTimeoutInMs = timeoutInMs;
    options.verifyPeer = true;
    options.caBundlePath = caFile;
    options.followRedirects = false;            // the token is never sent elsewhere
    options.useEnvironmentProxy = false;
    options.userAgent = UserAgent;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo transferInfo;
    if ( !client.Execute( request, options, response, &transferInfo ) )
    {
        if ( transferInfo.responseBodyTooLarge )
            error = "The answer of the CI gateway is larger than " + CCiUtils::FormatBytes( maxAnswerBytes );
        else
            error = "The CI gateway at " + url + " cannot be reached (" + CORE::CString( WEB::HttpTransferErrorClassToString( transferInfo.errorClass ) ) + ": " +
                    WEB::CHttpLogRedaction::RedactText( transferInfo.errorDescription ) + "). The CI tools work again once it can be reached.";
        return false;
    }
    status = response.statusCode;
    answer = response.content;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::CallJson( const CORE::CString& method             ,
                            const CORE::CString& pathAndQuery       ,
                            const CORE::CString& jsonBody           ,
                            UInt32 timeoutInMs                      ,
                            UInt64 maxAnswerBytes                   ,
                            CORE::CDataNode& json                   ,
                            CORE::CString& error                    ,
                            Int32* statusOut                        ) const
{GUCEF_TRACE;

    CORE::CDynamicBuffer body;
    if ( !jsonBody.IsNULLOrEmpty() )
        body.CopyFrom( jsonBody.ByteSize() - 1, jsonBody.C_String() );
    Int32 status = 0;
    CORE::CDynamicBuffer answer;
    const bool answered = Execute( method, pathAndQuery, jsonBody.IsNULLOrEmpty() ? GUCEF_NULL : &body, "application/json", timeoutInMs, maxAnswerBytes, status, answer, error );
    if ( GUCEF_NULL != statusOut )
        *statusOut = answered ? status : 0;
    if ( !answered )
        return false;

    json.Clear();
    const bool isJson = answer.GetDataSize() > 0 && CCiUtils::ParseJson( answer, json ) && GUCEF_DATATYPE_OBJECT == json.GetNodeType();
    if ( status >= 200 && status < 300 )
    {
        if ( isJson )
            return true;
        error = "The CI gateway at " + GetUrl() + " answered with something that is not its API (HTTP " + CORE::Int32ToString( status ) + ")";
        return false;
    }
    const CORE::CString message = isJson ? CCiUtils::Trimmed( json.GetAttributeValue( "error" ).AsString() ) : CORE::CString::Empty;
    const CORE::CString code = isJson ? CCiUtils::Trimmed( json.GetAttributeValue( "code" ).AsString() ) : CORE::CString::Empty;
    if ( 403 == status && !message.IsNULLOrEmpty() && ( code == "scope" || code == "not_listed" ) )
        error = message;            // the token was taken, what it asked for is not allowed
    else
    if ( 401 == status || 403 == status )
        error = "The CI gateway refused the token of this machine (HTTP " + CORE::Int32ToString( status ) + "): the machine may have been revoked, "
                "or lablink did not renew its token yet";
    else
    if ( !message.IsNULLOrEmpty() )
        error = message;
    else
        error = "The CI gateway at " + GetUrl() + " answered HTTP " + CORE::Int32ToString( status ) + " without an answer of its API";
    if ( 503 == status || 429 == status )
        error = "The CI gateway is busy: " + error;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Probe( CORE::CString& ciIds, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDataNode json;
    // short: the server waits for this before it serves its client
    if ( !CallJson( "GET", "/v1/ci", CORE::CString::Empty, ProbeTimeoutInMs, MaxJsonAnswerBytes, json, problem ) )
        return false;
    RememberIds( json );
    ciIds = ListCiIds();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Resolve( const CORE::CString& repoDir                     ,
                           const CORE::CString::StringVector& remotes       ,
                           CORE::CString& ciId                              ,
                           CORE::CString& description                       ,
                           CORE::CString& matchedRemote                     ,
                           CORE::CString::StringVector& examinedRemotes     ,
                           CORE::CString& error                             )
{GUCEF_TRACE;

    examinedRemotes.clear();
    if ( !repoDir.IsNULLOrEmpty() )
    {
        if ( !CORE::DirExists( repoDir ) )
        {
            error = "repo_dir is not an existing directory: " + repoDir;
            return false;
        }
        if ( !CDirectCiService::GetRemotes( repoDir, examinedRemotes, error ) )
            return false;
    }
    for ( size_t i=0; i<remotes.size(); ++i )
        examinedRemotes.push_back( remotes[ i ] );
    if ( examinedRemotes.empty() )
    {
        error = repoDir.IsNULLOrEmpty() ? CORE::CString( "Give repo_dir or remotes" ) : "The repository has no remotes, so no CI can be set up for it: " + repoDir;
        return false;
    }

    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    CCiJson::WriteStrings( examinedRemotes, "remotes", body );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    CORE::CDataNode json;
    if ( !CallJson( "POST", "/v1/resolve", bodyText, JsonTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    ciId = json.GetAttributeValue( "ciId" ).AsString();
    description = json.GetAttributeValue( "description" ).AsString();
    matchedRemote = json.GetAttributeValue( "matchedRemote" ).AsString();
    if ( ciId.IsNULLOrEmpty() )
    {
        error = "The CI gateway at " + GetUrl() + " answered without a CI id";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Describe( const CORE::CString& ciId     ,
                            const CORE::CString& repoDir  ,
                            CCiDescription& description   ,
                            CORE::CString& error          )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/ci?id=" + Encode( ciId ), CORE::CString::Empty, JsonTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    const CORE::CDataNode* ci = json.FindChild( "ci" );
    if ( GUCEF_NULL == ci )
    {
        error = "The CI gateway at " + GetUrl() + " answered without the options of " + ciId;
        return false;
    }
    CCiJson::Read( *ci, description );

    // the CI files of a working tree of this machine, which the gateway cannot see: a snapshot of it runs with them
    const CORE::CString matrixDir = json.GetAttributeValue( "matrixDir" ).AsString();
    if ( !repoDir.IsNULLOrEmpty() )
    {
        CORE::CString topLevel;
        if ( !TopLevel( repoDir, topLevel, error ) )
            return false;
        if ( !matrixDir.IsNULLOrEmpty() )
        {
            CCiDescription local;
            local.ciId = description.ciId;
            bool found = false;
            if ( !local.LoadFromMatrixFiles( CORE::CombinePath( topLevel, matrixDir ), found, error ) )
                return false;
            if ( found )
            {
                local.source = "the CI files of the repository (" + matrixDir + ")";
                description = local;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteCiService::MakeBundlePath( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const CORE::CString dir = CORE::CombinePath( m_workDir.IsNULLOrEmpty() ? CORE::CombinePath( CORE::TempDir(), "gucefMcpCi" ) : m_workDir, "bundles" );
    CORE::CreateDirs( dir );
    return CORE::CombinePath( dir, "snapshot-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-" + CORE::UInt32ToString( ++m_bundleCounter ) + ".bundle" );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CRemoteCiService::BundleBase( const CCiSnapshot& snapshot, const CORE::CString::StringVector& have )
{GUCEF_TRACE;

    // only what this repository has can be left out, and only what the snapshot builds on is worth leaving out
    CORE::CString::StringVector base;
    for ( size_t i=0; i<have.size() && i<MaxBaseCommits; ++i )
    {
        const CORE::CString& commit = have[ i ];
        if ( !CSnapshotBuilder::HasCommit( snapshot.repoDir, commit ) )
            continue;
        CORE::CString output;
        if ( commit == snapshot.commit )
        {
            // the gateway has the commit itself: the bundle holds just that commit, on top of its parents
            CORE::CProcess::TStringVector args;
            args.push_back( "rev-parse" );
            args.push_back( snapshot.commit + "^@" );
            if ( CCiUtils::RunGit( snapshot.repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
            {
                CORE::CString::StringVector parents = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
                for ( size_t p=0; p<parents.size(); ++p )
                    if ( CCiUtils::IsHex( CCiUtils::Trimmed( parents[ p ] ), 40 ) )
                        base.push_back( CCiUtils::Trimmed( parents[ p ] ) );
            }
            continue;
        }
        CORE::CProcess::TStringVector args;
        args.push_back( "merge-base" );
        args.push_back( "--is-ancestor" );
        args.push_back( commit );
        args.push_back( snapshot.commit );
        if ( CCiUtils::RunGit( snapshot.repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
            base.push_back( commit );
    }
    return base;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Upload( const CORE::CString& ciId          ,
                          const CORE::CString& bundlePath    ,
                          ICiProgress* progress              ,
                          CORE::CString& uploadId            ,
                          CORE::CString& error               ) const
{GUCEF_TRACE;

    const UInt64 size = CORE::FileSize( bundlePath );
    const CORE::CString sha256 = FileSha256( bundlePath );
    if ( 0 == size || sha256.IsNULLOrEmpty() )
    {
        error = "The snapshot bundle cannot be read: " + bundlePath;
        return false;
    }

    CORE::CDataNode json;
    if ( !CallJson( "POST", "/v1/uploads?ci=" + Encode( ciId ) + "&size=" + CORE::UInt64ToString( size ) + "&sha256=" + sha256, "{}",
                    JsonTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    uploadId = CCiUtils::Trimmed( json.GetAttributeValue( "id" ).AsString() );
    UInt32 chunkBytes = json.GetAttributeValue( "chunkBytes" ).AsUInt32( DefaultChunkBytes );
    if ( 0 == chunkBytes || chunkBytes > DefaultChunkBytes )
        chunkBytes = DefaultChunkBytes;
    if ( uploadId.IsNULLOrEmpty() )
    {
        error = "The CI gateway at " + GetUrl() + " did not start the upload";
        return false;
    }

    CORE::CFileAccess file;
    if ( !file.Open( bundlePath, "rb" ) || !file.Opened() )
    {
        error = "The snapshot bundle cannot be read: " + bundlePath;
        return false;
    }
    const CORE::CString uploadPath = "/v1/uploads?id=" + Encode( uploadId );
    UInt64 offset = 0;
    UInt32 failures = 0;
    CORE::CDynamicBuffer chunk;
    CORE::CString lastProblem;
    while ( offset < size )
    {
        if ( GUCEF_NULL != progress && progress->IsCancelled() )
        {
            file.Close();
            CORE::CString ignored;
            CallJson( "DELETE", uploadPath, CORE::CString::Empty, JsonTimeoutInMs, MaxJsonAnswerBytes, json, ignored );
            error = "Cancelled while sending the snapshot";
            return false;
        }
        const UInt32 step = (UInt32) ( size - offset < chunkBytes ? size - offset : chunkBytes );
        chunk.SetDataSize( 0 );
        chunk.SetBufferSize( step, false );
        UInt32 got = 0;
        if ( 0 == file.Setpos( offset ) )
            got = file.Read( chunk.GetBufferPtr(), 1, step );
        if ( got != step )
        {
            file.Close();
            error = "The snapshot bundle cannot be read: " + bundlePath;
            return false;
        }
        chunk.SetDataSize( step );

        Int32 status = 0;
        CORE::CDynamicBuffer answer;
        CORE::CString problem;
        const bool answered = Execute( "PUT", uploadPath + "&offset=" + CORE::UInt64ToString( offset ), &chunk, "application/octet-stream",
                                       ChunkTimeoutInMs, MaxJsonAnswerBytes, status, answer, problem );
        CORE::CDataNode result;
        const bool isJson = answered && answer.GetDataSize() > 0 && CCiUtils::ParseJson( answer, result );
        if ( answered && 200 == status && isJson )
        {
            offset = result.GetAttributeValue( "received" ).AsUInt64( offset + step );
            failures = 0;
            if ( GUCEF_NULL != progress )
                progress->OnProgress( (Float64) offset, (Float64) size, "sending the snapshot, " + CCiUtils::FormatBytes( offset ) + " of " + CCiUtils::FormatBytes( size ) );
            continue;
        }

        // refusals that another try does not change
        if ( answered && ( 400 == status || 401 == status || 403 == status || 404 == status || 413 == status ) )
        {
            file.Close();
            if ( isJson && !result.GetAttributeValue( "error" ).AsString().IsNULLOrEmpty() )
                error = "Sending the snapshot failed: " + result.GetAttributeValue( "error" ).AsString();
            else
                error = "Sending the snapshot failed (HTTP " + CORE::Int32ToString( status ) + ")";
            return false;
        }
        lastProblem = answered ? "HTTP " + CORE::Int32ToString( status ) : problem;
        if ( ++failures >= MaxChunkTries )
            break;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: A chunk of the snapshot failed (" << lastProblem << "), resuming from where the gateway is" );
        MT::ThreadDelay( 250 * failures );

        // where the gateway is: the chunk may have arrived even when its answer did not
        CORE::CDataNode state;
        CORE::CString stateProblem;
        if ( isJson && 409 == status && GUCEF_NULL != result.FindChild( "received" ) )
            offset = result.GetAttributeValue( "received" ).AsUInt64( offset );
        else
        if ( CallJson( "GET", uploadPath, CORE::CString::Empty, JsonTimeoutInMs, MaxJsonAnswerBytes, state, stateProblem ) )
            offset = state.GetAttributeValue( "received" ).AsUInt64( offset );
    }
    file.Close();
    if ( offset < size )
    {
        error = "Sending the snapshot failed after " + CORE::UInt32ToString( MaxChunkTries ) + " tries: " + lastProblem;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Submit( const CORE::CString& ciId       ,
                          const CORE::CString& repoDir    ,
                          const CCiSubmitOptions& options ,
                          CCiSnapshot& snapshot           ,
                          CORE::CString& runId            ,
                          CORE::CString& error            ,
                          ICiProgress* progress           )
{GUCEF_TRACE;

    if ( !options.commit.IsNULLOrEmpty() )
        return SubmitCommit( ciId, options, snapshot, runId, error, progress );
    if ( repoDir.IsNULLOrEmpty() || !CORE::DirExists( repoDir ) )
    {
        error = "repo_dir is not an existing directory: " + repoDir;
        return false;
    }

    // what the gateway knows of the CI id; an unknown id is refused there
    CORE::CDataNode info;
    if ( !CallJson( "GET", "/v1/ci?id=" + Encode( ciId ), CORE::CString::Empty, JsonTimeoutInMs, MaxJsonAnswerBytes, info, error ) )
        return false;

    // only a checkout of the CI id is sent there
    CORE::CString::StringVector remotes;
    if ( !CDirectCiService::GetRemotes( repoDir, remotes, error ) )
        return false;
    CORE::CString ciOfRepo;
    CORE::CString problem;
    if ( !remotes.empty() )
    {
        CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
        CCiJson::WriteStrings( remotes, "remotes", body );
        CORE::CString bodyText;
        CCiUtils::ToJson( body, bodyText );
        CORE::CDataNode json;
        Int32 status = 0;
        if ( CallJson( "POST", "/v1/resolve", bodyText, JsonTimeoutInMs, MaxJsonAnswerBytes, json, problem, &status ) )
            ciOfRepo = json.GetAttributeValue( "ciId" ).AsString();
        else
        if ( 404 != status )
        {
            error = problem;
            return false;
        }
    }
    if ( ciOfRepo != ciId )
    {
        error = "repo_dir is not a checkout of CI id " + ciId + " (its remotes: " + ( remotes.empty() ? CORE::CString( "none" ) : CCiUtils::Join( remotes, ", " ) ) +
                "). " + ( ciOfRepo.IsNULLOrEmpty() ? CORE::CString( "No CI is set up for it." ) : "Its CI id is " + ciOfRepo + "." );
        return false;
    }

    // the defaults of the CI id, then the mode and the filters against the CI files of this working tree
    CCiSubmitOptions effective = options;
    const CORE::CDataNode* defaults = info.FindChild( "defaults" );
    if ( GUCEF_NULL != defaults )
    {
        if ( effective.mode.IsNULLOrEmpty() )
            effective.mode = defaults->GetAttributeValue( "mode" ).AsString();
        if ( effective.toolchains.empty() )
            effective.toolchains = CCiUtils::GetStringArray( *defaults, "toolchains" );
        if ( effective.cppStandards.empty() )
            effective.cppStandards = CCiUtils::GetStringArray( *defaults, "cppStandards" );
        if ( effective.products.empty() )
            effective.products = CCiUtils::GetStringArray( *defaults, "products" );
    }
    if ( effective.mode.IsNULLOrEmpty() )
        effective.mode = "build";
    effective.mode = effective.mode.Lowercase();
    if ( effective.mode != "build" && effective.mode != "test" )
    {
        error = "mode \"" + effective.mode + "\" is not known: build or test";
        return false;
    }
    if ( !effective.toolchains.empty() || !effective.cppStandards.empty() || !effective.products.empty() )
    {
        CCiDescription description;
        if ( Describe( ciId, repoDir, description, problem ) && description.hasOptions && !description.CheckFilters( effective, ciId, error ) )
            return false;
    }

    CORE::CString machineName;
    {
        MT::CScopeMutex lock( m_lock );
        machineName = m_machineName;
    }
    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, "making the snapshot" );
    if ( !m_snapshotBuilder.Build( repoDir, effective.includeUncommitted, machineName, snapshot, error ) )
        return false;
    if ( effective.dryRun )
        return true;
    if ( GUCEF_NULL != progress && progress->IsCancelled() )
    {
        error = "Cancelled: the snapshot was made but not sent";
        return false;
    }

    // a bundle against what the gateway has, else with the whole history
    const CORE::CString::StringVector have = CCiUtils::GetStringArray( info, "have" );
    const CORE::CString::StringVector base = BundleBase( snapshot, have );
    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, base.empty() ? CORE::CString( "packing the snapshot with its whole history" ) : CORE::CString( "packing the snapshot" ) );
    const CORE::CString bundlePath = MakeBundlePath();
    CFileRemover removeBundle( bundlePath );
    if ( !m_snapshotBuilder.CreateBundle( snapshot, base, bundlePath, error ) )
        return false;
    const UInt64 bundleSize = CORE::FileSize( bundlePath );
    const CORE::CDataNode* limits = info.FindChild( "limits" );
    const UInt64 maxBytes = GUCEF_NULL != limits ? limits->GetAttributeValue( "maxSnapshotBytes" ).AsUInt64( 0 ) : 0;
    if ( maxBytes > 0 && bundleSize > maxBytes )
    {
        error = "The snapshot is " + CCiUtils::FormatBytes( bundleSize ) + " to send, more than the limit of the gateway of " + CCiUtils::FormatBytes( maxBytes ) +
                ". Large files that are not meant for the CI belong in .gitignore.\n" + snapshot.Describe();
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Sending the snapshot " << snapshot.commit << " to the gateway as a bundle of " << bundleSize << " bytes" <<
                      ( base.empty() ? " with its whole history" : CORE::CString( " against " ) + CORE::UInt32ToString( (UInt32) base.size() ) + " base commit(s)" ) );

    CORE::CString uploadId;
    if ( !Upload( ciId, bundlePath, progress, uploadId, error ) )
        return false;

    if ( GUCEF_NULL != progress )
        progress->OnProgress( (Float64) bundleSize, (Float64) bundleSize, "the CI takes the snapshot in" );
    CORE::CDataNode committed;
    if ( !CallJson( "POST", "/v1/uploads/commit?id=" + Encode( uploadId ), "{}", CommitTimeoutInMs, MaxJsonAnswerBytes, committed, error ) )
        return false;
    const CORE::CDataNode* sent = committed.FindChild( "snapshot" );
    CCiSnapshot gatewaySnapshot;
    if ( GUCEF_NULL != sent )
        CCiJson::Read( *sent, gatewaySnapshot );
    if ( gatewaySnapshot.commit != snapshot.commit )
    {
        error = "The CI gateway took in another commit than the snapshot (" + gatewaySnapshot.commit + ")";
        return false;
    }
    snapshot.branchName = gatewaySnapshot.branchName;

    if ( GUCEF_NULL != progress )
        progress->OnProgress( (Float64) bundleSize, (Float64) bundleSize, "starting the run" );
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "snapshot", committed.GetAttributeValue( "snapshotId" ).AsString( uploadId ) );
    CCiJson::Write( effective, *body.AddChild( "options", GUCEF_DATATYPE_OBJECT ) );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    CORE::CDataNode started;
    if ( !CallJson( "POST", "/v1/runs", bodyText, JsonTimeoutInMs, MaxJsonAnswerBytes, started, error ) )
        return false;
    runId = CCiUtils::Trimmed( started.GetAttributeValue( "runId" ).AsString() );
    if ( runId.IsNULLOrEmpty() )
    {
        error = "The CI gateway at " + GetUrl() + " answered without a run id";
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Run " << runId << " started for " << snapshot.repoDir << " on " << snapshot.branchName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Status( const CORE::CString& runId      ,
                          UInt32 waitSeconds              ,
                          CCiRunStatus& status            ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/runs?id=" + Encode( runId ) + "&wait=" + CORE::UInt32ToString( waitSeconds ), CORE::CString::Empty,
                    waitSeconds * 1000 + StatusMarginInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    CCiJson::Read( json, status );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Report( const CORE::CString& runId      ,
                          CCiReport& report               ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/runs/report?id=" + Encode( runId ), CORE::CString::Empty, DetailTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    CCiJson::Read( json, report );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::JobDetails( const CORE::CString& runId      ,
                              const CORE::CString& jobId      ,
                              const CORE::CString& product    ,
                              CCiJobDetail& detail            ,
                              CORE::CString& error            )
{GUCEF_TRACE;

    // the logs come whole, up to the download limit of the gateway: the tools save them here
    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/runs/jobs?id=" + Encode( runId ) + "&job=" + Encode( jobId ) + "&product=" + Encode( product ) + "&full=true",
                    CORE::CString::Empty, DetailTimeoutInMs, MaxDetailAnswerBytes, json, error ) )
        return false;
    CCiJson::Read( json, detail );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Cancel( const CORE::CString& runId      ,
                          CORE::CString& message          ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "POST", "/v1/runs/cancel?id=" + Encode( runId ), "{}", JsonTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    message = json.GetAttributeValue( "message" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Cleanup( const CORE::CString& ciId      ,
                           UInt32 olderThanDays           ,
                           CORE::CString& summary         ,
                           CORE::CString& error           )
{GUCEF_TRACE;

    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "ci", ciId );
    body.SetAttribute( "olderThanDays", olderThanDays );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    CORE::CDataNode json;
    if ( !CallJson( "POST", "/v1/cleanup", bodyText, DetailTimeoutInMs * 3, MaxJsonAnswerBytes, json, error ) )
        return false;
    summary = json.GetAttributeValue( "summary" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::SubmitCommit( const CORE::CString& ciId       ,
                                const CCiSubmitOptions& options ,
                                CCiSnapshot& snapshot           ,
                                CORE::CString& runId            ,
                                CORE::CString& error            ,
                                ICiProgress* progress           )
{GUCEF_TRACE;

    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, "the gateway sends the published commit to the CI" );
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "ci", ciId );
    CCiJson::Write( options, *body.AddChild( "options", GUCEF_DATATYPE_OBJECT ) );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    CORE::CDataNode started;
    if ( !CallJson( "POST", "/v1/runs", bodyText, CommitTimeoutInMs, MaxJsonAnswerBytes, started, error ) )
        return false;
    snapshot = CCiSnapshot();
    const CORE::CDataNode* sent = started.FindChild( "snapshot" );
    if ( GUCEF_NULL != sent )
        CCiJson::Read( *sent, snapshot );
    runId = CCiUtils::Trimmed( started.GetAttributeValue( "runId" ).AsString() );
    if ( runId.IsNULLOrEmpty() && !options.dryRun )
    {
        error = "The CI gateway at " + GetUrl() + " answered without a run id";
        return false;
    }
    if ( !runId.IsNULLOrEmpty() )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Run " << runId << " started for the published commit " << options.commit << " on " << snapshot.branchName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Images( const CORE::CString& runId      ,
                          CCiImageReport& report          ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/images?id=" + Encode( runId ), CORE::CString::Empty, JsonTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    CORE::CString problem;
    if ( !report.LoadFromJson( json, problem ) )
    {
        error = "The CI gateway at " + GetUrl() + " answered without the images of the run: " + problem;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::CallDeploy( const CORE::CString& path, const CORE::CString& bodyText, CCiDeployResult& result, CORE::CString& error )
{GUCEF_TRACE;

    result = CCiDeployResult();
    CORE::CDataNode json;
    Int32 status = 0;
    const bool answered = CallJson( "POST", path, bodyText, DeployTimeoutInMs, MaxJsonAnswerBytes, json, error, &status );
    const CORE::CDataNode* resultNode = json.FindChild( "result" );
    if ( GUCEF_NULL != resultNode && 0 != status )
        result.LoadFromJson( *resultNode );
    if ( answered && GUCEF_NULL == resultNode )
    {
        error = "The CI gateway at " + GetUrl() + " answered without the result of the call";
        return false;
    }
    return answered;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Deploy( const CORE::CString& ciId         ,
                          const CORE::CString& image        ,
                          const CORE::CString& target       ,
                          const CORE::CString& runOrCommit  ,
                          bool confirm                      ,
                          const CCiCaller& caller           ,
                          CCiDeployResult& result           ,
                          CORE::CString& error              ,
                          ICiProgress* progress             )
{GUCEF_TRACE;

    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, "the gateway deploys " + image + " to " + target );
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "ci", ciId );
    body.SetAttribute( "image", image );
    body.SetAttribute( "target", target );
    const CORE::CString given = CCiUtils::Trimmed( runOrCommit );
    body.SetAttribute( given.HasChar( ':', false ) > 0 ? "run" : "commit", given );
    body.SetAttribute( "confirm", confirm );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    return CallDeploy( "/v1/deploy", bodyText, result, error );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::ReleaseStatus( const CORE::CString& ciId     ,
                                 const CORE::CString& target   ,
                                 CCiReleaseStatus& status      ,
                                 CORE::CString& error          )
{GUCEF_TRACE;

    CORE::CDataNode json;
    if ( !CallJson( "GET", "/v1/release?ci=" + Encode( ciId ) + "&target=" + Encode( target ), CORE::CString::Empty, DetailTimeoutInMs, MaxJsonAnswerBytes, json, error ) )
        return false;
    status.LoadFromJson( json );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteCiService::Rollback( const CORE::CString& ciId       ,
                            const CORE::CString& image      ,
                            const CORE::CString& target     ,
                            bool confirm                    ,
                            const CCiCaller& caller         ,
                            CCiDeployResult& result         ,
                            CORE::CString& error            ,
                            ICiProgress* progress           )
{GUCEF_TRACE;

    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, "the gateway rolls " + target + " back" );
    CORE::CDataNode body( GUCEF_DATATYPE_OBJECT );
    body.SetAttribute( "ci", ciId );
    body.SetAttribute( "image", image );
    body.SetAttribute( "target", target );
    body.SetAttribute( "confirm", confirm );
    CORE::CString bodyText;
    CCiUtils::ToJson( body, bodyText );
    return CallDeploy( "/v1/rollback", bodyText, result, error );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
