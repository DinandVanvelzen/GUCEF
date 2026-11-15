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

/*
 *      The PH UDP Socket packet protocol:
 *
 *      1) Each packet should start with a 'magic number' (GUUDPSOCKETID) of sorts 
 *         to identify the packet as a GU protocol packet. 2 bytes.
 *      2) next is the packet checksum. 4 bytes.
 *      3) after that the packet type identifier. 2 bytes.
 *      4) flag to indicate wheter to deliver always (1) or only if it is the newest packet of this type (0). 1 byte.
 *      5) packet counter value for the packet type. 4 bytes.
 *      6) payload segment size. 2 bytes
 *      7) packet data payload. x bytes. with a max of ( max < 2 byte int max)
 *
 *      total packet header size: 15 bytes      
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "CPHUDPSocket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUPACKETHDRSIZE    15
#define GUUDPSOCKETID      63475UL
#define PACKETCOUNTERMAX   UINT32MAX-100;

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

const CORE::CEvent CPHUDPSocket::PHUDPSocketErrorEvent = "GUCEF::COM::CPHUDPSocket::PHUDPSocketErrorEvent";
const CORE::CEvent CPHUDPSocket::PHUDPSocketClosedEvent = "GUCEF::COM::CPHUDPSocket::PHUDPSocketClosedEvent";
const CORE::CEvent CPHUDPSocket::PHUDPSocketOpenedEvent = "GUCEF::COM::CPHUDPSocket::PHUDPSocketOpenedEvent";
const CORE::CEvent CPHUDPSocket::PHUDPPacketRecievedEvent = "GUCEF::COM::CPHUDPSocket::PHUDPPacketRecievedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPHUDPSocket::CPHUDPSocket( UInt32 minpayloadsize  ,
                            UInt16 sendpackettypes ,
                            UInt16 rcvpackettypes  )
        : _sock( false )                                    ,
          _buffer( GUPACKETHDRSIZE + minpayloadsize, true ) ,
          _sendpackettypes( sendpackettypes )               ,
          _rcvpackettypes( rcvpackettypes )                 ,
          _sendpcounters( GUCEF_NEW UInt32[ sendpackettypes ] )   ,
          _rcvpcounters( GUCEF_NEW UInt32[ rcvpackettypes ] )
{GUCEF_TRACE;

        memset( _sendpcounters, 0, sendpackettypes*4 );
        memset( _rcvpcounters, 0, rcvpackettypes*4 );
        
        SubscribeTo( &_sock );
}

/*-------------------------------------------------------------------------*/

CPHUDPSocket::~CPHUDPSocket()
{GUCEF_TRACE;
        GUCEF_DELETE []_sendpcounters;
        GUCEF_DELETE []_rcvpcounters;
}

/*-------------------------------------------------------------------------*/

void
CPHUDPSocket::BufferPacketSendInfo( const void* data         ,
                                    const UInt16 datasize    ,
                                    const UInt16 packettype  ,
                                    const bool deliveralways )
{GUCEF_TRACE;
        _buffer.SetDataSize( GUPACKETHDRSIZE + datasize );
        char* buffer = static_cast<char*>( _buffer.GetBufferPtr() );
        
        ( *((UInt16*) buffer) ) = GUUDPSOCKETID;
        ( *((UInt16*)(buffer+6)) ) = packettype;
        *(buffer+8) = deliveralways;
        ( *((UInt32*) (buffer+9)) ) = _sendpcounters[ packettype ]++;
        ( *((UInt16*) (buffer+13)) ) = datasize;                        
        memcpy( buffer+15, data, datasize );
        
        UInt32 sum( 0UL );
        for ( UInt16 i=0; i<datasize; ++i )
        {
                sum += *((char*)data)+i;
        }
        ( *((UInt32*) (buffer+2)) ) = sum;        
}                                    

/*-------------------------------------------------------------------------*/                         

Int32
CPHUDPSocket::SendPacketTo( const CIPv4Address& dest ,
                            const void* data       , 
                            UInt16 datasize        ,
                            UInt16 packettype      ,
                            bool deliveralways     )
{GUCEF_TRACE;
        if ( packettype >= _sendpackettypes ) return -1;
        
        BufferPacketSendInfo( data          ,
                              datasize      ,
                              packettype    , 
                              deliveralways );
        
        return _sock.SendPacketTo( dest                       ,
                                   _buffer.GetBufferPtr()     ,
                                   GUPACKETHDRSIZE + datasize ); 
}

/*-------------------------------------------------------------------------*/

Int32 
CPHUDPSocket::SendPacketTo( const GUCEF::CORE::CString& dnsname ,
                            UInt16 port                         ,
                            const void* data                    , 
                            UInt16 datasize                     ,
                            UInt16 packettype                   ,
                            bool deliveralways                  )
{GUCEF_TRACE;
        if ( packettype >= _sendpackettypes ) return -1;        
        
        BufferPacketSendInfo( data          ,
                              datasize      ,
                              packettype    , 
                              deliveralways );
        
        return _sock.SendPacketTo( dnsname                    ,
                                   port                       ,
                                   _buffer.GetBufferPtr()     ,
                                   GUPACKETHDRSIZE + datasize );
}

/*-------------------------------------------------------------------------*/

