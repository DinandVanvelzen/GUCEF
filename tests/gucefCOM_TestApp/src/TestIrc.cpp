/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COM_CIRCMESSAGE_H
#include "gucefCOM_CIrcMessage.h"
#define GUCEF_COM_CIRCMESSAGE_H
#endif /* GUCEF_COM_CIRCMESSAGE_H ? */

#ifndef GUCEF_COM_CIRCCLIENT_H
#include "gucefCOM_CIrcClient.h"
#define GUCEF_COM_CIRCCLIENT_H
#endif /* GUCEF_COM_CIRCCLIENT_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestIrc.h"

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

const UInt16 FakeIrcServerPort = 47851;

/**
 *  An IRC server on loopback that plays its part in the registration, SASL, channels and messages, and keeps
 *  what the client sent
 */
class CFakeIrcServer : public CORE::CObserver
{
    public:

    bool offerSasl;
    CORE::CString nickInUse;
    bool answerPings;
    CORE::CString::StringVector receivedLines;
    bool saslAccepted;
    bool registered;
    bool quitReceived;
    CORE::CString quitMessage;
    CORE::CString lastPrivMsg;
    CORE::CString lastPong;
    UInt32 pingsReceived;
    CORE::CString capRequest;

    CFakeIrcServer( void )
        : CORE::CObserver()
        , offerSasl( true )
        , nickInUse()
        , answerPings( true )
        , receivedLines()
        , saslAccepted( false )
        , registered( false )
        , quitReceived( false )
        , quitMessage()
        , lastPrivMsg()
        , lastPong()
        , pingsReceived( 0 )
        , capRequest()
        , m_server( false )
        , m_connection( GUCEF_NULL )
        , m_buffer()
        , m_nick()
        , m_userSeen( false )
        , m_capEnded( false )
    {GUCEF_TRACE;

        SubscribeTo( &m_server );
    }

