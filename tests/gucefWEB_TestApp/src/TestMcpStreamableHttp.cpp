/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COM_H
#include "gucefCOM.h"
#define GUCEF_COM_H
#endif /* GUCEF_COM_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#include "gucefWEB_CMcpStreamableHttpTransport.h"
#define GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H
#endif /* GUCEF_WEB_CMCPSTREAMABLEHTTPTRANSPORT_H ? */

#ifndef GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H
#include "gucefWEB_CMcpHttpExchangeChannel.h"
#define GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H
#endif /* GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H ? */

#ifndef GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#include "gucefWEB_CMcpStaticBearerAuthenticator.h"
#define GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#endif /* GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#include "gucefWEB_CHttpClientRequestTask.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#include "TestHttpRawClient.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H ? */

#ifndef GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H
#include "../../gucefCOM_TestApp/include/TestMcpSupport.h"
#define GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H
#endif /* GUCEF_COM_TESTAPP_TESTMCPSUPPORT_H ? */

#include "TestMcpStreamableHttp.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

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

using namespace GUCEF;

static const char* McpHttpTestToken = "mcp-http-test-token-0123456789";
static const char* McpHttpAccept = "application/json, text/event-stream";

/*-------------------------------------------------------------------------*/

static bool
McpHttpSame( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: expected \"" << expected << "\" but got \"" << actual << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
McpHttpSameInt( Int32 actual, Int32 expected, const char* what )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: expected " << what << " " << expected << " but got " << actual );
    return false;
}

/*-------------------------------------------------------------------------*/

/** A tool whose arguments are mirrored into Mcp-Param headers */
class CMcpHttpParamTool : public COM::CIMcpTool
{
    public:

    CMcpHttpParamTool( void ) : COM::CIMcpTool(), executions( 0 ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        descriptor.name = "query";
        descriptor.description = "A tool with header parameters";
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        CORE::CDataNode schema;
        McpTestParse( "{\"type\":\"object\",\"properties\":{\"region\":{\"type\":\"string\",\"x-mcp-header\":\"Region\"},"
                      "\"limit\":{\"type\":\"integer\",\"x-mcp-header\":\"Limit\"},\"dry\":{\"type\":\"boolean\",\"x-mcp-header\":\"Dry\"},"
                      "\"sql\":{\"type\":\"string\"}}}", schema );
        descriptor.inputSchema = schema;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context, const CORE::CDataNode& arguments,
                                            COM::CMcpToolResult& result, COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++executions;
        result.AddText( "ran" );
        return COM::MCP_EXECUTE_COMPLETE;
    }

    UInt32 executions;
};

/*-------------------------------------------------------------------------*/

/** The outcomes of the requests the server finished */
class CMcpHttpOutcomeRecorder : public CORE::CObserver
{
    public:

    CMcpHttpOutcomeRecorder( void ) : CORE::CObserver(), outcomes(), methods() {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == COM::CMcpServer::RequestFinishedEvent )
        {
            const COM::SMcpRequestEventInfo& info = static_cast< COM::TMcpRequestEventData* >( eventdata )->GetData();
            outcomes.push_back( info.outcome );
            methods.push_back( info.method );
        }
    }

    bool HasOutcome( const CORE::CString& method, COM::EMcpRequestOutcome outcome ) const
    {
        for ( size_t i=0; i<outcomes.size(); ++i )
            if ( methods[ i ] == method && outcomes[ i ] == outcome )
                return true;
        return false;
    }

    GUCEF::vector< COM::EMcpRequestOutcome > outcomes;
    CORE::CString::StringVector methods;
};

/*-------------------------------------------------------------------------*/

/** An MCP server on the application's pulse generator with the Streamable HTTP transport made through the factory */
class CMcpHttpTestRig
{
    public:

    CMcpHttpTestRig( void )
        : server( GUCEF_NULL )
        , transportPtr()
        , transport( GUCEF_NULL )
        , echo( GUCEF_NULL )
        , count( GUCEF_NULL )
        , ask( GUCEF_NULL )
        , params( GUCEF_NULL )
        , recorder()
    {}

    ~CMcpHttpTestRig()
    {
        if ( GUCEF_NULL != server )
        {
            recorder.UnsubscribeAllFromObserver();
            server->Stop( false );
            TestHttpPumpFor( 20 );
            GUCEF_DELETE server;
            server = GUCEF_NULL;
        }
    }

    bool Start( const WEB::CMcpHttpTransportSettings* settings = GUCEF_NULL, bool anonymous = false, const COM::CMcpServerSettings* serverSettings = GUCEF_NULL )
    {
        server = GUCEF_NEW COM::CMcpServer();
        if ( GUCEF_NULL != serverSettings )
            server->SetSettings( *serverSettings );
        COM::CMcpImplementationInfo info;
        info.name = "http-test-server";
        info.version = "1.0";
        server->SetServerInfo( info );
        recorder.SubscribeTo( server );

        CORE::CString error;
        echo = GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO );
        server->GetToolRegistry().Register( COM::CIMcpToolPtr( echo ), error );
        count = GUCEF_NEW CMcpTestTool( "count", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
        server->GetToolRegistry().Register( COM::CIMcpToolPtr( count ), error );
        ask = GUCEF_NEW CMcpTestTool( "ask_name", CMcpTestTool::ASK_NAME );
        server->GetToolRegistry().Register( COM::CIMcpToolPtr( ask ), error );
        params = GUCEF_NEW CMcpHttpParamTool();
        server->GetToolRegistry().Register( COM::CIMcpToolPtr( params ), error );

        transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( COM::CIMcpTransport::StreamableHttpTransportName );
        if ( transportPtr.IsNULL() )
            return false;
        transport = static_cast< WEB::CMcpStreamableHttpTransport* >( transportPtr.GetPointerAlways() );
        if ( GUCEF_NULL != settings )
            transport->SetSettings( *settings );
        if ( anonymous )
        {
            transport->SetAuthenticator( WEB::CWebGlobal::Instance()->GetMcpHttpAuthenticatorFactory().Create( WEB::CIMcpHttpAuthenticator::NoAuthenticatorName ) );
        }
        else
        {
            // made by the factory, as the transport would, and given its token in code
            WEB::CIMcpHttpAuthenticatorPtr authenticator = WEB::CWebGlobal::Instance()->GetMcpHttpAuthenticatorFactory().Create( WEB::CIMcpHttpAuthenticator::StaticBearerAuthenticatorName );
            WEB::CMcpStaticBearerAuthenticator* bearer = static_cast< WEB::CMcpStaticBearerAuthenticator* >( authenticator.GetPointerAlways() );
            bearer->SetToken( McpHttpTestToken );
            bearer->SetTokenLabel( "test" );
            transport->SetAuthenticator( authenticator );
        }
        server->AddTransport( transportPtr );
        return server->Start() && transport->IsActive();
    }

    UInt16 GetPort( void ) const { return transport->GetBoundPort(); }

