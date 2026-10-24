/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
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

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <fcntl.h>
  #include <signal.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define GUCEF_COMCORE_TEST_POSIX_SOCKETS
#endif

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#include "TestCode_Pump.h"

#include "TestCode_ClientServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

static const UInt16 TestServerPort = 47610;

#ifdef GUCEF_COMCORE_TEST_POSIX_SOCKETS

/** How often SIGPIPE came while the counting handler was in place */
static volatile sig_atomic_t g_sigPipeCount = 0;

extern "C" void
CountSigPipe( int signalNumber )
{
    ++g_sigPipeCount;
}

/**
 *  The descriptor of the socket of this process with the given local port and peer port: the socket objects
 *  do not give theirs out
 */
static int
FindConnectedDescriptor( UInt16 localPort, UInt16 peerPort )
{
    for ( int descriptor=0; descriptor<4096; ++descriptor )
    {
        struct sockaddr_in local;
        struct sockaddr_in peer;
        socklen_t localSize = sizeof( local );
        socklen_t peerSize = sizeof( peer );
        if ( 0 == getsockname( descriptor, (struct sockaddr*) &local, &localSize ) && AF_INET == local.sin_family &&
             0 == getpeername( descriptor, (struct sockaddr*) &peer, &peerSize ) &&
             localPort == ntohs( local.sin_port ) && peerPort == ntohs( peer.sin_port ) )
            return descriptor;
    }
    return -1;
}

#endif /* GUCEF_COMCORE_TEST_POSIX_SOCKETS ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A TCP server socket and a TCP client socket which talk to each other, and the record of what they did.
 *  The server answers every message it gets with the reply it was given.
 */
class CTestTCPClientServer : public CORE::CObserver
{
    public:

    CTestTCPClientServer( const CORE::CString& reply )
        : CORE::CObserver()
        , m_serverSock( false )
        , m_clientSock( false )
        , m_serverConnection( GUCEF_NULL )
        , m_reply( reply )
        , m_clientConnected( false )
        , m_clientDisconnected( false )
        , m_serverSawClient( false )
        , m_serverSawDisconnect( false )
        , m_serverReceived()
        , m_clientReceived()
    {GUCEF_TRACE;

        SubscribeTo( &m_serverSock );
        SubscribeTo( &m_clientSock );
    }

    virtual ~CTestTCPClientServer()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestTCPClientServer";
        return classTypeName;
    }

    bool Start( UInt16 port )
    {GUCEF_TRACE;

        return m_serverSock.ListenOnPort( port ) && m_clientSock.ConnectTo( "127.0.0.1", port );
    }

    CTCPClientSocket& Client( void ) { return m_clientSock; }

    /** The connection the server accepted, GUCEF_NULL before it did and after it closed */
    CTCPServerConnection* ServerConnection( void ) { return m_serverConnection; }
    CTCPServerSocket& Server( void ) { return m_serverSock; }

    bool m_clientConnected;
    bool m_clientDisconnected;
    bool m_serverSawClient;
    bool m_serverSawDisconnect;
    CORE::CString m_serverReceived;
    CORE::CString m_clientReceived;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( notifier == &m_serverSock )
        {
            if ( CTCPServerSocket::ClientConnectedEvent == eventid )
            {
                CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventdata );
                m_serverConnection = data->GetData().connection;
                SubscribeTo( m_serverConnection );
                m_serverSawClient = true;
            }
            else
            if ( CTCPServerSocket::ClientDisconnectedEvent == eventid )
            {
                m_serverConnection = GUCEF_NULL;
                m_serverSawDisconnect = true;
            }
        }
        else
        if ( notifier == &m_clientSock )
        {
            if ( CTCPClientSocket::ConnectedEvent == eventid )
            {
                m_clientConnected = true;
            }
            else
            if ( CTCPClientSocket::DisconnectedEvent == eventid )
            {
                m_clientDisconnected = true;
            }
            else
            if ( CTCPClientSocket::DataRecievedEvent == eventid )
            {
                CTCPClientSocket::TDataRecievedEventData* data = static_cast< CTCPClientSocket::TDataRecievedEventData* >( eventdata );
                m_clientReceived.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
            }
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventid && notifier == m_serverConnection )
        {
            CTCPConnection::TDataRecievedEventData* data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventdata );
            m_serverReceived.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
            m_serverConnection->Send( m_reply.C_String(), m_reply.ByteSize() - 1 );    // ByteSize includes the null terminator
        }
    }

    private:

    CTCPServerSocket m_serverSock;
    CTCPClientSocket m_clientSock;
    CTCPServerConnection* m_serverConnection;
    CORE::CString m_reply;
};

/*-------------------------------------------------------------------------*/

/**
 *  A server that greets every client the moment it accepts it, the way IRC and SMTP servers do, and a client
 *  that records in which order its events come
 */
