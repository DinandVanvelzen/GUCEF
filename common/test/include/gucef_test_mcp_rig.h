/*
 *  gucef common header: header-only in-memory MCP conversation rig for tests
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

/*
 *  Test support: a COM::CMcpServer and a client over an in-memory channel (gucef_test_mcp_channel.h), for the tests of
 *  any MCP tool set. The server runs on a pulse generator that the test thread drives, so the test thread is the
 *  protocol thread; tools on the thread pool and deferred tools work as they do over a real transport.
 *
 *      GUCEF::TESTFW::CTestMcpRig rig;                     // or rig( "local:tester", scopes ) for another principal
 *      rig.GetServer().GetToolRegistry().Register( myTool, error );
 *      GUCEF::CORE::CDataNode response;
 *      rig.CallTool( "1", "my_tool", "{\"packId\":\"p\"}", response );
 *      const GUCEF::CORE::CDataNode* result = response.FindChild( "result" );
 *
 *  Converse() pumps the server's pulse generator until the response of the request's id arrived (or the time is up);
 *  the notifications that came meanwhile (progress, list changes) are kept on the channel. Requests are modern
 *  (2026-07-28) requests with the _meta the specification asks for.
 *
 *  For test applications that link gucefCOM; the JSON codec plugin (dstorepluginJSONPARSER) must be loaded.
 */

#ifndef GUCEF_TEST_MCP_RIG_H
#define GUCEF_TEST_MCP_RIG_H

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_TEST_MCP_CHANNEL_H
#include "gucef_test_mcp_channel.h"
#define GUCEF_TEST_MCP_CHANNEL_H
#endif /* GUCEF_TEST_MCP_CHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A server on a pulse generator of the test thread and one client channel on it */
class CTestMcpRig
{
    public:

    static const char* ProtocolVersion( void ) { return "2026-07-28"; }

    /** A server of its own; the client is the principal ( "test", subject ) with the scopes */
    explicit CTestMcpRig( const char* subject = "local:test"                                 ,
                          const CORE::CString::StringVector& scopes = CORE::CString::StringVector() ,
                          COM::CMcpChannel::EKind kind = COM::CMcpChannel::CHANNEL_STREAM    )
        : m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , m_server( GUCEF_NULL )
        , m_channelPtr()
        , m_channel( GUCEF_NULL )
    {
        m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        m_server = GUCEF_NEW COM::CMcpServer( m_pulseGenerator );
        COM::CMcpImplementationInfo info;
        info.name = "gucef-test-server";
        info.version = "1.0.0";
        m_server->SetServerInfo( info );
        COM::CMcpPrincipal principal( "test", subject, true );
        principal.scopes = scopes;
        m_channel = GUCEF_NEW CTestMcpChannel( kind, principal );
        m_channelPtr = COM::CMcpChannelPtr( m_channel );
        m_server->OpenChannel( m_channelPtr );
    }

    ~CTestMcpRig()
    {
        m_server->CloseChannel( m_channelPtr );
        m_channelPtr.Unlink();
        m_channel = GUCEF_NULL;
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }

    COM::CMcpServer& GetServer( void ) { return *m_server; }

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const { return m_pulseGenerator; }

    CTestMcpChannel& GetChannel( void ) { return *m_channel; }

    /** Drives the server: one pulse per time, with a millisecond between them */
    void Pump( UInt32 times = 1 )
    {
        for ( UInt32 i=0; i<times; ++i )
        {
            m_pulseGenerator->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }
    }

    /** A message from the client, as JSON text */
    bool Send( const CORE::CString& messageText )
    {
        return m_channel->Receive( messageText );
    }

    /** Pumps until the response with the id (as JSON text) was sent or the time is up */
    bool WaitForResponse( const CORE::CString& idText, CORE::CDataNode& response, UInt32 timeoutMs = 5000 )
    {
        const UInt64 start = MT::PrecisionTickCount();
        while ( true )
        {
            if ( m_channel->FindResponse( idText, response ) )
                return true;
            if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
                return false;
            Pump();
        }
    }

    /** Sends the request and waits for its response; idText is the id as JSON text: 7 or "a" */
    bool Converse( const CORE::CString& idText, const CORE::CString& requestText, CORE::CDataNode& response, UInt32 timeoutMs = 5000 )
    {
        if ( !Send( requestText ) )
            return false;
        return WaitForResponse( idText, response, timeoutMs );
    }

    /** A modern request; 'params' are the members besides _meta, as JSON text without the braces */
    bool Request( const CORE::CString& idText                                        ,
                  const CORE::CString& method                                        ,
                  const CORE::CString& params                                        ,
                  CORE::CDataNode& response                                          ,
                  UInt32 timeoutMs = 5000                                            ,
                  const CORE::CString& extraMeta = CORE::CString::Empty              )
    {
        return Converse( idText, MakeRequest( idText, method, params, "{}", extraMeta ), response, timeoutMs );
    }

