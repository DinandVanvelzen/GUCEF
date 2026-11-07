/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#include <signal.h>

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CMCPSTDIOTRANSPORT_H
#include "gucefCOM_CMcpStdioTransport.h"
#define GUCEF_COM_CMCPSTDIOTRANSPORT_H
#endif /* GUCEF_COM_CMCPSTDIOTRANSPORT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestMcpSupport.h"
#include "TestMcpClientSupport.h"
#include "TestMcpStdio.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Counts what a server tells about its transports and the end of its input */
class CTransportEvents : public CORE::CObserver
{
    public:

    CTransportEvents( COM::CMcpServer& server )
        : CORE::CObserver(), shutdownRequests( 0 ), transportErrors( 0 ), lastReason()
    {
        typedef CORE::CTEventHandlerFunctor< CTransportEvents > TCallback;
        TCallback onShutdown( this, &CTransportEvents::OnShutdown );
        SubscribeTo( &server, COM::CMcpServer::ShutdownRequestedEvent, onShutdown );
        TCallback onError( this, &CTransportEvents::OnError );
        SubscribeTo( &server, COM::CMcpServer::TransportErrorEvent, onError );
    }

    virtual ~CTransportEvents()
    {
        UnsubscribeAllFromObserver();
    }

    void OnShutdown( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        ++shutdownRequests;
        const CORE::TCloneableString* reason = static_cast< const CORE::TCloneableString* >( eventData );
        if ( GUCEF_NULL != reason )
            lastReason = reason->GetData();
    }

    void OnError( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++transportErrors; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTransportEvents";
        return classTypeName;
    }

    GUCEF::UInt32 shutdownRequests;
    GUCEF::UInt32 transportErrors;
    CORE::CString lastReason;
};

/*-------------------------------------------------------------------------*/

/** A server with the stdio transport on a pair of pipes; the test is the client at the other end */
class CStdioFixture
{
    public:

