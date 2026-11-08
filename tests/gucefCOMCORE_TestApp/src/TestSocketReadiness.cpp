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
#include <algorithm>

#if !defined( _WIN32 )
  #include <fcntl.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_DVSOCKET_H
#include "dvwinsock.h"      /* the socket headers of the platform, for the raw sockets of the waiter cases */
#define GUCEF_COMCORE_DVSOCKET_H
#endif /* GUCEF_COMCORE_DVSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

#ifndef GUCEF_COMCORE_CSOCKETREADINESSWAITER_H
#include "gucefCOMCORE_CSocketReadinessWaiter.h"
#define GUCEF_COMCORE_CSOCKETREADINESSWAITER_H
#endif /* GUCEF_COMCORE_CSOCKETREADINESSWAITER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSocketReadiness.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define REQUIRE_TRUE_READINESS( t )     { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "SocketReadiness test: requirement failed: " #t ); return; } }

namespace {

const char* g_poolName = "gucef.test.readiness";

/*-------------------------------------------------------------------------*/

UInt64
ElapsedUs( UInt64 fromTicks, UInt64 toTicks )
{
    UInt64 resolution = MT::PrecisionTimerResolution();
    if ( toTicks <= fromTicks || 0 == resolution )
        return 0;
    return ( ( toTicks - fromTicks ) * 1000000 ) / resolution;
}

/*-------------------------------------------------------------------------*/

/** Waits on the test thread until the counter reaches the value or the time is up */
bool
WaitForCount( const MT::CTAtomic< UInt64 >& counter, UInt64 value, UInt32 timeoutInMs )
{
    UInt64 start = MT::PrecisionTickCount();
    while ( counter.Load() < value )
    {
        if ( ElapsedUs( start, MT::PrecisionTickCount() ) > (UInt64) timeoutInMs * 1000 )
            return false;
        MT::ThreadDelay( 0 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Float64
Median( GUCEF::vector< UInt64 >& samples )
{
    if ( samples.empty() )
        return 0.0;
    std::sort( samples.begin(), samples.end() );
    return (Float64) samples[ samples.size() / 2 ];
}

/*-------------------------------------------------------------------------*/

/** Counts the pulses of a pulse generator; the pulses come on the thread that drives it */
class CPulseCounter : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt64 > pulses;

    explicit CPulseCounter( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CObserver(), pulses( 0 )
    {
        typedef CORE::CTEventHandlerFunctor< CPulseCounter > TEventCallback;
        TEventCallback callback( this, &CPulseCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    virtual ~CPulseCounter()
    {
        UnsubscribeAllFromObserver();
    }

    private:

    void OnPulse( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        pulses.FetchAdd( 1 );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Waits until the task cycles: a task that was just started may still be in its setup, and its first pulse would
 *  come late for reasons that have nothing to do with sockets. Returns with the pulses the start caused counted
 */
bool
WaitUntilTaskCycles( COMCORE::CRpcProtocolTaskPtr task, CPulseCounter& counter )
{
    UInt64 start = MT::PrecisionTickCount();
    while ( 0 == counter.pulses.Load() )
    {
        task->GetPulseGenerator()->RequestImmediatePulseThroughDriver();
        if ( ElapsedUs( start, MT::PrecisionTickCount() ) > 2000000 )
            return false;
        MT::PrecisionDelay( 1 );
    }
    MT::PrecisionDelay( 50 );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  A pulse generator driver that only counts the immediate pulse requests it gets: shows exactly which pulse
 *  generator the waiter asked, without the pulses a running task makes of its own
 */
class CCountingPulseDriver : public CORE::CIPulseGeneratorDriver
{
    public:

    MT::CTAtomic< UInt64 > immediatePulseRequests;

    CCountingPulseDriver( void ) : CORE::CIPulseGeneratorDriver(), immediatePulseRequests( 0 ) {}

    virtual void RequestImmediatePulse( CORE::CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE { immediatePulseRequests.FetchAdd( 1 ); }
    virtual void RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator&, const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPeriodicPulses( CORE::CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestPulseInterval( CORE::CPulseGenerator&, const UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual TThreadId GetPulseDriverThreadId( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }
};

/*-------------------------------------------------------------------------*/

/** A pulse generator driven by a counting driver for the length of a case */
class CCountedPulseGenerator
{
    public:

    CCountingPulseDriver driver;
    CORE::PulseGeneratorPtr pulseGenerator;

    CCountedPulseGenerator( void ) : driver(), pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() ), m_baseline( 0 )
    {
        // linking a driver asks it for a pulse: not one of the waiter's
        pulseGenerator->SetPulseGeneratorDriver( &driver );
        m_baseline = driver.immediatePulseRequests.Load();
    }

    ~CCountedPulseGenerator()
    {
        pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
    }

    /** The requests made since the driver was linked */
    UInt64 Requests( void ) const { return driver.immediatePulseRequests.Load() - m_baseline; }

    /** Waits until Requests() reaches the count */
    bool WaitForRequests( UInt64 count, UInt32 timeoutInMs ) const { return WaitForCount( driver.immediatePulseRequests, m_baseline + count, timeoutInMs ); }

    private:

    UInt64 m_baseline;
};

/*-------------------------------------------------------------------------*/

/** A plain UDP socket of the platform bound to 127.0.0.1, for the cases on the waiter itself */
class CRawUdpSocket
{
    public:

    SOCKET descriptor;
    UInt16 port;

    CRawUdpSocket( void ) : descriptor( INVALID_SOCKET ), port( 0 ) {}
    ~CRawUdpSocket() { Close(); }

    bool Open( void )
    {
        Close();
        descriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( INVALID_SOCKET == descriptor )
            return false;
        struct sockaddr_in address;
        memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        address.sin_port = 0;
        if ( 0 != bind( descriptor, (struct sockaddr*) &address, (int) sizeof( address ) ) )
            return false;
        socklen_t addressSize = (socklen_t) sizeof( address );
        if ( 0 != getsockname( descriptor, (struct sockaddr*) &address, &addressSize ) )
            return false;
        port = ntohs( address.sin_port );
        return true;
    }

    bool SendTo( UInt16 targetPort, const char* text )
    {
        struct sockaddr_in address;
        memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        address.sin_port = htons( targetPort );
        int size = (int) strlen( text );
        return size == (int) sendto( descriptor, text, size, 0, (struct sockaddr*) &address, (int) sizeof( address ) );
    }

    void Close( void )
    {
        if ( INVALID_SOCKET == descriptor )
            return;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        closesocket( descriptor );
        #else
        close( descriptor );
        #endif
        descriptor = INVALID_SOCKET;
        port = 0;
    }
};

/*-------------------------------------------------------------------------*/

/** Records what the sockets on a task report, on the task's thread; the test thread reads the atomics */
class CSocketRecorder : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt64 > packets;
    MT::CTAtomic< UInt64 > bytes;
    MT::CTAtomic< UInt64 > lastTicks;
    MT::CTAtomic< UInt64 > connected;
    MT::CTAtomic< UInt64 > connectedTicks;
    MT::CTAtomic< UInt64 > accepted;

    CSocketRecorder( void )
        : CORE::CObserver(), packets( 0 ), bytes( 0 ), lastTicks( 0 ), connected( 0 ), connectedTicks( 0 ), accepted( 0 )
    {}

    virtual ~CSocketRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    void ListenTo( COMCORE::CUDPSocket& socket )
    {
        TEventCallback callback( this, &CSocketRecorder::OnPackets );
        SubscribeTo( &socket, COMCORE::CUDPSocket::UDPPacketsRecievedEvent, callback );
    }

    void ListenTo( COMCORE::CTCPClientSocket& socket )
    {
        TEventCallback connectedCallback( this, &CSocketRecorder::OnConnected );
        TEventCallback dataCallback( this, &CSocketRecorder::OnData );
        SubscribeTo( &socket, COMCORE::CTCPConnection::ConnectedEvent, connectedCallback );
        SubscribeTo( &socket, COMCORE::CTCPConnection::DataRecievedEvent, dataCallback );
    }

    /** Echoes what every accepted connection receives */
    void EchoFor( COMCORE::CTCPServerSocket& socket )
    {
        TEventCallback acceptCallback( this, &CSocketRecorder::OnAccepted );
        SubscribeTo( &socket, COMCORE::CTCPServerSocket::ClientConnectedEvent, acceptCallback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CSocketRecorder > TEventCallback;

    void OnPackets( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        UInt64 ticks = MT::PrecisionTickCount();
        if ( GUCEF_NULL == eventData )
            return;
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        lastTicks.Store( ticks );
        packets.FetchAdd( data.packetsReceived );
    }

    void OnConnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        connectedTicks.Store( MT::PrecisionTickCount() );
        connected.FetchAdd( 1 );
    }

    void OnData( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        UInt64 ticks = MT::PrecisionTickCount();
        if ( GUCEF_NULL == eventData )
            return;
        const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
        lastTicks.Store( ticks );
        bytes.FetchAdd( data.GetDataSize() );
    }

    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData )->GetData().connection;
        TEventCallback echoCallback( this, &CSocketRecorder::OnEcho );
        SubscribeTo( connection, COMCORE::CTCPConnection::DataRecievedEvent, echoCallback );
        accepted.FetchAdd( 1 );
    }

    void OnEcho( CORE::CNotifier* notifier, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
        static_cast< COMCORE::CTCPServerConnection* >( notifier )->Send( data.GetConstBufferPtr(), data.GetDataSize() );
    }
};

/*-------------------------------------------------------------------------*/

/** UDP datagrams sent from the test thread to a socket on a dedicated task, the time until its event on the task */
bool
MeasureUdpWake( bool useReadinessWake, UInt32 samplesWanted, Float64& medianUs )
{
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( g_poolName );
    if ( task.IsNULL() )
        return false;

    bool ok = true;
    GUCEF::vector< UInt64 > samples;
    {
        CSocketRecorder recorder;
        {
            CPulseCounter pulses( task->GetPulseGenerator() );
            if ( !WaitUntilTaskCycles( task, pulses ) )
                ok = false;
        }
        COMCORE::CUDPSocket receiver( task->GetPulseGenerator(), false );
        receiver.SetUseReadinessWake( useReadinessWake );
        recorder.ListenTo( receiver );
        COMCORE::CUDPSocket sender( false );
        if ( !receiver.Open( "127.0.0.1", 0 ) || !sender.Open( "127.0.0.1", 0 ) )
            ok = false;

        const char payload[] = "readiness";
        for ( UInt32 i=0; ok && i<samplesWanted; ++i )
        {
            UInt64 before = recorder.packets.Load();
            UInt64 sentTicks = MT::PrecisionTickCount();
            if ( sender.SendPacketTo( "127.0.0.1", receiver.GetPort(), payload, (UInt16) sizeof( payload ) ) <= 0 )
            {
                ok = false;
                break;
            }
            if ( !WaitForCount( recorder.packets, before + 1, 2000 ) )
            {
                ok = false;
                break;
            }
            samples.push_back( ElapsedUs( sentTicks, recorder.lastTicks.Load() ) );

            // the next datagram does not land right at the task's next periodic pulse
            MT::PrecisionDelay( 1 + ( i % 3 ) );
        }

        // no more pulses on the socket before it goes, then it is closed on this thread
        task->Stop();
        receiver.Close( false );
        sender.Close( false );
    }
    medianUs = Median( samples );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadiness: udp, " << ( useReadinessWake ? "readiness wake" : "periodic pulses" ) << ": median " << medianUs << " us over " << samples.size() << " datagrams" );
    return ok;
}

}

/*-------------------------------------------------------------------------*/

static void
TestOneShotArming( void )
{
    COMCORE::CSocketReadinessWaiterPtr waiter = COMCORE::CComCoreGlobal::Instance()->GetSocketReadinessWaiter();
    GUCEF_TESTFW_ASSERT_TRUE( !waiter.IsNULL() && waiter->IsAvailable() );
    if ( waiter.IsNULL() )
        return;

    CCountedPulseGenerator owner;
    CRawUdpSocket receiver;
    CRawUdpSocket sender;
    GUCEF_TESTFW_ASSERT_TRUE( receiver.Open() && sender.Open() );

    COMCORE::CSocketReadinessWaiter::TRegistrationId id = waiter->Register( (UInt64) receiver.descriptor, owner.pulseGenerator );
    GUCEF_TESTFW_ASSERT_TRUE( 0 != id );

    // readable but not armed: no wake
    GUCEF_TESTFW_ASSERT_TRUE( sender.SendTo( receiver.port, "one" ) && sender.SendTo( receiver.port, "two" ) && sender.SendTo( receiver.port, "three" ) );
    MT::PrecisionDelay( 100 );
    GUCEF_TESTFW_ASSERT_TRUE( 0 == owner.Requests() );

    // armed: one wake, and only one while nobody reads the datagrams
    GUCEF_TESTFW_ASSERT_TRUE( waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
    GUCEF_TESTFW_ASSERT_TRUE( owner.WaitForRequests( 1, 1000 ) );
    MT::PrecisionDelay( 250 );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SocketReadiness: one arming, data left unread: " << owner.Requests() << " pulse requests in 250 ms" );
    GUCEF_TESTFW_ASSERT_TRUE( 1 == owner.Requests() );

    // armed again with the data still there: woken at once, again once
    GUCEF_TESTFW_ASSERT_TRUE( waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
    GUCEF_TESTFW_ASSERT_TRUE( owner.WaitForRequests( 2, 1000 ) );
    MT::PrecisionDelay( 250 );
    GUCEF_TESTFW_ASSERT_TRUE( 2 == owner.Requests() );

    waiter->Unregister( id );
    GUCEF_TESTFW_ASSERT_FALSE( waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
}

/*-------------------------------------------------------------------------*/

static void
TestNoPulseWithoutDriver( void )
{
    COMCORE::CSocketReadinessWaiterPtr waiter = COMCORE::CComCoreGlobal::Instance()->GetSocketReadinessWaiter();
    if ( waiter.IsNULL() )
    {
        GUCEF_TESTFW_ERRORHERE;
        return;
    }

    // a pulse generator nobody drives would pulse on the calling thread if asked with RequestImmediatePulse()
    CORE::PulseGeneratorPtr undriven = CORE::CPulseGenerator::CreateSharedObj();
    CPulseCounter pulses( undriven );
    CRawUdpSocket receiver;
    CRawUdpSocket sender;
    GUCEF_TESTFW_ASSERT_TRUE( receiver.Open() && sender.Open() );
    GUCEF_TESTFW_ASSERT_FALSE( undriven->RequestImmediatePulseThroughDriver() );

    UInt64 wakes = waiter->GetWakeCount();
    COMCORE::CSocketReadinessWaiter::TRegistrationId id = waiter->Register( (UInt64) receiver.descriptor, undriven );
    GUCEF_TESTFW_ASSERT_TRUE( 0 != id && waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
    GUCEF_TESTFW_ASSERT_TRUE( sender.SendTo( receiver.port, "nobody drives" ) );
    MT::PrecisionDelay( 150 );
    GUCEF_TESTFW_ASSERT_TRUE( 0 == pulses.pulses.Load() );
    GUCEF_TESTFW_ASSERT_TRUE( wakes == waiter->GetWakeCount() );
    waiter->Unregister( id );
}

/*-------------------------------------------------------------------------*/

static void
TestCloseWhileWaiting( void )
{
    COMCORE::CSocketReadinessWaiterPtr waiter = COMCORE::CComCoreGlobal::Instance()->GetSocketReadinessWaiter();
    GUCEF_TESTFW_ASSERT_TRUE( !waiter.IsNULL() );
    if ( waiter.IsNULL() )
        return;

    UInt32 registrationsBefore = waiter->GetRegistrationCount();
    CCountedPulseGenerator ownerA;
    CCountedPulseGenerator ownerB;
    CRawUdpSocket socketA;
    CRawUdpSocket socketB;
    CRawUdpSocket sender;
    GUCEF_TESTFW_ASSERT_TRUE( socketA.Open() && sender.Open() );

    // A is armed while the waiter waits, then goes away under it
    COMCORE::CSocketReadinessWaiter::TRegistrationId idA = waiter->Register( (UInt64) socketA.descriptor, ownerA.pulseGenerator );
    GUCEF_TESTFW_ASSERT_TRUE( 0 != idA && waiter->Arm( idA, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
    MT::PrecisionDelay( 30 );
    SOCKET descriptorOfA = socketA.descriptor;
    UInt16 portOfA = socketA.port;
    waiter->Unregister( idA );
    socketA.Close();

    // B, usually under the descriptor number A had, belongs to another pulse generator
    GUCEF_TESTFW_ASSERT_TRUE( socketB.Open() );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SocketReadiness: the new socket " << ( socketB.descriptor == descriptorOfA ? "reuses" : "does not reuse" ) << " the closed socket's descriptor number" );
    COMCORE::CSocketReadinessWaiter::TRegistrationId idB = waiter->Register( (UInt64) socketB.descriptor, ownerB.pulseGenerator );
    GUCEF_TESTFW_ASSERT_TRUE( 0 != idB && idB != idA );
    GUCEF_TESTFW_ASSERT_TRUE( waiter->Arm( idB, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
    GUCEF_TESTFW_ASSERT_FALSE( waiter->Arm( idA, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );

    sender.SendTo( portOfA, "nobody" );
    GUCEF_TESTFW_ASSERT_TRUE( sender.SendTo( socketB.port, "for B" ) );
    GUCEF_TESTFW_ASSERT_TRUE( ownerB.WaitForRequests( 1, 1000 ) );
    MT::PrecisionDelay( 150 );
    GUCEF_TESTFW_ASSERT_TRUE( 0 == ownerA.Requests() );
    GUCEF_TESTFW_ASSERT_TRUE( 1 == ownerB.Requests() );

    waiter->Unregister( idB );
    GUCEF_TESTFW_ASSERT_TRUE( registrationsBefore == waiter->GetRegistrationCount() );
}

/*-------------------------------------------------------------------------*/

static void
TestShutdown( void )
{
    // a waiter of its own: the process wide one stays for the other cases
    COMCORE::CSocketReadinessWaiterPtr waiter = COMCORE::CSocketReadinessWaiter::CreateSharedObj();
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( g_poolName );
    GUCEF_TESTFW_ASSERT_TRUE( !waiter.IsNULL() && !task.IsNULL() );
    if ( waiter.IsNULL() || task.IsNULL() )
        return;

    {
        CRawUdpSocket socket;
        GUCEF_TESTFW_ASSERT_TRUE( socket.Open() );
        COMCORE::CSocketReadinessWaiter::TRegistrationId id = waiter->Register( (UInt64) socket.descriptor, task->GetPulseGenerator() );
        GUCEF_TESTFW_ASSERT_TRUE( 0 != id && waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );

        waiter->Shutdown();
        GUCEF_TESTFW_ASSERT_FALSE( waiter->IsAvailable() );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == waiter->GetRegistrationCount() );
        GUCEF_TESTFW_ASSERT_FALSE( waiter->Arm( id, COMCORE::CSocketReadinessWaiter::ReadinessRead ) );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == waiter->Register( (UInt64) socket.descriptor, task->GetPulseGenerator() ) );
        waiter->Unregister( id );   // nothing left to do
        waiter->Shutdown();         // twice is once
    }
    task->Stop();
}

/*-------------------------------------------------------------------------*/

static void
TestUdpWakeLatency( void )
{
    Float64 withWakeUs = 0.0;
    Float64 periodicUs = 0.0;
    GUCEF_TESTFW_ASSERT_TRUE( MeasureUdpWake( true, 50, withWakeUs ) );
    GUCEF_TESTFW_ASSERT_TRUE( MeasureUdpWake( false, 20, periodicUs ) );
    GUCEF_TESTFW_ASSERT_TRUE( withWakeUs > 0.0 && periodicUs > 0.0 );

    // without the wake the datagram waits for the task's next periodic pulse
    GUCEF_TESTFW_ASSERT_TRUE( withWakeUs < periodicUs );

    #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
    GUCEF_TESTFW_ASSERT_TRUE( withWakeUs < 2000.0 );
    #endif
}

/*-------------------------------------------------------------------------*/

static void
TestTcpWakeLatency( void )
{
    COMCORE::CRpcProtocolTaskPtr serverTask = COMCORE::CRpcProtocolTask::Start( g_poolName );
    COMCORE::CRpcProtocolTaskPtr clientTask = COMCORE::CRpcProtocolTask::Start( g_poolName );
    GUCEF_TESTFW_ASSERT_TRUE( !serverTask.IsNULL() && !clientTask.IsNULL() );
    if ( serverTask.IsNULL() || clientTask.IsNULL() )
        return;

    GUCEF::vector< UInt64 > samples;
    UInt64 connectUs = 0;
    {
        CSocketRecorder serverRecorder;
        CSocketRecorder clientRecorder;
        COMCORE::CTCPServerSocket server( serverTask->GetPulseGenerator(), false, 4 );
        server.SetUseReadinessWake( true );
        GUCEF_TESTFW_ASSERT_TRUE( server.GetUseReadinessWake() );
        serverRecorder.EchoFor( server );
        GUCEF_TESTFW_ASSERT_TRUE( server.SetBindAddress( "127.0.0.1" ) && server.ListenOnPort( 0 ) );

        COMCORE::CTCPClientSocket client( clientTask->GetPulseGenerator(), false );
        client.SetUseReadinessWake( true );
        clientRecorder.ListenTo( client );

        // both tasks past their start, which would otherwise count as connect time
        {
            CPulseCounter serverPulses( serverTask->GetPulseGenerator() );
            CPulseCounter clientPulses( clientTask->GetPulseGenerator() );
            GUCEF_TESTFW_ASSERT_TRUE( WaitUntilTaskCycles( serverTask, serverPulses ) && WaitUntilTaskCycles( clientTask, clientPulses ) );
        }

        // the end of the connect (writable) and the waiting connection (readable) wake the tasks
        UInt64 startTicks = MT::PrecisionTickCount();
        GUCEF_TESTFW_ASSERT_TRUE( client.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
        GUCEF_TESTFW_ASSERT_TRUE( WaitForCount( clientRecorder.connected, 1, 2000 ) );
        GUCEF_TESTFW_ASSERT_TRUE( WaitForCount( serverRecorder.accepted, 1, 2000 ) );
        connectUs = ElapsedUs( startTicks, clientRecorder.connectedTicks.Load() );

        const char payload[ 8 ] = { 'r', 'e', 'a', 'd', 'i', 'n', 'e', 's' };
        for ( UInt32 i=0; i<50 && 1 == clientRecorder.connected.Load(); ++i )
        {
            UInt64 before = clientRecorder.bytes.Load();
            UInt64 sentTicks = MT::PrecisionTickCount();
            if ( !client.Send( payload, sizeof( payload ) ) )
                break;
            if ( !WaitForCount( clientRecorder.bytes, before + sizeof( payload ), 2000 ) )
                break;
            samples.push_back( ElapsedUs( sentTicks, clientRecorder.lastTicks.Load() ) );
            MT::PrecisionDelay( 1 + ( i % 3 ) );
        }

        serverTask->Stop();
        clientTask->Stop();
        client.Close();
        server.Close();
    }

    GUCEF_TESTFW_ASSERT_TRUE( 50 == samples.size() );
    Float64 medianUs = Median( samples );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadiness: tcp, readiness wake on both tasks: connect " << connectUs << " us, echo round trip median " << medianUs << " us over " << samples.size() << " round trips" );

    #if !defined( __SANITIZE_THREAD__ ) && !defined( __SANITIZE_ADDRESS__ )
    GUCEF_TESTFW_ASSERT_TRUE( medianUs < 2000.0 );
    GUCEF_TESTFW_ASSERT_TRUE( connectUs < 5000 );
    #endif
}

/*-------------------------------------------------------------------------*/

static void
TestIdleWaiter( void )
{
    COMCORE::CSocketReadinessWaiterPtr waiter = COMCORE::CComCoreGlobal::Instance()->GetSocketReadinessWaiter();
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( g_poolName );
    GUCEF_TESTFW_ASSERT_TRUE( !waiter.IsNULL() && !task.IsNULL() );
    if ( waiter.IsNULL() || task.IsNULL() )
        return;

    {
        // registered, armed and silent sockets
        COMCORE::CUDPSocket first( task->GetPulseGenerator(), false );
        COMCORE::CUDPSocket second( task->GetPulseGenerator(), false );
        first.SetUseReadinessWake( true );
        second.SetUseReadinessWake( true );
        GUCEF_TESTFW_ASSERT_TRUE( first.Open( "127.0.0.1", 0 ) && second.Open( "127.0.0.1", 0 ) );
        GUCEF_TESTFW_ASSERT_TRUE( waiter->GetRegistrationCount() >= 2 );

        UInt64 waitsBefore = waiter->GetWaitCount();
        UInt64 wakesBefore = waiter->GetWakeCount();
        MT::PrecisionDelay( 1000 );
        UInt64 waits = waiter->GetWaitCount() - waitsBefore;
        UInt64 wakes = waiter->GetWakeCount() - wakesBefore;
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadiness: idle second: " << waits << " wait returns, " << wakes << " wakes" );

        // the waiter sleeps in the OS wait, its timeout (100 ms) is all that ends it
        GUCEF_TESTFW_ASSERT_TRUE( waits <= 12 );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == wakes );

        task->Stop();
        first.Close( false );
        second.Close( false );
    }
}

namespace {

/** Observes DataSentEvent on the thread that sends, as a subscriber that uses the payload would */
class CSentBytesCounter : public CORE::CObserver
{
    public:

    UInt64 bytes;

    CSentBytesCounter( void ) : CORE::CObserver(), bytes( 0 ) {}
    virtual ~CSentBytesCounter() { UnsubscribeAllFromObserver(); }

    void Attach( CORE::CNotifier* socket )
    {
        TEventCallback callback( this, &CSentBytesCounter::OnSent );
        SubscribeTo( socket, COMCORE::CTCPConnection::DataSentEvent, callback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CSentBytesCounter > TEventCallback;

    void OnSent( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
            bytes += static_cast< COMCORE::CTCPConnection::TDataSentEventData* >( eventData )->GetData().GetDataSize();
    }
};

/*-------------------------------------------------------------------------*/

/** Observes DataSentEvent from another thread: the event is mailed, so its payload is copied for the mail */
class CMailedSentBytesCounter : public CORE::CTSGNotifier
{
    public:

    MT::CTAtomic< UInt64 > bytes;

    explicit CMailedSentBytesCounter( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CTSGNotifier( pulseGenerator, false, false ), bytes( 0 )
    {}

    virtual ~CMailedSentBytesCounter() { SignalUpcomingDestruction(); }

    void Attach( CORE::CNotifier* socket )
    {
        SubscribeTo( socket, COMCORE::CTCPConnection::DataSentEvent );
    }

    protected:

    virtual void OnPumpedNotify( CORE::CNotifier*, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CTCPConnection::DataSentEvent == eventId && GUCEF_NULL != eventData )
            bytes.FetchAdd( static_cast< COMCORE::CTCPConnection::TDataSentEventData* >( eventData )->GetData().GetDataSize() );
    }
};

/*-------------------------------------------------------------------------*/

/** A plain TCP socket of the platform: a listener on 127.0.0.1 and the connection it accepts, read without blocking */
class CRawTcpPeer
{
    public:

    SOCKET listener;
    SOCKET connection;
    UInt16 port;

    CRawTcpPeer( void ) : listener( INVALID_SOCKET ), connection( INVALID_SOCKET ), port( 0 ) {}
    ~CRawTcpPeer() { CloseSocket( connection ); CloseSocket( listener ); }

    bool Listen( void )
    {
        listener = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( INVALID_SOCKET == listener )
            return false;
        struct sockaddr_in address;
        memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        if ( 0 != bind( listener, (struct sockaddr*) &address, (int) sizeof( address ) ) || 0 != listen( listener, 4 ) )
            return false;
        socklen_t addressSize = (socklen_t) sizeof( address );
        if ( 0 != getsockname( listener, (struct sockaddr*) &address, &addressSize ) )
            return false;
        port = ntohs( address.sin_port );
        return true;
    }

    bool Accept( void )
    {
        connection = accept( listener, GUCEF_NULL, GUCEF_NULL );
        if ( INVALID_SOCKET == connection )
            return false;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        u_long nonBlocking = 1;
        return 0 == ioctlsocket( connection, FIONBIO, &nonBlocking );
        #else
        return 0 == fcntl( connection, F_SETFL, fcntl( connection, F_GETFL, 0 ) | O_NONBLOCK );
        #endif
    }

    /** A plain client socket of the platform connected to this listener, accepted as the connection */
    bool ConnectRawClient( SOCKET& client )
    {
        client = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( INVALID_SOCKET == client )
            return false;
        int noDelay = 1;
        setsockopt( client, IPPROTO_TCP, TCP_NODELAY, (const char*) &noDelay, (int) sizeof( noDelay ) );
        struct sockaddr_in address;
        memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        address.sin_port = htons( port );
        if ( 0 != connect( client, (struct sockaddr*) &address, (int) sizeof( address ) ) )
            return false;
        return Accept();
    }

    void Drain( void )
    {
        char buffer[ 16384 ];
        while ( recv( connection, buffer, (int) sizeof( buffer ), 0 ) > 0 ) {}
    }

    static void CloseSocket( SOCKET& s )
    {
        if ( INVALID_SOCKET == s )
            return;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        closesocket( s );
        #else
        close( s );
        #endif
        s = INVALID_SOCKET;
    }
};

/*-------------------------------------------------------------------------*/

/** Nanoseconds per send() of the platform of 64 bytes, the floor under Send() */
Float64
MeasureRawSends( SOCKET client, CRawTcpPeer& peer, UInt32 sends )
{
    char payload[ 64 ];
    memset( payload, 'x', sizeof( payload ) );
    UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<sends; ++i )
    {
        if ( (int) sizeof( payload ) != (int) send( client, payload, (int) sizeof( payload ), 0 ) )
            return -1.0;
        if ( 63 == ( i % 64 ) )
            peer.Drain();
    }
    UInt64 elapsedTicks = MT::PrecisionTickCount() - start;
    peer.Drain();
    return ( elapsedTicks * 1000000000.0 / MT::PrecisionTimerResolution() ) / sends;
}

/*-------------------------------------------------------------------------*/

/** Nanoseconds per Send() of 64 bytes, the peer read empty every 64 sends so that nothing is queued */
Float64
MeasureSends( COMCORE::CTCPClientSocket& socket, CRawTcpPeer& peer, UInt32 sends )
{
    char payload[ 64 ];
    memset( payload, 'x', sizeof( payload ) );
    UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<sends; ++i )
    {
        if ( !socket.Send( payload, (UInt32) sizeof( payload ) ) )
            return -1.0;
        if ( 63 == ( i % 64 ) )
            peer.Drain();
    }
    UInt64 elapsedTicks = MT::PrecisionTickCount() - start;
    peer.Drain();
    return ( elapsedTicks * 1000000000.0 / MT::PrecisionTimerResolution() ) / sends;
}

}

/*-------------------------------------------------------------------------*/

static void
TestDataSentEventCost( void )
{
    // the socket on this thread's pulse generator, its peer a plain socket that this thread reads
    CRawTcpPeer peer;
    REQUIRE_TRUE_READINESS( peer.Listen() );
    COMCORE::CTCPClientSocket client( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false );
    REQUIRE_TRUE_READINESS( client.ConnectTo( "127.0.0.1", peer.port, true ) );
    REQUIRE_TRUE_READINESS( peer.Accept() );

    const UInt32 sends = 20000;
    MeasureSends( client, peer, 2000 );   // warm up

    Float64 nobodyNs = MeasureSends( client, peer, sends );

    Float64 sameThreadNs = 0.0;
    {
        CSentBytesCounter counter;
        counter.Attach( &client );
        sameThreadNs = MeasureSends( client, peer, sends );
        GUCEF_TESTFW_ASSERT_TRUE( (UInt64) sends * 64 == counter.bytes );
    }

    Float64 mailedNs = 0.0;
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( g_poolName );
    REQUIRE_TRUE_READINESS( !task.IsNULL() );
    {
        CMailedSentBytesCounter counter( task->GetPulseGenerator() );
        counter.Attach( &client );
        mailedNs = MeasureSends( client, peer, sends );
        task->GetPulseGenerator()->RequestPeriodicPulses( &counter, 1 );
        UInt64 start = MT::PrecisionTickCount();
        while ( counter.bytes.Load() < (UInt64) sends * 64 && MT::PrecisionTickCount() - start < 10 * MT::PrecisionTimerResolution() )
            MT::PrecisionDelay( 5 );
        GUCEF_TESTFW_ASSERT_TRUE( (UInt64) sends * 64 == counter.bytes.Load() );
        task->GetPulseGenerator()->RequestStopOfPeriodicUpdates( &counter );
        task->Stop();
    }

    // the same exchange without the socket class: what the system call costs
    Float64 rawNs = 0.0;
    {
        CRawTcpPeer rawPeer;
        SOCKET rawClient = INVALID_SOCKET;
        REQUIRE_TRUE_READINESS( rawPeer.Listen() && rawPeer.ConnectRawClient( rawClient ) );
        MeasureRawSends( rawClient, rawPeer, 2000 );
        rawNs = MeasureRawSends( rawClient, rawPeer, sends );
        CRawTcpPeer::CloseSocket( rawClient );
    }

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSend: send() of the platform, 64 bytes: " << rawNs << " ns" );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSend: CTCPClientSocket::Send of 64 bytes: " << nobodyNs << " ns without a DataSentEvent observer, "
               << sameThreadNs << " ns with one on the sending thread, " << mailedNs << " ns with one on another thread (the payload is copied for its mail)" );
    GUCEF_TESTFW_ASSERT_TRUE( rawNs > 0.0 && nobodyNs > 0.0 && sameThreadNs > 0.0 && mailedNs > 0.0 );
    client.Close();
}

/*-------------------------------------------------------------------------*/

/**
 *  The owner opens and closes a UDP socket on its thread while the task the socket lives on pulses it every
 *  millisecond: Open() and Close() publish the descriptor that Update() reads on the task's thread. Without a
 *  sanitizer the test shows that every cycle works; under the thread sanitizer it is the pin of the descriptor
 *  publication (a data race report on the descriptor otherwise).
 */
static void
TestUdpOpenCloseWhileTaskPulses( void )
{
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( g_poolName );
    REQUIRE_TRUE_READINESS( !task.IsNULL() );

    const UInt32 cycles = 50;
    UInt32 opened = 0;
    UInt64 pulsesDuring = 0;
    {
        CPulseCounter pulses( task->GetPulseGenerator() );
        GUCEF_TESTFW_ASSERT_TRUE( WaitUntilTaskCycles( task, pulses ) );
        task->GetPulseGenerator()->RequestPeriodicPulses( &pulses, 1 );

        COMCORE::CUDPSocket socket( task->GetPulseGenerator(), false );
        socket.SetUseReadinessWake( true );
        UInt64 before = pulses.pulses.Load();
        for ( UInt32 i=0; i<cycles; ++i )
        {
            if ( socket.Open( "127.0.0.1", 0 ) && socket.IsActive() )
                ++opened;

            // let the task pulse the open socket for a moment
            MT::PrecisionDelay( 2 );
            socket.Close( false );
            GUCEF_TESTFW_ASSERT_FALSE( socket.IsActive() );
        }
        pulsesDuring = pulses.pulses.Load() - before;

        task->GetPulseGenerator()->RequestStopOfPeriodicUpdates( &pulses );
        task->Stop();
        socket.Close( false );
    }
    GUCEF_TESTFW_ASSERT_TRUE( cycles == opened );

    // the task pulsed while the sockets were opened and closed, or the case shows nothing
    GUCEF_TESTFW_ASSERT_TRUE( pulsesDuring >= cycles );
}

/*-------------------------------------------------------------------------*/

#define RUN_READINESS_CASE( name, call )                            \
    GUCEF_TESTFW_TESTCASE( "socket_readiness: " name )              \
    try                                                             \
    {                                                               \
        call;                                                       \
    }                                                               \
    catch ( ... )                                                   \
    {                                                               \
        GUCEF_TESTFW_ERRORHERE;                                     \
    }                                                               \
    GUCEF_TESTFW_TESTCASE_END;

void
PerformSocketReadinessTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "socket_readiness" );

    COMCORE::CComCoreGlobal::Instance();
    RUN_READINESS_CASE( "arming is one shot: no wake storm while the data is not read", TestOneShotArming() );
    RUN_READINESS_CASE( "a pulse generator without a driver is not pulsed on the waiter's thread", TestNoPulseWithoutDriver() );
    RUN_READINESS_CASE( "a socket closed while the waiter waits, its descriptor number reused", TestCloseWhileWaiting() );
    RUN_READINESS_CASE( "a waiter shut down refuses registrations and forgets the old ones", TestShutdown() );
    RUN_READINESS_CASE( "udp: a datagram wakes the task at once instead of at its next periodic pulse", TestUdpWakeLatency() );
    RUN_READINESS_CASE( "udp: opened and closed on the owner's thread while its task pulses it", TestUdpOpenCloseWhileTaskPulses() );
    RUN_READINESS_CASE( "tcp: connect, accept and echo between two tasks with the wake", TestTcpWakeLatency() );
    RUN_READINESS_CASE( "idle: the waiter returns from its wait about every 100 ms and wakes nobody", TestIdleWaiter() );
    {
        GUCEF_TESTFW_SUITE_SCOPE( "socket_send" );
        RUN_READINESS_CASE( "tcp: the cost of DataSentEvent per send, by kind of observer", TestDataSentEventCost() );
    }
}

/*-------------------------------------------------------------------------*/
