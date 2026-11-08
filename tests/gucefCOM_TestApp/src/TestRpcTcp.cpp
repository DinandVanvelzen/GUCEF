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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H
#include "gucefCOM_CRpcTcpTransportListener.h"
#define GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H
#endif /* GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H ? */

#ifndef GUCEF_COM_CMCPRPCTRANSPORT_H
#include "gucefCOM_CMcpRpcTransport.h"
#define GUCEF_COM_CMCPRPCTRANSPORT_H
#endif /* GUCEF_COM_CMCPRPCTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPRPCRELAY_H
#include "gucefCOM_CMcpRpcRelay.h"
#define GUCEF_COM_CMCPRPCRELAY_H
#endif /* GUCEF_COM_CMCPRPCRELAY_H ? */

#define GUCEF_RPCTEST_ALLOCATION_COUNTER_IMPLEMENTATION
#include "TestRpcAllocationCounter.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#include "TestRpcEndpointFixture.h"

#include "TestRpcTransportSuite.h"

#include "TestRpcConformance.h"

#include "TestMcpSupport.h"

#include "TestRpcTcp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define ASSERT_OK( s )      { COMCORE::CRpcStatus assertStatus_( s ); if ( !assertStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcTcp test: line " << __LINE__ << ": " << assertStatus_.ToString() ); } }
#define REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcTcp test: requirement failed: " #t ); return; } }
#define ASSERT_CODE( c, s ) { COMCORE::CRpcStatus assertStatus_( s ); if ( c != assertStatus_.code ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcTcp test: line " << __LINE__ << ": expected " << COMCORE::RpcStatusCodeToString( c ) << ", got " << assertStatus_.ToString() ); } }

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

namespace {

const char* g_getPath = "gucef.sample.notes.Notes/Get";
const char* g_watchPath = "gucef.sample.notes.Notes/Watch";

/** A host on a tcp listener of the test's pulse generator, serving the notes service of the fixture */
class CTcpHostFixture
{
    public:

    CRpcEndpointFixture& f;
    COMCORE::CRpcContractPtr notes;
    COMCORE::CRpcHost* host;
    COMCORE::CIRpcTransportListenerPtr listener;
    CORE::CUri listenUri;
    COMCORE::CTlsServerSettings listenerTls;        /**< TLS of the listener when it has a certificate */
    COMCORE::CTlsSettings clientTls;
    bool clientUsesTls;

    explicit CTcpHostFixture( CRpcEndpointFixture& fixture )
        : f( fixture ), notes( LoadSampleNotesContract( 1, 2 ) ), host( GUCEF_NULL ), listener(), listenUri(), listenerTls(), clientTls(), clientUsesTls( false )
    {}

    ~CTcpHostFixture()
    {
        if ( !f.client.IsNULL() )
            f.client->Close( false );
        RpcTestPump( 20 );
        if ( GUCEF_NULL != host )
        {
            host->Stop();
            RpcTestPump( 10 );
        }
        if ( !listener.IsNULL() )
            listener->StopListening();
        GUCEF_DELETE host;
        host = GUCEF_NULL;
        listener.Unlink();
    }

    bool Start( void )
    {
        COMCORE::CRpcHostSettings settings;
        settings.endpointSettings = f.serverSettings;
        host = GUCEF_NEW COMCORE::CRpcHost( f.pulseGenerator, settings );
        if ( !host->GetServiceSet()->RegisterService( notes, "Notes", &f.service ) || !host->Start() )
            return false;
        listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "tcp" );
        if ( listener.IsNULL() )
            return false;
        static_cast< COM::CRpcTcpTransportListener* >( listener.GetPointerAlways() )->SetTlsSettings( listenerTls );
        if ( !listener->SetPulseGenerator( f.pulseGenerator ) || !listener->Listen( CORE::CUri( "tcp://127.0.0.1:0" ) ) )
            return false;
        listenUri = listener->GetListenUri();
        f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        return host->AddListener( listener );
    }

    /** A tcp transport to the listener on the test's pulse generator, wrapped by the capturing decorator */
    COMCORE::CIRpcTransportPtr NewClientTransport( CRpcLossyTransport** wire )
    {
        COMCORE::CIRpcTransportPtr transport = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( "tcp" );
        if ( transport.IsNULL() )
            return COMCORE::CIRpcTransportPtr();
        if ( clientUsesTls )
            static_cast< COM::CRpcTcpTransport* >( transport.GetPointerAlways() )->SetTlsSettings( clientTls );
        if ( !transport->SetPulseGenerator( f.pulseGenerator ) || !transport->Connect( listenUri ) )
            return COMCORE::CIRpcTransportPtr();
        return CRpcLossyTransport::Wrap( transport, wire );
    }