class CGreetingServerAndClient : public CORE::CObserver
{
    public:

    CGreetingServerAndClient( bool serverFirst )
        : CORE::CObserver()
        , m_firstServerSock( GUCEF_NULL )
        , m_clientSock( GUCEF_NULL )
        , m_secondServerSock( GUCEF_NULL )
        , m_connectedSeen( false )
        , m_dataBeforeConnected( false )
        , m_dataSeen( false )
    {GUCEF_TRACE;

        // The order of creation is the order the pulse generator serves them in
        if ( serverFirst )
            m_firstServerSock = GUCEF_NEW CTCPServerSocket( false );
        m_clientSock = GUCEF_NEW CTCPClientSocket( false );
        if ( !serverFirst )
            m_secondServerSock = GUCEF_NEW CTCPServerSocket( false );
        SubscribeTo( Server() );
        SubscribeTo( m_clientSock );
    }

    virtual ~CGreetingServerAndClient()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        GUCEF_DELETE m_clientSock;
        GUCEF_DELETE m_firstServerSock;
        GUCEF_DELETE m_secondServerSock;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CGreetingServerAndClient";
        return classTypeName;
    }

    CTCPServerSocket* Server( void ) { return GUCEF_NULL != m_firstServerSock ? m_firstServerSock : m_secondServerSock; }
    CTCPClientSocket* Client( void ) { return m_clientSock; }

    bool m_connectedSeen;
    bool m_dataBeforeConnected;
    bool m_dataSeen;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( notifier == Server() && CTCPServerSocket::ClientConnectedEvent == eventid )
        {
            CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventdata );
            data->GetData().connection->Send( "hello\r\n", 7 );
        }
        else
        if ( notifier == m_clientSock )
        {
            if ( CTCPClientSocket::ConnectedEvent == eventid )
                m_connectedSeen = true;
            else
            if ( CTCPClientSocket::DataRecievedEvent == eventid )
            {
                m_dataSeen = true;
                if ( !m_connectedSeen )
                    m_dataBeforeConnected = true;
            }
        }
    }

    private:

    CTCPServerSocket* m_firstServerSock;
    CTCPClientSocket* m_clientSock;
    CTCPServerSocket* m_secondServerSock;
};

/*-------------------------------------------------------------------------*/

/**
 *  Records what a UDP socket receives
 */
class CUdpReceiveRecorder : public CORE::CObserver
{
    public:

    CUdpReceiveRecorder( void )
        : CORE::CObserver()
        , m_received()
        , m_receivedSomething( false )
    {GUCEF_TRACE;

    }