    CStdioFixture( bool stopOnClose = true, UInt32 maxMessageBytes = 0 )
        : rig()
        , transport( GUCEF_NULL )
        , transportPtr()
        , events( GUCEF_NULL )
        , clientToServer( GUCEF_NULL )
        , serverToClient( GUCEF_NULL )
        , serverInput( CORE::CProcessPipe::GetInvalidHandle() )
        , serverOutput( CORE::CProcessPipe::GetInvalidHandle() )
        , lines()
        , started( false )
    {
        CORE::CProcessPipe::TNativeHandle read1, write1, read2, write2;
        if ( !McpTestMakePipe( read1, write1 ) || !McpTestMakePipe( read2, write2 ) )
            return;
        // the server reads read1 and writes write2; the client writes write1 and reads read2
        serverInput = read1;
        serverOutput = write2;
        clientToServer = GUCEF_NEW CORE::CProcessPipe( write1, CORE::CProcessPipe::PIPE_WRITE_TO_CHILD );
        serverToClient = GUCEF_NEW CORE::CProcessPipe( read2, CORE::CProcessPipe::PIPE_READ_FROM_CHILD );

        COM::CMcpServerSettings settings = rig.server->GetSettings();
        settings.stopOnStdinClosed = stopOnClose;
        if ( 0 != maxMessageBytes )
            settings.maxMessageBytes = maxMessageBytes;
        rig.server->SetSettings( settings );

        CORE::CString error;
        rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error );
        CMcpTestTool* slow = GUCEF_NEW CMcpTestTool( "slow", CMcpTestTool::COUNT, COM::MCP_EXECUTION_THREAD_POOL );
        slow->steps = 600;
        slow->stepMs = 5;
        slowTool = slow;
        rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( slow ), error );

        // the transport comes out of the factory of the module, as a configuration would make it
        transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "stdio" );
        if ( transportPtr.IsNULL() )
            return;
        transport = static_cast< COM::CMcpStdioTransport* >( transportPtr.GetPointerAlways() );
        transport->SetStreams( serverInput, serverOutput );
        events = GUCEF_NEW CTransportEvents( *rig.server );
        rig.server->AddTransport( transportPtr );
        started = rig.server->Start();
    }

    ~CStdioFixture()
    {
        GUCEF_DELETE events;
        rig.server->Stop( false );
        // the transport stopped its threads: the pipes are ours to close now
        GUCEF_DELETE clientToServer;
        GUCEF_DELETE serverToClient;
        CloseHandleIfOpen( serverInput );
        CloseHandleIfOpen( serverOutput );
    }

    static void CloseHandleIfOpen( CORE::CProcessPipe::TNativeHandle handle )
    {
        if ( CORE::CProcessPipe::GetInvalidHandle() == handle )
            return;
        CORE::CProcessPipe holder( handle, CORE::CProcessPipe::PIPE_READ_FROM_CHILD );      // closes it
    }

    /** Takes in what the server wrote, pulsing the server meanwhile */
    void Pump( UInt32 pulses = 1 )
    {
        for ( UInt32 i=0; i<pulses; ++i )
        {
            rig.Pulse();
            char buffer[ 8192 ];
            while ( GUCEF_NULL != serverToClient && serverToClient->Opened() && serverToClient->WaitForData( 0 ) )
            {
                UInt32 got = serverToClient->ReadSome( buffer, sizeof( buffer ) );
                if ( 0 == got )
                    break;
                lines.Append( buffer, got );
            }
        }
    }

    bool SendRaw( const CORE::CString& bytes )
    {
        // bytes: Length() counts characters
        return GUCEF_NULL != clientToServer && 1 == clientToServer->Write( bytes.C_String(), bytes.ByteSize() > 0 ? bytes.ByteSize() - 1 : 0, 1 );
    }

    bool SendLine( const CORE::CString& line )
    {
        return SendRaw( line + "\n" );
    }

    bool WaitForLine( CORE::CString& line, UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs; ++waited )
        {
            if ( lines.PopLine( line ) )
                return true;
            Pump();
        }
        return false;
    }

    /** Waits for the response with the id; other lines that come before it are dropped */
    bool WaitForResponse( const CORE::CString& id, CORE::CDataNode& response, UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs; ++waited )
        {
            CORE::CString line;
            while ( lines.PopLine( line ) )
            {
                CORE::CDataNode node;
                if ( McpTestParse( line, node ) && GUCEF_NULL == node.GetAttribute( "method" ) && GUCEF_NULL != node.GetAttribute( "id" ) && node.GetAttribute( "id" )->second.AsString() == id )
                {
                    response = node;
                    return true;
                }
            }
            Pump();
        }
        return false;
    }

    bool Call( const CORE::CString& id, const CORE::CString& request, CORE::CDataNode& response, UInt32 timeoutInMs = 5000 )
    {
        return SendLine( request ) && WaitForResponse( id, response, timeoutInMs );
    }

    /** Pumps until the condition is true or the time is over */
    bool PumpUntilShutdownRequested( UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs && events->shutdownRequests == 0; ++waited )
            Pump();
        return events->shutdownRequests > 0;
    }

    CMcpTestRig rig;
    COM::CMcpStdioTransport* transport;
    COM::CIMcpTransportPtr transportPtr;
    CTransportEvents* events;
    CMcpTestTool* slowTool;
    CORE::CProcessPipe* clientToServer;
    CORE::CProcessPipe* serverToClient;
    CORE::CProcessPipe::TNativeHandle serverInput;
    CORE::CProcessPipe::TNativeHandle serverOutput;
    CMcpTestLineBuffer lines;
    bool started;
};

/*-------------------------------------------------------------------------*/