    virtual ~CFakeIrcServer()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        m_server.Close();
    }

    bool Start( void )
    {GUCEF_TRACE;

        return m_server.ListenOnPort( FakeIrcServerPort );
    }

    void SendLine( const CORE::CString& line )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_connection )
            return;
        CORE::CString wire = line + "\r\n";
        m_connection->Send( wire.C_String(), wire.ByteSize() - 1 );
    }

    void DropConnection( void )
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_connection )
            m_connection->Close();
    }

    bool HasReceived( const CORE::CString& command ) const
    {GUCEF_TRACE;

        for ( size_t i=0; i<receivedLines.size(); ++i )
        {
            COM::CIrcMessage message;
            if ( message.Parse( receivedLines[ i ] ) && message.command == command )
                return true;
        }
        return false;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CFakeIrcServer";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( notifier == &m_server )
        {
            if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventId )
            {
                COMCORE::CTCPServerSocket::TClientConnectedEventData* data = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData );
                m_connection = data->GetData().connection;
                SubscribeTo( m_connection );

                // Servers talk first
                SendLine( ":fake.server NOTICE * :*** Looking up your hostname" );
            }
            else
            if ( COMCORE::CTCPServerSocket::ClientDisconnectedEvent == eventId )
            {
                m_connection = GUCEF_NULL;
            }
            return;
        }

        if ( notifier == m_connection && COMCORE::CTCPConnection::DataRecievedEvent == eventId )
        {
            const CORE::CDynamicBuffer& data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData )->GetData();
            m_buffer.Append( data.AsConstTypePtr< char >(), data.GetDataSize() );
            while ( true )
            {
                Int32 end = m_buffer.HasSubstr( "\r\n", true );
                if ( end < 0 )
                    break;
                CORE::CString line = m_buffer.SubstrToIndex( (UInt32) end, true );
                m_buffer = m_buffer.CutChars( (UInt32) end + 2, true, 0 );
                OnLine( line );
            }
        }
    }

    private:

    void TryRegister( void )
    {GUCEF_TRACE;

        if ( !registered && m_userSeen && m_capEnded && !m_nick.IsNULLOrEmpty() )
        {
            registered = true;
            SendLine( ":fake.server 001 " + m_nick + " :Welcome to the fake network" );
            SendLine( ":fake.server 005 " + m_nick + " CHANTYPES=# :are supported by this server" );
        }
    }

    void OnLine( const CORE::CString& line )
    {GUCEF_TRACE;

        receivedLines.push_back( line );
        COM::CIrcMessage message;
        if ( !message.Parse( line ) )
            return;

        const CORE::CString& command = message.command;
        if ( "CAP" == command )
        {
            const CORE::CString& sub = message.GetParam( 0 );
            if ( "LS" == sub )
                SendLine( CORE::CString( ":fake.server CAP * LS :multi-prefix server-time message-tags account-tag" ) + ( offerSasl ? " sasl=PLAIN" : "" ) );
            else
            if ( "REQ" == sub )
            {
                capRequest = message.GetParam( 1 );
                SendLine( ":fake.server CAP * ACK :" + capRequest );
            }
            else
            if ( "END" == sub )
            {
                m_capEnded = true;
                TryRegister();
            }
        }
        else
        if ( "AUTHENTICATE" == command )
        {
            if ( "PLAIN" == message.GetParam( 0 ) )
            {
                SendLine( "AUTHENTICATE +" );
            }
            else
            {
                CORE::CDynamicBuffer decoded;
                decoded.CopyAndDecodeBase64From( message.GetParam( 0 ), 0 );
                const char expected[] = "acct\0acct\0secret";
                saslAccepted = decoded.GetDataSize() == sizeof( expected ) - 1 && 0 == memcmp( decoded.GetConstBufferPtr(), expected, sizeof( expected ) - 1 );
                if ( saslAccepted )
                {
                    SendLine( ":fake.server 900 * *!*@* acct :You are now logged in as acct" );
                    SendLine( ":fake.server 903 * :SASL authentication successful" );
                }
                else
                {
                    SendLine( ":fake.server 904 * :SASL authentication failed" );
                }
            }
        }
        else
        if ( "NICK" == command )
        {
            const CORE::CString& nick = message.GetParam( 0 );
            if ( !registered && nick == nickInUse )
            {
                SendLine( ":fake.server 433 * " + nick + " :Nickname is already in use" );
                return;
            }
            if ( registered )
                SendLine( ":" + m_nick + "!user@host NICK :" + nick );
            m_nick = nick;
            TryRegister();
        }
        else
        if ( "USER" == command )
        {
            m_userSeen = true;
            TryRegister();
        }
        else
        if ( "JOIN" == command )
        {
            const CORE::CString& channel = message.GetParam( 0 );
            SendLine( ":" + m_nick + "!user@host JOIN " + channel );
            SendLine( ":fake.server 332 " + m_nick + " " + channel + " :The topic of the day" );
            SendLine( ":fake.server 353 " + m_nick + " = " + channel + " :@op +voiced @+both " + m_nick );
            SendLine( ":fake.server 366 " + m_nick + " " + channel + " :End of /NAMES list." );
        }
        else
        if ( "PART" == command )
        {
            SendLine( ":" + m_nick + "!user@host PART " + message.GetParam( 0 ) + " :" + message.GetParam( 1 ) );
        }
        else
        if ( "PRIVMSG" == command )
        {
            lastPrivMsg = message.GetParam( 1 );
            SendLine( "@time=2020-01-02T03:04:05.000Z;msgid=abc123;account=bob;+custom=a\\sb :bob!b@host PRIVMSG " + message.GetParam( 0 ) + " :hi there" );
        }
        else
        if ( "PING" == command )
        {
            ++pingsReceived;
            if ( answerPings )
                SendLine( ":fake.server PONG fake.server :" + message.GetParam( 0 ) );
        }
        else
        if ( "PONG" == command )
        {
            lastPong = message.GetParam( message.GetParamCount() - 1 );
        }
        else
        if ( "QUIT" == command )
        {
            quitReceived = true;
            quitMessage = message.GetParam( 0 );
        }
    }

    COMCORE::CTCPServerSocket m_server;
    COMCORE::CTCPServerConnection* m_connection;
    CORE::CString m_buffer;
    CORE::CString m_nick;
    bool m_userSeen;
    bool m_capEnded;
};