    virtual ~CUdpReceiveRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CUdpReceiveRecorder";
        return classTypeName;
    }

    CORE::CString m_received;
    bool m_receivedSomething;
    CIPv4Address m_source;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( CUDPSocket::UDPPacketsRecievedEvent == eventid && GUCEF_NULL != eventdata )
        {
            const CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< CUDPSocket::UDPPacketsRecievedEventData* >( eventdata )->GetData();
            for ( UInt32 i=0; i<data.packetsReceived; ++i )
            {
                const CORE::CDynamicBuffer& buffer = data.packets[ i ].dataBuffer.GetData();
                m_received.Append( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
                m_source = data.packets[ i ].sourceAddress;
                m_receivedSomething = true;
            }
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformClientServerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ClientServer" );

    GUCEF_TESTFW_TESTCASE( "Test: A TCP client connects to a TCP server, they exchange data and the client disconnects" )
        try
        {
            CTestTCPClientServer test( "pong" );
            ASSERT_TRUE( test.Start( TestServerPort ) );

            ASSERT_TRUE( PumpUntil( [&test]() { return test.m_clientConnected && test.m_serverSawClient; }, 5000 ) );

            ASSERT_TRUE( test.Client().Send( "ping", 4 ) );
            ASSERT_TRUE( PumpUntil( [&test]() { return test.m_serverReceived == "ping"; }, 5000 ) );
            ASSERT_TRUE( PumpUntil( [&test]() { return test.m_clientReceived == "pong"; }, 5000 ) );

            test.Client().Close();
            ASSERT_TRUE( PumpUntil( [&test]() { return test.m_serverSawDisconnect; }, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A TCP client reports an error when nothing is listening" )
        try
        {
            CTCPClientSocket client( false );
            bool errorOrClosed = false;
            struct CErrorObserver : public CORE::CObserver
            {
                bool* flag;
                CErrorObserver( bool* f ) : flag( f ) {}
                virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString n = "CErrorObserver"; return n; }
                virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& eventid, CORE::CICloneable* ) GUCEF_VIRTUAL_OVERRIDE
                {
                    if ( CTCPClientSocket::SocketErrorEvent == eventid || CTCPClientSocket::DisconnectedEvent == eventid )
                        *flag = true;
                }
            } observer( &errorOrClosed );
            observer.SubscribeTo( &client );

            // the port is not used by any test
            client.ConnectTo( "127.0.0.1", TestServerPort + 50 );
            ASSERT_TRUE( PumpUntil( [&errorOrClosed]() { return errorOrClosed; }, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A TCP client reports the connection before the data of a server that talks first" )
        try
        {
            for ( int serverFirst=0; serverFirst<2; ++serverFirst )
            {
                CGreetingServerAndClient test( 0 != serverFirst );
                UInt16 port = (UInt16)( TestServerPort + 2 + serverFirst );
                ASSERT_TRUE( test.Server()->ListenOnPort( port ) );
                ASSERT_TRUE( test.Client()->ConnectTo( "127.0.0.1", port ) );

                ASSERT_TRUE( PumpUntilTrue( test.m_dataSeen, 5000 ) );
                ASSERT_TRUE( test.m_connectedSeen );
                ASSERT_FALSE( test.m_dataBeforeConnected );
                test.Client()->Close();
                test.Server()->Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A UDP socket opens on a local address given as text and receives there" )
        try
        {
            CUDPSocket receiver( false );
            CUdpReceiveRecorder recorder;
            recorder.SubscribeTo( &receiver );
            ASSERT_TRUE( receiver.Open( "127.0.0.1", (UInt16)( TestServerPort + 4 ) ) );
            ASSERT_TRUE( receiver.IsActive() );
            ASSERT_TRUE( TestServerPort + 4 == receiver.GetPort() );

            CUDPSocket sender( false );
            ASSERT_TRUE( sender.Open( (UInt16) 0 ) );
            ASSERT_TRUE( 5 == sender.SendPacketTo( CIPv4Address( "127.0.0.1", (UInt16)( TestServerPort + 4 ) ), "hello", 5 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.m_receivedSomething, 5000 ) );
            ASSERT_TRUE( recorder.m_received == "hello" );

            // The receiver knows where it came from, so it can answer
            ASSERT_TRUE( recorder.m_source.AddressAsString() == "127.0.0.1" );
            ASSERT_TRUE( recorder.m_source.GetPortInHostByteOrder() == sender.GetPort() );

            // A local address that is not there cannot be opened
            CUDPSocket nowhere( false );
            ASSERT_FALSE( nowhere.Open( "no.such.host.invalid", (UInt16)( TestServerPort + 5 ) ) );

            sender.Close();
            receiver.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A UDP socket whose receive buffers grow and shrink receives full size packets intact" )
        try
        {
            CUDPSocket receiver( false );
            CUdpReceiveRecorder recorder;
            recorder.SubscribeTo( &receiver );

            // More buffers than before, and other sizes: the memory the receive buffers are views on moves
            ASSERT_TRUE( receiver.SetNrOfReceiveBuffers( 16, 64 ) );
            ASSERT_TRUE( receiver.SetNrOfReceiveBuffers( 40, 9216 ) );
            ASSERT_TRUE( receiver.SetNrOfReceiveBuffers( 3, 9216 ) );
            ASSERT_TRUE( receiver.Open( "127.0.0.1", (UInt16)( TestServerPort + 10 ) ) );

            char packet[ 9000 ];
            for ( UInt32 i=0; i<sizeof( packet ); ++i )
                packet[ i ] = (char)( 'a' + i % 26 );
            packet[ sizeof( packet ) - 1 ] = '\0';

            CUDPSocket sender( false );
            ASSERT_TRUE( sender.Open( (UInt16) 0 ) );
            ASSERT_TRUE( sizeof( packet ) == sender.SendPacketTo( CIPv4Address( "127.0.0.1", (UInt16)( TestServerPort + 10 ) ), packet, sizeof( packet ) ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.m_receivedSomething, 2000 ) );
            ASSERT_TRUE( recorder.m_received == CORE::CString( packet ) );

            sender.Close();
            receiver.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A closed UDP socket is inactive and closing it again leaves the socket opened after it alone" )
        try
        {
            CUDPSocket first( false );
            ASSERT_TRUE( first.Open( (UInt16) 0 ) );
            first.Close();
            ASSERT_FALSE( first.IsActive() );

            // The system hands the freed descriptor to the next socket
            CUDPSocket receiver( false );
            CUdpReceiveRecorder recorder;
            recorder.SubscribeTo( &receiver );
            ASSERT_TRUE( receiver.Open( "127.0.0.1", (UInt16)( TestServerPort + 6 ) ) );

            first.Close();
            ASSERT_TRUE( receiver.IsActive() );

            CUDPSocket sender( false );
            ASSERT_TRUE( sender.Open( (UInt16) 0 ) );
            ASSERT_TRUE( 5 == sender.SendPacketTo( CIPv4Address( "127.0.0.1", (UInt16)( TestServerPort + 6 ) ), "again", 5 ) );
            ASSERT_TRUE( PumpUntilTrue( recorder.m_receivedSomething, 2000 ) );
            ASSERT_TRUE( recorder.m_received == "again" );

            sender.Close();
            receiver.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: SetBlockingMode switches a socket to non-blocking and back" )
        #ifndef GUCEF_COMCORE_TEST_POSIX_SOCKETS
        GUCEF_TESTFW_SKIP( "The blocking mode of a socket is read with fcntl()" );
        #else
        try
        {
            int descriptor = socket( AF_INET, SOCK_STREAM, 0 );
            ASSERT_TRUE( descriptor >= 0 );
            if ( descriptor >= 0 )
            {
                ASSERT_TRUE( SetBlockingMode( descriptor, false ) );
                ASSERT_TRUE( 0 != ( fcntl( descriptor, F_GETFL ) & O_NONBLOCK ) );
                ASSERT_TRUE( SetBlockingMode( descriptor, true ) );
                ASSERT_TRUE( 0 == ( fcntl( descriptor, F_GETFL ) & O_NONBLOCK ) );
                close( descriptor );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        #endif
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A connection a non-blocking TCP server accepts is non-blocking too" )
        #ifndef GUCEF_COMCORE_TEST_POSIX_SOCKETS
        GUCEF_TESTFW_SKIP( "The blocking mode of a socket is read with fcntl()" );
        #else
        try
        {
            CTestTCPClientServer test( "reply" );
            ASSERT_TRUE( test.Start( (UInt16)( TestServerPort + 7 ) ) );
            ASSERT_TRUE( PumpUntilTrue( test.m_serverSawClient, 5000 ) );
            CTCPServerConnection* connection = test.ServerConnection();
            ASSERT_TRUE( GUCEF_NULL != connection );
            if ( GUCEF_NULL != connection )
            {
                int descriptor = FindConnectedDescriptor( (UInt16)( TestServerPort + 7 ), connection->GetRemoteTCPPort() );
                ASSERT_TRUE( descriptor >= 0 );
                int flags = descriptor >= 0 ? fcntl( descriptor, F_GETFL ) : -1;
                ASSERT_TRUE( flags >= 0 && 0 != ( flags & O_NONBLOCK ) );
            }
            test.Client().Close();
            test.Server().Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        #endif
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sending to a TCP peer that closed is an error, not a SIGPIPE" )
        #ifndef GUCEF_COMCORE_TEST_POSIX_SOCKETS
        GUCEF_TESTFW_SKIP( "There is no SIGPIPE on this platform" );
        #else
        // The default action of SIGPIPE ends the process: count it instead, so the test can report it
        struct sigaction counting;
        struct sigaction previous;
        memset( &counting, 0, sizeof( counting ) );
        counting.sa_handler = CountSigPipe;
        sigemptyset( &counting.sa_mask );
        sigaction( SIGPIPE, &counting, &previous );
        g_sigPipeCount = 0;
        try
        {
            // The server writes to a client that is gone; no pulses in between, so the server does not know yet
            {
                CTestTCPClientServer test( "reply" );
                ASSERT_TRUE( test.Start( (UInt16)( TestServerPort + 8 ) ) );
                ASSERT_TRUE( PumpUntilTrue( test.m_serverSawClient, 5000 ) && PumpUntilTrue( test.m_clientConnected, 5000 ) );
                CTCPServerConnection* connection = test.ServerConnection();
                ASSERT_TRUE( GUCEF_NULL != connection );
                test.Client().Close();
                MT::ThreadDelay( 50 );
                for ( UInt32 i=0; i<5 && GUCEF_NULL != connection; ++i )
                {
                    connection->Send( "data", 4 );
                    MT::ThreadDelay( 20 );
                }
                ASSERT_TRUE( 0 == g_sigPipeCount );
                test.Server().Close();
            }

            // The client writes to a server connection that is gone
            {
                CTestTCPClientServer test( "reply" );
                ASSERT_TRUE( test.Start( (UInt16)( TestServerPort + 9 ) ) );
                ASSERT_TRUE( PumpUntilTrue( test.m_serverSawClient, 5000 ) && PumpUntilTrue( test.m_clientConnected, 5000 ) );
                CTCPServerConnection* connection = test.ServerConnection();
                ASSERT_TRUE( GUCEF_NULL != connection );
                if ( GUCEF_NULL != connection )
                    connection->Close();
                MT::ThreadDelay( 50 );
                for ( UInt32 i=0; i<5; ++i )
                {
                    test.Client().Send( "data", 4 );
                    MT::ThreadDelay( 20 );
                }
                ASSERT_TRUE( 0 == g_sigPipeCount );
                test.Client().Close();
                test.Server().Close();
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
        sigaction( SIGPIPE, &previous, GUCEF_NULL );
        #endif
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
