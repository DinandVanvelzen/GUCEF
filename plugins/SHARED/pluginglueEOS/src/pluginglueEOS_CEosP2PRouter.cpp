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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pluginglueEOS_CEosP2PRouter.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosP2PRouter::CEosP2PRouter( const SEosP2PShim& shim )
    : m_shim( shim )
    , m_localUser()
    , m_relayControl( EOSRELAY_ALLOW_RELAYS )
    , m_sockets()
    , m_packet()
    , m_event()
    , m_dropped( 0 )
{GUCEF_TRACE;

    CEosP2PSocket::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEosP2PRouter::~CEosP2PRouter()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sockets.size(); ++i )
    {
        m_sockets[ i ]->CloseAllConnections();
        GUCEF_DELETE m_sockets[ i ];
    }
    m_sockets.clear();
    DeleteRetired();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PRouter::DeleteRetired( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_retired.size(); ++i )
        GUCEF_DELETE m_retired[ i ];
    m_retired.clear();
}

/*-------------------------------------------------------------------------*/

const SEosP2PShim&
CEosP2PRouter::GetShim( void ) const
{GUCEF_TRACE;

    return m_shim;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PRouter::SetLocalUser( const CORE::CString& localUser )
{GUCEF_TRACE;

    if ( localUser == m_localUser )
        return;

    // The connections belong to the user that opened them
    for ( size_t i=0; i<m_sockets.size(); ++i )
        m_sockets[ i ]->CloseAllConnections();
    m_localUser = localUser;
    if ( GUCEF_NULL != m_shim.setLocalUser )
        m_shim.setLocalUser( m_shim.context, m_localUser.C_String() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PRouter::GetLocalUser( void ) const
{GUCEF_TRACE;

    return m_localUser;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PRouter::SetRelayControl( EEosRelayControl control )
{GUCEF_TRACE;

    m_relayControl = control;
    if ( GUCEF_NULL == m_shim.setRelayControl )
        return false;
    return EOSRESULT_SUCCESS == m_shim.setRelayControl( m_shim.context, control );
}

/*-------------------------------------------------------------------------*/

EEosRelayControl
CEosP2PRouter::GetRelayControl( void ) const
{GUCEF_TRACE;

    return m_relayControl;
}

/*-------------------------------------------------------------------------*/

EEosNatType
CEosP2PRouter::GetNatType( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_shim.getNatType ? EOSNAT_UNKNOWN : m_shim.getNatType( m_shim.context );
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket*
CEosP2PRouter::AcquireSocket( const CORE::CString& name )
{GUCEF_TRACE;

    if ( !CEosP2PSocket::IsValidSocketName( name ) )
        return GUCEF_NULL;

    CEosP2PSocket* existing = FindSocket( name );
    if ( GUCEF_NULL != existing )
    {
        ++existing->m_refCount;
        return existing;
    }

    CEosP2PSocket* socket = GUCEF_NEW CEosP2PSocket( *this, name );
    m_sockets.push_back( socket );
    return socket;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PRouter::ReleaseSocket( CEosP2PSocket* socket )
{GUCEF_TRACE;

    if ( GUCEF_NULL == socket )
        return;
    for ( size_t i=0; i<m_sockets.size(); ++i )
    {
        if ( m_sockets[ i ] != socket )
            continue;
        if ( socket->m_refCount > 1 )
        {
            --socket->m_refCount;
            return;
        }
        socket->CloseAllConnections();
        m_sockets.erase( m_sockets.begin() + i );
        // Released from within one of its own events perhaps: it goes when the pump is done
        m_retired.push_back( socket );
        return;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PRouter::GetSocketCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_sockets.size();
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket*
CEosP2PRouter::FindSocket( const CORE::CString& name ) const
{GUCEF_TRACE;

    return FindSocketByText( name.C_String() );
}

/*-------------------------------------------------------------------------*/

CEosP2PSocket*
CEosP2PRouter::FindSocketByText( const char* name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_sockets.size(); ++i )
        if ( 0 == strcmp( m_sockets[ i ]->m_name.C_String(), name ) )
            return m_sockets[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

EEosResult
CEosP2PRouter::Send( const CORE::CString& remoteUser, const CORE::CString& socketName, UInt8 channel, const void* data, UInt32 size, EEosPacketReliability reliability )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shim.sendPacket || m_localUser.IsNULLOrEmpty() )
        return EOSRESULT_NOT_LOGGED_IN;
    return m_shim.sendPacket( m_shim.context, m_localUser.C_String(), remoteUser.C_String(), socketName.C_String(), channel, data, size, reliability, true );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PRouter::Close( const CORE::CString& remoteUser, const CORE::CString& socketName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shim.closeConnection || m_localUser.IsNULLOrEmpty() )
        return;
    m_shim.closeConnection( m_shim.context, m_localUser.C_String(), remoteUser.C_String(), socketName.C_String() );
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PRouter::Accept( const CORE::CString& remoteUser, const CORE::CString& socketName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shim.acceptConnection || m_localUser.IsNULLOrEmpty() )
        return false;
    return EOSRESULT_SUCCESS == m_shim.acceptConnection( m_shim.context, m_localUser.C_String(), remoteUser.C_String(), socketName.C_String() );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PRouter::Pump( void )
{GUCEF_TRACE;

    if ( !m_shim.IsUsable() || m_localUser.IsNULLOrEmpty() )
    {
        DeleteRetired();
        return;
    }

    // The events first: a peer is accepted before its packets are looked at
    UInt32 eventCount = 0;
    while ( eventCount < MaxPacketsPerPump && m_shim.pollEvent( m_shim.context, &m_event ) )
    {
        ++eventCount;
        if ( '\0' != m_event.localUser[ 0 ] && 0 != strcmp( m_event.localUser, m_localUser.C_String() ) )
            continue;
        CEosP2PSocket* socket = FindSocketByText( m_event.socketName );
        if ( GUCEF_NULL == socket )
        {
            // Nobody listens on that socket: a request is not accepted
            if ( EOSP2PEVENT_CONNECTION_REQUEST == m_event.kind && GUCEF_NULL != m_shim.closeConnection )
                m_shim.closeConnection( m_shim.context, m_localUser.C_String(), m_event.remoteUser, m_event.socketName );
            continue;
        }
        switch ( m_event.kind )
        {
            case EOSP2PEVENT_CONNECTION_REQUEST :       socket->OnConnectionRequest( m_event ); break;
            case EOSP2PEVENT_CONNECTION_ESTABLISHED :   socket->OnConnectionEstablished( m_event ); break;
            case EOSP2PEVENT_CONNECTION_INTERRUPTED :   socket->OnConnectionInterrupted( m_event ); break;
            case EOSP2PEVENT_CONNECTION_CLOSED :        socket->OnConnectionClosed( m_event ); break;
            default : break;
        }
    }

    char socketName[ EosP2PSocketBufferSize ];
    UInt32 received = 0;
    while ( received < MaxPacketsPerPump )
    {
        UInt32 size = 0;
        UInt8 channel = 0;
        socketName[ 0 ] = 0;
        m_packet.remoteUser[ 0 ] = 0;
        if ( !m_shim.receivePacket( m_shim.context, m_localUser.C_String(), (UInt32) sizeof( m_packet.data ), m_packet.data, &size, m_packet.remoteUser, socketName, &channel ) )
            break;
        ++received;
        m_packet.size = size;
        m_packet.channel = channel;
        CEosP2PSocket* socket = FindSocketByText( socketName );
        if ( GUCEF_NULL == socket || size > sizeof( m_packet.data ) )
        {
            ++m_dropped;
            continue;
        }
        socket->OnPacket( m_packet );
    }
    DeleteRetired();
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PRouter::GetDroppedPacketCount( void ) const
{GUCEF_TRACE;

    return m_dropped;
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
