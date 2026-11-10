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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestTcpSocketThreads.h"

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
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------*/

/** A thread that makes no progress for this long is waiting on a lock the other thread will not give up */
static const UInt32 TcpThreadsStallTimeInMs = 5000;

/** How long the threads of the server case run side by side */
static const UInt32 TcpThreadsRunTimeInMs = 1500;

/** After a deadlock a lock wait ends after the lock timeout of gucefMT: the threads end after that */
static const UInt32 TcpThreadsThreadEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;

/** Rounds of connecting and closing a client socket from the test thread */
static const UInt32 TcpThreadsClientRounds = 25;

/** Clients of the server case */
static const UInt32 TcpThreadsServerClients = 6;

/*-------------------------------------------------------------------------*/

/** The server side of a case: counts accepted connections and keeps those that are open, events come on the pulse thread */
class CTcpThreadsServerWatcher : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > accepted;
    MT::CTAtomic< UInt32 > disconnected;

    explicit CTcpThreadsServerWatcher( CTCPServerSocket& server )
        : CORE::CObserver()
        , accepted( 0 )
        , disconnected( 0 )
        , m_lock()
        , m_open()
    {
        TEventCallback connectedCallback( this, &CTcpThreadsServerWatcher::OnClientConnected );
        SubscribeTo( &server, CTCPServerSocket::ClientConnectedEvent, connectedCallback );
        TEventCallback disconnectedCallback( this, &CTcpThreadsServerWatcher::OnClientDisconnected );
        SubscribeTo( &server, CTCPServerSocket::ClientDisconnectedEvent, disconnectedCallback );
    }

    virtual ~CTcpThreadsServerWatcher()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsServerWatcher";
        return classTypeName;
    }

    /** an open connection of the server, taken out of the list: null when there is none */
    CTCPServerConnection* TakeOpenConnection( void )
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_open.empty() )
            return GUCEF_NULL;
        CTCPServerConnection* connection = m_open.front();
        m_open.erase( m_open.begin() );
        return connection;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsServerWatcher > TEventCallback;

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
        CTCPServerConnection* connection = data->GetData().connection;
        {
            MT::CScopeMutex lock( m_lock );
            m_open.push_back( connection );
        }
        accepted.FetchAdd( 1 );
    }

    void OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        CTCPServerSocket::TClientDisconnectedEventData* data = static_cast< CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
        CTCPServerConnection* connection = data->GetData().connectionInfo.connection;
        disconnected.FetchAdd( 1 );

        MT::CScopeMutex lock( m_lock );
        GUCEF::vector< CTCPServerConnection* >::iterator i = m_open.begin();
        while ( i != m_open.end() )
        {
            if ( (*i) == connection )
            {
                m_open.erase( i );
                break;
            }
            ++i;
        }
    }

    MT::CMutex m_lock;
    GUCEF::vector< CTCPServerConnection* > m_open;
};

/*-------------------------------------------------------------------------*/

/**
 *  Closes every connection of the server from within the first data event of that connection, on the pulse thread,
 *  where the connection holds its own lock. It waits a little first, so that a thread that sends to all clients of
 *  the server waits for that lock under the lock of the server socket by then
 */
class CTcpThreadsCloseInEventWatcher : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > closedInEvent;

    explicit CTcpThreadsCloseInEventWatcher( CTCPServerSocket& server )
        : CORE::CObserver()
        , closedInEvent( 0 )
    {
        TEventCallback connectedCallback( this, &CTcpThreadsCloseInEventWatcher::OnClientConnected );
        SubscribeTo( &server, CTCPServerSocket::ClientConnectedEvent, connectedCallback );
    }

    virtual ~CTcpThreadsCloseInEventWatcher()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsCloseInEventWatcher";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsCloseInEventWatcher > TEventCallback;

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
        TEventCallback dataCallback( this, &CTcpThreadsCloseInEventWatcher::OnClientData );
        SubscribeTo( data->GetData().connection, CTCPServerConnection::DataRecievedEvent, dataCallback );
    }

    void OnClientData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        CTCPServerConnection* connection = static_cast< CTCPServerConnection* >( notifier );
        if ( !connection->IsActive() )
            return;
        MT::ThreadDelay( 20 );
        connection->Close();
        closedInEvent.FetchAdd( 1 );
    }
};

/*-------------------------------------------------------------------------*/

/** Closes the first connections of the server from within their ClientConnectedEvent, as a server that refuses a client does */
class CTcpThreadsCloseOnConnectWatcher : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > closedOnConnect;

    CTcpThreadsCloseOnConnectWatcher( CTCPServerSocket& server, UInt32 toClose )
        : CORE::CObserver()
        , closedOnConnect( 0 )
        , m_toClose( toClose )
    {
        TEventCallback connectedCallback( this, &CTcpThreadsCloseOnConnectWatcher::OnClientConnected );
        SubscribeTo( &server, CTCPServerSocket::ClientConnectedEvent, connectedCallback );
    }

    virtual ~CTcpThreadsCloseOnConnectWatcher()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsCloseOnConnectWatcher";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsCloseOnConnectWatcher > TEventCallback;

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( closedOnConnect.Load() >= m_toClose )
            return;
        CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
        data->GetData().connection->Close();
        closedOnConnect.FetchAdd( 1 );
    }

    UInt32 m_toClose;
};

