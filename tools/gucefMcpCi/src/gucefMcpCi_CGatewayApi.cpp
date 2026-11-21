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

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiJson.h"
#include "gucefMcpCi_Tools.h"
#include "gucefMcpCi_CGatewayApi.h"

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

static const UInt64 SentSnapshotLifetimeInMs = 3600 * 1000;
static const UInt32 MaxSentSnapshots = 256;
static const UInt32 MaxDigestChars = 16000;
static const UInt32 DefaultTailLines = 200;
static const UInt32 MaxTailLines = 2000;
static const UInt32 DefaultCleanupDays = 14;

/** The routes of the API: method, exact path, whether the body is a chunk */
struct SRoute
{
    const char* method;
    const char* path;
    bool chunk;
};

static const SRoute Routes[] = { { "GET", "/v1/ci", false },
                                 { "POST", "/v1/resolve", false },
                                 { "POST", "/v1/uploads", false },
                                 { "PUT", "/v1/uploads", true },
                                 { "GET", "/v1/uploads", false },
                                 { "DELETE", "/v1/uploads", false },
                                 { "POST", "/v1/uploads/commit", false },
                                 { "POST", "/v1/runs", false },
                                 { "GET", "/v1/runs", false },
                                 { "GET", "/v1/runs/report", false },
                                 { "GET", "/v1/runs/jobs", false },
                                 { "POST", "/v1/runs/cancel", false },
                                 { "POST", "/v1/cleanup", false },
                                 { "GET", "/v1/images", false },
                                 { "POST", "/v1/deploy", false },
                                 { "GET", "/v1/release", false },
                                 { "POST", "/v1/rollback", false } };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
