/*
 *  gucefMcpLocalLlm_TestApp: tests of the gucefMcpLocalLlm MCP server
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

#include <fstream>

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#include "TestMcpSupport.h"
#include "TestMcpClientSupport.h"
#include "TestMcpLocalLlm.h"

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

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

const UInt32 QuickTimeoutInMs = 30000;
const UInt32 LlmTimeoutInMs = 900000;

/*-------------------------------------------------------------------------*/

CORE::CString
FindServer( void )
{
    CORE::CString dir = CORE::RelativePath( "$MODULEDIR$" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const char* names[] = { "gucefMcpLocalLlm.exe", "gucefMcpLocalLlm_d.exe" };
    #else
    const char* names[] = { "gucefMcpLocalLlm", "gucefMcpLocalLlm_d" };
    #endif
    for ( int i=0; i<2; ++i )
    {
        CORE::CString candidate = CORE::CombinePath( dir, names[ i ] );
        if ( CORE::FileExists( candidate ) )
            return candidate;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
LlmUrl( void )
{
    CORE::CString url = CORE::GetEnv( "LOCAL_LLM_URL" );
    return url.IsNULLOrEmpty() ? CORE::CString( "http://localhost:8080" ) : url;
}

/*-------------------------------------------------------------------------*/

CORE::CString
AgentScript( void )
{
    CORE::CString script = CORE::GetEnv( "LOCAL_AGENT_SCRIPT" );
    if ( script.IsNULLOrEmpty() )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        script = CORE::RelativePath( "$MODULEDIR$/local-agent.ps1" );
        #else
        script = CORE::RelativePath( "$MODULEDIR$/local-agent.sh" );
        #endif
    }
    return script;
}

/*-------------------------------------------------------------------------*/

/** One MCP POST over HTTP(S) as a 2025-11-25 client sends it: @return the status, 0 when no response came */
Int32
McpHttpPost( const CORE::CString& url, const CORE::CString& token, const CORE::CString& body, const CORE::CString& caBundle, CORE::CString& responseText )
{
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return 0;
    request.requestType = "POST";
    request.contentRepresentation = "application/json";
    request.resourceRepresentations.push_back( "application/json" );
    request.resourceRepresentations.push_back( "text/event-stream" );
    if ( !token.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = "Bearer " + token;
    request.otherHeaders[ "MCP-Protocol-Version" ] = "2025-11-25";
    request.content.CopyFrom( body.Length(), body.C_String() );
    WEB::CHttpClientRequestOptions options;
    options.caBundlePath = caBundle;
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, options, response ) )
        return 0;
    responseText = CORE::CString( static_cast< const char* >( response.content.GetConstBufferPtr() ), response.content.GetDataSize() );
    return response.statusCode;
}

/*-------------------------------------------------------------------------*/

CORE::CProcess::TStringMap
ChildEnvironment( void )
{
    // the log of the child goes to its standard error, never to a window, and its exit code is its own
    CORE::CProcess::TStringMap environment;
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "stderr";
    TESTFW::KeepChildExitCodeUnderSanitizers( environment );
    return environment;
}

/*-------------------------------------------------------------------------*/

/** A JSON string literal */
CORE::CString
Json( const CORE::CString& text )
{
    CORE::CString result = "\"";
    const char* c = text.C_String();
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        if ( '"' == c[ i ] || '\\' == c[ i ] )
            result += '\\';
        if ( '\n' == c[ i ] )
        {
            result += "\\n";
            continue;
        }
        result += c[ i ];
    }
    return result + "\"";
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, const CORE::CString& content )
{
    std::ofstream file( path.C_String(), std::ios::out | std::ios::binary );
    if ( !file )
        return false;
    file << content.C_String();
    return file.good();
}

/*-------------------------------------------------------------------------*/

