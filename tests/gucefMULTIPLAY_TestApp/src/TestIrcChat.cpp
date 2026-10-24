/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_COM_CIRCMESSAGE_H
#include "gucefCOM_CIrcMessage.h"
#define GUCEF_COM_CIRCMESSAGE_H
#endif /* GUCEF_COM_CIRCMESSAGE_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#include "gucefMULTIPLAY_CIrcChatService.h"
#define GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CIRCCHATSERVICE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestIrcChat.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt16 FakeIrcServerPort = 47920;
static const UInt16 ClosedIrcPort = 47921;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A scripted IRC server for one client on loopback, after the one of the COM tests: it registers the client,
 *  answers JOIN with the topic and the names, PART with the PART, a PRIVMSG to a channel with one from "bob"
 *  and a PRIVMSG to a nick with one from that nick. SendLine() scripts anything else.
 */
class CFakeIrcServer : public CORE::CObserver
{
    public:

    CORE::CString::StringVector receivedLines;
    bool registered;

    CFakeIrcServer( void )
        : CORE::CObserver()
        , receivedLines()
        , registered( false )
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

    bool HasReceived( const CORE::CString& line ) const
    {GUCEF_TRACE;

        for ( size_t i=0; i<receivedLines.size(); ++i )
            if ( receivedLines[ i ] == line )
                return true;
        return false;
    }

    /** Whether a message with the command and the parameters came in, however the client wrote it */
    bool HasReceived( const CORE::CString& command, const CORE::CString& param0, const CORE::CString& param1 ) const
    {GUCEF_TRACE;

        for ( size_t i=0; i<receivedLines.size(); ++i )
        {
            COM::CIrcMessage message;
            if ( message.Parse( receivedLines[ i ] ) && message.command == command &&
                 message.GetParam( 0 ) == param0 && message.GetParam( 1 ) == param1 )
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
                SendLine( ":fake.server CAP * LS :multi-prefix server-time message-tags account-tag" );
            else
            if ( "REQ" == sub )
                SendLine( ":fake.server CAP * ACK :" + message.GetParam( 1 ) );
            else
            if ( "END" == sub )
            {
                m_capEnded = true;
                TryRegister();
            }
        }
        else
        if ( "NICK" == command )
        {
            m_nick = message.GetParam( 0 );
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
            SendLine( ":fake.server 353 " + m_nick + " = " + channel + " :@op +voiced " + m_nick );
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
            const CORE::CString& target = message.GetParam( 0 );
            if ( '#' == target.C_String()[ 0 ] )
                SendLine( "@time=2020-01-02T03:04:05.000Z;msgid=abc123 :bob!b@host PRIVMSG " + target + " :hi there" );
            else
                SendLine( ":" + target + "!u@host PRIVMSG " + m_nick + " :got it" );
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

/** Records what a chat service reports */
class CChatRecorder : public CORE::CObserver
{
    public:

    GUCEF::vector< MULTIPLAY::CChatMessage > messages;
    GUCEF::vector< MULTIPLAY::CChatPresenceEventData > presence;
    UInt32 topics;
    CORE::CString lastTopic;
    UInt32 errors;

    CChatRecorder( void ) : CORE::CObserver(), messages(), presence(), topics( 0 ), lastTopic(), errors( 0 ) {}

    UInt32 MessageCount( void ) const { return (UInt32) messages.size(); }
    UInt32 PresenceCount( void ) const { return (UInt32) presence.size(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CChatRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIChatService::ChatMessageEvent == eventId )
            messages.push_back( *static_cast< const MULTIPLAY::CChatMessage* >( eventData ) );
        else
        if ( MULTIPLAY::CIChatService::ChatPresenceEvent == eventId )
            presence.push_back( *static_cast< const MULTIPLAY::CChatPresenceEventData* >( eventData ) );
        else
        if ( MULTIPLAY::CIChatService::ChatTopicEvent == eventId )
        {
            ++topics;
            lastTopic = static_cast< const MULTIPLAY::CChatTopicEventData* >( eventData )->topic;
        }
        else
        if ( MULTIPLAY::CIChatService::ChatErrorEvent == eventId )
            ++errors;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Pulses until the future is ready */
static bool
PumpUntilReady( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
        PumpFor( 1 );
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

static bool
FutureSucceeded( const CORE::CFutureResult& future )
{
    if ( !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    return !task.IsNULL() && CORE::TTaskStatus::TASKSTATUS_FINISHED == task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

static bool
HasMember( const MULTIPLAY::CIChatChannel* channel, const char* member )
{
    return GUCEF_NULL != channel && channel->GetMembers().find( member ) != channel->GetMembers().end();
}

/*-------------------------------------------------------------------------*/

void
PerformIrcChatTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING irc chat TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "IrcChat" );

    GUCEF_TESTFW_TESTCASE( "Test: Channel names and nicks are mapped for IRC" )
        try
        {
            MULTIPLAY::CIrcChatService chat;
            ASSERT_TRUE( "#gucef-lobby-1" == chat.ToIrcChannel( "lobby-1" ) );
            ASSERT_TRUE( "lobby-1" == chat.FromIrcChannel( "#gucef-lobby-1" ) );
            ASSERT_TRUE( "lobby-1" == chat.FromIrcChannel( "#GUCEF-lobby-1" ) );
            ASSERT_TRUE( "#other" == chat.ToIrcChannel( "#other" ) && "#other" == chat.FromIrcChannel( "#other" ) );
            chat.SetChannelPrefix( CORE::CString::Empty );
            ASSERT_TRUE( "#lobby-1" == chat.ToIrcChannel( "lobby-1" ) && "lobby-1" == chat.FromIrcChannel( "#lobby-1" ) );

            ASSERT_TRUE( "Alice_Smith" == MULTIPLAY::CIrcChatService::MakeNick( "Alice Smith" ) );
            ASSERT_TRUE( "_9Lives" == MULTIPLAY::CIrcChatService::MakeNick( "9Lives!" ) );
            ASSERT_TRUE( "Player" == MULTIPLAY::CIrcChatService::MakeNick( "!!!" ) );

            MULTIPLAY::CIChatServicePtr made = MULTIPLAY::CMultiplayGlobal::Instance()->GetChatServiceFactory().Create( "irc" );
            REQUIRE_TRUE( !made.IsNULL() );
            CORE::CDataNode config;
            config.SetAttribute( "channelPrefix", CORE::CString( "game" ) );
            CORE::CDataNode* irc = config.AddChild( "Irc", GUCEF_DATATYPE_OBJECT );
            irc->SetAttribute( "server", CORE::CString( "irc.example.org" ) );
            irc->SetAttribute( "port", CORE::CString( "6697" ) );
            ASSERT_TRUE( made->LoadConfig( config ) );
            MULTIPLAY::CIrcChatService* ircChat = static_cast< MULTIPLAY::CIrcChatService* >( made.GetPointerAlways() );
            ASSERT_TRUE( "game" == ircChat->GetChannelPrefix() && "irc.example.org" == ircChat->GetIrcSettings().server );
            ASSERT_TRUE( 6697 == ircChat->GetIrcSettings().port && "irc" == made->GetServiceName() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Chat over IRC: connect, channels, messages, direct messages and presence" )
        try
        {
            CFakeIrcServer server;
            REQUIRE_TRUE( server.Start() );

            MULTIPLAY::CIrcChatService chat;
            COM::CIrcClientSettings settings;
            settings.server = "127.0.0.1";
            settings.port = FakeIrcServerPort;
            chat.SetIrcSettings( settings );
            CChatRecorder recorder;
            recorder.SubscribeTo( &chat );

            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "alice-id";
            identity.displayName = "Alice Smith";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( chat.Connect( identity, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) && FutureSucceeded( future ) );
            ASSERT_TRUE( chat.IsConnected() && "Alice_Smith" == chat.GetLocalUserName() );

            // Joining: the presence, the topic and the members
            ASSERT_TRUE( chat.JoinChannel( "lobby-1" ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.topics, 1, 5000 ) );
            PumpFor( 50 );
            ASSERT_TRUE( server.HasReceived( "JOIN", "#gucef-lobby-1", CORE::CString::Empty ) );
            ASSERT_TRUE( 1 <= recorder.PresenceCount() );
            if ( 1 <= recorder.PresenceCount() )
            {
                ASSERT_TRUE( "lobby-1" == recorder.presence[ 0 ].channel && "Alice_Smith" == recorder.presence[ 0 ].user );
                ASSERT_TRUE( MULTIPLAY::CChatPresenceEventData::PRESENCE_JOINED == recorder.presence[ 0 ].presence );
            }
            ASSERT_TRUE( "The topic of the day" == recorder.lastTopic );
            const MULTIPLAY::CIChatChannel* channel = chat.GetChannel( "lobby-1" );
            REQUIRE_TRUE( GUCEF_NULL != channel );
            ASSERT_TRUE( "lobby-1" == channel->GetName() && "The topic of the day" == channel->GetTopic() );
            ASSERT_TRUE( HasMember( channel, "op" ) && HasMember( channel, "voiced" ) && HasMember( channel, "Alice_Smith" ) );
            CORE::CString::StringVector channels;
            chat.GetChannels( channels );
            ASSERT_TRUE( 1 == channels.size() && "lobby-1" == channels[ 0 ] );

            // Others come and go
            server.SendLine( ":bob!b@host JOIN #gucef-lobby-1" );
            server.SendLine( ":carol!c@host JOIN #gucef-lobby-1" );
            PumpFor( 100 );
            ASSERT_TRUE( HasMember( chat.GetChannel( "lobby-1" ), "bob" ) && HasMember( chat.GetChannel( "lobby-1" ), "carol" ) );

            // A message to the channel: our own (no echo-message) and the answer of bob
            UInt32 before = recorder.MessageCount();
            ASSERT_TRUE( chat.SendToChannel( "lobby-1", "hello" ) );
            PumpFor( 100 );
            ASSERT_TRUE( server.HasReceived( "PRIVMSG", "#gucef-lobby-1", "hello" ) );
            ASSERT_TRUE( before + 2 == recorder.MessageCount() );
            if ( before + 2 == recorder.MessageCount() )
            {
                const MULTIPLAY::CChatMessage& own = recorder.messages[ before ];
                ASSERT_TRUE( "lobby-1" == own.channel && "Alice_Smith" == own.from && "hello" == own.text && !own.isDirect );
                const MULTIPLAY::CChatMessage& answer = recorder.messages[ before + 1 ];
                ASSERT_TRUE( "lobby-1" == answer.channel && "bob" == answer.from && "hi there" == answer.text );
                ASSERT_TRUE( "abc123" == answer.id && 2020 == answer.time.GetYear() );
            }

            // A direct message and the answer
            before = recorder.MessageCount();
            ASSERT_TRUE( chat.SendDirect( "bob", "psst" ) );
            ASSERT_FALSE( chat.SendDirect( "#channel", "no" ) );
            PumpFor( 100 );
            ASSERT_TRUE( server.HasReceived( "PRIVMSG", "bob", "psst" ) );
            ASSERT_TRUE( before + 2 == recorder.MessageCount() );
            if ( before + 2 == recorder.MessageCount() )
            {
                ASSERT_TRUE( recorder.messages[ before ].isDirect && "bob" == recorder.messages[ before ].to );
                const MULTIPLAY::CChatMessage& answer = recorder.messages[ before + 1 ];
                ASSERT_TRUE( answer.isDirect && "bob" == answer.from && "Alice_Smith" == answer.to && "got it" == answer.text );
                ASSERT_TRUE( answer.channel.IsNULLOrEmpty() );
            }

            // Leaving: PART and QUIT
            UInt32 presenceBefore = recorder.PresenceCount();
            server.SendLine( ":bob!b@host PART #gucef-lobby-1 :bye" );
            server.SendLine( ":carol!c@host QUIT :gone" );
            PumpFor( 100 );
            ASSERT_TRUE( presenceBefore + 2 == recorder.PresenceCount() );
            if ( presenceBefore + 2 == recorder.PresenceCount() )
            {
                const MULTIPLAY::CChatPresenceEventData& part = recorder.presence[ presenceBefore ];
                ASSERT_TRUE( "bob" == part.user && "lobby-1" == part.channel && "bye" == part.reason );
                ASSERT_TRUE( MULTIPLAY::CChatPresenceEventData::PRESENCE_LEFT == part.presence );
                const MULTIPLAY::CChatPresenceEventData& quit = recorder.presence[ presenceBefore + 1 ];
                ASSERT_TRUE( "carol" == quit.user && "lobby-1" == quit.channel && "gone" == quit.reason );
            }
            ASSERT_FALSE( HasMember( chat.GetChannel( "lobby-1" ), "bob" ) || HasMember( chat.GetChannel( "lobby-1" ), "carol" ) );

            // A topic change
            server.SendLine( ":op!o@host TOPIC #gucef-lobby-1 :New topic" );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.topics, 2, 5000 ) );
            ASSERT_TRUE( "New topic" == recorder.lastTopic && "New topic" == chat.GetChannel( "lobby-1" )->GetTopic() );

            ASSERT_TRUE( chat.LeaveChannel( "lobby-1" ) );
            PumpFor( 100 );
            ASSERT_TRUE( GUCEF_NULL == chat.GetChannel( "lobby-1" ) );

            chat.Disconnect();
            ASSERT_FALSE( chat.IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Connecting to a server that is not there fails the future" )
        try
        {
            MULTIPLAY::CIrcChatService chat;
            COM::CIrcClientSettings settings;
            settings.server = "127.0.0.1";
            settings.port = ClosedIrcPort;
            chat.SetIrcSettings( settings );

            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "alice-id";
            identity.displayName = "Alice";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            chat.Connect( identity, future );
            ASSERT_TRUE( PumpUntilReady( future, 10000 ) );
            ASSERT_FALSE( FutureSucceeded( future ) );
            ASSERT_FALSE( chat.IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