/*-------------------------------------------------------------------------*/

/**
 *  Observes the end of one connection of the server: its DisconnectedEvent and the report of the close to the server
 *  socket (ClientDisconnectedEvent), in their order. Once the server socket heard of the close it may hand the
 *  connection object to a later client, whose observers would hear a DisconnectedEvent that came after it. Inside the
 *  DisconnectedEvent it also waits a while for the server socket to hand the object to a later client
 */
class CTcpThreadsReuseWatcher : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > disconnectEvents;
    MT::CTAtomic< UInt32 > reusedDuringEvent;
    MT::CTAtomic< UInt32 > reportedBeforeEvent;

    CTcpThreadsReuseWatcher( CTcpThreadsServerWatcher& watcher, CTCPServerSocket& server, CTCPServerConnection& connection )
        : CORE::CObserver()
        , disconnectEvents( 0 )
        , reusedDuringEvent( 0 )
        , reportedBeforeEvent( 0 )
        , m_watcher( watcher )
        , m_connection( &connection )
        , m_reported( 0 )
    {
        TEventCallback disconnectedCallback( this, &CTcpThreadsReuseWatcher::OnDisconnected );
        SubscribeTo( &connection, CTCPServerConnection::DisconnectedEvent, disconnectedCallback );
        TEventCallback reportedCallback( this, &CTcpThreadsReuseWatcher::OnReported );
        SubscribeTo( &server, CTCPServerSocket::ClientDisconnectedEvent, reportedCallback );
    }

    virtual ~CTcpThreadsReuseWatcher()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsReuseWatcher";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsReuseWatcher > TEventCallback;

    void OnReported( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        CTCPServerSocket::TClientDisconnectedEventData* data = static_cast< CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
        if ( data->GetData().connectionInfo.connection == m_connection )
            m_reported.FetchAdd( 1 );
    }

    void OnDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        disconnectEvents.FetchAdd( 1 );
        if ( 0 != m_reported.Load() )
            reportedBeforeEvent.FetchAdd( 1 );
        const UInt32 acceptedBefore = m_watcher.accepted.Load();
        const UInt64 startTicks = MT::PrecisionTickCount();
        while ( ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution() < 300 )
        {
            if ( m_watcher.accepted.Load() != acceptedBefore )
            {
                reusedDuringEvent.FetchAdd( 1 );
                break;
            }
            MT::ThreadDelay( 1 );
        }
    }

    CTcpThreadsServerWatcher& m_watcher;
    CTCPServerConnection* m_connection;
    MT::CTAtomic< UInt32 > m_reported;
};

/*-------------------------------------------------------------------------*/

/** Counts the DisconnectedEvent and DataRecievedEvent of one connection */
class CTcpThreadsConnectionEvents : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > disconnectEvents;
    MT::CTAtomic< UInt32 > dataEvents;

    explicit CTcpThreadsConnectionEvents( CTCPServerConnection& connection )
        : CORE::CObserver()
        , disconnectEvents( 0 )
        , dataEvents( 0 )
    {
        TEventCallback disconnectedCallback( this, &CTcpThreadsConnectionEvents::OnDisconnected );
        SubscribeTo( &connection, CTCPServerConnection::DisconnectedEvent, disconnectedCallback );
        TEventCallback dataCallback( this, &CTcpThreadsConnectionEvents::OnData );
        SubscribeTo( &connection, CTCPServerConnection::DataRecievedEvent, dataCallback );
    }

    virtual ~CTcpThreadsConnectionEvents()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsConnectionEvents";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsConnectionEvents > TEventCallback;

    void OnDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        disconnectEvents.FetchAdd( 1 );
    }

    void OnData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        dataEvents.FetchAdd( 1 );
    }
};

/*-------------------------------------------------------------------------*/

/** Closes the server socket from within its first ClientConnectedEvent, on the pulse thread */
class CTcpThreadsCloseServerOnConnect : public CORE::CObserver
{
    public:

    MT::CTAtomic< CTCPServerConnection* > accepted;

    explicit CTcpThreadsCloseServerOnConnect( CTCPServerSocket& server )
        : CORE::CObserver()
        , accepted( GUCEF_NULL )
        , m_server( server )
    {
        TEventCallback connectedCallback( this, &CTcpThreadsCloseServerOnConnect::OnClientConnected );
        SubscribeTo( &server, CTCPServerSocket::ClientConnectedEvent, connectedCallback );
    }

    virtual ~CTcpThreadsCloseServerOnConnect()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTcpThreadsCloseServerOnConnect";
        return classTypeName;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CTcpThreadsCloseServerOnConnect > TEventCallback;

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != accepted.Load() )
            return;
        CTCPServerSocket::TClientConnectedEventData* data = static_cast< CTCPServerSocket::TClientConnectedEventData* >( eventData );
        accepted.Store( data->GetData().connection );
        m_server.Close();
    }

    CTCPServerSocket& m_server;
};

/*-------------------------------------------------------------------------*/

/** Pumps the pulse generator on this thread until the condition holds or the time is up */
template< class TCondition >
static bool
PumpTcpUntil( CORE::PulseGeneratorPtr& pulseGenerator, const TCondition& condition, UInt32 timeoutInMs = 5000 )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    while ( !condition() && ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution() < timeoutInMs )
    {
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition();
}

