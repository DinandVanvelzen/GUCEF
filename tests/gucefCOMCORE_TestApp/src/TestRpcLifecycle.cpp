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

namespace {

/**
 *  A decorator whose last reference does not free it: the release is noted (CReleaseWatchDestructor), so a call that
 *  reaches it afterwards, which would have reached freed memory, is counted instead. An unsubscription on the hold
 *  thread waits until the gate is opened and then 300 ms more: the protocol thread holds there inside its pulse
 */
class CReleaseWatchTransport : public CRpcLossyTransport
{
    public:

    MT::CTAtomic< UInt32 > released;
    MT::CTAtomic< UInt32 > callsAfterRelease;
    MT::CTAtomic< TThreadId > holdThread;
    MT::CTAtomic< UInt32 > holding;
    MT::CTAtomic< UInt32 > gate;

    explicit CReleaseWatchTransport( const COMCORE::CIRpcTransportPtr& inner )
        : CRpcLossyTransport( inner ), released( 0 ), callsAfterRelease( 0 ), holdThread( 0 ), holding( 0 ), gate( 0 )
    {}

    virtual void Unsubscribe( CORE::CObserver* observer ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != released.Load() )
        {
            callsAfterRelease.FetchAdd( 1 );
            return;
        }
        if ( 0 != holdThread.Load() && holdThread.Load() == MT::GetCurrentThreadID() && 0 == holding.Load() )
        {
            holding.Store( 1 );
            UInt32 waited = 0;
            while ( 0 == gate.Load() && waited++ < 5000 )
                MT::PrecisionDelay( 1 );
            MT::PrecisionDelay( 300 );
        }
        CRpcLossyTransport::Unsubscribe( observer );
    }
};

/** Notes the release of the last reference of a CReleaseWatchTransport instead of destroying it */
class CReleaseWatchDestructor : public CORE::CTDynamicDestructorBase< CReleaseWatchTransport >
{
    public:

    virtual void DestroyObject( CReleaseWatchTransport* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE
    {
        objectToBeDestroyed->released.Store( 1 );
    }
};

/**
 *  Import (a client stream) of the sample contract: the handler keeps the call and subscribes to its CancelledEvent.
 *  The cancellation handler lets go of the kept reference, as handlers do;
 *  when that would be the last one (the call would go inside the delivery of its own mail) it is noted, and the
 *  reference is kept elsewhere instead, so the call does not go there
 */
class CCancelWatchService : public COMCORE::CRpcServiceImpl ,
                            public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > handled;
    MT::CTAtomic< UInt32 > cancelled;
    MT::CTAtomic< UInt32 > lastReferenceInOwnEvent;
    GUCEF::vector< COMCORE::CRpcServerCallPtr > kept;
    GUCEF::vector< COMCORE::CRpcServerCallPtr > graveyard;
    MT::CMutex lock;

    CCancelWatchService( void ) : COMCORE::CRpcServiceImpl(), CORE::CObserver(), handled( 0 ), cancelled( 0 ), lastReferenceInOwnEvent( 0 ), kept(), graveyard(), lock()
    {
        BindMethod( "Import", GUCEF_NEW COMCORE::CTRpcMethodHandler< CCancelWatchService >( this, &CCancelWatchService::OnImport ) );
        BindMethod( "Watch", GUCEF_NEW COMCORE::CTRpcMethodHandler< CCancelWatchService >( this, &CCancelWatchService::OnImport ) );
    }

    virtual ~CCancelWatchService() { UnsubscribeAllFromObserver(); }

    COMCORE::TRpcHandlerResult OnImport( COMCORE::CRpcServerCall& call )
    {
        {
            MT::CScopeMutex scope( lock );
            kept.push_back( call.GetSharedPtr() );
        }
        typedef CORE::CTEventHandlerFunctor< CCancelWatchService > TEventCallback;
        TEventCallback onCancelled( this, &CCancelWatchService::OnCancelled );
        SubscribeTo( &call, COMCORE::CRpcServerCall::CancelledEvent, onCancelled );
        handled.Store( 1 );
        return COMCORE::RPC_HANDLER_DEFERRED;
    }