    bool ConnectClient( void )
    {
        f.clientTransport = NewClientTransport( &f.clientWire );
        if ( f.clientTransport.IsNULL() )
            return false;
        f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
        f.client->AddWantedContract( "gucef.sample.notes", 1 );
        if ( !f.client->Start( f.clientTransport, true ) )
            return false;
        return f.PumpUntilBound( "gucef.sample.notes", 1, 5000 );
    }
};

/** A raw socket that records whether the peer closed it */
class CRawPeer : public CORE::CObserver
{
    public:

    COMCORE::CTCPClientSocket socket;
    bool connected;
    bool disconnected;

    explicit CRawPeer( const CORE::PulseGeneratorPtr& pulseGenerator )
        : CORE::CObserver(), socket( pulseGenerator, false ), connected( false ), disconnected( false )
    {
        typedef CORE::CTEventHandlerFunctor< CRawPeer > TEventCallback;
        TEventCallback onConnected( this, &CRawPeer::OnConnected );
        TEventCallback onDisconnected( this, &CRawPeer::OnDisconnected );
        SubscribeTo( &socket, COMCORE::CTCPConnection::ConnectedEvent, onConnected );
        SubscribeTo( &socket, COMCORE::CTCPConnection::DisconnectedEvent, onDisconnected );
    }

    virtual ~CRawPeer() { UnsubscribeAllFromObserver(); socket.Close(); }

    private:

    void OnConnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { connected = true; }
    void OnDisconnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { disconnected = true; }
};

bool
PumpUntilFlag( const CORE::PulseGeneratorPtr& pg, const bool& flag, UInt32 timeoutMs )
{
    CORE::PulseGeneratorPtr pulseGenerator( pg );
    UInt64 start = MT::PrecisionTickCount();
    while ( !flag )
    {
        pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution() > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

COMCORE::CRpcMessage
GetRequest( CRpcEndpointFixture& f, UInt64 noteId )
{
    COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
    request.SetUInt64( 1, noteId );
    return request;
}

}

/*-------------------------------------------------------------------------*/

static void
TestCallsThroughHost( void )
{
    CRpcEndpointFixture f;
    CTcpHostFixture h( f );
    REQUIRE_TRUE( h.Start() );
    ASSERT_TRUE( h.listenUri.GetAuthorityPort() != "0" );
    REQUIRE_TRUE( h.ConnectClient() );
    ASSERT_TRUE( 1 == h.host->GetEndpointCount() );

    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ), COMCORE::CRpcCallOptions(), 5000 );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "note 5" == result.response.GetString( 2 ) );
    ASSERT_TRUE( "tcp" == f.client->GetPeerPrincipal().transport );
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    h.host->GetEndpoints( endpoints );
    REQUIRE_TRUE( 1 == endpoints.size() );
    ASSERT_TRUE( 0 == endpoints[ 0 ]->GetPeerPrincipal().subject.HasSubstr( "tcp:127.0.0.1:" ) );
    ASSERT_FALSE( endpoints[ 0 ]->GetPeerPrincipal().isAuthenticated );

    // a server stream past the initial credit
    f.service.watchCount = 150;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
    REQUIRE_TRUE( !stream.IsNULL() );
    COMCORE::CRpcMessage event;
    UInt32 received = 0;
    while ( stream->ReadNext( event, status, 5000 ) )
        ++received;
    ASSERT_OK( status );
    ASSERT_TRUE( 150 == received );

    // the first bytes on the wire are the preface, then length prefixed frames
    ASSERT_TRUE( f.clientWire->CountSent( GUCEF_RPC_FRAME_HELLO ) == 1 );

    // reflection over tcp
    REQUIRE_TRUE( f.client->ResolveMethod( "gucef.rpc.Reflection/ListContracts", ref, status ) );
    COMCORE::CRpcMessage list = f.client->NewRequest( ref );
    COMCORE::CRpcMessage response;
    ASSERT_TRUE( f.client->Call( ref, list, response, status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( response.GetCount( 1 ) >= 2 );
}

/*-------------------------------------------------------------------------*/

static void
TestPrefaceAndFrameLimits( void )
{
    CRpcEndpointFixture f;
    CTcpHostFixture h( f );
    REQUIRE_TRUE( h.Start() );
    UInt16 port = (UInt16) CORE::StringToUInt32( h.listenUri.GetAuthorityPort() );
    {
        // an HTTP request is not this protocol: disconnected before anything else is read
        CRawPeer peer( f.pulseGenerator );
        REQUIRE_TRUE( peer.socket.ConnectTo( "127.0.0.1", port, false ) );
        REQUIRE_TRUE( PumpUntilFlag( f.pulseGenerator, peer.connected, 3000 ) );
        const char* http = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
        ASSERT_TRUE( peer.socket.Send( http, (UInt32) strlen( http ) ) );
        ASSERT_TRUE( PumpUntilFlag( f.pulseGenerator, peer.disconnected, 3000 ) );
        RpcTestPump( 10 );
        ASSERT_TRUE( 0 == h.host->GetEndpointCount() );
    }
    {
        // the preface, then a frame larger than the first frame may be
        CRawPeer peer( f.pulseGenerator );
        REQUIRE_TRUE( peer.socket.ConnectTo( "127.0.0.1", port, false ) );
        REQUIRE_TRUE( PumpUntilFlag( f.pulseGenerator, peer.connected, 3000 ) );
        CORE::CDynamicBuffer bytes;
        bytes.Append( COM::CRpcTcpTransport::Preface, sizeof( COM::CRpcTcpTransport::Preface ) );
        CORE::CBinaryWireWriter writer( bytes, bytes.GetDataSize() );
        writer.WriteVarUInt32( 10 * 1024 * 1024 );
        ASSERT_TRUE( peer.socket.Send( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) );
        ASSERT_TRUE( PumpUntilFlag( f.pulseGenerator, peer.disconnected, 3000 ) );
        ASSERT_TRUE( 0 == h.host->GetEndpointCount() );
    }

    // loopback only, unless allowed
    COM::CRpcTcpTransportListener other;
    ASSERT_FALSE( other.Listen( CORE::CUri( "tcp://10.1.2.3:0" ) ) );
    COM::CRpcTcpTransport transport;
    ASSERT_FALSE( transport.Connect( CORE::CUri( "tcp://10.1.2.3:4000" ) ) );
}

/*-------------------------------------------------------------------------*/

static void
TestResumeFromLoopbackOntoTcp( void )
{
    CRpcEndpointFixture f;
    CTcpHostFixture h( f );
    REQUIRE_TRUE( h.Start() );

    // the session starts over loopback, accepted by the same host, with an unauthenticated principal as tcp has
    // one: the resume proof is what identifies the peer then
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    f.serverWire->hasPrincipalOverride = true;
    f.serverWire->principalOverride = COMCORE::CRpcPrincipal( "loopback", "local:unverified", false );
    REQUIRE_TRUE( h.host->AcceptTransport( f.serverTransport ) );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 3000 ) );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 1 ) );
    ASSERT_OK( result.status );
    UInt8 before[ 16 ];
    UInt8 after[ 16 ];
    REQUIRE_TRUE( f.client->GetSessionId( before ) );
    UInt64 nextBefore = f.client->GetNextSendSeq();

    // the loopback transport goes; the session resumes over tcp
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 3000 ) );
    CRpcLossyTransport* tcpWire = GUCEF_NULL;
    COMCORE::CIRpcTransportPtr tcp = h.NewClientTransport( &tcpWire );
    REQUIRE_TRUE( !tcp.IsNULL() );
    REQUIRE_TRUE( f.client->Rebind( tcp ) );
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 5000 ) );
    ASSERT_TRUE( f.client->GetSessionId( after ) );
    ASSERT_TRUE( 0 == memcmp( before, after, 16 ) );
    ASSERT_TRUE( f.client->GetNextSendSeq() >= nextBefore );
    result = f.CallAndPump( g_getPath, GetRequest( f, 2 ), COMCORE::CRpcCallOptions(), 5000 );
    ASSERT_OK( result.status );
    ASSERT_TRUE( 2 == result.response.GetUInt64( 1 ) );
    ASSERT_TRUE( "tcp" == f.client->GetTransport()->GetTransportType() );
    ASSERT_TRUE( 1 == h.host->GetEndpointCount() );
    f.clientTransport = tcp;
}

