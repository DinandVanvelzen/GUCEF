/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#include "gucefCOMCORE_CRpcLoopbackTransportListener.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcEndpointFixture.h"

#include "TestRpcLifecycle.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

namespace {

const char* g_getPath = "gucef.sample.notes.Notes/Get";

/**
 *  A transport whose thread is given by its environment, as the thread of a drn transport is its node's: it refuses
 *  the pulse generator an endpoint offers, has none before Connect() and takes the one of its environment there.
 *  "ownthread://<name>" reaches the loopback listener "loopback://<name>"
 */
class CRpcOwnThreadTestTransport : public COMCORE::CRpcLoopbackTransport
{
    public:

    static const CORE::CString TransportType;
    static CORE::PulseGeneratorPtr environmentPulseGenerator;   /**< the thread of the environment */
    static bool environmentHasNoThread;                         /**< Connect() succeeds without a pulse generator */
    static UInt32 refusedPulseGenerators;

    CRpcOwnThreadTestTransport( void ) : COMCORE::CRpcLoopbackTransport(), m_connectCalled( false ) {}

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TransportType; }

    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++refusedPulseGenerators;
        return false;
    }

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_connectCalled || environmentHasNoThread )
            return CORE::PulseGeneratorPtr();
        return COMCORE::CRpcLoopbackTransport::GetPulseGenerator();
    }

    virtual bool Connect( const CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( remote.GetScheme() != TransportType )
            return false;
        m_connectCalled = true;
        if ( environmentHasNoThread )
            return true;
        COMCORE::CRpcLoopbackTransport::SetPulseGenerator( environmentPulseGenerator );
        return COMCORE::CRpcLoopbackTransport::Connect( CORE::CUri( "loopback://" + remote.GetAuthorityAndPath() ) );
    }

    private:

    bool m_connectCalled;
};

const CORE::CString CRpcOwnThreadTestTransport::TransportType = "ownthread";
CORE::PulseGeneratorPtr CRpcOwnThreadTestTransport::environmentPulseGenerator;
bool CRpcOwnThreadTestTransport::environmentHasNoThread = false;
UInt32 CRpcOwnThreadTestTransport::refusedPulseGenerators = 0;

CORE::CTFactory< COMCORE::CIRpcTransport, CRpcOwnThreadTestTransport, MT::CMutex > g_ownThreadTransportFactory;

/*-------------------------------------------------------------------------*/

/** Counts the events of a transport or a listener, on whichever thread raises them */
class CTransportEventCounter : public CORE::CObserver
{
    public:

    CTransportEventCounter( void ) : CORE::CObserver(), m_lock(), m_frames( 0 ), m_disconnects( 0 ), m_accepts( 0 ) {}
    virtual ~CTransportEventCounter() { UnsubscribeAllFromObserver(); }

    void WatchTransport( COMCORE::CIRpcTransport* transport )
    {
        typedef CORE::CTEventHandlerFunctor< CTransportEventCounter > TEventCallback;
        TEventCallback frame( this, &CTransportEventCounter::OnFrame );
        TEventCallback disconnected( this, &CTransportEventCounter::OnDisconnected );
        SubscribeTo( transport, COMCORE::CIRpcTransport::FrameReceivedEvent, frame );
        SubscribeTo( transport, COMCORE::CIRpcTransport::DisconnectedEvent, disconnected );
    }

    void WatchListener( COMCORE::CIRpcTransportListener* listener )
    {
        typedef CORE::CTEventHandlerFunctor< CTransportEventCounter > TEventCallback;
        TEventCallback accepted( this, &CTransportEventCounter::OnAccepted );
        SubscribeTo( listener, COMCORE::CIRpcTransportListener::TransportAcceptedEvent, accepted );
    }

    UInt32 GetFrames( void ) const { MT::CScopeMutex lock( m_lock ); return m_frames; }
    UInt32 GetDisconnects( void ) const { MT::CScopeMutex lock( m_lock ); return m_disconnects; }
    UInt32 GetAccepts( void ) const { MT::CScopeMutex lock( m_lock ); return m_accepts; }

    private:

