/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#include "pluginglueEOS_CEosP2PRouter.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H ? */

#include "pluginglueEOS_CEosP2PSocket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CEosP2PSocket::PacketReceivedEvent = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket::PacketReceivedEvent";
const CORE::CEvent CEosP2PSocket::ConnectionRequestEvent = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket::ConnectionRequestEvent";
const CORE::CEvent CEosP2PSocket::ConnectionEstablishedEvent = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket::ConnectionEstablishedEvent";
const CORE::CEvent CEosP2PSocket::ConnectionInterruptedEvent = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket::ConnectionInterruptedEvent";
const CORE::CEvent CEosP2PSocket::ConnectionClosedEvent = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket::ConnectionClosedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosP2PPacketEventData::CEosP2PPacketEventData( void )
    : CORE::CICloneable()
    , channel( 0 )
    , size( 0 )
{GUCEF_TRACE;

    remoteUser[ 0 ] = 0;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEosP2PPacketEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEosP2PPacketEventData( *this );
}

/*-------------------------------------------------------------------------*/

CEosP2PConnectionEventData::CEosP2PConnectionEventData( void )
    : CORE::CICloneable()
    , networkType( EOSNETWORK_NONE )
    , closeReason( EOSCLOSE_UNKNOWN )
    , accepted( false )
{GUCEF_TRACE;

    remoteUser[ 0 ] = 0;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEosP2PConnectionEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEosP2PConnectionEventData( *this );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::RegisterEvents( void )
{GUCEF_TRACE;

    PacketReceivedEvent.Initialize();
    ConnectionRequestEvent.Initialize();
    ConnectionEstablishedEvent.Initialize();
    ConnectionInterruptedEvent.Initialize();
    ConnectionClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocket::IsValidSocketName( const CORE::CString& name )
{GUCEF_TRACE;

    UInt32 size = name.ByteSize();
    if ( size < 2 || size - 1 > EosP2PMaxSocketNameLength )
        return false;
    const char* chars = name.C_String();
    for ( UInt32 i=0; i<size-1; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || '-' == c || '_' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket::CEosP2PSocket( CEosP2PRouter& router, const CORE::CString& name )
    : CORE::CObservingNotifier()
    , m_router( router )
    , m_name( name )
    , m_policy( ACCEPT_NONE )
    , m_allowed()
    , m_connections()
    , m_maxConnections( 64 )
    , m_refCount( 1 )
    , m_refused( 0 )
    , m_received( 0 )
    , m_sent( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket::~CEosP2PSocket()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PSocket::GetSocketName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::SetAcceptPolicy( TAcceptPolicy policy )
{GUCEF_TRACE;

    m_policy = policy;
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket::TAcceptPolicy
CEosP2PSocket::GetAcceptPolicy( void ) const
{GUCEF_TRACE;

    return m_policy;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::SetAllowedRemotes( const CORE::CString::StringSet& remotes )
{GUCEF_TRACE;

    m_allowed = remotes;
    if ( ACCEPT_LIST != m_policy )
        return;

    // Whoever is connected and no longer allowed goes
    TPeerVector connected = m_connections;
    for ( size_t i=0; i<connected.size(); ++i )
        if ( m_allowed.find( CORE::CString( connected[ i ].text ) ) == m_allowed.end() )
            CloseConnection( CORE::CString( connected[ i ].text ) );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::AllowRemote( const CORE::CString& remoteUser )
{GUCEF_TRACE;

    m_allowed.insert( remoteUser );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::DisallowRemote( const CORE::CString& remoteUser )
{GUCEF_TRACE;

    m_allowed.erase( remoteUser );
    if ( ACCEPT_LIST == m_policy && IndexOfConnection( remoteUser.C_String() ) >= 0 )
        CloseConnection( remoteUser );
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocket::IsRemoteAllowed( const CORE::CString& remoteUser ) const
{GUCEF_TRACE;

    switch ( m_policy )
    {
        case ACCEPT_ANY :   return true;
        case ACCEPT_LIST :  return m_allowed.find( remoteUser ) != m_allowed.end();
        default :           return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::SetMaxConnections( UInt32 maxConnections )
{GUCEF_TRACE;

    m_maxConnections = maxConnections;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PSocket::GetMaxConnections( void ) const
{GUCEF_TRACE;

    return m_maxConnections;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PSocket::GetConnectionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_connections.size();
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocket::IsConnectedTo( const CORE::CString& remoteUser ) const
{GUCEF_TRACE;

    return IndexOfConnection( remoteUser.C_String() ) >= 0;
}

/*-------------------------------------------------------------------------*/

Int32
CEosP2PSocket::IndexOfConnection( const char* remoteUser ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_connections.size(); ++i )
        if ( 0 == strcmp( m_connections[ i ].text, remoteUser ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::AddConnection( const char* remoteUser )
{GUCEF_TRACE;

    if ( IndexOfConnection( remoteUser ) >= 0 )
        return;
    SPeerId peer;
    strncpy( peer.text, remoteUser, sizeof( peer.text ) - 1 );
    peer.text[ sizeof( peer.text ) - 1 ] = 0;
    m_connections.push_back( peer );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::RemoveConnection( const char* remoteUser )
{GUCEF_TRACE;

    Int32 index = IndexOfConnection( remoteUser );
    if ( index >= 0 )
        m_connections.erase( m_connections.begin() + index );
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocket::SendPacket( const CORE::CString& remoteUser, const void* data, UInt32 size, EEosPacketReliability reliability, UInt8 channel )
{GUCEF_TRACE;

    if ( size > EosP2PMaxPacketSize || GUCEF_NULL == data )
        return false;

    // A packet to a peer opens the connection: that counts against the limit like one that was accepted
    if ( 0 != m_maxConnections && m_connections.size() >= m_maxConnections && IndexOfConnection( remoteUser.C_String() ) < 0 )
        return false;

    EEosResult result = m_router.Send( remoteUser, m_name, channel, data, size, reliability );
    if ( EOSRESULT_SUCCESS != result )
        return false;

    ++m_sent;
    AddConnection( remoteUser.C_String() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::CloseConnection( const CORE::CString& remoteUser )
{GUCEF_TRACE;

    bool known = IndexOfConnection( remoteUser.C_String() ) >= 0;
    m_router.Close( remoteUser, m_name );
    RemoveConnection( remoteUser.C_String() );
    if ( known )
    {
        CEosP2PConnectionEventData data;
        strncpy( data.remoteUser, remoteUser.C_String(), sizeof( data.remoteUser ) - 1 );
        data.remoteUser[ sizeof( data.remoteUser ) - 1 ] = 0;
        data.closeReason = EOSCLOSE_CLOSED_BY_LOCAL_USER;
        NotifyObservers( ConnectionClosedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::CloseAllConnections( void )
{GUCEF_TRACE;

    TPeerVector connected = m_connections;
    for ( size_t i=0; i<connected.size(); ++i )
        CloseConnection( CORE::CString( connected[ i ].text ) );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::OnConnectionRequest( const SEosP2PEvent& event )
{GUCEF_TRACE;

    CORE::CString remote( event.remoteUser );
    CEosP2PConnectionEventData data;
    strncpy( data.remoteUser, event.remoteUser, sizeof( data.remoteUser ) - 1 );
    data.remoteUser[ sizeof( data.remoteUser ) - 1 ] = 0;

    bool known = IndexOfConnection( event.remoteUser ) >= 0;
    bool accept = IsRemoteAllowed( remote );
    if ( accept && !known && 0 != m_maxConnections && m_connections.size() >= m_maxConnections )
        accept = false;

    if ( accept && m_router.Accept( remote, m_name ) )
    {
        AddConnection( event.remoteUser );
        data.accepted = true;
    }
    else
    {
        m_router.Close( remote, m_name );
        ++m_refused;
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosP2PSocket: A connection request on socket " << m_name << " is refused" );
    }
    NotifyObservers( ConnectionRequestEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::OnConnectionEstablished( const SEosP2PEvent& event )
{GUCEF_TRACE;

    CORE::CString remote( event.remoteUser );

    // A connection that is not wanted is not told to anyone
    if ( IndexOfConnection( event.remoteUser ) < 0 )
    {
        if ( !IsRemoteAllowed( remote ) )
        {
            m_router.Close( remote, m_name );
            return;
        }
        AddConnection( event.remoteUser );
    }

    CEosP2PConnectionEventData data;
    strncpy( data.remoteUser, event.remoteUser, sizeof( data.remoteUser ) - 1 );
    data.remoteUser[ sizeof( data.remoteUser ) - 1 ] = 0;
    data.networkType = event.networkType;
    data.accepted = true;
    NotifyObservers( ConnectionEstablishedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::OnConnectionInterrupted( const SEosP2PEvent& event )
{GUCEF_TRACE;

    if ( IndexOfConnection( event.remoteUser ) < 0 )
        return;
    CEosP2PConnectionEventData data;
    strncpy( data.remoteUser, event.remoteUser, sizeof( data.remoteUser ) - 1 );
    data.remoteUser[ sizeof( data.remoteUser ) - 1 ] = 0;
    NotifyObservers( ConnectionInterruptedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::OnConnectionClosed( const SEosP2PEvent& event )
{GUCEF_TRACE;

    if ( IndexOfConnection( event.remoteUser ) < 0 )
        return;
    RemoveConnection( event.remoteUser );
    CEosP2PConnectionEventData data;
    strncpy( data.remoteUser, event.remoteUser, sizeof( data.remoteUser ) - 1 );
    data.remoteUser[ sizeof( data.remoteUser ) - 1 ] = 0;
    data.closeReason = event.closeReason;
    NotifyObservers( ConnectionClosedEvent, &data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocket::OnPacket( CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    // The SDK only delivers packets of connections that were accepted; a peer that is not allowed is not listened to anyway
    if ( IndexOfConnection( packet.remoteUser ) < 0 )
    {
        if ( !IsRemoteAllowed( CORE::CString( packet.remoteUser ) ) || ( 0 != m_maxConnections && m_connections.size() >= m_maxConnections ) )
        {
            ++m_refused;
            return;
        }
        AddConnection( packet.remoteUser );
    }
    ++m_received;
    NotifyObservers( PacketReceivedEvent, &packet );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PSocket::GetRefusedConnectionCount( void ) const
{GUCEF_TRACE;

    return m_refused;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PSocket::GetReceivedPacketCount( void ) const
{GUCEF_TRACE;

    return m_received;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PSocket::GetSentPacketCount( void ) const
{GUCEF_TRACE;

    return m_sent;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PSocket::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::EOS::CEosP2PSocket";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