/*-------------------------------------------------------------------------*/

static void
TestNoStepDownOntoTcp( void )
{
    // a session of an authenticated principal (loopback) does not resume over unauthenticated tcp
    CRpcEndpointFixture f;
    CTcpHostFixture h( f );
    REQUIRE_TRUE( h.Start() );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    REQUIRE_TRUE( h.host->AcceptTransport( f.serverTransport ) );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 3000 ) );
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 3000 ) );
    CRpcLossyTransport* tcpWire = GUCEF_NULL;
    COMCORE::CIRpcTransportPtr tcp = h.NewClientTransport( &tcpWire );
    REQUIRE_TRUE( !tcp.IsNULL() );
    REQUIRE_TRUE( f.client->Rebind( tcp ) );
    ASSERT_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 5000 ) );
    f.clientTransport = tcp;
}

/*-------------------------------------------------------------------------*/

static void
TestMcpOverRpc( void )
{
    if ( !McpTestJsonAvailable() )
        GUCEF_TESTFW_SKIP( "no JSON codec plugin" );

    // the MCP server and both RPC endpoints on the pulse generator of the MCP test rig, which this thread drives
    CMcpTestRig rig;
    CORE::CString error;
    rig.server->GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error );
    COMCORE::CRpcServiceSetPtr services = COMCORE::CRpcServiceSet::Create();
    COM::CIMcpTransportPtr transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "rpc" );
    REQUIRE_TRUE( !transportPtr.IsNULL() );
    COM::CMcpRpcTransport* transport = static_cast< COM::CMcpRpcTransport* >( transportPtr.GetPointerAlways() );
    transport->SetServiceSet( services );
    rig.server->AddTransport( transportPtr );
    REQUIRE_TRUE( rig.server->Start() );
    REQUIRE_TRUE( transport->IsActive() );

    COMCORE::CRpcContractRegistry clientRegistry;
    COMCORE::CRpcContractRegistry serverRegistry;
    clientRegistry.SetCacheDirectory( CORE::CString::Empty );
    serverRegistry.SetCacheDirectory( CORE::CString::Empty );
    COMCORE::CRpcEndpointSettings clientSettings;
    COMCORE::CRpcEndpointSettings serverSettings;
    clientSettings.contractRegistry = &clientRegistry;
    serverSettings.contractRegistry = &serverRegistry;
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( rig.pulseGenerator, rig.pulseGenerator, a, b ) );
    COMCORE::CRpcEndpointPtr server = COMCORE::CRpcEndpoint::Create( rig.pulseGenerator, serverSettings );
    server->SetServiceSet( services );
    COMCORE::CRpcEndpointPtr client = COMCORE::CRpcEndpoint::Create( rig.pulseGenerator, clientSettings );
    client->AddWantedContract( "gucef.mcp", 1 );
    REQUIRE_TRUE( server->Start( b, false ) );
    REQUIRE_TRUE( client->Start( a, true ) );
    for ( UInt32 i=0; i<2000 && !client->IsContractBound( "gucef.mcp", 1 ); ++i )
    {
        rig.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    REQUIRE_TRUE( client->IsContractBound( "gucef.mcp", 1 ) );

    // tools/list through the relay: one JSON-RPC message per stream message
    COM::CMcpRpcRelay relay;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( relay.Open( client, status ) );
    ASSERT_TRUE( relay.SendMessage( McpTestRequest( "1", "tools/list" ) ) );
    CORE::CString answer;
    ASSERT_TRUE( relay.ReadMessage( answer, status, 5000 ) );
    ASSERT_OK( status );
    CORE::CDataNode response;
    ASSERT_TRUE( McpTestParse( answer, response ) );
    ASSERT_TRUE( answer.HasSubstr( "\"echo\"" ) >= 0 );
    ASSERT_TRUE( 1 == transport->GetChannelCount() );

    // stdio framing through the relay: two lines, two answers
    CORE::CString lines = McpTestRequest( "2", "tools/list" ) + "\n" + McpTestRequest( "3", "tools/list" ) + "\r\n";
    ASSERT_TRUE( (UInt32) ( lines.ByteSize() - 1 ) == relay.RelayInput( lines.C_String(), lines.ByteSize() - 1 ) );
    ASSERT_TRUE( relay.ReadMessage( answer, status, 5000 ) );
    ASSERT_TRUE( answer.HasSubstr( "\"id\":2" ) >= 0 );
    ASSERT_TRUE( relay.ReadMessage( answer, status, 5000 ) );
    ASSERT_TRUE( answer.HasSubstr( "\"id\":3" ) >= 0 );

    // the end of the stream closes the channel
    relay.Close();
    for ( UInt32 i=0; i<200 && 0 != transport->GetChannelCount(); ++i )
    {
        rig.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    ASSERT_TRUE( 0 == transport->GetChannelCount() );
    client->Close( false );
    server->Close( false );
    for ( UInt32 i=0; i<20; ++i )
        rig.pulseGenerator->ForceDirectPulse();
    rig.server->Stop( false );
}

/*-------------------------------------------------------------------------*/

static void
TestMcpChannelOfAClientThatDoesNotRead( void )
{
    if ( !McpTestJsonAvailable() )
        GUCEF_TESTFW_SKIP( "no JSON codec plugin" );

    // a client that sends requests but never reads the answers: beyond its credit the answers waited without a bound
    CMcpTestRig rig;
    COMCORE::CRpcServiceSetPtr services = COMCORE::CRpcServiceSet::Create();
    COM::CIMcpTransportPtr transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "rpc" );
    REQUIRE_TRUE( !transportPtr.IsNULL() );
    COM::CMcpRpcTransport* transport = static_cast< COM::CMcpRpcTransport* >( transportPtr.GetPointerAlways() );
    CORE::CDataNode config;
    config.SetAttribute( "maxPendingMessages", 8 );
    REQUIRE_TRUE( transport->LoadConfig( config ) );
    transport->SetServiceSet( services );
    rig.server->AddTransport( transportPtr );
    REQUIRE_TRUE( rig.server->Start() );

    COMCORE::CRpcContractRegistry clientRegistry;
    COMCORE::CRpcContractRegistry serverRegistry;
    clientRegistry.SetCacheDirectory( CORE::CString::Empty );
    serverRegistry.SetCacheDirectory( CORE::CString::Empty );
    COMCORE::CRpcEndpointSettings clientSettings;
    COMCORE::CRpcEndpointSettings serverSettings;
    clientSettings.contractRegistry = &clientRegistry;
    serverSettings.contractRegistry = &serverRegistry;
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( rig.pulseGenerator, rig.pulseGenerator, a, b ) );
    COMCORE::CRpcEndpointPtr server = COMCORE::CRpcEndpoint::Create( rig.pulseGenerator, serverSettings );
    server->SetServiceSet( services );
    COMCORE::CRpcEndpointPtr client = COMCORE::CRpcEndpoint::Create( rig.pulseGenerator, clientSettings );
    client->AddWantedContract( "gucef.mcp", 1 );
    REQUIRE_TRUE( server->Start( b, false ) );
    REQUIRE_TRUE( client->Start( a, true ) );
    for ( UInt32 i=0; i<2000 && !client->IsContractBound( "gucef.mcp", 1 ); ++i )
    {
        rig.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    REQUIRE_TRUE( client->IsContractBound( "gucef.mcp", 1 ) );

    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( client->ResolveMethod( "gucef.mcp.Session/Exchange", ref, status ) );
    COMCORE::CIRpcStreamPtr stream = client->OpenStream( ref, GUCEF_NULL );
    REQUIRE_TRUE( !stream.IsNULL() );
    stream->SetManualCredit( true );
    UInt32 sent = 0;
    for ( UInt32 round=0; round<5000 && sent < 200 && !stream->IsDone(); ++round )
    {
        COMCORE::CRpcMessage request = client->NewRequest( ref );
        request.SetString( 1, McpTestRequest( CORE::ToString( sent + 1 ), "tools/list" ) );
        if ( stream->Write( request ) )
            ++sent;
        rig.pulseGenerator->ForceDirectPulse();
    }
    for ( UInt32 i=0; i<2000 && !stream->IsDone(); ++i )
    {
        rig.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcp test: a client that does not read sent " << sent << " requests, its stream ended: " << stream->GetStatus().ToString() );
    ASSERT_TRUE( stream->IsDone() );
    ASSERT_CODE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, stream->GetStatus() );
    for ( UInt32 i=0; i<200 && 0 != transport->GetChannelCount(); ++i )
    {
        rig.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    ASSERT_TRUE( 0 == transport->GetChannelCount() );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == client->GetState() );

    client->Close( false );
    server->Close( false );
    for ( UInt32 i=0; i<20; ++i )
        rig.pulseGenerator->ForceDirectPulse();
    rig.server->Stop( false );
}

