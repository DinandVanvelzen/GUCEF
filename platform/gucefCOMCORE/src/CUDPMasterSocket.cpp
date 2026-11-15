/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include "CUDPMasterSocket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CUDPMasterSocket::UDPMasterSocketErrorEvent = "GUCEF::COMCORE::CUDPMasterSocket::UDPMasterSocketErrorEvent";
const CORE::CEvent CUDPMasterSocket::UDPMasterSocketClosedEvent = "GUCEF::COMCORE::CUDPMasterSocket::UDPMasterSocketClosedEvent";
const CORE::CEvent CUDPMasterSocket::UDPMasterSocketOpenedEvent = "GUCEF::COMCORE::CUDPMasterSocket::UDPMasterSocketOpenedEvent";
const CORE::CEvent CUDPMasterSocket::UDPChannelCreatedEvent = "GUCEF::COMCORE::CUDPMasterSocket::UDPChannelCreatedEvent";
const CORE::CEvent CUDPMasterSocket::UDPChannelToBeDestroyedEvent = "GUCEF::COMCORE::CUDPMasterSocket::UDPChannelToBeDestroyedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CUDPMasterSocket::RegisterEvents( void )
{GUCEF_TRACE;

    UDPMasterSocketErrorEvent.Initialize();
    UDPMasterSocketClosedEvent.Initialize();
    UDPMasterSocketOpenedEvent.Initialize();
    UDPChannelCreatedEvent.Initialize();
    UDPChannelToBeDestroyedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/
    
CUDPMasterSocket::CUDPMasterSocket( const CORE::PulseGeneratorPtr& pulseGenerator ,
                                    bool blocking                                 )
    : CObservingNotifier()        ,
      m_udpSocket( pulseGenerator , 
                   blocking       ) ,
      m_channelMap()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/
    
CUDPMasterSocket::CUDPMasterSocket( bool blocking )
    : CObservingNotifier()    ,
      m_udpSocket( blocking ) ,
      m_channelMap()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/
    
CUDPMasterSocket::~CUDPMasterSocket()
{GUCEF_TRACE;

    Close( true );
}

/*-------------------------------------------------------------------------*/
    
bool
CUDPMasterSocket::IsActive( void ) const
{GUCEF_TRACE;

    return m_udpSocket.IsActive();
}

/*-------------------------------------------------------------------------*/
    
bool
CUDPMasterSocket::Open( void )
{GUCEF_TRACE;

    return m_udpSocket.Open();
}

/*-------------------------------------------------------------------------*/
      
bool
CUDPMasterSocket::Open( UInt16 port )
{GUCEF_TRACE;

    return m_udpSocket.Open( port );
}

/*-------------------------------------------------------------------------*/
    
bool
CUDPMasterSocket::Open( const CORE::CString& localaddr ,
                        UInt16 port                    )
{GUCEF_TRACE;

    return m_udpSocket.Open( localaddr ,
                             port      );
}

/*-------------------------------------------------------------------------*/
               
UInt16
CUDPMasterSocket::GetPort( void ) const
{GUCEF_TRACE;

    return m_udpSocket.GetPort();
}

/*-------------------------------------------------------------------------*/
    
void
CUDPMasterSocket::Close( bool force /* = false */ )
{GUCEF_TRACE;

    return m_udpSocket.Close( force );
}

/*-------------------------------------------------------------------------*/
                           
bool
CUDPMasterSocket::IsBlocking( void ) const
{GUCEF_TRACE;

    return m_udpSocket.IsBlocking();
}

/*-------------------------------------------------------------------------*/    

CUDPChannel*
CUDPMasterSocket::GetChannelForAddress( const CIPv4Address& remoteAddr )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channelMap.find( remoteAddr );
    if ( i != m_channelMap.end() )
    {
        return (*i).second;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CUDPChannel*
CUDPMasterSocket::ReserveChannelForAddress( const CIPv4Address& remoteAddr )
{GUCEF_TRACE;

    // Check for an existing channel for the given address
    CUDPChannel* channel = GetChannelForAddress( remoteAddr );
    if ( GUCEF_NULL == channel )
    {
        // Reserve a new channel for the given address
        channel = GUCEF_NEW CUDPChannel( *this, remoteAddr );
        m_channelMap.insert( std::pair< CIPv4Address, CUDPChannel* >( remoteAddr, channel ) );
        
        UDPChannelCreatedEventData eData( channel );
        NotifyObservers( UDPChannelCreatedEvent, &eData );
    }
    return channel;
}

/*-------------------------------------------------------------------------*/

bool
CUDPMasterSocket::DestroyChannelForAddress( const CIPv4Address& remoteAddr )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channelMap.find( remoteAddr );
    if ( i != m_channelMap.end() )
    {
        UDPChannelToBeDestroyedEventData eData( (*i).second );
        NotifyObservers( UDPChannelToBeDestroyedEvent, &eData );
        
        GUCEF_DELETE ((*i).second);
        m_channelMap.erase( i );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CUDPMasterSocket::DestroyAllChannels( void )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channelMap.begin();
    while ( i != m_channelMap.end() )
    {
        UDPChannelToBeDestroyedEventData eData( (*i).second );
        NotifyObservers( UDPChannelToBeDestroyedEvent, &eData );
        
        GUCEF_DELETE (*i).second;
        (*i).second = GUCEF_NULL;
        ++i;
    }
    m_channelMap.clear();
}

/*-------------------------------------------------------------------------*/

void
CUDPMasterSocket::OnNotify( CORE::CNotifier* notifier                 ,
                            const CORE::CEvent& eventid               ,
                            CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // Mandatory: call the base class implementation
    CObservingNotifier::OnNotify( notifier  ,
                                  eventid   ,
                                  eventdata );
    
    // We only accept messages from our own UDP socket
    if ( notifier == &m_udpSocket )
    {
        if ( eventid == CUDPSocket::UDPPacketsRecievedEvent )
        {
            // convert eventdata into something usable
            CUDPSocket::UDPPacketsRecievedEventData* eData = static_cast< CUDPSocket::UDPPacketsRecievedEventData* >( eventdata );
            const CUDPSocket::TUdpPacketsRecievedEventData& data = eData->GetData();
            
            for ( UInt32 p=0; p<data.packetsReceived; ++p )
            {
                // Obtain a channel for the given peer address
                CUDPChannel* channel = ReserveChannelForAddress( data.packets[ p ].sourceAddress );
            
                // Force the channel to notify to its clients
                // Because the data buffer link will only live inside this scope it is safe to
                // link to the already linked buffer
                CORE::TConstLinkedCloneableBuffer buffer( &data.packets[ p ].dataBuffer.GetData() );
                channel->DoNotifyObservers( CUDPChannel::UDPPacketReceivedEvent ,
                                            &buffer                             );
            }
        }
        else
        if ( eventid == CUDPSocket::UDPSocketErrorEvent )
        {
            NotifyObservers( UDPMasterSocketErrorEvent, eventdata );
        }
        else
        if ( eventid == CUDPSocket::UDPSocketClosedEvent )
        {
            NotifyObservers( UDPMasterSocketClosedEvent, eventdata );
        }
        else
        if ( eventid == CUDPSocket::UDPSocketOpenedEvent )
        {
            NotifyObservers( UDPMasterSocketOpenedEvent, eventdata );
        }                
    }
}

/*-------------------------------------------------------------------------*/

Int32
CUDPMasterSocket::SendPacketTo( const CORE::CString& DNSName ,
                                UInt16 port                  ,
                                const void* data             , 
                                UInt16 dataSize              )
{GUCEF_TRACE;

    return m_udpSocket.SendPacketTo( DNSName  ,
                                     port     ,
                                     data     ,
                                     dataSize );    
}

/*-------------------------------------------------------------------------*/

Int32
CUDPMasterSocket::SendPacketTo( const CIPv4Address& dest ,
                                const void* data       , 
                                UInt16 datasize        )
{GUCEF_TRACE;

    return m_udpSocket.SendPacketTo( dest     ,
                                     data     ,
                                     datasize );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/