    void OnFrame( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { MT::CScopeMutex lock( m_lock ); ++m_frames; }
    void OnDisconnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { MT::CScopeMutex lock( m_lock ); ++m_disconnects; }
    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { MT::CScopeMutex lock( m_lock ); ++m_accepts; }

    mutable MT::CMutex m_lock;
    UInt32 m_frames;
    UInt32 m_disconnects;
    UInt32 m_accepts;
};

/*-------------------------------------------------------------------------*/

/** Defers every call of Notes/Get and keeps it, without subscribing to any of its events */
class CQuietDeferringService : public COMCORE::CRpcServiceImpl
{
    public:

    GUCEF::vector< COMCORE::CRpcServerCallPtr > held;

    CQuietDeferringService( void ) : COMCORE::CRpcServiceImpl(), held()
    {
        BindMethod( "Get", GUCEF_NEW COMCORE::CTRpcMethodHandler< CQuietDeferringService >( this, &CQuietDeferringService::OnGet ) );
    }

    COMCORE::TRpcHandlerResult OnGet( COMCORE::CRpcServerCall& call )
    {
        held.push_back( call.GetSharedPtr() );
        return COMCORE::RPC_HANDLER_DEFERRED;
    }
};

/*-------------------------------------------------------------------------*/

/** Waits, without pumping, until the counter reaches the target: for events raised by threads of their own */
bool
WaitForCount( const CTransportEventCounter& counter, UInt32 ( CTransportEventCounter::*getter )( void ) const, UInt32 target, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( ( counter.*getter )() < target )
    {
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Pumps the test thread's pulse generator until the counter reaches the target */
bool
PumpUntilCount( CRpcEndpointFixture& f, const CTransportEventCounter& counter, UInt32 ( CTransportEventCounter::*getter )( void ) const, UInt32 target, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( ( counter.*getter )() < target )
    {
        f.pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
TelemetryValueOf( const COMCORE::CRpcEndpointPtr& endpoint, const char* name )
{
    CORE::CDataNode telemetry;
    endpoint->GetTelemetry( telemetry );
    return telemetry.GetAttributeValue( name ).AsUInt64();
}

/*-------------------------------------------------------------------------*/

/** A HELLO frame (6.4.1) made by hand */
void
MakeHelloFrame( const COMCORE::SRpcHello& hello, CORE::CDynamicBuffer& frame )
{
    frame.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( frame );
    COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO, 0, 0, false, 0 );
    COMCORE::CRpcFrameCodec::WriteHello( writer, hello );
}

}

#define ASSERT_OK( s )      { COMCORE::CRpcStatus assertStatus_( s ); if ( !assertStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcLifecycle test: line " << __LINE__ << ": " << assertStatus_.ToString() ); } }
#define REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcLifecycle test: requirement failed: " #t ); return; } }

/*-------------------------------------------------------------------------*/

static void
TestConnectOverTransportWithItsOwnThread( void )
{
    // CRpcEndpoint::Connect( uri ) offers the transport a protocol task's pulse generator; a transport whose thread is
    // given by its environment (drn: the node's) refuses it and brings its own, known once it connected
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    CRpcEndpointFixture f;
    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings = f.serverSettings;
    COMCORE::CRpcHost host( f.pulseGenerator, hostSettings );
    REQUIRE_TRUE( host.RegisterService( notes, "Notes", &f.service ) );
    COMCORE::CIRpcTransportListenerPtr listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "loopback" );
    REQUIRE_TRUE( !listener.IsNULL() && listener->SetPulseGenerator( f.pulseGenerator ) );
    REQUIRE_TRUE( host.AddListener( listener ) && listener->Listen( CORE::CUri( "loopback://rpc-lifecycle-own-thread" ) ) );

    COMCORE::CRpcTransportFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory();
    factory.RegisterConcreteFactory( CRpcOwnThreadTestTransport::TransportType, &g_ownThreadTransportFactory );
    CRpcOwnThreadTestTransport::environmentPulseGenerator = f.pulseGenerator;
    CRpcOwnThreadTestTransport::environmentHasNoThread = false;
    CRpcOwnThreadTestTransport::refusedPulseGenerators = 0;

    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    ASSERT_TRUE( f.client->Connect( CORE::CUri( "ownthread://rpc-lifecycle-own-thread" ) ) );
    ASSERT_TRUE( 1 == CRpcOwnThreadTestTransport::refusedPulseGenerators );
    COMCORE::CIRpcTransportPtr transport = f.client->GetTransport();
    ASSERT_TRUE( !transport.IsNULL() && transport->GetPulseGenerator() == f.pulseGenerator );
    if ( f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) )
    {
        COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
        request.SetUInt64( 1, 4 );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, request );
        ASSERT_OK( result.status );
        ASSERT_TRUE( "note 4" == result.response.GetString( 2 ) );
    }
    else
    {
        ERRORHERE;
    }