/*-------------------------------------------------------------------------*/

/**
 *  Counts the events of an IRC client and keeps the data of the last of each kind that matters to the tests
 */
class CIrcRecorder : public CORE::CObserver
{
    public:

    UInt32 connected;
    UInt32 registered;
    UInt32 disconnected;
    UInt32 messages;
    UInt32 notices;
    UInt32 joins;
    UInt32 parts;
    UInt32 topics;
    UInt32 names;
    UInt32 nickChanges;
    UInt32 errors;
    UInt32 rawLines;
    COM::CIrcMessageEventData lastMessage;
    COM::CIrcMessageEventData lastTopic;
    COM::CIrcMessageEventData lastNickChange;

    CIrcRecorder( void )
        : CORE::CObserver()
        , connected( 0 )
        , registered( 0 )
        , disconnected( 0 )
        , messages( 0 )
        , notices( 0 )
        , joins( 0 )
        , parts( 0 )
        , topics( 0 )
        , names( 0 )
        , nickChanges( 0 )
        , errors( 0 )
        , rawLines( 0 )
        , lastMessage()
        , lastTopic()
        , lastNickChange()
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

        const COM::CIrcMessageEventData* data = static_cast< const COM::CIrcMessageEventData* >( eventData );
        if ( COM::CIrcClient::ConnectedEvent == eventId ) ++connected;
        else if ( COM::CIrcClient::RegisteredEvent == eventId ) ++registered;
        else if ( COM::CIrcClient::DisconnectedEvent == eventId ) ++disconnected;
        else if ( COM::CIrcClient::MessageEvent == eventId ) { ++messages; if ( GUCEF_NULL != data ) lastMessage = *data; }
        else if ( COM::CIrcClient::NoticeEvent == eventId ) ++notices;
        else if ( COM::CIrcClient::JoinEvent == eventId ) ++joins;
        else if ( COM::CIrcClient::PartEvent == eventId ) ++parts;
        else if ( COM::CIrcClient::TopicEvent == eventId ) { ++topics; if ( GUCEF_NULL != data ) lastTopic = *data; }
        else if ( COM::CIrcClient::NamesEvent == eventId ) ++names;
        else if ( COM::CIrcClient::NickChangedEvent == eventId ) { ++nickChanges; if ( GUCEF_NULL != data ) lastNickChange = *data; }
        else if ( COM::CIrcClient::ErrorEvent == eventId ) ++errors;
        else if ( COM::CIrcClient::RawLineEvent == eventId ) ++rawLines;
    }
};

/*-------------------------------------------------------------------------*/

