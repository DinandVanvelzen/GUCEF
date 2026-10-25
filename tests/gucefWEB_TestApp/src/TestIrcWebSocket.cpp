/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CIRCCLIENT_H
#include "gucefCOM_CIrcClient.h"
#define GUCEF_COM_CIRCCLIENT_H
#endif /* GUCEF_COM_CIRCCLIENT_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H
#include "gucefWEB_CIrcWebSocketTransport.h"
#define GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H
#endif /* GUCEF_WEB_CIRCWEBSOCKETTRANSPORT_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestIrcWebSocket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt16 IrcWebSocketPort = 47861;

/*-------------------------------------------------------------------------*/

bool
PumpUntilAtLeast( const UInt32& counter, UInt32 target, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && counter < target; ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return counter >= target;
}

/*-------------------------------------------------------------------------*/

bool
PumpUntilTrue( const bool& flag, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !flag; ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return flag;
}

/*-------------------------------------------------------------------------*/

bool
EnsureHashCodecPlugin( void )
{GUCEF_TRACE;

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

/*-------------------------------------------------------------------------*/

/**
 *  An IRC server behind a WebSocket: one line per text message, the text.ircv3.net sub protocol
 */
class CFakeIrcWebSocketServer : public CORE::CObserver
{
    public:

    bool registered;
    bool quitReceived;
    CORE::CString lastPrivMsg;
    CORE::CString subProtocol;
    CORE::CString requestUri;
    bool lineHadLineBreak;

    CFakeIrcWebSocketServer( void )
        : CORE::CObserver()
        , registered( false )
        , quitReceived( false )
        , lastPrivMsg()
        , subProtocol()
        , requestUri()
        , lineHadLineBreak( false )
        , m_server()
        , m_connection()
        , m_nick()
        , m_userSeen( false )
        , m_capEnded( false )
    {GUCEF_TRACE;

        WEB::CWebSocketServer::TStringVector subProtocols;
        subProtocols.push_back( "text.ircv3.net" );
        m_server.SetSupportedSubProtocols( subProtocols );
        SubscribeTo( &m_server );
    }

    virtual ~CFakeIrcWebSocketServer()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        m_server.Close();
    }

    bool Start( void )
    {GUCEF_TRACE;

        return m_server.Listen( IrcWebSocketPort );
    }

    void SendLine( const CORE::CString& line )
    {GUCEF_TRACE;

        if ( !m_connection.IsNULL() )
            m_connection->SendText( line );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CFakeIrcWebSocketServer";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( WEB::CWebSocketServer::ClientConnectedEvent == eventId )
        {
            m_connection = static_cast< WEB::TWebSocketServerConnectedEventData* >( eventData )->GetData();
            subProtocol = m_connection->GetPeerInfo().subProtocol;
            requestUri = m_connection->GetPeerInfo().requestUri;
            SendLine( ":fake.server NOTICE * :*** Welcome to the WebSocket gateway" );
        }
        else
        if ( WEB::CWebSocketServer::TextMessageEvent == eventId )
        {
            OnLine( static_cast< WEB::TWebSocketServerTextMessageEventData* >( eventData )->GetData().text );
        }
    }

    private:

    void TryRegister( void )
    {GUCEF_TRACE;

        if ( !registered && m_userSeen && m_capEnded && !m_nick.IsNULLOrEmpty() )
        {
            registered = true;
            SendLine( ":fake.server 001 " + m_nick + " :Welcome" );
        }
    }

    void OnLine( const CORE::CString& line )
    {GUCEF_TRACE;

        if ( line.HasChar( '\n' ) >= 0 || line.HasChar( '\r' ) >= 0 )
            lineHadLineBreak = true;

        COM::CIrcMessage message;
        if ( !message.Parse( line ) )
            return;

        if ( "CAP" == message.command )
        {
            if ( "LS" == message.GetParam( 0 ) )
                SendLine( ":fake.server CAP * LS :message-tags" );
            else if ( "REQ" == message.GetParam( 0 ) )
                SendLine( ":fake.server CAP * ACK :" + message.GetParam( 1 ) );
            else if ( "END" == message.GetParam( 0 ) )
            {
                m_capEnded = true;
                TryRegister();
            }
        }
        else
        if ( "NICK" == message.command )
        {
            m_nick = message.GetParam( 0 );
            TryRegister();
        }
        else
        if ( "USER" == message.command )
        {
            m_userSeen = true;
            TryRegister();
        }
        else
        if ( "PRIVMSG" == message.command )
        {
            lastPrivMsg = message.GetParam( 1 );
            SendLine( "@msgid=ws1 :alice!a@host PRIVMSG " + message.GetParam( 0 ) + " :welcome over the web\r\n" );
        }
        else
        if ( "QUIT" == message.command )
        {
            quitReceived = true;
        }
    }

    WEB::CWebSocketServer m_server;
    WEB::TWebSocketConnectionPtr m_connection;
    CORE::CString m_nick;
    bool m_userSeen;
    bool m_capEnded;
};

/*-------------------------------------------------------------------------*/

/**
 *  Counts the events of an IRC client
 */
class CIrcRecorder : public CORE::CObserver
{
    public:

    UInt32 connected;
    UInt32 registered;
    UInt32 disconnected;
    UInt32 messages;
    UInt32 notices;
    UInt32 errors;
    COM::CIrcMessageEventData lastMessage;

    CIrcRecorder( void )
        : CORE::CObserver()
        , connected( 0 )
        , registered( 0 )
        , disconnected( 0 )
        , messages( 0 )
        , notices( 0 )
        , errors( 0 )
        , lastMessage()
    {GUCEF_TRACE;

    }

