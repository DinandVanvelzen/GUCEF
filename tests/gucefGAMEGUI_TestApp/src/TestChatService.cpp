/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "TestChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< CTestChatService* > TServiceVector;

/** The users of the in-memory "server" */
TServiceVector&
GetRegistry( void )
{
    static TServiceVector registry;
    return registry;
}

void
Unregister( CTestChatService* service )
{
    TServiceVector& registry = GetRegistry();
    for ( TServiceVector::iterator i = registry.begin(); i != registry.end(); ++i )
    {
        if ( (*i) == service )
        {
            registry.erase( i );
            return;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CTestChatService::ServiceName = "test-chat";

/*-------------------------------------------------------------------------*/

CTestChatService::CTestChatService( void )
    : MULTIPLAY::CIChatService()
    , m_userName()
    , m_connected( false )
    , m_channels()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestChatService::~CTestChatService()
{GUCEF_TRACE;

    Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::Connect( const MULTIPLAY::CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    if ( identity.displayName.IsNULLOrEmpty() )
    {
        source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "A chat user needs a name" );
        future = source.GetFuture();
        return false;
    }
    if ( m_connected )
        Disconnect();

    m_userName = identity.displayName;
    m_connected = true;
    GetRegistry().push_back( this );
    source.Complete( identity.Clone(), true );
    future = source.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestChatService::Disconnect( void )
{GUCEF_TRACE;

    if ( !m_connected )
        return;

    CORE::CString::StringVector channels;
    GetChannels( channels );
    for ( size_t i=0; i<channels.size(); ++i )
        LeaveChannel( channels[ i ] );
    Unregister( this );
    m_connected = false;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::IsInChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    return m_channels.find( channel ) != m_channels.end();
}

/*-------------------------------------------------------------------------*/

void
CTestChatService::NotePresence( const CORE::CString& channel, const CORE::CString& user, MULTIPLAY::CChatPresenceEventData::TPresence presence )
{GUCEF_TRACE;

    TChannelMap::iterator c = m_channels.find( channel );
    if ( c == m_channels.end() )
        return;
    if ( MULTIPLAY::CChatPresenceEventData::PRESENCE_JOINED == presence )
        (*c).second.members.insert( user );
    else
    if ( MULTIPLAY::CChatPresenceEventData::PRESENCE_LEFT == presence )
        (*c).second.members.erase( user );
    MULTIPLAY::CChatPresenceEventData data( channel, user, presence, CORE::CString::Empty );
    NotifyObservers( ChatPresenceEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::JoinChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( !m_connected || channel.IsNULLOrEmpty() )
        return false;
    if ( IsInChannel( channel ) )
        return true;

    // who is in it already, and the topic they set
    MULTIPLAY::CChatChannel mine( channel );
    TServiceVector& registry = GetRegistry();
    for ( size_t i=0; i<registry.size(); ++i )
    {
        TChannelMap::const_iterator c = registry[ i ]->m_channels.find( channel );
        if ( c != registry[ i ]->m_channels.end() )
        {
            mine.members.insert( registry[ i ]->m_userName );
            mine.topic = (*c).second.topic;
        }
    }
    m_channels[ channel ] = mine;

    // everyone in it hears the newcomer, the newcomer too
    TServiceVector others = registry;
    for ( size_t i=0; i<others.size(); ++i )
        if ( others[ i ] != this && others[ i ]->IsInChannel( channel ) )
            others[ i ]->NotePresence( channel, m_userName, MULTIPLAY::CChatPresenceEventData::PRESENCE_JOINED );
    NotePresence( channel, m_userName, MULTIPLAY::CChatPresenceEventData::PRESENCE_JOINED );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::LeaveChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( !m_connected || !IsInChannel( channel ) )
        return false;

    TServiceVector others = GetRegistry();
    NotePresence( channel, m_userName, MULTIPLAY::CChatPresenceEventData::PRESENCE_LEFT );
    m_channels.erase( channel );
    for ( size_t i=0; i<others.size(); ++i )
        if ( others[ i ] != this && others[ i ]->IsInChannel( channel ) )
            others[ i ]->NotePresence( channel, m_userName, MULTIPLAY::CChatPresenceEventData::PRESENCE_LEFT );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestChatService::GetChannels( CORE::CString::StringVector& channels ) const
{GUCEF_TRACE;

    for ( TChannelMap::const_iterator i = m_channels.begin(); i != m_channels.end(); ++i )
        channels.push_back( (*i).first );
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CIChatChannel*
CTestChatService::GetChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.find( channel );
    return i != m_channels.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CTestChatService::Deliver( const MULTIPLAY::CChatMessage& message )
{GUCEF_TRACE;

    NotifyObservers( ChatMessageEvent, const_cast< MULTIPLAY::CChatMessage* >( &message ) );
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::SendToChannel( const CORE::CString& channel, const CORE::CString& text )
{GUCEF_TRACE;

    if ( !m_connected || !IsInChannel( channel ) || text.IsNULLOrEmpty() )
        return false;

    MULTIPLAY::CChatMessage message;
    message.channel = channel;
    message.from = m_userName;
    message.text = text;
    message.time = CORE::CDateTime::NowUTCDateTime();

    TServiceVector recipients = GetRegistry();
    for ( size_t i=0; i<recipients.size(); ++i )
        if ( recipients[ i ]->IsInChannel( channel ) )
            recipients[ i ]->Deliver( message );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::SendDirect( const CORE::CString& user, const CORE::CString& text )
{GUCEF_TRACE;

    if ( !m_connected || user.IsNULLOrEmpty() || text.IsNULLOrEmpty() )
        return false;

    MULTIPLAY::CChatMessage message;
    message.isDirect = true;
    message.from = m_userName;
    message.to = user;
    message.text = text;
    message.time = CORE::CDateTime::NowUTCDateTime();

    bool found = false;
    TServiceVector recipients = GetRegistry();
    for ( size_t i=0; i<recipients.size(); ++i )
    {
        if ( recipients[ i ]->m_userName == user && recipients[ i ] != this )
        {
            recipients[ i ]->Deliver( message );
            found = true;
        }
    }
    if ( !found )
        return false;
    Deliver( message );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::SetTopic( const CORE::CString& channel, const CORE::CString& topic )
{GUCEF_TRACE;

    if ( !m_connected || !IsInChannel( channel ) )
        return false;

    TServiceVector recipients = GetRegistry();
    for ( size_t i=0; i<recipients.size(); ++i )
    {
        TChannelMap::iterator c = recipients[ i ]->m_channels.find( channel );
        if ( c == recipients[ i ]->m_channels.end() )
            continue;
        (*c).second.topic = topic;
        MULTIPLAY::CChatTopicEventData data( channel, topic );
        recipients[ i ]->NotifyObservers( ChatTopicEvent, &data );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestChatService::ReportError( const CORE::CString& description )
{GUCEF_TRACE;

    TChatErrorEventData data( description );
    NotifyObservers( ChatErrorEvent, &data );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestChatService::GetLocalUserName( void ) const
{GUCEF_TRACE;

    return m_userName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestChatService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestChatService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestChatService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "CTestChatService";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/