CORE::CString
FindEchoServer( void )
{
    CORE::CString dir = CORE::RelativePath( "$MODULEDIR$" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const char* names[] = { "gucefMcpEcho.exe", "gucefMcpEcho_d.exe" };
    #else
    const char* names[] = { "gucefMcpEcho", "gucefMcpEcho_d" };
    #endif
    for ( int i=0; i<2; ++i )
    {
        CORE::CString candidate = CORE::CombinePath( dir, names[ i ] );
        if ( CORE::FileExists( candidate ) )
            return candidate;
    }
    return CORE::CString::Empty;
}

CORE::CProcess::TStringMap
ChildEnvironment( void )
{
    // the log of the child goes to its standard error, never to a window
    CORE::CProcess::TStringMap environment;
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "stderr";
    return environment;
}

/** A request with the capabilities of a client that can elicit */
const char* const ElicitationCapabilities = "{\"elicitation\":{\"form\":{}}}";

CORE::CString
Text( const CORE::CDataNode& response )
{
    return McpTestFirstText( response.FindChild( "result" ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpStdioTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP STDIO TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpStdio" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: The stdio transport over pipes: discover, list and call, one JSON message per line" )
        try
        {
            CStdioFixture fixture;
            ASSERT_TRUE( fixture.started );
            ASSERT_TRUE( GUCEF_NULL != fixture.transport && fixture.transport->IsActive() );
            ASSERT_TRUE( fixture.transport->GetTransportName() == "stdio" && fixture.transport->GetEndpointDescription() == "stdio" );
            if ( !fixture.started )
                return;

            CORE::CDataNode response;
            ASSERT_TRUE( fixture.Call( "1", McpTestRequest( "1", "server/discover" ), response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "resultType" ) == "complete" );
            ASSERT_TRUE( fixture.Call( "2", McpTestRequest( "2", "tools/list" ), response ) );
            ASSERT_TRUE( 2 == McpTestChildCount( McpTestResultChild( response, "tools" ) ) );
            ASSERT_TRUE( fixture.Call( "3", McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"over a pipe\"}" ), response ) );
            ASSERT_TRUE( Text( response ) == "echo: over a pipe" );

            // errors are answered, and the channel goes on
            ASSERT_TRUE( fixture.Call( "4", McpTestRequest( "4", "tools/call", "\"name\":\"nope\",\"arguments\":{}" ), response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( fixture.Call( "5", McpTestRequest( "5", "ping" ), response ) && -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( fixture.Call( "6", McpTestRequest( "6", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"again\"}" ), response ) && Text( response ) == "echo: again" );

            // two, three and four bytes per character, byte for byte, with CRLF on the way in
            const CORE::CString multi = "caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80 \xc3\xa9\xf0\x9f\x98\x80";
            ASSERT_TRUE( fixture.SendRaw( McpTestRequest( "7", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"" + multi + "\"}" ) + "\r\n" ) );
            ASSERT_TRUE( fixture.WaitForResponse( "7", response ) && Text( response ) == "echo: " + multi );

            // nothing but JSON on the output, and the principal is the parent process
            fixture.Pump( 20 );
            ASSERT_TRUE( 0 == fixture.lines.GetUnparsableCount() );
            ASSERT_TRUE( fixture.transport->GetChannel()->GetPrincipal().subject == "local:parent-process" );
            ASSERT_TRUE( fixture.transport->GetChannel()->GetPrincipal().transport == "stdio" );
            ASSERT_TRUE( fixture.transport->GetChannel()->GetKind() == COM::CMcpChannel::CHANNEL_STREAM );
            ASSERT_TRUE( 0 == fixture.events->shutdownRequests );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Stdio framing: lines in pieces, CRLF, blank lines, garbage, a batch and a line that is too large" )
        try
        {
            CStdioFixture fixture( true, 2000 );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;

            CORE::CDataNode response;
            const CORE::CString request = McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"pieces\"}" );
            ASSERT_TRUE( fixture.SendRaw( request.SubstrToIndex( 20, true ) ) );
            fixture.Pump( 5 );
            ASSERT_TRUE( fixture.SendRaw( CORE::CString( request.C_String() + 20, request.Length() - 20 ) + "\r\n" ) );
            ASSERT_TRUE( fixture.WaitForResponse( "1", response ) && Text( response ) == "echo: pieces" );

            // blank lines are nothing; two messages in one write are two messages
            ASSERT_TRUE( fixture.SendRaw( "\n\r\n\n" + McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"one\"}" ) + "\n" +
                                          McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"two\"}" ) + "\n" ) );
            ASSERT_TRUE( fixture.WaitForResponse( "2", response ) && Text( response ) == "echo: one" );
            ASSERT_TRUE( fixture.WaitForResponse( "3", response ) && Text( response ) == "echo: two" );

            // garbage is a parse error without an id, a batch an invalid request
            ASSERT_TRUE( fixture.SendLine( "this is not json" ) );
            CORE::CString line;
            ASSERT_TRUE( fixture.WaitForLine( line ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32700 == McpTestErrorCode( response ) );
            ASSERT_TRUE( fixture.SendLine( "[" + McpTestRequest( "4", "tools/list" ) + "]" ) );
            ASSERT_TRUE( fixture.WaitForLine( line ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32600 == McpTestErrorCode( response ) );

            // a line over the limit is skipped without being kept, answered without an id, and the next one is served
            CORE::CString huge;
            for ( UInt32 i=0; i<500; ++i )
                huge += "0123456789";
            ASSERT_TRUE( fixture.SendRaw( huge ) );
            fixture.Pump( 5 );
            ASSERT_TRUE( fixture.SendRaw( huge + "\n" + McpTestRequest( "5", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"after\"}" ) + "\n" ) );
            ASSERT_TRUE( fixture.WaitForLine( line ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32600 == McpTestErrorCode( response ) && McpTestErrorMessage( response ).HasSubstr( "too large" ) >= 0 );
            ASSERT_TRUE( fixture.WaitForResponse( "5", response ) && Text( response ) == "echo: after" );
            ASSERT_TRUE( 0 == fixture.lines.GetUnparsableCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The end of the input cancels the work, closes the channel and asks for a shutdown when the setting says so" )
        try
        {
            {
                CStdioFixture fixture( true );
                ASSERT_TRUE( fixture.started );
                if ( !fixture.started )
                    return;
                CORE::CDataNode response;

                // a line without a line end at the end of the input is still a message
                ASSERT_TRUE( fixture.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response ) );

                // work in flight on the pool is cancelled, and its answer never comes
                ASSERT_TRUE( fixture.SendLine( McpTestRequest( "2", "tools/call", "\"name\":\"slow\",\"arguments\":{\"steps\":600}" ) ) );
                for ( UInt32 i=0; i<3000 && fixture.slowTool->executions.Load() < 1; ++i )
                    fixture.Pump();
                ASSERT_TRUE( fixture.slowTool->executions.Load() >= 1 );
                ASSERT_TRUE( fixture.SendRaw( McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"last\"}" ) ) );
                fixture.Pump( 20 );
                fixture.clientToServer->Close();                 // the client closes its end
                ASSERT_TRUE( fixture.PumpUntilShutdownRequested() );
                ASSERT_TRUE( 1 == fixture.events->shutdownRequests );
                ASSERT_TRUE( fixture.events->lastReason.HasSubstr( "stdio" ) >= 0 );
                ASSERT_TRUE( fixture.transport->HasPeerGone() );
                ASSERT_TRUE( fixture.transport->GetChannel()->IsClosed() );
                fixture.Pump( 50 );

                // the last line without its end was processed before the end was; nothing of the cancelled call
                CORE::CString line;
                bool sawLast = false;
                bool sawSlow = false;
                while ( fixture.lines.PopLine( line ) )
                {
                    CORE::CDataNode node;
                    if ( McpTestParse( line, node ) )
                    {
                        if ( McpTestAttr( node, "id" ) == "3" )
                            sawLast = true;
                        if ( McpTestAttr( node, "id" ) == "2" )
                            sawSlow = true;
                    }
                }
                ASSERT_TRUE( sawLast );
                ASSERT_FALSE( sawSlow );
                ASSERT_TRUE( fixture.slowTool->executions.Load() >= 1 );
            }
            {
                // with the setting off the transport still lets go of the channel, but nobody is asked to stop
                CStdioFixture fixture( false );
                ASSERT_TRUE( fixture.started );
                if ( !fixture.started )
                    return;
                fixture.clientToServer->Close();
                for ( UInt32 i=0; i<500 && !fixture.transport->HasPeerGone(); ++i )
                    fixture.Pump();
                ASSERT_TRUE( fixture.transport->HasPeerGone() );
                fixture.Pump( 20 );
                ASSERT_TRUE( 0 == fixture.events->shutdownRequests );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A client that stops reading and goes away ends the transport, without a signal" )
        try
        {
            CStdioFixture fixture( true );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.Call( "1", McpTestRequest( "1", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"x\"}" ), response ) );

            // the reading end of the output goes away: the next write finds a broken pipe. SIGPIPE would end this process
            fixture.serverToClient->Close();
            ASSERT_TRUE( fixture.SendLine( McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"y\"}" ) ) );
            ASSERT_TRUE( fixture.PumpUntilShutdownRequested() );
            ASSERT_TRUE( fixture.events->transportErrors >= 1 );
            ASSERT_TRUE( fixture.transport->HasPeerGone() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    GUCEF_TESTFW_TESTCASE( "Test: Stdout is claimed by the transport and refused when claimed already, SIGPIPE only changes when asked" )
        try
        {
            CMcpTestRig rig;
            CORE::CString error;
            COM::CIMcpTransportPtr first = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "stdio" );
            ASSERT_FALSE( first.IsNULL() );

            // somebody else has the claim: the standard streams are not used, the transport does not start
            ASSERT_TRUE( CORE::CStandardOutputWriter::ClaimExclusive() );
            CTransportEvents events( *rig.server );
            ASSERT_FALSE( first->Start( *rig.server ) );
            ASSERT_FALSE( first->IsActive() );
            rig.Pulse( 3 );
            CORE::CStandardOutputWriter::ReleaseClaim();
            ASSERT_FALSE( CORE::CStandardOutputWriter::IsClaimed() );

            // streams of our own: no claim, and the SIGPIPE disposition is not touched
            struct sigaction before;
            ::sigaction( SIGPIPE, GUCEF_NULL, &before );
            {
                CStdioFixture fixture;
                ASSERT_TRUE( fixture.started );
                ASSERT_FALSE( CORE::CStandardOutputWriter::IsClaimed() );
                ASSERT_FALSE( fixture.transport->GetIgnoreSigPipe() );
                struct sigaction during;
                ::sigaction( SIGPIPE, GUCEF_NULL, &during );
                ASSERT_TRUE( during.sa_handler == before.sa_handler );
            }

            // asked for: ignored for the process, and put back by the application afterwards
            {
                CStdioFixture fixture;
                ASSERT_TRUE( fixture.started );
                fixture.rig.server->Stop( false );
                fixture.transport->SetIgnoreSigPipe( true );
                ASSERT_TRUE( fixture.rig.server->Start() );
                struct sigaction during;
                ::sigaction( SIGPIPE, GUCEF_NULL, &during );
                ASSERT_TRUE( SIG_IGN == during.sa_handler );
            }
            CORE::CStandardOutputWriter::SetSigPipeIgnoredProcessWide( false );
            struct sigaction after;
            ::sigaction( SIGPIPE, GUCEF_NULL, &after );
            ASSERT_TRUE( SIG_IGN != after.sa_handler );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The configuration of the server makes the stdio transport through the factory" )
        try
        {
            CMcpTestRig rig;
            CORE::CDataNode config;
            ASSERT_TRUE( McpTestParse( "{\"McpServer\":{\"serverName\":\"configured\",\"stopOnStdinClosed\":false,\"Transports\":[{\"type\":\"stdio\",\"ignoreSigPipe\":false,\"scopes\":\"read,write\"},{\"type\":\"tcp\",\"port\":0,\"requireToken\":false}]}}", config ) );
            ASSERT_TRUE( rig.server->LoadConfig( config ) );
            ASSERT_FALSE( rig.server->GetSettings().stopOnStdinClosed );
            ASSERT_TRUE( rig.server->GetSettings().serverName == "configured" );

            // an unknown type is reported and the others are made
            ASSERT_TRUE( McpTestParse( "{\"McpServer\":{\"Transports\":[{\"type\":\"carrier-pigeon\"},{\"type\":\"tcp\",\"port\":0,\"requireToken\":false}]}}", config ) );
            ASSERT_FALSE( rig.server->LoadConfig( config ) );
            ASSERT_TRUE( COM::CComGlobal::Instance()->GetMcpTransportFactory().IsConstructible( "stdio" ) );
            ASSERT_TRUE( COM::CComGlobal::Instance()->GetMcpTransportFactory().IsConstructible( "tcp" ) );
            ASSERT_FALSE( COM::CComGlobal::Instance()->GetMcpTransportFactory().IsConstructible( "streamable-http" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif

    GUCEF_TESTFW_TESTCASE( "Test: End to end: the child process gucefMcpEcho over its standard streams, in the 2026-07-28 era" )
        try
        {
            const CORE::CString exe = FindEchoServer();
            GUCEF_TESTFW_SKIP_IF( exe.IsNULLOrEmpty(), "tools/gucefMcpEcho is not next to the test application" );

            CMcpSubprocessClient client;
            ASSERT_TRUE( client.Launch( exe, CORE::CProcess::TStringVector(), ChildEnvironment() ) );
            CORE::CDataNode response;
            GUCEF::vector< CORE::CDataNode > others;

            // discover, no handshake before it
            ASSERT_TRUE( client.SendLine( McpTestRequest( "1", "server/discover" ) ) );
            ASSERT_TRUE( client.ReadResponse( "1", response, others, 20000 ) );
            ASSERT_TRUE( McpTestResultAttr( response, "resultType" ) == "complete" );
            const CORE::CDataNode* capabilities = McpTestResultChild( response, "capabilities" );
            ASSERT_TRUE( GUCEF_NULL != capabilities && GUCEF_NULL != capabilities->FindChild( "tools" ) );

            // tools: the read-only ones, the write tool is hidden until the policy names it
            ASSERT_TRUE( client.SendLine( McpTestRequest( "2", "tools/list" ) ) );
            ASSERT_TRUE( client.ReadResponse( "2", response, others, 10000 ) );
            const CORE::CDataNode* tools = McpTestResultChild( response, "tools" );
            ASSERT_TRUE( 3 == McpTestChildCount( tools ) );
            CORE::CString names;
            if ( GUCEF_NULL != tools )
                for ( CORE::CDataNode::const_iterator i = tools->ConstBegin(); i != tools->ConstEnd(); ++i )
                    names += McpTestAttr( *(*i), "name" ) + ",";
            ASSERT_TRUE( names == "ask_name,echo,slow_count," );

            ASSERT_TRUE( client.SendLine( McpTestRequest( "3", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"end to end\"}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "3", response, others, 10000 ) );
            ASSERT_TRUE( Text( response ) == "end to end" );

            // refused and invalid
            ASSERT_TRUE( client.SendLine( McpTestRequest( "4", "tools/call", "\"name\":\"add_note\",\"arguments\":{\"title\":\"t\",\"text\":\"x\"}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "4", response, others, 10000 ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "5", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":5}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "5", response, others, 10000 ) );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "true" && Text( response ).HasSubstr( "Invalid arguments for tool echo" ) >= 0 );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "6", "no/such/method" ) ) );
            ASSERT_TRUE( client.ReadResponse( "6", response, others, 10000 ) );
            ASSERT_TRUE( -32601 == McpTestErrorCode( response ) );
            ASSERT_TRUE( client.SendLine( McpTestLegacyRequest( "7", "tools/list" ) ) );
            ASSERT_TRUE( client.ReadResponse( "7", response, others, 10000 ) );
            ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );          // no _meta, no handshake: a modern request without its _meta
            ASSERT_TRUE( client.SendLine( "{not json" ) );
            CORE::CString line;
            ASSERT_TRUE( client.ReadLine( line, 10000 ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32700 == McpTestErrorCode( response ) );
            ASSERT_TRUE( client.SendLine( "{\"jsonrpc\":\"2.0\",\"id\":8,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"1999-01-01\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}" ) );
            ASSERT_TRUE( client.ReadResponse( "8", response, others, 10000 ) );
            ASSERT_TRUE( -32022 == McpTestErrorCode( response ) );

            // resources and prompts
            ASSERT_TRUE( client.SendLine( McpTestRequest( "9", "resources/read", "\"uri\":\"echo://about\"" ) ) );
            ASSERT_TRUE( client.ReadResponse( "9", response, others, 10000 ) );
            const CORE::CDataNode* contents = McpTestResultChild( response, "contents" );
            ASSERT_TRUE( 1 == McpTestChildCount( contents ) );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "10", "prompts/get", "\"name\":\"greet\",\"arguments\":{\"name\":\"Ada\"}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "10", response, others, 10000 ) );
            ASSERT_TRUE( GUCEF_NULL != McpTestResultChild( response, "messages" ) );

            // multi round-trip: without the capability a tool error, with it a request for input and a retry with the state
            ASSERT_TRUE( client.SendLine( McpTestRequest( "11", "tools/call", "\"name\":\"ask_name\",\"arguments\":{}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "11", response, others, 10000 ) );
            ASSERT_TRUE( McpTestResultAttr( response, "isError" ).Lowercase() == "true" );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "12", "tools/call", "\"name\":\"ask_name\",\"arguments\":{}", ElicitationCapabilities ) ) );
            ASSERT_TRUE( client.ReadResponse( "12", response, others, 10000 ) );
            ASSERT_TRUE( McpTestResultAttr( response, "resultType" ) == "input_required" );
            CORE::CString state = McpTestResultAttr( response, "requestState" );
            ASSERT_TRUE( !state.IsNULLOrEmpty() && state != "<none>" );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "13", "tools/call", "\"name\":\"ask_name\",\"arguments\":{},\"requestState\":\"" + state + "\","
                                                          "\"inputResponses\":{\"name_request\":{\"action\":\"accept\",\"content\":{\"name\":\"Bob\"}}}", ElicitationCapabilities ) ) );
            ASSERT_TRUE( client.ReadResponse( "13", response, others, 10000 ) );
            ASSERT_TRUE( Text( response ) == "Hello Bob" );

            // work that takes time: progress comes, a cancelled call gets no answer, and the server is still there
            ASSERT_TRUE( client.SendLine( McpTestRequest( "14", "tools/call", "\"name\":\"slow_count\",\"arguments\":{\"steps\":3,\"stepMs\":30}", "{}", "\"progressToken\":\"p1\"" ) ) );
            others.clear();
            ASSERT_TRUE( client.ReadResponse( "14", response, others, 10000 ) );
            ASSERT_TRUE( Text( response ) == "counted to 3" );
            UInt32 progressSeen = 0;
            for ( size_t i=0; i<others.size(); ++i )
                if ( McpTestAttr( others[ i ], "method" ) == "notifications/progress" )
                    ++progressSeen;
            ASSERT_TRUE( progressSeen >= 1 );

            ASSERT_TRUE( client.SendLine( McpTestRequest( "15", "tools/call", "\"name\":\"slow_count\",\"arguments\":{\"steps\":1000,\"stepMs\":10}" ) ) );
            client.Pump( 150 );
            ASSERT_TRUE( client.SendLine( McpTestNotification( "notifications/cancelled", "\"requestId\":15" ) ) );
            ASSERT_FALSE( client.TryReadResponse( "15", response, 1500 ) );
            ASSERT_TRUE( client.SendLine( McpTestRequest( "16", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"still here\"}" ) ) );
            others.clear();
            ASSERT_TRUE( client.ReadResponse( "16", response, others, 10000 ) );
            ASSERT_TRUE( Text( response ) == "still here" );
            for ( size_t i=0; i<others.size(); ++i )
                ASSERT_TRUE( McpTestAttr( others[ i ], "id" ) != "15" );

            // the legacy era on the same channel
            ASSERT_TRUE( client.SendLine( McpTestLegacyRequest( "17", "initialize", "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{},\"clientInfo\":{\"name\":\"old\",\"version\":\"1\"}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "17", response, others, 10000 ) );
            ASSERT_TRUE( McpTestResultAttr( response, "protocolVersion" ) == "2025-11-25" );
            ASSERT_TRUE( client.SendLine( McpTestNotification( "notifications/initialized" ) ) );
            ASSERT_TRUE( client.SendLine( McpTestLegacyRequest( "18", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"legacy\"}" ) ) );
            ASSERT_TRUE( client.ReadResponse( "18", response, others, 10000 ) );
            ASSERT_TRUE( Text( response ) == "legacy" && McpTestResultAttr( response, "resultType" ) == "<none>" );

            // closing the input is the end: the child exits by itself and promptly, and quietly with respect to standard output
            client.CloseInput();
            Int32 exitCode = -1;
            ASSERT_TRUE( client.WaitForExit( 10000, exitCode ) );
            ASSERT_TRUE( 0 == exitCode );
            client.Pump( 50 );
            while ( client.ReadLine( line, 0 ) ) {}
            ASSERT_TRUE( 0 == client.Stdout().GetUnparsableCount() );
            ASSERT_TRUE( client.Stdout().GetLineCount() > 10 );

            // the log is on standard error, and only there
            ASSERT_TRUE( client.GetStderrText().HasSubstr( "gucefMcpEcho: Serving on stdio" ) >= 0 );
            ASSERT_TRUE( client.GetStderrText().HasSubstr( "\"jsonrpc\"" ) < 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: End to end: a named write tool, the notes it makes, and the audit log of the child" )
        try
        {
            const CORE::CString exe = FindEchoServer();
            GUCEF_TESTFW_SKIP_IF( exe.IsNULLOrEmpty(), "tools/gucefMcpEcho is not next to the test application" );

            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpEchoAudit" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString auditPath = CORE::CombinePath( dir, "audit.jsonl" );
            CORE::DeleteFile( auditPath );

            CORE::CProcess::TStringVector args;
            args.push_back( "--allow-tool" );
            args.push_back( "add_note" );
            args.push_back( "--audit-log" );
            args.push_back( auditPath );

            {
                CMcpSubprocessClient client;
                ASSERT_TRUE( client.Launch( exe, args, ChildEnvironment() ) );
                CORE::CDataNode response;
                GUCEF::vector< CORE::CDataNode > others;

                ASSERT_TRUE( client.SendLine( McpTestRequest( "1", "tools/list" ) ) );
                ASSERT_TRUE( client.ReadResponse( "1", response, others, 20000 ) );
                ASSERT_TRUE( 4 == McpTestChildCount( McpTestResultChild( response, "tools" ) ) );
                ASSERT_TRUE( client.SendLine( McpTestRequest( "2", "tools/call", "\"name\":\"add_note\",\"arguments\":{\"title\":\"hello\",\"text\":\"password=hunter2 is a secret\"}" ) ) );
                ASSERT_TRUE( client.ReadResponse( "2", response, others, 10000 ) );
                ASSERT_TRUE( Text( response ) == "stored note 1" );
                ASSERT_TRUE( client.SendLine( McpTestRequest( "3", "resources/read", "\"uri\":\"echo://notes/1\"" ) ) );
                ASSERT_TRUE( client.ReadResponse( "3", response, others, 10000 ) );
                ASSERT_TRUE( 1 == McpTestChildCount( McpTestResultChild( response, "contents" ) ) );
                ASSERT_TRUE( client.SendLine( McpTestRequest( "4", "resources/read", "\"uri\":\"echo://notes/9\"" ) ) );
                ASSERT_TRUE( client.ReadResponse( "4", response, others, 10000 ) );
                ASSERT_TRUE( -32602 == McpTestErrorCode( response ) );

                client.CloseInput();
                Int32 exitCode = -1;
                ASSERT_TRUE( client.WaitForExit( 10000, exitCode ) );
                ASSERT_TRUE( 0 == exitCode );
            }

            // one line per request, each parses, the secret is not in the file but the digest of the call is
            CORE::CDynamicBuffer content;
            ASSERT_TRUE( content.LoadContentFromFile( auditPath ) );
            CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
            ASSERT_TRUE( text.HasSubstr( "hunter2" ) < 0 );
            CORE::CString::StringVector lines = text.ParseElements( '\n', false );
            UInt32 records = 0;
            bool sawNoteCall = false;
            for ( size_t i=0; i<lines.size(); ++i )
            {
                if ( lines[ i ].IsNULLOrEmpty() )
                    continue;
                CORE::CDataNode node;
                ASSERT_TRUE( McpTestParse( lines[ i ], node ) );
                ++records;
                ASSERT_TRUE( McpTestAttr( node, "transport" ) == "stdio" && McpTestAttr( node, "subject" ) == "local:parent-process" );
                if ( McpTestAttr( node, "target" ) == "add_note" )
                {
                    sawNoteCall = true;
                    ASSERT_TRUE( McpTestAttr( node, "argumentsDigest" ).Length() == 64 && McpTestAttr( node, "decision" ) == "allow" );
                }
            }
            ASSERT_TRUE( 4 == records );
            ASSERT_TRUE( sawNoteCall );
            CORE::DeleteFile( auditPath );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: End to end: bad options and an input that ends at once end the child cleanly" )
        try
        {
            const CORE::CString exe = FindEchoServer();
            GUCEF_TESTFW_SKIP_IF( exe.IsNULLOrEmpty(), "tools/gucefMcpEcho is not next to the test application" );
            Int32 exitCode = -1;
            {
                CMcpSubprocessClient client;
                CORE::CProcess::TStringVector args;
                args.push_back( "--no-such-option" );
                ASSERT_TRUE( client.Launch( exe, args, ChildEnvironment() ) );
                ASSERT_TRUE( client.WaitForExit( 20000, exitCode ) );
                ASSERT_TRUE( 2 == exitCode );
                ASSERT_TRUE( client.Stdout().GetLineCount() == 0 );
            }
            {
                // the client closes the input before it said anything: the server has nothing to do and leaves
                CMcpSubprocessClient client;
                ASSERT_TRUE( client.Launch( exe, CORE::CProcess::TStringVector(), ChildEnvironment() ) );
                client.CloseInput();
                ASSERT_TRUE( client.WaitForExit( 20000, exitCode ) );
                ASSERT_TRUE( 0 == exitCode );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL MCP STDIO TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
