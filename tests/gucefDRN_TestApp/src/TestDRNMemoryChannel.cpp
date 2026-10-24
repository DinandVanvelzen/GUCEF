/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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
#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#include "TestDRNMemoryChannel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

static const CORE::CString MemoryChannelType = "mem";

static MT::CMutex g_memoryChannelLock;
static GUCEF::map< UInt16, CDRNMemoryChannelListener* > g_memoryListeners;
static UInt16 g_nextAcceptedPort = 50000;

static CORE::CTFactory< DRN::CIDRNChannel, CDRNMemoryChannel, MT::CMutex > g_memoryChannelFactory;
static CORE::CTFactory< DRN::CIDRNChannelListener, CDRNMemoryChannelListener, MT::CMutex > g_memoryChannelListenerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static UInt16
PortOf( const DRN::CDRNEndpoint& endpoint )
{
    CORE::CString address = endpoint.GetAddress();
    const char* colon = strrchr( address.C_String(), ':' );
    return GUCEF_NULL == colon ? 0 : (UInt16) atoi( colon + 1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNMemoryChannel::CDRNMemoryChannel( void )
    : DRN::CIDRNChannel()
    , m_capabilities()
    , m_pulseGenerator()
    , m_remote()
    , m_peer( GUCEF_NULL )
    , m_connected( false )
    , m_announceConnected( false )
    , m_announceDisconnected( false )
    , m_queueLock()
    , m_queue()
    , m_queueHead( 0 )
    , m_queueCount( 0 )
    , m_delivering()
    , m_rxView()
    , m_rxEventData( &m_rxView )
{
    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = DRN::DRN_DEFAULT_DATAGRAM_SIZE;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
    m_delivering.SetBufferSize( m_capabilities.maxPacketSize, false );
}

/*-------------------------------------------------------------------------*/

CDRNMemoryChannel::~CDRNMemoryChannel()
{
    UnsubscribeAllFromObserver();
    Unlink();
    for ( UInt32 i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::Unlink( void )
{
    MT::CScopeMutex lock( g_memoryChannelLock );
    if ( GUCEF_NULL != m_peer )
    {
        m_peer->m_peer = GUCEF_NULL;
        m_peer->m_announceDisconnected = true;
        m_peer = GUCEF_NULL;
    }
    m_connected = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNMemoryChannel::Connect( const DRN::CDRNEndpoint& remote )
{
    CDRNMemoryChannel* peer = GUCEF_NULL;
    {
        MT::CScopeMutex lock( g_memoryChannelLock );
        GUCEF::map< UInt16, CDRNMemoryChannelListener* >::iterator i = g_memoryListeners.find( PortOf( remote ) );
        if ( i != g_memoryListeners.end() )
            peer = (*i).second->AcceptFrom( this );
        if ( GUCEF_NULL == peer )
            return false;
        m_peer = peer;
        m_remote = remote;
        m_connected = true;
        m_announceConnected = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::Accept( CDRNMemoryChannel* peer, const DRN::CDRNEndpoint& remote )
{
    m_peer = peer;
    m_remote = remote;
    m_connected = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::Close( void )
{
    Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CDRNMemoryChannel::IsConnected( void ) const
{
    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CDRNMemoryChannel::SendPacket( const void* data, UInt32 size )
{
    MT::CScopeMutex lock( g_memoryChannelLock );
    if ( !m_connected || GUCEF_NULL == m_peer || size > m_capabilities.maxPacketSize )
        return false;
    m_peer->Enqueue( data, size );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::Enqueue( const void* data, UInt32 size )
{
    MT::CScopeMutex lock( m_queueLock );
    if ( m_queueCount >= m_queue.size() )
    {
        UInt32 oldSize = (UInt32) m_queue.size();
        UInt32 newSize = oldSize < 64 ? 64 : oldSize * 2;
        TBufferVector grown( newSize, GUCEF_NULL );
        for ( UInt32 i=0; i<oldSize; ++i )
            grown[ i ] = m_queue[ ( m_queueHead + i ) % oldSize ];
        m_queue.swap( grown );
        m_queueHead = 0;
    }
    CORE::CDynamicBuffer*& slot = m_queue[ ( m_queueHead + m_queueCount ) % m_queue.size() ];
    if ( GUCEF_NULL == slot )
    {
        // as large as a packet can be, so that a slot never grows once it is there
        slot = GUCEF_NEW CORE::CDynamicBuffer();
        slot->SetBufferSize( m_capabilities.maxPacketSize, false );
    }
    slot->SetDataSize( 0 );
    slot->CopyFrom( size, data );
    ++m_queueCount;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::Deliver( void )
{
    while ( m_connected )
    {
        {
            MT::CScopeMutex lock( m_queueLock );
            if ( 0 == m_queueCount )
                return;
            const CORE::CDynamicBuffer& queued = *m_queue[ m_queueHead ];
            m_delivering.SetDataSize( 0 );
            m_delivering.CopyFrom( queued.GetDataSize(), queued.GetConstBufferPtr() );
            m_queueHead = ( m_queueHead + 1 ) % (UInt32) m_queue.size();
            --m_queueCount;
        }
        m_rxView.LinkTo( m_delivering.GetConstBufferPtr(), m_delivering.GetDataSize() );
        if ( !NotifyObservers( PacketReceivedEvent, &m_rxEventData ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{
    if ( CORE::CPulseGenerator::PulseEvent != eventid )
        return;

    if ( m_announceConnected )
    {
        m_announceConnected = false;
        if ( !NotifyObservers( ChannelConnectedEvent ) )
            return;
    }
    Deliver();
    if ( m_announceDisconnected )
    {
        m_announceDisconnected = false;
        m_connected = false;
        NotifyObservers( ChannelDisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

const DRN::CDRNChannelCapabilities&
CDRNMemoryChannel::GetCapabilities( void ) const
{
    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CDRNMemoryChannel::GetRemoteEndpoint( void ) const
{
    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNMemoryChannel::GetChannelType( void ) const
{
    return MemoryChannelType;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{
    if ( !m_pulseGenerator.IsNULL() )
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );
    m_pulseGenerator = pulseGenerator;
    if ( !m_pulseGenerator.IsNULL() )
        SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNMemoryChannel::GetPulseGenerator( void ) const
{
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CDRNMemoryChannelListener::CDRNMemoryChannelListener( void )
    : DRN::CIDRNChannelListener()
    , m_listenEndpoint()
    , m_pulseGenerator()
    , m_pendingAccepts()
    , m_listening( false )
{
}

/*-------------------------------------------------------------------------*/

CDRNMemoryChannelListener::~CDRNMemoryChannelListener()
{
    UnsubscribeAllFromObserver();
    StopListening();
}

/*-------------------------------------------------------------------------*/

bool
CDRNMemoryChannelListener::Listen( const DRN::CDRNEndpoint& local )
{
    MT::CScopeMutex lock( g_memoryChannelLock );
    UInt16 port = PortOf( local );
    if ( 0 == port || g_memoryListeners.find( port ) != g_memoryListeners.end() )
        return false;
    g_memoryListeners[ port ] = this;
    m_listenEndpoint = local;
    m_listening = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannelListener::StopListening( void )
{
    MT::CScopeMutex lock( g_memoryChannelLock );
    if ( !m_listening )
        return;
    g_memoryListeners.erase( PortOf( m_listenEndpoint ) );
    m_listening = false;
    m_pendingAccepts.clear();
}

/*-------------------------------------------------------------------------*/

bool
CDRNMemoryChannelListener::IsListening( void ) const
{
    return m_listening;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CDRNMemoryChannelListener::GetListenEndpoint( void ) const
{
    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNMemoryChannelListener::GetChannelType( void ) const
{
    return MemoryChannelType;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{
    if ( !m_pulseGenerator.IsNULL() )
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );
    m_pulseGenerator = pulseGenerator;
    if ( !m_pulseGenerator.IsNULL() )
        SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNMemoryChannelListener::GetPulseGenerator( void ) const
{
    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CDRNMemoryChannel*
CDRNMemoryChannelListener::AcceptFrom( CDRNMemoryChannel* connecting )
{
    // the caller holds the channel lock
    if ( !m_listening )
        return GUCEF_NULL;
    CDRNMemoryChannel* channel = GUCEF_NEW CDRNMemoryChannel();
    channel->SetPulseGenerator( m_pulseGenerator );
    channel->Accept( connecting, DRN::CDRNEndpoint( "mem://127.0.0.1:" + CORE::UInt32ToString( g_nextAcceptedPort++ ) ) );
    m_pendingAccepts.push_back( DRN::CIDRNChannelPtr( channel ) );
    return channel;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryChannelListener::OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{
    if ( CORE::CPulseGenerator::PulseEvent != eventid )
        return;

    GUCEF::vector< DRN::CIDRNChannelPtr > accepted;
    {
        MT::CScopeMutex lock( g_memoryChannelLock );
        if ( m_pendingAccepts.empty() )
            return;
        accepted.swap( m_pendingAccepts );
    }
    for ( UInt32 i=0; i<accepted.size(); ++i )
    {
        DRN::CDRNChannelEventData eventData( accepted[ i ] );
        if ( !NotifyObservers( ChannelAcceptedEvent, &eventData ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
RegisterMemoryChannelFactories( void )
{
    DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
    global->GetChannelFactory().RegisterConcreteFactory( MemoryChannelType, &g_memoryChannelFactory );
    global->GetChannelListenerFactory().RegisterConcreteFactory( MemoryChannelType, &g_memoryChannelListenerFactory );
}

/*-------------------------------------------------------------------------*/

void
UnregisterMemoryChannelFactories( void )
{
    DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
    global->GetChannelFactory().UnregisterConcreteFactory( MemoryChannelType );
    global->GetChannelListenerFactory().UnregisterConcreteFactory( MemoryChannelType );
}

/*-------------------------------------------------------------------------*/
