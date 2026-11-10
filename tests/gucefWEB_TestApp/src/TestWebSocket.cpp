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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_COMCORE_TESTAPP_TESTNOTLSPROVIDER_H
#include "../../gucefCOMCORE_TestApp/include/TestNoTlsProvider.h"
#define GUCEF_COMCORE_TESTAPP_TESTNOTLSPROVIDER_H
#endif /* GUCEF_COMCORE_TESTAPP_TESTNOTLSPROVIDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "TestWebSocket.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

static const UInt32 WAIT_TIMEOUT_IN_MS = 5000;

/*-------------------------------------------------------------------------*/

/**
 *  Drives the application pulse generator, which is what makes the sockets do their work,
 *  until the counter reached the expected value or the timeout expired
 */
static bool
PumpUntil( const UInt32& counter, UInt32 expected, UInt32 timeoutInMs = WAIT_TIMEOUT_IN_MS )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( counter < expected )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
            return false;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

static void
PumpFor( UInt32 durationInMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs < durationInMs )
    {
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Records the events of a web socket server and/or client, optionally acting as an echo server
 */
class CWsRecorder : public CORE::CObserver
{
    public:

    CWsRecorder( void )
        : serverConnects( 0 ), serverDisconnects( 0 ), serverTexts( 0 ), serverBinaries( 0 )
        , clientConnects( 0 ), clientCloses( 0 ), clientErrors( 0 ), clientTexts( 0 ), clientBinaries( 0 )
        , echoServer( false ), serverToScheduleOnText( GUCEF_NULL )
    {}

    void WatchServer( WEB::CWebSocketServer& server ) { SubscribeTo( &server ); }
    void WatchClient( WEB::CWebSocketClient& client ) { SubscribeTo( &client ); }

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata        ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == WEB::CWebSocketServer::ClientConnectedEvent )
        {
            const WEB::TWebSocketConnectionPtr& connection = static_cast< WEB::TWebSocketServerConnectedEventData* >( eventdata )->GetData();
            serverConnections.push_back( connection );
            serverPeers.push_back( connection->GetPeerInfo() );
            ++serverConnects;
        }
        else
        if ( eventid == WEB::CWebSocketServer::ClientDisconnectedEvent )
        {
            serverCloses.push_back( static_cast< WEB::TWebSocketServerClosedEventData* >( eventdata )->GetData().closeInfo );
            ++serverDisconnects;
        }
        else
        if ( eventid == WEB::CWebSocketServer::TextMessageEvent )
        {
            WEB::SWebSocketServerTextMessage& message = static_cast< WEB::TWebSocketServerTextMessageEventData* >( eventdata )->GetData();
            serverTextMessages.push_back( message.text );
            ++serverTexts;
            if ( echoServer )
                message.connection->SendText( message.text );
            if ( GUCEF_NULL != serverToScheduleOnText )
            {
                // The way to get a notifier deleted from within its own notification: it is deleted once the dispatch
                // loop has completed and NotifyObservers() then returns false to its caller
                WEB::CWebSocketServer* server = serverToScheduleOnText;
                serverToScheduleOnText = GUCEF_NULL;
                server->ScheduleForDestruction();
            }
        }
        else
        if ( eventid == WEB::CWebSocketServer::BinaryMessageEvent )
        {
            WEB::SWebSocketServerBinaryMessage& message = static_cast< WEB::TWebSocketServerBinaryMessageEventData* >( eventdata )->GetData();
            serverBinaryMessages.push_back( message.data );
            ++serverBinaries;
            if ( echoServer )
                message.connection->SendBinary( message.data.GetConstBufferPtr(), message.data.GetDataSize() );
        }
        else
        if ( eventid == WEB::CWebSocketClient::ConnectedEvent )
        {
            clientPeer = static_cast< WEB::TWebSocketConnectedEventData* >( eventdata )->GetData();
            ++clientConnects;
        }
        else
        if ( eventid == WEB::CWebSocketClient::ClosedEvent )
        {
            clientClose = static_cast< WEB::TWebSocketClosedEventData* >( eventdata )->GetData();
            ++clientCloses;
        }
        else
        if ( eventid == WEB::CWebSocketClient::ErrorEvent )
        {
            ++clientErrors;
        }
        else
        if ( eventid == WEB::CWebSocketClient::TextMessageEvent )
        {
            clientTextMessages.push_back( static_cast< WEB::TWebSocketTextMessageEventData* >( eventdata )->GetData().text );
            ++clientTexts;
        }
        else
        if ( eventid == WEB::CWebSocketClient::BinaryMessageEvent )
        {
            clientBinaryMessages.push_back( static_cast< WEB::TWebSocketBinaryMessageEventData* >( eventdata )->GetData().data );
            ++clientBinaries;
        }
    }

    UInt32 serverConnects, serverDisconnects, serverTexts, serverBinaries;
    UInt32 clientConnects, clientCloses, clientErrors, clientTexts, clientBinaries;
    GUCEF::vector< WEB::TWebSocketConnectionPtr > serverConnections;
    GUCEF::vector< WEB::SWebSocketPeerInfo > serverPeers;
    GUCEF::vector< WEB::SWebSocketCloseInfo > serverCloses;
    GUCEF::vector< CORE::CString > serverTextMessages;
    GUCEF::vector< CORE::CDynamicBuffer > serverBinaryMessages;
    WEB::SWebSocketPeerInfo clientPeer;
    WEB::SWebSocketCloseInfo clientClose;
    GUCEF::vector< CORE::CString > clientTextMessages;
    GUCEF::vector< CORE::CDynamicBuffer > clientBinaryMessages;
    bool echoServer;                        /**< echo everything the server receives back to its sender */
    WEB::CWebSocketServer* serverToScheduleOnText;  /**< scheduled for destruction from within the notification of the next text message */
};

/*-------------------------------------------------------------------------*/

