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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CMCPTCPTRANSPORT_H
#include "gucefCOM_CMcpTcpTransport.h"
#define GUCEF_COM_CMCPTCPTRANSPORT_H
#endif /* GUCEF_COM_CMCPTCPTRANSPORT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#include "gucefCOMCORE_CTlsProviderRegistry.h"
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "TestMcpSupport.h"
#include "TestMcpClientSupport.h"
#include "TestMcpTcp.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

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

const char* const TokenKey = "com.vanvelzensoftware.gucef/authToken";

/** Counts the requests that reached the server, the audit records and the closed channels */
class CServerTally : public CORE::CObserver
{
    public:

    CServerTally( COM::CMcpServer& server )
        : CORE::CObserver(), started( 0 ), channelsOpened( 0 ), channelsClosed( 0 ), audited( 0 ), auditText()
    {
        typedef CORE::CTEventHandlerFunctor< CServerTally > TCallback;
        TCallback onStarted( this, &CServerTally::OnStarted );
        SubscribeTo( &server, COM::CMcpServer::RequestStartedEvent, onStarted );
        TCallback onOpened( this, &CServerTally::OnOpened );
        SubscribeTo( &server, COM::CMcpServer::ChannelOpenedEvent, onOpened );
        TCallback onClosed( this, &CServerTally::OnClosed );
        SubscribeTo( &server, COM::CMcpServer::ChannelClosedEvent, onClosed );
        TCallback onAudit( this, &CServerTally::OnAudit );
        SubscribeTo( &server, COM::CMcpServer::AuditEvent, onAudit );
    }

    virtual ~CServerTally() { UnsubscribeAllFromObserver(); }

    void OnStarted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++started; }
    void OnOpened( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++channelsOpened; }
    void OnClosed( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++channelsClosed; }

    void OnAudit( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        ++audited;
        const COM::TMcpAuditEventData* data = static_cast< const COM::TMcpAuditEventData* >( eventData );
        CORE::CString line;
        if ( GUCEF_NULL != data && data->GetData().ToJsonLine( line ) )
            auditText += line + "\n";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CServerTally";
        return classTypeName;
    }

    GUCEF::UInt32 started;
    GUCEF::UInt32 channelsOpened;
    GUCEF::UInt32 channelsClosed;
    GUCEF::UInt32 audited;
    CORE::CString auditText;
};

/** A server with the TCP transport on a free port of the loopback */
class CTcpFixture
{
    public:

    CTcpFixture( bool requireToken, const char* token = GUCEF_NULL, UInt32 maxMessageBytes = 0, const COMCORE::CTlsServerSettings* tls = GUCEF_NULL )
        : rig()
        , transport( GUCEF_NULL )
        , transportPtr()
        , tally( GUCEF_NULL )
        , started( false )
    {
        COM::CMcpServerSettings settings = rig.server->GetSettings();
        if ( 0 != maxMessageBytes )
            settings.maxMessageBytes = maxMessageBytes;
        rig.server->SetSettings( settings );

        CORE::CString error;
        rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error );

        transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
        if ( transportPtr.IsNULL() )
            return;
        transport = static_cast< COM::CMcpTcpTransport* >( transportPtr.GetPointerAlways() );
        transport->SetPort( 0 );
        transport->SetRequireToken( requireToken );
        if ( GUCEF_NULL != token )
            transport->SetToken( token );
        if ( GUCEF_NULL != tls )
            transport->SetTlsSettings( *tls );
        tally = GUCEF_NEW CServerTally( *rig.server );
        rig.server->AddTransport( transportPtr );
        started = rig.server->Start();
    }

    ~CTcpFixture()
    {
        GUCEF_DELETE tally;
        rig.server->Stop( false );
    }

    UInt16 Port( void ) const { return GUCEF_NULL != transport ? transport->GetBoundPort() : 0; }

    /** Pulses the server and takes in what the client got, for a while */
    void Pump( CMcpRawTcpClient& client, UInt32 pulses = 1 )
    {
        for ( UInt32 i=0; i<pulses; ++i )
        {
            rig.Pulse();
            client.Receive( 0 );
        }
    }

    bool WaitForLine( CMcpRawTcpClient& client, CORE::CString& line, UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs; ++waited )
        {
            if ( client.Lines().PopLine( line ) )
                return true;
            if ( client.IsClosedByPeer() )
                return false;
            Pump( client );
        }
        return false;
    }

    bool WaitForResponse( CMcpRawTcpClient& client, const CORE::CString& id, CORE::CDataNode& response, UInt32 timeoutInMs = 5000 )
    {
        CORE::CString line;
        for ( UInt32 waited=0; waited<=timeoutInMs; ++waited )
        {
            while ( client.Lines().PopLine( line ) )
            {
                CORE::CDataNode node;
                if ( McpTestParse( line, node ) && GUCEF_NULL != node.GetAttribute( "id" ) && node.GetAttribute( "id" )->second.AsString() == id )
                {
                    response = node;
                    return true;
                }
            }
            if ( client.IsClosedByPeer() )
                return false;
            Pump( client );
        }
        return false;
    }

    /** Pumps until the server closed the connection */
    bool WaitForClose( CMcpRawTcpClient& client, UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs && !client.IsClosedByPeer(); ++waited )
        {
            rig.Pulse();
            client.Receive( 1 );
        }
        return client.IsClosedByPeer();
    }

    bool WaitForConnections( UInt32 count, UInt32 timeoutInMs = 5000 )
    {
        for ( UInt32 waited=0; waited<=timeoutInMs && transport->GetConnectionCount() != count; ++waited )
            rig.Pulse();
        return transport->GetConnectionCount() == count;
    }

    CMcpTestRig rig;
    COM::CMcpTcpTransport* transport;
    COM::CIMcpTransportPtr transportPtr;
    CServerTally* tally;
    bool started;
};

/** A request that carries the token */
CORE::CString
TokenRequest( const CORE::CString& id, const CORE::CString& method, const CORE::CString& params, const CORE::CString& token )
{
    return McpTestRequest( id, method, params, "{}", CORE::CString( "\"" ) + TokenKey + "\":\"" + token + "\"" );
}

CORE::CString
EchoParams( const char* text )
{
    return CORE::CString( "\"name\":\"echo\",\"arguments\":{\"text\":\"" ) + text + "\"}";
}

/** A client over TLS on the pulse generator of the server, that keeps what it receives */
class CMcpTlsLineClient : public CORE::CObserver
{
    public:

    COMCORE::CTlsClientConnection connection;
    CORE::CString received;
    bool connected;
    bool disconnected;

    explicit CMcpTlsLineClient( const CORE::PulseGeneratorPtr& pulseGenerator )
        : CORE::CObserver(), connection( pulseGenerator, false ), received(), connected( false ), disconnected( false )
    {
        SubscribeTo( &connection );
    }

    virtual ~CMcpTlsLineClient()
    {
        UnsubscribeAllFromObserver();
        connection.Close();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CMcpTlsLineClient";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CTCPConnection::ConnectedEvent == eventId )
            connected = true;
        else
        if ( COMCORE::CTCPConnection::DisconnectedEvent == eventId )
            disconnected = true;
        else
        if ( COMCORE::CTCPConnection::DataRecievedEvent == eventId )
        {
            const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
            received.Append( data.AsConstTypePtr< char >(), data.GetDataSize() );
        }
    }
};