/*-------------------------------------------------------------------------*/

namespace {

/**
 *  The conformance suite over tcp pairs on 127.0.0.1: a listener and a connecting transport, either on the test
 *  thread's pulse generator or each on a task of its own
 */
class CTcpConformanceTransports : public CIRpcConformanceTransports ,
                                  public CORE::CObserver
{
    public:

    /** initiatorOnTestThread: the connecting transport on the test thread's pulse generator also with ownThreads */
    explicit CTcpConformanceTransports( bool ownThreads, bool initiatorOnTestThread = false )
        : CIRpcConformanceTransports(), CORE::CObserver(), m_ownThreads( ownThreads ), m_initiatorOnTestThread( initiatorOnTestThread )
        , m_listener(), m_listenUri(), m_lock(), m_accepted()
    {
        m_listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "tcp" );
        if ( m_listener.IsNULL() )
            return;
        typedef CORE::CTEventHandlerFunctor< CTcpConformanceTransports > TEventCallback;
        TEventCallback accepted( this, &CTcpConformanceTransports::OnAccepted );
        SubscribeTo( m_listener.GetPointerAlways(), COMCORE::CIRpcTransportListener::TransportAcceptedEvent, accepted );
        if ( !m_ownThreads )
            m_listener->SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
        if ( m_listener->Listen( CORE::CUri( "tcp://127.0.0.1:0" ) ) )
            m_listenUri = m_listener->GetListenUri();
    }

