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

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#define GUCEF_COMCORE_CUDPSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"       /* gucefMT mutex class */
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"            /* gucefCORE platform string implementation */
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"      /* base class for all TCP and UDP related socket classes */
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

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
 *      UDP Socket class that wraps the O/S UDP socket functionality.
 *      This class can act as a blocking or non-blocking UDP socket.
 *      In both cases a UDPPacketRecievedEvent will be sent when a packet
 *      is received.
 */
class GUCEF_COMCORE_EXPORT_CPP CUDPSocket : public CSocket
{
    public:

    static const CORE::CEvent UDPSocketErrorEvent;
    static const CORE::CEvent UDPSocketClosingEvent;
    static const CORE::CEvent UDPSocketClosedEvent;
    static const CORE::CEvent UDPSocketOpenedEvent;
    static const CORE::CEvent UDPPacketsRecievedEvent;

    static void RegisterEvents( void );

    struct SPacketEntry
    {
        CIPv4Address sourceAddress;                      /**< the source address of the data */
        CORE::TConstLinkedCloneableBuffer dataBuffer;  /**< the received packet data */
    };
    typedef struct SPacketEntry TPacketEntry;
    typedef GUCEF::vector< TPacketEntry > TPacketEntryVector;

    struct SUdpPacketsRecievedEventData
    {
        TPacketEntryVector packets;               /**< Collection of all packets read from the socket in the latest batch, up to 'packetsReceived' */
        UInt32 packetsReceived;                   /**< The number of packets that have been received and can be obtained from 'packets' */
    };
    typedef struct SUdpPacketsRecievedEventData TUdpPacketsRecievedEventData;

    typedef CORE::CTCloneableObj< TUdpPacketsRecievedEventData > UDPPacketsRecievedEventData;
    typedef CORE::TCloneableInt32  TSocketErrorEventData;

    public:

    /**
     *      Creates a UDP socket object initialized to be either blocking
     *      or non-blocking. Choosing either type is a design decision.
     *
     *      @param blocking whether to construct the socket as a blocking socket
     */
    CUDPSocket( const CORE::PulseGeneratorPtr& pulseGenerator ,
                bool blocking                                 );

    /**
     *      Creates a UDP socket object initialized to be either blocking
     *      or non-blocking. Choosing either type is a design decision.
     *
     *      @param blocking whether to construct the socket as a blocking socket
     */
    CUDPSocket( bool blocking );

    /**
     *      Cleans up the allocated data and closes the socket
     */
    virtual ~CUDPSocket();

    /**
     *      Returns wheter the socket is currently active
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

    /**
     *      Attempts to open the UDP socket for use on the given local
     *      address and port. Basically the same as Open( UInt16 port )
     *      but here you binding to a specific local address mandatory.
     *
     *      @param localaddr local address and port to be used for this socket
     */
    bool Open( const CIPv4Address& localaddr );

    /**
     *  Attempts to set the default interface to be used for multicast   
     *
     *  @param multicastInterface The local interface to use by default
     */
    bool SetDefaultMulticastInterface( const CIPv4Address& multicastInterface );

    /**
     *  Attempts to join the given multicast group using exclusive mode
     *  This will accept multicast data from any source.
     *  To prevent receiving data from specific sources use Block/Unblock   
     *
     *  @param multicastGroup The multicast group to join
     *  @param alwaysJoinOnAllNetworkInterfaces whether to auto join on all available network interfaces
     *  @param treatHostInterfaceOfAnyAsJoinOnAll whether to join on all available network interfaces if the host bind interface is set to ANY
     */
    bool Join( const CIPv4Address& multicastGroup                ,
               bool alwaysJoinOnAllNetworkInterfaces = false   ,
               bool treatHostInterfaceOfAnyAsJoinOnAll = false );

    /**
     *  Attempts to join the given multicast group using exclusive mode
     *  This will accept multicast data from any source.
     *  To prevent receiving data from specific sources use Block/Unblock   
     *
     *  @param multicastGroup The multicast group to join
     *  @param networkInterfaceToJoinOn which network interface to use for the join request
     */
    bool JoinOnInterface( const CIPv4Address& multicastGroup           ,
                          const CIPv4Address& networkInterfaceToJoinOn );

    /**
     *  Attempts to join the given multicast group using inclusive mode
     *  This will accept multicast data from the source specified.
     *  To add additional allowed source addresses simply invoke this repeatedly per allowed source    
     *
     *  @param multicastGroup The multicast group to join
     *  @param srcAddr The allowed source address of multicast data
     */
    bool Join( const CIPv4Address& multicastGroup ,
               const CIPv4Address& srcAddr        );

    /**
     *  Attempts to join the leave multicast group using exclusive mode
     *  This will stop accepting multicast data from any source. 
     *
     *  @param multicastGroup The multicast group to join
     */
    bool Leave( const CIPv4Address& multicastGroup );