/** Pumps the pulse generator on this thread a number of times */
static void
PumpTcp( CORE::PulseGeneratorPtr& pulseGenerator, UInt32 times )
{
    for ( UInt32 i=0; i<times; ++i )
    {
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 2 );
    }
}

struct SServerHasActive
{
    const CTCPServerSocket& server; UInt32 count;
    SServerHasActive( const CTCPServerSocket& s, UInt32 c ) : server( s ), count( c ) {}
    bool operator()( void ) const { return server.GetActiveCount() >= count; }
};

struct SClientActive
{
    const CTCPClientSocket& client;
    explicit SClientActive( const CTCPClientSocket& c ) : client( c ) {}
    bool operator()( void ) const { return client.IsActive(); }
};

struct SClientInactive
{
    const CTCPClientSocket& client;
    explicit SClientInactive( const CTCPClientSocket& c ) : client( c ) {}
    bool operator()( void ) const { return !client.IsActive(); }
};

struct SCounterAtLeast
{
    const MT::CTAtomic< UInt32 >& counter; UInt32 value;
    SCounterAtLeast( const MT::CTAtomic< UInt32 >& c, UInt32 v ) : counter( c ), value( v ) {}
    bool operator()( void ) const { return counter.Load() >= value; }
};

/** Closes the connection of the case on a thread of its own */
static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsCloseTargetMain( void* data ) GUCEF_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/** What the threads of a case share with the test thread. The threads may outlive a case that saw a deadlock, so it is
 *  made on the heap and given up then */
struct STcpThreadsCase
{
    CORE::PulseGeneratorPtr pulseGenerator;
    CTCPServerSocket* server;
    CTcpThreadsServerWatcher* watcher;
    CTcpThreadsCloseInEventWatcher* closeInEventWatcher;     /**< only for the cases that close from within the data event */
    GUCEF::vector< CTCPClientSocket* > clients;
    UInt16 port;

    MT::CTAtomic< bool > stopPulsing;
    MT::CTAtomic< bool > stopWorking;
    MT::CTAtomic< UInt64 > pulses;
    MT::CTAtomic< UInt32 > senderRounds;
    MT::CTAtomic< UInt32 > closerRounds;
    MT::CTAtomic< UInt32 > closes;
    MT::CTAtomic< UInt32 > exceptions;
    MT::CTAtomic< CTCPServerConnection* > sendTarget;  /**< the connection the single send thread sends on */
    MT::CTAtomic< UInt32 > sendResult;                  /**< of that send: 0 not done yet, 1 sent, 2 refused */

    explicit STcpThreadsCase( UInt32 maxConnections = 100 )
        : pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , server( GUCEF_NULL )
        , watcher( GUCEF_NULL )
        , closeInEventWatcher( GUCEF_NULL )
        , clients()
        , port( 0 )
        , stopPulsing( false )
        , stopWorking( false )
        , pulses( 0 )
        , senderRounds( 0 )
        , closerRounds( 0 )
        , closes( 0 )
        , exceptions( 0 )
        , sendTarget( GUCEF_NULL )
        , sendResult( 0 )
    {
        // listening before the pulse thread runs: what is set up here happens before that thread starts
        server = GUCEF_NEW CTCPServerSocket( pulseGenerator, false, maxConnections );
        watcher = GUCEF_NEW CTcpThreadsServerWatcher( *server );
        if ( server->ListenOnPort( 0 ) )
            port = server->GetBoundPort();
    }