    virtual ~CIrcRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CIrcRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COM::CIrcClient::ConnectedEvent == eventId ) ++connected;
        else if ( COM::CIrcClient::RegisteredEvent == eventId ) ++registered;
        else if ( COM::CIrcClient::DisconnectedEvent == eventId ) ++disconnected;
        else if ( COM::CIrcClient::NoticeEvent == eventId ) ++notices;
        else if ( COM::CIrcClient::ErrorEvent == eventId ) ++errors;
        else if ( COM::CIrcClient::MessageEvent == eventId )
        {
            ++messages;
            if ( GUCEF_NULL != eventData )
                lastMessage = *static_cast< COM::CIrcMessageEventData* >( eventData );
        }
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIrcWebSocketTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "IrcWebSocket" );

    GUCEF_TESTFW_TESTCASE( "Test: gucefWEB registers the WebSocket transport for IRC" )
        try
        {
            WEB::CWebGlobal::Instance();
            ASSERT_TRUE( COM::CComGlobal::Instance()->GetIrcTransportFactory().IsConstructible( "websocket" ) );
            COM::CIIrcTransportPtr transport = COM::CComGlobal::Instance()->GetIrcTransportFactory().Create( "websocket" );
            ASSERT_FALSE( transport.IsNULL() );
            if ( !transport.IsNULL() )
            {
                ASSERT_TRUE( transport->GetTransportName() == "websocket" );
                ASSERT_FALSE( transport->IsConnected() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The IRC client registers and talks over a WebSocket with the text.ircv3.net sub protocol" )
        GUCEF_TESTFW_SKIP_IF( !EnsureHashCodecPlugin(), "The sha1 codec for the WebSocket handshake is not available" );
        try
        {
            CFakeIrcWebSocketServer server;
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings;
            settings.server = "127.0.0.1";
            settings.port = IrcWebSocketPort;
            settings.useWebSocket = true;
            settings.webSocketPath = "/irc";
            settings.nick = "webtester";
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 5000 ) );
            ASSERT_TRUE( 1 == recorder.connected );
            ASSERT_TRUE( recorder.notices >= 1 );
            ASSERT_TRUE( server.subProtocol == "text.ircv3.net" );
            ASSERT_TRUE( server.requestUri == "/irc" );
            ASSERT_TRUE( client.HasCapability( "message-tags" ) );

            ASSERT_TRUE( client.PrivMsg( "#web", "hello over the web" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.messages, 1, 5000 ) );
            ASSERT_TRUE( server.lastPrivMsg == "hello over the web" );
            ASSERT_TRUE( recorder.lastMessage.sourceNick == "alice" );
            ASSERT_TRUE( recorder.lastMessage.text == "welcome over the web" );
            ASSERT_TRUE( recorder.lastMessage.msgId == "ws1" );

            // One line per message, without CR LF
            ASSERT_FALSE( server.lineHadLineBreak );

            client.Disconnect( "bye" );
            ASSERT_TRUE( PumpUntilTrue( server.quitReceived, 5000 ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The IRC client talks over a WebSocket with TLS (wss) when the settings say useTls" )
        GUCEF_TESTFW_SKIP_IF( !EnsureHashCodecPlugin(), "The sha1 codec for the WebSocket handshake is not available" );
        CORE::CString whyNoTls;
        GUCEF_TESTFW_SKIP_IF( !TESTFW::CTlsTestCerts::IsToolAvailable(), "the openssl command line tool is not available" );
        GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNoTls ), "this system has no TLS provider" );
        try
        {
            TESTFW::CTlsTestCerts certs;
            ASSERT_TRUE( certs.IsReady() );
            CFakeIrcWebSocketServer server;
            ASSERT_TRUE( server.Start() );
            TESTFW::CTlsTestTerminator terminator;
            ASSERT_TRUE( terminator.Start( certs.GetPath( "good.crt" ), certs.GetPath( "good.key" ), IrcWebSocketPort ) );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings;
            settings.server = "localhost";
            settings.port = terminator.GetPort();
            settings.useWebSocket = true;
            settings.useTls = true;
            settings.tls.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
            settings.tls.SetUseSystemTrust( false );
            settings.webSocketPath = "/irc";
            settings.nick = "securetester";
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 10000 ) );
            ASSERT_TRUE( server.subProtocol == "text.ircv3.net" );
            ASSERT_TRUE( client.PrivMsg( "#web", "hello over tls" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.messages, 1, 5000 ) );
            ASSERT_TRUE( server.lastPrivMsg == "hello over tls" );

            client.Disconnect( "bye" );
            ASSERT_TRUE( PumpUntilTrue( server.quitReceived, 5000 ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 5000 ) );

            // an untrusted certificate: nothing is registered, the error is reported
            COM::CIrcClient refused;
            CIrcRecorder refusedRecorder;
            refusedRecorder.SubscribeTo( &refused );
            COM::CIrcClientSettings untrusting = settings;
            untrusting.tls = COMCORE::CTlsSettings();
            untrusting.tls.SetCaBundlePath( certs.GetPath( "other_ca.crt" ) );
            untrusting.tls.SetUseSystemTrust( false );
            ASSERT_TRUE( refused.Connect( untrusting ) );
            ASSERT_TRUE( PumpUntilAtLeast( refusedRecorder.errors, 1, 10000 ) );
            ASSERT_TRUE( 0 == refusedRecorder.registered );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