bool
ReadFile( const CORE::CString& path, CORE::CString& content )
{
    std::ifstream file( path.C_String(), std::ios::in | std::ios::binary );
    if ( !file )
        return false;
    std::string text( ( std::istreambuf_iterator< char >( file ) ), std::istreambuf_iterator< char >() );
    content = text;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Runs a command to the end. @return its exit code, -1 when it did not start */
Int32
RunCommand( const CORE::CString& exe, const CORE::CProcess::TStringVector& args, const CORE::CString& workingDir )
{
    CORE::CProcess process;
    if ( !process.Launch( exe, args, workingDir ) )
        return -1;
    Int32 exitCode = -1;
    if ( !process.WaitForExit( 60000, exitCode ) )
    {
        process.Terminate( false, 0 );
        return -1;
    }
    return exitCode;
}

/*-------------------------------------------------------------------------*/

Int32
Git( const CORE::CString& repoDir, const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "-C" );
    args.push_back( repoDir );
    const char* parts[] = { a, b, c, d };
    for ( int i=0; i<4; ++i )
        if ( GUCEF_NULL != parts[ i ] )
            args.push_back( parts[ i ] );
    return RunCommand( "git", args, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

/** A git repository with the files added; CommitAll() commits them */
bool
MakeRepo( const CORE::CString& dir, const CORE::CString::StringVector& names, const CORE::CString::StringVector& contents )
{
    CORE::CreateDirs( dir );
    for ( size_t i=0; i<names.size(); ++i )
        if ( !WriteFile( CORE::CombinePath( dir, names[ i ] ), contents[ i ] ) )
            return false;
    return 0 == Git( dir, "init", "-q" ) && 0 == Git( dir, "add", "." );
}

/*-------------------------------------------------------------------------*/

bool
CommitAll( const CORE::CString& dir )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "-C" );
    args.push_back( dir );
    args.push_back( "-c" );
    args.push_back( "user.name=test" );
    args.push_back( "-c" );
    args.push_back( "user.email=test@example.com" );
    args.push_back( "commit" );
    args.push_back( "-q" );
    args.push_back( "-m" );
    args.push_back( "fixture" );
    return 0 == RunCommand( "git", args, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

/** The server as a child process, talked to over its standard input and output */
class CServer
{
    public:

    CServer( void ) : m_client(), m_nextId( 1 ), m_started( false ) {}

    /** @param environment  variables for the server on top of ChildEnvironment() */
    bool Start( const CORE::CProcess::TStringVector& args, const CORE::CProcess::TStringMap& environment = CORE::CProcess::TStringMap() )
    {
        const CORE::CString exe = FindServer();
        CORE::CProcess::TStringMap childEnvironment = ChildEnvironment();
        for ( CORE::CProcess::TStringMap::const_iterator i = environment.begin(); i != environment.end(); ++i )
            childEnvironment[ (*i).first ] = (*i).second;
        m_started = !exe.IsNULLOrEmpty() && m_client.Launch( exe, args, childEnvironment );
        return m_started;
    }

    /** Calls a tool. @return false when no response came */
    bool Call( const CORE::CString& tool, const CORE::CString& argumentsJson, CORE::CString& text, bool& isError, UInt32 timeoutInMs = QuickTimeoutInMs )
    {
        const CORE::CString id = CORE::UInt32ToString( m_nextId++ );
        if ( !m_client.SendLine( McpTestRequest( id, "tools/call", "\"name\":" + Json( tool ) + ",\"arguments\":" + argumentsJson ) ) )
            return false;
        CORE::CDataNode response;
        GUCEF::vector< CORE::CDataNode > others;
        if ( !m_client.ReadResponse( id, response, others, timeoutInMs ) )
            return false;
        text = McpTestFirstText( response.FindChild( "result" ) );
        isError = McpTestResultAttr( response, "isError" ) == "true" || GUCEF_NULL != response.FindChild( "error" );
        return true;
    }

    /** The names of the tools, comma separated, in the order of the list */
    CORE::CString ToolNames( void )
    {
        const CORE::CString id = CORE::UInt32ToString( m_nextId++ );
        CORE::CDataNode response;
        GUCEF::vector< CORE::CDataNode > others;
        if ( !m_client.SendLine( McpTestRequest( id, "tools/list" ) ) || !m_client.ReadResponse( id, response, others, QuickTimeoutInMs ) )
            return CORE::CString::Empty;
        CORE::CString names;
        const CORE::CDataNode* tools = McpTestResultChild( response, "tools" );
        if ( GUCEF_NULL != tools )
            for ( CORE::CDataNode::const_iterator i = tools->ConstBegin(); i != tools->ConstEnd(); ++i )
                names += McpTestAttr( *(*i), "name" ) + ",";
        return names;
    }

    bool Stop( void )
    {
        if ( !m_started )
            return true;
        m_client.CloseInput();
        Int32 exitCode = -1;
        return m_client.WaitForExit( 30000, exitCode ) && 0 == exitCode;
    }

    private:

    CMcpSubprocessClient m_client;
    UInt32 m_nextId;
    bool m_started;
};

/*-------------------------------------------------------------------------*/

class CFakeLlamaServer;

/** Passes the requests of the HTTP server to a CFakeLlamaServer */
class CFakeLlamaHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeLlamaHandler( void ) : WEB::CIHttpServerRequestHandler(), m_server( GUCEF_NULL ) {}
    virtual ~CFakeLlamaHandler() {}

    void SetServer( CFakeLlamaServer* server ) { m_server = server; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeLlamaServer* m_server;
};

/*-------------------------------------------------------------------------*/

class CFakeLlamaHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeLlamaHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeLlamaHandler, MT::CMutex > TBase;

    explicit CFakeLlamaHandlerFactory( CFakeLlamaServer* server ) : TBase(), m_server( server ) {}
    virtual ~CFakeLlamaHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeLlamaHandler* handler = static_cast< CFakeLlamaHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetServer( m_server );
        return product;
    }

    private:

    CFakeLlamaServer* m_server;
};

/*-------------------------------------------------------------------------*/

/**
 *  A stand-in for llama.cpp llama-server on 127.0.0.1 that runs the given model: /health, /props, /slots and
 *  /v1/chat/completions, which answers "answer from <model>". Records the model each completion request names.
 */
class CFakeLlamaServer : public MT::CActiveObject
{
    public:

    explicit CFakeLlamaServer( const CORE::CString& model )
        : MT::CActiveObject()
        , m_model( model )
        , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , m_server( GUCEF_NULL )
        , m_factory( GUCEF_NULL )
        , m_running( false )
        , m_lock()
        , m_requestedModels()
    {
    }

    virtual ~CFakeLlamaServer()
    {
        Stop();
    }

    bool Start( void )
    {
        m_factory = GUCEF_NEW CFakeLlamaHandlerFactory( this );
        WEB::CHttpServerSettings settings;
        settings.bindAddress = "127.0.0.1";
        settings.keepAliveConnections = false;
        settings.processRequestsAsync = false;
        settings.allowWebSocketUpgrades = false;
        m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, m_factory, &settings );
        if ( !m_server->ListenOnPort( 0 ) )
            return false;
        m_running = true;
        return Activate( GUCEF_NULL, 1, 1 );
    }

    void Stop( void )
    {
        if ( !m_running )
            return;
        m_running = false;
        Deactivate( false, true );
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
        GUCEF_DELETE m_factory;
        m_factory = GUCEF_NULL;
    }

    CORE::CString GetUrl( void ) const
    {
        return "http://127.0.0.1:" + CORE::UInt16ToString( GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0 );
    }

    /** The model names the completion requests gave, comma separated */
    CORE::CString GetRequestedModels( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return m_requestedModels;
    }

    void Handle( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
    {
        std::string body;
        const CORE::CString& path = request.requestUri;
        if ( path == "/health" )
            body = "{\"status\":\"ok\"}";
        else
        if ( path == "/props" )
            body = "{\"model_alias\":\"" + m_model.STL_String() + "\",\"total_slots\":1,\"default_generation_settings\":{\"n_ctx\":131072}}";
        else
        if ( path == "/slots" )
            body = "[{\"id\":0,\"is_processing\":false}]";
        else
        if ( path == "/v1/chat/completions" )
        {
            const std::string content( (const char*) request.content.GetConstBufferPtr(), request.content.GetDataSize() );
            std::string model;
            const size_t key = content.find( "\"model\"" );
            if ( std::string::npos != key )
            {
                const size_t start = content.find( '"', content.find( ':', key ) ) + 1;
                model = content.substr( start, content.find( '"', start ) - start );
            }
            {
                MT::CScopeMutex lock( m_lock );
                m_requestedModels += CORE::CString( model ) + ",";
            }
            body = "{\"choices\":[{\"message\":{\"role\":\"assistant\",\"content\":\"answer from " + m_model.STL_String() +
                   "\"},\"finish_reason\":\"stop\"}]}";
        }
        else
        {
            response.statusCode = 404;
            return;
        }
        response.statusCode = 200;
        response.contentType = "application/json";
        response.content.Clear( true );
        response.content.CopyFrom( (UInt32) body.size(), body.c_str() );
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_NULL;
    }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->ForceDirectPulse();
        return false;
    }

    private:

    CORE::CString m_model;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    WEB::CHTTPServer* m_server;
    CFakeLlamaHandlerFactory* m_factory;
    bool m_running;
    MT::CMutex m_lock;
    CORE::CString m_requestedModels;
};