    ~STcpThreadsCase()
    {
        for ( size_t i=0; i<clients.size(); ++i )
        {
            clients[ i ]->Close();
            GUCEF_DELETE clients[ i ];
        }
        clients.clear();
        GUCEF_DELETE closeInEventWatcher;
        closeInEventWatcher = GUCEF_NULL;
        GUCEF_DELETE watcher;
        watcher = GUCEF_NULL;
        server->Close();
        GUCEF_DELETE server;
        server = GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

/** Pulses the pulse generator of the case: the sockets of the case run on this thread */
static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsPulseMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    STcpThreadsCase& tcpCase = *static_cast< STcpThreadsCase* >( data );
    while ( !tcpCase.stopPulsing.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            tcpCase.pulseGenerator->ForceDirectPulse();
            tcpCase.pulses.FetchAdd( 1 );
        }
        catch ( ... )
        {
            tcpCase.exceptions.FetchAdd( 1 );
        }
        MT::ThreadDelay( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Sends to all clients of the server, round after round */
static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsSenderMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    STcpThreadsCase& tcpCase = *static_cast< STcpThreadsCase* >( data );
    while ( !tcpCase.stopWorking.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            tcpCase.server->SendToAllClients( "x", 1 );
            tcpCase.senderRounds.FetchAdd( 1 );
        }
        catch ( ... )
        {
            tcpCase.exceptions.FetchAdd( 1 );
            break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Sends once on the connection of the case */
static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsSingleSendMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    STcpThreadsCase& tcpCase = *static_cast< STcpThreadsCase* >( data );
    try
    {
        const bool sent = tcpCase.sendTarget.Load()->Send( "secret", 6 );
        tcpCase.sendResult.Store( sent ? 1 : 2 );
    }
    catch ( ... )
    {
        tcpCase.exceptions.FetchAdd( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** Closes connections of the server and has their clients connect again, round after round */
static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsCloserMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    STcpThreadsCase& tcpCase = *static_cast< STcpThreadsCase* >( data );
    size_t nextClient = 0;
    while ( !tcpCase.stopWorking.Load( MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            CTCPServerConnection* connection = tcpCase.watcher->TakeOpenConnection();
            if ( GUCEF_NULL != connection )
            {
                connection->Close();
                tcpCase.closes.FetchAdd( 1 );

                // a client that lost its connection connects again, which gives the server a new one to close
                CTCPClientSocket* client = tcpCase.clients[ nextClient % tcpCase.clients.size() ];
                ++nextClient;
                if ( !client->IsActive() )
                    client->ConnectTo( "127.0.0.1", tcpCase.port, false );
            }
            else
            {
                for ( size_t i=0; i<tcpCase.clients.size(); ++i )
                    if ( !tcpCase.clients[ i ]->IsActive() )
                        tcpCase.clients[ i ]->ConnectTo( "127.0.0.1", tcpCase.port, false );
                MT::ThreadDelay( 1 );
            }
            tcpCase.closerRounds.FetchAdd( 1 );
        }
        catch ( ... )
        {
            tcpCase.exceptions.FetchAdd( 1 );
            break;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static UInt32 GUCEF_CALLSPEC_PREFIX
TcpThreadsCloseTargetMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    STcpThreadsCase& tcpCase = *static_cast< STcpThreadsCase* >( data );
    try
    {
        tcpCase.sendTarget.Load()->Close();
        tcpCase.closes.FetchAdd( 1 );
    }
    catch ( ... )
    {
        tcpCase.exceptions.FetchAdd( 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

static MT::SThreadData*
StartTcpThreadsThread( MT::TThreadFunc threadMain, STcpThreadsCase* tcpCase )
{
    MT::SThreadData* thread = MT::ThreadDataReserve();
    if ( GUCEF_NULL == thread )
        return GUCEF_NULL;
    if ( 0 == MT::ThreadCreate( thread, threadMain, tcpCase ) )
    {
        MT::ThreadDataCleanup( thread );
        return GUCEF_NULL;
    }
    return thread;
}

/*-------------------------------------------------------------------------*/

/** Waits for the thread to end, returns whether it did; the thread data of a thread that did not end stays */
static bool
EndTcpThreadsThread( MT::SThreadData* thread )
{
    if ( GUCEF_NULL == thread )
        return true;
    if ( 1 != MT::ThreadWait( thread, TcpThreadsThreadEndTimeInMs ) )
        return false;
    MT::ThreadDataCleanup( thread );
    return true;
}

/*-------------------------------------------------------------------------*/

static UInt64
TcpThreadsElapsedMs( UInt64 startTicks )
{
    return ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

/**
 *  Watches the two working threads of a case for the run time: both have to keep making progress
 *  @return false when one of them made no progress for TcpThreadsStallTimeInMs
 */
static bool
WatchTcpThreads( STcpThreadsCase& tcpCase )
{
    const UInt64 startTicks = MT::PrecisionTickCount();
    UInt64 senderProgressTicks = startTicks;
    UInt64 closerProgressTicks = startTicks;
    UInt32 lastSenderRounds = 0;
    UInt32 lastCloserRounds = 0;
    while ( true )
    {
        MT::ThreadDelay( 10 );
        const UInt64 nowTicks = MT::PrecisionTickCount();
        const UInt32 senderRounds = tcpCase.senderRounds.Load();
        const UInt32 closerRounds = tcpCase.closerRounds.Load();
        if ( senderRounds != lastSenderRounds )
        {
            lastSenderRounds = senderRounds;
            senderProgressTicks = nowTicks;
        }
        if ( closerRounds != lastCloserRounds )
        {
            lastCloserRounds = closerRounds;
            closerProgressTicks = nowTicks;
        }
        if ( TcpThreadsElapsedMs( senderProgressTicks ) > TcpThreadsStallTimeInMs ||
             TcpThreadsElapsedMs( closerProgressTicks ) > TcpThreadsStallTimeInMs  )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TcpSocketThreads: a thread made no progress for " << TcpThreadsStallTimeInMs
                << " ms, sender rounds " << senderRounds << ", closer rounds " << closerRounds << ", closes "
                << tcpCase.closes.Load() );
            return false;
        }
        if ( TcpThreadsElapsedMs( startTicks ) > TcpThreadsRunTimeInMs )
            return true;
    }
}

/*-------------------------------------------------------------------------*/

void
PerformTcpSocketThreadsTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING TCP socket threads TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "TcpSocketThreads" );

    GUCEF_TESTFW_TESTCASE( "A TCP client socket pulsed on another thread is connected, asked whether it is active and closed from the test thread" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase();
            ASSERT_TRUE( 0 != tcpCase->port );
            CTCPClientSocket* client = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( client );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            UInt32 connectedRounds = 0;
            for ( UInt32 round=0; round<TcpThreadsClientRounds && GUCEF_NULL != pulser; ++round )
            {
                const UInt32 acceptedBefore = tcpCase->watcher->accepted.Load();
                client->ConnectTo( "127.0.0.1", tcpCase->port, false );

                // the connect completes on the pulse thread, while this thread looks at the socket
                const UInt64 startTicks = MT::PrecisionTickCount();
                while ( tcpCase->watcher->accepted.Load() == acceptedBefore && TcpThreadsElapsedMs( startTicks ) < 5000 )
                {
                    client->IsActive();
                    MT::ThreadDelay( 1 );
                }
                if ( tcpCase->watcher->accepted.Load() != acceptedBefore )
                    ++connectedRounds;

                client->Close();
            }

            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( TcpThreadsClientRounds == connectedRounds );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( pulserEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Connections of a server socket closed on one thread while another sends to all clients of that server: no deadlock" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase();
            ASSERT_TRUE( 0 != tcpCase->port );
            for ( UInt32 i=0; i<TcpThreadsServerClients; ++i )
                tcpCase->clients.push_back( GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false ) );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            // every client connected before the threads that use the connections start
            for ( UInt32 i=0; i<TcpThreadsServerClients; ++i )
                tcpCase->clients[ i ]->ConnectTo( "127.0.0.1", tcpCase->port, false );
            UInt64 startTicks = MT::PrecisionTickCount();
            while ( tcpCase->watcher->accepted.Load() < TcpThreadsServerClients && TcpThreadsElapsedMs( startTicks ) < 5000 )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( tcpCase->watcher->accepted.Load() >= TcpThreadsServerClients );

            MT::SThreadData* sender = StartTcpThreadsThread( &TcpThreadsSenderMain, tcpCase );
            MT::SThreadData* closer = StartTcpThreadsThread( &TcpThreadsCloserMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != sender );
            ASSERT_TRUE( GUCEF_NULL != closer );

            const bool stalled = GUCEF_NULL != sender && GUCEF_NULL != closer && !WatchTcpThreads( *tcpCase );

            tcpCase->stopWorking.Store( true, MT::MEMORDER_RELEASE );
            const bool senderEnded = EndTcpThreadsThread( sender );
            const bool closerEnded = EndTcpThreadsThread( closer );
            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );

            ASSERT_FALSE( stalled );
            ASSERT_TRUE( senderEnded );
            ASSERT_TRUE( closerEnded );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( tcpCase->closes.Load() > 0 );
            ASSERT_TRUE( tcpCase->senderRounds.Load() > 0 );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( senderEnded && closerEnded && pulserEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection closed from within its own data event while another thread sends to all clients of the server: no deadlock, the close is reported" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase();
            ASSERT_TRUE( 0 != tcpCase->port );
            tcpCase->closeInEventWatcher = GUCEF_NEW CTcpThreadsCloseInEventWatcher( *tcpCase->server );
            CTCPClientSocket* client = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( client );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            MT::SThreadData* sender = StartTcpThreadsThread( &TcpThreadsSenderMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );
            ASSERT_TRUE( GUCEF_NULL != sender );

            // every round: a client connects and sends, the server closes the connection from within the data event while
            // the sender thread waits for the lock of that connection under the lock of the server socket
            const UInt32 rounds = 5;
            bool stalled = false;
            UInt32 roundsDone = 0;
            for ( UInt32 round=0; round<rounds && !stalled && GUCEF_NULL != pulser && GUCEF_NULL != sender; ++round )
            {
                const UInt32 acceptedBefore = tcpCase->watcher->accepted.Load();
                const UInt32 disconnectedBefore = tcpCase->watcher->disconnected.Load();
                client->Close();
                client->ConnectTo( "127.0.0.1", tcpCase->port, false );

                UInt64 startTicks = MT::PrecisionTickCount();
                while ( tcpCase->watcher->accepted.Load() == acceptedBefore && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );
                startTicks = MT::PrecisionTickCount();
                while ( !client->Send( "ping", 4 ) && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );

                // the sender has to keep going, and the server socket has to hear of the close
                startTicks = MT::PrecisionTickCount();
                UInt64 senderProgressTicks = startTicks;
                UInt32 lastSenderRounds = tcpCase->senderRounds.Load();
                while ( tcpCase->watcher->disconnected.Load() == disconnectedBefore )
                {
                    MT::ThreadDelay( 10 );
                    const UInt32 senderRounds = tcpCase->senderRounds.Load();
                    if ( senderRounds != lastSenderRounds )
                    {
                        lastSenderRounds = senderRounds;
                        senderProgressTicks = MT::PrecisionTickCount();
                    }
                    if ( TcpThreadsElapsedMs( senderProgressTicks ) > TcpThreadsStallTimeInMs || TcpThreadsElapsedMs( startTicks ) > 2 * TcpThreadsStallTimeInMs )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "TcpSocketThreads: round " << round << " made no progress, sender rounds " << senderRounds
                            << ", closed in the data event " << tcpCase->closeInEventWatcher->closedInEvent.Load() << ", disconnects " << tcpCase->watcher->disconnected.Load() );
                        stalled = true;
                        break;
                    }
                }
                if ( !stalled )
                    ++roundsDone;
            }

            tcpCase->stopWorking.Store( true, MT::MEMORDER_RELEASE );
            const bool senderEnded = EndTcpThreadsThread( sender );
            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );

            ASSERT_FALSE( stalled );
            ASSERT_TRUE( rounds == roundsDone );
            ASSERT_TRUE( rounds == tcpCase->closeInEventWatcher->closedInEvent.Load() );
            ASSERT_TRUE( senderEnded );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( senderEnded && pulserEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection closed by a thread that holds its lock is closed at once and reported to the server socket once that lock is given up" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase();
            ASSERT_TRUE( 0 != tcpCase->port );
            CTCPClientSocket* client = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( client );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            client->ConnectTo( "127.0.0.1", tcpCase->port, false );
            UInt64 startTicks = MT::PrecisionTickCount();
            CTCPServerConnection* connection = GUCEF_NULL;
            while ( GUCEF_NULL == connection && TcpThreadsElapsedMs( startTicks ) < 5000 )
            {
                connection = tcpCase->watcher->TakeOpenConnection();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( GUCEF_NULL != connection );

            if ( GUCEF_NULL != connection )
            {
                const UInt32 disconnectedBefore = tcpCase->watcher->disconnected.Load();
                bool reportedUnderLock = true;
                {
                    // as a sender does that closes after its last frame: the connection cannot be handed to a later
                    // client while this thread holds its lock, which needs the server socket not to hear of the close yet
                    MT::CObjectScopeLock lock( connection->AsLockable() );
                    connection->Close();
                    ASSERT_FALSE( connection->IsActive() );
                    MT::ThreadDelay( 50 );
                    reportedUnderLock = tcpCase->watcher->disconnected.Load() != disconnectedBefore;
                }
                ASSERT_FALSE( reportedUnderLock );

                startTicks = MT::PrecisionTickCount();
                while ( tcpCase->watcher->disconnected.Load() == disconnectedBefore && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( disconnectedBefore + 1 == tcpCase->watcher->disconnected.Load() );

                // the connection object takes the next client
                const UInt32 acceptedBefore = tcpCase->watcher->accepted.Load();
                client->Close();
                client->ConnectTo( "127.0.0.1", tcpCase->port, false );
                startTicks = MT::PrecisionTickCount();
                while ( tcpCase->watcher->accepted.Load() == acceptedBefore && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( acceptedBefore + 1 == tcpCase->watcher->accepted.Load() );
            }

            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( pulserEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection closed from within its ClientConnectedEvent frees its place: a server of one connection accepts the next client" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase( 1 );
            ASSERT_TRUE( 0 != tcpCase->port );
            CTcpThreadsCloseOnConnectWatcher* closer = GUCEF_NEW CTcpThreadsCloseOnConnectWatcher( *tcpCase->server, 1 );
            CTCPClientSocket* refused = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            CTCPClientSocket* next = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( refused );
            tcpCase->clients.push_back( next );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            // the first client is closed while it is being accepted
            refused->ConnectTo( "127.0.0.1", tcpCase->port, false );
            UInt64 startTicks = MT::PrecisionTickCount();
            while ( ( 0 == closer->closedOnConnect.Load() || 0 == tcpCase->watcher->disconnected.Load() ) && TcpThreadsElapsedMs( startTicks ) < 5000 )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( 1 == closer->closedOnConnect.Load() && 1 == tcpCase->watcher->disconnected.Load() );
            MT::ThreadDelay( 50 );

            // its place is free again
            next->ConnectTo( "127.0.0.1", tcpCase->port, false );
            startTicks = MT::PrecisionTickCount();
            while ( tcpCase->watcher->accepted.Load() < 2 && TcpThreadsElapsedMs( startTicks ) < 5000 )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( 2 == tcpCase->watcher->accepted.Load() );

            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( pulserEnded )
            {
                GUCEF_DELETE closer;
                GUCEF_DELETE tcpCase;
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection closed on another thread than the server socket's is handed to a later client only after its DisconnectedEvent was delivered" )
        try
        {
            // one connection object: the next client has to get the object of the one that is closed
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase( 1 );
            ASSERT_TRUE( 0 != tcpCase->port );
            CTCPClientSocket* first = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            CTCPClientSocket* second = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( first );
            tcpCase->clients.push_back( second );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            first->ConnectTo( "127.0.0.1", tcpCase->port, false );
            UInt64 startTicks = MT::PrecisionTickCount();
            CTCPServerConnection* connection = GUCEF_NULL;
            while ( GUCEF_NULL == connection && TcpThreadsElapsedMs( startTicks ) < 5000 )
            {
                connection = tcpCase->watcher->TakeOpenConnection();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( GUCEF_NULL != connection );

            if ( GUCEF_NULL != connection )
            {
                CTcpThreadsReuseWatcher reuseWatcher( *tcpCase->watcher, *tcpCase->server, *connection );

                // the second client waits in the backlog of the server socket until the object is free again
                const UInt32 acceptedBefore = tcpCase->watcher->accepted.Load();
                second->ConnectTo( "127.0.0.1", tcpCase->port, false );
                MT::ThreadDelay( 50 );
                ASSERT_TRUE( acceptedBefore == tcpCase->watcher->accepted.Load() );

                // closed on this thread, while the pulse thread accepts
                connection->Close();
                ASSERT_TRUE( 1 == reuseWatcher.disconnectEvents.Load() );
                ASSERT_TRUE( 0 == reuseWatcher.reportedBeforeEvent.Load() );
                ASSERT_TRUE( 0 == reuseWatcher.reusedDuringEvent.Load() );
                reuseWatcher.UnsubscribeAllFromObserver();

                // and handed to the second client after that
                startTicks = MT::PrecisionTickCount();
                while ( tcpCase->watcher->accepted.Load() == acceptedBefore && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( acceptedBefore + 1 == tcpCase->watcher->accepted.Load() );
            }

            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( pulserEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A send that waited for the lock of a connection that was closed meanwhile sends nothing, also when the system gave the number of the closed socket to a new one" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase();
            ASSERT_TRUE( 0 != tcpCase->port );
            CTCPClientSocket* client = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( client );

            MT::SThreadData* pulser = StartTcpThreadsThread( &TcpThreadsPulseMain, tcpCase );
            ASSERT_TRUE( GUCEF_NULL != pulser );

            client->ConnectTo( "127.0.0.1", tcpCase->port, false );
            UInt64 startTicks = MT::PrecisionTickCount();
            CTCPServerConnection* connection = GUCEF_NULL;
            while ( GUCEF_NULL == connection && TcpThreadsElapsedMs( startTicks ) < 5000 )
            {
                connection = tcpCase->watcher->TakeOpenConnection();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( GUCEF_NULL != connection );

            MT::SThreadData* sender = GUCEF_NULL;
            CORE::PulseGeneratorPtr newSocketPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CTCPClientSocket* newSocket = GUCEF_NULL;
            if ( GUCEF_NULL != connection )
            {
                MT::CObjectScopeLock lock( connection->AsLockable() );

                // the send finds the connection active and waits for its lock
                tcpCase->sendTarget.Store( connection );
                sender = StartTcpThreadsThread( &TcpThreadsSingleSendMain, tcpCase );
                ASSERT_TRUE( GUCEF_NULL != sender );
                MT::ThreadDelay( 100 );

                // the connection closes while the send waits, and the system hands the lowest free descriptor number, the
                // one of the closed socket, to the next socket that is made: one that connects to the server as well
                connection->Close();
                newSocket = GUCEF_NEW CTCPClientSocket( newSocketPulseGenerator, false );
                newSocket->ConnectTo( "127.0.0.1", tcpCase->port, false );
                startTicks = MT::PrecisionTickCount();
                while ( !newSocket->IsActive() && TcpThreadsElapsedMs( startTicks ) < 5000 )
                {
                    newSocketPulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( newSocket->IsActive() );
            }

            // now the send gets the lock
            startTicks = MT::PrecisionTickCount();
            while ( 0 == tcpCase->sendResult.Load() && TcpThreadsElapsedMs( startTicks ) < 5000 )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( 2 == tcpCase->sendResult.Load() );
            const bool senderEnded = EndTcpThreadsThread( sender );
            ASSERT_TRUE( senderEnded );

            if ( GUCEF_NULL != newSocket )
            {
                newSocket->Close();
                GUCEF_DELETE newSocket;
            }
            tcpCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulserEnded = EndTcpThreadsThread( pulser );
            ASSERT_TRUE( pulserEnded );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );

            if ( pulserEnded && senderEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection closed between two pulses, then the server socket closed and listening again: the next client of that connection object stays active and gets its events; the first close is told once" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CTCPServerSocket server( pulseGenerator, false, 1 );
            ASSERT_TRUE( server.ListenOnPort( 0 ) );
            CTCPClientSocket first( pulseGenerator, false );
            ASSERT_TRUE( first.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SServerHasActive( server, 1 ) ) );
            CTCPServerConnection* connection = server.GetConnection( 0 );
            CTcpThreadsConnectionEvents events( *connection );

            // closed by the user between two pulses, and the server socket closes before the next pulse
            connection->Close();
            ASSERT_TRUE( 1 == events.disconnectEvents.Load() );
            server.Close();
            ASSERT_TRUE( 1 == events.disconnectEvents.Load() );
            ASSERT_TRUE( 0 == server.GetActiveCount() );
            first.Close();
            PumpTcp( pulseGenerator, 5 );

            // listening again: the next client gets the same connection object
            ASSERT_TRUE( server.ListenOnPort( 0 ) );
            CTCPClientSocket second( pulseGenerator, false );
            ASSERT_TRUE( second.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SClientActive( second ) ) );
            PumpTcp( pulseGenerator, 20 );
            ASSERT_TRUE( connection->IsActive() );
            ASSERT_TRUE( 1 == server.GetActiveCount() );

            // its data and its end arrive
            ASSERT_TRUE( second.Send( "ping", 4 ) );
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SCounterAtLeast( events.dataEvents, 1 ) ) );
            second.Close();
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SCounterAtLeast( events.disconnectEvents, 2 ) ) );
            PumpTcp( pulseGenerator, 5 );
            ASSERT_TRUE( 2 == events.disconnectEvents.Load() );
            ASSERT_TRUE( 0 == server.GetActiveCount() );
            server.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A server socket closed from within its ClientConnectedEvent closes the connection it was accepting: not left open and active on a closed server" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CTCPServerSocket server( pulseGenerator, false, 2 );
            CTcpThreadsCloseServerOnConnect closer( server );
            ASSERT_TRUE( server.ListenOnPort( 0 ) );
            CTCPClientSocket client( pulseGenerator, false );
            ASSERT_TRUE( client.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            const UInt64 startTicks = MT::PrecisionTickCount();
            while ( GUCEF_NULL == closer.accepted.Load() && TcpThreadsElapsedMs( startTicks ) < 5000 )
            {
                pulseGenerator->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            CTCPServerConnection* connection = closer.accepted.Load();
            ASSERT_TRUE( GUCEF_NULL != connection );
            PumpTcp( pulseGenerator, 5 );
            ASSERT_FALSE( server.IsActive() );
            ASSERT_TRUE( 0 == server.GetActiveCount() );
            if ( GUCEF_NULL != connection )
                ASSERT_FALSE( connection->IsActive() );

            // the client hears of the end
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SClientInactive( client ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lowering the maximum of connections with dropConnections closes as many open connections as are over the new maximum" )
        try
        {
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
            CTCPServerSocket server( pulseGenerator, false, 3 );
            ASSERT_TRUE( server.ListenOnPort( 0 ) );
            CTCPClientSocket a( pulseGenerator, false );
            CTCPClientSocket b( pulseGenerator, false );
            CTCPClientSocket c( pulseGenerator, false );
            ASSERT_TRUE( a.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            ASSERT_TRUE( b.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            ASSERT_TRUE( c.ConnectTo( "127.0.0.1", server.GetBoundPort(), false ) );
            ASSERT_TRUE( PumpTcpUntil( pulseGenerator, SServerHasActive( server, 3 ) ) );

            server.SetMaxConnections( 1, true );
            UInt32 open = 0;
            for ( UInt32 i=0; i<3; ++i )
                if ( server.GetConnection( i )->IsActive() )
                    ++open;
            ASSERT_TRUE( 1 == open );
            PumpTcp( pulseGenerator, 5 );
            ASSERT_TRUE( 1 == server.GetActiveCount() );
            server.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A connection whose close another thread is still telling the server socket when that closes and listens again is not told twice and not handed to the next client before that" )
        try
        {
            STcpThreadsCase* tcpCase = GUCEF_NEW STcpThreadsCase( 1 );
            ASSERT_TRUE( 0 != tcpCase->port );
            CTCPClientSocket* first = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
            tcpCase->clients.push_back( first );
            ASSERT_TRUE( first->ConnectTo( "127.0.0.1", tcpCase->port, false ) );
            ASSERT_TRUE( PumpTcpUntil( tcpCase->pulseGenerator, SServerHasActive( *tcpCase->server, 1 ) ) );
            CTCPServerConnection* connection = tcpCase->server->GetConnection( 0 );
            CTcpThreadsConnectionEvents events( *connection );

            // another thread closes the connection, and tells the server socket last, under the lock of the server
            // socket, which this thread holds meanwhile: it is still telling while the server socket closes and listens
            // again. This thread pulses
            MT::SThreadData* closerThread = GUCEF_NULL;
            bool closerEnded = false;
            {
                MT::CObjectScopeLock serverLock( tcpCase->server->AsLockable() );
                tcpCase->sendTarget.Store( connection );
                closerThread = StartTcpThreadsThread( &TcpThreadsCloseTargetMain, tcpCase );
                ASSERT_TRUE( GUCEF_NULL != closerThread );
                UInt64 startTicks = MT::PrecisionTickCount();
                while ( 0 == events.disconnectEvents.Load() && TcpThreadsElapsedMs( startTicks ) < 5000 )
                    MT::ThreadDelay( 1 );
                ASSERT_TRUE( 1 == events.disconnectEvents.Load() );
                MT::ThreadDelay( 50 );

                tcpCase->server->Close();
                ASSERT_TRUE( 1 == events.disconnectEvents.Load() );
                ASSERT_TRUE( tcpCase->server->ListenOnPort( tcpCase->port ) );
                CTCPClientSocket* second = GUCEF_NEW CTCPClientSocket( tcpCase->pulseGenerator, false );
                tcpCase->clients.push_back( second );
                ASSERT_TRUE( second->ConnectTo( "127.0.0.1", tcpCase->port, false ) );
                PumpTcp( tcpCase->pulseGenerator, 50 );
                ASSERT_FALSE( connection->IsActive() );
                ASSERT_TRUE( 1 == tcpCase->watcher->accepted.Load() );
                ASSERT_TRUE( 0 == tcpCase->closes.Load() );
            }

            // once the close was told, the object goes to the next client
            closerEnded = EndTcpThreadsThread( closerThread );
            ASSERT_TRUE( closerEnded && 1 == tcpCase->closes.Load() );
            ASSERT_TRUE( PumpTcpUntil( tcpCase->pulseGenerator, SServerHasActive( *tcpCase->server, 1 ) ) );
            ASSERT_TRUE( connection->IsActive() && 2 == tcpCase->watcher->accepted.Load() );
            ASSERT_TRUE( 1 == events.disconnectEvents.Load() );
            ASSERT_TRUE( 0 == tcpCase->exceptions.Load() );
            events.UnsubscribeAllFromObserver();

            if ( closerEnded )
                GUCEF_DELETE tcpCase;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