    void OnCancelled( CORE::CNotifier* notifier, const CORE::CEvent&, CORE::CICloneable* )
    {
        MT::CScopeMutex scope( lock );
        for ( UInt32 i=0; i<kept.size(); ++i )
        {
            if ( static_cast< CORE::CNotifier* >( kept[ i ].GetPointerAlways() ) != notifier )
                continue;
            COMCORE::CRpcServerCallPtr call( kept[ i ] );
            kept.erase( kept.begin() + i );
            if ( 1 == call.GetReferenceCount() )
            {
                // this copy is all that is left: the kept reference was the last one
                lastReferenceInOwnEvent.Store( 1 );
                graveyard.push_back( call );
            }
            break;
        }
        cancelled.Store( 1 );
    }
};

/*-------------------------------------------------------------------------*/

/** Pulses a pulse generator as its driver thread until told to stop */
struct SProtocolPulser
{
    CORE::PulseGeneratorPtr pulseGenerator;
    MT::CTAtomic< TThreadId > threadId;
    MT::CTAtomic< UInt32 > stop;

    SProtocolPulser( void ) : pulseGenerator(), threadId( 0 ), stop( 0 ) {}
};

UInt32 GUCEF_CALLSPEC_PREFIX
ProtocolPulserMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SProtocolPulser* pulser = static_cast< SProtocolPulser* >( data );
    pulser->pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    pulser->threadId.Store( MT::GetCurrentThreadID() );
    while ( 0 == pulser->stop.Load() )
    {
        pulser->pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    return 0;
}

}

/*-------------------------------------------------------------------------*/

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