    virtual ~CTcpConformanceTransports()
    {
        UnsubscribeAllFromObserver();
        if ( !m_listener.IsNULL() )
            m_listener->StopListening();
        RpcTestPump( 20 );
    }

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_ownThreads ? "tcp, protocol on tasks of its own" : "tcp, protocol on the test thread";
    }

    virtual bool MakePair( COMCORE::CIRpcTransportPtr& initiator, COMCORE::CIRpcTransportPtr& acceptor ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_listener.IsNULL() || m_listenUri.GetScheme().IsNULLOrEmpty() )
            return false;
        initiator = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( "tcp" );
        if ( initiator.IsNULL() )
            return false;
        if ( ( !m_ownThreads || m_initiatorOnTestThread ) && !initiator->SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ) )
            return false;
        if ( !initiator->Connect( m_listenUri ) )
            return false;
        UInt64 start = MT::PrecisionTickCount();
        while ( MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
        {
            {
                MT::CScopeMutex lock( m_lock );
                if ( !m_accepted.IsNULL() )
                {
                    acceptor = m_accepted;
                    m_accepted.Unlink();
                    return true;
                }
            }
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
        return false;
    }

    private:

    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        MT::CScopeMutex lock( m_lock );
        m_accepted = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventData )->transport;
    }

    bool m_ownThreads;
    bool m_initiatorOnTestThread;
    COMCORE::CIRpcTransportListenerPtr m_listener;
    CORE::CUri m_listenUri;
    MT::CMutex m_lock;
    COMCORE::CIRpcTransportPtr m_accepted;
};

}

