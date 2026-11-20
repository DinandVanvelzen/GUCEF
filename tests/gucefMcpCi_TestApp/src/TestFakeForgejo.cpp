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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#include "TestMcpSupport.h"
#include "TestMcpCiSupport.h"
#include "TestFakeForgejo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Passes the requests of the HTTP server to a CFakeForgejo */
class CFakeForgejoHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeForgejoHandler( void ) : WEB::CIHttpServerRequestHandler(), m_server( GUCEF_NULL ) {}
    virtual ~CFakeForgejoHandler() {}

    void SetServer( CFakeForgejo* server ) { m_server = server; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_server )
        {
            response.statusCode = 503;
            return true;
        }
        m_server->Handle( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeForgejo* m_server;
};

/*-------------------------------------------------------------------------*/

class CFakeForgejoHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeForgejoHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeForgejoHandler, MT::CMutex > TBase;

    explicit CFakeForgejoHandlerFactory( CFakeForgejo* server ) : TBase(), m_server( server ) {}
    virtual ~CFakeForgejoHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeForgejoHandler* handler = static_cast< CFakeForgejoHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetServer( m_server );
        return product;
    }

    private:

    CFakeForgejo* m_server;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
NowIso( void )
{
    char buffer[ 32 ];
    time_t now = ::time( GUCEF_NULL );
    ::strftime( buffer, sizeof( buffer ), "%Y-%m-%dT%H:%M:%SZ", ::gmtime( &now ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
QueryValue( const CORE::CString& query, const char* name )
{
    CORE::CString::StringVector items = query.ParseElements( '&', false );
    for ( size_t i=0; i<items.size(); ++i )
    {
        const Int32 equals = items[ i ].HasChar( '=', true );
        if ( equals > 0 && items[ i ].SubstrToIndex( (UInt32) equals, true ) == name )
            return PercentDecode( items[ i ].CutChars( (UInt32) equals + 1, true ) );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static void
Respond( WEB::CHttpResponseData& response, int status, const CORE::CString& json )
{
    response.statusCode = status;
    response.content.Clear( true );
    if ( !json.IsNULLOrEmpty() )
    {
        response.contentType = "application/json";
        response.content.CopyFrom( json.ByteSize() - 1, json.C_String() );
    }
}

/*-------------------------------------------------------------------------*/

static bool
IsFinalStatus( const CORE::CString& status )
{
    return status == "success" || status == "failure" || status == "cancelled" || status == "skipped";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeForgejo::CStage::CStage( void )
    : runStatus( "waiting" )
    , jobNames()
    , jobStatuses()
    , polls( 1 )
{
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::CStage::CStage( const CORE::CString& status, UInt32 pollCount )
    : runStatus( status )
    , jobNames()
    , jobStatuses()
    , polls( pollCount )
{
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::CStage&
CFakeForgejo::CStage::AddJob( const CORE::CString& name, const CORE::CString& status )
{
    jobNames.push_back( name );
    jobStatuses.push_back( status );
    return *this;
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::CRun::CRun( void )
    : id( 0 )
    , number( 0 )
    , ref()
    , sha()
    , stages()
    , stage( 0 )
    , pollsInStage( 0 )
    , cancelled( false )
{
}

/*-------------------------------------------------------------------------*/

const CFakeForgejo::CStage&
CFakeForgejo::CRun::Current( void ) const
{
    return stages[ stage < stages.size() ? stage : stages.size() - 1 ];
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::CFakeForgejo( const CORE::CString& token, const CORE::CString& owner, const CORE::CString& repo )
    : MT::CActiveObject()
    , m_token( token )
    , m_owner( owner )
    , m_repo( repo )
    , m_bareRepo()
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_running( false )
    , m_returnRunInfo( true )
    , m_cancelSupported( true )
    , m_nextStages()
    , m_runs()
    , m_packageFiles()
    , m_nextRunId( 100 )
    , m_lastDispatchBody()
    , m_dispatchCount( 0 )
    , m_unauthorizedCount( 0 )
    , m_runListCount( 0 )
    , m_requestLog()
{
}

/*-------------------------------------------------------------------------*/

CFakeForgejo::~CFakeForgejo()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeForgejo::Start( void )
{
    CFakeForgejoHandlerFactory* factory = GUCEF_NEW CFakeForgejoHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    m_server->Close();
    GUCEF_DELETE m_server;
    m_server = GUCEF_NULL;
    GUCEF_DELETE static_cast< CFakeForgejoHandlerFactory* >( m_factory );
    m_factory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeForgejo::GetUrl( void ) const
{
    return "http://127.0.0.1:" + CORE::UInt16ToString( GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0 );
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::SetBareRepo( const CORE::CString& path )
{
    MT::CScopeMutex lock( m_lock );
    m_bareRepo = path;
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::SetReturnRunInfo( bool returnRunInfo )
{
    MT::CScopeMutex lock( m_lock );
    m_returnRunInfo = returnRunInfo;
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::SetCancelSupported( bool cancelSupported )
{
    MT::CScopeMutex lock( m_lock );
    m_cancelSupported = cancelSupported;
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::ScriptNextRun( const TStageVector& stages )
{
    MT::CScopeMutex lock( m_lock );
    m_nextStages = stages;
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::AddPackageFile( const CORE::CString& owner, const CORE::CString& name, const CORE::CString& version,
                              const CORE::CString& fileName, const CORE::CDynamicBuffer& content, const CORE::CString& createdAt )
{
    MT::CScopeMutex lock( m_lock );
    CPackageFile file;
    file.owner = owner;
    file.name = name;
    file.version = version;
    file.fileName = fileName;
    file.content = content;
    file.createdAt = createdAt.IsNULLOrEmpty() ? NowIso() : createdAt;
    m_packageFiles.push_back( file );
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::AddPackageFile( const CORE::CString& owner, const CORE::CString& name, const CORE::CString& version,
                              const CORE::CString& fileName, const CORE::CString& content, const CORE::CString& createdAt )
{
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( content.ByteSize() > 0 ? content.ByteSize() - 1 : 0, content.C_String() );
    AddPackageFile( owner, name, version, fileName, buffer, createdAt );
}

/*-------------------------------------------------------------------------*/

bool
CFakeForgejo::HasPackageVersion( const CORE::CString& owner, const CORE::CString& name, const CORE::CString& version ) const
{
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_packageFiles.size(); ++i )
        if ( m_packageFiles[ i ].owner == owner && m_packageFiles[ i ].name == name && m_packageFiles[ i ].version == version )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

Int64 CFakeForgejo::GetLastRunId( void ) const { MT::CScopeMutex lock( m_lock ); return m_runs.empty() ? 0 : m_runs[ m_runs.size()-1 ].id; }
CORE::CString CFakeForgejo::GetLastDispatchBody( void ) const { MT::CScopeMutex lock( m_lock ); return m_lastDispatchBody; }
UInt32 CFakeForgejo::GetDispatchCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_dispatchCount; }
UInt32 CFakeForgejo::GetUnauthorizedCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_unauthorizedCount; }
UInt32 CFakeForgejo::GetRunListCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_runListCount; }
CORE::CString CFakeForgejo::GetRequestLog( void ) const { MT::CScopeMutex lock( m_lock ); return m_requestLog; }

/*-------------------------------------------------------------------------*/

CFakeForgejo::CRun*
CFakeForgejo::FindRun( Int64 id )
{
    for ( size_t i=0; i<m_runs.size(); ++i )
        if ( m_runs[ i ].id == id )
            return &m_runs[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeForgejo::RunJson( const CRun& run ) const
{
    const CStage& stage = run.Current();
    const CORE::CString status = run.cancelled ? CORE::CString( "cancelled" ) : stage.runStatus;
    const CORE::CString started = status == "waiting" ? CORE::CString( "0001-01-01T00:00:00Z" ) : NowIso();
    const CORE::CString stopped = IsFinalStatus( status ) ? NowIso() : CORE::CString( "0001-01-01T00:00:00Z" );
    return "{\"id\":" + CORE::Int64ToString( run.id ) + ",\"index_in_repo\":" + CORE::Int64ToString( run.number ) +
           ",\"status\":" + Json( status ) + ",\"commit_sha\":" + Json( run.sha ) + ",\"prettyref\":" + Json( run.ref ) +
           ",\"event\":\"workflow_dispatch\",\"trigger_event\":\"workflow_dispatch\",\"title\":\"CI scratch snapshot\",\"workflow_id\":\"gucef-ci.yml\"" +
           ",\"created\":" + Json( NowIso() ) + ",\"started\":" + Json( started ) + ",\"stopped\":" + Json( stopped ) +
           ",\"html_url\":" + Json( GetUrl() + "/" + m_owner + "/" + m_repo + "/actions/runs/" + CORE::Int64ToString( run.number ) ) + "}";
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeForgejo::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeForgejo::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeForgejo::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::Handle( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    const CORE::CString method = request.requestType.Uppercase();
    const CORE::CString path = request.requestUri;
    const CORE::CString query = request.requestUriParams;

    MT::CScopeMutex lock( m_lock );
    m_requestLog += method + " " + path + ( query.IsNULLOrEmpty() ? CORE::CString::Empty : "?" + query ) + " (" +
                    CORE::UInt32ToString( request.content.GetDataSize() ) + " bytes)\n";

    WEB::CHttpRequestData::TStringMap::const_iterator auth = request.otherHeaders.find( "authorization" );
    if ( auth == request.otherHeaders.end() || (*auth).second != "token " + m_token )
    {
        ++m_unauthorizedCount;
        Respond( response, 401, "{\"message\":\"token is required\"}" );
        return;
    }

    const CORE::CString repoPrefix = "/api/v1/repos/" + m_owner + "/" + m_repo;
    if ( method == "GET" && path == repoPrefix )
    {
        // the repository itself: its default branch
        Respond( response, 200, "{\"name\":" + Json( m_repo ) + ",\"default_branch\":\"main\",\"private\":true}" );
        return;
    }
    if ( path.StartsWith( repoPrefix + "/" ) )
    {
        HandleRepo( method, path.CutChars( repoPrefix.Length(), true ), query, request, response );
        return;
    }
    if ( path.StartsWith( "/api/packages/" ) || path.StartsWith( "/api/v1/packages/" ) )
    {
        HandlePackages( method, path, query, request, response );
        return;
    }
    Respond( response, 404, "{\"message\":\"not found\"}" );
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::HandleRepo( const CORE::CString& method, const CORE::CString& rest, const CORE::CString& query,
                          const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    // POST /actions/workflows/{file}/dispatches
    if ( method == "POST" && rest.StartsWith( "/actions/workflows/" ) && rest.EndsWith( "/dispatches" ) )
    {
        const CORE::CString body( static_cast< const char* >( request.content.GetConstBufferPtr() ), request.content.GetDataSize() );
        m_lastDispatchBody = body;
        ++m_dispatchCount;
        CORE::CDataNode json;
        if ( !McpTestParse( body, json ) || json.GetAttributeValue( "ref" ).AsString().IsNULLOrEmpty() )
        {
            Respond( response, 400, "{\"message\":\"ref is required\"}" );
            return;
        }
        const CORE::CString ref = json.GetAttributeValue( "ref" ).AsString();
        const CORE::CString sha = GitOut( m_bareRepo, "rev-parse", "--verify", ( "refs/heads/" + ref ).C_String() );
        if ( sha.IsNULLOrEmpty() )
        {
            Respond( response, 422, "{\"message\":\"the ref does not exist\"}" );
            return;
        }
        CRun run;
        run.id = m_nextRunId++;
        run.number = (Int64) m_runs.size() + 1;
        run.ref = ref;
        run.sha = sha;
        run.stages = m_nextStages;
        if ( run.stages.empty() )
            run.stages.push_back( CStage( "waiting", 1000000 ) );
        m_runs.push_back( run );
        if ( m_returnRunInfo && json.GetAttributeValue( "return_run_info" ).AsBool( false ) )
        {
            CORE::CString jobs;
            const CStage& stage = run.Current();
            for ( size_t i=0; i<stage.jobNames.size(); ++i )
                jobs += ( i > 0 ? "," : "" ) + Json( stage.jobNames[ i ] );
            Respond( response, 201, "{\"id\":" + CORE::Int64ToString( run.id ) + ",\"run_number\":" + CORE::Int64ToString( run.number ) + ",\"jobs\":[" + jobs + "]}" );
        }
        else
        {
            Respond( response, 204, CORE::CString::Empty );
        }
        return;
    }

    // GET /actions/runs (newest first)
    if ( method == "GET" && rest == "/actions/runs" )
    {
        ++m_runListCount;
        const CORE::CString headSha = QueryValue( query, "head_sha" );
        CORE::CString list;
        UInt32 count = 0;
        for ( size_t i=m_runs.size(); i>0; --i )
        {
            const CRun& run = m_runs[ i-1 ];
            if ( !headSha.IsNULLOrEmpty() && run.sha != headSha )
                continue;
            list += ( count > 0 ? "," : "" ) + RunJson( run );
            ++count;
        }
        Respond( response, 200, "{\"total_count\":" + CORE::UInt32ToString( count ) + ",\"workflow_runs\":[" + list + "]}" );
        return;
    }

    // GET /actions/runs/{id}, POST /actions/runs/{id}/cancel
    if ( rest.StartsWith( "/actions/runs/" ) )
    {
        CORE::CString tail = rest.CutChars( 14, true );
        const bool cancel = tail.EndsWith( "/cancel" );
        if ( cancel )
            tail = tail.CutChars( 7, false );
        CRun* run = FindRun( CORE::StringToInt64( tail ) );
        if ( GUCEF_NULL == run )
        {
            Respond( response, 404, "{\"message\":\"run not found\"}" );
            return;
        }
        if ( cancel && method == "POST" )
        {
            if ( !m_cancelSupported )
            {
                Respond( response, 404, "{\"message\":\"not found\"}" );
                return;
            }
            run->cancelled = true;
            Respond( response, 200, "{}" );
            return;
        }
        if ( method != "GET" || cancel )
        {
            Respond( response, 405, "{\"message\":\"method not allowed\"}" );
            return;
        }
        // a stage that was seen often enough gives way to the next before the answer, so that the tasks asked for
        // after it show the same stage
        if ( run->pollsInStage >= run->Current().polls && run->stage + 1 < run->stages.size() )
        {
            ++run->stage;
            run->pollsInStage = 0;
        }
        ++run->pollsInStage;
        Respond( response, 200, RunJson( *run ) );
        return;
    }

    // GET /actions/tasks: the jobs of all runs, newest first
    if ( method == "GET" && rest == "/actions/tasks" )
    {
        UInt32 page = CORE::StringToUInt32( QueryValue( query, "page" ), 1 );
        UInt32 limit = CORE::StringToUInt32( QueryValue( query, "limit" ), 50 );
        if ( 0 == page )
            page = 1;
        if ( 0 == limit )
            limit = 50;
        GUCEF::vector< CORE::CString > tasks;
        for ( size_t i=m_runs.size(); i>0; --i )
        {
            const CRun& run = m_runs[ i-1 ];
            const CStage& stage = run.Current();
            for ( size_t j=stage.jobNames.size(); j>0; --j )
            {
                CORE::CString status = stage.jobStatuses[ j-1 ];
                if ( run.cancelled && !IsFinalStatus( status ) )
                    status = "cancelled";
                tasks.push_back( "{\"id\":" + CORE::Int64ToString( run.id * 100 + (Int64) j ) + ",\"name\":" + Json( stage.jobNames[ j-1 ] ) +
                                 ",\"status\":" + Json( status ) + ",\"run_number\":" + CORE::Int64ToString( run.number ) +
                                 ",\"head_sha\":" + Json( run.sha ) + ",\"head_branch\":" + Json( run.ref ) + ",\"event\":\"workflow_dispatch\"" +
                                 ",\"display_title\":\"CI scratch snapshot\",\"workflow_id\":\"gucef-ci.yml\",\"run_started_at\":" + Json( NowIso() ) +
                                 ",\"created_at\":" + Json( NowIso() ) + ",\"updated_at\":" + Json( NowIso() ) + ",\"url\":\"\"}" );
            }
        }
        CORE::CString list;
        const size_t first = (size_t) ( page - 1 ) * limit;
        for ( size_t i=first; i<tasks.size() && i<first+limit; ++i )
            list += ( i > first ? "," : "" ) + tasks[ i ];
        Respond( response, 200, "{\"total_count\":" + CORE::UInt32ToString( (UInt32) tasks.size() ) + ",\"workflow_runs\":[" + list + "]}" );
        return;
    }

    // GET /commits/{sha}/statuses
    if ( method == "GET" && rest.StartsWith( "/commits/" ) && rest.EndsWith( "/statuses" ) )
    {
        Respond( response, 200, "[]" );
        return;
    }

    // GET /branches
    if ( method == "GET" && rest == "/branches" )
    {
        const UInt32 page = CORE::StringToUInt32( QueryValue( query, "page" ), 1 );
        CORE::CString list;
        if ( page <= 1 )
        {
            CORE::CString::StringVector lines = GitOut( m_bareRepo, "for-each-ref", "--format=%(refname:short)|%(objectname)|%(committerdate:iso-strict)", "refs/heads" ).ParseElements( '\n', false );
            for ( size_t i=0; i<lines.size(); ++i )
            {
                CORE::CString::StringVector parts = lines[ i ].ParseElements( '|', true );
                if ( parts.size() < 3 )
                    continue;
                list += ( list.IsNULLOrEmpty() ? "" : "," ) + CORE::CString( "{\"name\":" ) + Json( parts[ 0 ] ) + ",\"commit\":{\"id\":" + Json( parts[ 1 ] ) +
                        ",\"timestamp\":" + Json( parts[ 2 ] ) + "},\"protected\":false}";
            }
        }
        Respond( response, 200, "[" + list + "]" );
        return;
    }

    // DELETE /branches/{name}
    if ( method == "DELETE" && rest.StartsWith( "/branches/" ) )
    {
        const CORE::CString name = PercentDecode( rest.CutChars( 10, true ) );
        if ( 0 != Git( m_bareRepo, "update-ref", "-d", ( "refs/heads/" + name ).C_String() ) )
        {
            Respond( response, 404, "{\"message\":\"branch not found\"}" );
            return;
        }
        Respond( response, 204, CORE::CString::Empty );
        return;
    }

    Respond( response, 404, "{\"message\":\"not found\"}" );
}

/*-------------------------------------------------------------------------*/

void
CFakeForgejo::HandlePackages( const CORE::CString& method, const CORE::CString& path, const CORE::CString& query,
                              const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    CORE::CString::StringVector parts = path.ParseElements( '/', false );

    // GET /api/v1/packages/{owner}?type=generic&q=name
    if ( 4 == parts.size() && parts[ 1 ] == "v1" && method == "GET" )
    {
        const CORE::CString owner = parts[ 3 ];
        const CORE::CString name = QueryValue( query, "q" );
        const UInt32 page = CORE::StringToUInt32( QueryValue( query, "page" ), 1 );
        CORE::CString list;
        CORE::CString::StringSet seen;
        if ( page <= 1 )
        {
            for ( size_t i=0; i<m_packageFiles.size(); ++i )
            {
                const CPackageFile& file = m_packageFiles[ i ];
                if ( file.owner != owner || ( !name.IsNULLOrEmpty() && file.name != name ) || seen.find( file.name + "/" + file.version ) != seen.end() )
                    continue;
                seen.insert( file.name + "/" + file.version );
                list += ( list.IsNULLOrEmpty() ? "" : "," ) + CORE::CString( "{\"id\":" ) + CORE::UInt32ToString( (UInt32) i + 1 ) + ",\"name\":" + Json( file.name ) +
                        ",\"version\":" + Json( file.version ) + ",\"type\":\"generic\",\"created_at\":" + Json( file.createdAt ) + "}";
            }
        }
        Respond( response, 200, "[" + list + "]" );
        return;
    }

    // /api/packages/{owner}/generic/{name}/{version}[/{file}]
    if ( parts.size() < 6 || parts[ 0 ] != "api" || parts[ 1 ] != "packages" || parts[ 3 ] != "generic" )
    {
        Respond( response, 404, "{\"message\":\"not found\"}" );
        return;
    }
    const CORE::CString owner = PercentDecode( parts[ 2 ] );
    const CORE::CString name = PercentDecode( parts[ 4 ] );
    const CORE::CString version = PercentDecode( parts[ 5 ] );
    if ( 6 == parts.size() && method == "DELETE" )
    {
        bool removed = false;
        for ( size_t i=m_packageFiles.size(); i>0; --i )
        {
            const CPackageFile& file = m_packageFiles[ i-1 ];
            if ( file.owner == owner && file.name == name && file.version == version )
            {
                m_packageFiles.erase( m_packageFiles.begin() + ( i-1 ) );
                removed = true;
            }
        }
        Respond( response, removed ? 204 : 404, removed ? CORE::CString::Empty : CORE::CString( "{\"message\":\"package not found\"}" ) );
        return;
    }
    if ( 7 != parts.size() )
    {
        Respond( response, 404, "{\"message\":\"not found\"}" );
        return;
    }
    const CORE::CString fileName = PercentDecode( parts[ 6 ] );
    if ( method == "PUT" )
    {
        CPackageFile file;
        file.owner = owner;
        file.name = name;
        file.version = version;
        file.fileName = fileName;
        file.content = request.content;
        file.createdAt = NowIso();
        m_packageFiles.push_back( file );
        Respond( response, 201, CORE::CString::Empty );
        return;
    }
    for ( size_t i=0; i<m_packageFiles.size(); ++i )
    {
        const CPackageFile& file = m_packageFiles[ i ];
        if ( file.owner == owner && file.name == name && file.version == version && file.fileName == fileName )
        {
            response.statusCode = 200;
            response.contentType = "application/octet-stream";
            response.content = file.content;
            return;
        }
    }
    Respond( response, 404, "{\"message\":\"package file not found\"}" );
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