/*-------------------------------------------------------------------------*/

bool
CFakeLlamaHandler::OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    if ( GUCEF_NULL == m_server )
    {
        response.statusCode = 503;
        return true;
    }
    m_server->Handle( request, response );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Server arguments: the LLM server, the agent (AgentScript() when empty) and, for run_and_digest, the fixture script */
CORE::CProcess::TStringVector
ServerArgs( const CORE::CString& logDir, bool allowFixtureCommand, const CORE::CString& agentScript = CORE::CString::Empty )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "--endpoint" );
    args.push_back( LlmUrl() );
    args.push_back( "--agent-script" );
    args.push_back( agentScript.IsNULLOrEmpty() ? AgentScript() : agentScript );
    if ( allowFixtureCommand )
    {
        args.push_back( "--allow-command" );
        args.push_back( "bash *print-log.sh *" );
    }
    if ( !logDir.IsNULLOrEmpty() )
    {
        args.push_back( "--log-dir" );
        args.push_back( logDir );
    }
    return args;
}

/*-------------------------------------------------------------------------*/

/** A script that prints a log file and exits with the given code: the stand-in for builds and test runs */
CORE::CString
WritePrintLogScript( const CORE::CString& dir )
{
    const CORE::CString path = CORE::CombinePath( dir, "print-log.sh" );
    WriteFile( path, "#!/bin/bash\ncat \"$1\"\nexit \"${2:-0}\"\n" );
    return path;
}

/*-------------------------------------------------------------------------*/