    /**
     *  Attempts to leave the given multicast group using inclusive mode
     *  This will stop accepting multicast data from the source specified.
     *  To leave for additional allowed source addresses simply invoke this repeatedly per allowed source    
     *
     *  @param multicastGroup The multicast group to leave
     *  @param srcAddr The allowed source address of multicast data
     */
    bool Leave( const CIPv4Address& multicastGroup ,
                const CIPv4Address& srcAddr        );

    /**
     *  Attempts to block data from the given multicast group for a specific source using exclusive mode
     *  This will stop accepting multicast data from the source specified.
     *  To block additional allowed source addresses simply invoke this repeatedly per allowed source    
     *
     *  @param multicastGroup The multicast group to leave
     *  @param srcAddr The allowed source address of multicast data
     */
    bool Block( const CIPv4Address& multicastGroup ,
                const CIPv4Address& srcAddr        );

    /**
     *   Attempts to unblock data from the given multicast group for a specific source using exclusive mode
     *  This will (re)start accepting multicast data from the source specified.
     *  To unblock additional blocked source addresses simply invoke this repeatedly per blocked source    
     *
     *  @param multicastGroup The multicast group to leave
     *  @param srcAddr The allowed source address of multicast data
     */
    bool Unblock( const CIPv4Address& multicastGroup ,
                  const CIPv4Address& srcAddr        );

    UInt16 GetPort( void ) const;

    /**
     *      Closes the socket connection.
     */
    void Close( bool shutdownOnly = false );