COM::CIrcClientSettings
MakeSettings( const CORE::CString& nick )
{GUCEF_TRACE;

    COM::CIrcClientSettings settings;
    settings.server = "127.0.0.1";
    settings.port = FakeIrcServerPort;
    settings.nick = nick;
    settings.realName = "GUCEF test";
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString::StringVector& list, const CORE::CString& value )
{GUCEF_TRACE;

    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] == value )
            return true;
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIrcMessageTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "IrcMessage" );

    GUCEF_TESTFW_TESTCASE( "Test: A line with tags, prefix, parameters and trailing is taken apart" )
        try
        {
            COM::CIrcMessage message;
            ASSERT_TRUE( message.Parse( "@time=2020-01-02T03:04:05.000Z;msgid=x;flag;esc=a\\sb\\:c\\\\d :nick!user@host PRIVMSG #chan :hello there: you\r\n" ) );
            ASSERT_TRUE( message.command == "PRIVMSG" );
            ASSERT_TRUE( message.prefix == "nick!user@host" );
            ASSERT_TRUE( message.GetSourceNick() == "nick" );
            ASSERT_TRUE( message.GetSourceUser() == "user" );
            ASSERT_TRUE( message.GetSourceHost() == "host" );
            ASSERT_TRUE( 2 == message.GetParamCount() );
            ASSERT_TRUE( message.GetParam( 0 ) == "#chan" );
            ASSERT_TRUE( message.GetParam( 1 ) == "hello there: you" );
            ASSERT_TRUE( message.GetParam( 5 ).IsNULLOrEmpty() );
            ASSERT_TRUE( 4 == message.tags.size() );

            CORE::CString value;
            ASSERT_TRUE( message.GetTag( "msgid", value ) && value == "x" );
            ASSERT_TRUE( message.HasTag( "flag" ) );
            ASSERT_TRUE( message.GetTag( "esc", value ) && value == "a b;c\\d" );
            ASSERT_FALSE( message.IsNumeric() );

            COM::CIrcMessage numeric;
            ASSERT_TRUE( numeric.Parse( ":server 001 me :Welcome" ) );
            ASSERT_TRUE( numeric.IsNumeric() );
            ASSERT_TRUE( numeric.GetSourceNick() == "server" );

            COM::CIrcMessage bare;
            ASSERT_TRUE( bare.Parse( "PING token" ) );
            ASSERT_TRUE( bare.command == "PING" && bare.GetParam( 0 ) == "token" );

            COM::CIrcMessage empty;
            ASSERT_FALSE( empty.Parse( "" ) );
            ASSERT_FALSE( empty.Parse( "@a=b :prefix" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A message is written back the way it reads" )
        try
        {
            COM::CIrcMessage message( "PRIVMSG" );
            message.AddParam( "#chan" ).AddParam( "two words" );
            message.SetTag( "+reply", "a;b c" );
            CORE::CString line = message.ToString();
            ASSERT_TRUE( line == "@+reply=a\\:b\\sc PRIVMSG #chan :two words" );

            COM::CIrcMessage read;
            ASSERT_TRUE( read.Parse( line ) );
            CORE::CString value;
            ASSERT_TRUE( read.GetTag( "+reply", value ) && value == "a;b c" );
            ASSERT_TRUE( read.GetParam( 1 ) == "two words" );

            COM::CIrcMessage single( "NICK" );
            single.AddParam( "me" );
            ASSERT_TRUE( single.ToString() == "NICK me" );

            COM::CIrcMessage colon( "PRIVMSG" );
            colon.AddParam( "x" ).AddParam( ":)" );
            ASSERT_TRUE( colon.ToString() == "PRIVMSG x ::)" );

            COM::CIrcMessage emptyTrailing( "TOPIC" );
            emptyTrailing.AddParam( "#chan" ).AddParam( "" );
            ASSERT_TRUE( emptyTrailing.ToString() == "TOPIC #chan :" );

            ASSERT_TRUE( COM::CIrcClient::NamesAreEqual( "Nick[1]", "nick{1}" ) );
            ASSERT_FALSE( COM::CIrcClient::NamesAreEqual( "nick1", "nick2" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformIrcClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "IrcClient" );

    GUCEF_TESTFW_TESTCASE( "Test: The client registers with capability negotiation and SASL" )
        try
        {
            CFakeIrcServer server;
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings = MakeSettings( "tester" );
            settings.saslAccount = "acct";
            settings.saslPassword = "secret";
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 5000 ) );

            ASSERT_TRUE( 1 == recorder.connected );
            ASSERT_TRUE( client.IsConnected() );
            ASSERT_TRUE( client.IsRegistered() );
            ASSERT_TRUE( server.saslAccepted );
            ASSERT_TRUE( client.GetNick() == "tester" );
            ASSERT_TRUE( client.HasCapability( "sasl" ) );
            ASSERT_TRUE( client.HasCapability( "server-time" ) );
            ASSERT_TRUE( client.HasCapability( "multi-prefix" ) );
            ASSERT_FALSE( client.HasCapability( "echo-message" ) );
            ASSERT_TRUE( server.capRequest.HasSubstr( "echo-message" ) < 0 );
            ASSERT_TRUE( recorder.notices >= 1 );
            ASSERT_TRUE( recorder.rawLines >= 5 );

            // The order of the registration
            ASSERT_TRUE( server.receivedLines.size() >= 3 );
            if ( server.receivedLines.size() >= 3 )
            {
                ASSERT_TRUE( server.receivedLines[ 0 ] == "CAP LS 302" );
                ASSERT_TRUE( server.receivedLines[ 1 ] == "NICK tester" );
                ASSERT_TRUE( server.receivedLines[ 2 ].StartsWith( "USER tester 0 * :GUCEF test" ) );
            }

            client.Disconnect( "bye now" );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 5000 ) );
            ASSERT_TRUE( PumpUntilTrue( server.quitReceived, 2000 ) );
            ASSERT_TRUE( server.quitMessage == "bye now" );
            ASSERT_FALSE( client.IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A nick in use is retried with the alternatives, and without SASL there is no AUTHENTICATE" )
        try
        {
            CFakeIrcServer server;
            server.offerSasl = false;
            server.nickInUse = "taken";
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings = MakeSettings( "taken" );
            settings.alternativeNicks.push_back( "taken" );
            settings.alternativeNicks.push_back( "second" );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 5000 ) );
            ASSERT_TRUE( client.GetNick() == "second" );
            ASSERT_FALSE( server.HasReceived( "AUTHENTICATE" ) );
            ASSERT_FALSE( client.HasCapability( "sasl" ) );

            // A nick change after registration goes through the server
            ASSERT_TRUE( client.SetNick( "third" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.nickChanges, 1, 5000 ) );
            ASSERT_TRUE( client.GetNick() == "third" );
            ASSERT_TRUE( recorder.lastNickChange.sourceNick == "second" );
            ASSERT_TRUE( recorder.lastNickChange.text == "third" );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Channels, members, topics and tagged messages, auto join included" )
        try
        {
            CFakeIrcServer server;
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings = MakeSettings( "tester" );
            settings.autoJoinChannels.push_back( "#lobby" );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.names, 1, 5000 ) );
            ASSERT_TRUE( 1 == recorder.joins );
            ASSERT_TRUE( 1 == recorder.topics );
            ASSERT_TRUE( recorder.lastTopic.target == "#lobby" );
            ASSERT_TRUE( recorder.lastTopic.text == "The topic of the day" );

            CORE::CString::StringVector channels;
            client.GetChannels( channels );
            ASSERT_TRUE( 1 == channels.size() && channels[ 0 ] == "#lobby" );

            CORE::CString::StringVector members;
            ASSERT_TRUE( client.GetChannelMembers( "#LOBBY", members ) );
            ASSERT_TRUE( 4 == members.size() );
            ASSERT_TRUE( Contains( members, "op" ) && Contains( members, "voiced" ) && Contains( members, "both" ) && Contains( members, "tester" ) );
            CORE::CString prefix;
            ASSERT_TRUE( client.GetChannelMemberPrefix( "#lobby", "both", prefix ) && prefix == "@+" );
            ASSERT_TRUE( client.GetChannelMemberPrefix( "#lobby", "op", prefix ) && prefix == "@" );
            CORE::CString topic;
            ASSERT_TRUE( client.GetTopic( "#lobby", topic ) && topic == "The topic of the day" );

            ASSERT_TRUE( client.PrivMsg( "#lobby", "hello world" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.messages, 1, 5000 ) );
            ASSERT_TRUE( server.lastPrivMsg == "hello world" );
            const COM::CIrcMessageEventData& message = recorder.lastMessage;
            ASSERT_TRUE( message.sourceNick == "bob" );
            ASSERT_TRUE( message.target == "#lobby" );
            ASSERT_TRUE( message.text == "hi there" );
            ASSERT_TRUE( message.msgId == "abc123" );
            ASSERT_TRUE( message.account == "bob" );
            ASSERT_TRUE( 2020 == message.serverTime.GetYear() );
            ASSERT_TRUE( message.tags.GetValueAlways( "+custom" ).AsString() == "a b" );

            // Text with a line break must not become a second command
            ASSERT_TRUE( client.PrivMsg( "#lobby", "one\r\nQUIT :injected" ) );
            PumpFor( 200 );
            ASSERT_FALSE( server.quitReceived );

            ASSERT_TRUE( client.Part( "#lobby", "leaving" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.parts, 1, 5000 ) );
            channels.clear();
            client.GetChannels( channels );
            ASSERT_TRUE( channels.empty() );
            client.Disconnect();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: PING is answered, a silent server is pinged and given up on" )
        try
        {
            CFakeIrcServer server;
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );

            COM::CIrcClientSettings settings = MakeSettings( "tester" );
            settings.pingTimeoutInSecs = 1;
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 5000 ) );

            server.SendLine( "PING :token123" );
            PumpFor( 200 );
            ASSERT_TRUE( server.lastPong == "token123" );

            // Answered pings keep the connection
            ASSERT_TRUE( PumpUntilAtLeast( server.pingsReceived, 1, 4000 ) );
            PumpFor( 1500 );
            ASSERT_TRUE( 0 == recorder.disconnected );

            server.answerPings = false;
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 6000 ) );
            ASSERT_TRUE( recorder.errors >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A connection the server drops is reported" )
        try
        {
            CFakeIrcServer server;
            ASSERT_TRUE( server.Start() );

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );
            ASSERT_TRUE( client.Connect( MakeSettings( "tester" ) ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 5000 ) );

            server.SendLine( "ERROR :Closing link" );
            server.DropConnection();
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 5000 ) );
            ASSERT_TRUE( recorder.errors >= 1 );
            ASSERT_FALSE( client.IsRegistered() );

            // The transports are created by name
            ASSERT_TRUE( COM::CComGlobal::Instance()->GetIrcTransportFactory().IsConstructible( "tcp" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The client works with a real server when GUCEF_TEST_IRC_SERVER=host:port is set" )
        GUCEF_TESTFW_REQUIRE_ENV( serverSetting, "GUCEF_TEST_IRC_SERVER" );
        try
        {
            COMCORE::CHostAddress serverAddress( CORE::CString( serverSetting.c_str() ) );
            COM::CIrcClientSettings settings;
            settings.server = serverAddress.GetHostname();
            settings.port = 0 != serverAddress.GetPortInHostByteOrder() ? serverAddress.GetPortInHostByteOrder() : 6667;
            settings.nick = "gucef" + CORE::UInt32ToString( (UInt32)( MT::PrecisionTickCount() % 100000 ) );
            settings.pingTimeoutInSecs = 60;

            COM::CIrcClient client;
            CIrcRecorder recorder;
            recorder.SubscribeTo( &client );
            ASSERT_TRUE( client.Connect( settings ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.registered, 1, 20000 ) );
            ASSERT_TRUE( client.Join( "#gucef-test" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.names, 1, 10000 ) );
            CORE::CString::StringVector members;
            ASSERT_TRUE( client.GetChannelMembers( "#gucef-test", members ) );
            ASSERT_TRUE( Contains( members, client.GetNick() ) );
            ASSERT_TRUE( client.PrivMsg( "#gucef-test", "hello from the GUCEF test" ) );
            ASSERT_TRUE( client.Part( "#gucef-test" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.parts, 1, 10000 ) );
            client.Disconnect( "done" );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.disconnected, 1, 10000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