/** run_and_digest on a fixture log */
bool
Digest( CServer& server, const CORE::CString& dir, const CORE::CString& script, const CORE::CString& logName, const CORE::CString& log,
        const char* exitCode, CORE::CString& text, bool& isError, const CORE::CString& workingDir = CORE::CString::Empty )
{
    const CORE::CString logPath = CORE::CombinePath( dir, logName );
    if ( !WriteFile( logPath, log ) )
        return false;
    const CORE::CString command = "bash " + script + " " + logPath + " " + exitCode;
    const CORE::CString runDir = workingDir.IsNULLOrEmpty() ? dir : workingDir;
    return server.Call( "run_and_digest", "{\"command\":" + Json( command ) + ",\"working_dir\":" + Json( runDir ) + "}", text, isError );
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

/** Whether the LLM server answers; the tests that need it are skipped otherwise */
bool
LlmAvailable( CServer& server )
{
    CORE::CString text;
    bool isError = true;
    return server.Call( "local_llm_status", "{}", text, isError ) && !isError;
}

/*-------------------------------------------------------------------------*/

bool
AgentAvailable( void )
{
    return CORE::FileExists( AgentScript() );
}

/*-------------------------------------------------------------------------*/

bool
GitAvailable( void )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "--version" );
    return 0 == RunCommand( "git", args, CORE::CString::Empty );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpLocalLlmTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "McpLocalLlm" );

    // the responses of the server are parsed with the JSON codec
    const bool hasServer = !FindServer().IsNULLOrEmpty() && McpTestJsonAvailable();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const bool hasBash = false;
    #else
    const bool hasBash = true;
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: The tool list: the delegate tools only with an agent script, run_and_digest only when commands are allowed" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Tools" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString agent = CORE::CombinePath( dir, "agent.sh" );      // only has to exist
            ASSERT_TRUE( WriteFile( agent, "#!/bin/bash\n" ) );
            {
                CServer server;
                ASSERT_TRUE( server.Start( ServerArgs( CORE::CString::Empty, false, CORE::CombinePath( dir, "missing.sh" ) ) ) );
                ASSERT_TRUE( server.ToolNames() == "ask_local_llm,local_llm_status," );
                ASSERT_TRUE( server.Stop() );
            }
            {
                CServer server;
                ASSERT_TRUE( server.Start( ServerArgs( CORE::CString::Empty, false, agent ) ) );
                ASSERT_TRUE( server.ToolNames() == "ask_local_llm,delegate_batch,delegate_to_local_llm,local_llm_status," );
                ASSERT_TRUE( server.Stop() );
            }
            {
                CServer server;
                ASSERT_TRUE( server.Start( ServerArgs( CORE::CString::Empty, true, agent ) ) );
                ASSERT_TRUE( server.ToolNames() == "ask_local_llm,delegate_batch,delegate_to_local_llm,local_llm_status,run_and_digest," );
                ASSERT_TRUE( server.Stop() );
            }
            {
                // no LLM server configured: only what works without one
                CServer server;
                CORE::CProcess::TStringVector args;
                args.push_back( "--agent-script" );
                args.push_back( agent );
                args.push_back( "--allow-command" );
                args.push_back( "true" );
                CORE::CProcess::TStringMap environment;
                environment[ "LOCAL_LLM_URL" ] = "";
                environment[ "LOCAL_LLM_URLS" ] = "";
                environment[ "LOCAL_LLM_EPHEMERAL_URLS" ] = "";
                ASSERT_TRUE( server.Start( args, environment ) );
                ASSERT_TRUE( server.ToolNames() == "local_llm_status,run_and_digest," );
                CORE::CString text;
                bool isError = false;
                ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
                ASSERT_TRUE( isError && Contains( text, "No LLM server is configured" ) );
                ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: GCC and Clang errors are counted once, with fatal errors, failed targets and warnings" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Gcc" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CombinePath( dir, "logs" ), true ) ) );
            const CORE::CString log =
                "[1/3] Building CXX object src/a.cpp.o\n"
                "FAILED: src/CMakeFiles/app.dir/a.cpp.o\n" +
                dir + "/src/a.cpp:10:5: error: 'foo' was not declared in this scope\n" +
                dir + "/src/a.cpp:10:5: error: 'foo' was not declared in this scope\n" +
                dir + "/src/b.cpp:3:1: warning: unused variable 'x' [-Wunused-variable]\n"
                "In file included from /elsewhere/c.cpp:1:\n"
                "/elsewhere/c.h:7:12: fatal error: missing.h: No such file or directory\n"
                "\x1b[1;31mclang: error: linker command failed with exit code 1\x1b[0m\n"
                "ninja: build stopped: subcommand failed.\n";
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( Digest( server, dir, script, "gcc.log", log, "1", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "Failed (exit 1" ) );
            ASSERT_TRUE( Contains( text, "Errors: 4 (3 distinct)" ) );
            ASSERT_TRUE( Contains( text, "  src/a.cpp:10:5: 'foo' was not declared in this scope [x2]" ) );     // relative to working_dir
            ASSERT_TRUE( Contains( text, "/elsewhere/c.h:7:12: fatal: missing.h: No such file or directory" ) );
            ASSERT_TRUE( Contains( text, "clang: error: linker command failed with exit code 1" ) );             // colour codes removed
            ASSERT_TRUE( Contains( text, "Failed targets (1):\n  src/CMakeFiles/app.dir/a.cpp.o" ) );
            ASSERT_TRUE( Contains( text, "Warnings: 1 (1 distinct)\n  src/b.cpp:3:1: unused variable 'x' [-Wunused-variable]" ) );
            ASSERT_FALSE( Contains( text, "In file included from" ) );

            // run in a build directory: the sources next to it are shown as ../
            const CORE::CString buildDir = CORE::CombinePath( dir, "build" );
            CORE::CreateDirs( buildDir );
            ASSERT_TRUE( Digest( server, dir, script, "build.log", dir + "/src/a.cpp:4:2: error: expected ';'\n", "1", text, isError, buildDir ) );
            ASSERT_TRUE( Contains( text, "  ../src/a.cpp:4:2: expected ';'" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: MSVC compiler and linker errors, MSBuild project suffixes" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Msvc" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CombinePath( dir, "logs" ), true ) ) );
            const CORE::CString log =
                "C:\\src\\app\\main.cpp(12,7): error C2065: 'count': undeclared identifier [C:\\src\\app\\app.vcxproj]\n"
                "C:\\src\\app\\main.cpp(12,7): error C2065: 'count': undeclared identifier [C:\\src\\app\\app.vcxproj]\n"
                "C:\\src\\app\\util.h(40): warning C4996: 'strcpy': This function may be unsafe. [C:\\src\\app\\app.vcxproj]\n"
                "C:\\src\\app\\gen.cpp(1): fatal error C1083: Cannot open include file: 'gen.h': No such file or directory\n"
                "main.obj : error LNK2019: unresolved external symbol \"void __cdecl run(void)\" referenced in function main\n"
                "LINK : fatal error LNK1120: 1 unresolved externals\n";
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( Digest( server, dir, script, "msvc.log", log, "1", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "Errors: 5 (4 distinct)" ) );
            ASSERT_TRUE( Contains( text, "C:\\src\\app\\main.cpp:12:7: C2065: 'count': undeclared identifier [x2]" ) );
            ASSERT_TRUE( Contains( text, "C:\\src\\app\\gen.cpp:1: fatal C1083: Cannot open include file" ) );
            ASSERT_TRUE( Contains( text, "main.obj: error LNK2019: unresolved external symbol" ) );
            ASSERT_TRUE( Contains( text, "LINK: fatal error LNK1120: 1 unresolved externals" ) );
            ASSERT_TRUE( Contains( text, "C:\\src\\app\\util.h:40: C4996: 'strcpy': This function may be unsafe." ) );
            ASSERT_FALSE( Contains( text, "vcxproj" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: CMake messages, GNU ld, make, exceptions" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_CMake" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CombinePath( dir, "logs" ), true ) ) );
            const CORE::CString log =
                "-- Configuring incomplete, errors occurred!\n"
                "CMake Error at CMakeLists.txt:12 (add_executable):\n"
                "  Cannot find source file:\n"
                "\n"
                "    src/missing.cpp\n"
                "\n"
                "CMake Warning (dev) at cmake/deps.cmake:3 (message):\n"
                "  Policy CMP0048 is not set\n"
                "\n"
                "/usr/bin/ld: obj/a.o: in function `main':\n"
                "a.cpp:(.text+0x1f): undefined reference to `helper()'\n"
                "/usr/bin/ld: cannot find -lfoo: No such file or directory\n"
                "make[2]: *** [CMakeFiles/app.dir/build.make:97: app] Error 1\n"
                "ModuleNotFoundError: No module named 'yaml'\n"
                "java.lang.IllegalStateException: not ready\n";
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( Digest( server, dir, script, "cmake.log", log, "2", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "CMakeLists.txt:12: Cannot find source file: src/missing.cpp" ) );
            ASSERT_TRUE( Contains( text, "cmake/deps.cmake:3: Policy CMP0048 is not set" ) );
            ASSERT_TRUE( Contains( text, "a.cpp: undefined reference to `helper()'" ) );
            ASSERT_TRUE( Contains( text, "/usr/bin/ld: cannot find -lfoo" ) );
            ASSERT_TRUE( Contains( text, "Failed targets (1):\n  CMakeFiles/app.dir/build.make:97: app" ) );
            ASSERT_TRUE( Contains( text, "ModuleNotFoundError: No module named 'yaml'" ) );
            ASSERT_TRUE( Contains( text, "java.lang.IllegalStateException: not ready" ) );
            ASSERT_FALSE( Contains( text, "Configuring incomplete" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: CTest and GUCEF test framework failures, test summary" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Tests" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CombinePath( dir, "logs" ), true ) ) );
            const CORE::CString log =
                "Test project /work/build\n"
                "    Start 1: core\n"
                "1/3 Test #1: core .............................   Passed    0.10 sec\n"
                "2026-11-11T08:00:00.000Z|1|ERROR|NORMAL|    ASSERTION FAILED: value == 3 at /work/tests/TestCore.cpp:88\n"
                "2026-11-11T08:00:00.000Z|1|WARNING|NORMAL|  Test END: Test: core values - FAILED (1 failures)\n"
                "67% tests passed, 1 tests failed out of 3\n"
                "\n"
                "Total Test time (real) =   0.35 sec\n"
                "\n"
                "The following tests FAILED:\n"
                "\t  2 - net (Failed)\n"
                "Errors while running CTest\n";
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( Digest( server, dir, script, "ctest.log", log, "8", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "/work/tests/TestCore.cpp:88: ASSERTION FAILED: value == 3" ) );
            ASSERT_TRUE( Contains( text, "Failed tests (2):\n  Test: core values - FAILED (1 failures)\n  2 - net (Failed)" ) );
            ASSERT_TRUE( Contains( text, "Tests: 67% tests passed, 1 tests failed out of 3" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: success, the full log is kept, refused commands do not run" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Run" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );
            const CORE::CString logDir = CORE::CombinePath( dir, "logs" );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( logDir, true ) ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( Digest( server, dir, script, "ok.log", "[2/2] Linking CXX executable app\nall good\n", "0", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Succeeded (exit 0" ) );
            ASSERT_TRUE( Contains( text, "2 lines of output" ) );
            ASSERT_FALSE( Contains( text, "Errors" ) );

            // the full output is in the log that the result names
            const std::string result = text.STL_String();
            const size_t logAt = result.find( "full log: " );
            ASSERT_TRUE( std::string::npos != logAt );
            const size_t logEnd = result.find( ')', logAt );
            const CORE::CString logPath = result.substr( logAt + 10, logEnd - logAt - 10 );
            CORE::CString logContent;
            ASSERT_TRUE( ReadFile( logPath, logContent ) );
            ASSERT_TRUE( Contains( logContent, "all good" ) && Contains( logContent, "$ bash " ) );

            // not run: neither the wrong program nor the allowed program with a shell around it
            const CORE::CString marker = CORE::CombinePath( dir, "marker" );
            ASSERT_TRUE( server.Call( "run_and_digest", "{\"command\":" + Json( "touch " + marker ) + ",\"working_dir\":" + Json( dir ) + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "not allowed" ) );
            ASSERT_TRUE( server.Call( "run_and_digest", "{\"command\":" + Json( "sh -c 'bash x/print-log.sh y; touch " + marker + "'" ) + ",\"working_dir\":" + Json( dir ) + "}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "not allowed" ) );
            ASSERT_FALSE( CORE::FileExists( marker ) );

            // the pattern allows bash with the script; the rest is passed as arguments, never to a shell
            ASSERT_TRUE( server.Call( "run_and_digest", "{\"command\":" + Json( "bash " + script + " " + CORE::CombinePath( dir, "ok.log" ) + " 0 ; touch " + marker ) + ",\"working_dir\":" + Json( dir ) + "}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_FALSE( CORE::FileExists( marker ) );

            ASSERT_TRUE( server.Call( "run_and_digest", "{\"command\":\"bash x/print-log.sh\",\"working_dir\":\"/nonexistent/dir\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "working_dir" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: Invalid arguments are refused before any LLM is used" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Invalid" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );

            const CORE::CString agent = CORE::CombinePath( dir, "agent.sh" );      // only has to exist
            ASSERT_TRUE( WriteFile( agent, "#!/bin/bash\n" ) );
            CServer server;
            CORE::CProcess::TStringVector args;
            args.push_back( "--endpoint" );
            args.push_back( "http://127.0.0.1:9" );     // nothing listens there
            args.push_back( "--agent-script" );
            args.push_back( agent );
            ASSERT_TRUE( server.Start( args ) );
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( server.Call( "delegate_to_local_llm", "{\"repo_dir\":\"/nonexistent/dir\",\"task\":\"x\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "repo_dir" ) );
            ASSERT_TRUE( server.Call( "delegate_to_local_llm", "{\"repo_dir\":" + Json( dir ) + ",\"task\":\"   \"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "task is empty" ) );
            ASSERT_TRUE( server.Call( "delegate_batch", "{\"repo_dir\":" + Json( dir ) + ",\"tasks\":[\"a\",\"\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "task 2 is empty" ) );
            ASSERT_TRUE( server.Call( "ask_local_llm", "{\"question\":\" \",\"paths\":[\"a\"]}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "question is empty" ) );

            // a valid request with no usable server says so, so the caller does the work itself
            ASSERT_TRUE( server.Call( "delegate_to_local_llm", "{\"repo_dir\":" + Json( dir ) + ",\"task\":\"x\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "No local LLM server is usable" ) && Contains( text, "http://127.0.0.1:9 (this machine): not reachable" ) );
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "Delegation is not available" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: Endpoint order: this machine, then the others, then ephemeral ones of other machines; unreachable ones are not checked again at once" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Endpoints" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "a.txt" ), "text\n" ) );

            // documentation address ranges (RFC 5737): never this machine, nothing answers there
            CServer server;
            CORE::CProcess::TStringVector args;
            args.push_back( "--ephemeral-endpoint" );
            args.push_back( "http://192.0.2.1:8080" );
            args.push_back( "--endpoint" );
            args.push_back( "http://198.51.100.1:8080" );
            args.push_back( "--endpoint" );
            args.push_back( "http://127.0.0.1:9" );
            ASSERT_TRUE( server.Start( args ) );
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "1. http://127.0.0.1:9 (this machine): not reachable" ) );
            ASSERT_TRUE( Contains( text, "2. http://198.51.100.1:8080: not reachable" ) );
            ASSERT_TRUE( Contains( text, "3. http://192.0.2.1:8080 (ephemeral: not always on): not reachable" ) );

            // just checked: the next request does not wait for them again
            const UInt64 start = MT::PrecisionTickCount();
            ASSERT_TRUE( server.Call( "ask_local_llm", "{\"question\":\"q\",\"paths\":[\"a.txt\"],\"base_dir\":" + Json( dir ) + "}", text, isError ) );
            const UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
            ASSERT_TRUE( isError && Contains( text, "No local LLM server is usable" ) );
            ASSERT_TRUE( Contains( text, "2. http://198.51.100.1:8080: not reachable" ) && Contains( text, "s ago" ) );
            ASSERT_TRUE( elapsedMs < 2000 );
            ASSERT_TRUE( server.Stop() );

            // on the ephemeral machine itself its endpoint is this machine's, and comes first
            CServer onEphemeral;
            args.clear();
            args.push_back( "--endpoint" );
            args.push_back( "http://198.51.100.1:8080" );
            args.push_back( "--ephemeral-endpoint" );
            args.push_back( "http://127.0.0.1:9" );
            ASSERT_TRUE( onEphemeral.Start( args ) );
            ASSERT_TRUE( onEphemeral.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_TRUE( Contains( text, "1. http://127.0.0.1:9 (this machine): not reachable" ) );
            ASSERT_TRUE( Contains( text, "2. http://198.51.100.1:8080: not reachable" ) );
            ASSERT_FALSE( Contains( text, "ephemeral" ) );
            ASSERT_TRUE( onEphemeral.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: Model routing: a call naming a model goes to a server running it, the configured model is preferred, unknown models are refused" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Models" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "a.txt" ), "text\n" ) );
            const CORE::CString ask = "{\"question\":\"q\",\"paths\":[\"a.txt\"],\"base_dir\":" + Json( dir );

            CFakeLlamaServer serverA( "model-a" );
            CFakeLlamaServer serverB( "Model-B" );
            ASSERT_TRUE( serverA.Start() );
            ASSERT_TRUE( serverB.Start() );

            CServer server;
            CORE::CProcess::TStringVector args;
            args.push_back( "--endpoint" );
            args.push_back( serverA.GetUrl() );
            args.push_back( "--endpoint" );
            args.push_back( serverB.GetUrl() );
            ASSERT_TRUE( server.Start( args ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Models: model-a (1 server, 1 idle slot), Model-B (1 server, 1 idle slot)" ) );

            // named: only that server, the name is matched without case; each request names the model of its server
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + ",\"model\":\"model-b\"}", text, isError ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "answer from Model-B" ) && Contains( text, "(local LLM Model-B, " ) );
            ASSERT_TRUE( serverB.GetRequestedModels() == "Model-B," );
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + ",\"model\":\"MODEL-A\"}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "answer from model-a" ) );

            // not named: the first server
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "answer from model-a" ) );
            ASSERT_TRUE( serverA.GetRequestedModels() == "model-a,model-a," );

            // a model no server runs: refused, with the models there are
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + ",\"model\":\"model-c\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "No reachable LLM server runs model 'model-c'" ) );
            ASSERT_TRUE( Contains( text, "Reachable models: model-a (1 server, 1 idle slot), Model-B (1 server, 1 idle slot)" ) );

            // the server of a model is gone: its model is refused, the other still works
            serverB.Stop();
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + ",\"model\":\"model-b\"}", text, isError ) );
            ASSERT_TRUE( isError && Contains( text, "No reachable LLM server runs model 'model-b'" ) );
            ASSERT_TRUE( server.Call( "ask_local_llm", ask + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "answer from model-a" ) );
            ASSERT_TRUE( server.Stop() );
            ASSERT_TRUE( serverB.Start() );

            // --model: preferred when a call names none; any other when no server runs it
            CServer preferring;
            args.push_back( "--model" );
            args.push_back( "model-b" );
            CORE::CProcess::TStringVector argsB = args;
            argsB[ 3 ] = serverB.GetUrl();
            ASSERT_TRUE( preferring.Start( argsB ) );
            ASSERT_TRUE( preferring.Call( "ask_local_llm", ask + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "answer from Model-B" ) );
            ASSERT_TRUE( preferring.Stop() );
            CServer fallback;
            argsB[ 5 ] = "model-x";
            ASSERT_TRUE( fallback.Start( argsB ) );
            ASSERT_TRUE( fallback.Call( "ask_local_llm", ask + "}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "answer from model-a" ) );
            ASSERT_TRUE( fallback.Stop() );

            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            // the agent of a delegation gets the server and the model of that server
            const CORE::CString agent = CORE::CombinePath( dir, "agent.sh" );
            ASSERT_TRUE( WriteFile( agent, "#!/bin/bash\necho \"agent url=$LOCAL_LLM_URL model=$LOCAL_LLM_MODEL\"\n" ) );
            CServer delegating;
            argsB.resize( 4 );
            argsB.push_back( "--agent-script" );
            argsB.push_back( agent );
            ASSERT_TRUE( delegating.Start( argsB ) );
            ASSERT_TRUE( delegating.Call( "delegate_to_local_llm", "{\"repo_dir\":" + Json( dir ) + ",\"task\":\"t\",\"model\":\"model-b\"}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "agent url=" + serverB.GetUrl() + " model=Model-B" ) );
            ASSERT_TRUE( delegating.Call( "delegate_batch", "{\"repo_dir\":" + Json( dir ) + ",\"tasks\":[\"t1\",\"t2\"],\"model\":\"model-a\"}", text, isError ) );
            ASSERT_TRUE( !isError && Contains( text, "2 of 2 tasks succeeded" ) );
            ASSERT_TRUE( Contains( text, "[1] ok (" + serverA.GetUrl() ) && Contains( text, "[2] ok (" + serverA.GetUrl() ) );
            ASSERT_FALSE( Contains( text, "model=Model-B" ) );
            ASSERT_TRUE( delegating.Stop() );
            #endif
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

    GUCEF_TESTFW_TESTCASE( "Test: The endpoints file is read again when it changes: servers come and go without a restart" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_EndpointsFile" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString file = CORE::CombinePath( dir, "endpoints" );

            CFakeLlamaServer serverA( "model-a" );
            CFakeLlamaServer serverB( "model-b" );
            ASSERT_TRUE( serverA.Start() );
            ASSERT_TRUE( serverB.Start() );
            ASSERT_TRUE( WriteFile( file, "# the servers\n" + serverA.GetUrl() + "\n" ) );

            // only the servers of the file: not those the environment of the tests names
            CORE::CProcess::TStringMap environment;
            environment[ "LOCAL_LLM_URL" ] = "";
            environment[ "LOCAL_LLM_URLS" ] = "";
            environment[ "LOCAL_LLM_EPHEMERAL_URLS" ] = "";
            CServer server;
            CORE::CProcess::TStringVector args;
            args.push_back( "--endpoints-file" );
            args.push_back( file );
            ASSERT_TRUE( server.Start( args, environment ) );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_TRUE( Contains( text, "model-a" ) && !Contains( text, "model-b" ) );

            // another size, so the change shows also within the resolution of the file time
            MT::ThreadDelay( 20 );
            ASSERT_TRUE( WriteFile( file, serverB.GetUrl() + "/\n  ephemeral " + serverA.GetUrl() + "   # a workstation\n" ) );
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            // both are on this machine, in the order of the file; an ephemeral server on this machine counts as this machine's
            ASSERT_TRUE( Contains( text, CORE::CString( "1. " ) + serverB.GetUrl() + " (this machine)" ) );
            ASSERT_TRUE( Contains( text, CORE::CString( "2. " ) + serverA.GetUrl() + " (this machine)" ) );

            // a file that is gone lists no server
            ASSERT_TRUE( CORE::DeleteFile( file ) );
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_FALSE( Contains( text, "model-a" ) || Contains( text, "model-b" ) );
            ASSERT_TRUE( server.Stop() );
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
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
            TESTFW::CTlsTestCerts certs;
            GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Http" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString tokenFile = CORE::CombinePath( dir, "token" );

            CFakeLlamaServer llama( "model-a" );
            ASSERT_TRUE( llama.Start() );

            const UInt16 plainPort = TESTFW::CTlsTestServer::FindFreePort();
            UInt16 tlsPort = TESTFW::CTlsTestServer::FindFreePort();
            for ( int i=0; i<10 && tlsPort == plainPort; ++i )
                tlsPort = TESTFW::CTlsTestServer::FindFreePort();
            CORE::CProcess::TStringVector args;
            args.push_back( "--listen" ); args.push_back( "127.0.0.1:" + CORE::UInt16ToString( plainPort ) );
            args.push_back( "--listen-tls" ); args.push_back( "127.0.0.1:" + CORE::UInt16ToString( tlsPort ) );
            args.push_back( "--tls-cert" ); args.push_back( certs.GetPath( "good.crt" ) );
            args.push_back( "--tls-key" ); args.push_back( certs.GetPath( "good.key" ) );
            args.push_back( "--token-file" ); args.push_back( tokenFile );
            args.push_back( "--endpoint" ); args.push_back( llama.GetUrl() );
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
            token = token.Trim( true ).Trim( false );
            ASSERT_TRUE( token.Length() >= 32 );

            const CORE::CString initialize = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"protocolVersion\":\"2025-11-25\","
                                             "\"capabilities\":{},\"clientInfo\":{\"name\":\"test\",\"version\":\"1\"}}}";
            const CORE::CString status = "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":{\"name\":\"local_llm_status\",\"arguments\":{}}}";
            const CORE::CString urls[] = { "http://127.0.0.1:" + CORE::UInt16ToString( plainPort ) + "/mcp", "https://localhost:" + CORE::UInt16ToString( tlsPort ) + "/mcp" };
            for ( int u=0; u<2; ++u )
            {
                CORE::CString text;
                ASSERT_TRUE( 200 == McpHttpPost( urls[ u ], token, initialize, certs.GetPath( "ca.crt" ), text ) );
                ASSERT_TRUE( Contains( text, "They run on the machine " ) );
                ASSERT_TRUE( 200 == McpHttpPost( urls[ u ], token, status, certs.GetPath( "ca.crt" ), text ) );
                ASSERT_TRUE( Contains( text, "model-a" ) );
                ASSERT_TRUE( 401 == McpHttpPost( urls[ u ], "wrong", status, certs.GetPath( "ca.crt" ), text ) );
            }

            // a client that does not trust the certificate gets no answer over HTTPS
            CORE::CString text;
            ASSERT_TRUE( 200 != McpHttpPost( urls[ 1 ], token, status, certs.GetPath( "other_ca.crt" ), text ) );

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

    GUCEF_TESTFW_TESTCASE( "Test: Serving options that do not fit together are refused at start" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            const char* sets[][ 4 ] = { { "--listen", "127.0.0.1:1", GUCEF_NULL, GUCEF_NULL },
                                        { "--listen-tls", "127.0.0.1:1", "--token-file", "/nonexistent/token" },
                                        { "--listen", "nowhere", "--token-file", "/nonexistent/token" } };
            for ( int i=0; i<3; ++i )
            {
                CORE::CProcess::TStringVector args;
                for ( int a=0; a<4 && GUCEF_NULL != sets[ i ][ a ]; ++a )
                    args.push_back( sets[ i ][ a ] );
                CORE::CProcess process;
                process.SetIsolateFromParent( true );
                ASSERT_TRUE( process.Launch( FindServer(), args, CORE::CString::Empty, ChildEnvironment() ) );
                Int32 exitCode = 0;
                ASSERT_TRUE( process.WaitForExit( 30000, exitCode ) );
                ASSERT_TRUE( 0 != exitCode );
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

    GUCEF_TESTFW_TESTCASE( "Test: local_llm_status and ask_local_llm: answers from the files with citations (needs an LLM server)" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer, "tools/gucefMcpLocalLlm is not next to the test application" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Ask" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );

            CServer server;
            CORE::CProcess::TStringVector args;
            args.push_back( "--endpoint" );
            args.push_back( "http://127.0.0.1:9" );     // unreachable, listed first: the next one is used
            args.push_back( "--endpoint" );
            args.push_back( LlmUrl() );
            args.push_back( "--agent-script" );
            args.push_back( AgentScript() );
            ASSERT_TRUE( server.Start( args ) );
            GUCEF_TESTFW_SKIP_IF( !LlmAvailable( server ), "no LLM server at LOCAL_LLM_URL" );

            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "local_llm_status", "{}", text, isError ) );
            ASSERT_FALSE( isError );
            // both may be this machine; then they keep the order given
            ASSERT_TRUE( Contains( text, "1. http://127.0.0.1:9 (this machine): not reachable" ) && Contains( text, "2. " + LlmUrl() ) );
            ASSERT_TRUE( Contains( text, ": reachable, model" ) );

            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "facts.txt" ), "first line\nsecond line\nThe frobnicator listens on port 7431.\nlast line\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "other.txt" ), "The frobnicator was written in 2019.\n" ) );
            ASSERT_TRUE( server.Call( "ask_local_llm", "{\"question\":\"Which port does the frobnicator listen on?\",\"paths\":[\"facts.txt\",\"other.txt\",\"missing.txt\"],\"base_dir\":" + Json( dir ) + "}", text, isError, LlmTimeoutInMs ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "7431" ) );
            ASSERT_TRUE( Contains( text, "facts.txt:3" ) );
            ASSERT_TRUE( Contains( text, "2 file(s), 5 lines" ) );
            ASSERT_TRUE( Contains( text, "Not read:\n  missing.txt: cannot be read" ) );

            // a line range: only line 1 to 2 is read, which does not hold the port
            ASSERT_TRUE( server.Call( "ask_local_llm", "{\"question\":\"Which port does the frobnicator listen on? Answer UNKNOWN if the files do not say.\",\"paths\":[\"facts.txt:1-2\"],\"base_dir\":" + Json( dir ) + "}", text, isError, LlmTimeoutInMs ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "1 file(s), 2 lines" ) );
            ASSERT_FALSE( Contains( text, "7431" ) );

            // wildcards: ** across directories, * within one; hidden directories are not searched
            CORE::CreateDirs( CORE::CombinePath( dir, "src/net" ) );
            CORE::CreateDirs( CORE::CombinePath( dir, "src/core/deep" ) );
            CORE::CreateDirs( CORE::CombinePath( dir, "src/.hidden" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "src/net/socket.cpp" ), "// opens sockets\nint OpenSocket( void ) { return 7; }\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "src/core/deep/timer.cpp" ), "// timers\nint StartTimer( void ) { return 9; }\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "src/core/notes.txt" ), "not code\n" ) );
            ASSERT_TRUE( WriteFile( CORE::CombinePath( dir, "src/.hidden/secret.cpp" ), "int Hidden( void );\n" ) );
            ASSERT_TRUE( server.Call( "ask_local_llm", "{\"question\":\"List the names of the functions defined in these files.\",\"paths\":[\"src/**/*.cpp\",\"src/*.h\"],\"base_dir\":" + Json( dir ) + "}", text, isError, LlmTimeoutInMs ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "2 file(s), 4 lines" ) );
            ASSERT_TRUE( Contains( text, "OpenSocket" ) && Contains( text, "StartTimer" ) );
            ASSERT_FALSE( Contains( text, "Hidden" ) );
            ASSERT_TRUE( Contains( text, "src/*.h: matches no files" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: run_and_digest: a failure in an unknown format is explained by the LLM (needs an LLM server)" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !hasBash, "needs tools/gucefMcpLocalLlm and bash" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Explain" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString script = WritePrintLogScript( dir );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CombinePath( dir, "logs" ), true ) ) );
            GUCEF_TESTFW_SKIP_IF( !LlmAvailable( server ), "no LLM server at LOCAL_LLM_URL" );
            CORE::CString text;
            bool isError = false;
            ASSERT_TRUE( Digest( server, dir, script, "odd.log", "step 1 done\nstep 2 done\ncould not open the database file data/app.db: permission denied\naborting\n", "3", text, isError ) );
            ASSERT_TRUE( isError );
            ASSERT_TRUE( Contains( text, "Failed (exit 3" ) );
            ASSERT_TRUE( Contains( text, "The local LLM read the end of the output" ) );
            ASSERT_TRUE( Contains( text, "permission" ) );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: delegate_to_local_llm edits a file and returns the diff (needs an LLM server and the agent)" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !GitAvailable(), "needs tools/gucefMcpLocalLlm and git" );
            GUCEF_TESTFW_SKIP_IF( !AgentAvailable(), "no agent script: set LOCAL_AGENT_SCRIPT" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Delegate" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CString::StringVector names, contents;
            names.push_back( "greet.txt" );
            contents.push_back( "hello world\n" );
            ASSERT_TRUE( MakeRepo( dir, names, contents ) && CommitAll( dir ) );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CString::Empty, false ) ) );
            GUCEF_TESTFW_SKIP_IF( !LlmAvailable( server ), "no LLM server at LOCAL_LLM_URL" );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "delegate_to_local_llm", "{\"repo_dir\":" + Json( dir ) + ",\"task\":\"In greet.txt replace the word world with the word GUCEF. Change nothing else.\"}", text, isError, LlmTimeoutInMs ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "Agent (" ) && Contains( text, "exit 0" ) );
            ASSERT_TRUE( Contains( text, "git diff:" ) && Contains( text, "+hello GUCEF" ) );   // a small diff is part of the result
            ASSERT_FALSE( Contains( text, "claude.ai connectors" ) );                           // local setup noise is filtered
            CORE::CString content;
            ASSERT_TRUE( ReadFile( CORE::CombinePath( dir, "greet.txt" ), content ) && content == "hello GUCEF\n" );
            ASSERT_TRUE( server.Stop() );
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

    GUCEF_TESTFW_TESTCASE( "Test: delegate_batch runs the tasks and reports each, then the combined change (needs an LLM server and the agent)" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !hasServer || !GitAvailable(), "needs tools/gucefMcpLocalLlm and git" );
            GUCEF_TESTFW_SKIP_IF( !AgentAvailable(), "no agent script: set LOCAL_AGENT_SCRIPT" );
            TESTFW::CScopedTestDir scratch( "gucefMcpLocalLlm_TestApp_Batch" );
            const CORE::CString dir = scratch.GetPath();
            CORE::CString::StringVector names, contents;
            names.push_back( "one.txt" );
            contents.push_back( "colour: red\n" );
            names.push_back( "two.txt" );
            contents.push_back( "colour: blue\n" );
            ASSERT_TRUE( MakeRepo( dir, names, contents ) && CommitAll( dir ) );

            CServer server;
            ASSERT_TRUE( server.Start( ServerArgs( CORE::CString::Empty, false ) ) );
            GUCEF_TESTFW_SKIP_IF( !LlmAvailable( server ), "no LLM server at LOCAL_LLM_URL" );
            CORE::CString text;
            bool isError = true;
            ASSERT_TRUE( server.Call( "delegate_batch", "{\"repo_dir\":" + Json( dir ) + ",\"tasks\":["
                                      "\"In one.txt replace red with green. Change nothing else and no other file.\","
                                      "\"In two.txt replace blue with yellow. Change nothing else and no other file.\"]}", text, isError, 2 * LlmTimeoutInMs ) );
            ASSERT_FALSE( isError );
            ASSERT_TRUE( Contains( text, "2 of 2 tasks succeeded" ) );
            ASSERT_TRUE( Contains( text, "[1] ok (" ) && Contains( text, "[2] ok (" ) );
            ASSERT_TRUE( Contains( text, "+colour: green" ) && Contains( text, "+colour: yellow" ) );
            CORE::CString content;
            ASSERT_TRUE( ReadFile( CORE::CombinePath( dir, "one.txt" ), content ) && content == "colour: green\n" );
            ASSERT_TRUE( ReadFile( CORE::CombinePath( dir, "two.txt" ), content ) && content == "colour: yellow\n" );
            ASSERT_TRUE( server.Stop() );
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