    /**
     *      Attempts to send a UDP packet to the given destination.
     *      Note that the data you send must fit within the payload
     *      space of a UDP packet.
     *      Note that this method is inefficient for regular
     *      transmissions to the same destination, use the other version
     *      for such a purpose.
     *
     *      @param dnsname DNS name of IP string of the remote host.
     *      @param port remote port that the data is to be sent to.
     *      @param data pointer to the data you wish to send
     *      @param datasize size in bytes of the data block pointed to by data
     *      @return the actual number of bytes that where sent. -1 indicates an error.
     */
    Int32 SendPacketTo( const CORE::CString& dnsname ,
                        UInt16 port                  ,
                        const void* data             ,
                        UInt16 datasize              );

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
                        UInt16 datasize        ) const;

    /**
     *      For blocking sockets this call will not return until data is
     *      received. For non-blocking sockets this call returns immediately
     *      if there is no data and returns an error value.
     *
     *      Received data is written into destbuf up to bufsize bytes.
     *      If the destination buffer is not large enough to hold the entire
     *      packet data then the remaining data will be lost !!!
     *
     *      If there are more packets queued after a successful read then
     *      a new UDPPacketRecievedEvent will be sent. This cycle repeats until
     *      there are no more packets in the queue.
     *
     *      @param src structure that will hold the information about the source of the data
     *      @param destbuf buffer the received data will be written to.
     *      @param bufsize size of the destination buffer in bytes.
     *      @return the number of bytes written to the given buffer, returns -1 on error.
     */
    Int32 Recieve( CIPv4Address& src ,
                   void* destbuf   ,
                   UInt16 bufsize  );

    /**
     *      Same as the other version of Recieve() except here
     *      you do not have to pass in a structure for source info.
     *      Only use this version if you don't care where the packet
     *      came from and you will use eventing to handle the payload
     *
     *      @param destbuf buffer the received data will be written to.
     *      @param bufsize size of the destination buffer in bytes.
     *      @return the number of bytes written to the given buffer, returns -1 on error.
     */
    Int32 Recieve( void* destbuf      ,
                   UInt16 bufsize     );


    /**
     *      Same as the other version of Recieve() except here
     *      you soley rely on the eventing mechanism
     *
     *      @return the number of bytes written to the given buffer, returns -1 on error.
     */
    Int32 Recieve( void );

    /**
     *      returns whether this socket is a blocking socket
     *
     *      @return whether this socket is a blocking socket
     */
    bool IsBlocking( void ) const;

    /**
     *  returns the size of the buffer used for a single received data packet
     *
     *  @return the size of the buffer for received data
     */
    UInt32 GetRecievedDataBufferSize( void ) const;

    /**
     *  sets the size of the buffer used for a single received data packet
     *  Typically you will want to set this to the exact size of your UDP packages
     *  That way you don't have to perform any additional mergers/parsing operations
     *
     *  @param newBufferSize size of the buffer for received data
     */
    bool SetRecievedDataBufferSize( const UInt32 newBufferSize );

    /**
     *  Sets the number of receive buffers utilized when reciving packages
     *  This allows for potentially more efficient batch operations
     *  If this is set to say 100 then the number of packages once can expect to receive from
     *  a UDPPacketsRecievedEvent is between 1 and 100.
     *  This reduces eventing overhead allowing for batch processing. Also on some operating systems
     *  different mechanisms could be leveraged to achieve higher throughput.
     *
     *  The historical equivelant setting is a value of 1.
     *
     *  Note that this setting together with SetRecievedDataBufferSize() determines the actual amount of memory required.
     *  If you have 100 receive buffers and a buffer size of 9216 that means the total receive buffer size will be
     *      100 * 9216 = 921600 bytes.
     *
     *  @param nrOfReceiveBuffers number of receive buffers to allocate memory for
     */
    bool SetNrOfReceiveBuffers( const UInt32 nrOfReceiveBuffers );

    /**
     *  Same as SetNrOfReceiveBuffers() which takes only nrOfReceiveBuffers except it also allows
     *  you to change the buffer size per receive buffer at the same time.
     */
    bool SetNrOfReceiveBuffers( const UInt32 nrOfReceiveBuffers, const UInt32 newBufferSize );

    UInt32 GetNrOfReceiveBuffers( void ) const;

    UInt32 GetNrOfDataReceivedEvents( bool resetCounter );

    UInt32 GetNrOfDataSentEvents( bool resetCounter );

    virtual UInt32 GetBytesReceived( bool resetCounter );

    virtual UInt32 GetBytesTransmitted( bool resetCounter );

    bool IsIncomingDataQueued( void );

    void SetAutoReOpenOnError( bool autoReOpen );

    bool GetAutoReOpenOnError( void ) const;

    void SetAllowMulticastLoopback( bool allowLoopback );

    bool GetAllowMulticastLoopback( void ) const;

    void SetMulticastTTL( Int32 ttl );

    Int32 GetMulticastTTL( void ) const;

    void SetAllowBroadcast( bool allowBroadcast );

    bool GetAllowBroadcast( void ) const;

    bool SetOsLevelSocketReceiveBufferSize( UInt32 osLevelSocketReceiveBufferSize );

    UInt32 GetOsLevelSocketReceiveBufferSize( void ) const;

    /**
     *  sets the max number of updates to perform on the socket for a given update cycle
     *  This setting applies to running a socket in non-blocking event driven mode.
     *  If you have a very busy socket you don't want the socket to get 'stuck' reading from the socket for example
     *  due to the fact that data is coming in so fast that by the time you finished reading from the socket more 
     *  data is waiting on the socket. In such cases you should use this setting to avoid this scenario.
     *
     *  @param maxUpdates number of updates to perform per update cycle
     */
    void SetMaxUpdatesPerCycle( UInt32 maxUpdates );

    UInt32 GetMaxUpdatesPerCycle( void ) const;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator );

    private:
    typedef CORE::CTEventHandlerFunctor< CUDPSocket > TEventCallback;

    void OnPulse( CORE::CNotifier* notifier           ,
                  const CORE::CEvent& eventid         ,
                  CORE::CICloneable* eventdata = NULL );

    bool Update( bool performRead );  /**< checks the socket for queued incoming data */

    bool PerformRead( void );

    bool PerformRead( UInt32 readIndex, UInt32& packetsRead );

    void RegisterEventHandlers( void );

    private:

    struct SUDPSockData;            /**< forward declaration of platform data storage structure */
    typedef struct CUDPSocket::SUDPSockData TUDPSockData;
    typedef GUCEF::vector< CORE::CDynamicBuffer > TDynamicBufferVector;

    bool m_autoReopenOnError;       /**< flag for feature to auto re-open the socket when after a socket error occurred */
    bool _blocking;                 /**< is this socket blocking ? */
    bool _checkfordata;             /**< check for data and dispatch event next update cycle ? */
    struct SUDPSockData* _data;     /**< container for platform specific data */
    CHostAddress m_hostAddress;     /**< retains the destination address and provides easy host-network conversion */
    MT::CMutex _datalock;           /**< mutex for thread-safety when manipulating the socket */
    CORE::CDynamicBuffer m_underlyingReceiveBuffer; /**< the actual buffer owning the memory used for receive buffers */
    TDynamicBufferVector m_receiveBuffers;         /**< multiple receive buffers mapped into the underlying receive buffer */
    UDPPacketsRecievedEventData m_udpPacketsReceivedEventData;
    UInt32 m_maxRecievedDataBufferSize;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    UInt32 m_maxUpdatesPerCycle;    /**< setting aimed at preventing a busy socket from hogging all the processing */
    bool m_allowMulticastLoopback;
    int m_multicastTTL;
    bool m_allowBroadcast;
    UInt32 m_osLevelSocketReceiveBufferSize;
    mutable UInt32 m_bytesReceived;
    mutable UInt32 m_bytesTransmitted;
    mutable UInt32 m_nrOfDataReceivedEvents;
    mutable UInt32 m_nrOfDataSentEvents;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */
