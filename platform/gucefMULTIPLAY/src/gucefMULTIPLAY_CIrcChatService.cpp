/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CIrcChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CIrcChatService::ServiceName = "irc";
const CORE::CString CIrcChatService::DefaultChannelPrefix = "gucef";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcChatService::CIrcChatService( void )
    : CIChatService()
    , m_client()
    , m_settings()
    , m_channelPrefix( DefaultChannelPrefix )
    , m_connectResult()
    , m_connectPending( false )
    , m_channels()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CIrcChatService::CIrcChatService( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CIChatService()
    , m_client( pulseGenerator )
    , m_settings()
    , m_channelPrefix( DefaultChannelPrefix )
    , m_connectResult()
    , m_connectPending( false )
    , m_channels()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CIrcChatService::~CIrcChatService()
{GUCEF_TRACE;

    UnsubscribeFrom( &m_client );
    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The chat service was destroyed" );
    }
    m_client.Disconnect();
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::RegisterEventHandlers( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    // A browser has no plain sockets
    m_settings.useWebSocket = true;
    #endif

    SubscribeTo( &m_client );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "A new connect replaced this one" );
    }
    m_client.Disconnect();
    m_channels.clear();

    COM::CIrcClientSettings settings( m_settings );
    if ( settings.nick.IsNULLOrEmpty() )
        settings.nick = MakeNick( identity.displayName );
    if ( settings.realName.IsNULLOrEmpty() )
        settings.realName = identity.displayName;

    m_connectResult = CORE::CFutureResultSource();
    future = m_connectResult.GetFuture();
    m_connectPending = true;

    if ( !m_client.Connect( settings ) )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Could not connect to the IRC server" );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "IrcChatService: Could not connect to " << settings.server << ":" << settings.port );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::Disconnect( void )
{GUCEF_TRACE;

    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "Disconnected before the server accepted the user" );
    }
    m_client.Disconnect();
    m_channels.clear();
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_client.IsRegistered();
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::JoinChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( channel.IsNULLOrEmpty() )
        return false;
    return m_client.Join( ToIrcChannel( channel ) );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::LeaveChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( channel.IsNULLOrEmpty() )
        return false;
    return m_client.Part( ToIrcChannel( channel ) );
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::GetChannels( CORE::CString::StringVector& channels ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        channels.push_back( (*i).second.name );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CIChatChannel*
CIrcChatService::GetChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.find( FoldName( ToIrcChannel( channel ) ) );
    if ( i != m_channels.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::SendToChannel( const CORE::CString& channel, const CORE::CString& text )
{GUCEF_TRACE;

    if ( !m_client.PrivMsg( ToIrcChannel( channel ), text ) )
        return false;

    if ( !EchoesOwnMessages() )
    {
        CChatMessage message;
        message.channel = FromIrcChannel( ToIrcChannel( channel ) );
        message.from = m_client.GetNick();
        message.text = text;
        message.time = CORE::CDateTime::NowUTCDateTime();
        NotifyObservers( ChatMessageEvent, &message );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::SendDirect( const CORE::CString& user, const CORE::CString& text )
{GUCEF_TRACE;

    if ( user.IsNULLOrEmpty() || IsChannelName( user ) )
        return false;
    if ( !m_client.PrivMsg( user, text ) )
        return false;

    if ( !EchoesOwnMessages() )
    {
        CChatMessage message;
        message.isDirect = true;
        message.from = m_client.GetNick();
        message.to = user;
        message.text = text;
        message.time = CORE::CDateTime::NowUTCDateTime();
        NotifyObservers( ChatMessageEvent, &message );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcChatService::GetLocalUserName( void ) const
{GUCEF_TRACE;

    return m_client.GetNick();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcChatService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::SetIrcSettings( const COM::CIrcClientSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const COM::CIrcClientSettings&
CIrcChatService::GetIrcSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::SetChannelPrefix( const CORE::CString& prefix )
{GUCEF_TRACE;

    m_channelPrefix = prefix;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcChatService::GetChannelPrefix( void ) const
{GUCEF_TRACE;

    return m_channelPrefix;
}

/*-------------------------------------------------------------------------*/

COM::CIrcClient&
CIrcChatService::GetIrcClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::IsChannelName( const CORE::CString& target )
{GUCEF_TRACE;

    if ( target.IsNULLOrEmpty() )
        return false;
    char first = target.C_String()[ 0 ];
    return '#' == first || '&' == first;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcChatService::FoldName( const CORE::CString& name )
{GUCEF_TRACE;

    // rfc1459 case mapping, as the server compares names
    CORE::CString folded = name.Lowercase();
    folded = folded.ReplaceChar( '[', '{' );
    folded = folded.ReplaceChar( ']', '}' );
    folded = folded.ReplaceChar( '\\', '|' );
    folded = folded.ReplaceChar( '~', '^' );
    return folded;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcChatService::ToIrcChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    if ( IsChannelName( channel ) )
        return channel;
    if ( m_channelPrefix.IsNULLOrEmpty() )
        return "#" + channel;
    return "#" + m_channelPrefix + "-" + channel;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcChatService::FromIrcChannel( const CORE::CString& ircChannel ) const
{GUCEF_TRACE;

    if ( !IsChannelName( ircChannel ) )
        return ircChannel;
    CORE::CString lead = m_channelPrefix.IsNULLOrEmpty() ? CORE::CString( "#" ) : "#" + m_channelPrefix + "-";
    if ( ircChannel.Length() > lead.Length() && FoldName( ircChannel.SubstrToIndex( lead.Length(), true ) ) == FoldName( lead ) )
        return ircChannel.CutChars( lead.Length(), true );
    return ircChannel;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcChatService::MakeNick( const CORE::CString& displayName )
{GUCEF_TRACE;

    // Letters, digits and []\`_^{|}- , not starting with a digit or '-'
    static const UInt32 maxNickLength = 30;
    const char* str = displayName.C_String();
    UInt32 size = displayName.ByteSize() > 0 ? displayName.ByteSize() - 1 : 0;
    char nick[ maxNickLength + 1 ];
    UInt32 length = 0;
    for ( UInt32 i=0; i<size && length<maxNickLength; ++i )
    {
        char c = str[ i ];
        bool letter = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
        bool digit = c >= '0' && c <= '9';
        bool special = GUCEF_NULL != strchr( "[]\\`_^{|}-", c ) && '\0' != c;
        if ( ' ' == c )
        {
            c = '_';
            special = true;
        }
        if ( !letter && !digit && !special )
            continue;
        if ( 0 == length && ( digit || '-' == c ) )
            nick[ length++ ] = '_';
        if ( length < maxNickLength )
            nick[ length++ ] = c;
    }
    nick[ length ] = '\0';
    if ( 0 == length )
        return "Player";
    return CORE::CString( nick );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::EchoesOwnMessages( void ) const
{GUCEF_TRACE;

    return m_client.HasCapability( "echo-message" );
}

/*-------------------------------------------------------------------------*/

CChatChannel*
CIrcChatService::FindChannel( const CORE::CString& ircChannel )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.find( FoldName( ircChannel ) );
    if ( i != m_channels.end() )
        return &(*i).second;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnRegistered( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IrcChatService: Registered on the IRC server as " << m_client.GetNick() );
    if ( m_connectPending )
    {
        m_connectPending = false;
        CORE::TCloneableString nick( m_client.GetNick() );
        m_connectResult.Complete( nick.Clone(), true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnDisconnected( void )
{GUCEF_TRACE;

    m_channels.clear();
    if ( m_connectPending )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "The IRC server closed the connection" );
    }
    TChatErrorEventData error( "Disconnected from the IRC server" );
    return NotifyObservers( ChatErrorEvent, &error );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnMessage( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    CChatMessage message;
    message.from = data.sourceNick;
    message.text = data.text;
    message.time = data.serverTime;
    message.id = data.msgId;

    if ( IsChannelName( data.target ) )
    {
        message.channel = FromIrcChannel( data.target );
    }
    else
    {
        // To our nick, or the echo of what we sent to someone
        message.isDirect = true;
        message.to = data.target;
    }
    return NotifyObservers( ChatMessageEvent, &message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnJoin( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    CORE::CString name = FromIrcChannel( data.target );
    if ( COM::CIrcClient::NamesAreEqual( data.sourceNick, m_client.GetNick() ) )
    {
        CChatChannel& channel = m_channels[ FoldName( data.target ) ];
        channel.name = name;
        channel.members.insert( data.sourceNick );
    }
    else
    {
        CChatChannel* channel = FindChannel( data.target );
        if ( GUCEF_NULL != channel )
            channel->members.insert( data.sourceNick );
    }

    CChatPresenceEventData presence( name, data.sourceNick, CChatPresenceEventData::PRESENCE_JOINED, CORE::CString::Empty );
    return NotifyObservers( ChatPresenceEvent, &presence );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnPart( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    CORE::CString name = FromIrcChannel( data.target );
    if ( COM::CIrcClient::NamesAreEqual( data.sourceNick, m_client.GetNick() ) )
    {
        m_channels.erase( FoldName( data.target ) );
    }
    else
    {
        CChatChannel* channel = FindChannel( data.target );
        if ( GUCEF_NULL != channel )
        {
            CORE::CString::StringSet::iterator m = channel->members.begin();
            while ( m != channel->members.end() )
            {
                if ( COM::CIrcClient::NamesAreEqual( (*m), data.sourceNick ) )
                {
                    channel->members.erase( m );
                    break;
                }
                ++m;
            }
        }
    }

    CChatPresenceEventData presence( name, data.sourceNick, CChatPresenceEventData::PRESENCE_LEFT, data.text );
    return NotifyObservers( ChatPresenceEvent, &presence );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnQuit( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    // The user leaves every channel we share with it, reported per channel
    CORE::CString::StringVector leftChannels;
    TChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        CORE::CString::StringSet& members = (*i).second.members;
        CORE::CString::StringSet::iterator m = members.begin();
        while ( m != members.end() )
        {
            if ( COM::CIrcClient::NamesAreEqual( (*m), data.sourceNick ) )
            {
                members.erase( m );
                leftChannels.push_back( (*i).second.name );
                break;
            }
            ++m;
        }
        ++i;
    }

    if ( leftChannels.empty() )
    {
        CChatPresenceEventData presence( CORE::CString::Empty, data.sourceNick, CChatPresenceEventData::PRESENCE_LEFT, data.text );
        return NotifyObservers( ChatPresenceEvent, &presence );
    }
    for ( size_t n=0; n<leftChannels.size(); ++n )
    {
        CChatPresenceEventData presence( leftChannels[ n ], data.sourceNick, CChatPresenceEventData::PRESENCE_LEFT, data.text );
        if ( !NotifyObservers( ChatPresenceEvent, &presence ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnNickChanged( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        CORE::CString::StringSet& members = (*i).second.members;
        CORE::CString::StringSet::iterator m = members.begin();
        while ( m != members.end() )
        {
            if ( COM::CIrcClient::NamesAreEqual( (*m), data.sourceNick ) )
            {
                members.erase( m );
                members.insert( data.text );
                break;
            }
            ++m;
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnTopic( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    CChatChannel* channel = FindChannel( data.target );
    if ( GUCEF_NULL != channel )
        channel->topic = data.text;

    CChatTopicEventData topic( FromIrcChannel( data.target ), data.text );
    return NotifyObservers( ChatTopicEvent, &topic );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnNames( const COM::CIrcMessageEventData& data )
{GUCEF_TRACE;

    CChatChannel* channel = FindChannel( data.target );
    if ( GUCEF_NULL == channel )
        return true;

    CORE::CString::StringVector members;
    m_client.GetChannelMembers( data.target, members );
    channel->members.clear();
    channel->members.insert( members.begin(), members.end() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::OnError( const COM::CIrcMessageEventData* data )
{GUCEF_TRACE;

    CORE::CString description = GUCEF_NULL != data ? data->text : CORE::CString( "IRC error" );

    // An error before the connection is up, such as a refused connection, ends the connect
    if ( m_connectPending && !m_client.IsConnected() )
    {
        m_connectPending = false;
        m_connectResult.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, description );
    }

    TChatErrorEventData error( description );
    return NotifyObservers( ChatErrorEvent, &error );
}

/*-------------------------------------------------------------------------*/

void
CIrcChatService::OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier != &m_client )
        return;

    if ( COM::CIrcClient::RegisteredEvent == eventId )
    {
        OnRegistered();
        return;
    }
    if ( COM::CIrcClient::DisconnectedEvent == eventId )
    {
        OnDisconnected();
        return;
    }

    const COM::CIrcMessageEventData* data = static_cast< const COM::CIrcMessageEventData* >( eventData );
    if ( COM::CIrcClient::ErrorEvent == eventId )
    {
        OnError( data );
        return;
    }
    if ( GUCEF_NULL == data )
        return;

    if ( COM::CIrcClient::MessageEvent == eventId )
        OnMessage( *data );
    else
    if ( COM::CIrcClient::JoinEvent == eventId )
        OnJoin( *data );
    else
    if ( COM::CIrcClient::PartEvent == eventId )
        OnPart( *data );
    else
    if ( COM::CIrcClient::QuitEvent == eventId )
        OnQuit( *data );
    else
    if ( COM::CIrcClient::NickChangedEvent == eventId )
        OnNickChanged( *data );
    else
    if ( COM::CIrcClient::TopicEvent == eventId )
        OnTopic( *data );
    else
    if ( COM::CIrcClient::NamesEvent == eventId )
        OnNames( *data );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "channelPrefix", m_channelPrefix );
    CORE::CDataNode* ircNode = config.AddChild( "Irc", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == ircNode )
        return false;
    return m_settings.SaveConfig( *ircNode );
}

/*-------------------------------------------------------------------------*/

bool
CIrcChatService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_channelPrefix = config.GetAttributeValueOrChildValueByName( "channelPrefix" ).AsString( m_channelPrefix, true );
    const CORE::CDataNode* ircNode = config.FindChild( "Irc" );
    if ( GUCEF_NULL != ircNode )
    {
        // The nick may come from the identity, so a config without one is fine
        m_settings.LoadConfig( *ircNode );
    }
    return !m_settings.server.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcChatService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CIrcChatService";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
