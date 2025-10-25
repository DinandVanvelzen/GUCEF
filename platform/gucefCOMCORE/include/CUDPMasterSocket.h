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

#ifndef GUCEF_COMCORE_CUDPMASTERSOCKET_H
#define GUCEF_COMCORE_CUDPMASTERSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H 
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPCHANNEL_H
#include "CUDPChannel.h"
#define GUCEF_COMCORE_CUDPCHANNEL_H
#endif /* GUCEF_COMCORE_CUDPCHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The UDP master socket adds the concept of data channels per peer to
 *  the UDP connection scheme. Each peer packet from a new peer will spawn
 *  a new UDP channel to which you can then subscribe. You can also reserve 
 *  channels in advance if you wish.
 *
 *  The main advantage of having such a system is that you can differentiate
 *  with ease between peers using a single UDP socket. 
 *  Making a UDP socket per peer us usually not an option because most people 
 *  don't want to open up that many UDP ports in their firewalls/routers. 
 *  It is especially annoying if the number of peers is variable.
 *
 *  Note that this system does not alter the transmissions in any way and as
 *  such can communicate with any standard UDP socket.
 */
class GUCEF_COMCORE_EXPORT_CPP CUDPMasterSocket : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent UDPMasterSocketErrorEvent;
    static const CORE::CEvent UDPMasterSocketClosedEvent;
    static const CORE::CEvent UDPMasterSocketOpenedEvent;
    static const CORE::CEvent UDPChannelCreatedEvent;
    static const CORE::CEvent UDPChannelToBeDestroyedEvent;
    
    static void RegisterEvents( void );
    
    typedef CORE::CTCloneableObj< CUDPChannel* > UDPChannelCreatedEventData;
    typedef CORE::CTCloneableObj< CUDPChannel* > UDPChannelToBeDestroyedEventData;
    
    public:

    /**
     *      Creates a UDP socket object initialized to be either blocking
     *      or non-blocking. Choosing either type is a design decision.
     *
     *      @param blocking whether to construct the socket as a blocking socket
     */        
    CUDPMasterSocket( const CORE::PulseGeneratorPtr& pulseGenerator ,
                      bool blocking                                 );
    
    /**
     *      Creates a UDP socket object initialized to be either blocking
     *      or non-blocking. Choosing either type is a design decision.
     *
     *      @param blocking whether to construct the socket as a blocking socket
     */        
    CUDPMasterSocket( bool blocking );
    
    /**
     *      Cleans up the allocated data and closes the socket
     */
    virtual ~CUDPMasterSocket();
    
    /**
     *      Returns whether the socket is currently active
     *
     *      @return activity status of the socket
     */
    bool IsActive( void ) const;
    
    /**
     *      Attempts to open the UDP socket for use.
     *      The port used is left up to the operating system.
     *      This will typically be used for clients in a client-server
     *      architecture.
     */
    bool Open( void );
    
    /**
     *      Attempts to open the UDP socket for use on the given port.
     *      This will typically be used for a server in a client-server
     *      architecture or for an peer-to-peer architecture.
     *      Any local address can be used.
     *
     *      @param port port used for sending and/or receiving data
     */        
    bool Open( UInt16 port );
    
    /**
     *      Attempts to open the UDP socket for use on the given local
     *      address and port. Basically the same as Open( UInt16 port )
     *      but here you binding to a specific local address mandatory.
     *
     *      @param localaddr local address to be used for this socket
     *      @param port port used for sending and/or receiving data
     */  
    bool Open( const CORE::CString& localaddr ,
               UInt16 port                    );
               
    UInt16 GetPort( void ) const;                   
    
    /**
     *      Closes the socket connection.
     */
    void Close( bool force = false );
    
    /**
     *      returns whether this socket is a blocking socket
     *
     *      @return whether this socket is a blocking socket
     */                                 
    bool IsBlocking( void ) const;

    /**
     *      Attempts to send a UDP packet to the given destination.
     *      Note that the data you send must fit within the payload
     *      space of a UDP packet.
     *      Note that this method is inefficient for regular
     *      transmissions to the same destination, use the other version
     *      for such a purpose.
     *
     *      @param DNSName DNS name of IP string of the remote host.
     *      @param port remote port that the data is to be sent to.
     *      @param data pointer to the data you wish to send
     *      @param datasize size in bytes of the data block pointed to by data
     *      @return the actual number of bytes that where sent. -1 indicates an error.
     */
    Int32 SendPacketTo( const CORE::CString& DNSName ,
                        UInt16 port                  ,
                        const void* data             , 
                        UInt16 dataSize              );

    /**
     *      Attempts to send a UDP packet to the given destination.
     *      Note that the data you send must fit within the payload
     *      space of a UDP packet.
     *
     *      @param dest Information about the packet destination.
     *      @param data pointer to the data you wish to send
     *      @param datasize size in bytes of the data block pointed to by data
     *      @return the actual number of bytes that where sent. -1 indicates an error.
     */
    Int32 SendPacketTo( const CIPv4Address& dest ,
                        const void* data       , 
                        UInt16 datasize        );
    
    /**
     *  Attempts to obtain the channel for the given address.
     *
     *  @return the requested channel or NULL if no channel is found for the given address
     */
    CUDPChannel* GetChannelForAddress( const CIPv4Address& remoteAddr );
    
    /**
     *  Attempts to obtain the channel for the given address.
     *  If no channel is found for the given address a new channel will be created.
     *
     *  @return the requested channel for the given address
     */
    CUDPChannel* ReserveChannelForAddress( const CIPv4Address& remoteAddr );
    
    /**
     *  Attempts to locate and then destroy a channel for the given address
     *
     *  @return whether or not a channel for the given address was found and destroyed
     */
    bool DestroyChannelForAddress( const CIPv4Address& remoteAddr );
    
    /**
     *  Destroys all existing channels
     */
    void DestroyAllChannels( void );

    protected:
    
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );
    
    private:

    typedef GUCEF::map< CIPv4Address, CUDPChannel* > TChannelMap;
    
    CUDPSocket m_udpSocket;
    TChannelMap m_channelMap;
    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CUDPMASTERSOCKET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 06-03-2007 :
        - Initial version

-----------------------------------------------------------------------------*/