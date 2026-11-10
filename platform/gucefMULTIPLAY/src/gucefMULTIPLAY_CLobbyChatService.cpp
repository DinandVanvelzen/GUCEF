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

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#include "gucefMULTIPLAY_CGucefLobbyService.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#include "gucefMULTIPLAY_CGucefLobby.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#include "gucefMULTIPLAY_CLobbyChatService.h"

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

const CORE::CString CLobbyChatService::ServiceName = "lobby";
const CORE::CString CLobbyChatService::LobbyChannelPrefix = "lobby:";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLobbyChatService::CLobbyChatService( void )
    : CIChatService()
    , m_service()
    , m_lobby()
    , m_serviceConfig()
    , m_localUserName()
    , m_channels()
    , m_messageCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyChatService::~CLobbyChatService()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CGucefLobbyService*
CLobbyChatService::GetGucefService( void ) const
{GUCEF_TRACE;

    if ( m_service.IsNULL() )
        return GUCEF_NULL;
    // the class is final in practice: an exact match of the class name is the check, no RTTI needed
    CILobbyService* service = const_cast< CILobbyService* >( m_service.GetPointerAlways() );
    return service->GetClassTypeName() == CGucefLobbyService::ClassTypeName() ? static_cast< CGucefLobbyService* >( service ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::SetLobbyService( CILobbyServicePtr service )
{GUCEF_TRACE;

    if ( !m_service.IsNULL() )
        UnsubscribeFrom( m_service.GetPointerAlways() );
    m_service = service;
    if ( GUCEF_NULL == GetGucefService() )
    {
        m_service = CILobbyServicePtr();
        return false;
    }
    SubscribeTo( m_service.GetPointerAlways() );
    if ( m_localUserName.IsNULLOrEmpty() )
        m_localUserName = GetGucefService()->GetIdentity().displayName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::AttachLobby( CILobbyPtr lobby )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
        UnsubscribeFrom( m_lobby.GetPointerAlways() );
    m_lobby = lobby;
    if ( m_lobby.IsNULL() || m_lobby->GetClassTypeName() != CGucefLobby::ClassTypeName() )
    {
        m_lobby = CILobbyPtr();
        return false;
    }
    SubscribeTo( m_lobby.GetPointerAlways() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::IsLobbyChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    if ( 0 != channel.HasSubstr( LobbyChannelPrefix, true ) )
        return false;
    const CGucefLobby* lobby = ( m_lobby.IsNULL() || m_lobby->GetClassTypeName() != CGucefLobby::ClassTypeName() ) ? GUCEF_NULL : static_cast< const CGucefLobby* >( m_lobby.GetPointerAlways() );
    return GUCEF_NULL != lobby && channel == LobbyChannelPrefix + lobby->GetLobbyId();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future )
{GUCEF_TRACE;

    m_localUserName = identity.displayName.IsNULLOrEmpty() ? identity.userId : identity.displayName;
    if ( m_service.IsNULL() )
    {
        CILobbyServicePtr own = CMultiplayGlobal::Instance()->GetLobbyServiceFactory().Create( CGucefLobbyService::ServiceName );
        if ( own.IsNULL() )
            return false;
        own->LoadConfig( m_serviceConfig );
        SetLobbyService( own );
    }
    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL == service )
        return false;
    if ( service->IsConnected() )
    {
        CORE::CFutureResultSource done;
        done.Complete( service->GetIdentity().Clone(), true );
        future = done.GetFuture();
        return true;
    }
    return service->Connect( identity, future );
}

/*-------------------------------------------------------------------------*/

void
CLobbyChatService::Disconnect( void )
{GUCEF_TRACE;

    m_channels.clear();
    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL != service )
        service->Disconnect();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::IsConnected( void ) const
{GUCEF_TRACE;

    CGucefLobbyService* service = GetGucefService();
    return GUCEF_NULL != service && service->IsConnected();
}

/*-------------------------------------------------------------------------*/

void
CLobbyChatService::UpdateLobbyMembers( void )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return;
    const CGucefLobby* lobby = static_cast< const CGucefLobby* >( m_lobby.GetPointerAlways() );
    TChannelMap::iterator i = m_channels.find( LobbyChannelPrefix + lobby->GetLobbyId() );
    if ( i == m_channels.end() )
        return;
    (*i).second.members.clear();
    const CLobbyState& state = lobby->GetState();
    for ( size_t m=0; m<state.members.size(); ++m )
        (*i).second.members.insert( state.members[ m ].name );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::JoinChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    if ( channel.IsNULLOrEmpty() )
        return false;
    if ( IsLobbyChannel( channel ) )
    {
        m_channels[ channel ] = CChatChannel( channel );
        UpdateLobbyMembers();
        return true;
    }
    if ( 0 == channel.HasSubstr( LobbyChannelPrefix, true ) )
        return false;

    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL == service || !service->IsConnected() )
        return false;
    if ( !service->SubscribeChannel( channel, true ) )
        return false;
    CORE::CVariant name( channel );
    if ( 0 == service->SendRequest( "chatjoin", &name ) )
        return false;
    m_channels[ channel ] = CChatChannel( channel );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::LeaveChannel( const CORE::CString& channel )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.find( channel );
    if ( i == m_channels.end() )
        return false;
    m_channels.erase( i );
    if ( IsLobbyChannel( channel ) )
        return true;

    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL == service )
        return false;
    CORE::CVariant name( channel );
    service->SendRequest( "chatleave", &name );
    return service->SubscribeChannel( channel, false );
}