    COM::CMcpServer* server;
    COM::CIMcpTransportPtr transportPtr;
    WEB::CMcpStreamableHttpTransport* transport;
    CMcpTestTool* echo;
    CMcpTestTool* count;
    CMcpTestTool* ask;
    CMcpHttpParamTool* params;
    CMcpHttpOutcomeRecorder recorder;
};

/*-------------------------------------------------------------------------*/

/** Header fields of an MCP POST, each line ending in CR LF */
static CORE::CString
McpHeaders( const CORE::CString& method, const CORE::CString& name = CORE::CString::Empty )
{
    CORE::CString headers = "MCP-Protocol-Version: 2026-07-28\r\nMcp-Method: " + method + "\r\n";
    if ( !name.IsNULLOrEmpty() )
        headers += "Mcp-Name: " + name + "\r\n";
    return headers;
}

/*-------------------------------------------------------------------------*/

/** A whole POST to the MCP endpoint: the body and the header fields given, plus Host, the type, the length, Accept and the token */
static CORE::CString
McpPost( UInt16 port, const CORE::CString& body, const CORE::CString& headers,
         bool withToken = true, const CORE::CString& accept = McpHttpAccept, const CORE::CString& path = "/mcp" )
{
    CORE::CString request = "POST " + path + " HTTP/1.1\r\nHost: 127.0.0.1:" + CORE::ToString( (UInt32) port ) + "\r\nContent-Type: application/json\r\n";
    if ( !accept.IsNULLOrEmpty() )
        request += "Accept: " + accept + "\r\n";
    if ( withToken )
        request += "Authorization: Bearer " + CORE::CString( McpHttpTestToken ) + "\r\n";
    request += headers;
    request += "Content-Length: " + CORE::ToString( body.ByteSize() > 0 ? body.ByteSize() - 1 : 0 ) + "\r\n\r\n" + body;
    return request;
}

/*-------------------------------------------------------------------------*/

/** What came back for one POST: the response, its JSON-RPC message (the body, or the last event of a stream) and the events */
struct SMcpHttpReply
{
    Int32 status;
    STestHttpResponse response;
    CORE::CDataNode message;
    bool hasMessage;
    CORE::CString::StringVector events;
    UInt32 comments;

    SMcpHttpReply( void ) : status( 0 ), response(), message(), hasMessage( false ), events(), comments( 0 ) {}

    bool IsSse( void ) const { return 0 == response.Header( "Content-Type" ).HasSubstr( "text/event-stream", 0, true ); }
};

/*-------------------------------------------------------------------------*/

static void
ParseReply( const STestHttpResponse& response, SMcpHttpReply& reply )
{
    reply.status = response.status;
    reply.response = response;
    reply.hasMessage = false;
    reply.events.clear();
    if ( reply.IsSse() )
    {
        TestHttpParseSseEvents( response.body, reply.events, &reply.comments );
        if ( !reply.events.empty() )
            reply.hasMessage = McpTestParse( reply.events.back(), reply.message );
    }
    else
    if ( !response.body.IsNULLOrEmpty() )
    {
        reply.hasMessage = McpTestParse( response.body, reply.message );
    }
}

/*-------------------------------------------------------------------------*/