/*-------------------------------------------------------------------------*/

static void
TestConformanceOverTcp( bool ownThreads )
{
    CTcpConformanceTransports transports( ownThreads );
    PerformRpcConformanceTests( transports );
}

/*-------------------------------------------------------------------------*/

static void
TestPerfOverTcp( bool ownThreads )
{
    CTcpConformanceTransports transports( ownThreads );
    SRpcUnaryFigures figures;
    ASSERT_TRUE( RpcPerfUnary( transports, RpcPerfCalls( 2000 ), RpcPerfCalls( 10000 ), figures ) );
    ASSERT_TRUE( figures.medianUs > 0.0 && figures.callsPerSecond > 0.0 );

    #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
    // bytes that arrive wake the protocol task of their socket: no wait for its next periodic pulse per hop
    if ( ownThreads )
        ASSERT_TRUE( figures.medianUs < 500.0 );
    #endif
}

namespace {

/** Subscribes to a transport and counts its frames; the frames come on the transport's protocol thread */
class CFrameCounter : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt64 > frames;

    CFrameCounter( void ) : CORE::CObserver(), frames( 0 ) {}
    virtual ~CFrameCounter() { UnsubscribeAllFromObserver(); }

    void Attach( COMCORE::CIRpcTransportPtr transport )
    {
        typedef CORE::CTEventHandlerFunctor< CFrameCounter > TEventCallback;
        TEventCallback callback( this, &CFrameCounter::OnFrame );
        SubscribeTo( transport.GetPointerAlways(), COMCORE::CIRpcTransport::FrameReceivedEvent, callback );
    }

    void Detach( COMCORE::CIRpcTransportPtr transport )
    {
        UnsubscribeFrom( *transport.GetPointerAlways() );
    }

    private:

    void OnFrame( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        frames.FetchAdd( 1 );
    }
};

}

/*-------------------------------------------------------------------------*/