/*-------------------------------------------------------------------------*/

void
CLobbyChatService::GetChannels( CORE::CString::StringVector& channels ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        channels.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CIChatChannel*
CLobbyChatService::GetChannel( const CORE::CString& channel ) const
{GUCEF_TRACE;

    TChannelMap::const_iterator i = m_channels.find( channel );
    return i == m_channels.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::SendToChannel( const CORE::CString& channel, const CORE::CString& text )
{GUCEF_TRACE;

    if ( m_channels.find( channel ) == m_channels.end() )
        return false;
    if ( IsLobbyChannel( channel ) )
        return static_cast< CGucefLobby* >( m_lobby.GetPointerAlways() )->SendChat( text );

    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL == service )
        return false;
    CORE::CVariant name( channel );
    CORE::CVariant message( text );
    return 0 != service->SendRequest( "say", &name, &message );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::SendDirect( const CORE::CString& user, const CORE::CString& text )
{GUCEF_TRACE;

    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL == service || user.IsNULLOrEmpty() )
        return false;
    CORE::CVariant to( user );
    CORE::CVariant message( text );
    if ( 0 == service->SendRequest( "dm", &to, &message ) )
        return false;

    // The server does not send it back to the sender
    CChatMessage own;
    own.isDirect = true;
    own.from = m_localUserName;
    own.to = user;
    own.text = text;
    own.time = CORE::CDateTime::NowUTCDateTime();
    own.id = "own-" + CORE::ToString( ++m_messageCount );
    NotifyObservers( ChatMessageEvent, &own );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyChatService::GetLocalUserName( void ) const
{GUCEF_TRACE;

    return m_localUserName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyChatService::GetServiceName( void ) const
{GUCEF_TRACE;

    return ServiceName;
}

/*-------------------------------------------------------------------------*/

void
CLobbyChatService::OnNotify( CORE::CNotifier* notifier                 ,
                             const CORE::CEvent& eventId               ,
                             CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( CGucefLobby::ChatEntryEvent == eventId )
    {
        const CChatMessage* message = static_cast< const CChatMessage* >( eventData );
        if ( m_channels.find( message->channel ) != m_channels.end() )
            NotifyObservers( ChatMessageEvent, eventData );
        return;
    }
    if ( CILobby::MemberJoinedEvent == eventId || CILobby::MemberLeftEvent == eventId )
    {
        UpdateLobbyMembers();
        return;
    }

    if ( CGucefLobbyService::ChannelEntryEvent == eventId )
    {
        const CGucefDirectoryReply* entry = static_cast< const CGucefDirectoryReply* >( eventData );
        const CORE::CString& channel = entry->reason;
        TChannelMap::iterator i = m_channels.find( channel );
        if ( i == m_channels.end() )
            return;
        CORE::CString kind = entry->args[ 0 ].AsString( CORE::CString::Empty, true );
        CORE::CString from = entry->args[ 1 ].AsString( CORE::CString::Empty, true );
        if ( "msg" == kind )
        {
            CChatMessage message;
            message.channel = channel;
            message.from = from;
            message.text = entry->args[ 2 ].AsString( CORE::CString::Empty, true );
            message.time.FromUnixEpochBasedTicksInMillisecs( entry->args[ 3 ].AsUInt64( 0, true ) );
            message.id = entry->args[ 4 ].AsString( CORE::CString::Empty, true );
            NotifyObservers( ChatMessageEvent, &message );
        }
        else
        {
            bool joined = "joined" == kind;
            if ( joined )
                (*i).second.members.insert( from );
            else
                (*i).second.members.erase( from );
            CChatPresenceEventData presence( channel, from, joined ? CChatPresenceEventData::PRESENCE_JOINED : CChatPresenceEventData::PRESENCE_LEFT, CORE::CString::Empty );
            NotifyObservers( ChatPresenceEvent, &presence );
        }
        return;
    }

    if ( CGucefLobbyService::DirectMessageEvent == eventId )
    {
        const CGucefDirectoryReply* entry = static_cast< const CGucefDirectoryReply* >( eventData );
        CChatMessage message;
        message.isDirect = true;
        message.from = entry->args[ 0 ].AsString( CORE::CString::Empty, true );
        message.to = m_localUserName;
        message.text = entry->args[ 1 ].AsString( CORE::CString::Empty, true );
        message.time.FromUnixEpochBasedTicksInMillisecs( entry->args[ 2 ].AsUInt64( 0, true ) );
        message.id = entry->args[ 3 ].AsString( CORE::CString::Empty, true );
        NotifyObservers( ChatMessageEvent, &message );
        return;
    }

    if ( CGucefLobbyService::ReplyEvent == eventId )
    {
        const CGucefDirectoryReply* reply = static_cast< const CGucefDirectoryReply* >( eventData );
        if ( !reply->ok )
        {
            TChatErrorEventData error( reply->reason );
            NotifyObservers( ChatErrorEvent, &error );
        }
        return;
    }

    if ( CILobbyService::ServiceDisconnectedEvent == eventId )
    {
        m_channels.clear();
        TChatErrorEventData error( "Disconnected from the lobby server" );
        NotifyObservers( ChatErrorEvent, &error );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL != service )
        return service->SaveConfig( config );
    config.Copy( m_serviceConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyChatService::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_serviceConfig = config;
    CGucefLobbyService* service = GetGucefService();
    if ( GUCEF_NULL != service )
        return service->LoadConfig( config );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyChatService::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbyChatService";
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