static bool
EnsureHashCodecPluginLoadedForSockets( void )
{
    if ( WEB::CWebSocketHandshake::IsAvailable() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return WEB::CWebSocketHandshake::IsAvailable();
}

static CORE::CString
UrlFor( UInt16 port, const CORE::CString& resource )
{
    return "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + resource;
}

/*-------------------------------------------------------------------------*/

static const UInt32 WsThreadsRunTimeInMs = 4000;
static const UInt32 WsThreadsStallTimeInMs = 2000;
static const Int32 WsThreadsTaskEndTimeInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS + 15000;
static const UInt32 WsThreadsWindow = 64;     /**< messages a side may have under way to the client */

static UInt64
WsThreadsElapsedMs( UInt64 startTicks )
{
    return ( MT::PrecisionTickCount() - startTicks ) * 1000 / MT::PrecisionTimerResolution();
}

/**
 *  A web socket server on a pulse generator of its own, which a task of the task manager pulses, and what that task,
 *  a task that sends on the server's connection and the test thread share. The server echoes every text of the client
 *  from within its message event. The tasks may outlive a case that saw a deadlock, so it is made on the heap and
 *  given up then
 */
class CWsThreadsCase : public CORE::CObserver
{
    public:

    CORE::PulseGeneratorPtr pulseGenerator;
    WEB::CHTTPServer* httpServer;
    WEB::CWebSocketServer* server;
    UInt16 port;

    MT::CTAtomic< bool > stopPulsing;
    MT::CTAtomic< bool > stopSending;
    MT::CTAtomic< UInt32 > pulses;
    MT::CTAtomic< UInt32 > connects;
    MT::CTAtomic< UInt32 > echoes;
    MT::CTAtomic< UInt32 > sent;                  /**< texts the sending task sent on the server's connection */
    MT::CTAtomic< UInt32 > sentReceived;          /**< of those, received by the client (written by the test thread) */
    MT::CTAtomic< UInt32 > sendFailures;
    MT::CTAtomic< UInt32 > exceptions;

    /** oneTcpConnection: the server socket has a single TCP connection object, a later client waits for it */
    explicit CWsThreadsCase( bool oneTcpConnection = false )
        : CORE::CObserver()
        , pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , httpServer( GUCEF_NULL )
        , server( GUCEF_NULL )
        , port( 0 )
        , stopPulsing( false )
        , stopSending( false )
        , pulses( 0 )
        , connects( 0 )
        , echoes( 0 )
        , sent( 0 )
        , sentReceived( 0 )
        , sendFailures( 0 )
        , exceptions( 0 )
        , m_lock()
        , m_connection()
    {
        // set up and listening before the pulse task runs: what is done here happens before that thread starts
        WEB::CHttpServerSettings settings;
        settings.allowWebSocketUpgrades = true;
        settings.processRequestsAsync = false;
        if ( oneTcpConnection )
            settings.maxClientConnections = 0;      // no limit of the HTTP server, and the socket has one connection more: one
        httpServer = GUCEF_NEW WEB::CHTTPServer( pulseGenerator, GUCEF_NULL, &settings );
        server = GUCEF_NEW WEB::CWebSocketServer( *httpServer );

        TEventCallback connectedCallback( this, &CWsThreadsCase::OnClientConnected );
        SubscribeTo( server, WEB::CWebSocketServer::ClientConnectedEvent, connectedCallback );
        TEventCallback textCallback( this, &CWsThreadsCase::OnTextMessage );
        SubscribeTo( server, WEB::CWebSocketServer::TextMessageEvent, textCallback );

        if ( httpServer->ListenOnPort( 0 ) )
            port = httpServer->GetBoundPort();
    }

    virtual ~CWsThreadsCase()
    {
        UnsubscribeAllFromObserver();
        m_connection.Unlink();
        GUCEF_DELETE server;
        server = GUCEF_NULL;
        httpServer->Close();
        GUCEF_DELETE httpServer;
        httpServer = GUCEF_NULL;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CWsThreadsCase";
        return classTypeName;
    }

    WEB::TWebSocketConnectionPtr GetConnection( void )
    {
        MT::CScopeMutex lock( m_lock );
        return m_connection;
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CWsThreadsCase > TEventCallback;

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const WEB::TWebSocketConnectionPtr& connection = static_cast< WEB::TWebSocketServerConnectedEventData* >( eventData )->GetData();
        {
            MT::CScopeMutex lock( m_lock );
            m_connection = connection;
        }
        connects.FetchAdd( 1 );
    }

    // on the pulse task, within the data event of the server's TCP connection
    void OnTextMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        WEB::SWebSocketServerTextMessage& message = static_cast< WEB::TWebSocketServerTextMessageEventData* >( eventData )->GetData();
        if ( message.connection->SendText( message.text ) )
            echoes.FetchAdd( 1 );
        else
            sendFailures.FetchAdd( 1 );
    }

    MT::CMutex m_lock;
    WEB::TWebSocketConnectionPtr m_connection;
};

/*-------------------------------------------------------------------------*/

/** Pulses the pulse generator of the case until told to stop: the server runs on this task's thread */
class CWsThreadsPulseTask : public CORE::CTaskConsumer
{
    public:

    explicit CWsThreadsPulseTask( CWsThreadsCase& wsCase ) : CORE::CTaskConsumer(), m_case( wsCase ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestWebSocket::CWsThreadsPulseTask"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( !m_case.stopPulsing.Load( MT::MEMORDER_ACQUIRE ) )
        {
            try
            {
                m_case.pulseGenerator->ForceDirectPulse();
                m_case.pulses.FetchAdd( 1 );
            }
            catch ( ... )
            {
                m_case.exceptions.FetchAdd( 1 );
            }
            MT::ThreadDelay( 1 );
        }
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    CWsThreadsCase& m_case;
};

typedef CORE::CTSharedPtr< CWsThreadsPulseTask, MT::CMutex > TWsThreadsPulseTaskPtr;

/*-------------------------------------------------------------------------*/

/** Sends numbered texts on the server's connection of the client until told to stop, with at most a window under way */
class CWsThreadsSendTask : public CORE::CTaskConsumer
{
    public:

    CWsThreadsSendTask( CWsThreadsCase& wsCase, const WEB::TWebSocketConnectionPtr& connection )
        : CORE::CTaskConsumer(), m_case( wsCase ), m_connection( connection ) {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "TestWebSocket::CWsThreadsSendTask"; }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( !m_case.stopSending.Load( MT::MEMORDER_ACQUIRE ) )
        {
            try
            {
                const UInt32 sent = m_case.sent.Load();
                if ( sent - m_case.sentReceived.Load() < WsThreadsWindow )
                {
                    if ( m_connection->SendText( "s" + CORE::UInt32ToString( sent ) ) )
                        m_case.sent.FetchAdd( 1 );
                    else
                        m_case.sendFailures.FetchAdd( 1 );
                }
                else
                {
                    MT::ThreadDelay( 0 );
                }
            }
            catch ( ... )
            {
                m_case.exceptions.FetchAdd( 1 );
                break;
            }
        }
        m_connection.Unlink();
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    CWsThreadsCase& m_case;
    WEB::TWebSocketConnectionPtr m_connection;
};

typedef CORE::CTSharedPtr< CWsThreadsSendTask, MT::CMutex > TWsThreadsSendTaskPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Stands between the HTTP server of a case and its web socket server to see the TCP connection of the upgraded client.
 *  When armed it acts when the close frame of the server goes out on that connection, which is on the thread that
 *  closes the web socket connection, under the lock of the TCP connection: it plays the client that ends its connection
 *  right then (the TCP connection closes) and waits a while for the server to hand the TCP connection object to a later
 *  client, which a server whose close is reported under that lock does at once. It only waits, it takes no lock
 */
class CWsCloseTap : public CORE::CObserver                  ,
                    public WEB::CIWebSocketUpgradeHandler
{
    public:

    MT::CTAtomic< COMCORE::CTCPServerConnection* > tcpConnection;
    MT::CTAtomic< bool > armed;
    MT::CTAtomic< bool > fired;
    MT::CTAtomic< bool > handedToLaterClientWhileClosing;

    explicit CWsCloseTap( WEB::CWebSocketServer& server )
        : CORE::CObserver()
        , WEB::CIWebSocketUpgradeHandler()
        , tcpConnection( GUCEF_NULL )
        , armed( false )
        , fired( false )
        , handedToLaterClientWhileClosing( false )
        , m_server( server )
    {
    }

    virtual ~CWsCloseTap() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CWsCloseTap";
        return classTypeName;
    }

    // on the thread of the server
    virtual bool OnWebSocketUpgradeRequested( WEB::CHTTPServer& server, COMCORE::CTCPServerConnection& connection, const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        tcpConnection.Store( &connection );
        TEventCallback sentCallback( this, &CWsCloseTap::OnDataSent );
        SubscribeTo( &connection, COMCORE::CTCPConnection::DataSentEvent, sentCallback );
        return Inner().OnWebSocketUpgradeRequested( server, connection, request, response );
    }

    virtual void OnWebSocketConnectionEstablished( WEB::CHTTPServer& server, COMCORE::CTCPServerConnection& connection, const WEB::CHttpRequestData& request ) GUCEF_VIRTUAL_OVERRIDE
    {
        Inner().OnWebSocketConnectionEstablished( server, connection, request );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CWsCloseTap > TEventCallback;

    WEB::CIWebSocketUpgradeHandler& Inner( void ) { return m_server; }

    void OnDataSent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataSentEventData* >( eventData )->GetData();
        if ( !armed.Load() || 0 == data.GetDataSize() || 0x88 != static_cast< const UInt8* >( data.GetConstBufferPtr() )[ 0 ] )
            return;
        armed.Store( false );
        fired.Store( true );

        COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
        connection->Close();
        const UInt64 startTicks = MT::PrecisionTickCount();
        while ( !connection->IsActive() && WsThreadsElapsedMs( startTicks ) < 500 )
            MT::ThreadDelay( 1 );
        handedToLaterClientWhileClosing.Store( connection->IsActive() );
    }

    WEB::CWebSocketServer& m_server;
};

/*-------------------------------------------------------------------------*/

/** The client of the case, on the test thread: checks that the texts of either side arrive whole and in order */
class CWsThreadsClientWatcher : public CORE::CObserver
{
    public:

    explicit CWsThreadsClientWatcher( CWsThreadsCase& wsCase )
        : CORE::CObserver(), connects( 0 ), closes( 0 ), echoesReceived( 0 ), orderErrors( 0 ), m_case( wsCase ) {}

    virtual ~CWsThreadsClientWatcher() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CWsThreadsClientWatcher";
        return classTypeName;
    }

    void Watch( WEB::CWebSocketClient& client )
    {
        TEventCallback connectedCallback( this, &CWsThreadsClientWatcher::OnConnected );
        SubscribeTo( &client, WEB::CWebSocketClient::ConnectedEvent, connectedCallback );
        TEventCallback closedCallback( this, &CWsThreadsClientWatcher::OnClosed );
        SubscribeTo( &client, WEB::CWebSocketClient::ClosedEvent, closedCallback );
        TEventCallback textCallback( this, &CWsThreadsClientWatcher::OnText );
        SubscribeTo( &client, WEB::CWebSocketClient::TextMessageEvent, textCallback );
    }

    UInt32 connects;
    UInt32 closes;
    UInt32 echoesReceived;
    UInt32 orderErrors;

    private:

    typedef CORE::CTEventHandlerFunctor< CWsThreadsClientWatcher > TEventCallback;

    void OnConnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++connects; }

    void OnClosed( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++closes; }

    void OnText( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        const CORE::CString& text = static_cast< WEB::TWebSocketTextMessageEventData* >( eventData )->GetData().text;
        if ( text.Length() < 2 )
        {
            ++orderErrors;
            return;
        }
        const UInt32 number = CORE::StringToUInt32( text.CutChars( 1, true ) );
        if ( 's' == text[ 0 ] )
        {
            // the texts of the sending task
            if ( number != m_case.sentReceived.Load() )
                ++orderErrors;
            m_case.sentReceived.FetchAdd( 1 );
        }
        else
        if ( 'c' == text[ 0 ] )
        {
            // the echoes of our own texts
            if ( number != echoesReceived )
                ++orderErrors;
            ++echoesReceived;
        }
        else
        {
            ++orderErrors;
        }
    }

    CWsThreadsCase& m_case;
};

/*-------------------------------------------------------------------------*/