HexDigit( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** Percent decoding of a query value, '+' as a space */
static CORE::CString
QueryDecode( const CORE::CString& text )
{
    CORE::CString result;
    const char* c = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '+' == c[ i ] )
            result += ' ';
        else
        if ( '%' == c[ i ] && i+2 < size && HexDigit( c[ i+1 ] ) >= 0 && HexDigit( c[ i+2 ] ) >= 0 )
        {
            result += (char) ( ( HexDigit( c[ i+1 ] ) << 4 ) | HexDigit( c[ i+2 ] ) );
            i += 2;
        }
        else
            result += c[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** The end of a text of at most maxBytes bytes, from the start of a line where there is one */
static CORE::CString
TailBytes( const CORE::CString& text, UInt64 maxBytes )
{
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( (UInt64) size <= maxBytes )
        return text;
    const char* c = text.C_String();
    UInt32 start = size - (UInt32) maxBytes;
    UInt32 lineStart = start;
    while ( lineStart < size && '\n' != c[ lineStart ] )
        ++lineStart;
    if ( lineStart + 1 < size )
        start = lineStart + 1;
    return CORE::CString( c + start, size - start );
}

/*-------------------------------------------------------------------------*/

static bool
IsTrue( const CORE::CString& text )
{
    return text.Equals( "true", false ) || text == "1" || text.Equals( "yes", false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGatewayApi::CRequest::CRequest( void )
    : method()
    , path()
    , parameters()
    , body()
    , machineId()
    , caller()
    , pollDeadlineInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGatewayApi::CRequest::Parameter( const char* name ) const
{GUCEF_TRACE;

    TParameterMap::const_iterator i = parameters.find( name );
    return i == parameters.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

CGatewayApi::CResponse::CResponse( void )
    : status( 500 )
    , body()
    , pollAgain( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGatewayApi::CSentSnapshot::CSentSnapshot( void )
    : ciId()
    , machineId()
    , snapshot()
    , runId()
    , starting( false )
    , sentInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGatewayApi::CGatewayApi( CDirectCiService& service, CUploadStore& uploads, CBundleReceiver& receiver )
    : m_service( service )
    , m_uploads( uploads )
    , m_receiver( receiver )
    , m_maxWaitSeconds( 600 )
    , m_maxFullLogBytes( 24 * 1024 * 1024 )
    , m_sent()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetMaxWaitSeconds( UInt32 maxWaitSeconds )
{GUCEF_TRACE;

    m_maxWaitSeconds = maxWaitSeconds > 0 ? maxWaitSeconds : 600;
}

/*-------------------------------------------------------------------------*/

UInt32
CGatewayApi::GetMaxWaitSeconds( void ) const
{GUCEF_TRACE;

    return m_maxWaitSeconds;
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetMaxFullLogBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    m_maxFullLogBytes = maxBytes > 0 ? maxBytes : 1;
}

/*-------------------------------------------------------------------------*/

bool
CGatewayApi::FindRoute( const CORE::CString& method, const CORE::CString& path, UInt32& maxBodyBytes, bool& pathKnown )
{GUCEF_TRACE;

    pathKnown = false;
    for ( size_t i=0; i<sizeof( Routes ) / sizeof( Routes[ 0 ] ); ++i )
    {
        if ( path != Routes[ i ].path )
            continue;
        pathKnown = true;
        if ( method == Routes[ i ].method )
        {
            maxBodyBytes = Routes[ i ].chunk ? CUploadStore::MaxChunkBytes : MaxJsonBodyBytes;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CGatewayApi::GetPaths( void )
{GUCEF_TRACE;

    CORE::CString::StringVector paths;
    for ( size_t i=0; i<sizeof( Routes ) / sizeof( Routes[ 0 ] ); ++i )
    {
        bool listed = false;
        for ( size_t p=0; p<paths.size() && !listed; ++p )
            listed = paths[ p ] == Routes[ i ].path;
        if ( !listed )
            paths.push_back( Routes[ i ].path );
    }
    return paths;
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::ParseQuery( const CORE::CString& query, CRequest::TParameterMap& parameters )
{GUCEF_TRACE;

    CORE::CString::StringVector pairs = query.ParseElements( '&', false );
    for ( size_t i=0; i<pairs.size(); ++i )
    {
        const Int32 equals = pairs[ i ].HasChar( '=', true );
        if ( equals <= 0 )
            continue;
        parameters[ QueryDecode( pairs[ i ].SubstrToIndex( (UInt32) equals, true ) ) ] = QueryDecode( pairs[ i ].CutChars( (UInt32) equals + 1, true ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetError( CResponse& response, Int32 status, const char* code, const CORE::CString& message )
{GUCEF_TRACE;

    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "error", message );
    json.SetAttribute( "code", code );
    SetJson( response, status, json );
    response.pollAgain = false;
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetJson( CResponse& response, Int32 status, const CORE::CDataNode& json )
{GUCEF_TRACE;

    response.status = status;
    if ( !CCiUtils::ToJson( json, response.body ) )
    {
        response.status = 500;
        response.body = "{\"error\":\"the answer could not be written as JSON\",\"code\":\"internal\"}";
    }
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetServiceError( CResponse& response, const CORE::CString& error )
{GUCEF_TRACE;

    if ( error.HasSubstr( "Unknown run id" ) >= 0 )
        SetError( response, 404, "unknown_run", error );
    else
    if ( error.HasSubstr( "Unknown job id" ) >= 0 )
        SetError( response, 404, "unknown_job", error );
    else
    if ( error.HasSubstr( "Unknown CI id" ) >= 0 )
        SetError( response, 404, "unknown_ci", error );
    else
    if ( error.HasSubstr( "No CI is set up" ) >= 0 )
        SetError( response, 404, "no_ci", error );
    else
    if ( error.HasSubstr( "Unknown filter values" ) >= 0 || error.StartsWith( "mode \"" ) )
        SetError( response, 400, "bad_options", error );
    else
        SetError( response, 502, "backend", error );
}

/*-------------------------------------------------------------------------*/

bool
CGatewayApi::SetUploadError( CResponse& response, CUploadStore::EResult result, const CORE::CString& problem )
{GUCEF_TRACE;

    switch ( result )
    {
        case CUploadStore::UPLOAD_OK: return false;
        case CUploadStore::UPLOAD_NOT_FOUND: SetError( response, 404, "unknown_upload", problem ); break;
        case CUploadStore::UPLOAD_OFFSET_MISMATCH: SetError( response, 409, "offset", problem ); break;
        case CUploadStore::UPLOAD_TOO_LARGE: SetError( response, 413, "too_large", problem ); break;
        case CUploadStore::UPLOAD_BUSY: SetError( response, 409, "busy", problem ); break;
        case CUploadStore::UPLOAD_FULL: SetError( response, 503, "uploads_full", problem ); break;
        case CUploadStore::UPLOAD_INCOMPLETE: SetError( response, 409, "incomplete", problem ); break;
        case CUploadStore::UPLOAD_HASH_MISMATCH: SetError( response, 422, "bad_sha256", problem ); break;
        case CUploadStore::UPLOAD_BAD_REQUEST: SetError( response, 400, "bad_request", problem ); break;
        default: SetError( response, 500, "io", problem ); break;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGatewayApi::ReadJsonBody( const CRequest& request, CResponse& response, CORE::CDataNode& json )
{GUCEF_TRACE;

    json.Clear();
    if ( 0 == request.body.GetDataSize() || !CCiUtils::ParseJson( request.body, json ) || GUCEF_DATATYPE_OBJECT != json.GetNodeType() )
    {
        SetError( response, 400, "bad_request", "the body must be a JSON object" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::AddLimits( CORE::CDataNode& json ) const
{GUCEF_TRACE;

    CORE::CDataNode* limits = json.AddChild( "limits", GUCEF_DATATYPE_OBJECT );
    limits->SetAttribute( "maxSnapshotBytes", m_uploads.GetMaxUploadBytes() );
    limits->SetAttribute( "chunkBytes", m_uploads.GetChunkBytes() );
    limits->SetAttribute( "maxWaitSeconds", m_maxWaitSeconds );
    limits->SetAttribute( "maxFullLogBytes", m_maxFullLogBytes );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::Execute( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    response = CResponse();
    const CORE::CString& method = request.method;
    const CORE::CString& path = request.path;
    if ( path == "/v1/ci" && method == "GET" )
        GetCi( request, response );
    else
    if ( path == "/v1/resolve" && method == "POST" )
        Resolve( request, response );
    else
    if ( path == "/v1/uploads" && method == "POST" )
        CreateUpload( request, response );
    else
    if ( path == "/v1/uploads" && method == "PUT" )
        WriteChunk( request, response );
    else
    if ( path == "/v1/uploads" && method == "GET" )
        GetUpload( request, response );
    else
    if ( path == "/v1/uploads" && method == "DELETE" )
        RemoveUpload( request, response );
    else
    if ( path == "/v1/uploads/commit" && method == "POST" )
        CommitUpload( request, response );
    else
    if ( path == "/v1/runs" && method == "POST" )
        StartRun( request, response );
    else
    if ( path == "/v1/runs" && method == "GET" )
        GetRun( request, response );
    else
    if ( path == "/v1/runs/report" && method == "GET" )
        GetReport( request, response );
    else
    if ( path == "/v1/runs/jobs" && method == "GET" )
        GetJob( request, response );
    else
    if ( path == "/v1/runs/cancel" && method == "POST" )
        CancelRun( request, response );
    else
    if ( path == "/v1/cleanup" && method == "POST" )
        Cleanup( request, response );
    else
    if ( path == "/v1/images" && method == "GET" )
        GetImages( request, response );
    else
    if ( path == "/v1/deploy" && method == "POST" )
        Deploy( request, response );
    else
    if ( path == "/v1/release" && method == "GET" )
        GetRelease( request, response );
    else
    if ( path == "/v1/rollback" && method == "POST" )
        Rollback( request, response );
    else
        SetError( response, 404, "not_found", "no such call: " + method + " " + path );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetCi( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    const CORE::CString ciId = request.Parameter( "id" );
    if ( ciId.IsNULLOrEmpty() )
    {
        CORE::CString::StringVector ids;
        const CCiRegistry::TDefinitionVector& definitions = m_service.GetRegistry().GetDefinitions();
        for ( size_t i=0; i<definitions.size(); ++i )
            ids.push_back( definitions[ i ].id );
        CCiJson::WriteStrings( ids, "ids", json );
        AddLimits( json );
        SetJson( response, 200, json );
        return;
    }

    CCiDescription description;
    CORE::CString error;
    if ( !m_service.Describe( ciId, CORE::CString::Empty, description, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    const CCiDefinition* ci = m_service.GetRegistry().FindById( ciId );
    CCiJson::Write( description, *json.AddChild( "ci", GUCEF_DATATYPE_OBJECT ) );
    if ( GUCEF_NULL != ci )
    {
        json.SetAttribute( "backend", ci->backend );
        json.SetAttribute( "matrixDir", ci->matrixDir );
        CORE::CDataNode* defaults = json.AddChild( "defaults", GUCEF_DATATYPE_OBJECT );
        defaults->SetAttribute( "mode", ci->defaultMode );
        CCiJson::WriteStrings( ci->defaultToolchains, "toolchains", *defaults );
        CCiJson::WriteStrings( ci->defaultCppStandards, "cppStandards", *defaults );
        CCiJson::WriteStrings( ci->defaultProducts, "products", *defaults );
    }

    // the commits a snapshot can build on; without them a snapshot is sent with its whole history
    CORE::CString::StringVector have;
    CORE::CString haveProblem;
    if ( !m_service.ListBaseCommits( ciId, have, haveProblem ) )
        json.SetAttribute( "haveProblem", haveProblem );
    CCiJson::WriteStrings( have, "have", json );
    AddLimits( json );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::Resolve( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode body;
    if ( !ReadJsonBody( request, response, body ) )
        return;
    const CORE::CString::StringVector remotes = CCiUtils::GetStringArray( body, "remotes" );
    CORE::CString ciId;
    CORE::CString description;
    CORE::CString matched;
    CORE::CString::StringVector examined;
    CORE::CString error;
    if ( !m_service.Resolve( CORE::CString::Empty, remotes, ciId, description, matched, examined, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "ciId", ciId );
    json.SetAttribute( "description", description );
    json.SetAttribute( "matchedRemote", matched );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::CreateUpload( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    const CORE::CString ciId = request.Parameter( "ci" );
    if ( GUCEF_NULL == m_service.GetRegistry().FindById( ciId ) )
    {
        SetError( response, 404, "unknown_ci", "Unknown CI id \"" + ciId + "\". Known CI ids: " + m_service.ListCiIds() + "." );
        return;
    }
    const UInt64 size = CORE::StringToUInt64( request.Parameter( "size" ), 0 );
    CORE::CString uploadId;
    CORE::CString problem;
    if ( SetUploadError( response, m_uploads.Create( ciId, request.machineId, size, request.Parameter( "sha256" ), uploadId, problem ), problem ) )
        return;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: Upload " << uploadId << " of " << size << " bytes for " << ciId << " from " << request.machineId );
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "id", uploadId );
    json.SetAttribute( "chunkBytes", m_uploads.GetChunkBytes() );
    json.SetAttribute( "received", (UInt64) 0 );
    SetJson( response, 201, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::WriteChunk( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    const CORE::CString offsetText = request.Parameter( "offset" );
    if ( offsetText.IsNULLOrEmpty() )
    {
        SetError( response, 400, "bad_request", "a chunk needs its offset" );
        return;
    }
    UInt64 received = 0;
    CORE::CString problem;
    const CUploadStore::EResult result = m_uploads.Write( request.Parameter( "id" ), request.machineId, CORE::StringToUInt64( offsetText, 0 ), request.body, received, problem );
    if ( CUploadStore::UPLOAD_OFFSET_MISMATCH == result )
    {
        CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
        json.SetAttribute( "error", problem );
        json.SetAttribute( "code", "offset" );
        json.SetAttribute( "received", received );
        SetJson( response, 409, json );
        return;
    }
    if ( SetUploadError( response, result, problem ) )
        return;
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "received", received );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetUpload( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    UInt64 received = 0;
    UInt64 size = 0;
    CORE::CString problem;
    if ( SetUploadError( response, m_uploads.GetState( request.Parameter( "id" ), request.machineId, received, size, problem ), problem ) )
        return;
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "received", received );
    json.SetAttribute( "size", size );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::RemoveUpload( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    const CUploadStore::EResult result = m_uploads.Remove( request.Parameter( "id" ), request.machineId );
    if ( SetUploadError( response, result, CUploadStore::UPLOAD_BUSY == result ? CORE::CString( "another call works on the upload" ) : "no upload \"" + request.Parameter( "id" ) + "\"" ) )
        return;
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "removed", true );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::CommitUpload( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    const CORE::CString uploadId = request.Parameter( "id" );
    CORE::CString path;
    CORE::CString ciId;
    CORE::CString problem;
    if ( SetUploadError( response, m_uploads.BeginCommit( uploadId, request.machineId, path, ciId, problem ), problem ) )
        return;

    // the bundle into the bare repository of the CI id, from there to the backend as in direct mode
    CCiSnapshot snapshot;
    CORE::CString error;
    const CBundleReceiver::EResult received = m_receiver.Receive( ciId, path, request.machineId, snapshot.repoDir, snapshot.commit, snapshot.branchName, error );
    if ( CBundleReceiver::BUNDLE_OK != received )
    {
        m_uploads.EndCommit( uploadId );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The snapshot of upload " << uploadId << " from " << request.machineId << " was not taken: " << error );
        if ( CBundleReceiver::BUNDLE_UNKNOWN_BASE == received )
            SetError( response, 422, "unknown_base", error );
        else
        if ( CBundleReceiver::BUNDLE_INVALID == received )
            SetError( response, 422, "bad_bundle", error );
        else
            SetError( response, 502, "backend", error );
        return;
    }
    CORE::CProcess::TStringVector args;
    args.push_back( "rev-parse" );
    args.push_back( "--verify" );
    args.push_back( snapshot.commit + "^" );
    CORE::CString output;
    if ( CCiUtils::RunGit( snapshot.repoDir, args, CORE::CProcess::TStringMap(), 60000, output ) )
        snapshot.baseCommit = CCiUtils::FindObjectId( output );

    if ( !m_service.SendSnapshot( ciId, snapshot, error ) )
    {
        m_uploads.EndCommit( uploadId );
        SetServiceError( response, error );
        return;
    }
    m_uploads.EndCommit( uploadId );

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_sent.size() >= MaxSentSnapshots )
        {
            // the oldest one goes
            TSentSnapshotMap::iterator oldest = m_sent.begin();
            for ( TSentSnapshotMap::iterator i = m_sent.begin(); i != m_sent.end(); ++i )
                if ( (*i).second.sentInMs < (*oldest).second.sentInMs )
                    oldest = i;
            m_sent.erase( oldest );
        }
        CSentSnapshot& sent = m_sent[ uploadId ];
        sent.ciId = ciId;
        sent.machineId = request.machineId;
        sent.snapshot = snapshot;
        sent.sentInMs = CCiUtils::MonotonicMs();
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The snapshot " << snapshot.commit << " of " << request.machineId << " is on " << snapshot.branchName );

    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "snapshotId", uploadId );
    CCiJson::Write( snapshot, *json.AddChild( "snapshot", GUCEF_DATATYPE_OBJECT ) );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::RemoveExpiredSnapshots( UInt64 nowInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSentSnapshotMap::iterator i = m_sent.begin();
    while ( i != m_sent.end() )
    {
        if ( !(*i).second.starting && nowInMs > (*i).second.sentInMs && nowInMs - (*i).second.sentInMs > SentSnapshotLifetimeInMs )
            m_sent.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::StartRun( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode body;
    if ( !ReadJsonBody( request, response, body ) )
        return;
    const CORE::CString snapshotId = CCiUtils::Trimmed( body.GetAttributeValue( "snapshot" ).AsString() );
    CCiSubmitOptions options;
    const CORE::CDataNode* optionsNode = body.FindChild( "options" );
    if ( GUCEF_NULL != optionsNode )
        CCiJson::Read( *optionsNode, options );
    if ( !options.commit.IsNULLOrEmpty() )
    {
        // a published commit: nothing was uploaded, the gateway sends it from its own clone
        StartCommitRun( request, CCiUtils::Trimmed( body.GetAttributeValue( "ci" ).AsString() ), options, response );
        return;
    }

    CSentSnapshot sent;
    {
        MT::CScopeMutex lock( m_lock );
        TSentSnapshotMap::iterator i = m_sent.find( snapshotId );
        if ( i == m_sent.end() || (*i).second.machineId != request.machineId )
        {
            SetError( response, 404, "unknown_snapshot", "no snapshot \"" + snapshotId + "\" was sent by this machine within the last hour; submit again" );
            return;
        }
        if ( !(*i).second.runId.IsNULLOrEmpty() )
        {
            // asked again, for instance after a time out: the run that was started
            CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
            json.SetAttribute( "runId", (*i).second.runId );
            SetJson( response, 200, json );
            return;
        }
        if ( (*i).second.starting )
        {
            SetError( response, 409, "busy", "a run is being started on the snapshot" );
            return;
        }
        (*i).second.starting = true;
        sent = (*i).second;
    }

    CORE::CString runId;
    CORE::CString error;
    const bool started = m_service.StartRun( sent.ciId, sent.snapshot, options, runId, error );
    {
        MT::CScopeMutex lock( m_lock );
        TSentSnapshotMap::iterator i = m_sent.find( snapshotId );
        if ( i != m_sent.end() )
        {
            (*i).second.starting = false;
            if ( started )
                (*i).second.runId = runId;
        }
    }
    if ( !started )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "runId", runId );
    SetJson( response, 201, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetRun( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CCiRunStatus status;
    CORE::CString error;
    if ( !m_service.Status( request.Parameter( "id" ), 0, status, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    CCiJson::Write( status, json );
    SetJson( response, 200, json );
    response.pollAgain = !status.IsFinished() && request.pollDeadlineInMs > CCiUtils::MonotonicMs();
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetReport( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CCiReport report;
    CORE::CString error;
    if ( !m_service.Report( request.Parameter( "id" ), report, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    CCiJson::Write( report, json );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetJob( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CCiJobDetail detail;
    CORE::CString error;
    if ( !m_service.JobDetails( request.Parameter( "id" ), request.Parameter( "job" ), request.Parameter( "product" ), detail, error ) )
    {
        SetServiceError( response, error );
        return;
    }

    // the digest of each log is made here, on the whole log; the log itself goes within bounds
    const bool full = IsTrue( request.Parameter( "full" ) );
    CORE::CString part = request.Parameter( "part" ).Lowercase();
    UInt32 limit = CORE::StringToUInt32( request.Parameter( "limit" ), DefaultTailLines );
    if ( 0 == limit || limit > MaxTailLines )
        limit = DefaultTailLines;
    const UInt32 offset = CORE::StringToUInt32( request.Parameter( "offset" ), 0 );
    UInt64 budget = m_maxFullLogBytes;
    for ( size_t i=0; i<detail.logs.size(); ++i )
    {
        CCiJobDetail::CLog& log = detail.logs[ i ];
        const UInt64 size = log.content.ByteSize() > 0 ? log.content.ByteSize() - 1 : 0;
        log.totalBytes = size;
        if ( 0 == size )
            continue;
        log.digest = CCiUtils::Bound( FormatLogDigest( log.content, detail.cell.workspace ), MaxDigestChars );
        if ( full )
        {
            if ( size > budget )
            {
                log.content = TailBytes( log.content, budget );
                log.truncated = true;
            }
            const UInt64 kept = log.content.ByteSize() > 0 ? log.content.ByteSize() - 1 : 0;
            budget = kept < budget ? budget - kept : 0;
        }
        else
        if ( part == "log_tail" )
        {
            UInt32 totalLines = 0;
            log.content = CCiUtils::TailLines( log.content, limit, offset, totalLines );
            log.truncated = true;
        }
        else
        {
            log.content.Clear();
            log.truncated = true;
        }
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    CCiJson::Write( detail, json );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::CancelRun( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CString message;
    CORE::CString error;
    if ( !m_service.Cancel( request.Parameter( "id" ), message, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "message", message );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::Cleanup( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode body;
    if ( !ReadJsonBody( request, response, body ) )
        return;
    UInt32 days = body.GetAttributeValue( "olderThanDays" ).AsUInt32( DefaultCleanupDays );
    if ( 0 == days )
        days = DefaultCleanupDays;
    CORE::CString summary;
    CORE::CString error;
    if ( !m_service.Cleanup( CCiUtils::Trimmed( body.GetAttributeValue( "ci" ).AsString() ), days, summary, error ) )
    {
        SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "summary", summary );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::StartCommitRun( const CRequest& request, const CORE::CString& ciId, const CCiSubmitOptions& options, CResponse& response )
{GUCEF_TRACE;

    CCiSnapshot snapshot;
    CORE::CString runId;
    CORE::CString error;
    if ( !m_service.Submit( ciId, CORE::CString::Empty, options, snapshot, runId, error ) )
    {
        if ( error.StartsWith( "commit must be" ) || error.HasSubstr( "the published repository" ) >= 0 )
            SetError( response, 422, "unpublished", error );
        else
            SetServiceError( response, error );
        return;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The published commit " << snapshot.commit << " for " << request.machineId << " is on " <<
                      snapshot.branchName << ( runId.IsNULLOrEmpty() ? CORE::CString::Empty : ", run " + runId ) );
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    json.SetAttribute( "runId", runId );
    CCiJson::Write( snapshot, *json.AddChild( "snapshot", GUCEF_DATATYPE_OBJECT ) );
    SetJson( response, options.dryRun ? 200 : 201, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetImages( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CCiImageReport report;
    CORE::CString error;
    if ( !m_service.Images( request.Parameter( "id" ), report, error ) )
    {
        if ( error.HasSubstr( "published no image report" ) >= 0 )
            SetError( response, 404, "no_images", error );
        else
            SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    report.SaveToJson( json );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::SetDeployAnswer( CResponse& response, bool success, const CCiDeployResult& result, const CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    Int32 status = 200;
    if ( !success )
    {
        const CORE::CString& code = result.code;
        if ( code == "scope" || code == "not_listed" )
            status = 403;
        else
        if ( code == "confirm" || code == "bad_request" )
            status = 400;
        else
        if ( code == "unknown_ci" || code == "unknown_run" )
            status = 404;
        else
        if ( code == "busy" )
            status = 409;
        else
        if ( code == "not_green" || code == "no_images" || code == "image_failed" || code == "unpublished" || code == "pull_policy" )
            status = 422;
        else
            status = 502;
        json.SetAttribute( "error", error );
        json.SetAttribute( "code", code.IsNULLOrEmpty() ? CORE::CString( "backend" ) : code );
    }
    result.SaveToJson( *json.AddChild( "result", GUCEF_DATATYPE_OBJECT ) );
    SetJson( response, status, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::Deploy( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode body;
    if ( !ReadJsonBody( request, response, body ) )
        return;
    CORE::CString runOrCommit = CCiUtils::Trimmed( body.GetAttributeValue( "run" ).AsString() );
    if ( runOrCommit.IsNULLOrEmpty() )
        runOrCommit = CCiUtils::Trimmed( body.GetAttributeValue( "commit" ).AsString() );
    // only the boolean true confirms
    const CORE::CVariant confirmValue = body.GetAttributeValue( "confirm" );
    const bool confirm = confirmValue.IsBoolean() && confirmValue.AsBool( false );
    CCiDeployResult result;
    CORE::CString error;
    const bool deployed = m_service.Deploy( CCiUtils::Trimmed( body.GetAttributeValue( "ci" ).AsString() ), CCiUtils::Trimmed( body.GetAttributeValue( "image" ).AsString() ),
                                            CCiUtils::Trimmed( body.GetAttributeValue( "target" ).AsString() ), runOrCommit, confirm, request.caller, result, error );
    SetDeployAnswer( response, deployed, result, error );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::GetRelease( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CCiReleaseStatus status;
    CORE::CString error;
    if ( !m_service.ReleaseStatus( request.Parameter( "ci" ), request.Parameter( "target" ), status, error ) )
    {
        if ( error.HasSubstr( "has the target" ) >= 0 || error.HasSubstr( "may be deployed" ) >= 0 )
            SetError( response, 404, "not_listed", error );
        else
            SetServiceError( response, error );
        return;
    }
    CORE::CDataNode json( GUCEF_DATATYPE_OBJECT );
    status.SaveToJson( json );
    SetJson( response, 200, json );
}

/*-------------------------------------------------------------------------*/

void
CGatewayApi::Rollback( const CRequest& request, CResponse& response )
{GUCEF_TRACE;

    CORE::CDataNode body;
    if ( !ReadJsonBody( request, response, body ) )
        return;
    const CORE::CVariant confirmValue = body.GetAttributeValue( "confirm" );
    const bool confirm = confirmValue.IsBoolean() && confirmValue.AsBool( false );
    CCiDeployResult result;
    CORE::CString error;
    const bool rolledBack = m_service.Rollback( CCiUtils::Trimmed( body.GetAttributeValue( "ci" ).AsString() ), CCiUtils::Trimmed( body.GetAttributeValue( "image" ).AsString() ),
                                                CCiUtils::Trimmed( body.GetAttributeValue( "target" ).AsString() ), confirm, request.caller, result, error );
    SetDeployAnswer( response, rolledBack, result, error );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
