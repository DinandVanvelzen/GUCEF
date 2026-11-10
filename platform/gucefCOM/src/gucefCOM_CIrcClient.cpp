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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#include "gucefCOM_CIrcClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIrcClient::ConnectedEvent = "GUCEF::COM::CIrcClient::ConnectedEvent";
const CORE::CEvent CIrcClient::RegisteredEvent = "GUCEF::COM::CIrcClient::RegisteredEvent";
const CORE::CEvent CIrcClient::DisconnectedEvent = "GUCEF::COM::CIrcClient::DisconnectedEvent";
const CORE::CEvent CIrcClient::MessageEvent = "GUCEF::COM::CIrcClient::MessageEvent";
const CORE::CEvent CIrcClient::NoticeEvent = "GUCEF::COM::CIrcClient::NoticeEvent";
const CORE::CEvent CIrcClient::JoinEvent = "GUCEF::COM::CIrcClient::JoinEvent";
const CORE::CEvent CIrcClient::PartEvent = "GUCEF::COM::CIrcClient::PartEvent";
const CORE::CEvent CIrcClient::QuitEvent = "GUCEF::COM::CIrcClient::QuitEvent";
const CORE::CEvent CIrcClient::NickChangedEvent = "GUCEF::COM::CIrcClient::NickChangedEvent";
const CORE::CEvent CIrcClient::TopicEvent = "GUCEF::COM::CIrcClient::TopicEvent";
const CORE::CEvent CIrcClient::NamesEvent = "GUCEF::COM::CIrcClient::NamesEvent";
const CORE::CEvent CIrcClient::ErrorEvent = "GUCEF::COM::CIrcClient::ErrorEvent";
const CORE::CEvent CIrcClient::RawLineEvent = "GUCEF::COM::CIrcClient::RawLineEvent";