static void
TestSubscribeWhileFramesFlow( void )
{
    // the transports run on protocol tasks of their own and raise their events there, while this thread subscribes
    // and unsubscribes: the notifier's list of observers is shared between the two threads
    CTcpConformanceTransports transports( true );
    COMCORE::CIRpcTransportPtr initiator;
    COMCORE::CIRpcTransportPtr acceptor;
    REQUIRE_TRUE( transports.MakePair( initiator, acceptor ) );
    UInt64 start = MT::PrecisionTickCount();
    while ( !initiator->IsConnected() && MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
        MT::PrecisionDelay( 1 );
    REQUIRE_TRUE( initiator->IsConnected() && acceptor->IsConnected() );

    CFrameCounter counter;
    const char frame[] = "a frame of the test";
    for ( UInt32 i=0; i<2000; ++i )
    {
        ASSERT_TRUE( acceptor->SendFrame( frame, (UInt32) sizeof( frame ), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
        counter.Attach( initiator );
        if ( 0 == ( i % 8 ) )
            MT::ThreadDelay( 0 );
        counter.Detach( initiator );
    }

    // the frames keep coming while subscribed at the end
    counter.Attach( initiator );
    UInt64 before = counter.frames.Load();
    for ( UInt32 i=0; i<10 && counter.frames.Load() == before; ++i )
    {
        acceptor->SendFrame( frame, (UInt32) sizeof( frame ), COMCORE::RPC_PROTECTION_CONFIDENTIAL );
        MT::PrecisionDelay( 20 );
    }
    ASSERT_TRUE( counter.frames.Load() > before );
    counter.Detach( initiator );

    initiator->Close();
    acceptor->Close();
    MT::PrecisionDelay( 50 );
}

/*-------------------------------------------------------------------------*/

static void
TestTcpSendPathAllocatesNothing( void )
{
    GUCEF_TESTFW_SKIP_IF( !RPCTEST::IsAllocationCountingAvailable(), RPCTEST::GetAllocationCountingUnavailableReason() );

    // the sending side on this thread, the receiving side on a task of its own (its allocations are not this thread's):
    // every flush of the queued frames to the socket allocated and copied a buffer
    CTcpConformanceTransports transports( true, true );
    COMCORE::CIRpcTransportPtr initiator;
    COMCORE::CIRpcTransportPtr acceptor;
    REQUIRE_TRUE( transports.MakePair( initiator, acceptor ) );
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 start = MT::PrecisionTickCount();
    while ( !( initiator->IsConnected() && acceptor->IsConnected() ) && MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
    {
        pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    REQUIRE_TRUE( initiator->IsConnected() && acceptor->IsConnected() );

    CFrameCounter counter;
    counter.Attach( acceptor );
    UInt8 frame[ 200 ];
    memset( frame, 7, sizeof( frame ) );
    UInt32 allocations = 0;

    // the allocations of gucefCOM's code are counted: the sockets of gucefCOMCORE log traces in Debug builds
    for ( UInt32 round=0; round<3; ++round )
    {
        if ( 2 == round )
            RPCTEST::StartAllocationCounting( (const void*) &COM::CComGlobal::Instance );
        for ( UInt32 i=0; i<500; ++i )
        {
            initiator->SendFrame( frame, (UInt32) sizeof( frame ), COMCORE::RPC_PROTECTION_CONFIDENTIAL );
            if ( 0 == i % 4 )
                initiator->SendFrame( frame, (UInt32) sizeof( frame ), COMCORE::RPC_PROTECTION_CONFIDENTIAL );
            pulseGenerator->ForceDirectPulse();
        }
        if ( 2 == round )
            allocations = RPCTEST::StopAllocationCounting();
    }
    start = MT::PrecisionTickCount();
    while ( counter.frames.Load() < 1875 && MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
    {
        pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcp test: allocations of gucefCOM's code on the sending side for 625 frames: " << allocations << ", frames received " << counter.frames.Load() );
    ASSERT_TRUE( 1875 == counter.frames.Load() );
    ASSERT_TRUE( 0 == allocations );
    if ( 0 != allocations )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTcp test: the first allocations:\n" << RPCTEST::GetCountedAllocationStacks() );
    counter.Detach( acceptor );
    initiator->Close();
    acceptor->Close();
    for ( UInt32 i=0; i<20; ++i )
    {
        pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/** the listener of the host with the certificate of the test authority, the client trusting only that authority */
static void
UseTls( CTcpHostFixture& h, const TESTFW::CTlsTestCerts& certs, const char* trustedCa = "ca" )
{
    h.listenerTls.SetCertificateChainPath( certs.GetPath( "good.crt" ) );
    h.listenerTls.SetPrivateKeyPath( certs.GetPath( "good.key" ) );
    h.clientTls.SetCaBundlePath( certs.GetPath( ( CORE::CString( trustedCa ) + ".crt" ).C_String() ) );
    h.clientTls.SetUseSystemTrust( false );
    h.clientUsesTls = true;
}

/*-------------------------------------------------------------------------*/

static void
SkipWithoutTls( const TESTFW::CTlsTestCerts& certs )
{
    CORE::CString whyNot;
    GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
    GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNot ), "this system has no TLS provider" );
    GUCEF_TESTFW_SKIP_IF( !certs.IsReady(), "the test certificates could not be made" );
}

/*-------------------------------------------------------------------------*/

static void
TestCallsOverTls( void )
{
    TESTFW::CTlsTestCerts certs;
    SkipWithoutTls( certs );

    CRpcEndpointFixture f;
    f.clientSettings.requireSecureTransport = true;
    f.serverSettings.requireSecureTransport = true;
    CTcpHostFixture h( f );
    UseTls( h, certs );
    REQUIRE_TRUE( h.Start() );
    REQUIRE_TRUE( h.ConnectClient() );

    // both sides require a secure transport: a session at all means both transports report it
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 7 ), COMCORE::CRpcCallOptions(), 5000 );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "note 7" == result.response.GetString( 2 ) );
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    h.host->GetEndpoints( endpoints );
    REQUIRE_TRUE( 1 == endpoints.size() );
    ASSERT_FALSE( endpoints[ 0 ]->GetPeerPrincipal().isAuthenticated );

    // a server stream past the initial credit, over TLS
    f.service.watchCount = 150;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
    REQUIRE_TRUE( !stream.IsNULL() );
    COMCORE::CRpcMessage event;
    UInt32 received = 0;
    while ( stream->ReadNext( event, status, 5000 ) )
        ++received;
    ASSERT_OK( status );
    ASSERT_TRUE( 150 == received );
}

/*-------------------------------------------------------------------------*/

static void
TestTlsRefusals( void )
{
    TESTFW::CTlsTestCerts certs;
    SkipWithoutTls( certs );
    {
        // a client that trusts another authority does not get a session
        CRpcEndpointFixture f;
        CTcpHostFixture h( f );
        UseTls( h, certs, "other_ca" );
        REQUIRE_TRUE( h.Start() );
        ASSERT_FALSE( h.ConnectClient() );
        ASSERT_TRUE( 0 == h.host->GetEndpointCount() );
    }
    {
        // a plain client to a TLS listener does not get a session
        CRpcEndpointFixture f;
        CTcpHostFixture h( f );
        UseTls( h, certs );
        h.clientUsesTls = false;
        REQUIRE_TRUE( h.Start() );
        ASSERT_FALSE( h.ConnectClient() );
        ASSERT_TRUE( 0 == h.host->GetEndpointCount() );
    }
    {
        // a host that requires a secure transport refuses plain tcp
        CRpcEndpointFixture f;
        f.serverSettings.requireSecureTransport = true;
        CTcpHostFixture h( f );
        REQUIRE_TRUE( h.Start() );
        ASSERT_FALSE( h.ConnectClient() );
    }
    {
        // TLS allows another bind address than loopback, a key that does not belong to the certificate stops the listening
        COM::CRpcTcpTransportListener open;
        COMCORE::CTlsServerSettings tls;
        tls.SetCertificateChainPath( certs.GetPath( "good.crt" ) );
        tls.SetPrivateKeyPath( certs.GetPath( "good.key" ) );
        open.SetTlsSettings( tls );
        ASSERT_TRUE( open.Listen( CORE::CUri( "tcp://0.0.0.0:0" ) ) );
        open.StopListening();

        COM::CRpcTcpTransportListener broken;
        tls.SetPrivateKeyPath( certs.GetPath( "wrongname.key" ) );
        broken.SetTlsSettings( tls );
        ASSERT_FALSE( broken.Listen( CORE::CUri( "tcp://127.0.0.1:0" ) ) );
    }
    {
        // the keys of TLS are read from the configuration of the transport and the listener
        CORE::CDataNode cfg;
        cfg.SetAttribute( "useTls", true );
        cfg.SetAttribute( "caBundlePath", certs.GetPath( "ca.crt" ) );
        COM::CRpcTcpTransport transport;
        ASSERT_TRUE( transport.LoadConfig( cfg ) );
        ASSERT_TRUE( transport.IsTls() );

        CORE::CDataNode listenerCfg;
        listenerCfg.SetAttribute( "certificateChainPath", certs.GetPath( "good.crt" ) );
        listenerCfg.SetAttribute( "privateKeyPath", certs.GetPath( "good.key" ) );
        COM::CRpcTcpTransportListener listener;
        ASSERT_TRUE( listener.LoadConfig( listenerCfg ) );
        ASSERT_TRUE( listener.GetTlsSettings().IsEnabled() );
    }
}

/*-------------------------------------------------------------------------*/

#define RUN_RPC_CASE( name, call )                                  \
    GUCEF_TESTFW_TESTCASE( "rpc_tcp: " name )                       \
    try                                                             \
    {                                                               \
        call;                                                       \
    }                                                               \
    catch ( const GUCEF::TESTFW::STestSkipRequest& )                \
    {                                                               \
        throw;                                                      \
    }                                                               \
    catch ( ... )                                                   \
    {                                                               \
        ERRORHERE;                                                  \
    }                                                               \
    GUCEF_TESTFW_TESTCASE_END;

void
PerformRpcTcpTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_tcp" );

    COM::CComGlobal::Instance();
    RUN_RPC_CASE( "calls, streams and reflection through a host on a loopback listener", TestCallsThroughHost() );
    RUN_RPC_CASE( "a peer without the preface or with an oversized frame is disconnected; loopback only", TestPrefaceAndFrameLimits() );
    RUN_RPC_CASE( "calls and streams over TLS between transports that both require a secure transport", TestCallsOverTls() );
    RUN_RPC_CASE( "TLS: an untrusted authority, a plain client and a plain transport for a secure host are refused; bind and configuration", TestTlsRefusals() );
    RUN_RPC_CASE( "a session resumes from loopback onto tcp", TestResumeFromLoopbackOntoTcp() );
    RUN_RPC_CASE( "an authenticated session does not resume over unauthenticated tcp", TestNoStepDownOntoTcp() );
    RUN_RPC_CASE( "MCP tools/list over gucef.mcp.Session/Exchange through the relay", TestMcpOverRpc() );
    RUN_RPC_CASE( "MCP over RPC: the channel of a client that does not read is bounded and ends", TestMcpChannelOfAClientThatDoesNotRead() );
    RUN_RPC_CASE( "a transport on a task of its own is subscribed to from another thread while frames flow", TestSubscribeWhileFramesFlow() );
    RUN_RPC_CASE( "the send path of a tcp transport allocates nothing per frame once warm (design 15)", TestTcpSendPathAllocatesNothing() );
    TestConformanceOverTcp( false );
    TestConformanceOverTcp( true );
    {
        // the frame inspecting tests of gucefCOMCORE_TestApp over tcp on the test thread
        CTcpConformanceTransports tcp( false );
        PerformRpcTransportSuiteTests( tcp );
    }
    {
        GUCEF_TESTFW_SUITE_SCOPE( "rpc_perf" );
        RUN_RPC_CASE( "unary latency and throughput over tcp on the test thread", TestPerfOverTcp( false ) );
        RUN_RPC_CASE( "unary latency and throughput over tcp, protocol on tasks of its own", TestPerfOverTcp( true ) );
    }
}

/*-------------------------------------------------------------------------*/