void 
CPHUDPSocket::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventid               ,
                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;
        
    // We only accept events from our local socket
    if ( notifier == &_sock )
    {
        if ( eventid == COMCORE::CUDPSocket::UDPPacketsRecievedEvent )
        {                        
            // Obtain data and pass it along for processing
            COMCORE::CUDPSocket::UDPPacketsRecievedEventData* eventData = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventdata );
            assert( GUCEF_NULL != eventData );
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = eventData->GetData();

            for ( UInt32 p=0; p<data.packetsReceived; ++p )
            {
                OnPacketRecieved( data.packets[ p ].sourceAddress ,
                                  data.packets[ p ].dataBuffer    );
            }
            return;
        }
        else
        if ( eventid == COMCORE::CUDPSocket::UDPSocketClosedEvent )
        {
            NotifyObservers( PHUDPSocketClosedEvent );
        }
        else
        if ( eventid == COMCORE::CUDPSocket::UDPSocketErrorEvent )
        {
            NotifyObservers( PHUDPSocketErrorEvent );
        }            
        else
        if ( eventid == COMCORE::CUDPSocket::UDPSocketOpenedEvent )
        {
            NotifyObservers( PHUDPSocketOpenedEvent );
        }                                             
    }
}                                

/*-------------------------------------------------------------------------*/

void
CPHUDPSocket::OnPacketRecieved( const CIPv4Address& sourceAddress                     ,
                                const CORE::TConstLinkedCloneableBuffer& dataBuffer )
{GUCEF_TRACE;
        /*
         *  Begin various checks before parsing packet info
         *  First we check if the data is even large enough to hold our header info
         */                                      
        const char* buffer = static_cast< const char* >( dataBuffer.GetData().GetConstBufferPtr() );
        UInt32 rbytes = dataBuffer.GetData().GetDataSize();  
        if ( rbytes > GUPACKETHDRSIZE )             
        {
                /*
                 *  Perform a sanity check on the payload size
                 */
                UInt16 datasize( *( const UInt16*)(buffer+13) );
                if ( rbytes == GUPACKETHDRSIZE + datasize )
                {
                        /*
                         *  Perform our simply 'hash' check to see if the data
                         *  has not been corrupted
                         */
                        const char* usrdata( buffer+GUPACKETHDRSIZE );
                        UInt32 sum( 0UL );
                        for ( UInt16 i=0; i<datasize; ++i )
                        {
                                sum += *(usrdata+i);        
                        }
                        if ( sum == *(UInt32*)(buffer+2) )
                        {
                                /*
                                 *      Now we know that the packet is in all likelyhood
                                 *      a valid packet. We will check if the packet should be
                                 *      dropped if it's to old and pass to the handler if 
                                 *      possible
                                 */
                                UInt16 packettype( *( const UInt16*)(buffer+6) );                                
                                if ( packettype < _rcvpackettypes )
                                {
                                        /*
                                         *  Check the number of the packet
                                         *  If the packet is no longer the latest news from the remote host
                                         *  then we will use the 'always deliver' boolean field in the packet 
                                         *  to determine if the packet should be dropped or passed on anyways
                                         */
                                        UInt32 packetnumber( *( const UInt32*)(buffer+6) );
                                        if ( ( _rcvpcounters[ packettype ] < packetnumber )      ||
                                             ( *(buffer+8) != 0 ) /* alwaysdeliver data field */  )
                                        {
                                                if ( _rcvpcounters[ packettype ] < packetnumber )
                                                {
                                                    _rcvpcounters[ packettype ] = packetnumber;
                                                }    
                                                
                                                CORE::CDynamicBuffer userData;
                                                userData.LinkTo( usrdata, datasize );
                                                CORE::TLinkedCloneableBuffer userDataBuffer( &userData );
                                                
                                                SPHUDPPacketRecievedEventData data;
                                                data.sourceAddress = sourceAddress;
                                                data.packetType = packettype;
                                                data.packetNumber = packetnumber;
                                                data.dataBuffer = userDataBuffer;
                                                
                                                PHUDPPacketRecievedEventData dispatchData( data );
                                                
                                                NotifyObservers( PHUDPPacketRecievedEvent, &dispatchData );
                                                return;
                                        }
                                }
                        }                                                        
                }                                                
        }
       // DEBUGOUTPUT( "CPHUDPSocket: dropped packet" );
}                                       

/*-------------------------------------------------------------------------*/
       
bool 
CPHUDPSocket::Open( void )
{GUCEF_TRACE;
        return _sock.Open();
}       

/*-------------------------------------------------------------------------*/ 
        
bool 
CPHUDPSocket::Open( UInt16 port )
{GUCEF_TRACE;
        return _sock.Open( port );
}

/*-------------------------------------------------------------------------*/

bool 
CPHUDPSocket::Open( const GUCEF::CORE::CString& localaddr ,
                    UInt16 port                           )
{GUCEF_TRACE;
        return _sock.Open( localaddr ,
                           port      );
}

/*-------------------------------------------------------------------------*/  
        
void 
CPHUDPSocket::Close( bool force )
{GUCEF_TRACE;
        return _sock.Close( force );               
}

/*-------------------------------------------------------------------------*/

bool 
CPHUDPSocket::IsActive( void ) const
{GUCEF_TRACE;
        return _sock.IsActive();
}

/*-------------------------------------------------------------------------*/

UInt16 
CPHUDPSocket::GetPort( void ) const
{GUCEF_TRACE;
        return _sock.GetPort();
}

/*-------------------------------------------------------------------------*/

void
CPHUDPSocket::RegisterEvents( void )
{GUCEF_TRACE;

    PHUDPSocketErrorEvent.Initialize();
    PHUDPSocketClosedEvent.Initialize();
    PHUDPSocketOpenedEvent.Initialize();
    PHUDPPacketRecievedEvent.Initialize();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