    /** tools/call; 'argumentsJson' is a JSON object text; a progress token is asked for when one is given */
    bool CallTool( const CORE::CString& idText                                       ,
                   const CORE::CString& toolName                                     ,
                   const CORE::CString& argumentsJson                                ,
                   CORE::CDataNode& response                                         ,
                   UInt32 timeoutMs = 5000                                           ,
                   const CORE::CString& progressToken = CORE::CString::Empty         )
    {
        CORE::CString extraMeta;
        if ( !progressToken.IsNULLOrEmpty() )
            extraMeta = "\"progressToken\":\"" + progressToken + "\"";
        return Request( idText, "tools/call", "\"name\":\"" + toolName + "\",\"arguments\":" + argumentsJson, response, timeoutMs, extraMeta );
    }

    /** notifications/cancelled of a request in flight */
    bool Cancel( const CORE::CString& idText, const CORE::CString& reason = "cancelled by the test" )
    {
        return Send( "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":" + idText + ",\"reason\":\"" + reason + "\"}}" );
    }

    /** The notifications of a method that were sent, in order */
    UInt32 GetNotifications( const CORE::CString& method, CTestMcpChannel::TMessageVector& found ) const
    {
        return m_channel->GetNotifications( method, found );
    }

    /** The _meta of a modern request with the given client capabilities (JSON object text) and extra members */
    static CORE::CString MakeMeta( const CORE::CString& capabilities = "{}", const CORE::CString& extra = CORE::CString::Empty )
    {
        CORE::CString meta = CORE::CString( "\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"" ) + ProtocolVersion() + "\","
                             "\"io.modelcontextprotocol/clientInfo\":{\"name\":\"gucef-test-client\",\"version\":\"1.0\"},"
                             "\"io.modelcontextprotocol/clientCapabilities\":" + capabilities;
        if ( !extra.IsNULLOrEmpty() )
            meta += "," + extra;
        meta += "}";
        return meta;
    }

    /** A modern request as JSON text */
    static CORE::CString MakeRequest( const CORE::CString& idText                                   ,
                                      const CORE::CString& method                                   ,
                                      const CORE::CString& params = CORE::CString::Empty            ,
                                      const CORE::CString& capabilities = "{}"                      ,
                                      const CORE::CString& extraMeta = CORE::CString::Empty         )
    {
        CORE::CString text = "{\"jsonrpc\":\"2.0\",\"id\":" + idText + ",\"method\":\"" + method + "\",\"params\":{" + MakeMeta( capabilities, extraMeta );
        if ( !params.IsNULLOrEmpty() )
            text += "," + params;
        text += "}}";
        return text;
    }

    /** The result member of a response; GUCEF_NULL for an error response */
    static const CORE::CDataNode* GetResult( const CORE::CDataNode& response )
    {
        return response.FindChild( "result" );
    }

    /** The structured content of a tools/call response; GUCEF_NULL when there is none */
    static const CORE::CDataNode* GetStructuredContent( const CORE::CDataNode& response )
    {
        const CORE::CDataNode* result = response.FindChild( "result" );
        return GUCEF_NULL != result ? result->FindChild( "structuredContent" ) : GUCEF_NULL;
    }

    /** Whether a tools/call response is a tool execution error */
    static bool IsToolError( const CORE::CDataNode& response )
    {
        const CORE::CDataNode* result = response.FindChild( "result" );
        if ( GUCEF_NULL == result )
            return false;
        const CORE::CDataNode::TKeyValuePair* isError = result->GetAttribute( "isError" );
        return GUCEF_NULL != isError && (*isError).second.AsBool( false );
    }

    /** The code of an error response, 0 for a result */
    static Int32 GetErrorCode( const CORE::CDataNode& response )
    {
        const CORE::CDataNode* error = response.FindChild( "error" );
        if ( GUCEF_NULL == error )
            return 0;
        const CORE::CDataNode::TKeyValuePair* code = error->GetAttribute( "code" );
        return GUCEF_NULL != code ? (*code).second.AsInt32( 0 ) : 0;
    }

    private:

    CTestMcpRig( const CTestMcpRig& src );              /**< not implemented */
    CTestMcpRig& operator=( const CTestMcpRig& src );   /**< not implemented */

    CORE::PulseGeneratorPtr m_pulseGenerator;
    COM::CMcpServer* m_server;
    COM::CMcpChannelPtr m_channelPtr;
    CTestMcpChannel* m_channel;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_MCP_RIG_H ? */