static void
TestEndpointDestroyedWhileItsPulseReleasesATransport( void )
{
    // An endpoint lets go of a detached transport in its protocol pulse, after it unsubscribed from it there. When
    // another thread destroys the endpoint at that moment, the destructor walks a copy of the endpoint's
    // subscriptions, which still names the transport, and calls it after the pulse let go of it: freed memory (a
    // drn pairing client let go of its endpoint while the node's thread pulsed it, investSERVICE_TestApp, SIGSEGV
    // in CObserver::UnsubscribeAllFromObserver() under ~CRpcEndpoint()). Here the transport's last reference does not
    // free it, so such a call is counted; the protocol thread holds in the pulse, after the copy was made
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    CRpcEndpointFixture f;

    // the protocol thread's pulse generator sorts before the transport in the subscription list (a set ordered by
    // address): the destructor then waits for the pulse before it reaches the transport
    GUCEF::vector< CORE::PulseGeneratorPtr > candidates;
    for ( UInt32 i=0; i<8; ++i )
        candidates.push_back( CORE::CPulseGenerator::CreateSharedObj() );
    SProtocolPulser pulser;
    pulser.pulseGenerator = candidates[ 0 ];
    for ( UInt32 i=1; i<candidates.size(); ++i )
        if ( static_cast< CORE::CNotifier* >( candidates[ i ].GetPointerAlways() ) < static_cast< CORE::CNotifier* >( pulser.pulseGenerator.GetPointerAlways() ) )
            pulser.pulseGenerator = candidates[ i ];

    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( pulser.pulseGenerator, pulser.pulseGenerator, a, b ) );
    CReleaseWatchDestructor watchDestructor;
    CReleaseWatchTransport* watched = GUCEF_NEW CReleaseWatchTransport( a );
    if ( static_cast< CORE::CNotifier* >( pulser.pulseGenerator.GetPointerAlways() ) > static_cast< CORE::CNotifier* >( watched ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcLifecycle test: the transport sorts before the pulse generator, the case cannot fail without the change it pins" );
    {
        CORE::CTSharedPtr< CReleaseWatchTransport, MT::CMutex > watchedPtr( watched, &watchDestructor );
        f.clientTransport = watchedPtr;
    }
    f.serverTransport = CRpcLossyTransport::Wrap( b, GUCEF_NULL );
    a.Unlink();

    MT::SThreadData* thread = MT::ThreadDataReserve();
    REQUIRE_TRUE( GUCEF_NULL != thread );
    REQUIRE_TRUE( 0 != MT::ThreadCreate( thread, &ProtocolPulserMain, &pulser ) );
    UInt32 waited = 0;
    while ( 0 == pulser.threadId.Load() && waited++ < 5000 )
        MT::PrecisionDelay( 1 );

    bool bound = f.Connect( notes, notes );
    ASSERT_TRUE( bound );
    if ( bound )
    {
        // the client endpoint holds the only reference: the transport is detached when the session closes, and let
        // go of in the client's next protocol pulse, which holds in the unsubscription (as a drn pairing client closes
        // its session and lets go of its endpoint)
        watched->holdThread.Store( pulser.threadId.Load() );
        f.clientTransport.Unlink();
        f.client->Close( true );
        waited = 0;
        while ( 0 == watched->holding.Load() && waited++ < 5000 )
        {
            f.pulseGenerator->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
        ASSERT_TRUE( 1 == watched->holding.Load() );

        // this thread destroys the endpoint while the pulse holds; the pulse goes on 300 ms later
        watched->gate.Store( 1 );
        f.client.Unlink();
        ASSERT_TRUE( 1 == watched->released.Load() );
        ASSERT_TRUE( 0 == watched->callsAfterRelease.Load() );
    }

    pulser.stop.Store( 1 );
    MT::ThreadWait( thread, 10000 );
    MT::ThreadDataCleanup( thread );
    if ( !f.server.IsNULL() )
        f.server->Close( false );
    f.server.Unlink();
    f.serverTransport.Unlink();
    b.Unlink();
    f.clientTransport.Unlink();
    f.client.Unlink();
    if ( 1 == watched->released.Load() )
        GUCEF_DELETE watched;
}

/*-------------------------------------------------------------------------*/

static void
TestEndedCallReleasedAfterItsOwnMail( void )
{
    // The endpoint lets go of an ended server call of the notification thread there (ReleaseOnNotificationThreadEvent).
    // When that mail is delivered before the call's own mail, the handler of the call's event holds the last reference
    // and lets go of it inside the delivery of the call's own mail: the call goes in the middle of its notification
    // (SIGSEGV in ~CNotifierScopeLock(), gucefDRN_TestApp rpcthreads). Here the session ends on the protocol thread
    // before the notification thread, which this thread drives, ran the call's handler: the cancellation is held
    // until the handler returned and then mailed to the call, behind the release the endpoint mailed already. The
    // handler notes a last reference instead of letting go of it
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    CRpcEndpointFixture f;
    SProtocolPulser pulser;
    pulser.pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( pulser.pulseGenerator, pulser.pulseGenerator, a, b ) );
    f.clientTransport = a;
    f.serverTransport = b;
    MT::SThreadData* thread = MT::ThreadDataReserve();
    REQUIRE_TRUE( GUCEF_NULL != thread );
    REQUIRE_TRUE( 0 != MT::ThreadCreate( thread, &ProtocolPulserMain, &pulser ) );
    UInt32 waited = 0;
    while ( 0 == pulser.threadId.Load() && waited++ < 5000 )
        MT::PrecisionDelay( 1 );

    CCancelWatchService service;
    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    bool started = f.server->RegisterService( notes, "Notes", &service, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    started = started && f.server->Start( f.serverTransport, false ) && f.client->Start( f.clientTransport, true );
    bool bound = started && f.PumpUntilBound( "gucef.sample.notes", 1, 5000 );
    ASSERT_TRUE( bound );
    if ( bound )
    {
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Import", ref, status ) );
        COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &chunk );
        REQUIRE_TRUE( !stream.IsNULL() );

        // without pulses of this thread: the call reaches the server and is mailed to this thread for its handler,
        // then the session ends (an abort), which cancels the call and retires it, and the endpoint mails its release
        MT::PrecisionDelay( 300 );
        ASSERT_TRUE( 0 == service.handled.Load() );
        f.client->Close( false );
        MT::PrecisionDelay( 300 );
        ASSERT_TRUE( 0 == service.handled.Load() );

        waited = 0;
        while ( 0 == service.cancelled.Load() && waited++ < 5000 )
        {
            f.pulseGenerator->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
        ASSERT_TRUE( 1 == service.handled.Load() && 1 == service.cancelled.Load() );
        RpcTestPump( 50 );
        ASSERT_TRUE( 0 == service.lastReferenceInOwnEvent.Load() );
        stream.Unlink();
    }

    f.client->Close( false );
    f.server->Close( false );
    RpcTestPump( 20 );
    f.client.Unlink();
    f.server.Unlink();
    pulser.stop.Store( 1 );
    MT::ThreadWait( thread, 10000 );
    MT::ThreadDataCleanup( thread );
    {
        MT::CScopeMutex scope( service.lock );
        service.graveyard.clear();
        service.kept.clear();
    }
    f.clientTransport.Unlink();
    f.serverTransport.Unlink();
    a.Unlink();
    b.Unlink();
}

/*-------------------------------------------------------------------------*/

static void
TestEndedServerStreamReleasedAfterItsOwnMailOnOneThread( void )
{
    // gucefDRNUTILS finding B-7: a host keeps the calls of its server streams and lets go of a call in the call's own
    // CancelledEvent. Everything on one pulse generator driven by this thread (transports, endpoints, events), as the
    // DRNUTILS tests do: the session ends with Close() on both sides, the endpoints go (with and without a pulse in
    // between), and the handler of the cancellation must never hold the last reference of the call (it notes it and
    // keeps the reference instead of freeing the call inside its own notification)
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( !notes.IsNULL() );
    for ( UInt32 variant=0; variant<4; ++variant )
    {
        const bool pumpBeforeRelease = 0 == ( variant & 1 );
        const COMCORE::TRpcExecutionMode mode = 0 == ( variant & 2 ) ? COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD : COMCORE::RPC_EXECUTION_CALLING_THREAD;
        CRpcEndpointFixture f;
        CCancelWatchService service;
        REQUIRE_TRUE( f.MakeLoopbackTransports() );
        f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
        f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
        REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &service, mode ) );
        f.client->AddWantedContract( "gucef.sample.notes", 1 );
        REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) && f.client->Start( f.clientTransport, true ) );
        REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 3000 ) );

        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Watch", ref, status ) );
        COMCORE::CRpcMessage request = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &request );
        REQUIRE_TRUE( !stream.IsNULL() );
        UInt32 waited = 0;
        while ( 0 == service.handled.Load() && waited++ < 3000 )
            RpcTestPump( 1 );
        ASSERT_TRUE( 1 == service.handled.Load() );

        f.client->Close( false );
        f.server->Close( false );
        if ( pumpBeforeRelease )
            RpcTestPump( 20 );
        f.client.Unlink();
        f.server.Unlink();
        RpcTestPump( 50 );
        ASSERT_TRUE( 1 == service.cancelled.Load() );
        if ( 0 != service.lastReferenceInOwnEvent.Load() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcLifecycle test: variant " << variant << ": the handler of CancelledEvent held the last reference of its server stream call" );
        ASSERT_TRUE( 0 == service.lastReferenceInOwnEvent.Load() );
        stream.Unlink();
        {
            MT::CScopeMutex scope( service.lock );
            service.graveyard.clear();
            service.kept.clear();
        }
        RpcTestPump( 5 );
    }
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
    RUN_RPC_CASE( "an endpoint destroyed on another thread while its protocol pulse lets go of a detached transport calls nothing of that transport afterwards", TestEndpointDestroyedWhileItsPulseReleasesATransport() );
    RUN_RPC_CASE( "an ended server call is let go of by the endpoint after its own mail was delivered, never so that a handler of its events holds the last reference", TestEndedCallReleasedAfterItsOwnMail() );
    RUN_RPC_CASE( "a server stream call ended with its session on one thread is let go of after its CancelledEvent was delivered: a handler that lets go of its reference there never holds the last one (B-7)", TestEndedServerStreamReleasedAfterItsOwnMailOnOneThread() );
}

/*-------------------------------------------------------------------------*/