void
PerformWebSocketTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING WebSocket server and client TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "WebSocket" );

    if ( !EnsureHashCodecPluginLoadedForSockets() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebSocket server and client TESTS skipped: the sha1-raw codec is not available" );
        return;
    }

    // every server listens on a port the system picks
    UInt16 port = 0;

    GUCEF_TESTFW_TESTCASE( "Test: Handshake, text and binary echo" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();

            CWsRecorder recorder;
            recorder.echoServer = true;
            recorder.WatchServer( server );

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/echo?x=1" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            ASSERT_TRUE( client.IsConnected() );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( 1 == recorder.serverPeers.size() );
            if ( 1 == recorder.serverPeers.size() )
            {
                ASSERT_TRUE( recorder.serverPeers[ 0 ].requestUri == "/echo?x=1" );
                ASSERT_TRUE( recorder.serverPeers[ 0 ].subProtocol.IsNULLOrEmpty() );
            }

            // text, including characters outside of ASCII
            const CORE::CString utf8Text( "h\xC3\xA9llo w\xC3\xBBrld \xE2\x82\xAC" );
            ASSERT_TRUE( client.SendText( utf8Text ) );
            ASSERT_TRUE( PumpUntil( recorder.clientTexts, 1 ) );
            ASSERT_TRUE( 1 == recorder.clientTextMessages.size() && recorder.clientTextMessages[ 0 ] == utf8Text );
            ASSERT_TRUE( 1 == recorder.serverTextMessages.size() && recorder.serverTextMessages[ 0 ] == utf8Text );

            // an empty text message
            ASSERT_TRUE( client.SendText( CORE::CString() ) );
            ASSERT_TRUE( PumpUntil( recorder.clientTexts, 2 ) );

            // binary
            const UInt8 smallBinary[] = { 0, 1, 2, 3, 0xFF, 0xFE };
            ASSERT_TRUE( client.SendBinary( smallBinary, sizeof( smallBinary ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientBinaries, 1 ) );
            ASSERT_TRUE( 1 == recorder.clientBinaryMessages.size() );
            if ( 1 == recorder.clientBinaryMessages.size() )
            {
                ASSERT_TRUE( sizeof( smallBinary ) == recorder.clientBinaryMessages[ 0 ].GetDataSize() );
                ASSERT_TRUE( 0 == memcmp( smallBinary, recorder.clientBinaryMessages[ 0 ].GetConstBufferPtr(), sizeof( smallBinary ) ) );
            }

            // large messages arrive in many network reads, include sizes around the boundaries
            // where reads and writes used to go wrong (64 KiB) and exact multiples of the read block size
            const UInt32 largeSizes[] = { 1024, 65535, 65536, 66000, 300000 };
            UInt32 expectedBinaries = 1;
            for ( UInt32 si=0; si<sizeof( largeSizes ) / sizeof( largeSizes[ 0 ] ); ++si )
            {
                CORE::CDynamicBuffer large;
                large.SetDataSize( largeSizes[ si ] );
                for ( UInt32 i=0; i<large.GetDataSize(); ++i )
                    large.AsTypePtr< UInt8 >()[ i ] = (UInt8) ( i * 31 + 7 + si );
                ASSERT_TRUE( client.SendBinary( large.GetConstBufferPtr(), large.GetDataSize() ) );
                ++expectedBinaries;
                ASSERT_TRUE( PumpUntil( recorder.clientBinaries, expectedBinaries, 15000 ) );
                ASSERT_TRUE( expectedBinaries == recorder.clientBinaryMessages.size() );
                if ( expectedBinaries == recorder.clientBinaryMessages.size() )
                {
                    ASSERT_TRUE( large.GetDataSize() == recorder.clientBinaryMessages[ expectedBinaries - 1 ].GetDataSize() );
                    ASSERT_TRUE( 0 == memcmp( large.GetConstBufferPtr(), recorder.clientBinaryMessages[ expectedBinaries - 1 ].GetConstBufferPtr(), large.GetDataSize() ) );
                }
            }

            client.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Message order is preserved" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );

            const UInt32 count = 200;
            for ( UInt32 i=0; i<count; ++i )
                ASSERT_TRUE( client.SendText( "message " + CORE::UInt32ToString( i ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverTexts, count ) );
            ASSERT_TRUE( count == recorder.serverTextMessages.size() );
            for ( UInt32 i=0; i<recorder.serverTextMessages.size(); ++i )
                ASSERT_TRUE( recorder.serverTextMessages[ i ] == "message " + CORE::UInt32ToString( i ) );
            client.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Closing handshake started by the client" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( 1 == server.GetConnectionCount() );

            ASSERT_TRUE( client.Close( 1000, "bye" ) );
            ASSERT_FALSE( client.SendText( "after close" ) );
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.clientCloses, 1 ) );
            ASSERT_TRUE( 1 == recorder.serverCloses.size() );
            if ( 1 == recorder.serverCloses.size() )
            {
                ASSERT_TRUE( 1000 == recorder.serverCloses[ 0 ].closeCode );
                ASSERT_TRUE( recorder.serverCloses[ 0 ].closeReason == "bye" );
                ASSERT_TRUE( recorder.serverCloses[ 0 ].closedByPeer );
            }
            ASSERT_TRUE( 1000 == recorder.clientClose.closeCode );
            ASSERT_FALSE( recorder.clientClose.closedByPeer );
            ASSERT_TRUE( 0 == server.GetConnectionCount() );
            ASSERT_TRUE( WEB::CWebSocketClient::STATE_DISCONNECTED == client.GetState() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Closing handshake started by the server" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );

            ASSERT_TRUE( 1 == recorder.serverConnections.size() );
            WEB::TWebSocketConnectionPtr connection = recorder.serverConnections[ 0 ];
            ASSERT_TRUE( connection->IsOpen() );
            ASSERT_TRUE( connection->Close( 1001, "going away" ) );
            ASSERT_FALSE( connection->IsOpen() );
            ASSERT_FALSE( connection->Close() );
            ASSERT_FALSE( connection->SendText( "too late" ) );
            ASSERT_TRUE( PumpUntil( recorder.clientCloses, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );
            ASSERT_TRUE( 1001 == recorder.clientClose.closeCode );
            ASSERT_TRUE( recorder.clientClose.closeReason == "going away" );
            ASSERT_TRUE( recorder.clientClose.closedByPeer );
            ASSERT_TRUE( 1 == recorder.serverCloses.size() && !recorder.serverCloses[ 0 ].closedByPeer );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A connection's message size limit changes at runtime, a larger message closes the connection with 1009" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( 16 * 1024 * 1024 == server.GetMaxMessageSize() );
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            CORE::CDynamicBuffer small;
            small.SetDataSize( 500 );
            memset( small.GetBufferPtr(), 0x11, 500 );
            CORE::CDynamicBuffer large;
            large.SetDataSize( 2000 );
            memset( large.GetBufferPtr(), 0x22, 2000 );

            // a limit raised at runtime takes a message the smaller one would have refused
            WEB::CWebSocketClient raised;
            recorder.WatchClient( raised );
            ASSERT_TRUE( raised.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            WEB::TWebSocketConnectionPtr first = recorder.serverConnections[ 0 ];
            first->SetMaxMessageSize( 100 );
            first->SetMaxMessageSize( 5000 );
            ASSERT_TRUE( 5000 == first->GetMaxMessageSize() );
            ASSERT_TRUE( raised.SendBinary( large.GetConstBufferPtr(), large.GetDataSize() ) );
            ASSERT_TRUE( PumpUntil( recorder.serverBinaries, 1 ) );
            ASSERT_TRUE( first->IsOpen() );
            ASSERT_TRUE( raised.Close( 1000, "done" ) );
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );

            // a lowered limit: what fits is taken, a larger message is refused and the connection closed
            WEB::CWebSocketClient lowered;
            recorder.WatchClient( lowered );
            ASSERT_TRUE( lowered.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 2 ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 2 ) );
            WEB::TWebSocketConnectionPtr second = recorder.serverConnections[ 1 ];
            ASSERT_TRUE( 16 * 1024 * 1024 == second->GetMaxMessageSize() );
            second->SetMaxMessageSize( 1000 );
            ASSERT_TRUE( lowered.SendBinary( small.GetConstBufferPtr(), small.GetDataSize() ) );
            ASSERT_TRUE( PumpUntil( recorder.serverBinaries, 2 ) );
            ASSERT_TRUE( lowered.SendBinary( large.GetConstBufferPtr(), large.GetDataSize() ) );
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 2 ) );
            ASSERT_TRUE( PumpUntil( recorder.clientCloses, 2 ) );
            ASSERT_TRUE( 2 == recorder.serverBinaries );
            ASSERT_FALSE( second->IsOpen() );
            ASSERT_TRUE( 2 == recorder.serverCloses.size() );
            if ( 2 == recorder.serverCloses.size() )
            {
                ASSERT_TRUE( 1009 == recorder.serverCloses[ 1 ].closeCode );
                ASSERT_FALSE( recorder.serverCloses[ 1 ].closedByPeer );
            }
            ASSERT_TRUE( 1009 == recorder.clientClose.closeCode );
            ASSERT_TRUE( recorder.clientClose.closedByPeer );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sub protocol negotiation" )
        try
        {
            WEB::CWebSocketServer server;
            WEB::CWebSocketServer::TStringVector supported;
            supported.push_back( "chat" );
            supported.push_back( "superchat" );
            server.SetSupportedSubProtocols( supported );
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            // The client offers 'foo' and 'superchat', the only one in common is selected
            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            WEB::CWebSocketClient::TStringVector offered;
            offered.push_back( "foo" );
            offered.push_back( "superchat" );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ), offered ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            ASSERT_TRUE( client.GetSubProtocol() == "superchat" );
            ASSERT_TRUE( recorder.clientPeer.subProtocol == "superchat" );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( 1 == recorder.serverPeers.size() && recorder.serverPeers[ 0 ].subProtocol == "superchat" );
            client.Disconnect();
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );

            // Nothing in common: the server refuses
            WEB::CWebSocketClient refusedClient;
            recorder.WatchClient( refusedClient );
            UInt32 errorsBefore = recorder.clientErrors;
            UInt32 closesBefore = recorder.clientCloses;
            WEB::CWebSocketClient::TStringVector unsupported;
            unsupported.push_back( "foo" );
            ASSERT_TRUE( refusedClient.Connect( UrlFor( port, "/" ), unsupported ) );
            ASSERT_TRUE( PumpUntil( recorder.clientCloses, closesBefore + 1 ) );
            ASSERT_TRUE( recorder.clientErrors > errorsBefore );
            ASSERT_FALSE( refusedClient.IsConnected() );
            ASSERT_TRUE( 0 == server.GetConnectionCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Broadcast reaches all clients" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();

            CWsRecorder recorderA;
            CWsRecorder recorderB;
            WEB::CWebSocketClient clientA;
            WEB::CWebSocketClient clientB;
            recorderA.WatchClient( clientA );
            recorderB.WatchClient( clientB );
            ASSERT_TRUE( clientA.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( clientB.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorderA.clientConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorderB.clientConnects, 1 ) );
            ASSERT_TRUE( 2 == server.GetConnectionCount() );

            ASSERT_TRUE( 2 == server.BroadcastText( "to everybody" ) );
            const UInt8 payload[] = { 9, 8, 7 };
            ASSERT_TRUE( 2 == server.BroadcastBinary( payload, sizeof( payload ) ) );
            ASSERT_TRUE( PumpUntil( recorderA.clientTexts, 1 ) );
            ASSERT_TRUE( PumpUntil( recorderB.clientTexts, 1 ) );
            ASSERT_TRUE( PumpUntil( recorderA.clientBinaries, 1 ) );
            ASSERT_TRUE( PumpUntil( recorderB.clientBinaries, 1 ) );
            ASSERT_TRUE( recorderA.clientTextMessages[ 0 ] == "to everybody" );
            ASSERT_TRUE( recorderB.clientTextMessages[ 0 ] == "to everybody" );

            // the server can also address a single client, directly on its connection object
            WEB::CWebSocketServer::TConnectionVector connections;
            server.GetConnections( connections );
            ASSERT_TRUE( 2 == connections.size() );
            if ( 2 == connections.size() )
            {
                ASSERT_TRUE( connections[ 0 ]->SendText( "just one" ) );
                ASSERT_TRUE( connections[ 0 ]->GetId() != connections[ 1 ]->GetId() );
            }

            clientA.Disconnect();
            clientB.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // A connection is sent on from any thread while its data is received on the thread of the server's pulse
    // generator, under the lock of the TCP connection, and the server answers from within its message event. A send
    // used to hold the lock of the web socket connection while it took the lock of the TCP connection, the receiving
    // side took them the other way around: the two threads deadlocked
    GUCEF_TESTFW_TESTCASE( "Test: A connection sent on from another thread while the server answers from within its message event: no deadlock, every message whole and in order" )
        try
        {
            CWsThreadsCase* wsCase = GUCEF_NEW CWsThreadsCase();
            ASSERT_TRUE( 0 != wsCase->port );

            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestWebSocketThreads" );
            TWsThreadsPulseTaskPtr pulseTask( GUCEF_NEW CWsThreadsPulseTask( *wsCase ) );
            ASSERT_FALSE( pool->StartTaskWithConsumer( pulseTask ).HasNoFuture() );

            WEB::CWebSocketClient client;
            CWsThreadsClientWatcher watcher( *wsCase );
            watcher.Watch( client );
            ASSERT_TRUE( client.Connect( UrlFor( wsCase->port, "/threads" ) ) );
            ASSERT_TRUE( PumpUntil( watcher.connects, 1 ) );
            UInt64 startTicks = MT::PrecisionTickCount();
            while ( 0 == wsCase->connects.Load() && WsThreadsElapsedMs( startTicks ) < WAIT_TIMEOUT_IN_MS )
                MT::ThreadDelay( 1 );
            WEB::TWebSocketConnectionPtr connection = wsCase->GetConnection();
            ASSERT_FALSE( connection.IsNULL() );

            TWsThreadsSendTaskPtr sendTask;
            if ( !connection.IsNULL() )
            {
                sendTask = TWsThreadsSendTaskPtr( GUCEF_NEW CWsThreadsSendTask( *wsCase, connection ) );
                ASSERT_FALSE( pool->StartTaskWithConsumer( sendTask ).HasNoFuture() );
            }
            connection.Unlink();

            // the client sends numbered texts for the server to echo, at most a window under way, and the test thread
            // watches both tasks: they have to keep making progress
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
            UInt32 clientSent = 0;
            UInt32 lastPulses = 0;
            UInt32 lastSent = 0;
            UInt64 pulseProgressTicks = MT::PrecisionTickCount();
            UInt64 sendProgressTicks = pulseProgressTicks;
            bool stalled = false;
            startTicks = MT::PrecisionTickCount();
            while ( WsThreadsElapsedMs( startTicks ) < WsThreadsRunTimeInMs )
            {
                if ( clientSent - watcher.echoesReceived < WsThreadsWindow && client.SendText( "c" + CORE::UInt32ToString( clientSent ) ) )
                    ++clientSent;
                pulseGenerator->ForceDirectPulse();
                MT::ThreadDelay( 0 );

                const UInt64 nowTicks = MT::PrecisionTickCount();
                if ( wsCase->pulses.Load() != lastPulses )
                {
                    lastPulses = wsCase->pulses.Load();
                    pulseProgressTicks = nowTicks;
                }
                if ( wsCase->sent.Load() != lastSent )
                {
                    lastSent = wsCase->sent.Load();
                    sendProgressTicks = nowTicks;
                }
                if ( WsThreadsElapsedMs( pulseProgressTicks ) > WsThreadsStallTimeInMs || WsThreadsElapsedMs( sendProgressTicks ) > WsThreadsStallTimeInMs )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebSocket threads: a task made no progress for " << WsThreadsStallTimeInMs
                        << " ms, pulses " << lastPulses << ", sent by the task " << lastSent << ", echoes " << wsCase->echoes.Load() );
                    stalled = true;
                    break;
                }
            }
            wsCase->stopSending.Store( true, MT::MEMORDER_RELEASE );
            const bool sendTaskEnded = sendTask.IsNULL() || sendTask->WaitForTaskToFinish( WsThreadsTaskEndTimeInMs );

            // what is still under way reaches the client
            startTicks = MT::PrecisionTickCount();
            while ( !stalled && sendTaskEnded && ( wsCase->sentReceived.Load() < wsCase->sent.Load() || watcher.echoesReceived < clientSent ) && WsThreadsElapsedMs( startTicks ) < WAIT_TIMEOUT_IN_MS )
            {
                pulseGenerator->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            client.Disconnect();

            wsCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulseTaskEnded = pulseTask->WaitForTaskToFinish( WsThreadsTaskEndTimeInMs );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "WebSocket threads: pulses " << wsCase->pulses.Load() << ", sent by the task " << wsCase->sent.Load()
                << ", received of those " << wsCase->sentReceived.Load() << ", client texts " << clientSent << ", echoes " << watcher.echoesReceived );

            ASSERT_FALSE( stalled );
            ASSERT_TRUE( sendTaskEnded );
            ASSERT_TRUE( pulseTaskEnded );
            ASSERT_TRUE( wsCase->sent.Load() > 0 );
            ASSERT_TRUE( watcher.echoesReceived > 0 );
            ASSERT_TRUE( wsCase->sentReceived.Load() == wsCase->sent.Load() );
            ASSERT_TRUE( watcher.echoesReceived == clientSent );
            ASSERT_TRUE( 0 == watcher.orderErrors );
            ASSERT_TRUE( 0 == wsCase->sendFailures.Load() );
            ASSERT_TRUE( 0 == wsCase->exceptions.Load() );

            if ( sendTaskEnded && pulseTaskEnded )
                GUCEF_DELETE wsCase;
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The close frame and the close of the TCP connection are one step under the lock of the TCP connection: with the
    // close made after that lock was given up, the connection could have ended and gone to a later client in between
    GUCEF_TESTFW_TESTCASE( "Test: A connection closed on another thread than the server's never closes a later client of the TCP connection it had" )
        try
        {
            CWsThreadsCase* wsCase = GUCEF_NEW CWsThreadsCase( true );
            ASSERT_TRUE( 0 != wsCase->port );
            CWsCloseTap tap( *wsCase->server );
            wsCase->httpServer->SetWebSocketUpgradeHandler( &tap );

            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "TestWebSocketThreads" );
            TWsThreadsPulseTaskPtr pulseTask( GUCEF_NEW CWsThreadsPulseTask( *wsCase ) );
            ASSERT_FALSE( pool->StartTaskWithConsumer( pulseTask ).HasNoFuture() );

            WEB::CWebSocketClient client;
            CWsThreadsClientWatcher watcher( *wsCase );
            watcher.Watch( client );
            ASSERT_TRUE( client.Connect( UrlFor( wsCase->port, "/close" ) ) );
            ASSERT_TRUE( PumpUntil( watcher.connects, 1 ) );
            UInt64 startTicks = MT::PrecisionTickCount();
            while ( 0 == wsCase->connects.Load() && WsThreadsElapsedMs( startTicks ) < WAIT_TIMEOUT_IN_MS )
                MT::ThreadDelay( 1 );
            WEB::TWebSocketConnectionPtr connection = wsCase->GetConnection();
            ASSERT_FALSE( connection.IsNULL() );
            ASSERT_TRUE( GUCEF_NULL != tap.tcpConnection.Load() );

            // the later client: the operating system takes its connection, the server socket waits for its one TCP
            // connection object to become free
            CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
            COMCORE::CTCPClientSocket later( pulseGenerator, false );
            ASSERT_TRUE( later.ConnectTo( "127.0.0.1", wsCase->port, false ) );
            PumpFor( 100 );
            ASSERT_TRUE( later.IsActive() );

            if ( !connection.IsNULL() && GUCEF_NULL != tap.tcpConnection.Load() )
            {
                tap.armed.Store( true );
                ASSERT_TRUE( connection->Close( 1000, "done" ) );
                ASSERT_TRUE( tap.fired.Load() );
                ASSERT_FALSE( tap.handedToLaterClientWhileClosing.Load() );

                // the later client gets the TCP connection object and keeps it
                startTicks = MT::PrecisionTickCount();
                while ( !tap.tcpConnection.Load()->IsActive() && WsThreadsElapsedMs( startTicks ) < WAIT_TIMEOUT_IN_MS )
                {
                    pulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( tap.tcpConnection.Load()->IsActive() );
                PumpFor( 300 );
                ASSERT_TRUE( later.IsActive() );
                ASSERT_TRUE( tap.tcpConnection.Load()->IsActive() );
            }
            connection.Unlink();
            later.Close();
            client.Disconnect();
            PumpFor( 50 );

            wsCase->stopPulsing.Store( true, MT::MEMORDER_RELEASE );
            const bool pulseTaskEnded = pulseTask->WaitForTaskToFinish( WsThreadsTaskEndTimeInMs );
            ASSERT_TRUE( pulseTaskEnded );
            ASSERT_TRUE( 0 == wsCase->exceptions.Load() );
            tap.UnsubscribeAllFromObserver();
            if ( pulseTaskEnded )
            {
                wsCase->httpServer->SetWebSocketUpgradeHandler( wsCase->server );
                GUCEF_DELETE wsCase;
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // The server re-uses its TCP connection objects for later clients, a connection object of an ended
    // connection which is still held by somebody must never end up sending to that later client
    GUCEF_TESTFW_TESTCASE( "Test: A connection object of an ended connection never sends to a later client" )
        try
        {
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder recorder;
            recorder.WatchServer( server );

            WEB::CWebSocketClient firstClient;
            CWsRecorder firstRecorder;
            firstRecorder.WatchClient( firstClient );
            ASSERT_TRUE( firstClient.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( firstRecorder.clientConnects, 1 ) );
            WEB::TWebSocketConnectionPtr stale = recorder.serverConnections[ 0 ];
            UInt32 staleId = stale->GetId();
            ASSERT_TRUE( stale->IsOpen() );

            firstClient.Disconnect();
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );
            ASSERT_FALSE( stale->IsOpen() );
            ASSERT_FALSE( stale->SendText( "to nobody" ) );
            ASSERT_TRUE( 0 == server.GetConnectionCount() );

            // a new client most likely gets the same TCP connection slot, and so the same id
            WEB::CWebSocketClient secondClient;
            CWsRecorder secondRecorder;
            secondRecorder.WatchClient( secondClient );
            ASSERT_TRUE( secondClient.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 2 ) );
            ASSERT_TRUE( PumpUntil( secondRecorder.clientConnects, 1 ) );
            ASSERT_TRUE( 2 == recorder.serverConnections.size() );
            WEB::TWebSocketConnectionPtr fresh = recorder.serverConnections[ 1 ];
            ASSERT_TRUE( fresh.GetPointer() != stale.GetPointer() );
            ASSERT_TRUE( fresh->IsOpen() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Stale connection id " + CORE::UInt32ToString( staleId ) + ", fresh connection id " + CORE::UInt32ToString( fresh->GetId() ) );

            // the stale handle does nothing, not even for the client now living in its former slot
            ASSERT_FALSE( stale->SendText( "from the past" ) );
            ASSERT_FALSE( stale->Close() );
            PumpFor( 200 );
            ASSERT_TRUE( 0 == secondRecorder.clientTexts );
            ASSERT_TRUE( secondClient.IsConnected() );

            // while the fresh handle works
            ASSERT_TRUE( fresh->SendText( "hello" ) );
            ASSERT_TRUE( PumpUntil( secondRecorder.clientTexts, 1 ) );
            ASSERT_TRUE( secondRecorder.clientTextMessages[ 0 ] == "hello" );
            secondClient.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // NotifyObservers() returns false when the notifier was deleted as a result of the notification, nothing may
    // touch the object after that. The supported way to have a notifier deleted from within its notification is
    // ScheduleForDestruction(). The server must then not use itself or hand out anything more to its connections
    GUCEF_TESTFW_TESTCASE( "Test: Scheduling the server for destruction from within its message event is safe" )
        try
        {
            // attached to an HTTP server that outlives it: a server which owns its HTTP server would be deleting
            // the socket stack which is in the middle of dispatching the event
            WEB::CHttpServerSettings httpSettings;
            httpSettings.allowWebSocketUpgrades = true;
            httpSettings.processRequestsAsync = false;
            WEB::CHTTPServer httpServer( GUCEF_NULL, &httpSettings );
            ASSERT_TRUE( httpServer.ListenOnPort( 0 ) );
            port = httpServer.GetBoundPort();
            WEB::CWebSocketServer* server = GUCEF_NEW WEB::CWebSocketServer( httpServer );

            // the server is destroyed through ScheduleForDestruction(), which unlinks the recorder from it
            CWsRecorder recorder;
            recorder.WatchServer( *server );
            recorder.serverToScheduleOnText = server;

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            ASSERT_TRUE( PumpUntil( recorder.serverConnects, 1 ) );
            WEB::TWebSocketConnectionPtr connection = recorder.serverConnections[ 0 ];

            // two messages in one go: the second must not be delivered to or through the deleted server
            ASSERT_TRUE( client.SendText( "first" ) );
            ASSERT_TRUE( client.SendText( "second" ) );
            ASSERT_TRUE( PumpUntil( recorder.serverTexts, 1 ) );
            PumpFor( 200 );
            ASSERT_TRUE( GUCEF_NULL == recorder.serverToScheduleOnText );
            ASSERT_TRUE( 1 == recorder.serverTexts );

            // the connection object we kept is harmless: it no longer belongs to a server
            ASSERT_FALSE( connection->SendText( "nobody home" ) );
            client.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Connecting to nothing reports an error and a close" )
        try
        {
            port = TESTFW::FindFreeTcpPort();     // free when it was looked up, nothing is started on it
            CWsRecorder recorder;
            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientCloses, 1 ) );
            ASSERT_TRUE( recorder.clientErrors >= 1 );
            ASSERT_FALSE( client.IsConnected() );
            ASSERT_TRUE( 1006 == recorder.clientClose.closeCode );

            // unusable urls are refused right away
            ASSERT_FALSE( client.Connect( "http://127.0.0.1/" ) );
            ASSERT_FALSE( client.Connect( "ftp://127.0.0.1/" ) );
            ASSERT_FALSE( client.Connect( "ws://" ) );
            ASSERT_FALSE( client.Connect( "ws://host:notaport/" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Attaching to an existing HTTP server" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.allowWebSocketUpgrades = true;
            settings.processRequestsAsync = false;
            WEB::CHTTPServer httpServer( GUCEF_NULL, &settings );
            ASSERT_TRUE( httpServer.ListenOnPort( 0 ) );
            port = httpServer.GetBoundPort();

            WEB::CWebSocketServer server( httpServer );
            ASSERT_FALSE( server.Listen( port + 1 ) );      // only for servers with their own HTTP server
            CWsRecorder recorder;
            recorder.echoServer = true;
            recorder.WatchServer( server );

            WEB::CWebSocketClient client;
            recorder.WatchClient( client );
            ASSERT_TRUE( client.Connect( UrlFor( port, "/attached" ) ) );
            ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
            ASSERT_TRUE( client.SendText( "ping pong" ) );
            ASSERT_TRUE( PumpUntil( recorder.clientTexts, 1 ) );
            ASSERT_TRUE( recorder.clientTextMessages[ 0 ] == "ping pong" );
            client.Disconnect();
            ASSERT_TRUE( PumpUntil( recorder.serverDisconnects, 1 ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: wss:// echo through a TLS terminator, and the failures of the TLS layer reach the client as errors" )
        CORE::CString whyNoTls;
        GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
        GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNoTls ), "this system has no TLS provider" );
        try
        {
            TESTFW::CTlsTestCerts certs;
            ASSERT_TRUE( certs.IsReady() );

            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( 0 ) );
            port = server.GetHttpServer().GetBoundPort();
            CWsRecorder serverRecorder;
            serverRecorder.echoServer = true;
            serverRecorder.WatchServer( server );

            TESTFW::CTlsTestTerminator terminator;
            ASSERT_TRUE( terminator.Start( certs.GetPath( "good.crt" ), certs.GetPath( "good.key" ), port ) );
            const CORE::CString url = "wss://localhost:" + CORE::UInt32ToString( terminator.GetPort() ) + "/secure?x=1";

            // the certificate of the test authority is trusted: text and binary messages go both ways
            {
                COMCORE::CTlsSettings settings;
                settings.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
                settings.SetUseSystemTrust( false );

                WEB::CWebSocketClient client;
                client.SetTlsSettings( settings );
                CWsRecorder recorder;
                recorder.WatchClient( client );
                ASSERT_TRUE( client.Connect( url ) );
                ASSERT_TRUE( PumpUntil( recorder.clientConnects, 1 ) );
                ASSERT_TRUE( client.IsConnected() );
                ASSERT_TRUE( client.SendText( "over tls" ) );
                ASSERT_TRUE( PumpUntil( recorder.clientTexts, 1 ) );
                ASSERT_TRUE( 1 == recorder.clientTextMessages.size() && recorder.clientTextMessages[ 0 ] == "over tls" );

                CORE::CDynamicBuffer large;
                large.SetDataSize( 200000 );
                for ( UInt32 i=0; i<large.GetDataSize(); ++i )
                    large.AsTypePtr< UInt8 >()[ i ] = (UInt8) ( i * 31 + 7 );
                ASSERT_TRUE( client.SendBinary( large.GetConstBufferPtr(), large.GetDataSize() ) );
                ASSERT_TRUE( PumpUntil( recorder.clientBinaries, 1, 15000 ) );
                ASSERT_TRUE( 1 == recorder.clientBinaryMessages.size() && 200000 == recorder.clientBinaryMessages[ 0 ].GetDataSize() );
                if ( 1 == recorder.clientBinaryMessages.size() )
                    ASSERT_TRUE( 0 == memcmp( large.GetConstBufferPtr(), recorder.clientBinaryMessages[ 0 ].GetConstBufferPtr(), large.GetDataSize() ) );

                // what the server saw is the resource of the url and no trace of TLS
                ASSERT_TRUE( PumpUntil( serverRecorder.serverConnects, 1 ) );
                ASSERT_TRUE( 1 == serverRecorder.serverPeers.size() && serverRecorder.serverPeers[ 0 ].requestUri == "/secure?x=1" );
                client.Disconnect();
            }

            // the system trust store does not know the authority of the test: the certificate is refused and the error says why
            {
                WEB::CWebSocketClient client;
                CWsRecorder recorder;
                recorder.WatchClient( client );
                ASSERT_TRUE( client.Connect( url ) );
                ASSERT_TRUE( PumpUntil( recorder.clientCloses, 1 ) );
                ASSERT_TRUE( recorder.clientErrors >= 1 );
                ASSERT_TRUE( 0 == recorder.clientConnects );
                ASSERT_TRUE( 1006 == recorder.clientClose.closeCode );
            }

            // without a TLS provider wss:// is refused at once, ws:// is not affected
            {
                TESTFW::CNoTlsProviderScope noTls;
                WEB::CWebSocketClient client;
                ASSERT_FALSE( client.Connect( url ) );
                ASSERT_TRUE( client.GetState() == WEB::CWebSocketClient::STATE_DISCONNECTED );
                ASSERT_TRUE( client.Connect( UrlFor( port, "/plain" ) ) );
                client.Disconnect();
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL WebSocket server and client TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