/** Pulses the server until the client has the text, or closed */
bool
PumpUntilReceived( CTcpFixture& fixture, CMcpTlsLineClient& client, const char* text, UInt32 timeoutInMs = 5000 )
{
    for ( UInt32 waited=0; waited<=timeoutInMs && client.received.HasSubstr( text ) < 0 && !client.disconnected; ++waited )
    {
        fixture.rig.Pulse();
        MT::ThreadDelay( 1 );
    }
    return client.received.HasSubstr( text ) >= 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpTcpTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING MCP TCP TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "McpTcp" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP TESTS skipped: the json codec is not available" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Test: The TCP transport listens on a free loopback port and serves lines, split or together" )
        try
        {
            CTcpFixture fixture( false );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            ASSERT_TRUE( fixture.transport->IsActive() && fixture.Port() > 0 );
            ASSERT_TRUE( fixture.transport->GetEndpointDescription() == "tcp://127.0.0.1:" + CORE::UInt32ToString( fixture.Port() ) );
            ASSERT_TRUE( fixture.transport->GetTransportName() == "tcp" );

            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            CORE::CDataNode response;
            ASSERT_TRUE( client.SendLine( McpTestRequest( "1", "server/discover" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "1", response ) );
            ASSERT_TRUE( McpTestResultAttr( response, "resultType" ) == "complete" );

            // a request in two pieces with CRLF, then two in one write
            const CORE::CString request = McpTestRequest( "2", "tools/call", EchoParams( "pieces" ) );
            ASSERT_TRUE( client.Send( CORE::CString( request.C_String(), 15 ) ) );
            fixture.Pump( client, 10 );
            ASSERT_TRUE( client.Send( CORE::CString( request.C_String() + 15, request.Length() - 15 ) + "\r\n" ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "2", response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: pieces" );
            ASSERT_TRUE( client.Send( McpTestRequest( "3", "tools/call", EchoParams( "a" ) ) + "\n" + McpTestRequest( "4", "tools/call", EchoParams( "b" ) ) + "\n" ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "3", response ) && fixture.WaitForResponse( client, "4", response ) );

            // the principal is local and anonymous when no token is asked for
            ASSERT_TRUE( fixture.tally->channelsOpened == 1 );
            ASSERT_TRUE( 1 == fixture.transport->GetConnectionCount() );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"subject\":\"local:tcp-anonymous\"" ) >= 0 && fixture.tally->auditText.HasSubstr( "\"transport\":\"tcp\"" ) >= 0 );
            ASSERT_TRUE( 0 == client.Lines().GetUnparsableCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Every connection has its own channel and its own legacy session" )
        try
        {
            CTcpFixture fixture( false );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient first;
            CMcpRawTcpClient second;
            ASSERT_TRUE( first.Connect( fixture.Port() ) && second.Connect( fixture.Port() ) );

            CORE::CDataNode response;
            ASSERT_TRUE( first.SendLine( McpTestLegacyRequest( "1", "initialize", "\"protocolVersion\":\"2025-11-25\",\"capabilities\":{},\"clientInfo\":{\"name\":\"old\",\"version\":\"1\"}" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( first, "1", response ) && McpTestResultAttr( response, "protocolVersion" ) == "2025-11-25" );

            // the first connection has a session and may ping, the second has none
            ASSERT_TRUE( first.SendLine( McpTestLegacyRequest( "2", "ping" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( first, "2", response ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( second.SendLine( McpTestLegacyRequest( "2", "ping" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( second, "2", response ) && GUCEF_NULL == response.FindChild( "result" ) );
            // the same id on two connections is no conflict
            ASSERT_TRUE( second.SendLine( McpTestRequest( "3", "tools/call", EchoParams( "second" ) ) ) );
            ASSERT_TRUE( first.SendLine( McpTestRequest( "3", "tools/call", EchoParams( "first" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( first, "3", response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: first" );
            ASSERT_TRUE( fixture.WaitForResponse( second, "3", response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: second" );
            ASSERT_TRUE( 2 == fixture.transport->GetConnectionCount() && 2 == fixture.tally->channelsOpened );

            // closing one is seen, and the other goes on
            first.Close();
            ASSERT_TRUE( fixture.WaitForConnections( 1 ) );
            ASSERT_TRUE( 1 == fixture.tally->channelsClosed );
            ASSERT_TRUE( second.SendLine( McpTestRequest( "4", "tools/call", EchoParams( "alone" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( second, "4", response ) );

            // connection objects are reused: five more come and go
            for ( UInt32 i=0; i<5; ++i )
            {
                CMcpRawTcpClient again;
                ASSERT_TRUE( again.Connect( fixture.Port() ) );
                CORE::CString id = CORE::UInt32ToString( 10 + i );
                ASSERT_TRUE( again.SendLine( McpTestRequest( id, "tools/call", EchoParams( "again" ) ) ) );
                ASSERT_TRUE( fixture.WaitForResponse( again, id, response ) && McpTestFirstText( response.FindChild( "result" ) ) == "echo: again" );
                again.Close();
                ASSERT_TRUE( fixture.WaitForConnections( 1 ) );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An HTTP request line closes the connection, and the JSON body behind it is never processed" )
        try
        {
            CTcpFixture fixture( false );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;

            // what a browser can make a page send to any local port, with a JSON body that would be a valid request
            const CORE::CString body = McpTestRequest( "1", "tools/call", EchoParams( "attack" ) );
            CORE::CString http = "POST /mcp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: text/plain\r\nContent-Length: " + CORE::UInt32ToString( body.Length() ) + "\r\n\r\n" + body + "\n";
            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            ASSERT_TRUE( client.Send( http ) );
            ASSERT_TRUE( fixture.WaitForClose( client ) );
            ASSERT_TRUE( 0 == client.Lines().GetLineCount() );
            ASSERT_TRUE( 0 == fixture.tally->started && 0 == fixture.tally->audited );
            ASSERT_TRUE( fixture.WaitForConnections( 0 ) );

            // the same, one line at a time
            CMcpRawTcpClient slow;
            ASSERT_TRUE( slow.Connect( fixture.Port() ) );
            ASSERT_TRUE( slow.Send( "GET / HTTP/1.1\r\n" ) );
            ASSERT_TRUE( fixture.WaitForClose( slow ) );
            ASSERT_TRUE( 0 == fixture.tally->started );

            // a good line first, then a line that is no JSON object: answered, then closed, and what follows it is never read
            CMcpRawTcpClient later;
            ASSERT_TRUE( later.Connect( fixture.Port() ) );
            CORE::CDataNode response;
            ASSERT_TRUE( later.SendLine( McpTestRequest( "2", "tools/call", EchoParams( "fine" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( later, "2", response ) );
            ASSERT_TRUE( 1 == fixture.tally->started );
            ASSERT_TRUE( later.Send( "Host: example.com\r\n" + McpTestRequest( "3", "tools/call", EchoParams( "behind" ) ) + "\n" + McpTestRequest( "4", "tools/call", EchoParams( "behind" ) ) + "\n" ) );
            ASSERT_TRUE( fixture.WaitForClose( later ) );
            ASSERT_TRUE( 1 == fixture.tally->started );
            ASSERT_TRUE( fixture.WaitForConnections( 0 ) );

            // a number, a string, an array: not JSON objects either; a line that is broken JSON but starts like an object is a parse error and the connection stays
            CMcpRawTcpClient odd;
            ASSERT_TRUE( odd.Connect( fixture.Port() ) );
            ASSERT_TRUE( odd.SendLine( "{broken" ) );
            CORE::CString line;
            ASSERT_TRUE( fixture.WaitForLine( odd, line ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32700 == McpTestErrorCode( response ) );
            ASSERT_TRUE( odd.SendLine( "[1,2,3]" ) );
            ASSERT_TRUE( fixture.WaitForClose( odd ) );
            ASSERT_TRUE( fixture.transport->IsActive() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The token is required per request, refused with the access denied error, and a connection that keeps trying is closed" )
        try
        {
            const char* token = "correct-horse-battery-staple";
            CTcpFixture fixture( true, token );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;

            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            CORE::CDataNode response;

            // good
            ASSERT_TRUE( client.SendLine( TokenRequest( "1", "tools/call", EchoParams( "welcome" ), token ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "1", response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: welcome" );

            // no token, a wrong one, one of another length, one that is no string: every one gets -31003
            ASSERT_TRUE( client.SendLine( McpTestRequest( "2", "tools/call", EchoParams( "no token" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "2", response ) && -31003 == McpTestErrorCode( response ) );
            ASSERT_TRUE( client.SendLine( TokenRequest( "3", "tools/call", EchoParams( "wrong" ), "correct-horse-battery-stapl3" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "3", response ) && -31003 == McpTestErrorCode( response ) );
            ASSERT_TRUE( McpTestErrorMessage( response ).HasSubstr( token ) < 0 );

            // the token is checked for every request, also after a good one, and the work of a refused request never runs
            ASSERT_TRUE( client.SendLine( TokenRequest( "4", "tools/call", EchoParams( "again fine" ), token ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "4", response ) && GUCEF_NULL != response.FindChild( "result" ) );

            // the third refusal on the connection closes it
            ASSERT_TRUE( client.SendLine( TokenRequest( "5", "server/discover", "", "" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "5", response ) && -31003 == McpTestErrorCode( response ) );
            ASSERT_TRUE( fixture.WaitForClose( client ) );

            // the token is nowhere in what was audited
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( token ) < 0 );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"subject\":\"local:tcp-token\"" ) >= 0 );

            // a new connection starts afresh
            CMcpRawTcpClient next;
            ASSERT_TRUE( next.Connect( fixture.Port() ) );
            ASSERT_TRUE( next.SendLine( TokenRequest( "6", "tools/call", EchoParams( "fresh" ), token ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( next, "6", response ) && GUCEF_NULL != response.FindChild( "result" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A token file that is not there is made, for the owner only, and read again by the next transport" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpTcpToken" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::CreateDirs( dir );
            const CORE::CString tokenFile = CORE::CombinePath( dir, "mcp.token" );
            CORE::DeleteFile( tokenFile );

            CORE::CString made;
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( tokenFile );
                ASSERT_TRUE( transport->Start( *rig.server ) );
                made = transport->GetToken();
                ASSERT_TRUE( made.Length() >= 40 );
                ASSERT_TRUE( CORE::FileExists( tokenFile ) );
                transport->Stop( false );

                CORE::CDynamicBuffer content;
                ASSERT_TRUE( content.LoadContentFromFile( tokenFile ) );
                ASSERT_TRUE( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) == made );
                #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
                struct stat info;
                ASSERT_TRUE( 0 == ::stat( tokenFile.C_String(), &info ) );
                ASSERT_TRUE( 0 == ( info.st_mode & ( S_IRWXG | S_IRWXO ) ) );
                #endif
            }
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( tokenFile );
                ASSERT_TRUE( transport->Start( *rig.server ) );
                ASSERT_TRUE( transport->GetToken() == made );
                transport->Stop( false );
            }
            {
                // a token is asked for and there is nowhere to get one
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                ASSERT_FALSE( transport->Start( *rig.server ) );
            }
            CORE::DeleteFile( tokenFile );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only loopback addresses are bound unless it is allowed" )
        try
        {
            ASSERT_TRUE( COM::CMcpTcpTransport::IsLoopbackAddress( "127.0.0.1" ) );
            ASSERT_TRUE( COM::CMcpTcpTransport::IsLoopbackAddress( "127.1.2.3" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "0.0.0.0" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "128.0.0.1" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "192.168.1.1" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "127.0.0" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "127.0.0.256" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "localhost" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "" ) );
            ASSERT_FALSE( COM::CMcpTcpTransport::IsLoopbackAddress( "127.0.0.1.5" ) );

            CMcpTestRig rig;
            CServerTally tally( *rig.server );
            {
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetRequireToken( false );
                transport->SetBindAddress( "0.0.0.0" );
                ASSERT_FALSE( transport->Start( *rig.server ) );
                ASSERT_FALSE( transport->IsActive() );
                transport->SetBindAddress( "not an address" );
                transport->SetAllowNonLoopback( true );
                ASSERT_FALSE( transport->Start( *rig.server ) );
                transport->SetBindAddress( "0.0.0.0" );
                ASSERT_TRUE( transport->Start( *rig.server ) );
                ASSERT_TRUE( transport->GetBoundPort() > 0 );
                transport->Stop( false );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A line over the limit is skipped and answered without an id, the connection goes on" )
        try
        {
            CTcpFixture fixture( false, GUCEF_NULL, 1500 );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );

            // 40 KB of a JSON object, with the line end only at the very end and sent in pieces
            CORE::CString filler;
            for ( UInt32 i=0; i<4000; ++i )
                filler += "0123456789";
            CORE::CString huge = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"pad\":\"" + filler + "\"}}";
            const UInt32 piece = huge.Length() / 4;
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( client.Send( CORE::CString( huge.C_String() + i * piece, i < 3 ? piece : huge.Length() - 3 * piece ) ) );
                fixture.Pump( client, 5 );
            }
            ASSERT_TRUE( client.Send( "\n" + McpTestRequest( "2", "tools/call", EchoParams( "next" ) ) + "\n" ) );
            CORE::CString line;
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.WaitForLine( client, line ) );
            ASSERT_TRUE( McpTestParse( line, response ) && -32600 == McpTestErrorCode( response ) && line.HasSubstr( "\"id\":null" ) >= 0 );
            ASSERT_TRUE( fixture.WaitForResponse( client, "2", response ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: next" );
            ASSERT_TRUE( 1 == fixture.tally->started );                  // only the second request reached the dispatch
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Stopping the transport closes the connections, and it can start again" )
        try
        {
            CTcpFixture fixture( false );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            CORE::CDataNode response;
            ASSERT_TRUE( client.SendLine( McpTestRequest( "1", "tools/call", EchoParams( "x" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "1", response ) );

            fixture.rig.server->Stop( false );
            ASSERT_FALSE( fixture.transport->IsActive() );
            ASSERT_TRUE( fixture.WaitForClose( client ) );
            ASSERT_TRUE( 0 == fixture.transport->GetBoundPort() );
            CMcpRawTcpClient refused;
            ASSERT_FALSE( refused.Connect( 1 ) );                     // nothing listens on port 1

            ASSERT_TRUE( fixture.rig.server->Start() );
            ASSERT_TRUE( fixture.transport->IsActive() && fixture.Port() > 0 );
            CMcpRawTcpClient again;
            ASSERT_TRUE( again.Connect( fixture.Port() ) );
            ASSERT_TRUE( again.SendLine( McpTestRequest( "2", "tools/call", EchoParams( "restarted" ) ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( again, "2", response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Text of two, three and four bytes per character comes back whole, byte for byte, in both directions" )
        try
        {
            CTcpFixture fixture( false );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            const char* texts[] = { "\xc3\xa9", "\xe2\x82\xac", "\xf0\x9f\x98\x80", "caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80 mixed\xc3\xa9\xf0\x9f\x98\x80" };
            for ( UInt32 i=0; i<4; ++i )
            {
                const CORE::CString id = CORE::ToString( i + 1 );
                ASSERT_TRUE( client.SendLine( McpTestRequest( id, "tools/call", EchoParams( texts[ i ] ) ) ) );
                CORE::CDataNode response;
                ASSERT_TRUE( fixture.WaitForResponse( client, id, response ) );
                ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == CORE::CString( "echo: " ) + texts[ i ] );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Eight connections that send nothing do not keep a ninth client from being served" )
        try
        {
            const char* token = "correct-horse-battery-staple";
            CTcpFixture fixture( true, token );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;

            CMcpRawTcpClient silent[ 8 ];
            for ( UInt32 i=0; i<8; ++i )
                ASSERT_TRUE( silent[ i ].Connect( fixture.Port() ) );
            for ( UInt32 i=0; i<50; ++i )
                fixture.rig.Pulse();

            // well within the time a silent connection may take for its first request
            CMcpRawTcpClient legitimate;
            ASSERT_TRUE( legitimate.Connect( fixture.Port() ) );
            ASSERT_TRUE( legitimate.SendLine( TokenRequest( "1", "tools/call", EchoParams( "served" ), token ) ) );
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.WaitForResponse( legitimate, "1", response, 3000 ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: served" );
            ASSERT_TRUE( fixture.transport->GetConnectionCount() <= 8 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With every place taken by served clients a new connection is held, and served once a place is free" )
        try
        {
            const char* token = "correct-horse-battery-staple";
            CTcpFixture fixture( true, token );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient served[ 8 ];
            CORE::CDataNode response;
            for ( UInt32 i=0; i<8; ++i )
            {
                const CORE::CString id = CORE::ToString( i + 1 );
                ASSERT_TRUE( served[ i ].Connect( fixture.Port() ) );
                ASSERT_TRUE( served[ i ].SendLine( TokenRequest( id, "tools/call", EchoParams( "in" ), token ) ) );
                ASSERT_TRUE( fixture.WaitForResponse( served[ i ], id, response ) );
            }

            // the ninth is not served while the eight are there, and not closed either
            CMcpRawTcpClient waiting;
            ASSERT_TRUE( waiting.Connect( fixture.Port() ) );
            ASSERT_TRUE( waiting.SendLine( TokenRequest( "100", "tools/call", EchoParams( "later" ), token ) ) );
            ASSERT_FALSE( fixture.WaitForResponse( waiting, "100", response, 300 ) );
            ASSERT_FALSE( waiting.IsClosedByPeer() );

            // one goes: the held one gets its place and its answer
            served[ 0 ].Close();
            ASSERT_TRUE( fixture.WaitForResponse( waiting, "100", response, 3000 ) );
            ASSERT_TRUE( McpTestFirstText( response.FindChild( "result" ) ) == "echo: later" );
            ASSERT_TRUE( fixture.transport->GetConnectionCount() <= 8 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request refused for its token is audited once, as not authenticated" )
        try
        {
            const char* token = "correct-horse-battery-staple";
            CTcpFixture fixture( true, token );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            CMcpRawTcpClient client;
            ASSERT_TRUE( client.Connect( fixture.Port() ) );
            CORE::CDataNode response;

            ASSERT_TRUE( client.SendLine( TokenRequest( "1", "tools/call", EchoParams( "bad" ), "guessed-token" ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "1", response ) && -31003 == McpTestErrorCode( response ) );
            ASSERT_TRUE( 1 == fixture.tally->audited );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "McpTcp test: the audit of a refused token: " << fixture.tally->auditText );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"authenticated\":false" ) >= 0 );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"authenticated\":true" ) < 0 );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "guessed-token" ) < 0 );

            // with the token the request is the authenticated principal's
            fixture.tally->auditText.Clear();
            ASSERT_TRUE( client.SendLine( TokenRequest( "2", "tools/call", EchoParams( "good" ), token ) ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "2", response ) && GUCEF_NULL != response.FindChild( "result" ) );
            ASSERT_TRUE( 2 == fixture.tally->audited );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"authenticated\":true" ) >= 0 );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"subject\":\"local:tcp-token\"" ) >= 0 );

            // a malformed _meta is refused before the token is looked at: not authenticated either
            fixture.tally->auditText.Clear();
            ASSERT_TRUE( client.SendLine( "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\",\"params\":{\"_meta\":7}}" ) );
            ASSERT_TRUE( fixture.WaitForResponse( client, "3", response ) && -32602 == McpTestErrorCode( response ) );
            ASSERT_TRUE( fixture.tally->auditText.HasSubstr( "\"authenticated\":false" ) >= 0 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    GUCEF_TESTFW_TESTCASE( "Test: A token file that is a link, or that others can read, is refused" )
        try
        {
            TESTFW::CScopedTestDir scopedDir( "gucefCOM_TestApp_McpTcpTokenSafety" );
            CORE::CString dir = scopedDir.GetPath();
            CORE::Remove_Directory( dir.C_String(), 1 );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            const CORE::CString planted = CORE::CombinePath( dir, "planted.token" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( planted, "a-token-someone-else-knows" ) );
            ::chmod( planted.C_String(), 0600 );

            // a link planted where the token file is to be: never followed, neither to read nor to write
            const CORE::CString linked = CORE::CombinePath( dir, "linked.token" );
            ASSERT_TRUE( 0 == ::symlink( planted.C_String(), linked.C_String() ) );
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( linked );
                ASSERT_FALSE( transport->Start( *rig.server ) );
                transport->Stop( false );
            }
            const CORE::CString dangling = CORE::CombinePath( dir, "dangling.token" );
            const CORE::CString nowhere = CORE::CombinePath( dir, "nowhere.token" );
            ASSERT_TRUE( 0 == ::symlink( nowhere.C_String(), dangling.C_String() ) );
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( dangling );
                ASSERT_FALSE( transport->Start( *rig.server ) );
                transport->Stop( false );
            }
            ASSERT_FALSE( CORE::FileExists( nowhere ) );

            // a token file the group or others can read is no secret any more
            ::chmod( planted.C_String(), 0644 );
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( planted );
                ASSERT_FALSE( transport->Start( *rig.server ) );
                transport->Stop( false );
            }

            // a new token file is made 0600 even with umask 0
            const CORE::CString fresh = CORE::CombinePath( dir, "fresh.token" );
            const mode_t oldMask = ::umask( 0 );
            bool started = false;
            {
                CMcpTestRig rig;
                COM::CIMcpTransportPtr keep = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "tcp" );
                COM::CMcpTcpTransport* transport = static_cast< COM::CMcpTcpTransport* >( keep.GetPointerAlways() );
                transport->SetTokenFile( fresh );
                started = transport->Start( *rig.server );
                transport->Stop( false );
            }
            ::umask( oldMask );
            ASSERT_TRUE( started );
            struct stat info;
            ASSERT_TRUE( 0 == ::lstat( fresh.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );
            CORE::Remove_Directory( dir.C_String(), 1 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Test: With a certificate and key the transport speaks TLS, the lines and the token inside it; a plain client gets nothing" )
        try
        {
            CORE::CString whyNot;
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
            GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNot ), "this system has no TLS provider" );
            TESTFW::CTlsTestCerts certs;
            GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );

            COMCORE::CTlsServerSettings tls;
            tls.SetCertificateChainPath( certs.GetPath( "good.crt" ) );
            tls.SetPrivateKeyPath( certs.GetPath( "good.key" ) );
            CTcpFixture fixture( true, "tls-token", 0, &tls );
            ASSERT_TRUE( fixture.started );
            if ( !fixture.started )
                return;
            ASSERT_TRUE( fixture.transport->GetEndpointDescription() == "tls://127.0.0.1:" + CORE::UInt32ToString( fixture.Port() ) );

            COMCORE::CTlsSettings trust;
            trust.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
            trust.SetUseSystemTrust( false );
            CMcpTlsLineClient client( fixture.rig.pulseGenerator );
            ASSERT_TRUE( client.connection.ConnectTo( "localhost", fixture.Port(), &trust ) );
            const CORE::CString line = TokenRequest( "1", "tools/call", EchoParams( "inside tls" ), "tls-token" ) + "\n";
            for ( UInt32 waited=0; waited<5000 && !client.connected && !client.disconnected; ++waited )
            {
                fixture.rig.Pulse();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( client.connected );
            ASSERT_TRUE( client.connection.Send( line.C_String(), line.Length() ) );
            ASSERT_TRUE( PumpUntilReceived( fixture, client, "echo: inside tls" ) );

            // a client without TLS is closed and none of its lines is served
            CMcpRawTcpClient plain;
            ASSERT_TRUE( plain.Connect( fixture.Port() ) );
            ASSERT_TRUE( plain.SendLine( TokenRequest( "2", "tools/call", EchoParams( "plain" ), "tls-token" ) ) );
            ASSERT_TRUE( fixture.WaitForClose( plain ) );
            CORE::CString got;
            ASSERT_FALSE( plain.Lines().PopLine( got ) );
        }
        catch( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: TLS allows another than a loopback address, a certificate that cannot be used stops the start, the keys come from the configuration" )
        try
        {
            CORE::CString whyNot;
            GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
            GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNot ), "this system has no TLS provider" );
            TESTFW::CTlsTestCerts certs;
            GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );

            CORE::CDataNode cfg;
            cfg.SetAttribute( "bindAddress", "0.0.0.0" );
            cfg.SetAttribute( "requireToken", false );
            cfg.SetAttribute( "certificateChainPath", certs.GetPath( "good.crt" ) );
            cfg.SetAttribute( "privateKeyPath", certs.GetPath( "good.key" ) );
            {
                CMcpTestRig rig;
                COM::CMcpTcpTransport transport;
                ASSERT_TRUE( transport.LoadConfig( cfg ) );
                ASSERT_TRUE( transport.GetTlsSettings().IsEnabled() );
                ASSERT_TRUE( transport.Start( *rig.server ) );
                transport.Stop( false );
            }
            {
                cfg.SetAttribute( "privateKeyPath", certs.GetPath( "wrongname.key" ) );
                CMcpTestRig rig;
                COM::CMcpTcpTransport transport;
                ASSERT_TRUE( transport.LoadConfig( cfg ) );
                ASSERT_FALSE( transport.Start( *rig.server ) );
                ASSERT_FALSE( transport.IsActive() );
            }
        }
        catch( const TESTFW::STestSkipRequest& )
        {
            throw;
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL MCP TCP TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
