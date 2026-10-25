/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_TEST_NO_TLS_PROVIDER_H
#include "gucef_test_no_tls_provider.h"
#define GUCEF_TEST_NO_TLS_PROVIDER_H
#endif /* GUCEF_TEST_NO_TLS_PROVIDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestWebSocket.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

static const UInt16 TEST_PORT_BASE = 47310;
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

    UInt16 port = TEST_PORT_BASE;

    GUCEF_TESTFW_TESTCASE( "Test: Handshake, text and binary echo" )
        try
        {
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );

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
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( 16 * 1024 * 1024 == server.GetMaxMessageSize() );
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            WEB::CWebSocketServer server;
            WEB::CWebSocketServer::TStringVector supported;
            supported.push_back( "chat" );
            supported.push_back( "superchat" );
            server.SetSupportedSubProtocols( supported );
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );

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

    // The server re-uses its TCP connection objects for later clients, a connection object of an ended
    // connection which is still held by somebody must never end up sending to that later client
    GUCEF_TESTFW_TESTCASE( "Test: A connection object of an ended connection never sends to a later client" )
        try
        {
            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );
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
            ++port;
            // attached to an HTTP server that outlives it: a server which owns its HTTP server would be deleting
            // the socket stack which is in the middle of dispatching the event
            WEB::CHttpServerSettings httpSettings;
            httpSettings.allowWebSocketUpgrades = true;
            httpSettings.processRequestsAsync = false;
            WEB::CHTTPServer httpServer( GUCEF_NULL, &httpSettings );
            ASSERT_TRUE( httpServer.ListenOnPort( port ) );
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
            ++port;
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
            ++port;
            WEB::CHttpServerSettings settings;
            settings.allowWebSocketUpgrades = true;
            settings.processRequestsAsync = false;
            WEB::CHTTPServer httpServer( GUCEF_NULL, &settings );
            ASSERT_TRUE( httpServer.ListenOnPort( port ) );

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

            ++port;
            WEB::CWebSocketServer server;
            ASSERT_TRUE( server.Listen( port ) );
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