    // a transport that refuses the protocol task and has no thread of its own after Connect(): refused
    CRpcOwnThreadTestTransport::environmentHasNoThread = true;
    COMCORE::CRpcEndpointPtr threadless = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    ASSERT_FALSE( threadless->Connect( CORE::CUri( "ownthread://rpc-lifecycle-own-thread" ) ) );
    ASSERT_TRUE( 2 == CRpcOwnThreadTestTransport::refusedPulseGenerators );
    threadless->Close( false );
    threadless.Unlink();

    f.client->Close( false );
    RpcTestPump( 20 );
    transport.Unlink();
    host.Stop();
    RpcTestPump( 10 );
    factory.UnregisterConcreteFactory( CRpcOwnThreadTestTransport::TransportType );
    CRpcOwnThreadTestTransport::environmentPulseGenerator.Unlink();
    CRpcOwnThreadTestTransport::environmentHasNoThread = false;
}

/*-------------------------------------------------------------------------*/

static void
TestHostReleasesRefusedTransports( void )
{
    // the host is the only holder of an accepted transport until its HELLO. When it refuses the transport inside one
    // of its events (a first frame that is not a HELLO, a refused resume, the session limit, a disconnect before the
    // HELLO) it lets go of it at the next pulse of the transport's thread: a notifier outlives its own notification.
    // Nobody else keeps the transports here (the loopback listener does not), so a release inside the event is a use
    // after free (address sanitizer)
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    CRpcEndpointFixture f;
    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings = f.serverSettings;
    hostSettings.maxSessions = 1;
    COMCORE::CRpcHost host( f.pulseGenerator, hostSettings );
    REQUIRE_TRUE( host.RegisterService( notes, "Notes", &f.service ) );
    COMCORE::CIRpcTransportListenerPtr listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "loopback" );
    REQUIRE_TRUE( !listener.IsNULL() && listener->SetPulseGenerator( f.pulseGenerator ) );
    const CORE::CUri uri( "loopback://rpc-lifecycle-host-refusals" );
    REQUIRE_TRUE( host.AddListener( listener ) && listener->Listen( uri ) );
    COMCORE::CRpcTransportFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory();

    // a session, so that a resume and the session limit have something to refuse against
    COMCORE::CIRpcTransportPtr clientTransport = factory.Create( "loopback" );
    REQUIRE_TRUE( !clientTransport.IsNULL() && clientTransport->SetPulseGenerator( f.pulseGenerator ) && clientTransport->Connect( uri ) );
    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.client->Start( clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );
    REQUIRE_TRUE( 1 == host.GetEndpointCount() );
    GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
    host.GetEndpoints( endpoints );
    REQUIRE_TRUE( 1 == endpoints.size() );
    COMCORE::CRpcEndpointPtr server = endpoints[ 0 ];

    CORE::CDynamicBuffer frame;
    for ( UInt32 refusal=0; refusal<4; ++refusal )
    {
        COMCORE::CIRpcTransportPtr raw = factory.Create( "loopback" );
        REQUIRE_TRUE( !raw.IsNULL() && raw->SetPulseGenerator( f.pulseGenerator ) && raw->Connect( uri ) );
        CTransportEventCounter rawEvents;
        rawEvents.WatchTransport( raw.GetPointerAlways() );
        RpcTestPump( 10 );      // the listener hands the accepted side to the host
        switch ( refusal )
        {
            case 0 :
            {
                // a first frame that is not a HELLO
                const UInt8 garbage[ 8 ] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
                ASSERT_TRUE( raw->SendFrame( garbage, sizeof( garbage ), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
                break;
            }
            case 1 :
            {
                // a resume HELLO for the live session with a proof that is not valid: the endpoint refuses it
                COMCORE::SRpcHello hello;
                hello.mode = GUCEF_RPC_HELLO_RESUME;
                hello.resumeCount = 1;
                REQUIRE_TRUE( server->GetSessionId( hello.sessionId ) );
                MakeHelloFrame( hello, frame );
                ASSERT_TRUE( raw->SendFrame( frame.GetConstBufferPtr(), frame.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
                break;
            }
            case 2 :
            {
                // a new session beyond maxSessions
                COMCORE::SRpcHello hello;
                hello.nodeName = "one-too-many";
                MakeHelloFrame( hello, frame );
                ASSERT_TRUE( raw->SendFrame( frame.GetConstBufferPtr(), frame.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
                break;
            }
            default :
            {
                // gone before its HELLO
                raw->Close();
                break;
            }
        }
        ASSERT_TRUE( PumpUntilCount( f, rawEvents, &CTransportEventCounter::GetDisconnects, 1, 3000 ) );
        RpcTestPump( 10 );
        ASSERT_FALSE( raw->IsConnected() );
        ASSERT_TRUE( 1 == host.GetEndpointCount() );
        if ( 1 == refusal )
        {
            // HELLO_ACK REFUSED came back before the close
            ASSERT_TRUE( 1 == rawEvents.GetFrames() );
            ASSERT_TRUE( 1 == TelemetryValueOf( server, "refusedResumes" ) );
        }
    }

    // the session itself is untouched
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == server->GetState() );
    COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
    request.SetUInt64( 1, 2 );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, request );
    ASSERT_OK( result.status );

    f.client->Close( false );
    RpcTestPump( 20 );
    server.Unlink();
    endpoints.clear();
    host.Stop();
    RpcTestPump( 10 );
}

/*-------------------------------------------------------------------------*/

static void
TestSubscriptionsWhileTheProtocolThreadRaisesEvents( void )
{
    // an endpoint subscribes to a transport on the thread that starts it, a host to a listener on the thread that adds
    // it, while their protocol threads raise events: the transport and the listener guard their observers with a
    // notifier lock of their own (design 10.2). Without it the thread sanitizer reports the race, and the observer
    // administration may be changed under a delivery
    CORE::PulseGeneratorPtr none;
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( none, none, a, b ) );
    CTransportEventCounter all;
    all.WatchTransport( b.GetPointerAlways() );
    const UInt32 frames = 4000;
    const UInt8 bytes[ 16 ] = { 0 };
    {
        CTransportEventCounter churn;
        for ( UInt32 i=0; i<frames; ++i )
        {
            ASSERT_TRUE( a->SendFrame( bytes, sizeof( bytes ), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
            if ( 0 == i % 2 )
                churn.WatchTransport( b.GetPointerAlways() );
            else
                churn.UnsubscribeFrom( *b.GetPointerAlways() );
        }
        ASSERT_TRUE( WaitForCount( all, &CTransportEventCounter::GetFrames, frames, 20000 ) );
    }
    ASSERT_TRUE( frames == all.GetFrames() );

    COMCORE::CIRpcTransportListenerPtr listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "loopback" );
    const CORE::CUri uri( "loopback://rpc-lifecycle-subscriptions" );
    REQUIRE_TRUE( !listener.IsNULL() && listener->Listen( uri ) );     // on a task of its own
    CTransportEventCounter accepts;
    accepts.WatchListener( listener.GetPointerAlways() );
    const UInt32 connections = 300;
    {
        CTransportEventCounter churn;
        GUCEF::vector< COMCORE::CIRpcTransportPtr > clients;
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();     // nobody pumps it: only the listener side runs
        for ( UInt32 i=0; i<connections; ++i )
        {
            COMCORE::CIRpcTransportPtr client = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( "loopback" );
            REQUIRE_TRUE( !client.IsNULL() && client->SetPulseGenerator( pulseGenerator ) && client->Connect( uri ) );
            clients.push_back( client );
            if ( 0 == i % 2 )
                churn.WatchListener( listener.GetPointerAlways() );
            else
                churn.UnsubscribeFrom( *listener.GetPointerAlways() );
        }
        ASSERT_TRUE( WaitForCount( accepts, &CTransportEventCounter::GetAccepts, connections, 20000 ) );
    }
    listener->StopListening();
}

/*-------------------------------------------------------------------------*/

static void
TestUnobservedServerCallsStayUnlinked( void )
{
    // an event of a server call that nobody subscribed to reaches nobody: the call is not linked to its event thread
    // for it (a linked call is a pumped observer of that pulse generator, asking it for periodic pulses while it
    // lives). Thousands of calls cancelled at a teardown made the protocol thread pulse without pause and starved a
    // thread that subscribed to it (drn, nodes on threads of their own)
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    const UInt32 calls = 50;
    for ( UInt32 observed=0; observed<2; ++observed )
    {
        CRpcEndpointFixture f;
        CQuietDeferringService quiet;
        if ( 0 == observed )
        {
            REQUIRE_TRUE( f.MakeLoopbackTransports() );
            f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
            f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
            f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
            REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &quiet ) );
            f.client->AddWantedContract( "gucef.sample.notes", 1 );
            REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) && f.client->Start( f.clientTransport, true ) );
            REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 3000 ) );
        }
        else
        {
            // the notes service holds note 999 and subscribes to its CancelledEvent
            REQUIRE_TRUE( f.Connect( notes, notes ) );
        }
        RpcTestPump( 20 );
        const UInt32 requestorsBefore = f.pulseGenerator->GetNrOfPeriodicPulseRequestors();
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<calls; ++i )
        {
            COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
            request.SetUInt64( 1, 999 );
            futures.push_back( f.client->CallAsync( g_getPath, request ) );
        }
        RpcTestPump( 50 );
        for ( UInt32 i=0; i<futures.size(); ++i )
            futures[ i ].RequestCancellation();
        for ( UInt32 i=0; i<futures.size(); ++i )
            ASSERT_TRUE( f.PumpUntilReady( futures[ i ], 3000 ) );
        RpcTestPump( 50 );
        const UInt32 requestorsAfter = f.pulseGenerator->GetNrOfPeriodicPulseRequestors();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcLifecycle test: " << calls << " cancelled server calls " << ( 1 == observed ? "with" : "without" )
                   << " an observer, periodic pulse requestors " << requestorsBefore << " before, " << requestorsAfter << " after" );
        if ( 0 == observed )
        {
            ASSERT_TRUE( calls == quiet.held.size() );
            ASSERT_TRUE( requestorsAfter == requestorsBefore );
        }
        else
        {
            // the observer still gets every event
            ASSERT_TRUE( calls == f.service.GetCancelledCalls() );
        }
        f.client->Close( false );
        f.server->Close( false );
        RpcTestPump( 20 );
        quiet.held.clear();
    }
}

/*-------------------------------------------------------------------------*/

static void
TestLossyDecoratorPassesDowngradeOn( void )
{
    // the test decorator counts the refusal and passes it on to the transport it decorates, which may report it
    // itself (drn: its anomaly report)
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, a, b ) );
    CRpcLossyTransport* inner = GUCEF_NULL;
    CRpcLossyTransport* outer = GUCEF_NULL;
    COMCORE::CIRpcTransportPtr innerPtr = CRpcLossyTransport::Wrap( a, &inner );
    COMCORE::CIRpcTransportPtr outerPtr = CRpcLossyTransport::Wrap( innerPtr, &outer );
    outerPtr->OnProtectionDowngradeRefused( COMCORE::RPC_PROTECTION_CONFIDENTIAL, COMCORE::RPC_PROTECTION_PUBLIC );
    ASSERT_TRUE( 1 == outer->downgradeRefusals );
    ASSERT_TRUE( 1 == inner->downgradeRefusals );
}

/*-------------------------------------------------------------------------*/

#define RUN_RPC_CASE( name, call )                                  \
    GUCEF_TESTFW_TESTCASE( "rpc_lifecycle: " name )                 \
    try                                                             \
    {                                                               \
        call;                                                       \
    }                                                               \
    catch ( ... )                                                   \
    {                                                               \
        ERRORHERE;                                                  \
    }                                                               \
    GUCEF_TESTFW_TESTCASE_END;

void
PerformRpcLifecycleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_lifecycle" );

    RUN_RPC_CASE( "Connect( uri ) over a transport that refuses the protocol task and brings its own thread", TestConnectOverTransportWithItsOwnThread() );
    RUN_RPC_CASE( "the host lets go of a refused accepted transport at the next pulse of its thread, not inside its event", TestHostReleasesRefusedTransports() );
    RUN_RPC_CASE( "subscriptions on another thread while the protocol thread raises transport and listener events", TestSubscriptionsWhileTheProtocolThreadRaisesEvents() );
    RUN_RPC_CASE( "the lossy test decorator passes a protection downgrade refusal on", TestLossyDecoratorPassesDowngradeOn() );
    RUN_RPC_CASE( "a server call nobody observes is not linked to its event thread for its events", TestUnobservedServerCallsStayUnlinked() );
}

/*-------------------------------------------------------------------------*/
