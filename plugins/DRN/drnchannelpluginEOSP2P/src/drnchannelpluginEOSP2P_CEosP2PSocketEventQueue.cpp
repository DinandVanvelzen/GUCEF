/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "drnchannelpluginEOSP2P_CEosP2PSocketEventQueue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosP2PSocketEventQueue::CQueuedEvent::CQueuedEvent( void )
    : type( EVENTTYPE_PACKET )
    , packet()
    , connection()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosP2PSocketEventQueue::CEosP2PSocketEventQueue( void )
    : m_lock()
    , m_waiting()
    , m_waitingCount( 0 )
    , m_taken()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosP2PSocketEventQueue::~CEosP2PSocketEventQueue()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_waiting.size(); ++i )
        GUCEF_DELETE m_waiting[ i ];
    m_waiting.clear();
    for ( size_t i=0; i<m_taken.size(); ++i )
        GUCEF_DELETE m_taken[ i ];
    m_taken.clear();
    m_waitingCount = 0;
}

/*-------------------------------------------------------------------------*/

CEosP2PSocketEventQueue::CQueuedEvent*
CEosP2PSocketEventQueue::NextEntry( void )
{GUCEF_TRACE;

    // lock held
    if ( m_waitingCount >= m_waiting.size() )
        m_waiting.push_back( GUCEF_NEW CQueuedEvent() );
    return m_waiting[ m_waitingCount++ ];
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocketEventQueue::AddPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    bool wasEmpty = 0 == m_waitingCount;
    CQueuedEvent* entry = NextEntry();
    entry->type = EVENTTYPE_PACKET;
    memcpy( entry->packet.remoteUser, packet.remoteUser, sizeof( entry->packet.remoteUser ) );
    entry->packet.channel = packet.channel;
    entry->packet.size = packet.size <= sizeof( entry->packet.data ) ? packet.size : (UInt32) sizeof( entry->packet.data );
    memcpy( entry->packet.data, packet.data, entry->packet.size );
    return wasEmpty;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PSocketEventQueue::AddConnectionEvent( TEventType type, const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    bool wasEmpty = 0 == m_waitingCount;
    CQueuedEvent* entry = NextEntry();
    entry->type = type;
    entry->connection = data;
    return wasEmpty;
}

/*-------------------------------------------------------------------------*/

const CEosP2PSocketEventQueue::TQueuedEventVector&
CEosP2PSocketEventQueue::TakeAll( UInt32& count )
{GUCEF_TRACE;

    // the entries handed out last time go back for reuse, the waiting ones are handed out
    MT::CScopeMutex lock( m_lock );
    m_taken.swap( m_waiting );
    count = m_waitingCount;
    m_waitingCount = 0;
    return m_taken;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PSocketEventQueue::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_waitingCount = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