/** One POST on a new connection, waiting for the whole answer */
static bool
McpExchangeOn( UInt16 port, const CORE::CString& request, SMcpHttpReply& reply, UInt32 timeoutInMs = 5000 )
{
    CTestHttpRawClient client;
    if ( !client.Connect( port ) )
        return false;
    client.Send( request, 0 );
    if ( !client.WaitForResponses( 1, timeoutInMs ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: no whole answer came, what did come: " << client.GetReceived() );
        return false;
    }
    TTestHttpResponseVector responses;
    client.GetResponses( responses );
    ParseReply( responses[ 0 ], reply );
    return true;
}

/*-------------------------------------------------------------------------*/

static Int32
ReplyErrorCode( const SMcpHttpReply& reply )
{
    return reply.hasMessage ? McpTestErrorCode( reply.message ) : 0;
}

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpRegistrationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: gucefWEB registers streamable-http in the COM transport factory and its authenticators; the defaults bind to 127.0.0.1 at /mcp" )
        try
        {
            ASSERT_TRUE( COM::CComGlobal::Instance()->GetMcpTransportFactory().IsConstructible( COM::CIMcpTransport::StreamableHttpTransportName ) );
            WEB::TMcpHttpAuthenticatorFactory& authenticators = WEB::CWebGlobal::Instance()->GetMcpHttpAuthenticatorFactory();
            ASSERT_FALSE( authenticators.Create( WEB::CIMcpHttpAuthenticator::StaticBearerAuthenticatorName ).IsNULL() );
            ASSERT_FALSE( authenticators.Create( WEB::CIMcpHttpAuthenticator::NoAuthenticatorName ).IsNULL() );

            WEB::CMcpHttpTransportSettings defaults;
            ASSERT_TRUE( "127.0.0.1" == defaults.bindAddress && "/mcp" == defaults.path && 0 == defaults.port );
            ASSERT_TRUE( "static-bearer" == defaults.authenticator && !defaults.allowNonLoopback && 32 == defaults.maxConnections );
            ASSERT_TRUE( defaults.allowedOrigins.empty() && defaults.allowedHosts.empty() );

            // a configuration round trip, through the server settings and the factory
            CORE::CDataNode cfg( "Transport" );
            WEB::CMcpHttpTransportSettings settings;
            settings.port = 1234;
            settings.allowedOrigins.push_back( "http://localhost:3000" );
            settings.authenticator = "none";
            ASSERT_TRUE( settings.SaveConfig( cfg ) );
            WEB::CMcpHttpTransportSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( cfg ) );
            ASSERT_TRUE( 1234 == loaded.port && 1 == loaded.allowedOrigins.size() && "none" == loaded.authenticator && loaded.allowedHosts.empty() );
            CORE::CDataNode badPath( "Transport" );
            badPath.SetAttribute( "path", "mcp" );
            ASSERT_FALSE( loaded.LoadConfig( badPath ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The transport refuses a non-loopback bind without allowNonLoopback, an unknown authenticator and a bearer authenticator without a token" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            {
                WEB::CMcpHttpTransportSettings settings;
                settings.bindAddress = "0.0.0.0";
                CMcpHttpTestRig rig;
                ASSERT_FALSE( rig.Start( &settings ) );
            }
            {
                COM::CMcpServer server;
                WEB::CMcpStreamableHttpTransport transport;
                WEB::CMcpHttpTransportSettings settings;
                settings.authenticator = "no-such-authenticator";
                transport.SetSettings( settings );
                ASSERT_FALSE( transport.Start( server ) );
                settings.authenticator = "static-bearer";      // neither a token nor a token file
                transport.SetSettings( settings );
                ASSERT_FALSE( transport.Start( server ) );
                ASSERT_FALSE( transport.IsActive() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A server configured with a streamable-http entry in Transports makes the transport through the factory and serves it" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CORE::CDataNode config;
            ASSERT_TRUE( McpTestParse( "{\"McpServer\":{\"serverName\":\"configured\",\"Transports\":[{\"type\":\"streamable-http\",\"port\":0,\"path\":\"/configured\",\"authenticator\":\"none\"}]}}", config ) );
            COM::CMcpServer server;
            ASSERT_TRUE( server.LoadConfig( config ) );
            ASSERT_TRUE( server.Start() );
            CORE::CDataNode saved;
            ASSERT_TRUE( server.SaveConfig( saved ) );
            const CORE::CString savedText = McpTestToJson( saved );
            ASSERT_TRUE( savedText.HasSubstr( "streamable-http" ) >= 0 && savedText.HasSubstr( "/configured" ) >= 0 );
            server.Stop( false );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The static bearer authenticator makes an owner-only token file, reads it back, and compares tokens" )
        try
        {
            CORE::CString tokenFile = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( tokenFile, "McpHttpTestBearerToken.txt" );
            CORE::DeleteFile( tokenFile );

            WEB::CMcpStaticBearerAuthenticator first;
            first.SetTokenFile( tokenFile );
            CORE::CString problem;
            ASSERT_TRUE( first.Prepare( problem ) );
            ASSERT_TRUE( CORE::FileExists( tokenFile ) );
            ASSERT_TRUE( first.GetToken().Length() >= 43 );

            WEB::CMcpStaticBearerAuthenticator second;
            second.SetTokenFile( tokenFile );
            ASSERT_TRUE( second.Prepare( problem ) );
            ASSERT_TRUE( first.GetToken() == second.GetToken() );

            WEB::CHttpRequestData request;
            COM::CMcpPrincipal principal( COM::CIMcpTransport::StreamableHttpTransportName, "anonymous", false );
            WEB::CHttpResponseData challenge;
            request.otherHeaders[ "authorization" ] = "Bearer " + first.GetToken();
            ASSERT_TRUE( second.Authenticate( request, principal, challenge ) && principal.isAuthenticated && "bearer:default" == principal.subject );
            request.otherHeaders[ "authorization" ] = "bearer   " + first.GetToken();
            ASSERT_TRUE( second.Authenticate( request, principal, challenge ) );
            request.otherHeaders[ "authorization" ] = "Bearer " + first.GetToken() + "x";
            ASSERT_FALSE( second.Authenticate( request, principal, challenge ) );
            ASSERT_TRUE( 401 == challenge.statusCode );
            request.otherHeaders[ "authorization" ] = "Basic " + first.GetToken();
            ASSERT_FALSE( second.Authenticate( request, principal, challenge ) );
            request.otherHeaders[ "authorization" ] = "Bearer " + first.GetToken() + ", Bearer " + first.GetToken();
            ASSERT_FALSE( second.Authenticate( request, principal, challenge ) );
            ASSERT_TRUE( WEB::CMcpStaticBearerAuthenticator::HasTokenInQuery( "a=1&access_token=x" ) );
            ASSERT_FALSE( WEB::CMcpStaticBearerAuthenticator::HasTokenInQuery( "a=1&b=2" ) );
            CORE::DeleteFile( tokenFile );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Host and Origin checks, and the decoding of header values" )
        try
        {
            CORE::CString::StringVector none;
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "127.0.0.1:4000", none, 4000 ) );
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "LOCALHOST:4000", none, 4000 ) );
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "[::1]:4000", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "127.0.0.1:4001", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "127.0.0.1", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "evil.example:4000", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "localhost.evil.example:4000", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "::1:4000", none, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "127.0.0.1:40x0", none, 4000 ) );
            CORE::CString::StringVector hosts;
            hosts.push_back( "mcp.example" );
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "mcp.example:4000", hosts, 4000 ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsHostAllowed( "localhost:4000", hosts, 4000 ) );

            CORE::CString::StringVector origins;
            origins.push_back( "http://localhost:3000" );
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "http://localhost:3000", origins ) );
            ASSERT_TRUE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "HTTP://LOCALHOST:3000", origins ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "http://localhost:3001", origins ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "https://localhost:3000", origins ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "null", origins ) );
            ASSERT_FALSE( WEB::CMcpStreamableHttpTransport::IsOriginAllowed( "http://evil.example", none ) );

            CORE::CString decoded;
            ASSERT_TRUE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "us-west1", decoded ) && "us-west1" == decoded );
            ASSERT_TRUE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "=?base64?IHBhZGRlZCA=?=", decoded ) && " padded " == decoded );
            ASSERT_TRUE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "=?base64?bGluZTEKbGluZTI=?=", decoded ) && "line1\nline2" == decoded );
            ASSERT_TRUE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "=?base64?PT9iYXNlNjQ/bGl0ZXJhbD89?=", decoded ) && "=?base64?literal?=" == decoded );
            ASSERT_TRUE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "=?BASE64?eA==?=", decoded ) && "=?BASE64?eA==?=" == decoded );   // the markers are case-sensitive
            ASSERT_FALSE( WEB::CMcpHttpExchangeChannel::DecodeHeaderValue( "bad\x01value", decoded ) );

            ASSERT_TRUE( 400 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32020, true, false ) );
            ASSERT_TRUE( 400 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32602, false, false ) );
            ASSERT_TRUE( 200 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32602, true, false ) );
            ASSERT_TRUE( 404 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32601, true, false ) );
            ASSERT_TRUE( 200 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32601, true, true ) );
            ASSERT_TRUE( 429 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -31001, true, false ) );
            ASSERT_TRUE( 503 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -31002, true, false ) );
            ASSERT_TRUE( 403 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -31003, true, false ) );
            ASSERT_TRUE( 200 == WEB::CMcpHttpExchangeChannel::StatusForErrorCode( -32603, true, false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpConversationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A modern conversation over HTTP: server/discover, tools/list and tools/call, answered with JSON on one keep-alive connection" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();

            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( port ) );
            ASSERT_TRUE( client.Send( McpPost( port, McpTestRequest( "1", "server/discover" ), McpHeaders( "server/discover" ) ) +
                                      McpPost( port, McpTestRequest( "2", "tools/list" ), McpHeaders( "tools/list" ) ) +
                                      McpPost( port, McpTestRequest( "\"three\"", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"hi\"}" ), McpHeaders( "tools/call", "echo" ) ), 0 ) );
            ASSERT_TRUE( client.WaitForResponses( 3 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 3 == client.GetResponses( responses ) );
            if ( 3 == responses.size() )
            {
                SMcpHttpReply discover, list, call;
                ParseReply( responses[ 0 ], discover );
                ParseReply( responses[ 1 ], list );
                ParseReply( responses[ 2 ], call );
                ASSERT_TRUE( 200 == discover.status && discover.hasMessage && !discover.IsSse() );
                ASSERT_TRUE( McpHttpSame( discover.response.Header( "Content-Type" ), "application/json" ) );
                ASSERT_FALSE( discover.response.HasHeader( "Mcp-Session-Id" ) );
                ASSERT_TRUE( McpHttpSame( McpTestResultAttr( discover.message, "resultType" ), "complete" ) );
                ASSERT_TRUE( 200 == list.status && 4 == McpTestChildCount( McpTestResultChild( list.message, "tools" ) ) );
                ASSERT_TRUE( 200 == call.status && McpHttpSame( McpTestFirstText( call.message.FindChild( "result" ) ), "echo: hi" ) );
                ASSERT_TRUE( McpHttpSame( McpTestAttr( call.message, "id" ), "three" ) );
            }
            ASSERT_FALSE( client.IsClosed() );
            TestHttpPumpFor( 50 );
            ASSERT_TRUE( 0 == rig.transport->GetOpenExchangeCount() );
            ASSERT_TRUE( 0 == rig.server->GetChannelCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Progress switches the reply to a request-scoped SSE stream that ends with the response; a JSON-only client gets JSON" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            COM::CMcpServerSettings serverSettings;
            serverSettings.progressMinIntervalMs = 0;
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start( GUCEF_NULL, false, &serverSettings ) );
            rig.count->steps = 5;
            rig.count->stepMs = 20;
            const UInt16 port = rig.GetPort();

            SMcpHttpReply reply;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "7", "tools/call", "\"name\":\"count\",\"arguments\":{\"steps\":5}", "{}", "\"progressToken\":\"p1\"" ),
                                               McpHeaders( "tools/call", "count" ) ), reply ) );
            ASSERT_TRUE( 200 == reply.status && reply.IsSse() );
            ASSERT_TRUE( McpHttpSame( reply.response.Header( "Cache-Control" ), "no-cache" ) );
            ASSERT_TRUE( McpHttpSame( reply.response.Header( "X-Accel-Buffering" ), "no" ) );
            ASSERT_FALSE( reply.response.HasHeader( "Content-Length" ) );
            ASSERT_TRUE( reply.events.size() >= 2 );
            UInt32 progressEvents = 0;
            for ( size_t i=0; i+1<reply.events.size(); ++i )
            {
                CORE::CDataNode event;
                if ( McpTestParse( reply.events[ i ], event ) && "notifications/progress" == McpTestAttr( event, "method" ) )
                    ++progressEvents;
            }
            ASSERT_TRUE( progressEvents >= 1 && progressEvents == reply.events.size() - 1 );
            ASSERT_TRUE( reply.hasMessage && McpHttpSame( McpTestFirstText( reply.message.FindChild( "result" ) ), "counted" ) );

            SMcpHttpReply jsonOnly;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "8", "tools/call", "\"name\":\"count\",\"arguments\":{\"steps\":3}", "{}", "\"progressToken\":\"p2\"" ),
                                               McpHeaders( "tools/call", "count" ), true, "application/json" ), jsonOnly ) );
            ASSERT_TRUE( 200 == jsonOnly.status && !jsonOnly.IsSse() && jsonOnly.hasMessage );
            ASSERT_TRUE( McpHttpSame( McpTestFirstText( jsonOnly.message.FindChild( "result" ) ), "counted" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Closing the stream of a modern request cancels it; nothing is left open" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            COM::CMcpServerSettings serverSettings;
            serverSettings.progressMinIntervalMs = 0;
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start( GUCEF_NULL, false, &serverSettings ) );
            rig.count->stepMs = 20;
            const UInt16 port = rig.GetPort();
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( port ) );
                ASSERT_TRUE( client.Send( McpPost( port, McpTestRequest( "9", "tools/call", "\"name\":\"count\",\"arguments\":{\"steps\":500}", "{}", "\"progressToken\":\"p3\"" ),
                                                   McpHeaders( "tools/call", "count" ) ), 0 ) );
                ASSERT_TRUE( client.WaitForText( "notifications/progress" ) );
                ASSERT_TRUE( 1 == rig.transport->GetOpenExchangeCount() );
                client.Close();
            }
            UInt32 waited = 0;
            while ( 0 == rig.count->sawCancellation.Load() && waited++ < 3000 )
                TestHttpPumpFor( 1 );
            ASSERT_TRUE( 1 == rig.count->sawCancellation.Load() );
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == rig.transport->GetOpenExchangeCount() );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
            ASSERT_TRUE( 0 == rig.server->GetChannelCount() );
            ASSERT_TRUE( rig.recorder.HasOutcome( "tools/call", COM::MCP_OUTCOME_CANCELLED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: subscriptions/listen is a stream with keep-alive comments and the notifications it asked for; closing it ends the subscription" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            WEB::CMcpHttpTransportSettings settings;
            settings.sseKeepAliveMs = 50;
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start( &settings ) );
            const UInt16 port = rig.GetPort();
            const CORE::CString listen = McpTestRequest( "\"sub\"", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true}" );

            // a client that takes no stream cannot listen
            SMcpHttpReply refused;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, listen, McpHeaders( "subscriptions/listen" ), true, "application/json" ), refused ) );
            ASSERT_TRUE( McpHttpSameInt( refused.status, 406, "status" ) );

            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( port ) );
            ASSERT_TRUE( client.Send( McpPost( port, listen, McpHeaders( "subscriptions/listen" ) ), 0 ) );
            ASSERT_TRUE( client.WaitForText( "notifications/subscriptions/acknowledged" ) );
            ASSERT_TRUE( 1 == rig.server->GetSubscriptionCount() );
            TestHttpPumpFor( 200 );

            CORE::CString error;
            ASSERT_TRUE( rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "late_tool", CMcpTestTool::ECHO ) ), error ) );
            ASSERT_TRUE( client.WaitForText( "notifications/tools/list_changed" ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 200 == responses[ 0 ].status && !responses[ 0 ].complete );
            if ( 1 == responses.size() )
            {
                SMcpHttpReply reply;
                ParseReply( responses[ 0 ], reply );
                ASSERT_TRUE( reply.IsSse() && reply.events.size() >= 2 && reply.comments >= 1 );
                ASSERT_TRUE( reply.events.size() >= 2 && reply.events[ 1 ].HasSubstr( "io.modelcontextprotocol/subscriptionId" ) > 0 );
            }
            client.Close();
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == rig.server->GetSubscriptionCount() );
            ASSERT_TRUE( 0 == rig.transport->GetOpenExchangeCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MRTR over HTTP: input_required with elicitation, then the retry with the answer" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply first;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "1", "tools/call", "\"name\":\"ask_name\",\"arguments\":{}", "{\"elicitation\":{}}" ),
                                               McpHeaders( "tools/call", "ask_name" ) ), first ) );
            ASSERT_TRUE( 200 == first.status && McpHttpSame( McpTestResultAttr( first.message, "resultType" ), "input_required" ) );
            const CORE::CString state = McpTestResultAttr( first.message, "requestState" );
            SMcpHttpReply second;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "2", "tools/call", "\"name\":\"ask_name\",\"arguments\":{},\"requestState\":\"" + state +
                                                                     "\",\"inputResponses\":{\"name_request\":{\"action\":\"accept\",\"content\":{\"name\":\"Ada\"}}}", "{\"elicitation\":{}}" ),
                                               McpHeaders( "tools/call", "ask_name" ) ), second ) );
            ASSERT_TRUE( 200 == second.status && McpHttpSame( McpTestFirstText( second.message.FindChild( "result" ) ), "Hello Ada" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The GUCEF HTTP client talks to the MCP endpoint" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();

            WEB::CHTTPClient httpClient;
            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "http://127.0.0.1:" + CORE::ToString( (UInt32) port ) + "/mcp" ) );
            request.requestType = "POST";
            request.contentRepresentation = "application/json";
            request.resourceRepresentations.push_back( "application/json" );
            request.resourceRepresentations.push_back( "text/event-stream" );
            request.otherHeaders[ "Authorization" ] = "Bearer " + CORE::CString( McpHttpTestToken );
            request.otherHeaders[ "MCP-Protocol-Version" ] = "2026-07-28";
            request.otherHeaders[ "Mcp-Method" ] = "tools/call";
            request.otherHeaders[ "Mcp-Name" ] = "echo";
            const CORE::CString body = McpTestRequest( "42", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"from the client\"}" );
            request.content.CopyFrom( body.Length(), body.C_String() );

            CORE::CFutureResult future = httpClient.ExecuteAsync( request );
            ASSERT_TRUE( future.HasAFuture() );
            UInt32 waited = 0;
            while ( !future.IsReady() && waited++ < 5000 )
                TestHttpPumpFor( 1 );
            ASSERT_TRUE( future.IsReady() );
            if ( future.IsReady() )
            {
                WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( future.GetResult() );
                ASSERT_TRUE( GUCEF_NULL != data );
                if ( GUCEF_NULL != data )
                {
                    ASSERT_TRUE( McpHttpSameInt( data->response.statusCode, 200, "status" ) );
                    CORE::CDataNode message;
                    const CORE::CString text( static_cast< const char* >( data->response.content.GetConstBufferPtr() ), data->response.content.GetDataSize() );
                    ASSERT_TRUE( McpTestParse( text, message ) );
                    ASSERT_TRUE( McpHttpSame( McpTestFirstText( message.FindChild( "result" ) ), "echo: from the client" ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpRefusalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Before the body: GET, DELETE and OPTIONS 405; Host and Origin 403; authentication 401; type 415; length 411 and 413; Accept 406" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            WEB::CMcpHttpTransportSettings settings;
            settings.allowedOrigins.push_back( "http://localhost:3000" );
            settings.maxBodyBytes = 2048;
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start( &settings ) );
            const UInt16 port = rig.GetPort();
            const CORE::CString host = "Host: 127.0.0.1:" + CORE::ToString( (UInt32) port ) + "\r\n";
            const CORE::CString auth = "Authorization: Bearer " + CORE::CString( McpHttpTestToken ) + "\r\n";
            const CORE::CString body = McpTestRequest( "1", "tools/list" );
            SMcpHttpReply reply;

            const char* methods[] = { "GET", "DELETE", "OPTIONS", "PUT" };
            for ( int i=0; i<4; ++i )
            {
                ASSERT_TRUE( McpExchangeOn( port, CORE::CString( methods[ i ] ) + " /mcp HTTP/1.1\r\n" + host + auth + "\r\n", reply ) );
                ASSERT_TRUE( McpHttpSameInt( reply.status, 405, "status" ) && McpHttpSame( reply.response.Header( "Allow" ), "POST" ) );
            }

            // DNS rebinding: a page under another name, or another port
            ASSERT_TRUE( McpExchangeOn( port, "POST /mcp HTTP/1.1\r\nHost: evil.example:" + CORE::ToString( (UInt32) port ) + "\r\nContent-Type: application/json\r\n" + auth + "Content-Length: " + CORE::ToString( body.Length() ) + "\r\n\r\n" + body, reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 403, "status" ) && ReplyErrorCode( reply ) == -31003 );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Origin: http://evil.example\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 403, "status" ) && reply.hasMessage && "<none>" != McpTestAttr( reply.message, "jsonrpc" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Origin: null\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 403, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Origin: http://localhost:3000\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) );

            // authentication
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ), false ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 401, "status" ) && McpHttpSame( reply.response.Header( "WWW-Authenticate" ), "Bearer" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Authorization: Bearer wrong-token\r\n", false ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 401, "status" ) && reply.response.Header( "WWW-Authenticate" ).HasSubstr( "invalid_token" ) > 0 );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ), false, McpHttpAccept, "/mcp?access_token=" + CORE::CString( McpHttpTestToken ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) );

            // what is sent, and what can be answered with
            ASSERT_TRUE( McpExchangeOn( port, "POST /mcp HTTP/1.1\r\n" + host + auth + McpHeaders( "tools/list" ) + "Content-Type: text/plain\r\nContent-Length: " + CORE::ToString( body.Length() ) + "\r\n\r\n" + body, reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 415, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, "POST /mcp HTTP/1.1\r\n" + host + auth + McpHeaders( "tools/list" ) + "Content-Type: application/json\r\n\r\n", reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 411, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, "POST /mcp HTTP/1.1\r\n" + host + auth + McpHeaders( "tools/list" ) + "Content-Type: application/json; charset=utf-8\r\nContent-Length: 4096\r\n\r\n", reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 413, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ), true, "text/html" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 406, "status" ) );

            // a web socket upgrade on the endpoint is no POST
            ASSERT_TRUE( McpExchangeOn( port, "GET /mcp HTTP/1.1\r\n" + host + "Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n", reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 405, "status" ) );

            // none of these reached the server
            ASSERT_TRUE( rig.recorder.outcomes.size() == 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The header checks of modern requests: MCP-Protocol-Version, Mcp-Method, Mcp-Name with the base64 sentinel, Mcp-Param numeric compare" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;
            const CORE::CString echoCall = McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" );

            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, "Mcp-Method: tools/call\r\nMcp-Name: echo\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, "MCP-Protocol-Version: 2025-06-18\r\nMcp-Method: tools/call\r\nMcp-Name: echo\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/list", "echo" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/call" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/call", "other" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpTestErrorMessage( reply.message ).HasSubstr( "Mcp-Name" ) >= 0 );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/call", "=?base64?ZWNobw==?=" ) ), reply ) );     // "echo"
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 0 == ReplyErrorCode( reply ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/call", "echo" ) + "Mcp-Method: tools/call\r\n" ), reply ) );     // a second Mcp-Method
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );

            const CORE::CString query = McpTestRequest( "2", "tools/call", "\"name\":\"query\",\"arguments\":{\"region\":\"us-west1\",\"limit\":42,\"dry\":true,\"sql\":\"select 1\"}" );
            const CORE::CString params = "Mcp-Param-Region: us-west1\r\nMcp-Param-Limit: 42.0\r\nMcp-Param-Dry: true\r\n";
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + params ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 0 == ReplyErrorCode( reply ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "mcp-param-region: us-west1\r\nMCP-PARAM-LIMIT: 42\r\nMcp-Param-Dry: true\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us-west1\r\nMcp-Param-Dry: true\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: eu-west1\r\nMcp-Param-Limit: 42\r\nMcp-Param-Dry: true\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us-west1\r\nMcp-Param-Limit: 43\r\nMcp-Param-Dry: true\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us-west1\r\nMcp-Param-Limit: 42\r\nMcp-Param-Dry: TRUE\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );

            // an argument that is not there needs no header, and a header without the argument is refused
            const CORE::CString partial = McpTestRequest( "3", "tools/call", "\"name\":\"query\",\"arguments\":{\"region\":\"us-west1\",\"limit\":null}" );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, partial, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us-west1\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 0 == ReplyErrorCode( reply ) );
            ASSERT_TRUE( McpHttpSame( McpTestResultAttr( reply.message, "isError" ), "true" ) );    // past the headers: null is no integer for the schema
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, partial, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us-west1\r\nMcp-Param-Dry: false\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpHttpSameInt( (Int32) rig.params->executions, 2, "executions of the tool" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Errors map to statuses: 404 for an unknown method, 400 for a parse error, a batch, a malformed _meta, an unsupported version; 200 for an unknown tool" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;

            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "1", "nothing/here" ), McpHeaders( "nothing/here" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 404, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32601, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, "{ this is not json", McpHeaders( "tools/list" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32700, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, "[" + McpTestRequest( "1", "tools/list" ) + "]", McpHeaders( "tools/list" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32600, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":{\"_meta\":7}}", McpHeaders( "tools/list" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32602, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2099-01-01\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}",
                                               "MCP-Protocol-Version: 2099-01-01\r\nMcp-Method: tools/list\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32022, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "1", "tools/call", "\"name\":\"nope\",\"arguments\":{}" ), McpHeaders( "tools/call", "nope" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32602, "code" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "1", "ping" ), McpHeaders( "ping" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 404, "status" ) );

            // a notification is accepted without a body, also notifications/cancelled, which HTTP does not need
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestNotification( "notifications/cancelled", "\"requestId\":5" ), CORE::CString::Empty ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 202, "status" ) && reply.response.body.IsNULLOrEmpty() );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestNotification( "notifications/whatever" ), CORE::CString::Empty ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 202, "status" ) );
            TestHttpPumpFor( 50 );
            ASSERT_TRUE( 0 == rig.server->GetChannelCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpLegacyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A 2025-11-25 client over HTTP: initialize without a session id, then requests by the header; Mcp-Session-Id is ignored" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;

            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestLegacyRequest( "1", "initialize", "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{},\"clientInfo\":{\"name\":\"old\",\"version\":\"1\"}" ), CORE::CString::Empty ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && McpHttpSame( McpTestResultAttr( reply.message, "protocolVersion" ), "2025-11-25" ) );
            ASSERT_FALSE( reply.response.HasHeader( "Mcp-Session-Id" ) );
            const CORE::CDataNode* capabilities = McpTestResultChild( reply.message, "capabilities" );
            ASSERT_TRUE( GUCEF_NULL != capabilities && GUCEF_NULL != capabilities->FindChild( "tools" ) );
            if ( GUCEF_NULL != capabilities && GUCEF_NULL != capabilities->FindChild( "tools" ) )
                ASSERT_TRUE( "false" == McpTestAttr( *capabilities->FindChild( "tools" ), "listChanged" ) );

            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestNotification( "notifications/initialized" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 202, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestLegacyRequest( "2", "tools/list" ), "MCP-Protocol-Version: 2025-11-25\r\nMcp-Session-Id: made-up\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 4 == McpTestChildCount( McpTestResultChild( reply.message, "tools" ) ) );
            ASSERT_TRUE( McpHttpSame( McpTestResultAttr( reply.message, "resultType" ), "<none>" ) );
            ASSERT_FALSE( reply.response.HasHeader( "Mcp-Session-Id" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestLegacyRequest( "3", "ping" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 0 == ReplyErrorCode( reply ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestLegacyRequest( "4", "nothing/here" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32601, "code" ) );

            // an older version in the header without modern _meta: a modern request without _meta
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestLegacyRequest( "5", "tools/list" ), "MCP-Protocol-Version: 2025-06-18\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A legacy request whose client went away runs to its end and is recorded as client_gone" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.count->stepMs = 10;
            const UInt16 port = rig.GetPort();
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( port ) );
                ASSERT_TRUE( client.Send( McpPost( port, McpTestLegacyRequest( "6", "tools/call", "\"name\":\"count\",\"arguments\":{\"steps\":20}" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), 30 ) );
                client.Close();
            }
            UInt32 waited = 0;
            while ( !rig.recorder.HasOutcome( "tools/call", COM::MCP_OUTCOME_CLIENT_GONE ) && waited++ < 5000 )
                TestHttpPumpFor( 1 );
            ASSERT_TRUE( rig.recorder.HasOutcome( "tools/call", COM::MCP_OUTCOME_CLIENT_GONE ) );
            ASSERT_TRUE( 0 == rig.count->sawCancellation.Load() );
            TestHttpPumpFor( 50 );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() && 0 == rig.server->GetChannelCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A legacy request cancelled with notifications/cancelled gets no JSON-RPC response, and its POST is completed with 204" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            rig.count->stepMs = 10;
            const UInt16 port = rig.GetPort();
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( port ) );
            ASSERT_TRUE( client.Send( McpPost( port, McpTestLegacyRequest( "7", "tools/call", "\"name\":\"count\",\"arguments\":{\"steps\":500}" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), 50 ) );

            SMcpHttpReply reply;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestNotification( "notifications/cancelled", "\"requestId\":7" ), "MCP-Protocol-Version: 2025-11-25\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 202, "status" ) );

            ASSERT_TRUE( client.WaitForResponses( 1, 3000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) );
            if ( 1 == responses.size() )
            {
                ASSERT_TRUE( McpHttpSameInt( responses[ 0 ].status, 204, "status of the cancelled POST" ) );
                ASSERT_TRUE( responses[ 0 ].body.IsNULLOrEmpty() );
            }
            ASSERT_TRUE( rig.recorder.HasOutcome( "tools/call", COM::MCP_OUTCOME_CANCELLED ) );
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == rig.server->GetInFlightCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpLoadTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Ten clients at once with modern requests, and the server stopping with streams open" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            GUCEF::vector< CTestHttpRawClient* > clients;
            for ( UInt32 i=0; i<10; ++i )
            {
                clients.push_back( GUCEF_NEW CTestHttpRawClient() );
                ASSERT_TRUE( clients.back()->Connect( port ) );
                clients.back()->Send( McpPost( port, McpTestRequest( CORE::ToString( i ), "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"c" + CORE::ToString( i ) + "\"}" ),
                                               McpHeaders( "tools/call", "echo" ) ), 0 );
            }
            UInt32 answered = 0;
            for ( UInt32 i=0; i<clients.size(); ++i )
            {
                if ( !clients[ i ]->WaitForResponses( 1 ) )
                    continue;
                TTestHttpResponseVector responses;
                clients[ i ]->GetResponses( responses );
                SMcpHttpReply reply;
                ParseReply( responses[ 0 ], reply );
                if ( 200 == reply.status && McpTestFirstText( reply.message.FindChild( "result" ) ) == "echo: c" + CORE::ToString( i ) )
                    ++answered;
            }
            ASSERT_TRUE( McpHttpSameInt( (Int32) answered, 10, "answered clients" ) );
            for ( UInt32 i=0; i<clients.size(); ++i )
                GUCEF_DELETE clients[ i ];
            clients.clear();

            // a stream that is open when the server stops ends with the graceful closure of the subscription
            CTestHttpRawClient listener;
            ASSERT_TRUE( listener.Connect( port ) );
            ASSERT_TRUE( listener.Send( McpPost( port, McpTestRequest( "\"sub\"", "subscriptions/listen", "\"notifications\":{\"toolsListChanged\":true}" ), McpHeaders( "subscriptions/listen" ) ), 0 ) );
            ASSERT_TRUE( listener.WaitForText( "notifications/subscriptions/acknowledged" ) );
            rig.server->Stop( true );
            ASSERT_TRUE( listener.WaitForClose( 3000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == listener.GetResponses( responses ) && responses[ 0 ].complete );
            if ( 1 == responses.size() )
            {
                SMcpHttpReply reply;
                ParseReply( responses[ 0 ], reply );
                ASSERT_TRUE( reply.hasMessage && McpHttpSame( McpTestResultAttr( reply.message, "resultType" ), "complete" ) );
            }
            ASSERT_FALSE( rig.transport->IsActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes are UTF-8 (RFC 3629): no overlong forms, no surrogates, nothing above U+10FFFF */
static bool
McpHttpIsUtf8( const CORE::CString& text )
{
    const unsigned char* bytes = reinterpret_cast< const unsigned char* >( text.C_String() );
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 i = 0;
    while ( i < size )
    {
        const unsigned char c = bytes[ i ];
        UInt32 length = 0;
        UInt32 codePoint = 0;
        if ( c < 0x80 ) { ++i; continue; }
        else if ( c >= 0xC2 && c <= 0xDF ) { length = 2; codePoint = c & 0x1F; }
        else if ( c >= 0xE0 && c <= 0xEF ) { length = 3; codePoint = c & 0x0F; }
        else if ( c >= 0xF0 && c <= 0xF4 ) { length = 4; codePoint = c & 0x07; }
        else return false;
        if ( i + length > size )
            return false;
        for ( UInt32 n=1; n<length; ++n )
        {
            if ( 0x80 != ( bytes[ i+n ] & 0xC0 ) )
                return false;
            codePoint = ( codePoint << 6 ) | ( bytes[ i+n ] & 0x3F );
        }
        if ( ( 3 == length && codePoint < 0x800 ) || ( 4 == length && ( codePoint < 0x10000 || codePoint > 0x10FFFF ) ) || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += length;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Keeps the audit records of a server */
class CMcpHttpAuditRecorder : public CORE::CObserver
{
    public:

    CMcpHttpAuditRecorder( void ) : CORE::CObserver(), records() {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == COM::CMcpServer::AuditEvent )
            records.push_back( static_cast< COM::TMcpAuditEventData* >( eventdata )->GetData() );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CMcpHttpAuditRecorder";
        return classTypeName;
    }

    GUCEF::vector< COM::CMcpAuditRecord > records;
};

/*-------------------------------------------------------------------------*/

static void
PerformMcpHttpHardeningTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Requests refused before dispatch (401, 403 for Host and Origin) are audited once each, as not authenticated, and a flood of them is summed up" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            CMcpHttpAuditRecorder audit;
            audit.SubscribeTo( rig.server, COM::CMcpServer::AuditEvent );
            const CORE::CString body = McpTestRequest( "1", "tools/list" );
            const CORE::CString auth = "Authorization: Bearer " + CORE::CString( McpHttpTestToken ) + "\r\n";
            SMcpHttpReply reply;

            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ), false ), reply ) && 401 == reply.status );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Authorization: Bearer guessed-token\r\n", false ), reply ) && 401 == reply.status );
            ASSERT_TRUE( McpExchangeOn( port, "POST /mcp HTTP/1.1\r\nHost: rebound.example:" + CORE::ToString( (UInt32) port ) + "\r\nContent-Type: application/json\r\n" + auth + McpHeaders( "tools/list" ) + "Content-Length: " + CORE::ToString( body.Length() ) + "\r\n\r\n" + body, reply ) && 403 == reply.status );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ) + "Origin: http://evil.example\r\n" ), reply ) && 403 == reply.status );
            TestHttpPumpFor( 50 );

            ASSERT_TRUE( McpHttpSameInt( (Int32) audit.records.size(), 4, "audit records" ) );
            for ( size_t i=0; i<audit.records.size(); ++i )
            {
                const COM::CMcpAuditRecord& record = audit.records[ i ];
                CORE::CString line;
                record.ToJsonLine( line );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: refusal audit: " << line );
                ASSERT_FALSE( record.isAuthenticated );
                ASSERT_TRUE( COM::MCP_OUTCOME_REFUSED == record.outcome );
                ASSERT_TRUE( record.transport == COM::CIMcpTransport::StreamableHttpTransportName );
                ASSERT_TRUE( line.HasSubstr( "\"authenticated\":false" ) >= 0 && line.HasSubstr( "\"reason\":" ) >= 0 );
                ASSERT_TRUE( line.HasSubstr( "guessed-token" ) < 0 && line.HasSubstr( McpHttpTestToken ) < 0 );
            }
            if ( 4 == audit.records.size() )
            {
                ASSERT_TRUE( audit.records[ 2 ].refusalReason.HasSubstr( "Host" ) >= 0 );
                ASSERT_TRUE( audit.records[ 3 ].refusalReason.HasSubstr( "Origin" ) >= 0 );
            }

            // a flood: the first ones one by one, the rest as a count
            audit.records.clear();
            for ( UInt32 i=0; i<60; ++i )
                McpExchangeOn( port, McpPost( port, body, McpHeaders( "tools/list" ), false ), reply );
            const size_t duringFlood = audit.records.size();
            TestHttpPumpFor( 2500 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: 60 refusals gave " << duringFlood << " audit records, " << audit.records.size() << " after the window" );
            // the summary comes with the next refusal after the second, or from the pulse once the second is over
            ASSERT_TRUE( duringFlood < 60 );
            ASSERT_TRUE( audit.records.size() < 60 );
            bool summed = false;
            for ( size_t i=0; i<audit.records.size(); ++i )
                summed = summed || audit.records[ i ].refusalReason.HasSubstr( "not recorded one by one" ) >= 0;
            ASSERT_TRUE( summed );
            audit.UnsubscribeAllFromObserver();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Header mismatch messages are UTF-8: a decoded Mcp-Name that is no UTF-8, and a value cut at 128 bytes" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;
            const CORE::CString echoCall = McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" );

            // base64 of the one byte 0xFF
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, echoCall, McpHeaders( "tools/call", "=?base64?/w==?=" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpHttpIsUtf8( reply.response.body ) );

            // a reply with text beyond ASCII is whole: its length is counted in bytes
            const CORE::CString utf8Call = McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80 \xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80x\"}" );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, utf8Call, McpHeaders( "tools/call", "echo" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && reply.hasMessage );
            ASSERT_TRUE( reply.response.body.HasSubstr( "echo: caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80 \xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80x" ) >= 0 && McpHttpIsUtf8( reply.response.body ) );
            ASSERT_TRUE( reply.response.body.ByteSize() - 1 == CORE::StringToUInt32( reply.response.Header( "Content-Length" ) ) );

            // 127 ASCII characters and a two byte character: a cut at 128 bytes would split it
            CORE::CString longName;
            for ( UInt32 i=0; i<127; ++i )
                longName += "a";
            longName += "\xc3\xa9";
            const CORE::CString longCall = McpTestRequest( "2", "tools/call", "\"name\":\"" + longName + "\",\"arguments\":{}" );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, longCall, McpHeaders( "tools/call", "other" ) ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            ASSERT_TRUE( McpHttpIsUtf8( reply.response.body ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A path other than the MCP endpoint is not found (404), whatever the method" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, "{}", CORE::CString::Empty, true, McpHttpAccept, "/nope" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 404, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, McpTestRequest( "1", "tools/list" ), McpHeaders( "tools/list" ), true, McpHttpAccept, "/mcp/extra" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 404, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, "GET /nope HTTP/1.1\r\nHost: 127.0.0.1:" + CORE::ToString( (UInt32) port ) + "\r\n\r\n", reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 404, "status" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An integer Mcp-Param header is a decimal number: hexadecimal, exponents, infinity and signs are refused" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !McpTestJsonAvailable(), "the json codec plugin is not available" );
            CMcpHttpTestRig rig;
            ASSERT_TRUE( rig.Start() );
            const UInt16 port = rig.GetPort();
            SMcpHttpReply reply;
            const CORE::CString query = McpTestRequest( "1", "tools/call", "\"name\":\"query\",\"arguments\":{\"region\":\"us\",\"limit\":42}" );

            const char* refused[] = { "0x2A", "4.2e1", "42e0", "inf", "nan", "+42", "42.", ".42", "42.5", "4 2", "-42", "18446744073709551658" };
            for ( UInt32 i=0; i<sizeof( refused ) / sizeof( refused[ 0 ] ); ++i )
            {
                ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us\r\nMcp-Param-Limit: " + refused[ i ] + "\r\n" ), reply ) );
                if ( 400 != reply.status )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamableHttp test: Mcp-Param-Limit \"" << refused[ i ] << "\" got " << reply.status );
                ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) && McpHttpSameInt( ReplyErrorCode( reply ), -32020, "code" ) );
            }
            const char* accepted[] = { "42", "42.0", "42.000" };
            for ( UInt32 i=0; i<sizeof( accepted ) / sizeof( accepted[ 0 ] ); ++i )
            {
                ASSERT_TRUE( McpExchangeOn( port, McpPost( port, query, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us\r\nMcp-Param-Limit: " + accepted[ i ] + "\r\n" ), reply ) );
                ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) && 0 == ReplyErrorCode( reply ) );
            }

            // exact beyond 2^53, and negative numbers
            const CORE::CString big = McpTestRequest( "2", "tools/call", "\"name\":\"query\",\"arguments\":{\"region\":\"us\",\"limit\":9007199254740993}" );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, big, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us\r\nMcp-Param-Limit: 9007199254740992\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 400, "status" ) );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, big, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us\r\nMcp-Param-Limit: 9007199254740993\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) );
            const CORE::CString negative = McpTestRequest( "3", "tools/call", "\"name\":\"query\",\"arguments\":{\"region\":\"us\",\"limit\":-7}" );
            ASSERT_TRUE( McpExchangeOn( port, McpPost( port, negative, McpHeaders( "tools/call", "query" ) + "Mcp-Param-Region: us\r\nMcp-Param-Limit: -7\r\n" ), reply ) );
            ASSERT_TRUE( McpHttpSameInt( reply.status, 200, "status" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_TESTCASE( "Test: The bearer token file is refused when it is a link or others can read it, and a new one is 0600 from the start" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefWEB_TestApp_McpBearerTokenSafety" );
            CORE::Remove_Directory( dir.C_String(), 1 );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            const CORE::CString planted = CORE::CombinePath( dir, "planted.token" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( planted, "a-token-someone-else-knows" ) );
            ::chmod( planted.C_String(), 0600 );
            CORE::CString problem;

            const CORE::CString linked = CORE::CombinePath( dir, "linked.token" );
            ASSERT_TRUE( 0 == ::symlink( planted.C_String(), linked.C_String() ) );
            {
                WEB::CMcpStaticBearerAuthenticator authenticator;
                authenticator.SetTokenFile( linked );
                ASSERT_FALSE( authenticator.Prepare( problem ) );
            }
            const CORE::CString dangling = CORE::CombinePath( dir, "dangling.token" );
            const CORE::CString nowhere = CORE::CombinePath( dir, "nowhere.token" );
            ASSERT_TRUE( 0 == ::symlink( nowhere.C_String(), dangling.C_String() ) );
            {
                WEB::CMcpStaticBearerAuthenticator authenticator;
                authenticator.SetTokenFile( dangling );
                ASSERT_FALSE( authenticator.Prepare( problem ) );
            }
            ASSERT_FALSE( CORE::FileExists( nowhere ) );

            ::chmod( planted.C_String(), 0644 );
            {
                WEB::CMcpStaticBearerAuthenticator authenticator;
                authenticator.SetTokenFile( planted );
                ASSERT_FALSE( authenticator.Prepare( problem ) );
            }

            const CORE::CString fresh = CORE::CombinePath( dir, "fresh.token" );
            const mode_t oldMask = ::umask( 0 );
            bool prepared = false;
            {
                WEB::CMcpStaticBearerAuthenticator authenticator;
                authenticator.SetTokenFile( fresh );
                prepared = authenticator.Prepare( problem );
            }
            ::umask( oldMask );
            ASSERT_TRUE( prepared );
            struct stat info;
            ASSERT_TRUE( 0 == ::lstat( fresh.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );
            CORE::Remove_Directory( dir.C_String(), 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformMcpStreamableHttpTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP Streamable HTTP TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpStreamableHttp" );

    PerformMcpHttpRegistrationTests();
    PerformMcpHttpConversationTests();
    PerformMcpHttpRefusalTests();
    PerformMcpHttpLegacyTests();
    PerformMcpHttpLoadTests();
    PerformMcpHttpHardeningTests();
}