static const UInt32 TimerIntervalInMs = 1000;
static const UInt32 SaslChunkSize = 400;
static const char* MemberPrefixChars = "~&@%+";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsMemberPrefixChar( char c )
{GUCEF_TRACE;

    for ( const char* p = MemberPrefixChars; '\0' != *p; ++p )
        if ( *p == c )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The capability names of a CAP list, without their values: "sasl=PLAIN,EXTERNAL multi-prefix" */
void
ParseCapabilityList( const CORE::CString& list, CORE::CString::StringSet& names )
{GUCEF_TRACE;

    CORE::CString::StringVector tokens = list.ParseElements( ' ', false );
    for ( size_t i=0; i<tokens.size(); ++i )
    {
        CORE::CString name = tokens[ i ];
        Int32 equals = name.HasChar( '=' );
        if ( equals >= 0 )
            name = name.SubstrToIndex( (UInt32) equals, true );
        if ( !name.IsNULLOrEmpty() )
            names.insert( name );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcMessageEventData::CIrcMessageEventData( void )
    : CORE::CICloneable()
    , sourceNick()
    , target()
    , text()
    , serverTime()
    , msgId()
    , account()
    , tags()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessageEventData::CIrcMessageEventData( const CIrcMessageEventData& src )
    : CORE::CICloneable( src )
    , sourceNick( src.sourceNick )
    , target( src.target )
    , text( src.text )
    , serverTime( src.serverTime )
    , msgId( src.msgId )
    , account( src.account )
    , tags( src.tags )
    , message( src.message )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessageEventData::~CIrcMessageEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessageEventData&
CIrcMessageEventData::operator=( const CIrcMessageEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        sourceNick = src.sourceNick;
        target = src.target;
        text = src.text;
        serverTime = src.serverTime;
        msgId = src.msgId;
        account = src.account;
        tags = src.tags;
        message = src.message;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CIrcMessageEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CIrcMessageEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    RegisteredEvent.Initialize();
    DisconnectedEvent.Initialize();
    MessageEvent.Initialize();
    NoticeEvent.Initialize();
    JoinEvent.Initialize();
    PartEvent.Initialize();
    QuitEvent.Initialize();
    NickChangedEvent.Initialize();
    TopicEvent.Initialize();
    NamesEvent.Initialize();
    ErrorEvent.Initialize();
    RawLineEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIrcClient::CIrcClient( void )
    : CORE::CObservingNotifier()
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_settings()
    , m_transport()
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_isConnected( false )
    , m_isRegistered( false )
    , m_capNegotiationDone( false )
    , m_saslInProgress( false )
    , m_nick()
    , m_nickAttempt( 0 )
    , m_offeredCapabilities()
    , m_enabledCapabilities()
    , m_channels()
    , m_lastReceivedInMs( 0 )
    , m_pingSentInMs( 0 )
    , m_pingSent( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CIrcClient::CIrcClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_settings()
    , m_transport()
    , m_timer( m_pulseGenerator, TimerIntervalInMs )
    , m_isConnected( false )
    , m_isRegistered( false )
    , m_capNegotiationDone( false )
    , m_saslInProgress( false )
    , m_nick()
    , m_nickAttempt( 0 )
    , m_offeredCapabilities()
    , m_enabledCapabilities()
    , m_channels()
    , m_lastReceivedInMs( 0 )
    , m_pingSentInMs( 0 )
    , m_pingSent( false )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CIrcClient::~CIrcClient()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    if ( !m_transport.IsNULL() )
    {
        m_transport->Disconnect();
        m_transport.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CIrcClient::GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcClient::FoldName( const CORE::CString& name )
{GUCEF_TRACE;

    // rfc1459 case mapping: []\~ are the upper case of {}|^
    const char* str = name.C_String();
    UInt32 size = name.ByteSize() > 0 ? name.ByteSize() - 1 : 0;
    CORE::CString folded( str, size );
    char* out = folded.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = out[ i ];
        if ( c >= 'A' && c <= 'Z' ) c = (char)( c - 'A' + 'a' );
        else if ( '[' == c ) c = '{';
        else if ( ']' == c ) c = '}';
        else if ( '\\' == c ) c = '|';
        else if ( '~' == c ) c = '^';
        out[ i ] = c;
    }
    return folded;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::NamesAreEqual( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    return FoldName( a ) == FoldName( b );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcClient::SanitizeText( const CORE::CString& text )
{GUCEF_TRACE;

    // A line break in the text would end the line and start another command
    if ( text.HasChar( '\r' ) < 0 && text.HasChar( '\n' ) < 0 )
        return text;
    return text.ReplaceChar( '\r', ' ' ).ReplaceChar( '\n', ' ' );
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::Reset( void )
{GUCEF_TRACE;

    m_isConnected = false;
    m_isRegistered = false;
    m_capNegotiationDone = false;
    m_saslInProgress = false;
    m_nickAttempt = 0;
    m_offeredCapabilities.clear();
    m_enabledCapabilities.clear();
    m_channels.clear();
    m_pingSent = false;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Connect( const CIrcClientSettings& settings )
{GUCEF_TRACE;

    if ( settings.server.IsNULLOrEmpty() || settings.nick.IsNULLOrEmpty() )
        return false;

    if ( !m_transport.IsNULL() )
    {
        UnsubscribeFrom( m_transport.GetPointerAlways() );
        m_transport->Disconnect();
        m_transport.Unlink();
    }
    Reset();
    m_settings = settings;
    m_nick = settings.nick;

    const CORE::CString& transportName = settings.useWebSocket ? CIIrcTransport::WebSocketTransportName : CIIrcTransport::TcpTransportName;
    m_transport = CComGlobal::Instance()->GetIrcTransportFactory().Create( transportName );
    if ( m_transport.IsNULL() )
    {
        ReportError( "There is no IRC transport \"" + transportName + "\"", GUCEF_NULL );
        return false;
    }
    SubscribeTo( m_transport.GetPointerAlways() );

    if ( !m_transport->Connect( settings ) )
    {
        ReportError( "Unable to connect to " + settings.server + ":" + CORE::UInt16ToString( settings.port ), GUCEF_NULL );
        return false;
    }
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::Disconnect( const CORE::CString& message )
{GUCEF_TRACE;

    if ( m_transport.IsNULL() )
        return;

    if ( m_transport->IsConnected() )
    {
        CIrcMessage quit( "QUIT" );
        if ( !message.IsNULLOrEmpty() )
            quit.AddParam( SanitizeText( message ) );
        Send( quit );
    }
    m_transport->Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_isConnected;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::IsRegistered( void ) const
{GUCEF_TRACE;

    return m_isRegistered;
}

/*-------------------------------------------------------------------------*/

const CIrcClientSettings&
CIrcClient::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::SendLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( m_transport.IsNULL() || !m_transport->IsConnected() )
        return false;
    return m_transport->SendLine( line );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Send( const CIrcMessage& message )
{GUCEF_TRACE;

    return SendLine( message.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::SendRaw( const CORE::CString& line )
{GUCEF_TRACE;

    return SendLine( SanitizeText( line ) );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Join( const CORE::CString& channel, const CORE::CString& key )
{GUCEF_TRACE;

    CIrcMessage message( "JOIN" );
    message.AddParam( channel );
    if ( !key.IsNULLOrEmpty() )
        message.AddParam( key );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Part( const CORE::CString& channel, const CORE::CString& reason )
{GUCEF_TRACE;

    CIrcMessage message( "PART" );
    message.AddParam( channel );
    if ( !reason.IsNULLOrEmpty() )
        message.AddParam( SanitizeText( reason ) );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::SetTopic( const CORE::CString& channel, const CORE::CString& topic )
{GUCEF_TRACE;

    CIrcMessage message( "TOPIC" );
    message.AddParam( channel ).AddParam( SanitizeText( topic ) );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::PrivMsg( const CORE::CString& target, const CORE::CString& text )
{GUCEF_TRACE;

    CIrcMessage message( "PRIVMSG" );
    message.AddParam( target ).AddParam( SanitizeText( text ) );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Notice( const CORE::CString& target, const CORE::CString& text )
{GUCEF_TRACE;

    CIrcMessage message( "NOTICE" );
    message.AddParam( target ).AddParam( SanitizeText( text ) );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcClient::GetNick( void ) const
{GUCEF_TRACE;

    return m_nick;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::SetNick( const CORE::CString& nick )
{GUCEF_TRACE;

    if ( nick.IsNULLOrEmpty() )
        return false;

    if ( !m_isRegistered )
    {
        m_nick = nick;
        m_settings.nick = nick;
        if ( m_isConnected )
        {
            CIrcMessage message( "NICK" );
            message.AddParam( nick );
            return Send( message );
        }
        return true;
    }

    // The server tells when it took: the NICK message about ourselves
    CIrcMessage message( "NICK" );
    message.AddParam( nick );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::HasCapability( const CORE::CString& name ) const
{GUCEF_TRACE;

    return m_enabledCapabilities.find( name ) != m_enabledCapabilities.end();
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::IsOwnNick( const CORE::CString& nick ) const
{GUCEF_TRACE;

    return NamesAreEqual( nick, m_nick );
}

/*-------------------------------------------------------------------------*/

CIrcClient::TChannel*
CIrcClient::FindChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.find( FoldName( channel ) );
    return i != m_channels.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CIrcClient::TChannel*
CIrcClient::FindChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.find( FoldName( channel ) );
    return i != m_channels.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::GetChannels( CORE::CString::StringVector& channels ) const
{GUCEF_TRACE;

    channels.reserve( channels.size() + m_channels.size() );
    TChannelMap::const_iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        channels.push_back( (*i).second.name );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::GetChannelMembers( const CORE::CString& channel, CORE::CString::StringVector& members ) const
{GUCEF_TRACE;

    const TChannel* found = FindChannel( channel );
    if ( GUCEF_NULL == found )
        return false;

    members.reserve( members.size() + found->members.size() );
    TMemberMap::const_iterator i = found->members.begin();
    while ( i != found->members.end() )
    {
        members.push_back( (*i).second.nick );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::GetChannelMemberPrefix( const CORE::CString& channel, const CORE::CString& nick, CORE::CString& prefix ) const
{GUCEF_TRACE;

    const TChannel* found = FindChannel( channel );
    if ( GUCEF_NULL == found )
        return false;
    TMemberMap::const_iterator i = found->members.find( FoldName( nick ) );
    if ( i == found->members.end() )
        return false;
    prefix = (*i).second.prefixes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::GetTopic( const CORE::CString& channel, CORE::CString& topic ) const
{GUCEF_TRACE;

    const TChannel* found = FindChannel( channel );
    if ( GUCEF_NULL == found )
        return false;
    topic = found->topic;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::AddMember( TChannel& channel, const CORE::CString& nickWithPrefixes, bool pending )
{GUCEF_TRACE;

    // With multi-prefix a member can have several prefixes, @+nick
    const char* str = nickWithPrefixes.C_String();
    UInt32 size = nickWithPrefixes.ByteSize() > 0 ? nickWithPrefixes.ByteSize() - 1 : 0;
    UInt32 prefixCount = 0;
    while ( prefixCount < size && IsMemberPrefixChar( str[ prefixCount ] ) )
        ++prefixCount;
    if ( prefixCount == size )
        return;

    TMember member;
    member.prefixes.Set( str, prefixCount );

    // A userhost-in-names nick!user@host keeps only the nick
    UInt32 nickEnd = prefixCount;
    while ( nickEnd < size && '!' != str[ nickEnd ] )
        ++nickEnd;
    member.nick.Set( str + prefixCount, nickEnd - prefixCount );

    TMemberMap& members = pending ? channel.pendingMembers : channel.members;
    members[ FoldName( member.nick ) ] = member;
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::RemoveMember( TChannel& channel, const CORE::CString& nick )
{GUCEF_TRACE;

    channel.members.erase( FoldName( nick ) );
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::FillEventData( const CIrcMessage& message, CIrcMessageEventData& eventData ) const
{GUCEF_TRACE;

    eventData.message = message;

    CORE::CString value;
    if ( message.GetTag( "time", value ) && eventData.serverTime.FromIso8601DateTimeString( value ) )
    {
        // as the server says
    }
    else
    {
        eventData.serverTime = CORE::CDateTime::NowUTCDateTime();
    }
    message.GetTag( "msgid", eventData.msgId );
    message.GetTag( "account", eventData.account );

    for ( size_t i=0; i<message.tags.size(); ++i )
        eventData.tags.Set( message.tags[ i ].key, message.tags[ i ].value );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::Notify( const CORE::CEvent& eventId           ,
                    const CIrcMessage& message            ,
                    const CORE::CString& sourceNick       ,
                    const CORE::CString& target           ,
                    const CORE::CString& text             )
{GUCEF_TRACE;

    CIrcMessageEventData eventData;
    FillEventData( message, eventData );
    eventData.sourceNick = sourceNick;
    eventData.target = target;
    eventData.text = text;
    return NotifyObservers( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::ReportError( const CORE::CString& text, const CIrcMessage* message )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "IrcClient: " << text );

    CIrcMessageEventData eventData;
    if ( GUCEF_NULL != message )
        FillEventData( *message, eventData );
    else
        eventData.serverTime = CORE::CDateTime::NowUTCDateTime();
    eventData.text = text;
    return NotifyObservers( ErrorEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnTransportConnected( void )
{GUCEF_TRACE;

    m_isConnected = true;
    m_lastReceivedInMs = GetTimeInMs();
    m_pingSent = false;

    if ( !NotifyObservers( ConnectedEvent ) )
        return false;

    // Capabilities first, so the server holds the registration until CAP END
    SendLine( "CAP LS 302" );

    CIrcMessage nick( "NICK" );
    nick.AddParam( m_nick );
    Send( nick );

    CIrcMessage user( "USER" );
    user.AddParam( m_settings.userName.IsNULLOrEmpty() ? m_nick : m_settings.userName );
    user.AddParam( "0" ).AddParam( "*" );
    user.AddParam( m_settings.realName.IsNULLOrEmpty() ? m_nick : m_settings.realName );
    Send( user );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnTransportDisconnected( void )
{GUCEF_TRACE;

    bool wasConnected = m_isConnected;
    Reset();
    m_timer.SetEnabled( false );
    if ( wasConnected )
        return NotifyObservers( DisconnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::EndCapabilityNegotiation( void )
{GUCEF_TRACE;

    if ( !m_capNegotiationDone )
    {
        m_capNegotiationDone = true;
        SendLine( "CAP END" );
    }
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::RequestCapabilities( const CORE::CString::StringSet& offered )
{GUCEF_TRACE;

    CORE::CString request;
    for ( size_t i=0; i<m_settings.wantedCapabilities.size(); ++i )
    {
        const CORE::CString& wanted = m_settings.wantedCapabilities[ i ];
        if ( offered.find( wanted ) == offered.end() || HasCapability( wanted ) )
            continue;
        if ( wanted == "sasl" && m_settings.saslAccount.IsNULLOrEmpty() )
            continue;
        if ( !request.IsNULLOrEmpty() )
            request += ' ';
        request += wanted;
    }

    if ( request.IsNULLOrEmpty() )
    {
        if ( !m_isRegistered )
            EndCapabilityNegotiation();
        return;
    }

    // Always as trailing, so a list of one capability is one parameter like a longer one
    SendLine( "CAP REQ :" + request );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnCap( const CIrcMessage& message )
{GUCEF_TRACE;

    // CAP <target> <subcommand> [*] :<list>
    const CORE::CString subCommand = message.GetParam( 1 ).Uppercase();
    bool moreToCome = message.GetParamCount() >= 4 && "*" == message.GetParam( 2 );
    const CORE::CString& list = message.GetParam( message.GetParamCount() - 1 );

    if ( "LS" == subCommand )
    {
        ParseCapabilityList( list, m_offeredCapabilities );
        if ( !moreToCome )
            RequestCapabilities( m_offeredCapabilities );
        return true;
    }
    if ( "ACK" == subCommand )
    {
        CORE::CString::StringSet acked;
        ParseCapabilityList( list, acked );
        bool saslAcked = false;
        CORE::CString::StringSet::iterator i = acked.begin();
        while ( i != acked.end() )
        {
            if ( (*i).StartsWith( "-" ) )
            {
                m_enabledCapabilities.erase( (*i).CutChars( 1, true, 0 ) );
            }
            else
            {
                m_enabledCapabilities.insert( (*i) );
                if ( "sasl" == (*i) )
                    saslAcked = true;
            }
            ++i;
        }

        if ( saslAcked && !m_settings.saslAccount.IsNULLOrEmpty() && !m_isRegistered )
        {
            m_saslInProgress = true;
            SendLine( "AUTHENTICATE PLAIN" );
            return true;
        }
        if ( !m_isRegistered && !m_saslInProgress )
            EndCapabilityNegotiation();
        return true;
    }
    if ( "NAK" == subCommand )
    {
        if ( !m_isRegistered && !m_saslInProgress )
            EndCapabilityNegotiation();
        return true;
    }
    if ( "NEW" == subCommand )
    {
        CORE::CString::StringSet added;
        ParseCapabilityList( list, added );
        m_offeredCapabilities.insert( added.begin(), added.end() );
        RequestCapabilities( added );
        return true;
    }
    if ( "DEL" == subCommand )
    {
        CORE::CString::StringSet removed;
        ParseCapabilityList( list, removed );
        CORE::CString::StringSet::iterator i = removed.begin();
        while ( i != removed.end() )
        {
            m_offeredCapabilities.erase( (*i) );
            m_enabledCapabilities.erase( (*i) );
            ++i;
        }
        return true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnAuthenticate( const CIrcMessage& message )
{GUCEF_TRACE;

    if ( !m_saslInProgress || "+" != message.GetParam( 0 ) )
        return true;

    // PLAIN: authorization id, authentication id and password, separated by NUL
    CORE::CDynamicBuffer plain( 256, true );
    const CORE::CString& account = m_settings.saslAccount;
    const CORE::CString& password = m_settings.saslPassword;
    UInt32 accountSize = account.ByteSize() > 0 ? account.ByteSize() - 1 : 0;
    UInt32 passwordSize = password.ByteSize() > 0 ? password.ByteSize() - 1 : 0;
    char nul = '\0';
    plain.Append( account.C_String(), accountSize, true );
    plain.Append( &nul, 1, true );
    plain.Append( account.C_String(), accountSize, true );
    plain.Append( &nul, 1, true );
    plain.Append( password.C_String(), passwordSize, true );

    CORE::CString encoded = CORE::Base64Encode( plain.GetConstBufferPtr(), plain.GetDataSize() );
    UInt32 encodedSize = encoded.ByteSize() > 0 ? encoded.ByteSize() - 1 : 0;

    // Sent in chunks of 400, a last chunk of exactly 400 is followed by an empty one
    UInt32 offset = 0;
    while ( offset < encodedSize )
    {
        UInt32 chunk = encodedSize - offset > SaslChunkSize ? SaslChunkSize : encodedSize - offset;
        SendLine( "AUTHENTICATE " + CORE::CString( encoded.C_String() + offset, chunk ) );
        offset += chunk;
    }
    if ( 0 == encodedSize || 0 == encodedSize % SaslChunkSize )
        SendLine( "AUTHENTICATE +" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::TryNextNick( void )
{GUCEF_TRACE;

    CORE::CString nextNick;
    if ( m_nickAttempt < m_settings.alternativeNicks.size() )
        nextNick = m_settings.alternativeNicks[ m_nickAttempt ];
    else
        nextNick = m_nick + "_";
    ++m_nickAttempt;

    m_nick = nextNick;
    CIrcMessage message( "NICK" );
    message.AddParam( nextNick );
    return Send( message );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnNumeric( const CIrcMessage& message, UInt32 numeric )
{GUCEF_TRACE;

    switch ( numeric )
    {
        case 1:     // RPL_WELCOME
        {
            if ( !m_isRegistered )
            {
                m_isRegistered = true;
                m_capNegotiationDone = true;
                m_saslInProgress = false;
                if ( !message.GetParam( 0 ).IsNULLOrEmpty() )
                    m_nick = message.GetParam( 0 );
                if ( !Notify( RegisteredEvent, message, message.GetSourceNick(), m_nick, message.GetParam( 1 ) ) )
                    return false;

                for ( size_t i=0; i<m_settings.autoJoinChannels.size(); ++i )
                    Join( m_settings.autoJoinChannels[ i ] );
            }
            return true;
        }
        case 332:   // RPL_TOPIC: <me> <channel> :<topic>
        {
            TChannel* channel = FindChannel( message.GetParam( 1 ) );
            if ( GUCEF_NULL != channel )
                channel->topic = message.GetParam( 2 );
            return Notify( TopicEvent, message, CORE::CString::Empty, message.GetParam( 1 ), message.GetParam( 2 ) );
        }
        case 353:   // RPL_NAMREPLY: <me> <symbol> <channel> :<names>
        {
            TChannel* channel = FindChannel( message.GetParam( 2 ) );
            if ( GUCEF_NULL != channel )
            {
                if ( !channel->namesInProgress )
                {
                    channel->namesInProgress = true;
                    channel->pendingMembers.clear();
                }
                CORE::CString::StringVector names = message.GetParam( 3 ).ParseElements( ' ', false );
                for ( size_t i=0; i<names.size(); ++i )
                    AddMember( *channel, names[ i ], true );
            }
            return true;
        }
        case 366:   // RPL_ENDOFNAMES: <me> <channel> :End of /NAMES list
        {
            TChannel* channel = FindChannel( message.GetParam( 1 ) );
            if ( GUCEF_NULL != channel )
            {
                if ( channel->namesInProgress )
                {
                    channel->members.swap( channel->pendingMembers );
                    channel->pendingMembers.clear();
                    channel->namesInProgress = false;
                }
                return Notify( NamesEvent, message, CORE::CString::Empty, channel->name, CORE::CString::Empty );
            }
            return true;
        }
        case 432:   // ERR_ERRONEUSNICKNAME
        case 433:   // ERR_NICKNAMEINUSE
        case 436:   // ERR_NICKCOLLISION
        {
            if ( !m_isRegistered )
                return TryNextNick();
            return ReportError( message.GetParam( message.GetParamCount() - 1 ), &message );
        }
        case 900:   // RPL_LOGGEDIN
        {
            return true;
        }
        case 903:   // RPL_SASLSUCCESS
        {
            m_saslInProgress = false;
            if ( !m_isRegistered )
                EndCapabilityNegotiation();
            return true;
        }
        case 902:   // ERR_NICKLOCKED
        case 904:   // ERR_SASLFAIL
        case 905:   // ERR_SASLTOOLONG
        case 906:   // ERR_SASLABORTED
        case 907:   // ERR_SASLALREADY
        {
            m_saslInProgress = false;
            if ( !m_isRegistered )
                EndCapabilityNegotiation();
            return ReportError( "SASL authentication failed: " + message.GetParam( message.GetParamCount() - 1 ), &message );
        }
        default:
        {
            // The other errors are passed on, the rest of the numerics only show as raw lines
            if ( numeric >= 400 && numeric < 600 )
                return ReportError( message.GetParam( message.GetParamCount() - 1 ), &message );
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnMessage( const CIrcMessage& message )
{GUCEF_TRACE;

    const CORE::CString& command = message.command;

    if ( message.IsNumeric() )
        return OnNumeric( message, CORE::StringToUInt32( command, 0 ) );

    if ( "PING" == command )
    {
        CIrcMessage pong( "PONG" );
        pong.params = message.params;
        Send( pong );
        return true;
    }
    if ( "PONG" == command )
    {
        m_pingSent = false;
        return true;
    }
    if ( "CAP" == command )
        return OnCap( message );
    if ( "AUTHENTICATE" == command )
        return OnAuthenticate( message );

    CORE::CString sourceNick = message.GetSourceNick();

    if ( "PRIVMSG" == command )
        return Notify( MessageEvent, message, sourceNick, message.GetParam( 0 ), message.GetParam( 1 ) );
    if ( "NOTICE" == command )
        return Notify( NoticeEvent, message, sourceNick, message.GetParam( 0 ), message.GetParam( 1 ) );

    if ( "JOIN" == command )
    {
        const CORE::CString& channelName = message.GetParam( 0 );
        if ( IsOwnNick( sourceNick ) )
        {
            TChannel& channel = m_channels[ FoldName( channelName ) ];
            channel.name = channelName;
            channel.namesInProgress = false;
        }
        TChannel* channel = FindChannel( channelName );
        if ( GUCEF_NULL != channel )
            AddMember( *channel, sourceNick, false );
        return Notify( JoinEvent, message, sourceNick, channelName, CORE::CString::Empty );
    }
    if ( "PART" == command )
    {
        const CORE::CString& channelName = message.GetParam( 0 );
        TChannel* channel = FindChannel( channelName );
        if ( GUCEF_NULL != channel )
        {
            if ( IsOwnNick( sourceNick ) )
                m_channels.erase( FoldName( channelName ) );
            else
                RemoveMember( *channel, sourceNick );
        }
        return Notify( PartEvent, message, sourceNick, channelName, message.GetParam( 1 ) );
    }
    if ( "KICK" == command )
    {
        const CORE::CString& channelName = message.GetParam( 0 );
        const CORE::CString& kicked = message.GetParam( 1 );
        TChannel* channel = FindChannel( channelName );
        if ( GUCEF_NULL != channel )
        {
            if ( IsOwnNick( kicked ) )
                m_channels.erase( FoldName( channelName ) );
            else
                RemoveMember( *channel, kicked );
        }
        return Notify( PartEvent, message, kicked, channelName, message.GetParam( 2 ) );
    }
    if ( "QUIT" == command )
    {
        TChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            RemoveMember( (*i).second, sourceNick );
            ++i;
        }
        return Notify( QuitEvent, message, sourceNick, CORE::CString::Empty, message.GetParam( 0 ) );
    }
    if ( "NICK" == command )
    {
        const CORE::CString& newNick = message.GetParam( 0 );
        CORE::CString oldFolded = FoldName( sourceNick );
        TChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            TMemberMap& members = (*i).second.members;
            TMemberMap::iterator m = members.find( oldFolded );
            if ( m != members.end() )
            {
                TMember member = (*m).second;
                member.nick = newNick;
                members.erase( m );
                members[ FoldName( newNick ) ] = member;
            }
            ++i;
        }
        if ( IsOwnNick( sourceNick ) )
        {
            m_nick = newNick;
            m_settings.nick = newNick;
        }
        return Notify( NickChangedEvent, message, sourceNick, CORE::CString::Empty, newNick );
    }
    if ( "TOPIC" == command )
    {
        TChannel* channel = FindChannel( message.GetParam( 0 ) );
        if ( GUCEF_NULL != channel )
            channel->topic = message.GetParam( 1 );
        return Notify( TopicEvent, message, sourceNick, message.GetParam( 0 ), message.GetParam( 1 ) );
    }
    if ( "ERROR" == command )
    {
        // The server is about to close the connection
        return ReportError( message.GetParam( 0 ), &message );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClient::OnLine( const CORE::CString& line )
{GUCEF_TRACE;

    m_lastReceivedInMs = GetTimeInMs();
    m_pingSent = false;

    CIrcMessage message;
    bool parsed = message.Parse( line );

    CIrcMessageEventData rawData;
    rawData.message = message;
    rawData.text = line;
    rawData.serverTime = CORE::CDateTime::NowUTCDateTime();
    if ( !NotifyObservers( RawLineEvent, &rawData ) )
        return false;

    if ( !parsed )
        return true;
    return OnMessage( message );
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::OnTimer( void )
{GUCEF_TRACE;

    if ( !m_isConnected || 0 == m_settings.pingTimeoutInSecs )
        return;

    UInt64 now = GetTimeInMs();
    UInt64 timeoutInMs = m_settings.pingTimeoutInSecs * (UInt64) 1000;
    if ( !m_pingSent )
    {
        if ( now - m_lastReceivedInMs >= timeoutInMs )
        {
            m_pingSent = true;
            m_pingSentInMs = now;
            CIrcMessage ping( "PING" );
            ping.AddParam( "gucef-" + CORE::UInt64ToString( now ) );
            Send( ping );
        }
    }
    else
    if ( now - m_pingSentInMs >= timeoutInMs )
    {
        if ( !ReportError( "The server did not answer within " + CORE::UInt32ToString( m_settings.pingTimeoutInSecs ) + " seconds", GUCEF_NULL ) )
            return;
        if ( !m_transport.IsNULL() )
            m_transport->Disconnect();
    }
}

/*-------------------------------------------------------------------------*/

void
CIrcClient::OnNotify( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_timer )
    {
        if ( CORE::CTimer::TimerUpdateEvent == eventId )
            OnTimer();
        return;
    }

    if ( m_transport.IsNULL() || notifier != m_transport.GetPointerAlways() )
        return;

    if ( CIIrcTransport::LineReceivedEvent == eventId && GUCEF_NULL != eventData )
    {
        OnLine( static_cast< CIIrcTransport::TLineReceivedEventData* >( eventData )->GetData() );
    }
    else
    if ( CIIrcTransport::ConnectedEvent == eventId )
    {
        OnTransportConnected();
    }
    else
    if ( CIIrcTransport::DisconnectedEvent == eventId )
    {
        OnTransportDisconnected();
    }
    else
    if ( CIIrcTransport::TransportErrorEvent == eventId )
    {
        CORE::CString description = GUCEF_NULL != eventData ? static_cast< CIIrcTransport::TTransportErrorEventData* >( eventData )->GetData() : CORE::CString( "Transport error" );
        ReportError( description, GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIrcClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
